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

//shortest path calculation

// note that the current implementation is only suitable for time-dependent minimum time shortest path on FIFO network, rather than time-dependent minimum cost shortest path
// the key reference (1) Shortest Path Algorithms in Transportation Models http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.51.5192
// (2) most efficient time-dependent minimum cost shortest path algorithm for all departure times
// Time-dependent, shortest-path algorithm for real-time intelligent vehicle highway system applications&quot;, Transportation Research Record 1408 – Ziliaskopoulos, Mahmassani - 1993

#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"

void DTANetworkForSP::BuildNetwork(int CurZoneID)  // build the network for shortest path calculation and fetch travel time and cost data from simulator
{

	// build a network from the current zone centriod (1 centriod here) to all the other zones' centriods (all the zones)

	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;

	m_PhysicalNodeSize = g_NodeVector.size();

	int IntervalLinkID=0;
	int FromID, ToID;

	unsigned int i;
	int t;

	for(i=0; i< m_PhysicalNodeSize + g_ODZoneSize+1; i++)
	{
		m_OutboundSizeAry[i] = 0;
		m_InboundSizeAry[i] =0;
	}

	// add physical links

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		FromID = g_LinkVector[li]->m_FromNodeID;
		ToID   = g_LinkVector[li]->m_ToNodeID;

		m_FromIDAry[g_LinkVector[li]->m_LinkID] = FromID;
		m_ToIDAry[g_LinkVector[li]->m_LinkID]   = ToID;

		//      TRACE("FromID %d -> ToID %d \n", FromID, ToID);
		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = g_LinkVector[li]->m_LinkID ;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = g_LinkVector[li]->m_LinkID ;
		m_InboundSizeAry[ToID] +=1;

		ASSERT(g_AdjLinkSize > m_OutboundSizeAry[FromID]);


		for(t = 0; t < m_SimulationHorizon; t +=g_DepartureTimetInterval)
		{
		int	link_entering_time_interval= t/g_DepartureTimetInterval;
			if(link_entering_time_interval >= m_AssignmentIntervalSize)
				link_entering_time_interval = m_AssignmentIntervalSize-1;

			if(g_LinkVector[li]->m_FromNodeNumber == 48 && g_LinkVector[li]->m_ToNodeNumber == 201)
				TRACE("");
			// we obtain simulated time-dependent travel time measurments from simulator, use that for time-dependent shortest path calculation
			float AvgTravelTime = g_LinkVector[li]->GetTravelTime (t,g_DepartureTimetInterval);

//			TRACE("%d -> %d, t %d, %f\n", g_NodeVector[g_LinkVector[li]->m_FromNodeID], g_NodeVector[g_LinkVector[li]->m_ToNodeID],t,AvgTravelTime);

			m_LinkTDTimeAry[g_LinkVector[li]->m_LinkID][link_entering_time_interval] = AvgTravelTime;
			m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][link_entering_time_interval]=  AvgTravelTime;
			// use travel time now, should use cost later
		}


	}

	int LinkID = g_LinkVector.size();

	// add outgoing connectors from origin zone to destination node
	for(i = 0; i< g_ZoneVector[CurZoneID].m_CentroidNodeAry.size(); i++)
	{
		FromID = m_PhysicalNodeSize; // m_PhysicalNodeSize is the centriod number for CurZoneNo
		ToID = g_ZoneVector[CurZoneID].m_CentroidNodeAry[i];
		// add outcoming connector from the centriod corresponding to the current zone: node ID = m_PhysicalNodeSize, to this physical node's ID

		//         TRACE("destination node of current zone %d: %d\n",CurZoneID, g_NodeVector[ToID]);

		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = LinkID;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = LinkID ;
		m_InboundSizeAry[ToID] +=1;


		ASSERT(g_AdjLinkSize > m_OutboundSizeAry[FromID]);

		for(int t=0; t <m_AssignmentIntervalSize; t++)
		{
			m_LinkTDTimeAry[LinkID][t] = 0;
			m_LinkTDCostAry[LinkID][t]= 0;
		}

		LinkID++;

	}

	// add incoming connector to destination zone which is not CurZoneNo connector from the centriod corresponding to the current zone: node ID = m_PhysicalNodeSize, to this physical node's ID
	for(int z= 1; z<= g_ODZoneSize; z++)
	{
		if(z != CurZoneID)
		{
			for(i = 0; i< g_ZoneVector[z].m_CentroidNodeAry.size(); i++)
			{
				FromID = g_ZoneVector[z].m_CentroidNodeAry[i];; // m_PhysicalNodeSize is the centriod number for CurZoneNo
				ToID =   m_PhysicalNodeSize + z; // m_PhysicalNodeSize is the centriod number for CurZoneNo, note that  .m_ZoneID start from 1

				m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
				m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = LinkID;
				m_OutboundSizeAry[FromID] +=1;

					m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = LinkID ;
					m_InboundSizeAry[ToID] +=1;


				ASSERT(g_AdjLinkSize >  m_OutboundSizeAry[FromID]);

				for( t=0; t <m_AssignmentIntervalSize; t++)
				{
					m_LinkTDTimeAry[LinkID][t] = 0;
					m_LinkTDCostAry[LinkID][t]= 0;
				}

				LinkID++;


			}
		}
	}

	m_NodeSize = m_PhysicalNodeSize + 1 + g_ODZoneSize;

}

void DTANetworkForSP::BuildPhysicalNetwork()
{

	// build a network from the current zone centriod (1 centriod here) to all the other zones' centriods (all the zones)

	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;

	m_NodeSize = g_NodeVector.size();

	int IntervalLinkID=0;
	int FromID, ToID;

	int i,t;

	for(i=0; i< m_NodeSize; i++)
	{
		m_OutboundSizeAry[i] = 0;
		m_InboundSizeAry[i] = 0;

	}

	// add physical links

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		FromID = g_LinkVector[li]->m_FromNodeID;
		ToID   = g_LinkVector[li]->m_ToNodeID;

		m_FromIDAry[g_LinkVector[li]->m_LinkID] = FromID;
		m_ToIDAry[g_LinkVector[li]->m_LinkID]   = ToID;

		//      TRACE("FromID %d -> ToID %d \n", FromID, ToID);
		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = g_LinkVector[li]->m_LinkID ;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = g_LinkVector[li]->m_LinkID  ;
		m_InboundSizeAry[ToID] +=1;


		ASSERT(g_AdjLinkSize > m_OutboundSizeAry[FromID]);

		int link_entering_time_interval;
		for(t=0; t <m_SimulationHorizon; t+=g_DepartureTimetInterval)
		{
			link_entering_time_interval= t/g_DepartureTimetInterval;
			if(link_entering_time_interval >= m_AssignmentIntervalSize)
				link_entering_time_interval = m_AssignmentIntervalSize-1;

			float AvgTravelTime = g_LinkVector[li]->m_FreeFlowTravelTime ;

			if(g_LinkVector[li]->m_FromNodeNumber == 48 && g_LinkVector[li]->m_ToNodeNumber == 202)
				TRACE("");

			m_LinkTDTimeAry[g_LinkVector[li]->m_LinkID][link_entering_time_interval] = AvgTravelTime;
			m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][link_entering_time_interval]=  AvgTravelTime;

			// use travel time now, should use cost later
		}


	}

	m_LinkSize = g_LinkVector.size();


}


bool DTANetworkForSP::TDLabelCorrecting_DoubleQueue(int origin, int departure_time, int vehicle_type=0, float VOT = -1)   // Pointer to previous node (node)
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

	int FromID, LinkID, ToID;


	float NewTime, NewCost;
	while(!SEList_empty())
	{
		FromID  = SEList_front();
		SEList_pop_front();

		if(debug_flag)
		{
			  TRACE("\nScan from node %d",g_NodeVector[FromID]);
		}

		NodeStatusAry[FromID] = 2;        //scaned

		for(i=0; i<m_OutboundSizeAry[FromID];  i++)  // for each arc (i,j) belong A(j)
		{
			LinkID = m_OutboundLinkAry[FromID][i];
			ToID = m_OutboundNodeAry[FromID][i];

			if(ToID == origin)
				continue;


			if(debug_flag)
			{
					  TRACE("\n   to node %d",g_NodeVector[ToID]);
			}
			// need to check here to make sure  LabelTimeAry[FromID] is feasible.


			int link_entering_time_interval = int(LabelTimeAry[FromID])/g_DepartureTimetInterval;
			if(link_entering_time_interval >= m_AssignmentIntervalSize)  // limit the size
				link_entering_time_interval = m_AssignmentIntervalSize-1;

			if(link_entering_time_interval < 0)  // limit the size
				link_entering_time_interval = 0;

			NewTime	 = LabelTimeAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval];  // time-dependent travel times come from simulator

//  road pricing module
			float toll = 0;
			if(LinkID < g_LinkVector.size()) // physical link, which is always sort first.
			{
			DTALink* pLink= g_LinkVector[LinkID];

			if(pLink->m_FromNodeNumber == 48 && pLink->m_ToNodeNumber == 202)
				TRACE("");

			int PricingType = g_VehicleTypeMap[vehicle_type].pricing_type ;
			
			if(VOT<0)  // no input value, fetch default VOT 
			{
			VOT = g_VehicleTypeMap[vehicle_type].average_VOT ;
			}

			 toll = pLink->GetTollRateInMinByVOT(NewTime,PricingType,VOT);
			 if(toll>0)
				 TRACE("");
			}
// end of road pricing module

			NewCost    = LabelCostAry[FromID] + m_LinkTDCostAry[LinkID][link_entering_time_interval] + toll;       // costs come from time-dependent tolls, VMS, information provisions

			if(NewCost < LabelCostAry[ToID] ) // be careful here: we only compare cost not time
			{
			if(debug_flag)
			{
					       TRACE("\n         UPDATE to %f, link travel time %f", NewCost, m_LinkTDCostAry[LinkID][link_entering_time_interval]);
			}

				if(NewTime > g_SimulationHorizon -1)
					NewTime = float(g_SimulationHorizon-1);

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




void DTANetworkForSP::BuildHistoricalInfoNetwork(int CurZoneID, int CurrentTime, float Perception_error_ratio)  // build the network for shortest path calculation and fetch travel time and cost real-time data from simulator
{
	// example 
	// zones 1, 2, 3
	// CurZone ID = 2
	// add all physical links
	// add incoming links from its own (DES) nodes to CurZone ID = 2 
	// add outgoing links from  other zones 1 and 3 to each (DES) node
	// when finding the shortest path for a vehicle, start with origin zone, back trace to the CurZone as destination zone

	// build a network from the current zone centriod (1 centriod here) to all the other zones' centriods (all the zones)

	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;

	m_PhysicalNodeSize = g_NodeVector.size();

	int IntervalLinkID=0;
	int FromID, ToID;

	int i,t;

	for(i=0; i< m_PhysicalNodeSize + g_ODZoneSize+1; i++)
	{
		m_OutboundSizeAry[i] = 0;
		m_InboundSizeAry[i] =0;
	}

	// add physical links

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		FromID = g_LinkVector[li]->m_FromNodeID;
		ToID   = g_LinkVector[li]->m_ToNodeID;

		m_FromIDAry[g_LinkVector[li]->m_LinkID] = FromID;
		m_ToIDAry[g_LinkVector[li]->m_LinkID]   = ToID;

		//      TRACE("FromID %d -> ToID %d \n", FromID, ToID);
		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = g_LinkVector[li]->m_LinkID ;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = g_LinkVector[li]->m_LinkID ;
		m_InboundSizeAry[ToID] +=1;

		ASSERT(g_AdjLinkSize > m_OutboundSizeAry[FromID]);


			//TRACE("%d -> %d, time %d", g_NodeVector[g_LinkVector[li]->m_FromNodeID], g_NodeVector[g_LinkVector[li]->m_ToNodeID],CurrentTime);
			float AvgTravelTime = g_LinkVector[li]->GetHistoricalTravelTime(CurrentTime);

			float Normal_random_value = g_RNNOF() * Perception_error_ratio*AvgTravelTime;
			
			float travel_time  = AvgTravelTime + Normal_random_value;
			if(travel_time < 0.1f)
				travel_time = 0.1f;
			//TRACE(" %6.3f zone %d \n",AvgTravelTime, CurZoneID);

			m_LinkTDTimeAry[g_LinkVector[li]->m_LinkID][0] = travel_time;
			m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][0]=  travel_time;



	}

	int LinkID = g_LinkVector.size();

		// add outgoing connector from the centriod corresponding to the current origin zone to physical nodes of the current zone
	for(i = 0; i< g_ZoneVector[CurZoneID].m_CentroidNodeAry.size(); i++)
	{
		FromID = m_PhysicalNodeSize; // m_PhysicalNodeSize is the centriod number for CurZoneNo // root node
		ToID = g_ZoneVector[CurZoneID].m_CentroidNodeAry [i];

		//         TRACE("destination node of current zone %d: %d\n",CurZoneID, g_NodeVector[ToID]);

		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = LinkID;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = LinkID ;
		m_InboundSizeAry[ToID] +=1;


		ASSERT(g_AdjLinkSize > m_OutboundSizeAry[FromID]);

		for(int t=0; t <m_AssignmentIntervalSize; t++)
		{
			m_LinkTDTimeAry[LinkID][t] = 0;
			m_LinkTDCostAry[LinkID][t]= 0;
		}

		LinkID++;

	}

	// add incoming connectors from the physicla nodes corresponding to a zone to the non-current zone.
	for(int z= 1; z<= g_ODZoneSize; z++)
	{
		if(z != CurZoneID)
		{
			for(i = 0; i<  g_ZoneVector[z].m_CentroidNodeAry.size(); i++)
			{
				FromID = g_ZoneVector[z].m_CentroidNodeAry [i]; // m_PhysicalNodeSize is the centriod number for CurZoneNo
				ToID =   m_PhysicalNodeSize + z; // m_PhysicalNodeSize is the centriod number for CurZoneNo, note that  .m_ZoneID start from 1

				m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
				m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = LinkID;
				m_OutboundSizeAry[FromID] +=1;

					m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = LinkID ;
					m_InboundSizeAry[ToID] +=1;


				ASSERT(g_AdjLinkSize >  m_OutboundSizeAry[FromID]);

				for( t=0; t <m_AssignmentIntervalSize; t++)
				{
					m_LinkTDTimeAry[LinkID][t] = 0;
					m_LinkTDCostAry[LinkID][t]= 0;
				}

				LinkID++;


			}
		}
	}

	m_NodeSize = m_PhysicalNodeSize + 1 + g_ODZoneSize;


}

void DTANetworkForSP::BuildTravelerInfoNetwork(int CurrentTime, float Perception_error_ratio)  // build the network for shortest path calculation and fetch travel time and cost real-time data from simulator
{

	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;

	int IntervalLinkID=0;
	int FromID, ToID;

	int i;

	// add physical links
	m_PhysicalNodeSize = g_NodeVector.size();

	for(i=0; i< m_PhysicalNodeSize; i++)
	{
		m_OutboundSizeAry[i] = 0;
		m_InboundSizeAry[i] =0;
	}


	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		FromID = g_LinkVector[li]->m_FromNodeID;
		ToID   = g_LinkVector[li]->m_ToNodeID;

		m_FromIDAry[g_LinkVector[li]->m_LinkID] = FromID;
		m_ToIDAry[g_LinkVector[li]->m_LinkID]   = ToID;

		//      TRACE("FromID %d -> ToID %d \n", FromID, ToID);
		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = g_LinkVector[li]->m_LinkID ;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = g_LinkVector[li]->m_LinkID ;
		m_InboundSizeAry[ToID] +=1;

		ASSERT(g_AdjLinkSize > m_OutboundSizeAry[FromID]);


			float AvgTravelTime = g_LinkVector[li]->GetPrevailingTravelTime(CurrentTime);
//			TRACE("\n%d -> %d, time %d, TT: %f", g_NodeVector[g_LinkVector[li]->m_FromNodeID], g_NodeVector[g_LinkVector[li]->m_ToNodeID],CurrentTime,AvgTravelTime);

			float Normal_random_value = g_RNNOF() * Perception_error_ratio*AvgTravelTime;


			
			float travel_time  = AvgTravelTime + Normal_random_value;
			if(travel_time < g_LinkVector[li]->m_FreeFlowTravelTime )
				travel_time = g_LinkVector[li]->m_FreeFlowTravelTime;


			m_LinkTDTimeAry[g_LinkVector[li]->m_LinkID][0] = travel_time;
			m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][0]=  travel_time;

	}


	m_NodeSize = m_PhysicalNodeSize;
}


bool DTANetworkForSP::OptimalTDLabelCorrecting_DoubleQueue(int origin, int departure_time, int destination)
// time -dependent label correcting algorithm with deque implementation
{

	int i;
	int debug_flag = 0;  // set 1 to debug the detail information
	if(debug_flag)
				TRACE("\nCompute shortest path from %d at time %d",origin, departure_time);

    bool bFeasiblePathFlag  = false;

	if(m_OutboundSizeAry[origin]== 0)
		return false;

	for(i=0; i <m_NodeSize; i++) // Initialization for all nodes
	{
		NodeStatusAry[i] = 0;

		for(int t=departure_time; t <m_SimulationHorizon; t+=m_AssignmentIntervalSize)
		{
			TD_LabelCostAry[i][t] = MAX_SPLABEL;
			TD_NodePredAry[i][t] = -1;  // pointer to previous NODE INDEX from the current label at current node and time
			TD_TimePredAry[i][t] = -1;  // pointer to previous TIME INDEX from the current label at current node and time
		}

	}

	//	TD_LabelCostAry[origin][departure_time] = 0;

	// Initialization for origin node at the preferred departure time, at departure time, cost = 0, otherwise, the delay at origin node

	TD_LabelCostAry[origin][departure_time]= 0;

	SEList_clear();
	SEList_push_front(origin);


	while(!SEList_empty())
	{
		int FromID  = SEList_front();
		SEList_pop_front();  // remove current node FromID from the SE list


		NodeStatusAry[FromID] = 2;        //scaned

		//scan all outbound nodes of the current node
		for(i=0; i<m_OutboundSizeAry[FromID];  i++)  // for each arc (i,j) belong A(i)
		{
			int LinkNo = m_OutboundLinkAry[FromID][i];
			int ToID = m_OutboundNodeAry[FromID][i];

			if(ToID == origin)  // remove possible loop back to the origin
				continue;


			if(debug_flag)
				TRACE("\nScan from node %d to node %d",g_NodeVector[FromID].m_NodeName,g_NodeVector[ToID].m_NodeName);

			// for each time step, starting from the departure time
			for(int t=departure_time; t <m_SimulationHorizon; t+=m_AssignmentIntervalSize)
			{
				if(TD_LabelCostAry[FromID][t]<MAX_SPLABEL-1)  // for feasible time-space point only
				{   
					int time_stopped = 0; 
					
						int NewToNodeArrivalTime	 = (int)(t + time_stopped + m_LinkTDTimeAry[LinkNo][t]);  // time-dependent travel times for different train type
						float NewCost  =  TD_LabelCostAry[FromID][t] + m_LinkTDTimeAry[LinkNo][t] + m_LinkTDCostAry[LinkNo][t];
						// costs come from time-dependent resource price or road toll

						if(NewToNodeArrivalTime > (m_SimulationHorizon -1))  // prevent out of bound error
							NewToNodeArrivalTime = (m_SimulationHorizon-1);

						if(NewCost < TD_LabelCostAry[ToID][NewToNodeArrivalTime] ) // we only compare cost at the downstream node ToID at the new arrival time t
						{

							if(ToID == destination)
							bFeasiblePathFlag = true; 


							if(debug_flag)
								TRACE("\n         UPDATE to %f, link cost %f at time %d", NewCost, m_LinkTDCostAry[LinkNo][t],NewToNodeArrivalTime);

							// update cost label and node/time predecessor

							TD_LabelCostAry[ToID][NewToNodeArrivalTime] = NewCost;
							TD_NodePredAry[ToID][NewToNodeArrivalTime] = FromID;  // pointer to previous NODE INDEX from the current label at current node and time
							TD_TimePredAry[ToID][NewToNodeArrivalTime] = t;  // pointer to previous TIME INDEX from the current label at current node and time

							// Dequeue implementation
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

					
					}
				}
				//another condition: in the SELite now: there is no need to put this node to the SEList, since it is already there.
			}

		}      // end of for each link

	}	// end of while


	ASSERT(bFeasiblePathFlag);

	return bFeasiblePathFlag;
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

	for(int t=departure_time; t <m_SimulationHorizon; t+=m_AssignmentIntervalSize)
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
}


int DTANetworkForSP::FindBestPathWithVOT(int origin, int departure_time, int destination, int vehicle_type, float VOT,int PathNodeList[MAX_NODE_SIZE_IN_A_PATH],float &TotalCost)   // Pointer to previous node (node)
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

	int FromID, LinkID, ToID;

	float NewTime, NewCost;
	while(!SEList_empty())
	{
		FromID  = SEList_front();
		SEList_pop_front();

		if(debug_flag)
		{
			  TRACE("\nScan from node %d",g_NodeVector[FromID]);
		}

		NodeStatusAry[FromID] = 2;        //scaned

		for(i=0; i<m_OutboundSizeAry[FromID];  i++)  // for each arc (i,j) belong A(j)
		{
			LinkID = m_OutboundLinkAry[FromID][i];
			ToID = m_OutboundNodeAry[FromID][i];

			if(ToID == origin)
				continue;


			int link_entering_time_interval = int(LabelTimeAry[FromID])/g_DepartureTimetInterval;
			if(link_entering_time_interval >= m_AssignmentIntervalSize)  // limit the size
				link_entering_time_interval = m_AssignmentIntervalSize-1;

			if(link_entering_time_interval < 0)  // limit the size
				link_entering_time_interval = 0;

			NewTime	 = LabelTimeAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval];  // time-dependent travel times come from simulator

//  road pricing module
			float toll = 0;
				if(LinkID < g_LinkVector.size()) // physical link, which is always sort first.
				{
				DTALink* pLink= g_LinkVector[LinkID];

				int PricingType = g_VehicleTypeMap[vehicle_type].pricing_type ;

				 toll = pLink->GetTollRateInMinByVOT(NewTime,PricingType, VOT);
				}

/*			if(g_TollingMethodFlag == 2)  // VMT toll
			{
				if(LinkID < g_LinkVector.size()) // physical link, which is always sort first.
				{
				DTALink* pLink= g_LinkVector[LinkID];
				 toll = pLink->m_Length * g_VMTTollingRate/max(1,VOT)*60;
				}
			}
*/
// end of road pricing module
			

			NewCost    = LabelCostAry[FromID] + m_LinkTDCostAry[LinkID][link_entering_time_interval] + toll;       // costs come from time-dependent tolls, VMS, information provisions

			if(NewCost < LabelCostAry[ToID] ) // be careful here: we only compare cost not time
			{
			if(debug_flag)
			{
					       TRACE("\n         UPDATE to %f, link travel time %f", NewCost, m_LinkTDCostAry[LinkID][link_entering_time_interval]);
			}

				if(NewTime > g_SimulationHorizon -1)
					NewTime = float(g_SimulationHorizon-1);

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

		int NodeSize = 0;
		
		int PredNode = NodePredAry[destination];		
		while(PredNode != origin && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
			{
				ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
				PathNodeList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				PredNode = NodePredAry[PredNode];
			}

		TotalCost = LabelCostAry[destination];

		if(TotalCost > MAX_SPLABEL-10)
		{
			ASSERT(false);
		}

		return NodeSize;
}

