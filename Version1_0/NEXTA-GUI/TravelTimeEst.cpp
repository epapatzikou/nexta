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
#include "CSVParser.h"
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
#include "DlgNetworkAlignment.h"
#include "Dlg_VehEmissions.h"

extern CDlgPathMOE	*g_pPathMOEDlg;
/******************************
External calling functions
if(ReadSensorData(directory+"input_sensor.csv") == true)
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

	m_HistLinkMOEAry.reserve(1440);
	int t;
	float VolumeSum = 0;
	float SpeedSum = 0;


	if( m_LinkMOEAry.size() < 1440)
	{   // no enought data to calculate historical average
		return;
	}

	for( t=0; t< 1440; t++)
	{
		SLinkMOE element; 
		// reset
		m_HistLinkMOEAry.push_back(element);

		// start counting
		int count = 0;


		for(int day =0; day <number_of_weekdays; day ++)
		{
				m_HistLinkMOEAry[t].SimulationSpeed +=m_LinkMOEAry[day*1440+t].SensorSpeed;
				m_HistLinkMOEAry[t].SimulationLinkFlow +=m_LinkMOEAry[day*1440+t].SensorLinkCount;
				m_HistLinkMOEAry[t].SimuArrivalCumulativeFlow +=m_LinkMOEAry[day*1440+t].SensorArrivalCumulativeFlow;
				m_HistLinkMOEAry[t].SimulationDensity += m_LinkMOEAry[day*1440+t].SensorDensity;
				m_HistLinkMOEAry[t].SimulatedTravelTime += m_LinkMOEAry[day*1440+t].SimulatedTravelTimeCopy;

				count++;

				// update min and max speed

				if((t>=8*60 && t<9*60)) //8-9AM
				{
					// update link-specific min and max speed
					if(m_LinkMOEAry[day*1440+t].SimulationSpeed < m_MinSpeed)
						m_MinSpeed = m_LinkMOEAry[day*1440+t].SensorSpeed;

					if(m_LinkMOEAry[day*1440+t].SimulationSpeed > m_MaxSpeed)
						m_MaxSpeed = m_LinkMOEAry[day*1440+t].SensorSpeed;


				}

		}

		if(count>=1) 
		{
			// calculate final mean statistics
			m_HistLinkMOEAry[t].SimulationSpeed /=count;
			m_HistLinkMOEAry[t].SimulationLinkFlow /=count;
			m_HistLinkMOEAry[t].SimuArrivalCumulativeFlow /=count;
			m_HistLinkMOEAry[t].SimulationDensity /=count;
			m_HistLinkMOEAry[t].SimulatedTravelTime /=count;
		}


	}

}



struc_traffic_state DTALink::GetPredictedState(int CurrentTime, int PredictionHorizon)  // return value is speed
{

	struc_traffic_state future_state;
	// step 1: calculate delta w
	float DeltaW =  m_LinkMOEAry[CurrentTime].SimulatedTravelTime -  m_HistLinkMOEAry[CurrentTime%1440].SimulatedTravelTime;

	// step 2: propogate delta w to Furture time
	//this is the most tricky part

	float FutureDeltaW = max(0,(1-PredictionHorizon)/45.0f)*DeltaW;   // after 45 min, FutureDeltaW becomes zero, completely come back to historical pattern
	// step 3: add future delta w to historical time at future time

	future_state.traveltime  = FutureDeltaW+ m_HistLinkMOEAry[(CurrentTime+PredictionHorizon)%1440].SimulatedTravelTime;
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

bool CTLiteDoc::ReadSensorData(LPCTSTR lpszFileName)
{
	CCSVParser parser;
	int error_count = 0;

	int data_count = 0;
	if (parser.OpenCSVFile(lpszFileName))
	{

			std::list<DTALink*>::iterator iLink;
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
			(*iLink)->m_bSensorData = false;
			}

	int sensor_count = 0;

	CString error_message;

	CString prev_error_message;
		while(parser.ReadRecord())
		{
			DTA_sensor sensor;


			if(!parser.GetValueByFieldName("from_node_id",sensor.FromNodeNumber )) 
				continue;
			if(!parser.GetValueByFieldName("to_node_id",sensor.ToNodeNumber )) 
				continue;
			if(!parser.GetValueByFieldName("sensor_type",sensor.SensorType)) 
				continue;

			parser.GetValueByFieldName("x_coord",sensor.pt.x );
			parser.GetValueByFieldName("y_coord",sensor.pt.y );
			

			DTALink* pLink;
/*			int link_id = 0;
			parser.GetValueByFieldName("link_id",link_id );
			if(link_id >0)
				pLink = m_LinkNoMap[link_id -1];
			else 
*/
			pLink = FindLinkWithNodeNumbers(sensor.FromNodeNumber , sensor.ToNodeNumber,lpszFileName );


			if(pLink!=NULL)
			{
				sensor.LinkID = pLink->m_LinkNo ;

				if(m_SensorMap.find(sensor.SensorID) == m_SensorMap.end())
				{
					m_SensorMap[sensor.SensorID] = sensor;
				}

				pLink->m_bSensorData  = true;

				float start_time_in_min =0;
				float end_time_in_min = 0;

				parser.GetValueByFieldName("start_time_in_min",start_time_in_min );
				parser.GetValueByFieldName("end_time_in_min",end_time_in_min );


				int volume_count= 0;

				pLink->m_SensorTypeString = sensor.SensorType;
				if(sensor.SensorType.find("link_count")!= string::npos)
				{
					if(parser.GetValueByFieldName ("count",volume_count )==false)
					{
						AfxMessageBox("Field count is missing from file input_sensor.csv. Please check.");
						return false;
					}
				}else if(sensor.SensorType.find("lane_count")!= string::npos)
				{
					if(parser.GetValueByFieldName ("count",volume_count ) == false)
					{
						AfxMessageBox("Field count is missing from file input_sensor.csv.  Please check.");
						return false;
					
					}
					volume_count = volume_count* pLink->m_NumLanes;
				} 



				data_count++;

				if(start_time_in_min <0 && error_message.GetLength () < 1000)
				{
					CString msg;
					msg.Format ("Sensor %d-> %d has an error of start_time_in_min <0.\n",sensor.FromNodeNumber , sensor.ToNodeNumber);

				if(prev_error_message!=msg)
				{
					error_message+=msg;
					prev_error_message=  msg;
				}

				}

				if(end_time_in_min < start_time_in_min+1 && error_message.GetLength () < 1000)
				{
					CString msg;
					msg.Format ("Sensor %d-> %d has an error of end_time_in_min <= start_time_in_min: %d < %d.\n",sensor.FromNodeNumber , sensor.ToNodeNumber,
						end_time_in_min, start_time_in_min);

				if(prev_error_message!=msg)
				{
					error_message+=msg;
					prev_error_message=  msg;
				}
				}
			
				for(int t = start_time_in_min; t< end_time_in_min; t++)
				{
					if((unsigned int)t < pLink->m_LinkMOEAry.size())
					{

//						if(!sensor.SensorType.empty () && sensor.SensorType.find("count")!= string::npos)
						{

							pLink->m_LinkMOEAry[ t].SensorLinkCount = volume_count/(max(1.0,end_time_in_min-start_time_in_min));  // convert to per hour link flow
						}



					}
				}
			}else
			{
				if(error_message.GetLength () < 1000)
				{

				CString msg;
				msg.Format ("Link %d -> %d in input_sensor.csv does not exist in input_link.csv.\n", sensor.FromNodeNumber , sensor.ToNodeNumber);
				if(prev_error_message!=msg)
				{
					error_message+=msg;
					prev_error_message=  msg;
				}

				error_count++;
				continue;
				}
			}

		}

		if(error_message.GetLength ()>=1)
		{
		
		AfxMessageBox(error_message);
			
		}


		if(m_SensorMap.size()>0)
		{
			m_SensorLocationLoadingStatus.Format("%d sensors and %d sensor records are loaded from file %s.",m_SensorMap.size(),data_count,lpszFileName);
			return true;
		}
		else
			return false; // no sensors have been specified
	}

	return false;
}

bool CTLiteDoc::ReadMultiDaySensorData(LPCTSTR lpszFileName)
{
/*
	CWaitCursor wc;
	FILE* st = NULL;
	std::list<DTALink*>::iterator iLink;


	int TimeHorizon = 1440; // 1440

	m_SamplingTimeInterval = 5;

	m_NumberOfDays = 30;

	float Occ_to_Density_Coef = 100.0f;


	CDlgSensorDataLoading dlg;

	if(m_SimulationLinkMOEDataLoadingStatus.GetLength ()>0)
		dlg.m_NumberOfDays  = 1;

	if(dlg.DoModal() ==IDOK)
	{
		m_SamplingTimeInterval = dlg.m_ObsTimeInterval;
		m_NumberOfDays = dlg.m_NumberOfDays;
		Occ_to_Density_Coef = dlg.m_Occ_to_Density_Coef;

		bool bResetMOEAryFlag = false;
			CString str_day;

			fopen_s(&st,lpszFileName,"r");

			if(st!=NULL)
			{
			for(int day =1; day <= m_NumberOfDays; day++)
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

				map<long, DTALink*>::iterator it;

				DTALink* pLink ;
				if ( (it = m_SensorIDtoLinkMap.find(SensorID)) != m_SensorIDtoLinkMap.end()) 
				{
					pLink = it->second ;

				}else
				{
					CString error_message;
					error_message.Format ("Reading error: Sensor ID %d has not been defined in file input_sensor_location.csv.");
					AfxMessageBox(error_message);
					fclose(st);
					return true;
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
 							pLink->m_LinkMOEAry[ t].SimulationLinkFlow = TotalFlow*60/m_SamplingTimeInterval/pLink->m_NumLanes;  // convert to per hour link flow
							pLink->m_LinkMOEAry[ t].SimulationSpeed = AvgLinkSpeed; 
							pLink->m_LinkMOEAry[ t].SimulatedTravelTime = pLink->m_SpeedLimit /max(1,AvgLinkSpeed)*100;


							if(Occupancy <=0.001)
								pLink->m_LinkMOEAry[t].SimulationDensity = pLink->m_LinkMOEAry[t].SimulationLinkFlow / max(1.0f,pLink->m_LinkMOEAry[t].SimulationSpeed);
							else
								pLink->m_LinkMOEAry[t].SimulationDensity = Occupancy * Occ_to_Density_Coef;

							// copy data to other intervals
							for(int tt = 1; tt<m_SamplingTimeInterval; tt++)
							{
								pLink->m_LinkMOEAry[ t+tt].SimulationLinkFlow = pLink->m_LinkMOEAry[t].SimulationLinkFlow ;
								pLink->m_LinkMOEAry[t+tt].SimulationSpeed = pLink->m_LinkMOEAry[t].SimulationSpeed;
								pLink->m_LinkMOEAry[t+tt].SimulationDensity = pLink->m_LinkMOEAry[t].SimulationDensity;
								pLink->m_LinkMOEAry[t+tt].SimulatedTravelTime = pLink->m_LinkMOEAry[t].SimulatedTravelTime;

							}
						}else // simulation data loaded
						{

							pLink->m_LinkMOEAry[ t].SensorLinkCount = TotalFlow*60/m_SamplingTimeInterval;  // convert to per hour link flow
							pLink->m_LinkMOEAry[ t].SensorSpeed = AvgLinkSpeed; 
							pLink->m_LinkMOEAry[ t].SimulatedTravelTimeCopy = pLink->m_SpeedLimit /max(1,AvgLinkSpeed)*100;


							if(Occupancy <=0.001)
								pLink->m_LinkMOEAry[t].SensorDensity = pLink->m_LinkMOEAry[t].SensorLinkCount / max(1.0f,pLink->m_LinkMOEAry[t].SensorSpeed);
							else
								pLink->m_LinkMOEAry[t].SensorDensity = Occupancy * Occ_to_Density_Coef;

							// copy data to other intervals
							for(int tt = 1; tt<m_SamplingTimeInterval; tt++)
							{
								pLink->m_LinkMOEAry[ t+tt].SensorLinkCount = pLink->m_LinkMOEAry[t].SensorLinkCount ;
								pLink->m_LinkMOEAry[t+tt].SensorSpeed = pLink->m_LinkMOEAry[t].SensorSpeed;
								pLink->m_LinkMOEAry[t+tt].SensorDensity = pLink->m_LinkMOEAry[t].SensorDensity;
								pLink->m_LinkMOEAry[t+tt].SimulatedTravelTimeCopy = pLink->m_LinkMOEAry[t].SimulatedTravelTimeCopy;

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

	}else
	{
	m_NumberOfDays = 0;
	}
*/
	return false;
}

void CTLiteDoc::ReadEventData(CString directory)
{
	//CWaitCursor wc;
	//FILE* st = NULL;

	//fopen_s(&st,directory+"event.csv","r");

	//if(st!=NULL)
	//{

	//	int number_of_samples = 0;
	//	int episode_no =1;
	//	while(!feof(st))
	//	{

	//		int DayNo =  g_read_integer(st);
	//		if(DayNo == -1)  // reach end of file
	//			break;

	//		int Month = g_read_integer(st);

	//		int DayOfMonth   = g_read_integer(st);
	//		int Year  = g_read_integer(st);
	//		int Hour   = g_read_integer(st);
	//		int Min   = g_read_integer(st);

	//		int start_t  = ((DayNo - 1)*1440+ Hour*60+Min);

	//		Month = g_read_integer(st);

	//		DayOfMonth   = g_read_integer(st);
	//		Year  = g_read_integer(st);
	//		Hour   = g_read_integer(st);
	//		Min   = g_read_integer(st);

	//		int end_t  = ((DayNo - 1)*1440+ Hour*60+Min);

	//		int EventCode   = g_read_integer(st);

	//		std::list<DTALink*>::iterator iLink;
	//		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	//		{

	//			int episode_duration  = end_t - start_t;
	//			for(int t = start_t; t<end_t; t+= 1)
	//			{

	//				if(t<(*iLink)->m_SimulationHorizon)
	//				{
	//					(*iLink)->m_LinkMOEAry[t].EventCode  = EventCode;
	//					(*iLink)->m_LinkMOEAry[t].EpisodeNo = episode_no;
	//					(*iLink)->m_LinkMOEAry[t].EpisoDuration = episode_duration;

	//					number_of_samples++;

	//				}

	//			}
	//		}


	//		episode_no++;
	//	}	

	//	m_EventDataLoadingStatus.Format("%d event data records are loaded from file event.csv.",number_of_samples);

	//	fclose(st);
	//}
}


void CTLiteDoc::BuildHistoricalDatabase()
{
	std::list<DTALink*>::iterator iLink;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		if((*iLink)->m_bSensorData  == true)
		{
			for(int t = 0; t<(*iLink)->m_SimulationHorizon; t+= m_SamplingTimeInterval)
			{
				if(t%1440 ==0)
				{  // reset at the begining of day
					(*iLink)->m_LinkMOEAry[t].SimuArrivalCumulativeFlow = (*iLink)->m_LinkMOEAry[t].SimulationLinkFlow;
				}else
				{
					(*iLink)->m_LinkMOEAry[t].SimuArrivalCumulativeFlow = (*iLink)->m_LinkMOEAry[t-m_SamplingTimeInterval].SimuArrivalCumulativeFlow  + (*iLink)->m_LinkMOEAry[t].SimulationLinkFlow ;

				}

				for(int tt= 1; tt<m_SamplingTimeInterval;tt++)
				{
					(*iLink)->m_LinkMOEAry[t+tt].SimuArrivalCumulativeFlow = 	(*iLink)->m_LinkMOEAry[t].SimuArrivalCumulativeFlow;
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
		_T("Single-day file (*.csv)|*.csv|"));
	if(dlg.DoModal() == IDOK)
	{
		bool bFormatFlag = true;
		FILE* st = NULL;
		fopen_s(&st,dlg.GetPathName(),"w");
		if(st!=NULL)
			{
			fprintf(st,"parameter arcs(i,j,t_e,t_l) link travel time /\n");
			std::list<DTALink*>::iterator iLink;

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{

				int day = 0;

						int beg_time_index = 0;
						int end_time_index =  16;
				   
						for(int t= beg_time_index+1; t< end_time_index; t++)
					{

							fprintf(st,"%d. %d. ", (*iLink)->m_FromNodeNumber, (*iLink)->m_ToNodeNumber );
							float travel_time = int((*iLink)->m_Length /300*60/15+0.5);

							int time_index_from = t- beg_time_index;
							int time_index_to = t+ travel_time - beg_time_index;
							fprintf(st,"%d. %d %4.1f\n", time_index_from,time_index_to,travel_time);

					}

				
				}

				fprintf(st,"/;\n");

//	capacity

				fprintf(st,"parameter TDcapacity(i,j,t_e) capacity /\n");

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{

				int beg_time_index = 0;
				int end_time_index =  20;
				   
				for(int t= beg_time_index+1; t< end_time_index; t++)
					{
							fprintf(st,"%d. %d. %d  6\n", (*iLink)->m_FromNodeNumber, (*iLink)->m_ToNodeNumber,t );

					}

				
			}

		fprintf(st,"/;\n");

		
		fclose(st);

	}
	}



	CFileDialog dlg2(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Multi-day file (*.csv)|*.csv|"));
	if(dlg2.DoModal() == IDOK)
	{
		bool bFormatFlag = true;
		FILE* st = NULL;
		fopen_s(&st,dlg2.GetPathName(),"w");
			fprintf(st,"parameter arcs(i,j,sc,t_e,t_l) link travel time /\n");
			std::list<DTALink*>::iterator iLink;

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{

				for( int day = 1; day <= 10; day ++)
				{

						int beg_time_index = 0;
						int end_time_index =  100;
				   
						for(int t= beg_time_index+1; t< end_time_index; t++)
					{

							fprintf(st,"%d. %d. %d.", (*iLink)->m_FromNodeNumber, (*iLink)->m_ToNodeNumber, day );
							float travel_time = (*iLink)->m_FreeFlowTravelTime * (1 + g_GetRandomRatio());

							int time_index_from = t- beg_time_index;
							int time_index_to = t+ travel_time - beg_time_index;
							fprintf(st,"%d. %d %4.1f\n", time_index_from,time_index_to,travel_time);

					}


				}
				
				}

				fprintf(st,"/;");
		fclose(st);

	}

}


int CTLiteDoc::AlternativeRouting(int NumberOfRoutes = 2)
{
/*
	CWaitCursor cws;
	m_NodeSizeSP = 0;  // reset 
	if(m_OriginNodeID>=0 && m_DestinationNodeID>=0)
	{
		if(m_pNetwork !=NULL)
		{
			delete m_pNetwork;
			m_pNetwork = NULL;
		}


		unsigned int iPath;
		for (iPath = 0; iPath < m_PathDisplayList.size(); iPath++)
		{
			DTAPath* pdp = m_PathDisplayList[iPath];

			if(pdp) delete pdp;
		}

		m_PathDisplayList.clear ();

		m_pNetwork = new DTANetworkForSP(m_NodeSet.size(), m_LinkSet.size(), 1, 1, m_AdjLinkSize);  //  network instance for single processor in multi-thread environment

		int NodeNodeSum = 0;

		// randomize link cost to avoid overlapping

		std::list<DTALink*>::iterator iLink;
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->m_OverlappingCost  = 0;

		}

		for(int p=0; p<NumberOfRoutes-1; p++)
		{
			NodeNodeSum = 0;
			TRACE("Path %d\n",p);

			m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, false);
			m_pNetwork->SimplifiedTDLabelCorrecting_DoubleQueue(m_OriginNodeID,0,0,0);
			int     NodeSize = 0;
			int PredNode = m_pNetwork->NodePredAry[m_DestinationNodeID];            
			m_PathNodeVectorSP[NodeSize++] = m_DestinationNodeID;  // node index 0 is the physical node, we do not add Origincentroid into PathNodeList, so NodeSize contains all physical nodes.
			while(PredNode != m_OriginNodeID && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
			{
				ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
				m_PathNodeVectorSP[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add Origincentroid into PathNodeList, so NodeSize contains all physical nodes.
				NodeNodeSum+= PredNode;
				PredNode = m_pNetwork->NodePredAry[PredNode];
			}
			m_PathNodeVectorSP[NodeSize++] = m_OriginNodeID;  // node index 0 is the physical node, we do not add Origincentroid into PathNodeList, so NodeSize contains all physical nodes.

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

					if(pLink == NULL) // no link is found
						break;

					pLink->m_OverlappingCost = 30;  // min

					TRACE("  %d-> %d",m_NodeIDtoNameMap[m_PathNodeVectorSP[i]], m_NodeIDtoNameMap[m_PathNodeVectorSP[i-1]]);
					if(pLink!=NULL)
					{
						pdp->m_LinkVector [m_NodeSizeSP-1-i] = pLink->m_LinkNo ; //starting from m_NodeSizeSP-2, to 0

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

		for(int p=0; p<NumberOfRoutes-1; p++)
		{
			TRACE("Path %d\n",p);
			NodeNodeSum=0;

			m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, true);
			m_pNetwork->SimplifiedTDLabelCorrecting_DoubleQueue(m_OriginNodeID,0,0,0);
			int     NodeSize = 0;
			int PredNode = m_pNetwork->NodePredAry[m_DestinationNodeID];            
			m_PathNodeVectorSP[NodeSize++] = m_DestinationNodeID;  // node index 0 is the physical node, we do not add Origincentroid into PathNodeList, so NodeSize contains all physical nodes.
			while(PredNode != m_OriginNodeID && PredNode!=-1 && NodeSize< MAX_NODE_SIZE_IN_A_PATH) // scan backward in the predessor array of the shortest path calculation results
			{
				ASSERT(NodeSize< MAX_NODE_SIZE_IN_A_PATH-1);
				m_PathNodeVectorSP[NodeSize++] = PredNode;  // node index 0 is the physical node, we do not add Origincentroid into PathNodeList, so NodeSize contains all physical nodes.
				NodeNodeSum+= PredNode;

				PredNode = m_pNetwork->NodePredAry[PredNode];
			}
			m_PathNodeVectorSP[NodeSize++] = m_OriginNodeID;  // node index 0 is the physical node, we do not add Origincentroid into PathNodeList, so NodeSize contains all physical nodes.

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
					if(pLink == NULL)
						break;
					pLink->m_OverlappingCost = 30.0f/NumberOfRoutes;  // min

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
						pdp->m_LinkVector [m_NodeSizeSP-1-i] = pLink->m_LinkNo ;  //starting from m_NodeSizeSP-2, to 0


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
					DTALink* pLink = m_LinkNoMap[m_PathDisplayList[p].m_LinkVector[i]];
					if(pLink == NULL)
						break;

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
						DTALink* pLink = m_LinkNoMap[m_PathDisplayList[p].m_LinkVector[i]];

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

	if(g_pPathMOEDlg  && g_pPathMOEDlg ->GetSafeHwnd ())
	{
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
	}
		return 1;
	}
*/
	return 0;
}


int CTLiteDoc::Routing(bool bCheckConnectivity)
{
	CWaitCursor cws;
	m_NodeSizeSP = 0;  // reset 

	m_PathDisplayList.clear ();		

	std::list<DTALink*>::iterator iLink;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_OverlappingCost  = 0;  // randomize link cost to avoid overlapping

			if(m_LinkTypeMap[(*iLink)->m_link_type].IsConnector ())
				(*iLink)->m_bConnector = true;


		// consider intermediate destination here
	}


	
	if(bCheckConnectivity == false)
	{
		if(m_OriginNodeID < 0 || m_DestinationNodeID <0)
	{
		m_SelectPathNo = -1;
		return 0;
	}
	}



 // create network every time, because we might add nodes/links on the fly
	
	if(m_pNetwork ==NULL)  
		{
		m_pNetwork = new DTANetworkForSP(m_NodeSet.size(), m_LinkSet.size(), 1, 1, m_AdjLinkSize);  //  network instance for single processor in multi-thread environment
		m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, false);

		}
		int NodeNodeSum = 0;

		int PathLinkList[MAX_NODE_SIZE_IN_A_PATH];

			float TotalCost;
			bool distance_flag = true;

			
			int     NodeSize ;
			
			if(bCheckConnectivity==true)
			{
				m_pNetwork->SimplifiedTDLabelCorrecting_DoubleQueue(m_OriginNodeID, 0, m_DestinationNodeID, 1, 10.0f,PathLinkList,TotalCost, distance_flag, true, false,0);   // Pointer to previous node (node)

					for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
					{
						(*iNode)->m_DistanceToRoot  = m_pNetwork->LabelCostAry[(*iNode)->m_NodeID ];
					}
		
				return 0;
			}

			//the code below is not checking connectivity

			std::vector<int> ODNodeSequenceVector;

			ODNodeSequenceVector.push_back(m_OriginNodeID);

			// add intermediate destinations 
			for(unsigned int idest = 0; idest < m_IntermediateDestinationVector.size(); idest++)
			{
			ODNodeSequenceVector.push_back(m_IntermediateDestinationVector[idest]);
			}

			ODNodeSequenceVector.push_back(m_DestinationNodeID);


				DTAPath path_element;
				path_element.Init (0,g_Simulation_Time_Horizon);

			for(unsigned int ODSequenceNo = 0; ODSequenceNo < ODNodeSequenceVector.size()-1; ODSequenceNo++)
			{
			NodeSize= m_pNetwork->SimplifiedTDLabelCorrecting_DoubleQueue(ODNodeSequenceVector[ODSequenceNo], 0, ODNodeSequenceVector[ODSequenceNo+1], 1, 10.0f,PathLinkList,TotalCost, distance_flag, false, false,m_RandomRoutingCoefficient);   // Pointer to previous node (node)
			
			// update m_PathDisplayList
			if(NodeSize <= 1)
			{
				TRACE("error");
				return 0;
			}


				for (int i=0 ; i < NodeSize-1; i++)
				{

						path_element.m_LinkVector.push_back(PathLinkList[i]) ; //starting from m_NodeSizeSP-2, to 0

						CString label;
						label.Format ("%d", 1);
						path_element.m_PathLabelVector.push_back(label);

						DTALink* pLink = m_LinkNotoLinkMap[PathLinkList[i]];

					
						if(pLink!=NULL)
						{ 
							path_element.m_Distance += m_LinkNotoLinkMap[PathLinkList[i]]->m_Length ;
							path_element.m_NumberOfSensorsPassed += pLink->m_bSensorData;

						if(path_element.m_LinkVector.size()==1) // first link
						{
							path_element.m_TravelTime = pLink->GetTravelTime(g_Simulation_Time_Stamp);
						}else
						{
							path_element.m_TravelTime = path_element.m_TravelTime + pLink->GetTravelTime(path_element.m_TravelTime);

						}
						}
				}
			}  // for each origin sequence
				m_PathDisplayList.push_back  (path_element);
				m_SelectPathNo = 0;  // select the first path
				UpdateAllViews(0);

	// calculate time-dependent travel time

/*
		for(unsigned int p = 0; p < m_PathDisplayList.size(); p++) // for each path
		{
			DTAPath path_element = m_PathDisplayList[p];

			for(int t=0; t< g_Simulation_Time_Horizon; t+= TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL)  // for each starting time
			{
				path_element.m_TimeDependentTravelTime[t] = t;  // t is the departure time

				for (int i=0 ; i < path_element.m_LinkSize; i++)  // for each pass link
				{
					DTALink* pLink = m_LinkNoMap[m_PathDisplayList[p].m_LinkVector[i]];
					if(pLink == NULL)
						break;

					path_element.m_TimeDependentTravelTime[t] += pLink->GetTravelTime(path_element.m_TimeDependentTravelTime[t]);

					// current arrival time at a link/node along the path, t in [t] is still index of departure time, t has a dimension of 0 to 1440* number of days


					//			    TRACE("\n path %d, time at %f, TT = %f",p, path_element.m_TimeDependentTravelTime[t], pLink->GetTravelTime(path_element.m_TimeDependentTravelTime[t]) );

				}

				path_element.m_TimeDependentTravelTime[t] -= t; // remove the starting time, so we have pure travel time;

				ASSERT(path_element.m_TimeDependentTravelTime[t]>=0);

				if( path_element.m_MaxTravelTime < path_element.m_TimeDependentTravelTime[t])
					path_element.m_MaxTravelTime = path_element.m_TimeDependentTravelTime[t];

				for(int tt=1; tt<TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL; tt++)
				{
					path_element.m_TimeDependentTravelTime[t+tt] = path_element.m_TimeDependentTravelTime[t];
				}


				//                              TRACE("\n path %d, time at %d = %f",p, t,path_element.m_TimeDependentTravelTime[t]  );

			}

			path_element.UpdateWithinDayStatistics();

			/// calculate fuel consumptions
			for(unsigned int p = 0; p < m_PathDisplayList.size(); p++) // for each path
			{
				path_element = m_PathDisplayList[p];

				for(int t=0; t< 1440; t+= TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL)  // for each starting time
				{
					float CurrentTime = t;
					float FuelSum = 0;
					float CO2EmissionsSum = 0;
					float CO2;


					for (int i=0 ; i < path_element.m_LinkSize; i++)  // for each pass link
					{
						DTALink* pLink = m_LinkNoMap[path_element.m_LinkVector[i]];

						ASSERT(pLink!=NULL);
						FuelSum += pLink->ObtainHistFuelConsumption(CurrentTime);
						CO2= pLink->ObtainHistCO2Emissions(CurrentTime);
						CO2EmissionsSum+=CO2;

						CurrentTime += pLink->ObtainHistTravelTime(CurrentTime);

						//                                      TRACE("\n path %d, time at %f, TT = %f, Fuel %f. FS %f",p, path_element.m_TimeDependentTravelTime[t], pLink->GetTravelTime(path_element.m_TimeDependentTravelTime[t]),Fuel, FuelSum );

					}


					path_element.m_WithinDayMeanTimeDependentFuelConsumption[t] = FuelSum;
					path_element.m_WithinDayMeanTimeDependentEmissions[t]=CO2EmissionsSum;

					float value_of_time = 6.5f/60.0f;   // per min
					float value_of_fuel = 3.0f;  // per gallon
					float value_of_emissions = 0.24f;  // per pounds

					path_element.m_WithinDayMeanGeneralizedCost[t] = value_of_time* path_element.GetTravelTimeMOE(t,2)
						+ value_of_fuel* path_element.m_WithinDayMeanTimeDependentFuelConsumption[t]
					+ value_of_emissions*path_element.m_WithinDayMeanTimeDependentEmissions[t];

					for(int tt=1; tt<TIME_DEPENDENT_TRAVLE_TIME_CALCULATION_INTERVAL; tt++)
					{
						path_element.m_WithinDayMeanTimeDependentFuelConsumption[t+tt] = path_element.m_WithinDayMeanTimeDependentFuelConsumption[t];
						path_element.m_WithinDayMeanTimeDependentEmissions[t+tt] = path_element.m_WithinDayMeanTimeDependentEmissions[t];
						path_element.m_WithinDayMeanGeneralizedCost[t+tt] = path_element.m_WithinDayMeanGeneralizedCost[t];

					}
				}

			}


		}
*/

/*
	if(g_pPathMOEDlg  && g_pPathMOEDlg ->GetSafeHwnd ())
	{
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
	}
	*/
		return 1;
}


DTALink* CTLiteDoc::FindLinkFromSensorLocation(float x, float y, CString orientation)
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

		if(orientation.MakeUpper().Find('E')>0 && pfrom.x > pto.x)  // East, Xfrom should be < XTo
			continue;  //skip

		if(orientation.MakeUpper().Find('S')>0 && pfrom.y < pto.y)  // South, Yfrom should be > YTo
			continue;

		if(orientation.MakeUpper().Find('W')>0 && pfrom.x < pto.x)  // West, Xfrom should be > XTo
			continue;

		if(orientation.MakeUpper().Find('N')>0 && pfrom.y > pto.y)  // North, Yfrom should be < YTo
			continue;

		float distance = g_GetPoint2LineDistance(p0, pfrom, pto);

		if(distance >=0 && distance < Min_distance)
		{
			return (*iLink);

		}
	}

	return NULL;

}

void CapacityReduction::GenerateAdditionalDelayDistribution(float EntranceTime,int VehicleID)
{
// to do: 
	// 1. from mean, COV -> lognormal 
	// without incidents -> random capacity reduction due to irregularity 
	// with incidents, start with capacity reduction 
	// 2. additional delay

	int RandomSeed = VehicleID;

	for (int i=0;i<100;i++)
	{

	}

	// call g_GetRandomRatio() 100 times;
	// for incidents, first percentage

}