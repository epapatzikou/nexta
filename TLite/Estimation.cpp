// Timetable.cpp : implementation of timetabling algorithm in the CTLiteDoc class
// //  Portions Copyright 2010 Hao Lei ()


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
#include "TLite.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "TLiteView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CTLiteDoc::OnEstimationOdestimation()
{
	// TODO: Add your command handler code here
}

void  CTLiteDoc::ReadObservationLinkVolumeData(LPCTSTR lpszFileName)
{
       FILE* st = NULL;

	fopen_s(&st,lpszFileName,"r");
	int count = 0 ;

	if(st!=NULL)
	{
		std::list<DTALink*>::iterator iLink;

		while(!feof(st))
		{
			//	sensor ID, Hourly flow volume
			long SensorID =g_read_integer(st);
			if(SensorID == -1)  // reach end of file
				break;

			float HourlyFlowVolume = g_read_float(st);
			map<long,long>::iterator it;

			int LinkID = -1;
			if ( (it = m_SensorIDtoLinkIDMap.find(SensorID)) != m_SensorIDtoLinkIDMap.end()) 
			{
				int LinkID = it->second;

				DTALink* pLink = m_LinkNoMap[LinkID];

				if(pLink!=NULL)
				{
					pLink->m_ObsHourlyLinkVolume = HourlyFlowVolume;
					count ++;
				}
			}
		}

		m_ObsLinkVolumeStatus.Format ("%d hourly link volume records are loaded from file %s.",count,lpszFileName);

		fclose(st);
	}
}
