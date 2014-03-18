//  Portions Copyright 2013 Xuesong Zhou, Jinjin Tang

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html

//    This file is part of DTALite.

//    DTALite is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    DTALite is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with DTALite.  If not, see <http://www.gnu.org/licenses/>.


// prototype 1: heterogeneous traveler
// prototype 2: departure time and mode options
// prototype 3: extention

// assignment module
// obtain simulation results, fetch shortest paths, assign vehicles to the shortest path according to gap function or MSA
#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"
#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <stdlib.h>  
#include <math.h>    

#include <iostream>
#include <conio.h>
#include <windows.h>

using namespace std;


DTANetworkForSP g_network_MP[_MAX_NUMBER_OF_PROCESSORS]; //  network instance for single processor in multi-thread environment: no more than 8 threads/cores

DTANetworkForSP g_network_VMS;


void DTANetworkForSP::AgentBasedPathAdjustment(int DayNo, int zone,int departure_time_begin, double current_time)
 // for pre-trip and en-route information user classes, for each departure time interval
{
	if(g_ODEstimationFlag==1)
		return;
	int PathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
	int CurrentPathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
	int TempPathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
	float AbsArrivalTimeOnDSNVector[MAX_NODE_SIZE_IN_A_PATH]={0};
	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int NodeSize;
	int TempNodeSize;

	int AssignmentInterval = g_FindAssignmentInterval(departure_time_begin);  // starting assignment interval


	// loop through the TDOVehicleArray to assign or update vehicle paths...
	for (int vi = 0; vi<g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][AssignmentInterval].VehicleArray.size(); vi++)
	{
		int VehicleID = g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][AssignmentInterval].VehicleArray[vi];
		DTAVehicle* pVeh  = g_VehicleMap[VehicleID];

		// if this is a pre-trip vehicle, and he has not obtained real-time information yet

		bool b_switch_flag = false;




		if(g_bVehicleAttributeUpdatingFlag ==true)  // if some vehicles' attribute might need updating
		{
		int current_time_in_min = (int)(current_time);

			if(pVeh->m_attribute_update_time_in_min <= current_time_in_min 
				&& pVeh->m_bEvacuationResponse == false
				&& pVeh->m_SimLinkSequenceNo < pVeh->m_NodeSize - 2) // if scheduled updating time is earlier than the current time, has not reached at destination yet 
			{
				pVeh->m_bEvacuationResponse  = true;
				// change destination zone number

				// check if the current link is still in evacuation zone
				int CurrentLinkID = pVeh->m_NodeAry[pVeh->m_SimLinkSequenceNo].LinkNo;
				
				if( g_LinkVector[CurrentLinkID]->IsInsideEvacuationZoneFlag(DayNo, current_time))
				{
				pVeh->m_DestinationZoneID  = pVeh->m_DestinationZoneID_Updated;
				pVeh->m_DestinationNodeID =  g_ZoneMap[pVeh->m_DestinationZoneID].GetRandomDestinationIDInZone ((pVeh->m_VehicleID%100)/100.0f); 

				pVeh->m_InformationClass = info_en_route; // 3, so all the vehicles will access real-time information 
				pVeh->m_TimeToRetrieveInfo = 99999; // no more update

				b_switch_flag = true;
				DTAEvacuationRespone response;
				response.LinkNo = CurrentLinkID;
				response.ResponseTime = current_time;
				//pVeh->m_EvacuationResponseVector.push_back (response);
				}
			}
		
		}



		// pVeh->m_DepartureTime +1: assume we access pre trip information one min before the trip

		if(pVeh->m_InformationClass == info_pre_trip 
			&& int(pVeh->m_DepartureTime +1) > current_time /* if the departure time rounds up to the current time (min by min), then perforom pre-trip routing  */
			&& int(pVeh->m_TimeToRetrieveInfo) <= current_time)
		{

						if(pVeh->m_VehicleID ==86)
						{
						TRACE("");
						}
			b_switch_flag = true;

			pVeh->m_TimeToRetrieveInfo = 99999; // no more update
		}

		// if this is an enroute info vehicle, 
		if(pVeh->m_InformationClass == info_en_route && pVeh->m_bLoaded == true &&
			int(pVeh->m_DepartureTime) < current_time &&
			(int)(pVeh->m_TimeToRetrieveInfo) <= current_time && pVeh ->m_bComplete == false)
		{

			pVeh->m_TimeToRetrieveInfo += 1; // next 1 min
			b_switch_flag = true;

			if(pVeh->m_VehicleID == 66 && current_time >=120)
			{
			TRACE("Current Time %f, Node Size = %d\n", current_time, pVeh->m_NodeSize);
			}


		}

		if(b_switch_flag == true)
		{

		if(pVeh->m_VehicleID == 68)
		{
		TRACE("");
		}
		float TotalCost = 0;
		bool bDistanceFlag = false;
		bool bDebugFlag = false;

		int StartingNodeID = pVeh->m_OriginNodeID;

		if(pVeh->m_InformationClass == info_en_route  && pVeh->m_bLoaded == true )  // en route info, has not reached destination yet
		{
			int CurrentLinkID = pVeh->m_NodeAry[pVeh->m_SimLinkSequenceNo].LinkNo;
			StartingNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

			if(pVeh->m_SimLinkSequenceNo == pVeh->m_NodeSize - 2)
				continue;

			if(StartingNodeID == pVeh->m_DestinationNodeID )  // you will reach the destination (on the last link).
				continue;

			// copy existing link number
			for(int i = 0; i<= pVeh->m_SimLinkSequenceNo; i++)
			{
				CurrentPathLinkList[i] = 	pVeh->m_NodeAry[i].LinkNo;
				AbsArrivalTimeOnDSNVector[i]= pVeh->m_NodeAry[i].AbsArrivalTimeOnDSN ;
			}
		}


		// use agent-specific information look up window to update the prevailing travel time conditions 
		// step 1: update m_prevailing_travel_time for each link
		// step 2: call function BuildPhysicalNetwork again to use m_prevailing_travel_time to overwrite link travel time data
		// step 3: use link travel time data in agent-based routing 

		NodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, StartingNodeID , pVeh->m_DepartureTime , pVeh->m_DestinationZoneID , pVeh->m_DestinationNodeID, pVeh->m_PricingType , pVeh->m_VOT, PathLinkList, TotalCost,bDistanceFlag, bDebugFlag);


	if(pVeh->m_InformationClass == info_en_route && pVeh->m_bLoaded == true )  // en route info
	{

		int count = pVeh->m_SimLinkSequenceNo;
		for(int i = 0; i<NodeSize-1; i++)
		{
			count  +=1;  // add link no from the new path starting from the current position 
			CurrentPathLinkList[count] = 	PathLinkList[i];;
		}

		NodeSize = count+2;

		for(int i = 0; i< NodeSize-1; i++)  // copy back
		{		
			PathLinkList[i] = CurrentPathLinkList[i];
		}
	
	
	}

	
		if(NodeSize<=1)
			continue;

			if( pVeh->m_NodeAry !=NULL)  // delete the old path
			{
				delete pVeh->m_NodeAry;
			}

			pVeh->m_NodeSize = NodeSize;


			if(pVeh->m_NodeSize>=2)  // for feasible path
			{

				if(NodeSize>=MAX_NODE_SIZE_IN_A_PATH)
				{
					cout << "PATH Size is out of bound of " << MAX_NODE_SIZE_IN_A_PATH << ": "<< NodeSize;
					g_ProgramStop();
				}

				pVeh->m_NodeAry = new SVehicleLink[NodeSize];

				if(pVeh->m_NodeAry==NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				int NodeNumberSum =0;
				float Distance =0;

				for(int i = 0; i< NodeSize-1; i++)
				{
					pVeh->m_NodeAry[i].AbsArrivalTimeOnDSN = AbsArrivalTimeOnDSNVector[i];

					pVeh->m_NodeAry[i].LinkNo = PathLinkList[i];
					NodeNumberSum += PathLinkList[i];

					if(g_LinkVector[pVeh->m_NodeAry [i].LinkNo]!=NULL)
					{
						DTALink* pLink = g_LinkVector[pVeh->m_NodeAry [i].LinkNo];
						TRACE("Prevailing Travel Time %f \n", pLink->m_prevailing_travel_time );

					}
					
					ASSERT(pVeh->m_NodeAry [i].LinkNo < g_LinkVector.size());
					Distance+= g_LinkVector[pVeh->m_NodeAry [i].LinkNo] ->m_Length ;

				}
				
					
				pVeh->m_Distance  = Distance;
				pVeh->m_NodeNumberSum = NodeNumberSum;


			}
		}
	} // for each vehicle on this OD pair

}


void g_AgentBasedVMSRoutingInitialization(int DayNo, double CurrentTime )  
// for VMS responsive vehicles
{
	if(g_ODEstimationFlag==1)
		return;


	int node_size  = g_NodeVector.size() +1 + g_ODZoneNumberSize;
	int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations
	g_network_VMS.Setup(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin);
	//special notes: creating network with dynamic memory is a time-consumping task, so we create the network once for each processors
	g_network_VMS.BuildPhysicalNetwork(0,0,g_TrafficFlowModelFlag, true, CurrentTime );

	int number_of_threads = omp_get_max_threads ( );

	int max_number_of_threads = g_GetPrivateProfileInt("computation", "max_number_of_threads_to_be_used", 8, g_DTASettingFileName);

	if(number_of_threads > max_number_of_threads)
		number_of_threads = max_number_of_threads ;


	if(number_of_threads > _MAX_NUMBER_OF_PROCESSORS)
	{ 
		cout<< "the number of threads is "<< number_of_threads << ", which is greater than _MAX_NUMBER_OF_PROCESSORS. Please contact developers!" << endl; 
		g_ProgramStop();
	}

	for(int ProcessID=0;  ProcessID < number_of_threads; ProcessID++)
	{
	g_network_MP[ProcessID].Setup(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin);
	//special notes: creating network with dynamic memory is a time-consumping task, so we create the network once for each processors
	g_network_MP[ProcessID].BuildPhysicalNetwork(0,0,g_TrafficFlowModelFlag, true, CurrentTime );
	}
}
void g_UpdateRealTimeInformation(double CurrentTime)
{
	int number_of_threads = omp_get_max_threads ( );

	int max_number_of_threads = g_GetPrivateProfileInt("computation", "max_number_of_threads_to_be_used", 8, g_DTASettingFileName);

	if(number_of_threads > max_number_of_threads)
		number_of_threads = max_number_of_threads ;


	if(number_of_threads > _MAX_NUMBER_OF_PROCESSORS)
	{ 
		cout<< "the number of threads is "<< number_of_threads << ", which is greater than _MAX_NUMBER_OF_PROCESSORS. Please contact developers!" << endl; 
		g_ProgramStop();
	}

	for(int ProcessID=0;  ProcessID < number_of_threads; ProcessID++)
	{
		g_network_MP[ProcessID].BuildPhysicalNetwork(0,0,g_TrafficFlowModelFlag, true, CurrentTime );	
	}
}
void g_AgentBasedPathAdjustmentWithRealTimeInfo(int VehicleID , double current_time)
// for VMS resonsive information
{
	if(g_ODEstimationFlag==1)
		return;

	int PathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
	int CurrentPathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
	int TempPathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
	float AbsArrivalTimeOnDSNVector[MAX_NODE_SIZE_IN_A_PATH]={0};
	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int NodeSize;
	int TempNodeSize;


		DTAVehicle* pVeh  = g_VehicleMap[VehicleID];


		if( pVeh->m_NodeSize >= MAX_NODE_SIZE_IN_A_PATH - 1 )
		{
		cout << "pVeh->m_NodeSize > MAX_NODE_SIZE_IN_A_PATH" << endl;
		g_ProgramStop();
		}

		// if this is a pre-trip vehicle, and he has not obtained real-time information yet

		float TotalCost = 0;
		bool bDistanceFlag = false;
		bool bDebugFlag = false;

		int StartingNodeID = pVeh->m_OriginNodeID;

			int CurrentLinkID = pVeh->m_NodeAry[pVeh->m_SimLinkSequenceNo].LinkNo;
			StartingNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

			if(StartingNodeID == pVeh->m_DestinationNodeID )  // you will reach the destination (on the last link).
				return;

			// copy existing link number (and remaining links)

			for(int i = 0; i< pVeh->m_NodeSize -1; i++)
			{
				CurrentPathLinkList[i] = 	pVeh->m_NodeAry[i].LinkNo;
				AbsArrivalTimeOnDSNVector[i]= pVeh->m_NodeAry[i].AbsArrivalTimeOnDSN ;
			}


		int count = pVeh->m_SimLinkSequenceNo;

			//if(is.Type == 2) // detour VMS
			//{
			//for(int ll = 0; ll < is.DetourLinkSize ; ll++)
			//{
			//PathLinkList[l++] = is.DetourLinkArray [ll];
			//CurrentTime+= g_LinkVector[is.DetourLinkArray [ll]]->m_FreeFlowTravelTime;  // add travel time along the path to get the starting time at the end of subpath
			//}

			//CurrentNodeID = g_LinkVector[is.DetourLinkArray [is.DetourLinkSize -1]]->m_ToNodeID;

			//}

		if(pVeh->m_OriginNodeID < 0 || pVeh->m_DestinationNodeID <0)
		{

		cout << "m_OriginNodeID < 0 || m_DestinationNodeID <0" << endl;
		getchar();
		
		}
		 
		NodeSize = g_network_VMS.FindBestPathWithVOT(pVeh->m_OriginZoneID, StartingNodeID , pVeh->m_DepartureTime , pVeh->m_DestinationZoneID , pVeh->m_DestinationNodeID, pVeh->m_PricingType , pVeh->m_VOT, PathLinkList, TotalCost,bDistanceFlag, bDebugFlag);


		int bSwitchFlag = 0;
		for(int i = 0; i<NodeSize-1; i++)
		{
			count  +=1;  // add link no from the new path starting from the current position 

			if(CurrentPathLinkList[count] != PathLinkList[i])
				bSwitchFlag = 1;
				

			CurrentPathLinkList[count] = 	PathLinkList[i];

		}

		NodeSize = count+2;

		for(int i = 0; i< NodeSize-1; i++)  // copy back
		{		
			PathLinkList[i] = CurrentPathLinkList[i];
		}
	
		
		if(NodeSize<=1)
			return;


//			// add link to VMS respons link
//#ifdef  _large_memory_usage
//			bool bLinkAdded = false;
//			for(int i =0; i< pVeh->m_VMSResponseVector.size(); i++)
//			{
//				if(pVeh->m_VMSResponseVector[i].LinkNo == CurrentLinkID)
//				{
//					bLinkAdded = true;
//					break;
//				}
//
//			}
//
//			if(!bLinkAdded)
//			{
//				DTAVMSRespone vms;
//				vms.LinkNo = CurrentLinkID;
//				vms.ResponseTime = current_time;
//				vms.SwitchFlag = bSwitchFlag;
//				pVeh->m_VMSResponseVector.push_back (vms);
//
//			}
//#endif

		if(bSwitchFlag==0)
			return;

			if( pVeh->m_NodeAry !=NULL)  // delete the old path
			{
				delete pVeh->m_NodeAry;
			}




			pVeh->m_NodeSize = NodeSize;


			if(pVeh->m_NodeSize>=2)  // for feasible path
			{

				if(NodeSize>=MAX_NODE_SIZE_IN_A_PATH)
				{
					cout << "PATH Size is out of bound: " << MAX_NODE_SIZE_IN_A_PATH << NodeSize;
					g_ProgramStop();
				}

				pVeh->m_NodeAry = new SVehicleLink[NodeSize];

				if(pVeh->m_NodeAry==NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				int NodeNumberSum =0;
				float Distance =0;

				for(int i = 0; i< NodeSize-1; i++)
				{
					pVeh->m_NodeAry[i].AbsArrivalTimeOnDSN = AbsArrivalTimeOnDSNVector[i];

					pVeh->m_NodeAry[i].LinkNo = PathLinkList[i];
					NodeNumberSum += PathLinkList[i];

					/*if(g_LinkVector[pVeh->m_NodeAry [i].LinkNo]==NULL)
					{
					cout << "Error: g_LinkVector[pVeh->m_NodeAry [i].LinkNo]==NULL", pVeh->m_NodeAry [i].LinkNo;
					getchar();
					exit(0);
					}
					*/
					ASSERT(pVeh->m_NodeAry [i].LinkNo < g_LinkVector.size());
					Distance+= g_LinkVector[pVeh->m_NodeAry [i].LinkNo] ->m_Length ;

				}
				
					
				pVeh->m_Distance  = Distance;
				pVeh->m_NodeNumberSum = NodeNumberSum;
			}
}

void g_OpenMPAgentBasedPathAdjustmentWithRealTimeInfo(int VehicleID , double current_time)
// for VMS resonsive information
{
	if(g_ODEstimationFlag==1)
		return;

	int PathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
	int CurrentPathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
	int TempPathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
	float AbsArrivalTimeOnDSNVector[MAX_NODE_SIZE_IN_A_PATH]={0};
	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int NodeSize;
	int TempNodeSize;


		DTAVehicle* pVeh  = g_VehicleMap[VehicleID];


		if( pVeh->m_NodeSize >= MAX_NODE_SIZE_IN_A_PATH - 1 )
		{
		cout << "pVeh->m_NodeSize > MAX_NODE_SIZE_IN_A_PATH" << endl;
		g_ProgramStop();
		}

		// if this is a pre-trip vehicle, and he has not obtained real-time information yet

		float TotalCost = 0;
		bool bDistanceFlag = false;
		bool bDebugFlag = false;

		int StartingNodeID = pVeh->m_OriginNodeID;

			int CurrentLinkID = pVeh->m_NodeAry[pVeh->m_SimLinkSequenceNo].LinkNo;
			StartingNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

			if(StartingNodeID == pVeh->m_DestinationNodeID )  // you will reach the destination (on the last link).
				return;

			// copy existing link number (and remaining links)

			for(int i = 0; i< pVeh->m_NodeSize -1; i++)
			{
				CurrentPathLinkList[i] = 	pVeh->m_NodeAry[i].LinkNo;
				AbsArrivalTimeOnDSNVector[i]= pVeh->m_NodeAry[i].AbsArrivalTimeOnDSN ;
			}


		int count = pVeh->m_SimLinkSequenceNo;

			//if(is.Type == 2) // detour VMS
			//{
			//for(int ll = 0; ll < is.DetourLinkSize ; ll++)
			//{
			//PathLinkList[l++] = is.DetourLinkArray [ll];
			//CurrentTime+= g_LinkVector[is.DetourLinkArray [ll]]->m_FreeFlowTravelTime;  // add travel time along the path to get the starting time at the end of subpath
			//}

			//CurrentNodeID = g_LinkVector[is.DetourLinkArray [is.DetourLinkSize -1]]->m_ToNodeID;

			//}

		if(pVeh->m_OriginNodeID < 0 || pVeh->m_DestinationNodeID <0)
		{

		cout << "m_OriginNodeID < 0 || m_DestinationNodeID <0" << endl;
		getchar();
		
		}

		int	processor_id = omp_get_thread_num( );  // starting from 0

	//	NodeSize = g_network_MP[processor_id].FindBestPathWithVOT(pVeh->m_OriginZoneID, StartingNodeID , pVeh->m_DepartureTime , pVeh->m_DestinationZoneID , pVeh->m_DestinationNodeID, pVeh->m_PricingType , pVeh->m_VOT, PathLinkList, TotalCost,bDistanceFlag, bDebugFlag);
		NodeSize = g_network_VMS.FindBestPathWithVOT(pVeh->m_OriginZoneID, StartingNodeID , pVeh->m_DepartureTime , pVeh->m_DestinationZoneID , pVeh->m_DestinationNodeID, pVeh->m_PricingType , pVeh->m_VOT, PathLinkList, TotalCost,bDistanceFlag, bDebugFlag);


		int bSwitchFlag = 0;
		for(int i = 0; i<NodeSize-1; i++)
		{
			count  +=1;  // add link no from the new path starting from the current position 

			if(CurrentPathLinkList[count] != PathLinkList[i])
				bSwitchFlag = 1;
				

			CurrentPathLinkList[count] = 	PathLinkList[i];

		}

		NodeSize = count+2;

		for(int i = 0; i< NodeSize-1; i++)  // copy back
		{		
			PathLinkList[i] = CurrentPathLinkList[i];
		}
	
		
		if(NodeSize<=1)
			return;


//			// add link to VMS respons link
//#ifdef  _large_memory_usage
//			bool bLinkAdded = false;
//			for(int i =0; i< pVeh->m_VMSResponseVector.size(); i++)
//			{
//				if(pVeh->m_VMSResponseVector[i].LinkNo == CurrentLinkID)
//				{
//					bLinkAdded = true;
//					break;
//				}
//
//			}
//
//			if(!bLinkAdded)
//			{
//				DTAVMSRespone vms;
//				vms.LinkNo = CurrentLinkID;
//				vms.ResponseTime = current_time;
//				vms.SwitchFlag = bSwitchFlag;
//				pVeh->m_VMSResponseVector.push_back (vms);
//
//			}
//#endif

		if(bSwitchFlag==0)
			return;

			if( pVeh->m_NodeAry !=NULL)  // delete the old path
			{
				delete pVeh->m_NodeAry;
			}




			pVeh->m_NodeSize = NodeSize;


			if(pVeh->m_NodeSize>=2)  // for feasible path
			{

				if(NodeSize>=MAX_NODE_SIZE_IN_A_PATH)
				{
					cout << "PATH Size is out of bound: " << MAX_NODE_SIZE_IN_A_PATH << NodeSize;
					g_ProgramStop();
				}

				pVeh->m_NodeAry = new SVehicleLink[NodeSize];

				if(pVeh->m_NodeAry==NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				int NodeNumberSum =0;
				float Distance =0;

				for(int i = 0; i< NodeSize-1; i++)
				{
					pVeh->m_NodeAry[i].AbsArrivalTimeOnDSN = AbsArrivalTimeOnDSNVector[i];

					pVeh->m_NodeAry[i].LinkNo = PathLinkList[i];
					NodeNumberSum += PathLinkList[i];

					/*if(g_LinkVector[pVeh->m_NodeAry [i].LinkNo]==NULL)
					{
					cout << "Error: g_LinkVector[pVeh->m_NodeAry [i].LinkNo]==NULL", pVeh->m_NodeAry [i].LinkNo;
					getchar();
					exit(0);
					}
					*/
					ASSERT(pVeh->m_NodeAry [i].LinkNo < g_LinkVector.size());
					Distance+= g_LinkVector[pVeh->m_NodeAry [i].LinkNo] ->m_Length ;

				}
				
					
				pVeh->m_Distance  = Distance;
				pVeh->m_NodeNumberSum = NodeNumberSum;
			}
}

void g_ReadRealTimeSimulationSettingsFile()
{
	CCSVParser parser_RTSimulation_settings;
	if (parser_RTSimulation_settings.OpenCSVFile("input_real_time_simulation_settings.csv",false))
	{

		// we enable information updating for real-time simulation mode
		g_bInformationUpdatingAndReroutingFlag  = true;

		int record_count = 0;

	cout << "File input_real_time_simulation_settings.csv is opened."<< endl;
	g_LogFile << "File input_real_time_simulation_settings.csv is opened."<< endl;

		while(parser_RTSimulation_settings.ReadRecord())
		{

		int timestamp_in_min = -1;
		std::string  break_point_flag;
		int output_TD_start_time_in_min = 0;

		int output_MOE_aggregation_time_interval_in_min = 1;
		int update_attribute_aggregation_time_interval_in_min = 1;
		


		std::string output_TD_link_travel_time_file, output_TD_link_MOE_file,
			update_trip_file,update_TD_link_attribute_file;

		int day_no = 0;
		parser_RTSimulation_settings.GetValueByFieldName("day_no",day_no);
		parser_RTSimulation_settings.GetValueByFieldName("timestamp_in_min",timestamp_in_min);

		if(day_no >=1)
			timestamp_in_min = (day_no-1)*1440 + timestamp_in_min; // consider day no

		parser_RTSimulation_settings.GetValueByFieldName("break_point_flag",break_point_flag);
		parser_RTSimulation_settings.GetValueByFieldName("output_TD_link_travel_time_file",output_TD_link_travel_time_file);
		parser_RTSimulation_settings.GetValueByFieldName("output_TD_link_MOE_file",output_TD_link_MOE_file);
		parser_RTSimulation_settings.GetValueByFieldName("output_TD_start_time_in_min",output_TD_start_time_in_min);
		parser_RTSimulation_settings.GetValueByFieldName("output_MOE_aggregation_time_interval_in_min",output_MOE_aggregation_time_interval_in_min);
		parser_RTSimulation_settings.GetValueByFieldName("update_attribute_time_interval_in_min",update_attribute_aggregation_time_interval_in_min);
	

		parser_RTSimulation_settings.GetValueByFieldName("update_trip_file",update_trip_file);
		parser_RTSimulation_settings.GetValueByFieldName("update_TD_link_attribute_file",update_TD_link_attribute_file);

			if(timestamp_in_min>=0)
			{
				if(break_point_flag.find ("break") != string::npos)
				{

					g_RealTimeSimulationSettingsMap[timestamp_in_min].break_point_flag  =true;

					if(break_point_flag.find ("break_and_wait") != string::npos)
					{
					g_RealTimeSimulationSettingsMap[timestamp_in_min].break_and_wait_flag   =true;
					
					}
						parser_RTSimulation_settings.GetValueByFieldName("output_agent_file",g_RealTimeSimulationSettingsMap[timestamp_in_min].output_agent_file );
						parser_RTSimulation_settings.GetValueByFieldName("output_trip_file",g_RealTimeSimulationSettingsMap[timestamp_in_min].output_trip_file);
						parser_RTSimulation_settings.GetValueByFieldName("output_od_moe_file",g_RealTimeSimulationSettingsMap[timestamp_in_min].output_od_moe_file);
						parser_RTSimulation_settings.GetValueByFieldName("output_td_skim_file",g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file);



					g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_link_travel_time_file = output_TD_link_travel_time_file;
					g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_link_MOE_file = output_TD_link_MOE_file;
					g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_start_time_in_min = output_TD_start_time_in_min;
					g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_aggregation_time_in_min = output_MOE_aggregation_time_interval_in_min;
					g_RealTimeSimulationSettingsMap[timestamp_in_min].update_trip_file = update_trip_file;


					g_RealTimeSimulationSettingsMap[timestamp_in_min].update_attribute_aggregation_time_interval_in_min = update_attribute_aggregation_time_interval_in_min;
					g_RealTimeSimulationSettingsMap[timestamp_in_min].update_TD_link_attribute_file  = update_TD_link_attribute_file;


					record_count ++;
				}

			}

		}

	cout << "File input_real_time_simulation_settings.csv has "<< record_count  << " breakpoint records." << endl;
	g_LogFile << "File input_real_time_simulation_settings.csv has "<<  record_count << " breakpoint records." << endl;

	}
}

void g_ExchangeRealTimeSimulationData(int day_no,int timestamp_in_min)
{
	// update timestamp using day no
	timestamp_in_min = day_no*1440+ timestamp_in_min;

	if(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_link_travel_time_file.size() >=1)
	{
		  OutputRealTimeLinkMOEData(
			  g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_link_travel_time_file,
			   g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_start_time_in_min ,
			   timestamp_in_min,
			    g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_aggregation_time_in_min,
				true);
	}

	if(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_link_MOE_file.size() >=1)
	{
		  OutputRealTimeLinkMOEData(
			  g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_link_MOE_file,
			   g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_start_time_in_min ,
			   timestamp_in_min,
			    g_RealTimeSimulationSettingsMap[timestamp_in_min].output_TD_aggregation_time_in_min,
				false);
	}

	if(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_trip_file .size() >=1 )
	{
		   // output agent and trip files;

		while (1)
		{
		char fname_trip[_MAX_PATH];

		sprintf(fname_trip,"%s",g_RealTimeSimulationSettingsMap[timestamp_in_min].output_trip_file.c_str ());

		bool FileOutput = OutputTripFile( fname_trip,g_RealTimeSimulationSettingsMap[timestamp_in_min].day_no ,true,true);
		
			if(FileOutput)
			{
				break;
			}
			else 
			{
				 Sleep(5000); // wait for 5 second
		
			 cout << "wait for 5 seconds... " << endl;
		
			}
		}

	}

		if(g_RealTimeSimulationSettingsMap[timestamp_in_min].break_and_wait_flag)
		{
		cout << "time clock =" << timestamp_in_min  << ", press any key to continue " << endl;
		
		getchar();

		// press any key will continue

		}

	if(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file.size() >=1)
	{
		

		if(timestamp_in_min == g_DemandLoadingEndTimeInMin)  // time-dependent travel time (from current day)
			g_AgentBasedAccessibilityMatrixGeneration(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file,true,timestamp_in_min);
		else  // use prevailing travel time at current time based on the last 15 min experienced link travel times
			g_AgentBasedAccessibilityMatrixGeneration(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file,false,timestamp_in_min);

		//ofstream output_ODTDMOE_file;

		//output_ODTDMOE_file.open (g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file.c_str ());
		////	output_ODImpact_file.open ("output_ImpactedOD.csv");
		//int department_time_interval = 15;
		//if(output_ODTDMOE_file.is_open ())
		//{
		//	cout << "     outputing " << g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file << endl;
		//	OutputTimeDependentODMOEData(output_ODTDMOE_file,department_time_interval, timestamp_in_min, 1);
		//	output_ODTDMOE_file.close();
		//	break;
		//}else
		//{
		//	
		//	cout << "Error: File " << g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file << " cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;

		//	cout << "wait for 5 seconds..." << endl;
		//	Sleep(5000);
		//}
		//}

		

	}


	if(g_RealTimeSimulationSettingsMap[timestamp_in_min].update_TD_link_attribute_file .size() >=1)
	{
		   // wait for update_TD_link_attribute_file;
		cout << "time clock =" << timestamp_in_min  << ", wait for file " << g_RealTimeSimulationSettingsMap[timestamp_in_min].update_TD_link_attribute_file << endl;

		while(1)
		{

			CCSVParser parser_link_TD_attribute;
			if (parser_link_TD_attribute.OpenCSVFile(g_RealTimeSimulationSettingsMap[timestamp_in_min].update_TD_link_attribute_file,false))
			{
					// updating file exists.
					parser_link_TD_attribute.CloseCSVFile ();

				g_UpdateRealTimeLinkMOEData(
					g_RealTimeSimulationSettingsMap[timestamp_in_min].update_TD_link_attribute_file,
					timestamp_in_min,
					g_RealTimeSimulationSettingsMap[timestamp_in_min].update_attribute_aggregation_time_interval_in_min );
				

				cout << "File " << g_RealTimeSimulationSettingsMap[timestamp_in_min].update_TD_link_attribute_file << " has been read. Continue" << endl;

				break;
			}else
			{
		
			 cout << "wait for 5 seconds... " << endl;
			  
			 Sleep(5000); // wait for 5 second
			
			}
		

		}
		
	}  // with input_TD_travel_time_file
	

	if(g_RealTimeSimulationSettingsMap[timestamp_in_min].update_trip_file .size() >=1)
	{
		   // wait for input_agent_updating_file;
		while(1)
		{

			bool b_trip_file_ready = g_ReadTripCSVFile(g_RealTimeSimulationSettingsMap[timestamp_in_min].update_trip_file.c_str (),false, false);

			int iteration  = 0;
			g_BuildPathsForAgents(iteration,false);

			if( b_trip_file_ready)
			{		
				break;
			}else
			{
			Sleep(5000); // wait for 5 second
			 cout << "wait for 5 seconds... " << endl;
			  
			}


		}
	}

	if(day_no == 0 && timestamp_in_min == 0 && g_RealTimeSimulationSettingsMap[timestamp_in_min].output_od_moe_file.size() >=1)  
		// at the first time interval  based on free_flow travel time
	{
		g_AgentBasedAccessibilityMatrixGeneration(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_od_moe_file,false,timestamp_in_min);
	
	}else if(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_od_moe_file.size() >=1)
	{

		ofstream output_ODMOE_file;

		output_ODMOE_file.open (g_RealTimeSimulationSettingsMap[timestamp_in_min].output_od_moe_file.c_str());
		//	output_ODImpact_file.open ("output_ImpactedOD.csv");
		if(output_ODMOE_file.is_open ())
		{

			int cut_off_volume = 1;
			OutputODMOEData(output_ODMOE_file,cut_off_volume,timestamp_in_min - 15);
		}
	}


}



void g_UpdateRealTimeLinkMOEData(std::string fname,int current_time_in_min, int update_MOE_aggregation_time_interval_in_min)
{
	CCSVParser parser_RTUpdateLinkMOE;
	if (parser_RTUpdateLinkMOE.OpenCSVFile(fname,false))
	{

		int record_count = 0;

	cout << "File input_real_time_simulation_settings.csv is opened."<< endl;
	g_LogFile << "File input_real_time_simulation_settings.csv is opened."<< endl;

		while(parser_RTUpdateLinkMOE.ReadRecord())
		{
			int from_node_id = 0 ;
			int to_node_id = 0;
			int timestamp_in_min = 0;

			parser_RTUpdateLinkMOE.GetValueByFieldName("from_node_id",from_node_id);
			parser_RTUpdateLinkMOE.GetValueByFieldName("to_node_id",to_node_id);
			parser_RTUpdateLinkMOE.GetValueByFieldName("timestamp_in_min",timestamp_in_min);

			float travel_time  = -1;

			parser_RTUpdateLinkMOE.GetValueByFieldName("travel_time_in_min",travel_time);

			if(g_LinkMap.find(GetLinkStringID(from_node_id,to_node_id))== g_LinkMap.end())
			{
				cout << "Link " << from_node_id << "-> " << to_node_id << " of file" << fname << " has not been defined in input_link.csv. Please check.";
				g_ProgramStop();
			}

			DTALink* pLink = g_LinkMap[GetLinkStringID(from_node_id,to_node_id)];

			if(pLink!=NULL)
			{

				int time = max(timestamp_in_min, current_time_in_min);
				pLink->UpdateFutureLinkAttribute(time, update_MOE_aggregation_time_interval_in_min, travel_time);
				cout << "Link " << from_node_id << "-> " << to_node_id << " has a updated travel time " << travel_time << " at time " << current_time_in_min << endl;
				g_LogFile << "Link " << from_node_id << "-> " << to_node_id << " has a updated travel time " << travel_time << " at time " << current_time_in_min << endl;

			}

		}

	}else
	{
		cout << "Error File " <<  fname << " cannot be opened. " << endl;
		getchar();

	}

}
