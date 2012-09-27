// DlgPathList.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "CSVParser.h"
#include "DlgPathList.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"

#include <string>
#include <sstream>
#include <vector>

extern std::list<int>	g_LinkDisplayList;
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
}


BEGIN_MESSAGE_MAP(CDlgPathList, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CDlgPathList::OnLvnItemchangedList)
	ON_BN_CLICKED(IDOK, &CDlgPathList::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgPathList::OnBnClickedCancel)
	ON_BN_CLICKED(ID_FIND_RANDOM_ROUTE, &CDlgPathList::OnBnClickedFindRandomRoute)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgPathList::OnCbnSelchangeCombo1)
	ON_COMMAND(ID_PATH_DATA_EXPORT_CSV, &CDlgPathList::OnPathDataExportCSV)
	ON_COMMAND(ID_DATA_IMPORT_CSV, &CDlgPathList::OnDataImportCsv)
END_MESSAGE_MAP()


// CDlgPathList message handlers
#define PATHDATACOLUMNSIZE 10

BOOL CDlgPathList::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	for(int c = 0; c <=10; c++)
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
	_T("No."),_T("From->To"),_T("Street Name"), _T("Length (ml)"), _T("Speed Limit"), _T("Travel Time (min)"),_T("# of lanes"),_T("Lane Saturation Flow Rate"),_T("Lane Capacity"), _T("Link Type")
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

    m_ListCtrl.DeleteAllItems();
      
	if(m_pDoc->m_PathDisplayList.size() > m_pDoc->m_SelectPathNo && m_pDoc->m_SelectPathNo!=-1)
	{
		float total_distance = 0;
		float total_travel_time = 0;

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
		sprintf_s(text, "%5.2f",pLink->m_Length);
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
		sprintf_s(text, "%s", m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str ());
		m_ListCtrl.SetItemText(Index,column_count++,text);

		}

		m_StrPathMOE.Format("Distance=%4.2f mile, Travel Time=%4.2f min",total_distance,total_travel_time);
		UpdateData(0);

	}

}
}

void CDlgPathList::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	
	UpdateData(1);
	
	m_pDoc->m_SelectedLinkID = -1;
	g_LinkDisplayList.clear ();

	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	while(pos!=NULL)
	{
		int nSelectedRow = m_ListCtrl.GetNextSelectedItem(pos);
		char str[100];
		m_ListCtrl.GetItemText (nSelectedRow,0,str,20);
		int LinkNo = atoi(str);
			m_pDoc->m_SelectedLinkID = LinkNo;
			g_LinkDisplayList.push_back(LinkNo);

	}
	if(m_ZoomToSelectedLink == true)
		m_pDoc->ZoomToSelectedLink(m_pDoc->m_SelectedLinkID);

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

		fprintf(st,"path_no,link_sequence_no,from_node_id,to_node_id,name,length (ml),speed_limit,free-flow travel_time,# of lanes,Lane Saturation Flow Rate,Lane Capacity,Link Type\n");

				for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];
				for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					if(pLink != NULL)
					{

				fprintf(st,"%d,%d,%d,%d,%s,%5.3f,%5.0f,%5.3f,%d,%5.0f,%5.1f,", p+1,i+1,pLink->m_FromNodeNumber , pLink->m_ToNodeNumber,  pLink->m_Name.c_str (), pLink->m_Length ,
				pLink->m_SpeedLimit, pLink->m_FreeFlowTravelTime , pLink->m_NumLanes,  pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane ,pLink->m_LaneCapacity );

				if(m_pDoc->m_LinkTypeMap.find(pLink->m_link_type) != m_pDoc->m_LinkTypeMap.end())
				{
				fprintf(st, "%s", m_pDoc->m_LinkTypeMap[pLink->m_link_type ].link_type_name.c_str ());
				}
					}

				fprintf(st,"\n");


				}
		} //for each path

				fprintf(st,"\n\n\n\n");
				fprintf(st,"path_no,time,travel_time\n");

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_pDoc->m_PathDisplayList[p];

			for(int t = m_pDoc->m_SimulationStartTime_in_min ; t< m_pDoc->m_SimulationEndTime_in_min; t+= time_step)  // for each starting time
			{
				path_element.m_TimeDependentTravelTime[t] = t;  // t is the departure time

				for (int i=0 ; i < path_element.m_LinkVector.size(); i++)  // for each pass link
				{
					DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[p].m_LinkVector[i]];
					if(pLink == NULL)
						break;

					path_element.m_TimeDependentTravelTime[t] += pLink->GetTravelTime(path_element.m_TimeDependentTravelTime[t]);

					// current arrival time at a link/node along the path, t in [t] is still index of departure time, t has a dimension of 0 to 1440* number of days

					//			    TRACE("\n path %d, time at %f, TT = %f",p, path_element.m_TimeDependentTravelTime[t], pLink->GetTravelTime(path_element.m_TimeDependentTravelTime[t]) );

				}

				path_element.m_TimeDependentTravelTime[t] -= t; // remove the starting time, so we have pure travel time;

				fprintf(st,"%d,%s,%.2f\n",p+1, m_pDoc->GetTimeStamString(t), path_element.m_TimeDependentTravelTime[t]);


				ASSERT(path_element.m_TimeDependentTravelTime[t]>=0);

				if( path_element.m_MaxTravelTime < path_element.m_TimeDependentTravelTime[t])
					path_element.m_MaxTravelTime = path_element.m_TimeDependentTravelTime[t];

				for(int tt=1; tt<time_step; tt++)
				{
					path_element.m_TimeDependentTravelTime[t+tt] = path_element.m_TimeDependentTravelTime[t];
				}


			}  // for each time
		} // for each path

				fclose(st);
	}
		m_pDoc->OpenCSVFileInExcel (export_file_name);
}


void CDlgPathList::OnDataImportCsv()
{

		for(unsigned int p = 0; p < m_pDoc->m_PathDisplayList.size(); p++) // for each path
		{

			m_pDoc->m_PathDisplayList[p].m_LinkVector.clear();
		}
	
		CFileDialog path_file_dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"Path Link Data File (*.csv)|*.csv||", NULL);
	if(path_file_dlg.DoModal() == IDOK)
	{

		m_pDoc->m_PathDisplayList.clear();
			DTAPath path_element;

			m_pDoc->m_PathDisplayList.push_back(path_element);

			m_pDoc->m_SelectPathNo=0;


		char lpszFileName[_MAX_PATH];
		
		sprintf(lpszFileName,"%s",path_file_dlg.GetPathName ());
		
	CCSVParser parser;
	if (parser.OpenCSVFile(lpszFileName))
	{
		bool bNodeNonExistError = false;
		while(parser.ReadRecord())
		{
			int link_id = 0;
			int from_node_id;
			int to_node_id;
			if(!parser.GetValueByFieldName("from_node_id",from_node_id)) 
			{
				AfxMessageBox("Field from_node_id has not been defined in file input_path_MOE.csv. Please check.");
				break;
			}
			if(!parser.GetValueByFieldName("to_node_id",to_node_id))
			{
				AfxMessageBox("Field to_node_id has not been defined in file input_path_MOE.csv. Please check.");
				break;
			}

			DTALink* pLink = m_pDoc->FindLinkWithNodeNumbers(from_node_id,to_node_id,lpszFileName,true);

			if(pLink==NULL)
				return;




			m_pDoc->m_PathDisplayList[0].m_LinkVector.push_back (pLink->m_LinkNo );

		}

	}
		ReloadData();


		






	}
}
