// DlgZoneToNodeMapping.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgZoneToNodeMapping.h"


// CDlgZoneToNodeMapping dialog

IMPLEMENT_DYNAMIC(CDlgZoneToNodeMapping, CDialog)

CDlgZoneToNodeMapping::CDlgZoneToNodeMapping(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgZoneToNodeMapping::IDD, pParent)
{

}

CDlgZoneToNodeMapping::~CDlgZoneToNodeMapping()
{
}

void CDlgZoneToNodeMapping::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_Mapping, m_ListMapping);
}


BEGIN_MESSAGE_MAP(CDlgZoneToNodeMapping, CDialog)
END_MESSAGE_MAP()


// CDlgZoneToNodeMapping message handlers

BOOL CDlgZoneToNodeMapping::OnInitDialog()
{
	CDialog::OnInitDialog();


//	m_ListMapping.AddString ();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
