// DlgLinkList.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgLinkList.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"

#include <string>
#include <sstream>

extern std::list<DTALink*>	g_LinkDisplayList;
// CDlgLinkList dialog

IMPLEMENT_DYNAMIC(CDlgLinkList, CDialog)

CDlgLinkList::CDlgLinkList(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlgLinkList::IDD, pParent)
	, m_ZoomToSelectedLink(FALSE)
{
 m_AVISensorFlag = false;
}

CDlgLinkList::~CDlgLinkList()
{
}

void CDlgLinkList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_ListCtrl);
	DDX_Check(pDX, IDC_CHECK_ZOOM_TO_SELECTED_LINK, m_ZoomToSelectedLink);
}


BEGIN_MESSAGE_MAP(CDlgLinkList, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CDlgLinkList::OnLvnItemchangedList)
	ON_BN_CLICKED(IDOK, &CDlgLinkList::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgLinkList::OnBnClickedCancel)
	ON_COMMAND(ID_LINKSELECTION_SHOWALLLINKS, &CDlgLinkList::OnLinkselectionShowalllinks)
	ON_COMMAND(ID_LINKSELECTION_SHOWHIGHWAYLINKSONLY, &CDlgLinkList::OnLinkselectionShowhighwaylinksonly)
	ON_COMMAND(ID_LINKSELECTION_SHOWRAMPLINKSONLY, &CDlgLinkList::OnLinkselectionShowramplinksonly)
	ON_COMMAND(ID_LINKSELECTION_SHOWARTERIALLINKSONLY, &CDlgLinkList::OnLinkselectionShowarteriallinksonly)
	ON_COMMAND(ID_LINKSELECTION_SHOWALLLINKSEXCEPTCONNECTORS, &CDlgLinkList::OnLinkselectionShowalllinksexceptconnectors)
	ON_UPDATE_COMMAND_UI(ID_LINKSELECTION_SHOWALLLINKS, &CDlgLinkList::OnUpdateLinkselectionShowalllinks)
	ON_UPDATE_COMMAND_UI(ID_LINKSELECTION_SHOWHIGHWAYLINKSONLY, &CDlgLinkList::OnUpdateLinkselectionShowhighwaylinksonly)
	ON_UPDATE_COMMAND_UI(ID_LINKSELECTION_SHOWRAMPLINKSONLY, &CDlgLinkList::OnUpdateLinkselectionShowramplinksonly)
	ON_UPDATE_COMMAND_UI(ID_LINKSELECTION_SHOWARTERIALLINKSONLY, &CDlgLinkList::OnUpdateLinkselectionShowarteriallinksonly)
	ON_UPDATE_COMMAND_UI(ID_LINKSELECTION_SHOWALLLINKSEXCEPTCONNECTORS, &CDlgLinkList::OnUpdateLinkselectionShowalllinksexceptconnectors)
END_MESSAGE_MAP()


// CDlgLinkList message handlers
#define LINKCOLUMNSIZE 11
#define LINKCOLUMNSIZE_AVI 7

BOOL CDlgLinkList::OnInitDialog()
{
	CDialog::OnInitDialog();

// Give better margin to editors
	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;  // Hao: this ponter should be delete. 
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);

	std::vector<std::string> m_Column_names;

_TCHAR *ColumnMOELabel[LINKCOLUMNSIZE] =
{
	_T("No."),_T("From Node"), _T("To Node"), _T("Length (ml)"), _T("Num of Lanes"), _T("Speed Limit"),
	_T("Lane Capacity"), _T("Link Type"), _T("VOC"),_T("Volume"), _T("Speed")
};


	//Add Columns and set headers
	for (size_t i=0;i< LINKCOLUMNSIZE;i++)
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
void CDlgLinkList::ReloadData()
{
	CWaitCursor cursor;

     m_ListCtrl.DeleteAllItems();
      
	std::list<DTALink*>::iterator iLink;
	int i = 0;
	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++, i++)
	{
	
		int type  = (*iLink) ->m_link_type ;
		if(m_LinkSelectionMode == eLinkSelection_FreewayOnly) 
		{
			if(m_pDoc->m_LinkTypeFreewayMap[type ]!=1) 
			continue; 
		}
		
		if(m_LinkSelectionMode == eLinkSelection_RampOnly) 
		{
			if(m_pDoc->m_LinkTypeRampMap[type ]!=1) 
			continue; 
		}

		if(m_LinkSelectionMode == eLinkSelection_ArterialOnly) 
		{
			if(m_pDoc->m_LinkTypeArterialMap[type ]!=1) 
			continue; 
		}

		if(m_LinkSelectionMode == eLinkSelection_NoConnectors) 
		{
			if(m_pDoc->m_LinkTypeConnectorMap[type ]==1) 
			continue; 
		}

		char text[100];
		sprintf_s(text, "%d",(*iLink)->m_LinkNo );
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,text , 0, 0, 0, NULL);

		sprintf_s(text, "%d",(*iLink)->m_FromNodeNumber);
		m_ListCtrl.SetItemText(Index,1,text);

		sprintf_s(text, "%d",(*iLink)->m_ToNodeNumber);
		m_ListCtrl.SetItemText(Index,2,text);

		sprintf_s(text, "%5.2f",(*iLink)->m_Length);
		m_ListCtrl.SetItemText(Index,3,text);

		sprintf_s(text, "%d",(*iLink)->m_NumLanes );
		m_ListCtrl.SetItemText(Index,4,text);

		sprintf_s(text, "%4.0f",(*iLink)->m_SpeedLimit  );
		m_ListCtrl.SetItemText(Index,5,text);

		sprintf_s(text, "%4.0f",(*iLink)->m_LaneCapacity  );
		m_ListCtrl.SetItemText(Index,6,text);

		sprintf_s(text, "%s", m_pDoc->m_LinkTypeVector[(*iLink)->m_link_type -1].link_type_name.c_str ());
		m_ListCtrl.SetItemText(Index,7,text);

		if((*iLink)->m_StaticVOC >0)
		{
		sprintf_s(text, "%5.2f",(*iLink)->m_StaticVOC    );
		m_ListCtrl.SetItemText(Index,8,text);

		sprintf_s(text, "%5.2f",(*iLink)->m_StaticSpeed    );
		m_ListCtrl.SetItemText(Index,9,text);

		sprintf_s(text, "%5.0f",(*iLink)->m_StaticLinkVolume     );
		m_ListCtrl.SetItemText(Index,10,text);
		}
	}

}

void CDlgLinkList::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
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

void CDlgLinkList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}

void CDlgLinkList::OnBnClickedCancel()
{
	CDialog::OnOK();
}


void CDlgLinkList::OnLinkselectionShowalllinks()
{
	m_LinkSelectionMode = eLinkSelection_AllLinks;
	ReloadData();
}


void CDlgLinkList::OnLinkselectionShowhighwaylinksonly()
{
	m_LinkSelectionMode = eLinkSelection_FreewayOnly;
	ReloadData();
}
void CDlgLinkList::OnLinkselectionShowramplinksonly()
{
	m_LinkSelectionMode = eLinkSelection_RampOnly;
	ReloadData();
}

void CDlgLinkList::OnLinkselectionShowarteriallinksonly()
{
	m_LinkSelectionMode = eLinkSelection_ArterialOnly;
	ReloadData();

}

void CDlgLinkList::OnLinkselectionShowalllinksexceptconnectors()
{
	m_LinkSelectionMode = eLinkSelection_NoConnectors;
	ReloadData();
}

void CDlgLinkList::OnUpdateLinkselectionShowalllinks(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkSelectionMode == eLinkSelection_AllLinks);
}

void CDlgLinkList::OnUpdateLinkselectionShowhighwaylinksonly(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkSelectionMode == eLinkSelection_FreewayOnly);
}

void CDlgLinkList::OnUpdateLinkselectionShowramplinksonly(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkSelectionMode == eLinkSelection_RampOnly);
}

void CDlgLinkList::OnUpdateLinkselectionShowarteriallinksonly(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkSelectionMode == eLinkSelection_ArterialOnly);
}

void CDlgLinkList::OnUpdateLinkselectionShowalllinksexceptconnectors(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkSelectionMode == eLinkSelection_NoConnectors);
}
