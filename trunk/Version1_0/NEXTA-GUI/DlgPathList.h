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
	ePathSelectionMode m_PathSelectionMode;
	bool m_AVISensorFlag;
	CDlgPathList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPathList();

	void ReloadData();
// Dialog Data
	enum { IDD = IDD_DIALOG_PATH_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
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
};
