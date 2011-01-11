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

#include "atlimage.h"
#include "math.h"
#include "Network.h"

enum Link_MOE {none,volume, speed, density, queuelength,fuel,emissions};

class CTLiteDoc : public CDocument
{
protected: // create from serialization only
	CTLiteDoc()
	{
		m_LinkMOEMode = none;
		MaxNodeKey = 60000;  // max: unsigned short 65,535;
		m_BackgroundBitmapLoaded  = false;
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

		m_NetworkRect.top  = 50;
		m_NetworkRect.bottom = 0;

		m_NetworkRect.left   = 0;
		m_NetworkRect.right = 100;

		m_UnitMile = 1;
		m_UnitFeet = 1/5280.0;
		m_bFitNetworkInitialized = false; 
	    m_BackgroundBitmapImportedButnotSaved = false;

	}
	DECLARE_DYNCREATE(CTLiteDoc)

	// Attributes
public:

	Link_MOE m_LinkMOEMode;
	std::vector<DTAPath*>	m_PathDisplayList;
	bool m_PathMOEDlgShowFlag;
	int m_SelectPathNo;

	int m_OriginNodeID;
	int m_DestinationNodeID;

	double m_UnitFeet, m_UnitMile;

	COLORREF m_ColorFreeway, m_ColorHighway, m_ColorArterial;

	BOOL OnOpenDocument(LPCTSTR lpszPathName);

	// two basic input
	bool ReadNodeCSVFile(LPCTSTR lpszFileName);   // for road network
	bool ReadLinkCSVFile(LPCTSTR lpszFileName);   // for road network
	
	// additional input
	void ReadSimulationLinkMOEData(LPCTSTR lpszFileName);
	void ReadObservationLinkVolumeData(LPCTSTR lpszFileName);

	bool ReadTimetableCVSFile(LPCTSTR lpszFileName);
	void ReadHistoricalData(CString directory);
	void ReadSensorLocationData(CString directory);
	void ReadHistoricalDataFormat2(CString directory);

	CString m_NodeDataLoadingStatus;
	CString m_LinkDataLoadingStatus;
	CString m_LinkTrainTravelTimeDataLoadingStatus;
	CString m_TimetableDataLoadingStatus;

	CString m_ObsLinkVolumeStatus;
	CString m_BackgroundImageFileLoadingStatus;

	CString m_SimulationLinkMOEDataLoadingStatus;
	CString m_SimulationVehicleDataLoadingStatus;
	
	
	int FindLinkFromSensorLocation(float x, float y, int direction);

int GetVehilePosition(DTAVehicle* pVehicle, double CurrentTime, float& ratio);
float GetLinkMOE(DTALink* pLink, Link_MOE LinkMOEMode, int CurrentTime);
public:
	std::list<DTANode*>		m_NodeSet;
	std::list<DTALink*>		m_LinkSet;
	std::list<DTAVehicle*>	m_VehicleSet;
	
	int m_AdjLinkSize;

	DTANetworkForSP* m_pNetwork;
	int Routing();


	void ReadTrainProfileCSVFile(LPCTSTR lpszFileName);
	void ReadVehicleCSVFile(LPCTSTR lpszFileName);
	
	bool TimetableOptimization_Lagrangian_Method();  //Lagrangian based.
	bool TimetableOptimization_Priority_Rule();  //Lagrangian based.

	void ReadBackgroundImageFile(LPCTSTR lpszFileName);
	int m_PathNodeVectorSP[MAX_NODE_SIZE_IN_A_PATH];
	long m_NodeSizeSP;


	std::map<int, DTANode*> m_NodeIDMap;
	std::map<long, DTALink*> m_LinkIDMap;
	std::map<long, DTAVehicle*> m_VehicleIDMap;
	

	std::map<int, int> m_NodeIDtoNameMap;
	std::map<int, int> m_NodeNametoIDMap;

	std::vector<DTA_sensor> m_SensorVector;

	std::vector<DTA_Train*> m_TrainVector;

	CString m_ProjectDirectory;

	GDRect m_NetworkRect;

	bool AddNewLink(int FromNodeID, int ToNodeID, bool bOffset = false)
	{
		DTALink* pLink = 0;

		pLink = FindLinkWithNodeIDs(FromNodeID,ToNodeID);
		if(pLink != NULL)
			return false;  // a link with the same from and to node numbers exists!

		pLink = new DTALink(1);
		pLink->m_LinkID = m_LinkSet.size();
		pLink->m_FromNodeNumber = m_NodeIDtoNameMap[FromNodeID];
		pLink->m_ToNodeNumber = m_NodeIDtoNameMap[ToNodeID];
		pLink->m_FromNodeID = FromNodeID;
		pLink->m_ToNodeID= ToNodeID;
		m_NodeIDMap[FromNodeID ]->m_Connections+=1;
		m_NodeIDMap[ToNodeID ]->m_Connections+=1;

		unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);
		m_NodeIDtoLinkMap[LinkKey] = pLink;

		pLink->m_NumLanes= 2;
		pLink->m_SpeedLimit= 60;
		pLink->m_Length= pLink->DefaultDistance();
		pLink->m_MaximumServiceFlowRatePHPL= 2000;
		pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
		pLink->m_link_type= 1;

		m_NodeIDMap[FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumLanes);
		pLink->m_FromPoint = m_NodeIDMap[FromNodeID]->pt;
		pLink->m_ToPoint = m_NodeIDMap[ToNodeID]->pt;

		if(bOffset)
		{
			double link_offset = m_UnitFeet*80;
			double DeltaX = pLink->m_ToPoint.x - pLink->m_FromPoint.x ;
			double DeltaY = pLink->m_ToPoint.y - pLink->m_FromPoint.y ;
			double theta = atan2(DeltaY, DeltaX);

			pLink->m_FromPoint.x += link_offset* cos(theta-PI/2.0f);
			pLink->m_ToPoint.x += link_offset* cos(theta-PI/2.0f);

			pLink->m_FromPoint.y += link_offset* sin(theta-PI/2.0f);
			pLink->m_ToPoint.y += link_offset* sin(theta-PI/2.0f);
		}


		pLink->SetupMOE();
		m_LinkSet.push_back (pLink);
		m_LinkIDMap[pLink->m_LinkID]  = pLink;



		return true;
	}

	DTANode* AddNewNode(GDPoint newpt)
	{
		DTANode* pNode = new DTANode;
		pNode->pt = newpt;
		pNode->m_NodeNumber = GetUnusedNodeNumber();
		pNode->m_NodeID = m_NodeSet.size();
		pNode->m_ZoneID = 0;
		m_NodeSet.push_back(pNode);
		m_NodeIDMap[pNode->m_NodeID] = pNode;
		m_NodeIDtoNameMap[pNode->m_NodeID ] = pNode->m_NodeNumber;
		m_NodeNametoIDMap[pNode->m_NodeNumber] = pNode->m_NodeID;

		return pNode;
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

	std::map<unsigned long, DTALink*> m_NodeIDtoLinkMap;
	std::map<long, long> m_SensorIDtoLinkIDMap;

	int MaxNodeKey;
	unsigned long GetLinkKey(int FromNodeID, int ToNodeID)
	{
		unsigned long LinkKey = FromNodeID*MaxNodeKey+ToNodeID;
		return LinkKey;
	}

	DTALink* FindLinkWithNodeNumbers(int FromNodeNumber, int ToNodeNumber)
	{
		int FromNodeID = m_NodeNametoIDMap[FromNodeNumber];
		int ToNodeID = m_NodeNametoIDMap[ToNodeNumber];

		unsigned long LinkKey = GetLinkKey( FromNodeID, ToNodeID);
		return m_NodeIDtoLinkMap[LinkKey];
	}

	DTALink* FindLinkWithNodeIDs(int FromNodeID, int ToNodeID)
	{

		unsigned long LinkKey = GetLinkKey( FromNodeID, ToNodeID);
		return m_NodeIDtoLinkMap[LinkKey];
	}




	int* m_ZoneCentroidSizeAry;  //Number of centroids per zone
	int** m_ZoneCentroidNodeAry; //centroid node Id per zone

	DTAZone* m_ZoneInfo;

	bool m_BackgroundBitmapLoaded;
	CImage m_BackgroundBitmap;  // background bitmap
	float m_ImageX1,m_ImageX2,m_ImageY1,m_ImageY2, m_ImageWidth, m_ImageHeight;
	float m_ImageXResolution, m_ImageYResolution;
	float m_ImageMoveSize;
	bool m_BackgroundBitmapImportedButnotSaved;


	// Operations
public:


	void SendTexttoStatusBar(CString str);

	// Overrides
public:
	bool m_bFitNetworkInitialized;
	void CalculateDrawingRectangle();

	CString m_ProjectFile;
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	BOOL SaveProject(LPCTSTR lpszPathName);

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
	afx_msg void OnToolGeneratesenesormappingtable();
	afx_msg void OnShowShowpathmoe();
	afx_msg void OnUpdateShowShowpathmoe(CCmdUI *pCmdUI);
	afx_msg void OnViewShowmoe();
	afx_msg void OnUpdateViewShowmoe(CCmdUI *pCmdUI);
	afx_msg void OnSearchListtrains();
	afx_msg void OnToolsTimetablingoptimization();
	afx_msg void OnTimetableImporttimetable();
	afx_msg void OnInitializetimetable();
	afx_msg void OnWindow2dview();
	afx_msg void OnOptimizetimetable_PriorityRule();
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
};


