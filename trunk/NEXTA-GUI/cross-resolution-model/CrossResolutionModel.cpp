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
#include "MainFrm.h"

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

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	TCHAR IniFilePath[_MAX_PATH];
	sprintf_s(IniFilePath,"%s\\LaneSettings.ini", pMainFrame->m_CurrentDirectory);

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

	sprintf_s(IniFilePath,"%s\\PhaseSettings.ini", pMainFrame->m_CurrentDirectory);
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

				float pt_x = NPtoSP_X(m_NodeIDMap[i]->pt,1/m_UnitFeet);
				float pt_y = NPtoSP_Y(m_NodeIDMap[i]->pt,1/m_UnitFeet);

			fprintf(st, "%i,%s,%i,%f,%f,", m_NodeIDMap[i_n]->m_NodeNumber, m_NodeIDMap[i_n]->m_Name.c_str(), m_NodeIDMap[i_n]->m_ControlType,
				pt_x, pt_y);
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
	// reset origin for converted network in synchro

	std::list<DTANode*>::iterator iNode;
	m_Origin.x  = 1000000;
	m_Origin.y  = 1000000;

	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
	m_Origin.x = min(m_Origin.x,(*iNode)->pt .x);
	m_Origin.y= min(m_Origin.y,(*iNode)->pt .y);
	}



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
				float pt_x = NPtoSP_X(m_NodeIDMap[i]->pt,1/m_UnitFeet);
				float pt_y = NPtoSP_Y(m_NodeIDMap[i]->pt,1/m_UnitFeet);

				fprintf(st, "%i,%i,%f,%f,%f,%s,%i,\n", m_NodeIDMap[i]->m_NodeNumber, 
				m_NodeIDMap[i]->m_ControlType, 
				pt_x, pt_y,
				0.0, m_NodeIDMap[i]->m_Name.c_str(), 0);
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


void CTLiteDoc::OGDF_WriteGraph(CString FileName)
{
	std::ofstream GMLFile;
	int nextId = 0;

	GMLFile.open (FileName, ios::out);
	if (GMLFile.is_open())
	{
		GMLFile.width(15);
		GMLFile.precision(7) ;
		GMLFile.setf(ios::fixed);
	}
	else
	{
		return;
	}
	GMLFile << "Creator \"NeXTA::writeGML\"\n";
	GMLFile << "directed 1\n";

	GMLFile << "graph [\n";

		std::list<DTANode*>::iterator iNode;
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
		GMLFile << "node [\n";
		GMLFile << "id " << (*iNode)->m_NodeNumber << "\n";
		GMLFile << "label  " << "\"" << (*iNode)->m_Name.c_str () << "\"\n";
		GMLFile << "x  " << (*iNode)->pt.x << "\n";
		GMLFile << "y  " << (*iNode)->pt.y << "\n";
		GMLFile << "]\n"; // node
		}

		std::list<DTALink*>::iterator iLink;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
		GMLFile << "edge [\n";
		GMLFile << "source " << (*iLink)->m_FromNodeNumber << "\n";
		GMLFile << "target " << (*iLink)->m_ToNodeNumber << "\n";
		GMLFile << "weight " << (*iLink)->m_Length << "\n";
		GMLFile << "]\n"; // edge
		
		}

	GMLFile << "]\n"; // graph
	GMLFile.close();

}