// TLiteView.cpp : implementation of the CTLiteView class
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

#include "stdafx.h"
#include "TLite.h"
#include "math.h"

#include "TLiteDoc.h"
#include "TLiteView.h"
#include "DlgMOE.h"
#include "Network.h"
#include "DlgFindALink.h"
#include "DlgPathMOE.h"
#include "GLView.h"
#include "DlgLinkProperties.h"
#include "DlgFindANode.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GDPoint g_Origin;
float g_Resolution;

extern std::list<DTALink*>	g_LinkDisplayList;
extern CDlgMOE *g_LinkMOEDlg;

extern COLORREF g_MOEDisplayColor[MAX_MOE_DISPLAYCOLOR];
extern float g_Simulation_Time_Stamp;

int g_ViewID = 0;
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
	ON_COMMAND(ID_Edit_Create1WayLink, &CTLiteView::OnEditCreate1waylink)
	ON_COMMAND(ID_Edit_Create2WayLinks, &CTLiteView::OnEditCreate2waylinks)
	ON_UPDATE_COMMAND_UI(ID_Edit_Create1WayLink, &CTLiteView::OnUpdateEditCreate1waylink)
	ON_UPDATE_COMMAND_UI(ID_Edit_Create2WayLinks, &CTLiteView::OnUpdateEditCreate2waylinks)
	ON_COMMAND(ID_SHOW_GRID, &CTLiteView::OnShowGrid)
	ON_UPDATE_COMMAND_UI(ID_SHOW_GRID, &CTLiteView::OnUpdateShowGrid)
	ON_COMMAND(ID_SHOW_LINKARROW, &CTLiteView::OnShowLinkarrow)
	ON_UPDATE_COMMAND_UI(ID_SHOW_LINKARROW, &CTLiteView::OnUpdateShowLinkarrow)
	ON_COMMAND(ID_VIEW_SHOWMOE, &CTLiteView::OnViewShowmoe)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWMOE, &CTLiteView::OnUpdateViewShowmoe)
	ON_COMMAND(ID_IMAGE_LOCKBACKGROUNDIMAGEPOSITION, &CTLiteView::OnImageLockbackgroundimageposition)
	ON_COMMAND(ID_VIEW_TEXTLABEL, &CTLiteView::OnViewTextlabel)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTLABEL, &CTLiteView::OnUpdateViewTextlabel)
	ON_COMMAND(ID_LINK_DELETE, &CTLiteView::OnLinkDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETESELECTEDLINK, &CTLiteView::OnUpdateEditDeleteselectedlink)
	ON_COMMAND(ID_LINK_EDITLINK, &CTLiteView::OnLinkEditlink)
	ON_COMMAND(ID_EDIT_CREATENODE, &CTLiteView::OnEditCreatenode)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CREATENODE, &CTLiteView::OnUpdateEditCreatenode)
	ON_COMMAND(ID_EDIT_DELETESELECTEDNODE, &CTLiteView::OnEditDeleteselectednode)
	ON_COMMAND(ID_EDIT_SELECTNODE, &CTLiteView::OnEditSelectnode)
	ON_COMMAND(ID_VIEW_SELECT_NODE, &CTLiteView::OnViewSelectNode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECT_NODE, &CTLiteView::OnUpdateViewSelectNode)
	ON_COMMAND(ID_SEARCH_NODE, &CTLiteView::OnSearchNode)
END_MESSAGE_MAP()

// CTLiteView construction/destruction
// CTLiteView construction/destruction

CBrush g_BlackBrush(RGB(10,10,10));
CPen g_BlackPen(PS_SOLID,1,RGB(0,0,0));
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
CPen g_TempLinkPen(PS_DASH,0,RGB(255,255,255));
CPen g_GridPen(PS_SOLID,1,RGB(190,190,190));

CPen g_PenVehicle(PS_SOLID,1,RGB(0,255,0));  // yellow
CPen g_BrushVehicle(PS_SOLID,1,RGB(0,255,0)); //magenta

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
	m_ViewID = g_ViewID++;
	m_bShowText = true;
	m_bMouseDownFlag = false;
	m_ShowAllPaths = true;
	m_OriginOnBottomFlag = -1;
	m_BackgroundColor =  RGB(0,100,0);
	//	RGB(102,204,204);

	m_ToolMode = move_tool;
	m_bMoveDisplay = false;
	m_bMoveImage = false;
	m_bShowImage = true;
	m_bShowGrid  = true;
	m_bShowLinkArrow = true;
	m_bShowNode = false;
	m_bShowNodeNumber = false;
	m_bShowLinkType  = true;


	m_Origin.x = 0;
	m_Origin.y = 0;

	m_NodeSize = 80;
	m_VehicleSize = 1;
	m_bShowSensor = true;
	
	m_SelectFromNodeNumber = 0;
	m_SelectToNodeNumber = 0;

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
	FollowGlobalViewParameters(this->m_ViewID);

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
	if(pDoc->m_BackgroundBitmapLoaded && m_bShowImage)
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

		pDoc->m_BackgroundBitmap.StretchBlt(memDC,point1.x,point1.y,point2.x-point1.x,abs(point2.y-point1.y),SRCCOPY);
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

	if(!pDoc->m_bFitNetworkInitialized  )
	{
		FitNetworkToScreen();
		pDoc->m_bFitNetworkInitialized = true;

	}

	pDC->SetBkMode(TRANSPARENT);

	// draw grids
	if(m_bShowGrid)
	{
		pDC->SelectObject(&g_GridPen);
		pDC->SetTextColor(RGB(255,228,181));

		CPoint FromPoint, ToPoint;
		GDPoint fromp, top;

		CRect ScreenRect;
		GetClientRect(ScreenRect);

		// get the closest power 10 number
		m_GridResolution = g_FindClosestYResolution(ScreenRect.Width ()/m_Resolution/10.0f);

		int LeftX  = int(SPtoNP(ScreenRect.TopLeft()).x);

		if(m_GridResolution>1)
			LeftX = LeftX- LeftX%int(m_GridResolution);

		for(double x = LeftX; x<= int(SPtoNP(ScreenRect.BottomRight ()).x)+1; x+=m_GridResolution)
		{
			fromp.x = x;
			fromp.y = SPtoNP(ScreenRect.TopLeft()).y;

			top.x = x;
			top.y = SPtoNP(ScreenRect.BottomRight()).y;

			FromPoint = NPtoSP(fromp);
			ToPoint = NPtoSP(top);

			pDC->SelectObject(&g_GridPen);

			pDC->MoveTo(FromPoint);
			pDC->LineTo(ToPoint);

			CString str;

			if(m_GridResolution<1)
				str.Format ("%.2f",x);
			else
				str.Format ("%.0f",x);

			pDC->TextOut(FromPoint.x,ScreenRect.TopLeft().y+10,str);
		}

		int BottomY  = int(SPtoNP(ScreenRect.BottomRight()).y);

		if(m_GridResolution>1)
			BottomY = BottomY- BottomY%int(m_GridResolution);

		for(double y = BottomY; y<= int(SPtoNP(ScreenRect.TopLeft ()).y)+1; y+=m_GridResolution)
		{

			fromp.x =  SPtoNP(ScreenRect.TopLeft()).x ;
			fromp.y = y;

			top.x = SPtoNP(ScreenRect.BottomRight()).x ;
			top.y = y ;

			FromPoint = NPtoSP(fromp);
			ToPoint = NPtoSP(top);

			pDC->MoveTo(FromPoint);
			pDC->LineTo(ToPoint);

			CString str;
			if(m_GridResolution<1)
				str.Format ("%.2f",y);
			else
				str.Format ("%.0f",y);

			pDC->TextOut(ScreenRect.TopLeft().x+10,FromPoint.y,str);

		}



	}

	// draw links
	std::list<DTALink*>::iterator iLink;
	std::list<DTALink*>::iterator iLinkDisplay;

	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		//		if(((*iLink)->m_LaneCapacity >3000))  // 
		//			continue;

		GDPoint P0 = (*iLink)->m_FromPoint;
		GDPoint P1 = (*iLink)->m_FromPoint;

		CPoint FromPoint = NPtoSP((*iLink)->m_FromPoint);
		CPoint ToPoint = NPtoSP((*iLink)->m_ToPoint);

		CPen LinkTypePen;
		CPen penmoe;
		float value = -1.0f ;

		if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
			continue; 

		if( 
			pDoc->m_LinkMOEMode != none && (
			(g_Simulation_Time_Stamp >=1 &&	g_Simulation_Time_Stamp < (*iLink)->m_SimulationHorizon) 
			|| 	pDoc->m_StaticAssignmentMode)) 
		{

			//float power = pDoc->GetLinkMOE((*iLink), pDoc->m_LinkMOEMode , (int)g_Simulation_Time_Stamp);

			float power;

				power= pDoc->GetTDLinkMOE((*iLink), pDoc->m_LinkMOEMode,(int)g_Simulation_Time_Stamp,value);

			float n= power*100;
			int R=(int)((255*n)/100);
			int G=(int)(255*(100-n)/100); 
			int B=0;

			penmoe.CreatePen (PS_SOLID, 1, RGB(R,G,B));
			pDC->SelectObject(&penmoe);


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

		if((*iLink)->m_DisplayLinkID>=0 )
		{
			g_SelectThickPenColor(pDC,(*iLink)->m_DisplayLinkID);
			pDC->SetTextColor(RGB(255,0,0));
		}else if  ((*iLink)->m_LinkID == pDoc->m_SelectedLinkID)
		{
			g_SelectThickPenColor(pDC,0);
			pDC->SetTextColor(RGB(255,0,0));
		}else
			pDC->SetTextColor(RGB(255,228,181));

		if( m_bShowText  && value>=0.01 )
		{
			CString str_text;

			int value_int100 = int(value*100);
			int value_int= (int)value;

			if(value_int100%100 == 0)  // no decimal point
				str_text.Format ("%d", value_int);
			else
				str_text.Format ("%4.1f", value);

			CPoint TextPoint; 
			TextPoint.x = FromPoint.x + (ToPoint.x - FromPoint.x)*2/3;
			TextPoint.y = FromPoint.y + (ToPoint.y - FromPoint.y)*2/3;

			pDC->TextOut(TextPoint.x,TextPoint.y, str_text);

		}

		if( pDoc->m_LinkMOEMode == vehicle)  // when showing vehicles, use black
			pDC->SelectObject(&g_BlackPen);

		pDC->MoveTo(FromPoint);
		pDC->LineTo(ToPoint);

		if(m_bShowLinkArrow)
		{
			double slopy = atan2((double)(FromPoint.y - ToPoint.y), (double)(FromPoint.x - ToPoint.x));
			double cosy = cos(slopy);
			double siny = sin(slopy);   
			double display_length  = sqrt((double)(FromPoint.y - ToPoint.y)*(FromPoint.y - ToPoint.y)+(double)(FromPoint.x - ToPoint.x)*(FromPoint.x - ToPoint.x));
			double arrow_size = min(7,display_length/5.0);

			if(arrow_size>0.2)
			{
				CPoint pts[3];
				pts[0] = ToPoint;
				pts[1].x = ToPoint.x + (int)(arrow_size * cosy - (arrow_size / 2.0 * siny) + 0.5);
				pts[1].y = ToPoint.y + (int)(arrow_size * siny + (arrow_size / 2.0 * cosy) + 0.5);
				pts[2].x = ToPoint.x + (int)(arrow_size * cosy + arrow_size / 2.0 * siny + 0.5);
				pts[2].y = ToPoint.y - (int)(arrow_size / 2.0 * cosy - arrow_size * siny + 0.5);

				pDC->Polygon(pts, 3);
			}

		}


		// draw link arrow

		//************************************



		// draw sensor flag

		if(m_bShowSensor && (*iLink)->m_bSensorData )
		{
			if((*iLink)->m_LinkID == pDoc->m_SelectedLinkID)
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
				DTALink* pLink = pDoc->m_LinkIDMap[pDoc->m_PathDisplayList[iPath]->m_LinkVector[i]];
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
			DTALink* pLink = pDoc->m_LinkIDMap[pDoc->m_PathDisplayList[pDoc->m_SelectPathNo]->m_LinkVector[i]];
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

		int node_size = min(4,max(2,int(m_NodeSize*pDoc->m_UnitFeet*m_Resolution)));

		pDC->SelectObject(&g_BlackBrush);
		if((*iNode)->m_NodeID == pDoc->m_SelectedNodeID)
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


	if( pDoc->m_LinkMOEMode == vehicle)
	{

		pDC->SelectObject(&g_PenVehicle);  //green
		pDC->SelectObject(&g_BrushVehicle); //green

		int vehicle_size = 1;

		if(m_GridResolution<2)
			vehicle_size = 2;

		if(m_GridResolution<1)
			vehicle_size = 3;

		if(m_GridResolution<0.5)
			vehicle_size = 4;

		std::list<DTAVehicle*>::iterator iVehicle;
		for (iVehicle = pDoc->m_VehicleSet.begin(); iVehicle != pDoc->m_VehicleSet.end(); iVehicle++)
		{
			if((*iVehicle)->m_bComplete && (*iVehicle)->m_DepartureTime <=g_Simulation_Time_Stamp &&
				g_Simulation_Time_Stamp <=(*iVehicle)->m_ArrivalTime && (*iVehicle)->m_NodeSize>=2)
			{

				float ratio = 0;
				int LinkID = pDoc->GetVehilePosition((*iVehicle), g_Simulation_Time_Stamp,ratio);

				DTALink* pLink = pDoc->m_LinkIDMap[LinkID];
				if(pLink!=NULL)
				{
					CPoint FromPoint = NPtoSP(pLink->m_FromPoint);
					CPoint ToPoint = NPtoSP(pLink->m_ToPoint);
					CPoint VehPoint;
					VehPoint.x= ratio*FromPoint.x + (1-ratio)*ToPoint.x;
					VehPoint.y= ratio*FromPoint.y + (1-ratio)*ToPoint.y;


					pDC->Ellipse (VehPoint.x - vehicle_size, VehPoint.y - vehicle_size,
						VehPoint.x + vehicle_size, VehPoint.y + vehicle_size);
				}

			}

		}
	}
	//////////////////////////////////////
	// draw OD demand

	if(pDoc->m_LinkMOEMode == oddemand && pDoc->m_DemandMatrix!=NULL)
	{
		int i,j;
		for (i = 0; i< pDoc->m_ODSize ; i++)
			for (j = 0; j< pDoc->m_ODSize ; j++)
			{

				float volume = pDoc->m_DemandMatrix [i][j] ;
				if(volume>=1)
				{
					int nodeid_for_origin_zone = pDoc->m_ZoneIDtoNodeIDMap[i];
					int nodeid_for_destination_zone = pDoc->m_ZoneIDtoNodeIDMap[j];

					// if default, return node id;
					//else return the last node id for zone
					DTANode* pNodeOrigin= pDoc->m_NodeIDMap[i];
					DTANode* pNodeDestination= pDoc->m_NodeIDMap[j];

					if(pNodeOrigin!=NULL  && pNodeDestination!=NULL)
					{
						CPoint FromPoint = NPtoSP(pNodeOrigin->pt);
						CPoint ToPoint = NPtoSP(pNodeDestination->pt);

						CPen penmoe;
						int Width = volume/pDoc->m_MaxODDemand*10;

						if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
							continue; 

					if(Width>=1)  //draw critical OD demand only
					{
						penmoe.CreatePen (PS_SOLID, Width, RGB(0,255,255));
						pDC->SelectObject(&penmoe);
						pDC->MoveTo(FromPoint);
						pDC->LineTo(ToPoint);
					}

					}

				}
			}
	}



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
	SetGlobalViewParameters();

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
	SetGlobalViewParameters();

	Invalidate();

}

void CTLiteView::OnViewZoomout()
{
	m_Resolution/=1.1f;
	SetGlobalViewParameters();
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
			CTLiteDoc* pDoc = GetDocument();

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
		pDoc->m_SelectedNodeID = -1;
		OnClickLink(nFlags, point);
	}

		if(m_ToolMode == select_node_tool)
	{
		pDoc->m_SelectedLinkID = -1;
		pDoc->m_SelectedNodeID = FindClosestNode(point, m_NodeSize*2.0f);
	}
		

	if(m_ToolMode == create_1waylink_tool || m_ToolMode == create_2waylinks_tool)
	{
		m_TempLinkStartPoint = point;
		m_TempLinkEndPoint = point;
		m_bMouseDownFlag = true;

		AfxGetApp()->LoadCursor(IDC_CREATE_LINK_CURSOR);

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
		SetGlobalViewParameters();

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

	if(m_ToolMode == create_1waylink_tool || m_ToolMode == create_2waylinks_tool)
	{
		m_bMouseDownFlag = false;
		CTLiteDoc* pDoc = GetDocument();

		CSize OffSet = m_TempLinkStartPoint - m_TempLinkEndPoint;

		if(abs(OffSet.cx) +  abs(OffSet.cy) <3)  // clicking on the same point, do not create links
			return;

		DTANode* pFromNode = 0;// create from node if there is no overlapping node
		float min_selection_distance = 20.0f;
		int FromNodeID = FindClosestNode(m_TempLinkStartPoint, min_selection_distance);
		if(FromNodeID ==-1)
		{
			pFromNode = pDoc->AddNewNode(SPtoNP(m_TempLinkStartPoint));
		}else
		{
			pFromNode = pDoc-> m_NodeIDMap[FromNodeID];
		}

		DTANode* pToNode = 0;// create from node if there is no overlapping node
		int ToNodeID = FindClosestNode(m_TempLinkEndPoint, min_selection_distance);
		if(ToNodeID ==-1)
		{
			pToNode = pDoc->AddNewNode(SPtoNP(m_TempLinkEndPoint));
		}else
		{
			pToNode = pDoc-> m_NodeIDMap[ToNodeID];
		}

		// create one way link

		if(m_ToolMode == create_1waylink_tool)
		{
			pDoc->AddNewLink(pFromNode->m_NodeID, pToNode->m_NodeID,false);
		}

		// create 2 way links with opposite direction
		if(m_ToolMode == create_2waylinks_tool)
		{
			pDoc->AddNewLink(pFromNode->m_NodeID, pToNode->m_NodeID,true);
			pDoc->AddNewLink(pToNode->m_NodeID,pFromNode->m_NodeID,true);

		}

		pDoc->m_bFitNetworkInitialized = true; // do not reflesh after adding links

	}

		if(m_ToolMode == create_node_tool)
	{
		CTLiteDoc* pDoc = GetDocument();

		DTANode* pFromNode = 0;// create from node if there is no overlapping node
		float min_selection_distance = 20.0f;
		int FromNodeID = FindClosestNode(point, min_selection_distance);
		if(FromNodeID ==-1)
		{
			pFromNode = pDoc->AddNewNode(SPtoNP(point));
		}

		pDoc->m_bFitNetworkInitialized = true; // do not reflesh after adding links

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
			SetGlobalViewParameters();
		
			Invalidate();

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
		Invalidate();

	}
	if(m_ToolMode == create_1waylink_tool || m_ToolMode == create_2waylinks_tool)
	{
		if(m_bMouseDownFlag)
		{
			// if it is the first moving operation, erase the previous temporal link
			if(m_TempLinkStartPoint!=m_TempLinkEndPoint)
				DrawTemporalLink(m_TempLinkStartPoint,m_TempLinkEndPoint);

			// update m_TempLinkEndPoint from the current mouse point
			m_TempLinkEndPoint = point;

			// draw a new temporal link
			DrawTemporalLink(m_TempLinkStartPoint,m_TempLinkEndPoint);
		}


		AfxGetApp()->LoadCursor(IDC_CREATE_LINK_CURSOR);

	}




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
	Invalidate();
}

void CTLiteView::OnNodeOrigin()
{
		CTLiteDoc* pDoc = GetDocument();

	pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	pDoc->m_OriginNodeID = pDoc->m_SelectedNodeID;
	pDoc->Routing();
	m_ShowAllPaths = true;
	Invalidate();

}

void CTLiteView::OnNodeDestination()
{


	CTLiteDoc* pDoc = GetDocument();
	pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	pDoc->m_DestinationNodeID = pDoc->m_SelectedNodeID;
	m_ShowAllPaths = true;
	pDoc->Routing();
}

void CTLiteView::OnContextMenu(CWnd* pWnd, CPoint point)
{

	CPoint MenuPoint = point;
	CTLiteDoc* pDoc = GetDocument();
	ScreenToClient(&point);

	m_CurrentMousePoint = point;

	//	CPoint select_pt = NPtoSP (pDoc->m_NodeIDMap[m_SelectedNodeID]->pt);

	//	CSize size = point-select_pt;
	//	if(pow((size.cx*size.cx + size.cy*size.cy),0.5) < m_NodeSize*2)
	{
		// In our client area - load the context menu
		CMenu cm;
		cm.LoadMenu(IDR_MENU1);

		CClientDC dc(this);
		// Put it up

		if(pDoc->m_SelectedLinkID>=0)  // link is selected 
		{
			// Get point in logical coordinates
			cm.GetSubMenu(1)->TrackPopupMenu(
				TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
				MenuPoint.x, MenuPoint.y, this);
		}
		else
		{
			// Get point in logical coordinates
			cm.GetSubMenu(0)->TrackPopupMenu(
				TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
				MenuPoint.x, MenuPoint.y, this);
		}
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
			pDoc->m_SelectedLinkID = (*iLink)->m_LinkID ;

			Min_distance = distance;
		}
	}		

	if(Min_distance > m_NodeSize*2)
	{
		pDoc->m_SelectedLinkID = -1;
		g_LinkDisplayList.clear ();
	}else
	{
		// if the control key is not pressed 
		if ( (nFlags & MK_CONTROL) ==false)
		{
			g_LinkDisplayList.clear ();
		}

	int bFoundFlag = false;
	for (iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++)
	{
		if((*iLink)->m_LinkID  == pDoc->m_SelectedLinkID)
		{ bFoundFlag = true; break;}

	}
		if(!bFoundFlag)
		g_LinkDisplayList.push_back(pDoc->m_LinkIDMap[pDoc->m_SelectedLinkID]);
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

}
void CTLiteView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
		CTLiteDoc* pDoc = GetDocument();

	if(!pDoc->m_StaticAssignmentMode || m_ToolMode == select_tool)
	{
	// create MOE Dlg when double clicking
	if(g_LinkMOEDlg==NULL)
	{
		g_LinkMOEDlg = new CDlgMOE();
		g_LinkMOEDlg->m_pDoc = pDoc;
		g_LinkMOEDlg->Create(IDD_DIALOG_MOE);

	}

	OnClickLink(nFlags, point);

	if(g_LinkMOEDlg  && g_LinkMOEDlg ->GetSafeHwnd ())
	{
		g_LinkMOEDlg->Invalidate (true);
	}

		OnViewShowmoe();
	}

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
			pDoc->m_SelectedLinkID = pLink->m_LinkID ;
			pDoc->m_SelectedNodeID = -1;

			m_SelectFromNodeNumber = dlg.m_FromNodeNumber;
			m_SelectToNodeNumber = dlg.m_ToNodeNumber;
			Invalidate();

		}else
		{

		CString str;
		str.Format ("Link %d->%d cannot be found.",dlg.m_FromNodeNumber ,dlg.m_ToNodeNumber);
		AfxMessageBox(str);
		}
	}

}

void CTLiteView::OnEditChangebackgroupimagethroughmouse()
{
	if(m_ToolMode!= bkimage_tool)
		m_ToolMode = bkimage_tool;
	else
		m_ToolMode = move_tool;
	
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

void  CTLiteView::DrawTemporalLink(CPoint start_point, CPoint end_point)
{
	CClientDC dc(this);
	OnPrepareDC(&dc);

	dc.SelectObject(&g_TempLinkPen);
	// Select drawing mode

	int oldROP2 = dc.SetROP2(R2_XORPEN);

	// Draw the line
	dc.MoveTo(start_point);
	dc.LineTo(end_point);

	// Reset drawing mode
	dc.SetROP2(oldROP2);

}

void CTLiteView::OnEditCreate1waylink()
{
	if(m_ToolMode != create_1waylink_tool)
		m_ToolMode = create_1waylink_tool;
	else   //reset to default selection tool
		m_ToolMode = select_tool;

}

void CTLiteView::OnEditCreate2waylinks()
{
	if(m_ToolMode != create_2waylinks_tool)
		m_ToolMode = create_2waylinks_tool;
	else   //reset to default selection tool
		m_ToolMode = select_tool;

}

void CTLiteView::OnUpdateEditCreate1waylink(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == create_1waylink_tool);

}

void CTLiteView::OnUpdateEditCreate2waylinks(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == create_2waylinks_tool);
}

void CTLiteView::OnShowGrid()
{
	m_bShowGrid = !m_bShowGrid;
	Invalidate();
}

void CTLiteView::OnUpdateShowGrid(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowGrid);
}

void CTLiteView::OnShowLinkarrow()
{
	m_bShowLinkArrow = !m_bShowLinkArrow;
	Invalidate();
}

void CTLiteView::OnUpdateShowLinkarrow(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowLinkArrow);
}

void CTLiteView::OnViewShowmoe()
{
	CTLiteDoc* pDoc = GetDocument();

	if(g_LinkDisplayList.size() > 0 && g_LinkMOEDlg!=NULL)  // if g_LinkMOEDlg is open, update
	{
		g_LinkMOEDlg->Invalidate ();
		g_LinkMOEDlg->ShowWindow(SW_SHOW);

	}
}

void CTLiteView::OnUpdateViewShowmoe(CCmdUI *pCmdUI)
{

}

void CTLiteView::OnImageLockbackgroundimageposition()
{
	m_ToolMode = move_tool;  // swtich m_ToolMode from bkimage_tool to other tool
}

void CTLiteView::OnViewTextlabel()
{
	m_bShowText = !m_bShowText;
	Invalidate();

}

void CTLiteView::OnUpdateViewTextlabel(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowText);
}


void CTLiteView::OnLinkDelete()
{

	CTLiteDoc* pDoc = GetDocument();
	if(pDoc->m_SelectedLinkID == -1)
	{
	AfxMessageBox("Please select a link first.");
	return;
	}
	pDoc->DeleteLink(pDoc->m_SelectedLinkID);
	pDoc->m_SelectedLinkID = -1;
	Invalidate();
}

void CTLiteView::OnUpdateEditDeleteselectedlink(CCmdUI *pCmdUI)
{
	pCmdUI->Enable (GetDocument()->m_SelectedLinkID>=0);
}

void CTLiteView::OnLinkEditlink()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_SelectedLinkID==-1)
	{
	AfxMessageBox("Please select a link first.");
	return;
	}

	DTALink* pLink= pDoc->m_LinkIDMap [pDoc->m_SelectedLinkID];
	if(pLink!=NULL)
	{
		CDlgLinkProperties dlg;
		dlg.FromNode = pLink->m_FromNodeNumber ;
		dlg.ToNode = pLink->m_ToNodeNumber ;
		dlg.LinkLength = pLink->m_Length ;
		dlg.SpeedLimit = pLink->m_SpeedLimit ;
		dlg.FreeFlowTravelTime = pLink->m_FreeFlowTravelTime ;
		dlg.LaneCapacity  = pLink->m_LaneCapacity ;
		dlg.nLane = pLink->m_NumLanes ;
		dlg.LinkType = pLink->m_link_type;

		dlg.DefaultSpeedLimit = pDoc->m_DefaultSpeedLimit ;
		dlg.DefaultCapacity = pDoc->m_DefaultCapacity ;
		dlg.DefaultnLane = pDoc->m_DefaultNumLanes;
		dlg.DefaultLinkType = pDoc->m_DefaultLinkType;

		if(dlg.DoModal() == IDOK)
		{
			pLink->m_Length = dlg.LinkLength;
			pLink->m_SpeedLimit = dlg.SpeedLimit;
			pLink->m_FreeFlowTravelTime = pLink->m_Length /pLink->m_SpeedLimit*60.0f; 

			pLink->m_LaneCapacity  = dlg.LaneCapacity;
			pLink->m_NumLanes  = dlg.nLane;
			pLink->m_link_type = dlg.LinkType ;

			pDoc->m_DefaultSpeedLimit = dlg.DefaultSpeedLimit;
			pDoc->m_DefaultCapacity = dlg.DefaultCapacity;
			pDoc->m_DefaultNumLanes = dlg.DefaultnLane;
			pDoc->m_DefaultLinkType = dlg.DefaultLinkType;

		}
		Invalidate();
	}
	
}


void CTLiteView::OnEditCreatenode()
{
	if(m_ToolMode != create_node_tool)
	{
		m_ToolMode = create_node_tool;
		m_bShowNode = true;
		m_bShowNodeNumber = true;

	}
	else   //reset to default selection tool
		m_ToolMode = select_tool;
}

void CTLiteView::OnUpdateEditCreatenode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == create_node_tool ? 1 : 0);
}

void CTLiteView::OnEditDeleteselectednode()
{
	CTLiteDoc* pDoc = GetDocument();
	if(pDoc->m_SelectedNodeID == -1)
	{
	AfxMessageBox("Please select a node first.");
	return;
	}
	if(pDoc->DeleteNode(pDoc->m_SelectedNodeID) ==false)
	{
	AfxMessageBox("Only an isolated node can be deleted. Please delete adjacent links first.");
	return;
	}
	pDoc->m_SelectedNodeID = -1;
	Invalidate();

}

void CTLiteView::OnEditSelectnode()
{
	// TODO: Add your command handler code here
}

void CTLiteView::OnViewSelectNode()
{
	m_ToolMode = select_node_tool;
}

void CTLiteView::OnUpdateViewSelectNode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == select_node_tool ? 1 : 0);
}

void CTLiteView::OnSearchNode()
{
	CDlgFindANode dlg;

	if(dlg.DoModal () == IDOK)
	{
		CTLiteDoc* pDoc = GetDocument();

		DTANode* pNode = pDoc->FindNodeWithNodeNumber (dlg.m_NodeNumber);

		if(pNode !=NULL)
		{
			pDoc->m_SelectedLinkID = -1;
			pDoc->m_SelectedNodeID = pNode->m_NodeID ;

			m_bShowNode = true;
			m_bShowNodeNumber = false;

			Invalidate();

		}else
		{

		CString str;
		str.Format ("Node %d cannot be found.",dlg.m_NodeNumber);
		AfxMessageBox(str);
		}
	}
}
