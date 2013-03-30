// TLiteDoc.h : interface of the CTLiteDoc class
//
//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com), Jinjin Tang ()

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
#include "afx.h"

#include "atlimage.h"
#include "math.h"
#include "Network.h"
#include ".\\cross-resolution-model\\SignalNode.h"
#include ".\\RailNetwork\\RailNetwork.h"


#include <iostream>
#include <fstream>

#ifndef _WIN64
#include <afxdb.h>          // MFC database support
#include <afxdao.h>
#endif
enum _GIS_DATA_TYPE {GIS_Point_Type=0,GIS_Line_Type, GIS_Polygon_Type}; 
enum eSEARCHMODE {efind_node = 0,efind_link,efind_path,efind_vehicle};
enum layer_mode
{ 
	layer_node = 0,
	layer_link,
	layer_movement,
	layer_zone,
	layer_connector,
	layer_ODMatrix,
	layer_link_MOE,
	layer_path, 
	layer_subarea, 
	layer_detector,
	layer_workzone,
	layer_incident,
	layer_VMS,
	layer_toll,
	layer_ramp,
	layer_vehicle_position,
	layer_transit,
	layer_grid,
	layer_background_image,
	layer_transit_accessibility,
};
enum Network_Data_Settings {_NODE_DATA = 0,_LINK_DATA, _ZONE_DATA, _ACTIVITY_LOCATION_DATA,_SENSOR_DATA, _MOVEMENT_DATA,_CALIBRATION_RESULT_DATA,MAX_NUM_OF_NETWORK_DATA_FILES};
enum Corridor_Data_Settings {_CORRIDOR_NODE_DATA = 0,_CORRIDOR_LINK_DATA, _CORRIDOR_SEGMENT_DATA, MAX_NUM_OF_CORRIDOR_DATA_FILES};
enum GIS_IMPORT_Data_Settings {_GIS_IMPORT_NODE_DATA = 0,_GIS_IMPORT_LINK_DATA, _GIS_IMPORT_GIS_LAYER_DATA, MAX_NUM_OF_GIS_IMPORT_DATA_FILES};
enum Link_MOE {MOE_none,MOE_volume, MOE_speed, MOE_queue_length, MOE_safety,MOE_density,MOE_traveltime,MOE_capacity, MOE_speedlimit, MOE_reliability, MOE_fftt, MOE_length, MOE_queuelength,MOE_fuel,MOE_emissions, MOE_vehicle, MOE_volume_copy, MOE_speed_copy, MOE_density_copy};

enum OD_MOE {odnone,critical_volume};

enum VEHICLE_CLASSIFICATION_SELECTION {CLS_network=0, CLS_OD,CLS_link_set,
CLS_sep_c_1,
CLS_path_trip,CLS_path_partial_trip,
CLS_sep_c_2,
CLS_subarea_generated,
CLS_subarea_traversing_through,
CLS_subarea_internal_to_external,
CLS_subarea_external_to_internal,
CLS_subarea_internal_to_internal_trip,
CLS_subarea_internal_to_internal_subtrip,
CLS_subarea_boundary_to_bounary_subtrip};
enum VEHICLE_X_CLASSIFICATION {CLS_all_vehicles=0,CLS_pricing_type,
CLS_sep_1,
CLS_time_interval_15_min,CLS_time_interval_30_min,CLS_time_interval_60_min,CLS_time_interval_2_hour,CLS_time_interval_4_hour,
CLS_sep_2,
CLS_distance_bin_0_2,CLS_distance_bin_1,CLS_distance_bin_2,CLS_distance_bin_5,CLS_distance_bin_10,
CLS_sep_3,
CLS_travel_time_bin_2,CLS_travel_time_bin_5,CLS_travel_time_bin_10,CLS_travel_time_bin_30,
CLS_sep_4,
CLS_VOT_10,CLS_VOT_15,CLS_VOT_10_SOV,CLS_VOT_10_HOV,CLS_VOT_10_truck,
CLS_information_class,CLS_vehicle_type};
enum VEHICLE_Y_CLASSIFICATION {
	CLS_vehicle_count=0,CLS_cumulative_vehicle_count,CLS_total_travel_time,CLS_total_travel_distance, 
	CLS_seperator_1,
	CLS_avg_travel_time,
	CLS_avg_travel_distance,
	CLS_avg_speed,
	CLS_avg_travel_time_per_mile,
	CLS_travel_time_Travel_Time_Index,
	CLS_seperator_2,
	CLS_travel_time_STD,
	CLS_travel_time_95_percentile,
	CLS_travel_time_90_percentile,
	CLS_travel_time_80_percentile,
	CLS_travel_time_Planning_Time_Index,
	CLS_travel_time_Buffer_Index,
	CLS_travel_time_Skew_Index,
	CLS_seperator_3,
	CLS_travel_time_per_mile_STD,
	CLS_travel_time_per_mile_95_percentile,
	CLS_travel_time_per_mile_90_percentile,
	CLS_travel_time_per_mile_80_percentile,
	CLS_seperator_4,
	CLS_total_toll_cost,CLS_avg_toll_cost,
	CLS_total_generalized_travel_time,CLS_avg_generalized_travel_time,
	CLS_seperator_5,
	CLS_total_Energy,
	CLS_total_CO2,
	CLS_total_NOx,
	CLS_total_CO,
	CLS_total_HC,
	CLS_total_gallon,

	CLS_avg_Energy_per_mile,
	CLS_avg_CO2_per_mile,
	CLS_avg_Nox_per_mile,
	CLS_avg_CO_per_mile,
	CLS_avg_HC_per_mile,
	CLS_avg_mile_per_gallon,

	// not used
	CLS_total_generalized_cost,CLS_avg_generalized_cost,
	CLS_avg_Energy,
	CLS_avg_CO2,
	CLS_avg_Nox,
	CLS_avg_CO,
	CLS_avg_HC,
	CLS_avg_mile

};
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
#define _TOTAL_NUMBER_OF_PROJECTS 5


class GridNodeSet
{
public:
	double x;
	double y;

	int x_int;
	int y_int;

	std::vector<int> m_NodeVector;
	std::vector<float> m_NodeX;
	std::vector<float> m_NodeY;

	//for transit
	std::vector<int> m_TripIDVector;
	std::vector<int> m_StopIDVector;

};

class PathStatistics
{
public: 

	std::vector<float> m_TravelTimeVector;
	std::vector<float> m_TravelTimePerMileVector;

	float GetTravelTimeStandardDeviation()
	{
		if( m_TravelTimeVector.size() ==0)
			return 0;

		float total_travel_time = 0 ;
		for(unsigned int i=0; i< m_TravelTimeVector.size(); i++)
		{
		total_travel_time+= m_TravelTimeVector[i];
		}

		float avg_travel_time  = total_travel_time /  m_TravelTimeVector.size();

		float total_variance = 0 ;
		for(unsigned int i=0; i< m_TravelTimeVector.size(); i++)
		{
		total_variance+= (m_TravelTimeVector[i]-avg_travel_time)* (m_TravelTimeVector[i]-avg_travel_time);
		}

		return sqrt(total_variance/m_TravelTimeVector.size());

	}

	float GetTravelTimePerMileStandardDeviation()
	{
		if( m_TravelTimePerMileVector.size() ==0)
			return 0;

		float total_travel_time_per_mile = 0 ;
		for(unsigned int i=0; i< m_TravelTimePerMileVector.size(); i++)
		{
		total_travel_time_per_mile+= m_TravelTimePerMileVector[i];
		}

		float avg_travel_time_per_mile  = total_travel_time_per_mile /  m_TravelTimePerMileVector.size();

		float total_variance = 0 ;
		for(unsigned int i=0; i< m_TravelTimePerMileVector.size(); i++)
		{
		total_variance+= (m_TravelTimePerMileVector[i]-avg_travel_time_per_mile)* (m_TravelTimePerMileVector[i]-avg_travel_time_per_mile);
		}

		return sqrt(total_variance/m_TravelTimePerMileVector.size());

	}


	PathStatistics()
	{
		Accessibility_Factor = 0;
		TotalVehicleSize = 0;
		TotalTravelTime = 0;
		TotalDistance = 0;
		TotalTravelTimeVariance = 0;
		TotalTravelTimePerMileVariance = 0;

		TotalCost = 0;
		TotalEmissions = 0;

		for(int i = 0; i< _TOTAL_NUMBER_OF_PROJECTS; i++)
		{
		 TotalVehicleSizeVector[i] = 0;
		 TotalTravelTimeVector[i] = 0;
		 TotalDistanceVector[i] = 0;
		 TotalCostVector[i] = 0;
		 TotalEmissionsVector[i] = 0;
		}
	}

	float GetAvgTravelTime()
	{
		return TotalTravelTime/max(1,TotalVehicleSize);
	}

	int	  Origin;
	int	  Destination;
	int   NodeNumberSum;
	int   NodeSize;

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


	int date_id;
	float departure_time_in_min;

	float Accessibility_Factor;
	int   TotalVehicleSize;
	float TotalTravelTime;
	float	TotalTravelTimeVariance;
	float	TotalTravelTimePerMileVariance;

	float TotalDistance;
	float TotalCost;
	float TotalEmissions;


	int   TotalVehicleSizeVector[_TOTAL_NUMBER_OF_PROJECTS];
	float TotalTravelTimeVector[_TOTAL_NUMBER_OF_PROJECTS];
	float TotalDistanceVector[_TOTAL_NUMBER_OF_PROJECTS];
	float TotalCostVector[_TOTAL_NUMBER_OF_PROJECTS];
	float TotalEmissionsVector[_TOTAL_NUMBER_OF_PROJECTS];

	CVehicleEmission emissiondata;

};

class Movement3Node
{
public:
	int TotalVehicleSize;
	int Phase1;
	int PermPhase1;
	int DetectPhase1;
	int EffectiveGreen;
	int Capacity;
	float VOC;
	float Delay;
	char LOS;
	float DischargeRate;




	Movement3Node()
	{
		TotalVehicleSize  = 0;
		Phase1 = -1;  // default value
		PermPhase1 = -1;
		DetectPhase1 = -1;

		EffectiveGreen = 0;
		Capacity = 0;
		VOC = 0;
		Delay = 0;
		LOS = 'A';
		DischargeRate = 0;
	}

};

class CTLiteDoc : public CDocument
{
public: // create from serialization only

	
	float m_PeakHourFactor;
	CTLiteDoc();



	DECLARE_DYNCREATE(CTLiteDoc)

	// Attributes
public:

	bool m_bIdentifyBottleneckAndOnOffRamps;
	void IdentifyBottleNeckAndOnOffRamps();
	void Modify(BOOL bModified=true)
{

   SetModifiedFlag(bModified);

   CString string_title = GetTitle();

   if(IsModified()&& string_title.Find(" *") == -1)
   {
      string_title += " *";
   }
   else if(!IsModified() && string_title.Find(" *") != -1)
   {
      string_title.Replace(" *", "");
   }

   SetTitle(string_title);
}

	bool m_bUseMileVsKMFlag;
	double m_ScreenWidth_InMile;
	CString m_LatLongA;
	CString m_LatLongB;


	std::vector<DTA_demand> m_ImportedDemandVector;

	bool m_ImportDemandColumnFormat;

	bool m_bSummaryDialog;
	int m_DocumentNo;
	int m_calibration_data_start_time_in_min;
	int m_calibration_data_end_time_in_min;

	double m_GridXStep;
	double m_GridYStep;

	bool m_bRunCrashPredictionModel;
	
	GridNodeSet m_GridMatrix[100][100];
	GDRect m_GridRect;

	std::map<int,int> m_AccessibleTripIDMap;

	double m_max_walking_distance;
	double m_max_accessible_transit_time_in_min;

	void FindAccessibleTripID(double x, double y);
	int FindClosestNode(double x, double y, double min_distance = 99999, int step_size = 1,double time_stamp_in_min = 9999);
	int FindClosestZone(double x, double y, double min_distance = 99999, int step_size = 1);


	std::vector <int> m_ZoneIDVector;
	std::vector <int> m_ZoneNumberVector;

	VehicleStatistics*** m_ODMOEMatrix;

	int m_DemandTypeSize; 

	int m_PreviousDemandTypeSize;
	int m_PreviousZoneNoSize;


	void ResetZoneIDVector();
	void ResetODMOEMatrix();

	void ShowTextLabel();

	void BuildGridSystem();

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
	bool m_LongLatFlag;
	int m_StartNodeNumberForNewNodes;
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
	int m_PreviousODSize;
	int m_ZoneNoSize;
	Link_MOE m_LinkMOEMode;
	Link_MOE m_PrevLinkMOEMode;

	OD_MOE m_ODMOEMode;

	std::vector<DTAPath>	m_PathDisplayList;
	bool m_PathMOEDlgShowFlag;
	int m_SelectPathNo;

	int m_OriginNodeID;
	int m_DestinationNodeID;

	double m_UnitFeet, m_UnitMile;
	bool m_bUnitMileInitialized;  // if UnitMile has not been initialized, then we default to long /lat 
	double m_OffsetInFeet;

	bool m_bLoadNetworkDataOnly;

	COLORREF m_ColorFreeway, m_ColorHighway, m_ColorArterial;

	ofstream m_AMSLogFile;

	BOOL OnOpenDocument(CString FileName, bool bLoadNetworkOnly =false);
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
	int m_YCorridonateFlag;
	bool m_bGPSDataSet;
	bool m_bEmissionDataAvailable;

	BOOL OnOpenRailNetworkDocument(LPCTSTR lpszPathName);

	// Open Graph drawing framework 
	void OGDF_WriteGraph(CString FileName);


	std::ofstream m_WarningFile;

	void OpenWarningLogFile(CString directory);
	// two basic input
	bool ReadNodeControlTypeCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadNodeCSVFile(LPCTSTR lpszFileName, int LayerNo=0);   // for road network
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
	bool TransitTripMatching();

	void OffsetLink();
	bool m_bBezierCurveFlag;

	void PerformPathTravelTimeReliabilityAnalysis();
	void GenerateOffsetLinkBand();

	void ReCalculateLinkBandWidth();
	bool ReadZoneCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadActivityLocationCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadDemandCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadMetaDemandCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadScenarioSettingCSVFile(LPCTSTR lpszFileName);   // for road network
	bool WriteScenarioSettingCSVFile(LPCTSTR lpszFileName);



	BOOL ReadDYNASMARTSimulationResults();
	void RecalculateLinkMOEFromVehicleTrajectoryFile();
	bool ReadSubareaCSVFile(LPCTSTR lpszFileName);
	void GenerateSubareaStatistics();
	bool ReadVOTCSVFile(LPCTSTR lpszFileName);  
	bool ReadTemporalDemandProfileCSVFile(LPCTSTR lpszFileName);  
	bool ReadVehicleTypeCSVFile(LPCTSTR lpszFileName);  
	bool ReadDemandTypeCSVFile(LPCTSTR lpszFileName);
	bool ReadLinkTypeCSVFile(LPCTSTR lpszFileName); 

	bool ReadScenarioData();   // for road network

	//scenario data

	int m_number_of_assignment_days;
	int m_traffic_flow_model;
	int m_traffic_assignment_method;

	int m_agent_demand_input_mode;
	int m_emission_data_output;
	int m_ODME_mode;
	float m_demand_multiplier;

	int m_NumberOfSecenarioSettings;

	//

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


	int ReadVMSScenarioData(int RemoveLinkFromNodeNumber= -1, int RemoveLinkToNodeNumber= -1);
	int ReadWorkZoneScenarioData(int RemoveLinkFromNodeNumber= -1, int RemoveLinkToNodeNumber= -1);
	int ReadIncidentScenarioData(int RemoveLinkFromNodeNumber= -1, int RemoveLinkToNodeNumber= -1);
	int ReadLink_basedTollScenarioData();

	bool WriteLink_basedTollScenarioData();
	bool WriteVMSScenarioData();
	bool WriteIncidentScenarioData();
	bool WriteWorkZoneScenarioData();
	bool WriteCapacityReductionScenarioDataFromSubareaLinks(CString Scenario_File_Name);

	bool ReadNodeGeoFile(LPCTSTR lpszFileName); 
	bool ReadLinkGeoFile(LPCTSTR lpszFileName);
	bool ReadZoneGeoFile(LPCTSTR lpszFileName); 

	bool ReadZoneShapeCSVFile(LPCTSTR lpszFileName); 

	bool ReadTripTxtFile(LPCTSTR lpszFileName);  
	bool Read3ColumnTripTxtFile(LPCTSTR lpszFileName);  

	// additional input
	void LoadSimulationOutput();
	void LoadGPSData();
	void ReadSimulationLinkMOEData(LPCTSTR lpszFileName);
	void ReadSimulationLinkMOEData_Parser(LPCTSTR lpszFileName);
	bool ReadSimulationLinkMOEData_Bin(LPCTSTR lpszFileName);
	void ReadTMCSpeedData(LPCTSTR lpszFileName); 
	bool ReadSimulationLinkOvarvallMOEData(LPCTSTR lpszFileName);
	void ReadObservationLinkVolumeData(LPCTSTR lpszFileName);

	bool ReadTimetableCVSFile(LPCTSTR lpszFileName);
	void ReadHistoricalData(CString directory);

	int m_SamplingTimeInterval;
	int m_AVISamplingTimeInterval;
	int m_NumberOfDays;
	int m_SimulationStartTime_in_min;
	int m_SimulationEndTime_in_min;


	bool ReadSensorData(LPCTSTR lpszFileName, int simulation_start_time_in_min = 0);
	void ReadEventData(CString directory);
	void BuildHistoricalDatabase();

	bool ReadMultiDaySensorData(LPCTSTR lpszFileName);
	bool ReadInputEmissionRateFile(LPCTSTR lpszFileName);


	std::string CString2StdString(CString str)
	{	 // Convert a TCHAR string to a LPCSTR
	  CT2CA pszConvertedAnsiString (str);

	  // construct a std::string using the LPCSTR input
	  std::string strStd (pszConvertedAnsiString);

	  return strStd;
	  }
	CEmissionRate EmissionRateData[MAX_VEHICLE_TYPE_SIZE][_MAXIMUM_OPERATING_MODE_SIZE];


	std::vector <CString> m_MessageStringVector;

	CString m_NodeDataLoadingStatus;
	CString m_LinkDataLoadingStatus;
	CString m_ConnectorDataLoadingStatus;
	CString m_ZoneDataLoadingStatus;
	CString m_DemandDataLoadingStatus;
	CString m_ScenarioDataLoadingStatus;

	CString m_LinkTrainTravelTimeDataLoadingStatus;
	CString m_TimetableDataLoadingStatus;

	CString m_SimulatedLinkVolumeStatus;
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
	bool GetGPSVehilePosition(DTAVehicle* pVehicle, double CurrentTime, GDPoint & pt);

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

	CString GetTurnDirectionString(DTA_APPROACH_TURN turn_dir)
	{
		CString str;
		switch (turn_dir)
		{
		case DTA_LANES_COLUME_init: str.Format("N/A"); break;
		case DTA_NBL2: str.Format("NBL2"); break;
		case DTA_NBL: str.Format("NBL"); break;
		case DTA_NBT: str.Format("NBT"); break;
		case DTA_NBR: str.Format("NBR"); break;
		case DTA_NBR2: str.Format("NBR2"); break;
		case DTA_SBL2: str.Format("SBL2"); break;
		case DTA_SBL: str.Format("SBL"); break;
		case DTA_SBT: str.Format("SBT"); break;
		case DTA_SBR: str.Format("SBR"); break;
		case DTA_SBR2: str.Format("SBR2"); break;
		case DTA_EBL2: str.Format("EBL2"); break;
		case DTA_EBL: str.Format("EBL"); break;
		case DTA_EBT: str.Format("EBT"); break;
		case DTA_EBR: str.Format("EBR"); break;
		case DTA_EBR2: str.Format("EBR2"); break;
		case DTA_WBL2: str.Format("WBL2"); break;
		case DTA_WBL: str.Format("WBL"); break;
		case DTA_WBT: str.Format("WBT"); break;
		case DTA_WBR: str.Format("WBR"); break;
		case DTA_WBR2: str.Format("WBR2"); break;
		case DTA_NEL: str.Format("NEL"); break;
		case DTA_NET: str.Format("NET"); break;
		case DTA_NER: str.Format("NER"); break;
		case DTA_NWL: str.Format("NWL"); break;
		case DTA_NWT: str.Format("NWT"); break;
		case DTA_NWR: str.Format("NWR"); break;
		case DTA_SEL: str.Format("SEL"); break;
		case DTA_SET: str.Format("SET"); break;
		case DTA_SER: str.Format("SER"); break;
		case DTA_SWL: str.Format("SWL"); break;
		case DTA_SWT: str.Format("SWT"); break;
		case DTA_SWR: str.Format("SWR"); break;

		default :  str.Format("N/A");
		}

		return str;
	}
	int GetLOSCode(float Value)
	{

		for(int los = 1; los < MAX_LOS_SIZE-1; los++)
		{
			if( (m_LOSBound[m_LinkMOEMode][los] <= Value && Value < m_LOSBound[m_LinkMOEMode][los+1]) ||
				(m_LOSBound[m_LinkMOEMode][los] >= Value && Value > m_LOSBound[m_LinkMOEMode][los+1]))

				return los;
		}
		return 1;
	}

public:

	void SetStatusText(CString StatusText);

	std::list<DTANode*>		m_NodeSet;
	std::list<DTALink*>		m_LinkSet;

	std::list<DTAPoint*>	m_DTAPointSet;
	std::list<DTALine*>		m_DTALineSet;


	std::vector<DTANode*>		m_SubareaNodeSet;
	std::list<DTALink*>		m_SubareaLinkSet;

	bool m_bSaveProjectFromSubareaCut;

	std::map<int, DTAZone>	m_ZoneMap;
	int m_ActivityLocationCount;
	std::vector<CString> m_DemandFileVector;
	int m_CriticalOriginZone;
	int m_CriticalDestinationZone;

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
	std::map<int, DTANode*> m_NodeNumberMap;

	std::map<int, DTANode*> m_SubareaNodeIDMap;
	bool CTLiteDoc::WriteSubareaFiles();

	std::map<long, DTALink*> m_LinkNoMap;


	bool m_EmissionDataFlag;

	int m_AdjLinkSize;

	DTANetworkForSP* m_pNetwork;
	DTANetworkForSP m_Network;

	float m_RandomRoutingCoefficient;

	std::vector<int> m_IntermediateDestinationVector;

	int Routing(bool bCheckConnectivity, bool bRebuildNetwork = true);
	int AlternativeRouting(int NumberOfRoutes);

	CString GetWorkspaceTitleName(CString strFullPath);
	CString m_ProjectTitle;

	float FillODMatrixFromCSVFile(LPCTSTR lpszFileName);
	void AdjustCoordinateUnitToMile();

	void ReadTrainProfileCSVFile(LPCTSTR lpszFileName);
	void ReadVehicleCSVFile_Parser(LPCTSTR lpszFileName);
	void ReadAMSPathCSVFile(LPCTSTR lpszFileName);
	int ReadAMSMovementCSVFile(LPCTSTR lpszFileName, int NodeNumber);

	void ReadVehicleCSVFile(LPCTSTR lpszFileName);
	bool ReadVehicleBinFile(LPCTSTR lpszFileName,int version_number);

	bool ReadAimCSVFiles(LPCTSTR lpszFileName, int date_id);
	bool ReadGPSBinFile(LPCTSTR lpszFileName, int date_id,int max_GPS_data_count);
	bool ReadDYNASMARTVehicleTrajectoryFile(LPCTSTR lpszFileName, int date_id);
	bool ReadDYNASMART_ControlFile();
	bool ReadDYNASMART_ControlFile_ForAMSHub();
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

	bool ReadBackgroundImageFile(LPCTSTR lpszFileName, bool bAskForInput = false);
	int m_PathNodeVectorSP[MAX_NODE_SIZE_IN_A_PATH];
	long m_NodeSizeSP;

	std::map<int, int> m_VehicleType2PricingTypeMap;
	std::map<int, int> m_NodeIDtoNumberMap;
	std::map<int, int> m_NodeNumbertoIDMap;
	std::map<int, int> m_NodeIDtoZoneNameMap;

	int m_SelectedLinkNo;
	bool m_ZoomToSelectedObject;
	void ZoomToSelectedLink(int SelectedLinkNo);
	void ZoomToSelectedNode(int SelectedNodeNumber);

	int m_SelectedNodeID;
	int m_SelectedZoneID;
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

	int m_LinkTypeFreeway;
	int m_LinkTypeArterial;
	int m_LinkTypeHighway;



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
	DTALink* AddNewLinkWithNodeNumbers(int FromNodeNumber, int ToNodeNumber, bool bOffset = false, bool bLongLatFlag = false)	
	{
		int FromNodeID =  -1;
			if (m_NodeNumberMap.find(FromNodeNumber)!= m_NodeNumberMap.end())
			{
			FromNodeID = m_NodeNumberMap[FromNodeNumber]->m_NodeID ;
			}

		int ToNodeID = -1;
			if (m_NodeNumberMap.find(ToNodeNumber)!= m_NodeNumberMap.end())
			{
			ToNodeID = m_NodeNumberMap[ToNodeNumber]->m_NodeID ;
			}

			if(FromNodeID>=0 && ToNodeID>=0)
			{
				return AddNewLink(FromNodeID,ToNodeID,bOffset,bLongLatFlag);
			}else

				return NULL;
			 
	}

		DTALink* AddNewLink(int FromNodeID, int ToNodeID, bool bOffset = false, bool bLongLatFlag = false)
	{
		Modify();
		DTALink* pLink = 0;

		pLink = FindLinkWithNodeIDs(FromNodeID,ToNodeID);

			if(pLink != NULL)
				return NULL;  // a link with the same from and to node numbers exists!

		pLink = new DTALink(1);
		pLink->m_AVISensorFlag = false;
		pLink->m_LinkNo = (int)(m_LinkSet.size());
		pLink->m_FromNodeNumber = m_NodeIDtoNumberMap[FromNodeID];
		pLink->m_ToNodeNumber = m_NodeIDtoNumberMap[ToNodeID];
		pLink->m_FromNodeID = FromNodeID;
		pLink->m_ToNodeID= ToNodeID;

		pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
		pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;

		m_NodeIDMap[FromNodeID ]->m_Connections+=1;

		m_NodeIDMap[FromNodeID ]->m_OutgoingLinkVector.push_back(pLink->m_LinkNo);

		m_NodeIDMap[ToNodeID ]->m_Connections+=1;

		if( m_LinkTypeMap[pLink->m_link_type].IsFreeway () ||  m_LinkTypeMap[pLink->m_link_type].IsRamp  ())
		{
		m_NodeIDMap[pLink->m_FromNodeID ]->m_bConnectedToFreewayORRamp = true;
		m_NodeIDMap[pLink->m_ToNodeID ]->m_bConnectedToFreewayORRamp = true;
		}


		unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
		m_NodeIDtoLinkMap[LinkKey] = pLink;

		__int64  LinkKey2 = GetLink64Key(pLink-> m_FromNodeNumber,pLink->m_ToNodeNumber);
		m_NodeNumbertoLinkMap[LinkKey2] = pLink;

		pLink->m_NumberOfLanes= m_DefaultNumLanes;
		pLink->m_SpeedLimit= m_DefaultSpeedLimit;
		pLink->m_ReversedSpeedLimit = m_DefaultSpeedLimit;
		pLink->m_avg_simulated_speed = m_DefaultSpeedLimit;

		double length  = pLink->DefaultDistance()/max(0.0000001,m_UnitMile);

		if(bLongLatFlag || m_LongLatFlag)
		{  // bLongLatFlag is user input,  m_LongLatFlag is the system input from the project file 
			length  =  g_CalculateP2PDistanceInMileFromLatitudeLongitude(pLink->m_FromPoint , pLink->m_ToPoint);
			m_UnitMile = 1.0/62;
			m_UnitFeet = 1.0/62/5280;
		}
		else 
			length  = pLink->DefaultDistance()/max(0.0000001,m_UnitMile);
		pLink->m_Length = max(0.00001,length);  // alllow mimum link length
		pLink->m_FreeFlowTravelTime = pLink->m_Length / pLink->m_SpeedLimit *60.0f;
		pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;

		pLink->m_MaximumServiceFlowRatePHPL= m_DefaultCapacity;
		pLink->m_LaneCapacity  = m_DefaultCapacity;
		pLink->m_link_type= m_DefaultLinkType;

		m_NodeIDMap[FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumberOfLanes);
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

			pt.x  = pLink->m_ShapePoints[si].x + max(1,pLink->m_NumberOfLanes - 1)*lane_offset* cos(theta-PI/2.0f);
			pt.y = pLink->m_ShapePoints[si].y + max(1,pLink->m_NumberOfLanes - 1)*lane_offset* sin(theta-PI/2.0f);

			pLink->m_BandRightShapePoints.push_back (pt);
		}


		m_LinkSet.push_back (pLink);
		m_LinkNoMap[pLink->m_LinkNo]  = pLink;

		return pLink;
	}

	
	void SplitLinksForOverlappingNodeOnLinks(int ThisNodeID, bool bOffset = false, bool bLongLatFlag = false)
	{
		std::vector<DTALink*> OverlappingLinks;

		GDPoint p0 = m_NodeIDMap[ThisNodeID]->pt ;

		// step 1: find overlapping links

		double threshold_in_feet = max(150,m_ScreenWidth_InMile*5280/600); // 150 feet, 50 meters or 1/600 screen width, about 2 pixels given a 1200 pixels for screen width
		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		for(int si = 0; si < (*iLink) ->m_ShapePoints .size()-1; si++)
		{

			GDPoint pfrom, pto;
			pfrom = (*iLink)->m_ShapePoints[si];
			pto =  (*iLink)->m_ShapePoints[si+1];


			float distance_in_feet =  g_GetPoint2LineDistance(p0, pfrom, pto,m_UnitMile)/max(0.0000001,m_UnitFeet);

			if(distance_in_feet< threshold_in_feet) 
			{
			OverlappingLinks.push_back((*iLink));
			}

		}
	}

		//step 2: create new links

		for(int i = 0; i < OverlappingLinks.size(); i++)
		{

		int ExistingFromNodeID = OverlappingLinks[i]->m_FromNodeID ;
		int ExistingToNodeID = OverlappingLinks[i]->m_ToNodeID ;


		for(int add_link_index = 1; add_link_index <=2; add_link_index++)
		{
		int FromNodeID, ToNodeID;
		
		if(add_link_index==1)  // first link
		{
		FromNodeID= ExistingFromNodeID;
		ToNodeID = ThisNodeID;
		}
		if(add_link_index==2) // second link
		{
		FromNodeID= ThisNodeID;
		ToNodeID = ExistingToNodeID;
		}


		DTALink* pLink = 0;

		pLink = FindLinkWithNodeIDs(FromNodeID,ToNodeID);

		if(pLink != NULL)
				continue;  // a link with the same from and to node numbers exists!

		pLink = new DTALink(1);
		pLink->m_AVISensorFlag = false;
		pLink->m_LinkNo = (int)(m_LinkSet.size());
		pLink->m_FromNodeNumber = m_NodeIDtoNumberMap[FromNodeID];
		pLink->m_ToNodeNumber = m_NodeIDtoNumberMap[ToNodeID];
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

		pLink->m_NumberOfLanes= OverlappingLinks[i]->m_NumberOfLanes ;
		pLink->m_SpeedLimit=  OverlappingLinks[i]->m_SpeedLimit ;
		pLink->m_ReversedSpeedLimit =  OverlappingLinks[i]->m_SpeedLimit ;
		pLink->m_avg_simulated_speed =  OverlappingLinks[i]->m_SpeedLimit ;

		double length  = pLink->DefaultDistance()/max(0.0000001,m_UnitMile);

		if(bLongLatFlag || m_LongLatFlag)  // bLongLatFlag is user input,  m_LongLatFlag is the system input from the project file 
			length  =  g_CalculateP2PDistanceInMileFromLatitudeLongitude(pLink->m_FromPoint , pLink->m_ToPoint);
		else 
			length  = pLink->DefaultDistance()/max(0.0000001,m_UnitMile);

		pLink->m_Length = max(0.00001,length);  // alllow mimum link length
		pLink->m_FreeFlowTravelTime = pLink->m_Length / pLink->m_SpeedLimit *60.0f;
		pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;

		pLink->m_MaximumServiceFlowRatePHPL= OverlappingLinks[i]->m_MaximumServiceFlowRatePHPL ;
		pLink->m_LaneCapacity  = OverlappingLinks[i]->m_LaneCapacity;
		pLink->m_link_type= OverlappingLinks[i]->m_link_type;

		m_NodeIDMap[FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumberOfLanes);
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

			pt.x  = pLink->m_ShapePoints[si].x + max(1,pLink->m_NumberOfLanes - 1)*lane_offset* cos(theta-PI/2.0f);
			pt.y = pLink->m_ShapePoints[si].y + max(1,pLink->m_NumberOfLanes - 1)*lane_offset* sin(theta-PI/2.0f);

			pLink->m_BandRightShapePoints.push_back (pt);
		}


		m_LinkSet.push_back (pLink);
		m_LinkNoMap[pLink->m_LinkNo]  = pLink;

		}

		}

	// step 3: delete overlapping links
			
		for(int i = 0; i < OverlappingLinks.size(); i++)
		{

			DeleteLink(OverlappingLinks[i]);  // delete link according to this link's pointer
		}

	
	}
	

	DTANode* AddNewNode(GDPoint newpt, int NewNodeNumber=0 , int LayerNo =0, bool ActivityLocation = false, bool bSplitLink = false)
	{
		Modify();
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
		m_NodeNumberMap[pNode->m_NodeNumber] = pNode;
		m_NodeIDtoNumberMap[pNode->m_NodeID ] = pNode->m_NodeNumber;
		m_NodeNumbertoIDMap[pNode->m_NodeNumber] = pNode->m_NodeID;

		if(bSplitLink)
		{
		SplitLinksForOverlappingNodeOnLinks(pNode->m_NodeID,false,false);
		}
		return pNode;
	}

	bool DeleteNode(int NodeID)
	{
		std::list<DTANode*>::iterator iNode;

		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if((*iNode)->m_Connections  == 0 && (*iNode)->m_NodeID  == NodeID)
			{

				int ZoneID = (*iNode)->m_ZoneID;

				m_ZoneMap[ZoneID].RemoveNodeActivityMode ((*iNode)->m_NodeNumber);
				
				m_NodeIDMap[(*iNode)->m_NodeID ] = NULL;
				m_NodeNumbertoIDMap[(*iNode)->m_NodeNumber  ] = -1;

				m_NodeSet.erase  (iNode);
				return true;
			}
		}

		return false;
	}

	bool DeleteLink(DTALink* pLink)
	{
		int FromNodeID   = pLink->m_FromNodeID ;
		int ToNodeID   = pLink->m_ToNodeID ;
		unsigned long LinkKey = GetLinkKey( FromNodeID , ToNodeID );
		
		m_NodeIDtoLinkMap[LinkKey] =NULL;

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
			if((*iLink) == pLink)
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

	bool DeleteLink(int LinkNo)
	{
		DTALink* pLink = m_LinkNoMap[LinkNo];

		if(pLink == NULL)
			return false;  // a link with the same from and to node numbers exists!

		return DeleteLink(pLink);
	}


	int GetUnusedNodeNumber()
	{
		int NewNodeNumber = m_StartNodeNumberForNewNodes;

		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if(NewNodeNumber <= (*iNode)->m_NodeNumber  )  // this node number has been used
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

	char GetApproachChar(DTA_Direction approach)
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
	void Construct4DirectionMovementVector(bool ResetFlag = false);
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
	void ExportLink3DLayerToKMLFiles_ColorCode(CString file_name, CString GIS_type_string,int ColorCode, bool no_curve_flag, int default_height);
	void ExportLink3DLayerToKMLFiles(CString file_name, CString GIS_type_string);
	void ExportPathLink3DLayerToKMLFiles(CString file_name, CString GIS_type_string);
	void ExportLinkDiffLayerToKMLFiles(CString file_name, CString GIS_type_string);

	std::map<CString, PathStatistics> m_PathMap;

	std::map<CString, PathStatistics> m_ODMatrixMap;

	std::map<CString, PathStatistics> m_ODProbeMatrixMap;

	std::map<CString, Movement3Node> m_Movement3NodeMap;  // turnning movement count

	std::map<CString, int> m_LinkFlowProportionMap;  // link-turnning movement count (i,j,tau, from node, to node, destination node)
	std::map<CString, int> m_LinkFlowProportionODMap;  // link-turnning movement count (for each OD, time pair: i,j,tau)


	std::map<CString, DTANodeMovement*> m_MovementPointerMap;  // turnning movement pointer

	void GeneratePathFromVehicleData();
	void ExportAgentLayerToKMLFiles(CString file_name, CString GIS_type_string);
	void ExportLinkMOEToKMLFiles(CString file_name);
	void ConvertLinkCSV2ShapeFiles(LPCTSTR lpszCSVFileName,LPCTSTR lpszShapeFileName, CString GISTypeString, _GIS_DATA_TYPE GIS_data_type);
	void SaveAMS_ODTable();

	void ExportPathflowToCSVFiles();
	std::map<std::string, DTALink*>  m_TMC2LinkMap;
	CString m_GISMessage;

	void ExportSynchroVersion6Files();
	bool m_bMovementAvailableFlag;
	void ExportQEMData(int NodeNumber);
	bool ReadSynchroPreGeneratedLayoutFile(LPCTSTR lpszFileName);
	CString m_Synchro_ProjectDirectory;

	void ExportOGRShapeFile();

	bool m_ImportNetworkAlready;
	void ImportOGRShapeFile(CString FileName);


	std::map<CString,DTA_Direction> m_PredefinedApproachMap;

	int Find_P2P_Angle(GDPoint p1, GDPoint p2);
	double Find_P2P_Distance(GDPoint p1, GDPoint p2);
	DTA_Turn Find_RelativeAngle_to_Left_OR_Right_Turn(int relative_angle);
	DTA_Turn Find_RelativeAngle_to_Left_OR_Right_Turn_1_OR_2(int relative_angle);


	DTA_Turn Find_RelativeAngle_to_Turn(int relative_angle);

	DTA_Direction g_Angle_to_Approach_8_direction(int angle);
	DTA_Direction g_Angle_to_Approach_4_direction(int angle);
	
	DTA_Direction Find_Closest_Angle_to_Approach(int angle);

	std::map<DTA_Direction,int> m_ApproachMap;

	std::map<DTA_Direction,DTA_Direction> m_OpposingDirectionMap;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::map<unsigned long, DTALink*> m_NodeIDtoLinkMap;
	std::map<__int64, DTALink*> m_NodeNumbertoLinkMap;

	std::map<long, DTALink*> m_LinkNotoLinkMap;
	std::map<long, DTALink*> m_LinkIDtoLinkMap;

	std::map<long, DTALink*> m_SensorIDtoLinkMap;
	std::map<long, int> m_AVISensorIDtoNodeIDMap;


	int Find_PPP_RelativeAngle(GDPoint p1, GDPoint p2, GDPoint p3);
	DTA_Turn Find_PPP_to_Turn(GDPoint p1, GDPoint p2, GDPoint p3);

	DTA_Turn Find_PPP_to_Turn_with_DTAApproach(GDPoint p1, GDPoint p2, GDPoint p3,DTA_Direction approach1, DTA_Direction approach2);

	DTA_Turn Find_PPP_to_All_Turns_with_DTAApproach(GDPoint p1, GDPoint p2, GDPoint p3,DTA_Direction approach1, DTA_Direction approach2);


	eSEARCHMODE m_SearchMode;
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
		if(m_NodeSet.size()==0)
			return NULL;
		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if(NodeNumber == (*iNode)->m_NodeNumber )
				return (*iNode);
		}
		return NULL;
	}

		DTANode* FindSignalNodeWithCoordinate(double x, double y, int LayerNo = 0, double min_distance = 9999999)
	{
		
		DTANode* pNode= NULL;

		int NodeID = -1;
		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if((*iNode)->m_LayerNo == LayerNo && (*iNode)->m_ControlType == m_ControlType_PretimedSignal)
			{
			double distance = sqrt( ((*iNode)->pt.x - x)*((*iNode)->pt.x - x) + ((*iNode)->pt.y - y)*((*iNode)->pt.y - y));
			if( distance <  min_distance)
			{
				min_distance= distance;
				pNode = (*iNode);
			}
			}
		}
		return pNode;
	}

	int FindNodeNumberWithCoordinate(double x, double y, double min_distance = 0.0000001)
	{
		
		DTANode* pNode= NULL;

		min_distance = 0.00000001;
		int NodeID = -1;
		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{

			double distance = sqrt( ((*iNode)->pt.x - x)*((*iNode)->pt.x - x) + ((*iNode)->pt.y - y)*((*iNode)->pt.y - y));
			if( distance <  min_distance)
			{
				min_distance= distance;
				pNode = (*iNode);
			}
		}
		if(pNode != NULL)
			return pNode->m_NodeNumber;
		else
			return NULL;
	}

	int FindNodeIDWithCoordinate(double x, double y, double min_distance = 0.0000001)
	{
		
		DTANode* pNode= NULL;

		min_distance = 0.00000001;
		int NodeID = -1;
		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{

			double distance = sqrt( ((*iNode)->pt.x - x)*((*iNode)->pt.x - x) + ((*iNode)->pt.y - y)*((*iNode)->pt.y - y));
			if( distance <  min_distance)
			{
				min_distance= distance;
				pNode = (*iNode);
			}
		}
		if(pNode != NULL)
			return pNode->m_NodeID;
		else
			return NULL;
	}


	int FindNonCentroidNodeNumberWithCoordinate(double x, double y)
	{
		
		DTANode* pNode= NULL;

		double min_distance = 999999;
		int NodeID = -1;
		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if((*iNode)->m_bCreatedbyNEXTA == true)
			{
			double distance = sqrt( ((*iNode)->pt.x - x)*((*iNode)->pt.x - x) + ((*iNode)->pt.y - y)*((*iNode)->pt.y - y));
			if( distance <  min_distance)
			{
				min_distance= distance;
				pNode = (*iNode);
			}
			}
		}
		if(pNode != NULL)
			return pNode->m_NodeNumber;
		else
			return NULL;
	}

	DTALink* FindFreewayLinkWithFromNodeNumber(int FromNodeNumber)
	{

		if(m_NodeNumberMap.find(FromNodeNumber)!= m_NodeNumberMap.end())
		{
		
		DTANode* pFromNode = m_NodeNumberMap[FromNodeNumber];

		for(unsigned int i = 0; i< pFromNode->m_OutgoingLinkVector.size(); i++)
		{
			
			DTALink* pLink = m_LinkNoMap[ pFromNode->m_OutgoingLinkVector[i]];

			if(pLink->m_FromNodeNumber == FromNodeNumber && m_LinkTypeMap[pLink->m_link_type].IsFreeway() )
				return pLink;
		
		}
		
		}
		return NULL;
	}
	DTALink* FindFreewayLinkWithToNodeNumber(int ToNodeNumber)
	{

		if(m_NodeNumberMap.find(ToNodeNumber)!= m_NodeNumberMap.end())
		{
		
		DTANode* pToNode = m_NodeNumberMap[ToNodeNumber];

		for(unsigned int i = 0; i< pToNode->m_IncomingLinkVector.size(); i++)
		{
			
			DTALink* pLink = m_LinkNoMap[pToNode->m_IncomingLinkVector[i]];

			if(pLink->m_ToNodeNumber == ToNodeNumber && m_LinkTypeMap[pLink->m_link_type].IsFreeway() )
				return pLink;
		
		}
		
		}
		return NULL;
	}

	DTALink* FindLinkWithNodeNumbers(int FromNodeNumber, int ToNodeNumber, CString FileName = "", bool bWarmingFlag = false)
	{
		if(m_NodeNumberMap.find(FromNodeNumber)!= m_NodeNumberMap.end())
		{
		
		DTANode* pFromNode = m_NodeNumberMap[FromNodeNumber];

		for(unsigned int i = 0; i< pFromNode->m_OutgoingLinkVector.size(); i++)
		{
			
			DTALink* pLink = m_LinkNoMap[pFromNode->m_OutgoingLinkVector[i]];

			if(pLink->m_ToNodeNumber == ToNodeNumber)
				return pLink;
		
		}
		}
			CString msg;

			if(FileName.GetLength() == 0)
			{
							return NULL;

			}else
			{
			msg.Format ("Link %d-> %d cannot be found in file %s.", FromNodeNumber, ToNodeNumber,FileName);
			AfxMessageBox(msg);
			return NULL;
			}

	}

	//DTALink* FindLinkWithNodeNumbers(int FromNodeNumber, int ToNodeNumber, CString FileName = "", bool bWarmingFlag = false)
	//{
	//	int FromNodeID = m_NodeNumbertoIDMap[FromNodeNumber];
	//	int ToNodeID = m_NodeNumbertoIDMap[ToNodeNumber];

	//	unsigned long LinkKey = GetLinkKey( FromNodeID, ToNodeID);

	//	map <unsigned long, DTALink*> :: const_iterator m_Iter = m_NodeIDtoLinkMap.find(LinkKey);

	//	if(m_Iter == m_NodeIDtoLinkMap.end( ) && bWarmingFlag)
	//	{
	//		CString msg;

	//		if(FileName.GetLength() == 0)
	//		{
	//			msg.Format ("Link %d-> %d cannot be found.", FromNodeNumber, ToNodeNumber);
	//		}else
	//		{
	//			msg.Format ("Link %d-> %d cannot be found in file %s.", FromNodeNumber, ToNodeNumber,FileName);
	//		}
	//		AfxMessageBox(msg);
	//		return NULL;
	//	}
	//	return m_NodeIDtoLinkMap[LinkKey];
	//}

	DTALink* FindLinkWithNodeIDs(int FromNodeID, int ToNodeID)
	{

		unsigned long LinkKey = GetLinkKey( FromNodeID, ToNodeID);
		if(m_NodeIDtoLinkMap.find(LinkKey)!=m_NodeIDtoLinkMap.end())
			return m_NodeIDtoLinkMap[LinkKey];
		else
			return NULL;
	}


	DTALink* FindLinkWithLinkNo(int LinkNo)
	{
	if(m_LinkNotoLinkMap.find(LinkNo) != m_LinkNotoLinkMap.end())
		return m_LinkNotoLinkMap[LinkNo];
	else
		return NULL;
	}


	DTALink* FindLinkWithLinkID(int LinkID)
	{
		if(m_LinkIDtoLinkMap.find(LinkID) != m_LinkIDtoLinkMap.end())
		{
		return m_LinkIDtoLinkMap[LinkID];
		}else
		return NULL;
	}
	CString GetTimeStampStrFromIntervalNo(int time_interval, bool with_single_quote=false);
	CString GetTimeStampFloatingPointStrFromIntervalNo(int time_interval);

	CString GetTimeStampString(int time_stamp_in_min);
	CString GetTimeStampString24HourFormat(int time_stamp_in_min);
	int* m_ZoneCentroidSizeAry;  //Number of centroids per zone
	int** m_ZoneCentroidNodeAry; //centroid node Id per zone

	std::vector<CString> m_SearchHistoryVector;

	COLORREF m_BackgroundColor;
	COLORREF m_ZoneColor;
	COLORREF m_ZoneTextColor;

	bool m_BackgroundBitmapLoaded;
	bool m_LongLatCoordinateFlag;
	CImage m_BackgroundBitmap;  // background bitmap
	double m_ImageX1,m_ImageX2,m_ImageY1,m_ImageY2, m_ImageWidth, m_ImageHeight;
	double m_ImageXResolution, m_ImageYResolution;
	double m_ImageMoveSize;
	double m_ImageWidthInMile;  // in mile

	// Operations
public:
	CString GetTableName(CString Tablename);
	CString ConstructSQL(CString Tablename);

	bool CreateNetworkFromExcelFile();
	bool ImportSensorData();
public: // subarea
	std::vector<GDPoint> m_SubareaShapePoints;

	bool CheckControlData();
	bool EditTrafficAssignmentOptions();
	void SendTexttoStatusBar(CString str,int Index = 0);
	bool SaveSubareaDemandFile();

	// Overrides
public:

	void ShowPathListDlg(bool bShowFlag);
	void RunExcelAutomation();
	void OpenCSVFileInExcel(CString filename);
	void Constructandexportsignaldata();
	void ConstructandexportVISSIMdata();
	void ReadSynchroUniversalDataFiles();

	bool ReadSynchroCombinedCSVFile(LPCTSTR lpszFileName);


	void ConvertOriginBasedDemandFile(LPCTSTR lpszFileName);
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
	BOOL SaveProject(LPCTSTR lpszPathName,int SelectedLayNo=0);
	BOOL SaveLinkData(LPCTSTR lpszPathName,bool bExport_Link_MOE_in_input_link_CSF_File, int SelectedLayNo);
	void CopyDefaultFiles();

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

	CString m_DefaultDataFolder;
	void CopyFileFromDefaultDataFolder(CString FileName)
	{

		if(m_ProjectDirectory.GetLength () ==0)
			return;

		if(CheckIfFileExsits(m_ProjectDirectory+FileName )==false)
		{
			CopyFile(m_DefaultDataFolder+FileName, m_ProjectDirectory+FileName, FALSE);
		}
	}

	bool CopyDefaultFile(CString DefaultDataFolder,CString CurrentProjectFolder, CString DestinationProjectDirectory, CString FileName, CString TargetFileName = "")
	{
		if(TargetFileName.IsEmpty ())
			TargetFileName = FileName;

		if(CheckIfFileExsits(DestinationProjectDirectory+FileName )==false)
		{
			int succeed_return;
			if(CheckIfFileExsits(CurrentProjectFolder+FileName )==true)
				succeed_return = CopyFile(CurrentProjectFolder+FileName, DestinationProjectDirectory+TargetFileName, FALSE);
			else
				succeed_return = CopyFile(DefaultDataFolder+FileName, DestinationProjectDirectory+TargetFileName, FALSE);

			return true; // copy success 
		}
		return false;  // file exits 
	}

	int FindCloseDTANode_WithNodeNumber(GDPoint pt, double threadshold, int this_node_number = -1)
	{
		 double min_distance  = 99999;
		 int NodeNumber = 0;
		for (std::list<DTANode*>::iterator iPoint = m_NodeSet.begin(); iPoint != m_NodeSet.end(); iPoint++)
		{

				double distance = Find_P2P_Distance((*iPoint)->pt,pt);
				if(min_distance > distance && (*iPoint)->m_NodeNumber != this_node_number )
				{
					min_distance = distance;
					NodeNumber = (*iPoint)->m_NodeNumber;
				}
		}
	
		if(min_distance < threadshold)
			return NodeNumber;
		else 
			return 0;
	}


	void WriteStringToLogFile(CString FileName, CString LogMessage)
	{
		ofstream outFile;

		outFile.open(m_ProjectDirectory + FileName);
		
		if(outFile.is_open ())
		{
		  outFile<< LogMessage;
		   outFile.close();
		}


		
	}
	
	void ReadInputPath();
	double m_PointA_x,m_PointA_y,m_PointB_x,m_PointB_y;
	double m_PointA_long,m_PointA_lat,m_PointB_long,m_PointB_lat;
	bool m_bPointA_Initialized, m_bPointB_Initialized;

	void ResetBackgroundImageCoordinate();


	void ExportToGISFile(LPCTSTR lpszCSVFileName,LPCTSTR lpszShapeFileName,  _GIS_DATA_TYPE GIS_data_type );
	int SelectLink(GDPoint point, double& final_matching_distance);
	// For demonstration
	CString m_SampleExcelNetworkFile;
	CString m_SampleOutputProjectFile;
	CString m_SampleExcelSensorFile;
	CString m_SampleNGSIMDataFile;

	bool FindObject(eSEARCHMODE SearchMode, int value1, int value12);

	void SaveMovementData(CString MovementFileName,  int NodeNumber);
	void RunQEMTool(CString MovementFileName, int NodeNumber);

	void RegenerateactivitylocationsForEmptyZone(int zoneid);
	// Implementation
	void GenerateMovementCountFromVehicleFile(float PeakHourFactor);
	void MapSignalDataAcrossProjects();
	void IdentifyLinkGroupCode();
	void UpdateUnitMile();
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
	afx_msg void OnExportGenerateTravelTimeMatrix();
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
	afx_msg void OnToolsGenerateodmatrixgravitymodel();
	afx_msg void OnLinkattributedisplayLinkname();
	afx_msg void OnUpdateLinkattributedisplayLinkname(CCmdUI *pCmdUI);
	afx_msg void OnToolsGeneratesignalcontrollocations();
	afx_msg void OnAssignmentSimulatinSettinsClicked();
	afx_msg void OnProjectNetworkData();
	afx_msg void OnLinkAddsensor();
	afx_msg void OnImportSynchrocombinedcsvfile();
	afx_msg void OnToolsObtainCyclelengthfromNearbySignals();
	afx_msg void OnToolsSaveprojectforexternallayer();
	afx_msg void OnToolsUpdateeffectivegreentimebasedoncyclelength();
	afx_msg void OnMoeTableDialog();
	afx_msg void OnToolsReverseverticalcoordinate();
	afx_msg void OnGenerategisshapefilesLoadlinkcsvfile();
	afx_msg void OnSafetyplanningtoolsRun();
	afx_msg void OnSafetyplanningtoolsGeneratenode();
	afx_msg void OnSensortoolsConverttoHourlyVolume();
	afx_msg void OnImportInrixshapefileandspeeddata();

#ifndef _WIN64
	CDaoDatabase m_Database;
#endif
	afx_msg void OnTrafficcontroltoolsTransfermovementdatafromreferencenetworktocurrentnetwork();
	afx_msg void OnDemandtoolsGenerateinput();
	afx_msg void OnDemandReconstructlinkmoeth();
	afx_msg void OnNetworktoolsResetlinklength();
	afx_msg void OnSubareaCreatezonefromsubarea();
	afx_msg void OnDemandRegenerateactivitylocations();
	afx_msg void OnDemandConvert();
	afx_msg void OnTrafficcontroltoolsTransfersignaldatafromreferencenetworktocurrentnetwork();
	afx_msg void OnImportBackgroundimage();
	afx_msg void OnZoneDeletezone();
	afx_msg void OnZoneViewzonedata();
	afx_msg void OnNodeViewnodedata();
	afx_msg void OnLinkViewlinkdata();
	afx_msg void OnMovementViewmovementdatatable();
	afx_msg void OnOdmatrixOddemandmatrix();
	afx_msg void OnWorkzoneViewworkzonedata();
	afx_msg void OnVmsViewvmsdatatable();
	afx_msg void OnTollViewtolldatatable();
	afx_msg void OnDetectorViewsensordatatable();
	afx_msg void OnConnectorViewactivitylocationdatatable();
	afx_msg void OnDetectorViewcalibration();
	afx_msg void OnLinkmoeExportlinkmoedatatoshapefile();
	afx_msg void OnNodeExportnodelayertogisshapefile();
	afx_msg void OnZoneExportzonelayertogisshapefile();
	afx_msg void OnGridUsemileasunitoflength();
	afx_msg void OnUpdateGridUsemileasunitoflength(CCmdUI *pCmdUI);
	afx_msg void OnGridUsekmasunitoflength();
	afx_msg void OnUpdateGridUsekmasunitoflength(CCmdUI *pCmdUI);
	afx_msg void OnGridUselong();
	afx_msg void OnUpdateGridUselong(CCmdUI *pCmdUI);
	afx_msg void OnCrashViewincidentdatatable();
	afx_msg void OnZoneRemoveactivitylocationsofselectedzone();
	afx_msg void OnZoneRegenerateactivitylocationsforselectedzone();
	afx_msg void OnShowMoePathlist();
	afx_msg void OnExportExportaggregatedlinkmoefile();
	afx_msg void OnHelpReportbug();
	afx_msg void OnFreewaytoolsView();
	afx_msg void OnExportGenerateshapefilesPathData();
	afx_msg void OnDetectorExportlinkflowproportionmatrixtocsvfile();
	afx_msg void OnDetectorUseodestimationoptimizationexcelprogram();
	afx_msg void OnDeleteWorkzone();
	afx_msg void OnCrashDeleteincidentonselectedlink();

	afx_msg void OnSubareaGenerateevacuationzonescenariofile();
	afx_msg void OnSubareaGenerateweatherscenariofile();
	afx_msg void OnSubareaGenerateworkzonescenariofilefromlinksinsidesubarea();

};
extern std::list<CTLiteDoc*>	g_DocumentList;
extern bool g_TestValidDocument(CTLiteDoc* pDoc);