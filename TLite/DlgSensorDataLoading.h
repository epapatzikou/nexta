#pragma once


// CDlgSensorDataLoading dialog

class CDlgSensorDataLoading : public CDialog
{
	DECLARE_DYNAMIC(CDlgSensorDataLoading)

public:
	CDlgSensorDataLoading(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSensorDataLoading();

// Dialog Data
	enum { IDD = IDD_DIALOGSensorDataLoading };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_ObsTimeInterval;
	int m_NumberOfDays;
	float m_Occ_to_Density_Coef;
	int m_StartingTime_in_min;
};
