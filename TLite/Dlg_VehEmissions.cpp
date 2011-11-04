// Dlg_VehEmissions.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_VehEmissions.h"
#define MAX_STRING_LENGTH  100


// CDlg_VehEmissions dialog

IMPLEMENT_DYNAMIC(CDlg_VehEmissions, CBaseDialog)

CDlg_VehEmissions::CDlg_VehEmissions(CWnd* pParent /*=NULL*/)
: CBaseDialog(CDlg_VehEmissions::IDD, pParent)
{

}

CDlg_VehEmissions::~CDlg_VehEmissions()
{

	if(m_ODMOEMatrix !=NULL)
		DeallocateDynamicArray<ODStatistics>(m_ODMOEMatrix,m_pDoc->m_ODSize+1,m_pDoc->m_ODSize+1);

}

void CDlg_VehEmissions::DoDataExchange(CDataExchange* pDX)
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
}


BEGIN_MESSAGE_MAP(CDlg_VehEmissions, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_LINK, &CDlg_VehEmissions::OnLbnSelchangeListLink)
	ON_LBN_SELCHANGE(IDC_LIST_VEHICLE, &CDlg_VehEmissions::OnLbnSelchangeListVehicle)
	ON_LBN_SELCHANGE(IDC_LIST_LINK2, &CDlg_VehEmissions::OnLbnSelchangeListLink2)
	ON_CBN_SELCHANGE(IDC_COMBO_Origin, &CDlg_VehEmissions::OnCbnSelchangeComboOrigin)
	ON_CBN_SELCHANGE(IDC_COMBO_Destination, &CDlg_VehEmissions::OnCbnSelchangeComboDestination)
	ON_CBN_SELCHANGE(IDC_COMBO_DepartureTime, &CDlg_VehEmissions::OnCbnSelchangeComboDeparturetime)
	ON_CBN_SELCHANGE(IDC_COMBO_VehicleType, &CDlg_VehEmissions::OnCbnSelchangeComboVehicletype)
	ON_CBN_SELCHANGE(IDC_COMBO_InformationClass, &CDlg_VehEmissions::OnCbnSelchangeComboInformationclass)
	ON_CBN_SELCHANGE(IDC_COMBO_TimeInterval, &CDlg_VehEmissions::OnCbnSelchangeComboTimeinterval)
	ON_CBN_SELCHANGE(IDC_COMBO_Min_Number_of_vehicles, &CDlg_VehEmissions::OnCbnSelchangeComboMinNumberofvehicles)
	ON_CBN_SELCHANGE(IDC_COMBO_Min_Travel_Time, &CDlg_VehEmissions::OnCbnSelchangeComboMinTravelTime)
	ON_CBN_SELCHANGE(IDC_COMBO_Min_TravelTimeIndex, &CDlg_VehEmissions::OnCbnSelchangeComboMinTraveltimeindex)
	ON_LBN_SELCHANGE(IDC_LIST_OD, &CDlg_VehEmissions::OnLbnSelchangeListOd)
	ON_LBN_SELCHANGE(IDC_LIST_PATH, &CDlg_VehEmissions::OnLbnSelchangeListPath)
	ON_CBN_SELCHANGE(IDC_COMBO_ImpactLink, &CDlg_VehEmissions::OnCbnSelchangeComboImpactlink)
	ON_LBN_DBLCLK(IDC_LIST_OD, &CDlg_VehEmissions::OnLbnDblclkListOd)
	ON_BN_CLICKED(ID_EXPORT, &CDlg_VehEmissions::OnBnClickedExport)
END_MESSAGE_MAP()


// CDlg_VehEmissions message handlers


BOOL CDlg_VehEmissions::OnInitDialog()
{


	CDialog::OnInitDialog();
	CString str;

	m_ImpactLinkBox.AddString ("N/A");
	std::list<DTALink*>::iterator iLink;
	for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
	{
		if((*iLink) ->CapacityReductionVector.size()>0)
		{
			str.Format ("%d->%d, incident", (*iLink) ->m_FromNodeNumber,(*iLink) ->m_ToNodeNumber );
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

	m_ODMOEMatrix = AllocateDynamicArray<ODStatistics>(m_pDoc->m_ODSize+1,m_pDoc->m_ODSize+1);

	m_VehicleTypeBox.AddString ("All");
	m_VehicleTypeBox.AddString ("LOV");
	m_VehicleTypeBox.AddString ("HOV");
	m_VehicleTypeBox.AddString ("Truck");
	m_VehicleTypeBox.SetCurSel (0);


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

	FilterOriginDestinationPairs();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlg_VehEmissions::FilterOriginDestinationPairs()
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
		int VehiclePricingType = m_VehicleTypeBox.GetCurSel();
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

		int count = 0;
		std::list<DTAVehicle*>::iterator iVehicle;

		for (iVehicle = m_pDoc->m_VehicleSet.begin(); iVehicle != m_pDoc->m_VehicleSet.end(); iVehicle++, count++)
		{
			DTAVehicle* pVehicle = (*iVehicle);

			if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
			{
				if( (pVehicle->m_OriginZoneID == Origin ||Origin ==0)&&
					(pVehicle->m_DestinationZoneID  == Destination ||Destination ==0)&&
					(m_pDoc->m_VehicleType2PricingTypeMap[pVehicle->m_VehicleType]  == VehiclePricingType ||VehiclePricingType ==0)&&
					(pVehicle->m_InformationClass  == InformationClass ||InformationClass ==0)&&
					(pVehicle->m_DepartureTime >= DepartureTime && pVehicle->m_DepartureTime <= DepartureTime+TimeInterval))
				{
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalVehicleSize+=1;
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalTravelTime += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
					m_ODMOEMatrix[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalDistance += pVehicle->m_Distance;

					if(ImpactLinkNo>=0)
					{
					for(int link= 1; link<pVehicle->m_NodeSize; link++)
					{
						if ( pVehicle->m_NodeAry[link].LinkID == ImpactLinkNo)
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

		for(i=1; i <= m_pDoc->m_ODSize ; i++)
			for(j=1; j<= m_pDoc->m_ODSize ; j++)
			{
				if(m_ODMOEMatrix[i][j].TotalVehicleSize>0)
				{
					float AvgDistance = m_ODMOEMatrix[i][j].TotalDistance /m_ODMOEMatrix[i][j].TotalVehicleSize;
					float AvgTravelTime = m_ODMOEMatrix[i][j].TotalTravelTime /m_ODMOEMatrix[i][j].TotalVehicleSize;

					if(m_ODMOEMatrix[i][j].TotalVehicleSize >= MinVehicleSize && 
						AvgDistance >= MinDistance && AvgTravelTime> AvgDistance*MinTTI)
					{

						CString ODInfoString;
						ODInfoString.Format ("%d->%d: %d vhc, %3.1f min, %3.1f mile",i,j,m_ODMOEMatrix[i][j].TotalVehicleSize, AvgTravelTime,AvgDistance);

						if(ImpactLinkNo<0)  // no impact link is selected
						{
						if(count<10000)
							m_ODList.AddString (ODInfoString);

						count ++;
						}else
						{ // ImpactLinkNo>0: impact link is selected
						
						if(count<10000 && m_ODMOEMatrix[i][j].bImpactFlag == true)
							m_ODList.AddString (ODInfoString);

						count ++;
						
						}
					}
				}
			}

			if(m_ODList.GetCount ()>0)
				m_ODList.SetCurSel (0);

			FilterPaths();
			ShowSelectedPath();
			ShowVehicles();
}
void CDlg_VehEmissions::FilterPaths()
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

	int VehiclePricingType = m_VehicleTypeBox.GetCurSel();
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

	int count = 0;
	std::list<DTAVehicle*>::iterator iVehicle;

	for (iVehicle = m_pDoc->m_VehicleSet.begin(); iVehicle != m_pDoc->m_VehicleSet.end(); iVehicle++, count++)
	{
		DTAVehicle* pVehicle = (*iVehicle);

		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
		{
			if( (pVehicle->m_OriginZoneID == Origin)&&
				(pVehicle->m_DestinationZoneID  == Destination)&&
				(m_pDoc->m_VehicleType2PricingTypeMap[pVehicle->m_VehicleType]  == VehiclePricingType ||VehiclePricingType ==0)&&
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
					ps_element.m_VehicleVector.push_back(pVehicle);


					for(int link= 1; link<pVehicle->m_NodeSize; link++)
					{
						ps_element.m_LinkVector.push_back(pVehicle->m_NodeAry[link].LinkID);
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

		CString PathInfoString;
		PathInfoString.Format ("%d: %d vehicles, %3.1f min, %3.1f mile",p+1, m_PathVector[p].TotalVehicleSize, AvgTravelTime,AvgDistance);
		m_PathList.AddString (PathInfoString);
	}

	m_PathList.SetCurSel(0);
	ShowSelectedPath();
	ShowVehicles();

}
void CDlg_VehEmissions::ShowVehicles()
{
	m_VehicleList.ResetContent ();
	int PathNo = m_PathList.GetCurSel ();
	if(PathNo>=0)
	{
		for(int v = 0; v< m_PathVector[PathNo].m_VehicleVector.size(); v++)
		{
			DTAVehicle* pVehicle = m_PathVector[PathNo].m_VehicleVector[v];
			CString VehicleInfoString;
			VehicleInfoString.Format ("No. %d, @%3.1f min, %3.1f min",pVehicle->m_VehicleID , pVehicle->m_DepartureTime, (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime) );
			m_VehicleList.AddString (VehicleInfoString);
		}
	}
}
void CDlg_VehEmissions::OnLbnSelchangeListLink()
{
	// TODO: Add your control notification handler code here
}


int CDlg_VehEmissions::GetOpMode(float vsp, float s_mph)
{
	int int_vsp = int(vsp/3)+1;
	int int_s_mph = int(s_mph/25)+1;

	std::map<int, int> OpMap;


	OpMap[	25	]=	11	;
	OpMap[	50	]=	21	;
	OpMap[	100	]=	33	;
	OpMap[	325	]=	12	;
	OpMap[	350	]=	22	;
	OpMap[	400	]=	33	;
	OpMap[	625	]=	13	;
	OpMap[	650	]=	23	;
	OpMap[	700	]=	35	;
	OpMap[	925	]=	14	;
	OpMap[	950	]=	24	;
	OpMap[	1000	]=	33	;
	OpMap[	1225	]=	15	;
	OpMap[	1250	]=	25	;
	OpMap[	1300	]=	35	;
	OpMap[	5025	]=	16	;
	OpMap[	5050	]=	26	;
	OpMap[	5100	]=	36;

	int key = 100*int_vsp+int_s_mph;
	return OpMap[key];
}
void CDlg_VehEmissions::OnLbnSelchangeListVehicle()
{

	/*
	float OpModeCount[40];
	int op;
	for( op= 0; op< 40; op++)
	{
	OpModeCount[op] = 0;
	}

	CString str;

	int CurSelNo = m_VehicleList.GetCurSel();

	m_LinkList.ResetContent ();

	if(CurSelNo >=0)
	{
	char m_Text[200];
	char m_Text_str[200];
	m_VehicleList.GetText (CurSelNo, m_Text);

	int vehicleID;

	sscanf(m_Text, "%d: %s", &vehicleID, m_Text_str);

	int count = 0;
	std::list<DTAVehicle*>::iterator iVehicle;
	DTAVehicle* thisVehicle = NULL;

	for (iVehicle = m_pDoc->m_VehicleSet.begin(); iVehicle != m_pDoc->m_VehicleSet.end(); iVehicle++, count++)
	{

	if( (*iVehicle)->m_VehicleID == vehicleID)
	{
	thisVehicle = (*iVehicle);
	break;
	}

	}

	for(int i = 0; i< thisVehicle->m_NodeSize-1; i++)
	{
	DTALink* pLink = m_pDoc->m_LinkNoMap[thisVehicle->m_NodeAry[i].LinkID];

	str.Format ("%d: %5.1f (min)", thisVehicle->m_NodeAry[i].LinkID, thisVehicle->m_NodeAry[i].ArrivalTimeOnDSN );

	float v = (thisVehicle->m_NodeAry[i+1].ArrivalTimeOnDSN - thisVehicle->m_NodeAry[i].ArrivalTimeOnDSN)*60/(pLink->m_Length/1600) ;
	float s_mph = (thisVehicle->m_NodeAry[i+1].ArrivalTimeOnDSN - thisVehicle->m_NodeAry[i].ArrivalTimeOnDSN)/60/(pLink->m_Length) ;
	float TermA = 0.156461f;
	float TermB = 0.00200193f;
	float Mass = 1.4788;
	float vsp = TermA/Mass*v + TermB/Mass*v*v+  TermB/Mass*v*v*v;
	int OpMode = GetOpMode(vsp,s_mph);
	//	OpModeCount[OpMode]+=1;

	m_LinkList.AddString (str);

	}

	m_LinkList.AddString("All Links");

	OpModeCount[11] = 5;
	OpModeCount[13] = 10;
	OpModeCount[15] = 3;
	OpModeCount[22] = 13;
	OpModeCount[24] = 12;

	float sum = 0;
	for( op= 10; op< 40; op++)
	{
	sum+=OpModeCount[op];
	}


	for( op= 10; op< 40; op++)
	{
	if(OpModeCount[op]>=1)
	{
	str.Format ("%d: %f%%", op,OpModeCount[op]/sum*100);
	m_OpModeList.AddString (str);
	}
	}
	}
	*/
}

void CDlg_VehEmissions::OnLbnSelchangeListLink2()
{
	// TODO: Add your control notification handler code here
}

void CDlg_VehEmissions::OnCbnSelchangeComboOrigin()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehEmissions::OnCbnSelchangeComboDestination()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehEmissions::OnCbnSelchangeComboDeparturetime()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehEmissions::OnCbnSelchangeComboVehicletype()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehEmissions::OnCbnSelchangeComboInformationclass()
{
	FilterOriginDestinationPairs();

}




void CDlg_VehEmissions::OnCbnSelchangeComboTimeinterval()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehEmissions::OnCbnSelchangeComboMinNumberofvehicles()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehEmissions::OnCbnSelchangeComboMinTravelTime()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehEmissions::OnCbnSelchangeComboMinTraveltimeindex()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehEmissions::OnLbnSelchangeListOd()
{
	FilterPaths();
}

void CDlg_VehEmissions::OnLbnSelchangeListPath()
{
	ShowSelectedPath();
	ShowVehicles();

}

void CDlg_VehEmissions::ShowSelectedPath()
{
	int PathNo = m_PathList.GetCurSel();

	if(PathNo >= 0)
		m_pDoc->SelectPath(m_PathVector[PathNo].m_LinkVector,1);

}


void CDlg_VehEmissions::OnCbnSelchangeComboImpactlink()
{
	FilterOriginDestinationPairs();
}

void CDlg_VehEmissions::OnLbnDblclkListOd()
{
	FilterPaths();
}


void CDlg_VehEmissions::OnBnClickedExport()
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
      }
   }	
}
bool CDlg_VehEmissions::ExportDataToCSVFileAllOD(char csv_file[_MAX_PATH])
{
     FILE* st;
      fopen_s(&st,csv_file,"w");

     if(st!=NULL)
      {
	 CWaitCursor wc;
	 fprintf(st,"origin_zone_id,destination_zone_id,total_number_of_vehicles,average_travel_time_in_min,average_distance_in_min\n");
		for(int i=0; i< m_ODList.GetCount (); i++)	// if one of "all" options is selected, we need to narrow down to OD pair
		{
			char m_Text[MAX_STRING_LENGTH];
			int Origin, Destination,TotalVehicleSize;
			float AvgTravelTime,AvgDistance;

			m_ODList.GetText (i, m_Text);
			sscanf(m_Text, "%d->%d: %d vhc, %3.1f min, %3.1f mile", &Origin, &Destination,&TotalVehicleSize, &AvgTravelTime,&AvgDistance);
			fprintf(st,"%d,%d,%d,%3.1f,%3.1f\n",Origin,Destination,TotalVehicleSize,AvgTravelTime,AvgDistance);
		}
	 
		fclose(st);
		return true;
	 }

	 return false;
}

