
#include "stdafx.h"
#include "XLEzAutomation.h"
#include "XLTestDataSource.h"

//Start static Excel Automation demo
void g_RunStaticExcel() 
{
	//CXLEzAutomation class constructor starts Excel and creates empty worksheet  
	CXLEzAutomation XL;
			//Close Excel if failed to open file 
			if(!XL.OpenExcelFile("C:\\NEXTA_OpenSource\\DTALite\\Test.xls"))
			{
				XL.ReleaseExcel();
//				MessageBox("Failed to Open Excel File", "Error", MB_OK);
				return;
			}

			XL.SetActiveWorksheet(0);
			//To access data use this:
			CString szCellValue1 = XL.GetCellValue(1, 1);
			CString szCellValue2 = XL.GetCellValue(1, 2);
			XL.SetCellValue(1,4,"100");
			CString szCellValue5 = XL.GetCellValue(1, 5);
			//Itterate through all cells given by (Column, Row) to
			//access all data

			//CTestDataSource provide set of data for testing XY plot function
	CTestDataSource DataSource;

	//All data will be stored by szData
	CString szData;
	//Itterate to fill data buffer with 100 data points
	for(int i = 0; i < 100; i++)
		szData = szData + DataSource.GetNextXYPoint();
	
	//Use clipboard export function to move all data to Excel worksheet
	XL.ExportCString(szData);
	
	//Y values for this plot are in Column = 2
	XL.CreateXYChart(2);
	
//	XL.EnableAlert(false);
	XL.ReleaseExcel();

}
/*
To feed formula to Excel:  create a string that starts with b=b, i.e., the same way as it is done in Excel. Send the string to the desired cell in Excel. 
In this example we feed in Y = 2*X + 1 to B2 cell, i.e.,   Y = B2  and X is taken from A1 cell:
{
CString szFormula = _T(b= 2*A1 + 1b);
XL. SetCellValue(2,1,  szFormula);
}
*/