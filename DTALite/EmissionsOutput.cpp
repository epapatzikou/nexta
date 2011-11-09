//  Portions Copyright 2010 Xuesong Zhou, Wen Deng

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

#define CF_SIMULATION_INTERVAL_PER_SECOND 2
#define _MAXIMUM_OPERATING_MODE_SIZE 41

enum SPEED_BIN {VSP_0_25mph,VSP_25_50mph,VSP_GT50mph};
enum VSP_BIN {VSP_LT0,VSP_0_3,VSP_3_6,VSP_6_9,VSP_9_12,VSP_GT12,VSP_6_12,VSP_LT6};

int OperatingModeMap[9][3];

class CEmissionRate 
{
public:
			float meanBaseRate_TotalEnergy;
			float meanBaseRate_CO2;
			float meanBaseRate_NOX;
			float meanBaseRate_CO;
			float meanBaseRate_HC;
};
CEmissionRate EmissionRateData[MAX_VEHICLE_TYPE_SIZE][_MAXIMUM_OPERATING_MODE_SIZE];
class CVehicleEmission 
{
public:
			float Energy;
			float CO2;
			float NOX;
			float CO;
			float HC;

		CVehicleEmission(int vehicle_type, std::map<int, int> OperatingModeCount)
		{
			Energy = 0;
			CO2 = 0;
			NOX = 0;
			CO = 0;
			HC = 0;

			for(std::map<int, int>::iterator iterOP  = OperatingModeCount.begin(); iterOP != OperatingModeCount.end (); iterOP++)
			{
				int OpModeID = iterOP->first; int count = iterOP->second;

			Energy+= EmissionRateData[vehicle_type][OpModeID].meanBaseRate_TotalEnergy*count/3600;
			CO2+= EmissionRateData[vehicle_type][OpModeID].meanBaseRate_CO2*count/3600;
			NOX+= EmissionRateData[vehicle_type][OpModeID].meanBaseRate_NOX*count/3600;
			CO+= EmissionRateData[vehicle_type][OpModeID].meanBaseRate_CO*count/3600;
			HC+= EmissionRateData[vehicle_type][OpModeID].meanBaseRate_HC*count/3600;
			}

		}

};




struct VehicleCFData
{
	int   VehicleID;

	float FreeflowDistance_per_SimulationInterval;  
	float   CriticalSpacing_in_meter;

	int StartTime_in_SimulationInterval; // in time interval, LinkStartTime, so it should be sorted
	int EndTime_in_SimulationInterval; // in time interval
	int   TimeLag_in_SimulationInterval;


};

class ODStatistics
{
public: 
	ODStatistics()
	{
		TotalVehicleSize = 0;
		TotalTravelTime = 0;
		TotalDistance = 0;
		TotalCost = 0;
		TotalEmissions = 0;
	}

	int   TotalVehicleSize;
	float TotalTravelTime;
	float TotalDistance;
	float TotalCost;
	float TotalEmissions;

};

SPEED_BIN GetSpeedBinNo(float speed_mph)
{
   if(speed_mph <=25)
	   return VSP_0_25mph;

   if(speed_mph <= 50)
	   return VSP_25_50mph;
   else 
	   return VSP_GT50mph;

}

VSP_BIN GetVSPBinNo(float vsp, float speed_mph)
{
	if(speed_mph < 50)
	{
		if(vsp <0) return VSP_LT0;

		if(vsp <3) return VSP_0_3;

		if(vsp <6) return VSP_3_6;

		if(vsp <9) return VSP_6_9;

		if(vsp <12) 
			return VSP_9_12;
		else
			return VSP_GT12;

	}else  // greate than 50
	{
		if(vsp <6) 
			return VSP_LT6;
		if(vsp <12) 
			return VSP_6_12;
		else
			return VSP_GT12;
	}
}

void ReadInputEmissionRateFile()
{
	CCSVParser parser_emission;
	if (parser_emission.OpenCSVFile("input_vehicle_emission_rate.csv"))
	{
		
		while(parser_emission.ReadRecord())
		{
			int vehicle_type;
			int opModeID;

			if(parser_emission.GetValueByFieldName("vehicle_type",vehicle_type) == false)
				break;
			if(parser_emission.GetValueByFieldName("opModeID",opModeID) == false)
				break;

			CEmissionRate element;
			if(parser_emission.GetValueByFieldName("meanBaseRate_TotalEnergy_(J/hr)",element.meanBaseRate_TotalEnergy) == false)
				break;
			if(parser_emission.GetValueByFieldName("meanBaseRate_CO2_(g/hr)",element.meanBaseRate_CO2) == false)
				break;
			if(parser_emission.GetValueByFieldName("meanBaseRate_NOX_(g/hr)",element.meanBaseRate_NOX) == false)
				break;
			if(parser_emission.GetValueByFieldName("meanBaseRate_CO_(g/hr)",element.meanBaseRate_CO) == false)
				break;
			if(parser_emission.GetValueByFieldName("meanBaseRate_HC_(g/hr)",element.meanBaseRate_HC) == false)
				break;


			EmissionRateData[vehicle_type][opModeID] = element;
		}
	}else
	{
		cout << "Error: File input_vehicle_emission_rate.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();

	}

	cout << "Reading file input_vehicle_emission_rate.csv..."<< endl;

}

void SetupOperatingModeVector()
{
	OperatingModeMap[VSP_LT0][VSP_0_25mph] = 11;
	OperatingModeMap[VSP_0_3][VSP_0_25mph] = 12;
	OperatingModeMap[VSP_3_6][VSP_0_25mph] = 13;
	OperatingModeMap[VSP_6_9][VSP_0_25mph] = 14;
	OperatingModeMap[VSP_9_12][VSP_0_25mph] = 15;
	OperatingModeMap[VSP_GT12][VSP_0_25mph] = 16;

	OperatingModeMap[VSP_LT0][VSP_25_50mph] = 21;
	OperatingModeMap[VSP_0_3][VSP_25_50mph] = 22;
	OperatingModeMap[VSP_3_6][VSP_25_50mph] = 23;
	OperatingModeMap[VSP_6_9][VSP_25_50mph] = 24;
	OperatingModeMap[VSP_9_12][VSP_25_50mph] = 25;
	OperatingModeMap[VSP_GT12][VSP_25_50mph] = 26;

	OperatingModeMap[VSP_GT12][VSP_GT50mph] = 36;
	OperatingModeMap[VSP_6_12][VSP_GT50mph] = 35;
	OperatingModeMap[VSP_LT6][VSP_GT50mph] = 33;

}

int GetOperatingMode(float vsp, float s_mph)
{

	SPEED_BIN SpeedBinnNo = GetSpeedBinNo(s_mph);
	VSP_BIN VSPBinnNo =  GetVSPBinNo(vsp, s_mph);

	int OPBin =  OperatingModeMap[VSPBinnNo][SpeedBinnNo];
	return OPBin;
}

int ComputeOperatingModeFromSpeed(float v,float a, float grade = 0, float Mass = 1.4788f)
{
	float TermA = 0.156461f;
	float TermB = 0.00200193f;
	float vsp = TermA/Mass*v + TermB/Mass*v*v+  TermB/Mass*v*v*v;
	float speed_mph = v * 2.23693629f;

	int OpMode = GetOperatingMode(vsp,speed_mph);
	return OpMode; 
}


void OutputEmissionData()
{
	ReadInputEmissionRateFile();
	SetupOperatingModeVector();

	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
		TRACE("\n compute emissions for link %d",li);
		cout << " ------------------------------ "  << endl;
		cout << " compute emissions for link " << li << endl;
		ComputeVSPForSingleLink(li);

	}

	// output speed data second by second

	FILE* st = NULL;
	FILE* st2 = NULL;

	fopen_s(&st,"output_vehicle_speed_data.txt","w");
	fopen_s(&st2,"output_vehicle_operating_mode.txt","w");

	if(st!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;
		//calculate the toll cost and emission cost
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{
			DTAVehicle* pVehicle = iterVM->second;

			pVehicle->m_OperatingModeCount.clear ();

//			fprintf(st, "Vehicle = %d\n", pVehicle->m_VehicleID );
			fprintf(st2, "Vehicle = %d\n", pVehicle->m_VehicleID );

//			fprintf(st, "number of speed data points = %d\n", pVehicle->m_SpeedVector .size());

			for(int time_index  =  0; time_index < pVehicle->m_SpeedVector .size(); time_index++)
			{
//				fprintf(st, "t = %d,%4.1f\n", pVehicle->m_SpeedVector[time_index].timestamp_in_second,pVehicle->m_SpeedVector[time_index].speed* 2.23693629f); // * 2.23693629 -> meter per second to mph
				
				float accelation = 0;
				if( time_index < pVehicle->m_SpeedVector .size() -1)
					accelation = pVehicle->m_SpeedVector[time_index+1].speed - pVehicle->m_SpeedVector[time_index].speed;

				int OperatingMode = ComputeOperatingModeFromSpeed(pVehicle->m_SpeedVector[time_index].speed, accelation);
				pVehicle->m_OperatingModeCount[OperatingMode]+=1;
				
			}
			fprintf(st2, "number of operating mode data points = %d\n", pVehicle->m_OperatingModeCount.size());

			for(std::map<int, int>::iterator iterOP  =  pVehicle->m_OperatingModeCount.begin(); iterOP != pVehicle->m_OperatingModeCount.end (); iterOP++)
			{
				fprintf(st2, "op=%d,%d\n", iterOP->first ,iterOP->second );
			}

		}
	
		fclose(st);
		fclose(st2);
	}

	fopen_s(&st,"output_vehicle_emission_MOE_summary.csv","w");

	if(st!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;

			fprintf(st, "vehicle_id,from_zone_id,to_zone_id,departure_time,vehicle_type,information_type,TotalEnergy_(J),CO2_(g),NOX_(g),CO_(g),HC_(g)\n");

		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;
			if(pVehicle->m_NodeSize >= 2 && pVehicle->m_OperatingModeCount.size()>0)  // with physical path in the network
			{
				CVehicleEmission element(pVehicle->m_VehicleType, pVehicle->m_OperatingModeCount );

				float m_gap = 0;
				fprintf(st,"%d,%d,%d,%4.2f,%d,%d,%f,%f,%f,%f,%f\n",
					pVehicle->m_VehicleID , pVehicle->m_OriginZoneID , pVehicle->m_DestinationZoneID,
					pVehicle->m_DepartureTime, pVehicle->m_VehicleType ,pVehicle->m_InformationClass,
					element.Energy ,element.CO2, element.NOX, element.CO, element.HC);
			}
		}
	   fclose(st);
	}

}
void ComputeVSPForSingleLink(int LinkNo)
{
	float** VechileDistanceAry;

	std::vector<VehicleCFData> m_VehicleDataVector;  // store data for vehicles passing through a lane of given link

	int m_NumberOfLanes = g_LinkVector[LinkNo]->m_NumLanes ;

	int m_NumberOfTimeSteps = g_SimulationHorizon*60*CF_SIMULATION_INTERVAL_PER_SECOND;  //60 seconds per min, 0.1 second as simulation interval

	for(int lane_no = 0; lane_no < m_NumberOfLanes; lane_no++)
	{
		m_VehicleDataVector.clear ();
		// step 1: for each lane
		// count the number of vehicles

		std::map<int, DTAVehicle*>::iterator iterVM;
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{
			DTAVehicle* pVehicle = iterVM->second;
			if( (pVehicle->m_VehicleID %m_NumberOfLanes == lane_no) && pVehicle->m_NodeSize>=2)
			{
				for(int i=0; i< pVehicle->m_NodeSize-1; i++)
				{
					if(pVehicle->m_aryVN [i].LinkID == LinkNo && pVehicle->m_aryVN[i].AbsArrivalTimeOnDSN < g_SimulationHorizon)  // for vehicles finish the trips
					{

						VehicleCFData element;
						element.VehicleID = pVehicle->m_VehicleID ;
						element.FreeflowDistance_per_SimulationInterval  = g_LinkVector[LinkNo]->m_SpeedLimit* 1609.344f/3600/CF_SIMULATION_INTERVAL_PER_SECOND; 
						//1609.344f: mile to meters ; 3600: # of seconds per hour
									//τ = 1/(wkj)
									// δ = 1/kj
						element.TimeLag_in_SimulationInterval = (int)(3600*CF_SIMULATION_INTERVAL_PER_SECOND/(g_LinkVector[LinkNo]->m_BackwardWaveSpeed * g_LinkVector[LinkNo]->m_KJam )+0.5);
						element.CriticalSpacing_in_meter = 1609.344f / g_LinkVector[LinkNo]->m_KJam ; 

						if(i == 0)
							element.StartTime_in_SimulationInterval = pVehicle->m_DepartureTime*60*CF_SIMULATION_INTERVAL_PER_SECOND;
						else
							element.StartTime_in_SimulationInterval = pVehicle->m_aryVN [i-1].AbsArrivalTimeOnDSN*60*CF_SIMULATION_INTERVAL_PER_SECOND;

						element.EndTime_in_SimulationInterval = pVehicle->m_aryVN [i].AbsArrivalTimeOnDSN*60*CF_SIMULATION_INTERVAL_PER_SECOND;

						m_VehicleDataVector.push_back (element);
						break;
					}
				}
			 // for each vehicle
			}
		}
		// // step 2: allocate dynamic data memory 

		VechileDistanceAry	= AllocateDynamicArray<float>(m_VehicleDataVector.size(),m_NumberOfTimeSteps);

		//allocate arrays and initialize data
		int t,v;

		for(t=0; t<m_NumberOfTimeSteps; t++)
		{
			for(v=0; v<m_VehicleDataVector.size(); v++)
			{
				VechileDistanceAry [v][t] = 0;
			}
		}
		// step 2: car following simulation

		float link_length_in_meter = g_LinkVector[LinkNo]->m_Length * 1609.344f; //1609.344f: mile to meters


		for(t=1; t<m_NumberOfTimeSteps; t++)
		{


			if(t%(60*CF_SIMULATION_INTERVAL_PER_SECOND*10) == 0)
			{
			int time_in_min = t/(60*CF_SIMULATION_INTERVAL_PER_SECOND);
			cout << " car following simulation at min " <<  time_in_min << endl;
			}

			for(v=0; v<m_VehicleDataVector.size(); v++)
			{

				DTAVehicle* pVehicle  = g_VehicleMap[m_VehicleDataVector[v].VehicleID];

				// for active vehicles (with positive speed or positive distance
				if(t>= m_VehicleDataVector[v].StartTime_in_SimulationInterval && t<= m_VehicleDataVector[v].EndTime_in_SimulationInterval ) 
				{
					//calculate free-flow position
					//xiF(t) = xi(t-τ) + vf(τ)
					VechileDistanceAry[v][t] = min(link_length_in_meter, VechileDistanceAry[v][t-1] +  m_VehicleDataVector[v].FreeflowDistance_per_SimulationInterval);
//					TRACE("veh %d, time%d,%f\n",v,t,VechileDistanceAry[v][t]);

					//calculate congested position
					if(v>=1)
					{
						//xiC(t) = xi-1(t-τ) - δ
						int time_t_minus_tau = t - m_VehicleDataVector[v].TimeLag_in_SimulationInterval; // need to convert time in second to time in simulation time interval

						if(time_t_minus_tau >=0)  // the leader has not reached destination yet
						{
							float CongestedDistance = VechileDistanceAry[v-1][time_t_minus_tau]  - m_VehicleDataVector[v].CriticalSpacing_in_meter ;
							// xi(t) = min(xAF(t), xAC(t))
							if (VechileDistanceAry[v][t]  > CongestedDistance && CongestedDistance >= VechileDistanceAry[v][t-1])
								VechileDistanceAry[v][t] = CongestedDistance;
						}

					}

					// output speed per second

					if(t%CF_SIMULATION_INTERVAL_PER_SECOND == 0)  // per_second
					{
						float SpeedBySecond = g_LinkVector[LinkNo]->m_SpeedLimit* 0.44704f ; // 1 mph = 0.44704 meters per second

						if(t >= m_VehicleDataVector[v].StartTime_in_SimulationInterval + CF_SIMULATION_INTERVAL_PER_SECOND)  // not the first second
						{
							SpeedBySecond = (VechileDistanceAry[v][t] - VechileDistanceAry[v][t-CF_SIMULATION_INTERVAL_PER_SECOND]);
						}

						VehicleTimestampSpeed element;
						element.timestamp_in_second = t%CF_SIMULATION_INTERVAL_PER_SECOND;
						element.speed = SpeedBySecond;
						pVehicle->m_SpeedVector.push_back(element);
					}

				}  // for active vehicle
			} // for each vehicle
		}  // for each time interval

	if(VechileDistanceAry!=NULL)
	{
		DeallocateDynamicArray<float>(VechileDistanceAry,m_VehicleDataVector.size(), m_NumberOfTimeSteps);

	}

	}

}


