// Dlg_ImageSettings.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_ImageSettings.h"


// CDlg_ImageSettings dialog

IMPLEMENT_DYNAMIC(CDlg_ImageSettings, CDialog)

CDlg_ImageSettings::CDlg_ImageSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_ImageSettings::IDD, pParent)
	, m_RealworldWidth(0)
	, m_RealworldHeight(0)
{

}

CDlg_ImageSettings::~CDlg_ImageSettings()
{
}

void CDlg_ImageSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_RealworldWidth);
	DDX_Text(pDX, IDC_EDIT_WIDTH2, m_RealworldHeight);
}


BEGIN_MESSAGE_MAP(CDlg_ImageSettings, CDialog)
	ON_EN_CHANGE(IDC_EDIT_WIDTH, &CDlg_ImageSettings::OnEnChangeEditWidth)
END_MESSAGE_MAP()


// CDlg_ImageSettings message handlers

void CDlg_ImageSettings::OnEnChangeEditWidth()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
