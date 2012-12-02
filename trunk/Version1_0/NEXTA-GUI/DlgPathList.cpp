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
#include <string>
#include <sstream>
#include <vector>

;
extern CDlgPathList* g_pPathListDlg;
// CDlgPathList dialog

IMPLEMENT_DYNAMIC(CDlgPathList, CDialog)

CDlgPathList::CDlgPathList(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlgPathList::IDD, pParent)
	, m_StrPathMOE(_T(""))
{
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
}


BEGIN_MESSAGE_MAP(CDlgPathList, CDialog)
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
END_MESSAGE_MAP()


// CDlgPathList message handlers
#define PATHDATACOLUMNSIZE 14

BOOL CDlgPathList::OnInitDialog()
{
	CDialog::OnInitDialog();
	
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
	_T("Path No"),_T("No."),_T("Link No"),_T("From->To"),_T("Street Name"), _T("Length (ml)"), _T("Speed Limit"), _T("Travel Time (min)"),_T("# of lanes"),_T("Lane Saturation Flow Rate"),_T("Lane Capacity"), _T("Link Type"), _T("Sensor Type"),  _T("Count") 
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
		str.Format("Path ID.%d: %s, %d links",i+1, m_pDoc->m_PathDisplayList[i].m_path_name.c_str (), m_pDoc->m_PathDisplayList[i].m_LinkVector .size());
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

		sprintf_s(text, "%s",m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_PathLabelVector[i]);
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,text , 0, 0, 0, NULL);


		sprintf_s(text, "%d",i+1);
		m_ListCtrl.SetItemText(Index,column_count++,text );


		// link no
		sprintf_s(text, "%d",pLink->m_LinkNo );
		m_ListCtrl.SetItemText(Index,column_count++,text );

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
		sprintf_s(text, "%d",pLink->m_NumLanes );
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
		m_StrPathMOE.Format("Distance=%4.2f mile, Travel Time=%4.2f min, # of Sensors = %d, detected count = %.4f ",
			total_distance,total_travel_time, number_of_sensors,total_count);

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


		fprintf(st,"\n\nPart I,time-dependent travel time");
		// 

		time_step= 15;

			fprintf(st,"\n\nTime,,");

			for(int t = m_pDoc->m_SimulationStartTime_in_min ; t< m_pDoc->m_SimulationEndTime_in_min; t+= time_step)  // for each starting time
			{
			fprintf(st,"%s,", m_pDoc->GetTimeStampString (t));
			}

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			
		
			fprintf(st,"\nPath %d, %s,",p+1,path_element.m_path_name .c_str ());

			for(int t = m_pDoc->m_SimulationStartTime_in_min ; t< m_pDoc->m_SimulationEndTime_in_min; t+= time_step)  // for each starting time
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
				m_pDoc->m_PathDisplayList[p].m_TimeDependentTravelTime[t] =   path_element.m_TimeDependentTravelTime[t] ;

				fprintf(st,"%.2f,", path_element.m_TimeDependentTravelTime[t]);


			}  // for each time

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

			for(int t = m_pDoc->m_SimulationStartTime_in_min ; t< m_pDoc->m_SimulationEndTime_in_min; t+= time_step)  // for each starting time
			{
			fprintf(st,"%s,", m_pDoc->GetTimeStampString (t));
			}


		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
			
		
			fprintf(st,"\nPath %d, %s,",p+1, path_element.m_path_name .c_str ());
			for(int t = m_pDoc->m_SimulationStartTime_in_min ; t< m_pDoc->m_SimulationEndTime_in_min; t+= time_step)  // for each starting time
			{
			fprintf(st,"%.2f,", path_element.m_TimeDependentTravelTime[t]/max(1,path_element.total_free_flow_travel_time));

			}  // for each time

		}

		fprintf(st,"Part II,link sequence\n\n");
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
				pLink->m_SpeedLimit, pLink->m_FreeFlowTravelTime , pLink->m_NumLanes,  pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane ,pLink->m_LaneCapacity );

						if(m_pDoc->m_LinkTypeMap.find(pLink->m_link_type) != m_pDoc->m_LinkTypeMap.end())
						{
						fprintf(st, "%s", m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str ());
						}
					}

				fprintf(st,"\n");


				}
		} //for each path

		// part II: time-dependent speed contour
		int step_size = 5;

		for(int aggregation_index = 0; aggregation_index<3; aggregation_index++)
		{
			switch (aggregation_index)
			{
			case 0: step_size = 5; break;
			case 1: step_size = 15; break;
			case 2: step_size = 30; break;
			
			default: step_size = 60;
			}
			 

			int previous_MOEAggregationIntervalInMin = g_MOEAggregationIntervalInMin;
			g_MOEAggregationIntervalInMin = step_size;

	fprintf(st,"path_id,link_sequence_no,from_node_id->to_node_id,name,length (ml),speed_limit,free-flow travel_time,# of lanes,Lane Saturation Flow Rate,Lane Capacity,Link Type\n");

			for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
		fprintf(st,"\n\nPart III,time-dependent speed contour for path %d, %s,(%d min)\n\n", p+1, path_element.m_path_name .c_str (),step_size);



				for (int i=path_element.m_LinkVector.size()-1 ; i >=0 ; i--)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];

				
					if(pLink != NULL)
					{

						for(int s = 0; s< pLink->m_Length / 0.2; s++)
						{

							if(s==0)
							{
							fprintf(st,"%d->%d,%s,%s,", pLink->m_FromNodeNumber , pLink->m_ToNodeNumber , m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str (), pLink->m_Name .c_str ());
							}else
							{
							fprintf(st,",,,");
							}

							for(int t = m_pDoc->m_SimulationStartTime_in_min ; t< m_pDoc->m_SimulationEndTime_in_min; t+= step_size)  // for each starting time
							{

								fprintf(st, "%.1f,", pLink->GetSimulationSpeed (t));

							}
							fprintf(st,"\n");
						}

					}
				
				
				}

					// time axis
					fprintf(st,",,,");
							for(int t = m_pDoc->m_SimulationStartTime_in_min ; t<= m_pDoc->m_SimulationEndTime_in_min; t+= step_size)  // for each starting time
							{
								if(t%15 == 0 || (t-1)%15 == 0 )
									fprintf(st,"%s,", m_pDoc->GetTimeStampString (t));
								else
									fprintf(st,",");
							
							}
							fprintf(st,"\n");



		} //for each path

		g_MOEAggregationIntervalInMin = previous_MOEAggregationIntervalInMin;
		}


		fprintf(st,"\n\nPart IV");
		fprintf(st,"\n\npath_no,departure time,travel_time_in_min,travel_time_index,total observed count on passing links\n");
		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			for(int t = m_pDoc->m_SimulationStartTime_in_min ; t< m_pDoc->m_SimulationEndTime_in_min; t+= time_step)  // for each starting time
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

				fprintf(st,"%d,%s,%.2f,%.2f,%.2f\n",p+1, m_pDoc->GetTimeStampString(t),
					path_element.m_TimeDependentTravelTime[t],path_element.m_TimeDependentTravelTime[t]/max(1,path_element.total_free_flow_travel_time), path_element.m_TimeDependentCount [t]);


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

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			m_pDoc->m_PathDisplayList[p].m_LinkVector.clear();
		}
	

		CFileDialog path_file_dlg (TRUE, "input_path.csv", "input_path.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"Path Link Data File (*.csv)|*.csv||", NULL);
	if(path_file_dlg.DoModal() == IDOK)
	{

		m_pDoc->m_PathDisplayList.clear();
		m_pDoc->m_SelectPathNo=0;


		char lpszFileName[_MAX_PATH];
		
		sprintf(lpszFileName,"%s",path_file_dlg.GetPathName ());

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

			int path_id = 0;
			if(!parser.GetValueByFieldName("path_id",path_id))
			{
				AfxMessageBox("Field path_id has not been defined in file input_path.csv. Please check.");
				break;
			}

			string path_name;
			parser.GetValueByFieldName("path_name",path_name);

			std::string TMC;
			parser.GetValueByFieldName("TMC",TMC);

			if(TMC.size () >=1)
			{

				if( m_pDoc->m_TMC2LinkMap.find (TMC) != m_pDoc->m_TMC2LinkMap.end())
				{
				pLink = m_pDoc->m_TMC2LinkMap[TMC];
				}else
				{
				CString str;
				str.Format ("TMC %s cannot be found in the current data set.",TMC.c_str () );
				AfxMessageBox(str);
				return;
				}

			}
			else
			{

			if(!parser.GetValueByFieldName("from_node_id",from_node_id)) 
			{
				AfxMessageBox("Field from_node_id has not been defined in file input_path.csv. Please check.");
				break;
			}
			if(!parser.GetValueByFieldName("to_node_id",to_node_id))
			{
				AfxMessageBox("Field to_node_id has not been defined in file input_path.csv. Please check.");
				break;
			}

			pLink = m_pDoc->FindLinkWithNodeNumbers(from_node_id,to_node_id,lpszFileName,true);
			}

			if(pLink==NULL)
			{
				return;
			}

			if(count==0)
			{
			m_pDoc->m_OriginNodeID  = pLink->m_FromNodeID ;
			}


			if(prev_path_id!= path_id)  //find new route
			{
				DTAPath path_element;
				m_pDoc->m_PathDisplayList.push_back(path_element);
				prev_path_id = path_id;
			}

			int route_no = m_pDoc->m_PathDisplayList.size()-1;
			m_pDoc->m_PathDisplayList[route_no].m_LinkVector.push_back (pLink->m_LinkNo );

			CString c_path_name;
			c_path_name.Format("%d",route_no+1);


			m_pDoc->m_PathDisplayList[route_no].m_PathLabelVector.push_back (c_path_name);

			if(m_pDoc->m_PathDisplayList[route_no].m_path_name.size() ==0)  // no value yet
			{
				if(path_name.size()>=1)
				{
					m_pDoc->m_PathDisplayList[route_no].m_path_name = path_name;
				}else
				{
					m_pDoc->m_PathDisplayList[route_no].m_path_name = m_pDoc->CString2StdString (c_path_name);
				}
			}

			count++;
		}

		if(pLink!=NULL)  // last link
		{
			m_pDoc->m_DestinationNodeID  = pLink->m_ToNodeID ;	
		}

	}else
	{
	AfxMessageBox("File cannot be found.");
	}
		ReloadData();

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

	m_pDoc->UpdateAllViews (0);

}

void CDlgPathList::OnBnClickedDataAnalysis()
{
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{

		(*iLink)-> m_bIncludedBySelectedPath = false;

	}

	// mark all links in the selected path
	if(m_pDoc->m_PathDisplayList.size() > m_pDoc->m_SelectPathNo && m_pDoc->m_SelectPathNo!=-1)
	{
		for (int i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector.size(); i++)
		{

		DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector[i]];
		pLink-> m_bIncludedBySelectedPath = true;
		}
	}

	CDlg_VehicleClassification dlg;
	dlg.m_pDoc = m_pDoc;
	m_pDoc->m_VehicleSelectionMode = CLS_path;
	dlg.m_VehicleSelectionNo  = CLS_path;
	dlg.DoModal ();

}

void CDlgPathList::OnDataGeneratesampleinputpathcsv()
{
	// calculate time-dependent travel time

	int time_step = 1;

	CString input_sample_file_name;

	input_sample_file_name = m_pDoc->m_ProjectDirectory +"input_path_sample.csv";
		// save demand here

	FILE* st;
	fopen_s(&st,input_sample_file_name,"w");
	if(st==NULL)
	{
		//the file has exists.
		m_pDoc->OpenCSVFileInExcel (input_sample_file_name);

		return;
		
	}

	if(m_pDoc->m_PathDisplayList.size()==0)
	{
		AfxMessageBox("To generate the file input_path_sample.csv, Please first define one or two paths by selecting the origin and destination nodes.", MB_ICONINFORMATION);
		return;
	}

	if(st!=NULL)
	{


		fprintf(st,"path_id,path_name,link_sequence_no,from_node_id,to_node_id,link_id,link_name\n");

				for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
				for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					if(pLink != NULL)
					{

						fprintf(st,"%d,path %d,%d,%d,%d,%d-%d,%s\n",
							p+1,p+1,i+1,pLink->m_FromNodeNumber , pLink->m_ToNodeNumber,pLink->m_FromNodeNumber , pLink->m_ToNodeNumber, pLink->m_Name .c_str ());

					}
				}


		} //for each path

		fclose(st);
		m_pDoc->OpenCSVFileInExcel (input_sample_file_name);
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
