// TSView.cpp : implementation of the CTimeSpaceView class
//

#include "stdafx.h"
#pragma warning(disable: 4786) // kill exceedingly long STL symbol warning
#include <set>
#include "TLite.h"
#include "TLiteDoc.h"
#include "Network.h"

#include "TSView.h"
#include <vector>
#include <algorithm>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern float g_Simulation_Time_Stamp;
extern CPen s_PenSimulationClock;
extern void g_SelectColorCode(CDC* pDC, int ColorCount);
extern void g_SelectSuperThickPenColor(CDC* pDC, int ColorCount);


CPen ThickPen(PS_SOLID,3,RGB(0,255,0));
CPen DoublePen(PS_SOLID,2,RGB(0,255,0));

CPen NormalPen(PS_SOLID,1,RGB(0,255,0));

CPen Normal2Pen(PS_SOLID,2,RGB(0,0,255));

CPen v_BluePen(PS_SOLID,1,RGB(0,0,255));
CPen v_BlackPen(PS_SOLID,1,RGB(0,0,0));
CPen v_RedPen(PS_SOLID,1,RGB(255,0,0));



CPen Pen(PS_SOLID,1,RGB(0,0,0));

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
	ON_WM_CLOSE()
	ON_COMMAND(ID_TOOLS_LOADVEHICLETRAJACTORYFILE, &CTimeSpaceView::OnToolsLoadvehicletrajactoryfile)
	ON_COMMAND(ID_NGSIMDATA_LANE1, &CTimeSpaceView::OnNgsimdataLane1)
	ON_COMMAND(ID_NGSIMDATA_LANE2, &CTimeSpaceView::OnNgsimdataLane2)
	ON_COMMAND(ID_NGSIMDATA_LANE3, &CTimeSpaceView::OnNgsimdataLane3)
	ON_COMMAND(ID_NGSIMDATA_LANE4, &CTimeSpaceView::OnNgsimdataLane4)
	ON_COMMAND(ID_NGSIMDATA_LANE5, &CTimeSpaceView::OnNgsimdataLane5)
	ON_COMMAND(ID_NGSIMDATA_LANE6, &CTimeSpaceView::OnNgsimdataLane6)
	ON_COMMAND(ID_NGSIMDATA_LANE7, &CTimeSpaceView::OnNgsimdataLane7)
	ON_COMMAND(ID_NGSIMDATA_COLORSPEED, &CTimeSpaceView::OnNgsimdataColorspeed)
	ON_UPDATE_COMMAND_UI(ID_NGSIMDATA_COLORSPEED, &CTimeSpaceView::OnUpdateNgsimdataColorspeed)
	ON_COMMAND(ID_NGSIMDATA_CARFOLLOWINGSIMULATION, &CTimeSpaceView::OnNgsimdataCarfollowingsimulation)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimeSpaceView construction/destruction
CTimeSpaceView::CTimeSpaceView()
{
	m_KJam = 220; // 220 veh/mile
	m_WaveSpeed = 12; // in mph 
	m_FreeflowSpeed = 60.0f;

	m_bShowSimulationDataFlag = false;
	m_bColorBySpeedFlag = false;
	m_SelectLaneID = 0;
	m_VehicleSnapshotAry = 0;
	m_NumberOfVehicles = 0;
	m_NumberOfTimeSteps = 0;

	m_bShowTimetable = true;

	m_bShowSlackTime = true;

	Cur_MOE_type1 = 0; 
	Cur_MOE_type2 = -1;

	m_NumberOfTimeSteps = 1440;
	m_TmLeft = 0;
	m_TmRight = m_NumberOfTimeSteps; 
	m_YLowerBound = 0;

	m_bShowResourcePrice = false;
	m_bShowNodeCostLabel = false;

	m_bMoveDisplay = false;

	bRangeInitialized = false;

	for(int LaneID = 0; LaneID < 7; LaneID++)
		VehicleSizeLane[LaneID] = 0;

}

CTimeSpaceView::~CTimeSpaceView()
{
	if(m_NumberOfVehicles>0)
	{
		DeallocateDynamicArray<VehicleSnapshotData>(m_VehicleSnapshotAry,m_NumberOfVehicles,m_NumberOfTimeSteps+1);
		m_VehicleDataList.clear ();
	}

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

	//	DrawNGSIMObjects(pDC,Cur_MOE_type1, PlotRect);

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

	DrawNGSIMObjects(&memDC,Cur_MOE_type1,PlotRect);

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

void CTimeSpaceView::DrawNGSIMObjects(CDC* pDC,int MOEType,CRect PlotRect)
{
	if(!bRangeInitialized)
	{
		InitializeTimeRange();
		bRangeInitialized = true;
	}

	if(m_TmLeft<0)
		m_TmLeft = 0;

	if(m_TmRight > m_NumberOfTimeSteps)
		m_TmRight = m_NumberOfTimeSteps;

	CPen DataPen(PS_SOLID,1,RGB(0,0,0));

	CPen TimePen(PS_DOT,1,RGB(0,255,0));

	CString str_MOE;

	str_MOE.Format ("Time-Space Graph: Lane %d", m_SelectLaneID);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&NormalPen);

	pDC->TextOut(PlotRect.right/2-100,PlotRect.top-20,str_MOE);

	int i;
	// step 2: calculate m_UnitDistance;
	// data unit
	m_UnitDistance = 1;
	if((m_YUpperBound - m_YLowerBound)>0)
		m_UnitDistance = (float)(PlotRect.bottom - PlotRect.top)/(m_YUpperBound - m_YLowerBound);


	// step 3: time interval
	int TimeXPosition;

	int TimeInterval = FindClosestTimeResolution(m_TmRight - m_TmLeft);

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

			int min, sec;
			min = i/600;
			sec =  (i- min*600)/10;
			wsprintf(buff,"%2d:%02d",min, sec);
			pDC->TextOut(TimeXPosition,PlotRect.bottom+3,buff);
		}
	}

	// 	step 4: draw time axis

	pDC->SelectObject(&NormalPen);

	pDC->MoveTo(PlotRect.left, PlotRect.top);
	pDC->LineTo(PlotRect.left,PlotRect.bottom);

	pDC->MoveTo(PlotRect.right, PlotRect.top);
	pDC->LineTo(PlotRect.right,PlotRect.bottom);

	// draw vehicles

	int display_resolution = 1;
	display_resolution = max(1,(m_TmRight - m_TmLeft)/2000.0f);

	if(m_bShowSimulationDataFlag == false)
	{ // show original data

		for(int v=0; v<m_NumberOfVehicles; v+=1)
		{
			float StartTime = m_VehicleDataList[v].StartTime;

			if(StartTime < m_TmLeft)
				StartTime = m_TmLeft;

			float EndTime = m_VehicleDataList[v].EndTime;

			if(EndTime > m_TmRight)
				EndTime = m_TmRight;

			bool bIniFlag  = false;
			if(StartTime< EndTime)
			{
				for(int t= StartTime; t<=EndTime; t+=display_resolution)  
				{

					if(m_VehicleSnapshotAry[v][t].LaneID ==m_SelectLaneID)
					{

						if(!m_bColorBySpeedFlag)
						{
							switch(m_VehicleDataList[v].VehicleType)
							{

							case 1: pDC->SelectObject(&v_BluePen); break;
							case 2: pDC->SelectObject(&v_BlackPen); break;
							case 3: pDC->SelectObject(&v_RedPen); break;
							default:
								pDC->SelectObject(&v_BlackPen);
							}

						}else
						{
							CPen penmoe;

							float power;

							float speed = max(1,m_VehicleSnapshotAry[v][t].Speed_mph);
							power= min(1.0f, 55/speed);

							float n= power*100;
							int R=(int)((255*n)/100);
							int G=(int)(255*(100-n)/100); 
							int B=0;

							penmoe.CreatePen (PS_SOLID, 1, RGB(R,G,B));
							pDC->SelectObject(&penmoe);

						}


						int x=(int)(PlotRect.left+(t - m_TmLeft)*m_UnitTime);
						int y = PlotRect.bottom - (int)(((m_VehicleSnapshotAry[v][t].LocalY-m_YLowerBound)*m_UnitDistance)+0.50);


						if(!bIniFlag)
						{
							pDC->MoveTo(x,y);
							bIniFlag = true;

						}else
						{
							pDC->LineTo(x,y);

						}

					}

				}
			}
		}

	}else
	{
		// show simulation data for m_SelectLaneID


		for(int v=0; v<VehicleSizeLane[m_SelectLaneID]; v+=1)
		{
			bool bIniFlag  = false;
			for(int t= StartTimeLane[m_SelectLaneID]; t<=EndTimeLane[m_SelectLaneID]; t+=display_resolution)  
			{
				pDC->SelectObject(&v_BlackPen);

				int x=(int)(PlotRect.left+(t - m_TmLeft)*m_UnitTime);
				int y = PlotRect.bottom - (int)(((m_VehicleSnapshotAry_Lane[m_SelectLaneID][v][t]-m_YLowerBound)*m_UnitDistance)+0.50);


				if(!bIniFlag)
				{
					pDC->MoveTo(x,y);
					bIniFlag = true;

				}else
				{
					pDC->LineTo(x,y);

				}

			}

		}
	}

}

void CTimeSpaceView::DrawObjects(CDC* pDC,int MOEType,CRect PlotRect)
{
	/* to be restored.
	if(!bRangeInitialized)
	{
	InitializeTimeRange();
	bRangeInitialized = true;
	}

	if(m_TmLeft<0)
	m_TmLeft = 0;

	if(m_TmRight > 1400)
	m_TmRight = 1400;

	CPen DataPen(PS_SOLID,1,RGB(0,0,0));

	CPen TimePen(PS_DOT,1,RGB(0,255,0));

	CString str_MOE;

	str_MOE.Format ("Time-Space Graph");

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&NormalPen);

	pDC->TextOut(PlotRect.right/2-100,PlotRect.top-20,str_MOE);

	m_UnitDistance = 1;
	if((m_YUpperBound - m_YLowerBound)>0)
	m_UnitDistance = (float)(PlotRect.bottom - PlotRect.top)/(m_YUpperBound - m_YLowerBound);


	// step 3: time interval
	int TimeXPosition;

	int TimeInterval = g_FindClosestTimeResolution(m_TmRight - m_TmLeft);

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


	int PrevToNodeNumber = -1;
	int bSpace = -1;

	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{

	int YFrom = PlotRect.bottom - (int)(((*iLink)->m_FromNodeY*m_UnitDistance)+0.50);
	int YTo= PlotRect.bottom - (int)(((*iLink)->m_ToNodeY*m_UnitDistance)+0.50);


	pDC->MoveTo(PlotRect.left, YFrom);
	pDC->LineTo(PlotRect.right,YFrom);

	pDC->MoveTo(PlotRect.left, YTo);
	pDC->LineTo(PlotRect.right,YTo);

	if(PrevToNodeNumber!=(*iLink)->m_FromNodeNumber)  // first from node or node change
	{
	bSpace++;
	if(bSpace==5)
	bSpace = 0;

	wsprintf(buff,"%d",(*iLink)->m_FromNodeNumber );
	pDC->TextOut(PlotRect.left-60+bSpace*10,YFrom-5,buff);
	}

	wsprintf(buff,"%d",(*iLink)->m_ToNodeNumber );
	pDC->TextOut(PlotRect.left-60+bSpace*10,YTo-5,buff);

	PrevToNodeNumber = (*iLink)->m_ToNodeNumber;
	}

	// draw trains
	for(unsigned int v = 0; v<pDoc->m_TrainVector.size(); v++)
	{

	DTA_Train* pTrain = pDoc->m_TrainVector[v];

	g_SelectColorCode(pDC, pTrain->m_TrainType-1);

	if(pTrain->m_TrainID == pDoc->m_SelectedTrainID )
	g_SelectSuperThickPenColor(pDC, pTrain->m_TrainType-1);


	for(int n = 1; n< pTrain->m_NodeSize; n++)
	{
	DTALink* pLink = pDoc->m_LinkIDMap[pTrain->m_aryTN[n].RailLinkID];

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
	*/
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
	if(m_TmRight>m_NumberOfTimeSteps) m_TmRight = m_NumberOfTimeSteps;

	Invalidate();
	return true;

}

void CTimeSpaceView::InitializeTimeRange()
{
	int min_timestamp = m_NumberOfTimeSteps;
	int max_timestamp = 0;

	CTLiteDoc* pDoc = GetTLDocument();

	if(pDoc->m_TrainVector.size() ==0)
	{
		m_TmLeft = 0;
		m_TmRight = m_NumberOfTimeSteps;

		return;
	}
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

}

void CTimeSpaceView::OnRButtonDown(UINT nFlags, CPoint point)
{

	InitializeTimeRange();

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
		int time_shift = max(1,OffSet.cx/5);
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

void CTimeSpaceView::OnClose()
{
	CView::OnClose();
}



void CTimeSpaceView::OnToolsLoadvehicletrajactoryfile()
{
	int Vehicle_ID; //1 
	int Frame_ID;//2

	int Total_Frames; //3
	int Global_Time ; //4

	float Local_X ; //5
	float Local_Y  ; //6
	float Global_X  ; //7
	float Global_Y  ; //8
	float Vehicle_Length ; //9
	float Vehicle_Width ; //10

	int	Vehicle_Class ; //11

	float	Vehicle_Velocity ; //12
	float	Vehicle_Acceleration  ; //13

	int 	Lane_Identification ; //14
	int	Preceding_Vehicle ; //15
	int	Following_Vehicle  ; //16

	float	Spacing ; //17
	float	Headway  ; //18

	CString str;
	CFileDialog dlg (FALSE, "*.txt", "*.txt",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"(*.txt)|*.txt||", NULL);
	if(dlg.DoModal() == IDOK)
	{


		CTime ExeStartTime = CTime::GetCurrentTime();

		CWaitCursor wc;
		char fname[_MAX_PATH];
		wsprintf(fname,"%s", dlg.GetPathName());
		FILE* st;
		fopen_s(&st,fname,"r");
		int line = 0;

		if(st!=NULL)
		{

			m_NumberOfVehicles = 0;
			m_NumberOfTimeSteps = 0;

			int Prev_Vehicle_ID = -1;
			while(!feof(st))
			{	
				int Vehicle_ID = g_read_integer(st);  //1
				if(Vehicle_ID < 0)
					break;

				if(Vehicle_ID != Prev_Vehicle_ID)
				{
					m_NumberOfVehicles++;
					Prev_Vehicle_ID = Vehicle_ID;

				}

				int Frame_ID = g_read_integer(st); //2

				if(Frame_ID > m_NumberOfTimeSteps)
					m_NumberOfTimeSteps = Frame_ID;

				int Total_Frames = g_read_integer(st); //3
				int Global_Time = g_read_integer(st); //4

				float Local_X = g_read_float(st); //5
				float Local_Y = g_read_float(st); //6
				float Global_X = g_read_float(st); //7
				float Global_Y = g_read_float(st); //8
				float Vehicle_Length = g_read_float(st); //9
				float Vehicle_Width = g_read_float(st); //10

				int	Vehicle_Class = g_read_integer(st); //11

				float	Vehicle_Velocity = g_read_float(st); //12
				float	Vehicle_Acceleration  = g_read_float(st); //13

				int 	Lane_Identification = g_read_integer(st); //14
				int	Preceding_Vehicle = g_read_integer(st); //15
				int	Following_Vehicle = g_read_integer(st); //16

				float	Spacing =  g_read_float(st); //17
				float	Headway =  g_read_float(st); //18

			}

		}
		fclose(st);

		// Reopen the file:
		m_VehicleSnapshotAry	= AllocateDynamicArray<VehicleSnapshotData>(m_NumberOfVehicles,m_NumberOfTimeSteps+1);

		m_YUpperBound = 0;
		m_YLowerBound = 9999999;

		m_TmLeft = 0;
		m_TmRight = m_NumberOfTimeSteps;


		fopen_s(&st,fname,"r");
		line = 0;

		if(st!=NULL)
		{

			m_NumberOfVehicles = 0;
			m_NumberOfTimeSteps = 0;

			int Prev_Vehicle_ID = -1;
			while(!feof(st))
			{	
				VehicleCFData vcd;

				Vehicle_ID = g_read_integer(st);  //1
				if(Vehicle_ID < 0)
					break;

				Frame_ID = g_read_integer(st); //2

				if(Frame_ID > m_NumberOfTimeSteps)
					m_NumberOfTimeSteps = Frame_ID;

				Total_Frames = g_read_integer(st); //3
				Global_Time = g_read_integer(st); //4

				Local_X = g_read_float(st); //5
				Local_Y = g_read_float(st); //6

				if(Local_Y > m_YUpperBound)
					m_YUpperBound = Local_Y;

				if(Local_Y < m_YLowerBound)
					m_YLowerBound = Local_Y;

				Global_X = g_read_float(st); //7
				Global_Y = g_read_float(st); //8
				Vehicle_Length = g_read_float(st); //9
				Vehicle_Width = g_read_float(st); //10

				Vehicle_Class = g_read_integer(st); //11

				Vehicle_Velocity = g_read_float(st); //12
				Vehicle_Acceleration  = g_read_float(st); //13

				Lane_Identification = g_read_integer(st); //14
				Preceding_Vehicle = g_read_integer(st); //15
				Following_Vehicle = g_read_integer(st); //16

				Spacing =  g_read_float(st); //17
				Headway =  g_read_float(st); //18

				if(Vehicle_ID != Prev_Vehicle_ID)
				{

					m_NumberOfVehicles++;

					m_VehicleIDtoNOMap[Vehicle_ID] = m_NumberOfVehicles;  // ID to sequential No.

					Prev_Vehicle_ID = Vehicle_ID;


					vcd.VehicleID = Vehicle_ID;

					vcd.StartTime = Frame_ID;
					vcd.EndTime = Frame_ID + Total_Frames;
					vcd.VehicleType = Vehicle_Class;

					m_VehicleDataList.push_back (vcd);

				}
				m_VehicleSnapshotAry [m_NumberOfVehicles-1] [Frame_ID].LocalY  =  Local_Y;  //m_NumberOfVehicles-1 make sure the index starting from 0
				m_VehicleSnapshotAry [m_NumberOfVehicles-1] [Frame_ID].LaneID  =  Lane_Identification; 
				m_VehicleSnapshotAry [m_NumberOfVehicles-1] [Frame_ID].Speed_mph  = Vehicle_Velocity * 0.681818182;  // convert from feet per second to mph

				line++;
			}

		}
		fclose(st);

		m_SelectLaneID  = 1;

		/// loading statistics
		CTime ExeEndTime = CTime::GetCurrentTime();

		CTimeSpan ts = ExeEndTime  - ExeStartTime;
		CString str_running_time;

		str_running_time.Format ("Load %d vehicles and %d lines from the vehicle trajectory file.\nLoading Time: %d hour %d min %d sec. \n",
			m_NumberOfVehicles, line, ts.GetHours(), ts.GetMinutes(), ts.GetSeconds());

		AfxMessageBox(str_running_time, MB_ICONINFORMATION);
		Invalidate();
	}
}
void CTimeSpaceView::OnNgsimdataLane1()
{
	m_SelectLaneID  = 1;
	Invalidate();
}

void CTimeSpaceView::OnNgsimdataLane2()
{
	m_SelectLaneID  = 2;
	Invalidate();
}

void CTimeSpaceView::OnNgsimdataLane3()
{
	m_SelectLaneID  = 3;
	Invalidate();
}

void CTimeSpaceView::OnNgsimdataLane4()
{
	m_SelectLaneID  = 4;
	Invalidate();
}

void CTimeSpaceView::OnNgsimdataLane5()
{
	m_SelectLaneID  = 5;
	Invalidate();
}

void CTimeSpaceView::OnNgsimdataLane6()
{
	m_SelectLaneID  = 6;
	Invalidate();
}

void CTimeSpaceView::OnNgsimdataLane7()
{
	m_SelectLaneID  = 7;
	Invalidate();
}

void CTimeSpaceView::OnNgsimdataColorspeed()
{
	m_bColorBySpeedFlag = !m_bColorBySpeedFlag;
	Invalidate();
}

void CTimeSpaceView::OnUpdateNgsimdataColorspeed(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bColorBySpeedFlag);
}

bool compare_timestamp (int first, int second)
{
	if(first < second) return true;
	else 
		return false;
}

void CTimeSpaceView::OnNgsimdataCarfollowingsimulation()
{
	// for each lane
	// find out the first vehicle on this lane
	// record the trajectory as the lead vehicle as the boundary condition
	// record the upstream and downstream sensor location: +- 50 feet 
	// count vehicles passing through sensors, build the arrival and departure timestamps
	// create new vehicle car following data array
	// simulation
	// display

	int m_NumberOfVehicles_lane  = m_NumberOfTimeSteps / 20;  // 2 second per vehicle at most
	int number_of_lanes  = 2;

	int t;
	m_VehicleSnapshotAry_Lane = Allocate3DDynamicArray<float>(number_of_lanes,m_NumberOfVehicles_lane,m_NumberOfTimeSteps+1);

	for(int LaneID=0; LaneID<number_of_lanes; LaneID++)
	{
		// find out the first vehicle on this lane

		int v;
		float FirstTimeStamp  = 999999;
		int FirstVehicleID = 0;
		for( v=0; v<m_NumberOfVehicles; v+=1)
		{
			for(t= m_VehicleDataList[v].StartTime; t<=m_VehicleDataList[v].EndTime; t+=1)  
			{

				if(m_VehicleSnapshotAry[v][t].LaneID ==LaneID+1)  //LaneID starts from 0
				{
					if(t < FirstTimeStamp)
					{
						FirstTimeStamp = t;
						FirstVehicleID = v;

					}
				}

			}
		}

		// record vehicle trajectory

		// for leader vehicle, initialize
		StartTimeLane[LaneID] = -1; 
		EndTimeLane[LaneID] = -1;


		for(t= m_VehicleDataList[FirstVehicleID].StartTime; t<=m_VehicleDataList[FirstVehicleID].EndTime; t+=1)  
		{

			if(StartTimeLane[LaneID] != -1 && m_VehicleSnapshotAry[FirstVehicleID][t].LaneID !=LaneID+1)
			{
				break; // switch to another link!
			}

			if(m_VehicleSnapshotAry[FirstVehicleID][t].LaneID ==LaneID+1)  //LaneID starts from 0
			{

				if(StartTimeLane[LaneID] == -1) // has not been initialized
				{
					StartTimeLane[LaneID] = t;
					StartLocalYLane[LaneID] = m_VehicleSnapshotAry[FirstVehicleID][t].LocalY;
				}

				EndTimeLane[LaneID] = t;  // keep assinging time stamp value
				EndLocalYLane[LaneID] = m_VehicleSnapshotAry[FirstVehicleID][t].LocalY;

				// record trajactory for the first vehicle
				m_VehicleSnapshotAry_Lane[LaneID][0][t] = m_VehicleSnapshotAry[FirstVehicleID][t].LocalY ; 
				// simulated data array (lane by lane)     original data array

			}
		}

		// set up the position for upstream and downstream (single) loop detectors, can be extended to double loops
		StartLocalYLane[LaneID] += 50.0f; // move up by 50 feet
		EndLocalYLane[LaneID] -= 50.0f; // move down by 50 feet


		// find passing time stamps of vehicles

		std::vector<int> StartPassingTimeStamp; 
		std::vector<int> EndPassingTimeStamp;

		for( v=0; v<m_NumberOfVehicles; v+=1)
		{
			for(t= m_VehicleDataList[v].StartTime; t<m_VehicleDataList[v].EndTime; t+=1)  
			{
				if(m_VehicleSnapshotAry[v][t].LaneID ==LaneID+1)  //LaneID starts from 0
				{

					if(m_VehicleSnapshotAry[v][t].LocalY <= StartLocalYLane[LaneID] && StartLocalYLane [LaneID]< m_VehicleSnapshotAry[v][t+1].LocalY )
					{
						StartPassingTimeStamp.push_back(t);
					}

					if(m_VehicleSnapshotAry[v][t].LocalY <= EndLocalYLane [LaneID] && EndLocalYLane [LaneID] < m_VehicleSnapshotAry[v][t+1].LocalY )
					{
						EndPassingTimeStamp.push_back(t);
					}

				}

			}
		}

		std::sort(StartPassingTimeStamp.begin(), StartPassingTimeStamp.end(), compare_timestamp);
		std::sort(EndPassingTimeStamp.begin(), EndPassingTimeStamp.end(), compare_timestamp);

		// here we assume the first leader vehicle is 0

		VehicleSizeLane[LaneID] = min(StartPassingTimeStamp.size (),EndPassingTimeStamp.size());
		for (v = 0; v < VehicleSizeLane[LaneID]; v++)  // 1 starts from the first follower.
		{
			TRACE("no.%d, %d, %d\n", StartPassingTimeStamp[v], EndPassingTimeStamp[v]); 
		}	

		// construct cumuluative count,
		// apply stochatic three detector theorem. 
		// reconstruct arrival and departure time sequences
		// run traffic simulation


		float m_FreeflowMoveDistance = m_FreeflowSpeed *5280 / 36000;  // 60 mph as free-flow speed, 5280 feet per mile, 3600 second with 0.1 seconds as time step

		for (v = 1; v < VehicleSizeLane[LaneID]; v++)  // start from vehicle 1 as the first follower
		{
			for(t= StartPassingTimeStamp[v]; t<= EndPassingTimeStamp[v]; t++) 
			{

				//calculate free-flow position
				//xiF(t) = xi(t-τ) + vf(τ)

				if(t==StartPassingTimeStamp[v] )
				{
					m_VehicleSnapshotAry_Lane[LaneID][v][t] =  StartLocalYLane[LaneID];  // assign starting position
					continue;
				}
				else
				{
					m_VehicleSnapshotAry_Lane[LaneID][v][t] = m_VehicleSnapshotAry_Lane[LaneID][v][t-1] +  m_FreeflowMoveDistance;
				}

				// if this vehicle exceeds the downstream sensor location, needs to stop (wait!)
				if( m_VehicleSnapshotAry_Lane[LaneID][v][t] > EndLocalYLane [LaneID] )
					m_VehicleSnapshotAry_Lane[LaneID][v][t] = EndLocalYLane [LaneID] ;

				// calculate congested position

				//τ = 1/(wkj)
				// δ = 1/kj
				float time_tau_in_time_step  = 1 /(m_WaveSpeed*m_KJam)*36000; // to 0.1 second  
				float spacing = 1.0f/m_KJam*5280;   // to feet
				//xiC(t) = xi-1(t-τ) - δ
				int time_t_minus_tau = t - int(time_tau_in_time_step); // need to convert time in second to time in simulation time interval

				if(time_t_minus_tau >=StartPassingTimeStamp[v-1])  // the leader has not reached destination yet
				{
					float CongestedDistance = max(StartLocalYLane[LaneID], m_VehicleSnapshotAry_Lane[LaneID][v-1][time_t_minus_tau]  - spacing); 
					// max (upsteram sensor location,) makes sure the congested distance is at least upsteram sensor location.
					// v-1 is the leader of the current vehicle, time_t_minus_tau is the lagged timestamp
					// xi(t) = min(xAF(t), xAC(t))
					if (m_VehicleSnapshotAry_Lane[LaneID][v][t]  > CongestedDistance && CongestedDistance >= m_VehicleSnapshotAry_Lane[LaneID][v][t-1])
						m_VehicleSnapshotAry_Lane[LaneID][v][t] = CongestedDistance;


				}

			}
		}
	}
	m_bShowSimulationDataFlag = true;

}

void  CTimeSpaceView::UpdateVehicleStartANDEndTimes(std::vector<int> &StartPassingTimeStamp, std::vector<int> &EndPassingTimeStamp)
{
		// construct cumuluative count, A, D. 
		// apply stochatic three detector theorem  to get A' and D'
		// apply deterministic detector theorem again to get the perfect A and D
		// reconstruct arrival and departure time sequences

}
