// Dlg_GISDataExchange.cpp : implementation file
//

#include "stdafx.h"
#include "TLite.h"
#include "Dlg_GISDataExchange.h"
#include "MainFrm.h"
#include "Shellapi.h"
#include "CGridListCtrlEx\CGridColumnTraitEdit.h"
#include "CGridListCtrlEx\CGridColumnTraitCombo.h"
#include "CGridListCtrlEx\CGridRowTraitXP.h"
#include "CSVParser.h"
#include "Geometry.h"
// CDlg_GISDataExchange dialog

IMPLEMENT_DYNAMIC(CDlg_GISDataExchange, CDialog)

CDlg_GISDataExchange::CDlg_GISDataExchange(CWnd* pParent /*=NULL*/)
: CDialog(CDlg_GISDataExchange::IDD, pParent)
, m_GIS_ShapeFile(_T(""))
, m_CSV_File(_T(""))
, m_bCreateNodeFromLink(FALSE)
, m_SaveInputNodeLinkFiles(FALSE)
{
}

CDlg_GISDataExchange::~CDlg_GISDataExchange()
{
}

void CDlg_GISDataExchange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_GIS_SHAPE_FILE, m_GIS_ShapeFile);
	DDX_Control(pDX, IDC_LIST1, m_MessageList);
	DDX_Text(pDX, IDC_EDIT_CSV_SHAPE_FILE, m_CSV_File);
	DDX_Control(pDX, IDC_GIS_DATA_TYPE_LIST, m_GISDataType_List);
	DDX_Check(pDX, IDC_CHECK1, m_bCreateNodeFromLink);
	DDX_Check(pDX, IDC_CHECK2, m_SaveInputNodeLinkFiles);
}


BEGIN_MESSAGE_MAP(CDlg_GISDataExchange, CDialog)
	ON_BN_CLICKED(ID_IMPORT_GPS_SHAPE_FILE, &CDlg_GISDataExchange::OnBnClickedImportGpsShapeFile)
	ON_BN_CLICKED(IDC_BUTTON_Find_Sensor_File, &CDlg_GISDataExchange::OnBnClickedButtonFindSensorFile)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDlg_GISDataExchange::OnLbnSelchangeList1)
	ON_BN_CLICKED(ID_EXPORT_CSV_FILE, &CDlg_GISDataExchange::OnBnClickedExportCsvFile)
	ON_BN_CLICKED(ID_EXPORT_GPS_SHAPE_FILE, &CDlg_GISDataExchange::OnBnClickedExportGpsShapeFile)
	ON_BN_CLICKED(ID_EXPORT_KML_FILE, &CDlg_GISDataExchange::OnBnClickedExportKmlFile)
	ON_BN_CLICKED(IDC_BUTTON_View_Sample_File, &CDlg_GISDataExchange::OnBnClickedButtonViewSampleFile)
	ON_BN_CLICKED(IDC_BUTTON_Load_Sample_Shape_File, &CDlg_GISDataExchange::OnBnClickedButtonLoadSampleShapeFile)
	ON_BN_CLICKED(IDC_BUTTON_Import_GIS_Shape_File, &CDlg_GISDataExchange::OnBnClickedButtonImportGisShapeFile)
	ON_BN_CLICKED(IDC_BUTTON_Export_to_CSV_File, &CDlg_GISDataExchange::OnBnClickedButtonExporttoCsvFile)
	ON_BN_CLICKED(IDC_BUTTON_View_EXPORTED_KML_FILE, &CDlg_GISDataExchange::OnBnClickedButtonViewExportedKmlFile)
	ON_BN_CLICKED(IDC_BUTTON_Find_CSF_File, &CDlg_GISDataExchange::OnBnClickedButtonFindCsfFile)
	ON_BN_CLICKED(IDOK, &CDlg_GISDataExchange::OnBnClickedOk)
	ON_BN_CLICKED(ID_EXPORT_GIS_Shape_File, &CDlg_GISDataExchange::OnBnClickedExportGisShapeFile)


END_MESSAGE_MAP()


// CDlg_GISDataExchange message handlers

void CDlg_GISDataExchange::OnBnClickedImportGpsShapeFile()
{
#ifndef _WIN64

	UpdateData(1);
	if(m_GIS_ShapeFile.GetLength () == 0 )
	{
		AfxMessageBox("Please select a file first.");
		return;
	}

	CWaitCursor wait;
	CString message_str;
	OGRRegisterAll();
	OGRDataSource       *poDS;

	bool bFindOverlappingNode = false;

	poDS = OGRSFDriverRegistrar::Open(m_GIS_ShapeFile, FALSE );
	if( poDS == NULL )
	{
		m_MessageList.AddString("Open file failed." );
		return;
	}

	int point_index = 0;
	int poLayers = ((OGRDataSource*)poDS)->GetLayerCount() ;
	for (int i=0; i < poLayers; i++) 
	{

		OGRLayer  *poLayer;

		poLayer = ((OGRDataSource*)poDS)->GetLayer(i);	

		if(poLayer == NULL)
		{
			message_str.Format("Open layer %d failed", i+1);
			m_MessageList.AddString (message_str);
			return;			
		}

		OGRFeature *poFeature;

		int feature_count = 0;

		// 0.03 miles
		// 0.02: shape length / miles
		double threashold = 0.02*0.001;

		poLayer->ResetReading();

		while( (poFeature = poLayer->GetNextFeature()) != NULL )
		{
			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();

			int id = feature_count;

			OGRGeometry *poGeometry;

			poGeometry = poFeature->GetGeometryRef();
			if( poGeometry != NULL 
				&& wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )  // point data
			{
				OGRPoint *poPoint = (OGRPoint *) poGeometry;

				// Create and insert the node
				DTAPoint* pDTAPoint = new DTAPoint;
				pDTAPoint->pt.x = poPoint->getX();
				pDTAPoint->pt.y = poPoint->getY();

				pDTAPoint->m_NodeNumber = id;
				pDTAPoint->m_NodeID = point_index;
				pDTAPoint->m_ZoneID = 0;
				pDTAPoint->m_ControlType = 0;

				m_pDoc->m_DTAPointSet.push_back(pDTAPoint);
				point_index++;
			}
			else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)  // line data
			{
				// Create and insert the node
				DTALine* pDTALine = new DTALine;

				std::string name =  poFeature->GetFieldAsString("Tmc");
				pDTALine->TMC_code = name;

				double Miles = poFeature->GetFieldAsDouble("Miles");

				pDTALine->Miles =  Miles;


				OGRLineString *poLine = (OGRLineString *) poGeometry;

				double shape_len = 0;

				int step = 1;

				// in case there are too many points
				if(poLine->getNumPoints()>=900)
					step = (int)(poLine->getNumPoints()/900)+1;

				for(unsigned int si = 0; si< poLine->getNumPoints(); si+= step)
				{
					GDPoint pt;
					pt.x  =  poLine->getX(si);
					pt.y  =  poLine->getY(si);
					pDTALine->m_ShapePoints .push_back(pt);

				}

				if(m_bCreateNodeFromLink)
				{

					GDPoint start_pt = pDTALine->m_ShapePoints[0];
					GDPoint end_pt = pDTALine->m_ShapePoints[pDTALine->m_ShapePoints.size()-1];


					//find or create from node number


					int Node_Number  =  0;

					if( bFindOverlappingNode ) 
						Node_Number = m_pDoc->FindCloseDTAPoint_NodeNumber(start_pt,threashold);
					if(Node_Number ==0)
					{
						DTAPoint* pDTAPoint = new DTAPoint;
						pDTAPoint->pt = start_pt;

						pDTAPoint->m_NodeNumber = m_pDoc->m_DTAPointSet.size() +1;
						pDTAPoint->m_NodeID = m_pDoc->m_DTAPointSet.size();
						pDTAPoint->m_ZoneID = 0;
						pDTAPoint->m_ControlType = 0;

						m_pDoc->m_DTAPointSet.push_back(pDTAPoint);
						Node_Number = pDTAPoint->m_NodeNumber;
					}
					pDTALine->m_FromNodeNumber = Node_Number;


					//find or create to node number
					Node_Number  = 0;

					if( bFindOverlappingNode ) 
						Node_Number  = m_pDoc->FindCloseDTAPoint_NodeNumber(end_pt,threashold);

					if(Node_Number ==0)
					{
						DTAPoint* pDTAPoint = new DTAPoint;
						pDTAPoint->pt = end_pt;

						pDTAPoint->m_NodeNumber = m_pDoc->m_DTAPointSet.size() +1;
						pDTAPoint->m_NodeID = m_pDoc->m_DTAPointSet.size();
						pDTAPoint->m_ZoneID = 0;
						pDTAPoint->m_ControlType = 0;

						m_pDoc->m_DTAPointSet.push_back(pDTAPoint);
						Node_Number = pDTAPoint->m_NodeNumber;
					}

					pDTALine->m_ToNodeNumber = Node_Number;
				}

				pDTALine->LineID = m_pDoc->m_DTALineSet.size()+1;
				//create link
				m_pDoc->m_DTALineSet.push_back(pDTALine);

				if(m_pDoc->m_DTALineSet.size()%1000 ==0)
				{
					message_str.Format("Processing %d link records.", m_pDoc->m_DTALineSet.size());
					m_MessageList.AddString(message_str);
				}
			}




		}

		// finish reading

		if(m_bCreateNodeFromLink)
		{
			m_pDoc->m_UnitMile  = 0.02;

			m_pDoc->m_UnitFeet = m_pDoc->m_UnitMile/5280.0f;  


			for (std::list<DTAPoint*>::iterator iPoint = m_pDoc->m_DTAPointSet.begin(); 
				iPoint != m_pDoc->m_DTAPointSet.end(); iPoint++)
			{
				int ThisNodeNumber = (*iPoint)->m_NodeNumber;
				int Node_Number  = m_pDoc->FindCloseDTAPoint_NodeNumber((*iPoint)->pt ,threashold, ThisNodeNumber );
				if(Node_Number != ThisNodeNumber && Node_Number!=0)
				{
					// find a close node, create a dummy link 
					m_pDoc->AddNewLinkWithNodeNumbers (ThisNodeNumber,Node_Number);

				}

				if(ThisNodeNumber %1000 == 0)
				{
					message_str.Format("processing %d nodes.", ThisNodeNumber);
					m_MessageList.AddString(message_str);
				}

			}

		}
		message_str.Format("Import %d link records from layer %d.", m_pDoc->m_DTALineSet.size(),i+1);
		m_MessageList.AddString(message_str);


		if(m_bCreateNodeFromLink)
		{
			message_str.Format("Create %d nodes from link shape points", m_pDoc->m_DTAPointSet.size());
			m_MessageList.AddString(message_str);


		}


		OGRFeature::DestroyFeature( poFeature );
		feature_count ++;
	}
	OGRDataSource::DestroyDataSource( poDS );
	if(m_SaveInputNodeLinkFiles)
	{
		SaveTNPProject();

	}

#endif
}

void CDlg_GISDataExchange::OnBnClickedButtonFindSensorFile()
{

	CString str;

	static char BASED_CODE szFilter[] = "GIS Shape File (*.shp)|*.shp||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{

		m_GIS_ShapeFile = dlg.GetPathName();
		UpdateData(false);

	}

}

void CDlg_GISDataExchange::OnLbnSelchangeList1()
{
	// TODO: Add your control notification handler code here
}
void CDlg_GISDataExchange::ExportDataToCSV(CString csv_file_name)
{
#ifndef _WIN64

	CString message_str;
	OGRRegisterAll();
	OGRDataSource       *poDS;

	poDS = OGRSFDriverRegistrar::Open(m_GIS_ShapeFile, FALSE );
	if( poDS == NULL )
	{
		m_MessageList.AddString("Open file failed." );
		return;
	}

	ofstream CSVFile;
	CSVFile.open (csv_file_name, ios::out);

	if(CSVFile.is_open ()  == false)
	{
		AfxMessageBox("This file cannot be found or opened.\n It might be currently used and locked by EXCEL.");
		return;	 
	}else
	{
		CSVFile.width(15);
		CSVFile.precision(6) ;
		CSVFile.setf(ios::fixed);
	}

	int poLayers = ((OGRDataSource*)poDS)->GetLayerCount() ;
	for (int i=0; i < poLayers; i++) 
	{

		OGRLayer  *poLayer;

		poLayer = ((OGRDataSource*)poDS)->GetLayer(i);	

		if(poLayer == NULL)
		{
			message_str.Format("Open layer %d failed", i+1);
			m_MessageList.AddString (message_str);
			return;			
		}

		OGRFeature *poFeature;

		int feature_count = 0;

		poLayer->ResetReading();

		while( (poFeature = poLayer->GetNextFeature()) != NULL )
		{
			OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
			int iField;

			if(feature_count == 0)    // first feature point, output field name;
			{
				for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
				{

					OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
					CString str = poFieldDefn->GetNameRef();
					str.Replace(" ", NULL);  // remove space
					CSVFile <<  str << "," ;

				}

				CSVFile << "geometry" << endl;

			}

			for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
			{

				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
				CString str;

				if( poFieldDefn->GetType() == OFTInteger )
					CSVFile <<  poFeature->GetFieldAsInteger( iField ) << ",";
				else if( poFieldDefn->GetType() == OFTReal )
					CSVFile <<  poFeature->GetFieldAsDouble(iField) << ",";
				else if( poFieldDefn->GetType() == OFTString )
				{
					str = poFeature->GetFieldAsString(iField);
					if(str.Find(',') >=0) 
						CSVFile << "\"" << poFeature->GetFieldAsString(iField)  << "\",";
					else
						CSVFile <<   poFeature->GetFieldAsString(iField)  << ",";
				}
				else
				{
					str = poFeature->GetFieldAsString(iField);
					if(str.Find(',') >=0) 
						CSVFile << "\"" << poFeature->GetFieldAsString(iField)  << "\",";
					else
						CSVFile <<   poFeature->GetFieldAsString(iField)  << ",";
				}

			}

			OGRGeometry *poGeometry;

			poGeometry = poFeature->GetGeometryRef();
			if( poGeometry != NULL )
			{
				if(wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
				{
					OGRPoint *poPoint = (OGRPoint *) poGeometry;

					CSVFile << "\"<Point><coordinates>" <<  poPoint->getX() << ","  << poPoint->getY() << ",0.0" << "</coordinates></Point>\"" ;
				}
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
				{
					OGRLineString *poLine = (OGRLineString *) poGeometry;

					CSVFile << "\"<LineString><coordinates>";

					for(unsigned int si = 0; si< poLine->getNumPoints(); si++)
					{
						CSVFile	 <<  poLine->getX(si) << ","  << poLine->getY(si) << ",0.0";

						if(si!=poLine->getNumPoints()-1)
							CSVFile << " ";
					}

					CSVFile << "</coordinates></LineString>\",";

				} if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon )
				{

					OGRPolygon* polygon = (OGRPolygon*)(poGeometry);

					OGRLinearRing *ring = polygon->getExteriorRing();
					OGRPoint point;

					CSVFile << "\"<Polygon><outerBoundaryIs><LinearRing><coordinates>";

					for(int i = 0; i < ring->getNumPoints(); i++)
					{
						ring->getPoint(i, &point);
						CSVFile	 <<  point.getX() << ","  << point.getY() << ",0.0";

						if(i!=ring->getNumPoints()-1)
							CSVFile << " ";
					}

					CSVFile << "</coordinates></LinearRing></outerBoundaryIs></Polygon>\"";
				}
				CSVFile << endl;

			}
			feature_count ++;
		}
		OGRFeature::DestroyFeature( poFeature );
		message_str.Format("Layer %d has %d features.", i+1, feature_count);
		m_MessageList.AddString(message_str);


	}

	OGRDataSource::DestroyDataSource( poDS );

	CSVFile.close();
#endif
}

void CDlg_GISDataExchange::OnBnClickedExportCsvFile()
{

	CString m_CSV_FileName;
	CFileDialog dlg (FALSE, "*.csv", "*.csv",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"(*.csv)|*.csv||", NULL);
	if(dlg.DoModal() == IDOK)
	{
		ExportDataToCSV(dlg.GetPathName());
		m_pDoc->OpenCSVFileInExcel (dlg.GetPathName());

	}


}

void CDlg_GISDataExchange::OnBnClickedExportGpsShapeFile()
{
	m_MessageList.ResetContent ();
	CString m_CSV_FileName;
	CFileDialog dlg (FALSE, "*.shp", "*.shp",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"(*.shp)|*.shp||", NULL);
	if(dlg.DoModal() == IDOK)
	{
		ExportLinkLayerToShapeFile(dlg.GetPathName());
		m_pDoc->OpenCSVFileInExcel(dlg.GetPathName());
	}
}

void CDlg_GISDataExchange::ExportLinkLayerToShapeFile(CString file_name)
{
	//ExportToGISFile(file_name,  "ESRI Shapefile");

}
void CDlg_GISDataExchange::OnBnClickedExportKmlFile()
{
	//	CString m_CSV_FileName;
	//CFileDialog dlg (FALSE, "*.kml", "*.kml",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
	//	"(*.kml)|*.kml||", NULL);
	//if(dlg.DoModal() == IDOK)
	//{
	//	ExportToGISFile(dlg.GetPathName(), "KML");
	//}
}

void CDlg_GISDataExchange::ExportToGISFile(LPCTSTR lpszCSVFileName,LPCTSTR lpszShapeFileName, CString GISTypeString )
{

#ifndef _WIN64

	m_MessageList.ResetContent ();

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
			m_MessageList.AddString (message_str);
			return;
		}

		OGRDataSource *poDS;

		poDS = poDriver->CreateDataSource(lpszShapeFileName, NULL );
		if( poDS == NULL )
		{
			message_str.Format ( "Creation of GIS output file %s failed.\nPlease do not overwrite the exiting file and please select a new file name.", 
				lpszShapeFileName );
			m_MessageList.AddString (message_str);
			return;
		}

		///// export to link layer

		// link layer 

		OGRLayer *poLayer;
		poLayer = poDS->CreateLayer( "link", NULL, wkbLineString, NULL );
		if( poLayer == NULL )
		{
			m_MessageList.AddString ("link Layer creation failed");
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

					m_MessageList.AddString (str);
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

					m_MessageList.AddString (str);
					return; 
				}

			}

			if(HeaderVector[i].size()>=11)
			{
				LongFieldVector.push_back (HeaderVector[i].c_str ());
			}

		}

		message_str.Format ("%d fields have been created.",HeaderVector.size());
		m_MessageList.AddString (message_str);

		if(LongFieldVector.size() >=1)
		{
			message_str.Format("Warning: Arc GIS file only supports field names with not more than 10 characters.\nThe following fields have long field names. "); 
			m_MessageList.AddString (message_str);
			for(unsigned l = 0; l< LongFieldVector.size(); l++)
			{
				message_str.Format ("%s",LongFieldVector[l]);
				m_MessageList.AddString (message_str);


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

				if( m_GIS_data_type == GIS_Point_Type && CoordinateVector.size ()==1)
				{
					OGRPoint pt;
					pt.setX( CoordinateVector[0].X );
					pt.setY( CoordinateVector[0].Y);
					poFeature->SetGeometry( &pt ); 

				}



				if( m_GIS_data_type == GIS_Line_Type)
				{


					OGRLineString line;
					for(unsigned int si = 0; si< CoordinateVector.size(); si++)
					{
						line.addPoint (CoordinateVector[si].X , CoordinateVector[si].Y);
					}

					poFeature->SetGeometry( &line ); 
				}


				if( m_GIS_data_type == GIS_Polygon_Type)
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


			} else
			{ // no geometry field


				/// create geometry data from m_GIS_data_type == GIS_Point_Type

				if( m_GIS_data_type == GIS_Point_Type )
				{

					double x, y;
					if(parser.GetValueByFieldName("x",x) && parser.GetValueByFieldName("y",y) )
					{
						OGRPoint pt;
						pt.setX( CoordinateVector[0].X );
						pt.setY( CoordinateVector[0].Y);
						poFeature->SetGeometry( &pt ); 

					}else
					{
						AfxMessageBox("Pleaes prepare fields x and y in the csv file in order to create a node GIS layer.", MB_ICONINFORMATION);
						return;

					}

				}

				///create geometry

				if( m_GIS_data_type == GIS_Line_Type)
				{

					int number_of_shape_points = 0;
					if(parser.GetValueByFieldName("number_of_shape_points", number_of_shape_points))
					{

						if(number_of_shape_points>=2)
						{

							OGRLineString line;

							for(int s= 1; s<= number_of_shape_points; s++)
							{
								CString str_x, str_y;
								str_x.Format ("x%d",s);
								str_y.Format ("y%d",s);
								double x = 0;
								double y = 0;

								string string_x, string_y;
								string_x  = m_pDoc->CString2StdString (str_x);
								string_y  = m_pDoc->CString2StdString (str_y);

								if(parser.GetValueByFieldName(string_x, x) && parser.GetValueByFieldName(string_y, y))
								{
									line.addPoint(x,y);
								}else
								{
									AfxMessageBox("Pleaes prepare fields x1,y1,x2,y2,...,xn,yn in the csv file in order to create a link GIS layer.", MB_ICONINFORMATION);

									return; 
								}


							}
							poFeature->SetGeometry( &line ); 

						}

					}else
					{ 
						AfxMessageBox("Pleaes prepare fields number_of_shape_points, x1,y1,x2,y2,...,xn,yn in the csv file in order to create a link GIS layer.", MB_ICONINFORMATION);
						return;
					}


				}

				// 


				if( m_GIS_data_type == GIS_Polygon_Type)
				{

					OGRPolygon polygon;
					OGRLinearRing  ring;

					int number_of_shape_points = 0;
					if(parser.GetValueByFieldName("number_of_shape_points", number_of_shape_points))
					{

						if(number_of_shape_points>=2)
						{

							OGRLineString line;

							for(int s= 0; s< number_of_shape_points; s++)
							{
								CString str_x, str_y;
								str_x.Format ("x%d",str_x);
								str_y.Format ("y%d",str_y);
								double x = 0;
								double y = 0;

								string string_x, string_y;
								string_x  = m_pDoc->CString2StdString (str_x);
								string_y  = m_pDoc->CString2StdString (str_y);

								if(parser.GetValueByFieldName(string_x, x) && parser.GetValueByFieldName(string_y, y))
								{
									ring.addPoint (x,y,1);
								}else
								{
									AfxMessageBox("Pleaes prepare fields x1,y1,x2,y2,...,xn,yn in the csv file in order to create a zone GIS layer.", MB_ICONINFORMATION);

									return; 
								}

							}
							polygon.addRing(&ring);

							poFeature->SetGeometry( &polygon ); 

						}


					}


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
		m_MessageList.AddString (message_str);


		OGRDataSource::DestroyDataSource( poDS );

		CString ShapeFile = lpszShapeFileName;
		CString ShapeFileFolder = ShapeFile.Left(ShapeFile.ReverseFind('\\') + 1);

		ShellExecute( NULL,  "explore", ShapeFileFolder, NULL,  NULL, SW_SHOWNORMAL );

	}
#endif

}

void CDlg_GISDataExchange::OnBnClickedButtonViewSampleFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleShapeFileFolder = "\\importing_sample_data_sets\\Utah_90th_South_Network\\";
	SampleShapeFileFolder = pMainFrame->m_CurrentDirectory + SampleShapeFileFolder;
	ShellExecute( NULL,  "explore", SampleShapeFileFolder, NULL,  NULL, SW_SHOWNORMAL );
}

void CDlg_GISDataExchange::OnBnClickedButtonLoadSampleShapeFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleShapeFile = "\\importing_sample_data_sets\\Utah_90th_South_Network\\SLC_90_Work_Zone_Network_Link.shp";
	SampleShapeFile = pMainFrame->m_CurrentDirectory + SampleShapeFile;
	m_GIS_ShapeFile = SampleShapeFile;
	UpdateData(false);
}

void CDlg_GISDataExchange::OnBnClickedButtonImportGisShapeFile()
{
	OnBnClickedImportGpsShapeFile();

}

void CDlg_GISDataExchange::OnBnClickedButtonExporttoCsvFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleShapeCSVFile = "\\importing_sample_data_sets\\Utah_90th_South_Network\\SLC_Link.csv";
	SampleShapeCSVFile = pMainFrame->m_CurrentDirectory + SampleShapeCSVFile;
	m_pDoc->OpenCSVFileInExcel (SampleShapeCSVFile);

}
void CDlg_GISDataExchange::OnBnClickedButtonViewExportedKmlFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleShapeCSVFile = "\\importing_sample_data_sets\\Utah_90th_South_Network\\SLC_Link.KML";
	SampleShapeCSVFile = pMainFrame->m_CurrentDirectory + SampleShapeCSVFile;
	m_pDoc->OpenCSVFileInExcel (SampleShapeCSVFile);
}

void CDlg_GISDataExchange::OnBnClickedButtonFindCsfFile()
{
	CString str;

	static char BASED_CODE szFilter[] = "CSV File (*.csv)|*.csv||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{

		m_CSV_File = dlg.GetPathName();
		UpdateData(false);

	}
}

void CDlg_GISDataExchange::OnBnClickedExportShapeFile()
{
}

void CDlg_GISDataExchange::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void CDlg_GISDataExchange::OnBnClickedExportGisShapeFile()
{
	m_MessageList.ResetContent ();
	CString m_CSV_FileName;
	CFileDialog dlg (FALSE, "*.shp", "*.shp",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"(*.shp)|*.shp||", NULL);
	if(dlg.DoModal() == IDOK)
	{
		m_GIS_data_type = (_GIS_DATA_TYPE)(m_GISDataType_List.GetCurSel ());
		ExportToGISFile(m_CSV_File,dlg.GetPathName(), "ESRI Shapefile");
	}
}

BOOL CDlg_GISDataExchange::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_GISDataType_List.AddString ("Point");
	m_GISDataType_List.AddString ("Line");
	m_GISDataType_List.AddString ("Polygon");
	m_GISDataType_List.SetCurSel (1);  // default line type

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlg_GISDataExchange::SaveTNPProject()
{
	CString directory;
	directory = m_GIS_ShapeFile.Left(m_GIS_ShapeFile.ReverseFind('\\') + 1);
	CString ProjectFileName = m_GIS_ShapeFile.Left(m_GIS_ShapeFile.ReverseFind('.') + 1);

	DeleteFile(ProjectFileName+"tnp");
	DeleteFile(directory+"input_node.csv");
	DeleteFile(directory+"input_link.csv");


	FILE* st =NULL;
	fopen_s(&st,ProjectFileName+"tnp","w");
	if(st!=NULL)
	{
		fprintf(st,"Imported from shape file %s.", ProjectFileName);

		fclose(st);
	}

	fopen_s(&st,directory+"input_node.csv","w");
	if(st!=NULL)
	{
		std::list<DTANode*>::iterator iNode;
		fprintf(st, "name,node_id,x,y,geometry\n");
		for (std::list<DTAPoint*>::iterator iPoint = m_pDoc->m_DTAPointSet.begin(); 
			iPoint !=m_pDoc-> m_DTAPointSet.end(); iPoint++)
		{

			//				std::replace( (*iNode)->m_Name.begin(), (*iNode)->m_Name.end(), ',', ' '); 

			fprintf(st, "%s,%d,%f,%f,\"<Point><coordinates>%f,%f</coordinates></Point>\"\n", 
				(*iPoint)->m_Name .c_str (),
				(*iPoint)->m_NodeNumber ,
				(*iPoint)->pt .x ,(*iPoint)->pt .y,(*iPoint)->pt .x ,(*iPoint)->pt .y);

		}

		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_node.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return;
	}

	fopen_s(&st,directory+"input_link.csv","w");
	if(st!=NULL)
	{
		std::list<DTALink*>::iterator iLink;
		fprintf(st,"link_id,TMC,from_node_id,to_node_id,length_in_mile,link_type,number_of_lanes,speed_limit_in_mph,lane_capacity_in_vhc_per_hour,geometry\n");

		std::list<DTALine*>::iterator iLine;

		for (iLine = m_pDoc->m_DTALineSet.begin(); iLine != m_pDoc->m_DTALineSet.end(); iLine++)
		{

			fprintf(st,"%d,%s,%d,%d,%.3f,1,1,50,1000,",  // default value
				(*iLine)->LineID , 
				(*iLine)->TMC_code.c_str (), 
				(*iLine)->m_FromNodeNumber ,
				(*iLine)->m_ToNodeNumber ,(*iLine)->Miles);

			// geometry
			fprintf(st,"\"<LineString><coordinates>");

			for(unsigned int si = 0; si< (*iLine)->m_ShapePoints.size(); si++)
			{
				fprintf(st,"%f,%f,0.0",(*iLine)->m_ShapePoints[si].x, (*iLine)->m_ShapePoints[si].y);

				if(si!=(*iLine)->m_ShapePoints.size()-1)
					fprintf(st," ");
			}

			fprintf(st,"</coordinates></LineString>\",");
			fprintf(st,"\n");

		}


		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_link.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return;
	}
}