//  Portions Copyright 2012 Xuesong Zhou

//   If you help write or modify the code, please also list your names here.
//   The reason of having copyright info here is to ensure all the modified version, as a whole, under the GPL 
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

// DTALite.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DTALite.h"
#include "Geometry.h"
#include "GlobalData.h"
#include "CSVParser.h"

#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>

using namespace std;
enum eSensivityTestMode {eSensivityDemand,eSensivityNumOfLanes,eSensivityToll};

	int CriticalLinkFromNodeNumberAry[4];
	int CriticalLinkToNodeNumberAry[4];

	int CriticalODFromZoneNumberAry[4];
	int CriticalODToZoneNumberAry[4];

	void g_UnitTestingTrafficAssignment(int begin_level  = 5, int end_level  = 30, int level_step = 5, int TotalUEIterationNumber = 20, 
									int TestDemandLevel = 80,int TestFromNode = 0, int TestToNode = 0, eSensivityTestMode SensivityTestMode = eSensivityDemand) 
{

	end_level = begin_level + (int)((end_level-begin_level)/max(1,level_step)) * level_step;
	int iteration;
	if(TotalUEIterationNumber == 0)
		g_UnitTestingLogFile << "*Setting: all shortest path, " << endl;
	else
		g_UnitTestingLogFile << "*Setting: Day-to-day learning ," << TotalUEIterationNumber << endl;

	g_UnitTestingLogFile  << " demand loading horizon: 7:00-8:00 " << endl ;

	int NumberOfCriticalLinks = 3;
	int NumberOfCriticalODPairs = 3;
	
	int cl;
	g_UnitTestingLogFile << ",,,,,,,,,,,,,";
	for( cl = 1; cl <= NumberOfCriticalLinks; cl++)
	{
		g_UnitTestingLogFile << "["<< cl << "]" 
					<< CriticalLinkFromNodeNumberAry[cl]<< "->" 
					<< CriticalLinkToNodeNumberAry[cl]<< ":" 
					<< ",,,,,,,,,,,,,,,,,";
	
	}

	int co;
	for( co = 1; co <= NumberOfCriticalODPairs; co++)
	{
		g_UnitTestingLogFile << "["<< co << "]" 
					<< CriticalODFromZoneNumberAry[co]<< "->" 
					<< CriticalODToZoneNumberAry[co]<< ":" 
					<< ",,";
	
	}

	g_UnitTestingLogFile <<endl;

	g_UnitTestingLogFile << ",,sensivity level, # of vehicles, Avg Travel Time (min), Avg Distance (miles), Avg Speed (mph),SOV TT,SOV Distance,HOV TT,HOV Distance,Truck TT,Truck Distance,";
	for( cl = 1; cl <= NumberOfCriticalLinks; cl++)
	{
		g_UnitTestingLogFile << "Critical Link ["<< cl << "]" 
					<< CriticalLinkFromNodeNumberAry[cl]<< "->" 
					<< CriticalLinkToNodeNumberAry[cl]<< ":" 
					<< " VOC,volume,ratio(link flow/total demand),HOV volume,HOV %,SOV volume,truck  volume,TT,speed,engery, CO2, NOX, HC, AADT,#OfCrashes,#OfFatalAndInjuryCrashes,#OfPDOCrashes,";
	}
				g_UnitTestingLogFile	<<  endl;

	for(iteration = begin_level; iteration <= end_level; iteration += level_step)
	{
	if(g_InitializeLogFiles()==0) 
	return;

	g_ReadDTALiteSettings();

	g_TrafficFlowModelFlag = 3;  		// Newell's traffic flow model
	g_EmissionDataOutputFlag = 0; 		// do not output emissions
	g_ODEstimationFlag = 0; 			// no OD estimation
	g_NumberOfIterations = TotalUEIterationNumber;			// 0+1 iterations
	g_AgentBasedAssignmentFlag = 0;		// no agent-based assigment 


	if(SensivityTestMode == eSensivityDemand)
	{
	g_DemandGlobalMultiplier =  0.01 * iteration;	// very low demand    
	}else
	{
	g_DemandGlobalMultiplier =  0.01 * TestDemandLevel;
	}


	g_SimulationOutput2UnitTesting = false;

	g_ReadInputFiles();
	
	if(SensivityTestMode == eSensivityNumOfLanes)
	{
		g_SetLinkAttributes(TestFromNode,TestToNode,iteration);
	}

	if(SensivityTestMode == eSensivityToll)
		{
		
		Toll toll;

		toll.DayNo = 0;
		toll.StartTime = 0;
		toll.EndTime = 1440;
		toll.TollRate[1] = iteration*0.01;  // different toll values for SOV here
		toll.TollRate[2] = 0;
		toll.TollRate[3] = 999;
		toll.TollRate[4] = 0;

		g_SetLinkTollValue(TestFromNode,TestToNode,toll);
		g_CreateLinkTollVector();

		}



	cout << "Start Traffic Assignment/Simulation... " << endl;

	if(g_AgentBasedAssignmentFlag==0)
		{
			cout << "OD Demand based dynamic traffic assignment... " << endl;
			g_ODBasedDynamicTrafficAssignment(); // multi-iteration dynamic traffic assignment
		}
		else{
			cout << "Agent based dynamic traffic assignment... " << endl;
			g_AgentBasedAssisnment();  // agent-based assignment
		}


	g_OutputSimulationStatistics();

		g_UnitTestingLogFile <<",," << iteration << "," << 
		g_SimulationResult.number_of_vehicles << "," <<
		g_SimulationResult.avg_travel_time_in_min << "," <<
		g_SimulationResult.avg_distance_in_miles << "," << 
		g_SimulationResult.avg_speed << "," ;

	int Count=0; 
	float AvgTravelTime, AvgDistance, AvgSpeed;


	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, 1, -1);
	
	g_UnitTestingLogFile << AvgTravelTime << "," << AvgDistance << "," ;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, 2, -1);
	int HOVCount = Count;
	g_UnitTestingLogFile << AvgTravelTime << "," << AvgDistance << " ," ;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, 3, -1);

	g_UnitTestingLogFile << AvgTravelTime << "," << AvgDistance <<  " ," ;


	for( cl = 1; cl <= NumberOfCriticalLinks; cl++)
	{
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			DTALink* pLink = g_LinkVector[li];

			if(pLink->m_FromNodeNumber == CriticalLinkFromNodeNumberAry[cl] && pLink->m_ToNodeNumber == CriticalLinkToNodeNumberAry[cl])
			{
			double average_travel_time = pLink->GetTravelTimeByMin(0, pLink->m_SimulationHorizon);
			double total_volume = pLink->CFlowArrivalCount;
			float FlowRatio = pLink->CFlowArrivalCount*100.0f/g_SimulationResult.number_of_vehicles;
			float HOV_volume = pLink->CFlowArrivalCount_PricingType[2];
			float HOVRatio = pLink->CFlowArrivalCount_PricingType[2]*100.0f/max(1,total_volume);
			float SOV_volume = pLink->CFlowArrivalCount_PricingType[1];
			float Truck_volume = pLink->CFlowArrivalCount_PricingType[3];

			double speed = pLink->m_Length / max(0.00001,average_travel_time) *60;  // unit: mph
			double capacity_simulation_horizon = pLink->m_MaximumServiceFlowRatePHPL * pLink->m_NumLanes * (g_DemandLoadingEndTimeInMin- g_DemandLoadingStartTimeInMin) / 60;
			double voc_ratio = pLink->CFlowArrivalCount / max(0.1,capacity_simulation_horizon);
			int percentage_of_speed_limit = int(speed/max(0.1,pLink->m_SpeedLimit)*100+0.5);
			g_UnitTestingLogFile << voc_ratio << "," << total_volume << "," << FlowRatio << "," << HOV_volume << "," << HOVRatio << "," << SOV_volume << "," << Truck_volume << ","
				<< average_travel_time << "," << speed << ",";
			g_UnitTestingLogFile <<	pLink->m_TotalEnergy  << "," << pLink->m_CO2 << ","  << pLink->m_NOX << "," << pLink->m_HC << ",";
			g_UnitTestingLogFile <<	pLink->m_AADT  << "," << pLink->m_NumberOfCrashes << ","  << pLink->m_NumberOfFatalAndInjuryCrashes << "," << pLink->m_NumberOfPDOCrashes << ",";
			}
		}

	}

	ODPathSet*** ODPathSetVector = NULL;

	int StatisticsIntervalSize = 1;
	ODPathSetVector = Allocate3DDynamicArray<ODPathSet>(g_ODZoneSize+1,g_ODZoneSize+1,StatisticsIntervalSize);

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

		int VehicleInWhichInterval= 0;

		std::vector<PathStatistics> Vehicle_ODTPathSet;

		Vehicle_ODTPathSet=ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet;

		bool ExistPath_Flag=false;
		int PathNo=0;
		for (std::vector<PathStatistics>::iterator IterPS=Vehicle_ODTPathSet.begin();IterPS!=Vehicle_ODTPathSet.end();IterPS++)
		{
			if (pVehicle->m_NodeNumberSum==IterPS->NodeSums) //existing path
			{
				ExistPath_Flag=true;
				break;
			}
			else                                             //New Path;
				ExistPath_Flag=false;
			PathNo++;
		}

			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].TotalVehicleSize+=1;
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].TotalDistance+=pVehicle->m_Distance;
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].TotalTravelTime+=(pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);

		if (ExistPath_Flag)				// if old path, add statistics
		{
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet[PathNo].TotalVehicleSize+=1;
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet[PathNo].TotalDistance+=pVehicle->m_Distance;
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet[PathNo].TotalTravelTime+=(pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
		}
		else				           // if new path, add
		{
			PathStatistics vehicle_PathStatistics;
			vehicle_PathStatistics.TotalVehicleSize=1;
			vehicle_PathStatistics.TotalDistance=pVehicle->m_Distance;
			vehicle_PathStatistics.TotalTravelTime=(pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
			vehicle_PathStatistics.NodeSums=pVehicle->m_NodeNumberSum;

			// add node id along the path
				int NodeID = g_LinkVector[pVehicle->m_aryVN [0].LinkID]->m_FromNodeID;  // first node
				int NodeName = g_NodeVector[NodeID].m_NodeName ;
				vehicle_PathStatistics.m_NodeNameArray .push_back (NodeName);
				for(int j = 0; j< pVehicle->m_NodeSize-1; j++)
				{
					int LinkID = pVehicle->m_aryVN [j].LinkID;
					NodeID = g_LinkVector[LinkID]->m_ToNodeID;
					NodeName = g_NodeVector[NodeID].m_NodeName ;
					vehicle_PathStatistics.m_NodeNameArray .push_back (NodeName);
				}
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet.push_back(vehicle_PathStatistics);

		}
	}

	int co;
	for( co = 1; co <= NumberOfCriticalODPairs; co++)
	{
		float avg_travel_time  = 0;
		float avg_travel_distance = 0;
		if(CriticalODFromZoneNumberAry[co] <=g_ODZoneSize && CriticalODToZoneNumberAry[co] <=g_ODZoneSize )
		{
		avg_travel_time = ODPathSetVector[CriticalODFromZoneNumberAry[co]][CriticalODToZoneNumberAry[co]][0].TotalTravelTime / 
			max(1,ODPathSetVector[CriticalODFromZoneNumberAry[co]][CriticalODToZoneNumberAry[co]][0].TotalVehicleSize );

		avg_travel_distance = ODPathSetVector[CriticalODFromZoneNumberAry[co]][CriticalODToZoneNumberAry[co]][0].TotalDistance  / 
			max(1,ODPathSetVector[CriticalODFromZoneNumberAry[co]][CriticalODToZoneNumberAry[co]][0].TotalVehicleSize );
		}

		g_UnitTestingLogFile << avg_travel_time << "," << avg_travel_distance << ",";
	}

					
	if(ODPathSetVector!=NULL)
		Deallocate3DDynamicArray<ODPathSet>(ODPathSetVector,g_ODZoneSize+1,g_ODZoneSize+1);

	g_UnitTestingLogFile << endl;

			// output general link statistics
		if(iteration == end_level )
		{
	std::set<DTALink*>::iterator iterLink;
	g_UnitTestingLogFile << "--- Link MOE ---" << endl;

	g_UnitTestingLogFile << "from_node_id,to_node_id,start_time_in_min,end_time_in_min,#_of_lanes, FFTT,Travel Time,total_link_volume,ratio,HOV_volume,HOV perc,SOV perc,Truck perc,lane_capacity_in_vhc_per_hour, volume_over_capacity_ratio, speed_limit_in_mph, speed_in_mph, percentage_of_speed_limit, level_of_service,#OfCrashes,#OfFatalAndInjuryCrashes,#OfPDOCrashes" << endl;

//		DTASafetyPredictionModel SafePredictionModel;
//		SafePredictionModel.UpdateCrashRateForAllLinks();

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			DTALink* pLink = g_LinkVector[li];
			double average_travel_time = pLink->GetTravelTimeByMin(0, pLink->m_SimulationHorizon);
			double speed = pLink->m_Length / max(0.00001,average_travel_time) *60;  // unit: mph
			double capacity_simulation_horizon = pLink->m_MaximumServiceFlowRatePHPL * pLink->m_NumLanes * (g_DemandLoadingEndTimeInMin- g_DemandLoadingStartTimeInMin) / 60;
			double voc_ratio = pLink->CFlowArrivalCount / max(0.1,capacity_simulation_horizon);
			int percentage_of_speed_limit = int(speed/max(0.1,pLink->m_SpeedLimit)*100+0.5);


				g_UnitTestingLogFile << g_NodeVector[pLink->m_FromNodeID].m_NodeName  << "," ;
				g_UnitTestingLogFile << g_NodeVector[pLink->m_ToNodeID].m_NodeName << "," ;
				g_UnitTestingLogFile << g_DemandLoadingStartTimeInMin << "," ;
				g_UnitTestingLogFile << g_DemandLoadingEndTimeInMin << "," ;
				g_UnitTestingLogFile << pLink->m_NumLanes    << "," ;
				g_UnitTestingLogFile << pLink->m_FreeFlowTravelTime   << "," ;
				g_UnitTestingLogFile << average_travel_time  << "," ;
				g_UnitTestingLogFile << pLink->CFlowArrivalCount << "," ; // total hourly arrival flow 
				float FlowRatio = pLink->CFlowArrivalCount*100.0f/g_SimulationResult.number_of_vehicles;
				g_UnitTestingLogFile << FlowRatio << "," ;
				g_UnitTestingLogFile << pLink->CFlowArrivalCount_PricingType [2] << "," ;
				g_UnitTestingLogFile << pLink->CFlowArrivalCount_PricingType [2]*100.0f/max(1,pLink->CFlowArrivalCount) << "," ;
				g_UnitTestingLogFile << pLink->CFlowArrivalCount_PricingType [1]*100.0f/max(1,pLink->CFlowArrivalCount) << "," ;
				g_UnitTestingLogFile << pLink->CFlowArrivalCount_PricingType [3]*100.0f/max(1,pLink->CFlowArrivalCount) << "," ;
				g_UnitTestingLogFile << pLink->m_MaximumServiceFlowRatePHPL << "," ;
				g_UnitTestingLogFile << voc_ratio << "," ;
				g_UnitTestingLogFile << pLink->CFlowArrivalCount_PricingType [2] << "," ;
				g_UnitTestingLogFile << pLink->m_SpeedLimit << "," ;
				g_UnitTestingLogFile << speed << "," ;
				g_UnitTestingLogFile << percentage_of_speed_limit << "," ;
				g_UnitTestingLogFile << g_GetLevelOfService(percentage_of_speed_limit) << "," ;
				g_UnitTestingLogFile <<	pLink->m_NumberOfCrashes << ","  ;
				g_UnitTestingLogFile <<	pLink->m_NumberOfFatalAndInjuryCrashes << "," ;
				g_UnitTestingLogFile <<	pLink->m_NumberOfPDOCrashes << ",";

				g_UnitTestingLogFile << endl;

		}
	g_UnitTestingLogFile << " ---End of Link MOE ---" << endl << endl;

	g_UnitTestingLogFile << "--- Path MOE ---" << endl;

	ODPathSet*** ODPathSetVector = NULL;

	int StatisticsIntervalSize = 1;
	ODPathSetVector = Allocate3DDynamicArray<ODPathSet>(g_ODZoneSize+1,g_ODZoneSize+1,StatisticsIntervalSize);

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

		int VehicleInWhichInterval= 0;

		std::vector<PathStatistics> Vehicle_ODTPathSet;

		Vehicle_ODTPathSet=ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet;

		bool ExistPath_Flag=false;
		int PathNo=0;
		for (std::vector<PathStatistics>::iterator IterPS=Vehicle_ODTPathSet.begin();IterPS!=Vehicle_ODTPathSet.end();IterPS++)
		{
			if (pVehicle->m_NodeNumberSum==IterPS->NodeSums) //existing path
			{
				ExistPath_Flag=true;
				break;
			}
			else                                             //New Path;
				ExistPath_Flag=false;
			PathNo++;
		}

		if (ExistPath_Flag)				// if old path, add statistics
		{
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet[PathNo].TotalVehicleSize+=1;
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet[PathNo].TotalDistance+=pVehicle->m_Distance;
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet[PathNo].TotalTravelTime+=(pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
		}
		else				           // if new path, add
		{
			PathStatistics vehicle_PathStatistics;
			vehicle_PathStatistics.TotalVehicleSize=1;
			vehicle_PathStatistics.TotalDistance=pVehicle->m_Distance;
			vehicle_PathStatistics.TotalTravelTime=(pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
			vehicle_PathStatistics.NodeSums=pVehicle->m_NodeNumberSum;

			// add node id along the path
				int NodeID = g_LinkVector[pVehicle->m_aryVN [0].LinkID]->m_FromNodeID;  // first node
				int NodeName = g_NodeVector[NodeID].m_NodeName ;
				vehicle_PathStatistics.m_NodeNameArray .push_back (NodeName);
				for(int j = 0; j< pVehicle->m_NodeSize-1; j++)
				{
					int LinkID = pVehicle->m_aryVN [j].LinkID;
					NodeID = g_LinkVector[LinkID]->m_ToNodeID;
					NodeName = g_NodeVector[NodeID].m_NodeName ;
					vehicle_PathStatistics.m_NodeNameArray .push_back (NodeName);
				}
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet.push_back(vehicle_PathStatistics);

		}
	}

		g_UnitTestingLogFile << "origin_zone_id, destination_zone_id, departure_time, path_No, ratio, NodeSum, node sequence, #_of_vehicles_completing_trips, trip_time_in_min " << endl;

		for(int i = 1; i<=g_ODZoneSize; i++)
			for(int j = 1; j<=g_ODZoneSize; j++)
				{
					int t = 0;
					int PathNo=1;
					for (std::vector<PathStatistics>::iterator IterPS=ODPathSetVector[i][j][t].PathSet.begin(); IterPS!=ODPathSetVector[i][j][t].PathSet.end();IterPS++)
					{
						if (IterPS->TotalVehicleSize>=50)
						{
							//int vehicles_completing_trips=ODPathSetVector[i][j][t].PathSet[PathNo].TotalVehicleSize;
							//int Total_TravelTime=ODPathSetVector[i][j][t].PathSet[PathNo].TotalTravelTime;
							//int NodeSums=ODPathSetVector[i][j][t].PathSet[PathNo].NodeSums;
							int vehicles_completing_trips=IterPS->TotalVehicleSize;
							int Total_TravelTime=IterPS->TotalTravelTime;
							int NodeSums=IterPS->NodeSums;
							float FlowRatio =vehicles_completing_trips*100.0f/g_SimulationResult.number_of_vehicles;

							g_UnitTestingLogFile << i << "," << j<< "," << g_DemandLoadingStartTimeInMin+t*30 << ",";
							g_UnitTestingLogFile <<  PathNo << "," << FlowRatio << "," <<  NodeSums << ",";

							for(int n = 0; n < IterPS->m_NodeNameArray .size(); n++)
							{
								g_UnitTestingLogFile << IterPS->m_NodeNameArray[n];
								
								if(n!=  IterPS->m_NodeNameArray .size()-1)
									g_UnitTestingLogFile << "->" ;
							
							}

							g_UnitTestingLogFile << "," << vehicles_completing_trips << "," << Total_TravelTime*1.0f/vehicles_completing_trips<< endl;
						}
						PathNo++;
					}
				}
					
	if(ODPathSetVector!=NULL)
		Deallocate3DDynamicArray<ODPathSet>(ODPathSetVector,g_ODZoneSize+1,g_ODZoneSize+1);

		g_UnitTestingLogFile << " ---End of Path MOE ---" << endl << endl;
	}

	g_FreeMemory();
	}

		g_UnitTestingLogFile << "---------,--------" << endl;

}

void g_DTALiteUnitTestingMain()
{
//	g_UnitTestingTrafficAssignment(0);

//	g_UnitTestingTrafficAssignment(20);

	// case 1: shortest path
//	g_UnitTestingTrafficAssignment(20,40,5,0);


	// case 2: user equilibirum
//	g_UnitTestingTrafficAssignment(20,80,5,20);

	CriticalLinkFromNodeNumberAry[1] = 1;
	CriticalLinkToNodeNumberAry[1] = 5;

	CriticalLinkFromNodeNumberAry[2] =  5;
	CriticalLinkToNodeNumberAry[2] = 7;

	CriticalLinkFromNodeNumberAry[3] =  1;
	CriticalLinkToNodeNumberAry[3] = 12;
	// case 3: change link attributes 
	//	WritePrivateProfileString("input_file","link_data","input_link_road_diet.csv", g_DTASettingFileName);

	CriticalLinkFromNodeNumberAry[1] = 201;
	CriticalLinkToNodeNumberAry[1] = 202;

	CriticalLinkFromNodeNumberAry[2] = 30;
	CriticalLinkToNodeNumberAry[2] = 25;

	CriticalLinkFromNodeNumberAry[3] =  202;
	CriticalLinkToNodeNumberAry[3] = 21;

	//
	CriticalODFromZoneNumberAry[1] = 1;
	CriticalODToZoneNumberAry[1] = 5;

	CriticalODFromZoneNumberAry[2] = 1;
	CriticalODToZoneNumberAry[2] = 2;

	CriticalODFromZoneNumberAry[3] =  1;
	CriticalODToZoneNumberAry[3] = 2;

	eSensivityTestMode test_mode = eSensivityDemand;

	int begin_level  = 25;
	int end_level  = 100;
	int level_step = 5;
	int TotalUEIterationNumber = 20;
	int TestDemandLevel = 25;
	int TestFromNode = 201;
	int TestToNode = 202;
	eSensivityTestMode SensivityTestMode = eSensivityDemand;

	g_UnitTestingTrafficAssignment(begin_level,end_level,level_step,TotalUEIterationNumber,
		TestDemandLevel,TestFromNode,TestToNode,SensivityTestMode);


	 begin_level  = 0;
	 end_level  = 5;
	 level_step = 1;
	 TotalUEIterationNumber = 60;
	 TestDemandLevel = 25;
	 TestFromNode = 201;
	 TestToNode = 202;
	 SensivityTestMode = eSensivityToll;

	g_DemandLoadingStartTimeInMin = 6*60;	
	g_DemandLoadingEndTimeInMin = 8*60;	

//	g_UnitTestingTrafficAssignment(begin_level,end_level,level_step,TotalUEIterationNumber,
//		TestDemandLevel,TestFromNode,TestToNode,SensivityTestMode);

//	g_UnitTestingTrafficAssignment(1,10,1,20,50,1,5,eSensivityNumOfLanes);

	// case 4: test toll sensitivity
//	g_UnitTestingHOTStrategy(0,200,25,220,20);


}
