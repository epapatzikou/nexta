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
float	 g_ODEstimation_Weight_Gap = 1.0f;
float    g_ODEstimation_StepSize  = 0.1f;

std::vector<PathArrayForEachODTK> g_ODTKPathVector;
int g_ODEstimationFlag = 0;
int g_ODEstimationMeasurementType = 0; // 0: flow, 1: density, 2, speed
int g_ODEstimation_StartingIteration = 2;

float*** g_HistODDemand = NULL;

int g_ODDemandIntervalSize = 1;
std::map<long, long> g_LinkIDtoSensorIDMap;


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

			FILE* st = NULL;

			fopen_s(&st,"input_hist_demand.csv","r");
			if(st!=NULL)
			{

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
						float demand_interval_ratio = g_DepartureTimetInterval/max(1, ending_time_in_min-starting_time_in_min);
						for(int demand_time_interval_no = starting_time_in_min/g_DepartureTimetInterval; demand_time_interval_no < ending_time_in_min/g_DepartureTimetInterval;  demand_time_interval_no++)
						{
							g_HistODDemand [originput_zone][destination_zone][time_interval_no]  = number_of_vehicles * demand_interval_ratio;
						}
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
				cout << "Reading file input_hist_demand.csv with "<< line_no << " lines."  << endl;

				fclose(st);
			}
}

void g_ReadLinkMeasurementFile(DTANetworkForSP* pPhysicalNetwork)
{
	std::map<long, long> SensorIDtoLinkIDMap;

	FILE* st = NULL;
	fopen_s(&st,"input_sensor_location.csv","r");

	if(st!=NULL)
	{
		while(!feof(st))
		{
			int FromNodeNumber =  g_read_integer(st);

			if(FromNodeNumber == -1)
				break;
			int ToNodeNumber =  g_read_integer(st);
			int SensorType  =  g_read_integer(st);
			int OrgSensorID  =  g_read_integer(st);

			int LinkID = pPhysicalNetwork->GetLinkNoByNodeIndex(g_NodeNametoIDMap[FromNodeNumber], g_NodeNametoIDMap[ToNodeNumber]);

			if(SensorType == 0) // freeway sensor only
			{
				SensorIDtoLinkIDMap[OrgSensorID] = LinkID;
				g_LinkIDtoSensorIDMap[LinkID] = OrgSensorID;
			}

		}
		cout << "Reading file input_sensor_location.csv with "<< SensorIDtoLinkIDMap.size() << " valid sensors." << endl;

	fopen_s(&st,"input_sensor_location.csv","r");

		fclose(st);
	}

	fopen_s(&st,"SensorDataDay001.csv","r");

	if(st!=NULL)
	{
		int number_of_samples = 0;
		while(!feof(st))
		{
			int Month = g_read_integer(st);
			if(Month == -1)  // reach end of file
				break;

			int DayOfMonth   = g_read_integer(st);
			int Year  = g_read_integer(st);
			int Hour   = g_read_integer(st);
			int Min   = g_read_integer(st);

			int SensorID  =  g_read_integer(st);
			float TotalFlow = g_read_float(st);
			float Occupancy = g_read_float(st);
			float Speed = g_read_float(st);

			map <long, long> :: iterator mIter  = SensorIDtoLinkIDMap.find(SensorID);

			if ( mIter != SensorIDtoLinkIDMap.end( ) )
			{

				int LinkID  = mIter -> second;

				DTALink* plink = g_LinkVector[LinkID];

				if(plink!=NULL)
				{
					int t  = max(0, (Hour*60+Min) );

					if(t >=  g_ObservationStartTime && t <=  g_ObservationEndTime)
					{
						t = t- g_ObservationStartTime;  // start from zero 
						int time_index = t / g_ObservationTimeInterval;
						plink->m_LinkMeasurementAry[time_index].StartTime =  t ;
						plink->m_LinkMeasurementAry[time_index].EndTime  = t + g_ObservationTimeInterval;

						plink->m_LinkMeasurementAry[time_index].ObsFlowCount = TotalFlow;

						if(Speed>=1)
						{
							plink->m_LinkMeasurementAry[time_index].ObsTravelTime = plink->m_Length / max(1,Speed)*60;   // converted from speed, 60, hour to min

							// only process density data when speed data is available, to obtain reliable input
							float density;
							if(Occupancy <=0.001)
								density = TotalFlow*60/g_ObservationTimeInterval/plink->m_NumLanes / max(1.0f,Speed);
							else
								density = Occupancy * 100;

							if(Occupancy>0.01) 
							{
								plink->m_LinkMeasurementAry[time_index].ObsNumberOfVehicles = density * plink->m_Length * plink->m_NumLanes ;  // converted from density
							}

						}
						number_of_samples++;
					}
				}
			}
		}

		cout << "Reading file SensorDataDay001.csv with "<< number_of_samples << " valid samples." << endl;
		fclose(st);


	}


}

void g_ReadObservedLinkMOEData(DTANetworkForSP* pPhysicalNetwork) // As second type of input data, that accepts data from simulation directly. 
{
	FILE* st = NULL;
	fopen_s(&st,"ObsLinkMOE.csv","r");

	if(st!=NULL)
	{
		cout << "Reading file ObsLinkMOE.csv" << endl;

		while(!feof(st))
		{
			// from_node_id, to_node_id, timestamp_in_min, travel_time_in_min, delay_in_min, link_volume_in_veh, link_volume_in_vehphpl,
			//density_in_veh_per_mile_per_lane, speed_in_mph, queue_length_in_, cumulative_arrival_count, cumulative_departure_count
			int from_node_number = g_read_integer(st);
			if(from_node_number < 0)
				break;
			int to_node_number =  g_read_integer(st);
			int t = g_read_float(st);

			float ObsTravelTimeIndex = g_read_float(st);
			float delay_in_min = g_read_float(st);
			float ObsFlow  = g_read_float(st);

			float volume_for_alllanes = g_read_float(st);

			float ObsDensity = g_read_float(st);
			float ObsSpeed = g_read_float(st);
			float ObsQueuePerc = g_read_float(st);
			int ObsCumulativeFlow =  g_read_integer(st);
			int cumulative_departure_count = g_read_integer(st);

			int LinkID = pPhysicalNetwork->GetLinkNoByNodeIndex(g_NodeNametoIDMap[from_node_number], g_NodeNametoIDMap[to_node_number]);

			DTALink* plink = g_LinkVector[LinkID];

			if(plink!=NULL && t >=  g_ObservationStartTime && t <=  g_ObservationEndTime)
			{

						int time_index = t / g_ObservationTimeInterval;
						plink->m_LinkMeasurementAry[time_index].StartTime =  t ;
						plink->m_LinkMeasurementAry[time_index].EndTime  = t + g_ObservationTimeInterval;

						plink->m_LinkMeasurementAry[time_index].ObsFlowCount = ObsFlow;

						if(ObsSpeed>=1)
						{
							plink->m_LinkMeasurementAry[time_index].ObsTravelTime = plink->m_Length / max(1,ObsSpeed)*60;   // converted from speed, 60, hour to min

							plink->m_LinkMeasurementAry[time_index].ObsNumberOfVehicles = ObsDensity * plink->m_Length * plink->m_NumLanes ;  // converted from density

						}
						
			}
		}
	fclose(st);
		// second step: start reading historical demand
		g_ReadHistDemandFile();
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
			float LeastTime = 999999.0;

			int p;
			for(p=1; p<=PathArray[DestZoneID].NumOfPaths; p++)
			{
				if(PathArray[DestZoneID].AvgPathTimes[p] < LeastTime)
				{
					LeastTime = PathArray[DestZoneID].AvgPathTimes[p];
					BestPath = p;
				}
			}
			PathArray[DestZoneID].BestPathIndex = BestPath;
			PathArray[DestZoneID].LeastTravelTime = LeastTime;


			for(p=1; p<=PathArray[DestZoneID].NumOfPaths; p++)
			{

				PathArray[DestZoneID].AvgPathGap[p] =  PathArray[DestZoneID].AvgPathTimes[p] -  LeastTime;
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

		g_EstimationLogFile << "OD demand " << zone << " -> " << DestZoneID <<  " @ "<< AssignmentInterval*g_DepartureTimetInterval << " with "<< PathArray[DestZoneID].NumOfPaths << " paths; Hist Demand =" << hist_demand << "; simu Demand = " << PathArray[DestZoneID].NumOfVehicles  << ";Dev = " << PathArray[DestZoneID].DeviationNumOfVehicles <<endl; 

		if(AssignmentInterval == 6) 
			TRACE("");

		for(int p=1; p<= PathArray[DestZoneID].NumOfPaths; p++)  // recall that path index start from 1 in this module
		{
			// step 4.2: assign the demand deviation as the initial value for path marginal 
			PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] = 0;
			// step 4.3: walk through the link sequence to accumlate the link marginals


			float ArrivalTime = AssignmentInterval * g_DepartureTimetInterval;
			int l = 0;
			while(l>=0 && l< PathArray[DestZoneID].PathSize[p]-1)  // for each link along the path
			{
				int LinkID = PathArray[DestZoneID].PathLinkSequences[p][l];

				DTALink* plink = g_LinkVector[LinkID];

				//static: always reset ArrivalTime to zero.
				if(g_TrafficFlowModelFlag == 0)
					ArrivalTime = 0.0f;

				if(plink->m_LinkMOEAry [(int)(ArrivalTime)].TrafficStateCode == 0) 					// if Arrival at Free-flow condition.
				{

					int obs_time_index = ArrivalTime/g_ObservationTimeInterval;  // convert arrival time to observation time index
					if(plink->m_LinkMeasurementAry [obs_time_index].ObsFlowCount > 0 && g_ODEstimationMeasurementType ==0) // with flow measurement
					{
					PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= plink->m_LinkMeasurementAry [obs_time_index].ErrorFlowCount;
					}

					if(plink->m_LinkMeasurementAry [obs_time_index].ObsNumberOfVehicles  > 0 && g_ODEstimationMeasurementType ==1) // with density measurement
					{
					 PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= plink->m_LinkMeasurementAry [obs_time_index].ErrorNumberOfVehicles;
					}

					if(plink->m_LinkMeasurementAry [obs_time_index].ErrorTravelTime   > 0 && g_ODEstimationMeasurementType ==2) // with travel time measurement
					{
							PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= 0;   // not changing travel time under free-flow congestions 
					}

					ArrivalTime+= plink->m_FreeFlowTravelTime ;
					l+= 1;  // move forward to the next link
				}

					int ArrivalTime_int = (int)(ArrivalTime);

					if (ArrivalTime_int >= g_SimulationHorizon)  // time of interest exceeds the simulation horizon
						break;

					TRACE("traffic state = %d \n",plink->m_LinkMOEAry [ArrivalTime_int].TrafficStateCode);
					if(plink->m_LinkMOEAry [ArrivalTime_int].TrafficStateCode >0) 					// if ArrivalTime is partially congested or fully congested,
				{

					int obs_time_index = ArrivalTime/g_ObservationTimeInterval; // convert arrival time to observation time index
					int obs_time_index_for_end_of_congestion = plink->m_LinkMOEAry [(int)(ArrivalTime)].EndTimeOfPartialCongestion/g_ObservationTimeInterval ;  // move to the end of congestion


					if(plink->m_LinkMeasurementAry [obs_time_index].ObsFlowCount > 0 && g_ODEstimationMeasurementType ==0) // with flow measurement
					{
						// flow rate gradient  // we assume the sensor is located upstream of the link, so we do not consider the departure flow rate increase at the downstream of the link
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= plink->m_LinkMeasurementAry [obs_time_index].ErrorFlowCount;

					}

						//density gradient
					if(plink->m_LinkMeasurementAry [obs_time_index].ObsNumberOfVehicles  > 0 && g_ODEstimationMeasurementType ==1) // with density measurement
						{
							for(int obs_time_id = obs_time_index; obs_time_id < obs_time_index_for_end_of_congestion; obs_time_id++)
							{
								PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= plink->m_LinkMeasurementAry [obs_time_id].ErrorNumberOfVehicles;
							}

						}

						// travel time gradient for travel time measurments: 
					if(plink->m_LinkMeasurementAry [obs_time_index].ObsTravelTime   > 0 && g_ODEstimationMeasurementType ==2) // with travel time measurement
						{
							for(int obs_time_id = obs_time_index; obs_time_id < obs_time_index_for_end_of_congestion; obs_time_id++)
							{
								PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= 1.0/plink->m_BPRLaneCapacity*60;   // 1/c as discharge rate, * 60 as min
							}
						}

					

					// move to the end of congestion
					int OldArrivalTime = (int)(ArrivalTime);
					ArrivalTime = plink->m_LinkMOEAry [(int)(ArrivalTime)].EndTimeOfPartialCongestion ;  
					int NewArrivalTime = (int)(ArrivalTime);
					
					// remark: EndTimeOfPartialCongestion is like the end time of the current episo. 
					// arrval time is the arrival time at the exit queue


					if(plink->m_LinkMOEAry [(int)(ArrivalTime)].TrafficStateCode == 0) // freeflow
					{
						l+= 1;  // move forward to the next link
					}
					else // fully congested
					{
						// keep the same link, move to the next arrival time, end of the event duration

						TRACE("fully congested, move to new time");
						ArrivalTime+=1;

					}

				}

					TRACE("link index l = %d, arrival time = %5.1f \n",l,ArrivalTime);


			}

			// calculate the total demand deviation statistics only for the first path
			if(p==1) 
				g_TotalDemandDeviation += PathArray[DestZoneID].DeviationNumOfVehicles;

			g_TotalMeasurementDeviation += PathArray[DestZoneID].MeasurementDeviationPathMarginal[p];

			// this is a very important path flow adjustment equation;

			float FlowAdjustment = 
			g_ODEstimation_Weight_ODDemand * PathArray[DestZoneID].DeviationNumOfVehicles /* graident wrt targe demand*/
				+ PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] /* gradient wrt measurements (link incoming flow, density and travel time*/
			+ g_ODEstimation_Weight_Gap*PathArray[DestZoneID].AvgPathGap[p];

			PathArray[DestZoneID].NewVehicleSize[p] = max(0,
				PathArray[DestZoneID].NumOfVehsOnEachPath[p]   // this is the existing path flow volume
			- g_ODEstimation_StepSize*FlowAdjustment); /*gradient wrt gap function*/

			
			g_EstimationLogFile << "OD " << zone << " -> " << DestZoneID << " path =" << p << " @ "<< AssignmentInterval*g_DepartureTimetInterval << ": Measurement Dev=" << PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] << 
				"; Demand Dev=" << PathArray[DestZoneID].DeviationNumOfVehicles <<
				"; Gap Dev "<< g_ODEstimation_Weight_Gap*PathArray[DestZoneID].AvgPathGap[p]  <<endl; 


			if(zone==2 && DestZoneID == 32)
				TRACE("");

			PathArrayForEachODTK element;
			int PathIndex  = g_ODTKPathVector.size();
			element.Setup (PathIndex,zone, DestZoneID, 1, AssignmentInterval *g_DepartureTimetInterval, (AssignmentInterval+1) *g_DepartureTimetInterval,
				PathArray[DestZoneID].PathSize[p]-1, PathArray[DestZoneID].PathLinkSequences[p],PathArray[DestZoneID].NewVehicleSize[p],PathArray[DestZoneID].PathNodeSums[p] );
			g_ODTKPathVector.push_back(element);


//			g_AssignmentLogFile << "OED: O: " << zone << ", D:" << DestZoneID << "Demand Dev: " << PathArray[DestZoneID].DeviationNumOfVehicles << ", Path:" << p << ", marginal: " <<  PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] << ", AvgTT: "<< PathArray[DestZoneID].AvgPathTimes[p]<< ", Gap : "<< PathArray[DestZoneID].AvgPathGap[p]<<  ", VehicleSize:" << PathArray[DestZoneID].NewVehicleSize[p] << ", flow adjustment" << FlowAdjustment << endl;

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

	 g_EstimationLogFile << " g_GenerateVehicleData_ODEstimation "  <<endl; 

	g_FreeVehicleVector();

	for(int z = 0; z < g_ODZoneSize+1; z++)
		for(int di = 0; di < g_DepartureTimetIntervalSize; di++)
		{
			g_TDOVehicleArray[z][di].VehicleArray.clear ();
		}

	



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
	g_EstimationLogFile << " Converting demand flow to vehicles... "  <<endl; 

	cout << "Converting demand flow to vehicles..."<< endl;

	std::sort(g_simple_vector_vehicles.begin(), g_simple_vector_vehicles.end());
	g_EstimationLogFile << " std::sort... "  <<endl; 

	std::vector<DTA_vhc_simple>::iterator kvhc =  g_simple_vector_vehicles.begin();

	DTAVehicle* pVehicle = 0;

	int i = 0;
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


		i++;
		}
		kvhc++;

	}
	g_simple_vector_vehicles.clear ();
}





void g_UpdateLinkMOEDeviation_ODEstimation()
{
//MAPE Mean absolute percentage error 
//RMSE  root mean sequared error

	float TotaMOESampleSize  = 0;
	float TotalFlowError = 0;
	float TotalFlowSequaredError = 0;

	float TotalMOEPercentageError = 0;
	float TotalMOEAbsError = 0;

	float TotalDensityError = 0;

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

					if(ObsFlowCount >= 1 && g_ODEstimationMeasurementType ==0)  // flow count
					{
						float AbosolutePercentageError = fabs((SimulatedFlowCount -  ObsFlowCount)/ObsFlowCount*100);
						float LaneFlowError = (SimulatedFlowCount -  ObsFlowCount)*60.0f/g_ObservationTimeInterval/g_LinkVector[li]->m_NumLanes;												
						g_EstimationLogFile << "Link " << g_LinkVector[li]->m_FromNodeNumber << "->" << g_LinkVector[li]->m_ToNodeNumber 
							<< " @ "<< time << " Obs link flow: "<< ObsFlowCount <<"; Error: " << SimulatedFlowCount -  ObsFlowCount << 
							", " << AbosolutePercentageError << " %" << "Lane Flow Error /h= " << LaneFlowError << endl;

						TotalMOEPercentageError +=AbosolutePercentageError ; 
						TotalMOEAbsError += fabs(LaneFlowError) ;
						TotaMOESampleSize ++;

					}

					if(g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsNumberOfVehicles > 0 && g_ODEstimationMeasurementType ==1)   // with density observations
					{

					int NumberOfVehicles  = g_LinkVector[li]->m_LinkMOEAry[time-g_ObservationTimeInterval].CumulativeArrivalCount - g_LinkVector[li]->m_LinkMOEAry[time-g_ObservationTimeInterval].CumulativeDepartureCount;
						g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuNumberOfVehicles  = NumberOfVehicles;
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ErrorNumberOfVehicles  = 
						g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuNumberOfVehicles - g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsNumberOfVehicles  ;

					
						float AbosolutePercentageError = fabs((g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ErrorNumberOfVehicles)/g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsNumberOfVehicles*100);
						float ObsDensity = (g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsNumberOfVehicles)/g_LinkVector[li]->m_NumLanes / g_LinkVector[li]->m_Length ;											
						float DensityError = (g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ErrorNumberOfVehicles)/g_LinkVector[li]->m_NumLanes / g_LinkVector[li]->m_Length ;											
						g_EstimationLogFile << "Link " << g_LinkVector[li]->m_FromNodeNumber << "->" << g_LinkVector[li]->m_ToNodeNumber 
							<< " @ "<< time << " Obs link density: "<< ObsFlowCount <<"; Error: " << DensityError << 
							", " << AbosolutePercentageError << " %" << "Density Error /h= " << DensityError << endl;

						TotalMOEPercentageError +=AbosolutePercentageError ; 
						TotalMOEAbsError += fabs(DensityError) ;
						TotaMOESampleSize ++;

					
					}

				if(g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsTravelTime  > 0 && g_ODEstimationMeasurementType == 2)  // with speed observations
					{ // not implemented yet. 
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuTravelTime    = g_LinkVector[li]->GetTravelTime(time-g_ObservationTimeInterval,1);
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ErrorTravelTime   = 
						g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuTravelTime - g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsTravelTime   ;
					}
					


				}
			}
		}	
	}
						if(TotaMOESampleSize > 0) 
							g_AssignmentLogFile << "Avg abs MOE error = " << TotalMOEAbsError /TotaMOESampleSize  << "Average Path flow Estimation MAPE = " << TotalMOEPercentageError / TotaMOESampleSize << " %" << endl;

}





void g_ExportLinkMOEToGroundTruthSensorData_ODEstimation()
{

	FILE* st = NULL;

	// write out the simulated data in SensorData format
	fopen_s(&st,"SensorDataDay000.csv","w");

	if(st!=NULL)
	{

		fprintf(st, "Unix Timestamp (local time),StationID,Total Flow_per_obs_interval,Avg Occupancy,Avg Speed\n");

	std::set<DTALink*>::iterator iterLink;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{	

		int LinkID = g_LinkVector[li]->m_LinkID ;

		map <long, long> :: iterator mIter  = g_LinkIDtoSensorIDMap.find(LinkID);

			if ( mIter != g_LinkIDtoSensorIDMap.end( ) )  // this link is a sensor
			{
			// compare the error statistics
			for(int time = 0; time< g_SimulationHorizon;time+=g_ObservationTimeInterval)
			{
				int time_index = time/g_ObservationTimeInterval;
				if(time_index >=1)
				{
					// CumulativeArrivalCount is available after this time interval, so we calculate statistics on time time_index-1
					int SimulatedFlowCount = g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount - g_LinkVector[li]->m_LinkMOEAry[time-g_ObservationTimeInterval].CumulativeArrivalCount; 
					int NumberOfVehicles  = g_LinkVector[li]->m_LinkMOEAry[time-g_ObservationTimeInterval].CumulativeArrivalCount - g_LinkVector[li]->m_LinkMOEAry[time-g_ObservationTimeInterval].CumulativeDepartureCount;
					float SimuTravelTime    = g_LinkVector[li]->GetTravelTime(time-g_ObservationTimeInterval,1);
					float SimulatedOccupancy = NumberOfVehicles/g_LinkVector[li]->m_Length / g_LinkVector[li]->m_NumLanes / 100;
					float SimulatedSpeed = g_LinkVector[li]->m_Length/ (max(0.001, SimuTravelTime)/60.0f);
					
					int hour = time/60;
					int min = time-hour*60;
		
					fprintf(st, "07/00/2010 %2d:%2d, %d, %d, %5.3f, %5.2f\n", hour, min, mIter->second, SimulatedFlowCount, SimulatedOccupancy,SimulatedSpeed);


				}
				}
			}	
	}
	fclose(st);

	}

}

