// Transit.h :
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
#pragma once
#include "stdafx.h"
#include "TLite.h"
#include "Network.h"
#include <vector>
#include <algorithm>


class PT_Stop
{
public:

int stop_id;
int stop_code;
CString stop_name;
CString stop_desc;
float stop_lat;
float stop_lon;
int zone_id;
int location_type;
CString direction;
CString position;


};

class PT_StopTime
{
public:
int trip_id;
int arrival_time;  // unit: min;
int departure_time;
int stop_id;
int stop_sequence;
int stop_headsign;
int pickup_type;
int drop_off_type;
float shape_dist_traveled;
int timepoint;

};

class PT_Trip
{
public:

	int route_id;
	CString service_id;
	int trip_id;
	int direction_id;
	int block_id;
	int shape_id;
	int trip_type;

	std::vector<PT_StopTime> m_PT_StopTimeVector;
	

};




class PT_Route 
{
public: 
int route_id;
CString route_short_name;
CString route_long_name;
CString route_type;
CString route_url;

std::vector<PT_Trip> m_PT_TripVector;


};
class PT_Network
{
public:

CString m_ProjectDirectory;


std::map<int, PT_Route> m_PT_RouteMap;
std::map<int, PT_Trip> m_PT_TripMap;
std::map<int, PT_Stop> m_PT_StopMap;

bool ReadGTFFiles();  // Google Transit files

void BuildTransitNetwork();

// node time label: node cost label

bool OptimalTDLabelCorrecting_DQ(int origin, int departure_time, int destination);
bool FindOptimalSolution(int origin, int departure_time, int destination,int PathNodeList[MAX_NODE_SIZE_IN_A_PATH]);


};

