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
#include <omp.h>

#include "DTALite.h"
#include "GlobalData.h"


using namespace std;
extern ofstream g_LogFile;

long g_precision_constant=100000L;

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

bool g_VehicularSimulation(double CurrentTime, int simulation_time_interval_no, int TrafficFlowModelFlag)
{
	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;
	std::map<int, DTAVehicle*>::iterator iterVM;
	int   PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};

	int time_stamp_in_min = int(CurrentTime+0.0001);

	std::list<struc_vehicle_item>::iterator vii;

	int vehicle_id_trace = 0;
	int link_id_trace = -1;
	bool debug_flag = true;

	//	TRACE("st=%d\n", simulation_time_interval_no);
	//DTALite:
	// vertical queue data structure
	// each link  ExitQueue, EntranceQueue: (VehicleID, ReadyTime)

	// load vehicle into network

	// step 1: scan all the vehicles, if a vehicle's start time >= CurrentTime, and there is available space in the first link,
	// load this vehicle into the ready queue

	// comment: we use map here as the g_VehicleMap map is sorted by departure time.
	// At each iteration, we start  the last loaded id, and exit if the departure time of a vehicle is later than the current time.

	for (iterVM = g_VehicleMap.find(g_LastLoadedVehicleID); iterVM != g_VehicleMap.end(); iterVM++)
	{
		DTAVehicle* pVeh = iterVM->second;
		if(pVeh->m_bLoaded == false && g_floating_point_value_less_than_or_eq_comparison(pVeh->m_DepartureTime, CurrentTime) && pVeh->m_NodeSize >=2)  // not being loaded
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
					int IS_id  = p_link->GetInformationResponseID(CurrentTime);
					if(IS_id >= 0)
					{
						if( g_VehicleMap[vi.veh_id]->GetRandomRatio()*100 < p_link->MessageSignVector [IS_id].ResponsePercentage )
						{  // vehicle rerouting

							g_VehicleRerouting(vi.veh_id,CurrentTime, p_link->MessageSignVector [IS_id]);

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


		}

		if(g_floating_point_value_less_than_or_eq_comparison(pVeh->m_DepartureTime,CurrentTime) == false)
		{
			break;
		}


	}

	// loading buffer

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		while(g_LinkVector[li]->LoadingBuffer.size() >0 && g_LinkVector[li]->GetNumLanes(CurrentTime)>0.01)  // no load vehicle into a blocked link
		{
			struc_vehicle_item vi = g_LinkVector[li]->LoadingBuffer.front();
			// we change the time stamp here to reflect the actual loading time into the network, especially for blocked link
			vi.time_stamp = CurrentTime;

			if(debug_flag && vi.veh_id == vehicle_id_trace )
				TRACE("Step 1: Time %f: Load vhc %d from buffer to physical link %d->%d\n",CurrentTime,vi.veh_id,g_LinkVector[li]->m_FromNodeNumber,g_LinkVector[li]->m_ToNodeNumber);

			int VehicleSize = (int)( g_LinkVector[li]->EntranceQueue.size() + g_LinkVector[li]->ExitQueue.size());

			// determine link in capacity 
			float AvailableSpaceCapacity = g_LinkVector[li]->m_VehicleSpaceCapacity - VehicleSize;

			// if we use BPR function, no density constraint is imposed --> TrafficFlowModelFlag == 0
			if(TrafficFlowModelFlag==0 || AvailableSpaceCapacity >= max(2,g_LinkVector[li]->m_VehicleSpaceCapacity*(1-g_MaxDensityRatioForVehicleLoading)))  // at least 10% remaining capacity or 2 vehicle space is left
			{
				g_LinkVector[li]->LoadingBuffer.pop_front ();
				g_LinkVector[li]->EntranceQueue.push_back(vi);
				g_LinkVector[li]->CFlowArrivalCount +=1;

			if(debug_flag && vi.veh_id == vehicle_id_trace )
				TRACE("Step 1: Time %f: Capacity available, remove vhc %d from buffer to physical link %d->%d\n",CurrentTime,vi.veh_id,g_LinkVector[li]->m_FromNodeNumber,g_LinkVector[li]->m_ToNodeNumber);

			}else
			{
				break;  // physical road is too congested, wait for next time interval to load
			}

		}
	}

	// step 2: move vehicle from EntranceQueue To ExitQueue, if ReadyTime <= CurrentTime)

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
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
					link_id_trace = li;
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
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			float PerHourCapacity = g_LinkVector[li]->GetHourlyPerLaneCapacity(CurrentTime);  // static capacity from BRP function
			float PerHourCapacityAtCurrentSimulatioInterval = PerHourCapacity;

			// freeway 

			if(g_LinkTypeFreewayMap [g_LinkVector[li]->m_link_type] ==1)  
			{
				if(g_StochasticCapacityMode &&  g_LinkVector[li]->m_StochaticCapcityFlag >=1 && simulation_time_interval_no%150 ==0)  // update stochastic capacity every 15 min
				{
					bool QueueFlag = false;

					if(g_LinkVector[li]->ExitQueue.size() > 0)  // vertical exit queue
						QueueFlag = true;

					PerHourCapacityAtCurrentSimulatioInterval = GetStochasticCapacity(QueueFlag,g_LinkVector[li]->GetHourlyPerLaneCapacity(CurrentTime));
				}
			}

			//Highway/Expressway,			 Principal arteria				Major arterial					 Frontage road
			if(g_LinkTypeFreewayMap[g_LinkVector[li]->m_link_type ] !=1 && g_LinkTypeRampMap[g_LinkVector[li]->m_link_type ]!=1)
			{
				PerHourCapacityAtCurrentSimulatioInterval = GetDynamicCapacityAtSignalizedIntersection(PerHourCapacity, g_CycleLength_in_seconds,CurrentTime);
			}

			// determine link out capacity 
			float MaximumFlowRate = PerHourCapacityAtCurrentSimulatioInterval *g_DTASimulationInterval/60.0f*g_LinkVector[li]->GetNumLanes(CurrentTime); //60 --> cap per min --> unit # of vehicle per simulation interval

			float Capacity = MaximumFlowRate;
			// use integer number of vehicles as unit of capacity
			g_LinkVector[li]-> LinkOutCapacity = g_GetRandomInteger_From_FloatingPointValue(Capacity);

			int vehiclesize = (int)(g_LinkVector[li]->CFlowArrivalCount - g_LinkVector[li]->CFlowDepartureCount);

			float fLinkInCapacity = 99999.0;

			// TrafficFlowModelFlag == 1 -> point queue model
			if(TrafficFlowModelFlag >=2)  // apply spatial link in capacity for spatial queue models( with spillback and shockwave) 
			{
				// determine link in capacity 
				float AvailableSpaceCapacity = g_LinkVector[li]->m_VehicleSpaceCapacity - vehiclesize;
				fLinkInCapacity = min (AvailableSpaceCapacity, MaximumFlowRate); 
				//			TRACE(" time %5.2f, SC: %5.2f, MFR %5.2f\n",CurrentTime, AvailableSpaceCapacity, MaximumFlowRate);

				// the inflow capcaity is the minimum of (1) incoming maximum flow rate (determined by the number of lanes) and (2) available space capacty  on the link.
				// use integer number of vehicles as unit of capacity

				if(TrafficFlowModelFlag ==3 && g_LinkTypeFreewayMap[g_LinkVector[li]->m_link_type] ==1)  // newell's model on freeway only
				{
					if(simulation_time_interval_no >=g_LinkVector[li]->m_BackwardWaveTimeInSimulationInterval )
					{
						int t_residual_minus_backwardwaveTime = int(simulation_time_interval_no - g_LinkVector[li]->m_BackwardWaveTimeInSimulationInterval) % MAX_TIME_INTERVAL_ADCURVE;

						float VehCountInJamDensity = g_LinkVector[li]->m_Length * g_LinkVector[li]->GetNumLanes(CurrentTime) *g_LinkVector[li]->m_KJam;
						int N_Arrival_Now_Constrainted = (int)(g_LinkVector[li]->m_CumuDeparturelFlow[t_residual_minus_backwardwaveTime] + VehCountInJamDensity);  //g_LinkVector[li]->m_Length 's unit is mile
						int t_residual_minus_1 = (simulation_time_interval_no - 1) % MAX_TIME_INTERVAL_ADCURVE;

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
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
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
						float MaxMergeCapacity = g_LinkVector [ g_LinkVector[li]->m_MergeOnrampLinkID ]->GetHourlyPerLaneCapacity(CurrentTime)*g_DTASimulationInterval/60.0f*g_LinkVector [ g_LinkVector[li]->m_MergeOnrampLinkID ]->GetNumLanes(CurrentTime) * 0.5f; //60 --> cap per min --> unit # of vehicle per simulation interval
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

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{

		// vehicle_out_count is the minimum of LinkOutCapacity and ExitQueue Size
	
		int vehicle_out_count = g_LinkVector[li]->LinkOutCapacity;


		float temp = g_LinkVector[li]->GetNumLanes();


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
					int IS_id  = p_Nextlink->GetInformationResponseID(CurrentTime);
					if(IS_id >= 0)
					{
						if( g_VehicleMap[vehicle_id]->GetRandomRatio()*100 < p_Nextlink->MessageSignVector [IS_id].ResponsePercentage )
						{  // vehicle rerouting

							g_VehicleRerouting(vehicle_id,CurrentTime, p_Nextlink->MessageSignVector [IS_id]);

							if(g_VehicleVector[vehicle_id]->m_InformationClass  == 1) /* historical info */
								g_VehicleVector[vehicle_id]->m_InformationClass=5; /* historical info, resposive to VMS */

							if(g_VehicleVector[vehicle_id]->m_InformationClass  == 2) /* pre-trip info */
								g_VehicleVector[vehicle_id]->m_InformationClass=7; /* pre-trip info, resposive to VMS */

						}else
						{
							if(g_VehicleVector[vehicle_id]->m_InformationClass  == 1) /* historical info */
								g_VehicleVector[vehicle_id]->m_InformationClass=4; /* historical info, nonresposive to VMS */

							if(g_VehicleVector[vehicle_id]->m_InformationClass  == 2) /* pre-trip info */
								g_VehicleVector[vehicle_id]->m_InformationClass=6; /* pre-trip info, nonresposive to VMS */

						}

					}



					vi.veh_id = vehicle_id;

					vi.time_stamp = ArrivalTimeOnDSN + p_Nextlink->GetFreeMovingTravelTime(TrafficFlowModelFlag, CurrentTime);

					// remark: when - TimeOnNextLink < 0, it means there are few seconds of left over on current link, which should be spent on next link

					p_Nextlink->EntranceQueue.push_back(vi);
					p_Nextlink->CFlowArrivalCount +=1;
					g_LinkVector[li]->CFlowDepartureCount +=1;

					g_LinkVector[li]->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime  += TravelTime;

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

				g_LinkVector[li]->ExitQueue.pop_front();

			}

			vehicle_out_count--;

		}


	}

	//	step 5: statistics collection
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
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

void g_AssignPathsForInformationUsers(double time, int simulation_time_interval_no)
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
					pNetwork[cid]->BuildTravelerInfoNetwork(time, COV_perception_erorr);
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
		g_LinkVector[li]->SetupMOE();
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
			g_LogFile << "BPR:"<< g_NodeVector[g_LinkVector[li]->m_FromNodeID].m_NodeName  << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID].m_NodeName <<" Flow:" << g_LinkVector[li]->m_BPRLinkVolume << "travel time:" << g_LinkVector[li]->m_BPRLinkTravelTime  << endl;

		}

	}


	g_Number_of_CompletedVehicles = 0;
	g_Number_of_GeneratedVehicles = 0;

	int Number_of_CompletedVehicles = 0;
	g_LastLoadedVehicleID = 0;
	for (std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin(); iterVehicle != g_VehicleVector.end(); iterVehicle++)
	{
		(*iterVehicle)->PreTripReset();
	}


	cout << "start simulation process..."  << endl;

	double time;
	int simulation_time_interval_no = 0;
	bool bPrintOut = true;

	// generate historical info based shortst path, based on constant link travel time

	for(time = g_DemandLoadingStartTimeInMin ; time< g_PlanningHorizon; simulation_time_interval_no++)  // the simulation time clock is advanced by 0.1 seconds
	{
		time= g_DemandLoadingStartTimeInMin+ simulation_time_interval_no*g_DTASimulationInterval;

		// generating paths for historical travel information
		if( SimulationMode == 1 /* one-shot simulation from demand */)
		{
			if(simulation_time_interval_no%(g_AggregationTimetInterval*10) == 0)
			{
				cout <<g_GetAppRunningTime()<<  "Calculating shortest paths..." << endl;

#pragma omp parallel for
				for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
				{
					int DepartureTimeInterval = time/g_AggregationTimetInterval;
					if(DepartureTimeInterval < g_AggregationTimetIntervalSize)
					{

						int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;
						int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

						// create network for shortest path calculation at this processor
						DTANetworkForSP network_MP(node_size, link_size, 1,g_AdjLinkSize); //  network instance for single processor in multi-thread environment
						int	id = omp_get_thread_num( );  // starting from 0
							// assign paths to historical information (for all vehicles)
							network_MP.HistInfoVehicleBasedPathAssignment(CurZoneID,time,time+g_AggregationTimetInterval);

					}

				}
			}

		g_AssignPathsForInformationUsers(time, simulation_time_interval_no);


			if(simulation_time_interval_no%(g_AggregationTimetInterval*10) == 0)
			{

				// reset statistics for departure-time-based travel time collection

				for(unsigned li = 0; li< g_LinkVector.size(); li++)
				{
					g_LinkVector[li]-> departure_count = 0;
					g_LinkVector[li]-> total_departure_based_travel_time = 0;
				}
			}
		}

		g_VehicularSimulation(time, simulation_time_interval_no, TrafficFlowModelFlag);

		if(bPrintOut && g_Number_of_GeneratedVehicles > 0 && g_Number_of_CompletedVehicles == g_Number_of_GeneratedVehicles)
		{		
			cout << "--Simulation completes as all the vehicles are out of the network.--" << endl;
			bPrintOut = false;
		}
		if(simulation_time_interval_no%50 == 0 && bPrintOut) // every 5 min
		{
			cout << "simulation clock:" << int(time) << " min, # of vehicles -- Generated: "<< g_Number_of_GeneratedVehicles << ", In network: "<<g_Number_of_GeneratedVehicles-g_Number_of_CompletedVehicles << endl;
			g_LogFile << "simulation clock:" << int(time/10) << ", # of vehicles  -- Generated: "<< g_Number_of_GeneratedVehicles << ", In network: "<<g_Number_of_GeneratedVehicles-g_Number_of_CompletedVehicles << endl;
		}
	}

	//	TRACE("g_Number_of_CompletedVehicles= %d\n", Number_of_CompletedVehicles);


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
	// end of dynamic traffic flow model


	if(g_ODEstimationFlag) 
		g_UpdateLinkMOEDeviation_ODEstimation();

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
	 // get length of file:
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
		ShortSimulationLogFile << "# of Vehicles = "<< g_VehicleVector.size() << ", Avg Travel Time =" << output.AvgTravelTime << " (min), Avg Distance = " << output.AvgDistance << " miles." << endl;

	}else
	{
		cout << "Error: File short_summary.log cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	
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


void g_VehicleRerouting(int v, float CurrentTime, MessageSign is) // v for vehicle id
{
	int PathLinkList[MAX_NODE_SIZE_IN_A_PATH];  // existing links
	int PathNodeList[MAX_NODE_SIZE_IN_A_PATH];  // new nodes
	DTANetworkForSP network(g_NodeVector.size(), g_LinkVector.size(), 1,g_AdjLinkSize);

	int CurrentLinkID = g_VehicleVector[v]->m_aryVN[g_VehicleVector[v]->m_SimLinkSequenceNo].LinkID;
	int	CurrentNodeID = g_LinkVector[CurrentLinkID]->m_ToNodeID;

	int l;
	// copy exsting links up to the current link
	for(l=0; l<= g_VehicleVector[v]->m_SimLinkSequenceNo; l++)
	{
		PathLinkList[l] = g_VehicleVector[v]->m_aryVN[l].LinkID;
	}


	if(is.Type == 2) // detour VMS
	{
	for(int ll = 0; ll < is.DetourLinkSize ; ll++)
	{
		PathLinkList[l++] = is.DetourLinkArray [ll];
		CurrentTime+= g_LinkVector[is.DetourLinkArray [ll]]->m_FreeFlowTravelTime;  // add travel time along the path to get the starting time at the end of subpath

	}
		CurrentNodeID = g_LinkVector[is.DetourLinkArray [is.DetourLinkSize -1]]->m_ToNodeID;

	}

	int LastLinkID = g_VehicleVector[v]->m_aryVN[g_VehicleVector[v]->m_NodeSize-2].LinkID;  // m_NodeSize-1 is # of links, m_NodeSize-2 is no. of link sequence
	int	LastNodeID = g_LinkVector[LastLinkID]->m_ToNodeID;


	float COV_perception_erorr = g_VMSPerceptionErrorRatio;
	network.BuildTravelerInfoNetwork(CurrentTime, COV_perception_erorr);
	network.TDLabelCorrecting_DoubleQueue(CurrentNodeID,CurrentTime,g_VehicleVector[v]->m_DemandType,g_VehicleVector[v]->m_VOT,false, false );

	// find shortest path
	int SubPathNodeSize = 0;
	int PredNode = LastNodeID;  

	// scan backward in the predessor array of the shortest path calculation results
	while(PredNode !=  CurrentNodeID && PredNode!=-1 && SubPathNodeSize< MAX_NODE_SIZE_IN_A_PATH)
	{
		ASSERT(SubPathNodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
		PathNodeList[SubPathNodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
		PredNode = network.NodePredAry[PredNode];
	}

	// add current node to find the links/paths
	PathNodeList[SubPathNodeSize++] = CurrentNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.

	int i;
	for(i = 0; i< SubPathNodeSize-1; i++)
	{
		// l is used before, continue counting
		PathLinkList[l++] = network.GetLinkNoByNodeIndex(PathNodeList[SubPathNodeSize-i-1], PathNodeList[SubPathNodeSize-i-2]);
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
		g_VehicleVector[v]->m_aryVN[i].LinkID = PathLinkList[i];
	}


}