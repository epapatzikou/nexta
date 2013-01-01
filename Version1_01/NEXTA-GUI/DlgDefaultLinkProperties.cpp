// DlgDefaultLinkProperties.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgDefaultLinkProperties.h"


// CDlgDefaultLinkProperties dialog

IMPLEMENT_DYNAMIC(CDlgDefaultLinkProperties, CDialog)

CDlgDefaultLinkProperties::CDlgDefaultLinkProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDefaultLinkProperties::IDD, pParent)
	, SpeedLimit(0)
	, LaneCapacity(0)
	, NumLanes(0)
{

}

CDlgDefaultLinkProperties::~CDlgDefaultLinkProperties()
{
}

void CDlgDefaultLinkProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SPEEDLIMIT, SpeedLimit);
	DDV_MinMaxFloat(pDX, SpeedLimit, 0, 200);
	DDX_Text(pDX, IDC_EDIT_LANE_CAPACITY, LaneCapacity);
	DDX_Text(pDX, IDC_EDIT_NUMLANES, NumLanes);
	DDV_MinMaxInt(pDX, NumLanes, 1, 20);
	DDX_Control(pDX, IDC_LinkTypeLIST, m_LinkTypeList);
}


BEGIN_MESSAGE_MAP(CDlgDefaultLinkProperties, CDialog)
	ON_LBN_SELCHANGE(IDC_LinkTypeLIST, &CDlgDefaultLinkProperties::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDOK, &CDlgDefaultLinkProperties::OnBnClickedOk)
	ON_BN_CLICKED(ID_CANCEL, &CDlgDefaultLinkProperties::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgDefaultLinkProperties message handlers

void CDlgDefaultLinkProperties::OnLbnSelchangeList1()
{
	int SelectLinkType = m_LinkTypeList.GetCurSel() +1;
	switch(SelectLinkType)
	{
	case 1: SpeedLimit = 65.0f; LaneCapacity = 1800; NumLanes = 3; break;
	case 2: SpeedLimit = 50.0f; LaneCapacity = 1450; NumLanes = 3; break;
	case 3: SpeedLimit = 40.0f; LaneCapacity = 1000; NumLanes = 3; break;
	case 4: SpeedLimit = 35.0f; LaneCapacity = 900; NumLanes = 3; break;
	case 5: SpeedLimit = 30.0f; LaneCapacity = 850; NumLanes = 2; break;
	case 6: SpeedLimit = 25.0f; LaneCapacity = 650; NumLanes = 1; break;
	case 7: SpeedLimit = 20.0f; LaneCapacity = 600; NumLanes = 1; break;
	case 8: SpeedLimit = 45.0f; LaneCapacity = 1000; NumLanes = 2; break;
	case 9: SpeedLimit = 30.0f; LaneCapacity = 1300; NumLanes = 2; break;
	case 10: SpeedLimit = 100.0f; LaneCapacity = 2000; NumLanes = 2; break;
	}

	UpdateData(false);
}

BOOL CDlgDefaultLinkProperties::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString str;
		for(std::map<int, DTALinkType>::iterator itr = m_pDoc->m_LinkTypeMap.begin(); itr != m_pDoc->m_LinkTypeMap.end(); itr++)
		{
			if(itr->second .link_type_name.length () > 0 && itr->second .type_code.length () > 0)
			{

				str.Format("%d:%s,%s", itr->second .link_type  , 
					itr->second .link_type_name.c_str () , 
					itr->second .type_code .c_str ());

				m_LinkTypeList.AddString (str);
			}
		}
		

		m_LinkTypeList.SetCurSel (LinkType-1);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgDefaultLinkProperties::OnBnClickedOk()
{
	LinkType = m_LinkTypeList.GetCurSel () +1;
	OnOK();
}

void CDlgDefaultLinkProperties::OnBnClickedCancel()
{
	OnCancel();
}
