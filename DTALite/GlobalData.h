//  Portions Copyright 2010 Xuesong Zhou

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

// global data structure
#include <iostream>
#include <fstream>
extern std::set<DTANode*>		g_NodeSet;
extern std::map<int, DTANode*> g_NodeMap;
extern std::map<int, int> g_NodeIDtoNameMap;


extern std::set<DTALink*>		g_LinkSet;
extern std::map<int, DTALink*> g_LinkMap;

extern std::vector<DTAVehicle*>		g_VehicleVector;
extern std::map<int, DTAVehicle*> g_VehicleMap;

extern int ***g_RTNodePredAry;  // vehicle type, origin, node, available in real-time simulation, range 0 -65535
extern unsigned char*** g_RTLabelCostAry; // vehicle type, origin, node, available in real-time simulation, to save memory, the unit is min, data range is 0 to 255.

// time inteval settings in assignment and simulation
extern double g_DTASimulationInterval; // min
extern int g_number_of_intervals_per_min; // round to nearest integer
extern int g_DepartureTimetInterval; // min

// maximal # of adjacent links of a node (including physical nodes and centriods( with connectors))
extern int g_AdjLinkSize; // initial value of adjacent links

extern int g_ODZoneSize;

// assignment and simulation settings
extern int g_NumberOfIterations;
extern float g_ConvergencyRelativeGapThreshold_in_perc;
extern int g_NumberOfInnerIterations;
extern int g_DepartureTimetIntervalSize;

extern int g_DemandLoadingHorizon; // min
extern std::vector<float> g_DemandTemporalShare_5_min_res;
extern int g_SimulationHorizon; // min
extern int g_Number_of_CompletedVehicles;
extern int g_Number_of_GeneratedVehicles;

extern int g_InfoTypeSize;  // for shortest path with generalized costs depending on LOV, HOV, trucks or other vehicle classes.

// parameters for day-to-day learning
extern int g_MinSwitchingRate;   // percentage
extern int g_LearningPercentage;
extern int g_TravelTimeDifferenceForSwitching;  // min

extern int g_StochasticCapacityMode ;
extern float g_FreewayJamDensity_in_vehpmpl;
extern int g_VehicleLoadingMode;
extern int g_MergeNodeModelFlag;
extern int g_TrafficFlowModelFlag;

// for traffic assignment 
extern int g_UEAssignmentMethod; // 0: MSA, 1: day-to-day learning with fixed switch rate 2: GAP-based switching rule for UE, 3: Gap-based switching rule + MSA step size for UE
extern float g_CurrentGapValue; // total network gap value in the current iteration
extern float g_PrevGapValue; // total network gap value in last iteration
extern float g_RelativeGap; // = abs(g_CurrentGapValue - g_PrevGapValue) / g_PrevGapValue 
extern int g_CurrentNumOfVehiclesSwitched; // total number of vehicles switching paths in the current iteration; for MSA, g_UEAssignmentMethod = 0
extern int g_PrevNumOfVehiclesSwitched; // // total number of vehicles switching paths in last iteration; for MSA, g_UEAssignmentMethod = 0
extern int g_ConvergenceThreshold_in_Num_Switch; // the convergence threshold in terms of number of vehicles switching paths; for MSA, g_UEAssignmentMethod = 0
extern int g_VehicleExperiencedTimeGap; // 1: Vehicle experienced time gap; 0: Avg experienced path time gap
extern int g_NewPathWithSwitchedVehicles; // number of new paths with vehicles switched to them

// for fast data acessing
extern int g_LastLoadedVehicleID; // scan vehicles to be loaded in a simulation interval

extern int* g_ZoneCentroidSizeAry;  //Number of centroids per zone
extern int** g_ZoneCentroidNodeAry; //centroid node Id per zone

extern FILE* g_ErrorFile;

extern NetworkMOE* g_NetworkMOEAry;

extern VehicleArrayForOriginDepartrureTimeInterval** g_TDOVehicleArray; // TDO for time-dependent origin;

extern VehicleArrayForOriginDepartrureTimeInterval** g_TDOPreTripVehicleArray; // TDO for pretrip time-dependent origin;


extern ofstream g_LogFile;
extern int* g_OVehicleSizeAry;
extern int* g_OPreTripVehicleSizeAry;

extern NetworkLoadingOutput* g_AssignmentMOEAry;
extern ofstream g_AssignmentLogFile;



