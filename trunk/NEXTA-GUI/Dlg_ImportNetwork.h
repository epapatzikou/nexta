#pragma once
#include "Geometry.h"
#include "CSVParser.h"
#include "RecordSetExt.h"
#include "TLite.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "afxwin.h"


// CDlg_ImportNetwork dialog

class CDlg_ImportNetwork : public CDialog
{
	DECLARE_DYNAMIC(CDlg_ImportNetwork)

public:

	CDlg_ImportNetwork(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_ImportNetwork();

	CTLiteDoc* m_pDOC;

	bool m_bImportNetworkOnly;

// Dialog Data
	enum { IDD = IDD_DIALOG_EXCEL_IMPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonFindExelFile();
	afx_msg void OnBnClickedButtonFindDemandCsvFile();
	afx_msg void OnBnClickedImport();
	CString m_Edit_Excel_File;
	CString m_Edit_Demand_CSV_File;
	CListBox m_MessageList;
	afx_msg void OnBnClickedImportNetworkOnly();
	CString m_Sensor_File;
	afx_msg void OnBnClickedButtonFindSensorFile();
	afx_msg void OnBnClickedImportSensorData();
	afx_msg void OnLbnSelchangeList1();
};
