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
#pragma warning(disable: 4244)  // stop warning: "conversion from 'int' to 'float', possible loss of data"
#pragma warning(disable: 4996)  // Consider using sscanf_s instead
#pragma warning(disable: 4101)  // unreferenced local variable


#include "resource.h"
#include "Utility.h"

#include <math.h>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <list>




enum DTA_Direction
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

extern int 	g_MOEAggregationIntervalInMin;
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

	void Reset()
	{
		Energy = 0;
		CO2 = 0;
		NOX = 0;
		CO = 0;
		HC = 0;
	
	
	}
};

extern float g_GetPoint2Point_Distance(GDPoint p1, GDPoint p2);

extern DTA_Turn g_RelativeAngle_to_Turn(int RelativeAngle);

extern float g_GetPoint2LineDistance(GDPoint pt, GDPoint FromPt, GDPoint ToPt);
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
  GDPoint pt;
};

class DTATimeDependentemand
{
public: 

	int starting_time_in_min;
	int ending_time_in_min;
	float time_dependent_value;
	int type;
	

	DTATimeDependentemand()
	{
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
	
		for(unsigned int i = 0; i< m_ActivityLocationVector.size(); i++)
		{
			m_Center.x += m_ActivityLocationVector[i].pt.x;
			m_Center.y += m_ActivityLocationVector[i].pt.y;

		}

		m_Center.x /= max(1,m_ActivityLocationVector.size());
		m_Center.y /= max(1,m_ActivityLocationVector.size());

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
		m_bWithinSubarea = true;

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

	void SetDefaultVehicleTypeDistribution()
	{
		if(demand_type==1 || demand_type==2) //SOV or HOV
		{
		vehicle_type_percentage[1] = 80;
		vehicle_type_percentage[2] = 20;
		}
	
		if(demand_type== 3) //truck
		{
		vehicle_type_percentage[3] = 30;
		vehicle_type_percentage[4] = 30;
		vehicle_type_percentage[5] = 40;
		}
	
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
	pair_key = -1;
	starting_time_in_min = 0;
	ending_time_in_min = 1440;
	turning_percentage = 0;
	turning_prohibition_flag = 0;
	turning_protected_flag = 0;
	turning_permitted_flag = 0;

	signal_control_no = 0;
	signal_group_no = 0;
	phase_index = 0;
	
	movement_hourly_capacity = 10000;

	for(int h = 0; h<24; h++)
		HourlyCount[h] = 0;

sim_turn_count = 0;
sim_turn_hourly_count = 0;
sim_turn_percentage = 0;
sim_turn_delay = 0; 

obs_turn_count = 0;
obs_turn_hourly_count = 0;
obs_turn_percentage = 0; 
obs_turn_delay = 0; 

   QEM_TurnVolume = 0;
   QEM_LinkVolume = 0;
   QEM_Lanes = 1;
   QEM_Shared = 0;
   QEM_Width = 12; 
   QEM_Storage = 0;
   QEM_StLanes = 0;
   QEM_Grade = 0;
   QEM_Speed = 40;
   QEM_IdealFlow = 1900;
   QEM_LostTime = 4;
   QEM_Phase1 = 0;
   QEM_PermPhase1 = 0;
   QEM_DetectPhase1 = 0;

   QEM_TurnPercentage = 0;
   QEM_EffectiveGreen = 0;
   QEM_Capacity = 0;
   QEM_VOC = 0;
   QEM_SatFlow = 0;
   QEM_Delay = 0;
   QEM_reference_node_number = 0;

	}

int pair_key;
int HourlyCount[24];


int IncomingLinkID;
int OutgoingLinkID;
DTA_Turn movement_turn;
DTA_Direction movement_approach;
DTA_APPROACH_TURN movement_dir;
string QEM_dir_string;

int QEM_reference_node_number;

int in_link_from_node_id;
int in_link_to_node_id;  // this equals to the current node number
int out_link_to_node_id;

int movement_hourly_capacity;
int starting_time_in_min;
int ending_time_in_min;
float turning_percentage;
int turning_prohibition_flag;
int turning_protected_flag;
int turning_permitted_flag;


float sim_turn_count; 
float sim_turn_hourly_count;
float sim_turn_percentage; 
float sim_turn_delay; 

float obs_turn_count; 
float obs_turn_hourly_count;
float obs_turn_percentage; 
float obs_turn_delay; 

   int QEM_TurnVolume;
   int QEM_LinkVolume;
   float QEM_TurnPercentage;

   int QEM_Lanes;
   int QEM_Shared;
   int QEM_Width;
   int QEM_Storage;
   int QEM_StLanes;
   float QEM_Grade;
   float QEM_Speed;
   float QEM_IdealFlow;
   float QEM_LostTime;
   int QEM_Phase1;
   int QEM_PermPhase1;
   int QEM_DetectPhase1;

   float QEM_EffectiveGreen;
   float QEM_Capacity;
   float QEM_VOC;
   float QEM_SatFlow;
   float QEM_Delay;
   CString QEM_LOS;

int phase_index;
int signal_control_no;  // for meso-scopic, link -based
int signal_group_no;  // for meso-scopic, link -based

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
      std::vector<int> movement_type_vector;  // prohibited, permitted, protected, free


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
		m_bNodeAvoidance  = false;
		m_QEM_ReferenceNodeNumber = 0;
		m_IntermediateDestinationNo = 0;
		m_NodeProduction =0;
		m_NodeAttraction =0;

		m_bZoneActivityLocationFlag = false;
		m_NodeNumber = 0;
		m_NodeOriginalNumber = 0;
		m_ControlType = 0;
		m_ZoneID = 0;
		m_TotalCapacity = 0;
		m_Connections = 0;
		m_LayerNo = 0;
		m_DistanceToRoot = 0;
		m_CycleLengthInSecond = 60;
		m_SignalOffsetInSecond = 0;
		m_NumberofPhases = 0;
		m_bSignalData = false;
		m_External_OD_flag = 0;
		m_tobeRemoved = false;
		m_bSubareaFlag = 0;  // when the associated link is removed as it is outside the boundary, then we mark its from and t nodes as subarea boundary node 
		m_CentroidUpdateFlag = 0;

		for(int si = 0; si <10; si++)
			m_SignalPhaseNo[si] = 0;

		m_SignalCycleLength = 0;

		m_bQEM_optimized = false;

	};


	~DTANode(){};

	bool m_bNodeAvoidance;
	float m_NodeProduction;
	float m_NodeAttraction;

	int m_CentroidUpdateFlag;  // used by node splitting for subarea
	int m_bSubareaFlag;
	bool m_tobeRemoved;
	bool m_bZoneActivityLocationFlag; 

	int m_External_OD_flag;

	std::vector<int> m_OutgoingLinkVector;
	std::vector<int> m_IncomingLinkVector;
	std::vector <DTANodeMovement> m_MovementVector;

	int FindMovementIndexFromDirecion(DTA_APPROACH_TURN movement_direction)
	{
	
		for(unsigned int i  = 0; i < m_MovementVector.size(); i++)
		{
			if(m_MovementVector[i].movement_dir == movement_direction)
				return i;

		}

		return -1;
	}

	int FindHourlyCountFromDirection(DTA_Direction movement_approach)
	{
	
		int link_count = 0;
		// sum up all movement along the same approach
		for(unsigned int i  = 0; i < m_MovementVector.size(); i++)
		{
			if(m_MovementVector[i].movement_approach  == movement_approach)
				link_count+= m_MovementVector[i].QEM_TurnVolume;

		}

		return link_count;
	}

	int get_link_pair_key(int in_link_from_node_id, int out_link_to_node_id)
	{
	int id_1 = (in_link_from_node_id+1)%10000;  // make it like a short integer
	int id_2 = (out_link_to_node_id+1)%10000;

	int pair_key  = id_1 *10000+ id_2;

	return pair_key;
	}


	void make_Link_Pair_to_Movement_Map()
	{

		for(unsigned int i  = 0; i < m_MovementVector.size(); i++)
		{
			int pair_key = get_link_pair_key( m_MovementVector[i].in_link_from_node_id,m_MovementVector[i].out_link_to_node_id);

			m_Link_Pair_to_Movement_Map[pair_key] = i;
			m_MovementVector[i].pair_key = pair_key;

		}

	}

	void ResetMovementMOE()
	{

		for(unsigned int i  = 0; i < m_MovementVector.size(); i++)
		{

			m_MovementVector[i].sim_turn_count  = 0;
			m_MovementVector[i].turning_percentage   = 0;
			m_MovementVector[i].sim_turn_delay = 0;
		}

	}

	std::map<int, int > m_Link_Pair_to_Movement_Map;

	void AddMovementCount(int Hour,int in_link_from_node_id, int out_link_to_node_id)
	{
	
		if(m_Link_Pair_to_Movement_Map.size()==0)
			make_Link_Pair_to_Movement_Map();

		int link_pair_key = get_link_pair_key( in_link_from_node_id,out_link_to_node_id);
		int movement_index = m_Link_Pair_to_Movement_Map[link_pair_key];

		ASSERT(link_pair_key == m_MovementVector[movement_index].pair_key );
		
		if(Hour<24)
		{
		m_MovementVector[movement_index].HourlyCount [Hour]++;
		}

		m_MovementVector[movement_index].sim_turn_count++;

	}


	int GetMovementIndex(int in_link_from_node_id, int in_link_to_node_id, int out_link_to_node_id)
	{
		if(m_Link_Pair_to_Movement_Map.size()==0)
			make_Link_Pair_to_Movement_Map();

		return get_link_pair_key(in_link_from_node_id, out_link_to_node_id);

		//for(unsigned int i  = 0; i < m_MovementVector.size(); i++)
		//{

		//if( m_MovementVector[i].in_link_from_node_id== in_link_from_node_id
		//&& m_MovementVector[i].in_link_to_node_id== in_link_to_node_id
		//&& m_MovementVector[i].out_link_to_node_id== out_link_to_node_id)
		//return i;
		//}

		return -1;  //not found

	}

	std::vector <DTANodeLaneTurn> m_LaneTurnVector;

	std::vector <DTANodePhase> m_PhaseVector;
	 
	int m_CycleLengthInSecond;
	int m_SignalOffsetInSecond;
	int m_NumberofPhases;
	float m_DistanceToRoot;
	string m_Name;
	GDPoint pt;
	GDPoint schedule_pt;

	int m_IntermediateDestinationNo;  ///id, starting from zero, continuous sequence

	bool m_bSignalData;
	int m_LayerNo;
	int m_NodeNumber;  //  original node number
	int m_QEM_ReferenceNodeNumber;  //  original node number
	int m_NodeOriginalNumber;  //  original node number
	int m_NodeID;  ///id, starting from zero, continuous sequence
	int m_ZoneID;  // If ZoneID > 0 --> centroid,  otherwise a physical node.
	int m_ControlType; // Type: ....
	float m_TotalCapacity;
	int m_Connections;  // number of connections

	//DTA_NodeMovementSet m_Movementt;

	// signal data

	int m_SignalPhaseNo[10];//optimized by QEM
	int m_SignalCycleLength; //optimized by QEM
	float m_PhaseDataMatrix[23][8]; //optimized by QEM
	bool m_bQEM_optimized;
	
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

	DTALine ()
	{
	m_FromNodeNumber = 0;
	m_ToNodeNumber = 0;
	
	}
	int LineID;
	double Miles;

	int m_FromNodeNumber;
	int m_ToNodeNumber;

	std::string TMC_code;
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


	float SimulationQueueLength;
	float SimulatedTravelTime;

	float SimulationSpeed;  // speed
	float SimulationLinkFlow;   // flow volume
	float SimulationDensity;   // SimulationDensity

	// these three copies are used to compare simulation results and observed results
	float SensorSpeed;  // speed
	float SensorLinkCount;   // flow volume
	float SensorDensity;   // SimulationDensity
	float SensorArrivalCumulativeFlow;   // flow volume

	float SimulatedTravelTimeCopy;
	float SimuArrivalCumulativeFlow;   // flow volume
	float SimuDepartureCumulativeFlow;   // flow volume


	float Energy;
	float CO2;
	float NOX;
	float CO;
	float HC;

	//   Density can be derived from CumulativeArrivalCount and CumulativeDepartureCount
	//   Flow can be derived from CumulativeDepartureCount
	//   AvgTravel time can be derived from CumulativeArrivalCount and TotalTravelTime

	SLinkMOE()
	{
		//EventCode = 0;
		//EpisoDuration = 0;
		//EpisodeNo = 0;
		SimulationQueueLength = 0;
		SimulatedTravelTime = 0;
		SimulationSpeed = 0;
		SimulationLinkFlow = 0;
		SimuArrivalCumulativeFlow = 0;
		SimuDepartureCumulativeFlow = 0;
		SimulationDensity = 0;

				
		// these four copies are used to compare simulation results and observed results
		SensorSpeed = 0;
		SensorLinkCount = 0;
		SensorDensity = 0;
		SimulatedTravelTimeCopy = 0;

		Energy = 0;
		CO2  = 0;
		NOX  = 0;
		CO  = 0;
		HC  = 0;

	};

	void SetupMOE(float FreeFlowTravelTime, float SpeedLimit)
	{
		SimulationQueueLength = 0;
		SimulatedTravelTime = FreeFlowTravelTime;
		SimulatedTravelTimeCopy = FreeFlowTravelTime;

		SimulationSpeed = SpeedLimit;
		SimulationLinkFlow = 0;
		SimuArrivalCumulativeFlow = 0;
		SimulationDensity = 0;

		Energy = 0;
		CO2  = 0;
		NOX  = 0;
		CO  = 0;
		HC  = 0;

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
		ScenarioNo = 0;

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
	int ScenarioNo;
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

	int ScenarioNo;
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
		ScenarioNo = 0;
		StartDayNo= 0;
		EndDayNo = 100;

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
		ScenarioNo = 0;
		StartDayNo = 0;
		EndDayNo = 100;

		for(int p = 0; p  < MAX_PRICING_TYPE_SIZE; p++)
		{
		TollRate[p] = 0;
		TollRateInMin [p] = 0;
		}
	}

	int ScenarioNo;
	float StartTime;
	float EndTime;
	int StartDayNo;
	int EndDayNo;
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

#include "Transit.h"
class DTALink
{
public:

	DTALink(int TimeHorizon)  // TimeHorizon's unit: per min
	{
		m_AdditionalCost = 0;
		m_EffectiveGreenTimeInSecond = 0;
		m_GreenStartTimetInSecond =0;
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
		m_SpeedLimit  = 50;
		m_ReversedSpeedLimit  = 50;
		m_Saturation_flow_rate_in_vhc_per_hour_per_lane = 1800;

		m_total_link_volume = 0;
		m_TotalVolumeForMovementCount = 0;
		m_MeanSpeed  = m_SpeedLimit;
		m_TotalTravelTime = 0;
		m_TotalDiffValue = 0;
		m_NumberOfMarkedVehicles = 0;
		m_AVISensorFlag = false;
		m_LinkID = 0;
		green_height = 10;
		red_height = 100;
		blue_height = 300;
		yellow_height = 1000;

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
		m_number_of_all_crashes = 0;
		m_num_of_fatal_and_injury_crashes_per_year =0;
		m_num_of_PDO_crashes_per_year = 0;

		m_number_of_intersection_crashes = 0;
		m_num_of_intersection_fatal_and_injury_crashes_per_year =0;
		m_num_of_intersection_PDO_crashes_per_year = 0;

		// safety prediction
		m_Num_Driveways_Per_Mile = 0;
		m_volume_proportion_on_minor_leg = 0;
		m_Num_3SG_Intersections = 0; 
		m_Num_3ST_Intersections = 0;
		m_Num_4SG_Intersections = 0;
		m_Num_4ST_Intersections = 0;

		m_LevelOfService = 'A';
		m_avg_waiting_time_on_loading_buffer = 0;

		m_avg_simulated_speed = 0;
		m_total_sensor_link_volume = 0;
		m_total_link_volume = 0;
		m_total_link_count_error = 0;
		m_simulated_AADT = 0;
		m_volume_over_capacity_ratio  = 0;
		m_volume_over_capacity_ratio = 0;
		m_StaticTravelTime = 0;
		m_Grade = 0;

		input_line_no = 0;

		m_bIncludedBySelectedPath = false;
		m_bIncludedinSubarea = false;

		m_AADT = 0;
		m_ReferenceFlowVolume  = 0;
		m_PeakHourFactor = 0.15f;
		m_bToBeShifted = true;

	 m_Num_Driveways_Per_Mile = 0;
	 m_volume_proportion_on_minor_leg = 0;
	 m_Num_3SG_Intersections = 0; 
	 m_Num_3ST_Intersections = 0; 
	 m_Num_4SG_Intersections = 0;
	 m_Num_4ST_Intersections = 0;

		m_Intersection_NumberOfCrashes = 0;
		m_Intersection_NumberOfFatalAndInjuryCrashes =0;
		m_Intersection_NumberOfPDOCrashes = 0;

	};

	std::string m_TMC_code;

	float m_StaticTravelTime;
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


	bool m_AVISensorFlag;
	int m_LayerNo;
	float m_Grade;
	string m_Name;

	// safety prediction
		double m_Intersection_NumberOfCrashes, m_Intersection_NumberOfFatalAndInjuryCrashes,m_Intersection_NumberOfPDOCrashes;

	// safety prediction
	double m_Num_Driveways_Per_Mile;
	double m_volume_proportion_on_minor_leg;
	double m_Num_3SG_Intersections; 
	double m_Num_3ST_Intersections; 
	double m_Num_4SG_Intersections;
	double m_Num_4ST_Intersections;

	// overall information

	float m_total_link_volume;
	float m_volume_over_capacity_ratio;
	char m_LevelOfService;
	float m_avg_waiting_time_on_loading_buffer;
	float m_avg_simulated_speed;
	float m_total_sensor_link_volume;
	float m_total_link_count_error;
	float m_simulated_AADT;
	double m_number_of_all_crashes;
	double m_num_of_fatal_and_injury_crashes_per_year;
	double m_num_of_PDO_crashes_per_year;

	double m_number_of_intersection_crashes;
	double m_num_of_intersection_fatal_and_injury_crashes_per_year;
	double m_num_of_intersection_PDO_crashes_per_year;
	// end of overall information

	int input_line_no;

	std::vector<GDPoint> m_ShapePoints;
	std::vector<GDPoint> m_Original_ShapePoints;  // original shape points from GIS (without offset)


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
			total_distance += g_GetPoint2Point_Distance(m_ShapePoints[si],m_ShapePoints[si+1]); 
		}

		if(total_distance < 0.0000001f)
			total_distance = 0.0000001f;

		float distance_ratio = 0;
		float P2Origin_distance = 0;
		m_ShapePointRatios.push_back(0.0f);
		for(si = 0; si < m_ShapePoints .size()-1; si++)
		{
			P2Origin_distance += g_GetPoint2Point_Distance(m_ShapePoints[si],m_ShapePoints[si+1]);
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
			return m_Length/m_HistLinkMOEAry[time].SimulationSpeed*60;  // *60: hour to min
		}
		else
			return m_FreeFlowTravelTime;
	}

	float ObtainHistFuelConsumption(unsigned int time)
	{
		if(time<m_HistLinkMOEAry.size() && m_bSensorData == true)
		{
			return m_Length*0.1268f*pow( max(1,m_HistLinkMOEAry[time].SimulationSpeed),-0.459f);  // Length*fuel per mile(speed), y= 0.1268x-0.459
		}
		else
			return m_Length*0.1268f*pow(m_SpeedLimit,-0.459f);  // Length*fuel per mile(speed_limit), y= 0.1268x-0.459
	}

	float ObtainHistCO2Emissions(unsigned int time)  // pounds
	{
		if(time<m_HistLinkMOEAry.size() && m_bSensorData == true)
		{
			return min(1.4f,m_Length*11.58f*pow(m_HistLinkMOEAry[time].SimulationSpeed,-0.818f));  // Length*fuel per mile(speed), y= 11.58x-0.818
		}
		else
			return m_Length*11.58f*pow(m_SpeedLimit,-0.818f);  // Length*fuel per mile(speed_limit), y= 11.58x-0.818
	}

	void ResetMOEAry(int TimeHorizon)
	{
		m_SimulationHorizon	= TimeHorizon;

		m_total_link_volume = 0;
		m_TotalVolumeForMovementCount= 0;
		m_MeanSpeed  = m_SpeedLimit;
		m_TotalTravelTime = 0;
		m_TotalDiffValue = 0;
		m_NumberOfMarkedVehicles = 0;

		int OldSize = (int)(m_LinkMOEAry.size());
		m_LinkMOEAry.resize (m_SimulationHorizon+1);

		int t;
		for(t=0; t<= TimeHorizon; t++)
		{
			m_LinkMOEAry[t].SetupMOE(m_FreeFlowTravelTime,m_SpeedLimit);
		}	
	};

	void ComputeHistoricalAvg(int number_of_weekdays);

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

	std::vector<SLinkMOE> m_HistLinkMOEAry;

	int m_ReliabilityIndex;
	int m_SafetyIndex;
	int m_EmissionsIndex;
	int m_MobilityIndex;


	bool m_bSensorData;

	string m_SensorTypeString;

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

	std::string m_geo_string;
	int green_height;  // for 3D KML
	int red_height;
	int blue_height;
	int yellow_height;


	float m_Kjam;

	// for crash prediction
	std::string group_1_code,group_2_code, group_3_code;

	float m_AADT_conversion_factor;

	float m_Wave_speed_in_mph;
	int m_EffectiveGreenTimeInSecond;
	int m_GreenStartTimetInSecond;
	string m_Mode_code;


	int m_DisplayLinkID;

	int m_FromNodeNumber;
	int m_ToNodeNumber;

	int m_NumberOfLeftTurnBay;
	int m_LeftTurnBayLengthInFeet;	
	int m_LeftTurnCapacity;

	DTA_Direction m_FromApproach;
	DTA_Direction m_ToApproach;
	int m_ReverseLinkId;

	float	m_OriginalLength;  // in miles
	float	m_Length;  // in miles



	PT_StopTime m_StopTimeRecord;
	float   m_AdditionalCost;
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

	int m_TotalVolumeForMovementCount;
	int m_TotalTravelTime;
	int m_TotalDiffValue;

	float GetSimulationSpeed(int current_time)
	{
		float total_value = 0;
		int total_count = 0;
		for(int t = current_time; t< current_time+g_MOEAggregationIntervalInMin; t++)
		{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
		{
			if(m_LinkMOEAry[t].SimulationLinkFlow >=1) // with flow
			{
				total_count++;
				total_value+= m_LinkMOEAry[t].SimulationSpeed;
			}
		}
		}

			if(total_count>=1)
				return total_value/total_count;
			else
			return m_avg_simulated_speed;
	}

	float GetSensorSpeed(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
		{
			if(m_LinkMOEAry[t].SensorLinkCount >=1) // with flow
				return m_LinkMOEAry[t].SensorSpeed;
		}
			return m_avg_simulated_speed;
	}

	float GetObsLaneVolume(int current_time)
	{

		float total_value = 0;
		int total_count = 0;
		for(int t = current_time; t< current_time+g_MOEAggregationIntervalInMin; t++)
		{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
		{
			if(m_LinkMOEAry[t].SimulationLinkFlow >=1) // with flow
			{
				total_count++;
				total_value+= m_LinkMOEAry[t].SimulationLinkFlow/m_NumLanes;
			}
		}
		}

			if(total_count>=1)
				return total_value/total_count;
			else
			return m_total_link_volume/m_NumLanes;


	}

		float GetSensorLinkHourlyVolume(int start_time, int end_time)
	{

		if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return 0;

		float total_volume = 0;
		for(int t= start_time ; t< end_time; t++)
		{
		
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			total_volume += m_LinkMOEAry[t].SensorLinkCount;
		}
		return total_volume*60/max(1,end_time - start_time);
	}


		float GetAvgLinkHourlyVolume(int start_time, int end_time)
	{

		if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_total_link_volume;

		float total_volume = 0;
		for(int t= start_time ; t< end_time; t++)
		{
		
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			total_volume += m_LinkMOEAry[t].SimulationLinkFlow;
		}
		return total_volume/max(1, end_time-start_time);
	}


		float GetAvgLinkSpeed(int start_time, int end_time)
	{

		if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_avg_simulated_speed;

		float total_Speed = 0;
		for(int t= start_time ; t< end_time; t++)
		{
		
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			total_Speed += m_LinkMOEAry[t].SimulationSpeed;
		}
		return total_Speed/max(1, end_time-start_time);
	}

		float GetAvgLinkTravelTime(int start_time, int end_time)
	{

		//to do
		return 0;
	}
	float GetSensorLaneVolume(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].SensorLinkCount/m_NumLanes;
		else
		{
			if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_total_link_volume/m_NumLanes;
			else
				return 0;
		}
	}

	float GetSensorLaneHourlyVolume(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].SensorLinkCount/m_NumLanes*60;
		else
		{
			if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_total_link_volume/m_NumLanes;
			else
				return 0;
		}
	}

	float GetSimulatedLinkOutVolume(int current_time)
	{

		float total_count = 0;
		
		if(current_time>=1 &&	current_time < m_SimulationHorizon && current_time < m_LinkMOEAry.size())
		{

		

		total_count = m_LinkMOEAry[current_time].SimuDepartureCumulativeFlow - 
			m_LinkMOEAry[current_time-1].SimuDepartureCumulativeFlow;

		if(total_count<0)
			total_count = 0;
		
		}

		return total_count*60.0f;  // from min volume to hourly volume

	}

	float GetSimulatedLinkVolume(int current_time)
	{
		float total_value = 0;
		int total_count = 0;
		for(int t = current_time; t< current_time+g_MOEAggregationIntervalInMin; t++)
		{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
		{
			if(m_LinkMOEAry[t].SimulationLinkFlow >=1) // with flow
			{
				total_count++;
				total_value+= m_LinkMOEAry[t].SimulationLinkFlow;
			}
		}
		}

			if(total_count>=1)
				return total_value/total_count;
			else
			return m_total_link_volume;
	}

	float GetSensorLinkHourlyVolume(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].SensorLinkCount*60;
		else
		{
			if(m_LinkMOEAry.size() == 0) // no time-dependent data 
				return m_total_link_volume;
			else
				return 0;
		}
	}

	float GetSimulatedTravelTime(int current_time)
	{

		float total_value = 0;
		int total_count = 0;
		for(int t = current_time; t< current_time+g_MOEAggregationIntervalInMin; t++)
		{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
		{
			if(m_LinkMOEAry[t].SimulationLinkFlow >=1) // with flow
			{
				total_count++;
				total_value+= m_LinkMOEAry[t].SimulatedTravelTime;
			}
		}
		}

			if(total_count>=1)
				return total_value/total_count;
			else
			return m_StaticTravelTime;

	}

	float GetSimulatedTravelTimeCopy(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return  m_LinkMOEAry[t].SimulatedTravelTimeCopy;  
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
		//if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
		//	return m_LinkMOEAry[t].EventCode;  
		//else
			return 0;
	}
	float GetSimuArrivalCumulativeFlow(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].SimuArrivalCumulativeFlow;  
		else
			return 0;
	}
	float GetSimuDepartureCumulativeFlow(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return m_LinkMOEAry[t].SimuDepartureCumulativeFlow ;  
		else
			return 0;
	}
	void SetDefaultCumulativeFlow()
	{
		float SimuArrivalCumulativeFlow = 0;

		for(int t =0; t< m_LinkMOEAry.size(); t++)
		{
			if(t>=1)
			{
				m_LinkMOEAry[t].SimuArrivalCumulativeFlow = max(SimuArrivalCumulativeFlow, m_LinkMOEAry[t].SimuArrivalCumulativeFlow); // in case there are empty flow volumes

			}

			SimuArrivalCumulativeFlow = m_LinkMOEAry[t].SimuArrivalCumulativeFlow;  

		}

	}

	float GetSimulationDensity(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return max(0, m_LinkMOEAry[t].SimulationDensity);  
		else
			return 0;
	}		

	float GetSimulationQueueLength(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return max(0, m_LinkMOEAry[t].SimulationQueueLength*100);  
		else
			return 0;
	}		


	float GetSensorDensity(int t)
	{
		if(t < m_SimulationHorizon && (unsigned int)t < m_LinkMOEAry.size())
			return max(0, m_LinkMOEAry[t].SensorDensity);  
		else
			return 0;
	}		

	float GetSpeed(int time)
	{
		return m_Length/GetTravelTime(time,1)*60.0f;  // 60.0f converts min to hour, unit of speed: mph
	}


	float GetTravelTime(int starting_time, int time_interval = 1)
	{

		//if(this->m_bSensorData == false)
		//	return m_FreeFlowTravelTime;

		float travel_time  = max(m_StaticTravelTime,m_FreeFlowTravelTime);

		if(starting_time + time_interval< m_SimulationHorizon)
		{
			float total_travel_time = 0;
			for(int t=starting_time; t< starting_time + time_interval && (unsigned int)t < m_LinkMOEAry.size(); t++)
			{
				total_travel_time +=  ( m_Length * 60/ max(1,m_LinkMOEAry[t].SimulationSpeed));

				
			}

			travel_time =  total_travel_time/time_interval;

			//if(travel_time < m_FreeFlowTravelTime)
			//	travel_time = m_FreeFlowTravelTime; // minimum travel time constraint for shortest path calculation

		}


		return max(0.0001f,travel_time);;

	};


};


class DTAPath
{
public:
	DTAPath()
	{
		m_bWithSensorTravelTime = false;
		total_free_flow_travel_time = 0;
		total_distance = 0;
		
		for(int t=0; t<1440; t++)
		{
			m_TimeDependentTravelTime[t] = 0;
			m_SensorTimeDependentTravelTime[t] = 0;
			m_TimeDependentEnergy[t]= 0;
			m_TimeDependentCO2[t]= 0;
			m_TimeDependentCO[t]= 0;
			m_TimeDependentHC[t]=0;
			m_TimeDependentNOX[t] = 0;
		}

		for(int t=0; t<1440; t++)
		{
			m_WithinDayMeanTimeDependentTravelTime[t] = 0;
			m_WithinDayMaxTimeDependentTravelTime[t] = 0;
			m_WithinDayMeanTimeDependentFuelConsumption[t] = 0;
			m_WithinDayMeanTimeDependentEmissions[t] = 0;
			m_WithinDayMeanGeneralizedCost[t] = 0;
			m_TimeDependentCount [t] = 0;
		}

		m_Distance =0;
		m_TravelTime = 0;
		m_Reliability = 0;
		m_Emissions = 0;
		m_Safety = 0;
		m_Fuel = 0;
		m_MaxTravelTime = 0;
		m_NumberOfSensorsPassed= 0;
		m_NumberOfPassiveSensorsPassed = 0;
		m_NumberOfActiveSensorsPassed = 0;



	}

	void Init(int LinkSize, int TimeHorizon)
	{
		m_number_of_days = max(1,TimeHorizon/1440);
	}
	void UpdateWithinDayStatistics();

	float GetTimeDependentMOE(int time, int MOEType, int MOEAggregationIntervalInMin = 1)
	{
		float total_value = 0;
		int total_count = 0;

		int agg_interval = MOEAggregationIntervalInMin;

		if(MOEType == 1)  //sensor data
			agg_interval = 1;

		for(int t = time; t< min(1440,time+agg_interval); t++)
		{
			float value = GetTimeDependentMOEBy1Min(t,MOEType);
			if(value>0.00001f) // with value
			{
				total_count++;
				total_value+= value;
			}
		}

		return total_value/max(1,total_count);
	}

	float GetErrorStatistics(int StartTime, int EndTime, int MOEAggregationIntervalInMin = 1)
	{

		int count = 0;
		float total_error = 0;

		for(int t = StartTime; t< min(1440,EndTime); t++)
		{
			if(m_SensorTimeDependentTravelTime[t]>0.01f && m_TimeDependentTravelTime[t]>0.01f)   // with data
			{
			float sensor_value = GetTimeDependentMOEBy1Min(t,1);

			float simulation_value = GetTimeDependentMOE(t,0,MOEAggregationIntervalInMin);
			total_error += fabs(sensor_value- simulation_value);

			count++;
			}
		}

	
		return total_error/max(1,count);
	}

	float GetRelativeErrorStatistics(int StartTime, int EndTime, int MOEAggregationIntervalInMin = 1)
	{

		int count = 0;
		float total_error = 0;

		for(int t = StartTime; t< min(1440,EndTime); t++)
		{
			if(m_SensorTimeDependentTravelTime[t]>0.01f && m_TimeDependentTravelTime[t]>0.01f)   // with data
			{
			float sensor_value = GetTimeDependentMOEBy1Min(t,1);

			float simulation_value = GetTimeDependentMOE(t,0,MOEAggregationIntervalInMin);
			total_error += fabs(sensor_value- simulation_value)/max(0.1,sensor_value)*100;

			count++;
			}
		}

	
		return total_error/max(1,count);
	}


	float GetTimeDependentMOEBy1Min(int time, int MOEType)
	{

		switch(MOEType)
		{
		case 0: return m_TimeDependentTravelTime[time];
		case 1: return m_SensorTimeDependentTravelTime[time];

		case 2: return m_TimeDependentEnergy[time];
		case 3: return m_TimeDependentCO2[time];
		case 4: return m_TimeDependentNOX[time];
		case 5: return m_TimeDependentCO[time];
		case 6: return m_TimeDependentHC[time];
		case 7: return m_TimeDependentEnergy[time]/1000/(121.7);  // gallon
		case 8: return total_distance/max(0.01,m_TimeDependentEnergy[time]/1000/121.7);  // gallon

		//case 1: return m_WithinDayMaxTimeDependentTravelTime[time];
		//case 2: return m_WithinDayMeanTimeDependentTravelTime[time]+1.67f/3.0f*(m_WithinDayMaxTimeDependentTravelTime[time]-m_WithinDayMeanTimeDependentTravelTime[time]);  // max-mean --> 3 sigma, use mean+ 1.67 sigma as utility,
		//case 3: return (m_WithinDayMaxTimeDependentTravelTime[time]-m_WithinDayMeanTimeDependentTravelTime[time])/3.0f;
		//case 4: return m_WithinDayMeanTimeDependentFuelConsumption[time];
		//case 5: return m_WithinDayMeanTimeDependentEmissions[time];
		//case 6: return m_WithinDayMeanGeneralizedCost[time];


		default: return m_TimeDependentTravelTime[time];
		}

	}

	~DTAPath()
	{
	}

	bool m_bWithSensorTravelTime;
	float total_free_flow_travel_time;
	float total_distance;

	std::vector<int> m_LinkVector;
	std::vector<CString> m_PathLabelVector;
	std::string m_path_name;

	int m_NodeNodeSum;

	float m_TimeDependentTravelTime[1440];
	float m_SensorTimeDependentTravelTime[1440];

	float m_TimeDependentEnergy[1440];
	float m_TimeDependentCO2[1440];
	float m_TimeDependentCO[1440];
	float m_TimeDependentHC[1440];
	float m_TimeDependentNOX[1440];

	float m_TimeDependentCount[1440];

	float m_MaxTravelTime;

	float m_WithinDayMeanTimeDependentTravelTime[1440];
	float m_WithinDayMaxTimeDependentTravelTime[1440];

	float m_WithinDayMeanTimeDependentFuelConsumption[1440];  // unit: gallon
	float m_WithinDayMeanTimeDependentEmissions[1440];  // unit: pounds

	float m_WithinDayMeanGeneralizedCost[1440];  // unit: pounds

	int m_number_of_days;

	float m_Distance;

	float m_TravelTime;
	float m_Reliability;
	float m_Emissions;
	float m_Safety;
	float m_Fuel;
	int m_NumberOfSensorsPassed;
	int m_NumberOfPassiveSensorsPassed;
	int m_NumberOfActiveSensorsPassed;


};






// link element of a vehicle path
class SVehicleLink
{  public:
unsigned short  LinkNo;  // range:
float ArrivalTimeOnDSN;     // absolute arrvial time at downstream node of a link: 0 for the departure time, including delay/stop time
//   float LinkWaitingTime;   // unit: 0.1 seconds
SVehicleLink()
{
	LinkNo = -1;
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

class VehicleLocationRecord
{
public:
float x;
float y;
float time_stamp_in_min;
float distance_in_km;

};
class DTAVehicle
{
public:

	int m_Age;
	bool m_bGPSVehicle;  // GPS vehicle
	int m_NodeSize;
	int m_NodeNumberSum;  // used for comparing two paths
	SVehicleLink *m_NodeAry; // link list arrary of a vehicle path

	int m_VehicleLocationSize;
	VehicleLocationRecord *m_LocationRecordAry; // link list arrary of a vehicle path

	unsigned int m_RandomSeed;
	int m_VehicleID;  //range: +2,147,483,647
	int m_DateID;

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

	bool m_bODMarked;
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
		m_VehicleLocationSize = 0;
		m_bMarked = false;
		m_Age = 0;
		m_NodeNumberSum = 0;
		m_DateID = 0;
		m_DemandType = 1;
		m_VOT = 10;
		m_bGPSVehicle = false;
		m_bODMarked = false;
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

		if(m_LocationRecordAry!=NULL)
			delete m_LocationRecordAry;
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

	void Reset()
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
	int day_count[21];
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
		for(int d= 0; d<= 20; d++)
			day_count[d]= 0;

		TotalVehicleSize = 0;
		TotalTravelTime = 0;
		TotalVariance = 0;
		TotalDistance = 0;
		TotalCost = 0;
		TotalEmissions = 0;
		TotalGeneralizedCost = 0;
		TotalGeneralizedTime = 0;
		bImpactFlag = false;
		AvgTravelTime = 0;
		AvgDistance = 0;

		emissiondata.Reset();
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
	float AvgTravelTime;
	float AvgDistance;

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




extern float g_RNNOF();

extern std::vector<DTAPath>	m_PathDisplayList;
extern int m_SelectPathNo;
extern float g_Simulation_Time_Stamp;

extern int  g_SimulationStartTime_in_min;
