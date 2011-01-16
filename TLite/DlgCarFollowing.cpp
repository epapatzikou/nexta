// DlgCarFollowing.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgCarFollowing.h"


// CDlgCarFollowing dialog
extern CPen s_PenSimulationClock;
IMPLEMENT_DYNAMIC(CDlgCarFollowing, CDialog)

CDlgCarFollowing::CDlgCarFollowing(CWnd* pParent /*=NULL*/)
: CDialog(CDlgCarFollowing::IDD, pParent)
{
	m_MOEType = 0;
	m_bMoveDisplay = false;

	float simulation_horizon = 10;  // min
	m_NumberOfTimeSteps = 600*simulation_horizon;  // 0.1 second as resultion, for 10 min
//	m_NumberOfVehicles = m_NumberOfTimeSteps/20;  // 2 second per vehicle as max
	m_NumberOfVehicles = 20;  // 2 second per vehicle as max
	m_TmLeft = 0;
	m_Range = m_NumberOfTimeSteps;
	m_TmRight = m_NumberOfTimeSteps;  // use 5 days as starting show

	// size 1K*36K*4=144M
	m_VechileCFDataAry = NULL;

	m_SimulationTimeInterval = 0.1f;
	m_FreeflowSpeed = 30.0f;  //30 meter per second = 67.108088761632mph
	m_KJam = 140.0f; // 140 veh/km = 225.4/mile
	m_MeanWaveSpeed = 15.0f; // in km/h

	m_YUpperBound = 2000;  // 2KM
	m_YLowerBound = 0;
	m_bDataAvailable = false;

}
void CDlgCarFollowing::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCarFollowing, CDialog)
	ON_WM_PAINT()
	ON_COMMAND(ID_SIMULATION_RUN, &CDlgCarFollowing::OnSimulationRun)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


// CDlgCarFollowing message handlers
void CDlgCarFollowing::CarFollowingSimulation()
{
	m_VechileCFDataAry	= AllocateDynamicArray<VehicleCarFollowingData>(m_NumberOfVehicles,m_NumberOfTimeSteps);
	int t,v;
	for(v=0; v<m_NumberOfVehicles; v++)
	{
		for(t=0; t<m_NumberOfTimeSteps; t++)
		{
			m_VechileCFDataAry[v][t].Distance = 0;
			m_VechileCFDataAry[v][t].FreeflowSpeed  = m_FreeflowSpeed;

			if(v==0 && t>=200 )  //impose speed change on the first vehicle at 20 seconds
				m_VechileCFDataAry[v][t].FreeflowSpeed = m_FreeflowSpeed/2;

			m_VechileCFDataAry[v][t].Acceration  = 0;
		}
	}

	// arrival rate is 1000 vehicles per hour, there is a vehicle per 36000/1000 interval 
	float flow_rate_per_hour = 1000;
	float generation_time_interval = 36000/flow_rate_per_hour;


	m_VehicleDataVector.resize(m_NumberOfVehicles);
	// load vehicles
	// generate random arrival time
	// for given arrival time, speed = free flow speed;

	float starting_time = 0;
	for(v=0; v<m_NumberOfVehicles; v++)
	{
		m_VehicleDataVector[v].StartTime = (int)starting_time;
		starting_time += generation_time_interval;

		m_VehicleDataVector[v].EndTime = 0;

		float random_value = g_RNNOF()*sqrt(m_MeanWaveSpeed*0.6);
		m_VehicleDataVector[v].WaveSpeed = max(0.01,m_MeanWaveSpeed+random_value);
		TRACE("veh %d, wave speed %f\n",v, m_VehicleDataVector[v].WaveSpeed);
	}

	// updating 
	for(t=0; t<m_NumberOfTimeSteps; t++)
	{
		for(v=0; v<m_NumberOfVehicles; v++)
		{
			// for active vehicles (with positive speed or positive distance
			if(t>=m_VehicleDataVector[v].StartTime ) 
			{
				//calculate free-flow position
				//xiF(t) = xi(t-τ) + vf(τ)
				m_VechileCFDataAry[v][t].Distance = m_VechileCFDataAry[v][t-1].Distance +  m_VechileCFDataAry[v][t].FreeflowSpeed *m_SimulationTimeInterval;

				if(m_VechileCFDataAry[v][t].Distance >= m_YUpperBound && m_VehicleDataVector[v].EndTime ==0)
				{
				m_VehicleDataVector[v].EndTime = t;

				}

//				if(v==0)
//				TRACE("veh %d,time%d,%f\n",v,t,m_VechileCFDataAry[v][t].Distance);

				//calculate congested position
				if(v>=1)
				{
					//τ = 1/(wkj)
					// δ = 1/kj
					float time_tau_in_sec  = 3600.0f /(m_VehicleDataVector[v].WaveSpeed*m_KJam);  // 3600 seconds per hour, 
					float spacing = 1000.0f/m_KJam;   //1000 meters
					//xiC(t) = xi-1(t-τ) - δ
					int time_t_minus_tau = t+1 - int(time_tau_in_sec/m_SimulationTimeInterval+0.5); // need to convert time in second to time in simulation time interval

					if(time_t_minus_tau >=0)  // the leader has not reached destination yet
					{
					float CongestedDistance = m_VechileCFDataAry[v-1][time_t_minus_tau].Distance  - spacing;
					// xi(t) = min(xAF(t), xAC(t))
					if (m_VechileCFDataAry[v][t].Distance  > CongestedDistance)
						m_VechileCFDataAry[v][t].Distance = CongestedDistance;
					}

				}

			}  // for active vehicle
		} // for each vehicle
	}  // for each time

	m_bDataAvailable = true;
}




void CDlgCarFollowing::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect PlotRect;
	GetClientRect(PlotRect);

	CRect PlotRectOrg = PlotRect;

	if(m_TmLeft<0)
		m_TmLeft = 0;
	if(m_TmRight >=m_Range)
		m_TmRight=m_Range;
	if(m_TmRight< m_TmLeft+10)
		m_TmRight= m_TmLeft+10;

	PlotRect.top += 25;
	PlotRect.bottom -= 35;
	PlotRect.left += 60;
	PlotRect.right -= 40;

	DrawTimeSpaceGraph(&dc, m_MOEType, PlotRect,true);

}

void CDlgCarFollowing::DrawTimeSpaceGraph(CPaintDC* pDC,int MOEType, CRect PlotRect, bool LinkTextFlag)
{
	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));
	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_SOLID,0,RGB(0,0,0));

	pDC->SetBkMode(TRANSPARENT);


	m_UnitTime = 1;
	if((m_TmRight - m_TmLeft)>0)
		m_UnitTime = (float)(PlotRect.right - PlotRect.left)/(m_TmRight - m_TmLeft);

	m_UnitDistance = (float)(PlotRect.bottom  - PlotRect.top)/(m_YUpperBound - 	m_YLowerBound);


	pDC->SelectObject(&TimePen);

	int TimeInterval = (m_TmRight-m_TmLeft)/10; 
	int TimeXPosition;

	char buff[20];
	for(int i=m_TmLeft;i<=m_TmRight;i+=TimeInterval)
	{
		if(i == m_TmLeft)
		{
			pDC->SelectObject(&NormalPen);

			i = int((m_TmLeft/TimeInterval)+0.5)*TimeInterval; // reset time starting point
		}
		else
			pDC->SelectObject(&DataPen);

		TimeXPosition=(long)(PlotRect.left+(i-m_TmLeft)*m_UnitTime);

		if(i>= m_TmLeft)
		{
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

	pDC->SelectObject(&s_PenSimulationClock);
	if(g_Simulation_Time_Stamp >=m_TmLeft && g_Simulation_Time_Stamp <= m_TmRight )
	{
		TimeXPosition=(long)(PlotRect.left+(g_Simulation_Time_Stamp -m_TmLeft)*m_UnitTime);
		pDC->MoveTo(TimeXPosition,PlotRect.bottom+2);
		pDC->LineTo(TimeXPosition,PlotRect.top);
	}


	if(m_bDataAvailable)
	{
		pDC->SelectObject(&DataPen);
		int t,v;
		for(v=0; v<m_NumberOfVehicles; v++)
		{
			for(t=m_VehicleDataVector[v].StartTime; t<m_VehicleDataVector[v].EndTime; t+=10)
			{
				int x=(int)(PlotRect.left+(t-m_TmLeft)*m_UnitTime);
				int y = PlotRect.bottom - (int)((m_VechileCFDataAry[v][t].Distance *m_UnitDistance)+0.50);

					if(t==m_VehicleDataVector[v].StartTime)
					{
						pDC->MoveTo(x,y);

					}else
					{
						pDC->LineTo(x,y);

					}
				
			}
		}
	}



}


void CDlgCarFollowing::OnSimulationRun()
{
	CWaitCursor wait;
	CarFollowingSimulation();
	Invalidate();

}

void CDlgCarFollowing::OnSize(UINT nType, int cx, int cy)
{
	RedrawWindow();

	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}

void CDlgCarFollowing::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_last_cpoint = point;
	AfxGetApp()->LoadCursor(IDC_MOVENETWORK);
	m_bMoveDisplay = true;


	CDialog::OnLButtonDown(nFlags, point);
	Invalidate();
}

BOOL CDlgCarFollowing::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int CurrentTime = (int)((pt.x - 60.0f)/m_UnitTime + m_TmLeft);

	if(zDelta <	 0)
	{
		m_TmLeft-=(0.2*(CurrentTime-m_TmLeft));
		m_TmRight+=(0.2*(m_TmRight-CurrentTime));
	}
	else
	{
		m_TmLeft+=(0.2*(CurrentTime-m_TmLeft));
		m_TmRight-=(0.2*(m_TmRight-CurrentTime));
	}

	if(m_TmLeft < 0)
		m_TmLeft = 0;

	Invalidate();
	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CDlgCarFollowing::OnMouseMove(UINT nFlags, CPoint point)
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
	CDialog::OnMouseMove(nFlags, point);
}

void CDlgCarFollowing::OnLButtonUp(UINT nFlags, CPoint point)
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

	CDialog::OnLButtonUp(nFlags, point);}

void CDlgCarFollowing::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_TmLeft = 0;
	m_TmRight = 0;
	int v;

	for(v=0; v<m_NumberOfVehicles; v++)
	{
		if( m_TmRight< m_VehicleDataVector[v].EndTime)
			 m_TmRight = m_VehicleDataVector[v].EndTime ;
	}


	Invalidate();
	CDialog::OnRButtonDown(nFlags, point);
}
