// DlgScenario.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "DlgScenario.h"

#include <map>
#include <vector>
#include <string>
#include <sstream>

// CDlgScenario dialog
static LPTSTR ELEMENTS[6] = {"Incident","Link_Based_Toll","Distance_Based_Toll",
							"Dynamic_Message_Sign","Ramp_Metering", "Work_Zone"};

//static string DefaultNames[6][] = {
//	{"Link","Type","Day_No","Start_Time_In_Min","End_Time_In_Min","Capacity_Reduction_Rate","Speed_Limit"},
//	{"Link","Start_Time_In_Min","End_Time_In_Min","HOV_Dollar_Per_Link","LOV_Dollar_Per_Link","Truck_Dollar_Per_Link"},
//	{"Link","Start_Time_In_Min","End_Time_In_Min","HOV_Dollar_Per_Mile","LOV_Dollar_Per_Mile","Truck_Dollar_Per_Mile"},
//	{"Link","Type","Start_Time_In_Min","End_Time_In_Min"},
//	{"Link","Start_Time_In_Min","End_Time_In_Min","Metering_Rate"},
//	{"Link","Type","Day_No","Start_Time_In_Min","End_Time_In_Min","Capacity_Reduction_Rate","Speed_Limit"}
//};

IMPLEMENT_DYNAMIC(CDlgScenario, CDialog)

CDlgScenario::CDlgScenario(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgScenario::IDD, pParent)
{

}

CDlgScenario::~CDlgScenario()
{
}


std::vector<std::string> CDlgScenario::GetHeaderList(int i)
{
	std::vector<std::string> HeaderList;

	switch (i)
	{
	case 0:
		HeaderList.push_back("Link");
		HeaderList.push_back("Type");
		HeaderList.push_back("Day_No");
		HeaderList.push_back("Start_Time_In_Min");
		HeaderList.push_back("End_Time_In_Min");
		HeaderList.push_back("Capacity_Reduction_Rate");
		HeaderList.push_back("Speed_Limit");
		break;
	case 1:
		HeaderList.push_back("Link");
		HeaderList.push_back("Start_Time_In_Min");
		HeaderList.push_back("End_Time_In_Min");
		HeaderList.push_back("HOV_Dollar_Per_Link");
		HeaderList.push_back("LOV_Dollar_Per_Link");
		HeaderList.push_back("Truck_Dollar_Per_Link");
		break;
	case 2:
		HeaderList.push_back("Link");
		HeaderList.push_back("Start_Time_In_Min");
		HeaderList.push_back("End_Time_In_Min");
		HeaderList.push_back("HOV_Dollar_Per_Mile");
		HeaderList.push_back("LOV_Dollar_Per_Mile");
		HeaderList.push_back("Truck_Dollar_Per_Mile");
		break;
	case 3:
		HeaderList.push_back("Link");
		HeaderList.push_back("Type");
		HeaderList.push_back("Start_Time_In_Min");
		HeaderList.push_back("End_Time_In_Min");
		break;
	case 4:
		HeaderList.push_back("Link");
		HeaderList.push_back("Start_Time_In_Min");
		HeaderList.push_back("End_Time_In_Min");
		HeaderList.push_back("Metering_Rate");
		break;
	case 5:
		HeaderList.push_back("Link");
		HeaderList.push_back("Type");
		HeaderList.push_back("Day_No");
		HeaderList.push_back("Start_Time_In_Min");
		HeaderList.push_back("End_Time_In_Min");
		HeaderList.push_back("Capacity_Reduction_Rate");
		HeaderList.push_back("Speed_Limit");
		break;
	}
	return HeaderList;
}
void CDlgScenario::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCENARIO_TAB, m_TabCtrl);
}

BOOL IsValidElement(LPTSTR element)
{
	BOOL Found = FALSE;

	for (int i=0;i<6;i++)
	{
		if (strcmp(ELEMENTS[i],element) == 0)
		{
			Found = TRUE;
		}
	}

	return Found;
}

BOOL CDlgScenario::OnInitDialog()
{
	using std::string;
	using std::map;
	using std::vector;

	CDialog::OnInitDialog();

	vector<string> name_vector;
	vector<vector<string>> value_vector;

	//Construct link string 
	std::vector<std::string> LinkString = GetLinkString();

	for (int i=0;i < 6;i++)
	{
		name_vector.clear();
		value_vector.clear();

		//Read individual xml file
		ReadXMLFile(ELEMENTS[i],name_vector,value_vector);

		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		tcItem.pszText = _T(ELEMENTS[i]);
		m_TabCtrl.InsertItem(i, &tcItem);


		//add individual tab dialog
		p_SubTabs[i] = new CDlgScenarioTab(GetHeaderList(i),value_vector,LinkString);
		p_SubTabs[i]->SetTabText(ELEMENTS[i]);
		p_SubTabs[i]->Create(IDD_DIALOG_SCENARIO_TAB,&m_TabCtrl);

	}

	p_SubTabs[0]->ShowWindow(SW_SHOW);

	for (int i=1;i<6;i++)
	{
		p_SubTabs[i]->ShowWindow(SW_HIDE);
	}

	m_PrevTab=0;

	SetRectangle();

	return TRUE;
}

void CDlgScenario::SetRectangle()
{
	CRect tabRect, itemRect;
	int nX, nY, nXc, nYc;

	m_TabCtrl.GetClientRect(&tabRect);
	m_TabCtrl.GetItemRect(0, &itemRect);

	nX=itemRect.left;
	nY=itemRect.bottom+1;
	nXc=tabRect.right-itemRect.left-1;
	nYc=tabRect.bottom-nY-1;

	p_SubTabs[0]->SetWindowPos(&m_TabCtrl.wndTop, nX, nY, nXc, nYc, SWP_SHOWWINDOW);
	for(int nCount=1; nCount < 6; nCount++)
	{
		p_SubTabs[nCount]->SetWindowPos(&m_TabCtrl.wndTop, nX, nY, nXc, nYc, SWP_HIDEWINDOW);
	}
}

BEGIN_MESSAGE_MAP(CDlgScenario, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgScenario::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgScenario::OnBnClickedCancel)
	ON_NOTIFY(TCN_SELCHANGE, IDC_SCENARIO_TAB, &CDlgScenario::OnTcnSelchangeScenarioTab)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CDlgScenario::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CDlgScenario::OnBnClickedButtonDelete)
END_MESSAGE_MAP()


// CDlgScenario message handlers

void CDlgScenario::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	std::string FirstLine = "<?xml version=\"1.0\"?>\n";
		
	for (int i=0;i<6;i++)
	{
		if (p_SubTabs[i]->ValidityCheck() != 0)
		{
			if (m_PrevTab != i)
			{
				p_SubTabs[m_PrevTab]->EnableWindow(FALSE);
				p_SubTabs[m_PrevTab]->ShowWindow(SW_HIDE);
				m_PrevTab = i;
				p_SubTabs[m_PrevTab]->EnableWindow(TRUE);
				p_SubTabs[m_PrevTab]->ShowWindow(SW_SHOW);
				m_TabCtrl.SetCurSel(i);
			}

			return;
		}

		std::string Str = p_SubTabs[i]->GenerateRecordString();

		if (Str.length() > 0)
		{
			std::ofstream outFile(m_pDoc->m_ProjectDirectory + p_SubTabs[i]->GetTabText() + ".xml");

			if (outFile.is_open())
			{
				outFile << FirstLine;
				outFile << Str;
			}

			outFile.close();
		}
		else
		{
			remove(m_pDoc->m_ProjectDirectory + p_SubTabs[i]->GetTabText() + ".xml");
		}
	}

	OnOK();
}

void CDlgScenario::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


void CDlgScenario::OnTcnSelchangeScenarioTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	if (m_PrevTab == m_TabCtrl.GetCurSel())
		return;
	p_SubTabs[m_PrevTab]->EnableWindow(FALSE);
	p_SubTabs[m_PrevTab]->ShowWindow(SW_HIDE);
	m_PrevTab = m_TabCtrl.GetCurSel();
	p_SubTabs[m_PrevTab]->EnableWindow(TRUE);
	p_SubTabs[m_PrevTab]->ShowWindow(SW_SHOW);

	*pResult = 0;
}

void CDlgScenario::OnBnClickedButtonAdd()
{
	// TODO: Add your control notification handler code here
	int cur_tab = m_TabCtrl.GetCurSel();
	p_SubTabs[cur_tab]->AddRow();
}

void CDlgScenario::OnBnClickedButtonDelete()
{
	// TODO: Add your control notification handler code here
	int cur_tab = m_TabCtrl.GetCurSel();
	p_SubTabs[cur_tab]->DeleteRow();
}

std::vector<std::string> CDlgScenario::GetLinkString()
{
	std::ifstream inFile;
	std::vector<std::string> linkstring;
	if (m_pDoc != NULL)
	{
		inFile.open(m_pDoc->m_ProjectDirectory + "input_link.csv");
		if (inFile.is_open())
		{
			std::string line;

			//skip header line
			std::getline(inFile,line);

			while(std::getline(inFile,line))
			{
				string subStr;
				string str = "[";

				std::istringstream lineStringStream(line);

				//from_node
				getline(lineStringStream,subStr,',');
				str += subStr + ",";

				//to_node
				getline(lineStringStream,subStr,',');
				str += subStr + "]";
				
				linkstring.push_back(str);
			}
		}
	}

	return linkstring;
}


BOOL CDlgScenario::ReadXMLFile(const char* ElementType, std::vector<std::string>& name_vector,std::vector<std::vector<std::string>>& value_vector)
{
	std::string fileName = m_pDoc->m_ProjectDirectory + ElementType + ".xml";
	BOOL Recorded = FALSE;
	std::vector<std::string> value;

	char* buffer = new char[fileName.length()+1];

	strcpy(buffer,fileName.c_str());

	if (m_XMLFile.LoadFromFile(buffer))
	{
		CXMLElement* p_Root = m_XMLFile.GetRoot();

		CXMLElement* p_Child = p_Root->GetFirstChild();

		while(p_Child)
		{
			LPTSTR ElementType_Read = p_Child->GetElementName();

			if (strcmp(ElementType,ElementType_Read) == 0)
			{
				CXMLElement* p_Element = p_Child->GetFirstChild();

				while(p_Element)
				{
					if (p_Element->GetElementType() == XET_ATTRIBUTE)
					{
						LPTSTR ElementName = p_Element->GetElementName();
						if (!Recorded)
						{
							name_vector.push_back(string(ElementName));
						}

						LPTSTR ElementValue = p_Element->GetValue();
						value.push_back(string(ElementValue));
					}

					p_Element = p_Child->GetNextChild();
				}

				if (!Recorded)
				{
					Recorded = TRUE;
				}
				value_vector.push_back(value);
				value.clear();
			}
			p_Child = p_Root->GetNextChild();
		}
	}

	return TRUE;
}