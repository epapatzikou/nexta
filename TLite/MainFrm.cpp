// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "TLite.h"
#include "TLiteDoc.h"
#include "GLView.h"


#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_NEW3DVIEW, &CMainFrame::OnWindowNew3dview)
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
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
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


	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

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
    }
 
    pTemplate->InitialUpdateFrame(pFrame, pDocument);
}
