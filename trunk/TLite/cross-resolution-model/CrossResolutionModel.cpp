// TLiteDoc.cpp : implementation of the CTLiteDoc class
//

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html


//    This file is part of NeXTA Version 3 (Open-source).

//    NEXTA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    NEXTA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with NEXTA.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "..//TLite.h"
#include "..//Network.h"
#include "..//TLiteDoc.h"
#include "SignalNode.h"


DTA_Approach CTLiteDoc::g_Angle_to_Approach_New(int angle)
{
      if(angle < 45)
      {
	  return DTA_East;
      }
      else if(angle < 135) 
      {
	   return DTA_North;
      }
	  else if(angle < 225) 
      {
	   return DTA_West;
      }
      else if(angle < 315) 
      {
	   return DTA_South;
	  }
	  else
	    return DTA_East;
}


DTA_Turn CTLiteDoc::g_RelativeAngle_to_Turn_New(int relative_angle)
{
   int min_through_diff = 45;

   int ideal_left = 90;
   int ideal_right = -90;
   int ideal_through = 0;


   if(abs(relative_angle - ideal_through) <= min_through_diff)
		  return DTA_Through;

   if(abs(relative_angle - ideal_left) <= min_through_diff)
		  return DTA_LeftTurn;

   if(abs(relative_angle - ideal_right) <= min_through_diff)
		  return DTA_RightTurn;

   return DTA_OtherTurn;
}


int CTLiteDoc::g_P2P_Angle_New(GDPoint p1, GDPoint p2)
{
   float delta_x  = p2.x - p1.x;
   float delta_y  = p2.y - p1.y;

   if(fabs(delta_x) < 0.00001)
      delta_x =0;

   if(fabs(delta_y) < 0.00001)
      delta_y =0;

   int angle = atan2(delta_y, delta_x) * 180 / PI + 0.5;
   // angle = 90 - angle;

   while(angle < 0) 
	   angle += 360;

   while(angle > 360) 
	   angle -= 360;
   
   return angle;

}

void CTLiteDoc::ConstructMovementVector(bool flag_Template)
{
	if(m_pNetwork!=NULL)
		delete m_pNetwork;

	m_pNetwork = new DTANetworkForSP(m_NodeSet.size(), m_LinkSet.size(), 1, 1, m_AdjLinkSize);  //  network instance for single processor in multi-thread environment
	m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, true, false);

	DTA_NodeMovementSet MovementTemplate; // template with 12 movements
	DTA_NodePhaseSet PhaseTemplate; // template with 8 phases

	//LoadMovementTemplateFile(MovementTemplate, PhaseTemplate);

	if (flag_Template)
	{
		if (!LoadMovementTemplateFile(MovementTemplate, PhaseTemplate))
			LoadMovementDefault(MovementTemplate, PhaseTemplate);
	}
	else
	{
		LoadMovementDefault(MovementTemplate, PhaseTemplate);
	}

	// generate all movements
	int i = 0;
	for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i++)
	{  // for current node
		
		if ((*iNode)->m_ControlType > 0)  //(m_pNetwork->m_InboundSizeAry[i] >= 3) // add node control types
		{
			for(int inbound_i= 0; inbound_i< m_pNetwork->m_InboundSizeAry[i]; inbound_i++)
			{
				// for each incoming link
				for(int outbound_i= 0; outbound_i< m_pNetwork->m_OutboundSizeAry [i]; outbound_i++)
				{
					//for each outging link
					int LinkID = m_pNetwork->m_InboundLinkAry[i][inbound_i];

					if (m_pNetwork->m_FromIDAry[LinkID] != m_pNetwork->m_OutboundNodeAry [i][outbound_i])
					{
						// do not consider u-turn

						DTA_Movement element;

						element.CurrentNodeID = i;						

						element.InboundLinkID = LinkID;
						element.UpNodeID = m_pNetwork->m_FromIDAry[LinkID];
						element.DestNodeID = m_pNetwork->m_OutboundNodeAry [i][outbound_i];

						GDPoint p1, p2, p3;
						p1  = m_NodeIDMap[element.UpNodeID]->pt;
						p2  = m_NodeIDMap[element.CurrentNodeID]->pt;
						p3  = m_NodeIDMap[element.DestNodeID]->pt;

						element.movement_approach = g_Angle_to_Approach_New(g_P2P_Angle_New(p1,p2));
						element.movement_turn = g_PPP_to_Turn(p1,p2,p3);

						// initialize movement here
						switch (element.movement_approach)
						{
							case DTA_North:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.dir = DTA_NBL; break;
									case DTA_Through: element.dir = DTA_NBT; break;
									case DTA_RightTurn: element.dir = DTA_NBR; break;
								}
								break;
							case DTA_East:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.dir = DTA_EBL; break;
									case DTA_Through: element.dir = DTA_EBT; break;
									case DTA_RightTurn: element.dir = DTA_EBR; break;
								}
								break;
							case DTA_South:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.dir = DTA_SBL; break;
									case DTA_Through: element.dir = DTA_SBT; break;
									case DTA_RightTurn: element.dir = DTA_SBR; break;
								}
								break;
							case DTA_West:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.dir = DTA_WBL; break;
									case DTA_Through: element.dir = DTA_WBT; break;
									case DTA_RightTurn: element.dir = DTA_WBR; break;
								}
								break;
						}

						// copy from template
						element.copy_from_MovementSet(MovementTemplate, element.dir);

						// add Movement into m_MovementVector
						bool exist_node_flag = false;
						if (!m_MovementVector.empty())
						{
							for (int m = 0; m < m_MovementVector.size(); m++)
							{
								if (m_MovementVector[m].CurrentNodeID == element.CurrentNodeID)
								{
									element.copy_to_MovementSet(m_MovementVector[m], element.dir);
									exist_node_flag = true;
									break;
								}
							}
						}
						if (!exist_node_flag) // add new DTA_NodeMovementSet
						{
							DTA_NodeMovementSet movement_set;
							movement_set.copy_parameters(MovementTemplate);
							movement_set.CurrentNodeID = element.CurrentNodeID;
							element.copy_to_MovementSet(movement_set, element.dir);
							m_MovementVector.push_back(movement_set);
							
							// generate DTA_NodePhaseSet for this Node
							DTA_NodePhaseSet PhaseSet;
							PhaseSet.copy_parameters(PhaseTemplate);
							PhaseSet.CurrentNodeID = element.CurrentNodeID;
							m_PhaseVector.push_back(PhaseSet);
						}
					}
					
				}

			}
		}
			
	
	}


	if(m_pNetwork!=NULL)
		delete m_pNetwork;

}

bool CTLiteDoc::LoadMovementTemplateFile(DTA_NodeMovementSet& MovementTemplate, DTA_NodePhaseSet& PhaseTemplate)
{
	const int LaneColumeSize = 12;
	const int LaneRowSize = 28;
	string lane_colume_name_str[LaneColumeSize] = { "NBL","NBT","NBR", "SBL", "SBT","SBR","EBL","EBT","EBR", "WBL","WBT","WBR"};
	string lane_row_name_str[LaneRowSize] = {"Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};
	
	TCHAR IniFilePath[_MAX_PATH];
	sprintf_s(IniFilePath,"%sLaneSettings.ini", m_ProjectDirectory);

	int i,j;
	//DTA_NodeMovementSet movement;// = new DTA_NodeMovementSet();
	MovementTemplate.CurrentNodeID = -1;

	for(j=0; j<LaneColumeSize;j++)
	{
		for(i=0; i<LaneRowSize; i++)
		{
			float value = g_GetPrivateProfileFloat(lane_colume_name_str[j].c_str(), lane_row_name_str[i].c_str(), 0, IniFilePath);
			// assign value to DataMatrix[i][j].m_text;
			MovementTemplate.DataMatrix[i+2][j].m_text = value;
		}
		// initial UpNodeID and DestNodeID as -1
		MovementTemplate.DataMatrix[0][j].m_text = -1;
		MovementTemplate.DataMatrix[1][j].m_text = -1;
	}

	const int PhaseColumeSize = 8;
	const int PhaseRowSize = 23;
	string phase_colume_name_str[PhaseColumeSize] = { "D1","D2","D3","D4","D5","D6","D7","D8"};
	string phase_row_name_str[PhaseRowSize] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};
	
	sprintf_s(IniFilePath,"%sPhaseSettings.ini", m_ProjectDirectory);
	PhaseTemplate.CurrentNodeID = -1;
	for(j=0; j<PhaseColumeSize;j++)
	{
		for(i=0; i<PhaseRowSize; i++)
		{
			float value = g_GetPrivateProfileFloat(phase_colume_name_str[j].c_str(), phase_row_name_str[i].c_str(), 0, IniFilePath);
			// assign value to DataMatrix[i][j].m_text;
			PhaseTemplate.DataMatrix[i][j].m_text = value;
		}
	}

	return true;
}

bool CTLiteDoc::LoadMovementDefault(DTA_NodeMovementSet& MovementTemplate, DTA_NodePhaseSet& PhaseTemplate)
{
	// set default value to movements
	const int LaneRowSize = 28;
	const int LaneColumeSize = 12;

	float default_value_M[LaneRowSize][LaneColumeSize] = 
	{
		{1,2,1,1,2,1,1,2,1,1,2,1},
		{0,0,0,0,0,0,0,0,0,0,0,},
		{12,12,12,12,12,12,12,12,12,12,12,12},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,45,0,0,45,0,0,45,0,0,45,0},
		{50,50,50,50,50,50,50,50,50,50,50,50},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{5,2,0,1,6,0,7,4,0,3,8,0},
		{2,0,2,6,0,6,4,0,4,8,0,8},
		{5,2,2,1,6,6,7,4,4,3,8,8},
		{1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900,1900},
		{4,4,4,4,4,4,4,4,4,4,4,4},
		{1863,3539,1863,1863,3539,1863,1863,3539,1863,1863,3539,1863},
		{1863,3539,1863,1863,3539,1863,1863,3539,1863,1863,3539,1863},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{1,1,1,1,1,1,1,1,1,1,1,1},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{20,20,20,20,20,20,20,20,20,20,20,20},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92,0.92},
		{100,100,100,100,100,100,100,100,100,100,100,100},
		{2,2,2,2,2,2,2,2,2,2,2,2},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0}
	};

	int i,j;
	MovementTemplate.CurrentNodeID = -1;

	for(j=0; j<LaneColumeSize;j++)
	{
		for(i=0; i<LaneRowSize; i++)
		{
			MovementTemplate.DataMatrix[i+2][j].m_text = default_value_M[i][j];
		}		
		// initial UpNodeID and DestNodeID as -1
		MovementTemplate.DataMatrix[0][j].m_text = -1;
		MovementTemplate.DataMatrix[1][j].m_text = -1;
	}

	
	// set default values to Phases
	const int PhaseColumeSize = 8;
	const int PhaseRowSize = 23;

	float default_value_P[PhaseRowSize][PhaseColumeSize] = 
	{
		{111,112,211,212,121,122,221,222},
		{4,15,4,15,4,15,4,15},
		{20,40,20,40,20,40,20,40},
		{3,3,3,3,3,3,3,3},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{2,0.2,2,0.2,2,0.2,2,0.2},
		{4,4.5,4,4.5,4,4.5,4,4.5},
		{0,2,0,2,0,2,0,2},
		{0,1,0,1,0,1,0,1},
		{0,5,0,5,0,5,0,5},
		{0,21,0,21,0,21,0,21},
		{0,20,0,20,0,20,0,20},
		{8,32,8,32,8,32,8,32},
		{0,1,0,1,0,1,0,1},
		{1,1,1,1,1,1,1,1},
		{88,105,18,38,88,113,18,48},
		{105,18,38,88,113,18,48,88},
		{101,12,34,82,109,12,44,82},
		{101,123,34,82,109,125,44,82},
		{115,0,45,65,115,8,45,75},
		{128,39,61,109,4,39,71,109},
		{128,18,61,109,4,20,71,109}
	};

	PhaseTemplate.CurrentNodeID = -1;
	for(i=0; i<PhaseRowSize; i++)
	{
		for(j=0; j<PhaseColumeSize;j++)
		{
			PhaseTemplate.DataMatrix[i][j].m_text = default_value_P[i][j];
		}
	}
	return true;
}


void CTLiteDoc::OnFileConstructandexportsignaldata()
{

	ConstructMovementVector(true);

		FILE* st = NULL;

	const int LaneColumeSize = 12;
	const int LaneRowSize = 30;
	string lane_colume_name_str[LaneColumeSize] = { "NBL","NBT","NBR", "SBL", "SBT","SBR","EBL","EBT","EBR", "WBL","WBT","WBR"};
	string lane_row_name_str[LaneRowSize] = {"UpNodeID","DestNodeID","Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};
	
	int i,j, m;
	int movement_size = m_MovementVector.size();

	// write lanes/movements file
	fopen_s(&st,m_ProjectDirectory+"Lanes.csv","w");
	if(st!=NULL)
	{
		fprintf(st, "RECORDNAME,INTID,");
		for(j=0; j<LaneColumeSize;j++)
			fprintf(st, "%s,", lane_colume_name_str[j].c_str());
		fprintf(st,"\n");

		for (m=0; m<movement_size;m++)
		{
			// write UpNodeID and DestNodeID using original m_NodeNumber
			for(i=0; i<2; i++)
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<LaneColumeSize;j++)
				{
					fprintf(st, "%i,",m_NodeIDMap[m_MovementVector[m].DataMatrix[i][j].m_text]->m_NodeNumber);
				}
				fprintf(st,"\n");
			}

			for(i=2; i<LaneRowSize; i++)
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<LaneColumeSize;j++)
				{
					fprintf(st, "%f,",m_MovementVector[m].DataMatrix[i][j].m_text);
				}
				fprintf(st,"\n");
			}
			// fprintf(st,"\n");
		}
		fprintf(st,"\n");
	
	fclose(st);
	}

	// write phase file
	const int PhaseColumeSize = 8;
	const int PhaseRowSize = 23;
	string phase_colume_name_str[PhaseColumeSize] = { "D1","D2","D3","D4","D5","D6","D7","D8"};
	string phase_row_name_str[PhaseRowSize] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};

	int p, phase_size = m_PhaseVector.size();

	fopen_s(&st,m_ProjectDirectory+"Phases.csv","w");
	if(st!=NULL)
	{
		fprintf(st, "RECORDNAME,INTID,");
		for(j=0; j<PhaseColumeSize;j++)
			fprintf(st, "%s,", phase_colume_name_str[j].c_str());
		fprintf(st,"\n");

		for (p=0; p<phase_size;p++)
		{
			for(i=0; i<PhaseRowSize; i++)
			{
				fprintf(st, "%s,", phase_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<PhaseColumeSize;j++)
				{
					fprintf(st, "%f,",m_PhaseVector[p].DataMatrix[i][j].m_text);
				}
				fprintf(st,"\n");
			}
			//fprintf(st,"\n");
		}
		fprintf(st,"\n");
	
	fclose(st);
	}

	EntireNetworkOutput();

}

void CTLiteDoc::EntireNetworkOutput()
{
	// write entire network with signal file
	//if(m_pNetwork!=NULL)
	//	delete m_pNetwork;

	m_pNetwork = new DTANetworkForSP(m_NodeSet.size(), m_LinkSet.size(), 1, 1, m_AdjLinkSize);  //  network instance for single processor in multi-thread environment
	m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, true, false);

	FILE* st = NULL;

	const int LaneColumeSize = 12;
	const int LaneRowSize = 30;
	string lane_colume_name_str[LaneColumeSize] = { "NBL","NBT","NBR", "SBL", "SBT","SBR","EBL","EBT","EBR", "WBL","WBT","WBR"};
	string lane_row_name_str[LaneRowSize] = {"UpNodeID","DestNodeID","Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};
	
	int i,j, m;
	int movement_size = m_MovementVector.size();

	const int PhaseColumeSize = 8;
	const int PhaseRowSize = 23;
	string phase_colume_name_str[PhaseColumeSize] = { "D1","D2","D3","D4","D5","D6","D7","D8"};
	string phase_row_name_str[PhaseRowSize] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};

	int p, phase_size = m_PhaseVector.size();

	fopen_s(&st,m_ProjectDirectory+"Synchro.csv","w");
	if(st!=NULL)
	{
		// write Network /////////////////////////////////////////////////////////////
		fprintf(st, "[Network]\n");
		fprintf(st, "Network Settings\n");
		fprintf(st, "RECORDNAME,DATA,\n");
		fprintf(st, "Metric,%i,\n",0);
		fprintf(st, "yellowTime,%f,\n",3.5);
		fprintf(st, "allRedTime,%f,\n",0.5);
		fprintf(st, "Walk,%i,\n",5);
		fprintf(st, "DontWalk,%i,\n",11);
		fprintf(st, "HV,%f,\n",0.02);
		fprintf(st, "PHF,%i,\n",1);
		fprintf(st, "DefWidth,%i,\n",12);
		fprintf(st, "DefFlow,%i,\n",1900);
		fprintf(st, "vehLength,%i,\n",25);
		fprintf(st, "growth,%i,\n",1);
		fprintf(st, "PedSpeed,%i,\n",4);
		fprintf(st, "LostTimeAdjust,%i,\n",0);
		fprintf(st, "\n");

		// write Nodes /////////////////////////////////////////////////////////////
		fprintf(st, "[Nodes]\n");
		fprintf(st, "Node Data\n");
		fprintf(st, "INTID,TYPE,X,Y,Z,DESCRIPTION,CBD,\n");
		for (i=0; i<m_NodeSet.size(); i++)
		{
			fprintf(st, "%i,%i,%f,%f,%f,%s,%i,\n", m_NodeIDMap[i]->m_NodeNumber, m_NodeIDMap[i]->m_ControlType, m_NodeIDMap[i]->pt.x, m_NodeIDMap[i]->pt.y, 0.0, m_NodeIDMap[i]->m_Name.c_str(), 0);
		}
		fprintf(st,"\n");

		// write Links /////////////////////////////////////////////////////////////
		fprintf(st, "[Links]\n");
		fprintf(st, "Link Data\n");
		fprintf(st, "RECORDNAME,INTID,NB,SB,EB,WB,\n");
		string link_row_name_str[18] = {"Up ID","Lanes","Name","Distance","Speed","Time","Grade","Median","Offset","TWLTL","Crosswalk Width","Mandatory Distance","Mandatory Distance2","Positioning Distance","Positioning Distance2","Curve Pt X","Curve Pt Y","Curve Pt Z"};
		DTA_Approach incoming_approach;
		GDPoint p1, p2;
		int current_node_id, up_node_id;
		long LinkID;
		int i_n = 0;
		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i_n++)
		{  // for current node		
			DTA_NodeBasedLinkSets Node_Link;
			current_node_id = i_n;
			Node_Link.initial(current_node_id);
			for(int inbound_i= 0; inbound_i< m_pNetwork->m_InboundSizeAry[i_n]; inbound_i++)
			{		
				LinkID = m_pNetwork->m_InboundLinkAry[i_n][inbound_i];
				up_node_id = m_pNetwork->m_FromIDAry[LinkID];
				p1  = m_NodeIDMap[up_node_id]->pt;
				p2  = m_NodeIDMap[current_node_id]->pt;
				incoming_approach = g_Angle_to_Approach_New(g_P2P_Angle_New(p1,p2));
				Node_Link.LoadLink(m_LinkNoMap[LinkID], incoming_approach);
			}
			// write file
			// Up_id
			fprintf(st, "%s,", link_row_name_str[0].c_str());
			fprintf(st, "%i,", m_NodeIDMap[current_node_id]->m_NodeNumber);
			for (j=0; j<4; j++)
			{
				if (Node_Link.link_flag[j])
					fprintf(st, "%i,", m_NodeIDMap[Node_Link.Up_ID[j]]->m_NodeNumber);
				else
					fprintf(st, ",");
			}
			fprintf(st,"\n");
			// Lanes
			fprintf(st, "%s,", link_row_name_str[1].c_str());
			fprintf(st, "%i,", m_NodeIDMap[current_node_id]->m_NodeNumber);
			for (j=0; j<4; j++)
			{
				if (Node_Link.link_flag[j])
					fprintf(st, "%i,", Node_Link.Lanes[j]);
				else
					fprintf(st, ",");
			}
			fprintf(st,"\n");
			// name
			fprintf(st, "%s,", link_row_name_str[2].c_str());
			fprintf(st, "%i,", m_NodeIDMap[current_node_id]->m_NodeNumber);
			for (j=0; j<4; j++)
			{
				if (Node_Link.link_flag[j])
					fprintf(st, "%s,", Node_Link.Name[j].c_str());
				else
					fprintf(st, ",");
			}
			fprintf(st,"\n");

			// data matrix
			for(i=0; i<15; i++)
			{
				fprintf(st, "%s,", link_row_name_str[i+3].c_str());
				fprintf(st, "%i,", m_NodeIDMap[current_node_id]->m_NodeNumber);
				for (j=0; j<4; j++)
				{
					if (Node_Link.link_flag[j])
						fprintf(st, "%f,", Node_Link.DataMatrix[i][j]);
					else
						fprintf(st, ",");
				}			
				fprintf(st,"\n");
			}
		}
		fprintf(st,"\n");

		// write Lanes /////////////////////////////////////////////////////////////
		fprintf(st, "[Lanes]\n");
		fprintf(st, "Lane Group Data\n");
		fprintf(st, "RECORDNAME,INTID,");
		for(j=0; j<LaneColumeSize;j++)
			fprintf(st, "%s,", lane_colume_name_str[j].c_str());
		fprintf(st,"\n");

		for (m=0; m<movement_size;m++)
		{
			// write UpNodeID and DestNodeID using original m_NodeNumber
			for(i=0; i<2; i++)
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<LaneColumeSize;j++)
				{
					fprintf(st, "%i,",m_NodeIDMap[m_MovementVector[m].DataMatrix[i][j].m_text]->m_NodeNumber);
				}
				fprintf(st,"\n");
			}

			for(i=2; i<LaneRowSize; i++)
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<LaneColumeSize;j++)
				{
					fprintf(st, "%f,",m_MovementVector[m].DataMatrix[i][j].m_text);
				}
				fprintf(st,"\n");
			}
			// fprintf(st,"\n");
		}
		fprintf(st,"\n");

		// write Timeplans /////////////////////////////////////////////////////////////
		fprintf(st, "[Timeplans]\n");
		fprintf(st, "Timing Plan Settings\n");
		fprintf(st, "RECORDNAME,INTID,DATA,\n");
		DTA_TimePlan time_plan;
		time_plan.initial();
		const int TimePlanRowSize = 10;
		string time_plan_row_name_str[TimePlanRowSize] = { "Control Type","Cycle Length","Lock Timings","Referenced To","Reference Phase","Offset","Master","Yield","Node 0","Node 1"};
		for (p=0; p<phase_size;p++)
		{
			for(i=0; i<TimePlanRowSize; i++)
			{
				fprintf(st, "%s,", time_plan_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_NodeNumber);
				fprintf(st, "%i,", time_plan.DataAry[i]);
				fprintf(st,"\n");
			}
		}
		fprintf(st,"\n");

		// write Phases /////////////////////////////////////////////////////////////
		fprintf(st, "[Phases]\n");
		fprintf(st, "Phasing Data\n");
		fprintf(st, "RECORDNAME,INTID,");
		for(j=0; j<PhaseColumeSize;j++)
			fprintf(st, "%s,", phase_colume_name_str[j].c_str());
		fprintf(st,"\n");

		for (p=0; p<phase_size;p++)
		{
			for(i=0; i<PhaseRowSize; i++)
			{
				fprintf(st, "%s,", phase_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<PhaseColumeSize;j++)
				{
					fprintf(st, "%f,",m_PhaseVector[p].DataMatrix[i][j].m_text);
				}
				fprintf(st,"\n");
			}
			//fprintf(st,"\n");
		}
		fprintf(st,"\n");

	
	fclose(st);
	}
	
	if(m_pNetwork!=NULL)
		delete m_pNetwork;

}