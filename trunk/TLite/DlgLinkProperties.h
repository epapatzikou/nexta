#pragma once
#include "afxwin.h"


// CDlgLinkProperties dialog

class CDlgLinkProperties : public CDialog
{
	DECLARE_DYNAMIC(CDlgLinkProperties)

public:
	CDlgLinkProperties(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgLinkProperties();

// Dialog Data
	enum { IDD = IDD_DIALOG_LinkProperties };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int FromNode;
	int ToNode;
	float LinkLength;
	float SpeedLimit;
	float FreeFlowTravelTime;
	int LaneCapacity;
	int nLane;
	int LinkType;
	CComboBox m_LinkTypeComboBox;

	float DefaultSpeedLimit;
	int DefaultCapacity;
	int DefaultnLane;
    int DefaultLinkType;

	virtual BOOL OnInitDialog();
	afx_msg void OnCbnEditchangeCombo1();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedSaveasdefault();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnCbnSelchangeCombo1();
};
