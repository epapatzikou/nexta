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
#include "CSVParser.h"
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

float    g_ODEstimation_WeightOnHistODDemand = 0.1f;
float	 g_ODEstimation_WeightOnUEGap = 1.0f;

float    g_ODEstimation_Weight_Flow = 1.0f;
float    g_ODEstimation_Weight_NumberOfVehicles = 1.0f;
float    g_ODEstimation_Weight_TravelTime = 1.0f;
float    g_ODEstimation_StepSize  = 0.1f;

std::vector<PathArrayForEachODTK> g_ODTKPathVector;
int g_ODEstimationFlag = 0;
int g_Agent_shortest_path_generation_flag = 0;
int g_ODEstimationMeasurementType = 0; // 0: flow, 1: density, 2, speed
int g_ODEstimation_StartingIteration = 2;

float*** g_HistODDemand = NULL;

int g_ODDemandIntervalSize = 1;
std::map<long, long> g_LinkIDtoSensorIDMap;

bool g_ReadLinkMeasurementFile(DTANetworkForSP* pPhysicalNetwork)
{
	CCSVParser parser;
	int count = 0;

	if (parser.OpenCSVFile("input_sensor.csv"))
	{
	int sensor_count = 0;
		while(parser.ReadRecord())
		{

			int FromNodeNumber, ToNodeNumber;

			if(!parser.GetValueByFieldName("from_node_id",FromNodeNumber )) 
				return false;
			if(!parser.GetValueByFieldName("to_node_id",ToNodeNumber )) 
				return false;

			float AADT, peak_hour_factor;
			parser.GetValueByFieldName("AADT",AADT  );
			parser.GetValueByFieldName("peak_hour_factor",peak_hour_factor   );

			int LinkID = pPhysicalNetwork->GetLinkNoByNodeIndex(g_NodeNametoIDMap[FromNodeNumber], g_NodeNametoIDMap[ToNodeNumber]);

			DTALink* pLink = g_LinkVector[LinkID];


			if(pLink!=NULL)
			{
				pLink->m_bSensorData  = true;
				pLink->m_ObservedFlowVolume = AADT*peak_hour_factor;

			}else
			{
				cout << "Reading Error in input_sensor.dat. Link " << FromNodeNumber << " -> " << ToNodeNumber << " does not exist. "  << endl;
				g_LogFile << "Reading Error in input_sensor.dat. Link " << FromNodeNumber << " -> " << ToNodeNumber << " does not exist. "  << endl;
			}

		}

	}
	cout << "Reading file input_sensor.csv with "<< count << " valid sensors." << endl;
	g_LogFile << "Reading file input_sensor.csv with "<< count << " valid sensors." << endl;
 
/* read static information first
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

				DTALink* pLink = g_LinkVector[LinkID];

				if(pLink!=NULL)
				{
					int t  = max(0, (Hour*60+Min) );

					if(t >=  g_ObservationStartTime && t <=  g_ObservationEndTime)
					{
						t = t- g_ObservationStartTime;  // start from zero 
						int time_index = t / g_ObservationTimeInterval;
						pLink->m_LinkMeasurementAry[time_index].StartTime =  t ;
						pLink->m_LinkMeasurementAry[time_index].EndTime  = t + g_ObservationTimeInterval;

						pLink->m_LinkMeasurementAry[time_index].ObsFlowCount = TotalFlow;

						if(Speed>=1)
						{
							pLink->m_LinkMeasurementAry[time_index].ObsTravelTime = pLink->m_Length / max(1,Speed)*60;   // converted from speed, 60, hour to min

							// only process density data when speed data is available, to obtain reliable input
							float density;
							if(Occupancy <=0.001)
								density = TotalFlow*60/g_ObservationTimeInterval/pLink->m_NumLanes / max(1.0f,Speed);
							else
								density = Occupancy * 100;

							if(Occupancy>0.01) 
							{
								pLink->m_LinkMeasurementAry[time_index].ObsNumberOfVehicles = density * pLink->m_Length * pLink->m_NumLanes ;  // converted from density
							}

						}
						number_of_samples++;
					}
				}
			}
		}

		cout << "Reading file SensorDataDay001.csv with "<< number_of_samples << " valid samples." << endl;
		fclose(st);

*/
 return true;
}




void g_ODDemandEstimation()
{

	//	g_ODDemandProportion = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);
	//	g_ODDemandAdjustment = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);
	//+1 to make the OD zone index begin at 1

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

	int CriticalOD_origin = 191;
	int CriticalOD_destination = 21;

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

	   if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination)
	   {
		   g_EstimationLogFile << "Critical OD demand " << zone << " -> " << DestZoneID << " path " << p << " gap: " << PathArray[DestZoneID].AvgPathGap[p] << endl;
	   }

				// we calculate the gap for the previous iteraiton, for each path, path travel time gap * path flow
				g_CurrentGapValue += (PathArray[DestZoneID].AvgPathGap[p]* PathArray[DestZoneID].NumOfVehsOnEachPath[p]);

			}

		}
	}

	// step 4: update OD demand flow 
	for(DestZoneID=1; DestZoneID <= g_ODZoneSize; DestZoneID++)  //  for each OD pair
	{
		// step 4.1: calculate the vehicle size deviation for  each O D tau pair

// to by modified
//		float hist_demand = g_HistODDemand[zone][DestZoneID][AssignmentInterval];
// ToDo: consider AssignmentInterval as temporal profile in the next version. 		

		// if demand has not been assigned to this destination zone yet,skip
		if( g_ZoneMap[zone].m_HistDemand.find(DestZoneID) == g_ZoneMap[zone].m_HistDemand.end())
			continue; 

		float hist_demand = g_ZoneMap[zone].m_HistDemand[DestZoneID];

		if(hist_demand < 0.001)  // only perform OD estimation when hist_demand  is positive
			continue; 

		PathArray[DestZoneID].DeviationNumOfVehicles = PathArray[DestZoneID].NumOfVehicles - hist_demand;

	   if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination)
	   {
		g_EstimationLogFile << "Critical OD demand " << zone << " -> " << DestZoneID <<  "with "<< PathArray[DestZoneID].NumOfPaths << " paths; Hist Demand =" << hist_demand << "; simu Demand = " << PathArray[DestZoneID].NumOfVehicles  << ";Dev = " << PathArray[DestZoneID].DeviationNumOfVehicles <<endl; 
	   }




		for(int p=1; p<= PathArray[DestZoneID].NumOfPaths; p++)  // recall that path index start from 1 in this module
		{
			// step 4.2: assign the demand deviation as the initial value for path marginal 
			PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] = 0;
			// step 4.3: walk through the link sequence to accumlate the link marginals

			float ArrivalTime = AssignmentInterval * g_AggregationTimetInterval;
			int l = 0;
			while(l>=0 && l< PathArray[DestZoneID].PathSize[p]-1)  // for each link along the path
			{
				int LinkID = PathArray[DestZoneID].PathLinkSequences[p][l];

				DTALink* pLink = g_LinkVector[LinkID];

				//static: always reset ArrivalTime to zero.
				if(g_TrafficFlowModelFlag == 0)
				{
					if( pLink->m_bSensorData)  // with sensor data
					{
					
					PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+=  pLink->m_FlowMeasurementError;

					   if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination)
					   {
						g_EstimationLogFile << "Critical OD demand " << zone << " -> " << DestZoneID << " passing link " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber 
									<< " Obs link flow: "<< pLink->m_ObservedFlowVolume <<", Simulated link flow: "<< pLink->CFlowArrivalCount << ", Measurement Error: " << pLink->m_FlowMeasurementError  << 
									", " <<  pLink->m_FlowMeasurementError/max(1,pLink->m_ObservedFlowVolume)*100 << " %, cumulative error: " <<  PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] << endl;

					   }

					}

				}else  // time dependent flow rates. 
				{
				if(pLink->m_LinkMOEAry [(int)(ArrivalTime)].TrafficStateCode == 0) 					// if Arrival at Free-flow condition.
					{

						int obs_time_index = ArrivalTime/g_ObservationTimeInterval;  // convert arrival time to observation time index

						if(pLink->m_LinkMeasurementAry [obs_time_index].ObsFlowCount > 0 && g_ODEstimationMeasurementType ==0) // with flow measurement
						{
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= pLink->m_LinkMeasurementAry [obs_time_index].DeviationOfFlowCount;
						}

						if(pLink->m_LinkMeasurementAry [obs_time_index].ObsNumberOfVehicles  > 0 && g_ODEstimationMeasurementType ==1) // with density measurement
						{
						 PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= pLink->m_LinkMeasurementAry [obs_time_index].DeviationOfNumberOfVehicles;
						}

						if(pLink->m_LinkMeasurementAry [obs_time_index].DeviationOfTravelTime   > 0 && g_ODEstimationMeasurementType ==2) // with travel time measurement
						{
								PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= 0;   // not changing travel time under free-flow congestions 
						}

						ArrivalTime+= pLink->m_FreeFlowTravelTime ;
					}
					}
						l+= 1;  // move forward to the next link

						int ArrivalTime_int = (int)(ArrivalTime);

						if (ArrivalTime_int >= g_PlanningHorizon)  // time of interest exceeds the simulation horizon
							break;

						TRACE("traffic state = %d \n",pLink->m_LinkMOEAry [ArrivalTime_int].TrafficStateCode);
						if(pLink->m_LinkMOEAry [ArrivalTime_int].TrafficStateCode >0) 					// if ArrivalTime is partially congested or fully congested,
					{

						int obs_time_index = ArrivalTime/g_ObservationTimeInterval; // convert arrival time to observation time index
						int obs_time_index_for_end_of_congestion = pLink->m_LinkMOEAry [(int)(ArrivalTime)].EndTimeOfPartialCongestion/g_ObservationTimeInterval ;  // move to the end of congestion


						if(pLink->m_LinkMeasurementAry [obs_time_index].ObsFlowCount > 0 && g_ODEstimationMeasurementType ==0) // with flow measurement
						{
							// flow rate gradient  // we assume the sensor is located upstream of the link, so we do not consider the departure flow rate increase at the downstream of the link
							PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= pLink->m_LinkMeasurementAry [obs_time_index].DeviationOfFlowCount;

						}

							//density gradient
						if(pLink->m_LinkMeasurementAry [obs_time_index].ObsNumberOfVehicles  > 0 && g_ODEstimationMeasurementType ==1) // with density measurement
							{
								for(int obs_time_id = obs_time_index; obs_time_id < obs_time_index_for_end_of_congestion; obs_time_id++)
								{
									PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= pLink->m_LinkMeasurementAry [obs_time_id].DeviationOfNumberOfVehicles;
								}

							}

							// travel time gradient for travel time measurments: 
						if(pLink->m_LinkMeasurementAry [obs_time_index].ObsTravelTime   > 0 && g_ODEstimationMeasurementType ==2) // with travel time measurement
							{
								for(int obs_time_id = obs_time_index; obs_time_id < obs_time_index_for_end_of_congestion; obs_time_id++)
								{
									PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= 1.0/pLink->m_BPRLaneCapacity*60;   // 1/c as discharge rate, * 60 as min
								}
							}

						

						// move to the end of congestion
						int OldArrivalTime = (int)(ArrivalTime);
						ArrivalTime = pLink->m_LinkMOEAry [(int)(ArrivalTime)].EndTimeOfPartialCongestion ;  
						int NewArrivalTime = (int)(ArrivalTime);
						
						// remark: EndTimeOfPartialCongestion is like the end time of the current episo. 
						// arrval time is the arrival time at the exit queue


						if(pLink->m_LinkMOEAry [(int)(ArrivalTime)].TrafficStateCode == 0) // freeflow
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
			g_ODEstimation_WeightOnHistODDemand * PathArray[DestZoneID].DeviationNumOfVehicles /* gradient wrt targe demand*/
				+ PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] /* gradient wrt measurements (link incoming flow, density and travel time*/
			+ g_ODEstimation_WeightOnUEGap*PathArray[DestZoneID].AvgPathGap[p];


			float lower_bound_of_path_flow = max(1, PathArray[DestZoneID].NumOfVehsOnEachPath[p]*0.5);
			float upper_bound_of_path_flow_adjustment = PathArray[DestZoneID].NumOfVehsOnEachPath[p]*0.5;

			int sign_of_flow_adjustment  = 1;

			if(FlowAdjustment < 0)
				sign_of_flow_adjustment = -1;

			// restrict the flow adjustment per iteration, no more than 25% of previous path flow at the previous iteration
			if( fabs(FlowAdjustment) > upper_bound_of_path_flow_adjustment)
				FlowAdjustment = upper_bound_of_path_flow_adjustment*sign_of_flow_adjustment;

			// we do not want to change path flow too much from one iteration to another iteration
			// if path flow is reset to zero, then no vehicles will be simulated along this path, and then we lose this path in the remaining iterations.
			// we just want to keep the set of efficient paths stable

			PathArray[DestZoneID].NewNumberOfVehicles[p] = max(lower_bound_of_path_flow,
				PathArray[DestZoneID].NumOfVehsOnEachPath[p]   // this is the existing path flow volume
			- g_ODEstimation_StepSize*FlowAdjustment); /*gradient wrt gap function*/

	   if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination)
	   {
		   g_EstimationLogFile << "Critical OD demand " << zone << " -> " << DestZoneID << ", path" << p << ", existing # of vehicles:" << PathArray[DestZoneID].NumOfVehsOnEachPath[p] <<", flow adjustment: " <<FlowAdjustment << ",New number of vehicles ="<< PathArray[DestZoneID].NewNumberOfVehicles[p] << endl;
	   }

			
/*			g_EstimationLogFile << "OD " << zone << " -> " << DestZoneID << " path =" << p << " @ "<< AssignmentInterval*g_AggregationTimetInterval << ": Measurement Dev=" << PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] << 
				"; Demand Dev=" << PathArray[DestZoneID].DeviationNumOfVehicles <<
				"; Gap Dev "<< g_ODEstimation_WeightOnUEGap*PathArray[DestZoneID].AvgPathGap[p]  <<endl; 
*/

			if(zone==2 && DestZoneID == 32)
				TRACE("");

			PathArrayForEachODTK element;
			int PathIndex  = g_ODTKPathVector.size();
			element.Setup (PathIndex,zone, DestZoneID, 1, AssignmentInterval *g_AggregationTimetInterval, (AssignmentInterval+1) *g_AggregationTimetInterval,
				PathArray[DestZoneID].PathSize[p]-1, PathArray[DestZoneID].PathLinkSequences[p],PathArray[DestZoneID].NewNumberOfVehicles[p],PathArray[DestZoneID].PathNodeSums[p] );
			g_ODTKPathVector.push_back(element);


//			g_AssignmentLogFile << "OED: O: " << zone << ", D:" << DestZoneID << "Demand Dev: " << PathArray[DestZoneID].DeviationNumOfVehicles << ", Path:" << p << ", marginal: " <<  PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] << ", AvgTT: "<< PathArray[DestZoneID].AvgPathTimes[p]<< ", Gap : "<< PathArray[DestZoneID].AvgPathGap[p]<<  ", VehicleSize:" << PathArray[DestZoneID].NewNumberOfVehicles[p] << ", flow adjustment" << FlowAdjustment << endl;

		}   // for path p

	}


}


void DTANetworkForSP::VehicleBasedPathAssignment_ODEstimation(int zone,int departure_time_begin, int departure_time_end, int iteration)
// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
{

	int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
	std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
	int AssignmentInterval = int(departure_time_begin/g_AggregationTimetInterval);  // starting assignment interval

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

	g_FreeMemoryForVehicleVector();

	for(int z = 0; z < g_ODZoneSize+1; z++)
		for(int di = 0; di < g_AggregationTimetIntervalSize; di++)
		{
			g_TDOVehicleArray[z][di].VehicleArray.clear ();
		}

	



	PathArrayForEachODTK element;

	for(int vi = 0; vi< g_ODTKPathVector.size(); vi++)
	{
		element = g_ODTKPathVector[vi];
		CreateVehicles(element.m_OriginZoneID,element.m_DestinationZoneID ,element.m_VehicleSize ,element.m_DemandType,element.m_starting_time_in_min,element.m_ending_time_in_min,element.m_PathIndex );

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


			pVehicle->m_DemandType	= kvhc->m_DemandType;
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


			int AssignmentInterval = int(pVehicle->m_DepartureTime/g_AggregationTimetInterval);

			if(AssignmentInterval >= g_AggregationTimetIntervalSize)
			{
				AssignmentInterval = g_AggregationTimetIntervalSize - 1;
			}
			g_TDOVehicleArray[pVehicle->m_OriginZoneID][AssignmentInterval].VehicleArray .push_back(i);


		i++;
		}
		kvhc++;

	}
	g_simple_vector_vehicles.clear ();
}





void g_UpdateLinkMOEDeviation_ODEstimation(NetworkLoadingOutput& output)
{
//MAPE Mean absolute percentage error 
//RMSE  root mean sequared error

	float TotaMOESampleSize  = 0;
	float TotalFlowError = 0;
	float TotalFlowSequaredError = 0;

	float TotalMOEPercentageError = 0;
	float TotalMOEAbsError = 0;

	float TotalDensityError = 0;

	int number_of_sensors = 0;
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{	
		DTALink* pLink = g_LinkVector[li];
			if( pLink->m_bSensorData)
			{
				pLink->m_FlowMeasurementError  = pLink->CFlowArrivalCount - pLink->m_ObservedFlowVolume;
				 output.LinkVolumeAvgAbsError += fabs(pLink->m_FlowMeasurementError );
				 output.LinkVolumeRootMeanSquaredError += (pLink->m_FlowMeasurementError*pLink->m_FlowMeasurementError);

				 float percentage_error = fabs(pLink->m_FlowMeasurementError/max(1,pLink->m_ObservedFlowVolume)*100 );
				 if(pLink->m_ObservedFlowVolume <0.1f)  // for no volume links, reset the error percentage to 100%.
				 {
					percentage_error = 100;
				 }

				 output.LinkVolumeAvgAbsPercentageError +=  percentage_error;

				g_EstimationLogFile << "Link " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber 
					<< " Obs link flow: "<< pLink->m_ObservedFlowVolume <<", Simulated link flow: "<< pLink->CFlowArrivalCount << ", Measurement Error: " << pLink->m_FlowMeasurementError  << 
						", " <<  pLink->m_FlowMeasurementError/max(1,pLink->m_ObservedFlowVolume)*100 << " %" << endl;

				number_of_sensors ++;
			}
			

		if(g_TrafficFlowModelFlag !=0)  // dynamic  traffic assignment
		{

			// compare the error statistics
			for(int time = g_DemandLoadingStartTimeInMin; time< g_PlanningHorizon;time+=g_ObservationTimeInterval)
			{

				int time_index = time/g_ObservationTimeInterval;
				if(time_index >=1 && g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsFlowCount >0)
				{
					// CumulativeArrivalCount is available after this time interval, so we calculate statistics on time time_index-1
					int SimulatedFlowCount = g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount - g_LinkVector[li]->m_LinkMOEAry[time-g_ObservationTimeInterval].CumulativeArrivalCount; 
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuFlowCount = SimulatedFlowCount;
					


					float ObsFlowCount  = g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsFlowCount;
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].DeviationOfFlowCount = SimulatedFlowCount -  ObsFlowCount;

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
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].DeviationOfNumberOfVehicles  = 
						g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuNumberOfVehicles - g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsNumberOfVehicles  ;

					
						float AbosolutePercentageError = fabs((g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].DeviationOfNumberOfVehicles)/g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsNumberOfVehicles*100);
						float ObsDensity = (g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsNumberOfVehicles)/g_LinkVector[li]->m_NumLanes / g_LinkVector[li]->m_Length ;											
						float DensityError = (g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].DeviationOfNumberOfVehicles)/g_LinkVector[li]->m_NumLanes / g_LinkVector[li]->m_Length ;											
						g_EstimationLogFile << "Link " << g_LinkVector[li]->m_FromNodeNumber << "->" << g_LinkVector[li]->m_ToNodeNumber 
							<< " @ "<< time << " Obs link density: "<< ObsFlowCount <<"; Error: " << DensityError << 
							", " << AbosolutePercentageError << " %" << "Density Error /h= " << DensityError << endl;

						TotalMOEPercentageError +=AbosolutePercentageError ; 
						TotalMOEAbsError += fabs(DensityError) ;
						TotaMOESampleSize ++;

					
					}

				if(g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsTravelTime  > 0 && g_ODEstimationMeasurementType == 2)  // with speed observations
					{ // not implemented yet. 
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuTravelTime    = g_LinkVector[li]->GetTravelTimeByMin(time-g_ObservationTimeInterval,1);
					g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].DeviationOfTravelTime   = 
						g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].SimuTravelTime - g_LinkVector[li]->m_LinkMeasurementAry[time_index-1].ObsTravelTime   ;
					}
					


				}
			}
		}	
	}

		 output.LinkVolumeAvgAbsPercentageError /=max(1,number_of_sensors);
		 output.LinkVolumeAvgAbsError/=max(1,number_of_sensors);
		  float mean_sqared_error = output.LinkVolumeRootMeanSquaredError/max(1,number_of_sensors);
		 output.LinkVolumeRootMeanSquaredError = sqrt(mean_sqared_error);

						if(TotaMOESampleSize > 0) 
							g_AssignmentLogFile << "Avg abs MOE error = " << TotalMOEAbsError /TotaMOESampleSize  << "Average Path flow Estimation MAPE = " << TotalMOEPercentageError / TotaMOESampleSize << " %" << endl;

}



