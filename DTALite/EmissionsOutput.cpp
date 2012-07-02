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

#define NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND 4
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
			int demand_type;
			int opModeID;

			if(parser_emission.GetValueByFieldName("vehicle_type",demand_type) == false)
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


			EmissionRateData[demand_type][opModeID] = element;
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


bool vehicleCF_sort_function (VehicleCFData i,VehicleCFData j) 
{ 
	return (i.StartTime_in_SimulationInterval <j.StartTime_in_SimulationInterval );
}

void OutputEmissionData()
{
	ReadInputEmissionRateFile();
	SetupOperatingModeVector();

	std::set<DTALink*>::iterator iterLink;
	int count = 0;
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
				DTALink* pLink = g_LinkVector[li];
				for(int LaneNo = 0; LaneNo < pLink->m_NumLanes; LaneNo++)
				{
					LaneVehicleCFData element; 
					pLink->m_VehicleDataVector.push_back (element);
				}
	}

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{
		DTAVehicle* pVehicle = iterVM->second;
		if( pVehicle->m_NodeSize>=2)
		{
			for(int i=0; i< pVehicle->m_NodeSize-1; i++)
			{
				if(pVehicle->m_aryVN[i].AbsArrivalTimeOnDSN < g_PlanningHorizon)  // for vehicles finish the trips
				{
					VehicleCFData element;
					element.VehicleID = pVehicle->m_VehicleID ;
					int LinkNo = pVehicle->m_aryVN[i].LinkID ;
					element.SequentialLinkNo = i;

					element.FreeflowDistance_per_SimulationInterval  = g_LinkVector[LinkNo]->m_SpeedLimit* 1609.344f/3600/NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND; 
					//1609.344f: mile to meters ; 3600: # of seconds per hour
					//τ = 1/(wkj)
					// δ = 1/kj
					element.TimeLag_in_SimulationInterval = (int)(3600*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND/(g_LinkVector[LinkNo]->m_BackwardWaveSpeed * g_LinkVector[LinkNo]->m_KJam )+0.5);
					element.CriticalSpacing_in_meter = 1609.344f / g_LinkVector[LinkNo]->m_KJam ; 

					if(i == 0)
						element.StartTime_in_SimulationInterval = pVehicle->m_DepartureTime*60*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
					else
						element.StartTime_in_SimulationInterval = pVehicle->m_aryVN [i-1].AbsArrivalTimeOnDSN*60*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;

					element.EndTime_in_SimulationInterval = pVehicle->m_aryVN [i].AbsArrivalTimeOnDSN*60*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
					element.LaneNo = element.VehicleID%g_LinkVector[LinkNo]->m_NumLanes;

					g_LinkVector[LinkNo]->m_VehicleDataVector[element.LaneNo].LaneData .push_back (element);

				}
			}
			// for each vehicle
		}
	}


	for(int li = 0; li< g_LinkVector.size(); li++)
	{
		//		TRACE("\n compute emissions for link %d",li);
		cout << "compute emissions for link " << li << endl;
		g_LinkVector[li]->ComputeVSP_FastMethod();
	}

	// output speed data second by second

	FILE* st2;
	fopen_s(&st2,"output_vehicle_operating_mode.txt","w");

	if(st2!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;
		//calculate the toll cost and emission cost
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			//			fprintf(st, "Vehicle = %d\n", pVehicle->m_VehicleID );
			fprintf(st2, "Vehicle = %d\n", pVehicle->m_VehicleID );

			//			fprintf(st, "number of speed data points = %d\n", pVehicle->m_SpeedVector .size());

			fprintf(st2, "number of operating mode data points = %d\n", pVehicle->m_OperatingModeCount.size());

			for(std::map<int, int>::iterator iterOP  =  pVehicle->m_OperatingModeCount.begin(); iterOP != pVehicle->m_OperatingModeCount.end (); iterOP++)
			{
				fprintf(st2, "op=%d,%d\n", iterOP->first ,iterOP->second );
			}

		}

		fclose(st2);
	}

	FILE* st;
	fopen_s(&st,"output_vehicle_emission_MOE_summary.csv","w");

	float Energy = 0;
	float CO2 = 0;
	float NOX = 0;
	float CO = 0;
	float HC = 0;

	if(st!=NULL)
	{
		std::map<int, DTAVehicle*>::iterator iterVM;

		fprintf(st, "vehicle_id,from_zone_id,to_zone_id,departure_time,vehicle_type,information_type,TotalEnergy_(J),CO2_(g),NOX_(g),CO_(g),HC_(g)\n");

		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;
			if(pVehicle->m_NodeSize >= 2 && pVehicle->m_OperatingModeCount.size()>0)  // with physical path in the network
			{
				CVehicleEmission element(pVehicle->m_VehicleType , pVehicle->m_OperatingModeCount );

				float m_gap = 0;
				fprintf(st,"%d,%d,%d,%4.2f,%d,%d,%f,%f,%f,%f,%f\n",
					pVehicle->m_VehicleID , pVehicle->m_OriginZoneID , pVehicle->m_DestinationZoneID,
					pVehicle->m_DepartureTime, pVehicle->m_VehicleType  ,pVehicle->m_InformationClass,
					element.Energy ,element.CO2, element.NOX, element.CO, element.HC);

			Energy+=element.Energy;
			CO2+=element.CO2;
			NOX += element.NOX;
			CO += element.CO;
			HC += element.HC;

			}
		}
		fclose(st);
	}
	if(ShortSimulationLogFile.is_open())
	{
		ShortSimulationLogFile << "," << Energy/1000 << "," << CO2/1000 << endl;
	}

}
void DTALink::ComputeVSP()  // VSP: vehicle specific power
{
#pragma omp parallel for 
	for(int LaneNo = 0; LaneNo < m_NumLanes; LaneNo++)
	{
		// step 1: sort vehicle arrival times
		sort(m_VehicleDataVector[LaneNo].LaneData.begin(), m_VehicleDataVector[LaneNo].LaneData.end(), vehicleCF_sort_function);

		int m_NumberOfTimeSteps = m_SimulationHorizon*60*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
		float **VechileDistanceAry	= AllocateDynamicArray<float>(m_VehicleDataVector[LaneNo].LaneData.size(),m_NumberOfTimeSteps);
		// step 2: car following simulation
		float link_length_in_meter = m_Length * 1609.344f; //1609.344f: mile to meters

		int t;
		int v_start = 0;

		for(t=1; t<m_NumberOfTimeSteps; t++)  // every time interval int the whole planning horizon
		{
			int v;
			for(v = v_start; v<m_VehicleDataVector[LaneNo].LaneData.size(); v++)  // every vehicle
			{

				if(t>= m_VehicleDataVector[LaneNo].LaneData[v].StartTime_in_SimulationInterval ) 
				{
					//calculate free-flow position
					//xiF(t) = xi(t-τ) + vf(τ)
					VechileDistanceAry[v][t] = min(link_length_in_meter, VechileDistanceAry[v][t-1] +  m_VehicleDataVector[LaneNo].LaneData[v].FreeflowDistance_per_SimulationInterval);
					//					TRACE("veh %d, time%d,%f\n",v,t,VechileDistanceAry[v][t]);

					//calculate congested position
					if(v>=1)
					{
						//xiC(t) = xi-1(t-τ) - δ
						int time_t_minus_tau = t - m_VehicleDataVector[LaneNo].LaneData[v].TimeLag_in_SimulationInterval; // need to convert time in second to time in simulation time interval

						if(time_t_minus_tau >=0)  // the leader has not reached destination yet
						{
							// vehicle v-1: previous car
							float CongestedDistance = VechileDistanceAry[v-1][time_t_minus_tau]  - m_VehicleDataVector[LaneNo].LaneData[v].CriticalSpacing_in_meter ;
							// xi(t) = min(xAF(t), xAC(t))
							if (VechileDistanceAry[v][t]  > CongestedDistance && CongestedDistance >= VechileDistanceAry[v][t-1])
								VechileDistanceAry[v][t] = CongestedDistance;
						}

					}

					// output speed per second

					if(t%NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND == 0)  // per_second
					{
						// for active vehicles (with positive speed or positive distance

						float SpeedBySecond = m_SpeedLimit* 0.44704f ; // 1 mph = 0.44704 meters per second

						if(t >= m_VehicleDataVector[LaneNo].LaneData[v].StartTime_in_SimulationInterval + NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND)  // not the first second
						{
							SpeedBySecond = (VechileDistanceAry[v][t] - VechileDistanceAry[v][t-NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND]);
						}

						// different lanes have different vehicle numbers, so we should not have openMP conflicts here
						DTAVehicle* pVehicle  = g_VehicleMap[m_VehicleDataVector[LaneNo].LaneData[v].VehicleID];
						float accelation = SpeedBySecond - pVehicle->m_PrevSpeed ;

						int OperatingMode = ComputeOperatingModeFromSpeed(SpeedBySecond, accelation);
						pVehicle->m_OperatingModeCount[OperatingMode]+=1;
						pVehicle->m_PrevSpeed  = SpeedBySecond;
					}

				}  // for active vehicle
				if( t > m_VehicleDataVector[LaneNo].LaneData[v].EndTime_in_SimulationInterval)
				{
					v_start = v+1;  // re-start from a large sequence number
					break;  // complete the trip for this vehicle on this link, no need to continue at this time t loop
				}

			} // for each vehicle

			if(v_start >= m_VehicleDataVector[LaneNo].LaneData.size())
			{
				break;   // all the vehicles complete trips
			}
		}  // for each time interval

		if(VechileDistanceAry!=NULL)
		{
			DeallocateDynamicArray<float>(VechileDistanceAry,m_VehicleDataVector[LaneNo].LaneData .size(), m_NumberOfTimeSteps);
		}

	}

}

void DTALink::ComputeVSP_FastMethod()  // VSP: vehicle specific power
{
		// step 1: car following simulation
		float link_length_in_meter = m_Length * 1609.344f; //1609.344f: mile to meters

		float BackwardWaveSpeed_in_meter_per_second = m_BackwardWaveSpeed  * 1609.344f / 3600; 
	for(int LaneNo = 0; LaneNo < m_NumLanes; LaneNo++)
	{
			int v;
			for(v = 0; v<m_VehicleDataVector[LaneNo].LaneData.size(); v++)
			{
				float distance = 0;  // from the starting point of the link to the current location
				float prev_distance = 0;

				DTAVehicle* pVehicle  = g_VehicleMap[m_VehicleDataVector[LaneNo].LaneData[v].VehicleID];

				std::map<int, int> l_OperatingModeCount;

				int SequentialLinkNo = m_VehicleDataVector[LaneNo].LaneData[v].SequentialLinkNo ;
				int VehicleLaneBasedCumulativeFlowCount  = pVehicle->m_aryVN[SequentialLinkNo].LaneBasedCumulativeFlowCount;

				for(int t = m_VehicleDataVector[LaneNo].LaneData[v].StartTime_in_SimulationInterval;
					t < m_VehicleDataVector[LaneNo].LaneData[v].EndTime_in_SimulationInterval; t+=1*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND) 
				{
					//calculate free-flow position
					//xiF(t) = xi(t-τ) + vf(τ)
					prev_distance = distance; 
					distance = min(link_length_in_meter, distance +  m_VehicleDataVector[LaneNo].LaneData[v].FreeflowDistance_per_SimulationInterval);
					//					TRACE("veh %d, time%d,%f\n",v,t,VechileDistanceAry[v][t]);

					//calculate congested position
						float BWTT_in_second  = (link_length_in_meter -  distance)/max(0.001,BackwardWaveSpeed_in_meter_per_second);
						int time_t_minus_BWTT = t - BWTT_in_second*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND; // need to convert time in second to time in simulation time interval
						int DTASimulationTimeIntervalNo = time_t_minus_BWTT*g_DTASimulationInterval;

						if(DTASimulationTimeIntervalNo < 0)
							DTASimulationTimeIntervalNo = 0;
						if(DTASimulationTimeIntervalNo >= m_CumuDeparturelFlow.size() -1)
							DTASimulationTimeIntervalNo =  m_CumuDeparturelFlow.size() -2;

						int DownstreamCumulativeDepartureCount = m_CumuDeparturelFlow [DTASimulationTimeIntervalNo];
						// reference: Newell's 3-detector theory 
							if(DownstreamCumulativeDepartureCount < ( VehicleLaneBasedCumulativeFlowCount- (link_length_in_meter -  distance)/ 1609.344f * m_KJam))  // / 1609.344f converts meter to mile
							{
							distance = prev_distance ; // DO NOT MOVE! jamed by backward wave!
							}
				
							float SpeedBySecond  = (distance - prev_distance)* 0.44704f; // // 1 mph = 0.44704 meters per second

					// output speed per second

					if(t%NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND == 0)  // per_second
					{
						// for active vehicles (with positive speed or positive distance

						// different lanes have different vehicle numbers, so we should not have openMP conflicts here
						float accelation = SpeedBySecond - pVehicle->m_PrevSpeed ;

						int OperatingMode = ComputeOperatingModeFromSpeed(SpeedBySecond, accelation);
						pVehicle->m_OperatingModeCount[OperatingMode]+=1;
						pVehicle->m_PrevSpeed  = SpeedBySecond;

						l_OperatingModeCount[OperatingMode]+=1;

					}

					}  // for each time t

					// calculate subtotal emissions for each vehicle
					CVehicleEmission element(pVehicle->m_VehicleType , pVehicle->m_OperatingModeCount );

					// add the vehicle specific emissions to link-based emission statistics
					m_TotalEnergy +=element.Energy;
					m_CO2 += element.CO2;
					m_NOX +=  element.NOX;
					m_CO += element.CO;
					m_HC +=element.HC;

			}  // for active vehicle
	
	} // for each lane
}

