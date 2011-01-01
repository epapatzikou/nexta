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

#pragma once


// CDlgMOE dialog

class CDlgMOE : public CDialog
{
	DECLARE_DYNAMIC(CDlgMOE)

public:

	CDlgMOE::CDlgMOE(CWnd* pParent =NULL)
	: CDialog(CDlgMOE::IDD, pParent)
{
	m_YUpperBound = 100;
	m_YLowerBound = 0;
	m_TmLeft = 0;
	m_Range = 1440*g_Number_of_Weekdays;
	m_TmRight = m_Range;  // use 5 days as starting show
	Cur_MOE_type1 = 0;
	Cur_MOE_type2 = -1;

	m_bMoveDisplay = false;

	m_bShowHistPattern = false;
	m_bShowVariability = false;

	m_ViewMode = 0;

	m_K_Jam = 250;
	m_Max_Flow =2500;

	}
	virtual ~CDlgMOE();

	double m_K_Jam;
	double m_Max_Flow;

	CPoint m_last_cpoint;
	bool m_bMoveDisplay;

   int m_ViewMode;  // time series, QK curve

   void DrawSingleQKPlot(CPaintDC* pDC, CRect PlotRect);
   void DrawSingleVKPlot(CPaintDC* pDC, CRect PlotRect);
   void DrawSingleVQPlot(CPaintDC* pDC, CRect PlotRect);


   void DrawTimeSeriesPlot();
   void DrawQKCurve();

   int Cur_MOE_type1;
   int Cur_MOE_type2;
   bool m_bShowHistPattern;
   bool m_bShowVariability;
   float m_UnitData, m_UnitTime;
   int m_Range;
   int m_TmLeft, m_TmRight;
   float m_YUpperBound;
   float m_YLowerBound;

   COLORREF itsBackgroundColor;

	void DrawPlot(CPaintDC* pDC,int MOEType, CRect PlotRect, bool LinkTextFlag);
	void DrawTimeSeries(int MOEType , CPaintDC* pDC, CRect PlotRect, bool LinkTextFlag);
	bool ExportDataToCSVFile(char csv_file[_MAX_PATH]);
   int GetMaxYValue(int MOEType);
// Dialog Data
	enum { IDD = IDD_DIALOG_MOE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnMoetypeLinkvolume();
	afx_msg void OnMoetypeSpeed();
	afx_msg void OnMoetypeCumulativevolume();
	afx_msg void OnUpdateMoetypeLinkvolume(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMoetypeSpeed(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMoetypeCumulativevolume(CCmdUI *pCmdUI);
	afx_msg void OnMoetype2Linkvolume();
	afx_msg void OnMoetype2Speed();
	afx_msg void OnMoetype2Cumulativevolume();
	afx_msg void OnMoetype2None();
	afx_msg void OnDataExport();
	afx_msg void OnMoetype1Linkvolume();
	afx_msg void OnMoetype1Speed();
	afx_msg void OnMoetype2LinkvolumeVph();
	afx_msg void OnMoetype2SpeedMph();
	afx_msg void OnViewMoetimeseries();
	afx_msg void OnViewQ();
	afx_msg void OnMoetype3Q();
	afx_msg void OnMoetype2Density();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnViewResettimerange();
	afx_msg void OnEstimationHistoricalavgpattern();
	afx_msg void OnUpdateEstimationHistoricalavgpattern(CCmdUI *pCmdUI);
	afx_msg void OnViewMoevariabilityplot();
	afx_msg void OnUpdateViewMoevariabilityplot(CCmdUI *pCmdUI);
};
