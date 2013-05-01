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


using namespace std;


DTANetworkForSP g_network_MP[_MAX_NUMBER_OF_PROCESSORS]; //  network instance for single processor in multi-thread environment: no more than 8 threads/cores

DTANetworkForSP g_network_VMS;

void g_AgentBasedPathAdjustment(int DayNo, double CurrentTime )  // for pre-trip and en-route information user classes
{
	if(g_ODEstimationFlag==1)
		return;
	int number_of_threads = omp_get_max_threads ( );


	if(DayNo==0 && g_network_MP[0].m_NodeSize ==0 )  // allocate memory once 
	{
	// reset random number seeds
	int node_size  = g_NodeVector.size() +1 + g_ODZoneNumberSize;
	int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

	// assign different zones to different processors

//	if(g_ODEstimationFlag==1)  // single thread mode for ODME 
//		number_of_threads = 1;

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
	}

	}

//#pragma omp parallel for
		for(int ProcessID=0;  ProcessID < number_of_threads; ProcessID++)
		{
			// create network for shortest path calculation at this processor
			int	id = omp_get_thread_num( );  // starting from 0

			//special notes: creating network with dynamic memory is a time-consumping task, so we create the network once for each processors
			g_network_MP[ProcessID].BuildPhysicalNetwork(0,0,g_TrafficFlowModelFlag, true, CurrentTime );


			for(int CurZoneID=1;  CurZoneID <= g_ODZoneNumberSize; CurZoneID++)
			{

				if((CurZoneID%number_of_threads) == ProcessID)  // if the remainder of a zone id (devided by the total number of processsors) equals to the processor id, then this zone id is 
				{

					// scan all possible departure times
					for(int departure_time = g_DemandLoadingStartTimeInMin; departure_time < CurrentTime; departure_time += g_AggregationTimetInterval)
					{

						if(g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray .size() > 0)
						{

							g_network_MP[id].AgentBasedPathAdjustment(DayNo, CurZoneID,departure_time,CurrentTime);

						}
					}  // for each departure time
				}
			}  // for each zone
		} // for each computer processor


}


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

	int AssignmentInterval = int(departure_time_begin/g_AggregationTimetInterval);  // starting assignment interval

	if(AssignmentInterval>= g_DemandLoadingEndTimeInMin/g_AggregationTimetInterval)
	{
		// reset assignment time interval 
	AssignmentInterval = g_DemandLoadingEndTimeInMin/g_AggregationTimetInterval-1;
	
	}

	// loop through the TDOVehicleArray to assign or update vehicle paths...
	for (int vi = 0; vi<g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray.size(); vi++)
	{
		int VehicleID = g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray[vi];
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
				
				pVeh->m_EvacuationResponseVector.push_back (response);

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

				if(NodeSize>=900)
				{
					cout << "PATH Size >900 " << NodeSize;
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
	if(DayNo==0 && g_network_VMS.m_NodeSize == 0)  // allocate memory once 
	{
	int node_size  = g_NodeVector.size() +1 + g_ODZoneNumberSize;
	int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

		g_network_VMS.Setup(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin);
	//special notes: creating network with dynamic memory is a time-consumping task, so we create the network once for each processors

	}
	g_network_VMS.BuildPhysicalNetwork(0,0,g_TrafficFlowModelFlag, true, CurrentTime );

}


void g_AgentBasedVMSPathAdjustmentWithRealTimeInfo(int VehicleID , double current_time)
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


			// add link to VMS respons link

			bool bLinkAdded = false;
			for(int i =0; i< pVeh->m_VMSResponseVector.size(); i++)
			{
				if(pVeh->m_VMSResponseVector[i].LinkNo == CurrentLinkID)
				{
					bLinkAdded = true;
					break;
				}

			}

			if(!bLinkAdded)
			{
				DTAVMSRespone vms;
				vms.LinkNo = CurrentLinkID;
				vms.ResponseTime = current_time;
				vms.SwitchFlag = bSwitchFlag;
				pVeh->m_VMSResponseVector.push_back (vms);

			}

		if(bSwitchFlag==0)
			return;

			if( pVeh->m_NodeAry !=NULL)  // delete the old path
			{
				delete pVeh->m_NodeAry;
			}




			pVeh->m_NodeSize = NodeSize;


			if(pVeh->m_NodeSize>=2)  // for feasible path
			{

				if(NodeSize>=900)
				{
					cout << "PATH Size >900 " << NodeSize;
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


