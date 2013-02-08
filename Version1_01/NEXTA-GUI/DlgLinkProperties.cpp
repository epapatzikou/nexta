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
	, m_TransitTravelTime(0)
	, m_TransitTransferTime(0)
	, m_TransitWaitingTime(0)
	, m_TransitFare(0)
	, m_BPR_Alpha(0)
	, m_BPR_Beta(0)
{
m_bTransitModeFlag = false;
m_bEditChange = false;
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
	DDV_MinMaxFloat(pDX, LinkLength, 0.0001f, 10000);
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

	DDX_Text(pDX, IDC_EDIT1, m_TransitTravelTime);
	DDX_Text(pDX, IDC_EDIT2, m_TransitTransferTime);
	DDX_Text(pDX, IDC_EDIT4, m_TransitWaitingTime);
	DDX_Text(pDX, IDC_EDIT5, m_TransitFare);
	DDX_Text(pDX, IDC_EDIT6, m_BPR_Alpha);
	DDX_Text(pDX, IDC_EDIT7, m_BPR_Beta);
}


BEGIN_MESSAGE_MAP(CDlgLinkProperties, CDialog)
	ON_CBN_EDITCHANGE(IDC_COMBO1, &CDlgLinkProperties::OnCbnEditchangeCombo1)
	ON_BN_CLICKED(IDOK, &CDlgLinkProperties::OnBnClickedOk)
	ON_BN_CLICKED(IDSAVEASDEFAULT, &CDlgLinkProperties::OnBnClickedSaveasdefault)
	ON_BN_CLICKED(ID_CANCEL, &CDlgLinkProperties::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgLinkProperties::OnCbnSelchangeCombo1)
	ON_EN_CHANGE(IDC_EDIT_LENGTH, &CDlgLinkProperties::OnEnChangeEditLength)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CDlgLinkProperties::OnBnClickedButtonUpdate)
	ON_EN_CHANGE(IDC_EDIT_STREET_NAME, &CDlgLinkProperties::OnEnChangeEditStreetName)
	ON_EN_CHANGE(IDC_EDIT_SPEEDLIMIT, &CDlgLinkProperties::OnEnChangeEditSpeedlimit)
	ON_EN_CHANGE(IDC_EDIT_NUMLANES, &CDlgLinkProperties::OnEnChangeEditNumlanes)
	ON_EN_CHANGE(IDC_EDIT_LANE_CAPACITY, &CDlgLinkProperties::OnEnChangeEditLaneCapacity)
	ON_EN_CHANGE(IDC_EDIT_SATURATION_FLOW_RATE, &CDlgLinkProperties::OnEnChangeEditSaturationFlowRate)
	ON_EN_CHANGE(IDC_EDIT_EFFECTIVE_GREEN_TIME, &CDlgLinkProperties::OnEnChangeEditEffectiveGreenTime)
	ON_EN_CHANGE(IDC_EDIT6, &CDlgLinkProperties::OnEnChangeEdit6)
	ON_EN_CHANGE(IDC_EDIT7, &CDlgLinkProperties::OnEnChangeEdit7)
	ON_EN_CHANGE(IDC_EDIT1, &CDlgLinkProperties::OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, &CDlgLinkProperties::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT4, &CDlgLinkProperties::OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT5, &CDlgLinkProperties::OnEnChangeEdit5)
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

	if(m_pDoc->m_bUseMileVsKMFlag)
	{
		SetDlgItemTextA(IDC_STATIC_UNIT_LENGTH,"(Miles)");
		SetDlgItemTextA(IDC_STATIC_UNIT_SPEED_LIMIT,"(mph)");
	}else
	{
		SetDlgItemTextA(IDC_STATIC_UNIT_LENGTH,"(km)");
		SetDlgItemTextA(IDC_STATIC_UNIT_SPEED_LIMIT,"(km/h)");
	}

		EnableDataBasedOnLinkType();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgLinkProperties::EnableDataBasedOnLinkType()
{
		int SelectLinkType = m_LinkTypeComboBox.GetCurSel ();

	std::map<int, DTALinkType>:: const_iterator itr;

	int count = 0;
	int link_type = -1;
	for(itr = m_pDoc->m_LinkTypeMap.begin(); itr != m_pDoc->m_LinkTypeMap.end(); itr++)
	{
	
		if(SelectLinkType == count)
		{
		link_type = itr->first;
			break;
		}
		count++;
	}

	if(link_type ==-1)
		return ;

		if(m_pDoc->m_LinkTypeMap[link_type].IsTransit () ||  m_pDoc->m_LinkTypeMap[link_type].IsWalking  ())
		{
		CEdit* pEdit_flow_rate = (CEdit*)GetDlgItem(IDC_EDIT_SATURATION_FLOW_RATE);
		pEdit_flow_rate->ShowWindow(SW_HIDE);
		
		CEdit* pEdit_green_time = (CEdit*)GetDlgItem(IDC_EDIT_EFFECTIVE_GREEN_TIME);
		pEdit_green_time->ShowWindow(SW_HIDE);

		CEdit* pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT1);
		pEdit1->ShowWindow(SW_SHOW);

		CEdit* pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT2);
		pEdit2->ShowWindow(SW_SHOW);

		CEdit* pEdit4 = (CEdit*)GetDlgItem(IDC_EDIT4);
		pEdit4->ShowWindow(SW_SHOW);

		CEdit* pEdit5 = (CEdit*)GetDlgItem(IDC_EDIT5);
		pEdit5->ShowWindow(SW_SHOW);	
		}else
		{

		CEdit* pEdit_flow_rate = (CEdit*)GetDlgItem(IDC_EDIT_SATURATION_FLOW_RATE);
		pEdit_flow_rate->ShowWindow(SW_SHOW);
		
		CEdit* pEdit_green_time = (CEdit*)GetDlgItem(IDC_EDIT_EFFECTIVE_GREEN_TIME);
		pEdit_green_time->ShowWindow(SW_SHOW);

		CEdit* pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT1);
		pEdit1->ShowWindow(SW_HIDE);

		CEdit* pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT2);
		pEdit2->ShowWindow(SW_HIDE);

		CEdit* pEdit4 = (CEdit*)GetDlgItem(IDC_EDIT4);
		pEdit4->ShowWindow(SW_HIDE);

		CEdit* pEdit5 = (CEdit*)GetDlgItem(IDC_EDIT5);
		pEdit5->ShowWindow(SW_HIDE);	
	
	}
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
	int SelectLinkType = m_LinkTypeComboBox.GetCurSel ();

	std::map<int, DTALinkType>:: const_iterator itr;

	int count = 0;
	for(itr = m_pDoc->m_LinkTypeMap.begin(); itr != m_pDoc->m_LinkTypeMap.end(); itr++)
	{
	
		if(SelectLinkType == count)
		{
			SpeedLimit = itr->second .default_speed;
			LaneCapacity = itr->second .default_lane_capacity ;
			nLane = itr->second .default_number_of_lanes ;

		}
		count++;
	}

	EnableDataBasedOnLinkType();

	UpdateData(false);

}

void CDlgLinkProperties::OnEnChangeEditLength()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnBnClickedButtonUpdate()
{
	UpdateData(1);
	FreeFlowTravelTime = LinkLength / max(0.001,SpeedLimit) * 60;
	UpdateData(0);

}

void CDlgLinkProperties::OnEnChangeEditStreetName()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEditSpeedlimit()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEditNumlanes()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEditLaneCapacity()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEditSaturationFlowRate()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEditEffectiveGreenTime()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEdit6()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEdit7()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEdit1()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEdit2()
{
	m_bEditChange = true;

}

void CDlgLinkProperties::OnEnChangeEdit4()
{
	m_bEditChange = true;
}

void CDlgLinkProperties::OnEnChangeEdit5()
{
	m_bEditChange = true;

}
