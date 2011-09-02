// DlgSensorDataLoading.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgSensorDataLoading.h"


// CDlgSensorDataLoading dialog

IMPLEMENT_DYNAMIC(CDlgSensorDataLoading, CDialog)

CDlgSensorDataLoading::CDlgSensorDataLoading(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSensorDataLoading::IDD, pParent)
	, m_ObsTimeInterval(5)
	, m_NumberOfDays(5)
	, m_Occ_to_Density_Coef(100)
	, m_StartingTime_in_min(0)
{

}

CDlgSensorDataLoading::~CDlgSensorDataLoading()
{
}

void CDlgSensorDataLoading::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TimeInterval, m_ObsTimeInterval);
	DDX_Text(pDX, IDC_EDIT_Number_of_Days, m_NumberOfDays);
	DDX_Text(pDX, IDC_EDIT_Occ_to_Density_Coef, m_Occ_to_Density_Coef);
	DDX_Text(pDX, IDC_EDIT_StartingTime_in_min, m_StartingTime_in_min);
	DDV_MinMaxInt(pDX, m_StartingTime_in_min, 0, 100000);
}


BEGIN_MESSAGE_MAP(CDlgSensorDataLoading, CDialog)
END_MESSAGE_MAP()


// CDlgSensorDataLoading message handlers
