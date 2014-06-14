//  Portions Copyright 2010 Xuesong Zhou, Jason Lu

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
extern CTime g_AppLastIterationStartTime;

int g_FindAssignmentInterval(int departure_time_begin)
{
	int AssignmentInterval = int(departure_time_begin / g_AggregationTimetInterval);  // starting assignment interval

	if (AssignmentInterval < g_DemandLoadingStartTimeInMin / g_AggregationTimetInterval)
	{
		// reset assignment time interval 
		AssignmentInterval = g_DemandLoadingStartTimeInMin / g_AggregationTimetInterval;
	}
	if (AssignmentInterval >= g_AggregationTimetIntervalSize)
		AssignmentInterval = g_AggregationTimetIntervalSize - 1;


	return AssignmentInterval;

}
std::map<CString, int> g_path_index_map;





bool g_GetSequentialUEAdjustmentTimePeriod(int iteration, float DepartureTime)
{
	// 20 iterations for each hour adjustment period

	int TimePeriodForSequentialAdjustment = 15;  //min
	int iteration_size = 20;

	int SequentialAdjustmentTimePeriodStart = g_DemandLoadingStartTimeInMin + iteration*1.0 / iteration_size*TimePeriodForSequentialAdjustment;
	if (SequentialAdjustmentTimePeriodStart >= g_DemandLoadingEndTimeInMin) // restart from the beginning 
		SequentialAdjustmentTimePeriodStart = g_DemandLoadingStartTimeInMin;

	int SequentialAdjustmentTimePeriodEnd = SequentialAdjustmentTimePeriodStart + TimePeriodForSequentialAdjustment;

	if (DepartureTime >= SequentialAdjustmentTimePeriodStart && DepartureTime <= SequentialAdjustmentTimePeriodEnd)  //within time interval
		return true;
	else
		return false;

}

DTANetworkForSP g_TimeDependentNetwork_MP[_MAX_NUMBER_OF_PROCESSORS]; //  network instance for single processor in multi-thread environment: no more than 8 threads/cores

void g_BuildPathsForAgents(int iteration, bool bRebuildNetwork, bool bOutputLog, int DemandLoadingStartTime, int DemandLoadingEndTime)
{
	bool bZoneBasedPathFlag = false;
	// assign different zones to different processors
	int number_of_threads = omp_get_max_threads();

	//	if(g_ODEstimationFlag==1)  // single thread mode for ODME 
	//		number_of_threads = 1;

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
		if (bRebuildNetwork || g_TimeDependentNetwork_MP[id].m_NodeSize == 0)
		{
			g_TimeDependentNetwork_MP[id].BuildPhysicalNetwork(iteration, -1, g_TrafficFlowModelFlag);  // build network for this zone, because different zones have different connectors...
		}
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

						if (bZoneBasedPathFlag == true)  // zone based assignment
						{
							bool debug_flag = false;
							for (int pricing_type = 1; pricing_type < MAX_PRICING_TYPE_SIZE; pricing_type++)  // from SOV, HOV, truck
							{

								g_TimeDependentNetwork_MP[id].TDLabelCorrecting_DoubleQueue_PerPricingType(g_NodeVector.size(), departure_time, pricing_type, g_PricingTypeMap[pricing_type].default_VOT, false, debug_flag);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo
							}

							if (g_ODEstimationFlag && iteration >= g_ODEstimation_StartingIteration)  // perform path flow adjustment after at least 10 normal OD estimation
								g_TimeDependentNetwork_MP[id].ZoneBasedPathAssignment_ODEstimation(CurZoneID, departure_time, departure_time + g_AggregationTimetInterval, iteration);
							else
								g_TimeDependentNetwork_MP[id].ZoneBasedPathAssignment(CurZoneID, departure_time, departure_time + g_AggregationTimetInterval, iteration, debug_flag);

						}    // agent based assignment
						else
						{

							float node_size = 1;
							if (g_ODEstimationFlag && iteration >= g_ODEstimation_StartingIteration)  // perform path flow adjustment after at least 10 normal OD estimation
								g_TimeDependentNetwork_MP[id].ZoneBasedPathAssignment_ODEstimation(CurZoneID, departure_time, departure_time + g_AggregationTimetInterval, iteration);
							else
								node_size = g_TimeDependentNetwork_MP[id].AgentBasedPathFindingAssignment(CurZoneID, departure_time, departure_time + g_AggregationTimetInterval, iteration);

						}

						if (g_ODZoneNumberSize > 1000 && departure_time == g_DemandLoadingStartTimeInMin)  // only for large networks and zones with data
						{

							if (bOutputLog)
							{
								if (g_ODEstimationFlag && iteration >= g_ODEstimation_StartingIteration)  // perform path flow adjustment after at least 10 normal OD estimation
									cout << "Processor " << id << " is adjusting OD demand table for zone " << CurZoneID << endl;
								else
									cout << "Processor " << id << " is calculating the shortest paths for zone " << CurZoneID << " with " << vehicle_size << " agents." << endl;

							}
						}


					}
				}  // for each departure time
			}
		}  // for each zone
	} // for each computer processor

	if (bOutputLog)
	{
		cout << ":: complete assignment " << g_GetAppRunningTime() << endl;
		g_LogFile << ":: complete assignment " << g_GetAppRunningTime() << endl;

	}
}
void g_AgentBasedAssisnment()  // this is an adaptation of OD trip based assignment, we now generate and assign path for each individual vehicle (as an agent with personalized value of time, value of reliability)
{
	// reset random number seeds
	int node_size = g_NodeVector.size() + 1 + g_ODZoneNumberSize;
	int link_size = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

	cout << ":: start assignment " << g_GetAppRunningTime() << endl;
	g_LogFile << ":: start assignment " << g_GetAppRunningTime() << endl;


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



	// ----------* start of outer loop *----------
	for (iteration = 0; NotConverged && iteration <= g_NumberOfIterations; iteration++)  // we exit from the loop under two conditions (1) converged, (2) reach maximum number of iterations
	{
		cout << "------- Iteration = " << iteration + 1 << "--------" << endl;

		if (g_ODEstimationFlag && iteration >= g_ODEstimation_StartingIteration)
			g_HistDemand.ResetUpdatedValue(); // reset update hist table


		g_CurrentGapValue = 0.0;
		g_CurrentRelativeGapValue = 0.0;
		g_CurrentNumOfVehiclesForUEGapCalculation = 0;
		g_CurrentNumOfVehiclesSwitched = 0;
		g_NewPathWithSwitchedVehicles = 0;


		////

		if (g_VehicleLoadingMode == vehicle_binary_file_mode && iteration == 0)
		{
			// do nothing	// we do not need to generate initial paths for vehicles for the first iteration of vehicle loading mode
		}
		else
		{
			g_BuildPathsForAgents(iteration, true, true, g_DemandLoadingStartTimeInMin, g_DemandLoadingEndTimeInMin);
		}

		////


		if (g_ODEstimationFlag && iteration >= g_ODEstimation_StartingIteration)  // re-generate vehicles based on global path set
		{
			g_GenerateVehicleData_ODEstimation();
		}
		cout << "---- Network Loading for Iteration " << iteration << "----" << endl;

		NetworkLoadingOutput SimuOutput;

		if (g_EmissionDataOutputFlag >= 1 && iteration == g_NumberOfIterations)  // last iteration
		{
			g_DTASimulationInterval = 0.1;
		}


		SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag, 0, iteration);
		g_GenerateSimulationSummary(iteration, NotConverged, TotalNumOfVehiclesGenerated, &SimuOutput);

	}  // for each assignment iteration

	cout << "Writing Vehicle Trajectory and MOE File... " << endl;

	if (iteration == g_NumberOfIterations)
	{
		iteration = g_NumberOfIterations - 1;  //roll back to the last iteration if the ending condition is triggered by "iteration < g_NumberOfIterations"
	}

	g_OutputMOEData(iteration);

}


float DTANetworkForSP::AgentBasedPathFindingAssignment(int zone, int departure_time_begin, int departure_time_end, int iteration)
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

		// 2 min for day to day learning, 
		// -100 min for  special case when using MSA
		// +100 will disallow any departure switch, because the benchmark (experienced travel time - 100) is too low. 


		float switching_rate = 0.05;   // default switching rate 
		if (g_UEAssignmentMethod == assignment_MSA)  // MSA
		{
			switching_rate = 1.0f / (iteration + 1);
		}
		if (g_UEAssignmentMethod == assignment_fixed_percentage) // day to day learning
		{
			switching_rate = float(g_LearningPercVector[iteration + 1]) / 100.0f; // 1: day-to-day learning
		}
		if (g_UEAssignmentMethod == assignment_gap_function_MSA_step_size) // gap function based method,
		{
			switching_rate = 1.0f / (iteration + 1);

			if (iteration > 20)
			{
				if (g_GetSequentialUEAdjustmentTimePeriod(iteration, pVeh->m_DepartureTime))
					switching_rate = 0.05;
				else
					switching_rate = 0.001;
			}
		}
		if (g_UEAssignmentMethod == assignment_day_to_day_learning_threshold_route_choice ||
			g_UEAssignmentMethod == assignment_day_to_day_learning_threshold_route_and_departure_time_choice)
		{
			switching_rate = float(g_LearningPercVector[iteration + 1]) / 100.0f; // 1: day-to-day learning
			// all agents have also to subject to bounded rationality rules


		}

		if (pVeh->m_OriginZoneID == pVeh->m_DestinationZoneID)
		{  // do not simulate intra zone traffic
			continue;
		}


		double RandomNumber = pVeh->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			


		bool bSwitchFlag = false;

		float ExperiencedTravelTime = pVeh->m_TripTime;
		float ExperiencedGeneralizedTravelTime = pVeh->m_TripTime + pVeh->m_TollDollarCost / max(1, pVeh->m_VOT);  // unit: min

		bool bAllowDepartureTimeSwitchFlag = false;


		if (pVeh->m_VehicleID == 1001)
			TRACE("");

		if (iteration == 0)  //we always switch at the first iteration
		{
			if (pVeh->m_NodeSize == 0)  // without pre-loaded path
			{
				bSwitchFlag = true;

				pVeh->m_PreferredDepartureTime = pVeh->m_DepartureTime;  // set departure time to m_PreferredDepartureTime

				bDebugFlag = false;
				// get the first feasible solution
				if (pVeh->m_OriginZoneID == 134 && pVeh->m_DestinationZoneID == 18177)
				{  // do not simulate intra zone traffic
					TRACE("vehicle id  = %d", VehicleID);
					bDebugFlag = true;
				}

				NodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, pVeh->m_OriginNodeID, pVeh->m_DepartureTime, pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, PathLinkList, TotalCost, bGeneralizedCostFlag, bDebugFlag);
				total_node_size += NodeSize;


				pVeh->m_bSwitch = true;
			}
			else
			{

				bSwitchFlag = false;

			}


		}
		else
		{ // iteration >=1
			if (g_UEAssignmentMethod == assignment_day_to_day_learning_threshold_route_and_departure_time_choice)
			{
				float gain_factor = 0.1f;

				if (iteration <= 1)
				{
					pVeh->m_EstimatedTravelTime = ExperiencedGeneralizedTravelTime;
				}
				else
				{
					pVeh->m_EstimatedTravelTime = pVeh->m_EstimatedTravelTime*(1 - gain_factor) + ExperiencedGeneralizedTravelTime*(gain_factor);
				}

				bSwitchFlag = false;
				// agent based implementation

				DTADecisionAlternative min_cost_alternative;
				int departuret_time_shift = 0;

				// swtich: find shortest path
				TempNodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, pVeh->m_OriginNodeID, pVeh->m_DepartureTime, pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, TempPathLinkList, TotalCost, bGeneralizedCostFlag, bDebugFlag);

				min_cost_alternative.UpdateForLowerAlternativeCost(TotalCost, 0, TempNodeSize, TempPathLinkList);

				if (g_UEAssignmentMethod == assignment_day_to_day_learning_threshold_route_and_departure_time_choice)  //departure time choice
				{
					departuret_time_shift = -5; //leave early
					// leaving option: +5*
					TempNodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, pVeh->m_OriginNodeID, pVeh->m_DepartureTime + departuret_time_shift, pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, TempPathLinkList, TotalCost, bGeneralizedCostFlag, bDebugFlag);
					// add additional cost due to leaving early
					TotalCost += fabs(departuret_time_shift*g_DepartureTimeChoiceEarlyDelayPenalty);
					min_cost_alternative.UpdateForLowerAlternativeCost(TotalCost, departuret_time_shift, TempNodeSize, TempPathLinkList);

					departuret_time_shift = 5; //leave later
					// leaving option: +5*
					TempNodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, pVeh->m_OriginNodeID, pVeh->m_DepartureTime + departuret_time_shift, pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, TempPathLinkList, TotalCost, bGeneralizedCostFlag, bDebugFlag);

					// add additional cost due to leaving late
					TotalCost += fabs(departuret_time_shift*g_DepartureTimeChoiceLateDelayPenalty);
					min_cost_alternative.UpdateForLowerAlternativeCost(TotalCost, departuret_time_shift, TempNodeSize, TempPathLinkList);

				}
				//switching to the min cost alternative

				if (min_cost_alternative.total_cost < pVeh->m_EstimatedTravelTime - g_TravelTimeDifferenceForSwitching
					|| min_cost_alternative.total_cost*(1 + g_RelativeTravelTimePercentageDifferenceForSwitching / 100) < pVeh->m_EstimatedTravelTime)
				{
					bSwitchFlag = true;
					final_departuret_time_shift = min_cost_alternative.final_departuret_time_shift;

					NodeSize = min_cost_alternative.node_size; // copy the shortest path array and # of nodes along the path

					for (int n = 0; n < NodeSize; n++)
					{
						PathLinkList[n] = min_cost_alternative.path_link_list[n];
					}
				}
			}
			else if (RandomNumber < switching_rate)  // g_Day2DayAgentLearningMethod==0: no learning, just switching 
			{

				if (g_UEAssignmentMethod == assignment_system_optimal )
				{

					if (pVeh->m_ArrivalTime > g_SystemOptimalStartingTimeinMin && (pVeh->m_InformationClass == info_personalized_info || pVeh->m_InformationClass == info_eco_so))  // personalized eco so users
					{
						// step 1: copy existing link numbers
						int  current_link_count = 0;
						float starting_time_in_min = pVeh->m_DepartureTime;
						int starting_node_id = pVeh->m_OriginNodeID;


						if (pVeh->m_DepartureTime < g_SystemOptimalStartingTimeinMin)
						{
						
						for (int i = 0; i < pVeh->m_NodeSize -1 ; i++)
						{
							CurrentPathLinkList[i] = pVeh->m_LinkAry[i].LinkNo;
							current_link_count++;


							if (pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN >= g_SystemOptimalStartingTimeinMin)  // wait up at the first time when receiving information 
							{
								starting_time_in_min = pVeh->m_LinkAry[i].AbsArrivalTimeOnDSN;
								starting_node_id = g_LinkVector[pVeh->m_LinkAry[i].LinkNo]->m_ToNodeID;  // start from the downstream node of current driving link;
								break;
							}
						}

						}

						if (current_link_count + 2 >= pVeh->m_NodeSize - 1 )  // arrive the destinatio already in the next step, no need to switch 
						{
							bSwitchFlag = false;

						}
						else
						{
							// setp 2: find shortest path from the current location 

							TRACE("\n current node: %d", g_NodeVector[starting_node_id].m_NodeNumber );


							if (pVeh->m_InformationClass == info_personalized_info)
							{
								bGeneralizedCostFlag = false;
								NodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, starting_node_id, starting_time_in_min, pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, PathLinkList, TotalCost, bGeneralizedCostFlag, bDebugFlag);
							}

							if (pVeh->m_InformationClass == info_eco_so)
							{
								bGeneralizedCostFlag = true;
								NodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, starting_node_id, starting_time_in_min, pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, PathLinkList, TotalCost, bGeneralizedCostFlag, bDebugFlag);
							}


							if (pVeh->m_DepartureTime < g_SystemOptimalStartingTimeinMin)  // leave earlier than the event time 
							{

								for (int i = 0; i < NodeSize - 1; i++)
								{
									CurrentPathLinkList[current_link_count] = PathLinkList[i];
									current_link_count += 1;  // add link no from the new path starting from the current position 

									TRACE("\n next link : %d -> %d", g_LinkVector[PathLinkList[i]]->m_FromNodeNumber, g_LinkVector[PathLinkList[i]]->m_ToNodeNumber);

								}

								NodeSize = current_link_count + 1;

								for (int i = 0; i < NodeSize - 1; i++)  // copy back from CurrentPathLinkList to PathLinkList;
								{
									PathLinkList[i] = CurrentPathLinkList[i];
								}
								TotalCost = TotalCost + (starting_time_in_min - pVeh->m_DepartureTime);

							}

							bSwitchFlag = true;

						}


					}
					else
					{
						// not need to switch as you have arrived at the destination before the event time 
						bSwitchFlag = false; 
					}


				}
				else
				{
					NodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, pVeh->m_OriginNodeID, pVeh->m_DepartureTime, pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, PathLinkList, TotalCost, bGeneralizedCostFlag, bDebugFlag);
					bSwitchFlag = true;

				}


	
			}
		}

		if (pVeh->m_bForcedSwitchAtFirstIteration == true)
		{
			// m_bForcedSwitchAtFirstIteration is true when the vehicle has to pass through a totally closed work zone link
			bSwitchFlag = true;
			pVeh->m_bForcedSwitchAtFirstIteration = false; // reset the flag to false in any case, as it is effective only for the first iteration

		}

		pVeh->m_gap_update = false;

		if (bSwitchFlag)  // for all vehicles that need to switch
		{

			pVeh->m_DepartureTime = pVeh->m_PreferredDepartureTime + final_departuret_time_shift;

			pVeh->SetMinCost(TotalCost);

			/// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle

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

				for (int i = 0; i < NodeSize - 1; i++)
				{
					pVeh->m_LinkAry[i].LinkNo = PathLinkList[i];
					NodeNumberSum += PathLinkList[i];

					/*if(g_LinkVector[pVeh->m_LinkAry [i].LinkNo]==NULL)
					{
					cout << "Error: g_LinkVector[pVeh->m_LinkAry [i].LinkNo]==NULL", pVeh->m_LinkAry [i].LinkNo;
					getchar();
					exit(0);
					}
					*/
					ASSERT(pVeh->m_LinkAry[i].LinkNo < g_LinkVector.size());
					Distance += g_LinkVector[pVeh->m_LinkAry[i].LinkNo]->m_Length;

				}
				if (NodeNumberSum != pVeh->m_NodeNumberSum)
				{  //switch to different  paths 
					pVeh->m_bSwitch = true;
				}


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

		}  // switch
		else
		{  // not switch but we calculate gap function
			if (g_CalculateUEGapForAllAgents ==1 )
			{
				NodeSize = FindBestPathWithVOT(pVeh->m_OriginZoneID, pVeh->m_OriginNodeID, pVeh->m_DepartureTime, pVeh->m_DestinationZoneID, pVeh->m_DestinationNodeID, pVeh->m_PricingType, pVeh->m_VOT, PathLinkList, TotalCost, bGeneralizedCostFlag, bDebugFlag);
				total_node_size += NodeSize;

				int NodeNumberSum = 0;

				for (int i = 0; i < NodeSize - 1; i++)
				{
					NodeNumberSum += PathLinkList[i];

				}

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




			}


		}

	} // for each vehicle on this OD pair

	return total_node_size / max(1, VehicleSize);

}





void DTANetworkForSP::ZoneBasedPathAssignment(int zone, int departure_time_begin, int departure_time_end, int iteration, bool debug_flag = false)
// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
{

	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int NodeSize;
	int PredNode;
	int AssignmentInterval = g_FindAssignmentInterval(departure_time_begin);  // starting assignment interval


	int vehicle_id_trace = -1;

	//PathArrayForEachODT *PathArray;
	//PathArray = new PathArrayForEachODT[g_ODZoneIDSize]; // remember to release memory

	// loop through the TDOVehicleArray to assign or update vehicle paths...
	for (int vi = 0; vi < g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][AssignmentInterval].VehicleArray.size(); vi++)
	{
		int VehicleID = g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][AssignmentInterval].VehicleArray[vi];
		DTAVehicle* pVeh = g_VehicleMap[VehicleID];
		ASSERT(pVeh != NULL);


		int OriginCentriod = m_PhysicalNodeSize;
		int DestinationCentriod = m_PhysicalNodeSize + pVeh->m_DestinationZoneID;

		float TotalCost = LabelCostVectorPerType[pVeh->m_PricingType][DestinationCentriod];
		if (TotalCost > MAX_SPLABEL - 10)
		{
			cout << "Warning: vehicle " << pVeh->m_VehicleID << " from zone " << pVeh->m_OriginZoneID << " to zone " << pVeh->m_DestinationZoneID << " does not have a physical path. Please check warning.log for details. " << endl;
			g_WarningFile << "Warning: vehicle " << pVeh->m_VehicleID << " from zone " << pVeh->m_OriginZoneID << " to zone " << pVeh->m_DestinationZoneID << " does not have a physical path. " << endl;
			pVeh->m_NodeSize = 0;
			continue;

		}

		bool bSwitchFlag = false;
		pVeh->m_bConsiderToSwitch = false;

		pVeh->m_bSwitch = false;


		if (iteration > 0) // update path assignments -> determine whether or not the vehicle will switch
		{


			pVeh->SetMinCost(TotalCost);


			float switching_rate = 1.0f / (iteration + 1);   // default switching rate from MSA
			if (g_UEAssignmentMethod == assignment_MSA)  // MSA
			{
				switching_rate = 1.0f / (iteration + 1);
			}
			if (g_UEAssignmentMethod == assignment_fixed_percentage) // day to day learning
			{
				switching_rate = float(g_LearningPercVector[iteration + 1]) / 100.0f; // 1: day-to-day learning
			}
			if (g_UEAssignmentMethod == assignment_gap_function_MSA_step_size) // gap function based method,
			{
				switching_rate = 1.0f / (iteration + 1);

				if (iteration > 20)
				{
					if (g_GetSequentialUEAdjustmentTimePeriod(iteration, pVeh->m_DepartureTime))
						switching_rate = 0.05;
					else
						switching_rate = 0.001;
				}
			}

			if (g_UEAssignmentMethod == assignment_day_to_day_learning_threshold_route_choice ||
				g_UEAssignmentMethod == assignment_day_to_day_learning_threshold_route_and_departure_time_choice)
			{
				switching_rate = float(g_LearningPercVector[iteration + 1]) / 100.0f; // 1: day-to-day learning
				// all agents have also to subject to bounded rationality rules


			}

			double RandomNumber = pVeh->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			

			if ((pVeh->m_bComplete == false && pVeh->m_NodeSize >= 2)) //for incomplete vehicles with feasible paths, need to switch at the next iteration
			{
				bSwitchFlag = true;
			}
			else
			{
				if (RandomNumber < switching_rate)
				{
					bSwitchFlag = true;
				}
			}
		}
		else	// iteration = 0;  at iteration 0, every vehicle needs a path for simulation, so every vehicle switches
		{

			if (pVeh->m_NodeSize == 0)  // no external path at the first iteration 
				bSwitchFlag = true;
			else
			{


			}
		}

		if (bSwitchFlag)
		{
			// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle
			NodeSize = 0;
			PredNode = NodePredVectorPerType[pVeh->m_PricingType][DestinationCentriod];
			while (PredNode != OriginCentriod && PredNode != -1) // scan backward in the predessor array of the shortest path calculation results
			{
				if (NodeSize >= MAX_NODE_SIZE_IN_A_PATH - 1)
				{

					g_LogFile << "error in path finding: too many nodes: OD pair: " << zone << " -> " << pVeh->m_DestinationZoneID << endl;

					for (int i = 0; i < NodeSize; i++)
					{
						cout << "error in path finding: too many nodes: OD pair: " << zone << " -> " << pVeh->m_DestinationZoneID << endl;
						cout << "Node sequence " << i << ":node " << g_NodeVector[temp_reversed_PathLinkList[i]].m_NodeNumber << ",cost: " << LabelCostVectorPerType[pVeh->m_PricingType][temp_reversed_PathLinkList[i]] << endl;


						g_LogFile << "Node sequence " << i << ":node " << g_NodeVector[temp_reversed_PathLinkList[i]].m_NodeNumber << ",cost: " << LabelCostVectorPerType[pVeh->m_PricingType][temp_reversed_PathLinkList[i]] << endl;

					}

					g_ProgramStop();
				}

				temp_reversed_PathLinkList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				PredNode = NodePredVectorPerType[pVeh->m_PricingType][PredNode];
			}

			// the first node in the shortest path is the super zone center, should not be counted

			int j = 0;

			int NodeNumberSum = 0;
			for (int i = NodeSize - 1; i >= 0; i--)
			{
				NodeNumberSum += temp_reversed_PathLinkList[i];
				PathNodeList[j++] = temp_reversed_PathLinkList[i];

				ASSERT(PathNodeList[j] < m_PhysicalNodeSize);
			}

			float m_gap;
			float ExperiencedTravelTime = pVeh->m_TripTime;


			m_gap = ExperiencedTravelTime - TotalCost;
			if (m_gap < 0) m_gap = 0.0;

			if (NodeNumberSum == pVeh->m_NodeNumberSum)  //same path
				m_gap = 0.0;


			g_CurrentGapValue += m_gap; // Jason : accumulate g_CurrentGapValue only when iteration >= 1
			g_CurrentRelativeGapValue += m_gap / max(0.1, ExperiencedTravelTime);
			g_CurrentNumOfVehiclesForUEGapCalculation += 1;



			// Jason : accumulate number of vehicles switching paths
			g_CurrentNumOfVehiclesSwitched += 1;

			pVeh->m_bConsiderToSwitch = true;
			pVeh->m_NodeSize = NodeSize;

			if (pVeh->m_LinkAry != NULL)
			{
				delete pVeh->m_LinkAry;
			}

			if (pVeh->m_NodeSize >= 2)
			{
				pVeh->m_bConsiderToSwitch = true;

				pVeh->m_LinkAry = new SVehicleLink[NodeSize];

				if (pVeh->m_LinkAry == NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				pVeh->m_NodeNumberSum = 0;
				pVeh->m_Distance = 0;

				for (int i = 0; i < NodeSize - 1; i++) // NodeSize-1 is the number of links along the paths
				{
					pVeh->m_LinkAry[i].LinkNo = GetLinkNoByNodeIndex(PathNodeList[i], PathNodeList[i + 1]);
					pVeh->m_NodeNumberSum += PathNodeList[i];
					pVeh->m_Distance += g_LinkVector[pVeh->m_LinkAry[i].LinkNo]->m_Length;
				}
				//cout << pVeh->m_VehicleID <<  " Distance" << pVeh->m_Distance <<  endl;;

				//// check whether or not this is a new path
				//int PathIndex = 0;
				//for(int p=1; p<=PathArray[pVeh->m_DestinationZoneID].NumOfPaths; p++)
				//{
				//	if(pVeh->m_NodeNumberSum == PathArray[pVeh->m_DestinationZoneID].PathNodeSums[p])
				//	{
				//		PathIndex = p;
				//		break;
				//	}
				//}
				//if(PathIndex == 0) // a new path found
				//	g_NewPathWithSwitchedVehicles++;	

				pVeh->m_bSwitch = true;

			}
			else
			{
				pVeh->m_bLoaded = false;
				pVeh->m_bComplete = false;

				if (iteration == 0)
				{
					g_WarningFile << "Warning: vehicle " << pVeh->m_VehicleID << " from zone " << pVeh->m_OriginZoneID << " to zone " << pVeh->m_DestinationZoneID << " does not have a physical path. Path Cost:" << TotalCost << endl;
				}
			}
		} // if(bSwitchFlag)
	}

	// delete LeastExperiencedTimes; // Jason : release memory
	//if(PathArray!=NULL)
	//	delete PathArray;
}



void DTANetworkForSP::HistInfoZoneBasedPathAssignment(int zone, int departure_time_begin, int departure_time_end)
// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
{

	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int NodeSize;
	int PredNode;
	int DepartureTimeInterval = int(departure_time_begin / g_AggregationTimetInterval);  // starting assignment interval

	// loop through the TDOVehicleArray to assign or update vehicle paths... : iteration 0: for all vehicles
	for (int vi = 0; vi<g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][DepartureTimeInterval].VehicleArray.size(); vi++)
	{
		int VehicleID = g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][DepartureTimeInterval].VehicleArray[vi];

		{
			DTAVehicle* pVeh = g_VehicleMap[VehicleID];
			ASSERT(pVeh != NULL);

			if (pVeh->m_NodeSize >0) // path assigned (from input_agent.csv)
				continue;

			BuildHistoricalInfoNetwork(zone, pVeh->m_DepartureTime, g_UserClassPerceptionErrorRatio[1]);  // build network for this zone, because different zones have different connectors...
			//using historical short-term travel time
			TDLabelCorrecting_DoubleQueue(g_NodeVector.size(), pVeh->m_DepartureTime, pVeh->m_PricingType, pVeh->m_VOT, false, false, false);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

			int OriginCentriod = m_PhysicalNodeSize;  // as root node
			int DestinationCentriod = m_PhysicalNodeSize + pVeh->m_DestinationZoneID;

			float TotalCost = LabelCostAry[DestinationCentriod];
			if (TotalCost > MAX_SPLABEL - 10)
			{
				cout << "Warning: vehicle " << pVeh->m_VehicleID << " from zone " << pVeh->m_OriginZoneID << " to zone " << pVeh->m_DestinationZoneID << " does not have a physical path. Please check warning.log for details. " << endl;
				g_WarningFile << "Warning: vehicle " << pVeh->m_VehicleID << " from zone " << pVeh->m_OriginZoneID << " to zone " << pVeh->m_DestinationZoneID << " does not have a physical path. " << endl;
				pVeh->m_NodeSize = 0;
				continue;

			}


			// starting from destination centriod
			NodeSize = 0;
			PredNode = NodePredAry[DestinationCentriod];  // PredNode is a physical node here.
			// stop when reaching origin centriod, back trace to origin zone as root node
			while (PredNode != OriginCentriod && PredNode != -1 && NodeSize < MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
			{
				ASSERT(NodeSize < MAX_NODE_SIZE_IN_A_PATH - 1);
				PathNodeList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				PredNode = NodePredAry[PredNode];
			}

			// PathNodeList stores all physical nodes

			pVeh->m_NodeSize = NodeSize;

			if (pVeh->m_LinkAry != NULL)
			{
				delete pVeh->m_LinkAry;
			}

			if (pVeh->m_NodeSize >= 2)
			{

				if (NodeSize >= MAX_NODE_SIZE_IN_A_PATH)
				{
					cout << "PATH Size is out of bound " << NodeSize;
					fprintf(g_ErrorFile, "Path for vehicle %d from zone %d to zone %d cannot be found. It might be due to connectivity issue.\n", VehicleID, zone, pVeh->m_DestinationZoneID);
					pVeh->m_NodeSize = 0;
					return;

				}

				pVeh->m_LinkAry = new SVehicleLink[NodeSize];

				if (pVeh->m_LinkAry == NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				pVeh->m_NodeNumberSum = 0;
				pVeh->m_Distance = 0;

				for (int i = 0; i < NodeSize - 1; i++)
				{
					//					TRACE("ID:%d, %d \n",i, g_NodeVector[PathNodeList[i]]);
					pVeh->m_LinkAry[i].LinkNo = GetLinkNoByNodeIndex(PathNodeList[NodeSize - i - 1], PathNodeList[NodeSize - i - 2]);
					pVeh->m_NodeNumberSum += PathNodeList[NodeSize - i - 2];

					if (g_LinkVector[pVeh->m_LinkAry[i].LinkNo] == NULL)
					{
						cout << "Error: g_LinkVector[pVeh->m_LinkAry [i].LinkNo]==NULL", pVeh->m_LinkAry[i].LinkNo;
						getchar();
						exit(0);
					}

					pVeh->m_Distance += g_LinkVector[pVeh->m_LinkAry[i].LinkNo]->m_Length;
				}
				//cout << pVeh->m_VehicleID <<  " Distance" << pVeh->m_Distance <<  endl;;
			}

		}
	}

}



void ConstructPathArrayForEachODT(PathArrayForEachODT PathArray[], int zone, int AssignmentInterval)
{  // this function has been enhanced for path flow adjustment
	int DestZoneID;
	for (DestZoneID = 1; DestZoneID <= g_ODZoneNumberSize; DestZoneID++) // initialization...
	{

		PathArray[DestZoneID].NumOfPaths = 0;
		PathArray[DestZoneID].NumOfVehicles = 0;
		PathArray[DestZoneID].DeviationNumOfVehicles = 0;
		PathArray[DestZoneID].BestPathIndex = 0;
		for (int p = 0; p < _MAX_ODT_PATH_SIZE_4_ODME; p++)
		{
			PathArray[DestZoneID].NumOfVehsOnEachPath[p] = 0;
			PathArray[DestZoneID].PathNodeSums[p] = 0;
			PathArray[DestZoneID].AvgPathTimes[p] = 0.0;
			PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] = 0.0;
			PathArray[DestZoneID].PathSize[p] = 0;
			// no need for initialization here
			//			for(int q=0; q< _MAX_PATH_NODE_SIZE_4_ODME; q++)
			//				PathArray[DestZoneID].PathLinkSequences[p][q] = 0.0;
		}
	}

	// Scan all vehicles and construct path array for each destination
	for (int vi = 0; vi < g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][AssignmentInterval].VehicleArray.size(); vi++)
	{
		int VehicleID = g_TDOVehicleArray[g_ZoneMap[zone].m_ZoneSequentialNo][AssignmentInterval].VehicleArray[vi];
		DTAVehicle* pVeh = g_VehicleMap[VehicleID];
		ASSERT(pVeh != NULL);

		int VehicleDest = pVeh->m_DestinationZoneID;
		float TripTime = pVeh->m_TripTime;
		int NodeSum = pVeh->m_NodeNumberSum;
		int NodeSize = pVeh->m_NodeSize;

		if (NodeSize >= _MAX_PATH_NODE_SIZE_4_ODME - 2)
		{
			cout << "ODME error: vehicle id " << VehicleID << " has NodeSize = " << NodeSize << ", which is greater than array size = " << _MAX_PATH_NODE_SIZE_4_ODME << endl;
			g_ProgramStop();

		}

		if (PathArray[VehicleDest].NumOfPaths == 0) // the first path for VehicleDest
		{
			PathArray[VehicleDest].NumOfPaths++;
			PathArray[VehicleDest].NumOfVehsOnEachPath[PathArray[VehicleDest].NumOfPaths]++;
			PathArray[VehicleDest].PathNodeSums[PathArray[VehicleDest].NumOfPaths] = NodeSum;
			PathArray[VehicleDest].AvgPathTimes[PathArray[VehicleDest].NumOfPaths] = TripTime;
			// obtain path link sequence from vehicle link sequence
			PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].NumOfPaths].LinkNoVector.clear();
			for (int i = 0; i < NodeSize - 1; i++)
			{
				PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].NumOfPaths].LinkNoVector.push_back(pVeh->m_LinkAry[i].LinkNo);
			}
			PathArray[VehicleDest].PathSize[PathArray[VehicleDest].NumOfPaths] = NodeSize;
		}
		else{
			int PathIndex = 0;
			for (int p = 1; p <= PathArray[VehicleDest].NumOfPaths; p++)
			{
				if (NodeSum == PathArray[VehicleDest].PathNodeSums[p])
				{
					PathIndex = p; // this veh uses the p-th in the set
					break;
				}
			}

			PathArray[VehicleDest].NumOfVehicles++;  // count the number of vehicles from o to d at assignment time interval tau

			if (PathIndex == 0) // a new path is found
			{
				PathArray[VehicleDest].NumOfPaths++;
				PathArray[VehicleDest].NumOfVehsOnEachPath[PathArray[VehicleDest].NumOfPaths]++;
				PathArray[VehicleDest].PathNodeSums[PathArray[VehicleDest].NumOfPaths] = NodeSum;
				PathArray[VehicleDest].AvgPathTimes[PathArray[VehicleDest].NumOfPaths] = TripTime;
				// obtain path link sequence from vehicle link sequence

				PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].NumOfPaths].LinkNoVector.clear();
				for (int i = 0; i < NodeSize - 1; i++)
				{
					PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].NumOfPaths].LinkNoVector.push_back(pVeh->m_LinkAry[i].LinkNo);
				}
				PathArray[VehicleDest].PathSize[PathArray[VehicleDest].NumOfPaths] = NodeSize;
			}
			else{ // an existing path found
				PathArray[VehicleDest].AvgPathTimes[PathIndex] = (PathArray[VehicleDest].AvgPathTimes[PathIndex] * PathArray[VehicleDest].NumOfVehsOnEachPath[PathIndex] + TripTime) / (PathArray[VehicleDest].NumOfVehsOnEachPath[PathIndex] + 1);
				PathArray[VehicleDest].NumOfVehsOnEachPath[PathIndex]++;
			}
		}
	}

	// identify the best path for each destination
	// calculate the path gap
	for (int DestZoneID = 1; DestZoneID <= g_ODZoneNumberSize; DestZoneID++)
	{
		if (PathArray[DestZoneID].NumOfPaths > 0)
		{
			int BestPath = 0;
			float BestTime = 999999.0;

			int p;
			for (p = 1; p <= PathArray[DestZoneID].NumOfPaths; p++)
			{
				if (PathArray[DestZoneID].AvgPathTimes[p] < BestTime)
				{
					BestTime = PathArray[DestZoneID].AvgPathTimes[p];
					BestPath = p;
				}
			}
			PathArray[DestZoneID].BestPathIndex = BestPath;
			PathArray[DestZoneID].LeastTravelTime = BestTime;


			for (p = 1; p <= PathArray[DestZoneID].NumOfPaths; p++)
			{

				PathArray[DestZoneID].AvgPathGap[p] = PathArray[DestZoneID].AvgPathTimes[p] - BestTime;
			}

		}
	}


}



void g_AgentBasedShortestPathGeneration()
{

	// find unique origin node
	// find unique destination node

	int node_size = g_NodeVector.size();
	int link_size = g_LinkVector.size();

	int line = 0;

	FILE* st_input = NULL;
	fopen_s(&st_input, "input_od_pairs.csv", "r");
	if (st_input != NULL)
	{
		char str[100];

		fscanf(st_input, "%[^\n]", str);  // read a line

		int origin_node_id, destination_node_id, record_id;

		while (!feof(st_input))
		{
			origin_node_id = 0;
			destination_node_id = 0;
			record_id = 0;

			int number_of_values = fscanf(st_input, "%d,%d,%d\n", &record_id, &origin_node_id, &destination_node_id);

			// the expected number of items = 3;
			if (number_of_values < 3)
				break;

			TRACE("%d;%d;%d\n", record_id, &origin_node_id, &destination_node_id);

			if (g_NodeNametoIDMap.find(origin_node_id) == g_NodeNametoIDMap.end())
			{
				//				cout<< "origin_node_id "  << origin_node_id << " in input_od_pairs.csv has not be defined in input_node.csv.  Please check line =" << line  << endl; 
				//				getchar();
				//				exit(0);
				continue;
			}

			if (g_NodeNametoIDMap.find(destination_node_id) == g_NodeNametoIDMap.end())
			{
				//				cout<< "destination_node_id "  << destination_node_id << " in input_od_pairs.csv has not be defined in input_node.csv. Please check line =" << line   << endl; 
				//				getchar();
				//				exit(0);
				continue;
			}

			int number_indx = g_NodeNametoIDMap[origin_node_id];
			int dest_node_index = g_NodeNametoIDMap[destination_node_id];

			DTADestination element;
			element.destination_number = destination_node_id;
			element.record_id = record_id;
			element.destination_node_index = dest_node_index;

			g_NodeVector[number_indx].m_DestinationVector.push_back(element);
			g_NodeVector[number_indx].m_bOriginFlag = true;

			if (line % 10000 == 0)
				cout << g_GetAppRunningTime() << " reading line " << line / 1000 << "k in input_od_pairs.csv." << endl;
			line++;
		}
		fclose(st_input);
	}
	else
	{
		cout << "File input_od_pairs.csv cannot be opened. Please check!" << endl;
		g_ProgramStop();
	}


	unsigned int i;
	int UniqueOriginSize = 0;
	int UniqueDestinationSize = 0;
	int number_of_threads = omp_get_max_threads();

	if (g_ParallelComputingMode == 0)
		number_of_threads = 1;

	for (i = 0; i < g_NodeVector.size(); i++)
	{
		if (g_NodeVector[i].m_bOriginFlag == true)
		{
			UniqueOriginSize += 1;
		}



		if (g_NodeVector[i].m_bDestinationFlag == true)
			UniqueDestinationSize += 1;

	}

	cout << "# of OD pairs = " << line << endl;
	cout << "# of unique origins = " << UniqueOriginSize << " with " << line / UniqueOriginSize << " nodes per origin" << endl;
	cout << "# of processors = " << number_of_threads << endl;

	g_LogFile << "# of OD pairs = " << line << endl;

	g_LogFile << "# of unique origins = " << UniqueOriginSize << " with " << line / UniqueOriginSize << " nodes per origin" << endl;
	g_LogFile << g_GetAppRunningTime() << "# of processors = " << number_of_threads << endl;



#pragma omp parallel for
	for (int ProcessID = 0; ProcessID < number_of_threads; ProcessID++)
	{
		// create network for shortest path calculation at this processor
		DTANetworkForSP g_TimeDependentNetwork_MP(node_size, link_size, 1, g_AdjLinkSize); //  network instance for single processor in multi-thread environment
		int	cpu_id = omp_get_thread_num();  // starting from 0
		g_TimeDependentNetwork_MP.BuildPhysicalNetwork(0, 0, g_TrafficFlowModelFlag);  // build network for this zone, because different zones have different connectors...

		for (int node_index = 0; node_index < node_size; node_index++)
		{
			if (node_index %number_of_threads == ProcessID && g_NodeVector[node_index].m_bOriginFlag)
			{
				if (node_index % 100 == cpu_id)
				{

					if (node_index % 100 == 0)  // only one CUP can output to log file
					{
						cout << g_GetAppRunningTime() << "processor " << cpu_id << " working on node  " << node_index << ", " << node_index*1.0f / node_size * 100 << "%" << endl;
						g_LogFile << g_GetAppRunningTime() << "processor " << cpu_id << " working on node  " << node_index << ", " << node_index*1.0f / node_size * 100 << "%" << endl;
					}
				}

				// this generate the one-to-all shortest path tree
				g_TimeDependentNetwork_MP.TDLabelCorrecting_DoubleQueue(node_index, 0, 1, DEFAULT_VOT, true, true, false);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

				for (int dest_no = 0; dest_no < g_NodeVector[node_index].m_DestinationVector.size(); dest_no++)
				{
					int dest_node_index = g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_index;
					g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_distance = g_TimeDependentNetwork_MP.LabelCostAry[dest_node_index];
					//					 TRACE("Label: %f: \n",g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_cost_label);

				}

			}
		}
	}


	FILE* st = NULL;
	fopen_s(&st, "output_shortest_path.txt", "w");
	if (st != NULL)
	{
		fprintf(st, "record_id,from_node_id,to_node_id,distance\n");
		for (int node_index = 0; node_index < node_size; node_index++)
		{
			if (node_index % 100000 == 0)
				cout << g_GetAppRunningTime() << " Computation engine is outputing results for node sequence " << node_index << endl;
			for (int dest_no = 0; dest_no < g_NodeVector[node_index].m_DestinationVector.size(); dest_no++)
			{
				int dest_node_index = g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_index;
				float label = g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_distance;

				fprintf(st, "%d,%d,%d,%4.2f\n", g_NodeVector[node_index].m_DestinationVector[dest_no].record_id,
					g_NodeVector[node_index].m_NodeNumber,
					g_NodeVector[node_index].m_DestinationVector[dest_no].destination_number, label);

			}

		}

		fclose(st);
	}
	else
	{
		cout << "File output_shortest_path.csv cannot be opened. Please check!" << endl;
		g_ProgramStop();

	}

	cout << g_GetAppRunningTime() << " Done!" << endl;

}




void g_GenerateSimulationSummary(int iteration, bool NotConverged, int TotalNumOfVehiclesGenerated, NetworkLoadingOutput* p_SimuOutput)
{
	if (g_AssignmentMOEVector.size() == 0)  // no assignment being involved
		return;

	if (g_EmissionDataOutputFlag==2)   //output emission for each iteration 
		g_CalculateEmissionMOE();

	TotalNumOfVehiclesGenerated = p_SimuOutput->NumberofVehiclesGenerated; // need this to compute avg gap

	g_AssignmentMOEVector[iteration] = (*p_SimuOutput);

	if (iteration >= 1) // Note: we output the gap for the last iteration, so "iteration-1"
	{
		//agent based, we record gaps only for vehicles switched (after they find the paths)
		p_SimuOutput->AvgUEGap = g_CurrentGapValue / max(1, g_CurrentNumOfVehiclesForUEGapCalculation);
		p_SimuOutput->AvgRelativeUEGap = g_CurrentRelativeGapValue * 100 / max(1, g_CurrentNumOfVehiclesForUEGapCalculation);
		g_PrevRelativeGapValue = p_SimuOutput->AvgRelativeUEGap;

	}

	float PercentageComplete = 0;

	if (p_SimuOutput->NumberofVehiclesGenerated >= 1)
		PercentageComplete = p_SimuOutput->NumberofVehiclesCompleteTrips*100.0f / max(1, p_SimuOutput->NumberofVehiclesGenerated);

	g_LogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Average Trip Time: " << p_SimuOutput->AvgTravelTime << ", Travel Time Index: " << p_SimuOutput->AvgTTI << ", Average Distance: " << p_SimuOutput->AvgDistance << ", Switch %:" << p_SimuOutput->SwitchPercentage << ", Number of Vehicles Complete Their Trips: " << p_SimuOutput->NumberofVehiclesCompleteTrips << ", " << PercentageComplete << "%" << endl;
	cout << g_GetAppRunningTime() << "Iter: " << iteration << ", Avg Trip Time: " << p_SimuOutput->AvgTripTime << ", Avg Trip Time Index: " << p_SimuOutput->AvgTTI << ", Avg Dist: " << p_SimuOutput->AvgDistance << ", Switch %:" << p_SimuOutput->SwitchPercentage << ", # of veh Complete Trips: " << p_SimuOutput->NumberofVehiclesCompleteTrips << ", " << PercentageComplete << "%" << endl;

	g_AssignmentLogFile << g_GetAppRunningTime() << "," << iteration << "," << p_SimuOutput->AvgTravelTime << "," << p_SimuOutput->AvgTTI << "," << p_SimuOutput->AvgDistance << "," << p_SimuOutput->SwitchPercentage << "," << p_SimuOutput->NumberofVehiclesCompleteTrips << "," << PercentageComplete << "%,";

	g_AssignmentLogFile << p_SimuOutput->AvgUEGap << "," << p_SimuOutput->TotalDemandDeviation << "," << p_SimuOutput->LinkVolumeAvgAbsError << "," << p_SimuOutput->LinkVolumeRootMeanSquaredError << "," << p_SimuOutput->LinkVolumeAvgAbsPercentageError;

	g_AssignmentLogFile << endl;

	int time_interval = 15; // min
	int start_time = (int)(g_DemandLoadingStartTimeInMin / time_interval)*time_interval;

	if (iteration == 0)
	{

		g_SummaryStatFile.Reset();
		g_SummaryStatFile.SetFieldName("Iteration #");
		g_SummaryStatFile.SetFieldName("CPU Running Time");
		g_SummaryStatFile.SetFieldName("Per Iteration CPU Running Time");
		g_SummaryStatFile.SetFieldName("# of agents");
		g_SummaryStatFile.SetFieldName("Avg Travel Time (min)");
		g_SummaryStatFile.SetFieldName("Avg Trip Time (min)");
		g_SummaryStatFile.SetFieldName("Avg Waiting Time at Origin (min)");
		g_SummaryStatFile.SetFieldName("Avg Trip Time Index=(Mean TT/Free-flow TT)");
		g_SummaryStatFile.SetFieldName("Avg Distance (miles)");
		g_SummaryStatFile.SetFieldName("Avg Speed (mph)");

		g_SummaryStatFile.SetFieldName("% considering to switch");
		g_SummaryStatFile.SetFieldName("% switched");
		g_SummaryStatFile.SetFieldName("% completing trips");
		g_SummaryStatFile.SetFieldName("network clearance time (in min)");
		g_SummaryStatFile.SetFieldName("Avg UE gap (min)");
		g_SummaryStatFile.SetFieldName("Relative UE gap (%)");



		//		if(g_ODEstimationFlag == 1)
		//		{
		//			g_SummaryStatFile.SetFieldName ("Demand Deviation");
		g_SummaryStatFile.SetFieldName("ODME: number of data points");

		g_SummaryStatFile.SetFieldName("ODME: Absolute link count error");
		g_SummaryStatFile.SetFieldName("ODME: % link count error");
		g_SummaryStatFile.SetFieldName("ODME: slope =observed/estimated link count");
		g_SummaryStatFile.SetFieldName("ODME: r_squared link count");


		if (g_ObsDensityAvailableFlag)
		{
			g_SummaryStatFile.SetFieldName("ODME: Absolute lane density error");
			g_SummaryStatFile.SetFieldName("ODME: % lane density error");
			g_SummaryStatFile.SetFieldName("ODME: slope =observed/estimated lane density");
			g_SummaryStatFile.SetFieldName("ODME: r_squared lane density");
		}

		//		g_SummaryStatFile.SetFieldName ("ODME: avg_simulated_to_avg_obs");
		//		}


		for (int time = start_time; time < g_DemandLoadingEndTimeInMin; time += time_interval)
		{

			std::string time_str = "travel_time_" + GetTimeClockString(time);
			g_SummaryStatFile.SetFieldName(time_str);

		}
		for (int time = start_time; time < g_DemandLoadingEndTimeInMin; time += time_interval)
		{

			std::string time_str = "Avg UE gap_" + GetTimeClockString(time);
			g_SummaryStatFile.SetFieldName(time_str);

		}

		for (int time = start_time; time < g_DemandLoadingEndTimeInMin; time += time_interval)
		{

			std::string time_str = "Relative UE gap_" + GetTimeClockString(time);
			g_SummaryStatFile.SetFieldName(time_str);

		}
		cout << "Avg Gap: " << p_SimuOutput->AvgUEGap;


		cout << endl;

		g_SummaryStatFile.WriteHeader(false, false);

	}

	int time_dependent_skim_file_output = g_GetPrivateProfileInt("ABM_integeration", "time_dependent_skim_file_output", 0, g_DTASettingFileName);

	if (time_dependent_skim_file_output > 0)
	{
		//
		std::string skim_file_name;
		int day = iteration + 1;

		CString day_str;
		day_str.Format("%d", day);


		skim_file_name = "OD_skim_day" + CString2StdString(day_str) + ".csv";
		cout << "outputing skim file" << skim_file_name << " ..." << endl;
		g_AgentBasedAccessibilityMatrixGeneration(skim_file_name, true, 1, 0);
		cout << "outputing skim file for all demand types" << skim_file_name << " ..." << endl;
		skim_file_name = "output_OD_skim_day" + CString2StdString(day_str) + ".csv";
		g_AccessibilityMatrixGenerationForAllDemandTypes(skim_file_name, true, 0);

		cout << "outputing link time-dependent MOE and trajectory data under skim outputting mode ..." << endl;
		g_OutputMOEData(iteration);

	}


	int time_dependent_HOV_skim_file_output = g_GetPrivateProfileInt("ABM_integeration", "time_dependent_HOV_skim_file_output", 0, g_DTASettingFileName);

	if (time_dependent_HOV_skim_file_output > 0)
	{
		//
		std::string skim_file_name;
		int day = iteration + 1;

		CString day_str;
		day_str.Format("%d", day);


		skim_file_name = "OD_HOV_skim_day" + CString2StdString(day_str) + ".csv";
		cout << "outputing HOV skim file" << skim_file_name << " ..." << endl;
		g_AgentBasedAccessibilityMatrixGeneration(skim_file_name, true, 2, 0);
	}
	int day_no = iteration + 1;

	g_SummaryStatFile.SetValueByFieldName("Iteration #", day_no);  // iteration from 0
	g_SummaryStatFile.SetValueByFieldName("CPU Running Time", g_GetAppRunningTime(false));
	g_SummaryStatFile.SetValueByFieldName("Per Iteration CPU Running Time", g_GetAppRunningTimePerIteration(false));

	g_AppLastIterationStartTime = CTime::GetCurrentTime();

	g_SummaryStatFile.SetValueByFieldName("# of agents", p_SimuOutput->NumberofVehiclesGenerated);
	g_SummaryStatFile.SetValueByFieldName("Avg Travel Time (min)", p_SimuOutput->AvgTravelTime);
	g_SummaryStatFile.SetValueByFieldName("Avg Trip Time (min)", p_SimuOutput->AvgTripTime);

	float buffer_waiting_time = p_SimuOutput->AvgTripTime - p_SimuOutput->AvgTravelTime;
	g_SummaryStatFile.SetValueByFieldName("Avg Waiting Time at Origin (min)", buffer_waiting_time);
	g_SummaryStatFile.SetValueByFieldName("Avg Trip Time Index=(Mean TT/Free-flow TT)", p_SimuOutput->AvgTTI);
	g_SummaryStatFile.SetValueByFieldName("Avg Distance (miles)", p_SimuOutput->AvgDistance);

	float avg_speed = p_SimuOutput->AvgDistance / max(0.1, p_SimuOutput->AvgTravelTime) * 60;

	g_SummaryStatFile.SetValueByFieldName("Avg Speed (mph)", avg_speed);
	g_SummaryStatFile.SetValueByFieldName("% switched", p_SimuOutput->SwitchPercentage);
	g_SummaryStatFile.SetValueByFieldName("% considering to switch", p_SimuOutput->ConsideringSwitchPercentage);

	g_SummaryStatFile.SetValueByFieldName("network clearance time (in min)", p_SimuOutput->NetworkClearanceTimeStamp_in_Min);

	g_SummaryStatFile.SetValueByFieldName("% completing trips", PercentageComplete);

	g_PercentageCompleteTrips = PercentageComplete;

	if (g_ODEstimationFlag == 1 && iteration >= g_ODEstimation_StartingIteration)
	{
		p_SimuOutput->AvgUEGap = 0;
		p_SimuOutput->AvgRelativeUEGap = 0;

		g_SummaryStatFile.SetValueByFieldName("ODME: number of data points", p_SimuOutput->ODME_result_link_count.data_size);
		g_SummaryStatFile.SetValueByFieldName("ODME: Absolute link count error", p_SimuOutput->LinkVolumeAvgAbsError);
		g_SummaryStatFile.SetValueByFieldName("ODME: % link count error", p_SimuOutput->LinkVolumeAvgAbsPercentageError);

		g_SummaryStatFile.SetValueByFieldName("ODME: slope =observed/estimated link count", p_SimuOutput->ODME_result_link_count.slope);
		g_SummaryStatFile.SetValueByFieldName("ODME: r_squared link count", p_SimuOutput->ODME_result_link_count.rsqr);

		if (g_ObsDensityAvailableFlag)
		{
			g_SummaryStatFile.SetValueByFieldName("ODME: slope =observed/estimated lane density", p_SimuOutput->ODME_result_lane_density.slope);
			g_SummaryStatFile.SetValueByFieldName("ODME: r_squared lane density", p_SimuOutput->ODME_result_lane_density.rsqr);
			g_SummaryStatFile.SetValueByFieldName("ODME: Absolute lane density error", p_SimuOutput->ODME_result_lane_density.avg_absolute_error);
			g_SummaryStatFile.SetValueByFieldName("ODME: % lane density error", p_SimuOutput->ODME_result_lane_density.avg_percentage_error);
		}
		//		g_SummaryStatFile.SetValueByFieldName ("ODME: avg_simulated_to_avg_obs",p_SimuOutput->ODME_result_link_count.avg_y_to_x_ratio  );
	}

	if (g_ODEstimationFlag == 1 && iteration >= g_ODEstimation_StartingIteration)
	{
		//ODME gap results
		float AvgUEGap = g_CurrentGapValue / max(1, p_SimuOutput->NumberofVehiclesGenerated);
		//		float AvgRelativeUEGap = g_CurrentRelativeGapValue / max(1, p_SimuOutput->NumberofVehiclesGenerated);
		float AvgRelativeUEGap = 0;
		g_SummaryStatFile.SetValueByFieldName("Avg UE gap (min)", AvgUEGap);
		g_SummaryStatFile.SetValueByFieldName("Relative UE gap (%)", AvgRelativeUEGap);


	}
	else  // simulation gap
	{
		if (iteration >= 1)
		{
			g_SummaryStatFile.SetValueByFieldName("Avg UE gap (min)", p_SimuOutput->AvgUEGap);
			g_SummaryStatFile.SetValueByFieldName("Relative UE gap (%)", p_SimuOutput->AvgRelativeUEGap);
		}

	}

	//-- time dependent MOE 

	for (int time = start_time; time < g_DemandLoadingEndTimeInMin; time += time_interval)
	{

		std::string time_str = "travel_time_" + GetTimeClockString(time);

		int time_interval_ = time / 15;
		double travel_time = p_SimuOutput->GetTimeDependentAvgTravelTime(time);

		g_SummaryStatFile.SetValueByFieldName(time_str, travel_time);

	}

	if (iteration >= 1)
	{

		for (int time = start_time; time < g_DemandLoadingEndTimeInMin; time += time_interval)
		{

			std::string time_str = "Avg UE gap_" + GetTimeClockString(time);
			double travel_time_gap = p_SimuOutput->GetTimeDependentAvgGap(time);

			g_SummaryStatFile.SetValueByFieldName(time_str, travel_time_gap);

		}

		for (int time = start_time; time < g_DemandLoadingEndTimeInMin; time += time_interval)
		{

			std::string time_str = "Relative UE gap_" + GetTimeClockString(time);

			double travel_relative_time_gap = p_SimuOutput->GetTimeDependentAvgRelativeGapInPercentage(time);

			g_SummaryStatFile.SetValueByFieldName(time_str, travel_relative_time_gap);

		}
	}

	g_SummaryStatFile.WriteRecord();
	g_WriteUserDefinedMOE(g_MultiScenarioSummaryStatFile, iteration + 1);

	////	if(0) //comment out day to day code
	//	{
	//
	//	unsigned li;
	//	for(li = 0; li< g_LinkVector.size(); li++)
	//	{
	//		DTALink* pLink = g_LinkVector[li];
	//
	//		Day2DayLinkMOE element;
	//		element.TotalFlowCount  = pLink->CFlowArrivalCount;
	//		element.AvgTravelTime= pLink->GetTravelTimeByMin(iteration,0, pLink->m_SimulationHorizon,g_TrafficFlowModelFlag);
	//		element.AvgSpeed = pLink->m_Length / max(0.00001,element.AvgTravelTime) *60;  // unit: mph
	//
	//		for(int i = 1; i < MAX_PRICING_TYPE_SIZE; i++)
	//		{
	//			element.CumulativeArrivalCount_PricingType[i] = pLink->CFlowArrivalCount_PricingType[i];
	//		}
	//
	//		element.m_NumberOfCrashes =  pLink->m_NumberOfCrashes;
	//		element.m_NumberOfFatalAndInjuryCrashes = pLink->m_NumberOfFatalAndInjuryCrashes;
	//		element.m_NumberOfPDOCrashes = pLink->m_NumberOfPDOCrashes;
	//
	//		pLink->m_Day2DayLinkMOEVector .push_back (element);
	//	}
	//
	//	}

	//if(g_ODEstimationFlag == 1)
	//	cout << "Avg Gap: " << p_SimuOutput->AvgUEGap   << ", Demand Dev:"	<< p_SimuOutput->TotalDemandDeviation << ", Avg volume error: " << p_SimuOutput->LinkVolumeAvgAbsError << ", Avg % error: " << p_SimuOutput->LinkVolumeAvgAbsPercentageError << endl;

	p_SimuOutput->ResetStatistics();

	// with or without inner loop 
	if (g_NumberOfInnerIterations == 0) // without inner loop
	{
		// check outer loop convergence (without inner loop)
		if (g_UEAssignmentMethod <= 1) // MSA and day-to-day learning
		{
			if (g_CurrentNumOfVehiclesSwitched < g_ConvergenceThreshold_in_Num_Switch)
				NotConverged = false; // converged!
		}
		else // gap-based approaches
		{
			//if(g_RelativeGap < g_ConvergencyRelativeGapThreshold_in_perc && !g_ODEstimationFlag )
			//	NotConverged = false; // converged! 
		}
	}
	else // ----------* with inner loop *----------
	{
		g_DynamicTraffcAssignmentWithinInnerLoop(iteration, NotConverged, TotalNumOfVehiclesGenerated);

	}	// end - with inner loop
}

void g_ZoneBasedDynamicTrafficAssignment()
{
	int node_size = g_NodeVector.size() + 1 + g_ODZoneNumberSize;

	int connector_count = 0;

	for (std::map<int, DTAZone>::iterator iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
	{
		connector_count += (iterZone->second.m_OriginActivityVector.size() + iterZone->second.m_DestinationActivityVector.size());  // only this origin zone has vehicles, then we build the network
	}

	int link_size = g_LinkVector.size() + connector_count; // maximal number of links including connectors assuming all the nodes are destinations

	g_LogFile << "Number of iterations = " << g_NumberOfIterations << endl;

	// Jason
	int iteration = 0;
	bool NotConverged = true;
	int TotalNumOfVehiclesGenerated = 0;
	int number_of_threads = g_number_of_CPU_threads();

	cout << "# of Computer Processors = " << number_of_threads << endl;

	int Current_AggregationTimetInterval = g_AggregationTimetInterval;
	// ----------* start of outer loop *----------
	for (iteration = 0; NotConverged && iteration <= g_NumberOfIterations; iteration++)  // we exit from the loop under two conditions (1) converged, (2) reach maximum number of iterations
	{
		cout << "------- Iteration = " << iteration << "--------" << endl;

		// initialize for each iteration
		g_CurrentGapValue = 0.0;
		g_CurrentRelativeGapValue = 0.0;
		g_CurrentNumOfVehiclesForUEGapCalculation = 0;
		g_CurrentNumOfVehiclesSwitched = 0;
		g_NewPathWithSwitchedVehicles = 0;

		// initialize for OD estimation
		g_TotalDemandDeviation = 0;
		g_TotalMeasurementDeviation = 0;
		int Actual_ODZoneSize = g_ZoneMap.size();  // Actual_ODZoneSize can be < ODZoneSize after subarea cut with new zones

		if (!(g_VehicleLoadingMode == vehicle_binary_file_mode && iteration == 0))  // we do not need to generate initial paths for vehicles for the first iteration of vehicle loading mode
		{
			g_EstimationLogFile << "----- Iteration = " << iteration << " ------" << endl;

			if (g_ODEstimationFlag && iteration >= g_ODEstimation_StartingIteration)
				g_HistDemand.ResetUpdatedValue(); // reset update hist table

#pragma omp parallel for
			for (int ProcessID = 0; ProcessID < number_of_threads; ProcessID++)
			{
				// create network for shortest path calculation at this processor
				int	id = omp_get_thread_num();  // starting from 0

				cout << "Processor " << id << " is working on shortest path calculation..  " << endl;
				//special notes: creating network with dynamic memory is a time-consumping task, so we create the network once for each processors

				for (int CurZoneID = 1; CurZoneID <= g_ODZoneNumberSize; CurZoneID++)
				{
					if (g_ZoneMap.find(CurZoneID) == g_ZoneMap.end())  // no such zone being defined
						continue;

					if (g_ProgramStopFlag == 1)
						g_ProgramStop();



					if ((CurZoneID%number_of_threads) == ProcessID)  // if the remainder of a zone id (devided by the total number of processsors) equals to the processor id, then this zone id is 
					{
						if (g_ZoneMap[CurZoneID].m_OriginVehicleSize > 0)  // only this origin zone has vehicles, then we build the network
						{
							// create network for shortest path calculation at this processor
							g_TimeDependentNetwork_MP[ProcessID].BuildNetworkBasedOnZoneCentriod(iteration, CurZoneID);  // build network for this zone, because different zones have different connectors...

							if (Actual_ODZoneSize > 300)  // only for large networks
							{
								cout << "Processor " << id << " is calculating the shortest paths for zone " << CurZoneID << endl;
							}


							int aggregation_interval = g_AggregationTimetInterval;

							//if (iteration == 0)
							//	aggregation_interval = 1440;

							// scan all possible departure times
							for (int departure_time = g_DemandLoadingStartTimeInMin; departure_time < g_DemandLoadingEndTimeInMin; departure_time += aggregation_interval)
							{

								//if (departure_time < 360 || departure_time >= 16*60)
								//	aggregation_interval = 360;
								//else
								//	aggregation_interval = 60;

								if (g_TDOVehicleArray[g_ZoneMap[CurZoneID].m_ZoneSequentialNo][departure_time / g_AggregationTimetInterval].VehicleArray.size() > 0)
								{

									bool debug_flag = false;

									for (int pricing_type = 1; pricing_type < MAX_PRICING_TYPE_SIZE; pricing_type++)  // from SOV, HOV, truck
									{
										g_TimeDependentNetwork_MP[ProcessID].TDLabelCorrecting_DoubleQueue_PerPricingType(g_NodeVector.size(), departure_time, pricing_type, g_PricingTypeMap[pricing_type].default_VOT, false, debug_flag);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo
									}

									if (g_ODEstimationFlag && iteration >= g_ODEstimation_StartingIteration)  // perform path flow adjustment after at least 10 normal OD estimation
										g_TimeDependentNetwork_MP[ProcessID].ZoneBasedPathAssignment_ODEstimation(CurZoneID, departure_time, departure_time + g_AggregationTimetInterval, iteration);
									else
										g_TimeDependentNetwork_MP[ProcessID].ZoneBasedPathAssignment(CurZoneID, departure_time, departure_time + g_AggregationTimetInterval, iteration, debug_flag);

								}
							} // for each departure time

						} // for zone with volume
					} // for zone id assigned to the processor id
				}	// for each zone
			}  // for each processor
			//the OD estimation code below should be single thread

			if (g_ODEstimationFlag && iteration >= g_ODEstimation_StartingIteration)  // re-generate vehicles based on global path set
			{
				g_GenerateVehicleData_ODEstimation();
			}

			cout << "---- Network Loading for Iteration " << iteration << "----" << endl;

			//	 DTANetworkForSP network(node_size, link_size, g_DemandLoadingHorizon);  // network instance for single-thread application

			NetworkLoadingOutput SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag, 0, iteration);
			g_GenerateSimulationSummary(iteration, NotConverged, TotalNumOfVehiclesGenerated, &SimuOutput);

		}	// end of outer loop

	} // for each assignment iteration

	cout << "Writing Vehicle Trajectory and MOE File... " << endl;

	if (iteration == g_NumberOfIterations)
	{
		iteration = g_NumberOfIterations - 1;  //roll back to the last iteration if the ending condition is triggered by "iteration < g_NumberOfIterations"
	}

	g_OutputMOEData(iteration);

}

