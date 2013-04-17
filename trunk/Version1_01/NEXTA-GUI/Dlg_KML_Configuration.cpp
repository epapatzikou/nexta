// Dlg_KML_Configuration.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_KML_Configuration.h"


// CDlg_KML_Configuration dialog


IMPLEMENT_DYNAMIC(CDlg_KML_Configuration, CDialog)

CDlg_KML_Configuration::CDlg_KML_Configuration(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_KML_Configuration::IDD, pParent)
	, m_KML_Height(0)
	, m_Transparency(0)
	, m_BandWidth(0)
{
	m_BandWidth = 20;
	m_KML_Height = 100;
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
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_KML_Height);
	DDV_MinMaxInt(pDX, m_KML_Height, 0, 100000);
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
}


BEGIN_MESSAGE_MAP(CDlg_KML_Configuration, CDialog)
	ON_BN_CLICKED(IDOK, &CDlg_KML_Configuration::OnBnClickedOk)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDlg_KML_Configuration::OnLbnSelchangeList1)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_REVERSE, &CDlg_KML_Configuration::OnBnClickedButtonReverse)
END_MESSAGE_MAP()


// CDlg_KML_Configuration message handlers

void CDlg_KML_Configuration::OnBnClickedButtonSpaceevenly()
{
	// TODO: Add your control notification handler code here
}

void CDlg_KML_Configuration::OnBnClickedOk()
{
	UpdateData(1);
	OnOK();
}
void CDlg_KML_Configuration::UpdateCategoryValues()
{
	m_KML_MOE_selection  = (eKML_ColorMode) m_List.GetCurSel ();

	std::list<DTALink*>::iterator iLink;
	float max_link_volume = 0;

	float max_value = 0;
	float min_value = 999999;

	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{
		DTALink* pLink = (*iLink); 
		float KML_color_value = 0;

		switch (m_KML_MOE_selection)
		{
		case e_KML_number_of_lanes: pLink->KML_color_value  =  pLink->m_NumberOfLanes ; break; 
		case e_KML_speed_limit: pLink->KML_color_value  =  pLink->m_SpeedLimit  ; break; 
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

	
	m_List.AddString ("Number of lanes");
	m_List.AddString ("Speed limit");
	m_List.AddString ("User defined KML attribute in input_link.csv");

	m_List.SetCurSel (0);
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
