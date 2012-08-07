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

#pragma once

#pragma warning(disable:4244)  // stop warning: "conversion from 'int' to 'float', possible loss of data"

#include "resource.h"

#include <math.h>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <list>
#include "CSVParser.h"
using namespace std;

// extention for multi-day equilibirum
#define MAX_FIFO_QUEUESIZE 5000
#define MAX_DAY_SIZE 1
#define MAX_PATH_LINK_SIZE 1000
#define MAX_MEASUREMENT_INTERVAL 15 

#define MAX_INFO_CLASS_SIZE 4
#define MAX_VEHICLE_TYPE_SIZE 10
#define MAX_PRICING_TYPE_SIZE 5  // LOV, HOV and Truck, intermodal: 5 = 4+ 1 , starts from 1 
#define MAX_TIME_INTERVAL_SIZE 96

#define MAX_SIZE_INFO_USERS 5 
#define MAX_VOT_RANGE 101
#define DEFAULT_VOT 12



enum SPEED_BIN {VSP_0_25mph=0,VSP_25_50mph,VSP_GT50mph};
enum VSP_BIN {VSP_LT0=0,VSP_0_3,VSP_3_6,VSP_6_9,VSP_9_12,VSP_GT12,VSP_6_12,VSP_LT6};


enum Traffic_State {FreeFlow,PartiallyCongested,FullyCongested};

enum Traffic_MOE {MOE_crashes,MOE_CO2, MOE_total_energy};

enum Tolling_Method {no_toll,time_dependent_toll,VMT_toll,SO_toll};
extern double g_DTASimulationInterval;
extern double g_CarFollowingSimulationInterval;

#define	MAX_SPLABEL 99999.0f
#define MAX_TIME_INTERVAL_ADCURVE 300  // 300 simulation intervals of data are stored to keep tract cummulative flow counts of each link
extern int g_AggregationTimetInterval;
extern float g_MinimumInFlowRatio;
extern float g_MaxDensityRatioForVehicleLoading;
extern int g_CycleLength_in_seconds;

extern float g_DefaultSaturationFlowRate_in_vehphpl;


#ifdef _WIN64
#define MAX_LINK_NO 99999999
#endif 

#ifndef _WIN64
#define MAX_LINK_NO 65530
#endif

#define MAX_NODE_SIZE_IN_A_PATH 2000
#define MAX_LINK_SIZE_IN_VMS 20

#define MAX_CPU_SIZE 20
// Linear congruential generator 
#define LCG_a 17364
#define LCG_c 0
#define LCG_M 65521  // it should be 2^32, but we use a small 16-bit number to save memory

void g_ProgramStop();
float g_RNNOF();
bool g_GetVehicleAttributes(int demand_type, int &VehicleType, int &PricingType, int &InformationClass, float &VOT);

string GetLinkStringID(int FromNodeName, int ToNodeName);

struct GDPoint
{
	double x;
	double y;
};
struct VehicleCFData
{
	int   VehicleID;
	int   LaneNo;
	int   SequentialLinkNo;
	float FreeflowDistance_per_SimulationInterval;  
	float   CriticalSpacing_in_meter;

	int StartTime_in_SimulationInterval; // in time interval, LinkStartTime, so it should be sorted
	int EndTime_in_SimulationInterval; // in time interval
	int   TimeLag_in_SimulationInterval;
};
class LinkMOEStatisticsData
{

public:
	int SOV_volume, HOV_volume, Truck_volume,Intermodal_volume;
	float number_of_crashes_per_year, number_of_fatal_and_injury_crashes_per_year,number_of_property_damage_only_crashes_per_year;
	char level_of_service;
	LinkMOEStatisticsData()
	{
		Init();
	}
	void Init()
	{
		level_of_service = 'A';
		SOV_volume = HOV_volume =  Truck_volume = Intermodal_volume = 0;
		number_of_crashes_per_year = number_of_fatal_and_injury_crashes_per_year = number_of_property_damage_only_crashes_per_year = 0;

	}

};
class EmissionStatisticsData
{
public: 

	float TotalEnergy;
	float TotalCO2;
	float TotalNOX;
	float TotalCO;
	float TotalHC;

	float AvgEnergy;
	float AvgCO2;
	float AvgNOX;
	float AvgCO;
	float AvgHC;

	EmissionStatisticsData()
	{
		Init();
	}
	void Init()
	{
		TotalEnergy = 0;
		TotalCO2 = 0;
		TotalNOX = 0;
		TotalCO = 0;
		TotalHC = 0;

		AvgEnergy = 0;
		AvgCO2  = 0;
		AvgNOX  = 0;
		AvgCO = 0;
		AvgHC = 0;
	}

};

class EmissionLaneData
{
public: 

	float Energy;
	float CO2;
	float NOX;
	float CO;
	float HC;
	EmissionLaneData()
	{
		Energy = 0;
		CO2 = 0;
		NOX = 0;
		CO = 0;
		HC = 0;
	}

};
class LaneVehicleCFData
{

public:
	LaneVehicleCFData(int SimulationHorizonInMin)
	{
		m_LaneEmissionVector.resize(SimulationHorizonInMin);
	};
	std::vector<EmissionLaneData> m_LaneEmissionVector;
	std::vector<VehicleCFData> LaneData;
};

class VOTDistribution
{
public:
	int demand_type;
	int VOT;
	float percentage;
	float cumulative_percentage_LB;
	float cumulative_percentage_UB;

};

class DTAActivityLocation
{ public: 
DTAActivityLocation()
{
	ExternalODFlag = 0;
}

int ZoneID;
int NodeID;
int ExternalODFlag;

};
class TimeOfDayDemand
{
public: 
	std::map<int, float> ValueMap;
	void SetValue(int AssignmentInterval, float value)
	{
		ValueMap[AssignmentInterval] = value;
	}

	void AddValue(int AssignmentInterval, float value)
	{
		if(ValueMap.find(AssignmentInterval)!= ValueMap.end())
		{
			ValueMap[AssignmentInterval] +=value;
		}else
		{
			ValueMap[AssignmentInterval] = value;

		}
	}

	float GetValue(int AssignmentInterval)
	{
		if(ValueMap.find(AssignmentInterval)!= ValueMap.end())
		{
			return ValueMap[AssignmentInterval];
		}else
			return 0;

	}

};

class DTAZone
{ 
public:
	std::map<int, TimeOfDayDemand> m_HistDemand;  // key destination zone, value, demand value

	int m_OriginVehicleSize;  // number of vehicles from this origin, for fast acessing
	std::vector<int> m_OriginActivityVector;
	std::vector<int> m_DestinationActivityVector;

	int GetRandomOriginNodeIDInZone(float random_ratio)
	{
		int ArraySize  = m_OriginActivityVector.size();

		int node_index = int(random_ratio*ArraySize);

		if(node_index >= ArraySize)
			node_index = ArraySize -1;

		ASSERT(node_index < ArraySize && ArraySize != 0);
		return m_OriginActivityVector[node_index];
	}

	int GetRandomDestinationIDInZone(float random_ratio)
	{
		int ArraySize  = m_DestinationActivityVector.size();

		int node_index = int(random_ratio*ArraySize);

		if(node_index >= ArraySize)
			node_index = ArraySize -1;

		if(node_index>=0)
			return m_DestinationActivityVector[node_index];
		else
			return 0;
	}

	DTAZone()
	{
		m_Capacity  =0;
		m_Demand = 0;
		m_OriginVehicleSize = 0;
	}
	~DTAZone()
	{
		m_HistDemand.clear();
		m_OriginActivityVector.clear();
		m_DestinationActivityVector.clear();
	}

	float m_Capacity;
	float m_Demand;

};

class DTADestination
{
public:
	int record_id;
	int destination_number;
	int destination_node_index;
	float destination_node_cost_label;
};
class DTANode
{
public:
	DTANode()
	{
		m_ControlType = 0;
		m_ZoneID = 0;
		m_TotalCapacity = 0;

		m_bOriginFlag = false;
		m_bDestinationFlag = false;

	};
	~DTANode()
	{
		m_DestinationVector.clear();
	};
	int m_NodeID;
	int m_NodeName;
	int m_ZoneID;  // If ZoneID > 0 --> centriod,  otherwise a physical node.
	int m_ControlType; // Type: ....
	float m_TotalCapacity;

	std::vector<DTADestination> m_DestinationVector;
	std::vector<int> m_IncomingLinkVector;


	bool m_bOriginFlag;
	bool m_bDestinationFlag;


};

// event structure in this "event-basd" traffic simulation
typedef struct{
	int veh_id;
	int veh_car_following_no;
	float time_stamp;

}struc_vehicle_item;

class Day2DayLinkMOE
{
public:
	float m_NumberOfCrashes;
	float m_NumberOfFatalAndInjuryCrashes;
	float m_NumberOfPDOCrashes;
	float AvgSpeed;
	float AvgTravelTime;
	int TotalFlowCount;

	int CumulativeArrivalCount_PricingType[MAX_PRICING_TYPE_SIZE];

	float CumulativeRevenue_PricingType[MAX_PRICING_TYPE_SIZE];

	Day2DayLinkMOE()
	{
		m_NumberOfCrashes = 0;
		m_NumberOfFatalAndInjuryCrashes = 0;
		m_NumberOfPDOCrashes = 0;
		AvgSpeed = 0;
		AvgTravelTime = 0;
		TotalFlowCount = 0;

		for(int i = 1; i < MAX_PRICING_TYPE_SIZE; i++)
		{
			CumulativeArrivalCount_PricingType[i] = 0;
			CumulativeRevenue_PricingType[i] = 0;
		}
	}

};

class SLinkMOE  // time-dependent link MOE
{
public:

	float Energy;
	float CO2;
	float NOX;
	float CO;
	float HC;
	float TotalTravelTime;   // cumulative travel time for vehicles departing at this time interval
	int TotalFlowCount;
	int CumulativeArrivalCount_PricingType[MAX_PRICING_TYPE_SIZE];
	float CumulativeRevenue_PricingType[MAX_PRICING_TYPE_SIZE];

	int CumulativeArrivalCount; 
	int CumulativeDepartureCount;
	int ExitQueueLength;
	int EndTimeOfPartialCongestion;  // time in min to the end of partial congestion
	int TrafficStateCode;  // 0: free-flow: 1: partial congested: 2: fully congested

	//   Density can be derived from CumulativeArrivalCount and CumulativeDepartureCount
	//   Flow can be derived from CumulativeDepartureCount
	//   AvgTravel time can be derived from CumulativeArrivalCount and TotalTravelTime

	SLinkMOE()
	{
		Energy = 0;
		CO2 = 0;
		NOX = 0;
		CO = 0;
		HC = 0;

		TotalTravelTime = 0;
		TotalFlowCount = 0;
		CumulativeArrivalCount  = 0;
		CumulativeDepartureCount = 0;
		ExitQueueLength = 0;
		EndTimeOfPartialCongestion = 0;
		TrafficStateCode = 0;  // free-flow

		for(int i = 1; i < MAX_PRICING_TYPE_SIZE; i++)
		{
			CumulativeArrivalCount_PricingType[i] = 0;
			CumulativeRevenue_PricingType[i] = 0;
		}

	};

	void SetupMOE()
	{
		Energy = 0;
		CO2 = 0;
		NOX = 0;
		CO = 0;
		HC = 0;

		TotalTravelTime = 0;
		TotalFlowCount = 0;
		CumulativeArrivalCount  = 0;
		CumulativeDepartureCount = 0;
		ExitQueueLength = 0;
		EndTimeOfPartialCongestion = 0;
		TrafficStateCode = 0;  // free-flow
		for(int i = 1; i < MAX_PRICING_TYPE_SIZE; i++)
		{
			CumulativeArrivalCount_PricingType[i] = 0;
			CumulativeRevenue_PricingType[i] = 0;
		}


	}

} ;

class SLinkMeasurement  // time-dependent link measurement
{
public:
	int StartTime;
	int EndTime;

	// observed
	float ObsFlowCount;
	float ObsNumberOfVehicles;  // converted from density
	float ObsTravelTime;   // converted from speed

	// simulated
	float SimuFlowCount;
	float SimuNumberOfVehicles;  // density
	float SimuTravelTime;   // departure time based

	// Deviation
	float DeviationOfFlowCount;
	float DeviationOfNumberOfVehicles;  // density
	float DeviationOfTravelTime;   // departure time based

	SLinkMeasurement()
	{
		ObsFlowCount = 0;
		ObsNumberOfVehicles = 0;
		ObsTravelTime = 0;

		SimuFlowCount = 0;
		SimuNumberOfVehicles = 0;
		SimuTravelTime =0;

		// error
		DeviationOfFlowCount = 0;
		DeviationOfNumberOfVehicles = 0;
		DeviationOfTravelTime = 0;

	}
};


class MergeIncomingLink
{
public:
	MergeIncomingLink()
	{
		m_LinkInCapacityRatio = 0;
	};
	int m_LinkID;
	int m_link_type;
	int m_NumLanes;
	float m_LinkInCapacityRatio;
};


class CapacityReduction
{
public:
	int StartDayNo;
	int EndDayNo;
	int StartTime;  // use integer
	int EndTime;    // use integer
	float LaneClosureRatio;
	float SpeedLimit;
};

class MessageSign
{
public:
	int StartDayNo;
	int EndDayNo;
	float StartTime;
	float EndTime;
	float ResponsePercentage;
	int	  Type;  // Type 1: warning. Type 2: Detour
	int   BestPathFlag;
	int   DetourLinkSize;
	int   DetourLinkArray[MAX_LINK_SIZE_IN_VMS];

	int* NodePredAry; 
	int* LinkNoAry;
	int NodeSize;
	int DownstreamNodeID;
	void UpdateNodePredAry(int DayNo, int CurTime);

	void Initialize(int node_size, int node_pred_ary[],int link_no_ary[])
	{
		if(NodePredAry ==NULL)
		{
			NodeSize = node_size;
			NodePredAry =  new int[node_size];
			LinkNoAry = new int[node_size];;

		}

		for(int i=0; i< node_size; i++)
		{
			NodePredAry[i] = node_pred_ary[i];
			LinkNoAry[i]= link_no_ary[i];
		}

	}

	MessageSign()
	{
		NodePredAry = NULL;
		LinkNoAry = NULL;
	}

	~MessageSign()
	{
		if(NodePredAry !=NULL)
			delete NodePredAry;

		if(LinkNoAry !=NULL)
			delete LinkNoAry;

	}

};

class Toll
{
public:
	int DayNo;
	float StartTime;
	float EndTime;
	float TollRate[MAX_PRICING_TYPE_SIZE];  // 4 is 3_+1 , as pricing 

	Toll()
	{
		for(int vt = 1; vt<MAX_PRICING_TYPE_SIZE; vt++)
			TollRate[vt]=0;
	}
};


class DTALink
{
public:
	DTALink(int TimeSize)  // TimeSize's unit: per min
	{

		TotalEnergy  = 0;
		TotalCO2  = 0;
		TotalNOX  = 0;
		TotalCO  = 0;
		TotalHC  = 0;

		CurrentSequenceNoForVechileDistanceAry = 0;
		CycleSizeForVechileDistanceAry = 0;
		VechileDistanceAry = NULL;
		FIFO_queue_acutal_max_size  = 0;
		FIFO_queue_max_size = 100;

		FIFOQueue  = new struc_vehicle_item[FIFO_queue_max_size];


		m_ObservedFlowVolume = 0;
		m_FlowMeasurementError = 0;
		m_AADT = 0;
		m_bSensorData = false;
		m_NumberOfCrashes = 0;
		m_NumberOfFatalAndInjuryCrashes = 0;
		m_NumberOfPDOCrashes = 0;

		m_Num_Driveways_Per_Mile = 20;
		m_volume_proportion_on_minor_leg = 0.1;
		m_Num_3SG_Intersections = 1;
		m_Num_3ST_Intersections = 1;
		m_Num_4SG_Intersections = 3;
		m_Num_4ST_Intersections = 0;

		m_SimulationHorizon	= TimeSize;
		m_LinkMOEAry.resize(m_SimulationHorizon+1);

		m_CumuArrivalFlow.resize(MAX_TIME_INTERVAL_ADCURVE+1);         // for cummulative flow counts: unit is per simulation time interval. e.g. 6 seconds 
		m_CumuDeparturelFlow.resize(MAX_TIME_INTERVAL_ADCURVE+1);      // TimeSize  (unit: min), TimeSize*10 = 0.1 min: number of simulation time intervals

		m_StochaticCapcityFlag = 0;
		m_BPRLinkVolume = 0;
		m_BPRLinkTravelTime = 0;
		m_bMergeFlag = 0;
		m_MergeOnrampLinkID = -1;
		m_MergeMainlineLinkID = -1;
		m_TollSize = 0;
		pTollVector = NULL;

	};

	void ResizeData(int TimeSize)  // TimeSize's unit: per min
	{
		m_SimulationHorizon	= TimeSize;
		m_LinkMOEAry.resize(m_SimulationHorizon+1);
		m_LinkMeasurementAry.clear();

	}

	bool 	GetImpactedFlag(int DayNo=0, int DepartureTime = -1)
	{
		for(unsigned int il = 0; il< CapacityReductionVector.size(); il++)
		{
			if((CapacityReductionVector[il].StartDayNo  <=DayNo && DayNo <= CapacityReductionVector[il].EndDayNo ) && (DepartureTime >= CapacityReductionVector[il].StartTime && DepartureTime<=CapacityReductionVector[il].EndTime + 60))  // 60 impacted after capacity reduction
			{
				return true;
			}
		}

		return false;
	}


	float GetHourlyPerLaneCapacity(int Time=-1)
	{
		return m_MaximumServiceFlowRatePHPL;
	}

	float GetNumLanes(int DayNo=0, int Time=-1)  // with lane closure
	{
		for(unsigned int il = 0; il< CapacityReductionVector.size(); il++)
		{
			if( (CapacityReductionVector[il].StartDayNo  <=DayNo && DayNo <= CapacityReductionVector[il].EndDayNo ) && (Time>= CapacityReductionVector[il].StartTime && Time<=CapacityReductionVector[il].EndTime))
			{
				return (1-CapacityReductionVector[il].LaneClosureRatio)*m_NumLanes;
			}
		}

		return (float)m_NumLanes;

	}

	int GetInformationResponseID(int DayNo=0, float Time=-1)  // from information signs
	{
		for(unsigned int il = 0; il< MessageSignVector.size(); il++)
		{
			if((MessageSignVector[il].StartDayNo  <=DayNo && DayNo <= MessageSignVector[il].EndDayNo )
				&&(Time>=MessageSignVector[il].StartTime && Time<=MessageSignVector[il].EndTime))
			{
				return il;
			}
		}

		return -1;

	}

	float GetTollRateInDollar(int DayNo, float Time, int PricingType)  
	{
		for(int il = 0; il< m_TollSize; il++)
		{
			if(( pTollVector[il].DayNo ==0 || DayNo == pTollVector[il].DayNo) && (Time >= pTollVector[il].StartTime && Time<=pTollVector[il].EndTime))
			{
				return pTollVector[il].TollRate[PricingType];
			}
		}
		return 0;
	}

	std::map<int, int> m_OperatingModeCount;
	std::vector<GDPoint> m_ShapePoints;
	std::vector <SLinkMOE> m_LinkMOEAry;

	float TotalEnergy;
	float TotalCO2;
	float TotalNOX;
	float TotalCO;
	float TotalHC;

	std::vector<LaneVehicleCFData> m_VehicleDataVector;   

	void ComputeVSP();
	//	void ComputeVSP_FastMethod();
	std::vector <SLinkMeasurement> m_LinkMeasurementAry;

	bool ContainFlowCount(float timestamp)
	{
		for(unsigned i = 0; i< m_LinkMeasurementAry.size(); i++)
		{
			if(m_LinkMeasurementAry[i].StartTime <= timestamp && timestamp <= m_LinkMeasurementAry[i].EndTime && m_LinkMeasurementAry[i].ObsFlowCount >=1 )
			{

				return true;
			}

		}

		return false;
	}

	int GetDeviationOfFlowCount(float timestamp)
	{
		for(unsigned i = 0; i< m_LinkMeasurementAry.size(); i++)
		{
			if(m_LinkMeasurementAry[i].StartTime <= timestamp && timestamp <= m_LinkMeasurementAry[i].EndTime && m_LinkMeasurementAry[i].ObsFlowCount >=1 )
			{

				return m_LinkMeasurementAry[i].DeviationOfFlowCount ;
			}

		}
		return 0;

	}

	int GetObsFlowCount(float timestamp)
	{
		for(unsigned i = 0; i< m_LinkMeasurementAry.size(); i++)
		{
			if(m_LinkMeasurementAry[i].StartTime <= timestamp && timestamp <= m_LinkMeasurementAry[i].EndTime && m_LinkMeasurementAry[i].ObsFlowCount >=1 )
			{

				return m_LinkMeasurementAry[i].ObsFlowCount; 
			}

		}
		return 0;

	}


	int GetSimulatedFlowCount(float timestamp)
	{
		for(unsigned i = 0; i< m_LinkMeasurementAry.size(); i++)
		{
			if(m_LinkMeasurementAry[i].StartTime <= timestamp && timestamp <= m_LinkMeasurementAry[i].EndTime && m_LinkMeasurementAry[i].ObsFlowCount >=1 )
			{

				int SimulatedFlowCount = m_LinkMOEAry[m_LinkMeasurementAry[i].EndTime].CumulativeArrivalCount - m_LinkMOEAry[m_LinkMeasurementAry[i].StartTime ].CumulativeArrivalCount; 

				return SimulatedFlowCount; 
			}

		}
		return 0;

	}

	std::vector<Day2DayLinkMOE> m_Day2DayLinkMOEVector;
	std::vector <int> m_CumuArrivalFlow;
	std::vector <int> m_CumuDeparturelFlow;

	std::vector<CapacityReduction> CapacityReductionVector;
	std::vector<MessageSign> MessageSignVector;
	std::vector<Toll> TollVector;

	int m_TollSize;
	Toll *pTollVector;  // not using SLT here to avoid issues with OpenMP

	int m_bMergeFlag;  // 1: freeway and freeway merge, 2: freeway and ramp merge
	std::vector<MergeIncomingLink> MergeIncomingLinkVector;
	int m_MergeOnrampLinkID;
	int m_MergeMainlineLinkID;

	std::list<struc_vehicle_item> LoadingBuffer;  //loading buffer of each link, to prevent grid lock

	std::list<struc_vehicle_item> EntranceQueue;  //link-in queue  of each link

	struc_vehicle_item EntranceBuffer[1000];  //link-in buffer  of each link, controlled by vehicle loading and transferring parts

	struc_vehicle_item* FIFOQueue;   // implementation through a cycle 
	int FIFO_front;
	int FIFO_end;
	int FIFO_queue_size;  // time-dependent size per simulation time interval
	int FIFO_queue_max_size;   // for memory allocation

	int FIFO_queue_acutal_max_size; // for car following simulation, across all simulation time intervals



	void FIFOQueue_init()
	{
		FIFO_front = 0;
		FIFO_end = 0;
		FIFO_queue_size = 0;

		if(FIFOQueue!=NULL)
		{

			delete FIFOQueue;

			FIFO_queue_max_size = 100;
			FIFOQueue  = new struc_vehicle_item[FIFO_queue_max_size];


		}

	}


	void FIFO_queue_check_size()
	{
		if(FIFO_queue_size >= FIFO_queue_max_size-10)
		{

			cout << "FIFO_queue_size>=MAX_FIFO_QUEUESIZE " << FIFO_queue_max_size;

			struc_vehicle_item* FIFOQueueTemp;   // implementation through a cycle 

			FIFOQueueTemp = new struc_vehicle_item[FIFO_queue_max_size*2];  // increase size


			for(int new_i = 0; new_i < FIFO_queue_size; new_i++)
			{
				int index = FIFO_front%(FIFO_queue_max_size-1);  // (FIFO_queue_max_size-1 is old cycle length
				FIFOQueueTemp[new_i] = FIFOQueue[index]; // copy old content

				FIFO_front = (FIFO_front+1)%(FIFO_queue_max_size-1); // move forward

			}

			//reset  front and end pointers
			FIFO_front = 0;
			FIFO_end = FIFO_queue_size;

			if(FIFOQueue)  // delete old pointer
				delete FIFOQueue;

			FIFOQueue = FIFOQueueTemp;  // copy pointer

			FIFO_queue_max_size  = FIFO_queue_max_size*2;  // increase actual size

		}
	}

	void FIFOQueue_push_back(struc_vehicle_item item)
	{
		FIFO_queue_check_size();

		if(CycleSizeForVechileDistanceAry>=5)  // car following simulation
		{
			// assign car following sequence no in the distance array
			item.veh_car_following_no = CurrentSequenceNoForVechileDistanceAry%CycleSizeForVechileDistanceAry;
			//		VechileDistanceAry[item.veh_car_following_no][i] = 0.0f; // initialize the starting position
			CurrentSequenceNoForVechileDistanceAry++;
		}

		FIFOQueue[FIFO_end] = item;
		FIFO_end = (FIFO_end+1)%(FIFO_queue_max_size-1); // move forward
		FIFO_queue_size ++;

		if(FIFO_queue_size > FIFO_queue_acutal_max_size)
		{
			FIFO_queue_acutal_max_size = FIFO_queue_size;  // update acutal max # of vehicles on a link at any given time
		}

	}

	struc_vehicle_item FIFOQueue_pop_front()
	{
		FIFO_queue_check_size();

		FIFOQueue[FIFO_front].veh_car_following_no  = -1;  // reset
		struc_vehicle_item item = FIFOQueue[FIFO_front];
		FIFO_front = (FIFO_front+1)%(FIFO_queue_max_size-1); // move forward
		FIFO_queue_size --;
		return item;
	}

	int EntranceBufferSize;
	int NewVehicleCount;
	int ExitVehicleCount;

	float **VechileDistanceAry;
	int CurrentSequenceNoForVechileDistanceAry;  // start from 0, 
	int CycleSizeForVechileDistanceAry; // cycle size

	std::list<struc_vehicle_item> ExitQueue;      // link-out queue of each link

	int m_LinkID;
	int m_OrgLinkID;    //original link id from input_link.csv file
	int m_FromNodeID;  // index starting from 0
	int m_ToNodeID;    // index starting from 0
	int m_link_code; //1: AB, 2: BA

	std::vector <int> m_InboundLinkWithoutUTurnVector;
	std::vector <int> m_OutboundLinkWithoutUTurnVector;


	int m_FromNodeNumber;
	int m_ToNodeNumber;

	float	m_Length;  // in miles
	float   m_VehicleSpaceCapacity; // in vehicles
	int	m_NumLanes;
	float	m_SpeedLimit;
	float m_KJam;
	float m_AADTConversionFactor;
	float m_BackwardWaveSpeed; // unit: mile/ hour
	float	m_MaximumServiceFlowRatePHPL;  //Capacity used in BPR for each link, reduced due to link type and other factors.

	int  m_StochaticCapcityFlag;  // 0: deterministic cacpty, 1: lane drop. 2: merge, 3: weaving
	// optional for display only
	int	m_link_type;

	// for MOE data array
	int m_SimulationHorizon;


	// traffic flow propagation
	float m_FreeFlowTravelTime; // min
	int m_BackwardWaveTimeInSimulationInterval; // simulation time interval

	float m_BPRLinkVolume;
	float m_BPRLinkTravelTime;

	//  multi-day equilibirum: travel time for stochastic capacity
	float m_BPRLaneCapacity;
	float m_DayDependentCapacity[MAX_DAY_SIZE];



	~DTALink()
	{
		if(FIFOQueue!=NULL)
			delete FIFOQueue;

		if(LoadingBufferVector!=NULL && LoadingBufferSize>=1)
			delete LoadingBufferVector;

		if(pTollVector)
			delete pTollVector;

		LoadingBuffer.clear();
		EntranceQueue.clear();
		ExitQueue.clear();
		MergeIncomingLinkVector.clear();

		m_ShapePoints.clear();
		m_LinkMOEAry.clear();
		m_LinkMeasurementAry.clear();
		m_VehicleDataVector.clear();

		m_CumuArrivalFlow.clear();
		m_CumuDeparturelFlow.clear();

		CapacityReductionVector.clear();
		MessageSignVector.clear();
		TollVector.clear();


	};

	float GetFreeMovingTravelTime(int TrafficModelFlag = 2, float Time = -1)
	{
		if(TrafficModelFlag == 0) // BRP model
			return m_BPRLinkTravelTime;
		else 
		{
			for(unsigned int il = 0; il< CapacityReductionVector.size(); il++)
			{
				if(Time>=CapacityReductionVector[il].StartTime && Time<=CapacityReductionVector[il].EndTime)
				{
					return m_Length/max(1,CapacityReductionVector[il].SpeedLimit)*60.0f;  // convert from hour to min;
				}
			}
			return m_FreeFlowTravelTime;
		}
	}

	float GetHistoricalTravelTime(int Time = -1)
	{	// default value for now
		return m_BPRLinkTravelTime;
	}

	void SetupMOE()
	{
		m_JamTimeStamp = (float) m_SimulationHorizon;
		m_FreeFlowTravelTime = m_Length/m_SpeedLimit*60.0f;  // convert from hour to min
		m_BPRLinkVolume = 0;
		m_BPRLinkTravelTime = m_FreeFlowTravelTime;
		m_FFTT_simulation_interval = int(m_FreeFlowTravelTime/g_DTASimulationInterval);
		LoadingBufferVector = NULL;

		LoadingBufferSize = 0;
		FIFOQueue_init();

		m_BackwardWaveTimeInSimulationInterval = int(m_Length/m_BackwardWaveSpeed*60/g_DTASimulationInterval); // assume backwave speed is 20 mph, 600 conversts hour to simulation intervals

		CFlowArrivalCount = 0;

		for(int pt = 1; pt < MAX_PRICING_TYPE_SIZE; pt++)
		{
			CFlowArrivalCount_PricingType[pt] = 0;
			CFlowArrivalRevenue_PricingType[pt] = 0;
		}

		for(int t=0; t<MAX_TIME_INTERVAL_ADCURVE; t++)
		{
			A[t] = 0;
			D[t] = 0;
		}

		StartIndexOfLoadingBuffer = 0;
		LoadingBufferVector = NULL;

		CFlowDepartureCount = 0;

		LinkOutCapacity = 0;
		LinkInCapacity = 0;

		VehicleCount = 0;

		int t;


		for(t=0; t< m_CumuArrivalFlow.size(); t++)
		{
			m_CumuArrivalFlow[t] = 0;
			m_CumuDeparturelFlow[t] = 0;

		}

		for(t=0; t<=m_SimulationHorizon; t++)
		{
			m_LinkMOEAry[t].SetupMOE();
		}
		LoadingBuffer.clear();
		EntranceQueue.clear();
		ExitQueue.clear();


	}

	void InitializeDayDependentCapacity()
	{
		for(int day = 0; day < MAX_DAY_SIZE; day ++)
		{
			float TotalCapacity =0;
			int number_of_days = 1;
			for(int t = 0; t< number_of_days; t++)
			{
				float Normal = g_RNNOF(); // call random number anyway, used in with queue and without queue cases
				float Shift  = 1.5f;
				float XM    = -0.97f;
				float S     = 0.68f;

				float Headway = exp(Normal*S+XM)+Shift;

				if(Headway < 1.5f)
					Headway = 1.5f;

				TotalCapacity += 3600.0f / Headway;

			}

			float hourly_capacity = TotalCapacity/number_of_days;
			TRACE("Hourly Capacity %f\n", hourly_capacity);

			m_DayDependentCapacity[day] = hourly_capacity/1870*m_MaximumServiceFlowRatePHPL;
			//1820.31 is the mean capacity derived from mean headway

		}
	}


	float m_DayDependentTravelTime[MAX_DAY_SIZE];
	float m_AverageTravelTime;

	float m_JamTimeStamp;


	int CFlowArrivalCount_PricingType[MAX_PRICING_TYPE_SIZE];
	float CFlowArrivalRevenue_PricingType[MAX_PRICING_TYPE_SIZE];

	int CFlowArrivalCount;
	int CFlowDepartureCount;

	int  A[MAX_TIME_INTERVAL_ADCURVE];
	int  D[MAX_TIME_INTERVAL_ADCURVE];
	int  m_FFTT_simulation_interval;  // integral value of  FFTT, in terms of simulation  time interval

	int* LoadingBufferVector;
	int  LoadingBufferSize;


	int  StartIndexOfLoadingBuffer;


	float m_ObservedFlowVolume;
	float m_FlowMeasurementError ;
	float m_AADT;
	bool m_bSensorData;

	// safety prediction
	double m_Num_Driveways_Per_Mile;
	double m_volume_proportion_on_minor_leg;
	double m_Num_3SG_Intersections; 
	double m_Num_3ST_Intersections; 
	double m_Num_4SG_Intersections;
	double m_Num_4ST_Intersections;

	double m_NumberOfCrashes;
	double m_NumberOfFatalAndInjuryCrashes;
	double m_NumberOfPDOCrashes;



	double m_AdditionalDelayDueToCrashes;

	unsigned int LinkOutCapacity;  // unit: number of vehiles
	int LinkInCapacity;   // unit: number of vehiles

	int VehicleCount;

	int departure_count;
	float total_departure_based_travel_time;

	float GetSpeed(int time)
	{
		return m_Length/max(0.1,GetTravelTimeByMin(-1,time,1))*60.0f;  // 60.0f converts min to hour, unit of speed: mph
	}

	int GetArrivalFlow(int time)
	{
		if(time < m_SimulationHorizon-1)  // if time = m_SimulationHorizon-1, time+1 = m_SimulationHorizon  --> no data available
			return m_LinkMOEAry[time+1].CumulativeArrivalCount - m_LinkMOEAry[time].CumulativeArrivalCount;
		else
			return 0;

	};

	int GetDepartureFlow(int time)
	{
		if(time < m_SimulationHorizon-1)  // if time = m_SimulationHorizon-1, time+1 = m_SimulationHorizon  --> no data available
			return m_LinkMOEAry[time+1].CumulativeDepartureCount - m_LinkMOEAry[time].CumulativeDepartureCount;
		else
			return 0;

	};

	float GetPrevailingTravelTime(int DayNo,int CurrentTime)
	{
		if(GetNumLanes(DayNo,CurrentTime)<0.01)   // road blockage
			return 1440; // unit min

		if(departure_count >= 1 && CurrentTime >0)
		{
			return total_departure_based_travel_time/departure_count;
		}
		else { // CurrentTime ==0 or departure_count ==0 
			return  m_FreeFlowTravelTime;
		}

	};


	float GetTravelTimeByMin(int DayNo,int starting_time, int time_interval)  // DayNo =-1: unknown day
	{
		float travel_time  = 0.0f;

		if(GetNumLanes(DayNo,starting_time)<0.01)   // road blockage
			return 9999; // unit min

		ASSERT(m_SimulationHorizon < m_LinkMOEAry.size());

		int t;
		float total_travel_time = 0;
		int total_flow =0;
		int time_end = min(starting_time+time_interval, m_SimulationHorizon);
		for(t=starting_time; t< time_end; t++)
		{
			total_travel_time += m_LinkMOEAry[t].TotalTravelTime;
			total_flow +=  m_LinkMOEAry[t].TotalFlowCount ;
		}

		if(total_flow >= 1)
		{
			travel_time =  total_travel_time/total_flow;
			if(travel_time < m_FreeFlowTravelTime)
				travel_time = m_FreeFlowTravelTime; // minimum travel time constraint for shortest path calculation
		}
		else
			travel_time =  m_FreeFlowTravelTime;

		if(travel_time > 99999)
		{
			TRACE("Error, link %d", this ->m_LinkID );
			for(t=starting_time; t< time_end; t++)
			{
				TRACE("t = %d,%f\n",t,m_LinkMOEAry[t].TotalTravelTime);
			}

		}


		return travel_time;

	};


};

// link element of a vehicle path

class SVehicleLink
{  public:

#ifdef _WIN64
unsigned long  LinkID;  // range: 4294967295
#else
unsigned short  LinkID;  // range:  65535
#endif

unsigned short  LaneBasedCumulativeFlowCount;  // range:  65535
float AbsArrivalTimeOnDSN;     // absolute arrvial time at downstream node of a link: 0 for the departure time, including delay/stop time
//   float LinkWaitingTime;   // unit: 0.1 seconds
SVehicleLink()
{
	LinkID = MAX_LINK_NO;
	AbsArrivalTimeOnDSN = 99999;
	//		LinkWaitingTime = 0;
	LaneBasedCumulativeFlowCount = 0;
}

};

struct VehicleTimestampSpeed
{
	int timestamp_in_second;
	float speed;

};

class TimeDependentDemandProfile
{
public:
	int from_zone_id;
	int to_zone_id;
	int demand_type;
	double time_dependent_ratio[MAX_TIME_INTERVAL_SIZE];

	TimeDependentDemandProfile()
	{
		for(int interval =0; interval < MAX_TIME_INTERVAL_SIZE; interval++)
		{
			time_dependent_ratio[interval] = 0;
		}

	}

};

class DTAVehicleType
{
public:
	DTAVehicleType()
	{
		vehicle_type = 1;

	}
	int vehicle_type;
	string vehicle_type_name;
};


class DTALinkType
{
public:
	float default_lane_capacity;
	int link_type;
	string link_type_name;
	string type_code;
	int safety_prediction_model_id;

	DTALinkType()
	{
		default_lane_capacity = 1000;
	}
	bool IsFreeway()
	{
		if(type_code.find('f')!= string::npos)
			return true;
		else
			return false;
	}

	bool IsHighway()
	{
		if(type_code.find('h')!= string::npos)
			return true;
		else
			return false;
	}

	bool IsArterial()
	{
		if(type_code.find('a')!= string::npos)
			return true;
		else
			return false;
	}

	bool IsRamp()
	{
		if(type_code.find('r')!= string::npos)
			return true;
		else
			return false;
	}

	bool IsConnector()
	{
		if(type_code.find('c')!= string::npos)
			return true;
		else
			return false;
	}

	bool IsTransit()
	{
		if(type_code.find('t')!= string::npos)
			return true;
		else
			return false;
	}
	bool IsWalking()
	{
		if(type_code.find('w')!= string::npos)
			return true;
		else
			return false;
	}
};

class DTAPath
{
public:
	std::vector<int> LinkSequence;
	int NodeSum;
	float Distance;
	float TravelTime;
	bool bDiverted;

	DTAPath ()
	{ 
		bDiverted = false;
		Distance = 0;
		TravelTime = 0;

	}
};

class VehicleSpeedProfileData
{
public:
	int FromNodeNumber;
	int ToNodeNumber;
	float Speed;
	float Acceleration;
	int OperationMode;
	int VSP;
	VSP_BIN VSPBinNo;
	SPEED_BIN SpeedBinNo;
	float Energy,CO2,NOX,CO,HC;
	int TimeOnThisLinkInSecond;

	CString GetSpeedBinNoString()
	{
		CString str;
		switch(SpeedBinNo)
		{
		case VSP_0_25mph: str.Format ("0_25 mph"); break;
		case VSP_25_50mph: str.Format ("25_50 mph"); break;
		case VSP_GT50mph: str.Format (">=50 mph"); break;

		}
		return str;

	}

	CString GetVSPBinNoString()
	{
		CString str;
		switch(VSPBinNo)
		{
		case VSP_LT0: str.Format ("VSP_<=0"); break;
		case VSP_0_3: str.Format ("VSP_0_3"); break;
		case VSP_3_6: str.Format ("VSP_3_6"); break;
		case VSP_6_9: str.Format ("VSP_6_9"); break;
		case VSP_9_12: str.Format ("VSP_9_12"); break;
		case VSP_GT12: str.Format ("VSP_>=12"); break;
		case VSP_6_12: str.Format ("VSP_6_12"); break;
		case VSP_LT6: str.Format ("VSP_<=6"); break;

		}
		return str;

	}

};
class DTADecisionAlternative
{
public:

	DTADecisionAlternative()
	{
	total_cost = 999999;  // initial value
	node_size = 0;
	final_departuret_time_shift = 0;
	}

	float total_cost;
	int final_departuret_time_shift;
	int node_size;
	int path_link_list[MAX_NODE_SIZE_IN_A_PATH];

	void UpdateForLowerAlternativeCost(float temp_total_cost, int departure_time_shift, int temp_node_size, int temp_path_link_list[MAX_NODE_SIZE_IN_A_PATH])
	{  // by using this updating funciton, we easily add many alternatives (for mode, departure time choices)

		if(temp_total_cost < total_cost)
	{
		total_cost = temp_total_cost;
		final_departuret_time_shift = departure_time_shift;
		node_size = temp_node_size;

		for(int n = 0; n < MAX_NODE_SIZE_IN_A_PATH; n++)
			path_link_list[n] = temp_path_link_list[n];
	}
	
	}

};
class DTAVehicle
{
public:
	int m_NodeSize;
	int m_NodeNumberSum;  // used for comparing two paths
	SVehicleLink *m_aryVN; // link list arrary of a vehicle path  // to do list, change this to a STL vector for better readability

	float m_PrevSpeed;
	std::map<int, int> m_OperatingModeCount;
	std::map<int, int> m_SpeedCount;
	std::map<int, VehicleSpeedProfileData> m_SpeedProfile;

	unsigned int m_RandomSeed;
	int m_VehicleID;  //range: +2,147,483,647

	int m_OriginZoneID;  
	int m_DestinationZoneID; 

	int m_OriginNodeID;
	int m_DestinationNodeID;

	int m_DemandType;     // 1: passenger,  2, HOV, 3, truck, 3: bus
	int m_PricingType;     // 1: passenger,  2, HOV, 3, truck, 3: bus
	int m_VehicleType;    // for emissions analysis
	int m_InformationClass;  // 1: historical, 2: pre-trip, 3: en-route

	int m_SimLinkSequenceNo; //  range 0, 65535

	bool  m_bImpacted;

	int m_TimeToRetrieveInfo;  // in simulation interval
	float m_DepartureTime;
	float m_PreferredDepartureTime;
	float m_Distance;

	float m_ArrivalTime;
	float m_TripTime;
	float m_EstimatedTravelTime;
	float m_Delay;

	bool m_bSwitched;  // switch route in assignment

	// used for simulation
	bool m_bLoaded; // be loaded into the physical network or not
	bool m_bComplete;

	float Energy,CO2,NOX,CO,HC;

	// multi-day equilibrium
	bool m_bETTFlag;
	int m_DayDependentLinkSize[MAX_DAY_SIZE];
	std::map<int, int> m_DayDependentAryLink;  // first key: day*MAX_PATH_LINK_SIZE + index, second element; return link index
	float m_DayDependentTripTime[MAX_DAY_SIZE];
	int m_DayDependentNodeNumberSum[MAX_DAY_SIZE];  // used for comparing two paths
	float m_DayDependentGap[MAX_DAY_SIZE];  // used for gap analysis
	float m_AvgDayTravelTime;
	float m_DayTravelTimeSTD;

	float m_VOT;        // range 0 to 255
	float m_TollDollarCost;
	float m_Emissions;
	float m_MinCost;
	float m_MeanTravelTime;
	float m_TravelTimeVariance;
	unsigned short m_NumberOfSamples;  // when switch a new path, the number of samples starts with 0

	std::map<int,DTAPath> Day2DayPathMap;

	void StorePath(int DayNo)
	{

		Day2DayPathMap[DayNo].NodeSum = m_NodeNumberSum;

		for(int i = 0; i < m_NodeSize; i++)
			Day2DayPathMap[DayNo].LinkSequence.push_back (m_aryVN[i].LinkID );

	};

	void PostTripUpdate(float TripTime)   
	{
		float GainFactor = 0.2f;  // will use formula from Kalman Filtering, eventually

		m_MeanTravelTime = (1-GainFactor)*m_MeanTravelTime + GainFactor*TripTime;
		m_NumberOfSamples +=1;
	};

	DTAVehicle()
	{
		Energy = CO2 = NOX = CO = HC = 0;
		m_PrevSpeed = 0;
		m_TimeToRetrieveInfo = -1;
		m_SimLinkSequenceNo = 0;

		m_NumberOfSamples =0;
		m_VOT = DEFAULT_VOT;
		m_TollDollarCost = 0;
		m_Emissions = 0;

		m_MinCost = 0;

		m_aryVN = NULL;
		m_NodeSize	= 0;
		m_bImpacted = false; 
		m_InformationClass = 1;
		m_DemandType = 1;
		m_VehicleType = 1;
		m_PricingType = 0;
		m_Emissions = 0;
		m_ArrivalTime = 0;
		//      m_FinalArrivalTime = 0;
		m_bLoaded = false;
		m_bSwitched = false;
		m_bComplete = false;
		m_TripTime = 900;  // default: for incomplete vehicles, they have an extremey long trip time
		m_Distance =0;
		m_Delay = 0;

	};
	~DTAVehicle()
	{
		if(m_aryVN != NULL)
			delete m_aryVN;

		m_OperatingModeCount.clear();
		m_DayDependentAryLink.clear();

		m_SpeedCount.clear();
		m_SpeedProfile.clear();

	};

	void PreTripReset()
	{
		if(m_PricingType!=4)
		{// non transit
			m_ArrivalTime = 0;
			m_TripTime = 0;
			m_TollDollarCost = 0;
			m_Emissions = 0;
		}

		m_bLoaded = false;
		m_bComplete = false;
		m_Delay = 0;


	}

	float GetRandomRatio()
	{
		m_RandomSeed = (LCG_a * m_RandomSeed + LCG_c) % LCG_M;  //m_RandomSeed is automatically updated.

		return float(m_RandomSeed)/LCG_M;
	}

	void SetMinCost(float MinCost)
	{
		m_MinCost = MinCost;
	};

	float GetMinCost()
	{
		return m_MinCost;

	};

};

class DTA_vhc_simple // used in STL sorting only
{
public:

	int m_OriginZoneID;
	int m_DestinationZoneID;

	int m_DemandType;
	int m_VehicleType;
	int m_PricingType;
	int m_InformationClass;

	float    m_DepartureTime;
	int m_TimeToRetrieveInfo;
	float m_VOT;
	long m_PathIndex;  // for OD estimation


	bool operator<(const DTA_vhc_simple &other) const
	{
		return m_DepartureTime < other.m_DepartureTime;
	}

};

class PricingType
{
public:
	int pricing_type; // 1: SOV, 2: HOV, 3, truck;
	float default_VOT;
	string type_name;
	PricingType()
	{
		default_VOT = 10;
	};
};
class DemandType
{
public:
	int demand_type;
	float vehicle_type_percentage[MAX_VEHICLE_TYPE_SIZE];
	float cumulative_type_percentage[MAX_VEHICLE_TYPE_SIZE];

	int pricing_type; // 1: SOV, 2: HOV, 3, truck;
	float info_class_percentage[MAX_INFO_CLASS_SIZE];
	float cumulative_info_class_percentage[MAX_INFO_CLASS_SIZE];

	string demand_type_name;

	DemandType()
	{
		for(int vehicle_type =0; vehicle_type < MAX_VEHICLE_TYPE_SIZE; vehicle_type++)
		{
			vehicle_type_percentage[vehicle_type] = 0;
			cumulative_type_percentage[vehicle_type] = 0;
		}
	}

};

class DemandProfile
{
public:
	int from_zone_id;
	double time_dependent_ratio[MAX_TIME_INTERVAL_SIZE];
	int demand_type;
	CString series_name;
	DemandProfile()
	{
		for(int interval =0; interval < MAX_TIME_INTERVAL_SIZE; interval++)
		{
			time_dependent_ratio[interval] = 0;
		}

	}

};

class VehicleArrayForOriginDepartrureTimeInterval
{
public:
	std::vector<int> VehicleArray;
	~VehicleArrayForOriginDepartrureTimeInterval()
	{
		VehicleArray.clear();
	}
};

class NetworkMOE
{
public:
	int CumulativeInFlow;
	int CumulativeOutFlow;
	int Flow_in_a_min;
	float AbsArrivalTimeOnDSN_in_a_min;
	float AvgTripTime;

	NetworkMOE()
	{
		CumulativeInFlow = 0;
		CumulativeOutFlow = 0;
		Flow_in_a_min = 0;
		AbsArrivalTimeOnDSN_in_a_min = 0;
		AvgTripTime = 0;
	}
};


template <typename T>
T **AllocateDynamicArray(int nRows, int nCols)
{
	T **dynamicArray;

	dynamicArray = new T*[nRows];

	if(dynamicArray == NULL)
	{
		cout << "Error: insufficent memory.";
		g_ProgramStop();

	}

	for( int i = 0 ; i < nRows ; i++ )
	{
		dynamicArray[i] = new T [nCols];

		if (dynamicArray[i] == NULL)
		{
			cout << "Error: insufficent memory.";
			g_ProgramStop();
		}

	}

	return dynamicArray;
}

template <typename T>
void DeallocateDynamicArray(T** dArray,int nRows, int nCols)
{
	if(!dArray)
		return;

	for(int x = 0; x < nRows; x++)
	{
		delete[] dArray[x];
	}

	delete [] dArray;

}


template <typename T>
T ***Allocate3DDynamicArray(int nX, int nY, int nZ)
{
	T ***dynamicArray;

	dynamicArray = new T**[nX];

	if (dynamicArray == NULL)
	{
		cout << "Error: insufficent memory.";
		g_ProgramStop();
	}

	for( int x = 0 ; x < nX ; x++ )
	{
		dynamicArray[x] = new T* [nY];

		if (dynamicArray[x] == NULL)
		{
			cout << "Error: insufficent memory.";
			g_ProgramStop();
		}

		for( int y = 0 ; y < nY ; y++ )
		{
			dynamicArray[x][y] = new T [nZ];
			if (dynamicArray[x][y] == NULL)
			{
				cout << "Error: insufficent memory.";
				g_ProgramStop();
			}
		}
	}

	return dynamicArray;

}

template <typename T>
void Deallocate3DDynamicArray(T*** dArray, int nX, int nY)
{
	if(!dArray)
		return;
	for(int x = 0; x < nX; x++)
	{
		for(int y = 0; y < nY; y++)
		{
			delete[] dArray[x][y];
		}

		delete[] dArray[x];
	}

	delete[] dArray;

}

template <typename M> void FreeClearMap( M & amap ) 
{
	for ( typename M::iterator it = amap.begin(); it != amap.end(); ++it ) {
		delete it->second;
	}
	amap.clear();
}

class DTALinkToll
{
public:

	bool m_bTollExist ;
	float TollValue[MAX_PRICING_TYPE_SIZE];

	DTALinkToll()
	{
		m_bTollExist = false;
		for(int i=1; i< MAX_PRICING_TYPE_SIZE; i++)
			TollValue[i] = 0;
	}
};

class DTANetworkForSP  // mainly for shortest path calculation, not just physical network
	// for shortes path calculation between zone centroids, for origin zone, there are only outgoing connectors, for destination zone, only incoming connectors
	// different shortest path calculations have different network structures, depending on their origions/destinations
{
public:
	int m_NumberOfSPCalculationIntervals;
	int m_StartIntervalForShortestPathCalculation;

	int m_PlanningHorizonInMin;
	int m_StartTimeInMin;

	int m_NodeSize;
	int m_PhysicalNodeSize;

	int m_ListFront;
	int m_ListTail;
	int m_LinkSize;

	int* m_LinkList;  // dimension number of nodes

	int** m_OutboundNodeAry; //Outbound node array
	int** m_OutboundLinkAry; //Outbound link array
	int** m_OutboundConnectorZoneIDAry; //Outbound connector array
	int* m_OutboundSizeAry;  //Number of outbound links

	int** m_OutboundMovementAry; //Outbound link movement array: for each link
	int* m_OutboundMovementSizeAry;  //Number of outbound movement for each link
	float** m_OutboundMovementCostAry; //Outbound link movement array: for each link

	int** m_InboundLinkAry; //inbound link array
	int* m_InboundSizeAry;  //Number of inbound links

	int* m_FromIDAry;
	int* m_ToIDAry;

	float** m_LinkTDTimeAry;

	float** m_LinkTDTransitTimeAry;

	float*  m_LinkTDDistanceAry;
	DTALinkToll** m_LinkTDCostAry;

	int* NodeStatusAry;                // Node status array used in KSP;
	float* LabelTimeAry;               // label - time
	int* NodePredAry;  

	int** NodePredVectorPerType;  
	float** LabelCostVectorPerType;

	float* LabelCostAry;

	int* LinkNoAry;  //record link no according to NodePredAry

	// movement calculation
	int* LinkStatusAry;                // Node status array used in KSP;
	float* LinkLabelTimeAry;               // label - time
	int* LinkPredAry;  
	float* LinkLabelCostAry;


	int m_Number_of_CompletedVehicles;
	int m_AdjLinkSize;

	//below are time-dependent cost label and predecessor arrays
	float** TD_LabelCostAry;
	int** TD_NodePredAry;  // pointer to previous NODE INDEX from the current label at current node and time
	int** TD_TimePredAry;  // pointer to previous TIME INDEX from the current label at current node and time

	std::list<int> m_ScanLinkList;  // used for movement-based scanning process, use a std implementation for simplicity

	int temp_reversed_PathLinkList[MAX_NODE_SIZE_IN_A_PATH];  // tempory reversed path node list

	DTANetworkForSP()
	{};

	void Setup(int NodeSize, int LinkSize, int PlanningHorizonInMin,int AdjLinkSize, int StartTimeInMin=0)
	{
		m_NodeSize = NodeSize;
		m_LinkSize = LinkSize;

		m_PlanningHorizonInMin = PlanningHorizonInMin;
		m_StartTimeInMin = StartTimeInMin;
		m_NumberOfSPCalculationIntervals = int(m_PlanningHorizonInMin/g_AggregationTimetInterval)+1;  // make sure it is not zero
		m_StartIntervalForShortestPathCalculation = int(m_StartTimeInMin/g_AggregationTimetInterval);

		m_AdjLinkSize = AdjLinkSize;

		m_OutboundSizeAry = new int[m_NodeSize];
		m_InboundSizeAry = new int[m_NodeSize];


		m_OutboundNodeAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize+1);
		m_OutboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize+1);
		m_OutboundConnectorZoneIDAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize+1);


		m_InboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize+1);

		//movement-specific array
		m_OutboundMovementAry = AllocateDynamicArray<int>(m_LinkSize,m_AdjLinkSize+1);
		m_OutboundMovementCostAry = AllocateDynamicArray<float>(m_LinkSize,m_AdjLinkSize+1);
		m_OutboundMovementSizeAry = new int[m_LinkSize];

		m_LinkList = new int[m_NodeSize];

		m_LinkTDDistanceAry = new float[m_LinkSize];

		m_LinkTDTimeAry   =  AllocateDynamicArray<float>(m_LinkSize,m_NumberOfSPCalculationIntervals);
		m_LinkTDTransitTimeAry  =  AllocateDynamicArray<float>(m_LinkSize,m_NumberOfSPCalculationIntervals);
		m_LinkTDCostAry   =  AllocateDynamicArray<DTALinkToll>(m_LinkSize,m_NumberOfSPCalculationIntervals);

		TD_LabelCostAry =  AllocateDynamicArray<float>(m_NodeSize,m_NumberOfSPCalculationIntervals);
		TD_NodePredAry = AllocateDynamicArray<int>(m_NodeSize,m_NumberOfSPCalculationIntervals);
		TD_TimePredAry = AllocateDynamicArray<int>(m_NodeSize,m_NumberOfSPCalculationIntervals);

		m_FromIDAry = new int[m_LinkSize];

		m_ToIDAry = new int[m_LinkSize];

		NodeStatusAry = new int[m_NodeSize];                    // Node status array used in KSP;

		NodePredVectorPerType   =  AllocateDynamicArray<int>(MAX_PRICING_TYPE_SIZE,m_NodeSize);
		LabelCostVectorPerType   =  AllocateDynamicArray<float>(MAX_PRICING_TYPE_SIZE,m_NodeSize);

		NodePredAry = new int[m_NodeSize];
		LinkNoAry = new int[m_NodeSize];
		LabelTimeAry = new float[m_NodeSize];                     // label - time
		LabelCostAry = new float[m_NodeSize];                     // label - cost

		LinkStatusAry = new int[m_LinkSize];                    // Node status array used in KSP;
		LinkPredAry = new int[m_LinkSize];
		LinkLabelTimeAry = new float[m_LinkSize];                     // label - time
		LinkLabelCostAry = new float[m_LinkSize];                     // label - cost

	};
	DTANetworkForSP(int NodeSize, int LinkSize, int PlanningHorizonInMin,int AdjLinkSize, int StartTimeInMin=0){

		Setup(NodeSize, LinkSize, PlanningHorizonInMin,AdjLinkSize,StartTimeInMin=0);
	};


	~DTANetworkForSP()
	{

		if(m_OutboundSizeAry && m_NodeSize>=1)  delete m_OutboundSizeAry;
		if(m_InboundSizeAry && m_NodeSize>=1)  delete m_InboundSizeAry;

		DeallocateDynamicArray<int>(m_OutboundNodeAry,m_NodeSize, m_AdjLinkSize+1);
		DeallocateDynamicArray<int>(m_OutboundLinkAry,m_NodeSize, m_AdjLinkSize+1);
		DeallocateDynamicArray<int>(m_OutboundConnectorZoneIDAry,m_NodeSize, m_AdjLinkSize+1);

		DeallocateDynamicArray<int>(m_InboundLinkAry,m_NodeSize, m_AdjLinkSize+1);

		// delete movement array
		if(m_OutboundMovementSizeAry)  delete m_OutboundMovementSizeAry;
		DeallocateDynamicArray<int>(m_OutboundMovementAry,m_LinkSize, m_AdjLinkSize+1);
		DeallocateDynamicArray<float>(m_OutboundMovementCostAry,m_LinkSize, m_AdjLinkSize+1);


		if(m_LinkList) delete m_LinkList;

		DeallocateDynamicArray<float>(m_LinkTDTimeAry,m_LinkSize,m_NumberOfSPCalculationIntervals);
		DeallocateDynamicArray<float>(m_LinkTDTransitTimeAry,m_LinkSize,m_NumberOfSPCalculationIntervals);
		DeallocateDynamicArray<DTALinkToll>(m_LinkTDCostAry,m_LinkSize,m_NumberOfSPCalculationIntervals);
		if(m_LinkTDDistanceAry) delete m_LinkTDDistanceAry;

		DeallocateDynamicArray<float>(TD_LabelCostAry,m_NodeSize,m_NumberOfSPCalculationIntervals);
		DeallocateDynamicArray<int>(TD_NodePredAry,m_NodeSize,m_NumberOfSPCalculationIntervals);
		DeallocateDynamicArray<int>(TD_TimePredAry,m_NodeSize,m_NumberOfSPCalculationIntervals);

		if(m_FromIDAry)		delete m_FromIDAry;
		if(m_ToIDAry)	delete m_ToIDAry;

		if(NodeStatusAry) delete NodeStatusAry;                 // Node status array used in KSP;
		if(NodePredAry) delete NodePredAry;


		DeallocateDynamicArray<int>(NodePredVectorPerType,MAX_PRICING_TYPE_SIZE,m_NodeSize);
		DeallocateDynamicArray<float>(LabelCostVectorPerType,MAX_PRICING_TYPE_SIZE,m_NodeSize);

		if(LinkNoAry) delete LinkNoAry;
		if(LabelTimeAry) delete LabelTimeAry;
		if(LabelCostAry) delete LabelCostAry;

		if(LinkStatusAry) delete LinkStatusAry;                 // Node status array used in KSP;
		if(LinkPredAry) delete LinkPredAry;
		if(LinkLabelTimeAry) delete LinkLabelTimeAry;
		if(LinkLabelCostAry) delete LinkLabelCostAry;

		m_ScanLinkList.clear();

	};

	float GetTollRateInMin(int LinkID, float Time, int PricingType);  // built-in function for each network_SP to avoid conflicts with OpenMP parallel computing


	void BuildNetworkBasedOnZoneCentriod(int DayNo,int ZoneID);
	void BuildHistoricalInfoNetwork(int CurZoneID, int CurrentTime, float Perception_error_ratio);
	void BuildTravelerInfoNetwork(int DayNo,int CurrentTime, int VMSLinkID, float Perception_error_ratio);
	void BuildPhysicalNetwork(int DayNo=-1, int CurZoneID =-1);
	void IdentifyBottlenecks(int StochasticCapacityFlag);

	bool TDLabelCorrecting_DoubleQueue(int origin, int departure_time, int pricing_type, float VOT, bool bDistanceCost, bool debug_flag);   // Pointer to previous node (node)
	bool TDLabelCorrecting_DoubleQueue_PerPricingType(int origin, int departure_time, int pricing_type, float VOT, bool bDistanceCost, bool debug_flag);   // Pointer to previous node (node)

	//movement based shortest path
	int FindBestPathWithVOT_Movement(int origin, int departure_time, int destination, int pricing_type, float VOT,int PathLinkList[MAX_NODE_SIZE_IN_A_PATH],float &TotalCost, bool distance_flag, bool debug_flag);

	bool OptimalTDLabelCorrecting_DQ(int origin, int departure_time, int destination);
	int  FindOptimalSolution(int origin, int departure_time, int destination,int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]);  // the last pointer is used to get the node array;
	int  FindBestPathWithVOT(int origin_zone, int origin, int departure_time, int destination_zone, int destination, int pricing_type, float VOT,int PathLinkList[MAX_NODE_SIZE_IN_A_PATH],float &TotalCost, bool distance_flag, bool bDebugFlag = false);


	void VehicleBasedPathAssignment(int zone,int departure_time_begin, int departure_time_end, int iteration,bool debug_flag);
	void AgentBasedPathFindingAssignment(int zone,int departure_time_begin, int departure_time_end, int iteration);
	void VehicleBasedPathAssignment_ODEstimation(int zone,int departure_time_begin, int departure_time_end, int iteration);
	void HistInfoVehicleBasedPathAssignment(int zone,int departure_time_begin, int departure_time_end);

	// SEList: Scan List implementation: the reason for not using STL-like template is to avoid overhead associated pointer allocation/deallocation
	void SEList_clear()
	{
		m_ListFront= -1;
		m_ListTail= -1;
	}

	void SEList_push_front(int node)
	{
		if(m_ListFront == -1)  // start from empty
		{
			m_LinkList[node] = -1;
			m_ListFront  = node;
			m_ListTail  = node;
		}
		else
		{
			m_LinkList[node] = m_ListFront;
			m_ListFront  = node;
		}

	}
	void SEList_push_back(int node)
	{
		if(m_ListFront == -1)  // start from empty
		{
			m_ListFront = node;
			m_ListTail  = node;
			m_LinkList[node] = -1;
		}
		else
		{
			m_LinkList[m_ListTail] = node;
			m_LinkList[node] = -1;
			m_ListTail  = node;
		}
	}

	bool SEList_empty()
	{
		return(m_ListFront== -1);
	}

	int SEList_front()
	{
		return m_ListFront;
	}

	void SEList_pop_front()
	{
		int tempFront = m_ListFront;
		m_ListFront = m_LinkList[m_ListFront];
		m_LinkList[tempFront] = -1;
	}

	int  GetLinkNoByNodeIndex(int usn_index, int dsn_index);

};



int g_read_integer(FILE* f);
int g_read_integer_with_char_O(FILE* f);

float g_read_float(FILE *f);

void ReadNetworkTables();
int CreateVehicles(int originput_zone, int destination_zone, float number_of_vehicles, int demand_type, float starting_time_in_min, float ending_time_in_min,long PathIndex = -1);

void Assignment_MP(int id, int nthreads, int node_size, int link_size, int iteration);

struct NetworkLoadingOutput
{
public:
	NetworkLoadingOutput()
	{
		ResetStatistics();
	}

	void ResetStatistics ()
	{
		AvgUEGap = 0;
		TotalDemandDeviation = 0;
		LinkVolumeAvgAbsError  =0 ;
		LinkVolumeAvgAbsPercentageError  =0 ;
		LinkVolumeRootMeanSquaredError = 0;
		CorrelationBetweenObservedAndSimulatedLinkVolume = 0;

		AvgTravelTime = 0;
		AvgDelay = 0;
		AvgTTI = 0;
		AvgDistance = 0;
		NumberofVehiclesCompleteTrips = 0;
		NumberofVehiclesGenerated = 0;
		SwitchPercentage = 0;
	}
	float AvgTravelTime;
	float AvgDelay;
	float AvgTTI;
	float AvgDistance;
	int   NumberofVehiclesCompleteTrips;
	int   NumberofVehiclesGenerated;
	float SwitchPercentage;
	float AvgUEGap;
	float TotalDemandDeviation;
	float LinkVolumeAvgAbsError;
	float LinkVolumeAvgAbsPercentageError;
	float LinkVolumeRootMeanSquaredError;
	float CorrelationBetweenObservedAndSimulatedLinkVolume;
};

NetworkLoadingOutput g_NetworkLoading(int TrafficFlowModelFlag, int SimulationMode, int Iteration);  // NetworkLoadingFlag = 0: static traffic assignment, 1: vertical queue, 2: spatial queue, 3: Newell's model, 


struct PathArrayForEachODT // Jason : store the path set for each OD pair and each departure time interval
{
	//  // for path flow adjustment
	int   NumOfVehicles;
	float   DeviationNumOfVehicles; 
	int   MeasurementDeviationPathMarginal[100];
	float AvgPathGap[100]; 
	float NewNumberOfVehicles[100]; 

	int  LeastTravelTime;

	//
	int   NumOfPaths;
	int   PathNodeSums[100];            // max 100 path for each ODT
	int   NumOfVehsOnEachPath[100]; 	
	int   PathLinkSequences[100][100];	// max 100 links on each path
	int   PathSize[100];				// number of nodes on each path
	//	int   BestPath[100];				// the link sequence of the best path for each ODT
	int   BestPathIndex;				// index of the best (i.e., least experienced time) path for each ODT
	float AvgPathTimes[100]; 	       // average path travel time across different vehicles on the same path with the same departure time


};

class PathArrayForEachODTK // Xuesong: store path set for each OD, tau and k set.
{
public: 
	long m_PathIndex; 
	unsigned short m_OriginZoneID;  //range 0, 65535
	unsigned short m_DestinationZoneID;  // range 0, 65535
	unsigned char m_DemandType;     // 1: passenger,  2, HOV, 2, truck, 3: bus

	int m_NodeSum; 

	float m_starting_time_in_min;
	float m_ending_time_in_min;

	int m_LinkSize;
	std::vector<int> m_LinkNoArray;
	float m_VehicleSize;

	PathArrayForEachODTK()
	{
		m_LinkSize = 0;
		m_VehicleSize = 0;

	}

	void Setup(int PathIndex,int OriginZoneID, int DestinationZoneID, int DemandType, int starting_time_in_min, int ending_time_in_min, int LinkSize, int PathLinkSequences[100], float VehicleSize, int NodeSum)
	{
		m_PathIndex  = PathIndex;

		m_OriginZoneID = OriginZoneID;
		m_DestinationZoneID = DestinationZoneID;
		m_DemandType = DemandType;

		m_starting_time_in_min = starting_time_in_min;
		m_ending_time_in_min = ending_time_in_min;


		m_LinkSize = LinkSize;

		m_VehicleSize = VehicleSize;
		for(int i = 0; i< LinkSize; i++)
		{
			m_LinkNoArray.push_back (PathLinkSequences[i]);
		}


		m_NodeSum = NodeSum;
	}
	~ PathArrayForEachODTK()
	{

		m_LinkNoArray.clear();

	}
};

class NetworkSimulationResult
{
public: 

	int number_of_vehicles;
	int number_of_vehicles_PricingType[MAX_PRICING_TYPE_SIZE];

	float avg_travel_time_in_min, avg_distance_in_miles, avg_speed;

	;

	float Energy,CO2,NOX,CO,HC;

	NetworkSimulationResult()
	{

		number_of_vehicles = 0;
		for(int p = 0; p < MAX_PRICING_TYPE_SIZE; p++)
		{
			number_of_vehicles_PricingType [p] = 0;
		}
		avg_travel_time_in_min = 0;
		avg_distance_in_miles = 0;

		Energy = 0;
		CO2 = 0;
		NOX = 0;
		CO = 0;
		HC = 0;
	}
};


class ODStatistics
{
public: 

	ODStatistics()
	{
		TotalVehicleSize = 0;
		TotalCompleteVehicleSize = 0;
		TotalTravelTime = 0;
		TotalDistance = 0;
		TotalCost = 0;
		TotalEmissions = 0;

		Diverted_TotalVehicleSize = 0;
		Diverted_TotalTravelTime = 0;
		Diverted_TotalDistance = 0;
	}

	int   TotalVehicleSize;
	int   TotalCompleteVehicleSize;
	float TotalTravelTime;
	float TotalDistance;
	float TotalCost;
	float TotalEmissions;

	int   Diverted_TotalVehicleSize;
	float Diverted_TotalTravelTime;
	float Diverted_TotalDistance;

};

class D2DODStatistics
{
public: 

	std::map<int,ODStatistics> D2DODStatisticsMap;

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
	std::vector<int> m_NodeNameArray;

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
	int   TotalVehicleSize;
	float TotalTravelTime;
	float TotalDistance;
	float TotalCost;
	float TotalEmissionsEnergy;
	float TotalEmissionsCO2;
	float TotalEmissionsCO;
	float TotalEmissionsNOX;
	float TotalEmissionsHC;

	ODPathSet()
	{
		TotalVehicleSize = 0;
		TotalTravelTime = 0;
		TotalDistance = 0;
		TotalCost = 0;
		TotalEmissionsEnergy = TotalEmissionsCO2 = TotalEmissionsNOX  = TotalEmissionsHC = TotalEmissionsCO =0;
	}


};


typedef struct  
{
	int vehicle_id;
	int from_zone_id;
	int to_zone_id;
	float departure_time;
	int demand_type;
	int pricing_type;
	int vehicle_type;
	int information_type;
	float value_of_time;
} struct_VehicleInfo_Header;



extern std::vector<PathArrayForEachODTK> g_ODTKPathVector;

void Assignment_MP(int id, int nthreads, int node_size, int link_size, int iteration);

void g_OutputMOEData(int iteration);

void OutputLinkMOEData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty);
void OutputNetworkMOEData(ofstream &output_NetworkTDMOE_file);
void OutputVehicleTrajectoryData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty);
void OutputODMOEData(ofstream &output_ODMOE_file,int cut_off_volume = 1);
void OutputTimeDependentODMOEData(ofstream &output_ODMOE_file,int department_time_intreval = 60, int cut_off_volume = 1);
void OutputEmissionData();
void OutputTimeDependentPathMOEData(ofstream &output_PathMOE_file, int cut_off_volume = 50);
void OutputAssignmentMOEData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty);


float g_GetPrivateProfileFloat( LPCTSTR section, LPCTSTR key, float def_value, LPCTSTR filename);
int g_WritePrivateProfileInt( LPCTSTR section, LPCTSTR key, int def_value, LPCTSTR filename) ;
int g_GetPrivateProfileInt( LPCTSTR section, LPCTSTR key, int def_value, LPCTSTR filename);

float GetStochasticCapacity(bool bQueueFlag, float CurrentCapacity);

float GetDynamicCapacityAtSignalizedIntersection(float HourlyCapacity, float CycleLength_in_seconds,double CurrentTime);
void InitWELLRNG512a (unsigned int *init);
double WELLRNG512a (void);

double g_GetRandomRatio();
int g_GetRandomInteger_From_FloatingPointValue(float Value);
int g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(float Value, int LinkID, int SimulationIntervalNo);

void g_ReadDTALiteVehicleFile(char fname[_MAX_PATH]);
void g_ReadDTALiteAgentCSVFile(char fname[_MAX_PATH]);
void g_ReadDemandFile();
void g_ReadDemandFileBasedOnUserSettings();

void g_VehicleRerouting(int DayNo,int v, float CurrentTime, MessageSign* p_is); // v for vehicle id
void g_ODBasedDynamicTrafficAssignment();
void g_AgentBasedAssisnment();

void g_MultiDayTrafficAssisnment();
void OutputMultipleDaysVehicleTrajectoryData(char fname[_MAX_PATH]);
int g_OutputSimulationSummary(float& AvgTravelTime, float& AvgDistance, float& AvgSpeed,float& AvgCost, EmissionStatisticsData &emission_data,
							  int InformationClass, int DemandType, int VehicleType, int DepartureTimeInterval);
void g_DynamicTraffcAssignmentWithinInnerLoop(int iteration, bool NotConverged, int TotalNumOfVehiclesGenerated);
void InnerLoopAssignment(int zone,int departure_time_begin, int departure_time_end, int inner_iteration);

void g_OutputLinkMOESummary(ofstream &LinkMOESummaryFile, int cut_off_volume=0);
void g_Output2WayLinkMOESummary(ofstream &LinkMOESummaryFile, int cut_off_volume=0);
void g_OutputSummaryKML(Traffic_MOE moe_mode);

extern CString g_GetAppRunningTime(bool with_title = true);
void g_ComputeFinalGapValue();

extern float g_UserClassPerceptionErrorRatio[MAX_SIZE_INFO_USERS];
extern float g_VMSPerceptionErrorRatio;
extern int g_information_updating_interval_of_en_route_info_travelers_in_min;
extern int g_information_updating_interval_of_VMS_in_min;
extern void ConstructPathArrayForEachODT(PathArrayForEachODT *, int, int); // construct path array for each ODT
extern void ConstructPathArrayForEachODT_ODEstimation(PathArrayForEachODT *, int, int); // construct path array for each ODT
extern void g_UpdateLinkMOEDeviation_ODEstimation(NetworkLoadingOutput& output);
extern void g_GenerateVehicleData_ODEstimation();
extern char g_GetLevelOfService(int PercentageOfSpeedLimit);
extern bool g_read_a_line(FILE* f, char* aline, int & size);

std::string g_GetTimeStampStrFromIntervalNo(int time_interval);

extern void g_FreeMemoryForVehicleVector();
extern void g_FreeODTKPathVector();

void g_AgentBasedShortestPathGeneration();

extern bool g_ReadLinkMeasurementFile();
//extern void g_ReadObservedLinkMOEData(DTANetworkForSP* pPhysicalNetwork);

// for OD estimation
extern float*** g_HistODDemand;
extern float    g_ODEstimation_WeightOnHistODDemand;
extern float    g_ODEstimation_Weight_Flow;
extern float    g_ODEstimation_Weight_NumberOfVehicles;
extern float    g_ODEstimation_Weight_TravelTime;
extern float    g_ODEstimation_WeightOnUEGap;
extern float    g_ODEstimation_StepSize;

extern int g_ODEstimationFlag;
extern int g_ODEstimationMeasurementType;
extern int g_ODEstimation_StartingIteration;

extern VehicleArrayForOriginDepartrureTimeInterval** g_TDOVehicleArray; // TDO for time-dependent origin;
extern std::vector<NetworkLoadingOutput>  g_AssignmentMOEVector;
extern std::vector<DTA_vhc_simple>   g_simple_vector_vehicles;

// for fast data acessing
extern int g_LastLoadedVehicleID; // scan vehicles to be loaded in a simulation interval

extern FILE* g_ErrorFile;
extern ofstream g_LogFile;
extern CCSVWriter g_SummaryStatFile;

extern ofstream g_AssignmentLogFile;
extern ofstream g_EstimationLogFile;
void g_DTALiteMain();
void g_DTALiteMultiScenarioMain();

extern int g_InitializeLogFiles();
extern void g_ReadDTALiteSettings();
extern int g_AgentBasedAssignmentFlag;
extern float g_DemandGlobalMultiplier;
extern void g_TrafficAssignmentSimulation();
extern void g_OutputSimulationStatistics(int Iteration);
extern void g_FreeMemory();
extern void g_CloseFiles();

extern NetworkSimulationResult g_SimulationResult;
extern void g_RunStaticExcel();
extern TCHAR g_DTASettingFileName[_MAX_PATH];
extern void g_SetLinkAttributes(int usn, int dsn, int NumOfLanes);
extern void g_ReadInputFiles();
void  ReadIncidentScenarioFile(string FileName);
void ReadVMSScenarioFile(string FileName);
void ReadLinkTollScenarioFile(string FileName);
void ReadWorkZoneScenarioFile(string FileName);
extern void g_CreateLinkTollVector();
extern void g_ReadDemandFile_Parser();
extern void g_OutputDay2DayVehiclePathData(char fname[_MAX_PATH],int StartIteration,int EndIteration);

extern	int g_OutputSimulationMOESummary(float& AvgTravelTime, float& AvgDistance, float& AvgSpeed, float & AvgCost, EmissionStatisticsData &emission_data, LinkMOEStatisticsData &link_data,
										 int DemandType=0,int VehicleType = 0, int InformationClass=0, int origin_zone_id = 0, int destination_zone_id = 0,
										 int from_node_id = 0, int mid_node_id	=0, int to_node_id	=0,	
										 int departure_starting_time	 = 0,int departure_ending_time= 144);