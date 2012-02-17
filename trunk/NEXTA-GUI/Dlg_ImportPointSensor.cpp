// Dlg_ImportPointSensor.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_ImportPointSensor.h"
#include "DlgSensorDataLoading.h"

// CDlg_ImportPointSensor dialog

IMPLEMENT_DYNAMIC(CDlg_ImportPointSensor, CDialog)

CDlg_ImportPointSensor::CDlg_ImportPointSensor(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_ImportPointSensor::IDD, pParent)
	, m_Sensor_File(_T(""))
{

}

CDlg_ImportPointSensor::~CDlg_ImportPointSensor()
{
}

void CDlg_ImportPointSensor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_Demand_CSV_File, m_Sensor_File);
	DDX_Control(pDX, IDC_LIST1, m_MessageList);
}


BEGIN_MESSAGE_MAP(CDlg_ImportPointSensor, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_Find_Sensor_File, &CDlg_ImportPointSensor::OnBnClickedButtonFindSensorFile)
	ON_BN_CLICKED(ID_IMPORT_Point_Sensor_Location_and_Data, &CDlg_ImportPointSensor::OnBnClickedImportPointSensorLocationandData)
END_MESSAGE_MAP()


// CDlg_ImportPointSensor message handlers
// Dlg_ImportPointSensor.cpp : implementation file
//


void CDlg_ImportPointSensor::OnBnClickedButtonFindSensorFile()
{
	static char BASED_CODE szFilter[] = "EXCEL 2003 Workbook (*.xls)|*.xls||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		m_Sensor_File = dlg.GetPathName();

		UpdateData(false);
	}
}

void CDlg_ImportPointSensor::OnBnClickedImportPointSensorLocationandData()
{
	CWaitCursor cursor;
	// Make sure the network is empty
	if(m_pDOC->m_LinkSet.size()==0)
	{
		AfxMessageBox("Please first load network data.");
		return;
	}

	m_MessageList.ResetContent ();

	bool bExist=true;

	CString strSQL;
	CString str_msg;

	UpdateData(true);

	if(m_pDOC->m_Database.IsOpen ())
		m_pDOC->m_Database.Close ();

	m_pDOC->m_Database.Open(m_Sensor_File, false, true, "excel 5.0; excel 97; excel 2000; excel 2003");

	// Open the EXCEL file
	std::string itsErrorMessage;

	// this accesses first sheet regardless of name.
	int i= 0;

	float Occ_to_Density_Coef = 100.0f;

	CDlgSensorDataLoading dlg;
	if(dlg.DoModal() ==IDOK)
	{
		m_pDOC->m_SamplingTimeInterval  = dlg.m_ObsTimeInterval;
		m_pDOC->m_AVISamplingTimeInterval  = dlg.m_AVISamplingTimeInterval ;
		m_pDOC->m_NumberOfDays = dlg.m_NumberOfDays;
		Occ_to_Density_Coef = dlg.m_Occ_to_Density_Coef;
		g_Simulation_Time_Horizon = 1440*m_pDOC->m_NumberOfDays;
	}
	// Read record

	CWaitCursor cursor_2;

	strSQL = m_pDOC->ConstructSQL("point-sensor-location");

	if(strSQL.GetLength() > 0)
	{
		// Read record
		CRecordsetExt rsSensorLocation(&m_pDOC->m_Database);
		rsSensorLocation.Open(dbOpenDynaset, strSQL);

		while(!rsSensorLocation.IsEOF())
		{
			DTA_sensor sensor;

			sensor.OrgSensorID =  rsSensorLocation.GetLong(CString("sensor_id"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field sensor_id cannot be found in the point-sensor-location table.");
				return;
			}

			sensor.SensorType =  rsSensorLocation.GetCString("sensor_type");

			bool b_find_link_flag = false;

			sensor.FromNodeNumber =  rsSensorLocation.GetLong(CString("from_node_id"),bExist,false);

			if(bExist)
			{

				sensor.ToNodeNumber =  rsSensorLocation.GetLong(CString("to_node_id"),bExist,false);
				if(!bExist)
				{
					AfxMessageBox("Field from_node_id cannot be found in the point-sensor-location table.");
					return;
				}
				sensor.RelativeLocationRatio = rsSensorLocation.GetLong(CString("relative_location_ratio"),bExist,false);
				if(!bExist)
				{
					AfxMessageBox("Field relative_location_ratio cannot be found in the point-sensor-location table.");
					return;
				}

				if(sensor.FromNodeNumber!= 0 && sensor.ToNodeNumber!=0)
				{
					DTALink* pLink = m_pDOC->FindLinkWithNodeNumbers(sensor.FromNodeNumber , sensor.ToNodeNumber, "point-sensor-location" );

					if(pLink!=NULL)
					{
						sensor.LinkID = pLink->m_LinkNo ;
						m_pDOC->m_SensorVector.push_back(sensor);
						m_pDOC->m_SensorIDtoLinkMap[sensor.OrgSensorID] = pLink;
						pLink->m_bSensorData  = true;
						pLink->ResetMOEAry (m_pDOC->m_NumberOfDays * 1440);

						b_find_link_flag  = true;

					}else
					{

						CString msg;
						msg.Format ("Link %d -> %d in point-sensor-location does not exit in the existing link data.");
						AfxMessageBox(msg);
						break;
						return;

					}
				}

			}

			if(b_find_link_flag  == false)  // no link from to nodes have been defined. 
			{
				float x = rsSensorLocation.GetDouble(CString("x"),bExist,false);
				if(!bExist)
				{
					AfxMessageBox("No link information is specified so field x is required in the point-sensor-location table.");
					return;
				}

				float y = rsSensorLocation.GetDouble(CString("y"),bExist,false);
				if(!bExist)
				{
					AfxMessageBox("No link information is specified so field y is required in the point-sensor-location table.");
					return;
				}
				CString orientation = rsSensorLocation.GetCString("orientation");

				sensor.LinkID = m_pDOC->FindLinkFromSensorLocation(x,y,orientation);

				if(sensor.LinkID > 0)
				{   
					DTALink* pLink = m_pDOC->FindLinkWithLinkNo(sensor.LinkID );
					sensor.FromNodeNumber  = pLink ->m_FromNodeNumber ;
					sensor.ToNodeNumber   = pLink ->m_ToNodeNumber  ;
					sensor.RelativeLocationRatio = 0.5;		
					pLink->ResetMOEAry (m_pDOC->m_NumberOfDays * 1440);
					m_pDOC->m_SensorVector.push_back(sensor);
				}

			}

			rsSensorLocation.MoveNext ();
		}
		rsSensorLocation.Close();
		str_msg.Format ( "%d sensors imported.", m_pDOC->m_SensorVector.size());
		m_MessageList.AddString (str_msg);
	}
	// point sensor data
	strSQL = m_pDOC->ConstructSQL("point-sensor-data");

	int number_of_samples = 0;

	if(strSQL.GetLength() > 0)
	{

		// Read record
		CRecordsetExt rsSensorData(&m_pDOC->m_Database);
		rsSensorData.Open(dbOpenDynaset, strSQL);

		while(!rsSensorData.IsEOF())
		{
			//			CString  unix_timestamp =  rsSensorData.GetCString("unix_timestamp_local_time");
			//			if(unix_timestamp.GetLength () <1)
			//				break;

			int  modeling_timestamp_day =  rsSensorData.GetLong(CString("modeling_timestamp_day"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field modeling_timestamp_day cannot be found in the point-sensor table.");
				return;
			}

			if(modeling_timestamp_day ==0)
				break;

			int  modeling_timestamp_min =  rsSensorData.GetLong(CString("modeling_timestamp_min"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field modeling_timestamp_min cannot be found in the point-sensor table.");
				return;
			}
			int  sensor_id =  rsSensorData.GetLong(CString("sensor_id"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field sensor_id cannot be found in the point-sensor table.");
				return;
			}

			float  total_link_flow_per_interval =  rsSensorData.GetDouble(CString("link_flow_per_time_interval"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field link_flow_per_time_interval cannot be found in the point-sensor table.");
				return;
			}

			float  occupancy =  rsSensorData.GetDouble(CString("occupancy"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field occupancy cannot be found in the point-sensor table.");
				return;
			}
			float  AvgLinkSpeed =  rsSensorData.GetDouble(CString("speed_in_mph"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field occupancy cannot be found in the point-sensor table.");
				return;
			}


			map<long, DTALink*>::iterator it;


			if ( (it = m_pDOC->m_SensorIDtoLinkMap.find(sensor_id)) != m_pDOC->m_SensorIDtoLinkMap.end()) 
			{
				DTALink* pLink = it->second;

				if(pLink!=NULL)
				{
					int t  = ((modeling_timestamp_day - 1)*1440+ modeling_timestamp_min) ;

					if(t>=0  && t< (pLink->m_LinkMOEAry.size()-m_pDOC->m_SamplingTimeInterval))
					{
						if(AvgLinkSpeed<=1)  // 0 or negative values means missing speed
							AvgLinkSpeed = pLink->m_SpeedLimit ;

						ASSERT(pLink->m_NumLanes > 0);

							pLink->m_LinkMOEAry[ t].ObsFlowCopy = total_link_flow_per_interval*60/m_pDOC->m_SamplingTimeInterval/pLink->m_NumLanes;  // convert to per hour link flow
							pLink->m_LinkMOEAry[ t].ObsSpeedCopy = AvgLinkSpeed; 
							pLink->m_LinkMOEAry[ t].ObsTravelTimeIndexCopy = pLink->m_SpeedLimit /max(1,AvgLinkSpeed)*100;

							if(occupancy <=0.001)
								pLink->m_LinkMOEAry[t].ObsDensityCopy = pLink->m_LinkMOEAry[t].ObsFlowCopy / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeedCopy);
							else
								pLink->m_LinkMOEAry[t].ObsDensityCopy = occupancy * Occ_to_Density_Coef;

							// copy data to other intervals
							for(int tt = 1; tt<m_pDOC->m_SamplingTimeInterval; tt++)
							{
								if(tt < pLink->m_LinkMOEAry.size())
								{
									pLink->m_LinkMOEAry[ t+tt].ObsFlowCopy = pLink->m_LinkMOEAry[t].ObsFlowCopy ;
									pLink->m_LinkMOEAry[t+tt].ObsSpeedCopy = pLink->m_LinkMOEAry[t].ObsSpeedCopy;
									pLink->m_LinkMOEAry[t+tt].ObsDensityCopy = pLink->m_LinkMOEAry[t].ObsDensityCopy;
									pLink->m_LinkMOEAry[t+tt].ObsTravelTimeIndexCopy = pLink->m_LinkMOEAry[t].ObsTravelTimeIndexCopy;
								}

							}
						}
					}
			number_of_samples++;
				}	
			

			rsSensorData.MoveNext ();
		}
		rsSensorData.Close  ();

		// build historical travel time
		std::list<DTALink*>::iterator iLink;

/*			for (iLink = m_pDOC->m_LinkSet.begin(); iLink != m_pDOC->m_LinkSet.end(); iLink++)
			{
				(*iLink)->ComputeHistoricalAvg(m_pDOC->m_NumberOfDays); 
			}
*/
		str_msg.Format("%d sensor data records loaded.",number_of_samples);
		m_MessageList.AddString (str_msg);
	}
}
