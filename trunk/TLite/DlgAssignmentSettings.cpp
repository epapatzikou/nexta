// DlgAssignmentSettings.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgAssignmentSettings.h"


// CDlgAssignmentSettings dialog

IMPLEMENT_DYNAMIC(CDlgAssignmentSettings, CDialog)

CDlgAssignmentSettings::CDlgAssignmentSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAssignmentSettings::IDD, pParent)
	, m_NumberOfIterations(10)
	, m_DemandGlobalMultiplier(1.0f)
{

}

CDlgAssignmentSettings::~CDlgAssignmentSettings()
{
}

void CDlgAssignmentSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_Number_Iterations, m_NumberOfIterations);
	DDV_MinMaxInt(pDX, m_NumberOfIterations, 1, 100);
	DDX_Text(pDX, IDC_EDIT_Multiplier, m_DemandGlobalMultiplier);
	DDV_MinMaxFloat(pDX, m_DemandGlobalMultiplier, 0.001f, 10.0f);
	DDX_Control(pDX, IDC_LIST_SIMULATION_METHOD, m_SimulationMethodControl);
	DDX_Control(pDX, IDC_LIST_DEMAND_LOADING_MODE, m_DemandLoadingModeList);
}


BEGIN_MESSAGE_MAP(CDlgAssignmentSettings, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SIMULATION_METHOD, &CDlgAssignmentSettings::OnLbnSelchangeListSimulationMethod)
	ON_BN_CLICKED(IDOK, &CDlgAssignmentSettings::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgAssignmentSettings message handlers


BOOL CDlgAssignmentSettings::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_SimulationMethodControl.AddString ("BPR Function");
	m_SimulationMethodControl.AddString ("Point Queue Model");
	m_SimulationMethodControl.AddString ("Spatial Queue Model");
	m_SimulationMethodControl.AddString ("Newell's N-Curve Model");
	m_SimulationMethodControl.SetCurSel(m_SimultionMethod);

	m_DemandLoadingModeList.AddString ("Demand Table");
	m_DemandLoadingModeList.AddString ("Vehicle File");
	m_DemandLoadingModeList.SetCurSel(m_DemandLoadingMode);


	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgAssignmentSettings::OnLbnSelchangeListSimulationMethod()
{
	m_SimultionMethod = m_SimulationMethodControl.GetCurSel();
}

void CDlgAssignmentSettings::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
