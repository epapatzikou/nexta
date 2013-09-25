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

void DTANode::QuickSignalOptimization()
{

	if(m_NodeNumber == 54154 || m_NodeNumber == 56161)
		TRACE("");

	if(m_ControlType != g_settings.pretimed_signal_control_type_code && m_ControlType != g_settings.actuated_signal_control_type_code)
		return;

		m_CycleLength_In_Second = g_settings.DefaultCycleTimeSignalOptimization;

//find direction 
		int NSTotalPerLaneCount = 0;
		int EWTotalPerLaneCount = 0;

		int incoming_link_count = 0;

		int NSCapacity = 0;
		int EWCapacity = 0;

		for(incoming_link_count=0;  incoming_link_count <  m_IncomingLinkVector.size(); incoming_link_count++)  // for each incoming link
		{

			int li = m_IncomingLinkVector[incoming_link_count];
			DTALink* pLink = g_LinkVector[li];
				if(pLink->m_Direction == 'N' || pLink->m_Direction == 'S' )
				{
				NSTotalPerLaneCount+= pLink->CFlowArrivalCount;
				NSCapacity += pLink->m_LaneCapacity * pLink->GetNumberOfLanes ();
				}

				if(pLink->m_Direction == 'E' || pLink->m_Direction == 'W' )
				{
				EWTotalPerLaneCount+= pLink->CFlowArrivalCount;
				EWCapacity += pLink->m_LaneCapacity * pLink->GetNumberOfLanes ();
				}
		}


		// no data available
		if(NSTotalPerLaneCount==0 )
		{

			NSTotalPerLaneCount = 0.5*NSCapacity;
		
		}

		if(EWTotalPerLaneCount==0 )
		{

			EWTotalPerLaneCount = 0.5*EWCapacity;
		
		}

		float total_intersection_volume = max(1,NSTotalPerLaneCount + EWTotalPerLaneCount);


		for(incoming_link_count=0;  incoming_link_count <  m_IncomingLinkVector.size(); incoming_link_count++)  // for each incoming link
		{

			int li = m_IncomingLinkVector[incoming_link_count];
			DTALink* pLink = g_LinkVector[li];
				if(pLink->m_Direction == 'N' || pLink->m_Direction == 'S' )
				{
				pLink->m_EffectiveGreenTime_In_Second = (m_CycleLength_In_Second * NSTotalPerLaneCount/total_intersection_volume);
				}

				if(pLink->m_Direction == 'E' || pLink->m_Direction == 'W' )
				{
				pLink->m_EffectiveGreenTime_In_Second = (m_CycleLength_In_Second * EWTotalPerLaneCount/total_intersection_volume);
				}

				if(pLink->m_EffectiveGreenTime_In_Second <=1)  // in case zero effective green 
					pLink->m_EffectiveGreenTime_In_Second = 5;

		}

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

			int middle_node_id = g_NodeNametoIDMap[node_id ];

			parser_movement.GetValueByFieldName("up_node_id",up_node_id);
			parser_movement.GetValueByFieldName("dest_node_id",dest_node_id);


			int prohibited_flag = 0;

			parser_movement.GetValueByFieldName ("prohibited_flag",prohibited_flag );


				if(prohibited_flag ==1)
				{
					g_ShortestPathWithMovementDelayFlag = true; // with movement input
						string movement_id = GetMovementStringID(up_node_id, node_id , dest_node_id);


						g_NodeVector[middle_node_id].m_MovementMap[movement_id].in_link_from_node_id = up_node_id;
						g_NodeVector[middle_node_id].m_MovementMap[movement_id].in_link_to_node_id = node_id ; 
						g_NodeVector[middle_node_id].m_MovementMap[movement_id].out_link_to_node_id = dest_node_id;

						g_NodeVector[middle_node_id].m_MovementMap[movement_id].b_turning_prohibited = true;   // assign movement to individual node

						g_number_of_prohibited_movements++;

						continue; // do not need to check further 
				}

				if(node_id  == 76 && up_node_id == 141)
				{
				TRACE("");
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

				int QEM_EffectiveGreen = 0;
				DTALink* pLink = g_LinkMap[GetLinkStringID(up_node_id,node_id)];
					parser_movement.GetValueByFieldName ("QEM_EffectiveGreen", QEM_EffectiveGreen );

					if(QEM_EffectiveGreen == 0 )
					{
						cout << "Movement " <<  up_node_id << " ->" << node_id << " ->" << dest_node_id << " has an effective green time of 0. Please check # of lanes for this movement." << endl;
						zero_effective_green_time_error_count++;
					}


				if(turn_type.find("Left") != string::npos )  // the # of lanes and speed for through movements are determined by link attribute
				{
					int QEM_Lanes = 0;
					int QEM_SatFlow = 1900;
					parser_movement.GetValueByFieldName ("QEM_Lanes", QEM_Lanes );

					//find link
					if(QEM_Lanes >= 1 && GetLinkStringID(up_node_id,node_id).size()>0 )
					{
						if(g_LinkMap.find(GetLinkStringID(up_node_id,node_id)) == g_LinkMap.end())  // no such link
							continue; 



						if(pLink->m_bSignalizedArterialType == true )  // only for arterial streets
						{

	
						// take maximum of left-turn and through effective green time as link effective green time (for left and through as default value)
						pLink->m_EffectiveGreenTime_In_Second = max(QEM_EffectiveGreen,pLink->m_EffectiveGreenTime_In_Second );

						pLink->m_LeftTurn_DestNodeNumber = dest_node_id;
						pLink->m_LeftTurn_NumberOfLanes = QEM_Lanes; 
						pLink->m_LeftTurn_EffectiveGreenTime_In_Second = QEM_EffectiveGreen;
						pLink->m_LeftTurn_SaturationFlowRate_In_vhc_per_hour_per_lane = pLink->m_SaturationFlowRate_In_vhc_per_hour_per_lane ;
						}

					}


				}

				if(turn_type.find("Through") != string::npos )  // the # of lanes and speed for through movements are determined by link attribute
				{
					int QEM_Lanes = 0;
					int QEM_SatFlow = 1900;
					parser_movement.GetValueByFieldName ("QEM_Lanes", QEM_Lanes );

					//find link
					if(QEM_Lanes >= 1 && GetLinkStringID(up_node_id,node_id).size()>0 )
					{
						if(g_LinkMap.find(GetLinkStringID(up_node_id,node_id)) == g_LinkMap.end())  // no such link
							continue; 


						DTALink* pLink = g_LinkMap[GetLinkStringID(up_node_id,node_id)];

						if(pLink->m_bSignalizedArterialType == true )  // only for arterial streets
						{

						// take maximum of left-turn and through effective green time as link effective green time (for left and through as default value)
						pLink->m_EffectiveGreenTime_In_Second = max(QEM_EffectiveGreen,pLink->m_EffectiveGreenTime_In_Second );

						//					pLink->m_SaturationFlowRate_In_vhc_per_hour_per_lane = QEM_SatFlow; // we use link based saturation flow rate
						}

					}


				}

				}
		}
		
	}

	if(zero_effective_green_time_error_count >=1)
	{
	g_ProgramStop();
	}

	// step 4: from dual ring structure: find the start_time for both through and left-turn movement
	// given input:
	// 1,2,3,4,: 5,6,7,8
	// store internal phase number for each movement: 
	// we use the standard phase sequence 

}