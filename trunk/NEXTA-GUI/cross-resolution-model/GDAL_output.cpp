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
#include "..//Data-Interface//XLEzAutomation.h"
#include "..//Data-Interface//XLTestDataSource.h"
#include "..//Data-Interface//include//ogrsf_frmts.h"
#include "MainFrm.h"

#include "SignalNode.h"
#include "..//Dlg_SignalDataExchange.h"


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
							(*iLink)->m_NumLanes ,
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
	//CXLEzAutomation class constructor starts Excel and creates empty worksheet  
	CXLEzAutomation XL;
	//Close Excel if failed to open file 
	if(!XL.OpenExcelFile("C:\\NEXTA_OpenSource\\DTALite\\test_QEM.xlsx"))
	{
		XL.ReleaseExcel();
		//				MessageBox("Failed to Open Excel File", "Error", MB_OK);
		return;
	}

	XL.SetActiveWorksheet(0);
	//To access data use this:
	XL.SetCellValue(2,4,"1");
	CString szCellValue5 = XL.GetCellValue(2, 52);
	//Itterate through all cells given by (Column, Row) to
	//access all data

	//CTestDataSource provide set of data for testing XY plot function
	CTestDataSource DataSource;

	/*	//All data will be stored by szData
	CString szData;
	//Itterate to fill data buffer with 100 data points
	for(int i = 0; i < 100; i++)
	szData = szData + DataSource.GetNextXYPoint();

	//Use clipboard export function to move all data to Excel worksheet
	XL.ExportCString(szData);

	//Y values for this plot are in Column = 2
	XL.CreateXYChart(2);
	*/	
	//	XL.EnableAlert(false);
	XL.ReleaseExcel();


}

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

void CTLiteDoc::ExportNodeLayerToGISFiles(CString file_name, CString GISTypeString)
{
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

}

void CTLiteDoc::ExportLinkLayerToGISFiles(CString file_name, CString GISTypeString)
{
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
		OGRFieldDefn oField11 ("iSpeed", OFTReal); 
		OGRFieldDefn oField12 ("iReliable", OFTReal); 

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
		if( poLayer->CreateField( &oField11 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField11.GetNameRef()); AfxMessageBox(str); return ;	}
		if( poLayer->CreateField( &oField12 ) != OGRERR_NONE ) {  str.Format("Creating field %s failed", oField12.GetNameRef()); AfxMessageBox(str); return ;	}

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
				poFeature->SetField("nLanes", (*iLink)->m_NumLanes );
				poFeature->SetField("SpeedLimit", (*iLink)->m_SpeedLimit );
				poFeature->SetField("LaneCap", (*iLink)->m_LaneCapacity );
				poFeature->SetField("FunctClass", (*iLink)->m_link_type );
				poFeature->SetField("FFTT", (*iLink)->m_FreeFlowTravelTime );

				float value;
				float mobility_index  = GetLinkMOE((*iLink), MOE_speed, 1, 360, value);
				float reliability_index  = GetLinkMOE((*iLink), MOE_reliability, 1, 360, value);
				poFeature->SetField("iSpeed", mobility_index);
				poFeature->SetField("iReliable", reliability_index );

				OGRLineString line;
				for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
				{
					line.addPoint ((*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y,(*iLink)->m_NumLanes);
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

}


void CTLiteDoc::ExportZoneLayerToGISFiles(CString file_name, CString GISTypeString)
{
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

		std::vector<std::string> style_names;
		// blue style
		style_names.push_back("transGreenPoly");
		fprintf(st,"<Style id=\"transGreenPoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ff00</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// red style
		style_names.push_back("transRedPoly");
		fprintf(st,"<Style id=\"transRedPoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d0000ff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// blue style
		style_names.push_back("transBluePoly");
		fprintf(st,"<Style id=\"transBluePoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7dff0000</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// yellow style
		style_names.push_back("transYellowPoly");
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
      	
		float max_zone_demand = 0;

		std::map<int, DTAZone>	:: const_iterator itr_o;
		for(itr_o = m_ZoneMap.begin(); itr_o != m_ZoneMap.end(); itr_o++)
		{
		
			for(std::map<int, DTAZone>	:: const_iterator itr_d = m_ZoneMap.begin(); itr_d != m_ZoneMap.end(); itr_d++)
			{
				float volume = m_ZoneMap[itr_o->first].m_ODDemandMatrix [itr_d->first].GetSubTotalValue ();
				if( volume > max_zone_demand)
					max_zone_demand = volume;

			}

		}

		int time_step = 4;
		for(int t = 6*4; t< 12*4; t+=time_step)
		{
		double ratio = 1;
		
		if(m_DemandProfileVector.size() > 0)
			ratio = m_DemandProfileVector[0].time_dependent_ratio[t];
 
   		std::map<int, DTAZone>	:: const_iterator itr_o;
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
			fprintf(st,"\t\t\t<TimeSpan>\n");

			CString time_stamp_str = GetTimeStampStrFromIntervalNo (t,false);
			CString time_stamp_str_end = GetTimeStampStrFromIntervalNo (t+time_step,false);

			fprintf(st,"\t\t\t<begin>2012-01-%0d</begin>\n",t/4);
			fprintf(st,"\t\t\t<end>2012-01-01-%0d</end>\n",(t+time_step)/4);

//			fprintf(st,"\t\t\t<begin>2012-01-01-T%s:00Z</begin>\n",time_stamp_str);
//			fprintf(st,"\t\t\t<end>2012-01-01-T%s:00Z</end>\n",time_stamp_str_end);

			fprintf(st,"\t\t\t </TimeSpan>\n");
	
        	fprintf(st,"\t\t<visibility>1</visibility>\n");

			int style_no = (t+itr_o->first) % style_names.size();

			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n",style_names[style_no].c_str ());
			fprintf(st,"\t\t<Polygon>\n");
			fprintf(st,"\t\t<extrude>1</extrude>\n");
			fprintf(st,"\t\t<altitudeMode>relativeToGround</altitudeMode>\n");
			fprintf(st,"\t\t<outerBoundaryIs>\n");
			fprintf(st,"\t\t<LinearRing>\n");
			fprintf(st,"\t\t<coordinates>\n");

			for(unsigned int si = 0; si< itr_o->second.m_ShapePoints.size(); si++)
			{
				float height = ratio*total_zone_demand*70000/max(1,max_zone_demand);
				fprintf(st,"\t\t\t%f,%f,%f\n", itr_o->second.m_ShapePoints[si].x, itr_o->second.m_ShapePoints[si].y,height);
			}

			fprintf(st,"\t\t</coordinates>\n");
			fprintf(st,"\t\t</LinearRing>\n");
			fprintf(st,"\t\t</outerBoundaryIs>\n");

			fprintf(st,"\t\t</Polygon>\n");
			fprintf(st,"\t</Placemark>\n");
		}  // for each zone
		}  //for time slice
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


		}
	}
}
void CTLiteDoc::ExportAgentLayerToKMLFiles(CString file_name, CString GISTypeString)
{
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

		std::vector<std::string> style_names;
		// blue style
		style_names.push_back("transGreenPoly");
		fprintf(st,"<Style id=\"transGreenPoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d00ff00</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// red style
		style_names.push_back("transRedPoly");
		fprintf(st,"<Style id=\"transRedPoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7d0000ff</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// blue style
		style_names.push_back("transBluePoly");
		fprintf(st,"<Style id=\"transBluePoly\">\n");
		fprintf(st,"<LineStyle>\n");
		fprintf(st,"<width>1.5</width>\n");
		fprintf(st,"</LineStyle>\n");
		fprintf(st,"<PolyStyle>\n");
		fprintf(st," <color>7dff0000</color>\n");
		fprintf(st,"</PolyStyle>\n");
   		fprintf(st,"</Style>\n");

		// yellow style
		style_names.push_back("transYellowPoly");
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

			int style_no = (itr->first % style_names.size());

			fprintf(st,"\t\t<styleUrl>#%s</styleUrl>\n",style_names[style_no].c_str ());

				poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
				poFeature->SetField("LinkID",(*iLink)->m_LinkID );
				poFeature->SetField("Name", (*iLink)->m_Name.c_str () );
				poFeature->SetField("A_Node", (*iLink)->m_FromNodeNumber );
				poFeature->SetField("B_Node", (*iLink)->m_ToNodeNumber );
				poFeature->SetField("IsOneWay", (*iLink)->m_Direction );
				poFeature->SetField("NumberOfLanes", (*iLink)->m_NumLanes );
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
					line.addPoint ((*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y,(*iLink)->m_NumLanes);
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
