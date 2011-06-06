//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com)

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html


//    This file is part of NeXTA Version 3 (Open-source).

//    NEXTA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    NEXTA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with NEXTA.  If not, see <http://www.gnu.org/licenses/>.
#pragma once

#include "resource.h"
#include "Utility.h"

#include <math.h>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <list>

#include "Timetable.h"

using namespace std;
#define PI 3.1415926

#define MAX_AdjLinkSize 15
#define	MAX_SPLABEL 99999
#define MAX_NODE_SIZE_IN_A_PATH 4000

#define NUM_PATHMOES 8  // Distance, Travel Time, Emissions, Safety
#define NUM_VEHPATHMOES 8  // Distance, Travel Time, Emissions, Safety
#define NUM_PATHS   6

#define MAX_TRAIN_TYPE_SIZE 2
#define TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL 5
#define MAX_DAY_SIZE 1 


struct GDPoint
{
	float x;
	float y;
};


struct GDRect
{
	float left, right,top, bottom;

	float Height() { return top - bottom; }
	float Width()  { return right - left; }

	bool PtInRect(GDPoint& pt)
	{
		return left <= pt.x && pt.x <= right && bottom <= pt.y && pt.y <= top;
	}

	GDPoint Center(){
		GDPoint pt;
		pt.x = left + (right - left) / 2;
		pt.y = bottom + (top - bottom) / 2;
		return pt;
	}

	void Expand(GDPoint& pt)  // Inflate by a point
	{
		left = min(left, pt.x);
		top = max(top, pt.y);
		right = max(right, pt.x);
		bottom = min(bottom, pt.y);
	}

	void Expand(GDRect& rect)  // Inflate by another Rectangle
	{
		left = min(left, rect.left);
		top = max(top, rect.top);
		right = max(right, rect.right);
		bottom = min(bottom, rect.bottom);
	}

};
typedef struct{
	float speed;
	float traveltime;
}struc_traffic_state;

extern float g_P2P_Distance(GDPoint p1, GDPoint p2);

extern float g_DistancePointLine(GDPoint pt, GDPoint FromPt, GDPoint ToPt);


/////

class DTAZone
{ 
public:
	DTAZone()
	{
		m_Capacity  =0;
		m_Demand = 0;

	}
	float m_Capacity;
	float m_Demand;

};
class DTANode
{
public:
	DTANode(){
		m_NodeNumber = 0;
		m_ControlType = 0;
		m_ZoneID = 0;
		m_TotalCapacity = 0;
		m_Connections = 0;
	};
	~DTANode(){};
	GDPoint pt;
	int m_NodeNumber;  //  original node number
	int m_NodeID;  ///id, starting from zero, continuous sequence
	int m_ZoneID;  // If ZoneID > 0 --> centriod,  otherwise a physical node.
	int m_ControlType; // Type: ....
	float m_TotalCapacity;
	int m_Connections;  // number of connections

};

// event structure in this "event-basd" traffic simulation
typedef struct{
	int veh_id;
	float time_stamp;
}struc_vehicle_item;



class SLinkMOE  // time-dependent link MOE
{
public:
	float ObsQueuePerc;
	float ObsTravelTimeIndex;
	float ObsSpeed;  // speed
	float ObsFlow;   // flow volume
	float ObsCumulativeFlow;   // flow volume
	float ObsDensity;   // ObsDensity
	int EventCode; // 0, 1: weather, 2: demand, 3: incident, 4: special events
	int EpisodeNo;
	int EpisoDuration;

	float PredSpeed;

	float FullCapacity;
	float ReducedCapacity;
	float EventStartTiming;
	float PredTTI15; // predicted travel time for the future 15 min

	//   Density can be derived from CumulativeArrivalCount and CumulativeDepartureCount
	//   Flow can be derived from CumulativeDepartureCount
	//   AvgTravel time can be derived from CumulativeArrivalCount and TotalTravelTime

	SLinkMOE()
	{
		EventCode = 0;
		EpisoDuration = 0;
		EpisodeNo = 0;
		ObsQueuePerc = 0;
		ObsTravelTimeIndex = 0;
		ObsSpeed = 0;

		ObsFlow = 0;
		ObsCumulativeFlow = 0;
		ObsDensity = 0;
		PredSpeed = 0;
		PredTTI15 = 0;
		FullCapacity = 0;
		ReducedCapacity = 0;
		EventStartTiming = 0;

	};

	void SetupMOE(float FreeFlowTravelTime, float SpeedLimit)
	{
		ObsQueuePerc = 0;
		ObsTravelTimeIndex = FreeFlowTravelTime;
		ObsSpeed = SpeedLimit;
		ObsFlow = 0;
		ObsCumulativeFlow = 0;
		ObsDensity = 0;
		PredSpeed = SpeedLimit;

	}

} ;


class MergeIncomingLink
{
public:
	MergeIncomingLink()
	{
		m_LinkInCapacityRatio = 0;
	};
	long m_LinkID;
	int m_link_type;
	int m_NumLanes;
	float m_LinkInCapacityRatio;
};


class CapacityReduction
{
public:
	float StartTime;
	float EndTime;
	float LaneClosureRatio;
	float SpeedLimit;
	float ServiceFlowRate;
};

class DTALink
{
public:

	DTALink(int TimeHorizon)  // TimeHorizon's unit: per min
	{
		m_ObsHourlyLinkVolume = 0;
		m_SimulationHorizon	= TimeHorizon;
		m_LinkMOEAry = new SLinkMOE[m_SimulationHorizon+1];
		m_HistLinkMOEAry = new SLinkMOE[min(m_SimulationHorizon+1,1441)];
		m_StochaticCapcityFlag = 0;
		m_bMergeFlag = 0;
		m_MergeOnrampLinkID = -1;
		m_MergeMainlineLinkID = -1;
		m_bSensorData = false;
		m_SensorID = -1;
		m_OverlappingCost = 0;
		m_DisplayLinkID = -1;

		m_MinSpeed = 40;
		m_MaxSpeed = 40;

		m_ResourceAry = NULL;
		m_StaticSpeed = 0;
		m_StaticLaneVolume = 0;
		m_StaticTravelTime = 0;
		m_StaticVOC  = 0;

	};

	float m_StaticSpeed, m_StaticLaneVolume;
	float m_StaticTravelTime, m_StaticVOC;

	//for timetabling use
	std::map<int, int> m_RuningTimeMap;  //indexed by train type

	int GetTrainRunningTime(int TrainType)
	{
		if(m_LaneCapacity<0.001)
			return 1440;

		map <int, int> :: iterator mIter  = m_RuningTimeMap.find(TrainType);

		if ( mIter == m_RuningTimeMap.end( ) )
			return 1440; // very large number for prohibited train type, one day
		else
			return  mIter -> second;  // return the running time value matching the train type
	}


	float ObtainHistTravelTime(int time)
	{
		if(m_HistLinkMOEAry!=NULL && time<1440 && m_bSensorData == true)
		{
			return m_Length/m_HistLinkMOEAry[time].ObsSpeed*60;  // *60: hour to min
		}
		else
			return m_FreeFlowTravelTime;
	}

	float ObtainHistFuelConsumption(int time)
	{
		if(m_HistLinkMOEAry!=NULL && time<1440 && m_bSensorData == true)
		{
			return m_Length*0.1268f*pow( max(1,m_HistLinkMOEAry[time].ObsSpeed),-0.459f);  // Length*fuel per mile(speed), y= 0.1268x-0.459
		}
		else
			return m_Length*0.1268f*pow(m_SpeedLimit,-0.459f);  // Length*fuel per mile(speed_limit), y= 0.1268x-0.459
	}

	float ObtainHistCO2Emissions(int time)  // pounds
	{
		if(m_HistLinkMOEAry!=NULL && time<1440 && m_bSensorData == true)
		{
			return min(1.4f,m_Length*11.58f*pow(m_HistLinkMOEAry[time].ObsSpeed,-0.818f));  // Length*fuel per mile(speed), y= 11.58x-0.818
		}
		else
			return m_Length*11.58f*pow(m_SpeedLimit,-0.818f);  // Length*fuel per mile(speed_limit), y= 11.58x-0.818
	}

	void ResetMOEAry(int TimeHorizon)
	{
		m_SimulationHorizon	= TimeHorizon;

		m_StaticSpeed = 0;
		m_StaticLaneVolume = 0;
		m_StaticTravelTime = 0;
		m_StaticVOC  = 0;

		if(m_LinkMOEAry !=NULL)
			delete m_LinkMOEAry;

		m_LinkMOEAry = new SLinkMOE[m_SimulationHorizon+1];

		int t;
		for(t=0; t<= TimeHorizon; t++)
		{
			m_LinkMOEAry[t].SetupMOE(m_FreeFlowTravelTime,m_SpeedLimit);
		}	

		if(m_HistLinkMOEAry !=NULL)
			delete m_HistLinkMOEAry;

		m_HistLinkMOEAry = new SLinkMOE[1441];

		for(t=0; t< 1440; t++)
		{
			m_HistLinkMOEAry[t].ObsSpeed = m_SpeedLimit;
			m_HistLinkMOEAry[t].ObsFlow = 0;
			m_HistLinkMOEAry[t].ObsTravelTimeIndex = m_FreeFlowTravelTime;
			m_HistLinkMOEAry[t].ObsCumulativeFlow = 0;
			m_HistLinkMOEAry[t].ObsDensity = 0;

		}	


	};

	void ComputeHistoricalAvg(int number_of_weekdays);


		struc_traffic_state GetPredictedState(int CurrentTime, int PredictionHorizon);  // return value is speed



	GDPoint m_FromPoint, m_ToPoint;
	double DefaultDistance()
	{
		return pow((m_FromPoint.x - m_ToPoint.x)*(m_FromPoint.x - m_ToPoint.x) + 
			(m_FromPoint.y - m_ToPoint.y)*(m_FromPoint.y - m_ToPoint.y),0.5f);
	}

	float 	GetImpactedFlag(int DepartureTime)
	{
		for(unsigned int il = 0; il< CapacityReductionVector.size(); il++)
		{
			if(DepartureTime >= CapacityReductionVector[il].StartTime && DepartureTime<=CapacityReductionVector[il].EndTime + 60)  // 60 impacted after capacity reduction
			{
				return CapacityReductionVector[il].LaneClosureRatio;
			}
		}

		return 0;
	}

	SResource *m_ResourceAry;
	void ResetResourceAry(int OptimizationHorizon)
	{
		if(m_ResourceAry!=NULL)
			delete m_ResourceAry;

		m_ResourceAry = new SResource[OptimizationHorizon];
	}

	SLinkMOE *m_LinkMOEAry;
	SLinkMOE *m_HistLinkMOEAry;

	bool m_bSensorData;
	int  m_SensorID;

	float m_ObsHourlyLinkVolume;
	int *aryCFlowA;
	int *aryCFlowD;

	std::vector<CapacityReduction> CapacityReductionVector;

	int m_bMergeFlag;  // 1: freeway and freeway merge, 2: freeway and ramp merge
	std::vector<MergeIncomingLink> MergeIncomingLinkVector;
	int m_MergeOnrampLinkID;
	int m_MergeMainlineLinkID;


	std::list<struc_vehicle_item> LoadingBuffer;  //loading buffer of each link, to prevent grid lock

	std::list<struc_vehicle_item> EntranceQueue;  //link-in queue  of each link
	std::list<struc_vehicle_item> ExitQueue;      // link-out queue of each link



	~DTALink(){
		if(m_LinkMOEAry) delete m_LinkMOEAry;
		if(m_HistLinkMOEAry) delete m_HistLinkMOEAry;
		if(aryCFlowA) delete aryCFlowA;
		if(aryCFlowD) delete aryCFlowD;

		if(m_ResourceAry) delete m_ResourceAry;



		LoadingBuffer.clear();
		EntranceQueue.clear();
		ExitQueue.clear();

	};

	float GetFreeMovingTravelTime(int TrafficModelFlag=2, float Time = -1)
	{
		if(TrafficModelFlag ==0) // BRP model
			return m_StaticTravelTime;
		else 
		{
			for(unsigned int il = 0; il< CapacityReductionVector.size(); il++)
			{
				if(Time>=CapacityReductionVector[il].StartTime && Time<=CapacityReductionVector[il].EndTime)
				{
					return m_Length/CapacityReductionVector[il].SpeedLimit*60.0f;  // convert from hour to min;
				}
			}
			return m_FreeFlowTravelTime;
		}
	}

	void SetupMOE()
	{
		m_JamTimeStamp = (float) m_SimulationHorizon;
		m_FreeFlowTravelTime = m_Length/m_SpeedLimit*60.0f;  // convert from hour to min
		m_StaticTravelTime = m_FreeFlowTravelTime;

		CFlowArrivalCount = 0;
		CFlowDepartureCount = 0;

		LinkOutCapacity = 0;
		LinkInCapacity = 0;

		VehicleCount = 0;

		int t;

		for(t=0; t<=m_SimulationHorizon; t++)
		{
			m_LinkMOEAry[t].SetupMOE(m_FreeFlowTravelTime,m_SpeedLimit);
		}
		LoadingBuffer.clear();
		EntranceQueue.clear();
		ExitQueue.clear();


	}
	int m_LinkID;
	int m_FromNodeID;  // index starting from 0
	int m_ToNodeID;    // index starting from 0

	int m_DisplayLinkID;

	int m_FromNodeNumber;
	int m_ToNodeNumber;

	float	m_Length;  // in miles
	float    m_VehicleSpaceCapacity; // in vehicles
	int		m_NumLanes;
	float	m_SpeedLimit;
	float	m_MaximumServiceFlowRatePHPL;  //Capacity used in BPR for each link, reduced due to link type and other factors.

	float m_FromNodeY;  // From Node, Y value
	float m_ToNodeY;    // To Node, Y value

	float m_MinSpeed;
	float m_MaxSpeed;
	float m_MeanSpeed;
	float m_MeanVolume;

	/* For min-by-min train timetabling, m_LaneCapacity is 1 for each min. 
	Example in airspace scheduling
	a sector is a volume of airspace for which a single air traffic control team has responsibility.
	The number of aircraft that can safely occupy a sector simultaneously is determined by controllers. 
	A typical range is around 8-15. 
	During a 15 minute time interval in US en-route airspace, 
	the typical upper bound limits of the order of 15-20.

	*/

	int  m_StochaticCapcityFlag;  // 0: deterministic cacpty, 1: lane drop. 2: merge, 3: weaving
	// optional for display only
	int	m_link_type;

	// for MOE data array
	int m_SimulationHorizon;

	// traffic flow propagation
	float m_FreeFlowTravelTime; // min
	int m_BackwardWaveTimeInSimulationInterval; // simulation time interval

	//  multi-day equilibirum: travel time for stochastic capacity
	float m_LaneCapacity;

	float m_AverageTravelTime;

	float m_OverlappingCost;

	float m_JamTimeStamp;

	int CFlowArrivalCount;
	int CFlowDepartureCount;

	unsigned int LinkOutCapacity;  // unit: number of vehiles
	int LinkInCapacity;   // unit: number of vehiles

	int VehicleCount;

	float GetObsSpeed(int t)
	{
		if(t < m_SimulationHorizon)
			return max(m_StaticSpeed, m_LinkMOEAry[t].ObsSpeed);  
		else
			return m_StaticSpeed;
	}
	float GetObsLaneVolume(int t)
	{
		if(t < m_SimulationHorizon)
			return max(m_StaticLaneVolume, m_LinkMOEAry[t].ObsFlow);  
		else
			return m_StaticLaneVolume;
	}

	float GetObsTravelTimeIndex(int t)
	{
		if(t < m_SimulationHorizon)
			return max(m_StaticTravelTime, m_LinkMOEAry[t].ObsTravelTimeIndex);  
		else
			return m_StaticTravelTime;
	}

	int GetEventCode(int t)
	{
		if(t < m_SimulationHorizon)
			return m_LinkMOEAry[t].EventCode;  
		else
			return 0;
	}
	float GetObsCumulativeFlow(int t)
	{
		if(t < m_SimulationHorizon)
			return max(m_StaticLaneVolume, m_LinkMOEAry[t].ObsCumulativeFlow);  
		else
			return m_StaticLaneVolume;
	}

	float GetObsDensity(int t)
	{
		if(t < m_SimulationHorizon)
			return max(0, m_LinkMOEAry[t].ObsDensity);  
		else
			return 0;
	}		

	float GetSpeed(int time)
	{
		return m_Length/GetTravelTime(time,1)*60.0f;  // 60.0f converts min to hour, unit of speed: mph
	}


	float GetTravelTime(int starting_time, int time_interval = 1)
	{

		if(GetImpactedFlag(120))
			return 100;

		if(this->m_bSensorData == false)
			return m_FreeFlowTravelTime;

		float travel_time  = max(m_StaticTravelTime,m_FreeFlowTravelTime);

		if(starting_time + time_interval< m_SimulationHorizon)
		{
			float total_travel_time = 0;
			for(int t=starting_time; t< starting_time + time_interval; t++)
			{
				total_travel_time += m_LinkMOEAry[t].ObsTravelTimeIndex/100;
			}

			travel_time =  total_travel_time/time_interval;

			if(travel_time < m_FreeFlowTravelTime)
				travel_time = m_FreeFlowTravelTime; // minimum travel time constraint for shortest path calculation

		}

		ASSERT(travel_time>=0.09);

		return travel_time;

	};


};


class DTAPath
{
public:
	DTAPath(int LinkSize, int TimeHorizon)
	{
		m_TimeHorizon = TimeHorizon;
		m_LinkSize = LinkSize;
		m_LinkVector = new int[LinkSize];
		m_number_of_days = max(1,TimeHorizon/1440);

		if(m_TimeHorizon>0)
		{
		m_TimeDependentTravelTime = new float[TimeHorizon];
		for(int t=0; t<TimeHorizon; t++)
		{
			m_TimeDependentTravelTime[t] = 0;
		}

		m_WithinDayMeanTimeDependentTravelTime = new float[1440];
		m_WithinDayMaxTimeDependentTravelTime  = new float[1440];

		m_WithinDayMeanTimeDependentFuelConsumption = new float[1440];
		m_WithinDayMeanTimeDependentEmissions= new float[1440];
		m_WithinDayMeanGeneralizedCost = new float[1440];



		for(int t=0; t<1440; t++)
		{
			m_WithinDayMeanTimeDependentTravelTime[t] = 0;
			m_WithinDayMaxTimeDependentTravelTime[t] = 0;
			m_WithinDayMeanTimeDependentFuelConsumption[t] = 0;
			m_WithinDayMeanTimeDependentEmissions[t] = 0;
			m_WithinDayMeanGeneralizedCost[t] = 0;
		}

		}
		m_Distance =0;
		m_TravelTime = 0;
		m_Reliability = 0;
		m_Emissions = 0;
		m_Safety = 0;
		m_Fuel = 0;
		m_MaxTravelTime = 0;


	}

	void UpdateWithinDayStatistics();
	

	float GetTravelTimeMOE(int time, int MOEType)
	{
		if(m_TimeHorizon==0)
			return 0;

		switch(MOEType)
		{
		case 0: return m_WithinDayMeanTimeDependentTravelTime[time];
		case 1: return m_WithinDayMaxTimeDependentTravelTime[time];
		case 2: return m_WithinDayMeanTimeDependentTravelTime[time]+1.67f/3.0f*(m_WithinDayMaxTimeDependentTravelTime[time]-m_WithinDayMeanTimeDependentTravelTime[time]);  // max-mean --> 3 sigma, use mean+ 1.67 sigma as utility,
		case 3: return (m_WithinDayMaxTimeDependentTravelTime[time]-m_WithinDayMeanTimeDependentTravelTime[time])/3.0f;
		case 4: return m_WithinDayMeanTimeDependentFuelConsumption[time];
		case 5: return m_WithinDayMeanTimeDependentEmissions[time];
		case 6: return m_WithinDayMeanGeneralizedCost[time];


		default: return m_WithinDayMeanTimeDependentTravelTime[time];
		}

	}

	~DTAPath()
	{
		if(m_LinkVector) delete m_LinkVector;
		if(m_TimeDependentTravelTime) delete m_TimeDependentTravelTime;

		if(m_WithinDayMeanTimeDependentTravelTime) delete m_WithinDayMeanTimeDependentTravelTime;
		if(m_WithinDayMaxTimeDependentTravelTime) delete m_WithinDayMaxTimeDependentTravelTime;

		if(m_WithinDayMeanTimeDependentFuelConsumption) delete m_WithinDayMeanTimeDependentFuelConsumption;
		if(m_WithinDayMeanTimeDependentEmissions) delete m_WithinDayMeanTimeDependentEmissions;

		if(m_WithinDayMeanGeneralizedCost) delete m_WithinDayMeanGeneralizedCost;

	}

	int m_LinkSize;
	int *m_LinkVector;
	int m_NodeNodeSum;

	float *m_TimeDependentTravelTime;
	float m_MaxTravelTime;

	float *m_WithinDayMeanTimeDependentTravelTime;
	float *m_WithinDayMaxTimeDependentTravelTime;

	float *m_WithinDayMeanTimeDependentFuelConsumption;  // unit: gallon
	float *m_WithinDayMeanTimeDependentEmissions;  // unit: pounds

	float *m_WithinDayMeanGeneralizedCost;  // unit: pounds

	int m_number_of_days;
	int m_TimeHorizon;


	float m_Distance;

	float m_TravelTime;
	float m_Reliability;
	float m_Emissions;
	float m_Safety;
	float m_Fuel;


};






// link element of a vehicle path
class SVehicleLink
{  public:
unsigned short  LinkID;  // range:
float ArrivalTimeOnDSN;     // absolute arrvial time at downstream node of a link: 0 for the departure time, including delay/stop time
//   float LinkWaitingTime;   // unit: 0.1 seconds
SVehicleLink()
{
	LinkID = 0;
	ArrivalTimeOnDSN = 0;
	//		LinkWaitingTime = 0;

}

};


class DTAVehicleAdditionalData   // this class contains non-essential information, we do not allocate memory for this additional info in the basic version
{
public:
	unsigned char m_VOT;        // range 0 to 255
	float m_TollDollar;
	float m_MinCost;
	float m_MeanTravelTime;
	float m_TravelTimeVariance;
	unsigned short m_NumberOfSamples;  // when switch a new path, the number of samples starts with 0

	DTAVehicleAdditionalData()
	{
		m_NumberOfSamples =0;
		m_VOT = 10;
		m_MinCost = 0;
	};

	void PostTripUpdate(float TripTime)   
	{
		float GainFactor = 0.2f;  // will use formula from Kalman Filtering, eventually

		m_MeanTravelTime = (1-GainFactor)*m_MeanTravelTime + GainFactor*TripTime;
		m_NumberOfSamples +=1;
	};


};

class DTAVehicle
{
public:

	int m_NodeSize;
	int m_NodeNumberSum;  // used for comparing two paths
	SVehicleLink *m_NodeAry; // link list arrary of a vehicle path

	unsigned int m_RandomSeed;
	int m_VehicleID;  //range: +2,147,483,647

	unsigned short m_OriginZoneID;  //range 0, 65535
	unsigned short m_DestinationZoneID;  // range 0, 65535

	unsigned char m_VehicleType;     // 0: passenger, 1: truck
	unsigned char m_InformationClass;  // 0: historical, 1: pre-trip, 2: en-route
	unsigned char m_Occupancy;  // 1: LOV, 2: HOV
	unsigned short m_SimLinkSequenceNo; //  range 0, 65535

	bool  m_bImpacted;

	float m_TimeToRetrieveInfo;
	float m_DepartureTime;
	float m_ArrivalTime;
	float m_TripTime;
	float m_Delay;
	float m_Distance;
	bool m_bSwitched;  // switch route in assignment

	// used for simulation
	bool m_bLoaded; // be loaded into the physical network or not
	bool m_bComplete;

	DTAVehicleAdditionalData* pVehData;

	// multi-day equilibrium
	bool m_bETTFlag;
	int m_DayDependentLinkSize[MAX_DAY_SIZE];
	std::map<int, int> m_DayDependentAryLink;  // first key: day*MAX_PATH_LINK_SIZE + index, second element; return link index
	float m_DayDependentTripTime[MAX_DAY_SIZE];
	int m_DayDependentNodeNumberSum[MAX_DAY_SIZE];  // used for comparing two paths
	float m_DayDependentGap[MAX_DAY_SIZE];  // used for gap analysis
	float m_AvgDayTravelTime;
	float m_DayTravelTimeSTD;

	DTAVehicle()
	{
		pVehData=NULL;
		m_TimeToRetrieveInfo = -1;

		m_NodeAry = NULL;
		m_NodeSize	= 0;
		m_bImpacted = false; 
		m_InformationClass = 0;
		m_VehicleType = 0;
		m_Occupancy = 0;

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
		if(m_NodeAry != NULL)
			delete m_NodeAry;

		if(pVehData!=NULL)
			delete pVehData;
	};

	void PreTripReset()
	{
		m_ArrivalTime = 0;
		m_bLoaded = false;
		m_bComplete = false;
		m_TripTime = 0;
		m_Delay = 0;

	}


public:

	void SetInformationClass();


public:  // fetch additional data
	int GetVOT()
	{
		if(pVehData==NULL)
			return 10;
		else
			return pVehData->m_VOT;

	};

	void SetMinCost(float MinCost)
	{
		if(pVehData!=NULL)
			pVehData->m_MinCost = MinCost;
	};

	float GetMinCost()
	{
		if(pVehData==NULL)
			return 0.0f;
		else
			return pVehData->m_MinCost;

	};

};

class DTA_vhc_simple // used in STL sorting only
{
public:

	int m_OriginZoneID;
	int m_DestinationZoneID;
	int m_VehicleType;
	float    m_DepartureTime;


	bool operator<(const DTA_vhc_simple &other) const
	{
		return m_DepartureTime < other.m_DepartureTime;
	}

};



class DTA_sensor
{

public:
	DTA_sensor()
	{
		LinkID = -1;
	}
	int FromNodeNumber;
	int ToNodeNumber;
	int LinkID;
	int SensorType;
	long OrgSensorID;
	GDPoint pt;


};
template <typename T>
T **AllocateDynamicArray(int nRows, int nCols)
{
	T **dynamicArray;

	dynamicArray = new T*[nRows];

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
extern void g_ProgramStop();

class DTANetworkForSP  // mainly for shortest path calculation, not just physical network
	// for shortes path calculation between zone centroids, for origin zone, there are only outgoing connectors, for destination zone, only incoming connectors
	// different shortest path calculations have different network structures, depending on their origions/destinations
{
public:
	int m_OptimizationIntervalSize;
	int m_NodeSize;
	int m_PhysicalNodeSize;
	int m_OptimizationHorizon;
	int m_OptimizationTimeInveral;
	int m_ListFront;
	int m_ListTail;
	int m_LinkSize;

	int* m_LinkList;  // dimension number of nodes

	int** m_OutboundNodeAry; //Outbound node array
	int** m_OutboundLinkAry; //Outbound link array
	int* m_OutboundSizeAry;  //Number of outbound links

	int** m_InboundLinkAry; //inbound link array
	int* m_InboundSizeAry;  //Number of inbound links

	int* m_FromIDAry;
	int* m_ToIDAry;

	float** m_LinkTDTimeAry;
	float** m_LinkTDCostAry;

	int* NodeStatusAry;                // Node status array used in KSP;

	float* LabelTimeAry;               // label - time
	int* NodePredAry;
	float* LabelCostAry;


	//below are time-dependent cost label and predecessor arrays
	float** TD_LabelCostAry;
	int** TD_NodePredAry;  // pointer to previous NODE INDEX from the current label at current node and time
	int** TD_TimePredAry;  // pointer to previous TIME INDEX from the current label at current node and time


	int m_Number_of_CompletedVehicles;
	int m_AdjLinkSize;

	// search tree
	struct SearchTreeElement
	{
		int CurrentNode;
		int PredecessorNode;
		int SearchLevel;
	};

	SearchTreeElement* m_SearchTreeList;  // predecessor

	int  m_TreeListSize;
	int  m_TreeListFront;
	int  m_TreeListTail;



	//

	DTANetworkForSP(int NodeSize, int LinkSize, int TimeHorizon, int TimeInterval, int AdjLinkSize){
		m_NodeSize = NodeSize;
		m_LinkSize = LinkSize;

		m_SearchTreeList = NULL;

		m_OptimizationHorizon = TimeHorizon;
		m_OptimizationTimeInveral = TimeInterval;
		m_AdjLinkSize = AdjLinkSize;


		m_OutboundSizeAry = new int[m_NodeSize];
		m_InboundSizeAry = new int[m_NodeSize];


		m_OutboundNodeAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);
		m_OutboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);
		m_InboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);


		m_LinkList = new int[m_NodeSize];

		m_OptimizationIntervalSize = int(m_OptimizationHorizon/m_OptimizationTimeInveral+0.1);  // make sure there is no rounding error
		m_LinkTDTimeAry   =  AllocateDynamicArray<float>(m_LinkSize,m_OptimizationIntervalSize);
		m_LinkTDCostAry   =  AllocateDynamicArray<float>(m_LinkSize,m_OptimizationIntervalSize);

		m_FromIDAry = new int[m_LinkSize];

		m_ToIDAry = new int[m_LinkSize];

		NodeStatusAry = new int[m_NodeSize];                    // Node status array used in KSP;
		NodePredAry = new int[m_NodeSize];
		LabelTimeAry = new float[m_NodeSize];                     // label - time
		LabelCostAry = new float[m_NodeSize];                     // label - cost

		TD_LabelCostAry =  AllocateDynamicArray<float>(m_NodeSize,m_OptimizationIntervalSize);
		TD_NodePredAry = AllocateDynamicArray<int>(m_NodeSize,m_OptimizationIntervalSize);
		TD_TimePredAry = AllocateDynamicArray<int>(m_NodeSize,m_OptimizationIntervalSize);


		if(m_OutboundSizeAry==NULL || m_LinkList==NULL || m_FromIDAry==NULL || m_ToIDAry==NULL  ||
			NodeStatusAry ==NULL || NodePredAry==NULL || LabelTimeAry==NULL || LabelCostAry==NULL)
		{
			cout << "Error: insufficent memory.";
			g_ProgramStop();
		}

	};

	DTANetworkForSP();

	void Init(int NodeSize, int LinkSize, int TimeHorizon,int AdjLinkSize)
	{
		m_NodeSize = NodeSize;
		m_LinkSize = LinkSize;

		m_OptimizationHorizon = TimeHorizon;
		m_AdjLinkSize = AdjLinkSize;


		m_OutboundSizeAry = new int[m_NodeSize];
		m_InboundSizeAry = new int[m_NodeSize];


		m_OutboundNodeAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);
		m_OutboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);
		m_InboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);


		m_LinkList = new int[m_NodeSize];

		m_OptimizationIntervalSize = int(TimeHorizon/m_OptimizationTimeInveral)+1;  // make sure it is not zero
		m_LinkTDTimeAry   =  AllocateDynamicArray<float>(m_LinkSize,m_OptimizationIntervalSize);
		m_LinkTDCostAry   =  AllocateDynamicArray<float>(m_LinkSize,m_OptimizationIntervalSize);

		m_FromIDAry = new int[m_LinkSize];

		m_ToIDAry = new int[m_LinkSize];

		NodeStatusAry = new int[m_NodeSize];                    // Node status array used in KSP;
		NodePredAry = new int[m_NodeSize];
		LabelTimeAry = new float[m_NodeSize];                     // label - time
		LabelCostAry = new float[m_NodeSize];                     // label - cost


		TD_LabelCostAry =  AllocateDynamicArray<float>(m_NodeSize,m_OptimizationIntervalSize);
		TD_NodePredAry = AllocateDynamicArray<int>(m_NodeSize,m_OptimizationIntervalSize);
		TD_TimePredAry = AllocateDynamicArray<int>(m_NodeSize,m_OptimizationIntervalSize);


		if(m_OutboundSizeAry==NULL || m_LinkList==NULL || m_FromIDAry==NULL || m_ToIDAry==NULL  ||
			NodeStatusAry ==NULL || NodePredAry==NULL || LabelTimeAry==NULL || LabelCostAry==NULL)
		{
			cout << "Error: insufficent memory.";
			g_ProgramStop();
		}

	};


	~DTANetworkForSP()
	{
		if(m_SearchTreeList) delete m_SearchTreeList;

		if(m_OutboundSizeAry)  delete m_OutboundSizeAry;
		if(m_InboundSizeAry)  delete m_InboundSizeAry;

		DeallocateDynamicArray<int>(m_OutboundNodeAry,m_NodeSize, m_AdjLinkSize);
		DeallocateDynamicArray<int>(m_OutboundLinkAry,m_NodeSize, m_AdjLinkSize);
		DeallocateDynamicArray<int>(m_InboundLinkAry,m_NodeSize, m_AdjLinkSize);


		if(m_LinkList) delete m_LinkList;

		DeallocateDynamicArray<float>(m_LinkTDTimeAry,m_LinkSize,m_OptimizationIntervalSize);
		DeallocateDynamicArray<float>(m_LinkTDCostAry,m_LinkSize,m_OptimizationIntervalSize);

		DeallocateDynamicArray<float>(TD_LabelCostAry,m_NodeSize,m_OptimizationIntervalSize);
		DeallocateDynamicArray<int>(TD_NodePredAry,m_NodeSize,m_OptimizationIntervalSize);
		DeallocateDynamicArray<int>(TD_TimePredAry,m_NodeSize,m_OptimizationIntervalSize);

		if(m_FromIDAry)		delete m_FromIDAry;
		if(m_ToIDAry)	delete m_ToIDAry;

		if(NodeStatusAry) delete NodeStatusAry;                 // Node status array used in KSP;
		if(NodePredAry) delete NodePredAry;
		if(LabelTimeAry) delete LabelTimeAry;
		if(LabelCostAry) delete LabelCostAry;



	};

	float GetTollRateInMin(int LinkID, float Time, int VehicleType);  // built-in function for each network_SP to avoid conflicts with OpenMP parallel computing


	void BuildNetwork(int ZoneID);
	void BuildHistoricalInfoNetwork(int CurZoneID, int CurrentTime, float Perception_error_ratio);
	void BuildTravelerInfoNetwork(int CurrentTime, float Perception_error_ratio);

	void BuildPhysicalNetwork(std::list<DTANode*>* p_NodeSet, std::list<DTALink*>* p_LinkSet, bool bRandomCost, bool bOverlappingCost);
	void BuildSpaceTimeNetworkForTimetabling(std::list<DTANode*>* p_NodeSet, std::list<DTALink*>* p_LinkSet, int TrainType);

	void IdentifyBottlenecks(int StochasticCapacityFlag);

	bool SimplifiedTDLabelCorrecting_DoubleQueue(int origin, int departure_time, int vehicle_type);   // Pointer to previous node (node)
	// simplifed version use a single node-dimension of LabelCostAry, NodePredAry

	//these two functions are for timetabling
	bool OptimalTDLabelCorrecting_DoubleQueue(int origin, int departure_time);
	// optimal version use a time-node-dimension of TD_LabelCostAry, TD_NodePredAry
	int FindOptimalSolution(int origin,  int departure_time,  int destination, DTA_Train* pTrain);
	bool GenerateSearchTree(int origin,  int destination, int node_size);

	// return node arrary from origin to destination, return travelling timestamp at each node
	// return number_of_nodes in path

	int FindInitiallSolution(int origin,  int departure_time,  int destination, DTA_Train* pTrain);

	void VehicleBasedPathAssignment(int zone,int departure_time_begin, int departure_time_end, int iteration);
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

struct VehicleCFData
{
	int VehicleID;
	int VehicleType; // 1 - motorcycle, 2 - auto, 3 - truck
	float StartTime; // in time interval, LinkStartTime, so it should be sorted
	float EndTime; // in time interval
};

struct VehicleSnapshotData
{
	float LocalY;
	float Speed_mph;
	int LaneID;
};

#pragma warning(disable:4244)  // stop warning: "conversion from 'int' to 'float', possible loss of data"
// Stop bugging me about this, live isn't perfect


extern float g_RNNOF();

extern std::vector<DTAPath*>	m_PathDisplayList;
extern int m_SelectPathNo;
extern float g_Simulation_Time_Stamp;
