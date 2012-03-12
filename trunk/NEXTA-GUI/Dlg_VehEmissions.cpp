// Dlg_VehEmissions.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_VehEmissions.h"
#define MAX_STRING_LENGTH  100


// CDlg_VehPathAnalysis dialog

IMPLEMENT_DYNAMIC(CDlg_VehPathAnalysis, CBaseDialog)

CDlg_VehPathAnalysis::CDlg_VehPathAnalysis(CWnd* pParent /*=NULL*/)
: CBaseDialog(CDlg_VehPathAnalysis::IDD, pParent)
, m_SingleVehicleID(0)
{
}

CDlg_VehPathAnalysis::~CDlg_VehPathAnalysis()
{

	if(m_ODMOEMatrix !=NULL)
		DeallocateDynamicArray<VehicleStatistics>(m_ODMOEMatrix,m_pDoc->m_ODSize+1,m_pDoc->m_ODSize+1);

	std::vector<int> LinkVector;
	// empty vector
	m_pDoc->HighlightPath(LinkVector,1);


}

void CDlg_VehPathAnalysis::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_VEHICLE, m_VehicleList);
	DDX_Control(pDX, IDC_COMBO_Origin, m_OriginBox);
	DDX_Control(pDX, IDC_COMBO_Destination, m_DestinationBox);
	DDX_Control(pDX, IDC_COMBO_DepartureTime, m_DepartureTimeBox);
	DDX_Control(pDX, IDC_COMBO_VehicleType, m_VehicleTypeBox);
	DDX_Control(pDX, IDC_OpMode_LINK, m_OpModeList);
	DDX_Control(pDX, IDC_COMBO_InformationClass, m_InformationClassBox);
	DDX_Control(pDX, IDC_COMBO_Min_Number_of_vehicles, m_MinVehicleSizeBox);
	DDX_Control(pDX, IDC_COMBO_Min_Travel_Time, m_MinDistanceBox);
	DDX_Control(pDX, IDC_COMBO_Min_TravelTimeIndex, m_MinTTIBox);
	DDX_Control(pDX, IDC_COMBO_TimeInterval, m_TimeIntervalBox);
	DDX_Control(pDX, IDC_LIST_OD, m_ODList);
	DDX_Control(pDX, IDC_LIST_PATH, m_PathList);
	DDX_Control(pDX, IDC_COMBO_ImpactLink, m_ImpactLinkBox);
	DDX_Control(pDX, IDC_SUMMARY_INFO, m_Summary_Info_Edit);
	DDX_Control(pDX, IDC_COMBO_VOT_LB, m_ComboBox_VOT_LB);
	DDX_Control(pDX, IDC_COMBO_VOT_UB, m_ComboBox_VOT_UB);
	DDX_Control(pDX, IDC_COMBO_DemandType, m_DemandTypeBox);
}


BEGIN_MESSAGE_MAP(CDlg_VehPathAnalysis, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_LINK, &CDlg_VehPathAnalysis::OnLbnSelchangeListLink)
	ON_LBN_SELCHANGE(IDC_LIST_VEHICLE, &CDlg_VehPathAnalysis::OnLbnSelchangeListVehicle)
	ON_LBN_SELCHANGE(IDC_LIST_LINK2, &CDlg_VehPathAnalysis::OnLbnSelchangeListLink2)
	ON_CBN_SELCHANGE(IDC_COMBO_Origin, &CDlg_VehPathAnalysis::OnCbnSelchangeComboOrigin)
	ON_CBN_SELCHANGE(IDC_COMBO_Destination, &CDlg_VehPathAnalysis::OnCbnSelchangeComboDestination)
	ON_CBN_SELCHANGE(IDC_COMBO_DepartureTime, &CDlg_VehPathAnalysis::OnCbnSelchangeComboDeparturetime)
	ON_CBN_SELCHANGE(IDC_COMBO_VehicleType, &CDlg_VehPathAnalysis::OnCbnSelchangeComboVehicletype)
	ON_CBN_SELCHANGE(IDC_COMBO_InformationClass, &CDlg_VehPathAnalysis::OnCbnSelchangeComboInformationclass)
	ON_CBN_SELCHANGE(IDC_COMBO_TimeInterval, &CDlg_VehPathAnalysis::OnCbnSelchangeComboTimeinterval)
	ON_CBN_SELCHANGE(IDC_COMBO_Min_Number_of_vehicles, &CDlg_VehPathAnalysis::OnCbnSelchangeComboMinNumberofvehicles)
	ON_CBN_SELCHANGE(IDC_COMBO_Min_Travel_Time, &CDlg_VehPathAnalysis::OnCbnSelchangeComboMinTravelTime)
	ON_CBN_SELCHANGE(IDC_COMBO_Min_TravelTimeIndex, &CDlg_VehPathAnalysis::OnCbnSelchangeComboMinTraveltimeindex)
	ON_LBN_SELCHANGE(IDC_LIST_OD, &CDlg_VehPathAnalysis::OnLbnSelchangeListOd)
	ON_LBN_SELCHANGE(IDC_LIST_PATH, &CDlg_VehPathAnalysis::OnLbnSelchangeListPath)
	ON_CBN_SELCHANGE(IDC_COMBO_ImpactLink, &CDlg_VehPathAnalysis::OnCbnSelchangeComboImpactlink)
	ON_LBN_DBLCLK(IDC_LIST_OD, &CDlg_VehPathAnalysis::OnLbnDblclkListOd)
	ON_BN_CLICKED(ID_EXPORT, &CDlg_VehPathAnalysis::OnBnClickedExport)
	ON_CBN_SELCHANGE(IDC_COMBO_VOT_LB, &CDlg_VehPathAnalysis::OnCbnSelchangeComboVotLb)
	ON_CBN_SELCHANGE(IDC_COMBO_VOT_UB, &CDlg_VehPathAnalysis::OnCbnSelchangeComboVotUb)
	ON_CBN_SELCHANGE(IDC_COMBO_DemandType, &CDlg_VehPathAnalysis::OnCbnSelchangeComboDemandtype)
	ON_BN_CLICKED(ID_EXPORT_PATH_DATA, &CDlg_VehPathAnalysis::OnBnClickedExportPathData)
	ON_BN_CLICKED(ID_EXPORT_VEHICLE_DATA, &CDlg_VehPathAnalysis::OnBnClickedExportVehicleData)
END_MESSAGE_MAP()


// CDlg_VehPathAnalysis message handlers


BOOL CDlg_VehPathAnalysis::OnInitDialog()
{
	CDialog::OnInitDialog();
	CString str;

	m_ImpactLinkBox.AddString ("N/A");
	std::list<DTALink*>::iterator iLink;
	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{
		if((*iLink) ->CapacityReductionVector.size()>0)
		{
			str.Format ("%d->%d, Incident", (*iLink) ->m_FromNodeNumber,(*iLink) ->m_ToNodeNumber );
			m_ImpactLinkBox.AddString (str);

		}
	}

	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{
		if((*iLink) ->MessageSignVector.size()>0)
		{
			str.Format ("%d->%d, VMS", (*iLink) ->m_FromNodeNumber,(*iLink) ->m_ToNodeNumber );
			m_ImpactLinkBox.AddString (str);

		}
	}
				
	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{
		if((*iLink) ->TollVector.size()>0)
		{
			str.Format ("%d->%d, Toll", (*iLink) ->m_FromNodeNumber,(*iLink) ->m_ToNodeNumber );
			m_ImpactLinkBox.AddString (str);

		}
	}
	m_ImpactLinkBox.SetCurSel (0);

	m_OriginBox.AddString("All");
	m_DestinationBox.AddString("All");

	int i;
	for(i=1; i <= m_pDoc->m_ODSize ; i++)
	{
		str.Format ("%d", i);
		m_OriginBox.AddString(str);
		m_DestinationBox.AddString(str);
	}
	m_OriginBox.SetCurSel (0);
	m_DestinationBox.SetCurSel (0);

	m_ODMOEMatrix = AllocateDynamicArray<VehicleStatistics>(m_pDoc->m_ODSize+1,m_pDoc->m_ODSize+1);

	m_VehicleTypeBox.AddString ("All");
	
	for(i = 0; i< m_pDoc->m_VehicleTypeVector.size(); i++)
	{
	m_VehicleTypeBox.AddString (m_pDoc->m_VehicleTypeVector[i].vehicle_type_name);
	}

	m_VehicleTypeBox.SetCurSel(0);

	m_DemandTypeBox.AddString("All");
	for(i = 0; i< m_pDoc->m_DemandTypeVector .size(); i++)
	{
	m_DemandTypeBox.AddString (m_pDoc->m_DemandTypeVector[i].demand_type_name);
	}
	m_DemandTypeBox.SetCurSel(0);

	m_InformationClassBox.AddString ("All");
	m_InformationClassBox.AddString ("Historical info");
	m_InformationClassBox.AddString ("Pretrip info");
	m_InformationClassBox.AddString ("En-route Info");
	m_InformationClassBox.SetCurSel (0);


	for(i=0; i <= 1440 ; i+=15)
	{
		str.Format ("%d", i);
		m_DepartureTimeBox.AddString(str);
	}
	m_DepartureTimeBox.SetCurSel (0);

	m_TimeIntervalBox.AddString("1440");
	m_TimeIntervalBox.AddString("15");
	m_TimeIntervalBox.AddString("30");
	m_TimeIntervalBox.AddString("60");
	m_TimeIntervalBox.AddString("120");
	m_TimeIntervalBox.AddString("240");
	m_TimeIntervalBox.AddString("480");
	m_TimeIntervalBox.SetCurSel (0);

	m_MinVehicleSizeBox.AddString ("0");
	m_MinVehicleSizeBox.AddString ("2");
	m_MinVehicleSizeBox.AddString ("5");
	m_MinVehicleSizeBox.AddString ("10");
	m_MinVehicleSizeBox.AddString ("25");
	m_MinVehicleSizeBox.AddString ("50");
	m_MinVehicleSizeBox.AddString ("75");
	m_MinVehicleSizeBox.AddString ("100");
	m_MinVehicleSizeBox.AddString ("200");
	m_MinVehicleSizeBox.AddString ("500");
	m_MinVehicleSizeBox.SetCurSel (1);

	m_MinDistanceBox.AddString ("0");
	m_MinDistanceBox.AddString ("1");
	m_MinDistanceBox.AddString ("2");
	m_MinDistanceBox.AddString ("3");
	m_MinDistanceBox.AddString ("4");
	m_MinDistanceBox.AddString ("5");
	m_MinDistanceBox.AddString ("10");
	m_MinDistanceBox.AddString ("15");
	m_MinDistanceBox.AddString ("20");
	m_MinDistanceBox.AddString ("30");
	m_MinDistanceBox.AddString ("50");
	m_MinDistanceBox.AddString ("100");
	m_MinDistanceBox.SetCurSel (0);

	m_MinTTIBox.AddString("1.0"); 
	m_MinTTIBox.AddString("1.25"); 
	m_MinTTIBox.AddString("1.5"); 
	m_MinTTIBox.AddString("1.75"); 
	m_MinTTIBox.AddString("2.0"); 
	m_MinTTIBox.AddString("3.0"); 
	m_MinTTIBox.AddString("4.0"); 
	m_MinTTIBox.SetCurSel (0);

	for(int vot = 0; vot<=100; vot+=5)
	{
		str.Format ("%d", vot);
		m_DepartureTimeBox.AddString(str);

		m_ComboBox_VOT_LB.AddString (str);
		m_ComboBox_VOT_UB.AddString (str);
	}

	m_ComboBox_VOT_LB.SetCurSel (0);
	m_ComboBox_VOT_UB.SetCurSel (20);

	FilterOriginDestinationPairs();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlg_VehPathAnalysis::FilterOriginDestinationPairs()
{

	CWaitCursor wait;
	int i,j;
	for(i=1; i <= m_pDoc->m_ODSize ; i++)
		for(j=1; j<= m_pDoc->m_ODSize ; j++)
		{
			m_ODMOEMatrix[i][j].Reset ();
		}

	int ImpactLinkNo = -1;
	if(m_ImpactLinkBox.GetCurSel()>0)
	{
		char m_Text[MAX_STRING_LENGTH];
		int FromNodeNumber, ToNodeNumber;
		m_ImpactLinkBox.GetLBText (m_ImpactLinkBox.GetCurSel(), m_Text);
		sscanf(m_Text, "%d->%d", &FromNodeNumber, &ToNodeNumber);
		DTALink* pLink = m_pDoc->FindLinkWithNodeNumbers(FromNodeNumber, ToNodeNumber);
		if(pLink!=NULL)
			ImpactLinkNo = pLink->m_LinkNo;
	}


		int Origin = m_OriginBox.GetCurSel();
		int Destination = m_DestinationBox.GetCurSel();
		int VehicleType = m_VehicleTypeBox.GetCurSel();
		int DemandType = m_DemandTypeBox.GetCurSel();

		int InformationClass = m_InformationClassBox.GetCurSel();

		char str[MAX_STRING_LENGTH];
		m_DepartureTimeBox.GetLBText(m_DepartureTimeBox.GetCurSel(), str);
		int DepartureTime = atoi(str);

		m_TimeIntervalBox.GetLBText(m_TimeIntervalBox.GetCurSel(), str);
		int TimeInterval = atoi(str);

		m_MinVehicleSizeBox.GetLBText(m_MinVehicleSizeBox.GetCurSel(), str);
		int MinVehicleSize = atoi(str);

		m_MinDistanceBox.GetLBText(m_MinDistanceBox.GetCurSel(), str);
		int MinDistance = atoi(str);

		m_MinTTIBox.GetLBText(m_MinTTIBox.GetCurSel(), str);
		float MinTTI = atof(str);

		int VOT_LB = m_ComboBox_VOT_LB.GetCurSel()*5;
		int VOT_UB = m_ComboBox_VOT_UB.GetCurSel()*5;

		int count = 0;
		std::list<DTAVehicle*>::iterator iVehicle;

		for (iVehicle = m_pDoc->m_VehicleSet.begin(); iVehicle != m_pDoc->m_VehicleSet.end(); iVehicle++, count++)
		{
			DTAVehicle* pVehicle = (*iVehicle);
			if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete && (pVehicle->m_VOT >= VOT_LB && pVehicle->m_VOT < VOT_UB) )  // with physical path in the network
			{
				if( (pVehicle->m_OriginZoneID == Origin ||Origin ==0)&&
					(pVehicle->m_DestinationZoneID  == Destination ||Destination ==0)&&
					(pVehicle->m_DemandType  == DemandType ||DemandType ==0)&&
					(pVehicle->m_VehicleType  == VehicleType ||VehicleType ==0)&&
					(pVehicle->m_InformationClass  == InformationClass ||InformationClass ==0)&&
					(pVehicle->m_DepartureTime >= DepartureTime && pVehicle->m_DepartureTime <= DepartureTime+TimeInterval))
				{
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalVehicleSize+=1;
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalTravelTime += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalDistance += pVehicle->m_Distance;
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalCost += pVehicle->m_TollDollarCost;
					
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].emissiondata.Energy += pVehicle->m_EmissionData .Energy;
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].emissiondata.CO2 += pVehicle->m_EmissionData .CO2;
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].emissiondata.NOX += pVehicle->m_EmissionData .NOX;
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].emissiondata.CO += pVehicle->m_EmissionData .CO;
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].emissiondata.HC += pVehicle->m_EmissionData .HC;

					if(ImpactLinkNo>=0)
					{
					for(int link= 1; link<pVehicle->m_NodeSize; link++)
					{
						if ( pVehicle->m_NodeAry[link].LinkNo  == ImpactLinkNo)
						{
							m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].bImpactFlag = true;
						}

					}
					}
				}
			}
		}

		count = 0;
		m_ODList.ResetContent ();

		VehicleStatistics total_summary;

		for(i=1; i <= m_pDoc->m_ODSize ; i++)
			for(j=1; j<= m_pDoc->m_ODSize ; j++)
			{
				if(m_ODMOEMatrix[i][j].TotalVehicleSize>0)
				{
					float AvgDistance = m_ODMOEMatrix[i][j].TotalDistance /m_ODMOEMatrix[i][j].TotalVehicleSize;
					float AvgTravelTime = m_ODMOEMatrix[i][j].TotalTravelTime /m_ODMOEMatrix[i][j].TotalVehicleSize;
					float AvgCost = m_ODMOEMatrix[i][j].TotalCost  /m_ODMOEMatrix[i][j].TotalVehicleSize;

					float AvgEnergy = m_ODMOEMatrix[i][j].emissiondata .Energy / m_ODMOEMatrix[i][j].TotalVehicleSize;
					float AvgCO2 = m_ODMOEMatrix[i][j].emissiondata.CO2  / m_ODMOEMatrix[i][j].TotalVehicleSize;

					if(m_ODMOEMatrix[i][j].TotalVehicleSize >= MinVehicleSize && 
						AvgDistance >= MinDistance && AvgTravelTime> AvgDistance*MinTTI)
					{

						CString ODInfoString;

						if(m_pDoc->m_EmissionDataFlag )
							ODInfoString.Format ("%d->%d: %d vhc, %3.1f min, %3.1f mile, $%4.3f, %5.1f(J), %4.1f(CO2_g) ",i,j,m_ODMOEMatrix[i][j].TotalVehicleSize, AvgTravelTime,AvgDistance,AvgCost,AvgEnergy, AvgCO2);
						else
							ODInfoString.Format ("%d->%d: %d vhc, %3.1f min, %3.1f mile, $%4.3f",i,j,m_ODMOEMatrix[i][j].TotalVehicleSize, AvgTravelTime,AvgDistance,AvgCost);


						if(ImpactLinkNo<0)  // no impact link is selected
						{
						if(count<10000)
						{
							total_summary.TotalVehicleSize +=m_ODMOEMatrix[i][j].TotalVehicleSize;
							total_summary.TotalTravelTime +=m_ODMOEMatrix[i][j].TotalTravelTime;
							total_summary.TotalDistance +=m_ODMOEMatrix[i][j].TotalDistance;
							total_summary.TotalCost +=m_ODMOEMatrix[i][j].TotalCost;
							total_summary.emissiondata .Energy += m_ODMOEMatrix[i][j].emissiondata .Energy ;
							total_summary.emissiondata .CO2  += m_ODMOEMatrix[i][j].emissiondata .CO2  ;

							m_ODList.AddString (ODInfoString);
						}

						count ++;
						}else
						{ // ImpactLinkNo>0: impact link is selected
						
						if(count<10000 && m_ODMOEMatrix[i][j].bImpactFlag == true)
						{
							total_summary.TotalVehicleSize +=m_ODMOEMatrix[i][j].TotalVehicleSize;
							total_summary.TotalTravelTime +=m_ODMOEMatrix[i][j].TotalTravelTime;
							total_summary.TotalDistance +=m_ODMOEMatrix[i][j].TotalDistance;
							total_summary.TotalCost  +=m_ODMOEMatrix[i][j].TotalCost;
							total_summary.emissiondata .Energy += m_ODMOEMatrix[i][j].emissiondata .Energy ;
							total_summary.emissiondata .CO2  += m_ODMOEMatrix[i][j].emissiondata .CO2  ;

							m_ODList.AddString (ODInfoString);
						}

						count ++;
						
						}
					}
				}
			}

				CString SummaryInfoString;
				if(m_pDoc->m_EmissionDataFlag )
				{
				SummaryInfoString.Format ("%d vhc, %3.1f min, %3.1f mile,$%4.3f, %5.1f(J), %4.1f(CO2_g)  ",total_summary.TotalVehicleSize, 
					total_summary.TotalTravelTime/max(1,total_summary.TotalVehicleSize),
					total_summary.TotalDistance /max(1,total_summary.TotalVehicleSize),
					total_summary.TotalCost/max(1,total_summary.TotalVehicleSize),
					total_summary.emissiondata .Energy/max(1,total_summary.TotalVehicleSize),
					total_summary.emissiondata .CO2/max(1,total_summary.TotalVehicleSize));
				}else
				{
				SummaryInfoString.Format ("%d vhc, %3.1f min, %3.1f mile,$%4.3f",total_summary.TotalVehicleSize, 
					total_summary.TotalTravelTime/max(1,total_summary.TotalVehicleSize),
					total_summary.TotalDistance /max(1,total_summary.TotalVehicleSize),
					total_summary.TotalCost/max(1,total_summary.TotalVehicleSize));
				}


				m_Summary_Info_Edit.SetWindowText (SummaryInfoString);

			if(m_ODList.GetCount ()>0)
				m_ODList.SetCurSel (0);

			FilterPaths();
			ShowSelectedPath();
			ShowVehicles();
}
void CDlg_VehPathAnalysis::FilterPaths()
{
	m_PathVector.clear();
	m_PathList.ResetContent ();
	m_VehicleList.ResetContent ();

	int ODPairNo = m_ODList.GetCurSel();
	int Origin, Destination;

	if(ODPairNo>=0)	// if one of "all" options is selected, we need to narrow down to OD pair
	{
		char m_Text[MAX_STRING_LENGTH];
		m_ODList.GetText (ODPairNo, m_Text);
		sscanf(m_Text, "%d->%d", &Origin, &Destination);

	}

	int VehicleType = m_VehicleTypeBox.GetCurSel();
	int DemandType = m_DemandTypeBox.GetCurSel();
	
	int InformationClass = m_InformationClassBox.GetCurSel();

	char str[50];
	m_DepartureTimeBox.GetLBText(m_DepartureTimeBox.GetCurSel(), str);
	int DepartureTime = atoi(str);

	m_TimeIntervalBox.GetLBText(m_TimeIntervalBox.GetCurSel(), str);
	int TimeInterval = atoi(str);

	m_MinVehicleSizeBox.GetLBText(m_MinVehicleSizeBox.GetCurSel(), str);
	int MinVehicleSize = atoi(str);

	m_MinDistanceBox.GetLBText(m_MinDistanceBox.GetCurSel(), str);
	int MinDistance = atoi(str);

	m_MinTTIBox.GetLBText(m_MinTTIBox.GetCurSel(), str);
	float MinTTI = atof(str);

		int VOT_LB = m_ComboBox_VOT_LB.GetCurSel()*5;
		int VOT_UB = m_ComboBox_VOT_UB.GetCurSel()*5;

	int count = 0;
	std::list<DTAVehicle*>::iterator iVehicle;

	for (iVehicle = m_pDoc->m_VehicleSet.begin(); iVehicle != m_pDoc->m_VehicleSet.end(); iVehicle++, count++)
	{
		DTAVehicle* pVehicle = (*iVehicle);

		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete &&(pVehicle->m_VOT >= VOT_LB && pVehicle->m_VOT < VOT_UB))  // with physical path in the network
		{
			if( (pVehicle->m_OriginZoneID == Origin)&&
				(pVehicle->m_DestinationZoneID  == Destination)&&
				(pVehicle->m_VehicleType  == VehicleType ||VehicleType ==0)&&
				(pVehicle->m_DemandType  == DemandType ||DemandType ==0)&&
				(pVehicle->m_InformationClass  == InformationClass ||InformationClass ==0)&&
				(pVehicle->m_DepartureTime >= DepartureTime && pVehicle->m_DepartureTime <= DepartureTime+TimeInterval))
			{

				bool bFingFlag =  false;

				for(int p = 0; p< m_PathVector.size(); p++)
				{
					//existing path
					if(pVehicle->m_NodeNumberSum == m_PathVector[p].NodeNumberSum  && pVehicle->m_NodeSize == m_PathVector[p].NodeSize )
					{
						m_PathVector[p].TotalVehicleSize+=1;
						m_PathVector[p].TotalTravelTime  += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
						m_PathVector[p].TotalDistance   += pVehicle->m_Distance;
						m_PathVector[p].TotalCost   += pVehicle->m_TollDollarCost;
						m_PathVector[p].TotalEmissions   += pVehicle->m_Emissions;
					
						m_PathVector[p].emissiondata.Energy += pVehicle->m_EmissionData .Energy;
						m_PathVector[p].emissiondata.CO2 += pVehicle->m_EmissionData .CO2;
						m_PathVector[p].emissiondata.NOX += pVehicle->m_EmissionData .NOX;
						m_PathVector[p].emissiondata.CO += pVehicle->m_EmissionData .CO;
						m_PathVector[p].emissiondata.HC += pVehicle->m_EmissionData .HC;

						m_PathVector[p].m_VehicleVector.push_back(pVehicle);
						bFingFlag = true;
						break;
					}
				}
				if(bFingFlag == false)
				{
					// new path
					PathStatistics ps_element;
					ps_element.NodeNumberSum = pVehicle->m_NodeNumberSum;
					ps_element.NodeSize = pVehicle->m_NodeSize;
					ps_element.TotalVehicleSize = 1;
					ps_element.TotalTravelTime  += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
					ps_element.TotalDistance   += pVehicle->m_Distance;
					ps_element.TotalCost    += pVehicle->m_TollDollarCost ;
					ps_element.TotalEmissions    += pVehicle->m_Emissions ;
						ps_element.emissiondata.Energy += pVehicle->m_EmissionData .Energy;
						ps_element.emissiondata.CO2 += pVehicle->m_EmissionData .CO2;
						ps_element.emissiondata.NOX += pVehicle->m_EmissionData .NOX;
						ps_element.emissiondata.CO += pVehicle->m_EmissionData .CO;
						ps_element.emissiondata.HC += pVehicle->m_EmissionData .HC;

					ps_element.m_VehicleVector.push_back(pVehicle);


					for(int link= 1; link<pVehicle->m_NodeSize; link++)
					{
						ps_element.m_LinkVector.push_back(pVehicle->m_NodeAry[link].LinkNo);
					}

					m_PathVector.push_back (ps_element);
				}
			}
		}
	}

	for(int p = 0; p< m_PathVector.size(); p++)
	{
		float AvgDistance = m_PathVector[p].TotalDistance /m_PathVector[p].TotalVehicleSize;
		float AvgTravelTime = m_PathVector[p].TotalTravelTime /m_PathVector[p].TotalVehicleSize;
		float AvgTravelCost = m_PathVector[p].TotalCost /m_PathVector[p].TotalVehicleSize;
		float AvgEnergy = m_PathVector[p].emissiondata .Energy / m_PathVector[p].TotalVehicleSize;
		float AvgCO2 = m_PathVector[p].emissiondata.CO2  / m_PathVector[p].TotalVehicleSize;


		CString PathInfoString;
		if(AvgTravelCost>=0.001)
		{
			if(m_pDoc->m_EmissionDataFlag )
				PathInfoString.Format ("%d: %d vehicles, %3.1f min, %3.1f mile, $%3.2f, %5.1f(J), %4.1f(CO2_g) ",p+1, m_PathVector[p].TotalVehicleSize, AvgTravelTime,AvgDistance,AvgTravelCost,AvgEnergy, AvgCO2);
			else
				PathInfoString.Format ("%d: %d vehicles, %3.1f min, %3.1f mile, $%3.2f",p+1, m_PathVector[p].TotalVehicleSize, AvgTravelTime,AvgDistance,AvgTravelCost);

		}
		else
		{
			if(m_pDoc->m_EmissionDataFlag )
				PathInfoString.Format ("%d: %d vehicles, %3.1f min, %3.1f mile, %5.1f(J), %4.1f(CO2_g)",p+1, m_PathVector[p].TotalVehicleSize, AvgTravelTime,AvgDistance,AvgEnergy, AvgCO2);
			else
				PathInfoString.Format ("%d: %d vehicles, %3.1f min, %3.1f mile, $%3.2f",p+1, m_PathVector[p].TotalVehicleSize, AvgTravelTime,AvgDistance,AvgTravelCost);
		}

		m_PathList.AddString (PathInfoString);
	}

	m_PathList.SetCurSel(0);
	ShowSelectedPath();
	ShowVehicles();

}
void CDlg_VehPathAnalysis::ShowVehicles()
{
	m_VehicleList.ResetContent ();
	int PathNo = m_PathList.GetCurSel ();
	if(PathNo>=0)
	{
		for(unsigned int v = 0; v< m_PathVector[PathNo].m_VehicleVector.size(); v++)
		{
			DTAVehicle* pVehicle = m_PathVector[PathNo].m_VehicleVector[v];
			CString VehicleInfoString;
			if(pVehicle->m_TollDollarCost >=0.001)
			{
			if(m_pDoc->m_EmissionDataFlag )
				VehicleInfoString.Format ("No. %d, %s, @%3.1f min, %3.1f min, $%3.2f,%5.1f(J), %4.1f(CO2_g)",pVehicle->m_VehicleID , m_pDoc->GetPricingTypeStr(pVehicle->m_PricingType ), pVehicle->m_DepartureTime, (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime),pVehicle->m_TollDollarCost, pVehicle->m_EmissionData .Energy , pVehicle->m_EmissionData.CO2  );
			else
				VehicleInfoString.Format ("No. %d, %s, @%3.1f min, %3.1f min, $%3.2f",pVehicle->m_VehicleID , m_pDoc->GetPricingTypeStr(pVehicle->m_PricingType ),pVehicle->m_DepartureTime, (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime),pVehicle->m_TollDollarCost);

			}
			else
			{
			if(m_pDoc->m_EmissionDataFlag )
				VehicleInfoString.Format ("No. %d, %s, @%3.1f min, %3.1f min,%5.1f(J), %4.1f(CO2_g)",pVehicle->m_VehicleID , m_pDoc->GetPricingTypeStr(pVehicle->m_PricingType ), pVehicle->m_DepartureTime, (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime),pVehicle->m_EmissionData .Energy , pVehicle->m_EmissionData.CO2  );
			else
				VehicleInfoString.Format ("No. %d, %s, @%3.1f min, %3.1f min, $%3.2f",pVehicle->m_VehicleID , m_pDoc->GetPricingTypeStr(pVehicle->m_PricingType ), pVehicle->m_DepartureTime, (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime),pVehicle->m_TollDollarCost);
			}

			if(m_VehicleList.GetCount () <= 20000)
			{
				if(m_VehicleList.GetCount () == 20000)
				{
					m_VehicleList.AddString ("Reaching maximum number of items in a list..");
				}else
				m_VehicleList.AddString (VehicleInfoString);
			}
		}
	}
}
void CDlg_VehPathAnalysis::OnLbnSelchangeListLink()
{
	// TODO: Add your control notification handler code here
}


void CDlg_VehPathAnalysis::OnLbnSelchangeListVehicle()
{
}

void CDlg_VehPathAnalysis::OnLbnSelchangeListLink2()
{
	// TODO: Add your control notification handler code here
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboOrigin()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboDestination()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboDeparturetime()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboVehicletype()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboInformationclass()
{
	FilterOriginDestinationPairs();

}




void CDlg_VehPathAnalysis::OnCbnSelchangeComboTimeinterval()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboMinNumberofvehicles()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboMinTravelTime()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboMinTraveltimeindex()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnLbnSelchangeListOd()
{
	FilterPaths();
}

void CDlg_VehPathAnalysis::OnLbnSelchangeListPath()
{
	ShowSelectedPath();
	ShowVehicles();

}

void CDlg_VehPathAnalysis::ShowSelectedPath()
{
	int PathNo = m_PathList.GetCurSel();

	if(PathNo >= 0)
		m_pDoc->HighlightPath(m_PathVector[PathNo].m_LinkVector,1);

}


void CDlg_VehPathAnalysis::OnCbnSelchangeComboImpactlink()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnLbnDblclkListOd()
{
	FilterPaths();
}


void CDlg_VehPathAnalysis::OnBnClickedExport()
{
   CString str;
   CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		    "(*.csv)|*.csv||", NULL);
   if(dlg.DoModal() == IDOK)
   {
      char fname[_MAX_PATH];
      wsprintf(fname,"%s", dlg.GetPathName());
      CWaitCursor wait;

      if(!ExportDataToCSVFileAllOD(fname))
      {
		 str.Format("The file %s could not be opened.\nPlease check if it is opened by Excel.", fname);
		 AfxMessageBox(str);
      }else
	  {
		m_pDoc->OpenCSVFileInExcel(fname);
	  }
   }	
}
bool CDlg_VehPathAnalysis::ExportDataToCSVFileAllOD(char csv_file[_MAX_PATH])
{
     FILE* st;
      fopen_s(&st,csv_file,"w");

     if(st!=NULL)
      {
	 CWaitCursor wc;
	 fprintf(st,"origin_zone_id->destination_zone_id,total_number_of_vehicles,average_travel_time_in_min,average_distance_in_min,average_cost\n");
		for(int i=0; i< m_ODList.GetCount (); i++)	// if one of "all" options is selected, we need to narrow down to OD pair
		{
			char m_Text[200];
			m_ODList.GetText (i, m_Text);
			fprintf(st,"%s\n",m_Text);
		}
	 
		fclose(st);
		return true;
	 }

	 return false;
}


void CDlg_VehPathAnalysis::OnCbnSelchangeComboVotLb()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboVotUb()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnCbnSelchangeComboDemandtype()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehPathAnalysis::OnBnClickedFindSingleVehicleId()
{
	// TODO: Add your control notification handler code here
}

void CDlg_VehPathAnalysis::OnBnClickedExportPathData()
{
   CString str;
   CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		    "(*.csv)|*.csv||", NULL);
   if(dlg.DoModal() == IDOK)
   {
      char fname[_MAX_PATH];
      wsprintf(fname,"%s", dlg.GetPathName());
      CWaitCursor wait;

      if(!ExportPathDataToCSVFile(fname))
      {
		 str.Format("The file %s could not be opened.\nPlease check if it is opened by Excel.", fname);
		 AfxMessageBox(str);
      }else
	  {
	  m_pDoc->OpenCSVFileInExcel(fname);
	  }
   }	

}


bool CDlg_VehPathAnalysis::ExportPathDataToCSVFile(char csv_file[_MAX_PATH])
{
    FILE* st;
      fopen_s(&st,csv_file,"w");

     if(st!=NULL)
      {
	 CWaitCursor wc;
	 fprintf(st,"path number,total_number_of_vehicles,average_travel_time_in_min,average_distance_in_min\n");
		for(int i=0; i< m_PathList.GetCount (); i++)	// if one of "all" options is selected, we need to narrow down to OD pair
		{
			char m_Text[200];
			m_PathList.GetText (i, m_Text);
			fprintf(st,"%s\n",m_Text);
		}
	 
		fclose(st);
		return true;
	 }

	 return false;
}

bool CDlg_VehPathAnalysis::ExportVehicleDataToCSVFile(char csv_file[_MAX_PATH])
{
	int PathNo = m_PathList.GetCurSel ();
	if(PathNo>=0)
	{
		return m_pDoc->WriteSelectVehicleDataToCSVFile(csv_file,m_PathVector[PathNo].m_VehicleVector);
	}
	return false;
}

void CDlg_VehPathAnalysis::OnBnClickedExportVehicleData()
{
   CString str;
   CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		    "(*.csv)|*.csv||", NULL);
   if(dlg.DoModal() == IDOK)
   {
      char fname[_MAX_PATH];
      wsprintf(fname,"%s", dlg.GetPathName());
      CWaitCursor wait;

      if(!ExportVehicleDataToCSVFile(fname))
      {
		 str.Format("The file %s could not be opened.\nPlease check if it is opened by Excel.", fname);
		 AfxMessageBox(str);
      }else
	  {
	  m_pDoc->OpenCSVFileInExcel(fname);
	  }
   }	
}
