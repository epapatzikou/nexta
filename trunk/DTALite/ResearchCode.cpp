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
		int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;
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
							network_MP.m_LinkTDTimeAry[g_LinkVector[li]->m_LinkID][0] = TravelTime;
//							network_MP.m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][0]= 0;
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
						network_MP.m_LinkTDTimeAry[g_LinkVector[li]->m_LinkID][0] = TravelTime;
//						network_MP.m_LinkTDCostAry[g_LinkVector[li]->m_LinkID][0]=  TravelTime;
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
					g_LinkVector[li]->m_BPRLinkTravelTime = g_LinkVector[li]->m_FreeFlowTravelTime*(1.0f+0.15f*(powf(g_LinkVector[li]->m_BPRLinkVolume/(g_LinkVector[li]->m_BPRLaneCapacity*g_LinkVector[li]->GetNumLanes()),4.0f)));
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
			//		g_AssignmentLogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Ave Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Avg Distance: " << SimuOutput.AvgDistance  << ", Switch %:" << SimuOutput.SwitchPercentage <<", Num of Vehicles Completing Trips: ";

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
							int NodeName = g_NodeVector[NodeID].m_NodeName;

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
		PathArray = new PathArrayForEachODT[g_ODZoneSize + 1]; // remember to release memory

		ConstructPathArrayForEachODT(PathArray, zone, AssignmentInterval); 

		// loop through the TDOVehicleArray to assign or update vehicle paths...
		for (int vi = 0; vi<g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray .size(); vi++)
		{
			int VehicleID = g_TDOVehicleArray[zone][AssignmentInterval].VehicleArray[vi];
			DTAVehicle* pVeh  = g_VehicleMap[VehicleID];
			ASSERT(pVeh!=NULL);

			bool bSwitchFlag = false;
			pVeh->m_bSwitched = false;

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

				if(pVeh->m_TripTime > MinTime + g_TravelTimeDifferenceForSwitching)
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

				pVeh->m_bSwitched = true;

				// get shortest path only when bSwitchFlag is true; no need to obtain shortest path for every vehicle
				NodeSize = PathArray[VehicleDest].PathSize[PathArray[VehicleDest].BestPathIndex];			

				pVeh->m_NodeSize = NodeSize;

				if( pVeh->m_aryVN !=NULL)
				{
					delete pVeh->m_aryVN;
				}

				if(pVeh->m_NodeSize>=2)
				{
					pVeh->m_bSwitched = true;

					if(NodeSize>=900)
					{
						cout << "PATH Size >900 " << NodeSize;
						g_ProgramStop();
					}

					pVeh->m_aryVN = new SVehicleLink[NodeSize];

					if(pVeh->m_aryVN==NULL)
					{
						cout << "Insufficient memory for allocating vehicle arrays!";
						g_ProgramStop();
					}

					pVeh->m_NodeNumberSum = PathArray[VehicleDest].PathNodeSums[PathArray[VehicleDest].BestPathIndex];

					pVeh->m_Distance =0;

					for(int i = 0; i< NodeSize-1; i++)
					{
						pVeh->m_aryVN[i].LinkID = PathArray[VehicleDest].PathLinkSequences[PathArray[VehicleDest].BestPathIndex][i];					
						pVeh->m_Distance+= g_LinkVector[pVeh->m_aryVN[i].LinkID]->m_Length;
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

				for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
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

				g_LogFile << g_GetAppRunningTime() << "Inner Loop Iteration: " << inner_iteration << ", Average Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Average Distance: " << SimuOutput.AvgDistance << ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%"<<endl;
				cout << g_GetAppRunningTime() << "Inner Loop Iteration: " << inner_iteration <<", Average Travel Time: " << SimuOutput.AvgTravelTime << ", Average Distance: " << SimuOutput.AvgDistance<< ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips << ", " << PercentageComplete << "%"<<endl;

				g_AssignmentLogFile << g_GetAppRunningTime() << "Inner Loop Iteration: " << inner_iteration << ", Ave Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Avg Distance: " << SimuOutput.AvgDistance  << ", Switch %:" << SimuOutput.SwitchPercentage <<", Num of Vehicles Completing Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%";			

				// check inner loop convergence
				g_RelativeGap = (fabs(g_CurrentGapValue - g_PrevGapValue) / g_PrevGapValue)*100;

				if(g_UEAssignmentMethod <= 1) // MSA and day-to-day learning
				{
					if(g_CurrentNumOfVehiclesSwitched < g_ConvergenceThreshold_in_Num_Switch)
						NotConvergedInner = false; // converged!
				}else // gap-based approaches
				{	
					if(g_RelativeGap < g_ConvergencyRelativeGapThreshold_in_perc)
						NotConvergedInner = false; // converged! 
				}

				g_PrevGapValue = g_CurrentGapValue; // update g_PrevGapValue

				float avg_gap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;
				g_AssignmentLogFile << ", Num of Vehicles Switching Paths = " << g_CurrentNumOfVehiclesSwitched << ", Gap at prev iteration = " << g_CurrentGapValue << ", Avg Gap = " << avg_gap << ", Relative Gap at prev iteration = " << g_RelativeGap << "%" << endl;		

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

	m_NodeSize = m_PhysicalNodeSize + 1 + g_ODZoneSize;


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




/* one one-shot simulation from demand 
		// generating paths for historical travel information

		if( SimulationMode == 1 // one-shot simulation from demand //)
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
				int VehicleLaneBasedCumulativeFlowCount  = pVehicle->m_aryVN[SequentialLinkNo].LaneBasedCumulativeFlowCount;

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
					CVehicleEmission element(pVehicle->m_VehicleType , pVehicle->m_OperatingModeCount );

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
			float MaximumFlowRate = pLink->m_MaximumServiceFlowRatePHPL *g_DTASimulationInterval/60.0f*pLink->GetNumLanes(DayNo,CurrentTime); //60 --> cap per min --> unit # of vehicle per simulation interval
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
						cout << "p_Nextlink->m_FromNodeID : veh " << pVeh->m_VehicleID <<  "seq: " << pVeh->m_SimLinkSequenceNo << " : " << " this link: " << pLink ->m_FromNodeNumber << "->" << pLink ->m_ToNodeNumber << " next link" << p_Nextlink ->m_FromNodeNumber << "->" << p_Nextlink ->m_ToNodeNumber << ";; current Node: " << g_NodeVector[node].m_NodeName;
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

