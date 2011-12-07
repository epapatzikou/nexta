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
	, m_SimulationHorizon(0)
	, m_EmissionDataOutput(FALSE)
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
	DDX_Text(pDX, IDC_EDIT_Simulation_Horizon, m_SimulationHorizon);
	DDV_MinMaxInt(pDX, m_SimulationHorizon, 0, 10000);
	DDX_Control(pDX, IDC_LIST_Routing_METHOD, m_RoutingMethod);
	DDX_Check(pDX, IDC_CHECK_EMISSION_DATA, m_EmissionDataOutput);
}


BEGIN_MESSAGE_MAP(CDlgAssignmentSettings, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SIMULATION_METHOD, &CDlgAssignmentSettings::OnLbnSelchangeListSimulationMethod)
	ON_BN_CLICKED(IDOK, &CDlgAssignmentSettings::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_COPY_VEHICLE_FILE, &CDlgAssignmentSettings::OnBnClickedButtonCopyVehicleFile)
	ON_LBN_SELCHANGE(IDC_LIST_DEMAND_LOADING_MODE, &CDlgAssignmentSettings::OnLbnSelchangeListDemandLoadingMode)
	ON_LBN_SELCHANGE(IDC_LIST_Routing_METHOD, &CDlgAssignmentSettings::OnLbnSelchangeListRoutingMethod)
END_MESSAGE_MAP()


// CDlgAssignmentSettings message handlers


BOOL CDlgAssignmentSettings::OnInitDialog()
{
	CDialog::OnInitDialog();

//	m_SimulationMethodControl.AddString ("BPR Function");
	m_SimulationMethodControl.AddString ("Point Queue Model");
	m_SimulationMethodControl.AddString ("Spatial Queue Model");
	m_SimulationMethodControl.AddString ("Newell's N-Curve Model");
	m_SimulationMethodControl.SetCurSel(m_SimultionMethod-1);

	m_DemandLoadingModeList.AddString ("Demand Table");
	m_DemandLoadingModeList.AddString ("Vehicle File");
	m_DemandLoadingModeList.SetCurSel(m_DemandLoadingMode);

	m_RoutingMethod.AddString ("OD-based routing");
	m_RoutingMethod.AddString ("Agent-based routing");
	m_RoutingMethod.SetCurSel(m_agent_based_assignment_flag);


	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgAssignmentSettings::OnLbnSelchangeListSimulationMethod()
{
	m_SimultionMethod = m_SimulationMethodControl.GetCurSel()+1;
}
void CDlgAssignmentSettings::OnLbnSelchangeListDemandLoadingMode()
{
	m_DemandLoadingMode = m_DemandLoadingModeList.GetCurSel();
}

void CDlgAssignmentSettings::OnLbnSelchangeListRoutingMethod()
{
	m_agent_based_assignment_flag = m_RoutingMethod.GetCurSel();
}

void CDlgAssignmentSettings::OnBnClickedOk()
{
	UpdateData(true);

	OnOK();
}

void CDlgAssignmentSettings::OnBnClickedButtonCopyVehicleFile()
{
   if(AfxMessageBox("Do you want to copy Vehicle.csv to input_vehicle.dat?", MB_YESNO|MB_ICONINFORMATION)== IDYES)
   {
      CWaitCursor wait;
      char fname_old[_MAX_PATH];
      char fname_new[_MAX_PATH];

      sprintf(fname_old,"%s",(m_ProjectDirectory + "Vehicle.csv"));
      sprintf(fname_new,"%s",(m_ProjectDirectory + "input_vehicle.dat"));

      FILE* st=fopen(fname_old,"r");

      if(st==NULL)
      {
	 AfxMessageBox("Source file Vehicle.dat cannot be found. Please submit a trial run to generate input_vehicle.dat.");
	 return;
      }
      fclose(st);

      if(CopyFile(fname_old, fname_new, false) == FALSE)
	 AfxMessageBox("Failed at copying Vehicle.csv to input_vehicle.dat.");
   }
}

