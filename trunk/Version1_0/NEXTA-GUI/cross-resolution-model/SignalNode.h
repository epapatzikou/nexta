
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



class DTA_NodePhaseSet
{
public:
	bool m_bPresetValueFromQEMExist;
	int CurrentNodeID;
	float BRP, MinGreen, MaxGreen, VehExt, TimeBeforeReduce, TimeToReduce, MinGap, Yellow, AllRed, Recall, Walk, DontWalk;
	float PedCalls, MinSplit, DualEntry, InhibitMax, Start, End, Yield, Yield170, LocalStart, LocalYield, LocalYield170; 
	CDataElement DataMatrix[23][8];
	// std::vector <int> PhaseMovementAry; // contains the index of Movements in NodeMovementsAry


	DTA_NodePhaseSet()
	{
		m_bPresetValueFromQEMExist = false;
	}

	void copy_parameters(DTA_NodePhaseSet P)
	{
		if(m_bPresetValueFromQEMExist== false)  // if only we do not have data from QEM
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
	bool link_flag[8];
	int Up_ID[8], Lanes[8];
	std::string Name[8];
	float DataMatrix[15][8];

	DTA_NodeBasedLinkSets()
	{
	}

	void initial(int node_id)
	{		
		for (int i=0; i<8; i++)
			link_flag[i] = false;

		nodeID = node_id;
	}

	void LoadLink(DTALink * link, DTA_Direction approach)
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

class DTA_Movement
{
public:
	int CurrentNodeID; int InboundLinkID;
    DTA_APPROACH_TURN movement_dir;
	DTA_Direction movement_approach;
	DTA_Turn movement_turn;
	int UpNodeID,  DestNodeID;
	int Lanes, Shared, Width, Storage, StLanes, Grade, Speed, FirstDetect, LastDetect, Phase1, PermPhase1, DetectPhase1, IdealFlow, LostTime; 
	int	SatFlow, SatFlowPerm, SatFlowRTOR, HeadwayFact, Volume, Peds, Bicycles, Growth, HeavyVehicles, BusStops, Midblock, Distance; 
	float PHF, TravelTime;

	DTA_Movement()
	{
	}


};

class DTA_NodeMovementSet
{
public:
	int CurrentNodeID; 
	CDataElement DataMatrix[30][32];
	DTA_Movement MovementMatrix[32];   //32 possible movements
//	DTA_NodeMovementSet &operator=( DTA_NodeMovementSet & );

	DTA_NodeMovementSet()
	{
		//Default values here
	}

	void copy_from_Movement(DTA_Movement movement, DTA_APPROACH_TURN c)
	{

		int i = 0;  // i is row index 
		DataMatrix[i][c].m_text = movement.UpNodeID; i++; 
		DataMatrix[i][c].m_text = movement.DestNodeID; i++; 
		DataMatrix[i][c].m_text = movement.Lanes; i++; 
		DataMatrix[i][c].m_text = movement.Shared; i++; 
		DataMatrix[i][c].m_text = movement.Width; i++; 
		DataMatrix[i][c].m_text = movement.Storage; i++; 
		DataMatrix[i][c].m_text = movement.StLanes; i++; 
		DataMatrix[i][c].m_text = movement.Grade; i++; 
		DataMatrix[i][c].m_text = movement.Speed; i++; 
		DataMatrix[i][c].m_text = movement.FirstDetect; i++; 
		DataMatrix[i][c].m_text = movement.LastDetect; i++; 
		DataMatrix[i][c].m_text = movement.Phase1; i++; 
		DataMatrix[i][c].m_text = movement.PermPhase1; i++; 
		DataMatrix[i][c].m_text = movement.DetectPhase1; i++; 
		DataMatrix[i][c].m_text = movement.IdealFlow; i++; 
		DataMatrix[i][c].m_text = movement.LostTime; i++; 
		DataMatrix[i][c].m_text = movement.SatFlow; i++; 
		DataMatrix[i][c].m_text = movement.SatFlowPerm; i++; 
		DataMatrix[i][c].m_text = movement.SatFlowRTOR; i++; 
		DataMatrix[i][c].m_text = movement.HeadwayFact; i++; 
		DataMatrix[i][c].m_text = movement.Volume; i++; 
		DataMatrix[i][c].m_text = movement.Peds; i++; 
		DataMatrix[i][c].m_text = movement.Bicycles; i++; 
		DataMatrix[i][c].m_text = movement.PHF; i++; 
		DataMatrix[i][c].m_text = movement.Growth; i++; 
		DataMatrix[i][c].m_text = movement.HeavyVehicles; i++; 
		DataMatrix[i][c].m_text = movement.BusStops; i++; 
		DataMatrix[i][c].m_text = movement.Midblock; i++; 
		DataMatrix[i][c].m_text = movement.Distance; i++; 
		DataMatrix[i][c].m_text = movement.TravelTime; i++; 
	}

	void copy_to_Movement(DTA_Movement &movement, DTA_APPROACH_TURN c)
	{
		int i = 2;
		movement.Lanes = DataMatrix[i][c].m_text; i++; 
		movement.Shared = DataMatrix[i][c].m_text; i++; 
		movement.Width = DataMatrix[i][c].m_text; i++; 
		movement.Storage = DataMatrix[i][c].m_text; i++; 
		movement.StLanes = DataMatrix[i][c].m_text; i++; 
		movement.Grade = DataMatrix[i][c].m_text; i++; 
		movement.Speed = DataMatrix[i][c].m_text; i++; 
		movement.FirstDetect = DataMatrix[i][c].m_text; i++; 
		movement.LastDetect = DataMatrix[i][c].m_text; i++; 
		movement.Phase1 = DataMatrix[i][c].m_text; i++; 
		movement.PermPhase1 = DataMatrix[i][c].m_text; i++; 
		movement.DetectPhase1 = DataMatrix[i][c].m_text; i++; 
		movement.IdealFlow = DataMatrix[i][c].m_text; i++; 
		movement.LostTime = DataMatrix[i][c].m_text; i++; 
		movement.SatFlow = DataMatrix[i][c].m_text; i++; 
		movement.SatFlowPerm = DataMatrix[i][c].m_text; i++; 
		movement.SatFlowRTOR = DataMatrix[i][c].m_text; i++; 
		movement.HeadwayFact = DataMatrix[i][c].m_text; i++; 
		movement.Volume = DataMatrix[i][c].m_text; i++; 
		movement.Peds = DataMatrix[i][c].m_text; i++; 
		movement.Bicycles = DataMatrix[i][c].m_text; i++; 
		movement.PHF = DataMatrix[i][c].m_text; i++; 
		movement.Growth = DataMatrix[i][c].m_text; i++; 
		movement.HeavyVehicles = DataMatrix[i][c].m_text; i++; 
		movement.BusStops = DataMatrix[i][c].m_text; i++; 
		movement.Midblock = DataMatrix[i][c].m_text; i++; 
		movement.Distance = DataMatrix[i][c].m_text; i++; 
		movement.TravelTime = DataMatrix[i][c].m_text; i++; 
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




