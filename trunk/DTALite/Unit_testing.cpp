//  Portions Copyright 2012 Xuesong Zhou

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
#include "Geometry.h"
#include "GlobalData.h"
#include "CSVParser.h"

#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>

using namespace std;
	void g_MultiScenarioTrafficAssignment() 
{

	g_SummaryStatFile.Open("output_summary.csv");
	g_SummaryStatFile.WriteTextLabel ("DTALite:\nA Fast Open Source DTA Engine\n");
	g_SummaryStatFile.WriteTextLabel("Software Version =,1.001\nRelease Date=,November 1st 2012\n");

	int scenario_no;
	string scenario_name;
	int TotalUEIterationNumber = 5;

	int TestDemandLevel = 80;
	int TestFromNode = 0;
	int TestToNode = 0;
	float TestNumberOfLanes = 2;
	float LOVCost = 0;
	float HOVCost = 0;
	float TruckCost = 0;

	int NumberOfCriticalLinks = 3;
	int NumberOfCriticalODPairs = 3;

	CCSVWriter csv_output("output_multi_scenario_results.csv");
	csv_output.WriteTextString("Unit of output:");
	csv_output.WriteTextString(",,distance=,miles");
	csv_output.WriteTextString(",,speed=,mph");
	csv_output.WriteTextString(",,energy=,1000 joule");
	csv_output.WriteTextString(",,CO2,NOX,CO,HC=,g");

	int cl;
	
	csv_output.SetFieldName ("scenario_no");
	csv_output.SetFieldName ("demand_multiplier");
	csv_output.SetFieldName ("scenario_name");
	csv_output.SetFieldName ("number_of_assignment_days");
	csv_output.SetFieldName ("traffic_flow_model");

	csv_output.SetFieldName ("default_arterial_k_jam");
	csv_output.SetFieldName ("default_cycle_length");

	
	CCSVParser parser_MOE_settings;
	if (parser_MOE_settings.OpenCSVFile("input_MOE_settings.csv"))
	{
		while(parser_MOE_settings.ReadRecord())
		{
			string moe_type, moe_category_label;

			int demand_type =  0;
			int vehicle_type = 0;
			int information_type = 0;
			int from_node_id = 0;
			int mid_node_id	=0;
			int to_node_id	=0;
			int origin_zone_id	=0;
			int destination_zone_id	=0;
			int departure_starting_time	 = 0;
			int departure_ending_time= 1440;
			int entrance_starting_time	= 0;
			int entrance_ending_time = 1440;


			parser_MOE_settings.GetValueByFieldName("moe_type",moe_type);
			parser_MOE_settings.GetValueByFieldName("moe_category_label",moe_category_label);
			
			TRACE("%s\n",moe_category_label.c_str ());
			if(moe_category_label.find (",") !=  string::npos)
			{
			moe_category_label = '"' + moe_category_label + '"' ;
			}


			csv_output.SetFieldNameWithCategoryName ("#_of_vehicles_"+moe_category_label,moe_category_label );
			csv_output.SetFieldName ("percentage_"+moe_category_label );
			csv_output.SetFieldName ("avg_distance_"+moe_category_label);
			csv_output.SetFieldName ("avg_travel_time(min)_"+moe_category_label);
			csv_output.SetFieldName ("avg_speed_"+moe_category_label);
			csv_output.SetFieldName ("avg_toll_cost_"+moe_category_label);

			csv_output.SetFieldName("avg_energy_"+moe_category_label);
			csv_output.SetFieldName("avg_CO2_"+moe_category_label);
			csv_output.SetFieldName("avg_NOX_"+moe_category_label);
			csv_output.SetFieldName("avg_CO_"+moe_category_label);
			csv_output.SetFieldName("avg_HC_"+moe_category_label);
			csv_output.SetFieldName("avg_MilesPerGallon_"+moe_category_label);
			
			if(moe_type.find("Link") != string::npos)  // Link MOE
			{
//			csv_output.SetFieldName("level_of_service"+moe_category_label);

			csv_output.SetFieldName("SOV_volume"+moe_category_label);
			csv_output.SetFieldName("HOV_volume"+moe_category_label);
			csv_output.SetFieldName("Truck_volume"+moe_category_label);
			csv_output.SetFieldName("Intermodal_volume"+moe_category_label);
			
			csv_output.SetFieldName("number_of_crashes_per_year"+moe_category_label);
			csv_output.SetFieldName("number_of_fatal_and_injury_crashes_per_year"+moe_category_label);
			csv_output.SetFieldName("number_of_property_damage_only_crashes_per_year"+moe_category_label);

			}
		
		}

	
		parser_MOE_settings.CloseCSVFile ();
	}


	csv_output.WriteHeader ();

		int line_no = 1;

	CCSVParser parser_scenario;
	if (parser_scenario.OpenCSVFile("input_scenario_settings.csv"))
	{
		if(g_InitializeLogFiles()==0) 
		return;

	g_TrafficFlowModelFlag = 3;  		// Newell's traffic flow model
	g_ODEstimationFlag = 0; 			// no OD estimation
	
	int max_scenarios = 50000;
	while(parser_scenario.ReadRecord())
		{
			if(line_no>= max_scenarios)
				break;
		
			//Read DTALite Settings first
		g_ReadDTALiteSettings();


		if(parser_scenario.GetValueByFieldNameWithPrintOut("scenario_no",scenario_no)==false)
		{
			cout << "Field scenario_no cannot be found in file input_scenario_settings.csv. Please check." << endl;
			g_ProgramStop();
		}
		
		if(parser_scenario.GetValueByFieldNameWithPrintOut("scenario_name",scenario_name)==false)
		{
			cout << "Field scenario_name cannot be found in file input_scenario_settings.csv. Please check." << endl;
			g_ProgramStop();
		}

		g_SummaryStatFile.WriteTextLabel ("----------------------");
		g_SummaryStatFile.WriteTextLabel (scenario_name.c_str ());
		g_SummaryStatFile.WriteTextLabel ("----------------------\n");

//		parser_scenario.GetValueByFieldNameWithPrintOut("demand_loading_mode",g_VehicleLoadingMode) ;
		g_VehicleLoadingMode = 2;  // default meta data mode

		if(parser_scenario.GetValueByFieldNameWithPrintOut("number_of_assignment_days",TotalUEIterationNumber)==false)
		{
			cout << "Field number_of_assignment_days cannot be found in file input_scenario_settings.csv. Please check." << endl;
			g_ProgramStop();
		}

		g_NumberOfIterations = TotalUEIterationNumber-1;			// 0+1 iterations

		g_AgentBasedAssignmentFlag = 1;  // default value
//		parser_scenario.GetValueByFieldName("agent_based_assignment",g_AgentBasedAssignmentFlag);


		int traffic_flow_model = 3;
		if(parser_scenario.GetValueByFieldNameWithPrintOut("traffic_flow_model",traffic_flow_model)==false)
		{
			cout << "Field traffic_flow_model cannot be found in file input_scenario_settings.csv. Please check." << endl;
			g_ProgramStop();
		}

		g_UEAssignmentMethod = 1;
		if(parser_scenario.GetValueByFieldName("traffic_assignment_method",g_UEAssignmentMethod)==false)
		{
			cout << "Field traffic_assignment_method has not been specified in file input_scenario_settings.csv. A default method of day-to-day learning is used." << endl;
			getchar();
		}

		g_FreewayBiasFactor = 1;
		if(parser_scenario.GetValueByFieldName("freeway_bias_factor",g_FreewayBiasFactor)==false)
		{
			cout << "Field freeway_bias_factor has not been specified in file input_scenario_settings.csv. A default factor of 1 is used." << endl;
			getchar();
		}


		if(g_FreewayBiasFactor<0.1 || g_FreewayBiasFactor>2)
		{
			cout << "Field freeway_bias_factor in file input_scenario_settings.csv is out of feasible range. A default factor of 1 is used." << endl;
			getchar();
		
		}

		g_DefaultArterialKJam = 0;
		parser_scenario.GetValueByFieldNameWithPrintOut("default_arterial_k_jam",g_DefaultArterialKJam);
				
		g_DefaultCycleLength = 0;
		parser_scenario.GetValueByFieldNameWithPrintOut("default_cycle_length",g_DefaultCycleLength);

		if(g_DefaultCycleLength==0) 
			g_SimulateSignals = 0;  // no signal simulation logic

		if(parser_scenario.GetValueByFieldNameWithPrintOut("demand_multiplier",g_DemandGlobalMultiplier)==false)
		{
			cout << "Field demand_multiplier cannot be found in file input_scenario_settings.csv. Please check." << endl;
			g_ProgramStop();
		}

		string File_Link_Based_Toll,File_Incident,File_MessageSign,File_WorkZone;

		g_EmissionDataOutputFlag  = 0;
		if(parser_scenario.GetValueByFieldNameWithPrintOut("emission_data_output",g_EmissionDataOutputFlag )==false)
		{
			cout << "Field emission_data_output cannot be found in file input_scenario_settings.csv. Please check." << endl;
			g_ProgramStop();
		}

		g_ODEstimationFlag = 0;

		parser_scenario.GetValueByFieldNameWithPrintOut ("ODME_mode",g_ODEstimationFlag );


		g_ReadInputFiles(scenario_no);
	
		cout << "Start Traffic Assignment/Simulation... " << endl;

			cout << "Agent based dynamic traffic assignment... " << endl;

	if(g_AgentBasedAssignmentFlag==1)
		g_AgentBasedAssisnment();  // agent-based assignment
	else
		g_ODBasedDynamicTrafficAssignment(); // multi-iteration dynamic traffic assignment

		g_OutputSimulationStatistics(g_NumberOfIterations);

		csv_output.SetValueByFieldName ("scenario_no",scenario_no);
		csv_output.SetValueByFieldName ("scenario_name",scenario_name);
		csv_output.SetValueByFieldName ("number_of_assignment_days",TotalUEIterationNumber);
		csv_output.SetValueByFieldName ("demand_multiplier",g_DemandGlobalMultiplier);

		csv_output.SetValueByFieldName ("traffic_flow_model",traffic_flow_model);
		csv_output.SetValueByFieldName ("default_arterial_k_jam",g_DefaultArterialKJam);
		csv_output.SetValueByFieldName ("default_cycle_length",g_DefaultCycleLength);

		CCSVParser parser_MOE_settings;
	if (parser_MOE_settings.OpenCSVFile("input_MOE_settings.csv"))
	{
		while(parser_MOE_settings.ReadRecord())
		{
			string moe_type, moe_category_label;

			int demand_type =  0;
			int vehicle_type = 0;
			int information_type = 0;
			int from_node_id = 0;
			int mid_node_id	=0;
			int to_node_id	=0;
			int origin_zone_id	=0;
			int destination_zone_id	=0;
			int departure_starting_time	 = 0;
			int departure_ending_time= 1440;
			int entrance_starting_time	= 0;
			int entrance_ending_time = 1440;


			parser_MOE_settings.GetValueByFieldName("moe_type",moe_type);
			parser_MOE_settings.GetValueByFieldName("moe_category_label",moe_category_label);

			cout << " outputing MOE type " << moe_type << ", " << moe_category_label << endl;
			parser_MOE_settings.GetValueByFieldName("demand_type",demand_type);
			parser_MOE_settings.GetValueByFieldName("vehicle_type",vehicle_type);
			parser_MOE_settings.GetValueByFieldName("information_type",information_type);
			parser_MOE_settings.GetValueByFieldName("from_node_id",from_node_id);
			parser_MOE_settings.GetValueByFieldName("mid_node_id",mid_node_id);
			parser_MOE_settings.GetValueByFieldName("to_node_id",to_node_id);
			parser_MOE_settings.GetValueByFieldName("origin_zone_id",origin_zone_id);
			parser_MOE_settings.GetValueByFieldName("destination_zone_id",destination_zone_id);
			parser_MOE_settings.GetValueByFieldName("departure_starting_time_in_min",departure_starting_time);
			parser_MOE_settings.GetValueByFieldName("departure_ending_time_in_min",departure_ending_time);
			parser_MOE_settings.GetValueByFieldName("entrance_starting_time_in_min",entrance_starting_time);
			parser_MOE_settings.GetValueByFieldName("entrance_ending_time_in_min",entrance_ending_time);
		

			int Count=0; 
			float AvgTripTime, AvgDistance, AvgSpeed, AvgCost;
			EmissionStatisticsData emission_data;
			LinkMOEStatisticsData  link_data;
			Count = g_OutputSimulationMOESummary(AvgTripTime,AvgDistance, AvgSpeed,AvgCost, emission_data, link_data,
				demand_type,vehicle_type, information_type, origin_zone_id,destination_zone_id,
				from_node_id, mid_node_id, to_node_id,	
				departure_starting_time,departure_ending_time,entrance_starting_time,entrance_ending_time );

			float percentage = Count*100.0f/max(1,g_SimulationResult.number_of_vehicles);

			csv_output.SetValueByFieldName ("percentage_"+g_DemandTypeMap[demand_type].demand_type_name, percentage );

			csv_output.SetValueByFieldName ("#_of_vehicles_"+ moe_category_label,Count );
			csv_output.SetValueByFieldName ("percentage_"+moe_category_label,percentage );
			csv_output.SetValueByFieldName ("avg_travel_time(min)_"+moe_category_label,AvgTripTime);
			csv_output.SetValueByFieldName ("avg_distance_"+moe_category_label,AvgDistance);
			csv_output.SetValueByFieldName ("avg_speed_"+ moe_category_label,AvgSpeed);
			csv_output.SetValueByFieldName ("avg_toll_cost_"+ moe_category_label,AvgCost);


			csv_output.SetValueByFieldName("avg_energy_"+ moe_category_label,emission_data.AvgEnergy);
			csv_output.SetValueByFieldName("avg_CO2_"+moe_category_label,emission_data.AvgCO2);
			csv_output.SetValueByFieldName("avg_NOX_"+moe_category_label,emission_data.AvgNOX);
			csv_output.SetValueByFieldName("avg_CO_"+moe_category_label,emission_data.AvgCO);
			csv_output.SetValueByFieldName("avg_HC_"+moe_category_label,emission_data.AvgHC);
			csv_output.SetValueByFieldName("avg_MilesPerGallon_"+moe_category_label,emission_data.AvgMilesPerGallon );


				if(moe_type.find("Link")!=string::npos) // Link MOE
				{
				csv_output.SetValueByFieldName("SOV_volume"+moe_category_label,link_data.SOV_volume );
				csv_output.SetValueByFieldName("HOV_volume"+moe_category_label,link_data.HOV_volume );
				csv_output.SetValueByFieldName("Truck_volume"+moe_category_label,link_data.Truck_volume );
				csv_output.SetValueByFieldName("Intermodal_volume"+moe_category_label,link_data.Intermodal_volume );
				
				csv_output.SetValueByFieldName("number_of_crashes_per_year"+moe_category_label,link_data.number_of_crashes_per_year );
				csv_output.SetValueByFieldName("number_of_fatal_and_injury_crashes_per_year"+moe_category_label,link_data.number_of_fatal_and_injury_crashes_per_year);
				csv_output.SetValueByFieldName("number_of_property_damage_only_crashes_per_year"+moe_category_label,link_data.number_of_property_damage_only_crashes_per_year );

				}


			}
	
	}  // input MOE settings


		csv_output.WriteRecord ();


	//
	g_FreeMemory();
	line_no++;
	}  // for each scenario

	csv_output.WriteTextLabel (g_GetAppRunningTime());
	parser_MOE_settings.CloseCSVFile ();

	}else
	{

		cout << "File input_scenario_settings.csv cannot be found. Please check." << endl;
		g_ProgramStop();
	
	}
	  
}

void g_DTALiteMultiScenarioMain()
{

	g_MultiScenarioTrafficAssignment();

}