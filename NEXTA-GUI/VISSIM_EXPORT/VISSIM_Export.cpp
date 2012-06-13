//  Portions Copyright 2010 Peng @ pdu@bjtu.edu.cn
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
#include "..//TLite.h"
#include "..//Network.h"
#include "..//CSVParser.h"
#include "..//Geometry.h"
#include "..//TLiteDoc.h"

#include "comutil.h" 
#include <clocale>
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <tchar.h>
#include <msxml2.h>   
#import "msxml4.dll" 


#include "VISSIM_Export.h"

// This function is the interface with the main program
void CTLiteDoc::ConstructandexportVISSIMdata()
{

	CWaitCursor cursor;
	Mustang ms;
		CString strFolder  = m_ProjectDirectory;
		CString strNodeCSV = strFolder + _T("input_node.csv");;
		CString strLinkCSV = strFolder + _T("input_link.csv");
		CString strLinkTypeCSV = strFolder + _T("input_link_type.csv");
		CString strANMFile = strFolder + _T("simulation.anm");
		CString strLogFile = strFolder + _T("msLog.log");
		CString strZoneCSV = strFolder + _T("input_zone.csv");

		std::string strNodeFileName,strLinkFileName,strLinkTypeFileName,strANMFileName,strLogFileName,strZoneFileName;
		USES_CONVERSION;
		strNodeFileName = T2A(strNodeCSV.GetBuffer());
		strLinkFileName = T2A(strLinkCSV.GetBuffer());
		strLinkTypeFileName = T2A(strLinkTypeCSV.GetBuffer());
		strANMFileName  = T2A(strANMFile.GetBuffer());
		strLogFileName  = T2A(strLogFile.GetBuffer());
		strZoneFileName = T2A(strZoneCSV.GetBuffer());

		ms.OpenLogFile(strLogFileName);
		ms.ReadInputNodeCSV(strNodeFileName);
		CWaitCursor wait;
		ms.ReadInputLinkCSV(strLinkFileName);
		ms.ReadInputLinkTypeCSV(strLinkTypeFileName);
		//ms.ReadInputZoneCSV(strZoneFileName);

		bool bReturn = ms.ClassifyNodes();
		ms.DumpNodeLink2Log();
		bReturn = ms.CreateDefaultLanes();
		bReturn = ms.ProcessLanes();
		ms.CreateDefaultData();
		ms.CreateANMFile(strANMFileName);
		ms.CloseLogFile();

		OnToolsProjectfolder();	
}

///////////////////////////////////////////////////////////////////////////////////////
//		realization of Mustang
///////////////////////////////////////////////////////////////////////////////////////
Mustang::Mustang()
{
}
Mustang::~Mustang()
{
	std::list<MNode*>::iterator iNode;
	for (iNode = m_NodeList.begin(); iNode != m_NodeList.end(); iNode++)
	{
		std::vector<MLaneTurn*>::iterator iTurn;
		for(iTurn=(*iNode)->LaneTurns.begin();iTurn!=(*iNode)->LaneTurns.end();iTurn++)
			delete (*iTurn);
		(*iNode)->LaneTurns.clear();
		delete *iNode;
	}
	m_NodeList.clear();

	std::list<MLink*>::iterator iLink;
	for(iLink = m_LinkList.begin();iLink!=m_LinkList.end();iLink++)
	{
		std::vector<MLane*>::iterator iLane;
		for(iLane=(*iLink)->inLanes.begin();iLane!=(*iLink)->inLanes.end();iLane++)
			delete (*iLane);
		(*iLink)->inLanes.clear();
		for(iLane=(*iLink)->outLanes.begin();iLane!=(*iLink)->outLanes.end();iLane++)
			delete (*iLane);
		(*iLink)->outLanes.clear();
		delete (*iLink);
	}
	m_LinkList.clear();
}
MNode::MNode()
{
}
MNode::~MNode()
{
}
MLink::MLink()
{
}
MLink::~MLink()
{
}
MLane::MLane()
{
}
MLane::~MLane()
{
}
MLaneTurn::MLaneTurn()
{
}
MZone::MZone()
{
}
MZone::~MZone()
{
}
CMVehClass::CMVehClass()
{
}

CMVehClass::~CMVehClass()
{
}
CMSignalControl::CMSignalControl()
{
}
CMSignalControl::~CMSignalControl()
{
}
CMSignalGroup::CMSignalGroup()
{
}
CMSignalGroup::~CMSignalGroup()
{
}

CString Mustang::Minutes2PTString(int nMin)
{
	CString	szRt = _T("");
	int mm = nMin / 60;
	int ss = nMin - mm*60;
	if ( 0 == mm)
		szRt.Format(TEXT("PT%dS"),ss);
	else if ( 0 == ss)
		szRt.Format(TEXT("PT%dM"),mm);
	else
		szRt.Format(TEXT("PT%dM%dS"),mm,ss);

	return szRt;
}
MLaneTurn::~MLaneTurn()
{
}
MDemand::MDemand(){}
MDemand::~MDemand(){}
float Mustang::L2X(float longitude,float latitude)
{
	float x = 0.0;
	x = (longitude - refLongi) * refScale * cos(latitude / 180);
	return x;
}
float Mustang::L2Y(float latitude)
{
	float y = 0.0;
	y = (latitude - refLati) * refScale;
	return y;
}
bool Mustang::CloseLogFile()
{
	if (m_logFile.is_open())
		m_logFile.close();
	return true;
	
}
bool Mustang::OpenLogFile(std::string strLogFileName)
{
	m_logFile.open (strLogFileName.c_str(), ios::out);
	if (m_logFile.is_open())
	{
		m_logFile.width(12);
		m_logFile.precision(3) ;
		m_logFile.setf(ios::fixed);
		return true;
	}
	else
	{
		AfxMessageBox("File warning.log cannot be opened, and it might be locked by another program!");
		return false;
	}

}
bool Mustang::ReadInputNodeCSV(std::string strFileName)
{
	m_logFile<< "ReadInputNodeCSV function called!"<<endl;
	CCSVParser parser;
	if (parser.OpenCSVFile(strFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int node_id;
			string name;
			MNode* pNode = 0;

			int control_type;
			double X;
			double Y;
			if(parser.GetValueByFieldName("node_id",node_id) == false)
				break;

			if(!parser.GetValueByFieldName("name",name))
				name = "";

			if(!parser.GetValueByFieldName("control_type",control_type))
				control_type = 0;

			// use the X and Y as default values first
			bool bFieldX_Exist = parser.GetValueByFieldName("x",X);
			parser.GetValueByFieldName("y",Y);

			string geo_string;

			if(parser.GetValueByFieldName("geometry",geo_string))
			{
				// overwrite when the field "geometry" exists
				CGeometry geometry(geo_string);

				std::vector<CCoordinate> CoordinateVector = geometry.GetCoordinateList();

				if(CoordinateVector.size()>0)
				{
					X = CoordinateVector[0].X;
					Y = CoordinateVector[0].Y;
				}

			}

			pNode = new MNode;
			pNode->m_Name = name;
			pNode->nControlType = control_type;
			if ( 0 == i)
			{
				refLongi = X;
				refLati  = Y;
				refScale = 111700.0;
			}
			
			pNode->ptLL.x = X;
			pNode->ptLL.y = Y;

			pNode->pt.x = L2X(X,Y);
			pNode->pt.y = L2Y(Y);

			pNode->m_NodeNumber = node_id;
			pNode->m_NodeID = i;
			pNode->m_ZoneID = 0;
			m_NodeList.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeIDtoNameMap[i] = node_id;
			m_NodeNametoIDMap[node_id] = i;
			i++;

			//			cout << "node = " << node << ", X= " << X << ", Y = " << Y << endl;

		}
		CString strMsg;
		strMsg.Format("%d nodes are loaded from file %s.",m_NodeList.size(),strFileName.c_str());
		m_logFile<<strMsg<<endl;
		AfxMessageBox(strMsg, MB_ICONINFORMATION);
		return true;
	}
	else
	{
		AfxMessageBox("Error: File input_node.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}
}
bool Mustang::ReadInputLinkCSV(std::string strFileName)
{
	CCSVParser parser;
	CString error_message;
	int i=0;

	if (parser.OpenCSVFile(strFileName))
	{
		while(parser.ReadRecord())
		{
			std::string name;
			int link_id = 0;
			int from_node_id;
			int to_node_id;
			int number_of_lanes= 1;
			int speed_limit_in_mph;
			int type = 1;
			std::string from_approach;
			std::string to_approach;
			int reverse_link_id = 0;
			int nFrom = 0;

			if(!parser.GetValueByFieldName("name",name))				name = "";

			if(!parser.GetValueByFieldName("link_id",link_id))
			{
				AfxMessageBox("Field link_id has not been defined in file input_link.csv. Please check.");
				break;
			}
			// 检查link_id的唯一性
			if ( !CheckDuplicateLink(link_id)) continue;
			if(!parser.GetValueByFieldName("from_node_id",from_node_id)) 
			{
				AfxMessageBox("Field from_node_id has not been defined in file input_link.csv. Please check.");
				break;
			}
			if(!parser.GetValueByFieldName("to_node_id",to_node_id))
			{
				AfxMessageBox("Field to_node_id has not been defined in file input_link.csv. Please check.");
				break;
			}
			if (m_NodeNametoIDMap.find(from_node_id)==m_NodeNametoIDMap.end() ||
				m_NodeNametoIDMap.find(to_node_id)	==m_NodeNametoIDMap.end() )
			{
				AfxMessageBox("Either from_node or to_node_id is not found in node list. Please check.");
				break;
			}

			if(!parser.GetValueByFieldName("number_of_lanes",number_of_lanes))		 number_of_lanes = 1; // default
			if(!parser.GetValueByFieldName("speed_limit_in_mph",speed_limit_in_mph)) speed_limit_in_mph = 20;
			if(!parser.GetValueByFieldName("link_type",type))						 type= 100;
			if(!parser.GetValueByFieldName("from_approach",from_approach))			 from_approach = "0";
			if(!parser.GetValueByFieldName("to_approach",to_approach))				 to_approach   = "0";
			if(!parser.GetValueByFieldName("reversed_link_id",reverse_link_id))		 reverse_link_id = 0;

			string geo_string;
			std::vector<CCoordinate> CoordinateVector;
			if(parser.GetValueByFieldName("geometry",geo_string))
			{
				// overwrite when the field "geometry" exists
				CGeometry geometry(geo_string);
				CoordinateVector = geometry.GetCoordinateList();
			}
			else
			{
				// no geometry information
				CCoordinate cc_from, cc_to; 
				cc_from.X = m_NodeIDMap[m_NodeNametoIDMap[from_node_id]]->ptLL.x;
				cc_from.Y = m_NodeIDMap[m_NodeNametoIDMap[from_node_id]]->ptLL.y;

				cc_to.X = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]]->ptLL.x;
				cc_to.Y = m_NodeIDMap[m_NodeNametoIDMap[to_node_id]]->ptLL.y;

				CoordinateVector.push_back(cc_from);
				CoordinateVector.push_back(cc_to);
			}

			if (0 == from_node_id || 0 == to_node_id)
			{
				m_logFile<<"Warning: Link ID ["<<link_id<<"] has no approach! Discarded."<<endl;
				continue;
			}

			MLink*	pLink = new MLink();
			m_LinkList.push_back(pLink);

			pLink->m_LinkNo = i;
			pLink->m_Name  = name;
			pLink->m_LinkID = link_id;
			pLink->m_FromNodeNumber = from_node_id;
			pLink->m_ToNodeNumber = to_node_id;
			pLink->m_FromNodeID = m_NodeNametoIDMap[from_node_id];
			pLink->m_ToNodeID= m_NodeNametoIDMap[to_node_id];

			for(int si = 0; si < CoordinateVector.size(); si++)
			{
				GDPoint	pt;
				pt.x = L2X(CoordinateVector[si].X,CoordinateVector[si].Y);
				pt.y = L2Y(CoordinateVector[si].Y);
				pLink->m_ShapePoints .push_back (pt);
			}

			pLink->m_NumLanes= max(1,number_of_lanes);
			pLink->m_SpeedLimit= max(20,speed_limit_in_mph);  // minimum speed limit is 20 mph
			pLink->m_LinkType= type; //type, default = 100;

			GetMLinkApproach(from_approach,&nFrom);
			pLink->m_FromNodeApproach	= nFrom;
			GetMLinkApproach(to_approach,&nFrom);
			pLink->m_ToNodeApproach		= nFrom;
			pLink->m_ReverseLinkID		= reverse_link_id;

			m_NodeIDMap[pLink->m_FromNodeID]->outLinks.push_back(pLink);
			m_NodeIDMap[pLink->m_ToNodeID]->inLinks.push_back(pLink);
			//m_NodeIDMap[pLink->m_FromNodeID]->outLinkMap[pLink->m_FromNodeApproach] = pLink;
			//m_NodeIDMap[pLink->m_ToNodeID]->inLinkMap[pLink->m_ToNodeApproach] = pLink;

			//m_LinkNotoLinkMap[i] = pLink;
			i++;
		}
		m_logFile<<"Sumary: All together "<<i<<" links have been read!"<<endl;
		return true;
	}
	else
	{
		AfxMessageBox("Error: File input_link.csv cannot be opened.\n It might be currently used and locked by EXCEL.");
		return false;
	}
}
bool Mustang::CheckDuplicateLink(int link_id)
{
	std::list<MLink*>::iterator iLink;
	for(iLink = m_LinkList.begin();iLink != m_LinkList.end();iLink++)
	{
		if ( link_id == (*iLink)->m_LinkID)
			return false;
	}
	return true;
}
void Mustang::DumpNodeLink2Log()
{
	std::list<MNode*>::iterator iNode;
	for(iNode=m_NodeList.begin();iNode!=m_NodeList.end();iNode++)
	{
		MNode *pNode = (*iNode);
		m_logFile<<"NodeID:"<<pNode->m_NodeID<<" NodeNumber:"<<pNode->m_NodeNumber<<" ProcsType: "<<pNode->m_nProcessType<<" inLink "<<pNode->inLinks.size()<<" outLink "<<pNode->outLinks.size()<<endl;
		std::vector<MLink*>::iterator iLink;
		for(iLink=pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
			m_logFile<<"inLinkID: "<<(*iLink)->m_LinkID<<" toAppr: "<<(*iLink)->m_ToNodeApproach<<endl;
		for(iLink=pNode->outLinks.begin();iLink!=pNode->outLinks.end();iLink++)
			m_logFile<<"outLinkID:"<<(*iLink)->m_LinkID<<" fmAppr: "<<(*iLink)->m_FromNodeApproach<<endl;
	}
}

int MLink::GetLaneCount(int nInOut, int RTL /* = 0*/)
{
	int nCount = 0;
	std::vector<MLane*>::iterator iLane;

	if ( 1 == nInOut )
	{
		if ( 0 == RTL)
		{
			nCount = inLanes.size();
		}
		else if ( 1 == RTL)
		{
			for(iLane=inLanes.begin();iLane!=inLanes.end();iLane++)
			{
				if ((*iLane)->rightTurn)
				{
					nCount ++;
				}
			}
		}
		else if ( 2 == RTL )
		{
			for(iLane=inLanes.begin();iLane!=inLanes.end();iLane++)
			{
				if ((*iLane)->through)
				{
					nCount ++;
				}
			}
		}
		else if ( 3 == RTL )
		{
			for(iLane=inLanes.begin();iLane!=inLanes.end();iLane++)
			{
				if ((*iLane)->leftTurn)
				{
					nCount ++;
				}
			}
		}
		else
		{
		}
	}
	else // 0 == nInOut
	{
		if ( 0 == RTL)
		{
			nCount = outLanes.size();
		}
		else if ( 1 == RTL)
		{
			for(iLane=outLanes.begin();iLane!=outLanes.end();iLane++)
			{
				if ((*iLane)->rightTurn)
				{
					nCount ++;
				}
			}
		}
		else if ( 2 == RTL )
		{
			for(iLane=outLanes.begin();iLane!=outLanes.end();iLane++)
			{
				if ((*iLane)->through)
				{
					nCount ++;
				}
			}
		}
		else if ( 3 == RTL )
		{
			for(iLane=outLanes.begin();iLane!=outLanes.end();iLane++)
			{
				if ((*iLane)->leftTurn)
				{
					nCount ++;
				}
			}
		}
		else
		{
		}
	}

	return nCount;
}
int Mustang::GetMLinkApproach(std::string dir,int* nAppr)
{
	std::string nn("N");
	std::string ee("E");
	std::string ss("S");
	std::string ww("W");

	if ( dir == nn)
		*nAppr = 1;
	else if ( dir == ee) 
		*nAppr = 2;
	else if ( dir == ss) 
		*nAppr = 3;
	else if ( dir == ww) 
		*nAppr = 4;
	else
		*nAppr = 0;

	return *nAppr;
}
int Mustang::GetNeighborApproach(int la, int RTL,int *appr)
{
	int neighbor=0;
	switch (RTL)
	{
	case 1://R
		if		(1	== la) neighbor = 4;
		else if (2	== la) neighbor = 1;
		else if (3	== la) neighbor = 2;
		else			   neighbor = 3;
		break;
	case 2://T
		if		(1	== la) neighbor = 3;
		else if (2	== la) neighbor = 4;
		else if (3	== la) neighbor = 1;
		else			   neighbor = 2;
		break;
	case 3://L
		if		(1	== la) neighbor = 2;
		else if (2	== la) neighbor = 3;
		else if (3	== la) neighbor = 4;
		else			   neighbor = 1;
		break;
	}
	if ( neighbor == 0)
		AfxMessageBox("Neighbor direction is wrong!");
	else
		*appr = neighbor;

	return neighbor;
}

bool Mustang::ReadInputLaneCSV(std::string strFileName)
{
	return true;
}
bool Mustang::ReadInputLinkTypeCSV(std::string strFileName)
{
	m_logFile<< "ReadInputLinkTypeCSV function called!"<<endl;
	m_szLinkTypes = _T("");     // This string is reserved for attributes of LINKTYPES. The following 3 are in groups
	
	CCSVParser parser;
	if (parser.OpenCSVFile(strFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int link_type;
			std::string link_type_name;
			int freeway_flag;
			int ramp_flag;
			int arterial_flag;
			CString tt;

			if(parser.GetValueByFieldName("link_type",link_type) == false)			 continue;
			if(parser.GetValueByFieldName("link_type_name",link_type_name) == false) continue;
			if(parser.GetValueByFieldName("freeway_flag",freeway_flag) == false)	 freeway_flag=0;
			if(parser.GetValueByFieldName("ramp_flag",ramp_flag) == false)			 ramp_flag=0;
			if(parser.GetValueByFieldName("arterial_flag",arterial_flag) == false)	 arterial_flag=0;

			tt.Format("%d",link_type);
			m_szLinkTypeNo.Add(tt);
			m_szLinkTypeName.Add(link_type_name.c_str());
			m_szDrivingBehavior.Add(_T("Urban"));
		}
	}
	return true;
}
bool Mustang::ReadInputDemandCSV(std::string strFileName)
{
	m_logFile<< "ReadInputDemandCSV function called!"<<endl;
	CCSVParser parser;
	if (parser.OpenCSVFile(strFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int from_zone_id;
			int to_zone_id;
			float number_of_vehicle_trips_type1;
			float number_of_vehicle_trips_type2;
			float number_of_vehicle_trips_type3;
			float number_of_vehicle_trips_type4;
			int starting_time_in_min;
			int ending_time_in_min;

			if(parser.GetValueByFieldName("from_zone_id",from_zone_id) == false)
				break;

			if(parser.GetValueByFieldName("to_zone_id",to_zone_id) == false)
				break;

			if(parser.GetValueByFieldName("number_of_vehicle_trips_type1",number_of_vehicle_trips_type1) == false)
				break;
			if(parser.GetValueByFieldName("number_of_vehicle_trips_type2",number_of_vehicle_trips_type2) == false)
				break;
			if(parser.GetValueByFieldName("number_of_vehicle_trips_type3",number_of_vehicle_trips_type3) == false)
				break;
			if(parser.GetValueByFieldName("number_of_vehicle_trips_type4",number_of_vehicle_trips_type4) == false)
				break;

			if(parser.GetValueByFieldName("starting_time_in_min",starting_time_in_min) == false)
				break;

			if(parser.GetValueByFieldName("ending_time_in_min",ending_time_in_min) == false)
				break;

			MDemand* pDemand = new MDemand();
			pDemand->m_nFromZone = from_zone_id;
			pDemand->m_nToZone = to_zone_id;
			pDemand->m_fDemand[0] = number_of_vehicle_trips_type1;
			pDemand->m_fDemand[1] = number_of_vehicle_trips_type2;
			pDemand->m_fDemand[2] = number_of_vehicle_trips_type3;
			pDemand->m_fDemand[3] = number_of_vehicle_trips_type4;
			pDemand->m_nStartMin  = starting_time_in_min;
			pDemand->m_nEndMin	  = ending_time_in_min;

			m_DemandVector.push_back(pDemand);
			i++;
		}
		std::vector<MDemand*>::iterator iDemand;
		for(iDemand=m_DemandVector.begin();iDemand!=m_DemandVector.end();iDemand++)
			m_logFile<<"Demand:"<<(*iDemand)->m_nFromZone<<","<<(*iDemand)->m_nToZone<<"["<<(*iDemand)->m_fDemand[0]<<"]"<<endl;
		return true;
	}
	else
	{
		AfxMessageBox("Error: File input_demand.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}
}
bool Mustang::ReadInputZoneCSV(std::string strFileName)
{
	m_logFile<< "ReadInputZoneCSV function called!"<<endl;
	CCSVParser parser;
	if (parser.OpenCSVFile(strFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int node_id;
			int zone_id;
			MNode* pNode = 0;

			if(parser.GetValueByFieldName("node_id",node_id) == false)
				break;

			if(parser.GetValueByFieldName("zone_id",zone_id) == false)
				break;

			MZone* pZone = new MZone();
			pZone->m_nID = zone_id;
			pNode = m_NodeIDMap[m_NodeNametoIDMap[node_id]];
			if (!pNode)
			{
				CString strError;
				strError.Format("In zone_id = %d, node_id = %d does not exist!",zone_id,node_id);
				m_logFile<<strError<<endl;
				AfxMessageBox(strError);
			}
			pZone->m_nodes.push_back(pNode);
			pZone->pt.x = pNode->pt.x;
			pZone->pt.y = pNode->pt.y;

			m_ZoneVector.push_back(pZone);
			i++;
		}
		std::vector<MZone*>::iterator iZone;
		for(iZone=m_ZoneVector.begin();iZone!=m_ZoneVector.end();iZone++)
		{
			m_logFile<<" zone_id: "<<(*iZone)->m_nID<<" node number:["<<(*iZone)->m_nodes.size()<<"]"<<endl;
			std::vector<MNode*>::iterator iNode;
			int i=0;
			for(iNode=(*iZone)->m_nodes.begin();iNode!=(*iZone)->m_nodes.end();iNode++)
				m_logFile<<"["<<++i<<"] node_id: "<<(*iNode)->m_NodeNumber<<endl;
		}
		return true;
	}
	else
	{
		AfxMessageBox("Error: File input_zone.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}
}
bool Mustang::ReadInputSignalCSV(std::string strFileName)
{
	//生成一个默认的定时方案，1为N、S直行；2为E、W直行；3为N、S左转；4为W、E左转。直行各40秒、左转各20秒
	return true;
}
int  Mustang::GetSCNO(int nNodeNumber)
{
	int n = 9133;
	return n;
}
int  Mustang::GetSGNO(int nNodeNumber, int appr, int RTL)
{
	//int		SG_NORTH_RIGHT	=1;
	//int		SG_NORTH_THROUGH=1;
	//int		SG_SOUTH_RIGHT	=1;
	//int		SG_SOUTH_THROUGH=1;
	//int		SG_EAST_RIGHT	=2;
	//int		SG_EAST_THROUGH	=2;
	//int		SG_WEST_RIGHT	=2;
	//int		SG_WEST_THROUGH	=2;
	//int		SG_EAST_LEFT	=3;
	//int		SG_WEST_LEFT	=3;
	//int		SG_NORTH_LEFT	=4;
	//int		SG_SOUTH_LEFT	=4;

	int SigGroupNo = 1;
	if ( 1 == appr || 3 == appr )
	{
		if ( 1 == RTL || 2 == RTL)
			SigGroupNo = 1;
		else // ( 3 == RTL)
			SigGroupNo = 4;
	}
	else // ( 4 == appr || 2 == appr)
	{
		if ( 1 == RTL || 2 == RTL)
			SigGroupNo = 2;
		else // ( 3 == RTL)
			SigGroupNo = 3;
	}
	return SigGroupNo;
}
bool Mustang::ClassifyNodes(void)
{
	// classify node types
	std::list<MNode*>::iterator iNode;
	MNode* pNode;
	std::vector<MLink*>::iterator iLink;
	int i=0;

	// classify nodes
	for (iNode = m_NodeList.begin(); iNode != m_NodeList.end(); iNode++)
	{
		pNode = (*iNode);
		int nInDegree = pNode->inLinks.size();
		int nOutDegree= pNode->outLinks.size();

		if ( nInDegree ==0 || nOutDegree == 0)
		{
			pNode->m_nProcessType = 0;
			pNode->nControlType   = 0;
		}
		else if (nInDegree == 1 && nOutDegree == 1)
		{
			iLink=pNode->inLinks.begin();
			int nReverseID = (*iLink)->m_ReverseLinkID;
			iLink=pNode->outLinks.begin();
			int nOutLinkID = (*iLink)->m_LinkID;
			if (nReverseID == nOutLinkID && nReverseID != 0)
				pNode->m_nProcessType = 1;
			else
				pNode->m_nProcessType = 2;
			pNode->nControlType = 0;
		}
		else if (nInDegree == 2 && nOutDegree == 2)
		{
			pNode->m_nProcessType =2;
			pNode->nControlType   =0;
		}
		else if (nInDegree ==3 && nOutDegree == 3)
		{
			// in和out缺少的方向相同；没有重复方向
			int nMiss = pNode->CheckMissingApproach();

			if (nMiss >= 1 && nMiss <=4)
			{
				pNode->m_nProcessType = 3;
				pNode->nControlType   = 1;
				// 赋方向映射
				for(iLink=pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
					pNode->inLinkMap[(*iLink)->m_ToNodeApproach] = (*iLink);
				for(iLink=pNode->outLinks.begin();iLink!=pNode->outLinks.end();iLink++)
					pNode->outLinkMap[(*iLink)->m_FromNodeApproach] = (*iLink);
			}
			else
			{
				pNode->m_nProcessType = 5;
				pNode->nControlType   = 0;
			}
		}
		else if (nInDegree == 4 && nOutDegree == 4)
		{
			int nIns=0,nOuts=0;
			for(iLink=pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
				nIns += (*iLink)->m_ToNodeApproach;
			for(iLink=pNode->outLinks.begin();iLink!=pNode->outLinks.end();iLink++)
				nOuts += (*iLink)->m_FromNodeApproach;

			if (nIns == nOuts && nIns == 10)
			{
				pNode->m_nProcessType = 4;
				pNode->nControlType   = 1;
				// 赋方向映射
				for(iLink=pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
					pNode->inLinkMap[(*iLink)->m_ToNodeApproach] = (*iLink);
				for(iLink=pNode->outLinks.begin();iLink!=pNode->outLinks.end();iLink++)
					pNode->outLinkMap[(*iLink)->m_FromNodeApproach] = (*iLink);
			}
			else
			{
				pNode->m_nProcessType = 5;
				pNode->nControlType   = 0;
			}
		}
		else
		{
			pNode->m_nProcessType = 5;
			pNode->nControlType   = 0;
		}
	}
	return true;
}

bool Mustang::CreateDefaultLanes(void)
{
	// create by default leftturn pocket for standard crossing and T junction
	std::list<MNode*>::iterator iNode;
	MNode* pNode;
	std::vector<MLink*>::iterator iLink;
	MLink* pLink;
	MLane* pLane;
	int i=0;
	// create lanes
	// 根据inlink和outlink中laneNumber生成lane：默认给标准十字路口和T路口生成leftTurn Pocket，边界点、不规则点和其他点只生成lane，无额外的pocket
	// 标准十字和T字，lane均给出方向；其他node的lane，只有直行属性
	for (iNode = m_NodeList.begin(); iNode != m_NodeList.end(); iNode++)
	{
		pNode = (*iNode);
		//if (pNode->m_NodeID == 27 || pNode->m_NodeID == 38)
		//{
		//	int abc;
		//	abc = 1 * 200;
		//}
		if ( 4 == pNode->m_nProcessType) // ltp at all inLinks
		{
			for(iLink=pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
			{
				pLink = (*iLink);				 
				for(i=1;i<=pLink->m_NumLanes;i++)
				{
					pLane = new MLane();
					pLink->inLanes.push_back(pLane);
					pLane->m_Index = i;
					pLane->m_LinkID = pLink->m_LinkID;
					pLane->m_NodeID = pNode->m_NodeID;
					pLane->through = 1;
					pLane->m_PocketLength = 0.0;
					pLane->leftTurn = 0;
					pLane->rightTurn = (1==i)?1:0; // 第一条lane准右转
					m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] in, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
						<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
				}
				pLane = new MLane(); // extra leftturn pocket
				pLink->inLanes.push_back(pLane);
				pLane->m_Index = i;
				pLane->m_LinkID = pLink->m_LinkID;
				pLane->m_NodeID = pNode->m_NodeID;
				pLane->through = 0;
				pLane->m_PocketLength = 15.0;
				pLane->leftTurn = 1;
				pLane->rightTurn = 0; 
				m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] in, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
					<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
			}
			for( iLink = pNode->outLinks.begin(); iLink != pNode->outLinks.end(); iLink++)// out lane
			{
				pLink = (*iLink);				 
				for(int i=1;i<=pLink->m_NumLanes;i++)
				{
					pLane = new MLane();
					pLink->outLanes.push_back(pLane);
					pLane->m_Index = i;
					pLane->m_LinkID = pLink->m_LinkID;
					pLane->m_NodeID = pNode->m_NodeID;
					pLane->through = 1;         
					pLane->m_PocketLength = 0.0;
					pLane->leftTurn = 1;
					pLane->rightTurn = 1; 
					m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] out, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
						<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
				}
			}
		}
		else if ( 3 == pNode->m_nProcessType) //ltp at two inLinks
		{
			int nIns=0,missingAppr=0;
			for(iLink=pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
				nIns += (*iLink)->m_ToNodeApproach;
			missingAppr = 10 - nIns ; 
			// 找出6根link
			int branchAppr,leftAppr,rightAppr;
			GetNeighborApproach(missingAppr,2,&branchAppr);
			GetNeighborApproach(branchAppr,3,&leftAppr);   //RTL
			GetNeighborApproach(branchAppr,1,&rightAppr);

			MLink* piBLink = pNode->inLinkMap[branchAppr];
			MLink* poBLink = pNode->outLinkMap[branchAppr];

			MLink* piLLink = pNode->inLinkMap[leftAppr];
			MLink* poLLink = pNode->outLinkMap[leftAppr];

			MLink* piRLink = pNode->inLinkMap[rightAppr];
			MLink* poRLink = pNode->outLinkMap[rightAppr];

			// 产生所有的车道
			for(i=1;i<=piBLink->m_NumLanes;i++) // branch in, all lanes right turn, extra left turn pocket
			{
				pLane = new MLane();
				piBLink->inLanes.push_back(pLane);
				pLane->m_Index = i;
				pLane->m_LinkID = piBLink->m_LinkID;
				pLane->m_NodeID = pNode->m_NodeID;
				pLane->through = 0;
				pLane->m_PocketLength = 0.0;
				pLane->leftTurn = 0;
				pLane->rightTurn =1; // 第一条lane准右转
				m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] in, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
					<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
			}
			pLane = new MLane(); // extra leftturn pocket
			piBLink->inLanes.push_back(pLane);
			pLane->m_Index = i;
			pLane->m_LinkID = piBLink->m_LinkID;
			pLane->m_NodeID = pNode->m_NodeID;
			pLane->through = 0;
			pLane->m_PocketLength = 15.0;
			pLane->leftTurn = 1;
			pLane->rightTurn = 0; 
			m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] in, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
				<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
			for(i=1;i<=poBLink->m_NumLanes;i++)// branch out
			{
				pLane = new MLane();
				poBLink->outLanes.push_back(pLane);
				pLane->m_Index = i;
				pLane->m_LinkID = poBLink->m_LinkID;
				pLane->m_NodeID = pNode->m_NodeID;
				pLane->through = 1;         
				pLane->m_PocketLength = 0.0;
				pLane->leftTurn = 1;
				pLane->rightTurn = 1; 
				m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] out, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
					<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
			}

			for(i=1;i<=piRLink->m_NumLanes;i++) // right in, all lanes through, extra left turn pocket
			{
				pLane = new MLane();
				piRLink->inLanes.push_back(pLane);
				pLane->m_Index = i;
				pLane->m_LinkID = piRLink->m_LinkID;
				pLane->m_NodeID = pNode->m_NodeID;
				pLane->through = 1;
				pLane->m_PocketLength = 0.0;
				pLane->leftTurn = 0;
				pLane->rightTurn =0; 
				m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] in, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
					<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
			}
			pLane = new MLane(); // extra leftturn pocket
			piRLink->inLanes.push_back(pLane);
			pLane->m_Index = i;
			pLane->m_LinkID = piRLink->m_LinkID;
			pLane->m_NodeID = pNode->m_NodeID;
			pLane->through = 0;
			pLane->m_PocketLength = 15.0;
			pLane->leftTurn = 1;
			pLane->rightTurn = 0; 
			m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] in, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
				<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;

			for(i=1;i<=poRLink->m_NumLanes;i++)// right out
			{
				pLane = new MLane();
				poRLink->outLanes.push_back(pLane);
				pLane->m_Index = i;
				pLane->m_LinkID = poRLink->m_LinkID;
				pLane->m_NodeID = pNode->m_NodeID;
				pLane->through = 1;         
				pLane->m_PocketLength = 0.0;
				pLane->leftTurn = 1;
				pLane->rightTurn = 1; 
				m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] out, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
					<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
			}	
			
			for(i=1;i<=piLLink->m_NumLanes;i++) // left in, all lanes through, first lane share right turn
			{
				pLane = new MLane();
				piLLink->inLanes.push_back(pLane);
				pLane->m_Index = i;
				pLane->m_LinkID = piLLink->m_LinkID;
				pLane->m_NodeID = pNode->m_NodeID;
				pLane->through = 1;
				pLane->m_PocketLength = 0.0;
				pLane->leftTurn = 0;
				pLane->rightTurn =(1==i)?1:0; 
				m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] in, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
					<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
			}
			for(i=1;i<=poLLink->m_NumLanes;i++)// right out
			{
				pLane = new MLane();
				poLLink->outLanes.push_back(pLane);
				pLane->m_Index = i;
				pLane->m_LinkID = poLLink->m_LinkID;
				pLane->m_NodeID = pNode->m_NodeID;
				pLane->through = 1;         
				pLane->m_PocketLength = 0.0;
				pLane->leftTurn = 1;
				pLane->rightTurn = 1; 
				m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] out, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
					<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
			}	
		}
		else
		{
			for(iLink=pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
			{
				pLink = (*iLink);
				for(i=1;i<=pLink->m_NumLanes;i++)
				{
					pLane = new MLane();
					pLink->inLanes.push_back(pLane);
					pLane->m_Index = i;
					pLane->m_LinkID = pLink->m_LinkID;
					pLane->m_NodeID = pNode->m_NodeID;
					pLane->through = 1;
					pLane->m_PocketLength = 0.0;
					pLane->leftTurn = (i==pLink->m_NumLanes)?1:0; //最后一条准左转
					pLane->rightTurn = (1==i)?1:0; // 第一条lane准右转
					m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] out, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
						<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
				}
			}
			for(iLink=pNode->outLinks.begin();iLink!=pNode->outLinks.end();iLink++)
			{
				pLink = (*iLink);
				for(i=1;i<=pLink->m_NumLanes;i++)
				{
					pLane = new MLane();
					pLink->outLanes.push_back(pLane);
					pLane->m_Index = i;
					pLane->m_LinkID = pLink->m_LinkID;
					pLane->m_NodeID = pNode->m_NodeID;
					pLane->through = 0;
					pLane->m_PocketLength = 0.0;
					pLane->leftTurn = 0; 
					pLane->rightTurn = 0; 
					m_logFile<<"Lane created: NodeNumber["<<m_NodeIDtoNameMap[pLane->m_NodeID]<<"] out, linkID: ["<<pLane->m_LinkID<<"] index: "<<pLane->m_Index
						<<" RTL ["<<pLane->rightTurn<<","<<pLane->through<<","<<pLane->leftTurn<<"] pocket: "<<pLane->m_PocketLength<<endl;
				}
			}
		}
	}

	return true;
}
bool Mustang::ProcessLanes(void)
{
	// 生成所有laneTurn，标准十字路口和T路口同时生成信号
	// inLinks的循环；outLinks的循环；根据方向判断RTL、如为u turn则continue; new一个turn；同时判断phase，简易和NEMA
	// 联接node和边界node的判断和处理：边界点至多只有一in一out、且互为reverse，不生成turn；联接node两in两out、或一in一out但不为reverse，生成turn但不生成信号。此二种情况均不考虑方向
	std::list<MNode*>::iterator iNode;
	MNode* pNode;
	std::vector<MLink*>::iterator iLink;
	MLink* pLink;
	int i;
	int nInCount,nOutCount,nTurnCount,nLeftTurnInIndex,nOffsetIndexIn,nOffsetIndexOut;
	int branchAppr=0;
	int leftAppr=0;
	int rightAppr=0;


	for (iNode = m_NodeList.begin(); iNode != m_NodeList.end(); iNode++)
	{
		pNode = (*iNode);
		if (pNode->m_nProcessType == 4)
		{
			
			//根据车道的转向属性和方位，产生laneturn，并给出默认的信号控制
			MLink *piLink;
			MLink *poRLink,*poTLink,*poLLink;
			int i;

			for(int appr=1;appr <=4;appr++)
			{
				piLink = pNode->inLinkMap[appr];
				int nRt=0;
				GetNeighborApproach(appr,1,&nRt);
				poRLink = pNode->outLinkMap[nRt];
				GetNeighborApproach(appr,2,&nRt);
				poTLink = pNode->outLinkMap[nRt];
				GetNeighborApproach(appr,3,&nRt);
				poLLink = pNode->outLinkMap[nRt];
				// 右转，与西出link的车道同号联接，直至不能配对（下同！）；
				int nInCount = piLink->GetLaneCount(1,1);
				int nOutCount= poRLink->GetLaneCount(0);
				int nTurnCount = min(nInCount,nOutCount);
				for(i=1;i<=nTurnCount;i++)
				{
					MLaneTurn *pLaneTurn = new MLaneTurn();
					pNode->LaneTurns.push_back(pLaneTurn);

					pLaneTurn->nFromLinkId = piLink->m_LinkID;
					pLaneTurn->nFromIndex  = i;
					pLaneTurn->nToLinkId   = poRLink->m_LinkID;
					pLaneTurn->nToIndex	   = i;
					pLaneTurn->nRTL		   = 1;
					pLaneTurn->nSCNO       = GetSCNO(pNode->m_NodeNumber);
					pLaneTurn->nSignalGroupNo = GetSGNO(pNode->m_NodeNumber,appr,1);
				}
				// 直行，与南出link的车道同号或错号联接
				// 入直行车道大于出车道，则从入的2号开始联接，即把入1作为Right turnonly；
				//           等于      ，同号联接
				//           小于      ，则从出的2好开始联接，即把出1作为right turnonly。
				nInCount = piLink->GetLaneCount(1,2);
				nOutCount= poTLink->GetLaneCount(0);
				int nOffsetIndexIn,nOffsetIndexOut;
				if ( nInCount > nOutCount)
				{
					nTurnCount = nOutCount;
					nOffsetIndexIn = 1;
					nOffsetIndexOut= 0;
				}
				else if ( nInCount == nOutCount)
				{
					nTurnCount = nOutCount;
					nOffsetIndexIn = 0;
					nOffsetIndexOut= 0;
				}
				else // nInCount < nOutCount
				{
					nTurnCount = nInCount;
					nOffsetIndexIn = 0;
					nOffsetIndexOut= 1;
				}
				for(i=1;i<=nTurnCount;i++)
				{
					MLaneTurn *pLaneTurn = new MLaneTurn();
					pNode->LaneTurns.push_back(pLaneTurn);

					pLaneTurn->nFromLinkId = piLink->m_LinkID;
					pLaneTurn->nFromIndex  = i + nOffsetIndexIn;
					pLaneTurn->nToLinkId   = poTLink->m_LinkID;
					pLaneTurn->nToIndex	   = i + nOffsetIndexOut;
					pLaneTurn->nRTL        = 2;
					pLaneTurn->nSCNO       = GetSCNO(pNode->m_NodeNumber);
					pLaneTurn->nSignalGroupNo = GetSGNO(pNode->m_NodeNumber,appr,2);
				}
				// 左转，与东出link的车道从大号开始对应联接
				nInCount = piLink->GetLaneCount(1,3);
				nOutCount= poLLink->GetLaneCount(0);
				nTurnCount = min(nInCount,nOutCount);
				int nLeftTurnInIndex = piLink->GetLaneCount(1,0);
				for(i=0;i<nTurnCount;i++)
				{
					MLaneTurn *pLaneTurn = new MLaneTurn();
					pNode->LaneTurns.push_back(pLaneTurn);

					pLaneTurn->nFromLinkId = piLink->m_LinkID;
					pLaneTurn->nFromIndex  = nLeftTurnInIndex - i;
					pLaneTurn->nToLinkId   = poLLink->m_LinkID;
					pLaneTurn->nToIndex	   = nOutCount - i;
					pLaneTurn->nRTL		   = 3;
					pLaneTurn->nSCNO       = GetSCNO(pNode->m_NodeNumber);
					pLaneTurn->nSignalGroupNo = GetSGNO(pNode->m_NodeNumber,appr,3);
				}
			}
			
		}
		else if (pNode->m_nProcessType == 3)
		{
			// T type 关键是确定出主干道和唯一的一个branch。缺少哪个方向，对应的方向就是branch
			// 3 个方向加起来的和，应相等，同时也知道缺少哪个方向了
			
			int  nTotal=0,missingAppr;
			for(iLink = pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
				nTotal += (*iLink)->m_ToNodeApproach;
			missingAppr = 10 - nTotal;

			// 找出6根link
			GetNeighborApproach(missingAppr,2,&branchAppr);
			GetNeighborApproach(branchAppr,3,&leftAppr);   //RTL
			GetNeighborApproach(branchAppr,1,&rightAppr);

			MLink* piBLink = pNode->inLinkMap[branchAppr];
			MLink* poBLink = pNode->outLinkMap[branchAppr];

			MLink* piLLink = pNode->inLinkMap[leftAppr];
			MLink* poLLink = pNode->outLinkMap[leftAppr];

			MLink* piRLink = pNode->inLinkMap[rightAppr];
			MLink* poRLink = pNode->outLinkMap[rightAppr];	
			
			// 产生laneTurns
			// ① branch in, to right out and to left out
			nInCount = piBLink->GetLaneCount(1,1);
			nOutCount= poRLink->GetLaneCount(0);
			nTurnCount = min(nInCount,nOutCount);
			int appr = piBLink->m_ToNodeApproach;
			for(i=1;i<=nTurnCount;i++)
			{
				MLaneTurn *pLaneTurn = new MLaneTurn();
				pNode->LaneTurns.push_back(pLaneTurn);

				pLaneTurn->nFromLinkId = piBLink->m_LinkID;
				pLaneTurn->nFromIndex  = i;
				pLaneTurn->nToLinkId   = poRLink->m_LinkID;
				pLaneTurn->nToIndex	   = i;
				pLaneTurn->nRTL		   = 1; // right turn
				if ( 1 == pNode->nControlType)
				{
					pLaneTurn->nSCNO       = GetSCNO(pNode->m_NodeNumber);
					pLaneTurn->nSignalGroupNo = GetSGNO(pNode->m_NodeNumber,appr,1);
				}
			}
			nInCount = piBLink->GetLaneCount(1,3);
			nOutCount= poLLink->GetLaneCount(0);
			nTurnCount = min(nInCount,nOutCount);
			nLeftTurnInIndex = piBLink->GetLaneCount(1,0);
			for(i=0;i<nTurnCount;i++)
			{
				MLaneTurn *pLaneTurn = new MLaneTurn();
				pNode->LaneTurns.push_back(pLaneTurn);

				pLaneTurn->nFromLinkId = piBLink->m_LinkID;
				pLaneTurn->nFromIndex  = nLeftTurnInIndex - i;
				pLaneTurn->nToLinkId   = poLLink->m_LinkID;
				pLaneTurn->nToIndex	   = nOutCount - i;
				pLaneTurn->nRTL		   = 3; //left turn
				if ( 1 == pNode->nControlType)
				{
					pLaneTurn->nSCNO       = GetSCNO(pNode->m_NodeNumber);
					pLaneTurn->nSignalGroupNo = GetSGNO(pNode->m_NodeNumber,appr,3);
				}
			}
			// ② right in, to branch(left turn) and left(through) out
			nInCount = piRLink->GetLaneCount(1,3);
			nOutCount= poBLink->GetLaneCount(0);
			nTurnCount = min(nInCount,nOutCount);
			nLeftTurnInIndex = piRLink->GetLaneCount(1,0);
			appr = piRLink->m_ToNodeApproach;
			for(i=0;i<nTurnCount;i++)
			{
				MLaneTurn *pLaneTurn = new MLaneTurn();
				pNode->LaneTurns.push_back(pLaneTurn);

				pLaneTurn->nFromLinkId = piRLink->m_LinkID;
				pLaneTurn->nFromIndex  = nLeftTurnInIndex - i;
				pLaneTurn->nToLinkId   = poBLink->m_LinkID;
				pLaneTurn->nToIndex	   = nOutCount - i;
				pLaneTurn->nRTL  = 3;
				if ( 1 == pNode->nControlType)
				{
					pLaneTurn->nSCNO       = GetSCNO(pNode->m_NodeNumber);
					pLaneTurn->nSignalGroupNo = GetSGNO(pNode->m_NodeNumber,appr,3);
				}
			}
			nInCount = piRLink->GetLaneCount(1,2);
			nOutCount= poLLink->GetLaneCount(0);
			nOffsetIndexIn,nOffsetIndexOut;
			if ( nInCount > nOutCount)
			{
				nTurnCount = nOutCount;
				nOffsetIndexIn = 1;
				nOffsetIndexOut= 0;
			}
			else if ( nInCount == nOutCount)
			{
				nTurnCount = nOutCount;
				nOffsetIndexIn = 0;
				nOffsetIndexOut= 0;
			}
			else // nInCount < nOutCount
			{
				nTurnCount = nInCount;
				nOffsetIndexIn = 0;
				nOffsetIndexOut= 1;
			}
			for(i=1;i<=nTurnCount;i++)
			{
				MLaneTurn *pLaneTurn = new MLaneTurn();
				pNode->LaneTurns.push_back(pLaneTurn);

				pLaneTurn->nFromLinkId = piRLink->m_LinkID;
				pLaneTurn->nFromIndex  = i + nOffsetIndexIn;
				pLaneTurn->nToLinkId   = poLLink->m_LinkID;
				pLaneTurn->nToIndex	   = i + nOffsetIndexOut;
				pLaneTurn->nRTL = 2; //through
				if ( 1 == pNode->nControlType)
				{
					pLaneTurn->nSCNO       = GetSCNO(pNode->m_NodeNumber);
					pLaneTurn->nSignalGroupNo = GetSGNO(pNode->m_NodeNumber,appr,2);
				}
			}
			// ③ left in, to branch(right turn) and right(through) out
			nInCount = piLLink->GetLaneCount(1,1);
			nOutCount= poBLink->GetLaneCount(0);
			nTurnCount = min(nInCount,nOutCount);
			appr = piLLink->m_ToNodeApproach;
			for(i=1;i<=nTurnCount;i++)
			{
				MLaneTurn *pLaneTurn = new MLaneTurn();
				pNode->LaneTurns.push_back(pLaneTurn);

				pLaneTurn->nFromLinkId = piLLink->m_LinkID;
				pLaneTurn->nFromIndex  = i;
				pLaneTurn->nToLinkId   = poBLink->m_LinkID;
				pLaneTurn->nToIndex	   = i;
				pLaneTurn->nRTL  = 1;
				if ( 1 == pNode->nControlType)
				{
					pLaneTurn->nSCNO       = GetSCNO(pNode->m_NodeNumber);
					pLaneTurn->nSignalGroupNo = GetSGNO(pNode->m_NodeNumber,appr,1);
				}
			}
			nInCount = piLLink->GetLaneCount(1,2);
			nOutCount= poRLink->GetLaneCount(0);
			nOffsetIndexIn,nOffsetIndexOut;
			if ( nInCount > nOutCount)
			{
				nTurnCount = nOutCount;
				nOffsetIndexIn = 1;
				nOffsetIndexOut= 0;
			}
			else if ( nInCount == nOutCount)
			{
				nTurnCount = nOutCount;
				nOffsetIndexIn = 0;
				nOffsetIndexOut= 0;
			}
			else // nInCount < nOutCount
			{
				nTurnCount = nInCount;
				nOffsetIndexIn = 0;
				nOffsetIndexOut= 1;
			}
			for(i=1;i<=nTurnCount;i++)
			{
				MLaneTurn *pLaneTurn = new MLaneTurn();
				pNode->LaneTurns.push_back(pLaneTurn);

				pLaneTurn->nFromLinkId = piLLink->m_LinkID;
				pLaneTurn->nFromIndex  = i + nOffsetIndexIn;
				pLaneTurn->nToLinkId   = poRLink->m_LinkID;
				pLaneTurn->nToIndex	   = i + nOffsetIndexOut;
				pLaneTurn->nRTL = 2;
				if ( 1 == pNode->nControlType)
				{
					pLaneTurn->nSCNO       = GetSCNO(pNode->m_NodeNumber);
					pLaneTurn->nSignalGroupNo = GetSGNO(pNode->m_NodeNumber,appr,2);
				}
			}
			
		}
		else if (pNode->m_nProcessType == 2)
		{
			for(iLink=pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
			{
				pLink = (*iLink);
				std::vector<MLink*>::iterator ioLink;
				for(ioLink=pNode->outLinks.begin();ioLink!=pNode->outLinks.end();ioLink++)
				{
					MLink *poLink = (*ioLink);
					if (pLink->m_ReverseLinkID == poLink->m_LinkID )
						continue;
					int nTurnCount = min(pLink->m_NumLanes,poLink->m_NumLanes);
					for(i=1;i<=nTurnCount;i++)
					{
						MLaneTurn* pLaneTurn = new MLaneTurn();
						pNode->LaneTurns.push_back(pLaneTurn);
						pLaneTurn->nFromLinkId = pLink->m_LinkID;
						pLaneTurn->nFromIndex  = i;
						pLaneTurn->nToLinkId   = poLink->m_LinkID;
						pLaneTurn->nToIndex    = i;
						pLaneTurn->nRTL		   = 2;  //connector is considered as through
						pLaneTurn->nSignalGroupNo = 0;
						pLaneTurn->nSCNO          = 0;
					}
				}
			}
		}
		else if (pNode->m_nProcessType == 1 || pNode->m_nProcessType == 0)
		{
			// do nothing for the border node
		}
		else
		{
			for(iLink=pNode->inLinks.begin();iLink!=pNode->inLinks.end();iLink++)
			{
				pLink = (*iLink);
				std::vector<MLink*>::iterator ioLink;
				for(ioLink=pNode->outLinks.begin();ioLink!=pNode->outLinks.end();ioLink++)
				{
					MLink *poLink = (*ioLink);
					if (pLink->m_ReverseLinkID == poLink->m_LinkID )
						continue;
					int nTurnCount = min(pLink->m_NumLanes,poLink->m_NumLanes);
					for(i=1;i<=nTurnCount;i++)
					{
						MLaneTurn* pLaneTurn = new MLaneTurn();
						pNode->LaneTurns.push_back(pLaneTurn);
						pLaneTurn->nFromLinkId = pLink->m_LinkID;
						pLaneTurn->nFromIndex  = i;
						pLaneTurn->nToLinkId   = poLink->m_LinkID;
						pLaneTurn->nToIndex    = i;
						pLaneTurn->nRTL		   = 0;  // can not judged
						pLaneTurn->nSignalGroupNo = 0;
						pLaneTurn->nSCNO          = 0;
					}
				}
			}
		}
		// dump all the laneturns
		m_logFile<<endl;
		m_logFile<<"NodeNumber: "<<pNode->m_NodeNumber<<" LaneTurns: ["<<pNode->LaneTurns.size()<<"]"<<endl;
		std::vector<MLaneTurn*>::iterator iTurn;
		for(iTurn=pNode->LaneTurns.begin();iTurn!=pNode->LaneTurns.end();iTurn++)
		{
			m_logFile<<"RTL="<<(*iTurn)->nRTL<<" FromLink "<<(*iTurn)->nFromLinkId<<" Lane "<<(*iTurn)->nFromIndex
				<<" ToLink "<<(*iTurn)->nToLinkId<<" Lane "<<(*iTurn)->nToIndex
				<<"SC ["<<(*iTurn)->nSCNO<<"] SG["<<(*iTurn)->nSignalGroupNo<<"]"<<endl;
		}
		m_logFile<<endl;
		
	} // end of for node list
	return true;
}

bool Mustang::CreateANMFile(std::string strFileName)
{
	int i,iCount,j,jCount;
	CString szTemp;
	char cTemp[100];

	setlocale(LC_ALL,"chs");

	HRESULT hr;
	hr = ::CoInitialize(NULL);

	//WriteTest(szFileName);
	//::getchar();
    MSXML2::IXMLDOMDocumentPtr pXMLDocument;  
    hr = pXMLDocument.CreateInstance(__uuidof(MSXML2::DOMDocument));
	
	//MSXML2::IXMLDocument2Ptr pDoc;
    //HRESULT hr = pDoc.CreateInstance("DOMDocument4.0");
	
	if (FAILED(hr))
		return false;
	//MSXML2::IXMLDOMProcessingInstructionPtr pPtr;
	//pPtr =  pXMLDocument->createProcessingInstruction("xml", "version=\"1.0\"encoding=\"UTF-8\"");
	//pXMLDocument->appendChild(pPtr);
	
	// AbstractNetwork
	MSXML2::IXMLDOMElementPtr pNode = pXMLDocument->createElement(_T("ABSTRACTNETWORKMODEL"));
	pNode->setAttribute(_T("VERSNO"),(_bstr_t)m_szVersNo);
	pNode->setAttribute(_T("FROMTIME"),(_bstr_t)m_szFromTime);
	pNode->setAttribute(_T("TOTIME"),(_bstr_t)m_szToTime);
	pNode->setAttribute(_T("NAME"),(_bstr_t)m_szName);
	pXMLDocument->appendChild(pNode);
	MSXML2::IXMLDOMElementPtr pAbstractNetworkNode = pNode;
	
	// Network
    MSXML2::IXMLDOMElementPtr pNetworkNode = pXMLDocument->createElement(_T("NETWORK"));
	pNetworkNode->setAttribute(_T("LEFTHANDTRAFFIC"),m_nLeftHandTraffic);
	pAbstractNetworkNode->appendChild(pNetworkNode);

	// Define node pointers for different section
	MSXML2::IXMLDOMElementPtr pVehTypesNode		= pXMLDocument->createElement(_T("VEHTYPES"));
	MSXML2::IXMLDOMElementPtr pVehClassesNode	= pXMLDocument->createElement(_T("VEHCLASSES"));
	MSXML2::IXMLDOMElementPtr pNodesNode		= pXMLDocument->createElement(_T("NODES"));
	MSXML2::IXMLDOMElementPtr pZonesNode		= pXMLDocument->createElement(_T("ZONES"));
	MSXML2::IXMLDOMElementPtr pLinkTypesNode	= pXMLDocument->createElement(_T("LINKTYPES"));
	MSXML2::IXMLDOMElementPtr pLinksNode		= pXMLDocument->createElement(_T("LINKS"));
	MSXML2::IXMLDOMElementPtr pPTStopsNode		= pXMLDocument->createElement(_T("PTSTOPS"));
	MSXML2::IXMLDOMElementPtr pPTLinesNode		= pXMLDocument->createElement(_T("PTLINES"));
	MSXML2::IXMLDOMElementPtr pSignalControlsNode= pXMLDocument->createElement(_T("SIGNALCONTROLS"));

	// Append node pointers just under Network
	pNetworkNode->appendChild(pVehTypesNode);
	pNetworkNode->appendChild(pVehClassesNode);
	pNetworkNode->appendChild(pNodesNode);
	pNetworkNode->appendChild(pZonesNode);
	pNetworkNode->appendChild(pLinkTypesNode);
	pNetworkNode->appendChild(pLinksNode);
	pNetworkNode->appendChild(pPTStopsNode);
	pNetworkNode->appendChild(pPTLinesNode);
	pNetworkNode->appendChild(pSignalControlsNode);

	// temporary pointers to append deeper levels
	MSXML2::IXMLDOMElementPtr pParentNode = NULL,pSubNode = NULL;

	pNode = pVehTypesNode;

	iCount = m_szVehTypeNo.GetSize();
	for(i=0;i<iCount;i++)
	{
		pSubNode = pXMLDocument->createElement(_T("VEHTYPE"));
		pNode->appendChild(pSubNode);
		pSubNode->setAttribute(_T("NO"),(_bstr_t)m_szVehTypeNo.GetAt(i));
		pSubNode->setAttribute(_T("NAME"),(_bstr_t)m_szVehTypeName.GetAt(i));
		pSubNode->setAttribute(_T("VEHCATEGORY"),(_bstr_t)m_szVehCategory.GetAt(i));
	}
	

	pParentNode = pVehClassesNode;

	iCount = m_VehClassArray.GetSize();
	for(i=0;i<iCount;i++)
	{
		CMVehClass* pVehClass = m_VehClassArray.GetAt(i);
		pSubNode=pXMLDocument->createElement(_T("VEHCLASS"));
		pParentNode->appendChild(pSubNode);
		pSubNode->setAttribute(_T("ID"),(_bstr_t)pVehClass->m_szVehClassId);

		pNode = pSubNode;

		jCount = pVehClass->m_szVehTypeIdArray.GetSize();
		for(j=0;j<jCount;j++)
		{
			pSubNode = pXMLDocument->createElement(_T("VEHTYPEID"));
			pNode->appendChild(pSubNode);
			szTemp = pVehClass->m_szVehTypeIdArray.GetAt(j);
			pSubNode->setAttribute(_T("NO"),(_bstr_t)szTemp);
		}
	}

	std::list<MNode*>::iterator iMNode;
	CString coord;
	for(iMNode = m_NodeList.begin(); iMNode != m_NodeList.end(); iMNode++)
	{
		MNode *pMNode = (*iMNode);
		pSubNode=pXMLDocument->createElement(_T("NODE"));
		pNodesNode->appendChild(pSubNode); // NODE 从属于 NODES

		pSubNode->setAttribute(_T("NO"),pMNode->m_NodeNumber);
		pSubNode->setAttribute(_T("NAME"),pMNode->m_Name.c_str());
		if (pMNode->pt.x == 0) coord = _T("0.0000");
		else coord.Format("%.4f",pMNode->pt.x );
		pSubNode->setAttribute(_T("XCOORD"),(_bstr_t)coord);
		if (pMNode->pt.y == 0) coord = _T("0.0000");
		else coord.Format("%.4f",pMNode->pt.y );
		pSubNode->setAttribute(_T("YCOORD"),(_bstr_t)coord);
		switch (pMNode->nControlType)
		{
		case 1:
			szTemp = _T("Signalized");
			break;
		case 2:
			szTemp = _T("TwoWayStop");
			break;
		default:
			szTemp = _T("Unknown");
			break;
		}
		pSubNode->setAttribute(_T("CONTROLTYPE"),(_bstr_t)szTemp);

		pParentNode = pSubNode;// parent保存的是本NODE的指针，除LANES外，还要加LANETURNS

		pSubNode = pXMLDocument->createElement(_T("LANES"));
		pParentNode->appendChild(pSubNode);

		pNode = pSubNode;
		std::vector<MLink*>::iterator iMLink;
		std::vector<MLane*>::iterator iMLane;
		float default_width = 3.75;

		// in lanes in in links
		for(iMLink=pMNode->inLinks.begin();iMLink!=pMNode->inLinks.end();iMLink++)
		{
			MLink* pMLink = (*iMLink);
			for(iMLane=pMLink->inLanes.begin();iMLane!=pMLink->inLanes.end();iMLane++)
			{
				MLane* pLane = (*iMLane);
				pSubNode=pXMLDocument->createElement(_T("LANE"));
				pNode->appendChild(pSubNode);
				pSubNode->setAttribute(_T("LINKID"),(_bstr_t)pLane->m_LinkID);
				pSubNode->setAttribute(_T("INDEX"),(_bstr_t)pLane->m_Index);
				if (pLane->m_PocketLength > 0.0)
				{
					pSubNode->setAttribute(_T("POCKET"),_T("true"));
					sprintf_s(cTemp,"%.4f",pLane->m_PocketLength);
				}
				else
				{
					pSubNode->setAttribute(_T("POCKET"),_T("false"));
					sprintf_s(cTemp,"%s","0.0000");
				}
				pSubNode->setAttribute(_T("POCKETLENGTH"),(_bstr_t)cTemp);
				sprintf_s(cTemp,"%.4f",default_width);
				pSubNode->setAttribute(_T("WIDTH"),(_bstr_t)cTemp);
			}
		}
		// out lanes in out links
		for(iMLink=pMNode->outLinks.begin();iMLink!=pMNode->outLinks.end();iMLink++)
		{
			MLink* pMLink = (*iMLink);
			for(iMLane=pMLink->outLanes.begin();iMLane!=pMLink->outLanes.end();iMLane++)
			{
				MLane* pLane = (*iMLane);
				pSubNode=pXMLDocument->createElement(_T("LANE"));
				pNode->appendChild(pSubNode);
				pSubNode->setAttribute(_T("LINKID"),(_bstr_t)pLane->m_LinkID);
				pSubNode->setAttribute(_T("INDEX"),(_bstr_t)pLane->m_Index);
				//if (pLane->m_PocketLength > 0.0)  这里将来应改为channel
				//{
				//	pSubNode->setAttribute(_T("POCKET"),_T("true"));
				//	sprintf_s(cTemp,"%lf",pLane->m_PocketLength);
				//}
				//else
				//{
				//	pSubNode->setAttribute(_T("POCKET"),_T("false"));
				//	sprintf_s(cTemp,"%s","0.0");
				//}
				pSubNode->setAttribute(_T("POCKET"),_T("false"));
				sprintf_s(cTemp,"%s","0.0000");
				pSubNode->setAttribute(_T("POCKETLENGTH"),(_bstr_t)cTemp);
				sprintf_s(cTemp,"%.4f",default_width);
				pSubNode->setAttribute(_T("WIDTH"),(_bstr_t)cTemp);
			}
		}

		pSubNode = pXMLDocument->createElement(_T("LANETURNS"));
		pParentNode->appendChild(pSubNode);

		pNode = pSubNode;
		std::vector<MLaneTurn*>::iterator iTurn;
		for(iTurn = pMNode->LaneTurns.begin();iTurn!=pMNode->LaneTurns.end();iTurn++)
		{
			MLaneTurn* pTurn = (*iTurn);
			pSubNode=pXMLDocument->createElement(_T("LANETURN"));
			pNode->appendChild(pSubNode);
			pSubNode->setAttribute(_T("FROMLINKID"),pTurn->nFromLinkId);
			pSubNode->setAttribute(_T("FROMLANEINDEX"),pTurn->nFromIndex);
			pSubNode->setAttribute(_T("TOLINKID"),pTurn->nToLinkId);
			pSubNode->setAttribute(_T("TOLANEINDEX"),pTurn->nToIndex);
			if (pTurn->nSCNO != 0) pSubNode->setAttribute(_T("SCNO"),pTurn->nSCNO);
			if (pTurn->nSignalGroupNo != 0) pSubNode->setAttribute(_T("SGNO"),pTurn->nSignalGroupNo);
		}

		pSubNode = pXMLDocument->createElement(_T("CROSSWALKS"));
		pParentNode->appendChild(pSubNode);
	}



	// ZONES   pZonesNode
	std::vector<MZone*>::iterator iZone;
	for(iZone=m_ZoneVector.begin();iZone!=m_ZoneVector.end();iZone++)
	{
		MZone* pZone = (*iZone);
		pSubNode=pXMLDocument->createElement(_T("ZONE"));
		pZonesNode->appendChild(pSubNode);
		pSubNode->setAttribute(_T("NO"),pZone->m_nID);
		CString coord;
		coord.Format("%.4f",pZone->pt.x );
		pSubNode->setAttribute(_T("XCOORD"),(_bstr_t)coord);
		coord.Format("%.4f",pZone->pt.y );
		pSubNode->setAttribute(_T("YCOORD"),(_bstr_t)coord);
		
		pParentNode = pSubNode;
		pSubNode=pXMLDocument->createElement(_T("CONNECTORS"));
		pParentNode->appendChild(pSubNode);

		pNode = pSubNode;
		std::vector<MNode*>::iterator iMNode;
		for(iMNode=pZone->m_nodes.begin();iMNode!=pZone->m_nodes.end();iMNode++)
		{
			pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
			pNode->appendChild(pSubNode);
			pSubNode->setAttribute(_T("NODENO"),(*iMNode)->m_NodeNumber);
			pSubNode->setAttribute(_T("DIRECTION"),(_bstr_t)"true");

			pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
			pNode->appendChild(pSubNode);
			pSubNode->setAttribute(_T("NODENO"),(*iMNode)->m_NodeNumber);
			pSubNode->setAttribute(_T("DIRECTION"),(_bstr_t)"false");
		}
	}
	
	// LINKTYPES   pLinkTypesNode
	pNode = pLinkTypesNode;

	iCount = m_szLinkTypeNo.GetSize();
	for(i=0;i<iCount;i++)
	{
		pSubNode = pXMLDocument->createElement(_T("LINKTYPE"));
		pNode->appendChild(pSubNode);
		pSubNode->setAttribute(_T("NO"),(_bstr_t)m_szLinkTypeNo.GetAt(i));
		pSubNode->setAttribute(_T("NAME"),(_bstr_t)m_szLinkTypeName.GetAt(i));
		pSubNode->setAttribute(_T("DRIVINGBEHAVIOR"),(_bstr_t)m_szDrivingBehavior.GetAt(i));
	}

	// LINKS pLinksNode
	std::list<MLink*>::iterator iMLink;
	for(iMLink = m_LinkList.begin();iMLink!=m_LinkList.end();iMLink++)
	{
		MLink* pLink=(*iMLink);	
		pSubNode=pXMLDocument->createElement(_T("LINK"));
		pLinksNode->appendChild(pSubNode);
		pSubNode->setAttribute(_T("ID"),		pLink->m_LinkID);
		pSubNode->setAttribute(_T("FROMNODENO"),pLink->m_FromNodeNumber);
		pSubNode->setAttribute(_T("TONODENO"),	pLink->m_ToNodeNumber);
		pSubNode->setAttribute(_T("LINKTYPENO"),pLink->m_LinkType);
		CString sl;
		sl.Format("%.4f",pLink->m_SpeedLimit);
		pSubNode->setAttribute(_T("SPEED"),		(_bstr_t)sl);
		pSubNode->setAttribute(_T("NUMLANES"),	pLink->m_NumLanes);
		if (pLink->m_ReverseLinkID != 0)
			pSubNode->setAttribute(_T("REVERSELINK"),pLink->m_ReverseLinkID);

		jCount = pLink->m_ShapePoints.size();
		if ( 2 == jCount ) continue; 

		pParentNode = pSubNode;
		pSubNode = pXMLDocument->createElement(_T("LINKPOLY"));
		pParentNode->appendChild(pSubNode);
		pNode = pSubNode;
		for(int n=1;n<jCount-1;n++)
		{
			pSubNode = pXMLDocument->createElement(_T("POINT"));
			pNode->appendChild(pSubNode);
			GDPoint GDP = pLink->m_ShapePoints[n];
			pSubNode->setAttribute(_T("INDEX"),n);
			coord.Format("%.4f",GDP.x );
			pSubNode->setAttribute(_T("XCOORD"),(_bstr_t)coord);
			coord.Format("%.4f",GDP.y );
			pSubNode->setAttribute(_T("YCOORD"),(_bstr_t)coord);
		}
	}
	
	// PTSTOPS
	// PTLINES

	// SIGNALCONTROLS  pSignalControlsNode
	iCount = m_SignalControlArray.GetSize();
	CString s1;
	for(i=0;i<iCount;i++)
	{
		CMSignalControl* pSC = m_SignalControlArray.GetAt(i);
		pSubNode=pXMLDocument->createElement(_T("SIGNALCONTROL"));
		pSignalControlsNode->appendChild(pSubNode);
		pSubNode->setAttribute(_T("NO"),		(_bstr_t)pSC->szNo);
		pSubNode->setAttribute(_T("NAME"),		(_bstr_t)pSC->szName);
		s1 = Minutes2PTString(pSC->nCycleTime);
		pSubNode->setAttribute(_T("CYCLETIME"),	(_bstr_t)s1);
		s1 = Minutes2PTString(pSC->nTimeOffset);
		pSubNode->setAttribute(_T("TIMEOFFSET"),(_bstr_t)s1);
		switch (pSC->nSignalizationType)
		{
		case 1:
			szTemp = _T("FixedTime");
			break;
		default:
			szTemp = _T("Unknown");
			break;
		}
		pSubNode->setAttribute(_T("SIGNALIZATIONTYPE"),(_bstr_t)szTemp);
		pSubNode->setAttribute(_T("CYCLETIMEFIXED"),(_bstr_t)(pSC->bCycleTimeFixed?"true":"false"));
		
		pParentNode = pSubNode;
		pSubNode=pXMLDocument->createElement(_T("SIGNALGROUPS"));
		pParentNode->appendChild(pSubNode);

		pNode = pSubNode;
		jCount = pSC->signalgroups.GetSize();
		for(j=0;j<jCount;j++)
		{
			CMSignalGroup* pSG = pSC->signalgroups.GetAt(j);
			pSubNode=pXMLDocument->createElement(_T("SIGNALGROUP"));
			pNode->appendChild(pSubNode);
			pSubNode->setAttribute(_T("NO"),		(_bstr_t)pSG->nNo );
			pSubNode->setAttribute(_T("NAME"),		(_bstr_t)pSG->szName);
			pSubNode->setAttribute(_T("GTSTART"),	(_bstr_t)Minutes2PTString(pSG->nGTStart));
			pSubNode->setAttribute(_T("GTEND"),		(_bstr_t)Minutes2PTString(pSG->nGTEnd));
			pSubNode->setAttribute(_T("MINGTIME"),	(_bstr_t)Minutes2PTString(pSG->nMingTime));
			pSubNode->setAttribute(_T("ATIME"),		(_bstr_t)Minutes2PTString(pSG->nATime));
			pSubNode->setAttribute(_T("SIGNALGROUPTYPE"),(_bstr_t)pSG->szSignalControlType);
		}
	}
	
	pXMLDocument->save(strFileName.c_str()); 
	//pXMLDocument->Release();  Cannot exit gracefully. Some problems still exist.
	//::CoUninitialize();
	return true;
}

void Mustang::CreateDefaultData()
{
	int i;

	m_szANMFileName = _T("testData.anm");
	m_szVersNo = _T("1.0");
	m_szFromTime = _T("00:00:00");
	m_szToTime = _T("10:00:00");
	m_szName = _T("Mustang.ver");
	m_nLeftHandTraffic=0;

	m_szVehTypes = _T("");      // This string is reserved for attributes of VEHTYPES. The following 3 are in groups
	CString s1,s2,s3;
	s1.Format(TEXT("%d"),1);
	s2.Format(TEXT("%s"),"PtDefault");
	s3.Format(TEXT("%s"),"Bus");
	m_szVehTypeNo.Add(s1);
	m_szVehTypeName.Add(s2);
	m_szVehCategory.Add(s3);

	s1.Format(TEXT("%d"),100);
	s2.Format(TEXT("%s"),"AllPurp");
	s3.Format(TEXT("%s"),"Car");
	m_szVehTypeNo.Add(s1);
	m_szVehTypeName.Add(s2);
	m_szVehCategory.Add(s3);

	s1.Format(TEXT("%d"),200);
	s2.Format(TEXT("%s"),"XX");
	s3.Format(TEXT("%s"),"Car");
	m_szVehTypeNo.Add(s1);
	m_szVehTypeName.Add(s2);
	m_szVehCategory.Add(s3);

	CMVehClass *pVehClass = new CMVehClass();
	pVehClass->m_szVehClassId = _T("PtDefault");
	s1 = _T("1");
	pVehClass->m_szVehTypeIdArray.Add(s1);
	m_VehClassArray.Add(pVehClass);

	pVehClass = new CMVehClass();
	pVehClass->m_szVehClassId = _T("C");
	s1 = _T("100");
	pVehClass->m_szVehTypeIdArray.Add(s1);
	s1 = _T("200");
	pVehClass->m_szVehTypeIdArray.Add(s1);
	m_VehClassArray.Add(pVehClass);
	
	//m_szLinkTypes = _T("");     // This string is reserved for attributes of LINKTYPES. The following 3 are in groups
	//m_szLinkTypeNo.Add(_T("1"));
	//m_szLinkTypeName.Add(_T("Major State Hwy"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("3"));
	//m_szLinkTypeName.Add(_T("Minor State Hwy"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("5"));
	//m_szLinkTypeName.Add(_T("Principal Arterial"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("7"));
	//m_szLinkTypeName.Add(_T("Minor Arterial"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("61"));
	//m_szLinkTypeName.Add(_T("Collector Arterial"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("71"));
	//m_szLinkTypeName.Add(_T("Neighborh. Collector"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("81"));
	//m_szLinkTypeName.Add(_T("Neighborhood"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("85"));
	//m_szLinkTypeName.Add(_T("Major Parking Access"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("91"));
	//m_szLinkTypeName.Add(_T("Artificial Connector"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("92"));
	//m_szLinkTypeName.Add(_T("Connector"));
	//m_szDrivingBehavior.Add(_T("Urban"));
	//m_szLinkTypeNo.Add(_T("100"));
	//m_szLinkTypeName.Add(_T("Common Connector"));
	//m_szDrivingBehavior.Add(_T("Urban"));

	m_szPTStops = _T("");

	m_szPTLines = _T("");

	CMSignalControl* pSC = new CMSignalControl();
	pSC->szNo = _T("9133");
	pSC->szName = _T("Signal9133");
	pSC->nCycleTime = 120;
	pSC->nTimeOffset = 0;
	pSC->nSignalizationType = 1;
	pSC->bCycleTimeFixed = true;
	m_SignalControlArray.Add(pSC);
	for(i=1;i<=4;i++)
	{
		CMSignalGroup *pSG = new CMSignalGroup();
		pSC->signalgroups.Add(pSG);
		pSG->nNo = i;
		pSG->szName.Format(TEXT("SG%2d"),i); 
		pSG->nGTStart = (i-1)*30;
		pSG->nGTEnd = i*30;
		pSG->nMingTime = 0;
		pSG->nATime = 0;
		pSG->szSignalControlType = _T("Cycle");
	}
}
bool Mustang::CreateANMRoutesFile(std::string strFileName)
{
	return true;
}


/*
// Follow is from Xuesong
void CTLiteDoc::ConstructandexportVISSIMdata()
{
	CString str;
	CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"VISSIM Data File (*.csv)|*.csv||", NULL);
	if(dlg.DoModal() == IDOK)
	{

		CWaitCursor wait;
		char fname[_MAX_PATH];
		wsprintf(fname,"%s", dlg.GetPathName());

		CString SynchroProjectFile = dlg.GetPathName();
		m_Synchro_ProjectDirectory  = SynchroProjectFile.Left(SynchroProjectFile.ReverseFind('\\') + 1);

		m_Network.Initialize (m_NodeSet.size(), m_LinkSet.size(), 1, m_AdjLinkSize);
		m_Network.BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, true, false);

		}


	// generate all movements
	int i = 0;
	for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++, i++)
	{  // for current node
		
		if ((*iNode)->m_ControlType > 1)  //(m_Network.m_InboundSizeAry[i] >= 3) // add node control types
		{
	

			// scan each inbound link and outbound link

			for(int inbound_i= 0; inbound_i< m_Network.m_InboundSizeAry[i]; inbound_i++)
			{
				// for each incoming link
				for(int outbound_i= 0; outbound_i< m_Network.m_OutboundSizeAry [i]; outbound_i++)
				{
					//for each outging link
					int LinkID = m_Network.m_InboundLinkAry[i][inbound_i];

					if (m_Network.m_FromIDAry[LinkID] != m_Network.m_OutboundNodeAry [i][outbound_i])
					{
						// do not consider u-turn

						DTA_Movement element;

						element.CurrentNodeID = i;						

						element.InboundLinkID = LinkID;
						element.UpNodeID = m_Network.m_FromIDAry[LinkID];
						element.DestNodeID = m_Network.m_OutboundNodeAry [i][outbound_i];

						GDPoint p1, p2, p3;
						p1  = m_NodeIDMap[element.UpNodeID]->pt;
						p2  = m_NodeIDMap[element.CurrentNodeID]->pt;
						p3  = m_NodeIDMap[element.DestNodeID]->pt;

						element.movement_approach = g_Angle_to_Approach_New(Find_P2P_Angle(p1,p2));
						element.movement_turn = Find_PPP_to_Turn(p1,p2,p3);

						// determine  movement type /direction here
						element.dir = DTA_LANES_COLUME_init;
						switch (element.movement_approach)
						{
							case DTA_North:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.dir = DTA_NBL; break;
									case DTA_Through: element.dir = DTA_NBT; break;
									case DTA_RightTurn: element.dir = DTA_NBR; break;
								}
								break;
							case DTA_East:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.dir = DTA_EBL; break;
									case DTA_Through: element.dir = DTA_EBT; break;
									case DTA_RightTurn: element.dir = DTA_EBR; break;
								}
								break;
							case DTA_South:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.dir = DTA_SBL; break;
									case DTA_Through: element.dir = DTA_SBT; break;
									case DTA_RightTurn: element.dir = DTA_SBR; break;
								}
								break;
							case DTA_West:
								switch (element.movement_turn)
								{
									case DTA_LeftTurn: element.dir = DTA_WBL; break;
									case DTA_Through: element.dir = DTA_WBT; break;
									case DTA_RightTurn: element.dir = DTA_WBR; break;
								}
								break;
						}

	
					}  // for each feasible movement (without U-turn)
					
				} // for each outbound link

			} // for each inbound link

		} // checking control type
	}// for each node
}
*/

void Mustang::WriteTest(CString szFileName)
{
    MSXML2::IXMLDOMDocumentPtr pXMLDocument;  
    HRESULT hr = pXMLDocument.CreateInstance(__uuidof(MSXML2::DOMDocument));
	
	//MSXML2::IXMLDocument2Ptr pDoc;
    //HRESULT hr = pDoc.CreateInstance("DOMDocument4.0");
	
	if (FAILED(hr))
		return ;
	//MSXML2::IXMLDOMProcessingInstructionPtr pPtr;

	//pPtr =  pXMLDocument->createProcessingInstruction("xml", "version=\"1.0\"encoding=\"UTF-8\"");
	//pXMLDocument->appendChild(pPtr);
	
	MSXML2::IXMLDOMElementPtr pNode = pXMLDocument->createElement(_T("ABSTRACTNETWORKMODEL"));
	pNode->setAttribute(_T("VERSNO"),"1.0");
	pNode->setAttribute(_T("FROMTIME"),"09:00:00");
	pNode->setAttribute(_T("TOTIME"),"12:00:00");
	pNode->setAttribute(_T("NAME"),"Reno_UNR.ver");
	pXMLDocument->appendChild(pNode);
	MSXML2::IXMLDOMElementPtr pAbstractNetworkNode = pNode;

    MSXML2::IXMLDOMElementPtr pNetworkNode = pXMLDocument->createElement(_T("NETWORK"));
	pNetworkNode->setAttribute(_T("LEFTHANDTRAFFIC"),0);
	pAbstractNetworkNode->appendChild(pNetworkNode);

	MSXML2::IXMLDOMElementPtr pVehTypesNode		= pXMLDocument->createElement(_T("VEHTYPES"));
	MSXML2::IXMLDOMElementPtr pVehClassesNode	= pXMLDocument->createElement(_T("VEHCLASSES"));
	MSXML2::IXMLDOMElementPtr pNodesNode		= pXMLDocument->createElement(_T("NODES"));
	MSXML2::IXMLDOMElementPtr pZonesNode		= pXMLDocument->createElement(_T("ZONES"));
	MSXML2::IXMLDOMElementPtr pLinkTypesNode	= pXMLDocument->createElement(_T("LINKTYPES"));
	MSXML2::IXMLDOMElementPtr pLinksNode		= pXMLDocument->createElement(_T("LINKS"));
	MSXML2::IXMLDOMElementPtr pPTStopsNode		= pXMLDocument->createElement(_T("PTSTOPS"));
	MSXML2::IXMLDOMElementPtr pPTLinesNode		= pXMLDocument->createElement(_T("PTLINES"));
	MSXML2::IXMLDOMElementPtr pSignalControlsNode= pXMLDocument->createElement(_T("SIGNALCONTROLS"));

	pNetworkNode->appendChild(pVehTypesNode);
	pNetworkNode->appendChild(pVehClassesNode);
	pNetworkNode->appendChild(pNodesNode);
	pNetworkNode->appendChild(pZonesNode);
	pNetworkNode->appendChild(pLinkTypesNode);
	pNetworkNode->appendChild(pLinksNode);
	pNetworkNode->appendChild(pPTStopsNode);
	pNetworkNode->appendChild(pPTLinesNode);
	pNetworkNode->appendChild(pSignalControlsNode);

	MSXML2::IXMLDOMElementPtr pParentNode = NULL,pSubNode = NULL;
	pNode = pVehTypesNode;
	
	MSXML2::IXMLElementPtr ptest = pVehTypesNode->parentNode;

	pSubNode = pXMLDocument->createElement(_T("VEHTYPE"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),1);
	pSubNode->setAttribute(_T("NAME"),"name1");
	pSubNode->setAttribute(_T("VEHCATEGORY"),"bus");

	pSubNode = pXMLDocument->createElement(_T("VEHTYPE"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),2);
	pSubNode->setAttribute(_T("NAME"),"name2");
	pSubNode->setAttribute(_T("VEHCATEGORY"),"car");

	pSubNode = pXMLDocument->createElement(_T("VEHTYPE"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),3);
	pSubNode->setAttribute(_T("NAME"),"name3");
	pSubNode->setAttribute(_T("VEHCATEGORY"),"truck");

	pParentNode = pVehClassesNode;

	pSubNode=pXMLDocument->createElement(_T("VEHCLASS"));
	pParentNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("ID"),"PtDefault");

	pNode = pSubNode;
	pSubNode = pXMLDocument->createElement(_T("VEHTYPEID"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("ID"),1);

	pSubNode=pXMLDocument->createElement(_T("VEHCLASS"));
	pParentNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("ID"),"C");

	pNode = pSubNode;
	pSubNode = pXMLDocument->createElement(_T("VEHTYPEID"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("ID"),100);

	pSubNode = pXMLDocument->createElement(_T("VEHTYPEID"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("ID"),200);

	pSubNode=pXMLDocument->createElement(_T("NODE"));
	pNodesNode->appendChild(pSubNode); // NODE 从属于 NODES

	pSubNode->setAttribute(_T("NO"),"4");
	pSubNode->setAttribute(_T("NAME"),"196th St; 44th AVE W");
	pSubNode->setAttribute(_T("XCOORD"),"389939.6592");
	pSubNode->setAttribute(_T("YCOORD"),"92118.5890");
	pSubNode->setAttribute(_T("CONTROLTYPE"),"Signalized");

	pParentNode = pSubNode;// parent保存的是本NODE的指针，除LANES外，还要加LANETURNS

	pSubNode = pXMLDocument->createElement(_T("LANES"));
	pParentNode->appendChild(pSubNode);

	pNode = pSubNode;
	
	// All the LANE s
	pSubNode=pXMLDocument->createElement(_T("LANE"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("LINKID"),"115A");
	pSubNode->setAttribute(_T("INDEX"),"1");
	pSubNode->setAttribute(_T("POCKET"),"false");
	pSubNode->setAttribute(_T("POCKETLENGTH"),"0.0");
	pSubNode->setAttribute(_T("WIDTH"),"3.6576");

	pSubNode=pXMLDocument->createElement(_T("LANE"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("LINKID"),"115A");
	pSubNode->setAttribute(_T("INDEX"),"2");
	pSubNode->setAttribute(_T("POCKET"),"false");
	pSubNode->setAttribute(_T("POCKETLENGTH"),"0.0");
	pSubNode->setAttribute(_T("WIDTH"),"3.6576");

	pSubNode=pXMLDocument->createElement(_T("LANE"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("LINKID"),"115A");
	pSubNode->setAttribute(_T("INDEX"),"3");
	pSubNode->setAttribute(_T("POCKET"),"true");
	pSubNode->setAttribute(_T("POCKETLENGTH"),"76.20");
	pSubNode->setAttribute(_T("WIDTH"),"3.6576");

	pSubNode = pXMLDocument->createElement(_T("LANETURNS"));
	pParentNode->appendChild(pSubNode);

	pNode = pSubNode;
	
	// All the LANETURN s
	pSubNode=pXMLDocument->createElement(_T("LANETURN"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("FROMLINKID"),"115A");
	pSubNode->setAttribute(_T("FROMLANEINDEX"),"1");
	pSubNode->setAttribute(_T("TOLINKID"),"115A");
	pSubNode->setAttribute(_T("TOLANEINDEX"),"2");
	pSubNode->setAttribute(_T("SCNO"),"6576");
	pSubNode->setAttribute(_T("SGNO"),"4");

	pSubNode=pXMLDocument->createElement(_T("LANETURN"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("FROMLINKID"),"115A");
	pSubNode->setAttribute(_T("FROMLANEINDEX"),"2");
	pSubNode->setAttribute(_T("TOLINKID"),"115B");
	pSubNode->setAttribute(_T("TOLANEINDEX"),"1");
	pSubNode->setAttribute(_T("SCNO"),"6576");
	pSubNode->setAttribute(_T("SGNO"),"2");

	pSubNode = pXMLDocument->createElement(_T("CROSSWALKS"));
	pParentNode->appendChild(pSubNode);


	// ZONES   pZonesNode
	pSubNode=pXMLDocument->createElement(_T("ZONE"));
	pZonesNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),"115");
	pSubNode->setAttribute(_T("XCOORD"),"388525.4393");
	pSubNode->setAttribute(_T("YCOORD"),"92524.4408");

	pParentNode = pSubNode;
	pSubNode=pXMLDocument->createElement(_T("CONNECTORS"));
	pParentNode->appendChild(pSubNode);

	pNode = pSubNode;
	pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NODENO"),"9593");
	pSubNode->setAttribute(_T("DIRECTION"),"false");

	pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NODENO"),"9593");
	pSubNode->setAttribute(_T("DIRECTION"),"true");

	pSubNode=pXMLDocument->createElement(_T("ZONE"));
	pZonesNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),"53");
	pSubNode->setAttribute(_T("XCOORD"),"388751.9669");
	pSubNode->setAttribute(_T("YCOORD"),"92357.9981");

	pParentNode = pSubNode;
	pSubNode=pXMLDocument->createElement(_T("CONNECTORS"));
	pParentNode->appendChild(pSubNode);

	pNode = pSubNode;
	pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NODENO"),"9055");
	pSubNode->setAttribute(_T("DIRECTION"),"false");

	pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NODENO"),"9095");
	pSubNode->setAttribute(_T("DIRECTION"),"false");

	pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NODENO"),"9809");
	pSubNode->setAttribute(_T("DIRECTION"),"false");
	
	pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NODENO"),"9055");
	pSubNode->setAttribute(_T("DIRECTION"),"true");

	pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NODENO"),"9095");
	pSubNode->setAttribute(_T("DIRECTION"),"true");

	pSubNode=pXMLDocument->createElement(_T("CONNECTOR"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NODENO"),"9809");
	pSubNode->setAttribute(_T("DIRECTION"),"true");
	
	// LINKTYPES   pLinkTypesNode
	pNode = pLinkTypesNode;

	pSubNode=pXMLDocument->createElement(_T("LINKTYPE"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),"31");
	pSubNode->setAttribute(_T("NAME"),"Major State Hwy");
	pSubNode->setAttribute(_T("DRIVINGBEHAVIOR"),"Urban");
	
	pSubNode=pXMLDocument->createElement(_T("LINKTYPE"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),"41");
	pSubNode->setAttribute(_T("NAME"),"Principal Arterial");
	pSubNode->setAttribute(_T("DRIVINGBEHAVIOR"),"Urban");
	
	
	// LINKS pLinksNode
	
	pSubNode=pXMLDocument->createElement(_T("LINK"));
	pLinksNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("ID"),"98A");
	pSubNode->setAttribute(_T("FROMNODENO"),"9");
	pSubNode->setAttribute(_T("TONODENO"),"9055");
	pSubNode->setAttribute(_T("LINKTYPENO"),"41");
	pSubNode->setAttribute(_T("SPEED"),"24.14");
	pSubNode->setAttribute(_T("NUMLANES"),"1");
	pSubNode->setAttribute(_T("REVERSELINK"),"98B");
	
	pSubNode=pXMLDocument->createElement(_T("LINK"));
	pLinksNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("ID"),"98B");
	pSubNode->setAttribute(_T("FROMNODENO"),"9055");
	pSubNode->setAttribute(_T("TONODENO"),"9");
	pSubNode->setAttribute(_T("LINKTYPENO"),"41");
	pSubNode->setAttribute(_T("SPEED"),"24.14");
	pSubNode->setAttribute(_T("NUMLANES"),"1");
	pSubNode->setAttribute(_T("REVERSELINK"),"98A");

	pParentNode = pSubNode;
	pSubNode = pXMLDocument->createElement(_T("LINKPOLY"));
	pParentNode->appendChild(pSubNode);

	pNode = pSubNode;
	pSubNode = pXMLDocument->createElement(_T("POINT"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("INDEX"),"1");
	pSubNode->setAttribute(_T("XCOORD"),"388751.9669");
	pSubNode->setAttribute(_T("YCOORD"),"92357.9981");
	
	// PTSTOPS
	// PTLINES

	// SIGNALCONTROLS  pSignalControlsNode
	pSubNode=pXMLDocument->createElement(_T("SIGNALCONTROL"));
	pSignalControlsNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),"9131");
	pSubNode->setAttribute(_T("NAME"),"16");
	pSubNode->setAttribute(_T("CYCLETIME"),"PT2M");
	pSubNode->setAttribute(_T("TIMEOFFSET"),"PT0S");
	pSubNode->setAttribute(_T("SIGNALIZATIONTYPE"),"FixedTime");
	pSubNode->setAttribute(_T("CYCLETIMEFIXED"),"true");

	pParentNode = pSubNode;
	pSubNode=pXMLDocument->createElement(_T("SIGNALGROUPS"));
	pParentNode->appendChild(pSubNode);

	pNode = pSubNode;

	pSubNode=pXMLDocument->createElement(_T("SIGNALGROUP"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),"4");
	pSubNode->setAttribute(_T("NAME"),"");
	pSubNode->setAttribute(_T("GTSTART"),"PT1M40S");
	pSubNode->setAttribute(_T("GTEND"),"PT2M");
	pSubNode->setAttribute(_T("MINGTIME"),"PT0S");
	pSubNode->setAttribute(_T("ATIME"),"PT0S");
	pSubNode->setAttribute(_T("SIGNALGROUPTYPE"),"Cycle");

	pSubNode=pXMLDocument->createElement(_T("SIGNALGROUP"));
	pNode->appendChild(pSubNode);
	pSubNode->setAttribute(_T("NO"),"3");
	pSubNode->setAttribute(_T("NAME"),"");
	pSubNode->setAttribute(_T("GTSTART"),"PT1M40S");
	pSubNode->setAttribute(_T("GTEND"),"PT2M");
	pSubNode->setAttribute(_T("MINGTIME"),"PT0S");
	pSubNode->setAttribute(_T("ATIME"),"PT0S");
	pSubNode->setAttribute(_T("SIGNALGROUPTYPE"),"Cycle");
	
	pXMLDocument->save(_bstr_t(szFileName));  
}

int MNode::CheckMissingApproach(void)
{
	if (inLinks.size() != 3 || outLinks.size() != 3) return 0;

	int sumIn=0,mulIn=1;
	std::vector<MLink*>::iterator iLink;
	for(iLink = inLinks.begin(); iLink != inLinks.end(); iLink++)
	{
		sumIn += (*iLink)->m_ToNodeApproach;
		mulIn *= (*iLink)->m_ToNodeApproach;
	}
	if (mulIn == 0) return 0;

	if ( (sumIn == 6 && mulIn == 6) ||
		 (sumIn == 9 && mulIn == 24)||
		 (sumIn == 8 && mulIn == 12)||
		 (sumIn == 7 && mulIn == 8) )
	{
	}
	else
		return 0;

	int sumOut=0,mulOut=1;
	for(iLink = outLinks.begin(); iLink != outLinks.end(); iLink++)
	{
		sumOut += (*iLink)->m_FromNodeApproach;
		mulOut *= (*iLink)->m_FromNodeApproach;
	}
	if ( sumIn != sumOut || mulIn != mulOut) return 0;

	return 10-sumIn;
}

