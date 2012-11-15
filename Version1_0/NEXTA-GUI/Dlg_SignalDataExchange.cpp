// Dlg_SignalDataExchange.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_SignalDataExchange.h"


// CDlg_SignalDataExchange dialog

IMPLEMENT_DYNAMIC(CDlg_SignalDataExchange, CDialog)

CDlg_SignalDataExchange::CDlg_SignalDataExchange(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_SignalDataExchange::IDD, pParent)
{

}

CDlg_SignalDataExchange::~CDlg_SignalDataExchange()
{
}

void CDlg_SignalDataExchange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_INFO, m_InfoList);
}


BEGIN_MESSAGE_MAP(CDlg_SignalDataExchange, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_DATA, &CDlg_SignalDataExchange::OnBnClickedButtonGenerateData)
	ON_BN_CLICKED(IDC_BUTTON_QEM, &CDlg_SignalDataExchange::OnBnClickedButtonQem)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_VISSIM_DATA, &CDlg_SignalDataExchange::OnBnClickedButtonGenerateVissimData)
	ON_BN_CLICKED(IDOK, &CDlg_SignalDataExchange::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlg_SignalDataExchange message handlers

void CDlg_SignalDataExchange::OnBnClickedButtonGenerateData()
{
	CWaitCursor wait;

   m_pDoc->GenerateMovementCountFromVehicleFile();

   m_pDoc->Constructandexportsignaldata ();
}

void CDlg_SignalDataExchange::OnBnClickedButtonQem()
{ 
	CWaitCursor wait;

   m_pDoc->GenerateMovementCountFromVehicleFile();
	m_pDoc->RunExcelAutomation();
}

void CDlg_SignalDataExchange::OnBnClickedButtonGenerateVissimData()
{
		CWaitCursor wait;

   m_pDoc->GenerateMovementCountFromVehicleFile();
	m_pDoc->ExportPathflowToCSVFiles();
	m_pDoc->ConstructandexportVISSIMdata();
}


void CDlg_SignalDataExchange::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

BOOL CDlg_SignalDataExchange::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString info_str;

	info_str.Format ("Demand loading time period: %s -> %s, %d min", m_pDoc->GetTimeStampString (m_pDoc->m_DemandLoadingStartTimeInMin ), m_pDoc->GetTimeStampString (m_pDoc->m_DemandLoadingEndTimeInMin ), 
		
	m_pDoc->m_DemandLoadingEndTimeInMin - m_pDoc->m_DemandLoadingStartTimeInMin	);

	m_InfoList.AddString (info_str);

	info_str.Format("Volume conversion factor = %.2f", 60.f/max(1,m_pDoc->m_DemandLoadingEndTimeInMin - m_pDoc->m_DemandLoadingStartTimeInMin));
	m_InfoList.AddString (info_str);

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
