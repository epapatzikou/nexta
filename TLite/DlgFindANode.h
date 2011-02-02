#pragma once


// CDlgFindANode dialog

class CDlgFindANode : public CDialog
{
	DECLARE_DYNAMIC(CDlgFindANode)

public:
	CDlgFindANode(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgFindANode();

// Dialog Data
	enum { IDD = IDD_DIALOGFindANode };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	long m_NodeNumber;
};
