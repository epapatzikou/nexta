// Dlg_SignalDataExchange.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_SignalDataExchange.h"
#include "Data-Interface\\XLEzAutomation.h"
#include "Data-Interface\\XLTestDataSource.h"
#include "MainFrm.h"
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
        if(!hProcess)  // did not find the process
            return true;  // no need to kill the process, good, we are ok now

        // TerminateAppEnum() posts WM_CLOSE to all windows whose PID
        // matches your process's.
        ::EnumWindows((WNDENUMPROC)
            CProcessManager::TerminateAppEnum, 
            (LPARAM) dwId);
        // Wait on the handle. If it signals, great. 
        //If it times out, then you kill it.
        if(WaitForSingleObject(hProcess, 5000)
            !=WAIT_OBJECT_0)
            bResult = TerminateProcess(hProcess,0);  // return non-zero as true
        else
            bResult = FALSE;   // we do not have handle to kill the process
        CloseHandle(hProcess);
        return bResult;  // true: succeed, false: failed
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
// CDlg_SignalDataExchange dialog

IMPLEMENT_DYNAMIC(CDlg_SignalDataExchange, CDialog)

CDlg_SignalDataExchange::CDlg_SignalDataExchange(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_SignalDataExchange::IDD, pParent)
{

}

CDlg_SignalDataExchange::~CDlg_SignalDataExchange()
{
}

void CDlg_SignalDataExchange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_INFO, m_InfoList);
}


BEGIN_MESSAGE_MAP(CDlg_SignalDataExchange, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_DATA, &CDlg_SignalDataExchange::OnBnClickedButtonGenerateData)
	ON_BN_CLICKED(IDC_BUTTON_QEM, &CDlg_SignalDataExchange::OnBnClickedButtonQem)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_VISSIM_DATA, &CDlg_SignalDataExchange::OnBnClickedButtonGenerateVissimData)
	ON_BN_CLICKED(IDOK, &CDlg_SignalDataExchange::OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_LIST_INFO, &CDlg_SignalDataExchange::OnLbnSelchangeListInfo)
END_MESSAGE_MAP()


// CDlg_SignalDataExchange message handlers

void CDlg_SignalDataExchange::OnBnClickedButtonGenerateData()
{
	CWaitCursor wait;

   m_pDoc->GenerateMovementCountFromVehicleFile();

   m_pDoc->Constructandexportsignaldata ();
}

void CDlg_SignalDataExchange::OnBnClickedButtonQem()
{ 

    m_pDoc->GenerateMovementCountFromVehicleFile();
	m_InfoList.ResetContent ();

   	CString directory;
	directory = m_pDoc->m_ProjectFile.Left( m_pDoc->m_ProjectFile.ReverseFind('\\') + 1);



#ifndef _WIN64

 ////kill Excel process first

	//CWaitCursor wait;

	//CProcessManager kill_process;


	//{
	//		   DWORD dwId;
	//			HANDLE hProcess = kill_process.FindProcess("EXCEL.exe", 	dwId);
	//			if(hProcess)
	//			{
	//			 AfxMessageBox("Please close all Excel applications before use the QEM method.",MB_ICONINFORMATION);
	//			}

	//	}


	//bool b_Excel_found_flag = false;
	//		   DWORD dwId;
	//			HANDLE hProcess = kill_process.FindProcess("EXCEL.exe", 	dwId);
	//			if(hProcess)
	//			{
	//				b_Excel_found_flag = true;
	//			}

	//	if(b_Excel_found_flag == true)
	//	{
	//		for(int try_i = 0; try_i < 5; try_i++)
	//		{
	//				if(kill_process.KillProcess("EXCEL.exe") == true) // succeed
	//					break;
	//		}
	//	}




	string phase_row_name_str[23] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};

	m_pDoc->m_Network.Initialize ( m_pDoc->m_NodeSet.size(),  m_pDoc->m_LinkSet.size(), 1,  m_pDoc->m_AdjLinkSize);
	m_pDoc->m_Network.BuildPhysicalNetwork(&( m_pDoc->m_NodeSet), &( m_pDoc->m_LinkSet),  m_pDoc->m_RandomRoutingCoefficient, false);

	m_InfoList.AddString ("Complete building the network...");
	float hourly_volume_conversion_factor = 	60.0f/max(1,m_pDoc->m_DemandLoadingEndTimeInMin - m_pDoc->m_DemandLoadingStartTimeInMin);
	
	m_InfoList.AddString ("Construct movement vector...");
	m_pDoc->ConstructMovementVector(true);

	m_InfoList.AddString ("Open Excel File...");
	
	if(AfxMessageBox("Please make sure that there is no Excel.exe in the Process Manager.\nThen press Yes to start running the QEM module.", MB_YESNO|MB_ICONINFORMATION)==IDNO)
		return;

	CXLEzAutomation XL;

	CString QEM2_Excel_File;
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	m_InfoList.AddString ("Construct Excel Automation...");


	QEM2_Excel_File.Format ("%s\\QEM_SIG.xls",pMainFrame->m_CurrentDirectory);

	//	QEM2_Excel_File.Format ("%sQEM_SIG.xls",pMainFrame->m_CurrentDirectory);

	//CString QEM_Node_File;
	//QEM_Node_File.Format("%sQEM_SIG_NODE_%d.xls",directory,ThisNodeNumber);

	//CopyFile( QEM2_Excel_File, QEM_Node_File, TRUE);

	//QEM2_Excel_File = QEM_Node_File;  // replace

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

	CString str;


	const int LaneColumnSize = 32;
	const int LaneRowSize = 30;
	int volume_row_number = 4;
	string lane_Column_name_str[LaneColumnSize] = { "NBL2","NBL","NBT","NBR","NBR2","SBL2","SBL","SBT","SBR","SBR2","EBL2","EBL","EBT","EBR","EBR2","WBL2","WBL","WBT","WBR","WBR2","NEL","NET","NER","NWL","NWT","NWR","SEL","SET","SER","SWL","SWT","SWR"};		
	string lane_row_name_str[LaneRowSize] = {"Up Node","Dest Node","Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};

	int QEM_node_count = 0;

	bool bSetCellValue = true;

	int movement_size = m_pDoc->m_MovementVector.size();

	for (int m=0; m<movement_size;m++)
	{
		int node_number = m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber;

		int node_id = m_pDoc->m_NodeNumbertoIDMap[node_number];


		str.Format ("Processing node %d", node_number);
		m_InfoList.AddString (str);


		if(m_pDoc->m_NodeIDMap[node_id]->m_ControlType ==  m_pDoc->m_ControlType_PretimedSignal ||
			m_pDoc->m_NodeIDMap[node_id]->m_ControlType ==  m_pDoc->m_ControlType_ActuatedSignal )  //this movement vector is the same as the current node
		{
			QEM_node_count++;
			// stage 1: write UpNodeID and DestNodeID using original m_NodeNumber

			// current node number
				for(int k = 2; k<38; k++)
				{
					if(bSetCellValue)
					{
					m_InfoList.AddString ("Set Cell Value in Excel");

					}
					XL.SetCellValue(2,k,m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // street names
				
					if(bSetCellValue)
					{
					m_InfoList.AddString ("Set Cell Value Done");
					bSetCellValue = false;
					}

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
				fprintf(st, "%i,", m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // current node id


				for(j=0; j<LaneColumnSize;j++)
				{
					int UpstreamNodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[0][j].m_text);

					if(UpstreamNodeID >=0)
					{

						int NodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[i][j].m_text);
						int NodeNumber =0;

						if(NodeID>=0)  //this movement has been initialized
						{
							NodeNumber = m_pDoc->m_NodeIDMap[NodeID]->m_NodeNumber;
						}
						XL.SetCellValue(3+j,2+i,NodeNumber);  // input from node value to QEM spreadsheet
						
						if(i==0)  // upstream node only
						{
						XL.SetCellValue(3+j,38,NodeNumber);  // street names
						}

						fprintf(st, "%i,",m_pDoc->m_NodeIDMap[NodeID]->m_NodeNumber);  

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
			fprintf(st, "%i,", m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // current node id

			for(j=0; j<LaneColumnSize;j++)
			{

				int FromNodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[0][j].m_text);
				int DestNodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[1][j].m_text);

				int FromNodeNumber = 0;
				int DestNodeNumber = 0;
				int CurrentNodeNumber = m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber;

				if(FromNodeID > 0)
					FromNodeNumber = m_pDoc->m_NodeIDMap[FromNodeID]->m_NodeNumber ;

				if(DestNodeID > 0)
					DestNodeNumber = m_pDoc->m_NodeIDMap[DestNodeID]->m_NodeNumber;

				CString movement_label;
				movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

				if(FromNodeNumber== 2085 && CurrentNodeNumber==4611 && DestNodeNumber==4608)
				{
					TRACE("");
				}
				movement_label_vector.push_back(movement_label);

				int count = 0;


				if(m_pDoc->m_Movement3NodeMap.find(movement_label) != m_pDoc->m_Movement3NodeMap.end())
				{
					count = m_pDoc->m_Movement3NodeMap[movement_label].TotalVehicleSize*hourly_volume_conversion_factor;
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
				fprintf(st, "%i,", m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<LaneColumnSize;j++)
				{
					int text = (int)(m_pDoc->m_MovementVector[m].DataMatrix[i][j].m_text);

						int FromNodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[0][j].m_text);
						int FromNodeNumber = 0;
						if(FromNodeID > 0)
							FromNodeNumber = m_pDoc->m_NodeIDMap[FromNodeID]->m_NodeNumber ;

						int CurrentNodeNumber = m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber;
						int DestNodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[1][j].m_text);

							int DestNodeNumber = 0;
							if(DestNodeID > 0)
								DestNodeNumber =m_pDoc->m_NodeIDMap[DestNodeID]->m_NodeNumber;

							CString movement_label;
							movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

							DTANodeMovement* pMovement = NULL;

							if(m_pDoc->m_MovementPointerMap.find(movement_label) != m_pDoc->m_MovementPointerMap.end())
							{
								DTANodeMovement* pMovement = m_pDoc->m_MovementPointerMap[movement_label];
							}


						if(FromNodeNumber>0)  // with movement
						{
						DTALink* pLink = m_pDoc->FindLinkWithNodeNumbers(FromNodeNumber,node_number);
						if(pLink!=NULL)
						{
						if(i==2) // lanes
							{

								if(lane_Column_name_str[j].find("T")!= string::npos ) // through
									text = pLink->m_NumberOfLanes;
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

			//stage 2: 

		m_InfoList.AddString ("fetch data from QEM");

			for(j=0; j<LaneColumnSize;j++)
			{
				int FromNodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[0][j].m_text);
				int DestNodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[1][j].m_text);

				int FromNodeNumber = 0;
				int DestNodeNumber = 0;
				int CurrentNodeNumber = m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber;

				if(FromNodeID > 0)
					FromNodeNumber = m_pDoc->m_NodeIDMap[FromNodeID]->m_NodeNumber ;

				if(DestNodeID > 0)
					DestNodeNumber = m_pDoc->m_NodeIDMap[DestNodeID]->m_NodeNumber;

				CString movement_label;
				movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

				if(FromNodeNumber >0)
				{
					CString szValue;
					szValue = XL.GetCellValue(3+j, 14);  // read Phase1 data from QEM
					m_pDoc->m_Movement3NodeMap[movement_label].Phase1 = atoi(szValue);

					szValue = XL.GetCellValue(3+j, 15); // read PermPhase1 data from QEM
					m_pDoc->m_Movement3NodeMap[movement_label].PermPhase1  = atoi(szValue);

					szValue = XL.GetCellValue(3+j, 16); // read DetectPhase1 data from QEM
					m_pDoc->m_Movement3NodeMap[movement_label].DetectPhase1   = atoi(szValue);

							DTANodeMovement* pMovement = NULL;

							if(m_pDoc->m_MovementPointerMap.find(movement_label) != m_pDoc->m_MovementPointerMap.end())
							{
								DTANodeMovement* pMovement = m_pDoc->m_MovementPointerMap[movement_label];
								if(pMovement !=NULL)
								{
								pMovement->QEM_Phase1  = m_pDoc->m_Movement3NodeMap[movement_label].Phase1;
								pMovement->QEM_PermPhase1  = m_pDoc->m_Movement3NodeMap[movement_label].PermPhase1;
								pMovement->QEM_DetectPhase1  = m_pDoc->m_Movement3NodeMap[movement_label].DetectPhase1;

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


					TRACE("node %d, movement %s: phase %d, perm_phase %d, detect_phase %d\n",CurrentNodeNumber, movement_label,m_pDoc->m_Movement3NodeMap[movement_label].Phase1, m_pDoc->m_Movement3NodeMap[movement_label].PermPhase1 , m_pDoc->m_Movement3NodeMap[movement_label].DetectPhase1 );
				}
			}


			CString szValue;
			for(int col = 0; col < 8; col++)  //8 phases
			{
			szValue = XL.GetCellValue(2+col, 41);  //   read timing data from QEM
			m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_SignalPhaseNo[1+col]  = atoi(szValue);

				for(int phase_attribute_no = 0;  phase_attribute_no< 23; phase_attribute_no++)
				{
					szValue = XL.GetCellValue(3+col, 43+phase_attribute_no);  //   read timing data from QEM
					m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_PhaseDataMatrix[phase_attribute_no][col] =  atof(szValue);

				}

			}

			szValue = XL.GetCellValue(10, 41);
			m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_SignalCycleLength  = atoi(szValue);

			m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_bQEM_optimized = true;

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

				fprintf(st, "%i,",m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // current node id

				for(j=0; j<LaneColumnSize;j++)
				{
					int FromNodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[0][j].m_text);
					int DestNodeID = (int)(m_pDoc->m_MovementVector[m].DataMatrix[1][j].m_text);

					int FromNodeNumber = 0;
					int DestNodeNumber = 0;
					int CurrentNodeNumber =m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_NodeNumber;

					if(FromNodeID > 0)
						FromNodeNumber =m_pDoc->m_NodeIDMap[FromNodeID]->m_NodeNumber ;

					if(DestNodeID > 0)
						DestNodeNumber =m_pDoc->m_NodeIDMap[DestNodeID]->m_NodeNumber;

					CString movement_label;
					movement_label.Format ("%d;%d;%d",FromNodeNumber,CurrentNodeNumber,DestNodeNumber);

					if(FromNodeNumber >0)
					{
					DTANodeMovement* pMovement = m_pDoc->m_MovementPointerMap[movement_label];

					if(CurrentNodeNumber == 94)
						TRACE("");

					if(pMovement!= NULL)
					{
					switch (i)
					{
					case 0: fprintf(st, "%d,",m_pDoc->m_Movement3NodeMap[movement_label].Phase1); break;
					case 1: fprintf(st, "%d,",m_pDoc->m_Movement3NodeMap[movement_label].PermPhase1 ); break;
					case 2: fprintf(st, "%d,",m_pDoc->m_Movement3NodeMap[movement_label].Phase1 ); break;
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
			fprintf(st,"%d,",m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_SignalPhaseNo[1+col]);
			}

			fprintf(st,"%d,",m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_SignalCycleLength);

			fprintf(st,"\n");

			// output phasing data logs

			for(int row = 0; row < 23; row++)
			{
				fprintf(st,"%s,",phase_row_name_str[row].c_str() ); // phase attribute label
				for (int col = 0; col < 8; col++)
				{ 
					fprintf(st,"%f,",m_pDoc->m_NodeIDMap[m_pDoc->m_MovementVector[m].CurrentNodeID]->m_PhaseDataMatrix[row][col]);
				}
			fprintf(st,"\n");
			}
	
			// end of log message

		}  // for each movement vector record
	}

	//Close Excel when done (or leave it open by deleting the
	//next line):
	//Before closing server in your application disable alerts by calling:

		m_InfoList.AddString ("Release Excel");

		XL.ReleaseExcel();

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
		for (itr = m_pDoc->m_Movement3NodeMap.begin(); itr != m_pDoc->m_Movement3NodeMap.end(); itr++)
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

	CString msg;
	msg.Format ("Excel Automation Done!\n%d signalized nodes now have estimated movement capacity and signal timing plans.",QEM_node_count);
	AfxMessageBox(msg, MB_ICONINFORMATION);

#else if
	CString msg;
	msg.Format ("Excel automation is not available under the 64-bit version yet.");
	AfxMessageBox(msg, MB_ICONINFORMATION);

#endif

}

void CDlg_SignalDataExchange::OnBnClickedButtonGenerateVissimData()
{
		CWaitCursor wait;

   m_pDoc->GenerateMovementCountFromVehicleFile();
	m_pDoc->ExportPathflowToCSVFiles();
	m_pDoc->ConstructandexportVISSIMdata();
}


void CDlg_SignalDataExchange::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

BOOL CDlg_SignalDataExchange::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString info_str;

	info_str.Format ("Demand loading time period: %s -> %s, %d min", m_pDoc->GetTimeStampString (m_pDoc->m_DemandLoadingStartTimeInMin ), m_pDoc->GetTimeStampString (m_pDoc->m_DemandLoadingEndTimeInMin ), 
		
	m_pDoc->m_DemandLoadingEndTimeInMin - m_pDoc->m_DemandLoadingStartTimeInMin	);

	m_InfoList.AddString (info_str);

	info_str.Format("Volume conversion factor = %.2f", 60.f/max(1,m_pDoc->m_DemandLoadingEndTimeInMin - m_pDoc->m_DemandLoadingStartTimeInMin));
	m_InfoList.AddString (info_str);

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlg_SignalDataExchange::OnLbnSelchangeListInfo()
{
	// TODO: Add your control notification handler code here
}
