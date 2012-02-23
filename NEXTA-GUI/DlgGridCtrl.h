#pragma once

#include "TLiteDoc.h"
#include "GridCtrl_src\\GridCtrl.h"

#include <vector>

class ZoneRecord
{
public:
	ZoneRecord(int z, int n):zone_num(z),node_name(n){};
	int zone_num;
	int node_name;
};

// CDlgGridCtrl dialog

class CDlgGridCtrl : public CDialog
{
	DECLARE_DYNAMIC(CDlgGridCtrl)

public:
	CDlgGridCtrl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgGridCtrl();

// Dialog Data
	enum { IDD = IDD_ODGRID_DIALOG };

	std::vector<ZoneRecord> ZoneRecordSet;
	CTLiteDoc* m_pDoc;

private:
	CGridCtrl m_Grid;
	bool m_bSizeChanged;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGridSavequit();
	afx_msg void OnBnClickedGridQuit();

	void OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	bool SaveDemandCSVFileExt(LPCTSTR lpszFileName);
	bool ReadDemandCSVFileExt(LPCTSTR lpszFileName);
	bool ReadZoneCSVFileExt(LPCTSTR lpszFileName);
	bool SaveZoneCSVFileExt(LPCTSTR lpszFileName);
	afx_msg void OnBnClickedButtonCreatezones();
	afx_msg void OnBnClickedButtonEditZoneNodeMapping();
	afx_msg void OnBnClickedButton1();
};
