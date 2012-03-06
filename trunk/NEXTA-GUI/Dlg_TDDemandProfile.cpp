// Dlg_TDDemandProfile.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_TDDemandProfile.h"


// CDlg_TDDemandProfile dialog

CDlg_TDDemandProfile::CDlg_TDDemandProfile(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_TDDemandProfile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlg_TDDemandProfile)
	m_dValue = 0.0;
	m_sLabel = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_iItem = -1;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDlg_TDDemandProfile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlg_TDDemandProfile)
	DDX_Text(pDX, IDC_VALUE, m_dValue);
	DDX_Text(pDX, IDC_LABEL, m_sLabel);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_DEMAND_PROFILE_LIST, m_ODProfileList);
}

BEGIN_MESSAGE_MAP(CDlg_TDDemandProfile, CDialog)
	//{{AFX_MSG_MAP(CDlg_TDDemandProfile)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RADIO1, OnRadioButtons)
	ON_BN_CLICKED(IDC_RADIO3, OnRadioColors)
	ON_BN_CLICKED(IDC_CUSTOM1, OnAdd)
	ON_NOTIFY(NSCS_SELECTEDITEM, IDC_CUSTOM1, OnChartSelectedItem)
	ON_BN_CLICKED(IDC_DEL, OnDel)
	ON_BN_CLICKED(IDC_RADIO2, OnRadioButtons)
	ON_BN_CLICKED(IDC_RADIO4, OnRadioColors)
	ON_BN_CLICKED(IDC_RADIO5, OnRadioColors)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_MODIFY, OnModify)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(IDC_MODIFY,OnUpdateModify)
	ON_UPDATE_COMMAND_UI(IDC_DEL,OnUpdateModify)
	ON_UPDATE_COMMAND_UI(IDC_ADD,OnUpdateAdd)
	ON_LBN_SELCHANGE(IDC_DEMAND_PROFILE_LIST, &CDlg_TDDemandProfile::OnLbnSelchangeDemandProfileList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlg_TDDemandProfile message handlers

BOOL CDlg_TDDemandProfile::OnInitDialog()
{

	CDialog::OnInitDialog();

	//Subclassing

	m_chart.SubclassDlgItem(IDC_CUSTOM1,this);

	// Add "About..." menu item to system menu.

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	m_chart.PrepareColors(CNSChartCtrl::SimpleColors);

	// processing demand data 

		m_StartTimeInterval = 96;
		m_EndTimeInterval = 0;

		for(unsigned int i = 0; i < m_pDoc->m_DemandProfileVector.size(); i++)
		{
			DTADemandProfile element = m_pDoc->m_DemandProfileVector[i];

			CString from_zone_str, to_zone_str, demand_type_str;

			if(element.from_zone_id > 0) 
				from_zone_str.Format (" from zone %d", element.from_zone_id);
			else
				from_zone_str.Format (" from all zones");

			if(element.to_zone_id > 0) 
				to_zone_str.Format (" to zone %d", element.from_zone_id);
			else
				to_zone_str.Format (" to all zones");


			if(element.demand_type > 0) 
				demand_type_str.Format (" for demand type %s", m_pDoc->m_DemandTypeVector[element.demand_type -1].demand_type_name);
			else
				demand_type_str.Format (" for all demand types");

			CString ProfileStr;
			ProfileStr.Format ("Series no.%d: ",i+1);
			ProfileStr = ProfileStr + from_zone_str + to_zone_str + demand_type_str;

			m_ODProfileList.AddString (ProfileStr);

			for(int t = 0; t< MAX_TIME_INTERVAL_SIZE; t++)
			{
				double ratio = element.time_dependent_ratio[t];
				if(ratio>0.00001)
				{
				if(t < m_StartTimeInterval)
					m_StartTimeInterval = t;

				if(t > m_EndTimeInterval)
					m_EndTimeInterval = t;
				}

			}
		}

//		fprintf(st,"from_zone_id,to_zone_id,demand_type,time_series_name,");

		int CurProfileNo = 0;

		if(m_ODProfileList.GetCount () > 0)
		{
			m_ODProfileList.SetCurSel (CurProfileNo);
			DTADemandProfile element = m_pDoc->m_DemandProfileVector[CurProfileNo];

//			fprintf(st,"%d,%d,%d,%s,", element.from_zone_id, element.to_zone_id,element.demand_type , element.series_name );

			for(int t = m_StartTimeInterval; t< m_EndTimeInterval; t++)
			{
				CString time_stamp_str = m_pDoc->GetTimeStampStrFromIntervalNo (t,false);
				double percentage = element.time_dependent_ratio[t];
				m_chart.AddValue(percentage,time_stamp_str);
			}

		}
	
	m_chart.SetChartStyle(NSCS_BAR);

	CheckRadioButton(IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
	CheckRadioButton(IDC_RADIO3,IDC_RADIO5,IDC_RADIO4);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlg_TDDemandProfile::OnSysCommand(UINT nID, LPARAM lParam)
{
		CDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDlg_TDDemandProfile::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDlg_TDDemandProfile::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDlg_TDDemandProfile::OnRadioButtons() 
{
	int state = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);
	
	if(state == IDC_RADIO1)
	{
		m_chart.SetChartStyle(NSCS_PIE);
	}
	else
	{
		m_chart.SetChartStyle(NSCS_BAR);
	}		
}

void CDlg_TDDemandProfile::OnRadioColors() 
{
	int state = GetCheckedRadioButton(IDC_RADIO3,IDC_RADIO5);
	
	switch(state){
		case IDC_RADIO3:
			m_chart.PrepareColors(CNSChartCtrl::GrayScale);
			break;
		case IDC_RADIO4:
			m_chart.PrepareColors(CNSChartCtrl::SimpleColors);
			break;
		case IDC_RADIO5:
			m_chart.ResetColors();
			m_chart.AddBrush(new CBrush(HS_BDIAGONAL,0x0));
			m_chart.AddBrush(new CBrush(HS_HORIZONTAL,0x0));
			m_chart.AddBrush(new CBrush(HS_VERTICAL,0x0));
			m_chart.AddBrush(new CBrush(HS_CROSS,0x0));
			m_chart.AddBrush(new CBrush(HS_DIAGCROSS,0x0));
			m_chart.AddBrush(new CBrush(HS_FDIAGONAL,0x0));
			break;
	}
	m_chart.Invalidate(FALSE);
}



void CDlg_TDDemandProfile::OnAdd() 
{
	if(m_iItem < 0)
	{
		UpdateData();
		if(m_dValue > 0.0 && !m_sLabel.IsEmpty())
		{
			m_chart.AddValue(m_dValue,m_sLabel,TRUE);
			m_iItem = -1;
			m_dValue = 0.0;
			m_sLabel = "";
			UpdateData(FALSE);
		}
	}
	UpdateDialogControls(this,FALSE);
}

void CDlg_TDDemandProfile::OnChartSelectedItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCHARTCTRL nmchart = (LPNMCHARTCTRL)pNMHDR;
	
	m_iItem = nmchart->iItem;

	if(m_iItem >= 0)
	{
		m_dValue = (float)(nmchart->dValue);  // convert to float to avoid many decimals
		m_sLabel = nmchart->sLabel;
	}else
	{
		m_dValue = 0;
		m_sLabel = "";
	}
	UpdateData(FALSE);
	UpdateDialogControls(this,FALSE);
	*pResult = FALSE;
}


void CDlg_TDDemandProfile::OnDel() 
{
	if(m_chart.DeleteItem(m_iItem))
	{
		m_dValue = 0;
		m_sLabel = "";
		m_iItem  = -1;
		UpdateData(FALSE);
		UpdateDialogControls(this,FALSE);
	}
}

void CDlg_TDDemandProfile::OnModify() 
{
	UpdateData();
	if(m_dValue > 0.0 && !m_sLabel.IsEmpty())
	{
		m_chart.ModifyItem(m_iItem,m_dValue,m_sLabel);
	}
}
void CDlg_TDDemandProfile::OnUpdateModify(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_iItem>=0);
}


void CDlg_TDDemandProfile::OnUpdateAdd(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_iItem < 0);
}


// CDlg_TDDemandProfile message handlers

void CDlg_TDDemandProfile::OnLbnSelchangeDemandProfileList()
{
		int CurProfileNo = m_ODProfileList.GetCurSel ();

		m_chart.ResetChart();

		if(m_ODProfileList.GetCount () > 0)
		{
			DTADemandProfile element = m_pDoc->m_DemandProfileVector[CurProfileNo];

			for(int t = m_StartTimeInterval; t< m_EndTimeInterval; t++)
			{
				CString time_stamp_str = m_pDoc->GetTimeStampStrFromIntervalNo (t,false);
				double percentage = element.time_dependent_ratio[t];
				m_chart.AddValue(percentage,time_stamp_str);
			}

		Invalidate();	

		}
}
