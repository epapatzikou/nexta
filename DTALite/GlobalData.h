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
extern std::vector<DTANode> g_NodeVector;
extern std::map<int, int> g_NodeNametoIDMap;
extern std::vector<DTALink*> g_LinkVector;
extern std::map<string, DTALink*> g_LinkMap;

extern std::map<int, DTAZone> g_ZoneMap;
extern std::vector<DTAVehicleType> g_VehicleTypeVector;
extern std::vector<DTAVehicle*>		g_VehicleVector;
extern std::map<int, DTAVehicle*> g_VehicleMap;
extern std::map<int, DemandType> g_DemandTypeMap;
extern std::map<int, PricingType> g_PricingTypeMap;
extern std::vector<TimeDependentDemandProfile> g_TimeDependentDemandProfileVector;

extern std::map<int, DTALinkType> g_LinkTypeMap;
extern	std::vector<VOTDistribution> g_VOTDistributionVector;


extern int g_DemandLoadingStartTimeInMin;
extern int g_DemandLoadingEndTimeInMin;

extern std::vector<NetworkMOE>  g_NetworkMOEAry;

// time inteval settings in assignment and simulation
extern double g_DTASimulationInterval; // min
extern int g_number_of_intervals_per_min; // round to nearest integer
extern int g_number_of_car_following_intervals_per_min;
extern int g_AggregationTimetInterval; // min

// maximal # of adjacent links of a node (including physical nodes and centriods( with connectors))
extern int g_AdjLinkSize; // initial value of adjacent links

extern int g_ODZoneSize;

// assignment and simulation settings
extern int g_ParallelComputingMode;
extern int g_NumberOfIterations;
extern int g_StartIterationsForOutputPath;
extern int g_EndIterationsForOutputPath;

extern float g_ConvergencyRelativeGapThreshold_in_perc;
extern int g_NumberOfInnerIterations;
extern int g_AggregationTimetIntervalSize;

extern int g_PlanningHorizon; // min
extern int g_Number_of_CompletedVehicles;
extern int g_Number_of_GeneratedVehicles;

extern int g_InfoTypeSize;  // for shortest path with generalized costs depending on LOV, HOV, trucks or other vehicle classes.

// parameters for day-to-day learning
extern int g_MinSwitchingRate;   // percentage
extern int g_LearningPercentage;
extern int g_TravelTimeDifferenceForSwitching;  // min

extern int g_StochasticCapacityMode ;
extern int g_VehicleLoadingMode;
extern int g_MergeNodeModelFlag;
extern int g_TrafficFlowModelFlag;
extern int g_EmissionDataOutputFlag;
extern int g_TimeDependentODMOEOutputFlag;
extern int g_VehiclePathOutputFlag;

extern int g_OutputSecondBySecondEmissionData;
extern int g_start_departure_time_in_min_for_output_second_by_second_emission_data;
extern int g_end_departure_time_in_min_for_output_second_by_second_emission_data;
extern int g_OutputEmissionOperatingModeData;
extern int g_TargetVehicleID_OutputSecondBySecondEmissionData;
extern int g_TollingMethodFlag;
extern float g_VMTTollingRate;

// for traffic assignment 
extern int g_UEAssignmentMethod; // 0: MSA, 1: day-to-day learning with fixed switch rate 2: GAP-based switching rule for UE, 3: Gap-based switching rule + MSA step size for UE, 4: departure time choice
extern int g_Day2DayAgentLearningMethod; // 0: no learning (use previous day), 1: route choice learning only, 2: route choice and departure time learning
extern float g_DepartureTimeChoiceEarlyDelayPenalty, g_DepartureTimeChoiceLateDelayPenalty;
extern float g_CurrentGapValue; // total network gap value in the current iteration
extern float g_PrevGapValue; // total network gap value in last iteration
extern float g_RelativeGap; // = abs(g_CurrentGapValue - g_PrevGapValue) / g_PrevGapValue 
extern int g_CurrentNumOfVehiclesSwitched; // total number of vehicles switching paths in the current iteration; for MSA, g_UEAssignmentMethod = 0
extern int g_PrevNumOfVehiclesSwitched; // // total number of vehicles switching paths in last iteration; for MSA, g_UEAssignmentMethod = 0
extern int g_ConvergenceThreshold_in_Num_Switch; // the convergence threshold in terms of number of vehicles switching paths; for MSA, g_UEAssignmentMethod = 0
extern int g_VehicleExperiencedTimeGap; // 1: Vehicle experienced time gap; 0: Avg experienced path time gap
extern int g_NewPathWithSwitchedVehicles; // number of new paths with vehicles switched to them

// for OD estimation statistics

extern float g_TotalDemandDeviation; 
extern float g_TotalMeasurementDeviation; 

extern ofstream g_scenario_short_description;
extern ofstream ShortSimulationLogFile;








