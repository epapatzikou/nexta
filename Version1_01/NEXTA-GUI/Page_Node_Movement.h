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
// CPage_Node_Movement dialog
#include "CGridListCtrlEx\\CGridListCtrlEx.h"

class CPage_Node_Movement : public CPropertyPage
{
	DECLARE_DYNAMIC(CPage_Node_Movement)

public:
	CPage_Node_Movement();
	virtual ~CPage_Node_Movement();

	bool m_bModifiedFlag;
// Dialog Data
	enum { IDD = IDD_DIALOG_NODE_MOVEMENTS };

	CGridListCtrlEx m_ListCtrl;
	int m_SelectedMovementIndex;
	int m_CurrentNodeID;

	std::vector<MovementBezier> m_MovementBezierVector;

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

	void SaveData();
	void DisplayList();
	void UpdateList();

	virtual void OnOK( );
	void DrawMovements(CPaintDC* pDC,CRect PlotRect);
	void DrawLink(CPaintDC* pDC,GDPoint pt1, GDPoint pt2, int NumberOfLanes,double theta, int lane_width);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	int m_CurrentNodeName;
	afx_msg void OnLvnItemchangedGridlistctrlex(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonSave();
	float m_PeakHourFactor;
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonQem();
	int m_CycleLengthInSec;
};
