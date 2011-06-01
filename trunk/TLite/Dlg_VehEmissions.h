#pragma once
#include "afxwin.h"
#include "TLiteDoc.h"

// CDlg_VehEmissions dialog

class CDlg_VehEmissions : public CDialog
{
	DECLARE_DYNAMIC(CDlg_VehEmissions)

public:
	CDlg_VehEmissions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_VehEmissions();

	CTLiteDoc* m_pDoc;

// Dialog Data
	enum { IDD = IDD_DIALOG_VEHICLE_EMISSIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int GetOpMode(float vsp, float s_mph);
	CListBox m_VehicleList;
	afx_msg void OnLbnSelchangeListLink();
	CListBox m_LinkList;
	CComboBox m_OriginBox;
	CComboBox m_DestinationBox;
	CComboBox m_DepartureTimeBox;
	CComboBox m_VehicleTypeBox;
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListVehicle();
	afx_msg void OnLbnSelchangeListLink2();
	CListBox m_OpModeList;
};
