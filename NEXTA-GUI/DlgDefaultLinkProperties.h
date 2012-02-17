#pragma once
#include "afxwin.h"


// CDlgDefaultLinkProperties dialog

class CDlgDefaultLinkProperties : public CDialog
{
	DECLARE_DYNAMIC(CDlgDefaultLinkProperties)

public:
	CDlgDefaultLinkProperties(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgDefaultLinkProperties();

// Dialog Data
	enum { IDD = IDD_DIALOG_DefaultLinkProperties };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	float SpeedLimit;
	int LaneCapacity;
	int NumLanes;
	int LinkType;
	afx_msg void OnLbnSelchangeList1();
	virtual BOOL OnInitDialog();
	CListBox m_LinkTypeList;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
