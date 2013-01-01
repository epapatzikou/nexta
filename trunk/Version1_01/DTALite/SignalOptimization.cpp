//  Portions Copyright 2010 Xuesong Zhou

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


