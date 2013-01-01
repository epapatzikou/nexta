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


// to do: network statistis have to be seprated 
				//// update network statistics


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


	void g_MultiDayTrafficAssisnment()
{
/*
		int node_size  = g_NodeVector.size() +1 + g_ODZoneNumberSize;
		int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

		g_LogFile << "Number of iterations = " << g_NumberOfIterations << endl;

		int iteration = 0;
		bool NotConverged = true;
		int TotalNumOfVehiclesGenerated = 0;



		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			g_LinkVector[li]->InitializeDayDependentCapacity();
		}

		for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
		{

			float RandomNumber= (*vIte)->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			
			if(RandomNumber < 0.05)  // percentage of PI users
				(*vIte)->m_bETTFlag = false;
			else
				(*vIte)->m_bETTFlag = true;
		}
		// 1st loop for each iteration
		for(iteration=0; NotConverged && iteration <= g_NumberOfIterations; iteration++)  // we exit from the loop under two conditions (1) converged, (2) reach maximum number of iterations
		{

			cout << "------- Iteration = "<<  iteration << "--------" << endl;

			// 2nd loop for each vehicle


			for(int day = 0; day < MAX_DAY_SIZE; day++)  // for VI users, we use iteration per day first then per vehicle, as vehicles share the same information per day
			{

		// output statistics
		if(bStartWithEmpty)
		{
			fprintf(st, "vehicle_id,from_zone_id,to_zone_id,departure_time,arrival_time,complete_flag,trip_time,demand_type,pricing_type,vehicle_type,information_type,value_of_time,toll_cost_in_dollar,emissions,distance_in_mile,number_of_nodes,path_sequence\n");
		}

#pragma omp parallel for
				for (std::map<int, DTAZone>::iterator iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
				{
					DTAZone *pZone = iterZone->second ;
					if(pZone.m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
					{
						// create network for shortest path calculation at this processor
						DTANetworkForSP network_MP(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin); //  network instance for single processor in multi-thread environment
						int	id = omp_get_thread_num( );  // starting from 0

						cout <<g_GetAppRunningTime()<<  "processor " << id << " is working on day " << day  << " PI assignment at zone  "<<  CurZoneID << endl;

						network_MP.BuildNetworkBasedOnZoneCentriod(CurZoneID);  // build network for this zone, because different zones have different connectors...

						for(unsigned li = 0; li< g_LinkVector.size(); li++)
						{
							float TravelTime;
							if(iteration == 0)
							{
								TravelTime = g_LinkVector[li]->m_FreeFlowTravelTime ;
							}
							else
							{
								TravelTime = g_LinkVector[li]->m_DayDependentTravelTime[day];
							}
							network_MP.m_LinkTDTimeAry[g_LinkVector[li]->m_LinkNo][0] = TravelTime;
//							network_MP.m_LinkTDCostAry[g_LinkVector[li]->m_LinkNo][0]= 0;
							// use travel time now, should use cost later
						}


						network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),0,1,DEFAULT_VOT,false,false);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

						for (int vi = 0; vi<g_TDOVehicleArray[CurZoneID][0].VehicleArray.size(); vi++)
						{

							int VehicleID = g_TDOVehicleArray[CurZoneID][0].VehicleArray[vi];
							DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
							ASSERT(pVeh!=NULL);

							if(pVeh->m_bETTFlag == false)  // VI users
							{

								// create network for shortest path calculation at this processor

								bool bSwitchFlag = false;


								int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
								// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle
								int NodeSize = 0;
								int OriginCentriod = network_MP.m_PhysicalNodeSize;

								int DestinationCentriod = network_MP.m_PhysicalNodeSize+ pVeh->m_DestinationZoneID ;

								if( iteration >=1)
								{

									pVeh->m_DayDependentGap[day]= pVeh->m_DayDependentTripTime[day] - network_MP.LabelCostAry [DestinationCentriod];
								}else
								{
									pVeh->m_DayDependentGap[day] = 0;
								}

								float switching_rate = 1.0f/(iteration+1); // 0: MSA 

								if( iteration >=1)
								{
									switching_rate = (1.0f/(iteration+1)) * (pVeh->m_DayDependentGap[day] / pVeh->m_DayDependentTripTime[day]); // 3: Gap-based switching rule + MSA step size for UE
								}

								float RandomNumber= pVeh->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			

								if(RandomNumber < switching_rate || iteration==0)  			
								{
									bSwitchFlag = true;
								}				

								if(bSwitchFlag)  
								{
									int PredNode = network_MP.NodePredAry[DestinationCentriod];		
									while(PredNode != OriginCentriod && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
									{
										ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
										PathNodeList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
										PredNode = network_MP.NodePredAry[PredNode];
									}

									for(int i = 0; i< NodeSize-1; i++)
									{
										int LinkNo = network_MP.GetLinkNoByNodeIndex(PathNodeList[NodeSize-i-1], PathNodeList[NodeSize-i-2]);
										pVeh->m_DayDependentAryLink[day*MAX_PATH_LINK_SIZE+i]= LinkNo;
										pVeh->m_DayDependentNodeNumberSum[day] +=PathNodeList[i];

									}
									pVeh->m_DayDependentLinkSize[day] = NodeSize-1;
								}
							}
						}

					}
				}
			}

#pragma omp parallel for
			std::map<int, DTAZone>::iterator iterZone;
				for (iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
				{
					DTAZone *pZone = iterZone->second ;
					if(pZone.m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
				{
					// create network for shortest path calculation at this processor
					DTANetworkForSP network_MP(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin); //  network instance for single processor in multi-thread environment
					int	id = omp_get_thread_num( );  // starting from 0

					cout <<g_GetAppRunningTime()<<  "processor " << id << " is working on ETT assignment at zone  "<<  CurZoneID << endl;

					network_MP.BuildNetworkBasedOnZoneCentriod(CurZoneID);  // build network for this zone, because different zones have different connectors...

					// 4th loop for each link cost
					for(unsigned li = 0; li< g_LinkVector.size(); li++)
					{
						float TravelTime;

						if(iteration == 0)
						{
							TravelTime = g_LinkVector[li]->m_FreeFlowTravelTime ;
						}
						else
						{
							TravelTime = g_LinkVector[li]->m_AverageTravelTime;
						}
						network_MP.m_LinkTDTimeAry[g_LinkVector[li]->m_LinkNo][0] = TravelTime;
//						network_MP.m_LinkTDCostAry[g_LinkVector[li]->m_LinkNo][0]=  TravelTime;
						// use travel time now, should use cost later
					}

					network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),0,1,DEFAULT_VOT,false,false);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo


					for (int vi = 0; vi<g_TDOVehicleArray[CurZoneID][0].VehicleArray.size(); vi++)
					{

						int VehicleID = g_TDOVehicleArray[CurZoneID][0].VehicleArray[vi];
						DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
						ASSERT(pVeh!=NULL);

						if(pVeh->m_bETTFlag == true)  // ETT users
						{

							int OriginCentriod = network_MP.m_PhysicalNodeSize;
							int DestinationCentriod = network_MP.m_PhysicalNodeSize+ pVeh->m_DestinationZoneID ;

							for(int day = 0; day < MAX_DAY_SIZE; day++)
							{
								if( iteration >=1)
								{
									pVeh->m_DayDependentGap[day]= pVeh->m_AvgDayTravelTime - network_MP.LabelCostAry [DestinationCentriod];

									if(pVeh->m_DayDependentGap[day] < -0.1)
									{
										g_AssignmentLogFile <<"iteration " << iteration << " gap < 0: " << pVeh->m_VehicleID << " " << pVeh->m_AvgDayTravelTime << " " << network_MP.LabelCostAry [DestinationCentriod] <<endl;
									}
								}else
								{
									pVeh->m_DayDependentGap[day] = 0;
								}

							}
							bool bSwitchFlag = false;

							float switching_rate = 1.0f/(iteration+1); // 0: MSA 
							if( iteration >=1)
							{
								switching_rate = (1.0f/(iteration+1)) * (pVeh->m_AvgDayTravelTime - network_MP.LabelCostAry [DestinationCentriod]) / pVeh->m_AvgDayTravelTime; // 3: Gap-based switching rule + MSA step size for UE
							}
							float RandomNumber= pVeh->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			


							if(RandomNumber < switching_rate || iteration==0)  			
							{
								bSwitchFlag = true;
							}				

							// shortest path once, apply for all days
							int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
							// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle
							int NodeSize = 0;


							if(bSwitchFlag)  
							{
								int PredNode = network_MP.NodePredAry[DestinationCentriod];		
								while(PredNode != OriginCentriod && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
								{
									ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
									PathNodeList[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
									PredNode = network_MP.NodePredAry[PredNode];
								}
								for(int i = 0; i< NodeSize-1; i++)
								{
									int LinkNo = network_MP.GetLinkNoByNodeIndex(PathNodeList[NodeSize-i-1], PathNodeList[NodeSize-i-2]);
									pVeh->m_DayDependentAryLink[0*MAX_PATH_LINK_SIZE+i]= LinkNo;
									pVeh->m_DayDependentNodeNumberSum[0] +=PathNodeList[i];
								}

								for(int day = 0; day < MAX_DAY_SIZE; day++)
								{
									pVeh->m_DayDependentLinkSize[day] = NodeSize-1;
								}
							}

						}
					}
				}
			}


			// below should be single thread

			cout << "---- Network Loading for Iteration " << iteration <<"----" << endl;


			float DayByDayTravelTimeSumETT = 0;
			float DayByDayTravelTimeSumVI = 0;
			float PI_gap_sum = 0;
			float ETT_gap_sum = 0;
			float total_gap = 0;
			int VehicleCountVI = 0;
			float VehicleCountETT = 0;
			float TTSTDSumVI = 0;
			float TTSTDSumETT = 0;


			for(unsigned li = 0; li< g_LinkVector.size(); li++)
			{
				g_LinkVector[li]->m_AverageTravelTime = 0;
			}

			for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
			{
				(*vIte)->m_AvgDayTravelTime = 0;
			}

			for(int day = 0; day < MAX_DAY_SIZE; day++)
			{
				cout << "day:"<< day << "....."  << endl;

				for(unsigned li = 0; li< g_LinkVector.size(); li++)
				{

					g_LinkVector[li]->m_BPRLinkVolume = 0;
				}


				// switch data for the following line
				for(unsigned li = 0; li< g_LinkVector.size(); li++)
				{
					g_LinkVector[li]->m_BPRLaneCapacity = g_LinkVector[li]->m_DayDependentCapacity[day];
				}

				// fetch day-dependent vehicle path

				for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
				{
					for(int i = 0; i< (*vIte)->m_DayDependentLinkSize[day]; i++)
					{
						int LinkID;

						if((*vIte)->m_bETTFlag == true)
							LinkID= (*vIte)->m_DayDependentAryLink[0*MAX_PATH_LINK_SIZE+i];
						else
							LinkID= (*vIte)->m_DayDependentAryLink[day*MAX_PATH_LINK_SIZE+i];

						g_LinkVector[LinkID]->m_BPRLinkVolume++;
					}


				}


				// BPR based loading
				for(unsigned li = 0; li< g_LinkVector.size(); li++)
				{
					g_LinkVector[li]->m_BPRLinkTravelTime = g_LinkVector[li]->m_FreeFlowTravelTime*(1.0f+0.15f*(powf(g_LinkVector[li]->m_BPRLinkVolume/(g_LinkVector[li]->m_BPRLaneCapacity*g_LinkVector[li]->GetNumberOfLanes()),4.0f)));
					//				g_LogFile << "day:"<< day << ", BPR:"<< g_NodeVector[g_LinkVector[li]->m_FromNodeID] << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID]<<" Flow:" << g_LinkVector[li]->m_BPRLinkVolume << "travel time:" << g_LinkVector[li]->m_BPRLinkTravelTime  << endl;
					//				cout << "day:"<< day << ", BPR:"<< g_NodeVector[g_LinkVector[li]->m_FromNodeID] << " ->" << g_NodeVector[g_LinkVector[li]->m_ToNodeID]<<" Flow:" << g_LinkVector[li]->m_BPRLinkVolume << "travel time:" << g_LinkVector[li]->m_BPRLinkTravelTime  << endl;
					g_LinkVector[li]->m_DayDependentTravelTime[day] = g_LinkVector[li]->m_BPRLinkTravelTime;
					g_LinkVector[li]->m_AverageTravelTime+=  g_LinkVector[li]->m_BPRLinkTravelTime/MAX_DAY_SIZE;
				}

				VehicleCountETT = 0;
				float TravelTimeSumETT = 0;

				VehicleCountVI = 0;
				float TravelTimeSumVI = 0;


				for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
				{
					(*vIte)->m_DayDependentTripTime[day] = 0;

					if((*vIte)->m_bETTFlag == true)
					{
						for(int i = 0; i< (*vIte)->m_DayDependentLinkSize[0]; i++)
						{
							int LinkID = (*vIte)->m_DayDependentAryLink[0*MAX_PATH_LINK_SIZE+i];
							(*vIte)->m_DayDependentTripTime[day] += g_LinkVector[LinkID]->m_BPRLinkTravelTime;
						}

						(*vIte)->m_AvgDayTravelTime += ((*vIte)->m_DayDependentTripTime[day]/MAX_DAY_SIZE);

						TravelTimeSumETT+= (*vIte)->m_DayDependentTripTime[day];
						ETT_gap_sum += (*vIte)->m_DayDependentGap [day];
						total_gap +=(*vIte)->m_DayDependentGap [day];
						VehicleCountETT++;
					}else
					{
						for(int i = 0; i< (*vIte)->m_DayDependentLinkSize[day]; i++)
						{
							int LinkID = (*vIte)->m_DayDependentAryLink[day*MAX_PATH_LINK_SIZE+i];
							(*vIte)->m_DayDependentTripTime[day] += g_LinkVector[LinkID]->m_BPRLinkTravelTime;
						}

						(*vIte)->m_AvgDayTravelTime += ((*vIte)->m_DayDependentTripTime[day]/MAX_DAY_SIZE);


						TravelTimeSumVI+= (*vIte)->m_DayDependentTripTime[day];
						PI_gap_sum += (*vIte)->m_DayDependentGap [day];
						total_gap +=(*vIte)->m_DayDependentGap [day];
						VehicleCountVI++;
					}

				}



				float AverageTravelTimeETT = TravelTimeSumETT/max(VehicleCountETT,1);
				float AverageTravelTimeVI = TravelTimeSumVI/max(VehicleCountVI,1);


				DayByDayTravelTimeSumETT += AverageTravelTimeETT;
				DayByDayTravelTimeSumVI += AverageTravelTimeVI;


				//			g_AssignmentLogFile << "Iteration: " << iteration << ", Day: " << day <<  ", Ave ETT Travel Time: " << AverageTravelTimeETT << ", Ave VI Travel Time: " << AverageTravelTimeVI << ", # of VI: " << VehicleCountVI  <<   endl;
				cout << g_GetAppRunningTime() << "Iteration: " << iteration << ", Day: " << day <<  ", Ave ETT Travel Time: " << AverageTravelTimeETT << ", Ave VI Travel Time: " << AverageTravelTimeVI << ", # of VI: " << VehicleCountVI  <<  endl;
			}

			for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
			{
				float total_var = 0;
				for(int day = 0; day < MAX_DAY_SIZE; day++)
				{
					total_var += pow((*vIte)->m_DayDependentTripTime[day] - (*vIte)->m_AvgDayTravelTime, 2);
				}
				(*vIte)->m_DayTravelTimeSTD = sqrt(total_var/max(1,(MAX_DAY_SIZE-1)));

				if((*vIte)->m_bETTFlag == true)
				{
					TTSTDSumETT +=(*vIte)->m_DayTravelTimeSTD;
				}else
				{
					TTSTDSumVI +=(*vIte)->m_DayTravelTimeSTD;
				}
			}

			float AverageTTSTDETT = TTSTDSumETT/max(VehicleCountETT,1);
			float AverageTTSTDVI = TTSTDSumVI/max(VehicleCountVI,1);


			float Average_PI_Gap = PI_gap_sum/MAX_DAY_SIZE /max(VehicleCountVI,1);
			float Average_ETT_Gap = ETT_gap_sum/MAX_DAY_SIZE /max(VehicleCountETT,1);
			float Avg_gap = total_gap/MAX_DAY_SIZE/(VehicleCountETT+VehicleCountVI);
			g_AssignmentLogFile << "--D2D Summary: Iteration: " << iteration << ", Ave D2D ETT Travel Time: " << DayByDayTravelTimeSumETT/MAX_DAY_SIZE << ", Ave D2D VI Travel Time: " << DayByDayTravelTimeSumVI/MAX_DAY_SIZE << ", Ave ETT Gap: " << Average_ETT_Gap << ", Ave VI Gap: " << Average_PI_Gap << ", Ave Gap: " << Avg_gap << " TTSTDETT: "<< AverageTTSTDETT << ",TTSTDVI: "<< AverageTTSTDVI << endl;

			// we should output gap here
			//		g_AssignmentLogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Ave Travel Time: " << SimuOutput.AvgTripTime << ", TTI: " << SimuOutput.AvgTTI  << ", Avg Distance: " << SimuOutput.AvgDistance  << ", Switch %:" << SimuOutput.SwitchPercentage <<", Num of Vehicles Completing Trips: ";

			//output vehicle trajectory data
			//	OutputLinkMOEData("LinkMOE.csv", iteration,bStartWithEmptyFile);
			//	OutputNetworkMOEData("NetworkMOE.csv", iteration,bStartWithEmptyFile);

		}

		//	OutputMultipleDaysVehicleTrajectoryData("MultiDayVehicle.csv");


	}
	void OutputMultipleDaysVehicleTrajectoryData(char fname[_MAX_PATH])
	{
		FILE* st = NULL;

		fopen_s(&st,fname,"w");

		if(st!=NULL)
		{
			std::map<int, DTAVehicle*>::iterator iterVM;
			int VehicleCount_withPhysicalPath = 0;

			fprintf(st, "vehicle_id,  originput_zone_id, destination_zone_id, departure_time, arrival_time, complete_flag, trip_time, demand_type, information_type, value_of_time, dollar_cost,emissions,distance_in_mile, number_of_nodes,  node id, node arrival time\n");


			for (vector<DTAVehicle*>::iterator vIte = g_VehicleVector.begin();vIte != g_VehicleVector.end();vIte++)
			{

				if((*vIte)->m_DayDependentLinkSize[0] >= 1)  // with physical path in the network
				{

					int UpstreamNodeID = 0;
					int DownstreamNodeID = 0;

					float TripTime = 0;

					if((*vIte)->m_bComplete)
						TripTime = (*vIte)->m_ArrivalTime-(*vIte)->m_DepartureTime;

					float m_gap = 0;
					fprintf(st,"%d,%d,%d,d%d,%4.2f,%4.2f,%d,%4.2f,%d,%d,%d,%4.1f,%4.2f,%4.2f,%d",
						(*vIte)->m_VehicleID ,(*vIte)->m_bETTFlag, (*vIte)->m_OriginZoneID , (*vIte)->m_DestinationZoneID,
						(*vIte)->m_DepartureTime, (*vIte)->m_ArrivalTime , (*vIte)->m_bComplete, TripTime,
						(*vIte)->m_DemandType ,(*vIte)->m_InformationClass, (*vIte)->m_VOT , (*vIte)->m_TollDollarCost, (*vIte)->m_Emissions,(*vIte)->m_Distance, (*vIte)->m_NodeSize);

					fprintf(st, "\n AVG %5.3f, STD %5.3f,",(*vIte)->m_AvgDayTravelTime , (*vIte)->m_DayTravelTimeSTD) ;

					for(int day=0; day < MAX_DAY_SIZE; day++)
					{
						fprintf(st, "day %d,%5.2f,",day,(*vIte)->m_DayDependentTripTime[day]) ;

						for(int i = 0; i< (*vIte)->m_DayDependentLinkSize[day]; i++)
						{

							int LinkID = (*vIte)->m_DayDependentAryLink[day*MAX_PATH_LINK_SIZE+i];
							int NodeID = g_LinkVector[LinkID]->m_ToNodeID;
							int NodeName = g_NodeVector[NodeID].m_NodeNumber;

							fprintf(st, "%d",NodeName) ;
						}

						fprintf(st,"\n");

					}

				}
			}
			fclose(st);
		}
	*/
	}


/*
				if(g_TollingMethodFlag == 2)  // VMT toll
			{
				if(LinkID < g_LinkVector.size()) // physical link, which is always sort first.
				{
				DTALink* pLink= g_LinkVector[LinkID];
				 toll = pLink->m_Length * g_VMTTollingRate/max(1,VOT)*60;
				}
			}
*/




	void InnerLoopAssignment(int zone,int departure_time_begin, int departure_time_end, int inner_iteration) // this subroutine is called only when iteration > 0
{
		int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};
		std::vector<DTAVehicle*>::iterator iterVehicle = g_VehicleVector.begin();
		int NodeSize;
		int AssignmentInterval = int(departure_time_begin/g_AggregationTimetInterval);  // starting assignment interval

		PathArrayForEachODT *PathArray;
		PathArray = new PathArrayForEachODT[g_ODZoneNumberSize + 1]; // remember to release memory

		ConstructPathArrayForEachODT(PathArray, zone, AssignmentInterval); 

		// loop through the TDOVehicleArray to assign or update vehicle paths...
		for (int vi = 0; vi<g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray .size(); vi++)
		{
			int VehicleID = g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray[vi];
			DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
			ASSERT(pVeh!=NULL);

			bool bSwitchFlag = false;
			pVeh->m_bConsiderToSwitch = false;

			float m_gap;
			float ExperiencedTravelTime = pVeh->m_TripTime;	
			int VehicleDest = pVeh->m_DestinationZoneID;
			int NodeSum = pVeh->m_NodeNumberSum;

			int PathIndex = 0;		
			for(int p=1; p<=PathArray[VehicleDest].NumOfPaths; p++)
			{
				if(NodeSum == PathArray[VehicleDest].PathNodeSums[p])
				{
					PathIndex = p;
					break;
				}
			}

			float AvgPathTime = PathArray[VehicleDest].AvgPathTimes[PathIndex];		
			float MinTime = PathArray[VehicleDest].AvgPathTimes[PathArray[VehicleDest].BestPathIndex];		
			pVeh->SetMinCost(MinTime);

			if(g_VehicleExperiencedTimeGap == 1) 
				m_gap = ExperiencedTravelTime - MinTime;
			else
				m_gap = AvgPathTime - MinTime; 		

			if(m_gap < 0) m_gap = 0.0;			

			g_CurrentGapValue += m_gap; // Jason : accumulate g_CurrentGapValue only when iteration >= 1

			float switching_rate;
			// switching_rate = 1.0f/(iteration+1);   // default switching rate from MSA

			switch (g_UEAssignmentMethod)
			{
			case 0: switching_rate = 1.0f/(inner_iteration+1); // 0: MSA 
				break;
			case 1: switching_rate = float(g_LearningPercentage)/100.0f; // 1: day-to-day learning

				if(pVeh->m_TripTime > MinTime + g_TravelTimeDifferenceForSwitching || pVeh->m_TripTime > MinTime *(1+ g_TravelTimeDifferenceForSwitching/100))
				{
					switching_rate = 1.0f;
				}

				break;
			case 2: switching_rate = m_gap / ExperiencedTravelTime; // 2: GAP-based switching rule for UE
				break;
			case 3: switching_rate = (1.0f/(inner_iteration+1)) * (m_gap / ExperiencedTravelTime); // 3: Gap-based switching rule + MSA step size for UE
				break;
			default: switching_rate = 1.0f/(inner_iteration+1); // default is MSA 
				break;
			}

			float RandomNumber= pVeh->GetRandomRatio();  // vehicle-dependent random number generator, very safe for multi-thread applications			

			if((pVeh->m_bComplete==false && pVeh->m_NodeSize >=2)) //for incomplete vehicles with feasible paths, need to switch at the next iteration
			{
				bSwitchFlag = true;
			}else
			{
				if(RandomNumber < switching_rate)  			
				{
					bSwitchFlag = true;
				}				
			} 

			if(bSwitchFlag)  
			{
				// accumulate number of vehicles switching paths
				g_CurrentNumOfVehiclesSwitched += 1; 

				pVeh->m_bConsiderToSwitch = true;

				// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle
				NodeSize = PathArray[VehicleDest].PathSize[PathArray[VehicleDest].BestPathIndex];			

				pVeh->m_NodeSize = NodeSize;

				if( pVeh->m_NodeAry !=NULL)
				{
					delete pVeh->m_NodeAry;
				}

				if(pVeh->m_NodeSize>=2)
				{
					pVeh->m_bConsiderToSwitch = true;

					if(NodeSize>=900)
					{
						cout << "PATH Size >900 " << NodeSize;
						g_ProgramStop();
					}

					pVeh->m_NodeAry = new SVehicleLink[NodeSize];

					if(pVeh->m_NodeAry==NULL)
					{
						cout << "Insufficient memory for allocating vehicle arrays!";
						g_ProgramStop();
					}

					pVeh->m_NodeNumberSum = PathArray[VehicleDest].PathNodeSums[PathArray[VehicleDest].BestPathIndex];

					pVeh->m_Distance =0;

					for(int i = 0; i< NodeSize-1; i++)
					{
						pVeh->m_NodeAry[i].LinkNo = PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].BestPathIndex][i];					
						pVeh->m_Distance+= g_LinkVector[pVeh->m_NodeAry[i].LinkNo]->m_Length;
					}
					//cout << pVeh->m_VehicleID <<  " Distance" << pVeh->m_Distance <<  endl;;
				}else
				{
					pVeh->m_bLoaded  = false;
					pVeh->m_bComplete = false;

					if(inner_iteration==0)
					{
						//					g_WarningFile  << "Warning: vehicle " <<  pVeh->m_VehicleID << " from zone " << pVeh ->m_OriginZoneID << " to zone "  << pVeh ->m_DestinationZoneID << " does not have a physical path. Path Cost:" << TotalCost  << endl;
					}
				}
			} // if(bSwitchFlag)
		}

		delete PathArray;	
	}

	void g_DynamicTraffcAssignmentWithinInnerLoop(int iteration, bool NotConverged, int TotalNumOfVehiclesGenerated)
{
	if(iteration == 0) // Note: iteration 0 in outer loop only assigns initial paths to vehicles (i.e., without updating path assignment)
	{
		// do something?
	}else
	{
		// check if any vehicles swittching to new paths
		if((g_NewPathWithSwitchedVehicles == 0)||(iteration > g_NumberOfIterations)) //  g_NewPathWithSwitchedVehicles is determined in VehicleBasedPathAssignment
		{
			NotConverged = false; // converged for "outer" loop! 
		}else // run inner loop only when there are vehicles swittching to new paths found by TDSP
		{
			bool NotConvergedInner = true;

			// ----------* enter inner loop *----------
			for(int inner_iteration = 1; NotConvergedInner && inner_iteration <= g_NumberOfInnerIterations; inner_iteration++)
			{					
				// initialize for each iteration
				g_CurrentGapValue = 0.0;
				g_CurrentNumOfVehiclesSwitched = 0;
				g_CurrentNumOfVehiclesForUEGapCalculation = 0;

				for(int CurZoneID=1;  CurZoneID <= g_ODZoneNumberSize; CurZoneID++)
				{
					if(g_ZoneMap[CurZoneID].m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
					{								
						// scan all possible departure times
						for(int departure_time = g_DemandLoadingStartTimeInMin; departure_time < g_DemandLoadingEndTimeInMin; departure_time += g_AggregationTimetInterval)

						{
							if(g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray .size() > 0)
							{
								// update path assignments, g_CurrentGapValue, and g_CurrentNumOfVehiclesSwitched
								InnerLoopAssignment(CurZoneID, departure_time, departure_time+g_AggregationTimetInterval, inner_iteration);										
							}
						} // end - for each departure time interval
					}
				} // end - for each origin

				// evaluate new path assignments by simulation-based network loading
				cout << "---- Network Loading for Inner Loop Iteration " << inner_iteration <<"----" << endl;

				NetworkLoadingOutput SimuOutput;										
				SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag,0,iteration);

				TotalNumOfVehiclesGenerated = SimuOutput.NumberofVehiclesGenerated; // need this to compute avg gap

				g_AssignmentMOEVector[iteration] = SimuOutput;

				float PercentageComplete = 0;
				if(SimuOutput.NumberofVehiclesGenerated>0)
					PercentageComplete = SimuOutput.NumberofVehiclesCompleteTrips*100.0f/SimuOutput.NumberofVehiclesGenerated;

				g_LogFile << g_GetAppRunningTime() << "Inner Loop Iteration: " << inner_iteration << ", Average Trip Time: " << SimuOutput.AvgTripTime << ", TTI: " << SimuOutput.AvgTTI  << ", Average Distance: " << SimuOutput.AvgDistance << ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%"<<endl;
				cout << g_GetAppRunningTime() << "Inner Loop Iteration: " << inner_iteration <<", Average Trip Time: " << SimuOutput.AvgTripTime << ", Average Distance: " << SimuOutput.AvgDistance<< ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips << ", " << PercentageComplete << "%"<<endl;

				g_AssignmentLogFile << g_GetAppRunningTime() << "Inner Loop Iteration: " << inner_iteration << ", Ave Trip Time: " << SimuOutput.AvgTripTime << ", TTI: " << SimuOutput.AvgTTI  << ", Avg Distance: " << SimuOutput.AvgDistance  << ", Switch %:" << SimuOutput.SwitchPercentage <<", Num of Vehicles Completing Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%";			


				if(g_UEAssignmentMethod <= 1) // MSA and day-to-day learning
				{
					if(g_CurrentNumOfVehiclesSwitched < g_ConvergenceThreshold_in_Num_Switch)
						NotConvergedInner = false; // converged!
				}else // gap-based approaches
				{	
					//if(g_RelativeGap < g_ConvergencyRelativeGapThreshold_in_perc)
					//	NotConvergedInner = false; // converged! 
				}

				//g_PrevGapValue = g_CurrentGapValue; // update g_PrevGapValue

				float avg_gap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;
				float avg_relative_gap = g_CurrentRelativeGapValue / TotalNumOfVehiclesGenerated *100;
				 
				g_AssignmentLogFile << ", Num of Vehicles Switching Paths = " << g_CurrentNumOfVehiclesSwitched << ", Gap at prev iteration = " << g_CurrentGapValue << ", Avg Gap = " << avg_gap << ", Relative Gap at prev iteration = " << avg_relative_gap << "%" << endl;		

			} // end - enter inner loop
		} // end - check outer loop convergency				
	}
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

	for(i=0; i< m_PhysicalNodeSize + g_ODZoneNumberSize+1; i++)
	{
		m_OutboundSizeAry[i] = 0;
		m_InboundSizeAry[i] =0;
	}

	// add physical links

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
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


			//TRACE("%d -> %d, time %d", g_NodeVector[g_LinkVector[li]->m_FromNodeID], g_NodeVector[g_LinkVector[li]->m_ToNodeID],CurrentTime);
			float AvgTripTime = g_LinkVector[li]->GetHistoricalTravelTime(CurrentTime);

			float Normal_random_value = g_RNNOF() * Perception_error_ratio*AvgTripTime;
			
			float travel_time  = AvgTripTime + Normal_random_value;
			if(travel_time < 0.1f)
				travel_time = 0.1f;
			//TRACE(" %6.3f zone %d \n",AvgTripTime, CurZoneID);

			m_LinkTDTimeAry[g_LinkVector[li]->m_LinkNo][0] = travel_time;
	}

	int LinkID = g_LinkVector.size();

		// add outgoing connector from the centriod corresponding to the current origin zone to physical nodes of the current zone
	for(i = 0; i< g_ZoneMap[CurZoneID].m_OriginActivityVector.size(); i++)
	{
		FromID = m_PhysicalNodeSize; // m_PhysicalNodeSize is the centriod number for CurZoneNo // root node
		ToID = g_ZoneMap[CurZoneID].m_OriginActivityVector [i];

		//         TRACE("destination node of current zone %d: %d\n",CurZoneID, g_NodeVector[ToID]);

		m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
		m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = LinkID;
		m_OutboundSizeAry[FromID] +=1;

		m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = LinkID ;
		m_InboundSizeAry[ToID] +=1;


		ASSERT(g_AdjLinkSize > m_OutboundSizeAry[FromID]);

		for(int t = m_StartIntervalForShortestPathCalculation; t <m_NumberOfSPCalculationIntervals; t++)
		{
			m_LinkTDTimeAry[LinkID][t] = 0;
		}

		LinkID++;

	}

	// add incoming connectors from the physicla nodes corresponding to a zone to the non-current zone.
		std::map<int, DTAZone>::iterator iterZone;
		for (iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
		{
		DTAZone zone = iterZone->second ;
		if(iterZone->first !=CurZoneID)   // only this origin zone has vehicles, then we build the network
		{
			for(i = 0; i<  zone.m_OriginActivityVector.size(); i++)
			{
				FromID = zone.m_OriginActivityVector [i]; // m_PhysicalNodeSize is the centriod number for CurZoneNo
				ToID =   m_PhysicalNodeSize + iterZone->first; // m_PhysicalNodeSize is the centriod number for CurZoneNo, note that  .m_ZoneID start from 1

				m_OutboundNodeAry[FromID][m_OutboundSizeAry[FromID]] = ToID;
				m_OutboundLinkAry[FromID][m_OutboundSizeAry[FromID]] = LinkID;
				m_OutboundSizeAry[FromID] +=1;

					m_InboundLinkAry[ToID][m_InboundSizeAry[ToID]] = LinkID ;
					m_InboundSizeAry[ToID] +=1;


				ASSERT(g_AdjLinkSize >  m_OutboundSizeAry[FromID]);

				for( t= m_StartIntervalForShortestPathCalculation; t <m_NumberOfSPCalculationIntervals; t++)
				{
					m_LinkTDTimeAry[LinkID][t] = 0;
				}

				LinkID++;


			}
		}
	}

	m_NodeSize = m_PhysicalNodeSize + 1 + g_ODZoneNumberSize;


}



bool DTANetworkForSP::OptimalTDLabelCorrecting_DQ(int origin, int departure_time, int destination)
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
				TRACE("\nScan from node %d to node %d",g_NodeVector[FromID].m_NodeNumber,g_NodeVector[ToID].m_NodeNumber);

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




/* one one-shot simulation from demand 
		// generating paths for historical travel information

		if( SimulationMode == 1 // one-shot simulation from demand //)
		{
			if(simulation_time_interval_no%(g_AggregationTimetInterval*10) == 0)
			{
				cout <<g_GetAppRunningTime()<<  "Calculating shortest paths..." << endl;

#pragma omp parallel for
				for(int CurZoneID=1;  CurZoneID <= g_ODZoneNumberSize; CurZoneID++)
				{
					int DepartureTimeInterval = time/g_AggregationTimetInterval;
					if(DepartureTimeInterval < g_AggregationTimetIntervalSize)
					{

						int node_size  = g_NodeVector.size() +1 + g_ODZoneNumberSize;
						int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

						// create network for shortest path calculation at this processor
						DTANetworkForSP network_MP(node_size, link_size, 1,g_AdjLinkSize); //  network instance for single processor in multi-thread environment
						int	id = omp_get_thread_num( );  // starting from 0
						// assign paths to historical information (for all vehicles)
						network_MP.HistInfoVehicleBasedPathAssignment(CurZoneID,time,time+g_AggregationTimetInterval);

					}

				}
			}

//			g_AssignPathsForInformationUsers(Iteration,time, simulation_time_interval_no);


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

*/
/*

void DTALink::ComputeVSP_FastMethod()  // VSP: vehicle specific power
{

	// step 1: car following simulation
		float link_length_in_meter = m_Length * 1609.344f; //1609.344f: mile to meters

		float BackwardWaveSpeed_in_meter_per_second = m_BackwardWaveSpeed  * 1609.344f / 3600; 
	for(int LaneNo = 0; LaneNo < m_NumLanes; LaneNo++)
	{
			int v;
			for(v = 0; v<m_VehicleDataVector[LaneNo].LaneData.size(); v++)
			{
				float distance = 0;  // from the starting point of the link to the current location
				float prev_distance = 0;

				DTAVehicle* pVehicle  = g_VehicleMap[m_VehicleDataVector[LaneNo].LaneData[v].VehicleID];

				std::map<int, int> l_OperatingModeCount;

				int SequentialLinkNo = m_VehicleDataVector[LaneNo].LaneData[v].SequentialLinkNo ;
				int VehicleLaneBasedCumulativeFlowCount  = pVehicle->m_NodeAry[SequentialLinkNo].LaneBasedCumulativeFlowCount;

				for(int t = m_VehicleDataVector[LaneNo].LaneData[v].StartTime_in_SimulationInterval;
					t < m_VehicleDataVector[LaneNo].LaneData[v].EndTime_in_SimulationInterval; t+=1*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND) 
				{
					//calculate free-flow position
					//xiF(t) = xi(t-τ) + vf(τ)
					prev_distance = distance; 
					distance = min(link_length_in_meter, distance +  m_VehicleDataVector[LaneNo].LaneData[v].FreeflowDistance_per_SimulationInterval);
					//					TRACE("veh %d, time%d,%f\n",v,t,VechileDistanceAry[v][t]);

					//calculate congested position
						float BWTT_in_second  = (link_length_in_meter -  distance)/max(0.001,BackwardWaveSpeed_in_meter_per_second);
						int time_t_minus_BWTT = t - BWTT_in_second*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND; // need to convert time in second to time in simulation time interval
						int DTASimulationTimeIntervalNo = time_t_minus_BWTT*g_DTASimulationInterval;

						if(DTASimulationTimeIntervalNo < 0)
							DTASimulationTimeIntervalNo = 0;
						if(DTASimulationTimeIntervalNo >= m_CumuDeparturelFlow.size() -1)
							DTASimulationTimeIntervalNo =  m_CumuDeparturelFlow.size() -2;

						int DownstreamCumulativeDepartureCount = m_CumuDeparturelFlow [DTASimulationTimeIntervalNo];
						// reference: Newell's 3-detector theory 
							if(DownstreamCumulativeDepartureCount < ( VehicleLaneBasedCumulativeFlowCount- (link_length_in_meter -  distance)/ 1609.344f * m_KJam))  // / 1609.344f converts meter to mile
							{
							distance = prev_distance ; // DO NOT MOVE! jamed by backward wave!
							}
				
							float SpeedBySecond  = (distance - prev_distance)* 0.44704f; // // 1 mph = 0.44704 meters per second

					// output speed per second

					if(t%NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND == 0)  // per_second
					{
						// for active vehicles (with positive speed or positive distance

						// different lanes have different vehicle numbers, so we should not have openMP conflicts here
						float accelation = SpeedBySecond - pVehicle->m_PrevSpeed ;

						int OperatingMode = ComputeOperatingModeFromSpeed(SpeedBySecond, accelation);
						pVehicle->m_OperatingModeCount[OperatingMode]+=1;
						pVehicle->m_PrevSpeed  = SpeedBySecond;

						l_OperatingModeCount[OperatingMode]+=1;

					}

					}  // for each time t

					// calculate subtotal emissions for each vehicle
					CVehicleEmission element(pVehicle->m_VehicleType , pVehicle->m_OperatingModeCount, pVehicle->m_Age );

					// add the vehicle specific emissions to link-based emission statistics
					m_TotalEnergy +=element.Energy;
					m_CO2 += element.CO2;
					m_NOX +=  element.NOX;
					m_CO += element.CO;
					m_HC +=element.HC;

			}  // for active vehicle

	} // for each lane

}


*/




bool g_VehicularSimulation_BasedOnADCurves_Backup(int DayNo, double CurrentTime, int simulation_time_interval_no, int TrafficFlowModelFlag)  // back up version relies on the AD curves to determine # of vehicles to be discharged
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

		// Cumulative flow counts
		DTALink* pLink  = g_LinkVector[li];

		pLink->EntranceBufferSize = 0;
		pLink->NewVehicleCount = 0;
		pLink->ExitVehicleCount = 0;
	}


	// step 1: load vehicles 

#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{

		// Cumulative flow counts
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
				vi.time_stamp = pVeh->m_DepartureTime + pLink->GetFreeMovingTravelTime(TrafficFlowModelFlag, DayNo);  // unit: min

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
//#pragma omp parallel for
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
			float MaximumFlowRate = pLink->m_LaneCapacity *g_DTASimulationInterval/60.0f*pLink->GetNumberOfLanes(DayNo,CurrentTime); //60 --> cap per min --> unit # of vehicle per simulation interval
			// use integer number of vehicles as unit of capacity

			int A_time_index = max(0,current_simulation_time_interval_no-1-pLink->m_FFTT_simulation_interval) % MAX_TIME_INTERVAL_ADCURVE;
			int D_time_index = max(0,current_simulation_time_interval_no-1) % MAX_TIME_INTERVAL_ADCURVE;
			int number_of_vehicles_in_vertical_queue = pLink->A[A_time_index] - pLink->D[D_time_index];

			int cap_value = g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(MaximumFlowRate,li);

			if(TrafficFlowModelFlag==0)  // large capacity value under BPR function mode
				cap_value = 99999;  

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
					t_link_arrival_time= int(pVeh->m_NodeAry[link_sequence_no-1].AbsArrivalTimeOnDSN);
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
				pVeh->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN = ArrivalTimeOnDSN;
				float TravelTime = 0;

				if(link_sequence_no >=1)
				{
					TravelTime= pVeh->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
						pVeh->m_NodeAry[link_sequence_no-1].AbsArrivalTimeOnDSN;
				}else
				{
					TravelTime= pVeh->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
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


					int NextLink = pVeh->m_NodeAry[pVeh->m_SimLinkSequenceNo].LinkNo;
					DTALink* p_Nextlink = g_LinkVector[NextLink];

					if(p_Nextlink->m_FromNodeID != node)
					{
						cout << "p_Nextlink->m_FromNodeID : veh " << pVeh->m_VehicleID <<  "seq: " << pVeh->m_SimLinkSequenceNo << " : " << " this link: " << pLink ->m_FromNodeNumber << "->" << pLink ->m_ToNodeNumber << " next link" << p_Nextlink ->m_FromNodeNumber << "->" << p_Nextlink ->m_ToNodeNumber << ";; current Node: " << g_NodeVector[node].m_NodeNumber;
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

					vi.time_stamp = ArrivalTimeOnDSN + p_Nextlink->GetFreeMovingTravelTime(TrafficFlowModelFlag,DayNo);
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
	// Cumulative flow counts

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

		// Cumulative flow counts
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

		// Cumulative flow counts
		DTALink* pLink  = g_LinkVector[li];

			 pLink->EntranceBufferSize = 0;
			 pLink->NewVehicleCount = 0;
			 pLink->ExitVehicleCount = 0;
	}


				// step 1: load vehicles 

#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{

		// Cumulative flow counts
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
			float MaximumFlowRate = pLink->m_LaneCapacity *g_DTASimulationInterval/60.0f*pLink->GetNumberOfLanes(DayNo,CurrentTime); //60 --> cap per min --> unit # of vehicle per simulation interval
			// use integer number of vehicles as unit of capacity

			int A_time_index = max(0,current_simulation_time_interval_no-1-pLink->m_FFTT_simulation_interval) % MAX_TIME_INTERVAL_ADCURVE;
			int D_time_index = max(0,current_simulation_time_interval_no-1) % MAX_TIME_INTERVAL_ADCURVE;
			int number_of_vehicles_in_vertical_queue = pLink->A[A_time_index] - pLink->D[D_time_index];

			int cap_value = g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(MaximumFlowRate,li);

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
					t_link_arrival_time= int(pVeh->m_NodeAry[link_sequence_no-1].AbsArrivalTimeOnDSN);
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
				pVeh->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN = ArrivalTimeOnDSN;
				float TravelTime = 0;

				if(link_sequence_no >=1)
				{
					TravelTime= pVeh->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
						pVeh->m_NodeAry[link_sequence_no-1].AbsArrivalTimeOnDSN;
				}else
				{
					TravelTime= pVeh->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
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


					int NextLink = pVeh->m_NodeAry[pVeh->m_SimLinkSequenceNo].LinkNo;
					DTALink* p_Nextlink = g_LinkVector[NextLink];

					if(p_Nextlink->m_FromNodeID != node)
					{
						cout << "p_Nextlink->m_FromNodeID : veh " << pVeh->m_VehicleID << ": " << " this link: " << pLink ->m_FromNodeNumber << "->" << pLink ->m_ToNodeNumber << " next link" << p_Nextlink ->m_FromNodeNumber << "->" << p_Nextlink ->m_ToNodeNumber << ";; current Node: " << g_NodeVector[node].m_NodeNumber;
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
	// Cumulative flow counts

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

		// Cumulative flow counts
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


bool g_VehicularSimulation_version_2(int DayNo, double CurrentTime, int simulation_time_interval_no, int TrafficFlowModelFlag)
{
	std::set<DTANode*>::iterator iterNode;
	std::set<DTALink*>::iterator iterLink;
	std::map<int, DTAVehicle*>::iterator iterVM;
	int   PathNodeList[MAX_NODE_SIZE_IN_A_PATH]={0};

	int time_stamp_in_min = int(CurrentTime+0.0001);

	std::list<struc_vehicle_item>::iterator vii;

	int vehicle_id_trace = 8297;
	int link_id_trace = -1;
	bool debug_flag = true ;


	//DTALite:
	// vertical queue data structure
	// each link  ExitQueue, EntranceQueue: (VehicleID, ReadyTime)

	// step 0: /// update VMS path provision

	if(simulation_time_interval_no%(g_information_updating_interval_of_VMS_in_min*10)==0)  //regenerate shortest path tree for VMS every g_information_updating_interval_of_VMS_in_min 
	{
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			DTALink * pLink = g_LinkVector[li];
			for(unsigned int m = 0; m<pLink->MessageSignVector.size(); m++)
			{

				if(g_LinkVector[li]->MessageSignVector[m].StartDayNo  <= DayNo &&
					DayNo <=pLink->MessageSignVector[m].EndDayNo &&
					CurrentTime >=pLink->MessageSignVector[m].StartTime && 
					CurrentTime <=pLink->MessageSignVector[m].EndTime)
				{
					//calculate shortest path
					DTANetworkForSP network(g_NodeVector.size(), g_LinkVector.size(), 1,g_AdjLinkSize);
					float COV_perception_erorr = g_VMSPerceptionErrorRatio;
					network.BuildTravelerInfoNetwork(DayNo,CurrentTime,pLink->m_LinkNo , COV_perception_erorr);
					network.TDLabelCorrecting_DoubleQueue(g_LinkVector[li]->m_ToNodeID ,CurrentTime,1,10,false, false );

					TRACE("\nVMS %d -> %d",g_LinkVector[li]->m_FromNodeNumber ,pLink->m_ToNodeNumber );
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
	#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{
			DTALink * pLink = g_LinkVector[li];

		while(pLink->LoadingBuffer.size() >0 && pLink->GetNumberOfLanes(DayNo,CurrentTime)>0.01)  // no load vehicle into a blocked link
		{
			struc_vehicle_item vi = pLink->LoadingBuffer.front();
			// we change the time stamp here to reflect the actual loading time into the network, especially for blocked link
			if( vi.time_stamp < CurrentTime)
				vi.time_stamp = CurrentTime;

			if(debug_flag && vi.veh_id == vehicle_id_trace )
				TRACE("Step 1: Time %f: Load vhc %d from buffer to physical link %d->%d\n",CurrentTime,vi.veh_id,pLink->m_FromNodeNumber,pLink->m_ToNodeNumber);

			int NumberOfVehiclesOnThisLinkAtCurrentTime = (int)( pLink->EntranceQueue.size() + pLink->ExitQueue.size());

			// determine link in capacity 
			float AvailableSpaceCapacity = pLink->m_VehicleSpaceCapacity - NumberOfVehiclesOnThisLinkAtCurrentTime;

			// if we use BPR function, no density constraint is imposed --> TrafficFlowModelFlag == 0
			if(TrafficFlowModelFlag==0 || AvailableSpaceCapacity >= max(2,pLink->m_VehicleSpaceCapacity*(1-g_MaxDensityRatioForVehicleLoading)))  // at least 10% remaining capacity or 2 vehicle space is left
			{
				pLink->LoadingBuffer.pop_front ();
				pLink->EntranceQueue.push_back(vi);

				pLink->CFlowArrivalCount +=1;


				int pricing_type = g_VehicleMap[vi.veh_id]->m_PricingType ;
				pLink->CFlowArrivalCount_PricingType[pricing_type] +=1;
				pLink->CFlowArrivalRevenue_PricingType[pricing_type] += pLink->GetTollRateInDollar(DayNo,CurrentTime,pricing_type);

				DTAVehicle* pVehicle  = g_VehicleMap[vi.veh_id];

				// add cumulative flow count to vehicle

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

#pragma omp parallel for
	for(int li = 0; li< link_size; li++)
	{

			DTALink * pLink = g_LinkVector[li];

		while(pLink->EntranceQueue.size() >0)
		{

			struc_vehicle_item vi = pLink->EntranceQueue.front();
			double PlannedArrivalTime = vi.time_stamp;

		if(debug_flag && pLink->m_FromNodeNumber == 34 &&  pLink->m_ToNodeNumber == 30 && CurrentTime >=860)
		{
			TRACE("Step 3: Time %f, Link: %d -> %d: entrance queue length: %d, exit queue length %d\n",
				CurrentTime, pLink->m_FromNodeNumber , pLink->m_ToNodeNumber,
				pLink->EntranceQueue.size(),pLink->ExitQueue.size());
		}

			if( g_floating_point_value_less_than_or_eq_comparison(PlannedArrivalTime, CurrentTime) )  // link arrival time within the simulation time interval
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



			float MaximumFlowRate = PerHourCapacityAtCurrentSimulatioInterval *g_DTASimulationInterval/60.0f*pLink->GetNumberOfLanes(DayNo,CurrentTime); //60 --> cap per min --> unit # of vehicle per simulation interval

			float Capacity = MaximumFlowRate;
			// use integer number of vehicles as unit of capacity
			
			pLink-> LinkOutCapacity = g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(Capacity,li);

		if(debug_flag && pLink->m_FromNodeNumber == 34 &&  pLink->m_ToNodeNumber == 30 && CurrentTime >=860)
		{
			TRACE("Step 3: Time %f, Link: %d -> %d: entrance queue length: %d, exit queue length %d, cap = %f, int, %d\n",
				CurrentTime, pLink->m_FromNodeNumber , pLink->m_ToNodeNumber,
				pLink->EntranceQueue.size(), pLink->ExitQueue.size(),Capacity, pLink-> LinkOutCapacity);
		}


			int NumberOfVehiclesOnThisLinkAtCurrentTime = (int)(pLink->CFlowArrivalCount - pLink->CFlowDepartureCount);

			float fLinkInCapacity = 99999.0;

			// TrafficFlowModelFlag == 1 -> point queue model
			if(TrafficFlowModelFlag >=2)  // apply spatial link in capacity for spatial queue models( with spillback and shockwave) 
			{
				// determine link in capacity 
				float AvailableSpaceCapacity = pLink->m_VehicleSpaceCapacity - NumberOfVehiclesOnThisLinkAtCurrentTime;
				fLinkInCapacity = min (AvailableSpaceCapacity, MaximumFlowRate); 
				//			TRACE(" time %5.2f, SC: %5.2f, MFR %5.2f\n",CurrentTime, AvailableSpaceCapacity, MaximumFlowRate);

				// the inflow capcaity is the minimum of (1) incoming maximum flow rate (determined by the number of lanes) and (2) available space capacty  on the link.
				// use integer number of vehicles as unit of capacity

				if(TrafficFlowModelFlag ==3 && g_LinkTypeMap[pLink->m_link_type] .IsFreeway())  // Newell's model on freeway only
				{
					if(simulation_time_interval_no >=pLink->m_BackwardWaveTimeInSimulationInterval ) /// we only apply backward wave checking after the simulation time is later than the backward wave speed interval
					{
						if(debug_flag && link_id_trace == li && CurrentTime >=480)
						{
							TRACE("Step 3: Time %f, Link: %d -> %d: tracing backward wave\n",
								CurrentTime, g_NodeVector[pLink->m_FromNodeID].m_NodeNumber , g_NodeVector[pLink->m_ToNodeID].m_NodeNumber,
								pLink->EntranceQueue.size(), pLink->ExitQueue.size());
						}

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

						int N_Now_minus_1 = (int)pLink->m_CumuArrivalFlow[t_residual_minus_1];
						int Flow_allowed = N_Arrival_Now_Constrainted - N_Now_minus_1;
										TRACE("\ntime %d, D:%d,A%d",simulation_time_interval_no,pLink->m_CumuDeparturelFlow[t_residual_minus_1],pLink->m_CumuArrivalFlow[t_residual_minus_1]);

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
								pLink->m_LinkMOEAry[(int)(CurrentTime)].TrafficStateCode = 2;  // 2: fully congested

								//							TRACE("Queue spillback at %d -> %d\n", g_NodeVector[pLink->m_FromNodeID], g_NodeVector[pLink->m_ToNodeID]);
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
			pLink-> LinkInCapacity= g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(fLinkInCapacity,li);

			if(debug_flag && link_id_trace == li )
			{
				TRACE("Step 3: Time %f, Link: %d -> %d: Incapacity %d, %f, OutCapacity: %d\n", CurrentTime, g_NodeVector[pLink->m_FromNodeID].m_NodeNumber , g_NodeVector[pLink->m_ToNodeID].m_NodeNumber ,pLink-> LinkInCapacity, fLinkInCapacity, pLink-> LinkOutCapacity);
			}
		}


		// distribute link in capacity to different incoming links
		for(unsigned li = 0; li< link_size; li++)
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

				if(TotalInFlowCount > pLink-> LinkInCapacity)  // demand > supply
				{

					if(pLink -> m_bMergeFlag == 1)  // merge with mulitple freeway/onramp links only
					{

						if(g_MergeNodeModelFlag==0)
						{
							// distribute capacity according to number of lanes, defined previously.
						}

						if(g_MergeNodeModelFlag==1)
							// distribute capacity according to number of incoming flow
						{
							for( il = 0; il< pLink->MergeIncomingLinkVector.size(); il++)
							{
								pLink->MergeIncomingLinkVector[il].m_LinkInCapacityRatio = g_LinkVector [ pLink->MergeIncomingLinkVector[il].m_LinkNo]->ExitQueue.size()*1.0f/TotalInFlowCount ;
							}
						}

						for( il = 0; il< pLink->MergeIncomingLinkVector.size(); il++)
						{
							float LinkOutCapacity = pLink-> LinkInCapacity * pLink->MergeIncomingLinkVector[il].m_LinkInCapacityRatio;

							int LinkOutCapacity_int= g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(LinkOutCapacity,li);
							g_LinkVector [pLink->MergeIncomingLinkVector[il].m_LinkNo]->LinkOutCapacity = LinkOutCapacity_int;

						}
					}
					if(pLink -> m_bMergeFlag == 2)  // merge with onramp
					{
						// step a. check with flow on onramp
						float MaxMergeCapacity = g_LinkVector [ pLink->m_MergeOnrampLinkID ]->GetHourlyPerLaneCapacity(CurrentTime)*g_DTASimulationInterval/60.0f*g_LinkVector [ pLink->m_MergeOnrampLinkID ]->GetNumberOfLanes(DayNo,CurrentTime) * 0.5f; //60 --> cap per min --> unit # of vehicle per simulation interval
						// 0.5f -> half of the onramp capacity
						// use integer number of vehicles as unit of capacity

						unsigned int MaxMergeCapacity_int=  g_GetRandomInteger_From_FloatingPointValue_BasedOnLinkIDAndTimeStamp(MaxMergeCapacity,li); 

						unsigned int FlowonOnRamp = g_LinkVector [ pLink->m_MergeOnrampLinkID ]->ExitQueue.size();
						int DownstreamLinkInCapacity = pLink->LinkInCapacity ;

						if(FlowonOnRamp > MaxMergeCapacity_int)  
							// ramp flow > max merge capacity on ramp  
							// over regions I and II in Dr. Rouphail's diagram
							// capacity on ramp = max merge capacity on ramp
							// capacity on main line = LinkInCapacity - capacity on ramp
							// if flow on main line > capacity on main line  // queue on main line
							// elsewise, no queue on mainline
						{
							g_LinkVector [ pLink->m_MergeOnrampLinkID ]->LinkOutCapacity = MaxMergeCapacity_int;
							g_LinkVector [pLink->m_MergeMainlineLinkID] ->LinkOutCapacity = DownstreamLinkInCapacity - MaxMergeCapacity_int;

						}else // ramp flow <= max merge capacity on ramp  // region III  
							// restrict the mainly capacity  // mainly capacity = LinkInCapacity - flow on ramp
						{
							g_LinkVector [ pLink->m_MergeOnrampLinkID ]->LinkOutCapacity = MaxMergeCapacity_int;
							g_LinkVector [pLink->m_MergeMainlineLinkID]->LinkOutCapacity = DownstreamLinkInCapacity - FlowonOnRamp;

						}
						//		g_LogFile << "merge: mainline capacity"<< CurrentTime << " "  << g_LinkVector [pLink->m_MergeMainlineLinkID] ->LinkOutCapacity << endl;

					}
				}
			}
		}
	}


	// step 4: move vehicles from ExitQueue to next link's EntranceQueue, if there is available capacity
	// NewTime = ReadyTime + FFTT(next link)


	// step 4.1: calculate movement capacity per simulation interval for movements defined in input_movement.csv
	// we will not parallel computing mode for this movement capacity now
	int node_size = g_NodeVector.size();
	for(unsigned node = 0; node < node_size; node++)
	{
		DTANode* pNode  = &(g_NodeVector[node]);
			for (std::map<string, DTANodeMovement>::iterator iter = pNode->m_MovementMap.begin(); 
				iter != pNode->m_MovementMap.end(); iter++)
			{
				if( iter->second.movement_hourly_capacity>=0 )  // if data are available
				{

					float movement_hourly_capacity = iter->second.movement_hourly_capacity/60 * g_DTASimulationInterval ;
					iter->second.movement_capacity_per_simulation_interval =  g_GetRandomInteger_SingleProcessorMode(movement_hourly_capacity); // hourly -> min -> 6 seconds);

					iter->second.movement_vehicle_counter = 0; // reset counter of passing vehicles to zero for each simulation interval

					//		TRACE("\n hourly cap: %f, cap per simulation interval %d",movement_hourly_capacity, iter->second.movement_capacity_per_simulation_interval);

				}


			}

	}


	int NextLink;
	DTALink* p_Nextlink;

	//for each node, we scan each incoming link in a randomly sequence, based on simulation_time_interval_no

		#pragma omp parallel for
	for(int node = 0; node < node_size; node++)
	{
		int IncomingLinkSize = g_NodeVector[node].m_IncomingLinkVector.size();
		int incoming_link_count = 0;

		int incoming_link_sequence = simulation_time_interval_no%max(1,IncomingLinkSize);  // random start point

		while(incoming_link_count < IncomingLinkSize)
		{

			int li = g_NodeVector[node].m_IncomingLinkVector[incoming_link_sequence];

			DTALink* pLink = g_LinkVector[li];
			//if(debug_flag && (pLink->m_FromNodeNumber  == 10 && pLink->m_ToNodeNumber == 5) && CurrentTime >=480)
			//{
			//	TRACE("Step 3: Time %f, Link: %d -> %d: tracing \n", CurrentTime, g_NodeVector[pLink->m_FromNodeID].m_NodeNumber , g_NodeVector[pLink->m_ToNodeID].m_NodeNumber);
			//}

			// vehicle_out_count is the minimum of LinkOutCapacity and ExitQueue Size

			int vehicle_out_count = pLink->LinkOutCapacity;

			//			g_LogFile << "link out capaity:"<< CurrentTime << " "  << g_NodeVector[pLink->m_FromNodeID] << " ->" << g_NodeVector[pLink->m_ToNodeID]<<" Cap:" << vehicle_out_count<< "queue:" << pLink->ExitQueue.size() << endl;

			if(pLink->ExitQueue.size() <= pLink-> LinkOutCapacity )
			{      // under capacity, constrained by existing queue
				vehicle_out_count = pLink->ExitQueue.size();
			}

			list<struc_vehicle_item>::iterator exit_queue_it = pLink->ExitQueue.begin();

			while(pLink->ExitQueue.size() >0 && vehicle_out_count >0 && exit_queue_it != pLink->ExitQueue.end())  // go through
			{
				struc_vehicle_item vi = (*exit_queue_it);

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
				// update cumultive lane based flow count
				g_VehicleMap[vehicle_id]->m_NodeAry[link_sequence_no].LaneBasedCumulativeFlowCount =  pLink->CFlowArrivalCount  / pLink->m_NumLanes ;

				// not reach destination yet
				int number_of_links = g_VehicleMap[vehicle_id]->m_NodeSize-1;
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

					if(g_NodeVector[node].m_MovementMap.size()>0)  // check movement capacity if there is an input movement table
					{

					int from_node = pLink->m_FromNodeNumber ;
					int to_node = pLink->m_ToNodeNumber ;
					int dest_node =  p_Nextlink->m_ToNodeNumber ;

					string movement_id = GetMovementStringID(from_node, to_node,dest_node);

					if(g_NodeVector[node].m_MovementMap.find(movement_id) != g_NodeVector[node].m_MovementMap.end()) // the capacity for this movement has been defined
					{

						DTANodeMovement movement_element = g_NodeVector[node].m_MovementMap[movement_id];
						if(movement_element.movement_vehicle_counter >= movement_element.movement_capacity_per_simulation_interval)
						{ // capacity are unavailable

							vehicle_out_count--;

							if(g_FIFOConditionAcrossDifferentMovementFlag==0)  // not enforcing FIFO conditions 
							{
								++exit_queue_it; // move to the next vehicle
								
							continue;  // skip the current vehicle, try the next vehicle

							}else
							{
								break; // not move any vehicles behind this vehicle
							}



						} else
						{
							// move to the next step to check if the link in capacity is available

							//								TRACE("move to the next step to check if the link in capacity is available");

						}

					}

					}  // end of movement checking
					//

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

						float FFTT = p_Nextlink->GetFreeMovingTravelTime(TrafficFlowModelFlag, CurrentTime);

						vi.time_stamp = ArrivalTimeOnDSN + FFTT;

						// remark: when - TimeOnNextLink < 0, it means there are few seconds of left over on current link, which should be spent on next link

///

		if(debug_flag && p_Nextlink->m_FromNodeNumber == 34 &&  p_Nextlink->m_ToNodeNumber == 30 && CurrentTime >=860)
		{
			TRACE("Step 4: Time %f, Link: %d -> %d: vi %d, exit time: %f, FFTT = %f\n",
				CurrentTime, p_Nextlink->m_FromNodeNumber , p_Nextlink->m_ToNodeNumber,
				vi.veh_id , vi.time_stamp, FFTT );
		}

						p_Nextlink->EntranceQueue.push_back(vi);  // move vehicle from current link to the entrance queue of the next link
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

						if(debug_flag && vi.veh_id == vehicle_id_trace )
						{
							TRACE("Step 4: target vehicle: link arrival time %d, total travel time on current link %d->%d, %f\n",t_link_arrival_time, g_NodeVector[pLink->m_FromNodeID].m_NodeNumber, g_NodeVector[pLink->m_ToNodeID].m_NodeNumber, pLink->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime);
						}

						//										TRACE("time %d, total travel time %f\n",t_link_arrival_time, pLink->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime);

						p_Nextlink->LinkInCapacity -=1; // reduce available space capacity by 1

						//remove this vehicle as it has moved to the final destination
						exit_queue_it = pLink->ExitQueue.erase (exit_queue_it);
						vehicle_out_count--;
						continue; // it will not call "++ exit_queue_it" again (in the end of this while loop)



					}else
					{
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
					float ArrivalTimeOnDSN = vi.time_stamp;  // no delay at destination node

					// update statistics for traveled link
					int link_sequence_no = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo;
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


					// finally move to next link
					g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo = g_VehicleMap[vehicle_id]->m_SimLinkSequenceNo+1;


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


				#pragma omp critical  // keep this section as a single thread as it involves network-wide statistics collection
                {
					g_NetworkMOEAry[OriginDepartureTime].AbsArrivalTimeOnDSN_in_a_min +=g_VehicleMap[vehicle_id]->m_TripTime;
					g_Number_of_CompletedVehicles +=1;
					g_NetworkMOEAry[time_stamp_in_min].CumulativeOutFlow = g_Number_of_CompletedVehicles;

				}

					pLink->CFlowDepartureCount +=1;
					pLink->m_LinkMOEAry[t_link_arrival_time].TotalTravelTime  += TravelTime;
					pLink->m_LinkMOEAry[t_link_arrival_time].TotalFlowCount += 1;

					//remove this vehicle as it has moved to the next link
					exit_queue_it = pLink->ExitQueue.erase (exit_queue_it);
					vehicle_out_count--;

					continue; // it will not call "++ exit_queue_it" again (in the end of this while loop)
				}



				++ exit_queue_it;
			}
			incoming_link_count++;
			incoming_link_sequence = (incoming_link_sequence+1) % IncomingLinkSize;  // increase incoming_link_sequence by 1 within IncomingLinkSize
		}  // for each incoming link
	} // for each node

	//	step 5: statistics collection
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

			if(pLink->m_LinkMOEAry [time_stamp_in_min].ExitQueueLength >=1 && pLink->m_LinkMOEAry [time_stamp_in_min].TrafficStateCode != 2)   // not fully congested
				pLink->m_LinkMOEAry [time_stamp_in_min].TrafficStateCode  = 1;  // partially congested

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


bool DTANetworkForSP::TDLabelCorrecting_DoubleQueue_PerPricingType(int origin, int departure_time, int pricing_type=1, float VOT = 10, bool distance_cost_flag = false, bool debug_flag = false)
// time -dependent label correcting algorithm with deque implementation
{
	int i;
	float AdditionalCostInMin = 0;

	if(m_OutboundSizeAry[origin]== 0)
		return false;

	for(i=0; i <m_NodeSize; i++) // Initialization for all nodes
	{

		NodePredVectorPerType[pricing_type][i]  = -1;

		NodeStatusAry[i] = 0;

		LabelTimeAry[i] = MAX_SPLABEL;
		LabelCostVectorPerType[pricing_type][i] = MAX_SPLABEL;

	}

	// Initialization for origin node
	LabelTimeAry[origin] = float(departure_time);
	LabelCostVectorPerType[pricing_type][origin] = 0;

	SEList_clear();
	SEList_push_front(origin);

	int FromID, LinkID, ToID;


	float NewTime, NewCost;
	while(!SEList_empty())
	{
		FromID  = SEList_front();
		SEList_pop_front();

		if(debug_flag )
		{
			if(FromID < m_PhysicalNodeSize)  // physical nodes
			{
				TRACE("\nScan from node %d",g_NodeVector[FromID].m_NodeNumber);
				if(g_NodeVector[FromID].m_NodeNumber == 11)
				{
					TRACE("dest!");
				}

			}
			else
				TRACE("\nScan from node_index %d",FromID);

		}

		NodeStatusAry[FromID] = 2;        //scaned

		for(i=0; i<m_OutboundSizeAry[FromID];  i++)  // for each arc (i,j) belong A(j)
		{
			LinkID = m_OutboundLinkAry[FromID][i];
			ToID = m_OutboundNodeAry[FromID][i];

			if(ToID == origin)
				continue;

			if(debug_flag )
			{
				if(ToID < m_PhysicalNodeSize)  // physical nodes
					TRACE("\n   to node %d", g_NodeVector[ToID].m_NodeNumber);
				else
					TRACE("\n   to node_index %d", ToID);

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
				NewCost    = LabelCostVectorPerType[pricing_type][FromID] + m_LinkTDDistanceAry[LinkID];
			else
				NewCost    = LabelCostVectorPerType[pricing_type][FromID] + m_LinkTDTimeAry[LinkID][link_entering_time_interval] ;

			if(VOT > 0.01 && m_LinkTDCostAry[LinkID][link_entering_time_interval].m_bTollExist) 
			{ // with VOT and toll
				AdditionalCostInMin = m_LinkTDCostAry[LinkID][link_entering_time_interval].TollValue [pricing_type]/VOT * 60.0f;       // 60.0f for 60 min per hour, costs come from time-dependent tolls, VMS, information provisions
				if(debug_flag ) 
					TRACE("AdditionalCostInMin = %f\n",AdditionalCostInMin);

				NewCost += AdditionalCostInMin;
			}


			if(NewCost < LabelCostVectorPerType[pricing_type][ToID] ) // be careful here: we only compare cost not time
			{
				if(debug_flag  && ToID < m_PhysicalNodeSize)  // physical nodes
				{
					TRACE("\n         UPDATE to node %d, cost: %f, link travel time %f", g_NodeVector[ToID].m_NodeNumber, NewCost, m_LinkTDTimeAry[LinkID][link_entering_time_interval]);
				}

				if(NewTime > g_PlanningHorizon -1)
					NewTime = float(g_PlanningHorizon-1);

				LabelTimeAry[ToID] = NewTime;

				LabelCostVectorPerType[pricing_type][ToID] = NewCost;
				if(ToID == 28)
				{
					TRACE("\n node g_NodeVector[ToID].m_NodeNumber: Newcost %f", NewCost);
				}


				NodePredVectorPerType[pricing_type][ToID]   = FromID;

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
//		// Cumulative flow counts
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
//		// Cumulative flow counts
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
//			float MaximumFlowRate = pLink->m_LaneCapacity *g_CarFollowingSimulationInterval/60.0f*pLink->GetNumberOfLanes(DayNo,CurrentTime); //60 --> cap per min --> unit # of vehicle per simulation interval
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
//					t_link_arrival_time= int(pVeh->m_NodeAry[link_sequence_no-1].AbsArrivalTimeOnDSN);
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
//				pVeh->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN = ArrivalTimeOnDSN;
//				float TravelTime = 0;
//
//				if(link_sequence_no >=1)
//				{
//					TravelTime= pVeh->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
//						pVeh->m_NodeAry[link_sequence_no-1].AbsArrivalTimeOnDSN;
//				}else
//				{
//					TravelTime= pVeh->m_NodeAry[link_sequence_no].AbsArrivalTimeOnDSN -
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
//					int NextLink = pVeh->m_NodeAry[pVeh->m_SimLinkSequenceNo].LinkNo;
//					DTALink* p_Nextlink = g_LinkVector[NextLink];
//
//					if(p_Nextlink->m_FromNodeID != node)
//					{
//						cout << "p_Nextlink->m_FromNodeID : veh " << pVeh->m_VehicleID << ": " << " this link: " << pLink ->m_FromNodeNumber << "->" << pLink ->m_ToNodeNumber << " next link" << p_Nextlink ->m_FromNodeNumber << "->" << p_Nextlink ->m_ToNodeNumber << ";; current Node: " << g_NodeVector[node].m_NodeNumber;
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
//	// Cumulative flow counts
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
//		// Cumulative flow counts
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

