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


		void g_OneShotNetworkLoading()
	{
		int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;
		int link_size  = g_LinkVector.size() + g_NodeVector.size(); // maximal number of links including connectors assuming all the nodes are destinations

		// assign different zones to different processors
		int nthreads = omp_get_max_threads ( );

		/*
		// when debugging use single processor only
		cout << "  Number of processors available = " << omp_get_num_procs ( ) << "\n";
		cout << "  Number of threads =              " << omp_get_max_threads ( ) << "\n";

		g_RealTimeShortestPathCalculation(0);

		CTime EndTime = CTime::GetCurrentTime();
		CTimeSpan ts = EndTime  - StartTime;

		cout << "  real-time info shortest path calculation takes " << ts.GetTotalSeconds()<< " seconds\n";
		cout << "   real-time info shortest path calculation for a node takes " << ts.GetTotalSeconds()*1.0 / node_size << " seconds\n";
		*/
		//	nthreads = 1;
		bool bStartWithEmptyFile = true;
		bool bStartWithEmptyFile_for_assignment = true;

		g_LogFile << "Number of Iterations = " << g_NumberOfIterations << endl;

		int iteration = 0;
		bool NotConverged = true;
		int TotalNumOfVehiclesGenerated = 0;
		std::set<DTALink*>::iterator iterLink;

		// BPR based loading
		for(unsigned li = 0; li< g_LinkVector.size(); li++)
		{
			g_LinkVector[li]->m_BPRLinkTravelTime = g_LinkVector[li]->m_FreeFlowTravelTime*(1.0f+0.15f*(powf(g_LinkVector[li]->m_BPRLinkVolume/(g_LinkVector[li]->m_BPRLaneCapacity*g_LinkVector[li]->GetNumLanes()),4.0f)));
		}
		NetworkLoadingOutput SimuOutput;
		int simulation_mode = 1; // simulation from demand
		SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag, simulation_mode,iteration);
		g_OutputMOEData(iteration);

	}

	int g_OutputSimulationSummary(float& AvgTravelTime, float& AvgDistance, float& AvgSpeed,
		int InformationClass=-1, int DemandType=-1,int DepartureTimeInterval = -1)
	{
		float TravelTimeSum = 0;
		float DistanceSum = 0;
		int VehicleCount = 0;



		for (vector<DTAVehicle*>::iterator v = g_VehicleVector.begin(); v != g_VehicleVector.end();v++)
		{

			if((*v)->m_bComplete )  // vehicle completes the trips
			{
				if( ((*v)->m_InformationClass == InformationClass ||InformationClass==-1)
					&& ((*v)->m_DemandType == DemandType ||DemandType==-1)
					&& ((int)((*v)->m_DepartureTime/ g_AggregationTimetInterval)== (int)(DepartureTimeInterval/g_AggregationTimetInterval) ||DepartureTimeInterval==-1))
				{
					TravelTimeSum += (*v)->m_TripTime;

					DistanceSum+= (*v)->m_Distance;

					VehicleCount++;

				}
			}
		}

		AvgTravelTime = TravelTimeSum/max(1,VehicleCount);  // unit: min
		AvgDistance = DistanceSum/max(1,VehicleCount);  // unit: mile

		AvgSpeed = AvgDistance/max(1,AvgTravelTime)*60;  // unit: mph
		return VehicleCount;
	}


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
	for(i = 0; i< g_ZoneMap[CurZoneID].m_CentroidNodeAry.size(); i++)
	{
		FromID = m_PhysicalNodeSize; // m_PhysicalNodeSize is the centriod number for CurZoneNo // root node
		ToID = g_ZoneMap[CurZoneID].m_CentroidNodeAry [i];

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
		if(zone.m_OriginVehicleSize>0 && iterZone->first !=CurZoneID)   // only this origin zone has vehicles, then we build the network
		{
			for(i = 0; i<  zone.m_CentroidNodeAry.size(); i++)
			{
				FromID = zone.m_CentroidNodeAry [i]; // m_PhysicalNodeSize is the centriod number for CurZoneNo
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

	}
	m_NodeSize = m_PhysicalNodeSize;
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



void g_StaticTrafficAssisnment()
{
		int node_size  = g_NodeVector.size() +1 + g_ODZoneSize;
	int connector_count = 0;

	for (std::map<int, DTAZone>::iterator iterZone = g_ZoneMap.begin(); iterZone != g_ZoneMap.end(); iterZone++)
	{
		connector_count += iterZone->second.m_CentroidNodeAry.size();  // only this origin zone has vehicles, then we build the network
	}

	int link_size  = g_LinkVector.size() + connector_count; // maximal number of links including connectors assuming all the nodes are destinations

		// assign different zones to different processors
		int nthreads = omp_get_max_threads ( );

		bool bStartWithEmptyFile = true;
		bool bStartWithEmptyFile_for_assignment = true;

		g_LogFile << "Number of iterations = " << g_NumberOfIterations << endl;


		int iteration = 0;
		bool NotConverged = true;
		int TotalNumOfVehiclesGenerated = 0;

		// ----------* start of outer loop *----------
		for(iteration=0; NotConverged && iteration < g_NumberOfIterations; iteration++)  // we exit from the loop under two conditions (1) converged, (2) reach maximum number of iterations
		{
			cout << "------- Iteration = "<<  iteration << "--------" << endl;

			// initialize for each iteration
			g_CurrentGapValue = 0.0;
			g_CurrentNumOfVehiclesSwitched = 0;
			g_NewPathWithSwitchedVehicles = 0; 


//#pragma omp parallel for
			for(int CurZoneID=1;  CurZoneID <= g_ODZoneSize; CurZoneID++)
			{
				if(g_ZoneMap[CurZoneID].m_OriginVehicleSize>0)  // only this origin zone has vehicles, then we build the network
				{
					// create network for shortest path calculation at this processor
					DTANetworkForSP network_MP(node_size, link_size, g_PlanningHorizon,g_AdjLinkSize,g_DemandLoadingStartTimeInMin); //  network instance for single processor in multi-thread environment
					int	id = omp_get_thread_num( );  // starting from 0

					cout <<g_GetAppRunningTime()<<  "processor " << id << " is working on assignment at zone  "<<  CurZoneID << endl;

					network_MP.BuildNetworkBasedOnZoneCentriod(CurZoneID);  // build network for this zone, because different zones have different connectors...

					// scan all possible departure times
					for(int departure_time = g_DemandLoadingStartTimeInMin; departure_time < g_DemandLoadingEndTimeInMin; departure_time += g_AggregationTimetInterval)
					{
						if(g_TDOVehicleArray[CurZoneID][departure_time/g_AggregationTimetInterval].VehicleArray.size() > 0)
						{
							bool debug_flag = false;
							if(CurZoneID == 1 && departure_time == 375)
							{
								debug_flag = true;
							
							}
							network_MP.TDLabelCorrecting_DoubleQueue(g_NodeVector.size(),departure_time,1,DEFAULT_VOT,false, debug_flag);  // g_NodeVector.size() is the node ID corresponding to CurZoneNo

							if(g_ODEstimationFlag && iteration>=g_ODEstimation_StartingIteration)  // perform path flow adjustment after at least 10 normal OD estimation
								network_MP.VehicleBasedPathAssignment_ODEstimation(CurZoneID,departure_time,departure_time+g_AggregationTimetInterval,iteration);
							else
								network_MP.VehicleBasedPathAssignment(CurZoneID,departure_time,departure_time+g_AggregationTimetInterval,iteration,debug_flag);

						}
					}
				}
			}

			// below should be single thread


			if(g_ODEstimationFlag && iteration>=g_ODEstimation_StartingIteration)  // re-generate vehicles based on global path set
			{
				g_GenerateVehicleData_ODEstimation();
			}

			cout << "---- Network Loading for Iteration " << iteration <<"----" << endl;

			NetworkLoadingOutput SimuOutput;
			SimuOutput = g_NetworkLoading(g_TrafficFlowModelFlag,0,iteration);

			TotalNumOfVehiclesGenerated = SimuOutput.NumberofVehiclesGenerated; // need this to compute avg gap

			g_AssignmentMOEVector[iteration]  = SimuOutput;

			float PercentageComplete = 0;

			if(SimuOutput.NumberofVehiclesGenerated>0)
				PercentageComplete =  SimuOutput.NumberofVehiclesCompleteTrips*100.0f/SimuOutput.NumberofVehiclesGenerated;

			g_LogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Average Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Average Distance: " << SimuOutput.AvgDistance << ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%"<<endl;
			cout << g_GetAppRunningTime() << "Iteration: " << iteration <<", Average Travel Time: " << SimuOutput.AvgTravelTime << ", Average Distance: " << SimuOutput.AvgDistance<< ", Switch %:" << SimuOutput.SwitchPercentage << ", Number of Vehicles Complete Their Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips << ", " << PercentageComplete << "%"<<endl;

			g_AssignmentLogFile << g_GetAppRunningTime() << "Iteration: " << iteration << ", Ave Travel Time: " << SimuOutput.AvgTravelTime << ", TTI: " << SimuOutput.AvgTTI  << ", Avg Distance: " << SimuOutput.AvgDistance  << ", Switch %:" << SimuOutput.SwitchPercentage <<", Num of Vehicles Completing Trips: " <<  SimuOutput.NumberofVehiclesCompleteTrips<< ", " << PercentageComplete << "%";			


			if(iteration <= 1) // compute relative gap after iteration 1
			{
				g_RelativeGap = 100; // 100%
			}else
			{
				g_RelativeGap = (fabs(g_CurrentGapValue - g_PrevGapValue) / min(1,g_PrevGapValue))*100;
			}
			g_PrevGapValue = g_CurrentGapValue; // update g_PrevGapValue

			if(iteration >= 1) // Note: we output the gap for the last iteration, so "iteration-1"
			{
				float avg_gap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;
				g_AssignmentLogFile << ", Num of Vehicles Switching Paths = " << g_CurrentNumOfVehiclesSwitched << ", Gap at prev iteration = " << g_CurrentGapValue << ", Avg Gap = " << avg_gap << ", Relative Gap at prev iteration = " << g_RelativeGap << "%" 
					<< ", total demand deviation for all paths: " << g_TotalDemandDeviation << ", total measurement deviation for all paths " << g_TotalMeasurementDeviation  << endl;				

			}else
				g_AssignmentLogFile << endl;		

		}	// end of outer loop

		cout << "Writing Vehicle Trajectory and MOE File... " << endl;

		if( iteration == g_NumberOfIterations)
		{ 
			iteration = g_NumberOfIterations -1;  //roll back to the last iteration if the ending condition is triggered by "iteration < g_NumberOfIterations"
		}

		bStartWithEmptyFile = true;
		g_OutputMOEData(iteration);

		g_ComputeFinalGapValue(); // Jason : compute and output final gap
		float avg_gap = g_CurrentGapValue / TotalNumOfVehiclesGenerated;
		g_AssignmentLogFile << "Final Iteration: " << iteration << ", Gap = " << g_CurrentGapValue << ", Avg Gap = " << avg_gap << ", Relative Gap = " << g_RelativeGap << "%" << endl;
}

