//  Portions Copyright 2010 Xuesong Zhou, Hao Lei

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
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

#include "stdafx.h"
#include "DTALite.h"
#include "GlobalData.h"

float*** g_HistODDemand;
float*** g_CurrentODDemand;
float*** g_ODDemandProportion;
float*** g_ODDemandAdjustment;

int g_ODDemandIntervalSize = 1;

void g_ReadHistDemandFile()
{
	FILE* st = NULL;

	fopen_s(&st,"input_hist_demand.csv","r");
	if(st!=NULL)
	{
		cout << "Reading file input_hist_demand.csv..."<< endl;

		int line_no = 1;
		int originput_zone, destination_zone;
		float number_of_vehicles ;
		int vehicle_type;
		float starting_time_in_min;
		float ending_time_in_min;

		while(!feof(st))
		{
			originput_zone = g_read_integer(st);
			if(originput_zone == -1)  // reach end of file
				break;

			destination_zone =  g_read_integer(st);
			number_of_vehicles =  g_read_float(st);

			vehicle_type =  g_read_integer(st);
			starting_time_in_min = g_read_float(st);
			ending_time_in_min = g_read_float(st);

			int time_interval_no = starting_time_in_min/g_DepartureTimetInterval;

			if(time_interval_no <g_ODDemandIntervalSize)
			{
			g_HistODDemand [originput_zone][destination_zone][time_interval_no]  = number_of_vehicles;
			}else
			{
			//error message
			}

//			TRACE("o:%d d: %d, %f,%d,%f,%f\n", originput_zone,destination_zone,number_of_vehicles,vehicle_type,starting_time_in_min,ending_time_in_min);

			float Interval= float(ending_time_in_min - starting_time_in_min);

			if(line_no %100000 ==0)
			{
				cout << g_GetAppRunningTime() << "Reading " << line_no/1000 << "K lines..."<< endl;
			}

			line_no++;
		}

		fclose(st);
	}
}

void g_ReadCurrentDemandFile()
{
	FILE* st = NULL;

	fopen_s(&st,"input_demand.csv","r");
	if(st!=NULL)
	{
		cout << "Reading file input_demand.csv..."<< endl;

		int line_no = 1;
		int originput_zone, destination_zone;
		float number_of_vehicles ;
		int vehicle_type;
		float starting_time_in_min;
		float ending_time_in_min;

		while(!feof(st))
		{
			originput_zone = g_read_integer(st);
			if(originput_zone == -1)  // reach end of file
				break;

			destination_zone =  g_read_integer(st);
			number_of_vehicles =  g_read_float(st);

			vehicle_type =  g_read_integer(st);
			starting_time_in_min = g_read_float(st);
			ending_time_in_min = g_read_float(st);

			int time_interval_no = starting_time_in_min/g_DepartureTimetInterval;

			if(time_interval_no <g_ODDemandIntervalSize)
			{
			g_CurrentODDemand [originput_zone][destination_zone][time_interval_no]  = number_of_vehicles;
			}else
			{
			//error message
			}

//			TRACE("o:%d d: %d, %f,%d,%f,%f\n", originput_zone,destination_zone,number_of_vehicles,vehicle_type,starting_time_in_min,ending_time_in_min);

			float Interval= float(ending_time_in_min - starting_time_in_min);

			if(line_no %100000 ==0)
			{
				cout << g_GetAppRunningTime() << "Reading " << line_no/1000 << "K lines..."<< endl;
			}

			line_no++;
		}

		fclose(st);
	}
}
void g_ODDemandEstimation()
{
	g_ODDemandIntervalSize = g_DemandLoadingHorizon/g_DepartureTimetInterval;

	g_HistODDemand = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);
	g_CurrentODDemand = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);

	g_ODDemandProportion = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);
	g_ODDemandAdjustment = Allocate3DDynamicArray<float>(g_ODZoneSize+1,g_ODZoneSize+1,g_ODDemandIntervalSize);
	//+1 to make the OD zone index begin at 1

	g_ReadHistDemandFile();
 
/* optimization notes
	given r as measurment error variance, P is OD-tau-to-link proportion coefficient

	close-form solution for OD demand adjustment
	= P'/(P'*P+r) *error
		 where P'*P is sum of (product of OD-to-link proportion) over all OD pairs
	
*/

  // outer loop for each adjustment iteration
  // for each outer loop
  
  // outer step 1: traffic simulation/assignment, call g_TrafficAssignmentSimulation()
  // outer step 2: g_ReadCurrentDemandFile();
  // outer step 3: initialize g_ODDemandAdjustment(O,D,tau) to zero
  // outer step 4: adjust OD for each link count

	/* below are inner steps for outer step 4
	// read link observation
	for each link count observation (a,t)
	{
		// below are  inner loop steps
		// step 0: calculate link observation deviation as dev
		// step 1: initialize g_ODDemandProportion(O,D,tau) = 0

		// steps below calculate link proportions
		// step 2. scan all vehicles, if vehicle v passes through link (a,t) then  g_ODDemandProportion(O,D,tau)+=1

		// step 3. g_ODDemandProportion(O,D,tau)  = g_ODDemandProportion(O,D,tau) /g_CurrentODDemand(O,D,tau)

		// step 4. calculate SumP=P'*P over all O, D, tau

		// step 5: for each time-dependent OD demand pair
		g_ODDemandAdjustment(O,D,tau) = g_ODDemandProportion(O,D,tau)/(SumP + r) * dev

	}
	endfor

	// outer step 5: add g_ODDemandAdjustment for each current ODdemand g_CurrentODDemand
	// if (g_ODDemandAdjustment+g_CurrentODDemand) is out of the bound of historical demand, reset to the bound

	// outer step 6: output g_CurrentODDemand to new demand table  
	
	//end for outer loop
	*/
	Deallocate3DDynamicArray<float>(g_HistODDemand,g_ODZoneSize+1,g_ODZoneSize+1);
	Deallocate3DDynamicArray<float>(g_CurrentODDemand,g_ODZoneSize+1,g_ODZoneSize+1);
	Deallocate3DDynamicArray<float>(g_ODDemandProportion,g_ODZoneSize+1,g_ODZoneSize+1);
	Deallocate3DDynamicArray<float>(g_ODDemandAdjustment,g_ODZoneSize+1,g_ODZoneSize+1);
}