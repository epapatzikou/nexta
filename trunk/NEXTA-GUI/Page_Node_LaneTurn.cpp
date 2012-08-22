// Page_Node_LaneTurn.cpp : implementation file
//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com); Peng Du ()

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
#include "Page_NOde_Lane.h"
#include "Page_Node_LaneTurn.h"

#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"

#include <string>
#include <sstream>
// CPage_Node_LaneTurn dialog


IMPLEMENT_DYNAMIC(CPage_Node_LaneTurn, CPropertyPage)

CPage_Node_LaneTurn::CPage_Node_LaneTurn()
	: CPropertyPage(CPage_Node_LaneTurn::IDD)
	, m_CurrentNodeName(0)
{
	m_SelectedMovementIndex = -1;
	
}

CPage_Node_LaneTurn::~CPage_Node_LaneTurn()
{
}

void CPage_Node_LaneTurn::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_GRIDLISTCTRLEX,m_ListCtrl);
	DDX_Text(pDX, IDC_EDIT_CURRENT_NODEID, m_CurrentNodeName);
}


BEGIN_MESSAGE_MAP(CPage_Node_LaneTurn, CPropertyPage)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GRIDLISTCTRLEX, &CPage_Node_LaneTurn::OnLvnItemchangedGridlistctrlex)
	ON_BN_CLICKED(IDC_BTN_DELTURN, &CPage_Node_LaneTurn::OnBnClickedBtnDelturn)
	ON_BN_CLICKED(IDC_BTN_ADDTURN, &CPage_Node_LaneTurn::OnBnClickedBtnAddturn)
END_MESSAGE_MAP()



BOOL CPage_Node_LaneTurn::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_CurrentNodeID =  m_pDoc->m_SelectedNodeID ;
	MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);

	SetDrawPara();
	InitLinkData();
	NormalizeAngles();
	CalculateOA();


	// Give better margin to editors
	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;  // Hao: this ponter should be delete. 
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);

	std::vector<std::string> m_Column_names;

	m_Column_names.push_back ("LaneTurn Index");
	m_Column_names.push_back ("From Link");
	m_Column_names.push_back ("From Lane");
	m_Column_names.push_back ("To Link");
	m_Column_names.push_back ("To Lane");
	m_Column_names.push_back ("Turn Type"); // RTL
	//m_Column_names.push_back ("SC No");
	//m_Column_names.push_back ("Group No");

	//Add Columns and set headers
	for (size_t i=0;i<m_Column_names.size();i++)
	{

		CGridColumnTrait* pTrait = NULL;
//		pTrait = new CGridColumnTraitEdit();
		m_ListCtrl.InsertColumnTrait((int)i,m_Column_names.at(i).c_str(),LVCFMT_LEFT,-1,-1, pTrait);
		m_ListCtrl.SetColumnWidth((int)i,LVSCW_AUTOSIZE_USEHEADER);

	}
	m_ListCtrl.SetColumnWidth(0, 80);

	//MCreateDefaultLaneTurns();
	//Add Rows, laneTurns

	FillTurnData();

	m_nInLinkIDSelected=-1;
	m_nOutLinkIDSelected=-1;
	m_nInLaneNumSelected=-1;
	m_nOutLaneNumSelected=-1;
	m_nTurnSelected=-1;

	CButton *btnAdd =(CButton*) GetDlgItem(IDC_BTN_ADDTURN);
	btnAdd->EnableWindow(FALSE);
	CButton *btnDel =(CButton*) GetDlgItem(IDC_BTN_DELTURN);
	btnDel->EnableWindow(FALSE);

	UpdateData(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CPage_Node_LaneTurn::FillTurnData()
{
	MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);

	for (int i=0;i<pNode->LaneTurns.size();i++)
	{
		MLaneTurn *pTurn = pNode->LaneTurns[i];

		CString str;
		str.Format("%d",i+1);
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,str , 0, 0, 0, NULL);

		str.Format ("%d", pTurn->nFromLinkId);
		m_ListCtrl.SetItemText(Index, 1,str);
		str.Format ("%d", pTurn->nFromIndex);
		m_ListCtrl.SetItemText(Index, 2,str);
		str.Format ("%d", pTurn->nToLinkId);
		m_ListCtrl.SetItemText(Index, 3,str);
		str.Format ("%d", pTurn->nToIndex);
		m_ListCtrl.SetItemText(Index, 4,str);
		if ( 1 == pTurn->nRTL )
			str.Format("Right Turn");
		else if ( 2 == pTurn->nRTL )
			str.Format("Through");
		else if ( 3 == pTurn->nRTL )
			str.Format("Left Turn");
		else
			str.Format("Unknown");
		m_ListCtrl.SetItemText(Index, 5,str);
		str.Format ("%d", pTurn->nSCNO);
		m_ListCtrl.SetItemText(Index, 6,str);
		str.Format ("%d", pTurn->nSignalGroupNo);
		m_ListCtrl.SetItemText(Index, 7,str);		
	}
	return;
}
void CPage_Node_LaneTurn::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect PlotRect;
	GetClientRect(PlotRect);
	m_PlotRect = PlotRect;

	m_PlotRect.top += 35;
	m_PlotRect.bottom -= 35;
	m_PlotRect.left += 550;
	m_PlotRect.right -= 50;

	DrawCentroidRadicalLines(&dc,m_PlotRect,false);
	DrawTest(&dc,m_PlotRect);
	//DrawMovements(&dc,m_PlotRect);
}
void CPage_Node_LaneTurn::DrawMovements(CPaintDC *pDC,CRect PlotRect)
{
	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));
	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_SOLID,0,RGB(0,0,0));
	CPen SelectedPen(PS_SOLID,4,RGB(255,0,0));
	CPen SelectedPhasePen(PS_SOLID,4,RGB(0,0,255));
	CPen redPen(PS_SOLID,3,RGB(255,0,0));
	CPen bluePen(PS_SOLID,3,RGB(0,0,255));

	CPen *pOldPen;

	CBrush  WhiteBrush(RGB(255,255,255)); 

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&DataPen);
	pDC->SelectObject(&WhiteBrush);

	pDC->Rectangle (PlotRect);

	CBrush  BrushLinkBand(RGB(128,128,128)); 
	pDC->SelectObject(&BrushLinkBand);

	int NODE_RADIUS = 3;
	int CIRCLE1		= 100;
	int CIRCLE2		= 175;
	int LANE_WIDTH	= 6;

	int node_set_back = CIRCLE1;
	int link_length   = CIRCLE2;
	int lane_width    = LANE_WIDTH;
	int mid_offset    = 1;

	GDPoint p1,p2,p3;

	for(int i=0;i<m_seriLink.size();i++)
	{
		MLink* pmLink = m_seriLink[i];
		p1  = m_pDoc->m_NodeIDMap[pmLink->m_FromNodeID]->pt;
		p2  = m_pDoc->m_NodeIDMap[pmLink->m_ToNodeID]->pt;
		
		double DeltaX = p2.x - p1.x ;
		double DeltaY = p2.y - p1.y ;
		double theta  = atan2(DeltaY, DeltaX);
		pmLink->theta = theta;

		GDPoint p1_new, p2_new;

		int ndir = (pmLink->nIO==1)? -1 : 1;
		p2_new.x = ndir*node_set_back*cos(theta);  
		p2_new.y = ndir*node_set_back*sin(theta);

		p1_new.x = ndir*link_length*cos(theta);
		p1_new.y = ndir*link_length*sin(theta);

		DrawLink(pDC,p1_new,p2_new,pmLink->m_NumLanes,theta,lane_width);
	}

	CPoint Point_Movement[4];

	GDPoint kz0,kz1,kz2,kzpre,kztemp;
	float kzRate=1.0;

	MLink *pm1 = m_seriLink[0];
	int nAngle0 = pm1->nAngle;

	int nd = (pm1->nIO==1)? -1 : 1;
	p1.x = nd*node_set_back*cos(pm1->theta);  
	p1.y = nd*node_set_back*sin(pm1->theta);
	p2.x = p1.x + (lane_width * pm1->m_NumLanes + mid_offset) * cos(pm1->theta-PI/2.0f);
	p2.y = p1.y + (lane_width * pm1->m_NumLanes + mid_offset) * sin(pm1->theta-PI/2.0f);

	kz1.x = p1.x * kzRate;
	kz1.y = p1.y * kzRate;
	kz2.x = kz1.x*kzRate;
	kz2.y = kz1.y*kzRate;
	//kz2.x = kz1.x + (lane_width * pm1->m_NumLanes + mid_offset) * cos(pm1->theta-PI/2.0f);
	//kz2.y = kz1.y + (lane_width * pm1->m_NumLanes + mid_offset) * sin(pm1->theta-PI/2.0f);

	GDPoint p0,ppre,pCenter,psuc;
	pCenter.x = 0.0;
	pCenter.y = 0.0;

	if ( pm1->nIO == 1)
	{
		p0 = p1;
		ppre = p2;
		kz0 = kz1;
		kzpre= kz2;
	}
	else
	{
		p0 = p2;
		ppre = p1;
		kz0 = kz2;
		kzpre= kz1;
	}

	pOldPen = pDC->SelectObject(&redPen);

	pDC->BeginPath();
	pDC->MoveTo(NPtoSP(pCenter));
	pDC->LineTo(NPtoSP(p0));
	pDC->LineTo(NPtoSP(ppre));
	pDC->LineTo(NPtoSP(pCenter));

	for(int i=1;i<m_seriLink.size();i++)
	{
		MLink* pmLink = m_seriLink[i];
		int ndir = (pmLink->nIO==1)? -1 : 1;
		p1.x = ndir*node_set_back*cos(pmLink->theta);  
		p1.y = ndir*node_set_back*sin(pmLink->theta);
		p2.x = p1.x + (lane_width * pmLink->m_NumLanes + mid_offset) * cos(pmLink->theta-PI/2.0f);
		p2.y = p1.y + (lane_width * pmLink->m_NumLanes + mid_offset) * sin(pmLink->theta-PI/2.0f);
		kz1.x = p1.x * kzRate;
		kz1.y = p1.y * kzRate;
		kz2.x = kz1.x*kzRate;
		kz2.y = kz1.y*kzRate;
		if ( pmLink->nIO == 0)
		{
			psuc = p2;
			p2   = p1;
			p1   = psuc;
			kztemp = kz1;
			kz1    = kz2;
			kz2    = kztemp;
		}
		if (pmLink->nAngle != pm1->nAngle)
		{//draw bezier
			pDC->MoveTo(NPtoSP(pCenter));
			pDC->LineTo(NPtoSP(ppre));
			pDC->LineTo(NPtoSP(p1));
			pDC->LineTo(NPtoSP(pCenter));

			Point_Movement[0] = NPtoSP(ppre);
			Point_Movement[1] = NPtoSP(kzpre);
			Point_Movement[2] = NPtoSP(kz1);
			Point_Movement[3] = NPtoSP(p1);
			pOldPen = pDC->SelectObject(&bluePen);
			pDC->PolyBezier(Point_Movement,4);
			pDC->SelectObject(pOldPen);
		}

		pDC->MoveTo(NPtoSP(pCenter));
		pDC->LineTo(NPtoSP(p1));
		pDC->LineTo(NPtoSP(p2));
		pDC->LineTo(NPtoSP(pCenter));

		ppre = p2;
		kzpre = kz2;

		pm1 = pmLink;
	}
	pDC->MoveTo(NPtoSP(pCenter));
	pDC->MoveTo(NPtoSP(ppre));
	pDC->LineTo(NPtoSP(p0));
	pDC->LineTo(NPtoSP(pCenter));

	Point_Movement[0] = NPtoSP(ppre);
	Point_Movement[1] = NPtoSP(kzpre);
	Point_Movement[2] = NPtoSP(kz0);
	Point_Movement[3] = NPtoSP(p0);
	pOldPen = pDC->SelectObject(&bluePen);
	pDC->PolyBezier(Point_Movement,4);
	pDC->SelectObject(pOldPen);

	pDC->EndPath();

	//pDC->StrokeAndFillPath();

	CRgn rgn;
	rgn.CreateFromPath(pDC);
	CBrush  GreyBrush(RGB(128,128,128));
	pDC->InvertRgn(&rgn);
	pDC->FillRgn(&rgn,&GreyBrush);

	pDC->SelectObject(pOldPen);

	// 下面应该draw控制圆点，似乎应将图象放大？

}
void CPage_Node_LaneTurn::DrawMovementsOld(CPaintDC* pDC,CRect PlotRect)
{

	m_MovementBezierVector.clear();

	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));
	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_SOLID,0,RGB(0,0,0));
	CPen SelectedPen(PS_SOLID,4,RGB(255,0,0));
	CPen SelectedPhasePen(PS_SOLID,4,RGB(0,0,255));

	CBrush  WhiteBrush(RGB(255,255,255)); 

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&DataPen);
	pDC->SelectObject(&WhiteBrush);

	pDC->Rectangle (PlotRect);

	CBrush  BrushLinkBand(RGB(128,128,128)); 
	pDC->SelectObject(&BrushLinkBand);

	DTANode* pNode  = m_pDoc->m_NodeIDMap [m_CurrentNodeID];

	int node_size = 10;
	int node_set_back = 75;

	int link_length = 150;
	int lane_width = 8;
	int text_length = link_length+ 20;

	CString str;
	//str.Format("%d",m_CurrentNodeName);
	//pDC->TextOutA( PlotRect.CenterPoint().x-5, PlotRect.CenterPoint().y-5,str);


	for (unsigned int i=0;i< pNode->m_MovementVector .size();i++)
	{
		DTANodeMovement movement = pNode->m_MovementVector[i];
		DTALink* pInLink  = m_pDoc->m_LinkNoMap [movement.IncomingLinkID];
		DTALink* pOutLink  = m_pDoc->m_LinkNoMap [movement.OutgoingLinkID ];

		GDPoint p1, p2, p3;
		// 1: fetch all data
		p1  = m_pDoc->m_NodeIDMap[movement.in_link_from_node_id ]->pt;
		p2  = m_pDoc->m_NodeIDMap[movement.in_link_to_node_id ]->pt;
		p3  = m_pDoc->m_NodeIDMap[movement.out_link_to_node_id]->pt;
		
		double DeltaX = p2.x - p1.x ;
		double DeltaY = p2.y - p1.y ;
		double theta = atan2(DeltaY, DeltaX);

		GDPoint p1_new, p2_new, p3_new;
		GDPoint p1_text, p3_text;
		GDPoint pt_movement[3];


		// 2. set new origin
		p2_new.x = (-1)*node_set_back*cos(theta);  
		p2_new.y = (-1)*node_set_back*sin(theta);


		//int link_mid_offset  = (pInLink->m_NumLanes/2 +1)*lane_width;  // mid
		int link_mid_offset  = 0;//lane_width;  // mid
		
		pt_movement[0].x = p2_new.x + link_mid_offset* cos(theta-PI/2.0f);
		pt_movement[0].y = p2_new.y + link_mid_offset* sin(theta-PI/2.0f);

		// 3 determine the control point for  PolyBezier
		float control_point_ratio = 0;
		if(movement.movement_turn == DTA_Through ) 
		{
		control_point_ratio = 0;
		}else if(movement.movement_turn == DTA_LeftTurn ) 
		{
			control_point_ratio = 1.2;
		}else
		{
			control_point_ratio = 0.5;
		}
		pt_movement[1].x = pt_movement[0].x + node_set_back*control_point_ratio*cos(theta);
		pt_movement[1].y = pt_movement[0].y + node_set_back*control_point_ratio*sin(theta);
		p1_new.x = (-1)*link_length*cos(theta);
		p1_new.y = (-1)*link_length*sin(theta);


		p1_text.x= (-1)*(text_length)*cos(theta);
		p1_text.y= (-1)*(text_length)*sin(theta);

		// 4: draw from node name
		str.Format("%d",m_pDoc->m_NodeIDtoNameMap [movement.in_link_from_node_id]);
		if(p1_text.y < -50)
			p1_text.y +=10;

		CPoint pt_text = NPtoSP(p1_text);

		pDC->TextOutA(pt_text.x-10,pt_text.y,str);

		DrawLink(pDC,p1_new,p2_new,pInLink->m_NumLanes,theta,lane_width);

////////////////////////////////////////////
		//5: outgoing link
		DeltaX = p3.x - p2.x ;
		DeltaY = p3.y - p2.y ;
		theta = atan2(DeltaY, DeltaX);

		// set new origin
		p2_new.x = node_set_back*cos(theta);  
		p2_new.y = node_set_back*sin(theta);

		link_mid_offset  = 0;//(pOutLink->m_NumLanes/2+1)*lane_width;
		pt_movement[2].x = p2_new.x + link_mid_offset* cos(theta-PI/2.0f);
		pt_movement[2].y = p2_new.y + link_mid_offset* sin(theta-PI/2.0f);


		p3_new.x = link_length*cos(theta);
		p3_new.y = link_length*sin(theta);

		p3_text.x= text_length*cos(theta);
		p3_text.y= text_length*sin(theta);


		//draw to node name
		str.Format("%d",m_pDoc->m_NodeIDtoNameMap [movement.out_link_to_node_id]);

		if(p3_text.y < -100)
			p3_text.y +=10;

		pt_text = NPtoSP(p3_text);


		pDC->TextOutA(pt_text.x-10 ,pt_text.y,str);

		DrawLink(pDC,p2_new,p3_new,pOutLink->m_NumLanes,theta,lane_width);


		// draw movement 

		CPoint Point_Movement[4];


		if(i == m_SelectedMovementIndex)
			pDC->SelectObject(&SelectedPen);
		else
			pDC->SelectObject(&NormalPen);

		Point_Movement[0]= NPtoSP(pt_movement[0]);
		Point_Movement[1]= NPtoSP(pt_movement[1]);
		Point_Movement[2]= NPtoSP(pt_movement[1]);
		Point_Movement[3]= NPtoSP(pt_movement[2]);

		MovementBezier element(Point_Movement[0], Point_Movement[1],Point_Movement[3]);
		
		m_MovementBezierVector.push_back (element);


		//pDC->PolyBezier(Point_Movement,4);

		//restore pen
		pDC->SelectObject(&DataPen);

	}
}

void CPage_Node_LaneTurn::DrawLink(CPaintDC* pDC,GDPoint pt_from, GDPoint pt_to,int NumberOfLanes, double theta, int lane_width)
{
		CPen midPen(PS_SOLID,0,RGB(255,255,0));
		CPen lanePen(PS_DOT,0,RGB(255,255,255));
		CPen outlinePen(PS_SOLID,0,RGB(0,0,0));
		CPen *pOldPen;

		CPoint pt1,pt2;
		GDPoint ptf,ptt;
		ptf.x = pt_from.x;
		ptf.y = pt_from.y;
		ptt.x = pt_to.x;
		ptt.y = pt_to.y;

		int mid_offset = 1;// mid width;
		int link_offset = lane_width;

		CPoint DrawPoint[4];

		DrawPoint[0] = NPtoSP(ptf);
		DrawPoint[1] = NPtoSP(ptt);

		ptf.x = pt_from.x + (link_offset * NumberOfLanes  + mid_offset) * cos(theta-PI/2.0f);
		ptt.x = pt_to.x   + (link_offset * NumberOfLanes  + mid_offset) * cos(theta-PI/2.0f);

		ptf.y = pt_from.y + (link_offset*NumberOfLanes +mid_offset) * sin(theta-PI/2.0f);
		ptt.y = pt_to.y   + (link_offset*NumberOfLanes +mid_offset) * sin(theta-PI/2.0f);

		DrawPoint[3] = NPtoSP(ptf);
		DrawPoint[2] = NPtoSP(ptt);

		pOldPen = pDC->SelectObject(&outlinePen);

		pDC->Polygon(DrawPoint,4);

		for(int i=0;i<NumberOfLanes;i++)
		{
			ptf.x = pt_from.x + (link_offset*i+mid_offset) * cos(theta-PI/2.0f);
			ptt.x = pt_to.x   + (link_offset*i+mid_offset) * cos(theta-PI/2.0f);

			ptf.y = pt_from.y + (link_offset*i+mid_offset) * sin(theta-PI/2.0f);
			ptt.y = pt_to.y   + (link_offset*i+mid_offset) * sin(theta-PI/2.0f);

			pt1 = NPtoSP(ptf);
			pt2 = NPtoSP(ptt);

			if ( 0 == i)
				pOldPen = pDC->SelectObject(&midPen);
			else
				pOldPen = pDC->SelectObject(&lanePen);

			pDC->MoveTo(pt1);
			pDC->LineTo(pt2);

		}
		pDC->SelectObject(pOldPen);
}
// CPage_Node_LaneTurn message handlers

void CPage_Node_LaneTurn::OnLButtonDown(UINT nFlags, CPoint point)
{
	int nRt = FindClickedLink(point);
	int nLinkID,nLaneNum;
	if (nRt == 0 ) 
	{
		m_nInLinkIDSelected = -1;
		m_nInLaneNumSelected = -1;
		m_nOutLinkIDSelected = -1;
		m_nOutLaneNumSelected = -1;
		m_nTurnSelected	= -1;

		CButton *btnAdd =(CButton*) GetDlgItem(IDC_BTN_ADDTURN);
		btnAdd->EnableWindow(FALSE);
		CButton *btnDel =(CButton*) GetDlgItem(IDC_BTN_DELTURN);
		btnDel->EnableWindow(FALSE);
	}
	else if ( nRt > 0 ) // in link
	{
		nLinkID = nRt / 100 ;
		nLaneNum= nRt - nLinkID * 100;
		m_nInLinkIDSelected = nLinkID;
		m_nInLaneNumSelected= nLaneNum;
	}
	else
	{
		nRt *= -1;
		nLinkID = nRt / 100 ;
		nLaneNum= nRt - nLinkID * 100;
		m_nOutLinkIDSelected = nLinkID;
		m_nOutLaneNumSelected= nLaneNum;
	}

	if ( m_nInLinkIDSelected >=0 && m_nOutLinkIDSelected >= 0 && m_nInLaneNumSelected >=0 && m_nOutLaneNumSelected>=0)
	{
		MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
		int nIndex = pNode->GetLaneTurnIndex(m_nInLinkIDSelected,m_nInLaneNumSelected,m_nOutLinkIDSelected,m_nOutLaneNumSelected);
		if ( nIndex >= 0 ) // existing turn
		{
			m_nTurnSelected = nIndex;
			CButton *btnAdd =(CButton*) GetDlgItem(IDC_BTN_ADDTURN);
			btnAdd->EnableWindow(FALSE);
			CButton *btnDel =(CButton*) GetDlgItem(IDC_BTN_DELTURN);
			btnDel->EnableWindow(TRUE);
		}
		else // possible new turn
		{
			m_nTurnSelected = nIndex;
			CButton *btnAdd =(CButton*) GetDlgItem(IDC_BTN_ADDTURN);
			btnAdd->EnableWindow(TRUE);
			CButton *btnDel =(CButton*) GetDlgItem(IDC_BTN_DELTURN);
			btnDel->EnableWindow(FALSE);
		}
	}

	MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
	for (int i=0;i<pNode->LaneTurns.size();i++)
	{
		char str[100];
		m_ListCtrl.GetItemText (i,0,str,20);
		int TurnIndex = atoi(str)-1; // the turn index has been sorted 

		if(TurnIndex == m_nTurnSelected)
			m_ListCtrl.SelectRow (i,true);
		else
			m_ListCtrl.SelectRow(i,false);
	}

	CPropertyPage::OnLButtonDown(nFlags, point);
	Invalidate();

}

void CPage_Node_LaneTurn::OnLvnItemchangedGridlistctrlex(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	while(pos!=NULL)
	{
		int nSelectedRow = m_ListCtrl.GetNextSelectedItem(pos);

		char str[100];
		m_ListCtrl.GetItemText (nSelectedRow,0,str,20);
		m_nTurnSelected = atoi(str)-1;

		m_ListCtrl.GetItemText (nSelectedRow,1,str,20);
		m_nInLinkIDSelected = atoi(str);

		m_ListCtrl.GetItemText (nSelectedRow,2,str,20);
		m_nInLaneNumSelected = atoi(str);

		m_ListCtrl.GetItemText (nSelectedRow,3,str,20);
		m_nOutLinkIDSelected = atoi(str);

		m_ListCtrl.GetItemText (nSelectedRow,4,str,20);
		m_nOutLaneNumSelected = atoi(str);
	}
	if ( m_nTurnSelected >= 0 )
	{
		CButton *btnAdd =(CButton*) GetDlgItem(IDC_BTN_ADDTURN);
		btnAdd->EnableWindow(FALSE);
		CButton *btnDel =(CButton*) GetDlgItem(IDC_BTN_DELTURN);
		btnDel->EnableWindow(TRUE);
	}
	else
	{
		CButton *btnAdd =(CButton*) GetDlgItem(IDC_BTN_ADDTURN);
		btnAdd->EnableWindow(FALSE);
		CButton *btnDel =(CButton*) GetDlgItem(IDC_BTN_DELTURN);
		btnDel->EnableWindow(FALSE);
	}
	Invalidate();
}
/////////////////////////////////////////////////
void CPage_Node_LaneTurn::MCreateDefaultLaneTurns()
{
/*
	OpenLogFile("logoflaneturn.log");
	MLane* pLane = NULL;
	int nNum = 0,i,j;
	int nLinkID;

	DTANode* pNode  = m_pDoc->m_NodeIDMap [m_CurrentNodeID];

	// create lanes
	for (i=0;i<pNode->m_MovementVector.size();i++)
	{
		DTANodeMovement movement = pNode->m_MovementVector[i];
		if (movement.movement_turn != DTA_LeftTurn) continue;

		// 先处理所有的leftturn，给inLink都默认产生pocket lane
		nLinkID = movement.IncomingLinkID;
		MLink* pInLink = GetMLink(nLinkID,1);
		if ( ! pInLink )
		{
			pInLink = new MLink();
			pInLink->m_LinkID = nLinkID;
			pInLink->m_FromNodeID = movement.in_link_from_node_id;
			pInLink->m_ToNodeID   = movement.in_link_to_node_id;
			pInLink->m_NumLanes = m_pDoc->m_LinkNoMap[nLinkID]->m_NumLanes;
			m_node.inLinks.push_back(pInLink);
			pInLink->nIO = 1;
			SerialLink(pInLink);

			// create lanes by default
			nNum = pInLink->m_NumLanes + 1;
			for (j=1 ; j<=nNum;j++)
			{
				pLane = new MLane();
				pInLink->inLanes.push_back(pLane);
				pLane->m_Index = j;
				pLane->m_LinkID = pInLink->m_LinkID;
				pLane->leftTurn = (j==1)?1:0;
				pLane->through  = (j==1)?0:1;
				pLane->rightTurn= (j==nNum)?1:0;
				pLane->m_PocketLength = (j==1)?15:0;
			}			
		}
		nLinkID = movement.OutgoingLinkID;
		MLink* pOutLink = GetMLink(nLinkID,0);
		if ( ! pOutLink )
		{
			pOutLink = new MLink();
			pOutLink->m_LinkID = nLinkID;
			pOutLink->m_FromNodeID = movement.in_link_to_node_id;
			pOutLink->m_ToNodeID   = movement.out_link_to_node_id;
			pOutLink->m_NumLanes = m_pDoc->m_LinkNoMap[nLinkID]->m_NumLanes;
			m_node.outLinks.push_back(pOutLink);
			pOutLink->nIO = 0;
			SerialLink(pOutLink);
			// create lanes by default
			nNum = pOutLink->m_NumLanes;
			for (j=1 ; j<=nNum;j++)
			{
				pLane = new MLane();
				pOutLink->outLanes.push_back(pLane);
				pLane->m_Index = j;
				pLane->m_LinkID = pOutLink->m_LinkID;
				pLane->leftTurn = 1;
				pLane->through  = 1;
				pLane->rightTurn= 1;
				pLane->m_PocketLength = 0;
			}			
		}
	}
	for (i=0;i<pNode->m_MovementVector.size();i++)
	{
		DTANodeMovement movement = pNode->m_MovementVector[i];
		if (movement.movement_turn == DTA_LeftTurn) continue;

		// 再处理所有的非leftturn，inLink不产生pocket lane
		nLinkID = movement.IncomingLinkID;
		MLink* pInLink = GetMLink(nLinkID,1);
		if ( ! pInLink )
		{
			pInLink = new MLink();
			pInLink->m_LinkID = nLinkID;
			pInLink->m_FromNodeID = movement.in_link_from_node_id;
			pInLink->m_ToNodeID   = movement.in_link_to_node_id;
			pInLink->m_NumLanes = m_pDoc->m_LinkNoMap[nLinkID]->m_NumLanes;
			m_node.inLinks.push_back(pInLink);
			pInLink->nIO = 1;
			SerialLink(pInLink);
			// create lanes by default
			nNum = pInLink->m_NumLanes;
			for (j=1 ; j<=nNum;j++)
			{
				pLane = new MLane();
				pInLink->inLanes.push_back(pLane);
				pLane->m_Index = j;
				pLane->m_LinkID = pInLink->m_LinkID;
				pLane->leftTurn = 0;
				pLane->through  = 1;
				pLane->rightTurn= (j==nNum)?1:0;
				pLane->m_PocketLength = 0;
			}			
		}
		nLinkID = movement.OutgoingLinkID;
		MLink* pOutLink = GetMLink(nLinkID,0);
		if ( ! pOutLink )
		{
			pOutLink = new MLink();
			pOutLink->m_LinkID = nLinkID;
			pOutLink->m_FromNodeID = movement.in_link_to_node_id;
			pOutLink->m_ToNodeID   = movement.out_link_to_node_id;
			pOutLink->m_NumLanes = m_pDoc->m_LinkNoMap[nLinkID]->m_NumLanes;
			m_node.outLinks.push_back(pOutLink);
			pOutLink->nIO = 0;
			SerialLink(pOutLink);
			// create lanes by default
			nNum = pOutLink->m_NumLanes;
			for (j=1 ; j<=nNum;j++)
			{
				pLane = new MLane();
				pOutLink->outLanes.push_back(pLane);
				pLane->m_Index = j;
				pLane->m_LinkID = pOutLink->m_LinkID;
				pLane->leftTurn = 1;
				pLane->through  = 1;
				pLane->rightTurn= 1;
				pLane->m_PocketLength = 0;
			}			
		}
	}
	// create turns by default
	for (int j=0;j<pNode->m_MovementVector.size();j++)
	{
		DTANodeMovement movement = pNode->m_MovementVector[j];
		MLink* piLink=NULL, *poLink=NULL;
		piLink = GetMLink(movement.IncomingLinkID,1);
		poLink = GetMLink(movement.OutgoingLinkID,0);
		int niLaneCount,noLaneCount,nTurnCount;
		if (movement.movement_turn == DTA_LeftTurn) 
		{
			niLaneCount = piLink->GetLaneCount(1,3);
			noLaneCount = poLink->GetLaneCount(0);
			nTurnCount  = min(niLaneCount,noLaneCount);
			int nLeftTurnInIndex = piLink->GetLaneCount(1,0);
			for(i=0;i<nTurnCount;i++)
			{
				MLaneTurn *pLaneTurn = new MLaneTurn();
				m_node.LaneTurns.push_back(pLaneTurn);

				pLaneTurn->nFromLinkId = piLink->m_LinkID;
				pLaneTurn->nFromIndex  = nLeftTurnInIndex - i;
				pLaneTurn->nToLinkId   = poLink->m_LinkID;
				pLaneTurn->nToIndex	   = noLaneCount - i;
				pLaneTurn->nRTL		   = 3;
				pLaneTurn->nSCNO       = 0; //GetSCNO(pNode->m_NodeNumber);
				pLaneTurn->nSignalGroupNo = 0; //GetSGNO(pNode->m_NodeNumber,appr,3);
			}
		}
		else if (movement.movement_turn == DTA_Through)
		{
			int nInCount = piLink->GetLaneCount(1,2);
			int nOutCount= poLink->GetLaneCount(0);
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
				m_node.LaneTurns.push_back(pLaneTurn);

				pLaneTurn->nFromLinkId = piLink->m_LinkID;
				pLaneTurn->nFromIndex  = i + nOffsetIndexIn;
				pLaneTurn->nToLinkId   = poLink->m_LinkID;
				pLaneTurn->nToIndex	   = i + nOffsetIndexOut;
				pLaneTurn->nRTL        = 2;
				pLaneTurn->nSCNO       = 0;//GetSCNO(pNode->m_NodeNumber);
				pLaneTurn->nSignalGroupNo = 0;//GetSGNO(pNode->m_NodeNumber,appr,2);
			}
		}
		else if (movement.movement_turn == DTA_RightTurn)
		{
			int nInCount = piLink->GetLaneCount(1,1);
			int nOutCount= poLink->GetLaneCount(0);
			int nTurnCount = min(nInCount,nOutCount);
			for(i=1;i<=nTurnCount;i++)
			{
				MLaneTurn *pLaneTurn = new MLaneTurn();
				m_node.LaneTurns.push_back(pLaneTurn);

				pLaneTurn->nFromLinkId = piLink->m_LinkID;
				pLaneTurn->nFromIndex  = i;
				pLaneTurn->nToLinkId   = poLink->m_LinkID;
				pLaneTurn->nToIndex	   = i;
				pLaneTurn->nRTL		   = 1;
				pLaneTurn->nSCNO       = 0;//GetSCNO(pNode->m_NodeNumber);
				pLaneTurn->nSignalGroupNo = 0;//GetSGNO(pNode->m_NodeNumber,appr,1);
			}
		}
		else
		{
		}
	}
	CloseLogFile();*/
}
MLink* CPage_Node_LaneTurn::GetMLink(int nLinkID, int nInOut)
{
	MLink* pMLink = NULL;
	std::vector<MLink*>::iterator iLink;
	MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
	if ( 1 == nInOut)
	{
		for(iLink = pNode->inLinks.begin(); iLink != pNode->inLinks.end(); iLink++)
		{
			if ( nLinkID == (*iLink)->m_LinkID)
			{
				pMLink = (*iLink);
				break;
			}
		}
	}
	else if ( 0 == nInOut)
	{
		for(iLink =pNode->outLinks.begin(); iLink != pNode->outLinks.end(); iLink++)
		{
			if ( nLinkID == (*iLink)->m_LinkID)
			{
				pMLink = (*iLink);
				break;
			}
		}
	}
	else
	{
	}
	return pMLink;
}
bool CPage_Node_LaneTurn::CloseLogFile()
{
	if (m_logFile.is_open())
		m_logFile.close();
	return true;
	
}
bool CPage_Node_LaneTurn::OpenLogFile(std::string strLogFileName)
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
void CPage_Node_LaneTurn::SerialLink(MLink *pmLink)
{
	GDPoint p1,p2;

	if ( 0 == pmLink->nIO)
	{
		p1  = m_pDoc->m_NodeIDMap[pmLink->m_FromNodeID]->pt;  // current node
		p2  = m_pDoc->m_NodeIDMap[pmLink->m_ToNodeID]->pt;
	}
	else // 1== io
	{
		p2  = m_pDoc->m_NodeIDMap[pmLink->m_FromNodeID]->pt;
		p1  = m_pDoc->m_NodeIDMap[pmLink->m_ToNodeID]->pt;    // current node
	}

	int nNewAngle = m_pDoc->Find_P2P_Angle(p1,p2);
	pmLink->nAngle = nNewAngle;
	bool bInserted = false;

	std::vector<MLink*>::iterator iLink;
	for(iLink = m_seriLink.begin();iLink!=m_seriLink.end();iLink++)
	{
		int nAngle = (*iLink)->nAngle;
		if (nNewAngle <= nAngle)
		{
			if ( (nNewAngle < nAngle) || (nNewAngle == nAngle &&  0 == pmLink->nIO))
			{ // insert before this pos
				m_seriLink.insert(iLink,pmLink);
			}
			else // nNewAngle == nAngle && 1 == pmLink->nIO
			{// insert after this pose
				m_seriLink.insert(iLink+1,pmLink);
			}
			bInserted = true;
			break;
		}
	}
	if (!bInserted)
	{
		m_seriLink.push_back(pmLink);
	}
}
void CPage_Node_LaneTurn::SetDrawPara()
{
	m_Para.nR1			= 30;   //basic offset. should offset lane number of the neighboring link
	m_Para.nR2			= 200;
	m_Para.nR3          = 110;
	m_Para.nR4          = 170;
	m_Para.nRL			= 180;
	m_Para.nR5			= 210;
	m_Para.nCentroidRadius = 6;
	m_Para.nPinPointRadius = 6;
	m_Para.nLaneWidth	= 14;
	m_Para.nLaneLength	= 90;
	m_Para.nALength		= 10;
	m_Para.fAWidth		= 0.2;
	
	m_Para.nAL1	= 0;
	m_Para.nAL2 = 10;
	m_Para.nAL3 = 10;
	m_Para.nAL4 = 30;
	m_Para.fAL1 = 0.2;
	m_Para.fAL2 = 0.4;
	m_Para.fAL3 = 0.5;	

	m_Para.nR180 = 30;
}
void CPage_Node_LaneTurn::InitLinkData()
{
	m_CurrentNodeID =  m_pDoc->m_SelectedNodeID ;
	m_CurrentNodeName = m_pDoc->m_NodeIDtoNameMap [m_CurrentNodeID];
	//DTANode* pNode  = m_pDoc->m_NodeIDMap [m_CurrentNodeID];
	MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
	for(int i=0;i<pNode->inLinks.size();i++)
	{
		MLink* pLink = pNode->inLinks[i];
		LinkRecord *pRecord = new LinkRecord(pLink,1,pLink->nInAngle);
		SerialLinkRecord(pRecord);
	}
	for(int i=0;i<pNode->outLinks.size();i++)
	{
		MLink* pLink = pNode->outLinks[i];
		LinkRecord *pRecord = new LinkRecord(pLink,0,pLink->nOutAngle);
		SerialLinkRecord(pRecord);
	}

	//GDPoint p1,p2;
	//int i,nAngle,nLinkID;

	//for(i=0;i<m_pDoc->m_Network.m_InboundSizeAry[m_CurrentNodeID];i++)
	//{
	//	nLinkID  = m_pDoc->m_Network.m_InboundLinkAry[m_CurrentNodeID][i];
	//	DTALink *pLink = m_pDoc->m_LinkNoMap[nLinkID];
	//	p1  = m_pDoc->m_NodeIDMap[pLink->m_ToNodeID]->pt;    // current node
	//	p2  = m_pDoc->m_NodeIDMap[pLink->m_FromNodeID]->pt;
	//	nAngle = m_pDoc->Find_P2P_Angle(p1,p2);
	//	LinkRecord *pRecord = new LinkRecord(pLink,1,nAngle);
	//	SerialLinkRecord(pRecord);
	//}
	//for(i=0;i<m_pDoc->m_Network.m_OutboundSizeAry[m_CurrentNodeID];i++)
	//{
	//	nLinkID  = m_pDoc->m_Network.m_OutboundLinkAry[m_CurrentNodeID][i];
	//	DTALink *pLink = m_pDoc->m_LinkNoMap[nLinkID];
	//	p1  = m_pDoc->m_NodeIDMap[pLink->m_FromNodeID]->pt;  // current node
	//	p2  = m_pDoc->m_NodeIDMap[pLink->m_ToNodeID]->pt;
	//	nAngle = m_pDoc->Find_P2P_Angle(p1,p2);
	//	LinkRecord *pRecord = new LinkRecord(pLink,0,nAngle);
	//	SerialLinkRecord(pRecord);
	//}
}
void CPage_Node_LaneTurn::SerialLinkRecord(LinkRecord* pRecord)
{
	bool bInserted = false;
	int nNewAngle  = pRecord->nP2PAngle;

	std::vector<LinkRecord*>::iterator iRecord;
	for(iRecord = m_LinkData.begin();iRecord!=m_LinkData.end();iRecord++)
	{
		int nAngle = (*iRecord)->nP2PAngle;
		if (nNewAngle <= nAngle)
		{
			if ( (nNewAngle < nAngle) || (nNewAngle == nAngle &&  0 == pRecord->nIO))
			{ // insert before this pos
				m_LinkData.insert(iRecord,pRecord);
			}
			else // nNewAngle == nAngle && 1 == pRecord->nIO
			{// insert after this pose
				m_LinkData.insert(iRecord+1,pRecord);
			}
			bInserted = true;
			break;
		}
	}
	if (!bInserted)
	{
		m_LinkData.push_back(pRecord);
	}
}
void CPage_Node_LaneTurn::NormalizeAngles()
{
	bool bNormalized = false;
	if (m_LinkData.size() == 8 &&
		m_LinkData[0]->nP2PAngle == m_LinkData[1]->nP2PAngle &&
		m_LinkData[2]->nP2PAngle == m_LinkData[3]->nP2PAngle &&
		m_LinkData[4]->nP2PAngle == m_LinkData[5]->nP2PAngle &&
		m_LinkData[6]->nP2PAngle == m_LinkData[7]->nP2PAngle )
	{
		int nAngle[4];
		for(int i=0;i<4;i++)
		{
			nAngle[i] = m_LinkData[2*i]->nP2PAngle;
		}
		if ( nAngle[0] <= 45 ) // first as the horizontal
		{
			int nClockwiseOff = nAngle[0];
			nAngle[0] = 0;
			nAngle[1] -= nClockwiseOff;
			if (nAngle[1] < 45 ) nAngle[1] = 45;
			if (nAngle[1] > 135) nAngle[1] = 135;
			nAngle[2] = 180;
			nAngle[3] -= nClockwiseOff;
			if (nAngle[3] < 225) nAngle[3] = 225;
			if (nAngle[3] > 315) nAngle[3] = 315;
		}
		else // last as the horizontal
		{
			int nCounterOff = 360 - nAngle[3];
			nAngle[3] = 0;
			nAngle[0] += nCounterOff;
			if (nAngle[0] < 45 ) nAngle[0] = 45;
			if (nAngle[0] > 135) nAngle[0] = 135;
			nAngle[1] = 180;
			nAngle[2] += nCounterOff;
			if (nAngle[2] < 225) nAngle[2] = 225;
			if (nAngle[2] > 315) nAngle[2] = 315;
		}
		for(int i=0;i<4;i++)
		{
			m_LinkData[2*i]->nNormAngle = nAngle[i];
			m_LinkData[2*i]->dNormTheta = nAngle[i] * PI / 180;
			m_LinkData[2*i+1]->nNormAngle = nAngle[i];
			m_LinkData[2*i+1]->dNormTheta = nAngle[i] * PI / 180;
		}
		bNormalized = true;
	}
	else // other types, process pending
	{
	}

	if (!bNormalized)
	{
		for(int i=0;i<m_LinkData.size();i++)
		{
			m_LinkData[i]->nNormAngle = m_LinkData[i]->nP2PAngle;
			m_LinkData[i]->dNormTheta = m_LinkData[i]->dTheta;
		}
	}
	
	return;
}
void CPage_Node_LaneTurn::CalculateOA()
{
	int nLaneWidth = m_Para.nLaneWidth;
	int nTotalWidth = 0;

	for(int i=0;i<m_LinkData.size();i++)
	{
		LinkRecord* p = m_LinkData[i];
		if ( 1 == p->nIO )
		{
			int nNext = i + 1;
			if (nNext == m_LinkData.size())
				nNext = 0;
			LinkRecord* pNext = m_LinkData[nNext];
			if ( 0 == pNext->nIO )
				nTotalWidth = pNext->GetTotalLaneNum() * nLaneWidth;
			else 
				nTotalWidth = 0;
			p->fOA = nTotalWidth / sin(pNext->dNormTheta - p->dNormTheta) ;
			if (p->fOA < 0)
				p->fOA *= -1;
		}
		else
		{
			int nPre = i - 1;
			if (nPre == -1)
				nPre = m_LinkData.size() -1;
			LinkRecord* pPre = m_LinkData[nPre];
			if ( 1 == pPre->nIO )
				nTotalWidth = pPre->GetTotalLaneNum() * nLaneWidth;
			else 
				nTotalWidth = 0;
			p->fOA = nTotalWidth / sin(pPre->dNormTheta - p->dNormTheta) ;
			if (p->fOA < 0)
				p->fOA *= -1;
		}
	}
}
void CPage_Node_LaneTurn::DrawTest(CPaintDC *pDC, CRect PlotRect)
{
	CBrush redBrush = CBrush(RGB(255,0,0));
	CBrush *pOldBrush;

	int nLaneWidth = m_Para.nLaneWidth;
	int nLaneLength= m_Para.nLaneLength;

	for(int i=0;i<m_LinkData.size();i++)
	{
		LinkRecord* p1 = m_LinkData[i];
		LinkRecord* p2 = NULL;
		bool bLaneOnly = false;

		if ( i == m_LinkData.size()-1 )
			p2 = m_LinkData[0];
		else
			p2 = m_LinkData[i+1];

		if ( p1->nNormAngle == p2->nNormAngle ) 
		{
			bLaneOnly = true; // 同一个link的in-n-out
		}

		GDPoint pO,pA,pB,pP,pAA,pBB,pPA,pPB,pABezier,pBBezier;
		int nExtension = 0;
		double dControl = 0.4;
		int nRadius2 = m_Para.nPinPointRadius;

		if ( abs(p2->nNormAngle - p1->nNormAngle) <= 200 &&
			 abs(p2->nNormAngle - p1->nNormAngle) >= 160) // around 180
		{
			int nOffset		= m_Para.nR180;
			int nLaneWidth	= m_Para.nLaneWidth;
			int nLaneLength = m_Para.nLaneLength;
			int nTotalLane,nCo;
			GDPoint pStart,pEnd,pText,pPinPoint,pLaneText,pLanePinPoint;
			
			nTotalLane = p1->GetTotalLaneNum();
			double theta = p1->dNormTheta;
			nCo = 0 == p1->nIO ? -1 : 1 ;
			pA.x	= nCo * nOffset * cos(p1->dNormTheta);
			pA.y	= nCo * nOffset * sin(p1->dNormTheta);
			pAA.x	= nCo * (nOffset + nLaneLength) * cos(p1->dNormTheta);
			pAA.y	= nCo * (nOffset + nLaneLength) * sin(p1->dNormTheta);
			pPinPoint.x	= nCo * (nOffset + nLaneLength*1.1) * cos(p1->dNormTheta);
			pPinPoint.y	= nCo * (nOffset + nLaneLength*1.1) * sin(p1->dNormTheta);
			pText.x	= nCo * (nOffset + nLaneLength*1.3) * cos(p1->dNormTheta);
			pText.y	= nCo * (nOffset + nLaneLength*1.3) * sin(p1->dNormTheta);
			for(int k = 1; k <= nTotalLane ; k++)
			{
				pStart.x = pA.x - k * nLaneWidth * cos(theta-PI/2.0f);
				pStart.y = pA.y - k * nLaneWidth * sin(theta-PI/2.0f);
				pEnd.x = pAA.x - k * nLaneWidth * cos(theta-PI/2.0f);
				pEnd.y = pAA.y - k * nLaneWidth * sin(theta-PI/2.0f);
				pLaneText.x = pText.x - (k-0.5) * nLaneWidth * cos(theta-PI/2.0f);
				pLaneText.y = pText.y - (k-0.5) * nLaneWidth * sin(theta-PI/2.0f);
				pLanePinPoint.x = pPinPoint.x - (k-0.5) * nLaneWidth * cos(theta-PI/2.0f);
				pLanePinPoint.y = pPinPoint.y - (k-0.5) * nLaneWidth * sin(theta-PI/2.0f);
				pDC->MoveTo(NPtoSP(pStart));
				pDC->LineTo(NPtoSP(pEnd));

				CString str;
				str.Format("%d",nTotalLane-k+1);
				CPoint ptText = NPtoSP(pLaneText);
				pDC->TextOutA(ptText.x,ptText.y,str);

				GDPoint lefttop,bottomright;
				CRect centerRect;

				lefttop.x = pLanePinPoint.x - nRadius2;
				lefttop.y = pLanePinPoint.y - nRadius2;
				bottomright.x = pLanePinPoint.x + nRadius2;
				bottomright.y = pLanePinPoint.y + nRadius2;

				centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
				centerRect.NormalizeRect();
				
				if ( (p1->nIO==1 && m_nInLinkIDSelected==p1->nLinkID && m_nInLaneNumSelected==(nTotalLane-k+1)) ||
					 (p1->nIO==0 && m_nOutLinkIDSelected==p1->nLinkID && m_nOutLaneNumSelected==(nTotalLane-k+1)) )
				{
					pOldBrush = pDC->SelectObject(&redBrush);
					pDC->Rectangle(centerRect);
					pDC->SelectObject(pOldBrush);
				}
				else
					pDC->Rectangle(centerRect);
			}

			// draw p2
			nTotalLane = p2->GetTotalLaneNum();
			theta = p2->dNormTheta;
			nCo = 1 == p2->nIO ? -1 : 1 ;
			pA.x	= nCo * nOffset * cos(p2->dNormTheta);
			pA.y	= nCo * nOffset * sin(p2->dNormTheta);
			pAA.x	= nCo * (nOffset + nLaneLength) * cos(p2->dNormTheta);
			pAA.y	= nCo * (nOffset + nLaneLength) * sin(p2->dNormTheta);
			pPinPoint.x	= nCo * (nOffset + nLaneLength*1.1) * cos(p2->dNormTheta);
			pPinPoint.y	= nCo * (nOffset + nLaneLength*1.1) * sin(p2->dNormTheta);
			pText.x	= nCo * (nOffset + nLaneLength*1.3) * cos(p2->dNormTheta);
			pText.y	= nCo * (nOffset + nLaneLength*1.3) * sin(p2->dNormTheta);
			for(int k = 1; k <= nTotalLane ; k++)
			{
				pStart.x = pA.x + k * nLaneWidth * cos(theta-PI/2.0f);
				pStart.y = pA.y + k * nLaneWidth * sin(theta-PI/2.0f);
				pEnd.x = pAA.x + k * nLaneWidth * cos(theta-PI/2.0f);
				pEnd.y = pAA.y + k * nLaneWidth * sin(theta-PI/2.0f);
				pLaneText.x = pText.x + (k-0.5) * nLaneWidth * cos(theta-PI/2.0f);
				pLaneText.y = pText.y + (k-0.5) * nLaneWidth * sin(theta-PI/2.0f);
				pLanePinPoint.x = pPinPoint.x + (k-0.5) * nLaneWidth * cos(theta-PI/2.0f);
				pLanePinPoint.y = pPinPoint.y + (k-0.5) * nLaneWidth * sin(theta-PI/2.0f);
				pDC->MoveTo(NPtoSP(pStart));
				pDC->LineTo(NPtoSP(pEnd));

				CString str;
				str.Format("%d",nTotalLane-k+1);
				CPoint ptText = NPtoSP(pLaneText);
				pDC->TextOutA(ptText.x,ptText.y,str);

				GDPoint lefttop,bottomright;
				CRect centerRect;

				lefttop.x = pLanePinPoint.x - nRadius2;
				lefttop.y = pLanePinPoint.y - nRadius2;
				bottomright.x = pLanePinPoint.x + nRadius2;
				bottomright.y = pLanePinPoint.y + nRadius2;

				centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
				centerRect.NormalizeRect();
				
				if ( (p2->nIO==1 && m_nInLinkIDSelected==p2->nLinkID && m_nInLaneNumSelected==(nTotalLane-k+1)) ||
					 (p2->nIO==0 && m_nOutLinkIDSelected==p2->nLinkID && m_nOutLaneNumSelected==(nTotalLane-k+1)) )
				{
					pOldBrush = pDC->SelectObject(&redBrush);
					pDC->Ellipse(centerRect);
					pDC->SelectObject(pOldBrush);
				}
				else
					pDC->Ellipse(centerRect);
			}

			
		}
		else // less than 
		{
			float fOA1 = p2->nIO == 0 ? p1->fOA : 0.0;
			float fOA2 = p1->nIO == 1 ? p2->fOA : 0.0;	

			pO.x = 0.0; pO.y = 0.0;
			pA.x = fOA1 * cos(p1->dNormTheta);
			pA.y = fOA1 * sin(p1->dNormTheta);
			nExtension = fOA1 + nLaneLength;
			pAA.x = nExtension * cos(p1->dNormTheta);
			pAA.y = nExtension * sin(p1->dNormTheta);
			pB.x = fOA2 * cos(p2->dNormTheta);
			pB.y = fOA2 * sin(p2->dNormTheta);
			nExtension = fOA2 + nLaneLength;
			pBB.x = nExtension * cos(p2->dNormTheta);
			pBB.y = nExtension * sin(p2->dNormTheta);
			pP.x = pA.x + pB.x;
			pP.y = pA.y + pB.y;
			pPA.x = pP.x + nLaneLength * cos(p1->dNormTheta);
			pPA.y = pP.y + nLaneLength * sin(p1->dNormTheta);
			pPB.x = pP.x + nLaneLength * cos(p2->dNormTheta);
			pPB.y = pP.y + nLaneLength * sin(p2->dNormTheta);
			pABezier.x = pP.x + dControl * nLaneLength * cos(p1->dNormTheta);
			pABezier.y = pP.y + dControl * nLaneLength * sin(p1->dNormTheta);
			pBBezier.x = pP.x + dControl * nLaneLength * cos(p2->dNormTheta);
			pBBezier.y = pP.y + dControl * nLaneLength * sin(p2->dNormTheta);

			if (p1->nIO == 1)
			{
				int nTotalLane = p1->GetTotalLaneNum();
				for(int k =1 ; k <= nTotalLane ; k++)
				{
					GDPoint pStart,pEnd,pText,pPinPoint,pLaneStart,pLaneEnd,pLaneText,pLanePinPoint;
					float fStart = fOA1 + dControl * nLaneLength;
					pStart.x = fStart * cos(p1->dNormTheta);
					pStart.y = fStart * sin(p1->dNormTheta);
					pLaneStart.x   = pStart.x + k * pB.x / nTotalLane;
					pLaneStart.y   = pStart.y + k * pB.y / nTotalLane;			
					pLaneEnd.x   = pAA.x + k * pB.x / nTotalLane;
					pLaneEnd.y   = pAA.y + k * pB.y / nTotalLane;	
					float fText = fOA1 + 1.3 * nLaneLength;
					pText.x = fText * cos(p1->dNormTheta);
					pText.y = fText * sin(p1->dNormTheta);
					pLaneText.x   = pText.x + (k-0.5) * pB.x / nTotalLane;
					pLaneText.y   = pText.y + (k-0.5) * pB.y / nTotalLane;
					float fPinPoint = fOA1 + 1.1 * nLaneLength;
					pPinPoint.x = fPinPoint * cos(p1->dNormTheta);
					pPinPoint.y = fPinPoint * sin(p1->dNormTheta);
					pLanePinPoint.x   = pPinPoint.x + (k-0.5) * pB.x / nTotalLane;
					pLanePinPoint.y   = pPinPoint.y + (k-0.5) * pB.y / nTotalLane;

					pDC->MoveTo(NPtoSP(pLaneStart));
					pDC->LineTo(NPtoSP(pLaneEnd));
					CString str;
					str.Format("%d",nTotalLane-k+1);
					CPoint ptText = NPtoSP(pLaneText);
					pDC->TextOutA(ptText.x,ptText.y,str);

					GDPoint lefttop,bottomright;
					CRect centerRect;

					lefttop.x = pLanePinPoint.x - nRadius2;
					lefttop.y = pLanePinPoint.y - nRadius2;
					bottomright.x = pLanePinPoint.x + nRadius2;
					bottomright.y = pLanePinPoint.y + nRadius2;

					centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
					centerRect.NormalizeRect();
					
					if ( m_nInLinkIDSelected==p1->nLinkID && m_nInLaneNumSelected==(nTotalLane-k+1) )
					{
						pOldBrush = pDC->SelectObject(&redBrush);
						pDC->Rectangle(centerRect);
						pDC->SelectObject(pOldBrush);
					}
					else
						pDC->Rectangle(centerRect);
				}
			}
			if (p2->nIO == 0)
			{
				int nTotalLane = p2->GetTotalLaneNum();
				for(int k =1 ; k <= nTotalLane ; k++)
				{
					GDPoint pStart,pEnd,pText,pPinPoint,pLaneStart,pLaneEnd,pLaneText,pLanePinPoint;
					float fStart = fOA2 + dControl * nLaneLength;
					pStart.x = fStart * cos(p2->dNormTheta);
					pStart.y = fStart * sin(p2->dNormTheta);
					pLaneStart.x   = pStart.x + k * pA.x / nTotalLane;
					pLaneStart.y   = pStart.y + k * pA.y / nTotalLane;			
					pLaneEnd.x   = pBB.x + k * pA.x / nTotalLane;
					pLaneEnd.y   = pBB.y + k * pA.y / nTotalLane;	
					float fText = fOA2 + 1.3 * nLaneLength;
					pText.x = fText * cos(p2->dNormTheta);
					pText.y = fText * sin(p2->dNormTheta);
					pLaneText.x   = pText.x + (k-0.5) * pA.x / nTotalLane;
					pLaneText.y   = pText.y + (k-0.5) * pA.y / nTotalLane;
					float fPinPoint = fOA2 + 1.1 * nLaneLength;
					pPinPoint.x = fPinPoint * cos(p2->dNormTheta);
					pPinPoint.y = fPinPoint * sin(p2->dNormTheta);
					pLanePinPoint.x   = pPinPoint.x + (k-0.5) * pA.x / nTotalLane;
					pLanePinPoint.y   = pPinPoint.y + (k-0.5) * pA.y / nTotalLane;

					pDC->MoveTo(NPtoSP(pLaneStart));
					pDC->LineTo(NPtoSP(pLaneEnd));
					CString str;
					str.Format("%d",nTotalLane-k+1);
					CPoint ptText = NPtoSP(pLaneText);
					pDC->TextOutA(ptText.x,ptText.y,str);

					GDPoint lefttop,bottomright;
					CRect centerRect;

					lefttop.x = pLanePinPoint.x - nRadius2;
					lefttop.y = pLanePinPoint.y - nRadius2;
					bottomright.x = pLanePinPoint.x + nRadius2;
					bottomright.y = pLanePinPoint.y + nRadius2;

					centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
					centerRect.NormalizeRect();
					
					if ( m_nOutLinkIDSelected==p2->nLinkID && m_nOutLaneNumSelected==(nTotalLane-k+1) )
					{
						pOldBrush = pDC->SelectObject(&redBrush);
						pDC->Ellipse(centerRect);
						pDC->SelectObject(pOldBrush);
					}
					else
						pDC->Ellipse(centerRect);
				}
			}

			// draw lanes seperators of one link
			if ( bLaneOnly ) continue;

			CPoint pCurve[4];
			pCurve[0] = NPtoSP(pABezier);
			pCurve[1] = NPtoSP(pP);
			pCurve[2] = NPtoSP(pP);
			pCurve[3] = NPtoSP(pBBezier);

			pDC->MoveTo(NPtoSP(pPA));
			pDC->LineTo(NPtoSP(pABezier));
			pDC->PolyBezier(pCurve,4);
			pDC->MoveTo(NPtoSP(pBBezier));
			pDC->LineTo(NPtoSP(pPB));
		}
	}
}
void CPage_Node_LaneTurn::DrawCentroidRadicalLines(CPaintDC *pDC, CRect PlotRect,bool bOriginAngle/*=true*/)
{
	CPen blackPen(PS_SOLID,0,RGB(0,0,0));
	CPen greyPen(PS_SOLID,2,RGB(128,128,128));
	CPen lanePen(PS_SOLID,1,RGB(0,0,0));
	CPen laneSelectedPen(PS_SOLID,2,RGB(255,0,0));

	CBrush  greyBrush(RGB(128,128,128)); 
	CBrush  whiteBrush(RGB(255,255,255)); 
	CBrush  redBrush(RGB(255,0,0)); 

	pDC->SetBkMode(TRANSPARENT);
	CPen *pOldPen = pDC->SelectObject(&blackPen);
	CBrush* pOldBrush = pDC->SelectObject(&whiteBrush);
	pDC->Rectangle(PlotRect);

	GDPoint cntPoint,lefttop,bottomright;
	CRect centerRect;

	cntPoint.x = 0.0;
	cntPoint.y = 0.0;
	int nRadius2 = m_Para.nCentroidRadius;
	lefttop.x = cntPoint.x - nRadius2;
	lefttop.y = cntPoint.y - nRadius2;
	bottomright.x = cntPoint.x + nRadius2;
	bottomright.y = cntPoint.y + nRadius2;
	
	pDC->SelectObject(&greyPen);
	pDC->SelectObject(&greyBrush);
	centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
	pDC->Ellipse(centerRect);

	for(int i=0;i<m_LinkData.size();i++)
	{
		LinkRecord *pRecord = m_LinkData[i];
		pDC->SelectObject(&greyPen);
		DrawRadicalLine(pDC,pRecord,bOriginAngle);
		//pDC->SelectObject(&lanePen);
		//pDC->SelectObject(&greyBrush);
		//DrawLanes(pDC,pRecord,bOriginAngle);
		//pDC->SelectObject(&blackPen);
		//if (pRecord->nLinkID == m_nSelectedLinkID)
		//	pDC->SelectObject(&redBrush);
		//DrawLinkPinPoint(pDC,pRecord,bOriginAngle);
	}
	// draw selected link pin point

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
	
}
void CPage_Node_LaneTurn::DrawRadicalLine(CPaintDC *pDC, LinkRecord *pRecord,bool bOriginAngle /* = true*/)
{
	GDPoint p1,p2;
	int nR1 = m_Para.nR1;
	int nR2 = m_Para.nR2;
	double theta = bOriginAngle ? pRecord->dTheta : pRecord->dNormTheta;

	p1.x	= nR1 * cos(theta);
	p1.y	= nR1 * sin(theta);
	
	p2.x	= nR2 * cos(theta);
	p2.y	= nR2 * sin(theta);

	pDC->MoveTo(NPtoSP(p1));
	pDC->LineTo(NPtoSP(p2));
}
void CPage_Node_LaneTurn::CreateGraphics()
{
/*	int nLaneWidth = m_Para.nLaneWidth;
	int nLaneLength= m_Para.nLaneLength;

	for(int i=0;i<m_LinkData.size();i++)
	{
		LinkRecord* p1 = m_LinkData[i];
		LinkRecord* p2 = NULL;
		bool bLaneOnly = false;

		if ( i == m_LinkData.size()-1 )
			p2 = m_LinkData[0];
		else
			p2 = m_LinkData[i+1];

		if ( p1->nNormAngle == p2->nNormAngle ) 
		{
			bLaneOnly = true; // 同一个link的in-n-out
		}

		GDPoint pO,pA,pB,pP,pAA,pBB,pPA,pPB,pABezier,pBBezier;
		int nExtension = 0;
		double dControl = 0.4;
		int nRadius2 = m_Para.nPinPointRadius;

		float fOA1 = p2->nIO == 0 ? p1->fOA : 0.0;
		float fOA2 = p1->nIO == 1 ? p2->fOA : 0.0;	
		if ( abs(p2->nNormAngle - p1->nNormAngle) <= 10)
		{
			fOA1 = 0;
			fOA2 = 0;
		}

		pO.x = 0.0; pO.y = 0.0;
		pA.x = fOA1 * cos(p1->dNormTheta);
		pA.y = fOA1 * sin(p1->dNormTheta);
		nExtension = fOA1 + nLaneLength;
		pAA.x = nExtension * cos(p1->dNormTheta);
		pAA.y = nExtension * sin(p1->dNormTheta);
		pB.x = fOA2 * cos(p2->dNormTheta);
		pB.y = fOA2 * sin(p2->dNormTheta);
		nExtension = fOA2 + nLaneLength;
		pBB.x = nExtension * cos(p2->dNormTheta);
		pBB.y = nExtension * sin(p2->dNormTheta);
		pP.x = pA.x + pB.x;
		pP.y = pA.y + pB.y;
		pPA.x = pP.x + nLaneLength * cos(p1->dNormTheta);
		pPA.y = pP.y + nLaneLength * sin(p1->dNormTheta);
		pPB.x = pP.x + nLaneLength * cos(p2->dNormTheta);
		pPB.y = pP.y + nLaneLength * sin(p2->dNormTheta);
		pABezier.x = pP.x + dControl * nLaneLength * cos(p1->dNormTheta);
		pABezier.y = pP.y + dControl * nLaneLength * sin(p1->dNormTheta);
		pBBezier.x = pP.x + dControl * nLaneLength * cos(p2->dNormTheta);
		pBBezier.y = pP.y + dControl * nLaneLength * sin(p2->dNormTheta);

		if (p1->nIO == 1)
		{
			int nTotalLane = p1->GetTotalLaneNum();
			for(int k =1 ; k <= nTotalLane ; k++)
			{
				GDPoint pStart,pEnd,pText,pPinPoint,pLaneStart,pLaneEnd,pLaneText,pLanePinPoint;
				float fStart = fOA1 + dControl * nLaneLength;
				pStart.x = fStart * cos(p1->dNormTheta);
				pStart.y = fStart * sin(p1->dNormTheta);
				pLaneStart.x   = pStart.x + k * pB.x / nTotalLane;
				pLaneStart.y   = pStart.y + k * pB.y / nTotalLane;			
				pLaneEnd.x   = pAA.x + k * pB.x / nTotalLane;
				pLaneEnd.y   = pAA.y + k * pB.y / nTotalLane;	
				float fText = fOA1 + 1.3 * nLaneLength;
				pText.x = fText * cos(p1->dNormTheta);
				pText.y = fText * sin(p1->dNormTheta);
				pLaneText.x   = pText.x + (k-0.5) * pB.x / nTotalLane;
				pLaneText.y   = pText.y + (k-0.5) * pB.y / nTotalLane;
				float fPinPoint = fOA1 + 1.1 * nLaneLength;
				pPinPoint.x = fPinPoint * cos(p1->dNormTheta);
				pPinPoint.y = fPinPoint * sin(p1->dNormTheta);
				pLanePinPoint.x   = pPinPoint.x + (k-0.5) * pB.x / nTotalLane;
				pLanePinPoint.y   = pPinPoint.y + (k-0.5) * pB.y / nTotalLane;

				pDC->MoveTo(NPtoSP(pLaneStart));
				pDC->LineTo(NPtoSP(pLaneEnd));
				CString str;
				str.Format("%d",nTotalLane-k+1);
				CPoint ptText = NPtoSP(pLaneText);
				pDC->TextOutA(ptText.x,ptText.y,str);

				GDPoint lefttop,bottomright;
				CRect centerRect;

				lefttop.x = pLanePinPoint.x - nRadius2;
				lefttop.y = pLanePinPoint.y - nRadius2;
				bottomright.x = pLanePinPoint.x + nRadius2;
				bottomright.y = pLanePinPoint.y + nRadius2;

				centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
				centerRect.NormalizeRect();
				
				pDC->Rectangle(centerRect);
			}
		}
		if (p2->nIO == 0)
		{
			int nTotalLane = p2->GetTotalLaneNum();
			for(int k =1 ; k <= nTotalLane ; k++)
			{
				GDPoint pStart,pEnd,pText,pPinPoint,pLaneStart,pLaneEnd,pLaneText,pLanePinPoint;
				float fStart = fOA2 + dControl * nLaneLength;
				pStart.x = fStart * cos(p2->dNormTheta);
				pStart.y = fStart * sin(p2->dNormTheta);
				pLaneStart.x   = pStart.x + k * pA.x / nTotalLane;
				pLaneStart.y   = pStart.y + k * pA.y / nTotalLane;			
				pLaneEnd.x   = pBB.x + k * pA.x / nTotalLane;
				pLaneEnd.y   = pBB.y + k * pA.y / nTotalLane;	
				float fText = fOA2 + 1.3 * nLaneLength;
				pText.x = fText * cos(p2->dNormTheta);
				pText.y = fText * sin(p2->dNormTheta);
				pLaneText.x   = pText.x + (k-0.5) * pA.x / nTotalLane;
				pLaneText.y   = pText.y + (k-0.5) * pA.y / nTotalLane;
				float fPinPoint = fOA2 + 1.1 * nLaneLength;
				pPinPoint.x = fPinPoint * cos(p2->dNormTheta);
				pPinPoint.y = fPinPoint * sin(p2->dNormTheta);
				pLanePinPoint.x   = pPinPoint.x + (k-0.5) * pA.x / nTotalLane;
				pLanePinPoint.y   = pPinPoint.y + (k-0.5) * pA.y / nTotalLane;

				pDC->MoveTo(NPtoSP(pLaneStart));
				pDC->LineTo(NPtoSP(pLaneEnd));
				CString str;
				str.Format("%d",nTotalLane-k+1);
				CPoint ptText = NPtoSP(pLaneText);
				pDC->TextOutA(ptText.x,ptText.y,str);

				GDPoint lefttop,bottomright;
				CRect centerRect;

				lefttop.x = pLanePinPoint.x - nRadius2;
				lefttop.y = pLanePinPoint.y - nRadius2;
				bottomright.x = pLanePinPoint.x + nRadius2;
				bottomright.y = pLanePinPoint.y + nRadius2;

				centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
				centerRect.NormalizeRect();
				
				pDC->Ellipse(centerRect);
			}
		}

		// draw lanes seperators of one link
		if ( bLaneOnly ) continue;

		CPoint pCurve[4];
		pCurve[0] = NPtoSP(pABezier);
		pCurve[1] = NPtoSP(pP);
		pCurve[2] = NPtoSP(pP);
		pCurve[3] = NPtoSP(pBBezier);

		pDC->MoveTo(NPtoSP(pPA));
		pDC->LineTo(NPtoSP(pABezier));
		pDC->PolyBezier(pCurve,4);
		pDC->MoveTo(NPtoSP(pBBezier));
		pDC->LineTo(NPtoSP(pPB));
	}
*/
}
int CPage_Node_LaneTurn::FindClickedLink(CPoint pt)
{
	int nRtLinkID=0,nRtLaneNum=0;

	int nLaneWidth = m_Para.nLaneWidth;
	int nLaneLength= m_Para.nLaneLength;

	for(int i=0;i<m_LinkData.size();i++)
	{
		LinkRecord* p1 = m_LinkData[i];
		LinkRecord* p2 = NULL;
		bool bLaneOnly = false;

		if ( i == m_LinkData.size()-1 )
			p2 = m_LinkData[0];
		else
			p2 = m_LinkData[i+1];

		if ( p1->nNormAngle == p2->nNormAngle ) 
		{
			bLaneOnly = true; // 同一个link的in-n-out
		}

		GDPoint pO,pA,pB,pP,pAA,pBB,pPA,pPB,pABezier,pBBezier;
		int nExtension = 0;
		double dControl = 0.4;
		int nRadius2 = m_Para.nPinPointRadius;

		if ( abs(p2->nNormAngle - p1->nNormAngle) <= 200 &&
			 abs(p2->nNormAngle - p1->nNormAngle) >= 160) // around 180
		{
			int nOffset		= m_Para.nR180;
			int nLaneWidth	= m_Para.nLaneWidth;
			int nLaneLength = m_Para.nLaneLength;
			int nTotalLane,nCo;
			GDPoint pStart,pEnd,pText,pPinPoint,pLaneText,pLanePinPoint;
			
			nTotalLane = p1->GetTotalLaneNum();
			double theta = p1->dNormTheta;
			nCo = 0 == p1->nIO ? -1 : 1 ;
			pA.x	= nCo * nOffset * cos(p1->dNormTheta);
			pA.y	= nCo * nOffset * sin(p1->dNormTheta);
			pAA.x	= nCo * (nOffset + nLaneLength) * cos(p1->dNormTheta);
			pAA.y	= nCo * (nOffset + nLaneLength) * sin(p1->dNormTheta);
			pPinPoint.x	= nCo * (nOffset + nLaneLength*1.1) * cos(p1->dNormTheta);
			pPinPoint.y	= nCo * (nOffset + nLaneLength*1.1) * sin(p1->dNormTheta);
			pText.x	= nCo * (nOffset + nLaneLength*1.3) * cos(p1->dNormTheta);
			pText.y	= nCo * (nOffset + nLaneLength*1.3) * sin(p1->dNormTheta);
			for(int k = 1; k <= nTotalLane ; k++)
			{
				pStart.x = pA.x - k * nLaneWidth * cos(theta-PI/2.0f);
				pStart.y = pA.y - k * nLaneWidth * sin(theta-PI/2.0f);
				pEnd.x = pAA.x - k * nLaneWidth * cos(theta-PI/2.0f);
				pEnd.y = pAA.y - k * nLaneWidth * sin(theta-PI/2.0f);
				pLaneText.x = pText.x - (k-0.5) * nLaneWidth * cos(theta-PI/2.0f);
				pLaneText.y = pText.y - (k-0.5) * nLaneWidth * sin(theta-PI/2.0f);
				pLanePinPoint.x = pPinPoint.x - (k-0.5) * nLaneWidth * cos(theta-PI/2.0f);
				pLanePinPoint.y = pPinPoint.y - (k-0.5) * nLaneWidth * sin(theta-PI/2.0f);
				//pDC->MoveTo(NPtoSP(pStart));
				//pDC->LineTo(NPtoSP(pEnd));

				CString str;
				str.Format("%d",nTotalLane-k+1);
				CPoint ptText = NPtoSP(pLaneText);
				//pDC->TextOutA(ptText.x,ptText.y,str);

				GDPoint lefttop,bottomright;
				CRect centerRect;

				lefttop.x = pLanePinPoint.x - nRadius2;
				lefttop.y = pLanePinPoint.y - nRadius2;
				bottomright.x = pLanePinPoint.x + nRadius2;
				bottomright.y = pLanePinPoint.y + nRadius2;

				centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
				centerRect.NormalizeRect();
				
				//pDC->Rectangle(centerRect);
				if ( centerRect.PtInRect(pt) )
				{
					nRtLinkID = p1->nLinkID;
					nRtLaneNum= nTotalLane-k+1;
					int IorO = p1->nIO==1?1:-1;
					return IorO * (nRtLinkID * 100 + nRtLaneNum);
				}
			}

			// draw p2
			nTotalLane = p2->GetTotalLaneNum();
			theta = p2->dNormTheta;
			nCo = 1 == p2->nIO ? -1 : 1 ;
			pA.x	= nCo * nOffset * cos(p2->dNormTheta);
			pA.y	= nCo * nOffset * sin(p2->dNormTheta);
			pAA.x	= nCo * (nOffset + nLaneLength) * cos(p2->dNormTheta);
			pAA.y	= nCo * (nOffset + nLaneLength) * sin(p2->dNormTheta);
			pPinPoint.x	= nCo * (nOffset + nLaneLength*1.1) * cos(p2->dNormTheta);
			pPinPoint.y	= nCo * (nOffset + nLaneLength*1.1) * sin(p2->dNormTheta);
			pText.x	= nCo * (nOffset + nLaneLength*1.3) * cos(p2->dNormTheta);
			pText.y	= nCo * (nOffset + nLaneLength*1.3) * sin(p2->dNormTheta);
			for(int k = 1; k <= nTotalLane ; k++)
			{
				pStart.x = pA.x + k * nLaneWidth * cos(theta-PI/2.0f);
				pStart.y = pA.y + k * nLaneWidth * sin(theta-PI/2.0f);
				pEnd.x = pAA.x + k * nLaneWidth * cos(theta-PI/2.0f);
				pEnd.y = pAA.y + k * nLaneWidth * sin(theta-PI/2.0f);
				pLaneText.x = pText.x + (k-0.5) * nLaneWidth * cos(theta-PI/2.0f);
				pLaneText.y = pText.y + (k-0.5) * nLaneWidth * sin(theta-PI/2.0f);
				pLanePinPoint.x = pPinPoint.x + (k-0.5) * nLaneWidth * cos(theta-PI/2.0f);
				pLanePinPoint.y = pPinPoint.y + (k-0.5) * nLaneWidth * sin(theta-PI/2.0f);
				//pDC->MoveTo(NPtoSP(pStart));
				//pDC->LineTo(NPtoSP(pEnd));

				CString str;
				str.Format("%d",nTotalLane-k+1);
				CPoint ptText = NPtoSP(pLaneText);
				//pDC->TextOutA(ptText.x,ptText.y,str);

				GDPoint lefttop,bottomright;
				CRect centerRect;

				lefttop.x = pLanePinPoint.x - nRadius2;
				lefttop.y = pLanePinPoint.y - nRadius2;
				bottomright.x = pLanePinPoint.x + nRadius2;
				bottomright.y = pLanePinPoint.y + nRadius2;

				centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
				centerRect.NormalizeRect();
				
				//pDC->Ellipse(centerRect);
				if ( centerRect.PtInRect(pt) )
				{
					nRtLinkID = p2->nLinkID;
					nRtLaneNum= nTotalLane-k+1;
					int IorO = p2->nIO==1?1:-1;
					return IorO * (nRtLinkID * 100 + nRtLaneNum);
				}
			}

			
		}
		else // less than 
		{
			float fOA1 = p2->nIO == 0 ? p1->fOA : 0.0;
			float fOA2 = p1->nIO == 1 ? p2->fOA : 0.0;	

			pO.x = 0.0; pO.y = 0.0;
			pA.x = fOA1 * cos(p1->dNormTheta);
			pA.y = fOA1 * sin(p1->dNormTheta);
			nExtension = fOA1 + nLaneLength;
			pAA.x = nExtension * cos(p1->dNormTheta);
			pAA.y = nExtension * sin(p1->dNormTheta);
			pB.x = fOA2 * cos(p2->dNormTheta);
			pB.y = fOA2 * sin(p2->dNormTheta);
			nExtension = fOA2 + nLaneLength;
			pBB.x = nExtension * cos(p2->dNormTheta);
			pBB.y = nExtension * sin(p2->dNormTheta);
			pP.x = pA.x + pB.x;
			pP.y = pA.y + pB.y;
			pPA.x = pP.x + nLaneLength * cos(p1->dNormTheta);
			pPA.y = pP.y + nLaneLength * sin(p1->dNormTheta);
			pPB.x = pP.x + nLaneLength * cos(p2->dNormTheta);
			pPB.y = pP.y + nLaneLength * sin(p2->dNormTheta);
			pABezier.x = pP.x + dControl * nLaneLength * cos(p1->dNormTheta);
			pABezier.y = pP.y + dControl * nLaneLength * sin(p1->dNormTheta);
			pBBezier.x = pP.x + dControl * nLaneLength * cos(p2->dNormTheta);
			pBBezier.y = pP.y + dControl * nLaneLength * sin(p2->dNormTheta);

			if (p1->nIO == 1)
			{
				int nTotalLane = p1->GetTotalLaneNum();
				for(int k =1 ; k <= nTotalLane ; k++)
				{
					GDPoint pStart,pEnd,pText,pPinPoint,pLaneStart,pLaneEnd,pLaneText,pLanePinPoint;
					float fStart = fOA1 + dControl * nLaneLength;
					pStart.x = fStart * cos(p1->dNormTheta);
					pStart.y = fStart * sin(p1->dNormTheta);
					pLaneStart.x   = pStart.x + k * pB.x / nTotalLane;
					pLaneStart.y   = pStart.y + k * pB.y / nTotalLane;			
					pLaneEnd.x   = pAA.x + k * pB.x / nTotalLane;
					pLaneEnd.y   = pAA.y + k * pB.y / nTotalLane;	
					float fText = fOA1 + 1.3 * nLaneLength;
					pText.x = fText * cos(p1->dNormTheta);
					pText.y = fText * sin(p1->dNormTheta);
					pLaneText.x   = pText.x + (k-0.5) * pB.x / nTotalLane;
					pLaneText.y   = pText.y + (k-0.5) * pB.y / nTotalLane;
					float fPinPoint = fOA1 + 1.1 * nLaneLength;
					pPinPoint.x = fPinPoint * cos(p1->dNormTheta);
					pPinPoint.y = fPinPoint * sin(p1->dNormTheta);
					pLanePinPoint.x   = pPinPoint.x + (k-0.5) * pB.x / nTotalLane;
					pLanePinPoint.y   = pPinPoint.y + (k-0.5) * pB.y / nTotalLane;

					//pDC->MoveTo(NPtoSP(pLaneStart));
					//pDC->LineTo(NPtoSP(pLaneEnd));
					CString str;
					str.Format("%d",nTotalLane-k+1);
					CPoint ptText = NPtoSP(pLaneText);
					//pDC->TextOutA(ptText.x,ptText.y,str);

					GDPoint lefttop,bottomright;
					CRect centerRect;

					lefttop.x = pLanePinPoint.x - nRadius2;
					lefttop.y = pLanePinPoint.y - nRadius2;
					bottomright.x = pLanePinPoint.x + nRadius2;
					bottomright.y = pLanePinPoint.y + nRadius2;

					centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
					centerRect.NormalizeRect();
					
					//pDC->Rectangle(centerRect);
					if ( centerRect.PtInRect(pt) )
					{
						nRtLinkID = p1->nLinkID;
						nRtLaneNum= nTotalLane-k+1;
						int IorO = p1->nIO==1?1:-1;
						return IorO * (nRtLinkID * 100 + nRtLaneNum);
					}
				}
			}
			if (p2->nIO == 0)
			{
				int nTotalLane = p2->GetTotalLaneNum();
				for(int k =1 ; k <= nTotalLane ; k++)
				{
					GDPoint pStart,pEnd,pText,pPinPoint,pLaneStart,pLaneEnd,pLaneText,pLanePinPoint;
					float fStart = fOA2 + dControl * nLaneLength;
					pStart.x = fStart * cos(p2->dNormTheta);
					pStart.y = fStart * sin(p2->dNormTheta);
					pLaneStart.x   = pStart.x + k * pA.x / nTotalLane;
					pLaneStart.y   = pStart.y + k * pA.y / nTotalLane;			
					pLaneEnd.x   = pBB.x + k * pA.x / nTotalLane;
					pLaneEnd.y   = pBB.y + k * pA.y / nTotalLane;	
					float fText = fOA2 + 1.3 * nLaneLength;
					pText.x = fText * cos(p2->dNormTheta);
					pText.y = fText * sin(p2->dNormTheta);
					pLaneText.x   = pText.x + (k-0.5) * pA.x / nTotalLane;
					pLaneText.y   = pText.y + (k-0.5) * pA.y / nTotalLane;
					float fPinPoint = fOA2 + 1.1 * nLaneLength;
					pPinPoint.x = fPinPoint * cos(p2->dNormTheta);
					pPinPoint.y = fPinPoint * sin(p2->dNormTheta);
					pLanePinPoint.x   = pPinPoint.x + (k-0.5) * pA.x / nTotalLane;
					pLanePinPoint.y   = pPinPoint.y + (k-0.5) * pA.y / nTotalLane;

					//pDC->MoveTo(NPtoSP(pLaneStart));
					//pDC->LineTo(NPtoSP(pLaneEnd));
					CString str;
					str.Format("%d",nTotalLane-k+1);
					CPoint ptText = NPtoSP(pLaneText);
					//pDC->TextOutA(ptText.x,ptText.y,str);

					GDPoint lefttop,bottomright;
					CRect centerRect;

					lefttop.x = pLanePinPoint.x - nRadius2;
					lefttop.y = pLanePinPoint.y - nRadius2;
					bottomright.x = pLanePinPoint.x + nRadius2;
					bottomright.y = pLanePinPoint.y + nRadius2;

					centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
					centerRect.NormalizeRect();
					
					//pDC->Ellipse(centerRect);
					if ( centerRect.PtInRect(pt) )
					{
						nRtLinkID = p2->nLinkID;
						nRtLaneNum= nTotalLane-k+1;
						int IorO = p2->nIO==1?1:-1;
						return IorO * (nRtLinkID * 100 + nRtLaneNum);
					}
				}
			}
		}
	}
	return 0;
}
void CPage_Node_LaneTurn::OnBnClickedBtnDelturn()
{
	if(m_nTurnSelected >=0)
	{
		MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
		MLaneTurn* pTurn=NULL;
		bool bFind = false;
		std::vector<MLaneTurn*>::iterator iTurn;
		for (iTurn=pNode->LaneTurns.begin();iTurn!=pNode->LaneTurns.end();iTurn++)
		{
			pTurn = (*iTurn);
			if(pTurn->nFromLinkId == m_nInLinkIDSelected &&
			   pTurn->nFromIndex  == m_nInLaneNumSelected &&
			   pTurn->nToLinkId   == m_nOutLinkIDSelected &&
			   pTurn->nToIndex    == m_nOutLaneNumSelected )
			{
				bFind = true;
				break;
			}
		}
		if (bFind)
		{
			delete pTurn;
			pNode->LaneTurns.erase(iTurn);
		}

		int nCount = m_ListCtrl.GetItemCount();  
		while (nCount--)  
		{  
			m_ListCtrl.DeleteItem(nCount);  
		}
		FillTurnData();

		m_nTurnSelected = -1;

		for (int i=0;i<pNode->LaneTurns.size();i++)
		{
			m_ListCtrl.SelectRow(i,false);
		}
		//del按钮置灰，add按钮置白
		CButton *btnAdd =(CButton*) GetDlgItem(IDC_BTN_ADDTURN);
		btnAdd->EnableWindow(TRUE);
		CButton *btnDel =(CButton*) GetDlgItem(IDC_BTN_DELTURN);
		btnDel->EnableWindow(FALSE);
	}
}

void CPage_Node_LaneTurn::OnBnClickedBtnAddturn()
{
	MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
	if(m_nInLinkIDSelected >=0 && m_nInLaneNumSelected >=0 && m_nOutLinkIDSelected >=0	&& m_nOutLaneNumSelected >= 0)
	{
		MLaneTurn* pTurn=new MLaneTurn();
		pNode->LaneTurns.push_back(pTurn);
		pTurn->nFromLinkId = m_nInLinkIDSelected ;
		pTurn->nFromIndex  = m_nInLaneNumSelected ;
		pTurn->nToLinkId   = m_nOutLinkIDSelected ;
		pTurn->nToIndex    = m_nOutLaneNumSelected ;
		MMovement* pMove   = pNode->GetMMovement(m_nInLinkIDSelected,m_nOutLinkIDSelected);
		if (pMove)
		{
			pTurn->nRTL = pMove->nRTL;
			pTurn->bForbid = pMove->bForbid;
		}
		else
			AfxMessageBox("Laneturn without movement!",MB_OK);
	}

	int nCount = m_ListCtrl.GetItemCount();  
	while (nCount--)  
	{  
		m_ListCtrl.DeleteItem(nCount);  
	}
	FillTurnData();
	//新增记录高亮显示
	int nTurnIndex = pNode->GetLaneTurnIndex(m_nInLinkIDSelected,m_nInLaneNumSelected,m_nOutLinkIDSelected,m_nOutLaneNumSelected);
	m_nTurnSelected = nTurnIndex;

	for (int i=0;i<pNode->LaneTurns.size();i++)
	{
		char str[100];
		m_ListCtrl.GetItemText (i,0,str,20);
		int TurnIndex = atoi(str)-1; // the turn index has been sorted 

		if(TurnIndex == m_nTurnSelected)
			m_ListCtrl.SelectRow (i,true);
		else
			m_ListCtrl.SelectRow(i,false);
	}
	//del按钮置白，add按钮置灰
	CButton *btnAdd =(CButton*) GetDlgItem(IDC_BTN_ADDTURN);
	btnAdd->EnableWindow(FALSE);
	CButton *btnDel =(CButton*) GetDlgItem(IDC_BTN_DELTURN);
	btnDel->EnableWindow(TRUE);

	Invalidate();
}
