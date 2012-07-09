// Page_Node_Movement.cpp : implementation file
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
#include "Page_Node_Movement.h"

#include "CGridListCtrlEx\CGridColumnTraitDateTime.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"

#include <string>
#include <sstream>

// CPage_Node_Movement dialog

PinPoint::PinPoint(){}
PinPoint::~PinPoint(){}

IMPLEMENT_DYNAMIC(CPage_Node_Movement, CPropertyPage)

CPage_Node_Movement::CPage_Node_Movement()
	: CPropertyPage(CPage_Node_Movement::IDD)
	, m_CurrentNodeName(0)
{
	m_nSelectedMovementIndex = -1;
	m_ptCenter = CPoint(0,0);
	m_dScale   = 1.0;
	m_bLBTNDown = false;
	m_nSelectedInLinkID = -1;
	m_nSelectedOutLinkID= -1;
	
}

CPage_Node_Movement::~CPage_Node_Movement()
{
	for(int i=0;i<m_PinPointSet.size();i++)
	{
		PinPoint* p = m_PinPointSet[i];
		delete p;
	}
	m_PinPointSet.clear();
}

void CPage_Node_Movement::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_GRIDLISTCTRLEX,m_ListCtrl);
	DDX_Text(pDX, IDC_EDIT_CURRENT_NODEID, m_CurrentNodeName);
}


BEGIN_MESSAGE_MAP(CPage_Node_Movement, CPropertyPage)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GRIDLISTCTRLEX, &CPage_Node_Movement::OnLvnItemchangedGridlistctrlex)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_GRIDLISTCTRLEX, &CPage_Node_Movement::OnLvnEndlabeleditGridlistctrlex)
END_MESSAGE_MAP()



BOOL CPage_Node_Movement::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_CurrentNodeID =  m_pDoc->m_SelectedNodeID;
	m_CurrentNodeName = m_pDoc->m_NodeIDtoNameMap [m_CurrentNodeID];

	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;  // Hao: this ponter should be delete. 
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);

	std::vector<std::string> m_Column_names;

	int nWidth[5] = {50,50,50,100,100};

	m_Column_names.push_back ("Indx");
	m_Column_names.push_back ("I Node");
	m_Column_names.push_back ("O Node");
	m_Column_names.push_back ("T Type");
	m_Column_names.push_back ("Prhb");
	//m_Column_names.push_back ("SC NO");
	//m_Column_names.push_back ("SG No");

	//Add Columns and set headers
	m_ListCtrl.InsertHiddenLabelColumn();
	for (size_t i=0;i<m_Column_names.size();i++)
	{
		CGridColumnTrait* pTrait = NULL;
		if ( i != m_Column_names.size()-1 )
		{
			pTrait = NULL; //new CGridColumnTraitEdit();
			m_ListCtrl.InsertColumnTrait((int)(i+1),m_Column_names.at(i).c_str(),LVCFMT_LEFT,nWidth[i],i,pTrait);
			//m_ListCtrl.SetColumnWidth((int)(i+1),LVSCW_AUTOSIZE_USEHEADER);
		}
		if ( i == m_Column_names.size()-1)
		{
			//CGridColumnTraitDateTime *pDTTrait = new CGridColumnTraitDateTime();
			//pDTTrait->AddImageIndex(nStateImageIdx, _T("Permit"), true);		// Unchecked (and not editable)
			//pDTTrait->AddImageIndex(nStateImageIdx+1, _T("Forbid"), true);	// Checked (and editable)
			//pDTTrait->SetToggleSelection(true);
			//pTrait = pDTTrait;
			CGridColumnTraitCombo* pComboTrait = new CGridColumnTraitCombo;
			pComboTrait->AddItem(0,_T("Permit"));
			pComboTrait->AddItem(1,_T("Forbid"));
			pTrait = pComboTrait;
			m_ListCtrl.InsertColumnTrait((int)(i+1),m_Column_names.at(i).c_str(),LVCFMT_LEFT,nWidth[i],i,pTrait);
		}

	}
	//m_ListCtrl.SetColumnWidth(0, 80);

	//Add Rows

	//DTANode* pNode  = m_pDoc->m_NodeIDMap [m_CurrentNodeID];
	MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);

	for (int i=0;i< pNode->Movements.size();i++)
	{
		CString str;
		str.Format("%d",i+1);
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,str , 0, 0, 0, NULL);

		m_ListCtrl.SetItemText(Index, 1,str);
		MMovement* pMovement = pNode->Movements[i];
		MLink* pLink;
		pLink = m_pView->m_ms.GetMLinkbyID( pMovement->nFromLinkId);
		int nLinkFromNodeID = pLink->m_FromNodeID;
		pLink = m_pView->m_ms.GetMLinkbyID( pMovement->nToLinkId);
		int nLinkToNodeID   = pLink->m_ToNodeID;

		str.Format ("%d", m_pDoc->m_NodeIDtoNameMap[nLinkFromNodeID] );
		m_ListCtrl.SetItemText(Index, 2,str);

		str.Format ("%d", m_pDoc->m_NodeIDtoNameMap[nLinkToNodeID] );
		m_ListCtrl.SetItemText(Index, 3,str);
		
		CString strTurn;
		switch(pMovement->nRTL)
		{
		case 1: strTurn = _T("RightTurn"); break;
		case 2: strTurn = _T("Through"); break;
		case 3: strTurn = _T("LeftTurn"); break;
		}
		m_ListCtrl.SetItemText(Index,4,strTurn);
		str.Format("%s",pMovement->bForbid ? "Forbid" : "Permit");
		m_ListCtrl.SetItemText(Index,5,str);

	}

	UpdateData(0);

	CreatePinPoints();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPage_Node_Movement::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect PlotRect;
	GetClientRect(PlotRect);
	m_PlotRect = PlotRect;

	m_PlotRect.top += 35;
	m_PlotRect.bottom -= 35;
	m_PlotRect.left += 450;
	m_PlotRect.right -= 50;

	//DrawMovements(&dc,m_PlotRect);
	DrawFrame(&dc,m_PlotRect);
	DrawCentroid(&dc,m_PlotRect);
	DrawPinPoints(&dc,m_PlotRect);
	DrawNewMovement(&dc);
}

void CPage_Node_Movement::DrawMovements(CPaintDC* pDC,CRect PlotRect)
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

	CBrush  BrushLinkBand(RGB(152,245,255)); 
	pDC->SelectObject(&BrushLinkBand);

		DTANode* pNode  = m_pDoc->m_NodeIDMap [m_CurrentNodeID];

		int node_size = 10;
		int node_set_back = 50;

		int link_length = 150;
		int lane_width = 10;
		int text_length = link_length+ 20;

		CString str;
		str.Format("%d",m_CurrentNodeName);

		pDC->TextOutA( PlotRect.CenterPoint().x-5, PlotRect.CenterPoint().y-5,str);


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


		int link_mid_offset  = (pInLink->m_NumLanes/2 +1)*lane_width;  // mid
		
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

		link_mid_offset  = (pOutLink->m_NumLanes/2+1)*lane_width;
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


		if(i == m_nSelectedMovementIndex)
			pDC->SelectObject(&SelectedPen);
		else
			pDC->SelectObject(&NormalPen);

		Point_Movement[0]= NPtoSP(pt_movement[0]);
		Point_Movement[1]= NPtoSP(pt_movement[1]);
		Point_Movement[2]= NPtoSP(pt_movement[1]);
		Point_Movement[3]= NPtoSP(pt_movement[2]);

		MovementBezier element(Point_Movement[0], Point_Movement[1],Point_Movement[3]);
		
		m_MovementBezierVector.push_back (element);


		pDC->PolyBezier(Point_Movement,4);

		//restore pen
		pDC->SelectObject(&DataPen);

	}
}

void CPage_Node_Movement::DrawLink(CPaintDC* pDC,GDPoint pt_from, GDPoint pt_to,int NumberOfLanes, double theta, int lane_width)
{
		CPoint DrawPoint[4];

		//then offset
		int link_offset = lane_width;

		CPen pen,*pOldPen;
		pen.CreatePen(PS_DASH,1,RGB(255,0,0));

		pt_from.x += link_offset* cos(theta-PI/2.0f);
		pt_to.x += link_offset* cos(theta-PI/2.0f);

		pt_from.y += link_offset* sin(theta-PI/2.0f);
		pt_to.y += link_offset* sin(theta-PI/2.0f);

		DrawPoint[0] = NPtoSP(pt_from);
		DrawPoint[1] = NPtoSP(pt_to);

		link_offset = NumberOfLanes*lane_width ;
		pt_from.x += link_offset* cos(theta-PI/2.0f);
		pt_to.x += link_offset* cos(theta-PI/2.0f);

		pt_from.y += link_offset* sin(theta-PI/2.0f);
		pt_to.y += link_offset* sin(theta-PI/2.0f);

		DrawPoint[2] = NPtoSP(pt_to);
		DrawPoint[3] = NPtoSP(pt_from);

		pOldPen = pDC->SelectObject(&pen);
		pDC->Polygon(DrawPoint, 4);
		pDC->SelectObject(pOldPen);

}
// CPage_Node_Movement message handlers

void CPage_Node_Movement::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bLBTNDown = true;
	SetFocus();
	PinPoint *pSelected = PinPointHitTest(point);
	if (pSelected)
	{
		// 加一些判断！
		if (pSelected->nIO == 1 )
		{
			m_nSelectedInLinkID = pSelected->nLinkID;
		}
		else
		{
			m_nSelectedOutLinkID = pSelected->nLinkID;
		}
	}
	else
	{
		m_nSelectedInLinkID = -1;
		m_nSelectedOutLinkID = -1;
	}
	m_nSelectedMovementIndex = UpdateMovementStatus(pSelected);

	if(m_nSelectedMovementIndex >=0)
	{
		MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
		for (int i=0;i<pNode->Movements.size();i++)
		{
			char str[100];
			m_ListCtrl.GetItemText (i,0,str,20);
			int MovementIndex = atoi(str)-1; // the movement index has been sorted 

			if(MovementIndex == m_nSelectedMovementIndex)
				m_ListCtrl.SelectRow (i,true);
			else
				m_ListCtrl.SelectRow(i,false);
		}
	}

	CPropertyPage::OnLButtonDown(nFlags, point);
	Invalidate();

}

void CPage_Node_Movement::OnLvnItemchangedGridlistctrlex(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	int nSelected = -1;
	//CString strChange;

	while(pos!=NULL)
	{
		int nSelectedRow = m_ListCtrl.GetNextSelectedItem(pos);
		CString str;
		str = m_ListCtrl.GetItemText (nSelectedRow,0);
		//str = m_ListCtrl.GetItemText (nSelectedRow,1);
		nSelected = atoi(str)-1;
		//strChange = m_ListCtrl.GetItemText (nSelectedRow,4);
		//strChange = m_ListCtrl.GetItemText (nSelectedRow,5);
	}
	m_nSelectedMovementIndex = nSelected;
	//if ( nSelected >= 0 )
	//{
	//	int nComboIndex;
	//	if ( _T("Permit") == strChange ) nComboIndex = 0;
	//	else nComboIndex = 1;
	//	UpdateMovement(nSelected,nComboIndex);
	//}
	Invalidate();
}
////////////////////////////////////////////////////////////////////////////////////////
void CPage_Node_Movement::DrawFrame(CPaintDC *pDC, CRect PlotRect)
{
	CPen NormalPen(PS_SOLID,2,RGB(0,0,0));
	CBrush  WhiteBrush(RGB(255,255,255));

	pDC->SetBkMode(TRANSPARENT);
	CPen *pOldPen = pDC->SelectObject(&NormalPen);
	CBrush* pOldBrush=pDC->SelectObject(&WhiteBrush);

	pDC->Rectangle (PlotRect);

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}
void CPage_Node_Movement::DrawCentroid(CPaintDC* pDC,CRect PlotRect)
{
	int nRadius1 = 10;
	int nRadius2 = 14;
	int nDiff    = nRadius2 - nRadius1;

	CPen NormalPen(PS_SOLID,1,RGB(0,0,0));
	CPen DashPen(PS_DOT,1,RGB(0,0,0));
	CPen *pOldPen;
	CBrush  WhiteBrush(RGB(255,255,255));
	CBrush *pOldBrush;

	pOldPen   = pDC->SelectObject(&DashPen);
	pOldBrush = pDC->SelectObject(&WhiteBrush);

	GDPoint cntPoint,lefttop,bottomright;
	CRect centerRect;

	cntPoint.x = 0.0;
	cntPoint.y = 0.0;
	
	lefttop.x = cntPoint.x - nRadius2;
	lefttop.y = cntPoint.y - nRadius2;
	bottomright.x = cntPoint.x + nRadius2;
	bottomright.y = cntPoint.y + nRadius2;
	
	centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
	pDC->Ellipse(centerRect);

	pDC->SelectObject(&NormalPen);
	lefttop.x = cntPoint.x - nRadius1;
	lefttop.y = cntPoint.y - nRadius1;
	bottomright.x = cntPoint.x + nRadius1;
	bottomright.y = cntPoint.y + nRadius1;
	
	centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));

	pDC->Ellipse(centerRect);

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}
void CPage_Node_Movement::CreatePinPoints()
{
	if ( m_PinPointSet.size() > 0 ) return;
	// 根据Node的in和outlink，生成PinPoint
	MNode *pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
	int i;
	for(i=0;i<pNode->inLinks.size();i++)
	{
		MLink* pLink = pNode->inLinks[i];
		PinPoint * p = new PinPoint();
		p->nIO		 = 1;
		p->nLinkID	 = pLink->m_LinkID;
		p->bSelected = false;
		MNode* ppn   = m_pView->m_ms.GetMNodebyID(pLink->m_FromNodeID);
		p->nNodeID	 = ppn->m_NodeID;
		p->pp		 = ppn->pt;
		m_PinPointSet.push_back(p);
	}
	for(i=0;i<pNode->outLinks.size();i++)
	{
		MLink* pLink = pNode->outLinks[i];
		PinPoint * p = new PinPoint();
		p->nIO		 = 0;
		p->nLinkID	 = pLink->m_LinkID;
		p->bSelected = false;
		MNode* ppn   = m_pView->m_ms.GetMNodebyID(pLink->m_ToNodeID);
		p->nNodeID	 = ppn->m_NodeID;
		p->pp		 = ppn->pt;
		m_PinPointSet.push_back(p);
	}

	int nOffset = 100;
	int nRadius = 5;  // pin point radius
	GDPoint p2 = pNode->pt;
	for(i=0;i<m_PinPointSet.size();i++)
	{
		PinPoint * p = m_PinPointSet[i];
		double DeltaX,DeltaY,theta;
		if ( 1 == p->nIO )
		{
			GDPoint p1 = p->pp;
			DeltaX = p2.x - p1.x ;
			DeltaY = p2.y - p1.y ;
			theta = atan2(DeltaY, DeltaX);
			p->dpp.x = (-1)*nOffset*cos(theta) + nRadius* cos(theta-PI/2.0f);  
			p->dpp.y = (-1)*nOffset*sin(theta) + nRadius* sin(theta-PI/2.0f);
			p->cpp.x = (-1)*nRadius*cos(theta) + nRadius* cos(theta-PI/2.0f);  
			p->cpp.y = (-1)*nRadius*sin(theta) + nRadius* sin(theta-PI/2.0f);  
		}
		else // 0 == p->nIO
		{
			GDPoint p3 = p->pp;
			DeltaX = p3.x - p2.x ;
			DeltaY = p3.y - p2.y ;
			theta = atan2(DeltaY, DeltaX);
			p->dpp.x = nOffset*cos(theta) + nRadius* cos(theta-PI/2.0f);  
			p->dpp.y = nOffset*sin(theta) + nRadius* sin(theta-PI/2.0f);
			p->cpp.x = nRadius*cos(theta) + nRadius* cos(theta-PI/2.0f);  
			p->cpp.y = nRadius*sin(theta) + nRadius* sin(theta-PI/2.0f);  
		}
	}
}
void CPage_Node_Movement::DrawPinPoints(CPaintDC *pDC, CRect PlotRect)
{
	int nRadius = 5;

	CPen NormalPen(PS_SOLID,1,RGB(0,0,0));
	CPen *pOldPen;
	CBrush  GreyBrush(RGB(128,128,128));
	CBrush  RedBrush(RGB(255,0,0));
	CBrush *pOldBrush;

	pOldPen = pDC->SelectObject(&NormalPen);
	pOldBrush = pDC->SelectObject(&GreyBrush);

	for(int i=0;i<m_PinPointSet.size();i++)
	{
		PinPoint *p = m_PinPointSet[i];
		GDPoint cntPoint,lefttop,bottomright;
		CRect centerRect;

		cntPoint.x = p->dpp.x;
		cntPoint.y = p->dpp.y;
		
		lefttop.x = cntPoint.x - nRadius;
		lefttop.y = cntPoint.y - nRadius;
		bottomright.x = cntPoint.x + nRadius;
		bottomright.y = cntPoint.y + nRadius;
		
		centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
		centerRect.NormalizeRect();
		if ( p->nLinkID == m_nSelectedInLinkID || p->nLinkID == m_nSelectedOutLinkID)
			pDC->SelectObject(&RedBrush);
		else
			pDC->SelectObject(&GreyBrush);
		pDC->Ellipse(centerRect);
	}
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}
PinPoint* CPage_Node_Movement::PinPointHitTest(CPoint pt)
{// return the pointer of the PinPoint clicked, or NULL
	PinPoint* pRt = NULL;
	int nRadius =5;

	for(int i=0;i<m_PinPointSet.size();i++)
	{
		PinPoint *p = m_PinPointSet[i];
		GDPoint cntPoint,lefttop,bottomright;
		CRect centerRect;

		cntPoint.x = p->dpp.x;
		cntPoint.y = p->dpp.y;
		
		lefttop.x = cntPoint.x - nRadius;
		lefttop.y = cntPoint.y - nRadius;
		bottomright.x = cntPoint.x + nRadius;
		bottomright.y = cntPoint.y + nRadius;
		
		centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
		centerRect.NormalizeRect();
		if ( centerRect.PtInRect(pt) )
		{
			pRt = p;
			break;
		}
	}
	return pRt;
}
void CPage_Node_Movement::DrawNewMovement(CPaintDC *pDC)
{
	if ( m_nSelectedMovementIndex < 0 ) return;

	CPen GreyPen(PS_SOLID,2,RGB(128,128,128));
	CPen RedPen(PS_SOLID,2,RGB(255,0,0));
	CPen *pOldPen;
	int nRadius = 5;

	pOldPen = pDC->SelectObject(&GreyPen);
	MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);

	for(int i=0;i<pNode->Movements.size();i++)
	{
		MMovement* move= pNode->Movements[i];
		if (i != m_nSelectedMovementIndex) continue;

		GDPoint pin = GetPinPointByLinkID(move->nFromLinkId)->dpp;
		GDPoint cin = GetPinPointByLinkID(move->nFromLinkId)->cpp;
		GDPoint pout= GetPinPointByLinkID(move->nToLinkId)->dpp;
		GDPoint cout= GetPinPointByLinkID(move->nToLinkId)->cpp;

		pDC->MoveTo(NPtoSP(pin));
		pDC->LineTo(NPtoSP(cin));
		//pDC->MoveTo(NPtoSP(cout));
		//pDC->LineTo(NPtoSP(pout));
		DrawArrow(pDC,cout,pout);

		GDPoint cntPoint,lefttop,bottomright;
		CRect centerRect;

		cntPoint.x = 0.0;
		cntPoint.y = 0.0;
		
		lefttop.x = cntPoint.x - nRadius;
		lefttop.y = cntPoint.y - nRadius;
		bottomright.x = cntPoint.x + nRadius;
		bottomright.y = cntPoint.y + nRadius;
		
		centerRect = CRect(NPtoSP(lefttop),NPtoSP(bottomright));
		centerRect.NormalizeRect();

		pDC->Arc(centerRect,NPtoSP(cin),NPtoSP(cout));

	}
	pDC->SelectObject(pOldPen);
}
PinPoint* CPage_Node_Movement::GetPinPointByLinkID(int nLinkID)
{
	PinPoint *p = NULL;
	for(int i=0;i<m_PinPointSet.size();i++)
	{
		p = m_PinPointSet[i];
		if ( p->nLinkID == nLinkID)
			break;
	}
	return p;
}
int CPage_Node_Movement::UpdateMovementStatus(PinPoint *p /*= 0*/)
{
	int nRt = -1;
	MNode* pNode  = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
	for(int i=0;i<pNode->Movements.size();i++)
	{
		MMovement *move= pNode->Movements[i];
		if ( m_nSelectedInLinkID == move->nFromLinkId &&
			 m_nSelectedOutLinkID== move->nToLinkId )
		{
			 nRt = i;
			 break;
		}
	}
	return nRt;
}
void CPage_Node_Movement::DrawArrow(CDC* pDC, GDPoint m_One, GDPoint m_Two)
{
    double slopy , cosy , siny;   
    double Par = 8.0;//length of Arrow (>)   
    slopy = atan2( (double)( m_One.y - m_Two.y ),(double)( m_One.x - m_Two.x ) );   
    cosy = cos( slopy );   
    siny = sin( slopy );   
   
    //draw a line between the 2 endpoint   
    pDC->MoveTo( NPtoSP(m_One) );   
    pDC->LineTo( NPtoSP(m_Two) );   
       
	GDPoint pa1,pa2;
	pa1.x = m_Two.x + Par * cosy -  Par / 2.0 * siny ;
	pa1.y = m_Two.y + Par * siny +  Par / 2.0 * cosy ;
	pa2.x = m_Two.x + Par * cosy +  Par / 2.0 * siny ;
	pa2.y = m_Two.y - Par * siny -  Par / 2.0 * cosy ; 
	pDC->MoveTo(NPtoSP(m_Two));
	pDC->LineTo(NPtoSP(pa1));
	pDC->LineTo(NPtoSP(pa2));
	pDC->LineTo(NPtoSP(m_Two));
}
void CPage_Node_Movement::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bLBTNDown) 
	{
		m_bLBTNDown = false;
	}

	CPropertyPage::OnLButtonUp(nFlags, point);
}

void CPage_Node_Movement::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bLBTNDown)
	{
		HCURSOR   hc     =LoadCursor(NULL,IDC_CROSS); 
		SetCursor(hc);
	}

	CPropertyPage::OnMouseMove(nFlags, point);
}

BOOL CPage_Node_Movement::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// 判断pt在作图rect内
	if (zDelta > 0)
	{
		m_dScale += 0.2;
		if (m_dScale >= 2)
			m_dScale = 2;
	}
	else
	{
		m_dScale -= 0.2;
		if (m_dScale <= 0.4)
			m_dScale = 0.4;
	}

	Invalidate();
	return TRUE;
}
void CPage_Node_Movement::UpdateMovement(int nSelectedIndex, int nComboIndex)
{
	MNode* pNode = m_pView->m_ms.GetMNodebyID(m_CurrentNodeID);
	for(int i=0;i<pNode->Movements.size();i++)
	{
		MMovement *p = pNode->Movements[i];
		if ( i == nSelectedIndex )
		{
			if ( 0 == nComboIndex )
				p->bForbid = false;
			else
				p->bForbid = true;
			break;
		}
	}
	return;
}
void CPage_Node_Movement::OnLvnEndlabeleditGridlistctrlex(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	int nSelected = -1;
	CString strChange;

	while(pos!=NULL)
	{
		int nSelectedRow = m_ListCtrl.GetNextSelectedItem(pos);
		CString str;
		str = m_ListCtrl.GetItemText (nSelectedRow,0);
		str = m_ListCtrl.GetItemText (nSelectedRow,1);
		nSelected = atoi(str)-1;
		strChange = m_ListCtrl.GetItemText (nSelectedRow,4);
		strChange = m_ListCtrl.GetItemText (nSelectedRow,5);
	}
	m_nSelectedMovementIndex = nSelected;
	if ( nSelected >= 0 )
	{
		int nComboIndex;
		if ( _T("Permit") == strChange ) nComboIndex = 0;
		else nComboIndex = 1;
		UpdateMovement(nSelected,nComboIndex);
	}
	Invalidate();
}
