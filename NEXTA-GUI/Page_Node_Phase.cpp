// Page_Node_Phase.cpp : implementation file
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
#include "Page_Node_Phase.h"

#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"

#include <string>
#include <sstream>

// CPage_Node_Phase dialog


IMPLEMENT_DYNAMIC(CPage_Node_Phase, CPropertyPage)

CPage_Node_Phase::CPage_Node_Phase()
	: CPropertyPage(CPage_Node_Phase::IDD)
	, m_CurrentNodeName(0)
{
	m_SelectedMovementIndex = -1;
	m_SelectedPhaseIndex = -1;
	m_NumberOfPhases = 0;

	SetPhasePara();

}

CPage_Node_Phase::~CPage_Node_Phase()
{
}

void CPage_Node_Phase::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_GRIDLISTCTRLEX,m_ListCtrl);
	DDX_Text(pDX, IDC_EDIT_CURRENT_NODEID, m_CurrentNodeName);
}


BEGIN_MESSAGE_MAP(CPage_Node_Phase, CPropertyPage)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_GRIDLISTCTRLEX, &CPage_Node_Phase::OnLvnItemchangedGridlistctrlex)
	ON_BN_CLICKED(IDC_BUTTON_NEW_PHASE, &CPage_Node_Phase::OnBnClickedButtonNewPhase)
	ON_BN_CLICKED(IDC_BUTTON_DEL_PHASE, &CPage_Node_Phase::OnBnClickedButtonDelPhase)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_GRIDLISTCTRLEX, &CPage_Node_Phase::OnLvnEndlabeleditGridlistctrlex)
END_MESSAGE_MAP()



BOOL CPage_Node_Phase::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	m_CurrentNodeID =  m_pDoc->m_SelectedNodeID ;
	m_CurrentNodeName = m_pDoc->m_NodeIDtoNameMap [m_CurrentNodeID];
	// Give better margin to editors
	m_ListCtrl.SetCellMargin(1.2);
	CGridRowTraitXP* pRowTrait = new CGridRowTraitXP;  // Hao: this ponter should be delete. 
	m_ListCtrl.SetDefaultRowTrait(pRowTrait);

	std::vector<std::string> m_Column_names;
	m_Column_names.push_back ("Phase Index");
	m_Column_names.push_back ("Phase Name");
	m_Column_names.push_back ("Light Sequence");  //combo
	m_Column_names.push_back ("Group No");		  //combo
	m_Column_names.push_back ("Start Time");      //edit
	m_Column_names.push_back ("Green Length");    //edit
	int nWidth[6] = {50,50,80,50,50,50};

	std::vector<CString> m_Sequence_names;
	m_Sequence_names.push_back("Red-Green");
	m_Sequence_names.push_back("Red-Yellow-Green");
	m_Sequence_names.push_back("Flashing-Yellow");

	std::vector<CString> m_GroupNames;
	m_GroupNames.push_back("1");
	m_GroupNames.push_back("2");
	m_GroupNames.push_back("3");
	m_GroupNames.push_back("4");
	

	DTANode* pNode  = m_pDoc->m_NodeIDMap [m_CurrentNodeID];

	//Add Columns and set headers
	for (size_t i=0;i<m_Column_names.size();i++)
	{
		CGridColumnTrait* pTrait = NULL;
		if (0 == i)
		{
		}
		if( 1 == i) 
		{
		}
		if( 2 == i) 
		{
			CGridColumnTraitCombo* pComboTrait = new CGridColumnTraitCombo;
			for(size_t j=0; j < m_Sequence_names.size() ; ++j)
				pComboTrait->AddItem((DWORD_PTR)j, m_Sequence_names[j]);
			pTrait = pComboTrait;
		}
		if( 3 == i) 
		{
			CGridColumnTraitCombo* pComboTrait = new CGridColumnTraitCombo;
			for(size_t j=0; j < m_GroupNames.size() ; ++j)
				pComboTrait->AddItem((DWORD_PTR)j, m_GroupNames[j]);
			pTrait = pComboTrait;
		}
		if( 4 == i) 
		{
			pTrait = new CGridColumnTraitEdit();
		}
		if( 5 == i) 
		{
			pTrait = new CGridColumnTraitEdit();
		}
		m_ListCtrl.InsertColumnTrait((int)i,m_Column_names.at(i).c_str(),LVCFMT_LEFT,nWidth[i],i, pTrait);
	}

	//Add Rows
	ReadPhaseData(-1);
	FillPhaseData();
	CString str;
	str.Format("%d",m_nCycleLength);
	GetDlgItem(IDC_EDIT_CYCLE_LENGTH)->SetWindowTextA(str);
	CComboBox * pCombo = (CComboBox *)GetDlgItem(IDC_COMBO_SC);
	for(int i=1001;i<=1003;i++)
	{
	   str.Format("%d",i);
	   pCombo->AddString(str); 
	}
	pCombo->SetCurSel(0);

	CButton* pBtnNew = (CButton*)GetDlgItem(IDC_BUTTON_NEW_PHASE);
	pBtnNew->EnableWindow(FALSE);
	CButton* pBtnDel = (CButton*)GetDlgItem(IDC_BUTTON_DEL_PHASE);
	pBtnDel->EnableWindow(FALSE);

	UpdateData(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPage_Node_Phase::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect PlotRect;
	GetClientRect(PlotRect);
	m_PlotRect = PlotRect;

	m_PlotRect.top += 35;
	m_PlotRect.bottom -= 35;
	m_PlotRect.left += 450;
	m_PlotRect.right -= 50;

	DrawBackground(&dc);
	DrawPhase(&dc);
/*
	DrawMovements(&dc,m_PlotRect,-1);

	DTANode* pNode  = m_pDoc->m_NodeIDMap [m_CurrentNodeID];


	int left = PlotRect.left + 100;
	int right = PlotRect.right - 100;

	int width = 100;
	int height  = 100;
	int bottom  = PlotRect.bottom - 20;
	int top  = bottom - height;
	int spacing = 20;

	for (unsigned int i=0;i< pNode->m_PhaseVector .size();i++)
	{
		CRect PlotPhaseRect;
		PlotPhaseRect.left = left + i* (width + spacing);
		PlotPhaseRect.right = PlotPhaseRect.left + width;

		PlotPhaseRect.top = top;
		PlotPhaseRect.bottom  = bottom;
		m_PlotRect= PlotPhaseRect;

		DrawPhaseMovements(&dc,PlotPhaseRect, i);

	}
*/
}

void CPage_Node_Phase::DrawMovements(CPaintDC* pDC,CRect PlotRect,  int CurrentPhaseIndex = -1)
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


		if(i == m_SelectedMovementIndex)
			pDC->SelectObject(&SelectedPen);
		else if(movement.phase_index  == CurrentPhaseIndex)
			pDC->SelectObject(&SelectedPhasePen);
		else
			pDC->SelectObject(&NormalPen);



		Point_Movement[0]= NPtoSP(pt_movement[0]);
		Point_Movement[1]= NPtoSP(pt_movement[1]);
		Point_Movement[2]= NPtoSP(pt_movement[1]);
		Point_Movement[3]= NPtoSP(pt_movement[2]);



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

void CPage_Node_Phase::DrawPhaseMovements(CPaintDC* pDC,CRect PlotRect,  int CurrentPhaseIndex = -1)
{

	m_MovementBezierVector.clear();

	CPen NormalPen(PS_SOLID,1,RGB(0,0,0));
	CPen TimePen(PS_DOT,1,RGB(0,0,0));
	CPen DataPen(PS_SOLID,0,RGB(0,0,0));
	CPen SelectedPen(PS_SOLID,1,RGB(255,0,0));
	CPen SelectedPhasePen(PS_SOLID,1,RGB(0,0,255));

	CBrush  WhiteBrush(RGB(255,255,255)); 

	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&DataPen);
	pDC->SelectObject(&WhiteBrush);

	pDC->Rectangle (PlotRect);

	CBrush  BrushLinkBand(RGB(152,245,255)); 
	pDC->SelectObject(&BrushLinkBand);

		DTANode* pNode  = m_pDoc->m_NodeIDMap [m_CurrentNodeID];

		int node_size = 20;
		int node_set_back = 25;

		int link_length = 10;
		int lane_width = 2;
		int text_length = link_length+ 2;

		CString str;
		str.Format("%d",m_CurrentNodeName);

//		pDC->TextOutA( PlotRect.CenterPoint().x-5, PlotRect.CenterPoint().y-5,str);


	for (unsigned int i=0;i< pNode->m_MovementVector .size();i++)
	{
		DTANodeMovement movement = pNode->m_MovementVector[i];

		if(CurrentPhaseIndex >=0 && movement.phase_index  == CurrentPhaseIndex)
				continue;  // draw movements in a phase only

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

//		pDC->TextOutA(pt_text.x-10,pt_text.y,str);

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


//		pDC->TextOutA(pt_text.x-10 ,pt_text.y,str);

		DrawLink(pDC,p2_new,p3_new,pOutLink->m_NumLanes,theta,lane_width);


		// draw movement 

		CPoint Point_Movement[4];


		if(i == m_SelectedMovementIndex)
			pDC->SelectObject(&SelectedPen);
		else if(movement.phase_index  == CurrentPhaseIndex)
			pDC->SelectObject(&SelectedPhasePen);
		else
			pDC->SelectObject(&NormalPen);



		Point_Movement[0]= NPtoSP(pt_movement[0]);
		Point_Movement[1]= NPtoSP(pt_movement[1]);
		Point_Movement[2]= NPtoSP(pt_movement[1]);
		Point_Movement[3]= NPtoSP(pt_movement[2]);



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
void CPage_Node_Phase::DrawLink(CPaintDC* pDC,GDPoint pt_from, GDPoint pt_to,int NumberOfLanes, double theta, int lane_width)
{
		CPoint DrawPoint[4];

		//then offset
		int link_offset = lane_width;

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

		pDC->Polygon(DrawPoint, 4);

}
// CPage_Node_Phase message handlers

void CPage_Node_Phase::OnLButtonDown(UINT nFlags, CPoint point)
{
	unsigned int i;

	for ( i=0;i< m_PhaseData.size();i++)
	{
		m_ListCtrl.SelectRow (i,false);
	}

	m_nSelectedPhaseNo =  FindClickedPhaseNo(point);
	
	if(m_nSelectedPhaseNo >0)
	{
		for ( i=0;i< m_PhaseData.size();i++)
		{
			char str[100];
			m_ListCtrl.GetItemText (i,0,str,20);
			int nPhaseNo = atoi(str);  

			if(nPhaseNo == m_nSelectedPhaseNo)
			{
				m_ListCtrl.SelectRow (i,true);
			}
		}
	}

	CPropertyPage::OnLButtonDown(nFlags, point);
	Invalidate();

}

void CPage_Node_Phase::OnLvnItemchangedGridlistctrlex(NMHDR *pNMHDR, LRESULT *pResult)
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
		m_nSelectedPhaseNo = atoi(str);
	}
	Invalidate();
}
void CPage_Node_Phase::ReadPhaseData(int nSC)
{
	if ( -1 == nSC ) //default
	{
		PhaseRecord* p;

		p = new PhaseRecord(1,"WBL",2,  0,3,1, 80,20);
		m_PhaseData.push_back(p);

		p = new PhaseRecord(2,"EBT",1,180,1,1,  0,40);
		m_PhaseData.push_back(p);

		p = new PhaseRecord(3,"NBL",2,270,4,1,100,20);
		m_PhaseData.push_back(p);

		p = new PhaseRecord(4,"SBT",1, 90,2,1, 40,40);
		m_PhaseData.push_back(p);

		p = new PhaseRecord(5,"EBL",2,180,3,1, 80,20);
		m_PhaseData.push_back(p);

		p = new PhaseRecord(6,"WBT",1,  0,1,1,  0,40);
		m_PhaseData.push_back(p);

		p = new PhaseRecord(7,"SBL",2, 90,4,1,100,20);
		m_PhaseData.push_back(p);

		p = new PhaseRecord(8,"NBT",1,270,2,1, 40,40);
		m_PhaseData.push_back(p);


		m_nCycleLength = 120;

		m_nSelectedPhaseNo = -1;
	}
}
void CPage_Node_Phase::FillPhaseData()
{
	for (unsigned int i=0;i<m_PhaseData.size();i++)
	{
		PhaseRecord* p=m_PhaseData[i];
		CString str;
		str.Format("%d",p->nPhaseNo);
		int Index = m_ListCtrl.InsertItem(LVIF_TEXT,i,str, 0, 0, 0, NULL);

		m_ListCtrl.SetItemText(Index, 1, p->strName);

		switch (p->nSequenceType)
		{
		case 1:
			str.Format("%s","Red-Green");
			break;
		case 2:
			str.Format("%s","Red-Yellow-Green");
			break;
		case 3:
			str.Format("%s","Flashing-Yellow");
			break;
		default:
			str.Format("%s","default");
			break;
		}
		m_ListCtrl.SetItemText(Index, 2, str);

		str.Format("%d",p->nGroupNo);
		m_ListCtrl.SetItemText(Index, 3, str);

		str.Format("%d",p->nStartTime);
		m_ListCtrl.SetItemText(Index, 4, str);

		str.Format("%d",p->nGreenLength);
		m_ListCtrl.SetItemText(Index, 5, str);
	}
}
void CPage_Node_Phase::OnBnClickedButtonNewPhase()
{
	PhaseRecord* p = new PhaseRecord();
	p->nPhaseNo = m_PhaseData.size()+1;
	p->nSequenceType = 1;
	p->nStartTime = 0;
	p->nGreenLength = 28;
	p->nYellowLength= 2;
	p->nRedLength   = 30;
	m_PhaseData.push_back(p);

	int nCount = m_ListCtrl.GetItemCount();  
	while (nCount--)  
	{  
		m_ListCtrl.DeleteItem(nCount);  
	}
	FillPhaseData();
}

void CPage_Node_Phase::OnBnClickedButtonDelPhase()
{
	if ( m_nSelectedPhaseNo < 0 )  return;
	std::vector<PhaseRecord*> tempData = m_PhaseData;
	m_PhaseData.clear();
	PhaseRecord* pDelete = NULL;
	int nNewPhaseNo = 1;

	for (int i=0;i<tempData.size();i++)
	{
		PhaseRecord* p = tempData[i];
		if ( m_nSelectedPhaseNo == p->nPhaseNo )
		{
			pDelete = p;
			continue;
		}
		else
		{
			p->nPhaseNo = nNewPhaseNo;
			m_PhaseData.push_back(p);
			nNewPhaseNo ++;
		}
	}

	if ( pDelete)
	{
		delete pDelete;
	}

	int nCount = m_ListCtrl.GetItemCount();  
	while (nCount--)  
	{  
		m_ListCtrl.DeleteItem(nCount);  
	}
	FillPhaseData();
}
void CPage_Node_Phase::DrawBackground(CPaintDC* pDC)
{
	CPen blackPen(PS_SOLID,0,RGB(0,0,0));
	CBrush  whiteBrush(RGB(255,255,255)); 

	pDC->SetBkMode(TRANSPARENT);
	CPen *pOldPen = pDC->SelectObject(&blackPen);
	CBrush* pOldBrush = pDC->SelectObject(&whiteBrush);
	pDC->Rectangle(m_PlotRect);

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}
void CPage_Node_Phase::DrawPhase(CPaintDC* pDC,int nPhaseNo /*= -1*/,bool bSelected /* = false */)
{
	if ( -1 == nPhaseNo )
	{
		for(int i=0;i<m_PhaseData.size();i++)
		{
			DrawPhase(pDC,m_PhaseData[i]->nPhaseNo);
		}
	}
	else
	{
		for(int i=0;i<m_PhaseData.size();i++)
		{
			PhaseRecord* p = m_PhaseData[i];
			if ( p->nPhaseNo == nPhaseNo )
			{
				// Draw a phase
				int nR1 = m_Para.nR1;
				int nR2 = m_Para.nR2;
				int nM1 = m_Para.nM1;
				int nW1 = m_Para.nW1;
				int nArrow = m_Para.nArrow;
				double dTheta = (double) (p->nAngle * PI ) / 180.0;

				CPen blackPen(PS_SOLID,0,RGB(0,0,0));
				CBrush  redBrush(RGB(255,0,0)); 
				CBrush  blackBrush(RGB(0,0,0)); 

				CPen *pOldPen = pDC->SelectObject(&blackPen);
				CBrush* pOldBrush;
				if ( m_nSelectedPhaseNo == p->nPhaseNo )
					pOldBrush = pDC->SelectObject(&redBrush);
				else
					pOldBrush = pDC->SelectObject(&blackBrush);
				
				GDPoint p1,p2,p3,p4,p5;
				CRect colorRect;

				if ( 1 == p->nPhaseType ) //through
				{
					p1.x = nR2 * cos(dTheta) - 2.0 * nW1 * cos(dTheta-PI/2.0f);
					p1.y = nR2 * sin(dTheta) - 2.0 * nW1 * sin(dTheta-PI/2.0f);
					
					p2.x = nR1 * cos(dTheta) - 1.5 * nW1 * cos(dTheta-PI/2.0f);
					p2.y = nR1 * sin(dTheta) - 1.5 * nW1 * sin(dTheta-PI/2.0f);

					p3.x = nR1 * cos(dTheta) - 3 * nW1 * cos(dTheta-PI/2.0f);
					p3.y = nR1 * sin(dTheta) - 3 * nW1 * sin(dTheta-PI/2.0f);

					p4.x = nR1 * cos(dTheta) - 3.5 * nW1 * cos(dTheta-PI/2.0f);
					p4.y = nR1 * sin(dTheta) - 3.5 * nW1 * sin(dTheta-PI/2.0f);

					p5.x = (nR1-nArrow) * cos(dTheta) - 2.5 * nW1 * cos(dTheta-PI/2.0f);
					p5.y = (nR1-nArrow) * sin(dTheta) - 2.5 * nW1 * sin(dTheta-PI/2.0f);

					colorRect = CRect(NPtoSP(p1),NPtoSP(p3));
					colorRect.NormalizeRect();
					pDC->Rectangle(colorRect);

					CPoint pt[3];
					pt[0] = NPtoSP(p2);
					pt[1] = NPtoSP(p4);
					pt[2] = NPtoSP(p5);

					pDC->Polygon(pt,3);
				}
				else // left turn
				{
					p1.x = nR2 * cos(dTheta);
					p1.y = nR2 * sin(dTheta);
					
					p3.x = nM1 * cos(dTheta) - nW1 * cos(dTheta-PI/2.0f);
					p3.y = nM1 * sin(dTheta) - nW1 * sin(dTheta-PI/2.0f);

					colorRect = CRect(NPtoSP(p1),NPtoSP(p3));
					colorRect.NormalizeRect();
					pDC->Rectangle(colorRect);

					p2.x = nM1 * cos(dTheta);
					p2.y = nM1 * sin(dTheta);
					
					p4.x = (nM1-2*nW1) * cos(dTheta) + nW1 * cos(dTheta-PI/2.0f);
					p4.y = (nM1-2*nW1) * sin(dTheta) + nW1 * sin(dTheta-PI/2.0f);

					p5.x = (nM1-nW1) * cos(dTheta) + nW1 * cos(dTheta-PI/2.0f);
					p5.y = (nM1-nW1) * sin(dTheta) + nW1 * sin(dTheta-PI/2.0f);


					CPoint pt[4];
					pt[0] = NPtoSP(p2);
					pt[1] = NPtoSP(p3);
					pt[2] = NPtoSP(p4);
					pt[3] = NPtoSP(p5);

					pDC->Polygon(pt,4);

					p1.x = nM1* cos(dTheta) + nW1 * cos(dTheta-PI/2.0f);
					p1.y = nM1* sin(dTheta) + nW1 * sin(dTheta-PI/2.0f);
					p2.x = (nM1-3*nW1) * cos(dTheta) + nW1 * cos(dTheta-PI/2.0f);
					p2.y = (nM1-3*nW1) * sin(dTheta) + nW1 * sin(dTheta-PI/2.0f);
					p3.x = (nM1-1.5*nW1) * cos(dTheta) + 2*nW1 * cos(dTheta-PI/2.0f);
					p3.y = (nM1-1.5*nW1) * sin(dTheta) + 2*nW1 * sin(dTheta-PI/2.0f);

					pt[0] = NPtoSP(p1);
					pt[1] = NPtoSP(p2);
					pt[2] = NPtoSP(p3);

					pDC->Polygon(pt,3);
				}

				pDC->SelectObject(pOldPen);
				pDC->SelectObject(pOldBrush);
			}
		}
	}
}
void CPage_Node_Phase::SetPhasePara()
{
	m_Para.nR1 = 70;
	m_Para.nR2 = 150;
	m_Para.nM1 = 110;
	m_Para.nW1 = 10;
	m_Para.nArrow = 20;
}
int  CPage_Node_Phase::FindClickedPhaseNo(CPoint point)
{
	int nPhaseNo = -1;

	for(int i=0;i<m_PhaseData.size();i++)
	{
		PhaseRecord* p = m_PhaseData[i];
		int nR1 = m_Para.nR1;
		int nR2 = m_Para.nR2;
		int nM1 = m_Para.nM1;
		int nW1 = m_Para.nW1;
		int nArrow = m_Para.nArrow;
		double dTheta = (double) (p->nAngle * PI ) / 180.0;

		GDPoint p1,p2,p3,p4,p5;
		CRect colorRect;

		if ( 1 == p->nPhaseType ) //through
		{
			p1.x = nR2 * cos(dTheta) - 2.0 * nW1 * cos(dTheta-PI/2.0f);
			p1.y = nR2 * sin(dTheta) - 2.0 * nW1 * sin(dTheta-PI/2.0f);
			
			p2.x = nR1 * cos(dTheta) - 1.5 * nW1 * cos(dTheta-PI/2.0f);
			p2.y = nR1 * sin(dTheta) - 1.5 * nW1 * sin(dTheta-PI/2.0f);

			p3.x = nR1 * cos(dTheta) - 3 * nW1 * cos(dTheta-PI/2.0f);
			p3.y = nR1 * sin(dTheta) - 3 * nW1 * sin(dTheta-PI/2.0f);

			p4.x = nR1 * cos(dTheta) - 3.5 * nW1 * cos(dTheta-PI/2.0f);
			p4.y = nR1 * sin(dTheta) - 3.5 * nW1 * sin(dTheta-PI/2.0f);

			p5.x = (nR1-nArrow) * cos(dTheta) - 2.5 * nW1 * cos(dTheta-PI/2.0f);
			p5.y = (nR1-nArrow) * sin(dTheta) - 2.5 * nW1 * sin(dTheta-PI/2.0f);

			colorRect = CRect(NPtoSP(p1),NPtoSP(p3));
			colorRect.NormalizeRect();
			if ( colorRect.PtInRect(point))
			{
				nPhaseNo = p->nPhaseNo;
				break;
			}

			CPoint pt[3];
			pt[0] = NPtoSP(p2);
			pt[1] = NPtoSP(p4);
			pt[2] = NPtoSP(p5);

			CRgn rgnA;
			rgnA.CreatePolygonRgn(pt,3,ALTERNATE);
			if ( rgnA.PtInRegion(point) )
			{
				nPhaseNo = p->nPhaseNo;
				break;
			}
		}
		else // left turn
		{
			p1.x = nR2 * cos(dTheta);
			p1.y = nR2 * sin(dTheta);
			
			p3.x = nM1 * cos(dTheta) - nW1 * cos(dTheta-PI/2.0f);
			p3.y = nM1 * sin(dTheta) - nW1 * sin(dTheta-PI/2.0f);

			colorRect = CRect(NPtoSP(p1),NPtoSP(p3));
			colorRect.NormalizeRect();
			if ( colorRect.PtInRect(point))
			{
				nPhaseNo = p->nPhaseNo;
				break;
			}

			p2.x = nM1 * cos(dTheta);
			p2.y = nM1 * sin(dTheta);
			
			p4.x = (nM1-2*nW1) * cos(dTheta) + nW1 * cos(dTheta-PI/2.0f);
			p4.y = (nM1-2*nW1) * sin(dTheta) + nW1 * sin(dTheta-PI/2.0f);

			p5.x = (nM1-nW1) * cos(dTheta) + nW1 * cos(dTheta-PI/2.0f);
			p5.y = (nM1-nW1) * sin(dTheta) + nW1 * sin(dTheta-PI/2.0f);


			CPoint pt[4];
			pt[0] = NPtoSP(p2);
			pt[1] = NPtoSP(p3);
			pt[2] = NPtoSP(p4);
			pt[3] = NPtoSP(p5);

			CRgn rgnA;
			rgnA.CreatePolygonRgn(pt,4,ALTERNATE);
			if ( rgnA.PtInRegion(point) )
			{
				nPhaseNo = p->nPhaseNo;
				break;
			}

			p1.x = nM1* cos(dTheta) + nW1 * cos(dTheta-PI/2.0f);
			p1.y = nM1* sin(dTheta) + nW1 * sin(dTheta-PI/2.0f);
			p2.x = (nM1-3*nW1) * cos(dTheta) + nW1 * cos(dTheta-PI/2.0f);
			p2.y = (nM1-3*nW1) * sin(dTheta) + nW1 * sin(dTheta-PI/2.0f);
			p3.x = (nM1-1.5*nW1) * cos(dTheta) + 2*nW1 * cos(dTheta-PI/2.0f);
			p3.y = (nM1-1.5*nW1) * sin(dTheta) + 2*nW1 * sin(dTheta-PI/2.0f);
			
			CPoint ppt[3];
			ppt[0] = NPtoSP(p1);
			ppt[1] = NPtoSP(p2);
			ppt[2] = NPtoSP(p3);

			CRgn rgnB;
			rgnB.CreatePolygonRgn(ppt,3,ALTERNATE);
			if ( rgnB.PtInRegion(point) )
			{
				nPhaseNo = p->nPhaseNo;
				break;
			}
		}
	}
	return nPhaseNo;
}
void CPage_Node_Phase::OnLvnEndlabeleditGridlistctrlex(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	*pResult = 0;

	POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition();
	int nSelected = -1;
	CString str,strPhase,strStart,strGT;

	while(pos!=NULL)
	{
		int nSelectedRow = m_ListCtrl.GetNextSelectedItem(pos);
		str = m_ListCtrl.GetItemText (nSelectedRow,0);
		nSelected = atoi(str)-1;
		strPhase = m_ListCtrl.GetItemText (nSelectedRow,4);
		strStart = m_ListCtrl.GetItemText (nSelectedRow,1);
		strGT    = m_ListCtrl.GetItemText (nSelectedRow,1);
	}
	if ( nSelected >= 0 )
	{
		UpdatePhase(atoi(strPhase),atoi(strStart),atoi(strGT));
	}
}
void CPage_Node_Phase::UpdatePhase(int nPhaseNo, int nStart, int nGT)
{
	for(int i=0;i<m_PhaseData.size();i++)
	{
		PhaseRecord* pPhaseRecord = m_PhaseData[i];
		if ( nPhaseNo == pPhaseRecord->nPhaseNo )
		{
			pPhaseRecord->nStartTime = nStart;
			pPhaseRecord->nGreenLength= nGT;
		}
	}
}
