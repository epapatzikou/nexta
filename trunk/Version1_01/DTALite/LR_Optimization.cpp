//  Portions Copyright 2014 Xuesong Zhou; Lagrangian-relaxation based, agent-based time-depenedent network optimization 
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

void g_AgentBasedOptimization()  // this is an adaptation of OD trip based assignment, we now generate and assign path for each individual vehicle (as an agent with personalized value of time, value of reliability)
{
	// reset random number seeds
	int node_size = g_NodeVector.size() + 1 + g_ODZoneNumberSize;
	int link_size = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

	cout << ":: start agent-based optimization " << g_GetAppRunningTime() << endl;
	g_LogFile << ":: start agent-based optimization " << g_GetAppRunningTime() << endl;


	int iteration = 0;
	bool NotConverged = true;
	int TotalNumOfVehiclesGenerated = 0;

	
	//cout << "------- Allocating memory for networks for " number_of_threads << " CPU threads" << endl;
	//cout << "Tips: If your computer encounters a memory allocation problem, please open file DTASettings.txt in the project folder " << endl;
	//cout << "find section [computation], set max_number_of_threads_to_be_used=1 or a small value to reduce memory usage. " << endl;
	//cout << "This modification could significantly increase the total runing time as a less number of CPU threads will be used. " << endl;

	//
	//" number_of_threads << " CPU threads" << endl;

	int number_of_threads = omp_get_max_threads();
	//	if(g_ODEstimationFlag==1)  // single thread mode for ODME 
	//		number_of_threads = 1;

	cout << "# of Computer Processors = " << number_of_threads << endl;


	number_of_threads = g_number_of_CPU_threads();

#pragma omp parallel for
	for (int ProcessID = 0; ProcessID < number_of_threads; ProcessID++)
	{

		// create network for shortest path calculation at this processor
		int	id = omp_get_thread_num();  // starting from 0
		g_TimeDependentNetwork_MP[id].BuildPhysicalNetwork(iteration, -1, g_TrafficFlowModelFlag);  // build network for this zone, because different zones have different connectors...
		g_TimeDependentNetwork_MP[id].InitializeTDLinkCost();
	}

	// ----------* start of outer loop *----------
	for (iteration = 0; NotConverged && iteration <= g_NumberOfIterations; iteration++)  // we exit from the loop under two conditions (1) converged, (2) reach maximum number of iterations
	{
		cout << "------- Iteration = " << iteration + 1 << "--------" << endl;

		g_CurrentGapValue = 0.0;
		g_CurrentRelativeGapValue = 0.0;
		g_CurrentNumOfVehiclesForUEGapCalculation = 0;
		g_CurrentNumOfVehiclesSwitched = 0;
		g_NewPathWithSwitchedVehicles = 0;

		//subgraident 
		//sub problem 1: time-dependent least path finding for each agent
		g_OptimizePathsForAgents(iteration, true, true, g_DemandLoadingStartTimeInMin, g_DemandLoadingEndTimeInMin);

		//subproblem 2: knapsack problem for selecting links to be constructed
		g_NetworkDesignKnapsackProblem(iteration, true, true, g_DemandLoadingStartTimeInMin, g_DemandLoadingEndTimeInMin);

		//cout << "---- Network Loading for Iteration " << iteration << "----" << endl;

		//NetworkLoadingOutput SimuOutput;
		//SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag, 0, iteration);


		//g_GenerateSimulationSummary(iteration, NotConverged, TotalNumOfVehiclesGenerated, &SimuOutput);

	}  // for each assignment iteration

	cout << "Writing Vehicle Trajectory and MOE File... " << endl;

	if (iteration == g_NumberOfIterations)
	{
		iteration = g_NumberOfIterations - 1;  //roll back to the last iteration if the ending condition is triggered by "iteration < g_NumberOfIterations"
	}

	g_OutputMOEData(iteration);

}

void 	DTANetworkForSP::InitializeTDLinkCost()
{
	unsigned li;
	for (li = 0; li < g_LinkVector.size(); li++)
	{
		DTALink* pLink = g_LinkVector[li];

		int ti;
		//initialization
		for (ti = 0; ti < m_NumberOfTDSPCalculationIntervals; ti += 1)
		{
			TD_LinkCostAry[pLink->m_LinkNo][ti] = 0.0;
		}
	}
}



void 	DTANetworkForSP::ResourcePricing_Subgraident(int iteration)
{
	// 

	float stepsize = 0.01;

	if (iteration < 20)   //MSA
		stepsize = 1 / (iteration + 1);
	else
		stepsize = 0.05;

	unsigned li;
	for (li = 0; li < g_LinkVector.size(); li++)
	{
		DTALink* pLink = g_LinkVector[li];

		int ti;
		//initialization
		for (ti = 0; ti < m_NumberOfTDSPCalculationIntervals; ti += 1)
		{
			TD_LinkCostAry[pLink->m_LinkNo][ti] = 0.0;

			if (iteration == 0)
			{
				TD_LinkVolumeAry[pLink->m_LinkNo][ti] = 0.0;
			}
		}


		float subgradient = 0;

		// determien the price
		if (pLink->TollVector.size() > 0)
		{
			for (ti = 0; ti < m_NumberOfTDSPCalculationIntervals; ti += 1)
			{

				subgradient = (TD_LinkVolumeAry[pLink->m_LinkNo][ti] - pLink->m_NetworkDesignBuildCapacity);

				TD_LinkCostAry[pLink->m_LinkNo][ti] += subgradient * stepsize;

				if (TD_LinkCostAry[pLink->m_LinkNo][ti] < 0)   // resource price must be positive or zero
					TD_LinkCostAry[pLink->m_LinkNo][ti] = 0;

			}

		}

		//initialization
		for (int ti = 0; ti < m_NumberOfTDSPCalculationIntervals; ti += 1)
		{
			TD_LinkVolumeAry[pLink->m_LinkNo][ti] = 0.0;
		}



	}
}

void g_NetworkDesignKnapsackProblem(int iteration, bool bRebuildNetwork, bool bOutputLog, int DemandLoadingStartTime, int DemandLoadingEndTime)
{

	// create this toll vector
	std::vector<VehicleLinkPrice> Global_PersonalizedRoadPriceVector;

	unsigned li;
	for (li = 0; li < g_LinkVector.size(); li++)
	{
		DTALink* pLink = g_LinkVector[li];
		if (pLink->TollVector.size() > 0)
		{
			VehicleLinkPrice element;
			element.LinkNo = pLink->m_LinkNo;
			element.RoadPrice = 0;
			element.RoadUsageFlag = 0;
			element.TotalTollColected = 0;

			Global_PersonalizedRoadPriceVector.push_back(element);

		}
	}


	// collect the toll from the memory of all vehicles, regardless if a link is used or not by the path
	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

		if (pVehicle->m_NodeSize >= 2)  // with physical path in the network
		{ 


			for (li = 0; li < pVehicle->m_PersonalizedRoadPriceVector.size(); li++)
			{
				Global_PersonalizedRoadPriceVector[li].RoadPrice  += pVehicle->m_PersonalizedRoadPriceVector[li].RoadPrice;
				Global_PersonalizedRoadPriceVector[li].TotalTollColected += pVehicle->m_PersonalizedRoadPriceVector[li].RoadPrice;
				Global_PersonalizedRoadPriceVector[li].RoadUsageFlag += pVehicle->m_PersonalizedRoadPriceVector[li].RoadUsageFlag;

			}

		}

	}

	// sort according to the toll collected

	std::sort(Global_PersonalizedRoadPriceVector.begin(), Global_PersonalizedRoadPriceVector.end());

	CString str;
	int K = 1;
	for (li = 0; li < Global_PersonalizedRoadPriceVector.size(); li++)
	{
	
		int LinkNo = Global_PersonalizedRoadPriceVector[li].LinkNo;
		DTALink* pLink = g_LinkVector[LinkNo];

		TRACE("\nLink %d -> %d: Price = %f ", pLink->m_FromNodeNumber, pLink->m_ToNodeNumber, Global_PersonalizedRoadPriceVector[li].RoadPrice );
		str.Format("Link %d -> %d: Price = %f Useage = %d", pLink->m_FromNodeNumber, pLink->m_ToNodeNumber, Global_PersonalizedRoadPriceVector[li].RoadPrice, Global_PersonalizedRoadPriceVector[li].RoadUsageFlag);

			g_AssignmentLogFile << "\niteration no. " << iteration << " " << str << endl;
		if (li < K)
			pLink->m_NetworkDesignBuildCapacity = 1;
		else
			pLink->m_NetworkDesignBuildCapacity = 0;

	}
}

void g_OptimizePathsForAgents(int iteration, bool bRebuildNetwork, bool bOutputLog, int DemandLoadingStartTime, int DemandLoadingEndTime)
{
	// assign different zones to different processors
	int number_of_threads = omp_get_max_threads();

	if (bOutputLog)
	{
		cout << "# of Computer Processors = " << number_of_threads << endl;
	}

	number_of_threads = g_number_of_CPU_threads();

#pragma omp parallel for
	for (int ProcessID = 0; ProcessID < number_of_threads; ProcessID++)
	{
		// create network for shortest path calculation at this processor
		int	id = omp_get_thread_num();  // starting from 0

		//special notes: creating network with dynamic memory is a time-consumping task, so we create the network once for each processors


		if (bOutputLog)
		{
			cout << "---- agent-based routing and assignment at processor " << ProcessID + 1 << endl;
		}

		for (int CurZoneID = 1; CurZoneID <= g_ODZoneNumberSize; CurZoneID++)
		{

			if (g_ZoneMap.find(CurZoneID) == g_ZoneMap.end())  // no such zone being defined
				continue;

			if ((CurZoneID%number_of_threads) == ProcessID)  // if the remainder of a zone id (devided by the total number of processsors) equals to the processor id, then this zone id is 
			{

				// scan all possible departure times
				for (int departure_time = DemandLoadingStartTime; departure_time < DemandLoadingEndTime; departure_time += g_AggregationTimetInterval)
				{
					int vehicle_size = g_TDOVehicleArray[g_ZoneMap[CurZoneID].m_ZoneSequentialNo][departure_time / g_AggregationTimetInterval].VehicleArray.size();
					if (vehicle_size > 0)
					{

						int node_size = g_TimeDependentNetwork_MP[id].AgentBasedPathOptimization(CurZoneID, departure_time, departure_time + g_AggregationTimetInterval, iteration);
							cout << "Processor " << id << " is adjusting OD demand table for zone " << CurZoneID << endl;
					}
				}  // for each departure time
			}
		}  // for each zone
	} // for each computer processor

	if (bOutputLog)
	{
		cout << ":: complete optimization " << g_GetAppRunningTime() << endl;
		g_LogFile << ":: complete optimization " << g_GetAppRunningTime() << endl;

	}
}



float DTANetworkForSP::AgentBasedPathOptimization(int zone, int departure_time_begin, int departure_time_end, int iteration)
// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
{
	//	int PathLinkList[MAX_NODE_SIZE_IN_A_PATH];
	int TempPathLinkList[MAX_NODE_SIZE_IN_A_PATH];

	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int NodeSize;
	int TempNodeSize;

	int AssignmentInterval = g_FindAssignmentInterval(departure_time_begin);

	
	int total_node_size = 0;
	// loop through the TDOVehicleArray to assign or update vehicle paths...

	float stepsize = 0.01;

	if (iteration < 20)
		stepsize = 1.0 / (iteration + 1);
	else
	    stepsize = 0.05;


	int VehicleSize = g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][AssignmentInterval].VehicleArray.size();
	for (int vi = 0; vi < VehicleSize; vi++)
	{
		int VehicleID = g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][AssignmentInterval].VehicleArray[vi];
		DTAVehicle* pVeh = g_VehicleMap[VehicleID];

		pVeh->m_bConsiderToSwitch = false;
		pVeh->m_bSwitch = false;


		/// finding optimal path 
		bool bDebugFlag = false;

		float TotalCost;
		bool bGeneralizedCostFlag = false;

		// general settings

		int final_departuret_time_shift = 0;  // unit: min
		TempNodeSize = 0;
		float switch_threshold = 0;   // 2 min for day to day learning, - 100 min for  special case when using MSA, so 15% of agents will be selected for swtiching for sure. 


		if (pVeh->m_OriginZoneID == pVeh->m_DestinationZoneID)
		{  // do not simulate intra zone traffic
			continue;
		}


		if (pVeh->m_VehicleID == 1001)
			TRACE("");

		unsigned li;

		// iteration 0: Load initial road price vector
		if (iteration == 0)
		{
			for (li = 0; li < g_LinkVector.size(); li++)
			{
				DTALink* pLink = g_LinkVector[li];
				if (pLink->TollVector.size() > 0)
				{
					VehicleLinkPrice element;
					element.LinkNo = pLink->m_LinkNo;
					element.RoadPrice = 0;
					element.RoadUsageFlag = 0; 

					pVeh->m_PersonalizedRoadPriceVector.push_back(element);

				}
			}
		}



		// we perform subgradient for each vehicle

		float subgradient = 0;
		for (li = 0; li < pVeh->m_PersonalizedRoadPriceVector.size(); li++)
		{

			DTALink* pLink = g_LinkVector[pVeh->m_PersonalizedRoadPriceVector[li].LinkNo];


			subgradient = pVeh->m_PersonalizedRoadPriceVector[li].RoadUsageFlag - pLink->m_NetworkDesignBuildCapacity;

			pVeh->m_PersonalizedRoadPriceVector[li].RoadPrice += subgradient * stepsize;

			if (pVeh->m_PersonalizedRoadPriceVector[li].RoadPrice < 0)

				pVeh->m_PersonalizedRoadPriceVector[li].RoadPrice = 0;

		}


		// copy road price for links to be built
		int ti;
		for (li = 0; li < pVeh->m_PersonalizedRoadPriceVector.size(); li++)
		{
			int LinkNo = pVeh->m_PersonalizedRoadPriceVector[li].LinkNo;


				for (ti = 0; ti < m_NumberOfSPCalculationIntervals; ti += 1)
				{
					if (pVeh->m_PersonalizedRoadPriceVector[li].RoadPrice > 0.0001)
					{
						TD_LinkCostAry[LinkNo][ti] = pVeh->m_PersonalizedRoadPriceVector[li].RoadPrice;
					}
					else
					{
						TD_LinkCostAry[LinkNo][ti] = 0;

					
					}
				}
			


		}

		// we find shortet path for each agent

		float TargeTravelTimeInMin = 10;
		float OptimialTravelTimeInMin = 0;
		NodeSize = FindOptimalPath_TDLabelCorrecting_DQ(pVeh->m_OriginZoneID, pVeh->m_OriginNodeID, pVeh->m_DepartureTime,
			pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, PathLinkList, TotalCost, bGeneralizedCostFlag, TargeTravelTimeInMin, OptimialTravelTimeInMin, bDebugFlag);


			pVeh->SetMinCost(TotalCost);

			g_CurrentNumOfVehiclesSwitched += 1;

			pVeh->m_bConsiderToSwitch = true;

			if (pVeh->m_LinkAry != NULL && pVeh->m_NodeSize > 0)  // delete the old path
			{
				delete pVeh->m_LinkAry;
			}

			pVeh->m_NodeSize = NodeSize;


			if (pVeh->m_NodeSize >= 2)  // for feasible path
			{
				pVeh->m_bConsiderToSwitch = true;

				pVeh->m_LinkAry = new SVehicleLink[NodeSize];

				if (pVeh->m_LinkAry == NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				int NodeNumberSum = 0;
				float Distance = 0;


				// initialization 
				unsigned li;
				for (li = 0; li < pVeh->m_PersonalizedRoadPriceVector.size(); li++)
				{
						pVeh->m_PersonalizedRoadPriceVector[li].RoadUsageFlag = 0;

				}

				// scan the path

				for (int i = 0; i < NodeSize - 1; i++)
				{
					pVeh->m_LinkAry[i].LinkNo = PathLinkList[i];
					NodeNumberSum += PathLinkList[i];

					TRACE("\nlink %d ->%d", g_LinkVector[PathLinkList[i]]->m_FromNodeNumber, g_LinkVector[PathLinkList[i]]->m_ToNodeNumber);


					ASSERT(pVeh->m_LinkAry[i].LinkNo < g_LinkVector.size());
					Distance += g_LinkVector[pVeh->m_LinkAry[i].LinkNo]->m_Length;

					if (g_LinkVector[PathLinkList[i]]->m_TollSequenceNo >= 0)  // candidate link
					{
						int TollSequenceNo = g_LinkVector[PathLinkList[i]]->m_TollSequenceNo;
						pVeh->m_PersonalizedRoadPriceVector[TollSequenceNo].RoadUsageFlag = 1;
					}


				}


	

				float ExperiencedGeneralizedTravelTime = TotalCost;

				float m_gap = ExperiencedGeneralizedTravelTime - TotalCost;

				if (m_gap < 0)
					m_gap = 0.0;

				if (NodeNumberSum == pVeh->m_NodeNumberSum)  //same path
					m_gap = 0.0;


				pVeh->m_gap_update = true;
				pVeh->m_gap = m_gap;

				g_CurrentGapValue += m_gap; // Jason : accumulate g_CurrentGapValue only when iteration >= 1
				g_CurrentRelativeGapValue += m_gap / max(0.1, ExperiencedGeneralizedTravelTime);
				g_CurrentNumOfVehiclesForUEGapCalculation += 1;



				pVeh->m_Distance = Distance;
				pVeh->m_NodeNumberSum = NodeNumberSum;


				if (pVeh->m_PricingType == 4)  //assign travel time for transit users
				{
					pVeh->m_ArrivalTime = pVeh->m_DepartureTime + 0.1;  // at leat 0.1 min

					for (int i = 0; i < NodeSize; i++)
					{

						if (pVeh->m_VehicleID == 771)
							TRACE("");

						if (i == 0)  // initialize departure time
						{
							pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN = pVeh->m_DepartureTime;
						}

						if (i < NodeSize - 1)  // for all physical links
						{
							float transit_travel_time = 0;
							DTALink* pLink = g_LinkVector[pVeh->m_LinkAry[i].LinkNo];

							if (g_LinkTypeMap[pLink->m_link_type].IsTransit() == false)
							{
								if (g_LinkTypeMap[pLink->m_link_type].IsFreeway() == true)
								{
									transit_travel_time = 99999;
								}
								else
								{
									transit_travel_time = pLink->m_Length / 5 * 60;  // walking speed  = 5 mph			
								}

							}
							else
							{
								transit_travel_time = pLink->m_FreeFlowTravelTime;  // calculated from speed limit of bus

							}


							pVeh->m_LinkAry[i + 1].AbsArrivalTimeOnDSN =
								pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN + transit_travel_time;

							if (pVeh->m_ArrivalTime < pVeh->m_LinkAry[i + 1].AbsArrivalTimeOnDSN)  // update arrival time
							{
								pVeh->m_ArrivalTime = pVeh->m_LinkAry[i + 1].AbsArrivalTimeOnDSN;
								pVeh->m_TripTime = pVeh->m_ArrivalTime - pVeh->m_DepartureTime;
							}


						}



					}
				}
				//cout << pVeh->m_VehicleID <<  " Distance" << pVeh->m_Distance <<  endl;;

			} 

		} // for each vehicle on this OD pair

	return total_node_size / max(1, VehicleSize);

}

int DTANetworkForSP::FindOptimalPath_TDLabelCorrecting_DQ(int origin_zone, int origin, int departure_time, int destination_zone, int destination, int pricing_type, float VOT, int PathLinkList[MAX_NODE_SIZE_IN_A_PATH], float &TotalCost, bool bGeneralizedCostFlag, float TargetTravelTime, float &OptimialTravelTimeInMin, bool bDebugFlag)
// time -dependent label correcting algorithm with deque implementati
{

#ifndef _large_memory_usage
	return 0;
#endif
	int BeginOfTDSPCalculationInterval = max(0, (departure_time - m_StartTimeInMin)* g_TDSPTimetIntervalSizeForMin);
	int EndOfTDSPCalculationInterval = min(m_NumberOfTDSPCalculationIntervals, (departure_time + TargetTravelTime - m_StartTimeInMin)* g_TDSPTimetIntervalSizeForMin );
	int i;
	int debug_flag = 1;  // set 1 to debug the detail information
	if (debug_flag)
		TRACE("\nCompute shortest path from %d at time %d", origin, departure_time);

	bool bFeasiblePathFlag = false;

	if (m_OutboundSizeAry[origin] == 0)
		return false;

	for (i = 0; i <m_NodeSize; i++) // Initialization for all nodes
	{
		NodeStatusAry[i] = 0;

		for (int t = BeginOfTDSPCalculationInterval; t <EndOfTDSPCalculationInterval; t++)
		{
			TD_LabelCostAry[i][t] = MAX_SPLABEL;
			TD_NodePredAry[i][t] = -1;  // pointer to previous NODE INDEX from the current label at current node and time
			TD_TimePredAry[i][t] = -1;  // pointer to previous TIME INDEX from the current label at current node and time
		}

	}


	int l;

	for (l = 0; l <m_LinkSize; l++) // Initialization for all nodes
	{
		for (int t = BeginOfTDSPCalculationInterval; t <EndOfTDSPCalculationInterval; t++)
		{

			int aggregated_time_interval = FindTDAggregatedTimeInterval(m_StartTimeInMin + t / g_TDSPTimetIntervalSizeForMin);
			TD_LinkTimeIntervalAry[l][t] = max(1, (int)(m_LinkTDTimeAry[l][aggregated_time_interval] * g_TDSPTimetIntervalSizeForMin+0.5));

			
		}

	}

	// Initialization for origin node at the preferred departure time, at departure time, cost = 0, otherwise, the delay at origin node
	TD_LabelCostAry[origin][BeginOfTDSPCalculationInterval] = 0;

	SEList_clear();
	SEList_push_front(origin);


	while (!SEList_empty())
	{
		int FromID = SEList_front();
		SEList_pop_front();  // remove current node FromID from the SE list


		NodeStatusAry[FromID] = 2;        //scaned

		//scan all outbound nodes of the current node
		for (i = 0; i<m_OutboundSizeAry[FromID]; i++)  // for each arc (i,j) belong A(i)
		{
			int LinkNo = m_OutboundLinkAry[FromID][i];
			int ToID = m_OutboundNodeAry[FromID][i];

			if (ToID == origin)  // remove possible loop back to the origin
				continue;

			int ToNodeNumber = g_NodeVector[ToID].m_NodeNumber;

			if (m_LinkConnectorFlag[LinkNo] == 1)  // only check the following speical condition when a link is a connector
			{
				int OriginTAZ = m_OutboundConnectorOriginZoneIDAry[FromID][i];
				int DestinationTAZ = m_OutboundConnectorDestinationZoneIDAry[FromID][i];

				if (OriginTAZ >= 1 /* TAZ >=1*/ && DestinationTAZ <= 0 && OriginTAZ != origin_zone)
					continue;  // special feature 1: skip connectors with origin TAZ only and do not belong to this origin zone

				if (DestinationTAZ >= 1 /* TAZ >=1*/ && OriginTAZ <= 0 && DestinationTAZ != destination_zone)
					continue;  // special feature 2: skip connectors with destination TAZ that do not belong to this destination zone

				if (OriginTAZ >= 1 /* TAZ >=1*/ && OriginTAZ != origin_zone  && DestinationTAZ >= 1 /* TAZ >=1*/ && DestinationTAZ != destination_zone)
					continue;  // special feature 3: skip connectors (with both TAZ at two ends) that do not belong to the origin/destination zones

				if (ToID == origin) // special feature 2: no detour at origin
					continue;
			}

			if (debug_flag)
				TRACE("\nScan from node %d to node %d", g_NodeVector[FromID].m_NodeNumber, g_NodeVector[ToID].m_NodeNumber);

			// for each time step, starting from the departure time
			for (int t = BeginOfTDSPCalculationInterval; t <EndOfTDSPCalculationInterval; t++)
			{
				if (TD_LabelCostAry[FromID][t]<MAX_SPLABEL - 1)  // for feasible time-space point only
				{
					int time_stopped = 0;

					int NewToNodeArrivalTimeInterval = t + time_stopped + TD_LinkTimeIntervalAry[LinkNo][t];  // time-dependent travel times for different train type

					if (NewToNodeArrivalTimeInterval == t)
						NewToNodeArrivalTimeInterval = t + 1;

					NewToNodeArrivalTimeInterval = GetFeasibleTDSPTimeInterval(NewToNodeArrivalTimeInterval);


					float NewCost = TD_LabelCostAry[FromID][t] + TD_LinkCostAry[LinkNo][NewToNodeArrivalTimeInterval];
					// costs come from time-dependent resource price or road toll

					if (NewCost < TD_LabelCostAry[ToID][NewToNodeArrivalTimeInterval]) // we only compare cost at the downstream node ToID at the new arrival time t
					{

						if (ToID == destination)
							bFeasiblePathFlag = true;


						if (debug_flag)
							TRACE("\n         UPDATE to %f, link cost %f at time %d", NewCost, TD_LinkCostAry[LinkNo][t], NewToNodeArrivalTimeInterval);

						// update cost label and node/time predecessor

						TD_LabelCostAry[ToID][NewToNodeArrivalTimeInterval] = NewCost;
						TD_NodePredAry[ToID][NewToNodeArrivalTimeInterval] = FromID;  // pointer to previous NODE INDEX from the current label at current node and time
						TD_LinkPredAry[ToID][NewToNodeArrivalTimeInterval] = LinkNo;  // pointer to previous LinkNo INDEX from the current label at current node and time
						TD_TimePredAry[ToID][NewToNodeArrivalTimeInterval] = t;  // pointer to previous TIME INDEX from the current label at current node and time

						// Dequeue implementation
						if (NodeStatusAry[ToID] == 2) // in the SEList_TD before
						{
							SEList_push_front(ToID);
							NodeStatusAry[ToID] = 1;
						}
						if (NodeStatusAry[ToID] == 0)  // not be reached
						{
							SEList_push_back(ToID);
							NodeStatusAry[ToID] = 1;
						}


					}
				}
				//another condition: in the SELite now: there is no need to put this node to the SEList, since it is already there.
			}

		}      // end of for each link

	}	// end of while

	if (bFeasiblePathFlag == false)
		return 0;

	return FindOptimalSolution(origin, departure_time, destination, PathLinkList, TargetTravelTime, OptimialTravelTimeInMin);
}
int DTANetworkForSP::FindOptimalSolution(int origin, int departure_time, int destination, int PathLinkList[MAX_NODE_SIZE_IN_A_PATH], float TargetTravelTimeInMin, float &OptimialTravelTimeInMin)  // the last pointer is used to get the node array
{

	// step 1: scan all the time label at destination node, consider time cost
	// step 2: backtrace to the origin (based on node and time predecessors)
	// step 3: reverse the backward path
	// return final optimal solution

	// step 1: scan all the time label at destination node, consider time cost
	int tmp_AryTN[MAX_NODE_SIZE_IN_A_PATH]; //backward temporal solution
	int tmp_AryTLink[MAX_NODE_SIZE_IN_A_PATH]; //backward temporal link solution

	float min_cost = MAX_SPLABEL;
	int min_cost_time_index = -1;

	int BeginOfTDSPCalculationInterval = max(0, (departure_time - m_StartTimeInMin)* g_TDSPTimetIntervalSizeForMin);
	int EndOfTDSPCalculationInterval = min(m_NumberOfTDSPCalculationIntervals, (departure_time + TargetTravelTimeInMin - m_StartTimeInMin)* g_TDSPTimetIntervalSizeForMin);

	for (int t = BeginOfTDSPCalculationInterval; t <EndOfTDSPCalculationInterval; t++)
	{
		if (TD_LabelCostAry[destination][t] < min_cost)
		{
			min_cost = TD_LabelCostAry[destination][t];
			min_cost_time_index = t;
		}

	}

	if (min_cost_time_index < 0)
		return 0;
		// if min_cost_time_index ==-1, then no feasible path if founded

	// step 2: backtrace to the origin (based on node and time predecessors)

	int	NodeSize = 0;

	//record the first node backward, destination node
	tmp_AryTN[NodeSize] = destination;

	NodeSize++;

	int PredTime = TD_TimePredAry[destination][min_cost_time_index];
	int PredNode = TD_NodePredAry[destination][min_cost_time_index];
	int PredLink = TD_LinkPredAry[destination][min_cost_time_index];
	int LinkSize = 0;
	tmp_AryTLink[LinkSize++] = PredLink;

	while (PredNode != origin && PredNode != -1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
	{
		ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH - 1);

		tmp_AryTN[NodeSize] = PredNode;

		NodeSize++;

		//record current values of node and time predecessors, and update PredNode and PredTime
		int PredTime_cur = PredTime;
		int PredNode_cur = PredNode;

		//TRACE("\n PredTime_cur = %d, PredNode_cur = %d ", PredTime_cur, PredNode_cur);

		PredNode = TD_NodePredAry[PredNode_cur][PredTime_cur];
		PredLink = TD_LinkPredAry[PredNode_cur][PredTime_cur];
		PredTime = TD_TimePredAry[PredNode_cur][PredTime_cur];
		tmp_AryTLink[LinkSize++] = PredLink;

	}

	OptimialTravelTimeInMin = (min_cost_time_index - PredTime) / g_TDSPTimetIntervalSizeForMin;
	tmp_AryTN[NodeSize] = origin;

	NodeSize++;

	// step 3: reverse the backward solution

	int i;
	int j = 0;
	for (i = LinkSize - 1; i >= 0; i--)
	{
		PathLinkList[j++] = tmp_AryTLink[i];
	}

	return NodeSize;

}
