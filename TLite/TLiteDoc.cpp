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
#include "Geometry.h"
#include "CSVParser.h"
#include "RecordSetExt.h"
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
#include "Dlg_ImageSettings.h"
#include "Shellapi.h"
#include "DlgSensorDataLoading.h"
#include "Dlg_ImportODDemand.h"
#include "DlgVehiclePath.h"
#include "DlgNetworkAlignment.h"
#include "Dlg_VehEmissions.h"
#include "DlgScenario.h"
#include "DlgMOETabView.h"
#include "Dlg_ImportShapeFiles.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDlgMOE *g_LinkMOEDlg = NULL;
CDlgPathMOE	*g_pPathMOEDlg = NULL;

extern 	  std::list<CView*>	g_ViewList;
extern float g_Simulation_Time_Stamp;

bool g_LinkMOEDlgShowFlag = false;
std::list<DTALink*>	g_LinkDisplayList;


// CTLiteDoc

IMPLEMENT_DYNCREATE(CTLiteDoc, CDocument)

BEGIN_MESSAGE_MAP(CTLiteDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, &CTLiteDoc::OnFileOpen)
	ON_COMMAND(ID_TOOL_GENERATESENESORMAPPINGTABLE, &CTLiteDoc::OnToolGeneratesenesormappingtable)
	ON_COMMAND(ID_SHOW_SHOWPATHMOE, &CTLiteDoc::OnShowShowpathmoe)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SHOWPATHMOE, &CTLiteDoc::OnUpdateShowShowpathmoe)
	ON_COMMAND(ID_SEARCH_LISTTRAINS, &CTLiteDoc::OnSearchListtrains)
	ON_COMMAND(ID_TIMETABLE_IMPORTTIMETABLE, &CTLiteDoc::OnTimetableImporttimetable)
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
	ON_COMMAND(ID_MOE_VEHICLE, &CTLiteDoc::OnMoeVehicle)
	ON_UPDATE_COMMAND_UI(ID_MOE_VEHICLE, &CTLiteDoc::OnUpdateMoeVehicle)
	ON_COMMAND(ID_TOOLS_VIEWSIMULATIONSUMMARY, &CTLiteDoc::OnToolsViewsimulationsummary)
	ON_COMMAND(ID_TOOLS_VIEWASSIGNMENTSUMMARYLOG, &CTLiteDoc::OnToolsViewassignmentsummarylog)
	ON_COMMAND(ID_HELP_VISITDEVELOPMENTWEBSITE, &CTLiteDoc::OnHelpVisitdevelopmentwebsite)
	ON_COMMAND(ID_TOOLS_RUNTRAFFICASSIGNMENT, &CTLiteDoc::OnToolsRuntrafficassignment)
	ON_COMMAND(ID_IMPORTODTRIPFILE_3COLUMNFORMAT, &CTLiteDoc::OnImportodtripfile3columnformat)
	ON_COMMAND(ID_TOOLS_PERFORMSCHEDULING, &CTLiteDoc::OnToolsPerformscheduling)
	ON_COMMAND(ID_FILE_CHANGECOORDINATESTOLONG, &CTLiteDoc::OnFileChangecoordinatestolong)
	ON_COMMAND(ID_FILE_OPENRAILNETWORKPROJECT, &CTLiteDoc::OnFileOpenrailnetworkproject)
	ON_COMMAND(ID_TOOLS_EXPORTOPMODEDISTRIBUTION, &CTLiteDoc::OnToolsExportopmodedistribution)
	ON_COMMAND(ID_TOOLS_ENUMERATEPATH, &CTLiteDoc::OnToolsEnumeratepath)
	ON_COMMAND(ID_TOOLS_EXPORTTOTIME, &CTLiteDoc::OnToolsExporttoHistDatabase)
	ON_COMMAND(ID_RESEARCHTOOLS_EXPORTTODTALITESENSORDATAFORMAT, &CTLiteDoc::OnResearchtoolsExporttodtalitesensordataformat)
	ON_COMMAND(ID_SCENARIO_CONFIGURATION, &CTLiteDoc::OnScenarioConfiguration)
	ON_COMMAND(ID_MOE_VIEWMOES, &CTLiteDoc::OnMoeViewmoes)
	ON_COMMAND(ID_IMPORTDATA_IMPORT, &CTLiteDoc::OnImportdataImport)
END_MESSAGE_MAP()


// CTLiteDoc construction/destruction


CTLiteDoc::~CTLiteDoc()
{
	m_WarningFile.close();

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



void CTLiteDoc::ReadSimulationLinkMOEData(LPCTSTR lpszFileName)
{
	FILE* st = NULL;

	// reopen
	fopen_s(&st,lpszFileName,"r");

	if(st!=NULL)
	{

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);  // use one day horizon as the default value
		}

		int i = 0;

		while(!feof(st))
		{
			// from_node_id, to_node_id, timestamp_in_min, travel_time_in_min, delay_in_min, link_volume_in_veh, link_volume_in_vehphpl,
			//density_in_veh_per_mile_per_lane, speed_in_mph, queue_length_in_, cumulative_arrival_count, cumulative_departure_count
			int from_node_number = g_read_integer(st);
			if(from_node_number < 0)
				break;
			int to_node_number =  g_read_integer(st);
			int t = g_read_float(st) + m_SimulationStartTime_in_min;


			DTALink* pLink = FindLinkWithNodeNumbers(from_node_number , to_node_number );

			ASSERT(t<=g_Simulation_Time_Horizon);
			if(pLink!=NULL)
			{
				pLink->m_LinkMOEAry[t].ObsTravelTimeIndex = g_read_float(st);
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
				CString msg;
				msg.Format ("Please check if line %d at file %s has a consistent link definition with input_link.csv.", i+2, lpszFileName);  // +2 for the first field name line
				AfxMessageBox(msg);
				break;
			}

		}

		g_Simulation_Time_Stamp = 0; // reset starting time
		g_SimulationStartTime_in_min = 0;

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
					pLink->m_StaticLaneVolume = g_read_float(st);
					pLink->m_StaticVOC   = g_read_float(st);
					float FreeflowTravelTime   = g_read_float(st);
					pLink->m_StaticTravelTime = g_read_float(st);
					pLink->m_StaticSpeed  = g_read_float(st);
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

	CWaitCursor wc;
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

				DTALink* pLink = m_LinkNoMap[LinkID];

				if(pLink!=NULL)
				{
					pLink->ResetMOEAry(g_Simulation_Time_Horizon);

					int Interval=5;

					int t  = ((day_of_year-FirstDay)*1440+ hour*60+min)/g_Data_Time_Interval;

					if(t<(*iLink)->m_SimulationHorizon)
					{

						if(AvgSpeed<=1)  // 0 or negative values means missing speed
							AvgSpeed = pLink->m_SpeedLimit ;

						ASSERT(pLink->m_NumLanes > 0);
						pLink->m_LinkMOEAry[ t].ObsFlow = TotalFlow*12/pLink->m_NumLanes;  // convert to per hour link flow
						pLink->m_LinkMOEAry[ t].ObsSpeed = AvgSpeed; 
						pLink->m_LinkMOEAry[ t].ObsTravelTimeIndex = pLink->m_Length/max(1,AvgSpeed);
						pLink->m_LinkMOEAry[t].ObsDensity = pLink->m_LinkMOEAry[t].ObsFlow / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeed);


						if(t>=1)
						{
							pLink->m_LinkMOEAry[t].ObsCumulativeFlow = pLink->m_LinkMOEAry[t-1].ObsCumulativeFlow + TotalFlow/12;
						}

					}

				}	
			}
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
				DTALink* pLink = m_LinkNoMap[LinkID];

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

					//                      TRACE("reading: %d, %f\n", LinkNo, Flow);

					if(pLink!=NULL)
					{
						pLink->m_LinkMOEAry[ t].ObsFlow = Flow*12/pLink->m_NumLanes;  // convert to per hour link flow


						if(t-Interval>0)
						{
							pLink->m_LinkMOEAry[t].ObsCumulativeFlow = pLink->m_LinkMOEAry[t-Interval].ObsCumulativeFlow + Flow/12;
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
				DTALink* pLink = m_LinkNoMap[LinkID];

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

					//                      TRACE("reading: %d, %f\n", LinkNo, Speed);

					if(pLink!=NULL)
					{
						pLink->m_LinkMOEAry[t].ObsSpeed  = Speed*0.621371192f;  // km/h -> mph
						pLink->m_LinkMOEAry[ t].ObsTravelTimeIndex = pLink->m_Length/max(1,pLink->m_LinkMOEAry[t].ObsSpeed);
						pLink->m_LinkMOEAry[t].ObsDensity = pLink->m_LinkMOEAry[t].ObsFlow / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeed);

						// copy data to other intervals
						for(int tt = 1; tt<Interval; tt++)
						{
							pLink->m_LinkMOEAry[t+tt].ObsSpeed = pLink->m_LinkMOEAry[t].ObsSpeed;
							pLink->m_LinkMOEAry[t+tt].ObsTravelTimeIndex = pLink->m_LinkMOEAry[t].ObsTravelTimeIndex ;
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



BOOL CTLiteDoc::OnOpenTrafficNetworkDocument(LPCTSTR lpszPathName)
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
	OpenWarningLogFile(directory);

	if(!ReadNodeCSVFile(directory+"input_node.csv")) return false;
	if(!ReadLinkCSVFile(directory+"input_link.csv")) return false;

	ReadScenarioData();

	CalculateDrawingRectangle();
	OffsetLink();

	//: comment out now, it uses alternative format	

	//	ReadSensorLocationData(directory+"input_sensor_location.csv");
	//	ReadHistoricalData(directory);

	if(ReadZoneCSVFile(directory+"input_zone.csv"))
	{
		ReadDemandCSVFile(directory+"input_demand.csv");
		LoadSimulationOutput();
	}


	if(ReadSensorLocationData(directory+"input_sensor_location.csv") == true)
	{
		CWaitCursor wc;
		ReadSensorData(directory);   // if there are sensor location data
		ReadEventData(directory); 
		BuildHistoricalDatabase();
	}

	//ReadObservationLinkVolumeData(directory+"input_static_obs_link_volume.csv");

	ReadBackgroundImageFile(directory+"Background.bmp");
	return true;
}
BOOL CTLiteDoc::OnOpenRailNetworkDocument(LPCTSTR lpszPathName)
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

	OpenWarningLogFile(directory);

	if(!ReadNodeCSVFile(directory+"input_node.csv")) return false;
	if(!ReadLinkCSVFile(directory+"input_link.csv")) return false;

	ReadScenarioData();

	CalculateDrawingRectangle();

	m_AdjLinkSize +=2;  // add two more elements to be safe

	std::list<DTANode*>::iterator iNode;
	//adjust XY coordinates if the corrdinate system is not consistenty
	if(fabs(m_UnitMile-1.00)>0.10)  // ask users if we need to adjust the XY coordinates
	{
		if(AfxMessageBox("The link length information in link.csv is not consistent with the X/Y coordinates in node.csv.\nDo you want to adjust the the X/Y coordinates in node.csv?", MB_YESNO) == IDYES)

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

	// for train timetabling
	ReadTrainProfileCSVFile(directory+"input_train_link_running_time.csv");
	ReadTimetableCVSFile(directory+"output_timetable.csv");

	ReadBackgroundImageFile(directory+"Background.bmp");


	return true;
}

BOOL CTLiteDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	CWaitCursor wait;
	g_VisulizationTemplate = e_traffic_assignment;

	OnOpenTrafficNetworkDocument(lpszPathName);

	CDlgFileLoading dlg;
	dlg.m_pDoc = this;
	dlg.DoModal ();

	UpdateAllViews(0);
	return true;

}

void CTLiteDoc::ReadBackgroundImageFile(LPCTSTR lpszFileName)
{
	//read impage file Background.bmp

	if(m_BackgroundBitmapLoaded)
		m_BackgroundBitmap.Detach ();

	m_BackgroundBitmap.Load(lpszFileName);

	m_BackgroundBitmapLoaded = !(m_BackgroundBitmap.IsNull ());
	//	m_BackgroundBitmapLoaded = true;


	if(m_BackgroundBitmapLoaded)
	{
		TCHAR IniFilePath[_MAX_PATH];
		sprintf_s(IniFilePath,"%s//in_opt_BackgroundImage.ini", m_ProjectDirectory);

		m_ImageX1 = g_GetPrivateProfileFloat("BackgroundImage", "x1", m_NetworkRect.left, IniFilePath);

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

	TCHAR IniFilePath[_MAX_PATH];
	sprintf_s(IniFilePath,"%s//in_opt_BackgroundImage.ini", m_ProjectDirectory);

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

	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Station location file (*.csv)|*.csv|"));
	if(dlg.DoModal() == IDOK)
	{
		fopen_s(&st,dlg.GetPathName(),"r");
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

					m_LinkNoMap[SensorLinkID]->m_bSensorData  = true;


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


		fopen_s(&st,directory+"input_sensor_location.csv","w");
		if(st!=NULL)
		{
			std::vector<DTA_sensor>::iterator iSensor;
			fprintf(st,"from_node_id,to_node_id,sensor_type,sensor_id\n");
			for (iSensor = m_SensorVector.begin(); iSensor != m_SensorVector.end(); iSensor++)
			{
				if((*iSensor).LinkID>=0)
				{
					DTALink* pLink = m_LinkNoMap[(*iSensor).LinkID];
					fprintf(st,"%d,%d,0,%d\n", pLink->m_FromNodeNumber, pLink->m_ToNodeNumber, (*iSensor).OrgSensorID);
				}
			}

			fclose(st);

		}
	}

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
	g_LinkMOEDlgShowFlag = !g_LinkMOEDlgShowFlag;

	if(g_LinkMOEDlgShowFlag)
	{
		if(g_LinkMOEDlg==NULL)
		{
			g_LinkMOEDlg = new CDlgMOE();
			g_LinkMOEDlg->m_pDoc = this;
			g_LinkMOEDlg->m_TmLeft = 0 ;
			g_LinkMOEDlg->m_TmRight = g_Simulation_Time_Horizon ;

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
	CCSVParser parser;
	if (parser.OpenCSVFile(lpszFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int node_id;
			string name;
			DTANode* pNode = 0;

			int control_type;
			double X;
			double Y;
			if(parser.GetValueByFieldName("node_id",node_id) == false)
				break;

			if(!parser.GetValueByFieldName("name",name))
				name = "";

			if(!parser.GetValueByFieldName("control_type",control_type))
				control_type = 0;

			// use the X and Y as default values first
			bool bFieldX_Exist = parser.GetValueByFieldName("x",X);
			parser.GetValueByFieldName("y",Y);

			string geo_string;

			if(parser.GetValueByFieldName("geometry",geo_string))
			{
				// overwrite when the field "geometry" exists
				CGeometry geometry(geo_string);

				std::vector<CCoordinate> CoordinateVector = geometry.GetCoordinateList();

				if(CoordinateVector.size()>0)
				{
				X = CoordinateVector[0].X;
				Y = CoordinateVector[0].Y;
				}

			}

			pNode = new DTANode;
			pNode->m_Name = name;
			pNode->m_ControlType = control_type;
			pNode->pt.x = X;
			pNode->pt.y = Y;

			pNode->m_NodeNumber = node_id;
			pNode->m_NodeID = i;
			pNode->m_ZoneID = 0;
			m_NodeSet.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeIDtoNameMap[i] = node_id;
			m_NodeNametoIDMap[node_id] = i;
			i++;

			//			cout << "node = " << node << ", X= " << X << ", Y = " << Y << endl;

		}

		m_NodeDataLoadingStatus.Format ("%d nodes are loaded from file %s.",m_NodeSet.size(),lpszFileName);
		return true;
	}else
	{
		AfxMessageBox("Error: File input_node.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

}

void CTLiteDoc::OffsetLink()
{
	if(m_bLinkShifted)
	{
		double link_offset = m_UnitFeet*80;  // 80 feet

		std::list<DTALink*>::iterator iLink;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			int last_shape_point_id = (*iLink) ->m_ShapePoints .size() -1;
			double DeltaX = (*iLink)->m_ShapePoints[last_shape_point_id].x - (*iLink)->m_ShapePoints[0].x;
			double DeltaY = (*iLink)->m_ShapePoints[last_shape_point_id].y - (*iLink)->m_ShapePoints[0].y;
			double theta = atan2(DeltaY, DeltaX);

			for(int si = 0; si < (*iLink) ->m_ShapePoints .size(); si++)
			{
				(*iLink)->m_ShapePoints[si].x += link_offset* cos(theta-PI/2.0f);
				(*iLink)->m_ShapePoints[si].y += link_offset* sin(theta-PI/2.0f);
			}

		}
	}

}

bool CTLiteDoc::ReadLinkCSVFile(LPCTSTR lpszFileName)
{

	long i = 0;
	DTALink* pLink = 0;
	float default_distance_sum=0;
	float length_sum = 0;
	CCSVParser parser;
	if (parser.OpenCSVFile("input_link.csv"))
	{
		bool bNodeNonExistError = false;
		while(parser.ReadRecord())
		{
			int link_id;
			int from_node_id;
			int to_node_id;
			int direction;
			double length_in_mile;
			int number_of_lanes=0;
			int speed_limit_in_mph=0;
			double capacity=0;
			int type;
			string name;
			double k_jam,wave_speed_in_mph;
			string mode_code;


			if(!parser.GetValueByFieldName("name",name))
				name = "";

			if(!parser.GetValueByFieldName("from_node_id",from_node_id)) 
			{
				AfxMessageBox("Field from_node_id has not been defined in file input_link.csv. Please check.");
				break;
			}
			if(!parser.GetValueByFieldName("to_node_id",to_node_id))
			{
				AfxMessageBox("Field to_node_id has not been defined in file input_link.csv. Please check.");
				break;
			}


				if(m_NodeNametoIDMap.find(from_node_id)== m_NodeNametoIDMap.end())
				{
					m_WarningFile<< "From Node Number "  << from_node_id << " in input_link.csv has not be defined in input_node.csv"  << endl; 
					bNodeNonExistError = true;
					break;
				}

				if(m_NodeNametoIDMap.find(to_node_id)== m_NodeNametoIDMap.end())
				{
					m_WarningFile<< "To Node Number "  << to_node_id << " in input_link.csv has not be defined in input_node.csv"  << endl; 
					bNodeNonExistError = true;
					break;

				}


			if(!parser.GetValueByFieldName("direction",direction))
					direction = 1;

			if(!parser.GetValueByFieldName("length_in_mile",length_in_mile))
			{
				AfxMessageBox("Field length_in_mile has not been defined in file input_link.csv. Please check.");
				break;
			}

			if(!parser.GetValueByFieldName("number_of_lanes",number_of_lanes))
			{
				AfxMessageBox("Field number_of_lanes has not been defined in file input_link.csv. Please check.");
				break;
			}

			if(!parser.GetValueByFieldName("speed_limit_in_mph",speed_limit_in_mph))
			{
				AfxMessageBox("Field speed_limit_in_mph has not been defined in file input_link.csv. Please check.");
				break;
			}

			if(!parser.GetValueByFieldName("lane_capacity_in_vhc_per_hour",capacity))
			{
				AfxMessageBox("Field lane_capacity_in_vhc_per_hour has not been defined in file input_link.csv. Please check.");
				break;
			}

			if(!parser.GetValueByFieldName("link_type",type))
			{
				AfxMessageBox("Field link_type has not been defined in file input_link.csv. Please check.");
				break;
			}

			if(!parser.GetValueByFieldName("jam_density_in_vhc_pmpl",k_jam))
				k_jam = 180;

			if(!parser.GetValueByFieldName("wave_speed_in_mph",wave_speed_in_mph))
				wave_speed_in_mph = 12;

			if(!parser.GetValueByFieldName("mode_code",mode_code))
				mode_code  = "";

			if(!parser.GetValueByFieldName("link_id",link_id))
				link_id = 0;

			string geo_string;

			std::vector<CCoordinate> CoordinateVector;
			if(parser.GetValueByFieldName("geometry",geo_string))
			{
				// overwrite when the field "geometry" exists
				CGeometry geometry(geo_string);
				CoordinateVector = geometry.GetCoordinateList();
				m_bLinkShifted = false;
			}else
			{
				// no geometry information
				CCoordinate cc_from, cc_to; 
				cc_from.X = m_NodeIDMap[m_NodeNametoIDMap[from_node_id]]->pt.x;
				cc_from.Y = m_NodeIDMap[m_NodeNametoIDMap[from_node_id]]->pt.y;

				cc_to.X = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]]->pt.x;
				cc_to.Y = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]]->pt.y;

				CoordinateVector.push_back(cc_from);
				CoordinateVector.push_back(cc_to);
			}

			int link_code_start = 1;
			int link_code_end = 1;

			if (direction == -1) // reversed
			{
				link_code_start = 2; link_code_end = 2;
			}

			if (direction == 0) // two-directional link
			{
				link_code_start = 1; link_code_end = 2;
			}

			for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
			{

				bool bNodeNonExistError = false;
				int m_SimulationHorizon = 1;
				pLink = new DTALink(m_SimulationHorizon);
				pLink->m_LinkNo = i;
				pLink->m_Name  = name;
				pLink->m_OrgDir = direction;
				pLink->m_LinkID = link_id;

				if(link_code == 1)  //AB link
				{
					pLink->m_FromNodeNumber = from_node_id;

					pLink->m_ToNodeNumber = to_node_id;
					pLink->m_Direction  = 1;

					for(int si = 0; si < CoordinateVector.size(); si++)
					{
					GDPoint	pt;
					pt.x = CoordinateVector[si].X;
					pt.y = CoordinateVector[si].Y;
					pLink->m_ShapePoints .push_back (pt);
					}

				}

				if(link_code == 2)  //BA link
				{
					pLink->m_FromNodeNumber = to_node_id;
					pLink->m_ToNodeNumber = from_node_id;
					pLink->m_Direction  = 1;

					for(int si = CoordinateVector.size()-1; si >=0; si--)
					{
					GDPoint	pt;
					pt.x = CoordinateVector[si].X;
					pt.y = CoordinateVector[si].Y;
					pLink->m_ShapePoints .push_back (pt);
					}
				}


				pLink->m_FromNodeID = m_NodeNametoIDMap[from_node_id];
				pLink->m_ToNodeID= m_NodeNametoIDMap[to_node_id];

				m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
				m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;

				unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
				m_NodeIDtoLinkMap[LinkKey] = pLink;
				m_NodeIDtoLinkMap[link_id] = pLink;

				pLink->m_NumLanes= number_of_lanes;
				pLink->m_SpeedLimit= speed_limit_in_mph;
				pLink->m_StaticSpeed = pLink->m_SpeedLimit;

				pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance
				pLink->m_MaximumServiceFlowRatePHPL= capacity;
				pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
				pLink->m_link_type= type;

				pLink->m_Kjam = k_jam;
				pLink->m_Wave_speed_in_mph  = wave_speed_in_mph;

				m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);


				pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
				pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;


				default_distance_sum+= pLink->DefaultDistance();
				length_sum += pLink ->m_Length;
				//			pLink->SetupMOE();
			}

			if(!bNodeNonExistError)
			{
				m_LinkSet.push_back (pLink);
				m_LinkNoMap[i]  = pLink;
				i++;
			}

		}

		if(bNodeNonExistError)
			AfxMessageBox("Some nodes in input_link.csv have not been defined in input_node.csv. Please check warning.log in the project folder.");

		m_UnitMile  = 1.0f;

		if(length_sum>0.000001f)
			m_UnitMile= length_sum / default_distance_sum ;

		m_UnitFeet = m_UnitMile/5280.0f;  


		if(m_UnitMile>50)  // long/lat must be very large and greater than 62!
		{
			if(AfxMessageBox("Is the long/lat coordinate system used in this data set?", MB_YESNO) == IDYES)
			{
				m_LongLatCoordinateFlag = true;
				m_UnitFeet = m_UnitMile/62/5280.0f;  // 62 is 1 long = 62 miles
			}
		}

		m_LinkDataLoadingStatus.Format ("%d links are loaded from file %s.",m_LinkSet.size(),lpszFileName);

		return true;
	}else
	{
		AfxMessageBox("Error: File input_link.csv cannot be opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

}

bool CTLiteDoc::ReadZoneCSVFile(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	bool bNodeNonExistError = false;
	m_NodeIDtoZoneNameMap.clear ();
	if(m_DemandMatrix!=NULL)
	{
		DeallocateDynamicArray<float>(m_DemandMatrix,m_ODSize,m_ODSize);
		m_DemandMatrix = NULL;
	}


	int lineno = 0 ;

	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int zone_number;
	
			if(parser.GetValueByFieldName("zone_id",zone_number) == false)
				break;

			int node_name;
			if(parser.GetValueByFieldName("node_id",node_name) == false)
				break;

			map <int, int> :: const_iterator m_Iter = m_NodeNametoIDMap.find(node_name);

			if(m_Iter == m_NodeNametoIDMap.end( ))
			{
				m_WarningFile<< "Node Number "  << node_name << " in input_zone.csv has not be defined in input_node.csv"  << endl; 
				bNodeNonExistError = true;

			}
			m_NodeIDtoZoneNameMap[m_NodeNametoIDMap[node_name]] = zone_number;
			// if there are multiple nodes for a zone, the last node id is recorded.
			int zoneid  = zone_number-1;
			m_ZoneIDtoNodeIDMap[zoneid] = m_NodeNametoIDMap[node_name];

			if(m_ODSize < zone_number)
				m_ODSize = zone_number;

			lineno++;
		}

		if(bNodeNonExistError)
			AfxMessageBox("Some nodes in input_zone.csv have not been defined in input_node.csv. Please check warning.log in the project folder.");

		m_ZoneDataLoadingStatus.Format ("%d node-zone mapping entries are loaded from file %s.",lineno,lpszFileName);
		return true;
	}else
	{
		m_ZoneDataLoadingStatus.Format ("File input_zone.csv does not exist. Use default node-zone mapping table.");

		return false;
	}

	m_ZoneVector.resize(m_ODSize+1);
}

bool CTLiteDoc::ReadDemandCSVFile(LPCTSTR lpszFileName)
{

	long lineno = 0;
	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
		if(m_DemandMatrix!=NULL)
		{
			DeallocateDynamicArray(m_DemandMatrix,m_ODSize,m_ODSize);
			m_DemandMatrix = NULL;
		}

		m_DemandMatrix   =  AllocateDynamicArray<float>(m_ODSize,m_ODSize);

		for(int i= 0; i<m_ODSize; i++)
			for(int j= 0; j<m_ODSize; j++)
			{
				m_DemandMatrix[i][j]= 0.0f;
			}

			m_MaxODDemand  = 1;


			int i=0;
		while(parser.ReadRecord())
		{
			int origin_zone_id, destination_zone_id;
			float number_of_vehicles, starting_time_in_min, ending_time_in_min;

			if(parser.GetValueByFieldName("from_zone_id",origin_zone_id) == false)
				break;
			if(parser.GetValueByFieldName("to_zone_id",destination_zone_id) == false)
				break;

			if(parser.GetValueByFieldName("number_of_vehicles",number_of_vehicles) == false)
				break;

			if(parser.GetValueByFieldName("starting_time_in_min",starting_time_in_min) == false)
				break;

			if(parser.GetValueByFieldName("ending_time_in_min",ending_time_in_min) == false)
				break;

			m_DemandMatrix[origin_zone_id][destination_zone_id] += number_of_vehicles;

				if(m_MaxODDemand < number_of_vehicles)
					m_MaxODDemand =  number_of_vehicles ;

				lineno++;
			}

			m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file %s.",lineno,lpszFileName);
			return true;
	}else
	{
		//		AfxMessageBox("Error: File input_demand.csv cannot be found or opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

}
bool CTLiteDoc::Read3ColumnTripTxtFile(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	long lineno = 1;
	if(st!=NULL)
	{
		if(m_DemandMatrix!=NULL)
		{
			DeallocateDynamicArray(m_DemandMatrix,m_ODSize,m_ODSize);
			m_DemandMatrix = NULL;
		}

		m_DemandMatrix   =  AllocateDynamicArray<float>(m_ODSize,m_ODSize);

		for(int i= 0; i<m_ODSize; i++)
			for(int j= 0; j<m_ODSize; j++)
			{
				m_DemandMatrix[i][j]= 0.0f;
			}

			m_MaxODDemand  = 1;

			while(!feof(st))
			{
				int origin_zone	       = g_read_integer(st);
				if(origin_zone == -1)  // reach end of file
					break;

				int destination_zone	   = g_read_integer(st);  // we might read "Origin" here, so if we find "O", we return 0 as a key to jump to the next origin.

				if(origin_zone > m_ODSize)
				{
					CString msg;
					msg.Format ("Error at line %d: origin zone number = %d, which is greater than the number of OD zones: %d.",lineno,origin_zone, m_ODSize);
					break;
				}

				if(destination_zone > m_ODSize)
				{
					CString msg;
					msg.Format ("Error at line %d: destination zone number = %d, which is greater than the number of OD zones: %d.",lineno,origin_zone, m_ODSize);
					break;
				}

				float number_of_vehicles = g_read_float(st);

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
		AfxMessageBox("Error: OD Trip file cannot be opened.");
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
		if(m_DemandMatrix!=NULL)
		{
			DeallocateDynamicArray(m_DemandMatrix,m_ODSize,m_ODSize);
			m_DemandMatrix = NULL;
		}

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

bool CTLiteDoc::ReadScenarioData()
{
	FILE* st = NULL;
	int i =0;
	fopen_s(&st,m_ProjectDirectory+"Incident.xml","r");
	if(st!=NULL)
	{
		int XML_version = g_read_float(st);

		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn );

			if(plink!=NULL)
			{
				CapacityReduction cs;

				cs.StartTime = g_read_integer(st);
				cs.EndTime = g_read_integer(st);
				cs.LaneClosureRatio= g_read_float(st);
				plink->CapacityReductionVector.push_back(cs);
				i++;
			}
		}


		fclose(st);
	}
	
//  Dynamic Message Sign
	fopen_s(&st,m_ProjectDirectory+"Dynamic Message Sign.xml","r");
	if(st!=NULL)
	{
		int XML_version = g_read_float(st);

		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn );

			if(plink!=NULL)
			{
				MessageSign ms;

				ms.StartTime = g_read_integer(st);
				ms.EndTime = g_read_integer(st);
				ms.ResponsePercentage= g_read_float(st);
				plink->MessageSignVector.push_back(ms);
				i++;
			}
		}

		fclose(st);
	}
// toll
	fopen_s(&st,m_ProjectDirectory+"Link Based Toll.xml","r");
	if(st!=NULL)
	{
		int XML_version = g_read_float(st);

		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn );

			if(plink!=NULL)
			{
				Toll tl;

				tl.StartTime = g_read_integer(st);
				tl.EndTime = g_read_integer(st);
				tl.TollRateInMin[0]= g_read_float(st);
				tl.TollRateInMin[1]= g_read_float(st);
				tl.TollRateInMin[2]= g_read_float(st);
				plink->TollVector.push_back(tl);
				i++;
			}
		}

		fclose(st);
	}
	
// toll
	fopen_s(&st,m_ProjectDirectory+"Distance Based_Toll.xml","r");
	if(st!=NULL)
	{
		int XML_version = g_read_float(st);

		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn );

			if(plink!=NULL)
			{
				Toll tl;

				tl.StartTime = g_read_integer(st);
				tl.EndTime = g_read_integer(st);
				tl.TollRate[0]= g_read_float(st);
				tl.TollRate[1]= g_read_float(st);
				tl.TollRate[2]= g_read_float(st);
				plink->TollVector.push_back(tl);
				i++;
			}
		}

		fclose(st);
	}
		m_LinkDataLoadingStatus.Format ("%d scenario records are loaded.",i);

	return true;
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

	////
	CWaitCursor wc;

	CString OldDirectory = m_ProjectDirectory;

	if(OldDirectory!=directory)
	{
		CopyFile(OldDirectory+"input_node.csv", directory+"input_node.csv", FALSE);
		CopyFile(OldDirectory+"input_link.csv", directory+"input_link.csv", FALSE);
		CopyFile(OldDirectory+"input_zone.csv", directory+"input_zone.csv", FALSE);
		CopyFile(OldDirectory+"input_demand.csv", directory+"input_demand.csv", FALSE);
		CopyFile(OldDirectory+"DTASettings.ini", directory+"DTASettings.ini", FALSE);
		CopyFile(OldDirectory+"LinkMOE.csv", directory+"LinkMOE.csv", FALSE);
		CopyFile(OldDirectory+"LinkStaticMOE.csv", directory+"LinkStaticMOE.csv", FALSE);
		CopyFile(OldDirectory+"NetworkMOE.csv", directory+"NetworkMOE.csv", FALSE);
		CopyFile(OldDirectory+"summary.log", directory+"summary.log", FALSE);
		CopyFile(OldDirectory+"assignment.log", directory+"assignment.log", FALSE);
		CopyFile(OldDirectory+"Vehicle.csv", directory+"Vehicle.csv", FALSE);
		CopyFile(OldDirectory+"error.log", directory+"error.log", FALSE);
		CopyFile(OldDirectory+"warning.log", directory+"warning.log", FALSE);

	}


	// update m_ProjectDirectory
	m_ProjectDirectory = directory;


	fopen_s(&st,lpszPathName,"w");
	if(st!=NULL)
	{
		fprintf(st,"This project include node.csv and link.csv.");
		fclose(st);
	}else
	{
		AfxMessageBox("Error in writing the project file. Please check if the file is opened by another project or the folder is read-only.");
	}

	fopen_s(&st,directory+"input_node.csv","w");
	if(st!=NULL)
	{
		std::list<DTANode*>::iterator iNode;
		fprintf(st, "name,node_id,control_type,geometry\n");
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			fprintf(st, "%s,%d,%d,\"<Point><coordinates>%f,%f</coordinates></Point>\"\n", (*iNode)->m_Name.c_str (), (*iNode)->m_NodeNumber , (*iNode)->m_ControlType, (*iNode)->pt .x, (*iNode)->pt .y );
		}

		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_node.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}

	fopen_s(&st,directory+"input_link.csv","w");
	if(st!=NULL)
	{
		std::list<DTALink*>::iterator iLink;
		fprintf(st,"name,link_id,from_node_id,to_node_id,direction,length_in_mile,number_of_lanes,speed_limit_in_mph,lane_capacity_in_vhc_per_hour,link_type,jam_density_in_vhc_pmpl,wave_speed_in_mph,mode_code,geometry\n");
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			fprintf(st,"%s,%d,%d,%d,%d,%f,%d,%f,%f,%d,%f,%f,%s,",
				(*iLink)->m_Name.c_str (),
				(*iLink)->m_LinkID, 
				(*iLink)->m_FromNodeNumber, 
				(*iLink)->m_ToNodeNumber , (*iLink)->m_Direction,(*iLink)->m_Length ,(*iLink)->m_NumLanes ,(*iLink)->m_SpeedLimit,(*iLink)->m_LaneCapacity ,(*iLink)->m_link_type,(*iLink)->m_Kjam, (*iLink)->m_Wave_speed_in_mph,(*iLink)->m_Mode_code.c_str ());
			fprintf(st,"\"<LineString><coordinates>");
				
			for(int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
			{
			fprintf(st,"%f,%f,0.0",(*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y);

			if(si!=(*iLink)->m_ShapePoints.size()-1)
				fprintf(st," ");
			}

			fprintf(st,"<LineString><coordinates>\"\n");
		}
		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_link.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}
	// save in_demand here

	if(m_BackgroundBitmapImportedButnotSaved)
	{
		m_BackgroundBitmap.Save(directory+"background.bmp",Gdiplus::ImageFormatBMP);  // always use bmp format
		m_BackgroundBitmapImportedButnotSaved = false;
	}

	OnFileSaveimagelocation();


	// Copy demand_HOV.dat
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
			msg.Format ("Files input_node.csv and input_link.csv have been successfully saved with %d nodes, %d links.",m_NodeSet.size(), m_LinkSet.size());
			AfxMessageBox(msg,MB_OK|MB_ICONINFORMATION);

			m_ProjectFile = fdlg.GetPathName();

			m_ProjectTitle = GetWorkspaceTitleName(m_ProjectFile);
			SetTitle(m_ProjectTitle);


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
			int m_VehicleID= g_read_integer(st);
			if(m_VehicleID == -1)
				break;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_VehicleID		= m_VehicleID;
			pVehicle->m_OriginZoneID	= g_read_integer(st);
			pVehicle->m_DestinationZoneID=g_read_integer(st);
			pVehicle->m_DepartureTime	= m_SimulationStartTime_in_min + g_read_float(st);
			pVehicle->m_ArrivalTime = m_SimulationStartTime_in_min + g_read_float(st);

			if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
				g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

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
					pVehicle->m_NodeAry[i].LinkID = pLink->m_LinkNo ;
				}

				// random error beyond 6 seconds for better ainimation

				float random_value = g_RNNOF()*0.01; // 0.1 min = 6 seconds


				pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = m_SimulationStartTime_in_min + g_read_float(st);
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

	CDlg_ImageSettings dlg_image;
	dlg_image.m_RealworldWidth = m_NetworkRect.right;
	dlg_image.m_RealworldHeight = m_NetworkRect.top;

	if(dlg_image.DoModal ()==IDOK)
	{
		m_NetworkRect.left  = 0;
		m_NetworkRect.bottom = 0;

		m_NetworkRect.right = dlg_image.m_RealworldWidth;
		m_NetworkRect.top = dlg_image.m_RealworldHeight;
		m_ImageWidth = dlg_image.m_RealworldWidth;
		m_ImageHeight = dlg_image.m_RealworldHeight;
		m_ImageXResolution = 1;
		m_ImageYResolution = 1;

		m_ImageX1 = 0;
		m_ImageY1 = 0;
		m_ImageX2 = dlg_image.m_RealworldWidth;
		m_ImageY2 = dlg_image.m_RealworldHeight;

	}

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

float CTLiteDoc::GetTDLinkMOE(DTALink* pLink, Link_MOE LinkMOEMode,int CurrentTime,float &value)
{

	float power = 0.0f;
	float max_link_volume = 8000.0f;
	float max_speed_ratio = 2.0f; 
	float max_density = 45.0f;

	switch (LinkMOEMode)
	{
	case volume:  power = pLink->m_StaticLaneVolume/max_link_volume; 
		value = pLink->m_StaticLaneVolume;
		break;
	case speed:   power = pLink->m_SpeedLimit / max(1, pLink->m_StaticSpeed)/max_speed_ratio; 
		value = pLink->m_StaticSpeed;
		break;
	case vcratio: power = pLink->m_StaticVOC;
		value = pLink->m_StaticVOC;
		break;
	case traveltime:  power = pLink->m_SpeedLimit / max(1, pLink->m_StaticSpeed)/max_speed_ratio; 
		value = pLink->m_StaticTravelTime;
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

	if(!m_StaticAssignmentMode)
	{
		if(pLink->m_SimulationHorizon > CurrentTime && CurrentTime >=1 && CurrentTime < pLink->m_SimulationHorizon)  //DTAoutput
		{
			switch (LinkMOEMode)
			{
			case volume:  power = pLink->m_LinkMOEAry[CurrentTime].ObsFlow* pLink->m_NumLanes/max_link_volume;
				value = pLink->m_LinkMOEAry[CurrentTime].ObsFlow* pLink->m_NumLanes;
				break;
			case speed:   power = pLink->m_SpeedLimit / max(1, pLink->m_LinkMOEAry[CurrentTime].ObsSpeed)/max_speed_ratio;
				value = pLink->m_LinkMOEAry[CurrentTime].ObsSpeed;
				break;
			case vcratio: power = pLink->m_LinkMOEAry[CurrentTime].ObsFlow/pLink->m_LaneCapacity;
				value = power;
				break;
			case traveltime:
				if(pLink->m_LinkMOEAry [CurrentTime].ObsTravelTimeIndex <=0.1)  // no data
					power = 0;
				else 
					power = pLink->m_SpeedLimit / pLink->m_LinkMOEAry [CurrentTime].ObsSpeed/max_speed_ratio; 

				value = pLink->m_LinkMOEAry [CurrentTime].ObsTravelTimeIndex;
				break;
			case density: power = pLink->m_LinkMOEAry[CurrentTime].ObsDensity /max_density; 
				value = pLink->m_LinkMOEAry[CurrentTime].ObsDensity;
				break;

			}
		}
	}

	if(power>=1.0f) power = 1.0f;
	if(power<0.0f) power = 0.0f;

	return power;
}

void CTLiteDoc::OnToolsCarfollowingsimulation()
{
	if(m_SelectedLinkID==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= m_LinkNoMap [m_SelectedLinkID];
	if(pLink!=NULL)
	{
		CDlgCarFollowing dlg;
		dlg.pDoc = this;
		dlg.m_SelectedLinkID = m_SelectedLinkID;
		dlg.m_YUpperBound = pLink->m_Length * 1609.344f;  // mile to meters
		dlg.m_FreeflowSpeed = pLink->m_SpeedLimit * 1609.344f / 3600; // mph to meter per second
		dlg.m_NumberOfLanes = pLink->m_NumLanes;
		dlg.DoModal ();
	}
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

	/*
	TCHAR szExe[1024];
	if( int(FindExecutable(
	strPath,									// pointer to filename
	strDir.IsEmpty() ? 0 : LPCTSTR(strDir),					// pointer to default directory
	szExe										// result-buffer
	)) <= 32 ) {
	OnExecError(GetLastError(), strCmd);
	return DWORD(-1);
	}
	*/
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

	sCommand.Format("%s\\DTALite.exe", pMainFrame->m_CurrentDirectory);

	g_ProcessExecute(sCommand, strParam, m_ProjectDirectory, true);

	CTime ExeEndTime = CTime::GetCurrentTime();

	CTimeSpan ts = ExeEndTime  - ExeStartTime;
	CString str_running_time;

	FILE* st = NULL;

	CString directory = m_ProjectDirectory;
	char simulation_short_summary[200];

	fopen_s(&st,directory+"short_summary.log","r");
	if(st!=NULL)
	{  
		fgets (simulation_short_summary , 200 , st);
		fclose(st);
	}

	str_running_time.Format ("Program execution has completed.\nSimulation Statistics: %s\nProgram execution time: %d hour(s) %d min(s) %d sec(s) \nDo you want to load the output now?",
		simulation_short_summary, ts.GetHours(), ts.GetMinutes(), ts.GetSeconds());

	if( AfxMessageBox(str_running_time, MB_YESNO| MB_ICONINFORMATION)==IDYES)
	{
		LoadSimulationOutput();
		UpdateAllViews(0);
	}

}
void CTLiteDoc::LoadSimulationOutput()
{
	CString DTASettingsPath = m_ProjectDirectory+"DTASettings.ini";

	int TrafficFlowModelFlag = (int)g_GetPrivateProfileFloat("simulation", "traffic_flow_model", 0, DTASettingsPath);	
	g_Simulation_Time_Horizon = (int) g_GetPrivateProfileFloat("simulation", "simulation_horizon_in_min", 1, DTASettingsPath);

	if(TrafficFlowModelFlag==0)  //BPR function 
	{
		m_StaticAssignmentMode = true;
		ReadSimulationLinkStaticMOEData(m_ProjectDirectory+"LinkStaticMOE.csv");
	}
	else {
		m_StaticAssignmentMode = false;

		ReadSimulationLinkMOEData(m_ProjectDirectory+"LinkMOE.csv");
	}

	ReadVehicleCSVFile(m_ProjectDirectory+"Vehicle.csv");
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
	OnToolsEditoddemandtable();
	m_bFitNetworkInitialized = false;
	UpdateAllViews(0);
}

void CTLiteDoc::OnToolsEditassignmentsettings()
{
	CString SettingsFile;
	SettingsFile.Format ("%sDTASettings.ini",m_ProjectDirectory);

	int NumberOfIterations = (int)(g_GetPrivateProfileFloat("assignment", "number_of_iterations", 10, SettingsFile));	
	float DemandGlobalMultiplier = g_GetPrivateProfileFloat("demand", "global_multiplier",1.0,SettingsFile);	
	int TrafficFlowModelFlag = (int)g_GetPrivateProfileFloat("simulation", "traffic_flow_model", 0, SettingsFile);	

	CDlgAssignmentSettings dlg;
	dlg.m_NumberOfIterations = NumberOfIterations;
	dlg.m_DemandGlobalMultiplier = DemandGlobalMultiplier;
	dlg.m_SimultionMethod  = TrafficFlowModelFlag;

	if(dlg.DoModal() ==IDOK)
	{
		char lpbuffer[64];
		NumberOfIterations = dlg.m_NumberOfIterations;
		DemandGlobalMultiplier = dlg.m_DemandGlobalMultiplier;
		TrafficFlowModelFlag = dlg.m_SimultionMethod;

		sprintf_s(lpbuffer,"%4d",TrafficFlowModelFlag);
		WritePrivateProfileString("simulation","traffic_flow_model",lpbuffer,SettingsFile);

		sprintf_s(lpbuffer,"%4d",NumberOfIterations);
		WritePrivateProfileString("assignment","number_of_iterations",lpbuffer,SettingsFile);

		sprintf_s(lpbuffer,"%5.3f",DemandGlobalMultiplier);
		WritePrivateProfileString("demand","global_multiplier",lpbuffer,SettingsFile);

	}

}

void CTLiteDoc::OnToolsEditoddemandtable()
{
	CDlgGridCtrl dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();
}

void CTLiteDoc::OnSearchLinklist()
{
	CDlgLinkList dlg;
	dlg.m_pDoc = this;
	dlg.DoModal ();

}

void CTLiteDoc::OnMoeVehicle()
{
	m_LinkMOEMode = vehicle;
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeVehicle(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == vehicle);
}
LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
	HKEY hkey;
	LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

	if (retval == ERROR_SUCCESS) {
		long datasize = MAX_PATH;
		TCHAR data[MAX_PATH];
		RegQueryValue(hkey, NULL, data, &datasize);
		lstrcpy(retdata,data);
		RegCloseKey(hkey);
	}

	return retval;
}
HINSTANCE g_OpenDocument(LPCTSTR url, int showcmd)
{
	TCHAR key[MAX_PATH + MAX_PATH];

	// First try ShellExecute()
	HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

	// If it failed, get the .htm regkey and lookup the program
	if ((UINT)result <= HINSTANCE_ERROR) {

		if (GetRegKey(HKEY_CLASSES_ROOT, _T(".csv"), key) == ERROR_SUCCESS) {
			lstrcat(key, _T("\\shell\\open\\command"));

			if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
				TCHAR *pos;
				pos = _tcsstr(key, _T("\"%1\""));
				if (pos == NULL) {			   // No quotes found
					pos = strstr(key, _T("%1"));	   // Check for %1, without quotes
					if (pos == NULL)			   // No parameter at all...
						pos = key+lstrlen(key)-1;
					else
						*pos = '\0';			 // Remove the parameter
				}
				else
					*pos = '\0';			  // Remove the parameter

				lstrcat(pos, _T(" "));
				lstrcat(pos, url);
				result = (HINSTANCE) WinExec(key,showcmd);
			}
		}
	}

	return result;
}

HINSTANCE g_GotoURL(LPCTSTR url, int showcmd)
{
	TCHAR key[MAX_PATH + MAX_PATH];

	// First try ShellExecute()
	HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

	// If it failed, get the .htm regkey and lookup the program
	if ((UINT)result <= HINSTANCE_ERROR) {

		if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) {
			lstrcat(key, _T("\\shell\\open\\command"));

			if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
				TCHAR *pos;
				pos = _tcsstr(key, _T("\"%1\""));
				if (pos == NULL) {			   // No quotes found
					pos = strstr(key, _T("%1"));	   // Check for %1, without quotes
					if (pos == NULL)			   // No parameter at all...
						pos = key+lstrlen(key)-1;
					else
						*pos = '\0';			 // Remove the parameter
				}
				else
					*pos = '\0';			  // Remove the parameter

				lstrcat(pos, _T(" "));
				lstrcat(pos, url);
				result = (HINSTANCE) WinExec(key,showcmd);
			}
		}
	}

	return result;
}

void CTLiteDoc::OnToolsViewsimulationsummary()
{

	g_OpenDocument(m_ProjectDirectory+"summary.log", SW_SHOW);
	/*
	CString sCommand;

	char WindowsDirectory[_MAX_PATH];
	GetWindowsDirectory(WindowsDirectory,_MAX_PATH);

	sCommand.Format ("write");
	CString strParam;
	strParam.Format ("%s",m_ProjectDirectory+"summary.log");
	CString strDir;
	strDir.Format ("%s", WindowsDirectory);

	g_ProcessExecute(sCommand, strParam, strDir, true);
	*/
}


void CTLiteDoc::OnToolsViewassignmentsummarylog()
{
	g_OpenDocument(m_ProjectDirectory+"assignment.log", SW_SHOW);
}

void CTLiteDoc::OnHelpVisitdevelopmentwebsite()
{
	g_OpenDocument("http://code.google.com/p/nexta/", SW_SHOW);
}

void CTLiteDoc::OnToolsRuntrafficassignment()
{
	bool bOKFlag = false;
	CString SettingsFile;
	SettingsFile.Format ("%sDTASettings.ini",m_ProjectDirectory);

	int NumberOfIterations = (int)(g_GetPrivateProfileFloat("assignment", "number_of_iterations", 10, SettingsFile));	
	float DemandGlobalMultiplier = g_GetPrivateProfileFloat("demand", "global_multiplier",1.0,SettingsFile);	
	int TrafficFlowModelFlag = (int)g_GetPrivateProfileFloat("simulation", "traffic_flow_model", 0, SettingsFile);	

	CDlgAssignmentSettings dlg;
	dlg.m_NumberOfIterations = NumberOfIterations;
	dlg.m_DemandGlobalMultiplier = DemandGlobalMultiplier;
	dlg.m_SimultionMethod  = TrafficFlowModelFlag;

	if(dlg.DoModal() ==IDOK)
	{
		char lpbuffer[64];
		NumberOfIterations = dlg.m_NumberOfIterations;
		DemandGlobalMultiplier = dlg.m_DemandGlobalMultiplier;
		TrafficFlowModelFlag = dlg.m_SimultionMethod;

		sprintf_s(lpbuffer,"%4d",TrafficFlowModelFlag);
		WritePrivateProfileString("simulation","traffic_flow_model",lpbuffer,SettingsFile);

		sprintf_s(lpbuffer,"%4d",NumberOfIterations);
		WritePrivateProfileString("assignment","number_of_iterations",lpbuffer,SettingsFile);

		sprintf_s(lpbuffer,"%5.3f",DemandGlobalMultiplier);
		WritePrivateProfileString("demand","global_multiplier",lpbuffer,SettingsFile);

		bOKFlag = true;
	}

	if(bOKFlag)
		OnToolsPerformtrafficassignment();
}

void CTLiteDoc::OnImportodtripfile3columnformat()
{
	CDlg_ImportODDemand dlg;
	if(dlg.DoModal ()==IDOK)
	{
		CString strFilter = "OD 3-column Trip File|*.txt";
		CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,strFilter);
		//	dlg.m_ofn.nFilterIndex = m_nFilterLoad;
		HRESULT hResult = (int)dlg.DoModal();
		if (FAILED(hResult)) {
			return;
		}
		ReadTripTxtFile(dlg.GetPathName());
		OnToolsEditoddemandtable();
		m_bFitNetworkInitialized = false;
		UpdateAllViews(0);	

	}

}

void CTLiteDoc::OnSearchVehicle()
{
	CDlgVehiclePath dlg;
	dlg.pDoc = this;
	dlg.DoModal ();

}

void CTLiteDoc::OnToolsPerformscheduling()
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

	sCommand.Format("%s\\FastTrain.exe", pMainFrame->m_CurrentDirectory);

	g_ProcessExecute(sCommand, strParam, m_ProjectDirectory, true);

	CTime ExeEndTime = CTime::GetCurrentTime();

	CTimeSpan ts = ExeEndTime  - ExeStartTime;
	CString str_running_time;

	FILE* st = NULL;

	CString directory = m_ProjectDirectory;
	char simulation_short_summary[200];

	fopen_s(&st,directory+"short_summary.log","r");
	if(st!=NULL)
	{  
		fgets (simulation_short_summary , 200 , st);
		fclose(st);
	}

	str_running_time.Format ("Program execution has completed.\nSimulation Statistics: %s\nProgram execution time: %d hour(s) %d min(s) %d sec(s) \nDo you want to load the output now?",
		simulation_short_summary, ts.GetHours(), ts.GetMinutes(), ts.GetSeconds());

	if( AfxMessageBox(str_running_time, MB_YESNO| MB_ICONINFORMATION)==IDYES)
	{
		LoadSimulationOutput();
		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnFileChangecoordinatestolong()
{
	CDlgNetworkAlignment  dlg;
	if(dlg.DoModal() ==IDOK)
	{
		if(dlg.m_Node1!=dlg.m_Node2 && m_NodeNametoIDMap.find(dlg.m_Node1)!= m_NodeNametoIDMap.end() && 
			m_NodeNametoIDMap.find(dlg.m_Node2)!= m_NodeNametoIDMap.end())
		{

			GDPoint m_Node1OrgPt, m_Node2OrgPt;
			m_Node1OrgPt = m_NodeIDMap[m_NodeNametoIDMap[dlg.m_Node1]]->pt;
			m_Node2OrgPt = m_NodeIDMap[m_NodeNametoIDMap[dlg.m_Node2]]->pt;

			float m_XScale = 1;
			float m_YScale = 1;

			if(m_Node1OrgPt.x - m_Node2OrgPt.x)
			{
				m_XScale = (dlg.m_NodeX1 - dlg.m_NodeX2)/(m_Node1OrgPt.x - m_Node2OrgPt.x);
			}

			if(m_Node1OrgPt.y - m_Node2OrgPt.y)
			{
				m_YScale = (dlg.m_NodeY1 - dlg.m_NodeY2)/(m_Node1OrgPt.y - m_Node2OrgPt.y);
			}

			float m_XOrigin = m_Node1OrgPt.x - dlg.m_NodeX1 /m_XScale;

			float m_YOrigin = m_Node1OrgPt.y - dlg.m_NodeY1 /m_YScale;

			// adjust node coordinates
			std::list<DTANode*>::iterator iNode;
			for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
			{

				(*iNode)->pt .x  = ((*iNode)->pt .x - m_XOrigin)*m_XScale;
				(*iNode)->pt .y  = ((*iNode)->pt .y - m_YOrigin)*m_YScale;
			}
			//adjust link cooridnates

			std::list<DTALink*>::iterator iLink;

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				(*iLink)->m_FromPoint.x = ((*iLink)->m_FromPoint.x -m_XOrigin)*m_XScale;
				(*iLink)->m_FromPoint.y = ((*iLink)->m_FromPoint.y -m_YOrigin)*m_YScale;

				(*iLink)->m_ToPoint.x = ((*iLink)->m_ToPoint.x -m_XOrigin)*m_XScale;
				(*iLink)->m_ToPoint.y = ((*iLink)->m_ToPoint.y -m_YOrigin)*m_YScale;
			}

		}

		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnFileOpenrailnetworkproject()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("NeXTA Project (*.dlp)|*.dlp|"));
	if(dlg.DoModal() == IDOK)
	{
		g_VisulizationTemplate = e_train_scheduling;
		OnOpenRailNetworkDocument(dlg.GetPathName());

		CDlgFileLoading dlg;
		dlg.m_pDoc = this;
		dlg.DoModal ();

		UpdateAllViews(0);
	}


}

void CTLiteDoc::OpenWarningLogFile(CString directory)
{
	m_WarningFile.open (directory+"warning.log", ios::out);
	if (m_WarningFile.is_open())
	{
		m_WarningFile.width(12);
		m_WarningFile.precision(3) ;
		m_WarningFile.setf(ios::fixed);
	}else
	{
		AfxMessageBox("File warning.log cannot be opened, and it might be locked by another program!");
	}
}
void CTLiteDoc::OnToolsExportopmodedistribution()
{
	CWaitCursor wc;
	CDlg_VehEmissions dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();

}

void CTLiteDoc::OnToolsEnumeratepath()
{
	CWaitCursor cws;

	int OD_index;

	int O_array[10]={52292,151094,136822,136054,143655,81422,140124,109800,52055,198841};

	int D_array[10]={154600,15843,47948,148424,52057,201951,240663,136327,144041,154523};

	for(OD_index = 0; OD_index<10; OD_index++)
	{
		m_OriginNodeID = m_NodeNametoIDMap[O_array[OD_index]];
		m_DestinationNodeID = m_NodeNametoIDMap[D_array[OD_index]];

		if(m_OriginNodeID>0 && m_DestinationNodeID>0)
		{
			if(m_pNetwork !=NULL)
				delete m_pNetwork;

			m_pNetwork = new DTANetworkForSP(m_NodeSet.size(), m_LinkSet.size(), 1, 1, m_AdjLinkSize);  //  network instance for single processor in multi-thread environment

			m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, true, false);

			m_pNetwork->GenerateSearchTree (m_OriginNodeID,m_DestinationNodeID,m_NodeSet.size());

			FILE* st = NULL;

			//		CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			//			_T("Path file (*.csv)|*.csv|"));
			//		if(dlg.DoModal() == IDOK)
			//		{
			CString str;
			str.Format ("C:\\path_set_%d.csv",OD_index);
			fopen_s(&st,str,"w");

			int NodeList[1000];

			int PathNo = 0;

			int i;
			for(i = 0; i < m_pNetwork->m_TreeListTail; i++)
			{
				if(m_pNetwork->m_SearchTreeList[i].CurrentNode == m_DestinationNodeID)
				{
					int nodeindex = 0;
					NodeList[nodeindex++] = m_pNetwork->m_SearchTreeList[i].CurrentNode;
					int Pred = m_pNetwork->m_SearchTreeList[i].PredecessorNode ;

					while(Pred!=0)
					{
						NodeList[nodeindex++] = m_pNetwork->m_SearchTreeList[Pred].CurrentNode;

						Pred = m_pNetwork->m_SearchTreeList[Pred].PredecessorNode ;
					}
					NodeList[nodeindex++] = m_pNetwork->m_SearchTreeList[Pred].CurrentNode;

					fprintf(st,"%d,%d,", PathNo,nodeindex);

					for(int n = nodeindex-1; n>=0; n--)
					{
						fprintf(st,"%d,", m_NodeIDtoNameMap[NodeList[n]]);
					}

					fprintf(st, "\n");
					PathNo++;
				}

			}
			fclose(st);
		}
	}

}

void CTLiteDoc::OnResearchtoolsExporttodtalitesensordataformat()
{
	CWaitCursor wc;
	int max_day = 23;
	FILE* st = NULL;

	for(int day = 0; day < max_day; day++)
	{
		CString str;
		str.Format( "SensorDataDay%03d.csv",day+1);
		fopen_s(&st,str,"w");
		if(st!=NULL)
		{
			std::list<DTALink*>::iterator iLink;

			fprintf(st,"Unix Timestamp (local time),StationID,Total Flow_per_obs_interval,Avg Density,Avg Speed\n");
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				if((*iLink)->m_bSensorData )
				{

					for(int t= day*1440; t<(day+1)*1440; t+=5)
					{
						//
						int hour = (t-day*1440)/60;
						int min =  (t-day*1440-hour*60);
						fprintf(st,"07/%02d/2010 %02d:%02d, %d %4.1f, 0, %4.1f\n", day, hour, min, (*iLink)->m_LinkNo+1,
							(*iLink)->m_LinkMOEAry[ t].ObsFlow/12, (*iLink)->m_LinkMOEAry[t].ObsSpeed);
					}


				}
			}

			fclose(st);

		}

	}

	fopen_s(&st,"input_sensor_location.csv","w");
	if(st!=NULL)
	{
		std::list<DTALink*>::iterator iLink;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_bSensorData )
			{
				//From Node,To Node,Type,OrgSensorID
				fprintf(st,"%d,%d,0,%d\n", (*iLink)->m_FromNodeNumber , (*iLink)->m_ToNodeNumber ,  (*iLink)->m_LinkNo+1);
			}

		}
	}
	fclose(st);
}
void CTLiteDoc::OnScenarioConfiguration()
{
	// TODO: Add your command handler code here

	if (m_ProjectDirectory.GetLength() == 0 ) 
	{
		MessageBox(NULL,"No open project!",NULL,MB_ICONWARNING);
		return;
	}


	CDlgScenario dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();
}

void CTLiteDoc::OnMoeViewmoes()
{
	// TODO: Add your command handler code here

	CDlgMOETabView dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();
}

void CTLiteDoc::OnImportdataImport()
{
	static char BASED_CODE szFilter[] = "EXCEL 2003 Workbook (*.xls)|*.xls||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		FillNetworkFromExcelFile(dlg.GetPathName());
	}
	CalculateDrawingRectangle();
	OffsetLink();
	UpdateAllViews(0);
}

bool CTLiteDoc::FillNetworkFromExcelFile(LPCTSTR pFileName)
{
	std::string itsErrorMessage;

	// Make sure the network is empty
	m_NodeSet.clear ();
	m_LinkSet.clear ();

	char warning[200];
	bool bExist=true;

	// Open the EXCEL file
	CDaoDatabase m_Database;
	CString strSQL;

	CString str_duplicated_link;

	m_Database.Open(pFileName, false, true, "excel 5.0; excel 97; excel 2000; excel 2003");

	// this provides number of sheets in .xls
	int nTables = m_Database.GetTableDefCount();
	if(nTables<3)
	{
		AfxMessageBox("Please make sure the Excel file has four worksheets: node, link, zone.", MB_ICONINFORMATION);
		return false;
	}

	// this accesses first sheet regardless of name.
	CDaoTableDefInfo TableInfo;

	m_Database.GetTableDefInfo(0, TableInfo);

	CString tablename1=(TableInfo).m_strName;
	tablename1.MakeUpper();

	if(tablename1.Find("NODE",0)!=0)
	{
		AfxMessageBox("Please make sure the 1st worksheet is an input_node table.", MB_ICONINFORMATION);
		return false;
	}


	CWaitCursor wait;

	strSQL = "select * from [";
	strSQL += TableInfo.m_strName;
	strSQL += "]";

	// Read record
	CRecordsetExt rsNode(&m_Database);
	rsNode.Open(dbOpenDynaset, strSQL);

	int i= 0;
	while(!rsNode.IsEOF())
	{
		int id = rsNode.GetLong(CString("node_id"),bExist,false);

		if(!bExist)
		{
			AfxMessageBox("Field node_id cannot be found in the node table.");
			return false;
		}

		if(id < 0)
		{
			sprintf(warning, "node_id: %d at row %d is invalid. Please check node table.", id, i+1);
			AfxMessageBox(warning);
			return false;
		}


		float x = rsNode.GetDouble(CString("x"),bExist,false);
		if(!bExist) 
		{
			AfxMessageBox("Field x cannot be found in the node table.");
			return false;
		}

		float y = rsNode.GetDouble(CString("y"),bExist,false);
		if(!bExist) 
		{
			AfxMessageBox("Field y cannot be found in the node table.");
			return false;
		}

		// Create and insert the node
		DTANode* pNode = new DTANode;
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

		rsNode.MoveNext();
	}	// end of while
	rsNode.Close();

	m_Database.GetTableDefInfo(1, TableInfo);

	CString tablename2=(TableInfo).m_strName;
	tablename2.MakeUpper();

	if(tablename2.Find("LINK",0)!=0){
		AfxMessageBox("Please make sure the 2nd worksheet is a link table.", MB_ICONINFORMATION);
		return false;
	}

	strSQL = "select * from [";
	strSQL += TableInfo.m_strName;
	strSQL += "]";

	// Read record
	CRecordsetExt rsLink(&m_Database);
	rsLink.Open(dbOpenDynaset, strSQL);

	i = 0;
	float default_distance_sum = 0;
	float length_sum = 0;
	while(!rsLink.IsEOF())
	{

		int from_node_id = rsLink.GetLong(CString("from_node_id"),bExist,false);
		if(!bExist)
		{
			AfxMessageBox("Field from_node_id cannot be found in the link table.");
			return false;
		}

		int to_node_id = rsLink.GetLong(CString("to_node_id"),bExist,false);
		if(!bExist) 
		{
			AfxMessageBox("Field to_node_id cannot be found in the link table.");
			return false;
		}


		if(from_node_id==0 ||to_node_id ==0)
			break;

		long link_id =  rsLink.GetLong(CString("link_id"),bExist,false);
		if(!bExist)
			link_id = 0;

		if(m_NodeNametoIDMap.find(from_node_id)== m_NodeNametoIDMap.end())
		{
			sprintf(warning, "from_node_id %d cannot be found in the node table!",from_node_id);
			AfxMessageBox(warning, MB_ICONINFORMATION);
			return false;
		}

		if(m_NodeNametoIDMap.find(to_node_id)== m_NodeNametoIDMap.end())
		{
			sprintf(warning, "to_node_id %d cannot be found in the node table!",to_node_id);
			return false;
		}

		DTALink* pExistingLink =  FindLinkWithNodeIDs(from_node_id,to_node_id);

		if(pExistingLink)
		{
			char warning[100];
			sprintf(warning, "Link %d-> %d is duplicated.\n", from_node_id,to_node_id);
			if(str_duplicated_link.GetLength () < 3000)  // not adding and showing too many links
			{
				str_duplicated_link += warning;

				continue;
			}
		}

		float length = rsLink.GetDouble(CString("length_in_mile"),bExist,false);
		if(!bExist) 
		{
			AfxMessageBox("Field length_in_mile cannot be found in the link table.");
			return false;
		}
		if(length > 100)
		{
			sprintf(warning, "The length of link %d -> %d is longer than 100 miles, please ensure the unit of link length in the link sheet is mile.",from_node_id,to_node_id);
			AfxMessageBox(warning);
			return false;
		}

		int number_of_lanes = rsLink.GetLong(CString("number_of_lanes"),bExist,false);
		if(!bExist)
		{
			AfxMessageBox("Field number_of_lanes cannot be found in the link table.");
			return false;
		}

		float speed_limit_in_mph= rsLink.GetLong(CString("speed_limit_in_mph"),bExist,false);
		if(!bExist) 
		{
			AfxMessageBox("Field speed_limit_in_mph cannot be found in the link table.");
			return false;
		}

		if(speed_limit_in_mph==0){
			sprintf(warning, "Link %d -> %d has a speed limit of 0, please sort the link table by speed_limit_in_mph and re-check it!",from_node_id,to_node_id);
			AfxMessageBox(warning, MB_ICONINFORMATION);
			return false;
		}

		float capacity_in_pcphpl= rsLink.GetDouble(CString("lane_capacity_in_vhc_per_hour"),bExist,false);
		if(!bExist)
		{
			AfxMessageBox("Field capacity_in_veh_per_hour_per_lane cannot be found in the link table.");
			return false;
		}

		if(capacity_in_pcphpl<0){
			sprintf(warning, "Link %d -> %d has a negative capacity, please sort the link table by capacity_in_veh_per_hour_per_lane and re-check it!",from_node_id,to_node_id);
			AfxMessageBox(warning, MB_ICONINFORMATION);
			return false;
		}

		int type = rsLink.GetLong(CString("link_type"),bExist,false);
		if(!bExist) 
		{
			AfxMessageBox("Field link_type cannot be found in the link table.");
			return false;
		}

		int direction = rsLink.GetLong(CString("direction"),bExist,false);
		if(!bExist) 
		{
			AfxMessageBox("Field direction cannot be found in the link table.");
			return false;
		}

		 CString name = rsLink.GetCString(CString("name"));

		float k_jam, wave_speed_in_mph;

		if(type==1)
		{
			k_jam = 220;
		}else
		{
		   k_jam = 120;
		}

		wave_speed_in_mph = 12;


		int m_SimulationHorizon = 1;

			int link_code_start = 1;
			int link_code_end = 1;

			if (direction == -1) // reversed
			{
				link_code_start = 2; link_code_end = 2;
			}

			if (direction == 0) // two-directional link
			{
				link_code_start = 1; link_code_end = 2;
			}

				// no geometry information
				CCoordinate cc_from, cc_to; 
				cc_from.X = m_NodeIDMap[m_NodeNametoIDMap[from_node_id]]->pt.x;
				cc_from.Y = m_NodeIDMap[m_NodeNametoIDMap[from_node_id]]->pt.y;

				cc_to.X = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]]->pt.x;
				cc_to.Y = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]]->pt.y;

				std::vector<CCoordinate> CoordinateVector;

				CoordinateVector.push_back(cc_from);
				CoordinateVector.push_back(cc_to);


			for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
			{

				bool bNodeNonExistError = false;
				int m_SimulationHorizon = 1;
				DTALink* pLink = new DTALink(m_SimulationHorizon);
				pLink->m_LinkNo = i;
				pLink->m_Name  = name;
				pLink->m_OrgDir = direction;
				pLink->m_LinkID = link_id;

				if(link_code == 1)  //AB link
				{
					pLink->m_FromNodeNumber = from_node_id;

					pLink->m_ToNodeNumber = to_node_id;
					pLink->m_Direction  = 1;

					for(int si = 0; si < CoordinateVector.size(); si++)
					{
					GDPoint	pt;
					pt.x = CoordinateVector[si].X;
					pt.y = CoordinateVector[si].Y;
					pLink->m_ShapePoints .push_back (pt);
					}

				}

				if(link_code == 2)  //BA link
				{
					pLink->m_FromNodeNumber = to_node_id;
					pLink->m_ToNodeNumber = from_node_id;
					pLink->m_Direction  = 1;

					for(int si = CoordinateVector.size()-1; si >=0; si--)
					{
					GDPoint	pt;
					pt.x = CoordinateVector[si].X;
					pt.y = CoordinateVector[si].Y;
					pLink->m_ShapePoints .push_back (pt);
					}
				}


				pLink->m_FromNodeID = m_NodeNametoIDMap[from_node_id];
				pLink->m_ToNodeID= m_NodeNametoIDMap[to_node_id];

				m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
				m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;

				unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
				m_NodeIDtoLinkMap[LinkKey] = pLink;
				m_NodeIDtoLinkMap[link_id] = pLink;

				pLink->m_NumLanes= number_of_lanes;
				pLink->m_SpeedLimit= speed_limit_in_mph;
				pLink->m_StaticSpeed = pLink->m_SpeedLimit;

				pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance
				pLink->m_MaximumServiceFlowRatePHPL= capacity;
				pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
				pLink->m_link_type= type;

				pLink->m_Kjam = k_jam;
				pLink->m_Wave_speed_in_mph  = wave_speed_in_mph;

				m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);

				pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
				pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;


				default_distance_sum+= pLink->DefaultDistance();
				length_sum += pLink ->m_Length;
				//			pLink->SetupMOE();
				m_LinkSet.push_back (pLink);
				m_LinkNoMap[i]  = pLink;
				i++;

			}


			rsLink.MoveNext();
	}

	rsLink.Close();

		m_UnitMile  = 1.0f;

		if(length_sum>0.000001f)
			m_UnitMile= length_sum / default_distance_sum ;

		m_UnitFeet = m_UnitMile/5280.0f;  


		if(m_UnitMile>50)  // long/lat must be very large and greater than 62!
		{
			if(AfxMessageBox("Is the long/lat coordinate system used in this data set?", MB_YESNO) == IDYES)
			{
				m_LongLatCoordinateFlag = true;
				m_UnitFeet = m_UnitMile/62/5280.0f;  // 62 is 1 long = 62 miles
			}
		}


	if(str_duplicated_link.GetLength() >0)
	{
		str_duplicated_link+= "\nDuplicated links are removed.";

		AfxMessageBox(str_duplicated_link);
	}

/////
	m_Database.GetTableDefInfo(2, TableInfo);

	CString tablename3=(TableInfo).m_strName;
	tablename3.MakeUpper();

	if(tablename3.Find("ZONE",0)!=0){
		AfxMessageBox("Please make sure the 3rd worksheet is a zone table.", MB_ICONINFORMATION);
		return false;
	}

	strSQL = "select * from [";
	strSQL += TableInfo.m_strName;
	strSQL += "]";

	
	bool bNodeNonExistError = false;
	m_NodeIDtoZoneNameMap.clear ();

	// Read record
	CRecordsetExt rsZone(&m_Database);
	rsZone.Open(dbOpenDynaset, strSQL);

	while(!rsZone.IsEOF())
	{
		int zone_number = rsZone.GetLong(CString("zone_id"),bExist,false);
		if(!bExist) 
		{
			AfxMessageBox("Field zone_id cannot be found in the zone table.");
			return false;
		}

		if(zone_number ==0)
			break;

		int node_name = rsZone.GetLong(CString("node_id"),bExist,false);

		if(!bExist) 
		{
			AfxMessageBox("Field node_id cannot be found in the zone table.");
			return false;
		}

		map <int, int> :: const_iterator m_Iter = m_NodeNametoIDMap.find(node_name);

			if(m_Iter == m_NodeNametoIDMap.end( ))
			{
			CString m_Warning;
			m_Warning.Format("Node Number %d in the zone tabe has not be defined in the node table", node_name);
			AfxMessageBox(m_Warning);
			return false;
			}
			m_NodeIDtoZoneNameMap[m_NodeNametoIDMap[node_name]] = zone_number;
			// if there are multiple nodes for a zone, the last node id is recorded.
			int zoneid  = zone_number-1;
			m_ZoneIDtoNodeIDMap[zoneid] = m_NodeNametoIDMap[node_name];

			if(m_ODSize < zone_number)
				m_ODSize = zone_number;

			rsZone.MoveNext ();
	}
	rsZone.Close();

/////
	m_Database.GetTableDefInfo(3, TableInfo);

	CString tablename4=(TableInfo).m_strName;
	tablename4.MakeUpper();

	if(tablename4.Find("DEMAND",0)!=0){
		AfxMessageBox("Please make sure the 4th worksheet is a demand table.", MB_ICONINFORMATION);
		return false;
	}

	strSQL = "select * from [";
	strSQL += TableInfo.m_strName;
	strSQL += "]";

	// Read record
	CRecordsetExt rsDemand(&m_Database);
	rsDemand.Open(dbOpenDynaset, strSQL);

		if(m_DemandMatrix!=NULL)
		{
			DeallocateDynamicArray(m_DemandMatrix,m_ODSize,m_ODSize);
			m_DemandMatrix = NULL;
		}

		m_DemandMatrix   =  AllocateDynamicArray<float>(m_ODSize,m_ODSize);

		for(int i= 0; i<m_ODSize; i++)
			for(int j= 0; j<m_ODSize; j++)
			{
				m_DemandMatrix[i][j]= 0.0f;
			}

			m_MaxODDemand  = 1;

	while(!rsDemand.IsEOF())
	{
			int origin_zone_id, destination_zone_id;
			float number_of_vehicles, starting_time_in_min, ending_time_in_min;

			origin_zone_id = rsDemand.GetLong(CString("from_zone_id"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field from_zone_id cannot be found in the demand table.");
				return false;
			}

			if( origin_zone_id ==0)
				break;

			destination_zone_id = rsDemand.GetLong(CString("to_zone_id"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field to_zone_id cannot be found in the demand table.");
				return false;
			}

			number_of_vehicles = rsDemand.GetDouble(CString("number_of_vehicles"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field number_of_vehicles cannot be found in the demand table.");
				return false;
			}

			starting_time_in_min = rsDemand.GetLong(CString("starting_time_in_min"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field starting_time_in_min cannot be found in the demand table.");
				return false;
			}

			ending_time_in_min = rsDemand.GetLong(CString("ending_time_in_min"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field ending_time_in_min cannot be found in the demand table.");
				return false;
			}
			m_DemandMatrix[origin_zone_id][destination_zone_id] += number_of_vehicles;

				if(m_MaxODDemand < number_of_vehicles)
					m_MaxODDemand =  number_of_vehicles ;

			rsDemand.MoveNext ();
	}
	rsDemand.Close();
	return true;
}

void CTLiteDoc::AdjustCoordinateUnitToMile()
{

	m_AdjLinkSize +=2;  // add two more elements to be safe

	std::list<DTANode*>::iterator iNode;
	//adjust XY coordinates if the corrdinate system is not consistenty
	if(fabs(m_UnitMile-1.00)>0.10)  // ask users if we need to adjust the XY coordinates
	{
		if(AfxMessageBox("The link length information in link.csv is not consistent with the X/Y coordinates in node.csv.\n Do you want to adjust the the X/Y coordinate unit to mile in node.csv?", MB_YESNO) == IDYES)

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

			if(m_LongLatCoordinateFlag)
				m_UnitFeet = m_UnitMile/62/5280.0f;  // 62 is 1 long = 62 miles

			CalculateDrawingRectangle();
			UpdateAllViews(0);

	}
}



bool CTLiteDoc::ReadZoneShapeCSVFile(LPCTSTR lpszFileName)
{
	CString information_msg;
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	int zone_shape_point_count = 0;
	int id;
	if(st!=NULL)
	{
		while(!feof(st))
		{
			id			= g_read_integer(st);
			if(id == -1)  // reach end of file
				break;

			int part_no			= g_read_integer(st);
			int point_id		= g_read_integer(st);

			DTAZone zone = m_ZoneVector[id];
			float x	= g_read_float(st);
			float y	= g_read_float(st);

			GDPoint	pt;
			pt.x = x;
			pt.y = y;

			zone.m_ShapePoints .push_back (pt);
			zone_shape_point_count ++;
			/*
			}else
			{
			CString msg;
			msg.Format ("zone %d in the zone shape csv file has not defined in the input node table.",id);
			AfxMessageBox(msg);
			fclose(st);
			return false;

			}
			*/
		}
		fclose(st);

		return true;
	}

	information_msg.Format ("%d nodes are loaded from the node csv file.",zone_shape_point_count,lpszFileName);

	return true;

}

bool CTLiteDoc::ReadNodeGeoFile(LPCTSTR lpszFileName)
{
	return ReadZoneShapeCSVFile(lpszFileName);

}


bool CTLiteDoc::ReadLinkGeoFile(LPCTSTR lpszFileName)
{
	CString information_msg;
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	int link_shape_point_count = 0;

	int id = 0;
	if(st!=NULL)
	{
		while(!feof(st))
		{
			id			= g_read_integer(st);
			if(id == -1)  // reach end of file
				break;

			int number_of_feature_points			= g_read_integer(st);

			DTALink* pLink = FindLinkWithLinkID (id);
			if(pLink!=NULL)
			{
				for(int i=0; i< number_of_feature_points; i++)
				{
					float x	= g_read_float(st);
					float y	= g_read_float(st);
					GDPoint	pt;
					pt.x = x;
					pt.y = y;

					pLink->m_ShapePoints .push_back (pt);
					link_shape_point_count ++;
				}

			}else
			{
				CString msg;
				msg.Format  ("link %d in the link shape csv file has not defined in the input shape table.",id);
				AfxMessageBox(msg,MB_ICONINFORMATION);
				fclose(st);
				return false;

			}
		}
		fclose(st);

	}

	information_msg.Format ("%d shape points are loaded from the link shape csv file.",link_shape_point_count,lpszFileName);

	std::list<DTALink*>::iterator iLink;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if( (*iLink)->m_ShapePoints .size() ==0)  // no shape points from external sources, use positoins from upstream and downstream nodes
		{
			(*iLink)->m_ShapePoints.push_back((*iLink)->m_FromPoint);
			(*iLink)->m_ShapePoints.push_back((*iLink)->m_ToPoint);

		}
	}


	return true;
}
