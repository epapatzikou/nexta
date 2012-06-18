#include "stdafx.h"
#include "Geometry.h"
#include "TLite.h"
#include "MainFrm.h"
#include "Shellapi.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "Data-Interface//include//ogrsf_frmts.h"



float ComputeCapacity(float capacity_in_pcphpl,int link_capacity_flag, float speed_limit_in_mph, int number_of_lanes)
{

	if(link_capacity_flag == 1)
		capacity_in_pcphpl = capacity_in_pcphpl/max(1,number_of_lanes);

	if(capacity_in_pcphpl ==0)
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

	return 0;
}
BOOL CTLiteDoc::OnOpenAMSDocument(CString FileName)
{
 
	char model_units[_MAX_STRING_SIZE];
    GetPrivateProfileString("model_attributes","units","MI",model_units,sizeof(model_units),FileName);

	bool bMileFlag = false;
	if(strcmp(model_units,"MI")== 0 )
		bMileFlag = true;

	int direction_field_flag = g_GetPrivateProfileInt("model_attributes","direction_field",0,FileName);
	int control_type_field_flag = g_GetPrivateProfileInt("model_attributes","control_type_field",0,FileName);
	int reserve_direction_field_flag = g_GetPrivateProfileInt("model_attributes","reserve_direction_field",0,FileName);
	int offset_link_flag = g_GetPrivateProfileInt("model_attributes","offset_link",1,FileName);
	int link_capacity_flag = g_GetPrivateProfileInt("model_attributes","lane_capapcity",1,FileName);
	int number_of_lanes_for_two_way_links_flag = g_GetPrivateProfileInt("model_attributes","number_of_lanes_for_two_way_links",0,FileName);


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

						x =   poPoint->getX();
						y  = poPoint->getY();
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
	
	// ************************************/
	// 2: link table

	// ************************************/
	bool read_link_layer = true;

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
	if(reserve_direction_field_flag)
	{
    GetPrivateProfileString("link_table","r_number_of_lanes","number_of_lanes",r_number_of_lanes_name,sizeof(r_number_of_lanes_name),FileName);
    GetPrivateProfileString("link_table","r_lane_capacity_in_vhc_per_hour","lane_capacity_in_vhc_per_hour",r_lane_capacity_in_vhc_per_hour_name,sizeof(r_lane_capacity_in_vhc_per_hour_name),FileName);
    GetPrivateProfileString("link_table","r_speed_limit_in_mph","speed_limit_in_mph",r_speed_limit_in_mph_name,sizeof(r_speed_limit_in_mph_name),FileName);
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

			float default_distance_sum=0;
			float length_sum = 0;

			bool bTwoWayLinkFlag = false;

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

				long link_id =  poFeature->GetFieldAsInteger(link_id_name);
				CString name =  poFeature->GetFieldAsString(link_name);
				int type = poFeature->GetFieldAsInteger(link_type_name);
				int direction = 1;
				if(direction_field_flag) 
					direction = poFeature->GetFieldAsInteger(direction_name);

				float length = poFeature->GetFieldAsDouble(length_name);

				int number_of_lanes = poFeature->GetFieldAsInteger(number_of_lanes_name);

				if(direction_field_flag == 1 && (direction==0 || direction==2) && number_of_lanes_for_two_way_links_flag ==1 )
				{
				number_of_lanes = number_of_lanes/2;
				}

				float speed_limit_in_mph= poFeature->GetFieldAsDouble(speed_limit_in_mph_name);
				float capacity_in_pcphpl= poFeature->GetFieldAsDouble(lane_capacity_in_vhc_per_hour_name);

				capacity_in_pcphpl = ComputeCapacity(capacity_in_pcphpl,link_capacity_flag, speed_limit_in_mph,number_of_lanes);
				int r_number_of_lanes; 
				float r_speed_limit_in_mph; 
				float r_capacity_in_pcphpl; 

				if(reserve_direction_field_flag)
				{
				r_number_of_lanes = poFeature->GetFieldAsInteger(r_number_of_lanes_name);
				if(direction_field_flag == 1 && (direction==0 || direction==2) && number_of_lanes_for_two_way_links_flag ==1)
				{
				number_of_lanes = r_number_of_lanes/2;
				}

				r_speed_limit_in_mph= poFeature->GetFieldAsDouble(r_speed_limit_in_mph_name);
				r_capacity_in_pcphpl= poFeature->GetFieldAsDouble(r_lane_capacity_in_vhc_per_hour_name);
				r_capacity_in_pcphpl = ComputeCapacity(r_capacity_in_pcphpl,link_capacity_flag, r_speed_limit_in_mph,number_of_lanes);

				}

				m_AMSLogFile << from_node_id << "," << to_node_id << "," << link_id << "," << name << "," << type << "," << direction << ",";
				m_AMSLogFile << length << "," << number_of_lanes << "," << speed_limit_in_mph << ","  << capacity_in_pcphpl << ",";

				if(reserve_direction_field_flag)
				{
				m_AMSLogFile << r_number_of_lanes << "," << r_speed_limit_in_mph << ","  << r_capacity_in_pcphpl << ",";
				
				}


				float grade = 0;
				float AADT_conversion_factor = 0.1;
				float k_jam, wave_speed_in_mph;
				if(m_LinkTypeFreewayMap.find(type)!=m_LinkTypeFreewayMap.end())  // freeway link
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
						for(unsigned int si = 0; si< poLine->getNumPoints(); si++)
						{
							CCoordinate pt;
							pt.X   =  poLine->getX(si);
							pt.Y =  poLine->getY(si);
							CoordinateVector.push_back(pt);
							m_AMSLogFile << pt.X << ";" << pt.Y << " ";

						}
						m_AMSLogFile << "}" ;

					}
				}


				if(from_node_id==0 && to_node_id ==0)  // test twice here for from and to nodes
				{
					AfxMessageBox("Invalid data: from_node_id==0 && to_node_id ==0 in the link table.");
					return false;
				}

/*		error checking
DTALink* pExistingLink =  FindLinkWithNodeIDs(m_NodeNametoIDMap[from_node_id],m_NodeNametoIDMap[to_node_id]);

				if(pExistingLink)
				{
					// str_msg.Format ("Link %d-> %d at row %d is duplicated with the previous link at row %d.\n", from_node_id,to_node_id, line_no, pExistingLink->input_line_no);
						continue;
				}
				if(length > 100)
				{
					//AMSlog("The length of link %d -> %d is longer than 100 miles, please ensure the unit of link length in the link sheet is mile.",from_node_id,to_node_id);
					return false;
				}
				if(number_of_lanes ==0)
				{
					//AMS << str_msg.Format ("Link %d -> %d has 0 lane. Skip.",from_node_id,to_node_id);
					continue; 
				}
				if(speed_limit_in_mph ==0)
				{
					// AMS	str_msg.Format ("Link %d -> %d has a speed limit of 0. Skip.",from_node_id,to_node_id);

					continue; 
				}
				if(capacity_in_pcphpl<0)
				{
					//AMS		rsLink.Close();
					return;
				}
*/

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

					if(length < 0.00001) // zero value in length field, we consider no length info.
					{
						float distance_in_mile = g_CalculateP2PDistanceInMileFromLatitudeLongitude(pLink->m_ShapePoints[0], pLink->m_ShapePoints[pLink->m_ShapePoints.size()-1]);
						pLink->m_Length = distance_in_mile;
					}

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

					{  // reset default value
					if(pLink->m_SpeedLimit<=50 && m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType == 0)
					{
						m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType = 3;  // signal control
					}
					
					}

					if(m_LinkTypeConnectorMap[type ]==1) // adjacent node of connectors
					{ 
						// mark them as activity location 
					m_NodeIDMap[pLink->m_FromNodeID ]->m_bZoneActivityLocationFlag = true;					
					m_NodeIDMap[pLink->m_ToNodeID ]->m_bZoneActivityLocationFlag = true;					
					}


					m_NodeIDMap[pLink->m_FromNodeID ]->m_OutgoingLinkVector.push_back(i);


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

		ConstructMovementVectorForEachNode();
		AssignUniqueLinkIDForEachLink();

		if(bTwoWayLinkFlag == true)
			m_bLinkToBeShifted = true;

		GenerateOffsetLinkBand();

		}  // layer
		// to do: # of nodes: control: two-way stop signs....

	OGRDataSource::DestroyDataSource( poDS );

			m_UnitMile  = 1.0f;
			if(length_sum>0.000001f)
				m_UnitMile=  default_distance_sum /length_sum;

			m_UnitFeet = m_UnitMile/5280.0f;  

			if(offset_link_flag)
			OffsetLink();

	}

	// ************************************/
	// 3: zone table
	// ************************************/
	char zone_table_file_name[_MAX_STRING_SIZE];
    GetPrivateProfileString("zone_table","reference_file_name","",zone_table_file_name,sizeof(zone_table_file_name),FileName);
	char zone_id_name[_MAX_STRING_SIZE];
    GetPrivateProfileString("zone_table","zone_id","ID",zone_id_name,sizeof(zone_id_name),FileName);

	CString zone_shape_file_name;
			zone_shape_file_name = m_ProjectDirectory + zone_table_file_name;

			poDS = OGRSFDriverRegistrar::Open(zone_table_file_name, FALSE );
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
							pt.x = point.getX();
							pt.y =point.getY();
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

		}
	

	return true;
}

bool CTLiteDoc::ReadTransCADDemandCSVFile(LPCTSTR lpszFileName)
{

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
			m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file %s.",line_no,lpszFileName);
			return true;
	}else
	{
		//		AfxMessageBox("Error: File input_demand.csv cannot be found or opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

}
