#pragma once

#include "TLiteDoc.h"
#include "GridCtrl_src\\GridCtrl.h"

// CDlgGridCtrl dialog

class CDlgGridCtrl : public CDialog
{
	DECLARE_DYNAMIC(CDlgGridCtrl)

public:
	CDlgGridCtrl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgGridCtrl();

// Dialog Data
	enum { IDD = IDD_ODGRID_DIALOG };

	CTLiteDoc* m_pDoc;

private:
	CGridCtrl m_Grid;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGridSavequit();
	afx_msg void OnBnClickedGridQuit();

	void OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	bool SaveDemandCSVFile(LPCTSTR lpszFileName);
	afx_msg void OnBnClickedButtonCreatezones();
	afx_msg void OnBnClickedButtonEditZoneNodeMapping();
};
