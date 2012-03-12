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
}


BEGIN_MESSAGE_MAP(CDlg_SignalDataExchange, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_DATA, &CDlg_SignalDataExchange::OnBnClickedButtonGenerateData)
	ON_BN_CLICKED(IDC_BUTTON_QEM, &CDlg_SignalDataExchange::OnBnClickedButtonQem)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_VISSIM_DATA, &CDlg_SignalDataExchange::OnBnClickedButtonGenerateVissimData)
END_MESSAGE_MAP()


// CDlg_SignalDataExchange message handlers

void CDlg_SignalDataExchange::OnBnClickedButtonGenerateData()
{
   m_pDoc->Constructandexportsignaldata ();
}

void CDlg_SignalDataExchange::OnBnClickedButtonQem()
{
	m_pDoc->RunExcelAutomation();
}

void CDlg_SignalDataExchange::OnBnClickedButtonGenerateVissimData()
{
	m_pDoc->ConstructandexportVISSIMdata();
}
