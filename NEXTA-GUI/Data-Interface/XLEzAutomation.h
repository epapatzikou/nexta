// Microsoft Excel Automation Class by Val Golovlev
//http://www.codeguru.com/cpp/data/mfc_database/microsoftexcel/article.php/c11745
// EzAutomation.h: interface for the CXLEzAutomation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EzAutomation_H__D140B9A3_1995_40AC_8E6D_8F23A95A63A2__INCLUDED_)
#define AFX_EzAutomation_H__D140B9A3_1995_40AC_8E6D_8F23A95A63A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "XLAutomation.h"
#define xlNormal -4143

class CXLEzAutomation  
{
public:
	CString m_CurrentFileName;
	BOOL InsertPictureFromBuffer(BYTE* pImage, int Column, int Row, double dXScale, double dYScale);
	BOOL OpenExcelFile(CString szFileName);
	BOOL PlacePictureToClipboard(BYTE* pImage);
	BOOL InsertPictureFromClipboard(int Column, int Row);
	BOOL InsertPictureFromFile(CString szFileName, int Column, int Row);
	CString GetCellValue(int nColumn, int nRow);
	BOOL SaveFileAs(CString szFileName);

	BOOL SaveFile()
	{
	return SaveFileAs(m_CurrentFileName);
	}

	BOOL DeleteRow(int nRow);
	BOOL ReleaseExcel();
	BOOL SetCellValue(int nColumn, int nRow, CString szValue);
	BOOL SetCellValue(int nColumn, int nRow, int number)
	{
	CString szValue;
	szValue.Format("%d",number);
	SetCellValue(nColumn, nRow,szValue);
	return true;
	}

	BOOL SetCellValue(int nColumn, int nRow, float number)
	{
	CString szValue;
	szValue.Format("%f",number);
	SetCellValue(nColumn, nRow,szValue);
	return true;

	}

	BOOL SetCellValue(int nColumn, int nRow, double number)
	{
	CString szValue;
	szValue.Format("%f",number);
	SetCellValue(nColumn, nRow,szValue);
	return true;

	}


	BOOL ExportCString(CString szDataCollection);
	BOOL UpdateXYChart(int nYColumn);
	BOOL CreateXYChart(int nYColumn);

	BOOL SetActiveWorksheet(int nWorksheet)
	{
	m_pXLServer->SetActiveWorksheet(nWorksheet);
	return true;
	}

	BOOL SetActiveWorksheet(CString name)
	{
	 for (int i = 1; i <= m_pXLServer->GetWorksheetsCount (); i++)
	 {
		 CString worksheet_name = m_pXLServer->GetWorksheetName(i);
		if(worksheet_name.Compare (name) ==0)
		{
			m_pXLServer->SetActiveWorksheet(i);
			return true;
		}
	 }
	 return false;
	}



	int AddNewWorkSheet()
	{
	return m_pXLServer->AddNewWorkSheet();
	}

	BOOL SetWorksheetTitle(CString wsTitle)
	{
		return m_pXLServer->SetWorksheetTitle(wsTitle);
	}
	CXLEzAutomation();
	CXLEzAutomation(BOOL bVisible);
	virtual ~CXLEzAutomation();

protected:
	CXLAutomation* m_pXLServer;
};

#endif // !defined(AFX_EzAutomation_H__D140B9A3_1995_40AC_8E6D_8F23A95A63A2__INCLUDED_)
