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
	, m_DemandLoadingMultipler(0)
{
	m_bModifiedFlag  = false;
}

CDlgAssignmentSettings::~CDlgAssignmentSettings()
{

}

void CDlgAssignmentSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_Number_Iterations, m_NumberOfIterations);
	DDV_MinMaxInt(pDX, m_NumberOfIterations, 1, 100);
	DDX_Control(pDX, IDC_LIST_DEMAND_LOADING_MODE, m_DemandLoadingModeList);
	DDX_Control(pDX, IDC_LIST_Routing_METHOD, m_AssignmentMethod);
	DDX_Control(pDX, IDC_LIST_SIMULATION_METHOD, m_SimulationMethodControl);

	DDX_Check(pDX, IDC_CHECK_EMISSION_DATA, m_EmissionDataOutput);
	DDX_Control(pDX, IDC_LIST_DEMAND_LOADING_MODE2, m_NetworkDataList);
	DDX_Text(pDX, IDC_EDIT_Demand_LoadingMultiplier, m_DemandLoadingMultipler);
}


BEGIN_MESSAGE_MAP(CDlgAssignmentSettings, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_SIMULATION_METHOD, &CDlgAssignmentSettings::OnLbnSelchangeListSimulationMethod)
	ON_BN_CLICKED(IDOK, &CDlgAssignmentSettings::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_COPY_VEHICLE_FILE, &CDlgAssignmentSettings::OnBnClickedButtonCopyVehicleFile)
	ON_LBN_SELCHANGE(IDC_LIST_DEMAND_LOADING_MODE, &CDlgAssignmentSettings::OnLbnSelchangeListDemandLoadingMode)
	ON_LBN_SELCHANGE(IDC_LIST_Routing_METHOD, &CDlgAssignmentSettings::OnLbnSelchangeListRoutingMethod)
	ON_BN_CLICKED(IDOK2, &CDlgAssignmentSettings::OnBnClickedOk2)
	ON_LBN_SELCHANGE(IDC_LIST_DEMAND_LOADING_MODE2, &CDlgAssignmentSettings::OnLbnSelchangeListDemandLoadingMode2)
	ON_BN_CLICKED(IDC_CHECK_EMISSION_DATA, &CDlgAssignmentSettings::OnBnClickedCheckEmissionData)
END_MESSAGE_MAP()


// CDlgAssignmentSettings message handlers


BOOL CDlgAssignmentSettings::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString str;
	str.Format("%d nodes", m_pDoc->m_NodeSet.size());
	m_NetworkDataList.AddString(str);

	str.Format("%d links", m_pDoc->m_LinkSet.size());
	m_NetworkDataList.AddString(str);

	str.Format("%d zones", m_pDoc->m_ZoneMap.size());
	m_NetworkDataList.AddString(str);

	str.Format("%d activity locations", m_pDoc->m_ActivityLocationCount);
	m_NetworkDataList.AddString(str);

	str.Format("%d link types", m_pDoc->m_LinkTypeMap.size());
	m_NetworkDataList.AddString(str);

	m_SimulationMethodControl.AddString ("BPR Function");
	m_SimulationMethodControl.AddString ("Point Queue Model");
	m_SimulationMethodControl.AddString ("Spatial Queue Model");
	m_SimulationMethodControl.AddString ("Newell's N-Curve Model");
	m_SimulationMethodControl.SetCurSel(m_pDoc->m_traffic_flow_model);


	m_AssignmentMethod.AddString("Method of Successive Average");
	m_AssignmentMethod.AddString("Day-to-Day Learning");
	m_AssignmentMethod.AddString("Gap function-based Assignment");
	m_AssignmentMethod.AddString("Gap funciton-based MSA");
	m_AssignmentMethod.AddString("Accessibility (Distance)");
	m_AssignmentMethod.AddString("Accessibility (Travel Time)");
	m_AssignmentMethod.SetCurSel(m_pDoc->m_traffic_assignment_method);

	
	m_DemandLoadingModeList.AddString("Demand Loading Time Period:");

	str.Format("%s->%s (%s->%s)", 
		m_pDoc->GetTimeStampString24HourFormat(m_pDoc->m_DemandLoadingStartTimeInMin),
		m_pDoc->GetTimeStampString24HourFormat(m_pDoc->m_DemandLoadingEndTimeInMin),
		m_pDoc->GetTimeStampString(m_pDoc->m_DemandLoadingStartTimeInMin),
		m_pDoc->GetTimeStampString(m_pDoc->m_DemandLoadingEndTimeInMin)
		);

	m_DemandLoadingModeList.AddString(str);

	if(m_pDoc->m_ODME_mode==1)
	{
		m_DemandLoadingModeList.AddString("OD demand estimation mode");
	}

	if(m_pDoc->m_agent_demand_input_mode==1)
	{
		m_DemandLoadingModeList.AddString("load demand data from input_agent.bin file");
	}else
	{
		m_DemandLoadingModeList.AddString("Demand files:");

		for(unsigned i = 0; i< m_pDoc->m_DemandFileVector.size(); i++)
		{
			m_DemandLoadingModeList.AddString( m_pDoc->m_DemandFileVector[i]);

		}

	}

	m_NumberOfIterations = m_pDoc->m_number_of_assignment_days;

	m_EmissionDataOutput = m_pDoc->m_emission_data_output;

	m_DemandLoadingMultipler = m_pDoc->m_demand_multiplier;

	UpdateData(0);
	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgAssignmentSettings::OnLbnSelchangeListSimulationMethod()
{
}
void CDlgAssignmentSettings::OnLbnSelchangeListDemandLoadingMode()
{

}

void CDlgAssignmentSettings::OnLbnSelchangeListRoutingMethod()
{

}

void CDlgAssignmentSettings::OnBnClickedOk()
{



	UpdateData(true);


	if(m_pDoc->m_traffic_flow_model != m_SimulationMethodControl.GetCurSel())
	{
		m_bModifiedFlag = true;
		m_pDoc->m_traffic_flow_model = m_SimulationMethodControl.GetCurSel();
	}
	
	
	if(m_pDoc->m_traffic_assignment_method != m_AssignmentMethod.GetCurSel())
	{
		m_bModifiedFlag = true;
		m_pDoc->m_traffic_assignment_method = m_AssignmentMethod.GetCurSel();
	}

	if(m_NumberOfIterations != m_pDoc->m_number_of_assignment_days)
	{
		m_bModifiedFlag = true;
		m_pDoc->m_number_of_assignment_days = m_NumberOfIterations;
	}

	if(m_EmissionDataOutput != m_pDoc->m_emission_data_output)
	{
		m_bModifiedFlag = true;
		m_pDoc->m_emission_data_output = m_EmissionDataOutput;
	}


	if(fabs(m_DemandLoadingMultipler - m_pDoc->m_demand_multiplier)>0.00001)
	{
		m_bModifiedFlag = true;
		m_pDoc->m_demand_multiplier = m_DemandLoadingMultipler;
	}


	if(m_bModifiedFlag == true)
	{
	
		m_pDoc->WriteScenarioSettingCSVFile(m_pDoc->m_ProjectDirectory +"input_scenario_settings.csv");
	}

	OnOK();
}

void CDlgAssignmentSettings::OnBnClickedButtonCopyVehicleFile()
{
 /*  if(AfxMessageBox("Do you want to copy Vehicle.csv to input_vehicle.dat?", MB_YESNO|MB_ICONINFORMATION)== IDYES)
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
   }*/
}


void CDlgAssignmentSettings::OnBnClickedOk2()
{
	OnCancel( );
}

void CDlgAssignmentSettings::OnLbnSelchangeListDemandLoadingMode2()
{
	
}

void CDlgAssignmentSettings::OnBnClickedCheckEmissionData()
{
	UpdateData(1);
	m_bModifiedFlag = true;

}

