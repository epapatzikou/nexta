#pragma once

#include "TLiteDoc.h"
#include "GridCtrl_src\\GridCtrl.h"
#include "CGridListCtrlEx\\CGridListCtrlEx.h"
#include <vector>
#include "afxwin.h"

class ZoneRecord
{
public:
	ZoneRecord(int z, int n):zone_num(z),node_name(n){};
	int zone_num;
	int node_name;
};

// CDlgODDemandGridCtrl dialog

class CDlgODDemandGridCtrl : public CDialog
{
	DECLARE_DYNAMIC(CDlgODDemandGridCtrl)

public:
	CDlgODDemandGridCtrl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgODDemandGridCtrl();

// Dialog Data
	enum { IDD = IDD_ODDEMAND_GRID_DIALOG };

	std::vector<ZoneRecord> ZoneRecordSet;
	CTLiteDoc* m_pDoc;
	void DisplayDemandMatrix();
	void DisplayDemandTypeTable();

	int m_SelectedDemandMetaType;


private:
	CGridCtrl m_ODMatrixGrid;
	CListCtrl m_DemandTypeGrid;
	bool m_bSizeChanged;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGridSavequit();
	afx_msg void OnBnClickedGridQuit();

	void OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	bool SaveDemandMatrix();

	bool SaveZoneCSVFileExt(LPCTSTR lpszFileName);
	afx_msg void OnBnClickedButtonCreatezones();
	afx_msg void OnBnClickedButtonEditZoneNodeMapping();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedGridSavequit2();
	float m_DemandMultipler;
	afx_msg void OnLvnItemchangedDemandtypelist(NMHDR *pNMHDR, LRESULT *pResult);
};
