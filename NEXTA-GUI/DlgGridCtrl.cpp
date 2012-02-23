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
}

BOOL CDlgGridCtrl::OnInitDialog()
{
	CDialog::OnInitDialog();
	CWaitCursor wait;
	bool bReadDemandSuccess = false;

	if (ReadZoneCSVFileExt(m_pDoc->m_ProjectDirectory + "input_zone.csv"))
	{
		if (ReadDemandCSVFileExt(m_pDoc->m_ProjectDirectory + "input_demand.csv"))
		{
			bReadDemandSuccess = true;
		}


		if (m_pDoc != NULL)
		{
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
									str.Format(_T("%.2f"),m_pDoc->m_DemandMatrix[i-1][j-1]);
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
		} // End if ()
		else
		{
			m_Grid.SetRowCount(1);
			m_Grid.SetColumnCount(1);

			m_Grid.SetFixedColumnCount(1);
			m_Grid.SetFixedRowCount(1);
		}
	} // End if Read Zone File

	return TRUE;  // return TRUE  unless you set the focus to a control
}



BEGIN_MESSAGE_MAP(CDlgGridCtrl, CDialog)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_CTRL, &CDlgGridCtrl::OnGridEndEdit)
	ON_BN_CLICKED(ID_GRID_SAVEQUIT, &CDlgGridCtrl::OnBnClickedGridSavequit)
	ON_BN_CLICKED(ID_GRID_QUIT, &CDlgGridCtrl::OnBnClickedGridQuit)
	ON_BN_CLICKED(IDC_BUTTON_CreateZones, &CDlgGridCtrl::OnBnClickedButtonCreatezones)
	ON_BN_CLICKED(IDC_BUTTON_Edit_Zone_Node_Mapping, &CDlgGridCtrl::OnBnClickedButtonEditZoneNodeMapping)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgGridCtrl::OnBnClickedButton1)
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
	CWaitCursor wait;
	bool ret = SaveDemandCSVFileExt(m_pDoc->m_ProjectDirectory+"input_demand.csv");

	if (!ret)
	{
		AfxMessageBox("Save Demand file failed!");
	}

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

bool CDlgGridCtrl::ReadZoneCSVFileExt(LPCTSTR lpszFileName)
{

	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
	m_pDoc->m_NodeIDtoZoneNameMap.clear();
	if(m_pDoc->m_DemandMatrix != NULL)
	{
		DeallocateDynamicArray<float>(m_pDoc->m_DemandMatrix,m_pDoc->m_ODSize,m_pDoc->m_ODSize);
		m_pDoc->m_DemandMatrix = NULL;
	}

	m_pDoc->m_ODSize = 0;

	while(parser.ReadRecord())
		{
			int zone_number;
	
			if(parser.GetValueByFieldName("zone_id",zone_number) == false)
				break;

			int node_name;
			if(parser.GetValueByFieldName("node_id",node_name) == false)
				break;

		ZoneRecordSet.push_back(ZoneRecord(zone_number,node_name));

		m_pDoc->m_NodeIDtoZoneNameMap[m_pDoc->m_NodeNametoIDMap[node_name]] = zone_number;

		int zoneid  = zone_number-1;
		m_pDoc->m_ZoneIDtoNodeIDMap[zoneid] = m_pDoc->m_NodeNametoIDMap[node_name];

		if(m_pDoc->m_ODSize < zone_number)
		{
			m_pDoc->m_ODSize = zone_number;
		}

	} // End while

	m_pDoc->m_DemandMatrix = AllocateDynamicArray<float>(m_pDoc->m_ODSize,m_pDoc->m_ODSize);

	return true;
	}
	return false;
}

bool CDlgGridCtrl::SaveZoneCSVFileExt(LPCTSTR lpszFileName)
{
	/*
	ofstream outFile(m_pDoc->m_ProjectDirectory + "input_zone.csv");

	if (!outFile.is_open())
	{
		outFile << "zone_id,node_id\n";

		list<DTANode*>::iterator it = m_pDoc->m_NodeSet.begin();

		//for ( unsigned int i=0;i<number_of_zones;i++,it++)
		//{
		//	outFile << (*it)->m_NodeNumber << "," << (*it)->m_NodeNumber << "\n";
		//}

		outFile.close();
	}
*/
	return true;
}

bool CDlgGridCtrl::ReadDemandCSVFileExt(LPCTSTR lpszFileName)
{
	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{

	for(int i= 0; i<m_pDoc->m_ODSize; i++)
	{
		for(int j= 0; j<m_pDoc->m_ODSize; j++)
		{
			m_pDoc->m_DemandMatrix[i][j]= 0.0f;
		}
	}


		while(parser.ReadRecord())
		{
			int origin_zone_id, destination_zone_id;
			float number_of_vehicles;

			if(parser.GetValueByFieldName("from_zone_id",origin_zone_id) == false)
				break;
			if(parser.GetValueByFieldName("to_zone_id",destination_zone_id) == false)
				break;
			if(parser.GetValueByFieldName("number_of_vehicle_trips_type1",number_of_vehicles) == false)
				break;

		if(origin_zone_id <=m_pDoc->m_ODSize && destination_zone_id<=m_pDoc->m_ODSize)
		{
			m_pDoc->m_DemandMatrix[origin_zone_id-1][destination_zone_id-1] += number_of_vehicles;
		}

	} // End while


	return true;
	}


	return false;
}

bool CDlgGridCtrl::SaveDemandCSVFileExt(LPCTSTR lpszFileName)
{
	ofstream outFile(lpszFileName);

	if (!outFile.is_open())
	{
		return false;
	}

	outFile << "from_zone_id,to_zone_id,number_of_vehicle_trips_type1,number_of_vehicle_trips_type2,number_of_vehicle_trips_type3,number_of_vehicle_trips_type4,starting_time_in_min,ending_time_in_min\n";

	float maxFlow = -1;
	for (int i=1;i<m_Grid.GetRowCount();i++)
	{
		CString originStr = m_Grid.GetItemText(i,0);
		for (int j=1;j<m_Grid.GetColumnCount();j++)
		{
			CString destStr = m_Grid.GetItemText(0,j);
			CString flowStr = m_Grid.GetItemText(i,j);

			float flow = atoi(flowStr);

			if (flow > maxFlow)
			{
				maxFlow = flow;
			}

			if (flow > 0)
			{
				outFile << originStr << "," << destStr << "," << flowStr << ",0,0,0,0,60\n";
			}

			m_pDoc->m_DemandMatrix[i-1][j-1] = flow;
		}
	}

	outFile.close();

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
		if(m_pDoc->m_DemandMatrix != NULL)
		{
			DeallocateDynamicArray(m_pDoc->m_DemandMatrix,m_pDoc->m_ODSize,m_pDoc->m_ODSize);
			m_pDoc->m_DemandMatrix = NULL;
		}

		unsigned int number_of_zones = dlg.m_NumberOfZones;
		if (number_of_zones > m_pDoc->m_NodeSet.size())
		{
			CString errorMsg = "The number of zones are greater than the number of nodes!\nPlease re-inpute it.";
			MessageBox(errorMsg);
			return;
		}

		ofstream outFile(m_pDoc->m_ProjectDirectory + "input_zone.csv");

		if (outFile.is_open())
		{
			ZoneRecordSet.clear();
			outFile << "zone_id,node_id\n";

			list<DTANode*>::iterator it = m_pDoc->m_NodeSet.begin();

			for ( unsigned int i=0;i<number_of_zones;i++,it++)
			{
				outFile << (*it)->m_NodeNumber << "," << (*it)->m_NodeNumber << "\n";
			}

			outFile.close();
		}

		ReadZoneCSVFileExt(m_pDoc->m_ProjectDirectory + "input_zone.csv");

		//m_pDoc->m_DemandMatrix = AllocateDynamicArray<float>(m_pDoc->m_ODSize,m_pDoc->m_ODSize);


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

		SaveDemandCSVFileExt(m_pDoc->m_ProjectDirectory + "input_demand.csv");

		// write input_zone.csv
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
