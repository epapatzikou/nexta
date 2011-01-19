// TLiteDoc.cpp : implementation of the CTLiteDoc class
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
#include "TLite.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "TLiteView.h"
#include "DlgMOE.h"
#include "DlgPathMOE.h"
#include "DlgTrainInfo.h"
#include "DlgFileLoading.h"
#include "DlgCarFollowing.h"
#include "MainFrm.h"
#include "Shellapi.h"
#include "DlgDefaultLinkProperties.h"
#include "DlgAssignmentSettings.h"
#include "DlgLinkList.h"
#include "DlgGridCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDlgMOE *g_LinkMOEDlg = NULL;
CDlgPathMOE	*g_pPathMOEDlg = NULL;
extern 	  std::list<CView*>	g_ViewList;
extern float g_Simulation_Time_Stamp;

bool g_LinkMOEDlgShowFlag = false;
std::list<DTALink*>	g_LinkDisplayList;


extern float g_GetPrivateProfileFloat( LPCTSTR section, LPCTSTR key, float def_value, LPCTSTR filename) ;

// CTLiteDoc

IMPLEMENT_DYNCREATE(CTLiteDoc, CDocument)

BEGIN_MESSAGE_MAP(CTLiteDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, &CTLiteDoc::OnFileOpen)
	ON_COMMAND(ID_TOOL_GENERATESENESORMAPPINGTABLE, &CTLiteDoc::OnToolGeneratesenesormappingtable)
	ON_COMMAND(ID_SHOW_SHOWPATHMOE, &CTLiteDoc::OnShowShowpathmoe)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SHOWPATHMOE, &CTLiteDoc::OnUpdateShowShowpathmoe)
	ON_COMMAND(ID_SEARCH_LISTTRAINS, &CTLiteDoc::OnSearchListtrains)
	ON_COMMAND(ID_TOOLS_TIMETABLINGOPTIMIZATION, &CTLiteDoc::OnToolsTimetablingoptimization)
	ON_COMMAND(ID_TIMETABLE_IMPORTTIMETABLE, &CTLiteDoc::OnTimetableImporttimetable)
	ON_COMMAND(ID_TIMETABLE_INITIALIZETIMETABLE, &CTLiteDoc::OnInitializetimetable)
	ON_COMMAND(ID_TIMETABLE_OPTIMIZETIMETABLE, &CTLiteDoc::OnOptimizetimetable_PriorityRule)
	ON_COMMAND(ID_FILE_SAVE_PROJECT, &CTLiteDoc::OnFileSaveProject)
	ON_COMMAND(ID_FILE_SAVE_PROJECT_AS, &CTLiteDoc::OnFileSaveProjectAs)
	ON_COMMAND(ID_ESTIMATION_ODESTIMATION, &CTLiteDoc::OnEstimationOdestimation)
	ON_COMMAND(ID_IMAGE_IMPORTBACKGROUNDIMAGE, &CTLiteDoc::OnImageImportbackgroundimage)
	ON_COMMAND(ID_FILE_DATALOADINGSTATUS, &CTLiteDoc::OnFileDataloadingstatus)
	ON_COMMAND(ID_MOE_VOLUME, &CTLiteDoc::OnMoeVolume)
	ON_COMMAND(ID_MOE_SPEED, &CTLiteDoc::OnMoeSpeed)
	ON_COMMAND(ID_MOE_DENSITY, &CTLiteDoc::OnMoeDensity)
	ON_COMMAND(ID_MOE_QUEUELENGTH, &CTLiteDoc::OnMoeQueuelength)
	ON_COMMAND(ID_MOE_FUELCONSUMPTION, &CTLiteDoc::OnMoeFuelconsumption)
	ON_COMMAND(ID_MOE_EMISSIONS, &CTLiteDoc::OnMoeEmissions)
	ON_UPDATE_COMMAND_UI(ID_MOE_VOLUME, &CTLiteDoc::OnUpdateMoeVolume)
	ON_UPDATE_COMMAND_UI(ID_MOE_SPEED, &CTLiteDoc::OnUpdateMoeSpeed)
	ON_UPDATE_COMMAND_UI(ID_MOE_DENSITY, &CTLiteDoc::OnUpdateMoeDensity)
	ON_UPDATE_COMMAND_UI(ID_MOE_QUEUELENGTH, &CTLiteDoc::OnUpdateMoeQueuelength)
	ON_UPDATE_COMMAND_UI(ID_MOE_FUELCONSUMPTION, &CTLiteDoc::OnUpdateMoeFuelconsumption)
	ON_UPDATE_COMMAND_UI(ID_MOE_EMISSIONS, &CTLiteDoc::OnUpdateMoeEmissions)
	ON_COMMAND(ID_MOE_NONE, &CTLiteDoc::OnMoeNone)
	ON_UPDATE_COMMAND_UI(ID_MOE_NONE, &CTLiteDoc::OnUpdateMoeNone)
	ON_COMMAND(ID_TOOLS_CARFOLLOWINGSIMULATION, &CTLiteDoc::OnToolsCarfollowingsimulation)
	ON_COMMAND(ID_TOOLS_PERFORMTRAFFICASSIGNMENT, &CTLiteDoc::OnToolsPerformtrafficassignment)
	ON_COMMAND(ID_MOE_VC_Ratio, &CTLiteDoc::OnMoeVcRatio)
	ON_UPDATE_COMMAND_UI(ID_MOE_VC_Ratio, &CTLiteDoc::OnUpdateMoeVcRatio)
	ON_COMMAND(ID_MOE_TRAVELTIME, &CTLiteDoc::OnMoeTraveltime)
	ON_UPDATE_COMMAND_UI(ID_MOE_TRAVELTIME, &CTLiteDoc::OnUpdateMoeTraveltime)
	ON_COMMAND(ID_MOE_CAPACITY, &CTLiteDoc::OnMoeCapacity)
	ON_UPDATE_COMMAND_UI(ID_MOE_CAPACITY, &CTLiteDoc::OnUpdateMoeCapacity)
	ON_COMMAND(ID_MOE_SPEEDLIMIT, &CTLiteDoc::OnMoeSpeedlimit)
	ON_UPDATE_COMMAND_UI(ID_MOE_SPEEDLIMIT, &CTLiteDoc::OnUpdateMoeSpeedlimit)
	ON_COMMAND(ID_MOE_FREEFLOWTRAVLETIME, &CTLiteDoc::OnMoeFreeflowtravletime)
	ON_UPDATE_COMMAND_UI(ID_MOE_FREEFLOWTRAVLETIME, &CTLiteDoc::OnUpdateMoeFreeflowtravletime)
	ON_COMMAND(ID_EDIT_DELETESELECTEDLINK, &CTLiteDoc::OnEditDeleteselectedlink)
	ON_COMMAND(ID_MOE_LENGTH, &CTLiteDoc::OnMoeLength)
	ON_UPDATE_COMMAND_UI(ID_MOE_LENGTH, &CTLiteDoc::OnUpdateMoeLength)
	ON_COMMAND(ID_EDIT_SETDEFAULTLINKPROPERTIESFORNEWLINKS, &CTLiteDoc::OnEditSetdefaultlinkpropertiesfornewlinks)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SETDEFAULTLINKPROPERTIESFORNEWLINKS, &CTLiteDoc::OnUpdateEditSetdefaultlinkpropertiesfornewlinks)
	ON_COMMAND(ID_TOOLS_PROJECTFOLDER, &CTLiteDoc::OnToolsProjectfolder)
	ON_COMMAND(ID_TOOLS_OPENNEXTAPROGRAMFOLDER, &CTLiteDoc::OnToolsOpennextaprogramfolder)
	ON_COMMAND(ID_MOE_ODDEMAND, &CTLiteDoc::OnMoeOddemand)
	ON_UPDATE_COMMAND_UI(ID_MOE_ODDEMAND, &CTLiteDoc::OnUpdateMoeOddemand)
	ON_COMMAND(ID_MOE_NOODMOE, &CTLiteDoc::OnMoeNoodmoe)
	ON_UPDATE_COMMAND_UI(ID_MOE_NOODMOE, &CTLiteDoc::OnUpdateMoeNoodmoe)
	ON_COMMAND(ID_ODTABLE_IMPORT_OD_TRIP_FILE, &CTLiteDoc::OnOdtableImportOdTripFile)
	ON_COMMAND(ID_TOOLS_EDITASSIGNMENTSETTINGS, &CTLiteDoc::OnToolsEditassignmentsettings)
	ON_COMMAND(ID_TOOLS_EDITODDEMANDTABLE, &CTLiteDoc::OnToolsEditoddemandtable)
	ON_COMMAND(ID_SEARCH_LINKLIST, &CTLiteDoc::OnSearchLinklist)
END_MESSAGE_MAP()


// CTLiteDoc construction/destruction


CTLiteDoc::~CTLiteDoc()
{
	if(m_pNetwork!=NULL)
		delete m_pNetwork;

	//	m_ODSize is used to keep the original size for m_DemandMatrix, in case some of nodes are deleted during operation
	if(m_DemandMatrix!=NULL)
			DeallocateDynamicArray<float>(m_DemandMatrix,m_ODSize,m_ODSize);

	// to do: delete m_TrainVector; m_node, link set;

}

BOOL CTLiteDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CTLiteDoc serialization

void CTLiteDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CTLiteDoc diagnostics

#ifdef _DEBUG
void CTLiteDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTLiteDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

int CTLiteDoc::FindLinkFromSensorLocation(float x, float y, int direction)
{

	double Min_distance = m_NetworkRect.Width()/100;  // set the selection threshod

	std::list<DTALink*>::iterator iLink;

	int SelectedLinkID = -1;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		GDPoint p0, pfrom, pto;
		p0.x  = x; p0.y  = y;
		pfrom.x  = (*iLink)->m_FromPoint.x; pfrom.y  = (*iLink)->m_FromPoint.y;
		pto.x  = (*iLink)->m_ToPoint.x; pto.y  = (*iLink)->m_ToPoint.y;

		if(direction == 1 && pfrom.x > pto.x)  // East, Xfrom should be < XTo
			continue;  //skip

		if(direction == 2 && pfrom.y < pto.y)  // South, Yfrom should be > YTo
			continue;

		if(direction == 3 && pfrom.x < pto.x)  // West, Xfrom should be > XTo
			continue;

		if(direction == 4 && pfrom.y > pto.y)  // North, Yfrom should be < YTo
			continue;

		float distance = g_DistancePointLine(p0, pfrom, pto);

		if(distance >0 && distance < Min_distance)
		{
			SelectedLinkID = (*iLink)->m_LinkID ;

			Min_distance = distance;
		}
	}

	return SelectedLinkID;

}

void CTLiteDoc::ReadSimulationLinkMOEData(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	g_Simulation_Time_Horizon = 1;
	if(st!=NULL)
	{
		while(!feof(st))
		{
			// from_node_id, to_node_id, timestamp_in_min, travel_time_in_min, delay_in_min, link_volume_in_veh, link_volume_in_vehphpl,
			//density_in_veh_per_mile_per_lane, speed_in_mph, queue_length_in_, cumulative_arrival_count, cumulative_departure_count
			int iteration_number = g_read_integer(st);
			if(iteration_number < 0)
				break;

			int from_node_number = g_read_integer(st);
			int to_node_number =  g_read_integer(st);
			int timestamp_in_min = g_read_float(st);

			if(g_Simulation_Time_Horizon < timestamp_in_min)
				g_Simulation_Time_Horizon = timestamp_in_min;

			float travel_time_in_min = g_read_float(st);
			float delay_in_min = g_read_float(st);
			float link_volume_in_veh = g_read_float(st);
			float link_volume_in_vehphpl = g_read_float(st);
			float density_in_veh_per_mile_per_lane = g_read_float(st);
			float speed_in_mph = g_read_float(st);
			float queue_length_perc = g_read_float(st);
			float cumulative_arrival_count =  g_read_integer(st);
			float cumulative_departure_count = g_read_integer(st);
		}

		fclose(st);
	}

	for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);
	}

	// reopen
	fopen_s(&st,lpszFileName,"r");

	if(st!=NULL)
	{
		int i = 0;
		while(!feof(st))
		{
			// from_node_id, to_node_id, timestamp_in_min, travel_time_in_min, delay_in_min, link_volume_in_veh, link_volume_in_vehphpl,
			//density_in_veh_per_mile_per_lane, speed_in_mph, queue_length_in_, cumulative_arrival_count, cumulative_departure_count
			int iteration_number = g_read_integer(st);
			if(iteration_number < 0)
				break;

			int from_node_number = g_read_integer(st);
			int to_node_number =  g_read_integer(st);
			int t = g_read_float(st);

			DTALink* pLink = FindLinkWithNodeNumbers(from_node_number , to_node_number );

			if(pLink!=NULL)
			{
				pLink->m_LinkMOEAry[t].ObsTravelTime = g_read_float(st);
				float delay_in_min = g_read_float(st);
				pLink->m_LinkMOEAry[t].ObsFlow  = g_read_float(st);
				float volume_for_alllanes = g_read_float(st);
				pLink->m_LinkMOEAry[t].ObsDensity = g_read_float(st);
				pLink->m_LinkMOEAry[t].ObsSpeed = g_read_float(st);
				pLink->m_LinkMOEAry[t].ObsQueuePerc = g_read_float(st);
				pLink->m_LinkMOEAry[t].ObsCumulativeFlow =  g_read_integer(st);
				float cumulative_departure_count = g_read_integer(st);
				i++;
			}else
			{
				// error message
			}

		}

		fclose(st);
		m_SimulationLinkMOEDataLoadingStatus.Format ("%d link records are loaded from file %s.",i,lpszFileName);
	}
}

void CTLiteDoc::ReadSimulationLinkStaticMOEData(LPCTSTR lpszFileName)
{

	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	g_Simulation_Time_Horizon = 1;
	for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);
	}

	int t=0;
	if(st!=NULL)
	{
		m_StaticAssignmentMode = true;
		int i = 1;
		while(!feof(st))
		{
			int from_node_number = g_read_integer(st);

			if(from_node_number == -1)
				break;

			int to_node_number =  g_read_integer(st);

			DTALink* pLink = FindLinkWithNodeNumbers(from_node_number , to_node_number );

			if(pLink!=NULL)

				if(pLink!=NULL)
				{
					float Capacity =   g_read_float(st);
					pLink->StaticFlow = g_read_float(st);
					pLink->StaticVOC   = g_read_float(st);
					float FreeflowTravelTime   = g_read_float(st);
					pLink->StaticTravelTime = g_read_float(st);
					pLink->StaticSpeed  = g_read_float(st);
					i++;
				}else
				{
					// error message
				}

		}

		fclose(st);
		m_SimulationLinkMOEDataLoadingStatus.Format ("%d link records are loaded from file %s.",i,lpszFileName);
	}

}

void CTLiteDoc::ReadHistoricalDataFormat2(CString directory)
{
	g_Simulation_Time_Horizon = 1440*g_Number_of_Weekdays/g_Data_Time_Interval;
	FILE* st = NULL;
	fopen_s(&st,directory+"SensorData.csv","r");
	int FirstDay = -1;

	if(st!=NULL)
	{

		std::list<DTALink*>::iterator iLink;

		while(!feof(st))
		{
			//			311831,60,2010-03-01 00:00:00,0,0,0,100
			long SensorID =g_read_integer(st);
			if(SensorID == -1)  // reach end of file
				break;

			int day_of_year = g_read_integer(st);

			if(FirstDay ==-1)
				FirstDay = day_of_year;

			int year = g_read_integer(st);
			int month = g_read_integer(st);
			int day = g_read_integer(st);
			int hour = g_read_integer(st);
			int min  = g_read_integer(st);
			int second = g_read_integer(st);
			float AvgSpeed = g_read_float(st);
			float AvgOcc = g_read_float(st);
			float TotalFlow = g_read_float(st);
			float PercentageObserved = g_read_float(st);

			map<long,long>::iterator it;

			int LinkID = -1;
			if ( (it = m_SensorIDtoLinkIDMap.find(SensorID)) != m_SensorIDtoLinkIDMap.end()) 
			{
				int LinkID = it->second;

				DTALink* pLink = m_LinkIDMap[LinkID];

				if(pLink!=NULL)
				{
					pLink->ResetMOEAry(g_Simulation_Time_Horizon);

					int Interval=5;

					int t  = ((day_of_year-FirstDay)*1440+ hour*60+min)/g_Data_Time_Interval;

					if(t<(*iLink)->m_SimulationHorizon)
					{

						ASSERT(pLink->m_NumLanes > 0);
						pLink->m_LinkMOEAry[ t].ObsFlow = TotalFlow*12/pLink->m_NumLanes;  // convert to per hour link flow
						pLink->m_LinkMOEAry[ t].ObsSpeed = AvgSpeed; 
						pLink->m_LinkMOEAry[ t].ObsTravelTime = pLink->m_Length/max(1,AvgSpeed);
						pLink->m_LinkMOEAry[t].ObsDensity = pLink->m_LinkMOEAry[t].ObsFlow / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeed);


						if(t>=1)
						{
							pLink->m_LinkMOEAry[t].ObsCumulativeFlow = pLink->m_LinkMOEAry[t-1].ObsCumulativeFlow + TotalFlow;
						}

					}

				}	
			}
		}

		fclose(st);
	}
}

void CTLiteDoc::ReadSensorLocationData(CString directory)
{

	CWaitCursor wc;
	FILE* st = NULL;
	bool bRectIni = false;
	fopen_s(&st,directory+"sensor.csv","r");
	if(st!=NULL)
	{
		int s = 1;
		while(!feof(st))
		{
			DTA_sensor sensor;

			sensor.FromNodeNumber =  g_read_integer(st);

			if(sensor.FromNodeNumber == -1)
				break;

			sensor.ToNodeNumber =  g_read_integer(st);
			sensor.SensorType  =  g_read_integer(st);
			sensor.OrgSensorID  =  g_read_integer(st);

			DTALink* pLink = FindLinkWithNodeNumbers(sensor.FromNodeNumber , sensor.ToNodeNumber );

			if(pLink!=NULL)
			{
				sensor.LinkID = pLink->m_LinkID ;
				m_SensorVector.push_back(sensor);
				m_SensorIDtoLinkIDMap[sensor.OrgSensorID] = pLink->m_LinkID;
				pLink->m_bSensorData  = true;

			}

			s++;
		}

		fclose(st);
	}

}
void CTLiteDoc::ReadHistoricalData(CString directory)
{
	CWaitCursor wc;
	FILE* st = NULL;
	std::list<DTALink*>::iterator iLink;
	int TimeHorizon = 1440; // 1440


	int Interval = 5;
	int day_count = 0;

	bool bResetMOEAryFlag = false;
	for(int day_of_month =1; day_of_month <= 31; day_of_month++)
	{
		CString str_day;
		str_day.Format ("%02d", day_of_month);

		fopen_s(&st,directory+"RWS2LinkObsValues"+str_day +".dat","r");

		if(st!=NULL)
		{

			if(!bResetMOEAryFlag)
			{
				g_Simulation_Time_Horizon = 1440*g_Number_of_Weekdays;

				for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
				{
					(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);
				}

				bResetMOEAryFlag = true;
			}

			g_read_integer(st); // 1140
			g_read_integer(st); // 5 

			std::list<DTALink*>::iterator iLink;


			while(!feof(st))
			{
				int SensorID  =  g_read_integer(st);
				if(SensorID == -1)  // reach end of file
					break;

				int LinkNo  = g_read_integer(st)-1;

				int Number_of_intervals = g_read_integer(st);

				int LinkID  = m_SensorVector[SensorID-1].LinkID;
				DTALink* pLink = m_LinkIDMap[LinkID];

				if(pLink==NULL)
				{
					CString msg;
					msg.Format("sensor %d does not exist",SensorID);
					AfxMessageBox(msg);
				}

				for(int t= day_count*1440; t<day_count*1440+TimeHorizon; t+=Interval)
				{
					if(t== day_count*1440)
					{
						pLink->m_LinkMOEAry[t].ObsCumulativeFlow = 0;
					}

					float Flow = g_read_float(st);

					//			TRACE("reading: %d, %f\n", LinkNo, Flow);

					if(pLink!=NULL)
					{
						pLink->m_LinkMOEAry[ t].ObsFlow = Flow*12/pLink->m_NumLanes;  // convert to per hour link flow


						if(t-Interval>0)
						{
							pLink->m_LinkMOEAry[t].ObsCumulativeFlow = pLink->m_LinkMOEAry[t-Interval].ObsCumulativeFlow + Flow;
						}

						// copy data to other intervals
						for(int tt = 1; tt<Interval; tt++)
						{
							pLink->m_LinkMOEAry[t+tt].ObsFlow = pLink->m_LinkMOEAry[t].ObsFlow;
							pLink->m_LinkMOEAry[t+tt].ObsCumulativeFlow = pLink->m_LinkMOEAry[t].ObsCumulativeFlow;
						}
					}
				}

			}

			fclose(st);
		}

		// speed
		fopen_s(&st,directory+"RWS2LinkSpdValues"+str_day+".dat","r");

		if(st!=NULL)
		{
			while(!feof(st))
			{
				int SensorID  =  g_read_integer(st);
				if(SensorID == -1)  // reach end of file
					break;
				TRACE("sensor ID = %d\n",SensorID);


				int LinkNo  = g_read_integer(st)-1;

				int Number_of_intervals = g_read_integer(st);

				int LinkID  = m_SensorVector[SensorID-1].LinkID;
				DTALink* pLink = m_LinkIDMap[LinkID];

				if(pLink==NULL)
				{
					CString msg;
					msg.Format("sensor %d does not exist",SensorID);
					AfxMessageBox(msg);
				}

				for(int t= day_count*1440; t<day_count*1440+TimeHorizon; t+=Interval)
				{

					float Speed = g_read_float(st);

					ASSERT(Speed<=2000);  // no large speed data, creat issues as two numbers are linked together

					// outlier checking:
					if(Speed >1.5* pLink->m_SpeedLimit )
						Speed = pLink->m_SpeedLimit;

					//			TRACE("reading: %d, %f\n", LinkNo, Speed);

					if(pLink!=NULL)
					{
						pLink->m_LinkMOEAry[t].ObsSpeed  = Speed*0.621371192f;  // km/h -> mph
						pLink->m_LinkMOEAry[ t].ObsTravelTime = pLink->m_Length/max(1,pLink->m_LinkMOEAry[t].ObsSpeed);
						pLink->m_LinkMOEAry[t].ObsDensity = pLink->m_LinkMOEAry[t].ObsFlow / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeed);

						// copy data to other intervals
						for(int tt = 1; tt<Interval; tt++)
						{
							pLink->m_LinkMOEAry[t+tt].ObsSpeed = pLink->m_LinkMOEAry[t].ObsSpeed;
							pLink->m_LinkMOEAry[t+tt].ObsDensity = pLink->m_LinkMOEAry[t].ObsDensity;

						}
					}
				}

			}

			day_count++;  // increaes day counter if there are data
			fclose(st);
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				(*iLink)->ComputeHistoricalAvg(23);  // total 23: 8 days as test
			}


		}



	}
}


BOOL CTLiteDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	FILE* st = NULL;
	//	cout << "Reading file node.csv..."<< endl;

	CString directory;
	m_ProjectFile = lpszPathName;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);


	m_ProjectDirectory = directory;
	m_ProjectTitle = GetWorkspaceTitleName(lpszPathName);
	SetTitle(m_ProjectTitle);

	CWaitCursor wc;


	if(!ReadNodeCSVFile(directory+"in_node.csv")) return false;
	if(!ReadLinkCSVFile(directory+"in_link.csv")) return false;

//	if(!ReadIncidentFile(directory+"incident.dat")) return false;


	CalculateDrawingRectangle();

	m_AdjLinkSize +=2;  // add two more elements to be safe

	std::list<DTANode*>::iterator iNode;
	//adjust XY coordinates if the corrdinate system is not consistenty
	if(fabs(m_UnitMile-1.00)>0.10)  // ask users if we need to adjust the XY coordinates
	{
		if(AfxMessageBox("The link length information in link.csv is not consistent with the X/Y coordinates in node.csv./nDo you want to adjust the the X/Y coordinates in node.csv?", MB_YESNO) == IDYES)

			for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
			{
				(*iNode)->pt.x = ((*iNode)->pt.x - min(m_NetworkRect.left,m_NetworkRect.right))*m_UnitMile;
				(*iNode)->pt.y = ((*iNode)->pt.y - min(m_NetworkRect.top, m_NetworkRect.bottom))*m_UnitMile;
			}

			std::list<DTALink*>::iterator iLink;
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				(*iLink)->m_FromPoint = m_NodeIDMap[(*iLink)->m_FromNodeID]->pt;
				(*iLink)->m_ToPoint = m_NodeIDMap[(*iLink)->m_ToNodeID]->pt;
			}

			m_UnitMile  = 1.0;
			m_UnitFeet = 1/5280.0;
			CalculateDrawingRectangle();

			UpdateAllViews(0);

	}

	bool m_bLinkShifted = true;
	if(m_bLinkShifted)
	{
		double link_offset = m_UnitFeet*80;  // 80 feet

		std::list<DTALink*>::iterator iLink;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			double DeltaX = (*iLink)->m_ToPoint.x - (*iLink)->m_FromPoint.x ;
			double DeltaY = (*iLink)->m_ToPoint.y - (*iLink)->m_FromPoint.y ;
			double theta = atan2(DeltaY, DeltaX);

			(*iLink)->m_FromPoint.x += link_offset* cos(theta-PI/2.0f);
			(*iLink)->m_ToPoint.x += link_offset* cos(theta-PI/2.0f);

			(*iLink)->m_FromPoint.y += link_offset* sin(theta-PI/2.0f);
			(*iLink)->m_ToPoint.y += link_offset* sin(theta-PI/2.0f);
		}
	}


	//: comment out now, it uses alternative format	ReadHistoricalDataFormat2(directory);

	if(!ReadZoneCSVFile(directory+"in_zone.csv")) return false;
	ReadDemandCSVFile(directory+"in_demand.csv");

	LoadSimulationOutput();

	ReadSensorLocationData(directory);
	ReadHistoricalData(directory);


	ReadObservationLinkVolumeData(directory+"input_static_obs_link_volume.csv");


	// for train timetabling
	ReadTrainProfileCSVFile(directory+"input_trainprofile.csv");
	ReadTimetableCVSFile(directory+"input_timetable.csv");

	ReadBackgroundImageFile(directory+"Background.bmp");

	m_bFitNetworkInitialized = false;

	CDlgFileLoading dlg;
	dlg.m_pDoc = this;
	dlg.DoModal ();

	UpdateAllViews(0);
	return true;
}

void CTLiteDoc::ReadBackgroundImageFile(LPCTSTR lpszFileName)
{
	//read impage file Background.bmp
	m_BackgroundBitmap.Load(lpszFileName);

	m_BackgroundBitmapLoaded = !(m_BackgroundBitmap.IsNull ());
	//	m_BackgroundBitmapLoaded = true;

	if(m_BackgroundBitmapLoaded)
	{
	TCHAR IniFilePath[_MAX_PATH] = _T("./in_opt_BackgroundImage.ini");

	m_ImageX1 = g_GetPrivateProfileFloat("BackgroundImage", "x1", m_NetworkRect.left, IniFilePath);
	m_ImageY1 = g_GetPrivateProfileFloat("BackgroundImage", "y1", m_NetworkRect.top, IniFilePath);
	m_ImageWidth = g_GetPrivateProfileFloat("BackgroundImage", "ImageWidth", m_NetworkRect.right-m_NetworkRect.left, IniFilePath);
	m_ImageHeight = g_GetPrivateProfileFloat("BackgroundImage", "ImageHeight", m_NetworkRect.bottom-m_NetworkRect.top, IniFilePath);

	m_ImageXResolution = g_GetPrivateProfileFloat("BackgroundImage", "ImageXResolution", 1, IniFilePath);
	m_ImageYResolution = g_GetPrivateProfileFloat("BackgroundImage", "ImageYResolution", 1, IniFilePath);
	m_ImageMoveSize = m_ImageWidth/1000.0f;
		m_BackgroundImageFileLoadingStatus.Format ("Optional background image file %s is loaded.",lpszFileName);
	
	}
	else
		m_BackgroundImageFileLoadingStatus.Format ("Optional background image file %s is not loaded.",lpszFileName);

}
// CTLiteDoc commands

void CTLiteDoc::OnFileOpen()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("NEXTA Project (*.dlp)|*.dlp|"));
	if(dlg.DoModal() == IDOK)
	{
		OnOpenDocument(dlg.GetPathName());
	}
}

void CTLiteDoc::OnFileSaveimagelocation()
{

	TCHAR IniFilePath[_MAX_PATH] = _T("./in_opt_BackgroundImage.ini");

	char lpbuffer[64];
	sprintf_s(lpbuffer,"%f",m_ImageX1);
	WritePrivateProfileString("BackgroundImage","x1",lpbuffer,IniFilePath);
	sprintf_s(lpbuffer,"%f",m_ImageY1);
	WritePrivateProfileString("BackgroundImage","y1",lpbuffer,IniFilePath);

	sprintf_s(lpbuffer,"%f",m_ImageWidth);
	WritePrivateProfileString("BackgroundImage","ImageWidth",lpbuffer,IniFilePath);

	sprintf_s(lpbuffer,"%f",m_ImageHeight);
	WritePrivateProfileString("BackgroundImage","ImageHeight",lpbuffer,IniFilePath);

	sprintf_s(lpbuffer,"%f",m_ImageXResolution);
	WritePrivateProfileString("BackgroundImage","ImageXResolution",lpbuffer,IniFilePath);

	sprintf_s(lpbuffer,"%f",m_ImageYResolution);
	WritePrivateProfileString("BackgroundImage","ImageYResolution",lpbuffer,IniFilePath);
}

COLORREF CTLiteDoc::GetLinkTypeColor(int LinkType)
{
	COLORREF color;
	switch (LinkType)
	{
	case 1: color = m_ColorFreeway; break;
	case 2: color = m_ColorHighway; break;
	default: color = m_ColorArterial;

	}
	return color;
}

void CTLiteDoc::OnToolGeneratesenesormappingtable()
{
	CWaitCursor wc;
	FILE* st = NULL;

	CString directory = m_ProjectDirectory;

	fopen_s(&st,directory+"station.csv","r");
	if(st!=NULL)
	{
		int s = 1;
		while(!feof(st))
		{
			DTA_sensor sensor;

			sensor.OrgSensorID  =  g_read_integer(st);
			if(sensor.OrgSensorID == -1)
				break;

			float y = g_read_float(st);  //lat
			float x = g_read_float(st);  //long
			int direction = g_read_integer(st);

			int SensorLinkID = FindLinkFromSensorLocation(x,y,direction);

			if(SensorLinkID>=0)
			{
				sensor.LinkID = SensorLinkID;
				m_SensorVector.push_back(sensor);

				m_LinkIDMap[SensorLinkID]->m_bSensorData  = true;


			}else
			{
				sensor.LinkID = -1;
				sensor.pt.x = x;
				sensor.pt.y  = y;
				m_SensorVector.push_back(sensor);

			}

			s++;
		}

		fclose(st);
	}

	// 	write sensor.csv
	// 	From Node,To Node,Type,OrgSensorID




	fopen_s(&st,directory+"sensor.csv","w");
	if(st!=NULL)
	{
		std::vector<DTA_sensor>::iterator iSensor;
		fprintf(st,"From Node,To Node,Type,OrgSensorID\n");
		for (iSensor = m_SensorVector.begin(); iSensor != m_SensorVector.end(); iSensor++)
		{
			if((*iSensor).LinkID>=0)
			{
				DTALink* pLink = m_LinkIDMap[(*iSensor).LinkID];
				fprintf(st,"%d,%d,0,%d\n", pLink->m_FromNodeNumber, pLink->m_ToNodeNumber, (*iSensor).OrgSensorID);
			}
		}

		fclose(st);

	}

}

int CTLiteDoc::Routing()
{

	CWaitCursor cws;
	m_NodeSizeSP = 0;  // reset 
	if(m_OriginNodeID>=0 && m_DestinationNodeID>=0)
	{
		if(m_pNetwork !=NULL)
			delete m_pNetwork;


		unsigned int iPath;
		for (iPath = 0; iPath < m_PathDisplayList.size(); iPath++)
		{
			DTAPath* pdp = m_PathDisplayList[iPath];

			if(pdp) delete pdp;
		}

		m_PathDisplayList.clear ();

		m_pNetwork = new DTANetworkForSP(m_NodeSet.size(), m_LinkSet.size(), 1, 1, m_AdjLinkSize);  //  network instance for single processor in multi-thread environment

		int Number_of_paths = 1;
		int NodeNodeSum = 0;

		// randomize link cost to avoid overlapping

		std::list<DTALink*>::iterator iLink;
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->m_OverlappingCost  = 0;

		}

		for(int p=0; p<Number_of_paths; p++)
		{
			NodeNodeSum = 0;
			TRACE("Path %d\n",p);

			m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, true, false);
			m_pNetwork->SimplifiedTDLabelCorrecting_DoubleQueue(m_OriginNodeID,0,0);
			int	NodeSize = 0;
			int PredNode = m_pNetwork->NodePredAry[m_DestinationNodeID];		
			m_PathNodeVectorSP[NodeSize++] = m_DestinationNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
			while(PredNode != m_OriginNodeID && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
			{
				ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
				m_PathNodeVectorSP[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				NodeNodeSum+= PredNode;
				PredNode = m_pNetwork->NodePredAry[PredNode];
			}
			m_PathNodeVectorSP[NodeSize++] = m_OriginNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.

			m_NodeSizeSP = NodeSize;


			// test overlapping path
			bool bNonOverlapping = true;
			for (iPath = 0; iPath < m_PathDisplayList.size(); iPath++)
			{
				DTAPath* pdp = m_PathDisplayList[iPath];

				if(pdp->m_LinkSize==NodeSize-1 &&  pdp->m_NodeNodeSum == NodeNodeSum)
				{
					bNonOverlapping = false;
					break;
				}

			}

			if(bNonOverlapping)
			{
				// update m_PathDisplayList
				DTAPath* pdp = new DTAPath(NodeSize-1,1440);


				for (int i=1 ; i < m_NodeSizeSP; i++)
				{
					DTALink* pLink = FindLinkWithNodeIDs(m_PathNodeVectorSP[i], m_PathNodeVectorSP[i-1]);
					pLink->m_OverlappingCost = 30;  // min

					TRACE("  %d-> %d",m_NodeIDtoNameMap[m_PathNodeVectorSP[i]], m_NodeIDtoNameMap[m_PathNodeVectorSP[i-1]]);
					if(pLink!=NULL)
					{
						pdp->m_LinkVector [m_NodeSizeSP-1-i] = pLink->m_LinkID ; //starting from m_NodeSizeSP-2, to 0

						pdp->m_Distance += pLink->m_Length ;

						if(i==1) // first link
						{
							pdp->m_TravelTime = g_Simulation_Time_Stamp + pLink->GetTravelTime(g_Simulation_Time_Stamp);
						}else
						{
							pdp->m_TravelTime = pdp->m_TravelTime + pLink->GetTravelTime(pdp->m_TravelTime);

						}

					}

				}
				pdp->m_NodeNodeSum = NodeNodeSum;
				m_PathDisplayList.push_back (pdp);

				if(m_PathDisplayList.size() >=NUM_PATHS-1)
					break;

				TRACE("\n",p);

			}


		}
		// find overlapping paths
		Number_of_paths = 10;

		for(int p=0; p<Number_of_paths; p++)
		{
			TRACE("Path %d\n",p);
			NodeNodeSum=0;

			m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, false, true);
			m_pNetwork->SimplifiedTDLabelCorrecting_DoubleQueue(m_OriginNodeID,0,0);
			int	NodeSize = 0;
			int PredNode = m_pNetwork->NodePredAry[m_DestinationNodeID];		
			m_PathNodeVectorSP[NodeSize++] = m_DestinationNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
			while(PredNode != m_OriginNodeID && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
			{
				ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
				m_PathNodeVectorSP[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				NodeNodeSum+= PredNode;

				PredNode = m_pNetwork->NodePredAry[PredNode];
			}
			m_PathNodeVectorSP[NodeSize++] = m_OriginNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.

			m_NodeSizeSP = NodeSize;


			// test overlapping path
			bool bNonOverlapping = true;
			for (iPath = 0; iPath < m_PathDisplayList.size(); iPath++)
			{
				DTAPath* pdp = m_PathDisplayList[iPath];

				if(pdp->m_LinkSize==NodeSize-1 &&  pdp->m_NodeNodeSum == NodeNodeSum)
				{
					bNonOverlapping = false;
					break;
				}

			}

			if(bNonOverlapping)
			{
				// update m_PathDisplayList
				DTAPath* pdp = new DTAPath(NodeSize-1,1440);

				for (int i=1 ; i < m_NodeSizeSP; i++)
				{
					DTALink* pLink = FindLinkWithNodeIDs(m_PathNodeVectorSP[i], m_PathNodeVectorSP[i-1]);
					pLink->m_OverlappingCost = 30.0f/Number_of_paths;  // min

					pdp->m_Distance += pLink->m_Length ;

					if(i==1) // first link
					{
						pdp->m_TravelTime = g_Simulation_Time_Stamp + pLink->GetTravelTime(g_Simulation_Time_Stamp);
					}else
					{
						pdp->m_TravelTime = pdp->m_TravelTime + pLink->GetTravelTime(pdp->m_TravelTime);
					}


					//					TRACE("  %d-> %d",m_NodeIDtoNameMap[m_PathNodeVectorSP[i]], m_NodeIDtoNameMap[m_PathNodeVectorSP[i-1]]);
					if(pLink!=NULL)
					{
						pdp->m_LinkVector [m_NodeSizeSP-1-i] = pLink->m_LinkID ;  //starting from m_NodeSizeSP-2, to 0


					}

				}
				pdp->m_NodeNodeSum = NodeNodeSum;


				m_PathDisplayList.push_back (pdp);

				//				TRACE("\nLinkSize =  %d, NodeNodeSum =%d\n",pdp->m_LinkSize ,pdp->m_NodeNodeSum);

				if(m_PathDisplayList.size() >=NUM_PATHS)
					break;

				TRACE("\n",p);

				m_SelectPathNo = -1;

			}


		}

		// calculate time-dependent travel time


		for(unsigned int p = 0; p < m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath* pdp = m_PathDisplayList[p];

			for(int t=0; t< g_Simulation_Time_Horizon; t+= TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL)  // for each starting time
			{
				pdp->m_TimeDependentTravelTime[t] =t;

				for (int i=0 ; i < pdp->m_LinkSize; i++)  // for each pass link
				{
					DTALink* pLink = m_LinkIDMap[m_PathDisplayList[p]->m_LinkVector[i]];

					pdp->m_TimeDependentTravelTime[t] += pLink->GetTravelTime(pdp->m_TimeDependentTravelTime[t]);

					//					TRACE("\n path %d, time at %f, TT = %f",p, pdp->m_TimeDependentTravelTime[t], pLink->GetTravelTime(pdp->m_TimeDependentTravelTime[t]) );

				}

				pdp->m_TimeDependentTravelTime[t] -= t; // remove the starting time, so we have pure travel time;

				ASSERT(pdp->m_TimeDependentTravelTime[t]>=0);

				if( pdp->m_MaxTravelTime < pdp->m_TimeDependentTravelTime[t])
					pdp->m_MaxTravelTime = pdp->m_TimeDependentTravelTime[t];


				//				TRACE("\n path %d, time at %d = %f",p, t,pdp->m_TimeDependentTravelTime[t]  );

			}

			pdp->UpdateWithinDayStatistics();

			/// calculate fuel consumptions
			for(unsigned int p = 0; p < m_PathDisplayList.size(); p++) // for each path
			{
				DTAPath* pdp = m_PathDisplayList[p];

				for(int t=0; t< min(1440,g_Simulation_Time_Horizon); t+= TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL)  // for each starting time
				{
					float CurrentTime = t;
					float FuelSum = 0;
					float CO2EmissionsSum = 0;
					float CO2;


					for (int i=0 ; i < pdp->m_LinkSize; i++)  // for each pass link
					{
						DTALink* pLink = m_LinkIDMap[m_PathDisplayList[p]->m_LinkVector[i]];

						FuelSum += pLink->ObtainHistFuelConsumption(CurrentTime);
						CO2= pLink->ObtainHistCO2Emissions(CurrentTime);
						CO2EmissionsSum+=CO2;

						CurrentTime += pLink->ObtainHistTravelTime(CurrentTime);

						//					TRACE("\n path %d, time at %f, TT = %f, Fuel %f. FS %f",p, pdp->m_TimeDependentTravelTime[t], pLink->GetTravelTime(pdp->m_TimeDependentTravelTime[t]),Fuel, FuelSum );

					}


					pdp->m_WithinDayMeanTimeDependentFuelConsumption[t] = FuelSum;
					pdp->m_WithinDayMeanTimeDependentEmissions[t]=CO2EmissionsSum;

					float value_of_time = 6.5f/60.0f;   // per min
					float value_of_fuel = 3.0f;  // per gallon
					float value_of_emissions = 0.24f;  // per pounds

					pdp->m_WithinDayMeanGeneralizedCost[t] = value_of_time* pdp->GetTravelTimeMOE(t,2)
						+ value_of_fuel* pdp->m_WithinDayMeanTimeDependentFuelConsumption[t]
					+ value_of_emissions*pdp->m_WithinDayMeanTimeDependentEmissions[t];

					//				TRACE("\n path %d, time at %d = %f",p, t,pdp->m_TimeDependentTravelTime[t]  );

				}

			}


		}

		UpdateAllViews(0);


		m_PathMOEDlgShowFlag = true;

		if(m_PathDisplayList.size() > 0)
		{
			if(g_pPathMOEDlg==NULL)
			{
				g_pPathMOEDlg = new CDlgPathMOE();
				g_pPathMOEDlg->m_pDoc  = this;

				g_pPathMOEDlg->Create(IDD_DIALOG_PATHMOE);
			}
			g_pPathMOEDlg->InsertPathMOEItem();

			g_pPathMOEDlg->ShowWindow(SW_SHOW);
		}
		return 1;
	}
	return 0;
}


void CTLiteDoc::OnShowShowpathmoe()
{
	m_PathMOEDlgShowFlag = !m_PathMOEDlgShowFlag;

	if(m_PathMOEDlgShowFlag)
	{
		if(g_pPathMOEDlg==NULL)
		{
			g_pPathMOEDlg = new CDlgPathMOE();
			g_pPathMOEDlg->m_pDoc = this;
			g_pPathMOEDlg->SetModelessFlag(TRUE);
			g_pPathMOEDlg->Create(IDD_DIALOG_PATHMOE);
		}

		g_pPathMOEDlg->ShowWindow(SW_SHOW);

	}else
	{
		g_pPathMOEDlg->ShowWindow(SW_HIDE);
	}

}

void CTLiteDoc::OnUpdateShowShowpathmoe(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(g_LinkMOEDlgShowFlag);
}

void CTLiteDoc::OnViewShowmoe()
{

	if(b_gStaticAssignmentFlag)
		return;

	g_LinkMOEDlgShowFlag = !g_LinkMOEDlgShowFlag;

	if(g_LinkMOEDlgShowFlag)
	{
		if(g_LinkMOEDlg==NULL)
		{
			g_LinkMOEDlg = new CDlgMOE();
			g_LinkMOEDlg->Create(IDD_DIALOG_MOE);
		}

		g_LinkMOEDlg->ShowWindow(SW_SHOW);

	}else
	{
		g_LinkMOEDlg->ShowWindow(SW_HIDE);
	}

}


void CTLiteDoc::OnSearchListtrains()
{
	CDlgTrainInfo train_dlg;

	train_dlg.m_pDoc = this;

	train_dlg.DoModal ();


}


bool CTLiteDoc::ReadNodeCSVFile(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");
	if(st!=NULL)
	{
		int i=0;
		DTANode* pNode = 0;
		while(!feof(st))
		{
			int id			= g_read_integer(st);
			if(id == -1)  // reach end of file
				break;

			float x	= g_read_float(st);
			float y	= g_read_float(st);
			// Create and insert the node
			pNode = new DTANode;
			pNode->pt.x = x;
			pNode->pt.y = y;

			pNode->m_NodeNumber = id;
			pNode->m_NodeID = i;
			pNode->m_ZoneID = 0;
			m_NodeSet.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeIDtoNameMap[i] = id;
			m_NodeNametoIDMap[id] = i;
			i++;
		}
		fclose(st);

		m_NodeDataLoadingStatus.Format ("%d nodes are loaded from file %s.",m_NodeSet.size(),lpszFileName);
		return true;
	}else
	{
		AfxMessageBox("Error: File in_node.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();

	}

}

bool CTLiteDoc::ReadLinkCSVFile(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	long i = 0;
	if(st!=NULL)
	{
		DTALink* pLink = 0;

		float default_distance_sum=0;
		float length_sum = 0;


		while(!feof(st))
		{
			int FromNodeNumber =  g_read_integer(st);
			if(FromNodeNumber == -1)  // reach end of file
				break;
			int ToNodeNumber = g_read_integer(st);

			int m_SimulationHorizon = 1;

			pLink = new DTALink(m_SimulationHorizon);

			pLink->m_LinkID = i;
			pLink->m_FromNodeNumber = FromNodeNumber;
			pLink->m_ToNodeNumber = ToNodeNumber;
			pLink->m_FromNodeID = m_NodeNametoIDMap[FromNodeNumber];
			pLink->m_ToNodeID= m_NodeNametoIDMap[ToNodeNumber];

			m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
			m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;


			unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
			m_NodeIDtoLinkMap[LinkKey] = pLink;

			float length = g_read_float(st);
			pLink->m_NumLanes= g_read_integer(st);
			pLink->m_SpeedLimit= g_read_float(st);
			pLink->StaticSpeed = pLink->m_SpeedLimit;

			pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance
			pLink->m_MaximumServiceFlowRatePHPL= g_read_float(st);
			pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
			pLink->m_link_type= g_read_integer(st);

			m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);

			pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
			pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;

			default_distance_sum+= pLink->DefaultDistance();
			length_sum += pLink ->m_Length;

			pLink->SetupMOE();
			m_LinkSet.push_back (pLink);
			m_LinkIDMap[i]  = pLink;
			i++;

		}

		m_UnitMile  = 1.0f;

		if(length_sum>0.000001f)
			m_UnitMile= length_sum / default_distance_sum ;

		m_UnitFeet = m_UnitMile/5280.0f;  

		fclose(st);
		m_LinkDataLoadingStatus.Format ("%d links are loaded from file %s.",m_LinkSet.size(),lpszFileName);


		return true;
	}else
	{
		AfxMessageBox("Error: File in_link.csv cannot be opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}


}

bool CTLiteDoc::ReadZoneCSVFile(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	m_NodeIDtoZoneNameMap.clear ();
		if(m_DemandMatrix!=NULL)
			DeallocateDynamicArray<float>(m_DemandMatrix,m_ODSize,m_ODSize);


	int lineno = 0 ;
	if(st!=NULL)
	{

		while(!feof(st))
		{
			int zone_number = g_read_integer(st);
			if(zone_number == -1)  // reach end of file
				break;

			int node_name=  g_read_integer(st);

			m_NodeIDtoZoneNameMap[m_NodeNametoIDMap[node_name]] = zone_number;
			// if there are multiple nodes for a zone, the last node id is recorded.
			int zoneid  = zone_number-1;
			m_ZoneIDtoNodeIDMap[zoneid] = m_NodeNametoIDMap[node_name];

			if(m_ODSize < zone_number)
				m_ODSize = zone_number;

			lineno++;
		}
		fclose(st);
		m_ZoneDataLoadingStatus.Format ("%d node-zone mapping entries are loaded from file %s.",lineno,lpszFileName);
		return true;
	}else
	{
		m_ZoneDataLoadingStatus.Format ("File in_zone.csv does not exist. Use default node-zone mapping table.");

		return false;
	}
}

bool CTLiteDoc::ReadDemandCSVFile(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	long lineno = 0;
	if(st!=NULL)
	{
		m_DemandMatrix   =  AllocateDynamicArray<float>(m_ODSize,m_ODSize);

		for(int i= 0; i<m_ODSize; i++)
			for(int j= 0; j<m_ODSize; j++)
			{
			m_DemandMatrix[i][j]= 0.0f;
			}

			m_MaxODDemand  = 1;
		while(!feof(st))
		{
			int origin_zone = g_read_integer(st);
			if(origin_zone == -1)  // reach end of file
				break;

			int destination_zone =  g_read_integer(st);
			float number_of_vehicles =  g_read_float(st);
			int vehicle_type =  g_read_integer(st);
			float starting_time_in_min = g_read_float(st);
			float ending_time_in_min = g_read_float(st);

			int origin_zone_id  = origin_zone - 1;
			int destination_zone_id  = destination_zone - 1;

			m_DemandMatrix[origin_zone_id][destination_zone_id] = number_of_vehicles;

			if(m_MaxODDemand < number_of_vehicles)
				m_MaxODDemand =  number_of_vehicles ;
	
			lineno++;
		}
		fclose(st);
		m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file %s.",lineno,lpszFileName);
		return true;
	}else
	{
		AfxMessageBox("Error: File in_demand.csv cannot be opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}
}

bool CTLiteDoc::ReadTripTxtFile(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	long lineno = 0;
	if(st!=NULL)
	{
		m_DemandMatrix   =  AllocateDynamicArray<float>(m_ODSize,m_ODSize);

		for(int i= 0; i<m_ODSize; i++)
			for(int j= 0; j<m_ODSize; j++)
			{
			m_DemandMatrix[i][j]= 0.0f;
			}

			m_MaxODDemand  = 1;

		while(!feof(st))
		{
		for(int i = 1; i <=m_ODSize; i++)
		{
			int origin_zone	       = g_read_integer(st);
			if(origin_zone == -1)  // reach end of file
				break;

			for(int j = 1; j <=m_ODSize; j++)
			{
				int destination_zone	   = g_read_integer_with_char_O(st);  // we might read "Origin" here, so if we find "O", we return 0 as a key to jump to the next origin.

				if(destination_zone == 0) // we reach "O" of string Origin 
				{
					break;
				}

				destination_zone = j;  // use destination that we read to override j

				float number_of_vehicles = g_read_float(st);

			int origin_zone_id  = origin_zone - 1;
			int destination_zone_id  = destination_zone - 1;

			m_DemandMatrix[origin_zone_id][destination_zone_id] = number_of_vehicles;

			if(m_MaxODDemand < number_of_vehicles)
				m_MaxODDemand =  number_of_vehicles ;
	
			lineno++;
			}
			}
		}
		fclose(st);
		m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file %s.",lineno,lpszFileName);
		return true;
	}else
	{
		AfxMessageBox("Error: OD Trip file cannot be opened.");
		return false;
		//		g_ProgramStop();
	}
}

bool CTLiteDoc::ReadIncidentFile(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	int i;
	if(st!=NULL)
	{
		int NumberofCapacityReductionLinks = g_read_integer(st);

		if(NumberofCapacityReductionLinks >0)
		{
			//			g_LogFile << "# of capacity reduction links = " << NumberofCapacityReductionLinks << endl;
		}


		for(i = 0; i < NumberofCapacityReductionLinks; i++)
		{
			int usn  = g_read_integer(st);
			int dsn =  g_read_integer(st);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn );

			if(plink!=NULL)
			{
				CapacityReduction cs;

				cs.StartTime = g_read_integer(st);
				cs.EndTime = g_read_integer(st);
				cs.LaneClosureRatio= g_read_float(st);
				plink->CapacityReductionVector.push_back(cs);
			}
		}

		m_LinkDataLoadingStatus.Format ("%d damage locations are loaded from file %s.",i,lpszFileName);

		fclose(st);
	}
	return true;
}
void CTLiteDoc::OnToolsTimetablingoptimization()
{
	TimetableOptimization_Lagrangian_Method();
}

void CTLiteDoc::OnOptimizetimetable_PriorityRule()
{
	TimetableOptimization_Priority_Rule();

}

void CTLiteDoc::SendTexttoStatusBar(CString str)
{
	POSITION pos = GetFirstViewPosition();
	CView *p = (CView *)this->GetNextView(pos);
	CStatusBar* bar = (CStatusBar*)(p->GetParentFrame()->GetMessageBar());
	bar->SetPaneText(1, str);
}


BOOL CTLiteDoc::SaveProject(LPCTSTR lpszPathName)
{
	FILE* st = NULL;
	CString directory;
	CString prj_file = lpszPathName;
	directory = prj_file.Left(prj_file.ReverseFind('\\') + 1);

	m_ProjectDirectory = directory;

	CWaitCursor wc;

	fopen_s(&st,lpszPathName,"w");
	if(st!=NULL)
	{
		fprintf(st,"This project include node.csv and link.csv.");
		fclose(st);
	}else
	{
		AfxMessageBox("Error in writing the project file. Please check if the file is opened by another project or the folder is read-only.");
	}

	fopen_s(&st,directory+"in_node.csv","w");
	if(st!=NULL)
	{
		std::list<DTANode*>::iterator iNode;
		fprintf(st, "node, X, Y\n");
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			fprintf(st, "%d, %10.4f, %10.4f\n", (*iNode)->m_NodeNumber , (*iNode)->pt .x, (*iNode)->pt .y);
		}

		fclose(st);
	}else
	{
		AfxMessageBox("Error: File node.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}

	fopen_s(&st,directory+"in_link.csv","w");
	if(st!=NULL)
	{
		std::list<DTALink*>::iterator iLink;
		fprintf(st,"from_node, to_node, length_in_mile, number_of_lanes, speed_limit_in_mph, lane_capacity_in_veh_per_time_interval (e.g. hour in traffic assignment), link_type_for_display\n");
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			fprintf(st,"%d, %d, %10.4f, %d, %5.0f, %7.2f, %d\n",
				(*iLink)->m_FromNodeNumber, (*iLink)->m_ToNodeNumber ,(*iLink)->m_Length ,(*iLink)->m_NumLanes ,(*iLink)->m_SpeedLimit,(*iLink)->m_LaneCapacity ,(*iLink)->m_link_type);
		}
		fclose(st);
	}else
	{
		AfxMessageBox("Error: File link.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}

/*
	fopen_s(&st,directory+"in_zone.csv","w");
	if(st!=NULL)
	{
		std::list<DTALink*>::iterator iLink;
		fprintf(st,"in_node, to_node, length_in_mile, number_of_lanes, speed_limit_in_mph, lane_capacity_in_veh_per_time_interval (e.g. hour in traffic assignment), link_type_for_display\n");
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			fprintf(st,"%d, %d, %10.4f, %d, %5.0f, %7.2f, %d\n",
				(*iLink)->m_FromNodeNumber, (*iLink)->m_ToNodeNumber ,(*iLink)->m_Length ,(*iLink)->m_NumLanes ,(*iLink)->m_SpeedLimit,(*iLink)->m_LaneCapacity ,(*iLink)->m_link_type);
		}
		fclose(st);
	}else
	{
		AfxMessageBox("Error: File in_zone.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}

*/
	// save in_demand here

	if(m_BackgroundBitmapImportedButnotSaved)
	{
		m_BackgroundBitmap.Save(directory+"background.bmp",Gdiplus::ImageFormatBMP);  // always use bmp format
		m_BackgroundBitmapImportedButnotSaved = false;
	}

	OnFileSaveimagelocation();

	return true;
}
void CTLiteDoc::OnFileSaveProject()
{
	if(m_ProjectDirectory.GetLength ()>3)
		SaveProject(m_ProjectFile);
	else
		OnFileSaveProjectAs();
}

void CTLiteDoc::OnFileSaveProjectAs()
{
	CFileDialog fdlg (FALSE, "*.dlp", "*.dlp",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"NeXTA Project (*.dlp)|*.dlp|", NULL);

	if(fdlg.DoModal()==IDOK)
	{
		CString path = fdlg.GetFileName ();
		CWaitCursor wait;
		if(SaveProject(fdlg.GetPathName()))
		{
			CString msg;
			msg.Format ("Files node.csv and link.csv have been successfully saved with %d nodes, %d links.",m_NodeSet.size(), m_LinkSet.size());
			AfxMessageBox(msg,MB_OK|MB_ICONINFORMATION);

		}
	}
	// TODO: Add your command handler code here
}

void CTLiteDoc::CalculateDrawingRectangle()
{
	bool bRectIni = false;
	m_AdjLinkSize = 0;

	for (std::list<DTANode*>::iterator iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		if((*iNode)->m_Connections >0)
		{
			if(!bRectIni)
			{
				m_NetworkRect.left = (*iNode)->pt.x ;
				m_NetworkRect.right = (*iNode)->pt.x;
				m_NetworkRect.top = (*iNode)->pt.y;
				m_NetworkRect.bottom = (*iNode)->pt.y;
				bRectIni = true;
			}

			if((*iNode)->m_Connections > m_AdjLinkSize)
				m_AdjLinkSize = (*iNode)->m_Connections;

			m_NetworkRect.Expand((*iNode)->pt);
		}

	}

}


void CTLiteDoc::ReadVehicleCSVFile(LPCTSTR lpszFileName)
{

	//   cout << "Read vehicle file... "  << endl;
	//iteration, vehicle_id,  origin_zone_id, destination_zone_id, departure_time,
	//	arrival_time, complete_flag, trip_time, vehicle_type, occupancy, information_type,
	//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
	//	node id, node arrival time

	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");
	if(st!=NULL)
	{
		int count = 0;
		while(!feof(st))
		{
			int iteration= g_read_integer(st);
			if(iteration == -1)
				break;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_VehicleID		= g_read_integer(st);
			pVehicle->m_OriginZoneID	= g_read_integer(st);
			pVehicle->m_DestinationZoneID=g_read_integer(st);
			pVehicle->m_DepartureTime	= g_read_float(st);
			pVehicle->m_ArrivalTime = g_read_float(st);
			int CompleteFlag = g_read_integer(st);
			if(CompleteFlag==0) 
				pVehicle->m_bComplete = false;
			else
				pVehicle->m_bComplete = true;

			pVehicle->m_TripTime  = g_read_float(st);

			pVehicle->m_VehicleType = (unsigned char)g_read_integer(st);
			pVehicle->m_InformationClass = (unsigned char)g_read_integer(st);
			pVehicle->m_Occupancy = (unsigned char)g_read_integer(st);

			//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
			float value_of_time = g_read_float(st);
			float path_min_cost = g_read_float(st);
			float distance_in_mile = g_read_float(st);

			pVehicle->m_NodeSize	= g_read_integer(st);
			pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

			for(int i=0; i< pVehicle->m_NodeSize; i++)
			{
				m_PathNodeVectorSP[i] = g_read_integer(st);
				if(i>=1)
				{
					DTALink* pLink = FindLinkWithNodeNumbers(m_PathNodeVectorSP[i-1],m_PathNodeVectorSP[i]);
					pVehicle->m_NodeAry[i].LinkID = pLink->m_LinkID ;
				}
				pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = g_read_float(st);
			}

			m_VehicleSet.push_back (pVehicle);
			m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;

			count++;
		}

		fclose(st);
		m_SimulationVehicleDataLoadingStatus.Format ("%d vehicles are loaded from file %s.",count,lpszFileName);

	}
}


int CTLiteDoc::GetVehilePosition(DTAVehicle* pVehicle, double CurrentTime, float& ratio)
{
	GDPoint pt;
	pt.x = 0;
	pt.y = 0;

	int search_counter = 0;
	int beg, end, mid;
	beg = 1;
	end = pVehicle->m_NodeSize-1;
	mid = (beg+end)/2;
	int i = mid;
	ratio = 0;
	float link_travel_time;
	float remaining_time;

	while(beg<=end)
	{
		if(CurrentTime >= pVehicle->m_NodeAry [i-1].ArrivalTimeOnDSN &&
			CurrentTime <= pVehicle->m_NodeAry [i].ArrivalTimeOnDSN )	// find the link between the time interval
		{

			link_travel_time = pVehicle->m_NodeAry [i].ArrivalTimeOnDSN - pVehicle->m_NodeAry [i-1].ArrivalTimeOnDSN;

			remaining_time = CurrentTime - pVehicle->m_NodeAry [i-1].ArrivalTimeOnDSN;

			ratio = 1-remaining_time/link_travel_time;

			if(ratio <0)
				ratio = 0;

			if(ratio >1)
				ratio = 1;

			return pVehicle->m_NodeAry [i].LinkID;

			//			pt.x =  ratio*pLink->m_FromPoint .x  +  (1-ratio)*ratio*pLink->m_ToPoint .x;
			//			pt.y =  ratio*pLink->m_FromPoint .y  +  (1-ratio)*ratio*pLink->m_ToPoint .y;

		}else  // not found
		{
			if( CurrentTime>= pVehicle->m_NodeAry [i].ArrivalTimeOnDSN)  // time stamp is after i
			{
				// shift to the right
				beg=mid+1;
				mid=(beg+end)/2;
				i = mid;
			}else //CurrentTime < pVehicle->m_NodeAry[i].ArrivalTimeOnDSN// time stamp is before i
			{   //shift to the left
				end=mid-1;
				mid=(beg+end)/2;
				i = mid;
			}

			search_counter++;

			if(search_counter > pVehicle->m_NodeSize)  // exception handling
			{
				//				ASSERT(false);
				return 0;
			}

		}
	}

	return 0;
}
void CTLiteDoc::OnImageImportbackgroundimage()
{
	CString strFilter = "Bitmap image|*.bmp|JPEG image|*.jpg|GIF image|*.gif|PNG image|*.png||";

	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,strFilter);
	//	dlg.m_ofn.nFilterIndex = m_nFilterLoad;
	HRESULT hResult = (int)dlg.DoModal();
	if (FAILED(hResult)) {
		return;
	}
	ReadBackgroundImageFile(dlg.GetPathName());
	m_bFitNetworkInitialized = false;
	m_BackgroundBitmapImportedButnotSaved = true;
	UpdateAllViews(0);
}


void CTLiteDoc::OnFileDataloadingstatus()
{
	CDlgFileLoading dlg;
	dlg.m_pDoc = this;
	dlg.DoModal ();
}

void CTLiteDoc::OnMoeVolume()
{
	m_LinkMOEMode = volume;
	UpdateAllViews(0);}

void CTLiteDoc::OnMoeSpeed()
{
	m_LinkMOEMode = speed;
	UpdateAllViews(0);}

void CTLiteDoc::OnMoeDensity()
{
	m_LinkMOEMode = density;
	UpdateAllViews(0);}

void CTLiteDoc::OnMoeQueuelength()
{
	m_LinkMOEMode = queuelength;
	UpdateAllViews(0);}

void CTLiteDoc::OnMoeFuelconsumption()
{
	m_LinkMOEMode = fuel;
	UpdateAllViews(0);}

void CTLiteDoc::OnMoeEmissions()
{
	m_LinkMOEMode = emissions;
	UpdateAllViews(0);}

void CTLiteDoc::OnUpdateMoeVolume(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == volume);
}

void CTLiteDoc::OnUpdateMoeSpeed(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == speed);
}

void CTLiteDoc::OnUpdateMoeDensity(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == density);
}

void CTLiteDoc::OnUpdateMoeQueuelength(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == queuelength);
}

void CTLiteDoc::OnUpdateMoeFuelconsumption(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == fuel);
}

void CTLiteDoc::OnUpdateMoeEmissions(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == emissions);

}

void CTLiteDoc::OnMoeNone()
{
	m_LinkMOEMode = none;
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeNone(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == none);
}

float CTLiteDoc::GetLinkMOE(DTALink* pLink, Link_MOE LinkMOEMode, int CurrentTime)
{
	float power = 0.0f;
	float max_link_volume = 8000.0f;

	float max_speed_ratio = 2.0f; 
	float max_density = 45.0f;
	switch (LinkMOEMode)
	{
	case volume:  power = pLink->m_LinkMOEAry[CurrentTime].ObsFlow* pLink->m_NumLanes/max_link_volume; break;
	case speed:   power = pLink->m_SpeedLimit / max(1, pLink->m_LinkMOEAry [CurrentTime].ObsSpeed)/max_speed_ratio; break;

	case density: power = pLink->m_LinkMOEAry[CurrentTime].ObsDensity /max_density; break;
	case fuel:
	case emissions: 

	default: power = 0.0;

	}
	if(power>=1.0f) power = 1.0f;
	if(power<0.0f) power = 0.0f;

	return power;
}

float CTLiteDoc::GetStaticLinkMOE(DTALink* pLink, Link_MOE LinkMOEMode,float &value)
{

	float power = 0.0f;
	float max_link_volume = 8000.0f;
	float max_speed_ratio = 2.0f; 
	float max_density = 45.0f;
	switch (LinkMOEMode)
	{
	case volume:  power = pLink->StaticFlow/max_link_volume; 
		value = pLink->StaticFlow;
		break;
	case speed:   power = pLink->m_SpeedLimit / max(1, pLink->StaticSpeed)/max_speed_ratio; 
		value = pLink->StaticSpeed;
		break;
	case vcratio: power = pLink->StaticVOC;
		value = pLink->StaticVOC;
		break;
	case traveltime:  power = pLink->m_SpeedLimit / max(1, pLink->StaticSpeed)/max_speed_ratio; 
		value = pLink->StaticTravelTime;
		break;

	case capacity:  power = 1-pLink->m_LaneCapacity/4000; 
		value = pLink->m_LaneCapacity * pLink->m_NumLanes ;
		break;

	case speedlimit:  power = max_speed_ratio / pLink->m_SpeedLimit;
		value = pLink->m_SpeedLimit  ;
		break;

	case fftt:  power =  max_speed_ratio / pLink->m_SpeedLimit;;
		value = pLink->m_FreeFlowTravelTime ;
		break;

	case length:  power = max_speed_ratio / pLink->m_SpeedLimit;;
		value = pLink->m_Length  ;
		break;

	default: power = 0.0;

	}
	if(power>=1.0f) power = 1.0f;
	if(power<0.0f) power = 0.0f;

	return power;
}

void CTLiteDoc::OnToolsCarfollowingsimulation()
{
	CDlgCarFollowing dlg;
	dlg.DoModal ();
}

DWORD g_ProcessWait(DWORD PID) 
{
   DWORD dwRetVal = DWORD(-1);
   HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
   if(hProcess) {
      do 
	  {
	 ::Sleep(500);
	 ::GetExitCodeProcess(hProcess, &dwRetVal);
      } while(dwRetVal == STILL_ACTIVE);
      CloseHandle(hProcess);
   }
   return dwRetVal;
}

DWORD g_ProcessExecute(CString & strCmd, CString & strArgs,  CString & strDir, BOOL bWait)
{

   STARTUPINFO StartupInfo;
   ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
   StartupInfo.cb = sizeof(STARTUPINFO);

   PROCESS_INFORMATION ProcessInfo;
   DWORD dwRetVal = DWORD(-1);

   BOOL bStat =	CreateProcess(
			      strCmd,						// pointer to name of executable module
			      0,			// pointer to command line string
			      0,						// pointer to process security attributes
			      0,						// pointer to thread security attributes
			      TRUE,						// handle inheritance flag
			      0,						// creation flags
			      0,						// pointer to new environment block
			      strDir.IsEmpty() ? 0 : LPCTSTR(strDir),
			      &StartupInfo,				// pointer to STARTUPINFO
			      &ProcessInfo				// pointer to PROCESS_INFORMATION
			     );
   if( bStat ) 
   {
      if( bWait ) 
	  {
		 ::WaitForInputIdle(ProcessInfo.hProcess, INFINITE);
		 dwRetVal = g_ProcessWait(ProcessInfo.dwProcessId);
      } else {
	 // before we return to the caller, we wait for the currently
	 // started application until it is ready to work.
		 ::WaitForInputIdle(ProcessInfo.hProcess, INFINITE);
		 dwRetVal = ProcessInfo.dwProcessId;
      }

      ::CloseHandle(ProcessInfo.hThread);
   }
   
   return dwRetVal;
}


void CTLiteDoc::OnToolsPerformtrafficassignment()
{
	STARTUPINFO si = { 0 };  
	PROCESS_INFORMATION pi = { 0 };  

	si.cb = sizeof(si); 

	CWaitCursor curs;
	if(!SetCurrentDirectory(m_ProjectDirectory))
	{
		if(m_ProjectDirectory.GetLength()==0)
		{
			AfxMessageBox("The project directory has not been specified.");
			return;
		}
		CString msg;
		msg.Format ("The specified project directory %s does not exist.", m_ProjectDirectory);
		AfxMessageBox(msg);
		return;
	}

	CString sCommand;
   CString strParam;
   CTime ExeStartTime = CTime::GetCurrentTime();

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	sCommand.Format("%s\\TrafficAssignment.exe", pMainFrame->m_CurrentDirectory);

	g_ProcessExecute(sCommand, strParam, m_ProjectDirectory, true);
	
   CTime ExeEndTime = CTime::GetCurrentTime();

   CTimeSpan ts = ExeEndTime  - ExeStartTime;
   CString str_running_time;
   str_running_time.Format ("%s\nProgram execution time: %d hour(s) %d min(s) %d sec(s) \n%s", "Program execution has completed.",
			    ts.GetHours(), ts.GetMinutes(), ts.GetSeconds(), "Do you want to load the output now?");

   if( AfxMessageBox(str_running_time, MB_YESNO| MB_ICONINFORMATION)==IDYES)
   {
		LoadSimulationOutput();
		UpdateAllViews(0);
   }

}
void CTLiteDoc::LoadSimulationOutput()
{
//	ReadSimulationLinkMOEData(m_ProjectDirectory+"LinkMOE.csv");
	ReadSimulationLinkStaticMOEData(m_ProjectDirectory+"LinkStaticMOE.csv");
//	ReadVehicleCSVFile(m_ProjectDirectory+"Vehicle.csv");
}


void CTLiteDoc::OnMoeVcRatio()
{
	m_LinkMOEMode = vcratio;
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeVcRatio(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == vcratio);
}

void CTLiteDoc::OnMoeTraveltime()
{
	m_LinkMOEMode = traveltime;

	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeTraveltime(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == traveltime);
}

void CTLiteDoc::OnMoeCapacity()
{
	m_LinkMOEMode = capacity;

	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeCapacity(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == capacity);
}

void CTLiteDoc::OnMoeSpeedlimit()
{
	m_LinkMOEMode = speedlimit;

	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeSpeedlimit(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == speedlimit);
}

void CTLiteDoc::OnMoeFreeflowtravletime()
{
	m_LinkMOEMode = fftt;

	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeFreeflowtravletime(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == fftt);
}


CString CTLiteDoc::GetWorkspaceTitleName(CString strFullPath)
{
   int len = strFullPath.GetLength();
   /////////////////////////////////

   CString	strPathTitleName;

   strPathTitleName.Empty ();
   bool StartFlag = false;

   for(int k=len-1;k>=0;k--)
   {
      if(strFullPath[k]=='\\')
	 break;

      if(strFullPath[k]=='.' && StartFlag == false)
      {
	 StartFlag = true;
	 continue;
      }

      if(StartFlag == true)
	 strPathTitleName+=strFullPath[k];

   }

   strPathTitleName.MakeReverse();

   return strPathTitleName;

}
void CTLiteDoc::OnEditDeleteselectedlink()
{
	
	// TODO: Add your command handler code here
}


void CTLiteDoc::OnMoeLength()
{
	m_LinkMOEMode = length;

	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeLength(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == length);
}

void CTLiteDoc::OnEditSetdefaultlinkpropertiesfornewlinks()
{
	CDlgDefaultLinkProperties dlg;

		dlg.SpeedLimit = m_DefaultSpeedLimit ;
		dlg.LaneCapacity = m_DefaultCapacity ;
		dlg.NumLanes = m_DefaultNumLanes;
		dlg.LinkType = m_DefaultLinkType;

		if(dlg.DoModal() == IDOK)
		{
		m_DefaultSpeedLimit = dlg.SpeedLimit;
		m_DefaultCapacity = dlg.LaneCapacity;
		m_DefaultNumLanes = dlg.NumLanes;
		m_DefaultLinkType = dlg.LinkType;

		}
}

void CTLiteDoc::OnUpdateEditSetdefaultlinkpropertiesfornewlinks(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CTLiteDoc::OnToolsProjectfolder()
{
  if(m_ProjectDirectory.GetLength()==0)
   {
      AfxMessageBox("The project directory has not been specified.");
      return;
   }

	ShellExecute( NULL,  "explore", m_ProjectDirectory, NULL,  NULL, SW_SHOWNORMAL );
}

void CTLiteDoc::OnToolsOpennextaprogramfolder()
{
   CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	ShellExecute( NULL,  "explore", pMainFrame->m_CurrentDirectory, NULL,  NULL, SW_SHOWNORMAL );
}

void CTLiteDoc::OnMoeOddemand()
{
	m_LinkMOEMode = oddemand;
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeOddemand(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == oddemand);
}

void CTLiteDoc::OnMoeNoodmoe()
{
	m_ODMOEMode = odnone;
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeNoodmoe(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ODMOEMode == none);
}


void CTLiteDoc::OnOdtableImportOdTripFile()
	{
	CString strFilter = "OD Trip File|*.txt";

	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,strFilter);
	//	dlg.m_ofn.nFilterIndex = m_nFilterLoad;
	HRESULT hResult = (int)dlg.DoModal();
	if (FAILED(hResult)) {
		return;
	}
	ReadTripTxtFile(dlg.GetPathName());
	m_bFitNetworkInitialized = false;
	UpdateAllViews(0);
}

void CTLiteDoc::OnToolsEditassignmentsettings()
{
	CString SettingsFile;
	SettingsFile.Format ("%sDTASettings.ini",m_ProjectDirectory);

	int NumberOfIterations = (int)(g_GetPrivateProfileFloat("assignment", "number_of_iterations", 10, SettingsFile));	
	float DemandGlobalMultiplier = g_GetPrivateProfileFloat("demand", "global_multiplier",1.0,SettingsFile);	

	CDlgAssignmentSettings dlg;
	dlg.m_NumberOfIterations = NumberOfIterations;
	dlg.m_DemandGlobalMultiplier = DemandGlobalMultiplier;

	if(dlg.DoModal() ==IDOK)
	{
	 NumberOfIterations = dlg.m_NumberOfIterations;
	 DemandGlobalMultiplier = dlg.m_DemandGlobalMultiplier;

	 char lpbuffer[64];
	sprintf_s(lpbuffer,"%4d",NumberOfIterations);
	WritePrivateProfileString("assignment","number_of_iterations",lpbuffer,SettingsFile);

	sprintf_s(lpbuffer,"%5.3f",DemandGlobalMultiplier);
	WritePrivateProfileString("demand","global_multiplier",lpbuffer,SettingsFile);

	}

}

void CTLiteDoc::OnToolsEditoddemandtable()
{
	CDlgGridCtrl dlg;
	dlg.DoModal();
}

void CTLiteDoc::OnSearchLinklist()
{
	CDlgLinkList dlg;
	dlg.m_pDoc = this;
	dlg.DoModal ();

}