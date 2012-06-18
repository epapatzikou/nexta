#include "stdafx.h"
#include "..\\TLite.h"
#include "..\\Network.h"
#include "..\\TLiteDoc.h"
#include "..\\TLiteView.h"
#include "include\\ogrsf_frmts.h"

void CTLiteDoc::ExportOGRShapeFile()
{

}

void CTLiteDoc::ImportOGRShapeFile(CString FileName)
{
	
		OGRRegisterAll();
			OGRDataSource       *poDS;

			poDS = OGRSFDriverRegistrar::Open(FileName, FALSE );
			if( poDS == NULL )
			{
				AfxMessageBox( "Open file failed." );
				return;
			}

	int poLayers = ((OGRDataSource*)poDS)->GetLayerCount() ;
    for (int i=0; i < poLayers; i++) 
    {

			OGRLayer  *poLayer;

	        poLayer = ((OGRDataSource*)poDS)->GetLayer(i);	

			if(poLayer == NULL)
			{
				AfxMessageBox( "Open layer failed." );
				return;			
			}

			OGRFeature *poFeature;

			poLayer->ResetReading();

			while( (poFeature = poLayer->GetNextFeature()) != NULL )
			{
				OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
				int iField;

				for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
				{
					OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );

					if( poFieldDefn->GetType() == OFTInteger )
						TRACE( "%d,", poFeature->GetFieldAsInteger( iField ) );
					else if( poFieldDefn->GetType() == OFTReal )
						TRACE( "%.3f,", poFeature->GetFieldAsDouble(iField) );
					else if( poFieldDefn->GetType() == OFTString )
						TRACE( "%s,", poFeature->GetFieldAsString(iField) );
					else
						TRACE( "%s,", poFeature->GetFieldAsString(iField) );
				}

				OGRGeometry *poGeometry;

				TRACE("    ");
				poGeometry = poFeature->GetGeometryRef();
				if( poGeometry != NULL 
					&& wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
				{
					OGRPoint *poPoint = (OGRPoint *) poGeometry;

					TRACE( "x = %f ,y = %f\n", poPoint->getX(), poPoint->getY() );
				}
				else
				{
					TRACE( "no point geometry\n" );
				}       
				OGRFeature::DestroyFeature( poFeature );
			}
	}

			OGRDataSource::DestroyDataSource( poDS );

}