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

// CDlg_GISDataExchange dialog

IMPLEMENT_DYNAMIC(CDlg_GISDataExchange, CDialog)

CDlg_GISDataExchange::CDlg_GISDataExchange(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_GISDataExchange::IDD, pParent)
	, m_GIS_ShapeFile(_T(""))
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
END_MESSAGE_MAP()


// CDlg_GISDataExchange message handlers

void CDlg_GISDataExchange::OnBnClickedImportGpsShapeFile()
{

	if(m_GIS_ShapeFile.GetLength () == 0 )
	{
	AfxMessageBox("Please select a file first.");
	return;
	}

	CWaitCursor wait;
	CString message_str;
	OGRRegisterAll();
			OGRDataSource       *poDS;

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
			poLayer->ResetReading();

			while( (poFeature = poLayer->GetNextFeature()) != NULL )
			{
				OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
				int iField;

				int id = feature_count;

				OGRGeometry *poGeometry;

				poGeometry = poFeature->GetGeometryRef();
				if( poGeometry != NULL 
					&& wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
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
				else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
				{
								// Create and insert the node
						DTALine* pDTALine = new DTALine;

					    OGRLineString *poLine = (OGRLineString *) poGeometry;

						for(unsigned int si = 0; si< poLine->getNumPoints(); si++)
						{
							GDPoint pt;
							pt.x  =  poLine->getX(si);
							pt.y  =  poLine->getY(si);
							pDTALine->m_ShapePoints .push_back(pt);

						}
						m_pDoc->m_DTALineSet.push_back(pDTALine);
				

				}
   
				OGRFeature::DestroyFeature( poFeature );
				feature_count ++;
			}
				message_str.Format("Import %d point records from layer %d.", feature_count,i+1);
				m_MessageList.AddString(message_str);
	}

			OGRDataSource::DestroyDataSource( poDS );

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
}

void CDlg_GISDataExchange::OnBnClickedExportCsvFile()
{
	if(m_GIS_ShapeFile.GetLength () == 0 )
	{
	AfxMessageBox("Please select a shape file first.");
	return;
	}

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
	ExportToGISFile(file_name,  "ESRI Shapefile");

}
void CDlg_GISDataExchange::OnBnClickedExportKmlFile()
{
		CString m_CSV_FileName;
	CFileDialog dlg (FALSE, "*.kml", "*.kml",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
		"(*.kml)|*.kml||", NULL);
	if(dlg.DoModal() == IDOK)
	{
		ExportToGISFile(dlg.GetPathName(), "KML");
	}
}

void CDlg_GISDataExchange::ExportToGISFile(CString file_name, CString GISTypeString)
{
	CString message_str;

    OGRSFDriver *poDriver;

    OGRRegisterAll();

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(
                GISTypeString );
    if( poDriver == NULL )
    {
        message_str.Format ( "%s driver not available.", GISTypeString );
		m_MessageList.AddString (message_str);
        return;
    }

    OGRDataSource *poDS;

    poDS = poDriver->CreateDataSource( file_name, NULL );
    if( poDS == NULL )
    {
        message_str.Format ( "Creation of output file %s failed", file_name );
		m_MessageList.AddString (message_str);
		return;
    }

	///// export to node layer
    OGRLayer *poLayer;

    poLayer = poDS->CreateLayer( "export_node", NULL, wkbPoint, NULL );
    if( poLayer == NULL )
    {
		m_MessageList.AddString ("Layer creation failed");
		return;
    }

    OGRFieldDefn oField( "Name", OFTString );

    oField.SetWidth(32);

    if( poLayer->CreateField( &oField ) != OGRERR_NONE )
    {
   
		m_MessageList.AddString ("Creating Name field failed.");
		return;		
	}

	std::list<DTAPoint*>::iterator iPoint;
		
	for (iPoint = m_pDoc->m_DTAPointSet.begin(); iPoint != m_pDoc->m_DTAPointSet.end(); iPoint++)
	{

        OGRFeature *poFeature;

        poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
        poFeature->SetField( "Name", (*iPoint)->m_NodeID );

        OGRPoint pt;
        
        pt.setX( (*iPoint)->pt .x );
        pt.setY( (*iPoint)->pt .y );
 
        poFeature->SetGeometry( &pt ); 

        if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
        {
           printf( "Failed to create feature in shapefile.\n" );
           exit( 1 );
        }


         OGRFeature::DestroyFeature( poFeature );
    }

    OGRDataSource::DestroyDataSource( poDS );

}
void CDlg_GISDataExchange::OnBnClickedButtonViewSampleFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleShapeFileFolder = "\\Sample-Portland-SHRP2-C05-subarea\\GIS_shape_files\\";
	SampleShapeFileFolder = pMainFrame->m_CurrentDirectory + SampleShapeFileFolder;
	ShellExecute( NULL,  "explore", SampleShapeFileFolder, NULL,  NULL, SW_SHOWNORMAL );
}

void CDlg_GISDataExchange::OnBnClickedButtonLoadSampleShapeFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleShapeFile = "\\Sample-Portland-SHRP2-C05-subarea\\GIS_shape_files\\Subarea_Highway_Crashes.shp";
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
	CString SampleShapeCSVFile = "\\Sample-Portland-SHRP2-C05-subarea\\GIS_shape_files\\crash.csv";
	SampleShapeCSVFile = pMainFrame->m_CurrentDirectory + SampleShapeCSVFile;
	m_pDoc->OpenCSVFileInExcel (SampleShapeCSVFile);

}
void CDlg_GISDataExchange::OnBnClickedButtonViewExportedKmlFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	CString SampleShapeCSVFile = "\\Sample-Portland-SHRP2-C05-subarea\\GIS_shape_files\\crash.kml";
	SampleShapeCSVFile = pMainFrame->m_CurrentDirectory + SampleShapeCSVFile;
	m_pDoc->OpenCSVFileInExcel (SampleShapeCSVFile);
}
