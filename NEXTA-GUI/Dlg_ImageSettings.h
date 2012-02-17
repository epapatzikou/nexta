#pragma once


// CDlg_ImageSettings dialog

class CDlg_ImageSettings : public CDialog
{
	DECLARE_DYNAMIC(CDlg_ImageSettings)

public:
	CDlg_ImageSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_ImageSettings();

// Dialog Data
	enum { IDD = IDD_DIALOG_Image_Width };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEditWidth();
	float m_RealworldWidth;
	float m_RealworldHeight;
};
