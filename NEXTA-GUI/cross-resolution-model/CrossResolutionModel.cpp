//  Portions Copyright 2010 Tao Xing (captainxingtao@gmail.com), Xuesong Zhou (xzhou99@gmail.com)
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
#include "..//Data-Interface//XLEzAutomation.h"
#include "..//Data-Interface//XLTestDataSource.h"
#include "..//Data-Interface//include//ogrsf_frmts.h"
#include "SignalNode.h"
#include "..//Dlg_SignalDataExchange.h"


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


DTA_Turn CTLiteDoc::Find_RelativeAngle_to_Turn(int relative_angle)
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


int CTLiteDoc::Find_P2P_Angle(GDPoint p1, GDPoint p2)
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
DTA_Turn CTLiteDoc::Find_PPP_to_Turn(GDPoint p1, GDPoint p2, GDPoint p3)
{

	int relative_angle = Find_PPP_RelativeAngle(p1,p2,p3);
	return Find_RelativeAngle_to_Turn(relative_angle);

}
int CTLiteDoc::Find_PPP_RelativeAngle(GDPoint p1, GDPoint p2, GDPoint p3)
{
 int relative_angle;

 int angle1 = Find_P2P_Angle(p1,p2);
 int angle2 = Find_P2P_Angle(p2,p3);
 relative_angle = angle2 - angle1;

      while(relative_angle > 180) 
		  relative_angle -= 360;

      while(relative_angle < -180)
		  relative_angle += 360;

 return relative_angle;
}

int CTLiteDoc::FindUniqueLinkID()
{
	int i;
	for( i= 1; i < m_LinkIDRecordVector.size(); i++) 
	{
		if( m_LinkIDRecordVector[i] == 0)
		{
			m_LinkIDRecordVector[i] = 1;
			return i;
		}
	}

	// all link ids have been used;
	m_LinkIDRecordVector.push_back(i);
	m_LinkIDRecordVector[i] = 1;
	return i;
}
void CTLiteDoc::AssignUniqueLinkIDForEachLink()
{

	
	std::list<DTALink*>::iterator iLink;

	int max_link_id = 1;
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				if((*iLink)->m_LinkID >  max_link_id)
				{
					max_link_id = (*iLink)->m_LinkID ;
				}
			 
			}

			//allocate unique id arrary

			m_LinkIDRecordVector.clear();

			max_link_id = max(max_link_id, m_LinkSet.size());
			for(int i = 0; i <= max_link_id; i++)
			{
			m_LinkIDRecordVector.push_back(0);
			}


		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_LinkID != 0)  // with possible real id
			{
				if(m_LinkIDRecordVector[(*iLink)->m_LinkID] == 0)  // not been used before
				{
				// mark link id
					m_LinkIDRecordVector[(*iLink)->m_LinkID] =1;
				}else
				{  // link id has been used
				(*iLink)->m_LinkID = 0; // need to reset link id;
				}

			}

		}

		// last step: find unique id 
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_LinkID == 0)
			{
					// need a new unique link id
				(*iLink)->m_LinkID = FindUniqueLinkID();
			
			}
		}

}

void  CTLiteDoc::ConstructMovementVectorForEachNode()
{

	m_AdjLinkSize = 0;

	for (std::list<DTANode*>::iterator iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		if((*iNode)->m_Connections >0)  // we might try import node layer only from shape file, so all nodes have no connected links. 
		{
			if((*iNode)->m_Connections > m_AdjLinkSize)
				m_AdjLinkSize = (*iNode)->m_Connections;

		}

	}

		m_Network.Initialize (m_NodeSet.size(), m_LinkSet.size(), 1, m_AdjLinkSize);
		m_Network.BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, false);

	// generate all movements
	int i = 0;
	for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i++)
	{  // for current node
		
			// scan each inbound link and outbound link

			// generate two default phases;
			(*iNode)->m_PhaseVector.clear();

			DTANodePhase node_phase;
			(*iNode)->m_PhaseVector.push_back(node_phase); // phase 1;
			(*iNode)->m_PhaseVector.push_back(node_phase); // phase 2;


			int inbound_i;

			for(inbound_i= 0; inbound_i< m_Network.m_InboundSizeAry[i]; inbound_i++)
			{
				// for each incoming link 
				for(int outbound_i= 0; outbound_i< m_Network.m_OutboundSizeAry [i]; outbound_i++)
				{
					//for each outging link
					int LinkID = m_Network.m_InboundLinkAry[i][inbound_i];

					if (m_Network.m_FromIDAry[LinkID] != m_Network.m_OutboundNodeAry [i][outbound_i])
					{
						// do not consider u-turn

						DTANodeMovement element;

						element.in_link_to_node_id = i;						

						element.IncomingLinkID = m_Network.m_InboundLinkAry[i][inbound_i];;
						element.OutgoingLinkID = m_Network.m_OutboundLinkAry [i][outbound_i];

						element.in_link_from_node_id = m_Network.m_FromIDAry[LinkID];
						element.out_link_to_node_id = m_Network.m_OutboundNodeAry [i][outbound_i];

						GDPoint p1, p2, p3;
						p1  = m_NodeIDMap[element.in_link_from_node_id]->pt;
						p2  = m_NodeIDMap[element.in_link_to_node_id]->pt;
						p3  = m_NodeIDMap[element.out_link_to_node_id]->pt;

						element.movement_approach = g_Angle_to_Approach_New(Find_P2P_Angle(p1,p2));
						// movement_approach is for in_bound link
						DTALink* pLink = m_LinkNoMap[LinkID];
						if(pLink!=NULL)
						{
							// we have not considered different directions/approach with respect to from and to nodes (e.g. with a curve)
							pLink ->m_FromApproach = element.movement_approach;
							pLink ->m_ToApproach = g_Angle_to_Approach_New(Find_P2P_Angle(p2,p1));
						}


						element.movement_turn = Find_PPP_to_Turn(p1,p2,p3);

						// determine  movement type /direction here
						element.movement_dir = DTA_LANES_COLUME_init;

						int PhaseIndex  = 0;
						switch (element.movement_approach)
						{
							case DTA_North:
								PhaseIndex = 0;
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.movement_dir = DTA_NBL; break;
									case DTA_Through: element.movement_dir = DTA_NBT; break;
									case DTA_RightTurn: element.movement_dir = DTA_NBR; break;
								}
								break;
							case DTA_East:
								PhaseIndex = 1;
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.movement_dir = DTA_EBL; break;
									case DTA_Through: element.movement_dir = DTA_EBT; break;
									case DTA_RightTurn: element.movement_dir = DTA_EBR; break;
								}
								break;
							case DTA_South:
								PhaseIndex = 0;
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.movement_dir = DTA_SBL; break;
									case DTA_Through: element.movement_dir = DTA_SBT; break;
									case DTA_RightTurn: element.movement_dir = DTA_SBR; break;
								}
								break;
							case DTA_West:
								PhaseIndex = 1;
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.movement_dir = DTA_WBL; break;
									case DTA_Through: element.movement_dir = DTA_WBT; break;
									case DTA_RightTurn: element.movement_dir = DTA_WBR; break;
								}
								break;
						}

						int movement_index = (*iNode)->m_MovementVector.size();
						element.phase_index = PhaseIndex;
						(*iNode)->m_MovementVector.push_back(element);

						//record the movement index into the right phase index
						(*iNode)->m_PhaseVector[PhaseIndex].movement_index_vector .push_back(movement_index);
						

					}  // for each feasible movement (without U-turn)
					
				} // for each outbound link

			} // for each inbound link
	}// for each node

}

void CTLiteDoc::ConstructMovementVector(bool flag_Template)
{

	m_MovementVector.clear();

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
		
		if ((*iNode)->m_ControlType > 0)  //(m_Network.m_InboundSizeAry[i] >= 3) // add node control types
		{
			// generate movement set and phase set

			DTA_NodeMovementSet movement_set;	



			// scan each inbound link and outbound link

			for(int inbound_i= 0; inbound_i< m_Network.m_InboundSizeAry[i]; inbound_i++)
			{
				// for each incoming link
				for(int outbound_i= 0; outbound_i< m_Network.m_OutboundSizeAry [i]; outbound_i++)
				{
					//for each outging link
					int LinkID = m_Network.m_InboundLinkAry[i][inbound_i];

					if (m_Network.m_FromIDAry[LinkID] != m_Network.m_OutboundNodeAry [i][outbound_i])
					{
						// do not consider u-turn

						DTA_Movement element;

						element.CurrentNodeID = i;						

						element.InboundLinkID = LinkID;
						element.UpNodeID = m_Network.m_FromIDAry[LinkID];
						element.DestNodeID = m_Network.m_OutboundNodeAry [i][outbound_i];

						GDPoint p1, p2, p3;
						p1  = m_NodeIDMap[element.UpNodeID]->pt;
						p2  = m_NodeIDMap[element.CurrentNodeID]->pt;
						p3  = m_NodeIDMap[element.DestNodeID]->pt;

						element.movement_approach = g_Angle_to_Approach_New(Find_P2P_Angle(p1,p2));
						element.movement_turn = Find_PPP_to_Turn(p1,p2,p3);

						// determine  movement type /direction here
						element.movement_dir = DTA_LANES_COLUME_init;
						switch (element.movement_approach)
						{
							case DTA_North:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.movement_dir = DTA_NBL; break;
									case DTA_Through: element.movement_dir = DTA_NBT; break;
									case DTA_RightTurn: element.movement_dir = DTA_NBR; break;
								}
								break;
							case DTA_East:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.movement_dir = DTA_EBL; break;
									case DTA_Through: element.movement_dir = DTA_EBT; break;
									case DTA_RightTurn: element.movement_dir = DTA_EBR; break;
								}
								break;
							case DTA_South:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.movement_dir = DTA_SBL; break;
									case DTA_Through: element.movement_dir = DTA_SBT; break;
									case DTA_RightTurn: element.movement_dir = DTA_SBR; break;
								}
								break;
							case DTA_West:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.movement_dir = DTA_WBL; break;
									case DTA_Through: element.movement_dir = DTA_WBT; break;
									case DTA_RightTurn: element.movement_dir = DTA_WBR; break;
								}
								break;
						}

						if (element.movement_dir > 0)
						{
							// copy from template
							MovementTemplate.copy_to_Movement(element, element.movement_dir);
						}

						// add Movement into m_MovementVector
						//TRACE("current node: %d, dir = %d\n", element.CurrentNodeID, element.movement_dir);

//						movement_set.MovementMatrix[element.movement_dir] = element;

					//	(*iNode)->m_MovementSet.MovementMatrix[element.movement_dir] = element;
						

					}  // for each feasible movement (without U-turn)
					
				} // for each outbound link

			} // for each inbound link

			movement_set.CurrentNodeID = i;
			m_MovementVector.push_back(movement_set);  // m_MovementVector for all nodes in the network
			TRACE("current node: %d\n", movement_set.CurrentNodeID);

		} // checking control type
	}// for each node
}

bool CTLiteDoc::LoadMovementTemplateFile(DTA_NodeMovementSet& MovementTemplate, DTA_NodePhaseSet& PhaseTemplate)
{
	const int LaneColumnSize = 12;
	const int LaneRowSize = 28;
	string lane_Column_name_str[LaneColumnSize] = { "NBL","NBT","NBR", "SBL", "SBT","SBR","EBL","EBT","EBR", "WBL","WBT","WBR"};
	string lane_row_name_str[LaneRowSize] = {"Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};
	
	TCHAR IniFilePath[_MAX_PATH];
	sprintf_s(IniFilePath,"%sLaneSettings.ini", m_Synchro_ProjectDirectory);

	int i,j;
	//DTA_NodeMovementSet movement;// = new DTA_NodeMovementSet();
	MovementTemplate.CurrentNodeID = -1;

	for(j=0; j<LaneColumnSize;j++)
	{
		for(i=0; i<LaneRowSize; i++)
		{
			float value = g_GetPrivateProfileFloat(lane_Column_name_str[j].c_str(), lane_row_name_str[i].c_str(), 0, IniFilePath);
			// assign value to DataMatrix[i][j].m_text;
			MovementTemplate.DataMatrix[i+2][j].m_text = value;
		}
		// initial UpNodeID and DestNodeID as -1
		MovementTemplate.DataMatrix[0][j].m_text = -1;
		MovementTemplate.DataMatrix[1][j].m_text = -1;
	}

	const int PhaseColumnSize = 8;
	const int PhaseRowSize = 23;
	string phase_Column_name_str[PhaseColumnSize] = { "D1","D2","D3","D4","D5","D6","D7","D8"};
	string phase_row_name_str[PhaseRowSize] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};
	
	sprintf_s(IniFilePath,"%sPhaseSettings.ini", m_Synchro_ProjectDirectory);
	PhaseTemplate.CurrentNodeID = -1;
	for(j=0; j<PhaseColumnSize;j++)
	{
		for(i=0; i<PhaseRowSize; i++)
		{
			float value = g_GetPrivateProfileFloat(phase_Column_name_str[j].c_str(), phase_row_name_str[i].c_str(), 0, IniFilePath);
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
	const int LaneColumnSize = 12;

	double default_value_M[LaneRowSize][LaneColumnSize] = 
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

	for(j=0; j<LaneColumnSize;j++)
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
	const int PhaseColumnSize = 8;
	const int PhaseRowSize = 23;

	double default_value_P[PhaseRowSize][PhaseColumnSize] = 
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
		for(j=0; j<PhaseColumnSize;j++)
		{
			PhaseTemplate.DataMatrix[i][j].m_text = default_value_P[i][j];
		}
	}
	return true;
}


void CTLiteDoc::OnFileConstructandexportsignaldata()
{
	CDlg_SignalDataExchange dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();

}
void CTLiteDoc::Constructandexportsignaldata()
{
	CString str;
	CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"Synchro Data File (*.csv)|*.csv||", NULL);
	if(dlg.DoModal() == IDOK)
	{

		CWaitCursor wait;
		char fname[_MAX_PATH];
		wsprintf(fname,"%s", dlg.GetPathName());

		CString SynchroProjectFile = dlg.GetPathName();
		m_Synchro_ProjectDirectory  = SynchroProjectFile.Left(SynchroProjectFile.ReverseFind('\\') + 1);

		m_Network.Initialize (m_NodeSet.size(), m_LinkSet.size(), 1, m_AdjLinkSize);
		m_Network.BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, false);

		ConstructMovementVector(true);

		ExportSingleSynchroFile(SynchroProjectFile);

		ExportSynchroVersion6Files();

		}

}


void CTLiteDoc::ExportSynchroVersion6Files()
{
			FILE* st = NULL;

		// write lanes/movements file
		const int LaneColumnSize = 12;
		const int LaneRowSize = 30;
		string lane_Column_name_str[LaneColumnSize] = { "NBL","NBT","NBR", "SBL", "SBT","SBR","EBL","EBT","EBR", "WBL","WBT","WBR"};
		string lane_row_name_str[LaneRowSize] = {"UpNodeID","DestNodeID","Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};

		int i,j, m;
		int movement_size = m_MovementVector.size();

		fopen_s(&st,m_Synchro_ProjectDirectory+"Lanes.csv","w");
		if(st!=NULL)
		{		
			fprintf(st, "Lane Group Data \n");
			fprintf(st, "RECORDNAME,INTID,");
			for(j=0; j<LaneColumnSize;j++)
				fprintf(st, "%s,", lane_Column_name_str[j].c_str());
			fprintf(st,"\n");

			for (m=0; m<movement_size;m++)
			{
				for(i=2; i<LaneRowSize; i++)
				{
					fprintf(st, "%s,", lane_row_name_str[i].c_str());
					fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);

					for(j=0; j<LaneColumnSize;j++)
					{
						float text = m_MovementVector[m].DataMatrix[i][j].m_text;
						if (text >= 0)
							fprintf(st, "%f,",text);
						else
							fprintf(st, ",");
					}
					fprintf(st,"\n");
				}
			}
			fprintf(st,"\n");

			fclose(st);
		}

		// write phase file
		const int PhaseColumnSize = 8;
		const int PhaseRowSize = 23;
		string phase_Column_name_str[PhaseColumnSize] = { "D1","D2","D3","D4","D5","D6","D7","D8"};
		string phase_row_name_str[PhaseRowSize] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};

		int p, phase_size = m_PhaseVector.size();

		fopen_s(&st,m_Synchro_ProjectDirectory+"Phasing.csv","w");
		if(st!=NULL)
		{
			fprintf(st, "Phasing Data \n");
			fprintf(st, "RECORDNAME,INTID,");
			for(j=0; j<PhaseColumnSize;j++)
				fprintf(st, "%s,", phase_Column_name_str[j].c_str());
			fprintf(st,"\n");

			for (p=0; p<phase_size;p++)
			{
				for(i=0; i<PhaseRowSize; i++)
				{
					fprintf(st, "%s,", phase_row_name_str[i].c_str());
					fprintf(st, "%i,", m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_NodeNumber);

					for(j=0; j<PhaseColumnSize;j++)
					{
						fprintf(st, "%f,",m_PhaseVector[p].DataMatrix[i][j].m_text);
					}
					fprintf(st,"\n");
				}
			}
			fprintf(st,"\n");

			fclose(st);
		}

		// write layout file
		DTA_Approach incoming_approach, out_approach;
		GDPoint p1, p2;
		int current_node_id, up_node_id, down_node_id;
		long LinkID;
		int i_n = 0;

		fopen_s(&st,m_Synchro_ProjectDirectory+"Layout.csv","w");
		if(st!=NULL)
		{
			fprintf(st, "Layout Data \n");
			fprintf(st, "INTID,INTNAME,TYPE,X,Y,NID,SID,EID,WID,NNAME,SNAME,ENAME,WNAME");
			fprintf(st,"\n");


			for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i_n++)
			{  // for current node	
				fprintf(st, "%i,%s,%i,%f,%f,", m_NodeIDMap[i_n]->m_NodeNumber, m_NodeIDMap[i_n]->m_Name.c_str(), m_NodeIDMap[i_n]->m_ControlType, m_NodeIDMap[i_n]->pt.x, m_NodeIDMap[i_n]->pt.y);
				// find connecting nodes and links at different directions
				DTA_NodeBasedLinkSets Node_Link;
				current_node_id = i_n;
				Node_Link.initial(current_node_id);
				for(int inbound_i= 0; inbound_i< m_Network.m_InboundSizeAry[i_n]; inbound_i++)
				{		
					LinkID = m_Network.m_InboundLinkAry[i_n][inbound_i];
					up_node_id = m_Network.m_FromIDAry[LinkID];
					p1  = m_NodeIDMap[up_node_id]->pt;
					p2  = m_NodeIDMap[current_node_id]->pt;
					incoming_approach = g_Angle_to_Approach_New(Find_P2P_Angle(p1,p2));
					switch (incoming_approach)
					{ 
					case DTA_North:
						Node_Link.link_flag[1] = true;
						Node_Link.Up_ID[1] = up_node_id;
						Node_Link.Name[1] = m_LinkNoMap[LinkID]->m_Name;
						break;
					case DTA_South:
						Node_Link.link_flag[0] = true;
						Node_Link.Up_ID[0] = up_node_id;
						Node_Link.Name[0] = m_LinkNoMap[LinkID]->m_Name;
						break;
					case DTA_East:
						Node_Link.link_flag[3] = true;
						Node_Link.Up_ID[3] = up_node_id;
						Node_Link.Name[3] = m_LinkNoMap[LinkID]->m_Name;
						break;
					case DTA_West:
						Node_Link.link_flag[2] = true;
						Node_Link.Up_ID[2] = up_node_id;
						Node_Link.Name[2] = m_LinkNoMap[LinkID]->m_Name;
						break;
					}
				}
				for(int outbound_i= 0; outbound_i< m_Network.m_OutboundSizeAry[i_n]; outbound_i++)
				{		
					LinkID = m_Network.m_OutboundLinkAry[i_n][outbound_i];
					down_node_id = m_Network.m_ToIDAry[LinkID];
					p1  = m_NodeIDMap[current_node_id]->pt;
					p2  = m_NodeIDMap[down_node_id]->pt;
					out_approach = g_Angle_to_Approach_New(Find_P2P_Angle(p1,p2));
					switch (out_approach)
					{ 
					case DTA_North:
						Node_Link.link_flag[0] = true;
						Node_Link.Up_ID[0] = down_node_id;
						Node_Link.Name[0] = m_LinkNoMap[LinkID]->m_Name;
						break;
					case DTA_South:
						Node_Link.link_flag[1] = true;
						Node_Link.Up_ID[1] = down_node_id;
						Node_Link.Name[1] = m_LinkNoMap[LinkID]->m_Name;
						break;
					case DTA_East:
						Node_Link.link_flag[2] = true;
						Node_Link.Up_ID[2] = down_node_id;
						Node_Link.Name[2] = m_LinkNoMap[LinkID]->m_Name;
						break;
					case DTA_West:
						Node_Link.link_flag[3] = true;
						Node_Link.Up_ID[3] = down_node_id;
						Node_Link.Name[3] = m_LinkNoMap[LinkID]->m_Name;
						break;
					}
				}
				// write into file
				for(i=0; i<4; i++)
				{
					if (Node_Link.link_flag[i])
						fprintf(st, "%i,", m_NodeIDMap[Node_Link.Up_ID[i]]->m_NodeNumber);
					else
						fprintf(st, ",");
				}
				for(i=0; i<4; i++)
				{
					if (Node_Link.link_flag[i] && !Node_Link.Name[i].empty() && Node_Link.Name[i] != "(null)")
						fprintf(st, "%i,", Node_Link.Name[i].c_str());
					else
						fprintf(st, ",");
				}
				fprintf(st,"\n");	
			}
			fprintf(st,"\n");

			fclose(st);
		}
		
		// write timing file
		fopen_s(&st,m_Synchro_ProjectDirectory+"Timing.csv","w");
		if(st!=NULL)
		{
			fprintf(st, "Timing Plans \n");
			fprintf(st, "PLANID,INTID,S1,S2,S3,S4,S5,S6,S7,S8,CL,OFF,LD,REF,CLR \n");
			DTA_TimePlan plan;
			plan.initial();

			for (p=0; p<phase_size;p++)
			{
				fprintf(st, "%i,%i,", plan.PlanID, m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_NodeNumber);
				DTA_Timing timing;
				timing.initial(m_PhaseVector[p], plan.DataAry[TIMING_Cycle_Length]);
				for(i=0; i<8; i++)
				{
					fprintf(st, "%i,", timing.DataAry[i]);
				}
				fprintf(st, "%i,%i,%i,%s,\n", timing.Cycle, timing.OFF, timing.LD, timing.REF.c_str());
			}
			fprintf(st,"\n");

			fclose(st);
		}

		// write volume file
		fopen_s(&st,m_Synchro_ProjectDirectory+"Volume.csv","w");
		if(st!=NULL)
		{
			fprintf(st, "Turning Movement Count \n");
			fprintf(st, "60 Minute Counts \n");
			fprintf(st, "DATE,TIME,INTID,NBL,NBT,NBR,SBL,SBT,SBR,EBL,EBT,EBR,WBL,WBT,WBR \n");
			for (m=0; m<movement_size;m++)
			{				
				fprintf(st, "12/12/2011,1700,%i,0,0,0,0,0,0,0,0,0,0,0,0", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);
				fprintf(st,"\n");
			}

			fprintf(st,"\n");

			fclose(st);

		}
}

void CTLiteDoc::ExportSingleSynchroFile(CString SynchroProjectFile)
{
	FILE* st = NULL;

	const int LaneColumnSize = 12;
	const int LaneRowSize = 30;
	string lane_Column_name_str[LaneColumnSize] = { "NBL","NBT","NBR", "SBL", "SBT","SBR","EBL","EBT","EBR", "WBL","WBT","WBR"};
	string lane_row_name_str[LaneRowSize] = {"UpNodeID","DestNodeID","Lanes","Shared","Width","Storage","StLanes","Grade","Speed","FirstDetect","LastDetect","Phase1","PermPhase1","DetectPhase1","IdealFlow","LostTime","SatFlow","SatFlowPerm","SatFlowRTOR","HeadwayFact","Volume","Peds","Bicycles","PHF","Growth","HeavyVehicles","BusStops","Midblock","Distance","TravelTime"};
	
	unsigned int i,j, m;
	int movement_size = m_MovementVector.size();

	const int PhaseColumnSize = 8;
	const int PhaseRowSize = 23;
	string phase_Column_name_str[PhaseColumnSize] = { "D1","D2","D3","D4","D5","D6","D7","D8"};
	string phase_row_name_str[PhaseRowSize] = {"BRP","MinGreen","MaxGreen","VehExt","TimeBeforeReduce","TimeToReduce","MinGap","Yellow","AllRed","Recall","Walk","DontWalk","PedCalls","MinSplit","DualEntry","InhibitMax","Start","End","Yield","Yield170","LocalStart","LocalYield","LocalYield170"};

	int p, phase_size = m_PhaseVector.size();

	fopen_s(&st,SynchroProjectFile,"w");
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
		for (i=0; i < m_NodeSet.size(); i++)
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
			for(int inbound_i= 0; inbound_i< m_Network.m_InboundSizeAry[i_n]; inbound_i++)
			{		
				LinkID = m_Network.m_InboundLinkAry[i_n][inbound_i];
				up_node_id = m_Network.m_FromIDAry[LinkID];
				p1  = m_NodeIDMap[up_node_id]->pt;
				p2  = m_NodeIDMap[current_node_id]->pt;
				incoming_approach = g_Angle_to_Approach_New(Find_P2P_Angle(p1,p2));
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
		for(j=0; j<LaneColumnSize;j++)
			fprintf(st, "%s,", lane_Column_name_str[j].c_str());
		fprintf(st,"\n");

		for (m=0; m<movement_size;m++)
		{
			// write UpNodeID and DestNodeID using original m_NodeNumber
			for(i=0; i<2; i++)
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);  // current node id

				for(j=0; j<LaneColumnSize;j++)
				{
					int NodeID = (int)(m_MovementVector[m].DataMatrix[i][j].m_text);
					TRACE("Node Label: %d\n",NodeID);

					if(NodeID>=0)  //this movement has been initialized
					{
						if(m_NodeIDMap.find(NodeID) == m_NodeIDMap.end())
						{
						AfxMessageBox("Error in node id!");
						return;
						}
						fprintf(st, "%i,",m_NodeIDMap[NodeID]->m_NodeNumber);  
					}else  // this movement has not been initialized, so the default value is -1
					{
						fprintf(st, ",");  
					
					}
				}
				fprintf(st,"\n");
			}

			for(i=2; i<LaneRowSize; i++)
			{
				fprintf(st, "%s,", lane_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_MovementVector[m].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<LaneColumnSize;j++)
				{
					float text = m_MovementVector[m].DataMatrix[i][j].m_text;
					if (text >= 0)
						fprintf(st, "%f,",text);
					else
						fprintf(st, ",");
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
		for(j=0; j<PhaseColumnSize;j++)
			fprintf(st, "%s,", phase_Column_name_str[j].c_str());
		fprintf(st,"\n");

		for (p=0; p<phase_size;p++)
		{
			for(i=0; i<PhaseRowSize; i++)
			{
				fprintf(st, "%s,", phase_row_name_str[i].c_str());
				fprintf(st, "%i,", m_NodeIDMap[m_PhaseVector[p].CurrentNodeID]->m_NodeNumber);

				for(j=0; j<PhaseColumnSize;j++)
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
	
	OpenCSVFileInExcel(SynchroProjectFile);
}
void CTLiteDoc::OnExportAms()
{
	const int NodeColumnSize = 92;
	CString node_Column_name_str[NodeColumnSize] = 
{
	"	Node	",
"	Geometry	",
"	NodeType	",
"	ControlType	",
"	Zone	",
"	Movement_USN1	",
"	Movement_USN2	",
"	Movement_USN3	",
"	Movement_USN4	",
"	Movement_USN5	",
"	Movement_USN6	",
"	TurnVol_USN1	",
"	TurnVol_USN2	",
"	TurnVol_USN3	",
"	TurnVol_USN4	",
"	TurnVol_USN5	",
"	TurnVol_USN6	",
"	RTOR_USN1	",
"	RTOR_USN2	",
"	RTOR_USN3	",
"	RTOR_USN4	",
"	RTOR_USN5	",
"	RTOR_USN6	",
"	Uturn_USN1	",
"	Uturn_USN2	",
"	Uturn_USN3	",
"	Uturn_USN4	",
"	Uturn_USN5	",
"	Uturn_USN6	",
"	Turn_multiplier_USN1	",
"	Turn_multiplier_USN2	",
"	Turn_multiplier_USN3	",
"	Turn_multiplier_USN4	",
"	Turn_multiplier_USN5	",
"	Turn_multiplier_USN6	",
"	Lane_Align_USN1	",
"	Lane_Align_USN2	",
"	Lane_Align_USN3	",
"	Lane_Align_USN4	",
"	Lane_Align_USN5	",
"	Lane_Align_USN6	",
"	Conditional_LT_USN1	",
"	Conditional_TH_USN1	",
"	Conditional_RT_USN1	",
"	Conditional_Turn1_USN1	",
"	Conditional_Turn2_USN1	",
"	Conditional_LT_USN2	",
"	Conditional_TH_USN2	",
"	Conditional_RT_USN2	",
"	Conditional_Turn1_USN2	",
"	Conditional_Turn2_USN2	",
"	Conditional_LT_USN3	",
"	Conditional_TH_USN3	",
"	Conditional_RT_USN3	",
"	Conditional_Turn1_USN3	",
"	Conditional_Turn2_USN3	",
"	Conditional_LT_USN4	",
"	Conditional_TH_USN4	",
"	Conditional_RT_USN4	",
"	Conditional_Turn1_USN4	",
"	Conditional_Turn2_USN4	",
"	Conditional_LT_USN5	",
"	Conditional_TH_USN5	",
"	Conditional_RT_USN5	",
"	Conditional_Turn1_USN5	",
"	Conditional_Turn2_USN5	",
"	Conditional_LT_USN6	",
"	Conditional_TH_USN6	",
"	Conditional_RT_USN6	",
"	Conditional_Turn1_USN6	",
"	Conditional_Turn2_USN6	",
"	Stopline_USN1	",
"	Stopline_USN2	",
"	Stopline_USN3	",
"	Stopline_USN4	",
"	Stopline_USN5	",
"	Stopline_USN6	",
"	Sight_Dist_USN1	",
"	Sight_Dist_USN2	",
"	Sight_Dist_USN3	",
"	Sight_Dist_USN4	",
"	Sight_Dist_USN5	",
"	Sight_Dist_USN6	",
"	Pedestrian_USN1	",
"	Pedestrian_USN2	",
"	Pedestrian_USN3	",
"	Pedestrian_USN4	",
"	Pedestrian_USN5	",
"	Pedestrian_USN6	",
"	Offramp_React	",
"	Offramp_HOV_React	",
"	Lane_Distribution	"};

	const int LinkColumnSize = 70;
	CString link_Column_name_str[LinkColumnSize] = 
{

	"	Link	",
"	Geometry	",
"	USN	",
"	DSN	",
"	Name	",
"	Type	",
"	Length	",
"	Lanes	",
"	PostSpeed	",
"	FreeFlowSpeed	",
"	Grade	",
"	Graphic	",
"	Curvature	",
"	Feature_points	",
"	Underpass	",
"	Startup_Delay	",
"	Superelevation	",
"	Pavement	",
"	Radius	",
"	Car_Follow_Sens_Multiplier	",
"	Auxiliary	",
"	Aux_Length	",
"	LC_Speed_Threshold	",
"	LC_Reaction	",
"	Left_Barrier	",
"	AddDrop	",
"	AddDrop_Type	",
"	AddDrop_Dist_USN	",
"	AddDrop_React	",
"	HV_Restrict	",
"	HV_Restrict_Lane	",
"	HV_Only	",
"	HV_React	",
"	HOV_#Lanes	",
"	HOV_Location	",
"	HOV_Type	",
"	HOV_USN	",
"	HOV_Length	",
"	HOV_React	",
"	HOV_User_TP	",
"	HOV_Pct_TP	",
"	Queue_Code_TP	",
"	Discharge_Hwy	",
"	LT_#Lane	",
"	LT_Length	",
"	RT_#Lane	",
"	RT_Length	",
"	Align_Lane	",
"	Align_DSN	",
"	Lane_Width	",
"	Lane_Conf	",
"	Bus_Station	",
"	MidBlock_ID	",
"	MidBlock_Flow	",
"	Parking	",
"	Park_Duration	",
"	Park_Freq	",
"	Park_Lt_DSN	",
"	Park_LT_Length	",
"	Park_RT_DSN	",
"	Park_RT_Length	",
"	Flow_Model	",
"	Saturation	",
"	Service_Flow	",
"	Speed_Adj	",
"	Link_Generation	",
"	Direction	",
"	Functional	",
"	Area_Type	",
"	Modes	"};


	CString AMS_Node_File, AMS_Link_File;
	CFileDialog node_dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"AMS Node Data File (*.csv)|*.csv||", NULL);
	if(node_dlg.DoModal() == IDOK)
	{
		AMS_Node_File = node_dlg.GetPathName();
	}else
	{
	return;
	}

	CFileDialog link_dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"AMS Link Data File (*.csv)|*.csv||", NULL);
	if(link_dlg.DoModal() == IDOK)
	{
		AMS_Link_File = link_dlg.GetPathName();

	}else
	{
	return;
	}



	CWaitCursor wait;
	FILE* st = NULL;
	fopen_s(&st,AMS_Node_File,"w");
	if(st!=NULL)
	{
	//CTestDataSource provide set of data for testing XY plot function
	int i;
	for(i = 0; i < NodeColumnSize; i++)
	{
	fprintf(st,"%s,",node_Column_name_str[i]);
	}
	fprintf(st,"\n");

	// Node sheet
		std::list<DTANode*>::iterator iNode;
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			fprintf(st, "%d,\"<Point><coordinates>%f,%f</coordinates></Point>\",,%d\n", (*iNode)->m_NodeNumber , (*iNode)->pt .x, (*iNode)->pt .y,(*iNode)->m_ControlType );
		}

	fclose(st);


	}
	



	fopen_s(&st,AMS_Link_File,"w");
	if(st!=NULL)
	{
	//CTestDataSource provide set of data for testing XY plot function
	int i;
	for(i = 0; i < LinkColumnSize; i++)
	{
	fprintf(st,"%s,",link_Column_name_str[i]);
	}
	fprintf(st,"\n");

		std::list<DTALink*>::iterator iLink;
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_AVISensorFlag == false)
			{
//		fprintf(st,"name,link_id,from_node_id,to_node_id,direction,length_in_mile,number_of_lanes,speed_limit_in_mph,lane_capacity_in_vhc_per_hour,link_type,jam_density_in_vhc_pmpl,wave_speed_in_mph,mode_code,grade,geometry\n");
			fprintf(st,"%d,",(*iLink)->m_LinkID);
				
			fprintf(st,"\"<LineString><coordinates>");

			for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
			{
				fprintf(st,"%f,%f,0.0",(*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y);
				if(si!=(*iLink)->m_ShapePoints.size()-1)
					fprintf(st," ");
			}
			fprintf(st,"</coordinates></LineString>\",");

			fprintf(st,"%d,%d,%s,%d,%5.1f,%d,%3.1f,%3.1f,%3.1f",
				(*iLink)->m_FromNodeNumber, 
				(*iLink)->m_ToNodeNumber ,
				(*iLink)->m_Name.c_str (),
				(*iLink)->m_link_type ,
				(*iLink)->m_Length*5280 ,  // mile -> feet
				(*iLink)->m_NumLanes ,
				(*iLink)->m_SpeedLimit,
				(*iLink)->m_SpeedLimit,
				(*iLink)->m_Grade);

			fprintf(st,"\n");
			}

		}
	fclose(st);

	}
	OpenCSVFileInExcel(AMS_Node_File);
	OpenCSVFileInExcel(AMS_Link_File);
}



void CTLiteDoc::RunExcelAutomation() 
{
	//CXLEzAutomation class constructor starts Excel and creates empty worksheet  
	CXLEzAutomation XL;
			//Close Excel if failed to open file 
			if(!XL.OpenExcelFile("C:\\NEXTA_OpenSource\\DTALite\\test_QEM.xlsx"))
			{
				XL.ReleaseExcel();
//				MessageBox("Failed to Open Excel File", "Error", MB_OK);
				return;
			}

			XL.SetActiveWorksheet(0);
			//To access data use this:
			XL.SetCellValue(2,4,"1");
			CString szCellValue5 = XL.GetCellValue(2, 52);
			//Itterate through all cells given by (Column, Row) to
			//access all data

			//CTestDataSource provide set of data for testing XY plot function
	CTestDataSource DataSource;

/*	//All data will be stored by szData
	CString szData;
	//Itterate to fill data buffer with 100 data points
	for(int i = 0; i < 100; i++)
		szData = szData + DataSource.GetNextXYPoint();
	
	//Use clipboard export function to move all data to Excel worksheet
	XL.ExportCString(szData);
	
	//Y values for this plot are in Column = 2
	XL.CreateXYChart(2);
*/	
//	XL.EnableAlert(false);
	XL.ReleaseExcel();


}

void AddGISField(std::vector<OGRFieldDefn> &OGRFieldVector, CString field_name,  OGRFieldType type)
{
    OGRFieldDefn oField(field_name, type );
	OGRFieldVector.push_back(oField);
}

bool CreateGISVector(std::vector<OGRFieldDefn> OGRFieldVector, OGRLayer *poLayer)
{
	for(int i = 0; i < OGRFieldVector.size(); i++)
	{
		OGRFieldDefn oField = OGRFieldVector[i];
		if( poLayer->CreateField( &oField ) != OGRERR_NONE )
		{
			CString str;
			str.Format("Creating field %s failed", oField.GetNameRef());
			AfxMessageBox(str);
			return false;		
		}
	}

	return true;
}

void CTLiteDoc::ExportNodeLayerToGISFiles(CString file_name, CString GISTypeString)
{
	CString message_str;

	// 
    OGRSFDriver *poDriver;

    OGRRegisterAll();

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(GISTypeString );
    if( poDriver == NULL )
    {
        m_GISMessage.Format ( "%s driver not available.", GISTypeString );
		AfxMessageBox(m_GISMessage);
        return;
    }

    OGRDataSource *poDS;
	
    poDS = poDriver->CreateDataSource(file_name, NULL );
    if( poDS == NULL )
    {
        m_GISMessage.Format ( "Creation of GIS output file %s failed.  Please make sure the file is not opened by another program.", file_name );
		AfxMessageBox(m_GISMessage);
		return;
    }

	///// export to node layer

	// node layer 
	{
    OGRLayer *poLayer;
    poLayer = poDS->CreateLayer( "node", NULL, wkbPoint, NULL );
    if( poLayer == NULL )
    {
		m_GISMessage = "Node layer creation failed";
		AfxMessageBox(m_GISMessage);
		return;
    }


	OGRFieldDefn oField1 ("Name", OFTString); 
	OGRFieldDefn oField2 ("NodeId", OFTInteger); 
	OGRFieldDefn oField3 ("NodeType", OFTInteger); 
	OGRFieldDefn oField4 ("x", OFTReal); 
	OGRFieldDefn oField5 ("y", OFTReal); 

	CString str;  
	if( poLayer->CreateField( &oField1 ) != OGRERR_NONE ) { str.Format("Creating field %s failed", oField1.GetNameRef()); AfxMessageBox(str); return; }
	if( poLayer->CreateField( &oField2 ) != OGRERR_NONE ) {	str.Format("Creating field %s failed", oField2.GetNameRef()); AfxMessageBox(str); return; }
	if( poLayer->CreateField( &oField3 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField3.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField4 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField4.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField5 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField5.GetNameRef()); AfxMessageBox(str); return ;	}

	std::list<DTANode*>::iterator iNode;

		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if((*iNode)->m_LayerNo ==0) 
			{
				OGRFeature *poFeature;

				poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
				poFeature->SetField("Name",(*iNode)->m_Name.c_str () );
				poFeature->SetField("NodeId", (*iNode)->m_NodeNumber );
				poFeature->SetField("NodeType", (*iNode)->m_ControlType );
				poFeature->SetField("x", (*iNode)->pt .x );
				poFeature->SetField("y", (*iNode)->pt .y );

				OGRPoint pt;
				pt.setX( (*iNode)->pt .x );
				pt.setY( (*iNode)->pt .y );
				poFeature->SetGeometry( &pt ); 

				if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
				{
					AfxMessageBox("Failed to create feature in shapefile.\n");
					return;

				}

	        OGRFeature::DestroyFeature( poFeature );
		 }
    }
		
	} // end of node layer

   OGRDataSource::DestroyDataSource( poDS );

}

void CTLiteDoc::ExportLinkLayerToGISFiles(CString file_name, CString GISTypeString)
{
	CString message_str;

	// 
    OGRSFDriver *poDriver;

    OGRRegisterAll();

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(GISTypeString );
    if( poDriver == NULL )
    {
        m_GISMessage.Format ( "%s driver not available.", GISTypeString );
		AfxMessageBox(m_GISMessage);
        return;
    }

    OGRDataSource *poDS;
	
    poDS = poDriver->CreateDataSource(file_name, NULL );
    if( poDS == NULL )
    {
        m_GISMessage.Format ( "Creation of GIS output file %s failed. Please make sure the file is not opened by another program.", file_name );
		AfxMessageBox(m_GISMessage);
		return;
    }

	///// export to node layer

	// link layer 
	{
       OGRLayer *poLayer;
	 poLayer = poDS->CreateLayer( "link", NULL, wkbLineString, NULL );
    if( poLayer == NULL )
    {
		AfxMessageBox("link Layer creation failed");
		return;
    }

	OGRFieldDefn oField1 ("LinkID", OFTInteger); 
	OGRFieldDefn oField2 ("Name", OFTString); 
	OGRFieldDefn oField3 ("A_Node", OFTInteger); 
	OGRFieldDefn oField4 ("B_Node", OFTInteger); 
	OGRFieldDefn oField5 ("IsOneWay", OFTInteger); 
	OGRFieldDefn oField6 ("nLanes", OFTInteger); 
	OGRFieldDefn oField7 ("SpeedLimit", OFTInteger); 
	OGRFieldDefn oField8 ("LaneCap", OFTInteger); 
	OGRFieldDefn oField9 ("FunctClass", OFTInteger); 
	OGRFieldDefn oField10 ("Grade", OFTReal); 
	OGRFieldDefn oField11 ("iSpeed", OFTReal); 
	OGRFieldDefn oField12 ("iReliable", OFTReal); 

	CString str;  
	if( poLayer->CreateField( &oField1 ) != OGRERR_NONE ) { str.Format("Creating field %s failed", oField1.GetNameRef()); AfxMessageBox(str); return; }
	if( poLayer->CreateField( &oField2 ) != OGRERR_NONE ) {	str.Format("Creating field %s failed", oField2.GetNameRef()); AfxMessageBox(str); return; }
	if( poLayer->CreateField( &oField3 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField3.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField4 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField4.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField5 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField5.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField6 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField6.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField7 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField7.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField8 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField8.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField9 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField9.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField10 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField10.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField11 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField11.GetNameRef()); AfxMessageBox(str); return ;	}
	if( poLayer->CreateField( &oField12 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField12.GetNameRef()); AfxMessageBox(str); return ;	}

	std::list<DTALink*>::iterator iLink;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_AVISensorFlag == false && (*iLink)->m_LayerNo ==0)
			{

				OGRFeature *poFeature;

				poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
				poFeature->SetField("LinkID",(*iLink)->m_LinkID );
				poFeature->SetField("Name", (*iLink)->m_Name.c_str () );
				poFeature->SetField("A_Node", (*iLink)->m_FromNodeNumber );
				poFeature->SetField("B_Node", (*iLink)->m_ToNodeNumber );
				poFeature->SetField("IsOneWay", (*iLink)->m_Direction );
				poFeature->SetField("NumberOfLanes", (*iLink)->m_NumLanes );
				poFeature->SetField("SpeedLimit", (*iLink)->m_SpeedLimit );
				poFeature->SetField("LaneCapacity", (*iLink)->m_LaneCapacity );
				poFeature->SetField("FunctionalClass", (*iLink)->m_link_type );
				poFeature->SetField("Grade", (*iLink)->m_Grade );

			float value;
			float mobility_index  = GetLinkMOE((*iLink), MOE_speed, 1, 360, value);
			float reliability_index  = GetLinkMOE((*iLink), MOE_reliability, 1, 360, value);
				poFeature->SetField("iSpeed", mobility_index);
				poFeature->SetField("iReliable", reliability_index );

				OGRLineString line;
				for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
				{
				line.addPoint ((*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y);
				}

				poFeature->SetGeometry( &line ); 


				if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
				{
					AfxMessageBox("Failed to create line feature in shapefile.\n");
					return;

				}  

	        OGRFeature::DestroyFeature( poFeature );
			}
		}

		} // end of link layer
    OGRDataSource::DestroyDataSource( poDS );

}

/*

bool CTLiteDoc::SignalPlanGeneration(int MaxGreen, int MinGreen, int Amber)
{

   // for each node

	// all links have not marked

	// generate a new phase


		// for each unassigned incoming link
		// find out paired link(s)  //opposite direction

		//for each movement

	// add movement into a phase
		
	// assign phase timing data data


}

*/