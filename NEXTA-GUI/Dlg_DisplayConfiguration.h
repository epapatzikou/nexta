#pragma once
#include "afxwin.h"
#include "TLiteDoc.h"
#include "TLiteView.h"


// CDlg_DisplayConfiguration dialog

class CDlg_DisplayConfiguration : public CDialog
{
	DECLARE_DYNAMIC(CDlg_DisplayConfiguration)

public:
	CDlg_DisplayConfiguration(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlg_DisplayConfiguration();

	CTLiteView* pView;
// Dialog Data
	enum { IDD = IDD_DIALOG_DISPLAY_CONFIG };

	std::vector<int> movement_text_size_vector;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	std::vector<int> m_AggregationValueVector;
	afx_msg void OnLbnSelchangeListLinkTextLabel();
	node_display_mode m_ShowNodeTextMode;

	CListBox m_Link_Label, m_Node_Label, m_Zone_Label;
	CListBox m_Movement_Label;

	afx_msg void OnLbnSelchangeListNodeTextLabel();
	virtual BOOL OnInitDialog();
	CListBox m_List_Node_Label;
	CListBox m_AggregationIntervalList;
	afx_msg void OnLbnSelchangeMoeAggregationIntervalList();
	afx_msg void OnLbnSelchangeListZoneTextLabel();
	afx_msg void OnBnClickedOk();

	afx_msg void OnLbnSelchangeListMovementTextLabel();
	CListBox m_GPS_Label;
	afx_msg void OnLbnSelchangeListGpsTextLabel();
	CComboBox m_ComboBox_MovementTextBoxSize;
	afx_msg void OnCbnSelchangeComboMovementtextboxsize();
	afx_msg void OnBnClickedButtonIncreasenodesize();
	afx_msg void OnBnClickedButtonDecreasenodesize();
	afx_msg void OnBnClickedButtonIncreasenodetextsize();
	afx_msg void OnBnClickedButtonDecreasenodetextsize();
	CListBox m_SizeTextControl_List;
	afx_msg void OnLbnDblclkListSizeTextControl();
};
