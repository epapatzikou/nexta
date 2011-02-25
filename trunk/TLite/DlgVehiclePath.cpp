// DlgVehiclePath.cpp : implementation file
//
// DlgPathMOE.cpp : implementation file
//
//  Portions Copyright 2010 Xuesong Zhou (xzhou99@gmail.com)

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
#include "DlgVehiclePath.h"
#include "stdafx.h"
#include "TLite.h"
#include "BaseDialog.h"
#include "Network.h"
#include "TLiteDoc.h"

static _TCHAR *_gVehColumnMOELabel[NUM_VEHPATHMOES] =
{
	_T("Origin"), _T("Destination"),  _T("User Class"), _T("DepartureTime"), 
	_T("Path Index"), _T("# of Vehicles"), _T("Avg Travel Time"), _T("Vehicle ID")
};

static int _gVehColumnFormat[NUM_VEHPATHMOES] =
{
	LVCFMT_LEFT, LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_CENTER
};

// CDlgVehiclePath dialog

IMPLEMENT_DYNAMIC(CDlgVehiclePath, CBaseDialog)

CDlgVehiclePath::CDlgVehiclePath(CWnd* pParent /*=NULL*/)
	: CBaseDialog(CDlgVehiclePath::IDD, pParent)
{

}

CDlgVehiclePath::~CDlgVehiclePath()
{
}

void CDlgVehiclePath::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_PATH_LIST_CONTROL, m_VehiclePathList);
//	DDX_Control(pDX, IDC_ODLIST, C_ODList);
}


BEGIN_MESSAGE_MAP(CDlgVehiclePath, CDialog)
END_MESSAGE_MAP()


// CDlgVehiclePath message handlers

BOOL CDlgVehiclePath::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	int i;
	LV_COLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for(i = 0; i<NUM_VEHPATHMOES; i++)
	{
		lvc.iSubItem = i;
		lvc.pszText = _gVehColumnMOELabel[i];
		lvc.cx = 90;
		lvc.fmt = _gVehColumnFormat[i];
		m_VehiclePathList.InsertColumn(i,&lvc);
	}

	m_VehiclePathList.SetExtendedStyle(LVS_EX_AUTOSIZECOLUMNS | LVS_EX_FULLROWSELECT |LVS_EX_HEADERDRAGDROP);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
