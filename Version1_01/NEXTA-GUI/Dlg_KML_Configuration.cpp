//Dlg_KML_Configuration.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "TLiteDoc.h"
#include "TLiteView.h"

#include "Dlg_KML_Configuration.h"


// CDlg_KML_Configuration dialog


IMPLEMENT_DYNAMIC(CDlg_KML_Configuration, CDialog)

CDlg_KML_Configuration::CDlg_KML_Configuration(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_KML_Configuration::IDD, pParent)
	, m_KML_Height_Ratio(0)
	, m_Transparency(0)
	, m_BandWidth(0)
	, m_MaxHeightValue(1)
{
	m_BandWidth = 20;
	m_KML_Height_Ratio = 100;
	m_Transparency = 50;

	for(int i = 0; i <=7; i++)
	{
	m_ColorCategoryValue[i] = i*1000;
	}
}

CDlg_KML_Configuration::~CDlg_KML_Configuration()
{
}

void CDlg_KML_Configuration::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_KML_Height_Ratio);
	DDX_Text(pDX, IDC_EDIT_Transparency, m_Transparency);
	DDV_MinMaxInt(pDX, m_Transparency, 0, 100);
	DDX_Text(pDX, IDC_EDIT_L1, m_ColorCategoryValue[1]);
	DDX_Text(pDX, IDC_EDIT_L2, m_ColorCategoryValue[2]);
	DDX_Text(pDX, IDC_EDIT_L3, m_ColorCategoryValue[3]);
	DDX_Text(pDX, IDC_EDIT_L4, m_ColorCategoryValue[4]);
	DDX_Text(pDX, IDC_EDIT_L5, m_ColorCategoryValue[5]);
	DDX_Text(pDX, IDC_EDIT_L6, m_ColorCategoryValue[6]);
	DDX_Text(pDX, IDC_EDIT_L7, m_ColorCategoryValue[7]);
	DDX_Text(pDX, IDC_EDIT_BandWidth, m_BandWidth);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_BITMAP, m_ColorRampStatic);
	DDX_Control(pDX, IDC_COMBO_Height, m_Height_ComboBox);
	DDX_Text(pDX, IDC_EDIT_HEIGHT_MAX, m_MaxHeightValue);
}


BEGIN_MESSAGE_MAP(CDlg_KML_Configuration, CDialog)
	ON_BN_CLICKED(IDOK, &CDlg_KML_Configuration::OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDlg_KML_Configuration::OnLbnSelchangeList1)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_REVERSE, &CDlg_KML_Configuration::OnBnClickedButtonReverse)
	ON_BN_CLICKED(IDOK3, &CDlg_KML_Configuration::OnBnClickedOk3)
	ON_BN_CLICKED(IDOK4, &CDlg_KML_Configuration::OnBnClickedOk4)
	ON_CBN_SELCHANGE(IDC_COMBO_Height, &CDlg_KML_Configuration::OnCbnSelchangeComboHeight)
	ON_EN_CHANGE(IDC_EDIT_HEIGHT_MAX, &CDlg_KML_Configuration::OnEnChangeEditHeightMax)

END_MESSAGE_MAP()


// CDlg_KML_Configuration message handlers

void CDlg_KML_Configuration::OnBnClickedButtonSpaceevenly()
{
	// TODO: Add your control notification handler code here
}

void CDlg_KML_Configuration::OnBnClickedOk()
{
	OnOK();
}
void CDlg_KML_Configuration::UpdateCategoryValues()
{
	m_KML_MOE_selection  = (eKML_ColorMode) m_List.GetCurSel ();

	std::list<DTALink*>::iterator iLink;
	float max_link_volume = 0;

	float max_value = 0;
	float min_value = 999999;


		CEdit* pEdit1 = (CEdit*)GetDlgItem(IDC_EDIT_L1);
		CEdit* pEdit2 = (CEdit*)GetDlgItem(IDC_EDIT_L2);
		CEdit* pEdit3 = (CEdit*)GetDlgItem(IDC_EDIT_L3);
		CEdit* pEdit4 = (CEdit*)GetDlgItem(IDC_EDIT_L4);
		CEdit* pEdit5 = (CEdit*)GetDlgItem(IDC_EDIT_L5);
		CEdit* pEdit6 = (CEdit*)GetDlgItem(IDC_EDIT_L6);
		CEdit* pEdit7 = (CEdit*)GetDlgItem(IDC_EDIT_L7);

	if(m_KML_MOE_selection == e_KML_green ||
		m_KML_MOE_selection == e_KML_yellow ||
		m_KML_MOE_selection == e_KML_red)
	{

		for(int i = 1; i<=7; i++)
		{
			m_ColorCategoryValue[i] = 0;
		}


		pEdit1->EnableWindow (0);
		pEdit2->EnableWindow (0);
		pEdit3->EnableWindow (0);
		pEdit4->EnableWindow (0);
		pEdit5->EnableWindow (0);
		pEdit6->EnableWindow (0);
		pEdit7->EnableWindow (0);



	}else
	{
		pEdit1->EnableWindow (1);
		pEdit2->EnableWindow (1);
		pEdit3->EnableWindow (1);
		pEdit4->EnableWindow (1);
		pEdit5->EnableWindow (1);
		pEdit6->EnableWindow (1);
		pEdit7->EnableWindow (1);


	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{
		DTALink* pLink = (*iLink); 
		float KML_color_value = 0;

		pLink->KML_single_color_code = -1;

		pLink->KML_color_value  = 0;
		switch (m_KML_MOE_selection)
		{
		case e_KML_green: pLink->KML_single_color_code =  1 ; break; 
		case e_KML_yellow: pLink->KML_single_color_code = 3 ; break; 
		case e_KML_red: pLink->KML_single_color_code = 6 ; break; 

		case e_KML_number_of_lanes: pLink->KML_color_value  =  pLink->m_NumberOfLanes ; break; 
		case e_KML_speed_limit: pLink->KML_color_value  =  pLink->m_SpeedLimit  ; break; 
		case e_KML_avg_speed: pLink->KML_color_value =  pLink->m_avg_simulated_speed; break;
		case e_KML_avg_speed_to_speed_limit_raio: 
			 pLink->KML_color_value =  pLink->m_avg_simulated_speed/max(1,  pLink->m_SpeedLimit); break;

		case e_KML_user_defined_attribue: pLink->KML_color_value  =  pLink->color_value  ; break; 
		}

		max_value = max(max_value, pLink->KML_color_value );
		min_value = min(min_value, pLink->KML_color_value );

	}
	min_value = 1;

	m_ColorCategoryValue[1] = min_value;
	m_ColorCategoryValue[7] = max_value;

	float incremental = (max_value - min_value)/ 6;

	for(int i = 2; i<=6; i++)
	{
		m_ColorCategoryValue[i] = m_ColorCategoryValue[i-1] + incremental;
	}

	}
	UpdateData(0);
}

void CDlg_KML_Configuration::OnLbnSelchangeList1()
{
	UpdateCategoryValues();


//				// show text condition 3: other statistics
//				if((m_ShowLinkTextMode == link_display_link_MOE || m_ShowLinkTextMode == link_display_link_LOS) && (screen_distance > 20 && (
//					pDoc->m_LinkMOEMode == MOE_queue_length 
//					|| pDoc->m_LinkMOEMode == MOE_speed 
//					|| pDoc->m_LinkMOEMode == MOE_volume 
//					|| pDoc->m_LinkMOEMode == MOE_density 
//					|| pDoc->m_LinkMOEMode == MOE_reliability
//					|| pDoc->m_LinkMOEMode == MOE_safety)))
//				{
//
//					float power  = pDoc->GetLinkMOE((*iLink), pDoc->m_LinkMOEMode,(int)g_Simulation_Time_Stamp,g_MOEAggregationIntervalInMin, value);
//					int LOS = pDoc->GetLOSCode(power);
//
//					if( m_ShowLinkTextMode == link_display_link_MOE )
//						str_reference_text.Format ("%.1f",value);
//
//					if( m_ShowLinkTextMode == link_display_link_LOS )
//					{
//						str_reference_text.Format ("%d",LOS);
//					}
//
//--------------

				//case link_display_crash_prediction_group_1_code:

				//	if((*iLink)->group_1_code.size ()>=1)
				//		str_text.Format ("%s",(*iLink)->group_1_code.c_str ()  );
				//	break;

				//case link_display_crash_prediction_group_2_code:
				//	if((*iLink)->group_2_code.size ()>=1)
				//		str_text.Format ("%s",(*iLink)->group_2_code.c_str ()  );
				//	break;

				//case link_display_crash_prediction_group_3_code:
				//	if((*iLink)->group_3_code.size ()>=1)
				//		str_text.Format ("%s",(*iLink)->group_3_code.c_str ()  );
				//	break;

				//case link_display_number_of_crashes:
				//	if((*iLink)->m_number_of_all_crashes  >=0.00001)
				//		str_text.Format ("%.2f",(*iLink)->m_number_of_all_crashes   );
				//	break;

				//case link_display_num_of_fatal_and_injury_crashes_per_year:
				//	if((*iLink)->m_number_of_all_crashes  >=0.00001)
				//		str_text.Format ("%.2f",(*iLink)->m_num_of_fatal_and_injury_crashes_per_year   ); break;

				//case link_display_num_of_PDO_crashes_per_year:
				//	if((*iLink)->m_number_of_all_crashes  >=0.00001)
				//		str_text.Format ("%.2f",(*iLink)->m_num_of_PDO_crashes_per_year   ); break;

				//case link_display_number_of_intersection_crashes:
				//	if((*iLink)->m_number_of_intersection_crashes  >=0.00001)
				//		str_text.Format ("%.2f",(*iLink)->m_number_of_intersection_crashes   );
				//	break;

				//case link_display_num_of_intersection_fatal_and_injury_crashes_per_year:
				//	if((*iLink)->m_number_of_intersection_crashes  >=0.00001)
				//		str_text.Format ("%.2f",(*iLink)->m_num_of_intersection_fatal_and_injury_crashes_per_year   ); break;

				//case link_display_num_of_intersection_PDO_crashes_per_year:
				//	if((*iLink)->m_number_of_intersection_crashes  >=0.00001)
				//		str_text.Format ("%.2f",(*iLink)->m_num_of_intersection_PDO_crashes_per_year   ); break;

				//case link_display_total_link_volume:
				//	if((*iLink)->m_total_link_volume >=1)
				//		str_text.Format ("%.0f",(*iLink)->m_total_link_volume   );

				//	break;

				//case link_display_avg_travel_time:
				//	str_text.Format ("%.1f",(*iLink)->GetTravelTime(pDoc->m_DemandLoadingStartTimeInMin , pDoc->m_DemandLoadingEndTimeInMin)  );
				//break;


				//case link_display_Num_Driveways_Per_Mile:
				//	str_text.Format ("%.1f",(*iLink)->m_Num_Driveways_Per_Mile    ); break;

				//case link_display_Num_3SG_Intersections:
				//	str_text.Format ("%.1f",(*iLink)->m_Num_3SG_Intersections    ); break;

				//case link_display_Num_3ST_Intersections:
				//	str_text.Format ("%.1f",(*iLink)->m_Num_3ST_Intersections    ); break;

				//case link_display_Num_4SG_Intersections:
				//	str_text.Format ("%.1f",(*iLink)->m_Num_4SG_Intersections    ); break;

				//case link_display_Num_4ST_Intersections:
				//	str_text.Format ("%.1f",(*iLink)->m_Num_4ST_Intersections    ); break;

				//}
//
//				case link_display_lane_capacity_per_hour:
//					str_text.Format ("%.0f",(*iLink)->m_LaneCapacity); break;
//
//				case link_display_link_capacity_per_hour:
//					str_text.Format ("%.0f",(*iLink)->m_LaneCapacity*(*iLink)->m_NumberOfLanes  ); break;
//				case link_display_link_grade:
//					str_text.Format ("%.2f",(*iLink)->m_Grade  ); break;
//
//				case link_display_jam_density_in_vhc_pmpl:
//					str_text.Format ("%.0f",(*iLink)->m_Kjam   ); break;
//
//
//				case link_display_volume_over_capacity_ratio:
//					str_text.Format ("%.2f",(*iLink)->m_volume_over_capacity_ratio    ); break;
//
//				case link_display_LevelOfService:
//					str_text.Format ("%c",(*iLink)->m_LevelOfService    ); break;
//
//				case link_display_total_sensor_link_volume:
//
//					if((*iLink)->m_total_sensor_link_volume >= 1)
//						str_text.Format ("%.0f",(*iLink)->m_total_sensor_link_volume     );
//					break;
//
//				case link_display_total_link_count_error:
//					if((*iLink)->m_total_link_count_error >=1)
//						str_text.Format ("%.0f",(*iLink)->m_total_link_count_error    ); 
//
//					break;
//
//				case link_display_simulated_AADT:
//					if((*iLink)->m_simulated_AADT >=1)
//						str_text.Format ("%.0f",(*iLink)->m_simulated_AADT     ); 


}

BOOL CDlg_KML_Configuration::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Height_ComboBox.AddString ("Default Height of 1");

	m_Height_ComboBox.AddString("Time-dependent Link MOE");
	m_Height_ComboBox.AddString("Time-dependent Link LOS Value");
	m_Height_ComboBox.AddString("--");
	m_Height_ComboBox.AddString("--");
	m_Height_ComboBox.AddString("--");
	m_Height_ComboBox.AddString("--");
	m_Height_ComboBox.AddString("Speed Limit (mph)");
	m_Height_ComboBox.AddString("Speed Limit (km/hour)");
	m_Height_ComboBox.AddString("Length (mile)");
	m_Height_ComboBox.AddString("Length (feet)");
	m_Height_ComboBox.AddString("Length (km)");
	m_Height_ComboBox.AddString("Length (meter)");
	m_Height_ComboBox.AddString("Free Flow Travel Time (min)");
	m_Height_ComboBox.AddString("Free Flow Travel Time (hour)");
	m_Height_ComboBox.AddString("# of Lanes");
	m_Height_ComboBox.AddString("Link Capacity Per Hour");
	m_Height_ComboBox.AddString("Lane Capacity Per Hour");

	m_Height_ComboBox.AddString("-- Additional Static Attributes --");
	m_Height_ComboBox.AddString("Saturation Flow Rate");
	m_Height_ComboBox.AddString("Effective Green Time Length In Second");
	m_Height_ComboBox.AddString("Effective Green Time Length In Second (Positive Number Only)");
	m_Height_ComboBox.AddString("Green Start Time In Second");
	m_Height_ComboBox.AddString("Grade");
	m_Height_ComboBox.AddString("Jam Density In vhc pmpl");
	m_Height_ComboBox.AddString("Wave Speed In mph");

	m_Height_ComboBox.AddString("BPR alpha term");
	m_Height_ComboBox.AddString("BPR beta term");

	m_Height_ComboBox.AddString("--");
	m_Height_ComboBox.AddString("Link type In Number");
	m_Height_ComboBox.AddString("--");

	m_Height_ComboBox.AddString("-- Safety Related Attributes --");
	m_Height_ComboBox.AddString("Number of Driveways Per Mile");
	m_Height_ComboBox.AddString("Volume Proportion on Minor Leg");
	m_Height_ComboBox.AddString("Number of 3SG Intersections");
	m_Height_ComboBox.AddString("Number of 3ST Intersections");
	m_Height_ComboBox.AddString("Number of 4SG Intersections");
	m_Height_ComboBox.AddString("Number of 4ST Intersections");

	m_Height_ComboBox.AddString("-- Simulation/Assignment Results --");
	m_Height_ComboBox.AddString("Total Link Volume");
	m_Height_ComboBox.AddString("Avg Travel Time (min)");
	m_Height_ComboBox.AddString("Volume over Capacity Ratio");
	m_Height_ComboBox.AddString("Level Of Service");
	m_Height_ComboBox.AddString("Avg Waiting Time on Loading Buffer");
	m_Height_ComboBox.AddString("Avg Simulated Speed");
	m_Height_ComboBox.AddString("-- Observations --");
	m_Height_ComboBox.AddString("Total Sensor Link Volume");
	m_Height_ComboBox.AddString("Total Link Count Error");
	m_Height_ComboBox.AddString("Simulated AADT");
	m_Height_ComboBox.AddString("Observed Bidirectional AADT");
	m_Height_ComboBox.AddString("Observed Peak Hour Volume");

	m_Height_ComboBox.AddString("-- Safety Prediction Results --");
	m_Height_ComboBox.AddString("--");
	m_Height_ComboBox.AddString("--");
	m_Height_ComboBox.AddString("--");

	m_Height_ComboBox.AddString("Number of Crashes Per Year");
	m_Height_ComboBox.AddString("Number of Fatal and Injury Crashes Per_Year");
	m_Height_ComboBox.AddString("Number of PDO Crashes Per Year");

	m_Height_ComboBox.AddString("Number of Intersection Crashes Per Year");
	m_Height_ComboBox.AddString("Number of Intersection Fatal and Injury Crashes Per_Year");
	m_Height_ComboBox.AddString("Number of Intersection PDO Crashes Per Year");

	m_Height_ComboBox.SetCurSel(0);

	m_List.AddString ("Green");
	m_List.AddString ("Yellow");
	m_List.AddString ("Red");
	m_List.AddString ("Number of lanes");
	m_List.AddString ("Speed limit");
	m_List.AddString ("Avg Speed");
	m_List.AddString ("Avg Speed to Speed Limit Ratio");
	m_List.AddString ("Average Speed");

	m_List.AddString ("User defined KML attribute in input_link.csv");

	m_List.SetCurSel (3);

	UpdateCategoryValues();
	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlg_KML_Configuration::OnPaint()
{
   CPaintDC dc(this); 
   CPen penLOS[MAX_LOS_SIZE];
   CBrush brushLOS[MAX_LOS_SIZE];

   int i;

   if(m_pDoc==NULL)
	   return;
   for(i = 1; i<MAX_LOS_SIZE-1; i++)
   {
      penLOS[i].CreatePen(PS_SOLID, 1, m_pDoc->m_colorLOS[i]);
      brushLOS[i].CreateSolidBrush(m_pDoc->m_colorLOS[i]);
   }

   dc.SetBkMode(TRANSPARENT);

    CRect entire_rect;

	m_ColorRampStatic.GetWindowRect(&entire_rect);
	ScreenToClient(&entire_rect); 

   
   CRect lr;

   float height = entire_rect.Height () / 6.0;

	  for(i = 1; i< MAX_LOS_SIZE-1; i++)
   {
      lr.top = entire_rect.top +height*(i-1) ;
      lr.bottom = lr.top + height;

      lr.left = entire_rect.left;
      lr.right = entire_rect.right; 

	  if(m_pDoc->m_ColorDirection==-1)  // reverse direction 
	  {
	  dc.SelectObject(penLOS[MAX_LOS_SIZE-1-i]);
      dc.SelectObject(brushLOS[MAX_LOS_SIZE-1-i]);
	  }else
	  {
	  dc.SelectObject(penLOS[i]);
      dc.SelectObject(brushLOS[i]);
  	  }

	  CString lengend_interval_str;
      dc.Rectangle(lr);
	}

}

void CDlg_KML_Configuration::OnBnClickedButtonReverse()
{
	if(m_pDoc->m_ColorDirection == 1)
		m_pDoc->m_ColorDirection = -1;
	else
		m_pDoc->m_ColorDirection = 1;

	Invalidate();
}

void CDlg_KML_Configuration::SelchangeComboHeight()
{
	float max_value = 0;

	link_text_display_mode HeightSelection = (link_text_display_mode) m_Height_ComboBox.GetCurSel();
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{
		DTALink* pLink = (*iLink); 

				pLink->m_UserDefinedHeight  =  1 ;

				CString str_text;
				switch (HeightSelection)
				{
				case  link_display_speed_limit_in_miles:
					str_text.Format ("%.1f",(*iLink)->m_SpeedLimit ); break;
				case link_display_length_in_miles:
					str_text.Format ("%.3f",(*iLink)->m_Length  ); break;
				case link_display_length_in_feet:
					str_text.Format ("%.0f",(*iLink)->m_Length*5280  ); break;
				case link_display_length_in_meters:
					str_text.Format ("%.0f",(*iLink)->m_Length*1.60934*1000  ); break;
				case  link_display_speed_limit_in_km:
					str_text.Format ("%.1f",(*iLink)->m_SpeedLimit*1.60934 ); break;
				case link_display_length_in_km:
					str_text.Format ("%.3f",(*iLink)->m_Length*1.60934  ); break;
				case  link_display_free_flow_travel_time_in_min:
					str_text.Format ("%.2f",(*iLink)->m_FreeFlowTravelTime   ); break;
				case  link_display_free_flow_travel_time_in_hour:
					str_text.Format ("%.3f",(*iLink)->m_FreeFlowTravelTime/60.0   ); break;

				case link_display_saturation_flow_rate:
					str_text.Format ("%.0f",(*iLink)->m_Saturation_flow_rate_in_vhc_per_hour_per_lane); break;
				case link_display_number_of_lanes:
					str_text.Format ("%d",(*iLink)->m_NumberOfLanes ); break;						
				case link_display_lane_capacity_per_hour:
					str_text.Format ("%.0f",(*iLink)->m_LaneCapacity); break;

				case link_display_link_capacity_per_hour:
					str_text.Format ("%.0f",(*iLink)->m_LaneCapacity*(*iLink)->m_NumberOfLanes  ); break;

				case link_display_effective_green_time_length_in_second:
					str_text.Format ("%d",(*iLink)->m_EffectiveGreenTimeInSecond  ); break;

				case link_display_BPR_alpha_term:
					str_text.Format ("%.4f",(*iLink)->m_BPR_alpha_term   ); break;

				case link_display_BPR_beta_term:
					str_text.Format ("%.4f",(*iLink)->m_BPR_beta_term    ); break;

				case link_display_effective_green_time_length_in_second_positive_number_only:

					if((*iLink)->m_EffectiveGreenTimeInSecond>0)
						str_text.Format ("%d",(*iLink)->m_EffectiveGreenTimeInSecond  ); 

					break;

				case link_display_green_start_time_in_second:
					str_text.Format ("%d",(*iLink)->m_GreenStartTimetInSecond  ); break;

				case link_display_link_grade:
					str_text.Format ("%.2f",(*iLink)->m_Grade  ); break;

				case link_display_jam_density_in_vhc_pmpl:
					str_text.Format ("%.0f",(*iLink)->m_Kjam   ); break;

				case link_display_wave_speed_in_mph:
					str_text.Format ("%.0f",(*iLink)->m_Wave_speed_in_mph  ); break;

				case link_display_link_type_in_number:
					str_text.Format ("%d", (*iLink)->m_link_type  ); break;

				case link_display_internal_link_id:
					str_text.Format ("%d",(*iLink)->m_LinkNo   ); break;

				case link_display_volume_over_capacity_ratio:
					str_text.Format ("%.2f",(*iLink)->m_volume_over_capacity_ratio    ); break;

				case link_display_avg_waiting_time_on_loading_buffer:
					if( (*iLink)->m_avg_waiting_time_on_loading_buffer > 0.1)
						str_text.Format ("%.1f",(*iLink)->m_avg_waiting_time_on_loading_buffer   ); break;

				case link_display_avg_simulated_speed:
					str_text.Format ("%.0f",(*iLink)->m_avg_simulated_speed    ); break;

				case link_display_total_sensor_link_volume:

					if((*iLink)->m_total_sensor_link_volume >= 1)
						str_text.Format ("%.0f",(*iLink)->m_total_sensor_link_volume     );
					break;

				case link_display_total_link_count_error:
					if((*iLink)->m_total_link_count_error >=1)
						str_text.Format ("%.0f",(*iLink)->m_total_link_count_error    ); 

					break;

				case link_display_simulated_AADT:
					if((*iLink)->m_simulated_AADT >=1)
						str_text.Format ("%.0f",(*iLink)->m_simulated_AADT     ); 
					break;

				case link_display_observed_AADT:
					if((*iLink)->m_observed_AADT >=1)
						str_text.Format ("%d",(*iLink)->m_observed_AADT      ); 
					break;

				case link_display_observed_peak_hourly_volume:
					if((*iLink)->m_observed_peak_hourly_volume >=1)
						str_text.Format ("%d",(*iLink)->m_observed_peak_hourly_volume     ); 
					break;

				case link_display_number_of_crashes:
					if((*iLink)->m_number_of_all_crashes  >=0.00001)
						str_text.Format ("%.2f",(*iLink)->m_number_of_all_crashes   );
					break;

				case link_display_num_of_fatal_and_injury_crashes_per_year:
					if((*iLink)->m_number_of_all_crashes  >=0.00001)
						str_text.Format ("%.2f",(*iLink)->m_num_of_fatal_and_injury_crashes_per_year   ); break;

				case link_display_num_of_PDO_crashes_per_year:
					if((*iLink)->m_number_of_all_crashes  >=0.00001)
						str_text.Format ("%.2f",(*iLink)->m_num_of_PDO_crashes_per_year   ); break;

				case link_display_number_of_intersection_crashes:
					if((*iLink)->m_number_of_intersection_crashes  >=0.00001)
						str_text.Format ("%.2f",(*iLink)->m_number_of_intersection_crashes   );
					break;

				case link_display_num_of_intersection_fatal_and_injury_crashes_per_year:
					if((*iLink)->m_number_of_intersection_crashes  >=0.00001)
						str_text.Format ("%.2f",(*iLink)->m_num_of_intersection_fatal_and_injury_crashes_per_year   ); break;

				case link_display_num_of_intersection_PDO_crashes_per_year:
					if((*iLink)->m_number_of_intersection_crashes  >=0.00001)
						str_text.Format ("%.2f",(*iLink)->m_num_of_intersection_PDO_crashes_per_year   ); break;

				case link_display_total_link_volume:
					if((*iLink)->m_total_link_volume >=1)
						str_text.Format ("%.0f",(*iLink)->m_total_link_volume   );

					break;

				case link_display_avg_travel_time:
					str_text.Format ("%.1f",(*iLink)->GetTravelTime(m_pDoc->m_DemandLoadingStartTimeInMin , m_pDoc->m_DemandLoadingEndTimeInMin)  );
				break;


				case link_display_Num_Driveways_Per_Mile:
					str_text.Format ("%.1f",(*iLink)->m_Num_Driveways_Per_Mile    ); break;

				case link_display_Num_3SG_Intersections:
					str_text.Format ("%.1f",(*iLink)->m_Num_3SG_Intersections    ); break;

				case link_display_Num_3ST_Intersections:
					str_text.Format ("%.1f",(*iLink)->m_Num_3ST_Intersections    ); break;

				case link_display_Num_4SG_Intersections:
					str_text.Format ("%.1f",(*iLink)->m_Num_4SG_Intersections    ); break;

				case link_display_Num_4ST_Intersections:
					str_text.Format ("%.1f",(*iLink)->m_Num_4ST_Intersections    ); break;

				}

				if(str_text.GetLength () >=1)
				{
				float value  = atof(str_text);
				pLink->m_UserDefinedHeight  =  value ;

				if(max_value < value)
					max_value = value;
				}


	}

		m_MaxHeightValue = max_value;
		m_KML_Height_Ratio  = 100/max(1,max_value);
		UpdateData(0);

}



void CDlg_KML_Configuration::OnBnClickedOk3()
{
	UpdateData(1);

	CString directory  = m_pDoc->m_ProjectDirectory ;

	CString KML_Link_3D_File = directory+"AMS_link.kml";
	DeleteFile(KML_Link_3D_File);
	m_pDoc->ExportLinkSingleAttributeLayerToKMLFiles(KML_Link_3D_File,"LIBKML",m_BandWidth, m_Transparency,m_ColorCategoryValue, m_KML_Height_Ratio);
	HINSTANCE result = ShellExecute(NULL, _T("open"), KML_Link_3D_File, NULL,NULL, SW_SHOW);

}


void CDlg_KML_Configuration::OnBnClickedOk4()
{
	UpdateData(1);

	CString directory  = m_pDoc->m_ProjectDirectory ;
	DeleteFile(directory+"AMS_link.shp");
	m_pDoc->ExportLinkLayerToGISFiles(directory+"AMS_link.shp","ESRI Shapefile");
	CString SHP_Link_File = directory+"AMS_link.shp";

	HINSTANCE result = ShellExecute(NULL, _T("open"), SHP_Link_File, NULL,NULL, SW_SHOW);

}

void CDlg_KML_Configuration::OnCbnSelchangeComboHeight()
{
	SelchangeComboHeight();

}

void CDlg_KML_Configuration::OnEnChangeEditHeightMax()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

