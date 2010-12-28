// TSView.cpp : implementation of the CTimeSpaceView class
//

#include "stdafx.h"
#pragma warning(disable: 4786) // kill exceedingly long STL symbol warning
#include <set>
#include "TLite.h"
#include "TLiteDoc.h"
#include "Network.h"

#include "TSView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPen ThickPen(PS_SOLID,3,RGB(0,255,0));
CPen DoublePen(PS_SOLID,2,RGB(0,255,0));

CPen HighSpeedPen(PS_SOLID,2,RGB(255,0,0));
CPen MediumSpeedPen(PS_SOLID,2,RGB(0,0,0));

CPen HighSpeedPen_Scenario(PS_DASH,0,RGB(255,0,0));
CPen MediumSpeedPen_Scenario(PS_DASH,0,RGB(0,0,0));

CPen NormalPen(PS_SOLID,0,RGB(0,255,0));

CPen Normal2Pen(PS_SOLID,2,RGB(0,0,255));
CPen BlackPen(PS_SOLID,1,RGB(0,0,0));
CPen RedPen(PS_SOLID,1,RGB(255,0,0));

CPen DashPen(PS_DASH,0,RGB(0,255,0));
CPen DoubleDashPen(PS_DASH,2,RGB(0,255,0));
CPen SelectPen(PS_DASH,2,RGB(255,0,0));


std::set<CTimeSpaceView*> g_set_TSView;
extern void g_UpdateAllViews();

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView

IMPLEMENT_DYNCREATE(CTimeSpaceView, CView)

BEGIN_MESSAGE_MAP(CTimeSpaceView, CView)
	//{{AFX_MSG_MAP(CTimeSpaceView)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView construction/destruction
CTimeSpaceView::CTimeSpaceView()
{
	m_bShowTimetable = true;
	m_bShowSlackTime = true;

   m_TmLeft=0;
   m_TmRight=60 * 8;
   m_Left=20;
   m_Right=20;
   m_Top=20;
   m_Bottom=20;

}

CTimeSpaceView::~CTimeSpaceView()
{
}

BOOL CTimeSpaceView::PreCreateWindow(CREATESTRUCT& cs)
{

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView drawing

void CTimeSpaceView::OnDraw(CDC* pDC)
{
	CTLiteDoc* pDoc = GetDocument();

	ASSERT_VALID(pDoc);

	// draw select path
	m_SegmentVector.clear();
	if(g_PathDisplayList.size()>=g_SelectPathNo && g_SelectPathNo!=-1)
	{

	m_TotalDistance = 0;

	unsigned int i;
	for (i=0 ; i<g_PathDisplayList[g_SelectPathNo]->m_LinkSize; i++)
	{
		CTSSegment segment;

		DTALink* pLink = pDoc->m_LinkMap[g_PathDisplayList[g_SelectPathNo]->m_LinkVector[i]];
		if(pLink!=NULL)
		{
		m_TotalDistance+=pLink->m_Length ;
		segment.Distance = m_TotalDistance;
		m_SegmentVector.push_back(segment);
		}
	}
   
  pViewDC = pDC;
   CBrush brush;
   if (!brush.CreateSolidBrush(RGB(255,255,255)))
      return;

   CRect drawrect;
   GetClientRect(drawrect);

   brush.UnrealizeObject();
   pDC->FillRect(drawrect, &brush);

   drawrect.NormalizeRect();

   double m_SizeX= drawrect.Width () - m_Left -m_Right;
   double m_SizeY= drawrect.Height () - m_Top -m_Bottom;

   m_TotalDistance = m_SegmentVector[m_SegmentVector.size()-1].Distance;

   m_XScale = m_SizeX *1.0 / (m_TmRight - m_TmLeft)  ;
   m_YScale = m_SizeY *1.0 / m_TotalDistance  ;

   // Draw Framework for Time Table
   pDC->SelectObject (&ThickPen);

   Draw(0,m_TmLeft,0,m_TmRight);
   Draw(m_SegmentVector.size(),m_TmLeft,m_SegmentVector.size(),m_TmRight);

   Draw(0,m_TmLeft,m_SegmentVector.size(),m_TmLeft);
   Draw(0,m_TmRight,m_SegmentVector.size(),m_TmRight);

   // Draw station line
   pDC->SelectObject (&DoublePen);
   for(i=0; i< m_SegmentVector.size()+1;i++)
      Draw(i,m_TmLeft,i,m_TmRight);

   // Draw time  line per two minutes
   pDC->SelectObject (&NormalPen);

   // Draw time  line per ten minutes
   if(m_GridFlag <= 1)
   {
      pDC->SelectObject (&DoublePen);
      for(i=m_TmLeft; i<m_TmRight;i+=10)
         Draw(0,i,m_SegmentVector.size(),i);
   }

   if(m_GridFlag <= 2){
      for(i=m_TmLeft; i<m_TmRight;i+=60)
      {
         pDC->SelectObject (&Normal2Pen);
         Draw(0,i,m_SegmentVector.size(),i);
      }
   }

/*
   if(m_bShowTimetable)
   {
      // Draw Train timetable
      for(i=0;i<  pDoc->m_TrainSize; i++)
         DrawTrain(i);
   
   }
	  
/*
   // Draw random scenario

	  if(g_SelectedSenario == 1)
	  {
		  int s;
		  for(s=0; s< g_OptScenarioSize ; s++)
		  DrawSchedule(s);
	  }

	  if(g_SelectedSenario > 1)
	  {
	  DrawSchedule(g_SelectedSenario-2);  // starting from 0
	  }

	  */
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView printing

BOOL CTimeSpaceView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CTimeSpaceView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CTimeSpaceView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView diagnostics

#ifdef _DEBUG
void CTimeSpaceView::AssertValid() const
{
	CView::AssertValid();
}

void CTimeSpaceView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTLiteDoc* CTimeSpaceView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTLiteDoc)));
	return (CTLiteDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView message handlers

void CTimeSpaceView::Draw(int Sta0, int Time0, int Sta1, int Time1,bool bText, int Value)
{
	CTLiteDoc* pDoc = GetDocument();

   int x0,y0,x1,y1;
   x0 = m_Left + int ((Time0 - m_TmLeft) * m_XScale);
   x1 = m_Left + int ((Time1 - m_TmLeft) * m_XScale);

   y0 = int (( m_TotalDistance - m_SegmentVector[Sta0-1].Distance  )* m_YScale ) + m_Top;
   y1 = int (( m_TotalDistance - m_SegmentVector[Sta1].Distance )* m_YScale ) + m_Top;

   pViewDC->MoveTo (x0,y0);
   pViewDC->LineTo (x1,y1);

   if(bText && Value > 0)
   {

	CString str;
	str.Format("%d", Value);

	pViewDC->TextOut((x0+x1)/2, (y0+y1)/2,str );

      
   }

}

void CTimeSpaceView::DrawTrain(int TrainNo)
{

	CTLiteDoc* pDoc = GetDocument();
	   int i;
 /*   
    if(pDoc->m_Trains[TrainNo].TrainType == 0)
	{
	    pViewDC->SelectObject (&HighSpeedPen);
		pViewDC->SetTextColor(RGB(255,0,0));
	}
	else
	{
	   pViewDC->SelectObject (&MediumSpeedPen);
       pViewDC->SetTextColor(RGB(0,0,0));
	}

 //   if(g_SelectTrainNo == TrainNo)
 //        pViewDC->SelectObject (&SelectPen);

   //Loop for each segment
   for(i = 0;i< m_SegmentVector.size(); i++)
   {
	  int SlackTime = pDoc->m_Trains[TrainNo].Table[i][1] - pDoc->m_Trains[TrainNo].Table[i][0] -  m_SegmentVector[i].RunningTime[pDoc->m_Trains[TrainNo].TrainType];

      Draw(i,pDoc->m_Trains[TrainNo].Table[i][0],i+1,pDoc->m_Trains[TrainNo].Table[i][1], m_bShowSlackTime, SlackTime);
   }
*/
}



void CTimeSpaceView::DrawSchedule(int ScenarioNo)
{
	CTLiteDoc* pDoc = GetDocument();
/*
  int j;
  int i;
      for(j=0;j< pDoc->m_TrainSize; j++)
      {
		if(pDoc->m_Trains[j].TrainType == 0)
			pViewDC->SelectObject (&HighSpeedPen_Scenario);
		else
		   pViewDC->SelectObject (&MediumSpeedPen_Scenario);

		for(i = 0;i< m_SegmentVector.size(); i++)
		   {
			  Draw(i,pDoc->m_Trains[j].Schedule[ScenarioNo][i][0],i+1,pDoc->m_Trains[j].Schedule[ScenarioNo][i][1]);
		   }
	  
	  }

*/
}

void CTimeSpaceView::RefreshWindow()
{
   OnUpdate(0, 0, 0);

}

void CTimeSpaceView::OnShowTimetable() 
{

   m_bShowTimetable = !m_bShowTimetable;
	
   OnUpdate(0, 0, 0);
	
}

void CTimeSpaceView::OnUpdateShowTimetable(CCmdUI* pCmdUI) 
{

   pCmdUI->SetCheck(m_bShowTimetable ? 1 : 0);
	
	
}

void CTimeSpaceView::OnShowSlacktime() 
{
	m_bShowSlackTime = !m_bShowSlackTime;
    OnUpdate(0, 0, 0);
	
}

void CTimeSpaceView::OnUpdateShowSlacktime(CCmdUI* pCmdUI) 
{
   pCmdUI->SetCheck(m_bShowSlackTime ? 1 : 0);
	
}
