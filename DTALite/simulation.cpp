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

//Simulation

#include "stdafx.h"
#include "math.h"
#include <algorithm>
#include <functional>
#include <omp.h>

#include "DTALite.h"
#include "GlobalData.h"
#include "SafetyPlanning.h"

extern bool g_VehicularSimulation_BasedOnADCurves(int DayNo, double CurrentTime, int simulation_time_interval_no, int TrafficFlowModelFlag);
extern bool g_VehicularCarFollowingSimulation(int DayNo, double CurrentTime, int simulation_time_interval_no, int TrafficFlowModelFlag);
extern  int NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
extern ofstream g_LogFile;

long g_precision_constant=100000L;
extern float g_DemandGlobalMultiplier;
bool g_floating_point_value_less_than_or_eq_comparison(double value1, double value2)
{
	long lValue1 = (long) (value1*g_precision_constant);
	long lValue2 = (long) (value2*g_precision_constant);

	if(lValue1<=lValue2)
		return true;
	else 
		return false;

}

bool g_Compare_Vehicle_Item(struc_vehicle_item item1, struc_vehicle_item item2)
{
	if(item1.time_stamp < item2.time_stamp)  // item1 is earlier than item2
		return true;
	else 
		return false;
}

bool g_VehicularSimulation(int DayNo, double CurrentTime, int simulation_time_interval_no, int TrafficFlowModelFlag)
{
	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;
	std::map<int, DTAVehicle*>::iterator iterVM;
	int   PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};

	int time_stamp_in_min = int(CurrentTime+0.0001);

	std::list<struc_vehicle_item>::iterator vii;

	int vehicle_id_trace = 8297;
	int link_id_trace = 31;
	bool debug_flag = false ;

	//	TRACE("st=%d\n", simulation_time_interval_no);
	//DTALite:
	// vertical queue data structure
	// each link  ExitQueue, EntranceQueue: (VehicleID, ReadyTime)

	// step 0: /// update VMS path provision

	if(simulation_time_interval_no%(g_information_updating_interval_of_VMS_in_min*10)==0)  //regenerate shortest path tree for VMS every g_information_updating_interval_of_VMS_in_min 
	{
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			for(unsigned int m = 0; m< g_LinkVector[li]->MessageSignVector.size(); m++)
			{

				if(g_LinkVector[li]->MessageSignVector[m].StartDayNo  <= DayNo &&
					DayNo <= g_LinkVector[li]->MessageSignVector[m].EndDayNo &&
					CurrentTime >= g_LinkVector[li]->MessageSignVector[m].StartTime && 
					CurrentTime <= g_LinkVector[li]->MessageSignVector[m].EndTime)
				{
					//calculate shortest path
					DTANetworkForSP network(g_NodeVector.size(), g_LinkVector.size(), 1,g_AdjLinkSize);
					float COV_perception_erorr = g_VMSPerceptionErrorRatio;
					network.BuildTravelerInfoNetwork(DayNo,CurrentTime, g_LinkVector[li]->m_LinkID , COV_perception_erorr);
					network.TDLabelCorrecting_DoubleQueue(g_LinkVector[li]->m_ToNodeID ,CurrentTime,1,10,false, false );

					TRACE("\nVMS %d -> %d",g_LinkVector[li]->m_FromNodeNumber , g_LinkVector[li]->m_ToNodeNumber );
					// update node predecessor
					g_LinkVector[li]->MessageSignVector[m].Initialize(g_NodeVector.size(), network.NodePredAry,network.LinkNoAry );

				}
		 }
		}

	}
	// load vehicle into network



	// step 1: scan all the vehicles, if a vehicle's start time >= CurrentTime, and there is available space in the first link,
	// load this vehicle into the ready queue

	// comment: we use map here as the g_VehicleMap map is sorted by departure time.
	// At each iteration, we start  the last loaded id, and exit if the departure time of a vehicle is later than the current time.

	for (iterVM = g_VehicleMap.find(g_LastLoadedVehicleID); iterVM != g_VehicleMap.end(); iterVM++)
	{
		DTAVehicle* pVeh = iterVM->second;
		if(pVeh->m_bLoaded == false && g_floating_point_value_less_than_or_eq_comparison(pVeh->m_DepartureTime, CurrentTime))  // not being loaded
		{
			if(pVeh->m_NodeSize >=2)  // with physical path
			{
			int FirstLink =pVeh->m_aryVN[0].LinkID;

			DTALink* p_link = g_LinkVector[FirstLink];

			if(p_link !=NULL)
			{
				struc_vehicle_item vi;
				vi.veh_id = pVeh->m_VehicleID ;
				g_LastLoadedVehicleID = pVeh->m_VehicleID ;

				vi.time_stamp = pVeh->m_DepartureTime + p_link->GetFreeMovingTravelTime(TrafficFlowModelFlag, CurrentTime);  // unit: min
				pVeh->m_bLoaded = true;
				pVeh->m_SimLinkSequenceNo = 0;

				if(debug_flag && vi.veh_id == vehicle_id_trace )
					TRACE("Step 1: Load vhc %d to link %d with departure time time %5.2f -> %5.2f\n",vi.veh_id,FirstLink,pVeh->m_DepartureTime,vi.time_stamp);

				p_link->LoadingBuffer.push_back(vi);  // need to fine-tune
				g_Number_of_GeneratedVehicles +=1;
				g_NetworkMOEAry[time_stamp_in_min].Flow_in_a_min +=1;
				g_NetworkMOEAry[time_stamp_in_min].CumulativeInFlow = g_Number_of_GeneratedVehicles;

				// access VMS information from the first link
				int IS_id  = p_link->GetInformationResponseID(DayNo,CurrentTime);
				if(IS_id >= 0)
				{
					if( g_VehicleMap[vi.veh_id]->GetRandomRatio()*100 < p_link->MessageSignVector [IS_id].ResponsePercentage )
					{  // vehicle rerouting

						g_VehicleRerouting(DayNo,vi.veh_id,CurrentTime, &(p_link->MessageSignVector [IS_id]));

						if(g_VehicleVector[vi.veh_id]->m_InformationClass  == 1) /* historical info */
							g_VehicleVector[vi.veh_id]->m_InformationClass=5; /* historical info, resposive to VMS */

						if(g_VehicleVector[vi.veh_id]->m_InformationClass  == 2) /* pre-trip info */
							g_VehicleVector[vi.veh_id]->m_InformationClass=7; /* pre-trip info, nonresposive to VMS */

					}else
					{
						if(g_VehicleVector[vi.veh_id]->m_InformationClass  == 1) /* historical info */
							g_VehicleVector[vi.veh_id]->m_InformationClass=4; /* historical info, non-resposive to VMS */

						if(g_VehicleVector[vi.veh_id]->m_InformationClass  == 2) /* pre-trip info */
							g_VehicleVector[vi.veh_id]->m_InformationClass=6; /* pre-trip info, non-resposive to VMS */

					}

				}
				}

			}  // with physical path
			else
			{	//without physical path: skip
				g_LastLoadedVehicleID = pVeh->m_VehicleID ;
			}


		}

		if(g_floating_point_value_less_than_or_eq_comparison(pVeh->m_DepartureTime,CurrentTime) == false)
		{
			break;
		}


	}

	// loading buffer

	int link_size = g_LinkVector.size();
	for(unsigned li = 0; li< link_size; li++)
	{
		while(g_LinkVector[li]->LoadingBuffer.size() >0 && g_LinkVector[li]->GetNumLanes(DayNo,CurrentTime)>0.01)  // no load vehicle into a blocked link
		{
			struc_vehicle_item vi = g_LinkVector[li]->LoadingBuffer.front();
			// we change the time stamp here to reflect the actual loading time into the network, especially for blocked link
			if( vi.time_stamp < CurrentTime)
				vi.time_stamp = CurrentTime;

			if(debug_flag && vi.veh_id == vehicle_id_trace )
				TRACE("Step 1: Time %f: Load vhc %d from buffer to physical link %d->%d\n",CurrentTime,vi.veh_id,g_LinkVector[li]->m_FromNodeNumber,g_LinkVector[li]->m_ToNodeNumber);

			int NumberOfVehiclesOnThisLinkAtCurrentTime = (int)( g_LinkVector[li]->EntranceQueue.size() + g_LinkVector[li]->ExitQueue.size());

			// determine link in capacity 
			float AvailableSpaceCapacity = g_LinkVector[li]->m_VehicleSpaceCapacity - NumberOfVehiclesOnThisLinkAtCurrentTime;

			// if we use BPR function, no density constraint is imposed --> TrafficFlowModelFlag == 0
			if(TrafficFlowModelFlag==0 || AvailableSpaceCapacity >= max(2,g_LinkVector[li]->m_VehicleSpaceCapacity*(1-g_MaxDensityRatioForVehicleLoading)))  // at least 10% remaining capacity or 2 vehicle space is left
			{
				g_LinkVector[li]->LoadingBuffer.pop_front ();
				g_LinkVector[li]->EntranceQueue.push_back(vi);

				g_LinkVector[li]->CFlowArrivalCount +=1;


				int pricing_type = g_VehicleMap[vi.veh_id]->m_PricingType ;
				g_LinkVector[li]->CFlowArrivalCount_PricingType[pricing_type] +=1;
				g_LinkVector[li]->CFlowArrivalRevenue_PricingType[pricing_type] += g_LinkVector[li]->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);

				DTAVehicle* pVehicle  = g_VehicleMap[vi.veh_id];

				// add cumulative flow count to vehicle

				pVehicle->m_TollDollarCost += g_LinkVector[li]->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);

				if(g_LinkVector[li]->CFlowArrivalCount !=  (g_LinkVector[li]->CFlowArrivalCount_PricingType[1]+ g_LinkVector[li]->CFlowArrivalCount_PricingType[2] + g_LinkVector[li]->CFlowArrivalCount_PricingType[3]))
				{
					TRACE("error!");
				}


				if(debug_flag && vi.veh_id == vehicle_id_trace )
					TRACE("Step 1: Time %f: Capacity available, remove vhc %d from buffer to physical link %d->%d\n",CurrentTime,vi.veh_id,g_LinkVector[li]->m_FromNodeNumber,g_LinkVector[li]->m_ToNodeNumber);

			}else
			{
				break;  // physical road is too congested, wait for next time interval to load
			}

		}
	}

	// step 2: move vehicles from EntranceQueue To ExitQueue, if ReadyTime <= CurrentTime)

#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{
		if(debug_flag && link_id_trace == li && CurrentTime >=484)
		{
			TRACE("Step 3: Time %f, Link: %d -> %d: entrance queue length: %d, exit queue length %d \n",
				CurrentTime, g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName , g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName,
				g_LinkVector[li]->EntranceQueue.size(), g_LinkVector[li]->ExitQueue.size());
		}

		while(g_LinkVector[li]->EntranceQueue.size() >0)
		{

			struc_vehicle_item vi = g_LinkVector[li]->EntranceQueue.front();
			double PlannedArrivalTime = vi.time_stamp;
			if( g_floating_point_value_less_than_or_eq_comparison(PlannedArrivalTime, CurrentTime) )  // link arrival time within the simulation time interval
			{
				g_LinkVector[li]->EntranceQueue.pop_front ();
				g_LinkVector[li]->ExitQueue.push_back(vi);

				if(debug_flag && vi.veh_id == vehicle_id_trace )
				{
					TRACE("Step 2: Time %f: Vhc %d moves from entrance queue to exit queue on link %d->%d\n",PlannedArrivalTime,vi.veh_id,g_LinkVector[li]->m_FromNodeNumber,g_LinkVector[li]->m_ToNodeNumber);
					//					link_id_trace = li;
				}

			}else
			{
				break;  // the vehicle's actual arrival time is later than the current time, so we exit from the loop, stop searching
			}

		}
	}

	// step 3: determine link in and out capacity

	if(TrafficFlowModelFlag ==0) // BPR function 
	{
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			// under BPR function, we do not impose the physical capacity constraint but use a BPR link travel time to move vehicles along links	
			g_LinkVector[li]->LinkOutCapacity = 99999;
			g_LinkVector[li]->LinkInCapacity = 99999;

		}
	}
	else // queueing model
	{


#pragma omp parallel for
		for(int li = 0; li< link_size; li++)
		{
			float PerHourCapacity = g_LinkVector[li]->GetHourlyPerLaneCapacity(CurrentTime);  // static capacity from BRP function
			float PerHourCapacityAtCurrentSimulatioInterval = PerHourCapacity;

			// freeway 

			if(g_LinkTypeMap [g_LinkVector[li]->m_link_type].IsFreeway())  
			{
				if(g_StochasticCapacityMode &&  g_LinkVector[li]->m_StochaticCapcityFlag >=1 && simulation_time_interval_no%150 ==0)  // update stochastic capacity every 15 min
				{
					bool QueueFlag = false;

					if(g_LinkVector[li]->ExitQueue.size() > 0)  // vertical exit queue
						QueueFlag = true;

					PerHourCapacityAtCurrentSimulatioInterval = GetStochasticCapacity(QueueFlag,g_LinkVector[li]->GetHourlyPerLaneCapacity(CurrentTime));
				}
			}

			/* comment out for now: Xuesong: avoid conflict for g_LinkTypeMap
			//Highway/Expressway,			 Principal arteria				Major arterial					 Frontage road
			if(!(g_LinkTypeMap[g_LinkVector[li]->m_link_type ].IsFreeway() || g_LinkTypeMap[g_LinkVector[li]->m_link_type ].IsRamp()))
			{
			PerHourCapacityAtCurrentSimulatioInterval = GetDynamicCapacityAtSignalizedIntersection(PerHourCapacity, g_CycleLength_in_seconds,CurrentTime);
			}
			*/

			// determine link out capacity 
			float MaximumFlowRate = PerHourCapacityAtCurrentSimulatioInterval *g_DTASimulationInterval/60.0f*g_LinkVector[li]->GetNumLanes(DayNo,CurrentTime); //60 --> cap per min --> unit # of vehicle per simulation interval

			float Capacity = MaximumFlowRate;
			// use integer number of vehicles as unit of capacity
			g_LinkVector[li]-> LinkOutCapacity = g_GetRandomInteger_From_FloatingPointValue(Capacity);

			int NumberOfVehiclesOnThisLinkAtCurrentTime = (int)(g_LinkVector[li]->CFlowArrivalCount - g_LinkVector[li]->CFlowDepartureCount);

			float fLinkInCapacity = 99999.0;

			// TrafficFlowModelFlag == 1 -> point queue model
			if(TrafficFlowModelFlag >=2)  // apply spatial link in capacity for spatial queue models( with spillback and shockwave) 
			{
				// determine link in capacity 
				float AvailableSpaceCapacity = g_LinkVector[li]->m_VehicleSpaceCapacity - NumberOfVehiclesOnThisLinkAtCurrentTime;
				fLinkInCapacity = min (AvailableSpaceCapacity, MaximumFlowRate); 
				//			TRACE(" time %5.2f, SC: %5.2f, MFR %5.2f\n",CurrentTime, AvailableSpaceCapacity, MaximumFlowRate);

				// the inflow capcaity is the minimum of (1) incoming maximum flow rate (determined by the number of lanes) and (2) available space capacty  on the link.
				// use integer number of vehicles as unit of capacity

				if(TrafficFlowModelFlag ==3 && g_LinkTypeMap[g_LinkVector[li]->m_link_type] .IsFreeway())  // Newell's model on freeway only
				{
					if(simulation_time_interval_no >=g_LinkVector[li]->m_BackwardWaveTimeInSimulationInterval ) /// we only apply backward wave checking after the simulation time is later than the backward wave speed interval
					{
						if(debug_flag && link_id_trace == li && CurrentTime >=480)
						{
							TRACE("Step 3: Time %f, Link: %d -> %d: tracing backward wave\n",
								CurrentTime, g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName , g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName,
								g_LinkVector[li]->EntranceQueue.size(), g_LinkVector[li]->ExitQueue.size());
						}

						int t_residual_minus_backwardwaveTime = max(0,simulation_time_interval_no - g_LinkVector[li]->m_BackwardWaveTimeInSimulationInterval)% MAX_TIME_INTERVAL_ADCURVE ;

						float VehCountUnderJamDensity = g_LinkVector[li]->m_Length * g_LinkVector[li]->GetNumLanes(DayNo,CurrentTime) *g_LinkVector[li]->m_KJam;
						// when there is a capacity reduction, in the above model, we assume the space capacity is also reduced proportional to the out flow discharge capacity, 
						// for example, if the out flow capacity is reduced from 5 lanes to 1 lanes, say 10K vehicles per hour to 2K vehicles per hour, 
						// our model will also reduce the # of vehicles can be stored on the incident site by the equivalent 4 lanes. 
						// this might cause the dramatic speed change on the incident site, while the upstream link (with the original space capacity) will take more time to propapate the speed change compared to the incident link. 
						// to do list: we should add another parameter of space capacity reduction ratio to represent the fact that the space capacity reduction magnitude is different from the outflow capacity reduction level,
						// particularly for road construction areas on long links, with smooth barrier on the merging section. 
						int N_Arrival_Now_Constrainted = (int)(g_LinkVector[li]->m_CumuDeparturelFlow[t_residual_minus_backwardwaveTime] + VehCountUnderJamDensity);  //g_LinkVector[li]->m_Length 's unit is mile
						int t_residual_minus_1 = max(0, simulation_time_interval_no - 1)% MAX_TIME_INTERVAL_ADCURVE ;

						int N_Now_minus_1 = (int)g_LinkVector[li]->m_CumuArrivalFlow[t_residual_minus_1];
						int Flow_allowed = N_Arrival_Now_Constrainted - N_Now_minus_1;
						//				TRACE("\ntime %d, D:%d,A%d",simulation_time_interval_no,g_LinkVector[li]->m_CumuDeparturelFlow[t_residual_minus_1],g_LinkVector[li]->m_CumuArrivalFlow[t_residual_minus_1]);

						if(Flow_allowed < 0)
							Flow_allowed  = 0;

						if( fLinkInCapacity  > Flow_allowed)
						{
							fLinkInCapacity = Flow_allowed;

							if(Flow_allowed == 0 && N_Arrival_Now_Constrainted > 0)
							{
								if ( g_LinkVector[li]->m_JamTimeStamp > CurrentTime)
									g_LinkVector[li]->m_JamTimeStamp = CurrentTime;

								//							g_LogFile << "Queue spillback"<< CurrentTime <<  g_NodeVector[g_LinkVector[li]->m_FromNodeID] << " -> " <<	g_NodeVector[g_LinkVector[li]->m_ToNodeID] << " " << g_LinkVector[li]->LinkInCapacity << endl;

								// update traffic state
								g_LinkVector[li]->m_LinkMOEAry[(int)(CurrentTime)].TrafficStateCode = 2;  // 2: fully congested

								//							TRACE("Queue spillback at %d -> %d\n", g_NodeVector[g_LinkVector[li]->m_FromNodeID], g_NodeVector[g_LinkVector[li]->m_ToNodeID]);
							}
						}
					}
				}


				float InflowRate = MaximumFlowRate *g_MinimumInFlowRatio;

				if(fLinkInCapacity < InflowRate)  // minimum inflow capacity to keep the network flowing
				{
					fLinkInCapacity = InflowRate; 
				}

			}

			// finally we convert the floating-point capacity to integral capacity in terms of number of vehicles
			g_LinkVector[li]-> LinkInCapacity= g_GetRandomInteger_From_FloatingPointValue(fLinkInCapacity);

			if(debug_flag && link_id_trace == li )
			{
				TRACE("Step 3: Time %f, Link: %d -> %d: Incapacity %d, %f, OutCapacity: %d\n", CurrentTime, g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName , g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName ,g_LinkVector[li]-> LinkInCapacity, fLinkInCapacity, g_LinkVector[li]-> LinkOutCapacity);
			}
		}


		// distribute link in capacity to different incoming links
		for(unsigned li = 0; li< link_size; li++)
		{
			unsigned int il;
			if(g_LinkVector[li] -> m_bMergeFlag >= 1)
			{
				int TotalInFlowCount = 0;
				for( il = 0; il< g_LinkVector[li]->MergeIncomingLinkVector.size(); il++)
				{
					TotalInFlowCount+= g_LinkVector [ g_LinkVector[li]->MergeIncomingLinkVector[il].m_LinkID]->ExitQueue.size();  // count vehiciles waiting at exit queue

				}

				if(TotalInFlowCount > g_LinkVector[li]-> LinkInCapacity)  // demand > supply
				{

					if(g_LinkVector[li] -> m_bMergeFlag == 1)  // merge with mulitple freeway/onramp links only
					{

						if(g_MergeNodeModelFlag==0)
						{
							// distribute capacity according to number of lanes, defined previously.
						}

						if(g_MergeNodeModelFlag==1)
							// distribute capacity according to number of incoming flow
						{
							for( il = 0; il< g_LinkVector[li]->MergeIncomingLinkVector.size(); il++)
							{
								g_LinkVector[li]->MergeIncomingLinkVector[il].m_LinkInCapacityRatio = g_LinkVector [ g_LinkVector[li]->MergeIncomingLinkVector[il].m_LinkID]->ExitQueue.size()*1.0f/TotalInFlowCount ;
							}
						}

						for( il = 0; il< g_LinkVector[li]->MergeIncomingLinkVector.size(); il++)
						{
							float LinkOutCapacity = g_LinkVector[li]-> LinkInCapacity * g_LinkVector[li]->MergeIncomingLinkVector[il].m_LinkInCapacityRatio;

							int LinkOutCapacity_int= g_GetRandomInteger_From_FloatingPointValue(LinkOutCapacity);
							g_LinkVector [g_LinkVector[li]->MergeIncomingLinkVector[il].m_LinkID]->LinkOutCapacity = LinkOutCapacity_int;

						}
					}
					if(g_LinkVector[li] -> m_bMergeFlag == 2)  // merge with onramp
					{
						// step a. check with flow on onramp
						float MaxMergeCapacity = g_LinkVector [ g_LinkVector[li]->m_MergeOnrampLinkID ]->GetHourlyPerLaneCapacity(CurrentTime)*g_DTASimulationInterval/60.0f*g_LinkVector [ g_LinkVector[li]->m_MergeOnrampLinkID ]->GetNumLanes(DayNo,CurrentTime) * 0.5f; //60 --> cap per min --> unit # of vehicle per simulation interval
						// 0.5f -> half of the onramp capacity
						// use integer number of vehicles as unit of capacity

						unsigned int MaxMergeCapacity_int= g_GetRandomInteger_From_FloatingPointValue(MaxMergeCapacity);

						unsigned int FlowonOnRamp = g_LinkVector [ g_LinkVector[li]->m_MergeOnrampLinkID ]->ExitQueue.size();
						int DownstreamLinkInCapacity = g_LinkVector[li]->LinkInCapacity ;

						if(FlowonOnRamp > MaxMergeCapacity_int)  
							// ramp flow > max merge capacity on ramp  
							// over regions I and II in Dr. Rouphail's diagram
							// capacity on ramp = max merge capacity on ramp
							// capacity on main line = LinkInCapacity - capacity on ramp
							// if flow on main line > capacity on main line  // queue on main line
							// elsewise, no queue on mainline
						{
							g_LinkVector [ g_LinkVector[li]->m_MergeOnrampLinkID ]->LinkOutCapacity = MaxMergeCapacity_int;
							g_LinkVector [g_LinkVector[li]->m_MergeMainlineLinkID] ->LinkOutCapacity = DownstreamLinkInCapacity - MaxMergeCapacity_int;

						}else // ramp flow <= max merge capacity on ramp  // region III  
							// restrict the mainly capacity  // mainly capacity = LinkInCapacity - flow on ramp
						{
							g_LinkVector [ g_LinkVector[li]->m_MergeOnrampLinkID ]->LinkOutCapacity = MaxMergeCapacity_int;
							g_LinkVector [g_LinkVector[li]->m_MergeMainlineLinkID]->LinkOutCapacity = DownstreamLinkInCapacity - FlowonOnRamp;

						}
						//		g_LogFile << "merge: mainline capacity"<< CurrentTime << " "  << g_LinkVector [g_LinkVector[li]->m_MergeMainlineLinkID] ->LinkOutCapacity << endl;

					}
				}
			}
		}
	}


	// step 4: move vehicles from ExitQueue to next link's EntranceQueue, if there is available capacity
	// NewTime = ReadyTime + FFTT(next link)


	int NextLink;
	DTALink* p_Nextlink;

	//for each node, we scan each incoming link in a randomly sequence, based on simulation_time_interval_no

	int node_size = g_NodeVector.size();
	for(unsigned node = 0; node < node_size; node++)
	{
		int IncomingLinkSize = g_NodeVector[node].m_IncomingLinkVector.size();
		int incoming_link_count = 0;

		int incoming_link_sequence = simulation_time_interval_no%max(1,IncomingLinkSize);  // random start point

		while(incoming_link_count < IncomingLinkSize)
		{

			int li = g_NodeVector[node].m_IncomingLinkVector[incoming_link_sequence];


			if(debug_flag && (g_LinkVector[li]->m_FromNodeNumber  == 10 && g_LinkVector[li]->m_ToNodeNumber == 5) && CurrentTime >=480)
			{
				TRACE("Step 3: Time %f, Link: %d -> %d: tracing \n", CurrentTime, g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName , g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName);
			}

			// vehicle_out_count is the minimum of LinkOutCapacity and ExitQueue Size

			int vehicle_out_count = g_LinkVector[li]->LinkOutCapacity;

			//			g_LogFile << "link out capaity:"<< CurrentTime << " "  << g_NodeVector[g_LinkVector[li]->m_FromNodeID] << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID]<<" Cap:" << vehicle_out_count<< "queue:" << g_LinkVector[li]->ExitQueue.size() << endl;

			if(g_LinkVector[li]->ExitQueue.size() <= g_LinkVector[li]-> LinkOutCapacity )
			{      // under capacity, constrained by existing queue
				vehicle_out_count = g_LinkVector[li]->ExitQueue.size();
			}

			while(g_LinkVector[li]->ExitQueue.size() >0 && vehicle_out_count >0)
			{
				struc_vehicle_item vi = g_LinkVector[li]->ExitQueue.front();

				int vehicle_id = vi.veh_id;

				// record arrival time at the downstream node of current link
				int link_sequence_no = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo;

				int t_link_arrival_time=0;
				if(link_sequence_no >=1)
				{
					t_link_arrival_time= int(g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no-1].AbsArrivalTimeOnDSN);
				}else
				{
					t_link_arrival_time = int(g_VehicleMap[vehicle_id]->m_DepartureTime);
				}
				// update cumultive lane based flow count
				g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no].LaneBasedCumulativeFlowCount =  g_LinkVector[li]->CFlowArrivalCount  / g_LinkVector[li]->m_NumLanes ;

				// not reach destination yet
				int number_of_links = g_VehicleMap[vehicle_id]->m_NodeSize-1;
				if(g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo < number_of_links-1)  // not reach destination yet
				{

					// advance to next link

					if(vehicle_id == vehicle_id_trace)
						TRACE("simulation link sequence no. %d",g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo);

					NextLink = g_VehicleMap[vehicle_id]->m_aryVN[g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo+1].LinkID;
					p_Nextlink = g_LinkVector[NextLink];

					if(p_Nextlink==NULL)
					{
						TRACE("Error at vehicle %d,",vehicle_id);
						ASSERT(p_Nextlink!=NULL);
					}

					if(p_Nextlink->LinkInCapacity > 0) // if there is available spatial capacity on next link, then move to next link, otherwise, stay on the current link
					{
						float ArrivalTimeOnDSN = 0;
						if(g_floating_point_value_less_than_or_eq_comparison(CurrentTime-g_DTASimulationInterval,vi.time_stamp)) 
							// arrival at previous interval
						{  // no delay 
							ArrivalTimeOnDSN = vi.time_stamp;
						}else
						{  // delayed at previous time interval, discharge at CurrentTime 
							ArrivalTimeOnDSN = CurrentTime; 
						}


						float TimeOnNextLink = 0;

						// update statistics for traveled link
						g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN = ArrivalTimeOnDSN;
						float TravelTime = 0;

						if(link_sequence_no >=1)
						{
							TravelTime= g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN -
								g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no-1].AbsArrivalTimeOnDSN;
						}else
						{
							TravelTime= g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN -
								g_VehicleMap[vehicle_id]->m_DepartureTime ;

						}
						g_VehicleMap[vehicle_id]->m_Delay += (TravelTime-g_LinkVector[li]->m_FreeFlowTravelTime);

						// finally move to next link
						g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo+1;

						// access VMS information here! p_Nextlink is now the current link
						int IS_id  = p_Nextlink->GetInformationResponseID(DayNo,CurrentTime);
						if(IS_id >= 0)
						{
							if( g_VehicleMap[vehicle_id]->GetRandomRatio()*100 < p_Nextlink->MessageSignVector [IS_id].ResponsePercentage )
							{  // vehicle rerouting

								g_VehicleRerouting(DayNo,vehicle_id,CurrentTime, &(p_Nextlink->MessageSignVector [IS_id]));

								if(g_VehicleVector[vehicle_id]->m_InformationClass  == 1) /* historical info */
									g_VehicleVector[vehicle_id]->m_InformationClass=5; /* historical info, resposive to VMS */

								if(g_VehicleVector[vehicle_id]->m_InformationClass  == 2) /* pre-trip info */
									g_VehicleVector[vehicle_id]->m_InformationClass=7; /* pre-trip info, resposive to VMS */

							}

						}



						vi.veh_id = vehicle_id;

						vi.time_stamp = ArrivalTimeOnDSN + p_Nextlink->GetFreeMovingTravelTime(TrafficFlowModelFlag, CurrentTime);

						// remark: when - TimeOnNextLink < 0, it means there are few seconds of left over on current link, which should be spent on next link

						p_Nextlink->EntranceQueue.push_back(vi);
						p_Nextlink->CFlowArrivalCount +=1;
						g_LinkVector[li]->CFlowDepartureCount +=1;

						int pricing_type = g_VehicleMap[vi.veh_id]->m_PricingType ;
						p_Nextlink->CFlowArrivalCount_PricingType[pricing_type] +=1;
						p_Nextlink->CFlowArrivalRevenue_PricingType[pricing_type] += p_Nextlink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);

						DTAVehicle* pVehicle = g_VehicleMap[vi.veh_id];

						pVehicle->m_TollDollarCost += p_Nextlink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);


						if(p_Nextlink->CFlowArrivalCount !=  (p_Nextlink->CFlowArrivalCount_PricingType[1]+ p_Nextlink->CFlowArrivalCount_PricingType[2] + p_Nextlink->CFlowArrivalCount_PricingType[3]))
						{
							//						TRACE("error!");
						}

						if(t_link_arrival_time < g_LinkVector[li]->m_LinkMOEAry.size())
						{
							g_LinkVector[li]->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime  += TravelTime;
							g_LinkVector[li]->m_LinkMOEAry[t_link_arrival_time].TotalFlowCount +=1;
						}


						g_LinkVector[li]-> departure_count +=1;
						g_LinkVector[li]-> total_departure_based_travel_time += TravelTime;

						if(debug_flag && vi.veh_id == vehicle_id_trace )
						{
							TRACE("Step 4: target vehicle: link arrival time %d, total travel time on current link %d->%d, %f\n",t_link_arrival_time, g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName, g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName, g_LinkVector[li]->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime);
						}

						//										TRACE("time %d, total travel time %f\n",t_link_arrival_time, g_LinkVector[li]->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime);

						p_Nextlink->LinkInCapacity -=1; // reduce available space capacity by 1

						g_LinkVector[li]->ExitQueue.pop_front();



					}else
					{
						// no capcity, do nothing, and stay in the vertical exit queue

						if(TrafficFlowModelFlag == 4)  // spatial queue with shock wave and FIFO principle: FIFO, then there is no capacity for one movement, the following vehicles cannot move even there are capacity
						{
							break;
						}

					}

					//			TRACE("move veh %d from link %d link %d %d\n",vehicle_id,g_LinkVector[li]->m_LinkNo, p_Nextlink->m_LinkNo);


				}else
				{

					// reach destination, increase the counter.
					float ArrivalTimeOnDSN = vi.time_stamp;  // no delay at destination node

					// update statistics for traveled link
					int link_sequence_no = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo;
					g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN = ArrivalTimeOnDSN;
					float TravelTime = 0;

					if(link_sequence_no >=1)
					{
						TravelTime= g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN -
							g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no-1].AbsArrivalTimeOnDSN;
					}else
					{
						TravelTime= g_VehicleMap[vehicle_id]->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN -
							g_VehicleMap[vehicle_id]->m_DepartureTime ;

					}
					g_VehicleMap[vehicle_id]->m_Delay += (TravelTime-g_LinkVector[li]->m_FreeFlowTravelTime);


					// finally move to next link
					g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo+1;

					g_Number_of_CompletedVehicles +=1;

					g_NetworkMOEAry[time_stamp_in_min].CumulativeOutFlow = g_Number_of_CompletedVehicles;

					g_VehicleMap[vehicle_id]->m_ArrivalTime = ArrivalTimeOnDSN;

					g_VehicleMap[vehicle_id]->m_TripTime = g_VehicleMap[vehicle_id]->m_ArrivalTime - g_VehicleMap[vehicle_id]->m_DepartureTime;

					if(debug_flag && vi.veh_id == vehicle_id_trace )
					{
						TRACE("Step 4: time %f, target vehicle reaches the destination, vehicle trip time: %f, # of links  = %d\n",
							ArrivalTimeOnDSN, g_VehicleMap[vehicle_id]->m_TripTime , 
							g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo);

						vehicle_id_trace = -1; // not tracking anymore 
					}

					g_VehicleMap[vehicle_id]->m_bComplete = true;
					int OriginDepartureTime = (int)(g_VehicleMap[vehicle_id]->m_DepartureTime );
					g_NetworkMOEAry[OriginDepartureTime].AbsArrivalTimeOnDSN_in_a_min +=g_VehicleMap[vehicle_id]->m_TripTime;

					g_LinkVector[li]->CFlowDepartureCount +=1;
					g_LinkVector[li]->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime  += TravelTime;
					g_LinkVector[li]->m_LinkMOEAry[t_link_arrival_time].TotalFlowCount += 1;

					g_LinkVector[li]->ExitQueue.pop_front();

				}

				vehicle_out_count--;

			}
			incoming_link_count++;
			incoming_link_sequence = (incoming_link_sequence+1) % IncomingLinkSize;  // increase incoming_link_sequence by 1 within IncomingLinkSize
		}  // for each incoming link
	} // for each node

	//	step 5: statistics collection
#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{

		// cummulative flow counts

		int t_residual = simulation_time_interval_no % MAX_TIME_INTERVAL_ADCURVE;

		g_LinkVector[li]->m_CumuArrivalFlow[t_residual] = g_LinkVector[li]->CFlowArrivalCount;
		g_LinkVector[li]->m_CumuDeparturelFlow[t_residual] = g_LinkVector[li]->CFlowDepartureCount;

		if(simulation_time_interval_no%g_number_of_intervals_per_min==0 )  // per min statistics
		{
			g_LinkVector[li]->VehicleCount = g_LinkVector[li]->CFlowArrivalCount - g_LinkVector[li]->CFlowDepartureCount;

			// queue is the number of vehicles at the end of simulation interval

			g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].ExitQueueLength = g_LinkVector[li]->ExitQueue.size(); 

			if(g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].ExitQueueLength >=1 && g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].TrafficStateCode != 2)   // not fully congested
				g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].TrafficStateCode  = 1;  // partially congested

			// time_stamp_in_min+1 is because we take the stastistics to next time stamp
			g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount =  g_LinkVector[li]->CFlowArrivalCount;

			// toll collection 

			for(int pt = 1; pt < MAX_PRICING_TYPE_SIZE; pt++)
			{
				g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount_PricingType[pt] = g_LinkVector[li]->CFlowArrivalCount_PricingType[pt];
				g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].CumulativeRevenue_PricingType[pt] = g_LinkVector[li]->CFlowArrivalRevenue_PricingType[pt];
			}

			g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].CumulativeDepartureCount = g_LinkVector[li]->CFlowDepartureCount;

			if(debug_flag && link_id_trace == li && vehicle_id_trace >= 0 )
			{
				TRACE("step 5: statistics collection: Time %d, link %d -> %d, Cumulative arrival count %d, cumulative departure count %d \n", time_stamp_in_min, g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName, g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName,
					g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount, g_LinkVector[li]->m_LinkMOEAry [time_stamp_in_min].CumulativeDepartureCount);
			}


		}
	}
	return true;
}

void g_AssignPathsForInformationUsers(int DayNo, double time, int simulation_time_interval_no)
{

	// find the shortst path for pre-trip and real time information users
	//percentage of hist info vehicles is given
	int PathLinkList[MAX_CPU_SIZE][MAX_NODE_SIZE_IN_A_PATH];  // existing links
	int PathNodeList[MAX_CPU_SIZE][MAX_NODE_SIZE_IN_A_PATH];  // new nodes

	vector<DTAVehicle*>::iterator vIte;
	int VSize = g_VehicleVector.size();

	DTANetworkForSP* pNetwork[MAX_CPU_SIZE];

	int thread; 
	for(thread = 0; thread < MAX_CPU_SIZE; thread++)
	{
		pNetwork[thread] = NULL;
	}
	//				cout <<g_GetAppRunningTime()<<  "Calculating real-time info paths..." << endl;

#pragma omp parallel for 
	for (int v= 0; v<VSize;v++)
	{
		// only fetch the path every few min when it is time to retrieve information
		if(( g_VehicleVector[v]->m_InformationClass ==2 || g_VehicleVector[v]->m_InformationClass ==3)
			&& g_VehicleVector[v]->m_bComplete==false &&
			g_VehicleVector[v]->m_TimeToRetrieveInfo == simulation_time_interval_no )
		{
			int	id = omp_get_thread_num( );  // starting from 0
			//						cout <<g_GetAppRunningTime()<<  "Core " << id << " is calculating paths for vehicle "<<  v  << endl;

			int	cid = omp_get_thread_num( );  // starting from 0

			int CurrentLinkID = g_VehicleVector[v]->m_aryVN[g_VehicleVector[v]->m_SimLinkSequenceNo].LinkID;
			int	CurrentNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

			int LastLinkID = g_VehicleVector[v]->m_aryVN[g_VehicleVector[v]->m_NodeSize-2].LinkID;  // m_NodeSize-1 is # of links, m_NodeSize-2 is no. of link sequence
			int	LastNodeID = g_LinkVector[LastLinkID]->m_ToNodeID;

			int l;
			// copy exsting links up to the current link
			for(l=0; l<= g_VehicleVector[v]->m_SimLinkSequenceNo; l++)
			{
				PathLinkList[cid][l] = g_VehicleVector[v]->m_aryVN[l].LinkID;
			}

			if(pNetwork[cid] == NULL)
			{
				pNetwork[cid] = new DTANetworkForSP(g_NodeVector.size(), g_LinkVector.size(), 1,g_AdjLinkSize);  //  network instance for single processor in multi-thread environment
			}

			float COV_perception_erorr = g_UserClassPerceptionErrorRatio[g_VehicleVector[v]->m_InformationClass];
			pNetwork[cid]->BuildTravelerInfoNetwork(DayNo,time, CurrentLinkID, COV_perception_erorr);
			pNetwork[cid]->TDLabelCorrecting_DoubleQueue(CurrentNodeID,time,g_VehicleVector[v]->m_DemandType,g_VehicleVector[v]->m_VOT,false, false );

			// find shortest path
			int SubPathNodeSize = 0;
			int PredNode = LastNodeID; 

			// scan backward in the predessor array of the shortest path calculation results
			while(PredNode !=  CurrentNodeID && PredNode!=-1 && SubPathNodeSize< MAX_NODE_SIZE_IN_A_PATH)
			{
				ASSERT(SubPathNodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
				PathNodeList[cid][SubPathNodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				PredNode = pNetwork[cid]->NodePredAry[PredNode];
			}

			// add current node to find the links/paths
			PathNodeList[cid][SubPathNodeSize++] = CurrentNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.

			int i;
			for(i = 0; i< SubPathNodeSize-1; i++)
			{
				// l is used before, continue counting
				PathLinkList[cid][l++] = pNetwork[cid]->GetLinkNoByNodeIndex(PathNodeList[cid][SubPathNodeSize-i-1], PathNodeList[cid][SubPathNodeSize-i-2]);
			}


			// update paths
			if( g_VehicleVector[v]->m_aryVN !=NULL)
			{
				delete g_VehicleVector[v]->m_aryVN;
			}

			g_VehicleVector[v]->m_NodeSize = l+1;
			g_VehicleVector[v]->m_aryVN = new SVehicleLink[g_VehicleVector[v]->m_NodeSize];

			if(g_VehicleVector[v]->m_aryVN==NULL)
			{
				cout << "Insufficient memory for allocating vehicle arrays!";
				g_ProgramStop();

			}

			// copy existing link sequence back to the new link no array
			for(i=0; i<l; i++)
			{
				g_VehicleVector[v]->m_aryVN[i].LinkID = PathLinkList[cid][i];
			}

			if(g_VehicleVector[v]->m_InformationClass  == 2) /* pre-trip info */
				g_VehicleVector[v]->m_TimeToRetrieveInfo += 99999;
			if(g_VehicleVector[v]->m_InformationClass  == 3) /* enroute info */
				g_VehicleVector[v]->m_TimeToRetrieveInfo += g_information_updating_interval_of_en_route_info_travelers_in_min*10;  // got information 10 min later

		}
	}

	for(thread = 0; thread < MAX_CPU_SIZE; thread++)
	{
		if(pNetwork[thread] != NULL)
			delete pNetwork[thread];
	}

}

NetworkLoadingOutput g_NetworkLoading(int TrafficFlowModelFlag=2, int SimulationMode = 0, int Iteration = 1)  // default spatial queue // SimulationMode= default 0: UE;  1: simulation from demand; 2: simulation from vehicle file
{
	NetworkLoadingOutput output;
	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;
	g_Number_of_CompletedVehicles = 0;
	g_Number_of_GeneratedVehicles = 0;

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		DTALink* pLink = g_LinkVector[li];
		pLink->SetupMOE();

		pLink->m_FFTT_simulation_interval = int(pLink->m_FreeFlowTravelTime/g_DTASimulationInterval);

		if(TrafficFlowModelFlag==1)
		{
		if(pLink->m_FFTT_simulation_interval >= MAX_TIME_INTERVAL_ADCURVE)
		{
			cout << "Error: link" << pLink->m_FromNodeNumber << "->" <<  pLink->m_ToNodeNumber << " has FFTT_simulation_interval = " << pLink->m_FFTT_simulation_interval << ">= MAX_TIME_INTERVAL_ADCURVE " << MAX_TIME_INTERVAL_ADCURVE << endl;
			g_ProgramStop();
		}
		}

	}

	cout << "Free global path set... " << endl;

	g_ODTKPathVector.clear();   
	// clear the global path set vector, will be regenerated in OD path flow adjustment module ConstructPathArrayForEachODT_ODEstimation()

	if(TrafficFlowModelFlag == 0) // using BPR function
	{
		//Calculate BPRLinkVolume based on previous vehicle paths
		for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
		{
			if((*vIte)->m_NodeSize >=2)
			{
				for(int i = 0; i< (*vIte)->m_NodeSize-1; i++)
				{
					g_LinkVector[(*vIte)->m_aryVN[i].LinkID]->m_BPRLinkVolume++;
				}
			}
		}

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			g_LinkVector[li]->m_BPRLinkTravelTime = g_LinkVector[li]->m_FreeFlowTravelTime*(1.0f+0.15f*(powf(g_LinkVector[li]->m_BPRLinkVolume/(max(1,g_LinkVector[li]->m_BPRLaneCapacity*g_LinkVector[li]->GetNumLanes())),4.0f)));

			float maximum_travel_time_ratio = 10;
			if(g_LinkVector[li]->m_BPRLinkTravelTime > g_LinkVector[li]->m_FreeFlowTravelTime * maximum_travel_time_ratio )
			{  // if BRP travel time is extremely large, 10 times slower than the speed limit, then enforce a minimum moving speed to move the vehicles outof the network....
				g_LinkVector[li]->m_BPRLinkTravelTime = g_LinkVector[li]->m_FreeFlowTravelTime * maximum_travel_time_ratio;
				g_LogFile << "Reset extreme large BPR travel time :" << g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName << "travel time:" << g_LinkVector[li]->m_BPRLinkTravelTime  << ", org raito: " << g_LinkVector[li]->m_BPRLinkTravelTime/g_LinkVector[li]->m_FreeFlowTravelTime  << endl;

			}
			g_LogFile << "BPR Travel Time for Link "<< g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName  << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName <<" Flow:" << g_LinkVector[li]->m_BPRLinkVolume << "travel time:" << g_LinkVector[li]->m_BPRLinkTravelTime  << endl;

		}
	}



	g_Number_of_CompletedVehicles = 0;
	g_Number_of_GeneratedVehicles = 0;

	int Number_of_CompletedVehicles = 0;
	g_LastLoadedVehicleID = 0;
	for (std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin(); iterVehicle != g_VehicleVector.end(); iterVehicle++)
	{
		DTAVehicle* pVeh = (*iterVehicle);
		pVeh->PreTripReset();

		if(pVeh->m_NodeSize >=2)  // has feasible path
		{
			int FirstLink =pVeh->m_aryVN[0].LinkID;

			DTALink* pLink = g_LinkVector[FirstLink];
			pLink ->LoadingBufferSize ++;
		}
		// load into loading buffer
	}
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		DTALink* pLink = g_LinkVector[li];

		if(pLink ->LoadingBufferVector!=NULL)
			delete pLink ->LoadingBufferVector;

		if(pLink ->LoadingBufferSize >=1)
		{
			pLink ->LoadingBufferVector = new int[pLink ->LoadingBufferSize];
			pLink ->LoadingBufferSize = 0; // reset
		}
	}

	// assign vehicles to loading buffer
	for (std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin(); iterVehicle != g_VehicleVector.end(); iterVehicle++)
	{
		DTAVehicle* pVeh = (*iterVehicle);
		pVeh->PreTripReset();

		if(pVeh->m_NodeSize >=2)  // has feasible path
		{
			int FirstLink =pVeh->m_aryVN[0].LinkID;

			DTALink* pLink = g_LinkVector[FirstLink];
			pLink ->LoadingBufferVector [pLink ->LoadingBufferSize ++] = pVeh->m_VehicleID ;
		}
		// load into loading buffer
	}


	cout << "start simulation process..."  << endl;

	double time;
	int simulation_time_interval_no = 0;
	bool bPrintOut = true;

	// generate historical info based shortst path, based on constant link travel time


	for(time = g_DemandLoadingStartTimeInMin ; time< g_PlanningHorizon; simulation_time_interval_no++)  // the simulation time clock is advanced by 0.1 seconds
	{
		time= g_DemandLoadingStartTimeInMin+ simulation_time_interval_no*g_DTASimulationInterval;

	//if(TrafficFlowModelFlag ==1) // point queue
	//	g_VehicularSimulation_BasedOnADCurves(Iteration,time, simulation_time_interval_no, TrafficFlowModelFlag);
	//else // , spatial queue and Newell's model
		g_VehicularSimulation(Iteration,time, simulation_time_interval_no, TrafficFlowModelFlag);

		if(bPrintOut && g_Number_of_GeneratedVehicles > 0 && g_Number_of_CompletedVehicles == g_Number_of_GeneratedVehicles)
		{		
			cout << "--Simulation completes as all the vehicles are out of the network.--" << endl;
			bPrintOut = false;
			break;  // exit from the simulation process
		}
		if(simulation_time_interval_no%50 == 0 && bPrintOut) // every 5 min
		{
			cout << "simu clock: " << int(time) << " min, # of veh -- Generated: "<< g_Number_of_GeneratedVehicles << ", In network: "<<g_Number_of_GeneratedVehicles-g_Number_of_CompletedVehicles << endl;
			g_LogFile << "simulation clock in min:" << int(time) << ", # of vehicles  -- Generated: "<< g_Number_of_GeneratedVehicles << ", In network: "<<g_Number_of_GeneratedVehicles-g_Number_of_CompletedVehicles << endl;
		}
	}


	//if(TrafficFlowModelFlag ==4)  // Simplified car following model
	//{

	//	int NumberOfTimeStepsInCarfollowingSimulationBuffer = 300; // 3 seconds for the car following simulation time window

	//	// start allocating vehicle distance array
	//for(unsigned li = 0; li< g_LinkVector.size(); li++)
	//{
	//	DTALink* pLink = g_LinkVector[li];
	//	pLink->VechileDistanceAry = AllocateDynamicArray<float>(pLink->FIFO_queue_acutal_max_size+10,NumberOfTimeStepsInCarfollowingSimulationBuffer);

	//	pLink->CurrentSequenceNoForVechileDistanceAry = 0;
	//	pLink->CycleSizeForVechileDistanceAry = pLink->FIFO_queue_acutal_max_size + 5;

	//}


	//	for(time = g_DemandLoadingStartTimeInMin ; time< g_PlanningHorizon; simulation_time_interval_no++)  // the simulation time clock is advanced by 0.1 seconds
	//{
	//	time= g_DemandLoadingStartTimeInMin+ simulation_time_interval_no*g_CarFollowingSimulationInterval ;

	//	//		g_VehicularSimulation(Iteration,time, simulation_time_interval_no, TrafficFlowModelFlag);
	//	g_VehicularCarFollowingSimulation(Iteration,time, simulation_time_interval_no, TrafficFlowModelFlag);

	//	if(bPrintOut && g_Number_of_GeneratedVehicles > 0 && g_Number_of_CompletedVehicles == g_Number_of_GeneratedVehicles)
	//	{		
	//		cout << "--Simulation completes as all the vehicles are out of the network.--" << endl;
	//		bPrintOut = false;
	//		break;  // exit from the simulation process
	//	}
	//	if(simulation_time_interval_no%(600*5) == 0 && bPrintOut) // every 5 min
	//	{
	//		cout << "simu clock:" << int(time) << " min, # of veh -- Generated: "<< g_Number_of_GeneratedVehicles << ", In network: "<<g_Number_of_GeneratedVehicles-g_Number_of_CompletedVehicles << endl;
	//		g_LogFile << "simulation clock in min:" << int(time) << ", # of vehicles  -- Generated: "<< g_Number_of_GeneratedVehicles << ", In network: "<<g_Number_of_GeneratedVehicles-g_Number_of_CompletedVehicles << endl;
	//	}
	//}
	//
	//// deallocation
	//for(unsigned li = 0; li< g_LinkVector.size(); li++)
	//{
	//	DTALink* pLink = g_LinkVector[li];
	//	DeallocateDynamicArray(pLink->VechileDistanceAry,pLink->FIFO_queue_acutal_max_size+10,NumberOfTimeStepsInCarfollowingSimulationBuffer);
	//}
	//
	//}


	//	TRACE("g_Number_of_CompletedVehicles= %d\n", Number_of_CompletedVehicles);


	// end of dynamic traffic flow model


	if(g_ODEstimationFlag) 
	{
	// generate EndTimeOfPartialCongestion
	// before this step, we already generate the state of each time stamp, in terms of free-flow, congestion and queue spillk back case. 
	for(unsigned li = 0; li< g_LinkVector.size(); li++)  // for each link
	{

		int NextCongestionTransitionTimeStamp = g_PlanningHorizon+10;  // // start with the initial value, no queue

		if(g_LinkVector[li]->m_LinkMOEAry[g_PlanningHorizon-1].ExitQueueLength>=1)  // remaining queue at the end of simulation horizon
			NextCongestionTransitionTimeStamp = g_PlanningHorizon-1;

		int time_min;
		for(time_min = g_PlanningHorizon-1; time_min>= g_DemandLoadingStartTimeInMin ; time_min--)  // move backward
		{
			// transition condition 1: from partial congestion to free-flow; action: move to the next lini
			if(time_min>=1 && g_LinkVector[li]->m_LinkMOEAry[time_min-1].ExitQueueLength>=1 && g_LinkVector[li]->m_LinkMOEAry[time_min].ExitQueueLength==0)  // previous time_min interval has queue, current time_min interval has no queue --> end of congestion 
			{
				NextCongestionTransitionTimeStamp = time_min;			
			}

			//transition condition 2: from partial congestion to fully congesed. action: --> move to the previous link
			if(time_min>=1 && g_LinkVector[li]->m_LinkMOEAry[time_min-1].ExitQueueLength>=1 && g_LinkVector[li]->m_LinkMOEAry[time_min].TrafficStateCode == 2 )
			{
				NextCongestionTransitionTimeStamp = time_min;			
			}


			if(g_LinkVector[li]->m_LinkMOEAry[time_min].ExitQueueLength > 0 ) // there is queue at time time_min, but it is not end of queue
			{
				g_LinkVector[li]->m_LinkMOEAry[time_min].EndTimeOfPartialCongestion = NextCongestionTransitionTimeStamp;
			}

		}

	}

	g_UpdateLinkMOEDeviation_ODEstimation(output);
	}

	float TotalTripTime = 0;
	float TotalDelay = 0;
	int VehicleSizeComplete = 0;
	float TotalDistance =0;
	int NumberofVehiclesSwitched  = 0;

	for (std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin(); iterVehicle != g_VehicleVector.end(); iterVehicle++)
	{
		if((*iterVehicle)->m_bComplete )
		{
			TotalTripTime+= (*iterVehicle)->m_TripTime;
			TotalDelay += (*iterVehicle)->m_Delay;
			TotalDistance+= (*iterVehicle)->m_Distance ;
			VehicleSizeComplete +=1;

			if((*iterVehicle)->m_bSwitched)
			{
				NumberofVehiclesSwitched+=1;
			}
		}
	}

	float AvgTravelTime = 0;
	float AvgDelay = 0;
	float AvgDistance = 0;
	float SwitchPercentage = 0;
	if(VehicleSizeComplete>0)
	{
		AvgTravelTime = TotalTripTime /VehicleSizeComplete;
		AvgDelay = TotalDelay /VehicleSizeComplete;
		AvgDistance = TotalDistance /VehicleSizeComplete;
		SwitchPercentage = (float)NumberofVehiclesSwitched*100.0f/VehicleSizeComplete;
	}

	output.NumberofVehiclesCompleteTrips = VehicleSizeComplete;
	output.NumberofVehiclesGenerated = g_Number_of_GeneratedVehicles;
	if(AvgTravelTime > 0)
	{
		output.AvgTravelTime = AvgTravelTime;
		output.AvgDelay = AvgDelay;
		output.AvgTTI = AvgTravelTime/(AvgTravelTime - AvgDelay);  // (AvgTravelTime - AvgDelay) is the free flow travel time
		output.AvgDistance = AvgDistance;
		output.SwitchPercentage = SwitchPercentage;
	}else
	{
		output.AvgTravelTime = 0;
		output.AvgDelay = 0;
		output.AvgTTI = 1;
		output.AvgDistance = 0;
		output.SwitchPercentage = 0;
	}

	/*
	ifstream is;
	int file_length= 0;
	is.open ("cumulative_scenario_summary.csv", ios::app);
	if(is.is_open())
	{
	 get length of file:
	is.seekg (0, ios::end);
	file_length = is.tellg();
	is.seekg (0, ios::beg);
	is.close();
	}

	ShortSimulationLogFile.open ("cumulative_scenario_summary.csv", ios::app);
	if(ShortSimulationLogFile.is_open())
	{

	if(file_length == 0)
	{  // write field names
	ShortSimulationLogFile << "short scenario summary,# of Vehicles,Avg Travel Time in min,Avg Travel Time Index (1.0=FFTT),Avg Distance in mile"<< endl;
	}

	ShortSimulationLogFile << g_scenario_short_description  << "," << g_VehicleVector.size() << "," << output.AvgTravelTime << "," << output.AvgTTI  << "," << output.AvgDistance << endl;
	ShortSimulationLogFile.close();
	}else
	{
	cout << "Error: File cumulative_scenario_summary.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
	g_ProgramStop();
	}

	*/
	if( Iteration == g_NumberOfIterations)  // output results at the last iteration
	{
		if(ShortSimulationLogFile.is_open())
		{
			ShortSimulationLogFile << "# of Veh = " << g_VehicleVector.size() << "; Avg Travel Time =" << output.AvgTravelTime 
				<<  "  min; Avg Distance = " << output.AvgDistance ;

			g_SimulationResult.number_of_vehicles  = g_VehicleVector.size();
			g_SimulationResult.avg_travel_time_in_min = output.AvgTravelTime;
			g_SimulationResult.avg_distance_in_miles = output.AvgDistance;
			g_SimulationResult.avg_speed = output.AvgDistance/(max(0.1,output.AvgTravelTime)/60.0f);


		}else
		{
			cout << "Error: File output_NetworkMOE cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
			g_ProgramStop();

		}
	}

	// update crash prediction statistics
	DTASafetyPredictionModel SafePredictionModel;
	SafePredictionModel.UpdateCrashRateForAllLinks();

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{
		DTAVehicle* pVehicle = iterVM->second;
		pVehicle->Day2DayPathMap[Iteration].Distance = pVehicle->m_Distance;
		pVehicle->Day2DayPathMap[Iteration].TravelTime =(pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);

		if(Iteration>=g_StartIterationsForOutputPath &&  Iteration<=g_EndIterationsForOutputPath)
		{
			pVehicle->StorePath(Iteration);
		}

	}

	return output;
}


float GetStochasticCapacity(bool bQueueFlag, float CurrentCapacity)  // per lane
{
	/* the parameters below are calibrated and documented in TRB paper titled.
	Identification and Calibration of Site-Specific Stochastic Freeway Breakdown and Queue Discharge (10-3274)
	Jia, Anxi - North Carolina State University, Raleigh 
	Williams, Billy M. - North Carolina State University, Raleigh 
	Rouphail, Nagui M. - North Carolina State University, Raleigh 
	*/
	float Normal = g_RNNOF(); // call random number anyway, used in with queue and without queue cases
	float  Capacity;
	if(bQueueFlag == false)  // pre-breakdown capacity, we have a good pdf for headways and convert them to flow rates
	{
		float Shift  = 1.5f;
		float XM    = -0.97f;
		float S     = 0.68f;

		float Headway = exp(Normal*S+XM)+Shift;

		if(Headway < 0.00001f)
			Headway = 0.00001f;

		Capacity = 3600.0f / Headway;


	}else   // queue exists, post-breakdown capcity
	{
		float fBeta  =  0.2f;
		float AvgDischargeFlow = 1850.0f/3600.0f;  // per second discharge rate
		CurrentCapacity = CurrentCapacity/3600.0f; // per second capacity
		float noise =  Normal*100.0f/3600.0f;

		// auto-regressive function: time series model
		Capacity =  CurrentCapacity  +  fBeta*(AvgDischargeFlow - CurrentCapacity) + noise;

		if(Capacity <0.0001f)
			Capacity = 0.0001f;

		Capacity*=3600.0f;  // convert back to per hour capacity

	}
	return Capacity;
}

float GetDynamicCapacityAtSignalizedIntersection(float HourlyCapacity, float CycleLength_in_seconds,double CurrentTime)
{

	float CyclceLength_in_min = CycleLength_in_seconds/60.0f;
	float g_over_c_ratio = HourlyCapacity/g_DefaultSaturationFlowRate_in_vehphpl;
	float EndofGreenTime = g_over_c_ratio*CyclceLength_in_min;

	float PerCycleTime_StartTime = CurrentTime-int(CurrentTime/CyclceLength_in_min)*CyclceLength_in_min;
	float PerCycleTime_EndTime = PerCycleTime_StartTime + g_DTASimulationInterval;  // unit: min

	float DynamicCapacity  = 0;
	if(PerCycleTime_EndTime < EndofGreenTime)  // before green time
		DynamicCapacity = g_DefaultSaturationFlowRate_in_vehphpl;
	else if (PerCycleTime_StartTime > EndofGreenTime) // after, red time
		DynamicCapacity =  0;
	else //  PerCycleTime_StartTime < EndofGreenTime < PerCycleTime_EndTime: partial time interval
	{
		DynamicCapacity =  g_DefaultSaturationFlowRate_in_vehphpl*(PerCycleTime_EndTime-EndofGreenTime)/g_DTASimulationInterval;
	}

	return DynamicCapacity;
}


void g_VehicleRerouting(int DayNo,int v, float CurrentTime, MessageSign* p_is) // v for vehicle id
{
	int PathLinkList[MAX_NODE_SIZE_IN_A_PATH]={-1};  // existing links
	int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={-1};  // new nodes
	SVehicleLink vehicle_link_ary [MAX_NODE_SIZE_IN_A_PATH];

	int CurrentLinkID = g_VehicleVector[v]->m_aryVN[g_VehicleVector[v]->m_SimLinkSequenceNo].LinkID;
	int	CurrentNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

	// copy exsting links up to the current link
	for(int l =0; l<= g_VehicleVector[v]->m_SimLinkSequenceNo; l++)
	{
		vehicle_link_ary[l].AbsArrivalTimeOnDSN = g_VehicleVector[v]->m_aryVN[l].AbsArrivalTimeOnDSN ;
		vehicle_link_ary[l].LaneBasedCumulativeFlowCount  =  g_VehicleVector[v]->m_aryVN[l].LaneBasedCumulativeFlowCount ;
		vehicle_link_ary[l].LinkID = g_VehicleVector[v]->m_aryVN[l].LinkID  ;
	}

	/*
	if(is.Type == 2) // detour VMS
	{
	for(int ll = 0; ll < is.DetourLinkSize ; ll++)
	{
	PathLinkList[l++] = is.DetourLinkArray [ll];
	CurrentTime+= g_LinkVector[is.DetourLinkArray [ll]]->m_FreeFlowTravelTime;  // add travel time along the path to get the starting time at the end of subpath

	}
	CurrentNodeID = g_LinkVector[is.DetourLinkArray [is.DetourLinkSize -1]]->m_ToNodeID;

	}
	*/
	int LastLinkID = g_VehicleVector[v]->m_aryVN[g_VehicleVector[v]->m_NodeSize-2].LinkID;  // m_NodeSize-1 is # of links, m_NodeSize-2 is no. of link sequence
	int	LastNodeID = g_LinkVector[LastLinkID]->m_ToNodeID;

	// find shortest path
	int SubPathNodeSize = 0;

	TRACE("\nlast node: %d, CurrentNodeID: %d ",g_NodeVector[LastNodeID].m_NodeName,g_NodeVector[CurrentNodeID].m_NodeName);

	int PredNode = LastNodeID ;  // destination

	// scan backward in the predessor array of the shortest path calculation results
	while(PredNode !=  CurrentNodeID && PredNode!=-1 && SubPathNodeSize< MAX_NODE_SIZE_IN_A_PATH)
	{
		ASSERT(SubPathNodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
		PathNodeList[SubPathNodeSize] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
		PathLinkList[SubPathNodeSize] = p_is->LinkNoAry [PredNode];
		PredNode = p_is->NodePredAry[PredNode];
		//		TRACE("\n<<: %d, ",g_NodeVector[PredNode].m_NodeName);
		SubPathNodeSize ++;
	}

	// add current node to find the links/paths
	PathNodeList[SubPathNodeSize++] = CurrentNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.

	// update paths
	if( g_VehicleVector[v]->m_aryVN !=NULL)
	{
		delete g_VehicleVector[v]->m_aryVN;
	}

	g_VehicleVector[v]->m_NodeSize = g_VehicleVector[v]->m_SimLinkSequenceNo + SubPathNodeSize +1;
	g_VehicleVector[v]->m_aryVN = new SVehicleLink[g_VehicleVector[v]->m_NodeSize];

	if(g_VehicleVector[v]->m_aryVN==NULL)
	{
		cout << "Insufficient memory for allocating vehicle arrays!";
		g_ProgramStop();

	}
	// copy existing link sequence back to the new link no array
	int i;
	for(i=0; i<=g_VehicleVector[v]->m_SimLinkSequenceNo; i++)
	{
		g_VehicleVector[v]->m_aryVN[i].AbsArrivalTimeOnDSN  = vehicle_link_ary[i].AbsArrivalTimeOnDSN ;
		g_VehicleVector[v]->m_aryVN[i].LaneBasedCumulativeFlowCount   = vehicle_link_ary[i].LaneBasedCumulativeFlowCount ;
		g_VehicleVector[v]->m_aryVN[i].LinkID   = vehicle_link_ary[i].LinkID ;
	}

	for(i=0; i<SubPathNodeSize-1; i++)
	{
		int link_id = PathLinkList[SubPathNodeSize-2-i];
		ASSERT(link_id < g_LinkVector.size());
		//		TRACE("\nlink from: %d, ",g_LinkVector[link_id]->m_FromNodeID );
		g_VehicleVector[v]->m_aryVN[g_VehicleVector[v]->m_SimLinkSequenceNo+1+i].LinkID = link_id;
	}

	g_VehicleVector[v]->Day2DayPathMap[DayNo].bDiverted = true;  // mark diversion

}





bool g_VehicularSimulation_BasedOnADCurves(int DayNo, double CurrentTime, int simulation_time_interval_no, int TrafficFlowModelFlag)
{

	int time_stamp_in_min = int(CurrentTime+0.0001);

	int current_simulation_time_interval_no = g_DemandLoadingStartTimeInMin*g_number_of_intervals_per_min+simulation_time_interval_no;

	//	TRACE("st=%d\n", simulation_time_interval_no);
	//DTALite:
	// vertical queue data structure
	// load vehicles into network

	// step 1: scan all the vehicles, if a vehicle's start time >= CurrentTime, and there is available space in the first link,
	// load this vehicle into the ready queue

	// comment: we use map here as the g_VehicleMap map is sorted by departure times.
	// At each iteration, we start  the last loaded id, and exit if the departure time of a vehicle is later than the current time.


	//for each node, we scan each incoming link in a randomly sequence, based on simulation_time_interval_no

	bool bParallelMode = true;
	int node_size = g_NodeVector.size();
	int link_size = g_LinkVector.size();


	//step 0: initialization 
	#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{

		// cummulative flow counts
		DTALink* pLink  = g_LinkVector[li];

			 pLink->EntranceBufferSize = 0;
			 pLink->NewVehicleCount = 0;
			 pLink->ExitVehicleCount = 0;
	}


				// step 1: load vehicles 

#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{

		// cummulative flow counts
		DTALink* pLink  = g_LinkVector[li];

		DTAVehicle* pVeh;

			while(pLink->StartIndexOfLoadingBuffer < pLink->LoadingBufferSize)
			{
				int VehicleID = pLink->LoadingBufferVector [pLink->StartIndexOfLoadingBuffer];
				pVeh = g_VehicleMap[VehicleID];
				if(g_floating_point_value_less_than_or_eq_comparison(pVeh->m_DepartureTime, CurrentTime) == false)
				{	
					// later than the current time
					break;
				}else
				{ // before than the current time: load
					pVeh->m_SimLinkSequenceNo = 0;
					pVeh->m_TollDollarCost +=pLink->GetTollRateInDollar(DayNo,CurrentTime,pVeh->m_PricingType );

					struc_vehicle_item vi;
					vi.veh_id = pVeh->m_VehicleID ;
					// update vehicle statistics
					vi.time_stamp = pVeh->m_DepartureTime + pLink->GetFreeMovingTravelTime(TrafficFlowModelFlag, CurrentTime);  // unit: min

					// update link statistics
					pLink->NewVehicleCount++;
					pLink->EntranceBuffer[pLink->EntranceBufferSize++]=vi;

					if(pLink->EntranceBufferSize>=900)
					{
					cout << "EntranceBufferEntranceBufferSize>=900"; 
					g_ProgramStop();
					
					}
					// add cumulative flow count to vehicle

					int pricing_type = pVeh->m_PricingType ;
					pLink->CFlowArrivalCount_PricingType[pricing_type] +=1;
					pLink->CFlowArrivalRevenue_PricingType[pricing_type] += pLink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);
					

					// access VMS information from the first link
					int IS_id  = pLink->GetInformationResponseID(DayNo,CurrentTime);
					if(IS_id >= 0)
					{
						if( pVeh->GetRandomRatio()*100 < pLink->MessageSignVector [IS_id].ResponsePercentage )
						{  // vehicle rerouting

							g_VehicleRerouting(DayNo,vi.veh_id,CurrentTime, &(pLink->MessageSignVector [IS_id]));

						}

					}
					// advance starting index of loading buffer
					pLink->StartIndexOfLoadingBuffer++;
				}

			}


		} // for each link
	


		// step 2: determine capacity
#pragma omp parallel for
	for(int node = 0; node < node_size; node++)  // for each node
	{
		int IncomingLinkSize = g_NodeVector[node].m_IncomingLinkVector.size();
		int incoming_link_count = 0;

		for(incoming_link_count=0;  incoming_link_count < IncomingLinkSize; incoming_link_count++)  // for each incoming link
		{

			int li = g_NodeVector[node].m_IncomingLinkVector[incoming_link_count];

			DTALink* pLink = g_LinkVector[li];

			// vehicle_out_count is the minimum of LinkOutCapacity and ExitQueue Size

			// determine link out capacity 
			float MaximumFlowRate = pLink->m_MaximumServiceFlowRatePHPL *g_DTASimulationInterval/60.0f*pLink->GetNumLanes(DayNo,CurrentTime); //60 --> cap per min --> unit # of vehicle per simulation interval
			// use integer number of vehicles as unit of capacity

			int A_time_index = max(0,current_simulation_time_interval_no-1-pLink->m_FFTT_simulation_interval) % MAX_TIME_INTERVAL_ADCURVE;
			int D_time_index = max(0,current_simulation_time_interval_no-1) % MAX_TIME_INTERVAL_ADCURVE;
			int number_of_vehicles_in_vertical_queue = pLink->A[A_time_index] - pLink->D[D_time_index];

			int cap_value = g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(MaximumFlowRate,li,current_simulation_time_interval_no);

			int vehicle_out_count = min(number_of_vehicles_in_vertical_queue,cap_value);
	
			// enforcing hard constraints on the number of vehicles can be discharged.
			if(vehicle_out_count > pLink->FIFO_queue_size)
			{
				vehicle_out_count = pLink->FIFO_queue_size;
			}
			// step 3:  move vehicles
			while(vehicle_out_count >=1)
			{

			 ASSERT(pLink->FIFO_queue_size>=1);
			  struc_vehicle_item vi = pLink->FIFOQueue_pop_front();
				pLink->CFlowDepartureCount +=1;
				pLink-> departure_count +=1;



				int vehicle_id = vi.veh_id;

				DTAVehicle* pVeh = g_VehicleMap[vehicle_id];


				// record arrival time at the downstream node of current link
				int link_sequence_no = pVeh->m_SimLinkSequenceNo;

				int t_link_arrival_time=0;
				if(link_sequence_no >=1)
				{
					t_link_arrival_time= int(pVeh->m_aryVN[link_sequence_no-1].AbsArrivalTimeOnDSN);
				}else
				{
					t_link_arrival_time = int(pVeh->m_DepartureTime);
				}

				float ArrivalTimeOnDSN = 0;
				if(g_floating_point_value_less_than_or_eq_comparison(CurrentTime-g_DTASimulationInterval,vi.time_stamp)) 
					// arrival at previous interval
				{  // no delay 
					ArrivalTimeOnDSN = vi.time_stamp;
				}else
				{  // delayed at previous time interval, discharge at CurrentTime 
					ArrivalTimeOnDSN = CurrentTime; 
				}

				// update statistics for traveled link
				pVeh->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN = ArrivalTimeOnDSN;
				float TravelTime = 0;

				if(link_sequence_no >=1)
				{
					TravelTime= pVeh->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN -
						pVeh->m_aryVN[link_sequence_no-1].AbsArrivalTimeOnDSN;
				}else
				{
					TravelTime= pVeh->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN -
						pVeh->m_DepartureTime ;

				}
				pVeh->m_Delay += (TravelTime-pLink->m_FreeFlowTravelTime);
				// finally move to next link
				pVeh->m_SimLinkSequenceNo = pVeh->m_SimLinkSequenceNo+1;


				pLink-> total_departure_based_travel_time += TravelTime;
				if(t_link_arrival_time < pLink->m_LinkMOEAry.size())
				{
					pLink->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime  += TravelTime;
					pLink->m_LinkMOEAry[t_link_arrival_time].TotalFlowCount +=1;
				}


				int number_of_links = pVeh->m_NodeSize-1;
				if(pVeh->m_SimLinkSequenceNo < number_of_links-1)  // not reach destination yet
				{
					// not reach destination yet
					// advance to next link


					int NextLink = pVeh->m_aryVN[pVeh->m_SimLinkSequenceNo].LinkID;
					DTALink* p_Nextlink = g_LinkVector[NextLink];

					if(p_Nextlink->m_FromNodeID != node)
					{
						cout << "p_Nextlink->m_FromNodeID : veh " << pVeh->m_VehicleID << ": " << " this link: " << pLink ->m_FromNodeNumber << "->" << pLink ->m_ToNodeNumber << " next link" << p_Nextlink ->m_FromNodeNumber << "->" << p_Nextlink ->m_ToNodeNumber << ";; current Node: " << g_NodeVector[node].m_NodeName;
					getchar();
					}

					// access VMS information here! p_Nextlink is now the current link
					int IS_id  = p_Nextlink->GetInformationResponseID(DayNo,CurrentTime);
					if(IS_id >= 0)
					{
						if( pVeh->GetRandomRatio()*100 < p_Nextlink->MessageSignVector [IS_id].ResponsePercentage )
						{  // vehicle rerouting

							g_VehicleRerouting(DayNo,vehicle_id,CurrentTime, &(p_Nextlink->MessageSignVector [IS_id]));

						}
					}



					vi.veh_id = vehicle_id;

					vi.time_stamp = ArrivalTimeOnDSN + p_Nextlink->GetFreeMovingTravelTime(TrafficFlowModelFlag, CurrentTime);
					// update link statistics
					p_Nextlink->EntranceBuffer [p_Nextlink->EntranceBufferSize]=vi;

					p_Nextlink->EntranceBufferSize++;
					if(p_Nextlink->EntranceBufferSize>=900)
					{
					cout << "EntranceBufferEntranceBufferSize>=900"; 
					g_ProgramStop();
					
					}
					

					// important notes: because all incoming links that have access to p_NextLink belong to one node, our parallelization around nodes is safe


					//int pricing_type = pVeh->m_PricingType ;
					//p_Nextlink->CFlowArrivalCount_PricingType[pricing_type] +=1;
					//p_Nextlink->CFlowArrivalRevenue_PricingType[pricing_type] += p_Nextlink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);
					//pVeh->m_TollDollarCost += p_Nextlink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);

				}else
				{  // reach destination

					pLink->ExitVehicleCount ++;
					pVeh->m_ArrivalTime = ArrivalTimeOnDSN;
					pVeh->m_TripTime = pVeh->m_ArrivalTime - pVeh->m_DepartureTime;
					pVeh->m_bComplete = true;
				}

				
				vehicle_out_count--;

			}


		}  // for each incoming link
	} // for each node

	// step 3: calculate statistics
	// cummulative flow counts

		// single thread
	for(int li = 0; li< link_size; li++)
	{	
					DTALink* pLink = g_LinkVector[li];
					// update network statistics
					g_Number_of_GeneratedVehicles += pLink->NewVehicleCount ;
					g_Number_of_CompletedVehicles += pLink->ExitVehicleCount ;

	g_NetworkMOEAry[time_stamp_in_min].Flow_in_a_min += pLink->NewVehicleCount;

	}
	g_NetworkMOEAry[time_stamp_in_min].CumulativeInFlow = g_Number_of_GeneratedVehicles;

				g_NetworkMOEAry[time_stamp_in_min].CumulativeOutFlow = g_Number_of_CompletedVehicles;
//to do					int OriginDepartureTime = (int)(pVeh->m_DepartureTime );
//to do					g_NetworkMOEAry[OriginDepartureTime].AbsArrivalTimeOnDSN_in_a_min +=pVeh->m_TripTime;

// step 4: move vehicles from EntranceBuffer to EntranceQueue

#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{

		// cummulative flow counts
		DTALink* pLink  = g_LinkVector[li];


		for(int i= 0; i < pLink->EntranceBufferSize; i++)
		{


			pLink->FIFOQueue_push_back (pLink->EntranceBuffer[i]);
			pLink->CFlowArrivalCount++;

			struc_vehicle_item vi =  pLink->EntranceBuffer[i];

		}
		pLink->EntranceBufferSize  = 0;

		int time_index  = current_simulation_time_interval_no  % MAX_TIME_INTERVAL_ADCURVE;
		pLink->A[time_index] = pLink->CFlowArrivalCount;
		pLink->D[time_index] = pLink->CFlowDepartureCount;

		//int t_residual = simulation_time_interval_no % MAX_TIME_INTERVAL_ADCURVE;

		//pLink->m_CumuArrivalFlow[t_residual] = pLink->CFlowArrivalCount;
		//pLink->m_CumuDeparturelFlow[t_residual] = pLink->CFlowDepartureCount;

		if(simulation_time_interval_no%g_number_of_intervals_per_min==0 )  // per min statistics
		{
			pLink->VehicleCount = pLink->CFlowArrivalCount - pLink->CFlowDepartureCount;
			pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount =  pLink->CFlowArrivalCount;
			pLink->m_LinkMOEAry [time_stamp_in_min].ExitQueueLength =  pLink->FIFO_queue_size;

			// toll collection 
			for(int pt = 1; pt < MAX_PRICING_TYPE_SIZE; pt++)
			{
				pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount_PricingType[pt] = pLink->CFlowArrivalCount_PricingType[pt];
				pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeRevenue_PricingType[pt] = pLink->CFlowArrivalRevenue_PricingType[pt];
			}

			pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeDepartureCount = pLink->CFlowDepartureCount;

		}
	}
	return true;
}

//bool g_VehicularCarFollowingSimulation(int DayNo, double CurrentTime, int simulation_time_interval_no, int TrafficFlowModelFlag)
//{
//
//	int time_stamp_in_min = int(CurrentTime+0.0001);
//
//	int current_simulation_time_interval_no = g_DemandLoadingStartTimeInMin*g_number_of_car_following_intervals_per_min+simulation_time_interval_no;
//
//	//	TRACE("st=%d\n", simulation_time_interval_no);
//	//DTALite:
//	// vertical queue data structure
//	// load vehicles into network
//
//	// step 1: scan all the vehicles, if a vehicle's start time >= CurrentTime, and there is available space in the first link,
//	// load this vehicle into the ready queue
//
//	// comment: we use map here as the g_VehicleMap map is sorted by departure times.
//	// At each iteration, we start  the last loaded id, and exit if the departure time of a vehicle is later than the current time.
//
//
//	//for each node, we scan each incoming link in a randomly sequence, based on simulation_time_interval_no
//
//	bool bParallelMode = true;
//	int node_size = g_NodeVector.size();
//	int link_size = g_LinkVector.size();
//
//
//	//step 0: initialization 
//	#pragma omp parallel for
//	for(int li = 0; li< link_size; li++)
//	{
//
//		// cummulative flow counts
//		DTALink* pLink  = g_LinkVector[li];
//
//			 pLink->EntranceBufferSize = 0;
//			 pLink->NewVehicleCount = 0;
//			 pLink->ExitVehicleCount = 0;
//	}
//
//
//				// step 1: load vehicles 
//
//#pragma omp parallel for
//	for(int li = 0; li< link_size; li++)
//	{
//
//		// cummulative flow counts
//		DTALink* pLink  = g_LinkVector[li];
//
//		DTAVehicle* pVeh;
//
//			while(pLink->StartIndexOfLoadingBuffer < pLink->LoadingBufferSize)
//			{
//				int VehicleID = pLink->LoadingBufferVector [pLink->StartIndexOfLoadingBuffer];
//				pVeh = g_VehicleMap[VehicleID];
//				if(g_floating_point_value_less_than_or_eq_comparison(pVeh->m_DepartureTime, CurrentTime) == false)
//				{	
//					// later than the current time
//					break;
//				}else
//				{ // before than the current time: load
//					pVeh->m_SimLinkSequenceNo = 0;
//					pVeh->m_TollDollarCost +=pLink->GetTollRateInDollar(DayNo,CurrentTime,pVeh->m_PricingType );
//
//					struc_vehicle_item vi;
//					vi.veh_id = pVeh->m_VehicleID ;
//					// update vehicle statistics
//					vi.time_stamp = pVeh->m_DepartureTime + pLink->GetFreeMovingTravelTime(TrafficFlowModelFlag, CurrentTime);  // unit: min
//
//					// update link statistics
//					pLink->NewVehicleCount++;
//					pLink->EntranceBuffer[pLink->EntranceBufferSize++]=vi;
//
//					if(pLink->EntranceBufferSize>=900)
//					{
//					cout << "EntranceBufferEntranceBufferSize>=900"; 
//					g_ProgramStop();
//					
//					}
//					// add cumulative flow count to vehicle
//
//					int pricing_type = pVeh->m_PricingType ;
//					pLink->CFlowArrivalCount_PricingType[pricing_type] +=1;
//					pLink->CFlowArrivalRevenue_PricingType[pricing_type] += pLink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);
//					
//
//
//					pLink->StartIndexOfLoadingBuffer++;
//				}
//
//			}
//
//
//		} // for each link
//	
//
//
//		// step 2: determine capacity
//#pragma omp parallel for
//	for(int node = 0; node < node_size; node++)  // for each node
//	{
//		int IncomingLinkSize = g_NodeVector[node].m_IncomingLinkVector.size();
//		int incoming_link_count = 0;
//
//		for(incoming_link_count=0;  incoming_link_count < IncomingLinkSize; incoming_link_count++)  // for each incoming link
//		{
//
//			int li = g_NodeVector[node].m_IncomingLinkVector[incoming_link_count];
//
//			DTALink* pLink = g_LinkVector[li];
//
//			// vehicle_out_count is the minimum of LinkOutCapacity and ExitQueue Size
//
//			// determine link out capacity 
//			float MaximumFlowRate = pLink->m_MaximumServiceFlowRatePHPL *g_CarFollowingSimulationInterval/60.0f*pLink->GetNumLanes(DayNo,CurrentTime); //60 --> cap per min --> unit # of vehicle per simulation interval
//			// use integer number of vehicles as unit of capacity
//
//			int A_time_index = max(0,current_simulation_time_interval_no-1-pLink->m_FFTT_simulation_interval) % MAX_TIME_INTERVAL_ADCURVE;
//			int D_time_index = max(0,current_simulation_time_interval_no-1) % MAX_TIME_INTERVAL_ADCURVE;
//			int number_of_vehicles_in_vertical_queue = pLink->A[A_time_index] - pLink->D[D_time_index];
//
//			int cap_value = g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(MaximumFlowRate,li,current_simulation_time_interval_no);
//
//			int vehicle_out_count = min(number_of_vehicles_in_vertical_queue,cap_value);
//	
//			// enforcing hard constraints on the number of vehicles can be discharged.
//			if(vehicle_out_count > pLink->FIFO_queue_size)
//			{
//				vehicle_out_count = pLink->FIFO_queue_size;
//			}
//			// step 3:  move vehicles
//			while(vehicle_out_count >=1)
//			{
//
//			 ASSERT(pLink->FIFO_queue_size>=1);
//			  struc_vehicle_item vi = pLink->FIFOQueue_pop_front();
//				pLink->CFlowDepartureCount +=1;
//				pLink-> departure_count +=1;
//
//
//				int vehicle_id = vi.veh_id;
//
//				DTAVehicle* pVeh = g_VehicleMap[vehicle_id];
//
//
//				// record arrival time at the downstream node of current link
//				int link_sequence_no = pVeh->m_SimLinkSequenceNo;
//
//				int t_link_arrival_time=0;
//				if(link_sequence_no >=1)
//				{
//					t_link_arrival_time= int(pVeh->m_aryVN[link_sequence_no-1].AbsArrivalTimeOnDSN);
//				}else
//				{
//					t_link_arrival_time = int(pVeh->m_DepartureTime);
//				}
//
//				float ArrivalTimeOnDSN = 0;
//				if(g_floating_point_value_less_than_or_eq_comparison(CurrentTime-g_CarFollowingSimulationInterval,vi.time_stamp)) 
//					// arrival at previous interval
//				{  // no delay 
//					ArrivalTimeOnDSN = vi.time_stamp;
//				}else
//				{  // delayed at previous time interval, discharge at CurrentTime 
//					ArrivalTimeOnDSN = CurrentTime; 
//				}
//
//				// update statistics for traveled link
//				pVeh->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN = ArrivalTimeOnDSN;
//				float TravelTime = 0;
//
//				if(link_sequence_no >=1)
//				{
//					TravelTime= pVeh->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN -
//						pVeh->m_aryVN[link_sequence_no-1].AbsArrivalTimeOnDSN;
//				}else
//				{
//					TravelTime= pVeh->m_aryVN[link_sequence_no].AbsArrivalTimeOnDSN -
//						pVeh->m_DepartureTime ;
//
//				}
//				pVeh->m_Delay += (TravelTime-pLink->m_FreeFlowTravelTime);
//				// finally move to next link
//				pVeh->m_SimLinkSequenceNo = pVeh->m_SimLinkSequenceNo+1;
//
//
//				pLink-> total_departure_based_travel_time += TravelTime;
//				if(t_link_arrival_time < pLink->m_LinkMOEAry.size())
//				{
//					pLink->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime  += TravelTime;
//					pLink->m_LinkMOEAry[t_link_arrival_time].TotalFlowCount +=1;
//				}
//
//
//				int number_of_links = pVeh->m_NodeSize-1;
//				if(pVeh->m_SimLinkSequenceNo < number_of_links-1)  // not reach destination yet
//				{
//					// not reach destination yet
//					// advance to next link
//
//
//					int NextLink = pVeh->m_aryVN[pVeh->m_SimLinkSequenceNo].LinkID;
//					DTALink* p_Nextlink = g_LinkVector[NextLink];
//
//					if(p_Nextlink->m_FromNodeID != node)
//					{
//						cout << "p_Nextlink->m_FromNodeID : veh " << pVeh->m_VehicleID << ": " << " this link: " << pLink ->m_FromNodeNumber << "->" << pLink ->m_ToNodeNumber << " next link" << p_Nextlink ->m_FromNodeNumber << "->" << p_Nextlink ->m_ToNodeNumber << ";; current Node: " << g_NodeVector[node].m_NodeName;
//					getchar();
//					}
//
//					vi.veh_id = vehicle_id;
//
//					vi.time_stamp = ArrivalTimeOnDSN + p_Nextlink->GetFreeMovingTravelTime(TrafficFlowModelFlag, CurrentTime);
//					// update link statistics
//					p_Nextlink->EntranceBuffer [p_Nextlink->EntranceBufferSize]=vi;
//
//					p_Nextlink->EntranceBufferSize++;
//					if(p_Nextlink->EntranceBufferSize>=900)
//					{
//					cout << "EntranceBufferEntranceBufferSize>=900"; 
//					g_ProgramStop();
//					
//					}
//					
//
//					// important notes: because all incoming links that have access to p_NextLink belong to one node, our parallelization around nodes is safe
//
//
//					//int pricing_type = pVeh->m_PricingType ;
//					//p_Nextlink->CFlowArrivalCount_PricingType[pricing_type] +=1;
//					//p_Nextlink->CFlowArrivalRevenue_PricingType[pricing_type] += p_Nextlink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);
//					//pVeh->m_TollDollarCost += p_Nextlink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);
//
//				}else
//				{  // reach destination
//
//					pLink->ExitVehicleCount ++;
//					pVeh->m_ArrivalTime = ArrivalTimeOnDSN;
//					pVeh->m_TripTime = pVeh->m_ArrivalTime - pVeh->m_DepartureTime;
//					pVeh->m_bComplete = true;
//				}
//
//				
//				vehicle_out_count--;
//
//			}
//
//
//		}  // for each incoming link
//	} // for each node
//
////------------------------------------ car following simulation
////	#pragma omp parallel for
//	for(int li = 0; li< link_size; li++)
//	{
//		DTALink* pLink  = g_LinkVector[li];
//
//		// calculate free-flow moving distance
//		float FreeflowDistance_per_SimulationInterval  = pLink->m_SpeedLimit* 1609.344f/3600/NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND; 
//		float CriticalSpacing_in_meter = 1609.344f / pLink->m_KJam ; 
//		float link_length_in_meter = pLink-> m_Length * 1609.344f; //1609.344f: mile to meters;
//		int TimeLag_in_SimulationInterval = (int)(3600*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND/(pLink->m_BackwardWaveSpeed * pLink->m_KJam )+0.5);
//
//		if(TimeLag_in_SimulationInterval>=300)
//		{
//		
//			TimeLag_in_SimulationInterval = 300;
//		
//		}
//	
//		for(int vehicle_sequence_no = 0; vehicle_sequence_no < pLink->FIFO_queue_size; vehicle_sequence_no++)
//	{
//		int index = (pLink->FIFO_front+vehicle_sequence_no) %(pLink->FIFO_queue_max_size-1);  // (FIFO_queue_max_size-1 is old cycle length
//		struc_vehicle_item item = pLink->FIFOQueue[index]; 
//
//		// car following simuluation
//		int current_time_interval_no = simulation_time_interval_no%300;  // 300 is the time dimension size of vehicle distance array
//		int previous_time_interval_no = max(0,simulation_time_interval_no-1)%300;  // 300 is the time dimension size of vehicle distance array
//
//		int current_vehicle_sequence_no = item.veh_car_following_no ;
//
//		float CurrentVehicleDistance = pLink->VechileDistanceAry [current_vehicle_sequence_no][previous_time_interval_no] + FreeflowDistance_per_SimulationInterval;
//
//		int leader_vehicle_sequence_no = (item.veh_car_following_no - pLink->m_NumLanes)%(pLink->FIFO_queue_max_size-1);
//
//		if(item.veh_car_following_no >= pLink->m_NumLanes && pLink->FIFOQueue[leader_vehicle_sequence_no].veh_car_following_no >=0)  // with leader car
//		{
//			//xiC(t) = xi-1(t-tau) - delta
//			int time_t_minus_tau = (current_time_interval_no - TimeLag_in_SimulationInterval)%300; // need to convert time in second to time in simulation time interval
//
//			if(time_t_minus_tau >=0)  // the leader has not reached destination yet
//			{
//				// vehicle v-1: previous car
//				float CongestedDistance =  pLink->VechileDistanceAry[leader_vehicle_sequence_no][time_t_minus_tau]  - CriticalSpacing_in_meter ;
//				// xi(t) = min(xAF(t), xAC(t))
//				if (CurrentVehicleDistance  > CongestedDistance && CongestedDistance >=  pLink->VechileDistanceAry[current_vehicle_sequence_no][previous_time_interval_no])
//					CurrentVehicleDistance = CongestedDistance;
//			}
//
//			pLink->VechileDistanceAry[current_vehicle_sequence_no][current_time_interval_no] = CurrentVehicleDistance;
//
//			}
//	
//		
//						if(simulation_time_interval_no%NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND == 0)  // per_second
//					{
//						// for active vehicles (with positive speed or positive distance
//
//						float SpeedBySecond = pLink->m_SpeedLimit* 0.44704f ; // 1 mph = 0.44704 meters per second
//
//						float vehicle_entrance_time_in_simulation_interval = (item.time_stamp - pLink->GetFreeMovingTravelTime(TrafficFlowModelFlag, CurrentTime))/g_CarFollowingSimulationInterval;
//
//						if(current_simulation_time_interval_no  >= vehicle_entrance_time_in_simulation_interval + NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND)  // not the first second
//						{
//							SpeedBySecond = (pLink->VechileDistanceAry[current_vehicle_sequence_no][simulation_time_interval_no] -   
//								pLink->VechileDistanceAry[current_vehicle_sequence_no][simulation_time_interval_no-NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND]);
//						}
//
//						// different lanes have different vehicle numbers, so we should not have openMP conflicts here
//						DTAVehicle* pVehicle  = g_VehicleMap[item.veh_id ];
//						float accelation = SpeedBySecond - pVehicle->m_PrevSpeed ;
//						//int OperatingMode =  pLink->ComputeOperatingModeFromSpeed(SpeedBySecond, accelation);
//						pVehicle->m_OperatingModeCount[OperatingMode]+=1;
//						pVehicle->m_PrevSpeed  = SpeedBySecond;
//					}
//	
//		}
//	}
//// car following simulation ------------------------------------
//
//	// step 3: calculate statistics
//	// cummulative flow counts
//
//		// single thread
//	for(int li = 0; li< link_size; li++)
//	{	
//					DTALink* pLink = g_LinkVector[li];
//					// update network statistics
//					g_Number_of_GeneratedVehicles += pLink->NewVehicleCount ;
//					g_Number_of_CompletedVehicles += pLink->ExitVehicleCount ;
//
//	g_NetworkMOEAry[time_stamp_in_min].Flow_in_a_min += pLink->NewVehicleCount;
//
//	}
//	g_NetworkMOEAry[time_stamp_in_min].CumulativeInFlow = g_Number_of_GeneratedVehicles;
//
//				g_NetworkMOEAry[time_stamp_in_min].CumulativeOutFlow = g_Number_of_CompletedVehicles;
////to do					int OriginDepartureTime = (int)(pVeh->m_DepartureTime );
////to do					g_NetworkMOEAry[OriginDepartureTime].AbsArrivalTimeOnDSN_in_a_min +=pVeh->m_TripTime;
//
//// step 4: move vehicles from EntranceBuffer to EntranceQueue
//
//#pragma omp parallel for
//	for(int li = 0; li< link_size; li++)
//	{
//
//		// cummulative flow counts
//		DTALink* pLink  = g_LinkVector[li];
//
//
//		for(int i= 0; i < pLink->EntranceBufferSize; i++)
//		{
//
//
//			pLink->FIFOQueue_push_back (pLink->EntranceBuffer[i]);
//			pLink->CFlowArrivalCount++;
//
//			struc_vehicle_item vi =  pLink->EntranceBuffer[i];
//
//		}
//		pLink->EntranceBufferSize  = 0;
//
//		int time_index  = current_simulation_time_interval_no  % MAX_TIME_INTERVAL_ADCURVE;
//		pLink->A[time_index] = pLink->CFlowArrivalCount;
//		pLink->D[time_index] = pLink->CFlowDepartureCount;
//
//		//int t_residual = simulation_time_interval_no % MAX_TIME_INTERVAL_ADCURVE;
//
//		//pLink->m_CumuArrivalFlow[t_residual] = pLink->CFlowArrivalCount;
//		//pLink->m_CumuDeparturelFlow[t_residual] = pLink->CFlowDepartureCount;
//
//		if(simulation_time_interval_no%g_number_of_car_following_intervals_per_min==0 )  // per min statistics
//		{
//			pLink->VehicleCount = pLink->CFlowArrivalCount - pLink->CFlowDepartureCount;
//			pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount =  pLink->CFlowArrivalCount;
//			pLink->m_LinkMOEAry [time_stamp_in_min].ExitQueueLength =  pLink->FIFO_queue_size;
//
//			// toll collection 
//			for(int pt = 1; pt < MAX_PRICING_TYPE_SIZE; pt++)
//			{
//				pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount_PricingType[pt] = pLink->CFlowArrivalCount_PricingType[pt];
//				pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeRevenue_PricingType[pt] = pLink->CFlowArrivalRevenue_PricingType[pt];
//			}
//
//			pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeDepartureCount = pLink->CFlowDepartureCount;
//
//		}
//	}
//	return true;
//}
//
//
