// Dlg_NodeProperties.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_NodeProperties.h"


// CDlg_NodeProperties dialog

IMPLEMENT_DYNAMIC(CDlg_NodeProperties, CDialog)

CDlg_NodeProperties::CDlg_NodeProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_NodeProperties::IDD, pParent)
	, NodeID(0)
	, NodeName(_T(""))
	, CycleLength(0)
	, ZoneID(0)

{

}

CDlg_NodeProperties::~CDlg_NodeProperties()
{
}

void CDlg_NodeProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NODE_ID, NodeID);
	DDX_Text(pDX, IDC_EDIT_STREET_NAME, NodeName);
	DDV_MaxChars(pDX, NodeName, 100);
	DDX_Text(pDX, IDC_EDIT_LENGTH, CycleLength);
	DDV_MinMaxInt(pDX, CycleLength, 0, 600);
	DDX_Text(pDX, IDC_EDIT_SPEEDLIMIT, ZoneID);
	DDV_MinMaxLong(pDX, ZoneID, 0, 1000000);
	DDX_Control(pDX, IDC_COMBO1, m_ControlTypeComboBox);
}


BEGIN_MESSAGE_MAP(CDlg_NodeProperties, CDialog)
	ON_BN_CLICKED(IDOK, &CDlg_NodeProperties::OnBnClickedOk)
	ON_BN_CLICKED(ID_CANCEL, &CDlg_NodeProperties::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON, &CDlg_NodeProperties::OnBnClickedButton)
END_MESSAGE_MAP()


// CDlg_NodeProperties message handlers

void CDlg_NodeProperties::OnBnClickedOk()
{
	UpdateData();
	ControlType = m_ControlTypeVector[m_ControlTypeComboBox.GetCurSel ()];

	if(ControlType== m_pDoc->m_ControlType_PretimedSignal || ControlType== m_pDoc-> m_ControlType_ActuatedSignal)
	{

		if(CycleLength==0)
		{
		AfxMessageBox("Please specify cycle length.");
		return;
		}
	}


	OnOK();
}

BOOL CDlg_NodeProperties::OnInitDialog()
{
	CDialog::OnInitDialog();

	std::map<int, DTALinkType>:: const_iterator itr;

	CString str;
	str.Format("Unknown Control (%d)",m_pDoc->m_ControlType_UnknownControl);
	m_ControlTypeComboBox.AddString (str);
	m_ControlTypeVector.push_back(m_pDoc->m_ControlType_UnknownControl);

	str.Format("No Control (%d)",m_pDoc->m_ControlType_NoControl);
	m_ControlTypeComboBox.AddString (str);
	m_ControlTypeVector.push_back(m_pDoc->m_ControlType_NoControl);

	str.Format("Yield Sign (%d)",m_pDoc->m_ControlType_YieldSign);
	m_ControlTypeComboBox.AddString (str);
	m_ControlTypeVector.push_back(m_pDoc->m_ControlType_NoControl);

	str.Format("2 Way Stop Sign (%d)",m_pDoc->m_ControlType_2wayStopSign);
	m_ControlTypeComboBox.AddString (str);
	m_ControlTypeVector.push_back(m_pDoc->m_ControlType_2wayStopSign);

	str.Format("4 Way Stop Sign (%d)",m_pDoc->m_ControlType_4wayStopSign);
	m_ControlTypeComboBox.AddString (str);
	m_ControlTypeVector.push_back(m_pDoc->m_ControlType_4wayStopSign);

	str.Format("Pretimed Signal (%d)",m_pDoc->m_ControlType_PretimedSignal);
	m_ControlTypeComboBox.AddString (str);
	m_ControlTypeVector.push_back(m_pDoc->m_ControlType_PretimedSignal);

	str.Format("Actuated Signal  (%d)",m_pDoc->m_ControlType_ActuatedSignal);
	m_ControlTypeComboBox.AddString (str);
	m_ControlTypeVector.push_back(m_pDoc->m_ControlType_ActuatedSignal);
	
	str.Format("Roundabout  (%d)",m_pDoc->m_ControlType_Roundabout);
	m_ControlTypeComboBox.AddString (str);
	m_ControlTypeVector.push_back(m_pDoc->m_ControlType_Roundabout);
	

	for(unsigned int i=0; i< m_ControlTypeVector.size(); i++)
	{
		if(ControlType == m_ControlTypeVector[i])
			m_ControlTypeComboBox.SetCurSel (i);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlg_NodeProperties::OnBnClickedCancel()
{
	OnCancel();
}

void CDlg_NodeProperties::OnBnClickedButton()
{
	CWaitCursor wait;
	if(ControlType== m_pDoc->m_ControlType_PretimedSignal || ControlType== m_pDoc-> m_ControlType_ActuatedSignal)
	{

		if(CycleLength==0)
		{
		AfxMessageBox("Please specify cycle length.");
		return;
		}
	}



	UpdateEffectiveGreenTime();
	OnOK();

}

void CDlg_NodeProperties::UpdateEffectiveGreenTime()
{
	m_pDoc->Modify();
	UpdateData();
	ControlType = m_ControlTypeVector[m_ControlTypeComboBox.GetCurSel ()];

	std::vector<int> data_vector;
	int count = 0;
	for (std::list<DTALink*>::iterator  iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_LayerNo == 0 )
		{
			int ToNodeID = (*iLink)->m_ToNodeID ;

			if(
				(NodeID == (*iLink)->m_ToNodeNumber)&&(ControlType == m_pDoc->m_ControlType_PretimedSignal || 
				ControlType == m_pDoc->m_ControlType_ActuatedSignal))
			{

				// from given BPR capacity to determine the effective green time
				(*iLink)->m_EffectiveGreenTimeInSecond = (int)(CycleLength * (*iLink)->m_LaneCapacity / (*iLink)->m_Saturation_flow_rate_in_vhc_per_hour_per_lane);
				data_vector.push_back((*iLink)->m_EffectiveGreenTimeInSecond);
				count++;

			}else
			{
				(*iLink)->m_EffectiveGreenTimeInSecond =0;
				data_vector.push_back((*iLink)->m_EffectiveGreenTimeInSecond);
				count++;
			}
		}
	
	}
	CString message;
	message.Format("The effective green time of %d links has been updated,\nbased on the cycle_length*lane_capacity/saturation_flow_rate.",count);
	AfxMessageBox(message,MB_ICONINFORMATION);
}