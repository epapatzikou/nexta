
#pragma once

#include <math.h>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <string>
#include <vector>
#include <list>
using namespace std;

enum LANES_COLUME
   {
		DTA_NBL =0,
		DTA_NBT,
		DTA_NBR,
		DTA_SBL,
		DTA_SBT,
		DTA_SBR,
		DTA_EBL,
		DTA_EBT,
		DTA_EBR,
		DTA_WBL,
		DTA_WBT,
		DTA_WBR
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

class CDataElement
{
public:
	int colume_index, row_index;
	float m_text;

	CDataElement()
	{
	m_text = -1;
	colume_index = 0;
	row_index = 0;

	}

};


class DTA_NodeMovementSet
{
public:
	int CurrentNodeID; 
	CDataElement DataMatrix[30][12];
//	DTA_NodeMovementSet &operator=( DTA_NodeMovementSet & );

	DTA_NodeMovementSet()
	{
		//Default values here
	}


	void copy_parameters(DTA_NodeMovementSet M)
	{
		int i,j;
		CurrentNodeID = M.CurrentNodeID;
		for(j=0; j<12;j++)
		{
			for(i=0; i<30; i++)
			{
				DataMatrix[i][j] = M.DataMatrix[i][j];
			}
		}
	}
};

class DTA_Movement
{
public:
	int CurrentNodeID; int InboundLinkID;
    LANES_COLUME dir;
	DTA_Approach movement_approach;
	DTA_Turn movement_turn;
	int UpNodeID,  DestNodeID;
	int Lanes, Shared, Width, Storage, StLanes, Grade, Speed, FirstDetect, LastDetect, Phase1, PermPhase1, DetectPhase1, IdealFlow, LostTime; 
	int	SatFlow, SatFlowPerm, SatFlowRTOR, HeadwayFact, Volume, Peds, Bicycles, Growth, HeavyVehicles, BusStops, Midblock, Distance; 
	float PHF, TravelTime;

	DTA_Movement()
	{
	}

	void copy_to_MovementSet(DTA_NodeMovementSet& movement_set, LANES_COLUME c)
	{
		int i = 0;
		movement_set.DataMatrix[i][c].m_text = UpNodeID; i++; 
		movement_set.DataMatrix[i][c].m_text = DestNodeID; i++; 
		movement_set.DataMatrix[i][c].m_text = Lanes; i++; 
		movement_set.DataMatrix[i][c].m_text = Shared; i++; 
		movement_set.DataMatrix[i][c].m_text = Width; i++; 
		movement_set.DataMatrix[i][c].m_text = Storage; i++; 
		movement_set.DataMatrix[i][c].m_text = StLanes; i++; 
		movement_set.DataMatrix[i][c].m_text = Grade; i++; 
		movement_set.DataMatrix[i][c].m_text = Speed; i++; 
		movement_set.DataMatrix[i][c].m_text = FirstDetect; i++; 
		movement_set.DataMatrix[i][c].m_text = LastDetect; i++; 
		movement_set.DataMatrix[i][c].m_text = Phase1; i++; 
		movement_set.DataMatrix[i][c].m_text = PermPhase1; i++; 
		movement_set.DataMatrix[i][c].m_text = DetectPhase1; i++; 
		movement_set.DataMatrix[i][c].m_text = IdealFlow; i++; 
		movement_set.DataMatrix[i][c].m_text = LostTime; i++; 
		movement_set.DataMatrix[i][c].m_text = SatFlow; i++; 
		movement_set.DataMatrix[i][c].m_text = SatFlowPerm; i++; 
		movement_set.DataMatrix[i][c].m_text = SatFlowRTOR; i++; 
		movement_set.DataMatrix[i][c].m_text = HeadwayFact; i++; 
		movement_set.DataMatrix[i][c].m_text = Volume; i++; 
		movement_set.DataMatrix[i][c].m_text = Peds; i++; 
		movement_set.DataMatrix[i][c].m_text = Bicycles; i++; 
		movement_set.DataMatrix[i][c].m_text = PHF; i++; 
		movement_set.DataMatrix[i][c].m_text = Growth; i++; 
		movement_set.DataMatrix[i][c].m_text = HeavyVehicles; i++; 
		movement_set.DataMatrix[i][c].m_text = BusStops; i++; 
		movement_set.DataMatrix[i][c].m_text = Midblock; i++; 
		movement_set.DataMatrix[i][c].m_text = Distance; i++; 
		movement_set.DataMatrix[i][c].m_text = TravelTime; i++; 
	}

	void copy_from_MovementSet(DTA_NodeMovementSet movement_set, LANES_COLUME c)
	{
		int i = 2;
		Lanes = movement_set.DataMatrix[i][c].m_text; i++; 
		Shared = movement_set.DataMatrix[i][c].m_text; i++; 
		Width = movement_set.DataMatrix[i][c].m_text; i++; 
		Storage = movement_set.DataMatrix[i][c].m_text; i++; 
		StLanes = movement_set.DataMatrix[i][c].m_text; i++; 
		Grade = movement_set.DataMatrix[i][c].m_text; i++; 
		Speed = movement_set.DataMatrix[i][c].m_text; i++; 
		FirstDetect = movement_set.DataMatrix[i][c].m_text; i++; 
		LastDetect = movement_set.DataMatrix[i][c].m_text; i++; 
		Phase1 = movement_set.DataMatrix[i][c].m_text; i++; 
		PermPhase1 = movement_set.DataMatrix[i][c].m_text; i++; 
		DetectPhase1 = movement_set.DataMatrix[i][c].m_text; i++; 
		IdealFlow = movement_set.DataMatrix[i][c].m_text; i++; 
		LostTime = movement_set.DataMatrix[i][c].m_text; i++; 
		SatFlow = movement_set.DataMatrix[i][c].m_text; i++; 
		SatFlowPerm = movement_set.DataMatrix[i][c].m_text; i++; 
		SatFlowRTOR = movement_set.DataMatrix[i][c].m_text; i++; 
		HeadwayFact = movement_set.DataMatrix[i][c].m_text; i++; 
		Volume = movement_set.DataMatrix[i][c].m_text; i++; 
		Peds = movement_set.DataMatrix[i][c].m_text; i++; 
		Bicycles = movement_set.DataMatrix[i][c].m_text; i++; 
		PHF = movement_set.DataMatrix[i][c].m_text; i++; 
		Growth = movement_set.DataMatrix[i][c].m_text; i++; 
		HeavyVehicles = movement_set.DataMatrix[i][c].m_text; i++; 
		BusStops = movement_set.DataMatrix[i][c].m_text; i++; 
		Midblock = movement_set.DataMatrix[i][c].m_text; i++; 
		Distance = movement_set.DataMatrix[i][c].m_text; i++; 
		TravelTime = movement_set.DataMatrix[i][c].m_text; i++; 
	}
};



class DTA_NodePhaseSet
{
public:
	int CurrentNodeID;
	float BRP, MinGreen, MaxGreen, VehExt, TimeBeforeReduce, TimeToReduce, MinGap, Yellow, AllRed, Recall, Walk, DontWalk;
	float PedCalls, MinSplit, DualEntry, InhibitMax, Start, End, Yield, Yield170, LocalStart, LocalYield, LocalYield170; 
	CDataElement DataMatrix[23][8];
	// std::vector <int> PhaseMovementAry; // contains the index of Movements in NodeMovementsAry


	DTA_NodePhaseSet()
	{
	}

	void copy_parameters(DTA_NodePhaseSet P)
	{
		int i,j;
		CurrentNodeID = P.CurrentNodeID;
		for(j=0; j<8;j++)
		{
			for(i=0; i<23; i++)
			{
				DataMatrix[i][j] = P.DataMatrix[i][j];
			}
		}
	}

};

class DTA_TimePlan
{
public:
	int PlanID;
	int DataAry[10];
		//Control_Type, Cycle_Length, Lock_Timings, Referenced_To, Reference_Phase, Offset, Master, Yield, Node_0, Node_1;

	DTA_TimePlan()
	{
	}

	void initial()
	{
		PlanID = 1;
		DataAry[0] = 3;
		DataAry[1] = 120;
		DataAry[2] = 0;
		DataAry[3] = 0;
		DataAry[4] = 206;
		DataAry[5] = 32;
		DataAry[6] = 0;
		DataAry[7] = 0;
		DataAry[8] = 32;
		DataAry[9] = 0;
	}
};

class DTA_Timing
{
public:
	int Node_id;
	int DataAry[8];
	int Cycle, OFF, LD, CLR;
	string REF;

	DTA_Timing()
	{
	}

	void initial(DTA_NodePhaseSet Phase, int cl)
	{
		Cycle = cl;
		for(int i=0; i<8; i++)
		{
			DataAry[i] = (int)(Phase.DataMatrix[PHASE_End][i].m_text) - (int)(Phase.DataMatrix[PHASE_Start][i].m_text);
			if (DataAry[i] < 0)
				DataAry[i] += Cycle;
		}
		LD = 1357;
		OFF = 0;
		REF = "26+";
	}

};

enum NodeBasedLinkData
{
	Distance,
	Speed,
	Time,
	Grade,
	Median,
	Offset,
	TWLTL,
	Crosswalk_Width,
	Mandatory_Distance,
	Mandatory_Distance2,
	Positioning_Distance,
	Positioning_Distance2,
	Curve_Pt_X,
	Curve_Pt_Y,
	Curve_Pt_Z
};

class DTA_NodeBasedLinkSets
{
public:
	int nodeID;
	bool link_flag[4];
	int Up_ID[4], Lanes[4];
	std::string Name[4];
	float DataMatrix[15][4];

	DTA_NodeBasedLinkSets()
	{
	}

	void initial(int node_id)
	{		
		for (int i=0; i<4; i++)
			link_flag[i] = false;
		nodeID = node_id;
	}

	void LoadLink(DTALink * link, DTA_Approach approach)
	{
		link_flag[approach] = true;
		Up_ID[approach] = link->m_FromNodeID;
		Lanes[approach] = link->m_NumLanes;
		Name[approach] = link->m_Name;
		DataMatrix[Distance][approach] = link->m_Length;
		DataMatrix[Speed][approach] = link->m_SpeedLimit;
		DataMatrix[Time][approach] = link->m_FreeFlowTravelTime;
		DataMatrix[Grade][approach] = link->m_Grade;
		DataMatrix[Median][approach] = 12;
		DataMatrix[Offset][approach] = 0;
		DataMatrix[TWLTL][approach] = 0;
		DataMatrix[Crosswalk_Width][approach] = 16;
		DataMatrix[Mandatory_Distance][approach] = link->DefaultDistance();
		DataMatrix[Mandatory_Distance2][approach] = link->DefaultDistance();
		DataMatrix[Positioning_Distance][approach] = link->DefaultDistance();
		DataMatrix[Positioning_Distance2][approach] = link->DefaultDistance();
		DataMatrix[Curve_Pt_X][approach] = 0;
		DataMatrix[Curve_Pt_Y][approach] = 0;
		DataMatrix[Curve_Pt_Z][approach] = 0;

	}

};