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
#include "Geometry.h"
#include "GlobalData.h"
#include "CSVParser.h"

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
DTANetworkForSP::BuildNetworkBasedOnZoneCentriod(int ZoneID)

step 3: convert time-dependent OD demand to vehicles data, sorted by departure time
CreateVehicles(int originput_zone, int destination_zone, float number_of_vehicles, int demand_type, float starting_time_in_min, float ending_time_in_min)


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
allow multiple centriods per zone in zone.csv, change BuildNetworkBasedOnZoneCentriod()

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

std::map<int, DTAZone> g_ZoneMap;
std::vector<DTAVehicleType> g_VehicleTypeVector;

std::vector<DTAVehicle*>		g_VehicleVector;

std::map<int, DTAVehicle*> g_VehicleMap;
std::map<int, DemandType> g_DemandTypeMap;

std::map<int, int> g_LinkTypeFreewayMap;
std::map<int, int> g_LinkTypeArterialMap;
std::map<int, int> g_LinkTypeRampMap;
std::vector<DTALinkType> g_LinkTypeVector;

// time inteval settings in assignment and simulation
double g_DTASimulationInterval = 0.10000; // min
int g_number_of_intervals_per_min = 10; // round to nearest integer
int g_AggregationTimetInterval = 15; // min
int g_AggregationTimetIntervalSize = 2;
float g_DemandGlobalMultiplier = 1.0f;

// maximal # of adjacent links of a node (including physical nodes and centriods( with connectors))
int g_AdjLinkSize = 30; // initial value of adjacent links

int g_ODZoneSize = 0;

// assignment and simulation settings
int g_NumberOfIterations = 1;
int g_ParallelComputingMode = 1;
int g_AgentBasedAssignmentFlag = 0;
float g_ConvergencyRelativeGapThreshold_in_perc;
int g_NumberOfInnerIterations;

int g_VehicleLoadingMode = 0; // not load from vehicle file by default, 1: load vehicle file
int g_PlanningHorizon = 600;

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

float g_UserClassPerceptionErrorRatio[MAX_SIZE_INFO_USERS] = {0};
float g_VMSPerceptionErrorRatio;

int g_information_updating_interval_of_en_route_info_travelers_in_min;


int g_LearningPercentage = 15;
int g_TravelTimeDifferenceForSwitching = 5;  // min

int g_StochasticCapacityMode = 0;
float g_MinimumInFlowRatio = 0.1f;
float g_MaxDensityRatioForVehicleLoading = 0.8f;
int g_CycleLength_in_seconds;
float g_DefaultSaturationFlowRate_in_vehphpl;


std::vector<VOTDistribution> g_VOTDistributionVector;
std::vector<TimeDependentDemandProfile> g_TimeDependentDemandProfileVector;
int g_DemandLoadingStartTimeInMin = 1440;
int g_DemandLoadingEndTimeInMin = 0;

ofstream g_scenario_short_description;
ofstream ShortSimulationLogFile;

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
int g_EmissionDataOutputFlag = 0;


int g_TollingMethodFlag = 0;
float g_VMTTollingRate = 0;

int g_MergeNodeModelFlag=1;

std::vector<NetworkMOE>  g_NetworkMOEAry;
std::vector<NetworkLoadingOutput>  g_AssignmentMOEVector;

using namespace std;

CTime g_AppStartTime;
unsigned int g_RandomSeed = 100;
unsigned int g_RandomSeedForVehicleGeneration = 101;
// Linear congruential generator 
#define g_LCG_a 17364
#define g_LCG_c 0
#define g_LCG_M 65521  

extern void g_RunStaticExcel();
float g_GetRandomRatio()
{
	g_RandomSeed = (g_LCG_a * g_RandomSeed + g_LCG_c) % g_LCG_M;  //m_RandomSeed is automatically updated.

	return float(g_RandomSeed)/g_LCG_M;

	//	return float(rand())/RAND_MAX;
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
	pPhysicalNetwork->BuildPhysicalNetwork();

	pPhysicalNetwork->TDLabelCorrecting_DoubleQueue(OriginForTesting,0,1,DEFAULT_VOT,false,false);  // CurNodeID is the node ID
	// assign shortest path calculation results to label array


	int count = 0;
	int centroid_count = 0;

	/*
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
	*/
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

void ReadScenarioInputFiles(DTANetworkForSP* pPhysicalNetwork)
{
	//******************************* scenario input //
	//

	FILE* st = NULL;

	fopen_s(&st,"Scenario_Incident.csv","r"); /// 
	if(st!=NULL)
	{
		cout << "Reading file Incident.csv..."<< endl;
		g_LogFile << "Reading file Incident.csv, "<< endl;
		int count = 0;
		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1) 
				break;
			int dsn =  g_read_integer(st);

			int LinkID = pPhysicalNetwork->GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

			DTALink* pLink = g_LinkVector[LinkID];

			if(pLink!=NULL)
			{
				CapacityReduction cs;
				cs.StartTime = g_read_integer(st);
				cs.EndTime = g_read_integer(st);
				cs.LaneClosureRatio= g_read_float(st)/100.0f; // percentage -> to ratio
				if(cs.LaneClosureRatio > 1.0)
					cs.LaneClosureRatio = 1.0;
				if(cs.LaneClosureRatio < 0.0)
					cs.LaneClosureRatio = 0.0;

				cs.SpeedLimit = pLink->m_SpeedLimit ; // use default speed limit for incidents
				pLink->CapacityReductionVector.push_back(cs);
				count++;

			}
		}

		g_scenario_short_description << "with " << count << "incident records;";
		g_LogFile << "incident records = " << count << endl;

		fclose(st);
	}

	fopen_s(&st,"Scenario_Dynamic_Message_Sign.csv","r");
	if(st!=NULL)
	{
		cout << "Reading file Dynamic_Message_Sign.csv..."<< endl;
		g_LogFile << "Reading file Dynamic_Message_Sign.csv, "<< endl;

		int count = 0;
		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);
			int LinkID = pPhysicalNetwork->GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

			DTALink* pLink = g_LinkVector[LinkID];

			if(pLink!=NULL)
			{
				MessageSign is;

				is.Type = 1;
				is.StartTime = g_read_integer(st);
				is.EndTime = g_read_integer(st);
				is.ResponsePercentage =  g_read_float(st);

				pLink->MessageSignVector.push_back(is);
				count++;
			}
		}

		g_scenario_short_description << "with " << count << "DMS records;";
		g_LogFile << "DMS records = " << count << endl;
		fclose(st);
	}



	fopen_s(&st,"Scenario_Link_Based_Toll.csv","r");

	if(st!=NULL)
	{
		cout << "Reading file Link_Based_Toll.csv..."<< endl;
		g_LogFile << "Reading file Link_Based_Toll.csv, "<< endl;

		int count = 0;

		while(true)
		{
			int usn  = g_read_integer(st);

			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);
			int LinkID = pPhysicalNetwork->GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

			DTALink* pLink = g_LinkVector[LinkID];

			if(pLink!=NULL)
			{
				Toll tc;  // toll collectio

				tc.StartTime = g_read_integer(st);
				tc.EndTime = g_read_integer(st);

				for(int vt = 0; vt< MAX_PRICING_TYPE_SIZE; vt++)
				{
					tc.TollRate [vt]= g_read_float(st); 
				}

				count++;
				pLink->TollVector.push_back(tc);

			}
		}

		cout << "Number of link-based toll records =  " << count << endl;

		fclose(st);

	}

	fopen_s(&st,"Scenario_Distance_Based_Toll.csv","r");

	if(st!=NULL)
	{
		cout << "Reading file Distance_Based_Toll.csv..."<< endl;
		g_LogFile << "Reading file Distance_Based_Toll.csv, "<< endl;

		int count = 0;

		while(true)
		{
			int usn  = g_read_integer(st);

			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);
			int LinkID = pPhysicalNetwork->GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

			DTALink* pLink = g_LinkVector[LinkID];

			if(pLink!=NULL)
			{
				Toll tc;  // toll collection

				tc.StartTime = g_read_integer(st);
				tc.EndTime = g_read_integer(st);

				for(int vt = 1; vt<=MAX_PRICING_TYPE_SIZE; vt++)
				{
					float per_mile_rate = g_read_float(st); 

					tc.TollRate [vt] = per_mile_rate*pLink->m_Length;
				}

				count++;
				pLink->TollVector.push_back(tc);
			}
		}

		g_LogFile << "Number of distance-based toll records =  " << count << endl;
		cout << "Number of distance-based toll records =  " << count << endl;

		g_scenario_short_description << "with " << count << "link pricing records;";

		fclose(st);
	}

	// transfer toll set to dynamic toll vector (needed for multi-core computation)
	std::set<DTALink*>::iterator iterLink;
	int count = 0;
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

			count++;		
		}

	}	
	g_LogFile << "pricing records = "<< count << endl;
}
void ReadInputFiles()
{
	int z;

	//*******************************
	// step 1: node input
	cout << "Step 1: Reading file input_node.csv..."<< endl;
	g_LogFile << "Step 1: Reading file input_node.csv..."<< endl;

	CCSVParser parser_node;
	if (parser_node.OpenCSVFile("input_node.csv"))
	{
		int i=0;
		while(parser_node.ReadRecord())
		{
			int node_id;
			DTANode* pNode = 0;

			if(parser_node.GetValueByFieldName("node_id",node_id) == false)
				break;

			DTANode Node;
			Node.m_NodeID = i;
			Node.m_ZoneID = 0;
			Node.m_NodeName = node_id;
			g_NodeVector.push_back(Node);
			g_NodeNametoIDMap[node_id] = i;
			i++;
		}
	}else
	{
		cout << "Error: File input_node.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();

	}

	//*******************************
	// step 2: link type input
	cout << "Step 2: Reading file input_link_type.csv..."<< endl;
	g_LogFile << "Step 2: Reading file input_link_type.csv.." << endl;

	CCSVParser parser_link_type;

	if (parser_link_type.OpenCSVFile("input_link_type.csv"))
	{
		while(parser_link_type.ReadRecord())
		{
			DTALinkType element;

			if(parser_link_type.GetValueByFieldName("link_type",element.link_type ) == false)
				break;

			if(parser_link_type.GetValueByFieldName("link_type_name",element.link_type_name ) == false)
				break;

			if(parser_link_type.GetValueByFieldName("freeway_flag",element.freeway_flag  ) == false)
				break;
			if(parser_link_type.GetValueByFieldName("ramp_flag",element.ramp_flag  ) == false)
				break;
			if(parser_link_type.GetValueByFieldName("arterial_flag",element.arterial_flag  ) == false)
				break;

			g_LinkTypeFreewayMap[element.link_type] = element.freeway_flag ;
			g_LinkTypeArterialMap[element.link_type] = element.arterial_flag  ;
			g_LinkTypeRampMap[element.link_type] = element.ramp_flag  ;

			g_LinkTypeVector.push_back(element);

		}
	}else
	{
		cout << "Error: File input_link_type.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}


	//*******************************
	// step 3: link data input

	cout << "Step 3: Reading file input_link.csv..."<< endl;
	g_LogFile << "Step 3: Reading file input_link.csv..." << endl;

	int i = 0;

	DTALink* pLink = 0;
	CCSVParser parser_link;
	if (parser_link.OpenCSVFile("input_link.csv"))
	{
		bool bNodeNonExistError = false;
		while(parser_link.ReadRecord())
		{
			int from_node_name = 0;
			int to_node_name = 0;
			int direction = 1;
			double length_in_mile = 1;
			int number_of_lanes = 1;
			int speed_limit_in_mph = 0;
			double capacity = 0;
			int type;
			string name, mode_code;
			double K_jam,wave_speed_in_mph;


			if(!parser_link.GetValueByFieldName("from_node_id",from_node_name)) 
			{
				cout << "Field from_node_id has not been defined in file input_link.csv. Please check.";
				getchar();
				exit(0);
			}
			if(!parser_link.GetValueByFieldName("to_node_id",to_node_name))
			{
				cout << "Field to_node_id has not been defined in file input_link.csv. Please check.";
				getchar();
				exit(0);

			}

			if(g_NodeNametoIDMap.find(from_node_name)== g_NodeNametoIDMap.end())
			{
				cout<< "From Node ID "  << from_node_name << " in input_link.csv has not be defined in input_node.csv"  << endl; 
				getchar();
				exit(0);

			}

			if(g_NodeNametoIDMap.find(to_node_name)== g_NodeNametoIDMap.end())
			{
				cout<< "To Node ID "  << to_node_name << " in input_link.csv has not be defined in input_node.csv"  << endl; 
				getchar();
				exit(0);
			}


			if(!parser_link.GetValueByFieldName("direction",direction))
				direction = 1;

			if(!parser_link.GetValueByFieldName("length_in_mile",length_in_mile))
			{
				cout<< "Field length_in_mile has not been defined in file input_link.csv. Please check.";
				getchar();
				exit(0);
			}

			if(!parser_link.GetValueByFieldName("number_of_lanes",number_of_lanes))
			{
				cout << "Field number_of_lanes has not been defined in file input_link.csv. Please check.";
				getchar();
				exit(0);
			}

			if(!parser_link.GetValueByFieldName("speed_limit_in_mph",speed_limit_in_mph))
			{
				cout << "Field speed_limit_in_mph has not been defined in file input_link.csv. Please check.";
				getchar();
				exit(0);
			}

			if(!parser_link.GetValueByFieldName("lane_capacity_in_vhc_per_hour",capacity))
			{
				cout << "Field lane_capacity_in_vhc_per_hour has not been defined in file input_link.csv. Please check.";
				getchar();
				exit(0);

			}

			if(!parser_link.GetValueByFieldName("link_type",type))
			{
				cout << "Field link_type has not been defined in file input_link.csv. Please check.";
				getchar();
				exit(0);
			}

			if(!parser_link.GetValueByFieldName("K_jam",K_jam))
				K_jam = 180;

			if(!parser_link.GetValueByFieldName("wave_speed_in_mph",wave_speed_in_mph))
				wave_speed_in_mph = 12;

			if(!parser_link.GetValueByFieldName("mode_code",mode_code))
				mode_code  = "";

			int link_code_start = 1;
			int link_code_end = 1;

			if (direction == -1) // reversed
			{
				link_code_start = 2; link_code_end = 2;
			}

			if (direction == 0) // two-directional link
			{
				link_code_start = 1; link_code_end = 2;
			}


			if(number_of_lanes == 0 || capacity <1)  // skip this link 
			{
				g_WarningFile << "link with 0 lane or 0 capacity, skip:" << pLink->m_FromNodeNumber << " -> " << pLink->m_ToNodeNumber << 
					" (" << 	pLink->m_FromNodeID << " -> "  << pLink->m_ToNodeID << ")" << endl;

				continue;
			}

			if(speed_limit_in_mph <0.1)  //reset the speed limit
				speed_limit_in_mph = 5;  // minium speed limit

			for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
			{

				int FromID = from_node_name;
				int ToID = to_node_name;
				if(link_code == 1)  //AB link
				{
					FromID = from_node_name;
					ToID = to_node_name;
				}
				if(link_code == 2)  //BA link
				{
					FromID = to_node_name;
					ToID = from_node_name;
				}

				pLink = new DTALink(g_PlanningHorizon);
				pLink->m_LinkID = i;
				pLink->m_FromNodeNumber = FromID;
				pLink->m_ToNodeNumber = ToID;
				pLink->m_FromNodeID = g_NodeNametoIDMap[pLink->m_FromNodeNumber ];
				pLink->m_ToNodeID= g_NodeNametoIDMap[pLink->m_ToNodeNumber];


				if( pLink->m_FromNodeNumber == 321984)
				{
					TRACE("find link %d\n", i);
				}

				float length = max(0.00001,length_in_mile);  // minimum link length 0.0001
				pLink->m_NumLanes= number_of_lanes;

				for(int LaneNo = 0; LaneNo < pLink->m_NumLanes; LaneNo++)
				{
					LaneVehicleCFData element; 
					pLink->m_VehicleDataVector.push_back (element);
				}
				pLink->m_SpeedLimit= speed_limit_in_mph;

				if(g_AgentBasedAssignmentFlag != 2)
					pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // we do not impose the minimum distance in this version
				else
					pLink->m_Length= length;

				pLink->m_MaximumServiceFlowRatePHPL= capacity;

				if(capacity < 10)
				{
					cout << "In file input_link.csv, line "<< i+1 << " has capacity <10" << capacity <<", which might not be realistic. Please correct the error" << endl;
					getchar();
					exit(0);
				}

				pLink->m_BPRLaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
				pLink->m_link_type= type;

				if(g_LinkTypeFreewayMap.find(type) == g_LinkTypeFreewayMap.end())
				{
					cout << "In file input_link.csv, line "<< i+1 << " has link type "<< type <<", which has not been defined in input_link_type.csv. Please correct the error" << endl;
					getchar();
					exit(0);			
				}

				pLink->m_KJam = K_jam;
				pLink->m_BackwardWaveSpeed = wave_speed_in_mph;

				pLink->m_VehicleSpaceCapacity = pLink->m_Length * pLink->m_NumLanes *K_jam; // bump to bump density


				g_NodeVector[pLink->m_FromNodeID ].m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);

				pLink->SetupMOE();
				g_LinkVector.push_back(pLink);
				i++;

				if(i == MAX_LINK_NO && g_AgentBasedAssignmentFlag != 2) // g_AgentBasedAssignmentFlag == 2  -> no vehicle simulation
				{
					cout << "The network has more than "<< MAX_LINK_NO << " links."<< endl <<"Please contact the developers for a new 64 bit version for this large-scale network." << endl;
					getchar();
					exit(0);

				}

			}
		}
	}else
	{
		cout << "Error: File input_link.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}


	if(g_AgentBasedAssignmentFlag == 2)
	{

		g_AgentBasedShortestPathGeneration();

		return;
	}

	//*******************************
	// step 4: zone input

	cout << "Step 4: Reading file input_zone.csv..."<< endl;
	g_LogFile << "Step 4: Reading file input_zone.csv..." << endl;



	// check how many centroids

	CCSVParser parser_zone2;

	if (parser_zone2.OpenCSVFile("input_zone.csv"))
	{
		int i=0;
		while(parser_zone2.ReadRecord())
		{
			int zone_number;

			if(parser_zone2.GetValueByFieldName("zone_id",zone_number) == false)
				break;

			int node_number;
			if(parser_zone2.GetValueByFieldName("node_id",node_number) == false)
				break;

			if(g_ODZoneSize < zone_number)
				g_ODZoneSize = zone_number;

			g_NodeVector[g_NodeNametoIDMap[node_number]].m_ZoneID = zone_number;

			g_ZoneMap[zone_number].m_CentroidNodeAry.push_back( g_NodeNametoIDMap[node_number]);
			g_ZoneMap[zone_number].m_Capacity += g_NodeVector[g_NodeNametoIDMap[node_number]].m_TotalCapacity ;

			if(g_ZoneMap[zone_number].m_CentroidNodeAry.size() > g_AdjLinkSize)
				g_AdjLinkSize = g_ZoneMap[zone_number].m_CentroidNodeAry.size();
		}

	}

	for (std::map<int, DTAZone>::iterator iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
	{
		if(iterZone->second.m_CentroidNodeAry.size() > 30)
		{
			g_LogFile << "Zone " << iterZone->first << ": # of connectors:" <<  iterZone->second.m_CentroidNodeAry.size()<< endl;
		}
	}

	//*******************************
	// step 5: vehicle type input

	cout << "Step 5: Reading file input_vehicle_type.csv..."<< endl;
	g_LogFile << "Step 5: Reading file input_vehicle_type.csv..."<< endl;

	CCSVParser parser_vehicle_type;

	if (parser_vehicle_type.OpenCSVFile("input_vehicle_type.csv"))
	{
		while(parser_vehicle_type.ReadRecord())
		{
			int vehicle_type;
			if(parser_vehicle_type.GetValueByFieldName("vehicle_type",vehicle_type) == false)
				break;

			string vehicle_type_name;
			parser_vehicle_type.GetValueByFieldName("vehicle_type_name",vehicle_type_name);

			DTAVehicleType element;
			element.vehicle_type = vehicle_type;
			element.vehicle_type_name  = vehicle_type_name.c_str ();

			g_VehicleTypeVector.push_back(element);

		}

	}

	//*******************************
	// step 6: demand type input


	CCSVParser parser_demand_type;
	cout << "Step 6: Reading file input_demand_type.csv..."<< endl;
	g_LogFile << "Step 6: Reading file input_demand_type.csv..."<< endl;

	if (parser_demand_type.OpenCSVFile("input_demand_type.csv"))
	{
		while(parser_demand_type.ReadRecord())
		{
			int demand_type =1;
			int pricing_type = 0;
			float average_VOT = 10;

			if(parser_demand_type.GetValueByFieldName("demand_type",demand_type) == false)
				break;

			if(parser_demand_type.GetValueByFieldName("average_VOT",average_VOT) == false)
				break;

			float ratio_pretrip = 0;
			float ratio_enroute = 0;

			if(parser_demand_type.GetValueByFieldName("pricing_type",pricing_type) == false)
				break;

			parser_demand_type.GetValueByFieldName("percentage_of_pretrip_info",ratio_pretrip);
			parser_demand_type.GetValueByFieldName("percentage_of_enroute_info",ratio_enroute);

			DemandType element;
			element.demand_type = demand_type;
			element.average_VOT = average_VOT;

			element.pricing_type = pricing_type;
			element.info_class_percentage[1] = ratio_pretrip;
			element.info_class_percentage[2] = ratio_enroute;
			element.info_class_percentage[0] = 100 - ratio_enroute - ratio_pretrip;

			for(int ic = 1; ic < MAX_INFO_CLASS_SIZE; ic++)
			{
				element.cumulative_info_class_percentage[ic] = element.cumulative_info_class_percentage[ic-1] + element.info_class_percentage[ic];
			}
			for( int i=0; i< g_VehicleTypeVector.size(); i++)
			{
				std::ostringstream  str_percentage_of_vehicle_type;
				str_percentage_of_vehicle_type << "percentage_of_vehicle_type" << i+1;

				float percentage_vehicle_type = 0;
				if(parser_demand_type.GetValueByFieldName(str_percentage_of_vehicle_type.str(),percentage_vehicle_type) == false)
				{
					cout << "Error: Field percentage_of_vehicle_type " << i+1 << " cannot be found in the input_demand_type.csv file.";
					g_ProgramStop();
					return;
				}else
				{
					element.vehicle_type_percentage[i+1] = percentage_vehicle_type;

					if(i>0)
					{
						element.cumulative_type_percentage [i+1] = element.cumulative_type_percentage [i] + percentage_vehicle_type;
					}

				}
			}


			g_DemandTypeMap[demand_type] = element;

		}

	}else
	{
		cout << "Error: File input_demand_type.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}

	//*******************************
	// step 7: time-dependent demand profile input

	if(g_TrafficFlowModelFlag==0)
	{
		cout << "Step 7: Static Traffic Assignment Mode: Skip reading file input_temporal_demand_profile.csv..."<< endl;
		g_LogFile << "Step 7: Static Traffic Assignment Mode: Skip reading file input_temporal_demand_profile.csv..."<< endl;
	}else
	{
		cout << "Step 7: Reading file input_temporal_demand_profile.csv..."<< endl;
		g_LogFile << "Step 7: Reading file input_temporal_demand_profile.csv..."<< endl;
	
	CCSVParser parser_TDProfile;

	if (parser_TDProfile.OpenCSVFile("input_temporal_demand_profile.csv"))
	{

		while(parser_TDProfile.ReadRecord())
		{
			int from_zone_id = 0;
			int to_zone_id = 0;
			int demand_type = 0;

			if(parser_TDProfile.GetValueByFieldName("from_zone_id",from_zone_id) == false)
				break;

			if(parser_TDProfile.GetValueByFieldName("to_zone_id",to_zone_id) == false)
				break;

			if(parser_TDProfile.GetValueByFieldName("demand_type",demand_type) == false)
				break;

			TimeDependentDemandProfile element; 
			element.demand_type = demand_type;
			element.from_zone_id = from_zone_id;
			element.to_zone_id = to_zone_id;


			for(int t = 0; t< MAX_TIME_INTERVAL_SIZE; t++)
			{
				std::string time_stamp_str = g_GetTimeStampStrFromIntervalNo (t);
				double ratio = 0;
				parser_TDProfile.GetValueByFieldName(time_stamp_str,ratio);


				if(ratio > 0.0001)
				{
					element.time_dependent_ratio[t] = ratio;
				}
			}

			g_TimeDependentDemandProfileVector.push_back (element);

		}


	}
	}



	//*******************************
	// step 8: VOT input

	////////////////////////////////////// VOT
	cout << "Step 8: Reading file input_VOT.csv..."<< endl;
	g_LogFile << "Step 8: Reading file input_VOT.csv..."<< endl;

	CCSVParser parser_VOT;

	float cumulative_percentage = 0;

	if (parser_VOT.OpenCSVFile("input_VOT.csv"))
	{
		int i=0;
		int old_demand_type = 0;
		while(parser_VOT.ReadRecord())
		{
			int demand_type = 0;

			if(parser_VOT.GetValueByFieldName("demand_type",demand_type) == false)
				break;

			if(demand_type!= old_demand_type)
				cumulative_percentage= 0;   //switch vehicle type, reset cumulative percentage


			int VOT;
			if(parser_VOT.GetValueByFieldName("VOT_dollar_per_hour",VOT) == false)
				break;

			float percentage;
			if(parser_VOT.GetValueByFieldName("percentage",percentage) == false)
				break;

			old_demand_type= demand_type;
			VOTDistribution element;
			element.demand_type = demand_type;
			element.percentage  = percentage;
			element.VOT = VOT;
			element.cumulative_percentage_LB = cumulative_percentage;
			cumulative_percentage+=percentage;
			element.cumulative_percentage_UB = cumulative_percentage;
			//			TRACE("Pricing type = %d, [%f,%f]\n",pricing_type,element.cumulative_percentage_LB,element.cumulative_percentage_UB);
			g_VOTDistributionVector.push_back(element);

		}

	}
	//*******************************
	// step 9: Crash Prediction input

	////////////////////////////////////// VOT
	cout << "Step 9: Reading file input_crash_prediction.csv..."<< endl;
	g_LogFile << "Step 9: Reading file input_crash_prediction.csv..."<< endl;

	/*	CCSVParser parser_safety;

	if (parser_safety.OpenCSVFile("input_crash_prediction_model.csv"))
	{
	while(parser_safety.ReadRecord())
	{


	parser_safety.GetValueByFieldName("safety_crash_model_id",element.safety_crash_model_id);
	parser_safety.GetValueByFieldName("model_name",element.model_name);
	parser_safety.GetValueByFieldName("alpha_constant",element.alpha_constant);
	parser_safety.GetValueByFieldName("beta_AADT",element.beta_AADT);
	parser_safety.GetValueByFieldName("gamma_AADT",element.gamma_AADT);
	parser_safety.GetValueByFieldName("t_driveway",element.t_driveway);
	parser_safety.GetValueByFieldName("n_driveway",element.n_driveway);
	parser_safety.GetValueByFieldName("proportion_fatal",element.proportion_fatal);
	parser_safety.GetValueByFieldName("length_coeff",element.length_coeff);
	parser_safety.GetValueByFieldName("on_ramp_ADT_coeff",element.on_ramp_ADT_coeff);
	parser_safety.GetValueByFieldName("off_ramp_ADT_coeff",element.off_ramp_ADT_coeff);
	parser_safety.GetValueByFieldName("upstream_DADT_coeff",element.upstream_DADT_coeff);
	parser_safety.GetValueByFieldName("freeway_constant",element.freeway_constant);
	parser_safety.GetValueByFieldName("freeway_lanes_coeff",element.freeway_lanes_coeff);
	parser_safety.GetValueByFieldName("inverse_spacing_coeff",element.inverse_spacing_coeff);
	parser_safety.GetValueByFieldName("avg_capacity_reduction_percentage",element.avg_capacity_reduction_percentage);
	parser_safety.GetValueByFieldName("avg_crash_duration_in_min",element.avg_crash_duration_in_min);
	parser_safety.GetValueByFieldName("avg_additional_delay_per_vehicle_per_crash_in_min",element.avg_additional_delay_per_vehicle_per_crash_in_min);

	g_SafetyModelVector.push_back(element);

	}
	}

	*/
	////////////////////////////



	// done with zone.csv
	DTANetworkForSP PhysicalNetwork(g_NodeVector.size(), g_LinkVector.size(), g_PlanningHorizon,g_AdjLinkSize);  //  network instance for single processor in multi-thread environment
	PhysicalNetwork.BuildPhysicalNetwork();
//	PhysicalNetwork.IdentifyBottlenecks(g_StochasticCapacityMode);
//	ConnectivityChecking(&PhysicalNetwork);

	//*******************************
	// step 10: demand trip file input

	// initialize the demand loading range, later resized by CreateVehicles
	g_DemandLoadingStartTimeInMin = 1440;
	g_DemandLoadingEndTimeInMin = 0;

	if(g_VehicleLoadingMode == 0)  // load from demand table
	{ 
		////////////////////////////////////// VOT
		cout << "Step 10: Reading file input_demand.csv..."<< endl;
		g_LogFile << "Step 10: Reading file input_demand.csv..."<< endl;

		g_ReadDemandFile();

	}else
	{  // load from vehicle file
		////////////////////////////////////// VOT
		cout << "Step 10: Reading file input_vehicle.csv..."<< endl;
		g_LogFile << "Step 10: Reading file input_vehicle.csv..."<< endl;

		ReadDTALiteVehicleFile("input_vehicle.csv",&PhysicalNetwork);

	}


	if(g_ODEstimationFlag == 1)  //  OD estimation mode 1: read measurement data directly
	{
		//*******************************
		cout << "Step 12: Reading file input_sensor.csv for OD Demand Adjustment"<< endl;
		g_LogFile << "Step 12: Reading file input_sensor.csv for OD Demand Adjustment"<< endl;

		g_ReadLinkMeasurementFile(&PhysicalNetwork);
		// second step: start reading historical demand
		//*******************************
		// step 12: historical demand input: in the current implementation, the starting demand is the historical OD demand matrix

	}


	/* to do: change this to the loop for map structure 
	for(z = 1; z <=g_ODZoneSize; z++)
	{
	if(g_ZoneMap[z]->m_Demand >= g_ZoneMap[z].m_Capacity )
	g_WarningFile << "Zone "<< z << " has demand " << g_ZoneMap[z]->m_Demand  << " and capacity " << g_ZoneMap[z].m_Capacity<< endl;

	}
	*/
	//*******************************
	// step 9: Crash Prediction input

	ReadScenarioInputFiles(&PhysicalNetwork);


	//	cout << "Global Loading Factor = "<< g_DemandGlobalMultiplier << endl;


	cout << "Number of Zones = "<< g_ODZoneSize  << endl;
	cout << "Number of Nodes = "<< g_NodeVector.size() << endl;
	cout << "Number of Links = "<< g_LinkVector.size() << endl;
	cout << "Number of Vehicles to be Simulated = "<< g_VehicleVector.size() << endl;
	cout <<	"Demand Loading Period = " << g_DemandLoadingStartTimeInMin << " min -> " << g_DemandLoadingEndTimeInMin << " min." << endl;

	if(g_DemandLoadingEndTimeInMin + 300 > g_PlanningHorizon)
	{
		//reset simulation horizon to make sure it is longer than the demand loading horizon
		g_PlanningHorizon = g_DemandLoadingEndTimeInMin+ 300;

		for(unsigned link_index = 0; link_index< g_LinkVector.size(); link_index++)
		{
			DTALink* pLink = g_LinkVector[link_index];
			pLink ->ResizeData (g_PlanningHorizon);
		}

	}
	g_NetworkMOEAry.resize (g_PlanningHorizon+1);  // "+1" as simulation time starts as 0
	g_AssignmentMOEVector.resize ( g_NumberOfIterations+1);  // "+1" as assignment iteration starts as 0


	cout << "Number of Vehicle Types = "<< g_VehicleTypeVector.size() << endl;
	cout << "Number of Demand Types = "<< g_DemandTypeMap.size() << endl;
	cout << "Number of Temporal Profile Records = "<< g_TimeDependentDemandProfileVector.size() << endl;
	cout << "Number of VOT records = "<< g_VOTDistributionVector.size() << endl;

	g_LogFile << "Number of Zones = "<< g_ODZoneSize  << endl;
	g_LogFile << "Number of Nodes = "<< g_NodeVector.size() << endl;
	g_LogFile << "Number of Links = "<< g_LinkVector.size() << endl;
	g_LogFile << "Number of Vehicles to be Simulated = "<< g_VehicleVector.size() << endl;
	g_LogFile << "Demand Loading Period = " << g_DemandLoadingStartTimeInMin << " min -> " << g_DemandLoadingEndTimeInMin << " min." << endl;
	g_LogFile << "Number of Vehicle Types = "<< g_VehicleTypeVector.size() << endl;
	g_LogFile << "Number of Demand Types = "<< g_DemandTypeMap.size() << endl;
	g_LogFile << "Number of Temporal Profile Records = "<< g_TimeDependentDemandProfileVector.size() << endl;
	g_LogFile << "Number of VOT records = "<< g_VOTDistributionVector.size() << endl;

}

void CreateVehicles(int originput_zone, int destination_zone, float number_of_vehicles, int demand_type, float starting_time_in_min, float ending_time_in_min, long PathIndex)
{
	if( originput_zone == destination_zone)  // do not simulate intra-zone traffic
		return; 

	// reset the range of demand loading interval

	if( starting_time_in_min < g_DemandLoadingStartTimeInMin)
		g_DemandLoadingStartTimeInMin = starting_time_in_min; 

	if( ending_time_in_min > g_DemandLoadingEndTimeInMin)
		g_DemandLoadingEndTimeInMin = ending_time_in_min; 


	int number_of_vehicles_to_be_generated = g_GetRandomInteger_From_FloatingPointValue(number_of_vehicles);

	for(int i=0; i< number_of_vehicles_to_be_generated; i++)
	{
		DTA_vhc_simple vhc;
		vhc.m_OriginZoneID = originput_zone;
		vhc.m_DestinationZoneID = destination_zone;
		vhc.m_PathIndex = PathIndex;

		g_ZoneMap[originput_zone].m_OriginVehicleSize ++;

		// generate the random departure time during the interval
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

		// important notes: to save memory and for simplicity, DTALite simulation still uses interval clock starts from as 0
		// when we output the all statistics, we need to 
		vhc.m_DemandType = demand_type;
		g_GetVehicleAttributes(vhc.m_DemandType, vhc.m_VehicleType, vhc.m_PricingType, vhc.m_InformationClass , vhc.m_VOT);

		g_simple_vector_vehicles.push_back(vhc);
	}
}
void ReadDTALiteVehicleFile(char fname[_MAX_PATH], DTANetworkForSP* pPhysicalNetwork)
{


	if(g_TrafficFlowModelFlag ==0)  //BRP  // static assignment parameters
	{
		g_DemandLoadingStartTimeInMin = 0;
		g_DemandLoadingEndTimeInMin = 60;
		g_AggregationTimetInterval =  60;
	}else   // dynamic traffic assignment
	{
	g_DemandLoadingStartTimeInMin= 0;
	g_DemandLoadingEndTimeInMin= int(g_PlanningHorizon*1.0f/g_AggregationTimetInterval+0.49f)*g_AggregationTimetInterval;  // round to the nearest large number
	
	}

	g_AggregationTimetIntervalSize = max(1,(g_DemandLoadingEndTimeInMin)/g_AggregationTimetInterval);
	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ODZoneSize+1, g_AggregationTimetIntervalSize);


	int   PathNodeList[MAX_NODE_SIZE_IN_A_PATH];

	cout << "Reading vehicle file as simulation input... "  << endl;

	FILE* st = NULL;
	fopen_s(&st,fname,"r");
	if(st!=NULL)
	{
		while(!feof(st))
		{
			int vehicle_id     = g_read_integer(st);
			if(vehicle_id == -1)  // reach end of file
				break;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_VehicleID = vehicle_id;
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
			pVehicle->m_DemandType = g_read_integer(st);

			if(g_DemandTypeMap.find(pVehicle->m_DemandType) == g_DemandTypeMap.end())
			{
				cout << "Error: The input_vehicle file has demand_type = " << pVehicle->m_DemandType << "for vehicle_id = " << vehicle_id << ", which has not been defined in input_demand_type.csv."<< endl;
				g_ProgramStop();
			}
			pVehicle->m_PricingType  = g_read_integer(st) -1;  // internal pricing type value starts from 0
			pVehicle->m_VehicleType = g_read_integer(st);

			pVehicle->m_InformationClass = g_read_integer(st);
			pVehicle->m_VOT = g_read_float(st);
			pVehicle->m_TollDollarCost = g_read_float(st);
			pVehicle->m_Emissions  = g_read_float(st);
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
					float travel_time = g_read_float(st);
					float emissions = g_read_float(st);
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

			int AssignmentInterval = int(pVehicle->m_DepartureTime/g_AggregationTimetInterval);

			if(AssignmentInterval >= g_AggregationTimetIntervalSize)
			{
				AssignmentInterval = g_AggregationTimetIntervalSize - 1;
			}
			g_TDOVehicleArray[pVehicle->m_OriginZoneID][AssignmentInterval].VehicleArray .push_back(pVehicle->m_VehicleID);


		}
		fclose(st);	
	}else
	{
		cout << "Error: File input_vehicle.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}
	g_scenario_short_description << "load vehicles from input_vehicle.csv;";
}


void g_ConvertDemandToVehicles() 
{
	g_LogFile << "Total number of vehicles to be simulated = "<< g_simple_vector_vehicles.size() << endl;

	std::sort(g_simple_vector_vehicles.begin(), g_simple_vector_vehicles.end());

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

			pVehicle->m_OriginNodeID	= g_ZoneMap[kvhc->m_OriginZoneID].GetRandomNodeIDInZone((pVehicle->m_VehicleID%100)/100.0f);  // use pVehicle->m_VehicleID/100.0f as random number between 0 and 1, so we can reproduce the results easily
			pVehicle->m_DestinationNodeID 	=  g_ZoneMap[kvhc->m_DestinationZoneID].GetRandomNodeIDInZone((pVehicle->m_VehicleID%100)/100.0f); 

			pVehicle->m_DepartureTime	= kvhc->m_DepartureTime;
			pVehicle->m_TimeToRetrieveInfo = (int)(pVehicle->m_DepartureTime*10);

			pVehicle->m_DemandType	= kvhc->m_DemandType;
			pVehicle->m_PricingType 	= kvhc->m_PricingType ;
			pVehicle->m_VehicleType = kvhc->m_VehicleType;

			pVehicle->m_InformationClass = kvhc->m_InformationClass;
			pVehicle->m_VOT = kvhc->m_VOT;

			pVehicle->m_NodeSize = 0;  // initialize NodeSize as o
			g_VehicleVector.push_back(pVehicle);

			g_VehicleMap[i]  = pVehicle;

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
	cout << "Total number of vehicles to be simulated = "<< g_simple_vector_vehicles.size() << endl;
	g_simple_vector_vehicles.clear ();
}
void g_ReadDemandFile()
{
	bool bFileReady = false;
	int i;

	CCSVParser parser_demand;

	float total_demand_in_demand_file = 0;
	float total_number_of_vehicles_to_be_generated = 0;

	if (parser_demand.OpenCSVFile("input_demand.csv"))
	{
		bFileReady = true;
		int line_no = 1;

		while(parser_demand.ReadRecord())
		{
			int originput_zone, destination_zone;
			float number_of_vehicles ;
			int demand_type;
			float starting_time_in_min;
			float ending_time_in_min;

			if(parser_demand.GetValueByFieldName("from_zone_id",originput_zone) == false)
				break;
			if(parser_demand.GetValueByFieldName("to_zone_id",destination_zone) == false)
				break;

			if(parser_demand.GetValueByFieldName("starting_time_in_min",starting_time_in_min) == false)
			{
				starting_time_in_min = 0;
			}

			if(parser_demand.GetValueByFieldName("ending_time_in_min",ending_time_in_min) == false)
			{
				ending_time_in_min = 60;
			}

			// static traffic assignment, set the demand loading horizon to [0, 60 min]
			if(g_TrafficFlowModelFlag ==0)  //BRP  // static assignment parameters
			{
			starting_time_in_min  = 0;
			ending_time_in_min = 60;
			}

			for(unsigned int demand_type = 1; demand_type <= g_DemandTypeMap.size(); demand_type++)
			{
				std::ostringstream  str_number_of_vehicles; 
				str_number_of_vehicles << "number_of_vehicle_trips_type" << demand_type;
				if(parser_demand.GetValueByFieldName(str_number_of_vehicles.str(),number_of_vehicles) == false)
				{
					cout << "Error: Field " << str_number_of_vehicles.str() << " cannot be found in input_demand.csv."<< endl;
					g_ProgramStop();
				}

				total_demand_in_demand_file += number_of_vehicles;
				number_of_vehicles*= g_DemandGlobalMultiplier;


				// we generate vehicles here for each OD data line
				//			TRACE("o:%d d: %d, %f,%d,%f,%f\n", originput_zone,destination_zone,number_of_vehicles,demand_type,starting_time_in_min,ending_time_in_min);

				if(g_ZoneMap.find(originput_zone)!= g_ZoneMap.end())
				{
					g_ZoneMap[originput_zone].m_Demand += number_of_vehicles;
					// setup historical demand value for static OD estimation
					g_ZoneMap[originput_zone].m_HistDemand[destination_zone] += number_of_vehicles;

					float Interval= ending_time_in_min - starting_time_in_min;

					float LoadingRatio = 1.0f;

					// condition 1: without time-dependent profile 

					if(g_TimeDependentDemandProfileVector.size() == 0) // no time-dependent profile
					{
						CreateVehicles(originput_zone,destination_zone,number_of_vehicles*LoadingRatio,demand_type,starting_time_in_min,ending_time_in_min);
					}else
					{
						// condition 2: with time-dependent profile 
						// for each time interval
						for(int time_interval = 0; time_interval <= MAX_TIME_INTERVAL_SIZE; time_interval++)
						{
							// go through all applicable temproal elements
							double time_dependent_ratio = 0;

							for(int i = 0; i < g_TimeDependentDemandProfileVector.size(); i++)
							{
								TimeDependentDemandProfile element = g_TimeDependentDemandProfileVector[i];
								if( (element.from_zone_id == originput_zone || element.from_zone_id == 0)
									&&(element.to_zone_id == destination_zone || element.to_zone_id == 0)
									&& (element.demand_type == demand_type || element.demand_type ==0) )
								{

									time_dependent_ratio = element.time_dependent_ratio[time_interval];

								}


							}

							if(time_dependent_ratio > 0.000001) // this is the last one applicable
							{
								// reset the time interval, create vehicles with the same origin, destination, changed # of vehicles, and time interval
								double number_of_vehicles_to_be_loaded = time_dependent_ratio* number_of_vehicles*LoadingRatio;
								starting_time_in_min = time_interval*15;
								ending_time_in_min = (time_interval+1)*15;

								CreateVehicles(originput_zone,destination_zone,number_of_vehicles_to_be_loaded,demand_type,starting_time_in_min,ending_time_in_min);
							}
						}

					}
					//  given the number of OD demand voluem to be created. we need to apply time-dependent profile for each data block , 
					if(line_no %100000 ==0)
					{
						cout << g_GetAppRunningTime() << "Reading " << line_no/1000 << "K lines..."<< endl;
					}
				}

			}
			line_no++;
		}

	}else
	{
		cout << "Error: File input_demand.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}

	g_LogFile << "Total demand volume in input_demand.csv = "<< total_demand_in_demand_file << endl;
	g_LogFile << "Demand Global Multiplier = "<< g_DemandGlobalMultiplier << endl;

	cout << "Total demand volume in input_demand.csv = " << total_demand_in_demand_file << endl;
	cout << "Demand Global Multiplier = "<< g_DemandGlobalMultiplier << endl;

	// create vehicle heres...
	cout << "Step 11: Converting demand flow to vehicles..."<< endl;
	g_scenario_short_description << "load vehicles from input_demand.csv;";

	// for static traffic assignment, reset g_AggregationTimetInterval to the demand loading horizon
	//
	if(g_TrafficFlowModelFlag ==0)  //BRP  // static assignment parameters
	{
		g_DemandLoadingStartTimeInMin = 0;
		g_DemandLoadingEndTimeInMin = 60;
		g_AggregationTimetInterval =  60;
	}

	// round the demand loading horizon using g_AggregationTimetInterval as time unit


	g_DemandLoadingStartTimeInMin= int(g_DemandLoadingStartTimeInMin/g_AggregationTimetInterval)*g_AggregationTimetInterval;
	g_DemandLoadingEndTimeInMin= int(g_DemandLoadingEndTimeInMin*1.0f/g_AggregationTimetInterval+0.49f)*g_AggregationTimetInterval;

	g_AggregationTimetIntervalSize = max(1,(g_DemandLoadingEndTimeInMin)/g_AggregationTimetInterval);
	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ODZoneSize+1, g_AggregationTimetIntervalSize);

	g_ConvertDemandToVehicles();
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

	g_ZoneMap.clear ();
	g_FreeMemoryForVehicleVector();

	DeallocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_TDOVehicleArray,g_ODZoneSize+1, g_AggregationTimetIntervalSize);  // +1 is because the zone numbers start from 1 not from 0

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
			fprintf(st, "from_node_id,to_node_id,timestamp_in_min,travel_time_in_min,delay_in_min,link_volume_in_veh_per_hour_per_lane,link_volume_in_veh_per_hour_for_all_lanes,density_in_veh_per_mile_per_lane,speed_in_mph,exit_queue_length,cumulative_arrival_count,cumulative_departure_count,cumulative_SOV_count,cumulative_HOV_count,cumulative_truck_count,cumulative_SOV_revenue,cumulative_HOV_revenue,cumulative_truck_revenue\n");
		}

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			for(int time = g_DemandLoadingStartTimeInMin; time< g_PlanningHorizon;time++)
			{
				if((g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount - g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount) > 0) // there are vehicles on the link
				{
					float LinkOutFlow = float(g_LinkVector[li]->GetDepartureFlow(time));
					float travel_time = g_LinkVector[li]->GetTravelTimeByMin(time,1);

					fprintf(st, "%d,%d,%d,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f, %d, %d, %d,",
						g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName, g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName,time,
						travel_time, travel_time - g_LinkVector[li]->m_FreeFlowTravelTime ,
						LinkOutFlow*60.0/g_LinkVector[li]->m_NumLanes ,LinkOutFlow*60.0,
						(g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount-g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount)/g_LinkVector[li]->m_Length /g_LinkVector[li]->m_NumLanes,
						g_LinkVector[li]->GetSpeed(time), g_LinkVector[li]->m_LinkMOEAry[time].ExitQueueLength, 
						g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount ,g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount);

					int pt;
					for(pt = 0; pt < MAX_PRICING_TYPE_SIZE; pt++)
					{
						fprintf(st, "%d,",g_LinkVector[li]->m_LinkMOEAry [time].CumulativeArrivalCount_PricingType[pt]); 
					}

					for(pt = 0; pt < MAX_PRICING_TYPE_SIZE; pt++)
					{
						fprintf(st, "%6.2f,",g_LinkVector[li]->m_LinkMOEAry [time].CumulativeRevenue_PricingType[pt]); 
					}

					fprintf(st,"\n");

				}



			}
		}
		fclose(st);
	}else
	{
		fprintf(g_ErrorFile, "File output_LinkMOE.csv cannot be opened. It might be currently used and locked by EXCEL.");
		cout << "Error: File output_LinkMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
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

		for(int time = g_DemandLoadingStartTimeInMin; time< g_PlanningHorizon;time++)
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


void g_OutputLinkMOESummary(char fname[_MAX_PATH])
{

	ofstream LinkMOESummaryFile;

	LinkMOESummaryFile.open (fname, ios::out);
	if (LinkMOESummaryFile.is_open())
	{
		LinkMOESummaryFile.width(12);
		LinkMOESummaryFile.precision(3) ;
		LinkMOESummaryFile.setf(ios::fixed);


		LinkMOESummaryFile << "from_node_id,to_node_id,start_time_in_min, end_time_in_min,total_link_volume,lane_capacity_in_vhc_per_hour, volume_over_capacity_ratio, speed_limit_in_mph, speed_in_mph, percentage_of_speed_limit, level_of_service, sensor_data_flag, sensor_link_volume, measurement_error_percentage, abs_measurement_error_percentage,simulated_AADT,num_of_crashes_per_year" << endl;

//		DTASafetyPredictionModel SafePredictionModel;
//		SafePredictionModel.UpdateCrashRateForAllLinks();

		std::set<DTALink*>::iterator iterLink;

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{

			DTALink* pLink = g_LinkVector[li];

			double average_travel_time = pLink->GetTravelTimeByMin(0, pLink->m_SimulationHorizon);
			double speed = pLink->m_Length / max(0.00001,average_travel_time) *60;  // unit: mph
			double capacity_simulation_horizon = pLink->m_MaximumServiceFlowRatePHPL * pLink->m_NumLanes * (g_DemandLoadingEndTimeInMin- g_DemandLoadingStartTimeInMin) / 60;
			double voc_ratio = pLink->CFlowArrivalCount / max(0.1,capacity_simulation_horizon);
			int percentage_of_speed_limit = int(speed/max(0.1,pLink->m_SpeedLimit)*100+0.5);


				LinkMOESummaryFile << g_NodeVector[pLink->m_FromNodeID].m_NodeName  << "," ;
				LinkMOESummaryFile << g_NodeVector[pLink->m_ToNodeID].m_NodeName << "," ;
				LinkMOESummaryFile << g_DemandLoadingStartTimeInMin << "," ;
				LinkMOESummaryFile << g_DemandLoadingEndTimeInMin << "," ;
				LinkMOESummaryFile << pLink->CFlowArrivalCount << "," ; // total hourly arrival flow 
				LinkMOESummaryFile << pLink->m_MaximumServiceFlowRatePHPL << "," ;
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

				LinkMOESummaryFile << error_percentage << "," ;
				LinkMOESummaryFile << fabs(error_percentage) << "," ;

				LinkMOESummaryFile << pLink->m_AADT << "," ;
				LinkMOESummaryFile << pLink->m_NumberOfCrashes << "," ;

				LinkMOESummaryFile << endl;	

		}

		LinkMOESummaryFile.close ();
		
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
		// output statistics
		if(bStartWithEmpty)
		{
			fprintf(st, "vehicle_id,from_zone_id,to_zone_id,departure_time,arrival_time,complete_flag,trip_time,demand_type,pricing_type,vehicle_type,information_type,value_of_time,toll_cost_in_dollar,emissions,distance_in_mile,number_of_nodes,path_sequence\n");
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
				fprintf(st,"%d,%d,%d,%4.2f,%4.2f,%d,%4.2f,%d,%d,%d,%d,%4.2f,%4.2f,%4.2f,%4.2f,%d,",
					pVehicle->m_VehicleID , pVehicle->m_OriginZoneID , pVehicle->m_DestinationZoneID,
					pVehicle->m_DepartureTime, pVehicle->m_ArrivalTime , pVehicle->m_bComplete, TripTime,			
					pVehicle->m_DemandType, pVehicle->m_PricingType+1 ,pVehicle->m_VehicleType,pVehicle->m_InformationClass, pVehicle->m_VOT , pVehicle->m_TollDollarCost, pVehicle->m_Emissions ,pVehicle->m_Distance, pVehicle->m_NodeSize);

				fprintf(st, "\"");

				int j = 0;
				if(g_LinkVector[pVehicle->m_aryVN [0].LinkID]==NULL)
				{

					cout << "Error: vehicle" << pVehicle->m_VehicleID << "at LinkID"<< pVehicle->m_aryVN [0].LinkID << endl;
					cin.get();  // pause

				}

				int NodeID = g_LinkVector[pVehicle->m_aryVN [0].LinkID]->m_FromNodeID;  // first node
				int NodeName = g_NodeVector[NodeID].m_NodeName ;
				int link_entering_time = int(pVehicle->m_DepartureTime);
				fprintf(st, "<%d;%4.2f;0;0>",
					NodeName,pVehicle->m_DepartureTime) ;

				float LinkWaitingTime = 0;
				for(j = 0; j< pVehicle->m_NodeSize-1; j++)
				{
					int LinkID = pVehicle->m_aryVN [j].LinkID;
					int NodeID = g_LinkVector[LinkID]->m_ToNodeID;
					int NodeName = g_NodeVector[NodeID].m_NodeName ;
					float LinkTravelTime = 0;
					float Emissions = 0;

					if(j>0)
					{
						link_entering_time = int(pVehicle->m_aryVN [j-1].AbsArrivalTimeOnDSN);
						LinkTravelTime = (pVehicle->m_aryVN [j].AbsArrivalTimeOnDSN) - (pVehicle->m_aryVN [j-1].AbsArrivalTimeOnDSN);
						//						LinkWaitingTime = pVehicle->m_aryVN [j].AbsArrivalTimeOnDSN - pVehicle->m_aryVN [j-1].AbsArrivalTimeOnDSN - g_LinkVector[LinkID]->m_FreeFlowTravelTime ;
						//						if(LinkWaitingTime <0)
						//							LinkWaitingTime = 0;

					}

					//						fprintf(st, ",,,,,,,,,,,,,,%d,%d%,%6.2f,%6.2f,%6.2f\n", j+2,NodeName,pVehicle->m_aryVN [j].AbsArrivalTimeOnDSN,LinkWaitingTime, g_LinkVector[LinkID]->m_LinkMOEAry [link_entering_time].TravelTime ) ;
					fprintf(st, "<%d; %4.2f;%4.2f;%4.2f>",NodeName,pVehicle->m_aryVN [j].AbsArrivalTimeOnDSN, LinkTravelTime,Emissions) ;
				}

				fprintf(st,"\"\n");

			}else
			{
				g_WarningFile << "Warning: Vehicle " <<  pVehicle->m_VehicleID << " from node " << g_NodeVector[pVehicle ->m_DestinationZoneID].m_NodeName << " to node "  << g_NodeVector[pVehicle ->m_OriginNodeID].m_NodeName << " does not have a physical path. Count:" << VehicleCount_withPhysicalPath  << endl;		
				VehicleCount_withPhysicalPath++;
			}

			// not loaded in simulation
			if(pVehicle->m_bLoaded == false) 
			{
				g_WarningFile << "Warning: Not loaded vehicle " << pVehicle->m_VehicleID << " from zone " << 
					pVehicle->m_OriginZoneID << " to zone " << pVehicle->m_DestinationZoneID << " departing at"
					<< pVehicle->m_DepartureTime << " demand type = " << (int)(pVehicle->m_DemandType) << " Node Size in path = " <<  pVehicle->m_NodeSize << endl;
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

	g_AssignmentLogFile.open ("output_assignment_log.csv", ios::out);
	if (g_AssignmentLogFile.is_open())
	{
		g_AssignmentLogFile.width(12);
		g_AssignmentLogFile.precision(3) ;
		g_AssignmentLogFile.setf(ios::fixed);
		g_AssignmentLogFile << "CPU_time,iteration_no,avg_travel_time_in_min,travel_time_index,avg_travel_distance_in_mile,vehicle_route_switching_rate,number_of_vehicles_completing_trips,perc_of_vehicles_completing_trips,avg_travel_time_gap_per_vehicle_in_min,target_demand_deviation,abs_estimation_error_of_link_volume,RMSE_of_est_link_volume,avg_abs_perc_error_of_est_link_volume " << endl;
	}
	else
	{

		cout << "File assignment_log.csv cannot be opened, and it might be locked by another program!" << endl;
		cin.get();  // pause
		return 0;
	}

	g_EstimationLogFile.open ("estimation_log.csv", ios::out);
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

	ShortSimulationLogFile.open ("short_summary.log", ios::out);
	if (ShortSimulationLogFile.is_open())
	{
		ShortSimulationLogFile.width(12);
		ShortSimulationLogFile.precision(3) ;
		ShortSimulationLogFile.setf(ios::fixed);
	}else
	{
		cout << "File short_summary.csv cannot be opened, and it might be locked by another program!" << endl;
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
	cout << "Version 0.99, Release Date 03/08/2012."<< endl;

	g_LogFile << "---DTALite: A Fast Open-Source DTA Simulation Engine---"<< endl;
	g_LogFile << "Version 0.98, Release Date 03/08/2012."<< endl;

	fopen_s(&g_ErrorFile,"error.log","w");
	if(g_ErrorFile==NULL)
	{
		cout << "CANNOT OPEN file error.log!" << endl;
		cin.get();  // pause
		return 0;
	}

	//		g_RunStaticExcel();
	return 1;
}

void g_ReadDTALiteSettings()
{
	TCHAR IniFilePath_DTA[_MAX_PATH] = _T("./DTASettings.ini");

	// if  ./DTASettings.ini does not exist, then we should print out all the default settings for user to change
	//

	g_TrafficFlowModelFlag = g_GetPrivateProfileInt("simulation", "traffic_flow_model", 1, IniFilePath_DTA);	
	g_EmissionDataOutputFlag = g_GetPrivateProfileInt("emission", "emission_data_output", 0, IniFilePath_DTA);	
	//		g_TollingMethodFlag = g_GetPrivateProfileInt("tolling", "method_flag", 0, IniFilePath_DTA);	
	//		g_VMTTollingRate = g_GetPrivateProfileFloat("tolling", "VMTRate", 0, IniFilePath_DTA);

	g_ODEstimationFlag = g_GetPrivateProfileInt("estimation", "od_demand_estimation", 0, IniFilePath_DTA);	
	g_ODEstimationMeasurementType = g_GetPrivateProfileInt("estimation", "measurement_type", 1, IniFilePath_DTA);	
	g_ODEstimation_WeightOnHistODDemand = g_GetPrivateProfileFloat("estimation", "weight_on_hist_oddemand", 1, IniFilePath_DTA);
	g_ODEstimation_WeightOnUEGap = g_GetPrivateProfileFloat("estimation", "weight_on_ue_gap", 1, IniFilePath_DTA);


	g_ODEstimation_StartingIteration = g_GetPrivateProfileInt("estimation", "starting_iteration", 10, IniFilePath_DTA);
	g_ObservationTimeInterval = g_GetPrivateProfileInt("estimation", "observation_time_interval", 5, IniFilePath_DTA);
	g_ObservationStartTime = g_GetPrivateProfileInt("estimation", "observation_start_time_in_min", 390, IniFilePath_DTA);
	g_ObservationEndTime = g_GetPrivateProfileInt("estimation", "observation_end_time_in_min", 570, IniFilePath_DTA);

	if(g_ObservationEndTime >= g_ObservationStartTime + g_PlanningHorizon)  // no later than the simulation end time
	g_ObservationEndTime = g_ObservationStartTime + g_PlanningHorizon;

	g_VehicleLoadingMode = g_GetPrivateProfileInt("demand", "load_vehicle_file_mode", 0, IniFilePath_DTA);	
	g_ParallelComputingMode = g_GetPrivateProfileInt("assignment", "parallel_computing", 1, IniFilePath_DTA);


	if(g_TrafficFlowModelFlag ==0)  //BRP  // static assignment parameters
	{
		g_AggregationTimetInterval = 60;  // one hour
		g_PlanningHorizon = 60; // one hour
		g_NumberOfInnerIterations = 0;
		g_NumberOfIterations = g_GetPrivateProfileInt("assignment", "number_of_iterations", 10, IniFilePath_DTA);	

		g_AgentBasedAssignmentFlag = g_GetPrivateProfileInt("assignment", "agent_based_assignment", 1, IniFilePath_DTA);
		g_DemandGlobalMultiplier = g_GetPrivateProfileFloat("demand", "global_multiplier",1.0,IniFilePath_DTA);	
	}
	else  //DTA parameters
	{

		g_PlanningHorizon = g_GetPrivateProfileInt("simulation", "simulation_horizon_in_min", 600, IniFilePath_DTA);	
		// research code:		g_StochasticCapacityMode = g_GetPrivateProfileInt("simulation", "stochatic_capacity_mode", 0, IniFilePath_DTA);

		g_MergeNodeModelFlag = g_GetPrivateProfileInt("simulation", "merge_node_model", 1, IniFilePath_DTA);	
		g_MinimumInFlowRatio = g_GetPrivateProfileFloat("simulation", "minimum_link_in_flow_ratio", 0.02f, IniFilePath_DTA);
		g_MaxDensityRatioForVehicleLoading  = g_GetPrivateProfileFloat("simulation", "max_density_ratio_for_loading_vehicles", 0.8f, IniFilePath_DTA);
		g_CycleLength_in_seconds = g_GetPrivateProfileFloat("simulation", "cycle_length_in_seconds", 120, IniFilePath_DTA);
		g_DefaultSaturationFlowRate_in_vehphpl = g_GetPrivateProfileFloat("simulation", "default_saturation_flow_rate_in_vehphpl", 1800, IniFilePath_DTA);


		g_NumberOfIterations = g_GetPrivateProfileInt("assignment", "number_of_iterations", 10, IniFilePath_DTA);	
		g_AgentBasedAssignmentFlag = g_GetPrivateProfileInt("assignment", "agent_based_assignment", 1, IniFilePath_DTA);
		g_AggregationTimetInterval = g_GetPrivateProfileInt("assignment", "aggregation_time_interval_in_min", 15, IniFilePath_DTA);	
		g_NumberOfInnerIterations = g_GetPrivateProfileInt("assignment", "number_of_inner_iterations", 0, IniFilePath_DTA);	
		g_ConvergencyRelativeGapThreshold_in_perc = g_GetPrivateProfileFloat("assignment", "convergency_relative_gap_threshold_percentage", 5, IniFilePath_DTA);	

		g_UEAssignmentMethod = g_GetPrivateProfileInt("assignment", "UE_assignment_method", 0, IniFilePath_DTA); // default is MSA

		// parameters for day-to-day learning mode
		g_LearningPercentage = g_GetPrivateProfileInt("assignment", "learning_percentage", 15, IniFilePath_DTA);
		g_TravelTimeDifferenceForSwitching = g_GetPrivateProfileInt("assignment", "travel_time_difference_for_switching_in_min", 5, IniFilePath_DTA);
		g_DemandGlobalMultiplier = g_GetPrivateProfileFloat("demand", "global_multiplier",1.0,IniFilePath_DTA);	
		g_start_iteration_for_MOEoutput = g_GetPrivateProfileInt("output", "start_iteration_for_MOE", -1, IniFilePath_DTA);	

		g_RandomSeed = g_GetPrivateProfileInt("simulation", "random_number_seed", 100, IniFilePath_DTA);

		g_UserClassPerceptionErrorRatio[1] = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_historical_info_travelers_perception_error",0.3f,IniFilePath_DTA);	
		g_UserClassPerceptionErrorRatio[2] = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_pretrip_info_travelers_perception_error",0.05f,IniFilePath_DTA);	
		g_UserClassPerceptionErrorRatio[3] = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_en-route_info_travelers_perception_error",0.05f,IniFilePath_DTA);	

		g_VMSPerceptionErrorRatio          = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_VMS_perception_error",0.05f,IniFilePath_DTA);	
		g_information_updating_interval_of_en_route_info_travelers_in_min= g_GetPrivateProfileInt("traveler_information", "information_updating_interval_of_en_route_info_travelers_in_min",5,IniFilePath_DTA);	


	}

	if(g_AgentBasedAssignmentFlag == 2) 
		g_PlanningHorizon = 1;

	srand(g_RandomSeed);
	g_LogFile << "Simulation Horizon (min) = " << g_PlanningHorizon << endl;
	g_LogFile << "Departure Time Interval (min) = " << g_AggregationTimetInterval << endl;
	g_LogFile << "Number of Iterations = "<< g_NumberOfIterations << endl;


	if(g_VehicleLoadingMode == 1)
	{
		g_LogFile << "Load vehicles from the trajectory file input_vehicle.csv" << endl;
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

void g_OutputSimulationStatistics(int NumberOfLinks = 9999999)
{
	g_OutputLinkMOESummary("output_LinkMOE_summary.csv");  // output assignment results anyway
	int Count=0; 
	float AvgTravelTime, AvgDistance, AvgSpeed;
	g_LogFile << "--- MOE for vehicles completing trips ---" << endl;

	Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, -1, -1);
	g_LogFile << " # of Vehicles = " << Count << " AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

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

	for(int departure_time = g_DemandLoadingStartTimeInMin; departure_time < g_DemandLoadingEndTimeInMin; departure_time += g_AggregationTimetInterval)
	{
		Count = g_OutputSimulationSummary(AvgTravelTime, AvgDistance, AvgSpeed, -1, -1, departure_time);
		if(Count>0)
			g_LogFile << "Time:" << departure_time << " min, # of Vehicles = " << Count << " ,AvgTravelTime = " << AvgTravelTime << " (min), AvgDistance = " << AvgDistance << " (miles), AvgSpeed =  " << AvgSpeed << " (mph)" <<  endl;

	}
	g_LogFile << endl;

	// output general link statistics
	std::set<DTALink*>::iterator iterLink;
	g_LogFile << "--- Link MOE ---" << endl;

	for(unsigned li = 0; li< min(NumberOfLinks,g_LinkVector.size()); li++)
	{
		float Capacity = g_LinkVector[li]->m_MaximumServiceFlowRatePHPL * g_LinkVector[li]->m_NumLanes;
		g_LogFile << "Link: " <<  g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName  << " -> " << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName << 
			", Link Capacity: " << Capacity <<
			", Inflow: " << g_LinkVector[li]->CFlowArrivalCount <<
			//				" Outflow: " << g_LinkVector[li]->CFlowDepartureCount <<
			", VOC Ratio: " << g_LinkVector[li]->CFlowDepartureCount /max(1,Capacity)  << endl;
	}

	g_LogFile << "---End of Link MOE ---" << endl;

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

	ReadInputFiles();

	cout << "Start Traffic Assignment/Simulation... " << endl;

	if(g_TrafficFlowModelFlag ==0)
	{
		if(g_AgentBasedAssignmentFlag==0)
		{
			cout << "Static Traffic Assignment.. " << endl;
			g_ODBasedDynamicTrafficAssignment(); // multi-iteration dynamic traffic assignment
		}
		else
			g_AgentBasedAssisnment();  // agent-based assignment

	}
	else 
	{ // dynamic traffic assignment
		if(g_AgentBasedAssignmentFlag==0)
		{
			cout << "OD Demand based dynamic traffic assignment... " << endl;
			g_ODBasedDynamicTrafficAssignment(); // multi-iteration dynamic traffic assignment
		}
		else{
			cout << "Agent based dynamic traffic assignment... " << endl;
			g_AgentBasedAssisnment();  // agent-based assignment
		}
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


	/**********************************************/
	//below is the main traffic assignment-simulation code


	g_ReadDTALiteSettings();
	g_TrafficAssignmentSimulation();
	g_OutputSimulationStatistics();
	g_FreeMemory();

	if(ShortSimulationLogFile.is_open())
	{
		ShortSimulationLogFile.close();
	}

	return nRetCode;
}

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
			if( g_LinkTypeFreewayMap[g_LinkVector[li]->m_link_type] == 1  &&  m_OutboundSizeAry[g_LinkVector[li]->m_ToNodeID] ==1)  // freeway or highway
			{
				int FromID = g_LinkVector[li]->m_FromNodeID;
				int ToID   = g_LinkVector[li]->m_ToNodeID;

				for(int i=0; i< m_OutboundSizeAry[ToID]; i++)
				{
					DTALink* pNextLink =  g_LinkVector[m_OutboundLinkAry[ToID][i]];
					if(g_LinkTypeFreewayMap[pNextLink->m_link_type ]==1 && pNextLink->m_NumLanes < g_LinkVector[li]->m_NumLanes && pNextLink->m_ToNodeID != FromID)
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
			if( (g_LinkTypeFreewayMap[g_LinkVector[li]->m_link_type ]==1 || g_LinkVector[li]->m_link_type ==9)
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
					if(g_LinkTypeRampMap[g_LinkVector[m_InboundLinkAry[FromID][il]]->m_link_type] == 1)  // on ramp as incoming link
					{
						bRampExistFlag = true;
						g_LinkVector[li]->m_MergeOnrampLinkID = m_InboundLinkAry[FromID][il];
					}
					if(g_LinkTypeFreewayMap[g_LinkVector[m_InboundLinkAry[FromID][il]]->m_link_type ]==1)  // freeway or highway
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




void DTASafetyPredictionModel::UpdateAADTConversionFactor()
{
	for(int t = g_DemandLoadingStartTimeInMin/15; t< g_DemandLoadingEndTimeInMin/15; t++)
	{
		m_AADTConversionFactorForStudyHorizion += time_dependent_flow_ratio [t];
	}

	if( m_AADTConversionFactorForStudyHorizion < 0.001)
		m_AADTConversionFactorForStudyHorizion = 0.001;

}

void DTASafetyPredictionModel::UpdateCrashRateForAllLinks()
{
	UpdateAADTConversionFactor();  // for dynamic traffic assignment only
	std::set<DTALink*>::iterator iterLink;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		DTALink* pLink = g_LinkVector[li];

		if(g_TrafficFlowModelFlag == 0) // static traffic assignment 
		{
		pLink->m_AADT = pLink->CFlowArrivalCount/0.15;
		}else  //dynamic traffic assignment 
		{
		pLink->m_AADT = pLink->CFlowArrivalCount/m_AADTConversionFactorForStudyHorizion;
		}

		double CrashRate= 0;
		if( g_LinkTypeFreewayMap[pLink->m_link_type] == 1)  // freeway
		{
			pLink->m_NumberOfCrashes  = EstimateFreewayCrashRatePerYear(pLink->m_AADT , pLink->m_Length );
		}

		if( g_LinkTypeArterialMap[pLink->m_link_type] == 1)   //arterial
		{
			pLink->m_NumberOfCrashes  = EstimateArterialCrashRatePerYear(pLink->m_AADT, pLink->m_Length );

		}

	}

}