// Dlg_DisplayConfiguration.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_DisplayConfiguration.h"
#include "MainFrm.h"

// CDlg_DisplayConfiguration dialog

IMPLEMENT_DYNAMIC(CDlg_DisplayConfiguration, CBaseDialog)

CDlg_DisplayConfiguration::CDlg_DisplayConfiguration(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlg_DisplayConfiguration::IDD, pParent)
	, m_bShowSignalNodeMovementOnly(TRUE)
{
	m_ShowNodeTextMode = node_display_none;

}

CDlg_DisplayConfiguration::~CDlg_DisplayConfiguration()
{
}

void CDlg_DisplayConfiguration::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LINK_TEXT_LABEL, m_Link_Label);
	DDX_Control(pDX, IDC_LIST_NODE_TEXT_LABEL, m_Node_Label);
	//	DDX_Control(pDX, IDC_MOE_AGGREGATION_INTERVAL_LIST, m_AggregationIntervalList);
	DDX_Control(pDX, IDC_LIST_MOVEMENT_TEXT_LABEL, m_Movement_Label);
	//	DDX_Control(pDX, IDC_LIST_GPS_TEXT_LABEL, m_GPS_Label);
	DDX_Check(pDX, IDC_CHECK_SIGNAL_NODE_ONLY, m_bShowSignalNodeMovementOnly);
	DDX_Control(pDX, IDC_COMBOTIMINGPLAN, m_TimingPlanComboBox);
	DDX_Control(pDX, IDC_COMBO_DataSource, m_ComboDataSourceType);
	DDX_Control(pDX, IDC_COMBO_Aggregation_Interval, m_Combo_Aggregation_Interval);
}


BEGIN_MESSAGE_MAP(CDlg_DisplayConfiguration, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_LINK_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListLinkTextLabel)
	ON_LBN_SELCHANGE(IDC_LIST_NODE_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListNodeTextLabel)
	ON_LBN_SELCHANGE(IDC_MOE_AGGREGATION_INTERVAL_LIST, &CDlg_DisplayConfiguration::OnLbnSelchangeMoeAggregationIntervalList)
	ON_LBN_SELCHANGE(IDC_LIST_ZONE_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListZoneTextLabel)
	ON_BN_CLICKED(IDOK, &CDlg_DisplayConfiguration::OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_LIST_MOVEMENT_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListMovementTextLabel)
	ON_LBN_SELCHANGE(IDC_LIST_GPS_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListGpsTextLabel)
	ON_LBN_DBLCLK(IDC_LIST_Size_Text_Control, &CDlg_DisplayConfiguration::OnLbnDblclkListSizeTextControl)
	ON_CBN_SELCHANGE(IDC_COMBO_WalkingDistance, &CDlg_DisplayConfiguration::OnCbnSelchangeComboWalkingdistance)
	ON_CBN_SELCHANGE(IDC_COMBO_TransitTime, &CDlg_DisplayConfiguration::OnCbnSelchangeComboTransittime)
	ON_LBN_SELCHANGE(IDC_LIST_Size_Text_Control, &CDlg_DisplayConfiguration::OnLbnSelchangeListSizeTextControl)
	ON_BN_CLICKED(IDC_BUTTON_INCREASE_SIZE, &CDlg_DisplayConfiguration::OnBnClickedButtonIncreaseSize)
	ON_BN_CLICKED(IDC_BUTTON_DECREASE_SIZE, &CDlg_DisplayConfiguration::OnBnClickedButtonDecreaseSize)
	ON_BN_CLICKED(IDC_CHECK_SIGNAL_NODE_ONLY, &CDlg_DisplayConfiguration::OnBnClickedCheckSignalNodeOnly)
	ON_BN_CLICKED(IDC_BUTTON_INCREASE_TEXT_SIZE, &CDlg_DisplayConfiguration::OnBnClickedButtonIncreaseTextSize)
	ON_BN_CLICKED(IDC_BUTTON_DECREASE_TEXT_SIZE, &CDlg_DisplayConfiguration::OnBnClickedButtonDecreaseTextSize)
	ON_BN_CLICKED(IDC_BUTTON_INCREASE_NODE_TEXT_SIZE, &CDlg_DisplayConfiguration::OnBnClickedButtonIncreaseNodeTextSize)
	ON_BN_CLICKED(IDC_BUTTON_DECREASE_NODE_TEXT_SIZE, &CDlg_DisplayConfiguration::OnBnClickedButtonDecreaseNodeTextSize)
	ON_BN_CLICKED(IDC_BUTTON_INCREASE_LINK_TEXT_SIZE2, &CDlg_DisplayConfiguration::OnBnClickedButtonIncreaseLinkTextSize2)
	ON_BN_CLICKED(IDC_BUTTON_DECREASE_LINK_TEXT_SIZE2, &CDlg_DisplayConfiguration::OnBnClickedButtonDecreaseLinkTextSize2)
	ON_CBN_SELCHANGE(IDC_COMBOTIMINGPLAN, &CDlg_DisplayConfiguration::OnCbnSelchangeCombotimingplan)
	ON_CBN_SELCHANGE(IDC_COMBO_DataSource, &CDlg_DisplayConfiguration::OnCbnSelchangeComboDatasource)
	ON_CBN_SELCHANGE(IDC_COMBO_Aggregation_Interval, &CDlg_DisplayConfiguration::OnCbnSelchangeComboAggregationInterval)
END_MESSAGE_MAP()


// CDlg_DisplayConfiguration message handlers

void CDlg_DisplayConfiguration::OnLbnSelchangeListLinkTextLabel()
{
	pView->m_ShowLinkTextMode  = (link_text_display_mode)m_Link_Label.GetCurSel();;
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnLbnSelchangeListNodeTextLabel()
{
	int m_ShowNodeTextMode =  m_Node_Label.GetCurSel();
	pView->m_ShowNodeTextMode = (node_display_mode)m_ShowNodeTextMode;
	pView->Invalidate ();

}

BOOL CDlg_DisplayConfiguration::OnInitDialog()
{
	CDialog::OnInitDialog();


	m_Node_Label.AddString("None");
	m_Node_Label.AddString("Node ID");
	m_Node_Label.AddString("Sequential Node No.");
	m_Node_Label.AddString("Zone ID of Activity Location");
	m_Node_Label.AddString("Cycle Length In Second");
	m_Node_Label.AddString("Cycle Length for Signals only");
	m_Node_Label.AddString("Offset In Second for Signals only");
	m_Node_Label.AddString("Intersection Name");
	m_Node_Label.AddString("Control Type");
//	m_Node_Label.AddString("Reserved Value");


	m_Node_Label.SetCurSel ((int)(m_ShowNodeTextMode));

	m_Link_Label.AddString("None");
	m_Link_Label.AddString("Street Name");

	CTLiteDoc* m_pDoc = pView->GetDocument();



	m_ComboDataSourceType.AddString ("Simulation Data");
	m_ComboDataSourceType.AddString ("Sensor Data");

	if(m_pDoc->m_PrimaryDataSource == eSimulationData)
		m_ComboDataSourceType.SetCurSel (0);
	else
		m_ComboDataSourceType.SetCurSel (1);




	if(m_pDoc->m_bUseMileVsKMFlag)
	{
	m_Link_Label.AddString("Speed Limit (mph)");
	m_Link_Label.AddString("Length (mile)");
	m_Link_Label.AddString("Length (feet)");
	}else
	{
	m_Link_Label.AddString("Speed Limit (mph)");
	m_Link_Label.AddString("Length (km)");
	m_Link_Label.AddString("Length (meter)");
	}

	m_Link_Label.AddString("# of Lanes ");
	m_Link_Label.AddString("# of Left Turn Lanes (positive value only)");

	m_Link_Label.AddString("Link Capacity Per Hour");
	m_Link_Label.AddString("Lane Capacity Per Hour");
	m_Link_Label.AddString("Total Link Volume");

	m_Link_Label.AddString("Level Of Service");

	if(m_pDoc->m_bUseMileVsKMFlag)
		m_Link_Label.AddString("Avg Simulated Speed (mph)");
	else
		m_Link_Label.AddString("Avg Simulated Speed (kmph)");

	m_Link_Label.AddString("Avg Travel Time (min)");
	m_Link_Label.AddString("Avg Delay Per Vehicle (min)");

	m_Link_Label.AddString("Link ID");
	m_Link_Label.AddString("Speed Sensor ID");
	m_Link_Label.AddString("Count Sensor ID");
	m_Link_Label.AddString("Link Key (for model MOE)");


	m_Link_Label.AddString("From ID -> To ID");

	m_Link_Label.AddString("Free Flow Travel Time (min)");
	m_Link_Label.AddString("Free Flow Travel Time (hour)");

	m_Link_Label.AddString("# of Right Turn Lanes (positive value only)");

	if(m_pDoc->m_bUseMileVsKMFlag)
	{
	m_Link_Label.AddString("Length of Left Turn Lanes (feet)");
	m_Link_Label.AddString("Length of Right Turn Lanes (feet)");
	}else
	{
	m_Link_Label.AddString("Length of Left Turn Lanes (meter)");
	m_Link_Label.AddString("Length of Right Turn Lanes (meter)");
	
	}

	m_Link_Label.AddString("-- Additional Static Attributes --");
	m_Link_Label.AddString("Saturation Flow Rate");
	m_Link_Label.AddString("Grade (%)");
	
	if(m_pDoc->m_bUseMileVsKMFlag)
	{
	m_Link_Label.AddString("Jam Density (vhc/mile/ln)");
	m_Link_Label.AddString("Wave Speed (mph)");
	}else
	{
	m_Link_Label.AddString("Jam Density (vhc/km/ln)");
	m_Link_Label.AddString("Wave Speed (kmph)");
	}
	m_Link_Label.AddString("BPR alpha term");
	m_Link_Label.AddString("BPR beta term");

	m_Link_Label.AddString("Link type In Text");
	m_Link_Label.AddString("Link type In Number");
	m_Link_Label.AddString("Internal Link id");

	m_Link_Label.AddString("-- Link MOE --");
	m_Link_Label.AddString("Total Link Delay (hour)");
	m_Link_Label.AddString("Total Volume over Capacity Ratio");

	if(m_pDoc->m_bUseMileVsKMFlag)
		m_Link_Label.AddString("Avg Speed (kmph)");
	else
		m_Link_Label.AddString("Avg Speed (mph)");

	m_Link_Label.AddString("Avg Waiting Time on Loading Buffer");


	m_Link_Label.AddString("Time-dependent Link Volume");
	m_Link_Label.AddString("Time-dependent Lane Volume");


	m_Link_Label.AddString("Time-dependent Speed (mph)");
	m_Link_Label.AddString("Time-dependent Speed (km/h)");
	m_Link_Label.AddString("Time-dependent Density (vhc/mile/ln)");
	
	m_Link_Label.AddString("Time-dependent Queue Length (%)");
	m_Link_Label.AddString("Time-dependent Congestion Duration (min)");
	m_Link_Label.AddString("Time-dependent Congestion Start Time");


//	m_Link_Label.AddString("Total Assigned Link Volume");
//	m_Link_Label.AddString("Total Link Volume of Incomplete Trips");

	m_Link_Label.AddString("-- Observations --");
	m_Link_Label.AddString("Total Observed Link Count");
	m_Link_Label.AddString("Total Observed vs. Simulated Link Count");
	m_Link_Label.AddString("Total Link Count Error");
//	m_Link_Label.AddString("Simulated AADT");
//	m_Link_Label.AddString("Observed Bidirectional AADT");
//	m_Link_Label.AddString("Observed Peak Hour Volume");

	
	m_Link_Label.SetCurSel ((int)(pView->m_ShowLinkTextMode));

		for(int tp = 0; tp< m_pDoc->m_TimingPlanVector.size(); tp++)  //  loop for all timing plans
		{

			std::string timing_plan_name = m_pDoc->m_TimingPlanVector[tp].timing_plan_name;  // fetch timing_plan (unique) name
		
			m_TimingPlanComboBox.AddString (timing_plan_name.c_str ());
		}

		if(m_TimingPlanComboBox.GetCount () >0)
		m_TimingPlanComboBox.SetCurSel (0);
	

	m_Movement_Label.AddString("None");
	m_Movement_Label.AddString ("Turn Type");
	m_Movement_Label.AddString ("# of Lanes");
	m_Movement_Label.AddString ("Simulated Hourly Count");
	m_Movement_Label.AddString ("Simulated Turning %");
	m_Movement_Label.AddString ("Simulated Turn Delay (sec)");
	m_Movement_Label.AddString ("QEM Phase1");
	m_Movement_Label.AddString ("QEM Effective Green (sec)");


	m_Movement_Label.AddString ("Upstream Node Number");
	m_Movement_Label.AddString ("Downstream Node Number");
	m_Movement_Label.AddString ("Up,Current,Dest Node Numbers");
	m_Movement_Label.AddString ("Protected//Permited//Prohibited");

	m_Movement_Label.AddString ("Simulated Total Count");
	m_Movement_Label.AddString ("Simulated Turn Delay (min)");

//	m_Movement_Label.AddString ("Observed Total Count");
	m_Movement_Label.AddString ("Observed Hourly Count");
	m_Movement_Label.AddString ("Observed Turning %");
	m_Movement_Label.AddString ("Observed Turn Delay (sec)");

	m_Movement_Label.AddString ("Shared Lane Flag");
	m_Movement_Label.AddString ("Lane Width");
	m_Movement_Label.AddString ("Storage");
	m_Movement_Label.AddString ("StLanes");
	m_Movement_Label.AddString ("Grade");
	m_Movement_Label.AddString ("Speed");

	m_Movement_Label.AddString ("Ideal Flow");
	m_Movement_Label.AddString ("Lost Time");
	m_Movement_Label.AddString ("PermPhase1");
	m_Movement_Label.AddString ("DetectPhase1");

	m_Movement_Label.AddString ("Volume");
	m_Movement_Label.AddString ("Turning %");


	m_Movement_Label.AddString ("Capacity");
	m_Movement_Label.AddString ("VOC");
	m_Movement_Label.AddString ("DischargeRate");
	m_Movement_Label.AddString ("Control Delay (sec)");
	m_Movement_Label.AddString ("LOS");

	m_Movement_Label.SetCurSel ((int)(pView->m_ShowMovementTextMode));

	//m_GPS_Label.AddString("None");
	//m_GPS_Label.AddString("Vehicle ID");
	//m_GPS_Label.AddString("Timestamp in min");
	//m_GPS_Label.AddString("Time Gap in min");
	//m_GPS_Label.AddString("GPS Speed (mph)");
	//m_GPS_Label.AddString("All Trajectories");
	//m_GPS_Label.SetCurSel ((int)(pView->m_ShowGPSTextMode));


	movement_text_size_vector.push_back(10);
	movement_text_size_vector.push_back(50);
	movement_text_size_vector.push_back(100);
	movement_text_size_vector.push_back(150);
	movement_text_size_vector.push_back(200);
	movement_text_size_vector.push_back(250);
	movement_text_size_vector.push_back(300);
	movement_text_size_vector.push_back(500);
	movement_text_size_vector.push_back(1000);
	movement_text_size_vector.push_back(2000);
	movement_text_size_vector.push_back(5000);

	CTLiteDoc* pDoc = pView->GetDocument();
	

	walking_distance_vector.push_back(0.01);
	walking_distance_vector.push_back(0.05);
	walking_distance_vector.push_back(0.10);
	walking_distance_vector.push_back(0.15);
	walking_distance_vector.push_back(0.20);
	walking_distance_vector.push_back(0.25);
	walking_distance_vector.push_back(0.30);
	walking_distance_vector.push_back(0.35);
	walking_distance_vector.push_back(0.40);
	walking_distance_vector.push_back(0.45);
	walking_distance_vector.push_back(0.50);
	walking_distance_vector.push_back(1.00);


	aggregation_interval_vector.push_back(1);
	aggregation_interval_vector.push_back(5);
	aggregation_interval_vector.push_back(15);
	aggregation_interval_vector.push_back(30);
	aggregation_interval_vector.push_back(60);
	aggregation_interval_vector.push_back(120);
	aggregation_interval_vector.push_back(1440);


	for(int ia = 0; ia < aggregation_interval_vector.size(); ia++)
	{
		CString msg ;
		msg.Format("%d",aggregation_interval_vector[ia]);
		m_Combo_Aggregation_Interval.AddString (msg);

		if(aggregation_interval_vector[ia] == g_MOEAggregationIntervalInMin)
		{
		m_Combo_Aggregation_Interval.SetCurSel  (ia);
		}
	}

	/*for(unsigned int i=0; i< walking_distance_vector.size(); i++)
	{
		CString str;
		str.Format("%.2f",walking_distance_vector[i]);
		m_ComboxBox_WalkingDistance.AddString(str);

		if(walking_distance_vector[i] == pView->GetDocument ()->m_max_walking_distance )
		{
		m_ComboxBox_WalkingDistance.SetCurSel (i);
		}
	}*/
	
	transit_time_vector.push_back(15);
	transit_time_vector.push_back(30);
	transit_time_vector.push_back(45);
	transit_time_vector.push_back(60);
	transit_time_vector.push_back(75);
	transit_time_vector.push_back(90);
	transit_time_vector.push_back(120);
	//
	//for(unsigned int i=0; i< transit_time_vector.size(); i++)
	//{
	//	CString str;
	//	str.Format("%d",transit_time_vector[i]);
	//	m_ComboxBox_TransitTime.AddString(str);

	//	if(transit_time_vector[i] == pView->GetDocument ()->m_max_accessible_transit_time_in_min   )
	//	{
	//	m_ComboxBox_TransitTime.SetCurSel (i);
	//	}
	//}
	//

	//m_AggregationValueVector.push_back(1);
	//m_AggregationValueVector.push_back(5);
	//m_AggregationValueVector.push_back(15);
	//m_AggregationValueVector.push_back(30);
	//m_AggregationValueVector.push_back(60);
	//m_AggregationValueVector.push_back(120);
	//m_AggregationValueVector.push_back(1440);

	//for(unsigned int i = 0;  i< m_AggregationValueVector.size (); i++)
	//{
	//	CString str;
	//	str.Format("%d min",m_AggregationValueVector[i]);

	//	m_AggregationIntervalList.AddString (str);

	//	if(g_MOEAggregationIntervalInMin  == m_AggregationValueVector[i])
	//	{
	//	m_AggregationIntervalList.SetCurSel (i);
	//
	//	}
	//
	//}

	//m_SizeTextControl_List.AddString ("Increase Node Size (Pg Up)");
	//m_SizeTextControl_List.AddString ("Decrease Node Size (Pg Up)");
	//m_SizeTextControl_List.AddString ("Increase Node Text Size");
	//m_SizeTextControl_List.AddString ("Decrease Node Text Size");
	//m_SizeTextControl_List.AddString ("Increase Link/Movement Text Size");
	//m_SizeTextControl_List.AddString ("Decrease Link/Movement Text Size");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlg_DisplayConfiguration::OnLbnSelchangeMoeAggregationIntervalList()
{
	g_MOEAggregationIntervalInMin = m_AggregationValueVector [ m_AggregationIntervalList.GetCurSel()];

//
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnLbnSelchangeListZoneTextLabel()
{
	// TODO: Add your control notification handler code here
}

void CDlg_DisplayConfiguration::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void CDlg_DisplayConfiguration::OnLbnSelchangeListMovementTextLabel()
{
	pView->m_ShowMovementTextMode  = (movement_text_display_mode)m_Movement_Label.GetCurSel();

	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnLbnSelchangeListGpsTextLabel()
{
	pView->m_ShowGPSTextMode  = (GPS_display_mode)m_GPS_Label.GetCurSel();

	pView->Invalidate ();
}


void CDlg_DisplayConfiguration::OnBnClickedButtonIncreasenodesize()
{
	CTLiteDoc* pDoc = pView->GetDocument();

	pDoc->m_NodeDisplaySize *=1.2;

	pView->Invalidate ();
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonDecreasenodesize()
{
	CTLiteDoc* pDoc = pView->GetDocument();

	pDoc->m_NodeDisplaySize /=1.2;
	pView->Invalidate ();
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->UpdateLegendView();

}

void CDlg_DisplayConfiguration::OnBnClickedButtonIncreasenodetextsize()
{
	CTLiteDoc* pDoc = pView->GetDocument();

	pDoc->m_NodeTextDisplayRatio *=1.1;

	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonDecreasenodetextsize()
{
	CTLiteDoc* pDoc = pView->GetDocument();

	pDoc->m_NodeTextDisplayRatio /=1.1;

	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnLbnDblclkListSizeTextControl()
{
	CTLiteDoc* pDoc = pView->GetDocument();



	int CurSelection = m_SizeTextControl_List.GetCurSel();

	switch (CurSelection)
	{
	case 0: pDoc->m_NodeDisplaySize *=1.2; break;
	case 1: pDoc->m_NodeDisplaySize /=1.2; break;
	case 2: 	pDoc->m_NodeTextDisplayRatio *=1.1; break;
	case 3: 	pDoc->m_NodeTextDisplayRatio /=1.1; break;
	case 4: 	pView->m_LinkTextFontSize = max(pView->m_LinkTextFontSize+1,pView->m_LinkTextFontSize * 1.1); break;
	case 5: 	pView->m_LinkTextFontSize = max(1,min (pView->m_LinkTextFontSize-1,pView->m_LinkTextFontSize/1.1)); break;

	}

	pView->Invalidate ();
		
}

void CDlg_DisplayConfiguration::OnCbnSelchangeComboWalkingdistance()
{
	 //pView->GetDocument ()->m_max_walking_distance  = walking_distance_vector [ m_ComboxBox_WalkingDistance.GetCurSel()];
	 //pView->FindAccessibleTripIDWithCurrentMousePoint();
	 //pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnCbnSelchangeComboTransittime()
{
	// pView->GetDocument ()->m_max_accessible_transit_time_in_min = transit_time_vector [ m_ComboxBox_TransitTime.GetCurSel()];
	//pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnLbnSelchangeListSizeTextControl()
{
	// TODO: Add your control notification handler code here
}

void CDlg_DisplayConfiguration::OnBnClickedButtonIncreaseSize()
{
	CTLiteDoc* pDoc = pView->GetDocument();

	pDoc->m_MovementTextBoxSizeInFeet *=1.1;
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonDecreaseSize()
{
	CTLiteDoc* pDoc = pView->GetDocument();

	pDoc->m_MovementTextBoxSizeInFeet /=1.1;
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnBnClickedCheckSignalNodeOnly()
{
	UpdateData();
	CTLiteDoc* pDoc = pView->GetDocument();
	pDoc->m_bShowSignalNodeMovementOnly = m_bShowSignalNodeMovementOnly;

	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonIncreaseTextSize()
{
	pView->m_MovmentTextSize*=1.1;
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonDecreaseTextSize()
{
	pView->m_MovmentTextSize/=1.1;
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonIncreaseNodeTextSize()
{
	pView->OnViewIncreasenodesize();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonDecreaseNodeTextSize()
{
	pView->OnViewDecreatenodesize();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonIncreaseLinkTextSize2()
{
	pView->m_LinkTextSize*=1.1;
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonDecreaseLinkTextSize2()
{
	pView->m_LinkTextSize/=1.1;
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnCbnSelchangeCombotimingplan()
{
	CString str;
	m_TimingPlanComboBox.GetLBText(m_TimingPlanComboBox.GetCurSel(), str);

	CTLiteDoc* m_pDoc = pView->GetDocument();

	 m_pDoc->m_CurrentDisplayTimingPlanName =  m_pDoc->CString2StdString (str); 
}

void CDlg_DisplayConfiguration::OnCbnSelchangeComboDatasource()
{
	CTLiteDoc* m_pDoc = pView->GetDocument();

		if(m_ComboDataSourceType.GetCurSel()==0)
			m_pDoc->m_PrimaryDataSource = eSimulationData;
		else
			m_pDoc->m_PrimaryDataSource = eSensorData;

	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnCbnSelchangeComboAggregationInterval()
{

	g_MOEAggregationIntervalInMin = aggregation_interval_vector [ m_Combo_Aggregation_Interval.GetCurSel()];

	Invalidate ();
}