#pragma once
#include "afxcmn.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "BaseDialog.h"
#include "CGridListCtrlEx\\CGridListCtrlEx.h"
#include "afxwin.h"

// CDlgPathList dialog
enum ePathSelectionMode {ePathSelection_AllPaths,ePathSelection_FreewayOnly,ePathSelection_RampOnly,ePathSelection_ArterialOnly,ePathSelection_NoConnectors};

class CDlgPathList : public CBaseDialog
{
	DECLARE_DYNAMIC(CDlgPathList)
	CTLiteDoc* m_pDoc;
public:

	int m_MOEAggregationIntervalInMin;
	void CalculateTimeDependentTravelTime();
	ePathSelectionMode m_PathSelectionMode;
	bool m_AVISensorFlag;
	CDlgPathList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPathList();

	void ReloadData();
// Dialog Data
	enum { IDD = IDD_DIALOG_PATH_LIST };

   float m_UnitDistance, m_UnitTime, m_UnitData;
	void DrawPlot(CPaintDC* pDC, CRect PlotRect);

   int m_Range;
   int m_TimeLeft, m_TimeRight;
   float m_YUpperBound;
   float m_YLowerBound;

protected:
		std::vector<int> m_AggregationValueVector;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	CGridListCtrlEx m_ListCtrl;
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	BOOL m_ZoomToSelectedLink;
	CString m_StrPathMOE;
	afx_msg void OnBnClickedFindAlternative();
	afx_msg void OnBnClickedFindRandomRoute();
	afx_msg void OnCbnSelchangeCombo1();

	CComboBox m_ComboRandomCoef;
	afx_msg void OnBnClickedExportRoute();
	afx_msg void OnPathDataExportCSV();
	afx_msg void OnDataImportCsv();
	afx_msg void OnBnClickedCheckZoomToSelectedLink();
	CListBox m_PathList;
	afx_msg void OnLbnSelchangeList1();
	afx_msg void OnBnClickedDataAnalysis();
	afx_msg void OnDataGeneratesampleinputpathcsv();
	afx_msg void OnDataCleanallpaths();
	CComboBox m_StartHour;
	CComboBox m_EndHour;
	afx_msg void OnCbnSelchangeComboStarthour();
	afx_msg void OnCbnSelchangeComboEndhour();
	CComboBox m_AggregationIntervalList;
	afx_msg void OnCbnSelchangeComboAggintrevallist();
	afx_msg void OnCbnSelchangeComboPlotType();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	CComboBox m_PlotType;
};
