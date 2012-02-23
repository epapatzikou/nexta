#pragma once

#include "TLite.h"
#include "Network.h"
#include "TLiteDoc.h"

// CDlg_SignalDataExchange dialog

class CDlg_SignalDataExchange : public CDialog
{
	DECLARE_DYNAMIC(CDlg_SignalDataExchange)

public:
	CDlg_SignalDataExchange(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_SignalDataExchange();

	CTLiteDoc* m_pDOC;
// Dialog Data
	enum { IDD = IDD_DIALOG_SIGNAL_DATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonGenerateData();
	afx_msg void OnBnClickedButtonQem();
};
