// DlgGridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgGridCtrl.h"

#include <map>
#include <iostream>
#include <fstream>

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

	CFileDialog fileDlg(TRUE,"csv","*.csv");
	int retCode = fileDlg.DoModal();

	if (retCode == IDOK)
	{
		CString fileNameCStr = fileDlg.GetPathName();

		m_ODTable.Init((LPCSTR)fileNameCStr);

		m_Grid.SetRowCount(m_ODTable.GetMaxOriginNum() + 1);
		m_Grid.SetColumnCount(m_ODTable.GetMaxDestNum() + 1);

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

		map<ODPair,DemandInfo>* ptrDemandInfo = m_ODTable.GetDefaultODDemand();

		if (ptrDemandInfo != NULL)
		{
			map<ODPair,DemandInfo>::iterator it;
			for (it = ptrDemandInfo->begin();it != ptrDemandInfo->end();it++)
			{
				int row = (*it).first.GetOrigin();
				int col = (*it).first.GetDestination();

				float flow = (*it).second.GetTotalFlow();

				item.row = row;
				item.col = col;
				str.Format(_T("%.2f"),flow);
				item.strText = str;

				m_Grid.SetItem(&item);
			}
		}

	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}



BEGIN_MESSAGE_MAP(CDlgGridCtrl, CDialog)
	ON_BN_CLICKED(ID_GRID_SAVEQUIT, &CDlgGridCtrl::OnBnClickedGridSavequit)
	ON_BN_CLICKED(ID_GRID_QUIT, &CDlgGridCtrl::OnBnClickedGridQuit)
END_MESSAGE_MAP()


// CDlgGridCtrl message handlers

void CDlgGridCtrl::OnBnClickedGridSavequit()
{
	ofstream outFile(m_ODTable.GetDemandFileName().c_str());
	//MessageBox(m_ODTable.GetDemandFileName().c_str());

	outFile << "from_zone_id, to_zone_id, number_of_vehicles, vehicle_type, starting_time_in_min, ending_time_in_min\n";

	for (int i=1;i<m_Grid.GetRowCount();i++)
	{
		CString origin = m_Grid.GetItemText(i,0);
		for (int j=1;j<m_Grid.GetColumnCount();j++)
		{
			CString dest = m_Grid.GetItemText(0,j);
			CString flow = m_Grid.GetItemText(i,j);
			CString line;

			if (atoi(flow) > 0)
			{
				outFile << origin << "," << dest << "," << flow << ",1,0,60\n";
			}
		}
	}

	outFile.close();

	OnOK();
}

void CDlgGridCtrl::OnBnClickedGridQuit()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}