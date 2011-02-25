// DlgMOE.cpp : implementation file
//
//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com)

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html

//    This file is part of NeXTA Version 3 (Open-source).

//    NEXTA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    NEXTA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with NEXTA.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "TLite.h"
#include "DlgMOE.h"
#include "Network.h"



extern CDlgMOE *g_LinkMOEDlg;
extern bool g_LinkMOEDlgShowFlag;
extern std::list<DTALink*>	g_LinkDisplayList;


void g_SelectColorCode(CDC* pDC, int ColorCount);

COLORREF s_ProjectColor=RGB(0,0,0);

CPen s_PenSimulationClock(PS_SOLID,2,RGB(0,255,0));

CPen s_PenHist(PS_SOLID,2,RGB(0,0,0));

CPen s_PenRealTimeProbe(PS_SOLID,2,RGB(0,0,255));
CBrush s_BrushRealTimeProbe(RGB(0,0,128));

CPen s_PenRealTimePrediction(PS_SOLID,3,RGB(255,0,255));

CPen s_PenRealTimeIrregularPrediction(PS_SOLID,1,RGB(255, 0, 255));


CPen s_PenTTI(PS_SOLID,0,RGB(0,0,255));
CBrush s_BrushTTI(RGB(0,255,0));

CPen s_PenTTI_Sensor(PS_SOLID,0,RGB(0,0,255));
CBrush s_BrushTTI_Sensor(RGB(0,0,255));


// CDlgMOE dialog

IMPLEMENT_DYNAMIC(CDlgMOE, CDialog)

CDlgMOE::~CDlgMOE()
{
}

void CDlgMOE::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgMOE, CDialog)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_COMMAND(ID_MOETYPE_LINKVOLUME, &CDlgMOE::OnMoetypeLinkvolume)
	ON_COMMAND(ID_MOETYPE_SPEED, &CDlgMOE::OnMoetypeSpeed)
	ON_COMMAND(ID_MOETYPE_CUMULATIVEVOLUME, &CDlgMOE::OnMoetypeCumulativevolume)
	ON_UPDATE_COMMAND_UI(ID_MOETYPE_LINKVOLUME, &CDlgMOE::OnUpdateMoetypeLinkvolume)
	ON_UPDATE_COMMAND_UI(ID_MOETYPE_SPEED, &CDlgMOE::OnUpdateMoetypeSpeed)
	ON_UPDATE_COMMAND_UI(ID_MOETYPE_CUMULATIVEVOLUME, &CDlgMOE::OnUpdateMoetypeCumulativevolume)
	ON_COMMAND(ID_MOETYPE2_LINKVOLUME, &CDlgMOE::OnMoetype2Linkvolume)
	ON_COMMAND(ID_MOETYPE2_SPEED, &CDlgMOE::OnMoetype2Speed)
	ON_COMMAND(ID_MOETYPE2_CUMULATIVEVOLUME, &CDlgMOE::OnMoetype2Cumulativevolume)
	ON_COMMAND(ID_MOETYPE2_NONE, &CDlgMOE::OnMoetype2None)
	ON_COMMAND(ID_DATA_EXPORT, &CDlgMOE::OnDataExport)
	ON_COMMAND(ID_MOETYPE1_LINKVOLUME, &CDlgMOE::OnMoetype1Linkvolume)
	ON_COMMAND(ID_MOETYPE1_SPEED, &CDlgMOE::OnMoetype1Speed)
	ON_COMMAND(ID_MOETYPE2_LINKVOLUME_VPH, &CDlgMOE::OnMoetype2LinkvolumeVph)
	ON_COMMAND(ID_MOETYPE2_SPEED_MPH, &CDlgMOE::OnMoetype2SpeedMph)
	ON_COMMAND(ID_VIEW_MOETIMESERIES, &CDlgMOE::OnViewMoetimeseries)
	ON_COMMAND(ID_VIEW_Q, &CDlgMOE::OnViewQ)
	ON_COMMAND(ID_MOETYPE3_Q, &CDlgMOE::OnMoetype3Q)
	ON_COMMAND(ID_MOETYPE2_DENSITY, &CDlgMOE::OnMoetype2Density)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_VIEW_RESETTIMERANGE, &CDlgMOE::OnViewResettimerange)
	ON_COMMAND(ID_ESTIMATION_HISTORICALAVGPATTERN, &CDlgMOE::OnEstimationHistoricalavgpattern)
	ON_UPDATE_COMMAND_UI(ID_ESTIMATION_HISTORICALAVGPATTERN, &CDlgMOE::OnUpdateEstimationHistoricalavgpattern)
	ON_COMMAND(ID_VIEW_MOEVARIABILITYPLOT, &CDlgMOE::OnViewMoevariabilityplot)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOEVARIABILITYPLOT, &CDlgMOE::OnUpdateViewMoevariabilityplot)
	ON_COMMAND(ID_MOETYPE1_TRAVELTIME, &CDlgMOE::OnMoetype1Traveltime)
	ON_COMMAND(ID_ESTIMATION_PREDICTION, &CDlgMOE::OnEstimationPrediction)
	ON_COMMAND(ID_MOETYPE3_NONE, &CDlgMOE::OnMoetype3None)
	ON_COMMAND(ID_MOETYPE1_DENSITY, &CDlgMOE::OnMoetype1Density)
	ON_UPDATE_COMMAND_UI(ID_MOETYPE1_DENSITY, &CDlgMOE::OnUpdateMoetype1Density)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_ESTIMATION_SHOWEVENTLABEL, &CDlgMOE::OnEstimationShoweventlabel)
	ON_UPDATE_COMMAND_UI(ID_ESTIMATION_SHOWEVENTLABEL, &CDlgMOE::OnUpdateEstimationShoweventlabel)
	ON_COMMAND(ID_DATA_EXPORT_WEATHER_DATA, &CDlgMOE::OnDataExportWeatherData)
	ON_COMMAND(ID_DATA_EXPORT_INCIDENT_DATA, &CDlgMOE::OnDataExportIncidentData)
	ON_COMMAND(ID_DATA_EXPORT_HIGH_DEMAND_DATA, &CDlgMOE::OnDataExportHighDemandData)
	ON_COMMAND(ID_DATA_EXPORT_SPECICAL_EVENT_DATA, &CDlgMOE::OnDataExportSpecicalEventData)
END_MESSAGE_MAP()


// CDlgMOE message handlers
void CDlgMOE::DrawTimeSeriesPlot()
{

	CPaintDC dc(this); // device context for painting
	CRect PlotRect;
	GetClientRect(PlotRect);

	CRect PlotRectOrg = PlotRect;

	if(m_TmLeft<0)
		m_TmLeft = 0;
	if(m_TmRight >=m_Range)
		m_TmRight=m_Range;
	if(m_TmRight< m_TmLeft+30)
		m_TmRight= m_TmLeft+30;


	if(Cur_MOE_type2== no_display)
	{
		PlotRect.top += 35;
		PlotRect.bottom -= 35;
		PlotRect.left += 60;
		PlotRect.right -= 100;

		DrawPlot(&dc,Cur_MOE_type1, PlotRect,true);


	}else
	{
		PlotRect.top += 20;
		PlotRect.bottom /=2;
		PlotRect.left += 60;
		PlotRect.right -= 100;

		DrawPlot(&dc, Cur_MOE_type1, PlotRect, true);

		PlotRect.top = PlotRectOrg.bottom /2+45;
		PlotRect.bottom = PlotRectOrg.bottom -20;
		PlotRect.left = PlotRectOrg.left+60;
		PlotRect.right = PlotRectOrg.right-100;

		DrawPlot(&dc,Cur_MOE_type2, PlotRect,false);


	}
}
void CDlgMOE::DrawQKCurve()
{
	CPaintDC dc(this); // device context for painting
	CRect PlotRect;
	GetClientRect(PlotRect);


	CRect PlotRectOrg = PlotRect;

	PlotRect.bottom = PlotRect.bottom/3;  // reserve space for QK curve

	if(Cur_MOE_type2==no_display)
	{
		PlotRect.top += 35;
		PlotRect.bottom = PlotRectOrg.bottom/2-35;
		PlotRect.left += 60;
		PlotRect.right -= 100;

		DrawPlot(&dc,Cur_MOE_type1, PlotRect,true);
	}else
	{
		PlotRect.top += 20;
		PlotRect.bottom = PlotRectOrg.bottom/3-35;;
		PlotRect.left += 60;
		PlotRect.right -= 100;

		DrawPlot(&dc, Cur_MOE_type1, PlotRect, true);

		PlotRect.top = PlotRectOrg.bottom/3+45;
		PlotRect.bottom = PlotRectOrg.bottom*2/3 -20;
		PlotRect.left = PlotRectOrg.left+60;
		PlotRect.right = PlotRectOrg.right-100;

		DrawPlot(&dc,Cur_MOE_type2, PlotRect,false);

	}

	// now draw q-k curve
	PlotRect.top = PlotRectOrg.bottom*2/3+20;
	PlotRect.bottom = PlotRectOrg.bottom-20;

	PlotRect.left = PlotRectOrg.left+60;
	PlotRect.right = PlotRectOrg.right/3;
	DrawSingleQKPlot(&dc,PlotRect);

	PlotRect.left = PlotRectOrg.right/3+40;
	PlotRect.right = PlotRectOrg.right*2/3-40;
	DrawSingleVKPlot(&dc,PlotRect);

	PlotRect.left = PlotRectOrg.right*2/3+40;
	PlotRect.right = PlotRectOrg.right-40;
	DrawSingleVQPlot(&dc,PlotRect);

}
void CDlgMOE::OnPaint()
{
	if(m_ViewMode == 0)
	{
		DrawTimeSeriesPlot();

	}

	if(m_ViewMode == 1)
	{
		DrawQKCurve();
	}
}


void CDlgMOE::DrawSingleQKPlot(CPaintDC* pDC, CRect PlotRect)
{

	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));

	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_DOT,0,RGB(0,0,0));

	pDC->TextOut(PlotRect.right/2,PlotRect.top-20,"Q-K Curve");

	pDC->SetBkMode(TRANSPARENT);

	m_YUpperBound = 2500;  // maximum lane flow

	float m_UnitDensity  = (float)(PlotRect.right - PlotRect.left)/(m_K_Jam);

	/// Y axis
	float YInterval = m_YUpperBound/10.0f;

	YInterval = (int)(YInterval)*1.0f;

	// data unit
	m_UnitData = 1;
	if((m_YUpperBound - m_YLowerBound)>0)
		m_UnitData = (float)(PlotRect.bottom - PlotRect.top)/(m_YUpperBound - m_YLowerBound);


	pDC->SelectObject(&TimePen);

	char buff[20];
	for(int i=0;i<=m_K_Jam;i+=50)
	{
		if(i == 0)
			pDC->SelectObject(&NormalPen);
		else
			pDC->SelectObject(&DataPen);

		int XPosition=(long)(PlotRect.left+(i)*m_UnitDensity);
		pDC->MoveTo(XPosition,PlotRect.bottom+2);
		pDC->LineTo(XPosition,PlotRect.top);

		wsprintf(buff,"%d",i  );

		pDC->TextOut(XPosition,PlotRect.bottom+3,buff);
	}


	int i;
	for(i=m_YLowerBound; i <= m_YUpperBound + YInterval-1; i+= YInterval)
	{
		if( i > m_YUpperBound)
			i = m_YUpperBound;

		if(i == m_YLowerBound)
			pDC->SelectObject(&NormalPen);
		else
			pDC->SelectObject(&DataPen);

		int TimeYPosition= PlotRect.bottom - (int)((i*m_UnitData)+0.50);

		pDC->MoveTo(PlotRect.left-2, TimeYPosition);
		pDC->LineTo(PlotRect.right,TimeYPosition);

		wsprintf(buff,"%d",i);
		pDC->TextOut(PlotRect.left-55,TimeYPosition-5,buff);
	}

	int LinkCount = 0;

	std::list<DTALink*>::iterator iLink;

	for (iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++,LinkCount++)
	{

		g_SelectColorCode(pDC,(*iLink)->m_DisplayLinkID);

		for(i=m_TmLeft;i<min((*iLink)->m_SimulationHorizon,m_TmRight);i+=1) // for each timestamp
		{

			int x=(long)(PlotRect.left+((*iLink)->m_LinkMOEAry[i].ObsDensity)*m_UnitDensity);

			int y= PlotRect.bottom - (int)(((*iLink)->m_LinkMOEAry[i].ObsFlow*m_UnitData));
			int size  =2;


			if((*iLink)->m_DisplayLinkID == 0)
				pDC->Ellipse(x-size,y-size,x+size,y+size);
			if((*iLink)->m_DisplayLinkID == 1)
				pDC->Rectangle(x-size,y-size,x+size,y+size);
			if((*iLink)->m_DisplayLinkID >= 2)
			{
				// cross
				pDC->MoveTo(x-size,y);
				pDC->LineTo(x+size,y);
				pDC->MoveTo(x,y-size);
				pDC->LineTo(x,y+size);
			}
		}
	}

}

void CDlgMOE::DrawSingleVKPlot(CPaintDC* pDC, CRect PlotRect)
{

	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));

	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_DOT,0,RGB(0,0,0));

	pDC->TextOut(PlotRect.right/2+60,PlotRect.top-20,"V-K Curve");

	pDC->SetBkMode(TRANSPARENT);

	m_YUpperBound = 100;  // maximum speed

	float m_UnitDensity  = (float)(PlotRect.right - PlotRect.left)/(300);

	/// Y axis
	float YInterval = m_YUpperBound/10.0f;

	YInterval = (int)(YInterval)*1.0f;

	// data unit
	m_UnitData = 1;
	if((m_YUpperBound - m_YLowerBound)>0)
		m_UnitData = (float)(PlotRect.bottom - PlotRect.top)/(m_YUpperBound - m_YLowerBound);


	pDC->SelectObject(&TimePen);

	char buff[20];
	for(int i=0;i<=m_K_Jam;i+=50)
	{
		if(i == 0)
			pDC->SelectObject(&NormalPen);
		else
			pDC->SelectObject(&DataPen);

		int XPosition=(long)(PlotRect.left+(i)*m_UnitDensity);
		pDC->MoveTo(XPosition,PlotRect.bottom+2);
		pDC->LineTo(XPosition,PlotRect.top);

		wsprintf(buff,"%d",i  );

		pDC->TextOut(XPosition,PlotRect.bottom+3,buff);
	}


	int i;
	for(i=m_YLowerBound; i <= m_YUpperBound + YInterval-1; i+= YInterval)
	{
		if( i > m_YUpperBound)
			i = m_YUpperBound;

		if(i == m_YLowerBound)
			pDC->SelectObject(&NormalPen);
		else
			pDC->SelectObject(&DataPen);

		int TimeYPosition= PlotRect.bottom - (int)((i*m_UnitData)+0.50);

		pDC->MoveTo(PlotRect.left-2, TimeYPosition);
		pDC->LineTo(PlotRect.right,TimeYPosition);

		wsprintf(buff,"%d",i);
		pDC->TextOut(PlotRect.left-40,TimeYPosition-5,buff);
	}

	int LinkCount = 0;

	std::list<DTALink*>::iterator iLink;

	for (iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++,LinkCount++)
	{

		g_SelectColorCode(pDC,(*iLink)->m_DisplayLinkID);

		for(i=m_TmLeft;i<min((*iLink)->m_SimulationHorizon , m_TmRight);i+=1) // for each timestamp
		{

			int x=(long)(PlotRect.left+((*iLink)->m_LinkMOEAry[i].ObsDensity)*m_UnitDensity);

			int y= PlotRect.bottom - (int)(((*iLink)->m_LinkMOEAry[i].ObsSpeed*m_UnitData));
			int size  =2;


			if((*iLink)->m_DisplayLinkID == 0)
				pDC->Ellipse(x-size,y-size,x+size,y+size);
			if((*iLink)->m_DisplayLinkID == 1)
				pDC->Rectangle(x-size,y-size,x+size,y+size);
			if((*iLink)->m_DisplayLinkID >= 2)
			{
				// cross
				pDC->MoveTo(x-size,y);
				pDC->LineTo(x+size,y);
				pDC->MoveTo(x,y-size);
				pDC->LineTo(x,y+size);
			}
		}
	}

}

void CDlgMOE::DrawSingleVQPlot(CPaintDC* pDC, CRect PlotRect)
{

	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));

	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_DOT,0,RGB(0,0,0));

	pDC->TextOut(PlotRect.right/2+60,PlotRect.top-20,"V-Q Curve");

	pDC->SetBkMode(TRANSPARENT);

	m_YUpperBound = 100;  // maximum speed

	float m_UnitFlow  = (float)(PlotRect.right - PlotRect.left)/(m_Max_Flow);

	/// Y axis
	float YInterval = m_YUpperBound/10.0f;

	YInterval = (int)(YInterval)*1.0f;

	// data unit
	m_UnitData = 1;
	if((m_YUpperBound - m_YLowerBound)>0)
		m_UnitData = (float)(PlotRect.bottom - PlotRect.top)/(m_YUpperBound - m_YLowerBound);


	pDC->SelectObject(&TimePen);

	char buff[20];
	for(int i=0;i<=m_Max_Flow;i+=500)
	{
		if(i == 0)
			pDC->SelectObject(&NormalPen);
		else
			pDC->SelectObject(&DataPen);

		int XPosition=(long)(PlotRect.left+(i)*m_UnitFlow);
		pDC->MoveTo(XPosition,PlotRect.bottom+2);
		pDC->LineTo(XPosition,PlotRect.top);

		wsprintf(buff,"%d",i  );

		pDC->TextOut(XPosition,PlotRect.bottom+3,buff);
	}


	int i;
	for(i=m_YLowerBound; i <= m_YUpperBound + YInterval-1; i+= YInterval)
	{
		if( i > m_YUpperBound)
			i = m_YUpperBound;

		if(i == m_YLowerBound)
			pDC->SelectObject(&NormalPen);
		else
			pDC->SelectObject(&DataPen);

		int TimeYPosition= PlotRect.bottom - (int)((i*m_UnitData)+0.50);

		pDC->MoveTo(PlotRect.left-2, TimeYPosition);
		pDC->LineTo(PlotRect.right,TimeYPosition);

		wsprintf(buff,"%d",i);
		pDC->TextOut(PlotRect.left-40,TimeYPosition-5,buff);
	}

	int LinkCount = 0;

	std::list<DTALink*>::iterator iLink;

	for (iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++,LinkCount++)
	{

		g_SelectColorCode(pDC,(*iLink)->m_DisplayLinkID);

		for(i=m_TmLeft;i<min((*iLink)->m_SimulationHorizon ,m_TmRight);i+=1) // for each timestamp
		{

			int x=(long)(PlotRect.left+((*iLink)->m_LinkMOEAry[i].ObsFlow)*m_UnitFlow);

			int y= PlotRect.bottom - (int)(((*iLink)->m_LinkMOEAry[i].ObsSpeed*m_UnitData));
			int size  =2;


			if((*iLink)->m_DisplayLinkID == 0)
				pDC->Ellipse(x-size,y-size,x+size,y+size);
			if((*iLink)->m_DisplayLinkID == 1)
				pDC->Rectangle(x-size,y-size,x+size,y+size);
			if((*iLink)->m_DisplayLinkID >= 2)
			{
				// cross
				pDC->MoveTo(x-size,y);
				pDC->LineTo(x+size,y);
				pDC->MoveTo(x,y-size);
				pDC->LineTo(x,y+size);
			}
		}
	}

}
void CDlgMOE::DrawPlot(CPaintDC* pDC,eLinkMOEMode  MOEType, CRect PlotRect, bool LinkTextFlag)
{
	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));
	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_DOT,0,RGB(0,0,0));

	pDC->SetBkMode(TRANSPARENT);

	m_YUpperBound = GetMaxYValue(MOEType);

	int TimeXPosition;

	int TimeInterval = g_FindClosestTimeResolution(m_TmRight - m_TmLeft);

	// time unit
	m_UnitTime = 1;
	if((m_TmRight - m_TmLeft)>0)
		m_UnitTime = (float)(PlotRect.right - PlotRect.left)/(m_TmRight - m_TmLeft);

	/// Y axis
	float YInterval = m_YUpperBound/10.0f;

	if(YInterval < 2)
		YInterval =  2;

	YInterval = (int)(YInterval)*1.0f;

	// data unit
	m_UnitData = 1;
	if((m_YUpperBound - m_YLowerBound)>0)
		m_UnitData = (float)(PlotRect.bottom - PlotRect.top)/(m_YUpperBound - m_YLowerBound);

	pDC->SelectObject(&TimePen);

	char buff[20];
	for(int i=m_TmLeft;i<m_TmRight;i+=TimeInterval)
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

				if(min_in_a_day==0 && i>=1440)
					wsprintf(buff,"%dd", i/1440+1 );
				else
					wsprintf(buff,"%dh",min_in_a_day/60 );

			}

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
	int i;


	YInterval = (int)(max(1,g_FindClosestYResolution(m_YUpperBound)));

	for(i=m_YLowerBound; i <= m_YUpperBound + YInterval-1; i+= YInterval)
	{
		if( i > m_YUpperBound)
			i = m_YUpperBound;

		if(i == m_YLowerBound)
			pDC->SelectObject(&NormalPen);
		else
			pDC->SelectObject(&DataPen);

		int TimeYPosition= PlotRect.bottom - (int)((i*m_UnitData)+0.50);

		pDC->MoveTo(PlotRect.left-2, TimeYPosition);
		pDC->LineTo(PlotRect.right,TimeYPosition);

		if(i <= m_YUpperBound)
		{
			wsprintf(buff,"%d",i);
			pDC->TextOut(PlotRect.left-55,TimeYPosition-5,buff);
		}
	}

	//Draw Time series
	DrawTimeSeries(MOEType, pDC, PlotRect,LinkTextFlag);

	if(m_bShowEventLabel)
		DrawEventCode(MOEType, pDC, PlotRect,LinkTextFlag);

}

void CDlgMOE::OnSize(UINT nType, int cx, int cy)
{
	RedrawWindow();

	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}


int CDlgMOE::GetMaxYValue(eLinkMOEMode MOEType)
{

	float YMax = 0;
	std::list<DTALink*>::iterator iLink;

	for (iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++)
	{
		float value = 50;

		int TimeRight = min( (*iLink)->m_SimulationHorizon,m_TmRight);
		m_TmLeft  = max(0,m_TmLeft);

		for(int i= m_TmLeft;i<TimeRight;i+=1) // for each timestamp
		{

			switch (MOEType)
			{

			case lane_volume: value= (*iLink)->GetObsLaneVolume (i); break;
			case speed_kmh: value= (*iLink)->GetObsSpeed (i)/0.621371192; break;
			case cummulative_volume: value= (*iLink)->GetObsCumulativeFlow(i); break;
			case link_volume: value= (*iLink)->GetObsLaneVolume (i)*(*iLink)->m_NumLanes; break;
			case speed_mph: value= (*iLink)->GetObsSpeed (i); break;
			case link_density: value= (*iLink)->GetObsDensity(i); break;
			case link_traveltime: value= (*iLink)->GetObsTravelTimeIndex(i); break;
			default: 0;
			}

			if(value > YMax)
				YMax = value;


		}
	}


	return max(10,int(YMax*10/9));
}

void CDlgMOE::DrawTimeSeries(eLinkMOEMode  MOEType , CPaintDC* pDC, CRect PlotRect,bool LinkTextFlag)
{
	int i;

	int Mod10 = 10;
	CString str_project;

	int LinkCount = 0;

	std::list<DTALink*>::iterator iLink;

	for (iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++,LinkCount++)
	{

		// draw time series
		bool b_ini_flag = false;

		g_SelectColorCode(pDC,(*iLink)->m_DisplayLinkID);

		// draw legend
		// PlotRect.right
		long TimeYPosition;


		CString str_MOE;
		switch (MOEType)
		{
		case lane_volume: str_MOE.Format ("Lane Volume: vphpl"); break;
		case speed_kmh: str_MOE.Format ("Speed: km/h"); break;
		case cummulative_volume: str_MOE.Format ("Cumulative Volume"); break;
		case link_volume: str_MOE.Format ("Link Volume: vph"); break;
		case speed_mph: str_MOE.Format ("Speed: mph"); break;
		case link_density: str_MOE.Format ("Density: vpmpl"); break;
		case link_traveltime:  str_MOE.Format ("Travel Time Index: (Base = 100)"); break;
		}
		pDC->TextOut(PlotRect.right/2,PlotRect.top-20,str_MOE);

		if(LinkTextFlag)
		{
			TimeYPosition= PlotRect.top + (int)(LinkCount*20);
			pDC->MoveTo(PlotRect.right,TimeYPosition);
			pDC->LineTo(PlotRect.right+30,TimeYPosition);

			CString str_link;
			str_link.Format ("[%d] %d->%d",(*iLink)->m_DisplayLinkID,(*iLink)->m_FromNodeNumber, (*iLink)->m_ToNodeNumber);
			pDC->TextOut(PlotRect.right,TimeYPosition,str_link);
		}

		long TimeXPosition=PlotRect.left;

		// draw normal MOE result
		b_ini_flag = false;

		for(i=m_TmLeft;i<m_TmRight;i+=1) // for each timestamp
		{

			TimeXPosition=(long)(PlotRect.left+(i-m_TmLeft)*m_UnitTime);


			float value = 0;

			switch (MOEType)
			{
			case lane_volume: value= (*iLink)->GetObsLaneVolume(i); break;
			case speed_kmh: value= (*iLink)->GetObsSpeed(i)/0.621371192; break;
			case cummulative_volume: value= (*iLink)->GetObsCumulativeFlow(i); break;
			case link_volume: value= (*iLink)->GetObsLaneVolume(i)*(*iLink)->m_NumLanes; break;
			case speed_mph: value= (*iLink)->GetObsSpeed(i); break;
			case link_density: value= (*iLink)->GetObsDensity(i); break;
			case link_traveltime: value= (*iLink)->GetObsTravelTimeIndex(i); break;

			default: value = 0;

			}

			// show both probe data and sensor data
			TimeYPosition= PlotRect.bottom - (int)((value*m_UnitData));

			if(!b_ini_flag)
			{
				pDC->MoveTo(TimeXPosition, TimeYPosition);
				b_ini_flag = true;
			}

			pDC->LineTo(TimeXPosition, TimeYPosition);

		}


		if(m_bShowPrediction)
		{
			pDC->SelectObject(&s_PenRealTimePrediction);
			b_ini_flag = false;

			for(i=g_Simulation_Time_Stamp;i<min(g_Simulation_Time_Stamp + 120,m_TmRight);i+=1) // for each timestamp
			{

				TimeXPosition=(long)(PlotRect.left+(i-m_TmLeft)*m_UnitTime);


				float value = 0;

				if(i<(*iLink)->m_SimulationHorizon )
				{

					struc_traffic_state state  = (*iLink)->GetPredictedState(g_Simulation_Time_Stamp, i-g_Simulation_Time_Stamp);


					switch (MOEType)
					{
					case speed_kmh: value= state.speed/0.621371192; break;
					case speed_mph: value= state.speed; break;
					case link_traveltime: value= state.traveltime;; break;

					default: value = 0;

					}
				}

				// show both probe data and sensor data
				TimeYPosition= PlotRect.bottom - (int)((value*m_UnitData));

				if(!b_ini_flag)
				{
					pDC->MoveTo(TimeXPosition, TimeYPosition);
					b_ini_flag = true;
				}

				pDC->LineTo(TimeXPosition, TimeYPosition);

			}
		}

		////////////
		// draw variability 

		if(m_bShowVariability)
		{
			for(i=0;i<g_Simulation_Time_Horizon;i+=1) // for each timestamp
			{

				TimeXPosition=(long)(PlotRect.left+(i%1440-m_TmLeft)*m_UnitTime);
				float value = 0;

				if(i<(*iLink)->m_SimulationHorizon)
				{

					switch (MOEType)
					{
					case lane_volume: value= (*iLink)->GetObsLaneVolume(i); break;
					case speed_kmh: value= (*iLink)->GetObsSpeed (i)/0.621371192; break;
					case cummulative_volume: value= (*iLink)->GetObsCumulativeFlow(i); break;
					case link_volume: value= (*iLink)->GetObsLaneVolume(i)*(*iLink)->m_NumLanes; break;
					case speed_mph: value= (*iLink)->GetObsSpeed (i); break;
					case link_density: value= (*iLink)->m_LinkMOEAry[i].ObsDensity; break;
					case link_traveltime: value= (*iLink)->GetObsTravelTimeIndex(i); break;

					default: value = 0;

					}
				}

				TimeYPosition= PlotRect.bottom - (int)((value*m_UnitData));

				if(i%1440==0)
				{
					pDC->MoveTo(TimeXPosition, TimeYPosition);
					b_ini_flag = true;
				}

				pDC->LineTo(TimeXPosition, TimeYPosition);

			}
			// draw historical MOE result
		}
		if(m_bShowHistPattern)
		{

			pDC->SelectObject (&s_PenHist);

			b_ini_flag = false;

			for(i=m_TmLeft;i<m_TmRight;i+=1) // for each timestamp
			{

				TimeXPosition=(long)(PlotRect.left+(i-m_TmLeft)*m_UnitTime);


				float value = 0;
				int time_of_day = i%1440;  

				if(i<(*iLink)->m_SimulationHorizon)
				{
					switch (MOEType)
					{
					case lane_volume: value= (*iLink)->m_HistLinkMOEAry [time_of_day].ObsFlow; break;
					case speed_kmh: value= (*iLink)->m_HistLinkMOEAry [time_of_day].ObsSpeed/0.621371192; break;
					case cummulative_volume: value= (*iLink)->m_HistLinkMOEAry [time_of_day].ObsCumulativeFlow; break;
					case link_volume: value= (*iLink)->m_HistLinkMOEAry [time_of_day].ObsFlow*(*iLink)->m_NumLanes; break;
					case speed_mph: value= (*iLink)->m_HistLinkMOEAry [time_of_day].ObsSpeed; break;
					case link_density: value= (*iLink)->m_HistLinkMOEAry [time_of_day].ObsDensity; break;
					case link_traveltime: value= (*iLink)->m_HistLinkMOEAry [time_of_day].ObsTravelTimeIndex; break;

					default: value = 0;

					}
				}

				// show both probe data and sensor data
				TimeYPosition= PlotRect.bottom - (int)((value*m_UnitData));

				if(!b_ini_flag)
				{
					pDC->MoveTo(TimeXPosition, TimeYPosition);
					b_ini_flag = true;
				}

				pDC->LineTo(TimeXPosition, TimeYPosition);

			}



		}
	}
}

void CDlgMOE::DrawEventCode(eLinkMOEMode  MOEType , CPaintDC* pDC, CRect PlotRect,bool TextFlag)
{
	int i;

	int LabelInterval = 5;
	int Mod10 = 10;
	CString str_project;

	pDC->SetTextColor(RGB(255,0,0));
	int LinkCount = 0;

	std::list<DTALink*>::iterator iLink;

	for (iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++,LinkCount++)
	{


		long TimeYPosition;
		long TimeXPosition;


		for(i=m_TmLeft;i<m_TmRight;i+=LabelInterval) // for each timestamp
		{

			TimeXPosition=(long)(PlotRect.left+(i-m_TmLeft)*m_UnitTime);

			float value = 0;

			switch (MOEType)
			{
			case lane_volume: value= (*iLink)->GetObsLaneVolume(i); break;
			case speed_kmh: value= (*iLink)->GetObsSpeed (i)/0.621371192; break;
			case cummulative_volume: value= (*iLink)->GetObsCumulativeFlow(i); break;
			case link_volume: value= (*iLink)->GetObsLaneVolume(i)*(*iLink)->m_NumLanes; break;
			case speed_mph: value= (*iLink)->GetObsSpeed (i); break;
			case link_density: value= (*iLink)->m_LinkMOEAry[i].ObsDensity; break;
			case link_traveltime: value= (*iLink)->GetObsTravelTimeIndex(i); break;

			default: value = 0;

			}

			// show both probe data and sensor data
			TimeYPosition= PlotRect.bottom - (int)((value*m_UnitData));

			switch ((*iLink)->GetEventCode(i))
			{
			case 1:

//				pDC->TextOut(TimeXPosition,TimeYPosition-5,"W");
				break;

			case 2: 
				pDC->TextOut(TimeXPosition,TimeYPosition-5,"D");
				break;

			case 3: 
				pDC->TextOut(TimeXPosition,TimeYPosition-5,"I");
				break;

			case 4: 
				pDC->TextOut(TimeXPosition,TimeYPosition-5,"S");
				break;
			}



		}


		// draw variability 

		if(m_bShowVariability)
		{
			for(i=0;i<g_Simulation_Time_Horizon;i+=LabelInterval) // for each timestamp
			{

				TimeXPosition=(long)(PlotRect.left+(i%1440-m_TmLeft)*m_UnitTime);

				float value = 0;
				int time_of_day = i%1440;  

				if(i<(*iLink)->m_SimulationHorizon)
				{
					switch (MOEType)
					{
					case lane_volume: value= (*iLink)->GetObsLaneVolume(i); break;
					case speed_kmh: value= (*iLink)->GetObsSpeed (i)/0.621371192; break;
					case cummulative_volume: value= (*iLink)->GetObsCumulativeFlow(i); break;
					case link_volume: value= (*iLink)->GetObsLaneVolume(i)*(*iLink)->m_NumLanes; break;
					case speed_mph: value= (*iLink)->GetObsSpeed (i); break;
					case link_density: value= (*iLink)->m_LinkMOEAry[i].ObsDensity; break;
					case link_traveltime: value= (*iLink)->GetObsTravelTimeIndex(i); break;

					default: value = 0;

					}
				}

				// show both probe data and sensor data
				TimeYPosition= PlotRect.bottom - (int)((value*m_UnitData));


				switch ((*iLink)->GetEventCode(i))
				{
				case 1:

//					pDC->TextOut(TimeXPosition,TimeYPosition-5,"W");
					break;

				case 2: 
					pDC->TextOut(TimeXPosition,TimeYPosition-5,"D");
					break;

				case 3: 
					pDC->TextOut(TimeXPosition,TimeYPosition-5,"I");
					break;

				case 4: 
					pDC->TextOut(TimeXPosition,TimeYPosition-5,"S");
					break;
				}

			}

		}	
	}
}
BOOL CDlgMOE::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgMOE::OnMoetypeLinkvolume()
{
	Cur_MOE_type1 = lane_volume;
	Invalidate();
}

void CDlgMOE::OnMoetypeSpeed()
{
	Cur_MOE_type1 = speed_kmh;
	SetWindowText("Speed km/h");
	Invalidate();}

void CDlgMOE::OnMoetypeCumulativevolume()
{
	Cur_MOE_type1 = cummulative_volume;
	SetWindowText("Cummulative Flow Count");
	Invalidate();
}

void CDlgMOE::OnUpdateMoetypeLinkvolume(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(Cur_MOE_type1 == lane_volume);

}

void CDlgMOE::OnUpdateMoetypeSpeed(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(Cur_MOE_type1 == speed_kmh);
}

void CDlgMOE::OnUpdateMoetypeCumulativevolume(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(Cur_MOE_type1 == cummulative_volume);
}

void CDlgMOE::OnMoetype2Linkvolume()
{
	Cur_MOE_type2 = lane_volume;
	Invalidate();
}

void CDlgMOE::OnMoetype2Speed()
{
	Cur_MOE_type2 = speed_kmh;
	Invalidate();
}

void CDlgMOE::OnMoetype2Cumulativevolume()
{
	Cur_MOE_type2 = cummulative_volume;
	Invalidate();
}

void CDlgMOE::OnMoetype2None()
{
	Cur_MOE_type2 = no_display;
	Invalidate();
}

void CDlgMOE::OnDataExport()
{
	ExportData(0);
}

void CDlgMOE::ExportData(int EventType)
{

	CString str;
	CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"(*.csv)|*.csv||", NULL);
	if(dlg.DoModal() == IDOK)
	{
		char fname[_MAX_PATH];
		wsprintf(fname,"%s", dlg.GetPathName());
		CWaitCursor wait;

		if(!ExportDataToCSVFile(fname,EventType))
		{
			str.Format("The file %s could not be opened.\nPlease check if it is opened by Excel.", fname);
			AfxMessageBox(str);
		}
	}
}

bool CDlgMOE::ExportDataToCSVFile(char csv_file[_MAX_PATH], int EventDataFlag)
{

	FILE* st;
	fopen_s(&st,csv_file,"w");

	if(st!=NULL)
	{
		fprintf(st, "Time,Day,Hour,Min");

		std::list<DTALink*>::iterator iLink;
		for (iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++)
		{
			fprintf(st, ",%d->%d, Event, Episode No, Duration, Lane Volumne(vphpl),Speed (mph),Density (vphpl), CumuFlow, TTI,",
				(*iLink)->m_FromNodeNumber ,(*iLink)->m_ToNodeNumber);

			fprintf(st, ",hist, Lane Volumne(vphpl),Speed (mph),Density (vphpl), CumuFlow, TTI Diff, Flow Diff"	);
		}
		fprintf(st, "\n");
		for(int i=m_TmLeft;i<m_TmRight;i+=5) // for each timestamp with real data
		{


			for (iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++)
			{
				if(EventDataFlag==0 || (EventDataFlag>=1 && EventDataFlag == (*iLink)->GetEventCode(i)))
					// all data or specific data type
				{
					int day, hour, min;
					day = i/1440;
					hour = (i-day*1440)/60;
					min =  i -day*1440- hour*60;
					fprintf(st, "%d,%d,%d,%d", i,day,hour,min);


					fprintf(st, ",,%d,%d,%d,%f,%f,%f,%f,%f", (*iLink)->GetEventCode(i), (*iLink)->m_LinkMOEAry[i].EpisodeNo , (*iLink)->m_LinkMOEAry[i].EpisoDuration ,
						(*iLink)->m_LinkMOEAry[i].ObsFlow,(*iLink)->m_LinkMOEAry[i].ObsSpeed,
						(*iLink)->m_LinkMOEAry[i].ObsDensity,(*iLink)->m_LinkMOEAry[i].ObsCumulativeFlow, (*iLink)->m_LinkMOEAry[i].ObsTravelTimeIndex );

					int time_of_day = i%1440;  

					fprintf(st, ",,%f,%f,%f,%f,%f,%f", (*iLink)->m_HistLinkMOEAry[time_of_day].ObsFlow,(*iLink)->m_HistLinkMOEAry[time_of_day].ObsSpeed,
						(*iLink)->m_HistLinkMOEAry[time_of_day].ObsDensity,(*iLink)->m_HistLinkMOEAry[time_of_day].ObsCumulativeFlow, ((*iLink)->m_LinkMOEAry[i].ObsTravelTimeIndex - (*iLink)->m_HistLinkMOEAry[time_of_day].ObsTravelTimeIndex), ((*iLink)->m_LinkMOEAry[i].ObsFlow- (*iLink)->m_HistLinkMOEAry[time_of_day].ObsFlow));

				}

				fprintf(st, "\n");

			}
		}

		fclose(st);
		return 1;
	}else
		return 0;

	return 0;
}

void CDlgMOE::OnMoetype1Linkvolume()
{
	Cur_MOE_type1 = link_volume;
	Invalidate();
}

void CDlgMOE::OnMoetype1Speed()
{
	Cur_MOE_type1 = speed_mph;
	Invalidate();
}

void CDlgMOE::OnMoetype2LinkvolumeVph()
{
	Cur_MOE_type2 = link_volume;
	Invalidate();
}

void CDlgMOE::OnMoetype2SpeedMph()
{
	Cur_MOE_type2 = speed_mph;
	Invalidate();
}

void CDlgMOE::OnViewMoetimeseries()
{
	m_ViewMode = no_display;
	Invalidate();
}

void CDlgMOE::OnViewQ()
{
	m_ViewMode = lane_volume;
	Invalidate();
}

void CDlgMOE::OnMoetype3Q()
{
	m_ViewMode = 1;
	Invalidate();
}

void CDlgMOE::OnMoetype2Density()
{
	Cur_MOE_type2 = link_density;
	Invalidate();
}

void CDlgMOE::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_last_cpoint = point;
	AfxGetApp()->LoadCursor(IDC_MOVENETWORK);
	m_bMoveDisplay = true;


	CDialog::OnLButtonDown(nFlags, point);
	Invalidate();
}

void CDlgMOE::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bMoveDisplay)
	{
		CSize OffSet = point - m_last_cpoint;
		if(OffSet.cx!=0)   // not change, do not reflesh the screen
		{
			int time_shift = OffSet.cx/m_UnitTime;
			m_TmLeft-= time_shift;
			m_TmRight-= time_shift;

			m_last_cpoint = point;

			Invalidate();
		}
	}
	CDialog::OnMouseMove(nFlags, point);

}

void CDlgMOE::OnLButtonUp(UINT nFlags, CPoint point)
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

	CDialog::OnLButtonUp(nFlags, point);
}

void CDlgMOE::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_TmLeft = 0;
	m_TmRight = g_Simulation_Time_Horizon;
	Invalidate();

	CDialog::OnRButtonUp(nFlags, point);
}

BOOL CDlgMOE::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
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

void CDlgMOE::OnViewResettimerange()
{
	m_TmLeft = 0;
	m_TmRight = g_Simulation_Time_Horizon;
	Invalidate();
}

void CDlgMOE::OnEstimationHistoricalavgpattern()
{
	m_bShowHistPattern = !m_bShowHistPattern;
	Invalidate();
}

void CDlgMOE::OnUpdateEstimationHistoricalavgpattern(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowHistPattern);
}

void CDlgMOE::OnViewMoevariabilityplot()
{
	m_bShowVariability = !m_bShowVariability;
	if(m_bShowVariability)
	{
		m_TmLeft = 0;
		m_TmRight = 1440;
	}else
	{
		m_TmLeft = 0;
		m_TmRight = m_Range;
	}
	Invalidate();

}

void CDlgMOE::OnUpdateViewMoevariabilityplot(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowVariability);
}

void CDlgMOE::OnMoetype1Traveltime()
{
	Cur_MOE_type1 = link_traveltime;
	Invalidate();
}

void CDlgMOE::OnEstimationPrediction()
{
	m_bShowPrediction = !m_bShowPrediction;
	Invalidate();
}

void CDlgMOE::OnMoetype3None()
{
	m_ViewMode = 0;
	Invalidate();
}

void CDlgMOE::OnMoetype1Density()
{
	Cur_MOE_type1 = link_density;
	Invalidate();
}

void CDlgMOE::OnUpdateMoetype1Density(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CDlgMOE::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu)
{
	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}
void CDlgMOE::OnEstimationShoweventlabel()
{
	m_bShowEventLabel = !m_bShowEventLabel;
	Invalidate();

}

void CDlgMOE::OnUpdateEstimationShoweventlabel(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowEventLabel);
}

void CDlgMOE::OnDataExportWeatherData()
{
	ExportData(1);
}

void CDlgMOE::OnDataExportIncidentData()
{
	ExportData(3);
}

void CDlgMOE::OnDataExportHighDemandData()
{
	ExportData(2);
}

void CDlgMOE::OnDataExportSpecicalEventData()
{
	ExportData(4);
}
