// DlgGridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgGridCtrl.h"

#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

using std::map;
using std::ofstream;


// CDlgGridCtrl dialog

IMPLEMENT_DYNAMIC(CDlgGridCtrl, CDialog)

CDlgGridCtrl::CDlgGridCtrl(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgGridCtrl::IDD, pParent)
{

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

							str.Format(_T("%.2f"),m_pDoc->m_DemandMatrix[i-1][j-1]);
							item.strText = str;

							m_Grid.SetItem(&item);
						} // neither i nor j is zero
					} // i is not zero
				} // Either i or j is not zero
			} // End for j
		} //End for i
	} // End if ()

	return TRUE;  // return TRUE  unless you set the focus to a control
}



BEGIN_MESSAGE_MAP(CDlgGridCtrl, CDialog)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_CTRL, &CDlgGridCtrl::OnGridEndEdit)
	ON_BN_CLICKED(ID_GRID_SAVEQUIT, &CDlgGridCtrl::OnBnClickedGridSavequit)
	ON_BN_CLICKED(ID_GRID_QUIT, &CDlgGridCtrl::OnBnClickedGridQuit)
END_MESSAGE_MAP()


// CDlgGridCtrl message handlers

void CDlgGridCtrl::OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
	CString content = m_Grid.GetItemText(pItem->iRow,pItem->iColumn);
	
	std::stringstream strstr;
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
	bool ret = SaveDemandCSVFile(m_pDoc->m_ProjectDirectory+"in_demand.csv");

	if (!ret)
	{
		AfxMessageBox("Save Demand file failed!");
	}

	OnOK();
}

bool CDlgGridCtrl::SaveDemandCSVFile(LPCTSTR lpszFileName)
{
	ofstream outFile(lpszFileName);

	outFile << "from_zone_id, to_zone_id, number_of_vehicles, vehicle_type, starting_time_in_min, ending_time_in_min\n";

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
				outFile << originStr << "," << destStr << "," << flowStr << ",1,0,60\n";
			}

			m_pDoc->m_DemandMatrix[i-1][j-1] = flow;
		}
	}

	outFile.close();

	m_pDoc->m_MaxODDemand = maxFlow;

	return true;
}

void CDlgGridCtrl::OnBnClickedGridQuit()
{
	OnCancel();
}