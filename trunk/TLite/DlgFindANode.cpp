// DlgFindANode.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgFindANode.h"


// CDlgFindANode dialog

IMPLEMENT_DYNAMIC(CDlgFindANode, CDialog)

CDlgFindANode::CDlgFindANode(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFindANode::IDD, pParent)
	, m_NodeNumber(0)
{

}

CDlgFindANode::~CDlgFindANode()
{
}

void CDlgFindANode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_NodeNumber);
}


BEGIN_MESSAGE_MAP(CDlgFindANode, CDialog)
END_MESSAGE_MAP()


// CDlgFindANode message handlers
