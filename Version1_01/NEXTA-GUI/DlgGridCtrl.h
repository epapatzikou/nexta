#pragma once

#include "TLiteDoc.h"
#include "GridCtrl_src\\GridCtrl.h"
#include "CGridListCtrlEx\\CGridListCtrlEx.h"
#include <vector>
#include "afxwin.h"

using std::map;
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
	int m_DemandSequenceNo;

	std::map<CString,float> m_ODMatrixMap;

	CString GetODKey(int origin,int destination)
	{
	CString str;
	str.Format("%d,%d",origin,destination);
	return str;

	}

	void SetODMatrx(int origin,int destination,float value)
	{
	m_ODMatrixMap[GetODKey(origin,destination)] = value;

	m_ODMatrixMap[GetODKey(origin,m_pDoc->m_ODSize+1)] += value; // destination subtotal

	m_ODMatrixMap[GetODKey(m_pDoc->m_ODSize+1,destination)] += value; // origin subtotal

	}

	CString m_SelectedFileName;
	std::vector<CString> DemandFileNameVector;
	void LoadDemandMatrixFromDemandFile(int DemandFileSequenceNo, int SelectedDemandMetaType);
private:
	CGridCtrl m_ODMatrixGrid;
	CListCtrl m_DemandTypeGrid;
	CListCtrl m_DemandFileGrid;
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
	afx_msg void OnLvnItemchangedDemandtypelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedEditMetaDatabase();
	afx_msg void OnLvnItemchangedDemandtypelist2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedEditMetaDatabase2();
	afx_msg void OnBnClickedEditVehicleTypeFile();
	afx_msg void OnBnClickedEditVehicleEmissionsFile();
	afx_msg void OnBnClickedEditMetaDatabase3();
};
