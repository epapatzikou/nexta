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
using namespace std;

// extention for multi-day equilibirum
#define MAX_DAY_SIZE 1
#define MAX_PATH_LINK_SIZE 1000

#define MAX_VEHICLE_TYPE_SIZE 4

extern float g_VOT[MAX_VEHICLE_TYPE_SIZE];  //1: LOV, 2: HOV, 3: trucks, starting from 1!

extern float g_VehicleTypeRatio[MAX_VEHICLE_TYPE_SIZE];  //1: LOV, 2: HOV, 3: trucks, starting from 1!

#define	MAX_SPLABEL 99999.0f
#define MAX_TIME_INTERVAL_ADCURVE 200  // 200 simulation intervals of data are stored to keep tract cummulative flow counts of each link
extern int g_DepartureTimetInterval;
extern float g_BackwardWaveSpeed_in_mph;
extern float g_MinimumInFlowRatio;
extern float g_MaxDensityRatioForVehicleLoading;
extern int g_CycleLength_in_seconds;
extern float g_DefaultSaturationFlowRate_in_vehphpl;
#define MAX_LINK_NO 65530
#define MAX_NODE_SIZE_IN_A_PATH 2000
#define MAX_LINK_SIZE_IN_VMS 20

#define MAX_CPU_SIZE 20
// Linear congruential generator 
#define LCG_a 17364
#define LCG_c 0
#define LCG_M 65521  // it should be 2^32, but we use a small 16-bit number to save memory

void g_ProgramStop();
float g_RNNOF();


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
		m_ControlType = 0;
		m_ZoneID = 0;
		m_TotalCapacity = 0;
	};
	~DTANode(){};
	int m_NodeID;
	int m_ZoneID;  // If ZoneID > 0 --> centriod,  otherwise a physical node.
	int m_ControlType; // Type: ....
	float m_TotalCapacity;

};

// event structure in this "event-basd" traffic simulation
typedef struct{
	int veh_id;
	float time_stamp;
}struc_vehicle_item;



class SLinkMOE  // time-dependent link MOE
{
public:
	float TotalTravelTime;   // cumulative travel time for vehicles departing at this time interval

	int CumulativeArrivalCount; 
	int CumulativeDepartureCount;
	int ExitQueueLength;
	int EndTimeOfCongestion;  // time in min to the end of congestion

	//   Density can be derived from CumulativeArrivalCount and CumulativeDepartureCount
	//   Flow can be derived from CumulativeDepartureCount
	//   AvgTravel time can be derived from CumulativeArrivalCount and TotalTravelTime

	SLinkMOE()
	{
		TotalTravelTime = 0;
		CumulativeArrivalCount  = 0;
		CumulativeDepartureCount = 0;
		ExitQueueLength = 0;
		EndTimeOfCongestion = 0;
	};

	void SetupMOE()
	{
		TotalTravelTime = 0;
		CumulativeArrivalCount  = 0;
		CumulativeDepartureCount = 0;
		ExitQueueLength = 0;
		EndTimeOfCongestion = 0;
	}

} ;


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
	int StartTime;  // use integer
	int EndTime;    // use integer
	float LaneClosureRatio;
	float SpeedLimit;
};

class InformationSign
{
public:
	float StartTime;
	float EndTime;
	float ResponsePercentage;
	int	  Type;
	int   BestPathFlag;
	int   DetourLinkSize;
	int   DetourLinkArray[MAX_LINK_SIZE_IN_VMS];

	InformationSign()
	{
	}

	~InformationSign()
	{
	}

};

class Toll
{
public:
	float StartTime;
	float EndTime;
	float TollRate[MAX_VEHICLE_TYPE_SIZE];
	float TollRateInMin[MAX_VEHICLE_TYPE_SIZE];
};

class DTALink
{
public:
	DTALink(int TimeSize)  // TimeSize's unit: per min
	{
		m_SimulationHorizon	= TimeSize;
		m_LinkMOEAry = new SLinkMOE[m_SimulationHorizon+1];
		aryCFlowA = new int[MAX_TIME_INTERVAL_ADCURVE+1];         // for cummulative flow counts: unit is per simulation time interval. e.g. 6 seconds
		aryCFlowD = new int[MAX_TIME_INTERVAL_ADCURVE+1];
		m_StochaticCapcityFlag = 0;
		m_BPRLinkVolume = 0;
		m_BPRLinkTravelTime = 0;
		m_bMergeFlag = 0;
		m_MergeOnrampLinkID = -1;
		m_MergeMainlineLinkID = -1;
		m_TollSize = 0;

	};

	bool 	GetImpactedFlag(int DepartureTime)
	{
		for(unsigned int il = 0; il< CapacityReductionVector.size(); il++)
		{
			if(DepartureTime >= CapacityReductionVector[il].StartTime && DepartureTime<=CapacityReductionVector[il].EndTime + 60)  // 60 impacted after capacity reduction
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

	float GetNumLanes(int Time=-1)  // with lane closure
	{
		for(unsigned int il = 0; il< CapacityReductionVector.size(); il++)
		{
			if(Time>=CapacityReductionVector[il].StartTime && Time<=CapacityReductionVector[il].EndTime)
			{
				return (1-CapacityReductionVector[il].LaneClosureRatio)*m_NumLanes;
			}
		}

		return (float)m_NumLanes;

	}

	int GetInformationResponseID(float Time)  // from information signs
	{
		for(unsigned int il = 0; il< InformationSignVector.size(); il++)
		{
			if(InformationSignVector[il].Type != 1  /* congestion waringing or detour VMS */ && Time>=InformationSignVector[il].StartTime && Time<=InformationSignVector[il].EndTime)
			{
				return il;
			}
		}

		return -1;

	}

	float GetTollRateInMin(float Time, int VehicleType)  // from information signs
	{
		for(int il = 0; il< m_TollSize; il++)
		{
			if(Time>=pTollVector[il].StartTime && Time<=pTollVector[il].EndTime)
			{
				return pTollVector[il].TollRateInMin [VehicleType];
			}
		}

		return 0;


	}

	SLinkMOE *m_LinkMOEAry;
	int *aryCFlowA;
	int *aryCFlowD;

	std::vector<CapacityReduction> CapacityReductionVector;
	std::vector<InformationSign> InformationSignVector;
	std::vector<Toll> TollVector;

	int m_TollSize;
	Toll *pTollVector;  // not using SLT here to avoid issues with OpenMP


	


	int m_bMergeFlag;  // 1: freeway and freeway merge, 2: freeway and ramp merge
	std::vector<MergeIncomingLink> MergeIncomingLinkVector;
	int m_MergeOnrampLinkID;
	int m_MergeMainlineLinkID;


	std::list<struc_vehicle_item> LoadingBuffer;  //loading buffer of each link, to prevent grid lock

	std::list<struc_vehicle_item> EntranceQueue;  //link-in queue  of each link
	std::list<struc_vehicle_item> ExitQueue;      // link-out queue of each link


	~DTALink(){
		if(m_LinkMOEAry) delete m_LinkMOEAry;

		if(aryCFlowA) delete aryCFlowA;
		if(aryCFlowD) delete aryCFlowD;

		LoadingBuffer.clear();
		EntranceQueue.clear();
		ExitQueue.clear();

	};

	float GetFreeMovingTravelTime(int TrafficModelFlag=2, float Time = -1)
	{
		if(TrafficModelFlag ==0) // BRP model
			return m_BPRLinkTravelTime;
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
		m_BackwardWaveTimeInSimulationInterval = int(m_Length/g_BackwardWaveSpeed_in_mph*600); // assume backwave speed is 20 mph, 600 conversts hour to simulation intervals

		CFlowArrivalCount = 0;
		CFlowDepartureCount = 0;

		LinkOutCapacity = 0;
		LinkInCapacity = 0;

		VehicleCount = 0;

		int t;


		for(t=0; t<=MAX_TIME_INTERVAL_ADCURVE; t++)
		{
			aryCFlowA[t] = 0;
			aryCFlowD[t] = 0;

		}

		for(t=0; t<=m_SimulationHorizon; t++)
		{
			m_LinkMOEAry[t].SetupMOE();
		}
		LoadingBuffer.clear();
		EntranceQueue.clear();
		ExitQueue.clear();


	}
	int m_LinkID;
	int m_FromNodeID;  // index starting from 0
	int m_ToNodeID;    // index starting from 0

	int m_FromNodeNumber;
	int m_ToNodeNumber;

	float	m_Length;  // in miles
	float    m_VehicleSpaceCapacity; // in vehicles
	int	m_NumLanes;
	int	m_SpeedLimit;
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

	int CFlowArrivalCount;
	int CFlowDepartureCount;

	unsigned int LinkOutCapacity;  // unit: number of vehiles
	int LinkInCapacity;   // unit: number of vehiles

	int VehicleCount;

	int departure_count;
	float total_departure_based_travel_time;




	float GetSpeed(int time)
	{
		return m_Length/GetTravelTime(time,1)*60.0f;  // 60.0f converts min to hour, unit of speed: mph
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

	float GetPrevailingTravelTime(int CurrentTime)
	{
		if(GetNumLanes(CurrentTime)<0.1)   // road blockage
			return 120; // unit min

		if(departure_count >= 1 && CurrentTime >0)
		{
			return total_departure_based_travel_time/departure_count;
		}
		else { // CurrentTime ==0 or departure_count ==0 
			return  m_FreeFlowTravelTime;
		}

	};


	float GetTravelTime(int starting_time, int time_interval)
	{
		float travel_time  = 0.0f;

		if(GetNumLanes(starting_time)<0.1)   // road blockage
			return 120; // unit min

		int total_flow = m_LinkMOEAry[min(starting_time+time_interval, m_SimulationHorizon)].CumulativeArrivalCount - m_LinkMOEAry[starting_time].CumulativeArrivalCount;

		if(total_flow >= 1)
		{
			float total_travel_time = 0;
			int time_end = min(starting_time+time_interval, m_SimulationHorizon);
			for(int t=starting_time; t< time_end; t++)
			{
				total_travel_time += m_LinkMOEAry[t].TotalTravelTime;
			}

			travel_time =  total_travel_time/total_flow;

			if(travel_time < m_FreeFlowTravelTime)
				travel_time = m_FreeFlowTravelTime; // minimum travel time constraint for shortest path calculation

		}
		else
			travel_time =  m_FreeFlowTravelTime;

		ASSERT(travel_time>=0.09);

		return travel_time;

	};


};






// link element of a vehicle path
class SVehicleLink
{  public:
unsigned short  LinkID;  // range:
float AbsArrivalTimeOnDSN;     // absolute arrvial time at downstream node of a link: 0 for the departure time, including delay/stop time
//   float LinkWaitingTime;   // unit: 0.1 seconds
SVehicleLink()
{
	LinkID = MAX_LINK_NO;
	AbsArrivalTimeOnDSN = 0;
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
	SVehicleLink *m_aryVN; // link list arrary of a vehicle path

	unsigned int m_RandomSeed;
	int m_VehicleID;  //range: +2,147,483,647

	unsigned short m_OriginZoneID;  //range 0, 65535
	unsigned short m_DestinationZoneID;  // range 0, 65535

	unsigned char m_VehicleType;     // 1: passenger,  2, HOV, 2, truck, 3: bus
	unsigned char m_InformationClass;  // 1: historical, 2: pre-trip, 3: en-route
	unsigned char m_Occupancy;  // 1: LOV, 2: HOV
	unsigned short m_SimLinkSequenceNo; //  range 0, 65535

	bool  m_bImpacted;

	int m_TimeToRetrieveInfo;  // in simulation interval
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
		m_SimLinkSequenceNo = 0;


		m_aryVN = NULL;
		m_NodeSize	= 0;
		m_bImpacted = false; 
		m_InformationClass = 1;
		m_VehicleType = 1;
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
		if(m_aryVN != NULL)
			delete m_aryVN;

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



	float GetRandomRatio()
	{
		m_RandomSeed = (LCG_a * m_RandomSeed + LCG_c) % LCG_M;  //m_RandomSeed is automatically updated.

		return float(m_RandomSeed)/LCG_M;
	}

public:


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
	int m_InformationClass;
	float    m_DepartureTime;
	int m_TimeToRetrieveInfo;


	bool operator<(const DTA_vhc_simple &other) const
	{
		return m_DepartureTime < other.m_DepartureTime;
	}

};


class VehicleArrayForOriginDepartrureTimeInterval
{
public:
	int Size;
	int* VehicleArray;

	void CreateArray()
	{
		if(Size > 0)
		{
			VehicleArray = new int[Size];
		}
	}

	VehicleArrayForOriginDepartrureTimeInterval()
	{
		Size = 0;
	}
	~VehicleArrayForOriginDepartrureTimeInterval()
	{
		if(Size > 0)
		{
			delete VehicleArray;
		}

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

class DTANetworkForSP  // mainly for shortest path calculation, not just physical network
	// for shortes path calculation between zone centroids, for origin zone, there are only outgoing connectors, for destination zone, only incoming connectors
	// different shortest path calculations have different network structures, depending on their origions/destinations
{
public:
	int m_AssignmentIntervalSize;
	int m_NodeSize;
	int m_PhysicalNodeSize;
	int m_SimulationHorizon;
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

	int m_Number_of_CompletedVehicles;
	int m_AdjLinkSize;

	DTANetworkForSP(int NodeSize, int LinkSize, int TimeSize,int AdjLinkSize){
		m_NodeSize = NodeSize;
		m_LinkSize = LinkSize;

		m_SimulationHorizon = TimeSize;
		m_AdjLinkSize = AdjLinkSize;


		m_OutboundSizeAry = new int[m_NodeSize];
		m_InboundSizeAry = new int[m_NodeSize];


		m_OutboundNodeAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);
		m_OutboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);
		m_InboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);


		m_LinkList = new int[m_NodeSize];

		m_AssignmentIntervalSize = int(TimeSize/g_DepartureTimetInterval)+1;  // make sure it is not zero
		m_LinkTDTimeAry   =  AllocateDynamicArray<float>(m_LinkSize,m_AssignmentIntervalSize);
		m_LinkTDCostAry   =  AllocateDynamicArray<float>(m_LinkSize,m_AssignmentIntervalSize);

		m_FromIDAry = new int[m_LinkSize];

		m_ToIDAry = new int[m_LinkSize];

		NodeStatusAry = new int[m_NodeSize];                    // Node status array used in KSP;
		NodePredAry = new int[m_NodeSize];
		LabelTimeAry = new float[m_NodeSize];                     // label - time
		LabelCostAry = new float[m_NodeSize];                     // label - cost

		if(m_OutboundSizeAry==NULL || m_LinkList==NULL || m_FromIDAry==NULL || m_ToIDAry==NULL  ||
			NodeStatusAry ==NULL || NodePredAry==NULL || LabelTimeAry==NULL || LabelCostAry==NULL)
		{
			cout << "Error: insufficent memory.";
			g_ProgramStop();
		}

	};

	DTANetworkForSP();

	void Init(int NodeSize, int LinkSize, int TimeSize,int AdjLinkSize)
	{
		m_NodeSize = NodeSize;
		m_LinkSize = LinkSize;

		m_SimulationHorizon = TimeSize;
		m_AdjLinkSize = AdjLinkSize;


		m_OutboundSizeAry = new int[m_NodeSize];
		m_InboundSizeAry = new int[m_NodeSize];


		m_OutboundNodeAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);
		m_OutboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);
		m_InboundLinkAry = AllocateDynamicArray<int>(m_NodeSize,m_AdjLinkSize);


		m_LinkList = new int[m_NodeSize];

		m_AssignmentIntervalSize = int(TimeSize/g_DepartureTimetInterval)+1;  // make sure it is not zero
		m_LinkTDTimeAry   =  AllocateDynamicArray<float>(m_LinkSize,m_AssignmentIntervalSize);
		m_LinkTDCostAry   =  AllocateDynamicArray<float>(m_LinkSize,m_AssignmentIntervalSize);

		m_FromIDAry = new int[m_LinkSize];

		m_ToIDAry = new int[m_LinkSize];

		NodeStatusAry = new int[m_NodeSize];                    // Node status array used in KSP;
		NodePredAry = new int[m_NodeSize];
		LabelTimeAry = new float[m_NodeSize];                     // label - time
		LabelCostAry = new float[m_NodeSize];                     // label - cost

		if(m_OutboundSizeAry==NULL || m_LinkList==NULL || m_FromIDAry==NULL || m_ToIDAry==NULL  ||
			NodeStatusAry ==NULL || NodePredAry==NULL || LabelTimeAry==NULL || LabelCostAry==NULL)
		{
			cout << "Error: insufficent memory.";
			g_ProgramStop();
		}

	};


	~DTANetworkForSP()
	{
		if(m_OutboundSizeAry)  delete m_OutboundSizeAry;
		if(m_InboundSizeAry)  delete m_InboundSizeAry;

		DeallocateDynamicArray<int>(m_OutboundNodeAry,m_NodeSize, m_AdjLinkSize);
		DeallocateDynamicArray<int>(m_OutboundLinkAry,m_NodeSize, m_AdjLinkSize);
		DeallocateDynamicArray<int>(m_InboundLinkAry,m_NodeSize, m_AdjLinkSize);


		if(m_LinkList) delete m_LinkList;

		DeallocateDynamicArray<float>(m_LinkTDTimeAry,m_LinkSize,m_AssignmentIntervalSize);
		DeallocateDynamicArray<float>(m_LinkTDCostAry,m_LinkSize,m_AssignmentIntervalSize);

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
	void BuildPhysicalNetwork();
	void IdentifyBottlenecks(int StochasticCapacityFlag);

	bool TDLabelCorrecting_DoubleQueue(int origin, int departure_time, int vehicle_type);   // Pointer to previous node (node)

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



int g_read_integer(FILE* f);
int g_read_integer_with_char_O(FILE* f);

float g_read_float(FILE *f);

void ReadNetworkTables();
void CreateVehicles(int originput_zone, int destination_zone, float number_of_vehicles, int vehicle_type, float starting_time_in_min, float ending_time_in_min);


void Assignment_MP(int id, int nthreads, int node_size, int link_size, int iteration);

struct NetworkLoadingOutput
{
public:
	float AvgTravelTime;
	float AvgDelay;
	float AvgTTI;
	float AvgDistance;
	int   NumberofVehiclesCompleteTrips;
	int   NumberofVehiclesGenerated;
	float SwitchPercentage;
};

NetworkLoadingOutput g_NetworkLoading(int TrafficFlowModelFlag, int SimulationMode);  // NetworkLoadingFlag = 0: static traffic assignment, 1: vertical queue, 2: spatial queue, 3: Newell's model, 

void Assignment_MP(int id, int nthreads, int node_size, int link_size, int iteration);


void OutputLinkMOEData(char fname[_MAX_PATH], int Iterationbool,bool bStartWithEmpty);
void OutputNetworkMOEData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty);
void OutputVehicleTrajectoryData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty);
void OutputAssignmentMOEData(char fname[_MAX_PATH], int Iteration,bool bStartWithEmpty);

float g_GetPrivateProfileFloat( LPCTSTR section, LPCTSTR key, float def_value, LPCTSTR filename);
int g_GetPrivateProfileInt( LPCTSTR section, LPCTSTR key, int def_value, LPCTSTR filename);



float GetStochasticCapacity(bool bQueueFlag, float CurrentCapacity);

float GetDynamicCapacityAtSignalizedIntersection(float HourlyCapacity, float CycleLength_in_seconds,double CurrentTime);

float g_GetRandomRatio();
int g_GetRandomInteger_From_FloatingPointValue(float Value);

void ReadDTALiteVehicleFile(char fname[_MAX_PATH], DTANetworkForSP* pPhysicalNetwork);
void ReadDemandFile(DTANetworkForSP* pPhysicalNetwork);

void g_VehicleRerouting(int v, float CurrentTime, InformationSign is); // v for vehicle id
void g_DynamicTrafficAssisnment();
void g_StaticTrafficAssisnment();
void g_OneShotNetworkLoading();
void g_MultiDayTrafficAssisnment();
void OutputMultipleDaysVehicleTrajectoryData(char fname[_MAX_PATH]);
int g_OutputSimulationSummary(float& AvgTravelTime, float& AvgDistance, float& AvgSpeed,
							  int InformationClass, int VehicleType,int DepartureTimeInterval);


void g_OutputVOCMOEData(char fname[_MAX_PATH]);

extern CString g_GetAppRunningTime();
void g_ComputeFinalGapValue();

struct PathArrayForEachODT // Jason : store the path set for each OD pair and each departure time interval
{
	int   NumOfPaths;
	int   PathNodeSums[100];            // max 100 path for each ODT
	int   NumOfVehsOnEachPath[100]; 	
	int   PathLinkSequences[100][100];	// max 100 links on each path
	int   PathSize[100];				// number of nodes on each path
	//	int   BestPath[100];				// the link sequence of the best path for each ODT
	int   BestPathIndex;				// index of the best (i.e., least experienced time) path for each ODT
	float AvgPathTimes[100]; 	
};

#define MAX_SIZE_INFO_USERS 5 

extern float g_UserClassPercentage[MAX_SIZE_INFO_USERS];
extern float g_UserClassPerceptionErrorRatio[MAX_SIZE_INFO_USERS];
extern float g_VMSPerceptionErrorRatio;
extern int g_information_updating_interval_of_en_route_info_travelers_in_min;

extern int g_K_HistInfo_ShortestPath;