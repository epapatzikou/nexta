//  Portions Copyright 2010 Tao Xing (captainxingtao@gmail.com), Xuesong Zhou (xzhou99@gmail.com)
//

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
#include "..//TLite.h"
#include "..//Network.h"
#include "..//TLiteDoc.h"
#ifndef _WIN64
#include "..//Data-Interface//include//ogrsf_frmts.h"
#endif 
#include "..//MainFrm.h"

#include "SignalNode.h"
#include "..//Dlg_SignalDataExchange.h"
#include "..//CSVParser.h"
#include "..//Geometry.h"


void CTLiteDoc::OnExportAms()
{
	const int NodeColumnSize = 92;
	CString node_Column_name_str[NodeColumnSize] = 
	{
		"	Node	",
		"	Geometry	",
		"	NodeType	",
		"	ControlType	",
		"	Zone	",
		"	Movement_USN1	",
		"	Movement_USN2	",
		"	Movement_USN3	",
		"	Movement_USN4	",
		"	Movement_USN5	",
		"	Movement_USN6	",
		"	TurnVol_USN1	",
		"	TurnVol_USN2	",
		"	TurnVol_USN3	",
		"	TurnVol_USN4	",
		"	TurnVol_USN5	",
		"	TurnVol_USN6	",
		"	RTOR_USN1	",
		"	RTOR_USN2	",
		"	RTOR_USN3	",
		"	RTOR_USN4	",
		"	RTOR_USN5	",
		"	RTOR_USN6	",
		"	Uturn_USN1	",
		"	Uturn_USN2	",
		"	Uturn_USN3	",
		"	Uturn_USN4	",
		"	Uturn_USN5	",
		"	Uturn_USN6	",
		"	Turn_multiplier_USN1	",
		"	Turn_multiplier_USN2	",
		"	Turn_multiplier_USN3	",
		"	Turn_multiplier_USN4	",
		"	Turn_multiplier_USN5	",
		"	Turn_multiplier_USN6	",
		"	Lane_Align_USN1	",
		"	Lane_Align_USN2	",
		"	Lane_Align_USN3	",
		"	Lane_Align_USN4	",
		"	Lane_Align_USN5	",
		"	Lane_Align_USN6	",
		"	Conditional_LT_USN1	",
		"	Conditional_TH_USN1	",
		"	Conditional_RT_USN1	",
		"	Conditional_Turn1_USN1	",
		"	Conditional_Turn2_USN1	",
		"	Conditional_LT_USN2	",
		"	Conditional_TH_USN2	",
		"	Conditional_RT_USN2	",
		"	Conditional_Turn1_USN2	",
		"	Conditional_Turn2_USN2	",
		"	Conditional_LT_USN3	",
		"	Conditional_TH_USN3	",
		"	Conditional_RT_USN3	",
		"	Conditional_Turn1_USN3	",
		"	Conditional_Turn2_USN3	",
		"	Conditional_LT_USN4	",
		"	Conditional_TH_USN4	",
		"	Conditional_RT_USN4	",
		"	Conditional_Turn1_USN4	",
		"	Conditional_Turn2_USN4	",
		"	Conditional_LT_USN5	",
		"	Conditional_TH_USN5	",
		"	Conditional_RT_USN5	",
		"	Conditional_Turn1_USN5	",
		"	Conditional_Turn2_USN5	",
		"	Conditional_LT_USN6	",
		"	Conditional_TH_USN6	",
		"	Conditional_RT_USN6	",
		"	Conditional_Turn1_USN6	",
		"	Conditional_Turn2_USN6	",
		"	Stopline_USN1	",
		"	Stopline_USN2	",
		"	Stopline_USN3	",
		"	Stopline_USN4	",
		"	Stopline_USN5	",
		"	Stopline_USN6	",
		"	Sight_Dist_USN1	",
		"	Sight_Dist_USN2	",
		"	Sight_Dist_USN3	",
		"	Sight_Dist_USN4	",
		"	Sight_Dist_USN5	",
		"	Sight_Dist_USN6	",
		"	Pedestrian_USN1	",
		"	Pedestrian_USN2	",
		"	Pedestrian_USN3	",
		"	Pedestrian_USN4	",
		"	Pedestrian_USN5	",
		"	Pedestrian_USN6	",
		"	Offramp_React	",
		"	Offramp_HOV_React	",
		"	Lane_Distribution	"};

		const int LinkColumnSize = 70;
		CString link_Column_name_str[LinkColumnSize] = 
		{

			"	Link	",
			"	Geometry	",
			"	USN	",
			"	DSN	",
			"	Name	",
			"	Type	",
			"	Length	",
			"	Lanes	",
			"	PostSpeed	",
			"	FreeFlowSpeed	",
			"	Grade	",
			"	Graphic	",
			"	Curvature	",
			"	Feature_points	",
			"	Underpass	",
			"	Startup_Delay	",
			"	Superelevation	",
			"	Pavement	",
			"	Radius	",
			"	Car_Follow_Sens_Multiplier	",
			"	Auxiliary	",
			"	Aux_Length	",
			"	LC_Speed_Threshold	",
			"	LC_Reaction	",
			"	Left_Barrier	",
			"	AddDrop	",
			"	AddDrop_Type	",
			"	AddDrop_Dist_USN	",
			"	AddDrop_React	",
			"	HV_Restrict	",
			"	HV_Restrict_Lane	",
			"	HV_Only	",
			"	HV_React	",
			"	HOV_#Lanes	",
			"	HOV_Location	",
			"	HOV_Type	",
			"	HOV_USN	",
			"	HOV_Length	",
			"	HOV_React	",
			"	HOV_User_TP	",
			"	HOV_Pct_TP	",
			"	Queue_Code_TP	",
			"	Discharge_Hwy	",
			"	LT_#Lane	",
			"	LT_Length	",
			"	RT_#Lane	",
			"	RT_Length	",
			"	Align_Lane	",
			"	Align_DSN	",
			"	Lane_Width	",
			"	Lane_Conf	",
			"	Bus_Station	",
			"	MidBlock_ID	",
			"	MidBlock_Flow	",
			"	Parking	",
			"	Park_Duration	",
			"	Park_Freq	",
			"	Park_Lt_DSN	",
			"	Park_LT_Length	",
			"	Park_RT_DSN	",
			"	Park_RT_Length	",
			"	Flow_Model	",
			"	Saturation	",
			"	Service_Flow	",
			"	Speed_Adj	",
			"	Link_Generation	",
			"	Direction	",
			"	Functional	",
			"	Area_Type	",
			"	Modes	"};


			CString AMS_Node_File, AMS_Link_File;
			CFileDialog node_dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
				"AMS Node Data File (*.csv)|*.csv||", NULL);
			if(node_dlg.DoModal() == IDOK)
			{
				AMS_Node_File = node_dlg.GetPathName();
			}else
			{
				return;
			}

			CFileDialog link_dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
				"AMS Link Data File (*.csv)|*.csv||", NULL);
			if(link_dlg.DoModal() == IDOK)
			{
				AMS_Link_File = link_dlg.GetPathName();

			}else
			{
				return;
			}



			CWaitCursor wait;
			FILE* st = NULL;
			fopen_s(&st,AMS_Node_File,"w");
			if(st!=NULL)
			{
				//CTestDataSource provide set of data for testing XY plot function
				int i;
				for(i = 0; i < NodeColumnSize; i++)
				{
					fprintf(st,"%s,",node_Column_name_str[i]);
				}
				fprintf(st,"\n");

				// Node sheet
				std::list<DTANode*>::iterator iNode;
				for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
				{
					fprintf(st, "%d,\"<Point><coordinates>%f,%f</coordinates></Point>\",,%d\n", (*iNode)->m_NodeNumber , (*iNode)->pt .x, (*iNode)->pt .y,(*iNode)->m_ControlType );
				}

				fclose(st);


			}




			fopen_s(&st,AMS_Link_File,"w");
			if(st!=NULL)
			{
				//CTestDataSource provide set of data for testing XY plot function
				int i;
				for(i = 0; i < LinkColumnSize; i++)
				{
					fprintf(st,"%s,",link_Column_name_str[i]);
				}
				fprintf(st,"\n");

				std::list<DTALink*>::iterator iLink;
				for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
				{
					if((*iLink)->m_AVISensorFlag == false)
					{
						//		fprintf(st,"name,link_id,from_node_id,to_node_id,direction,length_in_mile,number_of_lanes,speed_limit_in_mph,lane_capacity_in_vhc_per_hour,link_type,jam_density_in_vhc_pmpl,wave_speed_in_mph,mode_code,grade,geometry\n");
						fprintf(st,"%d,",(*iLink)->m_LinkID);

						fprintf(st,"\"<LineString><coordinates>");

						for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
						{
							fprintf(st,"%f,%f,0.0",(*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y);
							if(si!=(*iLink)->m_ShapePoints.size()-1)
								fprintf(st," ");
						}
						fprintf(st,"</coordinates></LineString>\",");

						fprintf(st,"%d,%d,%s,%d,%5.1f,%d,%3.1f,%3.1f,%3.1f",
							(*iLink)->m_FromNodeNumber, 
							(*iLink)->m_ToNodeNumber ,
							(*iLink)->m_Name.c_str (),
							(*iLink)->m_link_type ,
							(*iLink)->m_Length*5280 ,  // mile -> feet
							(*iLink)->m_NumberOfLanes ,
							(*iLink)->m_SpeedLimit,
							(*iLink)->m_SpeedLimit,
							(*iLink)->m_Grade);

						fprintf(st,"\n");
					}

				}
				fclose(st);

			}
			OpenCSVFileInExcel(AMS_Node_File);
			OpenCSVFileInExcel(AMS_Link_File);
}



void CTLiteDoc::RunExcelAutomation() 
{
	ExportQEMData(0);
}
#ifndef _WIN64
void AddGISField(std::vector<OGRFieldDefn> &OGRFieldVector, CString field_name,  OGRFieldType type)
{

	OGRFieldDefn oField(field_name, type );
	OGRFieldVector.push_back(oField);

}

bool CreateGISVector(std::vector<OGRFieldDefn> OGRFieldVector, OGRLayer *poLayer)
{
	for(int i = 0; i < OGRFieldVector.size(); i++)
	{
		OGRFieldDefn oField = OGRFieldVector[i];
		if( poLayer->CreateField( &oField ) != OGRERR_NONE )
		{
			CString str;
			str.Format("Creating field %s failed", oField.GetNameRef());
			AfxMessageBox(str);
			return false;		
		}
	}

	return true;
}
#endif 
void CTLiteDoc::ExportNodeLayerToGISFiles(CString file_name, CString GISTypeString)
{
#ifndef _WIN64

	CString message_str;

	OGRSFDriver *poDriver;

	OGRRegisterAll();

	poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(GISTypeString );
	if( poDriver == NULL )
	{
		m_GISMessage.Format ( "%s driver not available.", GISTypeString );
		AfxMessageBox(m_GISMessage);
		return;
	}

	OGRDataSource *poDS;

	poDS = poDriver->CreateDataSource(file_name, NULL );
	if( poDS == NULL )
	{
		m_GISMessage.Format ( "Creation of GIS output file %s failed.  Please make sure the file is not opened by another program.", file_name );
		AfxMessageBox(m_GISMessage);
		return;
	}

	///// export to node layer

	// node layer 
	{
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer( "node", NULL, wkbPoint, NULL );
		if( poLayer == NULL )
		{
			m_GISMessage = "Node layer creation failed";
			AfxMessageBox(m_GISMessage);
			return;
		}


		OGRFieldDefn oField1 ("Name", OFTString); 
		OGRFieldDefn oField2 ("NodeId", OFTInteger); 
		OGRFieldDefn oField3 ("NodeType", OFTInteger); 
		OGRFieldDefn oField4 ("x", OFTReal); 
		OGRFieldDefn oField5 ("y", OFTReal); 

		CString str;  
		if( poLayer->CreateField( &oField1 ) != OGRERR_NONE ) { str.Format("Creating field %s failed", oField1.GetNameRef()); AfxMessageBox(str); return; }
		if( poLayer->CreateField( &oField2 ) != OGRERR_NONE ) {	str.Format("Creating field %s failed", oField2.GetNameRef()); AfxMessageBox(str); return; }
		if( poLayer->CreateField( &oField3 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField3.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField4 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField4.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField5 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField5.GetNameRef()); AfxMessageBox(str); return ;	}

		std::list<DTANode*>::iterator iNode;

		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if((*iNode)->m_LayerNo ==0) 
			{
				OGRFeature *poFeature;

				poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
				poFeature->SetField("Name",(*iNode)->m_Name.c_str () );
				poFeature->SetField("NodeId", (*iNode)->m_NodeNumber );
				poFeature->SetField("NodeType", (*iNode)->m_ControlType );
				poFeature->SetField("x", (*iNode)->pt .x );
				poFeature->SetField("y", (*iNode)->pt .y );

				OGRPoint pt;
				pt.setX( (*iNode)->pt .x );
				pt.setY( (*iNode)->pt .y );
				poFeature->SetGeometry( &pt ); 

				if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
				{
					AfxMessageBox("Failed to create feature in shapefile.\n");
					return;

				}

				OGRFeature::DestroyFeature( poFeature );
		 }
		}

	} // end of node layer

	OGRDataSource::DestroyDataSource( poDS );
#endif
}

void CTLiteDoc::ExportLinkLayerToGISFiles(CString file_name, CString GISTypeString)
{
#ifndef _WIN64
	CString message_str;

	OGRSFDriver *poDriver;

	OGRRegisterAll();

	poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(GISTypeString );
	if( poDriver == NULL )
	{
		m_GISMessage.Format ( "%s driver not available.", GISTypeString );
		AfxMessageBox(m_GISMessage);
		return;
	}

	OGRDataSource *poDS;

	poDS = poDriver->CreateDataSource(file_name, NULL );
	if( poDS == NULL )
	{
		m_GISMessage.Format ( "Creation of GIS output file %s failed. Please make sure the file is not opened by another program.", file_name );
		AfxMessageBox(m_GISMessage);
		return;
	}

	///// export to node layer

	// link layer 
	{
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer( "link", NULL, wkbLineString, NULL );
		if( poLayer == NULL )
		{
			AfxMessageBox("link Layer creation failed");
			return;
		}

		OGRFieldDefn oField1 ("LinkID", OFTInteger); 
		OGRFieldDefn oField2 ("Name", OFTString); 
		OGRFieldDefn oField3 ("A_Node", OFTInteger); 
		OGRFieldDefn oField4 ("B_Node", OFTInteger); 
		OGRFieldDefn oField5 ("Length", OFTReal); 
		OGRFieldDefn oField6 ("nLanes", OFTInteger); 
		OGRFieldDefn oField7 ("SpeedLimit", OFTInteger); 
		OGRFieldDefn oField8 ("LaneCap", OFTInteger); 
		OGRFieldDefn oField9 ("FunctClass", OFTInteger); 
		OGRFieldDefn oField10 ("FFTT", OFTReal); 

		OGRFieldDefn oField_vol_h0 ("vol_0", OFTReal); 
		OGRFieldDefn oField_vol_h1 ("vol_1", OFTReal); 
		OGRFieldDefn oField_vol_h2 ("vol_2", OFTReal); 
		OGRFieldDefn oField_vol_h3 ("vol_3", OFTReal); 
		OGRFieldDefn oField_vol_h4 ("vol_4", OFTReal); 
		OGRFieldDefn oField_vol_h5 ("vol_5", OFTReal); 
		OGRFieldDefn oField_vol_h6 ("vol_6", OFTReal); 
		OGRFieldDefn oField_vol_h7 ("vol_7", OFTReal); 
		OGRFieldDefn oField_vol_h8 ("vol_8", OFTReal); 
		OGRFieldDefn oField_vol_h9 ("vol_9", OFTReal); 
		OGRFieldDefn oField_vol_h10 ("vol_10", OFTReal); 
		OGRFieldDefn oField_vol_h11 ("vol_11", OFTReal); 
		OGRFieldDefn oField_vol_h12 ("vol_12", OFTReal); 
		OGRFieldDefn oField_vol_h13 ("vol_13", OFTReal); 
		OGRFieldDefn oField_vol_h14 ("vol_14", OFTReal); 
		OGRFieldDefn oField_vol_h15 ("vol_15", OFTReal); 
		OGRFieldDefn oField_vol_h16 ("vol_16", OFTReal); 
		OGRFieldDefn oField_vol_h17 ("vol_17", OFTReal); 
		OGRFieldDefn oField_vol_h18 ("vol_18", OFTReal); 
		OGRFieldDefn oField_vol_h19 ("vol_19", OFTReal); 
		OGRFieldDefn oField_vol_h20 ("vol_20", OFTReal); 


		OGRFieldDefn oField_spd_h0 ("spd_0", OFTReal); 
		OGRFieldDefn oField_spd_h1 ("spd_1", OFTReal); 
		OGRFieldDefn oField_spd_h2 ("spd_2", OFTReal); 
		OGRFieldDefn oField_spd_h3 ("spd_3", OFTReal); 
		OGRFieldDefn oField_spd_h4 ("spd_4", OFTReal); 
		OGRFieldDefn oField_spd_h5 ("spd_5", OFTReal); 
		OGRFieldDefn oField_spd_h6 ("spd_6", OFTReal); 
		OGRFieldDefn oField_spd_h7 ("spd_7", OFTReal); 
		OGRFieldDefn oField_spd_h8 ("spd_8", OFTReal); 
		OGRFieldDefn oField_spd_h9 ("spd_9", OFTReal); 
		OGRFieldDefn oField_spd_h10 ("spd_10", OFTReal); 
		OGRFieldDefn oField_spd_h11 ("spd_11", OFTReal); 
		OGRFieldDefn oField_spd_h12 ("spd_12", OFTReal); 
		OGRFieldDefn oField_spd_h13 ("spd_13", OFTReal); 
		OGRFieldDefn oField_spd_h14 ("spd_14", OFTReal); 
		OGRFieldDefn oField_spd_h15 ("spd_15", OFTReal); 
		OGRFieldDefn oField_spd_h16 ("spd_16", OFTReal); 
		OGRFieldDefn oField_spd_h17 ("spd_17", OFTReal); 
		OGRFieldDefn oField_spd_h18 ("spd_18", OFTReal); 
		OGRFieldDefn oField_spd_h19 ("spd_19", OFTReal); 
		OGRFieldDefn oField_spd_h20 ("spd_20", OFTReal); 


		CString str;  
		if( poLayer->CreateField( &oField1 ) != OGRERR_NONE ) { str.Format("Creating field %s failed", oField1.GetNameRef()); AfxMessageBox(str); return; }
		if( poLayer->CreateField( &oField2 ) != OGRERR_NONE ) {	str.Format("Creating field %s failed", oField2.GetNameRef()); AfxMessageBox(str); return; }
		if( poLayer->CreateField( &oField3 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField3.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField4 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField4.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField5 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField5.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField6 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField6.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField7 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField7.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField8 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField8.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField9 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField9.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField10 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField10.GetNameRef()); AfxMessageBox(str); return ;	}

		if( poLayer->CreateField( &oField_vol_h0 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h0.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h1 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h1.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h2 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h2.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h3 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h3.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h4 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h4.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h5 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h5.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h6 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h6.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h7 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h7.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h8 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h8.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h9 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h9.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h10 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h10.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h11 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h11.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h12 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h12.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h13 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h13.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h14 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h14.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h15 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h15.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h16 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h16.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h17 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h17.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h18 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h18.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h19 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h19.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_vol_h10 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_vol_h20.GetNameRef()); AfxMessageBox(str); return ;	}

		if( poLayer->CreateField( &oField_spd_h0 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h0.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h1 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h1.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h2 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h2.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h3 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h3.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h4 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h4.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h5 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h5.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h6 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h6.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h7 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h7.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h8 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h8.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h9 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h9.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h10 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h10.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h11 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h11.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h12 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h12.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h13 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h13.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h14 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h14.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h15 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h15.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h16 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h16.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h17 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h17.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h18 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h18.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h19 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h19.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField_spd_h10 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField_spd_h20.GetNameRef()); AfxMessageBox(str); return ;	}

		std::list<DTALink*>::iterator iLink;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_AVISensorFlag == false && (*iLink)->m_LayerNo ==0)
			{

				OGRFeature *poFeature;

				poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
				poFeature->SetField("LinkID",(*iLink)->m_LinkID );
				poFeature->SetField("Name", (*iLink)->m_Name.c_str () );
				poFeature->SetField("A_Node", (*iLink)->m_FromNodeNumber );
				poFeature->SetField("B_Node", (*iLink)->m_ToNodeNumber );
				poFeature->SetField("Length", (*iLink)->m_Length  );
				poFeature->SetField("nLanes", (*iLink)->m_NumberOfLanes );
				poFeature->SetField("SpeedLimit", (*iLink)->m_SpeedLimit );
				poFeature->SetField("LaneCap", (*iLink)->m_LaneCapacity );
				poFeature->SetField("FunctClass", (*iLink)->m_link_type );
				poFeature->SetField("FFTT", (*iLink)->m_FreeFlowTravelTime );


				int hour = 0;

				
				poFeature->SetField("vol_0", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_1", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_2", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_3", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_4", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_5", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_6", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_7", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_8", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_9", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				
				poFeature->SetField("vol_10", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_11", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_12", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_13", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_14", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_15", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_16", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_17", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_18", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_19", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("vol_20", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60)); hour ++;

				hour= 0;
				poFeature->SetField("spd_0", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_1", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_2", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_3", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_4", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_5", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_6", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_7", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_8", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_9", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				
				poFeature->SetField("spd_10", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_11", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_12", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_13", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_14", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_15", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_16", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_17", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_18", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_19", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;
				poFeature->SetField("spd_20", (*iLink)->GetAvgLinkSpeed(hour*60,(hour+1)*60)); hour ++;

				//float value;
				//float mobility_index  = GetLinkMOE((*iLink), MOE_speed, 1, 360, value);
				//float reliability_index  = GetLinkMOE((*iLink), MOE_reliability, 1, 360, value);
				//poFeature->SetField("iSpeed", mobility_index);
				//poFeature->SetField("iReliable", reliability_index );

				OGRLineString line;
				for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
				{
					line.addPoint ((*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y,(*iLink)->m_NumberOfLanes);
				}

				poFeature->SetGeometry( &line ); 


				if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
				{
					AfxMessageBox("Failed to create line feature in shapefile.\n");
					return;

				}  

				OGRFeature::DestroyFeature( poFeature );
			}
		}

	} // end of link layer
	OGRDataSource::DestroyDataSource( poDS );
#endif
}


void CTLiteDoc::ExportZoneLayerToGISFiles(CString file_name, CString GISTypeString)
{
#ifndef _WIN64
	CString message_str;

	// 
	OGRSFDriver *poDriver;

	OGRRegisterAll();

	poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(GISTypeString );
	if( poDriver == NULL )
	{
		m_GISMessage.Format ( "%s driver not available.", GISTypeString );
		AfxMessageBox(m_GISMessage);	
		return;
	}

	OGRDataSource *poDS;

	poDS = poDriver->CreateDataSource(file_name, NULL );
	if( poDS == NULL )
	{
		m_GISMessage.Format ( "Creation of GIS output file %s failed. Please make sure the file is not opened by another program.", file_name );
		AfxMessageBox(m_GISMessage);
		return;
	}

	///// export to node layer

	// link layer 
	{
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer( "zone", NULL, wkbLineString, NULL );
		if( poLayer == NULL )
		{
			AfxMessageBox("link Layer creation failed");
			return;
		}

		OGRFieldDefn oField1 ("ZoneID", OFTInteger); 
		OGRFieldDefn oField2 ("Name", OFTString); 
		OGRFieldDefn oField3 ("extrude", OFTInteger); 
		OGRFieldDefn oField4 ("altitudeMode", OFTString); 

		CString str;  
		if( poLayer->CreateField( &oField1 ) != OGRERR_NONE ) { str.Format("Creating field %s failed", oField1.GetNameRef()); AfxMessageBox(str); return; }
		if( poLayer->CreateField( &oField2 ) != OGRERR_NONE ) {	str.Format("Creating field %s failed", oField2.GetNameRef()); AfxMessageBox(str); return; }
		if( poLayer->CreateField( &oField3 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField3.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField4 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField3.GetNameRef()); AfxMessageBox(str); return ;	}

		std::map<int, DTAZone>	:: const_iterator itr;
		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); ++itr)
		{

			OGRPolygon polygon;

			OGRFeature *poFeature;

			poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
			poFeature->SetField("ZoneID",itr->first );
			poFeature->SetField("extrude",1 );
			poFeature->SetField("altitudeMode","relativeToGround" );



			OGRLinearRing  ring;

			for(unsigned int si = 0; si< itr->second.m_ShapePoints.size(); si++)
			{
				ring.addPoint (itr->second.m_ShapePoints[si].x, itr->second.m_ShapePoints[si].y,700);
			}

			polygon.addRing(&ring);

			poFeature->SetGeometry( &polygon ); 


			if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
			{
				AfxMessageBox("Failed to create line feature in shapefile.\n");
				return;

			}  

			OGRFeature::DestroyFeature( poFeature );
		}
	} // end of zone layer
	OGRDataSource::DestroyDataSource( poDS );
#endif
}

void CTLiteDoc::ExportZoneLayerToKMLFiles(CString file_name, CString GISTypeString)
{
	FILE* st;
	fopen_s(&st,file_name,"w");
	if(st!=NULL)
	{
		fprintf(st,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(st,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		fprintf(st,"<Document>\n");
		fprintf(st,"<name>KmlFile</name>\n");


		// blue style
		fprintf(st,"<Style id=\"green\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ff00</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// red style

		fprintf(st,"<Style id=\"red\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d0000ff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// blue style

		fprintf(st,"<Style id=\"blue\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7dff0000</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// yellow style

		fprintf(st,"<Style id=\"yellow\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ffff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		fprintf(st,"<Folder>\n");
   		fprintf(st,"<name>Zone Layer</name>\n");
   		fprintf(st," <visibility>1</visibility>\n");
      	
		float max_zone_demand = 0;

		std::map<int, DTAZone>	:: iterator itr_o;
		for(itr_o = m_ZoneMap.begin(); itr_o != m_ZoneMap.end(); itr_o++)
		{
		
			for(std::map<int, DTAZone>	:: const_iterator itr_d = m_ZoneMap.begin(); itr_d != m_ZoneMap.end(); itr_d++)
			{
				float volume = m_ZoneMap[itr_o->first].GetTotalZonalDemand();
				if( volume > max_zone_demand)
					max_zone_demand = volume;

			}

		}

		int time_step = 4;
		int t = 0;
		double ratio = 1;
		
		int max_zone_height = g_GetPrivateProfileInt("KML_output","max_zone_height",500,m_ProjectFile);
		int min_zone_height = g_GetPrivateProfileInt("KML_output","min_zone_height",10,m_ProjectFile);

		if(m_DemandProfileVector.size() > 0)
			ratio = m_DemandProfileVector[0].time_dependent_ratio[t];
 
		for(itr_o = m_ZoneMap.begin(); itr_o != m_ZoneMap.end(); itr_o++)
		{
			float total_zone_demand = 0;
			
			for(std::map<int, DTAZone>	:: const_iterator itr_d = m_ZoneMap.begin(); itr_d != m_ZoneMap.end(); itr_d++)
			{
				float volume = m_ZoneMap[itr_o->first].m_ODDemandMatrix [itr_d->first].GetSubTotalValue ();
				total_zone_demand += volume;
			}

		fprintf(st,"\t<Placemark>\n");
			fprintf(st,"\t\t<name>%d</name>\n",itr_o->first );
/*			fprintf(st,"\t\t\t<TimeSpan>\n");

			CString time_stamp_str = GetTimeStampStrFromIntervalNo (t,false);
			CString time_stamp_str_end = GetTimeStampStrFromIntervalNo (t+time_step,false);

			fprintf(st,"\t\t\t<begin>2012-01-%0d</begin>\n",t);
			fprintf(st,"\t\t\t<end>2012-01-01-%0d</end>\n",t+time_step);

//			fprintf(st,"\t\t\t<begin>2012-01-01-T%s:00Z</begin>\n",time_stamp_str);
//			fprintf(st,"\t\t\t<end>2012-01-01-T%s:00Z</end>\n",time_stamp_str_end);

			fprintf(st,"\t\t\t </TimeSpan>\n");
*/	
        	fprintf(st,"\t\t<visibility>1</visibility>\n");

			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n",itr_o->second .color_code.c_str ());
			fprintf(st,"\t\t<Polygon>\n");
			fprintf(st,"\t\t<extrude>1</extrude>\n");
			fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			fprintf(st,"\t\t<outerBoundaryIs>\n");
			fprintf(st,"\t\t<LinearRing>\n");
			fprintf(st,"\t\t<coordinates>\n");

			float height = itr_o->second.GetTotalZonalDemand()* max_zone_height/max(1,max_zone_demand);
	
			if(height<=min_zone_height)
				height = min_zone_height;

				for(unsigned int si = 0; si< itr_o->second.m_ShapePoints.size(); si++)
			{


				fprintf(st,"\t\t\t%f,%f,%f\n", itr_o->second.m_ShapePoints[si].x, itr_o->second.m_ShapePoints[si].y,height);
			}

			fprintf(st,"\t\t</coordinates>\n");
			fprintf(st,"\t\t</LinearRing>\n");
			fprintf(st,"\t\t</outerBoundaryIs>\n");

			fprintf(st,"\t\t</Polygon>\n");
			fprintf(st,"\t</Placemark>\n");
		}  // for each zone

		   	fprintf(st,"</Folder>\n");
			fprintf(st,"</Document>\n");
			fprintf(st,"</kml>\n");
			fclose(st);
		} // end of file

}

void CTLiteDoc::ExportLink3DLayerToKMLFiles(CString file_name, CString GISTypeString)
{
	
//	m_LinkBandWidthMode =  LBW_number_of_lanes;
	m_LinkBandWidthMode  = LBW_link_volume;

	// other options:
//	m_LinkBandWidthMode  = LBW_number_of_lanes;

	std::list<DTALink*>::iterator iLink;
	float max_link_volume = 0;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_total_link_volume > max_link_volume)
			max_link_volume = (*iLink)->m_total_link_volume;
	}

	float min_link_volume = 99999;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_total_link_volume < min_link_volume)
			min_link_volume = (*iLink)->m_total_link_volume;
	}



	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		if(m_LinkBandWidthMode  == LBW_number_of_lanes)
		{
		(*iLink)->m_BandWidthValue = (*iLink)->m_NumberOfLanes *10;
		}else
		{
		(*iLink)->m_BandWidthValue = (*iLink)->m_total_link_volume*250/max_link_volume;
		
		}

	}
	GenerateOffsetLinkBand();


		// processing demand data 
	CString SettingsFile;
	SettingsFile.Format ("%sDTASettings.txt",m_ProjectDirectory);

	int DemandLoading_StartHour = (int) g_GetPrivateProfileDouble("demand", "loading_start_hour",6,SettingsFile);
	int DemandLoading_EndHour = (int) g_GetPrivateProfileDouble("demand", "loading_end_hour",12,SettingsFile);


	FILE* st;
	fopen_s(&st,file_name,"w");
	GDPoint BandPoint[2000];
	if(st!=NULL)
	{
		fprintf(st,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(st,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		fprintf(st,"<Document>\n");
		fprintf(st,"<name>KmlFile</name>\n");


		// blue style
		fprintf(st,"<Style id=\"green\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ff00</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// red style

		fprintf(st,"<Style id=\"red\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d0000ff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// blue style

		fprintf(st,"<Style id=\"blue\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7dff0000</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// yellow style

		fprintf(st,"<Style id=\"yellow\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ffff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		fprintf(st,"<Folder>\n");
   		fprintf(st,"<name>Zone Layer</name>\n");
   		fprintf(st," <visibility>1</visibility>\n");
      	

		int time_step = 4;

		int t = 0;
		double ratio = 1;
		
		int max_link_height = g_GetPrivateProfileInt("KML_output","max_link_height",5000,m_ProjectFile);
		int min_link_height = g_GetPrivateProfileInt("KML_output","min_link_height",10,m_ProjectFile);

 
		max_link_height = 500;

		float min_link_volume_threadshold= 50;

		

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if(min_link_volume < 0.1f || (*iLink)->m_total_link_volume >= min_link_volume_threadshold || m_LinkBandWidthMode  == LBW_number_of_lanes)
			{
			fprintf(st,"\t<Placemark>\n");
			fprintf(st,"\t\t<name>%d</name>\n",(*iLink)->m_LayerNo +1);
/*			fprintf(st,"\t\t\t<TimeSpan>\n");

			CString time_stamp_str = GetTimeStampStrFromIntervalNo (t,false);
			CString time_stamp_str_end = GetTimeStampStrFromIntervalNo (t+time_step,false);

			fprintf(st,"\t\t\t<begin>2012-01-%0d</begin>\n",t);
			fprintf(st,"\t\t\t<end>2012-01-01-%0d</end>\n",t+time_step);

//			fprintf(st,"\t\t\t<begin>2012-01-01-T%s:00Z</begin>\n",time_stamp_str);
//			fprintf(st,"\t\t\t<end>2012-01-01-T%s:00Z</end>\n",time_stamp_str_end);

			fprintf(st,"\t\t\t </TimeSpan>\n");
*/	
        	fprintf(st,"\t\t<visibility>1</visibility>\n");


			float total_speed = 0;
			int start_time_in_min = min((*iLink)->m_LinkMOEAry.size()-1,DemandLoading_StartHour*60+30);
			int end_time_in_min = min((*iLink)->m_LinkMOEAry.size(),DemandLoading_EndHour*60);

			if(start_time_in_min> end_time_in_min)
				end_time_in_min = start_time_in_min;


			for(int t = start_time_in_min; t<end_time_in_min ; t++)
			{
			total_speed += (*iLink)->m_LinkMOEAry[t].SimulationSpeed ;
			}
			float avg_speed = total_speed/max(1,end_time_in_min-start_time_in_min);
			float speed_limit_ratio =  avg_speed/max(1,(*iLink)->m_SpeedLimit);

			TRACE("\nspeed = %d, ratio = %f",avg_speed,speed_limit_ratio);

			string color_code = "green";

			if(speed_limit_ratio<0.9)
				color_code = "yellow";

			if(speed_limit_ratio<0.5)
				color_code = "red";


			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n",color_code.c_str ());
			fprintf(st,"\t\t<Polygon>\n");
			fprintf(st,"\t\t<extrude>1</extrude>\n");
			fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			fprintf(st,"\t\t<outerBoundaryIs>\n");
			fprintf(st,"\t\t<LinearRing>\n");
			fprintf(st,"\t\t<coordinates>\n");

			float height =(*iLink)->m_total_link_volume* max_link_height/max(1,max_link_volume);
	
			if(height<=min_link_height)
				height = min_link_height;

			int si;
			int band_point_index = 0;
		for(si = 0; si < (*iLink) ->m_ShapePoints .size(); si++)
		{
			BandPoint[band_point_index++] = (*iLink)->m_BandLeftShapePoints[si];
		}

		for(si = (*iLink) ->m_ShapePoints .size()-1; si >=0 ; si--)
		{
			BandPoint[band_point_index++] = (*iLink)->m_BandRightShapePoints[si];
		}

		BandPoint[band_point_index++]= (*iLink)->m_BandLeftShapePoints[0];



			for(unsigned int i = 0; i< band_point_index; i++)
			{
				fprintf(st,"\t\t\t%f,%f,%f\n", BandPoint[i].x,  BandPoint[i].y,height);
			}


			fprintf(st,"\t\t</coordinates>\n");
			fprintf(st,"\t\t</LinearRing>\n");
			fprintf(st,"\t\t</outerBoundaryIs>\n");

			fprintf(st,"\t\t</Polygon>\n");
			fprintf(st,"\t</Placemark>\n");
			}
		}  // for each link

		   	fprintf(st,"</Folder>\n");
			fprintf(st,"</Document>\n");
			fprintf(st,"</kml>\n");
			fclose(st);
		} // end of file

}

void CTLiteDoc::ExportLink3DLayerToKMLFiles_ColorCode(CString file_name, CString GISTypeString, int ColorCode, bool no_curve_flag, int default_height =-1)
{
	
	m_LinkBandWidthMode =  LBW_number_of_lanes;
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		if(ColorCode==0)
			(*iLink)->m_BandWidthValue = (*iLink)->m_NumberOfLanes *10;

		if(ColorCode==1)
			(*iLink)->m_BandWidthValue = (*iLink)->m_NumberOfLanes *30;
	}
	GenerateOffsetLinkBand();

	FILE* st;
	fopen_s(&st,file_name,"w");
	GDPoint BandPoint[2000];
	if(st!=NULL)
	{
		fprintf(st,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(st,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		fprintf(st,"<Document>\n");
		fprintf(st,"<name>KmlFile</name>\n");

		// blue style
		fprintf(st,"<Style id=\"green\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ff00</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// red style

		fprintf(st,"<Style id=\"red\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d0000ff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// blue style

		fprintf(st,"<Style id=\"blue\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7dff0000</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// yellow style

		fprintf(st,"<Style id=\"yellow\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ffff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		fprintf(st,"<Folder>\n");
   		fprintf(st,"<name>Zone Layer</name>\n");
   		fprintf(st," <visibility>1</visibility>\n");
      	

		int time_step = 4;

		int t = 0;
		double ratio = 1;
		
	 		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->green_height >=1|| (*iLink)->red_height >=1 || (*iLink)->blue_height >=1|| (*iLink)->yellow_height >=1)
			{
			fprintf(st,"\t<Placemark>\n");
			fprintf(st,"\t\t<name>%d</name>\n",(*iLink)->m_LayerNo +1);
/*			fprintf(st,"\t\t\t<TimeSpan>\n");

			CString time_stamp_str = GetTimeStampStrFromIntervalNo (t,false);
			CString time_stamp_str_end = GetTimeStampStrFromIntervalNo (t+time_step,false);

			fprintf(st,"\t\t\t<begin>2012-01-%0d</begin>\n",t);
			fprintf(st,"\t\t\t<end>2012-01-01-%0d</end>\n",t+time_step);

//			fprintf(st,"\t\t\t<begin>2012-01-01-T%s:00Z</begin>\n",time_stamp_str);
//			fprintf(st,"\t\t\t<end>2012-01-01-T%s:00Z</end>\n",time_stamp_str_end);

			fprintf(st,"\t\t\t </TimeSpan>\n");
*/	
        	fprintf(st,"\t\t<visibility>1</visibility>\n");


			string color_code = "green";

			if(ColorCode==1)
				color_code = "red";

			if(ColorCode==2)
				color_code = "blue";

			if(ColorCode==3)
				color_code = "yellow";

			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n",color_code.c_str ());
			fprintf(st,"\t\t<Polygon>\n");
			fprintf(st,"\t\t<extrude>1</extrude>\n");
			fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			fprintf(st,"\t\t<outerBoundaryIs>\n");
			fprintf(st,"\t\t<LinearRing>\n");
			fprintf(st,"\t\t<coordinates>\n");


			float height =(*iLink)->green_height;
	
				if(ColorCode==1)
					height =(*iLink)->red_height;

				if(ColorCode==2)
					height =(*iLink)->blue_height;

				if(ColorCode==3)
					height =(*iLink)->yellow_height;

				if(default_height>=0)
					height= default_height;

				int si;
			int band_point_index = 0;
			int size = (*iLink) ->m_ShapePoints .size();
		for(si = 0; si < (*iLink) ->m_ShapePoints .size(); si++)
		{
			if(no_curve_flag && size >=3 && si >=1 && si< size -1) // skip intermedate points
				continue;

			BandPoint[band_point_index++] = (*iLink)->m_BandLeftShapePoints[si];
		}

		for(si = (*iLink) ->m_ShapePoints .size()-1; si >=0 ; si--)
		{
			if(no_curve_flag && size >=3 && si >=1 && si< size -1) // skip intermedate points
				continue;

			BandPoint[band_point_index++] = (*iLink)->m_BandRightShapePoints[si];
		}

		BandPoint[band_point_index++]= (*iLink)->m_BandLeftShapePoints[0];



			for(unsigned int i = 0; i< band_point_index; i++)
			{
				fprintf(st,"\t\t\t%f,%f,%f\n", BandPoint[i].x,  BandPoint[i].y,height);
			}


			fprintf(st,"\t\t</coordinates>\n");
			fprintf(st,"\t\t</LinearRing>\n");
			fprintf(st,"\t\t</outerBoundaryIs>\n");

			fprintf(st,"\t\t</Polygon>\n");
			fprintf(st,"\t</Placemark>\n");
			}
		}  // for each link
	
		   	fprintf(st,"</Folder>\n");
			fprintf(st,"</Document>\n");
			fprintf(st,"</kml>\n");
			fclose(st);
		} // end of file

}

void CTLiteDoc::ExportLinkDiffLayerToKMLFiles(CString file_name, CString GISTypeString)
{
	
//	m_LinkBandWidthMode = LBW_link_volume;

	// other options:
	m_LinkBandWidthMode  = LBW_link_volume;

	std::list<DTALink*>::iterator iLink;
	float max_link_volume = 0;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_total_link_volume > max_link_volume)
			max_link_volume = (*iLink)->m_total_link_volume;
	}


	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		(*iLink)->m_BandWidthValue = (*iLink)->m_total_link_volume*250/max_link_volume;

	}


	GenerateOffsetLinkBand();


	FILE* st;
	fopen_s(&st,file_name,"w");
	GDPoint BandPoint[2000];
	if(st!=NULL)
	{
		fprintf(st,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(st,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		fprintf(st,"<Document>\n");
		fprintf(st,"<name>KmlFile</name>\n");


		// blue style
		fprintf(st,"<Style id=\"green\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ff00</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// red style

		fprintf(st,"<Style id=\"red\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d0000ff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// blue style

		fprintf(st,"<Style id=\"blue\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7dff0000</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// yellow style

		fprintf(st,"<Style id=\"yellow\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ffff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		fprintf(st,"<Folder>\n");
   		fprintf(st,"<name>Zone Layer</name>\n");
   		fprintf(st," <visibility>1</visibility>\n");
      	

		int time_step = 4;

		int t = 0;
		double ratio = 1;
		
		int max_link_height = g_GetPrivateProfileInt("KML_output","max_link_height",5000,m_ProjectFile);
		int min_link_height = g_GetPrivateProfileInt("KML_output","min_link_height",100,m_ProjectFile);

 
		max_link_height = 60;

		float min_link_volume_threadshold= 3000;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_total_link_volume >= min_link_volume_threadshold)
			{
			fprintf(st,"\t<Placemark>\n");
			fprintf(st,"\t\t<name>%d</name>\n",(*iLink)->m_LayerNo +1);
/*			fprintf(st,"\t\t\t<TimeSpan>\n");

			CString time_stamp_str = GetTimeStampStrFromIntervalNo (t,false);
			CString time_stamp_str_end = GetTimeStampStrFromIntervalNo (t+time_step,false);

			fprintf(st,"\t\t\t<begin>2012-01-%0d</begin>\n",t);
			fprintf(st,"\t\t\t<end>2012-01-01-%0d</end>\n",t+time_step);

//			fprintf(st,"\t\t\t<begin>2012-01-01-T%s:00Z</begin>\n",time_stamp_str);
//			fprintf(st,"\t\t\t<end>2012-01-01-T%s:00Z</end>\n",time_stamp_str_end);

			fprintf(st,"\t\t\t </TimeSpan>\n");
*/	
        	fprintf(st,"\t\t<visibility>1</visibility>\n");

			string color_code = "red";

			if((*iLink)->m_NumberOfLanes == 2)
				color_code = "green";

			if((*iLink)->m_NumberOfLanes == 1)
				color_code = "yellow";


			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n",color_code.c_str ());
			fprintf(st,"\t\t<Polygon>\n");
			fprintf(st,"\t\t<extrude>1</extrude>\n");
			fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			fprintf(st,"\t\t<outerBoundaryIs>\n");
			fprintf(st,"\t\t<LinearRing>\n");
			fprintf(st,"\t\t<coordinates>\n");

			float height =(*iLink)->m_total_link_volume* max_link_height/max(1,max_link_volume);
	
			if(height<=min_link_height)
				height = min_link_height;

			int si;
			int band_point_index = 0;
		for(si = 0; si < (*iLink) ->m_ShapePoints .size(); si++)
		{
			BandPoint[band_point_index++] = (*iLink)->m_BandLeftShapePoints[si];
		}

		for(si = (*iLink) ->m_ShapePoints .size()-1; si >=0 ; si--)
		{
			BandPoint[band_point_index++] = (*iLink)->m_BandRightShapePoints[si];
		}

		BandPoint[band_point_index++]= (*iLink)->m_BandLeftShapePoints[0];



			for(unsigned int i = 0; i< band_point_index; i++)
			{
				fprintf(st,"\t\t\t%f,%f,%f\n", BandPoint[i].x,  BandPoint[i].y,height);
			}


			fprintf(st,"\t\t</coordinates>\n");
			fprintf(st,"\t\t</LinearRing>\n");
			fprintf(st,"\t\t</outerBoundaryIs>\n");

			fprintf(st,"\t\t</Polygon>\n");
			fprintf(st,"\t</Placemark>\n");
			}
		}  // for each link

		   	fprintf(st,"</Folder>\n");
			fprintf(st,"</Document>\n");
			fprintf(st,"</kml>\n");
			fclose(st);
		} // end of file

}
void CTLiteDoc::GeneratePathFromVehicleData()
{
	m_PathMap.clear();

	std::list<DTAVehicle*>::iterator iVehicle;

	for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++)
	{
		DTAVehicle* pVehicle = (*iVehicle);

		if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
		{

			CString label;
			label.Format("%d,%d,%d,%d", pVehicle->m_OriginZoneID  , pVehicle->m_DestinationZoneID , pVehicle->m_NodeNumberSum , pVehicle->m_NodeSize );

		//existing path
						m_PathMap[label].Origin = pVehicle->m_OriginZoneID;
						m_PathMap[label].Destination  = pVehicle->m_DestinationZoneID;
						m_PathMap[label].TotalVehicleSize+=1;
						
						if(m_PathMap[label].TotalVehicleSize == 1)  // new path
						{

							for(int link= 1; link<pVehicle->m_NodeSize; link++)
							{
								m_PathMap[label].m_LinkVector.push_back(pVehicle->m_NodeAry[link].LinkNo);

								DTALink* pLink = m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo];

								if(link==1) // first link
								{
									m_PathMap[label].m_NodeVector.push_back(pLink->m_FromNodeNumber );
								}

								m_PathMap[label].m_NodeVector.push_back(pLink->m_ToNodeNumber);

								for(unsigned int si = 0; si < pLink ->m_ShapePoints .size(); si++)
								{
								m_PathMap[label].m_ShapePoints.push_back (pLink->m_ShapePoints[si]);
								}

							}


						}

						m_PathMap[label].TotalTravelTime  += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
						m_PathMap[label].TotalDistance   += pVehicle->m_Distance;
						m_PathMap[label].TotalCost   += pVehicle->m_TollDollarCost;
						m_PathMap[label].TotalEmissions   += pVehicle->m_Emissions;
					
						m_PathMap[label].emissiondata.Energy += pVehicle->m_EmissionData .Energy;
						m_PathMap[label].emissiondata.CO2 += pVehicle->m_EmissionData .CO2;
						m_PathMap[label].emissiondata.NOX += pVehicle->m_EmissionData .NOX;
						m_PathMap[label].emissiondata.CO += pVehicle->m_EmissionData .CO;
						m_PathMap[label].emissiondata.HC += pVehicle->m_EmissionData .HC;


						// reuse label as OD label
						label.Format("%d,%d", pVehicle->m_OriginZoneID  , pVehicle->m_DestinationZoneID);
						m_ODMatrixMap[label].Origin = pVehicle->m_OriginZoneID;
						m_ODMatrixMap[label].Destination  = pVehicle->m_DestinationZoneID;
						m_ODMatrixMap[label].TotalVehicleSize+=1;
						m_ODMatrixMap[label].TotalTravelTime  += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
						m_ODMatrixMap[label].TotalDistance   += pVehicle->m_Distance;
						m_ODMatrixMap[label].TotalCost   += pVehicle->m_TollDollarCost;
						m_ODMatrixMap[label].TotalEmissions   += pVehicle->m_Emissions;
					
						m_ODMatrixMap[label].emissiondata.Energy += pVehicle->m_EmissionData .Energy;
						m_ODMatrixMap[label].emissiondata.CO2 += pVehicle->m_EmissionData .CO2;
						m_ODMatrixMap[label].emissiondata.NOX += pVehicle->m_EmissionData .NOX;
						m_ODMatrixMap[label].emissiondata.CO += pVehicle->m_EmissionData .CO;
						m_ODMatrixMap[label].emissiondata.HC += pVehicle->m_EmissionData .HC;

		}
	}
}
void CTLiteDoc::ExportAgentLayerToKMLFiles(CString file_name, CString GISTypeString)
{
#ifndef _WIN64
	GeneratePathFromVehicleData();

	CString message_str;

	OGRSFDriver *poDriver;

	OGRRegisterAll();

	poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(GISTypeString );
	if( poDriver == NULL )
	{
		m_GISMessage.Format ( "%s driver not available.", GISTypeString );
		AfxMessageBox(m_GISMessage);
		return;
	}

	OGRDataSource *poDS;

	poDS = poDriver->CreateDataSource(file_name, NULL );
	if( poDS == NULL )
	{
		m_GISMessage.Format ( "Creation of GIS output file %s failed. Please make sure the file is not opened by another program.", file_name );
		AfxMessageBox(m_GISMessage);
		return;
	}

	///// export to node layer
		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer( "agent", NULL, wkbLineString, NULL );
		if( poLayer == NULL )
		{
			AfxMessageBox("link Layer creation failed");
			return;
		}

		OGRFieldDefn oField1 ("AgentID", OFTInteger); 
		OGRFieldDefn oField2 ("From_zone", OFTInteger); 
		OGRFieldDefn oField3 ("To_zone", OFTInteger); 
		OGRFieldDefn oField4 ("From_Link", OFTInteger); 
		OGRFieldDefn oField5 ("To_Link", OFTInteger); 
		OGRFieldDefn oField6 ("Volume", OFTReal); 

		CString str;  
		if( poLayer->CreateField( &oField1 ) != OGRERR_NONE ) { str.Format("Creating field %s failed", oField1.GetNameRef()); AfxMessageBox(str); return; }
		if( poLayer->CreateField( &oField2 ) != OGRERR_NONE ) {	str.Format("Creating field %s failed", oField2.GetNameRef()); AfxMessageBox(str); return; }
		if( poLayer->CreateField( &oField3 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField3.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField4 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField4.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField5 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField5.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField6 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField5.GetNameRef()); AfxMessageBox(str); return ;	}

		int count = 0;
		std::map<CString, PathStatistics> ::const_iterator itr;
		for (itr = m_PathMap.begin(); itr != m_PathMap.end(); itr++)
		{
			if((*itr).second .TotalVehicleSize >=10)
			{
				OGRFeature *poFeature;

				poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
				poFeature->SetField("AgentID", count++);
				poFeature->SetField("From_Zone", (*itr).second.Origin );
				poFeature->SetField("To_Zone", (*itr).second .Destination  );
				poFeature->SetField("From_Link", (*itr).second .m_LinkVector[0] );
				poFeature->SetField("To_Link", (*itr).second .m_LinkVector[(*itr).second .m_LinkVector.size()-1] );
				poFeature->SetField("Volume", (*itr).second .TotalVehicleSize );

				OGRLineString line;
				for(unsigned int si = 0; si< (*itr).second .m_ShapePoints.size(); si++)
				{
					line.addPoint ((*itr).second .m_ShapePoints[si].x, (*itr).second .m_ShapePoints[si].y,0);
				}

				poFeature->SetGeometry( &line ); 


				if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
				{
					AfxMessageBox("Failed to create line feature in shapefile.\n");
					return;

				}  

				OGRFeature::DestroyFeature( poFeature );
			}
			}
	
	OGRDataSource::DestroyDataSource( poDS );
#endif
}

void CTLiteDoc::ExportPathflowToCSVFiles()
{

	CString directory = m_ProjectDirectory;
	FILE* st = NULL;
	fopen_s(&st,directory+"output_path_flow.csv","w");
	if(st!=NULL)
	{
	fprintf(st,"route_index,vehicle_type,from_zone_id,from_node_id,to_zone_id,to_node_id,time_span_volume,day_volume,node_chain_number_of_nodes,node_chain_node_sequence\n");

		int route_index = 1;
		std::map<CString, PathStatistics> ::const_iterator itr;
		for (itr = m_PathMap.begin(); itr != m_PathMap.end(); itr++)
		{

			if((*itr).second .m_NodeVector  .size() >=2 && (*itr).second .TotalVehicleSize >=1)
			{
				int number_of_nodes = (*itr).second.m_NodeVector.size();
						// one vehicle type for now
				fprintf(st, "%d,%d,%d,%d,%d,%d,%d,%d,%d,", 
					route_index++,
					1, 
					(*itr).second.Origin, 
					(*itr).second.m_NodeVector[0],
					(*itr).second .Destination, 
					(*itr).second.m_NodeVector[number_of_nodes-1],
					(*itr).second .TotalVehicleSize,
					(*itr).second .TotalVehicleSize*10,  // 10 for 1/peak hour factor
					number_of_nodes);

				fprintf(st,"["); 
				for(unsigned int n = 0; n < number_of_nodes; n++)
				{
				fprintf(st,"%d;",(*itr).second.m_NodeVector[n]); 
				}
				fprintf(st,"]"); 

				fprintf(st,"\n"); 
			}
		}
	fclose(st);
	}
// write to a common path file
	fopen_s(&st,directory+"AMS_path_flow.csv","w");
	if(st!=NULL)
	{
	fprintf(st,"route_index,volume,node_chain_number_of_nodes,node_chain_node_sequence\n");

		int route_index = 1;
		std::map<CString, PathStatistics> ::const_iterator itr;
		for (itr = m_PathMap.begin(); itr != m_PathMap.end(); itr++)
		{

			if((*itr).second .TotalVehicleSize >=1)
			{
				int number_of_nodes = (*itr).second.m_NodeVector.size();
						// one vehicle type for now
				fprintf(st, "%d,%d,%d", 
					route_index++,
					(*itr).second .TotalVehicleSize,
					number_of_nodes);

				fprintf(st,"["); 
				for(unsigned int n = 0; n < number_of_nodes; n++)
				{
				fprintf(st,"%d;",(*itr).second.m_NodeVector[n]); 
				}
				fprintf(st,"]"); 

				fprintf(st,"\n"); 
			}
		}
	fclose(st);
	}

	
	// OD statistics
		fopen_s(&st,directory+"output_od_flow.csv","w");
	if(st!=NULL)
	{
	fprintf(st,"od_index,vehicle_type,from_zone_id,to_zone_id,time_span_volume,day_volume\n");

		int od_index = 1;
		std::map<CString, PathStatistics> ::const_iterator itr;
		for (itr = m_ODMatrixMap.begin(); itr != m_ODMatrixMap.end(); itr++)
		{

			if((*itr).second .TotalVehicleSize >=1)
			{
				int number_of_nodes = (*itr).second.m_NodeVector.size();
						// one vehicle type for now
				fprintf(st, "%d,%d,%d,%d,%d,%d\n", 
					od_index++,
					1, 
					(*itr).second.Origin, 
					(*itr).second .Destination, 
					(*itr).second .TotalVehicleSize,
					(*itr).second .TotalVehicleSize*10);  // 10 for 1/peak hour factor

			}	
		}
	fclose(st);
	}
	
	SaveAMS_ODTable();


}

void CTLiteDoc::SaveAMS_ODTable()
{
		CString directory = m_ProjectDirectory;
	FILE* st = NULL;

	//AMS OD statistics

		fopen_s(&st,directory+"AMS_OD_table.csv","w");
	if(st!=NULL)
	{
	fprintf(st,"from_zone_id,to_zone_id,time_span_volume\n");

		int od_index = 1;
		std::map<CString, PathStatistics> ::const_iterator itr;
		for (itr = m_ODMatrixMap.begin(); itr != m_ODMatrixMap.end(); itr++)
		{

			if((*itr).second .TotalVehicleSize >=1)
			{
				int number_of_nodes = (*itr).second.m_NodeVector.size();
						// one vehicle type for now
				fprintf(st, "%d,%d,%d\n", 
					(*itr).second.Origin, 
					(*itr).second .Destination, 
					(*itr).second .TotalVehicleSize);

			}	
		}
	fclose(st);
	}else
	{
	AfxMessageBox("File AMS_OD_table.csv cannot be opened.");
	}
}
void CTLiteDoc::ExportLinkMOEToKMLFiles(CString file_name)
{
/*
	FILE* st;
	fopen_s(&st,file_name,"w");
	if(st!=NULL)
	{
		fprintf(st,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(st,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		fprintf(st,"<Document>\n");
		fprintf(st,"<name>KmlFile</name>\n");

		std::vector<std::string> m_KML_style_name;
		// blue style
		m_KML_style_name.push_back("transGreenPoly");
		fprintf(st,"<Style id=\"transGreenPoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ff00</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// red style
		m_KML_style_name.push_back("transRedPoly");
		fprintf(st,"<Style id=\"transRedPoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d0000ff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// blue style
		m_KML_style_name.push_back("transBluePoly");
		fprintf(st,"<Style id=\"transBluePoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7dff0000</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// yellow style
		m_KML_style_name.push_back("transYellowPoly");
		fprintf(st,"<Style id=\"transYellowPoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ffff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		fprintf(st,"<Folder>\n");
   		fprintf(st,"<name>Zone Layer</name>\n");
   		fprintf(st," <visibility>1</visibility>\n");
      	
		std::list<DTALink*>::iterator iLink;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_AVISensorFlag == false && (*iLink)->m_LayerNo ==0)
			{

		fprintf(st,"\t<Placemark>\n");
			fprintf(st,"\t\t<name>%d</name>\n",itr->first );
			fprintf(st,"\t\t<visibility>1</visibility>\n");

			int style_no = (itr->first % m_KML_style_name.size());

			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n",m_KML_style_name[style_no].c_str ());

				poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
				poFeature->SetField("LinkID",(*iLink)->m_LinkID );
				poFeature->SetField("Name", (*iLink)->m_Name.c_str () );
				poFeature->SetField("A_Node", (*iLink)->m_FromNodeNumber );
				poFeature->SetField("B_Node", (*iLink)->m_ToNodeNumber );
				poFeature->SetField("IsOneWay", (*iLink)->m_Direction );
				poFeature->SetField("NumberOfLanes", (*iLink)->m_NumberOfLanes );
				poFeature->SetField("SpeedLimit", (*iLink)->m_SpeedLimit );
				poFeature->SetField("LaneCapacity", (*iLink)->m_LaneCapacity );
				poFeature->SetField("FunctionalClass", (*iLink)->m_link_type );
				poFeature->SetField("Grade", (*iLink)->m_Grade );

				float value;
				float mobility_index  = GetLinkMOE((*iLink), MOE_speed, 1, 360, value);
				float reliability_index  = GetLinkMOE((*iLink), MOE_reliability, 1, 360, value);
				poFeature->SetField("iSpeed", mobility_index);
				poFeature->SetField("iReliable", reliability_index );

				OGRLineString line;
				for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
				{
					line.addPoint ((*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y,(*iLink)->m_NumberOfLanes);
				}
			fprintf(st,"\t\t<Polygon>\n");
			fprintf(st,"\t\t<extrude>1</extrude>\n");
			fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			fprintf(st,"\t\t<outerBoundaryIs>\n");
			fprintf(st,"\t\t<LinearRing>\n");
			fprintf(st,"\t\t<coordinates>\n");

			for(unsigned int si = 0; si< itr->second.m_ShapePoints.size(); si++)
			{
				fprintf(st,"\t\t\t%f,%f,%f\n", itr->second.m_ShapePoints[si].x, itr->second.m_ShapePoints[si].y,700.0f);
			}

			fprintf(st,"\t\t</coordinates>\n");
			fprintf(st,"\t\t</LinearRing>\n");
			fprintf(st,"\t\t</outerBoundaryIs>\n");

			fprintf(st,"\t\t</Polygon>\n");
			fprintf(st,"\t</Placemark>\n");
		}
		   	fprintf(st,"</Folder>\n");
			fprintf(st,"</Document>\n");
			fprintf(st,"</kml>\n");
			fclose(st);
		}
		*/
}

void CTLiteDoc::ConvertLinkCSV2ShapeFiles(LPCTSTR lpszCSVFileName,LPCTSTR lpszShapeFileName, CString GISTypeString, _GIS_DATA_TYPE GIS_data_type)
{
#ifndef _WIN64

	CWaitCursor wait;
	CCSVParser parser;
	int i= 0;

	// open csv file
	if (parser.OpenCSVFile(lpszCSVFileName))
	{

		CString message_str;

		OGRSFDriver *poDriver;

		OGRRegisterAll();

		poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(GISTypeString );
		if( poDriver == NULL )
		{
			message_str.Format ( "%s driver not available.", GISTypeString );
			return;
		}

		OGRDataSource *poDS;

		poDS = poDriver->CreateDataSource(lpszShapeFileName, NULL );
		if( poDS == NULL )
		{
			message_str.Format ( "Creation of GIS output file %s failed.\nPlease do not overwrite the exiting file and please select a new file name.", 
				lpszShapeFileName );
			return;
		}

		///// export to link layer

		// link layer 

			OGRLayer *poLayer;
			poLayer = poDS->CreateLayer( "link", NULL, wkbLineString, NULL );
			if( poLayer == NULL )
			{
				message_str.Format("link Layer creation failed");
				return;
			}



			vector<string> HeaderVector = parser.GetHeaderVector();

			std::vector <CString> LongFieldVector;
			for(unsigned int i = 0; i < HeaderVector.size(); i++)
			{
					if(HeaderVector[i].find ("geometry") !=  string::npos||  HeaderVector[i].find ("name") !=  string::npos || HeaderVector[i].find ("code") !=  string::npos)
					{
						OGRFieldDefn oField (HeaderVector[i].c_str (), OFTString);

						CString str;  
						if( poLayer->CreateField( &oField ) != OGRERR_NONE ) 
						{ 
							str.Format("Creating field %s failed", oField.GetNameRef()); 

							return; 

						}
					}else
					{
						CString field_string  = HeaderVector[i].c_str ();

						OGRFieldDefn oField (field_string, OFTReal);

						CString str;  
						if( poLayer->CreateField( &oField ) != OGRERR_NONE ) 
						{ 
							str.Format("Creating field %s failed", oField.GetNameRef()); 

								return; 
						}

				}

					if(HeaderVector[i].size()>=11)
					{
						LongFieldVector.push_back (HeaderVector[i].c_str ());
					}

			}

			message_str.Format ("%d fields have been created.",HeaderVector.size());

			if(LongFieldVector.size() >=1)
			{
				message_str.Format("Warning: Arc GIS file only supports field names with not more than 10 characters.\nThe following fields have long field names. "); 
				for(unsigned l = 0; l< LongFieldVector.size(); l++)
				{
						message_str.Format ("%s",LongFieldVector[l]);

				
				}
			}

			int count = 0 ;
			while(parser.ReadRecord())
			{
				//create feature
				OGRFeature *poFeature;
				poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );

				//step 1: write all fields except geometry
				for(unsigned int i = 0; i < HeaderVector.size(); i++)
				{
					if(HeaderVector[i]!="geometry")
					{
						if(HeaderVector[i].find ("name") !=  string::npos || HeaderVector[i].find ("code") !=  string::npos)
						{

							std::string str_value;

							parser.GetValueByFieldName(HeaderVector[i],str_value);

//							TRACE("field: %s, value = %s\n",HeaderVector[i].c_str (),str_value.c_str ());
							poFeature->SetField(i,str_value.c_str ());
						}else
						{
							double value = 0;

							parser.GetValueByFieldName(HeaderVector[i],value);

//							TRACE("field: %s, value = %f\n",HeaderVector[i].c_str (),value);

							CString field_name = HeaderVector[i].c_str ();
							poFeature->SetField(i,value);



						}

					}
				}

					string geo_string;
					std::vector<CCoordinate> CoordinateVector;
					if(parser.GetValueByFieldName("geometry",geo_string))
					{
						// overwrite when the field "geometry" exists
						CGeometry geometry(geo_string);
						CoordinateVector = geometry.GetCoordinateList();

						if( GIS_data_type == GIS_Point_Type && CoordinateVector.size ()==1)
						{
								OGRPoint pt;
								pt.setX( CoordinateVector[0].X );
								pt.setY( CoordinateVector[0].Y);
								poFeature->SetGeometry( &pt ); 

						}



						if( GIS_data_type == GIS_Line_Type)
						{

						OGRLineString line;
						for(unsigned int si = 0; si< CoordinateVector.size(); si++)
						{
							line.addPoint (CoordinateVector[si].X , CoordinateVector[si].Y);
						}

						poFeature->SetGeometry( &line ); 
						}


						if( GIS_data_type == GIS_Polygon_Type)
						{

							OGRPolygon polygon;
							OGRLinearRing  ring;

							for(unsigned int si = 0; si<  CoordinateVector.size(); si++)
							{
								ring.addPoint (CoordinateVector[si].X , CoordinateVector[si].Y,1);
							}

							polygon.addRing(&ring);

							poFeature->SetGeometry( &polygon ); 

						}


					}




					if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
					{
						AfxMessageBox("Failed to create line feature in shapefile.\n");
						return;

					}  

					OGRFeature::DestroyFeature( poFeature );

					count++;
				}

				message_str.Format ("%d records have been created.",count);



			OGRDataSource::DestroyDataSource( poDS );

			CString ShapeFile = lpszShapeFileName;
			CString ShapeFileFolder = ShapeFile.Left(ShapeFile.ReverseFind('\\') + 1);

			ShellExecute( NULL,  "explore", ShapeFileFolder, NULL,  NULL, SW_SHOWNORMAL );

		}
#else
	AfxMessageBox("Please use NEXTA 32 bit version to generate GIS shape files.");
#endif

}
void CTLiteDoc::OnGenerategisshapefilesLoadlinkcsvfile()
{
#ifdef _WIN64

		AfxMessageBox("Please use NEXTA 32 bit version for this function.");
		return;

#endif

}