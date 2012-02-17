#pragma once

#include <vector>
#include <string>
#include "CGridListCtrlEx\\CGridListCtrlEx.h"
// CDlgScenarioTab dialog

class CDlgScenarioTab : public CDialog
{
	DECLARE_DYNAMIC(CDlgScenarioTab)

public:
	CDlgScenarioTab(CWnd* pParent = NULL);   // standard constructor
	CDlgScenarioTab(std::vector<std::string> names, std::vector<CString> default_value, std::vector<std::vector<std::string>> values, std::vector<std::string> linkstring);
	BOOL AddRow();
	BOOL DeleteRow();
	CString GenerateRecordString();
	void SetTabText(CString s);
	CString GetTabText();
	int ValidityCheck();
	virtual ~CDlgScenarioTab();

// Dialog Data
	enum { IDD = IDD_DIALOG_SCENARIO_TAB };
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

	std::vector<std::string> LinkString;

	CString m_TabText;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
