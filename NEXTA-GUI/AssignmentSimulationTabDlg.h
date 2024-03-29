#pragma once

#include <vector>
#include <string>
#include "CGridListCtrlEx\\CGridListCtrlEx.h"
// CAssignmentSimulationTabDlg dialog

class CAssignmentSimulationTabDlg : public CDialog
{
	DECLARE_DYNAMIC(CAssignmentSimulationTabDlg)

public:
	CAssignmentSimulationTabDlg(CWnd* pParent = NULL);   // standard constructor
	CAssignmentSimulationTabDlg(std::vector<std::string> names, std::vector<CString> default_value, std::vector<std::vector<std::string>> values);
	BOOL CopyRow();
	BOOL AddRow();
	BOOL DeleteRow();
	CString GenerateRecordString();
	void SetTabText(CString s);
	CString GetTabText();
	int ValidityCheck();
	virtual ~CAssignmentSimulationTabDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTING_TAB };
private:
	CGridListCtrlEx m_ListCtrl;
	std::vector<std::string> names;
	std::vector<std::vector<std::string>> values;
	std::vector<CString> default_value;

	int m_NumOfCols;
	int m_NumOfRows;

	int m_StartTime_Idx;
	int m_EndTime_Idx;
	int m_Rate_Idx;
	int m_TrafficFlowModel_Idx;
	int m_ODME_mode_Idx;
	int m_emission_data_output_Idx;
	int m_traffic_assignment_method_Idx;

	CString m_TabText;

	CImageList m_ImageList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
