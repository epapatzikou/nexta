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
	, m_NodeNumber(0)
	, m_StrNode(_T(""))
	, m_StrFromNode(_T(""))
	, m_StrToNode(_T(""))
	, m_StrVehicleID(_T(""))
{

}

CDlgFindALink::~CDlgFindALink()
{
}

void CDlgFindALink::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NODE, m_StrNode);
	DDX_Text(pDX, IDC_EDIT_FROM_NODE, m_StrFromNode);
	DDX_Text(pDX, IDC_EDIT_To_NODE, m_StrToNode);
	DDX_Text(pDX, IDC_EDIT_Vehicle_ID, m_StrVehicleID);
}


BEGIN_MESSAGE_MAP(CDlgFindALink, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgFindALink::OnBnClickedOk)
	ON_BN_CLICKED(IDOK_FIND_PATH, &CDlgFindALink::OnBnClickedFindPath)
	ON_BN_CLICKED(IDOK_FIND_NODE, &CDlgFindALink::OnBnClickedFindNode)
	ON_BN_CLICKED(IDOK_FIND_VEHICLE, &CDlgFindALink::OnBnClickedFindVehicle)
END_MESSAGE_MAP()


// CDlgFindALink message handlers

void CDlgFindALink::OnBnClickedOk()
{
	UpdateData(true);

	m_FromNodeNumber = atoi(m_StrFromNode);
	m_ToNodeNumber = atoi(m_StrToNode);

	m_SearchMode = efind_link;
	OnOK();
}

void CDlgFindALink::OnBnClickedFindPath()
{
	UpdateData(true);
	m_FromNodeNumber = atoi(m_StrFromNode);
	m_ToNodeNumber = atoi(m_StrToNode);

	m_SearchMode = efind_path;
	OnOK();
}

void CDlgFindALink::OnBnClickedFindNode()
{
	UpdateData(true);
	m_NodeNumber = atoi(m_StrNode);
	m_SearchMode = efind_node;
	OnOK();
}

BOOL CDlgFindALink::OnInitDialog()
{
	CDialog::OnInitDialog();


	m_StrFromNode.Format ("%d",m_FromNodeNumber);
	m_StrToNode.Format ("%d",m_ToNodeNumber);
	UpdateData(false);


	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgFindALink::OnBnClickedFindVehicle()
{
	UpdateData(true);
	m_VehicleNumber = atoi(m_StrVehicleID);
	m_SearchMode = efind_vehicle;
	OnOK();
	
}
