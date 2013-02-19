// TLite.h : main header file for the TLite application
//
//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com), Jeff Taylor (jeffrey.taylor.d@gmail.com)

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

#include "stdafx.h"
#include "Geometry.h"
#include "CSVParser.h"
#include "TLite.h"
#include "MainFrm.h"
#include "Shellapi.h"
#include "Network.h"
#include "TLiteDoc.h"
#ifndef _WIN64
#include "Data-Interface//include//ogrsf_frmts.h"
#endif 
class SynchroLaneData
{
public:
	int UpNode,DestNode,Lanes,Speed,SatFlow,Volume,Grade;

	int Phase1, Phase2,Phase3,Phase4;
	int PermPhase1,PermPhase2,PermPhase3,PermPhase4;
	SynchroLaneData ()
	{
		UpNode = 0;
		DestNode = 0;
		Lanes = 0;
		Speed = 0;
		Phase1 = 0;
		Phase2 = 0;
		Phase3 = 0;
		Phase4 = 0;
		
		PermPhase1 = 0;
		PermPhase2 = 0;
		PermPhase3 = 0;
		PermPhase4 = 0;

		SatFlow = 0;
		Volume = 0;
		Grade = 0;

	}

};

void CTLiteDoc::OGDF_WriteGraph(CString FileName)
{
	std::ofstream GMLFile;
	int nextId = 0;

	GMLFile.open (FileName, ios::out);
	if (GMLFile.is_open())
	{
		GMLFile.width(15);
		GMLFile.precision(7) ;
		GMLFile.setf(ios::fixed);
	}
	else
	{
		return;
	}
	GMLFile << "Creator \"NeXTA::writeGML\"\n";
	GMLFile << "directed 1\n";

	GMLFile << "graph [\n";

	std::list<DTANode*>::iterator iNode;
	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		GMLFile << "node [\n";
		GMLFile << "id " << (*iNode)->m_NodeNumber << "\n";
		GMLFile << "label  " << "\"" << (*iNode)->m_Name.c_str () << "\"\n";
		GMLFile << "x  " << (*iNode)->pt.x << "\n";
		GMLFile << "y  " << (*iNode)->pt.y << "\n";
		GMLFile << "]\n"; // node
	}

	std::list<DTALink*>::iterator iLink;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		GMLFile << "edge [\n";
		GMLFile << "source " << (*iLink)->m_FromNodeNumber << "\n";
		GMLFile << "target " << (*iLink)->m_ToNodeNumber << "\n";
		GMLFile << "weight " << (*iLink)->m_Length << "\n";
		GMLFile << "]\n"; // edge

	}

	GMLFile << "]\n"; // graph
	GMLFile.close();

}
float ComputeCapacity(float capacity_in_pcphpl,int link_capacity_flag, float speed_limit_in_mph, int number_of_lanes)
{

	if(link_capacity_flag == 1)
		capacity_in_pcphpl = capacity_in_pcphpl/max(1,number_of_lanes);

	if(capacity_in_pcphpl == 2)  // link capacity flag == 2, use default
	{  // generate default capacity 
		if( speed_limit_in_mph <=30)
			return 600;
		if( speed_limit_in_mph <=50)
			return 900;
		else if (speed_limit_in_mph <=60)
			return 1200;
		else 
			return 1800;
	}
	//default 0;

	return capacity_in_pcphpl;
}
BOOL CTLiteDoc::OnOpenAMSDocument(CString FileName)
{
	CWaitCursor wait;
	#ifndef _WIN64

	CString warning_message;

	std::string file_name = CString2StdString(FileName);

	std::string length_unit,decimal_degrees,with_reverse_direction_field, offset_link, oneway_vs_twoway, lane_vs_link, r_number_of_lanes_field, link_type_field;

	
	CCSVParser parser;
	parser.GetValueBySectionKeyFieldName(file_name,"configuration","length_unit","value",length_unit);

	parser.GetValueBySectionKeyFieldName(file_name,"configuration","with_decimal_long_lat","value",decimal_degrees);

	parser.GetValueBySectionKeyFieldName(file_name,"link","r_number_of_lanes","value",r_number_of_lanes_field);
	parser.GetValueBySectionKeyFieldName(file_name,"configuration","number_of_lanes_oneway_vs_twoway","value",oneway_vs_twoway);
	parser.GetValueBySectionKeyFieldName(file_name,"configuration","lane_capacity_vs_link_capacity","value",lane_vs_link);
	parser.GetValueBySectionKeyFieldName(file_name,"link","link_type","value",link_type_field);


	parser.GetValueBySectionKeyFieldName(file_name,"final_output","offset_link","value",offset_link);



	float long_lat_unit = 1.0f;
	if(decimal_degrees=="no" || decimal_degrees=="NO")
		long_lat_unit = 0.00001f;

	bool bMileFlag = false;
	if(length_unit=="mile")
		bMileFlag = true;

   
	int direction_field_flag = 0;
	int reverse_direction_field_flag  = 0;
	
	if(r_number_of_lanes_field.size()>0)
		reverse_direction_field_flag = 1;


	int offset_link_flag = 1;

	if(offset_link == "no")
		offset_link_flag = 0;

	int link_capacity_flag = 1;
	int number_of_lanes_for_two_way_links_flag = 0;

	if(oneway_vs_twoway == "twoway")
		number_of_lanes_for_two_way_links_flag = 1;

	if(lane_vs_link == "link")
		link_capacity_flag = 1;

	std::string centroid_file_name,connector_file_name;
	parser.GetValueBySectionKeyFieldName(file_name,"file_name","centroid","value",centroid_file_name);
	parser.GetValueBySectionKeyFieldName(file_name,"file_name","connector","value",connector_file_name);

	int use_optional_centroid_layer = 0;
	if(centroid_file_name.size()>0)
		use_optional_centroid_layer = 1;

	int use_optional_connector_layer = 0;
	if(connector_file_name.size()>0)
		use_optional_connector_layer = 1;


	// ************************************/
	// 1: node table

	// ************************************/



	string node_table_file_name;
	parser.GetValueBySectionKeyFieldName(file_name,"file_name","node","value",node_table_file_name);

	string node_name;
	parser.GetValueBySectionKeyFieldName(file_name,"node","name","value",node_name);

	string node_node_id;
	parser.GetValueBySectionKeyFieldName(file_name,"node","node_id","value",node_node_id);
	string node_control_type;
	parser.GetValueBySectionKeyFieldName(file_name,"node","control_type","value",node_control_type);


	string node_TAZ_name;
	parser.GetValueBySectionKeyFieldName(file_name,"node","TAZ","value",node_TAZ_name);


	//	; Control type 0 = unknown, 1 = uncontrolled, 2 = two-way stop, 6 = two-way yield, 3 = signalized, 4 = all-way stop, 5 = roundabout

	CString message_str;
	OGRRegisterAll();

	OGRDataSource       *poDS;

	CString node_shape_file_name;
	node_shape_file_name = m_ProjectDirectory + node_table_file_name.c_str ();

	poDS = OGRSFDriverRegistrar::Open(node_shape_file_name, FALSE );
	if( poDS == NULL )
	{

		CString msg;
		msg.Format("Open node shape file %s failed.",node_shape_file_name);
		AfxMessageBox(msg);
		return false;
	}

	int node_zone_mapping_count = 0;
	int poLayers = ((OGRDataSource*)poDS)->GetLayerCount() ;
	for (int j=0; j < poLayers; j++) 
	{

		OGRLayer  *poLayer;

		poLayer = ((OGRDataSource*)poDS)->GetLayer(j);	

		if(poLayer == NULL)
		{
			message_str.Format("Open layer %d failed", j+1);
			AfxMessageBox(message_str);

			return false;			
		}

		OGRFeature *poFeature;

		int feature_count = 0;

		poLayer->ResetReading();


		int i = 0;

		m_AMSLogFile << "1: node block---" << endl;
		m_AMSLogFile << "node id,control_type,TAZ,name,x,y" << endl;

		while( (poFeature = poLayer->GetNextFeature()) != NULL )
		{
			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			double x = 0;
			double y = 0;


			// node id
			int id = poFeature->GetFieldAsInteger(node_node_id.c_str ());

			if(id == 52508)
			{
			TRACE("");
			}
			int control_type = 0;

			if(node_control_type.size()>0 )
				control_type = poFeature->GetFieldAsInteger(node_control_type.c_str ());

			int TAZ= poFeature->GetFieldAsInteger(node_TAZ_name.c_str ());

			CString str_name = poFeature->GetFieldAsString(node_name.c_str ());

			m_AMSLogFile << id << "," << control_type << "," << TAZ << "," <<  str_name << ",";

			OGRGeometry *poGeometry;

			poGeometry = poFeature->GetGeometryRef();
			if( poGeometry != NULL )
			{
				if(wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
				{
					OGRPoint *poPoint = (OGRPoint *) poGeometry;

					x =   poPoint->getX()*long_lat_unit;
					y  = poPoint->getY()*long_lat_unit;
					m_AMSLogFile << x << "," << y << ",";

				}

			}

			DTANode* pNode = new DTANode;
			pNode->pt.x = x;
			pNode->pt.y = y;

			pNode->m_Name  = str_name;
			pNode->m_NodeNumber = id;
			pNode->m_NodeID = i;
			pNode->m_ZoneID = TAZ;
			pNode->m_ControlType = control_type;


			if(id==1067)
			{
				TRACE("");
			}
			if(TAZ>=1)
			{
				pNode->m_bZoneActivityLocationFlag = true;
				m_ZoneMap [TAZ].m_ZoneID = TAZ;

				DTAActivityLocation element;
				element.ZoneID  = TAZ;
				element.NodeNumber = id;
				m_ZoneMap [TAZ].m_ActivityLocationVector .push_back (element );

				if(m_ODSize < TAZ)
					m_ODSize = TAZ;

				node_zone_mapping_count ++;

			}


			m_NodeSet.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeIDtoNumberMap[i] = id;
			if(id == 54170)
			{
				TRACE("");
			}
			m_NodeNumbertoIDMap[id] = i;
			i++;

			m_AMSLogFile  << endl;
		}

		m_AMSLogFile << "imported " << i << " nodes. " << endl; 
		m_AMSLogFile << "imported " << m_ZoneMap.size() << " zones from the node layer. " << endl; 
		m_NodeDataLoadingStatus.Format ("%d nodes are loaded from file %s.",m_NodeSet.size(),node_shape_file_name);
		m_ZoneDataLoadingStatus.Format ("%d node-zone mapping entries are loaded from file %s.",node_zone_mapping_count,node_shape_file_name);

		// to do: # of nodes: control: two-way stop signs....
	}

	OGRDataSource::DestroyDataSource( poDS );

	// 1.2 optional zone_centroid_conversion

	if(use_optional_centroid_layer)
	{

	m_AMSLogFile << "read optional centroid layer from file " << centroid_file_name << endl; 
	parser.GetValueBySectionKeyFieldName(file_name,"centroid","name","value",node_name);
	parser.GetValueBySectionKeyFieldName(file_name,"centroid","node_id","value",node_node_id);
	parser.GetValueBySectionKeyFieldName(file_name,"centroid","TAZ","value",node_TAZ_name);

	//	; Control type 0 = unknown, 1 = uncontrolled, 2 = two-way stop, 6 = two-way yield, 3 = signalized, 4 = all-way stop, 5 = roundabout

	CString message_str;

	OGRDataSource       *poDS;

	CString centroid_shape_file_name;
	centroid_shape_file_name = m_ProjectDirectory + centroid_file_name.c_str ();

	poDS = OGRSFDriverRegistrar::Open(centroid_shape_file_name, FALSE );
	if( poDS == NULL )
	{
		CString msg;
		msg.Format("Open zone centroid shape file %s failed.",centroid_shape_file_name);
		AfxMessageBox(msg);

		return false;
	}

	int node_zone_mapping_count = 0;
	int poLayers = ((OGRDataSource*)poDS)->GetLayerCount() ;
	for (int j=0; j < poLayers; j++) 
	{

		OGRLayer  *poLayer;

		poLayer = ((OGRDataSource*)poDS)->GetLayer(j);	

		if(poLayer == NULL)
		{
			message_str.Format("Open layer %d failed", j+1);
			AfxMessageBox(message_str);

			return false;			
		}

		OGRFeature *poFeature;

		int feature_count = 0;

		poLayer->ResetReading();


		int i = 0;

		m_AMSLogFile << "1: zone centroid block---" << endl;
		m_AMSLogFile << "node id,TAZ,name,x,y" << endl;

		while( (poFeature = poLayer->GetNextFeature()) != NULL )
		{
			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			double x = 0;
			double y = 0;


			// node id
			int id = poFeature->GetFieldAsInteger(node_node_id.c_str ());

			int TAZ= poFeature->GetFieldAsInteger(node_TAZ_name.c_str ());

			if(TAZ == 0)  // if TAZ value is zero, then use node id as TAZ
				TAZ = id; 


			CString str_name = poFeature->GetFieldAsString(node_name.c_str ());

			m_AMSLogFile << id << "," << "," << TAZ << "," <<  str_name << ",";

			OGRGeometry *poGeometry;

			poGeometry = poFeature->GetGeometryRef();
			if( poGeometry != NULL )
			{
				if(wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
				{
					OGRPoint *poPoint = (OGRPoint *) poGeometry;

					x =   poPoint->getX()*long_lat_unit;
					y  = poPoint->getY()*long_lat_unit;
					m_AMSLogFile << x << "," << y << ",";

				}

			}



			DTANode* pNode = new DTANode;
			pNode->pt.x = x;
			pNode->pt.y = y;

			pNode->m_Name  = str_name;
			pNode->m_NodeNumber = id;
			pNode->m_NodeID = i;
			pNode->m_ZoneID = TAZ;
			pNode->m_ControlType = m_ControlType_NoControl;


			if(TAZ>=1)
			{
				pNode->m_bZoneActivityLocationFlag = true;
				m_ZoneMap [TAZ].m_ZoneID = TAZ;

				DTAActivityLocation element;
				element.ZoneID  = TAZ;
				element.NodeNumber = id;
				m_ZoneMap [TAZ].m_ActivityLocationVector .push_back (element );

				if(m_ODSize < TAZ)
					m_ODSize = TAZ;

				node_zone_mapping_count ++;

			}


			if(m_NodeNumbertoIDMap.find(id) == m_NodeNumbertoIDMap.end())  // no duplicated nodes
			{
			m_NodeSet.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeIDtoNumberMap[i] = id;

			if(id == 54170)
			{
				TRACE("");
			}

			m_NodeNumbertoIDMap[id] = i;
			i++;
			}

			m_AMSLogFile  << endl;
		}

		m_AMSLogFile << "imported " << i << " nodes. " << endl; 
		m_AMSLogFile << "imported " << m_ZoneMap.size() << " zones from the node layer. " << endl; 
		m_NodeDataLoadingStatus.Format ("%d nodes are loaded from file %s.",m_NodeSet.size(),centroid_shape_file_name);
		m_ZoneDataLoadingStatus.Format ("%d node-zone mapping entries are loaded from file %s.",node_zone_mapping_count,centroid_shape_file_name);

		// to do: # of nodes: control: two-way stop signs....
	}

	OGRDataSource::DestroyDataSource( poDS );

	}


	// ************************************/
	// 2: link table

	// ************************************/
	bool read_link_layer = true;

	bool bTwoWayLinkFlag = false;

	float default_distance_sum=0;
	float length_sum = 0;

		m_OffsetInFeet = 2;
		string link_table_file_name;
		string connector_table_file_name;
		string from_node_id_name;
		string to_node_id_name;
		string to_link_id_name;
		string link_id_name;


		std::map<long, int> LinkID_FromNodeNumber_Map;
		std::map<long, int> LinkID_ToNodeNumber_Map;

		string link_name;
		string link_type_name;
		string mode_code_name;
		string direction_name;
		string length_name;
		string number_of_lanes_name;
		string capacity_in_vhc_per_hour_name;
		string speed_limit_in_mph_name;

		parser.GetValueBySectionKeyFieldName(file_name,"file_name","link","value",link_table_file_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","from_node_id","value",from_node_id_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","to_node_id","value",to_node_id_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","link_id","value",link_id_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","name","value",link_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","link_type","value",link_type_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","mode_code","value",mode_code_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","direction","value",direction_name);

		parser.GetValueBySectionKeyFieldName(file_name,"link","length","value",length_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","number_of_lanes","value",number_of_lanes_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","hourly_capacity","value",capacity_in_vhc_per_hour_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","speed_limit","value",speed_limit_in_mph_name);


		string r_number_of_lanes_name;
		string r_lane_capacity_in_vhc_per_hour_name;
		string r_speed_limit_in_mph_name;
		string r_link_type_name;
		if(reverse_direction_field_flag)
		{
		parser.GetValueBySectionKeyFieldName(file_name,"link","r_number_of_lanes","value",r_number_of_lanes_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","r_hourly_capacity","value",r_lane_capacity_in_vhc_per_hour_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","r_speed_limit","value",r_speed_limit_in_mph_name);
		parser.GetValueBySectionKeyFieldName(file_name,"link","r_link_type","value",r_link_type_name);
		}



		CString link_shape_file_name;
		link_shape_file_name = m_ProjectDirectory + link_table_file_name.c_str ();

		poDS = OGRSFDriverRegistrar::Open(link_shape_file_name, FALSE );
		if( poDS == NULL )
		{

		CString msg;
		msg.Format("Open link shape file %s failed.",link_shape_file_name);
		AfxMessageBox(msg);

		return false;
		}

		poLayers = ((OGRDataSource*)poDS)->GetLayerCount();


		for (int j=0; j < poLayers; j++) 
		{

			OGRLayer  *poLayer;

			poLayer = ((OGRDataSource*)poDS)->GetLayer(j);	

			if(poLayer == NULL)
			{
				AfxMessageBox("Open link layer failed");
				return false;			
			}

			OGRFeature *poFeature;
			int feature_count = 0;
			poLayer->ResetReading();
			int line_no = 1;



			m_AMSLogFile << endl << endl << "2: link block---" << endl;
			m_AMSLogFile << "from_node_id,to_name_id,link_id,name,type,direction,length,number_of_lanes,speed_limit,capacity,";

			if(reverse_direction_field_flag)
			{
				m_AMSLogFile << "r_number_of_lanes,r_speed_limit,r_capacity," << endl;
			}
			else
			{
				m_AMSLogFile << " " << endl;
			}


			while( (poFeature = poLayer->GetNextFeature()) != NULL )
			{
				OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
				int from_node_id = poFeature->GetFieldAsInteger(from_node_id_name.c_str ());
				int to_node_id = poFeature->GetFieldAsInteger(to_node_id_name.c_str ());

				if(from_node_id ==  58014 && to_node_id ==  60718)
					TRACE("");

				int link_code_start = 1;
				int link_code_end = 1;


				if(from_node_id == 613)
					TRACE("");

				long link_id =  poFeature->GetFieldAsInteger(link_id_name.c_str ());

				LinkID_FromNodeNumber_Map[link_id] = from_node_id;
				LinkID_ToNodeNumber_Map[link_id] = to_node_id;

				CString name =  poFeature->GetFieldAsString(link_name.c_str ());
				int type = 0;
//				if(link_type_name.size() >=1)
					type = poFeature->GetFieldAsInteger(link_type_name.c_str ());

				CString mode_code = poFeature->GetFieldAsString(mode_code_name.c_str ());


				float speed_limit_in_mph= poFeature->GetFieldAsDouble(speed_limit_in_mph_name.c_str ());


				int direction = 1;
				if(direction_name.size()>0) 
					direction = poFeature->GetFieldAsInteger(direction_name.c_str ());
				else
				{
						// no direction field, we try to guess the link types
						if(reverse_direction_field_flag==1)
						{
	
						int type = poFeature->GetFieldAsInteger(link_type_name.c_str ());
						int r_link_type= poFeature->GetFieldAsInteger(r_link_type_name.c_str ());

						if(type >=1 && r_link_type>=1)
							direction = 2;

						if(type ==0 && r_link_type>=1)
							direction = -1;  // reverse

						if(type >=1 && r_link_type==0)
							direction = 1;  //forward

						}
				
				}




				int number_of_lanes = poFeature->GetFieldAsInteger(number_of_lanes_name.c_str ());

				if(direction_field_flag == 1 && (direction==0 || direction==2) && number_of_lanes_for_two_way_links_flag ==1 )
				{
					number_of_lanes = number_of_lanes/2;
				}



				if(type== 0 )  // no type information available
				{

					if(link_type_field.size()>0)// if link type information is required, skip this link
					{
							CString str;
							str.Format("link type (%s) for %d ->%d in the link shape file does not have valid values (which should be >=1).\n",link_type_name.c_str (), from_node_id, to_node_id);

								if(warning_message.GetLength ()<1000)  // to avoid too many error messages
							{
								warning_message+= str;

							}

							m_AMSLogFile << str;

						if(reverse_direction_field_flag)  // with reserved direction field
						{
							int r_link_type= poFeature->GetFieldAsInteger(r_link_type_name.c_str ());

							// if there is a reverse link, skip the following step only if r_link_type = 0
							if(r_link_type ==0)
								continue;
							else
							{
								// continue to the following steps for reverse link
								link_code_start = 2; link_code_end = 2;

							
							}
						}else // no reserve direction; skip the following steps
							continue;

					
					}
					else
					{// check speed limit to determine type

					if(speed_limit_in_mph>=55)
						type = 1; // default freeway
					else
						type = 4; // default arterial street

					if(number_of_lanes>=7)
						type = 10; // default connectors;

					int node_id  =0 ;
					
					if(m_NodeNumbertoIDMap.find (from_node_id) != m_NodeNumbertoIDMap.end())
					{
						node_id =  m_NodeNumbertoIDMap[from_node_id];
						DTANode* pFromNode = m_NodeIDMap[node_id];
						if(pFromNode->m_ZoneID >=1)
						{
						  // from node is a zone centroid, then the link is a connector
							type = 10;

						}


					}
					}
				
				}


				float capacity_in_pcphpl= poFeature->GetFieldAsDouble(capacity_in_vhc_per_hour_name.c_str ());

				capacity_in_pcphpl = ComputeCapacity(capacity_in_pcphpl,link_capacity_flag, speed_limit_in_mph,number_of_lanes);

				// if link_capacity_flag == 0, we give a default value
				int r_number_of_lanes =0; 
				int r_link_type = 0; 
				float r_speed_limit_in_mph = 0; 
				float r_capacity_in_pcphpl=0; 

				if(reverse_direction_field_flag)  // with reserved direction field
				{
					r_number_of_lanes = poFeature->GetFieldAsInteger(r_number_of_lanes_name.c_str());
					if(direction_field_flag == 1 && (direction==0 || direction==2) && number_of_lanes_for_two_way_links_flag ==1)
					{
						number_of_lanes = r_number_of_lanes/2;
					}

					r_speed_limit_in_mph= poFeature->GetFieldAsDouble(r_speed_limit_in_mph_name.c_str ());
					r_capacity_in_pcphpl= poFeature->GetFieldAsDouble(r_lane_capacity_in_vhc_per_hour_name.c_str ());
					r_capacity_in_pcphpl = ComputeCapacity(r_capacity_in_pcphpl,link_capacity_flag, r_speed_limit_in_mph,r_number_of_lanes);
					r_link_type= poFeature->GetFieldAsInteger(r_link_type_name.c_str ());

						if(m_LinkTypeMap[type ].IsConnector () && r_link_type ==0) // forward link is connector, r_link_type is not defined 
						{
						  r_link_type = type; //reset r_link_type by type
						}

					

				}else
				{  // no reserved link fields
					r_number_of_lanes = number_of_lanes;
					r_speed_limit_in_mph = speed_limit_in_mph;
					r_capacity_in_pcphpl= capacity_in_pcphpl;
					r_link_type = 0;

						if(m_LinkTypeMap[type ].IsConnector ()) // forward link is connector, r_link_type is not defined 
						{
						  r_link_type = type; //reset r_link_type by type
						}



				}

				// To Do Error checking, Numbera of lanes... 

				if(reverse_direction_field_flag)
				{
					m_AMSLogFile << r_number_of_lanes << "," << r_speed_limit_in_mph << ","  << r_capacity_in_pcphpl << ",";

				}


				float grade = 0;
				float AADT_conversion_factor = 0.1;
				float k_jam, wave_speed_in_mph;
				if(m_LinkTypeMap[type].IsFreeway ())  // freeway link
				{
					k_jam = 220;
				}else
				{
					k_jam = 120;
				}

				wave_speed_in_mph = 12;


				int m_SimulationHorizon = 1;

				if (direction == -1) // reversed
				{
					link_code_start = 2; link_code_end = 2;
				}


				if ((direction == 0 || direction ==2)&& (r_link_type >=1 )) // two-directional link and r_link_type is positive
				{
					link_code_start = 1; link_code_end = 2;
					bTwoWayLinkFlag = true;
				}

				OGRGeometry *poGeometry;
				std::vector<CCoordinate> CoordinateVector;

				bool bSkipShapePoints = false;
				if(bSkipShapePoints)
				{
				// no geometry information
				CCoordinate cc_from, cc_to; 
				cc_from.X = m_NodeIDMap[m_NodeNumbertoIDMap[from_node_id]]->pt.x;
				cc_from.Y = m_NodeIDMap[m_NodeNumbertoIDMap[from_node_id]]->pt.y;

				cc_to.X = m_NodeIDMap[m_NodeNumbertoIDMap[to_node_id]]->pt.x;
				cc_to.Y = m_NodeIDMap[m_NodeNumbertoIDMap[to_node_id]]->pt.y;

				CoordinateVector.push_back(cc_from);
				CoordinateVector.push_back(cc_to);
 
				}else
				{


				poGeometry = poFeature->GetGeometryRef();
				if( poGeometry != NULL )
				{	if(wkbFlatten(poGeometry->getGeometryType()) == wkbLineString )
				{
					OGRLineString *poLine = (OGRLineString *) poGeometry;

					int step = 1;

						int Number_of_Shape_Points = poLine->getNumPoints();

						m_AMSLogFile << "[" << 	Number_of_Shape_Points << "]" ;
						m_AMSLogFile << "{" ;

					if(Number_of_Shape_Points>= 20)  //
						step = (int)(Number_of_Shape_Points)/10;  // avoid samping from the last point
					
					int si;
					int si_with_data = 0;
					for( si = 0; si< Number_of_Shape_Points; si+=step)
					{

						CCoordinate pt;
						pt.X   =  poLine->getX(si)*long_lat_unit;
						pt.Y =  poLine->getY(si)*long_lat_unit;
						CoordinateVector.push_back(pt);
						si_with_data = si;

						m_AMSLogFile << pt.X << ";" << pt.Y << " ";

					}

						m_AMSLogFile << "%" << 	si << "%" ;

					if(si_with_data!= Number_of_Shape_Points-1)  // not include the last point
					{
						CCoordinate pt;
						pt.X   =  poLine->getX(Number_of_Shape_Points-1)*long_lat_unit;
						pt.Y =  poLine->getY(Number_of_Shape_Points-1)*long_lat_unit;
						CoordinateVector.push_back(pt);
						m_AMSLogFile << pt.X << ";" << pt.Y << " ";
					
					}

					m_AMSLogFile << "}" ;

				}
				}

				}

				float length = 0;
				
				if(length_name.size() > 0)
					length = poFeature->GetFieldAsDouble(length_name.c_str ());
				else
				{
					GDPoint pt1; 
					pt1.x = CoordinateVector[0].X;
					pt1.x = CoordinateVector[0].Y;

					GDPoint pt2; 
					pt2.x = CoordinateVector[CoordinateVector.size()-1].X;
					pt2.x = CoordinateVector[CoordinateVector.size()-1].Y;

					length =  g_CalculateP2PDistanceInMileFromLatitudeLongitude(pt1, pt2);
				}

				m_AMSLogFile << from_node_id << "," << to_node_id << "," << link_id << "," << name << "," << type << "," << direction << ",";
				m_AMSLogFile << length << "," << number_of_lanes << "," << speed_limit_in_mph << ","  << capacity_in_pcphpl << ",";



				if(from_node_id==0 && to_node_id ==0)  // test twice here for from and to nodes
				{
					AfxMessageBox("Invalid data: from_node_id==0 && to_node_id ==0 in the link table.");
					return false;
				}

				
				DTALink* pExistingLink =  FindLinkWithNodeIDs(m_NodeNumbertoIDMap[from_node_id],m_NodeNumbertoIDMap[to_node_id]);

				// error checking 
				CString str_msg;
				if(pExistingLink)
				{
				str_msg.Format ("Link %d-> %d at row %d is duplicated with the previous link at row %d. NEXTA will skip this link.\n", from_node_id,to_node_id, line_no, pExistingLink->input_line_no);
				continue;
				}

				if(from_node_id == to_node_id)
				{
				str_msg.Format ("Link %d-> %d has the same end nodes. NEXTA will skip this link.\n", from_node_id,to_node_id);
				continue;
				}

				if(length > 100)
				{
				str_msg.Format ("The length of link %d -> %d is longer than 100 miles, please ensure the unit of link length in the link sheet is mile.\n",from_node_id,to_node_id);
				}

				if(number_of_lanes ==0)
				{
				str_msg.Format ("Link %d -> %d has 0 lane.\n",from_node_id,to_node_id);
				}
				if(speed_limit_in_mph ==0)
				{
				str_msg.Format ("Link %d -> %d has a speed limit of 0.\n",from_node_id,to_node_id);
				}
				if(capacity_in_pcphpl<0)
				{
				str_msg.Format ("Link %d -> %d has a capcaity of 0.\n",from_node_id,to_node_id);
				}

				if(warning_message.GetLength ()<1000)  // to avoid too many error messages
				{
					warning_message+= str_msg;
				}


				line_no ++;
				for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
				{

					bool bNodeNonExistError = false;
					int m_SimulationHorizon = 1;
					DTALink* pLink = new DTALink(m_SimulationHorizon);
					pLink->m_LinkNo = m_LinkSet.size();
					pLink->m_Name  = name;
					pLink->m_OrgDir = direction;
					pLink->m_LinkID = link_id;

					CT2CA pszConvertedAnsiString (mode_code);
					// construct a std::string using the LPCSTR input
					std::string  strStd (pszConvertedAnsiString);

					pLink->m_Mode_code = strStd;

					if(link_code == 1)  //AB link
					{
						pLink->m_FromNodeNumber = from_node_id;

						pLink->m_ToNodeNumber = to_node_id;
						pLink->m_Direction  = 1;

						pLink->m_FromNodeID = m_NodeNumbertoIDMap[from_node_id];
						pLink->m_ToNodeID= m_NodeNumbertoIDMap[to_node_id];


						for(unsigned si = 0; si < CoordinateVector.size(); si++)
						{
							GDPoint	pt;
							pt.x = CoordinateVector[si].X;
							pt.y = CoordinateVector[si].Y;
							pLink->m_Original_ShapePoints .push_back (pt);
						pLink->m_ShapePoints .push_back (pt);
						}

					}

					if(link_code == 2)  //BA link
					{
						pLink->m_FromNodeNumber = to_node_id;
						pLink->m_ToNodeNumber = from_node_id;
						pLink->m_Direction  = 1;
						pLink->m_FromNodeID = m_NodeNumbertoIDMap[to_node_id];
						pLink->m_ToNodeID= m_NodeNumbertoIDMap[from_node_id];


						for(int si = CoordinateVector.size()-1; si >=0; si--)  // we need to put int here as si can be -1. 
						{
							GDPoint	pt;
							pt.x = CoordinateVector[si].X;
							pt.y = CoordinateVector[si].Y;
							pLink->m_Original_ShapePoints .push_back (pt);
							pLink->m_ShapePoints .push_back (pt);
						}
					}

					pLink->m_NumberOfLanes= number_of_lanes;
					pLink->m_SpeedLimit= speed_limit_in_mph;
					pLink->m_avg_simulated_speed = pLink->m_SpeedLimit;
					pLink->m_Length= length;  // minimum distance

					//if(length < 0.00001) // zero value in length field, we consider no length info.
					//{
					//	float distance_in_mile = g_CalculateP2PDistanceInMileFromLatitudeLongitude(pLink->m_ShapePoints[0], pLink->m_ShapePoints[pLink->m_ShapePoints.size()-1]);
					//	pLink->m_Length = distance_in_mile;
					//}

					pLink->m_FreeFlowTravelTime = pLink->m_Length / max(1,pLink->m_SpeedLimit) *60.0f;
					pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;

					pLink->m_MaximumServiceFlowRatePHPL= capacity_in_pcphpl;
					pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
					pLink->m_link_type= type;
					pLink->m_Grade = grade;


					if(link_code == 2)  //BA link
					{

						int R_number_of_lanes = r_number_of_lanes;

						float R_speed_limit_in_mph= r_speed_limit_in_mph;

						float R_lane_capacity_in_vhc_per_hour= r_capacity_in_pcphpl;
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
						pLink->m_link_type= max(type,r_link_type);  // commmented by Jeff Taylor. to be safe, so we take the max in case  type or r_link_type is 0



					}

					pLink->m_Kjam = k_jam;
					pLink->m_AADT_conversion_factor  = AADT_conversion_factor;
					pLink->m_Wave_speed_in_mph  = wave_speed_in_mph;

					m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumberOfLanes);

					pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
					pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;


					if(pLink->m_FromNodeNumber ==0 || pLink->m_ToNodeNumber ==0 )
					{
						//skip
						continue;
					}

					default_distance_sum+= pLink->DefaultDistance();
					length_sum += pLink ->m_Length;
					//			pLink->SetupMOE();
					pLink->input_line_no  = line_no;

					int index = m_LinkSet.size();

					m_LinkNoMap[index]  = pLink;
					m_LinkSet.push_back (pLink);
					m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
					m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;

					if(m_LinkTypeMap[type ].IsConnector ()) // adjacent node of connectors
					{ 
						// mark them as activity location 
						m_NodeIDMap[pLink->m_FromNodeID ]->m_bZoneActivityLocationFlag = true;					
						m_NodeIDMap[pLink->m_ToNodeID ]->m_bZoneActivityLocationFlag = true;					

						m_NodeIDMap[pLink->m_FromNodeID ]->m_ControlType = m_ControlType_NoControl;  // no control
						m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType = m_ControlType_NoControl;  // no control

					}


					m_NodeIDMap[pLink->m_FromNodeID ]->m_OutgoingLinkVector.push_back(index);
					m_NodeIDMap[pLink->m_ToNodeID ]->m_IncomingLinkVector.push_back(index);


					unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
					m_NodeIDtoLinkMap[LinkKey] = pLink;


					__int64  LinkKey2 = pLink-> m_FromNodeNumber* pLink->m_ToNodeNumber;
					m_NodeNumbertoLinkMap[LinkKey2] = pLink;



				}  // link code

				m_AMSLogFile  << endl;
			}  // feature

			m_AMSLogFile << "imported " << m_LinkSet.size() << " links. " << endl; 
			m_LinkDataLoadingStatus.Format ("%d links are loaded from file %s.",m_LinkSet.size(),link_shape_file_name);


		}  // layer
		// to do: # of nodes: control: two-way stop signs....

		OGRDataSource::DestroyDataSource( poDS );

		

				
		// determine control type for nodes
	

	int MinimumSpeedLimit4SignalControl  = g_GetPrivateProfileInt("control_type","minimum_speed_limit_for_signals",30,FileName);
	int MaximumSpeedLimit4SignalControl  = g_GetPrivateProfileInt("control_type","maximum_speed_limit_for_signals",60,FileName);

		std::list<DTALink*>::iterator iLink;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{

		DTALink* pLink = (*iLink);

					{  // reset default value
						if( m_LinkTypeMap[pLink->m_link_type ].IsArterial () == true &&
							pLink->m_SpeedLimit> MinimumSpeedLimit4SignalControl && pLink->m_SpeedLimit<= MaximumSpeedLimit4SignalControl && 
							m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType == 0 && 
							m_NodeIDMap[pLink->m_ToNodeID ]->m_IncomingLinkVector .size() >=3) 
						{ // speed range between 30 and 60, arterial streets, intersection has at least 3 legs
							m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType = m_ControlType_PretimedSignal;  // signal control
						}

						if(pLink->m_SpeedLimit<=30 && m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType == 0)
						{
							m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType = m_ControlType_4wayStopSign;  // signal control
						}

					}
		}



	if(use_optional_connector_layer == 1)
	{

		parser.GetValueBySectionKeyFieldName(file_name,"file_name","connector","value",connector_table_file_name);
	
		parser.GetValueBySectionKeyFieldName(file_name,"connector","zone_end","value",from_node_id_name);
		parser.GetValueBySectionKeyFieldName(file_name,"connector","node_end","value",to_node_id_name);
		parser.GetValueBySectionKeyFieldName(file_name,"connector","link_end","value",to_link_id_name);
		parser.GetValueBySectionKeyFieldName(file_name,"connector","length","value",length_name);
		parser.GetValueBySectionKeyFieldName(file_name,"connector","number_of_lanes","value",number_of_lanes_name);
		parser.GetValueBySectionKeyFieldName(file_name,"connector","hourly_capacity","value",capacity_in_vhc_per_hour_name);
		parser.GetValueBySectionKeyFieldName(file_name,"connector","speed_limit","value",speed_limit_in_mph_name);

		m_AMSLogFile << "starting converting connector from file " << connector_table_file_name;


		int default_number_of_lanes = 1;
		parser.GetValueBySectionKeyFieldName(file_name,"connector","default_speed_limit","value",default_number_of_lanes);

		int default_lane_capacity = 10000;
		int default_speed_limit = 100;
		int default_link_type = 99;
		parser.GetValueBySectionKeyFieldName(file_name,"connector","default_link_type","value",default_link_type);


		if(m_LinkTypeMap.find(default_link_type)==m_LinkTypeMap.end())
		{
		
			CString message;
			message.Format("default_link_type_for_connector=%d has not been defined in the input_link_type.csv file.\nThe NEXTA data conversion utility will add this link type definition automatically.");

			AfxMessageBox(message);

			m_LinkTypeMap[default_link_type].link_type = default_link_type;
			m_LinkTypeMap[default_link_type].type_code = 'c';
			m_LinkTypeMap[default_link_type].default_lane_capacity = 4000;


		}
		int direction =0;
		parser.GetValueBySectionKeyFieldName(file_name,"connector","direction","value",direction_name);

		CString connector_shape_file_name;
		connector_shape_file_name = m_ProjectDirectory + connector_table_file_name.c_str ();

		poDS = OGRSFDriverRegistrar::Open(connector_shape_file_name, FALSE );
		if( poDS == NULL )
		{
			CString msg;
			msg.Format ("Open connector shape file %s failed",link_shape_file_name);
			AfxMessageBox(msg);
			return false;
		}

		poLayers = ((OGRDataSource*)poDS)->GetLayerCount();

		for (int j=0; j < poLayers; j++) 
		{

			OGRLayer  *poLayer;

			poLayer = ((OGRDataSource*)poDS)->GetLayer(j);	

			if(poLayer == NULL)
			{
				CString error_message;

				error_message.Format("Open connector layer in the shape file %s failed",connector_shape_file_name);

				AfxMessageBox(error_message);
				return false;			
			}

			OGRFeature *poFeature;
			int feature_count = 0;
			poLayer->ResetReading();
			int i = 0;
			int line_no = 1;

			m_AMSLogFile << endl << endl << "4: " << connector_shape_file_name << " block---" << endl;
			m_AMSLogFile << "from_node_id,to_name_id,length,number_of_lanes,speed_limit,capacity," << endl;

			while( (poFeature = poLayer->GetNextFeature()) != NULL )
			{
				OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
				int from_node_id = poFeature->GetFieldAsInteger(from_node_id_name.c_str());
				int to_node_id = poFeature->GetFieldAsInteger(to_node_id_name.c_str());
				int to_link_id = poFeature->GetFieldAsInteger(to_link_id_name.c_str());

				if(direction_name.size()>0) 
				{
					direction = poFeature->GetFieldAsInteger(direction_name.c_str());

					CString direction_str = poFeature->GetFieldAsString(direction_name.c_str());

				}


				long link_id =  0;
				int type = default_link_type;  // find default connectors type.
				float length = 0.001;
				
				if(length_name.size() > 0)
					length = poFeature->GetFieldAsDouble(length_name.c_str ());

				int number_of_lanes = default_number_of_lanes;
				int capacity_in_pcphpl = default_lane_capacity;
				float speed_limit_in_mph = default_speed_limit;

//				m_AMSLogFile << from_node_id << "," << to_node_id << "," << length << "," << number_of_lanes << "," << speed_limit_in_mph << ","  << capacity_in_pcphpl << ",";

				float grade = 0;
				float AADT_conversion_factor = 0.1;
				float k_jam, wave_speed_in_mph;
				k_jam = 120;

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
					bTwoWayLinkFlag = true;
				}

				OGRGeometry *poGeometry;
				std::vector<CCoordinate> CoordinateVector;

				poGeometry = poFeature->GetGeometryRef();
				if( poGeometry != NULL )
				{	
					
					if(wkbFlatten(poGeometry->getGeometryType()) == wkbLineString )
				{
					OGRLineString *poLine = (OGRLineString *) poGeometry;

					m_AMSLogFile << "{" ;

					int step = 1;

						int Number_of_Shape_Points = poLine->getNumPoints();

						if(Number_of_Shape_Points > 100)
						{
						TRACE("\nNumber_of_Shape_Points = %d",Number_of_Shape_Points);
						}
					
					//if(Number_of_Shape_Points>=10)  //
					//	step = (int)(Number_of_Shape_Points/10);

					for(unsigned int si = 0; si< Number_of_Shape_Points; si+=step)  // up to Number_of_Shape_Points-1
					{
						CCoordinate pt;
						pt.X   =  poLine->getX(si)*long_lat_unit;
						pt.Y =  poLine->getY(si)*long_lat_unit;
						CoordinateVector.push_back(pt);
						m_AMSLogFile << pt.X << ";" << pt.Y << " ";

					}
						//CCoordinate pt;
						//pt.X   =  poLine->getX(Number_of_Shape_Points-1)*long_lat_unit;
						//pt.Y =  poLine->getY(Number_of_Shape_Points-1)*long_lat_unit;
						//CoordinateVector.push_back(pt);
						//m_AMSLogFile << pt.X << ";" << pt.Y << " ";

					m_AMSLogFile << "}" ;

				}

				
				if(from_node_id==0 && to_node_id ==0)  // test twice here for from and to nodes
				{
					AfxMessageBox("Invalid data: from_node_id==0 && to_node_id ==0 in the link table.");
					break;
				}


				line_no ++;
				for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
				{

					bool bNodeNonExistError = false;
					int m_SimulationHorizon = 1;
					DTALink* pLink = new DTALink(m_SimulationHorizon);
					pLink->m_LinkNo = m_LinkSet.size();
					pLink->m_Name  = "connector";
					pLink->m_OrgDir = direction;
					pLink->m_LinkID = link_id;


					if(to_link_id == 3522905)
					{
					TRACE("");
					}
					if(link_code == 1)  //AB link
					{
						pLink->m_FromNodeNumber = from_node_id;

						if(to_link_id>=1)
						{
							if(LinkID_ToNodeNumber_Map.find (to_link_id) != LinkID_ToNodeNumber_Map.end())
							{
								pLink->m_ToNodeNumber = max(LinkID_FromNodeNumber_Map[to_link_id], LinkID_ToNodeNumber_Map[to_link_id]);
								// from_node_id has been defined
								to_node_id = pLink->m_ToNodeNumber ;
							}
							else
							{
								CString error_message;

								error_message.Format("Invalid data: to_link_id %d in connector layer has not been defined in link layer.", to_link_id);		
								AfxMessageBox(error_message);
								return false;
							}
						}
						else
							pLink->m_ToNodeNumber = to_node_id;

						pLink->m_Direction  = 1;

						pLink->m_FromNodeID = m_NodeNumbertoIDMap[from_node_id];
						pLink->m_ToNodeID= m_NodeNumbertoIDMap[to_node_id];



						for(unsigned si = 0; si < CoordinateVector.size(); si++)
						{
							GDPoint	pt;
							pt.x = CoordinateVector[si].X;
							pt.y = CoordinateVector[si].Y;
							
							pLink->m_Original_ShapePoints .push_back (pt);
							pLink->m_ShapePoints .push_back (pt);
						}

						
					}

					if(link_code == 2)  //BA link
					{
						if(to_link_id>=1)
						{
							if(LinkID_ToNodeNumber_Map.find (to_link_id) != LinkID_ToNodeNumber_Map.end())
							{
								pLink->m_FromNodeNumber = max(LinkID_FromNodeNumber_Map[to_link_id],LinkID_ToNodeNumber_Map[to_link_id]);
								from_node_id = pLink->m_FromNodeNumber ;
							}
							else
							{
								CString error_message;

								error_message.Format("Invalid data: to_link_id %d in connector layer has not been defined in link layer.", to_link_id);		
								AfxMessageBox(error_message);
								return false;
							}
						}else
						{
						pLink->m_FromNodeNumber = to_node_id;
						}


						pLink->m_ToNodeNumber = from_node_id;
						pLink->m_Direction  = 1;
						pLink->m_FromNodeID = m_NodeNumbertoIDMap[to_node_id];
						pLink->m_ToNodeID= m_NodeNumbertoIDMap[from_node_id];


						for(int si = CoordinateVector.size()-1; si >=0; si--)  // we need to put int here as si can be -1. 
						{
							GDPoint	pt;
							pt.x = CoordinateVector[si].X;
							pt.y = CoordinateVector[si].Y;
							pLink->m_Original_ShapePoints .push_back (pt);
							pLink->m_ShapePoints .push_back (pt);
						}

					}

					pLink->m_NumberOfLanes= number_of_lanes;
					pLink->m_SpeedLimit= speed_limit_in_mph;
					pLink->m_avg_simulated_speed = pLink->m_SpeedLimit;
					pLink->m_Length= length;  // minimum distance

					pLink->m_FreeFlowTravelTime = pLink->m_Length / max(1,pLink->m_SpeedLimit) *60.0f;
					pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;

					pLink->m_MaximumServiceFlowRatePHPL= capacity_in_pcphpl;
					pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
					pLink->m_link_type= type;
					pLink->m_Grade = grade;


					if(link_code == 2)  //BA link
					{
						if(to_link_id>=1)
						{
							if(LinkID_ToNodeNumber_Map.find (to_link_id) != LinkID_ToNodeNumber_Map.end())
							{
								pLink->m_FromNodeNumber = max(LinkID_FromNodeNumber_Map[to_link_id],LinkID_ToNodeNumber_Map[to_link_id]);
								from_node_id = pLink->m_FromNodeNumber ;

							}
							else
							{
								CString error_message;

								error_message.Format("Invalid data: to_link_id %d in connector layer has not been defined in link layer.", to_link_id);		
								AfxMessageBox(error_message);
								return false;
							}
						}else
						{
						pLink->m_FromNodeNumber = to_node_id;
						}

						int R_number_of_lanes = number_of_lanes;

						float R_speed_limit_in_mph= speed_limit_in_mph;

						float R_lane_capacity_in_vhc_per_hour= capacity_in_pcphpl;
						float R_grade= grade;
						pLink->m_NumberOfLanes= number_of_lanes;
						pLink->m_SpeedLimit= speed_limit_in_mph;
						pLink->m_MaximumServiceFlowRatePHPL= R_lane_capacity_in_vhc_per_hour;
						pLink->m_Grade = R_grade;
						pLink->m_avg_simulated_speed = pLink->m_SpeedLimit;
						pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance
						pLink->m_FreeFlowTravelTime = pLink->m_Length / max(1,pLink->m_SpeedLimit) *60.0f;
						pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;
						pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
						pLink->m_link_type= type;

					}

					m_AMSLogFile << "Add connector " << from_node_id << " -> " << to_node_id << endl;

					pLink->m_Kjam = k_jam;
					pLink->m_AADT_conversion_factor  = AADT_conversion_factor;
					pLink->m_Wave_speed_in_mph  = wave_speed_in_mph;

					m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumberOfLanes);

					pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
					pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;


				pLink->input_line_no  = line_no;

					if(pLink->m_FromNodeNumber ==0 || pLink->m_ToNodeNumber ==0 )
					{
						//skip
						continue;
					}

					//if(to_link_id>=1) // generation link based
					//{
					//	// reset shape points;

					//	pLink->m_Original_ShapePoints.clear();
					//	pLink->m_ShapePoints.clear();
					//		
					//		GDPoint	pt;
					//		pt = pLink->m_FromPoint;

					//		pLink->m_Original_ShapePoints .push_back (pt);
					//		pLink->m_ShapePoints .push_back (pt);

					//		pt = pLink->m_ToPoint;

					//		pLink->m_Original_ShapePoints .push_back (pt);
					//		pLink->m_ShapePoints .push_back (pt);
					//}
					m_LinkNoMap[m_LinkSet.size()]  = pLink;


					m_LinkSet.push_back (pLink);
					m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
					m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;

						// mark them as activity location 
					m_NodeIDMap[pLink->m_FromNodeID ]->m_bZoneActivityLocationFlag = true;					
					m_NodeIDMap[pLink->m_FromNodeID ]->m_OutgoingLinkVector.push_back(i);


					unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
					m_NodeIDtoLinkMap[LinkKey] = pLink;


					__int64  LinkKey2 = pLink-> m_FromNodeNumber* pLink->m_ToNodeNumber;
					m_NodeNumbertoLinkMap[LinkKey2] = pLink;


					i++;

				}  // link code

				m_AMSLogFile  << endl;
			}  // feature
			}

			m_AMSLogFile << "imported " << i << " links. " << endl; 
			m_ConnectorDataLoadingStatus.Format ("%d connectors loaded from file %s.",m_LinkSet.size(),link_shape_file_name);


		}  // layer
		// to do: # of nodes: control: two-way stop signs....

		OGRDataSource::DestroyDataSource( poDS );


	}

	// post-processing link information

			Construct4DirectionMovementVector();
			AssignUniqueLinkIDForEachLink();

			if(bTwoWayLinkFlag == true)
				m_bLinkToBeShifted = true;

			GenerateOffsetLinkBand();

		m_UnitMile  = 1.0f;
		if(length_sum>0.000001f)
			m_UnitMile=  default_distance_sum /length_sum;

		m_UnitFeet = m_UnitMile/5280.0f;  

		if(offset_link_flag)
			OffsetLink();


	// ************************************/
	// 3: zone table
	// ************************************/
	string zone_table_file_name;
	string zone_id_name;
		parser.GetValueBySectionKeyFieldName(file_name,"file_name","zone","value",zone_table_file_name);
		parser.GetValueBySectionKeyFieldName(file_name,"zone","zone_id","value",zone_id_name);

	CString zone_shape_file_name;
	zone_shape_file_name = m_ProjectDirectory + zone_table_file_name.c_str();

	poDS = OGRSFDriverRegistrar::Open(zone_shape_file_name, FALSE );
	if( poDS == NULL )
	{  // zone layer file is not provided
		m_AMSLogFile << "Zone layer file is not provided. Use node file to generate zone layers." << endl; 
		m_ZoneDataLoadingStatus.Format ("Zone layer file is not provided. Use node file to generate zone layers.");

	}else  // read zone layer file
	{

		int poLayers = ((OGRDataSource*)poDS)->GetLayerCount() ;
		int line_no = 0;

		for (int j=0; j < poLayers; j++) 
		{

			OGRLayer  *poLayer;

			poLayer = ((OGRDataSource*)poDS)->GetLayer(j);	

			if(poLayer == NULL)
			{
				message_str.Format("Open layer %d failed", j+1);
				AfxMessageBox(message_str);

				return false;			
			}

			OGRFeature *poFeature;

			int feature_count = 0;

			poLayer->ResetReading();



			m_AMSLogFile << "3: zone block---" << endl;
			m_AMSLogFile << "zone id" << endl;

			int zone_record_count = 1;
			while( (poFeature = poLayer->GetNextFeature()) != NULL )
			{
				OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();

				// zone id
				int id = poFeature->GetFieldAsInteger(zone_id_name.c_str());

				if(id ==0)
				{
				
				CString str;
				str.Format ("Reading field %s = 0 at record No. %d for field %s. This field might not exist.",zone_id_name, zone_record_count,zone_id_name);
				
				AfxMessageBox(str);
				return false;
				}
				m_AMSLogFile << id << "," ;

				OGRGeometry *poGeometry;

				poGeometry = poFeature->GetGeometryRef();
				if( poGeometry != NULL )
				{

					if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon )
					{

						OGRPolygon* polygon = (OGRPolygon*)(poGeometry);

						OGRLinearRing *ring = polygon->getExteriorRing();
						OGRPoint point;

						m_AMSLogFile << "{";

						for(int i = 0; i < ring->getNumPoints(); i++)
						{
							ring->getPoint(i, &point);
							GDPoint pt;
							pt.x = point.getX()*long_lat_unit;
							pt.y =point.getY()*long_lat_unit;
							m_ZoneMap [id].m_ShapePoints.push_back (pt);

							m_AMSLogFile << pt.x << "," << pt.y << " ";

						}

						m_AMSLogFile << "}";
					} // wkbPolygon
					line_no++;
				} // poGeometry
			zone_record_count++;
			} // poFeature
		} // poLayers


		m_AMSLogFile << "imported " << line_no << " zone boundaries." << endl;
		m_ZoneDataLoadingStatus.Format ("%d zone boundary records are loaded from file %s.",line_no,zone_shape_file_name);

		if(warning_message.GetLength () >=1)
		{
			CString final_message; 
			final_message = "Warning messages:\n" + warning_message + "\nPlease check your original shape files and corresponding dbf files\n\n";
			AfxMessageBox(final_message);
		}


	}
#else  // 64 bit

	AfxMessageBox("NEXTA 64-bit version does not support shape file importing function. Please use NEXTA_32.exe ");

#endif
	return true;
}

bool  CTLiteDoc::ReadDemandMatrixFile(LPCTSTR lpszFileName,int demand_type)
{

	float total_demand = 0;
	long line_no = 0;
	CCSVParser parser;

	m_AMSLogFile << "Reading OD demand matrix, demand type: " << "," << demand_type << ", file: " << lpszFileName << endl;

	if (parser.OpenCSVFile(lpszFileName))
	{
		parser.ReadRecord();  // header
		while(parser.ReadRecord())
		{
			bool AMSLogOutput = true;

			int origin_zone, destination_zone;
			float number_of_vehicles ;

			int j;

			std::map<int, DTAZone>	:: const_iterator itr_o;
			std::map<int, DTAZone>	:: const_iterator itr_d;

			for(itr_o = m_ZoneMap.begin(); itr_o != m_ZoneMap.end(); itr_o++)  // for each origin zone
			{
				int origin_zone  = 0;
				if(parser.GetValueByFieldName("zone_id",origin_zone) == false)


					if(m_ZoneMap.find(origin_zone) == m_ZoneMap.end())
					{
						CString str;
						str.Format("origin zone id %d in OD demand matrix file %s has not been defined. Please check. ", origin_zone, lpszFileName);
						AfxMessageBox(str);
						return false;
					}

					for(itr_d = m_ZoneMap.begin(); itr_d != m_ZoneMap.end(); itr_d++) // for each destination zone
					{
						if(line_no >= 10)
							AMSLogOutput = false;

						int destination_zone = itr_d->first ;
						string dest_str; 

						char c_str[10]; 
						sprintf(c_str,"%d", destination_zone);
						string str;
						str = c_str;

						parser.GetValueByFieldName(str,number_of_vehicles);

						if(number_of_vehicles < -0.0001)
							number_of_vehicles = 0;


						if(origin_zone <= m_ODSize && destination_zone <= m_ODSize)
						{
							m_ZoneMap[origin_zone].m_ODDemandMatrix [destination_zone].SetValue (demand_type,number_of_vehicles);
							total_demand += number_of_vehicles;
						}

						if(AMSLogOutput)
							m_AMSLogFile << origin_zone << "," << destination_zone << "," ;
						if(number_of_vehicles < -0.0001)
							number_of_vehicles = 0;
					} // for each destination 

			} // for each origin

		}  // for each line
	

	}
	CString msg;
	msg.Format("%6.2f demand trips are importedfrom file %s",total_demand,lpszFileName);
	m_MessageStringVector.push_back (msg);

	return true;
}

bool CTLiteDoc::ReadTransCADDemandCSVFile(LPCTSTR lpszFileName)
{

	m_AMSLogFile << "Reading OD demand file (TransCAD format): " << ", file: " << lpszFileName << endl;

	float LengthinMB;
	FILE* pFile;
	fopen_s(&pFile,lpszFileName,"rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);
		LengthinMB= Length*1.0/1024/1024;
		if(LengthinMB > 100)
		{
			CString msg;
			msg.Format("The file %s is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load the input_demand file?",lpszFileName,LengthinMB);
			if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
				return true;
		}
	}

	float total_demand = 0;
	long line_no = 0;
	FILE* st;
	fopen_s(&st,lpszFileName, "r");
	if (st!=NULL)
	{
		bool AMSLogOutput = true;
		char  str_line[2000]; // input string
		int str_line_size;

		//g_read_a_line(st,str_line, str_line_size); //  skip the first line

		int origin_zone, destination_zone;
		float number_of_vehicles ;

		while( fscanf_s(st,"%d,%d,",&origin_zone,&destination_zone) >0)
		{
			// static traffic assignment, set the demand loading horizon to [0, 60 min]
			if(line_no >= 10)
				AMSLogOutput = false;

			if(AMSLogOutput)
				m_AMSLogFile << origin_zone << "," << destination_zone << "," ;

			for(unsigned int demand_type = 1; demand_type <= m_DemandTypeVector.size(); demand_type++)
			{
				number_of_vehicles = 0;
				if( fscanf_s(st,"%f,",&number_of_vehicles) == 0)
				{

					CString msg;
					msg.Format ("Demand type %d in line %d of file %s cannot be found. ",demand_type,line_no,lpszFileName);
					AfxMessageBox(msg,MB_OK|MB_ICONINFORMATION);
					return false;
				}
				if(AMSLogOutput)					
					m_AMSLogFile << number_of_vehicles << "," ;

				if(number_of_vehicles < -0.0001)
					number_of_vehicles = 0;

				if(origin_zone <= m_ODSize && destination_zone <= m_ODSize)
				{
					m_ZoneMap[origin_zone].m_ODDemandMatrix [destination_zone].SetValue (demand_type,number_of_vehicles);
					total_demand += number_of_vehicles;
				}
				else
				{
					CString msg;
					msg.Format ("Line %d in file %s has a zone number greater than the size of zones (%d). ",line_no,lpszFileName, m_ODSize);
					AfxMessageBox(msg,MB_OK|MB_ICONINFORMATION);
					return false;
				}
			}

			if(AMSLogOutput)					
				m_AMSLogFile << endl;

			line_no++;
		}

		m_AMSLogFile << "imported " << line_no << " demand elements." << endl;

		fclose(st);
		m_AMSLogFile << line_no << "demand entries are loaded from file " << lpszFileName << ". Total demand =  " << total_demand << endl;
		m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file %s. Total demand = %f",line_no,lpszFileName,total_demand);
		return true;
	}else
	{
		//		AfxMessageBox("Error: File input_demand.csv cannot be found or opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

	return true;
}

bool CTLiteDoc::ReadVISUMDemandCSVFile(LPCTSTR lpszFileName,int demand_type,int starting_time_in_min, int ending_time_in_min)
{

	m_AMSLogFile << "Reading OD demand file (VISUM format): " << ", file: " << lpszFileName << endl;

	float total_demand = 0;
	long line_no = 0;
	FILE* st;
	fopen_s(&st,lpszFileName, "r");
	if (st!=NULL)
	{
		bool AMSLogOutput = true;
		char  str_line[2000]; // input string
		int str_line_size;

		for(int skip = 0; skip < 8; skip++)
		g_read_a_line(st,str_line, str_line_size); //  skip the first 8 line

		int origin_zone, destination_zone;
		float number_of_vehicles ;

		while( fscanf_s(st,"%d,%d,%f",&origin_zone,&destination_zone,&number_of_vehicles) >0)
		{
			// static traffic assignment, set the demand loading horizon to [0, 60 min]
			if(line_no >= 10)
				AMSLogOutput = false;

			if(AMSLogOutput)
				m_AMSLogFile << origin_zone << "," << destination_zone << "," << number_of_vehicles << "," ;

				if(number_of_vehicles < -0.0001)
					number_of_vehicles = 0;

				if(origin_zone <= m_ODSize && destination_zone <= m_ODSize)
				{
					//m_ZoneMap[origin_zone].m_ODDemandMatrix [destination_zone].AddTimeDependentValue (demand_type,number_of_vehicles,starting_time_in_min, ending_time_in_min);
					total_demand += number_of_vehicles;
				}
				else
				{
					CString msg;
					msg.Format ("Line %d in file %s has a zone number greater than the size of zones (%d). ",line_no,lpszFileName, m_ODSize);
					AfxMessageBox(msg,MB_OK|MB_ICONINFORMATION);
					return false;
				}

			if(AMSLogOutput)					
				m_AMSLogFile << endl;

			line_no++;
		}

		m_AMSLogFile << "imported " << line_no << " demand elements." << endl;

		fclose(st);
		m_AMSLogFile << line_no << "demand entries are loaded from file " << lpszFileName << ". Total demand =  " << total_demand << endl;
		m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file %s. Total demand = %f",line_no,lpszFileName,total_demand);
		return true;
	}else
	{
		//		AfxMessageBox("Error: File input_demand.csv cannot be found or opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

	return true;
}

bool  CTLiteDoc::RunGravityModel()
{

	AfxMessageBox("Input: (1) production/attraction input_zone.csv;\n       (2) output_accessibility.csv\nOutput: AMS_OD_table.csv",MB_ICONINFORMATION);

	CCSVParser parser;

	if (parser.OpenCSVFile( CString2StdString(m_ProjectDirectory+"output_accessibility.csv")))
	{

		int from_TAZ, to_TAZ, accessibility_factor = 0;
		while(parser.ReadRecord())
		{

			if(parser.GetValueByFieldName("from_zone_id",from_TAZ) == false)
				break;

			if(parser.GetValueByFieldName("to_zone_id",to_TAZ) == false)
				break;

			parser.GetValueByFieldName("accessibility_factor",accessibility_factor);

			CString label;
			
			label.Format("%d,%d", from_TAZ   , to_TAZ);
			m_ODMatrixMap[label].Origin = from_TAZ;
			m_ODMatrixMap[label].Destination  = to_TAZ;
			m_ODMatrixMap[label].Accessibility_Factor = accessibility_factor;

		}
	}else
	{
	
	AfxMessageBox("File output_accessibility.csv cannot be opened.");
	}

				

	float total_demand = 0;
	long line_no = 0;

	std::map<int, DTAZone>	:: iterator itr_o;
	std::map<int, DTAZone>	:: iterator itr_d;

	// stage 1: determine initial production and attraction

	float total_production = 0;
	float total_attraction = 0;
	for(itr_o = m_ZoneMap.begin(); itr_o != m_ZoneMap.end(); itr_o++)  // for each origin
	{
		if(itr_o->second.m_bWithinSubarea)
		{
		for(unsigned int i = 0; i< itr_o->second.m_ActivityLocationVector.size(); i++)
		{
			DTANode* pNode = m_NodeIDMap[m_NodeNumbertoIDMap[itr_o->second.m_ActivityLocationVector[i].NodeNumber]];
				
 		total_production += itr_o->second .m_Production;
		total_attraction += itr_o->second .m_Attraction;

		}
		}
	}

	if(total_production <0.1  || total_attraction < 0.1)
	{
	
	AfxMessageBox("0 production/attraction is found.\nPlease first input production and attraction values for each zone in file input_zone.csv.");
	return false;
	}


	for(itr_o = m_ZoneMap.begin(); itr_o != m_ZoneMap.end(); itr_o++)  // for each origin
	{
		float total_relative_attraction = 0;

		if(itr_o->second.m_bWithinSubarea)
		{

		int destination_zone;

		for(itr_d = m_ZoneMap.begin(); itr_d != m_ZoneMap.end(); itr_d++)
		{
			if(itr_d->second.m_bWithinSubarea)
			{
				destination_zone = itr_d->first ;


					float accessibility_factor = 1;
					
						CString label;
				label.Format("%d,%d", itr_o->first   , itr_d->first);

					if(m_ODMatrixMap.find(label)  != m_ODMatrixMap.end())
					{
						accessibility_factor = m_ODMatrixMap[label].Accessibility_Factor;
					}

				total_relative_attraction += (itr_d)->second .m_Attraction * accessibility_factor;
			}

		}
		}

		if(total_relative_attraction <0.0001f)
			total_relative_attraction =  0.001f;

		float sub_total = 0 ;
		for(itr_d = m_ZoneMap.begin(); itr_d != m_ZoneMap.end(); itr_d++)  // for each destination
		{
			if(itr_d->second.m_bWithinSubarea)
			{
			int origin_zone  = itr_o->first ;
			int destination_zone = itr_d->first ;

					float accessibility_factor = 1;
					CString label;
				label.Format("%d,%d", itr_o->first   , itr_d->first);

					if(m_ODMatrixMap.find(label)  != m_ODMatrixMap.end())
					{
						accessibility_factor = m_ODMatrixMap[label].Accessibility_Factor;
					}


			float number_of_vehicles = itr_o->second .m_Production * (itr_d)->second .m_Attraction * accessibility_factor / max(0.00000001,total_relative_attraction);


			sub_total += number_of_vehicles;
			
			// m_ZoneMap[origin_zone].m_ODDemandMatrix [destination_zone].SetValue (demand_type,number_of_vehicles);

			m_ODMatrixMap[label].Origin = origin_zone;
			m_ODMatrixMap[label].Destination  = destination_zone;
			m_ODMatrixMap[label].TotalVehicleSize = number_of_vehicles; 
			total_demand += number_of_vehicles;

			if(number_of_vehicles < -0.0001)
				number_of_vehicles = 0;
			}
		}

	} // for each origin



	return true;
}



void  CTLiteDoc::ReadSynchroUniversalDataFiles()
{
	CWaitCursor wait;
	static char BASED_CODE szFilter[] = "Synchro UTDF LAYOUT File (LAYOUT.csv)|LAYOUT.csv||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		CString ProjectFile = dlg.GetPathName();
		CString Synchro_directory = ProjectFile.Left(ProjectFile.ReverseFind('\\') + 1);


		if(ReadSynchroLayoutFile(dlg.GetPathName()))
		{
			ReadSynchroLaneFile(Synchro_directory+"lanes.csv");  // we need lower cases of lanes.csv, rather than LANES.CSV
			OffsetLink();
			CalculateDrawingRectangle(true);
			m_bFitNetworkInitialized  = false;

		}
	
		UpdateAllViews(0);
	
	}

}

bool  CTLiteDoc::ReadSynchroLayoutFile_And_AddOutgoingLinks_For_ExternalNodes(LPCTSTR lpszFileName)
{

	string direction_vector[8] = {"NID","SID","EID","WID","NEID","NWID","SEID","SWID"};

	CCSVParser parser;
	parser.m_bSkipFirstLine  = true;  // skip the first line  
	if (parser.OpenCSVFile(lpszFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int node_id;
			string name;
			DTANode* pNode = 0;

			int node_type;
			double X;
			double Y;
			if(parser.GetValueByFieldName("INTID",node_id) == false)
				break;

			if(node_id==89)
			{
			TRACE("");
			
			}
			if(!parser.GetValueByFieldName("INTNAME",name))
				name = "";

			if(!parser.GetValueByFieldName("TYPE",node_type))
				node_type = 0;

			// use the X and Y as default values first
			bool bFieldX_Exist = parser.GetValueByFieldName("X",X);
			parser.GetValueByFieldName("Y",Y);

			//if(node_type != 1)  // not external node //XUESONG
			{

				for(int direction = 0; direction < 8; direction++)
				{
					int outgoing_node_number;
					if(parser.GetValueByFieldName(direction_vector[direction],outgoing_node_number))// value exits
					{
						// add a new link
						int from_node_id = m_NodeNumbertoIDMap[node_id];
						int to_node_id = m_NodeNumbertoIDMap[outgoing_node_number];

						if(m_NodeIDMap.find(to_node_id) != m_NodeIDMap.end())
						{
							//if(m_NodeIDMap[to_node_id]->m_ControlType == m_ControlType_ExternalNode) XUESONG
							{  // add new link if the outbound node is an external node
								AddNewLink(from_node_id, to_node_id,false, false);
								AddNewLink(to_node_id,from_node_id,false, false);
								TRACE("Add New Link = %d, %d\n", node_id, outgoing_node_number);
							}

						}

					
					}
			

				}
				
			}

		}
	}

	return true;
}


bool CTLiteDoc::ReadSynchroLayoutFile(LPCTSTR lpszFileName)
{
	m_NodeTypeMap[0] = "signalized intersection";
	m_NodeTypeMap[1] = "external node";
	m_NodeTypeMap[2] = "bend";
	m_NodeTypeMap[3] = "unsignalized";
	m_NodeTypeMap[4] = "roundabout";
	m_NodeTypeMap[5] = "";
	m_NodeTypeMap[6] = "";

	CCSVParser parser;
	parser.m_bSkipFirstLine  = true;  // skip the first line  
	if (parser.OpenCSVFile(lpszFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int node_id;
			string name;
			DTANode* pNode = 0;

			int node_type;
			double X;
			double Y;
			if(parser.GetValueByFieldName("INTID",node_id) == false)
				break;

			if(!parser.GetValueByFieldName("INTNAME",name))
				name = "";

			if(!parser.GetValueByFieldName("TYPE",node_type))
				node_type = 0;

			// use the X and Y as default values first
			bool bFieldX_Exist = parser.GetValueByFieldName("X",X);
			parser.GetValueByFieldName("Y",Y);

			if(m_NodeNumbertoIDMap.find(node_id) != m_NodeNumbertoIDMap.end())
			{
				CString error_message;
				error_message.Format ("Node %d in input_node.csv has been defined twice. Please check.", node_id);
				AfxMessageBox(error_message);
				return 0;
			}

			pNode = new DTANode;
			pNode->m_Name = name;

			if(node_type == 0) 
				pNode->m_ControlType = m_ControlType_PretimedSignal;
			if(node_type == 1) 
				pNode->m_ControlType = m_ControlType_ExternalNode;
			if(node_type == 2) 
				pNode->m_ControlType = m_ControlType_NoControl;


			pNode->pt.x = X/5280.0f;  // feet to mile
			pNode->pt.y = Y/5280.0f;  // feet to mile

			pNode->m_NodeNumber = node_id;
			pNode->m_NodeID = i;
			pNode->m_ZoneID = 0;
			m_NodeSet.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeIDtoNumberMap[i] = node_id;
			m_NodeNumbertoIDMap[node_id] = i;
			i++;

			TRACE("node = %d, X: %f, Y: %f\n", node_id, X, Y);

		}

		parser.CloseCSVFile ();

		// add links
		ReadSynchroLayoutFile_And_AddOutgoingLinks_For_ExternalNodes(lpszFileName);

		m_NodeDataLoadingStatus.Format ("%d nodes are loaded from file %s.",m_NodeSet.size(),lpszFileName);
		return true;
	}else
	{
		AfxMessageBox("Error: File LAYOUT.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}


}


bool CTLiteDoc::ReadSynchroLaneFile(LPCTSTR lpszFileName)
{
	bool bCreateNewNodeFlag = false;
	int LayerNo = 0;

	long i = 0;
	DTALink* pLink = 0;
	float default_distance_sum=0;
	float length_sum = 0;

	CString error_message;

	bool bTwoWayLinkFlag = false;
	CCSVParser parser;
	parser.m_bSkipFirstLine  = true;  // skip the first line  : Lane Group Data

	const int LaneColumnSize = 32;
//	string lane_Column_name_str[LaneColumnSize] = { "NBL2","NBL","NBT","NBR","NBR2","SBL2","SBL","SBT","SBR","SBR2","EBL2","EBL","EBT","EBR","EBR2","WBL2","WBL","WBT","WBR","WBR2","NEL","NET","NER","NWL","NWT","NWR","SEL","SET","SER","SWL","SWT","SWR"};
	string lane_Column_name_str[LaneColumnSize] = { "NBT","NBL2","NBL","NBR","NBR2",
		"SBT","SBL2","SBL","SBR","SBR2",
		"EBT","EBL2","EBL","EBR","EBR2",
		"WBT","WBL2","WBL","WBR","WBR2",
		"NET","NEL","NER",
		"NWT","NWL","NWR",
		"SET","SEL","SER",
		"SWT","SWL","SWR"};


	if (parser.OpenCSVFile(lpszFileName))
	{
		bool bNodeNonExistError = false;

		std::map<string,SynchroLaneData> LaneDataMap;

		bool NewLinkFlag=false;

		while(parser.ReadRecord())
		{
			int link_id = 0;
			int from_node_id = -1;
			int to_node_id = -1;
			int direction = 0;
			double length_in_mile;
			int number_of_lanes= 1;
			int speed_limit_in_mph= 60;
			double capacity_in_pcphpl= 2000;
			int type = 1;
			string name;
			float k_jam = 180;

			float wave_speed_in_mph = 12;
			string mode_code = "";

			float grade = 0;

			parser.GetValueByFieldName("RECORDNAME",name);
			int INTID;
			parser.GetValueByFieldName("INTID",INTID);

			int UpNode = 0;
			int DestNode = 0;
			int Lanes = 0;
			int Speed = 0;
			int Phase1= 0;
			int SatFlow = 0;
			int Volume = 0;

			int m;
			//			switch (name)
			if (name == "Up Node" || name == "UpNodeID")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					UpNode = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],UpNode);
					if(UpNode>0)
					{
						TRACE("UpNode -> CurNode:%d -> %d ",UpNode,INTID);
						LaneDataMap[lane_Column_name_str[m]].UpNode = UpNode;
					}
				}
			}

			if (name == "Dest Node"  || name == "DestNodeID")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int DestNode = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],DestNode);
					if(Lanes>0)
					{

						LaneDataMap[lane_Column_name_str[m]].DestNode  = DestNode;
					}
				}

			}
			if (name == "Lanes")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Lanes = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Lanes);
					if(Lanes>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Lanes  = Lanes;
					}
				}
			}

			if (name == "Speed")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Speed = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Speed);
					if(Speed>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Speed  = Speed;
					}
				}
			}

			if (name == "Phase1")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Phase1 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Phase1);
					if(Phase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Phase1  = Phase1;
					}
				}
			}

			if (name == "Phase2")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Phase2 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Phase2);
					if(Phase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Phase2  = Phase2;
					}
				}
			}

			if (name == "Phase3")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Phase3 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Phase3);
					if(Phase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Phase3  = Phase3;
					}
				}
			}

			if (name == "Phase4")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Phase4 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Phase4);
					if(Phase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Phase4  = Phase4;
					}
				}
			}


			// permitted phase
			if (name == "PermPhase1")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int PermPhase1 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],PermPhase1);
					if(PermPhase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].PermPhase1  = PermPhase1;
					}
				}
			}

			if (name == "PermPhase2")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int PermPhase2 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],PermPhase2);
					if(PermPhase2>0)
					{

						LaneDataMap[lane_Column_name_str[m]].PermPhase2  = PermPhase2;
					}
				}
			}

			if (name == "PermPhase3")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int PermPhase3 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],PermPhase3);
					if(PermPhase3>0)
					{

						LaneDataMap[lane_Column_name_str[m]].PermPhase3  = PermPhase3;
					}
				}
			}

			if (name == "PermPhase4")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int PermPhase4 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],PermPhase4);
					if(PermPhase4>0)
					{

						LaneDataMap[lane_Column_name_str[m]].PermPhase4  = PermPhase4;
					}
				}
			}


			if (name == "SatFlow")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int SatFlow = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],SatFlow);
					if(SatFlow>0)
					{

						LaneDataMap[lane_Column_name_str[m]].SatFlow  = SatFlow;
					}
				}
			}

			if (name == "Grade")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Grade = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Grade);
					if(SatFlow>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Grade  = Grade;
					}
				}
			}

			if (name == "TravelTime")  // we have finished reading all required data
			{
				//add links and set up phases;

				int max_phase_number = 0;

				int to_node_id  = INTID;

				// set control type
				for(m = 0; m< LaneColumnSize; m++)
				{

						if(LaneDataMap.find(lane_Column_name_str[m]) != LaneDataMap.end() )  // data exists
						{


							from_node_id  = LaneDataMap[lane_Column_name_str[m]].UpNode;
							int dest_node_id  = LaneDataMap[lane_Column_name_str[m]].DestNode ;

							if( max_phase_number < LaneDataMap[lane_Column_name_str[m]].Phase1 )
								max_phase_number = LaneDataMap[lane_Column_name_str[m]].Phase1;

							if( max_phase_number < LaneDataMap[lane_Column_name_str[m]].Phase2 )
								max_phase_number = LaneDataMap[lane_Column_name_str[m]].Phase2;

							if( max_phase_number < LaneDataMap[lane_Column_name_str[m]].Phase3 )
								max_phase_number = LaneDataMap[lane_Column_name_str[m]].Phase3;

							if( max_phase_number < LaneDataMap[lane_Column_name_str[m]].Phase4 )
								max_phase_number = LaneDataMap[lane_Column_name_str[m]].Phase4;

							if( max_phase_number < LaneDataMap[lane_Column_name_str[m]].PermPhase1 )
								max_phase_number = LaneDataMap[lane_Column_name_str[m]].PermPhase1;

							if( max_phase_number < LaneDataMap[lane_Column_name_str[m]].PermPhase2 )
								max_phase_number = LaneDataMap[lane_Column_name_str[m]].PermPhase2;

							if( max_phase_number < LaneDataMap[lane_Column_name_str[m]].PermPhase3 )
								max_phase_number = LaneDataMap[lane_Column_name_str[m]].PermPhase3;

							if( max_phase_number < LaneDataMap[lane_Column_name_str[m]].PermPhase4 )
								max_phase_number = LaneDataMap[lane_Column_name_str[m]].PermPhase4;

							TRACE("\nmovement %s, from node %d to node %d",
								lane_Column_name_str[m].c_str (), from_node_id, to_node_id);

	

							if(from_node_id<=0 || to_node_id <= 0)
								continue;

							// add link
							DTALink* pExistingLink =  FindLinkWithNodeIDs(m_NodeNumbertoIDMap[from_node_id],m_NodeNumbertoIDMap[to_node_id]);

							if(pExistingLink)
							{
								//the approach has been processed.
								continue;
							}

							if(from_node_id == to_node_id)
							{
							continue;
							}

							if(pLink->m_FromNodeNumber ==0 || pLink->m_ToNodeNumber ==0 )
							{
								//skip
								continue;
							}

							TRACE("\n add ->> movement %s, from node %d to node %d, with %d links",
								lane_Column_name_str[m].c_str (), from_node_id, to_node_id,  LaneDataMap[lane_Column_name_str[m]].Lanes );

							bool bToBeShifted = true;
							int number_of_lanes = LaneDataMap[lane_Column_name_str[m]].Lanes ;
							float speed_limit_in_mph = LaneDataMap[lane_Column_name_str[m]].Speed ;
							float capacity_in_pcphpl =  LaneDataMap[lane_Column_name_str[m]].SatFlow / max(1,number_of_lanes);  // convert per link flow rate to per lane flow rate

							int m_SimulationHorizon = 1;
							DTALink* pLink = new DTALink(m_SimulationHorizon);
							pLink->m_LayerNo = LayerNo;
							pLink->m_LinkNo = m_LinkSet.size();
							pLink->m_LinkID = m_LinkSet.size();


							pLink->m_FromNodeNumber = from_node_id;

							pLink->m_ToNodeNumber = to_node_id;
							pLink->m_Direction  = 1;

							if(pLink->m_FromNodeNumber == 12 && pLink->m_ToNodeNumber == 2)
							TRACE("");

							pLink->m_FromNodeID = m_NodeNumbertoIDMap[from_node_id];
							pLink->m_ToNodeID= m_NodeNumbertoIDMap[to_node_id];

							pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
							pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;
							float length_in_mile =  pLink->DefaultDistance();  // cooridnates have been changed to mile for unit

							pLink->m_bToBeShifted = bToBeShifted; 

							pLink->m_NumberOfLanes= number_of_lanes;
							pLink->m_SpeedLimit= max(20,speed_limit_in_mph);  // minimum speed limit is 20 mph
							pLink->m_avg_simulated_speed = pLink->m_SpeedLimit;

							//				pLink->m_Length= max(length_in_mile, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance, special note: we do not consider the minimum constraint here, but a vehicle cannot travel longer then 0.1 seconds
							pLink->m_Length= length_in_mile;
							pLink->m_FreeFlowTravelTime = pLink->m_Length/pLink->m_SpeedLimit*60.0f;  // convert from hour to min
							pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;

							pLink->m_MaximumServiceFlowRatePHPL= capacity_in_pcphpl;
							pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
							pLink->m_link_type= 4;  //arterial
							pLink->m_Grade = LaneDataMap[lane_Column_name_str[m]].Grade;

							int k_jam = 180;
							int wave_speed_in_mph = 12;

							pLink->m_Kjam = k_jam;
							pLink->m_Wave_speed_in_mph  = wave_speed_in_mph;

							m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
							m_NodeIDMap[pLink->m_FromNodeID ]->m_OutgoingLinkVector.push_back(pLink->m_LinkNo);
							m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;

							unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);

							m_NodeIDtoLinkMap[LinkKey] = pLink;

							__int64  LinkKey2 = GetLink64Key(pLink-> m_FromNodeNumber,pLink->m_ToNodeNumber);
							m_NodeNumbertoLinkMap[LinkKey2] = pLink;

							m_LinkNotoLinkMap[m_LinkSet.size()] = pLink;

							m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumberOfLanes);


							default_distance_sum+= pLink->DefaultDistance();
							length_sum += pLink ->m_Length;
							//			pLink->SetupMOE();

								GDPoint	pt;
								pt.x = pLink->m_FromPoint.x;
								pt.y = pLink->m_FromPoint.y;
							pLink->m_Original_ShapePoints .push_back (pt);
								pLink->m_ShapePoints .push_back (pt);


								pt.x = pLink->m_ToPoint.x;
								pt.y = pLink->m_ToPoint.y;
							pLink->m_Original_ShapePoints .push_back (pt);
								pLink->m_ShapePoints .push_back (pt);

							TRACE("\nAdd link no.%d,  %d -> %d",i,pLink->m_FromNodeNumber, pLink->m_ToNodeNumber );
							m_LinkNoMap[m_LinkSet.size()]  = pLink;
							m_LinkSet.push_back (pLink);
						}  // per major approach

					} // for each movement

					DTANode* pNode = m_NodeIDMap[m_NodeNumbertoIDMap[to_node_id]];	

					for(int p  = 1; p <= max_phase_number; p++)
					{
							DTANodePhase node_phase;

							pNode->m_PhaseVector.push_back(node_phase); // phase 1;
					}

					LaneDataMap.clear();  // clear data after adding a set of links
				}
		}
	}
		
		m_UnitMile  = 1.0f;
		if(length_sum>0.000001f)
			m_UnitMile=  default_distance_sum /length_sum;

		m_UnitFeet = m_UnitMile/5280.0f;  


		m_LinkDataLoadingStatus.Format ("%d links are loaded.",m_LinkSet.size());


	// add movement
	parser.CloseCSVFile ();

	if (parser.OpenCSVFile(lpszFileName))
	{
		bool bNodeNonExistError = false;

		std::map<string,SynchroLaneData> LaneDataMap;

		bool NewLinkFlag=false;

		while(parser.ReadRecord())
		{
			int link_id = 0;
			int from_node_id;
			int to_node_id;
			int direction = 0;
			double length_in_mile;
			int number_of_lanes= 1;
			int speed_limit_in_mph= 60;
			double capacity_in_pcphpl= 2000;
			int type = 1;
			string name;
			float k_jam = 180;

			float wave_speed_in_mph = 12;
			string mode_code = "";

			float grade = 0;

			parser.GetValueByFieldName("RECORDNAME",name);
			int INTID;
			parser.GetValueByFieldName("INTID",INTID);

			int UpNode = 0;
			int DestNode = 0;
			int Lanes = 0;
			int Speed = 0;
			int Phase1= 0;
			int SatFlow = 0;
			int Volume = 0;

			int m;
			//			switch (name)
			if (name == "Up Node")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					UpNode = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],UpNode);
					if(UpNode>0)
					{
						TRACE("UpNode -> CurNode:%d -> %d ",UpNode,INTID);
						LaneDataMap[lane_Column_name_str[m]].UpNode = UpNode;
					}
				}
			}

			if (name == "Dest Node")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int DestNode = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],DestNode);
					if(Lanes>0)
					{

						LaneDataMap[lane_Column_name_str[m]].DestNode  = DestNode;
					}
				}

			}
			if (name == "Lanes")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Lanes = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Lanes);
					if(Lanes>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Lanes  = Lanes;
					}
				}
			}

			if (name == "Speed")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Speed = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Speed);
					if(Speed>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Speed  = Speed;
					}
				}
			}

			if (name == "Phase1")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Phase1 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Phase1);
					if(Phase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Phase1  = Phase1;
					}
				}
			}

			if (name == "Phase2")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Phase2 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Phase2);
					if(Phase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Phase2  = Phase2;
					}
				}
			}

			if (name == "Phase3")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Phase3 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Phase3);
					if(Phase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Phase3  = Phase3;
					}
				}
			}

			if (name == "Phase4")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Phase4 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Phase4);
					if(Phase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Phase4  = Phase4;
					}
				}
			}


			// permitted phase
			if (name == "PermPhase1")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int PermPhase1 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],PermPhase1);
					if(PermPhase1>0)
					{

						LaneDataMap[lane_Column_name_str[m]].PermPhase1  = PermPhase1;
					}
				}
			}

			if (name == "PermPhase2")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int PermPhase2 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],PermPhase2);
					if(PermPhase2>0)
					{

						LaneDataMap[lane_Column_name_str[m]].PermPhase2  = PermPhase2;
					}
				}
			}

			if (name == "PermPhase3")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int PermPhase3 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],PermPhase3);
					if(PermPhase3>0)
					{

						LaneDataMap[lane_Column_name_str[m]].PermPhase3  = PermPhase3;
					}
				}
			}

			if (name == "PermPhase4")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int PermPhase4 = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],PermPhase4);
					if(PermPhase4>0)
					{

						LaneDataMap[lane_Column_name_str[m]].PermPhase4  = PermPhase4;
					}
				}
			}

			if (name == "SatFlow")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int SatFlow = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],SatFlow);
					if(SatFlow>0)
					{

						LaneDataMap[lane_Column_name_str[m]].SatFlow  = SatFlow;
					}
				}
			}

			if (name == "Grade")
			{
				for(m = 0; m< LaneColumnSize; m++)
				{
					int Grade = 0;
					parser.GetValueByFieldName(lane_Column_name_str[m],Grade);
					if(SatFlow>0)
					{

						LaneDataMap[lane_Column_name_str[m]].Grade  = Grade;
					}
				}
			}

			if (name == "TravelTime")  // we have finished reading all required data
			{
				//add movement and links
				for(m = 0; m< LaneColumnSize; m++)
				{

						if(LaneDataMap.find(lane_Column_name_str[m]) != LaneDataMap.end() )  // data exists
						{

							int from_node_id  = LaneDataMap[lane_Column_name_str[m]].UpNode;
							int to_node_id  = INTID;
							int dest_node_id  = LaneDataMap[lane_Column_name_str[m]].DestNode ;

							TRACE("\nmovement %s, from node %d to node %d -> %d",lane_Column_name_str[m].c_str (), from_node_id, to_node_id,dest_node_id);

						DTANodeMovement element;

					element.in_link_from_node_id = m_NodeNumbertoIDMap[from_node_id];		
					element.in_link_to_node_id = m_NodeNumbertoIDMap[to_node_id];						
					element.out_link_to_node_id = m_NodeNumbertoIDMap[dest_node_id];	

					DTALink* pIncomingLink =  FindLinkWithNodeIDs(m_NodeNumbertoIDMap[from_node_id],m_NodeNumbertoIDMap[to_node_id]);

					if(pIncomingLink)
						element.IncomingLinkID = pIncomingLink->m_LinkNo  ;

					DTALink* pOutcomingLink =  FindLinkWithNodeIDs(m_NodeNumbertoIDMap[to_node_id],m_NodeNumbertoIDMap[dest_node_id]);

					if(pOutcomingLink)
						element.OutgoingLinkID = pOutcomingLink->m_LinkNo ;


					GDPoint p1, p2, p3;
					p1  = m_NodeIDMap[element.in_link_from_node_id]->pt;
					p2  = m_NodeIDMap[element.in_link_to_node_id]->pt;
					p3  = m_NodeIDMap[element.out_link_to_node_id]->pt;

					element.movement_approach = g_Angle_to_Approach_4_direction(Find_P2P_Angle(p1,p2));
					element.movement_turn = Find_PPP_to_Turn(p1,p2,p3);

					// determine  movement type /direction here
					element.movement_dir = DTA_LANES_COLUME_init;

					switch (element.movement_approach)
					{
					case DTA_North:
						switch (element.movement_turn)
						{
						case DTA_LeftTurn: element.movement_dir = DTA_NBL; break;
						case DTA_Through: element.movement_dir = DTA_NBT; break;
						case DTA_RightTurn: element.movement_dir = DTA_NBR; break;
						}
						break;
					case DTA_East:

						switch (element.movement_turn)
						{
						case DTA_LeftTurn: element.movement_dir = DTA_EBL; break;
						case DTA_Through: element.movement_dir = DTA_EBT; break;
						case DTA_RightTurn: element.movement_dir = DTA_EBR; break;
						}
						break;
					case DTA_South:
						switch (element.movement_turn)
						{
						case DTA_LeftTurn: element.movement_dir = DTA_SBL; break;
						case DTA_Through: element.movement_dir = DTA_SBT; break;
						case DTA_RightTurn: element.movement_dir = DTA_SBR; break;
						}
						break;
					case DTA_West:

						switch (element.movement_turn)
						{
						case DTA_LeftTurn: element.movement_dir = DTA_WBL; break;
						case DTA_Through: element.movement_dir = DTA_WBT; break;
						case DTA_RightTurn: element.movement_dir = DTA_WBR; break;
						}
						break;
					}


					DTANode* pNode = m_NodeIDMap[m_NodeNumbertoIDMap[to_node_id]];	

					ASSERT(pNode!=NULL);

					int movement_index = pNode->m_MovementVector.size();
					
					// educated guess about the associatd phase, as a movement can be associated with multiple phases 
					element.phase_index = max(LaneDataMap[lane_Column_name_str[m]].Phase1,LaneDataMap[lane_Column_name_str[m]].PermPhase1);

					pNode->m_MovementVector.push_back(element);

					//record the movement index into the right phase index

					int PhaseIndex  = 0;
					
					PhaseIndex = LaneDataMap[lane_Column_name_str[m]].Phase1;
					if(PhaseIndex>=1)
						pNode->m_PhaseVector[PhaseIndex-1].movement_index_vector .push_back(movement_index);

					PhaseIndex = LaneDataMap[lane_Column_name_str[m]].Phase2;
					if(PhaseIndex>=1)
						pNode->m_PhaseVector[PhaseIndex-1].movement_index_vector .push_back(movement_index);

					PhaseIndex = LaneDataMap[lane_Column_name_str[m]].Phase3;
					if(PhaseIndex>=1)
						pNode->m_PhaseVector[PhaseIndex-1].movement_index_vector .push_back(movement_index);

					PhaseIndex = LaneDataMap[lane_Column_name_str[m]].Phase4;
					if(PhaseIndex>=1)
						pNode->m_PhaseVector[PhaseIndex-1].movement_index_vector .push_back(movement_index);
					
					////////////// permitted phase
					PhaseIndex = LaneDataMap[lane_Column_name_str[m]].PermPhase1;
					if(PhaseIndex>=1)
						pNode->m_PhaseVector[PhaseIndex-1].movement_index_vector .push_back(movement_index);

					PhaseIndex = LaneDataMap[lane_Column_name_str[m]].PermPhase2;
					if(PhaseIndex>=1)
						pNode->m_PhaseVector[PhaseIndex-1].movement_index_vector .push_back(movement_index);

					PhaseIndex = LaneDataMap[lane_Column_name_str[m]].PermPhase3;
					if(PhaseIndex>=1)
						pNode->m_PhaseVector[PhaseIndex-1].movement_index_vector .push_back(movement_index);

					PhaseIndex = LaneDataMap[lane_Column_name_str[m]].PermPhase4;
					if(PhaseIndex>=1)
						pNode->m_PhaseVector[PhaseIndex-1].movement_index_vector .push_back(movement_index);


					}  // per major approach



					} // for each movement
					LaneDataMap.clear();  // clear data after adding a set of links
				}

		}
	}

		//Construct4DirectionMovementVector();
//		GenerateOffsetLinkBand();
	return 1;
}


BOOL CTLiteDoc::ImportingTransportationPlanningDataSet(CString ProjectFileName, bool bNetworkOnly, bool bImportShapeFiles)
{
	if(m_ImportNetworkAlready)
	{
	AfxMessageBox("A data set has been imported. Please close the current data set to use the import function again.");
	}

	m_ImportNetworkAlready = true;

	CTime LoadingStartTime = CTime::GetCurrentTime();

	FILE* st = NULL;
	//	cout << "Reading file node.csv..."<< endl;

	CString directory;
	m_ProjectFile = ProjectFileName;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

	m_ProjectDirectory = directory;
	m_ProjectTitle = GetWorkspaceTitleName(ProjectFileName);
	SetTitle(m_ProjectTitle);

	// default data type definition files

	m_AMSLogFile.open ( m_ProjectDirectory + "AMS_data_conversion_log.csv", ios::out);
	if (m_AMSLogFile.is_open())
	{
		m_AMSLogFile.width(12);
		m_AMSLogFile.precision(3) ;
		m_AMSLogFile.setf(ios::fixed);
		m_AMSLogFile << "Start AMS reading..." << endl;
	}else
	{
		AfxMessageBox("File AMS_data_conversion_log.csv cannot be opened, and it might be locked by another program or the target data folder is read-only.");
		return false;
	}

	m_AMSLogFile << "Step 1: Read control type definition data" << endl;

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	CString DefaultDataFolder;
	DefaultDataFolder.Format ("%s\\default_data_folder\\",pMainFrame->m_CurrentDirectory);

	bool bNodeControlFile = false;
	bool bLinkTypeFile = false;
	bNodeControlFile = CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,m_ProjectDirectory,"input_node_control_type.csv");
	bLinkTypeFile = CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,m_ProjectDirectory,"input_link_type.csv");



	if(ReadNodeControlTypeCSVFile(directory+"input_node_control_type.csv") == false)
	{
		CString msg;
		msg.Format ("Please prepare node control type definition file input_link_type.csv and place it at folder %s.",directory);
			AfxMessageBox(msg);
		return false;
	}


	if(ReadLinkTypeCSVFile(directory+"input_link_type.csv") == false)
	{
		CString msg;
		msg.Format ("Please prepare link type definition file input_link_type.csv and place it at folder %s.",directory);
			AfxMessageBox(msg);
		return false;
	}

	if(bNodeControlFile || bNodeControlFile)
	{
		CString msg;
		msg.Format ("Please use Excel to review files input_node_control_type.csv and input_link_type.csv, as default files are used.");

			AfxMessageBox(msg, MB_ICONINFORMATION);

			OpenCSVFileInExcel(m_ProjectDirectory+"input_node_control_type.csv");
			OpenCSVFileInExcel(m_ProjectDirectory+"input_link_type.csv");
	}



	CWaitCursor wc;
	OpenWarningLogFile(directory);

	m_NodeSet.clear ();
	m_LinkSet.clear ();
	m_ODSize = 0;

	OnOpenAMSDocument(ProjectFileName);  
	CalculateDrawingRectangle(false);
	m_bFitNetworkInitialized  = false;


	ReadTransitFiles(directory+"transit_data\\");  // read transit data
	


	CTime LoadingEndTime = CTime::GetCurrentTime();

	CTimeSpan ts = LoadingEndTime  - LoadingStartTime;
	CString str_running_time;

	str_running_time.Format ("Network loading time: %d min(s) %d sec(s)...",ts.GetMinutes(), ts.GetSeconds());

	SetStatusText(str_running_time);

	m_AMSLogFile.close();
	m_ProjectFile = "";  // reset m_ProjectFile so that the user has to give a new project name
	return true;
}


void CTLiteDoc::OnImportDemanddataset()
{
	//CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
	//	_T("Importing Configuration (*.ini)|*.ini|"));
	//if(dlg.DoModal() == IDOK)
	//{
	//CTime LoadingStartTime = CTime::GetCurrentTime();

	//FILE* st = NULL;
	////	cout << "Reading file node.csv..."<< endl;

	//CString directory;
	//CString ProjectFile = dlg.GetPathName();
	//directory = ProjectFile.Left(ProjectFile.ReverseFind('\\') + 1);

	//// default data type definition files

	//m_AMSLogFile.open ( m_ProjectDirectory + "AMS_demand_conversion_log.csv", ios::out);
	//if (m_AMSLogFile.is_open())
	//{
	//	m_AMSLogFile.width(12);
	//	m_AMSLogFile.precision(3) ;
	//	m_AMSLogFile.setf(ios::fixed);
	//	m_AMSLogFile << "Start AMS demand reading..." << endl;
	//}else
	//{
	//	AfxMessageBox("File AMS_demand_conversion_log.csv cannot be opened, and it might be locked by another program or the target data folder is read-only.");
	//	return false;
	//}

	//int demand_format_flag = 0;
	//char demand_file_name[_MAX_STRING_SIZE] = "input_demand.csv";
	//char demand_file_field_name[_MAX_STRING_SIZE] = "demand_file_name";
	//char demand_type_field_name[_MAX_STRING_SIZE] ;
	//char demand_start_time_field_name[_MAX_STRING_SIZE];
	//char demand_end_field_name[_MAX_STRING_SIZE];

	//WritePrivateProfileString("demand_table","format_definition","0: AMS Demand CSV; 1: OD Matrix CSV; 2: 3-column format; 3: TransCAD 3-column CSV;4:VISUM matrix 8; 10: Gravity model",ProjectFileName);
	//demand_format_flag = g_GetPrivateProfileInt("demand_table","demand_format",1,ProjectFileName);

	//int number_of_tables = g_GetPrivateProfileInt("demand_table","number_of_files ",1,ProjectFileName);

	//for(int t = 1; t<= number_of_tables; t++)
	//{
	//fprintf(demand_file_field_name,"demand_file_name_table%d", t);
	//fprintf(demand_start_time_field_name,"demand_type_table%d", t);
	//fprintf(demand_end_field_name,"start_time_in_min_table%d", t);
	//fprintf(demand_end_field_name,"end_time_in_min_table%d", t);

	//g_GetProfileString("demand_table",demand_file_field_name,"input_demand.csv",demand_file_name,sizeof(demand_file_name),ProjectFileName);
	//
	//int demand_type = g_GetPrivateProfileInt("demand_table","number_of_files ",1,ProjectFileName);
	//int start_time_in_min = g_GetPrivateProfileInt("demand_table","number_of_files ",1,ProjectFileName);
	//int end_time_in_min = g_GetPrivateProfileInt("demand_table","number_of_files ",1,ProjectFileName);

	//CString msg;
	//msg.Format("demand_format= %d specified in %s is not supported. Please contact developers.",demand_format_flag,ProjectFileName);
	//
	//switch (demand_format_flag)
	//{
	//case 0:	ReadDemandCSVFile(directory+demand_file_name); break;
	//case 1: ReadDemandMatrixFile(directory+demand_file_name,1); break;
	//case 2: ReadTransCADDemandCSVFile(directory+demand_file_name); break;
	//case 3: ReadTransCADDemandCSVFile(directory+demand_file_name); break;
	//case 4: ReadVISUMDemandCSVFile(directory+demand_file_name,demand_type,start_time_in_min,end_time_in_min); break;
	//case 10: RunGravityModel(); break;
	//	
	//default:
	//	{
	//	AfxMessageBox(msg);
	//	}
	//}
	//}  //for demand table

	//m_AMSLogFile.close();

	//}
}

void CTLiteDoc::OnImportSynchrocombinedcsvfile()
{
	CWaitCursor wait;
	static char BASED_CODE szFilter[] = "Synchro Combined CSV File (*.csv)|*.csv||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		CString ProjectFile = dlg.GetPathName();
		CString Synchro_directory = ProjectFile.Left(ProjectFile.ReverseFind('\\') + 1);


		if(ReadSynchroCombinedCSVFile(dlg.GetPathName()))
		{
			OffsetLink();
			CalculateDrawingRectangle(true);
			m_bFitNetworkInitialized  = false;

		}

		m_LinkMOEMode = MOE_none;
		ShowTextLabel();

	
		UpdateAllViews(0);
	
	}
}

bool CTLiteDoc::ReadSynchroCombinedCSVFile(LPCTSTR lpszFileName)
{
	CWaitCursor wait;

	CString file_name;
	file_name.Format ("%s", lpszFileName);
	CString Synchro_directory = file_name.Left(file_name.ReverseFind('\\') + 1);
	// read users' prespecified control type
	if(ReadNodeControlTypeCSVFile(Synchro_directory+"input_node_control_type.csv") == false)
	{
	CString msg;
	msg.Format("Please first make sure file %s\\input_node_control_type.csv exists before importing synchro single csv file.", Synchro_directory);
	AfxMessageBox(msg, MB_ICONINFORMATION);
	
	}
	if(ReadLinkTypeCSVFile(Synchro_directory+"input_link_type.csv") == false)
	{
	CString msg;
	msg.Format("Please first make sure file %s\\input_link_type.csv exists before importing synchro single csv file.", Synchro_directory);
	AfxMessageBox(msg, MB_ICONINFORMATION);
	
	}

	
	std::map<int,int> UpNodeNumberMap;
	std::map<int,int> DestNodeNumberMap;


	std::map<int, int> m_INTIDMap;
	std::map<int, int> m_NodeOrgNumber2INTIDMap;
	std::map<int, int> m_NodeOrgNumber2NodeNoMap;

	CCSVParser parser;
	parser.m_bSynchroSingleCSVFile   = true;  // single file
	if (parser.OpenCSVFile(lpszFileName, false /*do not read first line as the header*/))
	{
		int i=0;
		int j= 1;  // actual node id for synchro

		std::map<int,int> from_node_id_map;

		while(parser.ReadRecord())
		{

			if(parser.m_SynchroSectionName.find ("Nodes")!=  string::npos) 
			{

			int intid;
			string name;
			DTANode* pNode = 0;

			int node_type;
			double X;
			double Y;
			if(parser.GetValueByFieldName("INTID",intid) == false)
				continue;

			if(!parser.GetValueByFieldName("INTNAME",name))
				name = "";

			if(!parser.GetValueByFieldName("TYPE",node_type))
				node_type = 0;

			// use the X and Y as default values first
			bool bFieldX_Exist = parser.GetValueByFieldName("X",X);
			parser.GetValueByFieldName("Y",Y);

			pNode = new DTANode;

			pNode->m_Name = name;

			pNode->m_ControlType = m_ControlType_NoControl;  // default value


			pNode->pt.x = X/5280.0f;  // feet to mile
			pNode->pt.y = Y/5280.0f;  // feet to mile


			int node_number_estimator  = j;
			if(intid < 10000 && intid > j)
			{
			  node_number_estimator = intid;  // encounter skipped nodes, move 
			  j = node_number_estimator;
			
			}
			pNode->m_NodeNumber = node_number_estimator; // intid is the name used , start from 1
			pNode->m_NodeOriginalNumber = node_number_estimator;
			m_INTIDMap [intid] = i;
			m_NodeOrgNumber2INTIDMap[intid] = node_number_estimator;
			pNode->m_NodeID = i;
			pNode->m_ZoneID = 0;
			pNode->m_CycleLengthInSecond = 0;
			m_NodeSet.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeIDtoNumberMap[i] = node_number_estimator; // start from 1
			m_NodeNumbertoIDMap[node_number_estimator] = i;
			m_NodeOrgNumber2NodeNoMap[intid] = i;
			i++;
			j++;

		} // node block
////////////////////////////////////////////

			m_DefaultLinkType = m_LinkTypeArterial;

		if(parser.m_SynchroSectionName.find ("Links")!=  string::npos) 
		{


			string direction_vector[8] = {"NB","SB","EB","WB","NE","NW","SE","SW"};

			int intid;
			string name;
			DTANode* pNode = 0;

			int node_type;
			double X;
			double Y;
			if(parser.GetValueByFieldName("INTID",intid) == false)
				continue;

			parser.GetValueByFieldName("RECORDNAME",name);
			int INTID;
				int m;
			//			switch (name)
			if (name == "Up ID" || name == "UpNodeID")
			{


				for(int direction = 0; direction < 8; direction++)
				{
					int  incoming_node_number;
					if(parser.GetValueByFieldName(direction_vector[direction],incoming_node_number))// value exits
					{
						// add a new link
						int to_node_id = m_INTIDMap[intid];


						if(m_NodeNumbertoIDMap.find(incoming_node_number) != m_NodeNumbertoIDMap.end() )
						{
							int from_node_id = m_NodeNumbertoIDMap[incoming_node_number];
							from_node_id_map[direction] = from_node_id;
							//if(m_NodeIDMap[to_node_id]->m_ControlType == m_ControlType_ExternalNode) XUESONG
							  // add new link if the outbound node is an external node
								AddNewLink(from_node_id, to_node_id,false, false);

						} else
						{
						
							if(m_NodeOrgNumber2NodeNoMap.find(incoming_node_number) != m_NodeOrgNumber2NodeNoMap.end())
							{

							int from_node_id = m_NodeOrgNumber2NodeNoMap[incoming_node_number];
							from_node_id_map[direction] = from_node_id;
							//if(m_NodeIDMap[to_node_id]->m_ControlType == m_ControlType_ExternalNode) XUESONG
							  // add new link if the outbound node is an external node
								AddNewLink(from_node_id, to_node_id,false);
							}

						}

					
					}
			

				}
	
			}

			///
			if (name == "Name")
			{

				for(int direction = 0; direction < 8; direction++)
				{
					string  name;
					if(parser.GetValueByFieldName(direction_vector[direction],name))// value exits
					{
						// add a new link
						int to_node_id = m_INTIDMap[intid];
						int from_node_id = from_node_id_map[direction];
						 DTALink* pLink = FindLinkWithNodeIDs(from_node_id, to_node_id);

						 if(pLink!=NULL)
						 {
							pLink->m_Name = name;
						 }

					}


				}	
				
			} // end of name attributes;
	
		} // link block
	
			if(parser.m_SynchroSectionName.find ("Lanes")!=  string::npos) 
		{

			// finish reading node and link blocks, now we construct movement data
		
			if(m_bMovementAvailableFlag==0)  // has not been initialized. 
			{
			Construct4DirectionMovementVector();

			m_MovementPointerMap.clear();

			std::list<DTANode*>::iterator iNode;
			for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
				{
							if((*iNode)->m_NodeID  == 7)
							{
								TRACE("");
							
							}

							for(unsigned int m = 0; m< (*iNode)->m_MovementVector .size(); m++)
						{

							DTANodeMovement movement = (*iNode)->m_MovementVector[m];

							CString label;
							int up_node_id = m_NodeIDMap[movement.in_link_from_node_id]->m_NodeID     ;
							int dest_node_id = m_NodeIDMap[movement.out_link_to_node_id ]->m_NodeID ;
							label.Format("%d;%d;%d", up_node_id,(*iNode)->m_NodeID ,dest_node_id);

							m_MovementPointerMap[label] = &((*iNode)->m_MovementVector[m]); // store pointer

		
				}
			
				}

			}

			//

			string direction_vector[12] = {"NBL","NBT","NBR","SBL","SBT","SBR","EBL","EBT","EBR","WBL",	"WBT",	"WBR"};

			string lane_att_name_str[28] = {"Lanes","Shared","Width","Storage","StLanes","Grade","Speed",
				"FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime",
				"SatFlow","Volume","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};

			int intid;
			string name;
			DTANode* pNode = 0;

			int node_type;
			double X;
			double Y;
			if(parser.GetValueByFieldName("INTID",intid) == false)
				continue;

			parser.GetValueByFieldName("RECORDNAME",name);

				int m;
			//			switch (name)
			if (name == "Up Node" || name == "UpNodeID")
			{
				UpNodeNumberMap.clear();
				DestNodeNumberMap.clear();

				for(int direction = 0; direction < 12; direction++)
				{
					int  incoming_node_number;
					if(parser.GetValueByFieldName(direction_vector[direction],incoming_node_number))// value exits
					{
						UpNodeNumberMap[direction] = incoming_node_number;

			
					}
			

				} //direction
	
			}  // Up Node
		
			if (name == "Dest Node" || name == "DestNodeID")
			{

				for(int direction = 0; direction < 12; direction++)
				{
					int  outgoing_node_number;
					if(parser.GetValueByFieldName(direction_vector[direction],outgoing_node_number))// value exits
					{

						DestNodeNumberMap[direction] = outgoing_node_number;
		
					}

				} //direction
	
			}  // Dest Node
			
			// Lanes

			int attribute_index = -1;

			if(name == "Volume")
			{
			TRACE("");
			}

			for(int n = 0;  n < 28; n ++)
			{

				if (name == lane_att_name_str[n])
				{
				attribute_index = n;
				break;
				}

				
			}
			

			if (attribute_index>=0)  // attrite has been defined
			{

						int to_node_id = m_INTIDMap[intid];
						for(int direction = 0; direction < 12; direction++)
				{



						if(UpNodeNumberMap.find(direction) != UpNodeNumberMap.end() )
						{

							int UpNodeNo = -1;
							
							if(m_NodeNumbertoIDMap.find(UpNodeNumberMap[direction]) != m_NodeNumbertoIDMap.end())
							{
							UpNodeNo = m_NodeNumbertoIDMap[UpNodeNumberMap[direction]];
							
							}else
							{
								if(m_NodeOrgNumber2NodeNoMap.find(UpNodeNumberMap[direction]) != m_NodeOrgNumber2NodeNoMap.end())
								{
								UpNodeNo = m_NodeOrgNumber2NodeNoMap[UpNodeNumberMap[direction]];
								}			
							
							}



							int DestNodeNo = -1;
							
							if(m_NodeNumbertoIDMap.find(DestNodeNumberMap[direction]) != m_NodeNumbertoIDMap.end())
							{
							DestNodeNo = m_NodeNumbertoIDMap[DestNodeNumberMap[direction]];
							
							}else
							{
								if(m_NodeOrgNumber2NodeNoMap.find(DestNodeNumberMap[direction]) != m_NodeOrgNumber2NodeNoMap.end())
								{
								DestNodeNo = m_NodeOrgNumber2NodeNoMap[DestNodeNumberMap[direction]];
								}	
							
							
							}


							int CurrentNodeNo = -1;
							if(m_NodeNumbertoIDMap.find(intid) != m_NodeNumbertoIDMap.end())
							{
							CurrentNodeNo = m_NodeNumbertoIDMap[intid];
							
							}else
							{
								if(m_NodeOrgNumber2NodeNoMap.find(intid) != m_NodeOrgNumber2NodeNoMap.end())
								{
								CurrentNodeNo = m_NodeOrgNumber2NodeNoMap[intid];
								}	
							
							
							}


								if(UpNodeNo<0)
									TRACE("\nNode ID %d cannot be found.",UpNodeNumberMap[direction]);

								if(DestNodeNo<0)
									TRACE("\nNode ID %d cannot be found.",DestNodeNumberMap[direction]);

								if(CurrentNodeNo<0)
									TRACE("\nNode ID %d cannot be found.",intid);


						CString label;
						label.Format("%d;%d;%d", UpNodeNo,CurrentNodeNo,DestNodeNo);
							if(m_MovementPointerMap.find(label) != m_MovementPointerMap.end())
							{
								DTANodeMovement* pMovement = m_MovementPointerMap[label];

								int  value = 0;
								if(parser.GetValueByFieldName(direction_vector[direction],value))// value exits
								{

									pMovement->QEM_dir_string = direction_vector[direction];


									/* {"Lanes","Shared","Width","Storage","StLanes","Grade","Speed",
				"FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow",
				14 "SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};
									*/
									switch(attribute_index)
									{
									case  0:// "Lanes"
									pMovement->QEM_Lanes = value; break;
									case  1:// "Shared"
									pMovement->QEM_Shared = value; break;
									case  2:// "Width"
									pMovement->QEM_Width = value; break;
									case  3:// "Storage"
									pMovement->QEM_Storage = value; break;
									case  4:// "StLanes"
									pMovement->QEM_StLanes = value; break;
									case  5:// "Grade"
									pMovement->QEM_Grade = value; break;
									case  6:// "Speed"
									pMovement->QEM_Speed = value; break;
									case  9:// "Phase1"
									pMovement->QEM_Phase1 = value; break;
									case  10:// "PermPhase1"
									pMovement->QEM_PermPhase1 = value; break;
									case  11:// "DetectPhase1"
									pMovement->QEM_DetectPhase1 = value; break;
									case  12:// "IdealFlow"
									pMovement->QEM_IdealFlow = value; break;
									case  13:// "LostTime"
									pMovement->QEM_LostTime = value; break;
									case  14:// "SatFlow"
									pMovement->QEM_SatFlow = value; break;
									case  15:// "Volume"
									pMovement->QEM_TurnVolume  = value; break;

									}
								}

							}else
							{
							
								TRACE("Cannot find movement %s\n", label);
							
							}

						}
				
				}  //direction
			} // lanes

/////////////


		}  // Lane attribute
			
	

			if(parser.m_SynchroSectionName.find ("Timeplans")!=  string::npos) 
			{

			string name;
			parser.GetValueByFieldName("RECORDNAME",name);


				if(name == "Cycle Length")
				{
				int intid;
				if(parser.GetValueByFieldName("INTID",intid) == false)
					continue;

				int node_id = m_INTIDMap[intid];

				int data = 0;
				if(parser.GetValueByFieldName("DATA",data) == false)
					continue;

				m_NodeIDMap[node_id]->m_CycleLengthInSecond = data;

				if(data > 10)  // set 
					m_NodeIDMap[node_id]->m_ControlType = m_ControlType_PretimedSignal;
				}

				if(name == "Offset")
				{
				int intid;
				if(parser.GetValueByFieldName("INTID",intid) == false)
					continue;

				int node_id = m_INTIDMap[intid];

				int data = 0;
				if(parser.GetValueByFieldName("DATA",data) == false)
					continue;

				m_NodeIDMap[node_id]->m_SignalOffsetInSecond  = data;

				}


			}

		}//while

	parser.CloseCSVFile ();

	// update QEM turn percentage

		// turning percentage
			std::list<DTANode*>::iterator iNode;

		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
				for(unsigned int m = 0; m< (*iNode)->m_MovementVector .size(); m++)
				{
					DTANodeMovement* pMovement = &((*iNode)->m_MovementVector[m]);
					DTALink* pLink0 = m_LinkNoMap[pMovement->IncomingLinkID  ];

					int total_link_count = 0;
					for(unsigned int j = 0; j< (*iNode)->m_MovementVector .size(); j++)
					{
					
						if((*iNode)->m_MovementVector[j].IncomingLinkID == pMovement->IncomingLinkID )
						{
						total_link_count+= (*iNode)->m_MovementVector[j].QEM_TurnVolume ;
						}
					
					}

					if (pMovement->QEM_TurnVolume >=1)
					{
						pMovement->QEM_TurnPercentage = 
						pMovement->QEM_TurnVolume * 100.0f / max(1,total_link_count);
					}
				}

		}


		m_NodeDataLoadingStatus.Format ("%d nodes are loaded from file %s.",m_NodeSet.size(),lpszFileName);
		return true;
	}else
	{
		AfxMessageBox("Error: File LAYOUT.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}


}


void CTLiteDoc::MapSignalDataAcrossProjects()
{

	class CrossReferenceNodeInfo
	{
	public:
		string intersection_name;
		string intersection_name2;
		int reference_node_id;
		int baseline_node_id;
	};

	// step 0: make sure two networks have been opened

	CTLiteDoc* pReferenceDoc = NULL;
		std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
	int project_index = 0 ;
	while (iDoc != g_DocumentList.end())
	{
		if((*iDoc)->m_NodeSet.size() >0 && (*iDoc)!=this)  
		{

		pReferenceDoc = (*iDoc);

		project_index++;
		}
		iDoc++;
	}

	if(pReferenceDoc == NULL)
	{
	
		AfxMessageBox("Please open the reference network data set in anoter window.");
		return;
	}
	
int count = 0 ;
	std::list<DTANode*>::iterator  iNode;

CFileDialog dlg (TRUE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
"Cross-reference Data File (*.csv)|*.csv||", NULL);
if(dlg.DoModal() == IDOK)
{

	CString SynchroProjectFile = dlg.GetPathName();

	CCSVParser parser;

	std::map<int, CrossReferenceNodeInfo> CrossReferenceNodeInfoMap;

	// step 1: read reference map
	CString cs = dlg.GetPathName();
	CT2CA pszConvertedAnsiString (cs);
	// construct a std::string using the LPCSTR input
	std::string strStd (pszConvertedAnsiString);

	if (parser.OpenCSVFile( strStd) )
	{

		string intersection_name;
		string intersection_name2;
		int reference_node_id;
		int baseline_node_id;

		while(parser.ReadRecord())
		{
		parser.GetValueByFieldName("baseline_node_id",baseline_node_id);
		
			if(baseline_node_id>=1)
			{
				parser.GetValueByFieldName("reference_node_id",reference_node_id);		
				parser.GetValueByFieldName("intersection_name",intersection_name);			
				parser.GetValueByFieldName("intersection_name2",intersection_name2);	

				CrossReferenceNodeInfoMap[baseline_node_id]. reference_node_id = reference_node_id;
				CrossReferenceNodeInfoMap[baseline_node_id]. intersection_name = intersection_name;
				CrossReferenceNodeInfoMap[baseline_node_id]. intersection_name2 = intersection_name2;

			
			}
		
		}

		
	}else
	{
	
	}

	// step 2: map cycle length and offset
	
	

	FILE* st = NULL;

	CString str_log_file;
	str_log_file.Format ("%s\\signal_data_reading_log.csv",m_ProjectDirectory);
	
	fopen_s(&st,str_log_file,"w");


	int count = 0;
	if( st !=NULL)
	{

	
//	fprintf(st,"intersection_name1,intersection_name2, baseline_node_id,reference_node_id,base_line_cycle_length_in_second,reference_cycle_length_in_second\n");
		
		fprintf(st,"------\n");
		fprintf(st,"Step 1: Check node control type,\n");

	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{

			int baseline_node_id = (*iNode)->m_NodeNumber  ;
			if(CrossReferenceNodeInfoMap.find(baseline_node_id) != CrossReferenceNodeInfoMap.end())
			{
			
				if((*iNode)->m_ControlType != m_ControlType_PretimedSignal && (*iNode)->m_ControlType != m_ControlType_ActuatedSignal)
				{
					fprintf(st,"Baseline,Node,%d,is not an signalized intersection, control type is changed to Pretimed Signal \n",  baseline_node_id);
					(*iNode)->m_ControlType = m_ControlType_PretimedSignal;
				}
			
			}
		}



		fprintf(st,"------\n");
		fprintf(st,"Step 2: Read Cycle length and offset,\n");

	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{

			int baseline_node_id = (*iNode)->m_NodeNumber  ;
			if(CrossReferenceNodeInfoMap.find(baseline_node_id) != CrossReferenceNodeInfoMap.end())
			{
				if((*iNode)->m_ControlType == m_ControlType_PretimedSignal || (*iNode)->m_ControlType == m_ControlType_ActuatedSignal)
				{
					
					DTANode* pRefNode = NULL;
					int reference_node_id =  CrossReferenceNodeInfoMap [baseline_node_id].reference_node_id;

					if( pReferenceDoc->m_NodeNumbertoIDMap.find(reference_node_id) !=  pReferenceDoc->m_NodeNumbertoIDMap.end())
					{
						int ReferenceNodeNo = pReferenceDoc->m_NodeNumbertoIDMap[reference_node_id];

						(*iNode)->m_CycleLengthInSecond = pReferenceDoc->m_NodeIDMap [ReferenceNodeNo] ->m_CycleLengthInSecond ;
						(*iNode)->m_SignalOffsetInSecond =pReferenceDoc->m_NodeIDMap [ReferenceNodeNo] ->m_SignalOffsetInSecond  ;
						fprintf(st,"Baseline,Node,%d,use reference node:,%d,obtains cycle length =,%d,offset,%d,%s,%s\n",  baseline_node_id,reference_node_id,(*iNode)->m_CycleLengthInSecond,(*iNode)->m_SignalOffsetInSecond,CrossReferenceNodeInfoMap [baseline_node_id].intersection_name.c_str (),CrossReferenceNodeInfoMap [baseline_node_id].intersection_name2.c_str () );
						count++;

					}else
					{
						
						fprintf(st,"Baseline,node,%d,'s reference node,%d,not found.\n",  reference_node_id,baseline_node_id);
					}
					
				}
			
			}
		}

	// step 3: construct node, movement direction map for reference project


			//step 3: map movement information
		fprintf(st,"------\n");
		fprintf(st,"Step 3: Find mapping movement,\n");

		
			for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
				{
					int baseline_node_id = (*iNode)->m_NodeNumber  ;
					if(CrossReferenceNodeInfoMap.find(baseline_node_id) != CrossReferenceNodeInfoMap.end())
					{
						if((*iNode)->m_ControlType == m_ControlType_PretimedSignal || (*iNode)->m_ControlType == m_ControlType_ActuatedSignal)
						{
							
							DTANode* pRefNode = NULL;
							int reference_node_id =  CrossReferenceNodeInfoMap [baseline_node_id].reference_node_id;

							if( pReferenceDoc->m_NodeNumbertoIDMap.find(reference_node_id) !=  pReferenceDoc->m_NodeNumbertoIDMap.end())
							{

								int ReferenceNodeNo = pReferenceDoc->m_NodeNumbertoIDMap[reference_node_id];

							for(unsigned int m = 0; m< (*iNode)->m_MovementVector .size(); m++)
							{

								DTANodeMovement baseline_movement = (*iNode)->m_MovementVector[m];

								int MovementIndex = pReferenceDoc->m_NodeIDMap [ReferenceNodeNo] ->FindMovementIndexFromDirecion(baseline_movement.movement_dir );

								if(baseline_movement.movement_dir >=0 && MovementIndex>=0)
								{
									DTANodeMovement* pThisMovement  = &((*iNode)->m_MovementVector[m]);
									DTANodeMovement reference_movement  =   pReferenceDoc->m_NodeIDMap [ReferenceNodeNo] ->m_MovementVector[MovementIndex];
									pThisMovement->QEM_TurnVolume = reference_movement.QEM_TurnVolume;
									

									//we use this function as it is possible th movements in the current network is not fully matched with the synchro network
									pThisMovement->QEM_LinkVolume  =
										pReferenceDoc->m_NodeIDMap [ReferenceNodeNo] ->FindHourlyCountFromDirection(reference_movement.movement_approach);

									pThisMovement->QEM_Lanes = reference_movement.QEM_Lanes;
									pThisMovement->QEM_Shared = reference_movement.QEM_Shared;
									pThisMovement->QEM_Width = reference_movement.QEM_Width;
									pThisMovement->QEM_Storage = reference_movement.QEM_Storage;
									pThisMovement->QEM_StLanes = reference_movement.QEM_StLanes;
									pThisMovement->QEM_Grade = reference_movement.QEM_Grade;
									pThisMovement->QEM_Speed = reference_movement.QEM_Speed;
									pThisMovement->QEM_IdealFlow = reference_movement.QEM_IdealFlow;
									pThisMovement->QEM_LostTime = reference_movement.QEM_LostTime;
									pThisMovement->QEM_Phase1 = reference_movement.QEM_Phase1;
									pThisMovement->QEM_DetectPhase1 = reference_movement.QEM_DetectPhase1;
									pThisMovement->QEM_TurnPercentage = reference_movement.QEM_TurnPercentage;
									pThisMovement->QEM_EffectiveGreen = reference_movement.QEM_EffectiveGreen;
									pThisMovement->QEM_Capacity = reference_movement.QEM_Capacity;
									pThisMovement->QEM_SatFlow = reference_movement.QEM_SatFlow;
									pThisMovement->QEM_VOC = reference_movement.QEM_VOC;
									pThisMovement->QEM_SatFlow = reference_movement.QEM_SatFlow;
									pThisMovement->QEM_Delay = reference_movement.QEM_Delay;
						
									fprintf(st,"Baseline,Node,%d,Up Node,%d,Dest Node,%d,%s,%s, obtains # of lanes =,%d,shared=,%d,Width=,%d,Storage=,%d\n",  
										baseline_node_id, 
										m_NodeIDMap[pThisMovement-> in_link_from_node_id]->m_NodeNumber,
										m_NodeIDMap[pThisMovement-> out_link_to_node_id]->m_NodeNumber,
										GetTurnDirectionString( pThisMovement-> movement_dir),
										GetTurnString( pThisMovement->movement_turn),
										pThisMovement->QEM_Lanes,
										pThisMovement->QEM_Shared,
										pThisMovement->QEM_Width,
										pThisMovement->QEM_Storage);
								}else
								{
									fprintf(st,"Baseline,Node,%d,Up Node,%d,Dest Node,%d,%s,%s,does not find reference movement.\n",  
										baseline_node_id, 
										m_NodeIDMap[(*iNode)->m_MovementVector[m]. in_link_from_node_id]->m_NodeNumber,
										m_NodeIDMap[(*iNode)->m_MovementVector[m]. out_link_to_node_id]->m_NodeNumber,
										GetTurnDirectionString((*iNode)->m_MovementVector[m]. movement_dir),
										GetTurnString((*iNode)->m_MovementVector[m].movement_turn));
							
								}

							} // for each movement
							}  // with reference node
						}  // signal control

					}
				}

		
		fclose(st);
	}else
	{
		CString str;
		str.Format("File %s cannot be opened.", str_log_file);
		AfxMessageBox(str);
	}
	
	
	OpenCSVFileInExcel(str_log_file);

	CString str;
	str.Format("%d nodes have obtained cycle length from the reference network. To accept the changes, please save the network.",count);
	AfxMessageBox(str, MB_ICONINFORMATION);

	}
}

void CTLiteDoc::OnImportInrixshapefileandspeeddata()
{
}


void CTLiteDoc::OnDemandConvert()
{
   static char BASED_CODE szFilter_demand[] = "Origin-based Static Demand Files (*.txt;*.oba))|*.txt;*.oba||";
   CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		   szFilter_demand);
   if(dlg.DoModal() == IDOK)
   {
	  ConvertOriginBasedDemandFile(dlg.GetPathName());
   } 

}

void  CTLiteDoc::ConvertOriginBasedDemandFile(LPCTSTR lpszFileName)
{
	CString directory;
	CString PathFile;
	PathFile.Format("%s",lpszFileName);
	directory = PathFile.Left(PathFile.ReverseFind('\\') + 1);


   FILE* st=fopen(lpszFileName,"r");
   FILE* outfile=fopen(directory+"input_demand.csv","w");

   if(outfile==NULL)
   {
   AfxMessageBox("File input_demand.csv cannot be opened to write.");
   
   return;
   
   }
   if(st!=NULL)
   {

	fprintf(outfile, "from_zone_id,to_zone_id,number_of_trips_demand_type1\n");

      // Number of matrices and the multiplication factor

      int ODSize = g_read_integer(st);
      float totoal_OD_volume =  g_read_float(st);

 
      // Destinations

      // There are two formats available

      for(int i = 1; i <= ODSize; i++)
      {
	 // Read each demand
	 int from_zone_id = g_read_integer(st);
	 
	 if(from_zone_id<=0) break;

		if(from_zone_id>i)  //advance
			i= from_zone_id;


	 for(int j = 1; j <= ODSize; j++)
	 {
	    int to_zone_id = g_read_integer_with_special_character(st,true,'O');  // break when reading "Origin"
	    if(to_zone_id<=0) break;

		float demand = g_read_float(st);


		if(to_zone_id>j)  //advance
			j= to_zone_id;

		fprintf(outfile,"%d,%d,%.4f\n", from_zone_id, to_zone_id,demand);

	 }

      }


      fclose(st);
	  fclose(outfile);

	  AfxMessageBox("File input_demand.csv has been generated.",MB_ICONINFORMATION    );

   }
}

void CTLiteDoc::IdentifyBottleNeckAndOnOffRamps()
{
	if(m_bIdentifyBottleneckAndOnOffRamps == true)
		return;
	else  // false
		m_bIdentifyBottleneckAndOnOffRamps = true;

	// keep working

	CWaitCursor wait;
	// ! there is an freeway or highway downstream with less number of lanes
		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		DTALink * pLink = (*iLink);
		if( m_LinkTypeMap[pLink->m_link_type].IsFreeway () 
			&&  m_NodeIDMap[pLink->m_ToNodeID ]->m_OutgoingLinkVector.size()==1)  // freeway or highway
		{
			int FromID = pLink->m_FromNodeID;
			int ToID   = pLink->m_ToNodeID;

			for(int i=0; i< m_NodeIDMap[ToID]->m_OutgoingLinkVector.size(); i++)
			{
				DTALink* pNextLink =  m_LinkNoMap[m_NodeIDMap[ToID]->m_OutgoingLinkVector[i]];
				if(m_LinkTypeMap[pNextLink->m_link_type ].IsFreeway () && pNextLink->m_NumberOfLanes  < pLink->m_NumberOfLanes && pNextLink->m_ToNodeID != FromID)
				{
					//					pLink->m_StochaticCapcityFlag = StochasticCapacityFlag;  //lane drop from current link to next link
					//g_LogFile << "lane drop:" << g_NodeVector[pLink->m_FromNodeID].m_NodeNumber << " ->" << g_NodeVector[pLink->m_ToNodeID].m_NodeNumber << endl;
				}

			}

		}
	}


	//// merge: one outgoing link, two more incoming links with at least freeway link

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		DTALink * pLink = (*iLink);
		int incoming_link_freeway_and_ramp_count = 0;
		bool no_arterial_incoming_link = true;

		DTANode* pFromNode = m_NodeIDMap[pLink->m_FromNodeID];

		for(int incoming_link = 0; incoming_link <  pFromNode->m_IncomingLinkVector .size(); incoming_link++) // one outgoing link without considering u-turn
		{
			DTALink* pIncomingLink = m_LinkNoMap[pFromNode->m_IncomingLinkVector[incoming_link]];

			if((pIncomingLink->m_FromNodeID != pLink->m_ToNodeID)) // non-uturn link
			{
				if(m_LinkTypeMap[pIncomingLink->m_link_type ].IsFreeway() //freeway link
					|| m_LinkTypeMap[pIncomingLink->m_link_type].IsRamp ())
				{
					incoming_link_freeway_and_ramp_count++;

				}else
				{
					no_arterial_incoming_link = false;

				}
			}

		}
		if(incoming_link_freeway_and_ramp_count >=2 && no_arterial_incoming_link)
		{
			TRACE("\nMerge link: %d->%d",pLink->m_FromNodeNumber , pLink->m_ToNodeNumber );
			pLink->m_bMergeFlag = 1;
		}

	}


	//// first count # of incoming freeway, highway or ramp links to each freeway/highway link
		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		DTALink * pLink = (*iLink);
		int FromID = pLink->m_FromNodeID;
		DTANode* pFromNode = m_NodeIDMap[FromID];
		if(pLink->m_bMergeFlag ==1 && pFromNode->m_IncomingLinkVector .size() == 2)  // is a merge bottlebeck link with two incoming links
		{
			int il;
			bool bRampExistFlag = false;
			bool bFreewayExistFlag = false;

			for(il = 0; il< pFromNode->m_IncomingLinkVector.size(); il++)
			{
				DTALink* pInLink = m_LinkNoMap[ pFromNode->m_IncomingLinkVector[il]];
				if(m_LinkTypeMap[pInLink->m_link_type].IsRamp ())  // on ramp as incoming link
				{
					bRampExistFlag = true;

					pInLink->m_bOnRampType = true;

					pLink ->m_FREEVALSegmentCode  = FREEVAL_ONR;

					pLink->m_MergeOnrampLinkID = pFromNode->m_IncomingLinkVector[il];
				}
				if(m_LinkTypeMap[pInLink->m_link_type ].IsFreeway () || 
					m_LinkTypeMap[pInLink->m_link_type ].IsHighway ())  // freeway or highway
				{
					bFreewayExistFlag = true;
					pLink->m_MergeMainlineLinkID = pFromNode->m_IncomingLinkVector[il];
				}
				if(bRampExistFlag && bFreewayExistFlag)
				{
					pLink->m_bMergeFlag = 2; // merge with ramp and mainline street
					//g_LogFile << "merge with ramp:" << g_NodeVector[pLink->m_FromNodeID].m_NodeNumber  << " ->" << g_NodeVector[pLink->m_ToNodeID].m_NodeNumber ;
					//g_LogFile << " with onramp:" << g_NodeVector[m_LinkNoMap[pLink->m_MergeOnrampLinkID]->m_FromNodeID].m_NodeNumber  << " ->" << g_NodeVector[m_LinkNoMap[pLink->m_MergeOnrampLinkID]->m_ToNodeID].m_NodeNumber ;
					//g_LogFile << " and freeway mainline:" << g_NodeVector[m_LinkNoMap[pLink->m_MergeMainlineLinkID]->m_FromNodeID ].m_NodeNumber << " ->" << g_NodeVector[m_LinkNoMap[pLink->m_MergeMainlineLinkID]->m_ToNodeID].m_NodeNumber << endl;
					break;
				}

			}


		}

		if(pLink->m_bMergeFlag ==1)
		{
			// merge with several merging ramps
			int ij;
			int TotalNumberOfLanes = 0;
			for( ij= 0; ij< pFromNode->m_IncomingLinkVector.size(); ij++)
			{
				TotalNumberOfLanes += m_LinkNoMap[  pFromNode->m_IncomingLinkVector[ij]]->m_NumberOfLanes  ;
			}

			for( ij= 0; ij< pFromNode->m_IncomingLinkVector.size(); ij++)
			{
				DTALink* pInLink = m_LinkNoMap[ pFromNode->m_IncomingLinkVector[ij]];

				RampLink mil;
				mil.m_LinkNo = pInLink->m_LinkNo ;
				mil.m_link_type =  pInLink->m_link_type ;
				mil.m_Length = pInLink->m_Length ;
				mil.m_NumberOfLanes =  pInLink->m_NumberOfLanes  ;
				mil.m_SpeedLimit = pInLink->m_SpeedLimit ;
				pLink->OnRampLinkVector.push_back(mil);
//				g_LogFile << "merge into freeway with multiple freeway/ramps:" << "No." << ij << " " << g_NodeVector[m_LinkNoMap[mil.m_LinkNo]->m_FromNodeID].m_NodeNumber  << " -> " << g_NodeVector[m_LinkNoMap[mil.m_LinkNo]->m_ToNodeID].m_NodeNumber <<  " with " << m_LinkNoMap[mil.m_LinkNo]->m_NumLanes  << " lanes and in flow capacity split " << mil.m_LinkInCapacityRatio << endl;
			}

		}

	}

	// determine offramp
		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		DTALink * pLink = (*iLink);
		if(m_LinkTypeMap[pLink->m_link_type].IsRamp ()) 
		{

			if(pLink->m_bOnRampType == false)  // all on ramp links have been marked
			{
		
				pLink->m_bOffRampType = true;


				DTALink* pFreewayLink= FindFreewayLinkWithToNodeNumber(pLink->m_FromNodeNumber );
				if(pFreewayLink!=NULL)
				{
				RampLink ramp;
				ramp.m_LinkNo = pLink->m_LinkNo ;
				ramp.m_Length = pLink->m_Length ;
				ramp.m_Length = pLink->m_Length ;
				ramp.m_link_type =  pLink->m_link_type ;
				ramp.m_NumberOfLanes =  pLink->m_NumberOfLanes  ;
				ramp.m_SpeedLimit = pLink->m_SpeedLimit ;
				pFreewayLink->OffRampLinkVector.push_back(ramp);
				//
				pFreewayLink->m_FREEVALSegmentCode = FREEVAL_OFR;
				}

				//	g_LogFile << "Offramp:" << g_NodeVector[pLink->m_FromNodeID].m_NodeNumber  << " -> " << g_NodeVector[pLink->m_ToNodeID].m_NodeNumber << endl;

			}


		}

	}
}

