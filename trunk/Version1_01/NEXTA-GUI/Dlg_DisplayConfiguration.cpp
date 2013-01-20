// Dlg_DisplayConfiguration.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_DisplayConfiguration.h"


// CDlg_DisplayConfiguration dialog

IMPLEMENT_DYNAMIC(CDlg_DisplayConfiguration, CBaseDialog)

CDlg_DisplayConfiguration::CDlg_DisplayConfiguration(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlg_DisplayConfiguration::IDD, pParent)
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
	DDX_Control(pDX, IDC_LIST_ZONE_TEXT_LABEL, m_Zone_Label);
	DDX_Control(pDX, IDC_LIST_NODE_TEXT_LABEL, m_Node_Label);
	DDX_Control(pDX, IDC_MOE_AGGREGATION_INTERVAL_LIST, m_AggregationIntervalList);
	DDX_Control(pDX, IDC_LIST_MOVEMENT_TEXT_LABEL, m_Movement_Label);
	DDX_Control(pDX, IDC_LIST_GPS_TEXT_LABEL, m_GPS_Label);
	DDX_Control(pDX, IDC_COMBO_MovementTextBoxSize, m_ComboBox_MovementTextBoxSize);
	DDX_Control(pDX, IDC_LIST_Size_Text_Control, m_SizeTextControl_List);
	DDX_Control(pDX, IDC_COMBO_WalkingDistance, m_ComboxBox_WalkingDistance);
	DDX_Control(pDX, IDC_COMBO_TransitTime, m_ComboxBox_TransitTime);
}


BEGIN_MESSAGE_MAP(CDlg_DisplayConfiguration, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_LINK_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListLinkTextLabel)
	ON_LBN_SELCHANGE(IDC_LIST_NODE_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListNodeTextLabel)
	ON_LBN_SELCHANGE(IDC_MOE_AGGREGATION_INTERVAL_LIST, &CDlg_DisplayConfiguration::OnLbnSelchangeMoeAggregationIntervalList)
	ON_LBN_SELCHANGE(IDC_LIST_ZONE_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListZoneTextLabel)
	ON_BN_CLICKED(IDOK, &CDlg_DisplayConfiguration::OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_LIST_MOVEMENT_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListMovementTextLabel)
	ON_LBN_SELCHANGE(IDC_LIST_GPS_TEXT_LABEL, &CDlg_DisplayConfiguration::OnLbnSelchangeListGpsTextLabel)
	ON_CBN_SELCHANGE(IDC_COMBO_MovementTextBoxSize, &CDlg_DisplayConfiguration::OnCbnSelchangeComboMovementtextboxsize)
	ON_LBN_DBLCLK(IDC_LIST_Size_Text_Control, &CDlg_DisplayConfiguration::OnLbnDblclkListSizeTextControl)
	ON_CBN_SELCHANGE(IDC_COMBO_WalkingDistance, &CDlg_DisplayConfiguration::OnCbnSelchangeComboWalkingdistance)
	ON_CBN_SELCHANGE(IDC_COMBO_TransitTime, &CDlg_DisplayConfiguration::OnCbnSelchangeComboTransittime)
	ON_LBN_SELCHANGE(IDC_LIST_Size_Text_Control, &CDlg_DisplayConfiguration::OnLbnSelchangeListSizeTextControl)
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
	m_Node_Label.AddString("Zone ID of Activity Location");
	m_Node_Label.AddString("Cycle Length In Second");
	m_Node_Label.AddString("Cycle Length for Signals only");
	m_Node_Label.AddString("Offset In Second for Signals only");
	m_Node_Label.AddString("Intersection Name");
	m_Node_Label.AddString("Control Type");


	m_Node_Label.SetCurSel ((int)(m_ShowNodeTextMode));

	m_Link_Label.AddString("None");
	m_Link_Label.AddString("Time-dependent Link MOE");
	m_Link_Label.AddString("Time-dependent Link LOS Value");
	m_Link_Label.AddString("Street Name");
	m_Link_Label.AddString("Link ID");
	m_Link_Label.AddString("TMC Code");
	m_Link_Label.AddString("From ID -> To ID");
	m_Link_Label.AddString("Speed Limit (mph)");
	m_Link_Label.AddString("Speed Limit (km/hour)");
	m_Link_Label.AddString("Length (mile)");
	m_Link_Label.AddString("Length (km)");
	m_Link_Label.AddString("Free Flow Travel Time (min)");
	m_Link_Label.AddString("Free Flow Travel Time (hour)");
	m_Link_Label.AddString("# of Lanes");
	m_Link_Label.AddString("Link Capacity Per Hour");
	m_Link_Label.AddString("Lane Capacity Per Hour");

	m_Link_Label.AddString("-- Additional Static Attributes --");
	m_Link_Label.AddString("Saturation Flow Rate");
	m_Link_Label.AddString("Effective Green Time Length In Second");
	m_Link_Label.AddString("Effective Green Time Length In Second (Positive Number Only)");
	m_Link_Label.AddString("Green Start Time In Second");
	m_Link_Label.AddString("Grade");
	m_Link_Label.AddString("Jam Density In vhc pmpl");
	m_Link_Label.AddString("Wave Speed In mph");
	m_Link_Label.AddString("Link type In Text");
	m_Link_Label.AddString("Link type In Number");
	m_Link_Label.AddString("Internal Link id");

	m_Link_Label.AddString("-- Safety Related Attributes --");
	m_Link_Label.AddString("Number of Driveways Per Mile");
	m_Link_Label.AddString("Volume Proportion on Minor Leg");
	m_Link_Label.AddString("Number of 3SG Intersections");
	m_Link_Label.AddString("Number of 3ST Intersections");
	m_Link_Label.AddString("Number of 4SG Intersections");
	m_Link_Label.AddString("Number of 4ST Intersections");

	m_Link_Label.AddString("-- Simulation/Assignment Results --");
	m_Link_Label.AddString("Total Link Volume");
	m_Link_Label.AddString("Volume over Capacity Ratio");
	m_Link_Label.AddString("Level Of Service");
	m_Link_Label.AddString("Avg Waiting Time on Loading Buffer");
	m_Link_Label.AddString("Avg Simulated Speed");
	m_Link_Label.AddString("Total Sensor Link Volume");
	m_Link_Label.AddString("Total Link Count Error");
	m_Link_Label.AddString("simulated AADT");

	m_Link_Label.AddString("-- Safety Prediction Results --");
	m_Link_Label.AddString("Group 1 Code");
	m_Link_Label.AddString("Group 2 Code");
	m_Link_Label.AddString("Group 3 Code");


	m_Link_Label.AddString("Number of Crashes Per Year");
	m_Link_Label.AddString("Number of Fatal and Injury Crashes Per_Year");
	m_Link_Label.AddString("Number of PDO Crashes Per Year");

	m_Link_Label.AddString("Number of Intersection Crashes Per Year");
	m_Link_Label.AddString("Number of Intersection Fatal and Injury Crashes Per_Year");
	m_Link_Label.AddString("Number of Intersection PDO Crashes Per Year");

	m_Link_Label.SetCurSel ((int)(pView->m_ShowLinkTextMode));

	m_Movement_Label.AddString("None");
	m_Movement_Label.AddString ("Turn Type");
	m_Movement_Label.AddString ("Upstream Node Number");
	m_Movement_Label.AddString ("Downtream Node Number");
	m_Movement_Label.AddString ("Up,Current,Dest Node Numbers");
	m_Movement_Label.AddString ("Protected//Permited//Prohibitted");

	m_Movement_Label.AddString ("Simulated Total Count");
	m_Movement_Label.AddString ("Simulated Hourly Count");
	m_Movement_Label.AddString ("Simulated Turning %");
	m_Movement_Label.AddString ("Simulated Turn Delay");

	m_Movement_Label.AddString ("Observed Total Count");
	m_Movement_Label.AddString ("Observed Hourly Count");
	m_Movement_Label.AddString ("Observed Turning %");
	m_Movement_Label.AddString ("Observed Turn Delay");

	m_Movement_Label.AddString ("QEM Turn Direction");
	m_Movement_Label.AddString ("QEM # of Lanes");
	m_Movement_Label.AddString ("QEM Shared Lane Flag");
	m_Movement_Label.AddString ("QEM Lane Width");
	m_Movement_Label.AddString ("QEM Storage");
	m_Movement_Label.AddString ("QEM StLanes");
	m_Movement_Label.AddString ("QEM Grade");
	m_Movement_Label.AddString ("QEM Speed");

	m_Movement_Label.AddString ("QEM Ideal Flow");
	m_Movement_Label.AddString ("QEM Lost Time");
	m_Movement_Label.AddString ("QEM Phase1");
	m_Movement_Label.AddString ("QEM PermPhase1");
	m_Movement_Label.AddString ("QEM DetectPhase1");

	m_Movement_Label.AddString ("QEM Volume");
	m_Movement_Label.AddString ("QEM Turning %");

	m_Movement_Label.AddString ("QEM Effective Green");
	m_Movement_Label.AddString ("QEM Capacity");
	m_Movement_Label.AddString ("QEM VOC");
	m_Movement_Label.AddString ("QEM DischargeRate");
	m_Movement_Label.AddString ("QEM Delay");
	m_Movement_Label.AddString ("QEM LOS");

	m_Movement_Label.SetCurSel ((int)(pView->m_ShowMovementTextMode));

	m_GPS_Label.AddString("None");
	m_GPS_Label.AddString("Vehicle ID");
	m_GPS_Label.AddString("Timestamp in min");
	m_GPS_Label.AddString("Time Gap in min");
	m_GPS_Label.AddString("GPS Speed (mph)");
	m_GPS_Label.AddString("All Trajectories");
	m_GPS_Label.SetCurSel ((int)(pView->m_ShowGPSTextMode));


	movement_text_size_vector.push_back(100);
	movement_text_size_vector.push_back(150);
	movement_text_size_vector.push_back(200);
	movement_text_size_vector.push_back(250);
	movement_text_size_vector.push_back(300);
	movement_text_size_vector.push_back(500);
	movement_text_size_vector.push_back(1000);
	movement_text_size_vector.push_back(2000);
	movement_text_size_vector.push_back(5000);

	for(unsigned int i=0; i< movement_text_size_vector.size(); i++)
	{
		CString str;
		str.Format("%d",movement_text_size_vector[i]);
		m_ComboBox_MovementTextBoxSize.AddString(str);

		if(movement_text_size_vector[i] == pView->m_MovementTextBoxSizeInFeet )
		{
		m_ComboBox_MovementTextBoxSize.SetCurSel (i);
		}
	}

	walking_distance_vector.push_back(0.25);
	walking_distance_vector.push_back(0.5);
	walking_distance_vector.push_back(0.75);
	walking_distance_vector.push_back(1);
	walking_distance_vector.push_back(1.25);
	walking_distance_vector.push_back(1.5);
	walking_distance_vector.push_back(2);
	walking_distance_vector.push_back(3);
	walking_distance_vector.push_back(4);
	walking_distance_vector.push_back(5);
	walking_distance_vector.push_back(10);

	for(unsigned int i=0; i< walking_distance_vector.size(); i++)
	{
		CString str;
		str.Format("%.1f",walking_distance_vector[i]);
		m_ComboxBox_WalkingDistance.AddString(str);

		if(walking_distance_vector[i] == pView->GetDocument ()->m_max_walking_distance )
		{
		m_ComboxBox_WalkingDistance.SetCurSel (i);
		}
	}
	
	transit_time_vector.push_back(15);
	transit_time_vector.push_back(30);
	transit_time_vector.push_back(45);
	transit_time_vector.push_back(60);
	transit_time_vector.push_back(75);
	transit_time_vector.push_back(90);
	transit_time_vector.push_back(120);
	
	for(unsigned int i=0; i< transit_time_vector.size(); i++)
	{
		CString str;
		str.Format("%d",transit_time_vector[i]);
		m_ComboxBox_TransitTime.AddString(str);

		if(transit_time_vector[i] == pView->GetDocument ()->m_max_accessible_transit_time_in_min   )
		{
		m_ComboxBox_TransitTime.SetCurSel (i);
		}
	}
	

	m_AggregationValueVector.push_back(1);
	m_AggregationValueVector.push_back(5);
	m_AggregationValueVector.push_back(15);
	m_AggregationValueVector.push_back(30);
	m_AggregationValueVector.push_back(60);
	m_AggregationValueVector.push_back(120);
	m_AggregationValueVector.push_back(1440);

	for(unsigned int i = 0;  i< m_AggregationValueVector.size (); i++)
	{
		CString str;
		str.Format("%d min",m_AggregationValueVector[i]);

		m_AggregationIntervalList.AddString (str);

		if(g_MOEAggregationIntervalInMin  == m_AggregationValueVector[i])
		{
		m_AggregationIntervalList.SetCurSel (i);
	
		}
	
	}

	m_SizeTextControl_List.AddString ("Increase Node Size (Pg Up)");
	m_SizeTextControl_List.AddString ("Decrease Node Size (Pg Up)");
	m_SizeTextControl_List.AddString ("Increase Node Text Size");
	m_SizeTextControl_List.AddString ("Decrease Node Text Size");
	m_SizeTextControl_List.AddString ("Increase Link/Movement Text Size");
	m_SizeTextControl_List.AddString ("Decrease Link/Movement Text Size");

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

void CDlg_DisplayConfiguration::OnCbnSelchangeComboMovementtextboxsize()
{
	pView->m_MovementTextBoxSizeInFeet =  movement_text_size_vector [ m_ComboBox_MovementTextBoxSize.GetCurSel()];
	pView->Invalidate ();

}

void CDlg_DisplayConfiguration::OnBnClickedButtonIncreasenodesize()
{
	CTLiteDoc* pDoc = pView->GetDocument();

	pDoc->m_NodeDisplaySize *=1.2;
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnBnClickedButtonDecreasenodesize()
{
	CTLiteDoc* pDoc = pView->GetDocument();

	pDoc->m_NodeDisplaySize /=1.2;
	pView->Invalidate ();

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
	 pView->GetDocument ()->m_max_walking_distance  = walking_distance_vector [ m_ComboxBox_WalkingDistance.GetCurSel()];
	 pView->FindAccessibleTripIDWithCurrentMousePoint();
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnCbnSelchangeComboTransittime()
{
	 pView->GetDocument ()->m_max_accessible_transit_time_in_min = transit_time_vector [ m_ComboxBox_TransitTime.GetCurSel()];
	pView->Invalidate ();
}

void CDlg_DisplayConfiguration::OnLbnSelchangeListSizeTextControl()
{
	// TODO: Add your control notification handler code here
}
