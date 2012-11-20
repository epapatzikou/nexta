//TLiteDoc.cpp : implementation of the CTLiteDoc class
//ortions Copyright 2012 Xuesong Zhou (xzhou99@gmail.com), Jeff Taylor (jeffrey.taylor.d@gmail.com)

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
#include "..//Geometry.h"
#include "..//CSVParser.h"
#include "..//TLite.h"
#include "..//Network.h"
#include "..//TLiteDoc.h"
#include "..//TLiteView.h"
#include "..//TLiteView.h"
#include "SafetyPlanning.h"



void CTLiteDoc::OnSafetyplanningtoolsRun()
{
	m_bRunCrashPredictionModel = true;
	CWaitCursor;
	CCSVWriter SafetyPredictionFile;

	CString file_str = m_ProjectDirectory +"output_crash_prediction.csv";

	 // Convert a TCHAR string to a LPCSTR
	  CT2CA pszConvertedAnsiString (file_str);

	  // construct a std::string using the LPCSTR input
	  std::string strStd (pszConvertedAnsiString);

	  double total_volume  = 0;
	  double max_number_of_crashes_per_mile = 0;
	 if(SafetyPredictionFile.Open(strStd))
	 {


		SafetyPredictionFile.SetFieldName ("node_id");

	SafetyPredictionFile.SetFieldName("link_id");
	SafetyPredictionFile.SetFieldName("from_node");
	SafetyPredictionFile.SetFieldName("to_node");
	SafetyPredictionFile.SetFieldName("length_mi");
	SafetyPredictionFile.SetFieldName("sim_volume");
	SafetyPredictionFile.SetFieldName("aadt");
	SafetyPredictionFile.SetFieldName("all_crash");
	SafetyPredictionFile.SetFieldName("seg_crash");
	SafetyPredictionFile.SetFieldName("int_crash");
	SafetyPredictionFile.SetFieldName("fi_crash");
	SafetyPredictionFile.SetFieldName("pdo_crash");
	SafetyPredictionFile.SetFieldName("seg_fi");
	SafetyPredictionFile.SetFieldName("seg_pdo");
	SafetyPredictionFile.SetFieldName("seg_sv_fi");
	SafetyPredictionFile.SetFieldName("seg_sv_pdo");
	SafetyPredictionFile.SetFieldName("seg_mv_fi");
	SafetyPredictionFile.SetFieldName("seg_mv_pdo");
	SafetyPredictionFile.SetFieldName("seg_dvw_fi");
	SafetyPredictionFile.SetFieldName("seg_dvw_pd");
	SafetyPredictionFile.SetFieldName("int_fi");
	SafetyPredictionFile.SetFieldName("int_pdo");
	SafetyPredictionFile.SetFieldName("int_3st");
	SafetyPredictionFile.SetFieldName("int_3sg");
	SafetyPredictionFile.SetFieldName("int_4st");
	SafetyPredictionFile.SetFieldName("int_4sg");
	SafetyPredictionFile.SetFieldName("int_3st_pd");
	SafetyPredictionFile.SetFieldName("int_3sg_pd");
	SafetyPredictionFile.SetFieldName("int_4st_pd");
	SafetyPredictionFile.SetFieldName("int_4sg_pd");
	SafetyPredictionFile.SetFieldName("int_3st_fi");
	SafetyPredictionFile.SetFieldName("int_3sg_fi");
	SafetyPredictionFile.SetFieldName("int_4st_fi");
	SafetyPredictionFile.SetFieldName("int_4sg_fi");
	SafetyPredictionFile.SetFieldName("all_crash_pm");
	SafetyPredictionFile.SetFieldName("seg_crash_pm");
	SafetyPredictionFile.SetFieldName("int_crash_pm");
	SafetyPredictionFile.SetFieldName("fi_crash_pm");
	SafetyPredictionFile.SetFieldName("pdo_crash_pm");
	SafetyPredictionFile.SetFieldName("seg_fi_pm");
	SafetyPredictionFile.SetFieldName("seg_pdo_pm");
	SafetyPredictionFile.SetFieldName("seg_sv_fi_pm");
	SafetyPredictionFile.SetFieldName("seg_sv_pdo_pm");
	SafetyPredictionFile.SetFieldName("seg_mv_fi_pm");
	SafetyPredictionFile.SetFieldName("seg_mv_pdo_pm");
	SafetyPredictionFile.SetFieldName("seg_dvw_fi_pm");
	SafetyPredictionFile.SetFieldName("seg_dvw_pd_pm");
	SafetyPredictionFile.SetFieldName("int_fi_pm");
	SafetyPredictionFile.SetFieldName("int_pdo_pm");
	SafetyPredictionFile.SetFieldName("int_3st_pm");
	SafetyPredictionFile.SetFieldName("int_3sg_pm");
	SafetyPredictionFile.SetFieldName("int_4st_pm");
	SafetyPredictionFile.SetFieldName("int_4sg_pm");
	SafetyPredictionFile.SetFieldName("int_3st_pd_pm");
	SafetyPredictionFile.SetFieldName("int_3sg_pd_pm");
	SafetyPredictionFile.SetFieldName("int_4st_pd_pm");
	SafetyPredictionFile.SetFieldName("int_4sg_pd_pm");
	SafetyPredictionFile.SetFieldName("int_3st_fi_pm");
	SafetyPredictionFile.SetFieldName("int_3sg_fi_pm");
	SafetyPredictionFile.SetFieldName("int_4st_fi_pm");
	SafetyPredictionFile.SetFieldName("int_4sg_fi_pm");
	SafetyPredictionFile.SetFieldName("geometry");

	SafetyPredictionFile.SetFieldName("all_crash_pv");

	 SafetyPredictionFile.WriteHeader();


	DTASafetyPredictionModel SafePredictionModel;
			std::list<DTALink*>::iterator iLink;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
		
		DTALink* pLink = (*iLink);

		pLink->m_AADT = pLink->m_total_link_volume/max(pLink->m_AADT_conversion_factor,0.0001) ;
		total_volume+= pLink->m_AADT;

		double CrashRate= 0;
		if( m_LinkTypeMap[pLink->m_link_type].IsFreeway ())  // freeway
		{
			pLink->m_NumberOfCrashes  = SafePredictionModel.EstimateFreewayCrashRatePerYear(pLink->m_AADT , pLink->m_Length );
		}

		if( m_LinkTypeMap[pLink->m_link_type].IsArterial ())   //arterial
		{

		// gather two-way data
		// sum of two-way AADT
		// make prediction
		// split crash statistics to two different directions, according to AADT
			float two_way_AADT = pLink->m_AADT;
			float crash_ratio_on_this_link = 1.0f;  // default value

			DTALink* pReversedLink = NULL;

			unsigned long ReversedLinkKey = GetLinkKey(pLink->m_ToNodeID, pLink->m_FromNodeID);

			int reversed_link_id = 0;
			if ( m_NodeIDtoLinkMap.find ( ReversedLinkKey) != m_NodeIDtoLinkMap.end())
			{
				  pReversedLink = m_NodeIDtoLinkMap[ReversedLinkKey];
			}

	
			if(pReversedLink != NULL)
			{ 
				two_way_AADT += pReversedLink->m_AADT;
				crash_ratio_on_this_link =  pLink->m_AADT/max(1,two_way_AADT);
			}


			pLink->m_NumberOfCrashes  = 
				 SafePredictionModel.EstimateArterialCrashRatePerYear(crash_ratio_on_this_link, 
				 pLink->m_NumberOfFatalAndInjuryCrashes, 
				 pLink->m_NumberOfPDOCrashes,
				 pLink->m_Intersection_NumberOfCrashes, 
				 pLink->m_Intersection_NumberOfFatalAndInjuryCrashes, pLink->m_Intersection_NumberOfPDOCrashes,
				two_way_AADT, 
				pLink->m_Length,
				pLink->m_Num_Driveways_Per_Mile,
				pLink->m_volume_proportion_on_minor_leg,
				pLink->m_Num_3SG_Intersections,
				pLink->m_Num_3ST_Intersections,
				pLink->m_Num_4SG_Intersections,
				pLink->m_Num_4ST_Intersections);

		}
		

	SafetyPredictionFile.SetValueByFieldName("link_id",pLink->m_LinkID );

	SafetyPredictionFile.SetValueByFieldName("from_node",pLink->m_FromNodeNumber );
	SafetyPredictionFile.SetValueByFieldName("to_node",pLink->m_ToNodeNumber );
	SafetyPredictionFile.SetValueByFieldName("length_mi",pLink->m_Length );
	SafetyPredictionFile.SetValueByFieldName("sim_volume",pLink->m_total_link_volume);
	SafetyPredictionFile.SetValueByFieldName("aadt",pLink->m_AADT );
	SafetyPredictionFile.SetValueByFieldName("all_crash",pLink->m_NumberOfCrashes);

	double seg_crash = SafePredictionModel.Nsv_total + SafePredictionModel.Nmvnd_total + SafePredictionModel.Nmvd_total;
	SafetyPredictionFile.SetValueByFieldName("seg_crash", seg_crash );
	SafetyPredictionFile.SetValueByFieldName("int_crash", pLink->m_Intersection_NumberOfCrashes);
	SafetyPredictionFile.SetValueByFieldName("fi_crash", pLink->m_NumberOfFatalAndInjuryCrashes);
	//SafetyPredictionFile.SetValueByFieldName("pdo_crash");
	//SafetyPredictionFile.SetValueByFieldName("seg_fi");
	//SafetyPredictionFile.SetValueByFieldName("seg_pdo");
	//SafetyPredictionFile.SetValueByFieldName("seg_sv_fi");
	//SafetyPredictionFile.SetValueByFieldName("seg_sv_pdo");
	//SafetyPredictionFile.SetValueByFieldName("seg_mv_fi");
	//SafetyPredictionFile.SetValueByFieldName("seg_mv_pdo");
	//SafetyPredictionFile.SetValueByFieldName("seg_dvw_fi");
	//SafetyPredictionFile.SetValueByFieldName("seg_dvw_pd");
	//SafetyPredictionFile.SetValueByFieldName("int_fi");
	//SafetyPredictionFile.SetValueByFieldName("int_pdo");
	//SafetyPredictionFile.SetValueByFieldName("int_3st");
	//SafetyPredictionFile.SetValueByFieldName("int_3sg");
	//SafetyPredictionFile.SetValueByFieldName("int_4st");
	//SafetyPredictionFile.SetValueByFieldName("int_4sg");
	//SafetyPredictionFile.SetValueByFieldName("int_3st_pd");
	//SafetyPredictionFile.SetValueByFieldName("int_3sg_pd");
	//SafetyPredictionFile.SetValueByFieldName("int_4st_pd");
	//SafetyPredictionFile.SetValueByFieldName("int_4sg_pd");
	//SafetyPredictionFile.SetValueByFieldName("int_3st_fi");
	//SafetyPredictionFile.SetValueByFieldName("int_3sg_fi");
	//SafetyPredictionFile.SetValueByFieldName("int_4st_fi");
	//SafetyPredictionFile.SetValueByFieldName("int_4sg_fi");

	double all_crash_pm = pLink->m_NumberOfCrashes/ max(0.0001,pLink->m_Length );
	SafetyPredictionFile.SetValueByFieldName("all_crash_pm",all_crash_pm);

		if (max_number_of_crashes_per_mile < all_crash_pm)
		{
			max_number_of_crashes_per_mile = all_crash_pm;
		}

	double all_crash_pv =  pLink->m_NumberOfCrashes/ max(0.0001,pLink->m_Length )/ max(1,pLink->m_AADT);
	SafetyPredictionFile.SetValueByFieldName("all_crash_pv",all_crash_pv);

	//SafetyPredictionFile.SetValueByFieldName("seg_crash_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_crash_pm");
	//SafetyPredictionFile.SetValueByFieldName("fi_crash_pm");
	//SafetyPredictionFile.SetValueByFieldName("pdo_crash_pm");
	//SafetyPredictionFile.SetValueByFieldName("seg_fi_pm");
	//SafetyPredictionFile.SetValueByFieldName("seg_pdo_pm");
	//SafetyPredictionFile.SetValueByFieldName("seg_sv_fi_pm");
	//SafetyPredictionFile.SetValueByFieldName("seg_sv_pdo_pm");
	//SafetyPredictionFile.SetValueByFieldName("seg_mv_fi_pm");
	//SafetyPredictionFile.SetValueByFieldName("seg_mv_pdo_pm");
	//SafetyPredictionFile.SetValueByFieldName("seg_dvw_fi_pm");
	//SafetyPredictionFile.SetValueByFieldName("seg_dvw_pd_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_fi_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_pdo_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_3st_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_3sg_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_4st_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_4sg_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_3st_pd_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_3sg_pd_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_4st_pd_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_4sg_pd_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_3st_fi_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_3sg_fi_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_4st_fi_pm");
	//SafetyPredictionFile.SetValueByFieldName("int_4sg_fi_pm");

	std::string  geo_string = "\"" + pLink->m_geo_string + "\"";

	SafetyPredictionFile.SetValueByFieldName("geometry",geo_string);

	SafetyPredictionFile.WriteRecord ();

		}  // for each link
		}
		

		if(total_volume<0.1)
		{
		
		AfxMessageBox("Simulated link volume data are not available.\nPlease run simulation to generate volume results.");
		return;
		}


	m_LOSBound[MOE_safety][1] = max_number_of_crashes_per_mile*1/7.0;
	m_LOSBound[MOE_safety][2] =  max_number_of_crashes_per_mile*2/7.0;
	m_LOSBound[MOE_safety][3] = max_number_of_crashes_per_mile*3/7.0;;
	m_LOSBound[MOE_safety][4] = max_number_of_crashes_per_mile*4/7.0;;
	m_LOSBound[MOE_safety][5] = max_number_of_crashes_per_mile*5/7.0;;
	m_LOSBound[MOE_safety][6] = max_number_of_crashes_per_mile*6/7.0;;
	m_LOSBound[MOE_safety][7] =  max_number_of_crashes_per_mile*7/7.0;



		CString message;
		message.Format("Crash prediction model completes for %d links.",m_LinkSet.size());

		AfxMessageBox(message, MB_ICONINFORMATION);
}


void CTLiteDoc::OnSafetyplanningtoolsGeneratenode()
{
	CString csv_file_str = m_ProjectDirectory +"output_crash_prediction.csv";
	CString shape_file_str = m_ProjectDirectory +"output_crash_prediction.shp";

	DeleteFile(shape_file_str);
	ConvertLinkCSV2ShapeFiles(csv_file_str,shape_file_str, "ESRI Shapefile",GIS_Line_Type);

}