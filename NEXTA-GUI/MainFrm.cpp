// MainFrm.cpp : implementation of the CMainFrame class
//http://www.axialis.com/free/icons/
// All the icons are licensed under the Creative Commons Attribution License.
// Icons: the credits of the authors: "Axialis Team"
//

#include "stdafx.h"
#include "TLite.h"
#include "TLiteDoc.h"
#include "TLiteView.h"
#include "GLView.h"
#include "TSView.h"

#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int  g_Player_Status=-1;	     //-1: edit 0: stop, 1: play, 2: pause
IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_COMMAND(ID_WINDOW_NEW3DVIEW, &CMainFrame::OnWindowNew3dview)
	ON_COMMAND(ID_SHOW_TIMETABLE, &CMainFrame::OnShowTimetable)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NEW3DVIEW, &CMainFrame::OnUpdateWindowNew3dview)
	ON_COMMAND(ID_WINDOW_SHOW2DVIEW, &CMainFrame::OnWindowShow2dview)
	ON_COMMAND(ID_3DDISPLAY_ANIMATION, &CMainFrame::On3ddisplayAnimation)
	ON_COMMAND(ID_ANIMATION_PLAY, &CMainFrame::OnAnimationPlay)
	ON_COMMAND(ID_ANIMATION_REWIND, &CMainFrame::OnAnimationRewind)
	ON_COMMAND(ID_ANIMATION_PAUSE, &CMainFrame::OnAnimationPause)
	ON_COMMAND(ID_ANIMATION_STOP, &CMainFrame::OnAnimationStop)
	ON_COMMAND(ID_VIEW_MOETOOLBAR, &CMainFrame::OnViewMoetoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOETOOLBAR, &CMainFrame::OnUpdateViewMoetoolbar)
	ON_COMMAND(ID_WINDOW_SHOWESTIMATIONVIEW, &CMainFrame::OnWindowShowestimationview)
	ON_COMMAND(ID_VIEW_DATATOOLBAR, &CMainFrame::OnViewDatatoolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DATATOOLBAR, &CMainFrame::OnUpdateViewDatatoolbar)
	ON_COMMAND(ID_ANIMATION_FORWARD, &CMainFrame::OnAnimationForward)
	ON_COMMAND(ID_ANIMATION_BACKWARD, &CMainFrame::OnAnimationBackward)
	ON_COMMAND(ID_ANIMATION_SKIPFORWARD, &CMainFrame::OnAnimationSkipforward)
	ON_COMMAND(ID_ANIMATION_SKIPBACKWARD, &CMainFrame::OnAnimationSkipbackward)
	ON_COMMAND(ID_WINDOW_SYNCHRONIZEDDISPLAY, &CMainFrame::OnWindowSynchronizeddisplay)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_SYNCHRONIZEDDISPLAY, &CMainFrame::OnUpdateWindowSynchronizeddisplay)
	ON_COMMAND(ID_VIEW_GIS_Layer_Panel, &CMainFrame::OnViewGisLayerPanel)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GIS_Layer_Panel, &CMainFrame::OnUpdateViewGisLayerPanel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_GISLAYER, &CMainFrame::OnLvnItemchangedListGislayer)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_GISLAYER, &CMainFrame::OnNMCustomdrawListGislayer)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_GISLAYER, &CMainFrame::OnNMRClickListGislayer)
	ON_BN_CLICKED(IDC_BUTTON_Database, &CMainFrame::OnBnClickedButtonDatabase)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction
#define _NUM_OF_GIS_LAYERS  16 
static _TCHAR *_gLayerLabel[_NUM_OF_GIS_LAYERS] =
{
	_T("Node"),
	_T("Link"),
	_T("Zone"),
	_T("Connector"),
	_T("Link MOE"),
	_T("OD Matrix"),
	_T("Detector"),
	_T("Subarea"), 
	_T("Work Zone"),
	_T("Incident"),
	_T("VMS"),
	_T("Toll"),
	_T("Crash"),
	_T("Ramp Meters"),
	_T("Bluetooth"),
	_T("GPS"),
};

#define _NUM_OF_RAIL_GIS_LAYERS  3 
static _TCHAR *_gRailLayerLabel[_NUM_OF_GIS_LAYERS] =
{
	_T("Node"),
	_T("Link"),
	_T("Schedule"),
};

CMainFrame::CMainFrame()
{
	m_bFeatureInfoInitialized  = false;
	m_bShowGISLayerToolBar = true;
	m_bShowMOEToolBar = true;
	m_bShowDataToolBar = true;
	m_bSynchronizedDisplay = true;
	m_iSelectedLayer = layer_link;

}

CMainFrame::~CMainFrame()
{
}
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(theApp.m_VisulizationTemplate == e_traffic_assignment)
		return OnCreate_TrafficNetwork(lpCreateStruct);

	if(theApp.m_VisulizationTemplate == e_train_scheduling)
		return OnCreate_RailNetwork(lpCreateStruct);

	return 0;
}
int CMainFrame::OnCreate_TrafficNetwork(LPCREATESTRUCT lpCreateStruct)
{

	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_wndToolBar.LoadToolBar(IDR_MAINFRAME2))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}

		if(!m_wndPlayerSeekBar.Create(this))
		{
			TRACE0("Failed to create m_wndPlayerSeekBar toolbar\n");
			return -1;     // fail to create
		}

		m_wndPlayerSeekBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
			CBRS_TOOLTIPS | CBRS_FLYBY);

		if (!m_MOEToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_MOEToolBar.LoadToolBar(IDR_VIEW_TOOLBAR))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}
		if (!m_wndStatusBar.Create(this) ||
			!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators)/sizeof(UINT)))
		{
			TRACE0("Failed to create status bar\n");
			return -1;      // fail to create
		}

		if (!m_wndReBar.Create(this) ||
			!m_wndReBar.AddBar(&m_wndToolBar) ||
			!m_wndReBar.AddBar(&m_wndPlayerSeekBar)
//			!m_wndReBar.AddBar(&m_wndLayerBar)
			)
		{
			TRACE0("Failed to create rebar\n");
			return -1;      // fail to create
		}


		m_MOEToolBar.SetButtonText(0,"Demand");
		m_MOEToolBar.SetButtonText(1,"Scenario");
		m_MOEToolBar.SetButtonText(2,"Simulation");
		m_MOEToolBar.SetButtonText(4,"Network");
		m_MOEToolBar.SetButtonText(5,"Animation");
		m_MOEToolBar.SetButtonText(6,"V/C");
		m_MOEToolBar.SetButtonText(7,"Volume");
		m_MOEToolBar.SetButtonText(8,"Speed");
		m_MOEToolBar.SetButtonText(9,"Queue");
		m_MOEToolBar.SetButtonText(11,"Emissions");
		m_MOEToolBar.SetButtonText(12,"Reliability");
		m_MOEToolBar.SetButtonText(13,"Safety");
		m_MOEToolBar.SetButtonText(15,"Link");
		m_MOEToolBar.SetButtonText(16,"Path");
		m_MOEToolBar.SetButtonText(17,"Vehicle");
		m_MOEToolBar.SetButtonText(18,"System");
		

		m_MOEToolBar.SetSizes(CSize(42,38),CSize(16,15));

		m_MOEToolBar.EnableDocking(CBRS_ALIGN_ANY);
		EnableDocking(CBRS_ALIGN_ANY);
		DockControlBar(&m_MOEToolBar);

		SetTimer(0,1000, NULL); // simulation reflesh timer
		m_wndPlayerSeekBar.Enable(true);
		//	   m_wndPlayerSeekBar.SetRange(0,100);
		m_wndPlayerSeekBar.SetRange(0,g_Simulation_Time_Horizon);

		//	// TODO: Delete these three lines if you don't want the toolbar to be dockable
		//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
		//	EnableDocking(CBRS_ALIGN_ANY);
		//	DockControlBar(&m_wndToolBar);

	if (!m_GISLayerBar.Create(this, IDD_DIALOG_GISLayer,
		CBRS_LEFT|CBRS_TOOLTIPS|CBRS_FLYBY, IDD_DIALOG_GISLayer))
	{
		TRACE0("Failed to create DlgBar\n");
		return -1;      // fail to create
	}


CListCtrl * pGISLayerList = (CListCtrl *)m_GISLayerBar.GetDlgItem(IDC_LIST_GISLAYER);

pGISLayerList->InsertColumn(0,"Layer",LVCFMT_LEFT,100);

	LV_ITEM lvi;
	for(int i = 0; i < _NUM_OF_GIS_LAYERS; i++)
	{
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = _gLayerLabel[i];
		pGISLayerList->InsertItem(&lvi);
		m_bShowLayerMap[(layer_mode)(i)] = false;

	}



	pGISLayerList->SetExtendedStyle(LVS_EX_CHECKBOXES);


	m_bShowLayerMap[layer_node] = true;
	m_bShowLayerMap[layer_link] = true;
	m_bShowLayerMap[layer_link_MOE] = true;
	m_bShowLayerMap[layer_zone] = true;
	m_bShowLayerMap[layer_detector] = true;

	for(int i = 0; i < _NUM_OF_GIS_LAYERS; i++)
	{

	pGISLayerList->SetCheck(i,m_bShowLayerMap[(layer_mode)(i)]);
	}

	return 0;
}

int CMainFrame::OnCreate_RailNetwork(LPCREATESTRUCT lpCreateStruct)
{

	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_wndToolBar.LoadToolBar(IDR_MAINFRAME_RAIL))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}

		if(!m_wndPlayerSeekBar.Create(this))
		{
			TRACE0("Failed to create m_wndPlayerSeekBar toolbar\n");
			return -1;     // fail to create
		}

		m_wndPlayerSeekBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
			CBRS_TOOLTIPS | CBRS_FLYBY);

		if (!m_wndStatusBar.Create(this) ||
			!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators)/sizeof(UINT)))
		{
			TRACE0("Failed to create status bar\n");
			return -1;      // fail to create
		}

		if (!m_wndReBar.Create(this) ||
			!m_wndReBar.AddBar(&m_wndToolBar) ||
			!m_wndReBar.AddBar(&m_wndPlayerSeekBar)
			)
		{
			TRACE0("Failed to create rebar\n");
			return -1;      // fail to create
		}


		SetTimer(0,1000, NULL); // simulation reflesh timer
		m_wndPlayerSeekBar.Enable(true);
		//	   m_wndPlayerSeekBar.SetRange(0,100);
		m_wndPlayerSeekBar.SetRange(0,g_Simulation_Time_Horizon);

		//	// TODO: Delete these three lines if you don't want the toolbar to be dockable
		//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
		//	EnableDocking(CBRS_ALIGN_ANY);
		//	DockControlBar(&m_wndToolBar);

	if (!m_GISLayerBar.Create(this, IDD_DIALOG_GISLayer,
		CBRS_LEFT|CBRS_TOOLTIPS|CBRS_FLYBY, IDD_DIALOG_GISLayer))
	{
		TRACE0("Failed to create DlgBar\n");
		return -1;      // fail to create
	}


CListCtrl * pGISLayerList = (CListCtrl *)m_GISLayerBar.GetDlgItem(IDC_LIST_GISLAYER);

pGISLayerList->InsertColumn(0,"Layer",LVCFMT_LEFT,100);

	LV_ITEM lvi;
	for(int i = 0; i < _NUM_OF_RAIL_GIS_LAYERS; i++)
	{
		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = _gRailLayerLabel[i];
		pGISLayerList->InsertItem(&lvi);
		m_bShowLayerMap[(layer_mode)(i)] = false;

	}
	pGISLayerList->SetExtendedStyle(LVS_EX_CHECKBOXES);

	m_bShowLayerMap[layer_node] = true;
	m_bShowLayerMap[layer_link] = true;
	m_bShowLayerMap[layer_link_MOE] = true;

	for(int i = 0; i < _NUM_OF_RAIL_GIS_LAYERS; i++)
	{

	pGISLayerList->SetCheck(i,m_bShowLayerMap[(layer_mode)(i)]);
	}

	return 0;
}
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers







void CMainFrame::OnWindowNew3dview()
{
	//CMDIChildWnd* pActiveChild = MDIGetActive();
	//CDocument* pDocument;
	//if (pActiveChild == NULL || (pDocument = pActiveChild->GetActiveDocument()) == NULL) {
	//	TRACE("Warning:  No active document for WindowNew command\n");
	//	AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
	//	return; // Command failed
	//}
	//// Otherwise, we have a new frame!
	//CDocTemplate* pTemplate = ((CTLiteApp*) AfxGetApp())->m_pTemplateGLView;
	//ASSERT_VALID(pTemplate);

	//CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	//if (pFrame == NULL)
	//{
	//	TRACE("Warning:  failed to create new frame\n");
	//	AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
	//	return; // Command failed
	//	// make it visisable
	//}
	//pTemplate->InitialUpdateFrame(pFrame, pDocument);

}

void CMainFrame::OnShowTimetable()
{

        CMDIChildWnd* pActiveChild = MDIGetActive();
        CDocument* pDocument;
        if (pActiveChild == NULL || (pDocument = pActiveChild->GetActiveDocument()) == NULL) {
                TRACE("Warning:  No active document for WindowNew command\n");
                AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
                return; // Command failed
        }

        // Otherwise, we have a new frame!
        CDocTemplate* pTemplate = ((CTLiteApp*) AfxGetApp())->m_pTemplateTimeTableView;
        ASSERT_VALID(pTemplate);

        CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
        if (pFrame == NULL)
        {
                TRACE("Warning:  failed to create new frame\n");
                AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
                return; // Command failed
        }
        pTemplate->InitialUpdateFrame(pFrame, pDocument);

}

void CMainFrame::OnUpdateWindowNew3dview(CCmdUI *pCmdUI)
{

}


void CMainFrame::OnWindowShow2dview()
{
	MDITile(MDITILE_VERTICAL);
}

void CMainFrame::On3ddisplayAnimation()
{
	// TODO: Add your command handler code here
}
void CMainFrame::OnTimer(UINT_PTR  nIDEvent)
{

	if(g_Player_Status == 1) // play
	{
		if(g_Simulation_Time_Stamp < g_Simulation_Time_Horizon)
		{
			g_Simulation_Time_Stamp += 1.0f;  // min
		}else
		{
			g_Simulation_Time_Stamp = g_Simulation_Time_Horizon;
			g_Player_Status = 0; // automatically stops when we reach the end of simulation horizon

		}

		m_wndPlayerSeekBar.SetPos(g_Simulation_Time_Stamp);

		UpdateAllViews();
	}

}
void CMainFrame::OnAnimationPlay()
{
	g_Player_Status = 1;

	UpdateAllViews();
}

void CMainFrame::OnAnimationRewind()
{
	g_Player_Status = 1;
	g_Simulation_Time_Stamp = g_SimulationStartTime_in_min;
	m_wndPlayerSeekBar.SetPos(g_Simulation_Time_Stamp);

	UpdateAllViews();
}

void CMainFrame::OnAnimationPause()
{
	g_Player_Status = 2;
}

void CMainFrame::OnAnimationStop()
{
	g_Player_Status = 0;
	g_Simulation_Time_Stamp = g_SimulationStartTime_in_min;
	m_wndPlayerSeekBar.SetPos(g_Simulation_Time_Stamp);

	UpdateAllViews();
}

void CMainFrame::OnViewMoetoolbar()
{
	m_bShowMOEToolBar= !m_bShowMOEToolBar;
	if(m_bShowMOEToolBar)
		m_MOEToolBar.ShowWindow (true);
	else
		m_MOEToolBar.ShowWindow (false);
}

void CMainFrame::OnUpdateViewMoetoolbar(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (m_bShowMOEToolBar);
}

void CMainFrame::UpdateAllViews()
{

	CTLiteApp* pApp;
	pApp = (CTLiteApp *) AfxGetApp(); 
	pApp->UpdateAllViews();

}
void CMainFrame::OnWindowShowestimationview()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL || (pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warning:  No active document for WindowNew command\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Command failed
	}
	// Otherwise, we have a new frame!
	//CDocTemplate* pTemplate = ((CTLiteApp*) AfxGetApp())->m_pTemplateGLView;
	//ASSERT_VALID(pTemplate);

	//CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	//if (pFrame == NULL)
	//{
	//	TRACE("Warning:  failed to create new frame\n");
	//	AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
	//	return; // Command failed
	//	// make it visisable
	//}
	//pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

void CMainFrame::OnViewDatatoolbar()
{
}

void CMainFrame::OnUpdateViewDatatoolbar(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (m_bShowDataToolBar);
}

void CMainFrame::OnAnimationForward()
{
	m_wndPlayerSeekBar.SetPos(min(g_Simulation_Time_Stamp+1,g_Simulation_Time_Horizon));
	UpdateAllViews();

}

void CMainFrame::OnAnimationBackward()
{
	m_wndPlayerSeekBar.SetPos(max(g_Simulation_Time_Stamp-1,0));
	UpdateAllViews();
}

void CMainFrame::OnAnimationSkipforward()
{
	m_wndPlayerSeekBar.SetPos(min(g_Simulation_Time_Stamp+5,g_Simulation_Time_Horizon));
	UpdateAllViews();
}

void CMainFrame::OnAnimationSkipbackward()
{
	m_wndPlayerSeekBar.SetPos(max(g_Simulation_Time_Stamp-5,0));
	UpdateAllViews();
}

void CMainFrame::OnWindowSynchronizeddisplay()
{
	m_bSynchronizedDisplay = !m_bSynchronizedDisplay;
}

void CMainFrame::OnUpdateWindowSynchronizeddisplay(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (m_bSynchronizedDisplay);
}


void CMainFrame::OnViewGisLayerPanel()
{
	m_bShowGISLayerToolBar= !m_bShowGISLayerToolBar;
	if(m_bShowGISLayerToolBar)
		ShowControlBar(&m_GISLayerBar,true,false);
	else
		ShowControlBar(&m_GISLayerBar,false,false);
}

void CMainFrame::OnUpdateViewGisLayerPanel(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (m_bShowGISLayerToolBar);
}

void CMainFrame::OnLvnItemchangedListGislayer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

CListCtrl * pGISLayerList = (CListCtrl *)m_GISLayerBar.GetDlgItem(IDC_LIST_GISLAYER);

	POSITION pos = pGISLayerList->GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int nItem = pGISLayerList->GetNextSelectedItem(pos);
		 m_iSelectedLayer = (layer_mode) nItem;
		 bool bChecked = pGISLayerList->GetCheck(nItem); 

	 	m_bShowLayerMap[m_iSelectedLayer] = bChecked;

 		 TRACE("%d: %s, %d\n",nItem,_gLayerLabel[nItem],bChecked);
	}

	pGISLayerList->Invalidate (1);
	UpdateAllViews();
}

void CMainFrame::OnNMCustomdrawListGislayer(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    // Take the default processing unless we set this to something else below.
    *pResult = CDRF_DODEFAULT;

    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.

    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        *pResult = CDRF_NOTIFYITEMDRAW;
        }
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        // This is the prepaint stage for an item. Here's where we set the
        // item's text color. Our return value will tell Windows to draw the
        // item itself, but it will use the new color we set here.
       
        COLORREF crText = RGB(0,0,0);

        if (pLVCD->nmcd.dwItemSpec  == (int)(m_iSelectedLayer) ) // first line
            crText = RGB(255,0,0);
        
        // Store the color back in the NMLVCUSTOMDRAW struct.
        pLVCD->clrText = crText;

        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
        }
}


void CMainFrame::FillFeatureInfo()
{
	CListCtrl * pGISFeatureList = (CListCtrl *)m_GISLayerBar.GetDlgItem(IDC_LIST_FEATURE_INFO);

	pGISFeatureList->DeleteAllItems ();
	
	if(!m_bFeatureInfoInitialized)
	{
	pGISFeatureList->InsertColumn(0,"Attribute",LVCFMT_LEFT,70);
	pGISFeatureList->InsertColumn(1,"Data",LVCFMT_LEFT,100);
	m_bFeatureInfoInitialized = true;
	}

	LV_ITEM lvi;
	for(int i = 0; i < m_FeatureInfoVector.size(); i++)
	{

		lvi.mask = LVIF_TEXT;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		char text_str[100];
		sprintf(text_str,"%s",  m_FeatureInfoVector[i].Attribute.c_str());

		lvi.pszText =  text_str;
		int Index = pGISFeatureList->InsertItem(&lvi);
		pGISFeatureList->SetItemText(Index,1,m_FeatureInfoVector[i].Data);
	}

}

void CMainFrame::OnNMRClickListGislayer(NMHDR *pNMHDR, LRESULT *pResult)
{
}

void CMainFrame::OnBnClickedButtonDatabase()
{

}

void CMainFrame::OnBnClickedButtonConfiguration()
{
	
}
