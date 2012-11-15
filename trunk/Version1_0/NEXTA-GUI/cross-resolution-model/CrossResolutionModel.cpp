//  Portions Copyright 2010 Tao Xing (captainxingtao@gmail.com), Xuesong Zhou (xzhou99@gmail.com)
//

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html


//    This file is part of NeXTA Version 3 (Open-source).

//    NEXTA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    NEXTA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with NEXTA.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "..//Geometry.h"
#include "..//CSVParser.h"
#include "..//TLite.h"
#include "..//Network.h"
#include "..//TLiteDoc.h"
#ifndef _WIN64
#include "..//Data-Interface//include//ogrsf_frmts.h"
#endif 
#include "..//Data-Interface//XLTestDataSource.h"
#include "..//Data-Interface//XLEzAutomation.h"
#include "..//MainFrm.h"

#include "SignalNode.h"
#include "..//Dlg_SignalDataExchange.h"

#include <tlhelp32.h>
//
// Some definitions from NTDDK and other sources
//




typedef LONG    NTSTATUS;
typedef LONG    KPRIORITY;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)

#define SystemProcessesAndThreadsInformation    5

typedef struct _CLIENT_ID {
    DWORD        UniqueProcess;
    DWORD        UniqueThread;
} CLIENT_ID;

typedef struct _UNICODE_STRING {
    USHORT        Length;
    USHORT        MaximumLength;
    PWSTR        Buffer;
} UNICODE_STRING;

typedef struct _VM_COUNTERS {
    SIZE_T        PeakVirtualSize;
    SIZE_T        VirtualSize;
    ULONG        PageFaultCount;
    SIZE_T        PeakWorkingSetSize;
    SIZE_T        WorkingSetSize;
    SIZE_T        QuotaPeakPagedPoolUsage;
    SIZE_T        QuotaPagedPoolUsage;
    SIZE_T        QuotaPeakNonPagedPoolUsage;
    SIZE_T        QuotaNonPagedPoolUsage;
    SIZE_T        PagefileUsage;
    SIZE_T        PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS {
    LARGE_INTEGER   KernelTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   CreateTime;
    ULONG            WaitTime;
    PVOID            StartAddress;
    CLIENT_ID        ClientId;
    KPRIORITY        Priority;
    KPRIORITY        BasePriority;
    ULONG            ContextSwitchCount;
    LONG            State;
    LONG            WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

// Note that the size of the SYSTEM_PROCESSES structure is 
// different on NT 4 and Win2K, but we don't care about it, 
// since we don't access neither IoCounters member nor 
//Threads array

typedef struct _SYSTEM_PROCESSES {
    ULONG            NextEntryDelta;
    ULONG            ThreadCount;
    ULONG            Reserved1[6];
    LARGE_INTEGER   CreateTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   KernelTime;
    UNICODE_STRING  ProcessName;
    KPRIORITY        BasePriority;
    ULONG            ProcessId;
    ULONG            InheritedFromProcessId;
    ULONG            HandleCount;
    ULONG            Reserved2[2];
    VM_COUNTERS        VmCounters;
#if _WIN32_WINNT >= 0x500
    IO_COUNTERS        IoCounters;
#endif
    SYSTEM_THREADS  Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;


class CProcessManager
{
private:
    //Functions loaded from Kernel32
    typedef HANDLE (WINAPI *PFCreateToolhelp32Snapshot)(
        DWORD dwFlags,       
        DWORD th32ProcessID  
        );

    typedef BOOL (WINAPI *PFProcess32First)(
        HANDLE hSnapshot,      
        LPPROCESSENTRY32 lppe  
        );

    typedef BOOL (WINAPI *PFProcess32Next)(
        HANDLE hSnapshot,      
        LPPROCESSENTRY32 lppe  
        );

    // Native NT API Definitions
    typedef NTSTATUS (WINAPI * PFZwQuerySystemInformation)
        (UINT, PVOID, ULONG, PULONG);
    typedef HANDLE (WINAPI* PFGetProcessHeap)(VOID);
    typedef LPVOID (WINAPI* PFHeapAlloc)
        (HANDLE,DWORD,SIZE_T);
    typedef BOOL (WINAPI* PFHeapFree)(HANDLE,DWORD,LPVOID);
public:
    CProcessManager() : FCreateToolhelp32Snapshot(NULL), 
        FProcess32First(NULL), FProcess32Next(NULL), 
        m_hKernelLib(NULL),
        m_hNTLib(NULL)
    {
        m_hKernelLib = ::LoadLibraryA("Kernel32");
        if (m_hKernelLib)  
        {
            // Find ToolHelp functions
            FCreateToolhelp32Snapshot = 
                (PFCreateToolhelp32Snapshot)
                ::GetProcAddress(m_hKernelLib,
                _TEXT("CreateToolhelp32Snapshot"));
            FProcess32First = (PFProcess32First)
                ::GetProcAddress(m_hKernelLib, 
                _TEXT("Process32First"));
            FProcess32Next = (PFProcess32Next)
                ::GetProcAddress(m_hKernelLib, 
                _TEXT("Process32Next"));
        }
        if(!FCreateToolhelp32Snapshot || 
            !FProcess32First || !FProcess32Next)
        { // i.e. we couldn't find the ToolHelp functions, 
            //so we must be on NT4. Let's load the
            // undocumented one instead.
            if(!m_hKernelLib)
                return; // can't do anything at all without 
            //the kernel.

            m_hNTLib = ::LoadLibraryA("ntdll.dll");
            if(m_hNTLib)
            {
                FQuerySysInfo = 
                    (PFZwQuerySystemInformation)
                    ::GetProcAddress(m_hNTLib, 
                    _TEXT("ZwQuerySystemInformation"));
                // load some support funcs from the kernel
                FGetProcessHeap = (PFGetProcessHeap)
                    ::GetProcAddress(m_hKernelLib, 
                    _TEXT("GetProcessHeap"));
                FHeapAlloc = (PFHeapAlloc)
                    ::GetProcAddress(m_hKernelLib, 
                    _TEXT("HeapAlloc"));
                FHeapFree = (PFHeapFree)
                    ::GetProcAddress(m_hKernelLib, 
                    _TEXT("HeapFree"));
            }
        }
    }
    ~CProcessManager()
    {
        if(m_hKernelLib)
            FreeLibrary(m_hKernelLib);
        if(m_hNTLib)
            FreeLibrary(m_hNTLib);
    }
    bool KillProcess(IN const char* pstrProcessName)
    {
        DWORD dwId;
        HANDLE hProcess = FindProcess(pstrProcessName, 
            dwId);
        BOOL bResult;
        if(!hProcess)
            return false;

        // TerminateAppEnum() posts WM_CLOSE to all windows whose PID
        // matches your process's.
        ::EnumWindows((WNDENUMPROC)
            CProcessManager::TerminateAppEnum, 
            (LPARAM) dwId);
        // Wait on the handle. If it signals, great. 
        //If it times out, then you kill it.
        if(WaitForSingleObject(hProcess, 1000)
            !=WAIT_OBJECT_0)
            bResult = TerminateProcess(hProcess,0);
        else
            bResult = TRUE; 
        CloseHandle(hProcess);
        return bResult == TRUE;
    }

    HANDLE FindProcess(IN const char* pstrProcessName,
        OUT DWORD& dwId)
    {
        if(!m_hKernelLib)
            return NULL;

        if(FCreateToolhelp32Snapshot && FProcess32First && 
            FProcess32Next) // use toolhelpapi
            return THFindProcess(pstrProcessName, dwId);
        if(FQuerySysInfo && FHeapAlloc && 
            FGetProcessHeap && FHeapFree) // use NT api
            return NTFindProcess(pstrProcessName, dwId);
        // neither one got loaded. Strange.
        return NULL;
    }

private:
    HANDLE THFindProcess(IN const char* pstrProcessName, 
        OUT DWORD& dwId)
    {
        HANDLE            hSnapShot=NULL;
        HANDLE            hResult = NULL;
        PROCESSENTRY32    processInfo;
        char*            pstrExeName;

        bool bFirst = true;
        ::ZeroMemory(&processInfo, sizeof(PROCESSENTRY32));
        processInfo.dwSize = sizeof(PROCESSENTRY32);
        hSnapShot = FCreateToolhelp32Snapshot(
            TH32CS_SNAPPROCESS, 0);
        if(hSnapShot == INVALID_HANDLE_VALUE)
            return NULL; 

        // ok now let's iterate with Process32Next until we 
        // match up the name of our process
        while((bFirst ? FProcess32First(hSnapShot, 
            &processInfo) : FProcess32Next(hSnapShot, 
            &processInfo)))
        {
            bFirst = false;
            // we need to check for path... and extract 
            // just the exe name
            pstrExeName = strrchr(processInfo.szExeFile, 
                '\\');
            if(!pstrExeName)
                pstrExeName = processInfo.szExeFile;
            else
                pstrExeName++; // skip the \
            // ok now compare against our process name
            if(stricmp(pstrExeName, pstrProcessName) == 0) 
                // wee weee we found it
            {
                // let's get a HANDLE on it
                hResult=OpenProcess(
                    SYNCHRONIZE|PROCESS_TERMINATE, TRUE, 
                    processInfo.th32ProcessID);
                dwId = processInfo.th32ProcessID;
                break;
            }
        } // while(Process32Next(hSnapShot, &processInfo){
        if(hSnapShot)
            CloseHandle(hSnapShot);
        return hResult;
    }
    HANDLE NTFindProcess(IN const char* pstrProcessName, 
        OUT DWORD& dwId)
    {
        HANDLE hHeap = FGetProcessHeap();
        NTSTATUS Status;
        ULONG cbBuffer = 0x8000;
        PVOID pBuffer = NULL;
        HANDLE hResult = NULL;
        // it is difficult to say a priory which size of 
        // the buffer will be enough to retrieve all 
        // information, so we startwith 32K buffer and 
        // increase its size until we get the
        // information successfully
        do
        {
            pBuffer = HeapAlloc(hHeap, 0, cbBuffer);
            if (pBuffer == NULL)
                return SetLastError(
                ERROR_NOT_ENOUGH_MEMORY), NULL;

            Status = FQuerySysInfo(
                SystemProcessesAndThreadsInformation,
                pBuffer, cbBuffer, NULL);

            if (Status == STATUS_INFO_LENGTH_MISMATCH)
            {
                HeapFree(hHeap, 0, pBuffer);
                cbBuffer *= 2;
            }
            else if (!NT_SUCCESS(Status))
            {
                HeapFree(hHeap, 0, pBuffer);
                return SetLastError(Status), NULL;
            }
        }
        while (Status == STATUS_INFO_LENGTH_MISMATCH);

        PSYSTEM_PROCESSES pProcesses = 
            (PSYSTEM_PROCESSES)pBuffer;

        for (;;)
        {
            PCWSTR pszProcessName = 
                pProcesses->ProcessName.Buffer;
            if (pszProcessName == NULL)
                pszProcessName = L"Idle";

            CHAR szProcessName[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, pszProcessName, 
                -1,szProcessName, MAX_PATH, NULL, NULL);

            if(stricmp(szProcessName, pstrProcessName) 
                == 0) // found it
            {
                hResult=OpenProcess(
                    SYNCHRONIZE|PROCESS_TERMINATE, TRUE, 
                    pProcesses->ProcessId);
                dwId = pProcesses->ProcessId;
                break;
            }

            if (pProcesses->NextEntryDelta == 0)
                break;

            // find the address of the next process 
            // structure
            pProcesses = (PSYSTEM_PROCESSES)(
                ((LPBYTE)pProcesses)
                + pProcesses->NextEntryDelta);
        }

        HeapFree(hHeap, 0, pBuffer);
        return hResult;
    }
    // callback function for window enumeration
    static BOOL CALLBACK TerminateAppEnum( HWND hwnd, 
        LPARAM lParam )
    {
        DWORD dwID ;

        GetWindowThreadProcessId(hwnd, &dwID) ;

        if(dwID == (DWORD)lParam)
        {
            PostMessage(hwnd, WM_CLOSE, 0, 0) ;
        }

        return TRUE ;
    }
    HMODULE            m_hNTLib;
    HMODULE            m_hKernelLib;
    // ToolHelp related functions
    PFCreateToolhelp32Snapshot    FCreateToolhelp32Snapshot;
    PFProcess32First            FProcess32First;
    PFProcess32Next                FProcess32Next;
    // native NT api functions
    PFZwQuerySystemInformation    FQuerySysInfo;
    PFGetProcessHeap            FGetProcessHeap;
    PFHeapAlloc                    FHeapAlloc;
    PFHeapFree                    FHeapFree;
};

DTA_Approach CTLiteDoc::Find_Closest_Angle_to_Approach(int angle)
{
	if(angle < 23)
	{
		return DTA_East;
	}else if(angle < 45 && m_ApproachMap.find(DTA_East)== m_ApproachMap.end())  // East has not been used previously
	{
		return DTA_East;
	}
	else if(angle < 68 ) 
	{
		if(m_ApproachMap.find(DTA_North)== m_ApproachMap.end())  //North not used
			return DTA_North;
		else
			return DTA_NorthEast;
	}
	else if(angle < 113) 
	{
		return DTA_North;
	}
	else if(angle < 135) 
	{
		if(m_ApproachMap.find(DTA_North)== m_ApproachMap.end())  //North not used
			return DTA_North;
		else
			return DTA_NorthWest;
	}
	else if(angle < 158) 
	{
		if(m_ApproachMap.find(DTA_West)== m_ApproachMap.end())  //West not used
			return DTA_West;
		else
			return DTA_NorthWest;
	}
	else if(angle < 203) 
	{
		return DTA_West;
	}
	else if(angle < 225 && m_ApproachMap.find(DTA_West)== m_ApproachMap.end())  //West not used
	{
		return DTA_West;
	}
	else if(angle < 248) 
	{
		if(m_ApproachMap.find(DTA_South)== m_ApproachMap.end())  //South not used
			return DTA_South;
		else
			return DTA_SouthWest;
	}
	else if(angle < 293) 
	{
		return DTA_South;
	}
	else if(angle < 315) 
	{
		if(m_ApproachMap.find(DTA_South)== m_ApproachMap.end())  //South not used
			return DTA_South;
		else
			return DTA_SouthEast;
	}
	else if(angle < 338) 
	{
		if(m_ApproachMap.find(DTA_East)== m_ApproachMap.end())  //East not used
			return DTA_East;
		else
			return DTA_SouthEast;
	}
	else
		return DTA_East;
}
DTA_Approach CTLiteDoc::g_Angle_to_Approach_8_direction(int angle)
{
	if(angle < 23)
	{
		return DTA_East;
	}
	else if(angle < 68) 
	{
		return DTA_NorthEast;
	}
	else if(angle < 113) 
	{
		return DTA_North;
	}
	else if(angle < 158) 
	{
		return DTA_NorthWest;
	}
	else if(angle < 203) 
	{
		return DTA_West;
	}
	else if(angle < 248) 
	{
		return DTA_SouthWest;
	}
	else if(angle < 293) 
	{
		return DTA_South;
	}
	else if(angle < 338) 
	{
		return DTA_SouthEast;
	}
	else
		return DTA_East;
}

DTA_Approach CTLiteDoc::g_Angle_to_Approach_4_direction(int angle)
{
	if(angle < 45)
	{
		return DTA_East;
	}
	else if(angle < 135) 
	{
		return DTA_North;
	}
	else if(angle < 225) 
	{
		return DTA_West;
	}
	else if(angle < 315) 
	{
		return DTA_South;
	}
	else
		return DTA_East;
}

DTA_Turn CTLiteDoc::Find_RelativeAngle_to_Left_OR_Right_Turn_1_OR_2(int relative_angle)
{
	int min_diff = 89;

	int ideal_left = 90;
	int ideal_right = -90;
	int ideal_through = 0;

	if(abs(relative_angle - ideal_left) <= min_diff)
		return DTA_LeftTurn;

	if(abs(relative_angle - ideal_right) <= min_diff)
		return DTA_RightTurn;

	return DTA_OtherTurn;
}


DTA_Turn CTLiteDoc::Find_RelativeAngle_to_Left_OR_Right_Turn(int relative_angle)
{
	int min_diff = 89;

	int ideal_left = 90;
	int ideal_right = -90;
	int ideal_through = 0;


	if(abs(relative_angle - ideal_left) <= min_diff)
		return DTA_LeftTurn;

	if(abs(relative_angle - ideal_right) <= min_diff)
		return DTA_RightTurn;

	return DTA_OtherTurn;
}

DTA_Turn CTLiteDoc::Find_RelativeAngle_to_Turn(int relative_angle)
{
	int min_through_diff = 45;

	int ideal_left = 90;
	int ideal_right = -90;
	int ideal_through = 0;


	if(abs(relative_angle - ideal_through) <= min_through_diff)
		return DTA_Through;

	if(abs(relative_angle - ideal_left) <= min_through_diff)
		return DTA_LeftTurn;

	if(abs(relative_angle - ideal_right) <= min_through_diff)
		return DTA_RightTurn;

	return DTA_OtherTurn;
}


int CTLiteDoc::Find_P2P_Angle(GDPoint p1, GDPoint p2)
{
	float delta_x  = p2.x - p1.x;
	float delta_y  = p2.y - p1.y;

	if(fabs(delta_x) < 0.00001)
		delta_x =0;

	if(fabs(delta_y) < 0.00001)
		delta_y =0;

	int angle = atan2(delta_y, delta_x) * 180 / PI + 0.5;
	// angle = 90 - angle;

	while(angle < 0) 
		angle += 360;

	while(angle > 360) 
		angle -= 360;

	return angle;

}
DTA_Turn CTLiteDoc::Find_PPP_to_Turn(GDPoint p1, GDPoint p2, GDPoint p3)
{

	int relative_angle = Find_PPP_RelativeAngle(p1,p2,p3);
	return Find_RelativeAngle_to_Turn(relative_angle);

}


DTA_Turn CTLiteDoc::Find_PPP_to_All_Turns_with_DTAApproach(GDPoint p1, GDPoint p2, GDPoint p3,DTA_Approach approach1, DTA_Approach approach2)
{
	if(approach2 == m_OpposingDirectionMap[approach1])
		return DTA_Through;
	else
	{
		int relative_angle = Find_PPP_RelativeAngle(p1,p2,p3);
		return Find_RelativeAngle_to_Left_OR_Right_Turn_1_OR_2(relative_angle);
	}

}


DTA_Turn CTLiteDoc::Find_PPP_to_Turn_with_DTAApproach(GDPoint p1, GDPoint p2, GDPoint p3,DTA_Approach approach1, DTA_Approach approach2)
{
	if(approach2 == m_OpposingDirectionMap[approach1])
		return DTA_Through;
	else
	{
		int relative_angle = Find_PPP_RelativeAngle(p1,p2,p3);
		return Find_RelativeAngle_to_Left_OR_Right_Turn(relative_angle);
	}

}


int CTLiteDoc::Find_PPP_RelativeAngle(GDPoint p1, GDPoint p2, GDPoint p3)
{
	int relative_angle;

	int angle1 = Find_P2P_Angle(p1,p2);
	int angle2 = Find_P2P_Angle(p2,p3);
	relative_angle = angle2 - angle1;

	while(relative_angle > 180) 
		relative_angle -= 360;

	while(relative_angle < -180)
		relative_angle += 360;

	return relative_angle;
}

int CTLiteDoc::FindUniqueLinkID()
{
	int i;
	for( i= 1; i < m_LinkIDRecordVector.size(); i++) 
	{
		if( m_LinkIDRecordVector[i] == 0)
		{
			m_LinkIDRecordVector[i] = 1;
			return i;
		}
	}

	// all link ids have been used;
	m_LinkIDRecordVector.push_back(i);
	m_LinkIDRecordVector[i] = 1;
	return i;
}
void CTLiteDoc::AssignUniqueLinkIDForEachLink()
{
	return;

	std::list<DTALink*>::iterator iLink;

	int max_link_id = 1;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_LinkID >  max_link_id)
		{
			max_link_id = (*iLink)->m_LinkID ;
		}

	}

	//allocate unique id arrary

	m_LinkIDRecordVector.clear();

	max_link_id = max(max_link_id, m_LinkSet.size());
	for(int i = 0; i <= max_link_id; i++)
	{
		m_LinkIDRecordVector.push_back(0);
	}


	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_LinkID != 0)  // with possible real id
		{
			if(m_LinkIDRecordVector[(*iLink)->m_LinkID] == 0)  // not been used before
			{
				// mark link id
				m_LinkIDRecordVector[(*iLink)->m_LinkID] =1;
			}else
			{  // link id has been used
				(*iLink)->m_LinkID = 0; // need to reset link id;
			}

		}

	}

	// last step: find unique id 
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_LinkID == 0)
		{
			// need a new unique link id
			(*iLink)->m_LinkID = FindUniqueLinkID();

		}
	}

}

void  CTLiteDoc::Construct4DirectionMovementVector(bool ResetFlag) // this part has four directions
{
	if(m_bMovementAvailableFlag == true && ResetFlag == false)
		return;

	m_AdjLinkSize = 0;

	for (std::list<DTANode*>::iterator iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		if((*iNode)->m_Connections >0)  // we might try import node layer only from shape file, so all nodes have no connected links. 
		{
			if((*iNode)->m_Connections > m_AdjLinkSize)
				m_AdjLinkSize = (*iNode)->m_Connections;

		}

	}

	m_Network.Initialize (m_NodeSet.size(), m_LinkSet.size(), 1, m_AdjLinkSize);
	m_Network.BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, false);

	// generate all movements
	int i = 0;
	for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i++)
	{  // for current node

		// scan each inbound link and outbound link


		// generate two default phases;
		(*iNode)->m_PhaseVector.clear();

		DTANodePhase node_phase;
		(*iNode)->m_PhaseVector.push_back(node_phase); // phase 1;
		(*iNode)->m_PhaseVector.push_back(node_phase); // phase 2;


		int inbound_i;

		for(inbound_i= 0; inbound_i< m_Network.m_InboundSizeAry[i]; inbound_i++)
		{
			// for each incoming link 
			for(int outbound_i= 0; outbound_i< m_Network.m_OutboundSizeAry [i]; outbound_i++)
			{
				//for each outging link
				int LinkID = m_Network.m_InboundLinkAry[i][inbound_i];

				if (m_Network.m_FromIDAry[LinkID] != m_Network.m_OutboundNodeAry [i][outbound_i])
				{
					// do not consider u-turn

					DTANodeMovement element;

					element.in_link_to_node_id = i;						

					element.IncomingLinkID = m_Network.m_InboundLinkAry[i][inbound_i];;
					element.OutgoingLinkID = m_Network.m_OutboundLinkAry [i][outbound_i];

					element.in_link_from_node_id = m_Network.m_FromIDAry[LinkID];
					element.out_link_to_node_id = m_Network.m_OutboundNodeAry [i][outbound_i];

					if((*iNode)->m_NodeNumber == 52508 )
					{
						TRACE("Up Node: %d\n",m_NodeIDMap[element.in_link_from_node_id]->m_NodeNumber);
					}

					GDPoint p1, p2, p3;
					p1  = m_NodeIDMap[element.in_link_from_node_id]->pt;
					p2  = m_NodeIDMap[element.in_link_to_node_id]->pt;
					p3  = m_NodeIDMap[element.out_link_to_node_id]->pt;

					element.movement_approach = g_Angle_to_Approach_4_direction(Find_P2P_Angle(p1,p2));
					// movement_approach is for in_bound link
					DTALink* pLink = m_LinkNoMap[LinkID];
					if(pLink!=NULL)
					{
						// we have not considered different directions/approach with respect to from and to nodes (e.g. with a curve)
						pLink ->m_FromApproach = element.movement_approach;
						pLink ->m_ToApproach = g_Angle_to_Approach_8_direction(Find_P2P_Angle(p2,p1));
					}


					element.movement_turn = Find_PPP_to_Turn(p1,p2,p3);

					// determine  movement type /direction here
					element.movement_dir = DTA_LANES_COLUME_init;

					int PhaseIndex  = 0;
					switch (element.movement_approach)
					{
					case DTA_North:
						PhaseIndex = 0;
						switch (element.movement_turn)
						{
						case DTA_LeftTurn: element.movement_dir = DTA_NBL; break;
						case DTA_Through: element.movement_dir = DTA_NBT; break;
						case DTA_RightTurn: element.movement_dir = DTA_NBR; break;
						}
						break;
					case DTA_East:
						PhaseIndex = 1;
						switch (element.movement_turn)
						{
						case DTA_LeftTurn: element.movement_dir = DTA_EBL; break;
						case DTA_Through: element.movement_dir = DTA_EBT; break;
						case DTA_RightTurn: element.movement_dir = DTA_EBR; break;
						}
						break;
					case DTA_South:
						PhaseIndex = 0;
						switch (element.movement_turn)
						{
						case DTA_LeftTurn: element.movement_dir = DTA_SBL; break;
						case DTA_Through: element.movement_dir = DTA_SBT; break;
						case DTA_RightTurn: element.movement_dir = DTA_SBR; break;
						}
						break;
					case DTA_West:
						PhaseIndex = 1;
						switch (element.movement_turn)
						{
						case DTA_LeftTurn: element.movement_dir = DTA_WBL; break;
						case DTA_Through: element.movement_dir = DTA_WBT; break;
						case DTA_RightTurn: element.movement_dir = DTA_WBR; break;
						}
						break;
					default:
						TRACE("");
					}

					int movement_index = (*iNode)->m_MovementVector.size();
					element.phase_index = PhaseIndex;
					(*iNode)->m_MovementVector.push_back(element);

					//record the movement index into the right phase index
					(*iNode)->m_PhaseVector[PhaseIndex].movement_index_vector .push_back(movement_index);


				}  // for each feasible movement (without U-turn)

			} // for each outbound link

		} // for each inbound link
	}// for each node

	m_bMovementAvailableFlag = true;
}

void CTLiteDoc::ConstructMovementVector(bool flag_Template) 
// this function has 8 directions
{
	if(m_bMovementAvailableFlag == true)
		return;

	m_bMovementAvailableFlag = true; // executed once only

	m_OpposingDirectionMap[DTA_North] = DTA_South;
	m_OpposingDirectionMap[DTA_South] = DTA_North;

	m_OpposingDirectionMap[DTA_East] = DTA_West;
	m_OpposingDirectionMap[DTA_West] =  DTA_East;

	m_OpposingDirectionMap[DTA_NorthEast] =  DTA_SouthWest;
	m_OpposingDirectionMap[DTA_SouthWest] =  DTA_NorthEast;

	m_OpposingDirectionMap[DTA_SouthEast] =  DTA_NorthWest;
	m_OpposingDirectionMap[DTA_NorthWest] =  DTA_SouthEast;

	m_Synchro_ProjectDirectory  = m_ProjectDirectory + "Exporting_Synchro_UTDF\\";

	ReadSynchroPreGeneratedLayoutFile(m_Synchro_ProjectDirectory+"Synchro_layout.csv");

	m_MovementVector.clear();
	m_PhaseVector.clear();

	DTA_NodeMovementSet MovementTemplate; // template with 12 movements
	DTA_NodePhaseSet PhaseTemplate; // template with 8 phases

	//LoadMovementTemplateFile(MovementTemplate, PhaseTemplate);

	if (flag_Template)
	{
		//LoadMovementDefault(MovementTemplate, PhaseTemplate);
		if (!LoadMovementTemplateFile(MovementTemplate, PhaseTemplate))
			LoadMovementDefault(MovementTemplate, PhaseTemplate);
	}
	else
	{
		LoadMovementDefault(MovementTemplate, PhaseTemplate);
	}


	int signal_count = 0;
	// generate all movements
	int i = 0;
	for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i++)
	{  // for current node

		//		if ((*iNode)->m_ControlType == m_ControlType_PretimedSignal || (*iNode)->m_ControlType == m_ControlType_ActuatedSignal)  //(m_Network.m_InboundSizeAry[i] >= 3) // add node control types
		{
			signal_count ++;
			// generate movement set
			DTA_NodeMovementSet movement_set;	

			// generate DTA_NodePhaseSet for this Node
			DTA_NodePhaseSet PhaseSet;
			PhaseSet.copy_parameters(PhaseTemplate);


			// step 1: mark movement turn first
			m_ApproachMap.clear();
			for(int inbound_i= 0; inbound_i< m_Network.m_InboundSizeAry[i]; inbound_i++)
			{
				// for each incoming link
				for(int outbound_i= 0; outbound_i< m_Network.m_OutboundSizeAry [i]; outbound_i++)
				{
					//for each outging link
					int LinkID = m_Network.m_InboundLinkAry[i][inbound_i];

					if (m_Network.m_FromIDAry[LinkID] != m_Network.m_OutboundNodeAry [i][outbound_i])
					{
						// do not consider u-turn

						DTA_Movement element;

						element.CurrentNodeID = i;						

						element.InboundLinkID = LinkID;
						element.UpNodeID = m_Network.m_FromIDAry[LinkID];
						element.DestNodeID = m_Network.m_OutboundNodeAry [i][outbound_i];

						GDPoint p1, p2, p3;
						p1  = m_NodeIDMap[element.UpNodeID]->pt;
						p2  = m_NodeIDMap[element.CurrentNodeID]->pt;
						p3  = m_NodeIDMap[element.DestNodeID]->pt;

						if(m_NodeIDMap[element.CurrentNodeID]->m_NodeNumber==93)
						{
						TRACE("Node 1: %d\nNode 2: %d\nNode 3: %d\n",m_NodeIDMap[element.UpNodeID]->m_NodeNumber,m_NodeIDMap[element.CurrentNodeID]->m_NodeNumber, m_NodeIDMap[element.DestNodeID]->m_NodeNumber );
						}

						element.movement_approach = g_Angle_to_Approach_8_direction(Find_P2P_Angle(p1,p2));

						m_ApproachMap[element.movement_approach] = 1;

					}
				}
			}


			// step 2: scan each inbound link and outbound link

			for(int inbound_i= 0; inbound_i< m_Network.m_InboundSizeAry[i]; inbound_i++)
			{

				// pre processing to determine the number of turning movements

				std::map<DTA_Turn,int> MovementCount;
				std::map<DTA_Turn,int> MaxAbsAngelForMovement;

				///////////////////////////////// begin of preprocessing              ////////////////////////////////////////////////////////////////////////////////
				{

					// for each incoming link
					for(int outbound_i= 0; outbound_i< m_Network.m_OutboundSizeAry [i]; outbound_i++)
					{
						//for each outging link
						int LinkID = m_Network.m_InboundLinkAry[i][inbound_i];

						if (m_Network.m_FromIDAry[LinkID] != m_Network.m_OutboundNodeAry [i][outbound_i])
						{
							// do not consider u-turn

							DTA_Movement element;

							element.CurrentNodeID = i;						

							element.InboundLinkID = LinkID;
							element.UpNodeID = m_Network.m_FromIDAry[LinkID];
							element.DestNodeID = m_Network.m_OutboundNodeAry [i][outbound_i];

							GDPoint p1, p2, p3;
							p1  = m_NodeIDMap[element.UpNodeID]->pt;
							p2  = m_NodeIDMap[element.CurrentNodeID]->pt;
							p3  = m_NodeIDMap[element.DestNodeID]->pt;


							// method 1: identify movement approach from scratch 
							element.movement_approach = Find_Closest_Angle_to_Approach(Find_P2P_Angle(p1,p2));
							// method 2: read pre-generated layout file from synchro

							CString str_key;
							str_key.Format("%d,%d",element.CurrentNodeID+1, element.UpNodeID+1);


							element.movement_turn = Find_PPP_to_Turn(p1,p2,p3);

							if (m_PredefinedApproachMap.find(str_key) != m_PredefinedApproachMap.end())  // approach has been predefined in synchro_layout.csv file
							{
								element.movement_approach = m_PredefinedApproachMap[str_key];

								str_key.Format("%d,%d",element.CurrentNodeID+1, element.DestNodeID+1);

								DTA_Approach approach2= m_PredefinedApproachMap[str_key];

								element.movement_turn = Find_PPP_to_Turn_with_DTAApproach(p1,p2,p3,element.movement_approach,approach2);
								MovementCount[element.movement_turn]+=1;

								if(MaxAbsAngelForMovement.find(element.movement_turn) == MaxAbsAngelForMovement.end()) // initialize
								{

									MaxAbsAngelForMovement[element.movement_turn] = abs(Find_PPP_RelativeAngle(p1,p2,p3));
								}else  // with data before
								{
									int relative_angel = abs(Find_PPP_RelativeAngle(p1,p2,p3));

									if(relative_angel > MaxAbsAngelForMovement[element.movement_turn])
										MaxAbsAngelForMovement[element.movement_turn] = relative_angel;

								}

							}

						}
					}
				}
				////////////////////////////////// end of processing /////////////////////////////////////////////// //

				// for each incoming link
				for(int outbound_i= 0; outbound_i< m_Network.m_OutboundSizeAry [i]; outbound_i++)
				{
					//for each outging link
					int LinkID = m_Network.m_InboundLinkAry[i][inbound_i];

					if (m_Network.m_FromIDAry[LinkID] != m_Network.m_OutboundNodeAry [i][outbound_i])
					{
						// do not consider u-turn

						DTA_Movement element;

						element.CurrentNodeID = i;						

						element.InboundLinkID = LinkID;
						element.UpNodeID = m_Network.m_FromIDAry[LinkID];
						element.DestNodeID = m_Network.m_OutboundNodeAry [i][outbound_i];

						GDPoint p1, p2, p3;
						p1  = m_NodeIDMap[element.UpNodeID]->pt;
						p2  = m_NodeIDMap[element.CurrentNodeID]->pt;
						p3  = m_NodeIDMap[element.DestNodeID]->pt;


						// method 1: identify movement approach from scratch 
						element.movement_approach = Find_Closest_Angle_to_Approach(Find_P2P_Angle(p1,p2));
						// method 2: read pre-generated layout file from synchro

						CString str_key;
						str_key.Format("%d,%d",element.CurrentNodeID+1, element.UpNodeID+1);


						element.movement_turn = Find_PPP_to_Turn(p1,p2,p3);

						if (m_PredefinedApproachMap.find(str_key) != m_PredefinedApproachMap.end())  // approach has been predefined in synchro_layout.csv file
						{
							element.movement_approach = m_PredefinedApproachMap[str_key];

							str_key.Format("%d,%d",element.CurrentNodeID+1, element.DestNodeID+1);

							DTA_Approach approach2= m_PredefinedApproachMap[str_key];

							element.movement_turn = Find_PPP_to_Turn_with_DTAApproach(p1,p2,p3,element.movement_approach,approach2);

							if(MovementCount[element.movement_turn] >=2)
							{
								// we have more than 1 movement, use angle to determine movement again

								int relative_angel = abs(Find_PPP_RelativeAngle(p1,p2,p3));

								if(relative_angel == MaxAbsAngelForMovement[element.movement_turn])
								{  // adjust movement

									if(element.movement_turn == DTA_LeftTurn)
										element.movement_turn = DTA_LeftTurn2;

									if(element.movement_turn == DTA_RightTurn)
										element.movement_turn = DTA_RightTurn2;

								}


							}

						}


						// determine  movement type /direction here
						element.movement_dir = DTA_LANES_COLUME_init;
						switch (element.movement_approach)
						{
						case DTA_North:
							switch (element.movement_turn)
							{
							case DTA_Through: element.movement_dir = DTA_NBT; break;
							case DTA_LeftTurn: element.movement_dir = DTA_NBL; break;
							case DTA_RightTurn: element.movement_dir = DTA_NBR; break;
							case DTA_LeftTurn2: element.movement_dir = DTA_NBL2; break;
							case DTA_RightTurn2: element.movement_dir = DTA_NBR2; break;
							}
							break;
						case DTA_East:
							switch (element.movement_turn)
							{
							case DTA_Through: element.movement_dir = DTA_EBT; break;
							case DTA_LeftTurn: element.movement_dir = DTA_EBL; break;
							case DTA_RightTurn: element.movement_dir = DTA_EBR; break;
							case DTA_LeftTurn2: element.movement_dir = DTA_EBL2; break;
							case DTA_RightTurn2: element.movement_dir = DTA_EBR2; break;
							}
							break;
						case DTA_South:
							switch (element.movement_turn)
							{
							case DTA_Through: element.movement_dir = DTA_SBT; break;
							case DTA_LeftTurn: element.movement_dir = DTA_SBL; break;
							case DTA_RightTurn: element.movement_dir = DTA_SBR; break;
							case DTA_LeftTurn2: element.movement_dir = DTA_SBL2; break;
							case DTA_RightTurn2: element.movement_dir = DTA_SBR2; break;
							}
							break;
						case DTA_West:
							switch (element.movement_turn)
							{
							case DTA_Through: element.movement_dir = DTA_WBT; break;
							case DTA_LeftTurn: element.movement_dir = DTA_WBL; break;
							case DTA_RightTurn: element.movement_dir = DTA_WBR; break;
							case DTA_LeftTurn2: element.movement_dir = DTA_WBL2; break;
							case DTA_RightTurn2: element.movement_dir = DTA_WBR2; break;
							}
							break;
						case DTA_NorthEast:
							switch (element.movement_turn)
							{
							case DTA_Through: element.movement_dir = DTA_NET; break;
							case DTA_LeftTurn: element.movement_dir = DTA_NEL; break;
							case DTA_RightTurn: element.movement_dir = DTA_NER; break;
							case DTA_LeftTurn2: element.movement_dir = DTA_NEL; break;
							case DTA_RightTurn2: element.movement_dir = DTA_NER; break;
							}
							break;
						case DTA_NorthWest:
							switch (element.movement_turn)
							{
							case DTA_Through: element.movement_dir = DTA_NWT; break;
							case DTA_LeftTurn: element.movement_dir = DTA_NWL; break;
							case DTA_RightTurn: element.movement_dir = DTA_NWR; break;
							case DTA_LeftTurn2: element.movement_dir = DTA_NWL; break;
							case DTA_RightTurn2: element.movement_dir = DTA_NWR; break;
							}
							break;
						case DTA_SouthEast:
							switch (element.movement_turn)
							{
							case DTA_Through: element.movement_dir = DTA_SET; break;
							case DTA_LeftTurn: element.movement_dir = DTA_SEL; break;
							case DTA_RightTurn: element.movement_dir = DTA_SER; break;
							case DTA_LeftTurn2: element.movement_dir = DTA_SEL; break;
							case DTA_RightTurn2: element.movement_dir = DTA_SER; break;
							}
							break;
						case DTA_SouthWest:
							switch (element.movement_turn)
							{
							case DTA_Through: element.movement_dir = DTA_SWT; break;
							case DTA_LeftTurn: element.movement_dir = DTA_SWL; break;
							case DTA_RightTurn: element.movement_dir = DTA_SWR; break;
							case DTA_LeftTurn2: element.movement_dir = DTA_SWL; break;
							case DTA_RightTurn2: element.movement_dir = DTA_SWR; break;
							}
							break;
						}

						if (element.movement_dir > 0)
						{
							// copy from template
							MovementTemplate.copy_to_Movement(element, element.movement_dir);
							// add Movement into m_MovementVector
							//TRACE("current node: %d, dir = %d\n", element.CurrentNodeID, element.movement_dir);
							movement_set.MovementMatrix[element.movement_dir] = element;
							movement_set.copy_from_Movement(element, element.movement_dir);
						}

						//	(*iNode)->m_MovementSet.MovementMatrix[element.movement_dir] = element;


					}  // for each feasible movement (without U-turn)

				} // for each outbound link

			} // for each inbound link

			movement_set.CurrentNodeID = i;
			m_MovementVector.push_back(movement_set);  // m_MovementVector for all nodes in the network
			TRACE("current node: %d\n", movement_set.CurrentNodeID);

			PhaseSet.CurrentNodeID = i;
			m_PhaseVector.push_back(PhaseSet);

		} // checking control type
	}// for each node

	m_ApproachMap.clear();

	if(signal_count == 0)
		AfxMessageBox("0 pretimed/actuated signal has been specified in input_node.csv in the current data set.");

	std::list<DTANode*>::iterator iNode;
	m_Origin.x  = 1000000;
	m_Origin.y  = 1000000;

	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		m_Origin.x = min(m_Origin.x,(*iNode)->pt .x);
		m_Origin.y= min(m_Origin.y,(*iNode)->pt .y);
	}

	TRACE("\n Size of m_MovementVector = %d", m_MovementVector.size());
}

bool CTLiteDoc::LoadMovementTemplateFile(DTA_NodeMovementSet& MovementTemplate, DTA_NodePhaseSet& PhaseTemplate)
{
	const int LaneColumnSize = 32;
	const int LaneRowSize = 28;
	string lane_Column_name_str[LaneColumnSize] = { "NBL2","NBL","NBT","NBR","NBR2","SBL2","SBL","SBT","SBR","SBR2","EBL2","EBL","EBT","EBR","EBR2","WBL2","WBL","WBT","WBR","WBR2","NEL","NET","NER","NWL","NWT","NWR","SEL","SET","SER","SWL","SWT","SWR"};		
	string lane_row_name_str[LaneRowSize] = {"Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	TCHAR IniFilePath[_MAX_PATH];
	sprintf_s(IniFilePath,"%s\\LaneSettings.ini", pMainFrame->m_CurrentDirectory);

	int i,j;
	//DTA_NodeMovementSet movement;// = new DTA_NodeMovementSet();
	MovementTemplate.CurrentNodeID = -1;

	for(j=0; j<LaneColumnSize;j++)
	{
		for(i=0; i<LaneRowSize; i++)
		{
			//float value = g_GetPrivateProfileFloat(lane_Column_name_str[j].c_str(), lane_row_name_str[i].c_str(), MovementTemplate.DataMatrix[i+2][j].m_text, IniFilePath);
			float value = g_GetPrivateProfileFloat(lane_Column_name_str[j].c_str(), lane_row_name_str[i].c_str(), 0, IniFilePath);
			// assign value to DataMatrix[i][j].m_text;
			MovementTemplate.DataMatrix[i+2][j].m_text = value;
		}
		// initial UpNodeID and DestNodeID as -1
		MovementTemplate.DataMatrix[0][j].m_text = -1;
		MovementTemplate.DataMatrix[1][j].m_text = -1;
	}

	const int PhaseColumnSize = 8;
	const int PhaseRowSize = 23;
	string phase_Column_name_str[PhaseColumnSize] = { "D1","D2","D3","D4","D5","D6","D7","D8"};
	string phase_row_name_str[PhaseRowSize] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};

	sprintf_s(IniFilePath,"%s\\PhaseSettings.ini", pMainFrame->m_CurrentDirectory);
	PhaseTemplate.CurrentNodeID = -1;
	for(j=0; j<PhaseColumnSize;j++)
	{
		for(i=0; i<PhaseRowSize; i++)
		{
			float value = g_GetPrivateProfileFloat(phase_Column_name_str[j].c_str(), phase_row_name_str[i].c_str(), 0, IniFilePath);
			// assign value to DataMatrix[i][j].m_text;
			PhaseTemplate.DataMatrix[i][j].m_text = value;
		}
	}

	return true;
}

bool CTLiteDoc::LoadMovementDefault(DTA_NodeMovementSet& MovementTemplate, DTA_NodePhaseSet& PhaseTemplate)
{
	// set default value to movements
	const int LaneRowSize = 28;
	const int LaneColumnSize = 32;

	double default_value_M[LaneRowSize][LaneColumnSize] = 
	{
		/*{1,2,1,1,2,1,1,2,1,1,2,1},
		{0,0,0,0,0,0,0,0,0,0,0,},
		{12,12,12,12,12,12,12,12,12,12,12,12},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,45,0,0,45,0,0,45,0,0,45,0},
		{50,50,50,50,50,50,50,50,50,50,50,50},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{5,2,0,1,6,0,7,4,0,3,8,0},
		{2,0,2,6,0,6,4,0,4,8,0,8},
		{5,2,2,1,6,6,7,4,4,3,8,8},
		{1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900},
		{4,4,4,4,4,4,4,4,4,4,4,4},
		{1863,3539,1863,1863,3539,1863,1863,3539,1863,1863,3539,1863},
		{1863,3539,1863,1863,3539,1863,1863,3539,1863,1863,3539,1863},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{1,1,1,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{20,20,20,20,20,20,20,20,20,20,20,20},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92},
		{100,100,100,100,100,100,100,100,100,100,100,100},
		{2,2,2,2,2,2,2,2,2,2,2,2},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0}*/

		{1,1,2,1,0,1,1,2,1,0,1,1,2,1,0,1,1,2,1,0,1,2,1,1,2,1,1,2,1,1,2,1},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,45,0,0,0,0,45,0,0,0,0,45,0,0,0,0,45,0,0,0,45,0,0,45,0,0,45,0,0,45,0},
		{50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{5,5,2,0,0,1,1,6,0,0,7,7,4,0,0,3,3,8,0,0,5,2,0,3,8,0,7,4,0,1,6,0},
		{8,8,0,8,6,6,8,0,8,6,6,8,0,8,6,6,8,0,8,6,8,0,8,8,0,8,8,0,8,8,0,8},
		{8,3,8,8,6,6,3,8,8,6,6,3,8,8,6,6,3,8,8,6,3,8,8,3,8,8,3,8,8,3,8,8},
		{1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900},
		{4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
		{0,1863,3539,1863,0,0,1863,3539,1863,0,0,1863,3539,1863,0,0,1863,3539,1863,0,1863,3539,1863,1863,3539,1863,1863,3539,1863,1863,3539,1863},
		{0,1863,3539,1863,0,0,1863,3539,1863,0,0,1863,3539,1863,0,0,1863,3539,1863,0,1863,3539,1863,1863,3539,1863,1863,3539,1863,1863,3539,1863},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,20,20,20,0,0,20,20,20,0,0,20,20,20,0,0,20,20,20,0,20,20,20,20,20,20,20,20,20,20,20,20},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92},
		{100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100},
		{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};

	int i,j;
	MovementTemplate.CurrentNodeID = -1;

	for(j=0; j<LaneColumnSize;j++)
	{
		for(i=0; i<LaneRowSize; i++)
		{
			MovementTemplate.DataMatrix[i+2][j].m_text = default_value_M[i][j];
		}		
		// initial UpNodeID and DestNodeID as -1
		MovementTemplate.DataMatrix[0][j].m_text = -1;
		MovementTemplate.DataMatrix[1][j].m_text = -1;
	}


	// set default values to Phases
	const int PhaseColumnSize = 8;
	const int PhaseRowSize = 23;

	double default_value_P[PhaseRowSize][PhaseColumnSize] = 
	{
		{111,112,211,212,121,122,221,222},
		{4,15,4,15,4,15,4,15},
		{20,40,20,40,20,40,20,40},
		{3,3,3,3,3,3,3,3},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{2,0.2,2,0.2,2,0.2,2,0.2},
		{4,4.5,4,4.5,4,4.5,4,4.5},
		{0,2,0,2,0,2,0,2},
		{0,1,0,1,0,1,0,1},
		{0,5,0,5,0,5,0,5},
		{0,21,0,21,0,21,0,21},
		{0,20,0,20,0,20,0,20},
		{8,32,8,32,8,32,8,32},
		{0,1,0,1,0,1,0,1},
		{1,1,1,1,1,1,1,1},
		{88,105,18,38,88,113,18,48},
		{105,18,38,88,113,18,48,88},
		{101,12,34,82,109,12,44,82},
		{101,123,34,82,109,125,44,82},
		{115,0,45,65,115,8,45,75},
		{128,39,61,109,4,39,71,109},
		{128,18,61,109,4,20,71,109}
	};

	PhaseTemplate.CurrentNodeID = -1;
	for(i=0; i<PhaseRowSize; i++)
	{
		for(j=0; j<PhaseColumnSize;j++)
		{
			PhaseTemplate.DataMatrix[i][j].m_text = default_value_P[i][j];
		}
	}
	return true;
}


void CTLiteDoc::OnFileConstructandexportsignaldata()
{

	//	if(m_import_shape_files_flag == 1)
	//		SaveProject(m_ProjectFile);

	CDlg_SignalDataExchange dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();

}
void CTLiteDoc::Constructandexportsignaldata()
{


	m_Synchro_ProjectDirectory  = m_ProjectDirectory + "Exporting_Synchro_UTDF\\";

	if ( GetFileAttributes(m_Synchro_ProjectDirectory) == INVALID_FILE_ATTRIBUTES) 
	{
		CreateDirectory(m_Synchro_ProjectDirectory,NULL);
	}
	m_Network.Initialize (m_NodeSet.size(), m_LinkSet.size(), 1, m_AdjLinkSize);
	m_Network.BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, false);

	ConstructMovementVector(true);

	ExportSynchroVersion6Files();

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	ShellExecute( NULL,  "explore", m_Synchro_ProjectDirectory, NULL,  NULL, SW_SHOWNORMAL );


	//		ExportSingleSynchroFile(SynchroProjectFile);



}




void CTLiteDoc::ExportSynchroVersion6Files()
{
	// to do: store old node number to another file
	std::list<DTANode*>::iterator  iNode;
	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		if((*iNode)->m_NodeOriginalNumber==0)  // overwrite only default value
		{
			(*iNode)->m_NodeOriginalNumber = (*iNode)->m_NodeNumber;
		}

		(*iNode)->m_NodeNumber = (*iNode)->m_NodeID +1;  //reset all node numbers
	}

	GDPoint m_Origin_Current = m_Origin;

	FILE* st = NULL;

	// write lanes/movements file
	const int LaneColumnSize = 32;
	const int LaneRowSize = 30;
	string lane_Column_name_str[LaneColumnSize] = { "NBL2","NBL","NBT","NBR","NBR2","SBL2","SBL","SBT","SBR","SBR2","EBL2","EBL","EBT","EBR","EBR2","WBL2","WBL","WBT","WBR","WBR2","NEL","NET","NER","NWL","NWT","NWR","SEL","SET","SER","SWL","SWT","SWR"};
	string lane_row_name_str[LaneRowSize] = {"Up Node","Dest Node","Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};

	int i,j, m;
	int movement_size = m_MovementVector.size();

	fopen_s(&st,m_Synchro_ProjectDirectory+"Lanes.csv","w");
	if(st!=NULL)
	{		
		fprintf(st, "Lane Group Data \n");
		fprintf(st, "RECORDNAME,INTID,");  // write titles of fields
		for(j=0; j<LaneColumnSize;j++)
			fprintf(st, "%s,", lane_Column_name_str[j].c_str());
		fprintf(st,"\n");

		for (m=0; m<movement_size;m++)
		{
			// write UpNodeID and DestNodeID using original m_NodeNumber
			for(i=0; i<2; i++)
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // current node id

				for(j=0; j<LaneColumnSize;j++)
				{
					int NodeID = (int)(m_MovementVector[m].DataMatrix[i][j].m_text);
					TRACE("Node Label: %d\n",NodeID);

					if(NodeID>=0)  //this movement has been initialized
					{
						if(m_NodeIDMap.find(NodeID) == m_NodeIDMap.end())
						{
							AfxMessageBox("Error in node id!");
							return;
						}
						fprintf(st, "%i,",m_NodeIDMap[NodeID]->m_NodeNumber);  
					}else  // this movement has not been initialized, so the default value is -1
					{
						fprintf(st, ",");  

					}
				}
				fprintf(st,"\n");
			}
			for(i=2; i<LaneRowSize; i++)
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<LaneColumnSize;j++)
				{

					float text = m_MovementVector[m].DataMatrix[i][j].m_text; // default value
					//find movement, overwrite data
					if( lane_row_name_str[i].find("Phase1")!= string::npos || lane_row_name_str[i].find("PermPhase1")!= string::npos ||  lane_row_name_str[i].find("DetectPhase1")!= string::npos )
					{
						int FromNodeID = (int)(m_MovementVector[m].DataMatrix[0][j].m_text);
						int DestNodeID = (int)(m_MovementVector[m].DataMatrix[1][j].m_text);

						int FromNodeNumber = 0;
						int DestNodeNumber = 0;
						int CurrentNodeNumber = m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeOriginalNumber;

						if(FromNodeID > 0)
							FromNodeNumber = m_NodeIDMap[FromNodeID]->m_NodeOriginalNumber ;

						if(DestNodeID > 0)
							DestNodeNumber = m_NodeIDMap[DestNodeID]->m_NodeOriginalNumber;

						CString movement_label;
						movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

						if(FromNodeNumber >0 && m_Movement3NodeMap.find(movement_label) != m_Movement3NodeMap.end())
						{
							if (lane_row_name_str[i].find("Phase1")!= string::npos && m_Movement3NodeMap[movement_label].Phase1 != -1)  // has been set/ initialized
							{
								text = m_Movement3NodeMap[movement_label].Phase1;
							}
							if (lane_row_name_str[i].find("PermPhase1")!= string::npos && m_Movement3NodeMap[movement_label].PermPhase1!= -1)
							{
								text = m_Movement3NodeMap[movement_label].PermPhase1;
							}

							if (lane_row_name_str[i].find("DetectPhase1")!= string::npos && m_Movement3NodeMap[movement_label].DetectPhase1!=-1)
							{
								text = m_Movement3NodeMap[movement_label].DetectPhase1;
							}
						}
					}

					if (text >= 0)
						fprintf(st, "%f,",text);
					else
						fprintf(st, ",");
				}
				fprintf(st,"\n");
			}
		}
		fprintf(st,"\n");

		fclose(st);
	}else
	{
		AfxMessageBox("File Lanes.csv cannot be opened.");
	}

	// write phase file
	const int PhaseColumnSize = 8;
	const int PhaseRowSize = 23;
	string phase_Column_name_str[PhaseColumnSize] = { "D1","D2","D3","D4","D5","D6","D7","D8"};
	string phase_row_name_str[PhaseRowSize] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};

	int p, phase_size = m_PhaseVector.size();

	fopen_s(&st,m_Synchro_ProjectDirectory+"Phasing.csv","w");
	if(st!=NULL)
	{
		fprintf(st, "Phasing Data \n");
		fprintf(st, "RECORDNAME,INTID,");
		for(j=0; j<PhaseColumnSize;j++)
			fprintf(st, "%s,", phase_Column_name_str[j].c_str());
		fprintf(st,"\n");

		for (p=0; p<phase_size;p++)
		{
			for(i=0; i<PhaseRowSize; i++)
			{
				fprintf(st, "%s,", phase_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_NodeNumber);  // intersection id

				if (  m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_bQEM_optimized)
				{
					for(j=0; j<PhaseColumnSize;j++)
					{
						fprintf(st, "%f,", m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_PhaseDataMatrix [i][j]); // use optimized vlaue from QEM
					}
				
				}
				else  // default value
				{
					for(j=0; j<PhaseColumnSize;j++)
					{
						fprintf(st, "%f,",m_PhaseVector[p].DataMatrix[i][j]); //
					}
				}
				fprintf(st,"\n");
			}
		}
		fprintf(st,"\n");

		fclose(st);
	}else
	{
		AfxMessageBox("File Phasing.csv cannot be opened.");
	}

	// write layout file
	DTA_Approach incoming_approach, out_approach;
	GDPoint p1, p2;
	int current_node_id, up_node_id, down_node_id;
	long LinkID;
	const int Dir_size = 8;

	fopen_s(&st,m_Synchro_ProjectDirectory+"Layout.csv","w");
	if(st!=NULL)
	{
		fprintf(st, "Layout Data \n");
		fprintf(st, "INTID,INTNAME,TYPE,X,Y,NID,SID,EID,WID,NEID,NWID,SEID,SWID,NNAME,SNAME,ENAME,WNAME,NENAME,NWNAME,SENAME,SWNAME");
		fprintf(st,"\n");

		// try to set world coordinate so that all coordinates are non-negative

		m_Origin.x  = 1000000;
		m_Origin.y  = 1000000;

		std::list<DTANode*>::iterator  iNode;
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			m_Origin.x = min(m_Origin.x,(*iNode)->pt .x);
			m_Origin.y= min(m_Origin.y,(*iNode)->pt .y);
		}

		int i_n = 0;
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i_n++)
		{  // for current node	

			double resolution = 1/max(0.000001,m_UnitFeet);
			float pt_x = NPtoSP_X((*iNode)->pt,resolution);
			float pt_y = NPtoSP_Y((*iNode)->pt,resolution);

			int control_type = 0;
			if((*iNode)->m_ControlType == m_ControlType_PretimedSignal)
				control_type = 0;  
			else
			{
				if( ((*iNode)->m_IncomingLinkVector.size() > 0 && (*iNode)->m_OutgoingLinkVector.size() ==0) /* only has incoming links*/ 
					||  ((*iNode)->m_IncomingLinkVector.size() == 0 && (*iNode)->m_OutgoingLinkVector.size() > 0) /* only has outgoing links*/ )
				{
					control_type = 1;   // external nodes
				}else
				{
					control_type = 3;   // unsignalized

				}

			}

			if(i_n==350)
				TRACE("");

			fprintf(st, "%i,%i,%i,%f,%f,", (*iNode)->m_NodeNumber, (*iNode)->m_NodeOriginalNumber , control_type,
				pt_x, pt_y);
			// find connecting nodes and links at different directions
			DTA_NodeBasedLinkSets Node_Link;
			current_node_id = i_n;
			Node_Link.initial(current_node_id);
			for(int inbound_i= 0; inbound_i< m_Network.m_InboundSizeAry[i_n]; inbound_i++)
			{		
				LinkID = m_Network.m_InboundLinkAry[i_n][inbound_i];
				up_node_id = m_Network.m_FromIDAry[LinkID];
				p1  = m_NodeIDMap[up_node_id]->pt;
				p2  = m_NodeIDMap[current_node_id]->pt;
				incoming_approach = g_Angle_to_Approach_8_direction(Find_P2P_Angle(p1,p2));
				switch (incoming_approach)
				{ 
				case DTA_North:
					Node_Link.link_flag[1] = true;
					Node_Link.Up_ID[1] = up_node_id;
					Node_Link.Name[1] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_South:
					Node_Link.link_flag[0] = true;
					Node_Link.Up_ID[0] = up_node_id;
					Node_Link.Name[0] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_East:
					Node_Link.link_flag[3] = true;
					Node_Link.Up_ID[3] = up_node_id;
					Node_Link.Name[3] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_West:
					Node_Link.link_flag[2] = true;
					Node_Link.Up_ID[2] = up_node_id;
					Node_Link.Name[2] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_NorthEast:
					Node_Link.link_flag[7] = true;
					Node_Link.Up_ID[7] = up_node_id;
					Node_Link.Name[7] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_NorthWest:
					Node_Link.link_flag[6] = true;
					Node_Link.Up_ID[6] = up_node_id;
					Node_Link.Name[6] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_SouthEast:
					Node_Link.link_flag[5] = true;
					Node_Link.Up_ID[5] = up_node_id;
					Node_Link.Name[5] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_SouthWest:
					Node_Link.link_flag[4] = true;
					Node_Link.Up_ID[4] = up_node_id;
					Node_Link.Name[4] = m_LinkNoMap[LinkID]->m_Name;
					break;
				}
			}
			for(int outbound_i= 0; outbound_i< m_Network.m_OutboundSizeAry[i_n]; outbound_i++)
			{		
				LinkID = m_Network.m_OutboundLinkAry[i_n][outbound_i];
				down_node_id = m_Network.m_ToIDAry[LinkID];
				p1  = m_NodeIDMap[current_node_id]->pt;
				p2  = m_NodeIDMap[down_node_id]->pt;
				out_approach = g_Angle_to_Approach_8_direction(Find_P2P_Angle(p1,p2));
				switch (out_approach)
				{ 
				case DTA_North:
					Node_Link.link_flag[0] = true;
					Node_Link.Up_ID[0] = down_node_id;
					Node_Link.Name[0] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_South:
					Node_Link.link_flag[1] = true;
					Node_Link.Up_ID[1] = down_node_id;
					Node_Link.Name[1] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_East:
					Node_Link.link_flag[2] = true;
					Node_Link.Up_ID[2] = down_node_id;
					Node_Link.Name[2] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_West:
					Node_Link.link_flag[3] = true;
					Node_Link.Up_ID[3] = down_node_id;
					Node_Link.Name[3] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_NorthEast:
					Node_Link.link_flag[4] = true;
					Node_Link.Up_ID[4] = down_node_id;
					Node_Link.Name[4] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_NorthWest:
					Node_Link.link_flag[5] = true;
					Node_Link.Up_ID[5] = down_node_id;
					Node_Link.Name[5] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_SouthEast:
					Node_Link.link_flag[6] = true;
					Node_Link.Up_ID[6] = down_node_id;
					Node_Link.Name[6] = m_LinkNoMap[LinkID]->m_Name;
					break;
				case DTA_SouthWest:
					Node_Link.link_flag[7] = true;
					Node_Link.Up_ID[7] = down_node_id;
					Node_Link.Name[7] = m_LinkNoMap[LinkID]->m_Name;
					break;
				}
			}
			// write into file
			for(i=0; i<Dir_size; i++)
			{
				if (Node_Link.link_flag[i])
					fprintf(st, "%i,", m_NodeIDMap[Node_Link.Up_ID[i]]->m_NodeNumber);
				else
					fprintf(st, ",");
			}
			for(i=0; i<Dir_size; i++)
			{
				if (Node_Link.link_flag[i] && !Node_Link.Name[i].empty() && Node_Link.Name[i] != "(null)")
					fprintf(st, "%s,", Node_Link.Name[i].c_str());
				else
					fprintf(st, ",");
			}
			fprintf(st,"\n");	
		}
		fprintf(st,"\n");

		fclose(st);
	}else
	{

		AfxMessageBox("File Layout.csv cannot be opened.");
	}

	// write timing file
	fopen_s(&st,m_Synchro_ProjectDirectory+"Timing.csv","w");
	if(st!=NULL)
	{
		fprintf(st, "Timing Plans \n");
		fprintf(st, "PLANID,INTID,S1,S2,S3,S4,S5,S6,S7,S8,CL,OFF,LD,REF,CLR \n");
		DTA_TimePlan plan;
		plan.initial();

		for (p=0; p<phase_size;p++)
		{
			fprintf(st, "%i,%i,", plan.PlanID, m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_NodeNumber);
			DTA_Timing timing;
			timing.initial(m_PhaseVector[p], plan.DataAry[TIMING_Cycle_Length]);
		if( m_NodeIDMap[m_PhaseVector[p].CurrentNodeID] ->m_bQEM_optimized == false)
		{
			for(i=0; i<8; i++)
			{
				fprintf(st, "%i,", timing.DataAry[i]);
			}
			fprintf(st, "%i,%i,%i,%s,\n", timing.Cycle, timing.OFF, timing.LD, timing.REF.c_str());
		}else
		{
			for(i=0; i<8; i++)
			{
				fprintf(st, "%i,", m_NodeIDMap[m_PhaseVector[p].CurrentNodeID] -> m_SignalPhaseNo[1+i]);
			}
			fprintf(st, "%i,%i,%i,%i,\n", m_NodeIDMap[m_PhaseVector[p].CurrentNodeID] ->m_SignalCycleLength,0,0,0);
		
		
		}
		}
		fprintf(st,"\n");

		fclose(st);
	}else
	{
		AfxMessageBox("File Timing.csv file cannot be opened.");
	}

	CWaitCursor  wait;

	float hourly_volume_conversion_factor = 60.0f/max(1,m_DemandLoadingEndTimeInMin - m_DemandLoadingStartTimeInMin);


	// write volume file
	fopen_s(&st,m_Synchro_ProjectDirectory+"Volume.csv","w");
	if(st!=NULL)
	{
		fprintf(st, "Turning Movement Count \n");
		fprintf(st, "60 Minute Counts \n");
		fprintf(st, "DATE,TIME,INTID,NBL2,NBL,NBT,NBR,NBR2,SBL2,SBL,SBT,SBR,SBR2,EBL2,EBL,EBT,EBR,EBR2,WBL2,WBL,WBT,WBR,WBR2,NEL,NET,NER,NWL,NWT,NWR,SEL,SET,SER,SWL,SWT,SWR\n");
		for (m=0; m<movement_size;m++)
		{				
			// DATE,TIME,INTID
			int CurNodeNumber  =  m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeID +1;
			fprintf(st, "07/20/2011,1700,%i,",CurNodeNumber);
			// 
			for(j=0; j<LaneColumnSize;j++)
			{

				int FromNodeID = (int)(m_MovementVector[m].DataMatrix[0][j].m_text);
				int DestNodeID = (int)(m_MovementVector[m].DataMatrix[1][j].m_text);

				int FromNodeNumber = 0;
				int DestNodeNumber = 0;
				int CurrentNodeNumber = m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeOriginalNumber;

				if(FromNodeID > 0)
					FromNodeNumber = m_NodeIDMap[FromNodeID]->m_NodeOriginalNumber;

				if(DestNodeID > 0)
					DestNodeNumber = m_NodeIDMap[DestNodeID]->m_NodeOriginalNumber;

				CString movement_label;
				movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

				int count = 0;

				if(m_Movement3NodeMap.find(movement_label) != m_Movement3NodeMap.end())
				{
					count = m_Movement3NodeMap[movement_label].TotalVehicleSize* hourly_volume_conversion_factor;

				}

				fprintf(st, "%i,",count);
			}
			fprintf(st, "\n");


		}

		fprintf(st,"\n");

		fclose(st);

	}else
	{
		AfxMessageBox("File Volume.csv file cannot be opened.");
	}


	m_Origin = m_Origin_Current;  // restore value


}

void CTLiteDoc::ExportSingleSynchroFile(CString SynchroProjectFile)
{ 

}





bool CTLiteDoc::ReadSynchroPreGeneratedLayoutFile(LPCTSTR lpszFileName)
{
	const int approach_size = 8;

	string approach_column_name_str[approach_size] = { "NID","SID",	"EID","WID","NEID","NWID","SEID","SWID"};

	// we need to use the opposite direction
	DTA_Approach approach_vector[approach_size] = { 
		DTA_South,
		DTA_North,
		DTA_West,
		DTA_East,
		DTA_SouthWest,
		DTA_SouthEast,
		DTA_NorthWest,
		DTA_NorthEast};

		int approach_node_id[approach_size];

		m_PredefinedApproachMap.clear();

		CCSVParser parser;
		parser.m_bSkipFirstLine  = true;  // skip the first line  
		if (parser.OpenCSVFile(lpszFileName))
		{
			int i=0;
			while(parser.ReadRecord())
			{
				int node_id;
				string name;
				DTANode* pNode = 0;


				if(parser.GetValueByFieldName("INTID",node_id) == false)
					break;

				for(int a = 0; a< approach_size; a++)
				{	
					approach_node_id[a]=0;  // initialization
					bool bField_Exist = parser.GetValueByFieldName(approach_column_name_str[a],approach_node_id[a]);

					if(bField_Exist && approach_node_id[a]>0)
					{
						TRACE("node = %d,%d,%s\n", node_id, approach_node_id[a], approach_column_name_str[a].c_str ());

						CString str_key;
						str_key.Format("%d,%d",node_id, approach_node_id[a]);

						m_PredefinedApproachMap[str_key] = approach_vector[a];

					}

				}


			}

			AfxMessageBox("Synchro_layout.csv file is used to specify the movement vector in QEM and Synchro exporting functions.", MB_ICONINFORMATION);
			return true;
		}
		return false;
}


void CTLiteDoc::ExportQEMData(int ThisNodeNumber)
{
	CString directory;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);



	#ifndef _WIN64

// kill Excel process first


	CProcessManager kill_process;


		if(m_bMovementAvailableFlag==false) // first time to detect EXCEL
		{
			   DWORD dwId;
				HANDLE hProcess = kill_process.FindProcess("EXCEL.exe", 	dwId);
				if(hProcess)
				{
				 AfxMessageBox("Please close all Excel applications before use the QEM method.",MB_ICONINFORMATION);
				}

		}

		CWaitCursor wait;

	bool b_Excel_found_flag = false;
			   DWORD dwId;
				HANDLE hProcess = kill_process.FindProcess("EXCEL.exe", 	dwId);
				if(hProcess)
				{
					b_Excel_found_flag = true;
				}

		if(b_Excel_found_flag == true)
		{
		while(kill_process.KillProcess("EXCEL.exe")==true);
		}

	string phase_row_name_str[23] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};

	m_Network.Initialize (m_NodeSet.size(), m_LinkSet.size(), 1, m_AdjLinkSize);
	m_Network.BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, false);

	float hourly_volume_conversion_factor = 	60.0f/max(1,m_DemandLoadingEndTimeInMin - m_DemandLoadingStartTimeInMin);
	
	ConstructMovementVector(true);

	CXLEzAutomation XL;

	CString QEM2_Excel_File;
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();


	if(ThisNodeNumber==0)
	{  // batch mode
	QEM2_Excel_File.Format ("%s\\QEM_SIG.xls",pMainFrame->m_CurrentDirectory);
	}else
	{
	QEM2_Excel_File.Format ("%s\\QEM_SIG.xls",pMainFrame->m_CurrentDirectory);

	//	QEM2_Excel_File.Format ("%sQEM_SIG.xls",pMainFrame->m_CurrentDirectory);

	//CString QEM_Node_File;
	//QEM_Node_File.Format("%sQEM_SIG_NODE_%d.xls",directory,ThisNodeNumber);

	//CopyFile( QEM2_Excel_File, QEM_Node_File, TRUE);

	//QEM2_Excel_File = QEM_Node_File;  // replace

	}
	
	
	if(XL.OpenExcelFile(QEM2_Excel_File)==false)
	{
	CString message; 
	message.Format ("File %s cannot be found.",QEM2_Excel_File);
	AfxMessageBox(message);
	}

	CString QEM2_CSV_LOG_File;
	QEM2_CSV_LOG_File.Format ("%s\\QEM_log.csv",pMainFrame->m_CurrentDirectory);

	FILE* st = NULL;
	fopen_s(&st,QEM2_CSV_LOG_File,"w");
	if(st==NULL)
	{

		AfxMessageBox("File QEM.csv cannot be opened. Please check if the file is locked by Excel.");
		return;
	}



	const int LaneColumnSize = 32;
	const int LaneRowSize = 30;
	int volume_row_number = 4;
	string lane_Column_name_str[LaneColumnSize] = { "NBL2","NBL","NBT","NBR","NBR2","SBL2","SBL","SBT","SBR","SBR2","EBL2","EBL","EBT","EBR","EBR2","WBL2","WBL","WBT","WBR","WBR2","NEL","NET","NER","NWL","NWT","NWR","SEL","SET","SER","SWL","SWT","SWR"};		
	string lane_row_name_str[LaneRowSize] = {"Up Node","Dest Node","Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};

	int QEM_node_count = 0;


	int movement_size = m_MovementVector.size();

	for (int m=0; m<movement_size;m++)
	{
		int node_number = m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber;

		if(ThisNodeNumber>=1 && node_number != ThisNodeNumber)
		{
		// show data for current node number only
		continue;
		}

		int node_id = m_NodeNametoIDMap[node_number];

		if(m_NodeIDMap[node_id]->m_ControlType ==  m_ControlType_PretimedSignal || m_NodeIDMap[node_id]->m_ControlType ==  m_ControlType_ActuatedSignal )  //this movement vector is the same as the current node
		{
			QEM_node_count++;
			// stage 1: write UpNodeID and DestNodeID using original m_NodeNumber

			// current node number
				for(int k = 2; k<38; k++)
				{
					XL.SetCellValue(2,k,m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // street names
				}

			int i,j;

			std::vector<CString> movement_label_vector;
			// log csv file title
			fprintf(st, "Lane Group Data \n");
			fprintf(st, "RECORDNAME,INTID,");  // write titles of fields
			for(j=0; j<LaneColumnSize;j++)
				fprintf(st, "%s,", lane_Column_name_str[j].c_str());
			fprintf(st,"\n");

			for(i=0; i<2; i++)  // upstream node, destination node
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // current node id


				for(j=0; j<LaneColumnSize;j++)
				{
					int UpstreamNodeID = (int)(m_MovementVector[m].DataMatrix[0][j].m_text);

					if(UpstreamNodeID >=0)
					{

						int NodeID = (int)(m_MovementVector[m].DataMatrix[i][j].m_text);
						int NodeNumber =0;

						if(NodeID>=0)  //this movement has been initialized
						{
							NodeNumber = m_NodeIDMap[NodeID]->m_NodeNumber;
						}
						XL.SetCellValue(3+j,2+i,NodeNumber);  // input from node value to QEM spreadsheet
						
						if(i==0)  // upstream node only
						{
						XL.SetCellValue(3+j,38,NodeNumber);  // street names
						}

						fprintf(st, "%i,",m_NodeIDMap[NodeID]->m_NodeNumber);  

					}else
					{
						int NodeNumber =0;
						XL.SetCellValue(3+j,2+i,NodeNumber); // input to node value to QEM spreadsheet
						fprintf(st, "%i,",NodeNumber);  

					}
				}
				fprintf(st,"\n");
			}

			///	TurnVolume			
			fprintf(st, "TurnVolume,");
			fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // current node id

			for(j=0; j<LaneColumnSize;j++)
			{

				int FromNodeID = (int)(m_MovementVector[m].DataMatrix[0][j].m_text);
				int DestNodeID = (int)(m_MovementVector[m].DataMatrix[1][j].m_text);

				int FromNodeNumber = 0;
				int DestNodeNumber = 0;
				int CurrentNodeNumber = m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber;

				if(FromNodeID > 0)
					FromNodeNumber = m_NodeIDMap[FromNodeID]->m_NodeNumber ;

				if(DestNodeID > 0)
					DestNodeNumber = m_NodeIDMap[DestNodeID]->m_NodeNumber;

				CString movement_label;
				movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

				if(FromNodeNumber== 2085 && CurrentNodeNumber==4611 && DestNodeNumber==4608)
				{
					TRACE("");
				}
				movement_label_vector.push_back(movement_label);

				int count = 0;


				if(m_Movement3NodeMap.find(movement_label) != m_Movement3NodeMap.end())
				{
					count = m_Movement3NodeMap[movement_label].TotalVehicleSize*hourly_volume_conversion_factor;
					XL.SetCellValue(3+j,volume_row_number,count); // input turning movement volume value to QEM spreadsheet
					fprintf(st, "%i,",count);
					TRACE("count = %d\n",count);

				}else
				{
					XL.SetCellValue(3+j,volume_row_number,0);								
					fprintf(st, "0,");
				}

				// if upstream node id >=0
			} // for each column/movement


			fprintf(st,"\n");

			//// # of lanes, shared lanes... 
			// starting row number 5, 
			for(i=2; i<9; i++)
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<LaneColumnSize;j++)
				{
					int text = (int)(m_MovementVector[m].DataMatrix[i][j].m_text);

						int FromNodeID = (int)(m_MovementVector[m].DataMatrix[0][j].m_text);
						int FromNodeNumber = 0;
						if(FromNodeID > 0)
							FromNodeNumber = m_NodeIDMap[FromNodeID]->m_NodeNumber ;

						int CurrentNodeNumber = m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber;
						int DestNodeID = (int)(m_MovementVector[m].DataMatrix[1][j].m_text);

							int DestNodeNumber = 0;
							if(DestNodeID > 0)
								DestNodeNumber = m_NodeIDMap[DestNodeID]->m_NodeNumber;

							CString movement_label;
							movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

							DTANodeMovement* pMovement = NULL;

							if(m_MovementPointerMap.find(movement_label) != m_MovementPointerMap.end())
							{
								DTANodeMovement* pMovement = m_MovementPointerMap[movement_label];
							}


						if(FromNodeNumber>0)  // with movement
						{
						DTALink* pLink = FindLinkWithNodeNumbers(FromNodeNumber,node_number);
						if(pLink!=NULL)
						{
						if(i==2) // lanes
							{

								if(lane_Column_name_str[j].find("T")!= string::npos ) // through
									text = pLink->m_NumLanes;
								else
								{
									text = 1; // left and right: to do list: input left-turn and right turn data fr # of lanes

									if(pMovement!=NULL)
										text = pMovement->QEM_Lanes ;

								}

							}

						if(i==8) // speed
							{
								text = pLink->m_SpeedLimit ;

							}

					if(pMovement!=NULL)
					{

						switch (i)
						{
						case 3: text = pMovement->QEM_Shared; break; //  // "Shared",
						case 4: text = pMovement->QEM_Width ; break; //  // "Width",
						case 5: text = pMovement->QEM_Storage ; break; //  // "Storage",
						case 6: text = pMovement->QEM_StLanes; break; //  // "StLanes",
						case 7: text = pMovement->QEM_Grade; break; //  // "",
						case 8: 
							text = pMovement->QEM_Speed;// "speed",
							
							break; //  
						
						}
					}

						}
		
						
						}
						// we have to consider some special attributes here
		
					if (text >= 0)
					{
						XL.SetCellValue(3+j,3+i,text);
						fprintf(st, "%d,",text);
					}
					else
					{
						XL.SetCellValue(3+j,3+i,0);
						fprintf(st, "0,");

					}

				}
				fprintf(st,"\n");
			}
			// movement volume

			//stage 2: fetch data from QEM
			for(j=0; j<LaneColumnSize;j++)
			{
				int FromNodeID = (int)(m_MovementVector[m].DataMatrix[0][j].m_text);
				int DestNodeID = (int)(m_MovementVector[m].DataMatrix[1][j].m_text);

				int FromNodeNumber = 0;
				int DestNodeNumber = 0;
				int CurrentNodeNumber = m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber;

				if(FromNodeID > 0)
					FromNodeNumber = m_NodeIDMap[FromNodeID]->m_NodeNumber ;

				if(DestNodeID > 0)
					DestNodeNumber = m_NodeIDMap[DestNodeID]->m_NodeNumber;

				CString movement_label;
				movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

				if(FromNodeNumber >0)
				{
					CString szValue;
					szValue = XL.GetCellValue(3+j, 14);  // read Phase1 data from QEM
					m_Movement3NodeMap[movement_label].Phase1 = atoi(szValue);

					szValue = XL.GetCellValue(3+j, 15); // read PermPhase1 data from QEM
					m_Movement3NodeMap[movement_label].PermPhase1  = atoi(szValue);

					szValue = XL.GetCellValue(3+j, 16); // read DetectPhase1 data from QEM
					m_Movement3NodeMap[movement_label].DetectPhase1   = atoi(szValue);

							DTANodeMovement* pMovement = NULL;

							if(m_MovementPointerMap.find(movement_label) != m_MovementPointerMap.end())
							{
								DTANodeMovement* pMovement = m_MovementPointerMap[movement_label];
								if(pMovement !=NULL)
								{
								pMovement->QEM_Phase1  = m_Movement3NodeMap[movement_label].Phase1;
								pMovement->QEM_PermPhase1  = m_Movement3NodeMap[movement_label].PermPhase1;
								pMovement->QEM_DetectPhase1  = m_Movement3NodeMap[movement_label].DetectPhase1;

								if(CurrentNodeNumber == 94) 
									TRACE("");

								pMovement->QEM_EffectiveGreen  = atoi(XL.GetCellValue(3+j, 17)); // EffectiveGreen 
								pMovement->QEM_Capacity  = atoi(XL.GetCellValue(3+j, 18)); // Capacity 
								pMovement->QEM_VOC   = atoi(XL.GetCellValue(3+j, 19)); // V/C 
								pMovement->QEM_Delay  = atoi(XL.GetCellValue(3+j, 20)); // Delay 
								pMovement->QEM_LOS = XL.GetCellValue(3+j, 21); // LOS 
								pMovement->QEM_SatFlow   = atoi(XL.GetCellValue(3+j, 39)); // DischargeRate 

								}
							}


					TRACE("node %d, movement %s: phase %d, perm_phase %d, detect_phase %d\n",CurrentNodeNumber, movement_label,m_Movement3NodeMap[movement_label].Phase1, m_Movement3NodeMap[movement_label].PermPhase1 , m_Movement3NodeMap[movement_label].DetectPhase1 );
				}
			}


			CString szValue;
			for(int col = 0; col < 8; col++)  //8 phases
			{
			szValue = XL.GetCellValue(2+col, 41);  //   read timing data from QEM
			m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_SignalPhaseNo[1+col]  = atoi(szValue);

				for(int phase_attribute_no = 0;  phase_attribute_no< 23; phase_attribute_no++)
				{
					szValue = XL.GetCellValue(3+col, 43+phase_attribute_no);  //   read timing data from QEM
					m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_PhaseDataMatrix[phase_attribute_no][col] =  atof(szValue);

				}

			}

			szValue = XL.GetCellValue(10, 41);
			m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_SignalCycleLength  = atoi(szValue);

			m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_bQEM_optimized = true;

			//log message
			for(int i=0; i<9; i++) // 9 lines
			{

				switch (i)
				{
				case 0: fprintf(st, "Phase1,"); break;
				case 1: fprintf(st, "PermPhase1,"); break;
				case 2: fprintf(st, "DetectPhase1,"); break;
				case 3: fprintf(st, "EffectiveGreen,"); break;
				case 4: fprintf(st, "Capacity ,"); break;
				case 5: fprintf(st, "V/C,"); break;
				case 6: fprintf(st, "Delay,"); break;
				case 7: fprintf(st, "LOS,"); break;
				case 8: fprintf(st, "DischargeRate,"); break;
				}

				if(i<=7)
				{
					fprintf(st, ",");
				}

				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // current node id

				for(j=0; j<LaneColumnSize;j++)
				{
					int FromNodeID = (int)(m_MovementVector[m].DataMatrix[0][j].m_text);
					int DestNodeID = (int)(m_MovementVector[m].DataMatrix[1][j].m_text);

					int FromNodeNumber = 0;
					int DestNodeNumber = 0;
					int CurrentNodeNumber = m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber;

					if(FromNodeID > 0)
						FromNodeNumber = m_NodeIDMap[FromNodeID]->m_NodeNumber ;

					if(DestNodeID > 0)
						DestNodeNumber = m_NodeIDMap[DestNodeID]->m_NodeNumber;

					CString movement_label;
					movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

					if(FromNodeNumber >0)
					{
					DTANodeMovement* pMovement = m_MovementPointerMap[movement_label];

					if(CurrentNodeNumber == 94)
						TRACE("");

					if(pMovement!= NULL)
					{
					switch (i)
					{
					case 0: fprintf(st, "%d,",m_Movement3NodeMap[movement_label].Phase1); break;
					case 1: fprintf(st, "%d,",m_Movement3NodeMap[movement_label].PermPhase1 ); break;
					case 2: fprintf(st, "%d,",m_Movement3NodeMap[movement_label].Phase1 ); break;
					case 3: fprintf(st, "%.0f,",pMovement->QEM_EffectiveGreen ); break;
					case 4: fprintf(st, "%.2f,",pMovement->QEM_VOC ); break;
					case 5: fprintf(st, "%.1f,",pMovement->QEM_Delay ); break;
					case 6: fprintf(st, "%s,", pMovement->QEM_LOS ); break;
					case 7: fprintf(st, "%.0f,",pMovement->QEM_SatFlow ); break;
					}
					}else
					{
						fprintf(st, ",");
					
					}

					}else
					{
						fprintf(st, ",");

					}
				}

				fprintf(st,"\n");
			}

			fprintf(st,"INTID,");

			int col;

			for(col = 0; col < 8; col++)
			{
			CString str;
			str.Format ("S%d,",col+1);
			fprintf(st,str);
			}

			fprintf(st,"CL,OFF,LD,REF,\n");

			fprintf(st,",");

			for(int col = 0; col < 8; col++)
			{
			fprintf(st,"%d,",m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_SignalPhaseNo[1+col]);
			}

			fprintf(st,"%d,",m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_SignalCycleLength);

			fprintf(st,"\n");

			// output phasing data logs

			for(int row = 0; row < 23; row++)
			{
				fprintf(st,"%s,",phase_row_name_str[row].c_str() ); // phase attribute label
				for (int col = 0; col < 8; col++)
				{ 
					fprintf(st,"%f,",m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_PhaseDataMatrix[row][col]);
				}
			fprintf(st,"\n");
			}
	
			// end of log message

		}  // for each movement vector record
	}

	//Close Excel when done (or leave it open by deleting the
	//next line):
	//Before closing server in your application disable alerts by calling:

	if(ThisNodeNumber ==0)
	{
		XL.ReleaseExcel();
	}

	//if(ThisNodeNumber >0)
	//{
	//XL.SaveFile();
	//}


	if(st!=NULL)
		fclose(st);
	// 


	fopen_s(&st,directory+"ouput_movement_phasing.csv","w");
	if(st!=NULL)
	{
		fprintf(st,"movement_index,three-node key,count\n");

		int movement_index = 1;
		std::map<CString, Movement3Node> ::const_iterator itr;
		for (itr = m_Movement3NodeMap.begin(); itr != m_Movement3NodeMap.end(); itr++)
		{
			fprintf(st, "%d,%s,%d,phase,%d,perm_phase,%d,detect_phase,%d\n", 
				movement_index++,
				(*itr).first, 
				(*itr).second.TotalVehicleSize,
				(*itr).second.Phase1, (*itr).second.PermPhase1 , (*itr).second.DetectPhase1

				);

		}
		fclose(st);
	}

	if(ThisNodeNumber<=0)
	{
	CString msg;
	msg.Format ("Excel Automation Done!\n%d signalized nodes now have estimated movement capacity and signal timing plans.",QEM_node_count);
	AfxMessageBox(msg, MB_ICONINFORMATION);
	}

#else if
	CString msg;
	msg.Format ("Excel automation is not available under the 64-bit version yet.");
	AfxMessageBox(msg, MB_ICONINFORMATION);

#endif

  }
