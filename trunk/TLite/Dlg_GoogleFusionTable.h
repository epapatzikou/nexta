#pragma once


// CDlg_GoogleFusionTable dialog

class CDlg_GoogleFusionTable : public CDialog
{
	DECLARE_DYNAMIC(CDlg_GoogleFusionTable)

public:
	CDlg_GoogleFusionTable(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_GoogleFusionTable();

// Dialog Data
	enum { IDD = IDD_DIALOG_Goolge_Fusion_Table };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
