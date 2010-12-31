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

extern long g_Simulation_Time_Stamp;
extern CPen s_PenSimulationClock;
extern void g_SelectColorCode(CDC* pDC, int ColorCount);


CPen ThickPen(PS_SOLID,3,RGB(0,255,0));
CPen DoublePen(PS_SOLID,2,RGB(0,255,0));


CPen NormalPen(PS_SOLID,1,RGB(0,255,0));

CPen Normal2Pen(PS_SOLID,2,RGB(0,0,255));
CPen BlackPen(PS_SOLID,1,RGB(0,0,0));
CPen RedPen(PS_SOLID,1,RGB(255,0,0));

CPen DashPen(PS_DASH,0,RGB(0,255,0));
CPen DoubleDashPen(PS_DASH,2,RGB(0,255,0));
CPen SelectPen(PS_DASH,2,RGB(255,0,0));

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView

IMPLEMENT_DYNCREATE(CTimeSpaceView, CView)

BEGIN_MESSAGE_MAP(CTimeSpaceView, CView)
	//{{AFX_MSG_MAP(CTimeSpaceView)
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_TIMETABLE_RESOURCEPRICE, &CTimeSpaceView::OnTimetableResourceprice)
	ON_UPDATE_COMMAND_UI(ID_TIMETABLE_RESOURCEPRICE, &CTimeSpaceView::OnUpdateTimetableResourceprice)
	ON_COMMAND(ID_TIMETABLE_NODECOSTLABEL, &CTimeSpaceView::OnTimetableNodecostlabel)
	ON_UPDATE_COMMAND_UI(ID_TIMETABLE_NODECOSTLABEL, &CTimeSpaceView::OnUpdateTimetableNodecostlabel)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_TIMETABLE_EXPORTTIMETABLE, &CTimeSpaceView::OnTimetableExporttimetable)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView construction/destruction
CTimeSpaceView::CTimeSpaceView()
{
	m_bShowTimetable = true;

	m_bShowSlackTime = true;

	Cur_MOE_type1 = 0; 
	Cur_MOE_type2 = -1;
	m_TmLeft = 0;
	m_TmRight = 1400; 
	m_YLowerBound = 0;

	m_bShowResourcePrice = false;
	m_bShowNodeCostLabel = false;

	m_bMoveDisplay = false;


}

CTimeSpaceView::~CTimeSpaceView()
{
}

BOOL CTimeSpaceView::PreCreateWindow(CREATESTRUCT& cs)
{

	return CView::PreCreateWindow(cs);
}

CTLiteDoc* CTimeSpaceView::GetTLDocument() 
{ return reinterpret_cast<CTLiteDoc*>(m_pDocument); }

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView drawing

void CTimeSpaceView::OnDraw(CDC* pDC)
{
	CRect rectClient(0,0,0,0);
	GetClientRect(&rectClient);

	CRect PlotRect;
	GetClientRect(PlotRect);

	PlotRect.top += 60;
	PlotRect.bottom -= 60;
	PlotRect.left += 60;
	PlotRect.right -= 60;

	DrawObjects(pDC,Cur_MOE_type1, PlotRect);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rectClient.Width(),
		rectClient.Height());
	memDC.SelectObject(&bmp);


	// Custom draw on top of memDC
	CBrush brush;
	if (!brush.CreateSolidBrush(RGB(255,255,255)))
		return;
	brush.UnrealizeObject();
	memDC.FillRect(rectClient, &brush);

	DrawObjects(&memDC,Cur_MOE_type1,PlotRect);

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &memDC, 0,
		0, SRCCOPY);

	ReleaseDC(pDC);

	// TODO: add draw code for native data here

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


#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView message handlers

void CTimeSpaceView::DrawObjects(CDC* pDC,int MOEType,CRect PlotRect)
{
	if(m_TmLeft<0)
		m_TmLeft = 0;

	if(m_TmRight > 1400)
		m_TmRight = 1400;

	CPen DataPen(PS_SOLID,1,RGB(0,0,0));

	CPen TimePen(PS_DOT,1,RGB(0,255,0));

	CString str_MOE;

	str_MOE.Format ("Time-Space Graph");

	if(m_bShowResourcePrice)
		str_MOE+=" -- with Resource Price";

	if(m_bShowNodeCostLabel)
		str_MOE+=" -- with Node Cost Label";

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&NormalPen);


	pDC->TextOut(PlotRect.right/2-100,PlotRect.top-20,str_MOE);

	// step 1: calculate m_YUpperBound;
	m_YUpperBound = 0;

	int i;
	std::list<DTALink*>::iterator iLink;
	CTLiteDoc* pDoc = GetTLDocument();


	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		
		(*iLink)->m_FromNodeY = m_YUpperBound;
		m_YUpperBound+=(*iLink)->m_Length ;
		(*iLink)->m_ToNodeY = m_YUpperBound;

	}

	// step 2: calculate m_UnitDistance;
	// data unit
	m_UnitDistance = 1;
	if((m_YUpperBound - m_YLowerBound)>0)
		m_UnitDistance = (float)(PlotRect.bottom - PlotRect.top)/(m_YUpperBound - m_YLowerBound);


	// step 3: time interval
	int TimeXPosition;

	int TimeInterval = 5;

	if(m_TmRight - m_TmLeft >=1440*7)
		TimeInterval = 1440;
	else if(m_TmRight - m_TmLeft >=1440*4)
		TimeInterval = 720;
	else if(m_TmRight - m_TmLeft >=2800)
		TimeInterval = 360;
	else if(m_TmRight - m_TmLeft >=1400)
		TimeInterval = 120;
	else if(m_TmRight - m_TmLeft >=400)
		TimeInterval = 60;
	else if(m_TmRight - m_TmLeft >=120)
		TimeInterval = 30;
	else 
		TimeInterval = 10;

	// time unit
	m_UnitTime = 1;
	if((m_TmRight - m_TmLeft)>0)
		m_UnitTime = (float)(PlotRect.right - PlotRect.left)/(m_TmRight - m_TmLeft);

	// step 4: draw time axis

	pDC->SelectObject(&TimePen);

	char buff[20];
	for(i=m_TmLeft;i<=m_TmRight;i+=TimeInterval)
	{
		TimeXPosition=(int)(PlotRect.left+(i-m_TmLeft)*m_UnitTime);

		if(i>= m_TmLeft)
		{
			pDC->MoveTo(TimeXPosition,PlotRect.bottom+2);
			pDC->LineTo(TimeXPosition,PlotRect.top);

			if(i/2 <10)
				TimeXPosition-=5;
			else
				TimeXPosition-=3;

			if(TimeInterval < 60)
			{
				int hour, min;
				hour = i/60;
				min =  i- hour*60;
				wsprintf(buff,"%2d:%02d",hour, min);
			}
			else
			{
				int min_in_a_day = i-int(i/1440*1440);

				wsprintf(buff,"%dh",min_in_a_day/60 );

			}
			pDC->TextOut(TimeXPosition,PlotRect.bottom+3,buff);
		}
	}

	// 	step 4: draw time axis

	pDC->SelectObject(&NormalPen);

	pDC->MoveTo(PlotRect.left, PlotRect.top);
	pDC->LineTo(PlotRect.left,PlotRect.bottom);

	pDC->MoveTo(PlotRect.right, PlotRect.top);
	pDC->LineTo(PlotRect.right,PlotRect.bottom);


		int PrevToNode = -1;
		bool bSpace = false;

	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{

		int YFrom = PlotRect.bottom - (int)(((*iLink)->m_FromNodeY*m_UnitDistance)+0.50);
		int YTo= PlotRect.bottom - (int)(((*iLink)->m_ToNodeY*m_UnitDistance)+0.50);


		pDC->MoveTo(PlotRect.left, YFrom);
		pDC->LineTo(PlotRect.right,YFrom);

		pDC->MoveTo(PlotRect.left, YTo);
		pDC->LineTo(PlotRect.right,YTo);

		if(PrevToNode!=(*iLink)->m_FromNodeID)  // first from node or node change
		{
			bSpace = !bSpace;
			wsprintf(buff,"%d",(*iLink)->m_FromNodeNumber );
			pDC->TextOut(PlotRect.left-30-bSpace*10,YFrom-5,buff);
		}

			wsprintf(buff,"%d",(*iLink)->m_ToNodeNumber );
			pDC->TextOut(PlotRect.left-30-bSpace*10,YTo-5,buff);
		
		PrevToNode = (*iLink)->m_ToNodeNumber;
	}

	// draw trains
	for(unsigned int v = 0; v<pDoc->m_TrainVector.size(); v++)
	{

		DTA_Train* pTrain = pDoc->m_TrainVector[v];

		g_SelectColorCode(pDC, pTrain->m_TrainType-1);


		for(int n = 1; n< pTrain->m_NodeSize; n++)
		{
			DTALink* pLink = pDoc->m_LinkMap[pTrain->m_aryTN[n].LinkID];

			ASSERT(pLink!=NULL);
			int YFrom = PlotRect.bottom - (int)(pLink->m_FromNodeY*m_UnitDistance+0.50);
			int YTo= PlotRect.bottom - (int)(pLink->m_ToNodeY*m_UnitDistance+0.50);

			int TimeXFrom=(int)(PlotRect.left+(pTrain->m_aryTN[n-1].NodeTimestamp-m_TmLeft)*m_UnitTime);
			int TimeXTo=(int)(PlotRect.left+(pTrain->m_aryTN[n].NodeTimestamp -m_TmLeft)*m_UnitTime);

			pDC->MoveTo(TimeXFrom, YFrom);
			pDC->LineTo(TimeXTo,YTo);

			if(n==1)
			{
				sprintf_s(buff, "%d",pTrain->m_TrainID);
				pDC->TextOut(TimeXFrom,YFrom-5,buff);

			}

		}

	}

	// draw resourece price

		for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
		{

			int YFrom = PlotRect.bottom - (int)(((*iLink)->m_FromNodeY*m_UnitDistance)+0.50);
			int YTo= PlotRect.bottom - (int)(((*iLink)->m_ToNodeY*m_UnitDistance)+0.50);

			for(int t=m_TmLeft;t<=m_TmRight;t++)
			{
				int TimeXPosition=(int)(PlotRect.left+(t-m_TmLeft)*m_UnitTime);

				if(m_bShowResourcePrice  && (*iLink)->m_ResourceAry!=NULL 
					&& (*iLink)->m_ResourceAry [t].Price > 0 && t%2==0)  // t%2==0 to reduce display overlapping
				{
					sprintf_s(buff, "%3.1f",(*iLink)->m_ResourceAry [t].Price);
					pDC->TextOut(TimeXPosition,int((YFrom+YTo)/2.0f-5),buff);

				}

				if(m_bShowNodeCostLabel && pDoc->m_pNetwork !=NULL)
				{
					float NodeCostLabel = pDoc->m_pNetwork->TD_LabelCostAry[(*iLink)->m_ToNodeID][t];

					if(NodeCostLabel >=0 && NodeCostLabel<MAX_SPLABEL-1)
					{
					sprintf_s(buff, "%3.1f",NodeCostLabel);
					pDC->TextOut(TimeXPosition,YTo,buff);
					}

				}

			}

		}
}
void CTimeSpaceView::RefreshWindow()
{
	OnUpdate(0, 0, 0);

}


BOOL CTimeSpaceView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int CurrentTime = int((pt.x - 60.0f)/m_UnitTime + m_TmLeft);

	int LeftShiftSize = max(5,int(0.2*(CurrentTime-m_TmLeft))/10*10);
	int RightShiftSize = max(5,int(0.2*(m_TmRight-CurrentTime))/10*10);

	if(zDelta <	 0)
	{
		m_TmLeft-=LeftShiftSize;
		m_TmRight+=RightShiftSize;
	}
	else
	{
		m_TmLeft+=LeftShiftSize;
		m_TmRight-=RightShiftSize;
	}

	if(m_TmLeft<0)  m_TmLeft = 0;
	if(m_TmRight>1440) m_TmRight = 1440;

	Invalidate();
	return true;

}

void CTimeSpaceView::OnRButtonDown(UINT nFlags, CPoint point)
{

	int min_timestamp = 1440;
	int max_timestamp = 0;

	CTLiteDoc* pDoc = GetTLDocument();
	// narrow the range for display

	for(unsigned int v = 0; v<pDoc->m_TrainVector.size(); v++)
	{

		DTA_Train* pTrain = pDoc->m_TrainVector[v];

		for(int n = 0; n< pTrain->m_NodeSize; n++)
		{

			if(min_timestamp > pTrain->m_aryTN[n].NodeTimestamp)
				min_timestamp = pTrain->m_aryTN[n].NodeTimestamp;
				
			if(max_timestamp < pTrain->m_aryTN[n].NodeTimestamp)
				max_timestamp = pTrain->m_aryTN[n].NodeTimestamp;

		}
	}

	m_TmLeft = min_timestamp;
	m_TmRight = max_timestamp;

	Invalidate();
	CView::OnRButtonDown(nFlags, point);
}

void CTimeSpaceView::OnTimetableResourceprice()
{
	m_bShowResourcePrice = !m_bShowResourcePrice;
	Invalidate();

}

void CTimeSpaceView::OnUpdateTimetableResourceprice(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowResourcePrice);
}

void CTimeSpaceView::OnTimetableNodecostlabel()
{
	m_bShowNodeCostLabel = ! m_bShowNodeCostLabel;
	Invalidate();


}

void CTimeSpaceView::OnUpdateTimetableNodecostlabel(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowNodeCostLabel);
}

void CTimeSpaceView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_last_cpoint = point;
	AfxGetApp()->LoadCursor(IDC_MOVENETWORK);
	m_bMoveDisplay = true;

	CView::OnLButtonDown(nFlags, point);
}

void CTimeSpaceView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bMoveDisplay)
	{
		CSize OffSet = point - m_last_cpoint;
		int time_shift = max(1,OffSet.cx/m_UnitTime);
		m_TmLeft-= time_shift;
		m_TmRight-= time_shift;

		m_last_cpoint = point;

		Invalidate();
	}
	CView::OnMouseMove(nFlags, point);
}

void CTimeSpaceView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_bMoveDisplay)
	{
		CSize OffSet = point - m_last_cpoint;
		int time_shift = max(1,OffSet.cx/m_UnitTime);
		m_TmLeft-= time_shift;
		m_TmRight-= time_shift;

		AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		m_bMoveDisplay = false;
		Invalidate();
	}
	CView::OnLButtonUp(nFlags, point);
}

void CTimeSpaceView::OnTimetableExporttimetable()
{
	CString str;
	CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"(*.csv)|*.csv||", NULL);
	if(dlg.DoModal() == IDOK)
	{
		char fname[_MAX_PATH];
		wsprintf(fname,"%s", dlg.GetPathName());
		CWaitCursor wait;

		if(!ExportTimetableDataToCSVFile(fname))
		{
			str.Format("The file %s could not be opened.\nPlease check if it is opened by Excel.", fname);
			AfxMessageBox(str);
		}
	}

}
bool CTimeSpaceView::ExportTimetableDataToCSVFile(char csv_file[_MAX_PATH])
{

	FILE* st;
	fopen_s(&st,csv_file,"w");

	if(st!=NULL)
	{
		fprintf(st, "train ID,train type,origin,destination,departure time,# of nodes,preferred arrival time,actual trip time,,arc processing time,arc waiting time,timestamp_X,node sequence_Y\n");

		CTLiteDoc* pDoc = GetTLDocument();
		// narrow the range for display
		
	for(unsigned int v = 0; v<pDoc->m_TrainVector.size(); v++)
	{

		DTA_Train* pTrain = pDoc->m_TrainVector[v];

		pTrain->m_ActualTripTime = pTrain->m_aryTN[pTrain->m_NodeSize -1].NodeTimestamp - pTrain->m_aryTN[0].NodeTimestamp;

		fprintf(st,"%d,%d,%d,%d,%d,%d,%d,%d\n", pTrain->m_TrainID , pTrain->m_TrainType ,pDoc->m_NodeIDtoNameMap [pTrain->m_OriginNodeID] ,pDoc->m_NodeIDtoNameMap[pTrain->m_DestinationNodeID ],pTrain->m_DepartureTime ,pTrain->m_NodeSize,pTrain->m_PreferredArrivalTime,pTrain->m_ActualTripTime);

		for(int n = 0; n< pTrain->m_NodeSize; n++)
		{
			DTALink* pLink = pDoc->m_LinkMap[pTrain->m_aryTN[n].LinkID];

			if(n==0)
			{
			pTrain->m_aryTN[n].TaskProcessingTime = pTrain->m_DepartureTime;
			pTrain->m_aryTN[n].TaskScheduleWaitingTime = pTrain->m_aryTN[n].NodeTimestamp - pTrain->m_DepartureTime;

			}else
			{
			pTrain->m_aryTN[n].TaskProcessingTime = pLink->m_RuningTimeMap [pTrain->m_TrainType];
			pTrain->m_aryTN[n].TaskScheduleWaitingTime = pTrain->m_aryTN[n].NodeTimestamp - pTrain->m_aryTN[n-1].NodeTimestamp - pTrain->m_aryTN[n].TaskProcessingTime ;
			}

		fprintf(st,",,,,,,,,,%d,%d,%d,%d\n", pTrain->m_aryTN[n].TaskProcessingTime,pTrain->m_aryTN[n].TaskScheduleWaitingTime ,pTrain->m_aryTN[n].NodeTimestamp , pDoc->m_NodeIDtoNameMap[pTrain->m_aryTN[n].NodeID]);
		}
	}

		fclose(st);
		return true;
	}
	return false;
}
