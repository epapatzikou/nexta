//  Portions Copyright 2010 Xuesong Zhou, Hao Lei

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

#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"
#include <algorithm>
using namespace std;

/***************
to do list:
completed: read historical demand file

0. read target OD demand

1. Read sensor data, assign data to each link. NEXTA format, speed/ travel time, link count or density

2. Perform traffic assignment, compare the error statistics based on vehicle trajectory
calculate the link MOE error(l,tau) = error term (obs MOE- simulated)

3. iterative adjustment at each iteration

3.1 for each link, for each observation time interval tau, identify the state and the end timestamp of a traffic state phase (FF-FF, FF-PC-FF, FF-PC-CC, CC-CC, CC-PC-FF)
min - by - min

3.2 gather all vehicle data to reconstruct a dynamic OD matrix,
calcuate each OD error term (target - simulated OD)

3.3 for each vehicle

	path flow marginal = OD error term () + sub-gradient multiplier * (current cost - shortest path cost)

	starting time (ARR arrival time) assuming in FF
	walk through each link l in the path 
	
	Find NTS for time ARR on link l

--- update vehicle marginal ---
	
--	FLOW MOE --
	{
	for the next timestamp (NTS) of the current phase
	path flow marginal += link flow error(l,ARR)* +1
	}

--	DENSITY MOE -- 
	from t = ARR to NTS
	{
	FF-FF, FF-PC-FF:
	path flow marginal += link DENSITY error(l,t)
	}
-- Travel time MOE --
	{
	FF-FF, CC-CC: no change
	FF--PC-FF or FF--PC-CC
		path flow marginal += link travel time error(l,t)

	}

	FF: move to the next link. 
	ARR = ARR+ FFTT(l)
	l = l+1
	
	FF-PC-FF:
	ARR = NTS+ FFTT(l)
	l = l+1

	FF-PC-CC: 
	move back to the previous link, l= l-1
	ARR = NTS - FFTT(l) /// now l is "l-1" previous link of the current link"

	CC-CC:
	move back to the previous link, l= l-1
	ARR = NTS - FFTT(l-1)

	ensure (ARR is greater then the departure time and the end of simulation time interval)
	
	ensure (l >=0, l <= number of links)
	
	end of case

	
	path flow marginal > 0 should reduce flow

	1 (current flow unit) - step size * path flow marginal

	run (- step size * path flow marginal)
	
	endfor


*////////

float    g_ODEstimation_Weight_ODDemand = 1.0f;
float    g_ODEstimation_Weight_Flow = 1.0f;
float    g_ODEstimation_Weight_NumberOfVehicles = 1.0f;
float    g_ODEstimation_Weight_TravelTime = 1.0f;
float	g_ODEstimation_Weight_Gap = 1.0f;
float    g_ODEstimation_StepSize  = 0.1f;
std::vector<PathArrayForEachODTK> g_ODTKPathVector;
int g_ODEstimationFlag = 0;
int g_ODEstimation_StartingIteration = 2;

float*** g_HistODDemand = NULL;

float*** g_CurrentODDemand = NULL;
float*** g_ODDemandProportion;
float*** g_ODDemandAdjustment;

int g_ODDemandIntervalSize = 1;

void g_ReadHistDemandFile()
{
	g_ODDemandIntervalSize = g_DemandLoadingHorizon/g_DepartureTimetInterval;

	g_HistODDemand = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);


	for(int i = 0; i<=g_ODZoneSize; i++)
		for(int j = 0; j<=g_ODZoneSize; j++)
			for(int t=0; t<g_ODDemandIntervalSize; t++)
			{
			g_HistODDemand[i][j][t] = 0;
			}
//g_CurrentODDemand = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);


	FILE* st = NULL;

	fopen_s(&st,"input_hist_demand.csv","r");
	if(st!=NULL)
	{
		cout << "Reading file input_hist_demand.csv..."<< endl;

		int line_no = 1;
		int originput_zone, destination_zone;
		float number_of_vehicles ;
		int vehicle_type;
		float starting_time_in_min;
		float ending_time_in_min;

		while(!feof(st))
		{
			originput_zone = g_read_integer(st);
			if(originput_zone == -1)  // reach end of file
				break;

			destination_zone =  g_read_integer(st);
			number_of_vehicles =  g_read_float(st);

			vehicle_type =  g_read_integer(st);
			starting_time_in_min = g_read_float(st);
			ending_time_in_min = g_read_float(st);

			int time_interval_no = starting_time_in_min/g_DepartureTimetInterval;

			if(time_interval_no <g_ODDemandIntervalSize)
			{
			g_HistODDemand [originput_zone][destination_zone][time_interval_no]  = number_of_vehicles;
			}else
			{
			//error message
			}

//			TRACE("o:%d d: %d, %f,%d,%f,%f\n", originput_zone,destination_zone,number_of_vehicles,vehicle_type,starting_time_in_min,ending_time_in_min);

			float Interval= float(ending_time_in_min - starting_time_in_min);

			if(line_no %100000 ==0)
			{
				cout << g_GetAppRunningTime() << "Reading " << line_no/1000 << "K lines..."<< endl;
			}

			line_no++;
		}

		fclose(st);
	}
}

void g_ReadLinkMeasurementFile(DTANetworkForSP* pPhysicalNetwork)
{
//from_node_id	 to_node_id	Start_ timestamp_in_min	end_timestamp_in_min	 link_volume_in_veh_per_interval_for_all_lanes	density_in_veh_per_mile_per_lane	 speed_in_mph
	FILE* st = NULL;

	fopen_s(&st,"input_measurement.csv","r");
	if(st!=NULL)
	{
		cout << "Reading file input_measurement.csv..."<< endl;

		int line_no = 1;
		int usn, dsn, start_time, end_time;
		float flow, density, speed;

		while(!feof(st))
		{
// from_node_id	 to_node_id	Start_timestamp_in_min	end_timestamp_in_min	 link_volume_in_veh_per_interval_for_all_lanes	density_in_veh_per_mile_per_lane	 speed_in_mph
// 1	2	25	26	0	0.25	35

			usn  = g_read_integer(st);

			if(usn < 0)
				break;

			dsn =  g_read_integer(st);

			int LinkID = pPhysicalNetwork->GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

			DTALink* plink = g_LinkVector[LinkID];

			if(plink!=NULL)
			{
			start_time  = g_read_integer(st);
			end_time =  g_read_integer(st);


			if(line_no==1)
			{
			g_ObservationTimeInterval = end_time - start_time;   // initialize observation time interval
			
			}
	
			if(start_time < g_SimulationHorizon)
			{
			int time_index = start_time/g_ObservationTimeInterval;

			flow = g_read_float(st);
			density = g_read_float(st);
			speed = g_read_float(st);

			plink->m_LinkMeasurementAry[time_index].StartTime = start_time;
			plink->m_LinkMeasurementAry[time_index].EndTime  = end_time;

			plink->m_LinkMeasurementAry[time_index].ObsFlowCount = flow;
			plink->m_LinkMeasurementAry[time_index].ObsNumberOfVehicles = density * plink->m_Length * plink->m_NumLanes ;  // converted from density
			plink->m_LinkMeasurementAry[time_index].ObsTravelTime = plink->m_Length / max(1,speed)*60;   // converted from speed, 60, hour to min

			}
		
			}

			line_no++;
		 }

	fclose(st);

	// second step: start reading historical demand
	g_ReadHistDemandFile();

	}
}


void g_ReadCurrentDemandFile()
{
	FILE* st = NULL;

	fopen_s(&st,"input_demand.csv","r");
	if(st!=NULL)
	{
		cout << "Reading file input_demand.csv..."<< endl;

		int line_no = 1;
		int originput_zone, destination_zone;
		float number_of_vehicles ;
		int vehicle_type;
		float starting_time_in_min;
		float ending_time_in_min;

		while(!feof(st))
		{
			originput_zone = g_read_integer(st);
			if(originput_zone == -1)  // reach end of file
				break;

			destination_zone =  g_read_integer(st);
			number_of_vehicles =  g_read_float(st);

			vehicle_type =  g_read_integer(st);
			starting_time_in_min = g_read_float(st);
			ending_time_in_min = g_read_float(st);

			int time_interval_no = starting_time_in_min/g_DepartureTimetInterval;

			if(time_interval_no <g_ODDemandIntervalSize)
			{
			g_CurrentODDemand [originput_zone][destination_zone][time_interval_no]  = number_of_vehicles;
			}else
			{
			//error message
			}

//			TRACE("o:%d d: %d, %f,%d,%f,%f\n", originput_zone,destination_zone,number_of_vehicles,vehicle_type,starting_time_in_min,ending_time_in_min);

			float Interval= float(ending_time_in_min - starting_time_in_min);

			if(line_no %100000 ==0)
			{
				cout << g_GetAppRunningTime() << "Reading " << line_no/1000 << "K lines..."<< endl;
			}

			line_no++;
		}

		fclose(st);
	}
}
void g_ODDemandEstimation()
{

//	g_ODDemandProportion = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);
//	g_ODDemandAdjustment = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);
	//+1 to make the OD zone index begin at 1

//	g_ReadHistDemandFile();
 
/* optimization notes
	given r as measurment error variance, P is path-tau-to-link proportion coefficient

	close-form solution for OD/path demand adjustment
	= P'/(P'*P+r) *error
		 where P'*P is sum of (product of path-to-link proportion) over all OD pairs
	
*/

  // outer loop for each adjustment iteration
  // for each outer loop
  
  // outer step 1: traffic simulation/assignment, call g_TrafficAssignmentSimulation()
  // outer step 2: g_ReadCurrentDemandFile();
  // outer step 3: initialize g_ODDemandAdjustment(O,D,tau) to zero
  // outer step 4: adjust OD for each link count

	/* below are inner steps for outer step 4
	// read link observation
	for each link count observation (a,t)
	{
		// below are  inner loop steps
		// step 0: calculate link observation deviation as dev
		// step 1: initialize g_ODDemandProportion(O,D,tau) = 0

		// steps below calculate link proportions
		// step 2. scan all vehicles, if vehicle v passes through link (a,t) then  g_ODDemandProportion(O,D,tau)+=1

		// step 3. g_ODDemandProportion(O,D,tau)  = g_ODDemandProportion(O,D,tau) /g_CurrentODDemand(O,D,tau)

		// step 4. calculate SumP=P'*P over all O, D, tau

		// step 5: for each time-dependent OD demand pair
		g_ODDemandAdjustment(O,D,tau) = g_ODDemandProportion(O,D,tau)/(SumP + r) * dev

	}
	endfor

	// outer step 5: add g_ODDemandAdjustment for each current ODdemand g_CurrentODDemand
	// if (g_ODDemandAdjustment+g_CurrentODDemand) is out of the bound of historical demand, reset to the bound

	// outer step 6: output g_CurrentODDemand to new demand table  
	
	//end for outer loop
	*/
	Deallocate3DDynamicArray<float>(g_ODDemandProportion,g_ODZoneSize+1,g_ODZoneSize+1);
	Deallocate3DDynamicArray<float>(g_ODDemandAdjustment,g_ODZoneSize+1,g_ODZoneSize+1);
}

void ConstructPathArrayForEachODT_ODEstimation(PathArrayForEachODT PathArray[], int zone, int AssignmentInterval)
{  // this function has been enhanced for path flow adjustment
	// step 1: initialization
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

	//step 2: construct path array
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

		PathArray[VehicleDest].NumOfVehicles++;  // count the number of vehicles from o to d at assignment time interval tau


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

	// step 3: identify the best path for each destination
	// calculate the path gap

	for(DestZoneID=1; DestZoneID <= g_ODZoneSize; DestZoneID++)
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

	// step 4: update OD demand flow 
		for(DestZoneID=1; DestZoneID <= g_ODZoneSize; DestZoneID++) // initialization...
	{
		// step 4.1: calculate the vehicle size deviation for  each O D tau pair

		float hist_demand = g_HistODDemand[zone][DestZoneID][AssignmentInterval];



		if(hist_demand < 0.001)  // only perform OD estimation when hist_demand  is positive
			continue; 

		PathArray[DestZoneID].DeviationNumOfVehicles = PathArray[DestZoneID].NumOfVehicles - hist_demand;


		for(int p=1; p<= PathArray[DestZoneID].NumOfPaths; p++)  // recall that path index start from 1 in this module
		{
		// step 4.2: assign the demand deviation as the initial value for path marginal 
			PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] = 0;
		// step 4.3: walk through the link sequence to accumlate the link marginals


					float ArrivalTime = AssignmentInterval * g_DepartureTimetInterval;
					int l = 0;
				while(l>=0 && l< PathArray[DestZoneID].PathSize[p]-1)
				{
					int LinkID = PathArray[DestZoneID].PathLinkSequences[p][l];

					DTALink* plink = g_LinkVector[LinkID];

					//static: always reset ArrivalTime to zero.
					if(g_TrafficFlowModelFlag)
						ArrivalTime = 0.0f;

					if(plink->m_LinkMOEAry [(int)(ArrivalTime)].TrafficStateCode == 0) 					// if ArrivalTime is Free-flow,
					{
					
					int obs_time_index = ArrivalTime/g_ObservationTimeInterval;
					if(plink->m_LinkMeasurementAry [obs_time_index].ObsFlowCount > 0) // with flow measurement
					{
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= plink->m_LinkMeasurementAry [obs_time_index].ErrorFlowCount;

					if(plink->m_LinkMeasurementAry [obs_time_index].ObsNumberOfVehicles  > 0) // with density measurement
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= plink->m_LinkMeasurementAry [obs_time_index].ErrorNumberOfVehicles;

					if(plink->m_LinkMeasurementAry [obs_time_index].ErrorTravelTime   > 0) // with travel time measurement
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= 0;   // not changing travel time [to be completed]
				
					}
					
						ArrivalTime+= plink->m_FreeFlowTravelTime ;
							l+= 1;  // move forward to the next link
					}
					
					if(plink->m_LinkMOEAry [(int)(ArrivalTime)].TrafficStateCode >0) 					// if ArrivalTime is partially congested or fully congested,
					{
					
					int obs_time_index = ArrivalTime/g_ObservationTimeInterval;
					if(plink->m_LinkMeasurementAry [obs_time_index].ObsFlowCount > 0) // with flow measurement
					{
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= plink->m_LinkMeasurementAry [obs_time_index].ErrorFlowCount;

					if(plink->m_LinkMeasurementAry [obs_time_index].ObsNumberOfVehicles  > 0) // with density measurement
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= plink->m_LinkMeasurementAry [obs_time_index].ErrorNumberOfVehicles;

					if(plink->m_LinkMeasurementAry [obs_time_index].ObsTravelTime   > 0) // with travel time measurement
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= 1.0/plink->m_BPRLaneCapacity*60;   // 1/c as discharge rate, * 60 as min
				
					}
					
						ArrivalTime+= plink->m_LinkMOEAry [(int)(ArrivalTime)].EndTimeOfPartialCongestion ;  // move to the end of congestion

						if(plink->m_LinkMOEAry [(int)(ArrivalTime)].TrafficStateCode == 0) // freeflow
							l+= 1;  // move forward to the next link
						else // fully congested
						{
								l-= 1;  // move backward to the previous link

								int LinkIDPrev = PathArray[DestZoneID].PathLinkSequences[p][l];

								DTALink* plinkPrev = g_LinkVector[LinkIDPrev];

								ArrivalTime -= plinkPrev->m_FreeFlowTravelTime ;


						}

					}


					
				}

		// calculate the total demand deviation statistics only for the first path
		if(p==1) 
			g_TotalDemandDeviation += PathArray[DestZoneID].DeviationNumOfVehicles;

		g_TotalMeasurementDeviation += PathArray[DestZoneID].MeasurementDeviationPathMarginal[p];

				PathArray[DestZoneID].NewVehicleSize[p] = max(0,
					PathArray[DestZoneID].NumOfVehsOnEachPath[p] 
				- g_ODEstimation_StepSize*(
					g_ODEstimation_Weight_ODDemand * PathArray[DestZoneID].DeviationNumOfVehicles
				+ PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] 
				+ g_ODEstimation_Weight_Gap*PathArray[DestZoneID].AvgPathGap[p]));





			PathArrayForEachODTK element;
			int PathIndex  = g_ODTKPathVector.size();
			element.Setup (PathIndex,zone, DestZoneID, 1, AssignmentInterval *g_DepartureTimetInterval, (AssignmentInterval+1) *g_DepartureTimetInterval,
				PathArray[DestZoneID].PathSize[p]-1, PathArray[DestZoneID].PathLinkSequences[p],PathArray[DestZoneID].NewVehicleSize[p],PathArray[DestZoneID].PathNodeSums[p] );
			g_ODTKPathVector.push_back(element);


			g_AssignmentLogFile << "OED: O: " << zone << ", D:" << DestZoneID << "Demand Dev: " << PathArray[DestZoneID].DeviationNumOfVehicles << ", Path:" << p << ", marginal: " <<  PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] << ", AvgTT: "<< PathArray[DestZoneID].AvgPathTimes[p]<< ", Gap : "<< PathArray[DestZoneID].AvgPathGap[p]<<  ", VehicleSize:" << PathArray[DestZoneID].NewVehicleSize[p] << endl;

		}   // for path p

	}


}


void DTANetworkForSP::VehicleBasedPathAssignment_ODEstimation(int zone,int departure_time_begin, int departure_time_end, int iteration)
// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
{

	int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int AssignmentInterval = int(departure_time_begin/g_DepartureTimetInterval);  // starting assignment interval

	// g_VehcileIDPerAssignmentIntervalMap is more like a cursor to record the last scanned position


	//PathArrayForEachODT PathArray[20]; // for test only
	PathArrayForEachODT *PathArray;
	PathArray = new PathArrayForEachODT[g_ODZoneSize + 1]; // remember to release memory

	ConstructPathArrayForEachODT_ODEstimation(PathArray, zone, AssignmentInterval);

	delete PathArray;
}


void g_GenerateVehicleData_ODEstimation()
{
	
	g_FreeODTKPathVector();

	FILE* st = NULL;
	bool bFileReady = false;
	int i,t,z;

			PathArrayForEachODTK element;

			for(int vi = 0; vi< g_ODTKPathVector.size(); vi++)
			{

				element = g_ODTKPathVector[vi];
			if(element.m_starting_time_in_min < g_DemandLoadingHorizon && element.m_OriginZoneID != element.m_DestinationZoneID )
			{
				CreateVehicles(element.m_OriginZoneID,element.m_DestinationZoneID ,element.m_VehicleSize ,element.m_VehicleType,element.m_starting_time_in_min,element.m_ending_time_in_min,element.m_PathIndex );
			}

			}
	// create vehicle heres...
	cout << "Converting demand flow to vehicles..."<< endl;

	std::sort(g_simple_vector_vehicles.begin(), g_simple_vector_vehicles.end());

	std::vector<DTA_vhc_simple>::iterator kvhc =  g_simple_vector_vehicles.begin();

	DTAVehicle* pVehicle = 0;

	i = 0;
	while(kvhc != g_simple_vector_vehicles.end())
	{
		if(kvhc->m_OriginZoneID!= kvhc->m_DestinationZoneID)    // only consider intra-zone traffic
		{

			pVehicle = new DTAVehicle;
			if(pVehicle == NULL)
			{
				cout << "Insufficient memory...";
				getchar();
				exit(0);

			}
			pVehicle->m_VehicleID		= i;
			pVehicle->m_RandomSeed = pVehicle->m_VehicleID;

			pVehicle->m_OriginZoneID	= kvhc->m_OriginZoneID ;
			pVehicle->m_DestinationZoneID 	= kvhc->m_DestinationZoneID ;
			pVehicle->m_DepartureTime	= kvhc->m_DepartureTime;
			pVehicle->m_TimeToRetrieveInfo = (int)(pVehicle->m_DepartureTime*10);


			pVehicle->m_VehicleType	= kvhc->m_VehicleType;
			pVehicle->m_Occupancy		= 1;
			pVehicle->m_InformationClass = kvhc->m_InformationClass;

			PathArrayForEachODTK element = g_ODTKPathVector[kvhc->m_PathIndex ];

				int NodeSize = element.m_LinkSize+1;
				pVehicle->m_NodeSize = NodeSize;
				pVehicle->m_aryVN = new SVehicleLink[pVehicle->m_NodeSize];

				if(pVehicle->m_aryVN==NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				pVehicle->m_NodeNumberSum =  element.m_NodeSum ;
				pVehicle->m_Distance =0;

				for(int j = 0; j< pVehicle->m_NodeSize-1; j++)
				{
					pVehicle->m_aryVN[j].LinkID = element.m_LinkNoArray[j];
					pVehicle->m_Distance+= g_LinkVector[pVehicle->m_aryVN [j].LinkID] ->m_Length ;
				}

			g_VehicleVector.push_back(pVehicle);
			g_VehicleMap[i]  = pVehicle;  // i is the vehicle id


			int AssignmentInterval = int(pVehicle->m_DepartureTime/g_DepartureTimetInterval);

			if(AssignmentInterval >= g_DepartureTimetIntervalSize)
			{
				AssignmentInterval = g_DepartureTimetIntervalSize - 1;
			}
			g_TDOVehicleArray[pVehicle->m_OriginZoneID][AssignmentInterval].VehicleArray .push_back(i);


		}
		kvhc++;
		i++;
	}
	g_simple_vector_vehicles.clear ();
}





void g_UpdateLinkMOEDeviation_ODEstimation()
{

	std::set<DTALink*>::iterator iterLink;

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{	
			if(g_TrafficFlowModelFlag ==0)  // static traffic assignment
			{

				g_LinkVector[li]->m_LinkMeasurementAry[0].SimuFlowCount = g_LinkVector[li]->m_BPRLinkVolume;
				g_LinkVector[li]->m_LinkMeasurementAry[0].ErrorFlowCount = g_LinkVector[li]->m_LinkMeasurementAry[0].SimuFlowCount - g_LinkVector[li]->m_LinkMeasurementAry[0].ObsFlowCount ;
			
				g_LinkVector[li]->m_LinkMeasurementAry[0].SimuNumberOfVehicles = 0;  // not utilizing vehicle measurments in static assignment 
				g_LinkVector[li]->m_LinkMeasurementAry[0].ErrorNumberOfVehicles  = 0;

				g_LinkVector[li]->m_LinkMeasurementAry[0].SimuTravelTime  = g_LinkVector[li]->m_BPRLinkTravelTime;  // not utilizing vehicle measurments in static assignment 
				g_LinkVector[li]->m_LinkMeasurementAry[0].ErrorTravelTime = g_LinkVector[li]->m_LinkMeasurementAry[0].SimuTravelTime   - g_LinkVector[li]->m_LinkMeasurementAry[0].ObsTravelTime;

			}else  // dynamic network loading
			{

		// compare the error statistics
				for(int time = 0; time< g_SimulationHorizon;time+=g_ObservationTimeInterval)
				{

				int time_index = time/g_ObservationTimeInterval;
				if(time_index >=1 && g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsFlowCount >0)
				{
					// CumulativeArrivalCount is available after this time interval, so we calculate statistics on time time_index-1
					int SimulatedFlowCount = g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount - g_LinkVector[li]->m_LinkMOEAry[time-g_ObservationTimeInterval].CumulativeArrivalCount; 
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuFlowCount = SimulatedFlowCount;

					float ObsFlowCount  = g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsFlowCount;
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ErrorFlowCount = SimulatedFlowCount -  ObsFlowCount;
						

					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuNumberOfVehicles  = 
						g_LinkVector[li]->m_LinkMOEAry[time-g_ObservationTimeInterval].CumulativeDepartureCount  - g_LinkVector[li]->m_LinkMOEAry[time-g_ObservationTimeInterval].CumulativeArrivalCount;
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ErrorNumberOfVehicles  = 
						g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuNumberOfVehicles - g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsNumberOfVehicles  ;

					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuTravelTime    = g_LinkVector[li]->GetTravelTime(time-g_ObservationTimeInterval,1);
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ErrorTravelTime   = 
						g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuTravelTime - g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsTravelTime   ;

				}
				}


				}	
		}
}
