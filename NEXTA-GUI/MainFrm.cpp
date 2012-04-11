// MainFrm.cpp : implementation of the CMainFrame class
//http://www.axialis.com/free/icons/
// All the icons are licensed under the Creative Commons Attribution License.
// Icons: the credits of the authors: "Axialis Team"
//

#include "stdafx.h"
#include "TLite.h"
#include "TLiteDoc.h"
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
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_bShowMOEToolBar = true;
	m_bShowDataToolBar = true;

}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

		if (!m_wndReBar.Create(this) ||
			!m_wndReBar.AddBar(&m_wndToolBar) ||
			!m_wndReBar.AddBar(&m_wndPlayerSeekBar)
			)
		{
			TRACE0("Failed to create rebar\n");
			return -1;      // fail to create
		}


	

		if (!m_AMSToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_AMSToolBar.LoadToolBar(IDR_AMS_TOOLBAR))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}

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

		m_MOEToolBar.SetButtonText(0,"Data");
		m_MOEToolBar.SetButtonText(1,"Simulation");
		m_MOEToolBar.SetButtonText(2,"Scenario");
		m_MOEToolBar.SetButtonText(3,"Network");
		m_MOEToolBar.SetButtonText(4,"Animation");
		m_MOEToolBar.SetButtonText(5,"V/C");
		m_MOEToolBar.SetButtonText(6,"Volume");
		m_MOEToolBar.SetButtonText(7,"Speed");
		m_MOEToolBar.SetButtonText(8,"Queue");
		m_MOEToolBar.SetButtonText(10,"Emissions");
		m_MOEToolBar.SetButtonText(11,"Reliability");
		m_MOEToolBar.SetButtonText(12,"Safety");
		m_MOEToolBar.SetButtonText(14,"Link");
		m_MOEToolBar.SetButtonText(15,"Path");
		m_MOEToolBar.SetButtonText(16,"Subarea");
		m_MOEToolBar.SetButtonText(17,"System");
		

		m_MOEToolBar.SetSizes(CSize(42,38),CSize(16,15));


/*		m_AMSToolBar.SetButtonText(0,"TOOL");
		m_AMSToolBar.SetButtonText(2,"Network");
		m_AMSToolBar.SetButtonText(3,"Import");
		m_AMSToolBar.SetButtonText(4,"Sensor");
		m_AMSToolBar.SetButtonText(5,"GIS");
		m_AMSToolBar.SetButtonText(6,"Signal");
		m_AMSToolBar.SetButtonText(7,"Transit");
		m_AMSToolBar.SetButtonText(8,"Cloud");
		m_AMSToolBar.SetButtonText(9,"Background");

		m_AMSToolBar.SetButtonText(11,"Simulation");
		m_AMSToolBar.SetButtonText(12,"ODME");
		m_AMSToolBar.SetButtonText(13,"TFME");
		m_AMSToolBar.SetButtonText(14,"Prediction");
		m_AMSToolBar.SetButtonText(16,"Editing");
		m_AMSToolBar.SetButtonText(18,"Pricing");
		m_AMSToolBar.SetButtonText(19,"Capacity");
		m_AMSToolBar.SetButtonText(20,"ATIS");
		m_AMSToolBar.SetButtonText(22,"Website");

		m_AMSToolBar.SetSizes(CSize(42,38),CSize(16,15));
*/		m_AMSToolBar.EnableDocking(CBRS_ALIGN_ANY);
		EnableDocking(CBRS_ALIGN_ANY);
		DockControlBar(&m_AMSToolBar);

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
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL || (pDocument = pActiveChild->GetActiveDocument()) == NULL) {
		TRACE("Warning:  No active document for WindowNew command\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Command failed
	}
	// Otherwise, we have a new frame!
	CDocTemplate* pTemplate = ((CTLiteApp*) AfxGetApp())->m_pTemplateGLView;
	ASSERT_VALID(pTemplate);

	CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL)
	{
		TRACE("Warning:  failed to create new frame\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Command failed
		// make it visisable
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);

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
void CMainFrame::OnTimer(UINT nIDEvent)
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
	CDocTemplate* pTemplate = ((CTLiteApp*) AfxGetApp())->m_pTemplateGLView;
	ASSERT_VALID(pTemplate);

	CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL)
	{
		TRACE("Warning:  failed to create new frame\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return; // Command failed
		// make it visisable
	}
	pTemplate->InitialUpdateFrame(pFrame, pDocument);}

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
