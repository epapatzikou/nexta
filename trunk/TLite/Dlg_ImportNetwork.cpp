// Dlg_ImportNetwork.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_ImportNetwork.h"
#include "DlgSensorDataLoading.h"


// CDlg_ImportNetwork dialog

IMPLEMENT_DYNAMIC(CDlg_ImportNetwork, CDialog)

CDlg_ImportNetwork::CDlg_ImportNetwork(CWnd* pParent /*=NULL*/)
: CDialog(CDlg_ImportNetwork::IDD, pParent)
, m_Edit_Excel_File(_T(""))
, m_Edit_Demand_CSV_File(_T(""))
, m_Sensor_File(_T(""))
{
    m_bImportNetworkOnly = false;
}

CDlg_ImportNetwork::~CDlg_ImportNetwork()
{
	if(m_pDOC->m_Database.IsOpen ())
		m_pDOC->m_Database.Close ();

}

void CDlg_ImportNetwork::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ExcelFile, m_Edit_Excel_File);
	DDX_Text(pDX, IDC_EDIT_Demand_CSV_File, m_Edit_Demand_CSV_File);
	DDX_Control(pDX, IDC_LIST1, m_MessageList);
	DDX_Text(pDX, IDC_EDIT_SENSOR_FILE, m_Sensor_File);
}


BEGIN_MESSAGE_MAP(CDlg_ImportNetwork, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_Find_Exel_File, &CDlg_ImportNetwork::OnBnClickedButtonFindExelFile)
	ON_BN_CLICKED(IDC_BUTTON_Find_Demand_CSV_File, &CDlg_ImportNetwork::OnBnClickedButtonFindDemandCsvFile)
	ON_BN_CLICKED(ID_IMPORT, &CDlg_ImportNetwork::OnBnClickedImport)
	ON_BN_CLICKED(ID_IMPORT_Network_Only, &CDlg_ImportNetwork::OnBnClickedImportNetworkOnly)
	ON_BN_CLICKED(IDC_BUTTON_Find_Sensor_File, &CDlg_ImportNetwork::OnBnClickedButtonFindSensorFile)
	ON_BN_CLICKED(ID_IMPORT_SENSOR_DATA, &CDlg_ImportNetwork::OnBnClickedImportSensorData)
END_MESSAGE_MAP()


// CDlg_ImportNetwork message handlers

void CDlg_ImportNetwork::OnBnClickedButtonFindExelFile()
{
	static char BASED_CODE szFilter[] = "EXCEL 2003 Workbook (*.xls)|*.xls||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		m_Edit_Excel_File = dlg.GetPathName();

		UpdateData(false);
	}

}

void CDlg_ImportNetwork::OnBnClickedButtonFindDemandCsvFile()
{
	static char BASED_CODE szFilter[] = "Demand CSV file (*.csv)|*.csv||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		m_Edit_Demand_CSV_File = dlg.GetPathName();

		UpdateData(false);
	}
}
void CDlg_ImportNetwork::OnBnClickedButtonFindSensorFile()
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

void CDlg_ImportNetwork::OnBnClickedImport()
{
	CWaitCursor cursor;
	// Make sure the network is empty
	m_pDOC->m_NodeSet.clear ();
	m_pDOC->m_LinkSet.clear ();
	m_MessageList.ResetContent ();

	bool bExist=true;

	CString strSQL;
	CString str_msg;

	UpdateData(true);

	if(m_pDOC->m_Database.IsOpen ())
		m_pDOC->m_Database.Close ();

	m_pDOC->m_Database.Open(m_Edit_Excel_File, false, true, "excel 5.0; excel 97; excel 2000; excel 2003");

	// Open the EXCEL file
	std::string itsErrorMessage;

	// this accesses first sheet regardless of name.
	int i= 0;


	// Read record

	strSQL = m_pDOC->ConstructSQL("1-NODE");

	if(strSQL.GetLength() > 0)
	{
		CRecordsetExt rsNode(&m_pDOC->m_Database);
		rsNode.Open(dbOpenDynaset, strSQL);

		while(!rsNode.IsEOF())
		{
			int id = rsNode.GetLong(CString("node_id"),bExist,false);

			if(!bExist)
			{
				m_MessageList.AddString ("Field node_id cannot be found in the node table.");
				rsNode.Close();
				return;
			}

			if(id < 0)
			{
				str_msg.Format ( "node_id: %d at row %d is invalid. Please check node table.", id, i+1);
				m_MessageList.AddString (str_msg);
				rsNode.Close();
				return;
			}


			float x = rsNode.GetDouble(CString("x"),bExist,false);
			if(!bExist) 
			{
				m_MessageList.AddString ("Field x cannot be found in the node table.");
				rsNode.Close();
				return;
			}

			float y = rsNode.GetDouble(CString("y"),bExist,false);
			if(!bExist) 
			{
				m_MessageList.AddString ("Field y cannot be found in the node table.");
				rsNode.Close();
				return;
			}

			// Create and insert the node
			DTANode* pNode = new DTANode;
			pNode->pt.x = x;
			pNode->pt.y = y;

			pNode->m_NodeNumber = id;
			pNode->m_NodeID = i;
			pNode->m_ZoneID = 0;
			m_pDOC->m_NodeSet.push_back(pNode);
			m_pDOC->m_NodeIDMap[i] = pNode;
			m_pDOC->m_NodeIDtoNameMap[i] = id;
			m_pDOC->m_NodeNametoIDMap[id] = i;
			i++;

			rsNode.MoveNext();
		}	// end of while
		rsNode.Close();
		str_msg.Format ( "%d nodes have been successfully imported.",m_pDOC->m_NodeSet.size());
		m_MessageList.AddString (str_msg);

	}else
	{
		str_msg.Format ( "Worksheet 1-node cannot be found in the given Excel file");
		m_MessageList.AddString (str_msg);
		return;
	}

	if(m_pDOC->m_NodeSet.size() > 0)
	{
		// Read record
		strSQL = m_pDOC->ConstructSQL("2-LINK");

		if(strSQL.GetLength () > 0)
		{
			CRecordsetExt rsLink(&m_pDOC->m_Database);
			rsLink.Open(dbOpenDynaset, strSQL);
			i = 0;
			int line_no = 2;
			float default_distance_sum = 0;
			float length_sum = 0;
			while(!rsLink.IsEOF())
			{

				int from_node_id = rsLink.GetLong(CString("from_node_id"),bExist,false);
				if(!bExist)
				{
					m_MessageList.AddString ("Field from_node_id cannot be found in the link table.");
					rsLink.Close();
					return;
				}

				int to_node_id = rsLink.GetLong(CString("to_node_id"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString("Field to_node_id cannot be found in the link table.");
					rsLink.Close();
					return;
				}


				if(from_node_id==0 ||to_node_id ==0)
					break;

				long link_id =  rsLink.GetLong(CString("link_id"),bExist,false);
				if(!bExist)
					link_id = 0;

				if(m_pDOC->m_NodeNametoIDMap.find(from_node_id)== m_pDOC->m_NodeNametoIDMap.end())
				{
					str_msg.Format("from_node_id %d at row %d cannot be found in the node table!",from_node_id, line_no);
					m_MessageList.AddString(str_msg);
					rsLink.Close();
					return;
				}

				if(m_pDOC->m_NodeNametoIDMap.find(to_node_id)== m_pDOC->m_NodeNametoIDMap.end())
				{
					str_msg.Format("to_node_id %d at row %d cannot be found in the node table!",to_node_id, line_no);
					m_MessageList.AddString(str_msg);
					rsLink.Close();
					return;
				}

				DTALink* pExistingLink =  m_pDOC->FindLinkWithNodeIDs(m_pDOC->m_NodeNametoIDMap[from_node_id],m_pDOC->m_NodeNametoIDMap[to_node_id]);

				if(pExistingLink)
				{
					str_msg.Format ("Link %d-> %d at row %d is duplicated with the previous link at row %d.\n", from_node_id,to_node_id, line_no, pExistingLink->input_line_no);
					if(m_MessageList.GetCount () < 3000)  // not adding and showing too many links
					{
						m_MessageList.AddString (str_msg);
						rsLink.MoveNext();
						continue;
					}
				}

				float length = rsLink.GetDouble(CString("length_in_mile"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString ("Field length_in_mile cannot be found in the link table.");
					rsLink.Close();
					return;
				}
				if(length > 100)
				{
					str_msg.Format("The length of link %d -> %d is longer than 100 miles, please ensure the unit of link length in the link sheet is mile.",from_node_id,to_node_id);
					m_MessageList.AddString(str_msg);
					rsLink.Close();
					return;
				}

				int number_of_lanes = rsLink.GetLong(CString("number_of_lanes"),bExist,false);
				if(!bExist)
				{
					m_MessageList.AddString("Field number_of_lanes cannot be found in the link table.");
					rsLink.Close();
					return;
				}

				float grade= rsLink.GetDouble(CString("grade"),bExist,false);

				float speed_limit_in_mph= rsLink.GetLong(CString("speed_limit_in_mph"),bExist,false);
				if(!bExist) 
				{
					AfxMessageBox("Field speed_limit_in_mph cannot be found in the link table.");
					rsLink.Close();
					return;
				}

				if(speed_limit_in_mph==0)
				{
					str_msg.Format ("Link %d -> %d has a speed limit of 0, please sort the link table by speed_limit_in_mph and re-check it!",from_node_id,to_node_id);
					AfxMessageBox(str_msg);
				}

				float capacity_in_pcphpl= rsLink.GetDouble(CString("lane_capacity_in_vhc_per_hour"),bExist,false);
				if(!bExist)
				{
					m_MessageList.AddString ("Field capacity_in_veh_per_hour_per_lane cannot be found in the link table.");
					rsLink.Close();
					return;
				}

				if(capacity_in_pcphpl<0)
				{
					str_msg.Format ( "Link %d -> %d has a negative capacity, please sort the link table by capacity_in_veh_per_hour_per_lane and re-check it!",from_node_id,to_node_id);
					AfxMessageBox(str_msg, MB_ICONINFORMATION);
					rsLink.Close();
					return;
				}

				int type = rsLink.GetLong(CString("link_type"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString("Field link_type cannot be found in the link table.");
					rsLink.Close();
					return;
				}

				int direction = rsLink.GetLong(CString("direction"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString("Field direction cannot be found in the link table.");
					rsLink.Close();
					return;
				}

				CString name = rsLink.GetCString(CString("name"));
				if(!bExist) 
				{
					m_MessageList.AddString("Field name cannot be found in the link table.");
					rsLink.Close();
					return;
				}

				float k_jam, wave_speed_in_mph;

				if(type==1)
				{
					k_jam = 220;
				}else
				{
					k_jam = 120;
				}

				wave_speed_in_mph = 12;


				int m_SimulationHorizon = 1;

				int link_code_start = 1;
				int link_code_end = 1;

				if (direction == -1) // reversed
				{
					link_code_start = 2; link_code_end = 2;
				}

				if (direction == 0) // two-directional link
				{
					link_code_start = 1; link_code_end = 2;
				}

				// no geometry information
				CCoordinate cc_from, cc_to; 
				cc_from.X = m_pDOC->m_NodeIDMap[m_pDOC->m_NodeNametoIDMap[from_node_id]]->pt.x;
				cc_from.Y = m_pDOC->m_NodeIDMap[m_pDOC->m_NodeNametoIDMap[from_node_id]]->pt.y;

				cc_to.X = m_pDOC->m_NodeIDMap[m_pDOC->m_NodeNametoIDMap[to_node_id]]->pt.x;
				cc_to.Y = m_pDOC->m_NodeIDMap[m_pDOC->m_NodeNametoIDMap[to_node_id]]->pt.y;

				std::vector<CCoordinate> CoordinateVector;

				CoordinateVector.push_back(cc_from);
				CoordinateVector.push_back(cc_to);


				for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
				{

					bool bNodeNonExistError = false;
					int m_SimulationHorizon = 1;
					DTALink* pLink = new DTALink(m_SimulationHorizon);
					pLink->m_LinkNo = i;
					pLink->m_Name  = name;
					pLink->m_OrgDir = direction;
					pLink->m_LinkID = link_id;

					if(link_code == 1)  //AB link
					{
						pLink->m_FromNodeNumber = from_node_id;

						pLink->m_ToNodeNumber = to_node_id;
						pLink->m_Direction  = 1;

						for(unsigned si = 0; si < CoordinateVector.size(); si++)
						{
							GDPoint	pt;
							pt.x = CoordinateVector[si].X;
							pt.y = CoordinateVector[si].Y;
							pLink->m_ShapePoints .push_back (pt);
						}

					}

					if(link_code == 2)  //BA link
					{
						pLink->m_FromNodeNumber = to_node_id;
						pLink->m_ToNodeNumber = from_node_id;
						pLink->m_Direction  = 1;

						for(unsigned si = CoordinateVector.size()-1; si >=0; si--)
						{
							GDPoint	pt;
							pt.x = CoordinateVector[si].X;
							pt.y = CoordinateVector[si].Y;
							pLink->m_ShapePoints .push_back (pt);
						}
					}


					pLink->m_FromNodeID = m_pDOC->m_NodeNametoIDMap[from_node_id];
					pLink->m_ToNodeID= m_pDOC->m_NodeNametoIDMap[to_node_id];

					m_pDOC->m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
					m_pDOC->m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;

					unsigned long LinkKey = m_pDOC->GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
					m_pDOC->m_NodeIDtoLinkMap[LinkKey] = pLink;

					pLink->m_NumLanes= number_of_lanes;
					pLink->m_SpeedLimit= speed_limit_in_mph;
					pLink->m_StaticSpeed = pLink->m_SpeedLimit;
					pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance
					pLink->m_MaximumServiceFlowRatePHPL= capacity_in_pcphpl;
					pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
					pLink->m_link_type= type;
					pLink->m_Grade = grade;


					if(link_code == 2)  //BA link
					{

						int R_number_of_lanes = rsLink.GetLong(CString("R_number_of_lanes"),bExist,false);
						if(!bExist) 
						{
							m_MessageList.AddString("Field R_number_of_lanes cannot be found in the link table.");
							rsLink.Close();
							return;
						}
						float R_speed_limit_in_mph= rsLink.GetLong(CString("R_speed_limit_in_mph"),bExist,false);
						if(!bExist) 
						{
							m_MessageList.AddString("Field R_speed_limit_in_mph cannot be found in the link table.");
							rsLink.Close();
							return;
						}
						float R_lane_capacity_in_vhc_per_hour= rsLink.GetDouble(CString("R_lane_capacity_in_vhc_per_hour"),bExist,false);
						if(!bExist) 
						{
							m_MessageList.AddString("Field R_lane_capacity_in_vhc_per_hour cannot be found in the link table.");
							rsLink.Close();
							return;
						}
						float R_grade= rsLink.GetDouble(CString("R_grade"),bExist,false);
						if(!bExist) 
						{
							m_MessageList.AddString("Field R_grade cannot be found in the link table.");
							rsLink.Close();
							return;
						}
						pLink->m_NumLanes= R_number_of_lanes;
						pLink->m_SpeedLimit= R_speed_limit_in_mph;
						pLink->m_StaticSpeed = pLink->m_SpeedLimit;
						pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance
						pLink->m_MaximumServiceFlowRatePHPL= R_lane_capacity_in_vhc_per_hour;
						pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
						pLink->m_link_type= type;
						pLink->m_Grade = R_grade;


					}

					pLink->m_Kjam = k_jam;
					pLink->m_Wave_speed_in_mph  = wave_speed_in_mph;

					m_pDOC->m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);

					pLink->m_FromPoint = m_pDOC->m_NodeIDMap[pLink->m_FromNodeID]->pt;
					pLink->m_ToPoint = m_pDOC->m_NodeIDMap[pLink->m_ToNodeID]->pt;
					default_distance_sum+= pLink->DefaultDistance();
					length_sum += pLink ->m_Length;
					//			pLink->SetupMOE();
					pLink->input_line_no  = line_no;
					m_pDOC->m_LinkSet.push_back (pLink);
					m_pDOC->m_LinkNoMap[i]  = pLink;
					i++;

				}

				rsLink.MoveNext();
//				TRACE("reading line %d\n", line_no);
			 line_no ++;
			}

			rsLink.Close();

			m_pDOC->m_UnitMile  = 1.0f;

			if(length_sum>0.000001f)
				m_pDOC->m_UnitMile=  default_distance_sum /length_sum;

			m_pDOC->m_UnitFeet = m_pDOC->m_UnitMile/5280.0f;  

			/*
			if(m_UnitMile>50)  // long/lat must be very large and greater than 62!
			{

			if(AfxMessageBox("Is the long/lat coordinate system used in this data set?", MB_YESNO) == IDYES)
			{
			m_LongLatCoordinateFlag = true;
			m_UnitFeet = m_UnitMile/62/5280.0f;  // 62 is 1 long = 62 miles
			}
			}
			*/

			m_pDOC->OffsetLink();

		}else
		{
			str_msg.Format ( "Worksheet 2-link cannot be found in the given Excel file");
			m_MessageList.AddString (str_msg);
			return;
		}


		str_msg.Format ("%d links have been sucessfully imported.",m_pDOC->m_LinkSet.size());
		m_MessageList.AddString(str_msg);
	}


	// Read record
	strSQL = m_pDOC->ConstructSQL("2-1-LINK-TYPE");
	if(strSQL.GetLength () > 0)
	{

		CRecordsetExt rsLinkType(&m_pDOC->m_Database);
		rsLinkType.Open(dbOpenDynaset, strSQL);

		while(!rsLinkType.IsEOF())
		{
			DTALinkType element;
			int link_type_number = rsLinkType.GetLong(CString("link_type"),bExist,false);
			if(!bExist) 
			{
				m_MessageList.AddString ("Field link_type cannot be found in input_link_type.csv.");
				return;
			}
			if(link_type_number ==0)
				break;

			element.link_type = link_type_number;
			element.link_type_name  = rsLinkType.GetCString(CString("link_type_name"));
			element.freeway_flag   = rsLinkType.GetLong (CString("freeway_flag"),bExist,false);
			if(!bExist) 
			{
				m_MessageList.AddString ("Field freeway_flag cannot be found in the input_link_type.csv.");
				return;
			}

			element.ramp_flag   = rsLinkType.GetLong (CString("ramp_flag"),bExist,false);
			if(!bExist) 
			{
				m_MessageList.AddString("Field ramp_flag cannot be found in the input_link_type.csv.");
				return;
			}

			element.arterial_flag    = rsLinkType.GetLong (CString("arterial_flag"),bExist,false);
			if(!bExist)
			{
				m_MessageList.AddString("Field arterial_flag cannot be found in the input_link_type.csv.");
				return;
			}

			m_pDOC->m_LinkTypeFreewayMap[element.link_type] = element.freeway_flag ;
			m_pDOC->m_LinkTypeArterialMap[element.link_type] = element.arterial_flag  ;
			m_pDOC->m_LinkTypeRampMap[element.link_type] = element.ramp_flag  ;

			m_pDOC->m_LinkTypeVector.push_back(element);

			rsLinkType.MoveNext ();
		}
		rsLinkType.Close();
		str_msg.Format ("%d link type defintions imported",m_pDOC->m_LinkTypeVector.size());
		m_MessageList.AddString(str_msg);
	}else
	{
		str_msg.Format ( "Worksheet 2-1-link-type cannot be found in the given Excel file");
		m_MessageList.AddString (str_msg);
		return;
	}

	if(m_bImportNetworkOnly == true)
		return;

	// ZONE table
	strSQL = m_pDOC->ConstructSQL("3-ZONE");

	bool bNodeNonExistError = false;
	m_pDOC->m_NodeIDtoZoneNameMap.clear ();

	m_pDOC->m_ODSize = 0;

	// Read record
	if(strSQL.GetLength () > 0)
	{
	CRecordsetExt rsZone(&m_pDOC->m_Database);
	rsZone.Open(dbOpenDynaset, strSQL);

	while(!rsZone.IsEOF())
	{
		int zone_number = rsZone.GetLong(CString("zone_id"),bExist,false);
		if(!bExist) 
		{
			AfxMessageBox("Field zone_id cannot be found in the zone table.");
			return;
		}

		if(zone_number ==0)
			break;

		int node_name = rsZone.GetLong(CString("node_id"),bExist,false);

		if(!bExist) 
		{
			AfxMessageBox("Field node_id cannot be found in the zone table.");
			return;
		}

		map <int, int> :: const_iterator m_Iter = m_pDOC->m_NodeNametoIDMap.find(node_name);

		if(m_Iter == m_pDOC->m_NodeNametoIDMap.end( ))
		{
			CString m_Warning;
			m_Warning.Format("Node Number %d in the zone tabe has not be defined in the node table", node_name);
			AfxMessageBox(m_Warning);
			return;
		}
		m_pDOC->m_NodeIDtoZoneNameMap[m_pDOC->m_NodeNametoIDMap[node_name]] = zone_number;
		// if there are multiple nodes for a zone, the last node id is recorded.
		int zoneid  = zone_number-1;
		m_pDOC->m_ZoneIDtoNodeIDMap[zoneid] = m_pDOC->m_NodeNametoIDMap[node_name];

		if(m_pDOC->m_ODSize < zone_number)
			m_pDOC->m_ODSize = zone_number;

		rsZone.MoveNext ();
	}
	rsZone.Close();

	str_msg.Format ( "%d node-to-zone records imported", m_pDOC->m_ZoneIDtoNodeIDMap.size());
	m_MessageList.AddString (str_msg);

	}

	/////

	strSQL = m_pDOC->ConstructSQL("4-2-vehicle-type");
	

	if (strSQL.GetLength() > 0)
	{
	CRecordsetExt rsVehicleType(&m_pDOC->m_Database);
	rsVehicleType.Open(dbOpenDynaset, strSQL);

	while(!rsVehicleType.IsEOF())
	{
		int type_no;
		float percentage;

		type_no = rsVehicleType.GetLong(CString("vehicle_type"),bExist,false);
		if(!bExist)
		{
			AfxMessageBox("Field vehicle_type cannot be found in the 4-2-vehicle-type table.");
			return;
		}
		if(type_no <=0)
			break;

		percentage = rsVehicleType.GetDouble(CString("percentage_of_base_demand_matrix"),bExist,false);
		if(!bExist)
		{
			AfxMessageBox("Field percentage_of_base_demand_matrix cannot be found in the 4-2-vehicle-type table.");
			return;
		}

		int pricing_type = rsVehicleType.GetDouble(CString("pricing_type"),bExist,false);
		if(!bExist)
		{
			pricing_type = type_no;  // default value
		}


		float average_VOT;
		average_VOT =  rsVehicleType.GetDouble(CString("average_VOT"),bExist,false);
		if(!bExist)
		{
			AfxMessageBox("Field average_VOT cannot be found in the 4-2-vehicle-type table.");
		}

		CString vehicle_type_name;
		CString pricing_type_name;

		vehicle_type_name = rsVehicleType.GetCString(CString("vehicle_type_name"));

		pricing_type_name = rsVehicleType.GetCString(CString("pricing_type_name"));

		DTAVehicleType element;
		element.vehicle_type = type_no;
		element.percentage  = percentage;
		element.pricing_type = pricing_type;
		element.vehicle_type_name = vehicle_type_name;
		element.pricing_type_name = pricing_type_name;
		element.average_VOT = average_VOT;

		m_pDOC->m_VehicleTypeVector.push_back(element);

		rsVehicleType.MoveNext ();
	}
	rsVehicleType.Close();

	str_msg.Format ( "%d vehicle types imported", m_pDOC->m_VehicleTypeVector.size());
	m_MessageList.AddString (str_msg);

	}

	strSQL = m_pDOC->ConstructSQL("4-PEAK-HOUR-DEMAND");

	if(strSQL.GetLength() > 0)
	{
	// Read record

	float total_demand = 0;
	CRecordsetExt rsDemand(&m_pDOC->m_Database);
	rsDemand.Open(dbOpenDynaset, strSQL);

	if(m_pDOC->m_DemandMatrix!=NULL)
	{
		DeallocateDynamicArray(m_pDOC->m_DemandMatrix,m_pDOC->m_ODSize,m_pDOC->m_ODSize);
		m_pDOC->m_DemandMatrix = NULL;
	}

	m_pDOC->m_DemandMatrix   =  AllocateDynamicArray<float>(m_pDOC->m_ODSize,m_pDOC->m_ODSize);

	for(int i= 0; i<m_pDOC->m_ODSize; i++)
		for(int j= 0; j<m_pDOC->m_ODSize; j++)
		{
			m_pDOC->m_DemandMatrix[i][j]= 0.0f;
		}

		while(!rsDemand.IsEOF())
		{
			int origin_zone_id, destination_zone_id, vehicle_type;
			float number_of_vehicles, starting_time_in_min, ending_time_in_min;

			origin_zone_id = rsDemand.GetLong(CString("from_zone_id"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field from_zone_id cannot be found in the demand table.");
				return;
			}

			if( origin_zone_id ==0)
				break;

			destination_zone_id = rsDemand.GetLong(CString("to_zone_id"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field to_zone_id cannot be found in the demand table.");
				return;
			}


			for(int	vehicle_type = 1; vehicle_type < m_pDOC->m_VehicleTypeVector.size(); vehicle_type++)
			{
				starting_time_in_min = 0;
				CString str_number_of_vehicles; 
				str_number_of_vehicles.Format ("number_of_vehicles_type%d", vehicle_type);
				ending_time_in_min = 60;
				number_of_vehicles = rsDemand.GetDouble(str_number_of_vehicles,bExist,false);
				if(!bExist)
				{

					CString str_number_of_vehicles_warning; 
					str_number_of_vehicles_warning.Format("Field %s cannot be found in the demand table.", str_number_of_vehicles);

					AfxMessageBox(str_number_of_vehicles_warning);
					return;
				}

				m_pDOC->m_DemandMatrix[origin_zone_id-1][destination_zone_id-1] += number_of_vehicles;
				total_demand+=number_of_vehicles;

				DTADemand element;
				element.from_zone_id = origin_zone_id;
				element.to_zone_id = destination_zone_id;
				element.vehicle_type = vehicle_type;
				element.starting_time_in_min = starting_time_in_min;
				element.ending_time_in_min = ending_time_in_min;
				element.number_of_vehicles = number_of_vehicles;

				m_pDOC->m_DemandVector.push_back (element);

			}

			rsDemand.MoveNext ();
		}
		rsDemand.Close();

		str_msg.Format ( "%f vehicles have been succesfully imported from the peak-hour-demand table.", total_demand);
		m_MessageList.AddString (str_msg);

	}

/////
		strSQL = m_pDOC->ConstructSQL("4-1-temporal-profile");


		if(strSQL.GetLength () > 0)
		{
		// Read record
		CRecordsetExt rsDemandProfile(&m_pDOC->m_Database);
		rsDemandProfile.Open(dbOpenDynaset, strSQL);

		while(!rsDemandProfile.IsEOF())
		{
			int interval_no;
			int starting_time_in_min, ending_time_in_min;
			float percentage;

			interval_no = rsDemandProfile.GetLong(CString("time_intervanl_no"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field time_intervanl_no cannot be found in the 4-1-temporal-profile table.");
				return;
			}
			if(interval_no <=0)
				break;

			starting_time_in_min = rsDemandProfile.GetLong(CString("starting_time_in_min"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field starting_time_in_min cannot be found in the 4-1-temporal-profile table.");
				return;
			}

			ending_time_in_min = rsDemandProfile.GetLong(CString("ending_time_in_min"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field ending_time_in_min cannot be found in the 4-1-temporal-profile table.");
				return;
			}
			percentage = rsDemandProfile.GetDouble(CString("percentage"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field percentage cannot be found in the 4-1-temporal-profile table.");
				return;
			}

			DTADemandProfile element;
			element.starting_time_in_min  = starting_time_in_min;
			element.ending_time_in_min = ending_time_in_min;
			element.percentage  = percentage;
			m_pDOC->m_DemandProfileVector.push_back(element);

			rsDemandProfile.MoveNext ();
		}
		rsDemandProfile.Close();

		if(m_pDOC->m_DemandProfileVector.size()>0)
		{
			m_pDOC->m_TempDemandVector = m_pDOC->m_DemandVector;  // copy to a tempory vector
			m_pDOC->m_DemandVector.clear ();

			for (int i=0; i< m_pDOC->m_TempDemandVector.size(); i++)
			{
				DTADemand element_static = m_pDOC->m_TempDemandVector[i];

				for(int t = 0; t< m_pDOC->m_DemandProfileVector.size(); t++)
				{
					DTADemand element_dynamic;
					element_dynamic = element_static;
					DTADemandProfile Profile_element = m_pDOC->m_DemandProfileVector[t];
					element_dynamic.starting_time_in_min = Profile_element.starting_time_in_min ;
					element_dynamic.ending_time_in_min  = Profile_element.ending_time_in_min  ;
					element_dynamic.number_of_vehicles  = element_static.number_of_vehicles*Profile_element.percentage   ;
					m_pDOC->m_DemandVector.push_back (element_dynamic);

				}
			}
		}


		if(m_pDOC->m_VehicleTypeVector.size()>0 && m_pDOC->m_DemandVector.size()==0)
		{
			m_MessageList.AddString("No demand has been input from the 4-peak-hour-demand worksheet. The alternative CSV file is used.");
			float total_demand = m_pDOC->FillODMatrixFromCSVFile(m_Edit_Demand_CSV_File);
			str_msg.Format ( "%f vehicles have been succesfully imported from the demand csv file.", total_demand);
			m_MessageList.AddString (str_msg);

		}

		if(m_pDOC->m_VehicleTypeVector.size()>0)
		{
			m_pDOC->m_TempDemandVector = m_pDOC->m_DemandVector;  // copy to a tempory vector
			m_pDOC->m_DemandVector.clear ();

			for (int i=0; i< m_pDOC->m_TempDemandVector.size(); i++)
			{
				DTADemand element_old = m_pDOC->m_TempDemandVector[i];

				for(int t = 0; t< m_pDOC->m_VehicleTypeVector.size(); t++)  // create new record for each vehicle type
				{
					if(m_pDOC->m_VehicleTypeVector[t].percentage > 0.0001)
					{

						DTADemand element_type;
						element_type = element_old;
						element_type.vehicle_type  = m_pDOC->m_VehicleTypeVector[t].vehicle_type ;
						element_type.number_of_vehicles  = element_old.number_of_vehicles*m_pDOC->m_VehicleTypeVector[t].percentage   ;
						m_pDOC->m_DemandVector.push_back (element_type);
					}
				}
			}
		}
		str_msg.Format ( "%d temporal profile records imported", m_pDOC->m_TempDemandVector.size());
		m_MessageList.AddString (str_msg);
		
}

		///// VOT distribution
		strSQL = m_pDOC->ConstructSQL("4-3-VOT-distribution");;

		// Read record

		if(strSQL.GetLength() > 0)
		{
		CRecordsetExt rsVOT(&m_pDOC->m_Database);
		rsVOT.Open(dbOpenDynaset, strSQL);

		while(!rsVOT.IsEOF())
		{

			int pricing_type;
			float percentage; float VOT;
			pricing_type = rsVOT.GetLong(CString("pricing_type"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field vehicle_type_no cannot be found in the 4-3-VOT-distribution table.");
				return;
			}
			if(pricing_type <=0)
				break;

			VOT = rsVOT.GetDouble(CString("VOT"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field VOT cannot be found in the 4-3-VOT-distribution table.");
				return;
			}

			percentage = rsVOT.GetDouble(CString("VOT_percentage"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field VOT_percentage cannot be found in the 4-3-VOT-distribution table.");
				return;
			}

			DTAVOTDistribution element;
			element.pricing_type = pricing_type;
			element.percentage  = percentage;
			element.VOT = VOT;

			m_pDOC->m_VOTDistributionVector.push_back(element);

			rsVOT.MoveNext ();
		}
		rsVOT.Close();

		str_msg.Format ( "%d Value of Time (VOT) records imported", m_pDOC->m_VOTDistributionVector.size());
		m_MessageList.AddString (str_msg);

		}

		strSQL = m_pDOC->ConstructSQL("4-4-vehicle-emission-rate");;

		if(strSQL.GetLength() > 0)
		{		// Read record
		CRecordsetExt rsEmissionRate(&m_pDOC->m_Database);
		rsEmissionRate.Open(dbOpenDynaset, strSQL);

		while(!rsEmissionRate.IsEOF())
		{


			int vehicle_type = rsEmissionRate.GetLong(CString("vehicle_type"),bExist,false);

			if(bExist)
			{
				int opModeID = rsEmissionRate.GetLong(CString("opModeID"),bExist,false);
				CEmissionRate element;
				element.meanBaseRate_TotalEnergy = rsEmissionRate.GetDouble(CString("meanBaseRate_TotalEnergy_(J/hr)"),bExist,false);
				element.meanBaseRate_CO2 = rsEmissionRate.GetDouble(CString("meanBaseRate_TotalEnergy_(J/hr)"),bExist,false);
				element.meanBaseRate_NOX = rsEmissionRate.GetDouble(CString("meanBaseRate_CO2_(g/hr)"),bExist,false);
				element.meanBaseRate_CO = rsEmissionRate.GetDouble(CString("meanBaseRate_NOX_(g/hr)"),bExist,false);
				element.meanBaseRate_HC = rsEmissionRate.GetDouble(CString("meanBaseRate_HC_(g/hr)"),bExist,false);

				ASSERT(vehicle_type < MAX_VEHICLE_TYPE_SIZE);
				ASSERT(opModeID < _MAXIMUM_OPERATING_MODE_SIZE);

				m_pDOC->EmissionRateData[vehicle_type][opModeID] = element;
			}
			rsEmissionRate.MoveNext ();
		}
		rsEmissionRate.Close();
		
		str_msg.Format ( "%d emissions records imported", m_pDOC->m_VOTDistributionVector.size());
		m_MessageList.AddString (str_msg);
	}

}



void CDlg_ImportNetwork::OnBnClickedImportNetworkOnly()
{
		m_bImportNetworkOnly = true;
		OnBnClickedImport();
		m_bImportNetworkOnly = false;  //reset flag
}


void CDlg_ImportNetwork::OnBnClickedImportSensorData()
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
		m_pDOC->m_NumberOfDays = dlg.m_NumberOfDays;
		Occ_to_Density_Coef = dlg.m_Occ_to_Density_Coef;

	}
	// Read record

	strSQL = m_pDOC->ConstructSQL("point-sensor-location");

	if(strSQL.GetLength() > 0)
	{
		// Read record
		CRecordsetExt rsSensorLocation(&m_pDOC->m_Database);
		rsSensorLocation.Open(dbOpenDynaset, strSQL);

		while(!rsSensorLocation.IsEOF())
		{
			DTA_sensor sensor;
			int SensorLinkID;

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
					DTALink* pLink = m_pDOC->FindLinkWithNodeNumbers(sensor.FromNodeNumber , sensor.ToNodeNumber );

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
		str_msg.Format ( "%d sensors imported", m_pDOC->m_SensorVector.size());
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

			if(number_of_samples>=11)
			TRACE("\nreading %d samples...", number_of_samples);


			int  modeling_timestamp_day =  rsSensorData.GetLong(CString("modeling_timestamp_day"),bExist,false);
				if(!bExist)
				{
					AfxMessageBox("Field modeling_timestamp_day cannot be found in the point-sensor table.");
					return;
				}

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

					if(t>=0  && t<pLink->m_LinkMOEAry.size())
					{
						if(AvgLinkSpeed<=1)  // 0 or negative values means missing speed
							AvgLinkSpeed = pLink->m_SpeedLimit ;

						ASSERT(pLink->m_NumLanes > 0);

						if(m_pDOC->m_bSimulationDataLoaded == false)  // simulation data not loaded
						 {

 							pLink->m_LinkMOEAry[ t].ObsFlow = total_link_flow_per_interval*60/m_pDOC->m_SamplingTimeInterval/pLink->m_NumLanes;  // convert to per hour link flow
							pLink->m_LinkMOEAry[ t].ObsSpeed = AvgLinkSpeed; 
							pLink->m_LinkMOEAry[ t].ObsTravelTimeIndex = pLink->m_SpeedLimit /max(1,AvgLinkSpeed)*100;

							if(occupancy <=0.001)
								pLink->m_LinkMOEAry[t].ObsDensity = pLink->m_LinkMOEAry[t].ObsFlow / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeed);
							else
								pLink->m_LinkMOEAry[t].ObsDensity = occupancy * Occ_to_Density_Coef;

							// copy data to other intervals
							for(int tt = 1; tt<m_pDOC->m_SamplingTimeInterval; tt++)
							{
								if(tt < pLink->m_LinkMOEAry.size())
								{
								pLink->m_LinkMOEAry[ t+tt].ObsFlow = pLink->m_LinkMOEAry[t].ObsFlow ;
								pLink->m_LinkMOEAry[t+tt].ObsSpeed = pLink->m_LinkMOEAry[t].ObsSpeed;
								pLink->m_LinkMOEAry[t+tt].ObsDensity = pLink->m_LinkMOEAry[t].ObsDensity;
								pLink->m_LinkMOEAry[t+tt].ObsTravelTimeIndex = pLink->m_LinkMOEAry[t].ObsTravelTimeIndex;
								}

							}
						}else // simulation data loaded
						{

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
				}	
			}
				number_of_samples++;
		
			rsSensorData.MoveNext ();
		}
			rsSensorData.Close  ();
			str_msg.Format("%d sensor data records loaded.",number_of_samples);
			m_MessageList.AddString (str_msg);
	}

	// Read record

	strSQL = m_pDOC->ConstructSQL("AVI-sensor-location");

	if(strSQL.GetLength() > 0)
	{
		// Read record
		CRecordsetExt rsAVINode(&m_pDOC->m_Database);
		rsAVINode.Open(dbOpenDynaset, strSQL);

		while(!rsAVINode.IsEOF())
		{
			int node_id = rsAVINode.GetLong(CString("node_id"),bExist,false);

			if(!bExist)
			{
				m_MessageList.AddString ("Field node_id cannot be found in the AVI-sensor-location table.");
				rsAVINode.Close();
				return;
			}

//			int node_id = rsAVINode.GetLong(CString("node_id"),bExist,false);

			if(!bExist)
			{
				m_MessageList.AddString ("Field node_id cannot be found in the AVI-sensor-location table.");
				rsAVINode.Close();
				return;
			}

	}

}
}