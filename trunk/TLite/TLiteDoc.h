// TLiteDoc.h : interface of the CTLiteDoc class
//
//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com)

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html


//    This file is part of NEXTA  Version 3 (Open-source).

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
#include "Network.h"


class CTLiteDoc : public CDocument
{
protected: // create from serialization only
	CTLiteDoc()
	{
	MaxNodeKey = 60000;  // max: unsigned short 65,535;
	m_BKBitmapLoaded  = false;
	m_ColorFreeway = RGB(198,226,255);
	m_ColorHighway = RGB(100,149,237);
	m_ColorArterial = RGB(0,0,0);
	m_pNetwork = NULL;
    m_OriginNodeID = -1;
	m_DestinationNodeID = -1;
	m_NodeSizeSP = 0;
	m_PathMOEDlgShowFlag = false;
	m_SelectPathNo = -1;
	
	}
	DECLARE_DYNCREATE(CTLiteDoc)

// Attributes
public:
    
	std::vector<DTAPath*>	m_PathDisplayList;
	bool m_PathMOEDlgShowFlag;
	int m_SelectPathNo;

	int m_OriginNodeID;
	int m_DestinationNodeID;

	double m_UnitFeet;

   COLORREF m_ColorFreeway, m_ColorHighway, m_ColorArterial;

BOOL OnOpenDocument(LPCTSTR lpszPathName);

void ReadLinkCSVFile(CString directory);   // for road network
void ReadArcCSVFile(CString directory);   // for rail/air network

void ReadHistoricalData(CString directory);
void ReadSensorLocationData(CString directory);
void ReadHistoricalDataFormat2(CString directory);

int FindLinkFromSensorLocation(float x, float y, int direction);


public:
std::list<DTANode*>		m_NodeSet;
std::list<DTALink*>		m_LinkSet;
int m_AdjLinkSize;

DTANetworkForSP* m_pNetwork;
int Routing();

bool ImportTimetableData(LPCTSTR lpszFileName);
bool TimetableOptimization();  //Lagrangian based.

long m_PathNodeVectorSP[MAX_NODE_SIZE_IN_A_PATH];
long m_NodeSizeSP;


std::map<int, DTANode*> m_NodeMap;
std::map<long, DTALink*> m_LinkMap;

std::map<int, int> m_NodeIDtoNameMap;
std::map<int, int> m_NodeNametoIDMap;

std::vector<DTA_sensor> m_SensorVector;

std::vector<DTA_Train*> m_TrainVector;

CString m_ProjectDirectory;

GDRect m_NetworkRect;

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

std::list<DTAVehicle*>		m_VehicleSet;
std::map<int, DTAVehicle*> m_VehicleMap;


bool m_BKBitmapLoaded;
CImage m_BKBitmap;  // background bitmap
float m_ImageX1,m_ImageX2,m_ImageY1,m_ImageY2, m_ImageWidth, m_ImageHeight;
float m_ImageXResolution, m_ImageYResolution;
float m_ImageMoveSize;


// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

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
};


