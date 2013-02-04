//  Portions Copyright 2010 Xuesong Zhou, Hao Lei

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
#define _MAXIMUM_AGE_SIZE 31

int OperatingModeMap[MAX_SPEED_BIN][MAX_VSP_BIN] = {-1};

class CEmissionRate 
{
public:
	bool bInitialized;
	float meanBaseRate_TotalEnergy;
	float meanBaseRate_CO2;
	float meanBaseRate_NOX;
	float meanBaseRate_CO;
	float meanBaseRate_HC;
	int Age;

	CEmissionRate()
	{
		bInitialized = false;
		meanBaseRate_TotalEnergy = meanBaseRate_CO2 = meanBaseRate_NOX = meanBaseRate_CO = meanBaseRate_HC = 0;
		Age = 0;
	}
};

CEmissionRate EmissionRateData[MAX_VEHICLE_TYPE_SIZE][_MAXIMUM_OPERATING_MODE_SIZE][_MAXIMUM_AGE_SIZE];

class CCycleAverageEmissionFactor
{
public:
	float emissionFactor_CO2;
	float emissionFactor_NOX;
	float emissionFactor_CO;
	float emissionFactor_HC;
	float average_cycle_speed;

	CCycleAverageEmissionFactor()
	{
		this->emissionFactor_CO2 = this->emissionFactor_NOX = this->emissionFactor_CO = this->emissionFactor_HC = 0.0f;
	}
	CCycleAverageEmissionFactor(float emissionFactor_CO2, float emissionFactor_NOX, float emissionFactor_CO, float emissionFactor_HC, float average_cycle_speed)
	{
		this->emissionFactor_CO2 = emissionFactor_CO2;
		this->emissionFactor_NOX = emissionFactor_NOX;
		this->emissionFactor_CO = emissionFactor_CO;
		this->emissionFactor_HC = emissionFactor_HC;
		this->average_cycle_speed = average_cycle_speed;
	}
};

map<float,CCycleAverageEmissionFactor> CycleAverageEmissionFactorMap[MAX_VEHICLE_TYPE_SIZE][_MAXIMUM_AGE_SIZE];

class CVehicleEmissionResult
{
public:
	float Energy;
	float CO2;
	float NOX;
	float CO;
	float HC;

	static void CalculateEmissions(int vehicle_type, std::map<int, int>& OperatingModeCount, int age, CVehicleEmissionResult& emissionResult)
	{
		emissionResult.Energy = 0;
		emissionResult.CO2 = 0;
		emissionResult.NOX = 0;
		emissionResult.CO = 0;
		emissionResult.HC = 0;

		for(std::map<int, int>::iterator iterOP  = OperatingModeCount.begin(); iterOP != OperatingModeCount.end (); iterOP++)
		{
			int OpModeID = iterOP->first; int count = iterOP->second;

			emissionResult.Energy+= EmissionRateData[vehicle_type][OpModeID][age].meanBaseRate_TotalEnergy*count/3600;
			emissionResult.CO2+= EmissionRateData[vehicle_type][OpModeID][age].meanBaseRate_CO2*count/3600;
			emissionResult.NOX+= EmissionRateData[vehicle_type][OpModeID][age].meanBaseRate_NOX*count/3600;
			emissionResult.CO+= EmissionRateData[vehicle_type][OpModeID][age].meanBaseRate_CO*count/3600;
			emissionResult.HC+= EmissionRateData[vehicle_type][OpModeID][age].meanBaseRate_HC*count/3600;
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
	//if(speed_mph < 50)
	//{
	if (vsp < 0) return VSP_LT0;

	if (vsp < 3) return VSP_0_3;

	if (vsp < 6) return VSP_3_6;

	if (vsp < 9) return VSP_6_9;

	if (vsp < 12) return VSP_9_12;

	if (vsp < 18) return VSP_12_18;

	if (vsp < 24) return VSP_18_24;

	if (vsp < 30) return VSP_24_30;

	return VSP_GT30;

	//}
	//else  // greate than 50
	//{
	//	if(vsp < 6) 
	//		return VSP_LT6;
	//	if(vsp < 12) 
	//		return VSP_6_12;
	//	if (vsp < 18)
	//		return VSP_12_18;
	//	if (vsp < 24)
	//		return VSP_18_24;
	//	if (vsp < 30)
	//		return VSP_24_30;
	//	else
	//		return VSP_GT30;
	//}
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
			if(parser_emission.GetValueByFieldName("OpModeID",opModeID) == false)
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


			if(parser_emission.GetValueByFieldName("age",element.Age) == false)
			{
				break;
			}


			EmissionRateData[vehicle_type][opModeID][element.Age] = element;
			EmissionRateData[vehicle_type][opModeID][element.Age].bInitialized = true;
		}
	}
	else
	{
		cout << "Error: File input_vehicle_emission_rate.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();

	}

	cout << "Reading file input_vehicle_emission_rate.csv..."<< endl;
}


void ReadInputCycleAverageEmissionFactors()
{
	CCSVParser parser_emission_factor;
	if (parser_emission_factor.OpenCSVFile("input_cycle_emission_factor.csv"))
	{
		int vehicle_type;
		float emissionFactor_CO2;
		float emissionFactor_NOX;
		float emissionFactor_CO;
		float emissionFactor_HC;
		float cycle_average_speed;
		int age;

		while(parser_emission_factor.ReadRecord())
		{
			if (parser_emission_factor.GetValueByFieldName("vehicle_type",vehicle_type) == false
				|| parser_emission_factor.GetValueByFieldName("CO2_emission_factor_(g/mi)",emissionFactor_CO2) == false
				|| parser_emission_factor.GetValueByFieldName("NOX_emission_factor_(g/mi)",emissionFactor_NOX) == false
				|| parser_emission_factor.GetValueByFieldName("CO_emission_factor_(g/mi)",emissionFactor_CO) == false
				|| parser_emission_factor.GetValueByFieldName("HC_emission_factor_(g/mi)",emissionFactor_HC) == false
				|| parser_emission_factor.GetValueByFieldName("cycle_average_speed_(mph)",cycle_average_speed) == false
				|| parser_emission_factor.GetValueByFieldName("age",age) == false
				)
			{
				break;
			}
			else
			{
				CycleAverageEmissionFactorMap[vehicle_type][age][cycle_average_speed] = CCycleAverageEmissionFactor(emissionFactor_CO2, emissionFactor_NOX, emissionFactor_CO, emissionFactor_HC, cycle_average_speed);
			}
		}
	}
	else
	{
		cout << "Error: File input_cycle_emission_factor.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}

	cout << "Reading file input_cycle_emission_factor.csv..."<< endl;
}

enum Pollutants {EMISSION_CO2,EMISSION_NOX,EMISSION_CO,EMISSION_HC,MAX_POLLUTANT};

float BaseCycleFractionOfOperatingMode[MAX_VEHICLE_TYPE_SIZE][41] = {0.0f};
float BaseCycleEmissionRate[MAX_VEHICLE_TYPE_SIZE][_MAXIMUM_AGE_SIZE][MAX_POLLUTANT] = {0.0f};

void ReadFractionOfOperatingModeForBaseCycle()
{
	CCSVParser parser_emission_factor;
	if (parser_emission_factor.OpenCSVFile("input_base_cycle_fraction_of_OpMode.csv"))
	{
		int vehicle_type;
		float value;

		while(parser_emission_factor.ReadRecord())
		{			
			if (parser_emission_factor.GetValueByFieldName("vehicle_type",vehicle_type) == false)
			{
				break;
			}
			else
			{
				if (vehicle_type >= MAX_VEHICLE_TYPE_SIZE)
				{
					cout << "Warning: unknown vehicle_type " << vehicle_type << " !\n";
					continue;
				}

				if (parser_emission_factor.GetValueByFieldName("0", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][0] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("1", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][1] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("11", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][11] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("12", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][12] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("13", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][13] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("14", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][14] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("15", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][15] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("16", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][16] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("21", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][21] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("22", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][22] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("23", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][23] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("24", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][24] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("25", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][25] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("27", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][27] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("28", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][28] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("29", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][29] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("30", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][30] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("33", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][33] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("35", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][35] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("37", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][37] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("38", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][38] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("39", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][39] = value;
				}

				if (parser_emission_factor.GetValueByFieldName("40", value))
				{
					BaseCycleFractionOfOperatingMode[vehicle_type][40] = value;
				}
			}
		}
	}
	else
	{
		cout << "Error: File input_base_cycle_fraction_of_OpMode.csv cannot be opened.\n It might be currently used and locked by EXCEL."<< endl;
		g_ProgramStop();
	}

	for (int vType = 0; vType < MAX_VEHICLE_TYPE_SIZE; vType++)
	{
		for (int vAge = 0; vAge < _MAXIMUM_AGE_SIZE; vAge++)
		{
			for (int opMode = 0; opMode < 41;opMode++)
			{
				if (EmissionRateData[vType][opMode][vAge].bInitialized)
				{
					BaseCycleEmissionRate[vType][vAge][EMISSION_CO2] += BaseCycleFractionOfOperatingMode[vType][opMode] * EmissionRateData[vType][opMode][vAge].meanBaseRate_CO2;
					BaseCycleEmissionRate[vType][vAge][EMISSION_NOX] += BaseCycleFractionOfOperatingMode[vType][opMode] * EmissionRateData[vType][opMode][vAge].meanBaseRate_NOX;
					BaseCycleEmissionRate[vType][vAge][EMISSION_CO] += BaseCycleFractionOfOperatingMode[vType][opMode] * EmissionRateData[vType][opMode][vAge].meanBaseRate_CO;
					BaseCycleEmissionRate[vType][vAge][EMISSION_HC] += BaseCycleFractionOfOperatingMode[vType][opMode] * EmissionRateData[vType][opMode][vAge].meanBaseRate_HC;
				}
			}
		}
	}
	cout << "Reading file input_cycle_emission_factor.csv..."<< endl;
}

CCycleAverageEmissionFactor* BaseCaseEmissionFactors(int vehicle_type, int vehicle_age, float average_speed)
{
	if ((vehicle_type >= 0 && vehicle_type < MAX_VEHICLE_TYPE_SIZE)
		&& (vehicle_age >= 0 && vehicle_age < _MAXIMUM_AGE_SIZE))
	{
		map<float,CCycleAverageEmissionFactor>& cycleAverageEmissionFactorMap = CycleAverageEmissionFactorMap[vehicle_type][vehicle_age];
		map<float,CCycleAverageEmissionFactor>::iterator iter, targetIterator;		
		targetIterator = iter = cycleAverageEmissionFactorMap.begin();
		float abs_speed_diff = abs(iter->first - average_speed);

		for (;iter != cycleAverageEmissionFactorMap.end(); iter++)
		{
			if (abs_speed_diff < abs(iter->first - average_speed))
			{
				break;
			}
			else
			{
				abs_speed_diff = abs(iter->first - average_speed);
				targetIterator = iter;
			}
		}

		return &(targetIterator->second);
	}
	else
	{
		return NULL;
	}
}

void SetupOperatingModeVector()
{
	OperatingModeMap[VSP_0_25mph][VSP_LT0] = 11;
	OperatingModeMap[VSP_0_25mph][VSP_0_3] = 12;
	OperatingModeMap[VSP_0_25mph][VSP_3_6] = 13;
	OperatingModeMap[VSP_0_25mph][VSP_6_9] = 14;
	OperatingModeMap[VSP_0_25mph][VSP_9_12] = 15;
	OperatingModeMap[VSP_0_25mph][VSP_12_18] = 16;
	OperatingModeMap[VSP_0_25mph][VSP_18_24] = 16;
	OperatingModeMap[VSP_0_25mph][VSP_24_30] = 16;
	OperatingModeMap[VSP_0_25mph][VSP_GT30] = 16;

	OperatingModeMap[VSP_25_50mph][VSP_LT0] = 21;
	OperatingModeMap[VSP_25_50mph][VSP_0_3] = 22;
	OperatingModeMap[VSP_25_50mph][VSP_3_6] = 23;
	OperatingModeMap[VSP_25_50mph][VSP_6_9] = 24;
	OperatingModeMap[VSP_25_50mph][VSP_9_12] = 25;
	OperatingModeMap[VSP_25_50mph][VSP_12_18] = 27;
	OperatingModeMap[VSP_25_50mph][VSP_18_24] = 28;
	OperatingModeMap[VSP_25_50mph][VSP_24_30] = 29;
	OperatingModeMap[VSP_25_50mph][VSP_GT30] = 30;

	OperatingModeMap[VSP_GT50mph][VSP_LT0] = 33;
	OperatingModeMap[VSP_GT50mph][VSP_0_3] = 33;
	OperatingModeMap[VSP_GT50mph][VSP_3_6] = 33;
	OperatingModeMap[VSP_GT50mph][VSP_6_9] = 35;
	OperatingModeMap[VSP_GT50mph][VSP_9_12] = 35;
	OperatingModeMap[VSP_GT50mph][VSP_12_18] = 37;
	OperatingModeMap[VSP_GT50mph][VSP_18_24] = 38;
	OperatingModeMap[VSP_GT50mph][VSP_24_30] = 39;
	OperatingModeMap[VSP_GT50mph][VSP_GT30] = 40;
}

int GetOperatingMode(float vsp, float s_mph)
{
	SPEED_BIN SpeedBinNo = GetSpeedBinNo(s_mph);
	VSP_BIN VSPBinNo =  GetVSPBinNo(vsp, s_mph);

	int OPBin =  OperatingModeMap[SpeedBinNo][VSPBinNo];
	return OPBin;
}

int ComputeOperatingModeFromSpeed(float &vsp, float v /*meter per second*/,float a, float grade = 0, int vehicle_type =1)
{
	int vehicle_type_no = vehicle_type -1; // start from 0
	double TermA = g_VehicleTypeVector[vehicle_type_no].rollingTermA ;
	double TermB = g_VehicleTypeVector[vehicle_type_no].rotatingTermB ;
	double TermC = g_VehicleTypeVector[vehicle_type_no].dragTermC  ;
	double Mass =  g_VehicleTypeVector[vehicle_type_no].sourceMass;
	vsp = (TermA*v + TermB*v*v +  TermC*v*v*v + v*a*Mass)/Mass;
	//vsp = TermA/Mass*v + TermB/Mass*v*v+  TermC/Mass*v*v*v;
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
	if(g_TrafficFlowModelFlag ==  tfm_BPR)
		return; // no emission output for BPR function

	ReadInputEmissionRateFile();
	ReadInputCycleAverageEmissionFactors();
	ReadFractionOfOperatingModeForBaseCycle();
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

	int totalSamples = (int)(g_VehicleMap.size() * g_OutputSecondBySecondEmissionDataPercentage);
	int sampling_interval = (int)(g_VehicleMap.size() / totalSamples);
	int vehicleCounts = 0;

	std::map<int, DTAVehicle*>::iterator iterVM;
	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{
		DTAVehicle* pVehicle = iterVM->second;
		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_PricingType != 4)
		{
			for(int i=0; i< pVehicle->m_NodeSize-1; i++)
			{
				if(pVehicle->m_bComplete )  // for vehicles finish the trips
				{
					VehicleCFData element;
					element.VehicleID = pVehicle->m_VehicleID ;
					int LinkNo = pVehicle->m_NodeAry[i].LinkNo ;
					element.SequentialLinkNo = i;

					element.FreeflowDistance_per_SimulationInterval  = g_LinkVector[LinkNo]->m_SpeedLimit* 1609.344f/3600/NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND; 
					//1609.344f: mile to meters ; 3600: # of seconds per hour
					//τ = 1/(wkj)
					// δ = 1/kj
					element.TimeLag_in_SimulationInterval = (int)(3600*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND/(g_LinkVector[LinkNo]->m_BackwardWaveSpeed * g_LinkVector[LinkNo]->m_KJam )+0.5);
					element.CriticalSpacing_in_meter = 1609.344f / g_LinkVector[LinkNo]->m_KJam ; 

					if(i == 0)
					{
						element.StartTime_in_SimulationInterval = pVehicle->m_DepartureTime*60*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
					}
					else
					{
						element.StartTime_in_SimulationInterval = pVehicle->m_NodeAry [i-1].AbsArrivalTimeOnDSN*60*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
					}

					element.EndTime_in_SimulationInterval = pVehicle->m_NodeAry [i].AbsArrivalTimeOnDSN*60*NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
					element.LaneNo = element.VehicleID%g_LinkVector[LinkNo]->m_NumLanes;

					g_LinkVector[LinkNo]->m_VehicleDataVector[element.LaneNo].LaneData .push_back(element);
				}
			}

			if (pVehicle->m_bComplete)
			{
				vehicleCounts++;
				if (vehicleCounts % sampling_interval == 0)
				{
					pVehicle->m_bDetailedEmissionOutput = true;
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

	CVehicleEmissionResult emissionResult;

	for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
	{
		DTAVehicle* pVehicle = iterVM->second;
		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_OperatingModeCount.size() > 0)  // with physical path in the network
		{
			CVehicleEmissionResult::CalculateEmissions(pVehicle->m_VehicleType, pVehicle->m_OperatingModeCount, pVehicle->m_Age, emissionResult);

			pVehicle->m_Emissions = emissionResult.CO2;
			pVehicle->Energy = emissionResult.Energy ;
			pVehicle->CO2 = emissionResult.CO2 ;
			pVehicle->NOX = emissionResult.NOX ;
			pVehicle->CO = emissionResult.CO ;
			pVehicle->HC = emissionResult.HC ;

			Energy += emissionResult.Energy;
			CO2 += emissionResult.CO2;
			NOX += emissionResult.NOX;
			CO += emissionResult.CO;
			HC += emissionResult.HC;
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

		FILE* fpSampledEmissionsFile;
		FILE* fpSampledCycleEmissionRateFile;

		int numOfSamples = 0;
		//int numOfVehicles = 0;

		if (g_OutputSecondBySecondEmissionData)
		{
			fopen_s(&fpSampledEmissionsFile, "output_sampled_vehicle_operating_mode.csv","w");
			fopen_s(&fpSampledCycleEmissionRateFile, "output_sampled_vehicle_cycle_emission_rate.csv","w");
			if (fpSampledEmissionsFile)
			{
				fprintf(fpSampledEmissionsFile, "Vehicle_id,Vehicle_Type,Vehicle_Age,Hour,Min,FromNodeNumber,ToNodeNumber,LinkType,TimeOnThisLinkInSecond,TripDurationInSecond,Speed(MPH),Acceleration(FEET/S^2),VSP,VSP_bin,Speed_bin,OperatingMode,Energy,CO2,NOX,CO,HC,Average_Speed(MPH),CO2_Rate(g/mi),NOX_Rate(g/mi),CO_Rate((g/mi),HC_Rate(g/mi)\n");
			}

			if (fpSampledCycleEmissionRateFile)
			{
				fprintf(fpSampledCycleEmissionRateFile, "Vehicle_id,Vehicle_Type,Vehicle_Age,Average_Speed(MPH),CO2_Rate(g/mi),NOX_Rate(g/mi),CO_Rate((g/mi),HC_Rate(g/mi)\n");
			}
		}

		if(st2 != NULL)
		{
			cout << "writing output_sampled_vehicle_operating_mode.csv..." << endl;
			std::map<int, DTAVehicle*>::iterator iterVM;
			//calculate the toll cost and emission cost
			for (iterVM = g_VehicleMap.begin(); iterVM != g_VehicleMap.end(); iterVM++)
			{
				DTAVehicle* pVehicle = iterVM->second;

				fprintf(st2, "Vehicle=,%d,Type=,%d\n", pVehicle->m_VehicleID,pVehicle->m_VehicleType);
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
					if (pVehicle->m_bDetailedEmissionOutput == false)
					{
						continue;
					}

					numOfSamples++;

					if(numOfSamples % 500 == 0)
					{
						cout << " " << numOfSamples << " emission samples have been generated ..." << endl;
					}

					int VehicleSimulationStartTime;
					int prevFromNodeNumber = -1;
					int prevToNodeNumber = -1;
					int prevLinkType = -1;
					int currLinkType = -1;
					bool isNewLink = false;

					int speedProfileSize = pVehicle->m_SpeedProfile.size();
					int speedProfileCounter = 0;
					float prev_speed = 0.0f;

					float prev_acceleration = -99999.0f; //in meter/sec^2
					float prev_prev_acceleration = -99999.0f; //in meter/sec^2

					float totalCO2 = 0.0f;
					float totalNOX = 0.0f;
					float totalCO = 0.0f;
					float totalHC = 0.0f;

					for(std::map<int, VehicleSpeedProfileData>::iterator iter_s  =  pVehicle->m_SpeedProfile.begin();
						iter_s != pVehicle->m_SpeedProfile.end();
						iter_s++, speedProfileCounter++)
					{  
						VehicleSpeedProfileData element  = iter_s->second;

						if (fpSampledEmissionsFile)
						{
							//Get the link type by from and to node numbers
							if (prevFromNodeNumber != element.FromNodeNumber || prevToNodeNumber != element.ToNodeNumber)
							{
								if(g_LinkMap.find(GetLinkStringID(element.FromNodeNumber,element.ToNodeNumber)) != g_LinkMap.end())
								{
									DTALink* pLink = g_LinkMap[GetLinkStringID(element.FromNodeNumber,element.ToNodeNumber)];
									currLinkType = prevLinkType = pLink->m_link_type;
								}
								else
								{
									currLinkType = prevLinkType = -1;
								}
								isNewLink = true;

								prevFromNodeNumber = element.FromNodeNumber;
								prevToNodeNumber = element.ToNodeNumber;
							}
							else
							{
								isNewLink = false;								
							}

							// If it is the first second on the first link or the last second on the last link, recalculate the acceleration rate, VSP and emissions
							if (iter_s == pVehicle->m_SpeedProfile.begin() || speedProfileCounter + 1 == speedProfileSize) 
							{
								isNewLink = false;

								VehicleSimulationStartTime = pVehicle->m_SpeedProfile.begin()->first;

								float acceleration;
								if (iter_s == pVehicle->m_SpeedProfile.begin())
								{
									acceleration = iter_s->second.Speed * 0.44704f; //mph to meter/sec^2
								}

								if (speedProfileCounter + 1 == speedProfileSize)
								{
									acceleration = -iter_s->second.Speed * 0.44704f;
									element.Speed = iter_s->second.Speed = 0.0f;
								}

								float vsp = 0;
								int OperatingMode = ComputeOperatingModeFromSpeed(vsp,iter_s->second.Speed * 0.44704f, acceleration,0,pVehicle->m_VehicleType);

								if (acceleration <= -2)
								{
									OperatingMode = 0;
								}
								else
								{
									if (prev_prev_acceleration < -1 && prev_acceleration < -1 && acceleration < -1)
									{
										if (prev_prev_acceleration != -99999.0f || prev_acceleration != -99999.0f)
										{
											OperatingMode = 0;
										}
									}
								}

								if (iter_s->second.Speed >= -1 && iter_s->second.Speed < 1)
								{
									OperatingMode = 1;
								}

								pVehicle->m_OperatingModeCount[element.OperationMode]--;
								pVehicle->m_OperatingModeCount[OperatingMode]++;							

								int vehicle_type = pVehicle->m_VehicleType;
								int age = pVehicle->m_Age;
								float Energy = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_TotalEnergy/3600;
								float CO2 = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_CO2/3600;
								float NOX = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_NOX/3600;
								float CO = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_CO/3600;
								float HC = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_HC/3600;

								element.Acceleration = acceleration * 3.28084f; //meter/sec^2 to feet/sec^2
								element.VSP = vsp;
								element.OperationMode = OperatingMode;
								element.VSPBinNo = GetVSPBinNo(vsp, element.Speed);
								element.SpeedBinNo = GetSpeedBinNo(element.Speed);
								element.Energy = Energy;
								element.CO2 = CO2;
								element.NOX = NOX;
								element.CO = CO;
								element.HC = HC;
							}

							// First second on the new Link, recalculate the acceleration rate, VSP and emissions
							if (isNewLink)
							{
								std::map<int, VehicleSpeedProfileData>::iterator prev_iter_s = --iter_s;
								iter_s++;
								float acceleration = (iter_s->second.Speed - prev_speed) * 0.44704f;
								float vsp = 0;
								int OperatingMode = ComputeOperatingModeFromSpeed(vsp,iter_s->second.Speed * 0.44704f, acceleration,0,pVehicle->m_VehicleType);
								if (acceleration <= -2)
								{
									OperatingMode = 0;
								}
								else
								{
									if (prev_prev_acceleration < -1 && prev_acceleration < -1 && acceleration < -1)
									{
										if (prev_prev_acceleration != -99999.0f || prev_acceleration != -99999.0f)
										{
											OperatingMode = 0;
										}
									}
								}

								if (iter_s->second.Speed >= -1 && iter_s->second.Speed < 1)
								{
									OperatingMode = 1;
								}

								pVehicle->m_OperatingModeCount[element.OperationMode]--;
								pVehicle->m_OperatingModeCount[OperatingMode]++;							

								int vehicle_type = pVehicle->m_VehicleType;
								int age = pVehicle->m_Age;
								float Energy = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_TotalEnergy/3600;
								float CO2 = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_CO2/3600;
								float NOX = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_NOX/3600;
								float CO = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_CO/3600;
								float HC = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_HC/3600;

								element.Acceleration = acceleration * 3.28084f; // meter/sec^2 to feet/sec^2
								element.VSP = vsp;
								element.OperationMode = OperatingMode;
								element.VSPBinNo = GetVSPBinNo(vsp, element.Speed);
								element.SpeedBinNo = GetSpeedBinNo(element.Speed);
								element.Energy = Energy;
								element.CO2 = CO2;
								element.NOX = NOX;
								element.CO = CO;
								element.HC = HC;
							}

							fprintf(fpSampledEmissionsFile, "%d,%d,%d,%d,%d,%d,%d,%s,%d,%d,%5.2f",
								pVehicle->m_VehicleID,
								pVehicle->m_VehicleType,
								pVehicle->m_Age,
								(int)(iter_s->first / 3600 / NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND),
								((int)(iter_s->first / 60 / NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND)) % 60,								
								element.FromNodeNumber,
								element.ToNodeNumber,
								(currLinkType == -1) ? "Unknown":g_LinkTypeMap[currLinkType].link_type_name.c_str(),
								element.TimeOnThisLinkInSecond,
								(int)((iter_s->first - VehicleSimulationStartTime) / NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND),
								element.Speed
								);

							if (iter_s != pVehicle->m_SpeedProfile.begin())
							{
								totalCO2 += element.CO2;
								totalNOX += element.NOX;
								totalCO += element.CO;
								totalHC += element.HC;

								fprintf(fpSampledEmissionsFile,",%5.2f,%f,%s,%s,%d,%f,%f,%f,%f,%f",
									element.Acceleration,
									element.VSP,
									element.GetVSPBinNoString(),
									element.GetSpeedBinNoString(),
									element.OperationMode,
									element.Energy,
									element.CO2, 
									element.NOX, 
									element.CO,
									element.HC
									);
							}

							if (speedProfileCounter + 1 == speedProfileSize)
							{
								fprintf(fpSampledCycleEmissionRateFile, "%d,%d,%d", 
									pVehicle->m_VehicleID,
									pVehicle->m_VehicleType,
									pVehicle->m_Age
									);

								float average_speed = pVehicle->m_Distance / (pVehicle->m_ArrivalTime - pVehicle->m_DepartureTime) * 60.0f;
								float base_average_speed;
								//Average speed for base cycle is 21.2 mph
								CCycleAverageEmissionFactor* pBaseCaseEmissionsFactors = BaseCaseEmissionFactors(pVehicle->m_VehicleType, pVehicle->m_Age,21.2f);
								if (pBaseCaseEmissionsFactors)
								{
									float speed_ratio = 21.2 / average_speed;									

									float correctedCO2 = pBaseCaseEmissionsFactors->emissionFactor_CO2 * (totalCO2 / speedProfileSize * 3600) / BaseCycleEmissionRate[pVehicle->m_VehicleType][pVehicle->m_Age][EMISSION_CO2] * speed_ratio;
									float correctedNOX = pBaseCaseEmissionsFactors->emissionFactor_NOX * (totalNOX / speedProfileSize * 3600) / BaseCycleEmissionRate[pVehicle->m_VehicleType][pVehicle->m_Age][EMISSION_NOX] * speed_ratio;
									float correctedCO = pBaseCaseEmissionsFactors->emissionFactor_CO * (totalCO / speedProfileSize * 3600) / BaseCycleEmissionRate[pVehicle->m_VehicleType][pVehicle->m_Age][EMISSION_CO] * speed_ratio;
									float correctedHC = pBaseCaseEmissionsFactors->emissionFactor_HC * (totalHC / speedProfileSize * 3600) / BaseCycleEmissionRate[pVehicle->m_VehicleType][pVehicle->m_Age][EMISSION_HC] * speed_ratio;

									fprintf(fpSampledEmissionsFile,",%f,%f,%f,%f,%f", average_speed, correctedCO2, correctedNOX, correctedCO, correctedHC);
									fprintf(fpSampledCycleEmissionRateFile,",%f,%f,%f,%f,%f", average_speed, correctedCO2, correctedNOX, correctedCO, correctedHC);
								}
								else
								{
									fprintf(fpSampledEmissionsFile,",%f,%f,%f,%f,%f", average_speed, totalCO2 / pVehicle->m_Distance, totalNOX / pVehicle->m_Distance, totalCO / pVehicle->m_Distance, totalHC / pVehicle->m_Distance);
									fprintf(fpSampledCycleEmissionRateFile,",%f,%f,%f,%f,%f", average_speed, totalCO2 / pVehicle->m_Distance, totalNOX / pVehicle->m_Distance, totalCO / pVehicle->m_Distance, totalHC / pVehicle->m_Distance);
								}

								fprintf(fpSampledCycleEmissionRateFile,"\n");
							}
							fprintf(fpSampledEmissionsFile,"\n");

							prev_speed = element.Speed;
							prev_prev_acceleration = prev_acceleration;
							prev_acceleration = element.Acceleration / 3.28084f;
						}
					}
				}
				fprintf(st2,"\n");
			}

			if (fpSampledEmissionsFile)
			{
				fclose(fpSampledEmissionsFile);
			}

			if (fpSampledCycleEmissionRateFile)
			{
				fclose(fpSampledCycleEmissionRateFile);
			}

			fclose(st2);

			cout << numOfSamples << " vehicles are sampled to output_sampled_vehicle_operating_mode.csv." << endl;
		}

	}
}
void DTALink::ComputeVSP()  // VSP: vehicle specific power
{
	const int Max_Time_Step_In_A_Cycle = 100;
	const int Max_Simulation_Steps = 60 * 60 * NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
	float *LeaderPositions = new float[Max_Simulation_Steps];
	float *FollowerPositions = new float[Max_Simulation_Steps];

	//#pragma omp parallel for 
	for(int LaneNo = 0; LaneNo < m_NumLanes; LaneNo++)
	{
		// step 1: sort vehicles arrival times
		sort(m_VehicleDataVector[LaneNo].LaneData.begin(), m_VehicleDataVector[LaneNo].LaneData.end(), vehicleCF_sort_function);

		//int m_NumberOfTimeSteps = m_SimulationHorizon * 60 * NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
		//float **VechileDistanceAry	= AllocateDynamicArray<float>(m_VehicleDataVector[LaneNo].LaneData.size(),Max_Time_Step_In_A_Cycle+10);

		// step 2: car following simulation
		float link_length_in_meter = m_Length * 1609.344f; //1609.344f: mile to meters

		for(int v = 0; v < m_VehicleDataVector[LaneNo].LaneData.size(); v++)  // first do loop: every vehicle
		{
			int start_time =  m_VehicleDataVector[LaneNo].LaneData[v].StartTime_in_SimulationInterval;
			int end_time =  m_VehicleDataVector[LaneNo].LaneData[v].EndTime_in_SimulationInterval;
			float prev_acceleration = -99999.0f; //acceleration at t-1
			float prev_prev_acceleration = -99999.0f; //acceleration at t-2

			// second do loop: start_time to end time
			for(int t = start_time; t < end_time; t+=1) 				
			{
				//calculate free-flow position
				//xiF(t) = xi(t-τ) + vf(τ)
				//VechileDistanceAry[v][t%Max_Time_Step_In_A_Cycle] = 0;
				FollowerPositions[t % Max_Simulation_Steps] = 0.0f;

				if(t >= start_time + 1)
				{
					//VechileDistanceAry[v][t%Max_Time_Step_In_A_Cycle] = min(link_length_in_meter, VechileDistanceAry[v][(t-1)%Max_Time_Step_In_A_Cycle] +  m_VehicleDataVector[LaneNo].LaneData[v].FreeflowDistance_per_SimulationInterval);
					FollowerPositions[t % Max_Simulation_Steps] = min(link_length_in_meter, FollowerPositions[(t-1) % Max_Simulation_Steps] +  m_VehicleDataVector[LaneNo].LaneData[v].FreeflowDistance_per_SimulationInterval);
				}
				//					TRACE("veh %d, time%d,%f\n",v,t,VechileDistanceAry[v][t]);

				//calculate congested position if it is not the first vehicle
				if(v >= 1)
				{
					//xiC(t) = xi-1(t-τ) - δ
					int time_t_minus_tau = t - m_VehicleDataVector[LaneNo].LaneData[v].TimeLag_in_SimulationInterval; // need to convert time in second to time in simulation time interval

					if(time_t_minus_tau >= 0)  // the leader has not reached destination yet
					{
						// vehicle v-1: previous car
						//float CongestedDistance = VechileDistanceAry[v-1][time_t_minus_tau%Max_Time_Step_In_A_Cycle] - m_VehicleDataVector[LaneNo].LaneData[v].CriticalSpacing_in_meter;
						float CongestedDistance = LeaderPositions[time_t_minus_tau % Max_Simulation_Steps] - m_VehicleDataVector[LaneNo].LaneData[v].CriticalSpacing_in_meter;
						// xi(t) = min(xAF(t), xAC(t))
						if (FollowerPositions[t % Max_Simulation_Steps] > CongestedDistance && CongestedDistance >= FollowerPositions[(t-1) % Max_Simulation_Steps])
						{
							FollowerPositions[t % Max_Simulation_Steps] = CongestedDistance;
						}
					}
				}

				// output speed per second
				if(t % NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND == 0)  // per_second
				{
					// for active vehicles (with positive speed or positive distance
					float SpeedBySecond = m_SpeedLimit * 0.44704f; // 1 mph = 0.44704 meters per second

					if(t >= m_VehicleDataVector[LaneNo].LaneData[v].StartTime_in_SimulationInterval + NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND)  // not the first second
					{
						//SpeedBySecond = (VechileDistanceAry[v][t%Max_Time_Step_In_A_Cycle] - VechileDistanceAry[v][max(0,t-NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND)%Max_Time_Step_In_A_Cycle]);
						SpeedBySecond = (FollowerPositions[t % Max_Simulation_Steps] - FollowerPositions[max(0,t - NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND) % Max_Simulation_Steps]);

						//if (this->m_FromNodeID == 3 && this->m_ToNodeID == 4 && m_VehicleDataVector[LaneNo].LaneData[v].VehicleID >= 3000 && m_VehicleDataVector[LaneNo].LaneData[v].VehicleID <= 3500)
						//{
						//	TRACE("\nv_id: %d, LaneNO = %d, t: %d, dist: %f,%f,%f",
						//		m_VehicleDataVector[LaneNo].LaneData[v].VehicleID, LaneNo, t,
						//		FollowerPositions[t % Max_Simulation_Steps],
						//		FollowerPositions[(t - NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND) % Max_Simulation_Steps],
						//		SpeedBySecond);
						//}
					}

					if (t + NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND >= end_time)
					{
						SpeedBySecond = m_SpeedLimit * 0.44704f;
					}

					// different lanes have different vehicle numbers, so we should not have openMP conflicts here
					DTAVehicle* pVehicle  = g_VehicleMap[m_VehicleDataVector[LaneNo].LaneData[v].VehicleID];
					float acceleration = SpeedBySecond - pVehicle->m_PrevSpeed;

					float vsp = 0;
					int OperatingMode = ComputeOperatingModeFromSpeed(vsp,SpeedBySecond, acceleration,0,pVehicle->m_VehicleType);

					if (acceleration <= -2)
					{
						OperatingMode = 0;
					}
					else
					{
						if (prev_prev_acceleration < -1 && prev_acceleration < -1 && acceleration < -1)
						{
							if (prev_prev_acceleration != -99999.0f || prev_acceleration != -99999.0f)
							{
								OperatingMode = 0;
							}
						}
					}

					if ((SpeedBySecond / 0.44704f) >= -1.0f && (SpeedBySecond / 0.44704f) < 1.0f)
					{
						OperatingMode = 1;
					}

					pVehicle->m_OperatingModeCount[OperatingMode]++;
					int integer_speed = SpeedBySecond / 0.44704f;  // convert meter per second to mile per hour
					pVehicle->m_SpeedCount[integer_speed]++;


					pVehicle->m_PrevSpeed  = SpeedBySecond;

					int vehicle_type = pVehicle->m_VehicleType;

					int age = pVehicle->m_Age ;

					int time_in_min = t / NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND / 60;

					if(EmissionRateData[vehicle_type][OperatingMode][age].bInitialized == false)
					{
						cout << "Emission rate data are not available for vehicle type = " <<  vehicle_type 
							<< ", operating mode = " << OperatingMode 
							<< " and age = " << age << endl;
						g_ProgramStop();
					}

					float Energy = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_TotalEnergy/3600;
					float CO2 = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_CO2/3600;
					float NOX = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_NOX/3600;
					float CO = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_CO/3600;
					float HC = EmissionRateData[vehicle_type][OperatingMode][age].meanBaseRate_HC/3600;

					m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].Energy+= Energy;
					m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].CO2+= CO2;
					m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].NOX+= NOX;
					m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].CO+= CO;
					m_VehicleDataVector[LaneNo].m_LaneEmissionVector [time_in_min].HC+= HC;

					if (pVehicle->m_VehicleID  ==  g_TargetVehicleID_OutputSecondBySecondEmissionData || 
						(g_OutputSecondBySecondEmissionData && pVehicle->m_DepartureTime >= g_start_departure_time_in_min_for_output_second_by_second_emission_data 
						&& pVehicle->m_DepartureTime <= g_end_departure_time_in_min_for_output_second_by_second_emission_data
						&& pVehicle->m_bDetailedEmissionOutput))  // record data
					{
						VehicleSpeedProfileData element;
						element.FromNodeNumber = this->m_FromNodeNumber ;
						element.ToNodeNumber  = this->m_ToNodeNumber ;
						element.TimeOnThisLinkInSecond = (t-start_time)/NUMBER_OF_CAR_FOLLOWING_SIMULATION_INTERVALS_PER_SECOND;
						element.Speed = SpeedBySecond / 0.44704f; // km/h to mph
						element.Acceleration = acceleration * 3.28084; //meter/sec^2 to feet/sec^2
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

					prev_prev_acceleration = prev_acceleration;
					prev_acceleration = acceleration * 0.44704f;
				}
			}  // for each time step

			float *tmp = LeaderPositions;
			LeaderPositions = FollowerPositions;
			FollowerPositions = tmp;
		} // for each vehicle

		//if(VechileDistanceAry != NULL)
		//{
		//	DeallocateDynamicArray<float>(VechileDistanceAry,m_VehicleDataVector[LaneNo].LaneData .size(), Max_Time_Step_In_A_Cycle+10);
		//}
	} //for each lane

	delete [] LeaderPositions;
	delete [] FollowerPositions;
}
