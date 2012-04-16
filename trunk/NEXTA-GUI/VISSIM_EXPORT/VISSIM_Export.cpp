//  Portions Copyright 2010 Peng....
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

#include "VISSIM_Export.h"


void CTLiteDoc::ConstructandexportVISSIMdata()
{
	CString str;
	CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"VISSIM Data File (*.csv)|*.csv||", NULL);
	if(dlg.DoModal() == IDOK)
	{

		CWaitCursor wait;
		char fname[_MAX_PATH];
		wsprintf(fname,"%s", dlg.GetPathName());

		CString SynchroProjectFile = dlg.GetPathName();
		m_Synchro_ProjectDirectory  = SynchroProjectFile.Left(SynchroProjectFile.ReverseFind('\\') + 1);

		m_Network.Initialize (m_NodeSet.size(), m_LinkSet.size(), 1, m_AdjLinkSize);
		m_Network.BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, false);

		}


	// generate all movements
	int i = 0;
	for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i++)
	{  // for current node
		
		if ((*iNode)->m_ControlType > 1)  //(m_Network.m_InboundSizeAry[i] >= 3) // add node control types
		{
	

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

	
					}  // for each feasible movement (without U-turn)
					
				} // for each outbound link

			} // for each inbound link

		} // checking control type
	}// for each node
}


