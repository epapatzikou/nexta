#pragma once
#include "afxwin.h"


// CDlgZoneToNodeMapping dialog

class CDlgZoneToNodeMapping : public CDialog
{
	DECLARE_DYNAMIC(CDlgZoneToNodeMapping)

public:
	CDlgZoneToNodeMapping(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgZoneToNodeMapping();

// Dialog Data
	enum { IDD = IDD_DIALOG_Zone_To_Node_Mapping };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox m_ListMapping;
	virtual BOOL OnInitDialog();
};
