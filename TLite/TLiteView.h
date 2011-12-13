// TLiteView.h : interface of the CTLiteView class
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
#include <afxstr.h>
#include <atlimage.h>
#include <comdef.h>



enum tool
   { move_tool, select_tool, select_node_tool, bkimage_tool, create_1waylink_tool, create_2waylinks_tool, create_node_tool, subarea_tool
   };


class CTLiteView : public CView
{
protected: // create from serialization only
	CTLiteView();
	DECLARE_DYNCREATE(CTLiteView)

// Attributes
public:

	bool isCreatingSubarea, isFinishSubarea;
	
	int m_SelectFromNodeNumber; 
	int m_SelectToNodeNumber;

	bool m_bShowSensor;
	bool m_bShowText;
	bool m_bShowAVISensor;

	LPPOINT m_subarea_points;
	// Create a polygonal region
    HRGN m_polygonal_region;

	CTLiteDoc* GetDocument() const;
	tool m_ToolMode; 
	
	double m_GridResolution;
	CPoint m_last_cpoint;
	bool m_bMoveDisplay;

	bool m_bMoveImage;
	bool m_bShowGrid;
	bool m_bShowLinkArrow;
	bool m_bShowNode;
	bool m_bShowNodeNumber;
	bool m_bShowImage;
	bool m_bShowLinkType;

	int m_NodeSize;
	int m_VehicleSize;


		COLORREF m_BackgroundColor;
	void FitNetworkToScreen();
	void DrawObjects(CDC* pDC);
	void DrawBitmap(CDC *pDC, CPoint point,UINT nIDResource );
	void DrawTemporalLink(CPoint start_point, CPoint end_point);

	CPoint m_TempLinkStartPoint, m_TempLinkEndPoint;
	CPoint m_TempZoneStartPoint, m_TempZoneEndPoint;

	CPoint m_FirstSubareaPoints;

	bool bFindCloseSubareaPoint(CPoint pt)
	{
		if(GetDocument()->m_SubareaShapePoints.size()>0)
		{
			double value = (pt.x - m_FirstSubareaPoints.x)*(pt.x - m_FirstSubareaPoints.x) + (pt.y - m_FirstSubareaPoints.y)*(pt.y -m_FirstSubareaPoints.y);
		double distance = sqrt(value);
			if(distance < 10) 
				return true;

		}

		return false;
	}

	bool m_bMouseDownFlag;

	CPoint m_ScreenOrigin;
	GDPoint m_Origin;
	float m_Resolution;

	int m_OriginOnBottomFlag;

	int m_ViewID;

	void FollowGlobalViewParameters(int CurrentViewID)
	{
/*		if(GetDocument()->m_bSetView && m_ViewID!=CurrentViewID)
		{
		 GetDocument()->m_Doc_Origin = m_Origin;
		 GetDocument()->m_Doc_Resolution = m_Resolution;
		}
		 
*/
	}
	void SetGlobalViewParameters()
	{
/*		m_Origin = GetDocument()->m_Doc_Origin;
		m_Resolution = GetDocument()->m_Doc_Resolution;
		GetDocument()->m_bSetView = true;
		GetDocument()->m_CurrentViewID  = m_ViewID;
		GetDocument()->UpdateAllViews(this);
*/
	}


    CPoint NPtoSP(GDPoint net_point) // convert network coordinate to screen coordinate
	{
		CPoint pt;
		pt.x = int((net_point.x-m_Origin.x)*m_Resolution+ m_ScreenOrigin.x+0.5);
		pt.y = int((net_point.y-m_Origin.y)*m_OriginOnBottomFlag*m_Resolution + m_ScreenOrigin.y+0.5);
		return pt;
	}

    GDPoint SPtoNP(CPoint screen_point)
	{
		GDPoint pt;
		pt.x = (screen_point.x-m_ScreenOrigin.x)/m_Resolution+m_Origin.x;
		pt.y = (screen_point.y-m_ScreenOrigin.y)/(m_OriginOnBottomFlag*m_Resolution)+m_Origin.y;
	
		return pt;
	}

	int FindClosestNode(CPoint point, float Min_distance);
	CPoint m_CurrentMousePoint;
	

// Operations
public:

	bool m_ShowAllPaths;

	void CopyLinkSetInSubarea();


// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CTLiteView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void OnClickLink(UINT nFlags, CPoint point);
protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewBackgroundcolor();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewShownetwork();
	afx_msg void OnViewMove();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnUpdateViewMove(CCmdUI *pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnViewSelect();
	afx_msg void OnUpdateViewSelect(CCmdUI *pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNodeOrigin();
	afx_msg void OnNodeDestination();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
public:
	afx_msg void OnViewSensor();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSearchFindlink();
	afx_msg void OnEditChangebackgroupimagethroughmouse();
	afx_msg void OnUpdateEditChangebackgroupimagethroughmouse(CCmdUI *pCmdUI);
	afx_msg void OnViewBackgroundimage();
	afx_msg void OnUpdateViewBackgroundimage(CCmdUI *pCmdUI);
	afx_msg void OnViewShowlinktype();
	afx_msg void OnUpdateViewShowlinktype(CCmdUI *pCmdUI);
	afx_msg void OnShowShownode();
	afx_msg void OnUpdateShowShownode(CCmdUI *pCmdUI);
	afx_msg void OnShowShowallpaths();
	afx_msg void OnUpdateShowShowallpaths(CCmdUI *pCmdUI);
	afx_msg void OnShowShownodenumber();
	afx_msg void OnUpdateShowShownodenumber(CCmdUI *pCmdUI);
	afx_msg void OnEditCreate1waylink();
	afx_msg void OnEditCreate2waylinks();
	afx_msg void OnUpdateEditCreate1waylink(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditCreate2waylinks(CCmdUI *pCmdUI);
	afx_msg void OnShowGrid();
	afx_msg void OnUpdateShowGrid(CCmdUI *pCmdUI);
	afx_msg void OnShowLinkarrow();
	afx_msg void OnUpdateShowLinkarrow(CCmdUI *pCmdUI);
	afx_msg void OnViewShowmoe();
	afx_msg void OnUpdateViewShowmoe(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMoeVehicle(CCmdUI *pCmdUI);
	afx_msg void OnImageLockbackgroundimageposition();
	afx_msg void OnViewTextlabel();
	afx_msg void OnUpdateViewTextlabel(CCmdUI *pCmdUI);
	afx_msg void OnLinkDelete();
	afx_msg void OnUpdateEditDeleteselectedlink(CCmdUI *pCmdUI);
	afx_msg void OnLinkEditlink();
	afx_msg void OnEditCreatenode();
	afx_msg void OnUpdateEditCreatenode(CCmdUI *pCmdUI);
	afx_msg void OnEditDeleteselectednode();
	afx_msg void OnEditSelectnode();
	afx_msg void OnViewSelectNode();
	afx_msg void OnUpdateViewSelectNode(CCmdUI *pCmdUI);
	afx_msg void OnSearchNode();
	afx_msg void OnEditCreatesubarea();
	afx_msg void OnUpdateEditCreatesubarea(CCmdUI *pCmdUI);
	afx_msg void OnToolsRemovenodesandlinksoutsidesubarea();
	afx_msg void OnViewShowAVISensor();
	afx_msg void OnUpdateViewShowAVISensor(CCmdUI *pCmdUI);
	afx_msg void OnFileDataexchangewithgooglefusiontables();
};

#ifndef _DEBUG  // debug version in TLiteView.cpp
inline CTLiteDoc* CTLiteView::GetDocument() const
   { return reinterpret_cast<CTLiteDoc*>(m_pDocument); }
#endif

