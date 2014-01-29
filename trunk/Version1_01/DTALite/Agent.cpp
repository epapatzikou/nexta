#include "stdafx.h"
#include <stdlib.h>
#include <crtdbg.h>

#include "DTALite.h"


#include "Geometry.h"
#include "GlobalData.h"
#include "CSVParser.h"
#include "SafetyPlanning.h"
#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>

using namespace std;

vector<int> ParseLineToIntegers(string line)
{
	vector<int> SeperatedIntegers;
	string subStr;
	istringstream ss(line);


	char Delimiter = ';';


		while (std::getline(ss,subStr,Delimiter))
		{
			int integer = atoi(subStr.c_str ());
			SeperatedIntegers.push_back(integer);
		}
	return SeperatedIntegers;
}


void g_ReadDSPVehicleFile(string file_name)
{
	if(g_TrafficFlowModelFlag == tfm_BPR)  //BRP  // static assignment parameters
	{
		g_AggregationTimetInterval =  60;
	}

	g_AggregationTimetIntervalSize = max(1,(g_DemandLoadingEndTimeInMin)/g_AggregationTimetInterval+1);
	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ZoneMap.size(), g_AggregationTimetIntervalSize);

/*     5311320           1    # of vehicles in the file, Max # of stops
        #   usec   dsec   stime vehcls vehtype ioc #ONode #IntDe info ribf    comp   izone Evac InitPos    VoT  tFlag pArrTime TP IniGas
        1  11261   3248    0.00     3     1     1     1     1     0  0.0000  1.0000 1110    0  0.26324894    1.12    2   21.0    7  0.0
         294  20.59

	 */

	FILE* st = NULL;

	fopen_s(&st,file_name.c_str (),"r"); /// 
	if(st!=NULL)
	{
		cout << "Reading file " <<  file_name << " ..."<< endl;
		g_LogFile << "Reading file " <<  file_name << endl;
		int count = 0;

		//# of vehicles in the file 
		g_read_integer(st);
		// Max # of stops 
		g_read_integer(st);
	float total_number_of_vehicles_to_be_generated = 0;

	int i = 0;
	int line_no =1;
		while(true)
		{

			line_no+=2;

			if(line_no%1000 ==0)
				cout << "loading " << line_no/1000 << " k lines" << endl;

		 // #
			int agent_id =  g_read_integer(st);

			if(agent_id<0)
				break;

			DTAVehicle* pVehicle = 0;

			pVehicle = new (std::nothrow) DTAVehicle;
			if(pVehicle == NULL)
			{
				cout << "Insufficient memory...";
				getchar();
				exit(0);

			}

			pVehicle->m_VehicleID = i;
			pVehicle->m_RandomSeed = pVehicle->m_VehicleID;

			// 	usec 

			int origin_node_number =  g_read_integer(st);
			pVehicle->m_OriginNodeID	= g_NodeNametoIDMap[origin_node_number];
			
			//dsec  

			g_read_integer(st);

			//stime 

			pVehicle->m_DepartureTime = g_read_float(st);


			if( pVehicle->m_DepartureTime < g_DemandLoadingStartTimeInMin || pVehicle->m_DepartureTime > g_DemandLoadingEndTimeInMin)
			{

				cout << "Error: agent " <<  agent_id << " in file " << file_name << " has a departure time of " << pVehicle->m_DepartureTime << ", which is out of the demand loading range: " << 
					g_DemandLoadingStartTimeInMin << "->" << g_DemandLoadingEndTimeInMin << " (min)." << endl << "Please check!" ;
				g_ProgramStop();
			}



			//vehicle class
			pVehicle->m_PricingType =  g_read_integer(st);
			
			pVehicle->m_DemandType = pVehicle->m_PricingType; 
			//vehicle type 
			
			pVehicle->m_VehicleType = g_read_integer(st);
			//information class  

			pVehicle->m_InformationClass = g_read_integer(st);
			 

			//#ONode 
			g_read_integer(st);
			// #IntDe 
			g_read_integer(st);
			//info 
			g_read_integer(st);

			//ribf  
			g_read_float(st);
			//comp 
			g_read_float(st);
			//izone 
			
			pVehicle->m_OriginZoneID =  g_read_integer(st);

			//Evac
			float evac_value  = g_read_float(st);
			//InitPos 
			g_read_float(st);
			//VoT 
			pVehicle->m_VOT = g_read_float(st);
			//tFlag 
			g_read_float(st);
			//pArrTime 
			float PATvalue =  g_read_float(st);
			//TP 
			float TP_value = g_read_float(st);
			//IniGas
			float value = g_read_float(st);

			pVehicle->m_DestinationZoneID = g_read_integer(st);




			// stop time?
			float travel_time_value = g_read_float(st);

	
			pVehicle->m_DestinationNodeID 	=   g_ZoneMap[pVehicle->m_DestinationZoneID].GetRandomDestinationIDInZone ((pVehicle->m_VehicleID%100)/100.0f); ;

			if(g_ZoneMap.find( pVehicle->m_OriginZoneID)!= g_ZoneMap.end())
			{
				g_ZoneMap[pVehicle->m_OriginZoneID].m_Demand += 1;
				g_ZoneMap[pVehicle->m_OriginZoneID].m_OriginVehicleSize += 1;

			}


			pVehicle->m_TimeToRetrieveInfo = pVehicle->m_DepartureTime;
			pVehicle->m_ArrivalTime  = 0;
			pVehicle->m_bComplete = false;
			pVehicle->m_bLoaded  = false;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions  = 0;
			pVehicle->m_Distance = 0;

			pVehicle->m_NodeSize = 0;

			pVehicle->m_NodeNumberSum =0;
			pVehicle->m_Distance =0;

			if(pVehicle->m_OriginZoneID == pVehicle->m_DestinationZoneID)
			{  // do not simulate intra zone traffic
			continue; 
			}
				if(g_DemandGlobalMultiplier<0.9999)
				{
					double random_value = g_GetRandomRatio();
					if(random_value>g_DemandGlobalMultiplier) // if random value is less than demand multiplier, then skip, not generate vehicles
					{

						delete pVehicle;
						continue;
					}
				}

			g_VehicleVector.push_back(pVehicle);
			g_VehicleMap[i]  = pVehicle;

			int AssignmentInterval = int(pVehicle->m_DepartureTime/g_AggregationTimetInterval);

			if(AssignmentInterval >= g_AggregationTimetIntervalSize)
			{
				AssignmentInterval = g_AggregationTimetIntervalSize - 1;
			}
			g_TDOVehicleArray[g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo][AssignmentInterval].VehicleArray .push_back(pVehicle->m_VehicleID);


			i++;

		}

	}else
	{
		cout << "File " << file_name << " cannot be opened. Please check." << endl;
		g_ProgramStop();

	}

}

bool AddPathToVehicle(DTAVehicle * pVehicle, std::vector<int> path_node_sequence, CString FileName) 
{
	pVehicle->m_NodeSize = path_node_sequence.size();

	if(pVehicle->m_NodeSize>=1)  // in case reading error
			{
				pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];
				pVehicle->m_NodeNumberSum = 0;
				for(int i=0; i< pVehicle->m_NodeSize; i++)
				{

					int node_id;
					float event_time_stamp,travel_time, emissions;

				 	pVehicle->m_NodeNumberSum += path_node_sequence[i];

					if(i==0)
						pVehicle->m_OriginNodeID = g_NodeNametoIDMap[path_node_sequence[0]];

					if(i==pVehicle->m_NodeSize-1)
						pVehicle->m_DestinationNodeID   = g_NodeNametoIDMap[path_node_sequence[pVehicle->m_NodeSize-1]];

					if(i>=1)
					{
						DTALink* pLink = g_LinkMap[GetLinkStringID(path_node_sequence[i-1],path_node_sequence[i])];
						if(pLink==NULL)
						{
							CString msg;
							msg.Format("Error in reading link %d->%d for vehicle id %d  in file %s.", path_node_sequence[i-1],path_node_sequence[i],pVehicle->m_VehicleID,FileName);
							cout << msg << endl;

							return false;
						}

						pVehicle->m_Distance+= pLink ->m_Length ;

						pVehicle->m_NodeAry[i-1].LinkNo  = pLink->m_LinkNo  ; // start from 0
					}


				}

	}
	return true;
}


void g_ReadDTALiteAgentCSVFile(string file_name)
{
	bool ecaculation_modeling_flag = false;

	if( file_name.find ("evacuation") != string::npos)
		ecaculation_modeling_flag = true;

	bool agent_group_flag = false;

	if( file_name.find ("group") != string::npos)
		agent_group_flag = true;

	if(g_TrafficFlowModelFlag == tfm_BPR)  //BRP  // static assignment parameters
	{
		g_AggregationTimetInterval =  60;
	}

	g_AggregationTimetIntervalSize = max(1,(g_DemandLoadingEndTimeInMin)/g_AggregationTimetInterval+1);
	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ZoneMap.size(), g_AggregationTimetIntervalSize);

	CCSVParser parser_agent;

	float total_number_of_vehicles_to_be_generated = 0;

	if (parser_agent.OpenCSVFile(file_name))
	{

		cout << "reading file " << file_name << endl;

		int line_no = 1;

		int i = 0;
		while(parser_agent.ReadRecord())
		{

			int agent_id = 0;

			parser_agent.GetValueByFieldNameRequired ("agent_id",agent_id);
			DTAVehicle* pVehicle = 0;

			pVehicle = new (std::nothrow) DTAVehicle;
			if(pVehicle == NULL)
			{
				cout << "Insufficient memory...";
				getchar();
				exit(0);

			}

			pVehicle->m_VehicleID = i;
			pVehicle->m_RandomSeed = pVehicle->m_VehicleID;

			parser_agent.GetValueByFieldNameRequired("from_zone_id",pVehicle->m_OriginZoneID);
			parser_agent.GetValueByFieldNameRequired("to_zone_id",pVehicle->m_DestinationZoneID);

			int origin_node_id = -1;
			int origin_node_number = -1;

			parser_agent.GetValueByFieldName("origin_node_id",origin_node_number);

			if(g_NodeNametoIDMap.find(origin_node_number)!= g_NodeNametoIDMap.end())  // convert node number to internal node id
			{
				origin_node_id = g_NodeNametoIDMap[origin_node_number];
			}

			int destination_node_id = -1;
			int destination_node_number= -1;
			parser_agent.GetValueByFieldName("destination_node_id",destination_node_number);

			if(g_NodeNametoIDMap.find(destination_node_number)!= g_NodeNametoIDMap.end()) // convert node number to internal node id
			{
				destination_node_id = g_NodeNametoIDMap[destination_node_number];
			}

			if(origin_node_id==-1)  // no default origin node value, re-generate origin node
				origin_node_id	= g_ZoneMap[pVehicle->m_OriginZoneID].GetRandomOriginNodeIDInZone ((pVehicle->m_VehicleID%100)/100.0f);  // use pVehicle->m_VehicleID/100.0f as random number between 0 and 1, so we can reproduce the results easily

			if(destination_node_id==-1)// no default destination node value, re-destination origin node
				destination_node_id 	=  g_ZoneMap[pVehicle->m_DestinationZoneID].GetRandomDestinationIDInZone ((pVehicle->m_VehicleID%100)/100.0f); 

			pVehicle->m_OriginNodeID	= origin_node_id; 
			pVehicle->m_DestinationNodeID 	=  destination_node_id;


			if(origin_node_id == destination_node_id)
			{  // do not simulate intra zone traffic
			continue; 
			}

			if(g_ZoneMap.find( pVehicle->m_OriginZoneID)!= g_ZoneMap.end())
			{
				g_ZoneMap[pVehicle->m_OriginZoneID].m_Demand += 1;
				g_ZoneMap[pVehicle->m_OriginZoneID].m_OriginVehicleSize += 1;

			}

			float departure_time = 0;
			parser_agent.GetValueByFieldNameRequired("departure_time",departure_time);

			pVehicle->m_DepartureTime  = departure_time;
			int beginning_departure_time = departure_time;


			if( pVehicle->m_DepartureTime < g_DemandLoadingStartTimeInMin || pVehicle->m_DepartureTime > g_DemandLoadingEndTimeInMin)
			{

				cout << "Error: agent " <<  agent_id << " in file " << file_name << " has a departure time of " << pVehicle->m_DepartureTime << ", which is out of the demand loading range: " << 
					g_DemandLoadingStartTimeInMin << "->" << g_DemandLoadingEndTimeInMin << " (min)." << endl << "Please change the setting in section agent_input, demand_loading_end_time_in_min in file DTASettings.txt" ;
				g_ProgramStop();
			}

			parser_agent.GetValueByFieldNameRequired("demand_type",pVehicle->m_DemandType);
			parser_agent.GetValueByFieldNameRequired("pricing_type",pVehicle->m_PricingType);
			parser_agent.GetValueByFieldNameRequired("vehicle_type",pVehicle->m_VehicleType);
			parser_agent.GetValueByFieldNameRequired("information_type",pVehicle->m_InformationClass);
			parser_agent.GetValueByFieldNameRequired("value_of_time",pVehicle->m_VOT);
			parser_agent.GetValueByFieldNameRequired("vehicle_age",pVehicle->m_Age );


			pVehicle->m_attribute_update_time_in_min  = -1;
			parser_agent.GetValueByFieldName("time_for_attribute_updating",pVehicle->m_attribute_update_time_in_min );
			parser_agent.GetValueByFieldName("updated_to_zone_id",pVehicle->m_DestinationZoneID_Updated );

			if(pVehicle->m_attribute_update_time_in_min>=0)  // there are vehicles' attributes need to be updated
			{

				if(g_ZoneMap.find(pVehicle->m_DestinationZoneID_Updated) == g_ZoneMap.end())
				{
				

					cout << "updated_to_zone_id " << pVehicle->m_DestinationZoneID_Updated << " does not exist for vehicle " << pVehicle->m_VehicleID << endl;
					g_ProgramStop();
						
				
				}
				g_bVehicleAttributeUpdatingFlag = true;
				g_bInformationUpdatingAndReroutingFlag = true;

			}


			int number_of_nodes = 0;
			parser_agent.GetValueByFieldNameRequired("number_of_nodes",number_of_nodes );

			std::vector<int> path_node_sequence;
			if(number_of_nodes >=2)
			{
			string path_node_sequence_str; 
			parser_agent.GetValueByFieldNameRequired("path_node_sequence",path_node_sequence_str);

			path_node_sequence = ParseLineToIntegers(path_node_sequence_str);

			AddPathToVehicle(pVehicle, path_node_sequence,file_name.c_str ());
			}

			pVehicle->m_TimeToRetrieveInfo = pVehicle->m_DepartureTime;
			pVehicle->m_ArrivalTime  = 0;
			pVehicle->m_bComplete = false;
			pVehicle->m_bLoaded  = false;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions  = 0;
			pVehicle->m_Distance = 0;

			pVehicle->m_NodeSize = 0;

			pVehicle->m_NodeNumberSum =0;
			pVehicle->m_Distance =0;

			int number_of_agents = 1;

			float ending_departure_time  = 0;
			if(agent_group_flag==true)
			{
			parser_agent.GetValueByFieldName("number_of_agents",number_of_agents);

			ending_departure_time = departure_time;

			parser_agent.GetValueByFieldName ("ending_departure_time",ending_departure_time);
			}


			for(int agent_i = 0; agent_i < number_of_agents; agent_i++)
			{

				if(agent_i>=1 && agent_group_flag)
				{
			
					DTAVehicle* pNewVehicle = new (std::nothrow) DTAVehicle;
					if(pNewVehicle == NULL)
					{
						cout << "Insufficient memory...";
						getchar();
						exit(0);
					}

					pNewVehicle = pVehicle;  // copy all attributes;

					// use new departure time 
					float departure_time  =  beginning_departure_time + (ending_departure_time - beginning_departure_time)*agent_i/number_of_agents+1; 
					pNewVehicle->m_DepartureTime = departure_time;

					// add node sequence 

					//if(number_of_nodes>=2)
					//{
					//AddPathToVehicle(pVehicle, path_node_sequence,file_name.c_str ());
					//}


					g_VehicleVector.push_back(pNewVehicle);
					g_VehicleMap[i]  = pNewVehicle;



				}else
				{
				g_VehicleVector.push_back(pVehicle);
				g_VehicleMap[pVehicle->m_VehicleID ]  = pVehicle;

				}





				int AssignmentInterval = int(pVehicle->m_DepartureTime/g_AggregationTimetInterval);

				if(AssignmentInterval >= g_AggregationTimetIntervalSize)
				{
					AssignmentInterval = g_AggregationTimetIntervalSize - 1;
				}

				ASSERT(pVehicle->m_OriginZoneID <= g_ODZoneNumberSize);

				g_TDOVehicleArray[g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo][AssignmentInterval].VehicleArray .push_back(pVehicle->m_VehicleID);

				i++;
			}
			

			line_no++;
		}



			cout << i << " agents have been read from file " << file_name << endl;

	}else
	{
		cout << "File " << file_name << " cannot be opened. Please check." << endl;
		g_ProgramStop();

	}

}

bool g_ReadTripCSVFile(string file_name, bool b_InitialLoadingFlag, bool bOutputLogFlag)
{
	float start_time_value = -100;

	if(b_InitialLoadingFlag)
	{ //allocate memory
	
	g_AggregationTimetIntervalSize = max(1,(g_DemandLoadingEndTimeInMin)/g_AggregationTimetInterval+1);
	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ZoneMap.size(), g_AggregationTimetIntervalSize);

	}

	CCSVParser parser_agent;

	float total_number_of_vehicles_to_be_generated = 0;

	if (parser_agent.OpenCSVFile(file_name,false))
	{

		if(bOutputLogFlag)
		{
		cout << "reading file " << file_name << endl;
		}
		int line_no = 1;

		int i = 0;

		int count = 0;

		int count_for_sameOD = 0;
		int count_for_not_defined_zones = 0;

		while(parser_agent.ReadRecord())
		{

			if((count+1)%1000 ==0 && bOutputLogFlag)
			{
			 cout << "reading " << count+1 << " records..." << endl;
			
			}
			count ++;

			int trip_id = 0;

			parser_agent.GetValueByFieldNameRequired ("trip_id",trip_id);
			DTAVehicle* pVehicle = 0;

			pVehicle = new (std::nothrow) DTAVehicle;
			if(pVehicle == NULL)
			{
				cout << "Insufficient memory...";
				getchar();
				exit(0);

			}

			pVehicle->m_VehicleID = g_VehicleVector.size();

			pVehicle->m_ExternalTripID = trip_id;


			pVehicle->m_RandomSeed = pVehicle->m_VehicleID;

			parser_agent.GetValueByFieldNameRequired("from_zone_id",pVehicle->m_OriginZoneID);
			parser_agent.GetValueByFieldNameRequired("to_zone_id",pVehicle->m_DestinationZoneID);

			if(g_ZoneMap.find( pVehicle->m_OriginZoneID)== g_ZoneMap.end() || g_ZoneMap.find( pVehicle->m_DestinationZoneID)== g_ZoneMap.end() )
			{
			count_for_not_defined_zones++;

			continue;
			}



			int origin_node_id = -1;
			int origin_node_number = -1;

			parser_agent.GetValueByFieldName("origin_node_id",origin_node_number);

			if(g_NodeNametoIDMap.find(origin_node_number)!= g_NodeNametoIDMap.end())  // convert node number to internal node id
			{
				origin_node_id = g_NodeNametoIDMap[origin_node_number];
			}

			int destination_node_id = -1;
			int destination_node_number= -1;
			parser_agent.GetValueByFieldName("destination_node_id",destination_node_number);

			if(g_NodeNametoIDMap.find(destination_node_number)!= g_NodeNametoIDMap.end()) // convert node number to internal node id
			{
				destination_node_id = g_NodeNametoIDMap[destination_node_number];
			}

			if(origin_node_id==-1)  // no default origin node value, re-generate origin node
				origin_node_id	= g_ZoneMap[pVehicle->m_OriginZoneID].GetRandomOriginNodeIDInZone ((pVehicle->m_VehicleID%100)/100.0f);  // use pVehicle->m_VehicleID/100.0f as random number between 0 and 1, so we can reproduce the results easily

			if(destination_node_id==-1)// no default destination node value, re-destination origin node
				destination_node_id 	=  g_ZoneMap[pVehicle->m_DestinationZoneID].GetRandomDestinationIDInZone ((pVehicle->m_VehicleID%100)/100.0f); 

			pVehicle->m_OriginNodeID	= origin_node_id; 
			pVehicle->m_DestinationNodeID 	=  destination_node_id;


			if(origin_node_id == destination_node_id)
			{  // do not simulate intra zone traffic

				count_for_sameOD ++;
			continue; 
			}

			if(g_ZoneMap.find( pVehicle->m_OriginZoneID)!= g_ZoneMap.end())
			{
				g_ZoneMap[pVehicle->m_OriginZoneID].m_Demand += 1;
				g_ZoneMap[pVehicle->m_OriginZoneID].m_OriginVehicleSize += 1;

			}

			float departure_time = 0;
			parser_agent.GetValueByFieldNameRequired("start_time_in_min",departure_time);


			if(	start_time_value < 0 )  // set first value
				start_time_value = departure_time;
			else if(start_time_value > departure_time +0.1)  // check if the departure times are sequential
			{
				cout << "start_time_in_min should be sequentially non-decreasing. Please check record " << i << " for trip_id " << trip_id << endl;
			g_ProgramStop();
			}else 
				start_time_value = departure_time;




			pVehicle->m_DepartureTime  = departure_time;
			int beginning_departure_time = departure_time;


			if( pVehicle->m_DepartureTime < g_DemandLoadingStartTimeInMin || pVehicle->m_DepartureTime > g_DemandLoadingEndTimeInMin)
			{

				cout << "Error: trip_id " <<  trip_id << " in file " << file_name << " has a start time of " << pVehicle->m_DepartureTime << ", which is out of the demand loading range: " << 
					g_DemandLoadingStartTimeInMin << "->" << g_DemandLoadingEndTimeInMin << " (min)." << endl << "Please change the setting in section agent_input, demand_loading_end_time_in_min in file DTASettings.txt" ;
				g_ProgramStop();
			}

			parser_agent.GetValueByFieldName("demand_type",pVehicle->m_DemandType);
			parser_agent.GetValueByFieldName("pricing_type",pVehicle->m_PricingType);
			parser_agent.GetValueByFieldName("vehicle_type",pVehicle->m_VehicleType);
			parser_agent.GetValueByFieldName("information_type",pVehicle->m_InformationClass);
			parser_agent.GetValueByFieldName("value_of_time",pVehicle->m_VOT);
			parser_agent.GetValueByFieldName("vehicle_age",pVehicle->m_Age );


			int number_of_nodes = 0;
			parser_agent.GetValueByFieldName("number_of_nodes",number_of_nodes );

			std::vector<int> path_node_sequence;
			if(number_of_nodes >=2)
			{
			string path_node_sequence_str; 
			parser_agent.GetValueByFieldName("path_node_sequence",path_node_sequence_str);

			path_node_sequence = ParseLineToIntegers(path_node_sequence_str);

			AddPathToVehicle(pVehicle, path_node_sequence,file_name.c_str ());
			}

			pVehicle->m_TimeToRetrieveInfo = pVehicle->m_DepartureTime;
			pVehicle->m_ArrivalTime  = 0;
			pVehicle->m_bComplete = false;
			pVehicle->m_bLoaded  = false;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions  = 0;
			pVehicle->m_Distance = 0;

			pVehicle->m_NodeSize = 0;

			pVehicle->m_NodeNumberSum =0;
			pVehicle->m_Distance =0;

			int number_of_agents = 1;

			float ending_departure_time  = 0;

			g_VehicleVector.push_back(pVehicle);
			g_VehicleMap[pVehicle->m_VehicleID ]  = pVehicle;

			int AssignmentInterval = int(pVehicle->m_DepartureTime/g_AggregationTimetInterval);

				if(AssignmentInterval >= g_AggregationTimetIntervalSize)
				{
					AssignmentInterval = g_AggregationTimetIntervalSize - 1;
				}

				ASSERT(pVehicle->m_OriginZoneID <= g_ODZoneNumberSize);

				g_TDOVehicleArray[g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo][AssignmentInterval].VehicleArray .push_back(pVehicle->m_VehicleID);

				i++;
			}
			

			line_no++;



			if(bOutputLogFlag)
			{
	
			cout << count << " records have been read from file " << file_name << endl;

			cout << i << " agents have been read from file " << file_name << endl;

			if(count_for_sameOD >=1)
				cout << "there are " << count_for_sameOD << " agents with the same from_zone_id and to_zone_id, which will not be simulated. " << endl;


			if(count_for_not_defined_zones >=1)
				cout << "there are " << count_for_not_defined_zones << " agents with zones not being defined in input_zone.csv file, which will not be simulated. " << endl;

			}
	}else
	{
		cout << "File " << file_name << " cannot be opened." << endl;
		
		return false;
	}

	return true;

}

bool g_ReadAgentBinFile(string file_name)
{
	int path_node_sequence[MAX_NODE_SIZE_IN_A_PATH];

	g_AggregationTimetIntervalSize = max(1,(g_DemandLoadingEndTimeInMin)/g_AggregationTimetInterval);
	g_TDOVehicleArray = AllocateDynamicArray<VehicleArrayForOriginDepartrureTimeInterval>(g_ZoneMap.size(), g_AggregationTimetIntervalSize);

	FILE* st = NULL;
	fopen_s(&st,file_name.c_str (),"rb");
	if(st!=NULL)
	{
		struct_VehicleInfo_Header header;

		int count =0;
		while(!feof(st))
		{

			size_t result = fread(&header,sizeof(struct_VehicleInfo_Header),1,st);

			if( header.vehicle_id < 0)
				break;

			if(result!=1)  // read end of file
				break;


			DTAVehicle* pVehicle = 0;
			//try
			//{
			pVehicle = new (std::nothrow) DTAVehicle;
			if(pVehicle == NULL)
			{
				cout << "Insufficient memory...";
				getchar();
				exit(0);

			}

			////}
			////catch (std::bad_alloc& exc)
			////{
			////	cout << "Insufficient memory...";
			////	getchar();
			////	exit(0);

			////}

			pVehicle->m_VehicleID		= header.vehicle_id;
			pVehicle->m_RandomSeed = pVehicle->m_VehicleID;

			pVehicle->m_OriginZoneID	= header.from_zone_id;
			pVehicle->m_DestinationZoneID = header.to_zone_id;

			g_ZoneMap[pVehicle->m_OriginZoneID].m_Demand += 1;
			g_ZoneMap[pVehicle->m_OriginZoneID].m_OriginVehicleSize += 1;


			pVehicle->m_DepartureTime	=  header.departure_time;
			pVehicle->m_PreferredDepartureTime = header.departure_time;
			pVehicle->m_ArrivalTime =  header.arrival_time;

			pVehicle->m_TripTime  = header.trip_time;

			pVehicle->m_DemandType = header.demand_type;
			pVehicle->m_PricingType = header.pricing_type;

			if(pVehicle->m_PricingType == 0) // unknown type
				pVehicle->m_PricingType = 1;

			pVehicle->m_VehicleType = header.vehicle_type;
			pVehicle->m_InformationClass = header.information_type;
			pVehicle->m_VOT = header.value_of_time;
			pVehicle->m_Age  = header.age;

			pVehicle->m_TimeToRetrieveInfo = pVehicle->m_DepartureTime;

			pVehicle->m_NodeSize = header.number_of_nodes;
			pVehicle->m_ArrivalTime  = 0;
			pVehicle->m_bComplete = false;
			pVehicle->m_bLoaded  = false;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions  = 0;
			pVehicle->m_Distance = 0;
			pVehicle->m_NodeNumberSum =0;

			int time_interval = pVehicle->m_DepartureTime/15;

			if(g_ODEstimationFlag==1 ) // having hist od only unde ODME mode
			{
				g_HistDemand.AddValue(pVehicle->m_OriginZoneID,pVehicle->m_DestinationZoneID,time_interval, 1); // to store the initial table as hist database
			}

			if(pVehicle->m_NodeSize>=1)  // in case reading error
			{
				pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

				pVehicle->m_NodeNumberSum = 0;
				for(int i=0; i< pVehicle->m_NodeSize; i++)
				{

					int node_id;
					float event_time_stamp,travel_time, emissions;

					struct_Vehicle_Node node_element;
					fread(&node_element,sizeof(node_element),1,st);

					path_node_sequence[i] = node_element.NodeName;
					pVehicle->m_NodeNumberSum += path_node_sequence[i];

					if(i==0)
						pVehicle->m_OriginNodeID = g_NodeNametoIDMap[path_node_sequence[0]];

					if(i==pVehicle->m_NodeSize-1)
						pVehicle->m_DestinationNodeID   = g_NodeNametoIDMap[path_node_sequence[pVehicle->m_NodeSize-1]];

					if(i>=1)
					{
						DTALink* pLink = g_LinkMap[GetLinkStringID(path_node_sequence[i-1],path_node_sequence[i])];
						if(pLink==NULL)
						{
							CString msg;
							msg.Format("Error in reading link %d->%d for vehicle id %d  in file %s.", path_node_sequence[i-1],path_node_sequence[i],header.vehicle_id,file_name.c_str ());
							cout << msg << endl; 
							fclose(st);
							g_ProgramStop();

							return false;
						}

							if(pLink->GetNumberOfLanes () <0.01)  // this is a blocked link by work zone
							{
								pVehicle->m_bForcedSwitchAtFirstIteration = true;
							
							}

						pVehicle->m_Distance+= pLink ->m_Length ;

						pVehicle->m_NodeAry[i-1].LinkNo  = pLink->m_LinkNo  ; // start from 0
					}


				}


				if(g_DemandGlobalMultiplier<0.9999)
				{
					double random_value = g_GetRandomRatio();
					if(random_value>g_DemandGlobalMultiplier) // if random value is less than demand multiplier, then skip, not generate vehicles
					{

						delete pVehicle;
						continue;
					}
				}


				g_VehicleVector.push_back(pVehicle);
				g_VehicleMap[pVehicle->m_VehicleID ]  = pVehicle;

				int AssignmentInterval = int(pVehicle->m_DepartureTime/g_AggregationTimetInterval);

				if(AssignmentInterval >= g_AggregationTimetIntervalSize)
				{
					AssignmentInterval = g_AggregationTimetIntervalSize - 1;
				}
				g_TDOVehicleArray[g_ZoneMap[pVehicle->m_OriginZoneID].m_ZoneSequentialNo][AssignmentInterval].VehicleArray .push_back(pVehicle->m_VehicleID);

				count++;

				if(count%10000==0)
					cout << "reading " << count/1000 << "K agents from binary file " << file_name << endl;
			} 
		}

		fclose(st);
		return true;

	}else
	{
		cout << "File input_agent.bin cannot be found. Please check."  << endl;
		g_ProgramStop();

		
	
	}
	return false;
}

