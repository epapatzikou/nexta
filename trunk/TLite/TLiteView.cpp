// TLiteView.cpp : implementation of the CTLiteView class
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

#include "stdafx.h"
#include "TLite.h"

#include "TLiteDoc.h"
#include "TLiteView.h"
#include "DlgMOE.h"
#include "Network.h"
#include "DlgFindALink.h"
#include "DlgPathMOE.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern std::list<DTALink*>	g_LinkDisplayList;
extern CDlgMOE *g_LinkMOEDlg;
extern CDlgPathMOE	*g_pPathMOEDlg;

extern COLORREF g_MOEDisplayColor[MAX_MOE_DISPLAYCOLOR];

std::list<CTLiteView*>	g_ViewList;

// CTLiteView
void g_UpdateAllViews(int Flag = 0)  //0: update all windows, 1: called from Link MOE, 2: called from path MOE
{
   std::list<CTLiteView*>::iterator it;
   for(it = g_ViewList.begin(); it != g_ViewList.end(); it++)
   {
      if((*it)->GetSafeHwnd ())
      {
	 (*it)->Invalidate (true);
      }
   }


   if(Flag !=1 && g_LinkMOEDlg  && g_LinkMOEDlg ->GetSafeHwnd ())
   {
      g_LinkMOEDlg->Invalidate (true);
   }


   if(Flag !=2&& g_pPathMOEDlg  && g_pPathMOEDlg ->GetSafeHwnd ())
   {
      g_pPathMOEDlg->InsertPathMOEItem();
      g_pPathMOEDlg->Invalidate (true);
   }

}

IMPLEMENT_DYNCREATE(CTLiteView, CView)

BEGIN_MESSAGE_MAP(CTLiteView, CView)

	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_BACKGROUNDCOLOR, &CTLiteView::OnViewBackgroundcolor)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()

	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_VIEW_ZOOMIN, &CTLiteView::OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, &CTLiteView::OnViewZoomout)
	ON_COMMAND(ID_VIEW_SHOWNETWORK, &CTLiteView::OnViewShownetwork)
	ON_COMMAND(ID_VIEW_MOVE, &CTLiteView::OnViewMove)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVE, &CTLiteView::OnUpdateViewMove)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_VIEW_SELECT, &CTLiteView::OnViewSelect)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECT, &CTLiteView::OnUpdateViewSelect)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_NODE_ORIGIN, &CTLiteView::OnNodeOrigin)
	ON_COMMAND(ID_NODE_DESTINATION, &CTLiteView::OnNodeDestination)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_VIEW_SENSOR, &CTLiteView::OnViewSensor)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_SEARCH_FINDLINK, &CTLiteView::OnSearchFindlink)
	ON_COMMAND(ID_VIEW_IMAGE, &CTLiteView::OnEditChangebackgroupimagethroughmouse)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IMAGE, &CTLiteView::OnUpdateEditChangebackgroupimagethroughmouse)
	ON_COMMAND(ID_VIEW_BACKGROUNDIMAGE, &CTLiteView::OnViewBackgroundimage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BACKGROUNDIMAGE, &CTLiteView::OnUpdateViewBackgroundimage)
	ON_COMMAND(ID_VIEW_SHOWLINKTYPE, &CTLiteView::OnViewShowlinktype)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWLINKTYPE, &CTLiteView::OnUpdateViewShowlinktype)
	ON_COMMAND(ID_SHOW_SHOWNODE, &CTLiteView::OnShowShownode)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SHOWNODE, &CTLiteView::OnUpdateShowShownode)
	ON_COMMAND(ID_SHOW_SHOWALLPATHS, &CTLiteView::OnShowShowallpaths)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SHOWALLPATHS, &CTLiteView::OnUpdateShowShowallpaths)
	ON_COMMAND(ID_SHOW_SHOWNODENUMBER, &CTLiteView::OnShowShownodenumber)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SHOWNODENUMBER, &CTLiteView::OnUpdateShowShownodenumber)
END_MESSAGE_MAP()

// CTLiteView construction/destruction
// CTLiteView construction/destruction

CBrush g_BlackBrush(RGB(10,10,10));
CPen g_PenSelectColor(PS_SOLID,2,RGB(255,0,0));
CPen g_PenDisplayColor(PS_SOLID,2,RGB(255,255,0));
CPen g_PenNodeColor(PS_SOLID,1,RGB(0,0,0));
CPen g_PenSensorColor(PS_SOLID,1,RGB(255,255,0));
CPen g_PenNotMatchedSensorColor(PS_SOLID,1,RGB(255,255,255));

CPen g_PenSelectColor0(PS_SOLID,1,RGB(255,0,0));  // red
CPen g_PenSelectColor1(PS_SOLID,1,RGB(0,255,0));  // green
CPen g_PenSelectColor2(PS_SOLID,1,RGB(255,0,255)); //magenta
CPen g_PenSelectColor3(PS_SOLID,1,RGB(0,255,255));   // cyan
CPen g_PenSelectColor4(PS_SOLID,1,RGB(0,0,255));  // blue
CPen g_PenSelectColor5(PS_SOLID,1,RGB(255,255,0)); // yellow

CBrush  g_BrushColor0(HS_BDIAGONAL, RGB(255,0,0));  // red
CBrush  g_BrushColor1(HS_FDIAGONAL,RGB(0,255,0));  // green
CBrush  g_BrushColor2(HS_VERTICAL,RGB(255,0,255)); //magenta
CBrush  g_BrushColor3(HS_HORIZONTAL,RGB(0,255,255));   // cyan
CBrush  g_BrushColor4(HS_CROSS,RGB(0,0,255));  // blue
CBrush  g_BrushColor5(HS_DIAGCROSS,RGB(255,255,0)); // yellow


CPen g_ThickPenSelectColor0(PS_SOLID,3,RGB(255,0,0));  // red
CPen g_ThickPenSelectColor1(PS_SOLID,3,RGB(0,255,0));  // green
CPen g_ThickPenSelectColor2(PS_SOLID,3,RGB(255,0,255)); //magenta
CPen g_ThickPenSelectColor3(PS_SOLID,3,RGB(0,255,255));   // cyan
CPen g_ThickPenSelectColor4(PS_SOLID,3,RGB(0,0,255));  // blue
CPen g_ThickPenSelectColor5(PS_SOLID,3,RGB(255,255,0)); // yellow

CPen g_SuperThickPenSelectColor0(PS_SOLID,5,RGB(255,0,0));  // red
CPen g_SuperThickPenSelectColor1(PS_SOLID,5,RGB(0,255,0));  // green
CPen g_SuperThickPenSelectColor2(PS_SOLID,5,RGB(255,0,255)); //magenta
CPen g_SuperThickPenSelectColor3(PS_SOLID,5,RGB(0,255,255));   // cyan
CPen g_SuperThickPenSelectColor4(PS_SOLID,5,RGB(0,0,255));  // blue
CPen g_SuperThickPenSelectColor5(PS_SOLID,5,RGB(255,255,0)); // yellow

CPen g_PenFreewayColor(PS_SOLID,1,RGB(255,211,155));  
CPen g_PenHighwayColor(PS_SOLID,1,RGB(100,149,237)); 
CPen g_PenArterialColor(PS_SOLID,1,RGB(0,0,0)); 

void g_SelectColorCode(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{
	
	case 0: pDC->SelectObject(&g_PenSelectColor0); break;
	case 1: pDC->SelectObject(&g_PenSelectColor1); break;
	case 2: pDC->SelectObject(&g_PenSelectColor2); break;
	case 3: pDC->SelectObject(&g_PenSelectColor3); break;
	case 4: pDC->SelectObject(&g_PenSelectColor4); break;
	case 5: pDC->SelectObject(&g_PenSelectColor5); break;
	default:
		pDC->SelectObject(&g_PenSelectColor5);
	}

}

void g_SelectThickPenColor(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{
	case 0: pDC->SelectObject(&g_ThickPenSelectColor0); break;
	case 1: pDC->SelectObject(&g_ThickPenSelectColor1); break;
	case 2: pDC->SelectObject(&g_ThickPenSelectColor2); break;
	case 3: pDC->SelectObject(&g_ThickPenSelectColor3); break;
	case 4: pDC->SelectObject(&g_ThickPenSelectColor4); break;
	case 5: pDC->SelectObject(&g_ThickPenSelectColor5); break;
	default:
		pDC->SelectObject(&g_PenSelectColor5);
	}
}

void g_SelectBrushColor(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{
	case 0: pDC->SelectObject(&g_BrushColor0); break;
	case 1: pDC->SelectObject(&g_BrushColor1); break;
	case 2: pDC->SelectObject(&g_BrushColor2); break;
	case 3: pDC->SelectObject(&g_BrushColor3); break;
	case 4: pDC->SelectObject(&g_BrushColor4); break;
	case 5: pDC->SelectObject(&g_BrushColor5); break;
	default:
		pDC->SelectObject(&g_BrushColor0);
	}
}


void g_SelectSuperThickPenColor(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{
	
	case 0: pDC->SelectObject(&g_SuperThickPenSelectColor0); break;
	case 1: pDC->SelectObject(&g_SuperThickPenSelectColor1); break;
	case 2: pDC->SelectObject(&g_SuperThickPenSelectColor2); break;
	case 3: pDC->SelectObject(&g_SuperThickPenSelectColor3); break;
	case 4: pDC->SelectObject(&g_SuperThickPenSelectColor4); break;
	case 5: pDC->SelectObject(&g_SuperThickPenSelectColor5); break;
	default:
		pDC->SelectObject(&g_SuperThickPenSelectColor5);
	}

}



CTLiteView::CTLiteView()
{
	m_ShowAllPaths = true;
	m_OriginOnBottomFlag = -1;
	m_bFitNetworkInitialized = false;
	m_BackgroundColor =  RGB(0,100,0);
//	RGB(102,204,204);
	
	m_ToolMode = move_tool;
	m_bMoveDisplay = false;
	m_bMoveImage = false;
	m_bShowImage = true;
	m_bShowNode = false;
	m_bShowNodeNumber = false;
	m_bShowLinkType  = true;
	m_SelectedNodeID = -1;
	m_SelectedLinkID = -1;



	m_NodeSize = 80;
	m_bShowSensor = true;

	m_SelectFromNodeNumber = 0;
	m_SelectToNodeNumber = 0;

	g_ViewList.push_back(this);


}
CTLiteView::~CTLiteView()
{
}

BOOL CTLiteView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CTLiteView drawing

void CTLiteView::DrawBitmap(CDC *pDC, CPoint point,UINT nIDResource )
{

   CBitmap bitmapImage;
   //load bitmap from resource
   bitmapImage.LoadBitmap(nIDResource);

   BITMAP bmpInfo;
   CSize bitSize;
   //get size of bitmap
   bitmapImage.GetBitmap(&bmpInfo);
   bitSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);

   CDC* pwholeWndDC;
   pwholeWndDC=pDC;

   CImageList imageList;
   //create ImageList
   imageList.Create(bitSize.cx, bitSize.cy, ILC_COLOR|ILC_MASK, 1, 1);
   //select color yellow is transparent color
   imageList.Add(&bitmapImage, RGB(255,255,0));// Add bitmap to CImageList.

   point.x-=bitSize.cx /2;
   point.y-=bitSize.cy /2;


   //drawing the first bitmap of imagelist Transparently
   //imageList.SetBkColor(RGB(0,0,0));
   imageList.Draw(pwholeWndDC,0, point, ILD_TRANSPARENT );

}
void CTLiteView::OnDraw(CDC* pDC)
{
	CRect rectClient(0,0,0,0);
	GetClientRect(&rectClient);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rectClient.Width(),
		rectClient.Height());
	memDC.SelectObject(&bmp);


	// Custom draw on top of memDC
	CBrush brush;
	if (!brush.CreateSolidBrush(m_BackgroundColor))
		return;
	brush.UnrealizeObject();
	memDC.FillRect(rectClient, &brush);

	CTLiteDoc* pDoc = GetDocument();
	if(pDoc->m_BKBitmapLoaded && m_bShowImage)
	{
		pDoc->m_ImageX2  = pDoc->m_ImageX1+ pDoc->m_ImageWidth * pDoc->m_ImageXResolution;
		pDoc->m_ImageY2  = pDoc->m_ImageY1+ pDoc->m_ImageHeight * pDoc->m_ImageYResolution;
		
		GDPoint IMPoint1;
		IMPoint1.x = pDoc->m_ImageX1;
		IMPoint1.y = pDoc->m_ImageY1;

		GDPoint IMPoint2;
		IMPoint2.x = pDoc->m_ImageX2;
		IMPoint2.y = pDoc->m_ImageY2;


		CPoint point1 = NPtoSP(IMPoint1);
		CPoint point2 = NPtoSP(IMPoint2);

		pDoc->m_BKBitmap.StretchBlt(memDC,point1.x,point1.y,point2.x-point1.x,abs(point2.y-point1.y),SRCCOPY);
	}


	DrawObjects(&memDC);

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &memDC, 0,
		0, SRCCOPY);

	ReleaseDC(pDC);

	// TODO: add draw code for native data here
}


// CTLiteView diagnostics

#ifdef _DEBUG
void CTLiteView::AssertValid() const
{
	CView::AssertValid();
}

void CTLiteView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTLiteDoc* CTLiteView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTLiteDoc)));
	return (CTLiteDoc*)m_pDocument;
}
#endif //_DEBUG


// CTLiteView message handlers
void CTLiteView::DrawObjects(CDC* pDC)
{
	CTLiteDoc* pDoc = GetDocument();

	std::list<DTANode*>::iterator iNode;

	if(pDoc->m_NodeSet.size()>0 && !m_bFitNetworkInitialized )
	{
		FitNetworkToScreen();
		m_bFitNetworkInitialized = true;

	}


	pDC->SetBkMode(TRANSPARENT);

	std::list<DTALink*>::iterator iLink;
	std::list<DTALink*>::iterator iLinkDisplay;

	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		CPoint FromPoint = NPtoSP((*iLink)->m_FromPoint);
		CPoint ToPoint = NPtoSP((*iLink)->m_ToPoint);

		CPen LinkTypePen;


		if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
			continue; 

			if((*iLink)->m_DisplayLinkID>=0)
			{
				g_SelectThickPenColor(pDC,(*iLink)->m_DisplayLinkID);
			}else if(m_bShowLinkType)
			{
	//		COLORREF link_color = pDoc->GetLinkTypeColor((*iLink)->m_link_type );
	//		LinkTypePen.CreatePen(PS_SOLID, 1, link_color);
	//		pDC->SelectObject(&LinkTypePen);

			switch ((*iLink)->m_link_type)
			{
			case 1:
				pDC->SelectObject(&g_PenFreewayColor);
				break;
			case 2:
				pDC->SelectObject(&g_PenHighwayColor);
				break;
			default:
				pDC->SelectObject(&g_PenArterialColor);
			
			}
			}else
				pDC->SelectObject(&g_PenArterialColor);
			

		pDC->MoveTo(FromPoint);
		pDC->LineTo(ToPoint);

		// draw sensor flag

		if(m_bShowSensor && (*iLink)->m_bSensorData )
		{
			if((*iLink)->m_LinkID == m_SelectedLinkID)
			{
				pDC->SelectObject(&g_PenSelectColor);
			}else
			{

				pDC->SelectObject(&g_PenSensorColor);
			}		
			CPoint midpoint;
			midpoint.x  = (FromPoint.x + ToPoint.x) / 2;
			midpoint.y  = (FromPoint.y + ToPoint.y) / 2;
			int size = 4;
			pDC->SelectStockObject(NULL_BRUSH);
			pDC->Rectangle(midpoint.x-size, midpoint.y-size, midpoint.x+size, midpoint.y+size);

		}

	}
// draw shortest path
	int i;

	unsigned int iPath;

	if(m_ShowAllPaths)
	{
	for (iPath = 0; iPath < pDoc->m_PathDisplayList.size(); iPath++)
	{
	g_SelectThickPenColor(pDC,iPath);

	for (i=0 ; i<pDoc->m_PathDisplayList[iPath]->m_LinkSize; i++)
	{
		DTALink* pLink = pDoc->m_LinkMap[pDoc->m_PathDisplayList[iPath]->m_LinkVector[i]];
		if(pLink!=NULL)
		{
		CPoint FromPoint = NPtoSP(pLink->m_FromPoint);
		CPoint ToPoint = NPtoSP(pLink->m_ToPoint);

		if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
			continue; 

		pDC->MoveTo(FromPoint);
		pDC->LineTo(ToPoint);
		
		}
	}
	}
	}

	// draw select path
	if((int)(pDoc->m_PathDisplayList.size()) >= pDoc->m_SelectPathNo && pDoc->m_SelectPathNo!=-1)
	{
	g_SelectSuperThickPenColor(pDC,pDoc->m_SelectPathNo);

	for (i=0 ; i<pDoc->m_PathDisplayList[pDoc->m_SelectPathNo]->m_LinkSize; i++)
	{
		DTALink* pLink = pDoc->m_LinkMap[pDoc->m_PathDisplayList[pDoc->m_SelectPathNo]->m_LinkVector[i]];
		if(pLink!=NULL)
		{
		CPoint FromPoint = NPtoSP(pLink->m_FromPoint);
		CPoint ToPoint = NPtoSP(pLink->m_ToPoint);

		if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
			continue; 

		pDC->MoveTo(FromPoint);
		pDC->LineTo(ToPoint);
		
		}
	}
	}
	
	
	


	for (iNode = pDoc->m_NodeSet.begin(); iNode != pDoc->m_NodeSet.end(); iNode++)
	{
		CPoint point = NPtoSP((*iNode)->pt);

		int node_size = max(2,int(m_NodeSize*pDoc->m_UnitFeet*m_Resolution));

		pDC->SelectObject(&g_BlackBrush);
		if((*iNode)->m_NodeID == m_SelectedNodeID)
		{
			pDC->SelectObject(&g_PenSelectColor);
		}else
		{
			pDC->SelectObject(&g_PenNodeColor);
		}

		if((*iNode)->m_NodeID == pDoc->m_OriginNodeID)
		{
			pDC->SelectObject(&g_PenSelectColor);

			node_size *= 1.5;
			pDC->Rectangle (point.x - node_size, point.y + node_size,
				point.x + node_size, point.y - node_size);

			pDC->SelectObject(&g_PenSelectColor);
			pDC->SetTextColor(RGB(255,0,0));
			pDC->SetBkColor(RGB(0,255,0)); 
			pDC->TextOut(point.x - node_size, point.y - node_size, _T("A"));


		}else if((*iNode)->m_NodeID == pDoc->m_DestinationNodeID)
		{
			pDC->SelectObject(&g_PenSelectColor);
			node_size *= 1.5;
			pDC->Rectangle (point.x - node_size, point.y + node_size,
				point.x + node_size, point.y - node_size);
			pDC->SetTextColor(RGB(255,0,0));
			pDC->SetBkColor(RGB(0,255,0));
			pDC->TextOut(point.x - node_size, point.y - node_size, _T("B"));

		}else
		{
			if(m_bShowNode)
			{
						pDC->Ellipse(point.x - node_size, point.y + node_size,
							point.x + node_size, point.y - node_size);
			if(m_bShowNodeNumber)
			{
			CString str_nodenumber;
			str_nodenumber.Format ("%d",(*iNode)->m_NodeNumber );
			pDC->SetTextColor(RGB(255,255,0));
			pDC->TextOut(point.x - node_size, point.y - node_size,str_nodenumber);
			}
			}



		}
	}

	// show not-matched sensors
	if(m_bShowSensor)
	{

				pDC->SelectObject(&g_PenNotMatchedSensorColor);

	std::vector<DTA_sensor>::iterator iSensor;

	for (iSensor = pDoc->m_SensorVector.begin(); iSensor != pDoc->m_SensorVector.end(); iSensor++)
	{
		CPoint point = NPtoSP((*iSensor).pt);

		int sensor_size = 2;
	if((*iSensor).LinkID<0)
	{
			pDC->Ellipse(point.x - sensor_size, point.y + sensor_size,
				point.x + sensor_size, point.y - sensor_size);
	}
		}
	}

	


	std::list<DTAVehicle*>::iterator iVehicle;

}

void CTLiteView::FitNetworkToScreen()
{
	CRect ScreenRect;
	GetClientRect(ScreenRect);
	m_ScreenOrigin = ScreenRect.CenterPoint ();

	CTLiteDoc* pDoc = GetDocument();

	if(pDoc==NULL)
		return;

	double res_wid = ScreenRect.Width()/(pDoc->m_NetworkRect.Width()+1);
	double res_height = ScreenRect.Height()/(pDoc->m_NetworkRect.Height()+1);

	m_Resolution = min(res_wid, res_height);  

	//+1 to avide devided by zero error;

	m_Resolution*=0.8f;

	m_Origin.x = pDoc->m_NetworkRect.Center ().x ;
	m_Origin.y = pDoc->m_NetworkRect.Center ().y ;
//	TRACE("Wid: %d, %f, m_Resolution=%f,m_Origin: x = %f\n",ScreenRect.Width(),pDoc->m_NetworkRect.Width(), m_Resolution, m_Origin.x);

}


// CTLiteView message handlers

void CTLiteView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	OnViewShownetwork();
}



void CTLiteView::OnViewBackgroundcolor()
{
	CColorDialog dlg(RGB(0, 0, 0), CC_FULLOPEN);
	if (dlg.DoModal() == IDOK)
	{
		m_BackgroundColor= dlg.GetColor();
		Invalidate();
	}
}



BOOL CTLiteView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if(m_ToolMode != bkimage_tool)  //select, move
	{
		if(zDelta > 0)
		{
			m_Resolution*=1.1f;
		}
		else
		{
			m_Resolution/=1.1f;
		}
	}else
	{

		// change image size
		CTLiteDoc* pDoc = GetDocument();

		if(zDelta > 0)
		{

			// control -> Y only
			// shift -> X only

			if(nFlags != MK_CONTROL)  // shift or nothing
				pDoc->m_ImageXResolution*=1.02f;

			if(nFlags != MK_SHIFT)  // control or nothing
				pDoc->m_ImageYResolution*=1.02f;
		}
		else
		{
			if(nFlags != MK_CONTROL)  // shift or nothing
				pDoc->m_ImageXResolution/=1.02f;

			if(nFlags != MK_SHIFT)  // control or nothing
				pDoc->m_ImageYResolution/=1.02f;
		}
	
	}

	Invalidate();

	return TRUE;
}

BOOL CTLiteView::OnEraseBkgnd(CDC* pDC)
{

	return TRUE;
}

void CTLiteView::OnViewZoomin()
{
	m_Resolution*=1.1f;
	Invalidate();

}

void CTLiteView::OnViewZoomout()
{
	m_Resolution/=1.1f;
	Invalidate();
}

void CTLiteView::OnViewShownetwork()
{
	FitNetworkToScreen();
	Invalidate();
}

void CTLiteView::OnViewMove()
{
	m_ToolMode = move_tool;
}

int CTLiteView::FindClosestNode(CPoint point, float Min_distance)
{
	int SelectedNodeID = -1;
		CTLiteDoc* pDoc = GetDocument();

		std::list<DTANode*>::iterator iNode;

		for (iNode = pDoc->m_NodeSet.begin(); iNode != pDoc->m_NodeSet.end(); iNode++)
		{
			CPoint NodePoint = NPtoSP((*iNode)->pt);

			CSize size = NodePoint - point;
			double distance = pow((size.cx*size.cx + size.cy*size.cy),0.5);
			if( distance < Min_distance)
			{
				SelectedNodeID = (*iNode)->m_NodeID ;
				Min_distance = distance;
			}

		}

		if(Min_distance > m_NodeSize*30)
		{
			SelectedNodeID = -1;
		}
		return SelectedNodeID;
}

void CTLiteView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(m_ToolMode == move_tool)
	{
		m_last_cpoint = point;
		AfxGetApp()->LoadCursor(IDC_MOVENETWORK);
		m_bMoveDisplay = true;
	}

	if(m_ToolMode == bkimage_tool)
	{
		m_last_cpoint = point;
		AfxGetApp()->LoadCursor(IDC_MOVENETWORK);
		m_bMoveImage = true;
	}

	if(m_ToolMode == select_tool)
	{

		m_SelectedNodeID = FindClosestNode(point, m_NodeSize*2.0f);
		OnClickLink(nFlags, point);

	}

	CView::OnLButtonDown(nFlags, point);
}


void CTLiteView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_ToolMode == move_tool)
	{
		CSize OffSet = point - m_last_cpoint;
		m_Origin.x -= OffSet.cx/m_Resolution;
		m_Origin.y -= OffSet.cy*m_OriginOnBottomFlag/m_Resolution;

		AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		m_bMoveDisplay = false;
	}

	if(m_ToolMode == bkimage_tool)
	{

		CSize OffSet = point - m_last_cpoint;
		CTLiteDoc* pDoc = GetDocument();
		pDoc->m_ImageX1  += OffSet.cx*pDoc->m_ImageMoveSize;
		pDoc->m_ImageY1  += OffSet.cy*m_OriginOnBottomFlag*pDoc->m_ImageMoveSize;

		AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		m_bMoveImage = false;
	}

	Invalidate();

	CView::OnLButtonUp(nFlags, point);
}

void CTLiteView::OnUpdateViewMove(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == move_tool ? 1 : 0);
}

void CTLiteView::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_ToolMode == move_tool && m_bMoveDisplay)
	{
		CSize OffSet = point - m_last_cpoint;
		//		if(!(OffSet.cx !=0 && OffSet.cy !=0))
		{
			m_Origin.x -= OffSet.cx/m_Resolution;
			m_Origin.y -= OffSet.cy*m_OriginOnBottomFlag/m_Resolution;
			m_last_cpoint = point;

		}

	}

	if(m_ToolMode == bkimage_tool && m_bMoveImage)
	{
		CSize OffSet = point - m_last_cpoint;
		//		if(!(OffSet.cx !=0 && OffSet.cy !=0))
		{
		CTLiteDoc* pDoc = GetDocument();
		pDoc->m_ImageX1  += OffSet.cx*pDoc->m_ImageMoveSize;
		pDoc->m_ImageY1  += OffSet.cy*m_OriginOnBottomFlag*pDoc->m_ImageMoveSize;

		m_last_cpoint = point;

		}

	}

	Invalidate();

	CView::OnMouseMove(nFlags, point);
}

void CTLiteView::OnViewSelect()
{
	m_ToolMode = select_tool;
}

void CTLiteView::OnUpdateViewSelect(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == select_tool ? 1 : 0);
}

void CTLiteView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnRButtonDown(nFlags, point);
}

void CTLiteView::OnNodeOrigin()
{
	m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	CTLiteDoc* pDoc = GetDocument();
	pDoc->m_OriginNodeID = m_SelectedNodeID;
	pDoc->Routing();
}

void CTLiteView::OnNodeDestination()
{
	m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	CTLiteDoc* pDoc = GetDocument();
	pDoc->m_DestinationNodeID = m_SelectedNodeID;
	pDoc->Routing();
}

void CTLiteView::OnContextMenu(CWnd* pWnd, CPoint point)
{


	CPoint MenuPoint = point;
	CTLiteDoc* pDoc = GetDocument();
	ScreenToClient(&point);

	m_CurrentMousePoint = point;

//	CPoint select_pt = NPtoSP (pDoc->m_NodeMap[m_SelectedNodeID]->pt);

//	CSize size = point-select_pt;
//	if(pow((size.cx*size.cx + size.cy*size.cy),0.5) < m_NodeSize*2)
	{
		// In our client area - load the context menu
		CMenu cm;
		cm.LoadMenu(IDR_MENU1);

		// Get point in logical coordinates
		CClientDC dc(this);
		// Put it up
		cm.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			MenuPoint.x, MenuPoint.y, this);
	}
}

void CTLiteView::OnViewSensor()
{
	m_bShowSensor = !m_bShowSensor;
	Invalidate();
}

void CTLiteView::OnClickLink(UINT nFlags, CPoint point)
{

	CTLiteDoc* pDoc = GetDocument();
	double Min_distance = 1000;

	std::list<DTALink*>::iterator iLink;

	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		CPoint FromPoint = NPtoSP((*iLink)->m_FromPoint);
		CPoint ToPoint = NPtoSP((*iLink)->m_ToPoint);

		GDPoint p0, pfrom, pto;
		p0.x  = point.x; p0.y  = point.y;
		pfrom.x  = FromPoint.x; pfrom.y  = FromPoint.y;
		pto.x  = ToPoint.x; pto.y  = ToPoint.y;

		float distance = g_DistancePointLine(p0, pfrom, pto);

		if(distance >0 && distance < Min_distance)
		{
			m_SelectedLinkID = (*iLink)->m_LinkID ;

			Min_distance = distance;
		}
	}		

	if(Min_distance > m_NodeSize*2)
	{
		m_SelectedLinkID = -1;
		g_LinkDisplayList.clear ();
	}else
	{
		// if the control key is not pressed 
		if ( (nFlags & MK_CONTROL) ==false)
		{
			g_LinkDisplayList.clear ();
		}

		g_LinkDisplayList.push_back(pDoc->m_LinkMap[m_SelectedLinkID]);

	
	}

	std::list<DTALink*>::iterator iLinkDisplay;

	// reset
	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_DisplayLinkID = -1;
	}

	int LinkCount=0;
	for (iLinkDisplay = g_LinkDisplayList.begin(); iLinkDisplay != g_LinkDisplayList.end(); iLinkDisplay++, LinkCount++)
	{
		(*iLinkDisplay)->m_DisplayLinkID = LinkCount;

	}

	if(g_LinkDisplayList.size() > 0 && g_LinkMOEDlg!=NULL)  // if g_LinkMOEDlg is open, update
	{
		g_LinkMOEDlg->Invalidate ();
		g_LinkMOEDlg->ShowWindow(SW_SHOW);

	}


}
void CTLiteView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// create MOE Dlg when double clicking
		if(g_LinkMOEDlg==NULL)
		{
			g_LinkMOEDlg = new CDlgMOE();
			g_LinkMOEDlg->Create(IDD_DIALOG_MOE);

		}

	OnClickLink(nFlags, point);

	/*		
	std::list<DTANode*>::iterator iNode;


	for (iNode = pDoc->m_NodeSet.begin(); iNode != pDoc->m_NodeSet.end(); iNode++)
	{
	CPoint NodePoint = NPtoSP((*iNode)->pt);

	CSize size = NodePoint - point;
	double distance = pow((size.cx*size.cx + size.cy*size.cy),0.5);
	if( distance < Min_distance)
	{
	m_SelectedNodeID = (*iNode)->m_NodeID ;
	Min_distance = distance;
	return;
	}

	}

	if(Min_distance > m_NodeSize*2)
	{
	m_SelectedNodeID = -1;
	}

	*/

	CView::OnLButtonDblClk(nFlags, point);
}

void CTLiteView::OnSearchFindlink()
{
	CDlgFindALink dlg;


	dlg.m_FromNodeNumber = m_SelectFromNodeNumber;
	dlg.m_ToNodeNumber = m_SelectToNodeNumber;

	if(dlg.DoModal () == IDOK)
	{
		CTLiteDoc* pDoc = GetDocument();

		DTALink* pLink = pDoc->FindLinkWithNodeNumbers(dlg.m_FromNodeNumber ,dlg.m_ToNodeNumber );

		if(pLink !=NULL)
		{
			m_SelectedLinkID = pLink->m_LinkID ;

			m_SelectFromNodeNumber = dlg.m_FromNodeNumber;
			m_SelectToNodeNumber = dlg.m_ToNodeNumber;

		}

		Invalidate();

	}

}

void CTLiteView::OnEditChangebackgroupimagethroughmouse()
{
	m_ToolMode = bkimage_tool;
}

void CTLiteView::OnUpdateEditChangebackgroupimagethroughmouse(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == bkimage_tool ? 1 : 0);
}

void CTLiteView::OnViewBackgroundimage()
{
	m_bShowImage = !m_bShowImage;
	Invalidate();
}

void CTLiteView::OnUpdateViewBackgroundimage(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowImage);
}

void CTLiteView::OnViewShowlinktype()
{
	m_bShowLinkType = !m_bShowLinkType;
	Invalidate();
}

void CTLiteView::OnUpdateViewShowlinktype(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowLinkType);

}


void CTLiteView::OnShowShownode()
{
	m_bShowNode = !m_bShowNode;
	Invalidate();
}

void CTLiteView::OnUpdateShowShownode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowNode);
}

void CTLiteView::OnShowShowallpaths()
{
	m_ShowAllPaths = !m_ShowAllPaths;
	Invalidate();
}

void CTLiteView::OnUpdateShowShowallpaths(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ShowAllPaths);
}



void CTLiteView::OnShowShownodenumber()
{
	m_bShowNodeNumber = !m_bShowNodeNumber;

	if(m_bShowNodeNumber == true)
		m_bShowNode = true;
	Invalidate();

}

void CTLiteView::OnUpdateShowShownodenumber(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowNodeNumber);
	
}
