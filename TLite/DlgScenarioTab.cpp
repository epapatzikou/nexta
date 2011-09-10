// DlgScenarioTab.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgScenarioTab.h"

#include "CGridListCtrlEx\CGridColumnTraitDateTime.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"
#include "CGridListCtrlEx\ViewConfigSection.h"
#include <string>
#include <sstream>

// CDlgScenarioTab dialog

IMPLEMENT_DYNAMIC(CDlgScenarioTab, CDialog)

CDlgScenarioTab::CDlgScenarioTab(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgScenarioTab::IDD, pParent)
{

}

CDlgScenarioTab::CDlgScenarioTab(std::vector<std::string> Names, std::vector<std::vector<std::string>> Values, std::vector<std::string> linkstring)
{
	names = Names;
	values = Values;
	LinkString = linkstring;
}

CDlgScenarioTab::~CDlgScenarioTab()
{
}

void CDlgScenarioTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_GRIDLISTCTRLEX,m_ListCtrl);
}

BOOL CDlgScenarioTab::OnInitDialog()
{
	CDialog::OnInitDialog();


	// Give better margin to editors
	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);

	//Add Columns and set headers
	for (int i=0;i<names.size();i++)
	{

		CGridColumnTrait* pTrait = NULL;
		if (i == 0)	// Link
		{
			CGridColumnTraitCombo* pComboTrait = new CGridColumnTraitCombo;

			for (size_t n=0;n<LinkString.size();n++)
			{
				pComboTrait->AddItem((int)n,LinkString.at(n).c_str());
			}

			pTrait = pComboTrait;
		}
		else
		{
			pTrait = new CGridColumnTraitEdit();
		}

		m_ListCtrl.InsertColumnTrait(i,names.at(i).c_str(),LVCFMT_LEFT,-1,-1, pTrait);
		m_ListCtrl.SetColumnWidth(i,LVSCW_AUTOSIZE_USEHEADER);

		if (names.at(i)=="Start_Time_In_Min")
		{
			m_StartTime_Idx = i;
		}

		if (names.at(i)=="End_Time_In_Min")
		{
			m_EndTime_Idx = i;
		}

		if (names.at(i) == "Capacity_Reduction_Rate")
		{
			m_Rate_Idx = i;
		}
	}

	m_ListCtrl.SetColumnWidth(0, 80);

	m_NumOfCols = names.size();


	//Add Rows
	for (int i=0;i<values.size();i++)
	{
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,values.at(i).at(0).c_str() , 0, 0, 0, NULL);
		for (int j=1;j<values.at(i).size();j++)
		{
			m_ListCtrl.SetItemText(Index, j,values.at(i).at(j).c_str());
		}
	}

	m_NumOfRows = values.size();

	return TRUE;
}

void CDlgScenarioTab::SetTabText(CString s)
{
	m_TabText = s;
}

CString CDlgScenarioTab::GetTabText()
{
	return m_TabText;
}


BOOL CDlgScenarioTab::AddRow()
{
	int Index = m_ListCtrl.InsertItem(LVIF_TEXT,m_NumOfRows,NULL,0,0,0,NULL);
	for (int i=0;i<m_NumOfCols;i++)
	{
		m_ListCtrl.SetItemText(Index,i,NULL);
	}

	m_NumOfRows++;

	m_ListCtrl.SetItemState(Index,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	m_ListCtrl.SetFocus();
	return TRUE;
}

int CDlgScenarioTab::ValidityCheck()
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
		start_time = end_time = -1;
		for (int j=0;j<col;j++)
		{
			CString text = m_ListCtrl.GetItemText(i,j);

			if (text.GetLength() == 0)
			{
				m_ListCtrl.SetItemState(i,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
				ErrorMsg.Format("Empty input at row %d col %d",i+1, j+1);
				MessageBox(ErrorMsg,NULL,MB_ICONERROR);
				ret = -1;
				return ret;
			}
			
			istringstream iss(text.GetBuffer());
			text.ReleaseBuffer();

			if (j != 0 && j != m_Rate_Idx)
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
					ErrorMsg.Format("Negative value is not valid at row %d col %d",i+1, j+1);
					MessageBox(ErrorMsg,NULL,MB_ICONERROR);
					ret = -2;
					return ret;
				}
				
				if (j == m_StartTime_Idx)
				{
					start_time = value;
				}

				if (j == m_EndTime_Idx)
				{
					end_time = value;
				}
			}

			if ( j == m_Rate_Idx)
			{
				double value;
				iss >> value;
				if (value < 0 || value >1)
				{
					m_ListCtrl.SetItemState(i,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
					ErrorMsg.Format("Capacity_Reduction_Rate at row %d col %d must be between 0 and 1!",i+1, j+1);
					MessageBox(ErrorMsg,NULL,MB_ICONERROR);
					ret = -3;
					return ret;
				}
			}

		}

		if (start_time > end_time)
		{
			m_ListCtrl.SetItemState(i,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			ErrorMsg.Format("Row %d: Start Time must be less than End Time!",i+1);
			MessageBox(ErrorMsg,NULL,MB_ICONERROR);
			//MessageBox("Start Time must be less than End Time!");
			ret = -4;
			return ret;
		}
	}

	return ret;
}

CString CDlgScenarioTab::GenerateRecordString()
{
	std::string Str;
	for (int i=0;i<m_ListCtrl.GetItemCount();i++)
	{
		std::string subStr = "<" + m_TabText + " ";
		for (int j=0;j<m_NumOfCols;j++)
		{
			subStr += names.at(j) + "=" + "\"" + std::string(m_ListCtrl.GetItemText(i,j)) + "\"";
			if (j != m_NumOfCols - 1)
			{
				subStr += " ";
			}
		}
		subStr += "></" + m_TabText + ">\n";

		Str += subStr;
	}

	return Str.c_str();
}

BOOL CDlgScenarioTab::DeleteRow()
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


BEGIN_MESSAGE_MAP(CDlgScenarioTab, CDialog)
END_MESSAGE_MAP()


// CDlgScenarioTab message handlers
