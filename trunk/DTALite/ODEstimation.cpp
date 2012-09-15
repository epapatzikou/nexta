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

int g_ODDemandIntervalSize = 1;
std::map<long, long> g_LinkIDtoSensorIDMap;

bool g_ReadLinkMeasurementFile()
{
	CCSVParser parser;
	int count = 0;
	int error_count = 0;

	if (parser.OpenCSVFile("input_sensor.csv"))
	{
		int sensor_count = 0;
		while(parser.ReadRecord())
		{

			int FromNodeNumber = 0; int ToNodeNumber = 0;

			if(!parser.GetValueByFieldName("from_node_id",FromNodeNumber )) 
				continue;
			if(!parser.GetValueByFieldName("to_node_id",ToNodeNumber )) 
				continue;

			if(g_LinkMap.find(GetLinkStringID(FromNodeNumber,ToNodeNumber))== g_LinkMap.end())
			{
				cout << "Link " << FromNodeNumber << "-> " << ToNodeNumber << " at line " << count+1 << " of file input_sensor.csv  has not been defined in input_link.csv. Please check." << endl;

				g_LogFile << "Link " << FromNodeNumber << "-> " << ToNodeNumber << " at line " << count+1 << " of file input_sensor.csv  has not been defined in input_link.csv. Please check." << endl;
				//if(error_count<=3)
				//{
				//	getchar();
				//}
				error_count ++;
				continue;
			}


			DTALink* pLink = g_LinkMap[GetLinkStringID(FromNodeNumber,ToNodeNumber)];

			if(pLink!=NULL)
			{

				int start_time_in_min = 0;
				int end_time_in_min  = 0;
				int volume_count = 0;

				parser.GetValueByFieldNameRequired("start_time_in_min",start_time_in_min );
				parser.GetValueByFieldNameRequired("end_time_in_min",end_time_in_min );

				parser.GetValueByFieldNameRequired ("volume_count",volume_count );

				float occupancy = 0;
				float avg_speed = 50;
				parser.GetValueByFieldName("occupancy",occupancy );
				parser.GetValueByFieldName("avg_speed",avg_speed );

				string name;
				parser.GetValueByFieldName("name",name );
				std::replace( name.begin(), name.end(), ',', ' '); 

				string direction;
				parser.GetValueByFieldName("direction",direction);
				std::replace( direction.begin(), direction.end(), ',', ' '); 

 
				pLink->m_bSensorData  = true;
				SLinkMeasurement element;
				element.name = name;
				element.direction = direction;
				element.StartTime = start_time_in_min;
				element.EndTime  = end_time_in_min;
				element.ObsFlowCount   = volume_count;
				element.ObsNumberOfVehicles = occupancy*100* pLink->m_Length * pLink->m_NumLanes;  // convert occupancy to density
				element.ObsTravelTime =  pLink->m_Length / max(1,avg_speed)*60;

				pLink->m_LinkMeasurementAry.push_back (element);
				count++;


			}else
			{
				cout << "Reading Error in input_sensor.dat. Link " << FromNodeNumber << " -> " << ToNodeNumber << " does not exist. "  << endl;
				g_EstimationLogFile << "Reading Error in input_sensor.dat. Link " << FromNodeNumber << " -> " << ToNodeNumber << " does not exist. "  << endl;
			}

		}

	}else
	{
		cout << "File input_sensor.dat does not exit or cannot be opened."  << endl;
		g_ProgramStop();
	

	}

			TCHAR ODMESettingFileName[_MAX_PATH] = _T("./ODME_Settings.txt");
			g_ODEstimationMeasurementType = g_GetPrivateProfileInt("estimation", "measurement_type", 1, ODMESettingFileName,true);	
			g_ODEstimation_StepSize = g_GetPrivateProfileFloat("estimation", "adjustment_step_size", 0.15, ODMESettingFileName,true);
			g_ODEstimation_WeightOnHistODDemand = g_GetPrivateProfileFloat("estimation", "weight_on_hist_oddemand", 1, ODMESettingFileName,true);
			g_ODEstimationStartTimeInMin = g_GetPrivateProfileInt("estimation", "estimation_start_time_in_min", 0, ODMESettingFileName,true);	
			g_ODEstimationEndTimeInMin = g_GetPrivateProfileInt("estimation", "estimation_end_time_in_min", 1440, ODMESettingFileName,true);	
			g_ODEstimation_WeightOnUEGap = g_GetPrivateProfileFloat("estimation", "weight_on_ue_gap", 1, ODMESettingFileName,true);
			g_ODEstimation_StartingIteration = g_GetPrivateProfileInt("estimation", "starting_iteration", 10, ODMESettingFileName,true);
		cout << "File input_sensor.csv has "<< count << " valid sensor records." << endl;
	g_LogFile << "Reading file input_sensor.csv with "<< count << " valid sensors." << endl;

			//cout << "DTALite will perform OD demand estimation, please review the above settings." << endl;
			//cout << "Please press 'n' if you want to exit and edit files ODME_Settings.txt and input_sensor.csv. Pleaes press the other key to continue." << endl;
			//char ret = getchar();
			//if(ret=='n')
			//	exit(0);

			//getchar(); // for return key
	return true;
}





void ConstructPathArrayForEachODT_ODEstimation(PathArrayForEachODT PathArray[], int zone, int AssignmentInterval)
{  // this function has been enhanced for path flow adjustment
	// step 1: initialization

	int CriticalOD_origin = 2;
	int CriticalOD_destination = 1;

	int DestZoneID; 
	for(DestZoneID=1; DestZoneID <= g_ODZoneSize; DestZoneID++) // initialization...
	{
		PathArray[DestZoneID].NumOfPaths = 0;
		PathArray[DestZoneID].NumOfVehicles = 0;
		PathArray[DestZoneID].DeviationNumOfVehicles = 0;
		PathArray[DestZoneID].BestPathIndex = 0;
		for(int p=0; p<_MAX_ODT_PATH_SIZE_4_ODME; p++)
		{
			PathArray[DestZoneID].NumOfVehsOnEachPath[p] = 0;
			PathArray[DestZoneID].PathNodeSums[p] = 0;
			PathArray[DestZoneID].AvgPathTimes[p] = 0.0;
			PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] = 0.0;
			PathArray[DestZoneID].PathSize[p] = 0;
			for(int q=0; q< _MAX_ODT_PATH_SIZE_4_ODME; q++)
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
				PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].NumOfPaths][i] = pVeh->m_NodeAry[i].LinkNo; 
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
					PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].NumOfPaths][i] = pVeh->m_NodeAry[i].LinkNo; 
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

		// if demand has not been assigned to this destination zone yet,skip

		float hist_demand = g_HistDemand.GetValue (zone, DestZoneID, AssignmentInterval);

		if(hist_demand < 0.001)  // only perform OD estimation when hist_demand  is positive
			continue; 

		PathArray[DestZoneID].DeviationNumOfVehicles = PathArray[DestZoneID].NumOfVehicles - hist_demand;

		if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination)
		{
			g_EstimationLogFile << "Critical OD demand " << zone << " -> " << DestZoneID <<  " with "<< PathArray[DestZoneID].NumOfPaths << " paths; Hist Demand =" << hist_demand << "; simu Demand = " << PathArray[DestZoneID].NumOfVehicles  << ";Dev = " << PathArray[DestZoneID].DeviationNumOfVehicles <<endl; 
		}


		for(int p=1; p<= PathArray[DestZoneID].NumOfPaths; p++)  // recall that path index start from 1 in this module
		{
			// step 4.2: assign the demand deviation as the initial value for path marginal 
			PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] = 0;
			// step 4.3: walk through the link sequence to accumlate the link marginals

			float ArrivalTime = AssignmentInterval * g_AggregationTimetInterval;

			if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination && ArrivalTime>=990)
			{
				g_EstimationLogFile << "Critical OD demand " << zone << " -> " << DestZoneID << "arrival time = "<< ArrivalTime << endl;
			}

			int l = 0;
			while(l>=0 && l< PathArray[DestZoneID].PathSize[p]-1)  // for each link along the path
			{
				int LinkID = PathArray[DestZoneID].PathLinkSequences[p][l];

				DTALink* pLink = g_LinkVector[LinkID];

						if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination && ArrivalTime>=990)
						{
							g_EstimationLogFile << "Link Index : " << l << "Critical OD demand " << zone << " -> " << DestZoneID << " passing link " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber << ", arrival time = "<< ArrivalTime << endl;
						}


					int ArrivalTime_int = (int)(ArrivalTime);

					if (ArrivalTime_int >= g_PlanningHorizon)  // time of interest exceeds the simulation horizon
						break;

				if(pLink->m_LinkMOEAry [(int)(ArrivalTime)].TrafficStateCode == 0) 					// if Arrival at Free-flow condition.
				{

					if(pLink->ContainFlowCount(ArrivalTime)) // with flow measurement
					{
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= pLink->GetDeviationOfFlowCount(ArrivalTime);

						if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination)
						{
							g_EstimationLogFile << "Critical OD demand " << zone << " -> " << DestZoneID << " passing link " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber 
								<< " Obs link flow: "<< pLink->GetObsFlowCount (ArrivalTime) <<", Simulated link flow: " << pLink->GetSimulatedFlowCount (ArrivalTime) << ", Measurement Error: " 
								<<  pLink->GetDeviationOfFlowCount(ArrivalTime)  << 
								", " <<   pLink->GetDeviationOfFlowCount(ArrivalTime)/max(1, pLink->GetObsFlowCount (ArrivalTime))*100 << " %, cumulative error: " <<  PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] << endl;

						}
					}

					ArrivalTime+= pLink->m_FreeFlowTravelTime ;
					l+= 1;  // move forward to the next link

				}else 		// if ArrivalTime is partially congested or fully congested,
				{


					TRACE("traffic state = %d \n",pLink->m_LinkMOEAry [ArrivalTime_int].TrafficStateCode);
					int timestamp_end_of_congestion = pLink->m_LinkMOEAry [(int)(ArrivalTime)].EndTimeOfPartialCongestion ;  // move to the end of congestion


					if(pLink->ContainFlowCount(ArrivalTime)) // with flow measurement
					{
						// flow rate gradient  // we assume the sensor is located upstream of the link, so we do not consider the departure flow rate increase at the downstream of the link
						PathArray[DestZoneID].MeasurementDeviationPathMarginal[p]+= pLink->GetDeviationOfFlowCount(ArrivalTime);

						if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination)
						{
							g_EstimationLogFile << "Critical OD demand " << zone << " -> " << DestZoneID << " passing link " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber 
								<< " Obs link flow: "<< pLink->GetObsFlowCount (ArrivalTime) <<", Simulated link flow: " << pLink->GetSimulatedFlowCount (ArrivalTime) << ", Measurement Error: " 
								<<  pLink->GetDeviationOfFlowCount(ArrivalTime)  << 
								", " <<   pLink->GetDeviationOfFlowCount(ArrivalTime)/max(1, pLink->GetObsFlowCount (ArrivalTime))*100 << " %, cumulative error: " <<  PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] << endl;

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


//				TRACE("link index l = %d, arrival time = %5.1f \n",l,ArrivalTime);
//				g_EstimationLogFile <<  "link index l = " << l << "arrival time = "<< ArrivalTime << endl;



			}

			#pragma omp critical  // protect global varaibles
			{
			// calculate the total demand deviation statistics only for the first path
			if(p==1) 
			{
				g_TotalDemandDeviation += PathArray[DestZoneID].DeviationNumOfVehicles;
						if(zone == CriticalOD_origin && DestZoneID == CriticalOD_destination)
						{
							g_EstimationLogFile << "Critical OD demand " << zone << " -> " << DestZoneID <<
								" TotalDemandDeviation = " <<  PathArray[DestZoneID].DeviationNumOfVehicles << endl;
						}
			}

			g_TotalMeasurementDeviation += PathArray[DestZoneID].MeasurementDeviationPathMarginal[p];

			// this is a very important path flow adjustment equation;

			float FlowAdjustment = 
				g_ODEstimation_WeightOnHistODDemand * PathArray[DestZoneID].DeviationNumOfVehicles /* gradient wrt target demand*/
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

			PathArrayForEachODTK element;
			int PathIndex  = g_ODTKPathVector.size();
			element.Setup (PathIndex,zone, DestZoneID, 1, AssignmentInterval *g_AggregationTimetInterval, (AssignmentInterval+1) *g_AggregationTimetInterval,
				PathArray[DestZoneID].PathSize[p]-1, PathArray[DestZoneID].PathLinkSequences[p],PathArray[DestZoneID].NewNumberOfVehicles[p],PathArray[DestZoneID].PathNodeSums[p] );
			g_ODTKPathVector.push_back(element);
			}

			//			g_AssignmentLogFile << "OED: O: " << zone << ", D:" << DestZoneID << "Demand Dev: " << PathArray[DestZoneID].DeviationNumOfVehicles << ", Path:" << p << ", marginal: " <<  PathArray[DestZoneID].MeasurementDeviationPathMarginal[p] << ", AvgTT: "<< PathArray[DestZoneID].AvgPathTimes[p]<< ", Gap : "<< PathArray[DestZoneID].AvgPathGap[p]<<  ", VehicleSize:" << PathArray[DestZoneID].NewNumberOfVehicles[p] << ", flow adjustment" << FlowAdjustment << endl;

		}   // for path p

	}

}


void DTANetworkForSP::VehicleBasedPathAssignment_ODEstimation(int zone,int departure_time_begin, int departure_time_end, int iteration)
// for vehicles starting from departure_time_begin to departure_time_end, assign them to shortest path using a proportion according to MSA or graident-based algorithms
{

	cout << " OD adjustment for zone " << zone << ", departure time " << departure_time_begin << " -> " << departure_time_end << endl;

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

//	g_EstimationLogFile << "g_GenerateVehicleData_ODEstimation "  <<endl; 

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
			CreateVehicles(element.m_OriginZoneID,element.m_DestinationZoneID ,element.m_VehicleSize ,element.m_DemandType,element.m_starting_time_in_min,element.m_ending_time_in_min,element.m_PathIndex,false);

		}
		// create vehicle heres...
//		g_EstimationLogFile << " Converting demand flow to vehicles... "  <<endl; 

		cout << "Converting demand flow to vehicles..."<< endl;

		std::sort(g_simple_vector_vehicles.begin(), g_simple_vector_vehicles.end());
//		g_EstimationLogFile << " std::sort... "  <<endl; 

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
				pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

				if(pVehicle->m_NodeAry==NULL)
				{
					cout << "Insufficient memory for allocating vehicle arrays!";
					g_ProgramStop();
				}

				pVehicle->m_NodeNumberSum =  element.m_NodeSum ;
				pVehicle->m_Distance =0;

				for(int j = 0; j< pVehicle->m_NodeSize-1; j++)
				{
					pVehicle->m_NodeAry[j].LinkNo = element.m_LinkNoArray[j];
					pVehicle->m_Distance+= g_LinkVector[pVehicle->m_NodeAry [j].LinkNo] ->m_Length ;
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





void g_UpdateLinkMOEDeviation_ODEstimation(NetworkLoadingOutput& output, int Iteration)
{
	//MAPE Mean absolute percentage error 
	//RMSE  root mean sequared error

//	g_EstimationLogFile << "--------------------------------- Iteration" << Iteration << " -----------------------------" << endl;
	float TotaMOESampleSize  = 0;
	float TotalFlowError = 0;
	float TotalFlowSequaredError = 0;

	float TotalMOEPercentageError = 0;
	float TotalMOEAbsError = 0;

	float TotalDensityError = 0;

	int number_of_sensors = 0;
					g_EstimationLogFile << "Iteration," << Iteration << "," << "direction" << ","<< "link name" << "," <<  "Link " << "from node" << ",->," << "to node" 
						<< ",time "<<  "start time in min" << "->" << "end time in min" <<  ",,"<< "observed link count" << "," << "simulated link count" <<", Error:, " << "Simulated flow count -  Obs flow count" << 
						"," << "Abosolute Percentage Error " << ",,Lane Flow Error /h=,Capacity ,obs voc,simu VOC" << endl;


					for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{	
		DTALink* pLink = g_LinkVector[li];
		if(g_TrafficFlowModelFlag !=0)  // dynamic  traffic assignment
		{

			// compare the error statistics
			for(unsigned int i = 0; i< pLink->m_LinkMeasurementAry.size(); i++)
			{

				if( pLink->m_LinkMeasurementAry[i].StartTime  < g_ODEstimationStartTimeInMin || pLink->m_LinkMeasurementAry[i].EndTime > g_ODEstimationEndTimeInMin)
					continue;

				// CumulativeArrivalCount is available after this time interval, so we calculate statistics on time time_index-1

				if(pLink->m_LinkMeasurementAry[i].EndTime <  pLink->m_LinkMOEAry.size())
				{
				int SimulatedFlowCount = pLink->m_LinkMOEAry[pLink->m_LinkMeasurementAry[i].EndTime].CumulativeArrivalCount - pLink->m_LinkMOEAry[pLink->m_LinkMeasurementAry[i].StartTime ].CumulativeArrivalCount; 

				if(SimulatedFlowCount>=0)
				{
				pLink->m_LinkMeasurementAry[i].SimuFlowCount = SimulatedFlowCount;


				int ObsFlowCount =  pLink->m_LinkMeasurementAry[i].ObsFlowCount;
				pLink->m_LinkMeasurementAry[i].DeviationOfFlowCount = SimulatedFlowCount -  pLink->m_LinkMeasurementAry[i].ObsFlowCount ;

				if(ObsFlowCount >= 1)  // flow count
				{
					int time_interval = pLink->m_LinkMeasurementAry[i].EndTime - pLink->m_LinkMeasurementAry[i].StartTime;
					float AbosolutePercentageError = abs((SimulatedFlowCount -  ObsFlowCount)*1.0f/ObsFlowCount*100);
					float LaneFlowError = (SimulatedFlowCount -  ObsFlowCount)*60.0f/
						max(1,time_interval)/pLink->m_NumLanes;												

					float obs_v_over_c_ratio = 0;
					float simu_over_c_ratio = 0;
					float link_capacity = pLink->GetNumLanes ()* pLink->m_LaneCapacity;

					if(link_capacity>1)
					{
					obs_v_over_c_ratio  = ObsFlowCount*60.0f/max(1,time_interval) /link_capacity;
					simu_over_c_ratio = SimulatedFlowCount*60.0f/max(1,time_interval) /link_capacity;
					}
					g_EstimationLogFile << "Iteration," << Iteration << "," << pLink->m_LinkMeasurementAry[i].name << "," << pLink->m_LinkMeasurementAry[i].direction  << ",Link " << pLink->m_FromNodeNumber << ",->," << pLink->m_ToNodeNumber 
						<< ",time "<<  pLink->m_LinkMeasurementAry[i].StartTime << "->" << pLink->m_LinkMeasurementAry[i].EndTime <<  ",Observed and simulated link count,"<< ObsFlowCount << "," << SimulatedFlowCount <<", Error:, " << SimulatedFlowCount -  ObsFlowCount << 
						"," << AbosolutePercentageError << " %" << ",Lane Flow Error /h=, " << LaneFlowError << "," << pLink->GetNumLanes ()* pLink->m_LaneCapacity << "," << obs_v_over_c_ratio << "," << simu_over_c_ratio << endl;

					TotalMOEPercentageError +=AbosolutePercentageError ; 
					TotalMOEAbsError += fabs(LaneFlowError) ;
					TotaMOESampleSize ++;

				}
				}
				}

			}
		}
	}	

	output.LinkVolumeAvgAbsPercentageError /=max(1,number_of_sensors);
	output.LinkVolumeAvgAbsError = TotalMOEAbsError /max(1,TotaMOESampleSize);
	float mean_sqared_error = output.LinkVolumeRootMeanSquaredError/max(1,number_of_sensors);
	output.LinkVolumeRootMeanSquaredError = sqrt(mean_sqared_error);

	if(TotaMOESampleSize > 0) 
		g_AssignmentLogFile << "Avg abs MOE error=, " << TotalMOEAbsError /max(1,TotaMOESampleSize)  << "Average Path flow Estimation MAPE =," << TotalMOEPercentageError / max(1,TotaMOESampleSize) << " %" << endl;

}



