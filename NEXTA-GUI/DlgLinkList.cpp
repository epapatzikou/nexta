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

extern std::list<int>	g_LinkDisplayList;
extern CDlgLinkList* g_pLinkListDlg;
// CDlgLinkList dialog

IMPLEMENT_DYNAMIC(CDlgLinkList, CDialog)

CDlgLinkList::CDlgLinkList(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlgLinkList::IDD, pParent)
	, m_ZoomToSelectedLink(FALSE)
	, m_StrDocTitles(_T(""))
{
 m_AVISensorFlag = false;

 m_pDoc = NULL;
 m_pDoc2 = NULL;
 m_bDoc2Ready = false;

}

CDlgLinkList::~CDlgLinkList()
{ 
	g_pLinkListDlg = NULL;
}

void CDlgLinkList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_ListCtrl);
	DDX_Check(pDX, IDC_CHECK_ZOOM_TO_SELECTED_LINK, m_ZoomToSelectedLink);
	DDX_Text(pDX, IDC_DOC_TITLE, m_StrDocTitles);
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


	ReloadData();

	return true;
}
void CDlgLinkList::ReloadData()
{
	if(m_pDoc==NULL)
		return;

	CWaitCursor cursor;

    m_ListCtrl.DeleteAllItems();

	while(m_ListCtrl.GetColumnCount ()>0)
	{
	m_ListCtrl.DeleteColumn (0);
	}

	if(m_bDoc2Ready)
	{
		m_StrDocTitles.Format("1: %s; 2: %s", m_pDoc->m_ProjectTitle , m_pDoc2->m_ProjectTitle); 
		UpdateData(0);
	}

	std::vector<std::string> m_Column_names;

	m_Column_names.push_back ("No.");
	m_Column_names.push_back ("Link Name");
	m_Column_names.push_back ("From Node");
	m_Column_names.push_back ("To Node");
	m_Column_names.push_back ("Length (ml)");
	m_Column_names.push_back ("Num of Lanes");
	m_Column_names.push_back ("Speed Limit");
	m_Column_names.push_back ("Lane Capacity");
	m_Column_names.push_back ("Link Type");

	if(m_pDoc2==NULL)
	{
	m_Column_names.push_back ("VOC");
	}else
	{
	m_Column_names.push_back ("VOC_1");
	m_Column_names.push_back ("VOC_2");
	m_Column_names.push_back ("VOC_Diff");
	}

	if(m_pDoc2==NULL)
	{
	m_Column_names.push_back ("Speed");
	}else
	{
	m_Column_names.push_back ("Speed_1");
	m_Column_names.push_back ("Speed_2");
	m_Column_names.push_back ("Speed_Diff");
	}


	if(m_pDoc2==NULL)
	{
	m_Column_names.push_back ("Volume");
	}else
	{
	m_Column_names.push_back ("Volume_1");
	m_Column_names.push_back ("Volume_2");
	m_Column_names.push_back ("Volume_Diff");
	
	}

	// single view
	m_Column_names.push_back ("Obs Volume");
	m_Column_names.push_back ("Volume Error");
	m_Column_names.push_back ("Error %");

	//Add Columns and set headers
	for (size_t i=0;i<m_Column_names.size();i++)
	{

		CGridColumnTraitText* pTrait = NULL;
//		pTrait = new CGridColumnTraitEdit();
		m_ListCtrl.InsertColumnTrait((int)i,m_Column_names.at(i).c_str(),LVCFMT_LEFT,-1,-1, pTrait);
		m_ListCtrl.SetColumnWidth((int)i,LVSCW_AUTOSIZE_USEHEADER);
	}
	m_ListCtrl.SetColumnWidth(0, 80);
      
	std::list<DTALink*>::iterator iLink;
	int i = 0;
	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++, i++)
	{
	
		int type  = (*iLink) ->m_link_type ;
		if(m_LinkSelectionMode == eLinkSelection_FreewayOnly) 
		{
			if(m_pDoc->m_LinkTypeMap[type ].IsFreeway () == false) 
			continue; 
		}
		
		if(m_LinkSelectionMode == eLinkSelection_RampOnly) 
		{
			if(m_pDoc->m_LinkTypeMap[type ].IsRamp () == false) 
			continue; 
		}

		if(m_LinkSelectionMode == eLinkSelection_ArterialOnly) 
		{
			if(m_pDoc->m_LinkTypeMap[type ].IsArterial () == false) 
			continue; 
		}

		if(m_LinkSelectionMode == eLinkSelection_NoConnectors) 
		{
			if(m_pDoc->m_LinkTypeMap[type ].IsConnector()) 
			continue; 
		}

		char text[100];
		sprintf_s(text, "%d",(*iLink)->m_LinkNo );
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,text , 0, 0, 0, NULL);

		m_ListCtrl.SetItemText(Index,1,(*iLink)->m_Name .c_str ());

		sprintf_s(text, "%d",(*iLink)->m_FromNodeNumber);
		m_ListCtrl.SetItemText(Index,2,text);

		sprintf_s(text, "%d",(*iLink)->m_ToNodeNumber);
		m_ListCtrl.SetItemText(Index,3,text);

		sprintf_s(text, "%5.2f",(*iLink)->m_Length);
		m_ListCtrl.SetItemText(Index,4,text);

		sprintf_s(text, "%d",(*iLink)->m_NumLanes );
		m_ListCtrl.SetItemText(Index,5,text);

		sprintf_s(text, "%4.0f",(*iLink)->m_SpeedLimit  );
		m_ListCtrl.SetItemText(Index,6,text);

		sprintf_s(text, "%4.0f",(*iLink)->m_LaneCapacity  );
		m_ListCtrl.SetItemText(Index,7,text);

		if(m_pDoc->m_LinkTypeMap.find((*iLink)->m_link_type) != m_pDoc->m_LinkTypeMap.end())
		{
		sprintf_s(text, "%s", m_pDoc->m_LinkTypeMap[(*iLink)->m_link_type].link_type_name.c_str ());
		m_ListCtrl.SetItemText(Index,8,text);
		}

	if(m_pDoc2==NULL)  // single document view
	{
		sprintf_s(text, "%5.2f",(*iLink)->m_StaticVOC    );
		m_ListCtrl.SetItemText(Index,9,text);

		sprintf_s(text, "%5.2f",(*iLink)->m_StaticSpeed    );
		m_ListCtrl.SetItemText(Index,10,text);

		sprintf_s(text, "%d",(*iLink)->m_TotalVolume     );
		m_ListCtrl.SetItemText(Index,11,text);

		sprintf_s(text, "%.0f",(*iLink)->m_ReferenceFlowVolume     );
		m_ListCtrl.SetItemText(Index,12,text);

		float error = 0;
		if( (*iLink)->m_ReferenceFlowVolume >=1)
			error = (*iLink)->m_TotalVolume - (*iLink)->m_ReferenceFlowVolume ;
		sprintf_s(text, "%.0f", error);
		m_ListCtrl.SetItemText(Index,13,text);

		float error_percentage = error / max(1,(*iLink)->m_ReferenceFlowVolume)*100 ;
		sprintf_s(text, "%.1f", error_percentage);
		m_ListCtrl.SetItemText(Index,14,text);

	}else
	{     // two document view


		sprintf_s(text, "%5.2f",(*iLink)->m_StaticVOC    );
		m_ListCtrl.SetItemText(Index,9,text);

		//9  
		// 10

		sprintf_s(text, "%5.2f",(*iLink)->m_StaticSpeed    );
		m_ListCtrl.SetItemText(Index,12,text);

		// 12
		//13

		sprintf_s(text, "%d",(*iLink)->m_TotalVolume     );
		m_ListCtrl.SetItemText(Index,15,text);

		// 15 m_pDoc2
		DTALink* pLink2 = m_pDoc2->FindLinkWithNodeNumbers ((*iLink)->m_FromNodeNumber ,(*iLink)->m_ToNodeNumber );
		if(pLink2!=NULL)  // a link is found in the second document
		{
		sprintf_s(text, "%d",pLink2->m_TotalVolume);
		m_ListCtrl.SetItemText(Index,16,text);

		int diff = pLink2->m_TotalVolume - (*iLink)->m_TotalVolume;

		sprintf_s(text, "%d",diff);
		m_ListCtrl.SetItemText(Index,17,text);

		
		}
		// 16


	}

	}

}

void CDlgLinkList::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	
	UpdateData(1);

	// test valid documents
	if(g_TestValidDocument(m_pDoc)==false)
		return;

	if(m_bDoc2Ready)
	{
	if(g_TestValidDocument(m_pDoc2)==false)
		return;
	}

	// 
	m_pDoc->m_SelectedLinkID = -1;

	if(m_bDoc2Ready)
		m_pDoc2->m_SelectedLinkID = -1;

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

			if(m_pDoc2)
				m_pDoc2->m_SelectedLinkID = LinkNo;

			g_LinkDisplayList.push_back(LinkNo);

	}

	if(m_ZoomToSelectedLink == true)
	{

		m_pDoc->ZoomToSelectedLink(m_pDoc->m_SelectedLinkID);

		if(m_bDoc2Ready)
			m_pDoc2->ZoomToSelectedLink(m_pDoc->m_SelectedLinkID);
	}

	Invalidate();
		
	m_pDoc->UpdateAllViews(0);

		if(m_bDoc2Ready)
			m_pDoc2->UpdateAllViews(0);

}

void CDlgLinkList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}

void CDlgLinkList::OnBnClickedCancel()
{
	CDialog::OnOK();
	g_bShowLinkList = false;
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
