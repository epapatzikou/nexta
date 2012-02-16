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

void DTANetworkForSP::BuildNetworkBasedOnZoneCentriod(int CurZoneID)  // build the network for shortest path calculation and fetch travel time and cost data from simulator
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

	BuildPhysicalNetwork();

	int LinkID = g_LinkVector.size();

	// add outgoing connectors from origin zone center(m_PhysicalNodeSize) to zone centriods
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

		for(int t= m_StartIntervalForShortestPathCalculation; t < m_NumberOfSPCalculationIntervals; t+=1)
		{
			m_LinkTDTimeAry[LinkID][t] = 0;
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

					m_LinkTDDistanceAry[LinkID] = 0;
					
					for( t= m_StartIntervalForShortestPathCalculation; t <m_NumberOfSPCalculationIntervals; t+=1)
				{
					m_LinkTDTimeAry[LinkID][t] = 0;
				}

				LinkID++;
			}
		}
	}
	m_NodeSize = m_PhysicalNodeSize + 1 + g_ODZoneSize;
}

void DTANetworkForSP::BuildPhysicalNetwork()
{

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

		m_LinkTDDistanceAry[g_LinkVector[li]->m_LinkID] = g_LinkVector[li]->m_Length ;

		int link_entering_time_interval;
		for(t = m_StartTimeInMin; t < m_PlanningHorizonInMin; t += g_AggregationTimetInterval)
		{
			link_entering_time_interval= t/g_AggregationTimetInterval;
			if(link_entering_time_interval >= m_NumberOfSPCalculationIntervals)
				link_entering_time_interval = m_NumberOfSPCalculationIntervals-1;

			// we obtain simulated time-dependent travel time measurments from simulator, use that for time-dependent shortest path calculation
			float AvgTravelTime = g_LinkVector[li]->GetTravelTimeByMin (t,g_AggregationTimetInterval);
			ASSERT(AvgTravelTime < 99999);

			m_LinkTDTimeAry[g_LinkVector[li]->m_LinkID][link_entering_time_interval] = AvgTravelTime;
			
			// copy pricing type dependent link toll values
			for(int il = 0; il< g_LinkVector[li]->m_TollSize; il++)
			{
				if(m_StartTimeInMin >= g_LinkVector[li]->pTollVector[il].StartTime && m_StartTimeInMin <= g_LinkVector[li]->pTollVector[il].EndTime)
				{
					m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][link_entering_time_interval].m_bTollExist = true;
					for(int pricing_type = 0; pricing_type < MAX_PRICING_TYPE_SIZE; pricing_type++)
					{
						m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][link_entering_time_interval].TollValue [pricing_type] = g_LinkVector[li]->pTollVector[il].TollRate[pricing_type];
					}
				}
			}

		}
	}

	m_LinkSize = g_LinkVector.size();
}


bool DTANetworkForSP::TDLabelCorrecting_DoubleQueue(int origin, int departure_time, int pricing_type=0, float VOT = 10, bool debug_flag = false)
// time -dependent label correcting algorithm with deque implementation
{

	int i;
	float AdditionalCostInMin = 0;

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

		if(debug_flag && FromID < m_PhysicalNodeSize)  // physical nodes
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

			if(debug_flag && ToID < m_PhysicalNodeSize)  // physical nodes
			{
					  TRACE("\n   to node %d",g_NodeVector[ToID].m_NodeName);
			}
			// need to check here to make sure  LabelTimeAry[FromID] is feasible.


			int link_entering_time_interval = int(LabelTimeAry[FromID])/g_AggregationTimetInterval;
			if(link_entering_time_interval >= m_NumberOfSPCalculationIntervals)  // limit the size
				link_entering_time_interval = m_NumberOfSPCalculationIntervals-1;

			if(link_entering_time_interval < 0)  // limit the size
				link_entering_time_interval = 0;

			NewTime	 = LabelTimeAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval];  // time-dependent travel times come from simulator

			ASSERT(NewTime < 9999999 && NewTime >=-0.00001);

			NewCost    = LabelCostAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval] ;
			
			if(VOT > 0.01 && m_LinkTDCostAry[LinkID][link_entering_time_interval].m_bTollExist) 
			{ // with VOT and toll
				AdditionalCostInMin = m_LinkTDCostAry[LinkID][link_entering_time_interval].TollValue [pricing_type]/VOT * 60.0f;       // 60.0f for 60 min per hour, costs come from time-dependent tolls, VMS, information provisions
				NewCost += AdditionalCostInMin;
			}

			if(NewCost < LabelCostAry[ToID] ) // be careful here: we only compare cost not time
			{
					if(debug_flag && ToID < m_PhysicalNodeSize)  // physical nodes
					{
						TRACE("\n         UPDATE to node %d, cost: %f, link travel time %f", g_NodeVector[ToID].m_NodeName, NewCost, m_LinkTDTimeAry[LinkID][link_entering_time_interval]);
					}

				if(NewTime > g_PlanningHorizon -1)
					NewTime = float(g_PlanningHorizon-1);

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

		for(int  t= m_StartIntervalForShortestPathCalculation; t <m_NumberOfSPCalculationIntervals; t++)
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
		for(int t = m_StartTimeInMin; t < m_PlanningHorizonInMin; t +=g_AggregationTimetInterval)
			{
				if(TD_LabelCostAry[FromID][t]<MAX_SPLABEL-1)  // for feasible time-space point only
				{   
					int time_stopped = 0; 
					
						int NewToNodeArrivalTime	 = (int)(t + time_stopped + m_LinkTDTimeAry[LinkNo][t]);  // time-dependent travel times for different train type
						float NewCost  =  TD_LabelCostAry[FromID][t] + m_LinkTDTimeAry[LinkNo][t] + m_LinkTDCostAry[LinkNo][t].TollValue [0];
						// costs come from time-dependent resource price or road toll

						if(NewToNodeArrivalTime > (m_NumberOfSPCalculationIntervals -1))  // prevent out of bound error
							NewToNodeArrivalTime = (m_NumberOfSPCalculationIntervals-1);

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


int DTANetworkForSP::FindBestPathWithVOT(int origin, int departure_time, int destination, int pricing_type, float VOT,int PathNodeList[MAX_NODE_SIZE_IN_A_PATH],float &TotalCost, bool distance_flag, bool debug_flag)   // Pointer to previous node (node)
// time-dependent label correcting algorithm with deque implementation
{

	int i;
		if(m_OutboundSizeAry[origin]== 0)
		return 0;

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
			  TRACE("\nScan from node %d",g_NodeVector[FromID].m_NodeName);

		NodeStatusAry[FromID] = 2;        //scaned

		for(i=0; i<m_OutboundSizeAry[FromID];  i++)  // for each arc (i,j) belong A(j)
		{
			LinkID = m_OutboundLinkAry[FromID][i];
			ToID = m_OutboundNodeAry[FromID][i];

			if(ToID == origin)
				continue;

			int link_entering_time_interval = int(LabelTimeAry[FromID])/g_AggregationTimetInterval;
			if(link_entering_time_interval >= m_NumberOfSPCalculationIntervals)  // limit the size
				link_entering_time_interval = m_NumberOfSPCalculationIntervals-1;

			if(link_entering_time_interval < 0)  // limit the size
				link_entering_time_interval = 0;

			if(distance_flag)
				NewTime	= LabelTimeAry[FromID] + m_LinkTDDistanceAry[LinkID];
			else // distance
				NewTime	= LabelTimeAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval];  // time-dependent travel times come from simulator

//  road pricing module
			float toll_in_min = 0;
			
			if(VOT > 0.01 && m_LinkTDCostAry[LinkID][link_entering_time_interval].m_bTollExist) 
			{ // with VOT and toll
				toll_in_min = m_LinkTDCostAry[LinkID][link_entering_time_interval].TollValue [pricing_type]/VOT * 60.0f;       // 60.0f for 60 min per hour, costs come from time-dependent tolls, VMS, information provisions

			 if(debug_flag)
			  TRACE("\ntoll in min = %f",toll_in_min);

			}

// end of road pricing module

			if(distance_flag)
				NewCost    = LabelCostAry[FromID] + m_LinkTDDistanceAry[LinkID];  // do not take into account toll value
			else 
				NewCost    = LabelCostAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval] + toll_in_min;       // costs come from time-dependent tolls, VMS, information provisions

			if(NewCost < LabelCostAry[ToID] ) // be careful here: we only compare cost not time
			{
					if(debug_flag)
						TRACE("\n         UPDATE to node %d, cost: %f, link travel time %f", g_NodeVector[ToID].m_NodeName, NewCost, m_LinkTDCostAry[LinkID][link_entering_time_interval]);

				if(NewTime > m_PlanningHorizonInMin -1)
					NewTime = m_PlanningHorizonInMin-1;

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
		temp_reversed_PathNodeList[NodeSize++] = destination;  // we use physical network, so please count the destination as the first node!

		int PredNode = NodePredAry[destination];	

		while(PredNode != origin && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
			{
				ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
				temp_reversed_PathNodeList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				PredNode = NodePredAry[PredNode];
			}

		temp_reversed_PathNodeList[NodeSize++] = origin;

		int j = 0;
		for(i = NodeSize-1; i>=0; i--)
		{
		PathNodeList[j++] = temp_reversed_PathNodeList[i];
		}

		TotalCost = LabelCostAry[destination];

		if(debug_flag)
		{
			for(i = 0; i < NodeSize; i++)
			{
			TRACE("\nPath sequence, no.%d, node %d\n",i, g_NodeVector[PathNodeList[i]].m_NodeName);
			}
			TRACE("\nPath sequence end, cost = ..%f\n",TotalCost);
		}

		if(TotalCost > MAX_SPLABEL-10)
		{
			//ASSERT(false);
			return 0;
		}

		return NodeSize;
}

