// TSView.h : interface of the CTimeSpaceView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSVIEW_H__81CB4DEC_FFC3_480A_9B92_163C4C8226DA__INCLUDED_)
#define AFX_TSVIEW_H__81CB4DEC_FFC3_480A_9B92_163C4C8226DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _MAX_LANE_SIZE 7

class CTimeSpaceView : public CView
{
protected: // create from serialization only
	CTimeSpaceView();
	DECLARE_DYNCREATE(CTimeSpaceView)

// Attributes
public:

	int m_NumberOfVehicles_lane;

	int m_lane_size;
	// processing NGSim data
	bool m_bShowSimulationDataFlag;
	bool m_bShowVehicleIDFlag;
	bool m_bShowLeaderPositionFlag;

	bool m_bShowBothResultsFlag;

	bool m_bColorBySpeedFlag; 
	VehicleSnapshotData ** m_VehicleSnapshotAry;
	int m_NumberOfVehicles;
	int m_NumberOfTimeSteps;  // 0.1 second as resultion

	std::map<int, int> m_VehicleIDtoNOMap;
	std::vector<VehicleCFData> m_VehicleDataList;
	int m_SelectedVehicleID;

	std::vector<int> m_VehicleNoVector_Lane;
	int FindPreceedingVehicleNo(int LaneNo,float LocalY, int t);
	int FindFollowingVehicleNo(int LaneNo,float LocalY, int t);


	float *** m_VehicleSnapshotAry_Lane; // simulation

	int ** m_VehicleStartTimeAry_Lane; // simulation
	int ** m_VehicleEndTimeAry_Lane; // simulation

	int StartTimeLane[_MAX_LANE_SIZE];  // for the first vehicle
	int EndTimeLane[_MAX_LANE_SIZE];

	float StartLocalYLane[_MAX_LANE_SIZE];
	float EndLocalYLane[_MAX_LANE_SIZE];

	int VehicleSizeLane[_MAX_LANE_SIZE];

	int m_SelectLaneID;

	void UpdateVehicleStartANDEndTimes(std::vector<int> &StartPassingTimeStamp, std::vector<int> &EndPassingTimeStamp);

	float m_KJam; // in veh/mile
	float m_WaveSpeed; // in mph 
	float m_FreeflowSpeed; // in mph 


	// end of processing NGSim data

	bool bRangeInitialized;

	CTLiteDoc* GetTLDocument();

	bool m_bShowResourcePrice;
	bool m_bShowNodeCostLabel;

	bool m_bShowTimetable; 
	bool m_bShowSlackTime;

	int Cur_MOE_type1; // 0: mean travel time
	int Cur_MOE_type2; // 

	bool m_bMoveDisplay;
	CPoint m_last_cpoint;

   float m_UnitDistance, m_UnitTime;
   int m_Range;
   int m_TmLeft, m_TmRight;
   float m_YUpperBound;
   float m_YLowerBound;

   double m_TotalDistance;

// Operations
public:
	void InitializeTimeRange();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeSpaceView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	bool ExportDataToFile(char csv_file[_MAX_PATH], int DataFlag);
void RefreshWindow();
//	void DrawSchedule(int ScenarioNo);
//	void DrawTrain(int TrainNo);

	void DrawObjects(CDC* pDC,int MOEType,CRect PlotRect);
	void DrawNGSIMObjects(CDC* pDC,int MOEType,CRect PlotRect);

	int FindClosestTimeResolution(double Value)
{

		int ResolutionVector[6] = {100,300,600,1200,3000,15000};
		double min_distance  = 9999999;

		Value = max(1,Value/6);  //1/6 of time horizion as resolution

		int ClosestResolution=1;
		for(int i=0; i<6;i++)
		{
			if(	fabs(Value-ResolutionVector[i]) < min_distance)
			{
				min_distance = fabs(Value-ResolutionVector[i]);
				ClosestResolution = ResolutionVector[i];
			}
		}
		return ClosestResolution;
};

	
	virtual ~CTimeSpaceView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTimeSpaceView)
	afx_msg void OnShowTimetable();
	afx_msg void OnUpdateShowTimetable(CCmdUI* pCmdUI);
	afx_msg void OnShowSlacktime();
	afx_msg void OnUpdateShowSlacktime(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimetableResourceprice();
	afx_msg void OnUpdateTimetableResourceprice(CCmdUI *pCmdUI);
	afx_msg void OnTimetableNodecostlabel();
	afx_msg void OnUpdateTimetableNodecostlabel(CCmdUI *pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg void OnToolsLoadvehicletrajactoryfile();
	afx_msg void OnNgsimdataLane1();
	afx_msg void OnNgsimdataLane2();
	afx_msg void OnNgsimdataLane3();
	afx_msg void OnNgsimdataLane4();
	afx_msg void OnNgsimdataLane5();
	afx_msg void OnNgsimdataLane6();
	afx_msg void OnNgsimdataLane7();
	afx_msg void OnNgsimdataColorspeed();
	afx_msg void OnUpdateNgsimdataColorspeed(CCmdUI *pCmdUI);
	afx_msg void OnNgsimdataCarfollowingsimulation();
	afx_msg void OnNgsimdataShowsimulatedresults();
	afx_msg void OnUpdateNgsimdataShowsimulatedresults(CCmdUI *pCmdUI);
	afx_msg void OnUpdateNgsimdataShowbothobservedandsimulationresults(CCmdUI *pCmdUI);
	afx_msg void OnNgsimdataShowbothobservedandsimulationresults();
	afx_msg void OnNgsimdataChangesettingsofcarfollowingmodel();
	afx_msg void OnNgsimdataCarfollowingdataextraction();
	afx_msg void OnNgsimdataShowvehicleid();
	afx_msg void OnUpdateNgsimdataShowvehicleid(CCmdUI *pCmdUI);
	afx_msg void OnNgsimdataShowpreceedingvehicleposition();
	afx_msg void OnUpdateNgsimdataShowpreceedingvehicleposition(CCmdUI *pCmdUI);
	afx_msg void OnNgsimdataSearchByVehicleId();
};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSVIEW_H__81CB4DEC_FFC3_480A_9B92_163C4C8226DA__INCLUDED_)
