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

#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>
using namespace std;

/*************************************
How to build a simple DTA simulator

step 0: basic input functions


defintions of DTANode, DTALink and DTAVehicle
utility function g_read_integer() g_read_float()
ReadNetworkData() and ReadVehicleData()

step 1: dynamic memory management
FreeMemory() to deallocate dynamic memory for the whole simulation program
Allocate and deallocate dynamic arrays for DTANetworkForSP

step 2: network building for traffic assignment
DTANetworkForSP::BuildNetwork(int ZoneID)

step 3: convert time-dependent OD demand to vehicles data, sorted by departure time
CreateVehicles(int originput_zone, int destination_zone, float number_of_vehicles, int vehicle_type, float starting_time_in_min, float ending_time_in_min)


step 4: Scan eligiable list and shortest path algorithm
SEList functions in DTANetworkForSP
TDLabelCorrecting_DoubleQueue(int origin, int departure_time)

step 5: assign path to vehicles
GetLinkNoByNodeIndex(int usn_index, int dsn_index)
VehicleBasedPathAssignment(int zone,int departure_time_begin, int departure_time_end, int iteration)

step 6: integerate network building, shortest path algorithm and path assignment to dynamic traffic assignment

step 7: output vehicle trajectory file

step 8: NetworkLoading

step 9: VehicularSimulation

step 10: parallel computing: assign vehicles to arrays with different origins/departure times, to speed up the calculation

step 11: parallel computing for different zones
OpenMP

step 12: prepare dynamic arrays for within-simulation shortest path for pre-trp and enroute info vehicles
store the data for each destination node to global array as follows.
//int ***g_RTNodePredAry;  // vehicle type, destination, node available in real-time simulation
//unsigned char *** g_RTLabelCostAry; // vehicle type, destination, node available in real-time simulation


Step 13:
allow multiple centriods per zone in zone.csv, change BuildNetwork()

step 13: stochastic capacity with random numbers for different zones

step 14: UE, SO algorithm, day to day learning algorithm
updating, estimating variance and making prediction

make this as a convergency process

route choice -> compare threshold

Step 15: GetDynamicCapacityAtSignalizedIntersection
Add dynamical outflow capacity for signalized intersection

step 16: assign pre-trip info shortest path to vehicles

habitual paths vs. real time info
information provides


feature functions: link-based shortest path to consider turnning panalty

*************************************/

/**************************
menu -> project -> property -> configuraiton -> debugging -> setup working directory

***************************/
// The one and only application object

CWinApp theApp;
std::vector<DTANode> g_NodeVector;
std::map<int, int> g_NodeNametoIDMap;


std::vector<DTALink*> g_LinkVector;


std::vector<DTAZone> g_ZoneVector;
std::vector<DTAVehicle*>		g_VehicleVector;
std::map<int, DTAVehicle*> g_VehicleMap;

// time inteval settings in assignment and simulation
double g_DTASimulationInterval = 0.10000; // min
int g_number_of_intervals_per_min = 10; // round to nearest integer
int g_DepartureTimetInterval = 5; // min
int g_DepartureTimetIntervalSize = 2;
float g_DemandGlobalMultiplier = 1.0f;

// maximal # of adjacent links of a node (including physical nodes and centriods( with connectors))
int g_AdjLinkSize = 30; // initial value of adjacent links

int g_ODZoneSize = 0;

// assignment and simulation settings
int g_NumberOfIterations = 1;
int g_AgentBasedAssignmentFlag = 0;
float g_ConvergencyRelativeGapThreshold_in_perc;
int g_NumberOfInnerIterations;

int g_DemandLoadingHorizon = 60; // min
int g_VehicleLoadingMode = 0; // not load from vehicle file by default, 1: load vehicle file
int g_SimulationHorizon = 120;
int g_DemandHorizonForLoadedVehicles = 60; // min

int g_ObservationTimeInterval = 1;  //min 
int g_ObservationStartTime = 390; // min
int g_ObservationEndTime = 570; // min
// assignment
int g_UEAssignmentMethod = 0; // 0: MSA, 1: day-to-day learning, 2: GAP-based switching rule for UE, 3: Gap-based switching rule + MSA step size for UE
float g_CurrentGapValue = 0.0; // total network gap value in the current iteration
float g_PrevGapValue = 0.0; // total network gap value in last iteration
float g_RelativeGap = 0.0; // = abs(g_CurrentGapValue - g_PrevGapValue) / g_PrevGapValue 
int g_CurrentNumOfVehiclesSwitched = 0; // total number of vehicles switching paths in the current iteration; for MSA, g_UEAssignmentMethod = 0
int g_PrevNumOfVehiclesSwitched = 0; // // total number of vehicles switching paths in last iteration; for MSA, g_UEAssignmentMethod = 0
int g_ConvergenceThreshold_in_Num_Switch; // the convergence threshold in terms of number of vehicles switching paths; for MSA, g_UEAssignmentMethod = 0
int g_VehicleExperiencedTimeGap = 1; // 1: Vehicle experienced time gap; 0: Avg experienced path time gap
int g_NewPathWithSwitchedVehicles = 0; // number of new paths with vehicles switched to them

float g_TotalDemandDeviation = 0;
float g_TotalMeasurementDeviation = 0; 

float g_UserClassPercentage[MAX_SIZE_INFO_USERS] = {0};
float g_UserClassPerceptionErrorRatio[MAX_SIZE_INFO_USERS] = {0};
float g_VMSPerceptionErrorRatio;

int g_information_updating_interval_of_en_route_info_travelers_in_min;

float g_VOT[MAX_VEHICLE_TYPE_SIZE];
float g_VehicleTypePercentage[MAX_VEHICLE_TYPE_SIZE];

int g_LearningPercentage = 15;
int g_TravelTimeDifferenceForSwitching = 5;  // min

int g_StochasticCapacityMode = 0;
float g_FreewayJamDensity_in_vehpmpl = 220;
float g_NonFreewayJamDensity_in_vehpmpl = 120;
float g_BackwardWaveSpeed_in_mph = 12;
float g_MinimumInFlowRatio = 0.1f;
float g_MaxDensityRatioForVehicleLoading = 0.8f;
int g_CycleLength_in_seconds;
float g_DefaultSaturationFlowRate_in_vehphpl;


VOTStatistics g_VOTStatVector[MAX_VOT_RANGE];

int g_Number_of_CompletedVehicles = 0;
int g_Number_of_CompletedVehiclesThatSwitch = 0;
int g_Number_of_GeneratedVehicles = 0;

int g_InfoTypeSize  = 1;  // for shortest path with generalized costs depending on LOV, HOV, trucks or other vehicle classes.
int g_start_iteration_for_MOEoutput = 0;

// for fast data acessing
int g_LastLoadedVehicleID = 0; // scan vehicles to be loaded in a simulation interval

VehicleArrayForOriginDepartrureTimeInterval** g_TDOVehicleArray; // TDO for time-dependent origin

std::vector<DTA_vhc_simple>   g_simple_vector_vehicles;	// vector of DSP_Vehicle, not pointer!;

FILE* g_ErrorFile = NULL;
ofstream g_LogFile;
ofstream g_AssignmentLogFile;
ofstream g_EstimationLogFile;
ofstream g_WarningFile;

int g_TrafficFlowModelFlag = 0;

int g_TollingMethodFlag = 0;
float g_VMTTollingRate = 0;

int g_MergeNodeModelFlag=1;

std::vector<NetworkMOE>  g_NetworkMOEAry;
std::vector<NetworkLoadingOutput>  g_AssignmentMOEAry;

using namespace std;

CTime g_AppStartTime;
unsigned int g_RandomSeed = 100;
unsigned int g_RandomSeedForVehicleGeneration = 101;
// Linear congruential generator 
#define g_LCG_a 17364
#define g_LCG_c 0
#define g_LCG_M 65521  

float g_GetRandomRatio()
{
	//		g_RandomSeed = (g_LCG_a * g_RandomSeed + g_LCG_c) % g_LCG_M;  //m_RandomSeed is automatically updated.

	//		return float(g_RandomSeed)/g_LCG_M;

	return float(rand())/RAND_MAX;
}

int g_GetRandomInteger_From_FloatingPointValue(float Value)
{
	float Residual = Value -int(Value);
	float RandomNumber= g_GetRandomRatio(); 
	if(RandomNumber < Residual)
	{
		return int(Value)+1;
	}
	else
	{
		return int(Value);
	}
}

float g_GetRandomRatioForVehicleGeneration()
{
	g_RandomSeedForVehicleGeneration = (g_LCG_a * g_RandomSeedForVehicleGeneration + g_LCG_c) % g_LCG_M;  //m_RandomSeed is automatically updated.

	return float(g_RandomSeedForVehicleGeneration)/g_LCG_M;

}


int  DTANetworkForSP:: GetLinkNoByNodeIndex(int usn_index, int dsn_index)
{
	int LinkNo = -1;
	for(int i=0; i < m_OutboundSizeAry[usn_index]; i++)
	{

		if(m_OutboundNodeAry[usn_index][i] == dsn_index)
		{
			LinkNo = m_OutboundLinkAry[usn_index][i];
			return LinkNo;
		}
	}

	cout << " Error in GetLinkNoByNodeIndex " << g_NodeVector[usn_index].m_NodeName  << "-> " << g_NodeVector[dsn_index].m_NodeName ;

	g_ProgramStop();


	return MAX_LINK_NO;


}

void ConnectivityChecking(DTANetworkForSP* pPhysicalNetwork)
{
	// network connectivity checking


	unsigned int i;
	int OriginForTesting=0;
	for(i=0; i< g_NodeVector.size(); i++)
	{
		if(pPhysicalNetwork->m_OutboundSizeAry [i] >0)
		{
			OriginForTesting = i;
			break;
		}
	}

	// starting with first node with origin nodes;
	pPhysicalNetwork->TDLabelCorrecting_DoubleQueue(OriginForTesting,0,1);  // CurNodeID is the node ID
	// assign shortest path calculation results to label array


	int count = 0;
	int centroid_count = 0;
	for(i=0; i< g_NodeVector.size(); i++)
	{
		if(pPhysicalNetwork->LabelCostAry[i] > MAX_SPLABEL-100)
		{

			if(g_NodeVector[i].m_ZoneID > 0)
			{
				cout << "Centroid "<<  g_NodeVector[i].m_NodeName  << " of zone " << g_NodeVector[i].m_ZoneID << " is not connected to node " << g_NodeVector[OriginForTesting].m_NodeName  << endl;
				g_WarningFile << "Centroid "<<  g_NodeVector[i].m_NodeName  << " of zone " << g_NodeVector[i].m_ZoneID << " is not connected to node " << g_NodeVector[OriginForTesting].m_NodeName  << endl;
				centroid_count ++;
			}else
			{
				cout << "Node "<<  g_NodeVector[i].m_NodeName  << " is not connected to node " << g_NodeVector[OriginForTesting].m_NodeName  << endl;
				g_WarningFile << "Node "<<  g_NodeVector[i].m_NodeName  << " is not connected to node " << g_NodeVector[OriginForTesting].m_NodeName  <<", Cost: "  << endl;
			}
			count++;

		}

	}

//	for(i=0; i< g_NodeVector.size(); i++)
//	{
//		g_WarningFile << "Node "<<  g_NodeVector[i] << " Cost: " << pPhysicalNetwork->LabelCostAry[i] << endl;
//	}
	if(count > 0)
	{
		cout << count << " nodes are not connected to "<< g_NodeVector[OriginForTesting].m_NodeName  << endl;
		g_WarningFile << count << " nodes are not connected to "<< g_NodeVector[OriginForTesting].m_NodeName  << endl;

		if(centroid_count > 0 )
		{
			cout << centroid_count << " controids are not connected to "<< g_NodeVector[OriginForTesting].m_NodeName  << endl;
			g_WarningFile << centroid_count << " controids are not connected to "<< g_NodeVector[OriginForTesting].m_NodeName  << endl;
			//			cout << "Please check file warning.log later. Press any key to continue..."<< endl;
			//getchar();
		}
	}

}
void ReadInputFiles()
{
	int z,t;
	FILE* st = NULL;
	cout << "Reading file input_node.csv..."<< endl;

	fopen_s(&st,"input_node.csv","r");
	if(st!=NULL)
	{
		int i=0;
		while(!feof(st))
		{
			DTANode Node;
			int name			= g_read_integer(st);
			TRACE("node %d\n ", name);

			if(name == -1)  // reach end of file
				break;

			float x	= g_read_float(st);
			float y	= g_read_float(st);
			// Create and insert the node
			Node.m_NodeID = i;
			Node.m_ZoneID = 0;
			Node.m_NodeName = name;
			g_NodeVector.push_back(Node);
			g_NodeNametoIDMap[name] = i;
			i++;
		}
		fclose(st);
	}else
	{
		cout << "Error: File input_node.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();

	}
	cout << "Reading file input_link.csv..."<< endl;

	fopen_s(&st,"input_link.csv","r");

	int i = 0;
	if(st!=NULL)
	{
		DTALink* pLink = 0;
		while(!feof(st))
		{
			int FromID =  g_read_integer(st);
			if(FromID == -1)  // reach end of file
				break;
			int ToID = g_read_integer(st);

			pLink = new DTALink(g_SimulationHorizon);
			pLink->m_LinkID = i;
			pLink->m_FromNodeNumber = FromID;
			pLink->m_ToNodeNumber = ToID;
			pLink->m_FromNodeID = g_NodeNametoIDMap[pLink->m_FromNodeNumber ];
			pLink->m_ToNodeID= g_NodeNametoIDMap[pLink->m_ToNodeNumber];
			float length = g_read_float(st);
			pLink->m_NumLanes= g_read_integer(st);
			pLink->m_SpeedLimit= g_read_float(st);
			pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance
			pLink->m_MaximumServiceFlowRatePHPL= g_read_float(st);
			pLink->m_BPRLaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
			pLink->m_link_type= g_read_integer(st);

			if(pLink->m_link_type == 1) // Freeway
			{
				pLink->m_VehicleSpaceCapacity = pLink->m_Length * pLink->m_NumLanes *g_FreewayJamDensity_in_vehpmpl; // bump to bump density
			}
			else // nonfreeway
			{
				pLink->m_VehicleSpaceCapacity = pLink->m_Length * pLink->m_NumLanes *g_NonFreewayJamDensity_in_vehpmpl; // bump to bump density
			}


			g_NodeVector[pLink->m_FromNodeID ].m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);

			pLink->SetupMOE();
			g_LinkVector.push_back(pLink);
			i++;

			if(i == MAX_LINK_NO)
			{
				cout << "The network has more than "<< MAX_LINK_NO << " links."<< endl <<"Please contact the developers for a new 64 bit version for this large-scale network." << endl;
				getchar();
				exit(0);

			}

		}
		fclose(st);
	}else
	{
		cout << "Error: File input_link.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}

	cout << "Reading file input_zone.csv..."<< endl;

	// step 1: check how many zones.
	fopen_s(&st,"input_zone.csv","r");
	if(st!=NULL)
	{
		int zone_number = 0;

		while(!feof(st))
		{
			zone_number	= g_read_integer(st);
			if(zone_number == -1)  // reach end of file
				break;

			if(g_ODZoneSize < zone_number)
				g_ODZoneSize = zone_number;

			int node_number= g_read_integer(st);
			g_NodeVector[g_NodeNametoIDMap[node_number]].m_ZoneID = zone_number;
		}
		fclose(st);
	}else
	{
		cout << "Error: File zone.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}
	g_ZoneVector.resize (g_ODZoneSize+1);

	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ODZoneSize+1, g_DepartureTimetIntervalSize);

	// step 2: check how many centroids

	fopen_s(&st,"input_zone.csv","r");
	if(st!=NULL)
	{
		int zone_number = 0;

		while(!feof(st))
		{
			zone_number	= g_read_integer(st);
			if(zone_number == -1)  // reach end of file
				break;
			int node_number= g_read_integer(st);

			g_ZoneVector[zone_number].m_CentroidNodeAry.push_back( g_NodeNametoIDMap[node_number]);
			g_ZoneVector[zone_number].m_Capacity += g_NodeVector[g_NodeNametoIDMap[node_number]].m_TotalCapacity ;

			if(g_ZoneVector[zone_number].m_CentroidNodeAry.size() > g_AdjLinkSize)
				g_AdjLinkSize = g_ZoneVector[zone_number].m_CentroidNodeAry.size();
		}
		fclose(st);
	}

	//	cout << "Done with zone.csv"<< endl;

	// done with zone.csv
	DTANetworkForSP PhysicalNetwork(g_NodeVector.size(), g_LinkVector.size(), g_SimulationHorizon,g_AdjLinkSize);  //  network instance for single processor in multi-thread environment
	PhysicalNetwork.BuildPhysicalNetwork();
	PhysicalNetwork.IdentifyBottlenecks(g_StochasticCapacityMode);

	cout << "Reading file incident.dat..."<< endl;

	fopen_s(&st,"incident.dat","r");
	if(st!=NULL)
	{
		int NumberofCapacityReductionLinks = g_read_integer(st);

		if(NumberofCapacityReductionLinks >0)
		{
			g_LogFile << "# of capacity reduction links = " << NumberofCapacityReductionLinks << endl;
		}


		for(i = 0; i < NumberofCapacityReductionLinks; i++)
		{
			int usn  = g_read_integer(st);
			int dsn =  g_read_integer(st);

			int LinkID = PhysicalNetwork.GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

			DTALink* plink = g_LinkVector[LinkID];

			if(plink!=NULL)
			{
				CapacityReduction cs;

				cs.StartTime = g_read_integer(st);
				cs.EndTime = g_read_integer(st);
				cs.LaneClosureRatio= g_read_float(st);
				plink->CapacityReductionVector.push_back(cs);
			}
		}

		fclose(st);
	}


	fopen_s(&st,"VMS.dat","r");
	if(st!=NULL)
	{
	cout << "Reading file incident.dat..."<< endl;

		int NumberofVMSs = g_read_integer(st);

		if(NumberofVMSs >0)
		{
			g_LogFile << "# of variable message signs = " << NumberofVMSs << endl;
		}


		for(i = 0; i < NumberofVMSs; i++)
		{
			int type  = g_read_integer(st);

			if(!(type== 2 || type== 3))
			{
				cout << "Error: A VMS with type 1 or 4 is input, but DTALite currently only models congestion warning and detour VMSs."<< endl;
				g_ProgramStop();

				fclose(st);
			}
			int usn  = g_read_integer(st);
			int dsn =  g_read_integer(st);
			int LinkID = PhysicalNetwork.GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

			DTALink* plink = g_LinkVector[LinkID];

			if(plink!=NULL)
			{
				InformationSign is;

				is.Type = type;
				if(type == 2)
				{
				is.ResponsePercentage =  g_read_float(st);
				is.DetourLinkSize   =  g_read_integer(st) -1;
				is.StartTime = g_read_integer(st);
				is.EndTime = g_read_integer(st);
				
				int NodeList[100];
				int n;
				for(n=0; n<= is.DetourLinkSize; n++)
				{
				NodeList[n] =  g_read_integer(st);
				}

				for(n=0; n< is.DetourLinkSize; n++)
				{
				is.DetourLinkArray[n] =  PhysicalNetwork.GetLinkNoByNodeIndex(g_NodeNametoIDMap[NodeList[n]], g_NodeNametoIDMap[NodeList[n+1]]);
				ASSERT(is.DetourLinkArray[n] >=0);

				}
				}

				if(type == 3)
				{

				is.ResponsePercentage  =  g_read_float(st);
				is.BestPathFlag  =  g_read_integer(st);
				is.StartTime = g_read_integer(st);
				is.EndTime = g_read_integer(st);
				}

				plink->InformationSignVector.push_back(is);
			}
		}

		fclose(st);
	}

	fopen_s(&st,"VOT.csv","r");
	if(st!=NULL)
	{
	cout << "Reading file VOT.csv..."<< endl;

		g_VOTStatVector[0].CumulativePercentage = 0;

		int PrevVOT = 0;

			while(!feof(st))
		{
			int VOT	= g_read_integer(st);
			if(VOT == -1)  // reach end of file
				break;

			// handle skipping vot ranges, use the cumulative value from the previous vot index
			for(int v = PrevVOT+1; v< VOT; v++)
			{
			g_VOTStatVector[v].CumulativePercentage = g_VOTStatVector[v-1].CumulativePercentage;
			}
			PrevVOT = VOT;

			float cumulative_percentage = g_read_float(st);
			g_VOTStatVector[VOT].CumulativePercentage = cumulative_percentage;
		}

			for(int v = PrevVOT+1; v< MAX_VOT_RANGE; v++)
			{
			g_VOTStatVector[v].CumulativePercentage = g_VOTStatVector[v-1].CumulativePercentage;
			}
		g_VOTStatVector[MAX_VOT_RANGE-1].CumulativePercentage = 1.0f;


		fclose(st);
	}else
	{
		int VOT;

		for(VOT = 1; VOT<=12; VOT++)
		{
			g_VOTStatVector[VOT].CumulativePercentage = 0.0f; 
		}

		for(VOT = 13; VOT<=100; VOT++)
		{
			g_VOTStatVector[VOT].CumulativePercentage = 1.0f; 
		}

	}





	fopen_s(&st,"toll.dat","r");
	if(st!=NULL)
	{
	cout << "Reading file toll.dat..."<< endl;

		for(int vt = 1; vt<MAX_VEHICLE_TYPE_SIZE; vt++)
		{
			g_VOT[vt]= g_read_float(st); 
		}

		float high_way_bias = g_read_float(st); 
		int NumberofTolls = g_read_integer(st);

		if(NumberofTolls >0)
		{
			g_LogFile << "# of tolls = " << NumberofTolls << endl;
		}


		for(i = 0; i < NumberofTolls; i++)
		{
			int usn  = g_read_integer(st);
			int dsn =  g_read_integer(st);
			int LinkID = PhysicalNetwork.GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

			DTALink* plink = g_LinkVector[LinkID];

			if(plink!=NULL)
			{
				Toll tc;  // toll collectio

				tc.StartTime = g_read_integer(st);
				tc.EndTime = g_read_integer(st);
				int link_based_flag = g_read_integer(st);

				for(int vt = 1; vt<MAX_VEHICLE_TYPE_SIZE; vt++)
				{
					tc.TollRate [vt]= g_read_float(st); 

					if(link_based_flag ==0) // distance based
					{
						tc.TollRate [vt] = tc.TollRate [vt]*plink->m_Length;
					}

					tc.TollRateInMin [vt]= tc.TollRate [vt]/g_VOT[vt]*60;  // VOT -> VOT in min
				}

			
				plink->TollVector.push_back(tc);
			}
		}

		std::set<DTALink*>::iterator iterLink;

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
					if(g_LinkVector[li]->TollVector .size() >0)
					{
					g_LinkVector[li]->m_TollSize = g_LinkVector[li]->TollVector .size();
					g_LinkVector[li]->pTollVector = new Toll[g_LinkVector[li]->m_TollSize ];

					for(int s = 0; s<g_LinkVector[li]->m_TollSize; s++)
					{
						g_LinkVector[li]->pTollVector[s] = g_LinkVector[li]->TollVector[s];
					}
					
					}

		}

		fclose(st);
	}

		if(g_ODEstimationFlag == 1)  //  OD estimation mode 1: read measurement data directly
		{
			g_ReadLinkMeasurementFile(&PhysicalNetwork);
					// second step: start reading historical demand
			g_ReadHistDemandFile();
		}

		if(g_ODEstimationFlag == 2) //  OD estimation mode 2: read simulated measurements
		{
		g_ReadObservedLinkMOEData(&PhysicalNetwork);
		g_ReadHistDemandFile();

		}

		ConnectivityChecking(&PhysicalNetwork);


	if(g_VehicleLoadingMode == 0)  // load from demand table
	{ 
		ReadDemandFile(&PhysicalNetwork);
		
	}else
	{  // load from vehicle file
		ReadDTALiteVehicleFile("Vehicle.csv",&PhysicalNetwork);

	}


	for(z = 1; z <=g_ODZoneSize; z++)
	{
		if(g_ZoneVector[z].m_Demand >= g_ZoneVector[z].m_Capacity )
			g_WarningFile << "Zone "<< z << " has demand " << g_ZoneVector[z].m_Demand  << " and capacity " << g_ZoneVector[z].m_Capacity<< endl;

	}

	cout << "Number of Zones = "<< g_ODZoneSize  << endl;
	cout << "Number of Nodes = "<< g_NodeVector.size() << endl;
	cout << "Number of Links = "<< g_LinkVector.size() << endl;
	cout << "Number of Vehicles = "<< g_VehicleVector.size() << endl;
	cout << "Running Time:" << g_GetAppRunningTime()  << endl;

	g_LogFile << "Number of Zones = "<< g_ODZoneSize  << endl;
	g_LogFile << "Number of Nodes = "<< g_NodeVector.size() << endl;
	g_LogFile << "Number of Links = "<< g_LinkVector.size() << endl;
	g_LogFile << "Number of Vehicles = "<< g_VehicleVector.size() << endl;

}



void CreateVehicles(int originput_zone, int destination_zone, float number_of_vehicles, int vehicle_type, float starting_time_in_min, float ending_time_in_min, long PathIndex)
{
	int number_of_vehicles_to_be_generated = g_GetRandomInteger_From_FloatingPointValue(number_of_vehicles);

	for(int i=0; i< number_of_vehicles_to_be_generated; i++)
	{
		DTA_vhc_simple vhc;
		vhc.m_OriginZoneID = originput_zone;

		g_ZoneVector[originput_zone].m_OriginVehicleSize ++;
		vhc.m_DestinationZoneID = destination_zone;

		vhc.m_PathIndex = PathIndex;

		float RandomRatio = 0;
		if(number_of_vehicles_to_be_generated <10) // small demand volume
		{
			RandomRatio= g_GetRandomRatioForVehicleGeneration(); // use random number twice to determine exact departure time of vehicles
		}
		else
		{
			RandomRatio = (i+1)*1.0f/number_of_vehicles_to_be_generated; // uniform distribution
		}

		vhc.m_DepartureTime = starting_time_in_min + RandomRatio*(ending_time_in_min-starting_time_in_min);

		float RandomPercentage= g_GetRandomRatio()*100.0f; 
		vhc.m_InformationClass = 1;
		if(RandomPercentage > g_UserClassPercentage[1] && RandomPercentage <= (g_UserClassPercentage[1] + g_UserClassPercentage[2]))
		{
			vhc.m_InformationClass = 2; // Pre-trip information
		}

		if(RandomPercentage > (g_UserClassPercentage[1] + g_UserClassPercentage[2]) && RandomPercentage <= (g_UserClassPercentage[1]+g_UserClassPercentage[2] + g_UserClassPercentage[3]))
		{
			vhc.m_InformationClass = 3; // Enroute information
		}


		vhc.m_VehicleType = vehicle_type;
		RandomPercentage= g_GetRandomRatio()*100.0f; 

		if(vehicle_type == 1) //base line demand
		{
		if(RandomPercentage > g_VehicleTypePercentage[1] && RandomPercentage <= (g_VehicleTypePercentage[1]+g_VehicleTypePercentage[2]))
		{
		vhc.m_VehicleType = 2;  //HOV
		}

		if(RandomPercentage > (g_VehicleTypePercentage[2]+g_VehicleTypePercentage[1]) && RandomPercentage <= (g_VehicleTypePercentage[1]+g_VehicleTypePercentage[2]+g_VehicleTypePercentage[3]))
		{
		vhc.m_VehicleType = 3;  // truck
		}

		}

		int DepartureTimeInterval = int(vhc.m_DepartureTime / g_DepartureTimetInterval);

		if(DepartureTimeInterval >= g_DepartureTimetIntervalSize)
			DepartureTimeInterval = g_DepartureTimetIntervalSize-1;


		vhc.m_PathIndex  = PathIndex;

		g_simple_vector_vehicles.push_back(vhc);

		if(g_DemandHorizonForLoadedVehicles < vhc.m_DepartureTime)
			g_DemandHorizonForLoadedVehicles = (int)(vhc.m_DepartureTime+1);

	}
}
void ReadDTALiteVehicleFile(char fname[_MAX_PATH], DTANetworkForSP* pPhysicalNetwork)
{
	int   PathNodeList[MAX_NODE_SIZE_IN_A_PATH];

	cout << "Reading vehicle file as simulation input... "  << endl;

	FILE* st = NULL;
	fopen_s(&st,fname,"r");
	if(st!=NULL)
	{
		while(!feof(st))
		{
			int Iteration     = g_read_integer(st);
			if(Iteration == -1)  // reach end of file
				break;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_VehicleID = g_read_integer(st);
			pVehicle->m_OriginZoneID = g_read_integer(st);
			pVehicle->m_DestinationZoneID = g_read_integer(st);
			pVehicle->m_DepartureTime = g_read_float(st);
			pVehicle->m_TimeToRetrieveInfo = (int)(pVehicle->m_DepartureTime*10);

			float m_ArrivalTime = g_read_float(st); // to be determined by simulation
			pVehicle->m_ArrivalTime  = 0;
			int CompleteFlag = g_read_integer(st);
			pVehicle->m_bComplete = false;
			pVehicle->m_bLoaded  = false;
			float TripTime  = g_read_float(st);  // to be determined by simulation
			pVehicle->m_VehicleType = g_read_integer(st);
			pVehicle->m_Occupancy = g_read_integer(st);
			pVehicle->m_InformationClass = g_read_integer(st);
			float VOT = g_read_float(st);
			float MinCost = g_read_float(st);
			pVehicle->m_Distance = g_read_float(st);

			pVehicle->m_NodeSize = g_read_integer(st);

			int NodeSize = pVehicle->m_NodeSize;

			if(pVehicle->m_NodeSize > 0)
			{
				pVehicle->m_aryVN = new SVehicleLink[NodeSize];
				if(pVehicle->m_aryVN==NULL)
				{

					cout << "Insufficient memory for allocating vehicle arrays!";
					getchar();
					exit(0);
				}

				pVehicle->m_NodeNumberSum =0;
				pVehicle->m_Distance =0;

				int i;
				for(i = 0; i< NodeSize; i++)
				{
					PathNodeList[i] = g_read_integer(st);
					float timestamp = g_read_float(st);
					pVehicle->m_NodeNumberSum +=PathNodeList[i];
				}

				// find out link id
				for(i = 0; i< NodeSize-1; i++)
				{
					pVehicle->m_aryVN[i].LinkID = pPhysicalNetwork->GetLinkNoByNodeIndex(g_NodeNametoIDMap[PathNodeList[i]],g_NodeNametoIDMap[PathNodeList[i+1]]);
					pVehicle->m_Distance+= g_LinkVector[pVehicle->m_aryVN [i].LinkID] ->m_Length ;

				}



			}

			g_VehicleVector.push_back(pVehicle);
			g_VehicleMap[pVehicle->m_VehicleID]  = pVehicle;

			if(g_DemandHorizonForLoadedVehicles < pVehicle->m_DepartureTime)
				g_DemandHorizonForLoadedVehicles = (int)(pVehicle->m_DepartureTime+1);

		}
		fclose(st);	
	}else
	{
		cout << "Error: File vehicle.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}
}

void ReadDemandFile(DTANetworkForSP* pPhysicalNetwork)
{
	FILE* st = NULL;
	bool bFileReady = false;
	int i,t,z;

	fopen_s(&st,"input_demand.csv","r");
	if(st!=NULL)
	{
		bFileReady = true;
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
			number_of_vehicles*= g_DemandGlobalMultiplier;


			vehicle_type =  g_read_integer(st);
			starting_time_in_min = g_read_float(st);
			ending_time_in_min = g_read_float(st);
			// we generate vehicles here for each OD data line

//			TRACE("o:%d d: %d, %f,%d,%f,%f\n", originput_zone,destination_zone,number_of_vehicles,vehicle_type,starting_time_in_min,ending_time_in_min);

			g_ZoneVector[originput_zone].m_Demand += number_of_vehicles;

			float Interval= float(ending_time_in_min - starting_time_in_min);

			float LoadingRatio = 1;
			if(starting_time_in_min < g_DemandLoadingHorizon && g_DemandLoadingHorizon < ending_time_in_min)  // simulation shorter than demand ending time.xed
			{
				ending_time_in_min = (float)(g_DemandLoadingHorizon);

				LoadingRatio = (g_DemandLoadingHorizon-starting_time_in_min)*1.0f/Interval;

			}

			if(starting_time_in_min < g_DemandLoadingHorizon && originput_zone!= destination_zone)
			{
				CreateVehicles(originput_zone,destination_zone,number_of_vehicles*LoadingRatio,vehicle_type,starting_time_in_min,ending_time_in_min);
			}
			else
			{
				//				cout << "Warning: ending_time_in_min from demand file" << ending_time_in_min << " < simulation horizon" << g_DemandLoadingHorizon << endl;
				//				g_WarningFile << "Warning: ending_time_in_min from demand file" << ending_time_in_min << " < simulation horizon" << g_DemandLoadingHorizon << endl;
			}

			if(line_no %100000 ==0)
			{
				cout << g_GetAppRunningTime() << "Reading " << line_no/1000 << "K lines..."<< endl;
			}

			line_no++;
		}

		fclose(st);
	}

	// 	alternatively, read demand.txt
	fopen_s(&st,"input_demand.txt","r");
	if(st!=NULL)
	{
		cout << "Reading file demand.txt..."<< endl;
		bFileReady = true;

		int line_no = 1;

		while(!feof(st))
		{
			int originput_zone	       = g_read_integer(st);
			if(originput_zone == -1)  // reach end of file
				break;

			int destination_zone	   = g_read_integer(st);

			float number_of_vehicles = g_read_float(st);
			number_of_vehicles*= g_DemandGlobalMultiplier;
			int vehicle_type		   = 1;
			float starting_time_in_min     = 0;
			float ending_time_in_min     = float(g_DemandLoadingHorizon);

			// we generate vehicles here for each OD data line

			CreateVehicles(originput_zone,destination_zone,number_of_vehicles,vehicle_type,starting_time_in_min,ending_time_in_min);
			if(line_no %100000 ==0)
			{
				cout << g_GetAppRunningTime() <<  "Reading " << line_no/1000 << "K lines..."<< endl;
			}

			line_no++;
		}

		fclose(st);
	}

// 	alternatively, read demand.txt
	fopen_s(&st,"input_trip.csv","r");
	if(st!=NULL)
	{
		cout << "Reading file input_trip.csv..."<< endl;
		bFileReady = true;

		int line_no = 1;

		while(!feof(st))
		{
			int originput_zone	       = g_read_integer(st);
			if(originput_zone == -1)  // reach end of file
				break;

			int destination_zone	   = g_read_integer(st);

			float number_of_vehicles = g_read_float(st);  // SOV
			 g_read_float(st); //HOV2
			 g_read_float(st); //HOV3
			 g_read_float(st); //Light Truck
			 g_read_float(st); //Medium Truck
			 g_read_float(st); //Heavy Truck
			 
			number_of_vehicles*= g_DemandGlobalMultiplier;
			int vehicle_type		   = 1;
			float starting_time_in_min     = 0;
			float ending_time_in_min     = float(g_DemandLoadingHorizon);

			// we generate vehicles here for each OD data line

			CreateVehicles(originput_zone,destination_zone,number_of_vehicles,vehicle_type,starting_time_in_min,ending_time_in_min);
			if(line_no %100000 ==0)
			{
				cout << g_GetAppRunningTime() <<  "Reading " << line_no/1000 << "K lines..."<< endl;
			}

			line_no++;
		}

		fclose(st);
	}
/*
	// 	alternatively, read demand.txt
	fopen_s(&st,"input_trips.txt","r");
	if(st!=NULL)
	{
		cout << "Reading file trips.txt..."<< endl;
		bFileReady = true;
		int line_no = 1;

		for(int i = 1; i <=g_ODZoneSize; i++)
		{
			int originput_zone	       = g_read_integer(st);
			if(originput_zone == -1)  // reach end of file
				break;

			for(int j = 1; j <=g_ODZoneSize; j++)
			{
				int destination_zone	   = g_read_integer_with_char_O(st);  // we might read "Origin" here, so if we find "O", we return 0 as a key to jump to the next origin.

				if(destination_zone == 0) // we reach "O" of string Origin 
				{
					break;
				}

				destination_zone = j;  // use destination that we read to override j

				float number_of_vehicles = g_read_float(st);

				number_of_vehicles*= g_DemandGlobalMultiplier;
				int vehicle_type		   = 1;
				float starting_time_in_min     = 0;
				float ending_time_in_min     = 60;

				// we generate vehicles here for each OD data line

				CreateVehicles(originput_zone,destination_zone,number_of_vehicles,vehicle_type,starting_time_in_min,ending_time_in_min);
				if(line_no %100000 ==0)
				{
					cout << g_GetAppRunningTime() <<  "Reading " << line_no/1000 << "K lines..."<< endl;
				}

				line_no++;
			}
		}

		fclose(st);
	}
*/

	if(bFileReady == false)
	{
		cout << "Error: File demand.csv or demand.txt or trips.txt cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();

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


			pVehicle->m_NodeSize = 0;  // initialize NodeSize as o
			g_VehicleVector.push_back(pVehicle);
			
			g_VehicleMap[i]  = pVehicle;


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
void FreeMemory()
{
	std::vector<DTAVehicle*>::iterator iterVehicle;

	// Free pointers

	cout << "Free node set... " << endl;

	if(g_HistODDemand !=NULL)
	Deallocate3DDynamicArray<float>(g_HistODDemand,g_ODZoneSize+1,g_ODZoneSize+1);

	g_NodeVector.clear();

	cout << "Free link set... " << endl;
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		DTALink* pLink = g_LinkVector[li];
		delete pLink;
	}

	g_FreeODTKPathVector();
	g_LinkVector.clear();
	g_LinkVector.clear();

	g_ZoneVector.clear ();
	g_FreeVehicleVector();

	DeallocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_TDOVehicleArray,g_ODZoneSize+1, g_DepartureTimetIntervalSize);  // +1 is because the zone numbers start from 1 not from 0

}




void OutputLinkMOEData(char fname[_MAX_PATH], int Iteration, bool bStartWithEmpty)
{
	FILE* st = NULL;

	if(bStartWithEmpty)
		fopen_s(&st,fname,"w");
	else
		fopen_s(&st,fname,"a");

	std::set<DTALink*>::iterator iterLink;

	if(st!=NULL)
	{
		if(bStartWithEmpty)
		{
			fprintf(st, "from_node_id, to_node_id, timestamp_in_min, travel_time_in_min, delay_in_min, link_volume_in_veh_per_hour_per_lane, link_volume_in_veh_per_hour_for_all_lanes,density_in_veh_per_mile_per_lane, speed_in_mph, exit_queue_length, cumulative_arrival_count, cumulative_departure_count\n");
		}

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{		for(int time = 0; time< g_SimulationHorizon;time++)
		{
			if((g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount-g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount) > 0) // there are vehicles on the link
			{
				float LinkOutFlow = float(g_LinkVector[li]->GetDepartureFlow(time));
				float travel_time = g_LinkVector[li]->GetTravelTime(time,1);

				fprintf(st, "%d,%d,%d,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f, %d, %d, %d\n",
					g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName, g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName,time,
					travel_time, travel_time - g_LinkVector[li]->m_FreeFlowTravelTime ,
					LinkOutFlow*60.0/g_LinkVector[li]->m_NumLanes ,LinkOutFlow*60.0,
					(g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount-g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount)/g_LinkVector[li]->m_Length /g_LinkVector[li]->m_NumLanes,
					g_LinkVector[li]->GetSpeed(time), g_LinkVector[li]->m_LinkMOEAry[time].ExitQueueLength, 
					g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount ,g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount);
			}

		}
		}
		fclose(st);
	}else
	{
		fprintf(g_ErrorFile, "File LinkMOE.csv cannot be opened. It might be currently used and locked by EXCEL.");
		cout << "Error: File LinkMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		cin.get();  // pause
	}


}

void OutputNetworkMOEData(char fname[_MAX_PATH], int Iteration, bool bStartWithEmpty)
{
	FILE* st = NULL;

	if(bStartWithEmpty)
		fopen_s(&st,fname,"w");
	else
		fopen_s(&st,fname,"a");

	if(st!=NULL)
	{
		if(bStartWithEmpty)
		{
			fprintf(st, "iteration, time_stamp_in_min, cumulative_in_flow_count, cumulative_out_flow_count, number_vehicles_in_network, flow_in_a_min, avg_trip_time_in_min\n");
		}

		for(int time = 0; time< g_SimulationHorizon;time++)
		{
			if(g_NetworkMOEAry[time].Flow_in_a_min > 0)
				g_NetworkMOEAry[time].AvgTripTime = g_NetworkMOEAry[time].AbsArrivalTimeOnDSN_in_a_min / g_NetworkMOEAry[time].Flow_in_a_min;

			if(time >=1)
			{
				if(g_NetworkMOEAry[time].CumulativeInFlow < g_NetworkMOEAry[time-1].CumulativeInFlow)
					g_NetworkMOEAry[time].CumulativeInFlow = g_NetworkMOEAry[time-1].CumulativeInFlow;

				if(g_NetworkMOEAry[time].CumulativeOutFlow < g_NetworkMOEAry[time-1].CumulativeOutFlow)
					g_NetworkMOEAry[time].CumulativeOutFlow = g_NetworkMOEAry[time-1].CumulativeOutFlow;
			}

			fprintf(st, "%d,%d, %d, %d,%d, %d, %6.2f\n", Iteration, time,
				g_NetworkMOEAry[time].CumulativeInFlow ,g_NetworkMOEAry[time].CumulativeOutFlow,
				g_NetworkMOEAry[time].CumulativeInFlow - g_NetworkMOEAry[time].CumulativeOutFlow, g_NetworkMOEAry[time].Flow_in_a_min ,g_NetworkMOEAry[time].AvgTripTime );
		}
		fclose(st);
	}else
	{
		fprintf(g_ErrorFile, "File NetworkMOE.csv cannot be opened. It might be currently used and locked by EXCEL.");
		cout << "Error: File NetworkMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		getchar();
		exit(0);

	}


}

void g_OutputVOCMOEData(char fname[_MAX_PATH])
{

	FILE* st = NULL;
		fopen_s(&st,fname,"w");

	if(st!=NULL)
	{
		fprintf(st, " ");  //write empty line for non-BRP loading

		if(g_TrafficFlowModelFlag == 0)
		{// BRP loading only

			fprintf(st, "FromNodeID, ToNodeID, Link Capacity per hour, Link Flow per hour, VOC Ratio, Free-Flow Travel Time (min), Travel Time (min), Speed (mph)\n");

		std::set<DTALink*>::iterator iterLink;

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			float Capacity = g_LinkVector[li]->m_MaximumServiceFlowRatePHPL * g_LinkVector[li]->m_NumLanes;
			fprintf(st, "%d,%d,%10.3f, %10.3f,%10.3f,%10.3f,%10.3f,%10.3f\n", g_NodeVector[g_LinkVector[li]->m_FromNodeID],
				g_NodeVector[g_LinkVector[li]->m_ToNodeID],
				g_LinkVector[li]->m_BPRLaneCapacity,
				g_LinkVector[li]->m_BPRLinkVolume,
				g_LinkVector[li]->m_BPRLinkVolume /g_LinkVector[li]->m_BPRLaneCapacity ,
				g_LinkVector[li]->m_FreeFlowTravelTime,
				g_LinkVector[li]->m_BPRLinkTravelTime, 
				g_LinkVector[li]->m_Length /(g_LinkVector[li]->m_BPRLinkTravelTime/60.0f));
		}

		}
		fclose(st);
	}else
	{
		fprintf(g_ErrorFile, "File %s cannot be opened. It might be currently used and locked by EXCEL.", fname);
		cout << "Error: File " << fname << " cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		getchar();
		exit(0);


	}
}

void OutputVehicleTrajectoryData(char fname[_MAX_PATH],int Iteration, bool bStartWithEmpty)
{

	FILE* st = NULL;

	if(bStartWithEmpty)
		fopen_s(&st,fname,"w");
	else
		fopen_s(&st,fname,"a");

	if(st!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;
		int VehicleCount_withPhysicalPath = 0;

		if(bStartWithEmpty)
		{
			fprintf(st, "vehicle_id,  originput_zone_id, destination_zone_id, departure_time, arrival_time, complete_flag, trip_time, vehicle_type, occupancy, information_type, value_of_time, path_min_cost,distance_in_mile, number_of_nodes,  node id, node arrival time\n");
		}

		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			if(pVehicle->m_NodeSize >= 2)  // with physical path in the network
			{

				int UpstreamNodeID = 0;
				int DownstreamNodeID = 0;

				int LinkID_0 = pVehicle->m_aryVN [0].LinkID;
				UpstreamNodeID= g_LinkVector[LinkID_0]->m_FromNodeID;
				DownstreamNodeID = g_LinkVector[LinkID_0]->m_ToNodeID;

				float TripTime = 0;

				if(pVehicle->m_bComplete)
					TripTime = pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime;

				float m_gap = 0;
				fprintf(st,"%d,o%d,d%d,%4.2f,%4.2f,c%d,%4.2f,%d,%d,i%d,%4.2f,%4.2f,%4.2f,%d\n",
					pVehicle->m_VehicleID , pVehicle->m_OriginZoneID , pVehicle->m_DestinationZoneID,
					pVehicle->m_DepartureTime, pVehicle->m_ArrivalTime , pVehicle->m_bComplete, TripTime,			
					pVehicle->m_VehicleType ,pVehicle->m_Occupancy,pVehicle->m_InformationClass, pVehicle->GetVOT() , pVehicle->GetMinCost(),pVehicle->m_Distance, pVehicle->m_NodeSize);

				

				int j = 0;
				if(g_LinkVector[pVehicle->m_aryVN [0].LinkID]==NULL)
				{

					cout << "Error: vehicle" << pVehicle->m_VehicleID << "at LinkID"<< pVehicle->m_aryVN [0].LinkID << endl;
					cin.get();  // pause

				}

				int NodeID = g_LinkVector[pVehicle->m_aryVN [0].LinkID]->m_FromNodeID;  // first node
				int NodeName = g_NodeVector[NodeID].m_NodeName ;
				int link_entering_time = int(pVehicle->m_DepartureTime);
				fprintf(st, ",%d,%4.2f\n",
					NodeName,pVehicle->m_DepartureTime) ;

				float LinkWaitingTime = 0;
				for(j = 0; j< pVehicle->m_NodeSize-1; j++)
				{
					int LinkID = pVehicle->m_aryVN [j].LinkID;
					int NodeID = g_LinkVector[LinkID]->m_ToNodeID;
					int NodeName = g_NodeVector[NodeID].m_NodeName ;

					if(j>0)
					{
						link_entering_time = int(pVehicle->m_aryVN [j-1].AbsArrivalTimeOnDSN);
						//						LinkWaitingTime = pVehicle->m_aryVN [j].AbsArrivalTimeOnDSN - pVehicle->m_aryVN [j-1].AbsArrivalTimeOnDSN - g_LinkVector[LinkID]->m_FreeFlowTravelTime ;
						//						if(LinkWaitingTime <0)
						//							LinkWaitingTime = 0;

					}

					//						fprintf(st, ",,,,,,,,,,,,,,%d,%d%,%6.2f,%6.2f,%6.2f\n", j+2,NodeName,pVehicle->m_aryVN [j].AbsArrivalTimeOnDSN,LinkWaitingTime, g_LinkVector[LinkID]->m_LinkMOEAry [link_entering_time].TravelTime ) ;
					fprintf(st, ",%d,%4.2f\n",NodeName,pVehicle->m_aryVN [j].AbsArrivalTimeOnDSN) ;
				}

			}else
			{
				g_WarningFile << "Warning: Vehicle " <<  pVehicle->m_VehicleID << " from " << pVehicle ->m_OriginZoneID << " to "  << pVehicle ->m_DestinationZoneID << " does not have a physical path. Count:" << VehicleCount_withPhysicalPath  << endl;		
				VehicleCount_withPhysicalPath++;
			}
		}
		fclose(st);
	}else
	{
		fprintf(g_ErrorFile, "File vehicle.csv cannot be opened. It might be currently used and locked by EXCEL.");
		cout << "Error: File vehicle.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		getchar();
		exit(0);


	}
}
void 	RT_ShortestPath_Thread(int id, int nthreads, int node_size, int link_size, int departure_time)
{

}

void g_RealTimeShortestPathCalculation(int Time)
{
	int nthreads = omp_get_max_threads ( );
	int NodeSize = g_NodeVector.size();
#pragma omp parallel for
	for(int CurNodeID=0; CurNodeID < NodeSize; CurNodeID++)
	{
		DTANetworkForSP network_RT_MP(NodeSize, g_LinkVector.size(), g_SimulationHorizon,g_AdjLinkSize);  //  network instance for single processor in multi-thread environment
		network_RT_MP.BuildPhysicalNetwork();

		int	id = omp_get_thread_num ( );  // starting from 0

		if(CurNodeID%100 == 0)
		{
			cout <<g_GetAppRunningTime() << "processor " << id << " is working on assignment at node  "<<  CurNodeID << endl;
		}
		network_RT_MP.TDLabelCorrecting_DoubleQueue(CurNodeID,Time,1);  // CurNodeID is the node ID
		// assign shortest path calculation results to label array



	}

}
int g_InitializeLogFiles()
{
	g_AppStartTime = CTime::GetCurrentTime();

	g_LogFile.open ("summary.log", ios::out);
	if (g_LogFile.is_open())
	{
		g_LogFile.width(12);
		g_LogFile.precision(3) ;
		g_LogFile.setf(ios::fixed);
	}else
	{
		cout << "File summary.log cannot be opened, and it might be locked by another program or the target data folder is read-only." << endl;
		cin.get();  // pause
		return 0;
	}

	g_AssignmentLogFile.open ("assignment.log", ios::out);
	if (g_AssignmentLogFile.is_open())
	{
		g_AssignmentLogFile.width(12);
		g_AssignmentLogFile.precision(3) ;
		g_AssignmentLogFile.setf(ios::fixed);
	}else
	{
		cout << "File assignment.log cannot be opened, and it might be locked by another program!" << endl;
		cin.get();  // pause
		return 0;
	}

		g_EstimationLogFile.open ("estimation.log", ios::out);
	if (g_EstimationLogFile.is_open())
	{
		g_EstimationLogFile.width(12);
		g_EstimationLogFile.precision(3) ;
		g_EstimationLogFile.setf(ios::fixed);
	}else
	{
		cout << "File estimation.log cannot be opened, and it might be locked by another program!" << endl;
		cin.get();  // pause
		return 0;
	}



	g_WarningFile.open ("warning.log", ios::out);
	if (g_WarningFile.is_open())
	{
		g_WarningFile.width(12);
		g_WarningFile.precision(3) ;
		g_WarningFile.setf(ios::fixed);
	}else
	{
		cout << "File warning.log cannot be opened, and it might be locked by another program!" << endl;
		cin.get();  // pause
		return 0;
	}

	cout << "DTALite: A Fast Open-Source DTA Simulation Engine"<< endl;
		cout << "sourceforge.net/projects/dtalite/"<< endl;
		cout << "Version 0.95, Release Date 01/15/2011."<< endl;

		g_LogFile << "---DTALite: A Fast Open-Source DTA Simulation Engine---"<< endl;
		g_LogFile << "sourceforge.net/projects/dtalite/"<< endl;
		g_LogFile << "Version 0.95, Release Date 01/15/2011."<< endl;

		fopen_s(&g_ErrorFile,"error.log","w");
		if(g_ErrorFile==NULL)
		{
			cout << "CANNOT OPEN file error.log!" << endl;
			cin.get();  // pause
			return 0;
		}

		return 1;
}

void g_ReadDTALiteSettings()
{
		TCHAR IniFilePath_DTA[_MAX_PATH] = _T("./DTASettings.ini");

		// if  ./DTASettings.ini does not exit, then we should print out all the default settings for user to change
		//

		g_TrafficFlowModelFlag = g_GetPrivateProfileInt("simulation", "traffic_flow_model", 0, IniFilePath_DTA);	
		g_TollingMethodFlag = g_GetPrivateProfileInt("tolling", "method_flag", 0, IniFilePath_DTA);	
		g_VMTTollingRate = g_GetPrivateProfileFloat("tolling", "VMTRate", 0, IniFilePath_DTA);

		g_ODEstimationFlag = g_GetPrivateProfileInt("estimation", "od_demand_estimation", 0, IniFilePath_DTA);	
		g_ODEstimationMeasurementType = g_GetPrivateProfileInt("estimation", "measurement_type", 1, IniFilePath_DTA);	

		g_ODEstimation_StartingIteration = g_GetPrivateProfileInt("estimation", "starting_iteration", 2, IniFilePath_DTA);
		g_ObservationTimeInterval = g_GetPrivateProfileInt("estimation", "observation_time_interval", 5, IniFilePath_DTA);
		g_ObservationStartTime = g_GetPrivateProfileInt("estimation", "observation_start_time_in_min", 390, IniFilePath_DTA);
		g_ObservationEndTime = g_GetPrivateProfileInt("estimation", "observation_end_time_in_min", 570, IniFilePath_DTA);

		if(g_ObservationEndTime >= g_ObservationStartTime + g_SimulationHorizon)  // no later than the simulation end time
			g_ObservationEndTime = g_ObservationStartTime + g_SimulationHorizon;


		if(g_TrafficFlowModelFlag ==0)  //BRP  // static assignment parameters
		{
			g_DemandLoadingHorizon = 60;
			g_DepartureTimetInterval = 60;
			g_SimulationHorizon = 600;
			g_NumberOfInnerIterations = 0;
			g_NumberOfIterations = g_GetPrivateProfileInt("assignment", "number_of_iterations", 10, IniFilePath_DTA);	
			g_AgentBasedAssignmentFlag = g_GetPrivateProfileInt("assignment", "agent_based_assignment", 0, IniFilePath_DTA);
			g_DemandGlobalMultiplier = g_GetPrivateProfileFloat("demand", "global_multiplier",1.0,IniFilePath_DTA);	
		}
		else  //DTA parameters
		{
		g_DemandLoadingHorizon = g_GetPrivateProfileInt("demand", "demand_loading_horizon_in_min", 60, IniFilePath_DTA);	
		g_VehicleTypePercentage[2] = g_GetPrivateProfileInt("demand", "percentage_HOV_from_baseline_demand", 0, IniFilePath_DTA);	
		g_VehicleTypePercentage[3] = g_GetPrivateProfileInt("demand", "percentage_Truck_from_baseline_demand", 0, IniFilePath_DTA);	
		g_VehicleTypePercentage[1] = 100.0- g_VehicleTypePercentage[2] - g_VehicleTypePercentage[3];

		g_VehicleLoadingMode = g_GetPrivateProfileInt("demand", "load_vehicle_file_mode", 0, IniFilePath_DTA);	

		g_SimulationHorizon = g_GetPrivateProfileInt("simulation", "simulation_horizon_in_min", 120, IniFilePath_DTA);	
		g_StochasticCapacityMode = g_GetPrivateProfileInt("simulation", "stochatic_capacity_mode", 0, IniFilePath_DTA);


		g_MergeNodeModelFlag = g_GetPrivateProfileInt("simulation", "merge_node_model", 1, IniFilePath_DTA);	
		g_FreewayJamDensity_in_vehpmpl = g_GetPrivateProfileFloat("simulation", "freeway_jam_density_in_vehphpl", 220, IniFilePath_DTA);
		g_NonFreewayJamDensity_in_vehpmpl = g_GetPrivateProfileFloat("simulation", "nonfreeway_jam_density_in_vehphpl", 120, IniFilePath_DTA);
		g_BackwardWaveSpeed_in_mph = g_GetPrivateProfileFloat("simulation", "backward_wave_speed_in_vehphpl", 12, IniFilePath_DTA);
		g_MinimumInFlowRatio = g_GetPrivateProfileFloat("simulation", "minput_link_in_flow_ratio", 0.02f, IniFilePath_DTA);
		g_MaxDensityRatioForVehicleLoading  = g_GetPrivateProfileFloat("simulation", "max_density_ratio_for_loading_vehicles", 0.8f, IniFilePath_DTA);
		g_CycleLength_in_seconds = g_GetPrivateProfileFloat("simulation", "cycle_length_in_seconds", 120, IniFilePath_DTA);
		g_DefaultSaturationFlowRate_in_vehphpl = g_GetPrivateProfileFloat("simulation", "default_saturation_flow_rate_in_vehphpl", 1800, IniFilePath_DTA);


		g_NumberOfIterations = g_GetPrivateProfileInt("assignment", "number_of_iterations", 10, IniFilePath_DTA);	
		g_AgentBasedAssignmentFlag = g_GetPrivateProfileInt("assignment", "agent_based_assignment", 0, IniFilePath_DTA);
		g_DepartureTimetInterval = g_GetPrivateProfileInt("assignment", "departure_time_interval_in_min", 60, IniFilePath_DTA);	
		g_NumberOfInnerIterations = g_GetPrivateProfileInt("assignment", "number_of_inner_iterations", 1, IniFilePath_DTA);	
		g_ConvergencyRelativeGapThreshold_in_perc = g_GetPrivateProfileFloat("assignment", "convergency_relative_gap_threshold_percentage", 5, IniFilePath_DTA);	

		g_UEAssignmentMethod = g_GetPrivateProfileInt("assignment", "UE_assignment_method", 0, IniFilePath_DTA); // default is MSA

		// parameters for day-to-day learning mode
		g_LearningPercentage = g_GetPrivateProfileInt("assignment", "learning_percentage", 15, IniFilePath_DTA);
		g_TravelTimeDifferenceForSwitching = g_GetPrivateProfileInt("assignment", "travel_time_difference_for_switching_in_min", 5, IniFilePath_DTA);

		g_DepartureTimetIntervalSize = max(1,g_DemandLoadingHorizon/g_DepartureTimetInterval);

		g_DemandGlobalMultiplier = g_GetPrivateProfileFloat("demand", "global_multiplier",1.0,IniFilePath_DTA);	
		g_start_iteration_for_MOEoutput = g_GetPrivateProfileInt("output", "start_iteration_for_MOE", -1, IniFilePath_DTA);	

		g_RandomSeed = g_GetPrivateProfileInt("simulation", "random_number_seed", 100, IniFilePath_DTA);

		g_UserClassPercentage[2] = g_GetPrivateProfileFloat("traveler_information", "percentage_of_pretrip_info_travelers",0.0,IniFilePath_DTA);	

		g_UserClassPercentage[3] = g_GetPrivateProfileFloat("traveler_information", "percentage_of_en_route_info_travelers",0.0,IniFilePath_DTA);	

		g_UserClassPerceptionErrorRatio[1] = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_historical_info_travelers_perception_error",0.3f,IniFilePath_DTA);	
		g_UserClassPerceptionErrorRatio[2] = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_pretrip_info_travelers_perception_error",0.05f,IniFilePath_DTA);	
		g_UserClassPerceptionErrorRatio[3] = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_en-route_info_travelers_perception_error",0.05f,IniFilePath_DTA);	
		g_VMSPerceptionErrorRatio          = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_VMS_perception_error",0.05f,IniFilePath_DTA);	
		g_information_updating_interval_of_en_route_info_travelers_in_min= g_GetPrivateProfileInt("traveler_information", "information_updating_interval_of_en_route_info_travelers_in_min",5,IniFilePath_DTA);	

		g_UserClassPercentage[1] = 100.0f - (g_UserClassPercentage[2] + g_UserClassPercentage[3]);
		}

		srand(g_RandomSeed);
		g_LogFile << "Demand Loading Horizon (min) = " << g_DemandLoadingHorizon << endl;
		g_LogFile << "Simulation Horizon (min) = " << g_SimulationHorizon << endl;
		g_LogFile << "Departure Time Interval (min) = " << g_DepartureTimetInterval << endl;
		g_LogFile << "Number of Iterations = "<< g_NumberOfIterations << endl;

		if( g_NumberOfIterations == 1 && (g_UserClassPercentage[2] + g_UserClassPercentage[3])>100)
		{
			cout << "Error: UserClassPercentage[2] + g_UserClassPercentage[3]) > 100. " << endl;
			getchar();
			exit(0);
		}


		g_LogFile << "Percentage of Pretrip Information Users = "<< g_UserClassPercentage[1] << "%"  << endl;

		if(g_VehicleLoadingMode == 1)
		{
			g_LogFile << "Load vehicle trajectory file = " << endl;
		}

		g_LogFile << "Traffic Flow Model =  ";
		switch( g_TrafficFlowModelFlag)
		{
		case 0: 		g_LogFile << "BPR Function" << endl;
			break;

		case 1: 		g_LogFile << "Point Queue Model" << endl;
			break;

		case 2: 		g_LogFile << "Spatial Queue Model" << endl;
			break;

		case 3: 		g_LogFile << "Newell's Cummulative Flow Count Model" << endl;
			break;

		default: 		g_LogFile << "No Valid Model is Selected" << endl;
			break; 
		}

				if(g_start_iteration_for_MOEoutput == -1)  // no value specified
		{

			g_start_iteration_for_MOEoutput = g_NumberOfIterations;
		}


}

void g_OutputSimulationStatistics()
{


		int Count=0; 
		float AvgTravelTime, AvgDistance, AvgSpeed;
		g_LogFile << "--- MOE for vehicles completing trips ---" << endl;

		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, -1, -1);
		g_LogFile << " # of Vehicles = " << Count << " AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	if(g_TrafficFlowModelFlag !=0)  //DTA
	{

		g_LogFile << "--- MOE for Each Information Class ---" << endl;

		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, 1, -1, -1);
		if(Count>0)
		g_LogFile << "Hist Knowledge (VMS non-responsive): # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" << endl;

		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, 2, -1, -1);
		if(Count>0)
		g_LogFile << "Pre-trip Info                      : # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, 3, -1, -1);
		if(Count>0)
		g_LogFile << "En-route Info                      : # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" << endl;

		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, 4, -1, -1);
		if(Count>0)
		g_LogFile << "Hist Knowledge (VMS responsive)    : # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	}
		g_LogFile << endl  << "--- MOE for Each Vehicle Type ---" << endl;

		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, 1, -1);
		if(Count>0)
		g_LogFile << "LOV Passenger Car: # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, 2, -1);
		if(Count>0)
		g_LogFile << "HOV Passenger Car: # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, 3, -1);
		if(Count>0)
		g_LogFile << "Truck            : # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

		g_LogFile << endl << "--- MOE for Each Departure Time Interval ---" << endl;

		for(int departure_time = 0; departure_time < g_DemandLoadingHorizon; departure_time += g_DepartureTimetInterval)
		{
		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, -1, departure_time);
		if(Count>0)
		g_LogFile << "Time:" << departure_time << " min, # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;
	
		}



		g_LogFile << endl;

		// output general link statistics
		std::set<DTALink*>::iterator iterLink;
		g_LogFile << "--- Link MOE ---" << endl;

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			float Capacity = g_LinkVector[li]->m_MaximumServiceFlowRatePHPL * g_LinkVector[li]->m_NumLanes;
			g_LogFile << "Link: " <<  g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName  << " -> " << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName << 
				", Link Capacity: " << Capacity <<
				", Inflow: " << g_LinkVector[li]->CFlowArrivalCount <<
				//				" Outflow: " << g_LinkVector[li]->CFlowDepartureCount <<
				", VOC Ratio: " << g_LinkVector[li]->CFlowDepartureCount /Capacity  << endl;
		}

		g_OutputVOCMOEData("LinkStaticMOE.csv");  // output assignment results anyway
}

void g_FreeMemory()
{
		cout << "Free memory... " << endl;
		FreeMemory();

		fclose(g_ErrorFile);
		g_LogFile.close();
		g_AssignmentLogFile.close();
		g_WarningFile.close();

		g_LogFile << "Assignment-Simulation Completed. " << g_GetAppRunningTime() << endl;
}

void g_TrafficAssignmentSimulation()
{
		g_NetworkMOEAry.resize (g_SimulationHorizon+1);  // "+1" as simulation time starts as 0
		g_AssignmentMOEAry.resize ( g_SimulationHorizon+1);  // "+1" as assignment iteration starts as 0

		ReadInputFiles();

			if(g_TrafficFlowModelFlag ==0)
			{
				if(g_AgentBasedAssignmentFlag==0)
					g_StaticTrafficAssisnment(); // multi-iteration static traffic assignment
				else
					g_AgentBasedAssisnment();  // agent-based assignment
			
			}
				
			else 
			{ // dynamic traffic assignment
				if(g_NumberOfIterations == 1)
				g_OneShotNetworkLoading();  //dynamic network loading only
			else
				if(g_AgentBasedAssignmentFlag==0)
					g_DynamicTrafficAssisnment(); // multi-iteration dynamic traffic assignment
				else
					g_AgentBasedAssisnment();  // agent-based assignment
			}
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

		// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;

		return 0;
	}


	if(g_InitializeLogFiles()==0) 
		return 0;


	g_ReadDTALiteSettings();


	/**********************************************/
	//below is the main traffic assignment-simulation code

		g_TrafficAssignmentSimulation();

		g_OutputSimulationStatistics();

		g_ExportLinkMOEToGroundTruthSensorData_ODEstimation();

		g_FreeMemory();
	return nRetCode;
}
/************************
//below is research code for multi-day traffic assignment, should not be included in the official release
		if(MAX_DAY_SIZE >=1)
		{
			g_MultiDayTrafficAssisnment();
		}

*************************/


void DTANetworkForSP::IdentifyBottlenecks(int StochasticCapacityFlag)
{

	ofstream BottleneckFile;
	BottleneckFile.open ("bottleneck.log", ios::out);
	if (BottleneckFile.is_open())
	{
		BottleneckFile.width(12);
		BottleneckFile.precision(3) ;
		BottleneckFile.setf(ios::fixed);


		g_LogFile << "The following freeway/highway bottlenecks are identified."<< endl;

		// ! there is an freeway or highway downstream with less number of lanes
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			if(g_LinkVector[li]->m_link_type <=2 &&  m_OutboundSizeAry[g_LinkVector[li]->m_ToNodeID] ==1)  // freeway or highway
			{
				int FromID = g_LinkVector[li]->m_FromNodeID;
				int ToID   = g_LinkVector[li]->m_ToNodeID;

				for(int i=0; i< m_OutboundSizeAry[ToID]; i++)
				{
					DTALink* pNextLink =  g_LinkVector[m_OutboundLinkAry[ToID][i]];
					if(pNextLink->m_link_type <=2 && pNextLink->m_NumLanes < g_LinkVector[li]->m_NumLanes && pNextLink->m_ToNodeID != FromID)
					{
						g_LinkVector[li]->m_StochaticCapcityFlag = StochasticCapacityFlag;  //lane drop from current link to next link
						BottleneckFile << "lane drop (type 1):" << g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName  << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName<< endl;
						g_LogFile << "lane drop:" << g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName << endl;
					}

				}

			}
		}


		// merge: one outgoing link, two more incoming links with at least freeway link

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			if( (g_LinkVector[li]->m_link_type <=2 || g_LinkVector[li]->m_link_type ==9)
				 && m_InboundSizeAry[g_LinkVector[li]->m_FromNodeID]>=2 
				 && m_OutboundSizeAry[g_LinkVector[li]->m_FromNodeID]==1)
			{
				bool UTurnLink = false;
				for(int il = 0; il<m_InboundSizeAry[g_LinkVector[li]->m_FromNodeID]; il++)
				{
					if(g_LinkVector[m_InboundLinkAry[g_LinkVector[li]->m_FromNodeID][il]]->m_FromNodeID == g_LinkVector[li]->m_ToNodeID)  // one of incoming link has from node as the same as this link's to node
					{
						UTurnLink = true;				
					}

				}
				if(UTurnLink == false)
					g_LinkVector[li]->m_bMergeFlag = 1;

			}
		}

		// first count # of incoming freeway, highway or ramp links to each freeway/highway link
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			int FromID = g_LinkVector[li]->m_FromNodeID;
			if(g_LinkVector[li]->m_bMergeFlag ==1 && m_InboundSizeAry[FromID] == 2)  // is a merge bottlebeck link with two incoming links
			{
				int il;
				bool bRampExistFlag = false;
				bool bFreewayExistFlag = false;

				for(il = 0; il<m_InboundSizeAry[FromID]; il++)
				{
					if(g_LinkVector[m_InboundLinkAry[FromID][il]]->m_link_type == 9)  // on ramp as incoming link
					{
						bRampExistFlag = true;
						g_LinkVector[li]->m_MergeOnrampLinkID = m_InboundLinkAry[FromID][il];
					}
					if(g_LinkVector[m_InboundLinkAry[FromID][il]]->m_link_type <= 2)  // freeway or highway
					{
						bFreewayExistFlag = true;
						g_LinkVector[li]->m_MergeMainlineLinkID = m_InboundLinkAry[FromID][il];
					}
					if(bRampExistFlag && bFreewayExistFlag)
					{
						g_LinkVector[li]->m_bMergeFlag = 2; // merge with ramp and mainline street
						g_LogFile << "merge with ramp:" << g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName  << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName ;
						g_LogFile << " with onramp:" << g_NodeVector[g_LinkVector[g_LinkVector[li]->m_MergeOnrampLinkID]->m_FromNodeID].m_NodeName  << " ->" << g_NodeVector[g_LinkVector[g_LinkVector[li]->m_MergeOnrampLinkID]->m_ToNodeID].m_NodeName ;
						g_LogFile << " and freeway mainline:" << g_NodeVector[g_LinkVector[g_LinkVector[li]->m_MergeMainlineLinkID]->m_FromNodeID ].m_NodeName << " ->" << g_NodeVector[g_LinkVector[g_LinkVector[li]->m_MergeMainlineLinkID]->m_ToNodeID].m_NodeName << endl;
						BottleneckFile << "freeway mainline (type 3):" << g_NodeVector[g_LinkVector[g_LinkVector[li]->m_MergeMainlineLinkID]->m_FromNodeID].m_NodeName  << " ->" << g_NodeVector[g_LinkVector[g_LinkVector[li]->m_MergeMainlineLinkID]->m_ToNodeID].m_NodeName << endl;
						BottleneckFile << "onramp (type 2) :" << g_NodeVector[g_LinkVector[g_LinkVector[li]->m_MergeOnrampLinkID]->m_FromNodeID].m_NodeName  << " ->" << g_NodeVector[g_LinkVector[g_LinkVector[li]->m_MergeOnrampLinkID]->m_ToNodeID].m_NodeName ;
						break;
					}

				}


			}

			if(g_LinkVector[li]->m_bMergeFlag ==1)
			{
				// merge with several merging ramps
				int ij;
				int TotalNumberOfLanes = 0;
				for( ij= 0; ij<m_InboundSizeAry[FromID]; ij++)
				{
					TotalNumberOfLanes += g_LinkVector[ m_InboundLinkAry[FromID][ij]]->m_NumLanes ;
				}

				for( ij= 0; ij<m_InboundSizeAry[FromID]; ij++)
				{
					MergeIncomingLink mil;
					mil.m_LinkID = m_InboundLinkAry[FromID][ij];
					mil.m_link_type = g_LinkVector[mil.m_LinkID]->m_link_type ;
					mil.m_NumLanes = g_LinkVector[mil.m_LinkID]->m_NumLanes ;
					mil.m_LinkInCapacityRatio = (float)(mil.m_NumLanes)/TotalNumberOfLanes;
					g_LinkVector[li]->MergeIncomingLinkVector.push_back(mil);
					g_LogFile << "merge into freeway with multiple freeway/ramps:" << "No." << ij << " " << g_NodeVector[g_LinkVector[mil.m_LinkID]->m_FromNodeID].m_NodeName  << " -> " << g_NodeVector[g_LinkVector[mil.m_LinkID]->m_ToNodeID].m_NodeName <<  " with " << g_LinkVector[mil.m_LinkID]->m_NumLanes  << " lanes and in flow capacity split " << mil.m_LinkInCapacityRatio << endl;
					BottleneckFile << "merge into freeway with multiple freeway/ramps: (type 3)" << g_NodeVector[g_LinkVector[mil.m_LinkID]->m_FromNodeID].m_NodeName << " -> " << g_NodeVector[g_LinkVector[mil.m_LinkID]->m_ToNodeID].m_NodeName<< endl;
				}

			}

		}
	}else
	{
		cout << "Error: File bottleneck.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}
}


CString g_GetAppRunningTime()
{

	CString str;

	CTime EndTime = CTime::GetCurrentTime();
	CTimeSpan ts = EndTime  - g_AppStartTime;

	str = ts.Format( "App Clock: %H:%M:%S --" );
	return str;
}


