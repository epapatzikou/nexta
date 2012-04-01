//  Portions Copyright 2012 Xuesong Zhou.
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
#include <iostream>                          // for cout, endl
#include <fstream>                           // for ofstream
#include <sstream>
using namespace std;

//#include "DYNASMART-interace.h"

// add description for DYNASMART
// Important References:
// Jayakrishnan, R., Mahmassani, H. S., and Hu, T.-Y., 1994a. An evaluation tool for advanced traffic information and management systems in urban networks. Transportation Research Part C, Vol. 2, No. 3, pp. 129-147.
// 


BOOL CTLiteDoc::OnOpenDYNASMARTProject(CString ProjectFileName, bool bNetworkOnly)
{
	CTime LoadingStartTime = CTime::GetCurrentTime();

	m_bLoadNetworkDataOnly = bNetworkOnly;

	CString directory;
	m_ProjectFile = ProjectFileName;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

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

			// Create and insert the node
			pNode = new DTANode;
			std::stringstream streams;
			 streams << node_id;

			pNode->m_Name = streams.str();
			pNode->m_ControlType = 0;

			// read xy.dat	
			int node_number = g_read_integer(pFileNodeXY);  //skip node number in xy.dta
			pNode->pt.x = g_read_float(pFileNodeXY);
			pNode->pt.y = g_read_float(pFileNodeXY);

			pNode->m_NodeNumber = node_id;
			pNode->m_NodeID = i;
			pNode->m_ZoneID = 0;
			m_NodeSet.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeIDtoNameMap[i] = node_id;
			m_NodeNametoIDMap[node_id] = i;
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
			pLink->m_FromNodeID = m_NodeNametoIDMap[pLink->m_FromNodeNumber];
			pLink->m_ToNodeID= m_NodeNametoIDMap[pLink->m_ToNodeNumber];

			int m_LeftBays= g_read_integer(pFile);
			int m_RightBays= g_read_integer(pFile);

			pLink->m_Length= g_read_float(pFile)/5280;   // 5280 feet per mile, NEXTA use mile as default length unit
			pLink->m_NumLanes= g_read_integer(pFile);

			int m_FlowModel= g_read_integer(pFile);
			int m_SpeedAdjustment= g_read_integer(pFile);
			pLink->m_SpeedLimit = g_read_integer(pFile);
			pLink->m_StaticSpeed = pLink->m_SpeedLimit;

			pLink->m_MaximumServiceFlowRatePHPL= g_read_float(pFile);
			pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;

			int m_SaturationFlowRate= g_read_integer(pFile);
			int DSP_link_type = g_read_integer(pFile);
				pLink->m_link_type = DSP_link_type;

			if(DSP_link_type == 3 || DSP_link_type == 4) 
				pLink->m_LaneCapacity  = 1400;

			if(DSP_link_type == 5) 
				pLink->m_LaneCapacity  = 1000;

			int m_grade= g_read_integer(pFile);

			m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
			m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;

			unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
			m_NodeIDtoLinkMap[LinkKey] = pLink;
			m_LinkNotoLinkMap[i] = pLink;

			m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);

			pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
			pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;
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

			TRACE("\nAdd link no.%d,  %d -> %d",i,pLink->m_FromNodeNumber, pLink->m_ToNodeNumber );
			m_LinkSet.push_back (pLink);
			m_LinkNoMap[i]  = pLink;
		}

		if(length_sum>0.000001f)
			m_UnitMile= default_distance_sum / length_sum ;

		m_UnitFeet = m_UnitMile/5280.0f;  

		m_LinkDataLoadingStatus.Format ("%d links are loaded.",m_LinkSet.size());
		ConstructMovementVectorForEachNode();

		GenerateOffsetLinkBand();


		fclose(pFile);
		fclose(pFileNodeXY);
	}else
	{
		AfxMessageBox("Error: File network.dat cannot be opened.");
		return false;
		//		g_ProgramStop();
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

				map <int, int> :: const_iterator m_Iter = m_NodeNametoIDMap.find(destination_node);

				if(m_Iter == m_NodeNametoIDMap.end( ))
				{
					m_WarningFile<< "Node Number "  << destination_node << " in destination.dat has not been defined in network.csv"  << endl; 
					fclose(pFile);
					return false;
				}

				int node_id  = m_NodeNametoIDMap[destination_node];
				m_NodeIDtoZoneNameMap[node_id] = zone_number;

				m_NodeIDMap[node_id]->m_ZoneID = zone_number;

				// if there are multiple nodes for a zone, the last node id is recorded.

				m_ZoneMap [zone_number].m_CentroidNodeAry .push_back (destination_node);

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

			for(int origin = 0;  origin < num_of_origins; origin++)
			{
				int from_node = g_read_integer(pFile);
				int to_node = g_read_integer(pFile);
				float loading_ratio = g_read_float(pFile);

				map <int, int> :: const_iterator m_Iter = m_NodeNametoIDMap.find(to_node);

				if(m_Iter == m_NodeNametoIDMap.end( ))
				{
					m_WarningFile<< "Node Number "  << to_node << " in origin.dat has not been defined in network.csv"  << endl; 
					fclose(pFile);
					return false;
				}

				int node_id  = m_NodeNametoIDMap[to_node];
				m_NodeIDtoZoneNameMap[node_id] = zone_number;

				m_NodeIDMap[node_id]->m_ZoneID = zone_number;

				// if there are multiple nodes for a zone, the last node id is recorded.

				if(m_ZoneMap [zone_number].m_CentroidNodeMap.find(to_node) == m_ZoneMap [zone_number].m_CentroidNodeMap.end())
				{
				m_ZoneMap [zone_number].m_CentroidNodeAry .push_back (to_node);
				}

			}

		}

		fclose(pFile);
	}

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
		// read the last value
		int end_of_simulation_horizon = g_read_float(pFile);
		TimeIntevalVector.push_back(end_of_simulation_horizon);

		for(i = 0; i < num_matrices; i++)
		{
			// Find a line with non-blank values to start
			// Origins
			double start_time= g_read_float(pFile); // start time

			for(int from_zone=0; from_zone< num_zones; from_zone++)
				for(int to_zone=0; to_zone< num_zones; to_zone++)
				{
					float demand_value = g_read_float(pFile) * demand_factor;

					DTADemand element;
					element.from_zone_id = from_zone+1;
					element.to_zone_id = to_zone+1;
					element.starting_time_in_min = TimeIntevalVector[i];
					element.ending_time_in_min = TimeIntevalVector[i+1];
					element.number_of_vehicles_per_demand_type.push_back(demand_value);
					m_DemandVector.push_back (element);
				}

		} // time-dependent matrix

		fclose(pFile);
	}

	OffsetLink();

	CalculateDrawingRectangle();

	m_bFitNetworkInitialized  = false;

	CTime LoadingEndTime = CTime::GetCurrentTime();

	CTimeSpan ts = LoadingEndTime  - LoadingStartTime;
	CString str_running_time;

	str_running_time.Format ("Network loading time: %d min(s) %d sec(s)...",ts.GetMinutes(), ts.GetSeconds());

	SetStatusText(str_running_time);

	if(bNetworkOnly)
		return true;

	// read vehicle trajectory file

	fopen_s(&pFile,directory+"VehTrajectory.dat","r");

   float LengthinMB;
   fopen_s(&pFile,directory+"VehTrajectory.dat","rb");
   if(pFile!=NULL)
   {
      fseek(pFile, 0, SEEK_END );
      int Length = ftell(pFile);
      fclose(pFile);
      LengthinMB= Length*1.0/1024/1024;
      if(LengthinMB>50)
      {
		 CString msg;
		 msg.Format("The vehtrajectory.dat file is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load the vehicle trajector files?",LengthinMB);
		 if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
			return true;
    }
   }

	fopen_s(&pFile,directory+"VehTrajectory.dat","r");

	if(pFile == NULL)
	{
		//   AfxMessageBox("VehTrajectory.dat cannot be opened.");
		return false;
	}


	int good_vehicle_id = 1;
	while(!feof(pFile) )
	{
		int m_VehicleID= g_read_integer(pFile);
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

		pVehicle->m_DepartureTime	= m_SimulationStartTime_in_min + g_read_float(pFile);

		if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
			g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

		// Total Travel Time=   0.34 # of Nodes=   2 VehType 1 LOO 1
		pVehicle->m_TripTime  = g_read_float(pFile);
		pVehicle->m_ArrivalTime = pVehicle->m_DepartureTime + pVehicle->m_TripTime;

		pVehicle->m_NodeSize	= g_read_integer(pFile) +1;  // +1 as we need to allocate one more node for the first node
		pVehicle->m_DemandType = (unsigned char)g_read_integer(pFile);
		pVehicle->m_PricingType = (unsigned char)g_read_integer(pFile);

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
			pLink->m_TotalVolume +=1;
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

		good_vehicle_id = m_VehicleID;
	}

	fclose(pFile);
	m_SimulationVehicleDataLoadingStatus.Format ("%d vehicles are loaded.",m_VehicleSet.size());

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

	// read speed data

	fopen_s(&pFile,directory+"fort.900","r");
	std::list<DTALink*>::iterator iLink;

	if(pFile!=NULL)
	{
		m_TrafficFlowModelFlag = 2; // enable dynamic display mode after reading speed data
		// read data every min
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);  // use one day horizon as the default value
		}

		for(int t = 0; t < g_Simulation_Time_Horizon; t++)
		{
			float timestamp = g_read_float(pFile);  // read timestamp in min

			if(timestamp < 0)  // end of file
				break;

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				(*iLink)->m_LinkMOEAry[t].ObsSpeed = g_read_float(pFile);  // speed;
			}

		}
	}

	// read flow rate
	fopen_s(&pFile,directory+"OutAccuVol.dat","r");

	if(pFile!=NULL)
	{
		g_read_float(pFile);  // read 10 min 
		// This file provides the accummulated number of veh. on of each link           10every sims ints

		for(int t = 0; t < g_Simulation_Time_Horizon; t++)
		{
			float timestamp = g_read_float(pFile);  // read timestamp in min

			if(timestamp < 0)  // end of file
				break;
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				(*iLink)->m_LinkMOEAry[t].ObsCumulativeFlow = g_read_float(pFile);  // cumulative flow;

				if(t>=1)
					(*iLink)->m_LinkMOEAry[t].ObsLinkFlow =  max(0,((*iLink)->m_LinkMOEAry[t].ObsCumulativeFlow - (*iLink)->m_LinkMOEAry[t-1].ObsCumulativeFlow)*60);
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


	return true;
}