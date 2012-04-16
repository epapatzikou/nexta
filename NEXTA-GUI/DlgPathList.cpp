// DlgPathList.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgPathList.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"

#include <string>
#include <sstream>

extern std::list<DTALink*>	g_LinkDisplayList;
// CDlgPathList dialog

IMPLEMENT_DYNAMIC(CDlgPathList, CDialog)

CDlgPathList::CDlgPathList(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlgPathList::IDD, pParent)
	, m_StrPathMOE(_T(""))
{
}

CDlgPathList::~CDlgPathList()
{
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
END_MESSAGE_MAP()


// CDlgPathList message handlers
#define PATACOLUMNSIZE 6

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

_TCHAR *ColumnMOELabel[PATACOLUMNSIZE] =
{
	_T("No."),_T("Stree Name"), _T("Length (ml)"), _T("Speed Limit"), _T("Travel Time"), _T("Link Type")
};


	//Add Columns and set headers
	for (size_t i=0;i< PATACOLUMNSIZE;i++)
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

		for (int i=0 ; i<m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkSize; i++)
		{
			DTALink* pLink = m_pDoc->m_LinkNoMap[m_pDoc->m_PathDisplayList[m_pDoc->m_SelectPathNo].m_LinkVector[i]];
		char text[100];
		sprintf_s(text, "%d",pLink->m_LinkNo );
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,text , 0, 0, 0, NULL);

		m_ListCtrl.SetItemText(Index,1,pLink->m_Name.c_str () );

		sprintf_s(text, "%5.2f",pLink->m_Length);
		m_ListCtrl.SetItemText(Index,2,text);
		total_distance+= pLink->m_Length;

		sprintf_s(text, "%4.0f",pLink->m_SpeedLimit  );
		m_ListCtrl.SetItemText(Index,3,text);

		sprintf_s(text, "%4.2f",pLink->m_FreeFlowTravelTime);
		m_ListCtrl.SetItemText(Index,4,text);
		total_travel_time+=pLink->m_FreeFlowTravelTime;

		sprintf_s(text, "%s", m_pDoc->m_LinkTypeVector[pLink->m_link_type -1].link_type_name.c_str ());
		m_ListCtrl.SetItemText(Index,5,text);

		}

		m_StrPathMOE.Format("Distance=%4.2f mile, Travel Time=%4.2f min",total_distance,total_travel_time);
		UpdateData(0);

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
			g_LinkDisplayList.push_back(m_pDoc->m_LinkNoMap[LinkNo]);

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
