#pragma once
#include "Interval.h"
#include "DemandInfo.h"
#include "ODPair.h"

#include <map>
#include <iostream>
#include <string>

using std::map;
using std::string;
using std::ostream;


class CODDemandTableExt
{
public:
	bool Init(const char*);
	map<ODPair,DemandInfo>* GetODDemandMapByInterval(Interval&);
	map<ODPair,DemandInfo>* GetDefaultODDemand();
	DemandInfo* GetODDemanInfo(Interval&,ODPair);

	bool InsertODDemandInfo(int, int, int, int, int, float);

	void PrintODDemandTable(ostream&, VehicleType);

	int GetMaxOriginNum() const { return m_MaxOriginNum;}
	int GetMaxDestNum() const { return m_MaxDestNum;}

	string GetDemandFileName() const { return m_DemandFileName;}

private:
	string m_DemandFileName;
	map<Interval,map<ODPair,DemandInfo>> m_ODData;
	int m_MaxOriginNum;
	int m_MaxDestNum;
};