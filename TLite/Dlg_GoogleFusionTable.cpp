// Dlg_GoogleFusionTable.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "MainFrm.h"

#include "Dlg_GoogleFusionTable.h"


// CDlg_GoogleFusionTable dialog

IMPLEMENT_DYNAMIC(CDlg_GoogleFusionTable, CDialog)

CDlg_GoogleFusionTable::CDlg_GoogleFusionTable(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_GoogleFusionTable::IDD, pParent)
	, m_ProjectFolder(_T(""))
	, m_NodeTableID(_T(""))
	, m_LinkTableID(_T(""))
{
	// m_NodeTableID //m_LinkTableID

}

CDlg_GoogleFusionTable::~CDlg_GoogleFusionTable()
{
}

void CDlg_GoogleFusionTable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ExcelFile5, m_ProjectFolder);
	DDX_Text(pDX, IDC_EDIT_ExcelFile, m_NodeTableID);
	DDX_Text(pDX, IDC_EDIT_ExcelFile2, m_LinkTableID);
}


BEGIN_MESSAGE_MAP(CDlg_GoogleFusionTable, CDialog)

	ON_BN_CLICKED(IDC_BUTTON_Download, &CDlg_GoogleFusionTable::OnBnClickedButtonDownload)
	ON_BN_CLICKED(IDC_BUTTON_Upload, &CDlg_GoogleFusionTable::OnBnClickedButtonUpload)
END_MESSAGE_MAP()


// CDlg_GoogleFusionTable message handlers

BOOL CDlg_GoogleFusionTable::OnInitDialog()
{
m_ProjectFolder = m_pDOC->m_ProjectDirectory;
UpdateData(false);
return true;
}

void CDlg_GoogleFusionTable::OnBnClickedButtonDownload()
{

	UpdateData(true);

	// m_NodeTableID //m_LinkTableID

	STARTUPINFO si = { 0 };  
	PROCESS_INFORMATION pi = { 0 };  

	si.cb = sizeof(si); 

	CWaitCursor curs;
	if(m_pDOC->m_ProjectDirectory.GetLength()==0)
		{
			AfxMessageBox("The project directory has not been specified.");
			return;
		}

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
//	ShellExecute( NULL,  "explore", pMainFrame->m_CurrentDirectory, NULL,  NULL, SW_SHOWNORMAL );

	CString sCommand;
	CString strParam;
	CTime ExeStartTime = CTime::GetCurrentTime();

//	sCommand.Format("%s\\curl.exe - transfer a URL",pMainFrame->m_CurrentDirectory, m_pDOC->m_ProjectDirectory);
	// save to local input_node, input_link 
//	curl "http://www.google.com/fusiontables/api/query?sql=SELECT+*+FROM+380584" >sample.csv

	m_pDOC->ProcessExecute(sCommand, strParam, m_pDOC->m_ProjectDirectory, true);
	CTime ExeEndTime = CTime::GetCurrentTime();
	CTimeSpan ts = ExeEndTime  - ExeStartTime;

	// message box here
}

void CDlg_GoogleFusionTable::OnBnClickedButtonUpload()
{
// http://skipperkongen.dk/2011/01/05/google-fusion-tables-cheat-sheet/
	// TODO: Add your control notification handler code here
	// read local sql files
	// login with default gmail account
	// insert into the specified table id
}
