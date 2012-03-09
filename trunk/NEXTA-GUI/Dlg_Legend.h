#pragma once

#include "Network.h"
#include "TLiteDoc.h"
#include "BaseDialog.h"
// CDlg_Legend dialog

class CDlg_Legend : public CBaseDialog
{
	DECLARE_DYNAMIC(CDlg_Legend)

public:
	CDlg_Legend(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_Legend();

	CTLiteDoc* m_pDoc;


// Dialog Data
	enum { IDD = IDD_DIALOG_Legend };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
   afx_msg void OnSize(UINT nType, int cx, int cy);
};
