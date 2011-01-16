// DlgAssignmentSettings.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgAssignmentSettings.h"


// CDlgAssignmentSettings dialog

IMPLEMENT_DYNAMIC(CDlgAssignmentSettings, CDialog)

CDlgAssignmentSettings::CDlgAssignmentSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAssignmentSettings::IDD, pParent)
	, m_NumberOfIterations(10)
	, m_DemandGlobalMultiplier(1.0f)
{

}

CDlgAssignmentSettings::~CDlgAssignmentSettings()
{
}

void CDlgAssignmentSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_Number_Iterations, m_NumberOfIterations);
	DDV_MinMaxInt(pDX, m_NumberOfIterations, 1, 100);
	DDX_Text(pDX, IDC_EDIT_Multiplier, m_DemandGlobalMultiplier);
	DDV_MinMaxFloat(pDX, m_DemandGlobalMultiplier, 0.001f, 10.0f);
}


BEGIN_MESSAGE_MAP(CDlgAssignmentSettings, CDialog)
END_MESSAGE_MAP()


// CDlgAssignmentSettings message handlers

