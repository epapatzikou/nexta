//  Portions Copyright 2010 Xuesong Zhou, Jeff Taylor, Jinjin Tang

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

extern bool g_VehicularSimulation_BasedOnADCurves(int DayNo, double CurrentTime, int simulation_time_interval_no, e_traffic_flow_model TrafficFlowModelFlag);
extern bool g_VehicularCarFollowingSimulation(int DayNo, double CurrentTime, int simulation_time_interval_no, e_traffic_flow_model TrafficFlowModelFlag);
extern  int NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
extern ofstream g_LogFile;

bool g_VehicularSimulation(int DayNo, double CurrentTime, int simulation_time_interval_no, e_traffic_flow_model TrafficFlowModelFlag)
{
	int trace_step = 14;

	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;
	std::map<int, DTAVehicle*>::iterator iterVM;

	int time_stamp_in_min = int(CurrentTime+0.0001);

	CString trace_msg;

	trace_msg.Format ("time %f, interval %d",CurrentTime,simulation_time_interval_no);
	g_ProgramTrace(trace_msg);

	std::list<struc_vehicle_item>::iterator vii;

	int vehicle_id_trace = 7;
	int link_id_trace = -1;


	bool debug_flag = true ;


	bool bRadioMessageActive  = false;
	float network_wide_RadioMessageResponsePercentage = 0; 

	if((simulation_time_interval_no)%10 == 0)  // every one minute
	{
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			DTALink * pLink = g_LinkVector[li];

			float ResponsePercentage = pLink->GetRadioMessageResponsePercentage(DayNo,CurrentTime);
			if( ResponsePercentage > -0.01)// positive vlaue
			{
				// use maximum response percentage when there are multiple messages
				network_wide_RadioMessageResponsePercentage = max(network_wide_RadioMessageResponsePercentage, ResponsePercentage);

			}

		}
	}
	//DTALite:
	// vertical queue data structure
	// each link  ExitQueue, EntranceQueue: (VehicleID, ReadyTime)

	// we update departure based information every 5 min
	if(g_bInformationUpdatingAndReroutingFlag && g_ODEstimationFlag !=1)
	{
		if((simulation_time_interval_no)%(10*g_information_updating_interval_in_min) == 0)
		{
			// reset statistics for departure-time-based travel time collection

			if(CurrentTime >= 65)
				TRACE("");

			for(unsigned li = 0; li< g_LinkVector.size(); li++)
			{
				DTALink * pLink = g_LinkVector[li];


				// weighting average
				float weight = 0.5;

				pLink->m_prevailing_travel_time = 
					(1-weight)*	pLink->m_prevailing_travel_time
					+ weight * 	max(pLink->m_FreeFlowTravelTime , 
					pLink->total_departure_based_travel_time/max(1, pLink->departure_count));

				if(pLink->m_FromNodeNumber == 5 && pLink->m_ToNodeNumber == 6)
				{
					TRACE("current time %f; prevailing time = %f\n",CurrentTime,  pLink->m_prevailing_travel_time);
				}

				g_LinkVector[li]-> departure_count = 0;
				g_LinkVector[li]-> total_departure_based_travel_time = 0;
			}
		}



		if(simulation_time_interval_no%10 ==0) //update routes for pre-trip or en-route information users, every min, (10 simulation time intervlas)
		{

			g_AgentBasedPathAdjustment(DayNo, CurrentTime);

			g_AgentBasedVMSRoutingInitialization(DayNo, CurrentTime);

		}

	}


	// user_defined information updating 

	int time_clock_in_min  = simulation_time_interval_no/10;
	if(simulation_time_interval_no%10 == 0 && g_RealTimeSimulationSettingsMap.find(time_clock_in_min)!= g_RealTimeSimulationSettingsMap.end())
	{  // we need to update travel time and agent file

		g_ExchangeRealTimeSimulationData(DayNo,time_clock_in_min);


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
			if(pVeh->m_PricingType == 4)
			{
				//transit route simulation

				pVeh->m_bLoaded = true;
				if(pVeh->m_NodeSize >=2)
					pVeh->m_bComplete = true;
				//skip this vehicle in road simulation
				g_LastLoadedVehicleID = pVeh->m_VehicleID ;	
				continue;


			}
			if(pVeh->m_NodeSize >=2)  // with physical path
			{
				int FirstLink =pVeh->m_NodeAry[0].LinkNo;

				DTALink* p_link = g_LinkVector[FirstLink];

				if(p_link !=NULL)
				{
					struc_vehicle_item vi;
					vi.veh_id = pVeh->m_VehicleID ;
					g_LastLoadedVehicleID = pVeh->m_VehicleID ;

					vi.time_stamp = pVeh->m_DepartureTime + p_link->GetFreeMovingTravelTime(TrafficFlowModelFlag, DayNo, CurrentTime);  // unit: min


					vi.veh_next_node_number = -1;  // initialization 
					if(pVeh->m_NodeSize >=3)   // fetch the next link's downstream node number
					{
						int NextLink =pVeh->m_NodeAry[1].LinkNo;

						DTALink* p_next_link = g_LinkVector[NextLink];
						vi.veh_next_node_number = p_next_link->m_ToNodeNumber ;
					}


					pVeh->m_bLoaded = true;
					pVeh->m_SimLinkSequenceNo = 0;

					if(debug_flag && vi.veh_id == vehicle_id_trace )
						TRACE("Step 1: Load vhc %d to link %d with departure time time %5.2f -> %5.2f\n",vi.veh_id,FirstLink,pVeh->m_DepartureTime,vi.time_stamp);

					p_link->LoadingBuffer.push_back(vi);  // need to fine-tune
					g_Number_of_GeneratedVehicles +=1;
					g_NetworkMOEAry[time_stamp_in_min].Flow_in_a_min +=1;
					g_NetworkMOEAry[time_stamp_in_min].CumulativeInFlow = g_Number_of_GeneratedVehicles;

					// access VMS information from the first link: 
					// condition 1: VMS is active on the first link
					// condition 2: Radio message is active throughout the network
					int IS_id  = p_link->GetInformationResponseID(DayNo,CurrentTime);
					if(IS_id >= 0)
					{
						if( g_VehicleMap[vi.veh_id]->GetRandomRatio()*100 < p_link->MessageSignVector [IS_id].ResponsePercentage )
						{  // vehicle rerouting

							g_AgentBasedVMSPathAdjustmentWithRealTimeInfo(vi.veh_id ,CurrentTime);
						}

					}

					if(bRadioMessageActive)
					{
						if( g_VehicleMap[vi.veh_id]->GetRandomRatio()*100 < network_wide_RadioMessageResponsePercentage )
						{  // vehicle rerouting

							g_AgentBasedVMSPathAdjustmentWithRealTimeInfo(vi.veh_id ,CurrentTime);
						}
						// check if a radio message has been enabled
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
	if(simulation_time_interval_no>=trace_step)
		g_ProgramTrace("loading buffer");

	int link_size = g_LinkVector.size();
#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{
		DTALink * pLink = g_LinkVector[li];

		if(pLink->m_FromNodeNumber == 14890 && pLink->m_ToNodeNumber == 14966 && CurrentTime>=600 )
		{
			TRACE("");
		}

		while(pLink->LoadingBuffer.size() >0 && pLink->GetNumberOfLanes(DayNo,CurrentTime)>0.01)  // no load vehicle into a blocked link
		{
			struc_vehicle_item vi = pLink->LoadingBuffer.front();
			// we change the time stamp here to reflect the actual loading time into the network, especially for blocked link
			if( g_floating_point_value_less_than_or_eq_comparison(vi.time_stamp , CurrentTime))
				vi.time_stamp = CurrentTime;

			if(debug_flag && vi.veh_id == vehicle_id_trace )
				TRACE("Step 1: Time %f: Load vhc %d from buffer to physical link %d->%d\n",CurrentTime,vi.veh_id,pLink->m_FromNodeNumber,pLink->m_ToNodeNumber);

			int NumberOfVehiclesOnThisLinkAtCurrentTime = (int)(pLink->CFlowArrivalCount - pLink->CFlowDepartureCount);

			// determine link in capacity 
			float AvailableSpaceCapacity = pLink->m_VehicleSpaceCapacity - NumberOfVehiclesOnThisLinkAtCurrentTime;

			// if we use BPR function, no density constraint is imposed --> TrafficFlowModelFlag == 0
			if(TrafficFlowModelFlag== tfm_BPR || AvailableSpaceCapacity >= max(2,pLink->m_VehicleSpaceCapacity*(1-g_MaxDensityRatioForVehicleLoading)))  // at least 10% remaining capacity or 2 vehicle space is left
			{
				pLink->LoadingBuffer.pop_front ();


				if(pLink->m_LeftTurn_NumberOfLanes >=1 && pLink->m_LeftTurn_DestNodeNumber == vi.veh_next_node_number  )
				{
					pLink->LeftEntrance_Queue.push_back(vi);

					int t_link_arrival_time = vi.time_stamp ; 
					if(t_link_arrival_time < pLink->m_LinkMOEAry.size())
					{
					pLink->m_LinkMOEAry[t_link_arrival_time].IntervalLeftArrivalCount++;
					}
						
				}else
				{
					pLink->EntranceQueue.push_back(vi);
				}


				pLink->CFlowArrivalCount +=1;


				int pricing_type = g_VehicleMap[vi.veh_id]->m_PricingType ;
				pLink->CFlowArrivalCount_PricingType[pricing_type] +=1;
				pLink->CFlowArrivalRevenue_PricingType[pricing_type] += pLink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);

				DTAVehicle* pVehicle  = g_VehicleMap[vi.veh_id];

				// mark the actual leaving time from the loading buffer, so that we can calculate the exact time for traversing the physical net
				pVehicle->m_LeavingTimeFromLoadingBuffer = CurrentTime;
				pLink->m_LoadingBufferWaitingTime+= (CurrentTime - pVehicle->m_DepartureTime );

				//if(pLink->m_LoadingBufferWaitingTime > 0.5f)
				//{
				//	TRACE(" buffer waiting");
				//}

				// add cumulative flow count to vehicle


				if(pLink->m_FromNodeNumber == 5 && pLink->m_ToNodeNumber == 6)
				{
					TRACE("");
				}
				pVehicle->m_TollDollarCost += pLink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);


				if(debug_flag && vi.veh_id == vehicle_id_trace )
					TRACE("Step 1: Time %f: Capacity available, remove vhc %d from buffer to physical link %d->%d\n",CurrentTime,vi.veh_id,pLink->m_FromNodeNumber,pLink->m_ToNodeNumber);

			}else
			{
				break;  // physical road is too congested, wait for next time interval to load
			}

		}
	}

	// step 2: move vehicles from EntranceQueue To ExitQueue, if ReadyTime <= CurrentTime)
	if(simulation_time_interval_no>=trace_step)
		g_ProgramTrace("move vehicles from EntranceQueue To ExitQueue");


#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{

		DTALink * pLink = g_LinkVector[li];



		while(pLink->EntranceQueue.size() >0)  // if there are vehicles in the entrance queue
		{

			struc_vehicle_item vi = pLink->EntranceQueue.front();
			double PlannedArrivalTime = vi.time_stamp;

			if(pLink->m_FromNodeNumber == 9 && pLink->m_ToNodeNumber == 10 )
			{
				TRACE("Time %f, PlannedArrivalTime = %f,  CurrentTime = %f, \n",
					CurrentTime, PlannedArrivalTime, CurrentTime );
			
			}

			if( g_floating_point_value_less_than_or_eq_comparison(PlannedArrivalTime, CurrentTime) )  // link (downstream) arrival time within the simulation time interval
			{
				pLink->EntranceQueue.pop_front ();
				pLink->ExitQueue.push_back(vi);

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


		// left-turn entrance queue 
		while(pLink->LeftEntrance_Queue.size() >0)  // if there are vehicles in the left-turn entrance queue
		{

			struc_vehicle_item vi = pLink->LeftEntrance_Queue.front();
			double PlannedArrivalTime = vi.time_stamp;

			if(debug_flag && pLink->m_FromNodeNumber == 1 &&  pLink->m_ToNodeNumber == 5 && CurrentTime >=10)
			{
				TRACE("Step 3: Time %f, Link: %d -> %d: entrance queue length: %d, exit queue length %d\n",
					CurrentTime, pLink->m_FromNodeNumber , pLink->m_ToNodeNumber,
					pLink->LeftEntrance_Queue.size(),pLink->LeftExit_Queue.size());
			}


			if(pLink->m_FromNodeNumber == 9 && pLink->m_ToNodeNumber == 10 )
			{
				TRACE("Time %f, PlannedArrivalTime = %f,  CurrentTime = %f, \n",
					CurrentTime, PlannedArrivalTime, CurrentTime );
			
			}

			if( g_floating_point_value_less_than_or_eq_comparison(PlannedArrivalTime, CurrentTime) )  // link (downstream) arrival time within the simulation time interval
			{
				pLink->LeftEntrance_Queue.pop_front ();
				pLink->LeftExit_Queue.push_back(vi);

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

	if(TrafficFlowModelFlag == tfm_BPR) // BPR function 
	{
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{

			// under BPR function, we do not impose the physical capacity constraint but use a BPR link travel time to move vehicles along links	
			g_LinkVector[li]->LinkOutCapacity = 99999;
			g_LinkVector[li]->LinkLeftOutCapacity = 99999;
			g_LinkVector[li]->LinkInCapacity = 99999;

		}
	}
	else // queueing model
	{

#pragma omp parallel for
		for(int li = 0; li< link_size; li++)
		{

			DTALink* pLink = g_LinkVector[li];
			float PerHourCapacity = pLink->GetHourlyPerLaneCapacity(CurrentTime);  // static capacity from BRP function
			float PerHourCapacityAtCurrentSimulatioInterval = PerHourCapacity;


			// freeway 

			if(g_LinkTypeMap [pLink->m_link_type].IsFreeway())  
			{
				if(g_StochasticCapacityMode &&  pLink->m_StochaticCapcityFlag >=1 && simulation_time_interval_no%150 ==0)  // update stochastic capacity every 15 min
				{
					bool QueueFlag = false;

					if(pLink->ExitQueue.size() > 0)  // vertical exit queue
						QueueFlag = true;

					PerHourCapacityAtCurrentSimulatioInterval = GetStochasticCapacity(QueueFlag,pLink->GetHourlyPerLaneCapacity(CurrentTime));
				}
			}


			if(pLink->m_FromNodeNumber == 1 && pLink->m_ToNodeNumber == 5 && CurrentTime>=0 )
			{
				TRACE("");
			}

			if(pLink->m_bSignalizedArterialType == true && g_SignalRepresentationFlag == signal_model_movement_effective_green_time)
			{


					if(pLink->m_EffectiveGreenTime_In_Second==0)
					{
						cout <<"Link " << pLink->m_FromNodeNumber << " ->" <<  pLink->m_ToNodeNumber  << " has an effective green time of 0 but for simulated vehicles. Please check the movement input data, and check if the movement has been prevented." << endl;
						
						g_ProgramStop();
					}
				//by default, we handle through turn here
				PerHourCapacityAtCurrentSimulatioInterval = GetTimeDependentCapacityAtSignalizedIntersection( 
					pLink->m_DownstreamCycleLength_In_Second, 
					pLink->m_EffectiveGreenTime_In_Second ,
					pLink->m_GreenStartTime_In_Second,  
					pLink->m_DownstreamNodeSignalOffset_In_Second, 
					CurrentTime, 
					pLink->m_SaturationFlowRate_In_vhc_per_hour_per_lane);

				if(pLink->m_LeftTurn_NumberOfLanes >0) // with left_turn 
				{

					if(pLink->m_FromNodeNumber == 1 && CurrentTime >=1.3 )
					{      // under capacity, constrained by existing left queue size
						TRACE("%f, %d, queue = %d\n",CurrentTime,pLink-> LinkLeftOutCapacity, pLink->LeftExit_Queue.size() );
					}
					float PerHourLeftTurnCapacityAtCurrentSimulatioInterval = 
						GetTimeDependentCapacityAtSignalizedIntersection( 
						pLink->m_DownstreamCycleLength_In_Second,
						pLink->m_LeftTurn_EffectiveGreenTime_In_Second , /*left turn handling for effective green time*/
						pLink->m_LeftTurnGreenStartTime_In_Second ,
						pLink->m_DownstreamNodeSignalOffset_In_Second,
						CurrentTime,
						pLink->m_LeftTurn_SaturationFlowRate_In_vhc_per_hour_per_lane);  /*left turn handling for saturation flow rate*/ 

					float PerLinkLeftTurnCapacityCountAtCurrentSimulatioInterval = PerHourLeftTurnCapacityAtCurrentSimulatioInterval
						*g_DTASimulationInterval/60.0f*pLink->m_LeftTurn_NumberOfLanes;

					if(g_RandomizedCapacityMode)
					{
						pLink-> LinkLeftOutCapacity = g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(PerLinkLeftTurnCapacityCountAtCurrentSimulatioInterval,li);
					}else
					{
						float PrevCumulativeLeftOutCapacityCount = pLink->m_CumulativeLeftOutCapacityCount;
						pLink->m_CumulativeLeftOutCapacityCount+= PerLinkLeftTurnCapacityCountAtCurrentSimulatioInterval;
						pLink-> LinkLeftOutCapacity = (int)pLink->m_CumulativeLeftOutCapacityCount - (int) PrevCumulativeLeftOutCapacityCount;

					}

				}

			}


			// determine link out capacity 

			bool OutFlowFlag = true;

			float number_of_lanes  = pLink->GetNumberOfLanes(DayNo,CurrentTime,OutFlowFlag);
			float number_vehicles_per_simulation_interval = PerHourCapacityAtCurrentSimulatioInterval *g_DTASimulationInterval/60.0f * number_of_lanes; //60 --> cap per min --> unit # of vehicle per simulation interval

			float Capacity = number_vehicles_per_simulation_interval;
			// use integer number of vehicles as unit of capacity

			if(g_RandomizedCapacityMode)
			{
				pLink-> LinkOutCapacity = g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(Capacity,li);
			}else
			{

				pLink->m_CumulativeOutCapacityCount+= Capacity;  // floating point variable to record number of vehicles can be sent out based on floating point capacity

				pLink-> LinkOutCapacity = (int)(pLink->m_CumulativeOutCapacityCount) - pLink-> m_CumulativeOutCapacityCountAtPreviousInterval;

				if(pLink->m_FromNodeNumber == 9 && pLink->m_ToNodeNumber == 10 )
				{
					TRACE("\ntime %f, link capacity per simulation interval = %d, cumulative out capacity: %f, cumulative out capacity count: %d ", 
						CurrentTime, pLink-> LinkOutCapacity , pLink->m_CumulativeOutCapacityCount,  pLink-> m_CumulativeOutCapacityCountAtPreviousInterval);
				
				}

				pLink-> m_CumulativeOutCapacityCountAtPreviousInterval += pLink->LinkOutCapacity; // accumulate the # of out vehicles from the capacity, this is an inter number 


			}


			int NumberOfVehiclesOnThisLinkAtCurrentTime = (int)(pLink->CFlowArrivalCount - pLink->CFlowDepartureCount);

			float fLinkInCapacity = 99999.0;

			// TrafficFlowModelFlag == 1 -> point queue model
			if(TrafficFlowModelFlag >=2)  // apply spatial link in capacity for spatial queue models( with spillback and shockwave) 
			{
				// determine link in capacity 
				float AvailableSpaceCapacity = pLink->m_VehicleSpaceCapacity - NumberOfVehiclesOnThisLinkAtCurrentTime;

				if(g_LinkTypeMap[pLink->m_link_type] .IsFreeway())
					fLinkInCapacity = min (AvailableSpaceCapacity, 1800 *g_DTASimulationInterval/60.0f* pLink->GetNumberOfLanes(DayNo,CurrentTime)); 
				else // non freeway links
					fLinkInCapacity = AvailableSpaceCapacity;

				if(fLinkInCapacity<0)
					fLinkInCapacity = 0;


				//			TRACE(" time %5.2f, SC: %5.2f, MFR %5.2f\n",CurrentTime, AvailableSpaceCapacity, MaximumFlowRate);

				// the inflow capcaity is the minimum of (1) incoming maximum flow rate (determined by the number of lanes) and (2) available space capacty  on the link.
				// use integer number of vehicles as unit of capacity

				//if(pLink->m_FromNodeNumber == 9 && pLink->m_ToNodeNumber == 10 && CurrentTime>=434.15)
				//{
				//TRACE("");
				//}
				if((TrafficFlowModelFlag == tfm_newells_model||TrafficFlowModelFlag == tfm_newells_model_with_emissions)  && g_LinkTypeMap[pLink->m_link_type] .IsFreeway())  // Newell's model on freeway only
				{
					if(simulation_time_interval_no >=pLink->m_BackwardWaveTimeInSimulationInterval ) /// we only apply backward wave checking after the simulation time is later than the backward wave speed interval
					{

						int t_residual_minus_backwardwaveTime = max(0,simulation_time_interval_no - pLink->m_BackwardWaveTimeInSimulationInterval)% MAX_TIME_INTERVAL_ADCURVE ;

						float VehCountUnderJamDensity = pLink->m_Length * pLink->GetNumberOfLanes(DayNo,CurrentTime) *pLink->m_KJam;
						// when there is a capacity reduction, in the above model, we assume the space capacity is also reduced proportional to the out flow discharge capacity, 
						// for example, if the out flow capacity is reduced from 5 lanes to 1 lanes, say 10K vehicles per hour to 2K vehicles per hour, 
						// our model will also reduce the # of vehicles can be stored on the incident site by the equivalent 4 lanes. 
						// this might cause the dramatic speed change on the incident site, while the upstream link (with the original space capacity) will take more time to propapate the speed change compared to the incident link. 
						// to do list: we should add another parameter of space capacity reduction ratio to represent the fact that the space capacity reduction magnitude is different from the outflow capacity reduction level,
						// particularly for road construction areas on long links, with smooth barrier on the merging section. 
						int N_Arrival_Now_Constrainted = (int)(pLink->m_CumuDeparturelFlow[t_residual_minus_backwardwaveTime] + VehCountUnderJamDensity);  //pLink->m_Length 's unit is mile
						int t_residual_minus_1 = max(0, simulation_time_interval_no - 1)% MAX_TIME_INTERVAL_ADCURVE ;

						ASSERT(t_residual_minus_1>=0);
						ASSERT(t_residual_minus_1<MAX_TIME_INTERVAL_ADCURVE);
						int N_Now_minus_1 = (int)pLink->m_CumuArrivalFlow[t_residual_minus_1];
						int Flow_allowed = N_Arrival_Now_Constrainted - N_Now_minus_1;

						//TRACE("\ntime %d, D:%d,A%d",simulation_time_interval_no,pLink->m_CumuDeparturelFlow[t_residual_minus_1],pLink->m_CumuArrivalFlow[t_residual_minus_1]);

						if(Flow_allowed < 0)
							Flow_allowed  = 0;

						if( fLinkInCapacity  > Flow_allowed)
						{
							fLinkInCapacity = Flow_allowed;

							if(Flow_allowed == 0 && N_Arrival_Now_Constrainted > 0)
							{
								if ( pLink->m_JamTimeStamp > CurrentTime)
									pLink->m_JamTimeStamp = CurrentTime;

								//							g_LogFile << "Queue spillback"<< CurrentTime <<  g_NodeVector[pLink->m_FromNodeID] << " -> " <<	g_NodeVector[pLink->m_ToNodeID] << " " << pLink->LinkInCapacity << endl;

								// update traffic state
								int CurrentTime_int = (int)(CurrentTime);

								if(CurrentTime_int >= pLink->m_LinkMOEAry.size())
								{
									cout << "CurrentTime_int >= pLink->m_LinkMOEAry.size()";
									g_ProgramStop();
								}

								pLink->m_LinkMOEAry[CurrentTime_int].TrafficStateCode = 2;  // 2: fully congested

								//							TRACE("Queue spillback at %d -> %d\n", g_NodeVector[pLink->m_FromNodeID], g_NodeVector[pLink->m_ToNodeID]);
							}
						}
					}
				}


				float InflowRate = number_vehicles_per_simulation_interval *g_MinimumInFlowRatio;

				if(CurrentTime >= (g_DemandLoadingEndTimeInMin + g_RelaxInFlowConstraintAfterDemandLoadingTime))  // g_RelaxInFlowConstraintAfterDemandLoadingTime min after demand loading period, do not apply in capacity constraint
				{
					InflowRate  = number_vehicles_per_simulation_interval;
				}

				if(fLinkInCapacity < InflowRate)  // minimum inflow capacity to keep the network flowing
				{
					fLinkInCapacity = InflowRate; 
				}

			}

			// finally we convert the floating-point capacity to integral capacity in terms of number of vehicles


			if(g_RandomizedCapacityMode)
			{

				pLink-> LinkInCapacity= g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(fLinkInCapacity,li);
				// new version with uniform inflow capa distribution 
			}else
			{


				pLink-> m_CumulativeInCapacityCount += fLinkInCapacity;
				pLink-> LinkInCapacity = (int) pLink-> m_CumulativeInCapacityCount - pLink->m_CumulativeInCapacityCountAtPreviousInterval;

				pLink->m_CumulativeInCapacityCountAtPreviousInterval += pLink->LinkInCapacity;


				//if(pLink->m_FromNodeNumber == 9 && pLink->m_ToNodeNumber == 10 && CurrentTime>=434)
				//{
				//	TRACE("\ntime %f, link out capacity per simulation interval = %d, cumulative out capacity: %f, cumulative in capacity count: %d ", 
				//		CurrentTime, pLink-> LinkInCapacity , pLink->m_CumulativeInCapacityCount,  pLink-> m_CumulativeInCapacityCountAtPreviousInterval);
				//
				//}

			}
			//

			//overwrite ramp in flow capacity
			if(g_LinkTypeMap[pLink->m_link_type].IsRamp ()) 
			{

				if(pLink->m_bOnRampType == true && g_settings.use_point_queue_model_for_on_ramps)
				{
					pLink-> LinkInCapacity =   99999; // reset in flow capacity constraint 
				}

				if(pLink->m_bOnRampType == false && g_settings.use_point_queue_model_for_off_ramps)
				{
					pLink-> LinkInCapacity =   99999; // reset outflow flow capacity constraint 
				}
			}


			if(debug_flag && pLink->m_FromNodeNumber ==60306 &&  pLink->m_ToNodeNumber ==54256 && CurrentTime>=5 )
			{
				TRACE("Step 3: Time %f, Link: %d -> %d: Incapacity int= %d, %f, OutCapacity: %d\n", CurrentTime, g_NodeVector[pLink->m_FromNodeID].m_NodeNumber , g_NodeVector[pLink->m_ToNodeID].m_NodeNumber ,pLink-> LinkInCapacity, fLinkInCapacity, pLink-> LinkOutCapacity);
			}
		}


		// distribute link in capacity to different incoming links
#pragma omp parallel for
		for(int li = 0; li< link_size; li++)
		{
			DTALink* pLink = g_LinkVector[li];
			unsigned int il;


			if(pLink -> m_bMergeFlag >= 1)
			{
				int TotalInFlowCount = 0;

				for( il = 0; il< pLink->MergeIncomingLinkVector.size(); il++)
				{
					TotalInFlowCount+= g_LinkVector [ pLink->MergeIncomingLinkVector[il].m_LinkNo]->ExitQueue.size();  // count vehiciles waiting at exit queue

				}

				if(TotalInFlowCount > pLink-> LinkInCapacity)  // total demand > supply
				{


					if(pLink -> m_bMergeFlag == 1 ||  pLink -> m_bMergeFlag == 2)  // merge with mulitple freeway/onramp links only, or for freeway/on-ramp merge when not using freeval model
					{


						if( pLink->MergeIncomingLinkVector.size() == 2) // two incoming links
						{

							for( il = 0; il< pLink->MergeIncomingLinkVector.size(); il++)
							{
								int il_other_link = 1- il; // il ==0 -> il_other_link = 1;, il ==1 -> il_other_link = 0;
								int demand_on_other_link =  g_LinkVector [pLink->MergeIncomingLinkVector[il_other_link].m_LinkNo]->ExitQueue.size();
								pLink->MergeIncomingLinkVector[il].m_LinkInRemainingCapaityPerSimuInterval =
									pLink-> LinkInCapacity -  demand_on_other_link;
								// remaining capacity = total in capacity  - demand on other link
							}

						}

						// default to link in capacity ratio 
						for( il = 0; il< pLink->MergeIncomingLinkVector.size(); il++)
						{

							float capacity1 = pLink-> LinkInCapacity * pLink->MergeIncomingLinkVector[il].m_LinkInCapacityRatio; 
							// capacity from proportional lane assignment 

							float capacity2 = pLink->MergeIncomingLinkVector[il].m_LinkInRemainingCapaityPerSimuInterval; 
							// remaining residual capacity from total inflow capacity 

							float capacity3 = g_LinkVector [pLink->MergeIncomingLinkVector[il].m_LinkNo]->LinkOutCapacity;
							//// outflow capacity of the incoming link 
							float LinkOutCapacity = min(capacity3,max( capacity1, capacity2));

							//							float capacity3 = g_LinkVector [pLink->MergeIncomingLinkVector[il].m_LinkNo]->LinkOutCapacity;
							// outflow capacity of the incoming link 

							// priority based model is used here from  Daganzo's Priority-Based Merge Model  (1995)
							int LinkOutCapacity_int =  0;

							if(g_RandomizedCapacityMode)
							{
								LinkOutCapacity_int = g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(LinkOutCapacity,li);
							}else
							{
								float PrevCumulativeMergeOutCapacityCount = g_LinkVector [pLink->MergeIncomingLinkVector[il].m_LinkNo]->m_CumulativeMergeOutCapacityCount;
								g_LinkVector [pLink->MergeIncomingLinkVector[il].m_LinkNo]->m_CumulativeMergeOutCapacityCount+= LinkOutCapacity;
								LinkOutCapacity_int = (int)g_LinkVector [pLink->MergeIncomingLinkVector[il].m_LinkNo]->m_CumulativeMergeOutCapacityCount - 
									(int) PrevCumulativeMergeOutCapacityCount;
							}
							g_LinkVector [pLink->MergeIncomingLinkVector[il].m_LinkNo]->LinkOutCapacity = LinkOutCapacity_int;

						} // for each incoming link
					}  //   merge with mulitple freeway/onramp links only, or for freeway/on-ramp merge when not using freeval model
				}  // total demand > supply
			} // m_bMergeFlag >= 1
		}  // 		// distribute link in capacity to different incoming links

	}  // // queueing model


	// step 4: move vehicles from ExitQueue to next link's EntranceQueue, if there is available capacity
	// NewTime = ReadyTime + FFTT(next link)
	if(simulation_time_interval_no>=trace_step)
		g_ProgramTrace("step 4: move vehicles from ExitQueue to next link's EntranceQueue");

	// step 4.1: calculate movement capacity per simulation interval for movements defined in input_movement.csv

	int node_size = g_NodeVector.size();

	//for each node, we scan each incoming link in a randomly sequence, based on simulation_time_interval_no
	if(simulation_time_interval_no>=trace_step)
		g_ProgramTrace("step 5: for each node, we scan each incoming link in a randomly sequence");

#pragma omp parallel for
	for(int node = 0; node < node_size; node++)
	{

		int NextLink;
		DTALink* p_Nextlink;

		int IncomingLinkSize = g_NodeVector[node].m_IncomingLinkVector.size();
		int incoming_link_count = 0;

		int incoming_link_sequence = simulation_time_interval_no%max(1,IncomingLinkSize);  // random start point


		while(incoming_link_count < IncomingLinkSize)
		{

			int li = g_NodeVector[node].m_IncomingLinkVector[incoming_link_sequence];

			DTALink* pLink = g_LinkVector[li];
			if(debug_flag && (pLink->m_FromNodeNumber  == 56619 && pLink->m_ToNodeNumber == 60306) && CurrentTime >=5)
			{
				TRACE("Step 3: Time %f, Link: %d -> %d: tracing queue: capacity: %d, queue size %d \n", 
					CurrentTime, g_NodeVector[pLink->m_FromNodeID].m_NodeNumber , g_NodeVector[pLink->m_ToNodeID].m_NodeNumber,
					pLink->LinkOutCapacity, pLink->ExitQueue.size() );
			}


			// vehicle_out_count is the minimum of LinkOutCapacity and ExitQueue Size

			int vehicle_out_count = pLink->LinkOutCapacity;

			//			g_LogFile << "link out capaity:"<< CurrentTime << " "  << g_NodeVector[pLink->m_FromNodeID] << " ->" << g_NodeVector[pLink->m_ToNodeID]<<" Cap:" << vehicle_out_count<< "queue:" << pLink->ExitQueue.size() << endl;

			if(pLink->ExitQueue.size() <= pLink-> LinkOutCapacity )
			{      // under capacity, constrained by existing queue
				vehicle_out_count = pLink->ExitQueue.size();
			}

			//////////////left turn handling  ///////////////////////
			int vehicle_left_out_count = pLink->LinkLeftOutCapacity;

			//			g_LogFile << "link out capaity:"<< CurrentTime << " "  << g_NodeVector[pLink->m_FromNodeID] << " ->" << g_NodeVector[pLink->m_ToNodeID]<<" Cap:" << vehicle_out_count<< "queue:" << pLink->ExitQueue.size() << endl;

			if(pLink->m_FromNodeNumber == 1 && CurrentTime >=30 )
			{      // under capacity, constrained by existing left queue size
				TRACE("%f, %d, queue = %d\n",CurrentTime,pLink-> LinkLeftOutCapacity, pLink->LeftExit_Queue.size() );
			}

			///////////////////////// end of left-turn handling ///////////////////////

			int through_blocking_count  = 0;
			int left_blocking_count  = 0;
			int blocking_node_number = 0;

			bool FetchVehicleFromLeftTurnQueue = false;

			if(debug_flag && pLink->m_FromNodeNumber == 1 &&  pLink->m_ToNodeNumber == 5 && CurrentTime >=30)
			{
				TRACE("Step 3: Time %f, Link: %d -> %d: entrance queue length: %d, exit queue length %d\n",
					CurrentTime, pLink->m_FromNodeNumber , pLink->m_ToNodeNumber,
					pLink->LeftEntrance_Queue.size(),pLink->LeftExit_Queue.size());
			}

			// we first discharge through queue and then left-turn queue 
			while( (pLink->ExitQueue.size() >0 && vehicle_out_count >0) || (pLink->LeftExit_Queue.size() >0 && vehicle_left_out_count >0))  // either condition 1 or condition 2 is met
			{
				struc_vehicle_item vi;


				if(pLink->ExitQueue.size() >0 && vehicle_out_count >0 && through_blocking_count == 0) // there is through capacity // condition 1  // not blocked 
				{
					vi = pLink->ExitQueue.front();
					FetchVehicleFromLeftTurnQueue = false;
				}else if(pLink->LeftExit_Queue.size() >0 && vehicle_left_out_count >0 )// condition 2 
				{
					vi = pLink->LeftExit_Queue.front();
					FetchVehicleFromLeftTurnQueue = true;

				}else
				{ // there is not left out capacity 
					break;

				}


				int vehicle_id = vi.veh_id;

				// record arrival time at the downstream node of current link
				int link_sequence_no = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo;

				int t_link_arrival_time=0;

				if(link_sequence_no >=1)
				{
					t_link_arrival_time= int(g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no-1].AbsArrivalTimeOnDSN);
				}else
				{
					t_link_arrival_time = int(g_VehicleMap[vehicle_id]->m_DepartureTime);
				}
				// not reach destination yet

				int number_of_links = g_VehicleMap[vehicle_id]->m_NodeSize-1;

				if(vehicle_id == 60 && CurrentTime > 100)
				{
					TRACE("..");
				}

				if(g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo < number_of_links-1)  // not reach destination yet
				{

					// advance to next link

					if(vehicle_id == vehicle_id_trace)
						TRACE("simulation link sequence no. %d",g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo);

					NextLink = g_VehicleMap[vehicle_id]->m_NodeAry[g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo+1].LinkNo;
					p_Nextlink = g_LinkVector[NextLink];

					if(p_Nextlink==NULL)
					{
						TRACE("Error at vehicle %d,",vehicle_id);
						ASSERT(p_Nextlink!=NULL);
					}


					//
					// test if movement capacity available
					//DTANodeMovement movement_element;
					//string movement_id;

					if(p_Nextlink->m_FromNodeNumber == 57547 && p_Nextlink->m_ToNodeNumber == 54720)
					{

						TRACE("");

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
						g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN = ArrivalTimeOnDSN;
						float TravelTime = 0;

						if(link_sequence_no >=1)
						{
							TravelTime= g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
								g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no-1].AbsArrivalTimeOnDSN;
						}else
						{
							TravelTime= g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
								g_VehicleMap[vehicle_id]->m_DepartureTime ;

						}
						g_VehicleMap[vehicle_id]->m_Delay += (TravelTime-pLink->m_FreeFlowTravelTime);
						g_VehicleMap[vehicle_id]->m_TripFFTT+=pLink->m_FreeFlowTravelTime;

						// finally move to next link
						g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo+1;

						// access VMS information here! p_Nextlink is now the current link
						int IS_id  = p_Nextlink->GetInformationResponseID(DayNo,CurrentTime);
						if(IS_id >= 0)
						{
							if( g_VehicleMap[vi.veh_id]->GetRandomRatio()*100 < p_Nextlink->MessageSignVector [IS_id].ResponsePercentage )
							{  // vehicle rerouting

								g_AgentBasedVMSPathAdjustmentWithRealTimeInfo(vi.veh_id ,CurrentTime);
							}

						}

						//check if radio message is active 
						if(bRadioMessageActive)
						{
							if( g_VehicleMap[vi.veh_id]->GetRandomRatio()*100 < network_wide_RadioMessageResponsePercentage )
							{  // vehicle rerouting

								g_AgentBasedVMSPathAdjustmentWithRealTimeInfo(vi.veh_id ,CurrentTime);
							}
							// check if a radio message has been enabled
						}



						vi.veh_id = vehicle_id;

						float FFTT = p_Nextlink->GetFreeMovingTravelTime(TrafficFlowModelFlag,DayNo,CurrentTime);

						if(vehicle_id==86)
						{
							TRACE("");
						}
						vi.time_stamp = ArrivalTimeOnDSN + FFTT;

						vi.veh_next_node_number  = -1;  // initialization 

						if(g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo < number_of_links-2)  // not reach the previous link before the destination yet
						{

							int NextLinkOfNextLink = g_VehicleMap[vehicle_id]->m_NodeAry[g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo+2].LinkNo;
							DTALink* p_NextNextlink = g_LinkVector[NextLinkOfNextLink];

							if(p_NextNextlink==NULL)
							{
								TRACE("Error at vehicle %d,",vehicle_id);
								ASSERT(p_Nextlink!=NULL);
							}

							vi.veh_next_node_number  = p_NextNextlink->m_ToNodeNumber ;
						}

						// remark: when - TimeOnNextLink < 0, it means there are few seconds of left over on current link, which should be spent on next link

						///

						if(debug_flag && p_Nextlink->m_FromNodeNumber == 34 &&  p_Nextlink->m_ToNodeNumber == 30 && CurrentTime >=860)
						{
							TRACE("Step 4: Time %f, Link: %d -> %d: vi %d, exit time: %f, FFTT = %f\n",
								CurrentTime, p_Nextlink->m_FromNodeNumber , p_Nextlink->m_ToNodeNumber,
								vi.veh_id , vi.time_stamp, FFTT );
						}

						// branch for left-turn queue

						if( p_Nextlink->m_LeftTurn_NumberOfLanes >=1 && p_Nextlink->m_LeftTurn_DestNodeNumber == vi.veh_next_node_number)
						{

							p_Nextlink->LeftEntrance_Queue.push_back(vi);

					int t_link_arrival_time = vi.time_stamp ; 
					if(t_link_arrival_time < p_Nextlink->m_LinkMOEAry.size())
					{
					p_Nextlink->m_LinkMOEAry[t_link_arrival_time].IntervalLeftArrivalCount++;
					}

						}else
						{
							p_Nextlink->EntranceQueue.push_back(vi);
						}


						// condition 1

						if(FetchVehicleFromLeftTurnQueue == false)
						{
							pLink->ExitQueue.pop_front();
						}else // condition 2
						{
							pLink->LeftExit_Queue.pop_front();

							int t_link_departure_time = vi.time_stamp ; 
							if(t_link_departure_time < pLink->m_LinkMOEAry.size())
							{
							pLink->m_LinkMOEAry[t_link_departure_time].IntervalLeftDepartureCount ++;
							}

						}



						p_Nextlink->CFlowArrivalCount +=1;
						pLink->CFlowDepartureCount +=1;

						int pricing_type = g_VehicleMap[vi.veh_id]->m_PricingType ;
						p_Nextlink->CFlowArrivalCount_PricingType[pricing_type] +=1;
						p_Nextlink->CFlowArrivalRevenue_PricingType[pricing_type] += p_Nextlink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);

						DTAVehicle* pVehicle = g_VehicleMap[vi.veh_id];



						pVehicle->m_TollDollarCost += p_Nextlink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);


						if(p_Nextlink->CFlowArrivalCount !=  (p_Nextlink->CFlowArrivalCount_PricingType[1]+ p_Nextlink->CFlowArrivalCount_PricingType[2] + p_Nextlink->CFlowArrivalCount_PricingType[3] + p_Nextlink->CFlowArrivalCount_PricingType[4]))
						{
							//						TRACE("error!");
						}

						if(t_link_arrival_time < pLink->m_LinkMOEAry.size())
						{
							pLink->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime  += TravelTime;
							pLink->m_LinkMOEAry[t_link_arrival_time].TotalFlowCount +=1;
						}



						pLink-> departure_count +=1;
						pLink-> total_departure_based_travel_time += TravelTime;

						//										TRACE("time %d, total travel time %f\n",t_link_arrival_time, pLink->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime);

						p_Nextlink->LinkInCapacity -=1; // reduce available space capacity by 1




					}else
					{
						if(FetchVehicleFromLeftTurnQueue == false)
							through_blocking_count++;
						else
						{
							left_blocking_count++;
							break; // we do not need to futher examine vehicles at the front of the left turn queue, as there is no capacity available 
						}



						blocking_node_number = p_Nextlink->m_ToNodeNumber ;  // overwrite the blocking node number
						// no capcity, do nothing, and stay in the vertical exit queue

						if(TrafficFlowModelFlag == 4)  // spatial queue with shock wave and FIFO principle: FIFO, then there is no capacity for one movement, the following vehicles cannot move even there are capacity
						{
							break;
						}

					}

					//			TRACE("move veh %d from link %d link %d %d\n",vehicle_id,pLink->m_LinkNo, p_Nextlink->m_LinkNo);


				}else
				{ 

					// reach destination, increase the counter.

					float ArrivalTimeOnDSN = CurrentTime;

					// update statistics for traveled link
					int link_sequence_no = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo;
					g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN = ArrivalTimeOnDSN;


					g_VehicleMap[vehicle_id]->m_TimeToRetrieveInfo = 99999; // no more update
					float TravelTime = 0;

					if(link_sequence_no >=1)
					{
						TravelTime= g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
							g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no-1].AbsArrivalTimeOnDSN;
					}else
					{
						TravelTime= g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
							g_VehicleMap[vehicle_id]->m_DepartureTime ;

					}
					g_VehicleMap[vehicle_id]->m_Delay += (TravelTime-pLink->m_FreeFlowTravelTime);


					// finally move to next link
					g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo+1;


					g_VehicleMap[vehicle_id]->m_ArrivalTime = ArrivalTimeOnDSN;

					g_VehicleMap[vehicle_id]->m_TripTime = g_VehicleMap[vehicle_id]->m_ArrivalTime - g_VehicleMap[vehicle_id]->m_DepartureTime;
					g_VehicleMap[vehicle_id]->m_TravelTime  = g_VehicleMap[vehicle_id]->m_ArrivalTime - g_VehicleMap[vehicle_id]->m_LeavingTimeFromLoadingBuffer ;

					if(debug_flag && vi.veh_id == vehicle_id_trace )
					{
						TRACE("Step 4: time %f, target vehicle reaches the destination, vehicle trip time: %f, # of links  = %d\n",
							ArrivalTimeOnDSN, g_VehicleMap[vehicle_id]->m_TripTime , 
							g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo);

						vehicle_id_trace = -1; // not tracking anymore 
					}

					g_VehicleMap[vehicle_id]->m_bComplete = true;
					int OriginDepartureTime = (int)(g_VehicleMap[vehicle_id]->m_DepartureTime );
#pragma omp critical
					{
						g_Number_of_CompletedVehicles +=1;
						g_NetworkMOEAry[time_stamp_in_min].CumulativeOutFlow = g_Number_of_CompletedVehicles;
						g_NetworkMOEAry[OriginDepartureTime].AbsArrivalTimeOnDSN_in_a_min +=g_VehicleMap[vehicle_id]->m_TripTime;
						g_NetworkMOEAry[OriginDepartureTime].TotalFreeFlowTravelTime +=g_VehicleMap[vehicle_id]->m_TripFFTT ;
						g_NetworkMOEAry[OriginDepartureTime].TotalDistance +=g_VehicleMap[vehicle_id]->m_Distance  ;
						g_NetworkMOEAry[OriginDepartureTime].TotalBufferWaitingTime += (g_VehicleMap[vehicle_id]->m_TripTime - g_VehicleMap[vehicle_id]->m_TravelTime);


					}

					pLink->CFlowDepartureCount +=1;

					if(pLink->m_FromNodeNumber == 6 && pLink->m_ToNodeNumber == 4 && CurrentTime >60)
					{
						TRACE("");
					}
					pLink-> departure_count +=1;
					pLink-> total_departure_based_travel_time += TravelTime;


					if(pLink->m_FromNodeNumber == 6 && pLink->m_ToNodeNumber == 4 && CurrentTime>=60)
					{
						TRACE("current time %f, travel time = %f @ %d\n", CurrentTime,TravelTime, t_link_arrival_time);
					}

					if(t_link_arrival_time < pLink->m_LinkMOEAry.size())
					{
						pLink->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime  += TravelTime;
						pLink->m_LinkMOEAry[t_link_arrival_time].TotalFlowCount += 1;
					}


					// when we are about to reach destination, we do not put vehicles to the left-turn queue, as there is no "next link of next link". 
					pLink->ExitQueue.pop_front();

				}

				// we deduct the out flow counter here 

				if(FetchVehicleFromLeftTurnQueue == false)
					vehicle_out_count--;
				else
					vehicle_left_out_count--;


			}


			if(g_OutputLinkCapacityFlag ==1 && CurrentTime >= g_OutputLinkCapacityStarting_Time && CurrentTime <= g_OutputLinkCapacityEnding_Time)
			{
				DTALinkOutCapacity element(CurrentTime,pLink->LinkOutCapacity,pLink->ExitQueue.size(), through_blocking_count, blocking_node_number , pLink->LinkLeftOutCapacity,pLink->LeftExit_Queue.size());
				pLink->m_OutCapacityVector .push_back (element);
			}

			incoming_link_count++;
			incoming_link_sequence = (incoming_link_sequence+1) % IncomingLinkSize;  // increase incoming_link_sequence by 1 within IncomingLinkSize
		}  // for each incoming link
	} // for each node

	//	step 5: statistics collection
	if(simulation_time_interval_no>=trace_step)
		g_ProgramTrace("step 5: step 5: statistics collection");

#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{

		// Cumulative flow counts

		int t_residual = simulation_time_interval_no % MAX_TIME_INTERVAL_ADCURVE;

		DTALink* pLink = g_LinkVector[li];
		pLink->m_CumuArrivalFlow[t_residual] = pLink->CFlowArrivalCount;
		pLink->m_CumuDeparturelFlow[t_residual] = pLink->CFlowDepartureCount;

		if(simulation_time_interval_no%g_number_of_intervals_per_min==0 )  // per min statistics
		{
			pLink->VehicleCount = pLink->CFlowArrivalCount - pLink->CFlowDepartureCount;

			// queue is the number of vehicles at the end of simulation interval

			pLink->m_LinkMOEAry [time_stamp_in_min].ExitQueueLength = pLink->ExitQueue.size(); 

			pLink->m_LinkMOEAry [time_stamp_in_min].LeftExit_QueueLength = pLink->LeftExit_Queue.size(); 

			if(pLink->m_LinkMOEAry [time_stamp_in_min].ExitQueueLength >=1 && pLink->m_LinkMOEAry [time_stamp_in_min].TrafficStateCode != 2)   // not fully congested
				pLink->m_LinkMOEAry [time_stamp_in_min].TrafficStateCode  = 1;  // partially congested

			pLink->m_LinkMOEAry [time_stamp_in_min].LoadingBuffer_QueueLength  = pLink->LoadingBuffer .size(); 

			if(pLink->m_LinkMOEAry [time_stamp_in_min].LoadingBuffer_QueueLength >=1 ) 
				pLink->m_LinkMOEAry [time_stamp_in_min].LoadingBuffer_TrafficStateCode  = 1;  // partially congested

			// time_stamp_in_min+1 is because we take the stastistics to next time stamp
			pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount =  pLink->CFlowArrivalCount;

			// toll collection 

			for(int pt = 1; pt < MAX_PRICING_TYPE_SIZE; pt++)
			{
				pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount_PricingType[pt] = pLink->CFlowArrivalCount_PricingType[pt];
				pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeRevenue_PricingType[pt] = pLink->CFlowArrivalRevenue_PricingType[pt];
			}

			pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeDepartureCount = pLink->CFlowDepartureCount;

			if(debug_flag && link_id_trace == li && vehicle_id_trace >= 0 )
			{
				TRACE("step 5: statistics collection: Time %d, link %d -> %d, Cumulative arrival count %d, cumulative departure count %d \n", time_stamp_in_min, g_NodeVector[pLink->m_FromNodeID].m_NodeNumber, g_NodeVector[pLink->m_ToNodeID].m_NodeNumber,
					pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeArrivalCount, pLink->m_LinkMOEAry [time_stamp_in_min].CumulativeDepartureCount);
			}


		}
	}
	return true;
}


NetworkLoadingOutput g_NetworkLoading(e_traffic_flow_model TrafficFlowModelFlag= tfm_spatial_queue, int SimulationMode = 0, int Iteration = 1)  // default spatial queue // SimulationMode= default 0: UE;  1: simulation from demand; 2: simulation from vehicle file
{
	NetworkLoadingOutput output;
	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;
	g_Number_of_CompletedVehicles = 0;
	g_Number_of_GeneratedVehicles = 0;


	//if(Iteration>= g_settings.IteraitonNoStartSignalOptimization && ((Iteration - g_settings.IteraitonNoStartSignalOptimization)%g_settings.IteraitonStepSizeSignalOptimization ==0))
	//{
	//	for(int i=0; i< g_NodeVector.size(); i++)
	//{
	//	g_NodeVector[i].QuickSignalOptimization ();
	//
	//}

	//}


	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		DTALink* pLink = g_LinkVector[li];
		pLink->SetupMOE();

		pLink->m_FFTT_simulation_interval = int(pLink->m_FreeFlowTravelTime/g_DTASimulationInterval);

		if(TrafficFlowModelFlag== tfm_point_queue)
		{
			if(pLink->m_FFTT_simulation_interval >= MAX_TIME_INTERVAL_ADCURVE)
			{
				TrafficFlowModelFlag = tfm_point_queue;
			}
		}

	}

	if(TrafficFlowModelFlag == tfm_BPR) // using BPR function
	{
		//Calculate BPRLinkVolume based on previous vehicle paths
		for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
		{
			if((*vIte)->m_NodeSize >=2)
			{
				for(int i = 0; i< (*vIte)->m_NodeSize-1; i++)
				{
					g_LinkVector[(*vIte)->m_NodeAry[i].LinkNo]->m_BPRLinkVolume++;
				}
			}
		}

		double hourly_demand_loading_muliplier = 1.0;

		hourly_demand_loading_muliplier = 60.0 / max(1, g_DemandLoadingEndTimeInMin - g_DemandLoadingStartTimeInMin);

		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			DTALink* pLink = g_LinkVector[li];
			pLink->m_BPRLinkTravelTime = pLink->m_FreeFlowTravelTime*(1.0f+pLink->m_BPR_Alpha *(powf(pLink->m_BPRLinkVolume*hourly_demand_loading_muliplier/(max(1,pLink->m_BPRLaneCapacity*pLink->GetNumberOfLanes())),pLink->m_BPR_Beta )));

			//float maximum_travel_time_ratio = 10;
			//if(pLink->m_FreeFlowTravelTime >0.01 && pLink->m_BPRLinkTravelTime > pLink->m_FreeFlowTravelTime * maximum_travel_time_ratio )
			//{  // if BRP travel time is extremely large, 10 times slower than the speed limit, then enforce a minimum moving speed to move the vehicles outof the network....
			//	pLink->m_BPRLinkTravelTime = pLink->m_FreeFlowTravelTime * maximum_travel_time_ratio;
			//	g_LogFile << "Reset extreme large BPR travel time :" << g_NodeVector[pLink->m_FromNodeID].m_NodeNumber << " ->" << g_NodeVector[pLink->m_ToNodeID].m_NodeNumber << "travel time:" << pLink->m_BPRLinkTravelTime  << ", org raito: " << pLink->m_BPRLinkTravelTime/pLink->m_FreeFlowTravelTime  << endl;

			//}

			if(Iteration == 19)
			{
				g_LogFile << "BPR Travel Time for Link "<< g_NodeVector[pLink->m_FromNodeID].m_NodeNumber  << " ->" << g_NodeVector[pLink->m_ToNodeID].m_NodeNumber <<" Flow:" << pLink->m_BPRLinkVolume << "travel time:" << pLink->m_BPRLinkTravelTime  << endl;
			}

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

		if(pVeh->m_InformationClass >=2)  // with real-time information group
		{
			g_bInformationUpdatingAndReroutingFlag = true;
		}

		//if(pVeh->m_NodeSize >=2)  // has feasible path
		//{
		//	int FirstLink =pVeh->m_NodeAry[0].LinkNo;

		//	DTALink* pLink = g_LinkVector[FirstLink];
		//	pLink ->LoadingBufferSize ++;
		//}
		//// load into loading buffer
	}
	//for(unsigned li = 0; li< g_LinkVector.size(); li++)
	//{
	//	DTALink* pLink = g_LinkVector[li];

	//	if(pLink ->LoadingBufferVector!=NULL)
	//		delete pLink ->LoadingBufferVector;

	//	if(pLink ->LoadingBufferSize >=1)
	//	{
	//		pLink ->LoadingBufferVector = new int[pLink ->LoadingBufferSize];
	//		pLink ->LoadingBufferSize = 0; // reset
	//	}
	//}

	//// assign vehicles to loading buffer
	//for (std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin(); iterVehicle != g_VehicleVector.end(); iterVehicle++)
	//{
	//	DTAVehicle* pVeh = (*iterVehicle);
	//	pVeh->PreTripReset();

	//	if(pVeh->m_NodeSize >=2)  // has feasible path
	//	{
	//		int FirstLink =pVeh->m_NodeAry[0].LinkNo;

	//		DTALink* pLink = g_LinkVector[FirstLink];
	//		pLink ->LoadingBufferVector [pLink ->LoadingBufferSize ++] = pVeh->m_VehicleID ;
	//	}
	//	// load into loading buffer
	//}


	cout << "start simulation process..."  << endl;

	double time;
	int simulation_time_interval_no = 0;
	bool bPrintOut = true;

	if(g_VehicleMap.size()>0)
	{

		std::map<int, DTAVehicle*>::iterator iterVM = g_VehicleMap.begin ();
		g_LastLoadedVehicleID = iterVM->first;

	}


	// generate historical info based shortst path, based on constant link travel time

	g_SimululationReadyToEnd = g_PlanningHorizon;

	for(time = g_DemandLoadingStartTimeInMin ; time< min(g_PlanningHorizon,g_SimululationReadyToEnd); simulation_time_interval_no++)  // the simulation time clock is advanced by 0.1 seconds
	{
		time= g_DemandLoadingStartTimeInMin+ simulation_time_interval_no*g_DTASimulationInterval;

		//if(TrafficFlowModelFlag ==1) // point queue
		//	g_VehicularSimulation_BasedOnADCurves(Iteration,time, simulation_time_interval_no, TrafficFlowModelFlag);
		//else // , spatial queue and Newell's model


		g_VehicularSimulation(Iteration,time, simulation_time_interval_no, TrafficFlowModelFlag);

		if(bPrintOut && g_Number_of_GeneratedVehicles > 0 && g_Number_of_CompletedVehicles == g_VehicleMap.size())
		{		
			cout << "--Simulation completes as all the vehicles are out of the network.--" << endl;
			output.NetworkClearanceTimeStamp_in_Min = time;
			output.NetworkClearanceTimePeriod_in_Min = time-g_DemandLoadingStartTimeInMin;
			bPrintOut = false;
			g_SimululationReadyToEnd = time + 10; // run for another 10 min for simulator to tally all statstics (for every 1 or 5 min)
		}
		if(simulation_time_interval_no%50 == 0 && bPrintOut) // every 5 min
		{
			int hour = ((int)(time))/60;
			int min = time - hour*60;

			CString time_str;
			time_str.Format("%2d:%02d",hour, min);

			cout << "simu clock: " << time_str << ",# of veh -- Generated: "<< g_Number_of_GeneratedVehicles << ", In network: "<<g_Number_of_GeneratedVehicles-g_Number_of_CompletedVehicles << endl;
			g_LogFile << "simulation clock: " << time_str << ", # of vehicles  -- Generated: "<< g_Number_of_GeneratedVehicles << ", In network: "<<g_Number_of_GeneratedVehicles-g_Number_of_CompletedVehicles << endl;
		}
	}

	// generate EndTimeOfPartialCongestion
	// before this step, we already generate the state of each time stamp, in terms of free-flow, congestion and queue spillk back case. 
	for(unsigned li = 0; li< g_LinkVector.size(); li++)  // for each link
	{

		DTALink* pLink = g_LinkVector[li];

		int NextCongestionTransitionTimeStamp = g_PlanningHorizon+10;  // // start with the initial value, no queue

		if(pLink->m_LinkMOEAry[g_PlanningHorizon-1].ExitQueueLength>=1)  // remaining queue at the end of simulation horizon
			NextCongestionTransitionTimeStamp = g_PlanningHorizon-1;


		int time_min;

		for(time_min = g_DemandLoadingStartTimeInMin+1; time_min <= g_PlanningHorizon-1 ; time_min++)  // move backward
		{

			pLink->m_LinkMOEAry[time_min].CumulativeArrivalCount  = max(pLink->m_LinkMOEAry[time_min-1].CumulativeArrivalCount,pLink->m_LinkMOEAry[time_min].CumulativeArrivalCount); // remove data holes
			pLink->m_LinkMOEAry[time_min].CumulativeDepartureCount  = max(pLink->m_LinkMOEAry[time_min-1].CumulativeDepartureCount ,pLink->m_LinkMOEAry[time_min].CumulativeDepartureCount); // remove data holes		}
		}

		for(time_min = g_PlanningHorizon-1; time_min>= g_DemandLoadingStartTimeInMin ; time_min--)  // move backward
		{
			// transition condition 1: from partial congestion to free-flow; action: move to the next lini
			if(time_min>=1 && pLink->m_LinkMOEAry[time_min-1].ExitQueueLength>=1 && pLink->m_LinkMOEAry[time_min].ExitQueueLength==0)  // previous time_min interval has queue, current time_min interval has no queue --> end of congestion 
			{
				NextCongestionTransitionTimeStamp = time_min;			
			}

			//transition condition 2: from partial congestion to fully congesed. action: --> move to the previous link
			if(time_min>=1 && pLink->m_LinkMOEAry[time_min-1].ExitQueueLength>=1 && pLink->m_LinkMOEAry[time_min].TrafficStateCode == 2 )
			{
				NextCongestionTransitionTimeStamp = time_min;			
			}


			if(pLink->m_LinkMOEAry[time_min].ExitQueueLength > 0 ) // there is queue at time time_min, but it is not end of queue
			{
				pLink->m_LinkMOEAry[time_min].EndTimeOfPartialCongestion = NextCongestionTransitionTimeStamp;
			}

		}

		// for loading buffer
		NextCongestionTransitionTimeStamp = g_PlanningHorizon+10;  // // start with the initial value, no queue

		for(time_min = g_PlanningHorizon-1; time_min>= g_DemandLoadingStartTimeInMin ; time_min--)  // move backward
		{
			// transition condition 1: from partial congestion to free-flow; action: move to the next lini
			if(time_min>=1 && pLink->m_LinkMOEAry[time_min-1].LoadingBuffer_QueueLength >=1 && pLink->m_LinkMOEAry[time_min].LoadingBuffer_QueueLength==0)  // previous time_min interval has queue, current time_min interval has no queue --> end of congestion 
			{
				NextCongestionTransitionTimeStamp = time_min;			
			}

			if(pLink->m_LinkMOEAry[time_min].LoadingBuffer_QueueLength > 0 ) // there is queue at time time_min, but it is not end of queue
			{
				pLink->m_LinkMOEAry[time_min].LoadingBuffer_EndTimeOfCongestion  = NextCongestionTransitionTimeStamp;
			}

		}

		for(int hour = g_DemandLoadingStartTimeInMin/60; hour < g_PlanningHorizon/60+1; hour++)  // used for ODME
		{
			pLink->SimultedHourlySpeed[hour] = pLink->m_Length / max(0.0001,pLink->GetTravelTimeByMin (Iteration,hour*60, 60 /*interval*/,TrafficFlowModelFlag)/60);  // 60: convert min to hour
		}
	}

	if(g_SensorDataCount>=1) 
	{
		g_UpdateLinkMOEDeviation_ODEstimation(output,Iteration);
	}

	if(Iteration == g_NumberOfIterations)
	{
		g_OutputODMEResults();
	}


	float TotalTripTime = 0;
	float TotalTripFFTT = 0;
	float TotalTravelTime = 0;
	float TotalDelay = 0;
	int VehicleSizeComplete = 0;
	float TotalDistance =0;
	int NumberofVehiclesSwitched  = 0;
	int NumberofVehiclesConsideringToSwitch  = 0;


	for (std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin(); iterVehicle != g_VehicleVector.end(); iterVehicle++)
	{
		if((*iterVehicle)->m_bComplete)
		{
			TotalTripTime+= (*iterVehicle)->m_TripTime;
			TotalTripFFTT+= (*iterVehicle)->m_TripFFTT;

			TotalTravelTime += (*iterVehicle)->m_TravelTime;
			TotalDelay += max(0,(*iterVehicle)->m_Delay);
			TotalDistance+= (*iterVehicle)->m_Distance ;

			VehicleSizeComplete +=1;


			// calcaulate time-dependent MOE for simulation daily output
			int time_interval =  (*iterVehicle)->m_DepartureTime / 15;

			output.TimeDedepentMOEMap[time_interval].TotalTripTime+= (*iterVehicle)->m_TripTime;
			output.TimeDedepentMOEMap[time_interval].TotalTripFFTT+= (*iterVehicle)->m_TripFFTT;

			output.TimeDedepentMOEMap[time_interval].TotalTravelTime += (*iterVehicle)->m_TravelTime;
			output.TimeDedepentMOEMap[time_interval].TotalDelay += max(0,(*iterVehicle)->m_Delay);
			output.TimeDedepentMOEMap[time_interval].TotalDistance+= (*iterVehicle)->m_Distance ;
			output.TimeDedepentMOEMap[time_interval].VehicleSizeComplete +=1;

			// only calculate gap for vehicles with updated gap measures

			if((*iterVehicle)->m_gap_update )
			{
				output.TimeDedepentGapMap[time_interval].NumberofVehiclesWithGapUpdate +=1;
				output.TimeDedepentGapMap[time_interval].total_gap  += (*iterVehicle)->m_gap;
				double relative_gap = max(0, (*iterVehicle)->m_gap);
				output.TimeDedepentGapMap[time_interval].total_relative_gap   += (relative_gap/max(0.1,(*iterVehicle)->m_TripTime)) ;
			}


			if((*iterVehicle)->m_bConsiderToSwitch )
			{
				NumberofVehiclesConsideringToSwitch+=1;
			}

			if((*iterVehicle)->m_bSwitch )
			{
				NumberofVehiclesSwitched+=1;
			}


		}
	}

	double AvgTravelTime = 0;
	double AvgTripTime = 0;
	double AvgTripTimeIndex = 0;
	double AvgDelay = 0;
	double AvgDistance = 0;
	double SwitchPercentage = 0;
	double ConsideringSwitchPercentage = 0;

	if(VehicleSizeComplete>=1)
	{
		AvgTripTime = TotalTripTime /max(1,VehicleSizeComplete);
		AvgTravelTime = TotalTravelTime /max(1,VehicleSizeComplete);
		AvgTripTimeIndex = TotalTripTime/max(0.1,TotalTripFFTT);
		AvgDelay = TotalDelay /max(1,VehicleSizeComplete);
		AvgDistance = TotalDistance /max(1,VehicleSizeComplete);
		SwitchPercentage =	NumberofVehiclesSwitched*100.0f/max(1,VehicleSizeComplete);
		ConsideringSwitchPercentage = NumberofVehiclesConsideringToSwitch*100.0f/max(1,VehicleSizeComplete);
	}

	//tally statistics for time-dependent MOE
	std::map<int, NetworkLoadingTimeDepedentMOE>::iterator iterTD;
	for (iterTD = output.TimeDedepentMOEMap.begin(); iterTD != output.TimeDedepentMOEMap.end(); iterTD++)
	{
		if((*iterTD).second .VehicleSizeComplete >=1)
		{
			(*iterTD).second.AvgTripTime = (*iterTD).second.TotalTripTime /max(1,(*iterTD).second .VehicleSizeComplete);
			(*iterTD).second.AvgTravelTime = (*iterTD).second.TotalTravelTime /max(1,(*iterTD).second .VehicleSizeComplete);
			(*iterTD).second.AvgTripTimeIndex = (*iterTD).second.TotalTripTime/max(0.1,(*iterTD).second .TotalTripFFTT);
			(*iterTD).second.AvgDelay = (*iterTD).second.TotalDelay /max(1,(*iterTD).second .VehicleSizeComplete);
			(*iterTD).second.AvgDistance = (*iterTD).second.TotalDistance /max(1,(*iterTD).second .VehicleSizeComplete);

		}
	}




	output.NumberofVehiclesCompleteTrips = VehicleSizeComplete;
	output.NumberofVehiclesGenerated = g_Number_of_GeneratedVehicles;
	if(AvgTripTime > 0)
	{
		output.AvgTripTime = AvgTripTime;
		output.AvgTravelTime = AvgTravelTime;
		output.AvgDelay = AvgDelay;
		output.AvgTTI = AvgTripTimeIndex;  // (AvgTripTime - AvgDelay) is the free flow travel time
		output.AvgDistance = AvgDistance;
		output.SwitchPercentage = SwitchPercentage;
		output.ConsideringSwitchPercentage = ConsideringSwitchPercentage;

		output.AvgTravelTime  = AvgTravelTime;
	}else
	{
		output.AvgTripTime = 0;
		output.AvgTravelTime  = 0;
		output.AvgDelay = 0;
		output.AvgTTI = 1;
		output.AvgDistance = 0;
		output.SwitchPercentage = 0;
		output.ConsideringSwitchPercentage = 0;
	}

	if( Iteration == g_NumberOfIterations)  // output results at the last iteration
	{
		g_SimulationResult.number_of_vehicles  = g_VehicleVector.size();
		g_SimulationResult.avg_trip_time_in_min = output.AvgTripTime;
		g_SimulationResult.avg_distance_in_miles = output.AvgDistance;
		g_SimulationResult.avg_speed = output.AvgDistance/(max(0.1,output.AvgTripTime)/60.0f);


	}

	// update crash prediction statistics
	//DTASafetyPredictionModel SafePredictionModel;
	//SafePredictionModel.UpdateCrashRateForAllLinks();

	if(0)  // comment out day to day learning code
	{
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

float GetTimeDependentCapacityAtSignalizedIntersection(int CycleLength_in_second, int EffectiveGreenTime_in_second,
													   int GreenStartTime_in_second,int offset_in_second, double CurrentTime, float SaturationFlowRate)
{

	ASSERT(CycleLength_in_second>=1);

	float simulation_time_interval_in_second = g_DTASimulationInterval*60;

	float GreenEndTime_in_second = GreenStartTime_in_second + EffectiveGreenTime_in_second;

	int green_time_in_second = 0;

	int simulation_start_time = CurrentTime*60;

		// consider offset
	if(simulation_start_time >= offset_in_second)
		simulation_start_time -=offset_in_second; 

	for(int t =simulation_start_time ; t< simulation_start_time +simulation_time_interval_in_second ; t++)
	{
		int mod_t  = t% CycleLength_in_second;
		if(mod_t>= GreenStartTime_in_second && mod_t<= GreenStartTime_in_second+ EffectiveGreenTime_in_second)
		{

			green_time_in_second ++;
		}

	}


	float DynamicCapacity  = 0;

	// max(0,intersection_end_time-intersection_start_time)  is the duraiton of intersection of two time intervals
	DynamicCapacity =  SaturationFlowRate*green_time_in_second/simulation_time_interval_in_second;

	return DynamicCapacity;
}


