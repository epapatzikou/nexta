#pragma once

#include "GridCtrl.h"
#include "ODDemandTableExt.h"

// CDlgGridCtrl dialog

class CDlgGridCtrl : public CDialog
{
	DECLARE_DYNAMIC(CDlgGridCtrl)

public:
	CDlgGridCtrl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgGridCtrl();

// Dialog Data
	enum { IDD = IDD_ODGRID_DIALOG };

private:
	CGridCtrl m_Grid;
	CODDemandTableExt m_ODTable;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGridSavequit();
	afx_msg void OnBnClickedGridQuit();
};
