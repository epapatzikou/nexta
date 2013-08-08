// DlgPathList.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "CSVParser.h"
#include "DlgPathList.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"
#include "Dlg_VehicleClassification.h"
#include "Dlg_UserInput.h"
#include <string>
#include <sstream>
#include <vector>

extern void g_SelectColorCode(CDC* pDC, int ColorCount);
extern float g_Simulation_Time_Stamp;
extern CPen s_PenSimulationClock;

extern void g_SelectSuperThickPenColor(CDC* pDC, int ColorCount);
extern void g_SelectThickPenColor(CDC* pDC, int ColorCount);
extern void g_SelectBrushColor(CDC* pDC, int ColorCount);


extern CDlgPathList* g_pPathListDlg;
// CDlgPathList dialog

IMPLEMENT_DYNAMIC(CDlgPathList, CDialog)

CDlgPathList::CDlgPathList(CWnd* pParent /*=NULL*/)
: CBaseDialog(CDlgPathList::IDD, pParent)
, m_StrPathMOE(_T(""))
{
	m_TimeLeft = 0;
	m_TimeRight = 1440;
	m_MOEAggregationIntervalInMin = 1;
}


CDlgPathList::~CDlgPathList()
{
	g_pPathListDlg = NULL;
}

void CDlgPathList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_ListCtrl);
	DDX_Check(pDX, IDC_CHECK_ZOOM_TO_SELECTED_LINK, m_ZoomToSelectedLink);
	DDX_Text(pDX, IDC_PATHMOE, m_StrPathMOE);
	DDX_Control(pDX, IDC_COMBO1, m_ComboRandomCoef);
	DDX_Control(pDX, IDC_LIST1, m_PathList);
	DDX_Control(pDX, IDC_COMBO_StartHour, m_StartHour);
	DDX_Control(pDX, IDC_COMBO_EndHour, m_EndHour);
	DDX_Control(pDX, IDC_COMBO_AggIntrevalList, m_AggregationIntervalList);
	DDX_Control(pDX, IDC_COMBO_PLOT_TYPE, m_PlotType);
}


BEGIN_MESSAGE_MAP(CDlgPathList, CDialog)
	ON_WM_PAINT()
	ON_WM_SIZE()

	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CDlgPathList::OnLvnItemchangedList)
	ON_BN_CLICKED(IDOK, &CDlgPathList::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgPathList::OnBnClickedCancel)
	ON_BN_CLICKED(ID_FIND_RANDOM_ROUTE, &CDlgPathList::OnBnClickedFindRandomRoute)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgPathList::OnCbnSelchangeCombo1)
	ON_COMMAND(ID_PATH_DATA_EXPORT_CSV, &CDlgPathList::OnPathDataExportCSV)
	ON_COMMAND(ID_DATA_IMPORT_CSV, &CDlgPathList::OnDataImportCsv)
	ON_BN_CLICKED(IDC_CHECK_ZOOM_TO_SELECTED_LINK, &CDlgPathList::OnBnClickedCheckZoomToSelectedLink)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDlgPathList::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDDATA_Analysis, &CDlgPathList::OnBnClickedDataAnalysis)
	ON_COMMAND(ID_DATA_GENERATESAMPLEINPUTPATHCSV, &CDlgPathList::OnDataGeneratesampleinputpathcsv)
	ON_COMMAND(ID_DATA_CLEANALLPATHS, &CDlgPathList::OnDataCleanallpaths)
	ON_CBN_SELCHANGE(IDC_COMBO_StartHour, &CDlgPathList::OnCbnSelchangeComboStarthour)
	ON_CBN_SELCHANGE(IDC_COMBO_EndHour, &CDlgPathList::OnCbnSelchangeComboEndhour)
	ON_CBN_SELCHANGE(IDC_COMBO_AggIntrevalList, &CDlgPathList::OnCbnSelchangeComboAggintrevallist)
	ON_CBN_SELCHANGE(IDC_COMBO_PLOT_TYPE, &CDlgPathList::OnCbnSelchangeComboPlotType)
	ON_BN_CLICKED(IDDATA_Analysis2, &CDlgPathList::OnBnClickedAnalysis2)
	ON_BN_CLICKED(IDDATA_FREEVAL_Analysis_Generate_File, &CDlgPathList::OnBnClickedFreevalAnalysisGenerateFile)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGELANECAPACITY, &CDlgPathList::OnChangeattributesforlinksalongpathChangelanecapacity)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGE, &CDlgPathList::OnChangeattributesforlinksalongpathChange)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGELINKTYPE, &CDlgPathList::OnChangeattributesforlinksalongpathChangelinktype)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGESPEEDLIMIT, &CDlgPathList::OnChangeattributesforlinksalongpathChangespeedlimit)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGESPEEDLIMIT_KMPH, &CDlgPathList::OnChangeattributesforlinksalongpathChangespeedlimitKmph)
		ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGEJAMDENSITY, &CDlgPathList::OnChangeattributesforlinksalongpathChangejamdensity)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGEBACKWAVESPEED, &CDlgPathList::OnChangeattributesforlinksalongpathChangebackwavespeed)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGESATURATIONFLOWRATE, &CDlgPathList::OnChangeattributesforlinksalongpathChangesaturationflowrate)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_CHANGEJAMDENSITY33625, &CDlgPathList::OnChangeattributesforlinksalongpathChangejamdensity33625)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_EFFECTIVEGREENTIME, &CDlgPathList::OnChangeattributesforlinksalongpathEffectivegreentime)
	ON_COMMAND(ID_CHANGEATTRIBUTESFORLINKSALONGPATH_DELETELINKSALONGPATH, &CDlgPathList::OnChangeattributesforlinksalongpathDeletelinksalongpath)
	ON_BN_CLICKED(IDDATA_DYNAMIC_Density_Contour, &CDlgPathList::OnBnClickedDynamicDensityContour)
	ON_BN_CLICKED(IDDATA_DYNAMIC_Speed_Contour, &CDlgPathList::OnBnClickedDynamicSpeedContour)
	ON_BN_CLICKED(IDDATA_DYNAMIC_Flow_Contour, &CDlgPathList::OnBnClickedDynamicFlowContour)
	ON_COMMAND(ID_DATA_SAVECURRENTPATH, &CDlgPathList::OnDataSavecurrentpath)
	ON_COMMAND(ID_DATA_EXPORTFREEVALSEGMENTFILE, &CDlgPathList::OnDataExportfreevalsegmentfile)
	ON_BN_CLICKED(IDC_BUTTON_GOOGLE_EARTH_KML, &CDlgPathList::OnBnClickedButtonGoogleEarthKml)
	ON_COMMAND(ID_DATA_DELETEEXISTINGPATHSININPUTPATHCSVFILE, &CDlgPathList::OnDataDeleteexistingpathsininputpathcsvfile)
END_MESSAGE_MAP()


// CDlgPathList message handlers
#define PATHDATACOLUMNSIZE 14

BOOL CDlgPathList::OnInitDialog()
{

	CDialog::OnInitDialog();

	m_PlotType.AddString ("Simulated Travel Time (min)");
	m_PlotType.AddString ("Simulated and Observed Travel Time (min)");
	m_PlotType.AddString ("Energy (KJ)");
	m_PlotType.AddString ("CO2 (g)");
	m_PlotType.AddString ("NOX (g)");
	m_PlotType.AddString ("CO (g)");
	m_PlotType.AddString ("HC (g)");
	m_PlotType.AddString ("Gasline (Gallon)");
	m_PlotType.AddString ("Miles Per Gallon");
	m_PlotType.SetCurSel (0);

	m_TimeLeft = int(m_pDoc->m_SimulationStartTime_in_min/30)*30 ;
	m_TimeRight = int(m_pDoc->m_SimulationEndTime_in_min/30)*30;

	for (int i = 0; i<= 24; i++)
	{
		CString str;
		str.Format("%d",i);

		m_StartHour.AddString (str);
		m_EndHour.AddString (str);
	}

	m_StartHour.SetCurSel(m_TimeLeft/60);
	m_EndHour.SetCurSel(min(24,m_TimeRight/60+1));

	m_AggregationValueVector.push_back(1);
	m_AggregationValueVector.push_back(5);
	m_AggregationValueVector.push_back(15);
	m_AggregationValueVector.push_back(30);
	m_AggregationValueVector.push_back(60);
	m_AggregationValueVector.push_back(120);
	m_AggregationValueVector.push_back(1440);

	for(unsigned int i = 0;  i< m_AggregationValueVector.size (); i++)
	{
		CString str;
		str.Format("%d min",m_AggregationValueVector[i]);

		m_AggregationIntervalList.AddString (str);

		if(g_MOEAggregationIntervalInMin  == m_AggregationValueVector[i])
		{
			m_AggregationIntervalList.SetCurSel (i);

		}

	}


	for(int c = 0; c <=11; c++)
	{
		CString str;
		str.Format ("%.1f",c*0.2f);
		m_ComboRandomCoef.AddString(str);
	}

	m_ComboRandomCoef.SetCurSel (0);

	// Give better margin to editors
	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;  // Hao: this ponter should be delete. 
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);

	std::vector<std::string> m_Column_names;

	_TCHAR *ColumnMOELabel[PATHDATACOLUMNSIZE] =
	{
		_T("No."),_T("From->To"),_T("Street Name"), _T("Length (ml)"), _T("Speed Limit"), _T("Free-flow Travel Time (min)"),_T("# of lanes"),_T("Lane Saturation Flow Rate"),_T("Lane Capacity"), _T("Link Type"), _T("Sensor Type"),  _T("Count") 
	};


	//Add Columns and set headers
	for (size_t i=0;i< PATHDATACOLUMNSIZE;i++)
	{
		CGridColumnTrait* pTrait = NULL;
		//		pTrait = new CGridColumnTraitEdit();
		m_ListCtrl.InsertColumnTrait((int)i,ColumnMOELabel[i],LVCFMT_LEFT,-1,-1, pTrait);
		m_ListCtrl.SetColumnWidth((int)i,LVSCW_AUTOSIZE_USEHEADER);
	}
	m_ListCtrl.SetColumnWidth(0, 80);

	ReloadData();

	return true;
}
void CDlgPathList::ReloadData()
{
	CWaitCursor cursor;

	m_PathList.ResetContent ();

	for(unsigned int i= 0; i< m_pDoc->m_PathDisplayList.size(); i++)
	{
		CString str;

		if(m_pDoc->m_PathDisplayList[i].m_bSavedPath)
			str.Format("Path ID.%d: %s, %d links ",i+1, m_pDoc->m_PathDisplayList[i].m_path_name.c_str (), m_pDoc->m_PathDisplayList[i].m_LinkVector .size());
		else
			str.Format("Path ID.%d: %s, %d links ",i+1, m_pDoc->m_PathDisplayList[i].m_path_name.c_str (), m_pDoc->m_PathDisplayList[i].m_LinkVector .size());

		m_PathList.AddString (str);

	}

	if(m_pDoc->m_SelectPathNo>=0 && m_pDoc->m_SelectPathNo< m_PathList.GetCount ())
	{
		m_PathList.SetCurSel(m_pDoc->m_SelectPathNo);
	}

	m_ListCtrl.DeleteAllItems();

	if(m_pDoc->m_PathDisplayList.size() > m_pDoc->m_SelectPathNo && m_pDoc->m_SelectPathNo!=-1)
	{
		float total_distance = 0;
		float total_travel_time = 0;
		int number_of_sensors = 0;
		float total_count = 0;

		for (int i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size(); i++)
		{

			int column_count  = 1;
			DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector[i]];
			char text[100];


			sprintf_s(text, "%d",i+1);
			int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,text , 0, 0, 0, NULL);

			// from -> to
			sprintf_s(text, "%d->%d",pLink->m_FromNodeNumber , pLink->m_ToNodeNumber );
			m_ListCtrl.SetItemText(Index,column_count++,text );

			// street name
			m_ListCtrl.SetItemText(Index,column_count++,pLink->m_Name.c_str () );

			//length
			sprintf_s(text, "%5.3f",pLink->m_Length);
			m_ListCtrl.SetItemText(Index,column_count++,text);
			total_distance+= pLink->m_Length;

			//speed limit
			sprintf_s(text, "%4.0f",pLink->m_SpeedLimit  );
			m_ListCtrl.SetItemText(Index,column_count++,text);

			//free flow travel time
			sprintf_s(text, "%4.2f",pLink->m_FreeFlowTravelTime);
			m_ListCtrl.SetItemText(Index,column_count++,text);
			total_travel_time+=pLink->m_FreeFlowTravelTime;

			//# of lanes
			sprintf_s(text, "%d",pLink->m_NumberOfLanes );
			m_ListCtrl.SetItemText(Index,column_count++,text);

			//saturation flow rate
			sprintf_s(text, "%4.2f",pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane );
			m_ListCtrl.SetItemText(Index,column_count++,text);

			//capacity
			sprintf_s(text, "%4.2f",pLink->m_LaneCapacity);
			m_ListCtrl.SetItemText(Index,column_count++,text);

			if(m_pDoc->m_LinkTypeMap.find(pLink->m_link_type) != m_pDoc->m_LinkTypeMap.end())
			{

				if(m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.size()>=1)
				{
					sprintf_s(text, "%s", m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str ());
					m_ListCtrl.SetItemText(Index,column_count++,text);
				}
				else
				{
					sprintf_s(text, "%s", m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str ());
					m_ListCtrl.SetItemText(Index,column_count++,text);
				}

			}

			if(pLink->m_bSensorData == true)
			{
				sprintf_s(text, "%s",pLink->m_SensorTypeString .c_str ());
				number_of_sensors += 1;
			}else
				sprintf_s(text, "");

			m_ListCtrl.SetItemText(Index,column_count++,text);


			if(pLink->m_bSensorData == true)
			{
				sprintf_s(text, "%.3f",pLink->GetSensorLaneVolume(g_Simulation_Time_Stamp));
				total_count+= pLink->GetSensorLaneVolume(g_Simulation_Time_Stamp);
			}else
				sprintf_s(text, "");

			m_ListCtrl.SetItemText(Index,column_count++,text);

		}


		if(m_PlotType.GetCurSel ()==0)
		{
			m_StrPathMOE.Format("Distance=%4.2f mi, Free-flow Travel Time=%4.2f min, # of Sensors = %d, detected count = %.4f ",
				total_distance,total_travel_time, number_of_sensors,total_count);
		}


		if(m_PlotType.GetCurSel ()==1)  // with sensor data
		{


			float avg_error =  m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].GetErrorStatistics(m_TimeLeft, m_TimeRight, m_MOEAggregationIntervalInMin);
			float avg_percentage_error = m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].GetRelativeErrorStatistics(m_TimeLeft, m_TimeRight, m_MOEAggregationIntervalInMin);

			m_StrPathMOE.Format("Distance=%4.2f mile, Free flow travel Time=%4.2f min, Avg abs error= %.2f (min), Avg perc error=%.1f%%",
				total_distance,total_travel_time, avg_error,avg_percentage_error);
		}


		UpdateData(0);
	}



}

void CDlgPathList::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{

	UpdateData(1);

	m_pDoc->m_SelectedLinkNo = -1;
	g_ClearLinkSelectionList();

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	while(pos!=NULL)
	{
		int nSelectedRow = m_ListCtrl.GetNextSelectedItem(pos);
		char str[100];
		m_ListCtrl.GetItemText (nSelectedRow,2,str,20);
		int LinkNo = atoi(str);
		m_pDoc->m_SelectedLinkNo = LinkNo;
		g_AddLinkIntoSelectionList(LinkNo, m_pDoc->m_DocumentNo );

	}
	if(m_ZoomToSelectedLink == true)
		m_pDoc->ZoomToSelectedLink(m_pDoc->m_SelectedLinkNo);

	Invalidate();

	m_pDoc->UpdateAllViews(0);

}

void CDlgPathList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnBnClickedCancel()
{
	CDialog::OnOK();
}



void CDlgPathList::OnBnClickedFindAlternative()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnBnClickedFindRandomRoute()
{
	m_pDoc->m_RandomRoutingCoefficient = m_ComboRandomCoef.GetCurSel ()*0.2f;
	m_pDoc->Routing(false);
	m_pDoc->UpdateAllViews(NULL);

	ReloadData();
	Invalidate();


}

void CDlgPathList::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnBnClickedExportRoute()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnPathDataExportCSV()
{
	// calculate time-dependent travel time
	int time_step = 1;

	CString export_file_name;

	export_file_name = m_pDoc->m_ProjectDirectory +"export_path_MOE.csv";
	// save demand here

	FILE* st;
	fopen_s(&st,export_file_name,"w");
	if(st!=NULL)
	{
		fprintf(st,"Summary\n");
		fprintf(st,"path index,path_name,distance(mile),free_flow_travel_time(min),path_name,avg_simulated_travel_time(min),avg_sensor_travel_time(min),percentage_difference,avg_time-dependent_error(min),avg_time-dependent_percentage_error (%)\n");
		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			float avg_simulated_travel_time = path_element.GetTimeDependentMOE (m_TimeLeft,0,m_TimeRight-m_TimeLeft);
			float avg_sensor_travel_time = path_element.GetTimeDependentMOE (m_TimeLeft,1,m_TimeRight-m_TimeLeft);

			fprintf(st,"\nPath %d,%s,%.2f,%.2f,%s,%.2f,%.2f,%.1f,%.2f,%.2f",
				p+1,path_element.m_path_name .c_str (), path_element.total_distance,path_element.total_free_flow_travel_time,
				path_element.m_path_name .c_str (),
				avg_simulated_travel_time,avg_sensor_travel_time, avg_simulated_travel_time/max(0.1,avg_simulated_travel_time)*100,
				path_element.GetErrorStatistics (m_TimeLeft,m_TimeRight,15), 
				path_element.GetRelativeErrorStatistics  (m_TimeLeft,m_TimeRight,15));
		}		


		fprintf(st,"\n\nPart I,time-dependent travel time");
		// 

		time_step= 15;


		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			fprintf(st,"\nTime,,,");

			for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
			}

			fprintf(st,"\nPath %d, %s,simulated travel time,",p+1,path_element.m_path_name .c_str ());

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentTravelTime[t]);

			}  // for each time


			if(path_element.m_bWithSensorTravelTime  == true);
			{
				fprintf(st,"\n,,observed travel time,");

				for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
				{
					fprintf(st,"%.2f,", path_element.m_SensorTimeDependentTravelTime[t]);
				}

				//fprintf(st,"\nRelative Difference,,",p+1,path_element.m_path_name .c_str ());

				//	for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
				//	{
				//		if(path_element.m_SensorTimeDependentTravelTime[t] >0.001f)
				//							fprintf(st,"%.2f,", path_element.m_TimeDependentTravelTime[t]/max(0.001,path_element.m_SensorTimeDependentTravelTime[t]));
				//		else //no data
				//							fprintf(st,",");

				//	}

				fprintf(st,"\n");

			}


		}

		// travel time index


		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			float total_free_flow_travel_time = 0;
			for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				total_free_flow_travel_time += pLink->m_FreeFlowTravelTime ;
			}
			m_pDoc->m_PathDisplayList[p].total_free_flow_travel_time = total_free_flow_travel_time;
		}

		fprintf(st,"\n\nTravel Time Tndex,,");

		for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
		{
			fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
		}


		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];


			fprintf(st,"\nPath %d, %s,",p+1, path_element.m_path_name .c_str ());
			for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentTravelTime[t]/max(1,path_element.total_free_flow_travel_time));

			}  // for each time

		}

		fprintf(st,"\n\nPart II,link sequence\n\n");
		fprintf(st,"path_id,link_sequence_no,from_node_id->to_node_id,from_node_id,to_node_id,name,length (ml),speed_limit,free-flow travel_time,# of lanes,Lane Saturation Flow Rate,Lane Capacity,Link Type\n");

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink != NULL)
				{

					fprintf(st,"%d,%d,\"[%d,%d]\",%d,%d,%s,%5.3f,%5.0f,%5.3f,%d,%5.0f,%5.1f,",
						p+1,i+1,pLink->m_FromNodeNumber , pLink->m_ToNodeNumber, pLink->m_FromNodeNumber , pLink->m_ToNodeNumber,   pLink->m_Name.c_str (), pLink->m_Length ,
						pLink->m_SpeedLimit, pLink->m_FreeFlowTravelTime , pLink->m_NumberOfLanes,  pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane ,pLink->m_LaneCapacity );

					if(m_pDoc->m_LinkTypeMap.find(pLink->m_link_type) != m_pDoc->m_LinkTypeMap.end())
					{
						fprintf(st, "%s", m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str ());
					}
				}

				fprintf(st,"\n");


			}
		} //for each path

		// part II: time-dependent speed contour
		int step_size = 1;

		for(int aggregation_index = 0; aggregation_index<4; aggregation_index++)
		{
			switch (aggregation_index)
			{
			case 0: step_size = 1; break;
			case 1: step_size = 5; break;
			case 2: step_size = 15; break;
			case 3: step_size = 30; break;

			default: step_size = 30;
			}

			int previous_MOEAggregationIntervalInMin = g_MOEAggregationIntervalInMin;
			g_MOEAggregationIntervalInMin = step_size;

			fprintf(st,"path_id,link_sequence_no,from_node_id->to_node_id,name,length (ml),speed_limit,free-flow travel_time,# of lanes,Lane Saturation Flow Rate,Lane Capacity,Link Type\n");

			for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
			{
				DTAPath path_element = m_pDoc->m_PathDisplayList[p];

				fprintf(st,"\n\nPart III,time-dependent speed contour for path %d, %s,(%d min)\n\n", p+1, path_element.m_path_name .c_str (),step_size);


					fprintf(st,",,,");

				for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
				{
					fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
				}

					fprintf(st,"\n");

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];


					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.2; s++)
						{
							 
							CString label = pLink->m_Name .c_str ();

							if(pLink->m_Name  == "(null)")
							{
							label.Format ("%d->%d",pLink->m_FromNodeNumber , pLink->m_ToNodeNumber);
							}

							fprintf(st,"%d->%d,%s,%s,", pLink->m_FromNodeNumber , pLink->m_ToNodeNumber , m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str (),label);

							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "%.1f,", pLink->GetSimulationSpeed (t));

							}
							fprintf(st,"\n");
						}

					}


				}

				fprintf(st,"\n\n");

				fprintf(st,"\n\nPart III,time-dependent density contour for path %d, %s,(%d min)\n\n", p+1, path_element.m_path_name .c_str (),step_size);

					fprintf(st,",,,");
				for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
				{
					fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
				}

					fprintf(st,"\n");

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];


					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.2; s++)
						{
							 
							CString label = pLink->m_Name .c_str ();

							if(pLink->m_Name  == "(null)")
							{
							label.Format ("%d->%d",pLink->m_FromNodeNumber , pLink->m_ToNodeNumber);
							}

							fprintf(st,"%d->%d,%s,%s,", pLink->m_FromNodeNumber , pLink->m_ToNodeNumber , m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str (),label);

							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "%.1f,", pLink->GetSimulationDensity (t));

							}
							fprintf(st,"\n");
						}

					}


				}

				// time axis
				fprintf(st,",,,");
				fprintf(st,"\n");


			} //for each path

			g_MOEAggregationIntervalInMin = previous_MOEAggregationIntervalInMin;
		}


		fprintf(st,"\n\nPart IV");
		fprintf(st,"\n\npath_no,departure time,simulated_travel_time_in_min,observed_travel_time_in_min,travel_time_index,total observed count on passing links\n");
		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
			{
				path_element.m_TimeDependentCount[t] = 0;
				path_element.m_TimeDependentTravelTime[t] = t;  // t is the departure time

				for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					if(pLink == NULL)
						break;

					path_element.m_TimeDependentTravelTime[t] += pLink->GetTravelTime(path_element.m_TimeDependentTravelTime[t]);

					if(i==0)// first link
						path_element.m_TimeDependentCount[t] +=  pLink->GetSensorLaneVolume(t);
					else
						path_element.m_TimeDependentCount[t] +=  pLink->GetSensorLaneVolume(path_element.m_TimeDependentTravelTime[t]);


					// current arrival time at a link/node along the path, t in [t] is still index of departure time, t has a dimension of 0 to 1440* number of days

					//			    TRACE("\n path %d, time at %f, TT = %f",p, path_element.m_TimeDependentTravelTime[t], pLink->GetTravelTime(path_element.m_TimeDependentTravelTime[t]) );

				}

				path_element.m_TimeDependentTravelTime[t] -= t; // remove the starting time, so we have pure travel time;

				fprintf(st,"%d,%s,%.2f,%.2f,%.2f,%.2f\n",p+1, m_pDoc->GetTimeStampString24HourFormat(t),
					path_element.GetTimeDependentMOE (t,0,15),path_element.GetTimeDependentMOE (t,1,1),
					path_element.m_TimeDependentTravelTime[t]/max(1,path_element.total_free_flow_travel_time), path_element.m_TimeDependentCount [t]);


				ASSERT(path_element.m_TimeDependentTravelTime[t]>=0);

				if( path_element.m_MaxTravelTime < path_element.m_TimeDependentTravelTime[t])
					path_element.m_MaxTravelTime = path_element.m_TimeDependentTravelTime[t];

				for(int tt=1; tt<time_step; tt++)
				{
					path_element.m_TimeDependentTravelTime[t+tt] = path_element.m_TimeDependentTravelTime[t];
				}


			}  // for each time
			fprintf(st,"\n");

		} // for each path

		fprintf(st,"\n\nPart V,time-dependent emission data");
		// 

		time_step= 15;


		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			fprintf(st,"\nTime,,,");

			for(int t = m_TimeLeft ; t< m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
			}

			fprintf(st,"\nPath %d, %s,path engery,",p+1,path_element.m_path_name .c_str ());

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentEnergy [t]);
			} 

			fprintf(st,"\nPath,,path CO2,");

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentCO2 [t]);
			} 

			fprintf(st,"\nPath,,path NOX,");

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentNOX [t]);
			} 

			fprintf(st,"\nPath,,path CO,");

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentCO [t]);
			} 


			fprintf(st,"\nPath,,path HC,");

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentHC[t]);
			} 
			// for each time

			fprintf(st,"\nPath %d, %s,path engery per mile,",p+1,path_element.m_path_name .c_str ());

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentEnergy [t]/max(0.1,path_element.total_distance ));

			} 

			fprintf(st,"\nPath %d, %s,path mile per gallon,",p+1,path_element.m_path_name .c_str ());

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.total_distance /max(0.01,path_element.m_TimeDependentEnergy [t]/1000/121.7));

			} 
			fprintf(st,"\nPath,,path CO2 per mile,");

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentCO2 [t]/max(0.1,path_element.total_distance ));
			} 

			fprintf(st,"\nPath,,path NOX  per mile,");

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentNOX [t]/max(0.1,path_element.total_distance ));
			} 

			fprintf(st,"\nPath,,path CO  per mile,");

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentCO [t]/max(0.1,path_element.total_distance ));
			} 


			fprintf(st,"\nPath,,path HC  per mile,");

			for(int t = m_TimeLeft ; t<m_TimeRight; t+= time_step)  // for each starting time
			{
				fprintf(st,"%.2f,", path_element.m_TimeDependentHC[t]/max(0.1,path_element.total_distance ));
			} 
			// for each time
			fprintf(st,"\n");

		}


		fclose(st);
	}else
	{
		AfxMessageBox("File cannot be opened.");
		return;
	}
	m_pDoc->OpenCSVFileInExcel (export_file_name);
}


void CDlgPathList::OnDataImportCsv()
{


	CString ErrorMessageVector;

	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
	{

		m_pDoc->m_PathDisplayList[p].m_LinkVector.clear();
	}


		m_pDoc->m_PathDisplayList.clear();
		m_pDoc->m_SelectPathNo=0;


		char lpszFileName[_MAX_PATH];

		sprintf(lpszFileName,"%sinput_path.csv",m_pDoc->m_ProjectDirectory);


		int prev_path_id = -1;
		CCSVParser parser;
		if (parser.OpenCSVFile(lpszFileName))
		{
			bool bNodeNonExistError = false;

			m_pDoc->m_OriginNodeID = -1;
			m_pDoc->m_DestinationNodeID = -1;

			int count = 0;
			DTALink* pLink =NULL;
			while(parser.ReadRecord())
			{
				int link_id = 0;
				int from_node_id;
				int to_node_id;

				int path_id = count;


				string path_name;
				parser.GetValueByFieldName("path_name",path_name);
				//find reference travel time
				DTAPath path_element;

				path_element.m_path_name  = path_name;

				m_pDoc->m_PathDisplayList.push_back (path_element);


				string node_sequence; 
				parser.GetValueByFieldName("node_sequence",node_sequence);\

					std::vector<int> node_sequence_vector;

				read_multiple_integers_from_a_string(node_sequence.c_str (), node_sequence_vector);

				int route_no = m_pDoc->m_PathDisplayList.size()-1;

				for(unsigned int i = 0; i < node_sequence_vector.size()-1; i++)
				{
				
					pLink = m_pDoc->FindLinkWithNodeNumbers(node_sequence_vector[i],node_sequence_vector[i+1],lpszFileName);
			

					if(pLink==NULL)
					{
						CString ErrorMessage;
						ErrorMessage.Format("Path Name %s: %d->%d does not exist.\n", path_name,node_sequence_vector[i],node_sequence_vector[i+1]);

						ErrorMessageVector+= ErrorMessage;
						return;
					}

					if(count==0)
					{
						m_pDoc->m_OriginNodeID  = pLink->m_FromNodeID ;
					}

					m_pDoc->m_PathDisplayList[route_no].m_LinkVector.push_back (pLink->m_LinkNo );

				}



				if(node_sequence_vector.size()>=2)
				{
					for(int t = 0 ; t< 1440; t+= 15)  // for each starting time
					{
						CString str = 		m_pDoc->GetTimeStampStrFromIntervalNo (t,true);

						std::string str_time = m_pDoc->CString2StdString (str);

						float travel_time = 0;
						parser.GetValueByFieldName(str_time,travel_time);


						if(travel_time >0.1f)
						{
							m_pDoc->m_PathDisplayList[route_no].m_bWithSensorTravelTime = true;
							for(int s = 0; s<15; s++)
							{
								m_pDoc->m_PathDisplayList[route_no].m_SensorTimeDependentTravelTime[t+s] = travel_time;
							}
						}

					}

				}


				CString c_path_name;
				c_path_name.Format("%d",route_no+1);


				count++;
			}

			if(pLink!=NULL)  // last link
			{
				m_pDoc->m_DestinationNodeID  = pLink->m_ToNodeID ;	
			}

		}else
		{
			AfxMessageBox("File input_path.csv cannot be opened.");
		}
		ReloadData();

		if(	m_PathList.GetCount() >=1)
		{

			CString msg;
			msg.Format("%d path(s) have been imported from file input_path.csv.", m_PathList.GetCount());

			AfxMessageBox(msg, MB_ICONINFORMATION);

			m_PathList.SetCurSel(0);
			Invalidate();

		}


		if(ErrorMessageVector.GetLength ()>0)
		{
			m_pDoc->WriteStringToLogFile("error_log_path_list.csv", ErrorMessageVector);

			ErrorMessageVector+= "Please check file error_log_path_list.csv in Excel.";
			AfxMessageBox(ErrorMessageVector);

		}
}

void CDlgPathList::OnBnClickedCheckZoomToSelectedLink()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnLbnSelchangeList1()
{
	m_pDoc->m_SelectPathNo = m_PathList.GetCurSel();

	ReloadData();

	Invalidate();
	m_pDoc->UpdateAllViews (0);

}

void CDlgPathList::OnBnClickedDataAnalysis()
{
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{

		(*iLink)-> m_bIncludedBySelectedPath = false;
		(*iLink)-> m_bFirstPathLink = false;
		(*iLink)-> m_bLastPathLink = false;
	}

	// mark all links in the selected path
	if(m_pDoc->m_PathDisplayList.size() > m_pDoc->m_SelectPathNo && m_pDoc->m_SelectPathNo!=-1)
	{
		for (int i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size(); i++)
		{

			DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector[i]];
			pLink-> m_bIncludedBySelectedPath = true;

			if(i==0)
			{
				pLink-> m_bFirstPathLink = true;
			}
			if(i== m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size()-1)
			{
				pLink-> m_bLastPathLink = true;
			}
		}
	}

	CDlg_VehicleClassification* m_pDlg = new CDlg_VehicleClassification; 

	m_pDlg->m_PresetChartTitle.Format ("Path No.%d", m_pDoc->m_SelectPathNo+1);

	m_pDlg->m_pDoc = m_pDoc;
	m_pDoc->m_VehicleSelectionMode = CLS_path_trip;
	m_pDlg->m_XSelectionNo = CLS_time_interval_15_min;
	m_pDlg->m_VehicleSelectionNo  = CLS_path_trip;
	m_pDlg->SetModelessFlag(true); // voila! this is all it takes to make your dlg modeless!
	m_pDlg->Create(IDD_DIALOG_Summary); 
	m_pDlg->ShowWindow(SW_SHOW); 

}

void CDlgPathList::OnDataGeneratesampleinputpathcsv()
{
	// calculate time-dependent travel time

	int time_step = 1;

	CString input_sample_file_name;

	input_sample_file_name.Format("%sinput_path.csv",m_pDoc->m_ProjectDirectory);
	// add path record

	// if the file is empty, add field titles

	bool bEmptyFile = true;
	FILE* pFile;
	fopen_s(&pFile,input_sample_file_name,"rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);
		if(Length > 0 )
			bEmptyFile = false;

	}
	// else we add a path record

	FILE* st;

	fopen_s(&st,input_sample_file_name,"a");
	if(st==NULL)
	{

		AfxMessageBox("Please close file input_path.csv in Excel.");
		return;

	}

	if(m_pDoc->m_PathDisplayList.size()==0)
	{
		AfxMessageBox("To generate the file input_path.csv, Please first define one path by selecting the origin and destination nodes.", MB_ICONINFORMATION);
		return;
	}

	if(st!=NULL)
	{


		if(bEmptyFile == true)
		{
			//write field titles
		fprintf(st,"path_name,node_sequence,");


		for(int min  = 0; min < 1440; min +=15)
		{
		CString str = 		m_pDoc->GetTimeStampStrFromIntervalNo (min,true);
		fprintf(st,"%s,",str);

		}


		fprintf(st,"\n");
		}

		CDlg_UserInput dlg;
		dlg.m_StrQuestion  = "Please specify the name of the path:";
		dlg.m_InputValue = "sample: 101 NB";

		CString path_name; 
		if(dlg.DoModal ()==IDOK)
		{
			path_name = dlg.m_InputValue;
		}else
		{

		fclose(st);
		return;
		}
		// path name
		fprintf(st,"%s,", path_name);


		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink != NULL)
				{
					fprintf(st,"%d;",
						pLink->m_FromNodeNumber);

				}

				if(i==path_element.m_LinkVector.size()-1)
				{
					if(pLink != NULL)
					{
						fprintf(st,"%d\n",
							pLink->m_ToNodeNumber);

					}
	
				}


			}


		} //for each path

		fclose(st);
		//m_pDoc->OpenCSVFileInExcel (input_sample_file_name);
	}
}

void CDlgPathList::OnDataCleanallpaths()
{
	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
	{
		m_pDoc->m_PathDisplayList[p].m_LinkVector.clear ();
	}
	m_pDoc->m_PathDisplayList.clear ();

	m_PathList.ResetContent ();

	m_ListCtrl.DeleteAllItems();

	m_pDoc->m_OriginNodeID = -1;
	m_pDoc->m_DestinationNodeID = -1;
	m_StrPathMOE.Format ("");
	UpdateData(0);
	m_pDoc->UpdateAllViews(0);

}

void CDlgPathList::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect PlotRect;
	GetClientRect(PlotRect);

	CRect rect;
	CWnd *pWnd = GetDlgItem(IDC_LIST1);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect); 

	CRect PlotRectOrg = PlotRect;

	if(m_TimeLeft<0)
		m_TimeLeft = 0;

	if(m_TimeRight< m_TimeLeft+60)
		m_TimeRight= m_TimeLeft+60;

	PlotRect.top += 70;
	PlotRect.bottom = PlotRect.top + 160;
	PlotRect.left = rect.right +50 ;

	PlotRect.right -= 40;

	DrawPlot(&dc, PlotRect);

}

void CDlgPathList::DrawPlot(CPaintDC* pDC,CRect PlotRect)
{
	if(m_PathList.GetCount () ==0)
	{

		pDC->TextOut(PlotRect.left,PlotRect.top+0 ,"No path has been defined.");
		pDC->TextOut(PlotRect.left,PlotRect.top+30 ,"Two methods for generating paths:");
		pDC->TextOut(PlotRect.left,PlotRect.top+50 ,"select GIS node layer and right click origin/destination nodes.");
		pDC->TextOut(PlotRect.left,PlotRect.top+70 ,"load path csv file from the menu Data->Import CSV.");

		return;

	}
	int p = 	m_PathList.GetCurSel();

	CalculateTimeDependentTravelTime();

	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));

	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_DOT,0,RGB(0,0,0));

	CString str_MOE;
	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&NormalPen);


	// step 1: calculate m_YUpperBound;
	m_YUpperBound = 0;
	int i;


	// step 2: calculate m_UnitDistance;
	// data unit
	m_UnitDistance = 1;
	if((m_YUpperBound - m_YLowerBound)>0)
		m_UnitDistance = (float)(PlotRect.bottom - PlotRect.top)/(m_YUpperBound - m_YLowerBound);


	// step 3: time interval
	int TimeXPosition;

	int TimeInterval = g_FindClosestTimeResolution(m_TimeRight - m_TimeLeft);

	// time unit
	m_UnitTime = 1;
	if((m_TimeRight - m_TimeLeft)>0)
		m_UnitTime = (float)(PlotRect.right - PlotRect.left)/(m_TimeRight - m_TimeLeft);


	// step 4: draw time axis

	pDC->SelectObject(&TimePen);



	char buff[20];
	for(i=m_TimeLeft;i<=m_TimeRight;i+=TimeInterval)
	{
		if(i == m_TimeLeft || i==m_TimeRight)
		{
			pDC->SelectObject(&NormalPen);

			//			i = int((m_TimeLeft/TimeInterval)+0.5)*TimeInterval; // reset time starting point
		}
		else
			pDC->SelectObject(&DataPen);

		TimeXPosition=(long)(PlotRect.left+(i-m_TimeLeft)*m_UnitTime);

		if(i>= m_TimeLeft)
		{
			pDC->MoveTo(TimeXPosition,PlotRect.bottom+2);
			pDC->LineTo(TimeXPosition,PlotRect.top);

			if(i/2 <10)
				TimeXPosition-=5;
			else
				TimeXPosition-=3;

			if(TimeInterval < 60)
			{
				int hour, min;
				hour = i/60;
				min =  i- hour*60;
				wsprintf(buff,"%2d:%02d",hour, min);
			}
			else
			{
				int min_in_a_day = i-int(i/1440*1440);

				wsprintf(buff,"%dh",min_in_a_day/60 );

			}
			pDC->TextOut(TimeXPosition,PlotRect.bottom+3,buff);
		}
	}

	pDC->SelectObject(&s_PenSimulationClock);
	if(g_Simulation_Time_Stamp >=m_TimeLeft && g_Simulation_Time_Stamp <= m_TimeRight )
	{
		TimeXPosition=(long)(PlotRect.left+(g_Simulation_Time_Stamp -m_TimeLeft)*m_UnitTime);
		pDC->MoveTo(TimeXPosition,PlotRect.bottom+2);
		pDC->LineTo(TimeXPosition,PlotRect.top);
	}


	m_YUpperBound = 0;
	m_YLowerBound = 0;
	TimeInterval = 1;

	int value_type = m_PlotType.GetCurSel ();

	for(int t=m_TimeLeft;t<m_TimeRight;t+=TimeInterval)
	{
		if( m_YUpperBound < m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, value_type, m_MOEAggregationIntervalInMin) )
			m_YUpperBound =  m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, value_type,m_MOEAggregationIntervalInMin);

	}

	if(m_PlotType.GetCurSel () == 1)  //additional data
	{
		for(int t=m_TimeLeft;t<m_TimeRight;t+=TimeInterval)
		{
			if( m_YUpperBound < m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, 0,m_MOEAggregationIntervalInMin) )
				m_YUpperBound =  m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, 0,m_MOEAggregationIntervalInMin);

			if( m_YUpperBound < m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, 1,m_MOEAggregationIntervalInMin) )
				m_YUpperBound =  m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, 1,m_MOEAggregationIntervalInMin);

		}


	}


	if(m_YUpperBound>=10.0f)
		m_YUpperBound = (m_YUpperBound/10.0f+1.0f)*10.f;
	else
		m_YUpperBound = int(m_YUpperBound)+2.0f;

	float YInterval = 0.2f;

	if(m_YUpperBound>=2.0f)
		YInterval = 0.5f;

	if(m_YUpperBound>=5.0f)
		YInterval = 1.f;

	if(m_YUpperBound>=10.0f)
		YInterval = 2.0f;

	if(m_YUpperBound>=20.0f)
		YInterval = 5.0f;

	if(m_YUpperBound>=50.f)
		YInterval = 10.f;

	if(m_YUpperBound>=80.f)
		YInterval = 20.f;

	if(m_YUpperBound>=150.f)
		YInterval = 30.f;

	if(m_YUpperBound>=500.f)
		YInterval = int(m_YUpperBound/5);

	// data unit
	m_UnitData = 1.f;
	if((m_YUpperBound - m_YLowerBound)>0.0f)
		m_UnitData = (float)(PlotRect.bottom - PlotRect.top)/(m_YUpperBound - m_YLowerBound);

	// draw Y axis

	for(float ii=m_YLowerBound; ii <= m_YUpperBound; ii+= YInterval)
	{
		if( ii > m_YUpperBound)
			ii = m_YUpperBound;

		if(ii == m_YLowerBound)
			pDC->SelectObject(&NormalPen);
		else
			pDC->SelectObject(&DataPen);

		int TimeYPosition= PlotRect.bottom - (int)((ii*m_UnitData)+0.50f);

		pDC->MoveTo(PlotRect.left-2, TimeYPosition);
		pDC->LineTo(PlotRect.right,TimeYPosition);

		if(ii <= m_YUpperBound)
		{
			if(YInterval>=1)
				sprintf_s(buff,"%3.0f",ii);
			else
				sprintf_s(buff,"%3.1f",ii);

			pDC->TextOut(PlotRect.left-45,TimeYPosition-5,buff);
		}
	}

	int MOEType = m_PlotType.GetCurSel ();


	if(m_PlotType.GetCurSel ()!=1)
	{

		CPen BluePen(PS_SOLID,1,RGB(0,0,255));

		pDC->SelectObject(&BluePen);


		for(int t=m_TimeLeft;t<m_TimeRight;t+=TimeInterval)
		{
			int TimeYPosition= PlotRect.bottom - (int)((m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, MOEType, m_MOEAggregationIntervalInMin)*m_UnitData)+0.50);
			TimeXPosition=(long)(PlotRect.left+(t-m_TimeLeft)*m_UnitTime);


			if(t==m_TimeLeft)
				pDC->MoveTo(TimeXPosition,TimeYPosition);
			else
				pDC->LineTo(TimeXPosition,TimeYPosition);

		}

	}


	if(m_PlotType.GetCurSel () ==1)  // draw sensor travel time
	{

		MOEType = 0;  // simulation
		CPen BluePen(PS_SOLID,1,RGB(0,0,255));

		pDC->SelectObject(&BluePen);


		for(int t=m_TimeLeft;t<m_TimeRight;t+=TimeInterval)
		{
			int TimeYPosition= PlotRect.bottom - (int)((m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, MOEType, m_MOEAggregationIntervalInMin)*m_UnitData)+0.50);
			TimeXPosition=(long)(PlotRect.left+(t-m_TimeLeft)*m_UnitTime);


			if(t==m_TimeLeft)
				pDC->MoveTo(TimeXPosition,TimeYPosition);
			else
				pDC->LineTo(TimeXPosition,TimeYPosition);

		}

		MOEType = 1;  // observation
		CPen SensorPen(PS_SOLID,1,RGB(255,64,64));	 // brown
		pDC->SelectObject(&SensorPen);


		for(int t=m_TimeLeft;t<m_TimeRight;t+=TimeInterval)
		{
			int TimeYPosition= PlotRect.bottom - (int)((m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, MOEType, m_MOEAggregationIntervalInMin)*m_UnitData)+0.50);
			TimeXPosition=(long)(PlotRect.left+(t-m_TimeLeft)*m_UnitTime);


			if(t==m_TimeLeft)
				pDC->MoveTo(TimeXPosition,TimeYPosition);
			else
				pDC->LineTo(TimeXPosition,TimeYPosition);

		}

	}

	//// max travel time  // draw band (mean, max) for selected paths
	//if(MOEType==3)  
	//{
	//	
	//	g_SelectSuperThickPenColor(pDC,p);


	//	CPoint pt[192];

	//	int pt_count = 0;
	//	int t;
	//	for(t=0;t<1440;t+=15)
	//		{
	//		int TimeYPosition= PlotRect.bottom - (int)((m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, 1, m_MOEAggregationIntervalInMin)*m_UnitData)+0.50);
	//			TimeXPosition=(long)(PlotRect.left+(t-m_TimeLeft)*m_UnitTime);

	//		pt[pt_count].x =TimeXPosition;
	//		pt[pt_count].y =TimeYPosition;
	//		
	//		pt_count++;

	//		}

	//	for(t=1440-15;t>=0;t-=15)
	//		{
	//		int TimeYPosition= PlotRect.bottom - (int)((m_pDoc->m_PathDisplayList[p].GetTimeDependentMOE(t, 0, m_MOEAggregationIntervalInMin)*m_UnitData)+0.50);
	//		TimeXPosition=(long)(PlotRect.left+(t-m_TimeLeft)*m_UnitTime);

	//		pt[pt_count].x =TimeXPosition;
	//		pt[pt_count].y =TimeYPosition;
	//		
	//		pt_count++;

	//		}

	//	g_SelectThickPenColor(pDC,p);
	//	g_SelectBrushColor(pDC, p);
	//	pDC->Polygon (pt,192);

	//}


	/*
	// step 5: draw segments along the select path

	m_SegmentDistanceVector.resize(m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo]->m_LinkSize);

	for (i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo]->m_LinkSize; i++)
	{
	DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo]->m_LinkVector[i]];
	if(pLink!=NULL)
	{
	m_YUpperBound+=pLink->m_Length ;

	m_SegmentDistanceVector[i] = m_YUpperBound;
	}
	}

	int TimeYPosition;
	int TimeYPositionPrev;

	for (i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo]->m_LinkSize; i++)
	{
	DTALink* pLink;

	// extra bottom line
	if(i == 0)
	{

	pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo]->m_LinkVector[i]];

	pDC->SelectObject(&NormalPen);
	TimeYPosition= PlotRect.bottom;
	TimeYPositionPrev = TimeYPosition;

	if(pLink!=NULL)
	{
	wsprintf(buff,"A: %d",pLink->m_FromNodeNumber );
	pDC->TextOut(PlotRect.left-50,TimeYPosition-5,buff);
	}

	pDC->MoveTo(PlotRect.left-2, TimeYPosition);
	pDC->LineTo(PlotRect.right,TimeYPosition);
	}


	pDC->SelectObject(&DataPen);

	TimeYPosition= PlotRect.bottom - (int)((m_SegmentDistanceVector[i]*m_UnitDistance)+0.50);

	pDC->MoveTo(PlotRect.left-2, TimeYPosition);
	pDC->LineTo(PlotRect.right,TimeYPosition);

	pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo]->m_LinkVector[i]];
	if(pLink!=NULL)
	{
	if(i== m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo]->m_LinkSize-1)
	{
	wsprintf(buff,"B: %d",pLink->m_ToNodeNumber );
	pDC->TextOut(PlotRect.left-50,TimeYPosition-5,buff);
	}
	else
	{
	if(TimeYPosition < TimeYPositionPrev-10 && TimeYPosition >= PlotRect.bottom-10)
	{
	wsprintf(buff,"%d",pLink->m_ToNodeNumber );
	pDC->TextOut(PlotRect.left-40,TimeYPosition-5,buff);
	}
	}

	TimeYPositionPrev = TimeYPosition;

	}
	}

	*/
}

void CDlgPathList::CalculateTimeDependentTravelTime()
{
	int time_step  =1;
	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
	{
		DTAPath path_element = m_pDoc->m_PathDisplayList[p];

		m_pDoc->m_PathDisplayList[p].total_free_flow_travel_time = 0;
		m_pDoc->m_PathDisplayList[p].total_distance = 0;

		for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
		{
			DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
			if(pLink == NULL)
				break;

			m_pDoc->m_PathDisplayList[p].total_free_flow_travel_time += pLink->m_FreeFlowTravelTime ;
			m_pDoc->m_PathDisplayList[p].total_distance += pLink->m_Length ;

		}

		for(int t = m_pDoc->m_SimulationStartTime_in_min ; t< m_pDoc->m_SimulationEndTime_in_min; t+= time_step)  // for each starting time
		{
			path_element.m_TimeDependentCount[t] = 0;
			path_element.m_TimeDependentTravelTime[t] = t;  // t is the departure time
			path_element.m_TimeDependentEnergy[t]= 0;
			path_element.m_TimeDependentCO2[t]= 0;
			path_element.m_TimeDependentCO[t]= 0;
			path_element.m_TimeDependentHC[t]=0;
			path_element.m_TimeDependentNOX[t] =0;

			for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink == NULL)
					break;

				path_element.m_TimeDependentTravelTime[t] += pLink->GetTravelTime(path_element.m_TimeDependentTravelTime[t]);

				int current_time = path_element.m_TimeDependentTravelTime[t];
				if(current_time >=1440)
					current_time = 1439;

				//path_element.m_TimeDependentEnergy[t] += pLink->m_LinkMOEAry [current_time].Energy;
				//path_element.m_TimeDependentCO2[t] += pLink->m_LinkMOEAry [current_time].CO2;
				//path_element.m_TimeDependentCO[t] += pLink->m_LinkMOEAry [current_time].CO;
				//path_element.m_TimeDependentHC[t] += pLink->m_LinkMOEAry [current_time].HC;
				//path_element.m_TimeDependentNOX[t] += pLink->m_LinkMOEAry [current_time].NOX;


				// current arrival time at a link/node along the path, t in [t] is still index of departure time, t has a dimension of 0 to 1440* number of days

				//			    TRACE("\n path %d, time at %f, TT = %f",p, path_element.m_TimeDependentTravelTime[t], pLink->GetTravelTime(path_element.m_TimeDependentTravelTime[t]) );

			}

			path_element.m_TimeDependentTravelTime[t] -= t; // remove the starting time, so we have pure travel time;
			m_pDoc->m_PathDisplayList[p].m_TimeDependentTravelTime[t] =   path_element.m_TimeDependentTravelTime[t] ;
			//m_pDoc->m_PathDisplayList[p].m_TimeDependentEnergy[t] =   path_element.m_TimeDependentEnergy[t] ;
			//m_pDoc->m_PathDisplayList[p].m_TimeDependentCO2[t] =   path_element.m_TimeDependentCO2[t] ;
			//m_pDoc->m_PathDisplayList[p].m_TimeDependentCO[t] =   path_element.m_TimeDependentCO[t] ;
			//m_pDoc->m_PathDisplayList[p].m_TimeDependentHC[t] =   path_element.m_TimeDependentHC[t] ;
			//m_pDoc->m_PathDisplayList[p].m_TimeDependentNOX[t] =   path_element.m_TimeDependentNOX[t] ;



		}  // for each time

	}

}
void CDlgPathList::OnCbnSelchangeComboStarthour()
{
	int sel = m_StartHour.GetCurSel();

	if((sel)* 60 < m_TimeRight)
	{
		m_TimeLeft = (sel)* 60;
	}else
	{// restore
		m_StartHour.SetCurSel (m_TimeLeft/60);

	}
	Invalidate();
}

void CDlgPathList::OnCbnSelchangeComboEndhour()
{
	int sel = m_EndHour.GetCurSel();

	if(m_TimeLeft <(sel)* 60)
	{
		m_TimeRight = (sel)* 60;
	}else
	{  // restore
		m_EndHour.SetCurSel(m_TimeRight/60);

	}

	Invalidate();
}

void CDlgPathList::OnCbnSelchangeComboAggintrevallist()
{
	m_MOEAggregationIntervalInMin = m_AggregationValueVector [ m_AggregationIntervalList.GetCurSel()];
	Invalidate();
}

void CDlgPathList::OnCbnSelchangeComboPlotType()
{
	Invalidate();
}
void CDlgPathList::OnSize(UINT nType, int cx, int cy)
{
	RedrawWindow();

	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}


void CDlgPathList::OnBnClickedAnalysis2()
{
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{

		(*iLink)-> m_bIncludedBySelectedPath = false;
		(*iLink)-> m_bFirstPathLink = false;
		(*iLink)-> m_bLastPathLink = false;

	}

	// mark all links in the selected path
	if(m_pDoc->m_PathDisplayList.size() > m_pDoc->m_SelectPathNo && m_pDoc->m_SelectPathNo!=-1)
	{
		for (int i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size(); i++)
		{

			DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector[i]];
			pLink-> m_bIncludedBySelectedPath = true;

			if(i==0)
			{
				pLink-> m_bFirstPathLink = true;
			}
			if(i== m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size()-1)
			{
				pLink-> m_bLastPathLink = true;
			}

		}
	}

	CDlg_VehicleClassification* m_pDlg = new CDlg_VehicleClassification; 

	m_pDlg->m_PresetChartTitle.Format ("End-to-End Path No.%d", m_pDoc->m_SelectPathNo+1);

	m_pDlg->m_pDoc = m_pDoc;
	m_pDoc->m_VehicleSelectionMode = CLS_path_partial_trip;
	m_pDlg->m_XSelectionNo = CLS_time_interval_15_min;
	m_pDlg->m_VehicleSelectionNo  = CLS_path_partial_trip;
	m_pDlg->SetModelessFlag(true); // voila! this is all it takes to make your dlg modeless!
	m_pDlg->Create(IDD_DIALOG_Summary); 
	m_pDlg->ShowWindow(SW_SHOW); 

}

void CDlgPathList::OnBnClickedFreevalAnalysisGenerateData()
{
	// TODO: Add your control notification handler code here
}

void CDlgPathList::OnBnClickedFreevalAnalysisGenerateFile()
{
	// calculate time-dependent travel time
	int time_step = 1;

	CString export_file_name;

	export_file_name = m_pDoc->m_ProjectDirectory +"FREEVAL_input_segment.csv";

	FILE* st;
	fopen_s(&st,export_file_name,"w");
	if(st!=NULL)
	{
		m_pDoc->m_SelectPathNo = m_PathList.GetCurSel();

		int time_stamp = 0;
		for(time_stamp = m_pDoc->m_DemandLoadingStartTimeInMin;  time_stamp < m_pDoc->m_DemandLoadingEndTimeInMin; time_stamp+=15)
		{
		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			if(m_pDoc->m_SelectPathNo!=p)
				continue;
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			
			int i;

			if(time_stamp == m_pDoc->m_DemandLoadingStartTimeInMin)
			{

				fprintf(st,"Time Interval,Segment Number,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink != NULL)
				{
					fprintf(st,"%d,",i+1 );
				}else
				{
				AfxMessageBox("missing link for in the path file. Please check!");
				fclose(st);
				return;
				}
			}
				fprintf(st,"\n");

			}


		// 1st row: segment label
				fprintf(st,"%s",m_pDoc->GetTimeStampString24HourFormat (time_stamp));

			fprintf(st,",Segment Label,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				fprintf(st,"S%0d,",i+1);
			}
				fprintf(st,"\n");
			
			// 2nd row: type
			fprintf(st,",Type (B;ONR;OFR;R or W),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"%s,", pLink->GetFREEVALCode());
			}
			fprintf(st,"\n");

			// 3rd row: 
			fprintf(st,",Length (ft),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"%.0f,",pLink->m_Length *5280);
			}
			fprintf(st,"\n");

			// 3rd row: 
			fprintf(st,",Number of Lanes,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"%d,",pLink->m_NumberOfLanes );
			}

			fprintf(st,"\n");

			// 4th row: 
			fprintf(st,",FF Speed (Mi/hr),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"%.0f,",pLink->m_SpeedLimit  );
			}

			fprintf(st,"\n");


			// 5th row: 
			fprintf(st,",Segment Demand (vph),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"%.0f,",pLink->GetAvgLinkHourlyVolume(time_stamp, time_stamp+15));
			}

			fprintf(st,"\n");


			// 6th row: 
			fprintf(st,",Capacity Adjustment Factor,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"1.00,");
			}

			fprintf(st,"\n");

			// 7th row: 
			fprintf(st,",Origin Demand Adjustment Factor,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"1.11,");
			}

			fprintf(st,"\n");

			// 8th row: 
			fprintf(st,",Destination Demand Adjustment Factor,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"1.11,");
			}

			fprintf(st,"\n");
			fprintf(st,",Detailed Info\n");

			//
			fprintf(st,",%% Trucks,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"5.0,");
			}

			fprintf(st,"\n");

			//-----------------------------------------
			fprintf(st,",%% of RV's,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"0,");
			}

			fprintf(st,"\n");

			//-----------------------------------------
			fprintf(st,",On-Ramp Demand (vph),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

				if(pLink->m_FREEVALSegmentCode == FREEVAL_ONR)
					fprintf(st,"%.0f,",pLink->GetAvgLinkHourlyVolume(time_stamp, time_stamp+15));
				else 
					fprintf(st,",");
			}

			fprintf(st,"\n");

		//-----------------------------------------
			fprintf(st,",On-Ramp %% Trucks,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink->m_FREEVALSegmentCode == FREEVAL_ONR)
					fprintf(st,"5,");
				else 
					fprintf(st,",");

			}

			fprintf(st,"\n");


		//-----------------------------------------
			fprintf(st,",On-Ramp %% RV's,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink->m_FREEVALSegmentCode == FREEVAL_ONR)
					fprintf(st,"0,");
				else 
					fprintf(st,",");
			}

			fprintf(st,"\n");

		//-----------------------------------------
			fprintf(st,",Off-Ramp Demand (vph),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink->m_FREEVALSegmentCode == FREEVAL_OFR)
					fprintf(st,"%.0f,",pLink->GetAvgLinkHourlyVolume(time_stamp, time_stamp+15));
				else 
					fprintf(st,",");
			}

			fprintf(st,"\n");

		//-----------------------------------------
			fprintf(st,",Off-Ramp %% Trucks,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink->m_FREEVALSegmentCode == FREEVAL_OFR)
					fprintf(st,"5,");
				else
					fprintf(st,",");


			}

			fprintf(st,"\n");

		//-----------------------------------------
			fprintf(st,",Off-Ramp %% RVs,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink->m_FREEVALSegmentCode == FREEVAL_OFR)
					fprintf(st,"0,");
				else
					fprintf(st,",");
			}

			fprintf(st,"\n");


		//-----------------------------------------
			fprintf(st,",Acc/Dec Lane Length (ft),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

				if(pLink->IsRampAttached())
					fprintf(st,"%.0f,",pLink->GetRelatedRampInfo().m_Length *5280);
				else
					fprintf(st,",");
			}

			fprintf(st,"\n");

		//-----------------------------------------
			fprintf(st,",Number of Lanes on Ramps,");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink->IsRampAttached())
					fprintf(st,"%d,",pLink->GetRelatedRampInfo().m_NumberOfLanes);
				else
					fprintf(st,",");
			}

			fprintf(st,"\n");

		//-----------------------------------------
			fprintf(st,",Ramp on Left or Right (L/R),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink->IsRampAttached())
					fprintf(st,"Right,");
				else
					fprintf(st,",");
			}

			fprintf(st,"\n");


		//-----------------------------------------
			fprintf(st,",Ramp FFS(mi/hr),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink->IsRampAttached())
					fprintf(st,"%.0f,",pLink->GetRelatedRampInfo().m_SpeedLimit);
				else
					fprintf(st,",");
			}
			fprintf(st,"\n");

			fprintf(st,"%s,",m_pDoc->GetTimeStampString24HourFormat (time_stamp) );

				fprintf(st,",Link (from->to),");
			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{
				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					fprintf(st,"%d->%d,",pLink->m_FromNodeNumber , pLink->m_ToNodeNumber );
			}
				fprintf(st,"\n");


		} //for each path

		}
		fclose(st);

		if(AfxMessageBox("File FREEVAL_input_segment.csv is generated in the project folder.", MB_ICONINFORMATION)==IDOK)
		{m_pDoc->OnFreewaytoolsView();}


		m_pDoc->OpenCSVFileInExcel (export_file_name);

		
	}else
		{
		
			AfxMessageBox("File FREEVAL_input_segment.csv cannot be opened to write. Please check if it is currently opened in EXCEL.", MB_ICONINFORMATION);
	
			return;
		}
	

}



void CDlgPathList::OnChangeattributesforlinksalongpathChangelanecapacity()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_lane_capacity;
	ChangeLinkAttributeDialog();
}

void CDlgPathList::OnChangeattributesforlinksalongpathChange()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_number_of_lanes;
	ChangeLinkAttributeDialog();

}

void CDlgPathList::OnChangeattributesforlinksalongpathChangelinktype()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_link_type;
	ChangeLinkAttributeDialog();
}

void CDlgPathList::OnChangeattributesforlinksalongpathChangespeedlimit()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_speed_limit_mph;
	ChangeLinkAttributeDialog();
	
}

void CDlgPathList::OnChangeattributesforlinksalongpathChangespeedlimitKmph()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_speed_limit_kmph;
	ChangeLinkAttributeDialog();
}



void CDlgPathList::OnChangeattributesforlinksalongpathChangejamdensity()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_jam_density_vhcpm;
	ChangeLinkAttributeDialog();

}

void CDlgPathList::OnChangeattributesforlinksalongpathChangebackwavespeed()
{
		m_ChangeLinkAttributeMode = eChangeLinkAttribute_backwardwave_speed_mph;
	ChangeLinkAttributeDialog();

}

void CDlgPathList::OnChangeattributesforlinksalongpathChangesaturationflowrate()
{
		m_ChangeLinkAttributeMode = eChangeLinkAttribute_saturation_flow_rate;
	ChangeLinkAttributeDialog();
}



void CDlgPathList::OnChangeattributesforlinksalongpathEffectivegreentime()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_effective_green_time;
	ChangeLinkAttributeDialog();
}
void CDlgPathList::OnChangeattributesforlinksalongpathChangejamdensity33625()
{
	m_ChangeLinkAttributeMode = eChangeLinkAttribute_jam_density_vhcpm;
	ChangeLinkAttributeDialog();

}


void CDlgPathList::ChangeLinkAttributeDialog()
{
		CDlg_UserInput dlg;

		dlg.m_StrQuestion  = "Please specify the input value:";


		float value = 0;

					switch(m_ChangeLinkAttributeMode)
					{
					case eChangeLinkAttribute_lane_capacity: dlg.m_InputValue = "2000"; break;
					case eChangeLinkAttribute_number_of_lanes: dlg.m_InputValue = "3"; break;
					case eChangeLinkAttribute_link_type: dlg.m_InputValue = "1"; break;
					case eChangeLinkAttribute_speed_limit_mph : dlg.m_InputValue = "65";break;
					case eChangeLinkAttribute_speed_limit_kmph: dlg.m_InputValue = "100"; break; 
					case eChangeLinkAttribute_jam_density_vhcpm : dlg.m_InputValue = "180"; break;
					case eChangeLinkAttribute_jam_density_vhcpkm: dlg.m_InputValue = "110"; break; 
					case eChangeLinkAttribute_backwardwave_speed_mph: dlg.m_InputValue = "12";; break;
					case eChangeLinkAttribute_backwardwave_speed_kmph: dlg.m_InputValue = "7.4"; break;
					case eChangeLinkAttribute_saturation_flow_rate: dlg.m_InputValue = "1800"; break;
					case eChangeLinkAttribute_effective_green_time : dlg.m_InputValue = "60"; break; 
					}
		

		if(dlg.DoModal ()==IDOK)
		{
			value = atof(dlg.m_InputValue) ;

			if(AfxMessageBox("Are you sure to make the change?",  MB_YESNO|MB_ICONINFORMATION)==IDYES)
			{
				ChangeLinkAttributeAlongPath(value);
			}
		}
	

}


void CDlgPathList::ChangeLinkAttributeAlongPath(float value)
{
		m_pDoc->m_SelectPathNo = m_PathList.GetCurSel();

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			if(m_pDoc->m_SelectPathNo!=p)
				continue;
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			
			int i;

			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{

				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink != NULL)
				{
					m_pDoc->Modify (true);

					int ToNodeID; 
					DTANode* pNode =NULL;
					switch(m_ChangeLinkAttributeMode)
					{
					case eChangeLinkAttribute_lane_capacity: pLink->m_LaneCapacity = value; break;
					case eChangeLinkAttribute_number_of_lanes: pLink->m_NumberOfLanes = value; break;
					case eChangeLinkAttribute_link_type: pLink->m_link_type = value; break;
					case eChangeLinkAttribute_speed_limit_mph : pLink->m_SpeedLimit = value; break;
					case eChangeLinkAttribute_speed_limit_kmph: pLink->m_SpeedLimit = value*0.621371; break; 
					case eChangeLinkAttribute_jam_density_vhcpm : pLink->m_Kjam = value; break;
					case eChangeLinkAttribute_jam_density_vhcpkm: pLink->m_Kjam = value*0.621371; break; 
					case eChangeLinkAttribute_backwardwave_speed_mph: pLink->m_Wave_speed_in_mph = value; break;
					case eChangeLinkAttribute_backwardwave_speed_kmph: pLink->m_Wave_speed_in_mph = value*0.621371; break;
					case eChangeLinkAttribute_saturation_flow_rate: pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane = value; break;
					case eChangeLinkAttribute_effective_green_time : 

						ToNodeID = pLink->m_ToNodeID ;
						pNode = m_pDoc->m_NodeNoMap[ToNodeID];
						//set default green time 
						if(pNode->m_ControlType == m_pDoc->m_ControlType_PretimedSignal || 
							pNode->m_ControlType == m_pDoc->m_ControlType_ActuatedSignal)
						{
								
							pLink->m_EffectiveGreenTimeInSecond = value; break;
						}
							
					
					
					}


				}  // for all links
			}
		}

		m_pDoc->UpdateAllViews(0);
}




void CDlgPathList::OnChangeattributesforlinksalongpathDeletelinksalongpath()
{
		m_pDoc->m_SelectPathNo = m_PathList.GetCurSel();

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			if(m_pDoc->m_SelectPathNo!=p)
				continue;
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			
			int i;

			for (i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
			{

				DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
				if(pLink != NULL)
				{

					m_pDoc->DeleteLink(pLink);
					m_pDoc->Modify (true);


				}  // for all links
			}
		}

		m_pDoc->m_PathDisplayList.clear ();

		m_pDoc->UpdateAllViews(0);
}

void CDlgPathList::OnBnClickedDynamicDensityContour()
{

	int ytics_stepsize  = 1;

	CString export_file_name, export_plt_file_name;

	export_file_name = m_pDoc->m_ProjectDirectory +"export_path_density.txt";

	export_plt_file_name = m_pDoc->m_ProjectDirectory +"export_path_density.plt";

	int yrange = 0;
	int xrange = m_TimeRight - m_TimeLeft +1;


	FILE* st;
	fopen_s(&st,export_file_name,"w");
	if(st!=NULL)
	{
		
	int step_size = 1;

	
	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			if(p != m_PathList.GetCurSel())
				continue;

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			if(path_element.m_LinkVector.size() >=15)
				ytics_stepsize = 4;

			if(path_element.m_LinkVector.size() >=30)
				ytics_stepsize = 10;

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];


					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{

							 
							CString label = pLink->m_Name .c_str ();

							//if(pLink->m_Name  == "(null)")
							//{
							//label.Format ("%d->%d",pLink->m_FromNodeNumber , pLink->m_ToNodeNumber);
							//}

							//fprintf(st,"%d->%d,%s,%s,", pLink->m_FromNodeNumber , pLink->m_ToNodeNumber , m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str (),label);

							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "%.1f ", pLink->GetSimulationDensity (t));

							}
							fprintf(st,"\n");

							yrange++;
						}

					}


				}

							// last line with zero
							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "0.0 ");

							}

							fprintf(st,"\n");

			} //for each path
		
	fclose(st);
	}


	FILE* st_plt;
	fopen_s(&st_plt,export_plt_file_name,"w");
	if(st_plt!=NULL)
	{

		CString xtics_str; 

		fprintf(st_plt,"set title \"Dynamic Density Contour\" \n");


		fprintf(st_plt,"set xlabel \"Time\"\n");
		fprintf(st_plt,"set ylabel \"Space\"  offset -3\n");

		int xtics_stepsize  = 30;

		if(xrange/xtics_stepsize >20)
			xtics_stepsize = 120;  // 2 hour interval
		else if(xrange/xtics_stepsize >10)
			xtics_stepsize = 60;   // 1 hour interval
		else if(xrange/xtics_stepsize < 5)
			xtics_stepsize = 10;   // 1 hour interval

			 
		for(int t = m_TimeLeft ; t<= m_TimeRight; t+= xtics_stepsize)  
		{
			CString str;
			str.Format("\"%s\" %d ",m_pDoc->GetTimeStampString24HourFormat (t), t-m_TimeLeft);

			if(t+ xtics_stepsize> m_TimeRight ) 
				xtics_str += str;
			else 
			{
				xtics_str += str ;
				xtics_str += "," ;
			}
		}

		fprintf(st_plt,"set xtics (%s) \n",xtics_str);

		CString ytics_str; 

		int yrange_i  = 0;

		CString last_node_number = " ";

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			if(p != m_PathList.GetCurSel())
				continue;

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{

							CString label = pLink->m_Name .c_str ();


							if(pLink->m_Name  == "(null)" || pLink->m_Name.size() ==0)
							{
							label.Format ("%d",pLink->m_FromNodeNumber);

							last_node_number.Format ("%d",pLink->m_ToNodeNumber);
							}

							if(s==0 && (i%ytics_stepsize) ==0)   // first segment 
							{
							CString str;
							str.Format("\"%s\" %d, ",label, yrange_i) ;

							ytics_str += str;
							
							}

							yrange_i++;
						}

					}


				}

				
			} //for each path


		CString str;
		str.Format("\"%s\" %d", last_node_number, yrange_i);
		ytics_str +=str;
		fprintf(st_plt,"set ytics (%s)\n",ytics_str);

		fprintf(st_plt,"set xrange [0:%d] \n",xrange);
		fprintf(st_plt,"set yrange [0:%d] \n",yrange);

		fprintf(st_plt,"set palette defined (0 \"white\", 10 \"green\", 30 \"yellow\", 50 \"red\")\n");


		fprintf(st_plt,"set pm3d map\n");
		fprintf(st_plt,"splot '%s' matrix\ notitle\n",export_file_name);

		fclose(st_plt);
	}else
	{
	AfxMessageBox("File export_path_density.plt cannot be opened. Please check");
	}

	HINSTANCE result = ShellExecute(NULL, _T("open"), export_plt_file_name, NULL,NULL, SW_SHOW);
	

}
void CDlgPathList::OnBnClickedDynamicSpeedContour()
{
	CString export_file_name, export_plt_file_name;

	export_file_name = m_pDoc->m_ProjectDirectory +"export_path_speed.txt";
	// 

	export_plt_file_name = m_pDoc->m_ProjectDirectory +"export_path_speed.plt";

	int yrange = 0;

	int ytics_stepsize  = 1;
	bool bFreewayFlag = true;
	FILE* st;
	fopen_s(&st,export_file_name,"w");
	if(st!=NULL)
	{
		
	int step_size = 1;
	
	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			if(p != m_PathList.GetCurSel())
				continue;

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];


			if(path_element.m_LinkVector.size() >=15)
				ytics_stepsize = 4;

			if(path_element.m_LinkVector.size() >=30)
				ytics_stepsize = 10;
				//for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
				//{
				//	fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
				//}

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];


					if(pLink != NULL)
					{

						if(m_pDoc->m_LinkTypeMap[pLink->m_link_type].IsFreeway () == false)
							bFreewayFlag = false;

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{
							 
							CString label = pLink->m_Name .c_str ();

							//if(pLink->m_Name  == "(null)")
							//{
							//label.Format ("%d->%d",pLink->m_FromNodeNumber , pLink->m_ToNodeNumber);
							//}

							//fprintf(st,"%d->%d,%s,%s,", pLink->m_FromNodeNumber , pLink->m_ToNodeNumber , m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str (),label);

							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "%.1f ", pLink->GetSimulationSpeed (t));

							}
							fprintf(st,"\n");
							yrange++;
						}

					}


				}
							// last line with zero
							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "0.0 ");

							}

							fprintf(st,"\n");

			} //for each path
		
	fclose(st);
	}

	int xrange = m_TimeRight - m_TimeLeft +1;


	FILE* st_plt;
	fopen_s(&st_plt,export_plt_file_name,"w");
	if(st_plt!=NULL)
	{

		CString xtics_str; 

		fprintf(st_plt,"set title \"Dynamic Speed Contour\" \n");


		fprintf(st_plt,"set xlabel \"Time\"\n");
		fprintf(st_plt,"set ylabel \"Space\" offset -3\n");

		int xtics_stepsize  = 30;

		if(xrange/xtics_stepsize >20)
			xtics_stepsize = 120;  // 2 hour interval
		else if(xrange/xtics_stepsize >10)
			xtics_stepsize = 60;   // 1 hour interval
		else if(xrange/xtics_stepsize < 5)
			xtics_stepsize = 10;   // 1 hour interval

			 
		for(int t = m_TimeLeft ; t<= m_TimeRight; t+= xtics_stepsize)  
		{
			CString str;
			str.Format("\"%s\" %d ",m_pDoc->GetTimeStampString24HourFormat (t), t-m_TimeLeft);

			if(t+ xtics_stepsize> m_TimeRight ) 
				xtics_str += str;
			else 
			{
				xtics_str += str ;
				xtics_str += "," ;
			}
		}

		fprintf(st_plt,"set xtics (%s) \n",xtics_str);

		CString ytics_str; 

		int yrange_i  = 0;

	CString last_node_number = " ";

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			if(p != m_PathList.GetCurSel())
				continue;

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{

							CString label = pLink->m_Name .c_str ();


							if(pLink->m_Name  == "(null)" || pLink->m_Name.size() ==0)
							{
							label.Format ("%d",pLink->m_FromNodeNumber);

							last_node_number.Format ("%d",pLink->m_ToNodeNumber);
							}

							if(s==0 && (i%ytics_stepsize) ==0)   // first segment 
							{
							CString str;
							str.Format("\"%s\" %d, ",label, yrange_i) ;

							ytics_str += str;
							
							}

							yrange_i++;
						}

					}


				}

				
			} //for each path


		CString str;
		str.Format("\"%s\" %d", last_node_number, yrange_i);
		ytics_str +=str;
		fprintf(st_plt,"set ytics (%s)\n",ytics_str);

		fprintf(st_plt,"set xrange [0:%d] \n",xrange);
		fprintf(st_plt,"set yrange [0:%d] \n",yrange);

		if(bFreewayFlag)
			fprintf(st_plt,"set palette defined (0 \"white\", 0.1 \"red\", 40 \"yellow\", 50 \"green\")\n");
		else
			fprintf(st_plt,"set palette defined (0 \"white\", 0.1 \"red\", 20 \"yellow\", 40 \"green\")\n");

		fprintf(st_plt,"set pm3d map\n");
		fprintf(st_plt,"splot '%s' matrix\ notitle\n",export_file_name);

		fclose(st_plt);
	}else
	{
	AfxMessageBox("File export_path_speed.plt cannot be opened. Please check");
	}



	HINSTANCE result = ShellExecute(NULL, _T("open"), export_plt_file_name, NULL,NULL, SW_SHOW);
}

void CDlgPathList::OnBnClickedDynamicFlowContour()
{
	int ytics_stepsize = 1;
	CString export_file_name, export_plt_file_name;

	export_file_name = m_pDoc->m_ProjectDirectory +"export_path_v_over_c.txt";
	// 

	export_plt_file_name = m_pDoc->m_ProjectDirectory +"export_path_v_over_c.plt";

	int yrange = 0;
	int xrange = m_TimeRight - m_TimeLeft +1;

	FILE* st;
	fopen_s(&st,export_file_name,"w");
	if(st!=NULL)
	{
		
	int step_size = 1;
	
	for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			if(p != m_PathList.GetCurSel())
				continue;

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			if(path_element.m_LinkVector.size() >=15)
				ytics_stepsize = 4;

			if(path_element.m_LinkVector.size() >=30)
				ytics_stepsize = 10;

				//for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
				//{
				//	fprintf(st,"%s,", m_pDoc->GetTimeStampString24HourFormat (t));
				//}

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];


					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{
							 
							CString label = pLink->m_Name .c_str ();

							//if(pLink->m_Name  == "(null)")
							//{
							//label.Format ("%d->%d",pLink->m_FromNodeNumber , pLink->m_ToNodeNumber);
							//}

							//fprintf(st,"%d->%d,%s,%s,", pLink->m_FromNodeNumber , pLink->m_ToNodeNumber , m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str (),label);

							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{
								float LaneClosurePercentage = pLink->GetImpactedFlag(t); // check capacity reduction event;
								float Capacity=  pLink->m_NumberOfLanes * pLink->m_LaneCapacity *(1-	LaneClosurePercentage);		

								float volume  = pLink->GetSimulatedLinkOutVolume (t);
								float voc = volume/max(1,Capacity);
								if(voc>1.01 && m_pDoc->m_traffic_flow_model!=0)  // not BPR function 
									voc = 1.0;
								fprintf(st, "%.2f ", voc);

								if( pLink->m_FromNodeNumber == 30 && pLink->m_ToNodeNumber == 31)
								{
						
								TRACE("\n%f; %f; %f", volume, Capacity, voc);
								}

							}
							fprintf(st,"\n");

							yrange++;
						}

					}


				}

						//last line with zero
							for(int t = m_TimeLeft ; t< m_TimeRight; t+= step_size)  // for each starting time
							{

								fprintf(st, "0.00 ");

							}

							fprintf(st,"\n");
			} //for each path
		
	fclose(st);
	}


	FILE* st_plt;
	fopen_s(&st_plt,export_plt_file_name,"w");
	if(st_plt!=NULL)
	{

		CString xtics_str; 

		fprintf(st_plt,"set title \"Dynamic Volume Over Capcity Contour\" \n");


		fprintf(st_plt,"set xlabel \"Time\"\n");
		fprintf(st_plt,"set ylabel \"Space\" offset -3\n");

		int xtics_stepsize  = 30;

		if(xrange/xtics_stepsize >20)
			xtics_stepsize = 120;  // 2 hour interval
		else if(xrange/xtics_stepsize >10)
			xtics_stepsize = 60;   // 1 hour interval
		else if(xrange/xtics_stepsize < 5)
			xtics_stepsize = 10;   // 1 hour interval

			 
		for(int t = m_TimeLeft ; t<= m_TimeRight; t+= xtics_stepsize)  
		{
			CString str;
			str.Format("\"%s\" %d ",m_pDoc->GetTimeStampString24HourFormat (t), t-m_TimeLeft);

			if(t+ xtics_stepsize> m_TimeRight ) 
				xtics_str += str;
			else 
			{
				xtics_str += str ;
				xtics_str += "," ;
			}
		}

		fprintf(st_plt,"set xtics (%s) \n",xtics_str);

		CString ytics_str; 

		int yrange_i  = 0;

		CString last_node_number = " ";

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			if(p != m_PathList.GetCurSel())
				continue;

			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

				for (int i= 0 ; i < path_element.m_LinkVector.size() ; i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.1; s++)
						{

							CString label = pLink->m_Name .c_str ();

							last_node_number.Empty ();
							if(pLink->m_Name  == "(null)" || pLink->m_Name.size() ==0)
							{
							label.Format ("%d",pLink->m_FromNodeNumber);

							last_node_number.Format ("%d",pLink->m_ToNodeNumber);
							}

							if(s==0 && (i%ytics_stepsize) ==0)   // first segment 
							{
							CString str;
							str.Format("\"%s\" %d, ",label, yrange_i) ;

							ytics_str += str;
							
							}

							yrange_i++;
						}

					}


				}

				
			} //for each path


		CString str;
		str.Format("\"%s\" %d", last_node_number, yrange_i);
		ytics_str +=str;
		fprintf(st_plt,"set ytics (%s)\n",ytics_str);

		fprintf(st_plt,"set xrange [0:%d] \n",xrange);
		fprintf(st_plt,"set yrange [0:%d] \n",yrange);

		fprintf(st_plt,"set palette defined (0 \"white\", 0.4 \"green\", 0.6 \"yellow\", 1 \"red\")\n");
		fprintf(st_plt,"set pm3d map\n");
		fprintf(st_plt,"splot '%s' matrix\ notitle\n",export_file_name);

		fclose(st_plt);
	}else
	{
	AfxMessageBox("File export_path_v_over_c.plt cannot be opened. Please check");
	}

	HINSTANCE result = ShellExecute(NULL, _T("open"), export_plt_file_name, NULL,NULL, SW_SHOW);
}

void CDlgPathList::OnDataSavecurrentpath()
{
		if(m_pDoc->m_SelectPathNo >=0 && m_pDoc->m_SelectPathNo < m_pDoc->m_PathDisplayList.size())
			m_pDoc->m_PathDisplayList [ m_pDoc->m_SelectPathNo].m_bSavedPath = true;
}

void CDlgPathList::OnDataExportfreevalsegmentfile()
{
	OnBnClickedFreevalAnalysisGenerateFile();
}

void CDlgPathList::OnBnClickedGoogleearth()
{
	m_pDoc->OnExportGenerateshapefilesPathData();
}

void CDlgPathList::OnBnClickedGoogleearthoutput()
{
	m_pDoc->OnExportGenerateshapefilesPathData();
}

void CDlgPathList::OnBnClickedButtonGoogleEarthKml()
{
	CWaitCursor wait;
	m_pDoc->OnExportGenerateshapefilesPathData();
}

void CDlgPathList::OnDataDeleteexistingpathsininputpathcsvfile()
{
	CString input_path_file_name;

	input_path_file_name.Format("%sinput_path.csv",m_pDoc->m_ProjectDirectory);

	bool reval = DeleteFile(input_path_file_name);
	if(reval)
	{
	AfxMessageBox("File input_path.csv has been deleted.",  MB_ICONINFORMATION );
	}else
	{
	AfxMessageBox("File input_path.csv cannot be deleted. Please check if the file is opened in Excel.",  MB_ICONINFORMATION );
	
	}
}
