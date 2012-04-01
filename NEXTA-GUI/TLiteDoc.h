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
#include "atlimage.h"
#include "math.h"
#include "Network.h"
#include ".\\cross-resolution-model\\SignalNode.h"
#include "Transit.h"
#include <iostream>
#include <fstream>
#include <afxdb.h>          // MFC database support
#include <afxdao.h>


enum Link_MOE {MOE_none,MOE_volume, MOE_speed, MOE_safety,MOE_vcratio,MOE_traveltime,MOE_capacity, MOE_speedlimit, MOE_fftt, MOE_length, MOE_oddemand, MOE_density, MOE_queuelength,MOE_fuel,MOE_emissions, MOE_vehicle, MOE_volume_copy, MOE_speed_copy, MOE_density_copy};

enum OD_MOE {odnone,critical_volume};

enum VEHICLE_CLASSIFICATION_SELECTION {CLS_network=0, CLS_OD,CLS_path,CLS_link,CLS_link_set,CLS_subarea_generated,CLS_subarea_traversing_through,CLS_subarea_internal_to_external,CLS_subarea_external_to_internal,CLS_subarea_internal_to_internal};
enum VEHICLE_X_CLASSIFICATION {CLS_pricing_type=0,CLS_VOT_10,CLS_VOT_15,CLS_VOT_10_SOV,CLS_VOT_10_HOV,CLS_VOT_10_truck,CLS_time_interval_15_min,CLS_time_interval_30_min,CLS_time_interval_60_min,CLS_information_class,CLS_vehicle_type};
enum VEHICLE_Y_CLASSIFICATION {CLS_vehicle_count=0,CLS_total_travel_time,CLS_avg_travel_time,CLS_total_travel_distance, CLS_avg_travel_distance,CLS_total_toll_cost,CLS_avg_toll_cost,CLS_total_generalized_cost,CLS_avg_generalized_cost,CLS_total_generalized_travel_time,CLS_avg_generalized_travel_time,CLS_total_CO2,CLS_avg_CO2};
enum LINK_BAND_WIDTH_MODE {LBW_number_of_lanes = 0, LBW_link_volume,LBW_number_of_marked_vehicles};

class CTLiteDoc : public CDocument
{
protected: // create from serialization only

	CTLiteDoc()
	{

	   m_AdjLinkSize = 20;  // initial value
	   m_colorLOS[0] = RGB(190,190,190);
	   m_colorLOS[1] = RGB(0,255,0);
	   m_colorLOS[2] = RGB(255,250,117);
	   m_colorLOS[3] = RGB(255,250,0);
	   m_colorLOS[4] = RGB(255,216,0);
	   m_colorLOS[5] = RGB(255,153,0);
	   m_colorLOS[6] = RGB(255,0,0);

	
	   m_FreewayColor = RGB(255,211,155);
	   m_HighwayColor = RGB(100,149,237); 
       m_ArterialColor = RGB(0,0,0);
	   m_ConnectorColor = RGB(255,255,0);

		m_BackgroundColor =  RGB(0,100,0);
		m_MaxLinkWidthAsNumberOfLanes = 5;
		m_MaxLinkWidthAsLinkVolume = 1000;

		m_VehicleSelectionMode = CLS_network;
		m_LinkBandWidthMode = LBW_number_of_lanes;
		m_bLoadNetworkDataOnly = false;
		m_SamplingTimeInterval = 1;
		m_AVISamplingTimeInterval = 5;

		m_bSimulationDataLoaded  = false;
		m_EmissionDataFlag = false;
		m_bLinkToBeShifted = true;
		m_SimulationStartTime_in_min = 0;  // 6 AM

		m_NumberOfDays = 0;
		m_LinkMOEMode = MOE_none;
		m_PrevLinkMOEMode = MOE_fftt;  // make sure the document gets a change to initialize the display view as the first mode is MOE_none
		m_ODMOEMode = odnone;

		MaxNodeKey = 60000;  // max: unsigned short 65,535;
		m_BackgroundBitmapLoaded  = false;
		m_LongLatCoordinateFlag = false;
		m_ColorFreeway = RGB(198,226,255);
		m_ColorHighway = RGB(100,149,237);
		m_ColorArterial = RGB(0,0,0);
		m_pNetwork = NULL;
		m_OriginNodeID = -1;

		m_DestinationNodeID = -1;
		m_NodeSizeSP = 0;


		m_PathMOEDlgShowFlag = false;
		m_SelectPathNo = -1;

		m_ImageX1 = 0;
		m_ImageX2 = 1000;
		m_ImageY1 = 0;
		m_ImageY2 = 1000;
		m_NodeDisplaySize = 50;
		m_ShowNodeLayer = false;


		m_NetworkRect.top  = 50;
		m_NetworkRect.bottom = 0;

		m_NetworkRect.left   = 0;
		m_NetworkRect.right = 100;

		m_UnitMile = 1;
		m_UnitFeet = 1/5280.0;
		m_OffsetInFeet = 20;
		m_LaneWidthInFeet = 20;
		m_bFitNetworkInitialized = false; 
	    m_BackgroundBitmapImportedButnotSaved = false;

		m_DefaultNumLanes = 3;
		m_DefaultSpeedLimit = 65.0f;
		m_DefaultCapacity = 1900.0f;
		m_DefaultLinkType = 1;
		m_DemandMatrix = NULL;
		m_ODSize = 0;
		m_SelectedLinkID = -1;
	    m_SelectedNodeID = -1;
		m_SelectedTrainID = -1;
	
		m_bSetView = false;
		m_bShowLegend = false;
		m_bShowLinkList  = false;

		for(int i=0; i<40;i++)
		{
			for(int los= 0; los < MAX_LOS_SIZE; los++)
			{
				m_LOSBound[i][los] = 0;
			}
		}

		// speed LOS bound
		m_LOSBound[MOE_speed][1] = 100;
		m_LOSBound[MOE_speed][2] = 90;
		m_LOSBound[MOE_speed][3] = 70;
		m_LOSBound[MOE_speed][4] = 50;
		m_LOSBound[MOE_speed][5] = 40;
		m_LOSBound[MOE_speed][6] = 33;
		m_LOSBound[MOE_speed][7] = 0;

		m_LOSBound[MOE_vcratio][1] = 0;
		m_LOSBound[MOE_vcratio][2] = 0.65f;
		m_LOSBound[MOE_vcratio][3] = 0.75f;
		m_LOSBound[MOE_vcratio][4] = 0.85f;
		m_LOSBound[MOE_vcratio][5] = 0.95f;
		m_LOSBound[MOE_vcratio][6] = 1.00f;
		m_LOSBound[MOE_vcratio][7] = 999;



		m_TrafficFlowModelFlag = 1;  // static traffic assignment as default
		m_Doc_Resolution = 1;
		m_bShowCalibrationResults = false;

		m_SampleExcelNetworkFile = "\\Sample_Import_Excel_Files\\sample_Portland_subarea_network.xls";
		m_SampleOutputProjectFile = "\\Sample_Output_Project_Folder";
		m_SampleExcelSensorFile = "\\Sample_Import_Excel_Files\\input_Portland_sensor_data.xls";


	}

	DECLARE_DYNCREATE(CTLiteDoc)

	// Attributes
public:

	PT_Network m_PT_network;  // public transit network class by Shuguang Li
	bool m_bShowCalibrationResults;

	int m_TrafficFlowModelFlag;
	COLORREF m_colorLOS[MAX_LOS_SIZE];

	COLORREF m_FreewayColor;
	COLORREF m_HighwayColor;
	COLORREF m_ArterialColor;
	COLORREF m_ConnectorColor;


	float m_LOSBound[40][MAX_LOS_SIZE];
	bool m_bShowLegend;
	bool m_bShowLinkList;
	int m_NodeDisplaySize;
	bool m_ShowNodeLayer;

	GDPoint m_Origin;

    int NPtoSP_X(GDPoint net_point,float Resolution) // convert network coordinate to screen coordinate
	{
		return int((net_point.x-m_Origin.x)*Resolution+0.5);
	}
    int NPtoSP_Y(GDPoint net_point,float Resolution) // convert network coordinate to screen coordinate
	{
		return int((net_point.y-m_Origin.y)*Resolution+0.5);

	}

	void ChangeNetworkCoordinates(int LayerNo, float XScale, float YScale, float deltaX_ratio, float deltaY_ratio);

	void HighlightPath(	std::vector<int>	m_LinkVector, int DisplayID);

	void HighlightSelectedVehicles(bool bSelectionFlag);
	int m_CurrentViewID;
	bool m_bSetView;
 	GDPoint m_Doc_Origin;
	float m_Doc_Resolution;

	int m_ODSize;
	float** m_DemandMatrix;
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

	BOOL OnOpenDocument(CString FileName);
	BOOL OnOpenTrafficNetworkDocument(CString ProjectFileName, bool bNetworkOnly = false);
	BOOL OnOpenDYNASMARTProject(CString ProjectFileName, bool bNetworkOnly = false);
	BOOL OnOpenRailNetworkDocument(LPCTSTR lpszPathName);

	std::ofstream m_WarningFile;

	void OpenWarningLogFile(CString directory);
	// two basic input
	bool ReadNodeCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadLinkCSVFile(LPCTSTR lpszFileName, bool bCreateNewNodeFlag, int LayerNo);   // for road network

	bool ReadTransitFiles(CString ProjectFolder);   // for road network

	void OffsetLink();


	void GenerateOffsetLinkBand();

	void ReCalculateLinkBandWidth();
	bool ReadZoneCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadDemandCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadDemandCSVFile_Parser(LPCTSTR lpszFileName);   // for road network
	bool ReadSubareaCSVFile(LPCTSTR lpszFileName);
	bool ReadVOTCSVFile(LPCTSTR lpszFileName);  
	bool ReadTemporalDemandProfileCSVFile(LPCTSTR lpszFileName);  
	bool ReadVehicleTypeCSVFile(LPCTSTR lpszFileName);  
	bool ReadDemandTypeCSVFile(LPCTSTR lpszFileName);
	bool ReadLinkTypeCSVFile(LPCTSTR lpszFileName); 
	
	bool ReadScenarioData();   // for road network

	bool ReadVMSScenarioData();
	bool WriteVMSScenarioData();

	bool ReadLink_basedTollScenarioData();
	bool WriteLink_basedTollScenarioData();

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
	void ReadSimulationLinkStaticMOEData(LPCTSTR lpszFileName);
	void ReadObservationLinkVolumeData(LPCTSTR lpszFileName);

	bool ReadTimetableCVSFile(LPCTSTR lpszFileName);
	void ReadHistoricalData(CString directory);
	
	int m_SamplingTimeInterval;
	int m_AVISamplingTimeInterval;
	int m_NumberOfDays;
	int m_SimulationStartTime_in_min;


	bool ReadSensorData(LPCTSTR lpszFileName);
	void ReadEventData(CString directory);
	void BuildHistoricalDatabase();

	bool ReadMultiDaySensorData(LPCTSTR lpszFileName);
	void ReadInputEmissionRateFile(LPCTSTR lpszFileName);
	CEmissionRate EmissionRateData[MAX_VEHICLE_TYPE_SIZE][_MAXIMUM_OPERATING_MODE_SIZE];
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

	std::list<DTACrash*>	m_CrashSet;

	std::list<DTANode*>		m_SubareaNodeSet;
	std::list<DTALink*>		m_SubareaLinkSet;

	std::map<int, DTAZone>	m_ZoneMap;
	std::list<DTAVehicle*>	m_VehicleSet;

	std::map<long, CAVISensorPair> m_AVISensorMap;
	
	std::map<int, DTANode*> m_NodeIDMap;

	std::map<int, DTANode*> m_SubareaNodeIDMap;
	bool CTLiteDoc::WriteSubareaFiles();

	std::map<long, DTALink*> m_LinkNoMap;
	std::map<long, DTAVehicle*> m_VehicleIDMap;


	bool m_EmissionDataFlag;
	
	int m_AdjLinkSize;

	DTANetworkForSP* m_pNetwork;
	DTANetworkForSP m_Network;

	int Routing(bool bCheckConnectivity);
	int AlternativeRouting(int NumberOfRoutes);

	CString GetWorkspaceTitleName(CString strFullPath);
	CString m_ProjectTitle;

	float FillODMatrixFromCSVFile(LPCTSTR lpszFileName);

	void AdjustCoordinateUnitToMile();

	void ReadTrainProfileCSVFile(LPCTSTR lpszFileName);
	void ReadVehicleCSVFile_Parser(LPCTSTR lpszFileName);
	void ReadVehicleCSVFile(LPCTSTR lpszFileName);
	bool WriteSelectVehicleDataToCSVFile(LPCTSTR lpszFileName, std::vector<DTAVehicle*> VehicleVector);
	void ReadVehicleEmissionFile(LPCTSTR lpszFileName);

	
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
	std::map<int, int> m_LinkTypeFreewayMap;
	std::map<int, int> m_LinkTypeArterialMap;
	std::map<int, int> m_LinkTypeRampMap;


	std::map<int, int> m_NodeIDtoNameMap;
	std::map<int, int> m_NodeNametoIDMap;
	std::map<int, int> m_NodeIDtoZoneNameMap;

	int m_SelectedLinkID;
	int m_SelectedNodeID;
	int m_SelectedTrainID;


	std::vector<DTA_sensor> m_SensorVector;
	std::vector<DTADemand> m_DemandVector;
	std::vector<DTAVehicleType> m_VehicleTypeVector;
	std::vector<DTADemandType> m_DemandTypeVector;
	std::vector<DTALinkType> m_LinkTypeVector;
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
		pLink->m_LinkNo = m_LinkSet.size();
		pLink->m_FromNodeNumber = m_NodeIDtoNameMap[FromNodeID];
		pLink->m_ToNodeNumber = m_NodeIDtoNameMap[ToNodeID];
		pLink->m_FromNodeID = FromNodeID;
		pLink->m_ToNodeID= ToNodeID;

		pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
		pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;

		m_NodeIDMap[FromNodeID ]->m_Connections+=1;
		m_NodeIDMap[ToNodeID ]->m_Connections+=1;

		unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
		m_NodeIDtoLinkMap[LinkKey] = pLink;

		__int64  LinkKey2 = pLink-> m_FromNodeNumber* pLink->m_ToNodeNumber;
		m_NodeNumbertoLinkMap[LinkKey2] = pLink;

		pLink->m_NumLanes= m_DefaultNumLanes;
		pLink->m_SpeedLimit= m_DefaultSpeedLimit;
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

			int last_shape_point_id = pLink ->m_ShapePoints .size() -1;
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

	DTANode* AddNewNode(GDPoint newpt, int NewNodeNumber=0 , int LayerNo =0)
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
		//resort link id;
		int i= 0;
		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++, i++)
		{
		(*iLink)->m_LinkNo = i;
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

	std::vector<DTA_NodeMovementSet> m_MovementVector;
	std::vector<DTA_NodePhaseSet> m_PhaseVector;

	// 	void ConstructMovementVector(bool flag_Template);
	void ConstructMovementVectorForEachNode();

	// function declaration for Synchro /////////////////////////////////////////////////////////////////////////////////
	void ConstructMovementVector(bool flag_Template);
	bool LoadMovementTemplateFile(DTA_NodeMovementSet& MovementTemplate, DTA_NodePhaseSet& PhaseTemplate);
	bool LoadMovementDefault(DTA_NodeMovementSet& MovementTemplate, DTA_NodePhaseSet& PhaseTemplate);
	void ExportSingleSynchroFile(CString SynchroProjectFile);

	void ExportSynchroVersion6Files();
	CString m_Synchro_ProjectDirectory;

	void ExportOGRShapeFile();
	void ImportOGRShapeFile(CString FileName);
	
	int Find_P2P_Angle(GDPoint p1, GDPoint p2);
	DTA_Turn Find_RelativeAngle_to_Turn(int relative_angle);
	DTA_Approach g_Angle_to_Approach_New(int angle);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::map<unsigned long, DTALink*> m_NodeIDtoLinkMap;
	std::map<__int64, DTALink*> m_NodeNumbertoLinkMap;

	std::map<long, DTALink*> m_LinkNotoLinkMap;
	std::map<long, DTALink*> m_SensorIDtoLinkMap;
	std::map<long, int> m_AVISensorIDtoNodeIDMap;


	int Find_PPP_RelativeAngle(GDPoint p1, GDPoint p2, GDPoint p3);
	DTA_Turn Find_PPP_to_Turn(GDPoint p1, GDPoint p2, GDPoint p3);

	int MaxNodeKey;
	unsigned long GetLinkKey(int FromNodeID, int ToNodeID)
	{

		unsigned long LinkKey = FromNodeID*MaxNodeKey+ToNodeID;
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

	DTALink* FindLinkWithNodeNumbers(int FromNodeNumber, int ToNodeNumber, CString FileName = "", bool bWarmingFlag = true)
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
		return m_NodeIDtoLinkMap[LinkKey];
	}

	DTALink* FindLinkWithNodeNumbers_64bitKey(int FromNodeNumber, int ToNodeNumber)
	{
		__int64  LinkKey = FromNodeNumber* ToNodeNumber;

	return m_NodeNumbertoLinkMap[LinkKey];
	}


	DTALink* FindLinkWithLinkNo(int LinkID)
	{
		return m_LinkNotoLinkMap[LinkID];
	}

	CString GetTimeStampStrFromIntervalNo(int time_interval, bool with_single_quote);
	CString GetTimeStampFloatingPointStrFromIntervalNo(int time_interval);

	int* m_ZoneCentroidSizeAry;  //Number of centroids per zone
	int** m_ZoneCentroidNodeAry; //centroid node Id per zone


	COLORREF m_BackgroundColor;
	bool m_BackgroundBitmapLoaded;
	bool m_LongLatCoordinateFlag;
	CImage m_BackgroundBitmap;  // background bitmap
	float m_ImageX1,m_ImageX2,m_ImageY1,m_ImageY2, m_ImageWidth, m_ImageHeight;
	float m_ImageXResolution, m_ImageYResolution;
	float m_ImageMoveSize;
	bool m_BackgroundBitmapImportedButnotSaved;

	CDaoDatabase m_Database;


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

	// Overrides
public:

	void RunExcelAutomation();
	void OpenCSVFileInExcel(CString filename);
	void Constructandexportsignaldata();
	void ConstructandexportVISSIMdata();
	bool m_bFitNetworkInitialized;
	void CalculateDrawingRectangle();

	DWORD ProcessExecute(CString & strCmd, CString & strArgs,  CString & strDir, BOOL bWait);
	DWORD ProcessWait(DWORD PID);

	CString m_ProjectFile;
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	BOOL SaveProject(LPCTSTR lpszPathName);

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
	afx_msg void OnViewShowmoe();
	afx_msg void OnUpdateViewShowmoe(CCmdUI *pCmdUI);
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
		afx_msg void OnMoeOddemand();
		afx_msg void OnUpdateMoeOddemand(CCmdUI *pCmdUI);
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
		afx_msg void OnFileOpenrailnetworkproject();
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
};


