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


extern ofstream g_WarningFile;
extern std::vector<DTANode> g_NodeVector;
extern std::map<int, int> g_NodeNametoIDMap;
extern std::vector<DTALink*> g_LinkVector;
extern std::map<string, DTALink*> g_LinkMap;
extern std::map<int, DTAZone> g_ZoneMap;
extern std::vector<int> g_ZoneNumber2NoVector;
extern std::vector<int> g_ZoneNo2NumberVector;
extern std::vector<DTAVehicleType> g_VehicleTypeVector;
extern std::vector<DTAVehicle*>		g_VehicleVector;
extern std::map<int, DTAVehicle*> g_VehicleMap;
extern std::map<int, DemandType> g_DemandTypeMap;
extern std::map<int, PricingType> g_PricingTypeMap;
extern std::map<int, DTALinkType> g_LinkTypeMap;
extern std::map<int, string> g_NodeControlTypeMap;

extern	std::vector<VOTDistribution> g_VOTDistributionVector;


extern int g_DemandLoadingStartTimeInMin;
extern int g_DemandLoadingEndTimeInMin;
extern float g_UpdatedDemandPrintOutThreshold;

extern unsigned int g_RandomSeed;

class HistoricalDemand
{

public:
	int m_ODZoneSize;
	HistoricalDemand()
	{
		m_ODZoneSize = 1;
		m_HistDemand = NULL;
	}
	~HistoricalDemand()
	{
	if(m_HistDemand!=NULL)
		{
		Deallocate3DDynamicArray(m_HistDemand,m_ODZoneSize+1,m_ODZoneSize+1);
		}
	if(m_UpdatedDemand!=NULL)
		{
		Deallocate3DDynamicArray(m_UpdatedDemand,m_ODZoneSize+1,m_ODZoneSize+1);
		}
	}

	float *** m_HistDemand;
	float *** m_UpdatedDemand;
	int m_StatisticsIntervalSize;
	int m_StartingTimeInterval;

	void Print(CString file_name)
	{
		FILE* st = fopen(file_name,"w");
		
		if(st!=NULL)
		{
			fprintf(st, "origin_zone,destination_zone,time_interval,hist_demand_value,updated_demand_value,difference, percentage_difference\n");
			for(int i= 0; i<m_ODZoneSize; i++)
			for(int j= 0; j<m_ODZoneSize; j++)
				for(int t= 0; t<=m_StatisticsIntervalSize; t++)
				{
					if(m_HistDemand[i][j][t]>= g_UpdatedDemandPrintOutThreshold)
					{
					fprintf(st, "%d,%d,%d,%.2f,%.2f,%.2f,%.1f\n",
						g_ZoneNo2NumberVector[i],g_ZoneNo2NumberVector[j],m_StartingTimeInterval/g_AggregationTimetInterval+t,
						m_HistDemand[i][j][t],m_UpdatedDemand[i][j][t],m_UpdatedDemand[i][j][t]-m_HistDemand[i][j][t],
						
						(m_UpdatedDemand[i][j][t]-m_HistDemand[i][j][t])*100 / m_HistDemand[i][j][t]);
					}
				}
		fclose(st);
		}

	
	}
	void Reset()
	{
			for(int i= 0; i<=m_ODZoneSize; i++)
			for(int j= 0; j<=m_ODZoneSize; j++)
				for(int t= 0; t<=m_StatisticsIntervalSize; t++)
				{
				m_HistDemand[i][j][t] = 0;
				m_UpdatedDemand[i][j][t] = 0;
				}

	}

	void Initialize ()
	{
		m_ODZoneSize = g_ZoneMap.size();

		m_StatisticsIntervalSize = 0;
		m_StartingTimeInterval = 0;


		if(m_HistDemand!=NULL)
		{
		Deallocate3DDynamicArray(m_HistDemand,m_ODZoneSize+1,m_ODZoneSize+1);
		}

		if(m_UpdatedDemand!=NULL)
		{
		Deallocate3DDynamicArray(m_UpdatedDemand,m_ODZoneSize+1,m_ODZoneSize+1);
		}

		m_StatisticsIntervalSize = (g_DemandLoadingEndTimeInMin - g_DemandLoadingStartTimeInMin)/g_AggregationTimetInterval;  // 15 min: department_time_intreval
		m_StartingTimeInterval = g_DemandLoadingStartTimeInMin/g_AggregationTimetInterval;

		m_HistDemand = Allocate3DDynamicArray<float>(m_ODZoneSize+1,m_ODZoneSize+1,m_StatisticsIntervalSize+2);
		m_UpdatedDemand = Allocate3DDynamicArray<float>(m_ODZoneSize+1,m_ODZoneSize+1,m_StatisticsIntervalSize+2);

		Reset();

	}
	void AddValue(int origin_zone_number, int destination_zone_number, int AssignmentInterval, float value)
	{

		if( origin_zone_number ==2 && destination_zone_number == 40)
			TRACE("");

		int origin_zone = g_ZoneNumber2NoVector[origin_zone_number];
		int destination_zone = g_ZoneNumber2NoVector[destination_zone_number];

		if(origin_zone > m_ODZoneSize || destination_zone > m_ODZoneSize || (AssignmentInterval-m_StartingTimeInterval) > m_StatisticsIntervalSize)
			return; 

		m_HistDemand[origin_zone][destination_zone][AssignmentInterval-m_StartingTimeInterval] += value;
	}


	float GetValue(int origin_zone_number, int destination_zone_number, int AssignmentInterval)
	{
		int origin_zone = g_ZoneNumber2NoVector[origin_zone_number];
		int destination_zone = g_ZoneNumber2NoVector[destination_zone_number];

		if(origin_zone > m_ODZoneSize || destination_zone > m_ODZoneSize || (AssignmentInterval-m_StartingTimeInterval) > m_StatisticsIntervalSize)
			return 0.0f; 

			return 	m_HistDemand[origin_zone][destination_zone][AssignmentInterval-m_StartingTimeInterval];

	}

	void AddUpdatedValue(int origin_zone_number, int destination_zone_number, int AssignmentInterval, float value)
	{

		int origin_zone = g_ZoneNumber2NoVector[origin_zone_number];
		int destination_zone = g_ZoneNumber2NoVector[destination_zone_number];


		if(origin_zone > m_ODZoneSize || destination_zone > m_ODZoneSize || (AssignmentInterval-m_StartingTimeInterval) > m_StatisticsIntervalSize)
			return; 

		m_UpdatedDemand[origin_zone][destination_zone][AssignmentInterval-m_StartingTimeInterval] += value;
	}

	void ResetUpdatedValue()
	{
			for(int i= 0; i<=m_ODZoneSize; i++)
			for(int j= 0; j<=m_ODZoneSize; j++)
				for(int t= 0; t<=m_StatisticsIntervalSize; t++)
				{
				m_UpdatedDemand[i][j][t] = 0;
				}

	}
	float GetUpdatedValue(int origin_zone_number, int destination_zone_number, int AssignmentInterval)
	{
		int origin_zone = g_ZoneNumber2NoVector[origin_zone_number];
		int destination_zone = g_ZoneNumber2NoVector[destination_zone_number];

		if(origin_zone > m_ODZoneSize || destination_zone > m_ODZoneSize || (AssignmentInterval-m_StartingTimeInterval) > m_StatisticsIntervalSize)
			return 0.0f; 

			return 	m_UpdatedDemand[origin_zone][destination_zone][AssignmentInterval-m_StartingTimeInterval];

	}

};

extern HistoricalDemand g_HistDemand;

extern std::vector<NetworkMOE>  g_NetworkMOEAry;

// time inteval settings in assignment and simulation
extern double g_DTASimulationInterval; // min
extern int g_number_of_intervals_per_min; // round to nearest integer
extern int g_number_of_car_following_intervals_per_min;
extern int g_AggregationTimetInterval; // min

// maximal # of adjacent links of a node (including physical nodes and centriods( with connectors))
extern int g_AdjLinkSize; // initial value of adjacent links

extern int m_ODZoneSize;

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

extern float g_TravelTimeDifferenceForSwitching;  // min
extern float g_RelativeTravelTimePercentageDifferenceForSwitching; // percentage


extern int g_StochasticCapacityMode ;
extern int g_UseFreevalRampMergeModelFlag;
extern int g_OutputLinkCapacityFlag;
extern int g_OutputLinkCapacityStarting_Time;
extern int g_OutputLinkCapacityEnding_Time;
extern int g_VehicleLoadingMode;
extern int g_MergeNodeModelFlag;
extern int g_FIFOConditionAcrossDifferentMovementFlag;
extern e_traffic_flow_model g_TrafficFlowModelFlag;
extern int g_ShortestPathWithMovementDelayFlag;
extern int g_EmissionDataOutputFlag;
extern int g_TimeDependentODMOEOutputFlag;
extern int g_VehiclePathOutputFlag;

extern int g_OutputSecondBySecondEmissionData;
extern float g_OutputSecondBySecondEmissionDataPercentage;
extern int g_EmissionSmoothVehicleTrajectory;
extern int g_start_departure_time_in_min_for_output_second_by_second_emission_data;
extern int g_end_departure_time_in_min_for_output_second_by_second_emission_data;
extern int g_OutputEmissionOperatingModeData;
extern int g_TargetVehicleID_OutputSecondBySecondEmissionData;
extern int g_TollingMethodFlag;
extern float g_VMTTollingRate;

// for traffic assignment 
extern e_assignment_method g_UEAssignmentMethod; // 0: MSA, 1: day-to-day learning with fixed switch rate 2: GAP-based switching rule for UE, 3: Gap-based switching rule + MSA step size for UE, 4: departure time choice
extern float g_FreewayBiasFactor; // 1: default value, 0.9 travel time on freeway will be weighted less

extern int g_Day2DayAgentLearningMethod; // 0: no learning (use previous day), 1: route choice learning only, 2: route choice and departure time learning
extern float g_DepartureTimeChoiceEarlyDelayPenalty, g_DepartureTimeChoiceLateDelayPenalty;
extern float g_CurrentGapValue; // total network gap value in the current iteration
extern float g_CurrentRelativeGapValue; // total  network relative gap value in the current iteration
extern float g_PercentageCompleteTrips; // total network gap value in the current iteration
extern float g_PrevRelativeGapValue; // total  network relative gap value in the current iteration

extern int g_CurrentNumOfVehiclesSwitched; // total number of vehicles switching paths in the current iteration; for MSA, g_UEAssignmentMethod = 0
extern int g_CurrentNumOfVehiclesForUEGapCalculation; // total number of vehicles for gap functions in the current iteration; for MSA, g_UEAssignmentMethod = 0
extern int g_PrevNumOfVehiclesSwitched; // // total number of vehicles switching paths in last iteration; for MSA, g_UEAssignmentMethod = 0
extern int g_ConvergenceThreshold_in_Num_Switch; // the convergence threshold in terms of number of vehicles switching paths; for MSA, g_UEAssignmentMethod = 0
extern int g_VehicleExperiencedTimeGap; // 1: Vehicle experienced time gap; 0: Avg experienced path time gap
extern int g_NewPathWithSwitchedVehicles; // number of new paths with vehicles switched to them
extern int g_ValidationDataStartTimeInMin;
extern int g_ValidationDataEndTimeInMin;
// for OD estimation statistics

extern float g_TotalDemandDeviation; 
extern float g_TotalMeasurementDeviation; 


extern ofstream g_scenario_short_description;
extern void OutputMovementMOEData(ofstream &output_MovementMOE_file);
extern bool g_floating_point_value_less_than_or_eq_comparison(double value1, double value2);
extern bool g_floating_point_value_less_than(double value1, double value2);
extern string GetMovementStringID(int FromNodeName, int ToNodeName, int DestNodeName);
extern void g_ReadDemandFileBasedOnMetaDatabase();
extern void g_ReadTimeDependentDemandProfile();
extern void g_ReadVOTProfile();
extern float g_GetRandomRatioForVehicleGeneration();
extern struc_LinearRegressionResult LeastRegression(std::vector <SensorDataPoint> &DataVector, bool bSetYInterceptTo0 = true);
extern std::string GetTimeClockString(int time);






