// Dlg_VehEmissions.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_VehEmissions.h"


// CDlg_VehEmissions dialog

IMPLEMENT_DYNAMIC(CDlg_VehEmissions, CDialog)

CDlg_VehEmissions::CDlg_VehEmissions(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_VehEmissions::IDD, pParent)
{

}

CDlg_VehEmissions::~CDlg_VehEmissions()
{
}

void CDlg_VehEmissions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_VEHICLE, m_VehicleList);
	DDX_Control(pDX, IDC_LIST_LINK, m_LinkList);
	DDX_Control(pDX, IDC_COMBO_Origin, m_OriginBox);
	DDX_Control(pDX, IDC_COMBO_Destination, m_DestinationBox);
	DDX_Control(pDX, IDC_COMBO_DepartureTime, m_DepartureTimeBox);
	DDX_Control(pDX, IDC_COMBO_VehicleType, m_VehicleTypeBox);
	DDX_Control(pDX, IDC_OpMode_LINK, m_OpModeList);
}


BEGIN_MESSAGE_MAP(CDlg_VehEmissions, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST_LINK, &CDlg_VehEmissions::OnLbnSelchangeListLink)
	ON_LBN_SELCHANGE(IDC_LIST_VEHICLE, &CDlg_VehEmissions::OnLbnSelchangeListVehicle)
	ON_LBN_SELCHANGE(IDC_LIST_LINK2, &CDlg_VehEmissions::OnLbnSelchangeListLink2)
END_MESSAGE_MAP()


// CDlg_VehEmissions message handlers


BOOL CDlg_VehEmissions::OnInitDialog()
{
	CDialog::OnInitDialog();

	int count = 0;
	std::list<DTAVehicle*>::iterator iVehicle;

	for (iVehicle = m_pDoc->m_VehicleSet.begin(); iVehicle != m_pDoc->m_VehicleSet.end(); iVehicle++, count++)
	{

		CString str;
		str.Format ("%d: %d -> %d, @%5.1f (min), %d links, TT: %5.1f (min)", (*iVehicle)->m_VehicleID, (*iVehicle)->m_OriginZoneID , (*iVehicle)->m_DestinationZoneID , (*iVehicle)->m_DepartureTime, (*iVehicle)->m_NodeSize-1, (*iVehicle)->m_TripTime);
		m_VehicleList.AddString (str);

		if(count >1000)  //only show the first 1000 vehicles
			break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
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
}

void CDlg_VehEmissions::OnLbnSelchangeListLink2()
{
	// TODO: Add your control notification handler code here
}
