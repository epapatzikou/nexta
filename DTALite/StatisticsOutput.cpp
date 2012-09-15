//  Portions Copyright 2010 Xuesong Zhou

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
{	
	if(g_EmissionDataOutputFlag == 1)
	{
		cout << "     outputing emssions data... " << endl;
		OutputEmissionData();
	}


	bool bStartWithEmptyFile = true;
	cout << "     outputing output_agent.csv... " << endl;
	OutputVehicleTrajectoryData("output_agent.csv", iteration,true);


	ofstream output_ODMOE_file;

	cout << "     outputing output_ODMOE.csv... " << endl;
	output_ODMOE_file.open ("output_ODMOE.csv");
	//	output_ODImpact_file.open ("output_ImpactedOD.csv");
	if(output_ODMOE_file.is_open ())
	{
		OutputODMOEData(output_ODMOE_file,1);
		output_ODMOE_file.close();
	}else
	{
		cout << "Error: File output_ODMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();	
	}

	ofstream output_MovementMOE_file;
	cout << "     outputing output_MovementMOE.csv... " << endl;
	output_MovementMOE_file.open ("output_MovementMOE.csv", fstream::app);
	//	output_ODImpact_file.open ("output_ImpactedOD.csv");
	if(output_MovementMOE_file.is_open ())
	{
		OutputMovementMOEData(output_MovementMOE_file);
	}else
	{
		cout << "Error: File output_MovementMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();	
	}
	
		ofstream output_NetworkTDMOE_file;
	cout << "     outputing output_NetworkTDMOE.csv... " << endl;

	output_NetworkTDMOE_file.open ("output_NetworkTDMOE.csv");
	//	output_ODImpact_file.open ("output_ImpactedOD.csv");
	if(output_NetworkTDMOE_file.is_open ())
	{
		OutputNetworkMOEData(output_NetworkTDMOE_file);
		output_NetworkTDMOE_file.close();
	}else
	{
		cout << "Error: File output_NetworkTDMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();	
	}

	cout << "     outputing output_LinkTDMOE.csv... " << endl;
	OutputLinkMOEData("output_LinkTDMOE.csv", iteration,bStartWithEmptyFile);

	//if(g_VehiclePathOutputFlag ==1)
	//{
	// cout << "     outputing output_path.csv from start day = " <<g_StartIterationsForOutputPath << " to end day = " <<g_EndIterationsForOutputPath << " ..." << endl;
	// g_OutputDay2DayVehiclePathData("output_path.csv",  g_StartIterationsForOutputPath,g_EndIterationsForOutputPath);
	//
	//}

	if(g_TimeDependentODMOEOutputFlag==1 || g_ODEstimationFlag ==1) 
	{
		ofstream output_ODTDMOE_file;

		output_ODTDMOE_file.open ("output_ODTDMOE.csv");
		//	output_ODImpact_file.open ("output_ImpactedOD.csv");
		int department_time_interval = 60;
		if(output_ODTDMOE_file.is_open ())
		{
			cout << "     outputing output_ODTDMOE.csv... " << endl;
			OutputTimeDependentODMOEData(output_ODTDMOE_file,department_time_interval,1);
			output_ODTDMOE_file.close();
		}else
		{
			cout << "Error: File output_ODTDMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
			g_ProgramStop();	
		}
	}
	cout << "     MOE output complete... " << endl;

	return;
	ofstream output_PathMOE_file;

	output_PathMOE_file.open ("output_PathMOE.csv");
	//	output_ODImpact_file.open ("output_ImpactedOD.csv");
	if(output_PathMOE_file.is_open ())
	{
		cout << "     outputing output_PathMOE.csv... " << endl;

		OutputTimeDependentPathMOEData(output_PathMOE_file,50);
		output_PathMOE_file.close();
	}else
	{
		cout << "Error: File output_PathMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();	
	}



}
void OutputMovementMOEData(ofstream &output_MovementMOE_file)
{

	output_MovementMOE_file << "node_id,incoming_link_from_node_id,outgoing_link_to_node_id,turning_direction,movement_hourly_capacity,total_vehicle_count,avg_vehicle_delay_in_sec "<< endl;
	
	for(int i = 0; i < g_NodeVector.size(); i++)
	{
		for (std::map<string, DTANodeMovement>::iterator iter = g_NodeVector[i].m_MovementMap.begin(); 
			iter != g_NodeVector[i].m_MovementMap.end(); iter++)
		{
			float avg_delay = iter->second.GetAvgDelay_In_Min ()*60; // *60 to convert min to seconds
			output_MovementMOE_file << iter->second.in_link_to_node_id << "," << iter->second.in_link_from_node_id <<"," << iter->second.out_link_to_node_id << "," << iter->second.turning_direction << ","<< iter->second.movement_hourly_capacity << ",";
			
			output_MovementMOE_file << iter->second.total_vehicle_count << "," << avg_delay << endl;
			
		}
	}

}

void OutputODMOEData(ofstream &output_ODMOE_file,int cut_off_volume)
{

	ODStatistics** ODMOEArray = NULL;


	ODMOEArray = AllocateDynamicArray<ODStatistics>(g_ODZoneSize+1,g_ODZoneSize+1);


	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

			
		ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalVehicleSize+=1;
		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
		{

			ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalCompleteVehicleSize+=1;
			ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalTravelTime += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
			ODMOEArray[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalDistance += pVehicle->m_Distance;
		}
	}




	output_ODMOE_file << "origin_zone_no,destination_zone_no,number_of_vehicles,number_of_vehicles_completing_trips,trip_time_in_min,distance_in_mile" << endl;

	for(int i = 1; i<=g_ODZoneSize; i++)
		for(int j = 1; j<=g_ODZoneSize; j++)
		{

				if(ODMOEArray[i][j].TotalVehicleSize>=cut_off_volume)
				{
					output_ODMOE_file << i << "," << j << "," << ODMOEArray[i][j].TotalVehicleSize <<"," <<
						 ODMOEArray[i][j].TotalCompleteVehicleSize <<"," <<
						ODMOEArray[i][j].TotalTravelTime/ max(1,ODMOEArray[i][j].TotalCompleteVehicleSize) << "," <<
						ODMOEArray[i][j].TotalDistance /  max(1,ODMOEArray[i][j].TotalCompleteVehicleSize) << endl;
				}
		}
		if(ODMOEArray !=NULL)
			DeallocateDynamicArray<ODStatistics>(ODMOEArray,g_ODZoneSize+1,g_ODZoneSize+1);

}

void OutputTimeDependentODMOEData(ofstream &output_ODMOE_file, int department_time_intreval,int cut_off_volume)
{
/*
	// step 1: collect statistics based on vehicle file

	return; // research code
	output_ODMOE_file << "from_zone_id,to_zone_id,departure_time,number_of_vehicles,day_no,travel_time_in_min,distance_in_mile,#_of_diverted_vehicles,percentage_of_diversion,direrted_travel_time_in_min,direrted_distance_in_mile,percentage_of_non_diversion,non-direrted_travel_time_in_min,non-direrted_distance_in_mile" << endl;

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
				{
					if(ODMOEArray[i][j].D2DODStatisticsMap[DayNo].TotalVehicleSize>= cut_off_volume)
					{

						float avg_travel_time = ODMOEArray[i][j].D2DODStatisticsMap[DayNo].TotalTravelTime/ ODMOEArray[i][j].D2DODStatisticsMap[DayNo].TotalVehicleSize;
						float avg_travel_distance = ODMOEArray[i][j].D2DODStatisticsMap[DayNo].TotalDistance/ ODMOEArray[i][j].D2DODStatisticsMap[DayNo].TotalVehicleSize;

						ASSERT(avg_travel_time > -0.1f);
						output_ODMOE_file << i <<  "," << j << "," <<
							g_DemandLoadingStartTimeInMin + t*department_time_intreval << "," <<  ODMOEArray[i][j].D2DODStatisticsMap[DayNo].TotalVehicleSize <<  "," << 
							DayNo << "," <<  avg_travel_time  << "," << avg_travel_distance << "," ;

						if(ODMOEArray[i][j].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize >=1)
						{
							float percentage = ODMOEArray[i][j].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize*100.0f/ODMOEArray[i][j].D2DODStatisticsMap[DayNo].TotalVehicleSize;
							float diverted_avg_travel_time = ODMOEArray[i][j].D2DODStatisticsMap[DayNo].Diverted_TotalTravelTime/ max(1,ODMOEArray[i][j].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize);
							float diverted_avg_travel_distance = ODMOEArray[i][j].D2DODStatisticsMap[DayNo].Diverted_TotalDistance/  max(1,ODMOEArray[i][j].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize);

							output_ODMOE_file <<  ODMOEArray[i][j].D2DODStatisticsMap[DayNo].Diverted_TotalVehicleSize <<  "," << 
								percentage << "," << diverted_avg_travel_time<< "," << diverted_avg_travel_distance  << ",";
						}

						output_ODMOE_file << endl;
					}
				}
			}


			if(ODMOEArray !=NULL)
				Deallocate3DDynamicArray<D2DODStatistics>(ODMOEArray,g_ODZoneSize+1,g_ODZoneSize+1);


			// step 3: generate output_demand.csv for future simulation
			FILE* st = NULL;
			fopen_s(&st,"output_demand.csv","w");

			if(st!=NULL)
			{
			fprintf(st, "from_zone_id, to_zone_id, number_of_vehicle_trips\n");
			for(int i = 1; i<=g_ODZoneSize; i++)
			for(int j = 1; j<=g_ODZoneSize; j++)
			for(int t=0; t<StatisticsIntervalSize; t++)
			{
//				if(ODMOEArray[i][j].TotalVehicleSize>=1)
				{
				//fprintf(st, "%d,%d,%d\n", i,j,ODMOEArray[i][j].TotalVehicleSize);
				}			
				}					
			fclose(st);
			}
*/
}

void OutputTimeDependentPathMOEData(ofstream &output_PathMOE_file, int cut_off_volume)
{
	return; // time-consuming... commented out for now
	ODPathSet** ODPathSetVector = NULL;

	ODPathSetVector = AllocateDynamicArray<ODPathSet>(g_ODZoneSize+1,g_ODZoneSize+1);

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

		if(pVehicle->m_NodeSize >=2 && pVehicle->m_bComplete )
		{
				ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalVehicleSize=1;
		}
	

	}

	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{

		DTAVehicle* pVehicle = iterVM->second;

		if(pVehicle->m_NodeSize >=2 && pVehicle->m_bComplete && ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].TotalVehicleSize  > cut_off_volume)
		{
			std::vector<PathStatistics> Vehicle_ODTPathSet;

			Vehicle_ODTPathSet=ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].PathSet;

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
				ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].PathSet[PathNo].TotalVehicleSize+=1;
				ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].PathSet[PathNo].TotalDistance+=pVehicle->m_Distance;
				ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].PathSet[PathNo].TotalTravelTime+=(pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
				ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].PathSet[PathNo].TotalEmissions+=pVehicle->m_Emissions;
			}
			else				           // if new path, add
			{
				PathStatistics vehicle_PathStatistics;
				vehicle_PathStatistics.TotalDistance=pVehicle->m_Distance;
				vehicle_PathStatistics.TotalTravelTime=(pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
				vehicle_PathStatistics.TotalEmissions = pVehicle->m_Emissions ;
				vehicle_PathStatistics.NodeSums=pVehicle->m_NodeNumberSum;

				// add node id along the path
				int NodeID = g_LinkVector[pVehicle->m_NodeAry [0].LinkNo]->m_FromNodeID;  // first node
				int NodeName = g_NodeVector[NodeID].m_NodeName ;
				vehicle_PathStatistics.m_NodeNameArray .push_back (NodeName);
				for(int j = 0; j< pVehicle->m_NodeSize-1; j++)
				{
					int LinkID = pVehicle->m_NodeAry [j].LinkNo;
					NodeID = g_LinkVector[LinkID]->m_ToNodeID;
					NodeName = g_NodeVector[NodeID].m_NodeName ;
					vehicle_PathStatistics.m_NodeNameArray .push_back (NodeName);
				}
				ODPathSetVector[pVehicle->m_OriginZoneID][pVehicle->m_DestinationZoneID].PathSet.push_back(vehicle_PathStatistics);

			}
		}
	}

	output_PathMOE_file << "origin_zone_id,destination_zone_id,path_no,%,node sum,node sequence,#_of_vehicles_completing_trips,trip_time,distance,avg_CO2" << endl;

	for(int i = 1; i<=g_ODZoneSize; i++)
		for(int j = 1; j<=g_ODZoneSize; j++)
		{
			int PathNo=1;
			for (std::vector<PathStatistics>::iterator IterPS=ODPathSetVector[i][j].PathSet.begin(); IterPS!=ODPathSetVector[i][j].PathSet.end();IterPS++)
			{
				if (IterPS->TotalVehicleSize>=cut_off_volume)
				{
					//int vehicles_completing_trips=ODPathSetVector[i][j].PathSet[PathNo].TotalVehicleSize;
					//int Total_TravelTime=ODPathSetVector[i][j].PathSet[PathNo].TotalTravelTime;
					//int NodeSums=ODPathSetVector[i][j].PathSet[PathNo].NodeSums;
					int vehicles_completing_trips=IterPS->TotalVehicleSize;
					int Total_TravelTime= IterPS->TotalTravelTime;
					float Total_Distance = IterPS->TotalDistance;
					float Total_emissions =IterPS->TotalEmissions;
					int NodeSums=IterPS->NodeSums;
					float FlowRatio =vehicles_completing_trips*100.0f/g_SimulationResult.number_of_vehicles;

					output_PathMOE_file << i << "," << j<< ",";
					output_PathMOE_file <<  PathNo << "," << FlowRatio << "," <<  NodeSums << ",";

					for(int n = 0; n < IterPS->m_NodeNameArray .size(); n++)
					{
						output_PathMOE_file << IterPS->m_NodeNameArray[n];

						if(n!=  IterPS->m_NodeNameArray .size()-1)
							output_PathMOE_file << "->" ;

					}

					output_PathMOE_file << "," << vehicles_completing_trips << "," << Total_TravelTime*1.0f/vehicles_completing_trips<< "," << Total_Distance*1.0f/vehicles_completing_trips<< ","<< Total_emissions/vehicles_completing_trips << endl;
				}
				PathNo++;
			}
		}

		if(ODPathSetVector!=NULL)
			DeallocateDynamicArray<ODPathSet>(ODPathSetVector,g_ODZoneSize+1,g_ODZoneSize+1);

}


int g_OutputSimulationSummary(float& AvgTravelTime, float& AvgDistance, float& AvgSpeed, float& AvgCost, EmissionStatisticsData &emission_data,
							  int InformationClass=-1, int DemandType=-1,int VehicleType = -1, int DepartureTimeInterval = -1)
{
	float TravelTimeSum = 0;
	float TravelCostSum = 0;
	float DistanceSum = 0;
	int VehicleCount = 0;

	emission_data.Init ();

	for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end();v++)
	{

		DTAVehicle* pVehicle = (*v);
		if(pVehicle->m_bComplete )  // vehicle completes the trips
		{
			if( (pVehicle->m_InformationClass == InformationClass ||InformationClass==-1)
				&& (pVehicle->m_DemandType == DemandType ||DemandType==-1)
				&& (pVehicle->m_VehicleType  == VehicleType ||VehicleType==-1)
				&& ((int)(pVehicle->m_DepartureTime/ g_AggregationTimetInterval)== (int)(DepartureTimeInterval/g_AggregationTimetInterval) ||DepartureTimeInterval==-1))
			{
				TravelTimeSum += pVehicle->m_TripTime;
				TravelCostSum += pVehicle->m_TollDollarCost ;

				DistanceSum+= pVehicle ->m_Distance;

				emission_data.TotalEnergy +=  pVehicle->Energy;
				emission_data.TotalCO2 +=  pVehicle->CO2;
				emission_data.TotalCO +=  pVehicle->CO;
				emission_data.TotalNOX +=  pVehicle->NOX;
				emission_data.TotalHC +=  pVehicle->HC;

				VehicleCount++;

			}
		}
	}

	AvgTravelTime = TravelTimeSum/max(1,VehicleCount);  // unit: min
	AvgDistance = DistanceSum/max(1,VehicleCount);  // unit: mile
	AvgSpeed = AvgDistance/max(0.0001,AvgTravelTime)*60;  // unit: mph
	AvgCost = TravelCostSum/max(1,VehicleCount);  // unit: mph

	emission_data.AvgEnergy = emission_data.TotalEnergy/max(1,VehicleCount);
	emission_data.AvgCO2 = emission_data.TotalCO2/max(1,VehicleCount);
	emission_data.AvgCO = emission_data.TotalCO/max(1,VehicleCount);
	emission_data.AvgNOX = emission_data.TotalNOX/max(1,VehicleCount);
	emission_data.AvgHC = emission_data.TotalHC/max(1,VehicleCount);

	return VehicleCount;
}


int g_OutputSimulationMOESummary(float& AvgTravelTime, float& AvgDistance, float& AvgSpeed, float & AvgTollCost,  EmissionStatisticsData &emission_data, LinkMOEStatisticsData &link_data,
								 int DemandType,int VehicleType, int InformationClass, int origin_zone_id, int destination_zone_id,
								 int from_node_id, int mid_node_id	, int to_node_id	,	
								 int departure_starting_time,int departure_ending_time,int entrance_starting_time, int entrance_ending_time)
{
	float TravelTimeSum = 0;
	float DistanceSum = 0;
	float CostSum = 0;
	int VehicleCount = 0;


	emission_data.Init ();
	link_data.Init ();

	if(from_node_id ==0)  // not link or path MOE
	{
		for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end();v++)
		{

			DTAVehicle* pVehicle = (*v);
			if(pVehicle->m_bComplete)  // vehicle completes the trips
			{
				if( (pVehicle->m_InformationClass == InformationClass ||InformationClass <=0)
					&& (pVehicle->m_DemandType == DemandType ||DemandType<=0)
					&& (pVehicle->m_VehicleType  == VehicleType ||VehicleType <=0)
					&& (pVehicle->m_OriginZoneID    == origin_zone_id ||origin_zone_id <=0)
					&& (pVehicle->m_DestinationZoneID    == destination_zone_id ||destination_zone_id <=0)
					&& ((int)(pVehicle->m_DepartureTime) >= departure_starting_time && (int)(pVehicle->m_DepartureTime < departure_ending_time)|| (departure_starting_time	 == 0 && departure_ending_time ==0) || (departure_starting_time	 == 0 && departure_ending_time == 1440)))
				{


					TravelTimeSum += pVehicle->m_TripTime;
					DistanceSum+= pVehicle ->m_Distance;
					CostSum += pVehicle ->m_TollDollarCost;
					emission_data.TotalEnergy +=  pVehicle->Energy;
					emission_data.TotalCO2 +=  pVehicle->CO2;
					emission_data.TotalCO +=  pVehicle->CO;
					emission_data.TotalNOX +=  pVehicle->NOX;
					emission_data.TotalHC +=  pVehicle->HC;


					double Gasoline = pVehicle->Energy*1000/(121.7);
					double MilesPerGallon = 0;
					
					if(Gasoline>0.0001)
						MilesPerGallon = pVehicle->m_Distance / max(0.0001,Gasoline);

					emission_data.TotalMilesPerGallon += MilesPerGallon;

					//1 gallon = 121.7 megajoules – LHV *

//source:
//http://ecotec-systems.com/Resources/FUEL_CONVERSION_WORK_SHEET.pdf
//					Energy contents are expressed as either High (gross) Heating Value (HHV) or Lower (net) 
//Heating Value (LHV). LHV is closest to the actual energy yield in most cases. HHV (including 
//condensation of combustion products) is greater by between 5% (in the case of coal) and 
//10% (for natural gas), depending mainly on the hydrogen content of the fuel. For most 
//biomass feed-stocks this difference appears to be 6-7%.

//					TRACE("veh id: %d,NOX: %f\n",pVehicle->m_VehicleID ,pVehicle->NOX);

					VehicleCount++;

				}
			}
		}

		AvgTravelTime = TravelTimeSum/max(1,VehicleCount);  // unit: min
		AvgDistance = DistanceSum/max(1,VehicleCount);  // unit: mile
		AvgSpeed = AvgDistance/max(1,AvgTravelTime)*60;  // unit: mph
		AvgTollCost = CostSum/max(1,VehicleCount);

		emission_data.AvgEnergy = emission_data.TotalEnergy/max(1,VehicleCount);
		emission_data.AvgCO2 = emission_data.TotalCO2/max(1,VehicleCount);
		emission_data.AvgCO = emission_data.TotalCO/max(1,VehicleCount);
		emission_data.AvgNOX = emission_data.TotalNOX/max(1,VehicleCount);
		emission_data.AvgHC = emission_data.TotalHC/max(1,VehicleCount);
		emission_data.AvgMilesPerGallon = emission_data.TotalMilesPerGallon/max(1,VehicleCount);

	}else
	{  

		if(mid_node_id== 0 )  // Link MOE
		{

			for(unsigned li = 0; li< g_LinkVector.size(); li++)
			{
				DTALink* pLink = g_LinkVector[li];

				if(pLink->m_FromNodeNumber == from_node_id && pLink->m_ToNodeNumber == to_node_id)
				{

					AvgTravelTime = pLink->GetTravelTimeByMin(g_NumberOfIterations,entrance_starting_time, entrance_ending_time-entrance_starting_time);
				
					AvgSpeed =		pLink->m_Length / max(0.00001,AvgTravelTime) *60;  // unit: mph
					ASSERT(AvgSpeed <= pLink->m_SpeedLimit +1 );
					AvgDistance = pLink->m_Length;
					
					VehicleCount = pLink->GetTrafficVolumeByMin(g_NumberOfIterations,entrance_starting_time, entrance_ending_time-entrance_starting_time);
					
					emission_data.AvgEnergy = pLink->TotalEnergy /max(1,pLink->CFlowArrivalCount );
					emission_data.AvgCO2 =  pLink->TotalCO2/max(1,pLink->CFlowArrivalCount );
					emission_data.AvgCO =   pLink->TotalCO/max(1,pLink->CFlowArrivalCount );
					emission_data.AvgNOX =  pLink->TotalNOX/max(1,pLink->CFlowArrivalCount );
					emission_data.AvgHC =  pLink->TotalHC/max(1,pLink->CFlowArrivalCount );

					link_data.SOV_volume =  pLink->CFlowArrivalCount_PricingType [1];
					link_data.HOV_volume  =  pLink->CFlowArrivalCount_PricingType [2];
					link_data.Truck_volume  =  pLink->CFlowArrivalCount_PricingType [3];
					link_data.Intermodal_volume  =  pLink->CFlowArrivalCount_PricingType [4];

					link_data.number_of_crashes_per_year  =	pLink->m_NumberOfCrashes;
					link_data.number_of_fatal_and_injury_crashes_per_year =	pLink->m_NumberOfFatalAndInjuryCrashes;
					link_data.number_of_property_damage_only_crashes_per_year = pLink->m_NumberOfPDOCrashes;

					break;
				} // for this link
			}
		}else  /////////////////////////////////// 3 point path
		{

			for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end();v++)
			{

				DTAVehicle* pVehicle = (*v);
				if(pVehicle->m_bComplete && pVehicle->m_PricingType !=4)  // vehicle completes the trips
				{
					if( (pVehicle->m_InformationClass == InformationClass ||InformationClass <=0)
						&& (pVehicle->m_DemandType == DemandType ||DemandType<=0)
						&& (pVehicle->m_VehicleType  == VehicleType ||VehicleType <=0)
						&& (pVehicle->m_OriginZoneID    == origin_zone_id ||origin_zone_id <=0)
						&& (pVehicle->m_DestinationZoneID    == destination_zone_id ||destination_zone_id <=0)
						&& ((int)(pVehicle->m_DepartureTime) >= departure_starting_time && (int)(pVehicle->m_DepartureTime < departure_ending_time)|| (departure_starting_time	 == 0 && departure_ending_time ==0) || (departure_starting_time	 == 0 && departure_ending_time == 1440)))
					{
						//step 1: test three point condition


						bool pass_from_node_id = false;
						bool pass_mid_node_id = false;
						bool pass_to_node_id = false;
						float from_node_id_timestamp = 0;
						float end_node_id_timestamp = 0;

						int NodeID = g_LinkVector[pVehicle->m_NodeAry [0].LinkNo]->m_FromNodeID;  // first node
						int NodeName = g_NodeVector[NodeID].m_NodeName ;
						float total_path_distance = 0;

						EmissionStatisticsData sub_path_emission_data;

						if(NodeName == from_node_id)
						{
							pass_from_node_id = true;
							from_node_id_timestamp = pVehicle->m_DepartureTime ;
						}

						for(int j = 0; j< pVehicle->m_NodeSize-1; j++)
						{
							int LinkID = pVehicle->m_NodeAry [j].LinkNo;
							NodeID = g_LinkVector[LinkID]->m_ToNodeID;
							NodeName = g_NodeVector[NodeID].m_NodeName ;

							if(NodeName == from_node_id)
							{
								pass_from_node_id = true;

								if(j>0)
									from_node_id_timestamp = pVehicle->m_NodeAry [j-1].AbsArrivalTimeOnDSN;
								else
									from_node_id_timestamp = pVehicle->m_DepartureTime ;
							}

							if(pass_from_node_id && NodeName == mid_node_id)
							{
								pass_mid_node_id = true;
							}

							if(pass_from_node_id)  // start counting distance along the path
							{
								DTALink* pLink  = g_LinkVector[LinkID];
								total_path_distance += pLink->m_Length ;
								// be careful here. we do not have link based statistics for each vehicle, so we use link based avg statistics
								sub_path_emission_data.TotalEnergy += pLink->TotalEnergy /max(1,pLink->CFlowArrivalCount );
								sub_path_emission_data.TotalCO2 += pLink->TotalCO2 /max(1,pLink->CFlowArrivalCount );
								sub_path_emission_data.TotalCO += pLink->TotalCO /max(1,pLink->CFlowArrivalCount );
								sub_path_emission_data.TotalNOX += pLink->TotalNOX /max(1,pLink->CFlowArrivalCount );
								sub_path_emission_data.TotalHC += pLink->TotalHC /max(1,pLink->CFlowArrivalCount );

								float Gasoline = pVehicle->Energy/(121.7*1000000);
								float MilesPerGallon = 0;
								
								if(Gasoline>0.0001)
								MilesPerGallon = pVehicle->m_Distance / max(0.0001,Gasoline);

								sub_path_emission_data.TotalMilesPerGallon += MilesPerGallon;

							}


							if(pass_mid_node_id && NodeName == to_node_id)
							{
								pass_to_node_id = true;
								ASSERT(j>0);
								end_node_id_timestamp = pVehicle->m_NodeAry [j].AbsArrivalTimeOnDSN;
								break;
							}


						}


						// step 2: check condition again for statitics testing

						if(pass_from_node_id && pass_mid_node_id && pass_to_node_id)
						{
							TravelTimeSum += (end_node_id_timestamp - from_node_id_timestamp);
							DistanceSum+= total_path_distance;
							VehicleCount++;

							emission_data.TotalEnergy +=  sub_path_emission_data.TotalEnergy;
							emission_data.TotalCO2 +=  sub_path_emission_data.TotalCO2;
							emission_data.TotalCO +=  sub_path_emission_data.TotalCO;
							emission_data.TotalNOX +=  sub_path_emission_data.TotalNOX;
							emission_data.TotalHC +=  sub_path_emission_data.TotalHC;
							emission_data.TotalMilesPerGallon  +=  sub_path_emission_data.TotalMilesPerGallon;

						}

					} // condition on demand type... 
				} //condition on complete vehicles
			} // for each vehicle

			AvgTravelTime = TravelTimeSum/max(1,VehicleCount);  // unit: min
			AvgDistance = DistanceSum/max(1,VehicleCount);  // unit: mile
			AvgSpeed = AvgDistance/max(0.001,AvgTravelTime)*60;  // unit: mph

			emission_data.AvgEnergy = emission_data.TotalEnergy/max(1,VehicleCount);
			emission_data.AvgCO2 = emission_data.TotalCO2/max(1,VehicleCount);
			emission_data.AvgCO = emission_data.TotalCO/max(1,VehicleCount);
			emission_data.AvgNOX = emission_data.TotalNOX/max(1,VehicleCount);
			emission_data.AvgHC = emission_data.TotalHC/max(1,VehicleCount);
			emission_data.AvgMilesPerGallon  = emission_data.TotalMilesPerGallon /max(1,VehicleCount);

		}
	}
	return VehicleCount;
}


void g_GenerateSummaryStatisticsTable()
{

	int cl;

	CCSVParser parser_MOE_settings;
	if (!parser_MOE_settings.OpenCSVFile("input_MOE_settings.csv"))
	{
		cout << "File input_MOE_settings.csv cannot be opened. Please check." << endl;
		g_ProgramStop();

	}


	g_SummaryStatFile.Reset ();
	g_SummaryStatFile.SetRowTitle(true);

	int moe_group = 0;
	int prev_moe_group = -1;

	while(parser_MOE_settings.ReadRecord())
	{
		string moe_type, moe_category_label;

		
		parser_MOE_settings.GetValueByFieldName("moe_type",moe_type);
		parser_MOE_settings.GetValueByFieldName("moe_group",moe_group);
		parser_MOE_settings.GetValueByFieldName("moe_category_label",moe_category_label);

		cout << "  outputing simulation summary for MOE " << moe_group << " "<< moe_category_label <<endl;

		if( prev_moe_group !=  moe_group)  // new group
		{
			g_SummaryStatFile.Reset ();

			g_SummaryStatFile.WriteNewEndofLine ();
			g_SummaryStatFile.WriteNewEndofLine ();

			CString str_moe_group;
			str_moe_group.Format ("--MOE Group %d--",moe_group);
			g_SummaryStatFile.WriteTextString (str_moe_group);

			if(moe_type.compare("network_time_dependent") == 0)  // time-dependent 
			{
				g_SummaryStatFile.WriteTextString ("Detailed Data:,output_NetworkTDMOE.csv");

				OutputNetworkMOEData(g_SummaryStatFile.outFile);

				continue; // skip the other MOEs
			}


			if(moe_type.compare("link_critical") == 0)  // critical link list
			{

				g_SummaryStatFile.WriteTextString ("Detailed Data:,output_LinkMOE.csv");

				int cut_off_volume = 100;
				parser_MOE_settings.GetValueByFieldName("cut_off_volume",cut_off_volume);
				g_OutputLinkMOESummary(g_SummaryStatFile.outFile, cut_off_volume);

				continue; // skip the other MOEs
			}

			if(moe_type.compare("od_critical") == 0)  // critical OD list
			{

				g_SummaryStatFile.WriteTextString ("Detailed Data:,output_ODMOE.csv");

				int cut_off_volume = 50;
				parser_MOE_settings.GetValueByFieldName("cut_off_volume",cut_off_volume);

				if(cut_off_volume ==0)
					cut_off_volume = 1;  // set minimum cut off value as 1



				OutputODMOEData(g_SummaryStatFile.outFile,cut_off_volume);

				if(g_TimeDependentODMOEOutputFlag==1)
				{
					g_SummaryStatFile.WriteTextString ("Detailed Data:,output_ODTDMOE.csv");
					OutputTimeDependentODMOEData(g_SummaryStatFile.outFile,60,cut_off_volume);
				}

				continue; // skip the other MOEs
			}

			if(moe_type.compare("path_critical") == 0)  // critical path list
			{

				g_SummaryStatFile.WriteTextString ("Detailed Data:,output_PathMOE.csv");

				int cut_off_volume = 50;
				parser_MOE_settings.GetValueByFieldName("cut_off_volume",cut_off_volume);
				OutputTimeDependentPathMOEData(g_SummaryStatFile.outFile, cut_off_volume);

				continue; // skip the other MOEs
			}


			g_SummaryStatFile.SetFieldName ("#_of_vehicles");
			g_SummaryStatFile.SetFieldName ("percentage" );
			g_SummaryStatFile.SetFieldName ("avg_distance");
			g_SummaryStatFile.SetFieldName ("avg_travel_time");
			g_SummaryStatFile.SetFieldName ("avg_speed");
			g_SummaryStatFile.SetFieldName ("avg_toll_cost");

			g_SummaryStatFile.SetFieldName("avg_energy");
			g_SummaryStatFile.SetFieldName("avg_CO2");
			g_SummaryStatFile.SetFieldName("avg_NOX");
			g_SummaryStatFile.SetFieldName("avg_CO");
			g_SummaryStatFile.SetFieldName("avg_HC");
			g_SummaryStatFile.SetFieldName("avg_MilesPerGallon");

			if(moe_category_label.compare("link") == 0)  // Link MOE
			{

				g_SummaryStatFile.SetFieldName("SOV_volume");
				g_SummaryStatFile.SetFieldName("HOV_volume");
				g_SummaryStatFile.SetFieldName("Truck_volume");
				g_SummaryStatFile.SetFieldName("Intermodal_volume");

				g_SummaryStatFile.SetFieldName("number_of_crashes_per_year");
				g_SummaryStatFile.SetFieldName("number_of_fatal_and_injury_crashes_per_year");
				g_SummaryStatFile.SetFieldName("number_of_property_damage_only_crashes_per_year");

			}
		}

		if( prev_moe_group !=  moe_group)
		{
			g_SummaryStatFile.WriteHeader ();  // write header for a new group
			prev_moe_group = moe_group;
		}


		int demand_type =  0;
		int vehicle_type = 0;
		int information_type = 0;
		int from_node_id = 0;
		int mid_node_id	=0;
		int to_node_id	=0;
		int origin_zone_id	=0;
		int destination_zone_id	=0;
		int departure_starting_time	 = 0;
		int departure_ending_time= 1440;
		int entrance_starting_time	= 0;
		int entrance_ending_time = 1440;


		parser_MOE_settings.GetValueByFieldName("moe_type",moe_type);
		parser_MOE_settings.GetValueByFieldName("moe_category_label",moe_category_label);
		parser_MOE_settings.GetValueByFieldName("demand_type",demand_type);
		parser_MOE_settings.GetValueByFieldName("vehicle_type",vehicle_type);
		parser_MOE_settings.GetValueByFieldName("information_type",information_type);
		parser_MOE_settings.GetValueByFieldName("from_node_id",from_node_id);
		parser_MOE_settings.GetValueByFieldName("mid_node_id",mid_node_id);
		parser_MOE_settings.GetValueByFieldName("to_node_id",to_node_id);
		parser_MOE_settings.GetValueByFieldName("origin_zone_id",origin_zone_id);
		parser_MOE_settings.GetValueByFieldName("destination_zone_id",destination_zone_id);
		parser_MOE_settings.GetValueByFieldName("departure_starting_time",departure_starting_time);
		parser_MOE_settings.GetValueByFieldName("departure_ending_time",departure_ending_time);
		parser_MOE_settings.GetValueByFieldName("entrance_starting_time",entrance_starting_time);
		parser_MOE_settings.GetValueByFieldName("entrance_ending_time",entrance_ending_time);

		int Count=0; 
		float AvgTravelTime = 0;
		float AvgDistance = 0;
		float AvgSpeed = 0;
		float AvgCost = 0;
		EmissionStatisticsData emission_data;
		LinkMOEStatisticsData  link_data;
		Count = g_OutputSimulationMOESummary(AvgTravelTime,AvgDistance, AvgSpeed, AvgCost,  emission_data, link_data,
			demand_type,vehicle_type, information_type, origin_zone_id,destination_zone_id,
			from_node_id, mid_node_id, to_node_id,	
			departure_starting_time,departure_ending_time,entrance_starting_time,entrance_ending_time);

		float percentage = Count*100.0f/max(1,g_SimulationResult.number_of_vehicles);

		g_SummaryStatFile.SetValueByFieldName ("#_of_vehicles",Count );
		g_SummaryStatFile.SetValueByFieldName ("percentage",percentage );
		g_SummaryStatFile.SetValueByFieldName ("avg_travel_time",AvgTravelTime);
		g_SummaryStatFile.SetValueByFieldName ("avg_distance",AvgDistance);
		g_SummaryStatFile.SetValueByFieldName ("avg_speed",AvgSpeed);
		g_SummaryStatFile.SetValueByFieldName ("avg_toll_cost",AvgCost);


		g_SummaryStatFile.SetValueByFieldName("avg_energy",emission_data.AvgEnergy);
		g_SummaryStatFile.SetValueByFieldName("avg_CO2",emission_data.AvgCO2);
		g_SummaryStatFile.SetValueByFieldName("avg_NOX",emission_data.AvgNOX);
		g_SummaryStatFile.SetValueByFieldName("avg_CO",emission_data.AvgCO);
		g_SummaryStatFile.SetValueByFieldName("avg_HC",emission_data.AvgHC);
		g_SummaryStatFile.SetValueByFieldName("avg_MilesPerGallon",emission_data.AvgMilesPerGallon );

		

		if(moe_type.compare ("link")==0) // Link MOE
		{
			g_SummaryStatFile.SetValueByFieldName("SOV_volume",link_data.SOV_volume );
			g_SummaryStatFile.SetValueByFieldName("HOV_volume",link_data.HOV_volume );
			g_SummaryStatFile.SetValueByFieldName("Truck_volume",link_data.Truck_volume );
			g_SummaryStatFile.SetValueByFieldName("Intermodal_volume",link_data.Intermodal_volume );

			g_SummaryStatFile.SetValueByFieldName("number_of_crashes_per_year",link_data.number_of_crashes_per_year );
			g_SummaryStatFile.SetValueByFieldName("number_of_fatal_and_injury_crashes_per_year",link_data.number_of_fatal_and_injury_crashes_per_year);
			g_SummaryStatFile.SetValueByFieldName("number_of_property_damage_only_crashes_per_year",link_data.number_of_property_damage_only_crashes_per_year );

		}
			
		TRACE("%s\n",moe_category_label.c_str ());
			if(moe_category_label.find (",") !=  string::npos)
			{
			moe_category_label = '"' + moe_category_label + '"' ;
			}

		string str = moe_category_label + ",";

		g_SummaryStatFile.WriteTextLabel (str.c_str ());

		g_SummaryStatFile.WriteRecord ();

	}


	parser_MOE_settings.CloseCSVFile ();

}
void g_OutputSimulationStatistics(int Iteration)
{

	g_SummaryStatFile.WriteTextString("Unit of output:");
	g_SummaryStatFile.WriteTextString("travel time=,min");
	g_SummaryStatFile.WriteTextString("travel Cost=,dollar");
	g_SummaryStatFile.WriteTextString("distance=,miles");
	g_SummaryStatFile.WriteTextString("speed=,mph");
	g_SummaryStatFile.WriteTextString("energy=,1000 joule");
	g_SummaryStatFile.WriteTextString("CO2=,gram");

	g_GenerateSummaryStatisticsTable();

	return;
	int NumberOfLinks = 9999999;

	ofstream LinkMOESummaryFile;

	LinkMOESummaryFile.open ("output_LinkMOE.csv", ios::out);
	if (LinkMOESummaryFile.is_open())
	{
		g_OutputLinkMOESummary(LinkMOESummaryFile);  // output assignment results anyway

		LinkMOESummaryFile.close();


	}else
	{
		cout << "Error: File output_LinkMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();

	}

//	g_OutputLinkOutCapacitySummary();

		LinkMOESummaryFile.open ("output_2WayLinkMOE.csv", ios::out);
	if (LinkMOESummaryFile.is_open())
	{
	//	g_Output2WayLinkMOESummary(LinkMOESummaryFile);  // output assignment results anyway

		LinkMOESummaryFile.close();
	}else
	{
		cout << "Error: File output_2WayLinkMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();

	}

	//	g_OutputSummaryKML(MOE_crashes);
	//	g_OutputSummaryKML(MOE_CO2);
	//	g_OutputSummaryKML(MOE_total_energy);

	EmissionStatisticsData emission_data;
	int Count=0; 
	float AvgTravelTime, AvgDistance, AvgSpeed, AvgCost;
	g_LogFile << "--- MOE for vehicles completing trips ---" << endl;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, AvgCost, emission_data, -1, -1, -1,-1);
	g_LogFile << " # of Vehicles = " << Count << " AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	g_LogFile << "--- MOE for Each Information Class ---" << endl;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, AvgCost, emission_data, 1, -1, -1,-1);
	if(Count>0)
		g_LogFile << "Hist Knowledge (VMS non-responsive): # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" << endl;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, AvgCost, emission_data, 2, -1, -1,-1);
	if(Count>0)
		g_LogFile << "Pre-trip Info                      : # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, AvgCost, emission_data, 3, -1, -1,-1);
	if(Count>0)
		g_LogFile << "En-route Info                      : # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" << endl;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, AvgCost,  emission_data, 4, -1, -1,-1);
	if(Count>0)
		g_LogFile << "Hist Knowledge (VMS responsive)    : # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	g_LogFile << endl  << "--- MOE for Each Vehicle Type ---" << endl;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, AvgCost,  emission_data, -1, 1, -1,-1);
	if(Count>0)
		g_LogFile << "LOV Passenger Car: # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, AvgCost,  emission_data, -1, 2, -1,-1);
	if(Count>0)
		g_LogFile << "HOV Passenger Car: # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, AvgCost,  emission_data, -1, 3, -1,-1);
	if(Count>0)
		g_LogFile << "Truck            : # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	g_LogFile << endl << "--- MOE for Each Departure Time Interval ---" << endl;

	for(int departure_time = g_DemandLoadingStartTimeInMin; departure_time < g_DemandLoadingEndTimeInMin; departure_time += g_AggregationTimetInterval)
	{
		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, AvgCost,  emission_data, -1, -1 ,-1, departure_time);
		if(Count>0)
			g_LogFile << "Time:" << departure_time << " min, # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	}
	g_LogFile << endl;

	// output general link statistics
	std::set<DTALink*>::iterator iterLink;
	g_LogFile << "--- Link MOE ---" << endl;

	for(unsigned li = 0; li< min(NumberOfLinks,g_LinkVector.size()); li++)
	{
		float Capacity = g_LinkVector[li]->m_LaneCapacity * g_LinkVector[li]->m_NumLanes;
		g_LogFile << "Link: " <<  g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName  << " -> " << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName << 
			", Link Capacity: " << Capacity <<
			", Inflow: " << g_LinkVector[li]->CFlowArrivalCount <<
			//				" Outflow: " << g_LinkVector[li]->CFlowDepartureCount <<
			", VOC Ratio: " << g_LinkVector[li]->CFlowDepartureCount /max(1,Capacity)  << endl;
	}

	g_LogFile << g_GetAppRunningTime()  <<" ---End of Link MOE ---" << endl;

}


void g_OutputLinkMOESummary(ofstream &LinkMOESummaryFile, int cut_off_volume)
{
	LinkMOESummaryFile.precision(4) ;
	LinkMOESummaryFile.setf(ios::fixed);

	LinkMOESummaryFile << "from_node_id,to_node_id,type_code,start_time_in_min,end_time_in_min,total_link_volume,lane_capacity_in_vhc_per_hour,volume_over_capacity_ratio,speed_limit_in_mph,speed_in_mph,percentage_of_speed_limit,level_of_service,sensor_data_flag,sensor_link_volume,simulated_link_volume,measurement_error_percentage,abs_measurement_error_percentage,simulated_AADT,num_of_crashes_per_year,num_of_fatal_and_injury_crashes_per_year,num_of_PDO_crashes_per_year,TotalEnergy_(J/hr),CO2_(g/hr),NOX_(g/hr),CO_(g/hr),HC_(g/hr),sov_volume,hov_volume,truck_volume,intermodal_volume,";

	//		DTASafetyPredictionModel SafePredictionModel;
	//		SafePredictionModel.UpdateCrashRateForAllLinks();

	LinkMOESummaryFile << "from_node_id,to_node_id,";
	int d;

	//volume
	for(d=0; d <=g_NumberOfIterations; d++)
	{
		LinkMOESummaryFile << "vol_d" << d << ",";
	}

	//speed
	for(d=0; d <=g_NumberOfIterations; d++)
	{
		LinkMOESummaryFile << "spd_d" << d << ",";
	}
	LinkMOESummaryFile << endl;

	std::set<DTALink*>::iterator iterLink;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		DTALink* pLink = g_LinkVector[li];

		double average_travel_time = pLink->GetTravelTimeByMin(g_NumberOfIterations,0, pLink->m_SimulationHorizon);
		double speed = pLink->m_Length / max(0.00001,average_travel_time) *60;  // unit: mph
		double capacity_simulation_horizon = pLink->m_LaneCapacity * pLink->m_NumLanes;
		double voc_ratio = pLink->CFlowArrivalCount / max(0.1,capacity_simulation_horizon);
		int percentage_of_speed_limit = int(speed/max(0.1,pLink->m_SpeedLimit)*100+0.5);

		if(pLink->CFlowArrivalCount >= cut_off_volume)
		{
			LinkMOESummaryFile << g_NodeVector[pLink->m_FromNodeID].m_NodeName  << "," ;
			LinkMOESummaryFile << g_NodeVector[pLink->m_ToNodeID].m_NodeName << "," ;
			LinkMOESummaryFile << g_LinkTypeMap[pLink->m_link_type].type_code.c_str ()<< "," ;
			LinkMOESummaryFile << g_DemandLoadingStartTimeInMin << "," ;
			LinkMOESummaryFile << g_DemandLoadingEndTimeInMin << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount << "," ; // total hourly arrival flow 
			LinkMOESummaryFile << pLink->m_LaneCapacity << "," ;
			LinkMOESummaryFile << voc_ratio << "," ;
			LinkMOESummaryFile << pLink->m_SpeedLimit << "," ;
			LinkMOESummaryFile << speed << "," ;
			LinkMOESummaryFile << percentage_of_speed_limit << "," ;
			LinkMOESummaryFile << g_GetLevelOfService(percentage_of_speed_limit) << "," ;
			LinkMOESummaryFile << pLink->m_bSensorData << "," ;
			LinkMOESummaryFile << pLink->m_ObservedFlowVolume << "," ;

			float error_percentage;

			if( pLink->m_bSensorData)
				error_percentage = (pLink->CFlowArrivalCount - pLink->m_ObservedFlowVolume) / max(1,pLink->m_ObservedFlowVolume) *100;
			else
				error_percentage  = 0;

			LinkMOESummaryFile << pLink->CFlowArrivalCount << "," ; // total hourly arrival flow 
			LinkMOESummaryFile << error_percentage << "," ;
			LinkMOESummaryFile << fabs(error_percentage) << "," ;

			LinkMOESummaryFile << pLink->m_AADT << "," ;
			LinkMOESummaryFile << pLink->m_NumberOfCrashes << "," ;
			LinkMOESummaryFile << pLink->m_NumberOfFatalAndInjuryCrashes << "," ;
			LinkMOESummaryFile << pLink->m_NumberOfPDOCrashes << "," ;

			LinkMOESummaryFile << pLink->TotalEnergy  << "," ;
			LinkMOESummaryFile << pLink->TotalCO2  << "," ;
			LinkMOESummaryFile << pLink->TotalNOX  << "," ;
			LinkMOESummaryFile << pLink->TotalCO  << "," ;
			LinkMOESummaryFile << pLink->TotalHC  << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount_PricingType[1]  << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount_PricingType[2]  << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount_PricingType[3]  << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount_PricingType[4]  << "," ;

			LinkMOESummaryFile << g_NodeVector[pLink->m_FromNodeID].m_NodeName  << "," ;
			LinkMOESummaryFile << g_NodeVector[pLink->m_ToNodeID].m_NodeName << "," ;

			//volume
			for(d=0; d <=g_NumberOfIterations; d++)
			{
				LinkMOESummaryFile << pLink->m_Day2DayLinkMOEVector [d].TotalFlowCount << ",";
			}

			//speed
			for(d=0; d <=g_NumberOfIterations; d++)
			{
				LinkMOESummaryFile << pLink->m_Day2DayLinkMOEVector [d].AvgSpeed  << ",";
			}
			LinkMOESummaryFile << endl;	

		}
	}


}

void g_OutputLinkOutCapacitySummary()
{
	ofstream LinkOutCapcaityFile;

	LinkOutCapcaityFile.open ("output_LinkCapacity.csv", ios::out);

	LinkOutCapcaityFile.precision(1) ;
	LinkOutCapcaityFile.setf(ios::fixed);

	std::set<DTALink*>::iterator iterLink;
	bool bTitlePrintOut = false;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		unsigned int t;

		DTALink* pLink = g_LinkVector[li];

		if(pLink->m_OutCapacityVector.size() > 0 )
		{
			if(!bTitlePrintOut)
			{
			LinkOutCapcaityFile << "time_stamp_in_min" << "," ;

			for(t = 0; t < pLink->m_OutCapacityVector.size(); t++)
			{
			LinkOutCapcaityFile << pLink->m_OutCapacityVector[t].time_stamp_in_min << "," ;
			}
			bTitlePrintOut = true;
			LinkOutCapcaityFile << endl;	
			}

			LinkOutCapcaityFile << "link " << pLink->m_FromNodeNumber << "->" << pLink->m_ToNodeNumber << ",";
			for( t = 0; t < pLink->m_OutCapacityVector.size(); t++)
			{
			LinkOutCapcaityFile << pLink->m_OutCapacityVector[t].hourly_out_capacity << "," ;
			}
			LinkOutCapcaityFile << endl;	

		}
	}

	LinkOutCapcaityFile.close();
}

void g_Output2WayLinkMOESummary(ofstream &LinkMOESummaryFile, int cut_off_volume)
{
	LinkMOESummaryFile.precision(4) ;
	LinkMOESummaryFile.setf(ios::fixed);

	LinkMOESummaryFile << "link_id,from_node_id,to_node_id,type_code,start_time_in_min,end_time_in_min,AB_total_link_volume,BA_total_link_volume" << endl;

	std::set<DTALink*>::iterator iterLink;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		DTALink* pLink = g_LinkVector[li];

		double AB_flow = pLink->CFlowArrivalCount;

		double BA_flow = 0;
		
		string BA_link_string = GetLinkStringID( pLink->m_ToNodeNumber,pLink->m_FromNodeNumber);
		if(g_LinkMap.find(BA_link_string)!= g_LinkMap.end())
			{
			DTALink* pLinkBA = g_LinkMap[BA_link_string];

			BA_flow = pLinkBA->CFlowArrivalCount;

			}

		pLink->CFlowArrivalCount;

		double average_travel_time = pLink->GetTravelTimeByMin(g_NumberOfIterations,0, pLink->m_SimulationHorizon);
		double speed = pLink->m_Length / max(0.00001,average_travel_time) *60;  // unit: mph
		double capacity_simulation_horizon = pLink->m_LaneCapacity * pLink->m_NumLanes;
		double voc_ratio = pLink->CFlowArrivalCount / max(0.1,capacity_simulation_horizon);
		int percentage_of_speed_limit = int(speed/max(0.1,pLink->m_SpeedLimit)*100+0.5);

		if(pLink->m_link_code ==1)  // AB
		{
			LinkMOESummaryFile << pLink->m_OrgLinkID    << "," ;
			LinkMOESummaryFile << g_NodeVector[pLink->m_FromNodeID].m_NodeName  << "," ;
			LinkMOESummaryFile << g_NodeVector[pLink->m_ToNodeID].m_NodeName << "," ;
			LinkMOESummaryFile << g_LinkTypeMap[pLink->m_link_type].type_code.c_str ()<< "," ;
			LinkMOESummaryFile << g_DemandLoadingStartTimeInMin << "," ;
			LinkMOESummaryFile << g_DemandLoadingEndTimeInMin << "," ;
			LinkMOESummaryFile << pLink->CFlowArrivalCount << "," ; // total hourly arrival flow 
			LinkMOESummaryFile << BA_flow << "," ; // total hourly arrival flow 
			LinkMOESummaryFile << endl;

		}
	}

}

void OutputVehicleTrajectoryData(char fname[_MAX_PATH],int Iteration, bool bStartWithEmpty)
{

	FILE* st_agent = NULL;

	FILE* st_struct = NULL;
	FILE* st_path_link_sequence = NULL;
	FILE* st_path = NULL;

	fopen_s(&st_agent,fname,"w");

	fopen_s(&st_struct,"agent.bin","wb");
	fopen_s(&st_path_link_sequence,"output_path_link_sequence.csv","w");
	fopen_s(&st_path,"output_path.csv","w");
	typedef struct  
	{
		int vehicle_id;
		int from_zone_id;
		int to_zone_id;
		float departure_time;
		float arrival_time;
		int complete_flag;
		float trip_time;
		int demand_type;
		int pricing_type;
		int vehicle_type;
		int information_type;
		float value_of_time;
		float toll_cost_in_dollar;
		float emissions;
		float distance_in_mile;
		int number_of_nodes;
		float Energy;
		float CO2;
		float NOX;
		float CO;
		float HC;
	} struct_Vehicle_Header;

	typedef  struct  
	{
		int NodeName;
		float AbsArrivalTimeOnDSN;
	} struct_Vehicle_Node;

	if(st_agent!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;
		int VehicleCount_withPhysicalPath = 0;
		// output statistics
		fprintf(st_agent, "agent_id,from_zone_id,to_zone_id,departure_time,arrival_time,complete_flag,trip_time,demand_type,pricing_type,vehicle_type,information_type,value_of_time,toll_cost_in_dollar,emissions,distance_in_mile,TotalEnergy_(KJ),CO2_(g),NOX_(g),CO_(g),HC_(g),number_of_nodes,path_sequence\n");

		if(st_path_link_sequence!=NULL)
			fprintf(st_path_link_sequence,"vehicle_id,from_zone_id,to_zone_id,link_sequence,from_node_id->to_node_id,link_length_in_miles,speed_limit,link_type,with_signal_flag,from_node_time_stamp_in_min,free_flow_travel_time_in_min,travel_time_in_min,delay_in_min\n");

		if(st_path!=NULL)
			fprintf(st_path,"vehicle_id,from_zone_id,to_zone_id,pricing_type,number_of_nodes,path_sequence\n");

		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;
			if(pVehicle->m_NodeSize >= 2)  // with physical path in the network
			{
				int UpstreamNodeID = 0;
				int DownstreamNodeID = 0;

				int LinkID_0 = pVehicle->m_NodeAry [0].LinkNo;
				UpstreamNodeID= g_LinkVector[LinkID_0]->m_FromNodeID;
				DownstreamNodeID = g_LinkVector[LinkID_0]->m_ToNodeID;

				float TripTime = 0;

				if(pVehicle->m_bComplete)
					TripTime = pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime;


				float m_gap = 0;
				fprintf(st_agent,"%d,%d,%d,%4.2f,%4.2f,%d,%4.2f,%d,%d,%d,%d,%4.2f,%4.3f,%4.3f,%4.3f,%f,%f,%f,%f,%f,%d,",
					pVehicle->m_VehicleID , pVehicle->m_OriginZoneID , pVehicle->m_DestinationZoneID,
					pVehicle->m_DepartureTime, pVehicle->m_ArrivalTime , pVehicle->m_bComplete, TripTime,			
					pVehicle->m_DemandType, pVehicle->m_PricingType ,pVehicle->m_VehicleType,
					pVehicle->m_InformationClass, 
					pVehicle->m_VOT , pVehicle->m_TollDollarCost, pVehicle->m_Emissions ,pVehicle->m_Distance, 
					pVehicle->Energy ,pVehicle->CO2, pVehicle->NOX, pVehicle->CO, pVehicle->HC,
					pVehicle->m_NodeSize);

				if(st_path!=NULL)
				{
					fprintf(st_path,"v%d,o%d,d%d,t%d,n%d,", pVehicle->m_VehicleID,pVehicle->m_OriginZoneID ,pVehicle->m_DestinationZoneID, pVehicle->m_PricingType , pVehicle->m_NodeSize);
				}
				struct_Vehicle_Header header;
				header.vehicle_id = pVehicle->m_VehicleID;
				header.from_zone_id = pVehicle->m_OriginZoneID;
				header. to_zone_id = pVehicle->m_DestinationZoneID;
				header. departure_time = pVehicle->m_DepartureTime;
				header. arrival_time = pVehicle->m_ArrivalTime;
				header. complete_flag = pVehicle->m_bComplete;
				header. trip_time = TripTime;
				header. demand_type = pVehicle->m_DemandType;
				header. pricing_type = pVehicle->m_PricingType;
				header. vehicle_type =pVehicle->m_VehicleType;
				header. information_type = pVehicle->m_InformationClass;
				header. value_of_time =pVehicle->m_VOT;
				header. toll_cost_in_dollar = pVehicle->m_TollDollarCost;
				header. emissions = pVehicle->m_Emissions;
				header. distance_in_mile = pVehicle->m_Distance;
				header. Energy = pVehicle->Energy;
				header.CO2 = pVehicle->CO2;
				header.NOX = pVehicle->NOX;
				header.CO =  pVehicle->CO;
				header.HC = pVehicle->HC;
				header. number_of_nodes = pVehicle->m_NodeSize;

				struct_VehicleInfo_Header InfoHeaderAsVehicleInput;
				InfoHeaderAsVehicleInput.vehicle_id = pVehicle->m_VehicleID;
				InfoHeaderAsVehicleInput.from_zone_id = pVehicle->m_OriginZoneID;
				InfoHeaderAsVehicleInput. to_zone_id = pVehicle->m_DestinationZoneID;
				InfoHeaderAsVehicleInput. departure_time = pVehicle->m_DepartureTime;
				InfoHeaderAsVehicleInput. demand_type = pVehicle->m_DemandType;
				InfoHeaderAsVehicleInput. pricing_type = pVehicle->m_PricingType;
				InfoHeaderAsVehicleInput. vehicle_type =pVehicle->m_VehicleType;
				InfoHeaderAsVehicleInput. information_type = pVehicle->m_InformationClass;
				InfoHeaderAsVehicleInput. value_of_time =pVehicle->m_VOT;

				fwrite(&header, sizeof(struct_Vehicle_Header), 1, st_struct);


				fprintf(st_agent, "\"");

				int j = 0;
				if(g_LinkVector[pVehicle->m_NodeAry [0].LinkNo]==NULL)
				{

					cout << "Error: vehicle" << pVehicle->m_VehicleID << "at LinkID"<< pVehicle->m_NodeAry [0].LinkNo << endl;
					cin.get();  // pause

				}

				int NodeID = g_LinkVector[pVehicle->m_NodeAry [0].LinkNo]->m_FromNodeID;  // first node
				int NodeName = g_NodeVector[NodeID].m_NodeName ;
				float link_entering_time = pVehicle->m_DepartureTime;
				fprintf(st_agent, "<%d;%4.2f;0;0>",
					NodeName,pVehicle->m_DepartureTime) ;

				if(st_path!=NULL)
				{
					fprintf(st_path,"%d,", NodeName);
				}


				struct_Vehicle_Node node_element;
				node_element. NodeName = NodeName;
				node_element. AbsArrivalTimeOnDSN = pVehicle->m_DepartureTime;
				fwrite(&node_element, sizeof(node_element), 1, st_struct);

				float LinkWaitingTime = 0;
				for(j = 0; j< pVehicle->m_NodeSize-1; j++)  // for all nodes
				{
					int LinkID = pVehicle->m_NodeAry [j].LinkNo;
					int NodeID = g_LinkVector[LinkID]->m_ToNodeID;
					int NodeName = g_NodeVector[NodeID].m_NodeName ;
					float LinkTravelTime = 0;
					float Emissions = 0;


					if(j==0) // origin node
					{
						link_entering_time =  pVehicle->m_DepartureTime;
					}
					else
					{
						link_entering_time = pVehicle->m_NodeAry [j-1].AbsArrivalTimeOnDSN;
					}

						LinkTravelTime = (pVehicle->m_NodeAry [j].AbsArrivalTimeOnDSN) - link_entering_time;
						LinkWaitingTime = pVehicle->m_NodeAry [j].AbsArrivalTimeOnDSN - link_entering_time - g_LinkVector[LinkID]->m_FreeFlowTravelTime ;
				
							int FromNodeID = g_LinkVector[LinkID]->m_FromNodeID;
							int FromNodeNumber = g_NodeVector[FromNodeID].m_NodeName ;

							int ToNodeID = g_LinkVector[LinkID]->m_ToNodeID;
							int ToNodeNumber = g_NodeVector[ToNodeID].m_NodeName ;

						if(j < pVehicle->m_NodeSize-2 &&  pVehicle->m_bComplete ) // we have not reached the destination yest
						{

							int NextLinkID = pVehicle->m_NodeAry [j+1].LinkNo;
							int NextNodeID = g_LinkVector[NextLinkID]->m_ToNodeID;
							int DestNodeNumber = g_NodeVector[NextNodeID].m_NodeName ;

							// construct movement id
							string movement_id = GetMovementStringID(FromNodeNumber, NodeName,DestNodeNumber);
							if(g_NodeVector[ToNodeID].m_MovementMap.find(movement_id) != g_NodeVector[ToNodeID].m_MovementMap.end()) // the capacity for this movement has been defined
							{

								g_NodeVector[ToNodeID].m_MovementMap[movement_id].total_vehicle_count++;
								g_NodeVector[ToNodeID].m_MovementMap[movement_id].total_vehicle_delay += LinkWaitingTime;

								TRACE("movement: %d, %f, link travel time %f\n", g_NodeVector[ToNodeID].m_MovementMap[movement_id].total_vehicle_count,g_NodeVector[ToNodeID].m_MovementMap[movement_id].total_vehicle_delay,LinkWaitingTime);

							}


						}

							if(st_path_link_sequence!=NULL)
							{
								//link_length_in_miles,speed_limit,link_type,with_signal_flag,
							
									if(pVehicle->m_bComplete)
										fprintf(st_path_link_sequence,"v%d,", pVehicle->m_VehicleID);
									else
										fprintf(st_path_link_sequence,"v_incomplete_%d,", pVehicle->m_VehicleID);

									fprintf(st_path_link_sequence,"%d,%d,%d,%d->%d,%.2f,%.0f,%s,%d,%.2f,%.2f,%.2f,%.2f\n",
										pVehicle->m_OriginZoneID ,pVehicle->m_DestinationZoneID,j,FromNodeNumber,ToNodeNumber, g_LinkVector[LinkID]->m_Length , g_LinkVector[LinkID]->m_SpeedLimit , g_LinkTypeMap[g_LinkVector[LinkID]->m_link_type].link_type_name.c_str (), g_LinkVector[LinkID]->m_bSignalizedArterialType,
									pVehicle->m_NodeAry [j].AbsArrivalTimeOnDSN,
									g_LinkVector[LinkID]->m_FreeFlowTravelTime ,LinkTravelTime,LinkWaitingTime);
							}


							//						fprintf(st_agent, ",,,,,,,,,,,,,,%d,%d%,%6.2f,%6.2f,%6.2f\n", j+2,NodeName,pVehicle->m_NodeAry [j].AbsArrivalTimeOnDSN,LinkWaitingTime, g_LinkVector[LinkID]->m_LinkMOEAry [link_entering_time].TravelTime ) ;
							fprintf(st_agent, "<%d; %4.2f;%4.2f;%4.2f>",NodeName,pVehicle->m_NodeAry [j].AbsArrivalTimeOnDSN, LinkTravelTime,Emissions) ;
							if(st_path!=NULL)
							{
								fprintf(st_path,"%d,", NodeName);
							}

							node_element. NodeName = NodeName;
							node_element. AbsArrivalTimeOnDSN = pVehicle->m_NodeAry [j].AbsArrivalTimeOnDSN;
							fwrite(&node_element, sizeof(node_element), 1, st_struct);

				
				} //for all nodes in path
				fprintf(st_agent,"\"\n");
				fprintf(st_path,"\n");
			}else
				{// without physical path in the network
				float TripTime = 0;


				fprintf(st_agent,"%d,%d,%d,%4.2f,%4.2f,%d,%4.2f,%d,%d,%d,%d,%4.2f,%4.3f,%4.3f,%4.3f,%f,%f,%f,%f,%f,%d\n",
					pVehicle->m_VehicleID , pVehicle->m_OriginZoneID , pVehicle->m_DestinationZoneID,
					pVehicle->m_DepartureTime, pVehicle->m_ArrivalTime , pVehicle->m_bComplete, TripTime,			
					pVehicle->m_DemandType, pVehicle->m_PricingType ,pVehicle->m_VehicleType,
					pVehicle->m_InformationClass, 
					pVehicle->m_VOT , pVehicle->m_TollDollarCost, pVehicle->m_Emissions ,pVehicle->m_Distance, 
					pVehicle->Energy ,pVehicle->CO2, pVehicle->NOX, pVehicle->CO, pVehicle->HC,
					pVehicle->m_NodeSize);

				// write everything out, without path sequence
				}
			//if(pVehicle->m_bLoaded == false) 
			//{
			//	cout << "Warning: Not loaded vehicle " << pVehicle->m_VehicleID << " from zone " << 
			//		pVehicle->m_OriginZoneID << " to zone " << pVehicle->m_DestinationZoneID << " departing at"
			//		<< pVehicle->m_DepartureTime << " demand type = " << (int)(pVehicle->m_DemandType) << " Node Size in path = " <<  pVehicle->m_NodeSize << endl;
			//}
		} // for all paths

		// not loaded in simulation


		fclose(st_agent);
		fclose(st_struct);

		if(st_path_link_sequence!=NULL)
			fclose(st_path_link_sequence);

		if(st_path!=NULL)
			fclose(st_path);

	}else
	{
		fprintf(g_ErrorFile, "File output_agent.csv cannot be opened. It might be currently used and locked by EXCEL.");
		cout << "Error: File output_agent.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		getchar();
	}
}


bool g_ReadAgentBinFile(string file_name)
{
	int m_PathNodeVectorSP[MAX_NODE_SIZE_IN_A_PATH];

	g_AggregationTimetIntervalSize = max(1,(g_DemandLoadingEndTimeInMin)/g_AggregationTimetInterval);
	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ODZoneSize+1, g_AggregationTimetIntervalSize);


	typedef struct  
	{
		int vehicle_id;
		int from_zone_id;
		int to_zone_id;
		float departure_time;
		float arrival_time;
		int complete_flag;
		float trip_time;
		int demand_type;
		int pricing_type;
		int vehicle_type;
		int information_type;
		float value_of_time;
		float toll_cost_in_dollar;
		float emissions;
		float distance_in_mile;
		int number_of_nodes;
	float Energy;
	float CO2;
	float NOX;
	float CO;
	float HC;
	} struct_Vehicle_Header;

	typedef  struct  
	{
		int NodeName;
		float AbsArrivalTimeOnDSN;
	} struct_Vehicle_Node;

	FILE* st = NULL;
	fopen_s(&st,file_name.c_str (),"rb");
	if(st!=NULL)
	{
		struct_Vehicle_Header header;

		int count =0;
		while(!feof(st))
		{

			size_t result = fread(&header,sizeof(struct_Vehicle_Header),1,st);

			if( header.vehicle_id < 0)
				break;

			if(result!=1)  // read end of file
				break;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;

			pVehicle->m_VehicleID		= header.vehicle_id;
			pVehicle->m_OriginZoneID	= header.from_zone_id;
			pVehicle->m_DestinationZoneID= header.to_zone_id;

			g_ZoneMap[pVehicle->m_OriginZoneID].m_Demand += 1;
			g_ZoneMap[pVehicle->m_OriginZoneID].m_OriginVehicleSize += 1;


			pVehicle->m_DepartureTime	=  header.departure_time;
			pVehicle->m_PreferredDepartureTime = header.departure_time;
			pVehicle->m_ArrivalTime =  header.arrival_time;

			pVehicle->m_TripTime  = header.trip_time;

			pVehicle->m_DemandType = header.demand_type;
			pVehicle->m_PricingType = header.pricing_type;

			pVehicle->m_VehicleType = header.vehicle_type;
			pVehicle->m_InformationClass = header.information_type;
			pVehicle->m_VOT = header.value_of_time;

			pVehicle->m_TimeToRetrieveInfo = (int)(pVehicle->m_DepartureTime*10);

			pVehicle->m_NodeSize = header.number_of_nodes;
			pVehicle->m_ArrivalTime  = 0;
			pVehicle->m_bComplete = false;
			pVehicle->m_bLoaded  = false;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions  = 0;
			pVehicle->m_Distance = 0;
			pVehicle->m_NodeNumberSum =0;

			if(pVehicle->m_NodeSize>=1)  // in case reading error
			{
				pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

				pVehicle->m_NodeNumberSum = 0;
				for(int i=0; i< pVehicle->m_NodeSize; i++)
				{

					int node_id;
					float time_stamp,travel_time, emissions;

					struct_Vehicle_Node node_element;
					fread(&node_element,sizeof(node_element),1,st);

					m_PathNodeVectorSP[i] = node_element.NodeName;
					pVehicle->m_NodeNumberSum += m_PathNodeVectorSP[i];

					if(i==0)
						pVehicle->m_OriginNodeID = g_NodeNametoIDMap[m_PathNodeVectorSP[0]];

					if(i==pVehicle->m_NodeSize-1)
						pVehicle->m_DestinationNodeID   = g_NodeNametoIDMap[m_PathNodeVectorSP[pVehicle->m_NodeSize-1]];

					if(i>=1)
					{
						DTALink* pLink = g_LinkMap[GetLinkStringID(m_PathNodeVectorSP[i-1],m_PathNodeVectorSP[i])];
						if(pLink==NULL)
						{
							AfxMessageBox("Error in reading file agent.bin");
							fclose(st);

							return false;
						}
						pVehicle->m_NodeAry[i-1].LinkNo  = pLink->m_LinkNo  ; // start from 0
					}


				}

			g_VehicleVector.push_back(pVehicle);
			g_VehicleMap[pVehicle->m_VehicleID ]  = pVehicle;

			int AssignmentInterval = int(pVehicle->m_DepartureTime/g_AggregationTimetInterval);

			if(AssignmentInterval >= g_AggregationTimetIntervalSize)
			{
				AssignmentInterval = g_AggregationTimetIntervalSize - 1;
			}
			g_TDOVehicleArray[pVehicle->m_OriginZoneID][AssignmentInterval].VehicleArray .push_back(pVehicle->m_VehicleID);

			count++;

			if(count%10000==0)
				cout << "reading " << count/1000 << "K agents from binary file " << file_name << endl;
			} 
		}

		fclose(st);
		return true;

	}
	return false;
}