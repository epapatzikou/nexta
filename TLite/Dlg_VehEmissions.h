#pragma once
#include "afxwin.h"
#include "TLiteDoc.h"
#include "BaseDialog.h"
#include "network.h"

// CDlg_VehEmissions dialog

class ODStatistics
{
public: 
	ODStatistics()
	{
	TotalVehicleSize = 0;
	TotalTravelTime = 0;
	TotalDistance = 0;
	TotalCost = 0;
	TotalEmissions = 0;
	}

	void Reset()
	{
	TotalVehicleSize = 0;
	TotalTravelTime = 0;
	TotalDistance = 0;
	TotalCost = 0;
	TotalEmissions = 0;
	bImpactFlag = false;
	}

	bool bImpactFlag;
	int   TotalVehicleSize;
	float TotalTravelTime;
	float TotalDistance;
	float TotalCost;
	float TotalEmissions;
	CVehicleEmission emissiondata;

};

class PathStatistics
{
public: 
	PathStatistics()
	{
	TotalVehicleSize = 0;
	TotalTravelTime = 0;
	TotalDistance = 0;
	TotalCost = 0;
	TotalEmissions = 0;
	}

	int   NodeNumberSum;
	int   NodeSize;

	std::vector<int> m_LinkVector;
	std::vector<DTAVehicle*> m_VehicleVector;

	int   TotalVehicleSize;
	float TotalTravelTime;
	float TotalDistance;
	float TotalCost;
	float TotalEmissions;
	CVehicleEmission emissiondata;


};



class CDlg_VehEmissions : public CBaseDialog
{
	DECLARE_DYNAMIC(CDlg_VehEmissions)

public:
	CDlg_VehEmissions(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_VehEmissions();

	CTLiteDoc* m_pDoc;
	ODStatistics** m_ODMOEMatrix;

	std::vector<PathStatistics> m_PathVector;

	void FilterOriginDestinationPairs();
	void FilterPaths();
	void ShowSelectedPath();
	void ShowVehicles();

	bool ExportDataToCSVFileAllOD(char fname[_MAX_PATH]);


// Dialog Data
	enum { IDD = IDD_DIALOG_VEHICLE_EMISSIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
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
	afx_msg void OnCbnSelchangeComboOrigin();
	afx_msg void OnCbnSelchangeComboDestination();
	afx_msg void OnCbnSelchangeComboDeparturetime();
	afx_msg void OnCbnSelchangeComboVehicletype();
	afx_msg void OnCbnSelchangeComboInformationclass();
	CComboBox m_InformationClassBox;
	CComboBox m_MinVehicleSizeBox;
	CComboBox m_MinDistanceBox;
	CComboBox m_MinTTIBox;
	CComboBox m_TimeIntervalBox;
	CListBox m_ODList;
	afx_msg void OnCbnSelchangeComboTimeinterval();
	afx_msg void OnCbnSelchangeComboMinNumberofvehicles();
	afx_msg void OnCbnSelchangeComboMinTravelTime();
	afx_msg void OnCbnSelchangeComboMinTraveltimeindex();
	CListBox m_PathList;
	afx_msg void OnLbnSelchangeListOd();
	afx_msg void OnLbnSelchangeListPath();
	CComboBox m_ImpactLinkBox;
	afx_msg void OnCbnSelchangeComboImpactlink();
	afx_msg void OnLbnDblclkListOd();
	afx_msg void OnBnClickedExport();
	CEdit m_Summary_Info_Edit;
	CComboBox m_ComboBox_VOT_LB;
	CComboBox m_ComboBox_VOT_UB;
	afx_msg void OnCbnSelchangeComboVotLb();
	afx_msg void OnCbnSelchangeComboVotUb();
};
