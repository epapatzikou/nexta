//  Portions Copyright 2010 Xuesong Zhou, Wen Deng

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
#include "GlobalData.h"

#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>


using namespace std;

void g_OutputMOEData(int iteration)
{	bool bStartWithEmptyFile = true;
OutputVehicleTrajectoryData("Vehicle.csv", iteration,true);

g_OutputDay2DayVehiclePathData("output_path.csv",0,10);

//OutputODMOEData("output_ODMOE.csv", iteration,true);
OutputEmissionData();

	int department_time_interval  = 60;  // min
	OutputTimeDependentODMOEData(department_time_interval);
//	OutputTimeDependentPathMOEData("TDPathMOE.csv", iteration,bStartWithEmptyFile);

	OutputLinkMOEData("output_LinkTDMOE.csv", iteration,bStartWithEmptyFile);
	OutputNetworkMOEData("output_NetworkTDMOE.csv", iteration,bStartWithEmptyFile);
}
void OutputODMOEData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty)
{

	ODStatistics*** ODMOEArray = NULL;

	int StatisticsIntervalSize = 1;  // we do not consider different time interval here

	ODMOEArray = Allocate3DDynamicArray<ODStatistics>(g_ODZoneSize+1,g_ODZoneSize+1,StatisticsIntervalSize);

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
		{
			int StatisticsInterval = 0;
			ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][StatisticsInterval].TotalVehicleSize+=1;
			ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][StatisticsInterval].TotalTravelTime += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
			ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][StatisticsInterval].TotalDistance += pVehicle->m_Distance;
		}
	}

	FILE* st = NULL;

	if(bStartWithEmpty)
		fopen_s(&st,fname,"w");
	else
		fopen_s(&st,fname,"a");

	if(st!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;

		if(bStartWithEmpty)
		{
			fprintf(st, "origin_zone_no,destination_zone_no,demand_type,information_type,number_of_vehicles_completing_trips,trip_time_in_min,cost_in_dollar,emissions\n");
		}

		for(int i = 1; i<=g_ODZoneSize; i++)
			for(int j = 1; j<=g_ODZoneSize; j++)
			{
				for(int t=0; t<StatisticsIntervalSize; t++)
				{
					if(ODMOEArray[i][j][t].TotalVehicleSize>=1)
					{
						fprintf(st, "%d,%d,0,0,%d,%5.2f,0,0\n", i,j, ODMOEArray[i][j][t].TotalVehicleSize, ODMOEArray[i][j][t].TotalTravelTime/ ODMOEArray[i][j][t].TotalVehicleSize);
					}
				}

			}
			fclose(st);
	}
	if(ODMOEArray !=NULL)
		Deallocate3DDynamicArray<ODStatistics>(ODMOEArray,g_ODZoneSize+1,g_ODZoneSize+1);

}

void OutputTimeDependentODMOEData(int department_time_intreval)
{
	// step 1: collect statistics based on vehicle file

	
	ofstream output_ODMOE_file, output_ODImpact_file;

	output_ODMOE_file.open ("output_ODMOE.csv");
//	output_ODImpact_file.open ("output_ImpactedOD.csv");

	if(output_ODMOE_file.is_open ())
	{
	output_ODMOE_file << "from_zone_id,to_zone_id,departure_time,number_of_vehicles,dayno,travel_time_in_min,distance_in_mile,#_of_diverted_vehicles,percentage_of_diversion,direrted_travel_time_in_min,direrted_distance_in_mile,percentage_of_non_diversion,non-direrted_travel_time_in_min,non-direrted_distance_in_mile" << endl;

	D2DODStatistics*** ODMOEArray = NULL;

	int StatisticsIntervalSize = (g_DemandLoadingEndTimeInMin - g_DemandLoadingStartTimeInMin)/department_time_intreval;

	ODMOEArray = Allocate3DDynamicArray<D2DODStatistics>(g_ODZoneSize+1,g_ODZoneSize+1,StatisticsIntervalSize);


	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
		{

			int StatisticsInterval = (pVehicle->m_DepartureTime)/department_time_intreval;

			if(StatisticsInterval >= StatisticsIntervalSize)  // avoid out of bound errors
				StatisticsInterval = StatisticsIntervalSize-1;

			ASSERT(pVehicle->m_OriginZoneID <= g_ODZoneSize);
			ASSERT(pVehicle->m_DestinationZoneID <= g_ODZoneSize);

			for(int DayNo = 0; DayNo <= g_NumberOfIterations; DayNo++)
			{

					ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][StatisticsInterval].D2DODStatisticsMap[DayNo].TotalVehicleSize+=1;
					ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][StatisticsInterval].D2DODStatisticsMap[DayNo].TotalTravelTime += pVehicle->Day2DayPathMap[DayNo].TravelTime ;
					ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][StatisticsInterval].D2DODStatisticsMap[DayNo].TotalDistance += pVehicle->Day2DayPathMap[DayNo].Distance;

					if( pVehicle->Day2DayPathMap[DayNo].bDiverted)
					{
					ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][StatisticsInterval].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize+=1;
					ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][StatisticsInterval].D2DODStatisticsMap[DayNo].Diverted_TotalTravelTime += pVehicle->Day2DayPathMap[DayNo].TravelTime ;
					ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][StatisticsInterval].D2DODStatisticsMap[DayNo].Diverted_TotalDistance += pVehicle->Day2DayPathMap[DayNo].Distance;
					}

			}
		}
	}



	// step 2: output od-based MOE statistics
		for(int i = 1; i<=g_ODZoneSize; i++)
			for(int j = 1; j<=g_ODZoneSize; j++)
				for(int t=0; t<StatisticsIntervalSize; t++)
				{
					for(int DayNo = 0; DayNo <= g_NumberOfIterations; DayNo++)
					if(ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].TotalVehicleSize>=1)
					{

						float avg_travel_time = ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].TotalTravelTime/ ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].TotalVehicleSize;
						float avg_travel_distance = ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].TotalDistance/ ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].TotalVehicleSize;

						ASSERT(avg_travel_time > -0.1f);
						output_ODMOE_file << i <<  "," << j << "," <<
							g_DemandLoadingStartTimeInMin + t*department_time_intreval << "," <<  ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].TotalVehicleSize <<  "," << 
							DayNo << "," <<  avg_travel_time  << "," << avg_travel_distance << "," ;

						if(ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize >=1)
						{
						float percentage = ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize*100.0f/ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].TotalVehicleSize;
						float diverted_avg_travel_time = ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].Diverted_TotalTravelTime/ max(1,ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize);
						float diverted_avg_travel_distance = ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].Diverted_TotalDistance/  max(1,ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize);

						output_ODMOE_file <<  ODMOEArray[i][j][t].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize <<  "," << 
							   percentage << "," << diverted_avg_travel_time<< "," << diverted_avg_travel_distance  << ",";
						}

						output_ODMOE_file << endl;
					}
				}


	if(output_ODImpact_file.is_open ())
	{
	
	}


	if(ODMOEArray !=NULL)
		Deallocate3DDynamicArray<D2DODStatistics>(ODMOEArray,g_ODZoneSize+1,g_ODZoneSize+1);

			output_ODMOE_file.close();
	}else
	{
		cout << "Error: File output_ODMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();	
	}


	// step 3: generate output_demand.csv for future simulation
/*
	fopen_s(&st,"output_demand.csv","w");

	if(st!=NULL)
	{

		fprintf(st, "from_zone_id, to_zone_id, starting_time_in_min,ending_time_in_min,number_of_vehicle_trips_type1,number_of_vehicle_trips_type2,number_of_vehicle_trips_type3\n");

		if(g_VehicleLoadingMode == 1) // load data form input_vehicle.csv
		{
			for(int i = 1; i<=g_ODZoneSize; i++)
				for(int j = 1; j<=g_ODZoneSize; j++)
					for(int t=0; t<StatisticsIntervalSize; t++)
					{
						if(ODMOEArray[i][j][t].TotalVehicleSize>=1)
						{
							fprintf(st, "%d,%d,%d,%d,%d,0,0,0\n", i,j,g_DemandLoadingStartTimeInMin,g_DemandLoadingStartTimeInMin + (t+1)*department_time_intreval, ODMOEArray[i][j][t].TotalVehicleSize);
						}
					}
		}else
		{
			// do nothing, so the output_demand.csv will be zero-out
		}
		fclose(st);
	}
*/

}

void OutputTimeDependentPathMOEData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty)
{

	ODPathSet*** ODPathSetVector = NULL;

	int StatisticsIntervalSize = (g_DemandLoadingEndTimeInMin - g_DemandLoadingStartTimeInMin)/30;  // 30 min aggregation interval

	ODPathSetVector = Allocate3DDynamicArray<ODPathSet>(g_ODZoneSize+1,g_ODZoneSize+1,StatisticsIntervalSize);

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

		int VehicleInWhichInterval=(pVehicle->m_DepartureTime-g_DemandLoadingStartTimeInMin)/30;

		if(VehicleInWhichInterval >= StatisticsIntervalSize)  // avoid out of bound errors
			VehicleInWhichInterval = StatisticsIntervalSize-1;

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
			ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID][VehicleInWhichInterval].PathSet.push_back(vehicle_PathStatistics);
		}
	}

	FILE* st = NULL;

	if(bStartWithEmpty)
		fopen_s(&st,fname,"w");
	else
		fopen_s(&st,fname,"a");

	if(st!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;

		if(bStartWithEmpty)
		{
			fprintf(st, "origin_zone_id, destination_zone_id, departure_time, path_No, NodeSum,demand_type, information_type, #_of_vehicles_completing_trips, trip_time_in_min, cost_in_dollar, emissions\n");
		}

		for(int i = 1; i<=g_ODZoneSize; i++)
			for(int j = 1; j<=g_ODZoneSize; j++)
				for(int t=0; t<StatisticsIntervalSize; t++)
				{
					int PathNo=1;
					for (std::vector<PathStatistics>::iterator IterPS=ODPathSetVector[i][j][t].PathSet.begin(); IterPS!=ODPathSetVector[i][j][t].PathSet.end();IterPS++)
					{
						if (IterPS->TotalVehicleSize>0)
						{
							//int vehicles_completing_trips=ODPathSetVector[i][j][t].PathSet[PathNo].TotalVehicleSize;
							//int Total_TravelTime=ODPathSetVector[i][j][t].PathSet[PathNo].TotalTravelTime;
							//int NodeSums=ODPathSetVector[i][j][t].PathSet[PathNo].NodeSums;
							int vehicles_completing_trips=IterPS->TotalVehicleSize;
							int Total_TravelTime=IterPS->TotalTravelTime;
							int NodeSums=IterPS->NodeSums;
							fprintf(st, "%d,%d,%d,%d,%d,0,0,%d,%5.2f,0,0\n", i,j,g_DemandLoadingStartTimeInMin+t*30,PathNo,NodeSums,vehicles_completing_trips,(float)Total_TravelTime/vehicles_completing_trips);
						}
						PathNo++;
					}
				}
				fclose(st);
	}
	if(ODPathSetVector!=NULL)
		Deallocate3DDynamicArray<ODPathSet>(ODPathSetVector,g_ODZoneSize+1,g_ODZoneSize+1);
}

