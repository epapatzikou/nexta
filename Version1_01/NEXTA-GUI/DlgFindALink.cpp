// DlgFindALink.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgFindALink.h"

extern int read_2_integers_from_a_string(CString str, int &value1, int &value2);
// CDlgFindALink dialog

IMPLEMENT_DYNAMIC(CDlgFindALink, CDialog)

CDlgFindALink::CDlgFindALink(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlgFindALink::IDD, pParent)
	, m_StrFind(_T(""))
{

}

CDlgFindALink::~CDlgFindALink()
{
}

void CDlgFindALink::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ComboBox);
	DDX_Control(pDX, IDC_LIST1, m_FindObjectList);
}


BEGIN_MESSAGE_MAP(CDlgFindALink, CBaseDialog)
	ON_BN_CLICKED(IDC_BUTTON_FIND, &CDlgFindALink::OnBnClickedButtonFind)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDlgFindALink::OnLbnSelchangeList1)
END_MESSAGE_MAP()


// CDlgFindALink message handlers



BOOL CDlgFindALink::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	m_FindObjectList.AddString("Node ID");
	m_FindObjectList.AddString("Link: FromNodeID ToNodeID");
	m_FindObjectList.AddString("Path: FromNodeID ToNodeID");
	m_FindObjectList.AddString("Vehicle: VehicleID");
	m_FindObjectList.SetCurSel(m_SearchMode);



	for(unsigned int i = 0; i <m_SearchHistoryVector.size() ; i++)
	{

		int index=  m_SearchHistoryVector.size() -1- i;
		m_ComboBox.AddString(m_SearchHistoryVector[index]);
	

	}
	UpdateData(false);


	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgFindALink::OnBnClickedFindVehicle()
{
	UpdateData(true);
	m_SearchMode = efind_vehicle;
	OnOK();
	
}

void CDlgFindALink::OnBnClickedButtonFind()
{
	m_SearchMode = (eSEARCHMODE)m_FindObjectList.GetCurSel();
	CString sItem;
	m_ComboBox.GetWindowText(m_StrFind);

	
	m_ComboBox.AddString(m_StrFind);

	bool bFound = false;
	for(unsigned int i = 0; i <m_SearchHistoryVector.size() ; i++)
	{

		if(m_SearchHistoryVector[i].Compare (m_StrFind)>0)
		{
			bFound = true;
			break;
		}
	

	}
	
	if(bFound == false)
	m_SearchHistoryVector.push_back(m_StrFind);

	int value1 = 0; 
	int value2 = 0;
	read_2_integers_from_a_string(m_StrFind,value1,value2);



	switch(m_SearchMode)
	{
	case efind_node: m_NodeNumber = value1; break;
	case efind_link: m_FromNodeNumber = value1; m_ToNodeNumber = value2; break;
	case efind_path: m_FromNodeNumber = value1; m_ToNodeNumber = value2; break;
	case efind_vehicle: m_VehicleNumber = value1; break;

	default: 
	m_NodeNumber = value1;
	}



	OnOK();

}

void CDlgFindALink::OnLbnSelchangeList1()
{
	// TODO: Add your control notification handler code here
}
