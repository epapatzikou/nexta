#pragma once

#include <vector>
#include "TLiteDoc.h"
#include "TLiteView.h"
// CPage_Node_LaneTurn dialog
#include "CGridListCtrlEx\\CGridListCtrlEx.h"
#include "VISSIM_EXPORT\\VISSIM_Export.h"
#include "page_node_lane.h"

class CPage_Node_LaneTurn : public CPropertyPage
{
	DECLARE_DYNAMIC(CPage_Node_LaneTurn)

public:
	CPage_Node_LaneTurn();
	virtual ~CPage_Node_LaneTurn();

// Dialog Data
	enum { IDD = IDD_DIALOG_NODE_LANE2 };

	CGridListCtrlEx m_ListCtrl;
	int m_SelectedMovementIndex;
	int m_CurrentNodeID;

	std::vector<MLink*> m_seriLink;  // serialed links
	void SerialLink(MLink* pmLink); // insert a link in the angle order, set nIO before calling

	std::vector<bool> m_SelectedRowVector;
	std::vector<MovementBezier> m_MovementBezierVector;
	std::ofstream m_logFile;

	int FindClickedMovement(CPoint pt)
	{
	
		float min_distance  = 50;
		int SelectedMovement = -1;

		for(unsigned int i = 0; i< m_MovementBezierVector.size(); i++)
		{
			float distance  = m_MovementBezierVector[i].GetMinDistance(pt) ;

			if(distance < min_distance)
			{
				min_distance = distance;
				SelectedMovement = i;
			}
		}

		return SelectedMovement;
	}
	
	CTLiteDoc* m_pDoc;
	CTLiteView* m_pView;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	CRect m_PlotRect;
	CPoint NPtoSP(GDPoint net_point)
	{
		CPoint pt;
		pt.x = m_PlotRect.CenterPoint().x + net_point.x;
		pt.y = m_PlotRect.CenterPoint().y - net_point.y;  // -1 here as the dialog screen is up side down
		return pt;
	}

	void DrawMovementsOld(CPaintDC* pDC,CRect PlotRect);
	void DrawMovements(CPaintDC* pDC,CRect PlotRect);
	void DrawLink(CPaintDC* pDC,GDPoint pt1, GDPoint pt2, int NumberOfLanes,double theta, int lane_width);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	int m_CurrentNodeName;
	afx_msg void OnLvnItemchangedGridlistctrlex(NMHDR *pNMHDR, LRESULT *pResult);

	void MCreateDefaultLaneTurns();
	MLink* GetMLink(int nLinkID,int nInOut);
	bool OpenLogFile(std::string strLogFileName);
	bool CloseLogFile();

	DrawPara	m_Para;
	std::vector<LinkRecord*> m_LinkData;
	int m_nInLinkIDSelected;
	int m_nOutLinkIDSelected;
	int m_nInLaneNumSelected;
	int m_nOutLaneNumSelected;
	int m_nTurnSelected;


	void SetDrawPara();
	void InitLinkData();
	void FillTurnData();
	void SerialLinkRecord(LinkRecord* pRecord);
	void NormalizeAngles();
	void CalculateOA();
	void DrawTest(CPaintDC* pDC,CRect PlotRect);
	void DrawCentroidRadicalLines(CPaintDC* pDC,CRect PlotRect,bool bOriginAngle=true);
	void DrawRadicalLine(CPaintDC* pDC,LinkRecord* pRecord,bool bOriginAngle=true);
	void CreateGraphics();
	int  FindClickedLink(CPoint pt);
	afx_msg void OnBnClickedBtnDelturn();
	afx_msg void OnBnClickedBtnAddturn();
};
