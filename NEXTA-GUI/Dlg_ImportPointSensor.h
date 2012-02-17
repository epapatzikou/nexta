#pragma once

#include "Geometry.h"
#include "CSVParser.h"
#include "RecordSetExt.h"
#include "TLite.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "afxwin.h"
// CDlg_ImportPointSensor dialog

class CDlg_ImportPointSensor : public CDialog
{
	DECLARE_DYNAMIC(CDlg_ImportPointSensor)

public:
	CTLiteDoc* m_pDOC;
	CString strSQL;
	CDlg_ImportPointSensor(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_ImportPointSensor();

// Dialog Data
	enum { IDD = IDD_DIALOG_EXCEL_IMPORT_POINT_SENSOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_Sensor_File;
	afx_msg void OnBnClickedButtonFindSensorFile();
	afx_msg void OnBnClickedImportPointSensorLocationandData();
	CListBox m_MessageList;
};
#pragma once


