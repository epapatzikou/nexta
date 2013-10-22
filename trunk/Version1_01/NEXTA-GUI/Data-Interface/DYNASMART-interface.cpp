//Portions Copyright 2012 Xuesong Zhou.
//

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
#include "..//Geometry.h"
#include "..//TLite.h"
#include "..//Network.h"
#include "..//TLiteDoc.h"
#include "..//Geometry.h"
#include "..//CSVParser.h"
#include <iostream>                          // for cout, endl
#include <fstream>                           // for ofstream
#include <sstream>
#include "..//Dlg_UserInput.h"
using namespace std;

//#include "DYNASMART-interace.h"

// add description for DYNASMART
// Important References:
// Jayakrishnan, R., Mahmassani, H. S., and Hu, T.-Y., 1994a. An evaluation tool for advanced traffic information and management systems in urban networks. Transportation Research Part C, Vol. 2, No. 3, pp. 129-147.
// 
std::map<int, int> NodeIDtoZoneNameMap;


bool CTLiteDoc::ReadGPSData(string FileName)
{
	CString directory;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

	m_ODSize  = 0;
	int good_vehicle_id = 1;

	CCSVParser parser;

	string str0 = FileName;
	CT2CA pszConvertedAnsiString (str0.c_str());
	// construct a std::string using the LPCSTR input
	std::string  strStd (pszConvertedAnsiString);

	if (parser.OpenCSVFile(strStd))
	{
		int i=0;
		while(parser.ReadRecord())
		{

			string Trajectory_ID, Trajectory_Start_Time;
			int Probe_ID,Origin_Area_ID,Destination_Area_ID;
			float Trajectory_Time,Trajectory_Length;

			if(parser.GetValueByFieldName("Trajectory ID",Trajectory_ID) == false)
				break;

			parser.GetValueByFieldName("Probe ID",Probe_ID);
			parser.GetValueByFieldName("Trajectory Time (s)",Trajectory_Time);
			parser.GetValueByFieldName("Trajectory Length (m)",Trajectory_Length);
			parser.GetValueByFieldName("Origin Area ID",Origin_Area_ID);
			parser.GetValueByFieldName("Destination Area ID",Destination_Area_ID);
			parser.GetValueByFieldName("Trajectory Start Time",Trajectory_Start_Time);

			int m_VehicleID= Probe_ID;
			//Trajectory ID,Probe ID,Trajectory Time (s),Trajectory Length (m),Origin Area ID,Destination Area ID,Trajectory Start Time

			//		64555_1,64555,2630.78,32963.56,468,579,Sat May 01 18:01:19 2010
			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_VehicleID		= m_VehicleID;

			int NodeSizeOffset = 0;
			pVehicle->m_bComplete = true;

			pVehicle->m_OriginZoneID	= Origin_Area_ID;
			pVehicle->m_DestinationZoneID = Destination_Area_ID;

			if( pVehicle->m_OriginZoneID > m_ODSize )
				m_ODSize = pVehicle->m_OriginZoneID ;

			if( pVehicle->m_DestinationZoneID > m_ODSize )
				m_ODSize = pVehicle->m_DestinationZoneID ;

			pVehicle->m_InformationClass = 0;

			pVehicle->m_DepartureTime	= m_SimulationStartTime_in_min ;

			if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
				g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

			// Total Travel Time=   0.34 # of Nodes=   2 VehType 1 LOO 1
			pVehicle->m_TripTime  = Trajectory_Time;
			pVehicle->m_ArrivalTime = pVehicle->m_DepartureTime + pVehicle->m_TripTime;

			pVehicle->m_NodeSize	= 0;
			pVehicle->m_DemandType = 1;
			pVehicle->m_PricingType = 1;

			pVehicle->m_VOT = 10;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions = 0;
			pVehicle->m_Distance = Trajectory_Length/5280;

			//pVehicle->m_Distance = 0;

			//pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

			//pVehicle->m_NodeNumberSum = 0;

			//m_PathNodeVectorSP[0] =  FirstNodeID;

			//int i;
			//for(i=1; i< pVehicle->m_NodeSize; i++)
			//{
			//	m_PathNodeVectorSP[i] = g_read_integer(pFile);

			//	pVehicle->m_NodeNumberSum += m_PathNodeVectorSP[i];
			//	DTALink* pLink = FindLinkWithNodeNumbers(m_PathNodeVectorSP[i-1],m_PathNodeVectorSP[i],directory+"VehTrajectory.dat");
			//	if(pLink==NULL)
			//	{
			//		CString str;
			//		str.Format ("Error in reading file Vehicle.csv, good vehicle id: %d",good_vehicle_id);
			//		AfxMessageBox(str);
			//		fclose(pFile);

			//		return false;
			//	}
			//	pVehicle->m_NodeAry[i].LinkNo  = pLink->m_LinkNo ;
			//	
			//	pLink->m_total_link_volume +=1;
			//}


			//// ==>Node Exit Time Point
			//for(i=1; i< pVehicle->m_NodeSize + NodeSizeOffset; i++)
			//{
			//	pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = m_SimulationStartTime_in_min + g_read_float(pFile);
			//}


			//// ==>Link Travel Time
			//for(i=1; i< pVehicle->m_NodeSize + NodeSizeOffset; i++)
			//{
			//	g_read_float(pFile);  // // travel time
			//}
			//// ==>Accumulated Stop Time
			//for(i=1; i< pVehicle->m_NodeSize +NodeSizeOffset; i++)
			//{
			//	g_read_float(pFile);  // stop time
			//}

			m_VehicleSet.push_back (pVehicle);
			m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;

			good_vehicle_id = m_VehicleID;
		}

	}

	return true;
}
BOOL CTLiteDoc::OnOpenDYNASMARTProject(CString ProjectFileName, bool bNetworkOnly)
{
	m_OriginOnBottomFlag = -1;
	CTime LoadingStartTime = CTime::GetCurrentTime();

	m_bLoadNetworkDataOnly = bNetworkOnly;

	m_bDYNASMARTDataSet = true;

	m_YCorridonateFlag = 1;

	FILE* pProjectFile = NULL;


	fopen_s(&pProjectFile,ProjectFileName,"r");
	if(pProjectFile!=NULL)
	{
		char  str_line[2000]; // input string
		int str_line_size;
		g_read_a_line(pProjectFile,str_line, str_line_size); //  skip the first line
		g_read_a_line(pProjectFile,str_line, str_line_size); //  skip the first line

		CString str_origin;
		str_origin.Format("%s", str_line,str_line_size);

		if(str_origin.Find("top") != -1 || str_origin.Find("TOP") != -1)
		{
			m_YCorridonateFlag = -1;

		}

		if(str_origin.Find("bottom") != -1 || str_origin.Find("BOTTOM") != -1)
		{
			m_YCorridonateFlag = 1;

		}

		fclose(pProjectFile);
	}


	int bRead_long_lat_coordinate_flag = (int)(g_GetPrivateProfileDouble("coordinate", "read_AMS_files",0,ProjectFileName));	

	CString directory;
	m_ProjectFile = ProjectFileName;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

	SetCurrentDirectory(directory);
	m_ProjectDirectory = directory;
	m_ProjectTitle = GetWorkspaceTitleName(ProjectFileName);
	SetTitle(m_ProjectTitle);

	FILE* pFile = NULL;

	FILE* pFileNodeXY = NULL;

	fopen_s(&pFileNodeXY,directory+"xy.dat","r");
	if(pFileNodeXY==NULL)
	{
		AfxMessageBox("File xy.dat cannot be opened.");
		return false;
	}

	int num_zones = 0;

	fopen_s(&pFile,directory+"network.dat","r");

	if(pFile!=NULL)
	{
		num_zones = g_read_integer(pFile);

		m_ODSize = num_zones;
		int num_nodes= g_read_integer(pFile);
		int num_links = g_read_integer(pFile);

		int KSPNumShortestPaths = g_read_integer(pFile);
		int UseSuperzonesFlag = g_read_integer(pFile);

		// Read node block
		int i;
		int node_id = 0, zoneNum = 0;
		DTANode* pNode = 0;
		for(i = 0; i < num_nodes; i++)
		{
			node_id			= g_read_integer(pFile);
			zoneNum	= g_read_integer(pFile);

			m_NodeNotoZoneNameMap[node_id] = zoneNum;
			NodeIDtoZoneNameMap[node_id] = zoneNum;

			// Create and insert the node
			pNode = new DTANode;
			std::stringstream streams;
			streams << node_id;

			pNode->m_Name = streams.str();
			pNode->m_ControlType = 0;

			// read xy.dat	
			int node_number = g_read_integer(pFileNodeXY);  //skip node number in xy.dta
			pNode->pt.x = g_read_float(pFileNodeXY);
			pNode->pt.y = g_read_float(pFileNodeXY)*m_YCorridonateFlag;

			pNode->m_NodeNumber = node_id;
			pNode->m_NodeNo = i;
			pNode->m_ZoneID = 0;
			m_NodeSet.push_back(pNode);
			m_NodeNoMap[i] = pNode;
			m_NodeNotoNumberMap[i] = node_id;
			m_NodeNumberMap[node_id] = pNode;
			m_NodeNumbertoNodeNoMap[node_id] = i;
		}

		m_NodeDataLoadingStatus.Format ("%d nodes are loaded.",m_NodeSet.size());


		// Read link block
		DTALink* pLink = 0;

		float default_distance_sum=0;
		float length_sum = 0;

		for(i = 0; i < num_links; i++)
		{
			int SimulationHorizon = 1;
			pLink = new DTALink(SimulationHorizon);
			pLink->m_LayerNo = 0;
			pLink->m_LinkNo = i;
			//			pLink->m_Name  = name;
			pLink->m_OrgDir = 1;
			pLink->m_LinkID = i;

			pLink->m_FromNodeNumber = g_read_integer(pFile);
			pLink->m_ToNodeNumber = g_read_integer(pFile);
			pLink->m_FromNodeID = m_NodeNumbertoNodeNoMap[pLink->m_FromNodeNumber];
			pLink->m_ToNodeID= m_NodeNumbertoNodeNoMap[pLink->m_ToNodeNumber];

			int m_LeftBays= g_read_integer(pFile);

			pLink->m_NumberOfLeftTurnLanes = m_LeftBays;
			int m_RightBays= g_read_integer(pFile);
			pLink->m_NumberOfRightTurnLanes = m_RightBays;

			pLink->m_Length = g_read_float(pFile)/5280;   // 5280 feet per mile, NEXTA use mile as default length unit
			pLink->m_NumberOfLanes= g_read_integer(pFile);

			int m_FlowModel= g_read_integer(pFile);
			int m_SpeedAdjustment= g_read_integer(pFile);
			pLink->m_SpeedLimit = g_read_integer(pFile);
			pLink->m_avg_simulated_speed = pLink->m_SpeedLimit;
			pLink->m_FreeFlowTravelTime = pLink->m_Length*60 / max(1,pLink->m_SpeedLimit);  // * 60: hour -> min

			pLink->m_MaximumServiceFlowRatePHPL= g_read_float(pFile);
			pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;

			int m_SaturationFlowRate= g_read_integer(pFile);

			pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane = m_SaturationFlowRate;

			int DSP_link_type = g_read_integer(pFile);
			pLink->m_link_type = DSP_link_type;


			//if(DSP_link_type == 3 || DSP_link_type == 4) 
			//	pLink->m_LaneCapacity  = 1400;

			//if(DSP_link_type == 5) 
			//	pLink->m_LaneCapacity  = 1000;

			int m_grade= g_read_integer(pFile);

			m_NodeNoMap[pLink->m_FromNodeID ]->m_Connections+=1;
			m_NodeNoMap[pLink->m_FromNodeID ]->m_OutgoingLinkVector.push_back(pLink->m_LinkNo);

			m_NodeNoMap[pLink->m_ToNodeID ]->m_IncomingLinkVector.push_back(pLink->m_LinkNo);

			m_NodeNoMap[pLink->m_ToNodeID ]->m_Connections+=1;

			unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
			m_NodeNotoLinkMap[LinkKey] = pLink;
			m_LinkNotoLinkMap[i] = pLink;

			m_NodeNoMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumberOfLanes);

			pLink->m_FromPoint = m_NodeNoMap[pLink->m_FromNodeID]->pt;
			pLink->m_ToPoint = m_NodeNoMap[pLink->m_ToNodeID]->pt;
			default_distance_sum+= pLink->DefaultDistance();
			length_sum += pLink ->m_Length;
			//			pLink->SetupMOE();


			std::vector<CCoordinate> CoordinateVector;
			// no geometry information
			CCoordinate cc_from, cc_to; 
			cc_from.X = pLink->m_FromPoint.x;
			cc_from.Y = pLink->m_FromPoint.y;

			cc_to.X = pLink->m_ToPoint.x;
			cc_to.Y = pLink->m_ToPoint.y;

			CoordinateVector.push_back(cc_from);
			CoordinateVector.push_back(cc_to);
			m_bLinkToBeShifted = true;

			int si;
			for(si = 0; si < CoordinateVector.size(); si++)
			{
				GDPoint	pt;
				pt.x = CoordinateVector[si].X;
				pt.y = CoordinateVector[si].Y;
				pLink->m_ShapePoints .push_back (pt);

			}

			pLink->m_Original_ShapePoints = pLink->m_ShapePoints ;


			TRACE("\nAdd link no.%d,  %d -> %d",i,pLink->m_FromNodeNumber, pLink->m_ToNodeNumber );
			m_LinkSet.push_back (pLink);
			m_LinkNoMap[i]  = pLink;
		}

		if(length_sum>0.000001f)
			m_UnitMile= default_distance_sum / length_sum ;

		m_UnitFeet = m_UnitMile/5280.0f;  

		m_LinkDataLoadingStatus.Format ("%d links are loaded.",m_LinkSet.size());
		ConstructMovementVector();



		fclose(pFile);
		fclose(pFileNodeXY);
	}else
	{
		AfxMessageBox("Error: File network.dat cannot be opened.");
		return false;
		//		g_ProgramStop();
	}


	FILE *st;
	fopen_s(&st,directory+"workzone.dat","r");
	if(st!=NULL)
	{
		int total_number = g_read_integer(st);
		int i= 0 ;
		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn,directory+"workzone.dat" );

			if(plink!=NULL)
			{
				CapacityReduction cs;
				cs.bWorkzone  = true; 

				cs.StartDayNo = 0;
				cs.EndDayNo = 100;
				cs.StartTime = g_read_float(st);
				cs.EndTime = g_read_float(st);
				cs.LaneClosurePercentage= g_read_float(st)*100;  //DYNASMART-P use ratio, DTALite use percentage
				cs.SpeedLimit = g_read_float(st);
				g_read_float(st);

				plink->CapacityReductionVector.push_back(cs);
				i++;
			}
		}
		fclose(st);
	}

	ResetODMOEMatrix();
	ReadSensorData();

	// read link xy data
	int ReadLinkXYFile = g_GetPrivateProfileInt("display","read_link_x_y",1,ProjectFileName);

	if(ReadLinkXYFile) 
	{
		FILE* pFileLinkXY = NULL;
		fopen_s(&pFileLinkXY,directory+"linkxy.dat","r");
		if(pFileLinkXY!=NULL)
		{

			m_bBezierCurveFlag = false; // do not apply bezier curve fitting 

			while (!feof(pFileLinkXY))
			{
				int from_node = g_read_integer(pFileLinkXY);

				if(from_node == -1)
					break; // ne
				int to_node = g_read_integer(pFileLinkXY);
				int number_of_shape_points = g_read_integer(pFileLinkXY);

				DTALink* pLink = FindLinkWithNodeNumbers(from_node, to_node,directory+"linkxy.dat");

				if(pLink != NULL)
				{
					pLink->m_ShapePoints.clear();  // clear up first

					for(int i = 0; i < number_of_shape_points; i++)
					{
						GDPoint	pt;
						pt.x =  g_read_float(pFileLinkXY);
						pt.y = g_read_float(pFileLinkXY)*m_YCorridonateFlag;
						pLink->m_ShapePoints .push_back (pt);
					}

					pLink->m_Original_ShapePoints = pLink->m_ShapePoints ;

					if(number_of_shape_points>=4)
						pLink->m_bToBeShifted  = false;

				}else
				{
					break;
				}
			}
			fclose(pFileLinkXY);
		}  // end of reading link xy 
	}

	// test

	//read zone.dat
	FILE* pZoneXY = NULL;
	fopen_s(&pZoneXY,directory+"zone.dat","r");
	if(pZoneXY!=NULL)
	{
		int number_of_feature_points = g_read_integer(pZoneXY);
		int number_of_zones = g_read_integer(pZoneXY);

		std::map<int, GDPoint> FeaturePointMap;

		for(int f = 0; f< number_of_feature_points; f++)
		{
			int feature_point_id = g_read_integer(pZoneXY);

			FeaturePointMap[feature_point_id].x = g_read_float(pZoneXY);
			FeaturePointMap[feature_point_id].y = g_read_float(pZoneXY)*m_YCorridonateFlag;
		}

		for(int z = 0; z< number_of_zones; z++)
		{
			int zone_id = g_read_integer(pZoneXY);

			if(zone_id==-1)   // end of linke
				break;
			m_ZoneMap [zone_id].m_ZoneID = zone_id;

			m_ZoneMap [zone_id].m_ShapePoints .clear();

			int number_of_feature_points = g_read_integer(pZoneXY);
			if(number_of_feature_points==-1)   // end of linke
				break;

			for(int f = 0; f < number_of_feature_points; f++)
			{
				int feature_point_id = g_read_integer(pZoneXY);
				if(feature_point_id==-1)   // end of linke
					break;

				if(FeaturePointMap.find(feature_point_id)!= FeaturePointMap.end())
				{
					GDPoint pt = FeaturePointMap[feature_point_id];
					m_ZoneMap [zone_id].m_ShapePoints.push_back (pt);
				}
			}

		}
		m_ZoneDataLoadingStatus.Format ("%d zone info records are loaded.",m_ZoneMap.size());

		fclose(pZoneXY);
	}





	// read destination.dat

	fopen_s(&pFile,directory+"destination.dat","r");
	if(pFile!=NULL)
	{
		for(int z=0; z< num_zones; z++)
		{
			int zone_number = g_read_integer(pFile);

			int num_destinations= g_read_integer(pFile);

			for(int dest = 0;  dest < num_destinations; dest++)
			{
				int destination_node = g_read_integer(pFile);

				map <int, int> :: iterator m_Iter = m_NodeNumbertoNodeNoMap.find(destination_node);

				if(m_Iter == m_NodeNumbertoNodeNoMap.end( ))
				{
					m_WarningFile<< "Node Number "  << destination_node << " in destination.dat has not been defined in network.csv"  << endl; 
					fclose(pFile);
					return false;
				}

				int node_id  = m_NodeNumbertoNodeNoMap[destination_node];
				m_NodeNotoZoneNameMap[node_id] = zone_number;
				NodeIDtoZoneNameMap[node_id] = zone_number;
				m_NodeNoMap [node_id ] -> m_ZoneID = zone_number;

				m_NodeNoMap[node_id]->m_ZoneID = zone_number;

				// if there are multiple nodes for a zone, the last node id is recorded.
				if(m_ZoneMap [zone_number].FindANode(node_id,-1) == false)
				{

					DTAActivityLocation element;
					element.ZoneID  = zone_number;
					
					if(m_NodeNoMap[node_id]->m_IncomingLinkVector .size() > 0 || m_NodeNoMap[node_id]->m_OutgoingLinkVector .size() >0)
					{
					
					element.NodeNumber = destination_node;



					if(destination_node == 3688)
						TRACE("");

					element.External_OD_flag = 0;

					if(m_NodeNoMap[node_id]->m_IncomingLinkVector .size()  ==0)
					element.External_OD_flag = 1;

					if(m_NodeNoMap[node_id]->m_OutgoingLinkVector .size()  ==0)
					element.External_OD_flag = -1;
		

					element.pt = m_NodeNoMap [node_id ]  ->pt;


					m_ZoneMap [zone_number].m_ZoneID = zone_number;
					m_ZoneMap [zone_number].m_ActivityLocationVector .push_back (element);
					}
				}
			}


		}

		fclose(pFile);
	}

	// read destination.dat

	fopen_s(&pFile,directory+"origin.dat","r");
	if(pFile!=NULL)
	{
		for(int z=0; z< num_zones; z++)
		{
			int zone_number = g_read_integer(pFile);

			int num_of_origins= g_read_integer(pFile);

			int number= g_read_float(pFile);

			for(int origin = 0;  origin < num_of_origins; origin++)
			{
				int from_node = g_read_integer(pFile);
				int to_node = g_read_integer(pFile);

				int node_id  = m_NodeNumbertoNodeNoMap[to_node];
				m_NodeNotoZoneNameMap[node_id] = zone_number;
				m_NodeNoMap[node_id]->m_ZoneID = zone_number;

				// if there are multiple nodes for a zone, the last node id is recorded.

				if(m_ZoneMap [zone_number].FindANode(to_node,1) == false)
				{
					DTAActivityLocation element;
					element.ZoneID  = zone_number;
					element.NodeNumber = to_node;
					element.External_OD_flag = 0;

					m_ZoneMap [zone_number].m_ActivityLocationVector .push_back (element);
				}

				float loading_ratio = g_read_float(pFile);

				map <int, int> :: iterator m_Iter = m_NodeNumbertoNodeNoMap.find(to_node);

				if(m_Iter == m_NodeNumbertoNodeNoMap.end( ))
				{
					m_WarningFile<< "Node Number "  << to_node << " in origin.dat has not been defined in network.dat"  << endl; 
					fclose(pFile);
					return false;
				}



			}

		}

		fclose(pFile);
	}

	//	ReadZoneCSVFile(directory+"input_zone.csv");

	DTADemandType demand_type_element;
	demand_type_element.demand_type =1;
	demand_type_element.demand_type_name = "SOV";
	demand_type_element.pricing_type = 1;
	demand_type_element.average_VOT = 10;
	m_DemandTypeVector.push_back(demand_type_element);

	demand_type_element.demand_type =2;
	demand_type_element.pricing_type = 2;
	demand_type_element.demand_type_name = "HOV";
	demand_type_element.average_VOT = 10;
	m_DemandTypeVector.push_back(demand_type_element);

	demand_type_element.demand_type =3;
	demand_type_element.pricing_type = 3;
	demand_type_element.demand_type_name = "truck";
	m_DemandTypeVector.push_back(demand_type_element);



	int ReadDemandFile = 1;

	fopen_s(&pFile,directory+"demand.dat","rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);
		float LengthinMB= Length*1.0/1024/1024;
		if(LengthinMB>20)
		{
			CString msg;
			msg.Format("The demand.dat file is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load this file?",LengthinMB);
			if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
			{
				ReadDemandFile = 0;
			}
		}

		fclose(pFile);
	}

	if(ReadDemandFile)
	{
		// read demand.dat
		fopen_s(&pFile,directory+"demand.dat","r");
		if(pFile!=NULL)
		{
			// Number of matrices and the multiplication factor
			int num_matrices = 0;

			num_matrices = g_read_integer(pFile);
			float demand_factor = g_read_float(pFile);

			std::vector<int> TimeIntevalVector;
			// Start times
			int i;
			for(i = 0; i < num_matrices; i++)
			{
				int start_time = g_read_float(pFile);
				TimeIntevalVector.push_back(start_time);

			}

			int time_interval = 60; // min

			if(TimeIntevalVector.size() >=2)
				time_interval = TimeIntevalVector[1] - TimeIntevalVector[0];

			// read the last value
			int end_of_simulation_horizon = g_read_float(pFile);

			TimeIntevalVector.push_back(end_of_simulation_horizon);
			m_PeakHourFactor = 60.0/max(1,end_of_simulation_horizon);

			long RecordCount = 0;
			float total_demand = 0;
			for(i = 0; i < num_matrices; i++)
			{
				// Find a line with non-blank values to start
				// Origins
				double start_time= g_read_float(pFile); // start time

				int demand_table_type = GetDemandTableType(1,start_time,start_time+time_interval);
				for(int from_zone=1; from_zone<= num_zones; from_zone++)
					for(int to_zone=1; to_zone<= num_zones; to_zone++)
					{
						float demand_value = g_read_float(pFile) * demand_factor;
						total_demand += demand_value;

						DTADemandVolume element;

						// obtain demand table type

						m_ZoneMap[from_zone].m_ODDemandMatrix [to_zone].SetValue (demand_table_type,demand_value);


						RecordCount++;
					}

			} // time-dependent matrix

			m_AMSLogFile << RecordCount << "demand entries are loaded from file demand.dat "  ". Total demand =  " 
				<< total_demand << endl;
			m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file demand.dat. Total demand = %f",RecordCount,total_demand);

			fclose(pFile);
		}
	}
	// set link type


	m_LinkTypeMap[1].type_code  = 'f';
	m_LinkTypeMap[2].type_code  = 'f';

	/// to do: 
	m_NodeTypeMap[0] = "";
	m_NodeTypeMap[1] = "No Control";
	m_NodeTypeMap[2] = "Yield Sign";
	m_NodeTypeMap[3] = "4-Way Stop Sign";
	m_NodeTypeMap[4] = "Pretimed Signal";
	m_NodeTypeMap[5] = "Actuated Signal";
	m_NodeTypeMap[6] = "2-Way Stop Sign";


	m_ControlType_UnknownControl = 0; 
	m_ControlType_NoControl = 1;
	m_ControlType_YieldSign = 2;
	m_ControlType_2wayStopSign =  6;
	m_ControlType_4wayStopSign = 3;
	m_ControlType_PretimedSignal = 4;
	m_ControlType_ActuatedSignal = 5;
	m_ControlType_Roundabout = 7;

	m_LinkTypeFreeway = 1;
	m_LinkTypeArterial = 3;
	m_LinkTypeHighway = 2;

	DTALinkType element;

	element.link_type = 1;
	element.type_code = 'f';
	element.link_type_name = "Freeway";
	m_LinkTypeMap[1] = element;

	element.link_type = 2;
	element.type_code = 'f';
	element.link_type_name = "Freeway with Detector";
	m_LinkTypeMap[2] = element;

	element.link_type = 3;
	element.type_code = 'r';
	element.link_type_name = "On Ramp";
	m_LinkTypeMap[3] = element;

	element.link_type = 4;
	element.type_code = 'f';
	element.link_type_name = "Off Ramp";
	m_LinkTypeMap[4] = element;

	element.link_type = 5;
	element.type_code = 'a';
	element.link_type_name = "Arterial";
	m_LinkTypeMap[5] = element;

	element.link_type = 6;
	element.type_code = 'f';
	element.link_type_name = "HOT";
	m_LinkTypeMap[6] = element;


	element.link_type = 7;
	element.type_code = 'h';
	element.link_type_name = "Highway";
	m_LinkTypeMap[7] = element;

	element.link_type = 8;
	element.type_code = 'f';
	element.link_type_name = "HOV";
	m_LinkTypeMap[8] = element;

	element.link_type = 9;
	element.type_code = 'f';
	element.link_type_name = "FreewayHOT";
	m_LinkTypeMap[9] = element;

	element.link_type = 10;
	element.type_code = 'f';
	element.link_type_name = "FreewayHOV";
	m_LinkTypeMap[10] = element;

	m_OffsetInFeet=15;
	OffsetLink();
	GenerateOffsetLinkBand();

	CalculateDrawingRectangle();
	BuildGridSystem();

	m_bFitNetworkInitialized  = false;

	//read control.dat

	// read control.dat

	ReadDYNASMART_ControlFile();


	CTime LoadingEndTime = CTime::GetCurrentTime();

	CTimeSpan ts = LoadingEndTime  - LoadingStartTime;
	CString str_running_time;

	str_running_time.Format ("Network loading time: %d min(s) %d sec(s)...",ts.GetMinutes(), ts.GetSeconds());

	SetStatusText(str_running_time);

	//	m_SimulationLinkMOEDataLoadingStatus.Format("Load %d GPS probe records.", m_VehicleIDMap.size());


	// read system.dat
	fopen_s(&pFile,directory+"system.dat","r");
	if(pFile!=NULL)
	{
		g_Simulation_Time_Horizon = g_read_integer(pFile);
		/*
		itsMaxIterations >> temp;
		itsVehicleGenMode = (veh_gen)temp;
		is >> itsIntervalsPerAggregation >> itsIntervalsPerAssignment
		>> itsMUCThreshold >> itsConvergeThreshold; */

		fclose(pFile);
	}


	if(m_bLoadNetworkDataOnly)
		return true;


	ReadDYNASMARTSimulationResults();
	return true;
}

void CTLiteDoc::RecalculateLinkMOEFromVehicleTrajectoryFile()
{
	std::list<DTALink*>::iterator iLink ;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		(*iLink)->ResetMOEAry(m_SimulationStartTime_in_min + g_Simulation_Time_Horizon);  // use one day horizon as the default value
	}

		g_SimulatedDayDataMap[ 0] = true;  // day 0 as default


	bool bHighresoltutionFlag = false;

	if(bHighresoltutionFlag)
	{

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->m_HighResoltionLinkOutCount.resize(1200);  // 0.1 min, 2 hours

			for(int t=0; t< (*iLink)->m_HighResoltionLinkOutCount.size(); t++)
			{
				(*iLink)->m_HighResoltionLinkOutCount[t] = 0;  // reset 
			}
		}
	}
	int count  = 0;
	std::list<DTAVehicle*>::iterator iVehicle;

	for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++, count++)
	{
		DTAVehicle* pVehicle = (*iVehicle);

		// ==>Node Exit Time Point
		for(int i=1; i< pVehicle->m_NodeSize ; i++)
		{
			if(pVehicle->m_NodeAry[i].LinkNo==-1) 
				continue;
			DTALink* pLinkCurrent = m_LinkNoMap[ pVehicle->m_NodeAry[i].LinkNo];

			int t = pVehicle->m_NodeAry[i].ArrivalTimeOnDSN+1;

			// timt t is the timestamp from the current link to the next link, in min 
			if(pLinkCurrent!=NULL)
			{
				//pLinkCurrent->m_LinkMOEAry[t].VehicleOutflowCount+=1;

				//int time_interval = int(pVehicle->m_NodeAry[i].ArrivalTimeOnDSN*10); // convert min to 0.1 min

				//if(time_interval< pLinkCurrent->m_HighResoltionLinkOutCount.size())
				//{

				//	pLinkCurrent->m_HighResoltionLinkOutCount[time_interval]+=1;

				//}

			}


			if(i==1)  // first link 
			{
				int dep_t = pVehicle->m_DepartureTime+1;
				if(pLinkCurrent!=NULL )
				{
					////pLinkCurrent->m_LinkMOEAry[dep_t].VehicleInflowCount+=1;

				}

			}

			if(i< (pVehicle->m_NodeSize-1)&& pVehicle->m_NodeAry[i+1].LinkNo!=-1) // next link
			{

				DTALink* pLinkNext = m_LinkNoMap[ pVehicle->m_NodeAry[i+1].LinkNo];

				if(pLinkNext!=NULL)
				{
					//pLinkNext->m_LinkMOEAry[t].VehicleInflowCount+=1;

				}
			}

		}
	}

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		(*iLink)->RecalculateLinkMOEBasedOnVehicleCount();
	}


	// output high resolution link out count file

	bool bLinkOutCapcaityFile = 0;
	if(bLinkOutCapcaityFile)
	{
		ofstream LinkOutCapcaityFile;

		LinkOutCapcaityFile.open (m_ProjectDirectory+"output_hr_link_out_count.csv", ios::out);

		LinkOutCapcaityFile.precision(1) ;
		LinkOutCapcaityFile.setf(ios::fixed);

		std::set<DTALink*>::iterator iterLink;
		bool bTitlePrintOut = false;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{

			unsigned int t;

			DTALink* pLink = (*iLink);

			if(!bTitlePrintOut)
			{
				LinkOutCapcaityFile << "time_stamp_in_0.1_min" << "," ;

				for(t = 0; t < pLink->m_HighResoltionLinkOutCount.size(); t++)
				{
					float time_stamp = t*0.1f;
					LinkOutCapcaityFile << time_stamp << "," ;
				}
				bTitlePrintOut = true;
				LinkOutCapcaityFile << endl;	
			}

			LinkOutCapcaityFile << "link out count " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber << ",";
			for( t = 0; t < pLink->m_HighResoltionLinkOutCount.size(); t++)
			{
				LinkOutCapcaityFile << pLink->m_HighResoltionLinkOutCount[t] << "," ;
			}
			LinkOutCapcaityFile << endl;	

			LinkOutCapcaityFile << endl;	
		}


		LinkOutCapcaityFile.close();
	}

}
BOOL CTLiteDoc::ReadDYNASMARTSimulationResults()
{

	CString directory;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

	FILE* pFile = NULL;

	// read speed data
	fopen_s(&pFile,directory+"fort.900","r");
	std::list<DTALink*>::iterator iLink;

	CString str;
	str.Format ("Start loading speed data...");
	SetStatusText(str);

	if(pFile!=NULL)
	{
		m_TrafficFlowModelFlag = 2; // enable dynamic display mode after reading speed data
		// read data every min
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->ResetMOEAry(m_SimulationStartTime_in_min + g_Simulation_Time_Horizon);  // use one day horizon as the default value
		}

		float value = 0;
		for(int t = m_SimulationStartTime_in_min; t < m_SimulationStartTime_in_min+g_Simulation_Time_Horizon; t++)
		{
			int timestamp = (int)(g_read_float(pFile));  // read timestamp in min

			if(timestamp%10==0)
			{
			str.Format ("Start loading speed data at min %d", timestamp );
			SetStatusText(str);

			}


			if(timestamp < 0)  // end of file
				break;

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				value = g_read_float(pFile);  // speed;

				if(t < (*iLink)->m_LinkMOEArySize -1 )
				{
				(*iLink)->m_LinkMOEAry[t+1].Speed = value;
				(*iLink)->m_LinkMOEAry[t+1].TravelTime = (*iLink)->m_Length * 60/ max(1,value);
				}
			}

		}
		fclose(pFile);
	}

	str.Format ("Start loading queue length data...");
	SetStatusText(str);

	// read queue length data
	fopen_s(&pFile,directory+"fort.600","r");
	if(pFile!=NULL)
	{
		// read data every min

		for(int t = m_SimulationStartTime_in_min; t < m_SimulationStartTime_in_min+g_Simulation_Time_Horizon; t++)
		{
			int timestamp = g_read_float(pFile);  // read timestamp in min

			if(timestamp < 0)  // end of file
				break;

			if(timestamp%10==0)
			{
			str.Format ("Start loading queue length data at min %d", timestamp );
			SetStatusText(str);

			}

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				float value  = g_read_float(pFile);  // queue length;

				if(value < -0.5f)
					break;
				else if(t < (*iLink)->m_LinkMOEArySize -1 )
				{
					(*iLink)->m_LinkMOEAry[t+1].QueueLength = value;
				}
			}

		}
		fclose(pFile);
	}

	str.Format ("Start loading density data...");
	SetStatusText(str);

	// read density data
	fopen_s(&pFile,directory+"fort.700","r");
	if(pFile!=NULL)
	{
		// read data every min

		for(int t = m_SimulationStartTime_in_min; t < m_SimulationStartTime_in_min+g_Simulation_Time_Horizon; t++)
		{
			int timestamp = g_read_float(pFile);  // read timestamp in min

			if(timestamp < 0)  // end of file
				break;

			if(timestamp%10==0)
			{
			str.Format ("Start loading density data at min %d", timestamp );
			SetStatusText(str);

			}

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				float value  = g_read_float(pFile);  // density

				if(value < -0.5f)
					break;
				else if(t < (*iLink)->m_LinkMOEArySize -1 )
				{
					(*iLink)->m_LinkMOEAry[t+1].Density  = value;
				}
			}

		}
		fclose(pFile);
	}

	str.Format ("Start loading volume data...");
	SetStatusText(str);


	bool CumulativeFlag = true;
	// read flow rate
	fopen_s(&pFile,directory+"OutAccuVol.dat","r");

	if(pFile==NULL)
	{
		fopen_s(&pFile,directory+"LinkVolume.dat","r");
		CumulativeFlag = false;
	}


	if(pFile!=NULL)
	{
		g_read_float(pFile);  // read 10 min 
		// This file provides the accummulated number of veh. on of each link           10every sims ints

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{

			(*iLink)->m_total_link_volume = 0;
		}

		for(int t = m_SimulationStartTime_in_min; t < m_SimulationStartTime_in_min+g_Simulation_Time_Horizon; t++)
		{
			int timestamp = g_read_float(pFile);  // read timestamp in min

			//		TRACE("timestamp = %f\n",timestamp);

			if(timestamp < 0)  // end of file
				break;

			if(timestamp%10==0)
			{
			str.Format ("Start loading volume data at min %d", timestamp );
			SetStatusText(str);

			}


			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				if(CumulativeFlag && t < (*iLink)->m_LinkMOEArySize -1 )

				{
					float value = g_read_float(pFile);  // cumulative flow;

					if(t < (*iLink)->m_LinkMOEArySize -1 )
						(*iLink)->m_LinkMOEAry[t].ArrivalCumulativeFlow = value;

					if(t>=m_SimulationStartTime_in_min+1)
					{
						float value =  max(0,((*iLink)->m_LinkMOEAry[t].ArrivalCumulativeFlow - (*iLink)->m_LinkMOEAry[t-1].ArrivalCumulativeFlow));

						(*iLink)->m_LinkMOEAry[t].LinkFlow = value *60;  //convert to hourly count
						if((*iLink)->m_FromNodeNumber == 58730 && (*iLink)->m_ToNodeNumber == 80639)
						{
							TRACE("%d,%f\n",t,value);

						}

					}
				}else 
				{
					float value  =  g_read_float(pFile);
					 if( t < (*iLink)->m_LinkMOEArySize -1)
					 {
					(*iLink)->m_LinkMOEAry[t].LinkFlow = value;
					 }

				}
			}

		}
		fclose(pFile);
	}


	// density_in_veh_per_mile_per_lane

	/*
	fopen_s(&st,fname,"r");
	if(st!=NULL)
	{
	int i;
	int id = 0, zoneNum = 0;
	DTANode* pNode = 0;
	for(i = 0; i < g_ODZoneSize; i++)
	{

	zoneNum	= g_read_integer(pFile);
	int num_nodes= g_read_integer(pFile);

	if(num_nodes > g_AdjLinkSize)
	g_AdjLinkSize = num_nodes + 10;  // increaes buffer size


	for(int n = 0; n< num_nodes; n++)
	{
	int node_number= g_read_integer(pFile);


	g_NodeMap[node_number]->m_ZoneID = zoneNum;
	}
	}
	fclose(pFile);
	}
	}
	*/
	// read vehicle trajectory file

	// read additional data from sensor (AMS format)

	m_SimulationStartTime_in_min = (int)(g_GetPrivateProfileDouble("simulation_result", "simulation_start_time_in_min",0,m_ProjectFile));	
	//	ReadSensorData(directory+"input_sensor.csv",simulation_start_time_in_min);


	str.Format ("Start loading vehicle trajectory data...");
	SetStatusText(str);

	fopen_s(&pFile,directory+"VehTrajectory.dat","r");

	float LengthinMB;
	bool bLoadVehicleData = true;
	fopen_s(&pFile,directory+"VehTrajectory.dat","rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);
		LengthinMB= Length*1.0/1024/1024;
		if(LengthinMB>20)
		{
			CString msg;
			msg.Format("The vehtrajectory.dat file is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load the vehicle trajector files?",LengthinMB);
			if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
				bLoadVehicleData = false;
		}
	}

	ReadBackgroundImageFile(m_ProjectFile);

	//LoadGPSData();


	if(bLoadVehicleData)
	{
		fopen_s(&pFile,directory+"VehTrajectory.dat","r");

		if(pFile != NULL)
		{
			char  str_line[2000]; // input string
			int str_line_size;
			g_read_a_line(pFile,str_line, str_line_size); //  skip the two line
			g_read_a_line(pFile,str_line, str_line_size); //  skip the two line

			bool bformat_b_flag = false;
			CString Cstr_line;
			Cstr_line.Format("%s", str_line);
			if(Cstr_line.Find (" B")!=-1)
			{
				bformat_b_flag = true;	
			}



			int good_vehicle_id = 1;


			DTALink* m_pPathLinkVector[MAX_NODE_SIZE_IN_A_PATH];

			while(!feof(pFile) )
			{

				int m_VehicleID= g_read_integer(pFile, false);
				if(m_VehicleID == -1)
					break;
				// Veh #    144 Tag= 3 OrigZ=  10 DestZ=  11 Class= 3 UstmN=    150 DownN=     14 DestN=     13 STime=   0.50 

				DTAVehicle* pVehicle = 0;
				pVehicle = new DTAVehicle;
				pVehicle->m_VehicleID		= m_VehicleID;

				int CompleteFlag = g_read_integer(pFile);
				int NodeSizeOffset = 0;
				if(CompleteFlag==0 || CompleteFlag==1) 
				{
					pVehicle->m_bComplete = false;
					NodeSizeOffset = -1;
				}
				else 
				{
					pVehicle->m_bComplete = true;
					NodeSizeOffset = 0; 
				}

				pVehicle->m_OriginZoneID	= g_read_integer(pFile);
				pVehicle->m_DestinationZoneID = g_read_integer(pFile);

				pVehicle->m_InformationClass = (unsigned char)g_read_integer(pFile);

				int FirstNodeID = g_read_integer(pFile); 
				int SecondNodeID = g_read_integer(pFile);    // (4) first node
				int DestinationNodeID = g_read_integer(pFile);    // last node

				//pVehicle->m_OriginZoneID = NodeIDtoZoneNameMap[FirstNodeID];
				//pVehicle->m_DestinationZoneID = NodeIDtoZoneNameMap[DestinationNodeID];

				if( pVehicle->m_OriginZoneID > m_ODSize )
					m_ODSize = pVehicle->m_OriginZoneID ;

				if( pVehicle->m_DestinationZoneID > m_ODSize )
					m_ODSize = pVehicle->m_DestinationZoneID ;


				pVehicle->m_DepartureTime	= m_SimulationStartTime_in_min + g_read_float(pFile);

				if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
					g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

				// Total Travel Time=   0.34 # of Nodes=   2 VehType 1 LOO 1
				pVehicle->m_TripTime  = g_read_float(pFile);
				pVehicle->m_ArrivalTime = pVehicle->m_DepartureTime + pVehicle->m_TripTime;

				pVehicle->m_NodeSize	= g_read_integer(pFile) +1;  // +1 as we need to allocate one more node for the first node
				pVehicle->m_DemandType = g_read_integer(pFile);
				if(pVehicle->m_DemandType==1)
					pVehicle->m_PricingType =  1;

				if(pVehicle->m_DemandType==2)
					pVehicle->m_PricingType =  3;

				if(pVehicle->m_DemandType==3)
					pVehicle->m_PricingType =  2;

				if(bformat_b_flag==false)
				{
					g_read_integer(pFile);
				}

				pVehicle->m_TollDollarCost = 0;
				pVehicle->m_Emissions = 0;

				pVehicle->m_Distance = 0;

				pVehicle->m_VOT = 10;
				if(bformat_b_flag)
				{
					g_read_float(pFile);
					pVehicle->m_VOT =  g_read_float(pFile);
					g_read_float(pFile);
					g_read_float(pFile);
					g_read_float(pFile);
					g_read_float(pFile);
				}

				//			pVehicle->m_VehicleType = (unsigned char)g_read_integer(pFile);


				pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

				pVehicle->m_NodeNumberSum = 0;

				m_PathNodeVectorSP[0] =  FirstNodeID;

				int i;
				for(i=1; i< pVehicle->m_NodeSize; i++)
				{
					m_PathNodeVectorSP[i] = g_read_integer(pFile);

					//// update origin destination data based on TAZ
					//		if(i==0 && m_NodeNoMap.find(m_PathNodeVectorSP[i]) != m_NodeNoMap.end())
					//		{
					//			pVehicle->m_OriginZoneID = m_NodeNoMap[m_PathNodeVectorSP[i] ]-> m_ZoneID;
					//		
					//		}

					//		if(i== pVehicle->m_NodeSize -1 && m_NodeNoMap.find(m_PathNodeVectorSP[i]) != m_NodeNoMap.end())
					//		{
					//			pVehicle->m_DestinationZoneID = m_NodeNoMap[m_PathNodeVectorSP[i] ]-> m_ZoneID;
					//		
					//		}

					pVehicle->m_NodeNumberSum += m_PathNodeVectorSP[i];
					DTALink* pLink = FindLinkWithNodeNumbers(m_PathNodeVectorSP[i-1],m_PathNodeVectorSP[i],directory+"VehTrajectory.dat");
					if(pLink==NULL)
					{
						CString str;
						str.Format ("Error in reading file Vehicle.csv, good vehicle id: %d",good_vehicle_id);
						AfxMessageBox(str);
						fclose(pFile);

						return false;
					}
					pVehicle->m_NodeAry[i].LinkNo  = pLink->m_LinkNo ;
					m_pPathLinkVector[i] = pLink;
					pVehicle->m_Distance +=pLink->m_Length ;
						if(pVehicle->m_NodeAry[i].ArrivalTimeOnDSN < 10000) // feasible arrival time
						{
						pLink->m_total_link_volume +=1;

						double travel_time = 0 ;
						if(i>=2)
						travel_time  = pVehicle->m_NodeAry[i].ArrivalTimeOnDSN - pVehicle->m_NodeAry[i-1].ArrivalTimeOnDSN;
						else // first link
						travel_time  = pVehicle->m_NodeAry[i].ArrivalTimeOnDSN - pVehicle->m_DepartureTime ;
						pLink->m_TotalTravelTime += travel_time;
						pLink->m_total_delay += ( travel_time- pLink->m_FreeFlowTravelTime );
						}
				}


				// ==>Node Exit Time Point
				for(i=1; i< pVehicle->m_NodeSize + NodeSizeOffset; i++)
				{
					pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = m_SimulationStartTime_in_min + pVehicle->m_DepartureTime + g_read_float(pFile);

				}


				// ==>Link Travel Time
				for(i=1; i< pVehicle->m_NodeSize + NodeSizeOffset; i++)
				{
					g_read_float(pFile);  // // travel time
				}
				// ==>Accumulated Stop Time
				for(i=1; i< pVehicle->m_NodeSize +NodeSizeOffset; i++)
				{
					g_read_float(pFile);  // stop time
				}

				m_VehicleSet.push_back (pVehicle);
				m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;

				if(m_VehicleSet.size()%10000==0)
				{
					CString str_veh;
					str_veh.Format ("loading %d vehicles",m_VehicleSet.size());
					SetStatusText(str_veh);
				}

				good_vehicle_id = m_VehicleID;
			}

			fclose(pFile);


			m_SimulationVehicleDataLoadingStatus.Format ("%d vehicles are loaded from VehTrajectory.dat .",m_VehicleSet.size());

		}
	}

	UpdateMovementDataFromVehicleTrajector();
	return true;
}

void CTLiteDoc::OnToolsReverseverticalcoordinate()
{

	if(m_bDYNASMARTDataSet== false)
	{

		AfxMessageBox("This memu/function is used for DYNASMARAT data sets.",MB_ICONINFORMATION);

		return;
	}

	char lpbuffer[64];

	sprintf_s(lpbuffer,"%d",m_YCorridonateFlag*(-1));

	WritePrivateProfileString("coordinate", "origin",lpbuffer,m_ProjectFile);

	CString str;
	str.Format("The horizional coordinate flag (y_coordinate_flag) has been reset to %s in project file %s.\nPlease close NEXTA and reload the project.",m_ProjectFile, lpbuffer );
	AfxMessageBox(str,MB_ICONINFORMATION);

}

bool CTLiteDoc::ReadDYNASMARTVehicleTrajectoryFile(LPCTSTR lpszFileName, int date_id)
{
	FILE* pFile;
	fopen_s(&pFile,lpszFileName,"r");

	if(pFile != NULL)
	{
		int good_vehicle_id = 1;
		while(!feof(pFile) )
		{
			int m_VehicleID= g_read_integer(pFile, false);
			if(m_VehicleID == -1)
				break;
			// Veh #    144 Tag= 3 OrigZ=  10 DestZ=  11 Class= 3 UstmN=    150 DownN=     14 DestN=     13 STime=   0.50 

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_VehicleID		= m_VehicleID;

			int CompleteFlag = g_read_integer(pFile);
			int NodeSizeOffset = 0;
			if(CompleteFlag==0 || CompleteFlag==1) 
			{
				pVehicle->m_bComplete = false;
				NodeSizeOffset = -1;
			}
			else 
			{
				pVehicle->m_bComplete = true;
				NodeSizeOffset = 0; 
			}

			pVehicle->m_OriginZoneID	= g_read_integer(pFile);
			pVehicle->m_DestinationZoneID = g_read_integer(pFile);

			pVehicle->m_InformationClass = (unsigned char)g_read_integer(pFile);

			int FirstNodeID = g_read_integer(pFile); 
			int SecondNodeID = g_read_integer(pFile);    // (4) first node
			int DestinationNodeID = g_read_integer(pFile);    // last node


			pVehicle->m_OriginZoneID = NodeIDtoZoneNameMap[FirstNodeID];
			pVehicle->m_DestinationZoneID = NodeIDtoZoneNameMap[DestinationNodeID];

			if( pVehicle->m_OriginZoneID > m_ODSize )
				m_ODSize = pVehicle->m_OriginZoneID ;

			if( pVehicle->m_DestinationZoneID > m_ODSize )
				m_ODSize = pVehicle->m_DestinationZoneID ;


			pVehicle->m_DepartureTime	= m_SimulationStartTime_in_min + g_read_float(pFile);

			if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
				g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

			// Total Travel Time=   0.34 # of Nodes=   2 VehType 1 LOO 1
			pVehicle->m_TripTime  = g_read_float(pFile);
			pVehicle->m_ArrivalTime = pVehicle->m_DepartureTime + pVehicle->m_TripTime;

			pVehicle->m_NodeSize	= g_read_integer(pFile) +1;  // +1 as we need to allocate one more node for the first node
			pVehicle->m_DemandType = g_read_integer(pFile);
			g_read_integer(pFile);
			pVehicle->m_PricingType =  pVehicle->m_DemandType ;

			//			pVehicle->m_VehicleType = (unsigned char)g_read_integer(pFile);

			pVehicle->m_VOT = 10;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions = 0;

			pVehicle->m_Distance = 0;

			pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

			pVehicle->m_NodeNumberSum = 0;

			m_PathNodeVectorSP[0] =  FirstNodeID;

			int i;
			for(i=1; i< pVehicle->m_NodeSize; i++)
			{
				m_PathNodeVectorSP[i] = g_read_integer(pFile);

				//// update origin destination data based on TAZ
				//if(i==0 )
				//{
				//	pVehicle->m_OriginZoneID = m_NodeNoMap[m_PathNodeVectorSP[i] ]-> m_ZoneID;

				//}

				//if(i== pVehicle->m_NodeSize -1 )
				//{
				//	pVehicle->m_DestinationZoneID = m_NodeNoMap[m_PathNodeVectorSP[i] ]-> m_ZoneID;

				//}

				pVehicle->m_NodeNumberSum += m_PathNodeVectorSP[i];
				DTALink* pLink = FindLinkWithNodeNumbers(m_PathNodeVectorSP[i-1],m_PathNodeVectorSP[i]);
				if(pLink==NULL)
				{
					CString str;
					str.Format ("Error in reading file Vehicle.csv, good vehicle id: %d",good_vehicle_id);
					AfxMessageBox(str);
					fclose(pFile);

					return false;
				}
				pVehicle->m_NodeAry[i].LinkNo  = pLink->m_LinkNo ;
				pVehicle->m_Distance +=pLink->m_Length ;

						if(pVehicle->m_NodeAry[i].ArrivalTimeOnDSN < 10000) // feasible arrival time
						{
						pLink->m_total_link_volume +=1;

						double travel_time = 0;
						if(i>=2)
						travel_time  = pVehicle->m_NodeAry[i].ArrivalTimeOnDSN - pVehicle->m_NodeAry[i-1].ArrivalTimeOnDSN;
						else // first link
						travel_time  = pVehicle->m_NodeAry[i].ArrivalTimeOnDSN - pVehicle->m_DepartureTime ;

						pLink->m_TotalTravelTime += travel_time;
						pLink->m_total_delay += ( travel_time- pLink->m_FreeFlowTravelTime );
						}
			}


			// ==>Node Exit Time Point
			for(i=1; i< pVehicle->m_NodeSize + NodeSizeOffset; i++)
			{
				pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = m_SimulationStartTime_in_min + g_read_float(pFile);
			}


			// ==>Link Travel Time
			for(i=1; i< pVehicle->m_NodeSize + NodeSizeOffset; i++)
			{
				g_read_float(pFile);  // // travel time
			}
			// ==>Accumulated Stop Time
			for(i=1; i< pVehicle->m_NodeSize +NodeSizeOffset; i++)
			{
				g_read_float(pFile);  // stop time
			}

			m_VehicleSet.push_back (pVehicle);
			m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;

			if(m_VehicleSet.size()%10000==0)
			{
				CString str;
				str.Format ("loading %d vehicles",m_VehicleSet.size());
				SetStatusText(str);
			}

			good_vehicle_id = m_VehicleID;
		}

		fclose(pFile);

		m_SimulationVehicleDataLoadingStatus.Format ("%d vehicles are loaded from VehTrajectory.dat .",m_VehicleSet.size());
	}
	return true;
}

bool CTLiteDoc::ReadDYNASMART_ControlFile()
{
	FILE* st;
	fopen_s(&st,m_ProjectDirectory+"control.dat","r");
	if(st != NULL)
	{
		int num_timing_plan = g_read_integer(st);

		for(int time_plan_no = 1; time_plan_no <= num_timing_plan; time_plan_no++)
		{

		//		double start_time = g_read_float(st);
		char  str_line[2000]; // input string
		int str_line_size;
		g_read_a_line(st,str_line, str_line_size); //  skip the second line

		// read the first block: Node - Control Type
		std::list<DTANode*>::iterator iNode;

		int last_good_node_number = 0;
		for ( iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			int node_name = g_read_integer(st);
			if(node_name == -1)
			{
				CString str;
				str.Format("Error in reading the node block of control.dat. Last valid node number = %d ", last_good_node_number);
				AfxMessageBox(str, MB_ICONINFORMATION);
				fclose(st);
			}

			last_good_node_number = node_name;

			DTANode*  pNode = m_NodeNoMap[m_NodeNumbertoNodeNoMap[node_name]];
			pNode->m_ControlType  = g_read_integer(st);
			pNode->m_NumberofPhases = g_read_integer(st);
			pNode->m_CycleLengthInSecond = g_read_integer(st);


			if(pNode->m_ControlType == m_ControlType_PretimedSignal || pNode->m_ControlType == m_ControlType_ActuatedSignal)
			{

				if(pNode->m_ControlType == m_ControlType_PretimedSignal)
					SetupSignalValue(node_name,time_plan_no,TIMING_ControlType, "pretimed");

				if(pNode->m_ControlType == m_ControlType_ActuatedSignal)
					SetupSignalValue(node_name,time_plan_no,TIMING_ControlType, "actuated");

			SetupSignalValue(node_name,time_plan_no,TIMING_RingType, "single_ring");
			SetupSignalValue(node_name,time_plan_no,TIMING_CycleLength, pNode->m_CycleLengthInSecond);
			SetupSignalValue(node_name,time_plan_no,TIMING_Offset, 0);
			}
		}

		// read the second block: Phase time and movement
		// read node by node

		int number_of_signals = 0;

		for ( iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			DTANode* pNode = (*iNode);

			if(pNode->m_ControlType == m_ControlType_PretimedSignal || pNode->m_ControlType == m_ControlType_ActuatedSignal)
			{
				pNode-> m_bSignalData = true;
				number_of_signals++;

				int time = 0; 
				int start_time = 0;
				int end_time = 0;

				for(int p  = 0; p < pNode->m_NumberofPhases; p++)
				{
					start_time = time;

					int node_name = g_read_integer(st);

					if(m_NodeNumbertoNodeNoMap.find(node_name) == m_NodeNumbertoNodeNoMap.end())
					{
						CString str;
						str.Format("Error in reading the signal data block of control.dat. Last valid node number = %d ", last_good_node_number);
						AfxMessageBox(str, MB_ICONINFORMATION);
						fclose(st);
					}
					last_good_node_number = node_name;


					DTANode*  pNode = m_NodeNoMap[m_NodeNumbertoNodeNoMap[node_name]];

					int phase_ID = g_read_integer(st);

					DTANodePhase phase;
				
					if( pNode->m_ControlType == m_ControlType_ActuatedSignal)
					{
					phase.max_green  = g_read_integer(st);
					phase.min_green   = g_read_integer(st);
					}
					if( pNode->m_ControlType == m_ControlType_PretimedSignal)
					{
					g_read_integer(st);  // offset
					phase.max_green   = g_read_integer(st);
					phase.min_green = phase.max_green ;
					}

					
					phase.amber  = g_read_integer(st);

					SetupPhaseData(node_name, time_plan_no, p+1, PHASE_MinGreen,phase.min_green);
					SetupPhaseData(node_name, time_plan_no, p+1, PHASE_MaxGreen,phase.max_green);
					SetupPhaseData(node_name, time_plan_no, p+1, PHASE_Yellow,phase.amber);


					int approach = g_read_integer(st);


					// approach node numbers (reserved 4 nodes)
					g_read_integer(st);
					g_read_integer(st);
					g_read_integer(st);
					g_read_integer(st);

					//
					// read all possible approaches

					std::string movement_vector; 

					for(int i=0; i< approach; i++)
					{

						int in_link_from_node_number = g_read_integer(st);
						int in_link_to_node_number = g_read_integer(st);

						int in_link_from_node_id = m_NodeNumbertoNodeNoMap[in_link_from_node_number];
						int in_link_to_node_id = m_NodeNumbertoNodeNoMap[in_link_to_node_number];

						int phase_ID2 = g_read_integer(st);	// remember to read redundant phase id

						int movement_size  = g_read_integer(st);

						for(int k=0; k<movement_size; k++)
						{
							int out_link_to_node_number =  g_read_integer(st);
							int out_link_to_node_id = m_NodeNumbertoNodeNoMap[out_link_to_node_number];

							movement_vector += string_format("%d",in_link_from_node_number);
							movement_vector += ":";
							movement_vector += string_format("%d",out_link_to_node_number); 
							movement_vector += ";";


						}  // movement
						
					} // approach 

					time += (phase.max_green ) ;
					end_time = time;
					time += (phase.amber) ;

					SetupPhaseData(node_name,time_plan_no, p+1, PHASE_MOVEMENT_VECTOR, movement_vector);
					SetupPhaseData(node_name,time_plan_no, p+1, PHASE_Start, start_time);
					SetupPhaseData(node_name,time_plan_no, p+1, PHASE_End, end_time);
				} // phase

			}   // control data
		}  // for each node

				UpdateAllMovementGreenStartAndEndTime(time_plan_no);

		}
		fclose(st);


	//	m_SignalDataLoadingStatus.Format ("%d signals are loaded.",number_of_signals);
	}

	
	return true;
}

bool CTLiteDoc::ReadDYNASMART_ControlFile_ForAMSHub()
{
	FILE* st;

	int number_of_nodes = 0;
		int number_of_signals = 0;

	std::vector<int> DSP_signal_node_vector;
	fopen_s(&st,m_ProjectDirectory+"dsp_control.dat","r");
	if(st != NULL)
	{
		CDlg_UserInput dlg;

		dlg.m_StrQuestion  = "Please specify the number of nodes to be loaded from file dsp_control.dat:";
		dlg.m_InputValue = "10";

		if(dlg.DoModal ()==IDOK)
		{
			number_of_nodes = atoi(dlg.m_InputValue) ;
		}

		CWaitCursor cursor;

		int num_timing_plan = g_read_integer(st);

		if(num_timing_plan > 1)
		{
			AfxMessageBox("Number of signal timing plan is greater than 1.  Only one signal timing plan is allowed to generate the actuated control data in the current version.", MB_ICONINFORMATION);
			fclose(st);
		}

		//		double start_time = g_read_float(st);
		char  str_line[2000]; // input string
		int str_line_size;
		g_read_a_line(st,str_line, str_line_size); //  skip the second line

		// read the first block: Node - Control Type
		int last_good_node_number = 0;
		for ( int i = 0; i< number_of_nodes; i++)
		{
			int node_name = g_read_integer(st);
			if(node_name == -1)
			{
				CString str;
				str.Format("Error in reading the node block of control.dat. Last valid node number = %d ", last_good_node_number);
				AfxMessageBox(str, MB_ICONINFORMATION);
				fclose(st);
			}

			last_good_node_number = node_name;

			if(m_NodeNumbertoNodeNoMap.find(node_name)!=m_NodeNumbertoNodeNoMap.end())
			{
				DTANode*  pNode = m_NodeNoMap[m_NodeNumbertoNodeNoMap[node_name]];
				pNode->m_ControlType  = g_read_integer(st);

				if(pNode->m_ControlType == m_ControlType_PretimedSignal || pNode->m_ControlType == m_ControlType_ActuatedSignal)
				{
					DSP_signal_node_vector.push_back (node_name);
				}


				pNode->m_NumberofPhases = g_read_integer(st);
				pNode->m_CycleLengthInSecond = g_read_integer(st);

				TRACE("\nNode Number = %d, type = %d",node_name,pNode->m_ControlType );
			}else
			{
				TRACE("\nMissing Node Number = %d",node_name);
				g_read_integer(st);
				g_read_integer(st);
				g_read_integer(st);



			}

		}

		// read the second block: Phase time and movement
		// read node by node

		for ( int i = 0; i< DSP_signal_node_vector.size(); i++)
		{

			int node_name = DSP_signal_node_vector[i];
			if(m_NodeNumbertoNodeNoMap.find(node_name)!=m_NodeNumbertoNodeNoMap.end())
			{
				DTANode*  pNode = m_NodeNoMap[m_NodeNumbertoNodeNoMap[node_name]];

				if(pNode->m_ControlType == m_ControlType_PretimedSignal || pNode->m_ControlType == m_ControlType_ActuatedSignal)
				{
					pNode-> m_bSignalData = true;
					number_of_signals++;

					for(int p  = 0; p < pNode->m_NumberofPhases; p++)
					{
						int node_name = g_read_integer(st);

						last_good_node_number = node_name;


						int phase_ID = g_read_integer(st);


						if(node_name == 54154)
						{
							TRACE("");

						}

						DTANodePhase phase;
						phase.max_green  = g_read_integer(st);
						phase.min_green   = g_read_integer(st);
						phase.amber  = g_read_integer(st);
						int approach = g_read_integer(st);


						// approach node numbers (reserved 4 nodes)
						g_read_integer(st);
						g_read_integer(st);
						g_read_integer(st);
						g_read_integer(st);

						//
						// read all possible approaches
						for(int i=0; i< approach; i++)
						{

							int in_link_from_node_number = g_read_integer(st);
							int in_link_to_node_number = g_read_integer(st);
							int in_link_from_node_id = m_NodeNumbertoNodeNoMap[in_link_from_node_number];
							int in_link_to_node_id = m_NodeNumbertoNodeNoMap[in_link_to_node_number];

							int phase_ID2 = g_read_integer(st);	// remember to read redundant phase id

							int movement_size  = g_read_integer(st);

							for(int k=0; k<movement_size; k++)
							{
								int out_link_to_node_id = m_NodeNumbertoNodeNoMap[g_read_integer(st)];

								int movement_index = pNode->GetMovementIndex(in_link_from_node_id, in_link_to_node_id, out_link_to_node_id);
								if(movement_index>=0)
								{
									phase.movement_index_map[movement_index] = true;

								}
							}  // movement
						} // approach 
					} // phase

				}   // control data
			}
		}  // for each node
		fclose(st);


		m_SignalDataLoadingStatus.Format ("%d signals are loaded.",number_of_signals);
	}

	return true;
}
