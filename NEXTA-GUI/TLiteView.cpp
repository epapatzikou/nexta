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
#include "MainFrm.h"
#include "DlgMOE.h"
#include "Network.h"
#include "DlgFindALink.h"
#include "DlgPathMOE.h"
#include "GLView.h"
#include "DlgLinkProperties.h"
#include "Dlg_GoogleFusionTable.h"
#include "Dlg_VehicleClassification.h"
#include "Dlg_TravelTimeReliability.h"
#include "Page_Node_Movement.h"
#include "Page_Node_Phase.h"
#include "Page_Node_LaneTurn.h"
#include "MyPropertySheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GDPoint g_Origin;
float g_Resolution;

extern std::list<int>	g_LinkDisplayList;

extern COLORREF g_MOEDisplayColor[MAX_MOE_DISPLAYCOLOR];
extern float g_Simulation_Time_Stamp;
extern bool g_LinkMOEDlgShowFlag;
int g_ViewID = 0;
IMPLEMENT_DYNCREATE(CTLiteView, CView)

BEGIN_MESSAGE_MAP(CTLiteView, CView)

	ON_WM_SIZE()
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
	ON_COMMAND(ID_EDIT_CREATESUBAREA, &CTLiteView::OnEditCreatesubarea)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CREATESUBAREA, &CTLiteView::OnUpdateEditCreatesubarea)
	ON_COMMAND(ID_TOOLS_REMOVENODESANDLINKSOUTSIDESUBAREA, &CTLiteView::OnToolsRemovenodesandlinksoutsidesubarea)
	ON_COMMAND(ID_VIEW_SHOW, &CTLiteView::OnViewShowAVISensor)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW, &CTLiteView::OnUpdateViewShowAVISensor)
	ON_COMMAND(ID_FILE_DATAEXCHANGEWITHGOOGLEFUSIONTABLES, &CTLiteView::OnFileDataexchangewithgooglefusiontables)
	ON_COMMAND(ID_EDIT_DELETELINKSOUTSIDESUBAREA, &CTLiteView::OnEditDeletelinksoutsidesubarea)
	ON_COMMAND(ID_EDIT_MOVENETWORKCOORDINATES, &CTLiteView::OnEditMovenetworkcoordinates)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVENETWORKCOORDINATES, &CTLiteView::OnUpdateEditMovenetworkcoordinates)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_VIEW_INCREASENODESIZE, &CTLiteView::OnViewIncreasenodesize)
	ON_COMMAND(ID_VIEW_DECREATENODESIZE, &CTLiteView::OnViewDecreatenodesize)
	ON_COMMAND(ID_NODE_CHECKCONNECTIVITYFROMHERE, &CTLiteView::OnNodeCheckconnectivityfromhere)
	ON_COMMAND(ID_NODE_DIRECTIONTOHEREANDVEHICLEANALAYSIS, &CTLiteView::OnNodeDirectiontohereandvehicleanalaysis)
	ON_COMMAND(ID_NODE_DIRECTIONFROMHEREANDVEHICLEANALASIS, &CTLiteView::OnNodeDirectionfromhereandvehicleanalasis)
	ON_COMMAND(ID_NODE_DIRECTIONTOHEREANDRELIABILITYANALYSIS, &CTLiteView::OnNodeDirectiontohereandreliabilityanalysis)
	ON_COMMAND(ID_LINK_INCREASEBANDWIDTH, &CTLiteView::OnLinkIncreasebandwidth)
	ON_COMMAND(ID_LINK_DECREASEBANDWIDTH, &CTLiteView::OnLinkDecreasebandwidth)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SENSOR, &CTLiteView::OnUpdateViewSensor)
	ON_COMMAND(ID_LINK_SWICHTOLINE_BANDWIDTH_MODE, &CTLiteView::OnLinkSwichtolineBandwidthMode)
	ON_COMMAND(ID_VIEW_TRANSITLAYER, &CTLiteView::OnViewTransitlayer)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TRANSITLAYER, &CTLiteView::OnUpdateViewTransitlayer)
	ON_COMMAND(ID_NODE_MOVEMENTPROPERTIES, &CTLiteView::OnNodeMovementproperties)
	ON_COMMAND(ID_LINK_LINEDISPLAYMODE, &CTLiteView::OnLinkLinedisplaymode)
	ON_UPDATE_COMMAND_UI(ID_LINK_LINEDISPLAYMODE, &CTLiteView::OnUpdateLinkLinedisplaymode)
	ON_COMMAND(ID_VIEW_ZONEBOUNDARY, &CTLiteView::OnViewZoneboundary)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZONEBOUNDARY, &CTLiteView::OnUpdateViewZoneboundary)
	ON_COMMAND(ID_VIEW_SHOW_CONNECTOR, &CTLiteView::OnViewShowConnector)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_CONNECTOR, &CTLiteView::OnUpdateViewShowConnector)
	ON_COMMAND(ID_VIEW_HIGHLIGHTCENTROIDSANDACTIVITYLOCATIONS, &CTLiteView::OnViewHighlightcentroidsandactivitylocations)
	ON_UPDATE_COMMAND_UI(ID_VIEW_HIGHLIGHTCENTROIDSANDACTIVITYLOCATIONS, &CTLiteView::OnUpdateViewHighlightcentroidsandactivitylocations)
	ON_COMMAND(ID_VIEW_BACKGROUNDCOLOR, &CTLiteView::OnViewBackgroundcolor)
	ON_COMMAND(ID_ACTIVITYLOCATIONMODE_NOLANDUSEACTIVITY, &CTLiteView::OnActivitylocationmodeNolanduseactivity)
	ON_UPDATE_COMMAND_UI(ID_ACTIVITYLOCATIONMODE_NOLANDUSEACTIVITY, &CTLiteView::OnUpdateActivitylocationmodeNolanduseactivity)
	ON_COMMAND(ID_ACTIVITYLOCATIONMODE_LANDUSEACTIVITY, &CTLiteView::OnActivitylocationmodeLanduseactivity)
	ON_UPDATE_COMMAND_UI(ID_ACTIVITYLOCATIONMODE_LANDUSEACTIVITY, &CTLiteView::OnUpdateActivitylocationmodeLanduseactivity)
	ON_COMMAND(ID_ACTIVITYLOCATIONMODE_EXTERNALORIGIN, &CTLiteView::OnActivitylocationmodeExternalorigin)
	ON_UPDATE_COMMAND_UI(ID_ACTIVITYLOCATIONMODE_EXTERNALORIGIN, &CTLiteView::OnUpdateActivitylocationmodeExternalorigin)
	ON_COMMAND(ID_ACTIVITYLOCATIONMODE_EXTERNALDESTINATION, &CTLiteView::OnActivitylocationmodeExternaldestination)
	ON_UPDATE_COMMAND_UI(ID_ACTIVITYLOCATIONMODE_EXTERNALDESTINATION, &CTLiteView::OnUpdateActivitylocationmodeExternaldestination)
	ON_COMMAND(ID_MOE_ODDEMAND, &CTLiteView::OnMoeOddemand)
	ON_UPDATE_COMMAND_UI(ID_MOE_ODDEMAND, &CTLiteView::OnUpdateMoeOddemand)
	ON_UPDATE_COMMAND_UI(ID_LINK_INCREASEBANDWIDTH, &CTLiteView::OnUpdateLinkIncreasebandwidth)
	ON_UPDATE_COMMAND_UI(ID_LINK_DECREASEBANDWIDTH, &CTLiteView::OnUpdateLinkDecreasebandwidth)
	ON_COMMAND(ID_EXPORT_CREATEVISSIMFILES, &CTLiteView::OnExportCreatevissimfiles)
	END_MESSAGE_MAP()

// CTLiteView construction/destruction
// CTLiteView construction/destruction

CBrush g_BlackBrush(RGB(10,10,10));
CPen g_BlackPen(PS_SOLID,1,RGB(0,0,0));
CPen g_TransitPen(PS_SOLID,1,RGB(0,255,255));

CPen g_LaneMarkingPen(PS_DASH,0,RGB(255,255,255));

CPen g_PenSelectColor(PS_SOLID,2,RGB(255,0,0));

CPen g_PenExternalDColor(PS_SOLID,2,RGB(173,255,047)); 
CPen g_PenExternalOColor(PS_SOLID,2,RGB(255,165,0));

CPen g_PenODColor(PS_SOLID,2,RGB(255,255,0));
CPen g_PenSelectPath(PS_SOLID,2,RGB(255,255,0));

CPen g_PenSignalColor(PS_SOLID,2,RGB(255,255,255));

CPen g_PenDisplayColor(PS_SOLID,2,RGB(255,255,0));
CPen g_PenNodeColor(PS_SOLID,1,RGB(0,0,0));

CPen g_PenConnectorColor(PS_DASH,1,RGB(0,0,255));
CPen g_PenCentroidColor(PS_SOLID,1,RGB(0,255,255));

CPen g_PenSignalNodeColor(PS_SOLID,1,RGB(255,255,0));
CPen g_PenQueueColor(PS_SOLID,3,RGB(255,0,0));

CPen g_PenQueueBandColor(PS_SOLID,1,RGB(255,0,0));
CBrush g_BrushQueueBandColor(RGB(255,0,0));

CPen g_PenCrashColor(PS_SOLID,1,RGB(255,0,0));
CBrush  g_BrushCrash(HS_VERTICAL,RGB(255,0,255)); //green
CBrush  g_TransitCrash(HS_CROSS,RGB(255,0,255)); //green

CPen g_PenSensorColor(PS_SOLID,0,RGB(0,255,0));
CBrush g_BrushSensor(RGB(0,255,0));
CPen g_PenNotMatchedSensorColor(PS_SOLID,1,RGB(255,255,255));

CPen g_PenSelectColor0(PS_SOLID,1,RGB(255,0,0));  // red
CPen g_PenSelectColor1(PS_SOLID,1,RGB(0,255,0));  // green
CPen g_PenSelectColor2(PS_SOLID,1,RGB(255,0,255)); //magenta
CPen g_PenSelectColor3(PS_SOLID,1,RGB(0,255,255));   // cyan
CPen g_PenSelectColor4(PS_SOLID,1,RGB(0,0,255));  // blue
CPen g_PenSelectColor5(PS_SOLID,1,RGB(255,255,0)); // yellow

CPen g_Pen2SelectColor0(PS_DOT ,2,RGB(0,255,0));  // green
CPen g_Pen2SelectColor1(PS_DOT ,2,RGB(255,0,0));  // red
CPen g_Pen2SelectColor2(PS_DOT ,2,RGB(255,0,255)); //magenta
CPen g_Pen2SelectColor3(PS_DOT ,2,RGB(0,255,255));   // cyan
CPen g_Pen2SelectColor4(PS_DOT ,2,RGB(0,0,255));  // blue
CPen g_Pen2SelectColor5(PS_DOT ,2,RGB(255,255,0)); // yellow

//observation

CPen g_PenSelectColor0_obs(PS_SOLID,4,RGB(255,0,0));  // red
CPen g_PenSelectColor1_obs(PS_SOLID,4,RGB(0,255,0));  // green
CPen g_PenSelectColor2_obs(PS_SOLID,4,RGB(255,0,255)); //magenta
CPen g_PenSelectColor3_obs(PS_SOLID,4,RGB(0,255,255));   // cyan
CPen g_PenSelectColor4_obs(PS_SOLID,4,RGB(0,0,255));  // blue
CPen g_PenSelectColor5_obs(PS_SOLID,4,RGB(255,255,0)); // yellow

CBrush  g_BrushColor0(HS_BDIAGONAL, RGB(255,0,0));  // red
CBrush  g_BrushColor1(HS_FDIAGONAL,RGB(0,255,0));  // green
CBrush  g_BrushColor2(HS_VERTICAL,RGB(255,0,255)); //magenta
CBrush  g_BrushColor3(HS_HORIZONTAL,RGB(0,255,255));   // cyan
CBrush  g_BrushColor4(HS_CROSS,RGB(0,0,255));  // blue
CBrush  g_BrushColor5(HS_DIAGCROSS,RGB(255,255,0)); // yellow

CPen g_PenHighlightPath(PS_SOLID,3,RGB(255,255,0));  // yellow
CBrush  g_BrushHighlightPath(RGB(255,255,0));  // yellow

CBrush  g_BrushLinkBand(RGB(125,125,0)); // 
CBrush  g_BrushLinkBandVolume(RGB(0,0,255)); // 

CBrush  g_BrushLinkReference(HS_CROSS, RGB(255,0,255)); //magenta
CPen    g_PenLinkReference(PS_SOLID,2,RGB(255,0,255));  //magenta

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


CPen g_TempLinkPen(PS_DASH,0,RGB(255,255,255));
CPen g_AVILinkPen(PS_DASH,0,RGB(0,255,0));
CPen g_SubareaLinkPen(PS_SOLID,0,RGB(255,255,0));
CPen g_SubareaPen(PS_DASH,2,RGB(255,0,0));

CPen g_GridPen(PS_SOLID,1,RGB(190,190,190));

CPen g_PenVehicle(PS_SOLID,1,RGB(0,255,0));  // yellow
CPen g_BrushVehicle(PS_SOLID,1,RGB(0,255,0)); //magenta
CPen g_BrushTransitUser(PS_SOLID,1,RGB(255,0,0)); //magenta

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
void g_SelectColorCode2(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{

	case 0: pDC->SelectObject(&g_Pen2SelectColor0); break;
	case 1: pDC->SelectObject(&g_Pen2SelectColor1); break;
	case 2: pDC->SelectObject(&g_Pen2SelectColor2); break;
	case 3: pDC->SelectObject(&g_Pen2SelectColor3); break;
	case 4: pDC->SelectObject(&g_Pen2SelectColor4); break;
	case 5: pDC->SelectObject(&g_Pen2SelectColor5); break;
	default:
		pDC->SelectObject(&g_Pen2SelectColor5);
	}

}

void g_SelectThickColorCode(CDC* pDC, int ColorCount)
{
	switch(ColorCount)
	{

	case 0: pDC->SelectObject(&g_PenSelectColor0_obs); break;
	case 1: pDC->SelectObject(&g_PenSelectColor1_obs); break;
	case 2: pDC->SelectObject(&g_PenSelectColor2_obs); break;
	case 3: pDC->SelectObject(&g_PenSelectColor3_obs); break;
	case 4: pDC->SelectObject(&g_PenSelectColor4_obs); break;
	case 5: pDC->SelectObject(&g_PenSelectColor5_obs); break;
	default:
		pDC->SelectObject(&g_PenSelectColor5_obs);
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
	m_msStatus = 0;

	if(theApp.m_VisulizationTemplate == e_train_scheduling) 
	m_bLineDisplayConditionalMode = true;

	if(theApp.m_VisulizationTemplate == e_traffic_assignment) 
		m_bLineDisplayConditionalMode = false;

	m_link_display_mode = link_display_mode_band; // 
	m_NodeTypeFaceName      = "Arial";

	m_bShowAVISensor = true;
	m_bShowZoneBoundary = true;
	m_bShowODDemandVolume = false;
	m_bShowConnector = false;
	m_bHighlightActivityLocation = false;
	isCreatingSubarea = false;
	isFinishSubarea = false;	
	m_ViewID = g_ViewID++;
	m_Resolution = 1;
	
	m_bShowText = false;

	m_bMouseDownFlag = false;
	m_ShowAllPaths = true;
	m_OriginOnBottomFlag = -1;

	//	RGB(102,204,204);

	m_ToolMode = move_tool;
	m_bMoveDisplay = false;
	m_bMoveImage = false;
	m_bMoveNetwork = false;
	m_bShowImage = true;
	m_bShowGrid  = false;
	m_bShowLinkArrow = false;
	m_bShowNode = true;
	m_bShowNodeNumber = true;
	m_bShowLinkType  = true;

	m_Origin.x = 0;
	m_Origin.y = 0;

	m_VehicleSize = 1;
	m_bShowSensor = true;
	m_bShowTransit = false;

	m_SelectFromNodeNumber = 0;
	m_SelectToNodeNumber = 0;
	g_ViewList.push_back(this);

}
CTLiteView::~CTLiteView()
{
	std::list<CTLiteView*>::iterator iView = g_ViewList.begin ();
	while (iView != g_ViewList.end())
	{
		if((*iView) == this)
		{
			g_ViewList.erase (iView);  // remove the Viewument to be deleted
			break;

		}
		iView++;
	}

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
//	FollowGlobalViewParameters(this->m_ViewID);


	CRect rectClient(0,0,0,0);
	GetClientRect(&rectClient);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rectClient.Width(),
		rectClient.Height());
	memDC.SelectObject(&bmp);


	// Custom draw on top of memDC
	CTLiteDoc* pDoc = GetDocument();

	CBrush brush;
	
	if (!brush.CreateSolidBrush(pDoc->m_BackgroundColor))
		return;

	brush.UnrealizeObject();
	memDC.FillRect(rectClient, &brush);

	m_OriginOnBottomFlag = pDoc->m_OriginOnBottomFlag;

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

	// step 1: set the objects in the network to the screen 

	std::list<DTANode*>::iterator iNode;

	if(!pDoc->m_bFitNetworkInitialized  )
	{
		FitNetworkToScreen();
		pDoc->m_bFitNetworkInitialized = true;

	}

	pDC->SetBkMode(TRANSPARENT);

	CRect ScreenRect;
	GetClientRect(ScreenRect);

	// step 2: draw grids
	if(m_bShowGrid)
	{
		pDC->SelectObject(&g_GridPen);
		pDC->SetTextColor(RGB(255,228,181));


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

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	// step 2: select font and color for node drawing, and compute the bandwidth for links
	CFont node_font;  // local font for nodes. dynamically created. it is effective only inside this function. if you want to pass this font to the other function, we need to pass the corresponding font pointer (which has a lot of communication overheads)
	int node_size = min(20,max(2,int(pDoc->m_NodeDisplaySize*pDoc->m_UnitFeet*m_Resolution)));

	int NodeTypeSize = pDoc->m_NodeTextDisplayRatio;
	int nFontSize =  max(node_size * NodeTypeSize, 10);

	m_LinkTextFontSize = min(node_size * NodeTypeSize, 15);

	node_font.CreatePointFont(nFontSize, m_NodeTypeFaceName);

	pDC->SelectObject(&node_font);

	COLORREF oldColor = pDC->SetTextColor(RGB(255,255,0));

	UINT oldAlign = pDC->SetTextAlign(TA_CENTER);

	// Text height
	TEXTMETRIC tm;
	memset(&tm, 0, sizeof TEXTMETRIC);
	pDC->GetOutputTextMetrics(&tm);

	//test the display width of a lane, if greather than 1, then band display mode

	if(pMainFrame->m_bShowLayerMap[layer_link] == true)
	{

	if(m_bLineDisplayConditionalMode)  // special condition
	{
		m_link_display_mode = link_display_mode_line;

	}else
	{
		if(pDoc->m_LinkMOEMode == MOE_none || pDoc->m_LaneWidthInFeet * pDoc->m_UnitFeet*m_Resolution * 5 > 0.1f) // 5 lanes
			m_link_display_mode = link_display_mode_band;
		else 
			m_link_display_mode = link_display_mode_line;
	}


	// draw links
	std::list<DTALink*>::iterator iLink;
	std::list<DTALink*>::iterator iLinkDisplay;

	CPoint FromPoint,ToPoint;
	CRect link_rect;


	CPen pen_moe[MAX_LOS_SIZE];
	CBrush brush_moe[MAX_LOS_SIZE];

	for(int los = 0; los < MAX_LOS_SIZE-1; los++)
	{
		pen_moe[los].CreatePen (PS_SOLID, 1, pDoc->m_colorLOS[los]);
		brush_moe[los].CreateSolidBrush (pDoc->m_colorLOS[los]);
	}

	CPen pen_freeway, pen_ramp, pen_arterial, pen_connector,pen_transit,pen_walking;
	CBrush brush_freeway, brush_ramp, brush_arterial, brush_connector,brush_walking;

	pen_freeway.CreatePen (PS_SOLID, 1, pDoc->m_FreewayColor);
	brush_freeway.CreateSolidBrush (pDoc->m_FreewayColor);

	pen_ramp.CreatePen (PS_SOLID, 1, pDoc->m_RampColor);
	brush_ramp.CreateSolidBrush (pDoc->m_RampColor);

	pen_arterial.CreatePen (PS_SOLID, 1, pDoc->m_ArterialColor);
	brush_arterial.CreateSolidBrush (pDoc->m_ArterialColor);

	pen_connector.CreatePen (PS_SOLID, 1, pDoc->m_ConnectorColor);
	brush_connector.CreateSolidBrush (pDoc->m_ConnectorColor);

	pen_transit.CreatePen (PS_SOLID, 1, pDoc->m_TransitColor);

	pen_walking.CreatePen (PS_SOLID, 1, pDoc->m_WalkingColor);
	brush_walking.CreateSolidBrush (pDoc->m_WalkingColor);

	// recongenerate the lind band width offset only when chaning display mode or on volume mode
	if(	pDoc -> m_PrevLinkMOEMode != pDoc -> m_LinkMOEMode || 
		pDoc->m_LinkMOEMode == MOE_volume || 
		pDoc->m_LinkMOEMode == MOE_speed ||
		pDoc->m_LinkMOEMode == MOE_emissions ||
		pDoc->m_LinkMOEMode == MOE_safety) 
	{
		pDoc->GenerateOffsetLinkBand();
	}

	if (pDoc->m_LinkMOEMode == MOE_volume || pDoc->m_LinkMOEMode == MOE_speed || pDoc->m_LinkMOEMode == MOE_emissions)   // under volume mode, dynamically generate volume band
	{
		pDoc->m_LinkBandWidthMode = LBW_link_volume;
	}else
	{
		pDoc->m_LinkBandWidthMode = LBW_number_of_lanes;  // e.g. queue
	}

	if(pDoc->m_LinkSet.size() > 0)  // update for a network with links
	{
		pDoc -> m_PrevLinkMOEMode = pDoc -> m_LinkMOEMode;
	}

	// step 3: draw all links
	int min_x, min_y, max_x, max_y;

	pDC->SelectObject(&g_BrushLinkBand);   //default brush  , then MOE, then apply speical conditions for volume and vehicle mode

//	TRACE("connector layer: %d\n", pMainFrame->m_bShowLayerMap[layer_connector]);

	for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{

		if( !pMainFrame->m_bShowLayerMap[layer_connector] )
		{
 			if (pDoc->m_LinkTypeMap[(*iLink)->m_link_type ].IsConnector ())  //hide connectors
				continue; 
		}

		// step 1: decide if a link is included in the screen region
		FromPoint = NPtoSP((*iLink)->m_FromPoint);
		ToPoint = NPtoSP((*iLink)->m_ToPoint);

		min_x = min(FromPoint.x,ToPoint.x);
		max_x = max(FromPoint.x,ToPoint.x);

		min_y = min(FromPoint.y,ToPoint.y);
		max_y = max(FromPoint.y,ToPoint.y);

		link_rect.SetRect(min_x-50,min_y-50,max_x+50,max_y+50);

		if(RectIsInsideScreen(link_rect,ScreenRect) == false)  // not inside the screen boundary
			continue;


		// step 4: select color and brush for MOE mode

		//			continue;
		CPen LinkTypePen;


		// step 5: select color and brush according to link MOE
		float value = -1.0f ;

		if( pDoc->m_LinkMOEMode != MOE_none) 
		{

			// dynamically create LOE mean 
			float power = pDoc->GetLinkMOE((*iLink), pDoc->m_LinkMOEMode,(int)g_Simulation_Time_Stamp,1, value);
			int LOS = pDoc->GetLOSCode(power);

			//draw link as lines
			if(m_link_display_mode == link_display_mode_line  || m_link_display_mode == link_display_mode_lane_group)
			{
				pDC->SelectObject(&pen_moe[LOS]);
				pDC->SelectObject(&g_BrushLinkBand);   //default brush  , then MOE, then condition with volume
			}
			else 
			{// display link as a band so use black pen
				pDC->SelectObject(&g_BlackPen);
				pDC->SelectObject(&brush_moe[LOS]);
			}

		}
		else if(m_bShowLinkType)  // show link type
		{
			//		COLORREF link_color = pDoc->GetLinkTypeColor((*iLink)->m_link_type );
			//		LinkTypePen.CreatePen(PS_SOLID, 1, link_color);
			//		pDC->SelectObject(&LinkTypePen);

			if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsFreeway ())
			{
				pDC->SelectObject(&pen_freeway);
				pDC->SelectObject(&brush_freeway);
			}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsArterial())
			{
				pDC->SelectObject(&pen_arterial);
				pDC->SelectObject(&brush_arterial);
			}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsRamp())
			{
				pDC->SelectObject(&pen_ramp);
				pDC->SelectObject(&brush_ramp);
			}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type] .IsConnector())
			{
				pDC->SelectObject(&pen_connector);
				pDC->SelectObject(&brush_connector);
			}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsTransit())
			{
				pDC->SelectObject(&pen_transit);
				pDC->SelectObject(&g_TransitCrash);
			}else if ( pDoc->m_LinkTypeMap[(*iLink)->m_link_type].IsWalking())
			{
				pDC->SelectObject(&pen_walking);
				pDC->SelectObject(&brush_walking);
			}else
			{
				pDC->SelectObject(&pen_arterial);
				pDC->SelectObject(&g_BrushLinkBand);   //default brush
			}

			// special condition for subarea link
			if((*iLink)->m_LayerNo > 0)
				pDC->SelectObject(&g_SubareaLinkPen);

		}else  // default arterial model
		{
			pDC->SelectObject(&pen_arterial);
			pDC->SelectObject(&g_BrushLinkBand);   //default brush
		}

		//special condition 1: vehicle mode

		if( pDoc->m_LinkMOEMode == MOE_vehicle)  // when showing vehicles, use black
			pDC->SelectObject(&g_BlackPen);

		//special condition 2:  volume mode
		if (pDoc->m_LinkMOEMode == MOE_volume)   // under volume mode, dynamically generate volume band
			pDC->SelectObject(&g_BrushLinkBandVolume);   //default brush

		// special condition 3: when a link is selected

		//outside subarea
		if((*iLink)->m_DisplayLinkID>=0 )
		{
			g_SelectThickPenColor(pDC,(*iLink)->m_DisplayLinkID);
			pDC->SetTextColor(RGB(255,0,0));
		}else if  ((*iLink)->m_LinkNo == pDoc->m_SelectedLinkID)
		{
			g_SelectThickPenColor(pDC,0);
			pDC->SetTextColor(RGB(255,0,0));
		}else
			pDC->SetTextColor(RGB(255,228,181));

		//step 6: draw link as line or band/bar
		if(m_link_display_mode == link_display_mode_line )  
		{
			// calibration mode, do not show volume
			DrawLinkAsLine((*iLink),pDC);
		}
		else if (m_link_display_mode == link_display_mode_lane_group)
		{
			DrawLinkAsLaneGroup((*iLink),pDC);
		}else  // as lane group
		{
			if(DrawLinkAsBand((*iLink),pDC,false)==false)
				return;
		}

			CPoint ScenarioPoint = NPtoSP((*iLink)->GetRelativePosition(0.6));  // get relative position of a link 

			if((*iLink) ->GetImpactedFlag(g_Simulation_Time_Stamp)>=0.1 || (g_Simulation_Time_Stamp ==0 && (*iLink) ->CapacityReductionVector.size()>0))
				DrawBitmap(pDC, ScenarioPoint, IDB_INCIDENT);

			if((*iLink) ->GetMessageSign(g_Simulation_Time_Stamp)>=0.1 || (g_Simulation_Time_Stamp ==0 && (*iLink) ->MessageSignVector.size()>0))
				DrawBitmap(pDC, ScenarioPoint, IDB_VMS);

			if((*iLink) ->GetTollValue(g_Simulation_Time_Stamp)>=0.1 || (g_Simulation_Time_Stamp ==0 && (*iLink) ->TollVector.size()>0))
				DrawBitmap(pDC, ScenarioPoint, IDB_TOLL);



			if(pMainFrame->m_bShowLayerMap[layer_detector] == true  && (*iLink)->m_bSensorData)  // only during non display mode
			{
				if((*iLink)->m_LinkNo == pDoc->m_SelectedLinkID)
				{
					pDC->SelectObject(&g_PenSelectColor);
				}else
				{

					pDC->SelectObject(&g_PenSensorColor);
				}		
				CPoint midpoint = NPtoSP((*iLink)->GetRelativePosition(0.5));
				int size = 6;
				pDC->SelectObject(g_BrushSensor);
				pDC->Rectangle(midpoint.x-size, midpoint.y-size, midpoint.x+size, midpoint.y+size);

			}

		if (pDoc->m_LinkMOEMode == MOE_volume && (*iLink)->GetObsLinkVolumeCopy(g_Simulation_Time_Stamp) > 1)   // draw provided AADT
		{
			pDC->SelectObject(&g_BrushLinkReference);   //reference brush
			pDC->SelectObject(&g_PenLinkReference);   //reference pen
			DrawLinkAsBand((*iLink),pDC,true);  // true: second band as observation
		}

		//step 7: show link labels
		if( m_bShowText )
		{

			float sqr_value = (FromPoint.x - ToPoint.x)*(FromPoint.x - ToPoint.x) + (FromPoint.y - ToPoint.y)*(FromPoint.y - ToPoint.y);
			float screen_distance = sqrt( sqr_value) ;

			// create rotation font
			CFont link_text_font;

			LOGFONT lf;
			memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
			lf.lfHeight = m_LinkTextFontSize;        // request a 12-pixel-height font

			int last_shape_point_id = (*iLink) ->m_ShapePoints .size() -1;
			float DeltaX = (*iLink)->m_ShapePoints[last_shape_point_id].x - (*iLink)->m_ShapePoints[0].x;
			float DeltaY = (*iLink)->m_ShapePoints[last_shape_point_id].y - (*iLink)->m_ShapePoints[0].y;

			float theta = atan2(DeltaY, DeltaX);

			float theta_deg = fmod(theta/PI*180,360);

			if(theta_deg < -90)
				theta_deg += 180;

			if(theta_deg > 90)
				theta_deg -= 180;

			lf.lfEscapement = theta_deg*10;
			strcpy(lf.lfFaceName, "Arial");       

			link_text_font.CreateFontIndirect(&lf);

			pDC->SelectObject(&link_text_font);

			// select text string to be displayed

			bool with_text = false;
			CString str_text, str_reference_text;

			// show text condition 1: street name
			if(pDoc->m_LinkMOEMode == MOE_none && (*iLink)->m_Name.length () > 0 && (*iLink)->m_Name!="(null)"  && screen_distance > 100 )
			{
				str_text = (*iLink)->m_Name.c_str ();
				with_text = true;
			}

			// show text condition 2: crash rates
			if(pDoc->m_LinkMOEMode == MOE_safety && screen_distance > 50 && (*iLink)->m_NumberOfCrashes >= 0.0001)
			{
				str_text.Format ("%6.2f",(*iLink)->m_NumberOfCrashes );
				with_text = true;
			}

			// show text condition 3: other statistics
			if(screen_distance > 20 && (pDoc->m_LinkMOEMode == MOE_queue_length || pDoc->m_LinkMOEMode == MOE_speed || pDoc->m_LinkMOEMode == MOE_volume || pDoc->m_LinkMOEMode == MOE_vcratio || pDoc->m_LinkMOEMode == MOE_reliability))
			{

				pDoc->GetLinkMOE((*iLink), pDoc->m_LinkMOEMode,(int)g_Simulation_Time_Stamp,1, value);

				if(value >0.001)  // with value
				{

					if( pDoc->m_LinkMOEMode == MOE_vcratio)
						str_text.Format ("%.1f",value);
					else if(pDoc->m_LinkMOEMode == MOE_queue_length || pDoc->m_LinkMOEMode == MOE_reliability)
						str_text.Format ("%.0f%%",value*100);
					else
						str_text.Format ("%.0f",value);

					with_text = true;

					if(pDoc->m_bShowCalibrationResults && pDoc->m_LinkMOEMode == MOE_volume)  // special conditions with calibration mode
					{
						if((*iLink)->m_bSensorData )
						{
							str_text.Format ("%.0f",value);
							float percentage_error = (value- (*iLink)->m_ReferenceFlowVolume) / max(1,(*iLink)->m_ReferenceFlowVolume)*100;
							//									str_reference_text.Format ("%.0f, %.0f%%",(*iLink)->m_ReferenceFlowVolume,percentage_error);
							str_reference_text.Format ("%.0f",(*iLink)->m_ReferenceFlowVolume);

							with_text = true;

						}else
						{  // do not show data without sensors
							with_text = false; 
						}

					}

				}
				else
				{
					with_text = false;
				}

			}

			// after all the above 3 conditions, show test now. 
			if(with_text)
			{
				CPoint TextPoint = NPtoSP((*iLink)->GetRelativePosition(0.3));
				pDC->SetBkColor(RGB(0,0, 0));
				pDC->SetTextColor(RGB(0,0,0));
				pDC->TextOut(TextPoint.x,TextPoint.y, str_text);

				if(str_reference_text.GetLength () > 0 )  // reference text with different color4
				{
					pDC->SetTextColor(RGB(128, 255, 255));
					pDC->TextOut(TextPoint.x,TextPoint.y+m_LinkTextFontSize, str_reference_text);
				}

			}

			// step 8:  show location of scenario/incident /work zone/ toll: link-based objects
			pDC->SetBkColor(RGB(0, 0, 0));

			//************************************

			// step 9: draw sensor (based on a link)
		}
	}
	}

	if(pMainFrame->m_bShowLayerMap[layer_link_MOE] == true && (theApp.m_VisulizationTemplate == e_train_scheduling))
	{	std::list<DTALink*>::iterator iLink;

		for (iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
		{
		DrawLinkTimeTable((*iLink),pDC,0);
		}
	}


	pDC->SelectObject(&node_font);

	// step 10: draw shortest path
	int i;
	/*
	unsigned int iPath;

	if(m_ShowAllPaths)
	{
	for (iPath = 0; iPath < pDoc->m_PathDisplayList.size(); iPath++)
	{
	g_SelectThickPenColor(pDC,0);

	for (i=0 ; i<pDoc->m_PathDisplayList[iPath].m_LinkSize; i++)
	{
	DTALink* pLink = pDoc->m_LinkNoMap[pDoc->m_PathDisplayList[iPath].m_LinkVector[i]];
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

	*/
	//draw select path


	if(pDoc->m_PathDisplayList.size() > pDoc->m_SelectPathNo && pDoc->m_SelectPathNo!=-1)
	{

		pDC->SelectObject(&g_PenSelectPath);

		for (i=0 ; i<pDoc->m_PathDisplayList[pDoc->m_SelectPathNo].m_LinkSize; i++)
		{
			DTALink* pLink = pDoc->m_LinkNoMap[pDoc->m_PathDisplayList[pDoc->m_SelectPathNo].m_LinkVector[i]];
			if(pLink!=NULL)
			{
				DrawLinkAsLine(pLink,pDC);

			}
		}
	}

	// step 11: select font for origin and destination  of the shortest path

	// step 11: all all nodes

	if(pMainFrame->m_bShowLayerMap[layer_node] == true)
	{
	CPoint point;
	float feet_size;

	CFont od_font;
	int nODNodeSize = max(node_size,10);
	int nODFontSize =  max(nODNodeSize * NodeTypeSize, 10);

	od_font.CreatePointFont(nODFontSize, m_NodeTypeFaceName);

	for (iNode = pDoc->m_NodeSet.begin(); iNode != pDoc->m_NodeSet.end(); iNode++)
	{

		point = NPtoSP((*iNode)->pt);

		CRect node_rect;
		node_rect.SetRect(point.x-50,point.y-50,point.x+50,point.y+50);

		if(RectIsInsideScreen(node_rect,ScreenRect) == false)  // not inside the screen boundary
			continue;

		pDC->SelectObject(&g_BlackBrush);
		pDC->SetTextColor(RGB(255,255,0));
		pDC->SelectObject(&g_PenNodeColor);
		pDC->SetBkColor(RGB(0,0,0));

		if((*iNode)->m_NodeID == pDoc->m_SelectedNodeID)
		{
			pDC->SelectObject(&g_PenSelectColor);
		}else if((*iNode)->m_ZoneID > 0 && m_bHighlightActivityLocation)
		{
			pDC->SelectObject(&g_PenCentroidColor);

			if((*iNode)->m_External_OD_flag == 1)  // external origin 
			{

				pDC->SelectObject(&g_PenExternalOColor);
				pDC->SelectObject(&g_BlackBrush);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->SetBkColor(RGB(0,0,0));

			}

			if((*iNode)->m_External_OD_flag == -1)  // external destination
			{
				pDC->SelectObject(&g_PenExternalDColor);
				pDC->SelectObject(&g_BlackBrush);
				pDC->SetTextColor(RGB(255,0,0));
				pDC->SetBkColor(RGB(0,0,0));


			}

		}else
		{
			pDC->SelectObject(&g_PenNodeColor);
		}

		if((*iNode)->m_NodeID == pDoc->m_OriginNodeID)
		{

			CFont* oldFont = pDC->SelectObject(&od_font);
			pDC->SelectObject(&g_PenSelectColor);
			pDC->SelectObject(&g_BlackBrush);
			pDC->SetTextColor(RGB(255,0,0));
			pDC->SetBkColor(RGB(0,0,0));

			TEXTMETRIC tmOD;
			memset(&tmOD, 0, sizeof TEXTMETRIC);
			pDC->GetOutputTextMetrics(&tmOD);

			pDC->Rectangle (point.x - nODNodeSize, point.y + nODNodeSize,
				point.x + nODNodeSize, point.y - nODNodeSize);

			point.y -= tmOD.tmHeight / 2;
			pDC->TextOut(point.x , point.y , _T("A"));

			pDC->SelectObject(oldFont);  // restore font



		}else if((*iNode)->m_NodeID == pDoc->m_DestinationNodeID)
		{
			CFont* oldFont = pDC->SelectObject(&od_font);// these are local font, created inside the function, we do not want to create them in another sub functions to speed up the display efficiency.

			TEXTMETRIC tmOD;
			memset(&tmOD, 0, sizeof TEXTMETRIC);
			pDC->GetOutputTextMetrics(&tmOD);

			pDC->SelectObject(&g_PenSelectColor);
			pDC->SelectObject(&g_BlackBrush);
			pDC->SetTextColor(RGB(255,0,0));
			pDC->SetBkColor(RGB(0,0,0));

			pDC->Rectangle (point.x - nODNodeSize, point.y + nODNodeSize,
				point.x + nODNodeSize, point.y - nODNodeSize);

			point.y -= tmOD.tmHeight / 2;
			pDC->TextOut(point.x , point.y , _T("B"));

			pDC->SelectObject(oldFont);  // restore font


		}else
		{
			if(m_bShowNode)
			{

				feet_size = pDoc->m_UnitFeet*m_Resolution;

				/// starting drawing nodes in normal mode

				if((*iNode)->m_DistanceToRoot > MAX_SPLABEL-1)
				{
					pDC->SelectObject(&g_PenSelectColor2_obs);

					feet_size = max(2,feet_size);  // show node anyway
				}


				if(feet_size*pDoc->m_NodeDisplaySize > 0.2) // add or condition to show all nodes
				{
					DrawNode(pDC, (*iNode),point, node_size,tm);
				}

			}
		}
	}
	}
	//step 12: draw generic point layer , e.g. crashes

	if(pMainFrame->m_bShowLayerMap[layer_crash])
	{
		pDC->SelectObject(&g_BlackBrush);
		pDC->SetTextColor(RGB(255,255,0));
		pDC->SelectObject(&g_PenSelectColor5);
		pDC->SetBkColor(RGB(0,0,0));

		std::list<DTAPoint*>::iterator iCrash;
		for (iCrash = pDoc->m_DTAPointSet.begin(); iCrash != pDoc->m_DTAPointSet.end(); iCrash++)
		{
			CPoint point = NPtoSP((*iCrash)->pt);

			node_size = 3;

			/// starting drawing nodes in normal mode
			pDC->Ellipse(point.x - node_size, point.y + node_size,
				point.x + node_size, point.y - node_size);

		}
	}

	if(pMainFrame->m_bShowLayerMap[layer_GPS])
	{
		pDC->SelectObject(&g_BlackBrush);
		pDC->SetTextColor(RGB(255,255,0));
		pDC->SelectObject(&g_PenSelectColor5);
		pDC->SetBkColor(RGB(0,0,0));


		for (int i =0; i < pDoc->m_HighlightGDPointVector .size(); i++)
		{
			CPoint point = NPtoSP(pDoc->m_HighlightGDPointVector[i]);

			node_size = 3;

			/// starting drawing nodes in normal mode
			pDC->Ellipse(point.x - node_size, point.y + node_size,
				point.x + node_size, point.y - node_size);

		}
	}


	// draw generic link layer
	std::list<DTALine*>::iterator iLine;

	if(pDoc->m_DTALineSet.size() > 0)
	{
		pDC->SelectObject(&g_BlackBrush);
		pDC->SetTextColor(RGB(255,255,0));
		pDC->SelectObject(&g_PenCrashColor);
		pDC->SetBkColor(RGB(0,0,0));

		for (iLine = pDoc->m_DTALineSet.begin(); iLine != pDoc->m_DTALineSet.end(); iLine++)
		{

			for(unsigned int i = 0; i< (*iLine)->m_ShapePoints .size(); i++)
			{
				CPoint point = NPtoSP((*iLine)->m_ShapePoints[i]);
				if(i==0)
					pDC->MoveTo(point);
				else
					pDC->LineTo(point);

			}

		}

	}

	// step 13: draw zone layer

	CPen ZonePen;

// Create a solid red pen of width 2.
	ZonePen.CreatePen(PS_DASH, 1, pDoc->m_ZoneColor);

	if(pMainFrame->m_bShowLayerMap[layer_zone])
	{	
		pDC->SelectObject(&ZonePen);

		CFont zone_font;  // local font for nodes. dynamically created. it is effective only inside this function. if you want to pass this font to the other function, we need to pass the corresponding font pointer (which has a lot of communication overheads)
		zone_font.CreatePointFont(nFontSize*4, m_NodeTypeFaceName);

		pDC->SelectObject(&zone_font);

		std::map<int, DTAZone>	:: const_iterator itr;


		pDC->SetTextColor(pDoc->m_ZoneTextColor);


		for(itr = pDoc->m_ZoneMap.begin(); itr != pDoc->m_ZoneMap.end(); itr++)
		{
			int center_x = 0;
			int center_y = 0;

			if(itr->second.m_ShapePoints .size() > 0)
			{

				for(int i = 0; i< itr->second.m_ShapePoints .size(); i++)
				{

					CPoint point =  NPtoSP(itr->second.m_ShapePoints[i]);

					center_x += point.x;
					center_y += point.y;

					if(i == 0)
						pDC->MoveTo(point);
					else
						pDC->LineTo(point);

				}

				CPoint point_0 =  NPtoSP(itr->second.m_ShapePoints[0]);  // back to the starting point

				pDC->LineTo(point_0);

			center_x  = center_x/max(1,itr->second.m_ShapePoints .size());
			center_y  = center_y/max(1,itr->second.m_ShapePoints .size()) - tm.tmHeight;

			CString zone_id_str;
			zone_id_str.Format("%d", itr->second.m_ZoneTAZ) ;

			pDC->TextOut(center_x , center_y , zone_id_str);
			}

		}

	}	


	// step 13: draw subarea layer
	if(GetDocument()->m_SubareaShapePoints.size() > 0 && pMainFrame->m_bShowLayerMap[layer_subarea])
	{
		CPoint point_0  = NPtoSP(GetDocument()->m_SubareaShapePoints[0]);

		pDC->MoveTo(point_0);

		for (int sub_i= 0; sub_i < GetDocument()->m_SubareaShapePoints.size(); sub_i++)
		{
			CPoint point =  NPtoSP(GetDocument()->m_SubareaShapePoints[sub_i]);
			pDC->LineTo(point);

		}
		if(isFinishSubarea)
			pDC->LineTo(point_0);

	}

	// step 14: show not-matched sensors
	if(m_bShowSensor)
	{

		pDC->SelectObject(&g_PenNotMatchedSensorColor);

		std::map<string, DTA_sensor>::iterator iSensor;

		for (iSensor = pDoc->m_SensorMap.begin(); iSensor != pDoc->m_SensorMap.end(); iSensor++)
		{
			CPoint point = NPtoSP((*iSensor).second .pt);

			int sensor_size = 2;
			//if((*iSensor).second .LinkID 0)
			//{
			//	pDC->Ellipse(point.x - sensor_size, point.y + sensor_size,
			//		point.x + sensor_size, point.y - sensor_size);
			//}
		}
	}

	// step 15: show vehicles

	if( pDoc->m_LinkMOEMode == MOE_vehicle)
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

				if((*iVehicle)->m_PricingType ==4)  // transit user
					pDC->SelectObject(&g_BrushTransitUser); //red transit users
		
				float ratio = 0;
				int LinkID = pDoc->GetVehilePosition((*iVehicle), g_Simulation_Time_Stamp,ratio);

				DTALink* pLink = pDoc->m_LinkNoMap[LinkID];
				if(pLink!=NULL)
				{

					CPoint VehPoint= NPtoSP(pLink->GetRelativePosition(ratio));

					pDC->Ellipse (VehPoint.x - vehicle_size, VehPoint.y - vehicle_size,
						VehPoint.x + vehicle_size, VehPoint.y + vehicle_size);
				}

			}

		}
	}
	//////////////////////////////////////
	// step 16: draw OD demand

	if(pMainFrame->m_bShowLayerMap[layer_ODMatrix])
	{
		int i,j;

		float MaxODDemand = 500;
		std::map<int, DTAZone>	:: const_iterator itr_o;
		std::map<int, DTAZone>	:: const_iterator itr_d;

		//for(itr_o = pDoc->m_ZoneMap.begin(); itr_o != pDoc->m_ZoneMap.end(); itr_o++)
		//	for(itr_d = pDoc->m_ZoneMap.begin(); itr_d != pDoc->m_ZoneMap.end(); itr_d++)
		//	{
		//		float volume = pDoc->m_ZoneMap[itr_o->first].m_ODDemandMatrix [itr_d->first].GetSubTotalValue ();
		//		if(volume > MaxODDemand & (itr_o->first !=itr_d->first))
		//			MaxODDemand = volume;

		//	}

			for(itr_o = pDoc->m_ZoneMap.begin(); itr_o != pDoc->m_ZoneMap.end(); itr_o++)
				for(itr_d = pDoc->m_ZoneMap.begin(); itr_d != pDoc->m_ZoneMap.end(); itr_d++)
				{
					float volume = pDoc->m_ZoneMap[itr_o->first].m_ODDemandMatrix [itr_d->first].GetSubTotalValue ();
					if(volume>=10 && (itr_o->first !=itr_d->first))
					{
						CPoint FromPoint = NPtoSP(pDoc->m_ZoneMap[itr_o->first].GetCenter());
						CPoint ToPoint = NPtoSP(pDoc->m_ZoneMap[itr_d->first].GetCenter());

						CPen penmoe;
						float Width = volume/MaxODDemand*10;

						if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
							continue; 

						if(Width>=0.2)  //draw critical OD demand only
						{
							penmoe.CreatePen (PS_SOLID, (int)(Width), RGB(0,255,255));
							pDC->SelectObject(&penmoe);
							pDC->MoveTo(FromPoint);
							pDC->LineTo(ToPoint);
						}

					}

				}
	}


	if(pDoc->m_ZoneMap.find(pDoc->m_CriticalOriginZone)!= pDoc->m_ZoneMap.end() 
		&& pDoc->m_ZoneMap.find(pDoc->m_CriticalDestinationZone)!= pDoc->m_ZoneMap.end())
	{
			CPoint FromPoint = NPtoSP(pDoc->m_ZoneMap[pDoc->m_CriticalOriginZone].GetCenter());
			CPoint ToPoint = NPtoSP(pDoc->m_ZoneMap[pDoc->m_CriticalDestinationZone].GetCenter());

			CPen penmoe;
			float Width = 5;

			if(Width>=0.2)  //draw critical OD demand only
			{
				penmoe.CreatePen (PS_SOLID, (int)(Width), RGB(0,255,255));
				pDC->SelectObject(&penmoe);
				pDC->MoveTo(FromPoint);
				pDC->LineTo(ToPoint);
			}

	}


	// step 17: draw Public Transit Layer

	if(m_bShowTransit)
	{
		pDC->SelectObject(&g_BlackBrush);
		pDC->SetTextColor(RGB(255,255,0));
		pDC->SelectObject(&g_TransitPen);
		DrawPublicTransitLayer(pDC);
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

	double res_wid = ScreenRect.Width()/(pDoc->m_NetworkRect.Width()+0.000001);
	double res_height = ScreenRect.Height()/(pDoc->m_NetworkRect.Height()+0.000001);

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


BOOL CTLiteView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if(m_ToolMode != backgroundimage_tool && m_ToolMode !=  network_coordinate_tool)  //select, move
	{
		if(zDelta > 0)
		{
			m_Resolution*=1.5f;
		}
		else
		{
			m_Resolution/=1.5f;
		}
	}else
	{

		if(m_ToolMode == network_coordinate_tool)
		{

			int LayerNo = 0;


			// change image size
			CTLiteDoc* pDoc = GetDocument();
			float increase_ratio = 1.005f;
			float decrease_ratio = 1.0f / increase_ratio;

			if(zDelta > 0)
			{

				// control -> Y only
				// shift -> X only



				if(nFlags != MK_CONTROL)  // shift or nothing
				{
					pDoc->ChangeNetworkCoordinates(LayerNo,increase_ratio,1.0f,0.0f,0.0f);
				}

				if(nFlags != MK_SHIFT)  // control or nothing
				{
					pDoc->ChangeNetworkCoordinates(LayerNo,1.0f,increase_ratio,0.0f,0.0f);
				}
			}
			else
			{
				if(nFlags != MK_CONTROL)  // shift or nothing
				{
					pDoc->ChangeNetworkCoordinates(LayerNo,decrease_ratio,1.0f,0.0f,0.0f);
				}

				if(nFlags != MK_SHIFT)  // control or nothing
				{
					pDoc->ChangeNetworkCoordinates(LayerNo,1.0f,decrease_ratio,0.0f,0.0f);
				}
			}
		}


		if(m_ToolMode == backgroundimage_tool)
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

//	if(Min_distance > pDoc->m_NodeDisplaySize*pDoc->m_UnitFeet*m_Resolution*20)
//	{
//		SelectedNodeID = -1;
//	}
	return SelectedNodeID;
}

int CTLiteView::FindClosestZone(CPoint point, float Min_distance)
{
	CTLiteDoc* pDoc = GetDocument();
	if(pDoc !=NULL)
	{
		GDPoint pt = SPtoNP (point);
		int ZoneID = pDoc->GetZoneID(pt);

		if(ZoneID > 0)
			return ZoneID;

	}
	return -1;
}
void CTLiteView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CTLiteDoc* pDoc = GetDocument();

	if(m_ToolMode == move_tool)
	{
		m_last_cpoint = point;
		m_last_left_down_point  = point;
		AfxGetApp()->LoadCursor(IDC_MOVENETWORK);
		m_bMoveDisplay = true;
	}



	if(m_ToolMode == select_feature_tool || m_ToolMode == select_link_tool)
	{
		m_last_cpoint = point;
		m_last_left_down_point  = point;
		m_bMoveDisplay = true;
	}
	
	if(m_ToolMode == backgroundimage_tool)
	{
		m_last_cpoint = point;
		AfxGetApp()->LoadCursor(IDC_MOVENETWORK);
		m_bMoveImage = true;
	}

	if(m_ToolMode == network_coordinate_tool)
	{
		m_last_cpoint = point;
		AfxGetApp()->LoadCursor(IDC_MOVENETWORK);
		m_bMoveNetwork = true;
	}


	if(m_ToolMode == create_1waylink_tool || m_ToolMode == create_2waylinks_tool)
	{
		m_TempLinkStartPoint = point;
		m_TempLinkEndPoint = point;
		m_bMouseDownFlag = true;

		AfxGetApp()->LoadCursor(IDC_CREATE_LINK_CURSOR);

	}
	if(m_ToolMode == subarea_tool)
	{
		m_TempZoneStartPoint = point;
		m_TempZoneEndPoint = point;

		m_bMouseDownFlag = true;

		if(!isCreatingSubarea && GetDocument()->m_SubareaShapePoints.size()==0)
		{
			// Record the start location and find the closest feature point
			m_FirstSubareaPoints = point;
			GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));

			isCreatingSubarea = true;
			SetCapture();
		}
		else
		{
			if(bFindCloseSubareaPoint(point) && GetDocument()->m_SubareaShapePoints.size()>= 3)
		 {
			 CWaitCursor wait;
			 GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));
			 GetDocument()->m_SubareaShapePoints.push_back(GetDocument()->m_SubareaShapePoints[0]);
			 isCreatingSubarea = false;
			 CopyLinkSetInSubarea();
			 isFinishSubarea = true;
			 m_ToolMode = select_link_tool;
			 ReleaseCapture();

		 }
			else
		 {
			 GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));
		 }
		}
	}

	CView::OnLButtonDown(nFlags, point);
}


void CTLiteView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CTLiteDoc* pDoc = GetDocument();
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	if(m_ToolMode == move_tool)
	{
		CSize OffSet = point - m_last_cpoint;
		m_Origin.x -= OffSet.cx/m_Resolution;
		m_Origin.y -= OffSet.cy*m_OriginOnBottomFlag/m_Resolution;

		AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		SetGlobalViewParameters();

		m_bMoveDisplay = false;
	}

	if(m_ToolMode == select_link_tool)
	{
		CSize OffSet_ButtonDownUp = point - m_last_left_down_point;
		CSize OffSet = point - m_last_cpoint;
		
		// use move tool mode when offset is positive

		if((abs(OffSet_ButtonDownUp.cx) + abs(OffSet_ButtonDownUp.cx)) >=2) 
		{
		m_Origin.x -= OffSet.cx/m_Resolution;
		m_Origin.y -= OffSet.cy*m_OriginOnBottomFlag/m_Resolution;

		AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		SetGlobalViewParameters();

		m_bMoveDisplay = false;
		}else
		{	// same point for LButton down and up messages

		}
	}

	if(m_ToolMode == select_feature_tool)
	{
		CSize OffSet_ButtonDownUp = point - m_last_left_down_point;
		CSize OffSet = point - m_last_cpoint;
		// use move tool mode when offset is positive
		if((abs(OffSet_ButtonDownUp.cx) + abs(OffSet_ButtonDownUp.cx)) >=2) 
		{
		m_Origin.x -= OffSet.cx/m_Resolution;
		m_Origin.y -= OffSet.cy*m_OriginOnBottomFlag/m_Resolution;

		AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		SetGlobalViewParameters();

		m_bMoveDisplay = false;
		}else
		{ // same point for LButton down and up messages

		switch (pMainFrame->m_iSelectedLayer )
		{
		case layer_node:

		pDoc->m_SelectedLinkID = -1;
		for (std::list<DTALink*>::iterator iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
		{
			(*iLink)->m_DisplayLinkID = -1;
		}

		pDoc->m_SelectedNodeID = FindClosestNode(point, pDoc->m_NodeDisplaySize*pDoc->m_UnitFeet*m_Resolution*2.0f);
		
		if(pDoc->m_SelectedNodeID >= 0)
		{
		
		pMainFrame->m_FeatureInfoVector.clear();
			
		DTANode* pNode = pDoc->m_NodeIDMap [pDoc->m_SelectedNodeID ];
		CFeatureInfo element;
		element.Attribute = "Node ID";
		element.Data.Format ("%d",pNode->m_NodeNumber  );
		pMainFrame->m_FeatureInfoVector.push_back (element);

			element.Attribute = "Control Type";

			if(pNode->m_ControlType == pDoc->m_ControlType_UnknownControl) element.Data.Format ("Unknown Control" );
			if(pNode->m_ControlType == pDoc->m_ControlType_ExternalNode) element.Data.Format ("External Node" );
			if(pNode->m_ControlType ==  pDoc->m_ControlType_NoControl) element.Data.Format ("No Control" );
			if(pNode->m_ControlType ==  pDoc->m_ControlType_YieldSign) element.Data.Format ("Yield Sign" );
			if(pNode->m_ControlType ==  pDoc->m_ControlType_2wayStopSign) element.Data.Format ("2 Way Stop" );
			if(pNode->m_ControlType ==  pDoc->m_ControlType_4wayStopSign) element.Data.Format ("4 Way Stop" );
			if(pNode->m_ControlType ==  pDoc->m_ControlType_PretimedSignal) element.Data.Format ("Pretimed Signal" );
			if(pNode->m_ControlType ==  pDoc->m_ControlType_AcuatedSignal) element.Data.Format ("Acuated Signal" );
			if(pNode->m_ControlType ==  pDoc->m_ControlType_Roundabout) element.Data.Format ("Roundabout" );

			pMainFrame->m_FeatureInfoVector.push_back (element);

		element.Attribute = "x";
		element.Data.Format ("%f",pNode->pt.x   );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		element.Attribute = "y";
		element.Data.Format ("%f",pNode->pt.y   );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		element.Attribute = "Associated Zone ID";
		element.Data.Format ("%d",pNode->m_ZoneID   );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		pMainFrame->FillFeatureInfo ();
		
		}else
		{
			pMainFrame->m_FeatureInfoVector.clear();
			pMainFrame->FillFeatureInfo ();
		}


		break;

		case layer_link:
		case layer_link_MOE:

		pDoc->m_SelectedNodeID = -1;
		OnClickLink(nFlags, point);

		if(pDoc->m_SelectedLinkID >= 0)
		{
		
		pMainFrame->m_FeatureInfoVector.clear();
		
		DTALink* pLink = pDoc->m_LinkNoMap [pDoc->m_SelectedLinkID ];
		CFeatureInfo element;
		element.Attribute = "Link ID";
		element.Data.Format ("%d",pLink->m_LinkNo );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		element.Attribute = "From Node ID";
		element.Data.Format ("%d",pLink->m_FromNodeNumber  );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		element.Attribute = "To Node ID";
		element.Data.Format ("%d",pLink->m_ToNodeNumber   );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		if(pDoc->m_LinkTypeMap.find(pLink->m_link_type) != pDoc->m_LinkTypeMap.end())
		{
		element.Attribute = "Type";
		element.Data.Format ("%s",pDoc->m_LinkTypeMap  [ pLink->m_link_type ].link_type_name.c_str () );
		pMainFrame->m_FeatureInfoVector.push_back (element);
		}

		element.Attribute = "Speed Limit (mph)";
		element.Data.Format ("%3.0f",pLink->m_SpeedLimit );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		element.Attribute = "Length (mile)";
		element.Data.Format ("%4.3f",pLink->m_Length   );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		element.Attribute = "FFTT (min)";
		element.Data.Format ("%4.3f",pLink->m_FreeFlowTravelTime    );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		element.Attribute = "# of lanes";
		element.Data.Format ("%d",pLink->m_NumLanes );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		element.Attribute = "Lane Capacity";
		element.Data.Format ("%4.0f",pLink->m_MaximumServiceFlowRatePHPL  );
		pMainFrame->m_FeatureInfoVector.push_back (element);

		pMainFrame->FillFeatureInfo ();

		
		}else
		{
			pMainFrame->m_FeatureInfoVector.clear();	
			pMainFrame->FillFeatureInfo ();
		}
		
		if(pMainFrame->m_iSelectedLayer == layer_link_MOE)
		{
		g_LinkMOEDlgShowFlag = false;
		pDoc->OnViewShowmoe();
		}

		break;

		}
		m_bMoveDisplay = false;


		}
	}

	if(m_ToolMode == backgroundimage_tool)
	{

		CSize OffSet = point - m_last_cpoint;
		CTLiteDoc* pDoc = GetDocument();
		pDoc->m_ImageX1  += OffSet.cx*pDoc->m_ImageMoveSize;
		pDoc->m_ImageY1  += OffSet.cy*m_OriginOnBottomFlag*pDoc->m_ImageMoveSize;

		AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		m_bMoveImage = false;
	}
	if(m_ToolMode == network_coordinate_tool)
	{
		AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		m_bMoveNetwork = false;
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

	if((m_ToolMode == move_tool || m_ToolMode == select_link_tool || m_ToolMode == select_feature_tool) && m_bMoveDisplay)
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
	if(m_ToolMode == backgroundimage_tool && m_bMoveImage)
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

	if(m_ToolMode == network_coordinate_tool && m_bMoveNetwork)
	{
		CSize OffSet = point - m_last_cpoint;
		//		if(!(OffSet.cx !=0 && OffSet.cy !=0))
		{
			CTLiteDoc* pDoc = GetDocument();
			float delta_x = OffSet.cx/m_Resolution;
			float delta_y = OffSet.cy*m_OriginOnBottomFlag/m_Resolution;

			pDoc->ChangeNetworkCoordinates(0,1.0f,1.0f,delta_x,delta_y);
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
	if(m_ToolMode == subarea_tool)
	{
		if(isCreatingSubarea)
		{
			// if it is the first moving operation, erase the previous temporal link
			if(m_TempZoneStartPoint!=m_TempZoneEndPoint)
				DrawTemporalLink(m_TempZoneStartPoint,m_TempZoneEndPoint);

			// update m_TempLinkEndPoint from the current mouse point
			m_TempZoneEndPoint = point;

			// draw a new temporal link
			DrawTemporalLink(m_TempZoneStartPoint,m_TempZoneEndPoint);
		}



		AfxGetApp()->LoadCursor(IDC_CURSOR_SUBAREA);

	}


	CString str;
	GDPoint  gdpt  = SPtoNP(point);
	str.Format("%.5f,%.5f", gdpt.x, gdpt.y);
	GetDocument()->SendTexttoStatusBar(str);

	CView::OnMouseMove(nFlags, point);
}

void CTLiteView::OnViewSelect()
{
	m_ToolMode = select_link_tool;
}

void CTLiteView::OnUpdateViewSelect(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == select_link_tool ? 1 : 0);
}

void CTLiteView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnRButtonDown(nFlags, point);

	if(m_ToolMode == subarea_tool && GetDocument()->m_SubareaShapePoints.size()>= 3)
	{
		CWaitCursor wait;
		GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));
		GetDocument()->m_SubareaShapePoints.push_back(GetDocument()->m_SubareaShapePoints[0]);
		isCreatingSubarea = false;
		CopyLinkSetInSubarea();
		isFinishSubarea = true;
		m_ToolMode = select_link_tool;
		ReleaseCapture();

		Invalidate();
	}
}

void CTLiteView::OnNodeOrigin()
{
	CTLiteDoc* pDoc = GetDocument();

	pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 100);  // 100 is screen unit

	pDoc->m_OriginNodeID = pDoc->m_SelectedNodeID;
	if(pDoc->m_SelectedNodeID>=0)
		TRACE("ONode %s selected.\n", pDoc->m_NodeIDMap [pDoc->m_OriginNodeID]->m_Name );
	pDoc->Routing(false);

	if(pDoc->m_bShowPathList)
		pDoc->ShowPathListDlg(pDoc->m_bShowPathList);

	m_ShowAllPaths = true;
	Invalidate();

}

void CTLiteView::OnNodeDestination()
{
	CTLiteDoc* pDoc = GetDocument();
	pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	pDoc->m_DestinationNodeID = pDoc->m_SelectedNodeID;
	if(pDoc->m_SelectedNodeID>=0)
		TRACE("ONode %s selected.\n", pDoc->m_NodeIDMap [pDoc->m_DestinationNodeID]->m_Name );

	m_ShowAllPaths = true;
	// pDoc->Routing(false);

	pDoc->AlternativeRouting(3);

	if(pDoc->m_bShowPathList)
		pDoc->ShowPathListDlg(pDoc->m_bShowPathList);

	Invalidate();

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



	for (std::list<DTALink*>::iterator iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{

		for(int si = 0; si < (*iLink) ->m_ShapePoints .size()-1; si++)
		{

			FromPoint = NPtoSP((*iLink)->m_ShapePoints[si]);
			ToPoint = NPtoSP((*iLink)->m_ShapePoints[si+1]);

			GDPoint p0, pfrom, pto;
			p0.x  = point.x; p0.y  = point.y;
			pfrom.x  = FromPoint.x; pfrom.y  = FromPoint.y;
			pto.x  = ToPoint.x; pto.y  = ToPoint.y;

			float distance = g_DistancePointLine(p0, pfrom, pto);

			if(distance >0 && distance < Min_distance)
			{
				pDoc->m_SelectedLinkID = (*iLink)->m_LinkNo ;

				Min_distance = distance;
			}
		}
	}		

	if(Min_distance > pDoc->m_NodeDisplaySize*pDoc->m_UnitFeet*m_Resolution*20)
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

		for (std::list<int>::iterator iLink = g_LinkDisplayList.begin(); iLink != g_LinkDisplayList.end(); iLink++)
		{
			DTALink* pLink = pDoc->m_LinkNoMap [(*iLink)];
			if(pLink->m_LinkNo  == pDoc->m_SelectedLinkID)
			{ 
				bFoundFlag = true; 
				break;
			}

		}
		if(!bFoundFlag)
			g_LinkDisplayList.push_back(pDoc->m_SelectedLinkID);
	}



	// reset
	for (std::list<DTALink*>::iterator iLink = pDoc->m_LinkSet.begin(); iLink != pDoc->m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_DisplayLinkID = -1;
	}

	int LinkCount=0;
	for (std::list<int>::iterator iLinkDisplay = g_LinkDisplayList.begin(); iLinkDisplay != g_LinkDisplayList.end(); iLinkDisplay++, LinkCount++)
	{
		pDoc->m_LinkNoMap[(*iLinkDisplay)]->m_DisplayLinkID = LinkCount;

	}

}
void CTLiteView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CTLiteDoc* pDoc = GetDocument();

/*	if( pDoc->m_TrafficFlowModelFlag > 0 || m_ToolMode == select_link_tool)
	{
		// create MOE Dlg when double clicking

		OnClickLink(nFlags, point);
		g_LinkMOEDlgShowFlag = false;
		pDoc->OnViewShowmoe();
	}
*/
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

	if(m_ToolMode == subarea_tool && GetDocument()->m_SubareaShapePoints.size()>= 3)
	{
		CWaitCursor wait;
		GetDocument()->m_SubareaShapePoints.push_back(SPtoNP(point));
		GetDocument()->m_SubareaShapePoints.push_back(GetDocument()->m_SubareaShapePoints[0]);
		isCreatingSubarea = false;
		CopyLinkSetInSubarea();
		isFinishSubarea = true;
		m_ToolMode = select_link_tool;
		ReleaseCapture();

		Invalidate();
	}

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

		if(dlg.m_SearchMode == efind_link)
		{

			DTALink* pLink = pDoc->FindLinkWithNodeNumbers(dlg.m_FromNodeNumber ,dlg.m_ToNodeNumber );

			if(pLink !=NULL)
			{
				pDoc->m_SelectedLinkID = pLink->m_LinkNo ;
				pDoc->m_SelectedNodeID = -1;

				m_SelectFromNodeNumber = dlg.m_FromNodeNumber;
				m_SelectToNodeNumber = dlg.m_ToNodeNumber;
				Invalidate();

			}
		}

		if(dlg.m_SearchMode == efind_node)
		{
			DTANode* pNode = pDoc->FindNodeWithNodeNumber (dlg.m_NodeNumber);
			if(pNode !=NULL)
			{
				pDoc->m_SelectedLinkID = -1;
				pDoc->m_SelectedNodeID = pNode->m_NodeID ;

				m_bShowNode = true;
				Invalidate();
			}else
			{
				CString str;

				if(dlg.m_NodeNumber != 0)  // 0 is the default node number in this dialog
				{
					str.Format ("Node %d cannot be found.",dlg.m_NodeNumber);
					AfxMessageBox(str);
				}
			}
		}

		if(dlg.m_SearchMode == efind_path)
		{
			m_SelectFromNodeNumber = dlg.m_FromNodeNumber;
			m_SelectToNodeNumber = dlg.m_ToNodeNumber;

			m_ShowAllPaths = true;

			DTANode* pFromNode = pDoc->FindNodeWithNodeNumber (dlg.m_FromNodeNumber);
			if(pFromNode ==NULL)
			{
				CString str;
				str.Format ("From Node %d cannot be found.",dlg.m_FromNodeNumber);
				AfxMessageBox(str);
				return;
			}else
			{
				pDoc->m_OriginNodeID = pFromNode->m_NodeID;

			}
			DTANode* pToNode = pDoc->FindNodeWithNodeNumber (dlg.m_ToNodeNumber);
			if(pToNode ==NULL)
			{
				CString str;
				str.Format ("To Node %d cannot be found.",dlg.m_ToNodeNumber);
				AfxMessageBox(str);
				return;
			}else
			{
				pDoc->m_DestinationNodeID = pToNode->m_NodeID;
			}

			pDoc->Routing(false);

		}

		if(dlg.m_SearchMode == efind_vehicle)
		{
			int SelectedVehicleID = dlg.m_VehicleNumber; // internal vehicle index starts from zero

			if(pDoc->m_VehicleIDMap.find(SelectedVehicleID) == pDoc->m_VehicleIDMap.end())
			{
				CString str_message;
				str_message.Format ("Vehicle Id %d cannot be found.", SelectedVehicleID+1);
				AfxMessageBox(str_message);
				return;
			}

			std::vector<int> LinkVector;
			DTAVehicle* pVehicle = pDoc->m_VehicleIDMap[SelectedVehicleID];
			for(int link= 1; link<pVehicle->m_NodeSize; link++)
			{
				LinkVector.push_back (pVehicle->m_NodeAry[link].LinkNo);
			}

			pDoc->HighlightPath(LinkVector,1);


			pDoc->m_HighlightGDPointVector.clear();

			pDoc->m_HighlightGDPointVector = pVehicle->m_GPSLocationVector ; // assign the GPS points to be highlighted

		}
	}

	Invalidate();
}

void CTLiteView::OnEditChangebackgroupimagethroughmouse()
{
	if(m_ToolMode!= backgroundimage_tool)
		m_ToolMode = backgroundimage_tool;
	else
		m_ToolMode = move_tool;

}

void CTLiteView::OnUpdateEditChangebackgroupimagethroughmouse(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == backgroundimage_tool ? 1 : 0);
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
		m_ToolMode = select_link_tool;

}

void CTLiteView::OnEditCreate2waylinks()
{
	if(m_ToolMode != create_2waylinks_tool)
		m_ToolMode = create_2waylinks_tool;
	else   //reset to default selection tool
		m_ToolMode = select_link_tool;

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


void CTLiteView::OnImageLockbackgroundimageposition()
{
	m_ToolMode = move_tool;  // swtich m_ToolMode from backgroundimage_tool to other tool
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

	DTALink* pLink= pDoc->m_LinkNoMap [pDoc->m_SelectedLinkID];
	if(pLink!=NULL)
	{
		CDlgLinkProperties dlg;

		dlg.m_pDoc = pDoc;

		dlg.m_LinkID = pLink->m_LinkID ;
		dlg.m_StreetName  = pLink->m_Name.c_str () ;
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

		if(dlg.DoModal() == IDOK)
		{
			pLink->m_Length = dlg.LinkLength;
			pLink->m_SpeedLimit = dlg.SpeedLimit;
			pLink->m_FreeFlowTravelTime = pLink->m_Length /pLink->m_SpeedLimit*60.0f; 

			pLink->m_Name  = dlg.m_StreetName;

			pLink->m_LaneCapacity  = dlg.LaneCapacity;

			if(pLink->m_NumLanes  != dlg.nLane)
			{
			pLink->m_NumLanes  = dlg.nLane;
			pDoc->GenerateOffsetLinkBand();  // update width of band
			}
			pLink->m_link_type = dlg.LinkType ;

			pDoc->m_DefaultSpeedLimit = dlg.DefaultSpeedLimit;
			pDoc->m_DefaultCapacity = dlg.DefaultCapacity;
			pDoc->m_DefaultNumLanes = dlg.DefaultnLane;


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
		m_ToolMode = select_link_tool;
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
	m_ToolMode = select_feature_tool;
}

void CTLiteView::OnUpdateViewSelectNode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == select_feature_tool ? 1 : 0);
}



void CTLiteView::OnEditCreatesubarea()
{
	m_ToolMode = subarea_tool;
		CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

		pMainFrame->m_bShowLayerMap[layer_subarea] = true;
	GetDocument()->m_SubareaShapePoints.clear();

}

void CTLiteView::OnUpdateEditCreatesubarea(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == subarea_tool ? 1 : 0);
}


void CTLiteView::CopyLinkSetInSubarea()
{
	CTLiteDoc* pDoc = GetDocument();

	if( pDoc->m_SubareaShapePoints.size() < 3)
		return;

	LPPOINT m_subarea_points = new POINT[pDoc->m_SubareaShapePoints.size()];
	for (int sub_i= 0; sub_i < pDoc->m_SubareaShapePoints.size(); sub_i++)
	{
		CPoint point =  NPtoSP(pDoc->m_SubareaShapePoints[sub_i]);
		m_subarea_points[sub_i].x = point.x;
		m_subarea_points[sub_i].y = point.y;
	}

	// Create a polygonal region
	m_polygonal_region = CreatePolygonRgn(m_subarea_points, pDoc->m_SubareaShapePoints.size(), WINDING);

	pDoc->m_SubareaNodeSet.clear();
	pDoc->m_SubareaLinkSet.clear();
	pDoc->m_SubareaNodeIDMap.clear();

	std::list<DTANode*>::iterator iNode = pDoc->m_NodeSet.begin ();
	while (iNode != pDoc->m_NodeSet.end())
	{
		CPoint point = NPtoSP((*iNode)->pt);
		if(PtInRegion(m_polygonal_region, point.x, point.y))  //outside subarea
		{
			pDoc->m_SubareaNodeSet .push_back ((*iNode));
			pDoc->m_SubareaNodeIDMap[(*iNode)->m_NodeID ]=  (*iNode);
			//inside subarea
		}
		iNode++;
	}

	std::list<DTALink*>::iterator iLink;

	iLink = pDoc->m_LinkSet.begin(); 

	while (iLink != pDoc->m_LinkSet.end())
	{
		if(pDoc->m_SubareaNodeIDMap.find((*iLink)->m_FromNodeID ) != pDoc->m_SubareaNodeIDMap.end() && pDoc->m_SubareaNodeIDMap.find((*iLink)->m_ToNodeID ) != pDoc->m_SubareaNodeIDMap.end()) 
		{
			pDoc->m_SubareaLinkSet.push_back (*iLink);

			(*iLink)->m_DisplayLinkID = 1;


		}
		iLink++;
	}


	DeleteObject(m_polygonal_region);
	delete [] m_subarea_points;

}

void CTLiteView::OnToolsRemovenodesandlinksoutsidesubarea()
{


	CTLiteDoc* pDoc = GetDocument();

	LPPOINT m_subarea_points = new POINT[pDoc->m_SubareaShapePoints.size()];
	for (unsigned int sub_i= 0; sub_i < pDoc->m_SubareaShapePoints.size(); sub_i++)
	{
		CPoint point =  NPtoSP(pDoc->m_SubareaShapePoints[sub_i]);
		m_subarea_points[sub_i].x = point.x;
		m_subarea_points[sub_i].y = point.y;
	}

	// Create a polygonal region
	m_polygonal_region = CreatePolygonRgn(m_subarea_points, pDoc->m_SubareaShapePoints.size(), WINDING);

	// step 01
		std::list<DTANode*>::iterator iNode;
			iNode = pDoc->m_NodeSet.begin ();


	while (iNode != pDoc->m_NodeSet.end())
	{
		CPoint point = NPtoSP((*iNode)->pt);
		if(PtInRegion(m_polygonal_region, point.x, point.y) == false)  //outside subarea
		{
			(*iNode)->m_bSubareaFlag = 0;
		}
		else
		{
			(*iNode)->m_bSubareaFlag = 1;	//inside subarea
		}

			(*iNode)->m_tobeRemoved = true;  // mark all to be removed first

			++iNode;
	}



	//step 02
	//remove links
	pDoc->m_LinkNoMap.clear();

	std::list<DTALink*>::iterator iLink;

	iLink = pDoc->m_LinkSet.begin(); 

	while (iLink != pDoc->m_LinkSet.end())
	{
		DTANode* pFromNode = pDoc->m_NodeIDMap[(*iLink)->m_FromNodeID];
		DTANode* pToNode = pDoc->m_NodeIDMap[(*iLink)->m_ToNodeID];

		if(pFromNode->m_bSubareaFlag == 0 &&  pToNode ->m_bSubareaFlag == 0 ) 
		{
			iLink = pDoc->m_LinkSet.erase(iLink);  // remove when one of end points are covered by the subarea

		}else
		{
			pDoc->m_LinkNoMap[(*iLink)->m_LinkNo] = (*iLink);

			pFromNode->m_tobeRemoved = false;  // no need to remove as long as one adjacent link is kept
			pToNode->m_tobeRemoved = false; 

			++iLink;
		}
	}


	// step 03: remove nodes
	pDoc->m_NodeIDMap.clear();

	iNode = pDoc->m_NodeSet.begin ();
	while (iNode != pDoc->m_NodeSet.end())
	{

		if((*iNode)->m_tobeRemoved == true )  //outside subarea
		{
			iNode = pDoc->m_NodeSet.erase (iNode);
		}
		else
		{
			pDoc->m_NodeIDMap[(*iNode)->m_NodeID ] = (*iNode);
			++iNode;
			//inside subarea
		}
	}

	

	// step 04: mark zones to be removed. 

		std::map<int, DTAZone>	:: iterator itr;

		for(itr = pDoc->m_ZoneMap.begin(); itr != pDoc->m_ZoneMap.end(); ++itr)
		{

			int activty_node_size = itr->second.m_ActivityLocationVector.size();
			int outofsubarea_count  = 0;
			// copy to another array first		
			std::vector<int> activity_node_vector;
				for(int i = 0; i< itr->second.m_ActivityLocationVector .size(); i++)
				{
					DTAActivityLocation element = itr->second.m_ActivityLocationVector[i];

					activity_node_vector.push_back (element.NodeNumber);
				}

				for(unsigned int k = 0; k < activity_node_vector.size(); k++)
				{
					int NodeID  = pDoc->m_NodeNametoIDMap [activity_node_vector[k]];
					if( pDoc->m_NodeIDMap.find(NodeID) == pDoc->m_NodeIDMap.end())
						itr->second.RemoveNodeActivityMode(activity_node_vector[k]);
				}

				if( itr->second.m_ActivityLocationVector.size() ==0 )
				{
				     itr->second.m_bWithinSubarea = false;
				}
		}

	// step 05: add new zones if the boundary has zone --

	iNode = pDoc->m_NodeSet.begin ();
	int TAZ = 10000;
	int boundary_zone_count = 0;
	while (iNode != pDoc->m_NodeSet.end())
	{

		if((*iNode)->m_bSubareaFlag ==0 && (*iNode)->m_ZoneID == 0)  // remaining boundary-and-not-activity-location nodes after out of subarea links are removed. 
		{
			// create new zone number and add this node as activity center
			// try to get TAZ as the node number

			while(pDoc->m_ZoneMap.find (TAZ) != pDoc->m_ZoneMap.end ())  // find unused TAZ as new TAZ
			{
				TAZ ++;
			}

			// now we have an unused TAZ  
				(*iNode)->m_bZoneActivityLocationFlag = true;

				pDoc->m_ZoneMap [TAZ].m_ZoneTAZ = TAZ;
				(*iNode)->m_ZoneID = TAZ;

				DTAActivityLocation element;
				element.ZoneID  = TAZ;
				element.NodeNumber = (*iNode)->m_NodeNumber  ;
				pDoc->m_ZoneMap [TAZ].m_ActivityLocationVector .push_back (element );
				boundary_zone_count++;
				TAZ ++;

		}
			++iNode;
	}



    // step 06: generate route file
		std::list<DTAVehicle*>::iterator iVehicle;
		pDoc->m_PathMap.clear();

	for (iVehicle = pDoc->m_VehicleSet.begin(); iVehicle != pDoc->m_VehicleSet.end(); iVehicle++)
	{
		DTAVehicle* pVehicle = (*iVehicle);

		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
		{
			
			// update node sequence
				std::vector<int> LinkVector;
			    std::vector<int> NodeVector;

							int StartFlag = 0; // 0: not start yet, 1: start node chain, 2: end node chain
							for(int link= 1; link<pVehicle->m_NodeSize; link++)
							{
								int OrgLinkNO = pVehicle->m_NodeAry[link].LinkNo;


								if( pDoc->m_LinkNoMap.find(OrgLinkNO)!= pDoc->m_LinkNoMap.end()) // the link exists in subarea 
								{

									if(StartFlag <=1)
										LinkVector.push_back(OrgLinkNO);

									if(StartFlag ==0)
										StartFlag = 1; // start node chain
								}else  // link has been removed
								{
									if(StartFlag==1) // node chain has been started 
									{
										StartFlag = 2;  // terminate
										break;
									} else 
									{
									 //continue // StartFlag ==0
									}
								}

							}
							// LinkVector for remaining links in subarea 

							if(LinkVector.size() >=2)
							{

								DTALink* pLinkOrg = pDoc->m_LinkNoMap[LinkVector[0]];
								DTALink* pLinkDes = pDoc->m_LinkNoMap[LinkVector[LinkVector.size()-1]];

								int NewOriginZoneID = pDoc->m_NodeIDMap[pLinkOrg->m_FromNodeID ]->m_ZoneID  ;
								int NewDestinationZoneID = pDoc->m_NodeIDMap[pLinkDes->m_ToNodeID ]->m_ZoneID  ;;
								int NewNodeNumberSum = 0;

							for(unsigned li = 0; li < LinkVector.size(); li++)
							{
								DTALink* pLink = pDoc->m_LinkNoMap[LinkVector[li]];
								if(li==0) // first link
								{
									NewNodeNumberSum += pLink->m_FromNodeNumber;
								}

								NewNodeNumberSum += pLink->m_ToNodeNumber;
							}

							int Newm_NodeSize = LinkVector.size() +1;
							CString label;
							label.Format("%d,%d,%d,%d", NewOriginZoneID  , NewDestinationZoneID , NewNodeNumberSum , Newm_NodeSize );

							//existing path
							pDoc->m_PathMap[label].Origin = NewOriginZoneID;
							pDoc->m_PathMap[label].Destination  = NewDestinationZoneID;
							pDoc->m_PathMap[label].TotalVehicleSize+=1;

						if(pDoc->m_PathMap[label].TotalVehicleSize == 1)  // new path
						{

							for(unsigned li = 0; li < LinkVector.size(); li++)
							{
								DTALink* pLink = pDoc->m_LinkNoMap[LinkVector[li]];
								if(li==0) // first link
								{
									pDoc->m_PathMap[label].m_NodeVector.push_back(pLink->m_FromNodeNumber );
								}

								pDoc->m_PathMap[label].m_NodeVector.push_back(pLink->m_ToNodeNumber);
					
							}
						}

						//
												// reuse label as OD label
						label.Format("%d,%d", pVehicle->m_OriginZoneID  , pVehicle->m_DestinationZoneID);
						pDoc->m_ODMatrixMap[label].Origin = NewOriginZoneID;
						pDoc->m_ODMatrixMap[label].Destination  = NewDestinationZoneID;
						pDoc->m_ODMatrixMap[label].TotalVehicleSize+=1;


						}

			}
	}

	DeleteObject(m_polygonal_region);
	delete [] m_subarea_points;

			int zone_count = 0;

		std::map<int, DTAZone>	:: const_iterator itr_zone;

		for(itr_zone = pDoc->m_ZoneMap.begin(); itr_zone != pDoc->m_ZoneMap.end(); ++itr_zone)
		{
			if(itr_zone->second.m_ActivityLocationVector .size() > 0)
				zone_count++;

		}
	CString SubareaCutMessage;
	SubareaCutMessage.Format ("The subarea includes %d nodes, %d links;\n%d zones, %d newly generated boundary zones;\n%d subarea OD pairs, %d subarea path records.",
		pDoc->m_NodeSet.size(), pDoc->m_LinkSet.size(), zone_count,boundary_zone_count,pDoc->m_ODMatrixMap.size(),
		pDoc->m_PathMap.size());

	AfxMessageBox(SubareaCutMessage, MB_ICONINFORMATION);


	Invalidate();
}

void CTLiteView::OnViewShowAVISensor()
{
	m_bShowAVISensor = !m_bShowAVISensor;
	Invalidate();
}

void CTLiteView::OnUpdateViewShowAVISensor(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowAVISensor ? 1 : 0);
}

void CTLiteView::OnFileDataexchangewithgooglefusiontables()
{
	CopyLinkSetInSubarea();
	CDlg_GoogleFusionTable dlg;
	dlg.m_pDoc= GetDocument();
	dlg.DoModal ();
}


void CTLiteView::OnEditDeletelinksoutsidesubarea()
{
	OnToolsRemovenodesandlinksoutsidesubarea();
}

void CTLiteView::OnEditMovenetworkcoordinates()
{
	if(m_ToolMode!= network_coordinate_tool)
	{
		m_ToolMode = network_coordinate_tool;
		m_bShowGrid = true;
	}
	else
		m_ToolMode = network_coordinate_tool;

	Invalidate();

}

void CTLiteView::OnUpdateEditMovenetworkcoordinates(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ToolMode == network_coordinate_tool ? 1 : 0);
}


void CTLiteView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	Invalidate();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTLiteView::OnViewIncreasenodesize()
{
	CTLiteDoc* pDoc = GetDocument();

	pDoc->m_NodeDisplaySize *=1.2;

	if(pDoc->m_NodeDisplaySize < 1)  // fast increase the size
		pDoc->m_NodeDisplaySize = 1;

	Invalidate();

}

void CTLiteView::OnViewDecreatenodesize()
{
	CTLiteDoc* pDoc = GetDocument();
	pDoc->m_NodeDisplaySize /=1.2;
	Invalidate();
}


void CTLiteView::DrawLinkAsLine(DTALink* pLink, CDC* pDC)
{
	// 

	// normal line
	CTLiteDoc* pDoc = GetDocument();


	for(int si = 0; si < pLink->m_ShapePoints.size()-1; si++)
	{
		FromPoint = NPtoSP(pLink->m_ShapePoints[si]);
		ToPoint = NPtoSP(pLink->m_ShapePoints[si+1]);

		if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
			continue; 


		pDC->MoveTo(FromPoint);
		pDC->LineTo(ToPoint);

		if(m_bShowLinkArrow && si ==0)
		{
			double slopy = atan2((double)(FromPoint.y - ToPoint.y), (double)(FromPoint.x - ToPoint.x));
			double cosy = cos(slopy);
			double siny = sin(slopy);   
			double display_length  = sqrt((double)(FromPoint.y - ToPoint.y)*(FromPoint.y - ToPoint.y)+(double)(FromPoint.x - ToPoint.x)*(FromPoint.x - ToPoint.x));
			double arrow_size = min(7,display_length/5.0);

			if(arrow_size>0.2)
			{

				m_arrow_pts[0] = ToPoint;
				m_arrow_pts[1].x = ToPoint.x + (int)(arrow_size * cosy - (arrow_size / 2.0 * siny) + 0.5);
				m_arrow_pts[1].y = ToPoint.y + (int)(arrow_size * siny + (arrow_size / 2.0 * cosy) + 0.5);
				m_arrow_pts[2].x = ToPoint.x + (int)(arrow_size * cosy + arrow_size / 2.0 * siny + 0.5);
				m_arrow_pts[2].y = ToPoint.y - (int)(arrow_size / 2.0 * cosy - arrow_size * siny + 0.5);

				pDC->Polygon(m_arrow_pts, 3);
			}

		}

	}

	if(pDoc->m_LinkMOEMode == MOE_queue_length)   // queue length mode
	{
		CPen * pOldPen  = pDC->SelectObject(&g_PenQueueColor);
		float value;
		float percentage = pDoc->GetLinkMOE(pLink, pDoc->m_LinkMOEMode,(int)g_Simulation_Time_Stamp,1, value);


		for(int si = 0; si < pLink->m_ShapePoints.size()-1; si++)
		{
			bool bDrawQueueCell = false;

			FromPoint = NPtoSP(pLink->m_ShapePoints[si]);
			ToPoint = NPtoSP(pLink->m_ShapePoints[si+1]);

			if(FromPoint.x==ToPoint.x && FromPoint.y==ToPoint.y)  // same node
				continue; 

			if(pLink->m_ShapePoints.size() == 2)
			{  // simple straight line


				if(percentage < 0.01f)
					break;

				GDPoint pt;
				pt.x =  pLink->m_ShapePoints[0].x + (1-percentage) * (pLink->m_ShapePoints[1].x - pLink->m_ShapePoints[0].x);
				pt.y =  pLink->m_ShapePoints[0].y + (1-percentage) * (pLink->m_ShapePoints[1].y - pLink->m_ShapePoints[0].y);

				FromPoint = NPtoSP(pt);  // new to point as the end of queue line
				bDrawQueueCell = true;

			}else  // 100 feature points
			{
				if( si > (1-percentage)*100)
					bDrawQueueCell = true;

			}


			if(bDrawQueueCell)
			{
				pDC->MoveTo(FromPoint);
				pDC->LineTo(ToPoint);
			}

		}

		pDC->SelectObject(pOldPen);
	}
}


bool CTLiteView::DrawLinkAsBand(DTALink* pLink, CDC* pDC, bool bObservationFlag =false)
{

	int band_point_index = 0;  

	if(pLink ->m_ShapePoints.size() > 900)
	{
		AfxMessageBox("Two many shape points...");
		return false;
	}
	int si; // we should not use unsigned integer here as si-- 
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	if(bObservationFlag == false)
	{  // simulated data
		for(si = 0; si < pLink ->m_ShapePoints .size(); si++)
		{
			m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandLeftShapePoints[si]);
		}

		for(si = pLink ->m_ShapePoints .size()-1; si >=0 ; si--)
		{
			m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandRightShapePoints[si]);
		}

		m_BandPoint[band_point_index++]= NPtoSP(pLink->m_BandLeftShapePoints[0]);

	}else if (pMainFrame->m_bShowLayerMap[layer_detector])
	{  //observed data
		if(pLink ->m_ReferenceBandLeftShapePoints.size() > 0)  // m_ReferenceBandLeftShapePoints has been initialized
		{

			for(si = 0; si < pLink ->m_ShapePoints .size(); si++)
			{
				m_BandPoint[band_point_index++] = NPtoSP(pLink->m_ReferenceBandLeftShapePoints[si]);
			}

			for(si = pLink ->m_ShapePoints .size()-1; si >=0 ; si--)
			{
				m_BandPoint[band_point_index++] = NPtoSP(pLink->m_ReferenceBandRightShapePoints[si]);
			}

			m_BandPoint[band_point_index++]= NPtoSP(pLink->m_ReferenceBandLeftShapePoints[0]);
		}

	}

	pDC->Polygon(m_BandPoint, band_point_index);

	// ****************************************/
	// draw queue length
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_LinkMOEMode == MOE_queue_length)   // queue length mode
	{
		CPen * pOldPen  = pDC->SelectObject(&g_PenQueueBandColor);
		CBrush * pOldBrush  = pDC->SelectObject(&g_BrushQueueBandColor);

		band_point_index = 0;

		float value;
		float percentage = pDoc->GetLinkMOE(pLink, pDoc->m_LinkMOEMode,(int)g_Simulation_Time_Stamp,1, value);

		if(percentage < 0.01f)  // no queue length
			return true; 
		// simulated data
		if(pLink->m_ShapePoints.size() == 2)
		{  // simple straight line


			// point 0
			GDPoint pt0;
			pt0.x =  pLink->m_BandLeftShapePoints[0].x + (1-percentage) * (pLink->m_BandLeftShapePoints[1].x - pLink->m_BandLeftShapePoints[0].x);
			pt0.y =  pLink->m_BandLeftShapePoints[0].y + (1-percentage) * (pLink->m_BandLeftShapePoints[1].y - pLink->m_BandLeftShapePoints[0].y);

			m_BandPoint[band_point_index++] = NPtoSP(pt0);

			// point 1
			m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandLeftShapePoints[1]);

			// point 2
			m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandRightShapePoints[1]);

			// point 3
			GDPoint pt3;
			pt3.x =  pLink->m_BandRightShapePoints[0].x + (1-percentage) * (pLink->m_BandRightShapePoints[1].x - pLink->m_BandRightShapePoints[0].x);
			pt3.y =  pLink->m_BandRightShapePoints[0].y + (1-percentage) * (pLink->m_BandRightShapePoints[1].y - pLink->m_BandRightShapePoints[0].y);
			m_BandPoint[band_point_index++] = NPtoSP(pt3);

			// point 3
			m_BandPoint[band_point_index++]= NPtoSP(pt0);

		}else  // multiple feature point segment
		{
			for(si = 0; si < pLink ->m_ShapePoints .size(); si++)
			{
				if( si > (1-percentage)*100)
				{
					m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandLeftShapePoints[si]);
				}
			}

			for(si = pLink ->m_ShapePoints .size()-1; si >=0 ; si--)
			{
				if( si > (1-percentage)*100)
				{					
					m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandRightShapePoints[si]);
				}
			}

			m_BandPoint[band_point_index++]= m_BandPoint [0];


		}

		pDC->Polygon(m_BandPoint, band_point_index);

		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);

	}


	return true;
}

bool CTLiteView::DrawLinkAsLaneGroup(DTALink* pLink, CDC* pDC)
{
	CTLiteDoc* pDoc = GetDocument();


	pLink->m_SetBackStart = min (pDoc->m_NodeDisplaySize, pLink->m_Length*0.3);  // restrict set back to 30% of link length
	pLink->m_SetBackEnd = min (pDoc->m_NodeDisplaySize, pLink->m_Length*0.3);

	pLink->AdjustLinkEndpointsWithSetBack();

	if(pLink ->m_ShapePoints.size() > 900)
	{
		AfxMessageBox("Two many shape points...");
		return false;
	}


	// generate m_BandLeftShapePoints, m_BandRightShapePoints for the whole link with setback points
	pLink->m_BandLeftShapePoints.clear();
	pLink->m_BandRightShapePoints.clear();


	double lane_offset = pDoc->m_UnitFeet*pDoc->m_LaneWidthInFeet;  // 20 feet per lane

	int last_shape_point_id = pLink->m_ShapePoints .size() -1;
	double DeltaX = pLink->m_ShapePoints[last_shape_point_id].x - pLink->m_ShapePoints[0].x;
	double DeltaY = pLink->m_ShapePoints[last_shape_point_id].y - pLink->m_ShapePoints[0].y;
	double theta = atan2(DeltaY, DeltaX);

	int si;  // we cannot use unsigned int here as there is a conditoin below
	// 	for(si = pLink ->m_ShapePoints .size()-1; si >=0 ; si--)
	for(si = 0; si < pLink ->m_ShapePoints .size(); si++)
	{
		GDPoint pt;

		GDPoint OrgShapepoint = pLink->m_ShapePoints[si];

		if(si == 0)
			OrgShapepoint = pLink->m_FromPointWithSetback;

		if(si == last_shape_point_id)
			OrgShapepoint = pLink->m_ToPointWithSetback;

		pt.x = OrgShapepoint.x - lane_offset* cos(theta-PI/2.0f);
		pt.y = OrgShapepoint.y - lane_offset* sin(theta-PI/2.0f);

		pLink->m_BandLeftShapePoints.push_back (pt);

		pt.x  = OrgShapepoint.x + pLink->m_NumLanes *lane_offset* cos(theta-PI/2.0f);
		pt.y = OrgShapepoint.y + pLink->m_NumLanes*lane_offset* sin(theta-PI/2.0f);

		pLink->m_BandRightShapePoints.push_back (pt);
	}



	int band_point_index = 0;
	// close the loop to construct

	for(si = 0; si < pLink ->m_ShapePoints .size(); si++)
	{
		m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandLeftShapePoints[si]);
	}

	for(si = pLink ->m_ShapePoints .size()-1; si >=0 ; si--)
	{
		m_BandPoint[band_point_index++] = NPtoSP(pLink->m_BandRightShapePoints[si]);
	}

	m_BandPoint[band_point_index++]= NPtoSP(pLink->m_BandLeftShapePoints[0]);


	pDC->Polygon(m_BandPoint, band_point_index);

	/*
	// draw lane markings

	CPen* OldPen = pDC->SelectObject(&g_LaneMarkingPen);

	for(int lane = 1; lane < pLink->m_NumLanes ; lane++)
	{
	for(si = 0; si < pLink ->m_ShapePoints .size(); si++)
	{
	GDPoint pt;
	GDPoint OrgShapepoint = pLink->m_ShapePoints[si];

	if(si == 0)
	{
	OrgShapepoint = pLink->m_FromPointWithSetback;
	}

	if(si == last_shape_point_id)
	OrgShapepoint = pLink->m_ToPointWithSetback;

	pt.x = OrgShapepoint.x + lane*lane_offset* cos(theta-PI/2.0f);
	pt.y = OrgShapepoint.y + lane*lane_offset* sin(theta-PI/2.0f);

	if(si == 0)
	pDC->MoveTo(NPtoSP(pt));
	else
	pDC->LineTo(NPtoSP(pt));

	}

	}
	pDC->SelectObject(OldPen);  // restore pen
	*/
	return true;
}

void CTLiteView::OnViewDisplaylanewidth()
{
	if(m_link_display_mode ==link_display_mode_line )
		m_link_display_mode = link_display_mode_band;
	else
		m_link_display_mode = link_display_mode_line;

	Invalidate();
}


void CTLiteView::OnNodeCheckconnectivityfromhere()
{
	CTLiteDoc* pDoc = GetDocument();
	pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit
	pDoc->m_OriginNodeID = pDoc->m_SelectedNodeID;
	pDoc->Routing(true);
	m_ShowAllPaths = true;
	Invalidate();
}

void CTLiteView::OnNodeDirectiontohereandvehicleanalaysis()
{
	OnNodeDestination();

	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_PathDisplayList .size()>0)
	{
		pDoc->m_VehicleSelectionMode = CLS_path;  // select path analysis
		CDlg_VehicleClassification dlg;
		dlg.m_VehicleSelectionNo = CLS_path;
		dlg.m_pDoc = pDoc;
		dlg.DoModal ();
	}

}

void CTLiteView::OnNodeDirectionfromhereandvehicleanalasis()
{
	OnNodeOrigin();

	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_PathDisplayList .size()>0)
	{
		pDoc->m_VehicleSelectionMode = CLS_path;  // select path analysis
		CDlg_VehicleClassification dlg;
		dlg.m_VehicleSelectionNo = CLS_path;
		dlg.m_pDoc = pDoc;
		dlg.DoModal ();
	}

}
extern void g_RandomCapacity(float* ptr, int num, float mean, float COV,int seed);
#define MAX_SAMPLE_SIZE 200

void CTLiteView::OnNodeDirectiontohereandreliabilityanalysis()
{    OnNodeDestination();
CTLiteDoc* pDoc = GetDocument();

bool b_Impacted = false;
float OriginalCapacity = 0.0f;
float ImpactDuration = 0.0f;
float LaneClosureRatio = 0.0f;

float CurrentTime = g_Simulation_Time_Stamp;

std::vector<float> LinkCapacity;
std::vector<float> LinkTravelTime;

float max_density = 0.0f;

int BottleneckIdx = 0;
int ImpactedLinkIdx = -1;

float free_flow_travel_time = 0.0f;


if(pDoc->m_PathDisplayList.size()>0)
{

	DTAPath* pPath = &pDoc->m_PathDisplayList[0];  // 0 is the current selected path
	for (int i=0;i<pPath->m_LinkSize;i++)  // for each pass link
	{
		DTALink* pLink = pDoc->m_LinkNoMap[pPath->m_LinkVector[i]];

		float linkcapacity = pLink->m_LaneCapacity;
		float linktraveltime = pLink->m_Length/pLink->GetObsSpeed(CurrentTime)*60;
		float density = pLink->GetObsDensity(CurrentTime);

		if (density > max_density) BottleneckIdx = i;

		LinkCapacity.push_back(linkcapacity);
		LinkTravelTime.push_back(linktraveltime);
		free_flow_travel_time += linktraveltime;

		// for the first link, i==0, use your current code to generate delay, 
		//additional for user-specified incidents along the routes, add additional delay based on input

		if (!b_Impacted)
		{
			LaneClosureRatio = pLink->GetImpactedFlag(CurrentTime); // check capacity reduction event

			if(LaneClosureRatio > 0.01) // This link is 
			{  
				// use the incident duration data in CapacityReductionVector[] to calculate the additional delay...
				//
				// CurrentTime +=additional delay...

				if (pLink->CapacityReductionVector.size() != 0)
				{
					ImpactDuration = pLink->CapacityReductionVector[0].EndTime - pLink->CapacityReductionVector[0].StartTime;
				}

				ImpactedLinkIdx = i;

				b_Impacted = true;

			}
		}

		CurrentTime += (pLink->m_Length/pLink->GetObsSpeed(CurrentTime))*60;
	}
}

CDlg_TravelTimeReliability dlg;
dlg.m_pDoc= pDoc;
dlg.LinkCapacity = LinkCapacity;
dlg.LinkTravelTime = LinkTravelTime;

dlg.m_BottleneckIdx = BottleneckIdx;

if (b_Impacted)
{
	dlg.m_bImpacted = b_Impacted;
	dlg.m_ImpactDuration = ImpactDuration;
	dlg.m_LaneClosureRatio = LaneClosureRatio/100.0f;
	dlg.m_ImpactedLinkIdx = ImpactedLinkIdx;
}

dlg.m_PathFreeFlowTravelTime = free_flow_travel_time;
dlg.DoModal ();
}
void CTLiteView::OnLinkIncreasebandwidth()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_LinkBandWidthMode == LBW_number_of_lanes)
	{
		pDoc->m_MaxLinkWidthAsNumberOfLanes  /=1.2;
	}
	// 
	if(pDoc->m_LinkBandWidthMode == LBW_link_volume)
	{
		pDoc->m_MaxLinkWidthAsLinkVolume  /=1.5;
	}

	pDoc->GenerateOffsetLinkBand();
	Invalidate();
}

void CTLiteView::OnLinkDecreasebandwidth()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc->m_LinkBandWidthMode == LBW_number_of_lanes)
	{
		pDoc->m_MaxLinkWidthAsNumberOfLanes  *=1.2;
	}
	// 
	if(pDoc->m_LinkBandWidthMode == LBW_link_volume)
	{
		pDoc->m_MaxLinkWidthAsLinkVolume  *=1.5;
	}


	pDoc->GenerateOffsetLinkBand();
	Invalidate();

}

void CTLiteView::OnUpdateViewSensor(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowSensor ? 1 : 0);
}

void CTLiteView::OnLinkSwichtolineBandwidthMode()
{

	m_link_display_mode = link_display_mode_band;
	CTLiteDoc* pDoc = GetDocument();
	pDoc->GenerateOffsetLinkBand();


	Invalidate();
}

void CTLiteView::OnViewTransitlayer()
{
	m_bShowTransit = !m_bShowTransit;
	Invalidate();
}

void CTLiteView::OnUpdateViewTransitlayer(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowTransit);
}

void CTLiteView::DrawPublicTransitLayer(CDC *pDC)
{

	CTLiteDoc* pDoc = GetDocument();

	CPoint ScreenPoint;
	CPoint ScreenPoint_Prev;  //previous screen point
	CSize size;
	double screen_distance;

	std::map<int, PT_Route>::iterator iPT_RouteMap;
	std::map<int, PT_Stop>::iterator iPT_StopMap;

	std::map<int, GDPoint>::iterator i_RouteShapePoints;         //route line shape point 

	for(iPT_RouteMap = pDoc->m_PT_network.m_PT_RouteMap.begin (); iPT_RouteMap != pDoc->m_PT_network.m_PT_RouteMap.end(); iPT_RouteMap++)
	{

		bool bStartPointFlag  = true;

		PT_Route route = iPT_RouteMap->second ;

		if(route.bInsideFreewayNetwork == true)
		{

			for(i_RouteShapePoints = route.m_RouteShapePoints.begin (); i_RouteShapePoints != route.m_RouteShapePoints.end (); i_RouteShapePoints++)
			{

				GDPoint shape_point = i_RouteShapePoints->second;
				double min_display_interval = 10;

				if( pDoc->m_NetworkRect.PtInRect(shape_point) == false)
				{
					min_display_interval = 40;
				}

				ScreenPoint = NPtoSP(shape_point);

				if(bStartPointFlag)
				{
					pDC->MoveTo (ScreenPoint);
					bStartPointFlag = false;
					ScreenPoint_Prev = ScreenPoint;
				}else
				{
					size = ScreenPoint - ScreenPoint_Prev;
					screen_distance = pow((size.cx*size.cx + size.cy*size.cy),0.5);

					if(screen_distance >=min_display_interval)  // 30 points
					{  // draw for different screen points only
						pDC->LineTo  (ScreenPoint);
						ScreenPoint_Prev = ScreenPoint;
					}
				}
			}
		}
	}
}

void CTLiteView::DrawNode(CDC *pDC, DTANode* pNode, CPoint point, int node_size,TEXTMETRIC tm)
{

	if(pNode->m_ZoneID >0)  // if destination node associated with zones
	{
		float zone_multipler = 1;
		pDC->Ellipse(point.x - node_size*zone_multipler, point.y + node_size*zone_multipler,
			point.x + node_size*zone_multipler, point.y - node_size*zone_multipler);
	}else if(pNode->m_bSignalData)  // traffic signal control
	{
		pDC->SelectObject(&g_PenSignalNodeColor);

		pDC->Ellipse(point.x - node_size, point.y + node_size,
			point.x + node_size, point.y - node_size);
	} else
	{
		pDC->Rectangle(point.x - node_size, point.y + node_size,
			point.x + node_size, point.y - node_size);
	}

	if(m_bShowNodeNumber)
	{

		CString str_nodenumber;
		str_nodenumber.Format ("%d",pNode->m_NodeNumber );

		if(m_bHighlightActivityLocation)
		{

			if(pNode->m_ZoneID > 0 && pNode->m_External_OD_flag == 1)  // external origin
				str_nodenumber.Format ("EO:%d",pNode->m_NodeNumber );
			if(pNode->m_ZoneID > 0 && pNode->m_External_OD_flag == -1)  // external destination
				str_nodenumber.Format ("ED:%d",pNode->m_NodeNumber );
		}

		if(pNode->m_DistanceToRoot > 0.00001 && pNode->m_DistanceToRoot < MAX_SPLABEL-1)  // check connectivity, overwrite with distance to the root
			str_nodenumber.Format ("%4.1f",pNode->m_DistanceToRoot );

		point.y -= tm.tmHeight / 2;

		pDC->TextOut(point.x , point.y,str_nodenumber);
	}

	if(pNode->m_DistanceToRoot > MAX_SPLABEL-1)  //restore pen
		pDC->SelectObject(&g_PenNodeColor);

}



void CTLiteView::OnNodeMovementproperties()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc == NULL)
		return;

	pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

	if(pDoc->m_SelectedNodeID >=0)
	{

	if ( 0 == m_msStatus )
	{
		m_ms.m_pDoc = pDoc;
		m_ms.PrepareData4Editing();
		m_msStatus = 1;
	}

		CString str;
		str.Format("Node %d",pDoc->m_NodeIDtoNameMap[pDoc->m_SelectedNodeID]);

		CMyPropertySheet sheet(str);
		sheet.SetTitle(str);
		CPage_Node_Movement MovementPage;
		MovementPage.m_pDoc = pDoc;
		MovementPage.m_pView= this;
		MovementPage.m_psp.dwFlags |= PSP_USETITLE;
		MovementPage.m_psp.pszTitle = _T("Movement");
		sheet.AddPage(&MovementPage);  // 0


		//if(pDoc->m_NodeIDMap.find(pDoc->m_SelectedNodeID) ==pDoc->m_NodeIDMap.end())
		//	return;

		//DTANode*  pNode = pDoc->m_NodeIDMap[pDoc->m_SelectedNodeID];

		CPage_Node_Lane LanePage;
		LanePage.m_pDoc = pDoc;
		LanePage.m_pView= this;
		LanePage.m_psp.dwFlags |= PSP_USETITLE;
		LanePage.m_psp.pszTitle = _T("Lane");
		sheet.AddPage(&LanePage);  // 2
		
		CPage_Node_LaneTurn LaneTurnPage;
		LaneTurnPage.m_pDoc = pDoc;
		LaneTurnPage.m_pView= this;
		LaneTurnPage.m_psp.dwFlags |= PSP_USETITLE;
		LaneTurnPage.m_psp.pszTitle = _T("LaneTurn");
		sheet.AddPage(&LaneTurnPage);  // 3

		CPage_Node_Phase PhasePage;
		PhasePage.m_pDoc = pDoc;
		PhasePage.m_pView= this;
		PhasePage.m_psp.dwFlags |= PSP_USETITLE;
		PhasePage.m_psp.pszTitle = _T("Phase");
		sheet.AddPage(&PhasePage);  // 4

		sheet.DoModal();
	}
}

void CTLiteView::OnLinkLinedisplaymode()
{
	m_bLineDisplayConditionalMode = !m_bLineDisplayConditionalMode;

	CTLiteDoc* pDoc = GetDocument();
	if(!m_bLineDisplayConditionalMode)
		pDoc->GenerateOffsetLinkBand();

	Invalidate();
}

void CTLiteView::OnUpdateLinkLinedisplaymode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bLineDisplayConditionalMode);
}

void CTLiteView::OnViewZoneboundary()
{
	m_bShowZoneBoundary = !m_bShowZoneBoundary;
	Invalidate();
}

void CTLiteView::OnUpdateViewZoneboundary(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowZoneBoundary);

}

void CTLiteView::OnViewShowConnector()
{
	m_bShowConnector = ! m_bShowConnector;
	Invalidate();
}

void CTLiteView::OnUpdateViewShowConnector(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowConnector);
}


void CTLiteView::OnViewHighlightcentroidsandactivitylocations()
{
	m_bHighlightActivityLocation = !m_bHighlightActivityLocation;
	Invalidate();
}

void CTLiteView::OnUpdateViewHighlightcentroidsandactivitylocations(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bHighlightActivityLocation);
}

void CTLiteView::OnViewBackgroundcolor()
{
	CTLiteDoc* pDoc = GetDocument();

	CColorDialog dlg(RGB(0, 0, 0), CC_FULLOPEN);
	if (dlg.DoModal() == IDOK)
	{
		pDoc->m_BackgroundColor= dlg.GetColor();
		pDoc->UpdateAllViews(0);
	}
	Invalidate();
}

void CTLiteView::OnActivitylocationmodeNolanduseactivity()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc != NULL)
	{
		int ZoneID = FindClosestZone(m_CurrentMousePoint,0);

		pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

		if(pDoc->m_SelectedNodeID >=0 && ZoneID>0)
		{
			pDoc->m_ZoneMap [ZoneID].RemoveNodeActivityMode(pDoc->m_NodeIDtoNameMap [pDoc->m_SelectedNodeID]);

			// remove zone attributes
			pDoc->m_NodeIDMap [pDoc->m_SelectedNodeID]->m_ZoneID = 0;

		}
	}
	Invalidate();
}

void CTLiteView::OnUpdateActivitylocationmodeNolanduseactivity(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CTLiteView::OnActivitylocationmodeLanduseactivity()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc != NULL)
	{
		int ZoneID = FindClosestZone(m_CurrentMousePoint,0);

		pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

		if(pDoc->m_SelectedNodeID >=0 && ZoneID>0)
		{
			pDoc->m_ZoneMap [ZoneID].SetNodeActivityMode (pDoc->m_NodeIDtoNameMap [pDoc->m_SelectedNodeID],0);  // 0 for land use activity
			// set zone attributes
			pDoc->m_NodeIDMap [pDoc->m_SelectedNodeID]->m_ZoneID = ZoneID;
			pDoc->m_NodeIDMap [pDoc->m_SelectedNodeID]->m_External_OD_flag = 0;

			pDoc->m_SelectedNodeID = -1;

		}
	}
	m_bHighlightActivityLocation  = true;
	Invalidate();
}

void CTLiteView::OnUpdateActivitylocationmodeLanduseactivity(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CTLiteView::OnActivitylocationmodeExternalorigin()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc != NULL)
	{
		int ZoneID = FindClosestZone(m_CurrentMousePoint,0);

		pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

		if(pDoc->m_SelectedNodeID >=0 && ZoneID>0)
		{
			pDoc->m_ZoneMap [ZoneID].SetNodeActivityMode (pDoc->m_NodeIDtoNameMap [pDoc->m_SelectedNodeID],1);  // 1 for external origin
			// set zone attributes
			pDoc->m_NodeIDMap [pDoc->m_SelectedNodeID]->m_ZoneID = ZoneID;
			pDoc->m_NodeIDMap [pDoc->m_SelectedNodeID]->m_External_OD_flag = 1;
			pDoc->m_SelectedNodeID = -1;
		}
	}
	m_bHighlightActivityLocation  = true;
	Invalidate();
}

void CTLiteView::OnUpdateActivitylocationmodeExternalorigin(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CTLiteView::OnActivitylocationmodeExternaldestination()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc != NULL)
	{
		int ZoneID = FindClosestZone(m_CurrentMousePoint,0);

		pDoc->m_SelectedNodeID = FindClosestNode(m_CurrentMousePoint, 300);  // 300 is screen unit

		if(pDoc->m_SelectedNodeID >=0 && ZoneID>0)
		{
			pDoc->m_ZoneMap [ZoneID].SetNodeActivityMode (pDoc->m_NodeIDtoNameMap [pDoc->m_SelectedNodeID],-1);  // -1 for external destination
			// set zone attributes
			pDoc->m_NodeIDMap [pDoc->m_SelectedNodeID]->m_ZoneID = ZoneID;
			pDoc->m_NodeIDMap [pDoc->m_SelectedNodeID]->m_External_OD_flag = -1;
			pDoc->m_SelectedNodeID = -1;

		}
	}
	m_bHighlightActivityLocation  = true;
	Invalidate();
}

void CTLiteView::OnUpdateActivitylocationmodeExternaldestination(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CTLiteView::OnMoeOddemand()
{
	m_bShowODDemandVolume = !m_bShowODDemandVolume;
	Invalidate();

}

void CTLiteView::OnUpdateMoeOddemand(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowODDemandVolume);
}

void CTLiteView::SetGlobalViewParameters()
{

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	if(pMainFrame->m_bSynchronizedDisplay)
	{

	std::list<CTLiteView*>::iterator iView = g_ViewList.begin ();
	while (iView != g_ViewList.end())
	{
		if((*iView) != this)  // not this window
		{
			(*iView)->m_Resolution  = m_Resolution;
			(*iView)->m_ScreenOrigin = m_ScreenOrigin;
			(*iView)->m_Origin  = m_Origin;
			(*iView)->Invalidate ();

		}
		iView++;
	}
	}
}
void CTLiteView::OnUpdateLinkIncreasebandwidth(CCmdUI *pCmdUI)
{
	CTLiteDoc* pDoc = GetDocument();
	pCmdUI->Enable ((pDoc->m_LinkMOEMode == MOE_volume || pDoc->m_LinkMOEMode == MOE_speed || pDoc->m_LinkMOEMode == MOE_vcratio || pDoc->m_LinkMOEMode == MOE_vcratio|| pDoc->m_LinkMOEMode == MOE_none) && m_bLineDisplayConditionalMode  == false);

}

void CTLiteView::OnUpdateLinkDecreasebandwidth(CCmdUI *pCmdUI)
{
	CTLiteDoc* pDoc = GetDocument();

	pCmdUI->Enable ((pDoc->m_LinkMOEMode == MOE_volume || pDoc->m_LinkMOEMode == MOE_speed || pDoc->m_LinkMOEMode == MOE_vcratio || pDoc->m_LinkMOEMode == MOE_vcratio|| pDoc->m_LinkMOEMode == MOE_none)&& m_bLineDisplayConditionalMode  == false);
}


void CTLiteView::CreateDefaultJunction()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc == NULL)
		return;

	if ( 0 == m_msStatus )
	{
		m_ms.m_pDoc = pDoc;
		m_ms.PrepareData4Editing();
		m_msStatus = 1;
		AfxMessageBox("Junctions are created with default information!",MB_OK);
	}
}


void CTLiteView::OnExportCreatevissimfiles()
{
	CTLiteDoc* pDoc = GetDocument();

	if(pDoc == NULL)
		return;

	if ( 0 == m_msStatus )
	{
		m_ms.m_pDoc = pDoc;
		m_ms.PrepareData4Editing();
		m_msStatus = 1;
		AfxMessageBox("Junctions are created with default information!",MB_OK);
	}
	if ( 1 == m_msStatus )
	{
		m_ms.Create2Files();
		AfxMessageBox("VISSIM files are created!",MB_OK);
	}
}

