
#pragma once

#include <math.h>
#include <deque>
#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <list>
using namespace std;






class DTAMovement
{
public:
	int CurrentNodeID; int InboundLinkID;
	DTA_Approach movement_approach;
	DTA_Turn movement_turn;
	int UpNodeID,  DestNodeID, LaneSize, Shared, Width, Storage, Speed, LostTime, IdealFlow, SatFlow, AllowRTOR, SatFlowRTOR;
	int Volume, Peds, Bicycles, Growth, HeavyVehicles, BusStops, Midblock, Distance, RightChanneled, RightRadius, Alignment, EnterBlocked, HeadwayFact, TurningSpeed; 
	float PHF, TravelTime;
	
//	DTAMovement &operator=( DTAMovement & );

	// How to quickly generate a DTAMovement object: 
	// 1. load default values from file and generate a DTAMovement template,
	// 2. use overload operator "=" to quickly generate new DTAMovement objects
	// 3. when loading default values from file, up to 12 DTAMovement templates may be generated: NBL, NBT, NBR, SBL, SBT, SBR, EBL, EBT, EBR, WBL, WBT, WBR
	// 4. for simplisity, 12 templates can be grouped into 3: left turn, right turn, through
	DTAMovement()
	{
		//Default values here
	}

	DTAMovement(int _UpNodeID, int _DestNodeID)
	{
		UpNodeID = _UpNodeID;
		DestNodeID = _DestNodeID;
	}
};

/*
// Define assignment operator.
DTAMovement &DTAMovement::operator=( DTAMovement &other )
{
	
	UpNodeID = other.UpNodeID;
	DestNodeID = other.DestNodeID;
	CurrentNodeID = other.CurrentNodeID;


	LaneSize = other.LaneSize;
	Shared = other.Shared;
	Width = other.Width;
	Storage = other.Storage;
	Speed = other.Speed;
	LostTime = other.LostTime;
	IdealFlow = other.IdealFlow;
	SatFlow = other.SatFlow;
	AllowRTOR = other.AllowRTOR;
	SatFlowRTOR = other.SatFlowRTOR;
	Volume = other.Volume;
	Peds = other.Peds;
	Bicycles = other.Bicycles;
	Growth = other.Growth;
	HeavyVehicles = other.HeavyVehicles;
	BusStops = other.BusStops;
	Midblock = other.Midblock;
	Distance = other.Distance;
	RightChanneled = other.RightChanneled;
	RightRadius = other.RightRadius;
	Alignment = other.Alignment;
	EnterBlocked = other.EnterBlocked;
	HeadwayFact = other.HeadwayFact;
	TurningSpeed  = other.TurningSpeed ;
	PHF = other.PHF;
	TravelTime = other.TravelTime;

	return *this;  // Assignment operator returns left side.
}
*/




class DTAPhase
{
public:
	float BRP, MinGreen, MaxGreen, VehExt, TimeBeforeReduce, TimeToReduce, MinGap, Yellow, AllRed, Recall, Walk;
	float DontWalk, PedCalls, MinSplit, DualEntry, InhibitMax, Start, End, Yield, Yield170, LocalStart, LocalYield, LocalYield170, ActGreen; 
	//std::vector <DTAMovement> PhaseMovementAry;
	std::vector <int> PhaseMovementAry; // contains the index of Movements in NodeMovementsAry

	DTAPhase &operator=( DTAPhase & );

	DTAPhase()
	{
	}


};
class DTASignalNode
{ 
public:
	int m_NodeID;
	std::vector <DTAMovement> NodeMovementAry; // Movement vector, 0 - 11: NBL, NBT, NBR, SBL, SBT, SBR, EBL, EBT, EBR, WBL, WBT, WBR


	std::vector <DTAPhase> NodePhaseAry;

	void ConstructPhase()
	{  
		//given related NodeMovementAry, Approach, 
		// group movement
		// generate phase
	
	}
	
	DTASignalNode(int _NodeID)
	{
		m_NodeID = _NodeID;
	}

	DTASignalNode(int _NodeID, DTAMovement NBL, DTAMovement NBT, DTAMovement NBR, DTAMovement SBL, DTAMovement SBT, 
		DTAMovement SBR, DTAMovement EBL, DTAMovement EBT, DTAMovement EBR, DTAMovement WBL, DTAMovement WBT, DTAMovement WBR)
	{	
		m_NodeID = _NodeID;
		// set Movement
		NodeMovementAry.resize(12);
		NodeMovementAry[0] = NBL;
		NodeMovementAry[1] = NBT;
		NodeMovementAry[2] = NBR;
		NodeMovementAry[3] = SBL;
		NodeMovementAry[4] = SBT;
		NodeMovementAry[5] = SBR;
		NodeMovementAry[6] = EBL;
		NodeMovementAry[7] = EBT;
		NodeMovementAry[8] = EBR;
		NodeMovementAry[9] = WBL;
		NodeMovementAry[10] = WBT;
		NodeMovementAry[11] = WBR;
		// set Phase

	}

	void InitMovement(DTAMovement LeftTurn, DTAMovement Through, DTAMovement RightTurn)
	{
		NodeMovementAry.resize(12);
		NodeMovementAry[0] = LeftTurn;
		NodeMovementAry[1] = Through;
		NodeMovementAry[2] = RightTurn;
		NodeMovementAry[3] = LeftTurn;
		NodeMovementAry[4] = Through;
		NodeMovementAry[5] = RightTurn;
		NodeMovementAry[6] = LeftTurn;
		NodeMovementAry[7] = Through;
		NodeMovementAry[8] = RightTurn;
		NodeMovementAry[9] = LeftTurn;
		NodeMovementAry[10] = Through;
		NodeMovementAry[11] = RightTurn;
	}

	void InitMovement(DTAMovement NBL, DTAMovement NBT, DTAMovement NBR, DTAMovement SBL, DTAMovement SBT, 
		DTAMovement SBR, DTAMovement EBL, DTAMovement EBT, DTAMovement EBR, DTAMovement WBL, DTAMovement WBT, DTAMovement WBR)
	{	
		NodeMovementAry.resize(12);
		NodeMovementAry[0] = NBL;
		NodeMovementAry[1] = NBT;
		NodeMovementAry[2] = NBR;
		NodeMovementAry[3] = SBL;
		NodeMovementAry[4] = SBT;
		NodeMovementAry[5] = SBR;
		NodeMovementAry[6] = EBL;
		NodeMovementAry[7] = EBT;
		NodeMovementAry[8] = EBR;
		NodeMovementAry[9] = WBL;
		NodeMovementAry[10] = WBT;
		NodeMovementAry[11] = WBR;
	}

};