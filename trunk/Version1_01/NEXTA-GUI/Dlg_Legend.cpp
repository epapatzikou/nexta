// Dlg_Legend.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_Legend.h"


// CDlg_Legend dialog

IMPLEMENT_DYNAMIC(CDlg_Legend, CBaseDialog)

CDlg_Legend::CDlg_Legend(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlg_Legend::IDD, pParent)
{

}

CDlg_Legend::~CDlg_Legend()
{
}

void CDlg_Legend::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_EMISSIONTYPE, m_ComboBox_EmissionType);
}


BEGIN_MESSAGE_MAP(CDlg_Legend, CBaseDialog)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_COMBO_EMISSIONTYPE, &CDlg_Legend::OnCbnSelchangeComboEmissiontype)
END_MESSAGE_MAP()


// CDlg_Legend message handlers
BOOL CDlg_Legend::OnInitDialog()
{
     CBaseDialog::OnInitDialog();

	 m_ComboBox_EmissionType.AddString ("Energy");
	 m_ComboBox_EmissionType.AddString ("CO2");
	 m_ComboBox_EmissionType.AddString ("NOX");
	 m_ComboBox_EmissionType.AddString ("CO");
	 m_ComboBox_EmissionType.AddString ("HC");

	 m_ComboBox_EmissionType.SetCurSel (0);

     m_ComboBox_EmissionType.ShowWindow (0); // not showing


     // Calculate your x and y coordinates of the upper-left corner 
     // of the dialog (in screen coordinates) based on some logic

     SetWindowPos(NULL, 200, 150, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

     return TRUE;
}
void CDlg_Legend::OnPaint()
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


   SetBackgroundColor(RGB(255, 255, 255));



   if(m_pDoc->m_LinkMOEMode == MOE_emissions)
 	 m_ComboBox_EmissionType.ShowWindow (1);
   else
   m_ComboBox_EmissionType.ShowWindow (0); // not showing


 switch(m_pDoc->m_LinkMOEMode )
 {
 case MOE_speed: SetWindowText("% of Speed Limit"); break;
 case MOE_density: SetWindowText("Density (vhc/mile/ln)"); break;
 case MOE_reliability: SetWindowText("Variability Ratio"); break;
 case MOE_emissions: 
	 
	 switch (m_pDoc->m_EmissionType )
	 {
	 case  DTA_Energy:  SetWindowText("Avg Energy (J) Per Mile"); break;	 
	 case  DTA_CO2:  SetWindowText("Avg CO2 (g) Per Mile"); break;	 
	 case  DTA_NOX:  SetWindowText("Avg NOX (g) Per Mile"); break;	 
	 case  DTA_CO:  SetWindowText("Avg CO (g) Per Mile"); break;	 
	 case  DTA_HC:  SetWindowText("Avg HC (g) er Mile"); break;	 
	 default:  SetWindowText("Avg Energy (J) Per Mile"); 
	 }
	 

	 break;
 case MOE_safety: SetWindowText("Annual Crash Frequency (per mile)"); break;
 default:  SetWindowText("Default Legend");

 }

 dc.SetBkMode(TRANSPARENT);

   int width =  40;
   int height = 20;
   CRect lr;

      lr.top = 10;
      lr.bottom = lr.top + height;
      lr.left = 10;
      lr.right = lr.left + width;

	  if(m_pDoc->m_LinkMOEMode < 0)
		  return;


	  int MOE_checking_index = m_pDoc->m_LinkMOEMode ;

		  if(m_pDoc->m_LinkMOEMode == MOE_emissions )
			 {

				 MOE_checking_index =  m_pDoc->m_LinkMOEMode + m_pDoc->m_EmissionType;
			 }

	  for(i = 1; i< MAX_LOS_SIZE-1; i++)
   {
      dc.SelectObject(penLOS[i]);
      dc.SelectObject(brushLOS[i]);
	  CString lengend_interval_str;

	  if(i != MAX_LOS_SIZE-2) 
		  lengend_interval_str.Format(" %.2f - %.2f",m_pDoc->m_LOSBound[MOE_checking_index][i],m_pDoc->m_LOSBound[MOE_checking_index][i+1]);
	  else
	  {
		if(m_pDoc->m_LOSBound[MOE_checking_index][i] > m_pDoc->m_LOSBound[MOE_checking_index][i+1])
		   lengend_interval_str.Format(" < %.2f ",m_pDoc->m_LOSBound[MOE_checking_index][i]);
		else
		   lengend_interval_str.Format(" > %.2f ",m_pDoc->m_LOSBound[MOE_checking_index][i]);
	  }

	  dc.TextOut(lr.right,lr.top,lengend_interval_str);
      dc.Rectangle(lr);
      lr.top = lr.bottom;
      lr.bottom = lr.top + height;
	}

/*
    width =  50;

    lr.top = lr.bottom + 20;

		double lane_offset = m_pDoc->m_UnitFeet*m_pDoc->m_LaneWidthInFeet;  // 20 feet per lane

	  if(m_pDoc->m_LinkBandWidthMode == LBW_number_of_lanes)
	{

	  for(i = 1; i<= 5; i+=2)
	  {
			double m_BandWidthValue =  i*5/max(1,m_pDoc->m_MaxLinkWidthAsNumberOfLanes);
			height = max(1,lane_offset * m_BandWidthValue * m_pDoc->m_Doc_Resolution);
			lr.bottom = lr.top + height;

			CString band_width_str;
			if(i==1)
				band_width_str.Format (" %d lane", i);
			else
				band_width_str.Format (" %d lanes", i);

			  dc.TextOut(lr.left,lr.top-20,band_width_str);
			  dc.Rectangle(lr);

			lr.left = lr.right;
			lr.right = lr.left + width;

	  }
	  
	
	 }
*/

}


void CDlg_Legend::OnSize(UINT nType, int cx, int cy)
{
   CBaseDialog::OnSize(nType, cx, cy);
   RedrawWindow();
}

void CDlg_Legend::OnCbnSelchangeComboEmissiontype()
{
	   if(m_pDoc->m_LinkMOEMode == MOE_emissions)
	   {
	   		m_pDoc->m_EmissionType = (DTA_EMISSION_TYPE) m_ComboBox_EmissionType.GetCurSel ();
	   
			Invalidate(1);
			m_pDoc->UpdateAllViews (0);
	   }
}
