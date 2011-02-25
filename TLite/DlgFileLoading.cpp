// DlgFileLoading.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgFileLoading.h"


// CDlgFileLoading dialog

IMPLEMENT_DYNAMIC(CDlgFileLoading, CDialog)

CDlgFileLoading::CDlgFileLoading(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFileLoading::IDD, pParent)
{

}

CDlgFileLoading::~CDlgFileLoading()
{
}

void CDlgFileLoading::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_NETEWORK_FILE, m_ListBox_NetworkData);
	DDX_Control(pDX, IDC_LIST_NETEWORK_FILE2, m_ListBox_OptimizationData);
	DDX_Control(pDX, IDC_LIST_OUTPUTDTA, m_ListBox_OutputData);
}


BEGIN_MESSAGE_MAP(CDlgFileLoading, CDialog)
END_MESSAGE_MAP()


// CDlgFileLoading message handlers

BOOL CDlgFileLoading::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ListBox_NetworkData.AddString (m_pDoc->m_NodeDataLoadingStatus);
	m_ListBox_NetworkData.AddString (m_pDoc->m_LinkDataLoadingStatus);

	m_ListBox_NetworkData.AddString (m_pDoc->m_ZoneDataLoadingStatus);

	m_ListBox_NetworkData.AddString (m_pDoc->m_DemandDataLoadingStatus);
	
	if(m_pDoc->m_BackgroundBitmapLoaded)
	m_ListBox_NetworkData.AddString (m_pDoc->m_BackgroundImageFileLoadingStatus);
	
	if(m_pDoc->m_SensorLocationLoadingStatus.GetLength() > 0)
	{
		m_ListBox_NetworkData.AddString(m_pDoc->m_SensorLocationLoadingStatus);
	}

	if(m_pDoc->m_SensorDataLoadingStatus.GetLength() > 0)
	{
		m_ListBox_NetworkData.AddString(m_pDoc->m_SensorDataLoadingStatus);
	}

	if(m_pDoc->m_EventDataLoadingStatus.GetLength() > 0)
	{
		m_ListBox_NetworkData.AddString(m_pDoc->m_EventDataLoadingStatus);
	}



	if(m_pDoc->m_LinkTrainTravelTimeDataLoadingStatus.GetLength () >0) // there are data being loaded
	{
		m_ListBox_OptimizationData.AddString (m_pDoc->m_LinkTrainTravelTimeDataLoadingStatus);	
	}

	if(m_pDoc->m_ObsLinkVolumeStatus.GetLength () >0) // there are data being loaded
	{
		m_ListBox_OptimizationData.AddString (m_pDoc->m_ObsLinkVolumeStatus);	
	}
	

	if(m_pDoc->m_TimetableDataLoadingStatus.GetLength () >0) // there are data being loaded
	{
		m_ListBox_OptimizationData.AddString (m_pDoc->m_TimetableDataLoadingStatus);	
	}

	if(m_pDoc->m_SimulationLinkMOEDataLoadingStatus.GetLength ()>0)
	{
	m_ListBox_OutputData.AddString (m_pDoc->m_SimulationLinkMOEDataLoadingStatus);
	}
	

	if(m_pDoc->m_SimulationVehicleDataLoadingStatus.GetLength ()>0)
	{
	m_ListBox_OutputData.AddString(m_pDoc->m_SimulationVehicleDataLoadingStatus);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

