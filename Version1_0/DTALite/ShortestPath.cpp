// Portions Copyright 2010 Xuesong Zhou

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
// Time-dependent, shortest-path algorithm for real-time intelligent vehicle highway system applications&quot;, Transportation Research Record 1408 � Ziliaskopoulos, Mahmassani - 1993

#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"

void DTANetworkForSP::BuildNetworkBasedOnZoneCentriod(int DayNo,int CurZoneID)  // build the network for shortest path calculation and fetch travel time and cost data from simulator
{
	// build a network from the current zone centriod (1 centriod here) to all the other zones' centriods (all the zones)
	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;

	m_PhysicalNodeSize = g_NodeVector.size();

	int IntervalLinkID=0;
	int FromID, ToID;

	unsigned int i;
	int t;

	for(i=0; i< m_PhysicalNodeSize + g_ODZoneNumberSize+1; i++)
	{
		m_OutboundSizeAry[i] = 0;
		m_InboundSizeAry[i] =0;
	}

	BuildPhysicalNetwork(DayNo,CurZoneID);

	int LinkID = g_LinkVector.size();

	// add outgoing connectors from origin zone center(m_PhysicalNodeSize) to zone centriods
	for(i = 0; i< g_ZoneMap[CurZoneID].m_OriginActivityVector.size(); i++)
	{
		FromID = m_PhysicalNodeSize; // m_PhysicalNodeSize is the centriod number for CurZoneNo
		ToID = g_ZoneMap[CurZoneID].m_OriginActivityVector[i];
		// add outcoming connector from the centriod corresponding to the current zone: node ID = m_PhysicalNodeSize, to this physical node's ID

		//         TRACE("destination node of current zone %d: %d\n",CurZoneID, g_NodeVector[ToID]);

		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = LinkID;

		m_OutboundConnectorZoneIDAry[FromID][m_OutboundSizeAry[FromID]] = LinkID;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = LinkID ;
		m_InboundSizeAry[ToID] +=1;


		ASSERT(g_AdjLinkSize >= m_OutboundSizeAry[FromID]);

		for(int t= m_StartIntervalForShortestPathCalculation; t < m_NumberOfSPCalculationIntervals; t+=1)
		{
			m_LinkTDTimeAry[LinkID][t] = 0;

			// copy pricing type dependent link toll values

		}


		// construct outbound movement vector
		for(int movement = 0; movement < m_OutboundSizeAry[ToID]; movement++)
		{
			int outbound_link = m_OutboundLinkAry[ToID][movement];

			m_OutboundMovementAry[LinkID][movement] = outbound_link;
			m_OutboundMovementDelayAry[LinkID][movement] = 0;   // we need to add time-dependent movement delay here
			m_OutboundMovementSizeAry[LinkID]++;

		}

		//end of  constructing outbound movement vector
		LinkID++;
	}



	// add incoming connector to destination zone which is not CurZoneNo connector from the centriod corresponding to the current zone: node ID = m_PhysicalNodeSize, to this physical node's ID
	std::map<int, DTAZone>::iterator iterZone;
	for (iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
	{
		DTAZone zone = iterZone->second ;

		if(iterZone->first != CurZoneID)  // only this origin zone has vehicles, then we build the network
		{
			for(i = 0; i< zone.m_DestinationActivityVector .size(); i++)
			{
				FromID = zone.m_DestinationActivityVector[i];; // m_PhysicalNodeSize is the centriod number for CurZoneNo
				ToID =   m_PhysicalNodeSize + iterZone->first ; // m_PhysicalNodeSize is the centriod number for CurZoneNo, note that  .m_ZoneID start from 1

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

				// construct outbound movement vector: no outgoing movement 
				m_OutboundMovementSizeAry[LinkID] = 0;
				LinkID++;
			}
		}
	}
	m_NodeSize = m_PhysicalNodeSize + 1 + g_ODZoneNumberSize;
}

void DTANetworkForSP::BuildPhysicalNetwork(int DayNo, int CurrentZoneNo)  // for agent based 
{

	//CurrentZoneNo >=0: called by zone based assignment 

	bool bDebug = false;

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

	for(i=0; i< m_LinkSize; i++)
	{
		m_OutboundMovementSizeAry[i] = 0;
	}

	// add physical links

	unsigned li;
	for(li = 0; li< g_LinkVector.size(); li++)
	{
		DTALink* pLink = g_LinkVector[li];
		FromID = pLink->m_FromNodeID;
		ToID   = pLink->m_ToNodeID;

			if( pLink->m_FromNodeNumber == 12730 && pLink->m_ToNodeNumber == 12742)
			{
			TRACE("");
			}

		m_FromIDAry[pLink->m_LinkNo] = FromID;
		m_ToIDAry[pLink->m_LinkNo]   = ToID;

		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = pLink->m_LinkNo ;

		int link_id = pLink->m_LinkNo ;
		if(g_LinkTypeMap[g_LinkVector[link_id]->m_link_type].IsConnector())
		{
			m_OutboundConnectorZoneIDAry[FromID][m_OutboundSizeAry[FromID]] = g_NodeVector[g_LinkVector[link_id]->m_FromNodeID ].m_ZoneID ;
			m_OutboundLinkConnectorZoneIDAry[link_id] = g_NodeVector[g_LinkVector[link_id]->m_FromNodeID ].m_ZoneID ;
		}else
		{
			m_OutboundConnectorZoneIDAry[FromID][m_OutboundSizeAry[FromID]]  = -1; // default values
			m_OutboundLinkConnectorZoneIDAry[link_id]  = -1;  // default values
		}

		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = pLink->m_LinkNo  ;
		m_InboundSizeAry[ToID] +=1;

		ASSERT(g_AdjLinkSize > m_OutboundSizeAry[FromID]);

		m_LinkTDDistanceAry[pLink->m_LinkNo] = pLink->m_Length ;

		int link_entering_time_interval;
		for(t = m_StartTimeInMin; t < m_PlanningHorizonInMin; t += g_AggregationTimetInterval)
		{
			link_entering_time_interval= t/g_AggregationTimetInterval;
			if(link_entering_time_interval >= m_NumberOfSPCalculationIntervals)
				link_entering_time_interval = m_NumberOfSPCalculationIntervals-1;

			// we obtain simulated time-dependent travel time measurments from simulator, use that for time-dependent shortest path calculation
			float AvgTripTime = pLink->GetTravelTimeByMin (DayNo,t,g_AggregationTimetInterval);

			if (g_LinkTypeMap[pLink->m_link_type ].IsFreeway () == true)
				AvgTripTime*=g_FreewayBiasFactor;

			if(AvgTripTime < 0.01f)  // to avoid possible loops
				AvgTripTime = 0.01f ;

			ASSERT(AvgTripTime < 99999);

			if(bDebug) 
			{
				TRACE("FromID %d -> ToID %d, time: %d, %f\n", g_NodeVector[FromID].m_NodeNumber, g_NodeVector[ToID].m_NodeNumber, t,AvgTripTime );
			}

			m_LinkTDTimeAry[pLink->m_LinkNo][link_entering_time_interval] = AvgTripTime;


			// copy pricing type dependent link toll values
			for(int itoll = 0; itoll< pLink->TollVector.size(); itoll++)
			{

				if((DayNo >= pLink->TollVector[itoll].StartDayNo && DayNo <= pLink->TollVector[itoll].EndDayNo  ) && m_StartTimeInMin >= pLink->TollVector[itoll].StartTime && m_StartTimeInMin <= pLink->TollVector[itoll].EndTime)
				{
					m_LinkTDCostAry[pLink->m_LinkNo][link_entering_time_interval].m_bTollExist = true;

					float speed =  pLink->m_Length /AvgTripTime*60;


					for(int pricing_type = 1; pricing_type < MAX_PRICING_TYPE_SIZE; pricing_type++)
					{
					//if(speed >=60)
					//{
					//	pLink->TollVector[itoll].TollRate[pricing_type] = pLink->TollVector[itoll].TollRate[pricing_type]-0.25;
					//}
					//if(speed <=45)
					//{
					//	pLink->TollVector[itoll].TollRate[pricing_type] = pLink->TollVector[itoll].TollRate[pricing_type]+0.25;
					//}

					//	if(pLink->TollVector[itoll].TollRate[pricing_type]<=0.25)
					//	{
					//	pLink->TollVector[itoll].TollRate[pricing_type] = 0.25;
					//	}

					//	if(pLink->TollVector[itoll].TollRate[pricing_type]>=2)
					//	{
					//	pLink->TollVector[itoll].TollRate[pricing_type] = 2;
					//	}
						m_LinkTDCostAry[pLink->m_LinkNo][link_entering_time_interval].TollValue [pricing_type] = pLink->TollVector[itoll].TollRate[pricing_type];
					}
				}
			}

			if (g_LinkTypeMap[pLink->m_link_type ].IsTransit() 
				|| g_LinkTypeMap[pLink->m_link_type ].IsWalking() )  // 
			{  // transit or walking link
				m_LinkTDCostAry[pLink->m_LinkNo][link_entering_time_interval].m_bTollExist = true;

				m_LinkTDCostAry[pLink->m_LinkNo][link_entering_time_interval].TollValue [1] = 100;  // transit links do not allow SOV
				m_LinkTDCostAry[pLink->m_LinkNo][link_entering_time_interval].TollValue [2] = 100;  // transit links do not allow HOV
				m_LinkTDCostAry[pLink->m_LinkNo][link_entering_time_interval].TollValue [3] = 100; // transit links do not allow trucks
				m_LinkTDCostAry[pLink->m_LinkNo][link_entering_time_interval].TollValue [4] = 0;  // default zero cost
			}

			if (g_LinkTypeMap[pLink->m_link_type ].IsTransit() ==false)
			{
				if (g_LinkTypeMap[pLink->m_link_type ].IsFreeway () == true)
				{
					m_LinkTDTransitTimeAry[pLink->m_LinkNo][link_entering_time_interval] = 99999;			
				}else
				{
					m_LinkTDTransitTimeAry[pLink->m_LinkNo][link_entering_time_interval] = pLink->m_Length/5*60;  // walking speed  = 5 mph			
				}

			}else
			{
				m_LinkTDTransitTimeAry[pLink->m_LinkNo][link_entering_time_interval] = m_LinkTDTimeAry[pLink->m_LinkNo][link_entering_time_interval];  // calculated from speed limit of bus

			}




		}

	}


	// construct outbound movement vector
	for(li = 0; li< g_LinkVector.size(); li++)
	{
		// if CurrentZoneNo ==- 1, we do not run the following line (to skip zone outgoing connectors)
		if( CurrentZoneNo >=0 && g_LinkTypeMap[g_LinkVector[li]->m_link_type].IsConnector() && g_NodeVector[g_LinkVector[li]->m_FromNodeID ].m_ZoneID != CurrentZoneNo) // only for non connector-links
			continue;

		//find downstream node
		ToID   = g_LinkVector[li]->m_ToNodeID;
		if(g_NodeVector[ToID].m_MovementMap.size()==0)  //without movement capacity input
		{
			for(int movement = 0; movement < m_OutboundSizeAry[ToID]; movement++)
			{
				int outbound_link = m_OutboundLinkAry[ToID][movement];
				m_OutboundMovementAry[li][movement] = outbound_link;
				m_OutboundMovementDelayAry[li][movement] = 0;   // we need to add time-dependent movement delay here
				m_OutboundMovementSizeAry[li]++;
			}

		}else //with movement capacity input	
		{
			for(int movement = 0; movement < m_OutboundSizeAry[ToID]; movement++)
			{
				int outbound_link = m_OutboundLinkAry[ToID][movement];
				m_OutboundMovementAry[li][movement] = outbound_link;

			
				int from_node = g_LinkVector[li]->m_FromNodeNumber ;
				int to_node = g_LinkVector[li]->m_ToNodeNumber ;
				int dest_node =  g_LinkVector[outbound_link]->m_ToNodeNumber ;
				string movement_id = GetMovementStringID(from_node, to_node,dest_node);
				if(g_NodeVector[ToID].m_MovementMap.find(movement_id) != g_NodeVector[ToID].m_MovementMap.end()) // the capacity for this movement has been defined
				{
						DTANodeMovement movement_element = g_NodeVector[ToID].m_MovementMap[movement_id];
						m_OutboundMovementDelayAry[li][movement] = movement_element.GetAvgDelay_In_Min();
				}else
				{
					m_OutboundMovementDelayAry[li][movement] = 0;   // we need to add time-dependent movement delay here
				}

				

				m_OutboundMovementSizeAry[li]++;
			}

		}

			//end of  constructing outbound movement vector
	}

	m_LinkSize = g_LinkVector.size();
}
void DTANetworkForSP::BuildTravelerInfoNetwork(int DayNo,int CurrentTime, int VMSLinkNo, float Perception_error_ratio)  // build the network for shortest path calculation and fetch travel time and cost real-time data from simulator
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

		if(g_LinkTypeMap[g_LinkVector[li]->m_link_type].IsConnector() && g_NodeVector[g_LinkVector[li]->m_FromNodeID ].m_ZoneID >= 0 
			&& g_LinkVector[li]->m_LinkNo != VMSLinkNo) // connector from centroid and not the starting link
			continue;

		FromID = g_LinkVector[li]->m_FromNodeID;
		ToID   = g_LinkVector[li]->m_ToNodeID;

		m_FromIDAry[g_LinkVector[li]->m_LinkNo] = FromID;
		m_ToIDAry[g_LinkVector[li]->m_LinkNo]   = ToID;

		//      TRACE("FromID %d -> ToID %d \n", FromID, ToID);
		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = g_LinkVector[li]->m_LinkNo ;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = g_LinkVector[li]->m_LinkNo ;
		m_InboundSizeAry[ToID] +=1;

		ASSERT(g_AdjLinkSize > m_OutboundSizeAry[FromID]);


		float AvgTripTime = g_LinkVector[li]->GetPrevailingTravelTime(DayNo,CurrentTime);
		//			TRACE("\n%d -> %d, time %d, TT: %f", g_NodeVector[g_LinkVector[li]->m_FromNodeID], g_NodeVector[g_LinkVector[li]->m_ToNodeID],CurrentTime,AvgTripTime);

		float Normal_random_value = g_RNNOF() * Perception_error_ratio*AvgTripTime;

		float travel_time  = AvgTripTime + Normal_random_value;
		if(travel_time < g_LinkVector[li]->m_FreeFlowTravelTime )
			travel_time = g_LinkVector[li]->m_FreeFlowTravelTime;

		m_LinkTDTimeAry[g_LinkVector[li]->m_LinkNo][0] = travel_time;

	}
	m_NodeSize = m_PhysicalNodeSize;
}


bool DTANetworkForSP::TDLabelCorrecting_DoubleQueue(int origin, int departure_time, int pricing_type=1, float VOT = 10, bool distance_cost_flag = false, bool debug_flag = false)
// time -dependent label correcting algorithm with deque implementation
{
	// this is the standard shortest path algorithm
	debug_flag = false;

	int i;
	float AdditionalCostInMin = 0;

	if(m_OutboundSizeAry[origin]== 0)
		return false;

	for(i=0; i <m_NodeSize; i++) // Initialization for all nodes
	{
		NodePredAry[i]  = -1;
		LinkNoAry[i] = -1;

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
			TRACE("\nScan from node %d",g_NodeVector[FromID].m_NodeNumber);
		}

		NodeStatusAry[FromID] = 2;        //scaned

		for(i=0; i<m_OutboundSizeAry[FromID];  i++)  // for each arc (i,j) belong A(j)
		{
			LinkID = m_OutboundLinkAry[FromID][i];
			ToID = m_OutboundNodeAry[FromID][i];

			if(ToID == origin)
				continue;

			if(debug_flag )  // physical nodes
			{
				TRACE("\n   to node %d", ToID);
			}
			// need to check here to make sure  LabelTimeAry[FromID] is feasible.


			int link_entering_time_interval = int(LabelTimeAry[FromID])/g_AggregationTimetInterval;
			if(link_entering_time_interval >= m_NumberOfSPCalculationIntervals)  // limit the size
				link_entering_time_interval = m_NumberOfSPCalculationIntervals-1;

			if(link_entering_time_interval < 0)  // limit the size
				link_entering_time_interval = 0;

			if(distance_cost_flag)
				NewTime	= LabelTimeAry[FromID];
			else // distance
				NewTime	= LabelTimeAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval];  // time-dependent travel times come from simulator

			if(distance_cost_flag)
				NewCost    = LabelCostAry[FromID] + m_LinkTDDistanceAry[LinkID];
			else
				NewCost    = LabelCostAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval] ;

			if(VOT > 0.01 && m_LinkTDCostAry[LinkID][link_entering_time_interval].m_bTollExist) 
			{ // with VOT and toll
				AdditionalCostInMin = m_LinkTDCostAry[LinkID][link_entering_time_interval].TollValue [pricing_type]/VOT * 60.0f;       // 60.0f for 60 min per hour, costs come from time-dependent tolls, VMS, information provisions
				NewCost += AdditionalCostInMin;
			}

			if(NewCost < LabelCostAry[ToID] ) // be careful here: we only compare cost not time
			{
				if(debug_flag )  // physical nodes
				{
					TRACE("\n         UPDATE to node %d, cost: %f, link travel time %f", ToID, NewCost, m_LinkTDTimeAry[LinkID][link_entering_time_interval]);
				}

				if(NewTime > g_PlanningHorizon -1)
					NewTime = float(g_PlanningHorizon-1);

				LabelTimeAry[ToID] = NewTime;
				LabelCostAry[ToID] = NewCost;
				NodePredAry[ToID]   = FromID;
				LinkNoAry[ToID] = LinkID;


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

				//another condition: in the SEList now: there is no need to put this node to the SEList, since it is already there.
			}

		}      // end of for each link

	} // end of while

	return true;
}



int DTANetworkForSP::FindBestPathWithVOT(int origin_zone, int origin, int departure_time, int destination_zone, int destination, int pricing_type, float VOT,int PathLinkList[MAX_NODE_SIZE_IN_A_PATH],float &TotalCost, bool distance_flag, bool debug_flag)   // Pointer to previous node (node)
// time-dependent label correcting algorithm with deque implementation
{

	if(g_ShortestPathWithMovementDelayFlag)
	return FindBestPathWithVOT_Movement(origin_zone, origin, departure_time, destination_zone, destination, pricing_type, VOT, PathLinkList, TotalCost,distance_flag, debug_flag);

	if(pricing_type == 0) // unknown type
		pricing_type = 1; 

	if(origin_zone ==8 && destination_zone==11 && origin==163)
	{
		debug_flag = true;
			TRACE("\nScan from root node %d,",g_NodeVector[origin].m_NodeNumber);
			TRACE("\ndestination node %d,",g_NodeVector[destination].m_NodeNumber);
	}

	// checking boundary condition for departure time changes
	if(departure_time < g_DemandLoadingStartTimeInMin)
		departure_time = g_DemandLoadingStartTimeInMin;

	if(departure_time > g_DemandLoadingEndTimeInMin)
		departure_time = g_DemandLoadingEndTimeInMin;


	int i;
	if(m_OutboundSizeAry[origin]== 0)
		return 0;  // no outgoing link from the origin

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
	float CostUpperBound = MAX_SPLABEL;

	float NewTime, NewCost;
	while(!SEList_empty())
	{
		FromID  = SEList_front();
		SEList_pop_front();


		if(debug_flag)
			TRACE("\nScan from node %d,",g_NodeVector[FromID].m_NodeNumber);

		NodeStatusAry[FromID] = 2;        //scaned

		for(i=0; i< m_OutboundSizeAry[FromID];  i++)  // for each arc (i,j) belong A(i)
		{
			LinkID = m_OutboundLinkAry[FromID][i];
			ToID = m_OutboundNodeAry[FromID][i];

			if(m_OutboundConnectorZoneIDAry[FromID][i] >=1 /* TAZ >=1*/ && (m_OutboundConnectorZoneIDAry[FromID][i]!= origin_zone ) )
			continue;  // special feature 1: skip connectors that do not belong to this origin zone

			if(ToID == origin) // special feature 2: no detour at origin
			continue;

			int link_entering_time_interval = int(LabelTimeAry[FromID])/g_AggregationTimetInterval;
			if(link_entering_time_interval >= m_NumberOfSPCalculationIntervals)  // limit the size
				link_entering_time_interval = m_NumberOfSPCalculationIntervals-1;

			if(link_entering_time_interval < 0)  // limit the size
				link_entering_time_interval = 0;

			if(distance_flag)
				NewTime	= LabelTimeAry[FromID];
			else 
			{// non- distance

				if(pricing_type ==4)  // transit // special feature 3: transit travel time
					NewTime	= LabelTimeAry[FromID] + m_LinkTDTransitTimeAry[LinkID][link_entering_time_interval];  // time-dependent travel times come from simulator
				else  // road users
					NewTime	= LabelTimeAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval];  // time-dependent travel times come from simulator
			}

			//  road pricing module
			float toll_in_min = 0;			// special feature 5: road pricing

			if(VOT > 0.01 && m_LinkTDCostAry[LinkID][link_entering_time_interval].m_bTollExist) 
			{ // with VOT and toll
				toll_in_min = m_LinkTDCostAry[LinkID][link_entering_time_interval].TollValue [pricing_type]/VOT * 60.0f;       // 60.0f for 60 min per hour, costs come from time-dependent tolls, VMS, information provisions

				if(debug_flag)
					TRACE("\ntoll in min = %f",toll_in_min);

			}

			// end of road pricing module
										 // special feature 6: update cost
				if(distance_flag)
					NewCost    = LabelCostAry[FromID] + m_LinkTDDistanceAry[LinkID];  // do not take into account toll value
				else 
				{  // non distance cost
					if(pricing_type ==4)  // transit 
						NewCost    = LabelCostAry[FromID] + m_LinkTDTransitTimeAry[LinkID][link_entering_time_interval] + toll_in_min;       // costs come from time-dependent tolls, VMS, information provisions
					else  // non transit
						NewCost    = LabelCostAry[FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval] + toll_in_min;       // costs come from time-dependent tolls, VMS, information provisions

				}

				if( g_floating_point_value_less_than(NewCost, LabelCostAry[ToID])  && NewCost < CostUpperBound) // special feature 7.1  we only compare cost not time
				{
					if(debug_flag && ( ToID== 9))
						TRACE("\n         UPDATE to node %d, cost: %f, link travel time %f", g_NodeVector[ToID].m_NodeNumber, NewCost, m_LinkTDTimeAry[LinkID][link_entering_time_interval]);

					if(NewTime > m_PlanningHorizonInMin -1)
						NewTime = m_PlanningHorizonInMin-1;

					LabelTimeAry[ToID] = NewTime;
					LabelCostAry[ToID] = NewCost;
					NodePredAry[ToID]   = FromID;
					LinkNoAry[ToID] = LinkID;

					if (ToID == destination) // special feature 7.2: update upper bound cost
					{
						CostUpperBound = LabelCostAry[ToID];
					}

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

					//another condition: in the SEList now: there is no need to put this node to the SEList, since it is already there.
				}

			}      // end of for each link



		} // end of while

		int LinkSize = 0;
		int PredNode = NodePredAry[destination];	
		temp_reversed_PathLinkList[LinkSize++] = LinkNoAry[destination];

		while(PredNode != origin && PredNode!=-1 && LinkSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
		{
			ASSERT(LinkSize< MAX_NODE_SIZE_IN_A_PATH-1);
			temp_reversed_PathLinkList[LinkSize++] = LinkNoAry[PredNode];

			PredNode = NodePredAry[PredNode];

			if(debug_flag)
			{
			TRACE("\nTrace from node %d,",g_NodeVector[PredNode].m_NodeNumber);
			}

		}

		int j = 0;
		for(i = LinkSize-1; i>=0; i--)
		{
			PathLinkList[j++] = temp_reversed_PathLinkList[i];
		}

		TotalCost = LabelCostAry[destination];

		if(debug_flag)
		{
			TRACE("\nnode based: Path sequence end, cost = ..%f\n",TotalCost);
		}

		if(TotalCost > MAX_SPLABEL-10)
		{
			//ASSERT(false);
			return 0;
		}




//		ASSERT(node_size_in_path == LinkSize+1);
//
//		for(int i=0; i< LinkSize; i++)
//		{
////		TRACE("node based: linkId : %d vs %d\n",PathLinkList[i], tempPathLinkList[i]);
//		ASSERT(PathLinkList[i]==tempPathLinkList[i]);
//		}

		return LinkSize+1; // as NodeSize
	}


	int DTANetworkForSP::FindBestPathWithVOT_Movement(int origin_zone, int origin, int departure_time,  int destination_zone, int destination, int pricing_type, float VOT,int PathLinkList[MAX_NODE_SIZE_IN_A_PATH],float &TotalCost, bool distance_flag, bool debug_flag=false)
		// time -dependent label correcting algorithm with deque implementation
	{

	// checking boundary condition for departure time changes
	if(departure_time < g_DemandLoadingStartTimeInMin)
		departure_time = g_DemandLoadingStartTimeInMin;

	if(departure_time > g_DemandLoadingEndTimeInMin)
		departure_time = g_DemandLoadingEndTimeInMin;

		int i;
		float AdditionalCostInMin = 0;

		if(m_OutboundSizeAry[origin]== 0)
			return false;

		for(i=0; i <m_LinkSize; i++) // Initialization for all links
		{
			LinkPredAry[i]  = -1;
			LinkStatusAry[i] = 0;

			LinkLabelTimeAry[i] = MAX_SPLABEL;
			LinkLabelCostAry[i] = MAX_SPLABEL;
		}

		// Initialization for origin node: for all outgoing links from origin node

		m_ScanLinkList.clear();
		for(i=0; i< m_OutboundSizeAry[origin];i++)
		{
			int LinkID = m_OutboundLinkAry[origin][i];

			int link_entering_time_interval = int(departure_time)/g_AggregationTimetInterval;
			LinkLabelTimeAry[LinkID] =  departure_time + m_LinkTDTimeAry[LinkID][link_entering_time_interval];
			LinkLabelCostAry[LinkID] =  departure_time + m_LinkTDTimeAry[LinkID][link_entering_time_interval];
			m_ScanLinkList.push_back (LinkID);

			if(m_ToIDAry[LinkID] == destination)  //reach destination on the first link
			{
				PathLinkList[0]= LinkID;
				return 2; // 2 nodes
			}
		}


		int FromLinkID, ToLinkID, NodeID;
		float CostUpperBound = MAX_SPLABEL;

		float NewTime, NewCost;
		while(m_ScanLinkList.size()>0)
		{
			FromLinkID  = m_ScanLinkList.front ();
			m_ScanLinkList.pop_front ();

			if(debug_flag) 
			{
				TRACE("\nScan from link %d",FromLinkID);
			}

			LinkStatusAry[FromLinkID] = 2;        //scaned

			for(i=0; i<m_OutboundMovementSizeAry[FromLinkID];  i++)  // for each arc (i,j) belong to A(i)
			{
				ToLinkID = m_OutboundMovementAry[FromLinkID][i];

				if(debug_flag )  // physical nodes
				{
					TRACE("\n   to link %d, downstream node %d ", ToLinkID,m_ToIDAry[ToLinkID]);
				}
				// need to check here to make sure  LabelTimeAry[FromID] is feasible.

			if(m_OutboundLinkConnectorZoneIDAry[ToLinkID] >=1 /* TAZ >=1*/ && (m_OutboundLinkConnectorZoneIDAry[ToLinkID]!= origin_zone ) )
			continue;  // special feature 1: skip connectors that do not belong to this origin zone

			if(m_ToIDAry[ToLinkID] == origin) // special feature 2: no detour at origin
			continue;


				int link_entering_time_interval = int(LinkLabelTimeAry[FromLinkID])/g_AggregationTimetInterval;
				if(link_entering_time_interval >= m_NumberOfSPCalculationIntervals)  // limit the size
					link_entering_time_interval = m_NumberOfSPCalculationIntervals-1;

				if(link_entering_time_interval < 0)  // limit the size
					link_entering_time_interval = 0;

//  original code				NewTime	= LinkLabelTimeAry[FromLinkID] + m_LinkTDTimeAry[ToLinkID][link_entering_time_interval] + m_OutboundMovementDelayAry[FromLinkID][i];  // time-dependent travel times come from simulator

			if(distance_flag)
				NewTime	= LabelTimeAry[FromLinkID];
			else 
			{// non- distance

				if(pricing_type ==4)  // transit // special feature 3: transit travel time
					NewTime	= LinkLabelTimeAry[FromLinkID] + m_LinkTDTransitTimeAry[ToLinkID][link_entering_time_interval];  // time-dependent travel times come from simulator
				else  // road users
					NewTime	= LinkLabelTimeAry[FromLinkID] + m_LinkTDTimeAry[ToLinkID][link_entering_time_interval] + m_OutboundMovementDelayAry[FromLinkID][i];  // time-dependent travel times come from simulator
			}



// original code			NewCost    = LinkLabelCostAry[FromLinkID] + m_LinkTDTimeAry[ToLinkID][link_entering_time_interval] + m_OutboundMovementDelayAry[FromLinkID][i];


			//  road pricing module
			float toll_in_min = 0;			// special feature 5: road pricing
			if(VOT > 0.01 && m_LinkTDCostAry[ToLinkID][link_entering_time_interval].m_bTollExist)	
			{ // with VOT and toll
					toll_in_min = m_LinkTDCostAry[ToLinkID][link_entering_time_interval].TollValue [pricing_type]/VOT * 60.0f;       // 60.0f for 60 min per hour, costs come from time-dependent tolls, VMS, information provisions
			}

										 // special feature 6: update cost
				if(distance_flag)
					NewCost    = LinkLabelCostAry[FromLinkID] + m_LinkTDDistanceAry[ToLinkID];  // do not take into account toll value
				else 
				{  // non distance cost
					if(pricing_type ==4)  // transit 
						NewCost    = LinkLabelCostAry[FromLinkID] + m_LinkTDTransitTimeAry[ToLinkID][link_entering_time_interval] + m_OutboundMovementDelayAry[FromLinkID][i] + toll_in_min;       // costs come from time-dependent tolls, VMS, information provisions
					else  // non transit
						NewCost    = LinkLabelCostAry[FromLinkID] + m_LinkTDTimeAry[ToLinkID][link_entering_time_interval] + m_OutboundMovementDelayAry[FromLinkID][i] + toll_in_min;       // costs come from time-dependent tolls, VMS, information provisions

				}


				if(NewCost < LinkLabelCostAry[ToLinkID] &&  NewCost < CostUpperBound ) // special feature 7.1  we only compare cost not time
				{
					if(debug_flag )  // physical nodes
					{
						TRACE("\n         UPDATE to link %d, downstream node %d, cost: %f, link travel time %f", ToLinkID, m_ToIDAry[ToLinkID],NewCost, m_LinkTDTimeAry[ToLinkID][link_entering_time_interval]);
					}

					if(NewTime > g_PlanningHorizon -1)
						NewTime = float(g_PlanningHorizon-1);

					LinkLabelTimeAry[ToLinkID] = NewTime;
					LinkLabelCostAry[ToLinkID] = NewCost;
					LinkPredAry[ToLinkID]   = FromLinkID;

					if (m_ToIDAry[ToLinkID] == destination) // special feature 7.2  : update upper bound cost
					{
						CostUpperBound = LinkLabelCostAry[ToLinkID];
					}

					// Dequeue implementation
					//
					if(LinkStatusAry[ToLinkID]==2) // in the SEList_TD before
					{
						m_ScanLinkList.push_back (ToLinkID);
						LinkStatusAry[ToLinkID] = 1;
					}
					if(LinkStatusAry[ToLinkID]==0)  // not be reached
					{
						m_ScanLinkList.push_back (ToLinkID);
						LinkStatusAry[ToLinkID] = 1;
					}

					//another condition: in the SEList now: there is no need to put this node to the SEList, since it is already there.
				}

			}      // end of for each link

		} // end of while


		// post processing: if destination >=0: physical node

		//step 1: find the incoming link has the lowerest cost to the destinatin node

		float min_cost = MAX_SPLABEL;
		int link_id_with_min_cost = -1;
		for(i =0; i< m_InboundSizeAry[destination]; i++)
		{ 
			int incoming_link = m_InboundLinkAry[destination][i];
			if(LinkLabelCostAry[incoming_link] < min_cost && LinkPredAry[incoming_link] >=0)
			{
				min_cost = LinkLabelCostAry[incoming_link];
				link_id_with_min_cost = incoming_link;
			}
		}

		ASSERT(link_id_with_min_cost >=0);

		//step 2 trace the incoming link to the first link in origin node

		int LinkSize = 0;
		temp_reversed_PathLinkList[LinkSize++] = link_id_with_min_cost;  // last link to destination
		int PrevLinkID = LinkPredAry[link_id_with_min_cost];	

		if(PrevLinkID==-1)
		{
		TRACE("Error!");
		
		}
		temp_reversed_PathLinkList[LinkSize++] = PrevLinkID;   //second last link

		while(PrevLinkID!=-1 && LinkSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
		{
			ASSERT(LinkSize< MAX_NODE_SIZE_IN_A_PATH-1);

			if( LinkPredAry[PrevLinkID]!= -1)
			{
				temp_reversed_PathLinkList[LinkSize++] = LinkPredAry[PrevLinkID];
			}
			PrevLinkID = LinkPredAry[PrevLinkID];
		}

		int j = 0;
		for(i = LinkSize-1; i>=0; i--)
		{
			PathLinkList[j++] = temp_reversed_PathLinkList[i];
			if(debug_flag )  
			{
				TRACE("\n  link no. %d, %d, %d ->%d",i, temp_reversed_PathLinkList[i]
				,m_FromIDAry[temp_reversed_PathLinkList[i]], m_ToIDAry[temp_reversed_PathLinkList[i]]);
			}

		}

		if(debug_flag)
		{
			TRACE("\nPath sequence end, cost = ..%f\n",min_cost);
		}

		return LinkSize+1;  // as node size

	}