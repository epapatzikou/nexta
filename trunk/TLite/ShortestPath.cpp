//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com)

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html

//    This file is part of NEXTA  Version 3 (Open-source).

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

//shortest path calculation

// note that the current implementation is only suitable for time-dependent minimum time shortest path on FIFO network, rather than time-dependent minimum cost shortest path
// the key reference (1) Shortest Path Algorithms in Transportation Models http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.51.5192
// (2) most efficient time-dependent minimum cost shortest path algorithm for all departure times
// Time-dependent, shortest-path algorithm for real-time intelligent vehicle highway system applications&quot;, Transportation Research Record 1408 – Ziliaskopoulos, Mahmassani - 1993

#include "stdafx.h"
#include "Network.h"
int g_DepartureTimetInterval = 5; // min
extern long g_Simulation_Time_Horizon;


void DTANetworkForSP::BuildPhysicalNetwork(std::set<DTANode*>*	p_NodeSet, std::set<DTALink*>*		p_LinkSet, bool bRandomCost,bool bOverlappingCost)
{

	// build a network from the current zone centriod (1 centriod here) to all the other zones' centriods (all the zones)

	float Perception_error_ratio = 0.7;

	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;

	m_NodeSize = p_NodeSet->size();

	int IntervalLinkID=0;
	int FromID, ToID;

	int i,t;

	for(i=0; i< m_NodeSize; i++)
	{
		m_OutboundSizeAry[i] = 0;
		m_InboundSizeAry[i] = 0;

	}

	// add physical links

	for(iterLink = p_LinkSet->begin(); iterLink != p_LinkSet->end(); iterLink++)
	{
		FromID = (*iterLink)->m_FromNodeID;
		ToID   = (*iterLink)->m_ToNodeID;

		m_FromIDAry[(*iterLink)->m_LinkID] = FromID;
		m_ToIDAry[(*iterLink)->m_LinkID]   = ToID;

		//      TRACE("FromID %d -> ToID %d \n", FromID, ToID);
		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = (*iterLink)->m_LinkID ;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = (*iterLink)->m_LinkID  ;
		m_InboundSizeAry[ToID] +=1;


		ASSERT(m_AdjLinkSize > m_OutboundSizeAry[FromID]);

		int link_entering_time_interval;
		for(t=0; t <m_SimulationHorizon; t+=g_DepartureTimetInterval)
		{
			link_entering_time_interval= t/g_DepartureTimetInterval;
			if(link_entering_time_interval >= m_AssignmentIntervalSize)
				link_entering_time_interval = m_AssignmentIntervalSize-1;

			float AvgTravelTime = (*iterLink)->GetTravelTime (t,g_DepartureTimetInterval);

			float Normal_random_value = g_RNNOF() * Perception_error_ratio*AvgTravelTime;
			
			if(bRandomCost==false)
				Normal_random_value = 0;


			float travel_time  = AvgTravelTime + Normal_random_value;

			if(bOverlappingCost)
			{
				travel_time+=(*iterLink)->m_OverlappingCost ;
			}

			if(travel_time < (*iterLink)->m_FreeFlowTravelTime )
				travel_time = (*iterLink)->m_FreeFlowTravelTime;

			m_LinkTDTimeAry[(*iterLink)->m_LinkID][link_entering_time_interval] = travel_time;
			m_LinkTDCostAry[(*iterLink)->m_LinkID][link_entering_time_interval]=  travel_time;

			// use travel time now, should use cost later
		}


	}

	m_LinkSize = p_LinkSet->size();


}


bool DTANetworkForSP::TDLabelCorrecting_DoubleQueue(int origin, int departure_time, int vehicle_type)   // Pointer to previous node (node)
// time -dependent label correcting algorithm with deque implementation
{

	int i;
	int debug_flag = 0;

	if(m_OutboundSizeAry[origin]== 0)
		return false;

	for(i=0; i <m_NodeSize; i++) // Initialization for all nodes
	{
		NodePredAry[i]  = -1;
		NodeStatusAry[i] = 0;

		LabelTimeAry[i] = MAX_SPLABEL;
		LabelCostAry[i] = MAX_SPLABEL;

	}

	// Initialization for origin node
	LabelTimeAry[origin] = float(departure_time);
	LabelCostAry[origin] = 0;

	SEList_clear();
	SEList_push_front(origin);

	int FromID, LinkNo, ToID;


	float NewTime, NewCost;
	while(!SEList_empty())
	{
		FromID  = SEList_front();
		SEList_pop_front();

		if(debug_flag)
			  TRACE("\nScan from node %d",FromID);

		NodeStatusAry[FromID] = 2;        //scaned

		for(i=0; i<m_OutboundSizeAry[FromID];  i++)  // for each arc (i,j) belong A(j)
		{
			LinkNo = m_OutboundLinkAry[FromID][i];
			ToID = m_OutboundNodeAry[FromID][i];

			if(ToID == origin)
				continue;


//					  TRACE("\n   to node %d",ToID);
			// need to check here to make sure  LabelTimeAry[FromID] is feasible.


			int link_entering_time_interval = int(LabelTimeAry[FromID])/g_DepartureTimetInterval;
			if(link_entering_time_interval >= m_AssignmentIntervalSize)  // limit the size
				link_entering_time_interval = m_AssignmentIntervalSize-1;

			if(link_entering_time_interval < 0)  // limit the size
				link_entering_time_interval = 0;

			NewTime	 = LabelTimeAry[FromID] + m_LinkTDTimeAry[LinkNo][link_entering_time_interval];  // time-dependent travel times come from simulator
			NewCost    = LabelCostAry[FromID] + m_LinkTDCostAry[LinkNo][link_entering_time_interval];       // costs come from time-dependent tolls, VMS, information provisions

			if(NewCost < LabelCostAry[ToID] ) // be careful here: we only compare cost not time
			{

//					       TRACE("\n         UPDATE to %f, link travel time %f", NewCost, m_LinkTDCostAry[LinkNo][link_entering_time_interval]);

				if(NewTime > g_Simulation_Time_Horizon -1)
					NewTime = float(g_Simulation_Time_Horizon-1);

				LabelTimeAry[ToID] = NewTime;
				LabelCostAry[ToID] = NewCost;
				NodePredAry[ToID]   = FromID;

				// Dequeue implementation
				//
				if(NodeStatusAry[ToID]==2) // in the SEList_TD before
				{
					SEList_push_front(ToID);
					NodeStatusAry[ToID] = 1;
				}
				if(NodeStatusAry[ToID]==0)  // not be reached
				{
					SEList_push_back(ToID);
					NodeStatusAry[ToID] = 1;
				}

				//another condition: in the SELite now: there is no need to put this node to the SEList, since it is already there.
			}

		}      // end of for each link

	} // end of while
	return true;
}



