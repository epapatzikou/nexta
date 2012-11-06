// DlgLinkProperties.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgLinkProperties.h"


// CDlgLinkProperties dialog

IMPLEMENT_DYNAMIC(CDlgLinkProperties, CDialog)

CDlgLinkProperties::CDlgLinkProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgLinkProperties::IDD, pParent)
	, FromNode(0)
	, ToNode(0)
	, LinkLength(0)
	, SpeedLimit(0)
	, FreeFlowTravelTime(0)
	, LaneCapacity(0)
	, nLane(0)
	, m_LinkID(0)
	, SaturationFlowRate(0)
	, EffectiveGreenTime(0)
	, StreetName(_T(""))
{

}

CDlgLinkProperties::~CDlgLinkProperties()
{
}

void CDlgLinkProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FROM_NODE, FromNode);
	DDV_MinMaxInt(pDX, FromNode, 1, 100000);
	DDX_Text(pDX, IDC_EDIT_To_NODE, ToNode);
	DDV_MinMaxInt(pDX, ToNode, 1, 1000000);
	DDX_Text(pDX, IDC_EDIT_LENGTH, LinkLength);
	DDV_MinMaxFloat(pDX, LinkLength, 0.001f, 10000);
	DDX_Text(pDX, IDC_EDIT_SPEEDLIMIT, SpeedLimit);
	DDV_MinMaxFloat(pDX, SpeedLimit, 5, 120);
	DDX_Text(pDX, IDC_EDIT_FFTT, FreeFlowTravelTime);
	DDX_Text(pDX, IDC_EDIT_LANE_CAPACITY, LaneCapacity);
	DDV_MinMaxInt(pDX, LaneCapacity, 0, 300000);
	DDX_Text(pDX, IDC_EDIT_NUMLANES, nLane);
	DDV_MinMaxInt(pDX, nLane, 0, 10);
	DDX_Control(pDX, IDC_COMBO1, m_LinkTypeComboBox);
	DDX_Text(pDX, IDC_EDIT_LINKID, m_LinkID);
	DDX_Text(pDX, IDC_EDIT_SATURATION_FLOW_RATE, SaturationFlowRate);
	DDV_MinMaxInt(pDX, SaturationFlowRate, 0, 5000);
	DDX_Text(pDX, IDC_EDIT_EFFECTIVE_GREEN_TIME, EffectiveGreenTime);
	DDV_MinMaxInt(pDX, EffectiveGreenTime, 0, 400);
	DDX_Text(pDX, IDC_EDIT_STREET_NAME, StreetName);
    DDV_MaxChars(pDX, StreetName, 100);

}


BEGIN_MESSAGE_MAP(CDlgLinkProperties, CDialog)
	ON_CBN_EDITCHANGE(IDC_COMBO1, &CDlgLinkProperties::OnCbnEditchangeCombo1)
	ON_BN_CLICKED(IDOK, &CDlgLinkProperties::OnBnClickedOk)
	ON_BN_CLICKED(IDSAVEASDEFAULT, &CDlgLinkProperties::OnBnClickedSaveasdefault)
	ON_BN_CLICKED(ID_CANCEL, &CDlgLinkProperties::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgLinkProperties::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CDlgLinkProperties message handlers

BOOL CDlgLinkProperties::OnInitDialog()
{
	CDialog::OnInitDialog();
	std::map<int, DTALinkType>:: const_iterator itr;

	for(itr = m_pDoc->m_LinkTypeMap.begin(); itr != m_pDoc->m_LinkTypeMap.end(); itr++)
	{
	
		CString str;
		str.Format("%d,%s",itr->first,itr->second .link_type_name.c_str ());
		m_LinkTypeComboBox.AddString (str);

		if(LinkType == itr->first)
		{
			m_LinkTypeComboBox.SetCurSel (m_LinkTypeComboBox.GetCount ()-1);
		}
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgLinkProperties::OnCbnEditchangeCombo1()
{

}

void CDlgLinkProperties::OnBnClickedOk()
{
	char m_Text[100];

	int CurSelNo = m_LinkTypeComboBox.GetCurSel ();

	char LinkTypeString[50]; 
	if(CurSelNo>0)
	{
	
		m_LinkTypeComboBox.GetLBText (CurSelNo,m_Text);
		sscanf_s(m_Text, "%d,%s", &LinkType, LinkTypeString,sizeof(LinkTypeString));
	
	}

	OnOK();
}

void CDlgLinkProperties::OnBnClickedSaveasdefault()
{
	UpdateData(false);

	DefaultSpeedLimit = SpeedLimit;
	DefaultCapacity = LaneCapacity;
	DefaultnLane = nLane;
    DefaultLinkType = LinkType;

}

void CDlgLinkProperties::OnBnClickedCancel()
{
	OnCancel();
}

void CDlgLinkProperties::OnCbnSelchangeCombo1()
{
	int SelectLinkType = LinkType = m_LinkTypeComboBox.GetCurSel ()+1;

	switch(SelectLinkType)
	{
	case 1: SpeedLimit = 65.0f; LaneCapacity = 1800; nLane = 3; break;
	case 2: SpeedLimit = 50.0f; LaneCapacity = 1450; nLane = 3; break;
	case 3: SpeedLimit = 40.0f; LaneCapacity = 1000; nLane = 3; break;
	case 4: SpeedLimit = 35.0f; LaneCapacity = 900; nLane = 3; break;
	case 5: SpeedLimit = 30.0f; LaneCapacity = 850; nLane = 2; break;
	case 6: SpeedLimit = 25.0f; LaneCapacity = 650; nLane = 1; break;
	case 7: SpeedLimit = 20.0f; LaneCapacity = 600; nLane = 1; break;
	case 8: SpeedLimit = 45.0f; LaneCapacity = 1000; nLane = 2; break;
	case 9: SpeedLimit = 30.0f; LaneCapacity = 1300; nLane = 2; break;
	case 10: SpeedLimit = 100.0f; LaneCapacity = 2000; nLane = 2; break;
	}

	UpdateData(true);

}
