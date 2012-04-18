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

enum MLinkApproach {
	North=1,
	East,
	South,
	West
};
class MLinkType
{
public:
	MLinkType();
	~MLinkType();
public:
	std::string strNo;
	std::string strName;
	std::string strDrivingBehavior;
};
class CMVehClass : public CObject
{
public:
	CMVehClass();
	~CMVehClass();
	CString m_szVehClassId;
	CStringArray m_szVehTypeIdArray;
};
typedef CArray <CMVehClass*,CMVehClass*&> CMVehClassArray;

class CMSignalGroup : public CObject
{
public:
	CMSignalGroup();
	~CMSignalGroup();
	int		nNo;
	CString szName;
	int		nGTStart;
	int		nGTEnd;
	int		nMingTime;
	int		nATime;
	CString	szSignalControlType;
};
typedef CArray<CMSignalGroup*,CMSignalGroup*> CMSignalGroupArray;

class CMSignalControl : public CObject
{
public:
	CMSignalControl();
	~CMSignalControl();
	CString		szNo;
	CString		szName;
	int			nCycleTime;
	int			nTimeOffset;
	int			nSignalizationType; // 1: FixedTime
	bool		bCycleTimeFixed;
	CMSignalGroupArray	signalgroups;
};
typedef CArray<CMSignalControl*,CMSignalControl*&> CMSignalControlArray;

class MLane
{
public:
	MLane();
	~MLane();
public:
	int		m_LinkID;
	int		m_NodeID;   // only the ToNode
	int		m_SCNO;		// redundant from node
	int		m_Index;
	float	m_PocketLength;
	float	m_ChannelLength;
	bool	leftTurn;
	bool	through;
	bool	rightTurn;
	int		m_SGNO;		// signal group no
};
class MLaneTurn
{
public:
	MLaneTurn();
	~MLaneTurn();
public:
	int		nFromLinkId;
	int		nToLinkId;
	int		nFromIndex;
	int		nToIndex;
	int		nRTL;  //Right=1,Through=2,Left=3
	int		nSCNO;
	int		nSignalGroupNo;
	int		nNEMA; //NEMA sequence
};

class MLink
{
public:
	MLink();
	~MLink();
public:
	std::string m_Name;
	int			m_LinkNo;
	int			m_LinkID;
	int			m_FromNodeNumber;
	int			m_ToNodeNumber;
	int			m_FromNodeID;
	int			m_ToNodeID;
	int			m_NumLanes;
	int			m_LinkType;
	float		m_LaneWidth;
	float		m_SpeedLimit;
	int			m_FromNodeApproach;
	int			m_ToNodeApproach;
	int			m_ReverseLinkID;
	std::vector<GDPoint> m_ShapePoints;	// inter-mediam points
	std::vector<MLane*> inLanes;
	std::vector<MLane*> outLanes;
	int			GetLaneCount(int nInOut,int RTL=0); // 1:In, 0:Out; RTL 1,2,3,0=all lane count
};

class MNode
{
public:
	MNode();
	~MNode();
	std::string	m_Name;			// old
	GDPoint		pt;				// ground position
	GDPoint		ptLL;			// latitude and longitude
	int			m_NodeNumber;	// old, original node number
	int			m_NodeID;		// old, id, starting from zero, continuous sequence
	int			m_ZoneID;		// old, If ZoneID > 0 --> centriod,  otherwise a physical node.
	int			nControlType;	// default = 1, Signalized; 2, TwoWayStop; 0, Unknown
	int			nSCNO;			// Signal control no
	std::vector<MLink*>	inLinks;// links with this node as to_node
	std::vector<MLink*> outLinks;//links with this node as from_node
	std::map<int,MLink*> inLinkMap;
	std::map<int,MLink*> outLinkMap;
	std::vector<MLaneTurn*> LaneTurns;
	int			m_nProcessType;  // 4: 标准十字路口 3:标准T路口 0: 边界点、只进或只出 1:边界点、一进一出 2:联接点、二进二出or一进一出（单联接） 5:其他情况
	int			CheckMissingApproach(void); // 检查T路口是否为标准，0为非标准，否则返回缺失的方向1-4
};
class MZone
{
public:
	MZone();
	~MZone();
public:
	int		m_nID;
	GDPoint pt;
	std::vector<MNode*> m_nodes;
};
class MDemand
{
public:
	MDemand();
	~MDemand();
public:
	int	m_nFromZone;
	int m_nToZone;
	float m_fDemand[4];
	int m_nStartMin;
	int m_nEndMin;
};
class Mustang
{
public:
	Mustang();
	~Mustang();
public:
	float refLongi; // reference longitude, set by the first node
	float refLati;  // reference latitude, set by the first node
	float refScale; // reference scale, set during the first node, 111700.0 meters
public:
	float  L2X(float longitude,float latitude);
	float  L2Y(float latitude);

public:
	// log file
	std::ofstream m_logFile;
	// core data structure for network
	std::list<MNode*> m_NodeList;
	std::map<int, MNode*> m_NodeIDMap;
	std::map<int, int> m_NodeIDtoNameMap;
	std::map<int, int> m_NodeNametoIDMap;
	std::list<MLink*> m_LinkList;
	std::map<int, MLink*> m_LinkNotoLinkMap;

	// auxillary data structure
	CString m_szANMFileName;
	CString m_szVersNo;
	CString m_szFromTime;
	CString m_szToTime;
	CString m_szName;
	int m_nLeftHandTraffic;

	CString m_szVehTypes;      // This string is reserved for attributes of VEHTYPES. The following 3 are in groups
	CStringArray m_szVehTypeNo;
	CStringArray m_szVehTypeName;
	CStringArray m_szVehCategory;

	CMVehClassArray m_VehClassArray;
	std::vector<MZone*> m_ZoneVector;
	std::vector<MDemand*> m_DemandVector;

	CString m_szLinkTypes;     // This string is reserved for attributes of LINKTYPES. The following 3 are in groups
	CStringArray m_szLinkTypeNo;
	CStringArray m_szLinkTypeName;
	CStringArray m_szDrivingBehavior;

	CString m_szPTStops;

	CString m_szPTLines;

	CMSignalControlArray	m_SignalControlArray;

public:
	bool OpenLogFile(std::string strLogFileName);
	bool CloseLogFile();
	bool ReadInputNodeCSV(std::string strFileName);
	bool ReadInputLinkCSV(std::string strFileName);
	bool ReadInputLinkTypeCSV(std::string strFileName);
	bool CheckDuplicateLink(int link_id);
	void DumpNodeLink2Log(); // called after ReadInputLinkCSV to check the info, debug only
	bool ReadInputDemandCSV(std::string strFileName);
	int  GetMLinkApproach(std::string dir,int* nAppr);
	bool ReadInputZoneCSV(std::string strFileName);
	bool ReadInputLaneCSV(std::string strFileName);
	bool ReadInputSignalCSV(std::string strFileName);

	bool ClassifyNodes(void);//set node ProcessType
	bool CreateDefaultLanes(void); //create default lanes if no lane data
	bool ProcessLanes(void);
	int  GetNeighborApproach(int la,int RTL,int *appr);// R,T,L = 1,2,3
	int	 GetSCNO(int nNodeNumber);
	int	 GetSGNO(int nNodeNumber,int appr,int RTL);

	bool CreateANMFile(std::string strFileName);
	CString Minutes2PTString(int nMin);
	void CreateDefaultData();
	bool CreateANMRoutesFile(std::string strFileName);

	void WriteTest(CString szFileName);
};