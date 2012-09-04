// DlgGridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Geometry.h"
#include "CSVParser.h"
#include "TLite.h"
#include "DlgGridCtrl.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"

#include "DlgZoneToNodeMapping.h"
#include "Dlg_ImportODDemand.h"
#include "Dlg_TDDemandProfile.h"

#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

using std::map;
using std::list;
using std::string;
using std::ofstream;
using std::ifstream;
using std::istringstream;

// CDlgODDemandGridCtrl dialog

IMPLEMENT_DYNAMIC(CDlgODDemandGridCtrl, CDialog)

BEGIN_MESSAGE_MAP(CDlgODDemandGridCtrl, CDialog)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_CTRL, &CDlgODDemandGridCtrl::OnGridEndEdit)
	ON_BN_CLICKED(ID_GRID_SAVEQUIT, &CDlgODDemandGridCtrl::OnBnClickedGridSavequit)
	ON_BN_CLICKED(ID_GRID_QUIT, &CDlgODDemandGridCtrl::OnBnClickedGridQuit)
	ON_BN_CLICKED(IDC_BUTTON_CreateZones, &CDlgODDemandGridCtrl::OnBnClickedButtonCreatezones)
	ON_BN_CLICKED(IDC_BUTTON_Edit_Zone_Node_Mapping, &CDlgODDemandGridCtrl::OnBnClickedButtonEditZoneNodeMapping)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgODDemandGridCtrl::OnBnClickedButton1)
	ON_BN_CLICKED(ID_GRID_SAVEQUIT2, &CDlgODDemandGridCtrl::OnBnClickedGridSavequit2)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DemandTypeLIST, &CDlgODDemandGridCtrl::OnLvnItemchangedDemandtypelist)
END_MESSAGE_MAP()



CDlgODDemandGridCtrl::CDlgODDemandGridCtrl(CWnd* pParent /*=NULL*/)
: CDialog(CDlgODDemandGridCtrl::IDD, pParent)
, m_DemandMultipler(0)
{
	m_bSizeChanged = false;
	m_SelectedDemandMetaType = -1;
}

CDlgODDemandGridCtrl::~CDlgODDemandGridCtrl()
{
}

void CDlgODDemandGridCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_GRID_CTRL,m_ODMatrixGrid);
	DDX_Control(pDX,IDC_DemandTypeLIST,m_DemandTypeGrid);
	DDX_Text(pDX, IDC_EDIT_DemandMultipler, m_DemandMultipler);
	DDV_MinMaxFloat(pDX, m_DemandMultipler, 0, 100);
}

void CDlgODDemandGridCtrl::DisplayDemandTypeTable()
{
	if(m_pDoc==NULL)
	return;

	std::vector<std::string> m_Column_names;

static _TCHAR *_gColumnTrainLabel[7] =
{
	_T("No."), _T("File Type."), _T("Demand Type"), _T("Start Time (min)"),
	_T("End Time (min)"), _T("Subtotal Demand Volume"), _T("File Name")};

	LV_COLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	//Add Columns and set headers
	for (size_t i=0;i<7;i++)
	{

		lvc.iSubItem = i;
		lvc.pszText = _gColumnTrainLabel[i];
		lvc.cx = 120;
		lvc.fmt = LVCFMT_LEFT;
		m_DemandTypeGrid.InsertColumn(i,&lvc);
	}


	m_DemandTypeGrid.SetExtendedStyle(LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT |LVS_EX_HEADERDRAGDROP);

	m_DemandTypeGrid.SetColumnWidth(0, 80);
      
	int FirstDemandTypeIndex= -1;
		for(int i = 0; i<m_pDoc->m_DemandMetaDataKeyVector.size();i++)
			{

				DTADemandMetaDataType demand_meta_data_element = m_pDoc->DTADemandMetaDataTypeMap[m_pDoc->m_DemandMetaDataKeyVector[i]];
			// can be also enhanced to edit the real time information percentage
		char text[300];
		sprintf_s(text, "%d",demand_meta_data_element.demand_table_type_no );
		int Index = m_DemandTypeGrid.InsertItem(LVIF_TEXT,i,text , 0, 0, 0, NULL);

		if(FirstDemandTypeIndex<0)
			FirstDemandTypeIndex = Index+1;  // initiailize the first demand

		sprintf_s(text, "%d",demand_meta_data_element.trip_type);
		m_DemandTypeGrid.SetItemText(Index,1,text);

		if((demand_meta_data_element.trip_type -1) <  m_pDoc->m_DemandTypeVector.size())
		{
		sprintf_s(text, "%s",m_pDoc->m_DemandTypeVector[demand_meta_data_element.trip_type-1].demand_type_name);
		m_DemandTypeGrid.SetItemText(Index,2,text);
		}

		sprintf_s(text, "%d",demand_meta_data_element.start_time_in_min);
		m_DemandTypeGrid.SetItemText(Index,3,text);

		sprintf_s(text, "%d",demand_meta_data_element.end_time_in_min );
		m_DemandTypeGrid.SetItemText(Index,4,text);

		sprintf_s(text, "%.4f",demand_meta_data_element.subtotal_demand_volume  );
		m_DemandTypeGrid.SetItemText(Index,5,text);
		
		sprintf_s(text, "%s",demand_meta_data_element.key );
		m_DemandTypeGrid.SetItemText(Index,6,text);
	}

	if(FirstDemandTypeIndex >= 0)
		m_DemandTypeGrid.SetItemState(FirstDemandTypeIndex,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

}



void CDlgODDemandGridCtrl::DisplayDemandMatrix()
{
	if(m_pDoc==NULL)
	return;

	bool bReadDemandSuccess = true;
	int demand_type =  m_SelectedDemandMetaType;

			m_ODMatrixGrid.SetRowCount(m_pDoc->m_ZoneMap.size() + 1);
			m_ODMatrixGrid.SetColumnCount(m_pDoc->m_ZoneMap.size() + 1);

			m_ODMatrixGrid.SetFixedColumnCount(1);
			m_ODMatrixGrid.SetFixedRowCount(1);

			GV_ITEM item;
			item.mask = GVIF_TEXT;

			CString str;


			if(demand_type>=1)
			{
			for (int i=0;i<m_ODMatrixGrid.GetRowCount();i++) //Row
			{
				for (int j=0;j<m_ODMatrixGrid.GetColumnCount();j++) //Column
				{
					if (i==0 && j==0) 
					{
						continue;
					}
					else
					{
						if (i==0 && j != 0)
						{
							item.row = i;
							item.col = j;

							str.Format(_T("%d"),j);
							item.strText = str;

							m_ODMatrixGrid.SetItem(&item);					
						}
						else
						{
							if (i != 0 && j == 0)
							{
								item.row = i;
								item.col = j;

								str.Format(_T("%d"),i);
								item.strText = str;

								m_ODMatrixGrid.SetItem(&item);		
							}
							else
							{
								item.row = i;
								item.col = j;

								if (bReadDemandSuccess)
								{
									str.Format(_T("%.4f"),m_pDoc->GetDemandVolume(i,j,demand_type));
								}
								else
								{
									str.Format(_T("%.2f"),0.0f);
								}

								item.strText = str;
								m_ODMatrixGrid.SetItem(&item);
							} // neither i nor j is zero
						} // i is not zero
					} // Either i or j is not zero
				} // End for j
			} //End for i
			}
	Invalidate();
}

BOOL CDlgODDemandGridCtrl::OnInitDialog()
{
	CDialog::OnInitDialog();
	CWaitCursor wait;
	bool bReadDemandSuccess = true;

	if (m_pDoc != NULL)
		{

	CString SettingsFile;
	SettingsFile.Format ("%sDTASettings.txt",m_pDoc->m_ProjectDirectory);

		m_DemandMultipler = g_GetPrivateProfileFloat("demand", "global_multiplier",1.0,SettingsFile);

		UpdateData(0);


		DisplayDemandTypeTable();
		DisplayDemandMatrix();
		
		} // End if ()
		else
		{
			m_ODMatrixGrid.SetRowCount(1);
			m_ODMatrixGrid.SetColumnCount(1);

			m_ODMatrixGrid.SetFixedColumnCount(1);
			m_ODMatrixGrid.SetFixedRowCount(1);
		}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}



// CDlgODDemandGridCtrl message handlers

void CDlgODDemandGridCtrl::OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
	CString content = m_ODMatrixGrid.GetItemText(pItem->iRow,pItem->iColumn);

	stringstream strstr;
	strstr << content;
	float flow;

	strstr >> flow;

	if (strstr.fail())
	{
		MessageBox("Illegal input for OD demand!");
		*pResult = -1;
		return;
	}

	if (flow < 0)
	{
		MessageBox("OD demand can not be negative!");
		*pResult = -1;
		return;
	}

	*pResult = 0;
}


void CDlgODDemandGridCtrl::OnBnClickedGridSavequit()
{

	OnOK();
}

inline bool ConvertToInt(std::istringstream& sstream, int& n)
{
	if ((sstream >> n).fail() || !sstream.eof()) //Convert fail or non-integer part exists
	{
		return false;
	}
	else
	{
		return true;
	}
}

inline bool ConvertToDouble(std::istringstream& sstream, double& value)
{

	if ((sstream >> value).fail() || !sstream.eof())
	{
		return false;
	}
	else
	{
		return true;
	}
}

inline void ResetStringStream(std::istringstream& sstream, std::string s)
{
	sstream.str(s);
	sstream.clear();
}


bool CDlgODDemandGridCtrl::SaveZoneCSVFileExt(LPCTSTR lpszFileName)
{

	return true;
}


bool CDlgODDemandGridCtrl::SaveDemandMatrix()
{

	if(m_pDoc==NULL)
	return false;

	int demand_type = 	m_SelectedDemandMetaType;

	if(demand_type < 1)
		return false;

	// step 1:
	 // clear all demand element

	for (int i=1;i<m_ODMatrixGrid.GetRowCount();i++)
	{
		CString originStr = m_ODMatrixGrid.GetItemText(i,0);
		for (int j=1;j<m_ODMatrixGrid.GetColumnCount();j++)
		{
			CString destStr = m_ODMatrixGrid.GetItemText(0,j);
			CString flowStr = m_ODMatrixGrid.GetItemText(i,j);

			int origin_zone_id = atoi(originStr);
			int destination_zone_id = atoi(destStr);
			float number_of_vehicles = atoi(flowStr);
			m_pDoc->m_ZoneMap[origin_zone_id].m_ODDemandMatrix [destination_zone_id].SetValue (demand_type, number_of_vehicles);
		}
	}


	return true;
}

void CDlgODDemandGridCtrl::OnBnClickedGridQuit()
{
	OnCancel();
}
void CDlgODDemandGridCtrl::OnBnClickedButtonCreatezones()
{
	CDlg_ImportODDemand dlg;
	if(dlg.DoModal() == IDOK)
	{

		unsigned int number_of_zones = dlg.m_NumberOfZones;
		if (number_of_zones > m_pDoc->m_NodeSet.size())
		{
			CString errorMsg = "The number of zones are greater than the number of nodes!\nPlease re-inpute it.";
			MessageBox(errorMsg);
			return;
		}

		m_ODMatrixGrid.SetRowCount(m_pDoc->m_ODSize + 1);
		m_ODMatrixGrid.SetColumnCount(m_pDoc->m_ODSize + 1);

		m_ODMatrixGrid.SetFixedColumnCount(1);
		m_ODMatrixGrid.SetFixedRowCount(1);

		GV_ITEM item;
		item.mask = GVIF_TEXT;

		CString str;

		for (int i=0;i<m_ODMatrixGrid.GetRowCount();i++) //Row
		{
			for (int j=0;j<m_ODMatrixGrid.GetColumnCount();j++) //Column
			{
				if (i==0 && j==0) 
				{
					continue;
				}
				else
				{
					if (i==0 && j != 0)
					{
						item.row = i;
						item.col = j;

						str.Format(_T("%d"),j);
						item.strText = str;

						m_ODMatrixGrid.SetItem(&item);					
					}
					else
					{
						if (i != 0 && j == 0)
						{
							item.row = i;
							item.col = j;

							str.Format(_T("%d"),i);
							item.strText = str;

							m_ODMatrixGrid.SetItem(&item);		
						}
						else
						{
							item.row = i;
							item.col = j;

							str.Format(_T("%.2f"),0.0f);
							item.strText = str;

							m_ODMatrixGrid.SetItem(&item);
						} // neither i nor j is zero
					} // i is not zero
				} // Either i or j is not zero
			} // End for j
		} //End for i

		SaveDemandMatrix();

		// write input_activity_location.csv
		// zone number , node number
		// make sure node number exists, if the node number does not exist, skip zone-node mapping pair
		// fill zero value for OD table
		// allow user to edit
	}

}

void CDlgODDemandGridCtrl::OnBnClickedButtonEditZoneNodeMapping()
{
	CDlgZoneToNodeMapping dlg;
	dlg.m_pDlg = this;
	dlg.DoModal();

}

void CDlgODDemandGridCtrl::OnBnClickedButton1()
{
	CDlg_TDDemandProfile dlg;
	dlg.m_pDoc = m_pDoc;
	if(dlg.DoModal() ==IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}

}

void CDlgODDemandGridCtrl::OnBnClickedGridSavequit2()
{
	CWaitCursor wait;
	bool ret = SaveDemandMatrix();

	if (!ret)
	{
		AfxMessageBox("Save Demand file failed!");
	}
}



void CDlgODDemandGridCtrl::OnLvnItemchangedDemandtypelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	m_SelectedDemandMetaType = -1;
	POSITION pos = m_DemandTypeGrid.GetFirstSelectedItemPosition();
	while(pos!=NULL)
	{
		int nSelectedRow = m_DemandTypeGrid.GetNextSelectedItem(pos);
		char str[100];
		m_DemandTypeGrid.GetItemText (nSelectedRow,1,str,20);
		m_SelectedDemandMetaType = atoi(str);
//		TRACE("Select %d\n",m_SelectedDemandMetaType);
		DisplayDemandMatrix();
	}

}
