// Dlg_ImportNetwork.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_ImportNetwork.h"
#include "DlgSensorDataLoading.h"
#include "MainFrm.h"
#include "Shellapi.h"


// CDlg_ImportNetwork dialog

IMPLEMENT_DYNAMIC(CDlg_ImportNetwork, CDialog)

CDlg_ImportNetwork::CDlg_ImportNetwork(CWnd* pParent /*=NULL*/)
: CDialog(CDlg_ImportNetwork::IDD, pParent)
, m_Edit_Excel_File(_T(""))
, m_Edit_Demand_CSV_File(_T(""))
, m_Sensor_File(_T(""))
, m_bRemoveConnectors(FALSE)
{
	m_bImportNetworkOnly = false;
}

CDlg_ImportNetwork::~CDlg_ImportNetwork()
{
	if(m_pDoc->m_Database.IsOpen ())
		m_pDoc->m_Database.Close ();

}

void CDlg_ImportNetwork::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ExcelFile, m_Edit_Excel_File);
	DDX_Text(pDX, IDC_EDIT_Demand_CSV_File, m_Edit_Demand_CSV_File);
	DDX_Control(pDX, IDC_LIST1, m_MessageList);
	DDX_Check(pDX, IDC_CHECK_REMOVE_CONNECTOR, m_bRemoveConnectors);
}


BEGIN_MESSAGE_MAP(CDlg_ImportNetwork, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_Find_Exel_File, &CDlg_ImportNetwork::OnBnClickedButtonFindExelFile)
	ON_BN_CLICKED(IDC_BUTTON_Find_Demand_CSV_File, &CDlg_ImportNetwork::OnBnClickedButtonFindDemandCsvFile)
	ON_BN_CLICKED(ID_IMPORT, &CDlg_ImportNetwork::OnBnClickedImport)
	ON_BN_CLICKED(ID_IMPORT_Network_Only, &CDlg_ImportNetwork::OnBnClickedImportNetworkOnly)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDlg_ImportNetwork::OnLbnSelchangeList1)
	ON_BN_CLICKED(ID_EXPORT_DATA, &CDlg_ImportNetwork::OnBnClickedExportData)
	ON_BN_CLICKED(IDC_BUTTON_View_Sample_File, &CDlg_ImportNetwork::OnBnClickedButtonViewSampleFile)
	ON_BN_CLICKED(IDC_BUTTON_Load_Sample_File, &CDlg_ImportNetwork::OnBnClickedButtonLoadSampleFile)
	ON_BN_CLICKED(IDC_BUTTON_View_Sample_CSV_File, &CDlg_ImportNetwork::OnBnClickedButtonViewSampleCsvFile)
	ON_BN_CLICKED(ID_IMPORT2, &CDlg_ImportNetwork::OnBnClickedImport2)
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

}

void CDlg_ImportNetwork::OnBnClickedImport()
{
	CWaitCursor cursor;
	// Make sure the network is empty
	m_pDoc->m_NodeSet.clear ();
	m_pDoc->m_LinkSet.clear ();
	m_MessageList.ResetContent ();

	UpdateData(true);
	bool bExist=true;

	CString strSQL;
	CString str_msg;

	UpdateData(true);

	if(m_pDoc->m_Database.IsOpen ())
		m_pDoc->m_Database.Close ();

	m_pDoc->m_Database.Open(m_Edit_Excel_File, false, true, "excel 5.0; excel 97; excel 2000; excel 2003");

	// Open the EXCEL file
	std::string itsErrorMessage;

	// this accesses first sheet regardless of name.
	int i= 0;


	// Read record

	strSQL = m_pDoc->ConstructSQL("1-NODE");

	if(strSQL.GetLength() > 0)
	{
		CRecordsetExt rsNode(&m_pDoc->m_Database);
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
			if(id == 0)  // reading empty line
				break;


			int control_type = rsNode.GetLong(CString("control_type"),bExist,false);
			if(!bExist) 
			{
				m_MessageList.AddString ("Field control_type cannot be found in the node table.");
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
			pNode->m_ControlType = control_type;

			m_pDoc->m_NodeSet.push_back(pNode);
			m_pDoc->m_NodeIDMap[i] = pNode;
			m_pDoc->m_NodeIDtoNameMap[i] = id;
			m_pDoc->m_NodeNametoIDMap[id] = i;
			i++;

			rsNode.MoveNext();
		}	// end of while
		rsNode.Close();

	}else
	{
		str_msg.Format ( "Worksheet 1-node cannot be found in the given Excel file");
		m_MessageList.AddString (str_msg);
		return;
	}

	// Read record
	strSQL = m_pDoc->ConstructSQL("2-1-LINK-TYPE");
	if(strSQL.GetLength () > 0)
	{

		CRecordsetExt rsLinkType(&m_pDoc->m_Database);
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

			element.connector_flag    = rsLinkType.GetLong (CString("connector_flag"),bExist,false);
			if(!bExist)
			{
				m_MessageList.AddString("Field connector_flag cannot be found in the input_link_type.csv.");
				return;
			}

			element.transit_flag    = rsLinkType.GetLong (CString("transit_flag"),bExist,false);
			if(!bExist)
			{
				m_MessageList.AddString("Field transit_flag cannot be found in the input_link_type.csv.");
				return;
			}

			element.walking_flag    = rsLinkType.GetLong (CString("walking_flag"),bExist,false);
			if(!bExist)
			{
				m_MessageList.AddString("Field walking_flag cannot be found in the input_link_type.csv.");
				return;
			}

			m_pDoc->m_LinkTypeFreewayMap[element.link_type] = element.freeway_flag ;
			m_pDoc->m_LinkTypeArterialMap[element.link_type] = element.arterial_flag  ;
			m_pDoc->m_LinkTypeRampMap[element.link_type] = element.ramp_flag  ;
			m_pDoc->m_LinkTypeConnectorMap[element.link_type] = element.connector_flag  ;
			m_pDoc->m_LinkTypeTransitMap[element.link_type] = element.transit_flag  ;
			m_pDoc->m_LinkTypeWalkingMap[element.link_type] = element.walking_flag  ;
			

			m_pDoc->m_LinkTypeMap[element.link_type] = element;

			rsLinkType.MoveNext ();
		}
		rsLinkType.Close();
		str_msg.Format ("%d link type definitions imported.",m_pDoc->m_LinkTypeMap.size());
		m_MessageList.AddString(str_msg);
	}else
	{
		str_msg.Format ( "Worksheet 2-1-link-type cannot be found in the given Excel file");
		m_MessageList.AddString (str_msg);
		return;
	}

	bool bAutogenerateNodeFlag = false;
	if(m_pDoc->m_NodeSet.size() == 0)
	{

		bAutogenerateNodeFlag = true;

		str_msg.Format ( "Worksheet 1-node contain 0 node. Upstream and downstream nodes in the link table is used to generate node info.");
		m_MessageList.AddString (str_msg);

	}else
	{
		str_msg.Format ( "%d nodes have been successfully imported.",m_pDoc->m_NodeSet.size());
		m_MessageList.AddString (str_msg);
	}
	
		// Read record
		strSQL = m_pDoc->ConstructSQL("2-LINK");

		if(strSQL.GetLength () > 0)
		{
			CRecordsetExt rsLink(&m_pDoc->m_Database);
			rsLink.Open(dbOpenDynaset, strSQL);
			i = 0;
			int line_no = 2;
			float default_distance_sum = 0;
			CString DTASettingsPath = m_pDoc->m_ProjectDirectory+"DTASettings.ini";
			float default_AADT_conversion_factor = g_GetPrivateProfileFloat("safety_planning", "default_AADT_conversion_factor", 0.1, DTASettingsPath);	


			float length_sum = 0;
			while(!rsLink.IsEOF())
			{

				int from_node_id = rsLink.GetLong(CString("from_node_id"),bExist,false);
				if(!bExist  ) 
				{
					if(m_pDoc->m_LinkSet.size() ==0)
					{
					// no link has been loaded
					m_MessageList.AddString ("Field from_node_id cannot be found in the link table.");
					rsLink.Close();
					return;
					}else
					{
					rsLink.MoveNext();
					break;
					// moveon					
					}
				}

				int to_node_id = rsLink.GetLong(CString("to_node_id"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString("Field to_node_id cannot be found in the link table.");
					rsLink.Close();
					return;
				}


				if(from_node_id==0 && to_node_id ==0)  // test twice here for from and to nodes
					break;


				long link_id =  rsLink.GetLong(CString("link_id"),bExist,false);
				if(!bExist)
					link_id = 0;

				int type = rsLink.GetLong(CString("link_type"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString("Field link_type cannot be found in the link table.");
					rsLink.Close();
					return;
				}


				if(bAutogenerateNodeFlag == false && m_pDoc->m_NodeNametoIDMap.find(from_node_id)== m_pDoc->m_NodeNametoIDMap.end())
				{


					str_msg.Format("from_node_id %d at row %d cannot be found in the node table!",from_node_id, line_no);
					m_MessageList.AddString(str_msg);
					//					rsLink.Close();
					rsLink.MoveNext();
					continue;
				}

				if(bAutogenerateNodeFlag == false && m_pDoc->m_NodeNametoIDMap.find(to_node_id)== m_pDoc->m_NodeNametoIDMap.end())
				{
					str_msg.Format("to_node_id %d at row %d cannot be found in the node table!",to_node_id, line_no);
					m_MessageList.AddString(str_msg);
					rsLink.MoveNext();
					continue;
				}

				std::vector<CCoordinate> CoordinateVector;

				CString geometry_str = rsLink.GetCString(CString("geometry"));

				if(bAutogenerateNodeFlag && geometry_str.GetLength () ==0)
				{
				
					m_MessageList.AddString("Field geometry cannot be found in the link table. This is required when no node info is given.");
					rsLink.Close();
					return;
				}

				if(geometry_str.GetLength () > 0)
				{

					CT2CA pszConvertedAnsiString (geometry_str);

				// construct a std::string using the LPCSTR input
				std::string geo_string (pszConvertedAnsiString);

				CGeometry geometry(geo_string);
				CoordinateVector = geometry.GetCoordinateList();

				if(bAutogenerateNodeFlag)  // add nodes
				{

					// from node
					if(m_pDoc->m_NodeNametoIDMap.find(from_node_id)== m_pDoc->m_NodeNametoIDMap.end())
					{
							GDPoint	pt;
							pt.x = CoordinateVector[0].X;
							pt.y = CoordinateVector[0].Y;
							
							bool ActivityLocationFlag = false;
							if(m_pDoc->m_LinkTypeConnectorMap[type ]==1) // adjacent node of connectors
								ActivityLocationFlag = true;

							m_pDoc->AddNewNode(pt, from_node_id, 0,ActivityLocationFlag);

					}

					// to node
					if(m_pDoc->m_NodeNametoIDMap.find(to_node_id)== m_pDoc->m_NodeNametoIDMap.end())
					{
							GDPoint	pt;
							pt.x = CoordinateVector[CoordinateVector.size()-1].X;
							pt.y = CoordinateVector[CoordinateVector.size()-1].Y;

							bool ActivityLocationFlag = false;
							if(m_pDoc->m_LinkTypeConnectorMap[type ]==1) // adjacent node of connectors
								ActivityLocationFlag = true;

							m_pDoc->AddNewNode(pt, to_node_id, 0,ActivityLocationFlag);
					}

				}

				}

				if(m_bRemoveConnectors && m_pDoc->m_LinkTypeConnectorMap[type ]==1) 
				{  // skip connectors
					rsLink.MoveNext();
					continue;
				}


				DTALink* pExistingLink =  m_pDoc->FindLinkWithNodeIDs(m_pDoc->m_NodeNametoIDMap[from_node_id],m_pDoc->m_NodeNametoIDMap[to_node_id]);

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

				if(number_of_lanes ==0)
				{
					str_msg.Format ("Link %d -> %d has 0 lane. Skip.",from_node_id,to_node_id);
					m_MessageList.AddString(str_msg);
					rsLink.MoveNext();
					continue; 
				}



				float grade= rsLink.GetDouble(CString("grade"),bExist,false);
				float AADT_conversion_factor = rsLink.GetDouble(CString("AADT_conversion_factor"),bExist,false);
				if(!bExist)
				{

				AADT_conversion_factor = default_AADT_conversion_factor;
				}

				float speed_limit_in_mph= rsLink.GetLong(CString("speed_limit_in_mph"),bExist,false);
				if(!bExist) 
				{
					AfxMessageBox("Field speed_limit_in_mph cannot be found in the link table.");
					rsLink.Close();
					return;
				}

				if(speed_limit_in_mph ==0)
				{
					str_msg.Format ("Link %d -> %d has a speed limit of 0. Skip.",from_node_id,to_node_id);
					m_MessageList.AddString(str_msg);
					rsLink.MoveNext();
					continue; 
				}

				float capacity_in_pcphpl= rsLink.GetDouble(CString("lane_capacity_in_veh_per_hour"),bExist,false);
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


				int direction = rsLink.GetLong(CString("two_way_flag"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString("Field two_way_flag cannot be found in the link table.");
					rsLink.Close();
					return;
				}

				CString name = rsLink.GetCString(CString("name"));

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


				if (direction == 0 || direction ==2) // two-directional link
				{
					link_code_start = 1; link_code_end = 2;
				}

				if(bAutogenerateNodeFlag == false)
				{
				// no geometry information
				CCoordinate cc_from, cc_to; 
				cc_from.X = m_pDoc->m_NodeIDMap[m_pDoc->m_NodeNametoIDMap[from_node_id]]->pt.x;
				cc_from.Y = m_pDoc->m_NodeIDMap[m_pDoc->m_NodeNametoIDMap[from_node_id]]->pt.y;

				cc_to.X = m_pDoc->m_NodeIDMap[m_pDoc->m_NodeNametoIDMap[to_node_id]]->pt.x;
				cc_to.Y = m_pDoc->m_NodeIDMap[m_pDoc->m_NodeNametoIDMap[to_node_id]]->pt.y;

				CoordinateVector.push_back(cc_from);
				CoordinateVector.push_back(cc_to);
				}

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

						pLink->m_FromNodeID = m_pDoc->m_NodeNametoIDMap[from_node_id];
						pLink->m_ToNodeID= m_pDoc->m_NodeNametoIDMap[to_node_id];


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
						pLink->m_FromNodeID = m_pDoc->m_NodeNametoIDMap[to_node_id];
						pLink->m_ToNodeID= m_pDoc->m_NodeNametoIDMap[from_node_id];


						for(int si = CoordinateVector.size()-1; si >=0; si--)  // we need to put int here as si can be -1. 
						{
							GDPoint	pt;
							pt.x = CoordinateVector[si].X;
							pt.y = CoordinateVector[si].Y;
							pLink->m_ShapePoints .push_back (pt);
						}
					}



					pLink->m_NumLanes= number_of_lanes;
					pLink->m_SpeedLimit= speed_limit_in_mph;
					pLink->m_StaticSpeed = pLink->m_SpeedLimit;
					pLink->m_Length= length;  // minimum distance

					if(length < 0.00001) // zero value in length field, we consider no length info.
					{
						float distance_in_mile = g_CalculateP2PDistanceInMileFromLatitudeLongitude(pLink->m_ShapePoints[0], pLink->m_ShapePoints[pLink->m_ShapePoints.size()-1]);
						pLink->m_Length = distance_in_mile;
					}

					pLink->m_FreeFlowTravelTime = pLink->m_Length / pLink->m_SpeedLimit *60.0f;
					pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;

					pLink->m_MaximumServiceFlowRatePHPL= capacity_in_pcphpl;
					pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
					pLink->m_link_type= type;
					pLink->m_Grade = grade;


					if(link_code == 2)  //BA link
					{

						int R_number_of_lanes = number_of_lanes;

						float R_speed_limit_in_mph= speed_limit_in_mph;

						float R_lane_capacity_in_vhc_per_hour= capacity_in_pcphpl;
						float R_grade= grade;
						pLink->m_NumLanes= R_number_of_lanes;
						pLink->m_SpeedLimit= R_speed_limit_in_mph;
						pLink->m_MaximumServiceFlowRatePHPL= R_lane_capacity_in_vhc_per_hour;
						pLink->m_Grade = R_grade;

						pLink->m_StaticSpeed = pLink->m_SpeedLimit;
						pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance
						pLink->m_FreeFlowTravelTime = pLink->m_Length / max(1,pLink->m_SpeedLimit) *60.0f;
						pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;
						pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
						pLink->m_link_type= type;



					}

					pLink->m_Kjam = k_jam;
					pLink->m_AADT_conversion_factor  = AADT_conversion_factor;
					pLink->m_Wave_speed_in_mph  = wave_speed_in_mph;

					m_pDoc->m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);

					pLink->m_FromPoint = m_pDoc->m_NodeIDMap[pLink->m_FromNodeID]->pt;
					pLink->m_ToPoint = m_pDoc->m_NodeIDMap[pLink->m_ToNodeID]->pt;


					default_distance_sum+= pLink->DefaultDistance();
					length_sum += pLink ->m_Length;
					//			pLink->SetupMOE();
					pLink->input_line_no  = line_no;
					
					

					m_pDoc->m_LinkSet.push_back (pLink);
					m_pDoc->m_LinkNoMap[i]  = pLink;
					m_pDoc->m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
					m_pDoc->m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;

					if(m_pDoc->m_LinkTypeConnectorMap[type ]==1) // adjacent node of connectors
					{ 
						// mark them as activity location 
					m_pDoc->m_NodeIDMap[pLink->m_FromNodeID ]->m_bZoneActivityLocationFlag = true;					
					m_pDoc->m_NodeIDMap[pLink->m_ToNodeID ]->m_bZoneActivityLocationFlag = true;					
					}



					m_pDoc->m_NodeIDMap[pLink->m_FromNodeID ]->m_OutgoingLinkVector.push_back(i);


					unsigned long LinkKey = m_pDoc->GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
					m_pDoc->m_NodeIDtoLinkMap[LinkKey] = pLink;


		__int64  LinkKey2 = pLink-> m_FromNodeNumber* pLink->m_ToNodeNumber;
		m_pDoc->m_NodeNumbertoLinkMap[LinkKey2] = pLink;


					i++;

				}

				rsLink.MoveNext();
				//				TRACE("reading line %d\n", line_no);
				line_no ++;
			}

			rsLink.Close();
			m_pDoc->GenerateOffsetLinkBand();

			m_pDoc->m_UnitMile  = 1.0f;

			if(length_sum>0.000001f)
				m_pDoc->m_UnitMile=  default_distance_sum /length_sum;

			m_pDoc->m_UnitFeet = m_pDoc->m_UnitMile/5280.0f;  

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

			m_pDoc->OffsetLink();

		}else
		{
			str_msg.Format ( "Worksheet 2-link cannot be found in the given Excel file");
			m_MessageList.AddString (str_msg);
			return;
		}


		if(bAutogenerateNodeFlag)  // generated when read shape files
		{
		str_msg.Format ( "%d nodes have been successfully imported.",m_pDoc->m_NodeSet.size());
		m_MessageList.AddString (str_msg);
		}

		str_msg.Format ("%d links have been sucessfully imported.",m_pDoc->m_LinkSet.size());
		m_MessageList.AddString(str_msg);
	



	if(m_bImportNetworkOnly == true)
		return;

	// ZONE table
	strSQL = m_pDoc->ConstructSQL("3-2-ZONE-ACTIVITY-LOCATION");

	//bool bNodeNonExistError = false;
	m_pDoc->m_NodeIDtoZoneNameMap.clear ();

	bool bNodeNonExistError = false;
	m_pDoc->m_NodeIDtoZoneNameMap.clear ();
	m_pDoc->m_ODSize = 0;

	// Read record
	int activity_location_count = 0;

	if(strSQL.GetLength () > 0)
	{
		CRecordsetExt rsZone(&m_pDoc->m_Database);
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

			map <int, int> :: const_iterator m_Iter = m_pDoc->m_NodeNametoIDMap.find(node_name);

			if(m_Iter == m_pDoc->m_NodeNametoIDMap.end( ))
			{
				CString m_Warning;
				m_Warning.Format("Node Number %d in the zone tabe has not been defined in the node table", node_name);
				AfxMessageBox(m_Warning);
				return;
			}
			m_pDoc->m_NodeIDtoZoneNameMap[m_pDoc->m_NodeNametoIDMap[node_name]] = zone_number;

			m_pDoc->m_NodeIDMap [ m_pDoc->m_NodeNametoIDMap[node_name] ] ->m_bZoneActivityLocationFlag = true;
			m_pDoc->m_NodeIDMap [ m_pDoc->m_NodeNametoIDMap[node_name] ] -> m_ZoneID = zone_number;
			// if there are multiple nodes for a zone, the last node id is recorded.


			DTAActivityLocation element;
			element.ZoneID  = zone_number;
			element.NodeNumber = node_name;


			m_pDoc->m_ZoneMap [zone_number].m_ActivityLocationVector .push_back (element);

			if(m_pDoc->m_ODSize < zone_number)
				m_pDoc->m_ODSize = zone_number;

			rsZone.MoveNext ();
			activity_location_count++;
		}
		rsZone.Close();

		str_msg.Format ( "%d activity location records imported",activity_location_count);
		m_MessageList.AddString (str_msg);
	}

	// ZONE table
	strSQL = m_pDoc->ConstructSQL("3-1-ZONE");

	// Read record
	if(strSQL.GetLength () > 0)
	{
		CRecordsetExt rsZone(&m_pDoc->m_Database);
		rsZone.Open(dbOpenDynaset, strSQL);

		int count = 0;
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

			// if there are multiple nodes for a zone, the last node id is recorded.
				std::vector<CCoordinate> CoordinateVector;

				CString geometry_str = rsZone.GetCString(CString("geometry"));

				if(geometry_str.GetLength () > 0)
				{

				CT2CA pszConvertedAnsiString (geometry_str);

				// construct a std::string using the LPCSTR input
				std::string geo_string (pszConvertedAnsiString);

				CGeometry geometry(geo_string);
				CoordinateVector = geometry.GetCoordinateList();

				m_pDoc->m_ZoneMap [zone_number].m_ZoneTAZ = zone_number;

			for(unsigned int f = 0; f < CoordinateVector.size(); f++)
			{
				GDPoint pt;
				pt.x = CoordinateVector[f].X;
				pt.y = CoordinateVector[f].Y;
				m_pDoc->m_ZoneMap [zone_number].m_ShapePoints.push_back (pt);
			}

				}
			if(m_pDoc->m_ODSize < zone_number)
				m_pDoc->m_ODSize = zone_number;
				

			rsZone.MoveNext ();
			count++;
		}
		rsZone.Close();

		str_msg.Format ( "%d zone boundary records are imported.",count);
		m_MessageList.AddString (str_msg);
	
		// assign zone numbers to connectors

		if(count>=1)  // with boundary
		{
		std::list<DTALink*>::iterator iLink;

			for (iLink = m_pDoc->m_LinkSet.begin(); iLink != m_pDoc->m_LinkSet.end(); iLink++)
			{
				if(m_pDoc->m_LinkTypeConnectorMap[(*iLink)->m_link_type ]==1)  // connectors
				{
					
					GDPoint pt_from = (*iLink)->m_FromPoint ;
					int ZoneID_from = m_pDoc->GetZoneID(pt_from);

					// assign id according to upstream node zone number first
					int ZoneID_to = 0;
					if(ZoneID_from <=0)
					{
					GDPoint pt_to = (*iLink)->m_ToPoint ;
					 ZoneID_to = m_pDoc->GetZoneID(pt_to);
					}

					// assign id according to downstream node zone number second

					int ZoneID = max(ZoneID_from, ZoneID_to);  // get large zone id under two different zone numbers
					if(ZoneID > 0)
						(*iLink)->m_ConnectorZoneID = ZoneID;

				}
			}
		}
		}

		// determine activity locations if no activity locations have been provided

		if(activity_location_count == 0)
		{
		std::list<DTANode*>::iterator iNode;

		for (iNode = m_pDoc->m_NodeSet.begin(); iNode != m_pDoc->m_NodeSet.end(); iNode++)
		{

			if((*iNode )->m_NodeNumber == 908)
			{
			TRACE("");
			}
			if((*iNode )->m_bZoneActivityLocationFlag)
			{
					int ZoneID = m_pDoc->GetZoneID((*iNode)->pt);
					if(ZoneID>0)
					{
						(*iNode )->m_ZoneID = ZoneID;
						DTAActivityLocation element;
						element.ZoneID  = ZoneID;
						element.NodeNumber = (*iNode )->m_NodeNumber;


						m_pDoc->m_ZoneMap [ZoneID].m_ActivityLocationVector .push_back (element );

					}
			
				}
		
		}
		str_msg.Format ( "%d activity locations identified",activity_location_count);
		m_MessageList.AddString (str_msg);
			
		}

		if(m_bRemoveConnectors)
		{
		std::list<DTANode*>::iterator iNode;

		std::vector <int> CentroidVector;
		for (iNode = m_pDoc->m_NodeSet.begin(); iNode != m_pDoc->m_NodeSet.end(); iNode++)
		{
			if((*iNode )->m_bZoneActivityLocationFlag && (*iNode )->m_Connections ==0)  // has been as activity location but no link connected
				CentroidVector.push_back((*iNode )->m_NodeID );

		}

		for(unsigned int i = 0; i < CentroidVector.size(); i++)
		{
			m_pDoc->DeleteNode (CentroidVector[i]);
		}
		
		str_msg.Format ( "%d centroids deleted",CentroidVector.size());
		m_MessageList.AddString (str_msg);
		
		}


	strSQL = m_pDoc->ConstructSQL("4-1-vehicle-type");


	if (strSQL.GetLength() > 0)
	{
		CRecordsetExt rsVehicleType(&m_pDoc->m_Database);
		rsVehicleType.Open(dbOpenDynaset, strSQL);

		while(!rsVehicleType.IsEOF())
		{
			int type_no;
			type_no = rsVehicleType.GetLong(CString("vehicle_type"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field vehicle_type cannot be found in the 4-1-vehicle-type table.");
				return;
			}

			if(type_no >= MAX_VEHICLE_TYPE_SIZE)
			{
				AfxMessageBox("Field vehicle_type is too large. Please contact the program developer.");
				return;

			}
			if(type_no <=0)
				break;

			CString vehicle_type_name;

			vehicle_type_name = rsVehicleType.GetCString(CString("vehicle_type_name"));

			DTAVehicleType element;
			element.vehicle_type = type_no;
			element.vehicle_type_name = vehicle_type_name;
			m_pDoc->m_VehicleTypeVector.push_back(element);

			rsVehicleType.MoveNext ();
		}
		rsVehicleType.Close();

		str_msg.Format ( "%d vehicle types imported", m_pDoc->m_VehicleTypeVector.size());
		m_MessageList.AddString (str_msg);

	}



	strSQL = m_pDoc->ConstructSQL("4-2-demand-type");


	if (strSQL.GetLength() > 0)
	{
		CRecordsetExt rsDemandType(&m_pDoc->m_Database);
		rsDemandType.Open(dbOpenDynaset, strSQL);

		while(!rsDemandType.IsEOF())
		{
			int type_no;
			type_no = rsDemandType.GetLong(CString("demand_type"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field demand_type cannot be found in the 4-2-demand-type table.");
				return;
			}
			if(type_no <=0)
				break;

			float average_VOT = rsDemandType.GetDouble(CString("average_VOT"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field average_VOT cannot be found in the 4-2-demand-type table.");
			}

			int pricing_type;
			pricing_type =  rsDemandType.GetLong(CString("pricing_type"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field pricing_type cannot be found in the 4-2-demand-type table.");
			}

			float ratio_pretrip = rsDemandType.GetLong(CString("percentage_of_pretrip_info"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field percentage_of_pretrip_info cannot be found in the 4-2-demand-type table.");
			}

			float ratio_enroute = rsDemandType.GetLong(CString("percentage_of_enroute_info"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field percentage_of_enroute_info cannot be found in the 4-2-demand-type table.");
			}

			if (ratio_pretrip  > 1.0)
			{
				AfxMessageBox("The field percentage_of_pretrip_info cannot be greater than 100% in the 4-2-demand-type table. Please check.");
				ratio_pretrip = 0;
			}

			if (ratio_enroute  > 1.0)
			{
				AfxMessageBox("The field percentage_of_enroute_info cannot be greater than 100% in the 4-2-demand-type table. Please check.");
				ratio_enroute = 0;
			}

			if ((ratio_pretrip + ratio_enroute) > 1.0)
			{
				AfxMessageBox("Sum of percentage_of_pretrip_info and percentage_of_enroute_info cannot be greater than 100% in the 4-2-demand-type table. Please check.");
				ratio_pretrip = 0;
				ratio_enroute = 0;

			}


			CString demand_type_name;

			demand_type_name = rsDemandType.GetCString(CString("demand_type_name"));

			DTADemandType element;
			element.demand_type = type_no;
			element.pricing_type = pricing_type;
			element.demand_type_name = demand_type_name;
			element.average_VOT = average_VOT;

			element.info_class_percentage[1] = ratio_pretrip * 100;
			element.info_class_percentage[2] = ratio_enroute * 100;
			element.info_class_percentage[0] = 100 - ratio_pretrip - ratio_enroute;


			for(unsigned int i=0; i< m_pDoc->m_VehicleTypeVector.size(); i++)
			{
				CString str_percentage_of_vehicle_type; 
				str_percentage_of_vehicle_type.Format ("percentage_of_vehicle_type%d", i+1);
				float percentage = rsDemandType.GetDouble(str_percentage_of_vehicle_type,bExist,false);
				if(!bExist)
				{

					CString str_percentage_of_vehicle_type_warning; 
					str_percentage_of_vehicle_type_warning.Format("Field %s cannot be found in the demand table.", str_percentage_of_vehicle_type);

					AfxMessageBox(str_percentage_of_vehicle_type_warning);
					return;
				}

				element.vehicle_type_percentage[i] = percentage*100;


			}


			m_pDoc->m_DemandTypeVector.push_back(element);

			rsDemandType.MoveNext ();
		}
		rsDemandType.Close();


		str_msg.Format ( "%d demand types imported.", m_pDoc->m_DemandTypeVector.size());
		m_MessageList.AddString (str_msg);

	}

	strSQL = m_pDoc->ConstructSQL("4-3-PEAK-PERIOD-DEMAND");

	if(strSQL.GetLength() > 0)
	{
		// Read record

		float total_demand = 0;
		CRecordsetExt rsDemand(&m_pDoc->m_Database);
		rsDemand.Open(dbOpenDynaset, strSQL);

			int line_no = 2;
			while(!rsDemand.IsEOF())
			{
				int origin_zone_id, destination_zone_id;
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

				starting_time_in_min =  rsDemand.GetLong(CString("starting_time_in_min"),bExist,false);
				if(!bExist)
				{
					AfxMessageBox("Field starting_time_in_min cannot be found in the demand table.");
					return;
				}
				ending_time_in_min =  rsDemand.GetLong(CString("ending_time_in_min"),bExist,false);
				if(!bExist)
				{
					AfxMessageBox("Field ending_time_in_min cannot be found in the demand table.");
					return;
				}

				if(ending_time_in_min < starting_time_in_min)
				{
					CString str_error; 
					str_error.Format ("Error: Value ending_time_in_min < starting_time_in_min at line no.%d in the demand table.",line_no);
					AfxMessageBox(str_error);
					return;
				}

				DTADemand element;
				element.from_zone_id = origin_zone_id;
				element.to_zone_id = destination_zone_id;

				if(origin_zone_id > m_pDoc->m_ODSize)
				{
					CString str_error; 
					str_error.Format ("Error: Value from_zone_id > numbers of zones at line no.%d in the demand table.",line_no);
					AfxMessageBox(str_error);
					return;
				}

				if(destination_zone_id > m_pDoc->m_ODSize)
				{
					CString str_error; 
					str_error.Format ("Error: Value to_zone_id > numbers of zones at line no.%d in the demand table.",line_no);
					AfxMessageBox(str_error);
					return;
				}

				element.starting_time_in_min = starting_time_in_min;
				element.ending_time_in_min = ending_time_in_min;

				for(unsigned int	demand_type = 1; demand_type < m_pDoc->m_DemandTypeVector.size(); demand_type++)
				{
					CString str_number_of_vehicles; 
					str_number_of_vehicles.Format ("number_of_trips_demand_type%d", demand_type);
					number_of_vehicles = rsDemand.GetDouble(str_number_of_vehicles,bExist,false);
					if(!bExist)
					{

						CString str_number_of_vehicles_warning; 
						str_number_of_vehicles_warning.Format("Field %s cannot be found in the demand table.", str_number_of_vehicles);

						AfxMessageBox(str_number_of_vehicles_warning);
						return;
					}

					m_pDoc->m_ZoneMap[origin_zone_id].m_ODDemandMatrix [destination_zone_id].SetValue (demand_type,number_of_vehicles);
					total_demand+=number_of_vehicles;
					element.number_of_vehicles_per_demand_type.push_back(number_of_vehicles);
				}

				rsDemand.MoveNext ();
				line_no++;
			}
			rsDemand.Close();

			str_msg.Format ( "%f vehicles have been succesfully imported from the peak-hour-demand table.", total_demand);
			m_MessageList.AddString (str_msg);

	}


	strSQL = m_pDoc->ConstructSQL("4-4-temporal-profile");


	if(strSQL.GetLength () > 0)
	{
		// Read record
		CRecordsetExt rsDemandProfile(&m_pDoc->m_Database);
		rsDemandProfile.Open(dbOpenDynaset, strSQL);

		while(!rsDemandProfile.IsEOF())
		{
			int from_zone_id, to_zone_id;

			from_zone_id = rsDemandProfile.GetLong(CString("from_zone_id"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field from_zone_id cannot be found in the 4-4-temporal-profile table.");
				return;
			}
			if(from_zone_id <0)
				break;

			to_zone_id = rsDemandProfile.GetLong(CString("to_zone_id"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field to_zone_id cannot be found in the 4-4-temporal-profile table.");
				return;
			}
			if(to_zone_id <0)
				break;


			int demand_type = rsDemandProfile.GetLong(CString("demand_type"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field demand_type cannot be found in the 4-4-temporal-profile table.");
				return;
			}
			if(demand_type <0)
				break;

			DTADemandProfile element; 

			element.demand_type = demand_type;
			element.from_zone_id = from_zone_id;
			element.to_zone_id = to_zone_id;

			element.series_name = rsDemandProfile.GetCString(CString("time_series_label"));

			bool bWithDataFlag = false;

			for(int t = 0; t< MAX_TIME_INTERVAL_SIZE; t++)
			{
				CString time_stamp_str = m_pDoc->GetTimeStampFloatingPointStrFromIntervalNo (t);
				TRACE("demand type = %d, t = %d, %s\n",demand_type, t,time_stamp_str );
				double ratio = rsDemandProfile.GetDouble(time_stamp_str,bExist,false);

				if(bExist && ratio > 0.0000001)
				{
					element.time_dependent_ratio[t] = ratio;
					bWithDataFlag = true;
				}

			}

			if(from_zone_id == 0 && demand_type ==0 && !bWithDataFlag)   // no valid data
				break;

			m_pDoc->m_DemandProfileVector.push_back (element);

			rsDemandProfile.MoveNext ();
		}
		rsDemandProfile.Close();

		str_msg.Format ( "%d temporal profile records imported", m_pDoc->m_DemandProfileVector.size());
		m_MessageList.AddString (str_msg);

	}

	///// VOT distribution
	strSQL = m_pDoc->ConstructSQL("5-VOT-distribution");

	// Read record

	if(strSQL.GetLength() > 0)
	{
		CRecordsetExt rsVOT(&m_pDoc->m_Database);
		rsVOT.Open(dbOpenDynaset, strSQL);

		while(!rsVOT.IsEOF())
		{

			int pricing_type;
			float percentage; float VOT;
			pricing_type = rsVOT.GetLong(CString("demand_type"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field demand_type_no cannot be found in the 5-VOT-distribution table.");
				return;
			}
			if(pricing_type <=0)
				break;

			VOT = rsVOT.GetDouble(CString("VOT_dollar_per_hour"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field VOT_dollar_per_hour cannot be found in the 5-VOT-distribution table.");
				return;
			}

			percentage = rsVOT.GetDouble(CString("VOT_percentage"),bExist,false);
			if(!bExist)
			{
				AfxMessageBox("Field VOT_percentage cannot be found in the 4-3-VOT-distribution table.");
				return;
			}

			DTAVOTDistribution element;
			element.demand_type = pricing_type;
			element.percentage  = percentage *100;
			element.VOT = VOT;

			m_pDoc->m_VOTDistributionVector.push_back(element);

			rsVOT.MoveNext ();
		}
		rsVOT.Close();

		str_msg.Format ( "%d Value of Time (VOT) records imported.", m_pDoc->m_VOTDistributionVector.size());
		m_MessageList.AddString (str_msg);

	}

	//// emission rate
	strSQL = m_pDoc->ConstructSQL("6-vehicle-emission-rate");;

	if(strSQL.GetLength() > 0)
	{		// Read record
		CRecordsetExt rsEmissionRate(&m_pDoc->m_Database);
		rsEmissionRate.Open(dbOpenDynaset, strSQL);

		while(!rsEmissionRate.IsEOF())
		{


			int vehicle_type = rsEmissionRate.GetLong(CString("vehicle_type"),bExist,false);

			if(vehicle_type <= 0)  //reading empty line
				break;

			if(bExist)
			{
				int opModeID = rsEmissionRate.GetLong(CString("opModeID"),bExist,false);
				CEmissionRate element;
				element.meanBaseRate_TotalEnergy = rsEmissionRate.GetDouble(CString("meanBaseRate_TotalEnergy_(J/hr)"),bExist,false);
				element.meanBaseRate_CO2 = rsEmissionRate.GetDouble(CString("meanBaseRate_CO2_(g/hr)"),bExist,false);
				element.meanBaseRate_NOX = rsEmissionRate.GetDouble(CString("meanBaseRate_NOX_(g/hr)"),bExist,false);
				element.meanBaseRate_CO = rsEmissionRate.GetDouble(CString("meanBaseRate_CO_(g/hr)"),bExist,false);
				element.meanBaseRate_HC = rsEmissionRate.GetDouble(CString("meanBaseRate_HC_(g/hr)"),bExist,false);

				ASSERT(vehicle_type < MAX_VEHICLE_TYPE_SIZE);
				ASSERT(opModeID < _MAXIMUM_OPERATING_MODE_SIZE);

				m_pDoc->EmissionRateData[vehicle_type][opModeID] = element;
			}
			rsEmissionRate.MoveNext ();
		}
		rsEmissionRate.Close();

		str_msg.Format ( "%d emissions records imported.", m_pDoc->m_VOTDistributionVector.size());
		m_MessageList.AddString (str_msg);
	}

	return;  // not reading sensor data for now.

	/// point sensor table
	strSQL = m_pDoc->ConstructSQL("9-sensor");;

	int sensor_count = 0;

	if(strSQL.GetLength() > 0)
	{
		// Read record
		CRecordsetExt rsAADT(&m_pDoc->m_Database);
		rsAADT.Open(dbOpenDynaset, strSQL);

		while(!rsAADT.IsEOF())
		{

		DTA_sensor sensor;
		sensor.FromNodeNumber =  rsAADT.GetLong(CString("from_node_id"),bExist,false);
		sensor.ToNodeNumber =  rsAADT.GetLong(CString("to_node_id"),bExist,false);
		sensor.SensorType  =  rsAADT.GetLong(CString("sensor_type"),bExist,false);
		
		if(!bExist)
		{
			AfxMessageBox("Field sensor_type cannot be found in the 9-sensor table.");
			return;
		}
		sensor.SensorID   =  rsAADT.GetLong(CString("sensor_id"),bExist,false);
		if(!bExist)
		{
			AfxMessageBox("Field sensor_id cannot be found in the 9-sensor table.");
			return;
		}
		int direction = rsAADT.GetLong(CString("AADT_direction"),bExist,false);
		if(!bExist)
		{
			AfxMessageBox("Field AADT_direction cannot be found in the 9-sensor table.");
			return;
		}

		sensor.AADT = rsAADT.GetDouble(CString("AADT"),bExist,true);
		sensor.peak_hour_factor = rsAADT.GetDouble(CString("peak_hour_factor"),bExist,true);

		bool b_find_link_flag = false;
		DTALink* pLink;

		if(direction ==0)
			sensor.AADT  = sensor.AADT/2;


		if(sensor.FromNodeNumber!= 0 && sensor.ToNodeNumber!=0)
		{

			pLink = m_pDoc->FindLinkWithNodeNumbers(sensor.FromNodeNumber , sensor.ToNodeNumber,"9-AADT", false );

			if(pLink!=NULL)
			{
				sensor.LinkID = pLink->m_LinkNo ;
				m_pDoc->m_SensorVector.push_back(sensor);
				m_pDoc->m_SensorIDtoLinkMap[sensor.SensorID] = pLink;

				pLink->m_bSensorData  = true;
				pLink->m_ReferenceFlowVolume = sensor.AADT*sensor.peak_hour_factor;
				pLink->m_AADT = sensor.AADT;
				pLink->m_PeakHourFactor = sensor.peak_hour_factor;
				
				b_find_link_flag  = true;

				if(direction ==0)
				{

					// switch from and to node numbers
					int temp  = sensor.FromNodeNumber;
					sensor.FromNodeNumber = sensor.ToNodeNumber;
					sensor.ToNodeNumber = temp;


				pLink = m_pDoc->FindLinkWithNodeNumbers(sensor.FromNodeNumber,sensor.ToNodeNumber,"9-AADT", false );

					if(pLink!=NULL)
					{
						sensor.LinkID = pLink->m_LinkNo ;
						m_pDoc->m_SensorVector.push_back(sensor);  // add the sensor for the reversed link
						m_pDoc->m_SensorIDtoLinkMap[sensor.SensorID] = pLink;

						pLink->m_bSensorData  = true;
						pLink->m_ReferenceFlowVolume = sensor.AADT*sensor.peak_hour_factor;
						pLink->m_AADT = sensor.AADT;
						pLink->m_PeakHourFactor = sensor.peak_hour_factor;
					}else
					{
					str_msg.Format ("(Reversed) Link %d -> %d in 9-AADT does not exit in the existing link data.",sensor.FromNodeNumber , sensor.ToNodeNumber);
					m_MessageList.AddString (str_msg);
					}
				
				}
				}else
			{

				str_msg.Format ("Link %d -> %d in 9-AADT does not exit in the existing link data.",sensor.FromNodeNumber , sensor.ToNodeNumber);
				m_MessageList.AddString (str_msg);

			}
		}
		
		float x = rsAADT.GetDouble(CString("x"),bExist,false);
			if(!b_find_link_flag && !bExist)
			{
				AfxMessageBox("No link information is specified so field x is required in the AADT table.");
				return;
			}

		float y = rsAADT.GetDouble(CString("y"),bExist,false);
			if(!b_find_link_flag && !bExist)
			{
				AfxMessageBox("No link information is specified so field y is required in the AADT table.");
				return;
			}
			CString orientation = rsAADT.GetCString(CString("orientation"));

			DTALink* pMatchedLink  = m_pDoc->FindLinkFromSensorLocation(x,y,orientation);

			if(pMatchedLink!=NULL && !b_find_link_flag)
				{
				pMatchedLink ->m_FromNodeNumber ;
				pMatchedLink ->m_ToNodeNumber  ;
				pMatchedLink->m_AADT = sensor.AADT;
				pMatchedLink->m_PeakHourFactor = sensor.peak_hour_factor;
				pMatchedLink->m_ReferenceFlowVolume = sensor.AADT*sensor.peak_hour_factor;

				sensor.RelativeLocationRatio = 0.5;		

			}

/*
			if(b_find_link_flag && (pMatchedLink !=  pLink))
			{
				str_msg.Format("Link %d -> %d has a different matched link %d->%d based on the x/y coordinate data. ", 
					pLink ->m_FromNodeNumber, pLink ->m_ToNodeNumber, pMatchedLink ->m_FromNodeNumber, pMatchedLink ->m_ToNodeNumber);
				m_MessageList.AddString (str_msg);

			}
*/

		rsAADT.MoveNext ();
		sensor_count ++;
		}
	rsAADT.Close();
	str_msg.Format ( "%d AADT records imported.", sensor_count);
	m_MessageList.AddString (str_msg);
}


/*

strSQL = m_pDoc->ConstructSQL("5-2-avi-sensor-location");

if(strSQL.GetLength() > 0)
{
// Read record
CRecordsetExt rsAVILink(&m_pDoc->m_Database);
rsAVILink.Open(dbOpenDynaset, strSQL);

while(!rsAVILink.IsEOF())
{
CAVISensorPair element; 

element.sensor_pair_id  = rsAVILink.GetLong(CString("sensor_pair_id"),bExist,false);

if(!bExist)
{
m_MessageList.AddString ("Field sensor_pair_id cannot be found in the AVI-sensor-location table.");
rsAVILink.Close();
return;
}

if(element.sensor_pair_id  == 0)
break;

element.from_node_id  = rsAVILink.GetLong(CString("from_node_id"),bExist,false);

if(!bExist)
{
m_MessageList.AddString ("Field from_node_id cannot be found in the AVI-sensor-location table.");
rsAVILink.Close();
return;
}

element.to_node_id  = rsAVILink.GetLong(CString("to_node_id"),bExist,false);
if(!bExist)
{
m_MessageList.AddString ("Field to_node_id cannot be found in the AVI-sensor-location table.");
rsAVILink.Close();
return;
}

if(m_pDoc->m_NodeNametoIDMap.find(element.from_node_id)== m_pDoc->m_NodeNametoIDMap.end())
{
str_msg.Format("from_node_id %d at pair %d cannot be found in the node table!",element.from_node_id, element.sensor_pair_id);
m_MessageList.AddString(str_msg);
rsAVILink.Close();
return;
}

if(m_pDoc->m_NodeNametoIDMap.find(element.to_node_id)== m_pDoc->m_NodeNametoIDMap.end())
{
str_msg.Format("to_node_id %d at pair %d cannot be found in the node table!",element.to_node_id, element.sensor_pair_id);
m_MessageList.AddString(str_msg);
rsAVILink.Close();
return;
}

DTALink* pLink = m_pDoc->AddNewLink(element.from_node_id, element.to_node_id, true, true);
if(pLink!=NULL)
{
pLink->ResetMOEAry (m_pDoc->m_NumberOfDays * 1440);
element.pLink = pLink;
m_pDoc->m_AVISensorMap[element.sensor_pair_id] = element;
}

rsAVILink.MoveNext ();
}
rsAVILink.Close  ();
str_msg.Format("%d AVI sensor location records loaded.",m_pDoc->m_AVISensorMap.size());
m_MessageList.AddString (str_msg);
}
*/
}



void CDlg_ImportNetwork::OnBnClickedImportNetworkOnly()
{
	m_bImportNetworkOnly = true;
	OnBnClickedImport();
	m_bImportNetworkOnly = false;  //reset flag
}


void CDlg_ImportNetwork::OnBnClickedImportSensorData()
{
	/*
	// Read record


	strSQL = m_pDoc->ConstructSQL("AVI-sensor-data");

	if(strSQL.GetLength() > 0)
	{
	int number_of_samples = 0;
	// Read record
	CRecordsetExt rsSensorData(&m_pDoc->m_Database);
	rsSensorData.Open(dbOpenDynaset, strSQL);

	while(!rsSensorData.IsEOF())
	{
	int  sensor_pair_id =  rsSensorData.GetLong(CString("sensor_pair_id"),bExist,false);
	TRACE("\nsensor_pair_id  = %d", sensor_pair_id);
	if(!bExist)
	{
	AfxMessageBox("Field sensor_pair_id cannot be found in the AVI-sensor-data table.");
	return;
	}

	if(sensor_pair_id == 0)
	break;

	int  modeling_timestamp_day =  rsSensorData.GetLong(CString("modeling_timestamp_day"),bExist,false);
	if(!bExist)
	{
	AfxMessageBox("Field modeling_timestamp_day cannot be found in the AVI-sensor-data table.");
	return;
	}

	int  modeling_timestamp_min =  rsSensorData.GetLong(CString("modeling_timestamp_min"),bExist,false);
	if(!bExist)
	{
	AfxMessageBox("Field modeling_timestamp_min cannot be found in the AVI-sensor-data table.");
	return;
	}
	float  total_link_flow_per_interval =  rsSensorData.GetDouble(CString("sample_count"),bExist,false);
	if(!bExist)
	{
	AfxMessageBox("Field sample_count cannot be found in the AVI-sensor-data table.");
	return;
	}

	float  AvgTravelTime = rsSensorData.GetDouble(CString("travel_time_in_min"),bExist,false);
	if(!bExist)
	{
	AfxMessageBox("Field travel_time_in_min cannot be found in the AVI-sensor-data table.");
	return;
	}



	if(m_pDoc->m_AVISensorMap.find(sensor_pair_id) == m_pDoc->m_AVISensorMap.end())
	{
	str_msg.Format("sensor_pair_id %d defined in row %d of the AVI-sensor-data table has not been defined in the AVI-sensor-location table!",sensor_pair_id,number_of_samples+2);
	AfxMessageBox(str_msg);
	rsSensorData.Close();
	return;
	}


	DTALink* pLink = m_pDoc->m_AVISensorMap[sensor_pair_id].pLink;

	m_pDoc->m_AVISensorMap[sensor_pair_id].number_of_samples +=1;
	if(pLink!=NULL)
	{
	int t  = ((modeling_timestamp_day - 1)*1440+ modeling_timestamp_min) ;

	if(t>=0  && t< (pLink->m_LinkMOEAry.size()-m_pDoc->m_AVISamplingTimeInterval))
	{

	float AvgLinkSpeed = pLink->m_SpeedLimit ;

	if (AvgTravelTime>0)
	AvgLinkSpeed = pLink->m_Length / (AvgTravelTime/60.0);  // per hour

	if(m_pDoc->m_bSimulationDataLoaded == false)  // simulation data not loaded
	{

	pLink->m_LinkMOEAry[ t].ObsLinkFlow = total_link_flow_per_interval*60/m_pDoc->m_SamplingTimeInterval/pLink->m_NumLanes;  // convert to per hour link flow
	pLink->m_LinkMOEAry[ t].ObsSpeed = AvgLinkSpeed; 
	pLink->m_LinkMOEAry[ t].ObsTravelTimeIndex = pLink->m_SpeedLimit /max(1,AvgLinkSpeed)*100;

	pLink->m_LinkMOEAry[t].ObsDensity = pLink->m_LinkMOEAry[t].ObsLinkFlow / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeed);

	// copy data to other intervals
	for(int tt = 1; tt<m_pDoc->m_AVISamplingTimeInterval; tt++)
	{
	if(tt < pLink->m_LinkMOEAry.size())
	{
	pLink->m_LinkMOEAry[ t+tt].ObsLinkFlow = pLink->m_LinkMOEAry[t].ObsLinkFlow ;
	pLink->m_LinkMOEAry[t+tt].ObsSpeed = pLink->m_LinkMOEAry[t].ObsSpeed;
	pLink->m_LinkMOEAry[t+tt].ObsDensity = pLink->m_LinkMOEAry[t].ObsDensity;
	pLink->m_LinkMOEAry[t+tt].ObsTravelTimeIndex = pLink->m_LinkMOEAry[t].ObsTravelTimeIndex;
	}

	}
	}else // simulation data loaded
	{

	pLink->m_LinkMOEAry[ t].ObsFlowCopy = total_link_flow_per_interval*60/m_pDoc->m_SamplingTimeInterval/pLink->m_NumLanes;  // convert to per hour link flow
	pLink->m_LinkMOEAry[ t].ObsSpeedCopy = AvgLinkSpeed; 
	pLink->m_LinkMOEAry[ t].ObsTravelTimeIndexCopy = pLink->m_SpeedLimit /max(1,AvgLinkSpeed)*100;


	pLink->m_LinkMOEAry[t].ObsDensityCopy = pLink->m_LinkMOEAry[t].ObsFlowCopy / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeedCopy);

	// copy data to other intervals
	for(int tt = 1; tt<m_pDoc->m_AVISamplingTimeInterval; tt++)
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

	number_of_samples++;
	rsSensorData.MoveNext ();
	}
	rsSensorData.Close  ();
	str_msg.Format("%d AVI sensor data records loaded.",m_pDoc->m_AVISensorMap.size());
	m_MessageList.AddString (str_msg);
	}
	*/
}

void CDlg_ImportNetwork::OnLbnSelchangeList1()
{
	// TODO: Add your control notification handler code here
}

void CDlg_ImportNetwork::OnBnClickedExportData()
{
	CString m_CSV_FileName;
	CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"(*.csv)|*.csv||", NULL);
	if(dlg.DoModal() == IDOK)
	{
			FILE* st;
			fopen_s(&st,dlg.GetPathName(),"w");

			if(st!=NULL)
			{
				for(int i=0; i< m_MessageList.GetCount (); i++)	// if one of "all" options is selected, we need to narrow down to OD pair
				{
					char m_Text[200];
					m_MessageList.GetText (i, m_Text);
					fprintf(st,"%s\n",m_Text);
				}
				fclose(st);

			}else
			{   CString str;
				str.Format("The file %s could not be opened.\nPlease check if it is opened by Excel.", dlg.GetPathName());
				AfxMessageBox(str);
			}
	
	m_pDoc->OpenCSVFileInExcel (dlg.GetPathName());

	}
}

void CDlg_ImportNetwork::OnBnClickedButtonViewSampleFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleExcelNetworkFile = pMainFrame->m_CurrentDirectory + m_pDoc->m_SampleExcelNetworkFile;
	m_pDoc->OpenCSVFileInExcel (SampleExcelNetworkFile);

}

void CDlg_ImportNetwork::OnBnClickedButtonLoadSampleFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleExcelNetworkFile = pMainFrame->m_CurrentDirectory + m_pDoc->m_SampleExcelNetworkFile;
	m_Edit_Excel_File = SampleExcelNetworkFile;
	UpdateData(false);
}

void CDlg_ImportNetwork::OnBnClickedButtonViewSampleCsvFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	m_pDoc->m_ProjectFile.Format("%s%s", pMainFrame->m_CurrentDirectory, _T("\\Sample-Portland-SHRP2-C05-subarea\\Portland_subarea.dlp"));

	CString str;
		str.Format("The current project file is saved as %s", MB_ICONINFORMATION);
	AfxMessageBox(str);

}

void CDlg_ImportNetwork::OnBnClickedButtonLoadSampleCsvFile()
{
	// TODO: Add your control notification handler code here
}

void CDlg_ImportNetwork::OnBnClickedButtonViewSampleProjectFolder()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleProjectFolder = "\\Sample-Portland-SHRP2-C05-subarea";
	SampleProjectFolder = pMainFrame->m_CurrentDirectory + SampleProjectFolder;
	ShellExecute( NULL,  "explore", SampleProjectFolder, NULL,  NULL, SW_SHOWNORMAL );
}

void CDlg_ImportNetwork::OnBnClickedImport2()
{
	OnBnClickedImport();
}
