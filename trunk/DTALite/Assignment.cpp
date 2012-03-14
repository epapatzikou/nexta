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

#include "CSVParser.h"
using namespace std;

std::vector<DTAPathData>   g_global_path_vector;	// global vector of path data
std::map<CString, int> g_path_index_map; 

extern ofstream g_WarningFile;

void ConstructPathArrayForEachODT(PathArrayForEachODT *, int, int); // construct path array for each ODT
void InnerLoopAssignment(int,int, int, int); // for inner loop assignment
void g_GenerateSimulationSummary(int iteration, bool NotConverged, int TotalNumOfVehiclesGenerated, NetworkLoadingOutput SimuOutput);
void g_OutputSimulationStatistics(int NumberOfLinks = 9999999);

void g_AgentBasedAssisnment()  // this is an adaptation of OD trip based assignment, we now generate and assign path for each individual vehicle (as an agent with personalized value of time, value of reliability)
{
		int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;
		int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

		// assign different zones to different processors
		int number_of_threads = omp_get_max_threads ( );

		cout<< "# of Computer Processors = "  << number_of_threads  << endl; 
		g_LogFile << "Number of iterations = " << g_NumberOfIterations << endl;

		int iteration = 0;
		bool NotConverged = true;
		int TotalNumOfVehiclesGenerated = 0;

		// ----------* start of outer loop *----------
		for(iteration=0; NotConverged && iteration <= g_NumberOfIterations; iteration++)  // we exit from the loop under two conditions (1) converged, (2) reach maximum number of iterations
		{
			cout << "------- Iteration = "<<  iteration << "--------" << endl;

			// initialize for each iteration
			g_CurrentGapValue = 0.0;
			g_CurrentNumOfVehiclesSwitched = 0;
			g_NewPathWithSwitchedVehicles = 0;

#pragma omp parallel for
			for(int ProcessID=0;  ProcessID < number_of_threads; ProcessID++)
			{
				// create network for shortest path calculation at this processor
				int	id = omp_get_thread_num( );  // starting from 0

				cout << "Processor " << id << " is working on shortest path calculation..  " << endl;

				DTANetworkForSP network_MP(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin); //  network instance for single processor in multi-thread environment
				network_MP.BuildPhysicalNetwork ();  // build network for this zone, because different zones have different connectors...
				//special notes: creating network with dynamic memory is a time-consumping task, so we create the network once for each processors

				for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
				{
	
					if(g_ODZoneSize > 300)  // only for large networks
					{
					cout << "Processor " << id << " is calculating the shortest paths for zone " << CurZoneID << endl;
					}

				if((CurZoneID%number_of_threads) == ProcessID)  // if the remainder of a zone id (devided by the total number of processsors) equals to the processor id, then this zone id is 
					{
						// scan all possible departure times
						for(int departure_time = g_DemandLoadingStartTimeInMin; departure_time < g_DemandLoadingEndTimeInMin; departure_time += g_AggregationTimetInterval)
						{

							if(g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray .size() > 0)
							{
								network_MP.AgentBasedPathFindingAssignment(CurZoneID,departure_time,departure_time+g_AggregationTimetInterval,iteration);
							}
						}  // for each departure time
					}
				}  // for each zone
			} // for each computer processor
			
			cout << "---- Network Loading for Iteration " << iteration <<"----" << endl;

			NetworkLoadingOutput SimuOutput;
			SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag,0,iteration);
			g_GenerateSimulationSummary(iteration,NotConverged, TotalNumOfVehiclesGenerated,SimuOutput);


		}  // for each assignment iteration

		cout << "Writing Vehicle Trajectory and MOE File... " << endl;

		if( iteration == g_NumberOfIterations)
		{ 
			iteration = g_NumberOfIterations -1;  //roll back to the last iteration if the ending condition is triggered by "iteration < g_NumberOfIterations"
		}

		g_OutputMOEData(iteration);

}


	void DTANetworkForSP::AgentBasedPathFindingAssignment(int zone,int departure_time_begin, int departure_time_end, int iteration)
		// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
	{
		int PathLinkList[MAX_NODE_SIZE_IN_A_PATH]={0};
		std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
		int NodeSize;
		int AssignmentInterval = int(departure_time_begin/g_AggregationTimetInterval);  // starting assignment interval

		int vehicle_id_trace  = 0;

		// loop through the TDOVehicleArray to assign or update vehicle paths...
for (int vi = 0; vi<g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray.size(); vi++)
{
			int VehicleID = g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray[vi];
			DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
			ASSERT(pVeh!=NULL);

			/// finding optimal path 
			bool bDebugFlag  = false;

			float TotalCost;
			bool bDistanceFlag = false;

			if(pVeh->m_VehicleID  == vehicle_id_trace)
			{
				TRACE("");
				bDebugFlag = true;
			}

			NodeSize = FindBestPathWithVOT(pVeh->m_OriginNodeID , pVeh->m_DepartureTime ,pVeh->m_DestinationNodeID, pVeh->m_PricingType , pVeh->m_VOT, PathLinkList, TotalCost,bDistanceFlag, bDebugFlag);
//			NodeSize = FindBestPathWithVOT_Movement(pVeh->m_OriginNodeID , pVeh->m_DepartureTime ,pVeh->m_DestinationNodeID, pVeh->m_PricingType , pVeh->m_VOT, PathLinkList, TotalCost,bDistanceFlag, bDebugFlag);

			bool bSwitchFlag = false;
			pVeh->m_bSwitched = false;

			if(iteration > 0) // update path assignments -> determine whether or not the vehicle will switch
			{
				float m_gap;
				float ExperiencedTravelTime = pVeh->m_TripTime;

				m_gap = ExperiencedTravelTime - TotalCost;

				if(m_gap<0)  // force negative gap values to zero, as experienced travel time can be smaller than the shortest path travel time for a (long) aggregation time period
					m_gap = 0;

				if(pVeh->m_VehicleID  == vehicle_id_trace)
			{
				TRACE("gap= %f = exp %f - shortest path %f\n",m_gap, ExperiencedTravelTime,TotalCost);

			}

				pVeh->SetMinCost(TotalCost);

				if(m_gap < 0) m_gap = 0.0;			

				g_CurrentGapValue += m_gap; // Jason : accumulate g_CurrentGapValue only when iteration >= 1

				float switching_rate = 1.0f/(iteration+1);   // default switching rate from MSA

				float RandomNumber= pVeh->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			

				if((pVeh->m_bComplete==false && pVeh->m_NodeSize >=2)) //for incomplete vehicles with feasible paths, need to switch at the next iteration
				{
					bSwitchFlag = true;
				}else
				{
					if(RandomNumber < switching_rate)  			
					{
						bSwitchFlag = true;
					}				
				} 
			}else	// iteration = 0;  at iteration 0, every vehicle needs a path for simulation, so every vehicle switches
			{
				bSwitchFlag = true;
			}

			if(bSwitchFlag)  
			{
				/// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle

				// Jason : accumulate number of vehicles switching paths
				g_CurrentNumOfVehiclesSwitched += 1; 

				pVeh->m_bSwitched = true;

				if( pVeh->m_aryVN !=NULL)
				{
					delete pVeh->m_aryVN;
				}

				pVeh->m_NodeSize = NodeSize;


				if(pVeh->m_NodeSize>=2)  // for feasible path
				{
					pVeh->m_bSwitched = true;

					if(NodeSize>=900)
					{
						cout << "PATH Size >900 " << NodeSize;
						g_ProgramStop();
					}

					pVeh->m_aryVN = new SVehicleLink[NodeSize];

					if(pVeh->m_aryVN==NULL)
					{
						cout << "Insufficient memory for allocating vehicle arrays!";
						g_ProgramStop();
					}

					pVeh->m_NodeNumberSum =0;
					pVeh->m_Distance =0;

					for(int i = 0; i< NodeSize-1; i++)
					{
						pVeh->m_aryVN[i].LinkID = PathLinkList[i];
						pVeh->m_NodeNumberSum += PathLinkList[i];

						if(pVeh->m_VehicleID  == vehicle_id_trace)
						{
							TRACE("Vehicle path: no. %d link: %d -> %d \n", i, 
								g_NodeVector[g_LinkVector[pVeh->m_aryVN[i].LinkID]->m_FromNodeID].m_NodeName,
								g_NodeVector[g_LinkVector[pVeh->m_aryVN[i].LinkID]->m_ToNodeID].m_NodeName);
						}

						/*if(g_LinkVector[pVeh->m_aryVN [i].LinkID]==NULL)
						{
						cout << "Error: g_LinkVector[pVeh->m_aryVN [i].LinkID]==NULL", pVeh->m_aryVN [i].LinkID;
						getchar();
						exit(0);
						}
						*/
						pVeh->m_Distance+= g_LinkVector[pVeh->m_aryVN [i].LinkID] ->m_Length ;
					}
					//cout << pVeh->m_VehicleID <<  " Distance" << pVeh->m_Distance <<  endl;;

				}else
				{
					if(iteration==0)
					{
						g_WarningFile  << "Warning: vehicle " <<  pVeh->m_VehicleID << " from  " << g_NodeVector[pVeh ->m_OriginNodeID].m_NodeName  << " to "  << g_NodeVector[pVeh ->m_DestinationNodeID].m_NodeName  << " does not have a physical path in the network. " << endl;
					TRACE("\nWarning: vehicle: %d from %d",g_NodeVector[pVeh ->m_OriginNodeID].m_NodeName , g_NodeVector[pVeh ->m_DestinationNodeID].m_NodeName);
					}

					pVeh->m_bLoaded  = false;
					pVeh->m_bComplete = false;
				}

			}

		} // for each vehicle on this OD pair

	}



void g_ODBasedDynamicTrafficAssignment()
{
	int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;

	int connector_count = 0;

	for (std::map<int, DTAZone>::iterator iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
	{
		connector_count += iterZone->second.m_CentroidNodeAry.size();  // only this origin zone has vehicles, then we build the network
	}

	int link_size  = g_LinkVector.size() + connector_count; // maximal number of links including connectors assuming all the nodes are destinations

	// assign different zones to different processors
	int nthreads = omp_get_max_threads ( );

	g_LogFile << "Number of iterations = " << g_NumberOfIterations << endl;

	// Jason
	int iteration = 0;
	bool NotConverged = true;
	int TotalNumOfVehiclesGenerated = 0;
	int number_of_threads = omp_get_num_threads();

	if(g_ParallelComputingMode == 0)
		number_of_threads = 1;


	cout<< "# of Computer Processors = "  << number_of_threads  << endl; 

	// ----------* start of outer loop *----------
	for(iteration=0; NotConverged && iteration <= g_NumberOfIterations; iteration++)  // we exit from the loop under two conditions (1) converged, (2) reach maximum number of iterations
	{
		cout << "------- Iteration = "<<  iteration << "--------" << endl;

		// initialize for each iteration
		g_CurrentGapValue = 0.0;
		g_CurrentNumOfVehiclesSwitched = 0;
		g_NewPathWithSwitchedVehicles = 0; 

		// initialize for OD estimation
		g_TotalDemandDeviation = 0;
		g_TotalMeasurementDeviation = 0; 

		if(!(g_VehicleLoadingMode == 1 && iteration == 0))  // we do not need to generate initial paths for vehicles for the first iteration of vehicle loading mode
		{
			g_EstimationLogFile << "----- Iteration = " << iteration << " ------" << endl; 

#pragma omp parallel for
			for(int ProcessID=0;  ProcessID < number_of_threads; ProcessID++)
			{
				// create network for shortest path calculation at this processor
				int	id = omp_get_thread_num( );  // starting from 0

				cout << "Processor " << id << " is working on shortest path calculation..  " << endl;

				DTANetworkForSP network_MP(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin); //  network instance for single processor in multi-thread environment
				//special notes: creating network with dynamic memory is a time-consumping task, so we create the network once for each processors

				for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
				{
					if((CurZoneID%number_of_threads) == ProcessID)  // if the remainder of a zone id (devided by the total number of processsors) equals to the processor id, then this zone id is 
					{
						if(g_ZoneMap[CurZoneID].m_OriginVehicleSize >0)  // only this origin zone has vehicles, then we build the network
						{
							// create network for shortest path calculation at this processor
							network_MP.BuildNetworkBasedOnZoneCentriod(CurZoneID);  // build network for this zone, because different zones have different connectors...

							if(g_ODZoneSize > 300)  // only for large networks
							{
							cout << "Processor " << id << " is calculating the shortest paths for zone " << CurZoneID << endl;
							}

							// scan all possible departure times
							for(int departure_time = g_DemandLoadingStartTimeInMin; departure_time < g_DemandLoadingEndTimeInMin; departure_time += g_AggregationTimetInterval)
							{
								if(g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray .size() > 0)
								{

							bool debug_flag = false;

							network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),departure_time,1,DEFAULT_VOT,false,debug_flag);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

									if(g_ODEstimationFlag && iteration>=g_ODEstimation_StartingIteration)  // perform path flow adjustment after at least 10 normal OD estimation
										network_MP.VehicleBasedPathAssignment_ODEstimation(CurZoneID,departure_time,departure_time+g_AggregationTimetInterval,iteration);
									else
										network_MP.VehicleBasedPathAssignment(CurZoneID,departure_time,departure_time+g_AggregationTimetInterval,iteration,debug_flag);
														
								}
							} // for each departure time

						} // for zone with volume
					} // for zone id assigned to the processor id
				}	// for each zone
			}  // for each processor
			//the OD estimation code below should be single thread

			if(g_ODEstimationFlag && iteration>=g_ODEstimation_StartingIteration)  // re-generate vehicles based on global path set
			{
				g_GenerateVehicleData_ODEstimation();
			}

			cout << "---- Network Loading for Iteration " << iteration <<"----" << endl;

			//	 DTANetworkForSP network(node_size, link_size, g_DemandLoadingHorizon);  // network instance for single-thread application

			NetworkLoadingOutput SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag,0,iteration);
			g_GenerateSimulationSummary(iteration,NotConverged, TotalNumOfVehiclesGenerated,SimuOutput);

		}	// end of outer loop

	} // for each assignment iteration

		cout << "Writing Vehicle Trajectory and MOE File... " << endl;

		if( iteration == g_NumberOfIterations)
		{ 
			iteration = g_NumberOfIterations -1;  //roll back to the last iteration if the ending condition is triggered by "iteration < g_NumberOfIterations"
		}

		g_OutputMOEData(iteration);

}


void DTANetworkForSP::VehicleBasedPathAssignment(int zone,int departure_time_begin, int departure_time_end, int iteration, bool debug_flag  = false)
		// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
{

		int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
		std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
		int NodeSize;
		int PredNode;
		int AssignmentInterval = int(departure_time_begin/g_AggregationTimetInterval);  // starting assignment interval

		int vehicle_id_trace  = -1;


		PathArrayForEachODT *PathArray;
		PathArray = new PathArrayForEachODT[g_ODZoneSize + 1]; // remember to release memory

		// loop through the TDOVehicleArray to assign or update vehicle paths...
		for (int vi = 0; vi < g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray.size(); vi++)
		{
			int VehicleID = g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray[vi];
			DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
			ASSERT(pVeh!=NULL);

			if(vehicle_id_trace == VehicleID)
			{
			debug_flag = true;
			}

			int OriginCentriod = m_PhysicalNodeSize;
			int DestinationCentriod = m_PhysicalNodeSize+ pVeh->m_DestinationZoneID ;

			float TotalCost = LabelCostAry[DestinationCentriod];
			if(TotalCost > MAX_SPLABEL-10)
			{
				ASSERT(false);
			}

			bool bSwitchFlag = false;
			pVeh->m_bSwitched = false;

			if(iteration > 0) // update path assignments -> determine whether or not the vehicle will switch
			{
				float m_gap;
				float ExperiencedTravelTime = pVeh->m_TripTime;

				if(g_VehicleExperiencedTimeGap == 1) // m_gap = vehicle experienced time - shortest path time (i.e., TotalCost)
				{
					m_gap = ExperiencedTravelTime - TotalCost;
				}
				else{ // m_gap = avg experienced path time - shortest path time

					int VehicleDest = pVeh->m_DestinationZoneID;
					int NodeSum = pVeh->m_NodeNumberSum;

					int PathIndex = 0;		
					for(int p=1; p<=PathArray[VehicleDest].NumOfPaths; p++)
					{
						if(NodeSum == PathArray[VehicleDest].PathNodeSums[p])
						{
							PathIndex = p;
							break;
						}
					}

					float AvgPathTime = PathArray[VehicleDest].AvgPathTimes[PathIndex];
					float MinTime = PathArray[VehicleDest].AvgPathTimes[PathArray[VehicleDest].BestPathIndex];
					// m_gap = AvgPathTime - MinTime; 
					m_gap = AvgPathTime - TotalCost; 
				}

				pVeh->SetMinCost(TotalCost);

				if(m_gap < 0) m_gap = 0.0;			

				g_CurrentGapValue += m_gap; // Jason : accumulate g_CurrentGapValue only when iteration >= 1

				float switching_rate;
				// switching_rate = 1.0f/(iteration+1);   // default switching rate from MSA

				// Jason
				switch (g_UEAssignmentMethod)
				{
				case 0: switching_rate = 1.0f/(iteration+1); // 0: MSA 
					break;
				case 1: switching_rate = float(g_LearningPercentage)/100.0f; // 1: day-to-day learning

					if(pVeh->m_TripTime > TotalCost + g_TravelTimeDifferenceForSwitching)
					{
						switching_rate = 1.0f;
					}

					break;
				case 2: switching_rate = m_gap / ExperiencedTravelTime; // 2: GAP-based switching rule for UE
					//case 2: switching_rate = (1.0f/(iteration+1)) * (m_gap / ExperiencedTravelTime); // 2: GAP-based switching rule for UE + Mixed Step-Size Scheme
					break;
				case 3: switching_rate = (1.0f/(iteration+1)) * (m_gap / ExperiencedTravelTime); // 3: Gap-based switching rule + MSA step size for UE
					break;
				default: switching_rate = 1.0f/(iteration+1); // default is MSA 
					break;
				}

				float RandomNumber= pVeh->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			

				if((pVeh->m_bComplete==false && pVeh->m_NodeSize >=2)) //for incomplete vehicles with feasible paths, need to switch at the next iteration
				{
					bSwitchFlag = true;
				}else
				{
					if(RandomNumber < switching_rate)  			
					{
						bSwitchFlag = true;
					}				
				} 
			}else	// iteration = 0;  at iteration 0, every vehicle needs a path for simulation, so every vehicle switches
			{
				bSwitchFlag = true;
			}

			if(bSwitchFlag)  
			{
				// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle
				NodeSize = 0;
				PredNode = NodePredAry[DestinationCentriod];		
				while(PredNode != OriginCentriod && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
				{
					ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
					temp_reversed_PathLinkList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
					PredNode = NodePredAry[PredNode];
				}

			// the first node in the shortest path is the super zone center, should not be counted

				int j = 0;
				int i;
				for( i = NodeSize-1; i>=0; i--)
				{
				PathNodeList[j++] = temp_reversed_PathLinkList[i];
				ASSERT(PathNodeList[j] < m_PhysicalNodeSize);
				}

			if(debug_flag)
			{
				for(i = 0; i < NodeSize; i++)
				{
				TRACE("\nNode sequence of vehicle path no.%d, node %d\n",i, g_NodeVector[PathNodeList[i]].m_NodeName);
				}
				TRACE("\nPath sequence end, cost = ..%f\n",TotalCost);
			}

				// Jason : accumulate number of vehicles switching paths
				g_CurrentNumOfVehiclesSwitched += 1; 

				pVeh->m_bSwitched = true;
				pVeh->m_NodeSize = NodeSize;

				if( pVeh->m_aryVN !=NULL)
				{
					delete pVeh->m_aryVN;
				}

				if(pVeh->m_NodeSize>=2)
				{
					pVeh->m_bSwitched = true;

					if(NodeSize>=900)
					{
						cout << "PATH Size >900 " << NodeSize;
						g_ProgramStop();
					}

					pVeh->m_aryVN = new SVehicleLink[NodeSize];

					if(pVeh->m_aryVN==NULL)
					{
						cout << "Insufficient memory for allocating vehicle arrays!";
						g_ProgramStop();
					}

					pVeh->m_NodeNumberSum =0;
					pVeh->m_Distance =0;

					for(int i = 0; i< NodeSize-1; i++) // NodeSize-1 is the number of links along the paths
					{
						pVeh->m_aryVN[i].LinkID = GetLinkNoByNodeIndex(PathNodeList[i], PathNodeList[i+1]);
						pVeh->m_NodeNumberSum += PathNodeList[i];
						pVeh->m_Distance+= g_LinkVector[pVeh->m_aryVN [i].LinkID] ->m_Length ;
					}
					//cout << pVeh->m_VehicleID <<  " Distance" << pVeh->m_Distance <<  endl;;

					// check whether or not this is a new path
					int PathIndex = 0;
					for(int p=1; p<=PathArray[pVeh->m_DestinationZoneID].NumOfPaths; p++)
					{
						if(pVeh->m_NodeNumberSum == PathArray[pVeh->m_DestinationZoneID].PathNodeSums[p])
						{
							PathIndex = p;
							break;
						}
					}
					if(PathIndex == 0) // a new path found
						g_NewPathWithSwitchedVehicles++;	

				}else
				{
					pVeh->m_bLoaded  = false;
					pVeh->m_bComplete = false;

					if(iteration==0)
					{
						g_WarningFile  << "Warning: vehicle " <<  pVeh->m_VehicleID << " from zone " << pVeh ->m_OriginZoneID << " to zone "  << pVeh ->m_DestinationZoneID << " does not have a physical path. Path Cost:" << TotalCost  << endl;
					}
				}
			} // if(bSwitchFlag)
		}

		// delete LeastExperiencedTimes; // Jason : release memory
		delete PathArray;
}



	void DTANetworkForSP::HistInfoVehicleBasedPathAssignment(int zone,int departure_time_begin, int departure_time_end)
		// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
	{

		int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
		std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
		int NodeSize;
		int PredNode;
		int DepartureTimeInterval = int(departure_time_begin/g_AggregationTimetInterval);  // starting assignment interval

		// loop through the TDOVehicleArray to assign or update vehicle paths... : iteration 0: for all vehicles
		for (int vi = 0; vi<g_TDOVehicleArray[zone][DepartureTimeInterval].VehicleArray.size(); vi++)
		{
			int VehicleID = g_TDOVehicleArray[zone][DepartureTimeInterval].VehicleArray[vi];

			{
				DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
				ASSERT(pVeh!=NULL);

				if(pVeh->m_NodeSize >0) // path assigned (from input_vehicle.csv)
					continue;

				BuildHistoricalInfoNetwork(zone, pVeh->m_DepartureTime , g_UserClassPerceptionErrorRatio[1]);  // build network for this zone, because different zones have different connectors...
				//using historical short-term travel time
				TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),pVeh->m_DepartureTime ,pVeh->m_DemandType,pVeh->m_VOT,false, false );  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

				int OriginCentriod = m_PhysicalNodeSize;  // as root node
				int DestinationCentriod =  m_PhysicalNodeSize+ pVeh->m_DestinationZoneID ;  

				float TotalCost = LabelCostAry[DestinationCentriod];
				if(TotalCost > MAX_SPLABEL-10)
				{
					ASSERT(false);
				}


				// starting from destination centriod
				NodeSize = 0;
				PredNode = NodePredAry[DestinationCentriod];  // PredNode is a physical node here.
				// stop when reaching origin centriod, back trace to origin zone as root node
				while(PredNode != OriginCentriod && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
				{
					ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
					PathNodeList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
					PredNode = NodePredAry[PredNode];
				}

				// PathNodeList stores all physical nodes

				pVeh->m_NodeSize = NodeSize;

				if( pVeh->m_aryVN !=NULL)
				{
					delete pVeh->m_aryVN;
				}

				if(pVeh->m_NodeSize>=2)
				{

					if(NodeSize>=900)
					{
						cout << "PATH Size >900 " << NodeSize  ;
						fprintf(g_ErrorFile, "Path for vehicle %d from zone %d to zone %d cannot be found. It might be due to connectivity issue.\n",VehicleID, zone, pVeh->m_DestinationZoneID);
						pVeh->m_NodeSize = 0;
						return;

					}

					pVeh->m_aryVN = new SVehicleLink[NodeSize];

					if(pVeh->m_aryVN==NULL)
					{
						cout << "Insufficient memory for allocating vehicle arrays!";
						g_ProgramStop();
					}

					pVeh->m_NodeNumberSum =0;
					pVeh->m_Distance =0;

					for(int i = 0; i< NodeSize-1; i++)
					{
						//					TRACE("ID:%d, %d \n",i, g_NodeVector[PathNodeList[i]]);
						pVeh->m_aryVN[i].LinkID = GetLinkNoByNodeIndex(PathNodeList[NodeSize-i-1], PathNodeList[NodeSize-i-2]);
						pVeh->m_NodeNumberSum +=PathNodeList[NodeSize-i-2];

						if(g_LinkVector[pVeh->m_aryVN [i].LinkID]==NULL)
						{
							cout << "Error: g_LinkVector[pVeh->m_aryVN [i].LinkID]==NULL", pVeh->m_aryVN [i].LinkID;
							getchar();
							exit(0);
						}

						pVeh->m_Distance+= g_LinkVector[pVeh->m_aryVN [i].LinkID] ->m_Length ;
					}
					//cout << pVeh->m_VehicleID <<  " Distance" << pVeh->m_Distance <<  endl;;
				}	

			}
		}

	}


	void g_ComputeFinalGapValue()
	{
		int node_size  = g_NodeVector.size() + 1 + g_ODZoneSize;
		int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

		g_CurrentGapValue = 0.0;
		g_CurrentNumOfVehiclesSwitched = 0;

		PathArrayForEachODT *PathArray;
		PathArray = new PathArrayForEachODT[g_ODZoneSize + 1]; // remember to release memory

		//#pragma omp parallel for
		for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
		{
			if(g_ZoneMap[CurZoneID].m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
			{
				// create network for shortest path calculation at this processor
				DTANetworkForSP network_MP(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin); //  network instance for single processor in multi-thread environment
				int id = omp_get_thread_num( );  // starting from 0
				network_MP.BuildNetworkBasedOnZoneCentriod(CurZoneID);  // build network for this zone, because different zones have different connectors...

				// scan all possible departure times
				for(int departure_time = g_DemandLoadingStartTimeInMin; departure_time < g_DemandLoadingEndTimeInMin; departure_time += g_AggregationTimetInterval)
				{
					if(g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray.size() > 0)
					{
						// loop through the TDOVehicleArray to obtain the least experienced trip time for each OD pair and each departure time interval
						/*
						float* LeastExperiencedTimes;
						LeastExperiencedTimes = new float[g_ODZoneSize + 1];				

						for(int DestZoneID=1; DestZoneID <= g_ODZoneSize; DestZoneID++)
						LeastExperiencedTimes[DestZoneID] = 1000000.0; // initialized with a big number

						for (int vi = 0; vi<g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray.size(); vi++)
						{
						int VehicleID = g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray[vi];
						DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
						ASSERT(pVeh!=NULL);

						int VehicleDestZoneID = pVeh->m_DestinationZoneID;
						if(pVeh->m_TripTime < LeastExperiencedTimes[VehicleDestZoneID])
						LeastExperiencedTimes[VehicleDestZoneID] = pVeh->m_TripTime;
						}					
						*/

						network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(), departure_time,1,DEFAULT_VOT,false,false);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

						int AssignmentInterval = departure_time/g_AggregationTimetInterval;

						if(g_VehicleExperiencedTimeGap == 0)
						{
							// loop through the TDOVehicleArray to obtain the avg experienced path time for each OD pair and each departure time interval
							ConstructPathArrayForEachODT(PathArray, CurZoneID, AssignmentInterval);
						}

						for (int vi = 0; vi < g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray.size(); vi++)
						{
							int VehicleID = g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray[vi];
							DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
							ASSERT(pVeh!=NULL);

							int OriginCentriod =  g_NodeVector.size();
							int DestinationCentriod = g_NodeVector.size() + pVeh->m_DestinationZoneID;

							float TotalCost = network_MP.LabelCostAry[DestinationCentriod];
							if(TotalCost > MAX_SPLABEL-10)
								ASSERT(false);

							float m_gap;
							float ExperiencedTravelTime = pVeh->m_TripTime;						
							pVeh->SetMinCost(TotalCost);

							if(g_VehicleExperiencedTimeGap == 1) 
							{
								m_gap = ExperiencedTravelTime - TotalCost;
							}
							else{
								int VehicleDest = pVeh->m_DestinationZoneID;
								int NodeSum = pVeh->m_NodeNumberSum;
								int PathIndex = 0;

								for(int p=1; p<=PathArray[VehicleDest].NumOfPaths; p++)
								{
									if(NodeSum == PathArray[VehicleDest].PathNodeSums[p])
									{
										PathIndex = p;
										break;
									}
								}

								float AvgPathTime = PathArray[VehicleDest].AvgPathTimes[PathIndex];
								float MinTime = PathArray[VehicleDest].AvgPathTimes[PathArray[VehicleDest].BestPathIndex];
								// m_gap = AvgPathTime - MinTime;
								m_gap = AvgPathTime - TotalCost;

							}						

							//float m_gap = ExperiencedTravelTime - LeastExperiencedTimes[pVeh->m_DestinationZoneID]; // Jason : use least experienced times instead of shortest path times

							if(m_gap < 0) m_gap = 0.0;

							g_CurrentGapValue += m_gap;
						}

						// delete LeastExperiencedTimes; // Jason : release memory

					}
				} // for each departure time interval
			}
		}
		delete PathArray;
	}

	void ConstructPathArrayForEachODT(PathArrayForEachODT PathArray[], int zone, int AssignmentInterval)
	{  // this function has been enhanced for path flow adjustment
		int DestZoneID; 
		for(DestZoneID=1; DestZoneID <= g_ODZoneSize; DestZoneID++) // initialization...
		{
			PathArray[DestZoneID].NumOfPaths = 0;
			PathArray[DestZoneID].NumOfVehicles = 0;
			PathArray[DestZoneID].DeviationNumOfVehicles = 0;
			PathArray[DestZoneID].BestPathIndex = 0;
			for(int p=0; p<100; p++)
			{
				PathArray[DestZoneID].NumOfVehsOnEachPath[p] = 0;
				PathArray[DestZoneID].PathNodeSums[p] = 0;
				PathArray[DestZoneID].AvgPathTimes[p] = 0.0;
				PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] = 0.0;
				PathArray[DestZoneID].PathSize[p] = 0;
				for(int q=0; q<100; q++)
					PathArray[DestZoneID].PathLinkSequences[p][q] = 0.0;
			}
		}

		// Scan all vehicles and construct path array for each destination
		for (int vi = 0; vi<g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray.size(); vi++)
		{
			int VehicleID = g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray[vi];
			DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
			ASSERT(pVeh!=NULL);

			int VehicleDest = pVeh->m_DestinationZoneID;
			float TripTime = pVeh->m_TripTime;
			int NodeSum = pVeh->m_NodeNumberSum;
			int NodeSize = pVeh->m_NodeSize;

			if(PathArray[VehicleDest].NumOfPaths == 0) // the first path for VehicleDest
			{
				PathArray[VehicleDest].NumOfPaths++;
				PathArray[VehicleDest].NumOfVehsOnEachPath[PathArray[VehicleDest].NumOfPaths]++;
				PathArray[VehicleDest].PathNodeSums[PathArray[VehicleDest].NumOfPaths] = NodeSum;
				PathArray[VehicleDest].AvgPathTimes[PathArray[VehicleDest].NumOfPaths] = TripTime;
				// obtain path link sequence from vehicle link sequence
				for(int i = 0; i< NodeSize-1; i++)
				{
					PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].NumOfPaths][i] = pVeh->m_aryVN[i].LinkID; 
				}
				PathArray[VehicleDest].PathSize[PathArray[VehicleDest].NumOfPaths] = NodeSize;
			}
			else{
				int PathIndex = 0;
				for(int p=1; p<=PathArray[VehicleDest].NumOfPaths; p++)
				{
					if(NodeSum == PathArray[VehicleDest].PathNodeSums[p])
					{
						PathIndex = p; // this veh uses the p-th in the set
						break;
					}
				}

				PathArray[VehicleDest].NumOfVehicles++;  // count the number of vehicles from o to d at assignment time interval tau

				if(PathIndex == 0) // a new path is found
				{
					PathArray[VehicleDest].NumOfPaths++;
					PathArray[VehicleDest].NumOfVehsOnEachPath[PathArray[VehicleDest].NumOfPaths]++;
					PathArray[VehicleDest].PathNodeSums[PathArray[VehicleDest].NumOfPaths] = NodeSum;
					PathArray[VehicleDest].AvgPathTimes[PathArray[VehicleDest].NumOfPaths] = TripTime;
					// obtain path link sequence from vehicle link sequence
					for(int i = 0; i< NodeSize-1; i++)
					{
						PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].NumOfPaths][i] = pVeh->m_aryVN[i].LinkID; 
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
		for(int DestZoneID=1; DestZoneID <= g_ODZoneSize; DestZoneID++)
		{
			if(PathArray[DestZoneID].NumOfPaths > 0)
			{
				int BestPath = 0;
				float BestTime = 999999.0;

				int p;
				for(p=1; p<=PathArray[DestZoneID].NumOfPaths; p++)
				{
					if(PathArray[DestZoneID].AvgPathTimes[p] < BestTime)
					{
						BestTime = PathArray[DestZoneID].AvgPathTimes[p];
						BestPath = p;
					}
				}
				PathArray[DestZoneID].BestPathIndex = BestPath;
				PathArray[DestZoneID].LeastTravelTime = BestTime;


				for(p=1; p<=PathArray[DestZoneID].NumOfPaths; p++)
				{

					PathArray[DestZoneID].AvgPathGap[p] =  PathArray[DestZoneID].AvgPathTimes[p] -  BestTime;
				}

			}
		}


	}



void g_AgentBasedShortestPathGeneration()
	{
    // find unique origin node
	// find unique destination node
	


	int node_size  = g_NodeVector.size();
	int link_size  = g_LinkVector.size();

	int line = 0;

	FILE* st_input = NULL;
	fopen_s(&st_input,"input_od_pairs.csv","r");
	if(st_input!=NULL)
	{
	char str[100];

	fscanf(st_input,"%[^\n]",str);  // read a line

	int origin_node_id, destination_node_id, record_id;

      while(!feof( st_input) )
	  {

	fscanf(st_input,"%d,%d,%d\n", &record_id, &origin_node_id, &destination_node_id);

		if(g_NodeNametoIDMap.find(origin_node_id)== g_NodeNametoIDMap.end())
				{
//				cout<< "origin_node_id "  << origin_node_id << " in input_od_pairs.csv has not be defined in input_node.csv.  Please check line =" << line  << endl; 
//				getchar();
//				exit(0);
					continue;
				}

				if(g_NodeNametoIDMap.find(destination_node_id)== g_NodeNametoIDMap.end())
				{
//				cout<< "destination_node_id "  << destination_node_id << " in input_od_pairs.csv has not be defined in input_node.csv. Please check line =" << line   << endl; 
//				getchar();
//				exit(0);
					continue;
				}

				int number_indx  = g_NodeNametoIDMap[origin_node_id];
				int dest_node_index = g_NodeNametoIDMap[destination_node_id];


				DTADestination element;
				element.destination_number = destination_node_id;
				element.record_id = record_id;
				element.destination_node_index = dest_node_index;
				
				g_NodeVector[number_indx].m_DestinationVector.push_back(element);
				g_NodeVector[number_indx].m_bOriginFlag  = true;

			if(line%10000==0)
				cout<<g_GetAppRunningTime() << " reading line "  << line/1000 << "k in input_od_pairs.csv."  << endl; 
				line++;
		}
	fclose(st_input);
	}




	unsigned int i;
	int UniqueOriginSize = 0;
	int UniqueDestinationSize = 0;
	int number_of_threads = omp_get_max_threads();

	if(g_ParallelComputingMode == 0)
		number_of_threads = 1;

	for(i=0; i< g_NodeVector.size(); i++)
	{
		if(g_NodeVector[i].m_bOriginFlag == true)
		{
			UniqueOriginSize +=1;
		}



		if(g_NodeVector[i].m_bDestinationFlag == true)
			UniqueDestinationSize +=1;

	}

	cout<< "# of OD pairs = "  << line << endl; 
	cout<< "# of unique origins = "  << UniqueOriginSize << " with " << line/UniqueOriginSize << " nodes per origin" << endl; 
	cout<< "# of processors = "  << number_of_threads  << endl; 

	g_LogFile<< "# of OD pairs = "  << line << endl; 
	g_LogFile<< "# of unique origins = "  << UniqueOriginSize << " with " << line/UniqueOriginSize << " nodes per origin" << endl; 
	g_LogFile <<  g_GetAppRunningTime() << "# of processors = "  << number_of_threads  << endl; 



	#pragma omp parallel for
	for(int ProcessID=0;  ProcessID < number_of_threads; ProcessID++)
	{
	// create network for shortest path calculation at this processor
			DTANetworkForSP network_MP(node_size, link_size, 1,g_AdjLinkSize); //  network instance for single processor in multi-thread environment
			int	cpu_id = omp_get_thread_num( );  // starting from 0
			network_MP.BuildPhysicalNetwork();  // build network for this zone, because different zones have different connectors...
			
			for(int node_index  = 0; node_index < node_size; node_index++)
			{
			if(node_index %number_of_threads == cpu_id && g_NodeVector[node_index].m_bOriginFlag)
				{
				if(node_index%100 ==cpu_id)
					{

						if(node_index%100 == 0)  // only one CUP can output to log file
						{
							cout << g_GetAppRunningTime()<<  "processor " << cpu_id << " working on node  "<<  node_index <<  ", "<< node_index*1.0f/node_size*100 << "%" <<  endl;
							g_LogFile << g_GetAppRunningTime()<<  "processor " << cpu_id << " working on node  "<<  node_index <<  ", "<< node_index*1.0f/node_size*100 << "%" <<  endl;
						}
					}

				network_MP.TDLabelCorrecting_DoubleQueue(node_index,0,1,DEFAULT_VOT,true,true);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

				for(int dest_no = 0; dest_no < g_NodeVector[node_index].m_DestinationVector.size(); dest_no++)
				{
					int dest_node_index =  g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_index;
					 g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_cost_label = network_MP.LabelCostAry[dest_node_index];
//					 TRACE("Label: %f: \n",g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_cost_label);

				}

				}
			}
	}
				

	FILE* st = NULL;
	fopen_s(&st,"output_shortest_path.txt","w");
	if(st!=NULL)
	{
	fprintf(st, "record_id,from_node_id,to_node_id,distance\n");
			for(int node_index  = 0; node_index < node_size; node_index++)
			{
				if(node_index %100000 ==0)
				cout <<g_GetAppRunningTime()<<  " Computation engine is outputing results for node sequence "<<  node_index << endl;
				for(int dest_no = 0; dest_no < g_NodeVector[node_index].m_DestinationVector.size(); dest_no++)
				{
					int dest_node_index =  g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_index;
					float label = g_NodeVector[node_index].m_DestinationVector[dest_no].destination_node_cost_label;

					fprintf(st, "%d, %d, %d, %4.4f\n", g_NodeVector[node_index].m_DestinationVector[dest_no].record_id, 
						g_NodeVector[node_index].m_NodeName,
						g_NodeVector[node_index].m_DestinationVector[dest_no].destination_number, label);

				}

			}
			
	fclose(st);
	}
	
//			cout <<g_GetAppRunningTime()<<  " Done!" << endl;

//			g_ProgramStop();
	}


	void g_GenerateSimulationSummary(int iteration, bool NotConverged, int TotalNumOfVehiclesGenerated, NetworkLoadingOutput SimuOutput)
{
	TotalNumOfVehiclesGenerated = SimuOutput.NumberofVehiclesGenerated; // need this to compute avg gap

	g_AssignmentMOEVector[iteration]  = SimuOutput;

		if(iteration <= 1) // compute relative gap after iteration 1
	{
		g_RelativeGap = 100; // 100%
	}else
	{
		g_RelativeGap = (fabs(g_CurrentGapValue - g_PrevGapValue) / g_PrevGapValue)*100;

		// comments: misleading statistics, especially when g_PrevGapValue is small. 
	}
	g_PrevGapValue = g_CurrentGapValue; // update g_PrevGapValue

	if(iteration >= 1) // Note: we output the gap for the last iteration, so "iteration-1"
	{
		SimuOutput.AvgUEGap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;

	}

	float PercentageComplete = 0;

	if(SimuOutput.NumberofVehiclesGenerated>0)
		PercentageComplete =  SimuOutput.NumberofVehiclesCompleteTrips*100.0f/SimuOutput.NumberofVehiclesGenerated;

	g_LogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Average Travel Time: " << SimuOutput.AvgTravelTime << ", Travel Time Index: " << SimuOutput.AvgTTI  << ", Average Distance: " << SimuOutput.AvgDistance << ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%"<<endl;
	cout << g_GetAppRunningTime() << "Iteration: " << iteration <<", Average Travel Time: " << SimuOutput.AvgTravelTime << ", Average Distance: " << SimuOutput.AvgDistance<< ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips << ", " << PercentageComplete << "%"<<endl;

	g_AssignmentLogFile << g_GetAppRunningTime() << "," << iteration << "," << SimuOutput.AvgTravelTime << "," << SimuOutput.AvgTTI  << "," << SimuOutput.AvgDistance  << "," << SimuOutput.SwitchPercentage <<"," <<  SimuOutput.NumberofVehiclesCompleteTrips<< "," << PercentageComplete << "%," ;
		
	g_AssignmentLogFile << SimuOutput.AvgUEGap   << ","	<< SimuOutput.TotalDemandDeviation << "," << SimuOutput.LinkVolumeAvgAbsError << "," << SimuOutput.LinkVolumeRootMeanSquaredError << ","<< SimuOutput.LinkVolumeAvgAbsPercentageError << endl;

	if(g_ODEstimationFlag == 1)
		cout << "Avg Gap: " << SimuOutput.AvgUEGap   << ", Demand Dev:"	<< SimuOutput.TotalDemandDeviation << ", Avg volume error: " << SimuOutput.LinkVolumeAvgAbsError << ", Avg % error: " << SimuOutput.LinkVolumeAvgAbsPercentageError << endl;



	SimuOutput.ResetStatistics ();   

	// with or without inner loop 
	if(g_NumberOfInnerIterations == 0) // without inner loop
	{											
		// check outer loop convergence (without inner loop)
		if(g_UEAssignmentMethod <= 1) // MSA and day-to-day learning
		{
			if(g_CurrentNumOfVehiclesSwitched < g_ConvergenceThreshold_in_Num_Switch)
				NotConverged = false; // converged!
		}else // gap-based approaches
		{	
			if(g_RelativeGap < g_ConvergencyRelativeGapThreshold_in_perc && !g_ODEstimationFlag )
				NotConverged = false; // converged! 
		}			
	}else // ----------* with inner loop *----------
	{			
		g_DynamicTraffcAssignmentWithinInnerLoop(iteration, NotConverged, TotalNumOfVehiclesGenerated);

	}	// end - with inner loop
}	

