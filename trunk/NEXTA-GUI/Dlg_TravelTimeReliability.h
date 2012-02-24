#pragma once
#include "Chart_Control\\NSChartCtrl.h"
#include "TLite.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "afxwin.h"


// CDlg_TravelTimeReliability dialog

class CDlg_TravelTimeReliability : public CDialog
{
	DECLARE_DYNAMIC(CDlg_TravelTimeReliability)

public:
	CDlg_TravelTimeReliability(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_TravelTimeReliability();

	CNSChartCtrl m_chart_traveltime;
	CNSChartCtrl m_chart_7factors;
	INT m_iItem;
	HICON m_hIcon;

	CTLiteDoc* m_pDoc;

	CString m_FactorLabel[10];
	int m_FactorSize;

	float m_PathFreeFlowTravelTime;

	float	m_dValue;
	CString	m_sLabel;

// Dialog Data
	enum { IDD = IDD_NSCHART_TRAVEL_TIME_RELIABILITY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void ExportData(CString fname);

	// Generated message map functions
	BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnRadioButtons();
	afx_msg void OnRadioColors();
	DECLARE_MESSAGE_MAP()
public:
	void DisplayTravelTimeChart();
	void Display7FactorChart();

	afx_msg void OnLbnSelchangeList1();
	CListBox m_ReliabilityMOEList;
	afx_msg void OnLbnSelchangeList2();
	CListBox m_7FactorMOEList;
	afx_msg void OnBnClickedExportData();
	afx_msg void OnBnClickedModify();
	afx_msg void OnChartSelectedItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedEditScenario();
};
