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

// assignment module
// obtain simulation results, fetch shortest paths, assign vehicles to the shortest path according to gap function or MSA
#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"
#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <stdlib.h>  // Jason
#include <math.h>    // Jason
using namespace std;

std::vector<DTAPathData>   g_global_path_vector;	// global vector of path data
std::map<CString, int> g_path_index_map; 


extern ofstream g_WarningFile;

void ConstructPathArrayForEachODT(PathArrayForEachODT *, int, int); // construct path array for each ODT
void InnerLoopAssignment(int,int, int, int); // for inner loop assignment

void Assignment_MP(int id, int nthreads, int node_size, int link_size, int iteration)
{


}

void g_DynamicTrafficAssisnment()
{
	int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;
	int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

	// assign different zones to different processors
	int nthreads = omp_get_max_threads ( );

	bool bStartWithEmptyFile = true;
	bool bStartWithEmptyFile_for_assignment = true;

	g_LogFile << "Number of iterations = " << g_NumberOfIterations << endl;

	// Jason
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

		// initialize for OD estimation
		g_TotalDemandDeviation = 0;
		g_TotalMeasurementDeviation = 0; 


		if(!(g_VehicleLoadingMode == 1 && iteration == 0))  // we do not need to generate initial paths for vehicles for the first iteration of vehicle loading mode
		{
#pragma omp parallel for
			for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
			{
				if(g_ZoneVector[CurZoneID].m_OriginVehicleSize >0)  // only this origin zone has vehicles, then we build the network
				{
					// create network for shortest path calculation at this processor
					DTANetworkForSP network_MP(node_size, link_size, g_SimulationHorizon,g_AdjLinkSize); //  network instance for single processor in multi-thread environment
					int	id = omp_get_thread_num( );  // starting from 0

					cout <<g_GetAppRunningTime()<<  "processor " << id << " is working on assignment at zone  "<<  CurZoneID << endl;

					network_MP.BuildNetwork(CurZoneID);  // build network for this zone, because different zones have different connectors...

					// scan all possible departure times
					for(int departure_time = 0; departure_time < g_DemandLoadingHorizon; departure_time += g_DepartureTimetInterval)
					{
						if(g_TDOVehicleArray[CurZoneID][departure_time/g_DepartureTimetInterval].VehicleArray .size() > 0)
						{
							network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),departure_time,1);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo
							
							if(g_ODEstimationFlag && iteration>=g_ODEstimation_StartingIteration)  // perform path flow adjustment after at least 10 normal OD estimation
								network_MP.VehicleBasedPathAssignment_ODEstimation(CurZoneID,departure_time,departure_time+g_DepartureTimetInterval,iteration);
							else
								network_MP.VehicleBasedPathAssignment(CurZoneID,departure_time,departure_time+g_DepartureTimetInterval,iteration);
						}
					}



				}
			}
		}
		// below should be single thread

		if(g_ODEstimationFlag && iteration>=g_ODEstimation_StartingIteration)  // re-generate vehicles based on global path set
		{
		g_GenerateVehicleData_ODEstimation();
		}

		cout << "---- Network Loading for Iteration " << iteration <<"----" << endl;

		NetworkLoadingOutput SimuOutput;
		//	 DTANetworkForSP network(node_size, link_size, g_DemandLoadingHorizon);  // network instance for single-thread application

		SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag,0);

		TotalNumOfVehiclesGenerated = SimuOutput.NumberofVehiclesGenerated; // need this to compute avg gap

		g_AssignmentMOEAry[iteration]  = SimuOutput;

		float PercentageComplete = 0;

		if(SimuOutput.NumberofVehiclesGenerated>0)
			PercentageComplete =  SimuOutput.NumberofVehiclesCompleteTrips*100.0f/SimuOutput.NumberofVehiclesGenerated;

		g_LogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Average Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Average Distance: " << SimuOutput.AvgDistance << ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%"<<endl;
		cout << g_GetAppRunningTime() << "Iteration: " << iteration <<", Average Travel Time: " << SimuOutput.AvgTravelTime << ", Average Distance: " << SimuOutput.AvgDistance<< ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips << ", " << PercentageComplete << "%"<<endl;

		g_AssignmentLogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Ave Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Avg Distance: " << SimuOutput.AvgDistance  << ", Switch %:" << SimuOutput.SwitchPercentage <<", Num of Vehicles Completing Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%";			


		if(iteration <= 1) // compute relative gap after iteration 1
		{
			g_RelativeGap = 100; // 100%
		}else
		{
			g_RelativeGap = (fabs(g_CurrentGapValue - g_PrevGapValue) / g_PrevGapValue)*100;
		}
		g_PrevGapValue = g_CurrentGapValue; // update g_PrevGapValue

		if(iteration >= 1) // Note: we output the gap for the last iteration, so "iteration-1"
		{
			float avg_gap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;
			g_AssignmentLogFile << ", Num of Vehicles Switching Paths = " << g_CurrentNumOfVehiclesSwitched << ", Gap at prev iteration = " << g_CurrentGapValue << ", Avg Gap = " << avg_gap << ", Relative Gap at prev iteration = " << g_RelativeGap << "%"
				<< ", total demand deviation for all paths: " << g_TotalDemandDeviation << ", total measurement deviation for all paths " << g_TotalMeasurementDeviation  << endl;				
		}else
			g_AssignmentLogFile << endl;		

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
				if(g_RelativeGap < g_ConvergencyRelativeGapThreshold_in_perc)
					NotConverged = false; // converged! 
			}			
		}else // ----------* with inner loop *----------
		{			
			if(iteration == 0) // Note: iteration 0 in outer loop only assigns initial paths to vehicles (i.e., without updating path assignment)
			{
				// do something?
			}else
			{
				// check if any vehicles swittching to new paths
				if((g_NewPathWithSwitchedVehicles == 0)||(iteration > g_NumberOfIterations)) //  g_NewPathWithSwitchedVehicles is determined in VehicleBasedPathAssignment
				{
					NotConverged = false; // converged for "outer" loop! 
				}else // run inner loop only when there are vehicles swittching to new paths found by TDSP
				{
					bool NotConvergedInner = true;

					// ----------* enter inner loop *----------
					for(int inner_iteration = 1; NotConvergedInner && inner_iteration <= g_NumberOfInnerIterations; inner_iteration++)
					{					
						// initialize for each iteration
						g_CurrentGapValue = 0.0;
						g_CurrentNumOfVehiclesSwitched = 0;

						for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
						{
							if(g_ZoneVector[CurZoneID].m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
							{								
								// scan all possible departure times
								for(int departure_time = 0; departure_time < g_DemandLoadingHorizon; departure_time += g_DepartureTimetInterval)
								{
									if(g_TDOVehicleArray[CurZoneID][departure_time/g_DepartureTimetInterval].VehicleArray .size() > 0)
									{
										// update path assignments, g_CurrentGapValue, and g_CurrentNumOfVehiclesSwitched
										InnerLoopAssignment(CurZoneID, departure_time, departure_time+g_DepartureTimetInterval, inner_iteration);										
									}
								} // end - for each departure time interval
							}
						} // end - for each origin

						// evaluate new path assignments by simulation-based network loading
						cout << "---- Network Loading for Inner Loop Iteration " << inner_iteration <<"----" << endl;

						NetworkLoadingOutput SimuOutput;										
						SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag,0);

						TotalNumOfVehiclesGenerated = SimuOutput.NumberofVehiclesGenerated; // need this to compute avg gap

						g_AssignmentMOEAry[iteration] = SimuOutput;

						float PercentageComplete = 0;
						if(SimuOutput.NumberofVehiclesGenerated>0)
							PercentageComplete = SimuOutput.NumberofVehiclesCompleteTrips*100.0f/SimuOutput.NumberofVehiclesGenerated;

						g_LogFile << g_GetAppRunningTime() << "Inner Loop Iteration: " << inner_iteration << ", Average Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Average Distance: " << SimuOutput.AvgDistance << ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%"<<endl;
						cout << g_GetAppRunningTime() << "Inner Loop Iteration: " << inner_iteration <<", Average Travel Time: " << SimuOutput.AvgTravelTime << ", Average Distance: " << SimuOutput.AvgDistance<< ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips << ", " << PercentageComplete << "%"<<endl;

						g_AssignmentLogFile << g_GetAppRunningTime() << "Inner Loop Iteration: " << inner_iteration << ", Ave Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Avg Distance: " << SimuOutput.AvgDistance  << ", Switch %:" << SimuOutput.SwitchPercentage <<", Num of Vehicles Completing Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%";			

						// check inner loop convergence
						g_RelativeGap = (fabs(g_CurrentGapValue - g_PrevGapValue) / g_PrevGapValue)*100;

						if(g_UEAssignmentMethod <= 1) // MSA and day-to-day learning
						{
							if(g_CurrentNumOfVehiclesSwitched < g_ConvergenceThreshold_in_Num_Switch)
								NotConvergedInner = false; // converged!
						}else // gap-based approaches
						{	
							if(g_RelativeGap < g_ConvergencyRelativeGapThreshold_in_perc)
								NotConvergedInner = false; // converged! 
						}

						g_PrevGapValue = g_CurrentGapValue; // update g_PrevGapValue

						float avg_gap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;
						g_AssignmentLogFile << ", Num of Vehicles Switching Paths = " << g_CurrentNumOfVehiclesSwitched << ", Gap at prev iteration = " << g_CurrentGapValue << ", Avg Gap = " << avg_gap << ", Relative Gap at prev iteration = " << g_RelativeGap << "%" << endl;		

					} // end - enter inner loop
				} // end - check outer loop convergency				
			}
		}	// end - with inner loop

	}	// end of outer loop

	cout << "Writing Vehicle Trajectory and MOE File... " << endl;

	if( iteration == g_NumberOfIterations)
	{ 
		iteration = g_NumberOfIterations -1;  //roll back to the last iteration if the ending condition is triggered by "iteration < g_NumberOfIterations"
	}

	bStartWithEmptyFile = true;
	OutputVehicleTrajectoryData("Vehicle.csv", iteration,true);
	//output vehicle trajectory data
	OutputLinkMOEData("LinkMOE.csv", iteration,bStartWithEmptyFile);
	OutputNetworkMOEData("NetworkMOE.csv", iteration,bStartWithEmptyFile);


	if(g_NumberOfInnerIterations == 0) // without inner loop
	{
		g_ComputeFinalGapValue(); // Jason : compute and output final gap
		float avg_gap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;
		g_AssignmentLogFile << "Final Iteration: " << iteration << ", Gap = " << g_CurrentGapValue << ", Avg Gap = " << avg_gap << ", Relative Gap = " << g_RelativeGap << "%" << endl;
	}
}


void InnerLoopAssignment(int zone,int departure_time_begin, int departure_time_end, int inner_iteration) // this subroutine is called only when iteration > 0
{
	int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int NodeSize;
	int AssignmentInterval = int(departure_time_begin/g_DepartureTimetInterval);  // starting assignment interval

	PathArrayForEachODT *PathArray;
	PathArray = new PathArrayForEachODT[g_ODZoneSize + 1]; // remember to release memory

	ConstructPathArrayForEachODT(PathArray, zone, AssignmentInterval); 

	// loop through the TDOVehicleArray to assign or update vehicle paths...
	for (int vi = 0; vi<g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray .size(); vi++)
	{
		int VehicleID = g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray[vi];
		DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
		ASSERT(pVeh!=NULL);

		bool bSwitchFlag = false;
		pVeh->m_bSwitched = false;

		float m_gap;
		float ExperiencedTravelTime = pVeh->m_TripTime;	
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
		pVeh->SetMinCost(MinTime);

		if(g_VehicleExperiencedTimeGap == 1) 
			m_gap = ExperiencedTravelTime - MinTime;
		else
			m_gap = AvgPathTime - MinTime; 		

		if(m_gap < 0) m_gap = 0.0;			

		g_CurrentGapValue += m_gap; // Jason : accumulate g_CurrentGapValue only when iteration >= 1

		float switching_rate;
		// switching_rate = 1.0f/(iteration+1);   // default switching rate from MSA

		switch (g_UEAssignmentMethod)
		{
		case 0: switching_rate = 1.0f/(inner_iteration+1); // 0: MSA 
			break;
		case 1: switching_rate = float(g_LearningPercentage)/100.0f; // 1: day-to-day learning

			if(pVeh->m_TripTime > MinTime + g_TravelTimeDifferenceForSwitching)
			{
				switching_rate = 1.0f;
			}

			break;
		case 2: switching_rate = m_gap / ExperiencedTravelTime; // 2: GAP-based switching rule for UE
			break;
		case 3: switching_rate = (1.0f/(inner_iteration+1)) * (m_gap / ExperiencedTravelTime); // 3: Gap-based switching rule + MSA step size for UE
			break;
		default: switching_rate = 1.0f/(inner_iteration+1); // default is MSA 
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

		if(bSwitchFlag)  
		{
			// accumulate number of vehicles switching paths
			g_CurrentNumOfVehiclesSwitched += 1; 

			pVeh->m_bSwitched = true;

			// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle
			NodeSize = PathArray[VehicleDest].PathSize[PathArray[VehicleDest].BestPathIndex];			

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

				pVeh->m_NodeNumberSum = PathArray[VehicleDest].PathNodeSums[PathArray[VehicleDest].BestPathIndex];

				pVeh->m_Distance =0;

				for(int i = 0; i< NodeSize-1; i++)
				{
					pVeh->m_aryVN[i].LinkID = PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].BestPathIndex][i];					
					pVeh->m_Distance+= g_LinkVector[pVeh->m_aryVN[i].LinkID]->m_Length;
				}
				//cout << pVeh->m_VehicleID <<  " Distance" << pVeh->m_Distance <<  endl;;
			}else
			{
				pVeh->m_bLoaded  = false;
				pVeh->m_bComplete = false;

				if(inner_iteration==0)
				{
					//					g_WarningFile  << "Warning: vehicle " <<  pVeh->m_VehicleID << " from zone " << pVeh ->m_OriginZoneID << " to zone "  << pVeh ->m_DestinationZoneID << " does not have a physical path. Path Cost:" << TotalCost  << endl;
				}
			}
		} // if(bSwitchFlag)
	}

	delete PathArray;	
}

void DTANetworkForSP::VehicleBasedPathAssignment(int zone,int departure_time_begin, int departure_time_end, int iteration)
// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
{

	int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int NodeSize;
	int PredNode;
	int AssignmentInterval = int(departure_time_begin/g_DepartureTimetInterval);  // starting assignment interval

	// g_VehcileIDPerAssignmentIntervalMap is more like a cursor to record the last scanned position

	//PathArrayForEachODT PathArray[20]; // for test only
	PathArrayForEachODT *PathArray;
	PathArray = new PathArrayForEachODT[g_ODZoneSize + 1]; // remember to release memory

	if((iteration > 0) && (g_VehicleExperiencedTimeGap == 0)) // we do not need to update path assignments in iteration 0; only assign initial paths and perform network loading
	{
		// Jason : loop through the TDOVehicleArray to obtain the avg experienced path time for each OD pair and each departure time interval			 
		ConstructPathArrayForEachODT(PathArray, zone, AssignmentInterval);

		// Jason : loop through the TDOVehicleArray to obtain the least experienced trip time for each OD pair and each departure time interval
		/*
		float* LeastExperiencedTimes;
		LeastExperiencedTimes = new float[g_ODZoneSize + 1];

		for(int DestZoneID=1; DestZoneID <= g_ODZoneSize; DestZoneID++)
		LeastExperiencedTimes[DestZoneID] = 1000000.0; // initialized with a big number

		for (int vi = 0; vi<g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray.size(); vi++)
		{
		int VehicleID = g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray[vi];
		DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
		ASSERT(pVeh!=NULL);

		int VehicleDestZoneID = pVeh->m_DestinationZoneID;
		if(pVeh->m_TripTime < LeastExperiencedTimes[VehicleDestZoneID])
		LeastExperiencedTimes[VehicleDestZoneID] = pVeh->m_TripTime;
		}*/
	}

	// loop through the TDOVehicleArray to assign or update vehicle paths...
	for (int vi = 0; vi<g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray.size(); vi++)
	{
		int VehicleID = g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray[vi];
		DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
		ASSERT(pVeh!=NULL);

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
				PathNodeList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				PredNode = NodePredAry[PredNode];
				LabelCostAry[PredNode];  // Jason : ??? we do not use intermediate node cost here.
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

				for(int i = 0; i< NodeSize-1; i++)
				{
					pVeh->m_aryVN[i].LinkID = GetLinkNoByNodeIndex(PathNodeList[NodeSize-i-1], PathNodeList[NodeSize-i-2]);
					pVeh->m_NodeNumberSum += PathNodeList[NodeSize-i-2];

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
	int DepartureTimeInterval = int(departure_time_begin/g_DepartureTimetInterval);  // starting assignment interval

	// loop through the TDOVehicleArray to assign or update vehicle paths... : iteration 0: for all vehicles
		for (int vi = 0; vi<g_TDOVehicleArray[zone][DepartureTimeInterval].VehicleArray.size(); vi++)
		{
			int VehicleID = g_TDOVehicleArray[zone][DepartureTimeInterval].VehicleArray[vi];

			{
				DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
				ASSERT(pVeh!=NULL);

				BuildHistoricalInfoNetwork(zone, pVeh->m_DepartureTime , g_UserClassPerceptionErrorRatio[1]);  // build network for this zone, because different zones have different connectors...
											//using historical short-term travel time
				TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),pVeh->m_DepartureTime ,pVeh->m_VehicleType );  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

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
		if(g_ZoneVector[CurZoneID].m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
		{
			// create network for shortest path calculation at this processor
			DTANetworkForSP network_MP(node_size, link_size, g_SimulationHorizon,g_AdjLinkSize); //  network instance for single processor in multi-thread environment
			int id = omp_get_thread_num( );  // starting from 0
			network_MP.BuildNetwork(CurZoneID);  // build network for this zone, because different zones have different connectors...

			// scan all possible departure times
			for(int departure_time = 0; departure_time < g_DemandLoadingHorizon; departure_time += g_DepartureTimetInterval)
			{
				if(g_TDOVehicleArray[CurZoneID][departure_time/g_DepartureTimetInterval].VehicleArray.size() > 0)
				{
					// loop through the TDOVehicleArray to obtain the least experienced trip time for each OD pair and each departure time interval
					/*
					float* LeastExperiencedTimes;
					LeastExperiencedTimes = new float[g_ODZoneSize + 1];				

					for(int DestZoneID=1; DestZoneID <= g_ODZoneSize; DestZoneID++)
					LeastExperiencedTimes[DestZoneID] = 1000000.0; // initialized with a big number

					for (int vi = 0; vi<g_TDOVehicleArray[CurZoneID][departure_time/g_DepartureTimetInterval].VehicleArray.size(); vi++)
					{
					int VehicleID = g_TDOVehicleArray[CurZoneID][departure_time/g_DepartureTimetInterval].VehicleArray[vi];
					DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
					ASSERT(pVeh!=NULL);

					int VehicleDestZoneID = pVeh->m_DestinationZoneID;
					if(pVeh->m_TripTime < LeastExperiencedTimes[VehicleDestZoneID])
					LeastExperiencedTimes[VehicleDestZoneID] = pVeh->m_TripTime;
					}					
					*/

					network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(), departure_time,1);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

					int AssignmentInterval = departure_time/g_DepartureTimetInterval;

					if(g_VehicleExperiencedTimeGap == 0)
					{
						// loop through the TDOVehicleArray to obtain the avg experienced path time for each OD pair and each departure time interval
						ConstructPathArrayForEachODT(PathArray, CurZoneID, AssignmentInterval);
					}

					for (int vi = 0; vi < g_TDOVehicleArray[CurZoneID][departure_time/g_DepartureTimetInterval].VehicleArray.size(); vi++)
					{
						int VehicleID = g_TDOVehicleArray[CurZoneID][departure_time/g_DepartureTimetInterval].VehicleArray[vi];
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

void g_MultiDayTrafficAssisnment()
{
	int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;
	int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

	g_LogFile << "Number of iterations = " << g_NumberOfIterations << endl;

	int iteration = 0;
	bool NotConverged = true;
	int TotalNumOfVehiclesGenerated = 0;

	

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		g_LinkVector[li]->InitializeDayDependentCapacity();
	}

	for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
	{

		float RandomNumber= (*vIte)->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			
		if(RandomNumber < 0.05)  // percentage of PI users
			(*vIte)->m_bETTFlag = false;
		else
			(*vIte)->m_bETTFlag = true;
	}
	// 1st loop for each iteration
	for(iteration=0; NotConverged && iteration <= g_NumberOfIterations; iteration++)  // we exit from the loop under two conditions (1) converged, (2) reach maximum number of iterations
	{

		cout << "------- Iteration = "<<  iteration << "--------" << endl;

		// 2nd loop for each vehicle


		for(int day = 0; day < MAX_DAY_SIZE; day++)  // for VI users, we use iteration per day first then per vehicle, as vehicles share the same information per day
		{


#pragma omp parallel for
			for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
			{
				if(g_ZoneVector[CurZoneID].m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
				{
					// create network for shortest path calculation at this processor
					DTANetworkForSP network_MP(node_size, link_size, g_SimulationHorizon,g_AdjLinkSize); //  network instance for single processor in multi-thread environment
					int	id = omp_get_thread_num( );  // starting from 0

					cout <<g_GetAppRunningTime()<<  "processor " << id << " is working on day " << day  << " PI assignment at zone  "<<  CurZoneID << endl;

					network_MP.BuildNetwork(CurZoneID);  // build network for this zone, because different zones have different connectors...

					for(unsigned li = 0; li< g_LinkVector.size(); li++)
					{
						float TravelTime;
						if(iteration == 0)
						{
							TravelTime = g_LinkVector[li]->m_FreeFlowTravelTime ;
						}
						else
						{
							TravelTime = g_LinkVector[li]->m_DayDependentTravelTime[day];
						}
						network_MP.m_LinkTDTimeAry[g_LinkVector[li]->m_LinkID][0] = TravelTime;
						network_MP.m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][0]=  TravelTime;
						// use travel time now, should use cost later
					}


					network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),0,1);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

					for (int vi = 0; vi<g_TDOVehicleArray[CurZoneID][0].VehicleArray.size(); vi++)
					{

						int VehicleID = g_TDOVehicleArray[CurZoneID][0].VehicleArray[vi];
						DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
						ASSERT(pVeh!=NULL);

						if(pVeh->m_bETTFlag == false)  // VI users
						{

							// create network for shortest path calculation at this processor

							bool bSwitchFlag = false;


							int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
							// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle
							int NodeSize = 0;
							int OriginCentriod = network_MP.m_PhysicalNodeSize;

							int DestinationCentriod = network_MP.m_PhysicalNodeSize+ pVeh->m_DestinationZoneID ;

							if( iteration >=1)
							{

								pVeh->m_DayDependentGap[day]= pVeh->m_DayDependentTripTime[day] - network_MP.LabelCostAry [DestinationCentriod];
							}else
							{
								pVeh->m_DayDependentGap[day] = 0;
							}

							float switching_rate = 1.0f/(iteration+1); // 0: MSA 

							if( iteration >=1)
							{
								switching_rate = (1.0f/(iteration+1)) * (pVeh->m_DayDependentGap[day] / pVeh->m_DayDependentTripTime[day]); // 3: Gap-based switching rule + MSA step size for UE
							}

							float RandomNumber= pVeh->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			

							if(RandomNumber < switching_rate || iteration==0)  			
							{
								bSwitchFlag = true;
							}				

							if(bSwitchFlag)  
							{
								int PredNode = network_MP.NodePredAry[DestinationCentriod];		
								while(PredNode != OriginCentriod && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
								{
									ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
									PathNodeList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
									PredNode = network_MP.NodePredAry[PredNode];
								}

								for(int i = 0; i< NodeSize-1; i++)
								{
									int LinkNo = network_MP.GetLinkNoByNodeIndex(PathNodeList[NodeSize-i-1], PathNodeList[NodeSize-i-2]);
									pVeh->m_DayDependentAryLink[day*MAX_PATH_LINK_SIZE+i]= LinkNo;
									pVeh->m_DayDependentNodeNumberSum[day] +=PathNodeList[i];

								}
								pVeh->m_DayDependentLinkSize[day] = NodeSize-1;
							}
						}
					}

				}
			}
		}

#pragma omp parallel for
		for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
		{
			if(g_ZoneVector[CurZoneID].m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
			{
				// create network for shortest path calculation at this processor
				DTANetworkForSP network_MP(node_size, link_size, g_SimulationHorizon,g_AdjLinkSize); //  network instance for single processor in multi-thread environment
				int	id = omp_get_thread_num( );  // starting from 0

				cout <<g_GetAppRunningTime()<<  "processor " << id << " is working on ETT assignment at zone  "<<  CurZoneID << endl;

				network_MP.BuildNetwork(CurZoneID);  // build network for this zone, because different zones have different connectors...

				// 4th loop for each link cost
				for(unsigned li = 0; li< g_LinkVector.size(); li++)
				{
					float TravelTime;

					if(iteration == 0)
					{
						TravelTime = g_LinkVector[li]->m_FreeFlowTravelTime ;
					}
					else
					{
						TravelTime = g_LinkVector[li]->m_AverageTravelTime;
					}
					network_MP.m_LinkTDTimeAry[g_LinkVector[li]->m_LinkID][0] = TravelTime;
					network_MP.m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][0]=  TravelTime;
					// use travel time now, should use cost later
				}

				network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),0,1);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo


				for (int vi = 0; vi<g_TDOVehicleArray[CurZoneID][0].VehicleArray.size(); vi++)
				{

					int VehicleID = g_TDOVehicleArray[CurZoneID][0].VehicleArray[vi];
					DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
					ASSERT(pVeh!=NULL);

					if(pVeh->m_bETTFlag == true)  // ETT users
					{

						int OriginCentriod = network_MP.m_PhysicalNodeSize;
						int DestinationCentriod = network_MP.m_PhysicalNodeSize+ pVeh->m_DestinationZoneID ;

						for(int day = 0; day < MAX_DAY_SIZE; day++)
						{
							if( iteration >=1)
							{
								pVeh->m_DayDependentGap[day]= pVeh->m_AvgDayTravelTime - network_MP.LabelCostAry [DestinationCentriod];

								if(pVeh->m_DayDependentGap[day] < -0.1)
								{
									g_AssignmentLogFile <<"iteration " << iteration << " gap < 0: " << pVeh->m_VehicleID << " " << pVeh->m_AvgDayTravelTime << " " << network_MP.LabelCostAry [DestinationCentriod] <<endl;
								}
							}else
							{
								pVeh->m_DayDependentGap[day] = 0;
							}

						}
						bool bSwitchFlag = false;

						float switching_rate = 1.0f/(iteration+1); // 0: MSA 
						if( iteration >=1)
						{
							switching_rate = (1.0f/(iteration+1)) * (pVeh->m_AvgDayTravelTime - network_MP.LabelCostAry [DestinationCentriod]) / pVeh->m_AvgDayTravelTime; // 3: Gap-based switching rule + MSA step size for UE
						}
						float RandomNumber= pVeh->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			


						if(RandomNumber < switching_rate || iteration==0)  			
						{
							bSwitchFlag = true;
						}				

						// shortest path once, apply for all days
						int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
						// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle
						int NodeSize = 0;


						if(bSwitchFlag)  
						{
							int PredNode = network_MP.NodePredAry[DestinationCentriod];		
							while(PredNode != OriginCentriod && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
							{
								ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
								PathNodeList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
								PredNode = network_MP.NodePredAry[PredNode];
							}
							for(int i = 0; i< NodeSize-1; i++)
							{
								int LinkNo = network_MP.GetLinkNoByNodeIndex(PathNodeList[NodeSize-i-1], PathNodeList[NodeSize-i-2]);
								pVeh->m_DayDependentAryLink[0*MAX_PATH_LINK_SIZE+i]= LinkNo;
								pVeh->m_DayDependentNodeNumberSum[0] +=PathNodeList[i];
							}

							for(int day = 0; day < MAX_DAY_SIZE; day++)
							{
								pVeh->m_DayDependentLinkSize[day] = NodeSize-1;
							}
						}

					}
				}
			}
		}


		// below should be single thread

		cout << "---- Network Loading for Iteration " << iteration <<"----" << endl;


		float DayByDayTravelTimeSumETT = 0;
		float DayByDayTravelTimeSumVI = 0;
		float PI_gap_sum = 0;
		float ETT_gap_sum = 0;
		float total_gap = 0;
		int VehicleCountVI = 0;
		float VehicleCountETT = 0;
		float TTSTDSumVI = 0;
		float TTSTDSumETT = 0;


		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			g_LinkVector[li]->m_AverageTravelTime = 0;
		}

		for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
		{
			(*vIte)->m_AvgDayTravelTime = 0;
		}

		for(int day = 0; day < MAX_DAY_SIZE; day++)
		{
			cout << "day:"<< day << "....."  << endl;

			for(unsigned li = 0; li< g_LinkVector.size(); li++)
			{

				g_LinkVector[li]->m_BPRLinkVolume = 0;
			}


			// switch data for the following line
			for(unsigned li = 0; li< g_LinkVector.size(); li++)
			{
				g_LinkVector[li]->m_BPRLaneCapacity = g_LinkVector[li]->m_DayDependentCapacity[day];
			}

			// fetch day-dependent vehicle path

			for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
			{
				for(int i = 0; i< (*vIte)->m_DayDependentLinkSize[day]; i++)
				{
					int LinkID;

					if((*vIte)->m_bETTFlag == true)
						LinkID= (*vIte)->m_DayDependentAryLink[0*MAX_PATH_LINK_SIZE+i];
					else
						LinkID= (*vIte)->m_DayDependentAryLink[day*MAX_PATH_LINK_SIZE+i];

					g_LinkVector[LinkID]->m_BPRLinkVolume++;
				}


			}


			// BPR based loading
			for(unsigned li = 0; li< g_LinkVector.size(); li++)
			{
				g_LinkVector[li]->m_BPRLinkTravelTime = g_LinkVector[li]->m_FreeFlowTravelTime*(1.0f+0.15f*(powf(g_LinkVector[li]->m_BPRLinkVolume/(g_LinkVector[li]->m_BPRLaneCapacity*g_LinkVector[li]->GetNumLanes()),4.0f)));
				//				g_LogFile << "day:"<< day << ", BPR:"<< g_NodeVector[g_LinkVector[li]->m_FromNodeID] << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID]<<" Flow:" << g_LinkVector[li]->m_BPRLinkVolume << "travel time:" << g_LinkVector[li]->m_BPRLinkTravelTime  << endl;
				//				cout << "day:"<< day << ", BPR:"<< g_NodeVector[g_LinkVector[li]->m_FromNodeID] << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID]<<" Flow:" << g_LinkVector[li]->m_BPRLinkVolume << "travel time:" << g_LinkVector[li]->m_BPRLinkTravelTime  << endl;
				g_LinkVector[li]->m_DayDependentTravelTime[day] = g_LinkVector[li]->m_BPRLinkTravelTime;
				g_LinkVector[li]->m_AverageTravelTime+=  g_LinkVector[li]->m_BPRLinkTravelTime/MAX_DAY_SIZE;
			}

			VehicleCountETT = 0;
			float TravelTimeSumETT = 0;

			VehicleCountVI = 0;
			float TravelTimeSumVI = 0;


			for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
			{
				(*vIte)->m_DayDependentTripTime[day] = 0;

				if((*vIte)->m_bETTFlag == true)
				{
					for(int i = 0; i< (*vIte)->m_DayDependentLinkSize[0]; i++)
					{
						int LinkID = (*vIte)->m_DayDependentAryLink[0*MAX_PATH_LINK_SIZE+i];
						(*vIte)->m_DayDependentTripTime[day] += g_LinkVector[LinkID]->m_BPRLinkTravelTime;
					}

					(*vIte)->m_AvgDayTravelTime += ((*vIte)->m_DayDependentTripTime[day]/MAX_DAY_SIZE);

					TravelTimeSumETT+= (*vIte)->m_DayDependentTripTime[day];
					ETT_gap_sum += (*vIte)->m_DayDependentGap [day];
					total_gap +=(*vIte)->m_DayDependentGap [day];
					VehicleCountETT++;
				}else
				{
					for(int i = 0; i< (*vIte)->m_DayDependentLinkSize[day]; i++)
					{
						int LinkID = (*vIte)->m_DayDependentAryLink[day*MAX_PATH_LINK_SIZE+i];
						(*vIte)->m_DayDependentTripTime[day] += g_LinkVector[LinkID]->m_BPRLinkTravelTime;
					}

					(*vIte)->m_AvgDayTravelTime += ((*vIte)->m_DayDependentTripTime[day]/MAX_DAY_SIZE);


					TravelTimeSumVI+= (*vIte)->m_DayDependentTripTime[day];
					PI_gap_sum += (*vIte)->m_DayDependentGap [day];
					total_gap +=(*vIte)->m_DayDependentGap [day];
					VehicleCountVI++;
				}

			}



			float AverageTravelTimeETT = TravelTimeSumETT/max(VehicleCountETT,1);
			float AverageTravelTimeVI = TravelTimeSumVI/max(VehicleCountVI,1);


			DayByDayTravelTimeSumETT += AverageTravelTimeETT;
			DayByDayTravelTimeSumVI += AverageTravelTimeVI;


			//			g_AssignmentLogFile << "Iteration: " << iteration << ", Day: " << day <<  ", Ave ETT Travel Time: " << AverageTravelTimeETT << ", Ave VI Travel Time: " << AverageTravelTimeVI << ", # of VI: " << VehicleCountVI  <<   endl;
			cout << g_GetAppRunningTime() << "Iteration: " << iteration << ", Day: " << day <<  ", Ave ETT Travel Time: " << AverageTravelTimeETT << ", Ave VI Travel Time: " << AverageTravelTimeVI << ", # of VI: " << VehicleCountVI  <<  endl;
		}

		for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
		{
			float total_var = 0;
			for(int day = 0; day < MAX_DAY_SIZE; day++)
			{
				total_var += pow((*vIte)->m_DayDependentTripTime[day] - (*vIte)->m_AvgDayTravelTime, 2);
			}
			(*vIte)->m_DayTravelTimeSTD = sqrt(total_var/max(1,(MAX_DAY_SIZE-1)));

			if((*vIte)->m_bETTFlag == true)
			{
				TTSTDSumETT +=(*vIte)->m_DayTravelTimeSTD;
			}else
			{
				TTSTDSumVI +=(*vIte)->m_DayTravelTimeSTD;
			}
		}

		float AverageTTSTDETT = TTSTDSumETT/max(VehicleCountETT,1);
		float AverageTTSTDVI = TTSTDSumVI/max(VehicleCountVI,1);


		float Average_PI_Gap = PI_gap_sum/MAX_DAY_SIZE /max(VehicleCountVI,1);
		float Average_ETT_Gap = ETT_gap_sum/MAX_DAY_SIZE /max(VehicleCountETT,1);
		float Avg_gap = total_gap/MAX_DAY_SIZE/(VehicleCountETT+VehicleCountVI);
		g_AssignmentLogFile << "--D2D Summary: Iteration: " << iteration << ", Ave D2D ETT Travel Time: " << DayByDayTravelTimeSumETT/MAX_DAY_SIZE << ", Ave D2D VI Travel Time: " << DayByDayTravelTimeSumVI/MAX_DAY_SIZE << ", Ave ETT Gap: " << Average_ETT_Gap << ", Ave VI Gap: " << Average_PI_Gap << ", Ave Gap: " << Avg_gap << " TTSTDETT: "<< AverageTTSTDETT << ",TTSTDVI: "<< AverageTTSTDVI << endl;

		// we should output gap here
		//		g_AssignmentLogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Ave Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Avg Distance: " << SimuOutput.AvgDistance  << ", Switch %:" << SimuOutput.SwitchPercentage <<", Num of Vehicles Completing Trips: ";

		//output vehicle trajectory data
		//	OutputLinkMOEData("LinkMOE.csv", iteration,bStartWithEmptyFile);
		//	OutputNetworkMOEData("NetworkMOE.csv", iteration,bStartWithEmptyFile);

	}

//	OutputMultipleDaysVehicleTrajectoryData("MultiDayVehicle.csv");


}
void OutputMultipleDaysVehicleTrajectoryData(char fname[_MAX_PATH])
{
	FILE* st = NULL;

	fopen_s(&st,fname,"w");

	if(st!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;
		int VehicleCount_withPhysicalPath = 0;

		fprintf(st, "vehicle_id,  originput_zone_id, destination_zone_id, departure_time, arrival_time, complete_flag, trip_time, vehicle_type, occupancy, information_type, value_of_time, path_min_cost,distance_in_mile, number_of_nodes,  node id, node arrival time\n");


		for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
		{

			if((*vIte)->m_DayDependentLinkSize[0] >= 1)  // with physical path in the network
			{

				int UpstreamNodeID = 0;
				int DownstreamNodeID = 0;

				float TripTime = 0;

				if((*vIte)->m_bComplete)
					TripTime = (*vIte)->m_ArrivalTime-(*vIte)->m_DepartureTime;

				float m_gap = 0;
				fprintf(st,"%d,e%d, o%d,d%d,%4.2f,%4.2f,c%d,%4.2f,%d,%d,%d,%d,%4.2f,%4.2f,%d",
					(*vIte)->m_VehicleID ,(*vIte)->m_bETTFlag, (*vIte)->m_OriginZoneID , (*vIte)->m_DestinationZoneID,
					(*vIte)->m_DepartureTime, (*vIte)->m_ArrivalTime , (*vIte)->m_bComplete, TripTime,
					(*vIte)->m_VehicleType ,(*vIte)->m_Occupancy,(*vIte)->m_InformationClass, (*vIte)->GetVOT() , (*vIte)->GetMinCost(),(*vIte)->m_Distance, (*vIte)->m_NodeSize);

				fprintf(st, "\n AVG %5.3f, STD %5.3f,",(*vIte)->m_AvgDayTravelTime , (*vIte)->m_DayTravelTimeSTD) ;

				for(int day=0; day < MAX_DAY_SIZE; day++)
				{
					fprintf(st, "day %d,%5.2f,",day,(*vIte)->m_DayDependentTripTime[day]) ;

					for(int i = 0; i< (*vIte)->m_DayDependentLinkSize[day]; i++)
					{

						int LinkID = (*vIte)->m_DayDependentAryLink[day*MAX_PATH_LINK_SIZE+i];
						int NodeID = g_LinkVector[LinkID]->m_ToNodeID;
						int NodeName = g_NodeVector[NodeID].m_NodeName;

						fprintf(st, "%d",NodeName) ;
					}

					fprintf(st,"\n");

				}

			}
		}
		fclose(st);
	}
}


void g_OneShotNetworkLoading()
{
	int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;
	int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

	// assign different zones to different processors
	int nthreads = omp_get_max_threads ( );

	/*
	// when debugging use single processor only
	cout << "  Number of processors available = " << omp_get_num_procs ( ) << "\n";
	cout << "  Number of threads =              " << omp_get_max_threads ( ) << "\n";

	g_RealTimeShortestPathCalculation(0);

	CTime EndTime = CTime::GetCurrentTime();
	CTimeSpan ts = EndTime  - StartTime;

	cout << "  real-time info shortest path calculation takes " << ts.GetTotalSeconds()<< " seconds\n";
	cout << "   real-time info shortest path calculation for a node takes " << ts.GetTotalSeconds()*1.0 / node_size << " seconds\n";
	*/
	//	nthreads = 1;
	bool bStartWithEmptyFile = true;
	bool bStartWithEmptyFile_for_assignment = true;

	g_LogFile << "Number of Iterations = " << g_NumberOfIterations << endl;

	int iteration = 0;
	bool NotConverged = true;
	int TotalNumOfVehiclesGenerated = 0;
	std::set<DTALink*>::iterator iterLink;

			// BPR based loading
			for(unsigned li = 0; li< g_LinkVector.size(); li++)
			{
				g_LinkVector[li]->m_BPRLinkTravelTime = g_LinkVector[li]->m_FreeFlowTravelTime*(1.0f+0.15f*(powf(g_LinkVector[li]->m_BPRLinkVolume/(g_LinkVector[li]->m_BPRLaneCapacity*g_LinkVector[li]->GetNumLanes()),4.0f)));
			}
	NetworkLoadingOutput SimuOutput;
	//	 DTANetworkForSP network(node_size, link_size, g_DemandLoadingHorizon);  // network instance for single-thread application
	int simulation_mode = 1; // simulation from demand
	SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag, simulation_mode);
	bStartWithEmptyFile = true;
	OutputVehicleTrajectoryData("Vehicle.csv", iteration,true);

	//output vehicle trajectory data
 	OutputLinkMOEData("LinkMOE.csv", iteration,bStartWithEmptyFile);
	OutputNetworkMOEData("NetworkMOE.csv", iteration,bStartWithEmptyFile);

}

int g_OutputSimulationSummary(float& AvgTravelTime, float& AvgDistance, float& AvgSpeed,
							  int InformationClass=-1, int VehicleType=-1,int DepartureTimeInterval = -1)
{
	float TravelTimeSum = 0;
	float DistanceSum = 0;
	int VehicleCount = 0;

	for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end();v++)
	{

		if((*v)->m_bComplete )  // vehicle completes the trips
		{
			if( ((*v)->m_InformationClass == InformationClass ||InformationClass==-1)
				&& ((*v)->m_VehicleType == VehicleType ||VehicleType==-1)
				   && ((int)((*v)->m_DepartureTime/ g_DepartureTimetInterval)== (int)(DepartureTimeInterval/g_DepartureTimetInterval) ||DepartureTimeInterval==-1))
			{
			TravelTimeSum += (*v)->m_TripTime;

			DistanceSum+= (*v)->m_Distance;

			VehicleCount++;
			
			}
		}
	}

	AvgTravelTime = TravelTimeSum/max(1,VehicleCount);  // unit: min
	AvgDistance = DistanceSum/max(1,VehicleCount);  // unit: mile

	AvgSpeed = AvgDistance/max(1,AvgTravelTime)*60;  // unit: mph
	return VehicleCount;
}


void g_StaticTrafficAssisnment()
{
	int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;
	int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

	// assign different zones to different processors
	int nthreads = omp_get_max_threads ( );

	bool bStartWithEmptyFile = true;
	bool bStartWithEmptyFile_for_assignment = true;

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
			for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
			{
				if(g_ZoneVector[CurZoneID].m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
				{
					// create network for shortest path calculation at this processor
					DTANetworkForSP network_MP(node_size, link_size, g_SimulationHorizon,g_AdjLinkSize); //  network instance for single processor in multi-thread environment
					int	id = omp_get_thread_num( );  // starting from 0

					cout <<g_GetAppRunningTime()<<  "processor " << id << " is working on assignment at zone  "<<  CurZoneID << endl;

					network_MP.BuildNetwork(CurZoneID);  // build network for this zone, because different zones have different connectors...

					// scan all possible departure times
					for(int departure_time = 0; departure_time < g_DemandLoadingHorizon; departure_time += g_DepartureTimetInterval)
					{
						if(g_TDOVehicleArray[CurZoneID][departure_time/g_DepartureTimetInterval].VehicleArray.size() > 0)
						{
							network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),departure_time,1);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo
							
							if(g_ODEstimationFlag && iteration>=g_ODEstimation_StartingIteration)  // perform path flow adjustment after at least 10 normal OD estimation
								network_MP.VehicleBasedPathAssignment_ODEstimation(CurZoneID,departure_time,departure_time+g_DepartureTimetInterval,iteration);
							else
								network_MP.VehicleBasedPathAssignment(CurZoneID,departure_time,departure_time+g_DepartureTimetInterval,iteration);

						}
					}
				}
			}

		// below should be single thread


		if(g_ODEstimationFlag && iteration>=g_ODEstimation_StartingIteration)  // re-generate vehicles based on global path set
		{
		g_GenerateVehicleData_ODEstimation();
		}

		cout << "---- Network Loading for Iteration " << iteration <<"----" << endl;

		NetworkLoadingOutput SimuOutput;
		//	 DTANetworkForSP network(node_size, link_size, g_DemandLoadingHorizon);  // network instance for single-thread application

		SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag,0);

		TotalNumOfVehiclesGenerated = SimuOutput.NumberofVehiclesGenerated; // need this to compute avg gap

		g_AssignmentMOEAry[iteration]  = SimuOutput;

		float PercentageComplete = 0;

		if(SimuOutput.NumberofVehiclesGenerated>0)
			PercentageComplete =  SimuOutput.NumberofVehiclesCompleteTrips*100.0f/SimuOutput.NumberofVehiclesGenerated;

		g_LogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Average Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Average Distance: " << SimuOutput.AvgDistance << ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%"<<endl;
		cout << g_GetAppRunningTime() << "Iteration: " << iteration <<", Average Travel Time: " << SimuOutput.AvgTravelTime << ", Average Distance: " << SimuOutput.AvgDistance<< ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips << ", " << PercentageComplete << "%"<<endl;

		g_AssignmentLogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Ave Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Avg Distance: " << SimuOutput.AvgDistance  << ", Switch %:" << SimuOutput.SwitchPercentage <<", Num of Vehicles Completing Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%";			


		if(iteration <= 1) // compute relative gap after iteration 1
		{
			g_RelativeGap = 100; // 100%
		}else
		{
			g_RelativeGap = (fabs(g_CurrentGapValue - g_PrevGapValue) / min(1,g_PrevGapValue))*100;
		}
		g_PrevGapValue = g_CurrentGapValue; // update g_PrevGapValue

		if(iteration >= 1) // Note: we output the gap for the last iteration, so "iteration-1"
		{
			float avg_gap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;
			g_AssignmentLogFile << ", Num of Vehicles Switching Paths = " << g_CurrentNumOfVehiclesSwitched << ", Gap at prev iteration = " << g_CurrentGapValue << ", Avg Gap = " << avg_gap << ", Relative Gap at prev iteration = " << g_RelativeGap << "%" 
			<< ", total demand deviation for all paths: " << g_TotalDemandDeviation << ", total measurement deviation for all paths " << g_TotalMeasurementDeviation  << endl;				
				
		}else
			g_AssignmentLogFile << endl;		

	}	// end of outer loop

	cout << "Writing Vehicle Trajectory and MOE File... " << endl;

	if( iteration == g_NumberOfIterations)
	{ 
		iteration = g_NumberOfIterations -1;  //roll back to the last iteration if the ending condition is triggered by "iteration < g_NumberOfIterations"
	}

	bStartWithEmptyFile = true;
	OutputVehicleTrajectoryData("Vehicle.csv", iteration,true);

	g_ComputeFinalGapValue(); // Jason : compute and output final gap
		float avg_gap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;
		g_AssignmentLogFile << "Final Iteration: " << iteration << ", Gap = " << g_CurrentGapValue << ", Avg Gap = " << avg_gap << ", Relative Gap = " << g_RelativeGap << "%" << endl;
}

