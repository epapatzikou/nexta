//  Portions Copyright 2010 Xuesong Zhou, Jinjin Tang

//   If you help write or modify the code, please also list your names here.
//   The reason of having copyright info here is to ensure all the modified version, as a whole, under the GPL 
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

// DTALite.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"
#include "CSVParser.h"

#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>


using namespace std;

extern int g_number_of_warnings ;

void g_ProhibitMovement(int up_node_id, int node_id , int dest_node_id) 
{
			string movement_id = GetMovementStringID(up_node_id, node_id , dest_node_id);

			if(g_NodeNametoIDMap.find(node_id)== g_NodeNametoIDMap.end())
				return;
			int node_no = g_NodeNametoIDMap[node_no];
			g_NodeVector[node_no].m_MovementMap[movement_id].in_link_from_node_id = up_node_id;
			g_NodeVector[node_no].m_MovementMap[movement_id].in_link_to_node_id = node_id ; 
			g_NodeVector[node_no].m_MovementMap[movement_id].out_link_to_node_id = dest_node_id;

			g_NodeVector[node_no].m_MovementMap[movement_id].b_turning_prohibited = true;   // assign movement to individual node

			g_number_of_prohibited_movements++;
}

void g_ReadAMSMovementData()
{

	// read data block per node:
	// movement-specific phase number, effective green time 
	// movement-specific green start time according to standard dual ring sequence

	// look up: 

	// step 1 
	// update node-specific cycle time using AMS_Movement's cycle tim

	// step 2:
	// when read through turn movement, we overwrite link-based m_EffectiveGreenTime_In_Second and m_EffectiveGreenTime_In_Second
	//	pLink->m_EffectiveGreenTime_In_Second ,
	//	pLink->m_GreenStartTime_In_Second,  
	// update saturation flow rate:


	// step 3:
	// if this a left turn movment and left-turn # of lanes >0
	//m_LeftTurn_EffectiveGreenTime_In_Second
	//

	// pLink->m_DownstreamCycleLength_In_Second

	CCSVParser parser_movement;

	int count = 0;
	int zero_effective_green_time_error_count = 0;

	if (parser_movement.OpenCSVFile("AMS_movement.csv",false))  // not required
	{
		while(parser_movement.ReadRecord())
		{
			int up_node_id, node_id, dest_node_id;

			if(parser_movement.GetValueByFieldName("node_id",node_id) == false)
				break;

			std::string timing_plan_name;
			if(parser_movement.GetValueByFieldName("timing_plan_name",timing_plan_name) == false)
				break;

			if(timing_plan_name!="ALLDAY")  // we only simulate a single timing plan now
				continue;

			int middle_node_id = g_NodeNametoIDMap[node_id ];

			parser_movement.GetValueByFieldName("up_node_id",up_node_id);
			parser_movement.GetValueByFieldName("dest_node_id",dest_node_id);

			int CycleLength = 0;
			int Offset = 0;

			parser_movement.GetValueByFieldName("CycleLength",CycleLength);
			parser_movement.GetValueByFieldName("Offset",Offset);

			g_NodeVector[middle_node_id].m_CycleLength_In_Second  = CycleLength;
			g_NodeVector[middle_node_id].m_SignalOffset_In_Second = Offset;
			

			int prohibited_flag = 0;

			parser_movement.GetValueByFieldName ("prohibited_flag",prohibited_flag );


				if(prohibited_flag ==1)
				{
					g_ShortestPathWithMovementDelayFlag = true; // with movement input

						g_ProhibitMovement(up_node_id, node_id , dest_node_id);


						continue; // do not need to check further 
				}

				if(node_id  == 5 && up_node_id == 1)
				{
				TRACE("");
				}

				if(CycleLength <1)  // do not simulate node with zero signal length
				{
				g_NodeVector[middle_node_id].m_ControlType = g_settings.no_signal_control_type_code;
				
				}

				// do not need to read signal timing data 
				if(	g_NodeVector[middle_node_id].m_ControlType != g_settings.pretimed_signal_control_type_code &&
					g_NodeVector[middle_node_id].m_ControlType != g_settings.actuated_signal_control_type_code )
					continue;

				

				std::string turn_type;

				parser_movement.GetValueByFieldName ("turn_type",turn_type );

				string strid = GetLinkStringID(up_node_id,node_id);
				if(g_LinkMap.find(strid)!= g_LinkMap.end())
				{

				float QEM_GreenStartTime = 9999;
				float QEM_GreenEndTime = 0;

				DTALink* pLink = g_LinkMap[GetLinkStringID(up_node_id,node_id)];
				pLink->m_bSignalizedArterialType = true;

					parser_movement.GetValueByFieldName ("GreenStartTime", QEM_GreenStartTime );
					parser_movement.GetValueByFieldName ("GreenEndTime", QEM_GreenEndTime );
				
				float QEM_EffectiveGreenTime  = 10;
					parser_movement.GetValueByFieldName ("EffectiveGreen", QEM_EffectiveGreenTime );


					int QEM_Lanes = 0;
					parser_movement.GetValueByFieldName ("Lanes", QEM_Lanes );
					float SatFlowRatePerLaneGroup = 1800*QEM_Lanes;
					parser_movement.GetValueByFieldName ("SatFlowRatePerLaneGroup", SatFlowRatePerLaneGroup );

					//float SatFlowRatePerLane = SatFlowRatePerLaneGroup/max(1,QEM_Lanes);
					// we use a default value here as the QEM should provide a more reliable value in the future 
					float SatFlowRatePerLane  = 1800;
					if(QEM_Lanes >=1 )
					{
						if(SatFlowRatePerLane <=1) // no value
							SatFlowRatePerLane = 1800; // use default
						else if (SatFlowRatePerLane <=200)
							SatFlowRatePerLane = 200; // set the minimum value

					}
					if(g_SignalRepresentationFlag != 0 && CycleLength >=1 && QEM_GreenStartTime >=  QEM_GreenEndTime && turn_type.find("Right") == string::npos)
					{
						cout << "Movement " <<  up_node_id << " ->" << node_id << " ->" << dest_node_id << 
							" has green time interval" << QEM_GreenStartTime << ", " << QEM_GreenEndTime << endl << "Please check AMS_movement.csv." << endl << "DTALite will simulate this node as no-control." << endl;
						zero_effective_green_time_error_count++;

						g_NodeVector[middle_node_id].m_ControlType = g_settings.no_signal_control_type_code;

					}


				if(turn_type.find("Left") != string::npos )  // the # of lanes and speed for through movements are determined by link attribute
				{

					//find link
					if(QEM_Lanes >= 1 && GetLinkStringID(up_node_id,node_id).size()>0 )
					{
						if(g_LinkMap.find(GetLinkStringID(up_node_id,node_id)) == g_LinkMap.end())  // no such link
							continue; 



						if(QEM_GreenStartTime >= QEM_GreenEndTime && QEM_Lanes >=1)
						{
							// we have to prevent this left-turn movement, as no green time being assigned. 
							g_ProhibitMovement(up_node_id, node_id , dest_node_id);
						
						}

	
						pLink->m_LeftTurn_DestNodeNumber = dest_node_id;
						pLink->m_LeftTurn_NumberOfLanes = QEM_Lanes; 

						pLink->m_LeftTurnGreenStartTime_In_Second = QEM_GreenStartTime;
						pLink->m_LeftTurn_EffectiveGreenTime_In_Second = (QEM_EffectiveGreenTime + QEM_GreenEndTime - QEM_GreenStartTime)/2.0;  // consider the lost time for permitted phases.
						pLink->m_LeftTurn_SaturationFlowRate_In_vhc_per_hour_per_lane = SatFlowRatePerLane ;
						}



				}

				if(turn_type.find("Through") != string::npos )  // the # of lanes and speed for through movements are determined by link attribute
				{

					//find link
					if(QEM_Lanes >= 1 && GetLinkStringID(up_node_id,node_id).size()>0 )
					{
						if(g_LinkMap.find(GetLinkStringID(up_node_id,node_id)) == g_LinkMap.end())  // no such link
							continue; 


						DTALink* pLink = g_LinkMap[GetLinkStringID(up_node_id,node_id)];


						if(g_SignalRepresentationFlag ==1 && CycleLength>=10 && QEM_GreenStartTime >= QEM_GreenEndTime && QEM_Lanes >=1)
						{
							// we have to prevent this left-turn movement, as no green time being assigned. 
							g_ProhibitMovement(up_node_id, node_id , dest_node_id);

							cout << "In file AMS_movement.csv, through movement " << up_node_id << "->" << node_id << "->" <<  dest_node_id << " has no green time being assigned at this signalized node." << endl << "Please check if the left-turn approach has more than 1 lane with positive green time" << endl;

							getchar();
							g_number_of_warnings++;
						
						}

						// take maximum of left-turn and through effective green time as link effective green time (for left and through as default value)
						
							pLink->m_GreenStartTime_In_Second = QEM_GreenStartTime;
							pLink->m_EffectiveGreenTime_In_Second = (QEM_EffectiveGreenTime + QEM_GreenEndTime - QEM_GreenStartTime)/2.0;  // consider the lost time for permitted phases.

							pLink->m_SaturationFlowRate_In_vhc_per_hour_per_lane = SatFlowRatePerLane; // we use link based saturation flow rate
						}



				}

				}
		}
		
	}

		//update link based cycle length and offset

		for(int li = 0; li< g_LinkVector.size(); li++)
		{

			DTALink* pLink = g_LinkVector[li];


				if(	g_NodeVector[pLink->m_ToNodeID].m_ControlType == g_settings.pretimed_signal_control_type_code ||
					g_NodeVector[pLink->m_ToNodeID].m_ControlType == g_settings.actuated_signal_control_type_code )
				{


						int CycleLength_In_Second = g_NodeVector[pLink->m_ToNodeID].m_CycleLength_In_Second;
						int SignalOffSet_In_Second = g_NodeVector[pLink->m_ToNodeID].m_SignalOffset_In_Second;

						if(g_SignalRepresentationFlag == signal_model_movement_effective_green_time && CycleLength_In_Second < 10  )  // use approximate cycle length
						{
							cout << "Input data warning: cycle length for signalized intersection " << g_NodeVector[pLink->m_ToNodeID]. m_NodeNumber << " = "<< CycleLength_In_Second << " seconds."<< endl << "Please any key to continue." << endl;
							getchar ();
							g_number_of_warnings++;

						}


						if(CycleLength_In_Second>=10)
						{
						pLink->m_DownstreamNodeSignalOffset_In_Second = SignalOffSet_In_Second;
						pLink->m_DownstreamCycleLength_In_Second = CycleLength_In_Second;
						}
				}

		}



	if(zero_effective_green_time_error_count >=1)
	{
	
		cout << "press any key to continue..." <<endl;  
	}

	// step 4: from dual ring structure: find the start_time for both through and left-turn movement
	// given input:
	// 1,2,3,4,: 5,6,7,8
	// store internal phase number for each movement: 
	// we use the standard phase sequence 

}