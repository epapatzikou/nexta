// Traffic Time Estimation, part of TLiteDoc.cpp : implementation of the CTLiteDoc class
//

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html


//    This file is part of NeXTA Version 3 (Open-source).

//    NEXTA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    NEXTA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with NEXTA.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "TLite.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "TLiteView.h"
#include "DlgMOE.h"
#include "DlgPathMOE.h"
#include "DlgTrainInfo.h"
#include "DlgFileLoading.h"
#include "DlgCarFollowing.h"
#include "MainFrm.h"
#include "Shellapi.h"
#include "DlgDefaultLinkProperties.h"
#include "DlgAssignmentSettings.h"
#include "DlgLinkList.h"
#include "DlgGridCtrl.h"
#include "Dlg_ImageSettings.h"
#include "Shellapi.h"
#include "DlgSensorDataLoading.h"
#include "Dlg_ImportODDemand.h"
#include "DlgVehiclePath.h"
#include "DlgNetworkAlignment.h"
#include "Dlg_VehEmissions.h"

extern CDlgPathMOE	*g_pPathMOEDlg;
/******************************
External calling functions
if(ReadSensorLocationData(directory+"input_sensor_location.csv") == true)
{
CWaitCursor wc;
ReadSensorData(directory);   // if there are sensor location data
ReadEventData(directory); 
}

**********************************************/

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


void DTALink::ComputeHistoricalAvg(int number_of_weekdays)
{

	m_MinSpeed = 200;
	m_MaxSpeed = 0;

	int t;
	float VolumeSum = 0;
	float SpeedSum = 0;


	for( t=0; t< 1440; t++)
	{

		// reset
		m_HistLinkMOEAry[t].ObsSpeed =0;
		m_HistLinkMOEAry[t].ObsFlow =0;
		m_HistLinkMOEAry[t].ObsCumulativeFlow =0;
		m_HistLinkMOEAry[t].ObsDensity =0;
		m_HistLinkMOEAry[t].ObsTravelTimeIndex = 0;

		// start counting
		int count = 0;


		for(int day =0; day <number_of_weekdays; day ++)
		{
			if(m_LinkMOEAry[day*1440+t].EventCode ==0)  // no event
			{
				m_HistLinkMOEAry[t].ObsSpeed +=m_LinkMOEAry[day*1440+t].ObsSpeed;
				m_HistLinkMOEAry[t].ObsFlow +=m_LinkMOEAry[day*1440+t].ObsFlow;
				m_HistLinkMOEAry[t].ObsCumulativeFlow +=m_LinkMOEAry[day*1440+t].ObsCumulativeFlow;
				m_HistLinkMOEAry[t].ObsDensity += m_LinkMOEAry[day*1440+t].ObsDensity;
				m_HistLinkMOEAry[t].ObsTravelTimeIndex += m_LinkMOEAry[day*1440+t].ObsTravelTimeIndex;

				count++;

				// update min and max speed

				if((t>=8*60 && t<9*60)) //8-9AM
				{
					// update link-specific min and max speed
					if(m_LinkMOEAry[day*1440+t].ObsSpeed < m_MinSpeed)
						m_MinSpeed = m_LinkMOEAry[day*1440+t].ObsSpeed;

					if(m_LinkMOEAry[day*1440+t].ObsSpeed > m_MaxSpeed)
						m_MaxSpeed = m_LinkMOEAry[day*1440+t].ObsSpeed;


				}
			}


		}

		if(count>=1) 
		{
			// calculate final mean statistics
			m_HistLinkMOEAry[t].ObsSpeed /=count;
			m_HistLinkMOEAry[t].ObsFlow /=count;
			m_HistLinkMOEAry[t].ObsCumulativeFlow /=count;
			m_HistLinkMOEAry[t].ObsDensity /=count;
			m_HistLinkMOEAry[t].ObsTravelTimeIndex /=count;
		}


	}

}


struc_traffic_state DTALink::GetPredictedState(int CurrentTime, int PredictionHorizon)  // return value is speed
{

	struc_traffic_state future_state;
	// step 1: calculate delta w
	float DeltaW =  m_LinkMOEAry[CurrentTime].ObsTravelTimeIndex -  m_HistLinkMOEAry[CurrentTime%1440].ObsTravelTimeIndex;

	// step 2: propogate delta w to Furture time
	//this is the most tricky part

	float FutureDeltaW = max(0,(1-PredictionHorizon)/45.0f)*DeltaW;   // after 45 min, FutureDeltaW becomes zero, completely come back to historical pattern
	// step 3: add future delta w to historical time at future time

	future_state.traveltime  = FutureDeltaW+ m_HistLinkMOEAry[(CurrentTime+PredictionHorizon)%1440].ObsTravelTimeIndex;
	// step 4: produce speed

	future_state.speed = m_Length/max(m_FreeFlowTravelTime,future_state.traveltime);

	return future_state;
}

void DTAPath::UpdateWithinDayStatistics()
{
	int t;
	for(t=0; t<1440; t++)
	{
		m_WithinDayMeanTimeDependentTravelTime[t] = 0;
		m_WithinDayMaxTimeDependentTravelTime[t] = 0;
	}

	for(t=0; t<1440*m_number_of_days; t++)
	{
		m_WithinDayMeanTimeDependentTravelTime[t%1440] += m_TimeDependentTravelTime[t]/m_number_of_days;
		m_WithinDayMaxTimeDependentTravelTime[t%1440] = max(m_WithinDayMaxTimeDependentTravelTime[t%1440],m_TimeDependentTravelTime[t]);
	}
}

bool CTLiteDoc::ReadSensorLocationData(LPCTSTR lpszFileName)
{
	CWaitCursor wc;
	FILE* st = NULL;
	bool bRectIni = false;
	fopen_s(&st,lpszFileName, "r");

	int sensor_count = 1;
	if(st!=NULL)
	{
		int sensor_count = 1;
		while(!feof(st))
		{
			DTA_sensor sensor;

			sensor.FromNodeNumber =  g_read_integer(st);

			if(sensor.FromNodeNumber == -1)
				break;

			sensor.ToNodeNumber =  g_read_integer(st);
			sensor.SensorType  =  g_read_integer(st);
			sensor.OrgSensorID  =  g_read_integer(st);

			DTALink* pLink = FindLinkWithNodeNumbers(sensor.FromNodeNumber , sensor.ToNodeNumber );

			if(pLink!=NULL)
			{
				sensor.LinkID = pLink->m_LinkID ;
				m_SensorVector.push_back(sensor);
				m_SensorIDtoLinkIDMap[sensor.OrgSensorID] = pLink->m_LinkID;
				pLink->m_bSensorData  = true;
			}else
			{

				CString msg;
				msg.Format ("Link %d -> %d in input_sensor_location.csv does not exit in input_link.csv.");
				AfxMessageBox(msg);
				break;


			}

			sensor_count++;
		}

		m_SensorLocationLoadingStatus.Format("%d sensor records are loaded from file %s.",sensor_count,lpszFileName);

		fclose(st);
		return true;
	}

	return false;
}

void CTLiteDoc::ReadSensorData(CString directory)
{
	CWaitCursor wc;
	FILE* st = NULL;
	std::list<DTALink*>::iterator iLink;
	int TimeHorizon = 1440; // 1440

	m_TimeInterval = 5;

	m_NumberOfDays = 1;

	float Occ_to_Density_Coef = 100.0f;


	CDlgSensorDataLoading dlg;

	if(m_SimulationLinkMOEDataLoadingStatus.GetLength ()>0)
		dlg.m_NumberOfDays  = 1;

	if(dlg.DoModal() ==IDOK)
	{
		m_TimeInterval = dlg.m_ObsTimeInterval;
		m_NumberOfDays = dlg.m_NumberOfDays;
		Occ_to_Density_Coef = dlg.m_Occ_to_Density_Coef;

		bool bResetMOEAryFlag = false;
		for(int day =1; day <= m_NumberOfDays; day++)
		{
			CString str_day;
			str_day.Format ("%03d", day);

			fopen_s(&st,directory+"SensorDataDay"+str_day +".csv","r");

			if(st!=NULL)
			{

				if(!bResetMOEAryFlag && m_SimulationLinkMOEDataLoadingStatus.GetLength () == 0) // simulation data are not loaded. reset data array
				{

					g_Simulation_Time_Horizon = 1440*m_NumberOfDays;

					for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
					{
						if((*iLink)->m_bSensorData == true)  // identified in the input_sensor_location.csv
						{
							(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);
						}
					}

					bResetMOEAryFlag = true;
				}

			
			int number_of_samples = 0;
			while(!feof(st))
			{

				int Month = g_read_integer(st);
				if(Month == -1)  // reach end of file
					break;

				int DayOfMonth   = g_read_integer(st);
				int Year  = g_read_integer(st);
				int Hour   = g_read_integer(st);
				int Min   = g_read_integer(st);

				int SensorID  =  g_read_integer(st);
				float TotalFlow = g_read_float(st);
				float Occupancy = g_read_float(st);
				float AvgLinkSpeed = g_read_float(st);

				int LinkID  = m_SensorIDtoLinkIDMap[SensorID];

				DTALink* pLink = NULL;
				if(LinkID>=0)
				{
					pLink = m_LinkIDMap[LinkID];
				}else
				{
					CString error_message;
					error_message.Format ("Reading error: Sensor ID %d has not been defined in file input_sensor_location.csv.");
					AfxMessageBox(error_message);
					fclose(st);
					return;
				}

				if(pLink!=NULL && pLink->m_bSensorData)
				{
					int t  = ((day - 1)*1440+ Hour*60+Min) ;

					if(t<pLink->m_SimulationHorizon)
					{

						if(AvgLinkSpeed<=1)  // 0 or negative values means missing speed
							AvgLinkSpeed = pLink->m_SpeedLimit ;


						ASSERT(pLink->m_NumLanes > 0);

						if(m_SimulationLinkMOEDataLoadingStatus.GetLength () == 0)  // simulation data not loaded
						{
							pLink->m_LinkMOEAry[ t].ObsFlow = TotalFlow*60/m_TimeInterval/pLink->m_NumLanes;  // convert to per hour link flow
							pLink->m_LinkMOEAry[ t].ObsSpeed = AvgLinkSpeed; 
							pLink->m_LinkMOEAry[ t].ObsTravelTimeIndex = pLink->m_SpeedLimit /max(1,AvgLinkSpeed)*100;


							if(Occupancy <=0.001)
								pLink->m_LinkMOEAry[t].ObsDensity = pLink->m_LinkMOEAry[t].ObsFlow / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeed);
							else
								pLink->m_LinkMOEAry[t].ObsDensity = Occupancy * Occ_to_Density_Coef;

							// copy data to other intervals
							for(int tt = 1; tt<m_TimeInterval; tt++)
							{
								pLink->m_LinkMOEAry[ t+tt].ObsFlow = pLink->m_LinkMOEAry[t].ObsFlow ;
								pLink->m_LinkMOEAry[t+tt].ObsSpeed = pLink->m_LinkMOEAry[t].ObsSpeed;
								pLink->m_LinkMOEAry[t+tt].ObsDensity = pLink->m_LinkMOEAry[t].ObsDensity;
								pLink->m_LinkMOEAry[t+tt].ObsTravelTimeIndex = pLink->m_LinkMOEAry[t].ObsTravelTimeIndex;

							}
						}else // simulation data loaded
						{

							pLink->m_LinkMOEAry[ t].ObsFlowCopy = TotalFlow*60/m_TimeInterval/pLink->m_NumLanes;  // convert to per hour link flow
							pLink->m_LinkMOEAry[ t].ObsSpeedCopy = AvgLinkSpeed; 
							pLink->m_LinkMOEAry[ t].ObsTravelTimeIndexCopy = pLink->m_SpeedLimit /max(1,AvgLinkSpeed)*100;


							if(Occupancy <=0.001)
								pLink->m_LinkMOEAry[t].ObsDensityCopy = pLink->m_LinkMOEAry[t].ObsFlowCopy / max(1.0f,pLink->m_LinkMOEAry[t].ObsSpeedCopy);
							else
								pLink->m_LinkMOEAry[t].ObsDensityCopy = Occupancy * Occ_to_Density_Coef;

							// copy data to other intervals
							for(int tt = 1; tt<m_TimeInterval; tt++)
							{
								pLink->m_LinkMOEAry[ t+tt].ObsFlowCopy = pLink->m_LinkMOEAry[t].ObsFlowCopy ;
								pLink->m_LinkMOEAry[t+tt].ObsSpeedCopy = pLink->m_LinkMOEAry[t].ObsSpeedCopy;
								pLink->m_LinkMOEAry[t+tt].ObsDensityCopy = pLink->m_LinkMOEAry[t].ObsDensityCopy;
								pLink->m_LinkMOEAry[t+tt].ObsTravelTimeIndexCopy = pLink->m_LinkMOEAry[t].ObsTravelTimeIndexCopy;

							}
						}


					}

				}	
				number_of_samples++;
			}

			m_SensorDataLoadingStatus.Format("%d sensor data records are loaded from file SensorDataDay***.csv.",number_of_samples);


			fclose(st);
		}
	}

	}	
}

void CTLiteDoc::ReadEventData(CString directory)
{
	CWaitCursor wc;
	FILE* st = NULL;

	fopen_s(&st,directory+"event.csv","r");

	if(st!=NULL)
	{

		int number_of_samples = 0;
		int episode_no =1;
		while(!feof(st))
		{

			int DayNo =  g_read_integer(st);
			if(DayNo == -1)  // reach end of file
				break;

			int Month = g_read_integer(st);

			int DayOfMonth   = g_read_integer(st);
			int Year  = g_read_integer(st);
			int Hour   = g_read_integer(st);
			int Min   = g_read_integer(st);

			int start_t  = ((DayNo - 1)*1440+ Hour*60+Min);

			Month = g_read_integer(st);

			DayOfMonth   = g_read_integer(st);
			Year  = g_read_integer(st);
			Hour   = g_read_integer(st);
			Min   = g_read_integer(st);

			int end_t  = ((DayNo - 1)*1440+ Hour*60+Min);

			int EventCode   = g_read_integer(st);

			std::list<DTALink*>::iterator iLink;
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{

				int episode_duration  = end_t - start_t;
				for(int t = start_t; t<end_t; t+= 1)
				{

					if(t<(*iLink)->m_SimulationHorizon)
					{
						(*iLink)->m_LinkMOEAry[t].EventCode  = EventCode;
						(*iLink)->m_LinkMOEAry[t].EpisodeNo = episode_no;
						(*iLink)->m_LinkMOEAry[t].EpisoDuration = episode_duration;

						number_of_samples++;

					}

				}
			}


			episode_no++;
		}	

		m_EventDataLoadingStatus.Format("%d event data records are loaded from file event.csv.",number_of_samples);

		fclose(st);
	}
}


void CTLiteDoc::BuildHistoricalDatabase()
{
	std::list<DTALink*>::iterator iLink;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		if((*iLink)->m_bSensorData  == true)
		{
			for(int t = 0; t<(*iLink)->m_SimulationHorizon; t+= m_TimeInterval)
			{
				if(t%1440 ==0)
				{  // reset at the begining of day
					(*iLink)->m_LinkMOEAry[t].ObsCumulativeFlow = (*iLink)->m_LinkMOEAry[t].ObsFlow;
				}else
				{
					(*iLink)->m_LinkMOEAry[t].ObsCumulativeFlow = (*iLink)->m_LinkMOEAry[t-m_TimeInterval].ObsCumulativeFlow  + (*iLink)->m_LinkMOEAry[t].ObsFlow ;

				}

				for(int tt= 1; tt<m_TimeInterval;tt++)
				{
					(*iLink)->m_LinkMOEAry[t+tt].ObsCumulativeFlow = 	(*iLink)->m_LinkMOEAry[t].ObsCumulativeFlow;
				}

			}


			(*iLink)->ComputeHistoricalAvg(m_NumberOfDays); 
		}
	}


}

void CTLiteDoc::OnToolsExporttoHistDatabase()
{
	CWaitCursor wc;
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Path file (*.csv)|*.csv|"));
	if(dlg.DoModal() == IDOK)
	{
		bool bFormatFlag = true;
		FILE* st = NULL;
		fopen_s(&st,dlg.GetPathName(),"w");
		if(st!=NULL)
		{
			std::list<DTALink*>::iterator iLink;

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{

				fprintf(st,"%d, %d\n", (*iLink)->m_FromNodeNumber, (*iLink)->m_ToNodeNumber );

				for(int day = 0; day < m_NumberOfDays; day++)
				{

					fprintf(st,"day, %d", day+1 );

					for(int t= 60*15.5; t<60*17; t++)
					{
						float travel_time = (*iLink)->m_FreeFlowTravelTime ;

						if((*iLink)->m_bSensorData )
						{
							travel_time = (*iLink)->m_FreeFlowTravelTime * (*iLink)->m_LinkMOEAry[1440*day+t].ObsTravelTimeIndex/100;
							TRACE("%d,%4.1f\n", t,(*iLink)->m_LinkMOEAry[1440*day+t].ObsTravelTimeIndex);
						}

						if(travel_time<0.1)
							travel_time = 0.1; // 0.1 min as the resolution.


						for(int res = 0; res<10; res++)
						{
							fprintf(st,",%4.2f", travel_time);

						}


					}
					fprintf(st,"\n");
				}

				fprintf(st,"\n");

			}
		}

		fclose(st);

	}


}


int CTLiteDoc::Routing()
{
	CWaitCursor cws;
	m_NodeSizeSP = 0;  // reset 
	if(m_OriginNodeID>=0 && m_DestinationNodeID>=0)
	{
		if(m_pNetwork !=NULL)
			delete m_pNetwork;


		unsigned int iPath;
		for (iPath = 0; iPath < m_PathDisplayList.size(); iPath++)
		{
			DTAPath* pdp = m_PathDisplayList[iPath];

			if(pdp) delete pdp;
		}

		m_PathDisplayList.clear ();

		m_pNetwork = new DTANetworkForSP(m_NodeSet.size(), m_LinkSet.size(), 1, 1, m_AdjLinkSize);  //  network instance for single processor in multi-thread environment

		int Number_of_paths = 1;
		int NodeNodeSum = 0;

		// randomize link cost to avoid overlapping

		std::list<DTALink*>::iterator iLink;
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->m_OverlappingCost  = 0;

		}

		for(int p=0; p<Number_of_paths; p++)
		{
			NodeNodeSum = 0;
			TRACE("Path %d\n",p);

			m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, true, false);
			m_pNetwork->SimplifiedTDLabelCorrecting_DoubleQueue(m_OriginNodeID,0,0);
			int     NodeSize = 0;
			int PredNode = m_pNetwork->NodePredAry[m_DestinationNodeID];            
			m_PathNodeVectorSP[NodeSize++] = m_DestinationNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
			while(PredNode != m_OriginNodeID && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
			{
				ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
				m_PathNodeVectorSP[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				NodeNodeSum+= PredNode;
				PredNode = m_pNetwork->NodePredAry[PredNode];
			}
			m_PathNodeVectorSP[NodeSize++] = m_OriginNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.

			m_NodeSizeSP = NodeSize;


			// test overlapping path
			bool bNonOverlapping = true;
			for (iPath = 0; iPath < m_PathDisplayList.size(); iPath++)
			{
				DTAPath* pdp = m_PathDisplayList[iPath];

				if(pdp->m_LinkSize==NodeSize-1 &&  pdp->m_NodeNodeSum == NodeNodeSum)
				{
					bNonOverlapping = false;
					break;
				}

			}

			if(bNonOverlapping)
			{
				// update m_PathDisplayList
				DTAPath* pdp = new DTAPath(NodeSize-1,g_Simulation_Time_Horizon);


				for (int i=1 ; i < m_NodeSizeSP; i++)
				{
					DTALink* pLink = FindLinkWithNodeIDs(m_PathNodeVectorSP[i], m_PathNodeVectorSP[i-1]);
					pLink->m_OverlappingCost = 30;  // min

					TRACE("  %d-> %d",m_NodeIDtoNameMap[m_PathNodeVectorSP[i]], m_NodeIDtoNameMap[m_PathNodeVectorSP[i-1]]);
					if(pLink!=NULL)
					{
						pdp->m_LinkVector [m_NodeSizeSP-1-i] = pLink->m_LinkID ; //starting from m_NodeSizeSP-2, to 0

						pdp->m_Distance += pLink->m_Length ;

						if(i==1) // first link
						{
							pdp->m_TravelTime = g_Simulation_Time_Stamp + pLink->GetTravelTime(g_Simulation_Time_Stamp);
						}else
						{
							pdp->m_TravelTime = pdp->m_TravelTime + pLink->GetTravelTime(pdp->m_TravelTime);

						}

					}

				}
				pdp->m_NodeNodeSum = NodeNodeSum;
				m_PathDisplayList.push_back (pdp);

				if(m_PathDisplayList.size() >=NUM_PATHS-1)
					break;

				TRACE("\n",p);

			}


		}
		// find overlapping paths
		Number_of_paths = 10;

		for(int p=0; p<Number_of_paths; p++)
		{
			TRACE("Path %d\n",p);
			NodeNodeSum=0;

			m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, false, true);
			m_pNetwork->SimplifiedTDLabelCorrecting_DoubleQueue(m_OriginNodeID,0,0);
			int     NodeSize = 0;
			int PredNode = m_pNetwork->NodePredAry[m_DestinationNodeID];            
			m_PathNodeVectorSP[NodeSize++] = m_DestinationNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
			while(PredNode != m_OriginNodeID && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
			{
				ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
				m_PathNodeVectorSP[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.
				NodeNodeSum+= PredNode;

				PredNode = m_pNetwork->NodePredAry[PredNode];
			}
			m_PathNodeVectorSP[NodeSize++] = m_OriginNodeID;  // node index 0 is the physical node, we do not add OriginCentriod into PathNodeList, so NodeSize contains all physical nodes.

			m_NodeSizeSP = NodeSize;


			// test overlapping path
			bool bNonOverlapping = true;
			for (iPath = 0; iPath < m_PathDisplayList.size(); iPath++)
			{
				DTAPath* pdp = m_PathDisplayList[iPath];

				if(pdp->m_LinkSize==NodeSize-1 &&  pdp->m_NodeNodeSum == NodeNodeSum)
				{
					bNonOverlapping = false;
					break;
				}

			}

			if(bNonOverlapping)
			{
				// update m_PathDisplayList
				DTAPath* pdp = new DTAPath(NodeSize-1,g_Simulation_Time_Horizon);

				for (int i=1 ; i < m_NodeSizeSP; i++)
				{
					DTALink* pLink = FindLinkWithNodeIDs(m_PathNodeVectorSP[i], m_PathNodeVectorSP[i-1]);
					pLink->m_OverlappingCost = 30.0f/Number_of_paths;  // min

					pdp->m_Distance += pLink->m_Length ;

					if(i==1) // first link
					{
						pdp->m_TravelTime = g_Simulation_Time_Stamp + pLink->GetTravelTime(g_Simulation_Time_Stamp);
					}else
					{
						pdp->m_TravelTime = pdp->m_TravelTime + pLink->GetTravelTime(pdp->m_TravelTime);
					}


					//                                      TRACE("  %d-> %d",m_NodeIDtoNameMap[m_PathNodeVectorSP[i]], m_NodeIDtoNameMap[m_PathNodeVectorSP[i-1]]);
					if(pLink!=NULL)
					{
						pdp->m_LinkVector [m_NodeSizeSP-1-i] = pLink->m_LinkID ;  //starting from m_NodeSizeSP-2, to 0


					}

				}
				pdp->m_NodeNodeSum = NodeNodeSum;


				m_PathDisplayList.push_back (pdp);

				//                              TRACE("\nLinkSize =  %d, NodeNodeSum =%d\n",pdp->m_LinkSize ,pdp->m_NodeNodeSum);

				if(m_PathDisplayList.size() >=NUM_PATHS)
					break;

				TRACE("\n",p);

				m_SelectPathNo = -1;

			}


		}

		// calculate time-dependent travel time


		for(unsigned int p = 0; p < m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath* pdp = m_PathDisplayList[p];

			for(int t=0; t< g_Simulation_Time_Horizon; t+= TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL)  // for each starting time
			{
				pdp->m_TimeDependentTravelTime[t] = t;  // t is the departure time

				for (int i=0 ; i < pdp->m_LinkSize; i++)  // for each pass link
				{
					DTALink* pLink = m_LinkIDMap[m_PathDisplayList[p]->m_LinkVector[i]];

					pdp->m_TimeDependentTravelTime[t] += pLink->GetTravelTime(pdp->m_TimeDependentTravelTime[t]);

					// current arrival time at a link/node along the path, t in [t] is still index of departure time, t has a dimension of 0 to 1440* number of days


					//			    TRACE("\n path %d, time at %f, TT = %f",p, pdp->m_TimeDependentTravelTime[t], pLink->GetTravelTime(pdp->m_TimeDependentTravelTime[t]) );

				}

				pdp->m_TimeDependentTravelTime[t] -= t; // remove the starting time, so we have pure travel time;

				ASSERT(pdp->m_TimeDependentTravelTime[t]>=0);

				if( pdp->m_MaxTravelTime < pdp->m_TimeDependentTravelTime[t])
					pdp->m_MaxTravelTime = pdp->m_TimeDependentTravelTime[t];

				for(int tt=1; tt<TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL; tt++)
				{
					pdp->m_TimeDependentTravelTime[t+tt] = pdp->m_TimeDependentTravelTime[t];
				}


				//                              TRACE("\n path %d, time at %d = %f",p, t,pdp->m_TimeDependentTravelTime[t]  );

			}

			pdp->UpdateWithinDayStatistics();

			/// calculate fuel consumptions
			for(unsigned int p = 0; p < m_PathDisplayList.size(); p++) // for each path
			{
				DTAPath* pdp = m_PathDisplayList[p];

				for(int t=0; t< 1440; t+= TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL)  // for each starting time
				{
					float CurrentTime = t;
					float FuelSum = 0;
					float CO2EmissionsSum = 0;
					float CO2;


					for (int i=0 ; i < pdp->m_LinkSize; i++)  // for each pass link
					{
						DTALink* pLink = m_LinkIDMap[m_PathDisplayList[p]->m_LinkVector[i]];

						FuelSum += pLink->ObtainHistFuelConsumption(CurrentTime);
						CO2= pLink->ObtainHistCO2Emissions(CurrentTime);
						CO2EmissionsSum+=CO2;

						CurrentTime += pLink->ObtainHistTravelTime(CurrentTime);

						//                                      TRACE("\n path %d, time at %f, TT = %f, Fuel %f. FS %f",p, pdp->m_TimeDependentTravelTime[t], pLink->GetTravelTime(pdp->m_TimeDependentTravelTime[t]),Fuel, FuelSum );

					}


					pdp->m_WithinDayMeanTimeDependentFuelConsumption[t] = FuelSum;
					pdp->m_WithinDayMeanTimeDependentEmissions[t]=CO2EmissionsSum;

					float value_of_time = 6.5f/60.0f;   // per min
					float value_of_fuel = 3.0f;  // per gallon
					float value_of_emissions = 0.24f;  // per pounds

					pdp->m_WithinDayMeanGeneralizedCost[t] = value_of_time* pdp->GetTravelTimeMOE(t,2)
						+ value_of_fuel* pdp->m_WithinDayMeanTimeDependentFuelConsumption[t]
					+ value_of_emissions*pdp->m_WithinDayMeanTimeDependentEmissions[t];

					for(int tt=1; tt<TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL; tt++)
					{
						pdp->m_WithinDayMeanTimeDependentFuelConsumption[t+tt] = pdp->m_WithinDayMeanTimeDependentFuelConsumption[t];
						pdp->m_WithinDayMeanTimeDependentEmissions[t+tt] = pdp->m_WithinDayMeanTimeDependentEmissions[t];
						pdp->m_WithinDayMeanGeneralizedCost[t+tt] = pdp->m_WithinDayMeanGeneralizedCost[t];

					}
				}

			}


		}

		UpdateAllViews(0);


		m_PathMOEDlgShowFlag = true;

		if(m_PathDisplayList.size() > 0)
		{
			if(g_pPathMOEDlg==NULL)
			{
				g_pPathMOEDlg = new CDlgPathMOE();
				g_pPathMOEDlg->m_pDoc  = this;

				g_pPathMOEDlg->Create(IDD_DIALOG_PATHMOE);
			}
			g_pPathMOEDlg->InsertPathMOEItem();

			g_pPathMOEDlg->ShowWindow(SW_SHOW);
		}
		return 1;
	}
	return 0;
}


int CTLiteDoc::FindLinkFromSensorLocation(float x, float y, int direction)
{

	double Min_distance = m_NetworkRect.Width()/100;  // set the selection threshod

	std::list<DTALink*>::iterator iLink;

	int SelectedLinkID = -1;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		GDPoint p0, pfrom, pto;
		p0.x  = x; p0.y  = y;
		pfrom.x  = (*iLink)->m_FromPoint.x; pfrom.y  = (*iLink)->m_FromPoint.y;
		pto.x  = (*iLink)->m_ToPoint.x; pto.y  = (*iLink)->m_ToPoint.y;

		if(direction == 1 && pfrom.x > pto.x)  // East, Xfrom should be < XTo
			continue;  //skip

		if(direction == 2 && pfrom.y < pto.y)  // South, Yfrom should be > YTo
			continue;

		if(direction == 3 && pfrom.x < pto.x)  // West, Xfrom should be > XTo
			continue;

		if(direction == 4 && pfrom.y > pto.y)  // North, Yfrom should be < YTo
			continue;

		float distance = g_DistancePointLine(p0, pfrom, pto);

		if(distance >0 && distance < Min_distance)
		{
			SelectedLinkID = (*iLink)->m_LinkID ;

			Min_distance = distance;
		}
	}

	return SelectedLinkID;

}