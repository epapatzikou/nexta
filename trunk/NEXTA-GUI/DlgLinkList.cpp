// DlgLinkList.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgLinkList.h"

extern std::list<DTALink*>	g_LinkDisplayList;
// CDlgLinkList dialog

IMPLEMENT_DYNAMIC(CDlgLinkList, CDialog)

CDlgLinkList::CDlgLinkList(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlgLinkList::IDD, pParent)
{
 m_AVISensorFlag = false;
}

CDlgLinkList::~CDlgLinkList()
{
}

void CDlgLinkList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_LinkList);
}


BEGIN_MESSAGE_MAP(CDlgLinkList, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CDlgLinkList::OnLvnItemchangedList)
	ON_BN_CLICKED(IDOK, &CDlgLinkList::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgLinkList message handlers
#define LINKCOLUMNSIZE 11
#define LINKCOLUMNSIZE_AVI 7

BOOL CDlgLinkList::OnInitDialog()
{
	CDialog::OnInitDialog();

	// insert columns


_TCHAR *ColumnMOELabel[LINKCOLUMNSIZE] =
{
	_T("No."),_T("From Node"), _T("To Node"), _T("Length (ml)"), _T("Num of Lanes"), _T("Speed Limit"),
	_T("Lane Capacity"), _T("Type"), _T("VOC"),_T("Volume"), _T("Speed")
};

_TCHAR *ColumnMOELabel_AVI[LINKCOLUMNSIZE_AVI] =
{
	_T("Pair #"),_T("From Node"), _T("To Node"), _T("Sensor Type"),_T("Num of Samples"), _T("Average Speed"),
	_T("Speed Variation")
};

int ColumnMOEWidth[LINKCOLUMNSIZE] =
{
	50,80,80,70,80,70,80,40,70,70,70
};


int i;
	LV_COLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	int column_size;

	if(!m_AVISensorFlag)
	{
	column_size = LINKCOLUMNSIZE;
	}else
	{
	column_size = LINKCOLUMNSIZE_AVI;
	}

	for(i = 0; i< column_size; i++)
	{
		lvc.iSubItem = i;
		lvc.pszText = ColumnMOELabel[i];
		lvc.cx = ColumnMOEWidth[i];
		lvc.fmt = LVCFMT_LEFT;
		m_LinkList.InsertColumn(i,&lvc);
	}
	m_LinkList.SetExtendedStyle(LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT |LVS_EX_HEADERDRAGDROP);

	LV_ITEM lvi;

	std::list<DTALink*>::iterator iLink;

	if(m_AVISensorFlag == false)
	{
	i = 0;
	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++, i++)
	{
		if((*iLink)->m_AVISensorFlag == false )
		{
		char text[100];
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		sprintf_s(text, "%d",(*iLink)->m_LinkNo +1) ;
		lvi.pszText = text;
		m_LinkList.InsertItem(&lvi);
		}

	}

	i=0;
	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++, i++)
	{
		if((*iLink)->m_AVISensorFlag == false )
		{
		char text[100];

		sprintf_s(text, "%d",(*iLink)->m_FromNodeNumber);
		m_LinkList.SetItemText(i,1,text);

		sprintf_s(text, "%d",(*iLink)->m_ToNodeNumber);
		m_LinkList.SetItemText(i,2,text);

		sprintf_s(text, "%5.2f",(*iLink)->m_Length);
		m_LinkList.SetItemText(i,3,text);

		sprintf_s(text, "%d",(*iLink)->m_NumLanes );
		m_LinkList.SetItemText(i,4,text);

		sprintf_s(text, "%4.0f",(*iLink)->m_SpeedLimit  );
		m_LinkList.SetItemText(i,5,text);

		sprintf_s(text, "%4.0f",(*iLink)->m_LaneCapacity   );
		m_LinkList.SetItemText(i,6,text);

		sprintf_s(text, "%d",(*iLink)->m_link_type    );
		m_LinkList.SetItemText(i,7,text);

		if((*iLink)->m_StaticVOC >0)
		{
		sprintf_s(text, "%5.2f",(*iLink)->m_StaticVOC    );
		m_LinkList.SetItemText(i,8,text);

		sprintf_s(text, "%5.2f",(*iLink)->m_StaticSpeed    );
		m_LinkList.SetItemText(i,9,text);

		sprintf_s(text, "%5.0f",(*iLink)->m_StaticLinkVolume     );
		m_LinkList.SetItemText(i,10,text);
		}
		}

	}
	}else
	{
		i = 0;
   std::map< long, CAVISensorPair >::const_iterator iter ;

   for ( iter= m_pDoc->m_AVISensorMap.begin(); iter !=  m_pDoc->m_AVISensorMap.end(); iter++ )
   {
       
		char text[100];
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		sprintf_s(text, "%d",iter->second.sensor_pair_id ) ;
		lvi.pszText = text;
		m_LinkList.InsertItem(&lvi);

	}

	i=0;
   for ( iter= m_pDoc->m_AVISensorMap.begin(); iter !=  m_pDoc->m_AVISensorMap.end(); iter++ )
	{
		char text[100];

		sprintf_s(text, "%d",iter->second.from_node_id);
		m_LinkList.SetItemText(i,1,text);

		sprintf_s(text, "%d",iter->second.to_node_id );
		m_LinkList.SetItemText(i,2,text);

		m_LinkList.SetItemText(i,3,iter->second.sensor_type);

		sprintf_s(text, "%d",iter->second.number_of_samples  );
		m_LinkList.SetItemText(i,2,text);

	}
	
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgLinkList::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_pDoc->m_SelectedLinkID = -1;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
		 
	g_LinkDisplayList.clear ();

	POSITION pos = m_LinkList.GetFirstSelectedItemPosition();

	if (pos != NULL)
	{
	if(m_AVISensorFlag == false)
	{
		m_pDoc->m_SelectedLinkID = m_LinkList.GetNextSelectedItem(pos);
		g_LinkDisplayList.push_back(m_pDoc->m_LinkNoMap[m_pDoc->m_SelectedLinkID]);

	}else
	{

//	m_pDoc->m_SelectedLinkID =  m_pDoc->m_AVISensorMap [m_LinkList.GetNextSelectedItem(pos)];
	g_LinkDisplayList.push_back(m_pDoc->m_LinkNoMap[m_pDoc->m_SelectedLinkID]);
	}


		Invalidate();

		m_pDoc->UpdateAllViews(0);
	}

}

void CDlgLinkList::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}
