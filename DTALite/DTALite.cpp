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
#include <stdlib.h>
#include <crtdbg.h>

#include "DTALite.h"
#include "Geometry.h"
#include "GlobalData.h"
#include "CSVParser.h"
#include "SafetyPlanning.h"
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
TCHAR g_DTASettingFileName[_MAX_PATH] = _T("./DTASettings.txt");

NetworkSimulationResult g_SimulationResult;
std::vector<DTANode> g_NodeVector;
std::map<int, int> g_NodeNametoIDMap;


std::vector<DTALink*> g_LinkVector;

std::map<int, DTAZone> g_ZoneMap;
std::vector<DTAVehicleType> g_VehicleTypeVector;

std::vector<DTAVehicle*>		g_VehicleVector;

std::map<int, DTAVehicle*> g_VehicleMap;
std::map<int, DemandType> g_DemandTypeMap;
std::map<int, PricingType> g_PricingTypeMap;
std::map<int, DTALinkType> g_LinkTypeMap;

// time inteval settings in assignment and simulation
double g_DTASimulationInterval = 0.10000; // min
int g_number_of_intervals_per_min = 10; // round to nearest integer
int g_AggregationTimetInterval = 15; // min
int g_AggregationTimetIntervalSize = 2;
float g_DemandGlobalMultiplier = 1.0f;


// maximal # of adjacent links of a node (including physical nodes and centriods( with connectors))
int g_AdjLinkSize = 30; // initial value of adjacent links

int g_ODZoneSize = 0;

int g_StartIterationsForOutputPath = 2;
int g_EndIterationsForOutputPath = 2;

// assignment and simulation settings
int g_NumberOfIterations = 1;
int g_ParallelComputingMode = 1;
int g_AgentBasedAssignmentFlag = 0;
bool g_SimulationOutput2UnitTesting = false;
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
int g_information_updating_interval_of_VMS_in_min = 60;


int g_LearningPercentage = 15;
int g_TravelTimeDifferenceForSwitching = 5;  // min

int g_StochasticCapacityMode = 0;
float g_MinimumInFlowRatio = 0.1f;
float g_MaxDensityRatioForVehicleLoading = 0.8f;
int g_CycleLength_in_seconds;
float g_DefaultSaturationFlowRate_in_vehphpl;


std::vector<VOTDistribution> g_VOTDistributionVector;
std::vector<TimeDependentDemandProfile> g_TimeDependentDemandProfileVector;
int g_DemandLoadingStartTimeInMin = 0;

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

VehicleArrayForOriginDepartrureTimeInterval** g_TDOVehicleArray =NULL; // TDO for time-dependent origin

std::vector<DTA_vhc_simple>   g_simple_vector_vehicles;	// vector of DSP_Vehicle, not pointer!;

FILE* g_ErrorFile = NULL;
ofstream g_LogFile;
ofstream g_AssignmentLogFile;
ofstream g_EstimationLogFile;
ofstream g_WarningFile;
ofstream g_UnitTestingLogFile;

int g_TrafficFlowModelFlag = 0;
int g_EmissionDataOutputFlag = 0;


int g_TollingMethodFlag = 0;
float g_VMTTollingRate = 0;

int g_MergeNodeModelFlag=1;

std::vector<NetworkMOE>  g_NetworkMOEAry;
std::vector<NetworkLoadingOutput>  g_AssignmentMOEVector;

CTime g_AppStartTime;

unsigned int g_RandomSeed = 100;
unsigned int g_RandomSeedForVehicleGeneration = 101;
// Linear congruential generator 
#define g_LCG_a 17364
#define g_LCG_c 0
#define g_LCG_M 65521  


using namespace std;

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
	pPhysicalNetwork->BuildPhysicalNetwork(0);

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

void g_CreateLinkTollVector()
{

	// transfer toll set to dynamic toll vector (needed for multi-core computation)
	std::set<DTALink*>::iterator iterLink;
	int count = 0;
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		if(g_LinkVector[li]->pTollVector!=NULL)
			delete g_LinkVector[li]->pTollVector;

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
}
void ReadScenarioInputFiles(DTANetworkForSP* pPhysicalNetwork)
{
	//******************************* scenario input //
	//

	FILE* st = NULL;

	fopen_s(&st,"Scenario_Incident.csv","r"); /// 
	if(st!=NULL)
	{
		cout << "Reading file Scenario_Incident.csv..."<< endl;
		g_LogFile << "Reading file Scenario_Incident.csv, "<< endl;
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
				cs.StartDayNo = g_read_integer(st);
				cs.EndDayNo = cs.StartDayNo;
				cs.StartTime = g_read_integer(st);
				cs.EndTime = g_read_integer(st);
				cs.LaneClosureRatio= g_read_float(st)/100.0f; // percentage -> to ratio
				if(cs.LaneClosureRatio > 1.0)
					cs.LaneClosureRatio = 1.0;
				if(cs.LaneClosureRatio < 0.0)
					cs.LaneClosureRatio = 0.0;
				cs.SpeedLimit = g_read_float(st);

				pLink->CapacityReductionVector.push_back(cs);
				count++;

			}
		}

		g_scenario_short_description << "with " << count << "incident records;";
		g_LogFile << "incident records = " << count << endl;

		fclose(st);
	}

	fopen_s(&st,"Scenario_Work_Zone.csv","r"); /// 
	if(st!=NULL)
	{
		cout << "Reading file Scenario_Work_Zone.csv..."<< endl;
		g_LogFile << "Reading file Scenario_Work_Zone.csv, "<< endl;
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
				cs.StartDayNo = g_read_integer(st);
				cs.EndDayNo = g_read_integer(st);
				cs.StartTime = g_read_integer(st);
				cs.EndTime = g_read_integer(st);
				cs.LaneClosureRatio= g_read_float(st)/100.0f; // percentage -> to ratio
				if(cs.LaneClosureRatio > 1.0)
					cs.LaneClosureRatio = 1.0;
				if(cs.LaneClosureRatio < 0.0)
					cs.LaneClosureRatio = 0.0;
				cs.SpeedLimit = g_read_float(st);

				pLink->CapacityReductionVector.push_back(cs);
				count++;

			}
		}

		g_scenario_short_description << "with " << count << "incident records;";
		g_LogFile << "work zone records = " << count << endl;

		fclose(st);
	}else
	{
		cout << "Work zone records = 0 " << endl;

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
				is.StartDayNo  = g_read_integer(st);
				is.EndDayNo   = g_read_integer(st);
				is.StartTime = g_read_integer(st);
				is.EndTime = g_read_integer(st);
				is.ResponsePercentage =  g_read_float(st);

				pLink->MessageSignVector.push_back(is);
				count++;
			}
		}

		g_scenario_short_description << "with " << count << "VMS records;";
		g_LogFile << "VMS records = " << count << endl;
		cout << "VMS records = " << count << endl;
		fclose(st);
	}else
	{
		cout << "VMS records = 0"<< endl;
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
				Toll tc;  // toll collection
				tc.DayNo = g_read_integer(st);

				tc.StartTime = g_read_integer(st);
				tc.EndTime = g_read_integer(st);

				for(int vt = 1; vt< MAX_PRICING_TYPE_SIZE; vt++)  // last one is transit fare
				{
					tc.TollRate [vt]= g_read_float(st); 
				}

				count++;
				pLink->TollVector.push_back(tc);

			}
		}

		cout << "Number of link-based toll records =  " << count << endl;

		fclose(st);

	}else
	{
		cout << "Number of link-based toll records = 0"<< endl;
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
	}else
	{
		cout << "Number of distance-based toll records = 0 "<< endl;
	}

	g_CreateLinkTollVector();
}
void g_ReadInputFiles()
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

	if (!parser_link_type.OpenCSVFile("input_link_type.csv"))
	{
		cout << "Error: File input_link_type.csv cannot be opened.\n Try to use default value."<< endl;
		ofstream link_type_file;
		link_type_file.open("input_link_type.csv");
		if(link_type_file.is_open ())
		{
			link_type_file<< "link_type,link_type_name,type_code" << endl;
			link_type_file<< "1,Freeway,f" << endl;
			link_type_file<< "2,Highway/Expressway,h" << endl;
			link_type_file<< "3,Principal arterial,a" << endl;
			link_type_file<< "4,Major arterial,a" << endl;
			link_type_file<< "5,Minor arterial,a" << endl;
			link_type_file<< "6,Collector,a" << endl;
			link_type_file<< "7,Local,a" << endl;
			link_type_file<< "8,Frontage road,a" << endl;
			link_type_file<< "9,Ramp,r" << endl;
			link_type_file<< "10,Zonal Connector,c" << endl;
			link_type_file<< "100,transit,t" << endl;
			link_type_file<< "200,walking,w" << endl;
			link_type_file.close();
		}

	}
	if (parser_link_type.inFile.is_open () || parser_link_type.OpenCSVFile("input_link_type.csv"))
	{
		while(parser_link_type.ReadRecord())
		{
			DTALinkType element;

			if(parser_link_type.GetValueByFieldName("link_type",element.link_type ) == false)
			{
				cout << "Error: Field link_type cannot be found in file input_link_type.csv."<< endl;
				g_ProgramStop();
			}


			if(parser_link_type.GetValueByFieldName("link_type_name",element.link_type_name ) == false)
			{
				cout << "Error: Field link_type_name cannot be found in file input_link_type.csv."<< endl;
				g_ProgramStop();
			}
			if(parser_link_type.GetValueByFieldName("type_code",element.type_code  ) == false)
			{
				cout << "Error: Field type_code cannot be found in file input_link_type.csv."<< endl;
				g_ProgramStop();
			}

			g_LinkTypeMap[element.link_type  ] = element;

		}
	}else
	{
		cout << "Error: File input_link_type.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;


	}


	//*******************************
	// step 3: link data input

	char InputLinkFileName[_MAX_PATH];

	GetPrivateProfileString("input_file","link_data","input_link.csv",InputLinkFileName,_MAX_PATH,g_DTASettingFileName);
	int AllowExtremelyLowCapacityFlag = g_GetPrivateProfileInt("input_checking", "allow_extremely_low_capacity", 1, g_DTASettingFileName);	


	cout << "Step 3: Reading file input_link.csv..."<< endl;
	g_LogFile << "Step 3: Reading file input_link.csv..." << endl;

	int i = 0;

	DTALink* pLink = 0;
	CCSVParser parser_link;
	if (parser_link.OpenCSVFile(InputLinkFileName))
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
			double K_jam,wave_speed_in_mph,AADT_conversion_factor;

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

			if(!parser_link.GetValueByFieldName("AADT_conversion_factor",AADT_conversion_factor))
				AADT_conversion_factor = 0.1;

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
				if(pLink==NULL)
				{
					cout << "Allocating memory error at line "<< i+1  << endl;
					getchar();
					exit(0);

				}
				pLink->m_LinkID = i;
				pLink->m_FromNodeNumber = FromID;
				pLink->m_ToNodeNumber = ToID;
				pLink->m_FromNodeID = g_NodeNametoIDMap[pLink->m_FromNodeNumber ];
				pLink->m_ToNodeID= g_NodeNametoIDMap[pLink->m_ToNodeNumber];

/* common out for now to speed up the process
				std::vector<CCoordinate> CoordinateVector;
				string geo_string;
				if(parser_link.GetValueByFieldName("geometry",geo_string))
				{
					// overwrite when the field "geometry" exists
					CGeometry geometry(geo_string);
					CoordinateVector = geometry.GetCoordinateList();
					for(int si = 0; si < CoordinateVector.size(); si++)
					{
						GDPoint	pt;
						pt.x = CoordinateVector[si].X;
						pt.y = CoordinateVector[si].Y;
						pLink->m_ShapePoints .push_back (pt);
					}
				}
*/
				// read safety-related data

				parser_link.GetValueByFieldName("num_driveways_per_mile",pLink->m_Num_Driveways_Per_Mile);
				parser_link.GetValueByFieldName("volume_proportion_on_minor_leg",pLink->m_volume_proportion_on_minor_leg);
				parser_link.GetValueByFieldName("num_3SG_intersections",pLink->m_Num_3SG_Intersections);				parser_link.GetValueByFieldName("num_driveways_per_mile",pLink->m_Num_3ST_Intersections);
				parser_link.GetValueByFieldName("num_4SG_intersections",pLink->m_Num_4SG_Intersections);
				parser_link.GetValueByFieldName("num_4ST_intersections",pLink->m_Num_4ST_Intersections);

				pLink->m_SpeedLimit= speed_limit_in_mph;

				float length = max(0.00001,length_in_mile);  // minimum link length 0.0001
				pLink->m_NumLanes= number_of_lanes;

				if(g_AgentBasedAssignmentFlag != 2)
					pLink->m_Length= max(length, pLink->m_SpeedLimit*0.1f/60.0f);  // we do not impose the minimum distance in this version
				else
					pLink->m_Length= length;

				pLink->m_MaximumServiceFlowRatePHPL= capacity;

				if(AllowExtremelyLowCapacityFlag == 0 && capacity < 10)
				{
					cout << "In file input_link.csv, line "<< i+1 << " has capacity <10" << capacity <<", which might not be realistic. Please correct the error." << endl;
					getchar();
				}

				pLink->m_BPRLaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
				pLink->m_link_type= type;

				if(g_LinkTypeMap.find(type) == g_LinkTypeMap.end())
				{
					cout << "In file input_link.csv, line "<< i+1 << " has link type "<< type <<", which has not been defined in input_link_type.csv. Please correct the error" << endl;
					getchar();
					exit(0);			
				}


				pLink->m_KJam = K_jam;
				pLink->m_AADTConversionFactor = AADT_conversion_factor;
				pLink->m_BackwardWaveSpeed = wave_speed_in_mph;

				pLink->m_VehicleSpaceCapacity = pLink->m_Length * pLink->m_NumLanes *K_jam; // bump to bump density


				g_NodeVector[pLink->m_FromNodeID ].m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);
				g_NodeVector[pLink->m_ToNodeID ].m_IncomingLinkVector.push_back(i);


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




	CCSVParser parser_zone;

	if (!parser_zone.OpenCSVFile("input_zone.csv"))
	{
		ofstream zone_file;
		zone_file.open("input_zone.csv");
		if(zone_file.is_open ())
		{
			zone_file << "zone_id,geometry" << endl;

			// use each node as possible zone
			for(unsigned int i=0; i< g_NodeVector.size(); i++)
			{  
				zone_file << g_NodeVector[i].m_NodeName << "," << endl;
			}

			zone_file.close();
		}


	}
	if (parser_zone.inFile .is_open () || parser_zone.OpenCSVFile("input_zone.csv"))
	{
		int i=0;

		while(parser_zone.ReadRecord())
		{
			int zone_number;

			if(parser_zone.GetValueByFieldName("zone_id",zone_number) == false)
				break;

			DTAZone zone;
			g_ZoneMap[zone_number] = zone;

		}
	}else
	{
		cout << "input_zone.csv cannot be opened."<< endl;
		g_ProgramStop();
	}

	// check how many centroids

	cout << "Step 5: Reading files input_activity_location.csv..."<< endl;
	g_LogFile << "Step 5: Reading file input_activity_location.csv..." << endl;

	CCSVParser parser_activity_location;

	if (!parser_activity_location.OpenCSVFile("input_activity_location.csv"))
	{

		ofstream activity_location_file;
		activity_location_file.open("input_activity_location.csv");
		if(activity_location_file.is_open ())
		{
			activity_location_file << "zone_id,node_id,external_OD_flag" << endl;

			// use each node as possible zone
			for(unsigned int i=0; i< g_NodeVector.size(); i++)
			{  
				activity_location_file << g_NodeVector[i].m_NodeName << "," << g_NodeVector[i].m_NodeName << ",0"<< endl;
			}

			activity_location_file.close();
		}
	}

	if (parser_activity_location.inFile .is_open () || parser_activity_location.OpenCSVFile("input_activity_location.csv"))
	{
		int i=0;
		while(parser_activity_location.ReadRecord())
		{
			int zone_number;

			if(parser_activity_location.GetValueByFieldName("zone_id",zone_number) == false)
				break;

			int node_number;
			if(parser_activity_location.GetValueByFieldName("node_id",node_number) == false)
				break;

			int nodeid = g_NodeNametoIDMap[node_number];
			if(g_ODZoneSize < zone_number)
				g_ODZoneSize = zone_number;

			int external_od_flag;
			if(parser_activity_location.GetValueByFieldName("external_OD_flag",external_od_flag) == false)
				external_od_flag  =0 ;

			g_NodeVector[g_NodeNametoIDMap[node_number]].m_ZoneID = zone_number;

			if(external_od_flag != -1) // not external destination
				g_ZoneMap[zone_number].m_OriginActivityVector.push_back(nodeid);

			if(external_od_flag != 1) // not external origin
				g_ZoneMap[zone_number].m_DestinationActivityVector.push_back(nodeid);

			g_ZoneMap[zone_number].m_Capacity += g_NodeVector[nodeid].m_TotalCapacity ;

			if(g_ZoneMap[zone_number].m_OriginActivityVector.size() > g_AdjLinkSize)
				g_AdjLinkSize = g_ZoneMap[zone_number].m_OriginActivityVector.size();
		}

	}else
	{
		cout << "input_activity_location.csv cannot be opened."<< endl;
		g_ProgramStop();
	}

	for (std::map<int, DTAZone>::iterator iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
	{
		if(iterZone->second.m_OriginActivityVector.size() > 30)
		{
			g_LogFile << "Zone " << iterZone->first << ": # of connectors:" <<  iterZone->second.m_OriginActivityVector.size()<< endl;
		}
	}

	//*******************************
	// step 5: vehicle type input

	cout << "Step 6: Reading file input_vehicle_type.csv..."<< endl;
	g_LogFile << "Step 6: Reading file input_vehicle_type.csv..."<< endl;

	CCSVParser parser_vehicle_type;

	if (!parser_vehicle_type.OpenCSVFile("input_vehicle_type.csv"))
	{
		cout << "input_vehicle_type.csv cannot be opened.  Use default values. "<< endl;

		ofstream VehicleTypeFile;
		VehicleTypeFile.open("input_vehicle_type.csv");
		if(VehicleTypeFile.is_open ())
		{
			VehicleTypeFile << "vehicle_type,vehicle_type_name" << endl;
			VehicleTypeFile << "vehicle_type,vehicle_type_name" << endl;
			VehicleTypeFile << "1,passenger car" << endl;
			VehicleTypeFile << "2,passenger truck" << endl;
			VehicleTypeFile << "3,light commercial truck" << endl;
			VehicleTypeFile << "4,single unit long-haul truck" << endl;
			VehicleTypeFile << "5,combination long-haul truck" << endl;
			VehicleTypeFile.close();
		}
	}

	if (parser_vehicle_type.inFile .is_open () || parser_vehicle_type.OpenCSVFile("input_vehicle_type.csv"))
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

	}else
	{
		cout << "input_vehicle_type.csv cannot be opened. "<< endl;
		g_ProgramStop();

	}

	//*******************************
	// step 6: demand type input

	CCSVParser parser_demand_type;
	cout << "Step 7.1: Reading file input_demand_type.csv..."<< endl;
	g_LogFile << "Step 7.1: Reading file input_demand_type.csv..."<< endl;
	if (!parser_demand_type.OpenCSVFile("input_demand_type.csv"))
	{
		ofstream demand_type_file;
		demand_type_file.open ("input_demand_type.csv");
		if(demand_type_file.is_open ())
		{
			demand_type_file << "demand_type,demand_type_name,average_VOT,pricing_type,percentage_of_pretrip_info,percentage_of_enroute_info,percentage_of_vehicle_type1,percentage_of_vehicle_type2,percentage_of_vehicle_type3,percentage_of_vehicle_type4,percentage_of_vehicle_type5" << endl;

			demand_type_file << "1,SOV,10.000,1,0.000,0.000,80.000,20.000,0.000,0.000,0.000," << endl;
			demand_type_file << "2,HOV,10.000,2,0.000,0.000,80.000,20.000,0.000,0.000,0.000," << endl;
			demand_type_file << "3,truck,20.000,3,0.000,0.000,0.000,0.000,30.000,30.000,40.000," << endl;
			demand_type_file << "4,intermodal,10.000,4,0.000,0.000,0.000,0.000,0.000,0.000,0.000," << endl;

			demand_type_file.close();
		}

	}

	if (parser_demand_type.inFile .is_open () || parser_demand_type.OpenCSVFile("input_demand_type.csv"))
	{
		while(parser_demand_type.ReadRecord())
		{
			int demand_type =1;
			int pricing_type = 0;
			float average_VOT = 10;

			if(parser_demand_type.GetValueByFieldName("demand_type",demand_type) == false)
				break;

			float ratio_pretrip = 0;
			float ratio_enroute = 0;

			if(parser_demand_type.GetValueByFieldName("pricing_type",pricing_type) == false)
				break;

			if(pricing_type < 0)
			{
				cout << "Error: Field pricing_type " << pricing_type << " should be >=1 in the input_demand_type.csv file.";
				g_ProgramStop();
				return;
			}
			parser_demand_type.GetValueByFieldName("percentage_of_pretrip_info",ratio_pretrip);
			parser_demand_type.GetValueByFieldName("percentage_of_enroute_info",ratio_enroute);

			DemandType element;
			element.demand_type = demand_type;

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

	CCSVParser parser_pricing_type;
	cout << "Step 7.2: Reading file input_pricing_type.csv..."<< endl;
	g_LogFile << "Step 7.2: Reading file input_pricing_type.csv..."<< endl;
	if (!parser_pricing_type.OpenCSVFile("input_pricing_type.csv"))
	{
		cout << "Error: File input_pricing_type.csv cannot be opened.\n Use default values."<< endl;
		ofstream PricingTypeFile;
		PricingTypeFile.open("input_pricing_type.csv");
		if(PricingTypeFile.is_open ())
		{
			PricingTypeFile << "pricing_type,default_VOT"<< endl;
			PricingTypeFile << "1,10"<< endl;
			PricingTypeFile << "2,20"<< endl;
			PricingTypeFile << "3,30"<< endl;
			PricingTypeFile << "4,10"<< endl;
			PricingTypeFile.close();
		}

	}

	if (parser_pricing_type.inFile .is_open () || parser_pricing_type.OpenCSVFile("input_pricing_type.csv"))
	{
		while(parser_pricing_type.ReadRecord())
		{
			int pricing_type = 0;
			float default_VOT = 10;

			if(parser_demand_type.GetValueByFieldName("pricing_type",pricing_type) == false)
				break;

			if(parser_demand_type.GetValueByFieldName("default_VOT",default_VOT) == false)
				break;

			if(pricing_type <= 0)
			{
				cout << "Error: Field pricing_type " << pricing_type << " should be >=1 in the input_demand_type.csv file.";
				g_ProgramStop();
				return;
			}

			PricingType element;
			element.pricing_type = pricing_type;
			element.default_VOT = default_VOT;
			g_PricingTypeMap[pricing_type] = element;

		}

	}else
	{
		cout << "Error: File input_pricing_type.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}


	//*******************************
	// step 7: time-dependent demand profile input

	cout << "Step 8: Reading file input_temporal_demand_profile.csv..."<< endl;
	g_LogFile << "Step 8: Reading file input_temporal_demand_profile.csv..."<< endl;

	CCSVParser parser_TDProfile;

	if (!parser_TDProfile.OpenCSVFile("input_temporal_demand_profile.csv"))
	{
		cout << "input_temporal_demand_profile.csv cannot be opened. Use default value"<< endl;
		ofstream temporal_demand_profile_file;
		temporal_demand_profile_file.open("input_temporal_demand_profile.csv");
		if(temporal_demand_profile_file.is_open ())
		{
			temporal_demand_profile_file << "from_zone_id,to_zone_id,demand_type,time_series_name,'00:00,'00:15,'00:30,'00:45,'01:00,'01:15,'01:30,'01:45,'02:00,'02:15,'02:30,'02:45,'03:00,'03:15,'03:30,'03:45,'04:00,'04:15,'04:30,'04:45,'05:00,'05:15,'05:30,'05:45,'06:00,'06:15,'06:30,'06:45,'07:00,'07:15,'07:30,'07:45,'08:00,'08:15,'08:30,'08:45,'09:00,'09:15,'09:30,'09:45,'10:00,'10:15,'10:30,'10:45,'11:00,'11:15,'11:30,'11:45,'12:00,'12:15,'12:30,'12:45,'13:00,'13:15,'13:30,'13:45,'14:00,'14:15,'14:30,'14:45,'15:00,'15:15,'15:30,'15:45,'16:00,'16:15,'16:30,'16:45,'17:00,'17:15,'17:30,'17:45,'18:00,'18:15,'18:30,'18:45,'19:00,'19:15,'19:30,'19:45,'20:00,'20:15,'20:30,'20:45,'21:00,'21:15,'21:30,'21:45,'22:00,'22:15,'22:30,'22:45,'23:00,'23:15,'23:30,'23:45" << endl;
			temporal_demand_profile_file << "0,0,0,,0.000638,0.000776,0.000736,0.000693,0.000673,0.000675,0.000678,0.000670,0.000623,0.000589,0.000544,0.000508,0.000485,0.000498,0.000496,0.000485,0.000521,0.000676,0.000906,0.001251,0.001857,0.002757,0.003900,0.005182,0.006929,0.009006,0.011595,0.014153,0.016485,0.017875,0.019169,0.020060,0.020260,0.019432,0.018501,0.017496,0.016258,0.014779,0.013464,0.012561,0.012095,0.012132,0.012495,0.013003,0.013392,0.013504,0.013642,0.013956,0.014242,0.014255,0.013949,0.013672,0.013371,0.013109,0.012968,0.013125,0.013508,0.013801,0.014118,0.014798,0.015879,0.016828,0.017805,0.018880,0.020253,0.021347,0.022224,0.023036,0.023580,0.022864,0.021493,0.021091,0.021049,0.019134,0.016075,0.014427,0.013855,0.012858,0.011267,0.010514,0.010125,0.009442,0.008378,0.008006,0.007973,0.007531,0.006662,0.006176,0.005833,0.005171,0.004126,0.003501,0.003078,0.002537,0.001726,0.001286" << endl;
			temporal_demand_profile_file.close();
		}

	}
	if (parser_TDProfile.inFile .is_open () || parser_TDProfile.OpenCSVFile("input_temporal_demand_profile.csv"))
	{

		float total_ratio = 0.0f;
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


			for(int t = g_DemandLoadingStartTimeInMin/15; t< g_DemandLoadingEndTimeInMin/15; t++)  // / 15 converts min to 15-min interval for demand patterns
			{
				std::string time_stamp_str = g_GetTimeStampStrFromIntervalNo (t);
				double ratio = 0;
				parser_TDProfile.GetValueByFieldName(time_stamp_str,ratio);


				if(ratio > 0.0001)
				{
					element.time_dependent_ratio[t] = ratio;
					total_ratio+= ratio;
				}
			}

			g_TimeDependentDemandProfileVector.push_back (element);

		}

		if( total_ratio < 0.001)
		{
			cout << "The total temporal ratio read from file input_temporal_demand_profile.csv is 0, which means no demand will be loaded. " << endl;
			cout << "Parameters loading_start_hour = " << g_DemandLoadingStartTimeInMin/60 << "; loading_end_hour = " << g_DemandLoadingEndTimeInMin/60 << endl;
			cout <<	"Please check if parameters loading_start_hour and loading_end_hour in file DTASettings.txt are correct " ;
			g_ProgramStop();
		}


	}


	//*******************************
	// step 8: VOT input

	////////////////////////////////////// VOT
	cout << "Step 9: Reading file input_VOT.csv..."<< endl;
	g_LogFile << "Step 9: Reading file input_VOT.csv..."<< endl;

	CCSVParser parser_VOT;

	float cumulative_percentage = 0;
	if (!parser_VOT.OpenCSVFile("input_VOT.csv"))
	{
		ofstream VOT_file;
		VOT_file.open ("input_VOT.csv");
		if(VOT_file.is_open ())
		{
			VOT_file << "demand_type,VOT_dollar_per_hour,percentage" << endl;
			VOT_file.close();

		}
	}

	if (parser_VOT.inFile .is_open () || parser_VOT.OpenCSVFile("input_VOT.csv"))
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

	}else
	{
		cout << "input_VOT.csv cannot be opened."<< endl;
		g_ProgramStop();
	}


	// done with zone.csv
//	DTANetworkForSP PhysicalNetwork(g_NodeVector.size(), g_LinkVector.size(), g_PlanningHorizon,g_AdjLinkSize);  //  network instance for single processor in multi-thread environment
//	PhysicalNetwork.BuildPhysicalNetwork(0);
//	PhysicalNetwork.IdentifyBottlenecks(g_StochasticCapacityMode);
	//	ConnectivityChecking(&PhysicalNetwork);

	//*******************************
	// step 10: demand trip file input

	// initialize the demand loading range, later resized by CreateVehicles
	if(g_VehicleLoadingMode == 0)  // load from demand table
	{ 
		////////////////////////////////////// VOT
		cout << "Step 10: Reading file input_demand.csv..."<< endl;
		g_LogFile << "Step 10: Reading file input_demand.csv..."<< endl;

//		g_ReadDemandFile_Parser();
		g_ReadDemandFile();

	}else
	{  // load from vehicle file
		////////////////////////////////////// VOT
		cout << "Step 10: Reading file input_vehicle.csv..."<< endl;
		g_LogFile << "Step 10: Reading file input_vehicle.csv..."<< endl;

//		ReadDTALiteVehicleFile("input_vehicle.csv",&PhysicalNetwork);

	}

	cout << "Step 11: Reading file input_sensor.csv "<< endl;
	g_LogFile << "Step 11: Reading file input_sensor.csv"<< endl;

//	g_ReadLinkMeasurementFile(&PhysicalNetwork);

	/* to do: change this to the loop for map structure 
	for(z = 1; z <=g_ODZoneSize; z++)
	{
	if(g_ZoneMap[z]->m_Demand >= g_ZoneMap[z].m_Capacity )
	g_WarningFile << "Zone "<< z << " has demand " << g_ZoneMap[z]->m_Demand  << " and capacity " << g_ZoneMap[z].m_Capacity<< endl;

	}
	*/
	//*******************************
	// step 9: Crash Prediction input

//	ReadScenarioInputFiles(&PhysicalNetwork);


	//	cout << "Global Loading Factor = "<< g_DemandGlobalMultiplier << endl;


	cout << "Number of Zones = "<< g_ODZoneSize  << endl;
	cout << "Number of Nodes = "<< g_NodeVector.size() << endl;
	cout << "Number of Links = "<< g_LinkVector.size() << endl;
	cout << "Number of Vehicles to be Simulated = "<< g_VehicleVector.size() << endl;
	cout <<	"Demand Loading Period = " << g_DemandLoadingStartTimeInMin << " min -> " << g_DemandLoadingEndTimeInMin << " min." << endl;

	if(g_PlanningHorizon < g_DemandLoadingEndTimeInMin + 300)
	{
		//reset simulation horizon to make sure it is longer than the demand loading horizon
		g_PlanningHorizon = g_DemandLoadingEndTimeInMin+ 300;

		for(unsigned link_index = 0; link_index< g_LinkVector.size(); link_index++)
		{
			DTALink* pLink = g_LinkVector[link_index];
			pLink ->ResizeData (g_PlanningHorizon);
		}

	}
	g_NetworkMOEAry.clear();

	for(int time = 0; time <=  g_PlanningHorizon; time++)
	{
		NetworkMOE element;
		g_NetworkMOEAry.push_back (element);
	}

	g_AssignmentMOEVector.clear();
	for(int iter = 0; iter <= g_NumberOfIterations; iter++)
	{
		NetworkLoadingOutput element;
		g_AssignmentMOEVector.push_back (element);
	}


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

		if(vhc.m_DepartureTime  >=1000)
			TRACE("");

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
		g_AggregationTimetInterval =  60;
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
			pVehicle->m_PricingType  = g_read_integer(st) ; 
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

			if(i == 25686)
				TRACE("");
			pVehicle->m_VehicleID		= i;
			pVehicle->m_RandomSeed = pVehicle->m_VehicleID;

			pVehicle->m_OriginZoneID	= kvhc->m_OriginZoneID ;
			pVehicle->m_DestinationZoneID 	= kvhc->m_DestinationZoneID ;

			pVehicle->m_OriginNodeID	= g_ZoneMap[kvhc->m_OriginZoneID].GetRandomOriginNodeIDInZone ((pVehicle->m_VehicleID%100)/100.0f);  // use pVehicle->m_VehicleID/100.0f as random number between 0 and 1, so we can reproduce the results easily
			pVehicle->m_DestinationNodeID 	=  g_ZoneMap[kvhc->m_DestinationZoneID].GetRandomDestinationIDInZone ((pVehicle->m_VehicleID%100)/100.0f); 

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
void g_ReadDemandFile_Parser()
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

			for(unsigned int demand_type = 1; demand_type <= g_DemandTypeMap.size(); demand_type++)
			{
				std::ostringstream  str_number_of_vehicles; 
				str_number_of_vehicles << "number_of_trips_demand_type" << demand_type;
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
						for(int time_interval = 0; time_interval < MAX_TIME_INTERVAL_SIZE; time_interval++)
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
				}

			}
			if(line_no %100000 ==0)
			{
				cout << g_GetAppRunningTime() << "Reading " << line_no/1000 << "K lines..."<< endl;
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
		g_AggregationTimetInterval =  60;
	}

	// round the demand loading horizon using g_AggregationTimetInterval as time unit


	g_AggregationTimetIntervalSize = max(1,(g_DemandLoadingEndTimeInMin)/g_AggregationTimetInterval);
	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ODZoneSize+1, g_AggregationTimetIntervalSize);

	g_ConvertDemandToVehicles();
}
void g_ReadDemandFile()
{
	bool bFileReady = false;
	int i;

	float total_demand_in_demand_file = 0;
	float total_number_of_vehicles_to_be_generated = 0;

	int max_line_number = 100000;

	FILE* st;
	fopen_s(&st,"input_demand.csv", "r");
	if (st!=NULL)
	{
		char  str_line[2000]; // input string
		int str_line_size;
		g_read_a_line(st,str_line, str_line_size); //  skip the first line

		bFileReady = true;
		int line_no = 1;

		int originput_zone, destination_zone;
		float number_of_vehicles ;
		int demand_type;
		float starting_time_in_min;
		float ending_time_in_min;

		while( fscanf(st,"%d,%d,%f,%f,",&originput_zone,&destination_zone,&starting_time_in_min, &ending_time_in_min) >0)
		{

			

			// static traffic assignment, set the demand loading horizon to [0, 60 min]
			if(g_TrafficFlowModelFlag ==0)  //BRP  // static assignment parameters
			{
				starting_time_in_min  = 0;
				ending_time_in_min = 60;
			}

			for(unsigned int demand_type = 1; demand_type <= g_DemandTypeMap.size(); demand_type++)
			{

				number_of_vehicles = 0;
				if( fscanf(st,"%f,",&number_of_vehicles) == 0)
				{
					cout << "Error: Demand Type " << demand_type << "at line " << line_no << "cannot be found in input_demand.csv."<< endl;
					break;
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
						for(int time_interval = 0; time_interval < MAX_TIME_INTERVAL_SIZE; time_interval++)
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
				}

			}
			if(line_no %100000 ==0)
			{
				cout << g_GetAppRunningTime() << "Reading " << line_no/1000 << "K lines..."<< endl;
			}
			line_no++;

			//if(line_no >= max_line_number)
			//	break;
		}

		fclose(st);
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
		g_AggregationTimetInterval =  60;
	}

	// round the demand loading horizon using g_AggregationTimetInterval as time unit


	g_AggregationTimetIntervalSize = max(1,(g_DemandLoadingEndTimeInMin)/g_AggregationTimetInterval);
	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ODZoneSize+1, g_AggregationTimetIntervalSize);

	g_ConvertDemandToVehicles();
}
void FreeMemory()
{


	// Free pointers

	cout << "Free node set... " << endl;

	if(g_HistODDemand !=NULL)
	{
		Deallocate3DDynamicArray<float>(g_HistODDemand,g_ODZoneSize+1,g_ODZoneSize+1);
		g_HistODDemand = NULL;
	}

	if(g_TDOVehicleArray!=NULL && g_ODZoneSize > 0);
	{
		DeallocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_TDOVehicleArray,g_ODZoneSize+1, g_AggregationTimetIntervalSize);  // +1 is because the zone numbers start from 1 not from 0
		g_TDOVehicleArray = NULL;
	}

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
	g_VehicleTypeVector.clear();
	g_NodeNametoIDMap.clear();

	//	FreeClearMap(g_ZoneMap);
	g_ZoneMap.clear ();
	g_DemandTypeMap.clear();
	g_TimeDependentDemandProfileVector.clear();
	g_FreeMemoryForVehicleVector();
	g_NetworkMOEAry.clear();

	g_VOTDistributionVector.clear();
	g_AssignmentMOEVector.clear();
	g_ODTKPathVector.clear();
	g_simple_vector_vehicles.clear();
}




void OutputLinkMOEData(char fname[_MAX_PATH], int Iteration, bool bStartWithEmpty)
{
	FILE* st = NULL;
	FILE* st_struct = NULL;

	if(bStartWithEmpty)
		fopen_s(&st,fname,"w");
	else
		fopen_s(&st,fname,"a");

	fopen_s(&st_struct,"output_LinkTDMOE.bin","wb");

	std::set<DTALink*>::iterator iterLink;

	typedef struct 
	{
		int from_node_id;
		int to_node_id;
		int timestamp_in_min;
		int travel_time_in_min;
		int delay_in_min;
		float link_volume_in_veh_per_hour_per_lane;
		float link_volume_in_veh_per_hour_for_all_lanes;
		float density_in_veh_per_mile_per_lane;
		float speed_in_mph;
		float exit_queue_length;
		int cumulative_arrival_count;
		int cumulative_departure_count;
		int cumulative_SOV_count;
		int cumulative_HOV_count;
		int cumulative_truck_count;
		int cumulative_SOV_revenue;
		int cumulative_HOV_revenue;
		int cumulative_truck_revenue;
	} struct_TDMOE;

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
					float travel_time = g_LinkVector[li]->GetTravelTimeByMin(Iteration,time,1);

					struct_TDMOE tdmoe_element;

					fprintf(st, "%d,%d,%d,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f, %d, %d, %d,",
						g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName, g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName,time,
						travel_time, travel_time - g_LinkVector[li]->m_FreeFlowTravelTime ,
						LinkOutFlow*60.0/g_LinkVector[li]->m_NumLanes ,LinkOutFlow*60.0,
						(g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount-g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount)/g_LinkVector[li]->m_Length /g_LinkVector[li]->m_NumLanes,
						g_LinkVector[li]->GetSpeed(time), g_LinkVector[li]->m_LinkMOEAry[time].ExitQueueLength, 
						g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount ,
						g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount);


					tdmoe_element. from_node_id = g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName;
					tdmoe_element. to_node_id = g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName;
					tdmoe_element. timestamp_in_min = time;
					tdmoe_element. travel_time_in_min = travel_time;
					tdmoe_element. delay_in_min = travel_time - g_LinkVector[li]->m_FreeFlowTravelTime;
					tdmoe_element. link_volume_in_veh_per_hour_per_lane = LinkOutFlow*60.0/g_LinkVector[li]->m_NumLanes;
					tdmoe_element. link_volume_in_veh_per_hour_for_all_lanes = LinkOutFlow*60.0;
					tdmoe_element.  density_in_veh_per_mile_per_lane = (g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount-g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount)/g_LinkVector[li]->m_Length /g_LinkVector[li]->m_NumLanes;
					tdmoe_element.  speed_in_mph = g_LinkVector[li]->GetSpeed(time);
					tdmoe_element. exit_queue_length = g_LinkVector[li]->m_LinkMOEAry[time].ExitQueueLength;
					tdmoe_element. cumulative_arrival_count = g_LinkVector[li]->m_LinkMOEAry[time].CumulativeArrivalCount;
					tdmoe_element. cumulative_departure_count = g_LinkVector[li]->m_LinkMOEAry[time].CumulativeDepartureCount;
					tdmoe_element. cumulative_SOV_count = g_LinkVector[li]->m_LinkMOEAry [time].CumulativeArrivalCount_PricingType[1];
					tdmoe_element. cumulative_HOV_count = g_LinkVector[li]->m_LinkMOEAry [time].CumulativeArrivalCount_PricingType[2];
					tdmoe_element. cumulative_truck_count = g_LinkVector[li]->m_LinkMOEAry [time].CumulativeArrivalCount_PricingType[3];
					tdmoe_element. cumulative_SOV_revenue = g_LinkVector[li]->m_LinkMOEAry [time].CumulativeRevenue_PricingType[1];
					tdmoe_element. cumulative_HOV_revenue = g_LinkVector[li]->m_LinkMOEAry [time].CumulativeRevenue_PricingType[2];
					tdmoe_element. cumulative_truck_revenue = g_LinkVector[li]->m_LinkMOEAry [time].CumulativeRevenue_PricingType[3];


					int pt;
					for(pt = 1; pt < MAX_PRICING_TYPE_SIZE; pt++)
					{
						fprintf(st, "%d,",g_LinkVector[li]->m_LinkMOEAry [time].CumulativeArrivalCount_PricingType[pt]); 
					}

					for(pt = 1; pt < MAX_PRICING_TYPE_SIZE; pt++)
					{
						fprintf(st, "%6.2f,",g_LinkVector[li]->m_LinkMOEAry [time].CumulativeRevenue_PricingType[pt]); 
					}

					fprintf(st,"\n");
					fwrite(&tdmoe_element,sizeof(tdmoe_element),1,st_struct);

				}



			}
		}
		fclose(st);
		fclose(st_struct);
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
		fprintf(g_ErrorFile, "File output_NetworkTDMOE.csv cannot be opened. It might be currently used and locked by EXCEL.");
		cout << "Error: File output_NetworkTDMOE.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		getchar();
		exit(0);

	}


}


void g_OutputLinkMOESummary(int iteration,char fname[_MAX_PATH])
{
	ofstream LinkMOESummaryFile;

	LinkMOESummaryFile.open (fname, ios::out);
	if (LinkMOESummaryFile.is_open())
	{
		LinkMOESummaryFile.width(12);
		LinkMOESummaryFile.precision(3) ;
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

			double average_travel_time = pLink->GetTravelTimeByMin(iteration,0, pLink->m_SimulationHorizon);
			double speed = pLink->m_Length / max(0.00001,average_travel_time) *60;  // unit: mph
			double capacity_simulation_horizon = pLink->m_MaximumServiceFlowRatePHPL * pLink->m_NumLanes;
			double voc_ratio = pLink->CFlowArrivalCount / max(0.1,capacity_simulation_horizon);
			int percentage_of_speed_limit = int(speed/max(0.1,pLink->m_SpeedLimit)*100+0.5);


			LinkMOESummaryFile << g_NodeVector[pLink->m_FromNodeID].m_NodeName  << "," ;
			LinkMOESummaryFile << g_NodeVector[pLink->m_ToNodeID].m_NodeName << "," ;
			LinkMOESummaryFile << g_LinkTypeMap[pLink->m_link_type].type_code.c_str ()<< "," ;
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

			LinkMOESummaryFile << pLink->CFlowArrivalCount << "," ; // total hourly arrival flow 
			LinkMOESummaryFile << error_percentage << "," ;
			LinkMOESummaryFile << fabs(error_percentage) << "," ;

			LinkMOESummaryFile << pLink->m_AADT << "," ;
			LinkMOESummaryFile << pLink->m_NumberOfCrashes << "," ;
			LinkMOESummaryFile << pLink->m_NumberOfFatalAndInjuryCrashes << "," ;
			LinkMOESummaryFile << pLink->m_NumberOfPDOCrashes << "," ;

			LinkMOESummaryFile << pLink->m_TotalEnergy  << "," ;
			LinkMOESummaryFile << pLink->m_CO2  << "," ;
			LinkMOESummaryFile << pLink->m_NOX  << "," ;
			LinkMOESummaryFile << pLink->m_CO  << "," ;
			LinkMOESummaryFile << pLink->m_HC  << "," ;
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

		LinkMOESummaryFile.close ();

	}else
	{
		fprintf(g_ErrorFile, "File %s cannot be opened. It might be currently used and locked by EXCEL.", fname);
		cout << "Error: File " << fname << " cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		getchar();
	}
}



void OutputVehicleTrajectoryData(char fname[_MAX_PATH],int Iteration, bool bStartWithEmpty)
{

	FILE* st = NULL;

	FILE* st_struct = NULL;

	if(bStartWithEmpty)
		fopen_s(&st,fname,"w");
	else
		fopen_s(&st,fname,"a");

	fopen_s(&st_struct,"vehicle.bin","wb");

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
	} struct_Vehicle_Header;

	typedef  struct  
	{
		int NodeName;
		float AbsArrivalTimeOnDSN;
	} struct_Vehicle_Node;

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
				fprintf(st,"%d,%d,%d,%4.2f,%4.2f,%d,%4.2f,%d,%d,%d,%d,%4.2f,%4.3f,%4.3f,%4.3f,%d,",
					pVehicle->m_VehicleID , pVehicle->m_OriginZoneID , pVehicle->m_DestinationZoneID,
					pVehicle->m_DepartureTime, pVehicle->m_ArrivalTime , pVehicle->m_bComplete, TripTime,			
					pVehicle->m_DemandType, pVehicle->m_PricingType ,pVehicle->m_VehicleType,
					pVehicle->m_InformationClass, 
					pVehicle->m_VOT , pVehicle->m_TollDollarCost, pVehicle->m_Emissions ,pVehicle->m_Distance, pVehicle->m_NodeSize);

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
				header. number_of_nodes = pVehicle->m_NodeSize;

				fwrite(&header, sizeof(struct_Vehicle_Header), 1, st_struct);

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

				struct_Vehicle_Node node_element;
				node_element. NodeName = NodeName;
				node_element. AbsArrivalTimeOnDSN = pVehicle->m_DepartureTime;
				fwrite(&node_element, sizeof(node_element), 1, st_struct);

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

					node_element. NodeName = NodeName;
					node_element. AbsArrivalTimeOnDSN = pVehicle->m_aryVN [j].AbsArrivalTimeOnDSN;
					fwrite(&node_element, sizeof(node_element), 1, st_struct);

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
		fclose(st_struct);
	}else
	{
		fprintf(g_ErrorFile, "File vehicle.csv cannot be opened. It might be currently used and locked by EXCEL.");
		cout << "Error: File vehicle.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		getchar();
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
		g_AssignmentLogFile << "CPU_time,iteration_no,avg_travel_time_in_min,travel_time_index,avg_travel_distance_in_mile,vehicle_route_switching_rate,number_of_vehicles_completing_trips,perc_of_vehicles_completing_trips,avg_travel_time_gap_per_vehicle_in_min,target_demand_deviation,abs_estimation_error_of_link_volume,RMSE_of_est_link_volume,avg_abs_perc_error_of_est_link_volume, ";

		int cl;
		int NumberOfCriticalLinks = 3;

		for( cl = 1; cl <= NumberOfCriticalLinks; cl++)
		{
			g_AssignmentLogFile << "Critical Link ["<< cl << "]" 
				<< CriticalLinkFromNodeNumberAry[cl]<< "->" 
				<< CriticalLinkToNodeNumberAry[cl]<< ":" 
				<< " VOC,volume,ratio,HOV volume,HOV ratio,SOV  Perc,truck  Perc,TT,speed,engery, CO2, NOX, HC, AADT,#OfCrashes,#OfFatalAndInjuryCrashes,#OfPDOCrashes,";
		}
		g_AssignmentLogFile	<<  endl;

	}
	else
	{
		cout << "File output_assignment_log.csv cannot be opened, and it might be locked by another program!" << endl;
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

	ShortSimulationLogFile.open ("output_NetworkMOE.csv", ios::out);
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
	cout << "Version 0.991, Release Date 04/24/2012."<< endl;

	g_LogFile << "---DTALite: A Fast Open-Source DTA Simulation Engine---"<< endl;
	g_LogFile << "Version 0.991, Release Date 04/24/2012."<< endl;

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

	// if  ./DTASettings.ini does not exist, then we should print out all the default settings for user to change
	//
	g_TrafficFlowModelFlag = g_GetPrivateProfileInt("simulation", "traffic_flow_model", 1, g_DTASettingFileName);	
	g_EmissionDataOutputFlag = g_GetPrivateProfileInt("emission", "emission_data_output", 0, g_DTASettingFileName);	
	//		g_TollingMethodFlag = g_GetPrivateProfileInt("tolling", "method_flag", 0, g_DTASettingFileName);	
	//		g_VMTTollingRate = g_GetPrivateProfileFloat("tolling", "VMTRate", 0, g_DTASettingFileName);

	g_ODEstimationFlag = g_GetPrivateProfileInt("estimation", "od_demand_estimation", 0, g_DTASettingFileName);	
	g_ODEstimationMeasurementType = g_GetPrivateProfileInt("estimation", "measurement_type", 1, g_DTASettingFileName);	
	g_ODEstimation_WeightOnHistODDemand = g_GetPrivateProfileFloat("estimation", "weight_on_hist_oddemand", 1, g_DTASettingFileName);
	g_ODEstimation_WeightOnUEGap = g_GetPrivateProfileFloat("estimation", "weight_on_ue_gap", 1, g_DTASettingFileName);


	g_ODEstimation_StartingIteration = g_GetPrivateProfileInt("estimation", "starting_iteration", 10, g_DTASettingFileName);
	g_ObservationTimeInterval = g_GetPrivateProfileInt("estimation", "observation_time_interval", 5, g_DTASettingFileName);
	g_ObservationStartTime = g_GetPrivateProfileInt("estimation", "observation_start_time_in_min", 390, g_DTASettingFileName);
	g_ObservationEndTime = g_GetPrivateProfileInt("estimation", "observation_end_time_in_min", 570, g_DTASettingFileName);


	if(g_ObservationEndTime >= g_ObservationStartTime + g_PlanningHorizon)  // no later than the simulation end time
		g_ObservationEndTime = g_ObservationStartTime + g_PlanningHorizon;

	g_VehicleLoadingMode = g_GetPrivateProfileInt("demand", "load_vehicle_file_mode", 0, g_DTASettingFileName);	
	g_ParallelComputingMode = g_GetPrivateProfileInt("assignment", "parallel_computing", 1, g_DTASettingFileName);


	// research code:		g_StochasticCapacityMode = g_GetPrivateProfileInt("simulation", "stochatic_capacity_mode", 0, g_DTASettingFileName);

	g_MergeNodeModelFlag = g_GetPrivateProfileInt("simulation", "merge_node_model", 1, g_DTASettingFileName);	
	g_MinimumInFlowRatio = g_GetPrivateProfileFloat("simulation", "minimum_link_in_flow_ratio", 0.02f, g_DTASettingFileName);
	g_MaxDensityRatioForVehicleLoading  = g_GetPrivateProfileFloat("simulation", "max_density_ratio_for_loading_vehicles", 0.8f, g_DTASettingFileName);
	g_CycleLength_in_seconds = g_GetPrivateProfileFloat("simulation", "cycle_length_in_seconds", 120, g_DTASettingFileName);
	g_DefaultSaturationFlowRate_in_vehphpl = g_GetPrivateProfileFloat("simulation", "default_saturation_flow_rate_in_vehphpl", 1800, g_DTASettingFileName);

	g_NumberOfIterations = g_GetPrivateProfileInt("assignment", "number_of_iterations", 10, g_DTASettingFileName);	
	g_AgentBasedAssignmentFlag = g_GetPrivateProfileInt("assignment", "agent_based_assignment", 1, g_DTASettingFileName);
	g_AggregationTimetInterval = g_GetPrivateProfileInt("assignment", "aggregation_time_interval_in_min", 15, g_DTASettingFileName);	
	g_NumberOfInnerIterations = g_GetPrivateProfileInt("assignment", "number_of_inner_iterations", 0, g_DTASettingFileName);	
	g_ConvergencyRelativeGapThreshold_in_perc = g_GetPrivateProfileFloat("assignment", "convergency_relative_gap_threshold_percentage", 5, g_DTASettingFileName);	
	g_StartIterationsForOutputPath = g_GetPrivateProfileInt("output", "start_iteration_output_path", g_NumberOfIterations, g_DTASettingFileName);	
	g_EndIterationsForOutputPath = g_GetPrivateProfileInt("output", "end_iteration_output_path", g_NumberOfIterations, g_DTASettingFileName);	

	if( g_StartIterationsForOutputPath  < 0) 
		g_StartIterationsForOutputPath = 0;

	if(g_EndIterationsForOutputPath > g_NumberOfIterations)
		g_EndIterationsForOutputPath = g_NumberOfIterations;


	g_UEAssignmentMethod = g_GetPrivateProfileInt("assignment", "UE_assignment_method", 1, g_DTASettingFileName); // default is MSA

	// parameters for day-to-day learning mode
	g_LearningPercentage = g_GetPrivateProfileInt("assignment", "learning_percentage", 15, g_DTASettingFileName);
	g_TravelTimeDifferenceForSwitching = g_GetPrivateProfileInt("assignment", "travel_time_difference_for_switching_in_min", 5, g_DTASettingFileName);
	g_DemandGlobalMultiplier = g_GetPrivateProfileFloat("demand", "global_multiplier",1.0,g_DTASettingFileName);	

	g_DemandLoadingStartTimeInMin = ((int) g_GetPrivateProfileFloat("demand", "loading_start_hour",6,g_DTASettingFileName)*60);	
	g_DemandLoadingEndTimeInMin = ((int)g_GetPrivateProfileFloat("demand", "loading_end_hour",12,g_DTASettingFileName)*60);	

	if(g_DemandLoadingEndTimeInMin >=2000)
	{
		cout << "Error: g_DemandLoadingEndTimeInMin >=2000" ;
		g_ProgramStop();
	}

	if(g_PlanningHorizon < g_DemandLoadingEndTimeInMin + 300)
	{
		//reset simulation horizon to make sure it is longer than the demand loading horizon
		g_PlanningHorizon = g_DemandLoadingEndTimeInMin+ 300;
	}

	g_start_iteration_for_MOEoutput = g_GetPrivateProfileInt("output", "start_iteration_for_MOE", -1, g_DTASettingFileName);	

	g_RandomSeed = g_GetPrivateProfileInt("simulation", "random_number_seed", 100, g_DTASettingFileName);

	g_UserClassPerceptionErrorRatio[1] = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_historical_info_travelers_perception_error",0.3f,g_DTASettingFileName);	
	g_UserClassPerceptionErrorRatio[2] = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_pretrip_info_travelers_perception_error",0.05f,g_DTASettingFileName);	
	g_UserClassPerceptionErrorRatio[3] = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_en-route_info_travelers_perception_error",0.05f,g_DTASettingFileName);	

	g_VMSPerceptionErrorRatio          = g_GetPrivateProfileFloat("traveler_information", "coefficient_of_variation_of_VMS_perception_error",0.05f,g_DTASettingFileName);	
	g_information_updating_interval_of_en_route_info_travelers_in_min = g_GetPrivateProfileInt("traveler_information", "information_updating_interval_of_en_route_info_travelers_in_min",5,g_DTASettingFileName);	
	g_information_updating_interval_of_VMS_in_min  = g_GetPrivateProfileInt("traveler_information", "information_updating_interval_of_VMS_in_min",60,g_DTASettingFileName);	


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

void g_OutputSimulationStatistics(int Iteration)
{
	int NumberOfLinks = 9999999;
	g_OutputLinkMOESummary(Iteration,"output_LinkMOE.csv");  // output assignment results anyway
	//	g_OutputSummaryKML(MOE_crashes);
	//	g_OutputSummaryKML(MOE_CO2);
	//	g_OutputSummaryKML(MOE_total_energy);

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

	g_LogFile << g_GetAppRunningTime()  <<" ---End of Link MOE ---" << endl;

}

void g_FreeMemory()
{
	cout << "Free memory... " << endl;
	FreeMemory();

	if(g_ErrorFile);
	fclose(g_ErrorFile);
	g_LogFile.close();
	g_AssignmentLogFile.close();
	g_WarningFile.close();

	g_LogFile << "Assignment-Simulation Completed. " << g_GetAppRunningTime() << endl;

	if(ShortSimulationLogFile.is_open())
	{
		ShortSimulationLogFile.close();
	}

}

void g_TrafficAssignmentSimulation()
{

	g_ReadInputFiles();


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

void g_DTALiteMain()
{
	if(g_InitializeLogFiles()==0) 
		return;

	g_ReadDTALiteSettings();
	g_TrafficAssignmentSimulation();
	g_OutputSimulationStatistics(g_NumberOfIterations);
	g_FreeMemory();


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

	/**********************************************/
	//below is the main traffic assignment-simulation code



	// if  ./DTASettings.ini does not exist, then we should print out all the default settings for user to change
	//
	g_AppStartTime = CTime::GetCurrentTime();


	int UnitTestingFlag = g_GetPrivateProfileInt("testing", "unit_testing", 0, g_DTASettingFileName);	

	if(UnitTestingFlag == 1)
	{
		g_UnitTestingLogFile.open ("unit_testing.csv", ios::out);
		if (g_UnitTestingLogFile.is_open())
		{
			g_UnitTestingLogFile.width(12);
			g_UnitTestingLogFile.precision(3) ;
			g_UnitTestingLogFile.setf(ios::fixed);
			g_UnitTestingLogFile << "Start unit testing..." << endl;
		}else
		{
			cout << "File unit_testing.log cannot be opened, and it might be locked by another program or the target data folder is read-only." << endl;
			cin.get();  // pause
			return 0;
		}

		g_DTALiteUnitTestingMain();

		g_UnitTestingLogFile.close();

	}
	else
	{
		g_DTALiteMain();

	}

	return nRetCode;
}

void DTANetworkForSP::IdentifyBottlenecks(int StochasticCapacityFlag)
{

	g_LogFile << "The following freeway/highway bottlenecks are identified."<< endl;

	// ! there is an freeway or highway downstream with less number of lanes
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		if( g_LinkTypeMap[g_LinkVector[li]->m_link_type].IsFreeway ()  &&  m_OutboundSizeAry[g_LinkVector[li]->m_ToNodeID] ==1)  // freeway or highway
		{
			int FromID = g_LinkVector[li]->m_FromNodeID;
			int ToID   = g_LinkVector[li]->m_ToNodeID;

			for(int i=0; i< m_OutboundSizeAry[ToID]; i++)
			{
				DTALink* pNextLink =  g_LinkVector[m_OutboundLinkAry[ToID][i]];
				if(g_LinkTypeMap[pNextLink->m_link_type ].IsFreeway () && pNextLink->m_NumLanes < g_LinkVector[li]->m_NumLanes && pNextLink->m_ToNodeID != FromID)
				{
					g_LinkVector[li]->m_StochaticCapcityFlag = StochasticCapacityFlag;  //lane drop from current link to next link
					g_LogFile << "lane drop:" << g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName << endl;
				}

			}

		}
	}


	// merge: one outgoing link, two more incoming links with at least freeway link

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		int incoming_link_freeway_and_ramp_count = 0;
		bool no_arterial_incoming_link = true;
		for(int incoming_link = 0; incoming_link <  m_InboundSizeAry[g_LinkVector[li]->m_FromNodeID]; incoming_link++) // one outgoing link without considering u-turn
		{
			int incoming_link_id = m_InboundLinkAry[g_LinkVector[li]->m_FromNodeID][incoming_link];

			if((g_LinkVector[incoming_link_id]->m_FromNodeID != g_LinkVector[li]->m_ToNodeID)) // non-uturn link
			{
				if(g_LinkTypeMap[g_LinkVector[incoming_link_id]->m_link_type ].IsFreeway() //freeway link
					|| g_LinkTypeMap[g_LinkVector[incoming_link_id]->m_link_type].IsRamp ())
				{
					incoming_link_freeway_and_ramp_count++;

				}else
				{
					no_arterial_incoming_link = false;

				}
			}

		}
		if(incoming_link_freeway_and_ramp_count >=2 && no_arterial_incoming_link)
		{
			TRACE("\nMerge link: %d->%d",g_LinkVector[li]->m_FromNodeNumber , g_LinkVector[li]->m_ToNodeNumber );
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
				if(g_LinkTypeMap[g_LinkVector[m_InboundLinkAry[FromID][il]]->m_link_type].IsRamp ())  // on ramp as incoming link
				{
					bRampExistFlag = true;
					g_LinkVector[li]->m_MergeOnrampLinkID = m_InboundLinkAry[FromID][il];
				}
				if(g_LinkTypeMap[g_LinkVector[m_InboundLinkAry[FromID][il]]->m_link_type ].IsFreeway () || 
					g_LinkTypeMap[g_LinkVector[m_InboundLinkAry[FromID][il]]->m_link_type ].IsHighway ())  // freeway or highway
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
			}

		}

	}

}





void DTASafetyPredictionModel::UpdateCrashRateForAllLinks()
{

	std::set<DTALink*>::iterator iterLink;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		DTALink* pLink = g_LinkVector[li];

		pLink->m_AADT = pLink->CFlowArrivalCount/max(pLink->m_AADTConversionFactor,0.0001) ;

		double CrashRate= 0;
		if( g_LinkTypeMap[pLink->m_link_type].IsFreeway ())  // freeway
		{
			pLink->m_NumberOfCrashes  = EstimateFreewayCrashRatePerYear(pLink->m_AADT , pLink->m_Length );
		}

		if( g_LinkTypeMap[pLink->m_link_type].IsArterial ())   //arterial
		{
			pLink->m_NumberOfCrashes  = 
				EstimateArterialCrashRatePerYear(pLink->m_NumberOfFatalAndInjuryCrashes, pLink->m_NumberOfPDOCrashes,
				pLink->m_AADT, 
				pLink->m_Length,
				pLink->m_Num_Driveways_Per_Mile,
				pLink->m_volume_proportion_on_minor_leg,
				pLink->m_Num_3SG_Intersections,
				pLink->m_Num_3ST_Intersections,
				pLink->m_Num_4SG_Intersections,
				pLink->m_Num_4ST_Intersections);

		}

	}

}

void g_OutputSummaryKML(Traffic_MOE moe_mode)
{

	float base_rate = 1;

	FILE* st;
	if(moe_mode == MOE_crashes)
	{
		fopen_s(&st,"simulated_crashes.kml","w");
		base_rate = 1;
	}
	if(moe_mode == MOE_CO2)
	{
		fopen_s(&st,"simulated_CO2.kml","w");

		float total_CO2 = 0 ;
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			DTALink* pLink = g_LinkVector[li];
			total_CO2 += pLink->m_CO2 ;
		}

		base_rate = total_CO2/5000;  // generate abound 5000 place marks per area
	}
	if(moe_mode == MOE_total_energy)
	{
		fopen_s(&st,"simulated_total_energy.kml","w");

		float total_energy = 0 ;
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			DTALink* pLink = g_LinkVector[li];
			total_energy += pLink->m_TotalEnergy ;
		}

		base_rate = total_energy/5000;  // generate abound 5000 place marks per area
	}


	if(st!=NULL)
	{
		fprintf(st,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(st,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		fprintf(st,"<Document>\n");

		if(moe_mode == MOE_crashes)
			fprintf(st,"<name>simulated_crashes</name>\n");
		if(moe_mode == MOE_CO2)
			fprintf(st,"<name>simulated_emission_CO2_event: unit: %.3f (g//hr)</name>\n",base_rate);
		if(moe_mode == MOE_total_energy)
			fprintf(st,"<name>simulated_t_event: unit: %.3f (J//hr)</name>\n",base_rate);


		int total_count = 0;

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			DTALink* pLink = g_LinkVector[li];

			int count  = 0;

			float value = 0;

			switch (moe_mode)
			{
			case MOE_crashes:
				value =   pLink->m_NumberOfCrashes/base_rate;break;
			case MOE_CO2:
				value =   pLink->m_CO2 /base_rate;break;
			case MOE_total_energy:
				value =   pLink->m_TotalEnergy /base_rate;break;
			default:
				value = 0;
			}



			count = (int)(value);
			float residual = value  - count;
			if(g_GetRandomRatio() < residual)
				count ++;


			for(int i = 0; i < count; i++)
			{
				float random_ratio = g_GetRandomRatio(); 

				float x, y;
				if( pLink  ->m_ShapePoints .size() ==2)
				{
					x = random_ratio * pLink  ->m_ShapePoints[0].x + (1-random_ratio)* pLink  ->m_ShapePoints[1].x;
					y = random_ratio * pLink  ->m_ShapePoints[0].y + (1-random_ratio)* pLink  ->m_ShapePoints[1].y;
				}else if (pLink  ->m_ShapePoints .size() >0)
				{
					int shape_point_id = pLink  ->m_ShapePoints .size()*random_ratio;
					x = pLink  ->m_ShapePoints[shape_point_id].x;
					y = pLink  ->m_ShapePoints[shape_point_id].y;
				}


				fprintf(st,"\t<Placemark>\n");
				fprintf(st,"\t <name>%d</name>\n",total_count);

				if(moe_mode == MOE_crashes)
				{

					if(g_GetRandomRatio()* pLink->m_NumberOfCrashes < pLink  ->m_NumberOfFatalAndInjuryCrashes)
					{
						fprintf(st,"\t <description>fatal crash</description>\n");
					}else if(g_GetRandomRatio()* pLink->m_NumberOfCrashes < pLink  ->m_NumberOfPDOCrashes)
					{
						fprintf(st,"\t <description>PTO crash</description>\n");
					}else
					{
						fprintf(st,"\t <description>crash</description>\n");
					}
				}
				if(moe_mode == MOE_CO2 || moe_mode == MOE_total_energy )
				{
					fprintf(st,"\t <description>%d-%d</description>\n",g_NodeVector[pLink->m_FromNodeID].m_NodeName, g_NodeVector[pLink->m_ToNodeID].m_NodeName);
				}


				fprintf(st,"\t <Point><coordinates>%f,%f,0</coordinates></Point>\n",x,y);
				fprintf(st,"\t</Placemark>\n");

				total_count++;
			}  // for each count
		} // for each link   
		fprintf(st,"</Document>\n");
		fprintf(st,"</kml>\n");
		fclose(st);
	}
}

void g_SetLinkTollValue(int usn, int dsn, Toll tc)
{
	DTANetworkForSP PhysicalNetwork(g_NodeVector.size(), g_LinkVector.size(), g_PlanningHorizon,g_AdjLinkSize);  //  network instance for single processor in multi-thread environment
	PhysicalNetwork.BuildPhysicalNetwork(0);
	int LinkID = PhysicalNetwork.GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

	DTALink* pLink = g_LinkVector[LinkID];

	if(pLink!=NULL)
	{
		pLink->TollVector.clear();
		pLink->TollVector.push_back(tc);
	}
}

void g_SetLinkAttributes(int usn, int dsn, int NumOfLanes)
{
	DTANetworkForSP PhysicalNetwork(g_NodeVector.size(), g_LinkVector.size(), g_PlanningHorizon,g_AdjLinkSize);  //  network instance for single processor in multi-thread environment
	PhysicalNetwork.BuildPhysicalNetwork(0);
	int LinkID = PhysicalNetwork.GetLinkNoByNodeIndex(g_NodeNametoIDMap[usn], g_NodeNametoIDMap[dsn]);

	DTALink* pLink = g_LinkVector[LinkID];
	pLink->m_NumLanes = NumOfLanes;

}



void g_OutputDay2DayVehiclePathData(char fname[_MAX_PATH],int StartIteration,int EndIteration)
{

	ofstream output_path_file;
	output_path_file.open(fname);
	if(output_path_file.is_open ())
	{
		output_path_file << "from_zone_id,to_zone_id,departure_time,vehicle_id,day_no,node_sum,number_of_nodes,path_sequence" << endl;

		std::map<int, DTAVehicle*>::iterator iterVM;
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{
			DTAVehicle* pVeh = iterVM->second;
					for(int DayNo = g_StartIterationsForOutputPath; DayNo<= g_EndIterationsForOutputPath; DayNo++)
					{
						DTAPath element = pVeh->Day2DayPathMap [DayNo];
						output_path_file << pVeh->m_OriginZoneID << "," << pVeh->m_DestinationZoneID << "," << pVeh->m_DepartureTime  << ",v" << pVeh->m_VehicleID  << "," << DayNo << "," <<
							element.NodeSum << "," << (int)(element.LinkSequence .size()) << ",{";

						if(element.LinkSequence.size()>=1)
						{
						int LinkID_0 = element.LinkSequence [0];
						output_path_file << g_LinkVector[LinkID_0]->m_FromNodeNumber << ";";

							for(int j = 0; j< element.LinkSequence.size()-1; j++)
							{
								int LinkID = element.LinkSequence[j];
								output_path_file << g_LinkVector[LinkID]->m_ToNodeNumber<< ";";
							}

						}


						output_path_file << "}" <<endl;
					} // for each day
				} // for each vehicle


		output_path_file.close();
	}
}
