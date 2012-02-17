// DlgTrainInfo.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgTrainInfo.h"


// CDlgTrainInfo dialog

#define NUM_TRAIN_INFO 8

static _TCHAR *_gColumnTrainLabel[NUM_TRAIN_INFO] =
{
	_T("Type"), _T("Train ID"), _T("Origin"), _T("Destination"),
	_T("Departure Time"), _T("# of Nodes"), _T("Pref. Arrival Time"), _T("Actual Trip Time")
};

static int _gColumnWidth[NUM_TRAIN_INFO] =
{
	70, 50, 50, 70, 90, 80, 100, 100
};

IMPLEMENT_DYNAMIC(CDlgTrainInfo, CDialog)

CDlgTrainInfo::CDlgTrainInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTrainInfo::IDD, pParent)
{

}

CDlgTrainInfo::~CDlgTrainInfo()
{
}

void CDlgTrainInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TRAIN_LIST_CONTROL, m_TrainListControl);
}


BEGIN_MESSAGE_MAP(CDlgTrainInfo, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_TRAIN_LIST_CONTROL, &CDlgTrainInfo::OnLvnItemchangedTrainListControl)
END_MESSAGE_MAP()


// CDlgTrainInfo message handlers
BOOL CDlgTrainInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_SmallImageList.Create(IDB_PATH_ICON, 16, 1, RGB(255, 255, 255));
	m_SmallImageList.SetOverlayImage(5, 1);

	m_TrainListControl.SetImageList(&m_SmallImageList, LVSIL_SMALL);

	// insert columns

	int i;
	LV_COLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for(i = 0; i<NUM_TRAIN_INFO; i++)
	{
		lvc.iSubItem = i;
		lvc.pszText = _gColumnTrainLabel[i];
		lvc.cx = _gColumnWidth[i];
		lvc.fmt = LVCFMT_LEFT;
		m_TrainListControl.InsertColumn(i,&lvc);
	}

	m_TrainListControl.SetExtendedStyle(LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT |LVS_EX_HEADERDRAGDROP);

	InsertTrainInfoItem();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgTrainInfo::InsertTrainInfoItem()
{
//	m_TrainListControl.DeleteAllItems ();
	// insert items

	
	unsigned i;

	for(i = 0; i<m_pDoc->m_TrainVector.size(); i++)
	{

		LV_ITEM lvi;
		char text[100];

		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
		lvi.iItem = i;
		lvi.iSubItem = 0;

		DTA_Train* pTrain = m_pDoc->m_TrainVector[i];

		sprintf_s(text, "%d",m_pDoc->m_TrainVector[i]->m_TrainID);

		lvi.pszText = text;
		int ImageNo = pTrain->m_TrainType-1;
		lvi.iImage = ImageNo;
		lvi.stateMask = LVIS_STATEIMAGEMASK;
		lvi.state = INDEXTOSTATEIMAGEMASK(ImageNo);

		m_TrainListControl.InsertItem(&lvi);

		sprintf_s(text, "%d",pTrain->m_TrainType);
		m_TrainListControl.SetItemText(i,0,text);

		sprintf_s(text, "%d",pTrain->m_TrainID);
		m_TrainListControl.SetItemText(i,1,text);

		sprintf_s(text, "%d",pTrain->m_OriginNodeNumber);
		m_TrainListControl.SetItemText(i,2,text);

		sprintf_s(text, "%d",pTrain->m_DestinationNodeNumber);
		m_TrainListControl.SetItemText(i,3,text);

		sprintf_s(text, "%d",pTrain->m_DepartureTime);
		m_TrainListControl.SetItemText(i,4,text);

		sprintf_s(text, "%d",pTrain->m_NodeSize);
		m_TrainListControl.SetItemText(i,5,text);

		if(pTrain->m_PreferredArrivalTime>0)
			sprintf_s(text, "%d",pTrain->m_PreferredArrivalTime);
		else
			sprintf_s(text, "-");

		m_TrainListControl.SetItemText(i,6,text);

		if(pTrain->m_ActualTripTime>0)
			sprintf_s(text, "%d",pTrain->m_ActualTripTime);
		else
			sprintf_s(text, "-");

		m_TrainListControl.SetItemText(i,7,text);


	}
}
void CDlgTrainInfo::OnLvnItemchangedTrainListControl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	m_pDoc->m_SelectedTrainID = -1;
		 
	POSITION pos = m_TrainListControl.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int SelectedNo = m_TrainListControl.GetNextSelectedItem(pos);
		
		DTA_Train* pTrain = m_pDoc->m_TrainVector[SelectedNo];

		m_pDoc->m_SelectedTrainID 	= pTrain ->m_TrainID ;

		Invalidate();

		m_pDoc->UpdateAllViews(0);
	}

}
