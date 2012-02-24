// Dlg_TravelTimeReliability.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_TravelTimeReliability.h"


// CDlg_TravelTimeReliability dialog

IMPLEMENT_DYNAMIC(CDlg_TravelTimeReliability, CDialog)

CDlg_TravelTimeReliability::CDlg_TravelTimeReliability(CWnd* pParent /*=NULL*/)
: CDialog(CDlg_TravelTimeReliability::IDD, pParent)
{
	m_dValue = 0;
	m_PathFreeFlowTravelTime = 15;
}

CDlg_TravelTimeReliability::~CDlg_TravelTimeReliability()
{
}

void CDlg_TravelTimeReliability::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_1, m_ReliabilityMOEList);
	DDX_Control(pDX, IDC_LIST2, m_7FactorMOEList);
	DDX_Text(pDX, IDC_VALUE, m_dValue);
	DDX_Text(pDX, IDC_LABEL, m_sLabel);

}


BEGIN_MESSAGE_MAP(CDlg_TravelTimeReliability, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RADIO1, OnRadioButtons)
	ON_BN_CLICKED(IDC_RADIO2, OnRadioButtons)
	ON_BN_CLICKED(IDC_RADIO3, OnRadioColors)
	ON_BN_CLICKED(IDC_RADIO4, OnRadioColors)
	ON_BN_CLICKED(IDC_RADIO5, OnRadioColors)
	ON_NOTIFY(NSCS_SELECTEDITEM, IDC_Factor_CHART, OnChartSelectedItem)
	ON_LBN_SELCHANGE(IDC_LIST_1, &CDlg_TravelTimeReliability::OnLbnSelchangeList1)
	ON_LBN_SELCHANGE(IDC_LIST2, &CDlg_TravelTimeReliability::OnLbnSelchangeList2)
	ON_BN_CLICKED(ID_EXPORT_DATA, &CDlg_TravelTimeReliability::OnBnClickedExportData)
	ON_BN_CLICKED(IDC_MODIFY, &CDlg_TravelTimeReliability::OnBnClickedModify)
	ON_BN_CLICKED(ID_EDIT_SCENARIO, &CDlg_TravelTimeReliability::OnBnClickedEditScenario)
END_MESSAGE_MAP()


// CDlg_TravelTimeReliability message handlers

BOOL CDlg_TravelTimeReliability::OnInitDialog()
{


	CDialog::OnInitDialog();

	m_FactorLabel[0]= "Capacity Variations";
	m_FactorLabel[1]= "Incidents";
	m_FactorLabel[2]= "Work zones";
	m_FactorLabel[3]= "Severe Weather";

	m_FactorSize= 4;

	m_7FactorMOEList.AddString ("Reliability Impact: 1.65*STD of delay");
	m_7FactorMOEList.AddString ("Delay Impact");
	m_7FactorMOEList.AddString ("Capacity Impact" );
	m_7FactorMOEList.AddString ("Occurrence Probability" );
	m_7FactorMOEList.SetCurSel(0);

	m_chart_7factors.SubclassDlgItem(IDC_Factor_CHART,this);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_chart_7factors.PrepareColors(CNSChartCtrl::SimpleColors);

	Display7FactorChart();


	m_chart_7factors.SetChartStyle(NSCS_BAR);

	CheckRadioButton(IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
	CheckRadioButton(IDC_RADIO3,IDC_RADIO5,IDC_RADIO4);

	//*************************************************************//
	// setup travel time chart
	m_chart_traveltime.SubclassDlgItem(IDC_TravelTime_CHART,this);
	m_chart_traveltime.PrepareColors(CNSChartCtrl::GrayScale);


	m_ReliabilityMOEList.AddString ("Probability Density Function");
	m_ReliabilityMOEList.AddString ("Cumulative Probability Density Function");
	m_ReliabilityMOEList.SetCurSel (0);

	DisplayTravelTimeChart();

	m_chart_traveltime.SetChartStyle(NSCS_BAR);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CDlg_TravelTimeReliability::OnPaint() 
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
HCURSOR CDlg_TravelTimeReliability::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDlg_TravelTimeReliability::OnRadioButtons() 
{
	int state = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);

	if(state == IDC_RADIO1)
	{
		m_chart_7factors.SetChartStyle(NSCS_PIE);
	}
	else
	{
		m_chart_7factors.SetChartStyle(NSCS_BAR);
	}		
}

void CDlg_TravelTimeReliability::OnRadioColors() 
{
	int state = GetCheckedRadioButton(IDC_RADIO3,IDC_RADIO5);

	switch(state){
		case IDC_RADIO3:
			m_chart_7factors.PrepareColors(CNSChartCtrl::GrayScale);
			break;
		case IDC_RADIO4:
			m_chart_7factors.PrepareColors(CNSChartCtrl::SimpleColors);
			break;
		case IDC_RADIO5:
			m_chart_7factors.ResetColors();
			m_chart_7factors.AddBrush(new CBrush(HS_BDIAGONAL,0x0));
			m_chart_7factors.AddBrush(new CBrush(HS_HORIZONTAL,0x0));
			m_chart_7factors.AddBrush(new CBrush(HS_VERTICAL,0x0));
			m_chart_7factors.AddBrush(new CBrush(HS_CROSS,0x0));
			m_chart_7factors.AddBrush(new CBrush(HS_DIAGCROSS,0x0));
			m_chart_7factors.AddBrush(new CBrush(HS_FDIAGONAL,0x0));
			break;
	}
	m_chart_7factors.Invalidate(FALSE);
}


void CDlg_TravelTimeReliability::OnLbnSelchangeList1()
{
	DisplayTravelTimeChart();
}

void CDlg_TravelTimeReliability::OnLbnSelchangeList2()
{
	Display7FactorChart();
}

void CDlg_TravelTimeReliability::OnBnClickedExportData()
{
	CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"Data File (*.csv)|*.csv||", NULL);
	if(dlg.DoModal() == IDOK)
	{
		ExportData(dlg.GetPathName ());
	}
	m_pDoc->OpenCSVFileInExcel(dlg.GetPathName ());
}

void CDlg_TravelTimeReliability::ExportData(CString fname)
{
	FILE* st;
	fopen_s(&st,fname,"w");

	if(st!=NULL)
	{
		CWaitCursor wc;
		fprintf(st,"Category,count\n");
		int count;
		CString travel_time_str;


		for(int t=10; t<30; t+=5)
		{
			travel_time_str.Format("%d",t);
			count += 30*g_GetRandomRatio();

			fprintf(st,"%s,%d",travel_time_str,count);

		}


		// to do, 7 factors 

		fclose(st);
	}else
	{
		AfxMessageBox("The selected data file cannot be opened.");
	}
}

void CDlg_TravelTimeReliability::OnBnClickedModify()
{
	UpdateData();
}
void CDlg_TravelTimeReliability::OnChartSelectedItem(NMHDR* pNMHDR, LRESULT* pResult)
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

void CDlg_TravelTimeReliability::OnBnClickedEditScenario()
{
	m_pDoc->OnScenarioConfiguration();
}

void CDlg_TravelTimeReliability::DisplayTravelTimeChart()
{
	int CurSelectionNo = m_ReliabilityMOEList.GetCurSel ();

	int count;
	CString travel_time_str;

	int t;
	m_chart_traveltime.ResetChart();		

		int travel_time_count_array[100];

		int total_count = 0;
		for(t=10; t<30; t+=5)
		{
		travel_time_count_array[t]= 30*g_GetRandomRatio();
		total_count+=travel_time_count_array[t];
		}

		// normalize 
		for(t=10; t<30; t+=5)
		{
		travel_time_count_array[t] = travel_time_count_array[t]*100.f/max(1,total_count);
		}

	if(CurSelectionNo == 0)
	{
		count = 0;
		for(t=10; t<30; t+=5)
		{
			travel_time_str.Format("%d",t);
			count = travel_time_count_array[t];

			m_chart_traveltime.AddValue(count,travel_time_str);
		}

	}else
	{
		count = 0;
		for(t=10; t<30; t+=5)
		{
			travel_time_str.Format("%d",t);
			count += travel_time_count_array[t];

			m_chart_traveltime.AddValue(count,travel_time_str);
		}

	}
	Invalidate();	

}

void CDlg_TravelTimeReliability::Display7FactorChart()
{
	int CurSelectionNo = m_ReliabilityMOEList.GetCurSel ();

	float percentage =0;

	m_chart_7factors.ResetChart();		

	if(CurSelectionNo == 0)
	{
		
		for(int i =0; i< m_FactorSize; i++)
		{
			percentage = 20*g_GetRandomRatio();
		m_chart_7factors.AddValue(percentage,m_FactorLabel[i]);
		}

	}else
	{
		for(int i =0; i< m_FactorSize; i++)
		{
			percentage += 20*g_GetRandomRatio();
		m_chart_7factors.AddValue(percentage,m_FactorLabel[i]);
		}

	}
	Invalidate();	
}
