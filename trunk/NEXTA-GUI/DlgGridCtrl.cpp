// DlgGridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Geometry.h"
#include "CSVParser.h"
#include "TLite.h"
#include "DlgGridCtrl.h"

#include "DlgZoneToNodeMapping.h"
#include "Dlg_ImportODDemand.h"


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

// CDlgGridCtrl dialog

IMPLEMENT_DYNAMIC(CDlgGridCtrl, CDialog)

CDlgGridCtrl::CDlgGridCtrl(CWnd* pParent /*=NULL*/)
: CDialog(CDlgGridCtrl::IDD, pParent)
{
	m_bSizeChanged = false;
}

CDlgGridCtrl::~CDlgGridCtrl()
{
}

void CDlgGridCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_GRID_CTRL,m_Grid);
	DDX_Control(pDX, IDC_LIST1, m_DemandTypeList);
}

void CDlgGridCtrl::DisplayDemandMatrix()
{
	if(m_pDoc==NULL)
	return;

	bool bReadDemandSuccess = true;
	int demand_type = m_DemandTypeList.GetCurSel()+1;
			m_Grid.SetRowCount(m_pDoc->m_ODSize + 1);
			m_Grid.SetColumnCount(m_pDoc->m_ODSize + 1);

			m_Grid.SetFixedColumnCount(1);
			m_Grid.SetFixedRowCount(1);

			GV_ITEM item;
			item.mask = GVIF_TEXT;

			CString str;

			for (int i=0;i<m_Grid.GetRowCount();i++) //Row
			{
				for (int j=0;j<m_Grid.GetColumnCount();j++) //Column
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

							m_Grid.SetItem(&item);					
						}
						else
						{
							if (i != 0 && j == 0)
							{
								item.row = i;
								item.col = j;

								str.Format(_T("%d"),i);
								item.strText = str;

								m_Grid.SetItem(&item);		
							}
							else
							{
								item.row = i;
								item.col = j;

								if (bReadDemandSuccess)
								{
									str.Format(_T("%.2f"),m_pDoc->GetDemandVolume(i,j,demand_type));
								}
								else
								{
									str.Format(_T("%.2f"),0.0f);
								}

								item.strText = str;
								m_Grid.SetItem(&item);
							} // neither i nor j is zero
						} // i is not zero
					} // Either i or j is not zero
				} // End for j
			} //End for i

	Invalidate();
}

BOOL CDlgGridCtrl::OnInitDialog()
{
	CDialog::OnInitDialog();
	CWaitCursor wait;
	bool bReadDemandSuccess = true;

	if (m_pDoc != NULL)
		{

		for(std::vector<DTADemandType>::iterator itr = m_pDoc->m_DemandTypeVector.begin(); itr != m_pDoc->m_DemandTypeVector.end(); itr++)
		{
			// can be also enhanced to edit the real time information percentage
			CString str;
			str.Format(" No.%d: %s, VOT= %5.3f", (*itr).demand_type , (*itr).demand_type_name, (*itr).average_VOT);
			m_DemandTypeList.AddString(str);
		}

		m_DemandTypeList.SetCurSel (0);

		DisplayDemandMatrix();
		
			} // End if ()
		else
		{
			m_Grid.SetRowCount(1);
			m_Grid.SetColumnCount(1);

			m_Grid.SetFixedColumnCount(1);
			m_Grid.SetFixedRowCount(1);
		}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}



BEGIN_MESSAGE_MAP(CDlgGridCtrl, CDialog)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_CTRL, &CDlgGridCtrl::OnGridEndEdit)
	ON_BN_CLICKED(ID_GRID_SAVEQUIT, &CDlgGridCtrl::OnBnClickedGridSavequit)
	ON_BN_CLICKED(ID_GRID_QUIT, &CDlgGridCtrl::OnBnClickedGridQuit)
	ON_BN_CLICKED(IDC_BUTTON_CreateZones, &CDlgGridCtrl::OnBnClickedButtonCreatezones)
	ON_BN_CLICKED(IDC_BUTTON_Edit_Zone_Node_Mapping, &CDlgGridCtrl::OnBnClickedButtonEditZoneNodeMapping)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgGridCtrl::OnBnClickedButton1)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDlgGridCtrl::OnLbnSelchangeList1)
	ON_BN_CLICKED(ID_GRID_SAVEQUIT2, &CDlgGridCtrl::OnBnClickedGridSavequit2)
END_MESSAGE_MAP()


// CDlgGridCtrl message handlers

void CDlgGridCtrl::OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
	CString content = m_Grid.GetItemText(pItem->iRow,pItem->iColumn);

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


void CDlgGridCtrl::OnBnClickedGridSavequit()
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


bool CDlgGridCtrl::SaveZoneCSVFileExt(LPCTSTR lpszFileName)
{

	return true;
}


bool CDlgGridCtrl::SaveDemandMatrix()
{

	if(m_pDoc==NULL)
	return false;

	int demand_type = m_DemandTypeList.GetCurSel() +1;

	// step 1:
	 // clear all demand element

	for (int i=1;i<m_Grid.GetRowCount();i++)
	{
		CString originStr = m_Grid.GetItemText(i,0);
		for (int j=1;j<m_Grid.GetColumnCount();j++)
		{
			CString destStr = m_Grid.GetItemText(0,j);
			CString flowStr = m_Grid.GetItemText(i,j);

			int origin_zone_id = atoi(originStr);
			int destination_zone_id = atoi(destStr);
			float number_of_vehicles = atoi(flowStr);
			m_pDoc->m_ZoneMap[origin_zone_id].m_ODDemandMatrix [destination_zone_id].SetValue (demand_type, number_of_vehicles);
		}
	}


	return true;
}

void CDlgGridCtrl::OnBnClickedGridQuit()
{
	OnCancel();
}
void CDlgGridCtrl::OnBnClickedButtonCreatezones()
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

			m_Grid.SetRowCount(m_pDoc->m_ODSize + 1);
		m_Grid.SetColumnCount(m_pDoc->m_ODSize + 1);

		m_Grid.SetFixedColumnCount(1);
		m_Grid.SetFixedRowCount(1);

		GV_ITEM item;
		item.mask = GVIF_TEXT;

		CString str;

		for (int i=0;i<m_Grid.GetRowCount();i++) //Row
		{
			for (int j=0;j<m_Grid.GetColumnCount();j++) //Column
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

						m_Grid.SetItem(&item);					
					}
					else
					{
						if (i != 0 && j == 0)
						{
							item.row = i;
							item.col = j;

							str.Format(_T("%d"),i);
							item.strText = str;

							m_Grid.SetItem(&item);		
						}
						else
						{
							item.row = i;
							item.col = j;

							str.Format(_T("%.2f"),0.0f);
							item.strText = str;

							m_Grid.SetItem(&item);
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

void CDlgGridCtrl::OnBnClickedButtonEditZoneNodeMapping()
{
	CDlgZoneToNodeMapping dlg;
	dlg.m_pDlg = this;
	dlg.DoModal();

}

void CDlgGridCtrl::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}

void CDlgGridCtrl::OnLbnSelchangeList1()
{
	DisplayDemandMatrix();
}

void CDlgGridCtrl::OnBnClickedGridSavequit2()
{
	CWaitCursor wait;
	bool ret = SaveDemandMatrix();

	if (!ret)
	{
		AfxMessageBox("Save Demand file failed!");
	}
}
