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
#pragma	warning(disable: 4018) // signed/unsigned mismatch
#pragma	warning(disable: 4305) //  truncation from 'double' to 'float'
#pragma	warning(disable: 4995) //  'CDaoRecordset': name was marked as #pragma deprecated
#pragma	warning(disable: 4267) //  'initializing' : conversion from 'size_t' to 'int', possible loss of data

#include "resource.h"
#include "Utility.h"

#include <math.h>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <list>



enum DTA_Approach
{
	DTA_North = 0,
	DTA_South,
	DTA_East,
	DTA_West,
	DTA_NorthEast,
	DTA_NorthWest,
	DTA_SouthEast,
	DTA_SouthWest
};

enum DTA_Turn
{
	DTA_LeftTurn,
	DTA_Through,
	DTA_RightTurn,
	DTA_OtherTurn,
	DTA_LeftTurn2,
	DTA_RightTurn2,
};


enum DTA_APPROACH_TURN
   {
	    DTA_LANES_COLUME_init = -1,
		DTA_NBL2 = 0,
		DTA_NBL,
		DTA_NBT,
		DTA_NBR,
		DTA_NBR2,
		DTA_SBL2,
		DTA_SBL,
		DTA_SBT,
		DTA_SBR,
		DTA_SBR2,
		DTA_EBL2,
		DTA_EBL,
		DTA_EBT,
		DTA_EBR,
		DTA_EBR2,
		DTA_WBL2,
		DTA_WBL,
		DTA_WBT,
		DTA_WBR,
		DTA_WBR2,
		DTA_NEL,
		DTA_NET,
		DTA_NER,
		DTA_NWL,
		DTA_NWT,
		DTA_NWR,
		DTA_SEL,
		DTA_SET,
		DTA_SER,
		DTA_SWL,
		DTA_SWT,
		DTA_SWR
   };

enum LANES_ROW
   {
	   LANES_UpNode = 0,
	   LANES_DestNode,
		LANES_Lanes,
		LANES_Shared,
		LANES_Width,
		LANES_Storage,
		LANES_StLanes,
		LANES_Grade,
		LANES_Speed,
		LANES_FirstDetect,
		LANES_LastDetect,
		LANES_Phase1,
		LANES_PermPhase1,
		LANES_DetectPhase1,
		LANES_IdealFlow,
		LANES_LostTime,
		LANES_SatFlow,
		LANES_SatFlowPerm,
		LANES_SatFlowRTOR,
		LANES_HeadwayFact,
		LANES_Volume,
		LANES_Peds,
		LANES_Bicycles,
		LANES_PHF,
		LANES_Growth,
		LANES_HeavyVehicles,
		LANES_BusStops,
		LANES_Midblock,
		LANES_Distance,
		LANES_TravelTime
   };

enum PHASE_ROW
{
	PHASE_BRP =0,
	PHASE_MinGreen,
	PHASE_MaxGreen,
	PHASE_VehExt,
	PHASE_TimeBeforeReduce,
	PHASE_TimeToReduce,
	PHASE_MinGap,
	PHASE_Yellow,
	PHASE_AllRed,
	PHASE_Recall,
	PHASE_Walk,
	PHASE_DontWalk,
	PHASE_PedCalls,
	PHASE_MinSplit,
	PHASE_DualEntry,
	PHASE_InhibitMax,
	PHASE_Start,
	PHASE_End,
	PHASE_Yield,
	PHASE_Yield170,
	PHASE_LocalStart,
	PHASE_LocalYield,
	PHASE_LocalYield170
};

enum TIMING_ROW
{
	TIMING_Control_Type =0,
	TIMING_Cycle_Length,
	TIMING_Lock_Timings,
	TIMING_Referenced_To,
	TIMING_Reference_Phase,
	TIMING_Offset,
	TIMING_Master,
	TIMING_Yield,
	TIMING_Node_0,
	TIMING_Node_1
};


#include "Timetable.h"

extern float g_GetRandomRatio();

using namespace std;
using std::string;
#define PI 3.1415926
#define MAX_LOS_SIZE  8

#define MAX_RANDOM_SAMPLE_SIZE 100

#define MAX_AdjLinkSize 15
#define	MAX_SPLABEL 99999
#define MAX_NODE_SIZE_IN_A_PATH 4000

#define NUM_PATHMOES 8  // Distance, Travel Time, Emissions, Safety
#define NUM_VEHPATHMOES 8  // Distance, Travel Time, Emissions, Safety
#define NUM_PATHS   6
#define MAX_VEHICLE_TYPE_SIZE 10
#define MAX_TIME_INTERVAL_SIZE 96
#define MAX_INFO_CLASS_SIZE 4
#define MAX_PRICING_TYPE_SIZE  5


#define _MAXIMUM_OPERATING_MODE_SIZE 41
#define MAX_TRAIN_TYPE_SIZE 2
#define TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL 5
#define MAX_DAY_SIZE 1 

struct GDPoint
{
	double x;
	double y;
};

struct GDRect
{
	double left, right,top, bottom;

	double Height() { return top - bottom; }
	double Width()  { return right - left; }

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

#include "RailNetwork\\RailNetwork.h"

typedef struct{
	float speed;
	float traveltime;
}struc_traffic_state;

class CVehicleEmission 
{
public:
	float Energy;
	float CO2;
	float NOX;
	float CO;
	float HC;
	CVehicleEmission()
	{
		Energy = 0;
		CO2 = 0;
		NOX = 0;
		CO = 0;
		HC = 0;

	}
};

extern float g_P2P_Distance(GDPoint p1, GDPoint p2);

extern DTA_Turn g_RelativeAngle_to_Turn(int RelativeAngle);

extern float g_DistancePointLine(GDPoint pt, GDPoint FromPt, GDPoint ToPt);
extern double g_CalculateP2PDistanceInMileFromLatitudeLongitude(GDPoint p1, GDPoint p2);
extern bool g_get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y) ;


/////

class DTAActivityLocation
{ 

public: 
	bool m_bWithinSubarea;
	DTAActivityLocation()
	{
		m_bWithinSubarea = true;
		External_OD_flag = 0;
	}

  int ZoneID;
  int NodeNumber;
  int External_OD_flag;  // 0: land use activity, 1: external origin, -1: external destination
  
};

class DTATimeDependentemand
{
public: 
	int movement_hourly_capacity;
	int starting_time_in_min;
	int ending_time_in_min;
	float time_dependent_value;
	int type;
	

	DTATimeDependentemand()
	{
	movement_hourly_capacity = -1;
	starting_time_in_min = 0;
	ending_time_in_min = 1440;
	time_dependent_value = 0;
	}

};

class DTADemandMetaDataType
{
public:
	string format_type;
	CString key;
	int demand_table_type_no;
	int trip_type;
	int start_time_in_min;
	int end_time_in_min;
	float subtotal_demand_volume;

	DTADemandMetaDataType()
	{
	format_type = "3 column";
	subtotal_demand_volume = 0;
	}

};
class DTADemandVolume
{
public:
	std::map<int, float> TypeValue;

	float total_demand;
	float travel_cost;
	float friction;

	DTADemandVolume()
	{
	travel_cost = 1;
	friction = 1;
	total_demand = 0;
	}

	void SetValue(int demand_type, float value)
	{
	TypeValue[demand_type]= value;
	total_demand += value;
	}

	float GetValue(int demand_type)
	{
	if( TypeValue.find(demand_type) != TypeValue.end())
		return TypeValue[demand_type];
	else
		return 0;
	}

	float GetSubTotalValue()
	{
	return total_demand;
	}


};
class DTAZone
{ 
public:

	string notes;
	string color_code;
	float  m_Height;
	float  m_Population;
	float m_Employment;
	float m_MedianIncome;
	float m_GrowthFactor;
	int m_CBDFlag;

	bool m_bWithinSubarea;

	float GetTotalZonalDemand()
	{
	float total_zonal_demand  = 0;

		std::map<int, DTADemandVolume> :: iterator itr;

		for(itr = m_ODDemandMatrix.begin(); itr != m_ODDemandMatrix.end(); itr++)
			{
				total_zonal_demand += m_ODDemandMatrix[itr->first ]. GetSubTotalValue ();
			}

		return total_zonal_demand;

	}
	std::vector<GDPoint> m_ShapePoints;
	std::map<int, DTADemandVolume> m_ODDemandMatrix;

	std::map<int, float> m_TotalTimeDependentZoneDemand;
	void SetTDZoneValue(int time_interval, float value)
	{
		m_TotalTimeDependentZoneDemand[time_interval] += value;

	}
	int m_ZoneTAZ;
	int m_OriginVehicleSize;  // number of vehicles from this origin, for fast acessing
	std::vector<DTAActivityLocation> m_ActivityLocationVector;

	float m_Production;
	float m_Attraction;

	bool bInitialized;
	GDPoint m_Center;

	GDPoint GetCenter()
	{
		if(bInitialized)
			return m_Center;
		else
		{  // not initialized yet
			m_Center.x = 0;
			m_Center.y = 0;
	
		for(unsigned int i = 0; i< m_ShapePoints.size(); i++)
		{
			m_Center.x += m_ShapePoints[i].x;
			m_Center.y += m_ShapePoints[i].y;

		}

		m_Center.x /= max(1,m_ShapePoints.size());
		m_Center.y /= max(1,m_ShapePoints.size());

		bInitialized = true;
				return m_Center;
		}

	}

	void RemoveNodeActivityMode(int NodeNumber)
	{
		for(unsigned int i = 0; i< m_ActivityLocationVector.size(); i++)
		{
		 if(m_ActivityLocationVector[i].NodeNumber ==NodeNumber)
		 {
		 m_ActivityLocationVector.erase(m_ActivityLocationVector.begin() + i);
		 return;
		 }
		}
	}

	void SetNodeActivityMode(int NodeNumber, int External_OD_flag)
	{
		RemoveNodeActivityMode(NodeNumber);  // remove existing record first
		DTAActivityLocation element;
		element.ZoneID = m_ZoneTAZ;
		element.NodeNumber = NodeNumber;
		element.External_OD_flag = External_OD_flag;
		m_ActivityLocationVector.push_back (element);
	}



	bool FindANode(int NodeNumber)
	{

		for(unsigned int i = 0; i< m_ActivityLocationVector.size(); i++)
		{
		 if(m_ActivityLocationVector[i].NodeNumber ==NodeNumber)
			 return true;
		}
	
		return false;
	}


	DTAZone()
	{
		m_Height = 0;
		color_code = "red";
		m_Capacity  =0;
		m_Demand = 0;
		m_OriginVehicleSize = 0;
		bInitialized = false;

		m_Production = 0;
		m_Attraction = 0;

		m_Population =0;
		m_Employment =0;
		m_MedianIncome = 0;
		m_GrowthFactor = 0;
		m_CBDFlag = 0;

	}

	BOOL IsInside(GDPoint pt)
	{

	double MinX  = pt.x ; 
	double MinY  = pt.y;

	double MaxX  = pt.x ; 
	double MaxY  = pt.y;

  int num_points = (int)(m_ShapePoints.size());
  for(unsigned i = 0; i < num_points; i++)
   {
     MinX = min(m_ShapePoints[i].x, MinX);
     MinY = min(m_ShapePoints[i].y, MinY);

     MaxX = max(m_ShapePoints[i].x, MaxX);
     MaxY = max(m_ShapePoints[i].y, MaxY);

  }

  double resolution_x = 1000/max(0.0000001,(MaxX - MinX));
  double resolution_y = 1000/max(0.0000001,(MaxY - MinY));

   LPPOINT zpts = new POINT[num_points];

   for(unsigned i = 0; i < num_points; i++)
   {
      zpts[i].x =  (int)((m_ShapePoints[i].x - MinX)*resolution_x+0.5);
      zpts[i].y =  (int)((m_ShapePoints[i].y - MinY)*resolution_y+0.5);
   }

   // Create a polygonal region
   HRGN hrgn = CreatePolygonRgn(zpts, num_points, WINDING);

   POINT current_point;
     current_point.x =  (int)((pt.x - MinX)*resolution_x+0.5);
      current_point.y =  (int)((pt.y - MinY)*resolution_y+0.5);


    BOOL bInside =  PtInRegion(hrgn, current_point.x , current_point.y );

	if(zpts!=NULL)
		delete zpts;

	return bInside;
	}

	
	float m_Capacity;
	float m_Demand;


};

class DTADemand
{
public:
	int from_zone_id;
	int to_zone_id;
	int starting_time_in_min;
	int ending_time_in_min;
	std::vector<float> number_of_vehicles_per_demand_type;

	DTADemand()
	{


	}	

};


class DTADemandProfile
{
public:
	int from_zone_id;
	int to_zone_id;
	double time_dependent_ratio[MAX_TIME_INTERVAL_SIZE];
	int demand_type;
	CString series_name;
	DTADemandProfile()
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
	int vehicle_type;
	CString vehicle_type_name;
};


class DTADemandType
{
public:
	int demand_type;
	float average_VOT;
	int pricing_type; // 1: SOV, 2: HOV, 3: truck, 4: intermodal
	float info_class_percentage[MAX_INFO_CLASS_SIZE];
	float vehicle_type_percentage[MAX_VEHICLE_TYPE_SIZE];

	DTADemandType()
	{
		for(int vehicle_type = 0; vehicle_type < MAX_VEHICLE_TYPE_SIZE; vehicle_type++)
		{
			vehicle_type_percentage[vehicle_type] = 0;
		}
		for(int info_class = 0; info_class < MAX_INFO_CLASS_SIZE; info_class++)
		{
			info_class_percentage[info_class] = 0;
		}
		info_class_percentage[0] = 1.0;
	}

	CString demand_type_name;
};

class DTALinkType
{
public:
	DTALinkType()
	{
	link_type = 0;
	default_lane_capacity = 1000;  // per hour per lane
	}

	float default_lane_capacity;
	int link_type;
	string link_type_name;
	string type_code;
	int safety_prediction_model_id;
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

class DTANodeType
{
public:
	int node_type;
	string node_type_name;
	int pretimed_signal_flag;
	int actuated_signal_flag;
	int stop_sign_flag;
	int yield_sign_flag;
};

class DTAVOTDistribution
{
public:
	int demand_type;
	float VOT;
	float percentage;
};
class DTANodeMovement
{
public:
	DTANodeMovement()
	{
	starting_time_in_min = 0;
	ending_time_in_min = 1440;
	turnning_percentage = 0;
	turning_prohibition_flag = 1;
	signal_control_no = 0;
	signal_group_no = 0;
	phase_index = 0;
	turn_volume = 10;
	movement_hourly_capacity = 1000;

	}

int IncomingLinkID;
int OutgoingLinkID;
DTA_Turn movement_turn;
DTA_Approach movement_approach;
DTA_APPROACH_TURN movement_dir;

int in_link_from_node_id;
int in_link_to_node_id;  // this equals to the current node number
int out_link_to_node_id;

float movement_hourly_capacity;
int starting_time_in_min;
int ending_time_in_min;
float turnning_percentage;
int turning_prohibition_flag;
int phase_index;
int signal_control_no;  // for meso-scopic, link -based
int signal_group_no;  // for meso-scopic, link -based
int turn_volume;
};

class DTANodeLaneTurn
{
public:
	DTANodeMovement movement_info;

int in_lane_index;
int out_lane_index;

int signal_control_no; // micro-scopic, lane-based
int signal_group_no; // micro-scopic, lane-based
};


  class DTANodePhase
   {
   public:
      DTANodePhase()
	  {
      min_green = 5;
	  max_green = 60;
	  amber = 2;
	  }

      int phase_number;
      int min_green;
	  int max_green;
	  int amber;

      std::vector<int> movement_index_vector;

	  // if a link is added or deleted from a link, the corresponding movement array should be adjusted. 

	  bool MovementIncluded(int MovementIndex)
	  {
		  
		  for(unsigned int m = 0; m < movement_index_vector.size(); m++)
		  {
			  if ( movement_index_vector[m] == MovementIndex)
				  return true;
		  
		  }
	  
		  return false;
	  
	  }
	};

class DTANodeSignal
{
public:
   DTANodeSignal();
   ~DTANodeSignal();

  int cycle_length;
   std::vector<DTANodePhase> phase_vector;

};

class DTANode
{
public:
	DTANode()
	{
		m_bZoneActivityLocationFlag = false;
		m_NodeNumber = 0;
		m_NodeOriginalNumber = 0;
		m_ControlType = 0;
		m_ZoneID = 0;
		m_TotalCapacity = 0;
		m_Connections = 0;
		m_LayerNo = 0;
		m_DistanceToRoot = 0;
		m_CycleLength =0;
		m_NumberofPhases = 0;
		m_bSignalData = false;
		m_External_OD_flag = 0;
		m_tobeRemoved = false;
		m_bSubareaFlag = 0;  // when the associated link is removed as it is outside the boundary, then we mark its from and t nodes as subarea boundary node 
		m_CentroidUpdateFlag = 0;
	};


	~DTANode(){};

	int m_CentroidUpdateFlag;  // used by node splitting for subarea
	int m_bSubareaFlag;
	bool m_tobeRemoved;
	bool m_bZoneActivityLocationFlag; 

	int m_External_OD_flag;

	std::vector<int> m_OutgoingLinkVector;
	std::vector<int> m_IncomingLinkVector;
	std::vector <DTANodeMovement> m_MovementVector;

	int GetMovementIndex(int in_link_from_node_id, int in_link_to_node_id, int out_link_to_node_id)
	{

		for(unsigned int i  = 0; i < m_MovementVector.size(); i++)
		{

		if( m_MovementVector[i].in_link_from_node_id== in_link_from_node_id
		&& m_MovementVector[i].in_link_to_node_id== in_link_to_node_id
		&& m_MovementVector[i].out_link_to_node_id== out_link_to_node_id)
		return i;
		}

		return -1;  //not found

	}

	std::vector <DTANodeLaneTurn> m_LaneTurnVector;

	std::vector <DTANodePhase> m_PhaseVector;
	 
	int m_CycleLength;
	int m_NumberofPhases;
	float m_DistanceToRoot;
	string m_Name;
	GDPoint pt;
	GDPoint schedule_pt;

	bool m_bSignalData;
	int m_LayerNo;
	int m_NodeNumber;  //  original node number
	int m_NodeOriginalNumber;  //  original node number
	int m_NodeID;  ///id, starting from zero, continuous sequence
	int m_ZoneID;  // If ZoneID > 0 --> centroid,  otherwise a physical node.
	int m_ControlType; // Type: ....
	float m_TotalCapacity;
	int m_Connections;  // number of connections

	// DTA_NodeMovementSet m_MovementSet;

};

class DTAPoint
{
public:
	DTAPoint(){
		m_NodeNumber = 0;
		m_ControlType = 0;
		m_ZoneID = 0;
		m_TotalCapacity = 0;
		m_Connections = 0;
		m_LayerNo = 0;
		m_DistanceToRoot = 0;
	};
	~DTAPoint(){};


	float m_DistanceToRoot;
	string m_Name;
	GDPoint pt;
	int m_LayerNo;
	int m_NodeNumber;  //  original node number
	int m_NodeID;  ///id, starting from zero, continuous sequence
	int m_ZoneID;  // If ZoneID > 0 --> centroid,  otherwise a physical node.
	int m_ControlType; // Type: ....
	float m_TotalCapacity;
	int m_Connections;  // number of connections

};

class DTALine
{
public:
	int LineID;
	std::vector<GDPoint> m_ShapePoints;
};
// event structure in this "event-basd" traffic simulation
typedef struct{
	int veh_id;
	float time_stamp;
}struc_vehicle_item;



class SLinkMOE  // time-dependent link MOE
{
public:
	float ObsQueueLength;
	float ObsTravelTimeIndex;

	float ObsSpeed;  // speed
	float ObsLinkFlow;   // flow volume
	float ObsDensity;   // ObsDensity

	// these three copies are used to compare simulation results and observed results
	float ObsSpeedCopy;  // speed
	float ObsFlowCopy;   // flow volume
	float ObsDensityCopy;   // ObsDensity
	float ObsTravelTimeIndexCopy;

	float ObsCumulativeFlow;   // flow volume
	float ObsCumulativeFlowCopy;   // flow volume

	int CumulativeArrivalCount_PricingType[MAX_PRICING_TYPE_SIZE];
	float CumulativeRevenue_PricingType[MAX_PRICING_TYPE_SIZE];


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
		ObsQueueLength = 0;
		ObsTravelTimeIndex = 0;
		ObsSpeed = 0;
		ObsLinkFlow = 0;
		ObsCumulativeFlow = 0;
		ObsDensity = 0;

		PredSpeed = 0;
		PredTTI15 = 0;
		FullCapacity = 0;
		ReducedCapacity = 0;
		EventStartTiming = 0;

		// these four copies are used to compare simulation results and observed results
		ObsSpeedCopy = 0;
		ObsFlowCopy = 0;
		ObsDensityCopy = 0;
		ObsTravelTimeIndexCopy = 0;

	};

	void SetupMOE(float FreeFlowTravelTime, float SpeedLimit)
	{
		ObsQueueLength = 0;
		ObsTravelTimeIndex = FreeFlowTravelTime;
		ObsTravelTimeIndexCopy = FreeFlowTravelTime;

		ObsSpeed = SpeedLimit;
		ObsLinkFlow = 0;
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
	long m_LinkNo;
	int m_link_type;
	int m_NumLanes;
	float m_LinkInCapacityRatio;
};


class CapacityReduction
{
public:
	CapacityReduction()
	{
		StartDayNo = 0;
		EndDayNo = 0;

		for (int i=0;i<MAX_RANDOM_SAMPLE_SIZE;i++)
		{
			CapacityReductionSamples[i] = 0.f;
			AdditionalDelaySamples[i] = 0.f;
		}

		COVRegularCapacityReduction = 0.1f;

		bWorkzone = false;
		bIncident = false;
	}

	bool bWorkzone;
	bool bIncident;

	float StartTime;
	float EndTime;
	int StartDayNo;
	int EndDayNo;
	float LaneClosureRatio;
	float SpeedLimit;
	float ServiceFlowRate;

	float MeanRegularCapacityReduction;  // from link data
	float COVRegularCapacityReduction;


	float MeanIncidentCapacityReduction;
	float COVIncidentCapacityReduction;

	float ProbabilityIncidentOccurance;

	float CapacityReductionSamples[MAX_RANDOM_SAMPLE_SIZE];
	float AdditionalDelaySamples[MAX_RANDOM_SAMPLE_SIZE];

	void GenerateAdditionalDelayDistribution(float EntranceTime,int VehicleID);

};

class MessageSign
{
public:

	int StartDayNo;
	int EndDayNo;
	float StartTime;
	float EndTime;
	float ResponsePercentage;

	/*
	int   BestPathFlag;
	int   DetourLinkSize;
	int   DetourLinkArray[MAX_LINK_SIZE_IN_VMS];
	*/

	MessageSign()
	{

	}

	~MessageSign()
	{
	}

};

class DTAToll
{
public:
	DTAToll()
	{
	DayNo = 0;

		for(int p = 0; p  < MAX_PRICING_TYPE_SIZE; p++)
		{
		TollRate[p] = 0;
		TollRateInMin [p] = 0;
		}
	}

	float StartTime;
	float EndTime;
	int DayNo;
	float TollRate[MAX_PRICING_TYPE_SIZE];
	float TollRateInMin[MAX_PRICING_TYPE_SIZE];
};
class DTALane
{
	// related to link
public: 

int from_node_id;
int to_node_id;
int lane_index; // from left starting from 1

int pocket_length; // 0 as normal, > 0 left turn with value
int channel_length; // 0 as normal  > 0 rightturn channel with value
int left_turn; //1 permitted, 0 not allowed 
int through; //1 permitted, 0 not allowed 
int right_turn; //1 permitted, 0 not allowed 
int signal_control_no; // signal control number 
int signal_group_no; // group number 
};

class DTALink
{
public:

	DTALink(int TimeHorizon)  // TimeHorizon's unit: per min
	{
		m_CentroidUpdateFlag = 0; 
		m_bTrainFromTerminal = false;
		m_bTrainToTerminal = false;
		m_bConnector = false;
		m_ConnectorZoneID = 0;
		m_NumberOfLeftTurnBay = 0;
		m_LeftTurnBayLengthInFeet = 0;	
		m_LeftTurnCapacity = 0;

		m_bOneWayLink = true;
		m_BandWidthValue = 1;
		m_ReferenceBandWidthValue = 0;
		m_SetBackStart = 0;
		m_SetBackEnd = 0;
		m_SpeedLimit  = 10;
		m_ReversedSpeedLimit  = 10;
		m_Saturation_flow_rate_in_vhc_per_hour_per_lane = 2000;

		m_TotalVolume = 0;
		m_MeanSpeed  = m_SpeedLimit;
		m_TotalTravelTime = 0;
		m_TotalDiffValue = 0;
		m_NumberOfMarkedVehicles = 0;
		m_AVISensorFlag = false;
		m_LinkID = 0;
		green_height = 0;
		red_height = 0;

		m_LayerNo = 0;
		m_OrgDir = 1;
		m_RailBidirectionalFlag = 1;
		m_Direction = 1;
		m_ObsHourlyLinkVolume = 0;
		m_SimulationHorizon	= TimeHorizon;
		m_LinkMOEAry.resize(m_SimulationHorizon+1);


		m_StochaticCapcityFlag = 0;
		m_bMergeFlag = 0;
		m_MergeOnrampLinkID = -1;
		m_MergeMainlineLinkID = -1;
		m_bSensorData = false;
		m_SensorID = -1;
		m_OverlappingCost = 0;
		m_DisplayLinkID = -1;

		m_Kjam = 180;
		m_AADT_conversion_factor = 0.1;
		m_Wave_speed_in_mph = 12;

		m_ReliabilityIndex = 100;
		m_SafetyIndex = 100;
		m_EmissionsIndex = 100;
		m_MobilityIndex = 100;


		m_MinSpeed = 40;
		m_MaxSpeed = 40;

		m_ResourceAry = NULL;
		m_NumberOfCrashes = 0;
		m_LevelOfService = 'A';

		m_StaticSpeed = 0;
		m_StaticLinkVolume = 0;
		m_StaticVOC  = 0;
		m_StaticTravelTime = 0;
		m_Grade = 0;

		input_line_no = 0;

		m_bIncludedBySelectedPath = false;
		m_bIncludedinSubarea = false;

		m_AADT = 0;
		m_ReferenceFlowVolume  = 0;
		m_PeakHourFactor = 0.15f;
		m_bToBeShifted = true;
	};


	int m_CentroidUpdateFlag;
	std::vector<DTALane> m_LaneVector;
	// end: for micro simulation 

	float m_AADT;
	float m_ReferenceFlowVolume;
	float m_PeakHourFactor;
	int  m_NumberOfMarkedVehicles;
	float m_BandWidthValue;
	float m_ReferenceBandWidthValue;

	bool m_bIncludedBySelectedPath;
	bool m_bIncludedinSubarea;
	double m_NumberOfCrashes;
	char m_LevelOfService;

	bool m_AVISensorFlag;
	int m_LayerNo;
	float m_Grade;
	string m_Name;

	int input_line_no;
	float m_StaticSpeed, m_StaticLinkVolume;
	float m_StaticTravelTime, m_StaticVOC;

	std::vector<GDPoint> m_ShapePoints;
	bool m_bToBeShifted;

	std::vector<GDPoint> m_BandLeftShapePoints;
	std::vector<GDPoint> m_BandRightShapePoints;

	std::vector<GDPoint> m_ReferenceBandLeftShapePoints;  // second band for observations
	std::vector<GDPoint> m_ReferenceBandRightShapePoints;

	std::vector<float> m_ShapePointRatios;

	void CalculateShapePointRatios()
	{

		m_ShapePointRatios.clear();

		float total_distance = 0; 
		unsigned int si;
		for(si = 0; si < m_ShapePoints .size()-1; si++)
		{
			total_distance += g_P2P_Distance(m_ShapePoints[si],m_ShapePoints[si+1]); 
		}

		if(total_distance < 0.0000001f)
			total_distance = 0.0000001f;

		float distance_ratio = 0;
		float P2Origin_distance = 0;
		m_ShapePointRatios.push_back(0.0f);
		for(si = 0; si < m_ShapePoints .size()-1; si++)
		{
			P2Origin_distance += g_P2P_Distance(m_ShapePoints[si],m_ShapePoints[si+1]);
			m_ShapePointRatios.push_back(P2Origin_distance/total_distance);
		}
	}

	GDPoint GetRelativePosition(float ratio)
	{
		GDPoint Pt;
		Pt.x= (m_ShapePoints[0].x+ m_ShapePoints[m_ShapePoints .size()-1].x)/2;
		Pt.y= (m_ShapePoints[0].y+ m_ShapePoints[m_ShapePoints .size()-1].y)/2;

		unsigned	int si;

		if(m_ShapePointRatios.size() == m_ShapePoints.size())
		{

			for(si = 0; si < m_ShapePoints .size()-1; si++)
			{

				if(ratio > m_ShapePointRatios[si] && ratio < m_ShapePointRatios[si+1])
				{

					float SectionRatio = m_ShapePointRatios[si+1] - m_ShapePointRatios[si];

					float RelateveRatio = 0;
					if(SectionRatio >0)
						RelateveRatio = (ratio - m_ShapePointRatios[si])/SectionRatio;

					Pt.x = m_ShapePoints[si].x + RelateveRatio*(m_ShapePoints[si+1].x - m_ShapePoints[si].x);
					Pt.y = m_ShapePoints[si].y + RelateveRatio*(m_ShapePoints[si+1].y - m_ShapePoints[si].y);

					return Pt;
				}
			}

		}
		return Pt;
	}


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


	float ObtainHistTravelTime(unsigned int time)
	{
		if(time < m_HistLinkMOEAry.size() && m_bSensorData == true)
		{
			return m_Length/m_HistLinkMOEAry[time].ObsSpeed*60;  // *60: hour to min
		}
		else
			return m_FreeFlowTravelTime;
	}

	float ObtainHistFuelConsumption(unsigned int time)
	{
		if(time<m_HistLinkMOEAry.size() && m_bSensorData == true)
		{
			return m_Length*0.1268f*pow( max(1,m_HistLinkMOEAry[time].ObsSpeed),-0.459f);  // Length*fuel per mile(speed), y= 0.1268x-0.459
		}
		else
			return m_Length*0.1268f*pow(m_SpeedLimit,-0.459f);  // Length*fuel per mile(speed_limit), y= 0.1268x-0.459
	}

	float ObtainHistCO2Emissions(unsigned int time)  // pounds
	{
		if(time<m_HistLinkMOEAry.size() && m_bSensorData == true)
		{
			return min(1.4f,m_Length*11.58f*pow(m_HistLinkMOEAry[time].ObsSpeed,-0.818f));  // Length*fuel per mile(speed), y= 11.58x-0.818
		}
		else
			return m_Length*11.58f*pow(m_SpeedLimit,-0.818f);  // Length*fuel per mile(speed_limit), y= 11.58x-0.818
	}

	void ResetMOEAry(int TimeHorizon)
	{
		m_SimulationHorizon	= TimeHorizon;

		m_TotalVolume = 0;
		m_MeanSpeed  = m_SpeedLimit;
		m_TotalTravelTime = 0;
		m_TotalDiffValue = 0;
		m_NumberOfMarkedVehicles = 0;

		int OldSize = (int)(m_LinkMOEAry.size());
		m_LinkMOEAry.resize (m_SimulationHorizon+1);


		int t;
		for(t=OldSize; t<= TimeHorizon; t++)
		{
			m_LinkMOEAry[t].SetupMOE(m_FreeFlowTravelTime,m_SpeedLimit);
		}	
	};

	void ComputeHistoricalAvg(int number_of_weekdays);

	void Compute15MinAvg();

	struc_traffic_state GetPredictedState(int CurrentTime, int PredictionHorizon);  // return value is speed

	CLinkTimeTable m_TimeTable;
	bool m_bTrainFromTerminal;
	bool m_bTrainToTerminal;

	
	GDPoint m_FromPoint, m_ToPoint;
	GDPoint m_ScheduleFromPoint, m_ScheduleToPoint;

	double m_SetBackStart, m_SetBackEnd;
	GDPoint m_FromPointWithSetback, m_ToPointWithSetback;

	double DefaultDistance()
	{
		return pow((m_FromPoint.x - m_ToPoint.x)*(m_FromPoint.x - m_ToPoint.x) + 
			(m_FromPoint.y - m_ToPoint.y)*(m_FromPoint.y - m_ToPoint.y),0.5);
	}

void AdjustLinkEndpointsWithSetBack()
{
   GDPoint Direction;

   double SetBackRatio = m_SetBackStart  /max(0.00001, DefaultDistance());

   Direction.x = (m_ToPoint.x - m_FromPoint.x)*SetBackRatio;
   Direction.y = (m_ToPoint.y - m_FromPoint.y)*SetBackRatio;

   // Adjust start location by this vector
   m_FromPointWithSetback.x = m_ShapePoints[0].x + Direction.x;
   m_FromPointWithSetback.y = m_ShapePoints[0].y + Direction.y;

   SetBackRatio = m_SetBackEnd  /max(0.000001, DefaultDistance());

   Direction.x = (m_FromPoint.x - m_ToPoint.x)*SetBackRatio;
   Direction.y = (m_FromPoint.y - m_ToPoint.y)*SetBackRatio;

   // Adjust start location by this vector
   m_ToPointWithSetback.x = m_ShapePoints[m_ShapePoints.size()-1].x + Direction.x;
   m_ToPointWithSetback.y = m_ShapePoints[m_ShapePoints.size()-1].y + Direction.y;

}
	float 	GetImpactedFlag(int DepartureTime)
	{
		for(unsigned int il = 0; il< CapacityReductionVector.size(); il++)
		{
			if(DepartureTime >= CapacityReductionVector[il].StartTime && DepartureTime<=CapacityReductionVector[il].EndTime )
			{
				return CapacityReductionVector[il].LaneClosureRatio;
			}
		}

		return 0;
	}

	float 	GetMessageSign(int DepartureTime)
	{
		for(unsigned int il = 0; il< MessageSignVector.size(); il++)
		{
			if(DepartureTime >= MessageSignVector[il].StartTime && DepartureTime<=MessageSignVector[il].EndTime) 
			{
				return MessageSignVector[il].ResponsePercentage;
			}
		}

		return 0;
	}

	float 	GetTollValue(int DepartureTime)
	{
		for(unsigned int il = 0; il< TollVector.size(); il++)
		{
			if(DepartureTime >= TollVector[il].StartTime && DepartureTime<=TollVector[il].EndTime) 
			{
				return 1;
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

	std::vector<SLinkMOE> m_LinkMOEAry;
	std::vector<SLinkMOE> m_LinkMOEAry_15min;

	std::vector<SLinkMOE> m_HistLinkMOEAry;

	int m_ReliabilityIndex;
	int m_SafetyIndex;
	int m_EmissionsIndex;
	int m_MobilityIndex;


	bool m_bSensorData;
	int  m_SensorID;

	float m_ObsHourlyLinkVolume;
	int *aryCFlowA;
	int *aryCFlowD;

	std::vector<CapacityReduction> CapacityReductionVector;
	std::vector<MessageSign> MessageSignVector;
	std::vector<DTAToll> TollVector;


	int m_bMergeFlag;  // 1: freeway and freeway merge, 2: freeway and ramp merge
	std::vector<MergeIncomingLink> MergeIncomingLinkVector;
	int m_MergeOnrampLinkID;
	int m_MergeMainlineLinkID;


	std::list<struc_vehicle_item> LoadingBuffer;  //loading buffer of each link, to prevent grid lock

	std::list<struc_vehicle_item> EntranceQueue;  //link-in queue  of each link
	std::list<struc_vehicle_item> ExitQueue;      // link-out queue of each link



	~DTALink(){
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

		int t;

		for(t=0; t<=m_SimulationHorizon; t++)
		{
			m_LinkMOEAry[t].SetupMOE(m_FreeFlowTravelTime,m_SpeedLimit);
		}
		LoadingBuffer.clear();
		EntranceQueue.clear();
		ExitQueue.clear();


	}
	int m_LinkNo;
	int m_OrgDir;
	int m_RailBidirectionalFlag;
	string m_TrackType;
	int m_Direction; 
	bool m_bOneWayLink;
	int m_LinkID;

	int m_FromNodeID;  // index starting from 0
	int m_ToNodeID;    // index starting from 0

	int green_height;  // for 3D KML
	int red_height;

	float m_Kjam;
	float m_AADT_conversion_factor;
	float m_Wave_speed_in_mph;
	string m_Mode_code;


	int m_DisplayLinkID;

	int m_FromNodeNumber;
	int m_ToNodeNumber;

	int m_NumberOfLeftTurnBay;
	int m_LeftTurnBayLengthInFeet;	
	int m_LeftTurnCapacity;

	DTA_Approach m_FromApproach;
	DTA_Approach m_ToApproach;
	int m_ReverseLinkId;

	float	m_OriginalLength;  // in miles
	float	m_Length;  // in miles
	float    m_VehicleSpaceCapacity; // in vehicles
	int		m_NumLanes;
	float	m_SpeedLimit;
	float	m_ReversedSpeedLimit;

	float	m_MaximumServiceFlowRatePHPL;  //Capacity used in BPR for each link, reduced due to link type and other factors.
	float   m_Saturation_flow_rate_in_vhc_per_hour_per_lane;

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
	bool m_bConnector;
	int m_ConnectorZoneID;

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

	int m_TotalVolume;
	int m_TotalTravelTime;
	int m_TotalDiffValue;

	float GetObsSpeed(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return max(m_StaticSpeed, m_LinkMOEAry[t].ObsSpeed);  
		else
			return m_StaticSpeed;
	}

	float GetObsSpeedCopy(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return max(m_StaticSpeed, m_LinkMOEAry[t].ObsSpeedCopy);  
		else
			return m_StaticSpeed;
	}

	float GetObsLaneVolume(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].ObsLinkFlow/m_NumLanes;
		else
		{
			if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_StaticLinkVolume/m_NumLanes;
			else
				return 0;
	
		}
	}

	float GetObsLaneVolumeCopy(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].ObsFlowCopy/m_NumLanes;
		else
		{
			if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_StaticLinkVolume/m_NumLanes;
			else
				return 0;
		}
	}

	float GetObsLinkVolume(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].ObsLinkFlow;
		else
		{
			if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_StaticLinkVolume;
			else
				return 0;
	
		}
	}

	float GetObsLinkVolumeCopy(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].ObsFlowCopy;
		else
		{
			if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_StaticLinkVolume;
			else
				return 0;
		}
	}

	float GetObsTravelTimeIndex(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].ObsTravelTimeIndex;
		else
		{
			if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_StaticTravelTime;
			else
				return 0;
		}
	}

	float GetObsTravelTimeIndexCopy(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return  m_LinkMOEAry[t].ObsTravelTimeIndexCopy;  
		else
		{
			if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_StaticTravelTime;
			else
				return 0;
		}
	}

	int GetEventCode(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].EventCode;  
		else
			return 0;
	}
	float GetObsCumulativeFlow(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].ObsCumulativeFlow;  
		else
			return m_StaticLinkVolume;
	}

	float GetObsDensity(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return max(0, m_LinkMOEAry[t].ObsDensity);  
		else
			return 0;
	}		

	float GetObsDensityCopy(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return max(0, m_LinkMOEAry[t].ObsDensityCopy);  
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
			for(int t=starting_time; t< starting_time + time_interval && (unsigned int)t < m_LinkMOEAry.size(); t++)
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
	DTAPath()
	{
		for(int t=0; t<1440; t++)
		{
			m_TimeDependentTravelTime[t] = 0;
		}

		for(int t=0; t<1440; t++)
		{
			m_WithinDayMeanTimeDependentTravelTime[t] = 0;
			m_WithinDayMaxTimeDependentTravelTime[t] = 0;
			m_WithinDayMeanTimeDependentFuelConsumption[t] = 0;
			m_WithinDayMeanTimeDependentEmissions[t] = 0;
			m_WithinDayMeanGeneralizedCost[t] = 0;
		}

		m_Distance =0;
		m_TravelTime = 0;
		m_Reliability = 0;
		m_Emissions = 0;
		m_Safety = 0;
		m_Fuel = 0;
		m_MaxTravelTime = 0;


	}

	void Init(int LinkSize, int TimeHorizon)
	{
		m_TimeHorizon = TimeHorizon;
		m_LinkSize = LinkSize;
		m_number_of_days = max(1,TimeHorizon/1440);
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
	}

	int m_LinkSize;
	int m_LinkVector[MAX_NODE_SIZE_IN_A_PATH];
	int m_NodeNodeSum;

	float m_TimeDependentTravelTime[1440];
	float m_MaxTravelTime;

	float m_WithinDayMeanTimeDependentTravelTime[1440];
	float m_WithinDayMaxTimeDependentTravelTime[1440];

	float m_WithinDayMeanTimeDependentFuelConsumption[1440];  // unit: gallon
	float m_WithinDayMeanTimeDependentEmissions[1440];  // unit: pounds

	float m_WithinDayMeanGeneralizedCost[1440];  // unit: pounds

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
unsigned short  LinkNo;  // range:
float ArrivalTimeOnDSN;     // absolute arrvial time at downstream node of a link: 0 for the departure time, including delay/stop time
//   float LinkWaitingTime;   // unit: 0.1 seconds
SVehicleLink()
{
	LinkNo = 0;
	ArrivalTimeOnDSN = 0;
	//		LinkWaitingTime = 0;

}

};


class DTAVehicleAdditionalData   // this class contains non-essential information, we do not allocate memory for this additional info in the basic version
{
public:
	float m_TollDollar;
	float m_MinCost;
	float m_MeanTravelTime;
	float m_TravelTimeVariance;
	unsigned short m_NumberOfSamples;  // when switch a new path, the number of samples starts with 0

	DTAVehicleAdditionalData()
	{
		m_NumberOfSamples =0;
		m_MinCost = 0;
	};

	void PostTripUpdate(float TripTime)   
	{
		float GainFactor = 0.2f;  // will use formula from Kalman Filtering, eventually

		m_MeanTravelTime = (1-GainFactor)*m_MeanTravelTime + GainFactor*TripTime;
		m_NumberOfSamples +=1;
	};


};

class GPSLocationRecord
{
public:
double x;
double y;

};
class DTAVehicle
{
public:

	int m_NodeSize;
	int m_NodeNumberSum;  // used for comparing two paths
	SVehicleLink *m_NodeAry; // link list arrary of a vehicle path

	unsigned int m_RandomSeed;
	int m_VehicleID;  //range: +2,147,483,647

	string m_VehicleKey;
	int m_OriginZoneID;  //range 0, 65535
	int m_DestinationZoneID;  // range 0, 65535

	short m_DemandType;     // 
	short m_PricingType;     // 
	short m_VehicleType;     //
	short m_InformationClass;  // 0: historical, 1: pre-trip, 2: en-route
	short m_Occupancy;  // 1: LOV, 2: HOV
	short m_SimLinkSequenceNo; //  range 0, 65535

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

	bool m_bMarked;

	DTAVehicleAdditionalData* pVehData;

	std::vector<GDPoint> m_GPSLocationVector;

	CVehicleEmission m_EmissionData;

	// multi-day equilibrium
	bool m_bETTFlag;
	int m_DayDependentLinkSize[MAX_DAY_SIZE];
	std::map<int, int> m_DayDependentAryLink;  // first key: day*MAX_PATH_LINK_SIZE + index, second element; return link index
	float m_DayDependentTripTime[MAX_DAY_SIZE];
	int m_DayDependentNodeNumberSum[MAX_DAY_SIZE];  // used for comparing two paths
	float m_DayDependentGap[MAX_DAY_SIZE];  // used for gap analysis
	float m_AvgDayTravelTime;
	float m_DayTravelTimeSTD;


	float m_TollDollarCost;
	float m_VOT;
	float m_Emissions;
	DTAVehicle()
	{
		m_bMarked = false;
		pVehData=NULL;
		m_TimeToRetrieveInfo = -1;
		m_TollDollarCost = 0;

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

		m_PricingType = -1;

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
	int m_VOT;


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
		AADT = 0;
		peak_hour_factor = 0;

	}
	int FromNodeNumber;
	int ToNodeNumber;
	int LinkID;
	string SensorType;

	float AADT;
	float peak_hour_factor;
	float Spd85Per;
	float PerTrucks;

	float RelativeLocationRatio;
	string SensorID;
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
extern bool g_read_a_line(FILE* f, char* aline, int & size);

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
	int* LinkNoAry;                // Node status array used in KSP;

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
		float TravelTime;
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
		LinkNoAry = new int[m_NodeSize];
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

	DTANetworkForSP()
	{
		m_OutboundSizeAry = NULL;
		m_OutboundNodeAry = NULL;
		m_SearchTreeList = NULL;
		m_InboundSizeAry = NULL;

		m_LinkList = NULL;

		m_FromIDAry = NULL;
		m_ToIDAry = NULL;

		NodeStatusAry  = NULL;
		LinkNoAry  = NULL;
		NodePredAry  = NULL;
		LabelTimeAry  = NULL;
		LabelCostAry  = NULL;


		m_NodeSize = 0;
		m_LinkSize = 0;
	};

	void Initialize(int NodeSize, int LinkSize, int TimeHorizon,int AdjLinkSize)
	{

		if(TimeHorizon <1)
		{
				AfxMessageBox("TimeHorizon <1!");
				TimeHorizon = 1;
				return;

		}
		if(m_OutboundNodeAry!=NULL)
		{
			FreeMemory();
		}

		m_NodeSize = NodeSize;
		m_LinkSize = LinkSize;

		m_OptimizationHorizon = TimeHorizon;
		m_OptimizationTimeInveral = 1;
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
		LinkNoAry = new int[m_NodeSize];                    // Node status array used in KSP;
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


	void FreeMemory()
	{
		if(m_SearchTreeList) delete m_SearchTreeList;

		if(m_OutboundSizeAry)  delete m_OutboundSizeAry;
		if(m_InboundSizeAry)  delete m_InboundSizeAry;

		if(m_NodeSize > 0)
		{
		DeallocateDynamicArray<int>(m_OutboundNodeAry,m_NodeSize, m_AdjLinkSize);
		DeallocateDynamicArray<int>(m_OutboundLinkAry,m_NodeSize, m_AdjLinkSize);
		DeallocateDynamicArray<int>(m_InboundLinkAry,m_NodeSize, m_AdjLinkSize);
		DeallocateDynamicArray<float>(TD_LabelCostAry,m_NodeSize,m_OptimizationIntervalSize);
		DeallocateDynamicArray<int>(TD_NodePredAry,m_NodeSize,m_OptimizationIntervalSize);
		DeallocateDynamicArray<int>(TD_TimePredAry,m_NodeSize,m_OptimizationIntervalSize);
		}


		if(m_LinkList) delete m_LinkList;

		if(m_LinkSize > 0)
		{
		DeallocateDynamicArray<float>(m_LinkTDTimeAry,m_LinkSize,m_OptimizationIntervalSize);
		DeallocateDynamicArray<float>(m_LinkTDCostAry,m_LinkSize,m_OptimizationIntervalSize);
		}

		if(m_FromIDAry)		delete m_FromIDAry;
		if(m_ToIDAry)	delete m_ToIDAry;

		if(NodeStatusAry) delete NodeStatusAry;                 // Node status array used in KSP;
		if(LinkNoAry) delete LinkNoAry;                 // Node status array used in KSP;
		if(NodePredAry) delete NodePredAry;
		if(LabelTimeAry) delete LabelTimeAry;
		if(LabelCostAry) delete LabelCostAry;
	}
	~DTANetworkForSP()
	{

		FreeMemory();
	};

	float GetTollRateInMin(int LinkID, float Time, int VehicleType);  // built-in function for each network_SP to avoid conflicts with OpenMP parallel computing


	void BuildNetwork(int ZoneID);
	void BuildHistoricalInfoNetwork(int CurZoneID, int CurrentTime, float Perception_error_ratio);
	void BuildTravelerInfoNetwork(int CurrentTime, float Perception_error_ratio);

	void BuildPhysicalNetwork(std::list<DTANode*>* p_NodeSet, std::list<DTALink*>* p_LinkSet, float RandomCostCoef, bool bOverlappingCost);
	void BuildSpaceTimeNetworkForTimetabling(std::list<DTANode*>* p_NodeSet, std::list<DTALink*>* p_LinkSet, int TrainType);

	void IdentifyBottlenecks(int StochasticCapacityFlag);

	int SimplifiedTDLabelCorrecting_DoubleQueue(int origin, int departure_time, int destination, int pricing_type, float VOT,int PathLinkList[MAX_NODE_SIZE_IN_A_PATH],float &TotalCost, bool distance_flag, bool check_connectivity_flag, bool debug_flag, float RandomCostCoef);   // Pointer to previous node (node)

	// simplifed version use a single node-dimension of LabelCostAry, NodePredAry

	//these two functions are for timetabling
	bool OptimalTDLabelCorrecting_DoubleQueue(int origin, int departure_time);
	// optimal version use a time-node-dimension of TD_LabelCostAry, TD_NodePredAry
	int FindOptimalSolution(int origin,  int departure_time,  int destination, DTA_Train* pTrain);
	bool GenerateSearchTree(int origin,  int destination, int node_size,float TravelTimeBound);

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


class CEmissionRate 
{
public:
	float meanBaseRate_TotalEnergy;
	float meanBaseRate_CO2;
	float meanBaseRate_NOX;
	float meanBaseRate_CO;
	float meanBaseRate_HC;

	CEmissionRate()
	{
		meanBaseRate_TotalEnergy = 0;
		meanBaseRate_CO2 = 0;
		meanBaseRate_NOX = 0;
		meanBaseRate_CO = 0;
		meanBaseRate_HC = 0;

	}

};

class CAVISensorPair
{

public:
	CAVISensorPair()
	{
		pLink = NULL;
		number_of_samples = 0;
	}
	int sensor_pair_id;
	int number_of_samples;
	CString sensor_type;
	int from_node_id;
	int to_node_id;
	DTALink* pLink;
};


class VehicleStatistics
{
public: 
	VehicleStatistics()
	{
		TotalVehicleSize = 0;
		TotalTravelTime = 0;
		TotalDistance = 0;
		TotalCost = 0;
		TotalEmissions = 0;
		TotalGeneralizedCost = 0;
		TotalGeneralizedTime = 0;
		DisplayValue = 0;
	}

	void Reset()
	{
		TotalVehicleSize = 0;
		TotalTravelTime = 0;
		TotalVariance = 0;
		TotalDistance = 0;
		TotalCost = 0;
		TotalEmissions = 0;
		TotalGeneralizedCost = 0;
		TotalGeneralizedTime = 0;
		bImpactFlag = false;
	}

	CString Label;
	float DisplayValue;
	bool bImpactFlag;
	int   TotalVehicleSize;
	float TotalTravelTime;
	float TotalVariance;
	float TotalDistance;
	float TotalGeneralizedCost;
	float TotalGeneralizedTime;
	float TotalCost;
	float TotalEmissions;
	CVehicleEmission emissiondata;

};

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

#pragma warning(disable:4244)  // stop warning: "conversion from 'int' to 'float', possible loss of data"
// Stop bugging me about this, live isn't perfect


extern float g_RNNOF();

extern std::vector<DTAPath>	m_PathDisplayList;
extern int m_SelectPathNo;
extern float g_Simulation_Time_Stamp;

extern int  g_SimulationStartTime_in_min;
