// TSView.h : interface of the CTimeSpaceView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSVIEW_H__81CB4DEC_FFC3_480A_9B92_163C4C8226DA__INCLUDED_)
#define AFX_TSVIEW_H__81CB4DEC_FFC3_480A_9B92_163C4C8226DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTimeSpaceView : public CView
{
protected: // create from serialization only
	CTimeSpaceView();
	DECLARE_DYNCREATE(CTimeSpaceView)

// Attributes
public:
	CTLiteDoc* GetTLDocument();

	bool m_bShowResourcePrice;
	bool m_bShowNodeCostLabel;

	bool m_bShowTimetable; 
	bool m_bShowSlackTime;

	int Cur_MOE_type1; // 0: mean travel time
	int Cur_MOE_type2; // 

	bool m_bMoveDisplay;
	CPoint m_last_cpoint;

   float m_UnitDistance, m_UnitTime;
   int m_Range;
   int m_TmLeft, m_TmRight;
   float m_YUpperBound;
   float m_YLowerBound;

   double m_TotalDistance;

// Operations
public:

	bool ExportTimetableDataToCSVFile(char csv_file[_MAX_PATH]);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeSpaceView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void RefreshWindow();
//	void DrawSchedule(int ScenarioNo);
//	void DrawTrain(int TrainNo);

	void DrawObjects(CDC* pDC,int MOEType,CRect PlotRect);

	virtual ~CTimeSpaceView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTimeSpaceView)
	afx_msg void OnShowTimetable();
	afx_msg void OnUpdateShowTimetable(CCmdUI* pCmdUI);
	afx_msg void OnShowSlacktime();
	afx_msg void OnUpdateShowSlacktime(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimetableResourceprice();
	afx_msg void OnUpdateTimetableResourceprice(CCmdUI *pCmdUI);
	afx_msg void OnTimetableNodecostlabel();
	afx_msg void OnUpdateTimetableNodecostlabel(CCmdUI *pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimetableExporttimetable();
	afx_msg void OnClose();
};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSVIEW_H__81CB4DEC_FFC3_480A_9B92_163C4C8226DA__INCLUDED_)
