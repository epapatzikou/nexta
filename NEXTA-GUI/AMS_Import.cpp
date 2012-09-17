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

	return capacity_in_pcphpl;
}
BOOL CTLiteDoc::OnOpenAMSDocument(CString FileName)
{
	#ifndef _WIN64

	CString warning_message = "";

	char model_units[_MAX_STRING_SIZE];
	GetPrivateProfileString("model_attributes","units","MI",model_units,sizeof(model_units),FileName);

	int b_long_lat_conversion_with_decimal_degree_flag =  g_GetPrivateProfileInt("model_attributes","long_lat_with_decimal_degrees",1,FileName);

	float long_lat_unit = 1.0f;
	if(b_long_lat_conversion_with_decimal_degree_flag == 0)
		long_lat_unit = 0.00001f;

	int addional_sign_of_latitude =  g_GetPrivateProfileInt("model_attributes","addional_sign_of_latitude",1,FileName);

	bool bMileFlag = false;
	if(strcmp(model_units,"MI")== 0 )
		bMileFlag = true;

	int direction_field_flag = g_GetPrivateProfileInt("model_attributes","direction_field",1,FileName);
	int control_type_field_flag = g_GetPrivateProfileInt("model_attributes","control_type_field",0,FileName);
	int reserve_direction_field_flag = g_GetPrivateProfileInt("model_attributes","reserve_direction_field",0,FileName);
	int offset_link_flag = g_GetPrivateProfileInt("model_attributes","offset_link",1,FileName);
	int link_capacity_flag = g_GetPrivateProfileInt("model_attributes","link_capacity_flag",1,FileName);
	int number_of_lanes_for_two_way_links_flag = g_GetPrivateProfileInt("model_attributes","number_of_lanes_for_two_way_links",0,FileName);
	int use_optional_centroid_layer = g_GetPrivateProfileInt("model_attributes","use_optional_centroid_layer",0,FileName);
	int use_optional_connector_layer = g_GetPrivateProfileInt("model_attributes","use_optional_connector_layer",0,FileName);


	// ************************************/
	// 1: node table

	// ************************************/



	char node_table_file_name[_MAX_STRING_SIZE];
	GetPrivateProfileString("node_table","reference_file_name","",node_table_file_name,sizeof(node_table_file_name),FileName);
	char node_name[_MAX_STRING_SIZE];
	GetPrivateProfileString("node_table","name","NAME",node_name,sizeof(node_name),FileName);
	char node_node_id[_MAX_STRING_SIZE];
	GetPrivateProfileString("node_table","node_id","NO",node_node_id,sizeof(node_node_id),FileName);
	char node_control_type[_MAX_STRING_SIZE];
	GetPrivateProfileString("node_table","control_type","CONTROLT~1",node_control_type,sizeof(node_control_type),FileName);

	char node_TAZ_name[_MAX_STRING_SIZE];
	GetPrivateProfileString("node_table","TAZ","TAZ",node_TAZ_name,sizeof(node_TAZ_name),FileName);


	//	; Control type 0 = unknown, 1 = uncontrolled, 2 = two-way stop, 6 = two-way yield, 3 = signalized, 4 = all-way stop, 5 = roundabout

	CString message_str;
	OGRRegisterAll();

	OGRDataSource       *poDS;

	CString node_shape_file_name;
	node_shape_file_name = m_ProjectDirectory + node_table_file_name;

	poDS = OGRSFDriverRegistrar::Open(node_shape_file_name, FALSE );
	if( poDS == NULL )
	{

		AfxMessageBox("Open node shape file failed.");
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
			float x = 0;
			float y = 0;


			// node id
			int id = poFeature->GetFieldAsInteger(node_node_id);

			int control_type = 0;

			if(control_type_field_flag)
				control_type_field_flag = poFeature->GetFieldAsInteger(node_control_type);

			int TAZ= poFeature->GetFieldAsInteger(node_TAZ_name);

			CString str_name = poFeature->GetFieldAsString(node_name);

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


			if(TAZ>=1)
			{
				pNode->m_bZoneActivityLocationFlag = true;
				m_ZoneMap [TAZ].m_ZoneTAZ = TAZ;

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
			m_NodeIDtoNameMap[i] = id;
			m_NodeNametoIDMap[id] = i;
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

	GetPrivateProfileString("zone_centroid_conversion","reference_file_name","",node_table_file_name,sizeof(node_table_file_name),FileName);

	m_AMSLogFile << "read optional centroid layer from file " << node_table_file_name << endl; 

	GetPrivateProfileString("zone_centroid_conversion","name","NAME",node_name,sizeof(node_name),FileName);
	GetPrivateProfileString("zone_centroid_conversion","node_id","NO",node_node_id,sizeof(node_node_id),FileName);
	GetPrivateProfileString("zone_centroid_conversion","TAZ","TAZ",node_TAZ_name,sizeof(node_TAZ_name),FileName);

	//	; Control type 0 = unknown, 1 = uncontrolled, 2 = two-way stop, 6 = two-way yield, 3 = signalized, 4 = all-way stop, 5 = roundabout

	CString message_str;

	OGRDataSource       *poDS;

	CString node_shape_file_name;
	node_shape_file_name = m_ProjectDirectory + node_table_file_name;

	poDS = OGRSFDriverRegistrar::Open(node_shape_file_name, FALSE );
	if( poDS == NULL )
	{

		AfxMessageBox("Open zone centroid shape file failed.");
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
			float x = 0;
			float y = 0;


			// node id
			int id = poFeature->GetFieldAsInteger(node_node_id);

			int control_type = 0;

			int TAZ= poFeature->GetFieldAsInteger(node_TAZ_name);

			if(TAZ == 0)  // if TAZ value is zero, then use node id as TAZ
				TAZ = id; 


			CString str_name = poFeature->GetFieldAsString(node_name);

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
			pNode->m_ControlType = control_type;


			if(TAZ>=1)
			{
				pNode->m_bZoneActivityLocationFlag = true;
				m_ZoneMap [TAZ].m_ZoneTAZ = TAZ;

				DTAActivityLocation element;
				element.ZoneID  = TAZ;
				element.NodeNumber = id;
				m_ZoneMap [TAZ].m_ActivityLocationVector .push_back (element );

				if(m_ODSize < TAZ)
					m_ODSize = TAZ;

				node_zone_mapping_count ++;

			}


			if(m_NodeNametoIDMap.find(id) == m_NodeNametoIDMap.end())  // no duplicated nodes
			{
			m_NodeSet.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeIDtoNameMap[i] = id;
			m_NodeNametoIDMap[id] = i;
			i++;
			}

			m_AMSLogFile  << endl;
		}

		m_AMSLogFile << "imported " << i << " nodes. " << endl; 
		m_AMSLogFile << "imported " << m_ZoneMap.size() << " zones from the node layer. " << endl; 
		m_NodeDataLoadingStatus.Format ("%d nodes are loaded from file %s.",m_NodeSet.size(),node_shape_file_name);
		m_ZoneDataLoadingStatus.Format ("%d node-zone mapping entries are loaded from file %s.",node_zone_mapping_count,node_shape_file_name);

		// to do: # of nodes: control: two-way stop signs....
	}

	OGRDataSource::DestroyDataSource( poDS );

	}


	// ************************************/
	// 2: link table

	// ************************************/
	bool bSkipShapePoints = false;
	bool read_link_layer = true;

	bool bTwoWayLinkFlag = false;

	float default_distance_sum=0;
	float length_sum = 0;

	if(read_link_layer)
	{
		char link_table_file_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","reference_file_name","reference_file_name",link_table_file_name,sizeof(link_table_file_name),FileName);

		char from_node_id_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","from_node_id","from_node_id",from_node_id_name,sizeof(from_node_id_name),FileName);
		char to_node_id_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","to_node_id","to_node_id",to_node_id_name,sizeof(to_node_id_name),FileName);

		char link_id_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","link_id","link_id",link_id_name,sizeof(link_id_name),FileName);

		char link_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","name","name",link_name,sizeof(link_name),FileName);

		char link_type_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","link_type","link_type",link_type_name,sizeof(link_type_name),FileName);

		char mode_code_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","mode_code","mode_code",mode_code_name,sizeof(mode_code_name),FileName);

		char direction_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","direction","direction",direction_name,sizeof(direction_name),FileName);

		char length_name[_MAX_STRING_SIZE];
		if(bMileFlag)
			GetPrivateProfileString("link_table","length_in_mile","length_in_mile",length_name,sizeof(length_name),FileName);
		else
			GetPrivateProfileString("link_table","length_in_km","length_in_km",length_name,sizeof(length_name),FileName);

		char number_of_lanes_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","number_of_lanes","number_of_lanes",number_of_lanes_name,sizeof(number_of_lanes_name),FileName);

		char lane_capacity_in_vhc_per_hour_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","lane_capacity_in_vhc_per_hour","lane_capacity_in_vhc_per_hour",lane_capacity_in_vhc_per_hour_name,sizeof(lane_capacity_in_vhc_per_hour_name),FileName);

		char speed_limit_in_mph_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("link_table","speed_limit_in_mph","speed_limit_in_mph",speed_limit_in_mph_name,sizeof(speed_limit_in_mph_name),FileName);

		char r_number_of_lanes_name[_MAX_STRING_SIZE];
		char r_lane_capacity_in_vhc_per_hour_name[_MAX_STRING_SIZE];
		char r_speed_limit_in_mph_name[_MAX_STRING_SIZE];
		char r_link_type_name[_MAX_STRING_SIZE];
		if(reserve_direction_field_flag)
		{
			GetPrivateProfileString("link_table","r_number_of_lanes","number_of_lanes",r_number_of_lanes_name,sizeof(r_number_of_lanes_name),FileName);
			GetPrivateProfileString("link_table","r_lane_capacity_in_vhc_per_hour","lane_capacity_in_vhc_per_hour",r_lane_capacity_in_vhc_per_hour_name,sizeof(r_lane_capacity_in_vhc_per_hour_name),FileName);
			GetPrivateProfileString("link_table","r_speed_limit_in_mph","speed_limit_in_mph",r_speed_limit_in_mph_name,sizeof(r_speed_limit_in_mph_name),FileName);
			GetPrivateProfileString("link_table","r_link_type","link_type",r_link_type_name,sizeof(r_link_type_name),FileName);
		}



		CString link_shape_file_name;
		link_shape_file_name = m_ProjectDirectory + link_table_file_name;

		poDS = OGRSFDriverRegistrar::Open(link_shape_file_name, FALSE );
		if( poDS == NULL )
		{
			AfxMessageBox("Open link shape file failed.");
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
			int i = 0;
			int line_no = 1;



			m_AMSLogFile << endl << endl << "2: link block---" << endl;
			m_AMSLogFile << "from_node_id,to_name_id,link_id,name,type,direction,length,number_of_lanes,speed_limit,capacity," << endl;

			if(reserve_direction_field_flag)
			{
				m_AMSLogFile << "r_number_of_lanes,r_speed_limit,r_capacity," << endl;
			}


			while( (poFeature = poLayer->GetNextFeature()) != NULL )
			{
				OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
				int from_node_id = poFeature->GetFieldAsInteger(from_node_id_name);
				int to_node_id = poFeature->GetFieldAsInteger(to_node_id_name);

				if(from_node_id == 613)
					TRACE("");

				long link_id =  poFeature->GetFieldAsInteger(link_id_name);
				CString name =  poFeature->GetFieldAsString(link_name);
				int type = poFeature->GetFieldAsInteger(link_type_name);

				CString mode_code = poFeature->GetFieldAsString(mode_code_name);


				float speed_limit_in_mph= poFeature->GetFieldAsDouble(speed_limit_in_mph_name);

				int direction = 1;
				if(direction_field_flag) 
					direction = poFeature->GetFieldAsInteger(direction_name);

				float length = poFeature->GetFieldAsDouble(length_name);

				int number_of_lanes = poFeature->GetFieldAsInteger(number_of_lanes_name);

				if(direction_field_flag == 1 && (direction==0 || direction==2) && number_of_lanes_for_two_way_links_flag ==1 )
				{
					number_of_lanes = number_of_lanes/2;
				}

				if(type==0)  // no type information available
				{
					// check speed limit to determine type

					if(speed_limit_in_mph>=55)
						type = 1; // default freeway
					else
						type = 4; // default arterial street

					if(number_of_lanes>=7)
						type = 10; // default connectors;

					
					int node_id  =0 ;
					
					if(m_NodeNametoIDMap.find (from_node_id) != m_NodeNametoIDMap.end())
					{
						node_id =  m_NodeNametoIDMap[from_node_id];
						DTANode* pFromNode = m_NodeIDMap[node_id];
						if(pFromNode->m_ZoneID >=1)
						{
						  // from node is a zone centroid, then the link is a connector
							type = 10;

						}


					}
				
				}


				float capacity_in_pcphpl= poFeature->GetFieldAsDouble(lane_capacity_in_vhc_per_hour_name);

				capacity_in_pcphpl = ComputeCapacity(capacity_in_pcphpl,link_capacity_flag, speed_limit_in_mph,number_of_lanes);

				// if link_capacity_flag == 0, we give a default value
				int r_number_of_lanes =0; 
				int r_link_type = 0; 
				float r_speed_limit_in_mph = 0; 
				float r_capacity_in_pcphpl=0; 

				if(reserve_direction_field_flag)  // with reserved direction field
				{
					r_number_of_lanes = poFeature->GetFieldAsInteger(r_number_of_lanes_name);
					if(direction_field_flag == 1 && (direction==0 || direction==2) && number_of_lanes_for_two_way_links_flag ==1)
					{
						number_of_lanes = r_number_of_lanes/2;
					}

					r_speed_limit_in_mph= poFeature->GetFieldAsDouble(r_speed_limit_in_mph_name);
					r_capacity_in_pcphpl= poFeature->GetFieldAsDouble(r_lane_capacity_in_vhc_per_hour_name);
					r_capacity_in_pcphpl = ComputeCapacity(r_capacity_in_pcphpl,link_capacity_flag, r_speed_limit_in_mph,number_of_lanes);
					r_link_type= poFeature->GetFieldAsInteger(r_link_type_name);

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

				m_AMSLogFile << from_node_id << "," << to_node_id << "," << link_id << "," << name << "," << type << "," << direction << ",";
				m_AMSLogFile << length << "," << number_of_lanes << "," << speed_limit_in_mph << ","  << capacity_in_pcphpl << ",";

				if(reserve_direction_field_flag)
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

				int link_code_start = 1;
				int link_code_end = 1;

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

				if(bSkipShapePoints)
				{
				// no geometry information
				CCoordinate cc_from, cc_to; 
				cc_from.X = m_NodeIDMap[m_NodeNametoIDMap[from_node_id]]->pt.x;
				cc_from.Y = m_NodeIDMap[m_NodeNametoIDMap[from_node_id]]->pt.y;

				cc_to.X = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]]->pt.x;
				cc_to.Y = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]]->pt.y;

				CoordinateVector.push_back(cc_from);
				CoordinateVector.push_back(cc_to);
 
				}else
				{


				poGeometry = poFeature->GetGeometryRef();
				if( poGeometry != NULL )
				{	if(wkbFlatten(poGeometry->getGeometryType()) == wkbLineString )
				{
					OGRLineString *poLine = (OGRLineString *) poGeometry;

					m_AMSLogFile << "{" ;

					int step = 1;

						int Number_of_Shape_Points = poLine->getNumPoints();

						if(Number_of_Shape_Points > 100)
						{
						TRACE("\nNumber_of_Shape_Points = %d",Number_of_Shape_Points);
						}
					
					if(Number_of_Shape_Points>=10)  //
						step = (int)(Number_of_Shape_Points/10);

					for(unsigned int si = 0; si< Number_of_Shape_Points; si+=step)
					{
						CCoordinate pt;
						pt.X   =  poLine->getX(si)*long_lat_unit;
						pt.Y =  poLine->getY(si)*long_lat_unit;
						CoordinateVector.push_back(pt);
						m_AMSLogFile << pt.X << ";" << pt.Y << " ";

					}
					m_AMSLogFile << "}" ;

				}
				}

				}
				if(from_node_id==0 && to_node_id ==0)  // test twice here for from and to nodes
				{
					AfxMessageBox("Invalid data: from_node_id==0 && to_node_id ==0 in the link table.");
					return false;
				}

				
				DTALink* pExistingLink =  FindLinkWithNodeIDs(m_NodeNametoIDMap[from_node_id],m_NodeNametoIDMap[to_node_id]);

				// error checking 
				CString str_msg;
				if(pExistingLink)
				{
				str_msg.Format ("Link %d-> %d at row %d is duplicated with the previous link at row %d. NEXTA will skip this link.\n", from_node_id,to_node_id, line_no, pExistingLink->input_line_no);
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
				warning_message+= str_msg;


				line_no ++;
				for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
				{

					bool bNodeNonExistError = false;
					int m_SimulationHorizon = 1;
					DTALink* pLink = new DTALink(m_SimulationHorizon);
					pLink->m_LinkNo = i;
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

						pLink->m_FromNodeID = m_NodeNametoIDMap[from_node_id];
						pLink->m_ToNodeID= m_NodeNametoIDMap[to_node_id];


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
						pLink->m_FromNodeID = m_NodeNametoIDMap[to_node_id];
						pLink->m_ToNodeID= m_NodeNametoIDMap[from_node_id];


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

					m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);

					pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
					pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;


					default_distance_sum+= pLink->DefaultDistance();
					length_sum += pLink ->m_Length;
					//			pLink->SetupMOE();
					pLink->input_line_no  = line_no;



					m_LinkSet.push_back (pLink);
					m_LinkNoMap[i]  = pLink;
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


					m_NodeIDMap[pLink->m_FromNodeID ]->m_OutgoingLinkVector.push_back(i);
					m_NodeIDMap[pLink->m_ToNodeID ]->m_IncomingLinkVector.push_back(i);


					unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
					m_NodeIDtoLinkMap[LinkKey] = pLink;


					__int64  LinkKey2 = pLink-> m_FromNodeNumber* pLink->m_ToNodeNumber;
					m_NodeNumbertoLinkMap[LinkKey2] = pLink;


					i++;

				}  // link code

				m_AMSLogFile  << endl;
			}  // feature

			m_AMSLogFile << "imported " << i << " links. " << endl; 
			m_LinkDataLoadingStatus.Format ("%d links are loaded from file %s.",m_LinkSet.size(),link_shape_file_name);


		}  // layer
		// to do: # of nodes: control: two-way stop signs....

		OGRDataSource::DestroyDataSource( poDS );

		}

				
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

		char link_table_file_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("connector_conversion","reference_file_name","reference_file_name",link_table_file_name,sizeof(link_table_file_name),FileName);
		m_AMSLogFile << "starting converting centors from file " << link_table_file_name;

		char from_node_id_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("connector_conversion","zone_end","ZONENO",from_node_id_name,sizeof(from_node_id_name),FileName);
		char to_node_id_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("connector_conversion","node_end","NODENO",to_node_id_name,sizeof(to_node_id_name),FileName);

		char length_name[_MAX_STRING_SIZE];
		if(bMileFlag)
			GetPrivateProfileString("connector_conversion","length_in_mile","length_in_mile",length_name,sizeof(length_name),FileName);
		else
			GetPrivateProfileString("connector_conversion","length_in_km","length_in_km",length_name,sizeof(length_name),FileName);


		int default_number_of_lanes = g_GetPrivateProfileInt("connector_conversion","default_number_of_lanes",2,FileName);
		int default_lane_capacity = g_GetPrivateProfileInt("connector_conversion","lane_capacity",10000,FileName);
		int default_speed_limit = g_GetPrivateProfileInt("connector_conversion","default_speed_limit",60,FileName);
		int default_link_type = g_GetPrivateProfileInt("connector_conversion","default_link_type_for_connector",99,FileName);


		if(m_LinkTypeMap.find(default_link_type)==m_LinkTypeMap.end())
		{
		
			CString message;
			message.Format("default_link_type_for_connector=%d has not been defined in the input_link_type.csv file.\nThe NEXTA data conversion utility will add this link type definition automatically.");

			AfxMessageBox(message);

			m_LinkTypeMap[default_link_type].link_type = default_link_type;
			m_LinkTypeMap[default_link_type].type_code = 'c';
			m_LinkTypeMap[default_link_type].default_lane_capacity = 4000;


		}
		char direction_name[_MAX_STRING_SIZE];
		GetPrivateProfileString("connector_conversion","direction","direction",direction_name,sizeof(direction_name),FileName);

		int direction = g_GetPrivateProfileInt("connector_conversion","default_direction",0,FileName);

		CString link_shape_file_name;
		link_shape_file_name = m_ProjectDirectory + link_table_file_name;

		poDS = OGRSFDriverRegistrar::Open(link_shape_file_name, FALSE );
		if( poDS == NULL )
		{
			AfxMessageBox("Open connector shape file failed.");
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
			int i = 0;
			int line_no = 1;

			m_AMSLogFile << endl << endl << "2.2: connector block---" << endl;
			m_AMSLogFile << "from_node_id,to_name_id,length,number_of_lanes,speed_limit,capacity," << endl;

			while( (poFeature = poLayer->GetNextFeature()) != NULL )
			{
				OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
				int from_node_id = poFeature->GetFieldAsInteger(from_node_id_name);
				int to_node_id = poFeature->GetFieldAsInteger(to_node_id_name);

				if(direction_field_flag) 
				{
					direction = poFeature->GetFieldAsInteger(direction_name);
				}


				long link_id =  0;
				int type = default_link_type;  // find default connectors type.
				float length = poFeature->GetFieldAsDouble(length_name);

				int number_of_lanes = default_number_of_lanes;
				int capacity_in_pcphpl = default_lane_capacity;
				float speed_limit_in_mph = default_speed_limit;

				m_AMSLogFile << from_node_id << "," << to_node_id << "," << length << "," << number_of_lanes << "," << speed_limit_in_mph << ","  << capacity_in_pcphpl << ",";

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
				{	if(wkbFlatten(poGeometry->getGeometryType()) == wkbLineString )
				{
					OGRLineString *poLine = (OGRLineString *) poGeometry;

					m_AMSLogFile << "{" ;

					int step = 1;

						int Number_of_Shape_Points = poLine->getNumPoints();

						if(Number_of_Shape_Points > 100)
						{
						TRACE("\nNumber_of_Shape_Points = %d",Number_of_Shape_Points);
						}
					
					if(Number_of_Shape_Points>=10)  //
						step = (int)(Number_of_Shape_Points/10);

					for(unsigned int si = 0; si< Number_of_Shape_Points; si+=step)
					{
						CCoordinate pt;
						pt.X   =  poLine->getX(si)*long_lat_unit;
						pt.Y =  poLine->getY(si)*long_lat_unit;
						CoordinateVector.push_back(pt);
						m_AMSLogFile << pt.X << ";" << pt.Y << " ";

					}
					m_AMSLogFile << "}" ;

				}

				
				if(from_node_id==0 && to_node_id ==0)  // test twice here for from and to nodes
				{
					AfxMessageBox("Invalid data: from_node_id==0 && to_node_id ==0 in the link table.");
					return false;
				}


				line_no ++;
				for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
				{

					bool bNodeNonExistError = false;
					int m_SimulationHorizon = 1;
					DTALink* pLink = new DTALink(m_SimulationHorizon);
					pLink->m_LinkNo = i;
					pLink->m_Name  = "connector";
					pLink->m_OrgDir = direction;
					pLink->m_LinkID = link_id;

					if(link_code == 1)  //AB link
					{
						pLink->m_FromNodeNumber = from_node_id;

						pLink->m_ToNodeNumber = to_node_id;
						pLink->m_Direction  = 1;

						pLink->m_FromNodeID = m_NodeNametoIDMap[from_node_id];
						pLink->m_ToNodeID= m_NodeNametoIDMap[to_node_id];


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
						pLink->m_FromNodeID = m_NodeNametoIDMap[to_node_id];
						pLink->m_ToNodeID= m_NodeNametoIDMap[from_node_id];


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

					pLink->m_FreeFlowTravelTime = pLink->m_Length / max(1,pLink->m_SpeedLimit) *60.0f;
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
						pLink->m_NumLanes= number_of_lanes;
						pLink->m_SpeedLimit= speed_limit_in_mph;
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

					m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);

					pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
					pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;


					default_distance_sum+= pLink->DefaultDistance();
					length_sum += pLink ->m_Length;
					//			pLink->SetupMOE();
					pLink->input_line_no  = line_no;

					m_LinkSet.push_back (pLink);
					m_LinkNoMap[i]  = pLink;
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
			m_LinkDataLoadingStatus.Format ("%d links are loaded from file %s.",m_LinkSet.size(),link_shape_file_name);


		}  // layer
		// to do: # of nodes: control: two-way stop signs....

		OGRDataSource::DestroyDataSource( poDS );


	}

	// post-processing link information

			//ConstructMovementVectorForEachNode();
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
	char zone_table_file_name[_MAX_STRING_SIZE];
	GetPrivateProfileString("zone_table","reference_file_name","",zone_table_file_name,sizeof(zone_table_file_name),FileName);
	char zone_id_name[_MAX_STRING_SIZE];
	GetPrivateProfileString("zone_table","zone_id","ID",zone_id_name,sizeof(zone_id_name),FileName);

	CString zone_shape_file_name;
	zone_shape_file_name = m_ProjectDirectory + zone_table_file_name;

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

				return false;			
			}

			OGRFeature *poFeature;

			int feature_count = 0;

			poLayer->ResetReading();



			m_AMSLogFile << "3: zone block---" << endl;
			m_AMSLogFile << "zone id" << endl;


			while( (poFeature = poLayer->GetNextFeature()) != NULL )
			{
				OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();

				// zone id
				int id = poFeature->GetFieldAsInteger(zone_id_name);

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
	float total_demand = 0;
	long line_no = 0;

	std::map<int, DTAZone>	:: iterator itr_o;
	std::map<int, DTAZone>	:: iterator itr_d;

	// stage 1: determine initial production and attraction
	for(itr_o = m_ZoneMap.begin(); itr_o != m_ZoneMap.end(); itr_o++)  // for each origin
	{
		if(itr_o->second.m_bWithinSubarea)
		{
		for(unsigned int i = 0; i< itr_o->second.m_ActivityLocationVector.size(); i++)
		{
			DTANode* pNode = m_NodeIDMap[m_NodeNametoIDMap[itr_o->second.m_ActivityLocationVector[i].NodeNumber]];
				
 		itr_o->second .m_Production = pNode ->m_NodeProduction;
		itr_o->second .m_Attraction = pNode ->m_NodeAttraction;

		}
		}
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
				total_relative_attraction += (itr_d)->second .m_Attraction;
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

			float number_of_vehicles = itr_o->second .m_Production * (itr_d)->second .m_Attraction / total_relative_attraction;


			sub_total += number_of_vehicles;
			
			// m_ZoneMap[origin_zone].m_ODDemandMatrix [destination_zone].SetValue (demand_type,number_of_vehicles);

			CString label;
			
			label.Format("%d,%d", origin_zone   , destination_zone);
			m_ODMatrixMap[label].Origin = origin_zone;
			m_ODMatrixMap[label].Destination  = destination_zone;
			m_ODMatrixMap[label].TotalVehicleSize = number_of_vehicles; 
			total_demand += number_of_vehicles;

			if(number_of_vehicles < -0.0001)
				number_of_vehicles = 0;
			}
		}

	} // for each origin

	return false;
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

			if(!parser.GetValueByFieldName("INTNAME",name))
				name = "";

			if(!parser.GetValueByFieldName("TYPE",node_type))
				node_type = 0;

			// use the X and Y as default values first
			bool bFieldX_Exist = parser.GetValueByFieldName("X",X);
			parser.GetValueByFieldName("Y",Y);

			if(node_type != 1)  // not external node
			{

				for(int direction = 0; direction < 8; direction++)
				{
					int outgoing_node_number;
					if(parser.GetValueByFieldName(direction_vector[direction],outgoing_node_number))// value exits
					{
						// add a new link
						int from_node_id = m_NodeNametoIDMap[node_id];
						int to_node_id = m_NodeNametoIDMap[outgoing_node_number];

						if(m_NodeIDMap.find(to_node_id) != m_NodeIDMap.end())
						{
							if(m_NodeIDMap[to_node_id]->m_ControlType == m_ControlType_ExternalNode)
							{  // add new link if the outbound node is an external node
								AddNewLink(from_node_id, to_node_id,false);
								AddNewLink(to_node_id,from_node_id,false);
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

			if(m_NodeNametoIDMap.find(node_id) != m_NodeNametoIDMap.end())
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
			m_NodeIDtoNameMap[i] = node_id;
			m_NodeNametoIDMap[node_id] = i;
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
				//add links and set up phases;

				int max_phase_number = 0;

				int to_node_id  = INTID;

				// set control type
				for(m = 0; m< LaneColumnSize; m++)
				{

						if(LaneDataMap.find(lane_Column_name_str[m]) != LaneDataMap.end() )  // data exists
						{


							int from_node_id  = LaneDataMap[lane_Column_name_str[m]].UpNode;
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

	
							// add link
							DTALink* pExistingLink =  FindLinkWithNodeIDs(m_NodeNametoIDMap[from_node_id],m_NodeNametoIDMap[to_node_id]);

							if(pExistingLink)
							{
								//the approach has been processed.
								continue;
							}
							TRACE("\n add ->> movement %s, from node %d to node %d, with %d links",
								lane_Column_name_str[m].c_str (), from_node_id, to_node_id,  LaneDataMap[lane_Column_name_str[m]].Lanes );

							int bToBeShifted = 1;
							int number_of_lanes = LaneDataMap[lane_Column_name_str[m]].Lanes ;
							float speed_limit_in_mph = LaneDataMap[lane_Column_name_str[m]].Speed ;
							float capacity_in_pcphpl =  LaneDataMap[lane_Column_name_str[m]].SatFlow / max(1,number_of_lanes);  // convert per link flow rate to per lane flow rate

							int m_SimulationHorizon = 1;
							DTALink* pLink = new DTALink(m_SimulationHorizon);
							pLink->m_LayerNo = LayerNo;
							pLink->m_LinkNo = i;
							pLink->m_LinkID = i;


							pLink->m_FromNodeNumber = from_node_id;

							pLink->m_ToNodeNumber = to_node_id;
							pLink->m_Direction  = 1;

							if(pLink->m_FromNodeNumber == 12 && pLink->m_ToNodeNumber == 2)
							TRACE("");

							pLink->m_FromNodeID = m_NodeNametoIDMap[from_node_id];
							pLink->m_ToNodeID= m_NodeNametoIDMap[to_node_id];

							pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
							pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;
							float length_in_mile =  pLink->DefaultDistance();  // cooridnates have been changed to mile for unit

							pLink->m_bToBeShifted = bToBeShifted; 

							pLink->m_NumLanes= number_of_lanes;
							pLink->m_SpeedLimit= max(20,speed_limit_in_mph);  // minimum speed limit is 20 mph
							pLink->m_StaticSpeed = pLink->m_SpeedLimit;

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

							m_LinkNotoLinkMap[i] = pLink;

							m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);


							default_distance_sum+= pLink->DefaultDistance();
							length_sum += pLink ->m_Length;
							//			pLink->SetupMOE();

								GDPoint	pt;
								pt.x = pLink->m_FromPoint.x;
								pt.y = pLink->m_FromPoint.y;
								pLink->m_ShapePoints .push_back (pt);

								pt.x = pLink->m_ToPoint.x;
								pt.y = pLink->m_ToPoint.y;
								pLink->m_ShapePoints .push_back (pt);

							TRACE("\nAdd link no.%d,  %d -> %d",i,pLink->m_FromNodeNumber, pLink->m_ToNodeNumber );
							m_LinkSet.push_back (pLink);
							m_LinkNoMap[i]  = pLink;
							i++;
						}  // per major approach

					} // for each movement

					DTANode* pNode = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]];	

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

					element.in_link_from_node_id = m_NodeNametoIDMap[from_node_id];		
					element.in_link_to_node_id = m_NodeNametoIDMap[to_node_id];						
					element.out_link_to_node_id = m_NodeNametoIDMap[dest_node_id];	

					DTALink* pIncomingLink =  FindLinkWithNodeIDs(m_NodeNametoIDMap[from_node_id],m_NodeNametoIDMap[to_node_id]);

					if(pIncomingLink)
						element.IncomingLinkID = pIncomingLink->m_LinkNo  ;

					DTALink* pOutcomingLink =  FindLinkWithNodeIDs(m_NodeNametoIDMap[to_node_id],m_NodeNametoIDMap[dest_node_id]);

					if(pOutcomingLink)
						element.OutgoingLinkID = pOutcomingLink->m_LinkNo ;


					GDPoint p1, p2, p3;
					p1  = m_NodeIDMap[element.in_link_from_node_id]->pt;
					p2  = m_NodeIDMap[element.in_link_to_node_id]->pt;
					p3  = m_NodeIDMap[element.out_link_to_node_id]->pt;

					element.movement_approach = g_Angle_to_Approach_New(Find_P2P_Angle(p1,p2));
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


					DTANode* pNode = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]];	

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

		//ConstructMovementVectorForEachNode();
//		GenerateOffsetLinkBand();
	return 1;
}


BOOL CTLiteDoc::ImportingTransportationPlanningDataSet(CString ProjectFileName, bool bNetworkOnly, bool bImportShapeFiles)
{

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

	m_ControlType_UnknownControl = g_GetPrivateProfileInt("control_type","unknown_control",0,ProjectFileName);
	m_ControlType_NoControl = g_GetPrivateProfileInt("control_type","no_control",1,ProjectFileName);
	m_ControlType_YieldSign = g_GetPrivateProfileInt("control_type","yield_sign",2,ProjectFileName);
	m_ControlType_2wayStopSign = g_GetPrivateProfileInt("control_type","2way_stop_sign",3,ProjectFileName);
	m_ControlType_4wayStopSign = g_GetPrivateProfileInt("control_type","4way_stop_sign",4,ProjectFileName);
	m_ControlType_PretimedSignal = g_GetPrivateProfileInt("control_type","pretimed_signal",5,ProjectFileName);
	m_ControlType_actuatedSignal = g_GetPrivateProfileInt("control_type","actuated_signal",6,ProjectFileName);
	m_ControlType_Roundabout = g_GetPrivateProfileInt("control_type","roundabout",7,ProjectFileName);


	char link_type_file_name[_MAX_STRING_SIZE];
	g_GetProfileString("default_data_tables","link_type_file_name","input_link_type.csv",link_type_file_name,sizeof(link_type_file_name),ProjectFileName);

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	CString DefaultDataFolder;
	DefaultDataFolder.Format ("%s\\default_data_folder\\",pMainFrame->m_CurrentDirectory);


	if(ReadLinkTypeCSVFile(directory+link_type_file_name) == false)
	{
	// read files from default fold
		ReadLinkTypeCSVFile(DefaultDataFolder+link_type_file_name);
	}

	char pricing_type_file_name[_MAX_STRING_SIZE];
	g_GetProfileString("default_data_tables","pricing_type_file_name","input_pricing_type.csv",pricing_type_file_name,sizeof(pricing_type_file_name),ProjectFileName);


	char VOT_file_name[_MAX_STRING_SIZE];
	g_GetProfileString("default_data_tables","value_of_time_file_name","input_VOT.csv",VOT_file_name,sizeof(VOT_file_name),ProjectFileName);
	if(ReadVOTCSVFile(directory+VOT_file_name)==false)
	{
	ReadVOTCSVFile(DefaultDataFolder+VOT_file_name);
	}

	char vehicle_type_file_name[_MAX_STRING_SIZE];
	g_GetProfileString("default_data_tables","vehicle_type_file_name ","input_vehicle_type.csv",vehicle_type_file_name,sizeof(vehicle_type_file_name),ProjectFileName);
	if(ReadVehicleTypeCSVFile(directory+vehicle_type_file_name)==false)
	{
		ReadVehicleTypeCSVFile(DefaultDataFolder+vehicle_type_file_name);
	}

	char demand_type_file_name[_MAX_STRING_SIZE];
	g_GetProfileString("default_data_tables","demand_type_file_name","input_demand_type.csv",demand_type_file_name,sizeof(demand_type_file_name),ProjectFileName);
	if(ReadDemandTypeCSVFile(directory+demand_type_file_name)==false)
	{
	ReadDemandTypeCSVFile(DefaultDataFolder+demand_type_file_name);
	}

	char emission_rate_file_name[_MAX_STRING_SIZE];
	g_GetProfileString("default_data_tables","vehicle_emission_rate_file_name ","input_vehicle_emission_rate.csv",emission_rate_file_name,sizeof(emission_rate_file_name),ProjectFileName);
	if(ReadInputEmissionRateFile(directory+emission_rate_file_name) == false)
	{
	ReadInputEmissionRateFile(DefaultDataFolder+emission_rate_file_name);
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