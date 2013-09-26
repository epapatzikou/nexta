// TLiteDoc.h : interface of the CTLiteDoc class
//
//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com)

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html


//    This file is part of NeXTA Version 3 (Open-source).

//    NEXTA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    NEXTA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with NEXTA.  If not, see <http://www.gnu.org/licenses/>.

// Dlg_ImportNetwork.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"

#ifndef _WIN64
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
, m_AutogenerateNodeFlag(FALSE)
, m_ImportZoneData(TRUE)
, m_bAddConnectorsForIsolatedNodes(FALSE)
, m_bUseLinkTypeForDefaultValues(FALSE)
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
	DDX_Control(pDX, IDC_LIST1, m_MessageList);
	DDX_Check(pDX, IDC_CHECK_ZONE_DATA, m_ImportZoneData);

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
	static char BASED_CODE szFilter[] = "Excel File (*.xls)|*.xls||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		UpdateData(true);

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
	m_pDoc->ClearNetworkData();

	m_MessageList.ResetContent ();

	UpdateData(true);



	bool bExist=true;

	CString strSQL;
	CString str_msg;

	UpdateData(true);

	if(m_Edit_Excel_File.GetLength () ==0)
	{
		
		AfxMessageBox("Please first provide Excel file to be imported.");

		return;
	}

	if(m_pDoc->m_Database.IsOpen ())
		m_pDoc->m_Database.Close ();

	m_pDoc->m_Database.Open(m_Edit_Excel_File, false, true, "excel 5.0; excel 97; excel 2000; excel 2003");

	// Open the EXCEL file
	std::string itsErrorMessage;


	// detetect if nodes will be automatically generated. 

			// Read record
		strSQL = m_pDoc->ConstructSQL("3-LINK");

		if(strSQL.GetLength () > 0)
		{
			CRecordsetExt rsLink(&m_pDoc->m_Database);
			rsLink.Open(dbOpenDynaset, strSQL);
			int from_node_id;
			while(!rsLink.IsEOF())
			{
			
				from_node_id = rsLink.GetLong(CString("from_node_id"),bExist,false);

				if(from_node_id==0)
				{
				m_AutogenerateNodeFlag = true;

//				AfxMessageBox("Field from_node_id is empty, so from_node and to_node fields will be automatically generated based on the geometry data from the link layer.", MB_ICONINFORMATION);
				}else
				{
				m_AutogenerateNodeFlag = false;
				}

				int number_of_lanes = rsLink.GetLong(CString("number_of_lanes"),bExist,false);
				float speed_limit_in_mph= rsLink.GetLong(CString("speed_limit"),bExist,false);
				float capacity_in_pcphpl= rsLink.GetDouble(CString("lane_capacity_in_vhc_per_hour"),bExist,false);


				if(number_of_lanes <=0 || speed_limit_in_mph <1 || capacity_in_pcphpl<1)
				{
				m_bUseLinkTypeForDefaultValues = true;

				str_msg.Format ( "Fields number_of_lanes, speed_limit_in_mph or capacity_in_pcphpl might be empty, so those fields will be automatically generated based on link type from the link layer.");
				m_MessageList.AddString (str_msg);

				}

				break;


			}
		}


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

			double x;
			double y;

			int control_type = 0;

				std::vector<CCoordinate> CoordinateVector;

				CString geometry_str;
				geometry_str= rsNode.GetCString(CString("geometry"),false);

				if(geometry_str.GetLength () > 0 )
				{

				CT2CA pszConvertedAnsiString (geometry_str);

				// construct a std::string using the LPCSTR input
				std::string geo_string (pszConvertedAnsiString);

				CGeometry geometry(geo_string);
				CoordinateVector = geometry.GetCoordinateList();

				if(CoordinateVector.size() >=1)
				{
				x = CoordinateVector[0].X;
				y = CoordinateVector[0].Y;
				}else
				{
				
					AfxMessageBox("Processing error in parsing geometry string in 1_node data sheet.");
					return;
				}

				}else
				{

				 x = rsNode.GetDouble(CString("x"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString ("Field x cannot be found in the node table.");
					rsNode.Close();
					return;
				}

				y = rsNode.GetDouble(CString("y"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString ("Field y cannot be found in the node table.");
					rsNode.Close();
					return;
				}
				}

			// Create and insert the node
			DTANode* pNode = new DTANode;
			pNode->pt.x = x;
			pNode->pt.y = y;

			pNode->m_NodeNumber = id;
			pNode->m_NodeNo = i;
			pNode->m_ZoneID = 0;
			pNode->m_ControlType = control_type;

			m_pDoc->m_NodeSet.push_back(pNode);
			m_pDoc->m_NodeNoMap[i] = pNode;
			m_pDoc->m_NodeNotoNumberMap[i] = id;
			m_pDoc->m_NodeNumberMap[id] = pNode;

			m_pDoc->m_NodeNumbertoNodeNoMap[id] = i;
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

	int connector_link_type =  0 ;

	// Read record
	strSQL = m_pDoc->ConstructSQL("2-LINK-TYPE");
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
				CString Message;
				Message.Format("Field link_type cannot be found in the link-type sheeet.");

				m_MessageList.AddString (Message);
				return;
			}
			if(link_type_number ==0)
				break;

			element.link_type = link_type_number;
			element.link_type_name  = rsLinkType.GetCString(CString("link_type_name"));
			element.type_code    = rsLinkType.GetCString (CString("type_code"));

			if(element.type_code.find("c") != string::npos)
					connector_link_type = element.link_type;

			element.default_lane_capacity     = rsLinkType.GetLong(CString("default_lane_capacity"),bExist,false);
			element.default_speed      = rsLinkType.GetFloat  (CString("default_speed_limit"));
			element.default_number_of_lanes       = rsLinkType.GetLong (CString("default_number_of_lanes"),bExist,false);
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

	if(m_pDoc->m_NodeSet.size() == 0 && m_AutogenerateNodeFlag == true)
	{

		str_msg.Format ( "Worksheet 1-node contain 0 node.");
		m_MessageList.AddString (str_msg);
		str_msg.Format ( "The geometry field in the link table is used to generate node info.");
		m_MessageList.AddString (str_msg);

	}else
	{
		str_msg.Format ( "%d nodes have been successfully imported.",m_pDoc->m_NodeSet.size());
		m_MessageList.AddString (str_msg);
	}
	
	/////////////////////////////////////////////////////////////////////

		// Read record to obtain the overall max and min x and y;

	double min_x = 0;
	double max_x = 0;
	double min_y = 0;
	double max_y = 0;

	bool b_RectangleInitialized = false;

	if(m_AutogenerateNodeFlag)
	{
		strSQL = m_pDoc->ConstructSQL("3-LINK");

		if(strSQL.GetLength () > 0)
		{
			CRecordsetExt rsLink(&m_pDoc->m_Database);
			rsLink.Open(dbOpenDynaset, strSQL);

			int from_node_id;
			int to_node_id ;
			while(!rsLink.IsEOF())
			{
				std::vector<CCoordinate> CoordinateVector;

				CString geometry_str;
				geometry_str= rsLink.GetCString(CString("geometry"),false);

				if(m_AutogenerateNodeFlag && geometry_str.GetLength () ==0)
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

				if(b_RectangleInitialized==false && CoordinateVector.size()>=1)
				{
				
					min_x = max_x = CoordinateVector[0].X;
					min_y = max_y = CoordinateVector[0].Y;
					b_RectangleInitialized = true;

				}else
				{

						min_x= min(min_x,CoordinateVector[0].X);
						min_x= min(min_x,CoordinateVector[CoordinateVector.size()-1].X);

						min_y= min(min_y,CoordinateVector[0].Y);
						min_y= min(min_y,CoordinateVector[CoordinateVector.size()-1].Y);

						max_x= max(max_x,CoordinateVector[0].X);
						max_x= max(max_x,CoordinateVector[CoordinateVector.size()-1].X);

						max_y= max(max_y,CoordinateVector[0].Y);
						max_y= max(max_y,CoordinateVector[CoordinateVector.size()-1].Y);
			
				}
				}

	
				rsLink.MoveNext();
				//				TRACE("reading line %d\n", line_no);
			}

			rsLink.Close();
		}
	}

	double min_distance_threadshold_for_overlapping_nodes = ((max_y- min_y) + (max_x - min_x))/100000.0;

	/////////////////////////////////////////////////////////////////////
		// Read record
		strSQL = m_pDoc->ConstructSQL("3-LINK");

		bool b_default_number_of_lanes_used = false;
		bool b_default_lane_capacity_used = false;

		int number_of_records_read = 0;

		int line_no = 2;

		if(strSQL.GetLength () > 0)
		{
			m_pDoc->m_bLinkToBeShifted = true;
			CRecordsetExt rsLink(&m_pDoc->m_Database);
			rsLink.Open(dbOpenDynaset, strSQL);
			i = 0;
			float default_distance_sum = 0;
			CString DTASettingsPath = m_pDoc->m_ProjectDirectory+"DTASettings.ini";
			float default_AADT_conversion_factor = g_GetPrivateProfileDouble("safety_planning", "default_AADT_conversion_factor", 0.1, DTASettingsPath);	


			float length_sum = 0;
			int from_node_id;
			int to_node_id ;
			while(!rsLink.IsEOF())
			{
				if(m_AutogenerateNodeFlag == false)
				{

				from_node_id = rsLink.GetLong(CString("from_node_id"),bExist,false);
				if(!bExist ) 
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
				}else
				{
					if(from_node_id == 0 && m_pDoc->m_LinkSet.size() ==0)
					{
					
					m_MessageList.AddString ("Field from_node_id has no valid data in the link table.");

					AfxMessageBox("Field from_node_id has no valid data in the link table.\n");
					rsLink.Close();
					break;
					}
				

				}

				to_node_id = rsLink.GetLong(CString("to_node_id"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString("Field to_node_id cannot be found in the link table.");
					rsLink.Close();
					return;
				}


				if(from_node_id==0 && to_node_id ==0)  // test twice here for from and to nodes
					break;


				}
				long link_id =  rsLink.GetLong(CString("link_id"),bExist,false);
				if(!bExist)
					link_id = 0;

				int type = rsLink.GetLong(CString("link_type"),bExist,false);
				if(!bExist) 
				{
					str_msg.Format("Field link_type cannot be found or has no value at row %d in the link sheet. Skip record.", line_no);
					m_MessageList.AddString(str_msg);
					rsLink.MoveNext();
					continue;
				}


				if(m_AutogenerateNodeFlag == false && m_pDoc->m_NodeNumbertoNodeNoMap.find(from_node_id)== m_pDoc->m_NodeNumbertoNodeNoMap.end())
				{
					str_msg.Format("from_node_id %d at row %d cannot be found in the link sheet!",from_node_id, line_no);
					m_MessageList.AddString(str_msg);
					rsLink.MoveNext();
					continue;
				}

				if(m_AutogenerateNodeFlag == false && m_pDoc->m_NodeNumbertoNodeNoMap.find(to_node_id)== m_pDoc->m_NodeNumbertoNodeNoMap.end())
				{
					str_msg.Format("to_node_id %d at row %d cannot be found in the link sheet!",to_node_id, line_no);
					m_MessageList.AddString(str_msg);
					rsLink.MoveNext();
					continue;
				}

				std::vector<CCoordinate> CoordinateVector;

				CString geometry_str;
				geometry_str= rsLink.GetCString(CString("geometry"),false);

				if(m_AutogenerateNodeFlag && geometry_str.GetLength () ==0)
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

				if(m_AutogenerateNodeFlag&& CoordinateVector.size() > 0)  // add nodes
				{

					from_node_id = m_pDoc->FindNodeNumberWithCoordinate(CoordinateVector[0].X,CoordinateVector[0].Y,min_distance_threadshold_for_overlapping_nodes);
					// from node
					if(from_node_id == 0)
					{
							GDPoint	pt;
							pt.x = CoordinateVector[0].X;
							pt.y = CoordinateVector[0].Y;
							
							bool ActivityLocationFlag = false;
							if(m_pDoc->m_LinkTypeMap[type ].IsConnector ()) // adjacent node of connectors
								ActivityLocationFlag = true;

							DTANode* pNode = m_pDoc->AddNewNode(pt, from_node_id, 0,ActivityLocationFlag);
							from_node_id = pNode->m_NodeNumber;  // update to_node_id after creating new node
							pNode->m_bCreatedbyNEXTA = true;

					}

					// to node
					to_node_id =  m_pDoc->FindNodeNumberWithCoordinate(CoordinateVector[CoordinateVector.size()-1].X,CoordinateVector[CoordinateVector.size()-1].Y,min_distance_threadshold_for_overlapping_nodes);
					// from node
					if(to_node_id==0)
					{
							GDPoint	pt;
							pt.x = CoordinateVector[CoordinateVector.size()-1].X;
							pt.y = CoordinateVector[CoordinateVector.size()-1].Y;

							bool ActivityLocationFlag = false;
							if(m_pDoc->m_LinkTypeMap[type ].IsConnector ()) // adjacent node of connectors
								ActivityLocationFlag = true;

							DTANode* pNode = m_pDoc->AddNewNode(pt, to_node_id, 0,ActivityLocationFlag);
							to_node_id = pNode->m_NodeNumber;  // update to_node_id after creating new node
							pNode->m_bCreatedbyNEXTA = true;
					}

				}

				}

				if(m_bRemoveConnectors && m_pDoc->m_LinkTypeMap[type ].IsConnector()) 
				{  // skip connectors
					rsLink.MoveNext();
					continue;
				}


				DTALink* pExistingLink =  m_pDoc->FindLinkWithNodeIDs(m_pDoc->m_NodeNumbertoNodeNoMap[from_node_id],m_pDoc->m_NodeNumbertoNodeNoMap[to_node_id]);

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

				float length = rsLink.GetDouble(CString("length"),bExist,false);
				if(!bExist) 
				{
					m_MessageList.AddString ("Field length cannot be found in the link table.");
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
				if(m_bUseLinkTypeForDefaultValues)
				{
				if(number_of_lanes<1)
				{
					if(m_pDoc->m_LinkTypeMap.find(type) != m_pDoc->m_LinkTypeMap.end())
					{

						number_of_lanes = m_pDoc->m_LinkTypeMap[type].default_number_of_lanes;

						if(b_default_number_of_lanes_used)
						{
						m_MessageList.AddString("Field number_of_lanes cannot be found in the link table.");
						m_MessageList.AddString("default_number_of_lanes from 2-link-type table is used.");
						b_default_number_of_lanes_used =true;
						}
					}else
					{
						CString link_type_str = rsLink.GetCString(CString("link_type"));


						if(m_pDoc->m_LinkTypeMap.size()>0)
						{
							std::map<int, DTALinkType>::iterator iter = m_pDoc->m_LinkTypeMap.begin ();
							type = iter->first;
						}

						CString link_type_message;
						link_type_message.Format ("link type %s for link %->%d is invald.", link_type_str);
						m_MessageList.AddString(link_type_message);

					}
					
				}else
					{
					m_MessageList.AddString("Field number_of_lanes cannot be found in the link table.");
					m_MessageList.AddString("default_number_of_lanes for this link type has not been defined in link_type table.");
					rsLink.Close();
					return;
					}
				}

				if(number_of_lanes <=0)
				{
					str_msg.Format ("number of lanes for link %d -> %d <= 0. Skip.",from_node_id,to_node_id);
					m_MessageList.AddString(str_msg);
					rsLink.MoveNext();
					continue; 
				}



				float grade= 0;
				float AADT_conversion_factor = 0.1;
				if(!bExist)
				{

				AADT_conversion_factor = default_AADT_conversion_factor;
				}

				float speed_limit_in_mph= rsLink.GetLong(CString("speed_limit"),bExist,false);

				if(speed_limit_in_mph <1 && m_bUseLinkTypeForDefaultValues)
				{
					if(m_pDoc->m_LinkTypeMap.find(type) != m_pDoc->m_LinkTypeMap.end())
					{

						speed_limit_in_mph = m_pDoc->m_LinkTypeMap[type].default_speed ;
						bExist = true;


					}
				}
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

				float capacity_in_pcphpl= rsLink.GetDouble(CString("lane_capacity_in_vhc_per_hour"),bExist,false);
				if(capacity_in_pcphpl<0.1 && m_bUseLinkTypeForDefaultValues)
				{
					if(m_pDoc->m_LinkTypeMap.find(type) != m_pDoc->m_LinkTypeMap.end())
					{

						capacity_in_pcphpl = m_pDoc->m_LinkTypeMap[type].default_lane_capacity ;

						if(b_default_lane_capacity_used)
						{
						m_MessageList.AddString("Field capacity_in_veh_per_hour_per_lane cannot be found in the link table.");
						m_MessageList.AddString("default_lane_capacity from 2-link-type table is used.");
						b_default_lane_capacity_used =true;
						}
					
					}

				}

				if(capacity_in_pcphpl<0)
				{

					str_msg.Format ( "Link %d -> %d has a negative capacity, please sort the link table by capacity_in_veh_per_hour_per_lane and re-check it!",from_node_id,to_node_id);
					AfxMessageBox(str_msg, MB_ICONINFORMATION);
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

				float k_jam, wave_speed_in_mph;

				if(type==1)
				{
					k_jam = 220;
				}else
				{
					k_jam = 190;
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

				if(m_AutogenerateNodeFlag == false)
				{
				// no geometry information
				CCoordinate cc_from, cc_to; 
				cc_from.X = m_pDoc->m_NodeNoMap[m_pDoc->m_NodeNumbertoNodeNoMap[from_node_id]]->pt.x;
				cc_from.Y = m_pDoc->m_NodeNoMap[m_pDoc->m_NodeNumbertoNodeNoMap[from_node_id]]->pt.y;

				cc_to.X = m_pDoc->m_NodeNoMap[m_pDoc->m_NodeNumbertoNodeNoMap[to_node_id]]->pt.x;
				cc_to.Y = m_pDoc->m_NodeNoMap[m_pDoc->m_NodeNumbertoNodeNoMap[to_node_id]]->pt.y;

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

						pLink->m_FromNodeID = m_pDoc->m_NodeNumbertoNodeNoMap[from_node_id];
						pLink->m_ToNodeID= m_pDoc->m_NodeNumbertoNodeNoMap[to_node_id];


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
						pLink->m_FromNodeID = m_pDoc->m_NodeNumbertoNodeNoMap[to_node_id];
						pLink->m_ToNodeID= m_pDoc->m_NodeNumbertoNodeNoMap[from_node_id];


						for(int si = CoordinateVector.size()-1; si >=0; si--)  // we need to put int here as si can be -1. 
						{
							GDPoint	pt;
							pt.x = CoordinateVector[si].X;
							pt.y = CoordinateVector[si].Y;
							pLink->m_ShapePoints .push_back (pt);
						}
					}

					pLink->m_NumberOfLanes= number_of_lanes;
					pLink->m_SpeedLimit= speed_limit_in_mph;
					pLink->m_avg_simulated_speed = pLink->m_SpeedLimit;
					pLink->m_Length= length;  // minimum distance

					if(length < 0.00001) // zero value in length field, we consider no length info.
					{
						float distance_in_mile = g_CalculateP2PDistanceInMileFromLatitudeLongitude(pLink->m_ShapePoints[0], pLink->m_ShapePoints[pLink->m_ShapePoints.size()-1]);
						pLink->m_Length = distance_in_mile;
					}
					default_distance_sum+= pLink->DefaultDistance();
					length_sum += pLink ->m_Length;
	

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
						pLink->m_NumberOfLanes= R_number_of_lanes;
						pLink->m_SpeedLimit= R_speed_limit_in_mph;
						pLink->m_MaximumServiceFlowRatePHPL= R_lane_capacity_in_vhc_per_hour;
						pLink->m_Grade = R_grade;

						pLink->m_avg_simulated_speed = pLink->m_SpeedLimit;
						pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance
						pLink->m_FreeFlowTravelTime = pLink->m_Length / max(1,pLink->m_SpeedLimit) *60.0f;
						pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;
						pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
						pLink->m_link_type= type;



					}

	
					double BPR_alpha_term = 0.15;
					double BPR_beta_term = 4;
					double transit_transfer_time_in_min	= 1;
					double transit_waiting_time_in_min = 3;
					double transit_fare = 1;


					BPR_alpha_term = rsLink.GetDouble(CString("BPR_alpha_term"),bExist,false);

					if(BPR_alpha_term > 0.000001)
					pLink->m_BPR_alpha_term = BPR_alpha_term;

					BPR_beta_term = rsLink.GetDouble(CString("BPR_beta_term"),bExist,false);

					if(BPR_beta_term > 0.00000001)
					pLink->m_BPR_beta_term = BPR_beta_term;


					pLink->m_Kjam = k_jam;
					pLink->m_AADT_conversion_factor  = AADT_conversion_factor;
					pLink->m_Wave_speed_in_mph  = wave_speed_in_mph;

					m_pDoc->m_NodeNoMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumberOfLanes);

					pLink->m_FromPoint = m_pDoc->m_NodeNoMap[pLink->m_FromNodeID]->pt;
					pLink->m_ToPoint = m_pDoc->m_NodeNoMap[pLink->m_ToNodeID]->pt;


					//			pLink->SetupMOE();
					pLink->input_line_no  = line_no;
					
					

					m_pDoc->m_LinkSet.push_back (pLink);
					m_pDoc->m_LinkNoMap[i]  = pLink;

					unsigned long LinkKey = m_pDoc->GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);

					m_pDoc->m_NodeNotoLinkMap[LinkKey] = pLink;

					__int64  LinkKey2 = m_pDoc->GetLink64Key(pLink-> m_FromNodeNumber,pLink->m_ToNodeNumber);
					m_pDoc->m_NodeNumbertoLinkMap[LinkKey2] = pLink;

					m_pDoc->m_LinkNotoLinkMap[i] = pLink;

					m_pDoc->m_LinkIDtoLinkMap[link_id] = pLink;


					m_pDoc->m_NodeNoMap[pLink->m_FromNodeID ]->m_Connections+=1;
					m_pDoc->m_NodeNoMap[pLink->m_ToNodeID ]->m_Connections+=1;

					if(m_pDoc->m_LinkTypeMap[type ].IsConnector ()) // adjacent node of connectors
					{ 
						// mark them as activity location 
					m_pDoc->m_NodeNoMap[pLink->m_FromNodeID ]->m_bZoneActivityLocationFlag = true;					
					m_pDoc->m_NodeNoMap[pLink->m_ToNodeID ]->m_bZoneActivityLocationFlag = true;					
					}


					m_pDoc->m_NodeNoMap[pLink->m_FromNodeID ]->m_OutgoingLinkVector.push_back(i);


					i++;

				}

				rsLink.MoveNext();
				//				TRACE("reading line %d\n", line_no);
				line_no ++;
				number_of_records_read ++;
			}

			rsLink.Close();

			m_pDoc->m_UnitMile  = 1.0f;

			if(length_sum>0.000001f)
				m_pDoc->m_UnitMile=  default_distance_sum /length_sum;

			m_pDoc->m_UnitFeet = m_pDoc->m_UnitMile/5280.0f;  
			m_pDoc->GenerateOffsetLinkBand();

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
			str_msg.Format ( "Worksheet 3-link cannot be found in the given Excel file");
			m_MessageList.AddString (str_msg);
			return;
		}


		if(m_AutogenerateNodeFlag)  // generated when read shape files
		{
		str_msg.Format ( "%d nodes have been successfully imported.",m_pDoc->m_NodeSet.size());
		m_MessageList.AddString (str_msg);
		}

		str_msg.Format ("%d links have been successfully imported from %d records.",m_pDoc->m_LinkSet.size(),number_of_records_read);
		m_MessageList.AddString(str_msg);
	

		// test if we need to add connectors for isolated nodes
		m_bAddConnectorsForIsolatedNodes = false;
		{
			for (std::list<DTANode*>::iterator  iNode = m_pDoc->m_NodeSet.begin(); iNode != m_pDoc->m_NodeSet.end(); iNode++)
			{
				if((*iNode)->m_bCreatedbyNEXTA == false)
				{

					if((*iNode)->m_Connections ==0)
					{
					
					m_bAddConnectorsForIsolatedNodes = true;

					str_msg.Format ("There are isoluated nodes in node table. Connectors will be added automatically.");
					m_MessageList.AddString(str_msg);
					break;
					}
				}

			}
		}

		if(m_bAddConnectorsForIsolatedNodes)
		{

			if(connector_link_type == 0)
			{
				AfxMessageBox("The link type for connectors has not been defined.");

				return;
			}
			m_pDoc->m_DefaultSpeedLimit = 10;
			m_pDoc->m_DefaultCapacity = 10000;
			m_pDoc->m_DefaultLinkType = connector_link_type; //connector

			int number_of_new_connectors = 0;

			for (std::list<DTANode*>::iterator  iNode = m_pDoc->m_NodeSet.begin(); iNode != m_pDoc->m_NodeSet.end(); iNode++)
			{
				if((*iNode)->m_Connections == 0)
				{

					int NodeNumber = m_pDoc->FindNonCentroidNodeNumberWithCoordinate((*iNode)->pt .x ,(*iNode)->pt .y , (*iNode)->m_NodeNumber);
					
					 m_pDoc->AddNewLinkWithNodeNumbers((*iNode)->m_NodeNumber , NodeNumber);
					 m_pDoc->AddNewLinkWithNodeNumbers(NodeNumber,(*iNode)->m_NodeNumber);
				
					number_of_new_connectors ++;
				}

			}
		
			str_msg.Format ("%d connectors have been successfully created",number_of_new_connectors);
			m_MessageList.AddString(str_msg);

		}

	//test if activity location has been defined


	if(m_ImportZoneData == false)
		return;

	// activity location table
	strSQL = m_pDoc->ConstructSQL("5-ACTIVITY-LOCATION");

	//bool bNodeNonExistError = false;
	m_pDoc->m_NodeNotoZoneNameMap.clear ();

	bool bNodeNonExistError = false;
	m_pDoc->m_NodeNotoZoneNameMap.clear ();
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

			map <int, int> :: const_iterator m_Iter = m_pDoc->m_NodeNumbertoNodeNoMap.find(node_name);

			if(m_Iter == m_pDoc->m_NodeNumbertoNodeNoMap.end( ))
			{
				CString m_Warning;
				m_Warning.Format("Node Number %d in the zone tabe has not been defined in the node table", node_name);
				AfxMessageBox(m_Warning);
				return;
			}
			m_pDoc->m_NodeNotoZoneNameMap[m_pDoc->m_NodeNumbertoNodeNoMap[node_name]] = zone_number;

			m_pDoc->m_NodeNoMap [ m_pDoc->m_NodeNumbertoNodeNoMap[node_name] ] ->m_bZoneActivityLocationFlag = true;
			m_pDoc->m_NodeNoMap [ m_pDoc->m_NodeNumbertoNodeNoMap[node_name] ] -> m_ZoneID = zone_number;
			// if there are multiple nodes for a zone, the last node id is recorded.


			DTAActivityLocation element;
			element.ZoneID  = zone_number;
			element.NodeNumber = node_name;

			element.External_OD_flag  = rsZone.GetLong(CString("external_OD_flag"),bExist,false);

			m_pDoc->m_ZoneMap [zone_number].m_ActivityLocationVector .push_back (element);

			if(m_pDoc->m_ODSize < zone_number)
				m_pDoc->m_ODSize = zone_number;

			rsZone.MoveNext ();
			activity_location_count++;
		}
		rsZone.Close();

		str_msg.Format ( "%d activity location records imported.",activity_location_count);
		m_MessageList.AddString (str_msg);
	}

	// ZONE table
	strSQL = m_pDoc->ConstructSQL("4-ZONE");

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
			//	std::vector<CCoordinate> CoordinateVector;

			//	CString geometry_str = rsZone.GetCString(CString("geometry"));

			//	if(geometry_str.GetLength () > 0)
			//	{

			//	CT2CA pszConvertedAnsiString (geometry_str);

			//	// construct a std::string using the LPCSTR input
			//	std::string geo_string (pszConvertedAnsiString);

			//	CGeometry geometry(geo_string);
			//	CoordinateVector = geometry.GetCoordinateList();

			//	m_pDoc->m_ZoneMap [zone_number].m_ZoneID = zone_number;

			//for(unsigned int f = 0; f < CoordinateVector.size(); f++)
			//{
			//	GDPoint pt;
			//	pt.x = CoordinateVector[f].X;
			//	pt.y = CoordinateVector[f].Y;
			//	m_pDoc->m_ZoneMap [zone_number].m_ShapePoints.push_back (pt);
			//}

			//	}
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
				if(m_pDoc->m_LinkTypeMap[(*iLink)->m_link_type ].IsConnector ())  // connectors
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
				CentroidVector.push_back((*iNode )->m_NodeNo );

		}

		for(unsigned int i = 0; i < CentroidVector.size(); i++)
		{
			m_pDoc->DeleteNode (CentroidVector[i]);
		}
		
		str_msg.Format ( "%d centroids deleted",CentroidVector.size());
		m_MessageList.AddString (str_msg);
		
		}

		//import demand

		// restart the data base
	if(m_pDoc->m_Database.IsOpen ())
		m_pDoc->m_Database.Close ();

	m_pDoc->m_Database.Open(m_Edit_Excel_File, false, true, "excel 5.0; excel 97; excel 2000; excel 2003");


	int demand_type = 0;

	m_pDoc->m_ImportDemandColumnFormat = demand_type; // 0 : matrix, 1: column
	if(demand_type == 0)  // matrix
	{
	// activity location table

	strSQL = m_pDoc->ConstructSQL("6-DEMAND-MATRIX");

	//bool bNodeNonExistError = false;
	m_pDoc->m_ImportedDemandVector .clear ();


	if(strSQL.GetLength () > 0)
	{
		CRecordsetExt rsDemand(&m_pDoc->m_Database);
		rsDemand.Open(dbOpenDynaset, strSQL);

		while(!rsDemand.IsEOF())
		{
			int from_zone_id = rsDemand.GetLong(CString("zone_id"),bExist,false);
			if(!bExist) 
			{
			demand_type = 1;
			break;
			}

			if(from_zone_id==0)
			{
				if(m_pDoc->m_ImportedDemandVector.size() ==0)
				{
				demand_type = 1;

				break;
				}

				break; 
			}


			if(m_pDoc-> m_ZoneMap.find(from_zone_id)== m_pDoc->m_ZoneMap.end())
			{
				CString message;
				message.Format("from_zone_id %d at line %d in the demand matrix has not been defined.", from_zone_id, m_pDoc->m_ImportedDemandVector.size() );
				AfxMessageBox(message);
				break;
			}

			if(from_zone_id ==0)
				break;

			int to_zone_id = 0;
		std::map<int, DTAZone>	:: const_iterator itr;

			int to_zone_index = 1;
		for(itr = m_pDoc->m_ZoneMap.begin(); itr != m_pDoc->m_ZoneMap.end(); itr++,to_zone_index++)
		{
			to_zone_id = itr->first;
			double number_of_vehicles = rsDemand.GetDouble(to_zone_index,bExist,false);
			if(!bExist) 
			{
				CString messsage;
				messsage.Format("Field %d cannot be found in the demand matrix.", to_zone_id);
				AfxMessageBox(messsage);
				return;
			}

			if(number_of_vehicles < -0.1)
			{
				CString message;
				message.Format("number_of_vehicles %f in the demand matrix is invalid.", number_of_vehicles);
				AfxMessageBox(message);
				break;
			}

			DTA_demand element;
			element.from_zone_id = from_zone_id;
			element.to_zone_id = to_zone_id;
			element.number_of_vehicles = number_of_vehicles;

			m_pDoc->m_ImportedDemandVector.push_back (element);

		}  // for all to zone id

		
	
			rsDemand.MoveNext ();

		}
		rsDemand.Close();

		str_msg.Format ( "%d demand element records imported.", m_pDoc->m_ImportedDemandVector.size());
		m_MessageList.AddString (str_msg);
	}

	}

	if(demand_type == 1)
	{
	// activity location table

	strSQL = m_pDoc->ConstructSQL("6-DEMAND-3-COLUMN");

	//bool bNodeNonExistError = false;
	m_pDoc->m_ImportedDemandVector .clear ();


	if(strSQL.GetLength () > 0)
	{
		CRecordsetExt rsDemand(&m_pDoc->m_Database);
		rsDemand.Open(dbOpenDynaset, strSQL);

		while(!rsDemand.IsEOF())
		{
			int from_zone_id = rsDemand.GetLong(CString("from_zone_id"),bExist,false);
			if(!bExist) 
			{
				AfxMessageBox("Field from_zone_id cannot be found in the demand table.\n Please make sure your demand input is three column-based format.");
				return;
			}

			if(m_pDoc-> m_ZoneMap.find(from_zone_id)== m_pDoc->m_ZoneMap.end())
			{
				CString message;
				message.Format("from_zone_id %d at line %d in the demand table has not been defined.", from_zone_id, m_pDoc->m_ImportedDemandVector.size() );
				AfxMessageBox(message);
				break;
			}

			if(from_zone_id ==0)
				break;

			int to_zone_id = rsDemand.GetLong(CString("to_zone_id"),bExist,false);


			if(!bExist) 
			{
				AfxMessageBox("Field node_id cannot be found in the demand table.");
				return;
			}

			if(m_pDoc->m_ZoneMap.find(to_zone_id)== m_pDoc->m_ZoneMap.end())
			{
				CString message;
				message.Format("to_zone_id %d at line %d  in the demand table has not been defined.", to_zone_id,  m_pDoc->m_ImportedDemandVector.size()+1);
				AfxMessageBox(message);
				break;
			}

			double number_of_vehicles =  rsDemand.GetDouble(CString("number_of_vehicles"),bExist,false); 

			if(number_of_vehicles < -0.1)
			{
				CString message;
				message.Format("number_of_vehicles %f in the demand table is invalid.", number_of_vehicles);
				AfxMessageBox(message);
				break;
			}

			DTA_demand element;
			element.from_zone_id = from_zone_id;
			element.to_zone_id = to_zone_id;
			element.number_of_vehicles = number_of_vehicles;

			m_pDoc->m_ImportedDemandVector.push_back (element);
	
			rsDemand.MoveNext ();

		}
		rsDemand.Close();

		str_msg.Format ( "%d demand element records imported.", m_pDoc->m_ImportedDemandVector.size());
		m_MessageList.AddString (str_msg);
	}

	}

	return;  // not reading sensor data for now.

}



void CDlg_ImportNetwork::OnBnClickedImportNetworkOnly()
{
	OnBnClickedImport();
	m_bImportNetworkOnly = false;  //reset flag
}


void CDlg_ImportNetwork::OnBnClickedImportSensorData()
{
	
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

	static char BASED_CODE szFilter[] = "Excel File (*.xls)|*.xls||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);

	dlg.m_ofn.lpstrInitialDir = SampleExcelNetworkFile;

	if(dlg.DoModal() == IDOK)
	{
		UpdateData(true);

		m_Edit_Excel_File = dlg.GetPathName();

		UpdateData(false);
	}

	UpdateData(false);
}

void CDlg_ImportNetwork::OnBnClickedButtonViewSampleCsvFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	m_pDoc->m_ProjectFile.Format("%s%s", pMainFrame->m_CurrentDirectory,m_pDoc->m_SampleExcelNetworkFile);

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



BOOL CDlg_ImportNetwork::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
#endif

