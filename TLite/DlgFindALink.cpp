// DlgFindALink.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgFindALink.h"


// CDlgFindALink dialog

IMPLEMENT_DYNAMIC(CDlgFindALink, CDialog)

CDlgFindALink::CDlgFindALink(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFindALink::IDD, pParent)
	, m_FromNodeNumber(0)
	, m_ToNodeNumber(0)
{

}

CDlgFindALink::~CDlgFindALink()
{
}

void CDlgFindALink::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FROM_NODE, m_FromNodeNumber);
	DDX_Text(pDX, IDC_EDIT_To_NODE, m_ToNodeNumber);
}


BEGIN_MESSAGE_MAP(CDlgFindALink, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgFindALink::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgFindALink message handlers

void CDlgFindALink::OnBnClickedOk()
{
	UpdateData(true);
	OnOK();
}
