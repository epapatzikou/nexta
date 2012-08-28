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

const int NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND = 10;
#define _MAXIMUM_OPERATING_MODE_SIZE 41


int OperatingModeMap[9][3] = {-1};

class CEmissionRate 
{
public:
	bool bInitialized;
	float meanBaseRate_TotalEnergy;
	float meanBaseRate_CO2;
	float meanBaseRate_NOX;
	float meanBaseRate_CO;
	float meanBaseRate_HC;

	CEmissionRate()
	{
		bInitialized = false;
		meanBaseRate_TotalEnergy = meanBaseRate_CO2 = meanBaseRate_NOX = meanBaseRate_CO = meanBaseRate_HC = 0;
		
	}
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
			int vehicle_type;
			int opModeID;

			if(parser_emission.GetValueByFieldName("vehicle_type",vehicle_type) == false)
				break;
			if(parser_emission.GetValueByFieldName("opModeID",opModeID) == false)
				break;

			CEmissionRate element;
			if(parser_emission.GetValueByFieldName("meanBaseRate_TotalEnergy_(KJ/hr)",element.meanBaseRate_TotalEnergy) == false)
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
			EmissionRateData[vehicle_type][opModeID].bInitialized = true;
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
	OperatingModeMap[VSP_GT12][VSP_25_50mph] = 27;  // change from 26 to 27 need to check with NCSU team

	OperatingModeMap[VSP_GT12][VSP_GT50mph] = 37;  // // change from 36 to 27 need to check with NCSU team
	OperatingModeMap[VSP_6_12][VSP_GT50mph] = 35;
	OperatingModeMap[VSP_LT6][VSP_GT50mph] = 33;

}

int GetOperatingMode(float vsp, float s_mph)
{

	SPEED_BIN SpeedBinNo = GetSpeedBinNo(s_mph);
	VSP_BIN VSPBinNo =  GetVSPBinNo(vsp, s_mph);

	int OPBin =  OperatingModeMap[VSPBinNo][SpeedBinNo];
	return OPBin;
}

int ComputeOperatingModeFromSpeed(float &vsp, float v /*meter per second*/,float a, float grade = 0, float Mass = 1.4788f)
{
	float TermA = 0.156461f;
	float TermB = 0.00200193f;
	vsp = TermA/Mass*v + TermB/Mass*v*v+  TermB/Mass*v*v*v;
	float speed_mph = v * 2.23693629f;  //3600 seconds / 1606 meters per hour

	int OpMode = GetOperatingMode(vsp,speed_mph);
	
	return OpMode; 
}


bool vehicleCF_sort_function (VehicleCFData i,VehicleCFData j) 
{ 
	return (i.StartTime_in_SimulationInterval <j.StartTime_in_SimulationInterval );
}

void OutputEmissionData()
{
	if(g_TrafficFlowModelFlag==0)
		return; // no emission output for BPR function

	ReadInputEmissionRateFile();
	SetupOperatingModeVector();

	std::set<DTALink*>::iterator iterLink;
	int count = 0;
	// step 1: generate data for different lanes
	for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
				DTALink* pLink = g_LinkVector[li];
				for(int LaneNo = 0; LaneNo < pLink->m_NumLanes; LaneNo++)
				{
					LaneVehicleCFData element(g_PlanningHorizon+1); 
					pLink->m_VehicleDataVector.push_back (element);
				}
	}

	// step 2: collect all vehicle passing each link

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{
		DTAVehicle* pVehicle = iterVM->second;
		if( pVehicle->m_NodeSize>=2 && pVehicle->m_PricingType !=4)
		{
			for(int i=0; i< pVehicle->m_NodeSize-1; i++)
			{
				if(pVehicle->m_bComplete )  // for vehicles finish the trips
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


	// calclate link based VSP
	// step 2: collect all vehicle passing each link


	for(int li = 0; li< g_LinkVector.size(); li++)
	{
		TRACE("\n compute emissions for link %d",li);
//		g_LogFile << "compute emissions for link " << li << endl;
		cout << "compute emissions for link " << li << endl;
		g_LinkVector[li]->ComputeVSP();
	}

	float Energy = 0;
	float CO2 = 0;
	float NOX = 0;
	float CO = 0;
	float HC = 0;


		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;
			if(pVehicle->m_NodeSize >= 2 && pVehicle->m_OperatingModeCount.size()>0)  // with physical path in the network
			{
				CVehicleEmission element(pVehicle->m_VehicleType , pVehicle->m_OperatingModeCount );

				pVehicle->m_Emissions = element.CO2;
				pVehicle->Energy =  element.Energy ;
				pVehicle->CO2 =  element.CO2 ;
				pVehicle->NOX =  element.NOX ;
				pVehicle->CO =  element.CO ;
				pVehicle->HC =  element.HC ;

			Energy+=element.Energy;
			CO2+=element.CO2;
			NOX += element.NOX;
			CO += element.CO;
			HC += element.HC;

			}
		}

		g_SimulationResult.Energy = Energy;
		g_SimulationResult.CO2 = CO2;
		g_SimulationResult.NOX = NOX;
		g_SimulationResult.CO = CO;
		g_SimulationResult.HC = HC;

			for(unsigned li = 0; li< g_LinkVector.size(); li++)
	{
				DTALink* pLink = g_LinkVector[li];

				 int TimeSize =  pLink->m_LinkMOEAry.size();
				 for(int t=0; t< TimeSize; t++)
				 {
						for(int LaneNo = 0; LaneNo < pLink->m_NumLanes; LaneNo++)
						{
						 pLink->m_LinkMOEAry [t].Energy += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].Energy;
						 pLink->m_LinkMOEAry [t].CO2 += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].CO2;
						 pLink->m_LinkMOEAry [t].NOX += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].NOX;
						 pLink->m_LinkMOEAry [t].CO += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].CO;
						 pLink->m_LinkMOEAry [t].HC += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].HC;

						 pLink->TotalEnergy += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].Energy;
						 pLink->TotalCO2 += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].CO2;
						 pLink->TotalNOX += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].NOX;
						 pLink->TotalCO += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].CO;
						 pLink->TotalHC += pLink->m_VehicleDataVector[LaneNo].m_LaneEmissionVector [t].HC;
						}
				 
				 }
	}

	// output speed data second by second
	if(g_OutputEmissionOperatingModeData)
	{

	FILE* st2;
	fopen_s(&st2,"output_vehicle_operating_mode.csv","w");

	if(st2!=NULL)
	{
		cout << "writing output_vehicle_operating_mode.csv..." << endl;
		std::map<int, DTAVehicle*>::iterator iterVM;
		//calculate the toll cost and emission cost
		for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
		{

			DTAVehicle* pVehicle = iterVM->second;

			//			fprintf(st, "Vehicle = %d\n", pVehicle->m_VehicleID );
			fprintf(st2, "Vehicle=,%d,Type=,%d\n", pVehicle->m_VehicleID,pVehicle->m_VehicleType);

			//			fprintf(st, "number of speed data points = %d\n", pVehicle->m_SpeedVector .size());

			fprintf(st2, "# of operating mode data points =,%d\n", pVehicle->m_OperatingModeCount.size());
			fprintf(st2, "Energy:, %f\n", pVehicle->Energy);
			fprintf(st2, "CO2:, %f\n", pVehicle->CO2 );
			fprintf(st2, "CO:, %f\n", pVehicle->CO );
			fprintf(st2, "HC:, %f\n", pVehicle->HC );
			fprintf(st2, "NOX:, %f\n", pVehicle->NOX);

			for(std::map<int, int>::iterator iterOP  =  pVehicle->m_OperatingModeCount.begin(); iterOP != pVehicle->m_OperatingModeCount.end (); iterOP++)
			{
				fprintf(st2, "op=,%d,count=,%d\n", iterOP->first ,iterOP->second );
			}

			for(std::map<int, int>::iterator iter_speed  =  pVehicle->m_SpeedCount.begin(); iter_speed != pVehicle->m_SpeedCount.end (); iter_speed++)
			{
				fprintf(st2, "speed=,%d,count=,%d\n", iter_speed->first ,iter_speed->second );
			}

			/// print out second by second profile
			if(g_OutputSecondBySecondEmissionData)
			{
				fprintf(st2, "time_in_second,FromNodeNumber,ToNodeNumber,TimeOnThisLinkInSecond,Speed,Acceleration,VSP,VSP_bin,Speed_bin,OperatingMode,Energy,CO2,NOX,CO,HC\n");

				if(pVehicle->m_VehicleID  %1000 ==0 )
				{
				cout << " outputing vehicle id " << pVehicle->m_VehicleID << "..." << endl;
				}

				for(std::map<int, VehicleSpeedProfileData>::iterator iter_s  =  pVehicle->m_SpeedProfile.begin(); iter_s != pVehicle->m_SpeedProfile.end (); iter_s++)
			{  

				VehicleSpeedProfileData element  = iter_s->second;
				fprintf(st2, "%d,%d,%d,%d,%5.2f,%5.2f,%f,%s,%s,%d,%f,%f,%f,%f,%f,\n", iter_s->first , 
					element.FromNodeNumber,
					element.ToNodeNumber,
					element.TimeOnThisLinkInSecond,
					element.Speed,
					element.Acceleration,
					element.VSP,
					element.GetVSPBinNoString () ,
					element.GetSpeedBinNoString()  ,
					element.OperationMode   ,
					element.Energy,
					element.CO2, 
					element.NOX, 
					element.CO,
					element.HC					
					);
			}
			}
				fprintf(st2,"\n");
		}

		fclose(st2);
	}

	}
}
void DTALink::ComputeVSP()  // VSP: vehicle specific power
{

	int max_time_step_in_a_cycle = 100;

//#pragma omp parallel for 

	for(int LaneNo = 0; LaneNo < m_NumLanes; LaneNo++)
	{
		// step 1: sort vehicles arrival times
		sort(m_VehicleDataVector[LaneNo].LaneData.begin(), m_VehicleDataVector[LaneNo].LaneData.end(), vehicleCF_sort_function);

		int m_NumberOfTimeSteps = m_SimulationHorizon*60*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
		float **VechileDistanceAry	= AllocateDynamicArray<float>(m_VehicleDataVector[LaneNo].LaneData.size(),max_time_step_in_a_cycle+10);
		// step 2: car following simulation
		float link_length_in_meter = m_Length * 1609.344f; //1609.344f: mile to meters

		int t;
		int v_start = 0;

		int v;
		for(v = v_start; v<m_VehicleDataVector[LaneNo].LaneData.size(); v++)  // first do loop: every vehicle
			{
				int start_time =  m_VehicleDataVector[LaneNo].LaneData[v].StartTime_in_SimulationInterval;
				int end_time =  m_VehicleDataVector[LaneNo].LaneData[v].EndTime_in_SimulationInterval;
				for(int t = start_time; t < end_time; t+=1) 
					// second do loop: start_time to end time
				{
					//calculate free-flow position
					//xiF(t) = xi(t-τ) + vf(τ)
					VechileDistanceAry[v][t%max_time_step_in_a_cycle] = 0;

					if(t>= start_time +1)
						VechileDistanceAry[v][t%max_time_step_in_a_cycle] = min(link_length_in_meter, VechileDistanceAry[v][(t-1)%max_time_step_in_a_cycle] +  m_VehicleDataVector[LaneNo].LaneData[v].FreeflowDistance_per_SimulationInterval);
					//					TRACE("veh %d, time%d,%f\n",v,t,VechileDistanceAry[v][t]);

					//calculate congested position
					if(v>=1)
					{
						//xiC(t) = xi-1(t-τ) - δ
						int time_t_minus_tau = t - m_VehicleDataVector[LaneNo].LaneData[v].TimeLag_in_SimulationInterval; // need to convert time in second to time in simulation time interval

						if(time_t_minus_tau >=0)  // the leader has not reached destination yet
						{
							// vehicle v-1: previous car
							float CongestedDistance = VechileDistanceAry[v-1][time_t_minus_tau%max_time_step_in_a_cycle]  - m_VehicleDataVector[LaneNo].LaneData[v].CriticalSpacing_in_meter ;
							// xi(t) = min(xAF(t), xAC(t))
							if (VechileDistanceAry[v][t%max_time_step_in_a_cycle]  > CongestedDistance && CongestedDistance >= VechileDistanceAry[v][(t-1)%max_time_step_in_a_cycle])
								VechileDistanceAry[v][t%max_time_step_in_a_cycle] = CongestedDistance;
						}

					}

					// output speed per second

					if(t%NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND == 0)  // per_second
					{
						// for active vehicles (with positive speed or positive distance

						float SpeedBySecond = m_SpeedLimit* 0.44704f ; // 1 mph = 0.44704 meters per second

						if(t >= m_VehicleDataVector[LaneNo].LaneData[v].StartTime_in_SimulationInterval + NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND)  // not the first second
						{
							SpeedBySecond = (VechileDistanceAry[v][t%max_time_step_in_a_cycle] - VechileDistanceAry[v][max(0,t-NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND)%max_time_step_in_a_cycle]);

//							TRACE("\nt: %d, dist: %f,%f, %f",t,VechileDistanceAry[v][t%max_time_step_in_a_cycle],VechileDistanceAry[v][(t-NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND)%max_time_step_in_a_cycle],SpeedBySecond);
						}

						// different lanes have different vehicle numbers, so we should not have openMP conflicts here
						DTAVehicle* pVehicle  = g_VehicleMap[m_VehicleDataVector[LaneNo].LaneData[v].VehicleID];
						float accelation = SpeedBySecond - pVehicle->m_PrevSpeed ;

						float vsp = 0;
						int OperatingMode = ComputeOperatingModeFromSpeed(vsp,SpeedBySecond, accelation);

						pVehicle->m_OperatingModeCount[OperatingMode]+=1;
						int integer_speed = SpeedBySecond /0.44704f;  // convert meter per second to mile per hour
						pVehicle->m_SpeedCount [integer_speed]+=1;


						pVehicle->m_PrevSpeed  = SpeedBySecond;

						int vehicle_type = pVehicle->m_VehicleType ;

						int time_in_min = t/NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND/60;

						if(EmissionRateData[vehicle_type][OperatingMode].bInitialized == false)
						{
						cout << "Emission rate data are not available for vehicle type = " <<  vehicle_type << " and  operating mode = " << OperatingMode << endl;

						
						g_ProgramStop();
					
						}

						float Energy = EmissionRateData[vehicle_type][OperatingMode].meanBaseRate_TotalEnergy/3600;
						float CO2 = EmissionRateData[vehicle_type][OperatingMode].meanBaseRate_CO2/3600;
						float NOX = EmissionRateData[vehicle_type][OperatingMode].meanBaseRate_NOX/3600;
						float CO = EmissionRateData[vehicle_type][OperatingMode].meanBaseRate_CO/3600;
						float HC = EmissionRateData[vehicle_type][OperatingMode].meanBaseRate_HC/3600;

						m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].Energy+= Energy;
						m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].CO2+= CO2;
						m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].NOX+= NOX;
						m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].CO+= CO;
						m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].HC+= HC;

						if(pVehicle->m_VehicleID  ==  g_TargetVehicleID_OutputSecondBySecondEmissionData || 
							(g_OutputSecondBySecondEmissionData && pVehicle->m_DepartureTime >= g_start_departure_time_in_min_for_output_second_by_second_emission_data && 
							pVehicle->m_DepartureTime <= g_end_departure_time_in_min_for_output_second_by_second_emission_data))  // record data
						{

						VehicleSpeedProfileData element;
						element.FromNodeNumber = this->m_FromNodeNumber ;
						element.ToNodeNumber  = this->m_ToNodeNumber ;
						element.TimeOnThisLinkInSecond = (t-start_time)/NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
						element.Speed = SpeedBySecond /0.44704f;
						element.Acceleration = accelation /0.44704f;
						element.OperationMode = OperatingMode;
						element.VSP  = vsp;
						element.SpeedBinNo = GetSpeedBinNo(element.Speed);
						element.VSPBinNo =  GetVSPBinNo(vsp, element.Speed);

						element.Energy = Energy;
						element.CO2 = CO2;
						element.NOX = NOX;
						element.CO = CO;
						element.HC = HC;


						pVehicle->m_SpeedProfile[t] = element;

						}



					}

				}  // for each time step

			} // for each vehicle

		if(VechileDistanceAry!=NULL)
		{
			DeallocateDynamicArray<float>(VechileDistanceAry,m_VehicleDataVector[LaneNo].LaneData .size(), max_time_step_in_a_cycle+10);
		}

	}

}
