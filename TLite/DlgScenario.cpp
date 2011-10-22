// DlgScenario.cpp : implementation file
//  Portions Copyright 2011 Hao Lei(haolei.sc@gmail.com), Xuesong Zhou (xzhou99@gmail.com)

//   If you help write or modify the code, please also list your names here.
//   The reason of having Copyright info here is to ensure all the modified version, as a whole, under the GPL 
//   and further prevent a violation of the GPL.

// More about "How to use GNU licenses for your own software"
// http://www.gnu.org/licenses/gpl-howto.html


//    This file is part of NeXTA Version 3 (Open-source).

//    NEXTA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    NEXTA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with NEXTA.  If not, see <http://www.gnu.org/licenses/>.



#include "stdafx.h"
#include "TLite.h"
#include "Geometry.h"
#include "CSVParser.h"
#include "DlgScenario.h"

#include <map>
#include <vector>
#include <string>
#include <sstream>

#define _MAX_SCENARIO_SIZE 6
// CDlgScenario dialog
static LPTSTR ELEMENTS[_MAX_SCENARIO_SIZE] = {"Incident","Link Based Toll","Distance Based_Toll",
							"Dynamic Message Sign","Ramp Metering", "Work Zone"};


IMPLEMENT_DYNAMIC(CDlgScenario, CDialog)

CDlgScenario::CDlgScenario(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgScenario::IDD, pParent)
{

}

CDlgScenario::~CDlgScenario()
{
}

void CDlgScenario::GetDefaultInfo(int i, std::vector<std::string>& HeaderList, std::vector<CString>& DefaultList)
{

	switch (i)
	{
	case 0:
		HeaderList.push_back("Link");
		HeaderList.push_back("Start Time in Min");
		HeaderList.push_back("End Time in min");
		HeaderList.push_back("Capacity Reduction Percentage (%)");

		DefaultList.push_back("0");
		DefaultList.push_back("60");
		DefaultList.push_back("80");
		break;
	case 1:
		HeaderList.push_back("Link");
		HeaderList.push_back("Start Time in Min");
		HeaderList.push_back("End Time in min");
		HeaderList.push_back("Charge for LOV ($)");
		HeaderList.push_back("Charge for HOV ($)");
		HeaderList.push_back("Charge for Truck ($)");

		DefaultList.push_back("0");
		DefaultList.push_back("60");
		DefaultList.push_back("0");
		DefaultList.push_back("1");
		DefaultList.push_back("1.5");
		break;
	case 2:
		HeaderList.push_back("Link");
		HeaderList.push_back("Start Time in Min");
		HeaderList.push_back("End Time in min");
		HeaderList.push_back("Charge for LOV ($/mile)");
		HeaderList.push_back("Charge for HOV ($/mile)");
		HeaderList.push_back("Charge for Truck ($/mile)");

		DefaultList.push_back("0");
		DefaultList.push_back("60");
		DefaultList.push_back("0.5");
		DefaultList.push_back("0");
		DefaultList.push_back("1");
		break;
	case 3:
		HeaderList.push_back("Link");
		HeaderList.push_back("Start Time in Min");
		HeaderList.push_back("End Time in min");
		HeaderList.push_back("Responce Percentage (%)");

		DefaultList.push_back("0");
		DefaultList.push_back("60");
		DefaultList.push_back("20");
		break;
	case 4:
		HeaderList.push_back("Link");
		HeaderList.push_back("Start Time in Min");
		HeaderList.push_back("End Time in min");
		HeaderList.push_back("Metering Rate");

		DefaultList.push_back("0");
		DefaultList.push_back("60");
		DefaultList.push_back("1500");
		break;
	case 5:
		HeaderList.push_back("Link");
		HeaderList.push_back("Day No");
		HeaderList.push_back("Start Time in Min");
		HeaderList.push_back("End Time in min");
		HeaderList.push_back("Capacity Reduction Percentage (%)");
		HeaderList.push_back("Speed Limit (mph)");

		DefaultList.push_back("1");
		DefaultList.push_back("0");
		DefaultList.push_back("60");
		DefaultList.push_back("40");
		DefaultList.push_back("45");
		break;
	case 6:
		HeaderList.push_back("Link");
		HeaderList.push_back("Start Time in Min");
		HeaderList.push_back("End Time in min");
		HeaderList.push_back("Variable Speed Limit in mph");

		DefaultList.push_back("0");
		DefaultList.push_back("60");
		DefaultList.push_back("60");

		break;
	}
}
void CDlgScenario::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCENARIO_TAB, m_TabCtrl);
}

BOOL IsValidElement(LPTSTR element)
{
	BOOL Found = FALSE;

	for (int i=0;i<_MAX_SCENARIO_SIZE;i++)
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

	for (int i=0;i < _MAX_SCENARIO_SIZE;i++)
	{
		name_vector.clear();
		value_vector.clear();

		//Read individual xml file
		ReadXMLFile(ELEMENTS[i],name_vector,value_vector);

		TCITEM tcItem;
		tcItem.mask = TCIF_TEXT;
		tcItem.pszText = _T(ELEMENTS[i]);
		m_TabCtrl.InsertItem(i, &tcItem);


		std::vector<string> DefaultHeader;
		std::vector<CString> DefaultValue;

		GetDefaultInfo(i,DefaultHeader,DefaultValue);
		//add individual tab dialog
		p_SubTabs[i] = new CDlgScenarioTab(DefaultHeader, DefaultValue, value_vector,LinkString);
		p_SubTabs[i]->SetTabText(ELEMENTS[i]);
		p_SubTabs[i]->Create(IDD_DIALOG_SCENARIO_TAB,&m_TabCtrl);

	}

	p_SubTabs[0]->ShowWindow(SW_SHOW);

	for (int i=1;i<_MAX_SCENARIO_SIZE;i++)
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
	for(int nCount=1; nCount < _MAX_SCENARIO_SIZE; nCount++)
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
		
	for (int i=0;i<_MAX_SCENARIO_SIZE;i++)
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

	m_pDoc->ReadScenarioData();
	m_pDoc->UpdateAllViews(0);

}

void CDlgScenario::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


void CDlgScenario::OnTcnSelchangeScenarioTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here

	//Error Checking before leaving current tab
	if (p_SubTabs[m_PrevTab]->ValidityCheck() != 0)
	{
		m_TabCtrl.SetCurSel(m_PrevTab);
		return;
	}
	else
	{
		if (m_PrevTab == m_TabCtrl.GetCurSel())	return;

		//Disable and hide previous tab
		p_SubTabs[m_PrevTab]->EnableWindow(FALSE);
		p_SubTabs[m_PrevTab]->ShowWindow(SW_HIDE);

		//Enable and show current tab
		m_PrevTab = m_TabCtrl.GetCurSel();
		p_SubTabs[m_PrevTab]->EnableWindow(TRUE);
		p_SubTabs[m_PrevTab]->ShowWindow(SW_SHOW);
	}

	
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
	std::vector<std::string> linkstring;

	if (m_pDoc != NULL)
	{
	long i = 0;
	DTALink* pLink = 0;
	float default_distance_sum=0;
	float length_sum = 0;
	CCSVParser parser;

	CString TempStr(m_pDoc->m_ProjectDirectory + "input_link.csv");
	std::string std_string(TempStr, TempStr.GetLength());

	if (parser.OpenCSVFile(std_string))
	{
		while(parser.ReadRecord())
		{
			int from_node_id;
			int to_node_id;
			int direction;

			if(!parser.GetValueByFieldName("from_node_id",from_node_id)) 
			{
				AfxMessageBox("Field from_node_id has not been defined in file input_link.csv. Please check.");
				break;
			}
			if(!parser.GetValueByFieldName("to_node_id",to_node_id))
			{
				AfxMessageBox("Field to_node_id has not been defined in file input_link.csv. Please check.");
				break;
			}

			if(!parser.GetValueByFieldName("direction",direction))
					direction = 1;

		int link_code_start = 1;
			int link_code_end = 1;

			if (direction == -1) // reversed
			{
				link_code_start = 2; link_code_end = 2;
			}

			if (direction == 0) // two-directional link
			{
				link_code_start = 1; link_code_end = 2;
			}

			for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
			{

			std::stringstream from_node_id_out;
			std::stringstream to_node_id_out;

			if(link_code == 1)
			{
			from_node_id_out << from_node_id;
			to_node_id_out << to_node_id;
			}
			if(link_code == 2)
			{
			from_node_id_out << to_node_id;
			to_node_id_out << from_node_id;
			}

				string subStr;
				string str = "[" + from_node_id_out.str() + "," + to_node_id_out.str() + "]";
				
				linkstring.push_back(str);
			}
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