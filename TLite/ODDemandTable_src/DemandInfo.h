#pragma once

enum VehicleType {PASSENGER,FEIGHT,MAXVEHICLETYPE};

class DemandInfo
{
public:
	DemandInfo()
	{
		m_FlowArray[PASSENGER] = 0.0f;
		m_FlowArray[FEIGHT] = 0.0f;
		m_TotalFlow = 0.0f;
	}

	float GetTotalFlow() const
	{
		return m_TotalFlow;
	}

	float GetFlowByVehicleType(VehicleType type) const
	{
		return m_FlowArray[type];
	}

	void SetFlowByVehicleType(VehicleType type, float flow)
	{
		m_FlowArray[type] = flow;
		m_TotalFlow += flow;
	}

private:
	//CODPair m_ODPair;
	float m_FlowArray[MAXVEHICLETYPE];
	float m_TotalFlow;
};