// TLiteDoc.h : interface of the CTLiteDoc class
//
//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com)

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

#pragma once
#pragma warning(disable:4995)  // warning C4995: 'CDaoDatabase': name was marked as #pragma deprecated
#pragma warning(disable:4995)  // warning C4995: 'CDaoDatabase': name was marked as #pragma deprecated
#define _MAX_STRING_SIZE _MAX_PATH
#include "atlimage.h"
#include "math.h"
#include "Network.h"
#include ".\\cross-resolution-model\\SignalNode.h"
#include ".\\RailNetwork\\RailNetwork.h"

#include "Transit.h"
#include <iostream>
#include <fstream>

#ifndef _WIN64
#include <afxdb.h>          // MFC database support
#include <afxdao.h>
#endif

enum layer_mode
{ 
	layer_node = 0,
	layer_link,
	layer_link_label,
	layer_zone,
	layer_connector,
	layer_link_MOE,
	layer_ODMatrix,
	layer_detector,
	layer_subarea, 
	layer_workzone,
	layer_crash,
	layer_VMS,
	layer_toll,
	layer_ramp,
	layer_bluetooth,
	layer_GPS
};

enum Link_MOE {MOE_none,MOE_volume, MOE_speed, MOE_queue_length, MOE_safety,MOE_vcratio,MOE_traveltime,MOE_capacity, MOE_speedlimit, MOE_reliability, MOE_fftt, MOE_length, MOE_density, MOE_queuelength,MOE_fuel,MOE_emissions, MOE_vehicle, MOE_volume_copy, MOE_speed_copy, MOE_density_copy};

enum OD_MOE {odnone,critical_volume};

enum VEHICLE_CLASSIFICATION_SELECTION {CLS_network=0, CLS_OD,CLS_path,CLS_link,CLS_link_set,CLS_subarea_generated,CLS_subarea_traversing_through,CLS_subarea_internal_to_external,CLS_subarea_external_to_internal,CLS_subarea_internal_to_internal};
enum VEHICLE_X_CLASSIFICATION {CLS_pricing_type=0,CLS_VOT_10,CLS_VOT_15,CLS_VOT_10_SOV,CLS_VOT_10_HOV,CLS_VOT_10_truck,CLS_time_interval_15_min,CLS_time_interval_30_min,CLS_time_interval_60_min,CLS_information_class,CLS_vehicle_type};
enum VEHICLE_Y_CLASSIFICATION {CLS_vehicle_count=0,CLS_total_travel_time,CLS_avg_travel_time,CLS_total_travel_distance, CLS_avg_travel_distance,CLS_total_toll_cost,CLS_avg_toll_cost,CLS_total_generalized_cost,CLS_avg_generalized_cost,CLS_total_generalized_travel_time,CLS_avg_generalized_travel_time,CLS_total_CO2,CLS_avg_CO2};
enum LINK_BAND_WIDTH_MODE {LBW_number_of_lanes = 0, LBW_link_volume,LBW_number_of_marked_vehicles};


class MovementBezier
{
public: 
	CPoint P0,P1,P2;

	MovementBezier(CPoint p0, CPoint p1, CPoint p2)
	{
		P0 = p0;
		P1 = p1;
		P2 = p2;

	}


	float GetMinDistance(CPoint pt)
	{
		int x1 = P0.x;
		int y1 = P0.y;
		int x2 = P1.x;
		int y2 = P1.y;
		int x3 = P2.x;
		int y3 = P2.y;

		int i;
		float min_distance  = 99999;
		for (i=0; i < 100; ++i)
		{
			double t = (double)i /100.0;
			double a = pow((1.0 - t), 2.0);
			double b = 2.0 * t * (1.0 - t);
			double c = pow(t, 2.0);
			double x = a * x1 + b * x2 + c * x3;
			double y = a * y1 + b * y2 + c * y3;
			min_distance = min(sqrt( (x-pt.x)*(x-pt.x) + (y-pt.y)*(y-pt.y)),min_distance);
		}
		return min_distance;
	}

};
class PathStatistics
{
public: 
	PathStatistics()
	{
		TotalVehicleSize = 0;
		TotalTravelTime = 0;
		TotalDistance = 0;
		TotalCost = 0;
		TotalEmissions = 0;
	}

	float GetAvgTravelTime()
	{
		return TotalTravelTime/max(1,TotalVehicleSize);
	}

	int	  Origin;
	int	  Destination;
	int   NodeNumberSum;
	int   NodeSize;
	int   TotalVehicleSize;

	CString GetPathLabel()
	{
		CString label;
		label.Format("%d,%d,%d,%d", Origin , Destination, NodeNumberSum, NodeSize);
		return label;
	}

	std::vector<int> m_LinkVector;
	std::vector<int> m_NodeVector;
	std::vector<DTALink*> m_LinkPointerVector;  // used when generating physical side streets from centroids
	std::vector<GDPoint> m_ShapePoints;
	std::vector<DTAVehicle*> m_VehicleVector;


	float TotalTravelTime;
	float TotalDistance;
	float TotalCost;
	float TotalEmissions;
	CVehicleEmission emissiondata;


};

class Movement3Node
{
public:
	int TotalVehicleSize;
	int Phase1;
	int PermPhase1;
	int DetectPhase1;

	Movement3Node()
	{
		TotalVehicleSize  = 0;
		Phase1 = -1;  // default value
		PermPhase1 = -1;
		DetectPhase1 = -1;
	}

};

class CTLiteDoc : public CDocument
{
public: // create from serialization only

	CTLiteDoc();


	DECLARE_DYNCREATE(CTLiteDoc)

	// Attributes
public:


	void ShowTextLabel();



	int FindClosestNode(GDPoint point)
	{
		int SelectedNodeID = -1;

		double min_distance = 99999;
		std::list<DTANode*>::iterator iNode;

		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			double cx = point.x - (*iNode)->pt.x;
			double cy = point.y - (*iNode)->pt.y;

			double distance = pow((cx*cx + cy*cy),0.5);
			if( distance < min_distance)
			{
				SelectedNodeID = (*iNode)->m_NodeID ;
				min_distance = distance;
			}

		}

		return SelectedNodeID;

	}

	std::map<std::string,std::string> m_KML_style_map;

	float GetDemandVolume(int origin,int destination,int demand_type)
	{

		if(m_ZoneMap.find(origin) != m_ZoneMap.end())
		{
			if(m_ZoneMap[origin].m_ODDemandMatrix .find(destination) != m_ZoneMap[origin].m_ODDemandMatrix.end())
			{
				return  m_ZoneMap[origin].m_ODDemandMatrix[destination].GetValue (demand_type);
			}

		}
		return 0;
	}

	int m_OriginOnBottomFlag;
	PT_Network m_PT_network;  // public transit network class by Shuguang Li
	bool m_bShowCalibrationResults;

	int m_TrafficFlowModelFlag;
	COLORREF m_colorLOS[MAX_LOS_SIZE];

	COLORREF m_FreewayColor;
	COLORREF m_RampColor;
	COLORREF m_ArterialColor;
	COLORREF m_ConnectorColor;
	COLORREF m_TransitColor;
	COLORREF m_WalkingColor;

	float m_LOSBound[40][MAX_LOS_SIZE];
	bool m_bShowLegend;
	bool m_bShowPathList;
	float m_NodeDisplaySize;
	float m_VehicleDisplaySize;
	float m_NodeTextDisplayRatio;

	GDPoint m_Origin;

	int NPtoSP_X(GDPoint net_point,double Resolution) // convert network coordinate to screen coordinate
	{
		return int((net_point.x-m_Origin.x)*Resolution+0.5);
	}
	int NPtoSP_Y(GDPoint net_point,double Resolution) // convert network coordinate to screen coordinate
	{
		return int((net_point.y-m_Origin.y)*Resolution+0.5);

	}

	void ChangeNetworkCoordinates(int LayerNo, float XScale, float YScale, float deltaX_ratio, float deltaY_ratio);

	void HighlightPath(	std::vector<int>	m_LinkVector, int DisplayID);

	std::vector<GDPoint> m_HighlightGDPointVector;

	void HighlightSelectedVehicles(bool bSelectionFlag);
	int m_CurrentViewID;
	bool m_bSetView;
	GDPoint m_Doc_Origin;
	float m_Doc_Resolution;

	int m_ODSize;
	Link_MOE m_LinkMOEMode;
	Link_MOE m_PrevLinkMOEMode;

	OD_MOE m_ODMOEMode;

	std::vector<DTAPath>	m_PathDisplayList;
	bool m_PathMOEDlgShowFlag;
	int m_SelectPathNo;

	int m_OriginNodeID;
	int m_DestinationNodeID;

	double m_UnitFeet, m_UnitMile;
	double m_OffsetInFeet;

	bool m_bLoadNetworkDataOnly;

	COLORREF m_ColorFreeway, m_ColorHighway, m_ColorArterial;

	ofstream m_AMSLogFile;

	BOOL OnOpenDocument(CString FileName);
	BOOL OnOpenAMSDocument(CString FileName);
	bool ReadTransCADDemandCSVFile(LPCTSTR lpszFileName);
	bool ReadVISUMDemandCSVFile(LPCTSTR lpszFileName,int demand_type,int start_time_in_min,int end_time_in_min);
	bool RunGravityModel();
	bool ReadDemandMatrixFile(LPCTSTR lpszFileName,int demand_type);
	BOOL OnOpenTrafficNetworkDocument(CString ProjectFileName, bool bNetworkOnly = false, bool bImportShapeFiles = false);
	BOOL ImportingTransportationPlanningDataSet(CString ProjectFileName, bool bNetworkOnly = false, bool bImportShapeFiles = false);

	BOOL OnOpenRailNetworkDocument(CString ProjectFileName, bool bNetworkOnly = false);
	BOOL OnOpenDYNASMARTProject(CString ProjectFileName, bool bNetworkOnly = false);
	bool ReadGPSData(string FileName);
	bool m_bDYNASMARTDataSet;

	BOOL OnOpenRailNetworkDocument(LPCTSTR lpszPathName);

	// Open Graph drawing framework 
	void OGDF_WriteGraph(CString FileName);


	std::ofstream m_WarningFile;

	void OpenWarningLogFile(CString directory);
	// two basic input
	bool ReadNodeControlTypeCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadNodeCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadLinkCSVFile(LPCTSTR lpszFileName, bool bCreateNewNodeFlag, int LayerNo);   // for road network

	bool ReadGPSCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadGPSDataFile(LPCTSTR lpszFileName);   // for road network

	bool ReadRailLinkTypeCSVFile(LPCTSTR lpszFileName); 
	bool ReadRailNodeCSVFile(LPCTSTR lpszFileName);   // for rail network
	bool ReadRailLinkCSVFile(LPCTSTR lpszFileName, bool bCreateNewNodeFlag, int LayerNo);   // for rail network
	bool ReadRailTrainXMLFile(CString FileName);   // for rail network
	bool ReadTrainInfoCSVFile(LPCTSTR lpszFileName);
	bool ReadRailMOWCSVFile(LPCTSTR lpszFileName);
	bool CheckFeasibility();   // for rail network

	std::vector<train_schedule> m_train_schedule_vector;
	std::map<string, train_info> m_train_map;
	std::vector<MaintenanceOfWay> m_RailMOW_vector;


	std::map<CString, DTADemandMetaDataType> DTADemandMetaDataTypeMap;
	std::vector<CString> m_DemandMetaDataKeyVector;  // keep the order of meta demand database.

	int GetDemandTableType(int DemandType,int start_time_in_min, int end_time_in_min)
	{
		CString TableTypeKey;
		TableTypeKey.Format("type_%d_%d_%d_min",DemandType,start_time_in_min,end_time_in_min);


		if(DTADemandMetaDataTypeMap.find(TableTypeKey)!= DTADemandMetaDataTypeMap.end())
		{
			return DTADemandMetaDataTypeMap[TableTypeKey].demand_table_type_no ;
		}else
		{
			int demand_table_type_no = DTADemandMetaDataTypeMap.size()+1;  // starting from 1

			DTADemandMetaDataType element;
			element.demand_table_type_no = demand_table_type_no;
			element.key = TableTypeKey;
			element.trip_type = DemandType;
			element.start_time_in_min = start_time_in_min;
			element.end_time_in_min = end_time_in_min;
			DTADemandMetaDataTypeMap[TableTypeKey] = element;
			m_DemandMetaDataKeyVector.push_back(TableTypeKey);

			return demand_table_type_no;

		}

	}


	std::vector <int> m_LinkIDRecordVector;  // used to record if a unique link id has been used;
	int FindUniqueLinkID();

	bool ReadTransitFiles(CString ProjectFolder);   // for road network

	void OffsetLink();
	bool m_bBezierCurveFlag;


	void GenerateOffsetLinkBand();

	void ReCalculateLinkBandWidth();
	bool ReadZoneCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadActivityLocationCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadDemandCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadMetaDemandCSVFile(LPCTSTR lpszFileName);   // for road network

	
	bool ReadSubareaCSVFile(LPCTSTR lpszFileName);
	bool ReadVOTCSVFile(LPCTSTR lpszFileName);  
	bool ReadTemporalDemandProfileCSVFile(LPCTSTR lpszFileName);  
	bool ReadVehicleTypeCSVFile(LPCTSTR lpszFileName);  
	bool ReadDemandTypeCSVFile(LPCTSTR lpszFileName);
	bool ReadLinkTypeCSVFile(LPCTSTR lpszFileName); 

	bool ReadScenarioData();   // for road network

	int m_DemandLoadingStartTimeInMin;
	int m_DemandLoadingEndTimeInMin;

	// structure for demand file

	struct DemandRecordData
	{
		INT32 origin_zone, destination_zone;
		INT32 starting_time_in_min;
		INT32 ending_time_in_min;
		float number_of_vehicles[5] ;
	};


	int ReadVMSScenarioData();
	int ReadWorkZoneScenarioData();
	int ReadIncidentScenarioData();
	int ReadLink_basedTollScenarioData();

	bool WriteLink_basedTollScenarioData();
	bool WriteVMSScenarioData();
	bool WriteIncidentScenarioData();
	bool WriteWorkZoneScenarioData();



	bool ReadNodeGeoFile(LPCTSTR lpszFileName); 
	bool ReadLinkGeoFile(LPCTSTR lpszFileName);
	bool ReadZoneGeoFile(LPCTSTR lpszFileName); 

	bool ReadZoneShapeCSVFile(LPCTSTR lpszFileName); 

	bool ReadTripTxtFile(LPCTSTR lpszFileName);  
	bool Read3ColumnTripTxtFile(LPCTSTR lpszFileName);  

	// additional input
	void LoadSimulationOutput();
	void ReadSimulationLinkMOEData(LPCTSTR lpszFileName);
	void ReadSimulationLinkMOEData_Parser(LPCTSTR lpszFileName);
	void ReadSimulationLinkMOEData_Bin(LPCTSTR lpszFileName);
	void ReadSimulationLinkStaticMOEData(LPCTSTR lpszFileName);
	void ReadObservationLinkVolumeData(LPCTSTR lpszFileName);

	bool ReadTimetableCVSFile(LPCTSTR lpszFileName);
	void ReadHistoricalData(CString directory);

	int m_SamplingTimeInterval;
	int m_AVISamplingTimeInterval;
	int m_NumberOfDays;
	int m_SimulationStartTime_in_min;
	int m_SimulationEndTime_in_min;


	bool ReadSensorData(LPCTSTR lpszFileName);
	void ReadEventData(CString directory);
	void BuildHistoricalDatabase();

	bool ReadMultiDaySensorData(LPCTSTR lpszFileName);
	bool ReadInputEmissionRateFile(LPCTSTR lpszFileName);
	CEmissionRate EmissionRateData[MAX_VEHICLE_TYPE_SIZE][_MAXIMUM_OPERATING_MODE_SIZE];

	int m_import_shape_files_flag;

	std::vector <CString> m_MessageStringVector;

	CString m_NodeDataLoadingStatus;
	CString m_LinkDataLoadingStatus;
	CString m_ZoneDataLoadingStatus;
	CString m_DemandDataLoadingStatus;
	CString m_ScenarioDataLoadingStatus;

	CString m_LinkTrainTravelTimeDataLoadingStatus;
	CString m_TimetableDataLoadingStatus;

	CString m_ObsLinkVolumeStatus;
	CString m_BackgroundImageFileLoadingStatus;

	CString m_SimulationLinkMOEDataLoadingStatus;
	bool m_bSimulationDataLoaded;
	CString m_SimulationVehicleDataLoadingStatus;
	CString m_PathDataLoadingStatus;
	CString m_MovementDataLoadingStatus;
	CString m_SignalDataLoadingStatus;
	CString m_SensorLocationLoadingStatus;

	CString m_SensorDataLoadingStatus;
	CString m_EventDataLoadingStatus;
	CString m_StrLoadingTime;


	DTALink* FindLinkFromSensorLocation(float x, float y, CString orientation);

	int GetVehilePosition(DTAVehicle* pVehicle, double CurrentTime, float& ratio);
	float GetLinkMOE(DTALink* pLink, Link_MOE LinkMOEMode, int CurrentTime);
	float GetLinkMOE(DTALink* pLink, Link_MOE LinkMOEMode, int CurrentTime,  int AggregationIntervalInMin, float &value);

	CString GetTurnString(DTA_Turn turn)
	{
		CString str;
		switch (turn)
		{
		case DTA_LeftTurn:  str.Format("Left"); break;
		case DTA_Through:  str.Format("Through"); break;
		case DTA_RightTurn:  str.Format("Right"); break;
		default :  str.Format("Other"); break;
		}

		return str;
	}

	int GetLOSCode(float Power)
	{

		if(m_LinkMOEMode == MOE_speed)
			Power *=100;

		for(int los = 1; los < MAX_LOS_SIZE-1; los++)
		{
			if( (m_LOSBound[m_LinkMOEMode][los] <= Power && Power < m_LOSBound[m_LinkMOEMode][los+1]) ||
				(m_LOSBound[m_LinkMOEMode][los] >= Power && Power > m_LOSBound[m_LinkMOEMode][los+1]))

				return los;
		}
		return 0;
	}

public:

	void SetStatusText(CString StatusText);

	std::list<DTANode*>		m_NodeSet;
	std::list<DTALink*>		m_LinkSet;

	std::list<DTAPoint*>	m_DTAPointSet;
	std::list<DTALine*>		m_DTALineSet;


	std::list<DTANode*>		m_SubareaNodeSet;
	std::list<DTALink*>		m_SubareaLinkSet;

	bool m_bSaveProjectFromSubareaCut;

	std::map<int, DTAZone>	m_ZoneMap;
	int m_CriticalOriginZone;
	int m_CriticalDestinationZone;

	VehicleStatistics** m_ODMOEMatrix;

	int GetZoneID(GDPoint pt)
	{
		// for all zones
		std::map<int, DTAZone>	:: const_iterator itr;

		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)
		{
			DTAZone Zone = itr->second;

			if(Zone.IsInside (pt))
				return itr->first;
		}

		return -1;
	}

	int GetZoneIDFromShapePoints(GDPoint pt)
	{
		// for all zones
		std::map<int, DTAZone>	:: const_iterator itr;
		double min_distance = 999999;
		int ZoneID = -1;
		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)
		{
			DTAZone Zone = itr->second;

			float distance = g_CalculateP2PDistanceInMileFromLatitudeLongitude(pt, Zone.GetCenter());  // go through each GPS location point
			if(distance < min_distance)
			{
				min_distance = distance;
				ZoneID = itr->first;
			}
		}

		return ZoneID;
	}

	std::list<DTAVehicle*>	m_VehicleSet;
	std::map<long, DTAVehicle*> m_VehicleIDMap;

	std::list<DTAVehicle*>	m_ProbeSet;
	std::map<long, DTAVehicle*> m_ProbeMap;

	std::map<long, CAVISensorPair> m_AVISensorMap;

	std::map<int, DTANode*> m_NodeIDMap;

	std::map<int, DTANode*> m_SubareaNodeIDMap;
	bool CTLiteDoc::WriteSubareaFiles();

	std::map<long, DTALink*> m_LinkNoMap;


	bool m_EmissionDataFlag;

	int m_AdjLinkSize;

	DTANetworkForSP* m_pNetwork;
	DTANetworkForSP m_Network;

	float m_RandomRoutingCoefficient;

	std::vector<int> m_IntermediateDestinationVector;

	int Routing(bool bCheckConnectivity);
	int AlternativeRouting(int NumberOfRoutes);

	CString GetWorkspaceTitleName(CString strFullPath);
	CString m_ProjectTitle;

	float FillODMatrixFromCSVFile(LPCTSTR lpszFileName);
	void AdjustCoordinateUnitToMile();

	void ReadTrainProfileCSVFile(LPCTSTR lpszFileName);
	void ReadVehicleCSVFile_Parser(LPCTSTR lpszFileName);
	void ReadAMSPathCSVFile(LPCTSTR lpszFileName);
	void ReadAMSMovementCSVFile(LPCTSTR lpszFileName);

	void ReadVehicleCSVFile(LPCTSTR lpszFileName);
	bool ReadVehicleBinFile(LPCTSTR lpszFileName);

	bool WriteSelectVehicleDataToCSVFile(LPCTSTR lpszFileName, std::vector<DTAVehicle*> VehicleVector);

	CString GetPricingTypeStr(int PricingType)
	{

		if(PricingType == 1) return "SOV";
		if(PricingType == 2) return "HOV";
		if(PricingType == 3) return "truck";

		return "NULL";
	}

	VEHICLE_CLASSIFICATION_SELECTION m_VehicleSelectionMode;
	LINK_BAND_WIDTH_MODE m_LinkBandWidthMode;
	float m_MaxLinkWidthAsNumberOfLanes;
	float m_MaxLinkWidthAsLinkVolume;

	std::map<int, VehicleStatistics> m_ClassificationTable;

	bool SelectVehicleForAnalysis(DTAVehicle* pVehicle, VEHICLE_CLASSIFICATION_SELECTION vehicle_selection);
	void MarkLinksInSubarea();
	void GenerateClassificationForDisplay(VEHICLE_X_CLASSIFICATION x_classfication, VEHICLE_Y_CLASSIFICATION y_classfication);

	CString FindClassificationLabel(VEHICLE_X_CLASSIFICATION x_classfication, int index);
	int FindClassificationNo(DTAVehicle* pVehicle, VEHICLE_X_CLASSIFICATION x_classfication);
	void GenerateVehicleClassificationData(VEHICLE_CLASSIFICATION_SELECTION vehicle_selection, VEHICLE_X_CLASSIFICATION x_classfication);

	void ReadBackgroundImageFile(LPCTSTR lpszFileName);
	int m_PathNodeVectorSP[MAX_NODE_SIZE_IN_A_PATH];
	long m_NodeSizeSP;

	std::map<int, int> m_VehicleType2PricingTypeMap;
	std::map<int, int> m_NodeIDtoNameMap;
	std::map<int, int> m_NodeNametoIDMap;
	std::map<int, int> m_NodeIDtoZoneNameMap;

	int m_SelectedLinkID;
	void ZoomToSelectedLink(int SelectedLinkNo);
	void ZoomToSelectedNode(int SelectedNodeNumber);

	int m_SelectedNodeID;
	int m_SelectedVehicleID;
	string m_SelectedTrainHeader;


	std::map<string, DTA_sensor> m_SensorMap;
	std::vector<DTAVehicleType> m_VehicleTypeVector;
	std::vector<DTADemandType> m_DemandTypeVector;
	std::map<int,DTALinkType> m_LinkTypeMap;
	std::map<int, string> m_NodeTypeMap;

	int m_ControlType_UnknownControl;
	int m_ControlType_NoControl;
	int m_ControlType_ExternalNode;
	int m_ControlType_YieldSign;
	int m_ControlType_2wayStopSign;
	int m_ControlType_4wayStopSign;
	int m_ControlType_PretimedSignal;
	int m_ControlType_ActuatedSignal;
	int m_ControlType_Roundabout;


	std::vector<DTAVOTDistribution> m_VOTDistributionVector;

	std::vector<DTADemandProfile> m_DemandProfileVector;


	std::vector<DTA_Train*> m_TrainVector;

	CString m_ProjectDirectory;

	GDRect m_NetworkRect;

	float m_DefaultNumLanes;
	float m_DefaultSpeedLimit;
	float m_DefaultCapacity;
	float m_DefaultLinkType;

	bool m_bLinkToBeShifted;

	int m_LaneWidthInFeet;

	void ShowLegend(bool ShowLegendStatus);
	DTALink* AddNewLink(int FromNodeID, int ToNodeID, bool bOffset = false, bool bAVIFlag = false)
	{
		DTALink* pLink = 0;

		if(bAVIFlag == false) // not AVI sensor link, --> physical link
		{
			pLink = FindLinkWithNodeIDs(FromNodeID,ToNodeID);
			if(pLink != NULL)
				return NULL;  // a link with the same from and to node numbers exists!
		}

		pLink = new DTALink(1);
		pLink->m_AVISensorFlag = bAVIFlag;
		pLink->m_LinkNo = (int)(m_LinkSet.size());
		pLink->m_FromNodeNumber = m_NodeIDtoNameMap[FromNodeID];
		pLink->m_ToNodeNumber = m_NodeIDtoNameMap[ToNodeID];
		pLink->m_FromNodeID = FromNodeID;
		pLink->m_ToNodeID= ToNodeID;

		pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
		pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;

		m_NodeIDMap[FromNodeID ]->m_Connections+=1;
		m_NodeIDMap[FromNodeID ]->m_OutgoingLinkVector.push_back(pLink->m_LinkNo);

		m_NodeIDMap[ToNodeID ]->m_Connections+=1;

		unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
		m_NodeIDtoLinkMap[LinkKey] = pLink;

		__int64  LinkKey2 = GetLink64Key(pLink-> m_FromNodeNumber,pLink->m_ToNodeNumber);
		m_NodeNumbertoLinkMap[LinkKey2] = pLink;

		pLink->m_NumLanes= m_DefaultNumLanes;
		pLink->m_SpeedLimit= m_DefaultSpeedLimit;
		pLink->m_ReversedSpeedLimit = m_DefaultSpeedLimit;
		pLink->m_StaticSpeed = m_DefaultSpeedLimit;
		pLink->m_Length= pLink->DefaultDistance()/max(0.0000001,m_UnitMile);
		pLink->m_FreeFlowTravelTime = pLink->m_Length / pLink->m_SpeedLimit *60.0f;
		pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;

		pLink->m_MaximumServiceFlowRatePHPL= m_DefaultCapacity;
		pLink->m_LaneCapacity  = m_DefaultCapacity;
		pLink->m_link_type= m_DefaultLinkType;

		m_NodeIDMap[FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);
		pLink->m_FromPoint = m_NodeIDMap[FromNodeID]->pt;
		pLink->m_ToPoint = m_NodeIDMap[ToNodeID]->pt;


		if(bOffset)
		{
			double link_offset = m_UnitFeet*m_OffsetInFeet;
			double DeltaX = pLink->m_ToPoint.x - pLink->m_FromPoint.x ;
			double DeltaY = pLink->m_ToPoint.y - pLink->m_FromPoint.y ;
			double theta = atan2(DeltaY, DeltaX);

			pLink->m_FromPoint.x += link_offset* cos(theta-PI/2.0f);
			pLink->m_ToPoint.x += link_offset* cos(theta-PI/2.0f);

			pLink->m_FromPoint.y += link_offset* sin(theta-PI/2.0f);
			pLink->m_ToPoint.y += link_offset* sin(theta-PI/2.0f);
		}
		pLink->m_ShapePoints.push_back(pLink->m_FromPoint);
		pLink->m_ShapePoints.push_back(pLink->m_ToPoint);

		pLink->CalculateShapePointRatios();

		double lane_offset = m_UnitFeet*m_LaneWidthInFeet;  // 20 feet per lane

		unsigned int last_shape_point_id = pLink ->m_ShapePoints .size() -1;
		double DeltaX = pLink->m_ShapePoints[last_shape_point_id].x - pLink->m_ShapePoints[0].x;
		double DeltaY = pLink->m_ShapePoints[last_shape_point_id].y - pLink->m_ShapePoints[0].y;
		double theta = atan2(DeltaY, DeltaX);

		for(unsigned int si = 0; si < pLink ->m_ShapePoints .size(); si++)
		{
			GDPoint pt;

			pt.x = pLink->m_ShapePoints[si].x - lane_offset* cos(theta-PI/2.0f);
			pt.y = pLink->m_ShapePoints[si].y - lane_offset* sin(theta-PI/2.0f);

			pLink->m_BandLeftShapePoints.push_back (pt);

			pt.x  = pLink->m_ShapePoints[si].x + max(1,pLink->m_NumLanes - 1)*lane_offset* cos(theta-PI/2.0f);
			pt.y = pLink->m_ShapePoints[si].y + max(1,pLink->m_NumLanes - 1)*lane_offset* sin(theta-PI/2.0f);

			pLink->m_BandRightShapePoints.push_back (pt);
		}


		m_LinkSet.push_back (pLink);
		m_LinkNoMap[pLink->m_LinkNo]  = pLink;
		return pLink;
	}

	DTANode* AddNewNode(GDPoint newpt, int NewNodeNumber=0 , int LayerNo =0, bool ActivityLocation = false)
	{
		DTANode* pNode = new DTANode;
		pNode->pt = newpt;
		pNode->m_LayerNo = LayerNo;
		pNode->m_NodeID = GetUnusedNodeID();
		if(NewNodeNumber ==0 )
		{
			pNode->m_NodeNumber = GetUnusedNodeNumber();
		}
		else
		{
			pNode->m_NodeNumber = NewNodeNumber;
		}


		pNode->m_ZoneID = 0;
		pNode->m_bZoneActivityLocationFlag = ActivityLocation;
		m_NodeSet.push_back(pNode);
		m_NodeIDMap[pNode->m_NodeID] = pNode;
		m_NodeIDtoNameMap[pNode->m_NodeID ] = pNode->m_NodeNumber;
		m_NodeNametoIDMap[pNode->m_NodeNumber] = pNode->m_NodeID;

		return pNode;
	}

	bool DeleteNode(int NodeID)
	{
		std::list<DTANode*>::iterator iNode;

		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if((*iNode)->m_Connections  == 0 && (*iNode)->m_NodeID  == NodeID)
			{
				m_NodeSet.erase  (iNode);
				return true;
			}
		}

		return false;
	}

	bool DeleteLink(int LinkNo)
	{
		DTALink* pLink = 0;
		pLink = m_LinkNoMap[LinkNo];
		if(pLink == NULL)
			return false;  // a link with the same from and to node numbers exists!

		int FromNodeID   = pLink->m_FromNodeID ;
		int ToNodeID   = pLink->m_ToNodeID ;

		m_NodeIDMap[FromNodeID ]->m_Connections-=1;

		for(int ii = 0; ii< m_NodeIDMap[FromNodeID ]->m_OutgoingLinkVector.size();ii++)
		{
			if(m_NodeIDMap[FromNodeID ]->m_OutgoingLinkVector[ii] == pLink->m_LinkNo)
			{
				m_NodeIDMap[FromNodeID ]->m_OutgoingLinkVector.erase(m_NodeIDMap[FromNodeID ]->m_OutgoingLinkVector.begin()+ii);

				break;
			}
		}

		m_NodeIDMap[ToNodeID ]->m_Connections-=1;
		m_LinkNoMap[pLink->m_LinkNo]  = NULL;

		std::list<DTALink*>::iterator iLink;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_LinkNo == LinkNo)
			{
				m_LinkSet.erase  (iLink);
				break;
			}
		}

		// 
		//resort link no;

		m_LinkNotoLinkMap.clear();

		m_LinkNoMap.clear();
		int i= 0;
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++, i++)
		{
			(*iLink)->m_LinkNo = i;
			m_LinkNotoLinkMap[i] = (*iLink);
			m_LinkNoMap[i] = (*iLink);
		}


		// remove isolated nodes
		DeleteNode (FromNodeID);
		DeleteNode (ToNodeID);

		return true;
	}


	int GetUnusedNodeNumber()
	{
		int NewNodeNumber = 1;

		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if(NewNodeNumber <= (*iNode)->m_NodeNumber)
				NewNodeNumber = (*iNode)->m_NodeNumber +1;
		}

		return NewNodeNumber;

	}

	int GetUnusedNodeID()
	{
		int NewNodeID = 0;

		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if(NewNodeID <= (*iNode)->m_NodeID)
				NewNodeID = (*iNode)->m_NodeID +1;
		}

		return NewNodeID;

	}

	char GetApproachChar(DTA_Approach approach)
	{
		char c;
		switch (approach) 
		{
		case DTA_North: c = 'N'; break;
		case DTA_East: c = 'E'; break;
		case DTA_South: c = 'S'; break;
		case DTA_West: c = 'W'; break;
		default: c = '0'; break;

		}
		return c;
	};

	std::vector<DTA_NodeMovementSet> m_MovementVector;
	std::vector<DTA_NodePhaseSet> m_PhaseVector;

	// 	void ConstructMovementVector(bool flag_Template);
	void ConstructMovementVectorForEachNode();
	void AssignUniqueLinkIDForEachLink();

	// function declaration for Synchro /////////////////////////////////////////////////////////////////////////////////
	void ConstructMovementVector(bool flag_Template);
	bool LoadMovementTemplateFile(DTA_NodeMovementSet& MovementTemplate, DTA_NodePhaseSet& PhaseTemplate);
	bool LoadMovementDefault(DTA_NodeMovementSet& MovementTemplate, DTA_NodePhaseSet& PhaseTemplate);
	void ExportSingleSynchroFile(CString SynchroProjectFile);

	void ExportNodeLayerToGISFiles(CString file_name, CString GIS_type_string);
	void ExportLinkLayerToGISFiles(CString file_name, CString GIS_type_string);
	void ExportZoneLayerToGISFiles(CString file_name, CString GIS_type_string);
	void ExportZoneLayerToKMLFiles(CString file_name, CString GIS_type_string);
	void ExportLink3DLayerToKMLFiles_ColorCode(CString file_name, CString GIS_type_string,int ColorCode, bool no_curve_flag);
	void ExportLink3DLayerToKMLFiles(CString file_name, CString GIS_type_string);
	void ExportLinkDiffLayerToKMLFiles(CString file_name, CString GIS_type_string);

	std::map<CString, PathStatistics> m_PathMap;

	std::map<CString, PathStatistics> m_ODMatrixMap;

	std::map<CString, PathStatistics> m_ODProbeMatrixMap;

	std::map<CString, Movement3Node> m_Movement3NodeMap;  // turnning movement count


	void GeneratePathFromVehicleData();
	void ExportAgentLayerToKMLFiles(CString file_name, CString GIS_type_string);
	void ExportLinkMOEToKMLFiles(CString file_name);
	void SaveAMS_ODTable();

	void ExportPathflowToCSVFiles();

	CString m_GISMessage;

	void ExportSynchroVersion6Files();
	void ExportQEMData(int NodeNumber);
	bool ReadSynchroPreGeneratedLayoutFile(LPCTSTR lpszFileName);
	CString m_Synchro_ProjectDirectory;

	void ExportOGRShapeFile();
	void ImportOGRShapeFile(CString FileName);


	std::map<CString,DTA_Approach> m_PredefinedApproachMap;

	int Find_P2P_Angle(GDPoint p1, GDPoint p2);
	DTA_Turn Find_RelativeAngle_to_Left_OR_Right_Turn(int relative_angle);
	DTA_Turn Find_RelativeAngle_to_Left_OR_Right_Turn_1_OR_2(int relative_angle);


	DTA_Turn Find_RelativeAngle_to_Turn(int relative_angle);

	DTA_Approach g_Angle_to_Approach_New(int angle);
	DTA_Approach Find_Closest_Angle_to_Approach(int angle);

	std::map<DTA_Approach,int> m_ApproachMap;

	std::map<DTA_Approach,DTA_Approach> m_OpposingDirectionMap;



	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::map<unsigned long, DTALink*> m_NodeIDtoLinkMap;
	std::map<__int64, DTALink*> m_NodeNumbertoLinkMap;

	std::map<long, DTALink*> m_LinkNotoLinkMap;
	std::map<long, DTALink*> m_SensorIDtoLinkMap;
	std::map<long, int> m_AVISensorIDtoNodeIDMap;


	int Find_PPP_RelativeAngle(GDPoint p1, GDPoint p2, GDPoint p3);
	DTA_Turn Find_PPP_to_Turn(GDPoint p1, GDPoint p2, GDPoint p3);

	DTA_Turn Find_PPP_to_Turn_with_DTAApproach(GDPoint p1, GDPoint p2, GDPoint p3,DTA_Approach approach1, DTA_Approach approach2);

	DTA_Turn Find_PPP_to_All_Turns_with_DTAApproach(GDPoint p1, GDPoint p2, GDPoint p3,DTA_Approach approach1, DTA_Approach approach2);


	int MaxNodeKey;
	int MaxNode64Key;
	unsigned long GetLinkKey(int FromNodeID, int ToNodeID)
	{

		unsigned long LinkKey = FromNodeID*MaxNodeKey+ToNodeID;
		return LinkKey;
	}

	__int64 GetLink64Key(int FromNodeNumber, int ToNodeNumber)
	{

		__int64 LinkKey = FromNodeNumber*MaxNode64Key+ToNodeNumber;
		return LinkKey;
	}

	DTANode* FindNodeWithNodeNumber(int NodeNumber)
	{
		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if(NodeNumber == (*iNode)->m_NodeNumber )
				return (*iNode);
		}
		return NULL;
	}

	DTALink* FindLinkWithNodeNumbers(int FromNodeNumber, int ToNodeNumber, CString FileName = "", bool bWarmingFlag = false)
	{
		int FromNodeID = m_NodeNametoIDMap[FromNodeNumber];
		int ToNodeID = m_NodeNametoIDMap[ToNodeNumber];

		unsigned long LinkKey = GetLinkKey( FromNodeID, ToNodeID);

		map <unsigned long, DTALink*> :: const_iterator m_Iter = m_NodeIDtoLinkMap.find(LinkKey);

		if(m_Iter == m_NodeIDtoLinkMap.end( ) && bWarmingFlag)
		{
			CString msg;

			if(FileName.GetLength() == 0)
			{
				msg.Format ("Link %d-> %d cannot be found.", FromNodeNumber, ToNodeNumber);
			}else
			{
				msg.Format ("Link %d-> %d cannot be found in file %s.", FromNodeNumber, ToNodeNumber,FileName);
			}
			AfxMessageBox(msg);
			return NULL;
		}
		return m_NodeIDtoLinkMap[LinkKey];
	}

	DTALink* FindLinkWithNodeIDs(int FromNodeID, int ToNodeID)
	{

		unsigned long LinkKey = GetLinkKey( FromNodeID, ToNodeID);
		if(m_NodeIDtoLinkMap.find(LinkKey)!=m_NodeIDtoLinkMap.end())
			return m_NodeIDtoLinkMap[LinkKey];
		else
			return NULL;
	}


	DTALink* FindLinkWithLinkNo(int LinkID)
	{
		return m_LinkNotoLinkMap[LinkID];
	}

	CString GetTimeStampStrFromIntervalNo(int time_interval, bool with_single_quote);
	CString GetTimeStampFloatingPointStrFromIntervalNo(int time_interval);

	CString GetTimeStamString(int time_stamp_in_min);
	int* m_ZoneCentroidSizeAry;  //Number of centroids per zone
	int** m_ZoneCentroidNodeAry; //centroid node Id per zone


	COLORREF m_BackgroundColor;
	COLORREF m_ZoneColor;
	COLORREF m_ZoneTextColor;

	bool m_BackgroundBitmapLoaded;
	bool m_LongLatCoordinateFlag;
	CImage m_BackgroundBitmap;  // background bitmap
	float m_ImageX1,m_ImageX2,m_ImageY1,m_ImageY2, m_ImageWidth, m_ImageHeight;
	float m_ImageXResolution, m_ImageYResolution;
	float m_ImageMoveSize;
	bool m_BackgroundBitmapImportedButnotSaved;

#ifndef _WIN64
	CDaoDatabase m_Database;
#endif

	// Operations
public:
	CString GetTableName(CString Tablename);
	CString ConstructSQL(CString Tablename);

	bool CreateNetworkFromExcelFile();
	bool ImportSensorData();
public: // subarea
	std::vector<GDPoint> m_SubareaShapePoints;

	bool EditTrafficAssignmentOptions();
	void SendTexttoStatusBar(CString str);
	bool SaveDemandFile();

	// Overrides
public:

	void ShowPathListDlg(bool bShowFlag);
	void RunExcelAutomation();
	void OpenCSVFileInExcel(CString filename);
	void Constructandexportsignaldata();
	void ConstructandexportVISSIMdata();
	void ReadSynchroUniversalDataFiles();
	bool ReadSynchroLayoutFile(LPCTSTR lpszFileName);
	bool ReadSynchroLayoutFile_And_AddOutgoingLinks_For_ExternalNodes(LPCTSTR lpszFileName);
	bool ReadSynchroLaneFile(LPCTSTR lpszFileName);

	bool m_bFitNetworkInitialized;
	void CalculateDrawingRectangle(bool NodeLayerOnly = false);

	DWORD ProcessExecute(CString & strCmd, CString & strArgs,  CString & strDir, BOOL bWait);
	DWORD ProcessWait(DWORD PID);

	CString m_ProjectFile;
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	bool m_bExport_Link_MOE_in_input_link_CSF_File;
	BOOL SaveProject(LPCTSTR lpszPathName);

	bool CheckIfFileExsits(LPCTSTR lpszFileName)
	{

	FILE* pFile = NULL;
	fopen_s(&pFile,lpszFileName,"rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);

		if(Length> 10) // minimum size
		return true;
	}		
		return false;

	}

	void CopyDefaultFile(CString DefaultDataFolder,CString CurrentProjectFolder, CString ProjectDirectory, CString FileName)
	{
		if(CheckIfFileExsits(ProjectDirectory+FileName )==false)
		{
			if(CheckIfFileExsits(CurrentProjectFolder+FileName )==true)
				CopyFile(CurrentProjectFolder+FileName, ProjectDirectory+FileName, FALSE);
			else
				CopyFile(DefaultDataFolder+FileName, ProjectDirectory+FileName, FALSE);
		}
	}

	// For demonstration
	CString m_SampleExcelNetworkFile;
	CString m_SampleOutputProjectFile;
	CString m_SampleExcelSensorFile;
	CString m_SampleNGSIMDataFile;

	// Implementation
public:
	virtual ~CTLiteDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileOpen();
	afx_msg void OnFileSaveimagelocation();
	COLORREF GetLinkTypeColor(int LinkType);
	afx_msg void OnShowShowpathmoe();
	afx_msg void OnUpdateShowShowpathmoe(CCmdUI *pCmdUI);
	afx_msg void OnSearchListtrains();
	afx_msg void OnTimetableImporttimetable();
	afx_msg void OnWindow2dview();
	afx_msg void OnFileSaveProject();
	afx_msg void OnFileSaveProjectAs();
	afx_msg void OnEstimationOdestimation();
	afx_msg void OnImageImportbackgroundimage();
	afx_msg void OnFileDataloadingstatus();
	afx_msg void OnMoeVolume();
	afx_msg void OnMoeSpeed();
	afx_msg void OnMoeDensity();
	afx_msg void OnMoeQueuelength();
	afx_msg void OnMoeFuelconsumption();
	afx_msg void OnMoeEmissions();
	afx_msg void OnUpdateMoeVolume(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMoeSpeed(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMoeDensity(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMoeQueuelength(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMoeFuelconsumption(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMoeEmissions(CCmdUI *pCmdUI);
	afx_msg void OnMoeNone();
	afx_msg void OnUpdateMoeNone(CCmdUI *pCmdUI);
	afx_msg void OnToolsCarfollowingsimulation();
	afx_msg void OnToolsPerformtrafficassignment();
	afx_msg void OnMoeVcRatio();
	afx_msg void OnUpdateMoeVcRatio(CCmdUI *pCmdUI);
	afx_msg void OnMoeTraveltime();
	afx_msg void OnUpdateMoeTraveltime(CCmdUI *pCmdUI);
	afx_msg void OnMoeCapacity();
	afx_msg void OnUpdateMoeCapacity(CCmdUI *pCmdUI);
	afx_msg void OnMoeSpeedlimit();
	afx_msg void OnUpdateMoeSpeedlimit(CCmdUI *pCmdUI);
	afx_msg void OnMoeFreeflowtravletime();
	afx_msg void OnUpdateMoeFreeflowtravletime(CCmdUI *pCmdUI);
	afx_msg void OnEditDeleteselectedlink();
	afx_msg void OnImportAgentFile();
	afx_msg void OnImportNgsimFile();

	afx_msg void OnMoeLength();
	afx_msg void OnUpdateMoeLength(CCmdUI *pCmdUI);
	afx_msg void OnEditSetdefaultlinkpropertiesfornewlinks();
	afx_msg void OnUpdateEditSetdefaultlinkpropertiesfornewlinks(CCmdUI *pCmdUI);
	afx_msg void OnToolsProjectfolder();
	afx_msg void OnToolsOpennextaprogramfolder();
	afx_msg void OnMoeNoodmoe();
	afx_msg void OnUpdateMoeNoodmoe(CCmdUI *pCmdUI);
	afx_msg void OnOdtableImportOdTripFile();
	afx_msg void OnToolsEditassignmentsettings();
	afx_msg void OnToolsEditoddemandtable();
	afx_msg void OnSearchLinklist();
	afx_msg void OnMoeVehicle();
	afx_msg void OnUpdateMoeVehicle(CCmdUI *pCmdUI);
	afx_msg void OnToolsViewsimulationsummary();
	afx_msg void OnToolsViewassignmentsummarylog();
	afx_msg void OnHelpVisitdevelopmentwebsite();
	afx_msg void OnToolsRuntrafficassignment();
	afx_msg void OnImportodtripfile3columnformat();
	afx_msg void OnToolsPerformscheduling();
	afx_msg void OnFileChangecoordinatestolong();
	afx_msg void OnToolsExportopmodedistribution();
	afx_msg void OnToolsEnumeratepath();
	afx_msg void OnToolsExporttoHistDatabase();
	afx_msg void OnResearchtoolsExporttodtalitesensordataformat();
	afx_msg void OnScenarioConfiguration();
	afx_msg void OnMoeViewmoes();
	afx_msg void OnImportdataImport();
	afx_msg void OnMoeVehiclepathanalaysis();
	afx_msg void OnFileConstructandexportsignaldata();
	afx_msg void OnFileImportDemandFromCsv();
	afx_msg void OnImportSensorData();
	afx_msg void OnImportLinkmoe();
	afx_msg void OnImportVehiclefile();
	afx_msg void OnLinkmoeEmissions();
	afx_msg void OnUpdateLinkmoeEmissions(CCmdUI *pCmdUI);
	afx_msg void OnLinkmoeReliability();
	afx_msg void OnUpdateLinkmoeReliability(CCmdUI *pCmdUI);
	afx_msg void OnLinkmoeSafety();
	afx_msg void OnUpdateLinkmoeSafety(CCmdUI *pCmdUI);
	afx_msg void OnExportAms();
	afx_msg void OnImportAvi();
	afx_msg void OnImportGps33185();
	afx_msg void OnImportVii();
	afx_msg void OnImportWeather33188();
	afx_msg void OnImportGps();
	afx_msg void OnImportWorkzone();
	afx_msg void OnImportIncident();
	afx_msg void OnImportWeather();
	afx_msg void OnImportPricing();
	afx_msg void OnImportAtis();
	afx_msg void OnImportBus();
	afx_msg void OnLinkAddWorkzone();
	afx_msg void OnLinkAddvms();
	afx_msg void OnImportLinklayerinkml();
	afx_msg void OnEditOffsetlinks();
	afx_msg void OnImportSubarealayerformapmatching();
	afx_msg void OnFileOpenNetworkOnly();
	afx_msg void OnLinkAddlink();
	afx_msg void OnLinkAddhovtoll();
	afx_msg void OnLinkAddhottoll();
	afx_msg void OnLinkAddtolledexpresslane();
	afx_msg void OnLinkConvertgeneralpurposelanetotolledlane();
	afx_msg void OnProjectEdittime();
	afx_msg void OnLinkVehiclestatisticsanalaysis();
	afx_msg void OnSubareaDeletesubarea();
	afx_msg void OnSubareaViewvehiclestatisticsassociatedwithsubarea();
	afx_msg void OnToolsTraveltimereliabilityanalysis();
	afx_msg void OnLinkLinkbar();
	afx_msg void OnImportArcgisshapefile();
	afx_msg void OnLinkIncreaseoffsetfortwo();
	afx_msg void OnLinkDecreaseoffsetfortwo();
	afx_msg void OnLinkNooffsetandnobandwidth();
	afx_msg void OnViewShowhideLegend();
	afx_msg void OnMoeViewlinkmoesummaryfile();
	afx_msg void OnViewCalibrationview();
	afx_msg void OnUpdateViewCalibrationview(CCmdUI *pCmdUI);
	afx_msg void OnMoeViewtrafficassignmentsummaryplot();
	afx_msg void OnMoeViewoddemandestimationsummaryplot();
	afx_msg void OnProjectEditpricingscenariodata();
	afx_msg void OnLinkViewlink();
	afx_msg void OnDeleteSelectedLink();
	afx_msg void OnImportRegionalplanninganddtamodels();
	afx_msg void OnExportGeneratezone();
	afx_msg void OnExportGenerateshapefiles();
	afx_msg void OnLinkmoedisplayQueuelength();
	afx_msg void OnUpdateLinkmoedisplayQueuelength(CCmdUI *pCmdUI);
	afx_msg void OnUpdateLinkmoeTraveltimereliability(CCmdUI *pCmdUI);


	afx_msg void OnMoePathlist();
	afx_msg void OnViewShowmoe();
	afx_msg void OnUpdateViewShowmoe(CCmdUI *pCmdUI);
	afx_msg void OnFileUploadlinkdatatogooglefusiontable();
	afx_msg void On3Viewdatainexcel();
	afx_msg void On5Viewdatainexcel();
	afx_msg void OnMoeViewnetworktimedependentmoe();
	afx_msg void On2Viewdatainexcel33398();
	afx_msg void On2Viewnetworkdata();
	afx_msg void On3Viewoddatainexcel();
	afx_msg void OnMoeOpenallmoetables();
	afx_msg void OnFileOpenNewRailDoc();
	afx_msg void OnBnClickedButtonDatabase();
	afx_msg void OnToolsUnittesting();
	afx_msg void OnViewTraininfo();
	afx_msg void OnImportAmsdataset();
	afx_msg void OnDemandfileOddemandmatrix();
	afx_msg void OnDemandfileHovoddemandmatrix();
	afx_msg void OnDemandfileTruckoddemandmatrix();
	afx_msg void OnDemandfileIntermodaloddemandmatrix();
	afx_msg void OnLinkAddIncident();
	afx_msg void OnToolsGeneratephysicalzonecentroidsonroadnetwork();
	afx_msg void OnImportDemanddataset();
	afx_msg void OnNodeIncreasenodetextsize();
	afx_msg void OnNodeDecreasenodetextsize();
	afx_msg void OnToolsCheckingfeasibility();
	afx_msg void OnToolsGpsmapmatching();
	afx_msg void OnImportSynchroutdfcsvfiles();
	afx_msg void OnProjectEditmoesettings();
	afx_msg void OnProjectMultiScenarioResults();
	afx_msg void OnProject12();
	afx_msg void OnViewMovementMoe();
	afx_msg void OnProjectTimeDependentLinkMoe();
	afx_msg void OnViewOdmeResult();
	afx_msg void OnProjectViewAgentMoe();
	afx_msg void OnProjectOdmatrixestimationinput();
	afx_msg void OnProjectInputsensordataforodme();
	afx_msg void OnHelpUserguide();
	afx_msg void OnToolsGenerateodmatrixgravitymodel();
	afx_msg void OnLinkattributedisplayLinkname();
	afx_msg void OnUpdateLinkattributedisplayLinkname(CCmdUI *pCmdUI);
	afx_msg void OnToolsGeneratesignalcontrollocations();
};
extern std::list<CTLiteDoc*>	g_DocumentList;
extern bool g_TestValidDocument(CTLiteDoc* pDoc);