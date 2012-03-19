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
#pragma once

#include "stdafx.h"
#include "TLite.h"
#include "Network.h"
#include "Transit.h"
#include "Geometry.h"
#include "CSVParser.h"
#include <vector>
#include <algorithm>
#include "TLiteDoc.h"
#include "TLiteView.h"

bool PT_Network::ReadGTFFiles(GDRect network_rect)  // Google Transit files
{
	//	// step 1: read  route files
	string str0 = m_ProjectDirectory +"routes.txt";
	//string str =  "h:/routes.csv";

	CT2CA pszConvertedAnsiString (str0.c_str());
	// construct a std::string using the LPCSTR input
	std::string  strStd (pszConvertedAnsiString);
	CCSVParser parser;

	if (parser.OpenCSVFile(strStd))
	{
		int count =0;
		PT_Route route;
		while(parser.ReadRecord())
		{


			if(parser.GetValueByFieldName("route_id",route.route_id ) == false)
				break;
			if(parser.GetValueByFieldName("route_long_name",route.route_long_name) == false)
				route.route_long_name="";
			if(parser.GetValueByFieldName("route_short_name",route.route_short_name) == false)
				route.route_short_name="";			
			if(parser.GetValueByFieldName("route_url",route.route_url) == false)
				route.route_url="";			
			if(parser.GetValueByFieldName("route_type",route.route_type) == false)
				route.route_type="";		

			// make sure there is not duplicated key
			// If the requested key is not found, find() returns the end iterator for
			//the container, so:

			if(m_PT_RouteMap.find(route.route_id)  == m_PT_RouteMap.end() )
			{ 
				m_PT_RouteMap[route.route_id] = route;  
			}
			else
			{
				AfxMessageBox("Duplicated Route ID!");
			}


			count++;

		}
		parser.CloseCSVFile ();

	}

	///read stop information
	string str2 = m_ProjectDirectory +"stops.txt";


	CT2CA pszConvertedAnsiString2 (str2.c_str());
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

			if(parser.GetValueByFieldName("stop_lat",stop.m_ShapePoint.y   ) == false)
				break;

			if(parser.GetValueByFieldName("stop_lon",stop.m_ShapePoint.x   ) == false)
				break;

			if(parser.GetValueByFieldName("direction",stop.direction  ) == false)
				stop.direction="";

			if(parser.GetValueByFieldName("location_type",stop.location_type) == false)
				stop.location_type=0;

			if(parser.GetValueByFieldName("position",stop.position  ) == false)
				stop.position="";

			if(parser.GetValueByFieldName("stop_code",stop.stop_code ) == false)
				stop.stop_code=0;

			if(parser.GetValueByFieldName("stop_desc",stop.stop_desc) == false)
				break;

			if(parser.GetValueByFieldName("stop_name",stop.stop_name) == false)
				stop.stop_name="";

			if(parser.GetValueByFieldName("zone_id",stop.zone_id) == false)
				stop.zone_id=0;


			if(m_PT_StopMap.find(stop.stop_id) == m_PT_StopMap.end())
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

	//read trip file
	string str3 = m_ProjectDirectory +"trips.txt";
	CT2CA pszConvertedAnsiString3 (str3.c_str());
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

			if(parser.GetValueByFieldName("route_id",trip.route_id) == false)
				break;			

			if(parser.GetValueByFieldName("service_id",trip.service_id) == false)
				break;			

			if(parser.GetValueByFieldName("block_id",trip.block_id) == false)
				trip.block_id=0;			

			if(parser.GetValueByFieldName("direction_id",trip.direction_id) == false)
				trip.direction_id=0;

			if(parser.GetValueByFieldName("shape_id",trip.shape_id) == false)
				trip.shape_id=0;

			if(parser.GetValueByFieldName("trip_type",trip.trip_type) == false)
				trip.trip_type=0;	 	

			if(m_PT_TripMap.find(trip.trip_id) == m_PT_TripMap.end())
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


	// read stop_times.txt

	string str4 = m_ProjectDirectory +"stop_times.txt";
	CT2CA pszConvertedAnsiString4 (str4.c_str());
	// construct a std::string using the LPCSTR input
	std::string  strStd4 (pszConvertedAnsiString4);
	if (parser.OpenCSVFile(strStd4))
	{
		int count =0;
		while(parser.ReadRecord())
		{
			PT_StopTime TransitStopTime;

			if(parser.GetValueByFieldName("stop_id",TransitStopTime.stop_id) == false)
				break;	

			if(parser.GetValueByFieldName("stop_sequence",TransitStopTime.stop_sequence) == false)
				break; 			

			if(parser.GetValueByFieldName("arrival_time",TransitStopTime.arrival_time) == false)
				break;

			if(parser.GetValueByFieldName("departure_time",TransitStopTime.departure_time) == false)
				break;	

			if(parser.GetValueByFieldName("trip_id",TransitStopTime.trip_id) == false)
				break; 

			if(parser.GetValueByFieldName("drop_off_type",TransitStopTime.drop_off_type) == false)
				TransitStopTime.drop_off_type=0; 

			if(parser.GetValueByFieldName("pickup_type",TransitStopTime.pickup_type) == false)
				TransitStopTime.pickup_type=0;

			if(parser.GetValueByFieldName("shape_dist_traveled",TransitStopTime.shape_dist_traveled) == false)
				TransitStopTime.shape_dist_traveled=0;	

			if(parser.GetValueByFieldName("stop_headsign",TransitStopTime.stop_headsign) == false)
				TransitStopTime.stop_headsign=0; 

			if(parser.GetValueByFieldName("timepoint",TransitStopTime.timepoint) == false)
				TransitStopTime.timepoint=0;	


			m_PT_StopTimeVector.push_back(TransitStopTime) ;

			count++;

		}
		parser.CloseCSVFile ();
	}

	//read route shape file
	string str5 = m_ProjectDirectory +"shapes.txt";
	CT2CA pszConvertedAnsiString5 (str5.c_str());
	// construct a std::string using the LPCSTR input
	std::string  strStd5 (pszConvertedAnsiString5);
	if (parser.OpenCSVFile(strStd5))
	{
		int count =0;
		while(parser.ReadRecord())
		{
			PT_shape_feature_point ft;
			//GDPoint point;

			if(parser.GetValueByFieldName("shape_id",ft.shape_id) == false)
				break;	

			if(parser.GetValueByFieldName("shape_pt_lat",ft.shape_pt_lat) == false)
				break; 			

			if(parser.GetValueByFieldName("shape_pt_lon",ft.shape_pt_lon) == false)
				break;

			if(parser.GetValueByFieldName("shape_pt_sequence",ft.shape_pt_sequence) == false)
				break;	

			//			if(parser.GetValueByFieldName("shape_dist_traveled",Tripshapes.shape_dist_traveled) == false)
			//				Tripshapes.shape_dist_traveled=0; 


			m_PT_shapes_map[ft.shape_id].feature_point_vector.push_back(ft);
			count++;

		}
		parser.CloseCSVFile ();
	}

	//read transfer file
	string str6 = m_ProjectDirectory +"transfers.txt";
	CT2CA pszConvertedAnsiString6 (str6.c_str());
	// construct a std::string using the LPCSTR input
	std::string  strStd6 (pszConvertedAnsiString6);
	if (parser.OpenCSVFile(strStd6))
	{
		int count =0;
		while(parser.ReadRecord())
		{
			PT_transfers transfers;

			if(parser.GetValueByFieldName("from_stop_id",transfers.from_stop_id) == false)
				break;	

			if(parser.GetValueByFieldName("to_stop_id",transfers.to_stop_id) == false)
				break; 			

			if(parser.GetValueByFieldName("transfer_type",transfers.transfer_type) == false)
				break;

			m_PT_transfers.push_back(transfers) ;
			count++;
		}
		parser.CloseCSVFile ();
	}



	// step 2: assemble data
	// add stop times into trips
	// add trips into routes
	std::map<int, PT_Route>::iterator iPT_RouteMap;
	std::map<int, PT_Trip>::iterator iPT_TripMap;
	std::map<int, PT_Stop>::iterator iPT_StopMap;
	std::vector<PT_StopTime>::iterator iPT_StopTimeVector;

	std::vector<PT_shapes>::iterator  iPT_shapes; 

	//find shape line point on the route by the (route_id) the route----(trip.id,route.id,shape.id)trip table--(shape_id)shapes table
	for ( iPT_RouteMap=m_PT_RouteMap.begin() ; iPT_RouteMap != m_PT_RouteMap.end(); iPT_RouteMap++ )
	{
		for ( iPT_TripMap=m_PT_TripMap.begin() ; iPT_TripMap != m_PT_TripMap.end(); iPT_TripMap++ )
		{
			if ((*iPT_RouteMap).second.route_id  ==(*iPT_TripMap).second.route_id )// find shape_id in the trip
			{
				//find shape line point on the route by the (route_id) the route----(trip.id,route.id,shape.id)trip table--(shape_id)shapes table

				if( m_PT_shapes_map.find((*iPT_TripMap).second.shape_id) != m_PT_shapes_map.end())
				{ // make sure shape id exists
					for ( int i = 0; i< m_PT_shapes_map[(*iPT_TripMap).second.shape_id].feature_point_vector.size(); i++)
					{ // for each feature point with the same shape id
						PT_shape_feature_point ft = m_PT_shapes_map[(*iPT_TripMap).second.shape_id].feature_point_vector[i];

						// insert the feature point according to the shape point sequence
						m_PT_RouteMap[(*iPT_RouteMap).first].m_RouteShapePoints[ft.shape_pt_sequence].y =ft.shape_pt_lat;
						m_PT_RouteMap[(*iPT_RouteMap).first].m_RouteShapePoints[ft.shape_pt_sequence].x =ft.shape_pt_lon;

						GDPoint pt;
						pt.x = ft.shape_pt_lon;
						pt.y = ft.shape_pt_lat;

						if( network_rect.PtInRect(pt) == true)
							(*iPT_RouteMap).second.bInsideFreewayNetwork = true;

					}

				}


				//find bus stop point on the route by the (route_id) the route----(trip.id,route.id,shape.id)trip table----(trip.id,stop.id)stoptime table--(stop.id)stop table
				for ( iPT_StopTimeVector=m_PT_StopTimeVector.begin() ; iPT_StopTimeVector != m_PT_StopTimeVector.end(); iPT_StopTimeVector++ )
				{
					if ((*iPT_StopTimeVector).trip_id == (*iPT_TripMap).second.trip_id)// find shape_id in the trip
					{ // find trip_id in the bus stop time ,then get stop_id, further obtain stop location in stop table
						iPT_StopMap= m_PT_StopMap.find((*iPT_StopTimeVector).stop_id);
						m_PT_RouteMap[(*iPT_RouteMap).first].m_RouteBusStopShapePoints[(*iPT_StopMap).second.stop_id]=(*iPT_StopMap).second.m_ShapePoint ;

			  }
				}    
				break;  // one shape point sequence for one signle trip id on the route

			}

		}




	}






	return true;
}

bool CTLiteDoc::ReadTransitFiles(CString TransitDataProjectFolder)
{

	CString str = TransitDataProjectFolder;
	CT2CA pszConvertedAnsiString (str);
	// construct a std::string using the LPCSTR input
	std::string  strStd (pszConvertedAnsiString);

	m_PT_network.m_ProjectDirectory = strStd;
	m_PT_network.ReadGTFFiles(m_NetworkRect);

	return true;
}


void CTLiteView::DrawPublicTransitLayer(CDC *pDC)
{

	CTLiteDoc* pDoc = GetDocument();

	CPoint ScreenPoint;

	std::map<int, PT_Route>::iterator iPT_RouteMap;
	std::map<int, PT_Stop>::iterator iPT_StopMap;

	std::map<int, GDPoint>::iterator i_RouteShapePoints;         //route line shape point 

	for(iPT_RouteMap = pDoc->m_PT_network.m_PT_RouteMap.begin (); iPT_RouteMap != pDoc->m_PT_network.m_PT_RouteMap.end(); iPT_RouteMap++)
	{

		bool bStartPointFlag  = true;

		PT_Route route = iPT_RouteMap->second ;

		if(route.bInsideFreewayNetwork == true)
		{
		for(i_RouteShapePoints = route.m_RouteShapePoints.begin (); i_RouteShapePoints != route.m_RouteShapePoints.end (); i_RouteShapePoints++)
		{

			GDPoint shape_point = i_RouteShapePoints->second;
			ScreenPoint = NPtoSP(shape_point);

			if(bStartPointFlag)
			{
				pDC->MoveTo (ScreenPoint);
				bStartPointFlag = false;
			}else
			{
				pDC->LineTo  (ScreenPoint);
			}
		}
		}
	}
}
