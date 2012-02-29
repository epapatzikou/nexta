// Dlg_TravelTimeReliability.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_TravelTimeReliability.h"
#include "math.h"
#include <vector>

using std::vector;


// CDlg_TravelTimeReliability dialog

float proportion[4] = {0.6f,0.18f,0.12f,0.1f};
int IntProportion[4];

void g_RandomCapacity(float* ptr, int num, float mean, float COV,int seed)
{
	float mu, sigma;
	float variance = pow(mean*COV,2);
	mu = log(mean) - 1/2*log(1+variance/pow(mean,2));
	sigma = sqrt(log(1+variance/pow(mean,2)));

	srand(seed);

	for (int i=0;i<num;i++,ptr++)
	{
		*ptr = exp(mu+sigma*g_RNNOF());
	}
}

IMPLEMENT_DYNAMIC(CDlg_TravelTimeReliability, CDialog)

CDlg_TravelTimeReliability::CDlg_TravelTimeReliability(CWnd* pParent /*=NULL*/)
: CDialog(CDlg_TravelTimeReliability::IDD, pParent)
{
	m_dValue = 0;
	m_PathFreeFlowTravelTime = 15;

	for (int i=0;i<MAX_SAMPLE_SIZE;i++)
	{
		Capacity[i] = AdditionalDelay[i] = 0.0f;
		TravelTime[i] = 0.0f;
	}
}

void CDlg_TravelTimeReliability::UpdateCapacityAndDelay()
{
	//int IntProportion[4];
	int sum=0;

	for (int i=1;i<4;i++)
	{
		IntProportion[i] = (int)(proportion[i]*MAX_SAMPLE_SIZE);
		sum += IntProportion[i];
	}

	//Normal
	IntProportion[0] = MAX_SAMPLE_SIZE - sum;

	float capacity_lower_bound  = 2000;  // to aoid extrem large travel time


	for (int i=0;i<LinkCapacity.size();i++)
	{
		if (i != m_BottleneckIdx && i != m_ImpactedLinkIdx) //Non-bottleneck Link 
		{
			for (int j=0;j<MAX_SAMPLE_SIZE;j++)
			{
				TravelTime[i] += LinkTravelTime[i];
			}
		}
		else
		{

			if (i == m_BottleneckIdx)
			{
				if (m_bImpacted == false) // No impacted link by incidents
				{
					g_RandomCapacity(&Capacity[0],IntProportion[0],LinkCapacity[i],0.2f,100);
					g_RandomCapacity(&Capacity[IntProportion[0]],IntProportion[1],LinkCapacity[i]*0.4,0.2f,100);
					g_RandomCapacity(&Capacity[IntProportion[0]+IntProportion[1]],IntProportion[2],900,0.2f,100);
					g_RandomCapacity(&Capacity[MAX_SAMPLE_SIZE-IntProportion[3]],IntProportion[3],1400,0.2f,100);

					for (int j=0;j<MAX_SAMPLE_SIZE;j++)
					{
						AdditionalDelay[j] = LinkTravelTime[i]*(1-Capacity[j]/max(capacity_lower_bound,LinkCapacity[i]));
						TravelTime[j] += LinkTravelTime[i] + AdditionalDelay[i];
					}
				}
				else // Include impacted link by incidents
				{
					for (int j=0;j<MAX_SAMPLE_SIZE;j++)
					{
						TravelTime[i] += LinkTravelTime[i];
					}
				}
			}
			else //Impacted link by incidents
			{
				g_RandomCapacity(&Capacity[0],IntProportion[0],LinkCapacity[i],0.1f,100);
				g_RandomCapacity(&Capacity[IntProportion[0]],IntProportion[1],LinkCapacity[i]*m_LaneClosureRatio,0.3f,100);
				g_RandomCapacity(&Capacity[IntProportion[0]+IntProportion[1]],IntProportion[2],LinkCapacity[i]*0.5f,0.2f,100);
				g_RandomCapacity(&Capacity[MAX_SAMPLE_SIZE-IntProportion[3]],IntProportion[3],LinkCapacity[i]*0.7f,0.2f,100);

				for (int j=0;j<MAX_SAMPLE_SIZE;j++)
				{
					AdditionalDelay[j] = LinkTravelTime[i]*(1-Capacity[j]/max(capacity_lower_bound,LinkCapacity[i]));
					TravelTime[j] += LinkTravelTime[i] + AdditionalDelay[i];
				}
			}

		}
	}
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

	UpdateCapacityAndDelay();


	m_FactorLabel[0]= "Capacity Variations";
	m_FactorLabel[1]= "Incidents";
	m_FactorLabel[2]= "Work Zones";
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
	m_ReliabilityMOEList.AddString ("Cumulative Distribution Function");
	m_ReliabilityMOEList.SetCurSel (0);

	DisplayTravelTimeChart();

//	m_chart_traveltime.SetChartStyle(NSCS_BAR);
	m_chart_traveltime.SetChartStyle(NSCS_LINE);


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
		int count=0;
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

	if (m_dValue >= 1.0f)
	{
		MessageBox("Value cannot be greater than 1.0!","Error",MB_OK | MB_ICONSTOP);
		return;
	}
	int idx=0;
	for (int i=0;i<4;i++)
	{
		if (m_FactorLabel[i].Compare(this->m_sLabel)==0) 
		{
			idx = i;
			break;
		}
	}

	float sum = 0.0f;
	for (int i=0;i<4;i++)
	{
		if (idx == i) continue;
		sum += proportion[i];
	}

	if ((sum + m_dValue - 1.0f) > 0.0001)
	{
		MessageBox("The sum of proportion is greater than 1.0!","Error",MB_OK | MB_ICONSTOP);
		return;
	}
	proportion[idx] = m_dValue;



	UpdateCapacityAndDelay();

	Invalidate();
	Display7FactorChart();
	DisplayTravelTimeChart();


}
void CDlg_TravelTimeReliability::OnChartSelectedItem(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCHARTCTRL nmchart = (LPNMCHARTCTRL)pNMHDR;

	m_iItem = nmchart->iItem;

	if(m_iItem >= 0)
	{
		m_dValue = proportion[m_iItem];
		m_sLabel = nmchart->sLabel;

	}
	//{
	//	m_dValue = (float)(nmchart->dValue);  // convert to float to avoid many decimals
	//	m_sLabel = nmchart->sLabel;
	//}
	//else
	//{
	//	m_dValue = 0;
	//	m_sLabel = "";
	//}
	UpdateData(FALSE);
	UpdateDialogControls(this,FALSE);
	*pResult = FALSE;
}

void CDlg_TravelTimeReliability::OnBnClickedEditScenario()
{
	m_pDoc->OnScenarioConfiguration();
}

void GenerateXAndY(float* x_ptr, int num, vector<CString>& X, vector<float>& Y, int& step_size)
{
	float max=-1.0f;
	float min= 999999.0f;
	float* ptr = x_ptr;
	for (int i=0;i<num;i++)
	{
		if (*ptr > max) max = *ptr;
		if (*ptr < min) min = *ptr;
		ptr++;
	}

	step_size = 5;

	//if (max - min > 50) 
	//{
	//	step_size = 5;
	//}
	//else
	//{
	//	step_size = 2;
	//}

	int lowerbound = (int)(min / step_size)*step_size;
	int upperbound = (int)(max / step_size) * step_size;

	if (upperbound < max) upperbound += step_size;

	for (int i=0;i<upperbound;i+=step_size)
	{
		CString str;
		str.Format("%d",lowerbound+i);
		X.push_back(str);
		Y.push_back(0.0f);
	}

	ptr = x_ptr;
	for (int i=0;i<num;i++)
	{
		float tmp = *(ptr+i);
		int idx = (int)(tmp-lowerbound)/step_size;
		if (idx < Y.size())
		{
			Y[idx]++;
		}
		else
		{
			Y[Y.size()-1]++;
		}
	}


}

void CDlg_TravelTimeReliability::DisplayTravelTimeChart()
{
	int CurSelectionNo = m_ReliabilityMOEList.GetCurSel ();

	int count;
	CString travel_time_str;


	vector<CString> XLabel;
	vector<float> YLabel;
	int step_size;
	GenerateXAndY(TravelTime,100,XLabel,YLabel,step_size);

	int t;
	m_chart_traveltime.ResetChart();		

	//int travel_time_count_array[100];


	//// normalize 
	//for(t=10; t<30; t+=5)
	//{
	//	travel_time_count_array[t] = travel_time_count_array[t]*100.f/max(1,total_count);
	//}

	if(CurSelectionNo == 0)
	{
		//count = 0;
		//for(t=10; t<30; t+=5)
		//{
		//	travel_time_str.Format("%d",t);
		//	count = travel_time_count_array[t];

		//	m_chart_traveltime.AddValue(count,travel_time_str);
		//}

		for (size_t i=0;i<XLabel.size();i++)
		{
			m_chart_traveltime.AddValue(YLabel[i],XLabel[i]);
		}

	}
	else
	{
		//count = 0;
		//for(t=10; t<30; t+=5)
		//{
		//	travel_time_str.Format("%d",t);
		//	count += travel_time_count_array[t];

		//	m_chart_traveltime.AddValue(count,travel_time_str);
		//}


		float total_count = 0.0f;
		for(size_t i=0;i<YLabel.size();i++)
		{
			total_count+=YLabel[i];
		}

		float count = 0.0f;
		for (size_t i=0;i<XLabel.size();i++)
		{
			count += YLabel[i];
			m_chart_traveltime.AddValue(count/total_count*100.0f,XLabel[i]);
		}
	}
	Invalidate();	

}

float GetMean(float* p,int num)
{
	float sum = 0.0f;
	float* ptr = p;

	for (int i=0;i<num;i++)
	{
		sum += *ptr++;
	}

	return sum/num;
}
float GetSTD(float* p, int num,float mean)
{
	float sum=0.0f;
	float* ptr = p;

	for (int i=0;i<num;i++)
	{
		sum += pow((mean-*ptr),2);
	}

	return sqrt(sum/(num-1));
}
void CDlg_TravelTimeReliability::Display7FactorChart()
{
	int CurSelectionNo = m_7FactorMOEList.GetCurSel ();

	float value =0;

	m_chart_7factors.ResetChart();		

	switch(CurSelectionNo)
	{
	case 0:
		for(int i =0; i< m_FactorSize; i++)
		{
			value = 1.65*GetSTD(TravelTime+IntProportion[i],IntProportion[i],m_PathFreeFlowTravelTime);
			m_chart_7factors.AddValue(value,m_FactorLabel[i]);
		}
		break;
	case 1:
		for(int i =0; i< m_FactorSize; i++)
		{
			value = 1.65*GetSTD(AdditionalDelay+IntProportion[i],IntProportion[i],GetMean(AdditionalDelay,MAX_SAMPLE_SIZE));
			m_chart_7factors.AddValue(value,m_FactorLabel[i]);
		}
		break;
	case 2:
		for(int i =0; i< m_FactorSize; i++)
		{
			int pos = 0;
			for (int j=0;j<i;j++)
			{
				pos += IntProportion[j];
			}

			value = GetMean(Capacity+pos,IntProportion[i]);
			m_chart_7factors.AddValue(value,m_FactorLabel[i]);
		}
		break;
	case 3:
		for(int i =0; i< m_FactorSize; i++)
		{
			m_chart_7factors.AddValue(proportion[i],m_FactorLabel[i]);
		}
		break;
	}

	Invalidate();	
}
