#pragma once
#include "afxwin.h"


// CDlgAssignmentSettings dialog

class CDlgAssignmentSettings : public CDialog
{
	DECLARE_DYNAMIC(CDlgAssignmentSettings)

public:
	CDlgAssignmentSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgAssignmentSettings();

// Dialog Data
	enum { IDD = IDD_DIALOG_Settings };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEdit1();
	int m_NumberOfIterations;
	float m_DemandGlobalMultiplier;
	int m_SimultionMethod;
	int m_DemandLoadingMode;
	virtual BOOL OnInitDialog();
	CListBox m_SimulationMethodControl;
	afx_msg void OnLbnSelchangeListSimulationMethod();
	CListBox m_DemandLoadingModeList;
	afx_msg void OnBnClickedOk();
};
