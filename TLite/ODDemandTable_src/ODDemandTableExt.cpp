#include "StdAfx.h"
#include "ODDemandTableExt.h"

#include <iostream>
#include <fstream>
#include <string>

using std::map;
using std::ifstream;
using std::string;

bool CODDemandTableExt::Init(const char* demandFile)
{
	m_MaxOriginNum = -1;
	m_MaxDestNum = -1;


	if (demandFile == NULL)
	{
		return false;
	}

	m_DemandFileName = (char*) demandFile;

	ifstream inFile(demandFile);
	string s;

	//Skip first line
	getline(inFile,s);

	int origin;
	int dest;
	float flow;
	int type;
	int start;
	int end;

	char* ptr;
	while(getline(inFile,s))
	{
		char* cstr= (char*)malloc(s.size()+1);
		strcpy(cstr,s.c_str());

		ptr = strtok(cstr,",");
		if (ptr != NULL)
		{
			origin = atoi(ptr);
		}
		else
		{
			free(cstr);
			break;
		}

		ptr = strtok(NULL,",");
		if (ptr != NULL)
		{

			dest = atoi(ptr);
		}
		else
		{
			free(cstr);
			break;
		}

		ptr = strtok(NULL,",");
		if (ptr != NULL)
		{
			flow = (float)atof(ptr);
		}
		else
		{
			free(cstr);
			break;
		}

		ptr = strtok(NULL,",");
		if (ptr != NULL)
		{
			type = atoi(ptr);
		}
		else
		{
			free(cstr);
			break;
		}

		ptr = strtok(NULL,",");
		if (ptr != NULL)
		{
			start = atoi(ptr);
		}
		else
		{
			free(cstr);
			break;
		}

		ptr = strtok(NULL,",");
		if (ptr != NULL)
		{
			end = atoi(ptr);
		}
		else
		{
			free(cstr);
			break;
		}

		if (type > MAXVEHICLETYPE)
		{
			std::cerr << "Vehicle type " << type << " not supported!\n";
		}
		else
		{
			InsertODDemandInfo(start,end,origin,dest,type,flow);

			m_MaxOriginNum = (m_MaxOriginNum>origin)?m_MaxOriginNum:origin;
			m_MaxDestNum = (m_MaxDestNum>dest)?m_MaxDestNum:dest;
		}

		free(cstr);
	}

	inFile.close();

	return true;
}

bool CODDemandTableExt::InsertODDemandInfo(int start, int end, int origin, int dest, int type, float flow)
{
	bool ret = false;
	
	Interval interval(start,end);

	ODPair odPair(origin,dest);

	m_ODData[interval][odPair].SetFlowByVehicleType((VehicleType)type,flow);
	
	return ret;
}

map<ODPair,DemandInfo>* CODDemandTableExt::GetODDemandMapByInterval(Interval& interval)
{
	if (m_ODData.find(interval) != m_ODData.end())
	{
		return &m_ODData[interval];
	}
	else
	{
		return NULL;
	}
}

map<ODPair,DemandInfo>* CODDemandTableExt::GetDefaultODDemand()
{
	if (m_ODData.size() > 0)
	{
		return &((*m_ODData.begin()).second);
	}
	else
	{
		return NULL;
	}
}


void CODDemandTableExt::PrintODDemandTable(std::ostream& out, VehicleType type)
{
	if (m_ODData.size() <= 0)
	{
		return;
	}

	map<Interval,map<ODPair,DemandInfo>>::iterator it;

	for (it = m_ODData.begin();it != m_ODData.end();it++)
	{
		out << "Interval:" << (*it).first.GetStartTime() << ", " << (*it).first.GetEndTime() << "  ---------------------" << std::endl;

		for (map<ODPair,DemandInfo>::iterator secIt = (*it).second.begin();secIt != (*it).second.end();secIt++)
		{
			out << (*secIt).first.GetOrigin() << "," << (*secIt).first.GetDestination() << ",";
			out << (*secIt).second.GetFlowByVehicleType(type) << std::endl;
		}

		out << "\n\n";
	}
}