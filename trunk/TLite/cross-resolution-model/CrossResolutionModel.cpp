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

void CTLiteDoc::ConstructMovementVector()
{
	if(m_pNetwork!=NULL)
		delete m_pNetwork;

			m_pNetwork = new DTANetworkForSP(m_NodeSet.size(), m_LinkSet.size(), 1, 1, m_AdjLinkSize);  //  network instance for single processor in multi-thread environment
			m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, true, false);


			int i = 0;
		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i++)
		{  // for current node
			for(int inbound_i= 0; inbound_i< m_pNetwork->m_InboundSizeAry [i]; inbound_i++)
			{
				// for each incoming link
				for(int outbound_i= 0; outbound_i< m_pNetwork->m_OutboundSizeAry [i]; outbound_i++)
				{
					//for each outging link

					DTAMovement element;

					element.CurrentNodeID = i;
					
					int LinkID = m_pNetwork->m_InboundLinkAry[i][inbound_i];

					element.InboundLinkID = LinkID;
					element.UpNodeID = m_pNetwork->m_FromIDAry[LinkID];
					element.DestNodeID = m_pNetwork->m_OutboundNodeAry [i][outbound_i];

					GDPoint p1, p2, p3;
					p1  = m_NodeIDMap[element.UpNodeID]->pt;
					p2  = m_NodeIDMap[element.CurrentNodeID]->pt;
					p3  = m_NodeIDMap[element.DestNodeID]->pt;

					element.movement_approach = g_Angle_to_Approach(g_P2P_Angle(p1,p2));
					element.movement_turn = g_PPP_to_Turn(p1,p2,p3);


				}
			
			}
				
		
		}
	if(m_pNetwork!=NULL)
		delete m_pNetwork;

}
void CTLiteDoc::OnFileConstructandexportsignaldata()
{

	ConstructMovementVector();

		FILE* st = NULL;

	// initialize all movement here
	

		fopen_s(&st,m_ProjectDirectory+"Lanes.csv","w");
		if(st!=NULL)
		{

		

		
		fclose(st);
		}

}