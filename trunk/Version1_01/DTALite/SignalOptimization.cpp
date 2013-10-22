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

			g_NodeVector[node_id].m_MovementMap[movement_id].in_link_from_node_id = up_node_id;
			g_NodeVector[node_id].m_MovementMap[movement_id].in_link_to_node_id = node_id ; 
			g_NodeVector[node_id].m_MovementMap[movement_id].out_link_to_node_id = dest_node_id;

			g_NodeVector[node_id].m_MovementMap[movement_id].b_turning_prohibited = true;   // assign movement to individual node

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

				if(node_id  == 84 && up_node_id == 6)
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

				float QEM_GreenStartTime = 9999;
				float QEM_GreenEndTime = 0;

				DTALink* pLink = g_LinkMap[GetLinkStringID(up_node_id,node_id)];
					parser_movement.GetValueByFieldName ("GreenStartTime", QEM_GreenStartTime );
					parser_movement.GetValueByFieldName ("GreenEndTime", QEM_GreenEndTime );

					if(g_SignalRepresentationFlag != 0 && QEM_GreenStartTime >=  QEM_GreenEndTime )
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

						if(QEM_GreenStartTime >= QEM_GreenEndTime && QEM_Lanes >=1)
						{
							// we have to prevent this left-turn movement, as no green time being assigned. 
							g_ProhibitMovement(up_node_id, node_id , dest_node_id);
						
						}

	
						pLink->m_LeftTurn_DestNodeNumber = dest_node_id;
						pLink->m_LeftTurn_NumberOfLanes = QEM_Lanes; 

						pLink->m_LeftTurnGreenStartTime_In_Second = QEM_GreenStartTime;
						pLink->m_LeftTurn_EffectiveGreenTime_In_Second = QEM_GreenEndTime - QEM_GreenStartTime;
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

						if(QEM_GreenStartTime >= QEM_GreenEndTime && QEM_Lanes >=1)
						{
							// we have to prevent this left-turn movement, as no green time being assigned. 
							g_ProhibitMovement(up_node_id, node_id , dest_node_id);

							cout << "In file AMS_movement.csv, through movement " << up_node_id << "->" << node_id << "->" <<  dest_node_id << " has no green time being assigned at this signalized node." << endl << "Please check if the left-turn approach has more than 1 lane with positive green time" << endl;

							getchar();
							g_number_of_warnings++;
						
						}

						// take maximum of left-turn and through effective green time as link effective green time (for left and through as default value)
						
							pLink->m_GreenStartTime_In_Second = QEM_GreenStartTime;
							pLink->m_EffectiveGreenTime_In_Second = QEM_GreenEndTime - QEM_GreenStartTime;

						//					pLink->m_SaturationFlowRate_In_vhc_per_hour_per_lane = QEM_SatFlow; // we use link based saturation flow rate
						}

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
							cout << "Input data warning: cycle length for signalized intersection " << g_NodeVector[pLink->m_ToNodeID]. m_NodeNumber << " = "<< CycleLength_In_Second << " seconds." << endl;
							getchar ();
							g_number_of_warnings++;

						}


						if(CycleLength_In_Second>=10)
						{
						pLink->m_bSignalizedArterialType = true;
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