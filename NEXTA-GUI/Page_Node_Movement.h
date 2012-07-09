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

#include <vector>
#include "TLiteDoc.h"
#include "TLiteView.h"
// CPage_Node_Movement dialog
#include "CGridListCtrlEx\\CGridListCtrlEx.h"

class PinPoint
{
public:
	PinPoint();
	~PinPoint();
	int nIO; //1 in link 0 out link
	int nLinkID;
	int nNodeID;  //所衔接的另外一个node
	GDPoint pp;   //所衔接的另外一个node的坐标
	GDPoint dpp;  //draw pp, before NPtoSP()
	GDPoint cpp;  //center pp, before NPtoSP()
	bool bSelected;
};
class CPage_Node_Movement : public CPropertyPage
{
	DECLARE_DYNAMIC(CPage_Node_Movement)

public:
	CPage_Node_Movement();
	virtual ~CPage_Node_Movement();

// Dialog Data
	enum { IDD = IDD_DIALOG_NODE_LANE1 };

	CGridListCtrlEx m_ListCtrl;
	int m_CurrentNodeID;
	std::vector<bool> m_SelectedRowVector;
	std::vector<MovementBezier> m_MovementBezierVector;
public: // added 0523
	std::vector<PinPoint*> m_PinPointSet;
	int	m_nSelectedInLinkID;
	int m_nSelectedOutLinkID;
	int m_nSelectedMovementIndex;
	CPoint m_ptCenter;
	double m_dScale;
	bool   m_bLBTNDown;
	HICON m_hIcon;
	CImageList m_ImageList;

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
		pt.x = m_PlotRect.CenterPoint().x + net_point.x * m_dScale;
		pt.y = m_PlotRect.CenterPoint().y - net_point.y * m_dScale;  // -1 here as the dialog screen is up side down
		return pt;
	}

	void DrawMovements(CPaintDC* pDC,CRect PlotRect);
	void DrawLink(CPaintDC* pDC,GDPoint pt1, GDPoint pt2, int NumberOfLanes,double theta, int lane_width);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	int m_CurrentNodeName;
	afx_msg void OnLvnItemchangedGridlistctrlex(NMHDR *pNMHDR, LRESULT *pResult);

public: // JE adds
	void DrawFrame(CPaintDC* pDC,CRect PlotRect);
	void DrawCentroid(CPaintDC* pDC,CRect PlotRect);
	void CreatePinPoints();
	void DrawPinPoints(CPaintDC* pDC,CRect PlotRect);
	PinPoint* PinPointHitTest(CPoint pt);
	void DrawNewMovement(CPaintDC* pDC);
	PinPoint *GetPinPointByLinkID(int nLinkID);
	int UpdateMovementStatus(PinPoint* p=NULL);
	void DrawArrow(CDC* pDC,GDPoint m_One, GDPoint m_Two);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void  UpdateMovement(int nSelectedIndex,int nComboIndex);
	afx_msg void OnLvnEndlabeleditGridlistctrlex(NMHDR *pNMHDR, LRESULT *pResult);
};
