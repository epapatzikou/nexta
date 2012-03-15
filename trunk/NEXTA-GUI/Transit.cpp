// Transit.cpp : Implementation file
//
//  Portions Copyright 2012 Shuguang Li (xalxlsg@gmail.com)

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
#include "Transit.h"
#include "Geometry.h"
#include "CSVParser.h"
#include <vector>
#include <algorithm>

bool PT_Network::ReadGTFFiles()  // Google Transit files
{

	/*
	// step 1: read files
	CString str = m_ProjectDirectory +"routes.txt";
	CT2CA pszConvertedAnsiString (str);
	// construct a std::string using the LPCSTR input
	std::string  strStd (pszConvertedAnsiString);
	CCSVParser parser;

	if (parser.OpenCSVFile(strStd))
	{
		int count =0;
		while(parser.ReadRecord())
		{
			PT_Route route;

			if(parser.GetValueByFieldName("route_id",route.route_id ) == false)
				break;

			if(parser.GetValueByFieldName("route_long_name",route.route_long_name  ) == false)
				break;

			// make sure there is not duplicated key
			if(m_PT_RouteMap.find(route.route_id) != m_PT_RouteMap.end())
			{
				m_PT_RouteMap[route.route_id] = route;
			}else
			{
			AfxMessageBox("Duplicated Route ID!");
			}
			count++;

		}
		parser.CloseCSVFile ();

	}

	CString str2 = m_ProjectDirectory +"stops.txt";
	CT2CA pszConvertedAnsiString2 (str2);
	// construct a std::string using the LPCSTR input
	std::string  strStd2 (pszConvertedAnsiString2);
	if (parser.OpenCSVFile(strStd2))
	{
		int count =0;
		while(parser.ReadRecord())
		{
			PT_Stop stop;

			if(parser.GetValueByFieldName("stop_id",stop.stop_id ) == false)
				break;

			if(parser.GetValueByFieldName("stop_lat",stop.stop_lat  ) == false)
				break;

			
			if(m_PT_StopMap.find(stop.stop_id) != m_PT_StopMap.end())
			{
				m_PT_StopMap[stop.stop_id] = stop;
			}else
			{
			AfxMessageBox("Duplicated Stop ID!");
			}

			count++;

		}
		parser.CloseCSVFile ();

	}

////////////////
	CString str3 = m_ProjectDirectory +"trips.txt";
	CT2CA pszConvertedAnsiString3 (str3);
	// construct a std::string using the LPCSTR input
	std::string  strStd3 (pszConvertedAnsiString3);
	if (parser.OpenCSVFile(strStd3))
	{
		int count =0;
		while(parser.ReadRecord())
		{
			PT_Trip trip;

			if(parser.GetValueByFieldName("trip_id",trip.trip_id) == false)
				break;
			
			if(m_PT_TripMap.find(trip.trip_id) != m_PT_TripMap.end())
			{
				m_PT_TripMap[trip.trip_id] = trip;
			}else
			{
			AfxMessageBox("Duplicated Trip ID!");
			}

			count++;

		}
		parser.CloseCSVFile ();
	}
	*/

	// read stop_times.txt


	// step 2: assemble data
	// add stop times into trips
	// add trips into routes
return true;
}