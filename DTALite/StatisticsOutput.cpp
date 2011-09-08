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
class ODStatistics
{
public: 
	ODStatistics()
	{
	TotalVehicleSize = 0;
	TotalTravelTime = 0;
	TotalDistance = 0;
	TotalCost = 0;
	TotalEmissions = 0;
	}

	int   TotalVehicleSize;
	float TotalTravelTime;
	float TotalDistance;
	float TotalCost;
	float TotalEmissions;

};


class PathStatistics
{
public: 
	PathStatistics()
	{
	TotalVehicleSize = 0;
	TotalTravelTime = 0;
	TotalDistance = 0;
	TotalCost = 0;
	TotalEmissions = 0;
	}

	int   NodeSums;

	int m_LinkSize;
	std::vector<int> m_LinkNoArray;

	int   TotalVehicleSize;
	float TotalTravelTime;
	float TotalDistance;
	float TotalCost;
	float TotalEmissions;

};

class ODPathSet
{
public: 
	std::vector<PathStatistics> PathSet;

};


void g_OutputMOEData(int iteration)
{	bool bStartWithEmptyFile = true;
	OutputVehicleTrajectoryData("Vehicle.csv", iteration,true);
	OutputODMOEData("ODMOE.csv", iteration,true);
	OutputTimeDependentODMOEData("TDODMOE.csv", iteration,true);
	OutputTimeDependentPathMOEData("TDPathMOE.csv", iteration,bStartWithEmptyFile);

	OutputLinkMOEData("LinkMOE.csv", iteration,bStartWithEmptyFile);
	OutputNetworkMOEData("NetworkMOE.csv", iteration,bStartWithEmptyFile);
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
			fprintf(st, "origin_zone_id, destination_zone_id, vehicle_type, information_type, #_of_vehicles_completing_trips, trip_time_in_min, cost_in_dollar, emissions\n");
		}

		for(int i = 1; i<=g_ODZoneSize; i++)
		for(int j = 1; j<=g_ODZoneSize; j++)
			for(int t=0; t<StatisticsIntervalSize; t++)
			{
			if(ODMOEArray[i][j][t].TotalVehicleSize>=1)
			{
			fprintf(st, "%d,%d,0,0,%d,%5.2f,0,0\n", i,j, ODMOEArray[i][j][t].TotalVehicleSize, ODMOEArray[i][j][t].TotalTravelTime/ ODMOEArray[i][j][t].TotalVehicleSize);
			}
			}


		fclose(st);
	}
	if(ODMOEArray !=NULL)
	Deallocate3DDynamicArray<ODStatistics>(ODMOEArray,g_ODZoneSize+1,g_ODZoneSize+1);

}

void OutputTimeDependentODMOEData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty)
{

	ODStatistics*** ODMOEArray = NULL;

	int StatisticsIntervalSize = g_DemandLoadingHorizon/15;

	ODMOEArray = Allocate3DDynamicArray<ODStatistics>(g_ODZoneSize+1,g_ODZoneSize+1,StatisticsIntervalSize);

		std::map<int, DTAVehicle*>::iterator iterVM;
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
			{

				int StatisticsInterval = pVehicle->m_DepartureTime/15;


				if(StatisticsInterval >= StatisticsIntervalSize)  // avoid out of bound errors
					StatisticsInterval = StatisticsIntervalSize-1;

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
			fprintf(st, "origin_zone_id, destination_zone_id, departure_time, vehicle_type, information_type, #_of_vehicles_completing_trips, trip_time_in_min, cost_in_dollar, emissions\n");
		}

		for(int i = 1; i<=g_ODZoneSize; i++)
		for(int j = 1; j<=g_ODZoneSize; j++)
			for(int t=0; t<StatisticsIntervalSize; t++)
			{
			if(ODMOEArray[i][j][t].TotalVehicleSize>=1)
			{
			fprintf(st, "%d,%d,%d,0,0,%d,%5.2f,0,0\n", i,j,t*15, ODMOEArray[i][j][t].TotalVehicleSize, ODMOEArray[i][j][t].TotalTravelTime/ ODMOEArray[i][j][t].TotalVehicleSize);
			}
			}


		fclose(st);
	}
	if(ODMOEArray !=NULL)
	Deallocate3DDynamicArray<ODStatistics>(ODMOEArray,g_ODZoneSize+1,g_ODZoneSize+1);

}

void OutputTimeDependentPathMOEData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty)
{
/*
	ODPathSet*** ODPathSetVector = NULL;

	int StatisticsIntervalSize = g_DemandLoadingHorizon/30;

	ODPathSetVector = Allocate3DDynamicArray<ODPathSet>(g_ODZoneSize+1,g_ODZoneSize+1,StatisticsIntervalSize);

		std::map<int, DTAVehicle*>::iterator iterVM;
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
			{
				// step 0: search if there is an existing path for a given NodeSum, # of links, 
				// if new path, add
				// if old path, add statistics


				int StatisticsInterval = pVehicle->m_DepartureTime/30;

				if(StatisticsInterval >= StatisticsIntervalSize)  // avoid out of bound errors
					StatisticsInterval = StatisticsIntervalSize-1;

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
			fprintf(st, "origin_zone_id, destination_zone_id, departure_time, vehicle_type, information_type, #_of_vehicles_completing_trips, trip_time_in_min, cost_in_dollar, emissions\n");
		}

		for(int i = 1; i<=g_ODZoneSize; i++)
		for(int j = 1; j<=g_ODZoneSize; j++)
			for(int t=0; t<StatisticsIntervalSize; t++)
			{
			if(ODMOEArray[i][j][t].TotalVehicleSize>=1)
			{
			fprintf(st, "%d,%d,%d,0,0,%d,%5.2f,0,0\n", i,j,t*15, ODMOEArray[i][j][t].TotalVehicleSize, ODMOEArray[i][j][t].TotalTravelTime/ ODMOEArray[i][j][t].TotalVehicleSize);
			}
			}


		fclose(st);
	}
	if(ODPathSet !=NULL)
	Deallocate3DDynamicArray<ODPathSet>(ODMOEArray,g_ODZoneSize+1,g_ODZoneSize+1);
*/
}
