// DlgScenarioTab.cpp : implementation file

//  Portions Copyright 2011 Hao Lei(haolei.sc@gmail.com), Xuesong Zhou (xzhou99@gmail.com)

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
#include "NetworkDataTabDlg.h"

#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"

#include <string>
#include <sstream>
extern std::list<int>	g_LinkDisplayList;
// CNetworkDataTabDlg dialog
IMPLEMENT_DYNAMIC(CNetworkDataTabDlg, CDialog)

CNetworkDataTabDlg::CNetworkDataTabDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetworkDataTabDlg::IDD, pParent)
{

}

CNetworkDataTabDlg::CNetworkDataTabDlg(std::vector<std::string> Names, std::vector<CString> default_value,
								 std::vector<std::vector<std::string>> Values)
{
	names = Names;
	values = Values;
	this->default_value = default_value;
}

CNetworkDataTabDlg::~CNetworkDataTabDlg()
{
}

void CNetworkDataTabDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_GRIDLISTCTRLEX,m_ListCtrl);
}

BOOL CNetworkDataTabDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Give better margin to editors
	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;  // Hao: this ponter should be delete. 
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);

	//Add Columns and set headers
	for (size_t i=0;i<names.size();i++)
	{
		CGridColumnTrait* pTrait = NULL;

		pTrait = new CGridColumnTraitEdit();

		m_ListCtrl.InsertColumnTrait((int)i,names.at(i).c_str(),LVCFMT_LEFT,-1,-1, pTrait);
		m_ListCtrl.SetColumnWidth((int)i,LVSCW_AUTOSIZE_USEHEADER);
	}


	if(names.size()>=1)
	{
	m_ListCtrl.SetColumnWidth(0, 80);
	}

	m_NumOfCols = names.size();


	//Add Rows
	for (size_t i=0;i<values.size();i++)
	{
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,values.at(i).at(0).c_str() , 0, 0, 0, NULL);
		for (size_t j=1; j < values.at(i).size();j++)
		{
				int nCol = j;
				m_ListCtrl.SetItemText(Index, nCol,values.at(i).at(j).c_str());
		}
	}

	m_NumOfRows = values.size();


	if(strcmp(m_TabText,"Sensor") ==0 && m_SelectedFromNodeName >=1 && m_SelectedToNodeName >=1)
	{
		AddRowForLink (m_SelectedFromNodeName, m_SelectedToNodeName);
	}

	return TRUE;
}

void CNetworkDataTabDlg::SetTabText(CString s)
{
	m_TabText =  s;
}

CString CNetworkDataTabDlg::GetTabText()
{
	return m_TabText;
}

BOOL CNetworkDataTabDlg::CopyRow()
{
	if(m_NumOfRows==0)
		return FALSE;

	int OriginalNumOfRows = m_NumOfRows;
	int Index = m_NumOfRows;

	int k;
	int number_of_selected_rows = 0;

	for(k = 0; k < OriginalNumOfRows; k++)
	{
		if(m_ListCtrl.IsRowSelected(k))  // if row is selected
		{
			number_of_selected_rows++;
		}
	}

	if(number_of_selected_rows ==0)
	{
		m_ListCtrl.SelectRow (m_NumOfRows-1,true);  // auto select last row
	}

	for(k = 0; k < OriginalNumOfRows; k++)
	{
		if(m_ListCtrl.IsRowSelected(k))  // if row is selected
		{
			//static CString
			Index = m_ListCtrl.InsertItem(LVIF_TEXT,m_NumOfRows,NULL,0,0,0,NULL);
			for (int i=0;i< m_NumOfCols;i++)
			{
				CString text = m_ListCtrl.GetItemText(k,i);
				m_ListCtrl.SetItemText(Index,i,text);
			}
			

			m_NumOfRows++;
		}
	}

	m_ListCtrl.SetItemState(Index,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	m_ListCtrl.SetFocus();
	return TRUE;
}

BOOL CNetworkDataTabDlg::AddRow()
{
	//static CString
	int Index = m_ListCtrl.InsertItem(LVIF_TEXT,m_NumOfRows,NULL,0,0,0,NULL);
	for (int i=1;i< min(default_value.size()+1,m_NumOfCols);i++)
	{
		m_ListCtrl.SetItemText(Index,i,default_value.at(i-1));
	}
	
	m_NumOfRows++;

	m_ListCtrl.SetItemState(Index,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	m_ListCtrl.SetFocus();
	return TRUE;
}


BOOL CNetworkDataTabDlg::AddRowForLink(int from_node_name,int to_node_name)
{
	//static CString
	int Index = m_ListCtrl.InsertItem(LVIF_TEXT,m_NumOfRows,NULL,0,0,0,NULL);
	for (int i=1;i< m_NumOfCols;i++)
	{
		if(strcmp(names[i].c_str (),"from_node_id")==0)
		{   
			CString str;
			str.Format ("%d", from_node_name);
			m_ListCtrl.SetItemText(Index,i,str);
		} else if (strcmp(names[i].c_str (),"to_node_id")==0)
		{
			CString str;
			str.Format ("%d", to_node_name);
			m_ListCtrl.SetItemText(Index,i,str);
		} else if (strcmp(names[i].c_str (),"sensor_id")==0)
		{
		  // empty
		
		} else if (strcmp(names[i].c_str (),"link_count")==0)
		{
		   // empty
		
		}else
		{
			
				CString text = m_ListCtrl.GetItemText(Index-1,i);  // copy from previous record
				m_ListCtrl.SetItemText(Index,i,text);
		}
	}
	
	m_NumOfRows++;

	m_ListCtrl.SetItemState(Index,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	m_ListCtrl.SetFocus();
	return TRUE;
}

int CNetworkDataTabDlg::ValidityCheck()
{
	using std::string;
	using std::istringstream;

	int ret = 0;

	int col = m_ListCtrl.GetColumnCount();
	int row = m_ListCtrl.GetItemCount();

	int start_time, end_time;

	CString ErrorMsg;

	for (int i=0;i<row;i++)
	{

		for (int j=0;j<col;j++)
		{
			CString text = m_ListCtrl.GetItemText(i,j);

			//if (text.GetLength() == 0)
			//{
			//	m_ListCtrl.SetItemState(i,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			//	ErrorMsg.Format("[%s] cannot be empty",names.at(j).c_str());
			//	MessageBox(ErrorMsg,NULL,MB_ICONERROR);
			//	ret = -1;
			//	return ret;
			//}
			
			istringstream iss(text.GetBuffer());
			text.ReleaseBuffer();

			if (j != 0)
			{
				int value;
				iss >> value;

				//if (!iss.eof())
				//{
				//	ErrorMsg.Format("Integer is required at row %d col %d",i+1, j+1);
				//	MessageBox(ErrorMsg,NULL,MB_ICONERROR);
				//	return -5;
				//}

				if (value < 0)
				{
					m_ListCtrl.SetItemState(i,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
					ErrorMsg.Format("Negative value is not valid at [%s]",names.at(j).c_str());
					MessageBox(ErrorMsg,NULL,MB_ICONERROR);
					ret = -2;
					return ret;
				}
				
			}


		}

	}

	return ret;
}

CString CNetworkDataTabDlg::GenerateRecordString()
{
	std::string Str;

	for (int i=0;i<names.size();i++)
	{
		Str += names.at(i);
		if (i != names.size() - 1)
		{
			Str += ",";
		}
	}

	Str += "\n";

	for (int i=0;i<m_ListCtrl.GetItemCount();i++)
	{
		std::string subStr;
		for (int j=0;j<m_NumOfCols;j++)
		{
			std::string tmp = std::string(m_ListCtrl.GetItemText(i,j));

			subStr += tmp;

			if (j != m_NumOfCols - 1)
			{
				subStr += ",";
			}
			
		}
		subStr += "\n";

		Str += subStr;
	}

	return Str.c_str();
}

BOOL CNetworkDataTabDlg::DeleteRow()
{
	int nItem = m_ListCtrl.GetSelectionMark();
	if (nItem == -1)
	{
		MessageBox("No record is selected!");
		return FALSE;
	}
	
	m_ListCtrl.DeleteItem(nItem);

	m_NumOfRows--;

	return TRUE;
}

void CNetworkDataTabDlg::ZoomToSelectedObject()
{

	CString TabText = GetTabText ();

	if(TabText == "Link")
	{
	g_LinkDisplayList.clear ();

	int nSelectedRow = m_ListCtrl.GetSelectionMark();
	if (nSelectedRow == -1)
	{
		MessageBox("No record is selected!");
		return;
	}

		CString str;
		int from_node_id = -1;
		int to_node_id = -1;

		for (int i=0;i< m_ListCtrl.GetColumnCount() ;i++)
		{
			if(strcmp(names[i].c_str (),"from_node_id")==0)
			{   
				str = m_ListCtrl.GetItemText(nSelectedRow,i);
				from_node_id = atoi(str);

			} else if (strcmp(names[i].c_str (),"to_node_id")==0)
			{
				str = m_ListCtrl.GetItemText(nSelectedRow,i);
				to_node_id = atoi(str);

			}
		}

		DTALink* pLink = m_pDoc->FindLinkWithNodeNumbers(from_node_id,to_node_id);
		if(pLink!=NULL)
		{
			m_pDoc->m_SelectedLinkNo = pLink->m_LinkNo;
			g_LinkDisplayList.push_back(pLink->m_LinkNo);
		}

	
	m_pDoc->ZoomToSelectedLink(m_pDoc->m_SelectedLinkNo);

	}
		
	m_pDoc->UpdateAllViews(0);


}

void CNetworkDataTabDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	if(m_ZoomToSelectedObject ==false)
		return;

	UpdateData(1);
	CString TabText = GetTabText ();

	if(TabText == "Link" || TabText == "Calibration Data")
	{
	g_LinkDisplayList.clear ();

	}

	
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	while(pos!=NULL)
	{
		int nSelectedRow = m_ListCtrl.GetNextSelectedItem(pos);
	CString str;

			if(TabText.Find("Link")>=0 || TabText == "Calibration Data")
			{
		int from_node_id = -1;
		int to_node_id = -1;

		for (int i=0;i< m_ListCtrl.GetColumnCount() ;i++)
		{
			if(strcmp(names[i].c_str (),"from_node_id")==0)
			{   
				str = m_ListCtrl.GetItemText(nSelectedRow,i);
				from_node_id = atoi(str);

			} else if (strcmp(names[i].c_str (),"to_node_id")==0)
			{
				str = m_ListCtrl.GetItemText(nSelectedRow,i);
				to_node_id = atoi(str);

			}
		}
		DTALink* pLink = m_pDoc->FindLinkWithNodeNumbers(from_node_id,to_node_id);
		if(pLink!=NULL)
		{
			m_pDoc->m_SelectedLinkNo = pLink->m_LinkNo;
			g_LinkDisplayList.push_back(pLink->m_LinkNo);
		}
			m_pDoc->ZoomToSelectedLink(m_pDoc->m_SelectedLinkNo);


		}

	}

	Invalidate();
		
	m_pDoc->UpdateAllViews(0);

}

BEGIN_MESSAGE_MAP(CNetworkDataTabDlg, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GRIDLISTCTRLEX, &CNetworkDataTabDlg::OnLvnItemchangedList)
	ON_NOTIFY(TCN_SELCHANGE, IDC_SETTING_TAB, &CNetworkDataTabDlg::OnTcnSelchangeSettingTab)
END_MESSAGE_MAP()


// CDlgScenarioTab message handlers

void CNetworkDataTabDlg::OnTcnSelchangeSettingTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
