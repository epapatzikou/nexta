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


DTANetworkForSP g_PrevailingTimeNetwork_MP[_MAX_NUMBER_OF_PROCESSORS]; //  network instance for single processor in multi-thread environment: no more than 8 threads/cores
ODPathSet** g_ODPathSetVector = NULL;

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

	int AssignmentInterval = g_FindAssignmentIntervalIndexFromTime(departure_time_begin);  // starting assignment interval


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
				int CurrentLinkID = pVeh->m_LinkAry[pVeh->m_SimLinkSequenceNo].LinkNo;
				
				if( g_LinkVector[CurrentLinkID]->IsInsideEvacuationZoneFlag(DayNo, current_time))
				{
				pVeh->m_DestinationZoneID  = pVeh->m_DestinationZoneID_Updated;
				pVeh->m_DestinationNodeID =  g_ZoneMap[pVeh->m_DestinationZoneID].GetRandomDestinationIDInZone ((pVeh->m_VehicleID%100)/100.0f); 

				pVeh->m_InformationClass = info_en_route_and_pre_trip; // 3, so all the vehicles will access real-time information 
				pVeh->m_TimeToRetrieveInfo = 99999; // no more update

				b_switch_flag = true;
				DTAEvacuationRespone response;
				response.LinkNo = CurrentLinkID;
				response.ResponseTime = current_time;
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
		if(pVeh->m_InformationClass == info_en_route_and_pre_trip && pVeh->m_bLoaded == true &&
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
		bool bGeneralizedCostFlag = false;
		bool bDebugFlag = false;

		int StartingNodeID = pVeh->m_OriginNodeID;

		if(pVeh->m_InformationClass == info_en_route_and_pre_trip  && pVeh->m_bLoaded == true )  // en route info, has not reached destination yet
		{
			int CurrentLinkID = pVeh->m_LinkAry[pVeh->m_SimLinkSequenceNo].LinkNo;
			StartingNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

			if(pVeh->m_SimLinkSequenceNo == pVeh->m_NodeSize - 2)
				continue;

			if(StartingNodeID == pVeh->m_DestinationNodeID )  // you will reach the destination (on the last link).
				continue;

			// copy existing link number
			for(int i = 0; i<= pVeh->m_SimLinkSequenceNo; i++)
			{
				CurrentPathLinkList[i] = 	pVeh->m_LinkAry[i].LinkNo;
				AbsArrivalTimeOnDSNVector[i]= pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN ;
			}
		}


		// use agent-specific information look up window to update the prevailing travel time conditions 
		// step 1: update m_prevailing_travel_time for each link
		// step 2: call function BuildPhysicalNetwork again to use m_prevailing_travel_time to overwrite link travel time data
		// step 3: use link travel time data in agent-based routing 

		NodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, StartingNodeID , pVeh->m_DepartureTime , pVeh->m_DestinationZoneID , pVeh->m_DestinationNodeID, pVeh->m_PricingType , pVeh->m_VOT, PathLinkList, TotalCost,bGeneralizedCostFlag, bDebugFlag);


	if(pVeh->m_InformationClass == info_en_route_and_pre_trip && pVeh->m_bLoaded == true )  // en route info
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

			if( pVeh->m_LinkAry !=NULL)  // delete the old path
			{
				delete pVeh->m_LinkAry;
			}

			pVeh->m_NodeSize = NodeSize;


			if(pVeh->m_NodeSize>=2)  // for feasible path
			{

				if(NodeSize>=MAX_NODE_SIZE_IN_A_PATH)
				{
					cout << "PATH Size is out of bound of " << MAX_NODE_SIZE_IN_A_PATH << ": "<< NodeSize;
					g_ProgramStop();
				}

				pVeh->m_LinkAry = new SVehicleLink[NodeSize];

				if(pVeh->m_LinkAry==NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				int NodeNumberSum =0;
				float Distance =0;

				for(int i = 0; i< NodeSize-1; i++)
				{
					pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN = AbsArrivalTimeOnDSNVector[i];

					pVeh->m_LinkAry[i].LinkNo = PathLinkList[i];
					NodeNumberSum += PathLinkList[i];

					if(g_LinkVector[pVeh->m_LinkAry [i].LinkNo]!=NULL)
					{
						DTALink* pLink = g_LinkVector[pVeh->m_LinkAry [i].LinkNo];
						TRACE("Prevailing Travel Time %f \n", pLink->m_prevailing_travel_time );

					}
					
					ASSERT(pVeh->m_LinkAry [i].LinkNo < g_LinkVector.size());
					Distance+= g_LinkVector[pVeh->m_LinkAry [i].LinkNo] ->m_Length ;

				}
				
					
				pVeh->m_Distance  = Distance;
				pVeh->m_NodeNumberSum = NodeNumberSum;


			}
		}
	} // for each vehicle on this OD pair

}

void g_ApplyExternalPathInput(int departure_time_begin)
 // for pre-trip and en-route information user classes, for each departure time interval
{
	if(g_ODEstimationFlag==1)
		return;
	int AssignmentInterval = g_FindAssignmentIntervalIndexFromTime(departure_time_begin);  // starting assignment interval

	for(int origin_zone_index = 0; origin_zone_index <g_ODZoneIDSize; origin_zone_index++)
	{
		// loop through the TDOVehicleArray to assign or update vehicle paths...
		for (int vi = 0; vi<g_TDOVehicleArray[origin_zone_index][AssignmentInterval].VehicleArray.size(); vi++)
		{
			int VehicleID = g_TDOVehicleArray[origin_zone_index][AssignmentInterval].VehicleArray[vi];
			DTAVehicle* pVeh  = g_VehicleMap[VehicleID];

			g_UseExternalPath(pVeh);

		} // for each vehicle on this OD pair
	}
}

int g_number_of_CPU_threads()
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

	return number_of_threads;

}

int g_number_of_CPU_threads_for_real_time_routing()
{
	int number_of_threads = omp_get_max_threads();

	int max_number_of_threads = g_GetPrivateProfileInt("computation", "max_number_of_threads_for_real_time_routing", 1, g_DTASettingFileName);

	if (number_of_threads > max_number_of_threads)
		number_of_threads = max_number_of_threads;


	if (number_of_threads > _MAX_NUMBER_OF_PROCESSORS)
	{
		cout << "the number of threads is " << number_of_threads << ", which is greater than _MAX_NUMBER_OF_PROCESSORS. Please contact developers!" << endl;
		g_ProgramStop();
	}

	return number_of_threads;

}

void g_ShortestPathDataMemoryAllocation()  
// for VMS responsive vehicles
{


	if (g_use_routing_policy_from_external_input == 1 )
	{	
		g_ODPathSetVector = AllocateDynamicArray<ODPathSet>(g_ODZoneIDSize+1,g_ODZoneIDSize+1);
	}

	int node_size = g_NodeVector.size() + 1+  g_ODZoneIDSize;
	int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

	cout << "Allocate Memory: node_size= " << node_size << ",link_size =" << link_size << endl;


	// under zone based mode, we add connectors for each destination 
	int number_of_connectors = 0;
	std::map<int, DTAZone>::iterator iterZone;
	for (iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
	{
		DTAZone zone = iterZone->second;
		number_of_connectors += zone.m_DestinationActivityVector.size();
	}

	link_size += number_of_connectors;

	int number_of_threads_for_real_time_routing = g_number_of_CPU_threads_for_real_time_routing();
	for (int ProcessID = 0; ProcessID < number_of_threads_for_real_time_routing; ProcessID++)
	{
	g_PrevailingTimeNetwork_MP[ProcessID].Setup(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin);
	//special notes: creating network with dynamic memory is a time-consumping task, so we create the network once for each processors
	g_PrevailingTimeNetwork_MP[ProcessID].BuildPhysicalNetwork(0, 0, g_TrafficFlowModelFlag, true, g_DemandLoadingStartTimeInMin);
	}
	int number_of_threads = g_number_of_CPU_threads();

	for (int ProcessID = 0; ProcessID < number_of_threads; ProcessID++)
	{
		g_TimeDependentNetwork_MP[ProcessID].Setup(node_size, link_size, g_PlanningHorizon, g_AdjLinkSize, g_DemandLoadingStartTimeInMin, g_ODEstimationFlag);
	}


	cout << "------- Memory allocation completed.-------" << endl;

}
void g_UpdateRealTimeInformation(double CurrentTime)
{
	int number_of_threads_for_real_time_routing = g_number_of_CPU_threads_for_real_time_routing();
	for (int ProcessID = 0; ProcessID < number_of_threads_for_real_time_routing; ProcessID++)
	{
		g_PrevailingTimeNetwork_MP[ProcessID].UpdateCurrentTravelTime(0, CurrentTime);
	}
}
void g_AgentBasedPathAdjustmentWithRealTimeInfo(int ProcessID, int VehicleID , double current_time)
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


		int StartingNodeID = pVeh->m_OriginNodeID;

		float TotalCost = 0;
		bool bGeneralizedCostFlag = false;
		bool bDebugFlag = false;
		int count = 0;

		if (pVeh->m_NodeSize >= 1) // with path
		{

		// if this is a pre-trip vehicle, and he has not obtained real-time information yet



			int CurrentLinkID = pVeh->m_LinkAry[pVeh->m_SimLinkSequenceNo].LinkNo;
			StartingNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

			if(StartingNodeID == pVeh->m_DestinationNodeID )  // you will reach the destination (on the last link).
				return;

			// copy existing link number (and remaining links)

			for(int i = 0; i< pVeh->m_NodeSize -1; i++)
			{
				CurrentPathLinkList[i] = 	pVeh->m_LinkAry[i].LinkNo;
				AbsArrivalTimeOnDSNVector[i]= pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN ;
			}


		count = pVeh->m_SimLinkSequenceNo;

			//if(is.Type == 2) // detour VMS
			//{
			//for(int ll = 0; ll < is.DetourLinkSize ; ll++)
			//{
			//PathLinkList[l++] = is.DetourLinkArray [ll];
			//CurrentTime+= g_LinkVector[is.DetourLinkArray [ll]]->m_FreeFlowTravelTime;  // add travel time along the path to get the starting time at the end of subpath
			//}

			//CurrentNodeID = g_LinkVector[is.DetourLinkArray [is.DetourLinkSize -1]]->m_ToNodeID;

			//}
		}
		if(pVeh->m_OriginNodeID < 0 || pVeh->m_DestinationNodeID <0)
		{

		cout << "m_OriginNodeID < 0 || m_DestinationNodeID <0" << endl;
		getchar();
		
		}
		 

		NodeSize = g_PrevailingTimeNetwork_MP[ProcessID].FindBestPathWithVOT(pVeh->m_OriginZoneID, StartingNodeID, pVeh->m_DepartureTime, pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, PathLinkList, TotalCost, bGeneralizedCostFlag, bDebugFlag);


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
//#ifdef  _large_memory_usage_lr
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

			if( pVeh->m_LinkAry !=NULL)  // delete the old path
			{
				delete pVeh->m_LinkAry;
			}




			pVeh->m_NodeSize = NodeSize;


			if(pVeh->m_NodeSize>=2)  // for feasible path
			{

				if(NodeSize>=MAX_NODE_SIZE_IN_A_PATH)
				{
					cout << "PATH Size is out of bound: " << MAX_NODE_SIZE_IN_A_PATH << NodeSize;
					g_ProgramStop();
				}

				pVeh->m_LinkAry = new SVehicleLink[NodeSize];

				if(pVeh->m_LinkAry==NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				int NodeNumberSum =0;
				float Distance =0;

				for(int i = 0; i< NodeSize-1; i++)
				{
					pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN = AbsArrivalTimeOnDSNVector[i];

					pVeh->m_LinkAry[i].LinkNo = PathLinkList[i];
					NodeNumberSum += PathLinkList[i];

					/*if(g_LinkVector[pVeh->m_LinkAry [i].LinkNo]==NULL)
					{
					cout << "Error: g_LinkVector[pVeh->m_LinkAry [i].LinkNo]==NULL", pVeh->m_LinkAry [i].LinkNo;
					getchar();
					exit(0);
					}
					*/
					ASSERT(pVeh->m_LinkAry [i].LinkNo < g_LinkVector.size());
					Distance+= g_LinkVector[pVeh->m_LinkAry [i].LinkNo] ->m_Length ;

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
		bool bGeneralizedCostFlag = false;
		bool bDebugFlag = false;

		int StartingNodeID = pVeh->m_OriginNodeID;

			int CurrentLinkID = pVeh->m_LinkAry[pVeh->m_SimLinkSequenceNo].LinkNo;
			StartingNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

			if(StartingNodeID == pVeh->m_DestinationNodeID )  // you will reach the destination (on the last link).
				return;

			// copy existing link number (and remaining links)

			for(int i = 0; i< pVeh->m_NodeSize -1; i++)
			{
				CurrentPathLinkList[i] = 	pVeh->m_LinkAry[i].LinkNo;
				AbsArrivalTimeOnDSNVector[i]= pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN ;
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
		NodeSize = g_PrevailingTimeNetwork_MP[processor_id].FindBestPathWithVOT(pVeh->m_OriginZoneID, StartingNodeID , pVeh->m_DepartureTime , pVeh->m_DestinationZoneID , pVeh->m_DestinationNodeID, pVeh->m_PricingType , pVeh->m_VOT, PathLinkList, TotalCost,bGeneralizedCostFlag, bDebugFlag);


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
//#ifdef  _large_memory_usage_lr
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

			if( pVeh->m_LinkAry !=NULL)  // delete the old path
			{
				delete pVeh->m_LinkAry;
			}




			pVeh->m_NodeSize = NodeSize;


			if(pVeh->m_NodeSize>=2)  // for feasible path
			{

				if(NodeSize>=MAX_NODE_SIZE_IN_A_PATH)
				{
					cout << "PATH Size is out of bound: " << MAX_NODE_SIZE_IN_A_PATH << NodeSize;
					g_ProgramStop();
				}

				pVeh->m_LinkAry = new SVehicleLink[NodeSize];

				if(pVeh->m_LinkAry==NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				int NodeNumberSum =0;
				float Distance =0;

				for(int i = 0; i< NodeSize-1; i++)
				{
					pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN = AbsArrivalTimeOnDSNVector[i];

					pVeh->m_LinkAry[i].LinkNo = PathLinkList[i];
					NodeNumberSum += PathLinkList[i];

					/*if(g_LinkVector[pVeh->m_LinkAry [i].LinkNo]==NULL)
					{
					cout << "Error: g_LinkVector[pVeh->m_LinkAry [i].LinkNo]==NULL", pVeh->m_LinkAry [i].LinkNo;
					getchar();
					exit(0);
					}
					*/
					ASSERT(pVeh->m_LinkAry [i].LinkNo < g_LinkVector.size());
					Distance+= g_LinkVector[pVeh->m_LinkAry [i].LinkNo] ->m_Length ;

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

		if (g_UEAssignmentMethod != assignment_real_time_simulation)
		{
			cout << "File input_real_time_simulation_settings.csv exists in the project folder." << endl;
			cout << "If you want to use real time simulation mode, please make sure the traffic_assignment_method = 8 in input_scenario_settings.csv as assignment_real_time_simulation" << endl;
			cout << "If you do not want to use real time simulation mode, please rename or delete file input_real_time_simulation_settings.csv." << endl;
			g_ProgramStop();
		}

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


		int real_time_skim_file_output = g_GetPrivateProfileInt("ABM_integeration", "real_time_skim_file_output", 1, g_DTASettingFileName);

		if (g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file.size() >= 1 && real_time_skim_file_output ==1)
	{
		

		if(timestamp_in_min == g_DemandLoadingEndTimeInMin)  // time-dependent travel time (from current day)
			g_AgentBasedAccessibilityMatrixGeneration(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file,true,1,timestamp_in_min);
		else  // use prevailing travel time at current time based on the last 15 min experienced link travel times
			g_AgentBasedAccessibilityMatrixGeneration(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file,false,1,timestamp_in_min);

		int real_time_HOV_skim_file_output = g_GetPrivateProfileInt("ABM_integeration", "real_time_HOV_skim_file_output", 1, g_DTASettingFileName);

		if (real_time_HOV_skim_file_output == 1)
		{
			if(timestamp_in_min == g_DemandLoadingEndTimeInMin)  // time-dependent travel time (from current day)
				g_AgentBasedAccessibilityMatrixGeneration("HOV_"+g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file,true,2,timestamp_in_min);
			else  // use prevailing travel time at current time based on the last 15 min experienced link travel times
				g_AgentBasedAccessibilityMatrixGeneration("HOV_"+g_RealTimeSimulationSettingsMap[timestamp_in_min].output_td_skim_file,false,2,timestamp_in_min);
		}

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
			int number_of_vehicles = 0;
			bool b_trip_file_ready = g_ReadTripCSVFile(g_RealTimeSimulationSettingsMap[timestamp_in_min].update_trip_file.c_str(), false, number_of_vehicles);

			int iteration  = 0;

			bool bRebuildNetwork = false;
			
			//if(timestamp_in_min%15 ==0)  //rebuild the shoret path network every 15 min
			//	bRebuildNetwork = true;


			// use input link travel time 
			g_BuildPathsForAgents(iteration,bRebuildNetwork,false,timestamp_in_min, timestamp_in_min+15);

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
		g_AgentBasedAccessibilityMatrixGeneration(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_od_moe_file,false,1,timestamp_in_min);

		int time_dependent_HOV_skim_file_output = g_GetPrivateProfileInt("ABM_integeration", "time_dependent_HOV_skim_file_output", 0, g_DTASettingFileName);

		if(time_dependent_HOV_skim_file_output == 1)
		{
			g_AgentBasedAccessibilityMatrixGeneration(g_RealTimeSimulationSettingsMap[timestamp_in_min].output_od_moe_file,false,2,timestamp_in_min);
		}
	
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

void g_RealTimeDynamicToll(int day_no, int timestamp_in_min)
{
	// update timestamp using day no
	timestamp_in_min = day_no * 1440 + timestamp_in_min;

	// part I: determine dynamic tolling price 
	for (unsigned li = 0; li < g_LinkVector.size(); li++)
	{
		DTALink * pLink = g_LinkVector[li];


		if (pLink->m_DynamicTollType == 1)
		{
			int PricingType = 1;
			float speed = pLink->m_Length / max(pLink->m_FreeFlowTravelTime, pLink->m_prevailing_travel_time) * 60;  // mile per hour
			float speed_limit_over_speed = pLink->m_SpeedLimit / max(0.01, speed);
			float voc = pow((speed_limit_over_speed - 1) / 0.15, 0.25);
			float min_rate = 0.25;
			float max_rate = 3;
			float dynamic_toll_rate = 1;
			dynamic_toll_rate = (max_rate - min_rate) * pow(voc - 0.15, 2) * pLink->pTollVector[0].TollRate[2];
			if (dynamic_toll_rate < min_rate)
				dynamic_toll_rate = min_rate;

			if (dynamic_toll_rate > max_rate)
				dynamic_toll_rate = max_rate;


			pLink->pTollVector[0].TollRate[PricingType] = dynamic_toll_rate;

			if (g_DebugLogFile != NULL)
			{
				fprintf(g_DebugLogFile, "\nDay NO: %d, Min: %d, Link %d->%d, speed: %f, speed_limit_over_speed: %f, voc: %f, rate: %f  ", day_no, timestamp_in_min, pLink->m_FromNodeNumber, pLink->m_ToNodeNumber,
					speed, speed_limit_over_speed, voc, dynamic_toll_rate);
			}

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

bool g_ReadRealTimeLinkAttributeData(int current_time_in_second)
{
	
	char fname[_MAX_PATH];

	sprintf(fname,".//real_time_data_exchange//input_link_attribute_sec_%d.csv", current_time_in_second);

	CCSVParser parser_RTLinkAttribute;
	if (parser_RTLinkAttribute.OpenCSVFile(fname, false))
	{

		int record_count = 0;

		cout << "File " << fname <<" is opened." << endl;
		g_LogFile << "File " << fname << " is opened." << endl;

		while (parser_RTLinkAttribute.ReadRecord())
		{
			std::string link_key;

			parser_RTLinkAttribute.GetValueByFieldName("link_key", link_key);

			if (g_LinkKeyMap.find(link_key) != g_LinkKeyMap.end())
			{
				DTALink* pLink = g_LinkKeyMap[link_key];
					if (pLink != NULL)
					{

						//link_inflow_capacity, link_outflow_capacity, link_storage_capacity, speed_limit, pricing_demand_type%d

						float link_inflow_capacity, link_outflow_capacity, link_storage_capacity, speed_limit;
						if (parser_RTLinkAttribute.GetValueByFieldName("link_inflow_capacity", link_inflow_capacity) == true)
						{
							// update numnber of lanes, so the link in flow capacity is updated. 
							pLink->m_OutflowNumLanes = link_inflow_capacity /1800; 
						}

						if (parser_RTLinkAttribute.GetValueByFieldName("link_outflow_capacity", link_outflow_capacity) == true)
						{
							// update numnber of lanes, so the link in flow capacity is updated. 
							pLink->m_OutflowNumLanes = link_outflow_capacity / max(1, pLink->GetHourlyPerLaneCapacity(current_time_in_second));
						}

						if (parser_RTLinkAttribute.GetValueByFieldName("link_storage_capacity", link_storage_capacity) == true)
						{
							// update numnber of lanes, so the link in flow capacity is updated. 
							pLink->m_VehicleSpaceCapacity = max(1,link_storage_capacity);
						}

						if (parser_RTLinkAttribute.GetValueByFieldName("speed_limit", speed_limit) == true)
						{
							// update numnber of lanes, so the link in flow capacity is updated. 
							pLink->m_SpeedLimit  = max(1, speed_limit);
						}					
						
						for (int i = 1; i < MAX_PRICING_TYPE_SIZE; i++)
						{
							char field_name[_MAX_PATH];
							sprintf(field_name, "pricing_demand_type%d", i);

							float pricing_rate = 0;
							if (parser_RTLinkAttribute.GetValueByFieldName(field_name, pricing_rate) == true)
							{
								// update numnber of lanes, so the link in flow capacity is updated. 
								pLink->RealTimePricingRate[i] = max(1, pricing_rate);
							}
						}

					}
			}
		}

		return true;
	}
	else
	{
		return false;
	}

	return true;

}



bool g_ReadRealTimeTripData(int current_time_in_minute, bool b_InitialLoadingFlag)
{
	float start_time_value = -100;

	if (b_InitialLoadingFlag)
	{ //allocate memory

		g_AllocateDynamicArrayForVehicles();
	}

	char file_name[_MAX_PATH];

	sprintf(file_name, ".//real_time_data_exchange//input_trip_min_%d.csv", current_time_in_minute);

	bool bOutputLogFlag = true;
	CCSVParser parser_agent;


	float total_number_of_vehicles_to_be_generated = 0;

	if (parser_agent.OpenCSVFile(file_name, false))
	{

		if (bOutputLogFlag)
		{
			cout << "reading file " << file_name << endl;
		}
		int line_no = 1;

		int i = 0;

		int count = 0;

		int count_for_sameOD = 0;
		int count_for_not_defined_zones = 0;

		while (parser_agent.ReadRecord())
		{

			if ((count + 1) % 1000 == 0 && bOutputLogFlag)
			{
				cout << "reading " << count + 1 << " records..." << endl;

			}
			count++;

			int trip_id = 0;

			parser_agent.GetValueByFieldNameRequired("trip_id", trip_id);
			DTAVehicle* pVehicle = 0;

			pVehicle = new (std::nothrow) DTAVehicle;
			if (pVehicle == NULL)
			{
				cout << "Insufficient memory...";
				getchar();
				exit(0);

			}

			pVehicle->m_VehicleID = g_VehicleVector.size();

			pVehicle->m_ExternalTripID = trip_id;


			pVehicle->m_RandomSeed = pVehicle->m_VehicleID;

			parser_agent.GetValueByFieldNameRequired("from_zone_id", pVehicle->m_OriginZoneID);
			parser_agent.GetValueByFieldNameRequired("to_zone_id", pVehicle->m_DestinationZoneID);

			if (g_ZoneMap.find(pVehicle->m_OriginZoneID) == g_ZoneMap.end() || g_ZoneMap.find(pVehicle->m_DestinationZoneID) == g_ZoneMap.end())
			{
				count_for_not_defined_zones++;

				continue;
			}



			int origin_node_id = -1;
			int origin_node_number = -1;

			parser_agent.GetValueByFieldName("origin_node_id", origin_node_number);

			if (g_NodeNametoIDMap.find(origin_node_number) != g_NodeNametoIDMap.end())  // convert node number to internal node id
			{
				origin_node_id = g_NodeNametoIDMap[origin_node_number];
			}

			int destination_node_id = -1;
			int destination_node_number = -1;
			parser_agent.GetValueByFieldName("destination_node_id", destination_node_number);

			if (g_NodeNametoIDMap.find(destination_node_number) != g_NodeNametoIDMap.end()) // convert node number to internal node id
			{
				destination_node_id = g_NodeNametoIDMap[destination_node_number];
			}

			if (origin_node_id == -1)  // no default origin node value, re-generate origin node
				origin_node_id = g_ZoneMap[pVehicle->m_OriginZoneID].GetRandomOriginNodeIDInZone((pVehicle->m_VehicleID % 100) / 100.0f);  // use pVehicle->m_VehicleID/100.0f as random number between 0 and 1, so we can reproduce the results easily

			if (destination_node_id == -1)// no default destination node value, re-destination origin node
				destination_node_id = g_ZoneMap[pVehicle->m_DestinationZoneID].GetRandomDestinationIDInZone((pVehicle->m_VehicleID % 100) / 100.0f);

			pVehicle->m_OriginNodeID = origin_node_id;
			pVehicle->m_DestinationNodeID = destination_node_id;


			if (origin_node_id == destination_node_id)
			{  // do not simulate intra zone traffic

				count_for_sameOD++;
				continue;
			}

			if (g_ZoneMap.find(pVehicle->m_OriginZoneID) != g_ZoneMap.end())
			{
				g_ZoneMap[pVehicle->m_OriginZoneID].m_Demand += 1;
				g_ZoneMap[pVehicle->m_OriginZoneID].m_OriginVehicleSize += 1;

			}

			float departure_time = 0;
			parser_agent.GetValueByFieldNameRequired("start_time_in_min", departure_time);


			if (start_time_value < 0)  // set first value
				start_time_value = departure_time;
			else if (start_time_value > departure_time + 0.00001)  // check if the departure times are sequential
			{
				departure_time = start_time_value; // use a larger value 
				start_time_value = departure_time;
			}

			pVehicle->m_DepartureTime = departure_time;
			int beginning_departure_time = departure_time;


			if (pVehicle->m_DepartureTime < g_DemandLoadingStartTimeInMin || pVehicle->m_DepartureTime > g_DemandLoadingEndTimeInMin)
			{

				cout << "Error: trip_id " << trip_id << " in file " << file_name << " has a start time of " << pVehicle->m_DepartureTime << ", which is out of the demand loading range: " <<
					g_DemandLoadingStartTimeInMin << "->" << g_DemandLoadingEndTimeInMin << " (min)." << endl << "Please change the setting in section agent_input, demand_loading_end_time_in_min in file DTASettings.txt";
				g_ProgramStop();
			}

			if (parser_agent.GetValueByFieldName("demand_type", pVehicle->m_DemandType) == true)
			{
				parser_agent.GetValueByFieldName("pricing_type", pVehicle->m_PricingType);
				parser_agent.GetValueByFieldName("vehicle_type", pVehicle->m_VehicleType);
				parser_agent.GetValueByFieldName("information_type", pVehicle->m_InformationClass);
				parser_agent.GetValueByFieldName("value_of_time", pVehicle->m_VOT);
				parser_agent.GetValueByFieldName("vehicle_age", pVehicle->m_Age);
			}
			else
			{


			}





			int number_of_nodes = 0;
			parser_agent.GetValueByFieldName("number_of_nodes", number_of_nodes);

			std::vector<int> path_node_sequence;
			if (number_of_nodes >= 2)
			{
				string path_node_sequence_str;
				parser_agent.GetValueByFieldName("path_node_sequence", path_node_sequence_str);

				path_node_sequence = ParseLineToIntegers(path_node_sequence_str);

				AddPathToVehicle(pVehicle, path_node_sequence, file_name);
			}
			else if (g_use_routing_policy_from_external_input)
			{
					cout << " use external file from routing policy for vehicle " << pVehicle->m_VehicleID << endl;

					g_UseExternalPath(pVehicle);
			}
			
			
			pVehicle->m_TimeToRetrieveInfo = pVehicle->m_DepartureTime;
			pVehicle->m_ArrivalTime = 0;
			pVehicle->m_bComplete = false;
			pVehicle->m_bLoaded = false;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions = 0;
			pVehicle->m_Distance = 0;

			pVehicle->m_NodeSize = 0;

			pVehicle->m_NodeNumberSum = 0;
			pVehicle->m_Distance = 0;

			int number_of_agents = 1;

			float ending_departure_time = 0;

			g_VehicleVector.push_back(pVehicle);
			g_VehicleMap[pVehicle->m_VehicleID] = pVehicle;

			int AssignmentInterval = g_FindAssignmentIntervalIndexFromTime(pVehicle->m_DepartureTime);

			ASSERT(pVehicle->m_OriginZoneID <= g_ODZoneNumberSize);

			g_TDOVehicleArray[g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo][AssignmentInterval].VehicleArray.push_back(pVehicle->m_VehicleID);

			i++;
		}


		line_no++;



		if (bOutputLogFlag)
		{

			cout << count << " records have been read from file " << file_name << endl;

			cout << i << " agents have been read from file " << file_name << endl;

			if (count_for_sameOD >= 1)
				cout << "there are " << count_for_sameOD << " agents with the same from_zone_id and to_zone_id, which will not be simulated. " << endl;


			if (count_for_not_defined_zones >= 1)
				cout << "there are " << count_for_not_defined_zones << " agents with zones not being defined in input_zone.csv file, which will not be simulated. " << endl;

		}
	}
	else
	{
		cout << "Waiting for file " << file_name << "... " << endl;

		return false;
	}

	return true;
}

