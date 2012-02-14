//  Portions Copyright 2010 Xuesong Zhou, Jason Lu

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


// prototype 1: heterogeneous traveler
// prototype 2: departure time and mode options
// prototype 3: extention

// assignment module
// obtain simulation results, fetch shortest paths, assign vehicles to the shortest path according to gap function or MSA
#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"
#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <stdlib.h>  // Jason
#include <math.h>    // Jason

void g_OutputVOTStatistics()
{
/*
	int vot;
	for( vot = 0; vot<MAX_VOT_RANGE; vot++)
	{
	g_VOTStatVector[vot].TotalVehicleSize = 0;
	g_VOTStatVector[vot].TotalTravelTime = 0;
	g_VOTStatVector[vot].TotalDistance  = 0;

	}
	for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end();v++)
	{
	if((*v)->m_bComplete )  // vehicle completes the trips
	{
	vot = (*v)->m_VehData .m_VOT;
	g_VOTStatVector[vot].TotalVehicleSize +=1;
	g_VOTStatVector[vot].TotalTravelTime += (*v)->m_TripTime;
	g_VOTStatVector[vot].TotalDistance += (*v)->m_Distance;

	}
	}

	for(vot = 0; vot<MAX_VOT_RANGE; vot++)
	{
	if(g_VOTStatVector[vot].TotalVehicleSize > 0 )
	{
	g_AssignmentLogFile << "VOT= " << vot << ", # of vehicles = " << g_VOTStatVector[vot].TotalVehicleSize << ", Avg Travel Time = " << g_VOTStatVector[vot].TotalTravelTime/g_VOTStatVector[vot].TotalVehicleSize << ", Avg Distance = " << g_VOTStatVector[vot].TotalDistance /g_VOTStatVector[vot].TotalVehicleSize << endl;

	}
	}
	*/
}

int DTANetworkForSP::FindOptimalSolution(int origin, int departure_time, int destination,int PathNodeList[MAX_NODE_SIZE_IN_A_PATH])  // the last pointer is used to get the node array
{

	// step 1: scan all the time label at destination node, consider time cost
	// step 2: backtrace to the origin (based on node and time predecessors)
	// step 3: reverse the backward path
	// return final optimal solution

	// step 1: scan all the time label at destination node, consider time cost
	int tmp_AryTN[MAX_NODE_SIZE_IN_A_PATH]; //backward temporal solution

	float min_cost = MAX_SPLABEL;
	int min_cost_time_index = -1;

	/*

	for(int t=departure_time; t <m_SimulationHorizon; t++)
	{
		if(TD_LabelCostAry[destination][t] < min_cost)
		{
			min_cost = TD_LabelCostAry[destination][t];
			min_cost_time_index = t;
		}

	}
		
	ASSERT(min_cost_time_index>0); // if min_cost_time_index ==-1, then no feasible path if founded

	// step 2: backtrace to the origin (based on node and time predecessors)

	int	NodeSize = 0;

	//record the first node backward, destination node
	tmp_AryTN[NodeSize]= destination;

	NodeSize++;

	int PredTime = TD_TimePredAry[destination][min_cost_time_index];
	int PredNode = TD_NodePredAry[destination][min_cost_time_index];

	while(PredNode != origin && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
	{
		ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);

		tmp_AryTN[NodeSize]= PredNode;
		NodeSize++;

		//record current values of node and time predecessors, and update PredNode and PredTime
		int PredTime_cur = PredTime;
		int PredNode_cur = PredNode;

		PredNode = TD_NodePredAry[PredNode_cur][PredTime_cur];
		PredTime = TD_TimePredAry[PredNode_cur][PredTime_cur];

	}

	tmp_AryTN[NodeSize] = origin;

	NodeSize++;

	// step 3: reverse the backward solution

	int i;
	for(i = 0; i< NodeSize; i++)
	{
		PathNodeList[i]		= tmp_AryTN[NodeSize-1-i];
	}

	return NodeSize;

	*/

	return 0;
}



/************************
//below is research code for multi-day traffic assignment, should not be included in the official release
		if(MAX_DAY_SIZE >=1)
		{
			g_MultiDayTrafficAssisnment();
		}

*************************/

