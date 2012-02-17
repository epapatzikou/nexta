#pragma once
#include "afxcmn.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "BaseDialog.h"


// CDlgLinkList dialog

class CDlgLinkList : public CBaseDialog
{
	DECLARE_DYNAMIC(CDlgLinkList)
	CTLiteDoc* m_pDoc;


public:
	bool m_AVISensorFlag;
	CDlgLinkList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgLinkList();

// Dialog Data
	enum { IDD = IDD_DIALOG_LINK_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_LinkList;
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
};
