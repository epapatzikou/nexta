//TLiteDoc.cpp : implementation of the CTLiteDoc class
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
#include "Geometry.h"
#include "CSVParser.h"
#include "RecordSetExt.h"
#include "TLite.h"
#include "Network.h"
#include "TLiteDoc.h"
#include "TLiteView.h"
#include "DlgMOE.h"
#include "DlgPathMOE.h"
#include "DlgTrainInfo.h"
#include "DlgFileLoading.h"
#include "DlgCarFollowing.h"
#include "MainFrm.h"
#include "Shellapi.h"
#include "DlgDefaultLinkProperties.h"
#include "DlgAssignmentSettings.h"
#include "CGridListCtrlEx\\CGridListCtrlEx.h"

#include "Zip_utility\\zip.h"
#include "Zip_utility\\unzip.h"

#include "DlgLinkList.h"
#include "DlgPathList.h"
#include "Dlg_UnitTestingList.h"
#include "DlgGridCtrl.h"
#include "Dlg_ImageSettings.h"
#include "Shellapi.h"
#include "DlgSensorDataLoading.h"
#include "Dlg_ImportODDemand.h"
#include "DlgNetworkAlignment.h"
#include "Dlg_VehEmissions.h"
#include "DlgScenario.h"
#include "DlgMOETabView.h"
#include "Dlg_ImportShapeFiles.h"
#include "Dlg_ImportPlanningModel.h"
#include "Dlg_ImportNetwork.h"
#include "Dlg_ImportPointSensor.h"
#include "Dlg_SignalDataExchange.h"

#include "Dlg_TDDemandProfile.h"
#include "Dlg_PricingConfiguration.h"
#include "Dlg_LinkVisualizationConfig.h"

#include "Data-Interface\\XLEzAutomation.h"
#include "Data-Interface\\XLTestDataSource.h"
#include "Dlg_VehicleClassification.h"
#include "Dlg_Find_Vehicle.h"
#include "Dlg_TravelTimeReliability.h"
#include "Dlg_GISDataExchange.h"
#include "Dlg_Legend.h"

#include "LinePlot\\LinePlotTest.h"
#include "LinePlot\\LinePlotTestDlg.h"
#include "Page_Node_Movement.h"

#include "AssignmentSimulationSettingDlg.h"
#include "NetworkDataDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDlgMOE *g_LinkMOEDlg = NULL;
CDlgPathMOE	*g_pPathMOEDlg = NULL;
CDlg_Legend* g_pLegendDlg = NULL;
CDlgLinkList* g_pLinkListDlg = NULL;
CDlg_VehPathAnalysis* g_pVehiclePathDlg = NULL;
CDlgPathList* g_pPathListDlg = NULL;
CDlg_UnitTestingList* g_pUnitTestingListDlg = NULL;

bool g_bShowLinkList = false;
bool g_bShowVehiclePathDialog = false;



extern float g_Simulation_Time_Stamp;
bool g_LinkMOEDlgShowFlag = false;
std::list<s_link_selection>	g_LinkDisplayList;
std::list<CTLiteDoc*>	g_DocumentList;

std::vector<CDlg_VehicleClassification*>	g_SummaryDialogVector;

bool g_bValidDocumentChanged = false;
std::list<CTLiteView*>	g_ViewList;



void g_ClearLinkSelectionList()
{
	std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
	while (iDoc != g_DocumentList.end())
	{
		for (std::list<DTALink*>::iterator iLink = (*iDoc)->m_LinkSet.begin(); iLink != (*iDoc)->m_LinkSet.end(); iLink++)
		{
			(*iLink)->m_DisplayLinkID = -1;
		}
		iDoc++;
	}

	g_LinkDisplayList.clear ();
}

void g_AddLinkIntoSelectionList(int link_no, int document_no, bool b_SelectOtherDocuments, double x, double y)
{
	s_link_selection element;
	element.link_no = link_no;
	element.document_no = document_no;

	g_LinkDisplayList.push_back(element);

	std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
	while (iDoc != g_DocumentList.end())
	{
		if((*iDoc)->m_DocumentNo != document_no)  // not the current document
		{
			GDPoint point;
			point.x = x;
			point.y = y;
			double matching_distance = 0;
			int sel_link_no = (*iDoc)->SelectLink(point, matching_distance);
			if(sel_link_no>=0)  //select link
			{
				element.link_no = sel_link_no;
				element.document_no = (*iDoc)->m_DocumentNo ;
				g_LinkDisplayList.push_back(element);
			}
			//update anyway
			(*iDoc)->UpdateAllViews (0);

		}

		iDoc++;
	}

}
// CTLiteDoc

IMPLEMENT_DYNCREATE(CTLiteDoc, CDocument)

BEGIN_MESSAGE_MAP(CTLiteDoc, CDocument)
	ON_COMMAND(ID_FILE_OPEN, &CTLiteDoc::OnFileOpen)
	ON_COMMAND(ID_SHOW_SHOWPATHMOE, &CTLiteDoc::OnShowShowpathmoe)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SHOWPATHMOE, &CTLiteDoc::OnUpdateShowShowpathmoe)
	ON_COMMAND(ID_SEARCH_LISTTRAINS, &CTLiteDoc::OnSearchListtrains)
	ON_COMMAND(ID_TIMETABLE_IMPORTTIMETABLE, &CTLiteDoc::OnTimetableImporttimetable)
	ON_COMMAND(ID_FILE_SAVE_PROJECT, &CTLiteDoc::OnFileSaveProject)
	ON_COMMAND(ID_FILE_SAVE_PROJECT_AS, &CTLiteDoc::OnFileSaveProjectAs)
	ON_COMMAND(ID_ESTIMATION_ODESTIMATION, &CTLiteDoc::OnEstimationOdestimation)
	ON_COMMAND(ID_IMAGE_IMPORTBACKGROUNDIMAGE, &CTLiteDoc::OnImageImportbackgroundimage)
	ON_COMMAND(ID_FILE_DATALOADINGSTATUS, &CTLiteDoc::OnFileDataloadingstatus)
	ON_COMMAND(ID_MOE_VOLUME, &CTLiteDoc::OnMoeVolume)
	ON_COMMAND(ID_MOE_SPEED, &CTLiteDoc::OnMoeSpeed)
	ON_COMMAND(ID_MOE_DENSITY, &CTLiteDoc::OnMoeDensity)
	ON_COMMAND(ID_MOE_QUEUELENGTH, &CTLiteDoc::OnMoeQueuelength)
	ON_COMMAND(ID_MOE_FUELCONSUMPTION, &CTLiteDoc::OnMoeFuelconsumption)
	ON_COMMAND(ID_MOE_EMISSIONS, &CTLiteDoc::OnMoeEmissions)
	ON_UPDATE_COMMAND_UI(ID_MOE_VOLUME, &CTLiteDoc::OnUpdateMoeVolume)
	ON_UPDATE_COMMAND_UI(ID_MOE_SPEED, &CTLiteDoc::OnUpdateMoeSpeed)
	ON_UPDATE_COMMAND_UI(ID_MOE_DENSITY, &CTLiteDoc::OnUpdateMoeDensity)
	ON_UPDATE_COMMAND_UI(ID_MOE_QUEUELENGTH, &CTLiteDoc::OnUpdateMoeQueuelength)
	ON_UPDATE_COMMAND_UI(ID_MOE_FUELCONSUMPTION, &CTLiteDoc::OnUpdateMoeFuelconsumption)
	ON_UPDATE_COMMAND_UI(ID_MOE_EMISSIONS, &CTLiteDoc::OnUpdateMoeEmissions)
	ON_COMMAND(ID_MOE_NONE, &CTLiteDoc::OnMoeNone)
	ON_UPDATE_COMMAND_UI(ID_MOE_NONE, &CTLiteDoc::OnUpdateMoeNone)
	ON_COMMAND(ID_TOOLS_CARFOLLOWINGSIMULATION, &CTLiteDoc::OnToolsCarfollowingsimulation)
	ON_COMMAND(ID_TOOLS_PERFORMTRAFFICASSIGNMENT, &CTLiteDoc::OnToolsPerformtrafficassignment)
	ON_COMMAND(ID_MOE_VC_Ratio, &CTLiteDoc::OnMoeVcRatio)
	ON_UPDATE_COMMAND_UI(ID_MOE_VC_Ratio, &CTLiteDoc::OnUpdateMoeVcRatio)
	ON_COMMAND(ID_MOE_TRAVELTIME, &CTLiteDoc::OnMoeTraveltime)
	ON_UPDATE_COMMAND_UI(ID_MOE_TRAVELTIME, &CTLiteDoc::OnUpdateMoeTraveltime)
	ON_COMMAND(ID_MOE_CAPACITY, &CTLiteDoc::OnMoeCapacity)
	ON_UPDATE_COMMAND_UI(ID_MOE_CAPACITY, &CTLiteDoc::OnUpdateMoeCapacity)
	ON_COMMAND(ID_MOE_SPEEDLIMIT, &CTLiteDoc::OnMoeSpeedlimit)
	ON_UPDATE_COMMAND_UI(ID_MOE_SPEEDLIMIT, &CTLiteDoc::OnUpdateMoeSpeedlimit)
	ON_COMMAND(ID_MOE_FREEFLOWTRAVLETIME, &CTLiteDoc::OnMoeFreeflowtravletime)
	ON_UPDATE_COMMAND_UI(ID_MOE_FREEFLOWTRAVLETIME, &CTLiteDoc::OnUpdateMoeFreeflowtravletime)
	ON_COMMAND(ID_EDIT_DELETESELECTEDLINK, &CTLiteDoc::OnEditDeleteselectedlink)
	ON_COMMAND(ID_MOE_LENGTH, &CTLiteDoc::OnMoeLength)
	ON_UPDATE_COMMAND_UI(ID_MOE_LENGTH, &CTLiteDoc::OnUpdateMoeLength)
	ON_COMMAND(ID_EDIT_SETDEFAULTLINKPROPERTIESFORNEWLINKS, &CTLiteDoc::OnEditSetdefaultlinkpropertiesfornewlinks)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SETDEFAULTLINKPROPERTIESFORNEWLINKS, &CTLiteDoc::OnUpdateEditSetdefaultlinkpropertiesfornewlinks)
	ON_COMMAND(ID_TOOLS_PROJECTFOLDER, &CTLiteDoc::OnToolsProjectfolder)
	ON_COMMAND(ID_TOOLS_OPENNEXTAPROGRAMFOLDER, &CTLiteDoc::OnToolsOpennextaprogramfolder)
	ON_COMMAND(ID_MOE_NOODMOE, &CTLiteDoc::OnMoeNoodmoe)
	ON_UPDATE_COMMAND_UI(ID_MOE_NOODMOE, &CTLiteDoc::OnUpdateMoeNoodmoe)
	ON_COMMAND(ID_ODTABLE_IMPORT_OD_TRIP_FILE, &CTLiteDoc::OnOdtableImportOdTripFile)
	ON_COMMAND(ID_TOOLS_EDITASSIGNMENTSETTINGS, &CTLiteDoc::OnToolsEditassignmentsettings)
	ON_COMMAND(ID_TOOLS_EDITODDEMANDTABLE, &CTLiteDoc::OnToolsEditoddemandtable)
	ON_COMMAND(ID_SEARCH_LINKLIST, &CTLiteDoc::OnSearchLinklist)
	ON_COMMAND(ID_MOE_VEHICLE, &CTLiteDoc::OnMoeVehicle)
	ON_UPDATE_COMMAND_UI(ID_MOE_VEHICLE, &CTLiteDoc::OnUpdateMoeVehicle)
	ON_COMMAND(ID_TOOLS_VIEWSIMULATIONSUMMARY, &CTLiteDoc::OnToolsViewsimulationsummary)
	ON_COMMAND(ID_TOOLS_VIEWASSIGNMENTSUMMARYLOG, &CTLiteDoc::OnToolsViewassignmentsummarylog)
	ON_COMMAND(ID_HELP_VISITDEVELOPMENTWEBSITE, &CTLiteDoc::OnHelpVisitdevelopmentwebsite)
	ON_COMMAND(ID_TOOLS_RUNTRAFFICASSIGNMENT, &CTLiteDoc::OnToolsRuntrafficassignment)
	ON_COMMAND(ID_IMPORTODTRIPFILE_3COLUMNFORMAT, &CTLiteDoc::OnImportodtripfile3columnformat)
	ON_COMMAND(ID_TOOLS_PERFORMSCHEDULING, &CTLiteDoc::OnToolsPerformscheduling)
	ON_COMMAND(ID_FILE_CHANGECOORDINATESTOLONG, &CTLiteDoc::OnFileChangecoordinatestolong)
	ON_COMMAND(ID_TOOLS_EXPORTOPMODEDISTRIBUTION, &CTLiteDoc::OnToolsExportopmodedistribution)
	ON_COMMAND(ID_TOOLS_ENUMERATEPATH, &CTLiteDoc::OnToolsEnumeratepath)
	ON_COMMAND(ID_TOOLS_EXPORTTOTIME, &CTLiteDoc::OnToolsExporttoHistDatabase)
	ON_COMMAND(ID_RESEARCHTOOLS_EXPORTTODTALITESENSORDATAFORMAT, &CTLiteDoc::OnResearchtoolsExporttodtalitesensordataformat)
	ON_COMMAND(ID_SCENARIO_CONFIGURATION, &CTLiteDoc::OnScenarioConfiguration)
	ON_COMMAND(ID_MOE_VIEWMOES, &CTLiteDoc::OnMoeViewmoes)
	ON_COMMAND(ID_IMPORT_SINGLEEXCELFILE, &CTLiteDoc::OnImportdataImport)
	ON_COMMAND(ID_MOE_VEHICLEPATHANALAYSIS, &CTLiteDoc::OnMoeVehiclepathanalaysis)
	ON_COMMAND(ID_FILE_CONSTRUCTANDEXPORTSIGNALDATA, &CTLiteDoc::OnFileConstructandexportsignaldata)
	ON_COMMAND(ID_FILE_IMPORT_DEMAND_FROM_CSV, &CTLiteDoc::OnFileImportDemandFromCsv)
	ON_COMMAND(ID_IMPORT_SENSOR_DATA, &CTLiteDoc::OnImportSensorData)
	ON_COMMAND(ID_IMPORT_LINKMOE, &CTLiteDoc::OnImportLinkmoe)
	ON_COMMAND(ID_IMPORT_VEHICLEFILE, &CTLiteDoc::OnImportVehiclefile)
	ON_COMMAND(ID_LINKMOE_EMISSIONS, &CTLiteDoc::OnLinkmoeEmissions)
	ON_UPDATE_COMMAND_UI(ID_LINKMOE_EMISSIONS, &CTLiteDoc::OnUpdateLinkmoeEmissions)
	ON_COMMAND(ID_LINKMOE_RELIABILITY, &CTLiteDoc::OnLinkmoeReliability)
	ON_UPDATE_COMMAND_UI(ID_LINKMOE_RELIABILITY, &CTLiteDoc::OnUpdateLinkmoeReliability)
	ON_COMMAND(ID_LINKMOE_SAFETY, &CTLiteDoc::OnLinkmoeSafety)
	ON_UPDATE_COMMAND_UI(ID_LINKMOE_SAFETY, &CTLiteDoc::OnUpdateLinkmoeSafety)
	ON_COMMAND(ID_IMPORT_AGENT_FILE, &CTLiteDoc::OnImportAgentFile)
	ON_COMMAND(ID_IMPORT_NGSIM_FILE, &CTLiteDoc::OnImportNgsimFile)
	ON_COMMAND(ID_EXPORT_AMS, &CTLiteDoc::OnExportAms)
	ON_COMMAND(ID_IMPORT_AVI, &CTLiteDoc::OnImportAvi)
	ON_COMMAND(ID_IMPORT_GPS33185, &CTLiteDoc::OnImportGps33185)
	ON_COMMAND(ID_IMPORT_VII, &CTLiteDoc::OnImportVii)
	ON_COMMAND(ID_IMPORT_WEATHER33188, &CTLiteDoc::OnImportWeather33188)
	ON_COMMAND(ID_IMPORT_GPS, &CTLiteDoc::OnImportGps)
	ON_COMMAND(ID_IMPORT_WORKZONE, &CTLiteDoc::OnImportWorkzone)
	ON_COMMAND(ID_IMPORT_INCIDENT, &CTLiteDoc::OnImportIncident)
	ON_COMMAND(ID_IMPORT_WEATHER, &CTLiteDoc::OnImportWeather)
	ON_COMMAND(ID_IMPORT_PRICING, &CTLiteDoc::OnImportPricing)
	ON_COMMAND(ID_IMPORT_ATIS, &CTLiteDoc::OnImportAtis)
	ON_COMMAND(ID_IMPORT_BUS, &CTLiteDoc::OnImportBus)
	ON_COMMAND(ID_LINK_ADD_WORKZONE, &CTLiteDoc::OnLinkAddWorkzone)
	ON_COMMAND(ID_LINK_ADDVMS, &CTLiteDoc::OnLinkAddvms)
	ON_COMMAND(ID_IMPORT_LINKLAYERINKML, &CTLiteDoc::OnImportLinklayerinkml)
	ON_COMMAND(ID_EDIT_OFFSETLINKS, &CTLiteDoc::OnEditOffsetlinks)
	ON_COMMAND(ID_IMPORT_SUBAREALAYERFORMAPMATCHING, &CTLiteDoc::OnImportSubarealayerformapmatching)
	ON_COMMAND(ID_LINK_ADDLINK, &CTLiteDoc::OnLinkAddlink)
	ON_COMMAND(ID_LINK_ADDHOVTOLL, &CTLiteDoc::OnLinkAddhovtoll)
	ON_COMMAND(ID_LINK_ADDHOTTOLL, &CTLiteDoc::OnLinkAddhottoll)
	ON_COMMAND(ID_LINK_ADDTOLLEDEXPRESSLANE, &CTLiteDoc::OnLinkAddtolledexpresslane)
	ON_COMMAND(ID_LINK_CONVERTGENERALPURPOSELANETOTOLLEDLANE, &CTLiteDoc::OnLinkConvertgeneralpurposelanetotolledlane)
	ON_COMMAND(ID_PROJECT_EDITTIME, &CTLiteDoc::OnProjectEdittime)
	ON_COMMAND(ID_LINK_VEHICLESTATISTICSANALAYSIS, &CTLiteDoc::OnLinkVehiclestatisticsanalaysis)
	ON_COMMAND(ID_SUBAREA_DELETESUBAREA, &CTLiteDoc::OnSubareaDeletesubarea)
	ON_COMMAND(ID_SUBAREA_VIEWVEHICLESTATISTICSASSOCIATEDWITHSUBAREA, &CTLiteDoc::OnSubareaViewvehiclestatisticsassociatedwithsubarea)
	ON_COMMAND(ID_TOOLS_TRAVELTIMERELIABILITYANALYSIS, &CTLiteDoc::OnToolsTraveltimereliabilityanalysis)
	ON_COMMAND(ID_LINK_LINKBAR, &CTLiteDoc::OnLinkLinkbar)
	ON_COMMAND(ID_IMPORT_ARCGISSHAPEFILE, &CTLiteDoc::OnImportArcgisshapefile)
	ON_COMMAND(ID_LINK_INCREASEOFFSETFORTWO, &CTLiteDoc::OnLinkIncreaseoffsetfortwo)
	ON_COMMAND(ID_LINK_DECREASEOFFSETFORTWO, &CTLiteDoc::OnLinkDecreaseoffsetfortwo)
	ON_COMMAND(ID_LINK_NOOFFSETANDNOBANDWIDTH, &CTLiteDoc::OnLinkNooffsetandnobandwidth)
	ON_COMMAND(ID_VIEW_SHOWHIDE_LEGEND, &CTLiteDoc::OnViewShowhideLegend)
	ON_COMMAND(ID_MOE_VIEWLINKMOESUMMARYFILE, &CTLiteDoc::OnMoeViewlinkmoesummaryfile)
	ON_COMMAND(ID_VIEW_CALIBRATIONVIEW, &CTLiteDoc::OnViewCalibrationview)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CALIBRATIONVIEW, &CTLiteDoc::OnUpdateViewCalibrationview)
	ON_COMMAND(ID_MOE_VIEWTRAFFICASSIGNMENTSUMMARYPLOT, &CTLiteDoc::OnMoeViewtrafficassignmentsummaryplot)
	ON_COMMAND(ID_MOE_VIEWODDEMANDESTIMATIONSUMMARYPLOT, &CTLiteDoc::OnMoeViewoddemandestimationsummaryplot)
	ON_COMMAND(ID_PROJECT_EDITPRICINGSCENARIODATA, &CTLiteDoc::OnProjectEditpricingscenariodata)
	ON_COMMAND(ID_LINK_VIEWLINK, &CTLiteDoc::OnLinkViewlink)
	ON_COMMAND(ID_DELETE_SELECTED_LINK, &CTLiteDoc::OnDeleteSelectedLink)
	ON_COMMAND(ID_IMPORT_REGIONALPLANNINGANDDTAMODELS, &CTLiteDoc::OnImportRegionalplanninganddtamodels)
	ON_COMMAND(ID_EXPORT_GENERATEZONE, &CTLiteDoc::OnExportGenerateTravelTimeMatrix)
	ON_COMMAND(ID_EXPORT_GENERATESHAPEFILES, &CTLiteDoc::OnExportGenerateshapefiles)
	ON_COMMAND(ID_LINKMOEDISPLAY_QUEUELENGTH, &CTLiteDoc::OnLinkmoedisplayQueuelength)
	ON_UPDATE_COMMAND_UI(ID_LINKMOEDISPLAY_QUEUELENGTH, &CTLiteDoc::OnUpdateLinkmoedisplayQueuelength)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_TRAVELTIMERELIABILITYANALYSIS, &CTLiteDoc::OnUpdateLinkmoeTraveltimereliability)

	ON_COMMAND(ID_MOE_PATHLIST, &CTLiteDoc::OnMoePathlist)
	ON_COMMAND(ID_VIEW_SHOWMOE, &CTLiteDoc::OnViewShowmoe)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWMOE, &CTLiteDoc::OnUpdateViewShowmoe)
	ON_COMMAND(ID_FILE_UPLOADLINKDATATOGOOGLEFUSIONTABLE, &CTLiteDoc::OnFileUploadlinkdatatogooglefusiontable)
	ON_COMMAND(ID_3_VIEWDATAINEXCEL, &CTLiteDoc::On3Viewdatainexcel)
	ON_COMMAND(ID_5_VIEWDATAINEXCEL, &CTLiteDoc::On5Viewdatainexcel)
	ON_COMMAND(ID_MOE_VIEWNETWORKTIMEDEPENDENTMOE, &CTLiteDoc::OnMoeViewnetworktimedependentmoe)
	ON_COMMAND(ID_2_VIEWDATAINEXCEL33398, &CTLiteDoc::On2Viewdatainexcel33398)
	ON_COMMAND(ID_2_VIEWNETWORKDATA, &CTLiteDoc::On2Viewnetworkdata)
	ON_COMMAND(ID_3_VIEWODDATAINEXCEL, &CTLiteDoc::On3Viewoddatainexcel)
	ON_COMMAND(ID_MOE_OPENALLMOETABLES, &CTLiteDoc::OnMoeOpenallmoetables)
	ON_COMMAND(ID_FILE_OPEN_NEW_RAIL_DOC, &CTLiteDoc::OnFileOpenNewRailDoc)
	ON_BN_CLICKED(IDC_BUTTON_Database, &CTLiteDoc::OnBnClickedButtonDatabase)
	ON_COMMAND(ID_TOOLS_UNITTESTING, &CTLiteDoc::OnToolsUnittesting)
	ON_COMMAND(ID_VIEW_TRAININFO, &CTLiteDoc::OnViewTraininfo)
	ON_COMMAND(ID_IMPORT_AMSDATASET, &CTLiteDoc::OnImportAmsdataset)
	ON_COMMAND(ID_DEMANDFILE_ODDEMANDMATRIX, &CTLiteDoc::OnDemandfileOddemandmatrix)
	ON_COMMAND(ID_DEMANDFILE_HOVODDEMANDMATRIX, &CTLiteDoc::OnDemandfileHovoddemandmatrix)
	ON_COMMAND(ID_DEMANDFILE_TRUCKODDEMANDMATRIX, &CTLiteDoc::OnDemandfileTruckoddemandmatrix)
	ON_COMMAND(ID_DEMANDFILE_INTERMODALODDEMANDMATRIX, &CTLiteDoc::OnDemandfileIntermodaloddemandmatrix)
	ON_COMMAND(ID_LINK_ADDINCIDENT, &CTLiteDoc::OnLinkAddIncident)
	ON_COMMAND(ID_TOOLS_GENERATEPHYSICALZONECENTROIDSONROADNETWORK, &CTLiteDoc::OnToolsGeneratephysicalzonecentroidsonroadnetwork)
	ON_COMMAND(ID_IMPORT_DEMANDDATASET, &CTLiteDoc::OnImportDemanddataset)
	ON_COMMAND(ID_NODE_INCREASENODETEXTSIZE, &CTLiteDoc::OnNodeIncreasenodetextsize)
	ON_COMMAND(ID_NODE_DECREASENODETEXTSIZE, &CTLiteDoc::OnNodeDecreasenodetextsize)
	ON_COMMAND(ID_TOOLS_CHECKINGFEASIBILITY, &CTLiteDoc::OnToolsCheckingfeasibility)
	ON_COMMAND(ID_TOOLS_GPSMAPMATCHING, &CTLiteDoc::OnToolsGpsmapmatching)
	ON_COMMAND(ID_IMPORT_SYNCHROUTDFCSVFILES, &CTLiteDoc::OnImportSynchroutdfcsvfiles)
	ON_COMMAND(ID_PROJECT_EDITMOESETTINGS, &CTLiteDoc::OnProjectEditmoesettings)
	ON_COMMAND(ID_PROJECT_Multi_Scenario_Results, &CTLiteDoc::OnProjectMultiScenarioResults)
	ON_COMMAND(ID_PROJECT_12, &CTLiteDoc::OnProject12)
	ON_COMMAND(ID_VIEW_MOVEMENT_MOE, &CTLiteDoc::OnViewMovementMoe)
	ON_COMMAND(ID_PROJECT_TIME_DEPENDENT_LINK_MOE, &CTLiteDoc::OnProjectTimeDependentLinkMoe)
	ON_COMMAND(ID_VIEW_ODME_Result, &CTLiteDoc::OnViewOdmeResult)
	ON_COMMAND(ID_PROJECT_VIEW_AGENT_MOE, &CTLiteDoc::OnProjectViewAgentMoe)
	ON_COMMAND(ID_PROJECT_ODMATRIXESTIMATIONINPUT, &CTLiteDoc::OnProjectOdmatrixestimationinput)
	ON_COMMAND(ID_PROJECT_INPUTSENSORDATAFORODME, &CTLiteDoc::OnProjectInputsensordataforodme)
	ON_COMMAND(ID_HELP_USERGUIDE, &CTLiteDoc::OnHelpUserguide)
	ON_COMMAND(ID_TOOLS_GENERATEODMATRIXGRAVITYMODEL, &CTLiteDoc::OnToolsGenerateodmatrixgravitymodel)
	ON_COMMAND(ID_LINKATTRIBUTEDISPLAY_LINKNAME, &CTLiteDoc::OnLinkattributedisplayLinkname)
	ON_UPDATE_COMMAND_UI(ID_LINKATTRIBUTEDISPLAY_LINKNAME, &CTLiteDoc::OnUpdateLinkattributedisplayLinkname)
	ON_COMMAND(ID_TOOLS_GENERATESIGNALCONTROLLOCATIONS, &CTLiteDoc::OnToolsGeneratesignalcontrollocations)
	ON_COMMAND(ID_AssigngmentSimulationSettings, &CTLiteDoc::OnAssignmentSimulatinSettinsClicked)
	ON_COMMAND(ID_PROJECT_1_NETWORK, &CTLiteDoc::OnProjectNetworkData)
	ON_COMMAND(ID_LINK_ADDSENSOR, &CTLiteDoc::OnLinkAddsensor)
	ON_COMMAND(ID_IMPORT_SYNCHROCOMBINEDCSVFILE, &CTLiteDoc::OnImportSynchrocombinedcsvfile)
	ON_COMMAND(ID_TOOLS_OBTAINCYCLELENGTHFROM, &CTLiteDoc::OnToolsObtainCyclelengthfromNearbySignals)
	ON_COMMAND(ID_TOOLS_SAVEPROJECTFOREXTERNALLAYER, &CTLiteDoc::OnToolsSaveprojectforexternallayer)
	ON_COMMAND(ID_TOOLS_UPDATEEFFECTIVEGREENTIMEBASEDONCYCLELENGTH, &CTLiteDoc::OnToolsUpdateeffectivegreentimebasedoncyclelength)
	ON_COMMAND(ID_MOE_MOE, &CTLiteDoc::OnMoeTableDialog)
	ON_COMMAND(ID_TOOLS_REVERSEVERTICALCOORDINATE, &CTLiteDoc::OnToolsReverseverticalcoordinate)
	ON_COMMAND(ID_GENERATEGISSHAPEFILES_LOADLINKCSVFILE, &CTLiteDoc::OnGenerategisshapefilesLoadlinkcsvfile)
	ON_COMMAND(ID_SAFETYPLANNINGTOOLS_RUN, &CTLiteDoc::OnSafetyplanningtoolsRun)
	ON_COMMAND(ID_SAFETYPLANNINGTOOLS_GENERATENODE, &CTLiteDoc::OnSafetyplanningtoolsGeneratenode)
	ON_COMMAND(ID_SENSORTOOLS_CONVERTTOHOURCOUNT, &CTLiteDoc::OnSensortoolsConverttoHourlyVolume)
	ON_COMMAND(ID_IMPORT_INRIXSHAPEFILEANDSPEEDDATA, &CTLiteDoc::OnImportInrixshapefileandspeeddata)
	ON_COMMAND(ID_TRAFFICCONTROLTOOLS_TRANSFERMOVEMENTDATAFROMREFERENCENETWORKTOCURRENTNETWORK, &CTLiteDoc::OnTrafficcontroltoolsTransfermovementdatafromreferencenetworktocurrentnetwork)
	ON_COMMAND(ID_DEMANDTOOLS_GENERATEINPUT, &CTLiteDoc::OnDemandtoolsGenerateinput)
	ON_COMMAND(ID_DEMAND_RECONSTRUCTLINKMOETH, &CTLiteDoc::OnDemandReconstructlinkmoeth)
	ON_COMMAND(ID_NETWORKTOOLS_RESETLINKLENGTH, &CTLiteDoc::OnNetworktoolsResetlinklength)
	ON_COMMAND(ID_SUBAREA_CREATEZONEFROMSUBAREA, &CTLiteDoc::OnSubareaCreatezonefromsubarea)
	ON_COMMAND(ID_DEMAND_REGENERATEACTIVITYLOCATIONS, &CTLiteDoc::OnDemandRegenerateactivitylocations)
	ON_COMMAND(ID_DEMAND_CONVERT, &CTLiteDoc::OnDemandConvert)
	ON_COMMAND(ID_TRAFFICCONTROLTOOLS_TRANSFERSIGNALDATAFROMREFERENCENETWORKTOCURRENTNETWORK, &CTLiteDoc::OnTrafficcontroltoolsTransfersignaldatafromreferencenetworktocurrentnetwork)
	END_MESSAGE_MAP()


// CTLiteDoc construction/destruction

CTLiteDoc::CTLiteDoc()
{
	m_ImportDemandColumnFormat = true;
	m_bSummaryDialog = false;
	m_StartNodeNumberForNewNodes = 1;
	m_agent_demand_input_mode = 0;
	m_ODME_mode = 0;
	m_demand_multiplier = 1;

	m_number_of_assignment_days = 20;
	m_traffic_flow_model = 3;
	m_traffic_assignment_method = 1;

	m_ActivityLocationCount = 0;
	m_SearchMode= efind_node;
	m_LongLatFlag = false; // default, we do not know if the coordinate system is long or lat
	m_bUnitMileInitialized = false;
	g_bValidDocumentChanged = true;

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	m_DefaultDataFolder.Format ("%s\\default_data_folder\\",pMainFrame->m_CurrentDirectory);

	m_ZoneNoSize  = 0;
	m_bRunCrashPredictionModel = false;
	m_ZoomToSelectedObject = true;
	m_max_walking_distance = 0.5;
	m_max_accessible_transit_time_in_min = 15;

	m_DemandTypeSize = 1; // just one demand trip type:
	m_ODMOEMatrix = NULL;

	m_bMovementAvailableFlag = false;

	m_ImportNetworkAlready = false;
	m_DemandLoadingStartTimeInMin = 420;
	m_DemandLoadingEndTimeInMin = 480;

	m_bSaveProjectFromSubareaCut = false;
	m_bExport_Link_MOE_in_input_link_CSF_File = false;
	m_bBezierCurveFlag = true;

	m_CriticalOriginZone = -1;
	m_CriticalDestinationZone = -1;

	m_ControlType_UnknownControl = 0;
	m_ControlType_NoControl = 1;
	m_ControlType_YieldSign = 2;
	m_ControlType_2wayStopSign = 3;
	m_ControlType_4wayStopSign = 4;
	m_ControlType_PretimedSignal = 5;
	m_ControlType_ActuatedSignal = 6;
	m_ControlType_Roundabout = 7;
	m_ControlType_ExternalNode = 100;


	m_LinkTypeMap[1].link_type = 1;
	m_LinkTypeMap[1].link_type_name = "Freeway";
	m_LinkTypeMap[1].type_code = "f";

	m_LinkTypeMap[2].link_type = 2;
	m_LinkTypeMap[2].link_type_name = "Highway";
	m_LinkTypeMap[2].type_code = "h";

	m_LinkTypeMap[3].link_type = 3;
	m_LinkTypeMap[3].link_type_name = "Principal arterial";
	m_LinkTypeMap[3].type_code = "a";

	m_LinkTypeMap[4].link_type = 4;
	m_LinkTypeMap[4].link_type_name = "Major arterial";
	m_LinkTypeMap[4].type_code = "a";

	m_LinkTypeMap[5].link_type = 5;
	m_LinkTypeMap[5].link_type_name = "Minor arterial";
	m_LinkTypeMap[5].type_code = "a";

	m_LinkTypeMap[6].link_type = 6;
	m_LinkTypeMap[6].link_type_name = "Collector";
	m_LinkTypeMap[6].type_code = "a";

	m_LinkTypeMap[7].link_type = 7;
	m_LinkTypeMap[7].link_type_name = "Local";
	m_LinkTypeMap[7].type_code = "a";

	m_LinkTypeMap[8].link_type = 8;
	m_LinkTypeMap[8].link_type_name = "Frontage road";
	m_LinkTypeMap[8].type_code = "a";

	m_LinkTypeMap[9].link_type = 9;
	m_LinkTypeMap[9].link_type_name = "Ramp";
	m_LinkTypeMap[9].type_code = "r";

	m_LinkTypeMap[10].link_type = 10;
	m_LinkTypeMap[10].link_type_name = "Zonal connector";
	m_LinkTypeMap[10].type_code = "c";

	m_LinkTypeMap[100].link_type = 100;
	m_LinkTypeMap[100].link_type_name = "Transit link";
	m_LinkTypeMap[100].type_code = "t";

	m_LinkTypeMap[200].link_type = 200;
	m_LinkTypeMap[200].link_type_name = "Walking link";
	m_LinkTypeMap[200].type_code = "w";

	m_LinkTypeFreeway = 1;
	m_LinkTypeArterial = 3;
	m_LinkTypeHighway = 2;


	// blue style
	m_KML_style_map["green"] = "7d00ff00";
	m_KML_style_map["red"] = "7d0000ff";
	m_KML_style_map["blue"] = "7dff0000";
	m_KML_style_map["yellow"] = "7d00ffff";


	m_OriginOnBottomFlag = 1;

	m_DocumentNo = g_DocumentList.size();
	g_DocumentList.push_back (this);

	m_RandomRoutingCoefficient = 0.0f;
	m_bDYNASMARTDataSet = false;
	m_bGPSDataSet = false;
	m_bEmissionDataAvailable = false;
	m_AdjLinkSize = 20;  // initial value
	m_colorLOS[0] = RGB(190,190,190);
	m_colorLOS[1] = RGB(0,255,0);
	m_colorLOS[2] = RGB(255,250,117);
	m_colorLOS[3] = RGB(255,250,0);
	m_colorLOS[4] = RGB(255,216,0);
	m_colorLOS[5] = RGB(255,153,0);
	m_colorLOS[6] = RGB(255,0,0);

	m_FreewayColor = RGB(030,144,255);
	m_RampColor = RGB(160,032,240); 
	m_ArterialColor = RGB(034,139,034);
	m_ConnectorColor = RGB(255,165,000);
	m_TransitColor = RGB(255,0,255);
	m_WalkingColor = RGB(127,255,0);

	m_MaxLinkWidthAsNumberOfLanes = 5;
	m_MaxLinkWidthAsLinkVolume = 1000;

	m_VehicleSelectionMode = CLS_network;
	m_LinkBandWidthMode = LBW_number_of_lanes;
	m_bLoadNetworkDataOnly = false;
	m_SamplingTimeInterval = 1;
	m_AVISamplingTimeInterval = 5;

	m_bSimulationDataLoaded  = false;
	m_EmissionDataFlag = false;
	m_bLinkToBeShifted = true;
	m_SimulationStartTime_in_min = 0;  // 6 AM
	m_SimulationEndTime_in_min = 1440;

	m_NumberOfDays = 0;
	m_LinkMOEMode = MOE_none;
	m_PrevLinkMOEMode = MOE_fftt;  // make sure the document gets a change to initialize the display view as the first mode is MOE_none
	m_ODMOEMode = odnone;

	MaxNodeKey = 60000;  // max: unsigned short 65,535;
	m_BackgroundBitmapLoaded  = false;
	m_LongLatCoordinateFlag = false;
	m_ColorFreeway = RGB(198,226,255);
	m_ColorHighway = RGB(100,149,237);
	m_ColorArterial = RGB(0,0,0);
	m_pNetwork = NULL;
	m_OriginNodeID = -1;

	m_DestinationNodeID = -1;
	m_NodeSizeSP = 0;


	m_PathMOEDlgShowFlag = false;
	m_SelectPathNo = -1;

	m_ImageX1 = 0;
	m_ImageX2 = 1000;
	m_ImageY1 = 0;
	m_ImageY2 = 1000;

	m_ZoneColor = RGB(000,191,255);
	m_ZoneTextColor = RGB(0,191,255);

	if(theApp.m_VisulizationTemplate == e_traffic_assignment)
	{
		m_NodeDisplaySize = 50;  // in feet
		m_VehicleDisplaySize = 10; // in feet
		m_BackgroundColor =  RGB(255,255,255);  //white
		m_NodeTextDisplayRatio = 4;

	}

	if(theApp.m_VisulizationTemplate == e_train_scheduling)
	{
		m_NodeDisplaySize = 2000;  // in feet
		m_VehicleDisplaySize= 1000; // in feet
		m_BackgroundColor =  RGB(255,255,255);
		m_NodeTextDisplayRatio = 8;
	}



	m_NetworkRect.top  = 50;
	m_NetworkRect.bottom = 0;

	m_NetworkRect.left   = 0;
	m_NetworkRect.right = 100;

	m_UnitMile = 1;
	m_UnitFeet = 1/5280.0;
	m_OffsetInFeet = 0;
	m_LaneWidthInFeet = 10;
	m_bFitNetworkInitialized = false; 
	
	m_DefaultNumLanes = 3;
	m_DefaultSpeedLimit = 65.0f;
	m_DefaultCapacity = 1900.0f;
	m_DefaultLinkType = 1;

	m_ODSize = 0;
	m_SelectedLinkNo = -1;
	m_SelectedNodeID = -1;
	m_SelectedVehicleID = -1;

	m_bSetView = false;
	m_bShowLegend = false;
	m_bShowPathList = false;

	for(int i=0; i<40;i++)
	{
		for(int los= 0; los < MAX_LOS_SIZE; los++)
		{
			m_LOSBound[i][los] = 0;
		}
	}

	// speed LOS bound
	m_LOSBound[MOE_speed][1] = 100;
	m_LOSBound[MOE_speed][2] = 80;
	m_LOSBound[MOE_speed][3] = 65;
	m_LOSBound[MOE_speed][4] = 50;
	m_LOSBound[MOE_speed][5] = 40;
	m_LOSBound[MOE_speed][6] = 33;
	m_LOSBound[MOE_speed][7] = 0;

	m_LOSBound[MOE_reliability][1] = 0;
	m_LOSBound[MOE_reliability][2] = 0.1f;
	m_LOSBound[MOE_reliability][3] = 0.2f;
	m_LOSBound[MOE_reliability][4] = 0.3f;
	m_LOSBound[MOE_reliability][5] = 0.5f;
	m_LOSBound[MOE_reliability][6] = 0.7f;
	m_LOSBound[MOE_reliability][7] = 999;

	m_LOSBound[MOE_density][1] = 0;
	m_LOSBound[MOE_density][2] = 11;
	m_LOSBound[MOE_density][3] = 17;
	m_LOSBound[MOE_density][4] = 25;
	m_LOSBound[MOE_density][5] = 35;
	m_LOSBound[MOE_density][6] = 45;
	m_LOSBound[MOE_density][7] = 999;


	m_TrafficFlowModelFlag = 1;  // static traffic assignment as default
	m_Doc_Resolution = 1;
	m_bShowCalibrationResults = false;

	m_SampleExcelNetworkFile = "\\importing_sample_data_sets\\sample_data_set.xls";
	//	m_SampleExcelSensorFile = "\\Sample_Import_Excel_Files\\input_Portland_sensor_data.xls";

}

static bool DeleteLinkPointer( DTALink * theElement ) { delete theElement; return true; }

CTLiteDoc::~CTLiteDoc()
{
	g_bValidDocumentChanged = true;
	CWaitCursor wait;
	if(m_ODMOEMatrix !=NULL)
		Deallocate3DDynamicArray<VehicleStatistics>(m_ODMOEMatrix,	m_PreviousDemandTypeSize, m_PreviousZoneNoSize);


	if(g_LinkMOEDlg!=NULL && g_LinkMOEDlg->m_pDoc == this)
	{
		delete g_LinkMOEDlg;
		g_LinkMOEDlg= NULL;

	}

	if(g_pPathMOEDlg!=NULL &&  g_pPathMOEDlg->m_pDoc == this)
	{
		delete g_pPathMOEDlg;
		g_pPathMOEDlg= NULL;

	}

	if(g_pLegendDlg!=NULL &&  g_pLegendDlg->m_pDoc == this)
	{
		delete g_pLegendDlg;
		g_pLegendDlg= NULL;

	}

	if(g_pLinkListDlg!=NULL && g_pLinkListDlg->m_pDoc == this)
	{
		delete g_pLinkListDlg;
		g_pLinkListDlg= NULL;

	}

	if(g_pVehiclePathDlg!=NULL && g_pVehiclePathDlg->m_pDoc == this)
	{
		delete g_pVehiclePathDlg;
		g_pVehiclePathDlg= NULL;

	}
	if(g_pPathListDlg!=NULL && g_pPathListDlg->m_pDoc == this)
	{
		delete g_pPathListDlg;
		g_pPathListDlg= NULL;

	}

	if(g_pUnitTestingListDlg!=NULL && g_pUnitTestingListDlg->m_pDoc == this)
	{
		delete g_pUnitTestingListDlg;
		g_pUnitTestingListDlg= NULL;

	}

	std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
	while (iDoc != g_DocumentList.end())
	{
		if((*iDoc) == this)
		{
			g_DocumentList.erase (iDoc);  // remove the document to be deleted
			break;

		}
		iDoc++;
	}

	m_WarningFile.close();

	if(m_pNetwork!=NULL)
		delete m_pNetwork;


	//	m_LinkSet.remove_if (DeleteLinkPointer);

	/*
	std::list<DTANode*>::iterator iNode;
	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
	DTANode* pNode = (*iNode);

	if(pNode!=NULL)
	delete pNode;

	}
	*/
}

BOOL CTLiteDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CTLiteDoc serialization

void CTLiteDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CTLiteDoc diagnostics

#ifdef _DEBUG
void CTLiteDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTLiteDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG



void CTLiteDoc::SetStatusText(CString StatusText)
{
	CTLiteView* pView = 0;
	POSITION pos = GetFirstViewPosition();
	if(pos != NULL)
	{
		pView = (CTLiteView*) GetNextView(pos);
	}

	if(pView!=NULL) pView->SetStatusText(StatusText);

}

void CTLiteDoc::ReadSimulationLinkMOEData_Parser(LPCTSTR lpszFileName)
{
	CCSVParser parser;
	int i= 0;
	if (parser.OpenCSVFile(lpszFileName))
	{

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);  // use one day horizon as the default value
		}

		while(parser.ReadRecord())
		{

			int from_node_number;
			if(parser.GetValueByFieldName("from_node_id",from_node_number) == false)
				break;
			int to_node_number ;

			if(parser.GetValueByFieldName("to_node_id",to_node_number) == false)
				break;

			int t;

			if(parser.GetValueByFieldName("timestamp_in_min",t) == false)
				break;

			DTALink* pLink = FindLinkWithNodeNumbers(from_node_number , to_node_number, lpszFileName );

			if(pLink!=NULL)
			{
				if(t < g_Simulation_Time_Horizon)
				{
					//travel_time_in_min, delay_in_min, link_volume_in_veh, link_volume_in_vehphpl,
					//density_in_veh_per_mile_per_lane, speed_in_mph, queue_length_in_, cumulative_arrival_count, cumulative_departure_count
					parser.GetValueByFieldName("travel_time_in_min",pLink->m_LinkMOEAry[t].SimulatedTravelTime);
					//				parser.GetValueByFieldName("delay_in_min",pLink->m_LinkMOEAry[t].SimulatedTravelTime);
					//parser.GetValueByFieldName("link_volume_in_veh_per_hour_per_lane",pLink->m_LinkMOEAry[t].SimulationLinkFlow);
					parser.GetValueByFieldName("link_volume_in_veh_per_hour_for_all_lanes",pLink->m_LinkMOEAry[t].SimulationLinkFlow);
					parser.GetValueByFieldName("density_in_veh_per_mile_per_lane",pLink->m_LinkMOEAry[t].SimulationDensity );
					parser.GetValueByFieldName("speed_in_mph",pLink->m_LinkMOEAry[t].SimulationSpeed);
					parser.GetValueByFieldName("exit_queue_length",pLink->m_LinkMOEAry[t].SimulationQueueLength );
					parser.GetValueByFieldName("cumulative_arrival_count",pLink->m_LinkMOEAry[t].SimuArrivalCumulativeFlow);

					//				parser.GetValueByFieldName("cumulative_departure_count",pLink->m_LinkMOEAry[t].SimulatedTravelTime);
				}
				i++;
			}else
			{
				CString msg;
				msg.Format ("Please check if link %d->%d at file %s still exists in input_link.csv.", from_node_number , to_node_number, lpszFileName);  // +2 for the first field name line
				AfxMessageBox(msg);
				break;
			}

		}

		m_bSimulationDataLoaded = true;

		g_Simulation_Time_Stamp = 0; // reset starting time
		g_SimulationStartTime_in_min = 0;

		m_SimulationLinkMOEDataLoadingStatus.Format ("%d link records are loaded from file %s.",i,lpszFileName);
	}
}

bool CTLiteDoc::ReadSimulationLinkMOEData_Bin(LPCTSTR lpszFileName)
{

	float max_engergy_per_mile = 0;

	typedef struct 
	{
		int from_node_id;
		int to_node_id;
		int timestamp_in_min;
		int travel_time_in_min;
		int delay_in_min;
		float link_volume_in_veh_per_hour_per_lane;
		float link_volume_in_veh_per_hour_for_all_lanes;
		float density_in_veh_per_mile_per_lane;
		float speed_in_mph;
		float exit_queue_length;
		int cumulative_arrival_count;
		int cumulative_departure_count;
		int cumulative_SOV_count;
		int cumulative_HOV_count;
		int cumulative_truck_count;
		int cumulative_intermodal_count;
		int cumulative_SOV_revenue;
		int cumulative_HOV_revenue;
		int cumulative_truck_revenue;
		int cumulative_intermodal_revenue;
		float Energy;
		float CO2;
		float NOX;
		float CO;
		float HC;

	} struct_TDMOE;

	int i= 0;
	FILE* pFile;


	fopen_s(&pFile,lpszFileName,"rb");
	if(pFile!=NULL)
	{
		m_SimulationStartTime_in_min = 1440;
		m_SimulationEndTime_in_min = 0;

		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);
		float LengthinMB= Length*1.0/1024/1024;
		if(LengthinMB>20)
		{
			CString msg;
			msg.Format("The time-dependent link MOE file is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load this file?",LengthinMB);
			if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
				return 1;
		}
		fclose(pFile);
	}

	/////////////////
	fopen_s(&pFile,lpszFileName,"rb");
	if(pFile!=NULL)
	{

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);  // use one day horizon as the default value
		}

		while(!feof(pFile))
		{
			struct_TDMOE element;
			size_t result = fread(&element,sizeof(element),1,pFile);
			if(result!= 1)
				break;

			int from_node_number = element.from_node_id;
			int to_node_number = element.to_node_id;

			int t = element.timestamp_in_min;

			if(t< m_SimulationStartTime_in_min)
			{
				m_SimulationStartTime_in_min = t; // reset simulation start time 

			}

			if(t >  m_SimulationEndTime_in_min)
			{
				m_SimulationEndTime_in_min = t; // reset simulation end time 

			}



			DTALink* pLink = FindLinkWithNodeNumbers(from_node_number , to_node_number, lpszFileName );

			if(pLink!=NULL)
			{	
				if(t < g_Simulation_Time_Horizon)
				{
					//travel_time_in_min, delay_in_min, link_volume_in_veh, link_volume_in_vehphpl,
					//density_in_veh_per_mile_per_lane, speed_in_mph, queue_length_in_, cumulative_arrival_count, cumulative_departure_count
					pLink->m_LinkMOEAry[t].SimulatedTravelTime = element.travel_time_in_min;
					//				parser.GetValueByFieldName("delay_in_min",pLink->m_LinkMOEAry[t].SimulatedTravelTime);
					//parser.GetValueByFieldName("link_volume_in_veh_per_hour_per_lane",pLink->m_LinkMOEAry[t].SimulationLinkFlow);
					pLink->m_LinkMOEAry[t].SimulationLinkFlow = element.link_volume_in_veh_per_hour_for_all_lanes;
					pLink->m_LinkMOEAry[t].SimulationDensity  = element.density_in_veh_per_mile_per_lane;


					if(pLink->m_FromNodeNumber == 60306 && pLink->m_ToNodeNumber == 54256)
					{
						TRACE("%d, density = %f\n",t, element.density_in_veh_per_mile_per_lane);
					}

					pLink->m_LinkMOEAry[t].SimulationSpeed = element.speed_in_mph;
					pLink->m_LinkMOEAry[t].SimulationQueueLength = element.exit_queue_length;

					pLink->m_LinkMOEAry[t].SimuArrivalCumulativeFlow = element.cumulative_arrival_count;
					pLink->m_LinkMOEAry[t].SimuDepartureCumulativeFlow = element.cumulative_departure_count;

					float per_min_in_flow_count = max(1,element.link_volume_in_veh_per_hour_for_all_lanes/60);
					pLink->m_LinkMOEAry[t].Energy = element.Energy/per_min_in_flow_count; /*  *60   to convert it to per min data */
					pLink->m_LinkMOEAry[t].CO2 = element.CO2*60/per_min_in_flow_count;
					pLink->m_LinkMOEAry[t].NOX = element.NOX*60/per_min_in_flow_count;
					pLink->m_LinkMOEAry[t].CO = element.CO*60/per_min_in_flow_count;
					pLink->m_LinkMOEAry[t].HC = element.HC*60/per_min_in_flow_count;

					if(max_engergy_per_mile < pLink->m_LinkMOEAry[t].Energy )
					{
						max_engergy_per_mile = pLink->m_LinkMOEAry[t].Energy;
					}

					//				parser.GetValueByFieldName("cumulative_departure_count",pLink->m_LinkMOEAry[t].SimulatedTravelTime);
				}
				i++;
			}else
			{
				CString msg;
				msg.Format ("Please check if link %d->%d at file %s still exists in input_link.csv.", from_node_number , to_node_number, lpszFileName);  // +2 for the first field name line
				AfxMessageBox(msg);
				break;
			}

		}

		m_LOSBound[MOE_emissions][1] = max_engergy_per_mile*1/7.0;
		m_LOSBound[MOE_emissions][2] =  max_engergy_per_mile*2/7.0;
		m_LOSBound[MOE_emissions][3] = max_engergy_per_mile*3/7.0;;
		m_LOSBound[MOE_emissions][4] = max_engergy_per_mile*4/7.0;;
		m_LOSBound[MOE_emissions][5] = max_engergy_per_mile*5/7.0;;
		m_LOSBound[MOE_emissions][6] = max_engergy_per_mile*6/7.0;;
		m_LOSBound[MOE_emissions][7] =  max_engergy_per_mile*7/7.0;


		fclose(pFile);

		m_bSimulationDataLoaded = true;

		g_Simulation_Time_Stamp = 0; // reset starting time
		g_SimulationStartTime_in_min = 0;

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->SetDefaultCumulativeFlow();  // set default value for cumulative flow count
		}

		m_SimulationLinkMOEDataLoadingStatus.Format ("%d link records are loaded from file %s.",i,lpszFileName);
		return 1;
	}
	return 0;
}
void CTLiteDoc::ReadSimulationLinkMOEData(LPCTSTR lpszFileName)
{

	FILE* st;
	fopen_s(&st,lpszFileName, "r");
	if (st!=NULL)
	{
		char  str_line[2000]; // input string
		int str_line_size;
		g_read_a_line(st,str_line, str_line_size); //  skip the first line

		int i= 0;

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);  // use one day horizon as the default value
		}
		int from_node_number;
		int to_node_number ;
		int t;
		float travel_time_in_min;
		float delay_in_min;
		float link_volume_in_veh_per_hour_per_lane;
		float link_volume_in_veh_per_hour_for_all_lanes;
		float density_in_veh_per_mile_per_lane;
		float speed_in_mph;
		int exit_queue_length;
		int cumulative_arrival_count;
		int cumulative_departure_count;
		int cumulative_SOV_count;
		int cumulative_HOV_count;
		int cumulative_truck_count;
		float cumulative_SOV_revenue;
		float cumulative_HOV_revenue;
		float cumulative_truck_revenue;

		while(fscanf_s(st,"%d,%d,%d,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d,%d,%f,%f,%f,", 
			&from_node_number,
			&to_node_number,
			&t,
			&travel_time_in_min,
			&delay_in_min,
			&link_volume_in_veh_per_hour_per_lane,
			&link_volume_in_veh_per_hour_for_all_lanes,
			&density_in_veh_per_mile_per_lane,
			&speed_in_mph,
			&exit_queue_length,
			&cumulative_arrival_count,
			&cumulative_departure_count,
			&cumulative_SOV_count,
			&cumulative_HOV_count,
			&cumulative_truck_count,
			&cumulative_SOV_revenue,
			&cumulative_HOV_revenue,
			&cumulative_truck_revenue)>0)
		{

			DTALink* pLink = FindLinkWithNodeNumbers(from_node_number , to_node_number, lpszFileName );

			if(pLink!=NULL)
			{
				if(t < g_Simulation_Time_Horizon)
				{
					//travel_time_in_min, delay_in_min, link_volume_in_veh, link_volume_in_vehphpl,
					//density_in_veh_per_mile_per_lane, speed_in_mph, queue_length_in_, cumulative_arrival_count, cumulative_departure_count
					pLink->m_LinkMOEAry[t].SimulatedTravelTime = travel_time_in_min;
					//				parser.GetValueByFieldName("delay_in_min",pLink->m_LinkMOEAry[t].SimulatedTravelTime);
					//parser.GetValueByFieldName("link_volume_in_veh_per_hour_per_lane",pLink->m_LinkMOEAry[t].SimulationLinkFlow);
					pLink->m_LinkMOEAry[t].SimulationLinkFlow = link_volume_in_veh_per_hour_for_all_lanes;
					pLink->m_LinkMOEAry[t].SimulationDensity = density_in_veh_per_mile_per_lane;
					pLink->m_LinkMOEAry[t].SimulationSpeed = speed_in_mph;
					pLink->m_LinkMOEAry[t].SimulationQueueLength = exit_queue_length;
					pLink->m_LinkMOEAry[t].SimuArrivalCumulativeFlow = cumulative_arrival_count;

					//				parser.GetValueByFieldName("cumulative_departure_count",pLink->m_LinkMOEAry[t].SimulatedTravelTime);
				}
				i++;
			}else
			{
				CString msg;
				msg.Format ("Please check if link %d->%d at file %s still exists in input_link.csv.", from_node_number , to_node_number, lpszFileName);  // +2 for the first field name line
				AfxMessageBox(msg);
				break;
			}

		}

		m_bSimulationDataLoaded = true;

		g_Simulation_Time_Stamp = 0; // reset starting time
		g_SimulationStartTime_in_min = 0;

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->SetDefaultCumulativeFlow();  // set default value for cumulative flow count
		}

		m_SimulationLinkMOEDataLoadingStatus.Format ("%d link records are loaded from file %s.",i,lpszFileName);

		fclose(st);
	}else
	{
		//		AfxMessageBox("Error: File output_LinkMOE.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return;

	}
}

void CTLiteDoc::ReadSimulationLinkOvarvallMOEData(LPCTSTR lpszFileName)
{
	double total_number_of_crashes = 0;

	CCSVParser parser;
	int i= 0;
	if (parser.OpenCSVFile(lpszFileName))
	{

		while(parser.ReadRecord())
		{

			int from_node_number;
			if(parser.GetValueByFieldName("from_node_id",from_node_number) == false)
				break;
			int to_node_number ;

			if(parser.GetValueByFieldName("to_node_id",to_node_number) == false)
				break;



			DTALink* pLink = FindLinkWithNodeNumbers(from_node_number , to_node_number, lpszFileName );

			if(pLink!=NULL)
			{

				parser.GetValueByFieldName("speed_in_mph",pLink->m_avg_simulated_speed );
				parser.GetValueByFieldName("total_link_volume",pLink->m_total_link_volume );
				parser.GetValueByFieldName("volume_over_capacity_ratio",pLink->m_volume_over_capacity_ratio );
				parser.GetValueByFieldName("level_of_service",pLink->m_LevelOfService);

				parser.GetValueByFieldName("avg_waiting_time_on_loading_buffer_(min)",pLink->m_avg_waiting_time_on_loading_buffer);
				parser.GetValueByFieldName("sensor_link_volume",pLink->m_total_sensor_link_volume);
				parser.GetValueByFieldName("measurement_error",pLink->m_total_link_count_error);
				//				parser.GetValueByFieldName("simulated_AADT",pLink->m_simulated_AADT);

				// we have to re-calculate this value as the users might change the AADT conversion factor in input_link.csv
				pLink->m_simulated_AADT = pLink->m_total_link_volume / max(0.01,pLink->m_AADT_conversion_factor);

				parser.GetValueByFieldName("num_of_crashes_per_year",pLink->m_number_of_all_crashes);
				parser.GetValueByFieldName("num_of_fatal_and_injury_crashes_per_year",pLink->m_num_of_fatal_and_injury_crashes_per_year);
				parser.GetValueByFieldName("num_of_PDO_crashes_per_year",pLink->m_num_of_PDO_crashes_per_year);

				parser.GetValueByFieldName("num_of_intersection_crashes_per_year",pLink->m_number_of_intersection_crashes);
				parser.GetValueByFieldName("num_of_intersection_fatal_and_injury_crashes_per_year",pLink->m_num_of_intersection_fatal_and_injury_crashes_per_year);
				parser.GetValueByFieldName("num_of_intersection_PDO_crashes_per_year",pLink->m_num_of_intersection_PDO_crashes_per_year);


				i++;
			}

		}
		m_SimulationLinkMOEDataLoadingStatus.Format ("%d link records are loaded from file %s.",i,lpszFileName);
	}else
	{
		//		AfxMessageBox("Error: File output_LinkMOE.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
	}

}

void CTLiteDoc::ReadHistoricalData(CString directory)
{
}



BOOL CTLiteDoc::OnOpenTrafficNetworkDocument(CString ProjectFileName, bool bNetworkOnly, bool bImportShapeFiles)
{

	CTime LoadingStartTime = CTime::GetCurrentTime();

	m_bLoadNetworkDataOnly = bNetworkOnly;
	FILE* st = NULL;
	//	cout << "Reading file node.csv..."<< endl;

	CString directory;
	m_ProjectFile = ProjectFileName;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

	m_ProjectDirectory = directory;
	m_ProjectTitle = GetWorkspaceTitleName(ProjectFileName);
	SetTitle(m_ProjectTitle);

	CopyDefaultFiles();

	// read users' prespecified control type
	ReadNodeControlTypeCSVFile(directory+"input_node_control_type.csv");
	ReadLinkTypeCSVFile(directory+"input_link_type.csv");

	if(bNetworkOnly ==false)
	{  // we need to check the data consistency here
		ReadVOTCSVFile(directory+"input_VOT.csv");
		ReadVehicleTypeCSVFile(directory+"input_vehicle_type.csv");
		ReadDemandTypeCSVFile(directory+"input_demand_type.csv");
		ReadInputEmissionRateFile(directory+"input_vehicle_emission_rate.csv");
	}

	CWaitCursor wc;
	OpenWarningLogFile(directory);

	m_NodeSet.clear ();
	m_LinkSet.clear ();
	m_ODSize = 0;

	// test if input_node.csv can be opened.
	/*	ifstream input_node_file(directory+"input_node.csv");

	*/

	m_LongLatFlag = (bool)(g_GetPrivateProfileDouble("coordinate_info", "long_lat_flag", 1, ProjectFileName));

	ReadBackgroundImageFile(ProjectFileName);


	if(!ReadNodeCSVFile(directory+"input_node.csv") && m_BackgroundBitmapLoaded ==false)
		return false;
	if(!ReadLinkCSVFile(directory+"input_link.csv",false,false) && m_BackgroundBitmapLoaded ==false) 
		return false;

	ReadTMCSpeedData(directory+"input_TMC_speed.csv");

	if(ReadZoneCSVFile(directory+"input_zone.csv"))
	{
		ReadActivityLocationCSVFile(directory+"input_activity_location.csv");
	}



	LoadSimulationOutput();


	CalculateDrawingRectangle(false);
	m_bFitNetworkInitialized  = false;

	ReadTransitFiles(directory+"Transit_Data\\");  // read transit data

	CTime LoadingEndTime = CTime::GetCurrentTime();

	CTimeSpan ts = LoadingEndTime  - LoadingStartTime;
	CString str_running_time;

	str_running_time.Format ("Network loading time: %d min(s) %d sec(s)...",ts.GetMinutes(), ts.GetSeconds());

	SetStatusText(str_running_time);

	if(bNetworkOnly)
		return true;

	//	OffsetLink(); we do not need to offset here as the importing function has done so. 
	ReadScenarioData();

	//: comment out now, it uses alternative format	

	//	ReadSensorData(directory+"input_sensor_location.csv");
	//	ReadHistoricalData(directory);

	ReadSensorData(directory+"input_sensor.csv");

	//ReadObservationLinkVolumeData(directory+"input_static_obs_link_volume.csv");

	ReadBackgroundImageFile(ProjectFileName);

	ReadSubareaCSVFile(directory+"input_subarea.csv");


	m_AMSLogFile.close();
	return true;
}
BOOL CTLiteDoc::OnOpenRailNetworkDocument(LPCTSTR lpszPathName)
{
	FILE* st = NULL;
	//	cout << "Reading file node.csv..."<< endl;

	CString directory;
	m_ProjectFile = lpszPathName;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);


	m_ProjectDirectory = directory;
	m_ProjectTitle = GetWorkspaceTitleName(lpszPathName);
	SetTitle(m_ProjectTitle);

	CWaitCursor wc;

	OpenWarningLogFile(directory);

	if(!ReadNodeCSVFile(directory+"input_node.csv")) return false;
	if(!ReadLinkCSVFile(directory+"input_link.csv",false,false)) return false;

	ReadScenarioData();

	CalculateDrawingRectangle();

	m_AdjLinkSize +=2;  // add two more elements to be safe

	std::list<DTANode*>::iterator iNode;
	//adjust XY coordinates if the corrdinate system is not consistenty
	if(fabs(m_UnitMile-1.00)>0.10)  // ask users if we need to adjust the XY coordinates
	{
		if(AfxMessageBox("The link length information in link.csv is not consistent with the X/Y coordinates in node.csv.\nDo you want to adjust the the X/Y coordinates in node.csv?", MB_YESNO) == IDYES)

			for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
			{
				(*iNode)->pt.x = ((*iNode)->pt.x - min(m_NetworkRect.left,m_NetworkRect.right))*m_UnitMile;
				(*iNode)->pt.y = ((*iNode)->pt.y - min(m_NetworkRect.top, m_NetworkRect.bottom))*m_UnitMile;
			}

			std::list<DTALink*>::iterator iLink;
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				(*iLink)->m_FromPoint = m_NodeIDMap[(*iLink)->m_FromNodeID]->pt;
				(*iLink)->m_ToPoint = m_NodeIDMap[(*iLink)->m_ToNodeID]->pt;
			}

			//			m_UnitMile  = 1.0;
			//			m_UnitFeet = 1/5280.0;
			CalculateDrawingRectangle();

			UpdateAllViews(0);

	}

	if(m_bLinkToBeShifted)
	{

		std::list<DTALink*>::iterator iLink;

		double minimum_link_length = 999999;
				for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink) -> m_bToBeShifted)
			{
				if((*iLink)->m_Length <  minimum_link_length)
					minimum_link_length = (*iLink)->m_Length ;
			}
		}
			double link_offset = max(minimum_link_length*0.025*m_UnitMile, m_UnitFeet*m_OffsetInFeet);  // 80 feet


		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink) -> m_bToBeShifted)
			{
				double DeltaX = (*iLink)->m_ToPoint.x - (*iLink)->m_FromPoint.x ;
				double DeltaY = (*iLink)->m_ToPoint.y - (*iLink)->m_FromPoint.y ;
				double theta = atan2(DeltaY, DeltaX);

				(*iLink)->m_FromPoint.x += link_offset* cos(theta-PI/2.0f);
				(*iLink)->m_ToPoint.x += link_offset* cos(theta-PI/2.0f);

				(*iLink)->m_FromPoint.y += link_offset* sin(theta-PI/2.0f);
				(*iLink)->m_ToPoint.y += link_offset* sin(theta-PI/2.0f);
			}
		}
	}

	// for train timetabling
	ReadTrainProfileCSVFile(directory+"input_train_link_running_time.csv");
	ReadTimetableCVSFile(directory+"output_timetable.csv");

	return true;
}

BOOL CTLiteDoc::OnOpenDocument(CString ProjectFileName, bool bLoadNetworkOnly )
{
	CWaitCursor wait;

	CTime LoadingStartTime = CTime::GetCurrentTime();

	if(ProjectFileName.Find("tnp")>=0)  //Transportation network project format
	{

		OnOpenTrafficNetworkDocument(ProjectFileName,bLoadNetworkOnly);
	}else if(ProjectFileName.Find("dws")>=0)  //DYNASMART-P format
	{
		OnOpenDYNASMARTProject(ProjectFileName,bLoadNetworkOnly);
	}else if(ProjectFileName.Find("rnp")>=0)  //rail network project format
	{
		OnOpenRailNetworkDocument(ProjectFileName,bLoadNetworkOnly);

	}else
	{
		AfxMessageBox("The selected file type is not selected.");
		return false;	

	}

	
	m_NodeDisplaySize = max(1,g_GetPrivateProfileDouble("GUI","node_display_size",50,ProjectFileName));

	m_NodeTextDisplayRatio = max(1,g_GetPrivateProfileDouble("GUI","node_text_display_ratio",4,ProjectFileName));

	CTime LoadingEndTime = CTime::GetCurrentTime();

	CTimeSpan ts = LoadingEndTime  - LoadingStartTime;

	m_StrLoadingTime.Format ("Overall loading time: %d min(s) %d sec(s)...",ts.GetMinutes(), ts.GetSeconds());

	CDlgFileLoading dlg;
	dlg.m_pDoc = this;
	dlg.DoModal ();

	UpdateAllViews(0);
	return true;

}

bool CTLiteDoc::ReadBackgroundImageFile(LPCTSTR lpszFileName)
{
	//read impage file Background.bmp

	if(m_BackgroundBitmapLoaded)
		m_BackgroundBitmap.Detach ();

	m_BackgroundBitmap.Load(m_ProjectDirectory + "background_image.bmp");

	m_BackgroundBitmapLoaded = !(m_BackgroundBitmap.IsNull ());
	//	m_BackgroundBitmapLoaded = true;

		m_ImageXResolution = 1;
		m_ImageYResolution = 1;

	if(m_BackgroundBitmapLoaded)
	{

		m_OriginOnBottomFlag = (int)(g_GetPrivateProfileDouble("coordinate_info", "origin_on_bottom_flag", m_OriginOnBottomFlag, lpszFileName));
		m_ImageX1 = g_GetPrivateProfileDouble("background_image_coordinate_info", "left", m_NetworkRect.left, lpszFileName);
		m_ImageY1 = g_GetPrivateProfileDouble("background_image_coordinate_info", "bottom", m_NetworkRect.bottom, lpszFileName);
		m_ImageX2 = g_GetPrivateProfileDouble("background_image_coordinate_info", "right", m_NetworkRect.right, lpszFileName);
		m_ImageY2 = g_GetPrivateProfileDouble("background_image_coordinate_info", "top", m_NetworkRect.top, lpszFileName);

		m_ImageWidth = fabs(m_ImageX2 - m_ImageX1);
		m_ImageHeight = fabs(m_ImageY2 - m_ImageY1);

		m_ImageMoveSize = m_ImageWidth/2000.0f;
		m_BackgroundImageFileLoadingStatus.Format ("Optional background image file is loaded.");

	}
	else
		m_BackgroundImageFileLoadingStatus.Format ("Optional background image file is not loaded.");

	return m_BackgroundBitmapLoaded;
}
// CTLiteDoc commands

void CTLiteDoc::OnFileOpen()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Transportation Network Project (*.tnp)|*.tnp|"));
	if(dlg.DoModal() == IDOK)
	{
		OnOpenDocument(dlg.GetPathName());
	}
}

void CTLiteDoc::OnFileSaveimagelocation()
{

	TCHAR IniFilePath[_MAX_PATH];
	sprintf_s(IniFilePath,"%s", m_ProjectFile);

	char lpbuffer[64];

	sprintf_s(lpbuffer,"%f",m_ImageX1);
	WritePrivateProfileString("background_image_coordinate_info","left",lpbuffer,IniFilePath);
	sprintf_s(lpbuffer,"%f",m_ImageY1);
	WritePrivateProfileString("background_image_coordinate_info","bottom",lpbuffer,IniFilePath);

	sprintf_s(lpbuffer,"%f",m_ImageX2);
	WritePrivateProfileString("background_image_coordinate_info","right",lpbuffer,IniFilePath);
	sprintf_s(lpbuffer,"%f",m_ImageY2);
	WritePrivateProfileString("background_image_coordinate_info","top",lpbuffer,IniFilePath);

}

COLORREF CTLiteDoc::GetLinkTypeColor(int LinkType)
{
	COLORREF color;
	switch (LinkType)
	{
	case 1: color = m_ColorFreeway; break;
	case 2: color = m_ColorHighway; break;
	default: color = m_ColorArterial;

	}
	return color;
}

void CTLiteDoc::OnShowShowpathmoe()
{
	m_PathMOEDlgShowFlag = !m_PathMOEDlgShowFlag;

	if(m_PathMOEDlgShowFlag)
	{
		if(g_pPathMOEDlg==NULL)
		{
			g_pPathMOEDlg = new 

				CDlgPathMOE();

			g_pPathMOEDlg->m_pDoc = this;
			g_pPathMOEDlg->SetModelessFlag(TRUE);
			g_pPathMOEDlg->Create(IDD_DIALOG_PATHMOE);
		}

		if(g_pPathMOEDlg->GetSafeHwnd())
			g_pPathMOEDlg->ShowWindow(SW_SHOW);

	}else
	{
		if(g_pPathMOEDlg->GetSafeHwnd())
		{
			g_pPathMOEDlg->ShowWindow(SW_HIDE);
		}
	}

}

void CTLiteDoc::OnUpdateShowShowpathmoe(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(g_LinkMOEDlgShowFlag);
}



void CTLiteDoc::OnSearchListtrains()
{
	CDlgTrainInfo train_dlg;

	train_dlg.m_pDoc = this;

	train_dlg.DoModal ();


}

bool CTLiteDoc::ReadNodeControlTypeCSVFile(LPCTSTR lpszFileName)
{

	m_NodeTypeMap[0] = "unknown_control";
	m_NodeTypeMap[1] = "no_control";
	m_NodeTypeMap[2] = "yield_sign";
	m_NodeTypeMap[3] = "2way_stop_sign";
	m_NodeTypeMap[4] = "4way_stop_sign";
	m_NodeTypeMap[5] = "pretimed_signal";
	m_NodeTypeMap[6] = "actuated_signal";
	m_NodeTypeMap[7] = "roundabout";


	CCSVParser parser;
	if (parser.OpenCSVFile(lpszFileName))
	{
		int control_type_code;
		int i=0;
		while(parser.ReadRecord())
		{
			control_type_code = 0;
			parser.GetValueByFieldName("unknown_control",control_type_code);
			m_NodeTypeMap[control_type_code] = "unknown_control";
			m_ControlType_UnknownControl = control_type_code;

			control_type_code = 1;
			parser.GetValueByFieldName("no_control",control_type_code);
			m_NodeTypeMap[control_type_code] = "no_control";
			m_ControlType_NoControl = control_type_code;

			control_type_code = 2;
			parser.GetValueByFieldName("yield_sign",control_type_code);
			m_NodeTypeMap[control_type_code] = "yield_sign";
			m_ControlType_YieldSign = control_type_code;

			control_type_code = 3;
			parser.GetValueByFieldName("2way_stop_sign",control_type_code);
			m_NodeTypeMap[control_type_code] = "2way_stop_sign";
			m_ControlType_2wayStopSign = control_type_code;

			control_type_code = 4;
			parser.GetValueByFieldName("4way_stop_sign",control_type_code);
			m_NodeTypeMap[control_type_code] = "4way_stop_sign";
			m_ControlType_4wayStopSign = control_type_code;

			control_type_code = 5;
			parser.GetValueByFieldName("pretimed_signal",control_type_code);
			m_NodeTypeMap[control_type_code] = "pretimed_signal";
			m_ControlType_PretimedSignal = control_type_code;

			control_type_code = 6;
			parser.GetValueByFieldName("actuated_signal",control_type_code);
			m_NodeTypeMap[control_type_code] = "actuated_signal";
			m_ControlType_ActuatedSignal = control_type_code;

			control_type_code = 7;
			parser.GetValueByFieldName("roundabout",control_type_code);
			m_NodeTypeMap[control_type_code] = "roundabout";
			m_ControlType_Roundabout = control_type_code;


			control_type_code = 100;
			parser.GetValueByFieldName("roundabout",m_ControlType_ExternalNode);
			m_NodeTypeMap[control_type_code] = "roundabout";
			m_ControlType_Roundabout = control_type_code;

			break;  // just one line
		}



		return true;
	}

	return false;
}

bool CTLiteDoc::ReadNodeCSVFile(LPCTSTR lpszFileName, int LayerNo)
{


	CCSVParser parser;
	if (parser.OpenCSVFile(lpszFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int node_id;
			string name;
			DTANode* pNode = 0;

			int control_type;
			double X;
			double Y;

			node_id =1;
			if(parser.GetValueByFieldName("node_id",node_id) == false)
				break;



			//	TRACE("Node ID: %d\n", node_id);

			if(LayerNo==1) // additional layer
				node_id = node_id*(-1);

			if(!parser.GetValueByFieldName("name",name))
				name = "";

			if(!parser.GetValueByFieldName("control_type",control_type))
				control_type = 0;

			// use the X and Y as default values first
			bool bFieldX_Exist = parser.GetValueByFieldName("x",X);
			parser.GetValueByFieldName("y",Y);

			string geo_string;

			if(parser.GetValueByFieldName("geometry",geo_string))
			{
				// overwrite when the field "geometry" exists
				CGeometry geometry(geo_string);

				std::vector<CCoordinate> CoordinateVector = geometry.GetCoordinateList();

				if(CoordinateVector.size()>0)
				{
					X = CoordinateVector[0].X;
					Y = CoordinateVector[0].Y;
				}

			}

			if(m_NodeNumbertoIDMap.size()!= 0 && m_NodeNumbertoIDMap.find(node_id) != m_NodeNumbertoIDMap.end())
			{
				CString error_message;
				error_message.Format ("Node %d in input_node.csv has been defined twice. Please check.", node_id);
				AfxMessageBox(error_message);
				return 0;
			}

			pNode = new DTANode;
			if(LayerNo ==0)
				pNode->m_NodeNumber = node_id;
			else
				pNode->m_NodeNumber = node_id*(-1);

			pNode->m_ControlType = control_type;
			pNode->m_LayerNo = LayerNo;

			parser.GetValueByFieldName("cycle_length_in_second",pNode->m_CycleLengthInSecond);
			parser.GetValueByFieldName("signal_offset_in_second",pNode->m_SignalOffsetInSecond);

			parser.GetValueByFieldName("QEM_ReferenceNodeNumber",pNode->m_QEM_ReferenceNodeNumber);


			pNode->pt.x = X;
			pNode->pt.y = Y;

			pNode->m_NodeID = i;
			pNode->m_ZoneID = 0;



			m_NodeSet.push_back(pNode);
			m_NodeIDMap[i] = pNode;
			m_NodeNumberMap[node_id] = pNode;
			m_NodeIDtoNumberMap[i] = node_id;
			m_NodeNumbertoIDMap[node_id] = i;
			i++;

			//			cout << "node = " << node << ", X= " << X << ", Y = " << Y << endl;

		}

		bool bRectInitialized = false;
		m_AdjLinkSize = 0;

		for (std::list<DTANode*>::iterator iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if(!bRectInitialized)
			{
				m_GridRect.left = (*iNode)->pt.x ;
				m_GridRect.right = (*iNode)->pt.x;
				m_GridRect.top = (*iNode)->pt.y;
				m_GridRect.bottom = (*iNode)->pt.y;
				bRectInitialized = true;
			}

			m_GridRect.Expand((*iNode)->pt);
		}

		m_GridXStep = max(0.0001,m_GridRect.Width () /100);

		m_GridYStep = max(0.0001,m_GridRect.Height ()/100);


		for (std::list<DTANode*>::iterator iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			int x_key = ((*iNode)->pt.x - m_GridRect.left)/ m_GridXStep;
			int y_key = ((*iNode)->pt.y - m_GridRect.bottom)/ m_GridYStep;

			//feasible region
			x_key = max(0,x_key);
			x_key = min(99,x_key);

			y_key = max(0,y_key);
			y_key = min(99,y_key);

			m_GridMatrix[x_key][y_key].m_NodeVector .push_back ((*iNode)->m_NodeID );
			m_GridMatrix[x_key][y_key].m_NodeX .push_back ((*iNode)->pt.x );
			m_GridMatrix[x_key][y_key].m_NodeY .push_back ((*iNode)->pt.y );

		}


		m_NodeDataLoadingStatus.Format ("%d nodes are loaded from file %s.",m_NodeSet.size(),lpszFileName);
		return true;
	}else
	{
		AfxMessageBox("Error: File input_node.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

}

void CTLiteDoc::FindAccessibleTripID(double x, double y)
{
	m_AccessibleTripIDMap.clear();

	int x_key = (x - m_GridRect.left)/ m_GridXStep;
	int y_key = (y - m_GridRect.bottom)/ m_GridYStep;

	//feasible region
	x_key = max(0,x_key);
	x_key = min(99,x_key);

	y_key = max(0,y_key);
	y_key = min(99,y_key);


	int x_step_in_grid = int(m_max_walking_distance /1.4 *m_UnitMile/m_GridXStep);
	int y_step_in_grid = int(m_max_walking_distance /1.4 *m_UnitMile/m_GridYStep);

	for(int x_i = max(0,x_key-x_step_in_grid); x_i <= min(99,x_key+x_step_in_grid); x_i++)
		for(int y_i = max(0,y_key-y_step_in_grid); y_i <= min(99,y_key+y_step_in_grid); y_i++)
		{

			GridNodeSet element = m_GridMatrix[x_i][y_i];

			for(unsigned int i = 0; i < element.m_TripIDVector .size(); i++)
			{

				m_AccessibleTripIDMap[element.m_TripIDVector[i]]= 1;

			}	// per node in a grid cell

		} // for nearby cell


}
int CTLiteDoc::FindClosestNode(double x, double y, double min_distance,  int step_size)
{

	step_size = int(min_distance/m_GridXStep+1);

	int x_key = (x - m_GridRect.left)/ m_GridXStep;
	int y_key = (y -m_GridRect.bottom)/ m_GridYStep;

	//feasible region
	x_key = max(0,x_key);
	x_key = min(99,x_key);

	y_key = max(0,y_key);
	y_key = min(99,y_key);

	int NodeId = -1;


	for(int x_i = max(0,x_key- step_size); x_i <= min(99,x_key+ step_size); x_i++)
		for(int y_i = max(0,y_key-step_size); y_i <= min(99,y_key+step_size ); y_i++)
		{

			GridNodeSet element = m_GridMatrix[x_i][y_i];

			for(unsigned int i = 0; i < element.m_NodeVector.size(); i++)
			{

				double distance = sqrt( (x-element.m_NodeX[i])*(x-element.m_NodeX[i]) + (y-element.m_NodeY[i])*(y-element.m_NodeY[i]));

				if(distance < min_distance)
				{

					min_distance = distance;

					NodeId =  element.m_NodeVector[i];
				}



			}	// per node in a grid cell

		} // for nearby cell

		return NodeId;
}

int CTLiteDoc::FindClosestZone(double x, double y, double min_distance,  int step_size)
{

	step_size = int(min_distance/m_GridXStep+1);

	int x_key = (x - m_GridRect.left)/ m_GridXStep;
	int y_key = (y -m_GridRect.bottom)/ m_GridYStep;

	//feasible region
	x_key = max(0,x_key);
	x_key = min(99,x_key);

	y_key = max(0,y_key);
	y_key = min(99,y_key);

	int NodeId = -1;
	int ZoneNumber = -1;


	for(int x_i = max(0,x_key- step_size); x_i <= min(99,x_key+ step_size); x_i++)
		for(int y_i = max(0,y_key-step_size); y_i <= min(99,y_key+step_size ); y_i++)
		{

			GridNodeSet element = m_GridMatrix[x_i][y_i];

			for(unsigned int i = 0; i < element.m_NodeVector.size(); i++)
			{
				if( m_NodeIDMap[element.m_NodeVector[i]]->m_ZoneID >0)

				{
					double distance = sqrt( (x-element.m_NodeX[i])*(x-element.m_NodeX[i]) + (y-element.m_NodeY[i])*(y-element.m_NodeY[i]));

					if(distance < min_distance)
					{

						min_distance = distance;

						NodeId =  element.m_NodeVector[i];
						ZoneNumber  = m_NodeIDMap[NodeId]->m_ZoneID;
					}
				}



			}	// per node in a grid cell

		} // for nearby cell

		return ZoneNumber;
}

void CTLiteDoc::ReCalculateLinkBandWidth()
{ //output: m_BandWidthValue for each link
	std::list<DTALink*>::iterator iLink;

	if(m_MaxLinkWidthAsLinkVolume < 10)
		m_MaxLinkWidthAsLinkVolume = 10;

	float VolumeRatio = 1/m_MaxLinkWidthAsLinkVolume;  // 1000 vehicles flow rate as 1 lanes

	float max_total_volume = 1;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_total_link_volume > max_total_volume)
		{
			max_total_volume = (*iLink)->m_total_link_volume;
		}
	}

	float TotalVolumeRatio = VolumeRatio;

	float LaneVolumeEquivalent = 200;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		// default mode
		(*iLink)->m_BandWidthValue =  (*iLink)->m_NumberOfLanes*LaneVolumeEquivalent*VolumeRatio;

		if(m_LinkBandWidthMode == LBW_number_of_lanes)
		{
			if(m_LinkTypeMap[(*iLink)->m_link_type ].IsConnector ())  // 1 lane as connector
				(*iLink)->m_BandWidthValue =  min(1,(*iLink)->m_NumberOfLanes)*LaneVolumeEquivalent*VolumeRatio;
			else
				(*iLink)->m_BandWidthValue =  (*iLink)->m_NumberOfLanes*LaneVolumeEquivalent*VolumeRatio;


		}else if(m_LinkBandWidthMode == LBW_link_volume)
		{
			if(m_LinkMOEMode == MOE_safety)  // safety
			{
				(*iLink)->m_BandWidthValue = (*iLink)->m_number_of_all_crashes *10*VolumeRatio;   // 10 crashes as 5 lanes. 
			}else
			{
				if(g_Simulation_Time_Stamp>=1) // dynamic traffic assignment mode
				{
					float link_volume = 0;

					GetLinkMOE((*iLink), MOE_volume,g_Simulation_Time_Stamp, g_MOEAggregationIntervalInMin, link_volume);

					(*iLink)->m_BandWidthValue = link_volume*VolumeRatio; 
				}else  // total volume
				{
					float link_volume = 0;

					GetLinkMOE((*iLink), MOE_volume,m_DemandLoadingStartTimeInMin, m_DemandLoadingEndTimeInMin-m_DemandLoadingStartTimeInMin, link_volume);

					(*iLink)->m_BandWidthValue = link_volume*VolumeRatio;
				}
			}

			if(m_LinkMOEMode == MOE_volume && (*iLink)->m_bSensorData)  // reference volume
			{
				(*iLink)->m_ReferenceBandWidthValue = (*iLink)->GetSensorLinkHourlyVolume(g_Simulation_Time_Stamp)*VolumeRatio; 
			}

		}else if (m_LinkBandWidthMode == LBW_number_of_marked_vehicles)
		{
			(*iLink)->m_BandWidthValue =  (*iLink)->m_NumberOfMarkedVehicles *VolumeRatio;
		}else
			// default value
		{
			(*iLink)->m_BandWidthValue =  (*iLink)->m_NumberOfLanes*LaneVolumeEquivalent*VolumeRatio;
		}

	}

}
void CTLiteDoc::GenerateOffsetLinkBand()
{
	ReCalculateLinkBandWidth();

	std::list<DTALink*>::iterator iLink;

	double lane_offset = m_UnitFeet*m_LaneWidthInFeet;  // 20 feet per lane

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_BandLeftShapePoints.clear();
		(*iLink)->m_BandRightShapePoints.clear();

		(*iLink)->m_ReferenceBandLeftShapePoints.clear();
		(*iLink)->m_ReferenceBandRightShapePoints.clear();


		int last_shape_point_id = (*iLink) ->m_ShapePoints .size() -1;
		double DeltaX = (*iLink)->m_ShapePoints[last_shape_point_id].x - (*iLink)->m_ShapePoints[0].x;
		double DeltaY = (*iLink)->m_ShapePoints[last_shape_point_id].y - (*iLink)->m_ShapePoints[0].y;
		double theta = 0;

		if(fabs(DeltaY)>0.0000001)
			theta = atan2(DeltaY, DeltaX);


		for(unsigned int si = 0; si < (*iLink) ->m_ShapePoints .size(); si++)
		{

			// calculate theta for each feature point segment
			if(si>= 1 && ((*iLink) ->m_ShapePoints .size() >4 || m_LinkTypeMap[(*iLink)->m_link_type].IsRamp ()))  // ramp or >4 feature points
			{
				last_shape_point_id = si;
				DeltaX = (*iLink)->m_ShapePoints[last_shape_point_id].x - (*iLink)->m_ShapePoints[si-1].x;
				DeltaY = (*iLink)->m_ShapePoints[last_shape_point_id].y - (*iLink)->m_ShapePoints[si-1].y;

				if(fabs(DeltaY)>0.00001)
					theta = atan2(DeltaY, DeltaX);
			}

			GDPoint pt;

			pt.x = (*iLink)->m_ShapePoints[si].x ;
			pt.y = (*iLink)->m_ShapePoints[si].y ;

			(*iLink)->m_BandLeftShapePoints.push_back (pt);

			pt.x  = (*iLink)->m_ShapePoints[si].x + (*iLink)->m_BandWidthValue*lane_offset* cos(theta-PI/2.0f);
			pt.y = (*iLink)->m_ShapePoints[si].y + (*iLink)->m_BandWidthValue*lane_offset* sin(theta-PI/2.0f);

			(*iLink)->m_BandRightShapePoints.push_back (pt);

			if((*iLink)->m_bSensorData) // refernece band
			{

				pt.x = (*iLink)->m_ShapePoints[si].x ;
				pt.y = (*iLink)->m_ShapePoints[si].y ;

				(*iLink)->m_ReferenceBandLeftShapePoints.push_back (pt);

				pt.x  = (*iLink)->m_ShapePoints[si].x + (*iLink)->m_ReferenceBandWidthValue*lane_offset* cos(theta-PI/2.0f);
				pt.y = (*iLink)->m_ShapePoints[si].y + (*iLink)->m_ReferenceBandWidthValue*lane_offset* sin(theta-PI/2.0f);
				(*iLink)->m_ReferenceBandRightShapePoints.push_back (pt);
			}

		}

	}
}

void CTLiteDoc::OffsetLink()
{
	CWaitCursor wait;
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if( (*iLink) ->m_Original_ShapePoints .size() ==0)// no original shape points
		{

			(*iLink) ->m_Original_ShapePoints = (*iLink) ->m_ShapePoints;
		}
	}

	if(m_bLinkToBeShifted)
	{
		std::list<DTALink*>::iterator iLink;

		double minimum_link_length = 999999;
				for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink) -> m_bToBeShifted)
			{
				if((*iLink)->m_Length <  minimum_link_length)
					minimum_link_length = (*iLink)->m_Length ;
			}
		}
			double link_offset = max(minimum_link_length*0.025*m_UnitMile, m_UnitFeet*m_OffsetInFeet);  // 80 feet

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink) -> m_bToBeShifted)
			{

				//Test if an opposite link exits
				unsigned long OppositeLinkKey = GetLinkKey((*iLink)->m_ToNodeID,(*iLink)->m_FromNodeID);
				if ( m_NodeIDtoLinkMap.find(OppositeLinkKey) != m_NodeIDtoLinkMap.end())
				{
					// set two links as two-way links
					(*iLink)->m_bOneWayLink = false;

					DTALink* pLink = m_NodeIDtoLinkMap[OppositeLinkKey];
					if(pLink!=NULL)
						pLink->m_bOneWayLink = false;

				}
			}
		}



		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if( (*iLink)->m_bOneWayLink == false)// apply link split to two way links
			{

				int last_shape_point_id = (*iLink) ->m_Original_ShapePoints .size() -1;

				ASSERT(last_shape_point_id>=0);

				double DeltaX = (*iLink)->m_Original_ShapePoints[last_shape_point_id].x - (*iLink)->m_Original_ShapePoints[0].x;
				double DeltaY = (*iLink)->m_Original_ShapePoints[last_shape_point_id].y - (*iLink)->m_Original_ShapePoints[0].y;
				double theta = 0;			
				if(fabs(DeltaY)>0.000000001)
					theta= atan2(DeltaY, DeltaX);

				for(unsigned int si = 0; si < (*iLink) ->m_Original_ShapePoints .size(); si++)
				{
					// calculate theta for each feature point segment
					if(si>= 1 && ( (*iLink) ->m_Original_ShapePoints .size() >4 || m_LinkTypeMap[(*iLink)->m_link_type].IsRamp ()))  // ramp or >4 feature points
					{
						last_shape_point_id = si;
						DeltaX = (*iLink)->m_Original_ShapePoints[last_shape_point_id].x - (*iLink)->m_Original_ShapePoints[si-1].x;
						DeltaY = (*iLink)->m_Original_ShapePoints[last_shape_point_id].y - (*iLink)->m_Original_ShapePoints[si-1].y;


						//if(fabs(DeltaY)>0.001)
						//	theta= atan2(DeltaY, DeltaX);
					}

					(*iLink)->m_ShapePoints[si].x = (*iLink)->m_Original_ShapePoints[si].x + link_offset* cos(theta-PI/2.0f);
					(*iLink)->m_ShapePoints[si].y = (*iLink)->m_Original_ShapePoints[si].y +  link_offset* sin(theta-PI/2.0f);
				}
			}


		}
	}

	m_bBezierCurveFlag = false;

	if(m_bBezierCurveFlag)  //do not apply m_bBezierCurveFlag 
	{
		BezierCurve bezier_smoothing;

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			// bezier smoothing
			std::vector<GDPoint> NewShapePoints;

			if((*iLink) ->m_ShapePoints .size() >2)
			{
				// general smooth curve
				bezier_smoothing.Bezier2D((*iLink) ->m_ShapePoints, NewShapePoints);

				// re-write to data
				(*iLink) ->m_ShapePoints = NewShapePoints;

			}
		}

	}
}

bool CTLiteDoc::ReadLinkCSVFile(LPCTSTR lpszFileName, bool bCreateNewNodeFlag = false,  int LayerNo = 0)
{

	long i = 0;
	DTALink* pLink = 0;
	float default_coordinate_distance_sum=0;
	float length_sum_in_mile = 0;

	CString error_message;
	CString warning_message = "";
	int warning_message_no = 0;

	bool bTwoWayLinkFlag = false;
	CCSVParser parser;
	if (parser.OpenCSVFile(lpszFileName))
	{
		bool bNodeNonExistError = false;
		while(parser.ReadRecord())
		{

			long link_id = 0;
			int from_node_id;
			int to_node_id;
			int direction = 0;
			double length_in_mile;
			int number_of_lanes= 1;
			int speed_limit_in_mph= 60;
			double capacity_in_pcphpl= 2000;
			int type = 1;
			string name;
			float k_jam = 180;

			CString DTASettingsPath = m_ProjectDirectory+"DTASettings.txt";

			float AADT_conversion_factor = g_GetPrivateProfileDouble("safety_planning", "default_AADT_conversion_factor", 0.1, DTASettingsPath);	

			float wave_speed_in_mph = 12;
			string mode_code = "";

			float grade = 0;

			if(!parser.GetValueByFieldName("name",name))
				name = "";

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


			if(LayerNo == 1)  // for subarea mapmatching, all the node ids are converted to negative numbers. 
			{
				from_node_id = from_node_id *(-1);
				to_node_id = to_node_id *(-1);
			}

			if(m_NodeNumbertoIDMap.find(from_node_id)== m_NodeNumbertoIDMap.end())
			{
				if(bCreateNewNodeFlag == false)  // not create new node
				{
					CString warning;
					warning.Format ("from_node_id %d in input_link.csv has not been defined in input_node.csv.\n",from_node_id);
					bNodeNonExistError = true;

					if(warning_message.GetLength () < 3000)  // not adding and showing too many links
					{
						warning_message += warning;
					}

					continue;
				}else  // need to create a new node with geometry information
				{
					string geo_string;
					std::vector<CCoordinate> CoordinateVector;
					if(parser.GetValueByFieldName("geometry",geo_string))
					{
						// overwrite when the field "geometry" exists
						CGeometry geometry(geo_string);
						CoordinateVector = geometry.GetCoordinateList();
						if(CoordinateVector.size()>=2)
						{
							GDPoint	pt; 
							pt.x = CoordinateVector[0].X;
							pt.y = CoordinateVector[0].Y;
							AddNewNode(pt, from_node_id, LayerNo);

						}else
						{

							error_message.Format ("Field geometry in Link %s has less than 2 feature points, coordinate information of upstream node cannot be extracted. Please check.",name.c_str ());
							AfxMessageBox(error_message);
							return false;
						}

					}else
					{
						AfxMessageBox("Field geometry cannot be found in input_link.csv. Please check.");
						return false;
					}

				}	
			}

			if(m_NodeNumbertoIDMap.find(to_node_id)== m_NodeNumbertoIDMap.end())
			{
				if(bCreateNewNodeFlag == false)  // not create new node
				{
					CString warning;
					warning.Format ("to_node_id %d in input_link.csv has not been defined in input_node.csv\n",to_node_id);
					bNodeNonExistError = true;

					if(warning_message.GetLength () < 3000)  // not adding and showing too many links
					{
						warning_message += warning;
					}
					continue;
				}else  // create new node
				{
					string geo_string;
					std::vector<CCoordinate> CoordinateVector;
					if(parser.GetValueByFieldName("geometry",geo_string))
					{
						// overwrite when the field "geometry" exists
						CGeometry geometry(geo_string);
						CoordinateVector = geometry.GetCoordinateList();
						if(CoordinateVector.size()>=2)
						{
							GDPoint	pt; 
							pt.x = CoordinateVector[CoordinateVector.size()-1].X;
							pt.y = CoordinateVector[CoordinateVector.size()-1].Y;
							AddNewNode(pt, to_node_id, LayerNo);

						}else
						{
							error_message.Format ("Field geometry in Link %s has less than 2 feature points, coordinate information of upstream node cannot be extracted. Please check.",name.c_str ());
							AfxMessageBox(error_message);
							return false;
						}

					}else
					{
						AfxMessageBox("Field geometry cannot be found in input_link.csv. Please check.");
						return false;
					}


				}

			}


			DTALink* pExistingLink =  FindLinkWithNodeIDs(m_NodeNumbertoIDMap[from_node_id],m_NodeNumbertoIDMap[to_node_id]);

			if(pExistingLink)
			{

				CString warning;
				warning.Format ("Link %d-> %d is duplicated.\n", from_node_id,to_node_id);
				if(warning_message.GetLength () < 3000)  // not adding and showing too many links
				{
					warning_message += warning;
				}
				continue;
			}


			if(!parser.GetValueByFieldName("length_in_mile",length_in_mile))
			{
				AfxMessageBox("Field length_in_mile has not been defined in file input_link.csv. Please check.");
				break;
			}


			if(!parser.GetValueByFieldName("direction",direction))
				direction = 1;



			if(!parser.GetValueByFieldName("number_of_lanes",number_of_lanes))
			{
				AfxMessageBox("Field number_of_lanes has not been defined in file input_link.csv. Please check.");
				break;
			}
			int length_of_bays_in_feet = 3000;
			int number_of_left_turn_lanes= 0;
			parser.GetValueByFieldName("number_of_left_turn_lanes",number_of_left_turn_lanes);
			int number_of_right_turn_lanes = 0;
			parser.GetValueByFieldName("number_of_right_turn_lanes",number_of_right_turn_lanes);



			if(!parser.GetValueByFieldName("speed_limit_in_mph",speed_limit_in_mph))
			{
				error_message.Format ("Link %s: Field speed_limit_in_mph has not been defined in file input_link.csv. Please check.",name.c_str ());
				AfxMessageBox(error_message);
				break;
			}

			int green_height = 0;
			parser.GetValueByFieldName("KML_green_height",green_height);
			int red_height = 0;
			parser.GetValueByFieldName("KML_red_height",red_height);

			int blue_height = 0;
			parser.GetValueByFieldName("KML_blue_height",blue_height);
			int yellow_height = 0;
			parser.GetValueByFieldName("KML_yellow_height",yellow_height);

			float saturation_flow_rate_in_vhc_per_hour_per_lane = 2000;
			parser.GetValueByFieldName("saturation_flow_rate_in_vhc_per_hour_per_lane",saturation_flow_rate_in_vhc_per_hour_per_lane);


			if(!parser.GetValueByFieldName("lane_capacity_in_vhc_per_hour",capacity_in_pcphpl))
			{
				error_message.Format("Link %s: Field lane_capacity_in_vhc_per_hour has not been defined in file input_link.csv. Please check.",name.c_str ());
				AfxMessageBox(error_message);
				break;
			}

			if(!parser.GetValueByFieldName("link_type",type))
			{
				AfxMessageBox("Field link_type has not been defined in file input_link.csv. Please check.");
				break;
			}


			if(!parser.GetValueByFieldName("jam_density_in_vhc_pmpl",k_jam))
				k_jam = 180;

			if(!parser.GetValueByFieldName("AADT_conversion_factor",AADT_conversion_factor))
				AADT_conversion_factor = 0.1;

			int EffectiveGreenTimeInSecond = 0;
			parser.GetValueByFieldName("effective_green_time_length_in_second",EffectiveGreenTimeInSecond);

			int m_GreenStartTimetInSecond = 0;
			parser.GetValueByFieldName("green_start_time_in_second",m_GreenStartTimetInSecond);

			if(!parser.GetValueByFieldName("grade",grade))
				grade = 0;

			if(!parser.GetValueByFieldName("wave_speed_in_mph",wave_speed_in_mph))
				wave_speed_in_mph = 12;

			if(!parser.GetValueByFieldName("mode_code",mode_code))
				mode_code  = "";

			if(mode_code.find ('"') !=  string::npos)
			{
				mode_code = '"' + mode_code + '"' ;
			}


			std::string group_1_code,group_2_code, group_3_code;

			parser.GetValueByFieldName("group_1_code",group_1_code);
			parser.GetValueByFieldName("group_2_code",group_2_code);
			parser.GetValueByFieldName("group_3_code",group_3_code);


			double num_driveways_per_mile = 0;
			double volume_proportion_on_minor_leg  = 0;
			double num_3SG_intersections  = 0;
			double num_3ST_intersections  = 0;
			double num_4SG_intersections  = 0;
			double num_4ST_intersections  = 0;

			parser.GetValueByFieldName("num_driveways_per_mile",num_driveways_per_mile);
			parser.GetValueByFieldName("volume_proportion_on_minor_leg",volume_proportion_on_minor_leg);
			parser.GetValueByFieldName("num_3SG_intersections",num_3SG_intersections);
			parser.GetValueByFieldName("num_3ST_intersections",num_3ST_intersections);
			parser.GetValueByFieldName("num_4SG_intersections",num_4SG_intersections);
			parser.GetValueByFieldName("num_4ST_intersections",num_4ST_intersections);

			float transit_travel_time_in_min = 0;
			float transit_transfer_time_in_min = 2;
			float transit_waiting_time_in_min = 5;
			float transit_fare_in_dollar = 1;
			float BPR_alpha_term = 0.15;
			float BPR_beta_term = 4;

			parser.GetValueByFieldName("transit_travel_time_in_min",transit_travel_time_in_min);
			parser.GetValueByFieldName("transit_transfer_time_in_min",transit_transfer_time_in_min);
			parser.GetValueByFieldName("transit_waiting_time_in_min",transit_waiting_time_in_min);
			parser.GetValueByFieldName("transit_fare_in_dollar",transit_fare_in_dollar);
			parser.GetValueByFieldName("BPR_alpha_term",BPR_alpha_term);
			parser.GetValueByFieldName("BPR_beta_term",BPR_beta_term);




			if(!parser.GetValueByFieldName("link_id",link_id))  // no value
			{
				// mark it as 0 first, and we then find a new unique link id after reading all links
				link_id = 0;
			}
			string geo_string;

			std::vector<CCoordinate> Original_CoordinateVector;
			if(parser.GetValueByFieldName("original_geometry",geo_string))
			{
				// overwrite when the field "geometry" exists
				CGeometry geometry(geo_string);
				Original_CoordinateVector = geometry.GetCoordinateList();
			}

			bool bToBeShifted = true;
			std::vector<CCoordinate> CoordinateVector;
			if(parser.GetValueByFieldName("geometry",geo_string))
			{
				// overwrite when the field "geometry" exists
				CGeometry geometry(geo_string);
				CoordinateVector = geometry.GetCoordinateList();
				m_bLinkToBeShifted = false;
				bToBeShifted = false;
			}else
			{
				// no geometry information
				CCoordinate cc_from, cc_to; 
				cc_from.X = m_NodeIDMap[m_NodeNumbertoIDMap[from_node_id]]->pt.x;
				cc_from.Y = m_NodeIDMap[m_NodeNumbertoIDMap[from_node_id]]->pt.y;

				cc_to.X = m_NodeIDMap[m_NodeNumbertoIDMap[to_node_id]]->pt.x;
				cc_to.Y = m_NodeIDMap[m_NodeNumbertoIDMap[to_node_id]]->pt.y;

				CoordinateVector.push_back(cc_from);
				CoordinateVector.push_back(cc_to);
			}



			int link_code_start = 1;
			int link_code_end = 1;

			if (direction == -1) // reversed
			{
				link_code_start = 2; link_code_end = 2;
			}

			if (direction == 0) // two-directional link
			{
				link_code_start = 1; link_code_end = 2;
				bTwoWayLinkFlag = true;
			}



			string TMC_code;
			parser.GetValueByFieldName("TMC",TMC_code);

			for(int link_code = link_code_start; link_code <=link_code_end; link_code++)
			{

				bool bNodeNonExistError = false;
				int m_SimulationHorizon = 1;
				pLink = new DTALink(m_SimulationHorizon);

				pLink->m_LayerNo = LayerNo;
				pLink->m_LinkNo = i;
				pLink->m_Name  = name;
				pLink->m_OrgDir = direction;
				pLink->m_LinkID = link_id;

				pLink->m_Mode_code = mode_code;

				pLink->m_NumberOfLeftTurnLanes =  number_of_left_turn_lanes ;
				pLink->m_NumberOfRightTurnLanes =  number_of_right_turn_lanes ;

				pLink->green_height = green_height;
				pLink->red_height = red_height;
				pLink->blue_height = blue_height;
				pLink->yellow_height = yellow_height;


	
				pLink->m_geo_string  = geo_string;

				if(TMC_code.size() >=1)  // TMC exists 
				{
					pLink->m_TMC_code = TMC_code;
					m_TMC2LinkMap [TMC_code] = pLink;
				}

				pLink->group_1_code = group_1_code;
				pLink->group_2_code = group_2_code;
				pLink->group_3_code = group_3_code;


				if(link_code == 1)  //AB link
				{
					pLink->m_FromNodeNumber = from_node_id;

					pLink->m_ToNodeNumber = to_node_id;
					pLink->m_Direction  = 1;

					pLink->m_FromNodeID = m_NodeNumbertoIDMap[from_node_id];
					pLink->m_ToNodeID= m_NodeNumbertoIDMap[to_node_id];

					int si;

					for(si = 0; si < CoordinateVector.size(); si++)
					{
						GDPoint	pt;
						pt.x = CoordinateVector[si].X;
						pt.y = CoordinateVector[si].Y;
						pLink->m_ShapePoints .push_back (pt);

					}

				}

				if(link_code == 2)  //BA link
				{
					pLink->m_FromNodeNumber = to_node_id;
					pLink->m_ToNodeNumber = from_node_id;
					pLink->m_Direction  = 1;

					pLink->m_FromNodeID = m_NodeNumbertoIDMap[to_node_id];
					pLink->m_ToNodeID= m_NodeNumbertoIDMap[from_node_id];

					for(int si = CoordinateVector.size()-1; si >=0; si--)
					{
						GDPoint	pt;
						pt.x = CoordinateVector[si].X;
						pt.y = CoordinateVector[si].Y;
						pLink->m_ShapePoints .push_back (pt);
					}

					pLink->m_bToBeShifted = bToBeShifted; 
				}
				// original geometry

				if(Original_CoordinateVector.size()>0) // data available
				{
					for(unsigned int si = 0; si < Original_CoordinateVector.size(); si++)
					{
						GDPoint	pt;
						pt.x = Original_CoordinateVector[si].X;
						pt.y = Original_CoordinateVector[si].Y;
						pLink->m_Original_ShapePoints .push_back (pt);

					}
				}else
				{  //   data not available, we might use the previously offet shape points to construct the "base-line" shape points
					pLink->m_Original_ShapePoints =  pLink->m_ShapePoints;

					m_OffsetInFeet = 0;

				}
				pLink->m_NumberOfLanes= number_of_lanes;
				pLink->m_SpeedLimit= max(20,speed_limit_in_mph);  // minimum speed limit is 20 mph
				pLink->m_avg_simulated_speed = pLink->m_SpeedLimit;

				//				pLink->m_Length= max(length_in_mile, pLink->m_SpeedLimit*0.1f/60.0f);  // minimum distance, special note: we do not consider the minimum constraint here, but a vehicle cannot travel longer then 0.1 seconds
				pLink->m_Length= length_in_mile;
				pLink->m_FreeFlowTravelTime = pLink->m_Length/pLink->m_SpeedLimit*60.0f;  // convert from hour to min
				pLink->m_StaticTravelTime = pLink->m_FreeFlowTravelTime;

					pLink->m_TransitTravelTime = max(pLink->m_FreeFlowTravelTime,transit_travel_time_in_min);
			pLink->m_TransitTransferTime = transit_transfer_time_in_min;
			pLink->m_TransitWaitingTime =  transit_waiting_time_in_min;
			pLink->m_TransitFareInDollar = transit_fare_in_dollar;
			pLink->m_BPR_alpha_term =  BPR_alpha_term;
			pLink->m_BPR_beta_term  = BPR_beta_term;


				pLink->m_MaximumServiceFlowRatePHPL= capacity_in_pcphpl;
				pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane  = saturation_flow_rate_in_vhc_per_hour_per_lane;
				pLink->m_LaneCapacity  = pLink->m_MaximumServiceFlowRatePHPL;
				pLink->m_link_type= type;
				pLink->m_Grade = grade;

				pLink->m_Kjam = k_jam;
				pLink->m_AADT_conversion_factor = AADT_conversion_factor;
				pLink->m_Wave_speed_in_mph  = wave_speed_in_mph;
				pLink->m_EffectiveGreenTimeInSecond = EffectiveGreenTimeInSecond;
				pLink->m_GreenStartTimetInSecond = m_GreenStartTimetInSecond;


				pLink->m_Num_Driveways_Per_Mile = num_driveways_per_mile;
				pLink->m_volume_proportion_on_minor_leg = volume_proportion_on_minor_leg;
				pLink->m_Num_3SG_Intersections = num_3SG_intersections;
				pLink->m_Num_3ST_Intersections = num_3ST_intersections;
				pLink->m_Num_4SG_Intersections = num_4SG_intersections;
				pLink->m_Num_4ST_Intersections = num_4ST_intersections;


				m_NodeIDMap[pLink->m_FromNodeID ]->m_Connections+=1;
				m_NodeIDMap[pLink->m_ToNodeID ]->m_Connections+=1;


				if( m_LinkTypeMap[pLink->m_link_type].IsFreeway () ||  m_LinkTypeMap[pLink->m_link_type].IsRamp  ())
				{
				m_NodeIDMap[pLink->m_FromNodeID ]->m_bConnectedToFreewayORRamp = true;
				m_NodeIDMap[pLink->m_ToNodeID ]->m_bConnectedToFreewayORRamp = true;

				}

				m_NodeIDMap[pLink->m_FromNodeID ]->m_OutgoingLinkVector.push_back(pLink->m_LinkNo);
				m_NodeIDMap[pLink->m_ToNodeID ]->m_IncomingLinkVector.push_back(pLink->m_LinkNo);

				//estimation node level production and attraction
				m_NodeIDMap[pLink->m_FromNodeID ]->m_NodeProduction += min(pLink->m_LaneCapacity,pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane) * min(4,pLink->m_NumberOfLanes )/2.0f; // min() is used to avoid artefial large capacity and lare number of lanes
				m_NodeIDMap[pLink->m_ToNodeID ]->m_NodeAttraction += min(pLink->m_LaneCapacity,pLink->m_Saturation_flow_rate_in_vhc_per_hour_per_lane) * min(4,pLink->m_NumberOfLanes )/2.0f;  // /2 is used to get reasonable demand



				unsigned long LinkKey = GetLinkKey( pLink->m_FromNodeID, pLink->m_ToNodeID);

				m_NodeIDtoLinkMap[LinkKey] = pLink;

				__int64  LinkKey2 = GetLink64Key(pLink-> m_FromNodeNumber,pLink->m_ToNodeNumber);
				m_NodeNumbertoLinkMap[LinkKey2] = pLink;

				m_LinkNotoLinkMap[i] = pLink;

				m_LinkIDtoLinkMap[link_id] = pLink;


				m_NodeIDMap[pLink->m_FromNodeID ]->m_TotalCapacity += (pLink->m_MaximumServiceFlowRatePHPL* pLink->m_NumberOfLanes);

				pLink->m_FromPoint = m_NodeIDMap[pLink->m_FromNodeID]->pt;
				pLink->m_ToPoint = m_NodeIDMap[pLink->m_ToNodeID]->pt;

				if(pLink->m_Length>=0.00001)
				{
					default_coordinate_distance_sum+= pLink->DefaultDistance();
					length_sum_in_mile += pLink ->m_Length;
				}
				//			pLink->SetupMOE();



				if(!bNodeNonExistError)
				{
					//TRACE("\nAdd link no.%d,  %d -> %d",i,pLink->m_FromNodeNumber, pLink->m_ToNodeNumber );
					m_LinkSet.push_back (pLink);
					m_LinkNoMap[i]  = pLink;
					i++;
				}
			}

		}

		if(bNodeNonExistError)
		{
			ofstream m_WarningLogFile;

			m_WarningLogFile.open ( m_ProjectDirectory + "warning.log", ios::out);
			if (m_WarningLogFile.is_open())
			{
				m_WarningLogFile.width(12);
				m_WarningLogFile.precision(3) ;
				m_WarningLogFile.setf(ios::fixed);
				m_WarningLogFile << warning_message << endl;

			}


			m_WarningLogFile.close ();

			if(AfxMessageBox("Some nodes in input_link.csv have not been defined in input_node.csv. Do you want to view warning.log?",  MB_YESNO) == IDYES )
			{
				OpenCSVFileInExcel(m_ProjectDirectory + "warning.log");
			}
		}

		m_UnitMile  = 1.0f;

		if(length_sum_in_mile>0.000001f)
		{
			m_UnitMile= default_coordinate_distance_sum / length_sum_in_mile ;

			m_bUnitMileInitialized = true;  // the unit mile has been reset according to the actual distance 
		}

		m_UnitFeet = m_UnitMile/5280.0f;  

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)-> m_Length <=0.00001)  //reset zero length link
			{
				(*iLink)-> m_Length = pLink->DefaultDistance()/m_UnitMile;

			}

		}

		CString SettingsFile;
		SettingsFile.Format ("%sDTASettings.txt",m_ProjectDirectory);
		int long_lat_coordinate_flag = (int)(g_GetPrivateProfileDouble("GUI", "long_lat_coordinate_flag", 1, SettingsFile));	

		/*		if(m_UnitMile<1/50)  // long/lat must be very large and greater than 1/62!
		{

		m_LongLatCoordinateFlag = true;
		m_UnitFeet = m_UnitMile/62/5280.0f;  // 62 is 1 long = 62 miles
		WritePrivateProfileString("GUI","long_lat_coordinate_flag","0",SettingsFile);

		}
		else
		{
		WritePrivateProfileString("GUI","long_lat_coordinate_flag","0",SettingsFile);

		}
		*/
		m_LinkDataLoadingStatus.Format ("%d links are loaded from file %s.",m_LinkSet.size(),lpszFileName);

		Construct4DirectionMovementVector();

		AssignUniqueLinkIDForEachLink();

		if(bTwoWayLinkFlag == true)
			m_bLinkToBeShifted = true;

		GenerateOffsetLinkBand();

		if(warning_message.GetLength () >=1)
		{
			CString final_message; 
			final_message = "Warning messages:\n" + warning_message + "\n\nIf you save the network in *.tnp again, then duplicated links and links with non-existings nodes will be automatically removed.\n";
			AfxMessageBox(final_message);
		}
		return true;
	}else
	{
		AfxMessageBox("Error: File input_link.csv cannot be opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

}

void CTLiteDoc::GenerateSubareaStatistics()
{
	//	
	//if(m_SubareaShapePoints.size()>0)
	//{
	//std::list<DTALink*>::iterator iLink;

	//iLink = m_LinkSet.begin(); 

	//int subarea_link_count = 0;

	//while (iLink != m_LinkSet.end())
	//{
	//	if((*iLink)->m_bIncludedinSubarea ==true)
	//	{
	//	subarea_link_count++;
	//	}
	//	iLink++;
	//}

	//if(subarea_link_count>=1)
	//	return; // do nothing

	//	CTLiteView* pView = 0;
	//	POSITION pos = GetFirstViewPosition();
	//	if(pos != NULL)
	//	{
	//		pView = (CTLiteView*) GetNextView(pos);
	//		if(pView!=NULL )
	//		{
	//			pView->CopyLinkSetInSubarea();
	//		}

	//	}
	//}

}
bool CTLiteDoc::ReadSubareaCSVFile(LPCTSTR lpszFileName)
{

	int lineno = 0 ;
	m_SubareaShapePoints.clear ();

	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			GDPoint point;

			if(parser.GetValueByFieldName("x",point.x) == false)
				break;

			if(parser.GetValueByFieldName("y",point.y) == false)
				break;

			m_SubareaShapePoints.push_back(point);
			lineno++;
		}

		if(m_SubareaShapePoints.size()>0)
		{
			CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
			pMainFrame->m_bShowLayerMap[layer_subarea] = true;
		

		}
	}

	return true;
}

bool CTLiteDoc::ReadActivityLocationCSVFile(LPCTSTR lpszFileName)
{
	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
		int lineno =0;
		m_ActivityLocationCount = 0;
		while(parser.ReadRecord())
		{
			int zone_number;

			if(parser.GetValueByFieldName("zone_id",zone_number) == false)
			{
				if(lineno == 0)
				{
					AfxMessageBox("Field zone_id has not been defined in file input_activity_location.csv. Please check.");
				}
				return false;
			}

			int node_name;
			if(parser.GetValueByFieldName("node_id",node_name) == false)
			{
				AfxMessageBox("Field node_id has not been defined in file input_activity_location.csv. Please check.");
				return false;
			}

			map <int, int> :: const_iterator m_Iter = m_NodeNumbertoIDMap.find(node_name);

			if(m_Iter == m_NodeNumbertoIDMap.end( ))
			{
				m_WarningFile<< "Node Number "  << node_name << " in input_activity_location.csv has not been defined in input_node.csv"  << endl; 

				CString str;
				str.Format("Node Number %d in input_activity_location.csv has not been defined in input_node.csv", node_name );
				AfxMessageBox(str);
				return false;

			}

			// if there are multiple nodes for a zone, the last node id is recorded.
			DTAActivityLocation element;
			element.ZoneID  = zone_number;
			element.NodeNumber = node_name;
			element.pt = m_NodeIDMap [m_NodeNumbertoIDMap[node_name] ] ->pt;

			int External_OD_Flag  = 0;
			parser.GetValueByFieldName("external_OD_flag",External_OD_Flag);
			element.External_OD_flag = External_OD_Flag;

			m_NodeIDtoZoneNameMap[m_NodeNumbertoIDMap[node_name]] = zone_number;
			m_NodeIDMap [m_NodeNumbertoIDMap[node_name] ] -> m_ZoneID = zone_number;
			m_NodeIDMap [m_NodeNumbertoIDMap[node_name] ] ->m_External_OD_flag = element.External_OD_flag;

			m_ZoneMap [zone_number].m_ActivityLocationVector .push_back (element);

			if(m_ODSize < zone_number)
				m_ODSize = zone_number;

			lineno++;
			m_ActivityLocationCount++;
		}

		m_ZoneDataLoadingStatus.Format ("%d node-zone mapping entries are loaded from file %s.",lineno,lpszFileName);
		return true;
	}else
	{
		m_ZoneDataLoadingStatus.Format ("File input_activity_location.csv does not exist. Use default node-zone mapping table.");

		return false;
	}

}

bool CTLiteDoc::ReadZoneCSVFile(LPCTSTR lpszFileName)
{
	bool bNodeNonExistError = false;
	m_NodeIDtoZoneNameMap.clear ();
	int lineno = 0 ;

	CCSVParser parser;

	m_ZoneMap.clear ();

	if (parser.OpenCSVFile(lpszFileName))
	{
		int i=0;
		while(parser.ReadRecord())
		{
			int zone_number;
			float production  = 0;
			float attraction  = 0;

			if(parser.GetValueByFieldName("zone_id",zone_number) == false)
			{
				if(i==0) // first line
				{
					AfxMessageBox("Field zone_id has not been defined in file input_activity_location.csv. Please check.");
				}
				return false;
			}

			int TAZ = zone_number;
			parser.GetValueByFieldName("TAZ",TAZ);


			parser.GetValueByFieldName("production",production);
			parser.GetValueByFieldName("attraction",attraction);

			m_ZoneMap [zone_number].m_ZoneID = TAZ;
			m_ZoneMap [zone_number].m_Production  = production;
			m_ZoneMap [zone_number].m_Attraction  = attraction;

			string color_code; 
			if(parser.GetValueByFieldName("color_code", color_code))
			{
				if(m_KML_style_map.find(color_code)!= m_KML_style_map.end())  // color_code has been defined 
					m_ZoneMap [zone_number].color_code  = color_code;
			}

			string notes; 

			if(parser.GetValueByFieldName("notes",notes))
				m_ZoneMap [zone_number].notes  = notes;

			float height; 
			if(parser.GetValueByFieldName("height",height  ))
				m_ZoneMap [zone_number].m_Height= height;

			std::vector<CCoordinate> CoordinateVector;
			string geo_string;

			if(parser.GetValueByFieldName("geometry",geo_string))
			{
				// overwrite when the field "geometry" exists
				CGeometry geometry(geo_string);
				CoordinateVector = geometry.GetCoordinateList();

				int si;
				for(si = 0; si < CoordinateVector.size(); si++)
				{
					GDPoint	pt;
					pt.x = CoordinateVector[si].X;
					pt.y = CoordinateVector[si].Y;
					m_ZoneMap [zone_number].m_ShapePoints .push_back (pt);

				}

			}
			if(m_ODSize < zone_number)
				m_ODSize = zone_number;

			lineno++;
		}

		if(bNodeNonExistError)
			AfxMessageBox("Some nodes in input_zone.csv have not been defined in input_node.csv. Please check warning.log in the project folder.");

		m_ZoneDataLoadingStatus.Format ("%d zone info records are loaded from file %s.",lineno,lpszFileName);
		return true;
	}else
	{
		m_ZoneDataLoadingStatus.Format ("File input_zone.csv does not exist. Use default zone-TAZ mapping table.");

		return false;
	}

}

bool CTLiteDoc::ReadScenarioSettingCSVFile(LPCTSTR lpszFileName)
{
		CCSVParser parser_scenario;
	if (parser_scenario.OpenCSVFile(lpszFileName))
	{

		m_NumberOfSecenarioSettings = 0;
		while(parser_scenario.ReadRecord())
		{
		parser_scenario.GetValueByFieldName("number_of_assignment_days",m_number_of_assignment_days);

		parser_scenario.GetValueByFieldName("traffic_flow_model",m_traffic_flow_model);

		parser_scenario.GetValueByFieldName("traffic_assignment_method",m_traffic_assignment_method);
		parser_scenario.GetValueByFieldName("agent_demand_input_mode",m_agent_demand_input_mode);
		parser_scenario.GetValueByFieldName("ODME_mode",m_ODME_mode);
		parser_scenario.GetValueByFieldName("emission_data_output",m_emission_data_output);
		parser_scenario.GetValueByFieldName("demand_multiplier",m_demand_multiplier);
		m_NumberOfSecenarioSettings++;
		}
		
	}
	return true;
}


bool CTLiteDoc::WriteScenarioSettingCSVFile(LPCTSTR lpszFileName)
{
int scenario_no;
string scenario_name;
int random_seed;
int default_arterial_k_jam;
int default_cycle_length;
int ODME_start_iteration;

float ODME_max_percentage_deviation_wrt_hist_demand;
float ODME_step_size;
float freeway_bias_factor;
int ue_gap_calculation_method;
int agent_demand_input_mode;
int calibration_data_start_time_in_min;
int calibration_data_end_time_in_min;
int routing_movement_delay_mode;

int accessibility_mode;

		CCSVParser parser_scenario;
	if (parser_scenario.OpenCSVFile(lpszFileName))
	{
		m_NumberOfSecenarioSettings = 0;

		while(parser_scenario.ReadRecord())
		{
		parser_scenario.GetValueByFieldName("scenario_no",scenario_no);
		parser_scenario.GetValueByFieldName("scenario_name",scenario_name);

		// the following parameters can be changed through dialog
		//parser_scenario.GetValueByFieldName("number_of_assignment_days",m_number_of_assignment_days);//
		//parser_scenario.GetValueByFieldName("traffic_flow_model",m_traffic_flow_model);
		//parser_scenario.GetValueByFieldName("traffic_assignment_method",m_traffic_assignment_method);
		//parser_scenario.GetValueByFieldName("demand_multiplier",m_demand_multiplier);
		//parser_scenario.GetValueByFieldName("emission_data_output",m_emission_data_output);

		parser_scenario.GetValueByFieldName("agent_demand_input_mode",m_agent_demand_input_mode);
		parser_scenario.GetValueByFieldName("ODME_mode",m_ODME_mode);

		parser_scenario.GetValueByFieldName("random_seed",random_seed);
		parser_scenario.GetValueByFieldName("default_arterial_k_jam",default_arterial_k_jam);
		parser_scenario.GetValueByFieldName("default_cycle_length",default_cycle_length);

		parser_scenario.GetValueByFieldName("ODME_start_iteration",ODME_start_iteration);
		parser_scenario.GetValueByFieldName("ODME_max_percentage_deviation_wrt_hist_demand",ODME_max_percentage_deviation_wrt_hist_demand);
		parser_scenario.GetValueByFieldName("ODME_step_size",ODME_step_size);
		parser_scenario.GetValueByFieldName("freeway_bias_factor",freeway_bias_factor);
		parser_scenario.GetValueByFieldName("ue_gap_calculation_method",ue_gap_calculation_method);
		parser_scenario.GetValueByFieldName("agent_demand_input_mode",agent_demand_input_mode);
		parser_scenario.GetValueByFieldName("calibration_data_start_time_in_min",calibration_data_start_time_in_min);
		parser_scenario.GetValueByFieldName("calibration_data_end_time_in_min",calibration_data_end_time_in_min);
		parser_scenario.GetValueByFieldName("routing_movement_delay_mode",routing_movement_delay_mode);
//	parser_scenario.GetValueByFieldName("accessibility_calculation_mode",accessibility_mode);
		
		m_NumberOfSecenarioSettings++;
		}
	
	parser_scenario.CloseCSVFile ();


	}


	if(m_NumberOfSecenarioSettings==1)
	{
	CCSVWriter ScenarioFile;

	if(ScenarioFile.Open(lpszFileName))
	{


		ScenarioFile.SetFieldNameAndValue("scenario_no",scenario_no);
		ScenarioFile.SetFieldNameAndValue("scenario_name",scenario_name);
		ScenarioFile.SetFieldNameAndValue("number_of_assignment_days",m_number_of_assignment_days);//
		ScenarioFile.SetFieldNameAndValue("traffic_flow_model",m_traffic_flow_model);
		ScenarioFile.SetFieldNameAndValue("traffic_assignment_method",m_traffic_assignment_method);
		ScenarioFile.SetFieldNameAndValue("agent_demand_input_mode",m_agent_demand_input_mode);
		ScenarioFile.SetFieldNameAndValue("ODME_mode",m_ODME_mode);
		ScenarioFile.SetFieldNameAndValue("emission_data_output",m_emission_data_output);
		ScenarioFile.SetFieldNameAndValue("demand_multiplier",m_demand_multiplier);

		ScenarioFile.SetFieldNameAndValue("random_seed",random_seed);
		ScenarioFile.SetFieldNameAndValue("default_arterial_k_jam",default_arterial_k_jam);
		ScenarioFile.SetFieldNameAndValue("default_cycle_length",default_cycle_length);

		ScenarioFile.SetFieldNameAndValue("ODME_start_iteration",ODME_start_iteration);
		ScenarioFile.SetFieldNameAndValue("ODME_max_percentage_deviation_wrt_hist_demand",ODME_max_percentage_deviation_wrt_hist_demand);
		ScenarioFile.SetFieldNameAndValue("ODME_step_size",ODME_step_size);
		ScenarioFile.SetFieldNameAndValue("freeway_bias_factor",freeway_bias_factor);
		ScenarioFile.SetFieldNameAndValue("ue_gap_calculation_method",ue_gap_calculation_method);
		ScenarioFile.SetFieldNameAndValue("agent_demand_input_mode",agent_demand_input_mode);
		ScenarioFile.SetFieldNameAndValue("calibration_data_start_time_in_min",calibration_data_start_time_in_min);
		ScenarioFile.SetFieldNameAndValue("calibration_data_end_time_in_min",calibration_data_end_time_in_min);
		ScenarioFile.SetFieldNameAndValue("routing_movement_delay_mode",routing_movement_delay_mode);
//		ScenarioFile.SetFieldNameAndValue("accessibility_calculation_mode",accessibility_mode);


		ScenarioFile.WriteHeader ();
		ScenarioFile.WriteRecord ();

	}
	}else if (m_NumberOfSecenarioSettings >=2)
	{
	AfxMessageBox("There are multiple scenarios in input_scenario_settings.csv. Please use Excel to make changes.", MB_ICONINFORMATION);

	OpenCSVFileInExcel(lpszFileName);
	
	}

	return true;
}


bool CTLiteDoc::ReadMetaDemandCSVFile(LPCTSTR lpszFileName)
{
	CString directory;

	// reset
	m_DemandLoadingStartTimeInMin = 1440;
	m_DemandLoadingEndTimeInMin = 0;

	m_DemandFileVector.clear ();

	if(m_ProjectFile.GetLength () ==0 )
	{
		AfxMessageBox("The project directory has not been specified. Please save the project to a new folder first.");
		return false;
	}
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);


	CCSVParser parser;
	if (parser.OpenCSVFile(lpszFileName))
	{

		while(parser.ReadRecord())
		{
			string file_name;
			string format_type;
			int demand_type= 1;
			int start_time_in_min,end_time_in_min;
			float subtotal_demand_volume;

			parser.GetValueByFieldName("file_name",file_name);

			m_DemandFileVector.push_back (file_name.c_str ());
			parser.GetValueByFieldName("format_type",format_type);
			parser.GetValueByFieldName("demand_type",demand_type);

			if(demand_type >= m_DemandTypeVector.size())  // demand type out of bound
				break;

			parser.GetValueByFieldName("start_time_in_min",start_time_in_min);
			parser.GetValueByFieldName("end_time_in_min",end_time_in_min);
			parser.GetValueByFieldName("subtotal_demand_volume",subtotal_demand_volume);

			if(m_DemandLoadingStartTimeInMin > start_time_in_min)
				m_DemandLoadingStartTimeInMin = start_time_in_min;

			if(m_DemandLoadingEndTimeInMin < end_time_in_min)
				m_DemandLoadingEndTimeInMin = end_time_in_min;


		}

	}  // for each record
	return true;

}

bool CTLiteDoc::ReadDemandCSVFile(LPCTSTR lpszFileName)
{
	float LengthinMB;
	FILE* pFile;
	long Length;
	fopen_s(&pFile,lpszFileName,"rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		Length = ftell(pFile);
		fclose(pFile);

		LengthinMB= Length*1.0/1024/1024;
		if(LengthinMB > 100)
		{
			CString msg;
			msg.Format("The input_demand.csv file is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load the input_demand file?",LengthinMB);
			if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
				return true;
		}
	}

	//  // step 1: unzip
	//
	//CString directory;

	//if(m_ProjectFile.GetLength () ==0 )
	//{
	//	AfxMessageBox("The project directory has not been specified. Please save the project to a new folder first.");
	//	return false;
	//}
	//directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

	//HZIP hz = OpenZip(directory+"TNP_data.zip",0);
	//   
	//SetCurrentDirectory(directory);

	//   ZIPENTRY ze; GetZipItem(hz,-1,&ze); int numitems=ze.index;
	// 	for (int zi=0; zi<numitems; zi++)
	//  { 
	//GetZipItem(hz,zi,&ze);
	//   UnzipItem(hz,zi,ze.name);
	//  }
	//   CloseZip(hz);


	//// step 2: read bin file

	// // obtain file size:
	//// obtain file size:
	// pFile = fopen ( directory+"input_demand.bin" , "rb" );
	// if (pFile!=NULL)
	// {
	//  float total_demand = 0;
	// fseek (pFile , 0 , SEEK_END);
	// long lSize = ftell (pFile);
	// rewind (pFile);
	// int RecordCount = lSize/sizeof(DemandRecordData);
	// if(RecordCount >=1)
	// {
	// // allocate memory to contain the whole file:
	// DemandRecordData* pDemandData = new DemandRecordData [RecordCount];
	// // copy the file into the buffer:
	// fread (pDemandData,1,sizeof(DemandRecordData)*RecordCount,pFile);
	// int demand_type;
	// int demand_type_size = min(5,m_DemandTypeVector.size());
	// for(int i = 0; i < RecordCount; i++)
	// {
	//for(demand_type = 0; demand_type < demand_type_size; demand_type++)
	//{
	//	total_demand +=  pDemandData[i].number_of_vehicles[demand_type];
	//	m_ZoneMap[pDemandData[i].origin_zone ].m_ODDemandMatrix [pDemandData[i].destination_zone].SetValue (demand_type+1,pDemandData[i].number_of_vehicles[demand_type]);
	//}
	// 
	// }
	// 
	// /* the whole file is now loaded in the memory buffer. */

	// // terminate
	// fclose (pFile);

	//	m_AMSLogFile << RecordCount << "demand entries are loaded from file input_demand.bin "  ". Total demand =  " 
	//		<< total_demand << endl;
	//	m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file input_demand.bin. Total demand = %f",RecordCount,total_demand);


	// if(pDemandData!=NULL)
	//  delete pDemandData;

	//
	//  return true;
	// }
	// }

	////


	long lineno = 0;
	float total_demand = 0;
	FILE* st;
	fopen_s(&st,lpszFileName, "r");
	if (st!=NULL)
	{
		char  str_line[2000]; // input string
		int str_line_size;
		g_read_a_line(st,str_line, str_line_size); //  skip the first line

		int origin_zone, destination_zone;
		float number_of_vehicles ;
		float starting_time_in_min;
		float ending_time_in_min;


		while( fscanf_s(st,"%d,%d,%f,%f,",&origin_zone,&destination_zone,&starting_time_in_min, &ending_time_in_min) >0)
		{
			// static traffic assignment, set the demand loading horizon to [0, 60 min]
			for(unsigned int demand_type = 1; demand_type <= m_DemandTypeVector.size(); demand_type++)
			{
				number_of_vehicles = 0;
				if( fscanf_s(st,"%f,",&number_of_vehicles) == 0)
				{

					CString msg;
					msg.Format ("Demand type %d in line %d of file %s cannot be found. ",demand_type,lineno+1,lpszFileName);
					AfxMessageBox(msg,MB_OK|MB_ICONINFORMATION);
					return false;
				}

				if(number_of_vehicles < -0.0001)
					number_of_vehicles = 0;

				if(origin_zone <= m_ODSize && destination_zone <= m_ODSize)
				{
					m_ZoneMap[origin_zone].m_ODDemandMatrix [destination_zone].SetValue (demand_type,number_of_vehicles);
					total_demand += number_of_vehicles;
				}
				else
				{
					CString msg;
					msg.Format ("Line %d in file %s has a zone number greater than the size of zones (%d). ",lineno,lpszFileName, m_ODSize);
					AfxMessageBox(msg,MB_OK|MB_ICONINFORMATION);
					return false;
				}
			}

			lineno++;
		}

		fclose(st);
		m_AMSLogFile << lineno << "demand entries are loaded from file " << lpszFileName << ". Total demand =  " << total_demand << endl;
		m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file %s. Total demand = %f",lineno,lpszFileName,total_demand);
		return true;
	}else
	{
		//		AfxMessageBox("Error: File input_demand.csv cannot be found or opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

}


bool CTLiteDoc::ReadDemandTypeCSVFile(LPCTSTR lpszFileName)
{
	long lineno = 1;
	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
		m_DemandTypeVector.clear();

		while(parser.ReadRecord())
		{
			int demand_type;
			float averageVOT;

			if(parser.GetValueByFieldName("demand_type",demand_type) == false)
				break;

			parser.GetValueByFieldName("average_VOT",averageVOT);

			string demand_type_name, pricing_type_name;
			parser.GetValueByFieldName("demand_type_name",demand_type_name);

			int pricing_type;
			float ratio_pretrip, ratio_enroute;

			parser.GetValueByFieldName("pricing_type",pricing_type);
			parser.GetValueByFieldName("percentage_of_pretrip_info",ratio_pretrip);
			parser.GetValueByFieldName("percentage_of_enroute_info",ratio_enroute);

			DTADemandType element;
			element.demand_type = demand_type;
			element.demand_type_name  = demand_type_name.c_str ();
			element.average_VOT = averageVOT;
			element.pricing_type = pricing_type;
			element.info_class_percentage[1] = ratio_pretrip;
			element.info_class_percentage[2] = ratio_enroute;
			element.info_class_percentage[0] = 1 - ratio_enroute - ratio_pretrip;

			for( int i=0; i< m_VehicleTypeVector.size(); i++)
			{
				std::ostringstream  str_percentage_of_vehicle_type;
				str_percentage_of_vehicle_type << "percentage_of_vehicle_type" << i+1;

				float percentage_vehicle_type = 0;
				if(parser.GetValueByFieldName(str_percentage_of_vehicle_type.str(),percentage_vehicle_type))
					element.vehicle_type_percentage[i]= percentage_vehicle_type;
			}

			m_DemandTypeVector.push_back(element);

			lineno++;
		}
		m_AMSLogFile << "Read " << m_DemandTypeVector.size() << " demand types from file "  << lpszFileName << endl; 

		CString msg;
		msg.Format("Imported %d demand types from file %s",m_DemandTypeVector.size(),lpszFileName);
		m_MessageStringVector.push_back (msg);

		return true;
	}else
	{
		CString msg;
		msg.Format("Imported 0 demand types from file %s",lpszFileName);
		m_MessageStringVector.push_back (msg);

		return false;
	}

}

bool CTLiteDoc::ReadVehicleTypeCSVFile(LPCTSTR lpszFileName)
{
	long lineno = 1;
	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
		m_VehicleTypeVector.clear();

		while(parser.ReadRecord())
		{
			int demand_type;

			if(parser.GetValueByFieldName("vehicle_type",demand_type) == false)
				break;

			string vehicle_type_name;
			parser.GetValueByFieldName("vehicle_type_name",vehicle_type_name);

			DTAVehicleType element;
			element.vehicle_type = demand_type;
			element.vehicle_type_name  = vehicle_type_name.c_str ();

			m_VehicleTypeVector.push_back(element);

			lineno++;
		}

		m_AMSLogFile << "Read " << m_VehicleTypeVector.size() << " vehicle types from file "  << lpszFileName << endl; 

		return true;
	}else
	{
		AfxMessageBox("Error: File input_vehicle_type.csv cannot be found or opened.\n It might be currently used and locked by EXCEL.");
		return false;
		//		g_ProgramStop();
	}

}

bool CTLiteDoc::ReadLinkTypeCSVFile(LPCTSTR lpszFileName)
{
	long lineno = 0;
	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
		m_LinkTypeMap.clear();
		while(parser.ReadRecord())
		{
			DTALinkType element;

			if(parser.GetValueByFieldName("link_type",element.link_type ) == false)
			{
				if(lineno==0)
				{
					AfxMessageBox("Field link_type cannot be found in input_link_type.csv.");
				}
				break;
			}


			if(element.link_type<=0)
			{
				CString str;
				str.Format ("Field link_type in file input_link_type.csv has an invalid value of %d at line %d. Please check.",element.link_type, lineno);
				AfxMessageBox(str);
				break;
			}

			if(parser.GetValueByFieldName("link_type_name",element.link_type_name ) == false)
			{
				AfxMessageBox("Field link_type_name cannot be found in input_link_type.csv.");
				break;
			}


			if(parser.GetValueByFieldName("type_code",element.type_code   ) == false)
			{
				AfxMessageBox("Field type_code cannot be found in input_link_type.csv.");
				break;
			}

			if(parser.GetValueByFieldName("default_lane_capacity",element.default_lane_capacity   ) == false && lineno==0)
			{
				AfxMessageBox("Field default_lane_capacity cannot be found in input_link_type.csv.");
			}

			if(parser.GetValueByFieldName("default_speed_limit",element.default_speed) == false  && lineno==0)
			{
				AfxMessageBox("Field default_speed cannot be found in input_link_type.csv.");
			}

			if(parser.GetValueByFieldName("default_number_of_lanes",element.default_number_of_lanes) == false  && lineno==0)
			{
				AfxMessageBox("Field default_number_of_lanes cannot be found in input_link_type.csv.");
			}
			
			if(element.type_code .find('f') != string::npos)
			{
				m_LinkTypeFreeway = element.link_type;
			}

			if(element.type_code .find('a') != string::npos)
			{
				m_LinkTypeArterial = element.link_type;
			}

			if(element.type_code .find('h') != string::npos)
			{
				m_LinkTypeHighway = element.link_type;
			}

			m_LinkTypeMap[element.link_type] = element ;

			lineno++;
		}

		CString msg;
		msg.Format("Imported %d link types from file %s",m_LinkTypeMap.size(),lpszFileName);
		m_MessageStringVector.push_back (msg);
		m_AMSLogFile << "Read " << m_LinkTypeMap.size() << " link types from file "  << lpszFileName << endl; 

		return true;
	}else
	{
		return false;

	}

}
bool CTLiteDoc::ReadTemporalDemandProfileCSVFile(LPCTSTR lpszFileName)
{
	long lineno = 1;
	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
		while(parser.ReadRecord())
		{

			int from_zone_id, to_zone_id;
			int demand_type;

			if(parser.GetValueByFieldName("from_zone_id",from_zone_id) == false)
				break;

			if(parser.GetValueByFieldName("to_zone_id",to_zone_id) == false)
				break;

			if(parser.GetValueByFieldName("demand_type",demand_type) == false)
				break;

			string time_series_label;
			parser.GetValueByFieldName("time_series_label",time_series_label);


			DTADemandProfile element; 

			element.demand_type = demand_type;
			element.from_zone_id = from_zone_id;
			element.to_zone_id = to_zone_id;

			element.series_name = time_series_label.c_str ();

			for(int t = 0; t< MAX_TIME_INTERVAL_SIZE; t++)
			{
				CString time_stamp_str = GetTimeStampStrFromIntervalNo (t,true);

				// Convert a TCHAR string to a LPCSTR
				CT2CA pszConvertedAnsiString (time_stamp_str);

				// construct a std::string using the LPCSTR input
				std::string strStd (pszConvertedAnsiString);

				float percentage = 0.0f;
				parser.GetValueByFieldName(strStd,percentage);

				if( percentage>0.00001)
					element.time_dependent_ratio[t] = percentage;

			}

			m_DemandProfileVector.push_back (element);

			lineno++;
		}
	}
	m_AMSLogFile << "Read " << m_DemandProfileVector.size() << " temporal demand elements from file "  << lpszFileName << endl; 

	CString msg;
	msg.Format("Imported %d temporal demand elements from file %s",m_DemandProfileVector.size(),lpszFileName);
	m_MessageStringVector.push_back (msg);

	if(m_DemandProfileVector.size()>=1)
		return true;
	else
		return false;

	return true;
}
bool CTLiteDoc::ReadVOTCSVFile(LPCTSTR lpszFileName)
{
	long lineno = 1;
	CCSVParser parser;

	if (parser.OpenCSVFile(lpszFileName))
	{
		m_VOTDistributionVector.clear();
		while(parser.ReadRecord())
		{

			int demand_type;
			float percentage; float VOT;

			if(parser.GetValueByFieldName("demand_type",demand_type) == false)
				break;
			if(parser.GetValueByFieldName("percentage",percentage) == false)
				break;
			if(parser.GetValueByFieldName("VOT_dollar_per_hour",VOT) == false)
			{
				AfxMessageBox("Field VOT_dollar_per_hour cannot be found input_VOT.csv");
				break;
			}

			DTAVOTDistribution element;
			element.demand_type = demand_type;
			element.percentage  = percentage;
			element.VOT = VOT;

			m_VOTDistributionVector.push_back(element);

			lineno++;
		}
	}

	m_AMSLogFile << "Read " << m_VOTDistributionVector.size() << " VOT elements from file "  << lpszFileName << endl; 
	CString msg;
	msg.Format("Imported %d VOT elements from file %s",m_VOTDistributionVector.size(),lpszFileName);
	m_MessageStringVector.push_back (msg);

	if(m_VOTDistributionVector.size() >= 1)
		return true;
	else
		return false;



}
bool CTLiteDoc::Read3ColumnTripTxtFile(LPCTSTR lpszFileName)
{

	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	long lineno = 1;
	if(st!=NULL)
	{


		while(!feof(st))
		{
			int origin_zone	       = g_read_integer(st);
			if(origin_zone == -1)  // reach end of file
				break;

			int destination_zone	   = g_read_integer(st);  // we might read "Origin" here, so if we find "O", we return 0 as a key to jump to the next origin.

			if(origin_zone > m_ODSize)
			{
				CString msg;
				msg.Format ("Error at line %d: origin zone number = %d, which is greater than the number of OD zones: %d.",lineno,origin_zone, m_ODSize);
				break;
			}

			if(destination_zone > m_ODSize)
			{
				CString msg;
				msg.Format ("Error at line %d: destination zone number = %d, which is greater than the number of OD zones: %d.",lineno,origin_zone, m_ODSize);
				break;
			}

			float number_of_vehicles = g_read_float(st);

			m_ZoneMap[origin_zone].m_ODDemandMatrix [destination_zone].SetValue (1, number_of_vehicles);

			lineno++;
		}
		fclose(st);
		m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file %s.",lineno,lpszFileName);
		return true;
	}else
	{
		AfxMessageBox("Error: OD Trip file cannot be opened.");
		return false;
		//		g_ProgramStop();
	}
}
bool CTLiteDoc::ReadTripTxtFile(LPCTSTR lpszFileName)
{
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	long lineno = 0;
	if(st!=NULL)
	{

		while(!feof(st))
		{
			for(int i = 1; i <=m_ODSize; i++)
			{
				int origin_zone	       = g_read_integer(st);
				if(origin_zone == -1)  // reach end of file
					break;

				for(int j = 1; j <=m_ODSize; j++)
				{
					int destination_zone	   = g_read_integer_with_char_O(st);  // we might read "Origin" here, so if we find "O", we return 0 as a key to jump to the next origin.

					if(destination_zone == 0) // we reach "O" of string Origin 
					{
						break;
					}

					destination_zone = j;  // use destination that we read to override j

					float number_of_vehicles = g_read_float(st);

					m_ZoneMap[origin_zone].m_ODDemandMatrix [destination_zone].SetValue (1, number_of_vehicles);
					lineno++;
				}
			}
		}
		fclose(st);
		m_DemandDataLoadingStatus.Format ("%d demand entries are loaded from file %s.",lineno,lpszFileName);
		return true;
	}else
	{
		AfxMessageBox("Error: OD Trip file cannot be opened.");
		return false;
		//		g_ProgramStop();
	}
}

bool CTLiteDoc::ReadScenarioData()
{

	// clean up all scenario data first
	for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->CapacityReductionVector .clear();
		(*iLink)->TollVector .clear();
		(*iLink)->MessageSignVector.clear ();

	}

	//  Dynamic Message Sign
	int i =0;
	i+=ReadIncidentScenarioData();
	i+=ReadWorkZoneScenarioData();
	i+=ReadVMSScenarioData();
	i+=ReadLink_basedTollScenarioData();

	if(i > 0)
	{
		m_ScenarioDataLoadingStatus.Format ("%d scenario records are loaded.",i);
	}

	return true;
}


void CTLiteDoc::SendTexttoStatusBar(CString str)
{
	POSITION pos = GetFirstViewPosition();
	CView *p = (CView *)this->GetNextView(pos);
	CStatusBar* bar = (CStatusBar*)(p->GetParentFrame()->GetMessageBar());
	bar->SetPaneText(0, str);
}


bool  CTLiteDoc::SaveSubareaDemandFile()
{
	FILE* st = NULL;
	FILE* st_meta_data = NULL;
	CString directory;

	if(m_ProjectFile.GetLength () ==0 )
	{
		AfxMessageBox("The project directory has not been specified. Please save the project to a new folder first.");
		return false;
	}

	if(m_bSaveProjectFromSubareaCut == true)  // after subarea cut, we need to regenerate demand matrix
	{
		// export path flow file 
		directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

		ExportPathflowToCSVFiles();

		fopen_s(&st_meta_data,directory+"input_demand_meta_data.csv","w");
		if(st_meta_data!=NULL)
		{
			fprintf(st_meta_data,"scenario_no,file_sequence_no,file_name,format_type,number_of_lines_to_be_skipped,loading_multiplier,start_time_in_min,end_time_in_min,apply_additional_time_dependent_profile,subtotal_in_last_column,number_of_demand_types,demand_type_1,demand_type_2,demand_type_3,demand_type_4,'00:00,'00:15,'00:30,'00:45,'01:00,'01:15,'01:30,'01:45,'02:00,'02:15,'02:30,'02:45,'03:00,'03:15,'03:30,'03:45,'04:00,'04:15,'04:30,'04:45,'05:00,'05:15,'05:30,'05:45,'06:00,'06:15,'06:30,'06:45,'07:00,'07:15,'07:30,'07:45,'08:00,'08:15,'08:30,'08:45,'09:00,'09:15,'09:30,'09:45,'10:00,'10:15,'10:30,'10:45,'11:00,'11:15,'11:30,'11:45,'12:00,'12:15,'12:30,'12:45,'13:00,'13:15,'13:30,'13:45,'14:00,'14:15,'14:30,'14:45,'15:00,'15:15,'15:30,'15:45,'16:00,'16:15,'16:30,'16:45,'17:00,'17:15,'17:30,'17:45,'18:00,'18:15,'18:30,'18:45,'19:00,'19:15,'19:30,'19:45,'20:00,'20:15,'20:30,'20:45,'21:00,'21:15,'21:30,'21:45,'22:00,'22:15,'22:30,'22:45,'23:00,'23:15,'23:30,'23:45\n");
			fprintf(st_meta_data,"0,1,AMS_OD_table.csv,column,1,1,%d,%d,0,0,1,1\n",m_DemandLoadingStartTimeInMin,m_DemandLoadingEndTimeInMin);
			// m_DemandLoadingStartTimeInMin,m_DemandLoadingEndTimeInMin are read from the original project
			fclose(st_meta_data);

			AfxMessageBox("File input_demand_meta_data.csv is reset with AMS_OD_table.csv as a new demand input file, as a result of the subarea cut.", MB_ICONINFORMATION);
		}else
		{
			AfxMessageBox("Error: File input_demand_meta_data.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
			return false;
		}


		m_bSaveProjectFromSubareaCut = false;
	}
	return true;
}

void  CTLiteDoc::CopyDefaultFiles()
{

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	CString DefaultDataFolder;

	DefaultDataFolder.Format ("%s\\default_data_folder\\",pMainFrame->m_CurrentDirectory);
	CString directory = m_ProjectDirectory;


	if(m_bDYNASMARTDataSet)
	{
		CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_node_control_type_dsp.csv","input_node_control_type.csv");
		CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_link_type_dsp.csv","input_link_type.csv");
		CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_demand_meta_data_dsp.csv","input_demand_meta_data.csv");

	}else
	{

		CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_node_control_type.csv");
		CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_link_type.csv");

		if(m_ImportedDemandVector.size() >0)  // we have imported data, so we delete the file first
		{
		DeleteFile(m_ProjectDirectory+"input_demand_meta_data.csv");
		}

		if(m_ImportDemandColumnFormat==true)
			CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_demand_meta_data.csv");
		else
			CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_demand_meta_data_matrix.csv","input_demand_meta_data.csv");

	}


	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_demand_type.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_pricing_type.csv");

	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_vehicle_emission_rate.csv");

	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_scenario_settings.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_vehicle_type.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_VOT.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_zone.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_activity_location.csv");

	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_MOE_settings.csv");
	//	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_sensor.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"input_demand.csv");

	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"ms_vehtypes.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"ms_linktypes.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"ms_signal.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"ms_vehclasses.csv");
	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"ODME_settings.txt");

	CopyDefaultFile(DefaultDataFolder,m_ProjectDirectory,directory,"background_image.bmp");

}
BOOL CTLiteDoc::SaveProject(LPCTSTR lpszPathName, int SelectedLayNo = 0)
{

	FILE* st = NULL;
	CString directory;
	CString prj_file = lpszPathName;
	directory = prj_file.Left(prj_file.ReverseFind('\\') + 1);

	CWaitCursor wc;

	CString OldDirectory = m_ProjectDirectory;

	if(OldDirectory!=directory)
	{

		// copy output files
		CopyFile(OldDirectory+"output_LinkMOE.csv", directory+"output_LinkMOE.csv", FALSE);
		CopyFile(OldDirectory+"LinkStaticMOE.csv", directory+"LinkStaticMOE.csv", FALSE);
		CopyFile(OldDirectory+"agent.bin", directory+"agent.bin", FALSE);
		CopyFile(OldDirectory+"output_LinkTDMOE.bin", directory+"output_LinkTDMOE.bin", FALSE);

	//		CopyFile(OldDirectory+"NetworkMOE_1min.csv", directory+"NetworkMOE_1min.csv", FALSE);

		CopyFile(OldDirectory+"demand.dat", directory+"demand.dat", FALSE);
		CopyFile(OldDirectory+"demand_HOV.dat", directory+"demand_HOV.dat", FALSE);
		CopyFile(OldDirectory+"demand_truck.dat", directory+"demand_truck.dat", FALSE);

	}

	// update m_ProjectDirectory
	CopyDefaultFiles();
	m_ProjectDirectory = directory;

	char lpbuffer[64];

	sprintf_s(lpbuffer,"%f",m_NodeDisplaySize);

	WritePrivateProfileString("GUI", "node_display_size",lpbuffer,lpszPathName);
	WritePrivateProfileString("Version", "1.0",lpbuffer,lpszPathName);

	fopen_s(&st,directory+"input_node_control_type.csv","w");
	if(st!=NULL)
	{
		fprintf(st, "control_type_name,unknown_control,no_control,yield_sign,2way_stop_sign,4way_stop_sign,pretimed_signal,actuated_signal,roundabout\n");
		fprintf(st,"control_type");
		fprintf(st,",%d",m_ControlType_UnknownControl);
		fprintf(st,",%d",m_ControlType_NoControl);
		fprintf(st,",%d",m_ControlType_YieldSign);
		fprintf(st,",%d",m_ControlType_2wayStopSign);
		fprintf(st,",%d",m_ControlType_4wayStopSign);
		fprintf(st,",%d",m_ControlType_PretimedSignal);
		fprintf(st,",%d",m_ControlType_ActuatedSignal);
		fprintf(st,",%d",m_ControlType_Roundabout);
		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_node_control_type.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}




	sprintf_s(lpbuffer,"%f",m_NodeDisplaySize);
	WritePrivateProfileString("GUI","node_display_size",lpbuffer,lpszPathName);

	sprintf_s(lpbuffer,"%f",m_NodeTextDisplayRatio);
	WritePrivateProfileString("GUI","node_text_display_ratio",lpbuffer,lpszPathName);

	fopen_s(&st,directory+"input_node.csv","w");
	if(st!=NULL)
	{
		std::list<DTANode*>::iterator iNode;
		fprintf(st, "name,node_id,QEM_reference_node_id,control_type,control_type_name,cycle_length_in_second,signal_offset_in_second,x,y,geometry\n");
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if((*iNode)->m_LayerNo == SelectedLayNo) 
			{

				//if((*iNode)->m_IncomingLinkVector.size()>=3)  // more than 2 movements
				//	(*iNode)->m_ControlType = m_ControlType_PretimedSignal;

				CString control_type_name="";

				if( m_NodeTypeMap.find((*iNode)->m_ControlType)!= m_NodeTypeMap.end())				{
					control_type_name = m_NodeTypeMap[(*iNode)->m_ControlType].c_str() ;
				}

				if((*iNode)->m_ControlType != m_ControlType_PretimedSignal && (*iNode)->m_ControlType != m_ControlType_ActuatedSignal)
				{
					(*iNode)->m_CycleLengthInSecond = 0;
					(*iNode)->m_SignalOffsetInSecond = 0;
				}

				std::replace( (*iNode)->m_Name.begin(), (*iNode)->m_Name.end(), ',', ' '); 

				fprintf(st, "%s,%d,%d,%d,%s,%d,%d,%f,%f,\"<Point><coordinates>%f,%f</coordinates></Point>\"\n", 
					(*iNode)->m_Name.c_str (), (*iNode)->m_NodeNumber , (*iNode)->m_QEM_ReferenceNodeNumber , (*iNode)->m_ControlType,   control_type_name, 
					(*iNode)->m_CycleLengthInSecond ,
					(*iNode)->m_SignalOffsetInSecond  ,
					(*iNode)->pt .x, (*iNode)->pt .y,(*iNode)->pt .x, (*iNode)->pt .y);
			}
		}

		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_node.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}


	m_bExport_Link_MOE_in_input_link_CSF_File = false;
	fopen_s(&st,directory+"input_link.csv","w");
	if(st!=NULL)
	{
		std::list<DTALink*>::iterator iLink;
		fprintf(st,"name,link_id,TMC,from_node_id,to_node_id,link_type_name,direction,length_in_mile,number_of_lanes,speed_limit_in_mph,saturation_flow_rate_in_vhc_per_hour_per_lane,lane_capacity_in_vhc_per_hour,link_type,jam_density_in_vhc_pmpl,wave_speed_in_mph,effective_green_time_length_in_second,green_start_time_in_second,AADT_conversion_factor,mode_code,grade,geometry,original_geometry,");
		fprintf(st,"transit_travel_time_in_min,transit_transfer_time_in_min,transit_waiting_time_in_min,transit_fare_in_dollar,BPR_alpha_term,BPR_beta_term,");
		fprintf(st,"KML_green_height,KML_red_height,KML_blue_height,KML_yellow_height,");


		// ANM output
		fprintf(st,"number_of_left_turn_lanes,length_of_bays_in_feet,number_of_right_turn_lanes,from_approach,to_approach,reversed_link_id,");

		if(m_bExport_Link_MOE_in_input_link_CSF_File)  // save time-dependent MOE
		{
			int hour;
			for(hour =0; hour <= 23; hour++)
			{
				CString str_MOE_hour;
				str_MOE_hour.Format ("h%d_link_volume,",hour);
				fprintf(st,str_MOE_hour);
			}

			for(hour =0; hour <= 23; hour++)
			{
				CString str_MOE_hour;
				str_MOE_hour.Format ("h%d_speed,",hour);
				fprintf(st,str_MOE_hour);
			}

			for(hour =0; hour <= 23; hour++)
			{
				CString str_MOE_hour;
				str_MOE_hour.Format ("h%d_travel_time,",hour);
				fprintf(st,str_MOE_hour);
			}
		}

		//		 safety prediction attributes
		fprintf(st,"group_1_code,group_2_code,group_3_code,num_driveways_per_mile,volume_proportion_on_minor_leg,num_3SG_intersections,num_3ST_intersections,num_4SG_intersections,num_4ST_intersections");
		fprintf(st,"\n");	

		for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			if((*iLink)->m_LayerNo == SelectedLayNo && (*iLink)->m_FromNodeID != (*iLink)->m_ToNodeID)
			{
				int ToNodeID = (*iLink)->m_ToNodeID ;
				DTANode* pNode = m_NodeIDMap[ToNodeID];
				//set default green time 
				if(pNode->m_ControlType == m_ControlType_PretimedSignal || 
					pNode->m_ControlType == m_ControlType_ActuatedSignal)
				{

					if((*iLink)->m_EffectiveGreenTimeInSecond ==0)  // no default value
					{
						// from given BPR capacity to determine the effective green time
						(*iLink)->m_EffectiveGreenTimeInSecond = (int)(pNode->m_CycleLengthInSecond * (*iLink)->m_LaneCapacity / (*iLink)->m_Saturation_flow_rate_in_vhc_per_hour_per_lane);

						if((*iLink)->m_EffectiveGreenTimeInSecond > pNode->m_CycleLengthInSecond )
							(*iLink)->m_EffectiveGreenTimeInSecond = pNode->m_CycleLengthInSecond ;


					}

				}


				CString link_type_name = " ";

				if(m_LinkTypeMap.find((*iLink)->m_link_type) != m_LinkTypeMap.end())
				{
					link_type_name = m_LinkTypeMap[(*iLink)->m_link_type].link_type_name.c_str ();
				}

				int sign_flag = 1;
				if(SelectedLayNo == 1)  // external layer
					sign_flag = -1;

				std::replace( (*iLink)->m_Name.begin(), (*iLink)->m_Name.end(), ',', ' '); 

				fprintf(st,"%s,%d,%s,%d,%d,%s,%d,%.5f,%d,%.1f,%.1f,%.1f,%d,%.1f,%.1f,%d,%d,%.3f,\"%s\",%.1f,",
					(*iLink)->m_Name.c_str (),
					(*iLink)->m_LinkID, 
					(*iLink)->m_TMC_code .c_str (),

					(*iLink)->m_FromNodeNumber*sign_flag, 
					(*iLink)->m_ToNodeNumber*sign_flag, 
					link_type_name,
					(*iLink)->m_Direction,(*iLink)->m_Length ,(*iLink)->m_NumberOfLanes ,
					(*iLink)->m_SpeedLimit,
					(*iLink)->m_Saturation_flow_rate_in_vhc_per_hour_per_lane,
					(*iLink)->m_LaneCapacity ,(*iLink)->m_link_type,(*iLink)->m_Kjam, (*iLink)->m_Wave_speed_in_mph, 
					(*iLink)->m_EffectiveGreenTimeInSecond, (*iLink)->m_GreenStartTimetInSecond,
					(*iLink)->m_AADT_conversion_factor ,(*iLink)->m_Mode_code.c_str (), (*iLink)->m_Grade);

				// geometry
				fprintf(st,"\"<LineString><coordinates>");

				for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
				{
					fprintf(st,"%f,%f,0.0",(*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y);

					if(si!=(*iLink)->m_ShapePoints.size()-1)
						fprintf(st," ");
				}

				fprintf(st,"</coordinates></LineString>\",");

				// original_geometry
				fprintf(st,"\"<LineString><coordinates>");

				for(unsigned int si = 0; si< (*iLink)->m_Original_ShapePoints.size(); si++)
				{
					fprintf(st,"%f,%f,0.0",(*iLink)->m_Original_ShapePoints[si].x, (*iLink)->m_Original_ShapePoints[si].y);

					if(si!=(*iLink)->m_Original_ShapePoints.size()-1)
						fprintf(st," ");
				}

				fprintf(st,"</coordinates></LineString>\",");

				fprintf(st,"%.1f,%.1f,%.1f,%.2f,%.4f,%.4f,",
					(*iLink)->m_TransitTravelTime, (*iLink)->m_TransitTransferTime,(*iLink)->m_TransitWaitingTime, (*iLink)->m_TransitFareInDollar, (*iLink)->m_BPR_alpha_term, (*iLink)->m_BPR_beta_term);
				// mobility and reliability
				fprintf(st,"%d,%d,%d,%d,",(*iLink)->green_height, (*iLink)->red_height,(*iLink)->blue_height, (*iLink)->yellow_height);

				// ANM output
				unsigned long ReversedLinkKey = GetLinkKey((*iLink)->m_ToNodeID, (*iLink)->m_FromNodeID);
				int reversed_link_id = 0;
				if ( m_NodeIDtoLinkMap.find ( ReversedLinkKey) != m_NodeIDtoLinkMap.end())
				{
					DTALink * pLine  = m_NodeIDtoLinkMap[ReversedLinkKey];
					if(pLine!= NULL)
						reversed_link_id = pLine -> m_LinkID ;
				}


				fprintf(st,"%d,%d,%d,%c,%c,%d,",(*iLink)->m_NumberOfLeftTurnLanes,(*iLink)->m_LeftTurnBayLengthInFeet, (*iLink)->m_NumberOfRightTurnLanes,
					GetApproachChar((*iLink)->m_FromApproach), GetApproachChar((*iLink)->m_ToApproach),reversed_link_id);

				fprintf(st,"%s,%s,%s,", (*iLink) -> group_1_code.c_str(),(*iLink)->  group_2_code.c_str(),(*iLink) -> group_3_code.c_str());

				if(m_bExport_Link_MOE_in_input_link_CSF_File)  // save time-dependent MOE
				{ int hour;
				for(hour =0; hour <= 23; hour++)
				{ 
					fprintf(st,"%f,", (*iLink)->GetAvgLinkHourlyVolume(hour*60,(hour+1)*60));
				}

				for(hour =0; hour <= 23; hour++)
				{ 
					float avg_travel_time = (*iLink)->m_Length *60/max (1,(*iLink)->GetAvgLinkSpeed (hour*60,(hour+1)*60));
					fprintf(st,"%.2f,", (*iLink)->GetAvgLinkSpeed (hour*60,(hour+1)*60));
				}
				for(hour =0; hour <= 23; hour++)
				{ 
					float avg_travel_time = (*iLink)->m_Length *60/max (1,(*iLink)->GetAvgLinkSpeed (hour*60,(hour+1)*60));
					fprintf(st,"%.2f,", avg_travel_time );
				}
				}



				fprintf(st,"%5.1f,%5.1f,%5.1f,%5.1f,%5.1f,%5.1f", 
					(*iLink)->m_Num_Driveways_Per_Mile,
					(*iLink)->m_volume_proportion_on_minor_leg,
					(*iLink)->m_Num_3SG_Intersections,
					(*iLink)->m_Num_3ST_Intersections,
					(*iLink)->m_Num_4SG_Intersections,
					(*iLink)->m_Num_4ST_Intersections);

				fprintf(st,"\n");
			}  // layer no ==0
		}

		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_link.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}


	CCSVWriter MovementFile;

	CString movement_str = directory+"AMS_movement.csv";

	// Convert a TCHAR string to a LPCSTR
	CT2CA pszConvertedAnsiString (movement_str);

	// construct a std::string using the LPCSTR input
	std::string strStd (pszConvertedAnsiString);

	if(MovementFile.Open(strStd))
	{


		MovementFile.SetFieldName ("node_id");
		MovementFile.SetFieldName ("QEM_reference_node_id");
		MovementFile.SetFieldName ("up_node_id");
		MovementFile.SetFieldName ("dest_node_id");
		MovementFile.SetFieldName ("name");
		MovementFile.SetFieldName ("turn_type");
		MovementFile.SetFieldName ("turn_direction");

		MovementFile.SetFieldName ("prohibitted_flag");
		MovementFile.SetFieldName ("protected_flag");
		MovementFile.SetFieldName ("permitted_flag");

		MovementFile.SetFieldName ("sim_turn_volume");
		MovementFile.SetFieldName ("sim_turn_hourly_volume");
		MovementFile.SetFieldName ("sim_turn_percentage");
		MovementFile.SetFieldName ("sim_turn_delay_in_second");

		MovementFile.SetFieldName ("obs_turn_volume");
		MovementFile.SetFieldName ("obs_turn_hourly_volume");
		MovementFile.SetFieldName ("obs_turn_percentage");
		MovementFile.SetFieldName ("obs_turn_delay_in_second");


		MovementFile.SetFieldName ("QEM_TurnDirection");
		MovementFile.SetFieldName ("QEM_TurnVolume");
		MovementFile.SetFieldName ("QEM_LinkVolume");
		MovementFile.SetFieldName ("QEM_Lanes");
		MovementFile.SetFieldName ("QEM_Shared");
		MovementFile.SetFieldName ("QEM_Width");
		MovementFile.SetFieldName ("QEM_Storage");
		MovementFile.SetFieldName ("QEM_StLanes");
		MovementFile.SetFieldName ("QEM_Grade");
		MovementFile.SetFieldName ("QEM_Speed");
		MovementFile.SetFieldName ("QEM_IdealFlow");
		MovementFile.SetFieldName ("QEM_LostTime");
		MovementFile.SetFieldName ("QEM_Phase1");
		MovementFile.SetFieldName ("QEM_PermPhase1");
		MovementFile.SetFieldName ("QEM_DetectPhase1");

		MovementFile.SetFieldName ("QEM_EffectiveGreen");
		MovementFile.SetFieldName ("QEM_Capacity");
		MovementFile.SetFieldName ("QEM_VOC");
		MovementFile.SetFieldName ("QEM_SatFlow");
		MovementFile.SetFieldName ("QEM_Delay");
		MovementFile.SetFieldName ("QEM_LOS");

		MovementFile.WriteHeader();

		std::list<DTANode*>::iterator iNode;
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if((*iNode)->m_LayerNo == SelectedLayNo) 
			{
				for(unsigned int m = 0; m< (*iNode)->m_MovementVector .size(); m++)
				{


					DTANodeMovement movement = (*iNode)->m_MovementVector[m];

					MovementFile.SetValueByFieldName  ("node_id",(*iNode)->m_NodeNumber);

					MovementFile.SetValueByFieldName  ("name",(*iNode)->m_Name);

					MovementFile.SetValueByFieldName ("turn_type",GetTurnString(movement.movement_turn));

					MovementFile.SetValueByFieldName ("turn_direction",GetTurnDirectionString(movement.movement_dir));


					//if nodes have been cut outside the network, so we do not need save them
					if(m_NodeIDMap.find (movement.in_link_from_node_id) == m_NodeIDMap.end())
						continue;

					if(m_NodeIDMap.find (movement.out_link_to_node_id) == m_NodeIDMap.end())
						continue;


					int up_node_id = m_NodeIDMap[movement.in_link_from_node_id]->m_NodeNumber  ;


					MovementFile.SetValueByFieldName ("up_node_id",up_node_id);
					int dest_node_id = m_NodeIDMap[movement.out_link_to_node_id ]->m_NodeNumber ;
					MovementFile.SetValueByFieldName ("dest_node_id",dest_node_id);

					MovementFile.SetValueByFieldName ("prohibitted_flag",movement.turning_prohibition_flag);
					MovementFile.SetValueByFieldName ("protected_flag",movement.turning_prohibition_flag);
					MovementFile.SetValueByFieldName ("permitted_flag",movement.turning_prohibition_flag);

					MovementFile.SetValueByFieldName ("sim_turn_volume",movement.sim_turn_count );
					MovementFile.SetValueByFieldName ("sim_turn_hourly_volume",movement.sim_turn_hourly_count );
					MovementFile.SetValueByFieldName ("sim_turn_percentage",movement.turning_percentage);
					MovementFile.SetValueByFieldName ("sim_turn_delay_in_second",movement.sim_turn_delay );

					MovementFile.SetValueByFieldName ("obs_turn_volume",movement.obs_turn_delay );
					MovementFile.SetValueByFieldName ("obs_turn_hourly_volume",movement.obs_turn_delay );
					MovementFile.SetValueByFieldName ("obs_turn_percentage",movement.obs_turn_delay );
					MovementFile.SetValueByFieldName ("obs_turn_delay_in_second",movement.obs_turn_delay );

					MovementFile.SetValueByFieldName ("QEM_TurnDirection", movement.QEM_dir_string );
					MovementFile.SetValueByFieldName ("QEM_TurnVolume",movement.QEM_TurnVolume );


					MovementFile.SetValueByFieldName ("QEM_LinkVolume", movement.QEM_LinkVolume);

					MovementFile.SetValueByFieldName ("QEM_Lanes",movement.QEM_Lanes );
					MovementFile.SetValueByFieldName ("QEM_Shared",movement.QEM_Shared );
					MovementFile.SetValueByFieldName ("QEM_Width",movement.QEM_Width );
					MovementFile.SetValueByFieldName ("QEM_Storage",movement.QEM_Storage );
					MovementFile.SetValueByFieldName ("QEM_StLanes",movement.QEM_StLanes );
					MovementFile.SetValueByFieldName ("QEM_Grade",movement.QEM_Grade );
					MovementFile.SetValueByFieldName ("QEM_Speed",movement.QEM_Speed );
					MovementFile.SetValueByFieldName ("QEM_IdealFlow",movement.QEM_IdealFlow );
					MovementFile.SetValueByFieldName ("QEM_LostTime",movement.QEM_LostTime );
					MovementFile.SetValueByFieldName ("QEM_Phase1",movement.QEM_Phase1 );
					MovementFile.SetValueByFieldName ("QEM_PermPhase1",movement.QEM_PermPhase1 );
					MovementFile.SetValueByFieldName ("QEM_DetectPhase1",movement.QEM_DetectPhase1 );

					MovementFile.SetValueByFieldName ("QEM_EffectiveGreen",movement.QEM_EffectiveGreen );
					MovementFile.SetValueByFieldName ("QEM_Capacity",movement.QEM_Capacity );
					MovementFile.SetValueByFieldName ("QEM_VOC",movement.QEM_VOC );
					MovementFile.SetValueByFieldName ("QEM_SatFlow",movement.QEM_SatFlow );
					MovementFile.SetValueByFieldName ("QEM_Delay",movement.QEM_Delay );

					CT2CA pszConvertedAnsiString (movement.QEM_LOS);
					// construct a std::string using the LPCSTR input
					std::string strStd (pszConvertedAnsiString);

					MovementFile.SetValueByFieldName ("QEM_LOS", strStd);

					MovementFile.WriteRecord ();

				}
			}
		}

	}
	//fopen_s(&st,directory+"input_phase.csv","w");
	//if(st!=NULL)
	//{
	//	std::list<DTANode*>::iterator iNode;
	//	fprintf(st, "node_name,node_id,phase_index,incoming_link_from_node_id,outgoing_link_to_node_id,turning_direction\n");
	//	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	//	{
	//		if((*iNode)->m_LayerNo == SelectedLayNo) 
	//		{
	//			for(unsigned int p = 0; p< (*iNode)->m_PhaseVector .size(); p++)
	//			{
	//				for(unsigned int m = 0; m< (*iNode)->m_PhaseVector[p].movement_index_vector.size() ; m++)
	//				{
	//					int movement_index = (*iNode)->m_PhaseVector[p].movement_index_vector[m];;
	//					DTANodeMovement movement = (*iNode)->m_MovementVector[movement_index];


	//					fprintf(st,"%s,%d,%d,%d,%d,%s\n",(*iNode)->m_Name.c_str (), 
	//						(*iNode)->m_NodeNumber ,
	//						p+1,
	//						m_NodeIDtoNumberMap[movement.in_link_from_node_id],
	//						m_NodeIDtoNumberMap[movement.out_link_to_node_id ],
	//						GetTurnString(movement.movement_turn));
	//				}
	//			}
	//		}
	//	}

	//	fclose(st);
	//}else
	//{
	//	AfxMessageBox("Error: File input_phase.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
	//	return false;
	//}
	if(m_bLoadNetworkDataOnly)  // only network data are loaded, no need to save the other data.
		return true; 

	// save zone here

	fopen_s(&st,directory+"input_zone.csv","w");
	if(st!=NULL)
	{
		fprintf(st,"zone_id,production,attraction,origin_num_of_veh,origin_avg_distance,destination_num_of_veh,destination_avg_distance,color_code,height,notes,geometry\n");

		std::map<int, DTAZone>	:: const_iterator itr;

		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)
		{
			if(itr->second.m_bWithinSubarea && itr->first>=1)
			{
				//create activity location for empty zones

				if(itr->first == 1065)
				{
				TRACE("");
				}

				fprintf(st, "%d,%f,%f,%d,%f,%d,%f,%s,%f,%s,", 
					itr->first,
					itr->second .m_Production ,
					itr->second .m_Attraction, 
					itr->second .m_OriginTotalNumberOfVehicles , itr->second .m_OriginTotalTravelDistance /max(1,itr->second .m_OriginTotalNumberOfVehicles),
					itr->second .m_DestinationTotalNumberOfVehicles , itr->second .m_DestinationTotalTravelDistance /max(1,itr->second .m_DestinationTotalNumberOfVehicles),
					itr->second .color_code.c_str () , 
					itr->second.m_Height,
					itr->second .notes.c_str ());

				fprintf(st,"\"<Polygon><outerBoundaryIs><LinearRing><coordinates>");
				for(unsigned int si = 0; si< itr->second.m_ShapePoints.size(); si++)
				{
					fprintf(st,"%f,%f,0.0",itr->second.m_ShapePoints[si].x, itr->second.m_ShapePoints[si].y);

					if(si!=itr->second.m_ShapePoints.size()-1)
						fprintf(st," ");
				}
				fprintf(st,"\"</coordinates></LinearRing></outerBoundaryIs></Polygon>\"");
				fprintf(st, "\n");
			}
		}

		fclose(st);

	}else
	{
		AfxMessageBox("Error: File input_zone.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}

	fopen_s(&st,directory+"input_activity_location.csv","w");
	if(st!=NULL)
	{
		fprintf(st,"zone_id,node_id,external_OD_flag\n");

		std::map<int, DTAZone>	:: iterator itr;

		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); ++itr)
		{
			for(int i = 0; i< itr->second.m_ActivityLocationVector .size(); i++)
			{
				DTAActivityLocation element = itr->second.m_ActivityLocationVector[i];
				fprintf(st, "%d,%d,%d\n", element.ZoneID , element.NodeNumber , element.External_OD_flag );
			}

		}
		fclose(st);

	}else
	{
		AfxMessageBox("Error: File input_activity_location.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}


	fopen_s(&st,directory+"input_od_pairs.csv","w");
	if(st!=NULL)
	{
		fprintf(st,"from_zone_id,to_zone_id,from_node_id,to_node_id\n");

		std::map<int, DTAZone>	:: iterator itr;
		std::map<int, DTAZone>	:: iterator itr2;

		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)  // from zone
		{

					for(itr2 = m_ZoneMap.begin(); itr2 != m_ZoneMap.end(); itr2++)  // to zone
					{

						if(itr->second.m_ActivityLocationVector .size() >0  && itr2->second.m_ActivityLocationVector .size() >0)
						{
						DTAActivityLocation element_from = itr->second.m_ActivityLocationVector[0];
						DTAActivityLocation element_to = itr2->second.m_ActivityLocationVector[0];
						fprintf(st, "%d,%d,%d,%d\n", element_from.ZoneID,element_to.ZoneID, element_from.NodeNumber,element_to.NodeNumber);
						}

					}			
			
			

		}
		fclose(st);

	}else
	{
		AfxMessageBox("Error: File input_od_pairs.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}

	// save demand here
	fopen_s(&st,directory+"input_subarea.csv","w");
	if(st!=NULL)
	{
		fprintf(st,"feature_id,x,y\n");
		for (unsigned int sub_i= 0; sub_i < m_SubareaShapePoints.size(); sub_i++)
		{
			fprintf(st, "%d,%f,%f\n", sub_i, m_SubareaShapePoints[sub_i].x, m_SubareaShapePoints[sub_i].y);
		}


		fclose(st);

	}else
	{
		AfxMessageBox("Error: File input_subarea.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}


	if(m_VehicleTypeVector.size()==0)  // no data available, use default values
	{

		DTAVehicleType element;
		element.vehicle_type = 1; element.vehicle_type_name = "passenger car";  m_VehicleTypeVector.push_back (element);
		element.vehicle_type = 2; element.vehicle_type_name = "passenger truck";  m_VehicleTypeVector.push_back (element);
		element.vehicle_type = 3; element.vehicle_type_name = "light commercial truck";  m_VehicleTypeVector.push_back (element);
		element.vehicle_type = 4; element.vehicle_type_name = "single unit long-haul truck";  m_VehicleTypeVector.push_back (element);
		element.vehicle_type = 5; element.vehicle_type_name = "combination long-haul truck";  m_VehicleTypeVector.push_back (element);

	}

	//// save vehicle type info here  : xuesong: do not overwrite the vehicle type information, it is editable by users now
	//fopen_s(&st,directory+"input_vehicle_type.csv","w");
	//if(st!=NULL)
	//{

	//	fprintf(st,"vehicle_type,vehicle_type_name\n");
	//	for(std::vector<DTAVehicleType>::iterator itr = m_VehicleTypeVector.begin(); itr != m_VehicleTypeVector.end(); ++itr)
	//	{
	//		(*itr).vehicle_type_name.Replace (",", " ");
	//		{
	//			fprintf(st, "%d,%s\n", (*itr).vehicle_type , (*itr).vehicle_type_name);
	//		}

	//	}

	//	fclose(st);
	//}else
	//{
	//	AfxMessageBox("Error: File input_vehicle_type.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
	//	return false;
	//}


	//// save demand type info here
	//fopen_s(&st,directory+"input_demand_type.csv","w");
	//if(st!=NULL)
	//{
	//	fprintf(st,"demand_type,demand_type_name,average_VOT,pricing_type,percentage_of_pretrip_info,percentage_of_enroute_info,");


	//	unsigned int i;
	//	for( i=0; i< m_VehicleTypeVector.size(); i++)
	//	{
	//		CString str_percentage_of_vehicle_type; 
	//		fprintf (st,"percentage_of_vehicle_type%d,", i+1);
	//	}
	//	fprintf (st,"\n");

	//	if(m_DemandTypeVector.size()==0)  // no data available, use default values
	//	{
	//		DTADemandType element;
	//		element.demand_type =1;
	//		element.demand_type_name = "SOV";
	//		element.pricing_type = 1;
	//		element.average_VOT = 10;
	//		m_DemandTypeVector.push_back(element);

	//		element.demand_type =2;
	//		element.pricing_type = 2;
	//		element.demand_type_name = "HOV";
	//		element.average_VOT = 10;
	//		m_DemandTypeVector.push_back(element);

	//		element.demand_type =3;
	//		element.pricing_type = 3;
	//		element.demand_type_name = "truck";
	//		m_DemandTypeVector.push_back(element);

	//		element.demand_type =4;
	//		element.pricing_type = 4;
	//		element.demand_type_name = "intermodal";
	//		m_DemandTypeVector.push_back(element);

	//	}


	//	for(std::vector<DTADemandType>::iterator itr = m_DemandTypeVector.begin(); itr != m_DemandTypeVector.end(); ++itr)
	//	{
	//		(*itr).demand_type_name.Replace (",", " ");
	//		{
	//			fprintf(st, "%d,%s,%5.3f,%d,%5.3f,%5.3f,", (*itr).demand_type , (*itr).demand_type_name, (*itr).average_VOT, (*itr).pricing_type , (*itr).info_class_percentage[1], (*itr).info_class_percentage[2]);
	//		}

	//		for(i=0; i< m_VehicleTypeVector.size(); i++)
	//		{
	//			fprintf (st,"%5.3f,", (*itr).vehicle_type_percentage [i]);
	//		}
	//		fprintf (st,"\n");
	//	}

	//	fclose(st);
	//}else
	//{
	//	AfxMessageBox("Error: File input_demand_type.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
	//	return false;
	//}
	// save link type info here
	fopen_s(&st,directory+"input_link_type.csv","w");
	if(st!=NULL)
	{
		fprintf(st,"link_type,link_type_name,type_code,default_lane_capacity,default_speed_limit,default_number_of_lanes\n");
		for(std::map<int, DTALinkType>::iterator itr = m_LinkTypeMap.begin(); itr != m_LinkTypeMap.end(); itr++)
		{
			std::replace( itr->second .link_type_name.begin(), itr->second .link_type_name.end(), ',', ' '); 

			if(itr->second .link_type_name.length () > 0 && itr->second .type_code.length () > 0)
			{

				fprintf(st, "%d,%s,%s,%f,%.2f,%d\n", itr->first , 
					itr->second .link_type_name.c_str () , 
					itr->second .type_code .c_str (),
					itr->second .default_lane_capacity,
					itr->second .default_speed,
					itr->second .default_number_of_lanes );
			}
		}

		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_link_type.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}



	SaveSubareaDemandFile();


	// save demand if a matrix has been imported 
	if(m_ImportedDemandVector.size()>0)
	{

		CopyDefaultFiles(); // include input_demand_meta-database, input_vehicle type... 

		// 3 column format
		if(m_ImportDemandColumnFormat == true)
		{
		fopen_s(&st,directory+"input_demand.csv","w");
		if(st!=NULL)
		{
			fprintf(st,"from_zone_id,to_zone_id,number_of_vehicles\n");

			for(unsigned int i = 0; i < m_ImportedDemandVector.size(); i++)
			{
			fprintf(st,"%d,%d,%f\n", m_ImportedDemandVector[i].from_zone_id,  m_ImportedDemandVector[i].to_zone_id ,  m_ImportedDemandVector[i].number_of_vehicles  );
			}
			fclose(st);

		}else
		{
			AfxMessageBox("Error: File input_demand.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
			return false;
		}
		fclose(st);
		}else
		{
		fopen_s(&st,directory+"input_demand.csv","w");
		if(st!=NULL)
		{
			fprintf(st,"zone_id");
		std::map<int, DTAZone>	:: const_iterator itr;

		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)
		{
			fprintf(st,",%d",itr->first );
		}


			fprintf(st,"\n");

		std::map<int, DTAZone>	:: const_iterator itr_to_zone_id;

		int index = 0;
		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)
		{
			fprintf(st,"%d,",itr->first );
			
			for(itr_to_zone_id = m_ZoneMap.begin(); itr_to_zone_id != m_ZoneMap.end(); itr_to_zone_id++)
			{
				if(index< m_ImportedDemandVector.size())
				{
					fprintf(st,"%f,", m_ImportedDemandVector[index++].number_of_vehicles );
				}
			}

					fprintf(st,"\n");

		}

		fclose(st);

		}else
		{
			AfxMessageBox("Error: File input_demand.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
			return false;
		
		}

		
		}


	}

	OnFileSaveimagelocation();

	WriteLink_basedTollScenarioData();
	WriteVMSScenarioData();
	WriteIncidentScenarioData();
	WriteWorkZoneScenarioData();


	return true;
}
void CTLiteDoc::OnFileSaveProject()
{
	if(theApp.m_VisulizationTemplate == e_train_scheduling)
		return;

	if(m_ProjectDirectory.GetLength ()>3 && m_ProjectFile.GetLength()>3)
		SaveProject(m_ProjectFile);
	else
		OnFileSaveProjectAs();
}

void CTLiteDoc::OnFileSaveProjectAs()
{
	try{
	CFileDialog fdlg (FALSE, "*.tnp", "*.tnp",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_LONGNAMES,
		"Transportation Network Project (*.tnp)|*.tnp|");

	
	if(fdlg.DoModal()==IDOK)
	{
		CString path = fdlg.GetFileName ();
		CWaitCursor wait;
		m_ProjectFile = fdlg.GetPathName();
		m_ProjectTitle = GetWorkspaceTitleName(m_ProjectFile);
		if(SaveProject(fdlg.GetPathName()))
		{
			CString msg;
			msg.Format ("Files input_node.csv, input_link.csv and input_zone.csv have been successfully saved with %d nodes, %d links, %d zones.",m_NodeSet.size(), m_LinkSet.size(), m_ZoneMap.size());
			AfxMessageBox(msg,MB_OK|MB_ICONINFORMATION);

			SetTitle(m_ProjectTitle);


		}
	}

	} catch (char *str  ) 
	{
	AfxMessageBox(str,MB_ICONINFORMATION);
	}
	// TODO: Add your command handler code here
}

void CTLiteDoc::CalculateDrawingRectangle(bool NodeLayerOnly)
{

	std::list<DTALink*>::iterator iLink;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->CalculateShapePointRatios();

	}

	bool bRectInitialized = false;
	m_AdjLinkSize = 0;

	for (std::list<DTANode*>::iterator iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		if(NodeLayerOnly || (*iNode)->m_Connections >0 )  // we might try import node layer only from shape file, so all nodes have no connected links. 
		{
			if(!bRectInitialized)
			{
				m_NetworkRect.left = (*iNode)->pt.x ;
				m_NetworkRect.right = (*iNode)->pt.x;
				m_NetworkRect.top = (*iNode)->pt.y;
				m_NetworkRect.bottom = (*iNode)->pt.y;
				bRectInitialized = true;
			}

			if((*iNode)->m_Connections > m_AdjLinkSize)
				m_AdjLinkSize = (*iNode)->m_Connections;

			m_NetworkRect.Expand((*iNode)->pt);
		}

	}

	for (std::list<DTAPoint*>::iterator iPoint = m_DTAPointSet.begin(); iPoint != m_DTAPointSet.end(); iPoint++)
	{
		if(!bRectInitialized)
		{
			m_NetworkRect.left = (*iPoint)->pt.x ;
			m_NetworkRect.right = (*iPoint)->pt.x;
			m_NetworkRect.top = (*iPoint)->pt.y;
			m_NetworkRect.bottom = (*iPoint)->pt.y;
			bRectInitialized = true;
		}

		m_NetworkRect.Expand((*iPoint)->pt);

	}

	if(m_BackgroundBitmapLoaded)  // if there is a bitmap background
	{

		if(!bRectInitialized)  // there is no node or link layer
		{
			m_NetworkRect.left = m_ImageX1;
			m_NetworkRect.right = m_ImageX2;
			m_NetworkRect.top = m_ImageY2;
			m_NetworkRect.bottom = m_ImageY1;
			bRectInitialized = true;

		}
			
	}

	for (std::list<DTALine*>::iterator iLine = m_DTALineSet.begin(); iLine != m_DTALineSet.end(); iLine++)
	{
		for(unsigned int i = 0; i< (*iLine)->m_ShapePoints .size(); i++)
		{
			if(!bRectInitialized)
			{
				m_NetworkRect.left = (*iLine)->m_ShapePoints[i].x ;
				m_NetworkRect.right = (*iLine)->m_ShapePoints[i].x;
				m_NetworkRect.top = (*iLine)->m_ShapePoints[i].y;
				m_NetworkRect.bottom = (*iLine)->m_ShapePoints[i].y;

				bRectInitialized = true;
			}else
			{
				m_NetworkRect.Expand((*iLine)->m_ShapePoints[i]);
			}
		}

	}

	if(theApp.m_VisulizationTemplate == e_train_scheduling)
	{

		CString str;
		str.Format("%d nodes and %d arcs are loaded.", m_NodeSet.size(), m_LinkSet.size());
		AfxMessageBox(str,MB_ICONINFORMATION);
	}


}


bool CTLiteDoc::WriteSelectVehicleDataToCSVFile(LPCTSTR lpszFileName, std::vector<DTAVehicle*> VehicleVector)
{

	// vehicle_id,  origin_zone_id, destination_zone_id, departure_time,
	//	arrival_time, complete_flag, trip_time, demand_type, occupancy, information_type,
	//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
	//	node id, node arrival time

	ofstream VehicleDataFile;
	VehicleDataFile.open (lpszFileName, ios::out);
	if (VehicleDataFile.is_open())
	{
		VehicleDataFile.width(12);
		VehicleDataFile.precision(4) ;
		VehicleDataFile.setf(ios::fixed);
		VehicleDataFile << "vehicle_id,origin_zone_id,destination_zone_id,departure_time, trip_time, complete_flag, demand_type,pricing_type,vehicle_type, information_class,value_of_time, toll_dollar_cost,emissions,distance_in_mile, number_of_nodes, <node id;node arrival time>" << endl;

		for(int v = 0; v < VehicleVector.size(); v++)
		{

			DTAVehicle* pVehicle = VehicleVector[v] ;

			if(pVehicle!=NULL)
			{
				VehicleDataFile << pVehicle->m_VehicleID << ",";
				VehicleDataFile << pVehicle->m_OriginZoneID << ",";
				VehicleDataFile << pVehicle->m_DestinationZoneID << ",";
				VehicleDataFile << pVehicle->m_DepartureTime << ",";
				VehicleDataFile << pVehicle->m_TripTime << ",";
				if(pVehicle->m_bComplete == 0)
					VehicleDataFile << "0,";
				else
					VehicleDataFile << "1,";

				VehicleDataFile << pVehicle->m_DemandType << ",";
				VehicleDataFile << pVehicle->m_PricingType << ",";
				VehicleDataFile << pVehicle->m_VehicleType << ",";
				VehicleDataFile << pVehicle->m_InformationClass << ",";
				VehicleDataFile << pVehicle->m_VOT << ",";
				VehicleDataFile << pVehicle->m_TollDollarCost << ",";
				VehicleDataFile << pVehicle->m_Emissions << ",";
				VehicleDataFile << pVehicle->m_Distance << ",";
				VehicleDataFile << pVehicle->m_NodeSize << ",\"";

				//for(int i=0; i< pVehicle->m_NodeSize-1; i++)
				//{
				//	VehicleDataFile << "<"; 

				//	if(pVehicle->m_NodeAry[i].LinkNo!=-1 && m_LinkNoMap.find (pVehicle->m_NodeAry[i].LinkNo) != m_LinkNoMap.end())
				//	{
				//	DTALink* pLink = m_LinkNoMap[pVehicle->m_NodeAry[i].LinkNo];
				//
				//	VehicleDataFile << pLink->m_FromNodeNumber << ";";
				//	VehicleDataFile << pVehicle->m_NodeAry[i].ArrivalTimeOnDSN << ">";
				//	}
				//}
				//VehicleDataFile << pVehicle->m_NodeSize << "\"";
				VehicleDataFile << endl;
			}
		}
		VehicleDataFile.close();
		return true;
	}

	return false;
}
void CTLiteDoc::ReadVehicleCSVFile_Parser(LPCTSTR lpszFileName)
{

	//   cout << "Read vehicle file... "  << endl;
	// vehicle_id,  origin_zone_id, destination_zone_id, departure_time,
	//	arrival_time, complete_flag, trip_time, demand_type, occupancy, information_type,
	//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
	//	node id, node arrival time

	float LengthinMB;
	FILE* pFile;
	fopen_s(&pFile,lpszFileName,"rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);
		LengthinMB= Length*1.0/1024/1024;
		if(LengthinMB>50)
		{
			CString msg;
			msg.Format("The Vehicle.csv file is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load the vehicle file?",LengthinMB);
			if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
				return;
		}
	}

	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");
	if(st!=NULL)
	{
		m_VehicleSet.clear();
		int count = 0;
		while(!feof(st))
		{
			int m_VehicleID= g_read_integer(st);
			if(m_VehicleID == -1)
				break;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_VehicleID		= m_VehicleID;
			pVehicle->m_OriginZoneID	= g_read_integer(st);
			pVehicle->m_DestinationZoneID =g_read_integer(st);
			pVehicle->m_DepartureTime	=  g_read_float(st);
			pVehicle->m_ArrivalTime =  g_read_float(st);

			if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
				g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

			int CompleteFlag = g_read_integer(st);
			if(CompleteFlag==0) 
				pVehicle->m_bComplete = false;
			else
				pVehicle->m_bComplete = true;

			pVehicle->m_TripTime  = g_read_float(st);

			pVehicle->m_DemandType = g_read_integer(st);
			pVehicle->m_PricingType = g_read_integer(st);

			pVehicle->m_VehicleType = g_read_integer(st);
			pVehicle->m_InformationClass = g_read_integer(st);
			pVehicle->m_VOT = g_read_float(st);
			pVehicle->m_TollDollarCost = g_read_float(st);
			pVehicle->m_Emissions = g_read_float(st);
			float distance_in_mile = g_read_float(st);

			pVehicle->m_Distance = distance_in_mile;
			pVehicle->m_NodeSize	= g_read_integer(st);

			if(pVehicle->m_NodeSize>=1)  // in case reading error
			{
				pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

				pVehicle->m_NodeNumberSum = 0;
				for(int i=0; i< pVehicle->m_NodeSize; i++)
				{
					m_PathNodeVectorSP[i] = g_read_integer(st);
					pVehicle->m_NodeNumberSum += m_PathNodeVectorSP[i];
					if(i>=1)
					{
						DTALink* pLink = FindLinkWithNodeNumbers(m_PathNodeVectorSP[i-1],m_PathNodeVectorSP[i],lpszFileName);
						if(pLink==NULL)
						{
							AfxMessageBox("Error in reading file Vehicle.csv");
							fclose(st);

							return;
						}
						pVehicle->m_NodeAry[i].LinkNo  = pLink->m_LinkNo ;
						pLink->m_TotalTravelTime +=  pVehicle->m_NodeAry[i].ArrivalTimeOnDSN - pVehicle->m_NodeAry[i-1].ArrivalTimeOnDSN;
					}

					// random error beyond 6 seconds for better ainimation

					float random_value = g_RNNOF()*0.01; // 0.1 min = 6 seconds


					pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = m_SimulationStartTime_in_min + g_read_float(st);

					g_read_float(st);  // // travel time
					g_read_float(st);  // // emissions
				}

				m_VehicleSet.push_back (pVehicle);
				m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;


				count++;
			} 
		}

		fclose(st);
		m_SimulationVehicleDataLoadingStatus.Format ("%d vehicles are loaded from file %s.",count,lpszFileName);

	}
}

void CTLiteDoc::ReadVehicleCSVFile(LPCTSTR lpszFileName)
{

	//   cout << "Read vehicle file... "  << endl;
	// vehicle_id,  origin_zone_id, destination_zone_id, departure_time,
	//	arrival_time, complete_flag, trip_time, demand_type, occupancy, information_type,
	//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
	//	node id, node arrival time

	float LengthinMB;
	FILE* pFile;
	fopen_s(&pFile,lpszFileName,"rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);
		LengthinMB= Length*1.0/1024/1024;
		if(LengthinMB>250)
		{
			CString msg;
			msg.Format("The Vehicle.csv file is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load the vehicle file?",LengthinMB);
			if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
				return;
		}
	}



	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");
	if(st!=NULL)
	{
		char  str_line[2000]; // input string
		int str_line_size;
		g_read_a_line(st,str_line, str_line_size); //  skip the first line


		m_VehicleSet.clear();
		int count = 0;
		int vehicle_id;
		int from_zone_id;
		int to_zone_id;
		float departure_time;
		float arrival_time;
		int complete_flag;
		float trip_time;
		int demand_type;
		int pricing_type;
		int vehicle_type;
		int information_type;
		float value_of_time;
		float toll_cost_in_dollar;
		float emissions;
		float distance_in_mile;
		int number_of_nodes;

		while(fscanf(st,"%d,%d,%d,%f,%f,%d,%f,%d,%d,%d,%d,%f,%f,%f,%f,%d,\"",
			&vehicle_id,
			&from_zone_id,
			&to_zone_id,
			&departure_time,
			&arrival_time,
			&complete_flag,
			&trip_time,
			&demand_type,
			&pricing_type,
			&vehicle_type,
			&information_type,
			&value_of_time,
			&toll_cost_in_dollar,
			&emissions,
			&distance_in_mile,
			&number_of_nodes)>0	)
		{
			if(vehicle_id <0)
				break;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_VehicleID		= vehicle_id;
			pVehicle->m_OriginZoneID	= from_zone_id;
			pVehicle->m_DestinationZoneID = to_zone_id;
			pVehicle->m_DepartureTime	=  departure_time;
			pVehicle->m_ArrivalTime =  arrival_time;

			if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
				g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

			if(complete_flag==0) 
				pVehicle->m_bComplete = false;
			else
				pVehicle->m_bComplete = true;

			pVehicle->m_TripTime  = trip_time;

			pVehicle->m_DemandType = demand_type;
			pVehicle->m_PricingType = pricing_type;

			pVehicle->m_VehicleType = vehicle_type;
			pVehicle->m_InformationClass = information_type;
			pVehicle->m_VOT = value_of_time;
			pVehicle->m_TollDollarCost =toll_cost_in_dollar;
			pVehicle->m_Emissions = emissions;
			pVehicle->m_Distance = distance_in_mile;
			pVehicle->m_NodeSize	= number_of_nodes;



			if(pVehicle->m_NodeSize>=1)  // in case reading error
			{
				pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

				pVehicle->m_NodeNumberSum = 0;
				for(int i=0; i< pVehicle->m_NodeSize; i++)
				{

					int node_id;
					float time_stamp,travel_time, emissions;

					if(fscanf(st,"<%d;%f;%f;%f>",&node_id,&time_stamp,&travel_time, &emissions) ==0)
						break;


					m_PathNodeVectorSP[i] = node_id;
					pVehicle->m_NodeNumberSum += m_PathNodeVectorSP[i];
					if(i>=1)
					{
						DTALink* pLink = FindLinkWithNodeNumbers(m_PathNodeVectorSP[i-1],m_PathNodeVectorSP[i]);
						if(pLink==NULL)
						{
							AfxMessageBox("Error in reading file Vehicle.csv");
							fclose(st);

							return;
						}
						pVehicle->m_NodeAry[i].LinkNo  = pLink->m_LinkNo ;
						pLink->m_TotalTravelTime +=  pVehicle->m_NodeAry[i].ArrivalTimeOnDSN - pVehicle->m_NodeAry[i-1].ArrivalTimeOnDSN;
					}

					// random error beyond 6 seconds for better ainimation

					float random_value = g_RNNOF()*0.01; // 0.1 min = 6 seconds


					pVehicle->m_NodeAry[i].ArrivalTimeOnDSN =  time_stamp;

				}
				fscanf(st, "\"");
				m_VehicleSet.push_back (pVehicle);
				m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;


				count++;
			} 
		}

		fclose(st);
		m_SimulationVehicleDataLoadingStatus.Format ("%d vehicles are loaded from file %s.",count,lpszFileName);

	}
}

void CTLiteDoc::ReadAMSPathCSVFile(LPCTSTR lpszFileName)
{

	//   cout << "Read path file... "  << endl;
	// vehicle_id,  origin_zone_id, destination_zone_id, departure_time,
	//	arrival_time, complete_flag, trip_time, demand_type, occupancy, information_type,
	//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
	//	node id, node arrival time

	float LengthinMB;
	FILE* pFile;
	fopen_s(&pFile,lpszFileName,"rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);
		LengthinMB= Length*1.0/1024/1024;
		if(LengthinMB>250)
		{
			CString msg;
			msg.Format("The AMS_path_flow.csv file is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load the file?",LengthinMB);
			if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
				return;
		}
	}


	int count = 0;

	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");
	if(st!=NULL)
	{
		m_PathMap.clear();

		while(!feof(st))
		{
			int path_id = g_read_integer(st);
			if(path_id < 0)
				break;
			int vehicle_count = g_read_integer(st);
			int number_of_nodes = g_read_integer(st);

			CString label;
			label.Format("%d", path_id);

			//existing path
			m_PathMap[label].TotalVehicleSize  = vehicle_count;


			for(int i = 0; i< number_of_nodes; i++)
			{
				int node_number = g_read_integer(st);
				m_PathMap[label].m_NodeVector.push_back(node_number);

				if(i >=1)  // to be used when node number changes after generating physical links from connectors
				{
					DTALink* pLink = FindLinkWithNodeNumbers(m_PathMap[label].m_NodeVector[i-1],node_number);
					m_PathMap[label].m_LinkPointerVector .push_back(pLink);
				}
			}

			count++;
		} 
		fclose(st);
	}

	m_PathDataLoadingStatus.Format ("%d AMS paths are loaded from file %s.",count,lpszFileName);

}

void CTLiteDoc::ReadAMSMovementCSVFile(LPCTSTR lpszFileName)
{

	m_MovementPointerMap.clear();

	std::list<DTANode*>::iterator iNode;
	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		if((*iNode)->m_LayerNo == 0) 
		{
			for(unsigned int m = 0; m< (*iNode)->m_MovementVector .size(); m++)
			{

				DTANodeMovement movement = (*iNode)->m_MovementVector[m];

				CString label;
				int up_node_id = m_NodeIDMap[movement.in_link_from_node_id]->m_NodeNumber  ;
				int dest_node_id = m_NodeIDMap[movement.out_link_to_node_id ]->m_NodeNumber ;
				label.Format("%d;%d;%d", up_node_id,(*iNode)->m_NodeNumber,dest_node_id);

				m_MovementPointerMap[label] = &((*iNode)->m_MovementVector[m]); // store pointer

				// set default value for through turns

				if(movement.movement_turn == DTA_Through)
				{
					DTALink* pLink = FindLinkWithNodeNumbers(up_node_id,(*iNode)->m_NodeNumber,lpszFileName );
					if(pLink !=NULL)
					{

						(*iNode)->m_MovementVector[m].QEM_Lanes = pLink->m_NumberOfLanes;
						(*iNode)->m_MovementVector[m].QEM_Speed  = pLink->m_SpeedLimit ;

					}
				}


			}
		}
	}

	CCSVParser parser_movement;

	int count = 0;

	if (parser_movement.OpenCSVFile(lpszFileName))
	{
		while(parser_movement.ReadRecord())
		{
			int up_node_id, node_id, dest_node_id;

			if(parser_movement.GetValueByFieldName("node_id",node_id) == false)
				break;

			parser_movement.GetValueByFieldName("up_node_id",up_node_id);
			parser_movement.GetValueByFieldName("dest_node_id",dest_node_id);


			CString label;
			label.Format("%d;%d;%d", up_node_id,node_id,dest_node_id);

			if(m_MovementPointerMap.find(label) != m_MovementPointerMap.end())
			{
				DTANodeMovement* pMovement = m_MovementPointerMap[label];

				parser_movement.GetValueByFieldName ("prohibitted_flag",pMovement->turning_prohibition_flag);
				parser_movement.GetValueByFieldName ("protected_flag",pMovement->turning_protected_flag );
				parser_movement.GetValueByFieldName ("permitted_flag",pMovement->turning_permitted_flag);

				//parser_movement.GetValueByFieldName ("sim_turn_volume",pMovement->sim_turn_count );
				//parser_movement.GetValueByFieldName ("sim_turn_hourly_volume",pMovement->sim_turn_hourly_count );
				//parser_movement.GetValueByFieldName ("sim_turn_percentage",pMovement->turning_percentage);
				//parser_movement.GetValueByFieldName ("sim_turn_delay_in_second",pMovement->sim_turn_delay );

				parser_movement.GetValueByFieldName ("obs_turn_volume",pMovement->obs_turn_delay );
				parser_movement.GetValueByFieldName ("obs_turn_hourly_volume",pMovement->obs_turn_delay );
				parser_movement.GetValueByFieldName ("obs_turn_percentage",pMovement->obs_turn_delay );
				parser_movement.GetValueByFieldName ("obs_turn_delay_in_second",pMovement->obs_turn_delay );

				std::string turn_type;

				parser_movement.GetValueByFieldName ("turn_type",turn_type );
				parser_movement.GetValueByFieldName ("turn_direction",pMovement->QEM_dir_string );

				parser_movement.GetValueByFieldName ("QEM_TurnVolume",pMovement->QEM_TurnVolume );
				if(turn_type.find("Through") != string::npos )  // the # of lanes and speed for through movements are determined by link attribute
				{
					parser_movement.GetValueByFieldName ("QEM_Lanes",pMovement->QEM_Lanes );
					parser_movement.GetValueByFieldName ("QEM_Speed",pMovement->QEM_Speed );
				}

				parser_movement.GetValueByFieldName ("QEM_Shared",pMovement->QEM_Shared );
				parser_movement.GetValueByFieldName ("QEM_Width",pMovement->QEM_Width );
				parser_movement.GetValueByFieldName ("QEM_Storage",pMovement->QEM_Storage );
				parser_movement.GetValueByFieldName ("QEM_StLanes",pMovement->QEM_StLanes );
				parser_movement.GetValueByFieldName ("QEM_Grade",pMovement->QEM_Grade );

				parser_movement.GetValueByFieldName ("QEM_IdealFlow",pMovement->QEM_IdealFlow );
				parser_movement.GetValueByFieldName ("QEM_LostTime",pMovement->QEM_LostTime );
				parser_movement.GetValueByFieldName ("QEM_Phase1",pMovement->QEM_Phase1 );
				parser_movement.GetValueByFieldName ("QEM_PermPhase1",pMovement->QEM_PermPhase1 );
				parser_movement.GetValueByFieldName ("QEM_DetectPhase1",pMovement->QEM_DetectPhase1 );

				parser_movement.GetValueByFieldName ("QEM_EffectiveGreen",pMovement->QEM_EffectiveGreen );
				parser_movement.GetValueByFieldName ("QEM_Capacity",pMovement->QEM_Capacity );
				parser_movement.GetValueByFieldName ("QEM_VOC",pMovement->QEM_VOC );
				parser_movement.GetValueByFieldName ("QEM_SatFlow",pMovement->QEM_SatFlow );
				parser_movement.GetValueByFieldName ("QEM_Delay",pMovement->QEM_Delay );

				std::string str_los;
				parser_movement.GetValueByFieldName ("QEM_LOS", str_los );
				pMovement->QEM_LOS = str_los.c_str ();

				count++;

			}
		}
	}

	m_MovementDataLoadingStatus.Format ("%d AMS movements are loaded from file %s.",count,lpszFileName);

}
bool CTLiteDoc::ReadVehicleBinFile(LPCTSTR lpszFileName)
{
	//   cout << "Read vehicle file... "  << endl;
	// vehicle_id,  origin_zone_id, destination_zone_id, departure_time,
	//	arrival_time, complete_flag, trip_time, demand_type, occupancy, information_type,
	//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
	//	node id, node arrival time

	bool bCreateMovementMap = true;
	float LengthinMB;
	FILE* pFile;
	fopen_s(&pFile,lpszFileName,"rb");
	if(pFile!=NULL)
	{
		fseek(pFile, 0, SEEK_END );
		int Length = ftell(pFile);
		fclose(pFile);
		LengthinMB= Length*1.0/1024/1024;
		if(LengthinMB>50)
		{
			CString msg;
			msg.Format("The agent.bin file is %5.1f MB in size.\nIt could take quite a while to load this file.\nWould you like to load the vehicle file?",LengthinMB);
			if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
			{
				fclose(pFile);
				return true;
			}
		}

		//if(LengthinMB>20)  // if the file size is greater then 20 MB, ask the question
		//{
		//	CString msg;

		//	msg.Format("Do you want to create the movement mapping table (necessary for Synchro exporting function)?",LengthinMB);
		//	if(AfxMessageBox(msg,MB_YESNO|MB_ICONINFORMATION)==IDNO)
		//		bCreateMovementMap = false;
		//}
		fclose(pFile);
	}

	CString SettingsFile;
	SettingsFile.Format ("%sDTASettings.txt",m_ProjectDirectory);

	int version_number = 2;
	typedef struct  
	{
		int vehicle_id;
		int from_zone_id;
		int to_zone_id;
		float departure_time;
		float arrival_time;
		int complete_flag;
		float trip_time;
		int demand_type;
		int pricing_type;
		int vehicle_type;
		int information_type;
		float value_of_time;
		float toll_cost_in_dollar;
		float emissions;
		float distance_in_mile;
		int number_of_nodes;
		float Energy;
		float CO2;
		float NOX;
		float CO;
		float HC;


	} struct_Vehicle_Header_version_1;

	typedef struct  
	{
		int age;
		int version_no;

		int reserverd_field1;
		float reserverd_field2;
		int reserverd_field3;


	} struct_Vehicle_Header_extension_version_2;

	typedef  struct  
	{
		int NodeName;
		float AbsArrivalTimeOnDSN;
	} struct_Vehicle_Node;

	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"rb");
	if(st!=NULL)
	{
		m_VehicleSet.clear();
		m_Movement3NodeMap.clear();


		struct_Vehicle_Header_version_1 header;

		struct_Vehicle_Header_extension_version_2 header_extension;

		int count =0;
		while(!feof(st))
		{

			size_t result;
			size_t result_extension;

			result = fread(&header,sizeof(struct_Vehicle_Header_version_1),1,st);
			if(version_number == 2)
				result_extension = fread(&header_extension,sizeof(struct_Vehicle_Header_extension_version_2),1,st);

			if( header.vehicle_id < 0)
				break;

			if(result!=1)  // read end of file
				break;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_VehicleID		= header.vehicle_id;
			pVehicle->m_OriginZoneID	= header.from_zone_id;
			pVehicle->m_DestinationZoneID= header.to_zone_id;
			pVehicle->m_DepartureTime	=  header.departure_time;
			pVehicle->m_ArrivalTime =  header.arrival_time;

			if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
				g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

			if(header.complete_flag==0) 
				pVehicle->m_bComplete = false;
			else
				pVehicle->m_bComplete = true;

			pVehicle->m_TripTime  = header.trip_time;

			pVehicle->m_DemandType = header.demand_type;
			pVehicle->m_PricingType = header.pricing_type;

			pVehicle->m_VehicleType = header.vehicle_type;
			pVehicle->m_InformationClass = header.information_type;
			pVehicle->m_VOT = header.value_of_time;
			pVehicle->m_TollDollarCost = header.toll_cost_in_dollar;
			pVehicle->m_Emissions = header.emissions;
			pVehicle->m_Distance = header.distance_in_mile;
			pVehicle->m_NodeSize	= header.number_of_nodes;

			m_ZoneMap[pVehicle->m_OriginZoneID].m_OriginTotalNumberOfVehicles += 1;
			m_ZoneMap[pVehicle->m_OriginZoneID].m_OriginTotalTravelDistance  += pVehicle->m_Distance ;

			m_ZoneMap[pVehicle->m_DestinationZoneID].m_DestinationTotalNumberOfVehicles += 1;
			m_ZoneMap[pVehicle->m_DestinationZoneID].m_DestinationTotalTravelDistance  += pVehicle->m_Distance ;


			pVehicle->m_EmissionData .Energy = header.Energy;

			if(header.Energy >1)
				m_bEmissionDataAvailable =true;

			pVehicle->m_EmissionData .CO2 = header.CO2;
			pVehicle->m_EmissionData .NOX = header.NOX;
			pVehicle->m_EmissionData .CO = header.CO;
			pVehicle->m_EmissionData .HC = header.HC;

			if(version_number == 2)
			{
				pVehicle->m_Age = header_extension.age;
			}

			if(pVehicle->m_NodeSize>=1)  // in case reading error
			{
				pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

				pVehicle->m_NodeNumberSum = 0;
				for(int i=0; i< pVehicle->m_NodeSize; i++)
				{


					struct_Vehicle_Node node_element;
					fread(&node_element,sizeof(node_element),1,st);

					m_PathNodeVectorSP[i] = node_element.NodeName;
					pVehicle->m_NodeNumberSum += m_PathNodeVectorSP[i];
					if(i>=1)
					{
						DTALink* pLink = FindLinkWithNodeNumbers(m_PathNodeVectorSP[i-1],m_PathNodeVectorSP[i]);
						if(pLink==NULL)
						{
							AfxMessageBox("Error in reading file agent.bin");
							fclose(st);

							return false;
						}
						pVehicle->m_NodeAry[i].LinkNo  = pLink->m_LinkNo ;
						pLink->m_TotalTravelTime +=  pVehicle->m_NodeAry[i].ArrivalTimeOnDSN - pVehicle->m_NodeAry[i-1].ArrivalTimeOnDSN;
						pLink->m_total_link_volume +=1;


					}

					// random error beyond 6 seconds for better ainimation

					float random_value = g_RNNOF()*0.01; // 0.1 min = 6 seconds

					pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = node_element.AbsArrivalTimeOnDSN;

				}

				// movement count


				m_VehicleSet.push_back (pVehicle);
				m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;


				count++;
			} 
		}


		// movement count
		std::list<DTANode*>::iterator iNode;
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			(*iNode)->ResetMovementMOE ();		
		}

		std::list<DTAVehicle*>::iterator iVehicle;
		for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++)
		{
			DTAVehicle* pVehicle = (*iVehicle);

			for(int i= 2; i<pVehicle->m_NodeSize; i++)
			{
				DTALink* pLink0 = m_LinkNoMap[pVehicle->m_NodeAry[i-1].LinkNo];
				DTALink* pLink1 = m_LinkNoMap[pVehicle->m_NodeAry[i].LinkNo];

				DTANode* pNode = m_NodeIDMap[ pLink0->m_ToNodeID ];

				if(pNode->m_NodeNumber == 2)
				{
					TRACE("");
				}

				int Hour = pVehicle->m_NodeAry[i].ArrivalTimeOnDSN / 60;  // min to hour
				if(pLink0 !=NULL && pLink1!=NULL)
				{
				pLink0->m_TotalVolumeForMovementCount++;
				pNode->AddMovementCount(Hour, pLink0->m_FromNodeID, pLink1->m_ToNodeID );
				}

			}
		}

		// turning percentage
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			for(unsigned int m = 0; m< (*iNode)->m_MovementVector .size(); m++)
			{
				DTANodeMovement* pMovement = &((*iNode)->m_MovementVector[m]);
				DTALink* pLink0 = m_LinkNoMap[pMovement->IncomingLinkID  ];

				int total_link_count = 0;
				for(unsigned int j = 0; j< (*iNode)->m_MovementVector .size(); j++)
				{

					if((*iNode)->m_MovementVector[j].IncomingLinkID == pMovement->IncomingLinkID )
					{
						total_link_count+= (*iNode)->m_MovementVector[j].sim_turn_count ;
					}

				}

				if (pMovement->sim_turn_count >=1)
				{
					pMovement->turning_percentage = 
						pMovement->sim_turn_count * 100.0f / max(1,total_link_count);
				}
			}

		}


		fclose(st);
		m_SimulationVehicleDataLoadingStatus.Format ("%d vehicles are loaded from file %s.",count,lpszFileName);
		return true;

	}
	return false;
}

int CTLiteDoc::GetVehilePosition(DTAVehicle* pVehicle, double CurrentTime, float& ratio)
{
	GDPoint pt;
	pt.x = 0;
	pt.y = 0;

	int search_counter = 0;
	int beg, end, mid;
	beg = 1;
	end = pVehicle->m_NodeSize-1;
	mid = (beg+end)/2;
	int i = mid;
	ratio = 0;
	float link_travel_time;
	float traveling_time;

	while(beg<=end)
	{
		if(CurrentTime >= pVehicle->m_NodeAry [i-1].ArrivalTimeOnDSN &&
			CurrentTime <= pVehicle->m_NodeAry [i].ArrivalTimeOnDSN )	// find the link between the time interval
		{

			link_travel_time = pVehicle->m_NodeAry [i].ArrivalTimeOnDSN - pVehicle->m_NodeAry [i-1].ArrivalTimeOnDSN;

			traveling_time = CurrentTime - pVehicle->m_NodeAry [i-1].ArrivalTimeOnDSN;

			ratio = traveling_time/link_travel_time;

			if(ratio <0)
				ratio = 0;

			if(ratio >1)
				ratio = 1;

			if(pVehicle->m_NodeAry [i].LinkNo  == 457)
				TRACE("");

			return pVehicle->m_NodeAry [i].LinkNo ;

			//			pt.x =  ratio*pLink->m_FromPoint .x  +  (1-ratio)*ratio*pLink->m_ToPoint .x;
			//			pt.y =  ratio*pLink->m_FromPoint .y  +  (1-ratio)*ratio*pLink->m_ToPoint .y;

		}else  // not found
		{
			if( CurrentTime>= pVehicle->m_NodeAry [i].ArrivalTimeOnDSN)  // time stamp is after i
			{
				// shift to the right
				beg=mid+1;
				mid=(beg+end)/2;
				i = mid;
			}else //CurrentTime < pVehicle->m_NodeAry[i].ArrivalTimeOnDSN// time stamp is before i
			{   //shift to the left
				end=mid-1;
				mid=(beg+end)/2;
				i = mid;
			}

			search_counter++;

			if(search_counter > pVehicle->m_NodeSize)  // exception handling
			{
				//				ASSERT(false);
				return 0;
			}

		}
	}

	return 0;
}

bool CTLiteDoc::GetGPSVehilePosition(DTAVehicle* pVehicle, double CurrentTime, GDPoint& pt)
{

	pt.x = pVehicle->m_LocationRecordAry[0].x;
	pt.y = pVehicle->m_LocationRecordAry[0].y;

	int search_counter = 0;
	int beg, end, mid;
	beg = 1;
	end = pVehicle->m_NodeSize-1;
	mid = (beg+end)/2;
	int i = mid;
	double ratio = 0;
	float link_travel_time;
	float remaining_time;

	while(beg<=end)
	{
		if(CurrentTime >= pVehicle->m_LocationRecordAry [i-1].time_stamp_in_min &&
			CurrentTime <= pVehicle->m_LocationRecordAry [i].time_stamp_in_min )	// find the link between the time interval
		{

			link_travel_time = pVehicle->m_LocationRecordAry [i].time_stamp_in_min - pVehicle->m_LocationRecordAry [i-1].time_stamp_in_min;

			remaining_time = CurrentTime - pVehicle->m_LocationRecordAry [i-1].time_stamp_in_min;

			ratio = 1-remaining_time/link_travel_time;

			if(ratio <0)
				ratio = 0;

			if(ratio >1)
				ratio = 1;

			pt.x = ratio * pVehicle->m_LocationRecordAry [i-1].x + (1- ratio)* pVehicle->m_LocationRecordAry [i].x ;
			pt.y = ratio * pVehicle->m_LocationRecordAry [i-1].y + (1- ratio)* pVehicle->m_LocationRecordAry [i].y ;

			return true ;

		}else  // not found
		{
			if( CurrentTime>= pVehicle->m_LocationRecordAry [i].time_stamp_in_min)  // time stamp is after i
			{
				// shift to the right
				beg=mid+1;
				mid=(beg+end)/2;
				i = mid;
			}else //CurrentTime < pVehicle->m_NodeAry[i].ArrivalTimeOnDSN// time stamp is before i
			{   //shift to the left
				end=mid-1;
				mid=(beg+end)/2;
				i = mid;
			}

			search_counter++;

			if(search_counter > pVehicle->m_NodeSize)  // exception handling
			{
				//				ASSERT(false);
				return false;
			}

		}
	}

	return false;
}
void CTLiteDoc::OnImageImportbackgroundimage()
{
	
	if(!m_BackgroundBitmapLoaded)
		ReadBackgroundImageFile(m_ProjectFile);

		m_ImageX2  = m_ImageX1+ m_ImageWidth * m_ImageXResolution;
		m_ImageY2  = m_ImageY1+ m_ImageHeight * m_ImageYResolution;

	CDlg_ImageSettings dlg_image;

	dlg_image.m_pDoc = this;

	dlg_image.m_X1 = m_ImageX1;
	dlg_image.m_Y1 = m_ImageY1;

	dlg_image.m_X2 = m_ImageX2;
	dlg_image.m_Y2 = m_ImageY2;

	if(m_BackgroundBitmapLoaded)
	{
		dlg_image.m_Image_File_Message  = "Background bitmap file has been loaded.";

	}
	else 
	{
		dlg_image.m_Image_File_Message  = "Please prepare background bitmap file as\nbackground_image.bmp.";
	}

	if(dlg_image.DoModal ()==IDOK)
	{
		m_ImageX1 = dlg_image.m_X1;
		m_ImageY1 = dlg_image.m_Y1;

		m_ImageX2 = dlg_image.m_X2;
		m_ImageY2 = dlg_image.m_Y2;
	
		OnFileSaveimagelocation();

	}

	m_bFitNetworkInitialized = false;
	UpdateAllViews(0);
}


void CTLiteDoc::OnFileDataloadingstatus()
{
	CDlgFileLoading dlg;
	dlg.m_pDoc = this;
	dlg.DoModal ();
}

void CTLiteDoc::OnMoeVolume()
{
	m_LinkMOEMode = MOE_volume;
	m_LinkBandWidthMode = LBW_link_volume;
	ShowLegend(false);

	GenerateOffsetLinkBand();
	UpdateAllViews(0);}

void CTLiteDoc::OnMoeSpeed()
{
	m_LinkMOEMode = MOE_speed;
	m_LinkBandWidthMode = LBW_link_volume;

	ShowLegend(true);
	GenerateOffsetLinkBand();
	UpdateAllViews(0);
}

void CTLiteDoc::OnMoeDensity()
{
	m_LinkMOEMode = MOE_density;
	m_LinkBandWidthMode = LBW_link_volume;

	GenerateOffsetLinkBand();
	UpdateAllViews(0);
}

void CTLiteDoc::OnMoeQueuelength()
{
	m_LinkMOEMode = MOE_queuelength;
	m_LinkBandWidthMode = LBW_link_volume;

	GenerateOffsetLinkBand();
	UpdateAllViews(0);}

void CTLiteDoc::OnMoeFuelconsumption()
{
	m_LinkMOEMode = MOE_fuel;
	m_LinkBandWidthMode = LBW_link_volume;

	UpdateAllViews(0);}

void CTLiteDoc::OnMoeEmissions()
{
	m_LinkMOEMode = MOE_emissions;
	m_LinkBandWidthMode = LBW_link_volume;
	ShowLegend(false);
	UpdateAllViews(0);}

void CTLiteDoc::OnUpdateMoeVolume(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_volume);
}

void CTLiteDoc::OnUpdateMoeSpeed(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_speed);
}

void CTLiteDoc::OnUpdateMoeDensity(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_density);
}

void CTLiteDoc::OnUpdateMoeQueuelength(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_queuelength);
}

void CTLiteDoc::OnUpdateMoeFuelconsumption(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_fuel);
}

void CTLiteDoc::OnUpdateMoeEmissions(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_emissions);

}

void CTLiteDoc::OnMoeNone()
{
	m_LinkMOEMode = MOE_none;

	ShowLegend(false);

	// visualization configuration
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeNone(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_none);
}

float CTLiteDoc::GetLinkMOE(DTALink* pLink, Link_MOE LinkMOEMode,int CurrentTime, int AggregationIntervalInMin, float &value)
{

	float power = 0.0f;
	float max_link_volume = 8000.0f;
	float max_speed_ratio = 2.0f; 
	float max_density = 45.0f;

	switch (LinkMOEMode)
	{
	case MOE_volume:  power = pLink->m_total_link_volume/max_link_volume; 
		value = pLink->m_total_link_volume;
		break;
	case MOE_speed:   power = pLink->m_avg_simulated_speed/max(1, pLink->m_SpeedLimit); 
		value = pLink->m_avg_simulated_speed;
		break;

	case MOE_reliability:   power = pLink->m_volume_over_capacity_ratio;
		value = pLink->m_volume_over_capacity_ratio;
		break;

	case MOE_density: power = 0;
		value = 0;
		break;
	case MOE_traveltime:  
		value = pLink->m_StaticTravelTime;
		break;

	case MOE_capacity:  
		value = pLink->m_LaneCapacity * pLink->m_NumberOfLanes ;
		break;

	case MOE_speedlimit:  
		value = pLink->m_SpeedLimit  ;
		break;

	case MOE_fftt: 
		value = pLink->m_FreeFlowTravelTime ;
		break;

	case MOE_length: 
		value = pLink->m_Length  ;
		break;

	case MOE_queue_length:
		value =  0;
		break;

	case MOE_safety:  power = pLink->m_number_of_all_crashes/ max(0.0001,pLink->m_Length ); 
		value = pLink->m_number_of_all_crashes/ max(0.0001,pLink->m_Length );
		break;


	default: power = 0.0;

	}

	if(m_TrafficFlowModelFlag > 0)
	{
		int StartTime = CurrentTime;
		int EndTime  = min(CurrentTime + AggregationIntervalInMin, pLink->m_SimulationHorizon);

		float max_power = 0; 
		float min_power = 1;

		if(LinkMOEMode == MOE_reliability)
		{
			CurrentTime = int(CurrentTime/60)*60;  // hour by hour
			AggregationIntervalInMin = 60;
			EndTime  = min(CurrentTime + AggregationIntervalInMin, pLink->m_SimulationHorizon);

		}
		float total_value = 0;
		int total_measurement_count = 0;


		for(CurrentTime  = StartTime; CurrentTime < EndTime; CurrentTime ++)
		{
			if(pLink->m_SimulationHorizon > CurrentTime && CurrentTime >=1 && CurrentTime < pLink->m_SimulationHorizon)  //DTAoutput
			{
				if(pLink->m_LinkMOEAry[CurrentTime].SimulationLinkFlow>=1 || pLink->m_LinkMOEAry[CurrentTime].SimulationDensity >=0.1)
				{
					total_measurement_count++;

					switch (LinkMOEMode)
					{
					case MOE_volume:  

						total_value+= pLink->m_LinkMOEAry[CurrentTime].SimulationLinkFlow;

						break;
					case MOE_speed: 
					case MOE_reliability: 

						total_value+= pLink->m_LinkMOEAry[CurrentTime].SimulationSpeed;

						break;
					case MOE_traveltime:
						total_value+= pLink->m_LinkMOEAry[CurrentTime].SimulatedTravelTime;
						break;
					case MOE_density:
						total_value+= pLink->m_LinkMOEAry[CurrentTime].SimulationDensity;
						break;
					case MOE_queue_length: 
						total_value+= pLink->m_LinkMOEAry[CurrentTime].SimulationQueueLength;
						break;

					case MOE_emissions:
						total_value+= pLink->m_LinkMOEAry[CurrentTime].Energy ;
						break;

					}

				}
			}  // end of for loop
		}

		value = total_value/max(1,total_measurement_count);

		switch (LinkMOEMode)
		{
		case MOE_volume:  
			power = value/max_link_volume;
			break;
		case MOE_speed: 
			power =  min(100,value / pLink->m_SpeedLimit*100);
			break;
		case MOE_reliability: 
			power =  value / pLink->m_SpeedLimit;
			break;
		case MOE_traveltime:
			if(value <=0.1)  // no data
				power = 0;
			else 
				power = pLink->m_SpeedLimit / max(1,value)*100; 

			break;
		case MOE_density:
			power = value; 
			break;
		case MOE_queue_length: 
			power = value  ; 
			break;
		case MOE_safety:  power = pLink->m_number_of_all_crashes/ max(0.0001,pLink->m_Length ); 
			value = pLink->m_number_of_all_crashes/ max(0.0001,pLink->m_Length );
			break;

		case MOE_emissions:
			power = value;
			break;


		}



	}

	return power;
}

void CTLiteDoc::OnToolsCarfollowingsimulation()
{
	if(m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= m_LinkNoMap [m_SelectedLinkNo];
	if(pLink!=NULL)
	{
		CDlgCarFollowing dlg;
		dlg.pDoc = this;
		dlg.m_SelectedLinkNo = m_SelectedLinkNo;
		dlg.m_YUpperBound = pLink->m_Length * 1609.344f;  // mile to meters
		dlg.m_FreeflowSpeed = pLink->m_SpeedLimit * 1609.344f / 3600; // mph to meter per second
		dlg.m_NumberOfLanes = pLink->m_NumberOfLanes;
		dlg.DoModal ();
	}
}

DWORD CTLiteDoc::ProcessWait(DWORD PID) 
{
	DWORD dwRetVal = DWORD(-1);
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
	if(hProcess) {
		do 
		{
			::Sleep(500);
			::GetExitCodeProcess(hProcess, &dwRetVal);
		} while(dwRetVal == STILL_ACTIVE);
		CloseHandle(hProcess);
	}
	return dwRetVal;
}

DWORD CTLiteDoc::ProcessExecute(CString & strCmd, CString & strArgs,  CString & strDir, BOOL bWait)
{

	STARTUPINFO StartupInfo;
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION ProcessInfo;
	DWORD dwRetVal = DWORD(-1);

	/*
	TCHAR szExe[1024];
	if( int(FindExecutable(
	strPath,									// pointer to filename
	strDir.IsEmpty() ? 0 : LPCTSTR(strDir),					// pointer to default directory
	szExe										// result-buffer
	)) <= 32 ) {
	OnExecError(GetLastError(), strCmd);
	return DWORD(-1);
	}
	*/
	BOOL bStat =	CreateProcess(
		strCmd,						// pointer to name of executable module
		0,			// pointer to command line string
		0,						// pointer to process security attributes
		0,						// pointer to thread security attributes
		TRUE,						// handle inheritance flag
		0,						// creation flags
		0,						// pointer to new environment block
		strDir.IsEmpty() ? 0 : LPCTSTR(strDir),
		&StartupInfo,				// pointer to STARTUPINFO
		&ProcessInfo				// pointer to PROCESS_INFORMATION
		);
	if( bStat ) 
	{
		if( bWait ) 
		{
			::WaitForInputIdle(ProcessInfo.hProcess, INFINITE);
			dwRetVal = ProcessWait(ProcessInfo.dwProcessId);
		} else {
			// before we return to the caller, we wait for the currently
			// started application until it is ready to work.
			::WaitForInputIdle(ProcessInfo.hProcess, INFINITE);
			dwRetVal = ProcessInfo.dwProcessId;
		}

		::CloseHandle(ProcessInfo.hThread);
	}

	return dwRetVal;
}


void CTLiteDoc::OnToolsPerformtrafficassignment()
{
	STARTUPINFO si = { 0 };  
	PROCESS_INFORMATION pi = { 0 };  

	si.cb = sizeof(si); 

	CWaitCursor curs;
	if(!SetCurrentDirectory(m_ProjectDirectory))
	{
		if(m_ProjectDirectory.GetLength()==0)
		{
			AfxMessageBox("The project directory has not been specified.");
			return;
		}
		CString msg;
		msg.Format ("The specified project directory %s does not exist.", m_ProjectDirectory);
		AfxMessageBox(msg);
		return;
	}

	CString sCommand;
	CString strParam;
	CTime ExeStartTime = CTime::GetCurrentTime();

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	if(m_bDYNASMARTDataSet)
		sCommand.Format("%s\\planning.exe", pMainFrame->m_CurrentDirectory);
	else
	{
#ifndef _WIN64
		sCommand.Format("%s\\DTALite_32.exe", pMainFrame->m_CurrentDirectory);
#else
		sCommand.Format("%s\\DTALite_64.exe", pMainFrame->m_CurrentDirectory);
#endif
	}

	ProcessExecute(sCommand, strParam, m_ProjectDirectory, true);

	CTime ExeEndTime = CTime::GetCurrentTime();

	CTimeSpan ts = ExeEndTime  - ExeStartTime;
	CString str_running_time;

	FILE* st = NULL;

	CString directory = m_ProjectDirectory;

	str_running_time.Format ("Program execution has completed.\nProgram execution time: %d hour(s) %d min(s) %d sec(s).\nPlease check time-dependent link MOEs during period %s->%s.\nDo you want to view the output summary file now?",

		ts.GetHours(), ts.GetMinutes(), ts.GetSeconds(),
		GetTimeStampString24HourFormat(m_DemandLoadingStartTimeInMin),
		GetTimeStampString24HourFormat(m_DemandLoadingEndTimeInMin));


	if( AfxMessageBox(str_running_time, MB_YESNO| MB_ICONINFORMATION)==IDYES)
	{
		OnToolsViewassignmentsummarylog();
	}
		LoadSimulationOutput();
		UpdateAllViews(0);


}
void CTLiteDoc::LoadSimulationOutput()
{

	ReadMetaDemandCSVFile(m_ProjectDirectory+"input_demand_meta_data.csv");
	ReadScenarioSettingCSVFile(m_ProjectDirectory+"input_scenario_settings.csv");

	CString DTASettingsPath = m_ProjectDirectory+"DTASettings.txt";
	g_Simulation_Time_Horizon = 1440;
	SetStatusText("Loading output link time-dependent data");
	ReadSimulationLinkOvarvallMOEData(m_ProjectDirectory+"output_LinkMOE.csv");
	ReadSimulationLinkMOEData_Bin(m_ProjectDirectory+"output_LinkTDMOE.bin");
	//SetStatusText("Loading AMS movement data");
	ReadAMSMovementCSVFile(m_ProjectDirectory+"AMS_movement.csv");

	SetStatusText("Loading agent trajectory data");
	if(ReadVehicleBinFile(m_ProjectDirectory+"agent.bin")==false)
	{
		ReadVehicleCSVFile(m_ProjectDirectory+"output_agent.csv");
	}

	char microsimulation_file_name[_MAX_STRING_SIZE];
	g_GetProfileString("microsimulation_data","file_name","",microsimulation_file_name,sizeof(microsimulation_file_name),m_ProjectFile);

	CString microsimulation_file_str;

	microsimulation_file_str.Format("%s",microsimulation_file_name);

	if(microsimulation_file_str.Find("bin") >=0)
	{
		ReadTraceBinFile(m_ProjectDirectory+microsimulation_file_name,0);
	}

	if(microsimulation_file_str.Find("csv") >=0)
	{
		ReadAimCSVFiles(m_ProjectDirectory+microsimulation_file_name,0);
	}

	char GPS_file_name[_MAX_STRING_SIZE];
	g_GetProfileString("GPS_data","file_name","",GPS_file_name,sizeof(GPS_file_name),m_ProjectFile);

	CString GPS_file_str;

	GPS_file_str.Format("%s",GPS_file_name);

	if(GPS_file_str.Find("bin") >=0)
	{
		ReadGPSBinFile(m_ProjectDirectory+GPS_file_name,0);
	}


	SetStatusText("Generating OD statistics...");

	ResetODMOEMatrix();
	ReadAMSPathCSVFile(m_ProjectDirectory+"AMS_path_flow.csv");

	SetStatusText("");


}


void CTLiteDoc::OnMoeVcRatio()
{
	m_LinkMOEMode = MOE_density;
	ShowLegend(true);
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeVcRatio(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_density);
}

void CTLiteDoc::OnMoeTraveltime()
{
	m_LinkMOEMode = MOE_traveltime;

	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeTraveltime(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_traveltime);
}

void CTLiteDoc::OnMoeCapacity()
{
	m_LinkMOEMode = MOE_capacity;
	ShowTextLabel();
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeCapacity(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_capacity);
}

void CTLiteDoc::OnMoeSpeedlimit()
{
	m_LinkMOEMode = MOE_speedlimit;
	ShowTextLabel();
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeSpeedlimit(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_speedlimit);
}

void CTLiteDoc::OnMoeFreeflowtravletime()
{
	m_LinkMOEMode = MOE_fftt;
	ShowTextLabel();

	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeFreeflowtravletime(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_fftt);
}


CString CTLiteDoc::GetWorkspaceTitleName(CString strFullPath)
{
	int len = strFullPath.GetLength();
	/////////////////////////////////

	CString	strPathTitleName;

	strPathTitleName.Empty ();
	bool StartFlag = false;

	for(int k=len-1;k>=0;k--)
	{
		if(strFullPath[k]=='\\')
			break;

		if(strFullPath[k]=='.' && StartFlag == false)
		{
			StartFlag = true;
			continue;
		}

		if(StartFlag == true)
			strPathTitleName+=strFullPath[k];

	}

	strPathTitleName.MakeReverse();

	return strPathTitleName;

}
void CTLiteDoc::OnEditDeleteselectedlink()
{

	// TODO: Add your command handler code here
}


void CTLiteDoc::OnMoeLength()
{
	m_LinkMOEMode = MOE_length;
	ShowTextLabel();
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeLength(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_length);
}

void CTLiteDoc::OnEditSetdefaultlinkpropertiesfornewlinks()
{
	CDlgDefaultLinkProperties dlg;
	dlg.m_pDoc  = this;

	dlg.SpeedLimit = m_DefaultSpeedLimit ;
	dlg.LaneCapacity = m_DefaultCapacity ;
	dlg.NumLanes = m_DefaultNumLanes;
	dlg.LinkType = m_DefaultLinkType;

	dlg.m_StartNodeNumberForNewNodes = m_StartNodeNumberForNewNodes;
	if(dlg.DoModal() == IDOK)
	{
		m_DefaultSpeedLimit = dlg.SpeedLimit;
		m_DefaultCapacity = dlg.LaneCapacity;
		m_DefaultNumLanes = dlg.NumLanes;
		m_DefaultLinkType = dlg.LinkType;

		m_LongLatFlag = dlg.m_bLongLatSystem ;
		
		UpdateUnitMile();
		m_StartNodeNumberForNewNodes = 	dlg.m_StartNodeNumberForNewNodes ;



	}
}

void CTLiteDoc::OnUpdateEditSetdefaultlinkpropertiesfornewlinks(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CTLiteDoc::OnToolsProjectfolder()
{
	if(m_ProjectDirectory.GetLength()==0)
	{
		AfxMessageBox("The project directory has not been specified.");
		return;
	}

	ShellExecute( NULL,  "explore", m_ProjectDirectory, NULL,  NULL, SW_SHOWNORMAL );
}

void CTLiteDoc::OnToolsOpennextaprogramfolder()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	ShellExecute( NULL,  "explore", pMainFrame->m_CurrentDirectory, NULL,  NULL, SW_SHOWNORMAL );
}

void CTLiteDoc::OnMoeNoodmoe()
{
	m_ODMOEMode = odnone;
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeNoodmoe(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_ODMOEMode == MOE_none);
}


void CTLiteDoc::OnOdtableImportOdTripFile()
{
	CString strFilter = "OD Trip File|*.txt";

	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,strFilter);
	//	dlg.m_ofn.nFilterIndex = m_nFilterLoad;
	HRESULT hResult = (int)dlg.DoModal();
	if (FAILED(hResult)) {
		return;
	}
	ReadTripTxtFile(dlg.GetPathName());
	OnToolsEditoddemandtable();
	m_bFitNetworkInitialized = false;
	UpdateAllViews(0);
}

void CTLiteDoc::OnToolsEditassignmentsettings()
{
	EditTrafficAssignmentOptions();
}

void CTLiteDoc::OnToolsEditoddemandtable()
{
	CDlgODDemandGridCtrl dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();
}

void CTLiteDoc::OnSearchLinklist()
{

	g_bShowLinkList = !g_bShowLinkList;

	if(g_bShowLinkList)
	{
		if(g_pLinkListDlg==NULL)
		{
			g_pLinkListDlg = new CDlgLinkList();
			g_pLinkListDlg->Create(IDD_DIALOG_LINK_LIST);
		}

		// update using pointer to the active document; 

		if(g_pLinkListDlg->GetSafeHwnd())
		{
			// we udpate the pointer list for document every time we open this link list window
			std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
			while (iDoc != g_DocumentList.end())
			{
				if ((*iDoc)->m_NodeSet.size()>0)
				{

					if(g_pLinkListDlg->m_pDoc ==NULL)
						g_pLinkListDlg->m_pDoc = (*iDoc);
					else if((*iDoc)!= g_pLinkListDlg->m_pDoc)
					{
						g_pLinkListDlg->m_pDoc2 = (*iDoc);
						g_pLinkListDlg->m_bDoc2Ready = true;
					}

				}
				iDoc++;

			}

			g_pLinkListDlg->ReloadData ();
			g_pLinkListDlg->ShowWindow(SW_HIDE);
			g_pLinkListDlg->ShowWindow(SW_SHOW);
		}
	}else
	{
		if(g_pLinkListDlg!=NULL && g_pLinkListDlg->GetSafeHwnd())
		{
			g_pLinkListDlg->ShowWindow(SW_HIDE);
		}
	}

}

void CTLiteDoc::OnMoeVehicle()
{
	m_LinkMOEMode = MOE_vehicle;
	GenerateOffsetLinkBand();
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateMoeVehicle(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_vehicle);
}
LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
	HKEY hkey;
	LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

	if (retval == ERROR_SUCCESS) {
		long datasize = MAX_PATH;
		TCHAR data[MAX_PATH];
		RegQueryValue(hkey, NULL, data, &datasize);
		lstrcpy(retdata,data);
		RegCloseKey(hkey);
	}

	return retval;
}
HINSTANCE g_OpenDocument(LPCTSTR url, int showcmd)
{
	TCHAR key[MAX_PATH + MAX_PATH];

	// First try ShellExecute()
	HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

	// If it failed, get the .htm regkey and lookup the program
	if ((UINT)result <= HINSTANCE_ERROR) {

		if (GetRegKey(HKEY_CLASSES_ROOT, _T(".csv"), key) == ERROR_SUCCESS) {
			lstrcat(key, _T("\\shell\\open\\command"));

			if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
				TCHAR *pos;
				pos = _tcsstr(key, _T("\"%1\""));
				if (pos == NULL) {			   // No quotes found
					pos = strstr(key, _T("%1"));	   // Check for %1, without quotes
					if (pos == NULL)			   // No parameter at all...
						pos = key+lstrlen(key)-1;
					else
						*pos = '\0';			 // Remove the parameter
				}
				else
					*pos = '\0';			  // Remove the parameter

				lstrcat(pos, _T(" "));
				lstrcat(pos, url);
				result = (HINSTANCE) WinExec(key,showcmd);
			}
		}
	}

	return result;
}

HINSTANCE g_GotoURL(LPCTSTR url, int showcmd)
{
	TCHAR key[MAX_PATH + MAX_PATH];

	// First try ShellExecute()
	HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

	// If it failed, get the .htm regkey and lookup the program
	if ((UINT)result <= HINSTANCE_ERROR) {

		if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) {
			lstrcat(key, _T("\\shell\\open\\command"));

			if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
				TCHAR *pos;
				pos = _tcsstr(key, _T("\"%1\""));
				if (pos == NULL) {			   // No quotes found
					pos = strstr(key, _T("%1"));	   // Check for %1, without quotes
					if (pos == NULL)			   // No parameter at all...
						pos = key+lstrlen(key)-1;
					else
						*pos = '\0';			 // Remove the parameter
				}
				else
					*pos = '\0';			  // Remove the parameter

				lstrcat(pos, _T(" "));
				lstrcat(pos, url);
				result = (HINSTANCE) WinExec(key,showcmd);
			}
		}
	}

	return result;
}

void CTLiteDoc::OnToolsViewsimulationsummary()
{
	CDlg_VehicleClassification* m_pDlg = new CDlg_VehicleClassification; 

	g_SummaryDialogVector.push_back (m_pDlg);  // collect memory block
	m_pDlg->m_pDoc = this;
	m_pDlg->SetModelessFlag(true); // voila! this is all it takes to make your dlg modeless!
	m_pDlg->Create(IDD_NSCHART_VEHICLE_DIALOG); 
	m_pDlg->ShowWindow(SW_SHOW); 

	m_bSummaryDialog = true;
}


void CTLiteDoc::OnToolsViewassignmentsummarylog()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_summary.csv");
}

void CTLiteDoc::OnHelpVisitdevelopmentwebsite()
{
	g_OpenDocument("http://code.google.com/p/nexta/", SW_SHOW);
}

bool CTLiteDoc::EditTrafficAssignmentOptions()
{

	CDlgAssignmentSettings dlg;
	dlg.m_pDoc = this;


	ReadDemandTypeCSVFile(m_ProjectDirectory+"input_demand_type.csv");
	ReadMetaDemandCSVFile(m_ProjectDirectory+"input_demand_meta_data.csv");
	ReadScenarioSettingCSVFile(m_ProjectDirectory+"input_scenario_settings.csv");

	if(dlg.DoModal() ==IDOK)
	{
		return true;
	}else
	{ 
		return false;
	}
}

void CTLiteDoc::OnToolsRuntrafficassignment()
{
	if(m_bDYNASMARTDataSet)
		OnToolsPerformtrafficassignment();
	else
	{ //DTALite Settings
		if(EditTrafficAssignmentOptions() == true)
		OnToolsPerformtrafficassignment();
	}

}

void CTLiteDoc::OnImportodtripfile3columnformat()
{
	CDlg_ImportODDemand dlg;
	if(dlg.DoModal ()==IDOK)
	{
		CString strFilter = "OD 3-column Trip File|*.txt";
		CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,strFilter);
		//	dlg.m_ofn.nFilterIndex = m_nFilterLoad;
		HRESULT hResult = (int)dlg.DoModal();
		if (FAILED(hResult)) {
			return;
		}
		ReadTripTxtFile(dlg.GetPathName());
		OnToolsEditoddemandtable();
		m_bFitNetworkInitialized = false;
		UpdateAllViews(0);	

	}

}

void CTLiteDoc::OnToolsPerformscheduling()
{
	STARTUPINFO si = { 0 };  
	PROCESS_INFORMATION pi = { 0 };  

	si.cb = sizeof(si); 

	CWaitCursor curs;
	if(!SetCurrentDirectory(m_ProjectDirectory))
	{
		if(m_ProjectDirectory.GetLength()==0)
		{
			AfxMessageBox("The project directory has not been specified.");
			return;
		}
		CString msg;
		msg.Format ("The specified project directory %s does not exist.", m_ProjectDirectory);
		AfxMessageBox(msg);
		return;
	}

	CString sCommand;
	CString strParam;
	CTime ExeStartTime = CTime::GetCurrentTime();

	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	sCommand.Format("%s\\FastTrain.exe", pMainFrame->m_CurrentDirectory);

	ProcessExecute(sCommand, strParam, m_ProjectDirectory, true);

	CTime ExeEndTime = CTime::GetCurrentTime();

	CTimeSpan ts = ExeEndTime  - ExeStartTime;
	CString str_running_time;

	FILE* st = NULL;

	CString directory = m_ProjectDirectory;
	char simulation_short_summary1[200];
	char simulation_short_summary2[200];
	char simulation_short_summary3[200];

	fopen_s(&st,directory+"short_summary.csv","r");
	if(st!=NULL)
	{  
		fgets (simulation_short_summary1, 200 , st);
		fgets (simulation_short_summary2 , 200 , st);
		fgets (simulation_short_summary3, 200 , st);
		fclose(st);
	}

	str_running_time.Format ("Program execution has completed.\nSimulation Statistics: %s\n%s\n%sProgram execution time: %d hour(s) %d min(s) %d sec(s) \nDo you want to load the output now?",
		simulation_short_summary1,simulation_short_summary2, simulation_short_summary3,ts.GetHours(), ts.GetMinutes(), ts.GetSeconds());

	if( AfxMessageBox(str_running_time, MB_YESNO| MB_ICONINFORMATION)==IDYES)
	{
		LoadSimulationOutput();
		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnFileChangecoordinatestolong()
{
	CDlgNetworkAlignment  dlg;
	if(dlg.DoModal() ==IDOK)
	{
		if(dlg.m_Node1!=dlg.m_Node2 && m_NodeNumbertoIDMap.find(dlg.m_Node1)!= m_NodeNumbertoIDMap.end() && 
			m_NodeNumbertoIDMap.find(dlg.m_Node2)!= m_NodeNumbertoIDMap.end())
		{

			GDPoint m_Node1OrgPt, m_Node2OrgPt;
			m_Node1OrgPt = m_NodeIDMap[m_NodeNumbertoIDMap[dlg.m_Node1]]->pt;
			m_Node2OrgPt = m_NodeIDMap[m_NodeNumbertoIDMap[dlg.m_Node2]]->pt;

			float m_XScale = 1;
			float m_YScale = 1;

			if(m_Node1OrgPt.x - m_Node2OrgPt.x)
			{
				m_XScale = (dlg.m_NodeX1 - dlg.m_NodeX2)/(m_Node1OrgPt.x - m_Node2OrgPt.x);
			}

			if(m_Node1OrgPt.y - m_Node2OrgPt.y)
			{
				m_YScale = (dlg.m_NodeY1 - dlg.m_NodeY2)/(m_Node1OrgPt.y - m_Node2OrgPt.y);
			}

			float m_XOrigin = m_Node1OrgPt.x - dlg.m_NodeX1 /m_XScale;

			float m_YOrigin = m_Node1OrgPt.y - dlg.m_NodeY1 /m_YScale;

			// adjust node coordinates
			std::list<DTANode*>::iterator iNode;
			for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
			{

				(*iNode)->pt .x  = ((*iNode)->pt .x - m_XOrigin)*m_XScale;
				(*iNode)->pt .y  = ((*iNode)->pt .y - m_YOrigin)*m_YScale;
			}
			//adjust link cooridnates

			std::list<DTALink*>::iterator iLink;

			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				(*iLink)->m_FromPoint.x = ((*iLink)->m_FromPoint.x -m_XOrigin)*m_XScale;
				(*iLink)->m_FromPoint.y = ((*iLink)->m_FromPoint.y -m_YOrigin)*m_YScale;

				(*iLink)->m_ToPoint.x = ((*iLink)->m_ToPoint.x -m_XOrigin)*m_XScale;
				(*iLink)->m_ToPoint.y = ((*iLink)->m_ToPoint.y -m_YOrigin)*m_YScale;


				for(unsigned int si = 0; si< (*iLink)->m_Original_ShapePoints.size(); si++)
				{

					(*iLink)->m_Original_ShapePoints[si].x = ((*iLink)->m_Original_ShapePoints[si].x - m_XOrigin)*m_XScale;
					(*iLink)->m_Original_ShapePoints[si].y = ((*iLink)->m_Original_ShapePoints[si].y - m_YOrigin)*m_YScale;

				}

				for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
				{

					(*iLink)->m_ShapePoints[si].x = ((*iLink)->m_ShapePoints[si].x - m_XOrigin)*m_XScale;
					(*iLink)->m_ShapePoints[si].y = ((*iLink)->m_ShapePoints[si].y - m_YOrigin)*m_YScale;

				}

			}

			// zone layer
			std::map<int, DTAZone>	:: iterator itr;

			for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); ++itr)
			{
				for(unsigned int si = 0; si< itr->second.m_ShapePoints.size(); si++)
				{
					itr->second.m_ShapePoints[si].x = (itr->second.m_ShapePoints[si].x - m_XOrigin)*m_XScale;
					itr->second.m_ShapePoints[si].y = (itr->second.m_ShapePoints[si].y - m_YOrigin)*m_YScale;

				}

			}

			// image layer

		m_ImageX1 = (m_ImageX1- m_XOrigin)*m_XScale;
		m_ImageY1 = (m_ImageY1- m_YOrigin)*m_YScale;
		m_ImageX2 = (m_ImageX2- m_XOrigin)*m_XScale;
		m_ImageY2 =  (m_ImageY2- m_YOrigin)*m_YScale;

		m_ImageWidth = fabs(m_ImageX2 - m_ImageX1);
		m_ImageHeight = fabs(m_ImageY2 - m_ImageY1);

		m_ImageMoveSize = m_ImageWidth/2000.0f;

			CString str_result;
			str_result.Format ("The coordinates of %d nodes, %d links and %d zones have been adjusted to long/lat format.\nPleaes save the network to confirm the change.\nYou can use NEXTA_32.exe ->menu->Tools->GIS tools->Export GIS shape files to check the changed network on Google Maps",m_NodeSet.size(),m_LinkSet.size(),m_ZoneMap.size());
			AfxMessageBox(str_result, MB_ICONINFORMATION);
		}

		UpdateAllViews(0);
	}
}

void CTLiteDoc::OpenWarningLogFile(CString directory)
{
	m_WarningFile.open (directory+"warning.log", ios::out);
	if (m_WarningFile.is_open())
	{
		m_WarningFile.width(12);
		m_WarningFile.precision(3) ;
		m_WarningFile.setf(ios::fixed);
	}else
	{
		AfxMessageBox("File warning.log cannot be opened, and it might be locked by another program!");
	}
}
void CTLiteDoc::OnToolsExportopmodedistribution()
{

}

void CTLiteDoc::OnToolsEnumeratepath()
{

	CWaitCursor cws;

	int OD_index;

	int O_array[10]={53661,101846,165091,226988,209476,41017,54466,94991,78110,16589};

	int D_array[10]={144304,33737,70979,72725,79930,101989,89676,21233,84885,156041};

	float TravelTime_array[10]={132.3,112.87,65,92.12,124.03,147.83,152.58,82.9,54.63,65.49};

	for(OD_index = 0; OD_index<10; OD_index++)
	{
		m_OriginNodeID = m_NodeNumbertoIDMap[O_array[OD_index]];
		m_DestinationNodeID = m_NodeNumbertoIDMap[D_array[OD_index]];
		float TravelTimeBound  = TravelTime_array[OD_index];
		if(m_OriginNodeID>0 && m_DestinationNodeID>0)
		{
			if(m_pNetwork !=NULL)
			{
				delete m_pNetwork;
				m_pNetwork = NULL;
			}

			m_pNetwork = new DTANetworkForSP(m_NodeSet.size(), m_LinkSet.size(), 1, 1, m_AdjLinkSize);  //  network instance for single processor in multi-thread environment

			m_pNetwork->BuildPhysicalNetwork(&m_NodeSet, &m_LinkSet, m_RandomRoutingCoefficient, false);

			m_pNetwork->GenerateSearchTree (m_OriginNodeID,m_DestinationNodeID,m_NodeSet.size()*5,TravelTimeBound);

			FILE* st = NULL;

			//		CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			//			_T("Path file (*.csv)|*.csv|"));
			//		if(dlg.DoModal() == IDOK)
			//		{
			CString str;
			str.Format ("C:\\path_set_%d.csv",OD_index);
			fopen_s(&st,str,"w");

			int NodeList[1000];

			int PathNo = 0;

			int i;
			for(i = 0; i < m_pNetwork->m_TreeListTail; i++)
			{
				if(m_pNetwork->m_SearchTreeList[i].CurrentNode == m_DestinationNodeID)
				{
					int nodeindex = 0;
					NodeList[nodeindex++] = m_pNetwork->m_SearchTreeList[i].CurrentNode;
					int Pred = m_pNetwork->m_SearchTreeList[i].PredecessorNode ;

					while(Pred!=0)
					{
						NodeList[nodeindex++] = m_pNetwork->m_SearchTreeList[Pred].CurrentNode;

						Pred = m_pNetwork->m_SearchTreeList[Pred].PredecessorNode ;
					}
					NodeList[nodeindex++] = m_pNetwork->m_SearchTreeList[Pred].CurrentNode;

					fprintf(st,"%d,%d,", PathNo,nodeindex);

					for(int n = nodeindex-1; n>=0; n--)
					{
						fprintf(st,"%d,", m_NodeIDtoNumberMap[NodeList[n]]);
					}

					fprintf(st, "\n");
					PathNo++;
				}

			}
			fclose(st);
		}
	}

}

void CTLiteDoc::OnResearchtoolsExporttodtalitesensordataformat()
{
	CWaitCursor wc;
	int max_day = 23;
	FILE* st = NULL;

	for(int day = 0; day < max_day; day++)
	{
		CString str;
		str.Format( "SensorDataDay%03d.csv",day+1);
		fopen_s(&st,str,"w");
		if(st!=NULL)
		{
			std::list<DTALink*>::iterator iLink;

			fprintf(st,"Unix Timestamp (local time),StationID,Total Flow_per_obs_interval,Avg Density,Avg Speed\n");
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				if((*iLink)->m_bSensorData )
				{

					for(int t= day*1440; t<(day+1)*1440; t+=5)
					{
						//
						int hour = (t-day*1440)/60;
						int min =  (t-day*1440-hour*60);
						fprintf(st,"07/%02d/2010 %02d:%02d, %d %4.1f, 0, %4.1f\n", day, hour, min, (*iLink)->m_LinkNo+1,
							(*iLink)->m_LinkMOEAry[ t].SimulationLinkFlow/12, (*iLink)->m_LinkMOEAry[t].SimulationSpeed);
					}


				}
			}

			fclose(st);

		}

	}


}
void CTLiteDoc::OnScenarioConfiguration()
{
	// TODO: Add your command handler code here

	if (m_ProjectDirectory.GetLength() == 0 ) 
	{
		MessageBox(NULL,"No open project!",NULL,MB_ICONWARNING);
		return;
	}

	CDlgScenario dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();
}

void CTLiteDoc::OnMoeViewmoes()
{
	// TODO: Add your command handler code here

	CDlgMOETabView dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();
}

void CTLiteDoc::OnImportdataImport()
{
#ifndef _WIN64

	CDlg_ImportNetwork dlg;
	dlg.m_pDoc = this;
	if(dlg.DoModal() == IDOK)
	{
		OffsetLink();
		GenerateOffsetLinkBand();
		CalculateDrawingRectangle();
		m_bFitNetworkInitialized  = false;
		UpdateAllViews(0);

		if(m_ProjectDirectory.GetLength()==0 && m_NodeSet.size() > 0)  //with newly imported nodes
		{
			if(AfxMessageBox("Do you want to save the imported project now?",MB_YESNO|MB_ICONINFORMATION)==IDYES)
			{
				OnFileSaveProjectAs();
			}
			return;
		}

	}
#endif
}

float CTLiteDoc::FillODMatrixFromCSVFile(LPCTSTR lpszFileName)
{

	float total_demand = 0;

	// Convert a TCHAR string to a LPCSTR
	CT2CA pszConvertedAnsiString (lpszFileName);

	// construct a std::string using the LPCSTR input
	std::string strStd (pszConvertedAnsiString);


	CCSVParser parser_ODMatrix;

	if (parser_ODMatrix.OpenCSVFile(strStd))
	{
		while(parser_ODMatrix.ReadRecord())
		{

			int origin_zone_id, destination_zone_id;
			float number_of_vehicles;
			float starting_time_in_min = 0;
			float ending_time_in_min = 1440;

			if(parser_ODMatrix.GetValueByFieldName("from_zone_id",origin_zone_id) == false)
			{
				AfxMessageBox("Field from_zone_id cannot be found in the demand csv file.");
				return false;
			}

			if(parser_ODMatrix.GetValueByFieldName("to_zone_id",destination_zone_id) == false)
			{
				AfxMessageBox("Field to_zone_id cannot be found in the demand csv file.");
				return false;
			}

			DTADemand element;
			element.from_zone_id = origin_zone_id;
			element.to_zone_id = destination_zone_id;
			element.starting_time_in_min = 0;
			element.ending_time_in_min = 60;
			for(unsigned int type = 0; type < m_DemandTypeVector.size(); type++)
			{

				std::ostringstream  demand_string;
				demand_string << "number_of_trips_demand_type" << type+1;

				if(parser_ODMatrix.GetValueByFieldName(demand_string.str(),number_of_vehicles) == false)
				{
					CString str_number_of_vehicles_warning; 
					str_number_of_vehicles_warning.Format("Field demand_value_type%d cannot be found in the demand csv file.", type);
					AfxMessageBox(str_number_of_vehicles_warning);
					return false;
				}
				m_ZoneMap[origin_zone_id].m_ODDemandMatrix [destination_zone_id].SetValue (type, number_of_vehicles);
				element.number_of_vehicles_per_demand_type .push_back (number_of_vehicles);
				total_demand+= number_of_vehicles;

			}


		}
	}
	return total_demand;
}

CString CTLiteDoc::GetTableName(CString Tablename)
{
	CString EmptyString;

#ifndef _WIN64

	int nTables = m_Database.GetTableDefCount();


	for(int i = 0; i < 	nTables; i++)
	{// this accesses first sheet regardless of name.
		CDaoTableDefInfo TableInfo;

		m_Database.GetTableDefInfo(i, TableInfo);

		CString tablename=(TableInfo).m_strName;
		tablename.MakeUpper();
		if(tablename.Find(Tablename.MakeUpper(),0)>0)
		{
			return TableInfo.m_strName;
		}

	}

	CString ErrorMessage;
	ErrorMessage += "Table ";
	ErrorMessage += Tablename;
	ErrorMessage += " cannot be found in the given Excel file. Pleaes check.";

	AfxMessageBox(ErrorMessage, MB_ICONINFORMATION);

#endif
	return EmptyString;

}

CString CTLiteDoc::ConstructSQL(CString Tablename)
{
	CString strSQL;

	CString table_name  = GetTableName(Tablename);
	if(table_name.GetLength () > 0)
	{
		strSQL = "select * from [";
		strSQL += table_name;
		strSQL += "]";
		return strSQL;
	}
	return strSQL;

}
bool CTLiteDoc::CreateNetworkFromExcelFile()
{return true;}

bool CTLiteDoc::ImportSensorData()
{

	return true;

}
void CTLiteDoc::AdjustCoordinateUnitToMile()
{

	m_AdjLinkSize +=2;  // add two more elements to be safe

	std::list<DTANode*>::iterator iNode;
	//adjust XY coordinates if the corrdinate system is not consistenty
	if(fabs(m_UnitMile-1.00)>0.10)  // ask users if we need to adjust the XY coordinates
	{
		if(AfxMessageBox("The link length information in link.csv is not consistent with the X/Y coordinates in node.csv.\n Do you want to adjust the the X/Y coordinate unit to mile in node.csv?", MB_YESNO) == IDYES)

			for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
			{
				(*iNode)->pt.x = ((*iNode)->pt.x - min(m_NetworkRect.left,m_NetworkRect.right))*m_UnitMile;
				(*iNode)->pt.y = ((*iNode)->pt.y - min(m_NetworkRect.top, m_NetworkRect.bottom))*m_UnitMile;
			}

			std::list<DTALink*>::iterator iLink;
			for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				(*iLink)->m_FromPoint = m_NodeIDMap[(*iLink)->m_FromNodeID]->pt;
				(*iLink)->m_ToPoint = m_NodeIDMap[(*iLink)->m_ToNodeID]->pt;
			}

			m_UnitMile  = 1.0;
			m_UnitFeet = 1/5280.0;

			if(m_LongLatCoordinateFlag)
				m_UnitFeet = m_UnitMile/62/5280.0f;  // 62 is 1 long = 62 miles

			CalculateDrawingRectangle();
			UpdateAllViews(0);

	}
}



bool CTLiteDoc::ReadZoneShapeCSVFile(LPCTSTR lpszFileName)
{
	CString information_msg;
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	int zone_shape_point_count = 0;
	int id;
	if(st!=NULL)
	{
		while(!feof(st))
		{
			id			= g_read_integer(st); 
			if(id == -1)  // reach end of file
				break;

			int part_no			= g_read_integer(st);
			int point_id		= g_read_integer(st);

			DTAZone zone = m_ZoneMap[id];

			float x	= g_read_float(st);
			float y	= g_read_float(st);

			GDPoint	pt;
			pt.x = x;
			pt.y = y;

			zone.m_ShapePoints .push_back (pt);
			zone_shape_point_count ++;
			/*
			}else
			{
			CString msg;
			msg.Format ("zone %d in the zone shape csv file has not defined in the input node table.",id);
			AfxMessageBox(msg);
			fclose(st);
			return false;

			}
			*/
		}
		fclose(st);

		return true;
	}

	information_msg.Format ("%d nodes are loaded from the node csv file.",zone_shape_point_count,lpszFileName);

	return true;

}

bool CTLiteDoc::ReadNodeGeoFile(LPCTSTR lpszFileName)
{
	return ReadZoneShapeCSVFile(lpszFileName);

}


bool CTLiteDoc::ReadLinkGeoFile(LPCTSTR lpszFileName)
{
	CString information_msg;
	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"r");

	int link_shape_point_count = 0;

	int id = 0;
	if(st!=NULL)
	{
		while(!feof(st))
		{
			id			= g_read_integer(st);
			if(id == -1)  // reach end of file
				break;

			int number_of_feature_points			= g_read_integer(st);

			DTALink* pLink = FindLinkWithLinkNo (id);
			if(pLink!=NULL)
			{
				for(int i=0; i< number_of_feature_points; i++)
				{
					float x	= g_read_float(st);
					float y	= g_read_float(st);
					GDPoint	pt;
					pt.x = x;
					pt.y = y;

					pLink->m_ShapePoints .push_back (pt);
					link_shape_point_count ++;
				}

			}else
			{
				CString msg;
				msg.Format  ("link %d in the link shape csv file has not defined in the input shape table.",id);
				AfxMessageBox(msg,MB_ICONINFORMATION);
				fclose(st);
				return false;

			}
		}
		fclose(st);

	}

	information_msg.Format ("%d shape points are loaded from the link shape csv file.",link_shape_point_count,lpszFileName);

	std::list<DTALink*>::iterator iLink;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if( (*iLink)->m_ShapePoints .size() ==0)  // no shape points from external sources, use positoins from upstream and downstream nodes
		{
			(*iLink)->m_ShapePoints.push_back((*iLink)->m_FromPoint);
			(*iLink)->m_ShapePoints.push_back((*iLink)->m_ToPoint);

		}
	}


	return true;
}

void CTLiteDoc::OnMoeVehiclepathanalaysis()
{
	if(g_bValidDocumentChanged && g_pVehiclePathDlg!=NULL)  // either a new document is added or an old document is delete. 
	{
		//	delete g_pVehiclePathDlg; potential memory leak, but operator delete is problematic for gridlist
		g_pVehiclePathDlg= NULL;
		g_bValidDocumentChanged = false;
	}

	g_bShowVehiclePathDialog = !g_bShowVehiclePathDialog;

	if(g_bShowVehiclePathDialog)
	{
		if(g_pVehiclePathDlg==NULL)
		{
			g_pVehiclePathDlg = new CDlg_VehPathAnalysis();
			g_pVehiclePathDlg->m_pDoc = this;
			g_pVehiclePathDlg->Create(IDD_DIALOG_VEHICLE_PATH);
		}

		// update using pointer to the active document; 

		if(g_pVehiclePathDlg->GetSafeHwnd())
		{
			// we udpate the pointer list for document every time we open this link list window
			g_pVehiclePathDlg->m_pDoc = this;
			g_pVehiclePathDlg->ShowWindow(SW_HIDE);
			g_pVehiclePathDlg->ShowWindow(SW_SHOW);
			g_pVehiclePathDlg->FilterOriginDestinationPairs();
		}
	}else
	{
		if(g_pVehiclePathDlg!=NULL && g_pVehiclePathDlg->GetSafeHwnd())
		{
			g_pVehiclePathDlg->ShowWindow(SW_HIDE);
		}
	}

}

void CTLiteDoc::HighlightPath(	std::vector<int>	m_LinkVector, int DisplayID = 1)
{
	for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_DisplayLinkID = -1;
	}

	for (unsigned int l = 0; l < m_LinkVector.size(); l++)
	{
		DTALink* pLink = FindLinkWithLinkNo (m_LinkVector[l]);
		if(pLink!=NULL)
		{
			pLink->m_DisplayLinkID = DisplayID;
		}

	}
	UpdateAllViews(0);
}



void CTLiteDoc::HighlightSelectedVehicles(bool bSelectionFlag)
{
	CWaitCursor wc;

	for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_DisplayLinkID = -1;
		(*iLink)->m_NumberOfMarkedVehicles++;
	}

	if(bSelectionFlag) // selection
	{
		std::list<DTAVehicle*>::iterator iVehicle;

		bool bTraceFlag = true;

		for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++)
		{
			DTAVehicle* pVehicle = (*iVehicle);
			if(pVehicle->m_bMarked)
			{
				for(int link= 1; link<pVehicle->m_NodeSize; link++)
				{
					if(m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end())
					{
						m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo ]->m_DisplayLinkID = 1;
						m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo ]->m_NumberOfMarkedVehicles++;
					}
				}
			}		
		}
	} // de-selection: do nothing
	UpdateAllViews(0);

}

bool CTLiteDoc::ReadInputEmissionRateFile(LPCTSTR lpszFileName)
{
	CCSVParser parser_emission;
	if (parser_emission.OpenCSVFile(lpszFileName))
	{

		int line_no = 1;
		while(parser_emission.ReadRecord())
		{
			int vehicle_type;
			int opModeID;

			if(parser_emission.GetValueByFieldName("vehicle_type",vehicle_type) == false)
				break;
			if(parser_emission.GetValueByFieldName("opModeID",opModeID) == false)
				break;

			CEmissionRate element;
			if(parser_emission.GetValueByFieldName("meanBaseRate_TotalEnergy_(J/hr)",element.meanBaseRate_TotalEnergy) == false)
				break;
			if(parser_emission.GetValueByFieldName("meanBaseRate_CO2_(g/hr)",element.meanBaseRate_CO2) == false)
				break;
			if(parser_emission.GetValueByFieldName("meanBaseRate_NOX_(g/hr)",element.meanBaseRate_NOX) == false)
				break;
			if(parser_emission.GetValueByFieldName("meanBaseRate_CO_(g/hr)",element.meanBaseRate_CO) == false)
				break;
			if(parser_emission.GetValueByFieldName("meanBaseRate_HC_(g/hr)",element.meanBaseRate_HC) == false)
				break;

			ASSERT(vehicle_type < MAX_VEHICLE_TYPE_SIZE);
			ASSERT(opModeID < _MAXIMUM_OPERATING_MODE_SIZE);
			EmissionRateData[vehicle_type][opModeID] = element;
			line_no++;
		}
		m_AMSLogFile << "Read " << line_no << " emission rate elements from file "  << lpszFileName << endl; 

		CString msg;
		msg.Format("Imported %d emission rate elements from file %s",line_no,lpszFileName);
		m_MessageStringVector.push_back (msg);

		if(line_no>=1)
			return true;
		else
			return false;
	}
	CString msg;
	msg.Format("Imported 0 emission rate element from file %s",lpszFileName);
	m_MessageStringVector.push_back (msg);

	return false;

}




void CTLiteDoc::OnFileImportDemandFromCsv()
{
	static char BASED_CODE szFilter[] = "CSV (*.csv)|*.csv||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		FillODMatrixFromCSVFile(dlg.GetPathName ());
	}

	CalculateDrawingRectangle();
	m_bFitNetworkInitialized  = false;
	UpdateAllViews(0);
}


void CTLiteDoc::OnImportSensorData()
{
	CDlg_ImportPointSensor dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();

	CalculateDrawingRectangle();
	m_bFitNetworkInitialized  = false;
	UpdateAllViews(0);
}

void CTLiteDoc::OnImportLinkmoe()
{
	static char BASED_CODE szFilter[] = "Simulated Link MOE CSV (*.csv)|*.csv||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		ReadSimulationLinkMOEData(dlg.GetPathName ());
	}

	CalculateDrawingRectangle();
	m_bFitNetworkInitialized  = false;
	UpdateAllViews(0);
}

void CTLiteDoc::OnImportVehiclefile()
{
	static char BASED_CODE szFilter[] = "Simulated Vehicle Trajectory CSV (*.csv)|*.csv||";
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		szFilter);
	if(dlg.DoModal() == IDOK)
	{
		ReadVehicleCSVFile(dlg.GetPathName ());
	}

	CalculateDrawingRectangle();
	m_bFitNetworkInitialized  = false;
	UpdateAllViews(0);
}

void CTLiteDoc::OnLinkmoeEmissions()
{
	m_LinkMOEMode = MOE_emissions;
	GenerateOffsetLinkBand();
	ShowLegend(false);

	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateLinkmoeEmissions(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_emissions);

}

void CTLiteDoc::OnLinkmoeReliability()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::OnUpdateLinkmoeReliability(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CTLiteDoc::OnLinkmoeSafety()
{
	m_LinkMOEMode = MOE_safety;

	if(m_bRunCrashPredictionModel == false) 
	{
		OnSafetyplanningtoolsRun();
	}

	ShowLegend(true);
	GenerateOffsetLinkBand();
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateLinkmoeSafety(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_safety);
}

void CTLiteDoc::OnImportAgentFile()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::OnImportNgsimFile()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
	pMainFrame->OnShowTimetable();

}

void CTLiteDoc::OpenCSVFileInExcel(CString filename)
{
	/*
	CXLEzAutomation XL;
	//Close Excel if failed to open file 
	if(!XL.OpenExcelFile(filename))
	{
	XL.ReleaseExcel();

	return;
	}
	*/

	//	CString on_line_address;
	//	on_line_address.Format ("http://www.google.com/fusiontables/DataSource?dsrcid=%s",m_LinkTableID);

	HINSTANCE result = ShellExecute(NULL, _T("open"), filename, NULL,NULL, SW_SHOW);



}

bool CTLiteDoc::WriteSubareaFiles()
{
	// update m_ProjectDirectory
	FILE* st = NULL;
	//	cout << "Reading file node.csv..."<< endl;

	CString directory;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

	fopen_s(&st,directory+"input_subarea_node.csv","w");
	if(st!=NULL)
	{

		fprintf(st, "name,node_id,control_type,geometry\n");
		for (unsigned i = 0; i < m_SubareaNodeSet.size(); i++)
		{
			DTANode* pNode = m_SubareaNodeSet[i];
			fprintf(st, "%s,%d,%d,\"<Point><coordinates>%f,%f</coordinates></Point>\"\n", pNode->m_Name.c_str (), pNode->m_NodeNumber , pNode->m_ControlType, pNode->pt .x, pNode->pt .y );
		}

		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_subarea_node.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}

	fopen_s(&st,directory+"input_subarea_link.csv","w");
	if(st!=NULL)
	{
		std::list<DTALink*>::iterator iLink;
		for (iLink = m_SubareaLinkSet.begin(); iLink != m_SubareaLinkSet.end(); iLink++)
		{
			if((*iLink)->m_AVISensorFlag == false)
			{

				(*iLink)->m_ReliabilityIndex  = (*iLink)->m_Length * (*iLink)->m_NumberOfLanes;
				(*iLink)->m_SafetyIndex   = (*iLink)->m_link_type * (*iLink)->m_NumberOfLanes;
				(*iLink)->m_MobilityIndex = (*iLink)->m_SpeedLimit+10*g_GetRandomRatio();
				(*iLink)->m_EmissionsIndex = (*iLink)->ObtainHistCO2Emissions(0);
				float predicted_speed  = (*iLink)->m_SpeedLimit * g_GetRandomRatio();
			}

		}

		fprintf(st,"congestion_index,link_id,from_node_id,to_node_id,direction,length_in_mile,number_of_lanes,speed_limit_in_mph,lane_capacity_in_vhc_per_hour,link_type,jam_density_in_vhc_pmpl,wave_speed_in_mph,mode_code,grade,geometry\n");
		for (iLink = m_SubareaLinkSet.begin(); iLink != m_SubareaLinkSet.end(); iLink++)
		{
			if((*iLink)->m_AVISensorFlag == false)
			{
				fprintf(st,"%d,%d,%d,%d,%d,%f,%d,%f,%f,%d,%f,%f,%s,%f,",
					(*iLink)->m_MobilityIndex,
					(*iLink)->m_LinkID, 
					(*iLink)->m_FromNodeNumber, 
					(*iLink)->m_ToNodeNumber , (*iLink)->m_Direction,(*iLink)->m_Length ,(*iLink)->m_NumberOfLanes ,(*iLink)->m_SpeedLimit,(*iLink)->m_LaneCapacity ,(*iLink)->m_link_type,(*iLink)->m_Kjam, (*iLink)->m_Wave_speed_in_mph,(*iLink)->m_Mode_code.c_str (), (*iLink)->m_Grade);
				fprintf(st,"\"<LineString><coordinates>");

				for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
				{
					fprintf(st,"%f,%f,0.0",(*iLink)->m_ShapePoints[si].x, (*iLink)->m_ShapePoints[si].y);

					if(si!=(*iLink)->m_ShapePoints.size()-1)
						fprintf(st," ");
				}


				fprintf(st,"</coordinates></LineString>\"\n");
			}
		}

		fclose(st);
	}else
	{
		AfxMessageBox("Error: File input_subarea_link.csv cannot be opened.\nIt might be currently used and locked by EXCEL.");
		return false;
	}

	return true;
}

void CTLiteDoc::OnImportAvi()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::OnImportGps33185()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::OnImportVii()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::OnImportWeather33188()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::OnImportGps()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::OnImportWorkzone()
{
	OnScenarioConfiguration();
}

void CTLiteDoc::OnImportIncident()
{
	OnScenarioConfiguration();
}

void CTLiteDoc::OnImportWeather()
{
	OnScenarioConfiguration();
}

void CTLiteDoc::OnImportPricing()
{
	OnScenarioConfiguration();
}

void CTLiteDoc::OnImportAtis()
{
	OnScenarioConfiguration();
}

void CTLiteDoc::OnImportBus()
{
	OnScenarioConfiguration();
}

void CTLiteDoc::OnLinkAddIncident()
{
	if(m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= m_LinkNoMap [m_SelectedLinkNo];
	if(pLink!=NULL)
	{

		ReadIncidentScenarioData();

		CapacityReduction cs;
		cs.bWorkzone  = false; 

		cs.ScenarioNo =  0; 
		cs.StartDayNo  = 20;
		cs.EndDayNo	   = 20;
		cs.StartTime = 600;
		cs.EndTime = 640;
		cs.LaneClosureRatio= 60;
		cs.SpeedLimit = 30;

		pLink->CapacityReductionVector.push_back(cs);


		WriteIncidentScenarioData();

		CDlgScenario dlg(INCIDENT);
		dlg.m_pDoc = this;

		dlg.DoModal();

		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnLinkAddWorkzone()
{
	if(m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= m_LinkNoMap [m_SelectedLinkNo];
	if(pLink!=NULL)
	{

		ReadWorkZoneScenarioData();

		CapacityReduction cs;
		cs.bWorkzone  = true; 

		cs.ScenarioNo =  0; 
		cs.StartDayNo  = 0;
		cs.EndDayNo	   = 100;
		cs.StartTime = 0;
		cs.EndTime = 1440;
		cs.LaneClosureRatio= 50;
		cs.SpeedLimit = 50;

		pLink->CapacityReductionVector.push_back(cs);


		WriteWorkZoneScenarioData();

		CDlgScenario dlg(WORKZONE);
		dlg.m_pDoc = this;

		dlg.DoModal();

		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnLinkAddvms()
{
	if(m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= m_LinkNoMap [m_SelectedLinkNo];
	if(pLink!=NULL)
	{

		ReadVMSScenarioData();

		// add VMS
		MessageSign ms;

		ms.ScenarioNo = 0;
		ms.StartDayNo = 0;
		ms.EndDayNo  = 100;
		ms.StartTime = 0;
		ms.EndTime = 1440;
		ms.ResponsePercentage= 20;
		pLink->MessageSignVector.push_back(ms);

		WriteVMSScenarioData();

		CDlgScenario dlg(DYNMSGSIGN);
		dlg.m_pDoc = this;

		dlg.DoModal();

		UpdateAllViews(0);
	}

}

int CTLiteDoc::ReadLink_basedTollScenarioData()
{
	FILE* st = NULL;
	int i =0;
	// toll
	CString toll_file = m_ProjectDirectory+"Scenario_Link_Based_Toll.csv";
	fopen_s(&st,toll_file,"r");
	if(st!=NULL)
	{
		// reset
		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->TollVector .clear ();
		}

		while(true)
		{
			int usn  = g_read_integer(st,false);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st,false);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn,toll_file );

			if(plink!=NULL)
			{
				DTAToll tl;
				tl.ScenarioNo  = g_read_integer(st);
				tl.StartDayNo   = g_read_integer(st);
				tl.EndDayNo    = g_read_integer(st);

				tl.StartTime = g_read_integer(st);
				tl.EndTime = g_read_integer(st);
				tl.TollRate[1]= g_read_float(st);
				tl.TollRate[2]= g_read_float(st);
				tl.TollRate[3]= g_read_float(st);
				tl.TollRate[4]= g_read_float(st);
				plink->TollVector.push_back(tl);
				i++;
			}
		}

		fclose(st);
	}
	return i;
}

bool CTLiteDoc::WriteLink_basedTollScenarioData()
{

	FILE* st = NULL;
	int i =0;
	//  Dynamic Message Sign
	fopen_s(&st,m_ProjectDirectory+"Scenario_Link_Based_Toll.csv","w");
	if(st!=NULL)
	{
		// reset

		fprintf(st, "Link,Scenario No,Start Day,End Day,Start Time in Min,End Time in min,Charge for LOV ($),Charge for HOV ($),Charge for Truck ($),Charge for Intermodal ($) \n");

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{

			for(unsigned int i = 0; i < (*iLink)->TollVector  .size(); i++)
			{
				fprintf(st,"\"[%d,%d]\",%d,%d,%d,%3.0f,%3.0f,%3.1f,%3.1f,%3.1f,%3.1f\n", (*iLink)->m_FromNodeNumber , (*iLink)->m_ToNodeNumber ,
					(*iLink)->TollVector[i].ScenarioNo  ,
					(*iLink)->TollVector[i].StartDayNo  ,
					(*iLink)->TollVector[i].EndDayNo  ,(*iLink)->TollVector[i].StartTime , (*iLink)->TollVector[i].EndTime ,
					(*iLink)->TollVector[i].TollRate [1],(*iLink)->TollVector[i].TollRate [2],(*iLink)->TollVector[i].TollRate [3], (*iLink)->TollVector[i].TollRate [4]);
			}
		}

		fclose(st);
	}

	return true;
}

bool CTLiteDoc::WriteIncidentScenarioData()
{

	FILE* st = NULL;
	int i =0;
	//  incident scenario
	fopen_s(&st,m_ProjectDirectory+"Scenario_Incident.csv","w");
	if(st!=NULL)
	{
		// reset

		fprintf(st, "Link,Scenario No, Day No,Start Time in Min,End Time in min,Capacity Reduction Percentage (%),Speed Limit (mph)\n");

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{

			for(unsigned int i = 0; i < (*iLink)->CapacityReductionVector  .size(); i++)
			{

				CapacityReduction element  = (*iLink)->CapacityReductionVector[i];

				if(element.bWorkzone == false)
				{
					fprintf(st,"\"[%d,%d]\",%d,%d,%3.0f,%3.0f,%3.1f,%3.1f\n", (*iLink)->m_FromNodeNumber , (*iLink)->m_ToNodeNumber ,
						element.ScenarioNo ,element.StartDayNo , element.StartTime , element.EndTime ,element.LaneClosureRatio, element.SpeedLimit );
				}
			}
		}

		fclose(st);
	}

	return true;
}

bool CTLiteDoc::WriteWorkZoneScenarioData()
{
	FILE* st = NULL;
	int i =0;
	//  incident scenario
	fopen_s(&st,m_ProjectDirectory+"Scenario_Work_Zone.csv","w");
	if(st!=NULL)
	{
		// reset

		fprintf(st, "Link,Scenario No,Start Day,End Day,Start Time in Min,End Time in min,Capacity Reduction Percentage (%),Speed Limit (mph)\n");

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{

			for(unsigned int i = 0; i < (*iLink)->CapacityReductionVector  .size(); i++)
			{

				CapacityReduction element  = (*iLink)->CapacityReductionVector[i];

				if(element.bWorkzone == true)
				{
					fprintf(st,"\"[%d,%d]\",%d,%d,%d,%3.0f,%3.0f,%3.1f,%3.1f\n", (*iLink)->m_FromNodeNumber , (*iLink)->m_ToNodeNumber ,
						element.ScenarioNo,element.StartDayNo  , element.EndDayNo , element.StartTime , element.EndTime ,element.LaneClosureRatio, element.SpeedLimit );
				}
			}
		}

		fclose(st);
	}

	return true;
}

int CTLiteDoc::ReadWorkZoneScenarioData()
{
	// clean up all scenario data first
	for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->CapacityReductionVector .clear();

	}
	int i = 0;

	CString workzone_file = m_ProjectDirectory+"Scenario_Work_Zone.csv";
	int error_count  = 0; 
	FILE* st =NULL;
	fopen_s(&st,workzone_file,"r");
	if(st!=NULL)
	{
		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn,workzone_file );

			if(plink!=NULL)
			{
				CapacityReduction cs;
				cs.bWorkzone  = true; 

				cs.ScenarioNo =  g_read_integer(st); 
				cs.StartDayNo  = g_read_integer(st);
				cs.EndDayNo	   = g_read_integer(st);
				cs.StartTime = g_read_float(st);
				cs.EndTime = g_read_float(st);
				cs.LaneClosureRatio= g_read_float(st);
				cs.SpeedLimit = g_read_float(st);

				plink->CapacityReductionVector.push_back(cs);
				i++;
			}else
			{
				error_count ++;
				if(error_count >= 3) break; // no more than 3 warnings

			}
		}
		fclose(st);
	}

	return i;
}

int CTLiteDoc::ReadIncidentScenarioData()
{
	// clean up all scenario data first
	for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->CapacityReductionVector .clear();

	}

	CString incident_file = m_ProjectDirectory+"Scenario_Incident.csv";
	FILE* st;
	fopen_s(&st,incident_file,"r");
	int error_count  = 0;

	int i = 0;
	if(st!=NULL)
	{
		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn,incident_file );

			if(plink!=NULL)
			{
				CapacityReduction cs;
				cs.bIncident = true; 

				cs.ScenarioNo = g_read_integer(st);
				cs.StartDayNo = g_read_integer(st);
				cs.EndDayNo = cs.StartDayNo;
				cs.StartTime = g_read_integer(st);
				cs.EndTime = g_read_integer(st);
				cs.LaneClosureRatio= g_read_float(st);
				cs.SpeedLimit = g_read_float(st);
				plink->CapacityReductionVector.push_back(cs);
				i++;
			}else
			{
				error_count ++;
				if(error_count >= 3) break; // no more than 3 warnings
			}

		}
		fclose(st);
	}

	return i;
}

int CTLiteDoc::ReadVMSScenarioData()
{
	// clean up all scenario data first
	for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->MessageSignVector.clear ();

	}

	FILE* st = NULL;
	int i =0;
	//  Dynamic Message Sign
	CString VMS_file = m_ProjectDirectory+"Scenario_Dynamic_Message_Sign.csv";
	fopen_s(&st,VMS_file,"r");
	if(st!=NULL)
	{
		// reset
		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->MessageSignVector.clear ();
		}

		while(true)
		{
			int usn  = g_read_integer(st);
			if(usn == -1)
				break;

			int dsn =  g_read_integer(st);

			DTALink* plink = FindLinkWithNodeNumbers(usn,dsn,VMS_file );

			if(plink!=NULL)
			{
				MessageSign ms;
				ms.ScenarioNo = g_read_integer(st);
				ms.StartDayNo  = g_read_integer(st);
				ms.EndDayNo  = g_read_integer(st);
				ms.StartTime = g_read_integer(st);
				ms.EndTime = g_read_integer(st);
				ms.ResponsePercentage= g_read_float(st);
				plink->MessageSignVector.push_back(ms);
				i++;
			}
		}

		fclose(st);
	}

	return i;
}
bool CTLiteDoc::WriteVMSScenarioData()
{
	FILE* st = NULL;
	int i =0;
	//  Dynamic Message Sign
	fopen_s(&st,m_ProjectDirectory+"Scenario_Dynamic_Message_Sign.csv","w");
	if(st!=NULL)
	{
		// reset
		fprintf(st, "Link,Scenario No,Start Day No,End Day No,Start Time in Min,End Time in min,Responce Percentage (%%)\n");

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{

			for(unsigned int i = 0; i < (*iLink)->MessageSignVector .size(); i++)
			{
				fprintf(st,"\"[%d,%d]\",%d,%d,%d,%3.0f,%3.0f,%3.1f\n", (*iLink)->m_FromNodeNumber , (*iLink)->m_ToNodeNumber ,
					(*iLink)->MessageSignVector[i].ScenarioNo ,(*iLink)->MessageSignVector[i].StartDayNo , (*iLink)->MessageSignVector[i].EndDayNo , 
					(*iLink)->MessageSignVector[i].StartTime , (*iLink)->MessageSignVector[i].EndTime ,(*iLink)->MessageSignVector[i].ResponsePercentage);
			}
		}

		fclose(st);
	}

	return true;
}

void CTLiteDoc::OnImportLinklayerinkml()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Extenral Link Layer Data in KML/CSV Format (*.csv)|*.csv|"));
	if(dlg.DoModal() == IDOK)
	{
		ReadLinkCSVFile(dlg.GetPathName(),true, false);

	}
	OffsetLink();
	CalculateDrawingRectangle();
	m_bFitNetworkInitialized  = false;
}

void CTLiteDoc::OnEditOffsetlinks()
{
	m_bLinkToBeShifted = true;
	OffsetLink();
	UpdateAllViews(0);
}


void CTLiteDoc::OnImportSubarealayerformapmatching()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Traffic network project for reference network(*.tnp)|*.tnp|"));
	if(dlg.DoModal() == IDOK)
	{
		CWaitCursor wait;
		CString ProjectFile = dlg.GetPathName();
		CString Second_Project_directory = ProjectFile.Left(ProjectFile.ReverseFind('\\') + 1);

		if(strcmp(Second_Project_directory, m_ProjectDirectory)==0)
		{
			AfxMessageBox("Please ensure the reference network data set is loaded from a folder different from the current project folder.");

			return ;
		}

		ReadNodeCSVFile(Second_Project_directory+"input_node.csv", 1);  // 1 for subarea layer
		ReadLinkCSVFile(Second_Project_directory+"input_link.csv",false, 1);  // 1 for subarea layer
		OffsetLink();
		CalculateDrawingRectangle();
		m_bFitNetworkInitialized  = false;
		UpdateAllViews(0);
	}

}

void CTLiteDoc::ChangeNetworkCoordinates(int LayerNo, float XScale, float YScale, float delta_x, float delta_y)
{
	GDRect NetworkRect;

	bool bRectInitialized = false;

	for (std::list<DTANode*>::iterator iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		if((*iNode)->m_Connections >0 && (*iNode)->m_LayerNo == LayerNo)   // for selected layer only
		{
			if(!bRectInitialized)
			{
				NetworkRect.left = (*iNode)->pt.x ;
				NetworkRect.right = (*iNode)->pt.x;
				NetworkRect.top = (*iNode)->pt.y;
				NetworkRect.bottom = (*iNode)->pt.y;
				bRectInitialized = true;
			}

			NetworkRect.Expand((*iNode)->pt);
		}

	}

	float m_XOrigin = NetworkRect.Center ().x ;

	float m_YOrigin = NetworkRect.Center ().y ;

	// adjust node coordinates
	std::list<DTANode*>::iterator iNode;
	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{

		if((*iNode)->m_LayerNo == LayerNo)  // for selected layer only
		{
			(*iNode)->pt .x  = ((*iNode)->pt .x - m_XOrigin)*XScale + m_XOrigin + delta_x;
			(*iNode)->pt .y  = ((*iNode)->pt .y - m_YOrigin)*YScale + m_YOrigin + delta_y;
		}
	}

	//adjust link cooridnates

	std::list<DTALink*>::iterator iLink;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_LayerNo == LayerNo)   // for selected layer only
		{

			(*iLink)->m_FromPoint.x = ((*iLink)->m_FromPoint.x -m_XOrigin)*XScale  + m_XOrigin + delta_x;
			(*iLink)->m_FromPoint.y = ((*iLink)->m_FromPoint.y -m_YOrigin)*YScale  + m_YOrigin + delta_y;

			(*iLink)->m_ToPoint.x = ((*iLink)->m_ToPoint.x -m_XOrigin)*XScale + m_XOrigin + delta_x;
			(*iLink)->m_ToPoint.y = ((*iLink)->m_ToPoint.y -m_YOrigin)*YScale  + m_YOrigin + delta_y;

			for(unsigned int si = 0; si< (*iLink)->m_ShapePoints.size(); si++)
			{

				(*iLink)->m_ShapePoints[si].x = ((*iLink)->m_ShapePoints[si].x - m_XOrigin)*XScale  + m_XOrigin + delta_x;
				(*iLink)->m_ShapePoints[si].y = ((*iLink)->m_ShapePoints[si].y - m_YOrigin)*YScale  + m_YOrigin + delta_y;

			}

			for(unsigned int si = 0; si< (*iLink)->m_BandLeftShapePoints.size(); si++)
			{

				(*iLink)->m_BandLeftShapePoints[si].x = ((*iLink)->m_BandLeftShapePoints[si].x - m_XOrigin)*XScale  + m_XOrigin + delta_x;
				(*iLink)->m_BandLeftShapePoints[si].y = ((*iLink)->m_BandLeftShapePoints[si].y - m_YOrigin)*YScale  + m_YOrigin + delta_y;

			}	

			for(unsigned int si = 0; si< (*iLink)->m_BandRightShapePoints.size(); si++)
			{

				(*iLink)->m_BandRightShapePoints[si].x = ((*iLink)->m_BandRightShapePoints[si].x - m_XOrigin)*XScale  + m_XOrigin + delta_x;
				(*iLink)->m_BandRightShapePoints[si].y = ((*iLink)->m_BandRightShapePoints[si].y - m_YOrigin)*YScale  + m_YOrigin + delta_y;

			}	


		}

	}
	//	CalculateDrawingRectangle();

	//	m_bFitNetworkInitialized  = false;

}
void CTLiteDoc::OnFileOpenNetworkOnly()
{
	static char BASED_CODE szFilter[] = "NEXTA Data Files (*.dws;*.tnp)|*.dws; *.tnp|DYNASMART Workspace Files (*.dws)|*.dws|Transportation Network Projects (*.tnp)|*.tnp|All Files (*.*)|*.*||";

	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);

	if(dlg.DoModal() == IDOK)
	{
		CWaitCursor wait;

		CString ProjectFileName = dlg.GetPathName ();

		if(ProjectFileName.Find("tnp")>=0)  //Transportation network project format
		{
			OnOpenTrafficNetworkDocument(ProjectFileName,true);
		}else if(ProjectFileName.Find("dws")>=0)  //DYNASMART-P format
		{
			OnOpenDYNASMARTProject(ProjectFileName,true);
		}
		CDlgFileLoading dlg;
		dlg.m_pDoc = this;
		dlg.DoModal ();

		UpdateAllViews(0);

	}

}

void CTLiteDoc::OnLinkAddlink()
{
	if(m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= m_LinkNoMap [m_SelectedLinkNo];
	if(pLink!=NULL)
	{

		ReadLink_basedTollScenarioData();

		// add toll
		DTAToll toll;

		toll.StartTime = 0;
		toll.EndTime = 1440;
		toll.TollRate[1] = 0.5;
		toll.TollRate[2] = 0.5;
		toll.TollRate[3] = 0.5;
		toll.TollRate[4] = 0;

		pLink->TollVector.push_back(toll);

		WriteLink_basedTollScenarioData();

		CDlgScenario dlg;
		dlg.m_SelectTab = 3;
		dlg.m_pDoc = this;

		dlg.DoModal();

		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnLinkAddhovtoll()
{
	if(m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= m_LinkNoMap [m_SelectedLinkNo];
	if(pLink!=NULL)
	{

		ReadLink_basedTollScenarioData();

		// add toll
		DTAToll toll;

		toll.StartTime = 0;
		toll.EndTime = 1440;
		toll.TollRate[1] = 999;
		toll.TollRate[2] = 0;
		toll.TollRate[3] = 999;

		pLink->TollVector.push_back(toll);

		WriteLink_basedTollScenarioData();

		CDlgScenario dlg(0);
		dlg.m_pDoc = this;

		dlg.m_SelectTab = 3;
		dlg.DoModal();

		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnLinkAddhottoll()
{
	if(m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= m_LinkNoMap [m_SelectedLinkNo];
	if(pLink!=NULL)
	{

		ReadLink_basedTollScenarioData();

		// add toll
		DTAToll toll;

		toll.StartTime = 0;
		toll.EndTime = 1440;
		toll.TollRate[1] = 0.5;
		toll.TollRate[2] = 0;
		toll.TollRate[3] = 0.5;

		pLink->TollVector.push_back(toll);

		WriteLink_basedTollScenarioData();

		CDlgScenario dlg(0);
		dlg.m_pDoc = this;

		dlg.m_SelectTab = 3;
		dlg.DoModal();

		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnLinkAddtolledexpresslane()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::OnLinkConvertgeneralpurposelanetotolledlane()
{
	// TODO: Add your command handler code here
}

CString CTLiteDoc::GetTimeStampStrFromIntervalNo(int time_interval, bool with_single_quote)
{
	CString str;
	int hour = time_interval/4;
	int min = (time_interval - hour*4)*15;

	if(with_single_quote)
	{
		if(hour<10)
			str.Format ("'0%d:%02d",hour,min);
		else
			str.Format ("'%2d:%02d",hour,min);
	}else  //without typewriter single quotes
	{
		if(hour<10)
			str.Format ("0%d:%02d",hour,min);
		else
			str.Format ("%2d:%02d",hour,min);
	}


	return str;

}


CString CTLiteDoc::GetTimeStampFloatingPointStrFromIntervalNo(int time_interval)
{
	CString str;
	int hour = time_interval/4;
	int min = (time_interval - hour*4)*15;

	str.Format ("%dh%02d",hour,min);

	return str;

}

CString CTLiteDoc::GetTimeStampString(int time_stamp_in_min)
{
	CString str;
	int hour = time_stamp_in_min/60;
	int min = time_stamp_in_min - hour*60;

	if(hour<12)
		str.Format ("%02d:%02d AM",hour,min);
	else if(hour==12)
		str.Format ("%02d:%02d PM",12,min);
	else
		str.Format ("%02d:%02d PM",hour-12,min);

	return str;

}

CString CTLiteDoc::GetTimeStampString24HourFormat(int time_stamp_in_min)
{
	CString str;
	int hour = time_stamp_in_min/60;
	int min = time_stamp_in_min - hour*60;

	str.Format ("%2d:%02d",hour,min);

	return str;

}
void CTLiteDoc::OnProjectEdittime()
{
}

int CTLiteDoc::FindClassificationNo(DTAVehicle* pVehicle, VEHICLE_X_CLASSIFICATION x_classfication)
{

	int index = -1;  // no classification

	switch(x_classfication)
	{
	case CLS_all_vehicles: index =0 ; break;
	case CLS_pricing_type: index = pVehicle->m_PricingType ; break;
	case CLS_VOT_10: index = pVehicle->m_VOT /10 ; break;
	case CLS_VOT_15: index = pVehicle->m_VOT /15 ; break;
	case CLS_VOT_10_SOV: 
		if(pVehicle->m_PricingType != 1)
			index =  -1;  // no considered.
		else
			index = pVehicle->m_VOT /10 ; 
		break;
	case CLS_VOT_10_HOV: 
		if(pVehicle->m_PricingType != 2)
			index =  -1;  // no considered.
		else
			index = pVehicle->m_VOT /10 ; 
		break;
	case CLS_VOT_10_truck: 
		if(pVehicle->m_PricingType != 3)
			index =  -1;  // no considered.
		else
			index = pVehicle->m_VOT /10 ; 
		break;
	case CLS_time_interval_15_min: index = pVehicle->m_DepartureTime /15; break;
	case CLS_time_interval_30_min: index = pVehicle->m_DepartureTime /30; break;
	case CLS_time_interval_60_min: index = pVehicle->m_DepartureTime /60; break;
	case CLS_time_interval_2_hour: index = pVehicle->m_DepartureTime /120; break;
	case CLS_time_interval_4_hour: index = pVehicle->m_DepartureTime /240; break;

	case CLS_distance_bin_0_2: index = pVehicle->m_Distance /0.2; break;
	case CLS_distance_bin_1: index = pVehicle->m_Distance /1; break;

	case CLS_distance_bin_2: index = pVehicle->m_Distance /2; break;
	case CLS_distance_bin_5: index = pVehicle->m_Distance /5; break;
	case CLS_distance_bin_10: index = pVehicle->m_Distance /10; break;
	case CLS_travel_time_bin_2: index = pVehicle->m_TripTime /2; break;
	case CLS_travel_time_bin_5: index = pVehicle->m_TripTime /5; break;
	case CLS_travel_time_bin_10: index = pVehicle->m_TripTime /10; break;
	case CLS_travel_time_bin_30: index = pVehicle->m_TripTime /30; break;

	case CLS_information_class: index = pVehicle->m_InformationClass ; break;
	case CLS_vehicle_type: index = pVehicle->m_VehicleType  ; break;
	default: 
		TRACE("ERROR. No classification available!");
	};

	return index;
}


CString CTLiteDoc::FindClassificationLabel(VEHICLE_X_CLASSIFICATION x_classfication, int index)
{

	CString label;

	switch(x_classfication)
	{
	case CLS_pricing_type: 
		label = GetPricingTypeStr(index);
		break;

	case CLS_VOT_10: 
	case CLS_VOT_10_SOV:
	case CLS_VOT_10_HOV:
	case CLS_VOT_10_truck:
		label.Format ("$%d-$%d",index*10,(index+1)*10);
		break;

	case CLS_VOT_15:
		label.Format ("$%d-$%d",index*15,(index+1)*15);
		break;

	case CLS_time_interval_15_min: 
		label.Format ("%s-%s",GetTimeStampStrFromIntervalNo(index,false),GetTimeStampStrFromIntervalNo((index+1),false));
		break;

	case CLS_time_interval_30_min: 
		label.Format ("%s-%s",GetTimeStampStrFromIntervalNo(index*2,false),GetTimeStampStrFromIntervalNo((index+1)*2,false));
		break;

	case CLS_time_interval_60_min: 
		label.Format ("%s-%s",GetTimeStampStrFromIntervalNo(index*4,false),GetTimeStampStrFromIntervalNo((index+1)*4,false));
		break;

	case CLS_time_interval_2_hour: 
		label.Format ("%s-%s",GetTimeStampStrFromIntervalNo(index*8,false),GetTimeStampStrFromIntervalNo((index+1)*8,false));
		break;

	case CLS_time_interval_4_hour: 
		label.Format ("%s-%s",GetTimeStampStrFromIntervalNo(index*16,false),GetTimeStampStrFromIntervalNo((index+1)*16,false));
		break;

	case CLS_distance_bin_0_2: 
		label.Format ("%.1f-%.1f",index*0.2,(index+1)*0.2);
		break;
	case CLS_distance_bin_1: 
		label.Format ("%d-%d",index*1,(index+1)*1);
		break;
	case CLS_distance_bin_2: 
		label.Format ("%d-%d",index*2,(index+1)*2);
		break;

	case CLS_distance_bin_5: 
		label.Format ("%d-%d",index*5,(index+1)*5);
		break;
	case CLS_distance_bin_10: 
		label.Format ("%d-%d",index*10,(index+1)*10);
		break;
	case CLS_travel_time_bin_2: 
		label.Format ("%d-%d",index*2,(index+1)*2);
		break;
	case CLS_travel_time_bin_5: 
		label.Format ("%d-%d",index*5,(index+1)*5);
		break;
	case CLS_travel_time_bin_10: 
		label.Format ("%d-%d",index*10,(index+1)*10);
		break;
	case CLS_travel_time_bin_30: 
		label.Format ("%d-%d",index*30,(index+1)*30);
		break;
	case CLS_information_class: 
		label = "UE";
		if(index == 1) label = "Historical info";
		if(index == 2) label = "Pretrip info";
		if(index == 3) label = "En-route Info";
		break;

	case CLS_vehicle_type: 
		label = m_VehicleTypeVector[index-1].vehicle_type_name;
		break;
	default: 
		TRACE("ERROR. No classification available!");
	};

	return label;
}

bool CTLiteDoc::SelectVehicleForAnalysis(DTAVehicle* pVehicle, VEHICLE_CLASSIFICATION_SELECTION vehicle_selection)
{
	if(!pVehicle->m_bComplete)
		return false;

	if(vehicle_selection == CLS_network)
		return  true;  // all the vehicles

	if(vehicle_selection == CLS_OD)
		return  pVehicle->m_bODMarked;  // marked by vehicle path dialog


	if(vehicle_selection == CLS_link_set)
	{
		for(int link= 1; link<pVehicle->m_NodeSize; link++)
		{

			if(m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_DisplayLinkID>=0)  // in one of selected links
			{
				return true;		
			}
		}
		return false;
	}

	if(vehicle_selection == CLS_path_trip)
	{
		if(m_PathDisplayList.size() == 0)
			return false;

		if(m_SelectPathNo >= m_PathDisplayList.size())
			return false;

		int count_of_links_in_selected_path = 0;

		for(int link= 1; link<pVehicle->m_NodeSize; link++)
		{
			if(m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end() &&  m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_bIncludedBySelectedPath )
			{
				count_of_links_in_selected_path++;	
			}
		}

		if(count_of_links_in_selected_path == m_PathDisplayList[m_SelectPathNo].m_LinkVector.size() && m_PathDisplayList[m_SelectPathNo].m_LinkVector.size() >0)
			return true;
		else
			return false;
	}

	if(vehicle_selection == CLS_path_partial_trip)
	{
		if(m_PathDisplayList.size() == 0)
			return false;

		if(m_SelectPathNo >= m_PathDisplayList.size())
			return false;

		int count_of_links_in_selected_path = 0;

		for(int link= 1; link<pVehicle->m_NodeSize; link++)
		{
			if(m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end() &&  m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_bIncludedBySelectedPath )
			{
				count_of_links_in_selected_path++;	
			}
		}

		if(count_of_links_in_selected_path == m_PathDisplayList[m_SelectPathNo].m_LinkVector.size() && m_PathDisplayList[m_SelectPathNo].m_LinkVector.size() >0)
		{
			//update subtrip travel time

			float subtrip_distance = 0;
			float subtrip_free_flow_travel_time = 0;
			bool b_distance_counting_flag = false;
			for(int link= 1; link<pVehicle->m_NodeSize; link++)
			{

				if(m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end() &&  m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_bFirstPathLink  )
				{
					pVehicle->m_path_start_node_departure_time = pVehicle->m_NodeAry[link-1].ArrivalTimeOnDSN;
					b_distance_counting_flag = true;
				}

				if(b_distance_counting_flag)  // count from the first link to the last link
				{
					subtrip_distance += m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo] ->m_Length ;
					subtrip_free_flow_travel_time += m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo] ->m_FreeFlowTravelTime  ;
				}


				if(m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end() &&  m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_bLastPathLink  )
				{
					pVehicle->m_path_end_node_arrival_time  = pVehicle->m_NodeAry[link].ArrivalTimeOnDSN;

					pVehicle->m_path_travel_time = pVehicle->m_path_end_node_arrival_time - pVehicle->m_path_start_node_departure_time;

					b_distance_counting_flag = false;

					break;

				}


			}

			pVehicle->m_path_distance  = subtrip_distance;
			pVehicle->m_path_free_flow_travel_time = subtrip_free_flow_travel_time;

			return true;
		}
		else
			return false;
	}

	if(vehicle_selection == CLS_subarea_generated)
	{
		// if the first link of a vehicle is marked, then return true

		if(pVehicle->m_NodeSize>2)
		{

			for(int link= 1; link< pVehicle->m_NodeSize; link++)
			{

				if(m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end())  // first  valide link
				{
					if(m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_bIncludedinSubarea == true)
						return true;
					else
						return false;
				}

			}

		}
		return false;
	}

	if(vehicle_selection == CLS_subarea_traversing_through)
	{

		if(pVehicle->m_NodeSize<3)
			return false;
		// now pVehicle->m_NodeSize>=3;

		// condition 1: if the first link of a vehicle is marked (in the subarea), then return false
		if(m_LinkNoMap.find(pVehicle->m_NodeAry[1].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap.find(pVehicle->m_NodeAry[1].LinkNo) != m_LinkNoMap.end())
		{
			if( m_LinkNoMap[pVehicle->m_NodeAry[1].LinkNo]->m_bIncludedinSubarea == true)
				return false;
		}

		// now the first link is not in subarea
		// condition 2: a link is in subarea
		bool bPassingSubareaFlag = false;
		for(int link= 2; link<pVehicle->m_NodeSize-1; link++)
		{

			if( m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_bIncludedinSubarea )
			{
				bPassingSubareaFlag = true;	
				break;
			}
		}

		if(bPassingSubareaFlag == false)
			return false;
		// now there is a link is in subarea
		// condition 3: test if the last link is out of subarea

		int last_link  = pVehicle->m_NodeSize-1;
		if(m_LinkNoMap.find(pVehicle->m_NodeAry[last_link].LinkNo) != m_LinkNoMap.end() &&  m_LinkNoMap[pVehicle->m_NodeAry[last_link].LinkNo]->m_bIncludedinSubarea == false)
			return true;  // last link is outside of the subarea area, yes!
		else
			return false;

	}

	if(vehicle_selection == CLS_subarea_internal_to_external)
	{
		if(pVehicle->m_NodeSize<2)
			return false;
		// now pVehicle->m_NodeSize>=2;

		// condition 1: if the first link of a vehicle is not marked (not in the subarea), then return false
		if( m_LinkNoMap.find(pVehicle->m_NodeAry[1].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap[pVehicle->m_NodeAry[1].LinkNo]->m_bIncludedinSubarea == false)
			return false;

		// now the first link is in subarea
		// condition 2: test if the last link is out of subarea

		int last_link  = pVehicle->m_NodeSize-1;
		if(m_LinkNoMap.find(pVehicle->m_NodeAry[last_link].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap[pVehicle->m_NodeAry[last_link].LinkNo]->m_bIncludedinSubarea  == false)
			return true;  // outside, yes,
		else
			return false;
	}

	if(vehicle_selection == CLS_subarea_external_to_internal)
	{

		if(pVehicle->m_NodeSize<2)
			return false;
		// now pVehicle->m_NodeSize>=2;

		// condition 1: if the first link of a vehicle is marked (in the subarea), then return false
		if(m_LinkNoMap.find(pVehicle->m_NodeAry[1].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap[pVehicle->m_NodeAry[1].LinkNo]->m_bIncludedinSubarea == true)
			return false;

		// now the first link is not in the subarea
		// condition 2: test if the last link is in subarea

		int last_link  = pVehicle->m_NodeSize-1;
		if( m_LinkNoMap.find(pVehicle->m_NodeAry[last_link].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap[pVehicle->m_NodeAry[last_link].LinkNo]->m_bIncludedinSubarea  == true)
			return true;  // inside, yes!
		else
			return false;
	}
	if(vehicle_selection == CLS_subarea_internal_to_internal_trip)
	{

		if(pVehicle->m_NodeSize<2)  
			return false;
		// now pVehicle->m_NodeSize>=2;

		for(int link= 1; link<pVehicle->m_NodeSize; link++)
		{
			if( m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_bIncludedinSubarea == false )
				return false; // this is a link outside of the subarea
		}

		// pass all the tests
		return true;
	}


	if(vehicle_selection == CLS_subarea_internal_to_internal_subtrip)
	{

		if(pVehicle->m_NodeSize<2)  
			return false;
		// now pVehicle->m_NodeSize>=2;

		float subtrip_distance = 0;
		float subtrip_free_flow_travel_time = 0;
		int distance_counting_flag = 0;
		for(int link= 1; link<pVehicle->m_NodeSize; link++)
		{
			if( m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_bIncludedinSubarea == true )
			{
				if(distance_counting_flag==0)
				{
					distance_counting_flag = 1;
					pVehicle->m_subarea_start_node_departure_time = pVehicle->m_NodeAry [link-1].ArrivalTimeOnDSN ;

				}

				if(distance_counting_flag==1)
				{
					subtrip_distance+= m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_Length ;
					subtrip_free_flow_travel_time += m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_FreeFlowTravelTime ;
				}

				pVehicle->m_subarea_distance = subtrip_distance; // for inside path
				pVehicle->m_subarea_free_flow_travel_time = subtrip_free_flow_travel_time;
				pVehicle->m_subarea_end_node_arrival_time  = pVehicle->m_NodeAry [link].ArrivalTimeOnDSN ;
				pVehicle->m_subarea_travel_time = pVehicle->m_subarea_end_node_arrival_time  - pVehicle->m_subarea_start_node_departure_time;

			}else if(distance_counting_flag == 1) 
			{  //leaving subarea

				distance_counting_flag = 2;

				break;


			}

		}

		// pass all the tests
		if(distance_counting_flag>=1)
			return true;
	}
	if(vehicle_selection == CLS_subarea_boundary_to_bounary_subtrip)
	{

		if(pVehicle->m_NodeSize<2)  
			return false;
		// now pVehicle->m_NodeSize>=2;

		float subtrip_distance = 0;
		float subtrip_free_flow_travel_time = 0;
		int distance_counting_flag = 0;
		for(int link= 1; link<pVehicle->m_NodeSize; link++)
		{
			if( m_LinkNoMap.find(pVehicle->m_NodeAry[link].LinkNo) != m_LinkNoMap.end() && m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_bIncludedinSubarea == true )
			{
				if(distance_counting_flag==0)
				{
					distance_counting_flag = 1;
					pVehicle->m_subarea_start_node_departure_time = pVehicle->m_NodeAry [link-1].ArrivalTimeOnDSN ;

				}

				if(distance_counting_flag==1)
				{
					subtrip_distance+= m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_Length ;
					subtrip_free_flow_travel_time += m_LinkNoMap[pVehicle->m_NodeAry[link].LinkNo]->m_FreeFlowTravelTime ;
				}

				pVehicle->m_subarea_distance = subtrip_distance; // for inside path
				pVehicle->m_subarea_free_flow_travel_time = subtrip_free_flow_travel_time;
				pVehicle->m_subarea_end_node_arrival_time  = pVehicle->m_NodeAry [link].ArrivalTimeOnDSN ;
				pVehicle->m_subarea_travel_time = pVehicle->m_subarea_end_node_arrival_time  - pVehicle->m_subarea_start_node_departure_time;

			}else if(distance_counting_flag == 1) 
			{  //leaving subarea

				distance_counting_flag = 2;

				break;


			}

		}

		// pass all the tests
		if(distance_counting_flag==2)
			return true;
	}
	return false;
}
void CTLiteDoc::MarkLinksInSubarea()
{
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		(*iLink)->m_bIncludedinSubarea = false;  // reset all the links are not selected by the path
	}

	for (iLink = m_SubareaLinkSet.begin(); iLink != m_SubareaLinkSet.end(); iLink++)
	{
		(*iLink)->m_bIncludedinSubarea = true;
	}
}

void CTLiteDoc::GenerateVehicleClassificationData(VEHICLE_CLASSIFICATION_SELECTION vehicle_selection, VEHICLE_X_CLASSIFICATION x_classfication)
{
	m_ClassificationTable.clear ();  //reset

	std::list<DTAVehicle*>::iterator iVehicle;

	//calculate free-flow travel time
	for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++)
	{
		DTAVehicle* pVehicle = (*iVehicle);

		if(pVehicle->m_FreeflowTripTime <0.001f)  // freeflow travel time has not been defined yet
		{

			DTAVehicle* pVehicle = (*iVehicle);

			for(int i=1; i< pVehicle->m_NodeSize ; i++)
			{
				if(m_LinkNoMap.find(pVehicle->m_NodeAry[i].LinkNo)!= m_LinkNoMap.end()) 
				{
				DTALink* pLinkCurrent = m_LinkNoMap[ pVehicle->m_NodeAry[i].LinkNo];
						pVehicle->m_FreeflowTripTime+= pLinkCurrent->m_FreeFlowTravelTime ;
				}
			}
		
		}
	}

	bool bTraceFlag = true;

	for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++)
	{
		DTAVehicle* pVehicle = (*iVehicle);

		pVehicle->m_bMarked = false;
		if(SelectVehicleForAnalysis(pVehicle, m_VehicleSelectionMode) == true)  // belong this classification 
		{
			pVehicle->m_bMarked = true;
			if(bTraceFlag)  // trace single vehicle id
			{
				TRACE("vehicle name: %d\n",pVehicle->m_VehicleID+1  );
				bTraceFlag = false;
			}
			int index = FindClassificationNo(pVehicle,x_classfication);

			if(index>=0)  // -1 will not be consideded
			{
				m_ClassificationTable[index].TotalVehicleSize+=1;


				if(vehicle_selection== CLS_path_partial_trip)  // subpath
				{
					m_ClassificationTable[index].TotalTravelTime  +=  pVehicle->m_path_travel_time;
					m_ClassificationTable[index].TotalFreeflowTravelTime  +=  pVehicle->m_path_free_flow_travel_time;
					
					m_ClassificationTable[index].TotalDistance   += pVehicle->m_path_distance;				
					m_ClassificationTable[index].TotalTravelTimePerMile += ( pVehicle->m_path_travel_time /max(0.01,pVehicle->m_path_distance));

					m_ClassificationTable[index].m_data_vector_travel_time .AddData (pVehicle->m_path_travel_time);
					m_ClassificationTable[index].m_data_vector_travel_time_per_mile .AddData ( pVehicle->m_path_travel_time /max(0.01,pVehicle->m_path_distance));


				}else if (vehicle_selection == CLS_subarea_internal_to_internal_subtrip || vehicle_selection == CLS_subarea_boundary_to_bounary_subtrip)
				{
					m_ClassificationTable[index].TotalTravelTime  += pVehicle->m_subarea_travel_time ;
					m_ClassificationTable[index].TotalFreeflowTravelTime  += pVehicle->m_subarea_travel_time ;
					m_ClassificationTable[index].TotalDistance   += pVehicle->m_subarea_distance;				
					m_ClassificationTable[index].TotalTravelTimePerMile += (pVehicle->m_subarea_travel_time)/max(0.01,pVehicle->m_subarea_distance);

					m_ClassificationTable[index].m_data_vector_travel_time .AddData (pVehicle->m_subarea_travel_time);
					m_ClassificationTable[index].m_data_vector_travel_time_per_mile .AddData (( pVehicle->m_subarea_travel_time) /max(0.01,pVehicle->m_subarea_distance));
				}
				else  //
				{
					m_ClassificationTable[index].TotalTravelTime  += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
					m_ClassificationTable[index].TotalFreeflowTravelTime  += (pVehicle->m_FreeflowTripTime);
					
					m_ClassificationTable[index].TotalDistance   += pVehicle->m_Distance;				
					m_ClassificationTable[index].TotalTravelTimePerMile += ( (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime)/pVehicle->m_Distance);

					m_ClassificationTable[index].m_data_vector_travel_time .AddData (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
					m_ClassificationTable[index].m_data_vector_travel_time_per_mile .AddData (
						( pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime) /max(0.01,pVehicle->m_Distance));
				}

				m_ClassificationTable[index].TotalCost   += pVehicle->m_TollDollarCost;
				m_ClassificationTable[index].TotalGeneralizedCost += ( pVehicle->m_TollDollarCost + pVehicle->m_TripTime /60.0f* pVehicle->m_VOT );
				m_ClassificationTable[index].TotalGeneralizedTime += pVehicle->m_TollDollarCost/max(1,pVehicle->m_VOT)*60.0f + pVehicle->m_TripTime ;
				m_ClassificationTable[index].TotalEmissions   += pVehicle->m_Emissions;

				m_ClassificationTable[index].emissiondata.Energy += pVehicle->m_EmissionData .Energy;
				m_ClassificationTable[index].emissiondata.CO2 += pVehicle->m_EmissionData .CO2;
				m_ClassificationTable[index].emissiondata.NOX += pVehicle->m_EmissionData .NOX;
				m_ClassificationTable[index].emissiondata.CO += pVehicle->m_EmissionData .CO;
				m_ClassificationTable[index].emissiondata.HC += pVehicle->m_EmissionData .HC;


			}

		}

	}

	//variability
	for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++)
	{
		DTAVehicle* pVehicle = (*iVehicle);


		if(SelectVehicleForAnalysis(pVehicle, m_VehicleSelectionMode) == true) 
		{
			int index = FindClassificationNo(pVehicle,x_classfication);

			if(index>=0)  // -1 will not be consideded
			{
				float deviation = 0;
				float travel_time_per_mile_deviation = 0;


				if(vehicle_selection== CLS_path_partial_trip)  //  subpath
				{
					deviation = (pVehicle->m_path_travel_time)- 
						(m_ClassificationTable[index].TotalTravelTime/max(1,m_ClassificationTable[index].TotalVehicleSize));


					travel_time_per_mile_deviation = 
						(pVehicle->m_path_travel_time )/max(0.001,pVehicle->m_path_distance  ) -  
						(m_ClassificationTable[index].TotalTravelTimePerMile/max(1,m_ClassificationTable[index].TotalVehicleSize));
				}else if(vehicle_selection== CLS_subarea_internal_to_internal_subtrip || vehicle_selection == CLS_subarea_boundary_to_bounary_subtrip)  // subarea partial path
				{
					deviation = (pVehicle->m_subarea_travel_time )- 
						(m_ClassificationTable[index].TotalTravelTime/max(1,m_ClassificationTable[index].TotalVehicleSize));


					travel_time_per_mile_deviation = 
						(pVehicle->m_subarea_travel_time )/max(0.001,pVehicle->m_subarea_distance  ) -  
						(m_ClassificationTable[index].TotalTravelTimePerMile/max(1,m_ClassificationTable[index].TotalVehicleSize));

				}else
				{
					deviation = (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime)- 
						(m_ClassificationTable[index].TotalTravelTime/max(1,m_ClassificationTable[index].TotalVehicleSize));

					travel_time_per_mile_deviation = 
						(pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime)/max(0.001,pVehicle->m_Distance ) -  
						(m_ClassificationTable[index].TotalTravelTimePerMile/max(1,m_ClassificationTable[index].TotalVehicleSize));
				}

				m_ClassificationTable[index].TotalTravelTimeVariance   +=  deviation*deviation;
				m_ClassificationTable[index].TotalTravelTimePerMileVariance   +=  travel_time_per_mile_deviation*travel_time_per_mile_deviation;

			}

		}

	}
}


void CTLiteDoc::GenerateClassificationForDisplay(VEHICLE_X_CLASSIFICATION x_classfication, VEHICLE_Y_CLASSIFICATION y_classfication)
{
	// input: 	m_ClassificationTable
	// output: label and display value

	std::map< int, VehicleStatistics >::iterator iter;

	for ( iter = m_ClassificationTable.begin(); iter != m_ClassificationTable.end(); iter++ )
	{
		int index = iter->first;
		float value= 0;

		m_ClassificationTable[index].Label = FindClassificationLabel(x_classfication,index);

		// enum VEHICLE_Y_CLASSIFICATION {CLS_vehicle_count,CLS_total_travel_time,CLS_avg_travel_time,CLS_total_toll_cost,CLS_avg_toll_cost,CLS_total_generalized_cost,CLS_avg_generalized_cost,CLS_total_travel_distance, CLS_avg_travel_distance,CLS_total_CO2,CLS_avg_CO2};


		switch(y_classfication)
		{
		case CLS_vehicle_count: 
			value = m_ClassificationTable[index].TotalVehicleSize;
			break;
		case CLS_cumulative_vehicle_count: 

			value  = 0;
			for(int ii = 0; ii <= index; ii++)
				if(m_ClassificationTable.find(ii) != m_ClassificationTable.end())
				{
					value += m_ClassificationTable[ii].TotalVehicleSize;
				}
				break;

		case CLS_total_travel_time: 
			value = m_ClassificationTable[index].TotalTravelTime ;
			break;
		case CLS_avg_travel_time: 
			value = m_ClassificationTable[index].TotalTravelTime/max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;

		case CLS_travel_time_95_percentile: 
			value = m_ClassificationTable[index].m_data_vector_travel_time .GetDataVectorPercentage (95);
			break;

		case CLS_travel_time_90_percentile: 
			value = m_ClassificationTable[index].m_data_vector_travel_time .GetDataVectorPercentage (90);
			break;

		case CLS_travel_time_80_percentile: 
			value = m_ClassificationTable[index].m_data_vector_travel_time .GetDataVectorPercentage (80);
			break;

		case CLS_travel_time_per_mile_95_percentile: 
			value = m_ClassificationTable[index].m_data_vector_travel_time_per_mile .GetDataVectorPercentage (95);
			break;

		case CLS_travel_time_per_mile_90_percentile: 
			value = m_ClassificationTable[index].m_data_vector_travel_time_per_mile .GetDataVectorPercentage (90);
			break;

		case CLS_travel_time_per_mile_80_percentile: 
			value = m_ClassificationTable[index].m_data_vector_travel_time_per_mile .GetDataVectorPercentage (80);
			break;
			//Buffer Index = (95th percentile travel time – mean travel time) / mean travel time
		case CLS_travel_time_Buffer_Index: 
			value = (m_ClassificationTable[index].m_data_vector_travel_time .GetDataVectorPercentage (95)- m_ClassificationTable[index].m_data_vector_travel_time.GetDataVectorMean ())/max(0.01,m_ClassificationTable[index].m_data_vector_travel_time.GetDataVectorMean ());
			break;
			//Skew Index = (90th percentile travel time – median travel time) / (median travel time – 10th percentile travel time)
		case CLS_travel_time_Skew_Index: 
			value = (m_ClassificationTable[index].m_data_vector_travel_time .GetDataVectorPercentage (90)- m_ClassificationTable[index].m_data_vector_travel_time.GetDataVectorPercentage(50))/
				max(0.01,m_ClassificationTable[index].m_data_vector_travel_time.GetDataVectorPercentage (50) -m_ClassificationTable[index].m_data_vector_travel_time.GetDataVectorPercentage (10) );
			break;
		case CLS_travel_time_Travel_Time_Index:  // mean of travel time distribution/free flow travel time for a given path
			value = m_ClassificationTable[index].TotalTravelTime/max(0.01,m_ClassificationTable[index].TotalFreeflowTravelTime);
			break;
		case CLS_travel_time_Planning_Time_Index: //95th percentile of travel time distribution/free flow travel time for a given path
			value = m_ClassificationTable[index].m_data_vector_travel_time .GetDataVectorPercentage (95)/max(0.01,m_ClassificationTable[index].TotalFreeflowTravelTime/m_ClassificationTable[index].TotalVehicleSize);
			break;
		case CLS_total_toll_cost: 
			value = m_ClassificationTable[index].TotalCost ;
			break;
		case CLS_avg_toll_cost: 
			value = m_ClassificationTable[index].TotalCost /max(1,m_ClassificationTable[index].TotalVehicleSize); // *100, dollar
			break;

		case CLS_total_generalized_cost: 
			value = m_ClassificationTable[index].TotalGeneralizedCost  ;
			break;
		case CLS_total_generalized_travel_time: 
			value = m_ClassificationTable[index].TotalGeneralizedTime  ;
			break;
		case CLS_avg_generalized_cost: 
			value = m_ClassificationTable[index].TotalGeneralizedCost /max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;
		case CLS_avg_generalized_travel_time:
			value = m_ClassificationTable[index].TotalGeneralizedTime /max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;

		case CLS_total_travel_distance: 
			value = m_ClassificationTable[index].TotalDistance   ;
			break;
		case CLS_avg_travel_distance: 
			value = m_ClassificationTable[index].TotalDistance /max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;
		case CLS_avg_speed: 
			value = m_ClassificationTable[index].TotalDistance /max(0.1,m_ClassificationTable[index].TotalTravelTime/60.0f);
			break;
		case CLS_travel_time_STD: 
			value = sqrt(m_ClassificationTable[index].TotalTravelTimeVariance /max(1,m_ClassificationTable[index].TotalVehicleSize));
			break;
		case CLS_travel_time_per_mile_STD: 
			value = sqrt(m_ClassificationTable[index].TotalTravelTimePerMileVariance /max(1,m_ClassificationTable[index].TotalVehicleSize));
			break;
		case CLS_avg_travel_time_per_mile: 
			value = m_ClassificationTable[index].TotalTravelTimePerMile /max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;
		case CLS_total_Energy: 
			value = m_ClassificationTable[index].emissiondata .Energy     ;
			break;
		case CLS_avg_Energy: 
			value = m_ClassificationTable[index].emissiondata .Energy   /max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;
		case CLS_avg_Energy_per_mile: 
			value = m_ClassificationTable[index].emissiondata .Energy   /max(1,m_ClassificationTable[index].TotalDistance);
			break;

		case CLS_total_CO2: 
			value = m_ClassificationTable[index].emissiondata .CO2    ;
			break;
		case CLS_avg_CO2: 
			value = m_ClassificationTable[index].emissiondata .CO2  /max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;
		case CLS_avg_CO2_per_mile: 
			value = m_ClassificationTable[index].emissiondata .CO2  /max(1,m_ClassificationTable[index].TotalDistance);
			break;
		case CLS_total_NOx: 
			value = m_ClassificationTable[index].emissiondata .NOX     ;
			break;
		case CLS_avg_Nox: 
			value = m_ClassificationTable[index].emissiondata .NOX  /max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;
		case CLS_avg_Nox_per_mile: 
			value = m_ClassificationTable[index].emissiondata .NOX  /max(1,m_ClassificationTable[index].TotalDistance);
			break;
		case CLS_total_CO: 
			value = m_ClassificationTable[index].emissiondata .CO     ;
			break;
		case CLS_avg_CO: 
			value = m_ClassificationTable[index].emissiondata .CO  /max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;
		case CLS_avg_CO_per_mile: 
			value = m_ClassificationTable[index].emissiondata .CO 	/max(1,m_ClassificationTable[index].TotalDistance);
			break;
		case CLS_total_HC: 
			value = m_ClassificationTable[index].emissiondata .HC     ;
			break;
		case CLS_avg_HC: 
			value = m_ClassificationTable[index].emissiondata .HC  /max(1,m_ClassificationTable[index].TotalVehicleSize);
			break;
		case CLS_avg_HC_per_mile: 
			value = m_ClassificationTable[index].emissiondata .HC  /max(1,m_ClassificationTable[index].TotalDistance);
			break;
		case CLS_total_gallon: 
			value = m_ClassificationTable[index].emissiondata .Energy/1000/(121.7)     ;
			break;
		case CLS_avg_mile_per_gallon: 
			value = m_ClassificationTable[index].TotalDistance  /max(1,m_ClassificationTable[index].emissiondata .Energy/1000/(121.7));
			break;


		default: 
			value = 0;
			TRACE("ERROR. No classification available!");
		};

		m_ClassificationTable[index].DisplayValue = value;

	}
}


void CTLiteDoc::OnLinkVehiclestatisticsanalaysis()
{
	if(m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	m_VehicleSelectionMode = CLS_link_set;  // select link analysis

	CDlg_VehicleClassification dlg;

	dlg.m_VehicleSelectionNo = CLS_link_set;
	dlg.m_pDoc = this;
	dlg.DoModal ();

}



void CTLiteDoc::OnSubareaDeletesubarea()
{
	m_SubareaShapePoints.clear ();
	UpdateAllViews(0);
}

void CTLiteDoc::OnSubareaViewvehiclestatisticsassociatedwithsubarea()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::OnToolsTraveltimereliabilityanalysis()
{

	//std::vector<float> LinkCapacity;
	//std::vector<float> LinkTravelTime;

	//LinkCapacity.push_back(1800.0f);
	//LinkTravelTime.push_back(20.0f);

	//CDlg_TravelTimeReliability dlg;
	//dlg.m_pDoc= this;
	//dlg.LinkCapacity = LinkCapacity;
	//dlg.LinkTravelTime = LinkTravelTime;

	//dlg.m_BottleneckIdx = 0;
	//dlg.m_PathFreeFlowTravelTime = 20.0f;  // for a predefined corridor
	//dlg.DoModal ();

	m_LinkMOEMode = MOE_reliability;
	m_LinkBandWidthMode = LBW_link_volume;
	ShowLegend(true);
	GenerateOffsetLinkBand();
	UpdateAllViews(0);

}


void CTLiteDoc::OnLinkLinkbar()
{

	CDlg_LinkVisualizationConfig dlg;
	dlg.m_pDoc= this;
	dlg.m_link_band_width_mode = m_LinkBandWidthMode;
	if(dlg.DoModal() == IDOK)
	{
		m_LinkBandWidthMode = dlg.m_link_band_width_mode;
		GenerateOffsetLinkBand();
		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnImportArcgisshapefile()
{
#ifndef _WIN64

#else 
	AfxMessageBox("NEXTA 64-bit version does not support shape file importing function. Please use NEXTA_32.exe ");
	return;
#endif

	CDlg_GISDataExchange dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();
	//

	//m_UnitFeet = 1;  // default value
	//m_NodeDisplaySize = 50;
	CalculateDrawingRectangle();
	m_bFitNetworkInitialized  = false;
	UpdateAllViews(0);


	/*	CString str;
	CFileDialog dlg (FALSE, "*.shp", "*.shp",OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_LONGNAMES,
	"Shape File (*.shp)|*.shp||", NULL);
	if(dlg.DoModal() == IDOK)
	{

	CWaitCursor wait;
	ImportOGRShapeFile(dlg.GetPathName());
	}
	*/
}



void CTLiteDoc::OnLinkIncreaseoffsetfortwo()
{
	m_OffsetInFeet += 2;// 5 feet
	m_bLinkToBeShifted  = true;
	OffsetLink();  // offset shape points
	GenerateOffsetLinkBand();  // from shape points to reference points
	UpdateAllViews(0);
}

void CTLiteDoc::OnLinkDecreaseoffsetfortwo()
{
	m_OffsetInFeet -= 2; //5feet

	//if(m_OffsetInFeet<0)
	//	m_OffsetInFeet = 0;


	//	m_OffsetInFeet = max (5,m_OffsetInFeet);  // minimum 5 feet
	m_bLinkToBeShifted  = true;
	OffsetLink();
	GenerateOffsetLinkBand();
	UpdateAllViews(0);
}

void CTLiteDoc::OnLinkNooffsetandnobandwidth()
{
	std::list<DTALink*>::iterator iLink;

	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		(*iLink)->m_ShapePoints.clear();

		(*iLink)->m_ShapePoints .push_back ((*iLink)->m_FromPoint);
		(*iLink)->m_ShapePoints .push_back ((*iLink)->m_ToPoint);
	}

	m_OffsetInFeet=10;
	m_bLinkToBeShifted  = true;
	m_LaneWidthInFeet = 2;
	OffsetLink();
	GenerateOffsetLinkBand();
	UpdateAllViews(0);
}

void CTLiteDoc::OnViewShowhideLegend()
{
	m_bShowLegend = !m_bShowLegend;

	ShowLegend(m_bShowLegend);
}

void CTLiteDoc::ShowLegend(bool ShowLegendStatus)
{
	m_bShowLegend = ShowLegendStatus;
	if(m_bShowLegend)
	{
		if(g_pLegendDlg==NULL)
		{
			g_pLegendDlg = new CDlg_Legend();
			g_pLegendDlg->m_pDoc = this;
			g_pLegendDlg->Create(IDD_DIALOG_Legend);
		}

		// update using pointer to the active document; 

		if(g_pLegendDlg->GetSafeHwnd())
		{
			g_pLegendDlg->m_pDoc = this;
			g_pLegendDlg->ShowWindow(SW_HIDE);
			g_pLegendDlg->ShowWindow(SW_SHOW);
		}
	}else
	{
		if(g_pLegendDlg!=NULL && g_pLegendDlg->GetSafeHwnd())
		{
			g_pLegendDlg->ShowWindow(SW_HIDE);
		}
	}
}
void CTLiteDoc::OnMoeViewlinkmoesummaryfile()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_LinkMOE.csv");
}

void CTLiteDoc::OnViewCalibrationview()
{
	m_bShowCalibrationResults = !m_bShowCalibrationResults;

	if(m_bShowCalibrationResults == true)
	{
		m_LinkMOEMode = MOE_volume;
		OnMoeViewoddemandestimationsummaryplot();
		UpdateAllViews(0);
	}
}

void CTLiteDoc::OnUpdateViewCalibrationview(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowCalibrationResults);
}

void CTLiteDoc::OnMoeViewtrafficassignmentsummaryplot()
{
	NetworkLoadingOutput element;
	CCSVParser parser;

	CString str = m_ProjectDirectory +"output_assignment_log.csv";
	CT2CA pszConvertedAnsiString (str);

	// construct a std::string using the LPCSTR input
	std::string  strStd (pszConvertedAnsiString);

	if (parser.OpenCSVFile(strStd))
	{
		COLORREF crColor1 = RGB (255, 0, 0);
		COLORREF crColor2 = RGB (0, 0, 255);
		COLORREF crColor3 = RGB (0, 128, 0);

		CLinePlotData element_avg_travel_time_in_min;
		element_avg_travel_time_in_min.crPlot = crColor1;
		element_avg_travel_time_in_min.szName  = "Avg Travel Time (min)";

		CLinePlotData element_avg_travel_distance_in_mile;
		element_avg_travel_distance_in_mile.crPlot = crColor2;
		element_avg_travel_distance_in_mile.szName  = "Avg Distance (mile)";

		CLinePlotData element_avg_travel_time_gap_per_vehicle_in_min;
		element_avg_travel_distance_in_mile.crPlot = crColor3;
		element_avg_travel_time_gap_per_vehicle_in_min.szName  = "Avg TT Gap (min)";

		int iteration =0;
		while(parser.ReadRecord())
		{
			FLOATPOINT data;
			data.x = iteration;

			if(parser.GetValueByFieldName("avg_travel_time_in_min",data.y) == false)
				break;
			element_avg_travel_time_in_min.vecData.push_back(data);

			if(parser.GetValueByFieldName("avg_travel_distance_in_mile",data.y) == false)
				break;
			element_avg_travel_distance_in_mile.vecData.push_back(data);

			if(iteration!=0)
			{
				if(parser.GetValueByFieldName("avg_travel_time_gap_per_vehicle_in_min",data.y) == false)
					break;
				element_avg_travel_time_gap_per_vehicle_in_min.vecData.push_back(data);
			}

			iteration++;

		}

		if(iteration > 1)
		{

			CLinePlotTestDlg dlg;
			dlg.m_pDoc  = this;

			dlg.m_XCaption = "Iteration";
			dlg.m_YCaption = "Traffic Assignment MOE";
			dlg.m_PlotDataVector.push_back(element_avg_travel_time_in_min);
			dlg.m_PlotDataVector.push_back(element_avg_travel_distance_in_mile);
			dlg.m_PlotDataVector.push_back(element_avg_travel_time_gap_per_vehicle_in_min);

			dlg.DoModal();
		}else
		{
			AfxMessageBox("File output_assignment_log.csv does not have assignment results. Please first run traffic assignment.");

		}
	}else
	{
		AfxMessageBox("File output_assignment_log.csv does not exist. Please first run traffic assignment.");
	}
}

void CTLiteDoc::OnMoeViewoddemandestimationsummaryplot()
{
	CLinePlotTestDlg dlg;
	dlg.m_pDoc  = this;

	dlg.m_XCaption = "Observed Link Volume";
	dlg.m_YCaption = "Simulated Link Volume";

	CString str = m_ProjectDirectory +"output_validation_results.csv";
	CT2CA pszConvertedAnsiString (str);
	// construct a std::string using the LPCSTR input
	std::string  strStd (pszConvertedAnsiString);
	CCSVParser parser;

	if (parser.OpenCSVFile(strStd))
	{
		COLORREF crColor1 = RGB (100, 149, 237);

		CLinePlotData element_link_volume;
		element_link_volume.crPlot = crColor1;
		element_link_volume.szName  = "link volume";
		element_link_volume.lineType = enum_LpScatter;

		int count =0;
		while(parser.ReadRecord())
		{
			FLOATPOINT data;

			if(parser.GetValueByFieldName("observed_link_count",data.x) == false)
				break;

			int from_node_id, to_node_id;

			parser.GetValueByFieldName("from_node_id",from_node_id);
			parser.GetValueByFieldName("to_node_id",to_node_id);

			DTALink* pLink  = FindLinkWithNodeNumbers(from_node_id , to_node_id, str );


			if(pLink!=NULL)
			{
				data.LinkNo = pLink->m_LinkNo ;
				if( m_LinkTypeMap[pLink->m_link_type].IsFreeway ())
				{   
					data.LinkType = _lp_freeway;
					data.crColor  = m_FreewayColor;

				}

				if( m_LinkTypeMap[pLink->m_link_type].IsArterial  ())
				{
					data.LinkType = _lp_arterial;
					data.crColor = m_ArterialColor;
				}

				if( m_LinkTypeMap[pLink->m_link_type].IsHighway   ())
				{
					data.LinkType = _lp_ramp;
					data.crColor = m_ArterialColor;
				}

				data.Hour  = -1;
			}


			if(data.x >0.1f)   // with sensor data
			{

				if(parser.GetValueByFieldName("simulated_link_count",data.y) == false)  // estimated data
					break;
				element_link_volume.vecData.push_back(data);

				count++;

			}
		}

		if(count >=1)
		{


			//float r2 = 0.91f;
			//msg.Format ("R2 = %.3f",r2);
			//dlg.m_MessageVector.push_back(msg);

			dlg.m_PlotDataVector.push_back(element_link_volume);

		}else
		{
			AfxMessageBox("No sensor data are available. Please first input link volume data in file input_sensor.csv.");
		}
	}else
	{
		AfxMessageBox("File output_LinkMOE.csv does not exist. Please first run traffic assignment.");
	}

	// aggregated result
	OnSensortoolsConverttoHourlyVolume();
	{


		CCSVParser SettingFile;

		if(SettingFile.OpenCSVFile (CString2StdString(m_ProjectDirectory+"input_scenario_settings.csv")))
		{
			while(SettingFile.ReadRecord())
			{
				SettingFile.GetValueByFieldName("calibration_data_start_time_in_min",m_calibration_data_start_time_in_min);
				SettingFile.GetValueByFieldName("calibration_data_end_time_in_min",m_calibration_data_end_time_in_min);

			}

		}



		CCSVParser parser;

		if (parser.OpenCSVFile( CString2StdString(m_ProjectDirectory +"output_validation_results_aggregated.csv")))
		{
			COLORREF crColor1 = RGB (100, 149, 237);

			CLinePlotData element_link_volume;
			element_link_volume.crPlot = crColor1;
			element_link_volume.szName  = "aggregated hourly volume";
			element_link_volume.lineType = enum_LpScatter;

			int count =0;
			while(parser.ReadRecord())
			{
				FLOATPOINT data;

				if(parser.GetValueByFieldName("observed_link_count",data.x) == false)
					break;

				int from_node_id, to_node_id;

				parser.GetValueByFieldName("from_node_id",from_node_id);
				parser.GetValueByFieldName("to_node_id",to_node_id);

				DTALink* pLink  = FindLinkWithNodeNumbers(from_node_id , to_node_id, str );

				if(pLink!=NULL)
				{
					data.LinkNo = pLink->m_LinkNo ;
					if( m_LinkTypeMap[pLink->m_link_type].IsFreeway ())
					{   
						data.LinkType = _lp_freeway;
						data.crColor  = m_FreewayColor;

					}

					if( m_LinkTypeMap[pLink->m_link_type].IsArterial  ())
					{
						data.LinkType = _lp_arterial;
						data.crColor = m_ArterialColor;
					}

					if( m_LinkTypeMap[pLink->m_link_type].IsHighway   ())
					{
						data.LinkType = _lp_ramp;
						data.crColor = m_ArterialColor;
					}

					int start_time_in_min;
					parser.GetValueByFieldName("start_time_in_min",start_time_in_min);
					data.Hour = start_time_in_min/60;

				}


				if(data.x >0.1f)   // with sensor data
				{

					if(parser.GetValueByFieldName("simulated_link_count",data.y) == false)  // estimated data
						break;
					element_link_volume.vecData.push_back(data);

					count++;

				}
			}

			if(count >=1)
			{


				//float r2 = 0.91f;
				//msg.Format ("R2 = %.3f",r2);
				//dlg.m_MessageVector.push_back(msg);

				dlg.m_PlotDataVector.push_back(element_link_volume);

			}
		}
	}

	dlg.DoModal();

}

void CTLiteDoc::OnProjectEditpricingscenariodata()
{
	CDlg_PricingConfiguration dlg;
	dlg.m_pDoc = this;
	if(dlg.DoModal() ==IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
}

void CTLiteDoc::OnLinkViewlink()
{
	CDlgScenario dlg(0);
	dlg.m_pDoc = this;

	//dlg.m_SelectTab = 0;
	dlg.DoModal();
}

void CTLiteDoc::OnDeleteSelectedLink()
{
	if(m_SelectedLinkNo == -1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}
	DeleteLink(m_SelectedLinkNo);
	m_SelectedLinkNo = -1;
	UpdateAllViews(0);

}


void CTLiteDoc::OnImportRegionalplanninganddtamodels()
{
	CDlg_ImportPlanningModel dlg;
	dlg.DoModal ();
}

void CTLiteDoc::OnExportGenerateTravelTimeMatrix()
{
	CWaitCursor wait;

	std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
	int project_index = 0 ;
	while (iDoc != g_DocumentList.end())
	{
		if((*iDoc)->m_ProjectTitle.GetLength () >0)  
		{

			std::list<DTAVehicle*>::iterator iVehicle;

			for (iVehicle = (*iDoc)->m_VehicleSet.begin(); iVehicle != (*iDoc)->m_VehicleSet.end(); iVehicle++)
			{
				DTAVehicle* pVehicle = (*iVehicle);

				if(pVehicle->m_NodeSize >= 2 && pVehicle->m_bComplete)  // with physical path in the network
				{	

					CString label;
					// reuse label as OD label
					label.Format("%d,%d", pVehicle->m_OriginZoneID  ,pVehicle->m_DestinationZoneID);
					m_ODMatrixMap[label].Origin = pVehicle->m_OriginZoneID;
					m_ODMatrixMap[label].Destination  = pVehicle->m_DestinationZoneID;
					m_ODMatrixMap[label].TotalVehicleSize+=1;
					m_ODMatrixMap[label].TotalVehicleSizeVector[project_index]+=1;
					m_ODMatrixMap[label].TotalTravelTimeVector[project_index]  += pVehicle->m_TripTime ;
				}
			}
			project_index++;
		}
		iDoc++;
	}




	CString directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);
	FILE* st;
	fopen_s(&st,directory+"output_travel_time_matrix.csv","w");
	if(st!=NULL)
	{
		fprintf(st,"from_zone_id,from_zone_id,");

		iDoc = g_DocumentList.begin ();
		project_index = 0;
		while (iDoc != g_DocumentList.end())
		{

			if((*iDoc)->m_ProjectTitle.GetLength () >0)
			{
				fprintf(st,"%d,%s,%s,,,,",  project_index+1, (*iDoc)->m_ProjectDirectory, (*iDoc)->m_ProjectTitle);
				project_index++;
			}
			iDoc++;
		}

		fprintf(st,"\n");

		fprintf(st,"from_zone_id,from_zone_id,");

		iDoc = g_DocumentList.begin ();
		project_index = 0;

		while (iDoc != g_DocumentList.end())
		{
			if((*iDoc)->m_ProjectTitle.GetLength () >0)
			{

				if(project_index == 0)
				{
					fprintf(st,"number_of_vehicles,avg_travel_time (min),total_travel_time (min),");
				}else
				{ // 		project_index >=1
					fprintf(st,"number_of_vehicles,avg_travel_time (min),total_travel_time (min),difference_in_number_of_vehicles,difference_of_avg_travel_time,diff_total_travel_time,");		
				}

				project_index ++;
			}

			iDoc++;
		}

		fprintf(st,"\n");


		std::map<int, DTAZone>	:: const_iterator itrFrom;
		std::map<int, DTAZone>	:: const_iterator itrTo;

		for(itrFrom = m_ZoneMap.begin(); itrFrom != m_ZoneMap.end(); itrFrom++)
		{
			for(itrTo = m_ZoneMap.begin(); itrTo != m_ZoneMap.end(); itrTo++)
			{
				int  centroid_pair_size  = 0;
				float total_travel_time  = 0;

				//for(int i = 0; i< itrFrom->second.m_ActivityLocationVector .size(); i++)
				//{
				//	for(int j = 0; j< itrTo->second.m_ActivityLocationVector .size(); j++)
				//	{
				//		int FromNodeNo = m_NodeNumbertoIDMap[itrFrom->second.m_ActivityLocationVector[i].NodeNumber];
				//		int ToNodeNo =  m_NodeNumbertoIDMap[itrTo->second.m_ActivityLocationVector[j].NodeNumber];

				//		float TotalCost = 0;
				//		m_Network.SimplifiedTDLabelCorrecting_DoubleQueue(FromNodeNo, 0, ToNodeNo, 1, 10.0f,PathLinkList,TotalCost, false, false, false,0);   // Pointer to previous node (node)

				//		if(TotalCost<999)  // feasible cost
				//		{
				//		total_travel_time += TotalCost;
				//		centroid_pair_size++;
				//		}

				//	} // per centroid in destination zone
				//} // per centroid in origin zone

				CString label;
				// reuse label as OD label
				label.Format("%d,%d", itrFrom->first  ,  itrTo->first);

				if(m_ODMatrixMap.find(label) != m_ODMatrixMap.end())
				{

					float avg_travel_time = max(0.5,total_travel_time / max(1,centroid_pair_size));

					fprintf(st, "%d,%d,", itrFrom->first , itrTo->first);

					project_index = 0;
					iDoc = g_DocumentList.begin ();	

					while (iDoc != g_DocumentList.end())
					{
						if((*iDoc)->m_ProjectTitle.GetLength () >0)  
						{
							fprintf(st, "%d,%5.2f,%5.2f,", 
								m_ODMatrixMap[label].TotalVehicleSizeVector[project_index] , 

								m_ODMatrixMap[label].TotalTravelTimeVector[project_index]/max(1,m_ODMatrixMap[label].TotalVehicleSizeVector[project_index]),

								m_ODMatrixMap[label].TotalTravelTimeVector[project_index]);

							if(project_index >=1)
							{
								fprintf(st, "%d,%5.2f,%5.2f,", 
									m_ODMatrixMap[label].TotalVehicleSizeVector[project_index] - m_ODMatrixMap[label].TotalVehicleSizeVector[0] , 

									m_ODMatrixMap[label].TotalTravelTimeVector[project_index]/max(1,m_ODMatrixMap[label].TotalVehicleSizeVector[project_index]) - m_ODMatrixMap[label].TotalTravelTimeVector[0]/max(1,m_ODMatrixMap[label].TotalVehicleSizeVector[0]),

									m_ODMatrixMap[label].TotalTravelTimeVector[project_index] - m_ODMatrixMap[label].TotalVehicleSizeVector[0] 
								);
							}

							project_index++;
						}
						iDoc++;

						if(project_index  >= _TOTAL_NUMBER_OF_PROJECTS )
							break;
					}

					fprintf(st,"\n");


				}


			} //per origin zone

		}//per destination zone

		fclose(st);
	}else
	{
		CString msg;
		msg.Format("File %s cannot be opened.\nIt might be currently used and locked by EXCEL.",directory+"output_travel_time_matrix.csv");
		AfxMessageBox(msg);
		return;
	}

	OpenCSVFileInExcel(directory+"output_travel_time_matrix.csv");
}

void CTLiteDoc::OnExportGenerateshapefiles()
{
	if(m_ProjectDirectory.GetLength()==0)
	{
		AfxMessageBox("The project directory has not been specified. Please save the project to a new folder first.");
		OnFileSaveProjectAs();
		return;
	}

	CWaitCursor wc;

	CString directory;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

	//DeleteFile(directory+"AMS_node.shp");
	//DeleteFile(directory+"AMS_node.dbf");
	//DeleteFile(directory+"AMS_node.shx");
	//ExportNodeLayerToGISFiles(directory+"AMS_node.shp","ESRI Shapefile");

	//DeleteFile(directory+"AMS_link.shp");
	//DeleteFile(directory+"AMS_link.dbf");
	//DeleteFile(directory+"AMS_link.shx");
	//ExportLinkLayerToGISFiles(directory+"AMS_link.shp","ESRI Shapefile");

	//DeleteFile(directory+"AMS_node.kmz");
	//ExportNodeLayerToGISFiles(directory+"AMS_node.kml","KML");

	m_bExport_Link_MOE_in_input_link_CSF_File = false;
	OnFileSaveProject();  // save time-dependent MOE to input_link MOE file

	DeleteFile(directory+"AMS_link.shp");
	ExportLinkLayerToGISFiles(directory+"AMS_link.shp","ESRI Shapefile");

	//	OGDF_WriteGraph(directory+"graph.gml");

	DeleteFile(directory+"AMS_zone.kmz");
	ExportZoneLayerToKMLFiles(directory+"AMS_zone.kml","LIBKML");

	DeleteFile(directory+"AMS_link_3D.kml");
	ExportLink3DLayerToKMLFiles(directory+"AMS_link_3D.kml","LIBKML");

	ExportLink3DLayerToKMLFiles_ColorCode(directory+"AMS_link_green_2D.kml","LIBKML",0,false,1);
	ExportLink3DLayerToKMLFiles_ColorCode(directory+"AMS_link_green_3D.kml","LIBKML",0,false,-1);
	ExportLink3DLayerToKMLFiles_ColorCode(directory+"AMS_link_red_3D.kml","LIBKML",1,false,-1);
	ExportLink3DLayerToKMLFiles_ColorCode(directory+"AMS_link_blue_3D.kml","LIBKML",2,false,-1);
	ExportLink3DLayerToKMLFiles_ColorCode(directory+"AMS_link_yellow_3D.kml","LIBKML",3,false,-1);

	ExportLink3DLayerToKMLFiles_ColorCode(directory+"AMS_link_green_3D_SL.kml","LIBKML",0,true,-1);
	ExportLink3DLayerToKMLFiles_ColorCode(directory+"AMS_link_red_3D_SL.kml","LIBKML",1,true,-1);
	ExportLink3DLayerToKMLFiles_ColorCode(directory+"AMS_link_blue_3D_SL.kml","LIBKML",2,true,-1);
	ExportLink3DLayerToKMLFiles_ColorCode(directory+"AMS_link_yellow_3D_SL.kml","LIBKML",3,true,-1);

	//DeleteFile(directory+"AMS_agent.kmz");
	//ExportAgentLayerToKMLFiles(directory+"AMS_agent.kml","KML");

	OnToolsProjectfolder();

}

void CTLiteDoc::OnLinkmoedisplayQueuelength()
{
	m_LinkMOEMode = MOE_queue_length;
	m_LinkBandWidthMode = LBW_number_of_lanes;

	ShowLegend(false);
	GenerateOffsetLinkBand();
	UpdateAllViews(0);

}

void CTLiteDoc::OnUpdateLinkmoedisplayQueuelength(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_queue_length);
}

void CTLiteDoc::OnUpdateLinkmoeTraveltimereliability(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_reliability);
}



void CTLiteDoc::ZoomToSelectedNode(int SelectedNodeNumber)
{
	if(m_NodeNumbertoIDMap.find(SelectedNodeNumber) != m_NodeNumbertoIDMap.end())
	{
		DTANode* pNode= m_NodeIDMap[m_NodeNumbertoIDMap[SelectedNodeNumber]];
		m_Origin = pNode->pt;
		m_SelectedLinkNo = -1;
		m_SelectedNodeID = pNode->m_NodeID ;

		CTLiteView* pView = 0;
		POSITION pos = GetFirstViewPosition();
		if(pos != NULL)
		{
			pView = (CTLiteView*) GetNextView(pos);
			if(pView!=NULL)
			{
				pView->m_Origin = m_Origin;
				pView->Invalidate ();
			}
		}

	}
}

void CTLiteDoc::ZoomToSelectedLink(int SelectedLinkNo)
{
	if(m_LinkNoMap.find(SelectedLinkNo) != m_LinkNoMap.end())
	{
		DTALink* pLink= m_LinkNoMap [SelectedLinkNo];
		GDPoint pt;
		pt.x = (pLink->m_FromPoint.x + pLink->m_ToPoint.x)/2;
		pt.y = (pLink->m_FromPoint.y + pLink->m_ToPoint.y)/2;
		m_Origin = pt;

		CTLiteView* pView = 0;
		POSITION pos = GetFirstViewPosition();
		if(pos != NULL)
		{
			pView = (CTLiteView*) GetNextView(pos);
			if(pView!=NULL)
			{
				pView->m_Origin = m_Origin;
				pView->Invalidate ();
			}
		}

	}
}

void CTLiteDoc::ShowPathListDlg(bool bShowFlag)
{
	if(bShowFlag)
	{
		if(g_pPathListDlg==NULL)
		{
			g_pPathListDlg = new CDlgPathList();
			g_pPathListDlg->m_pDoc = this;
			g_pPathListDlg->Create(IDD_DIALOG_PATH_LIST);
		}

		// update using pointer to the active document; 

		if(g_pPathListDlg->GetSafeHwnd())
		{
			g_pPathListDlg->m_pDoc = this;
			g_pPathListDlg->ReloadData ();
			g_pPathListDlg->ShowWindow(SW_HIDE);
			g_pPathListDlg->ShowWindow(SW_SHOW);
		}
	}else
	{
		if(g_pPathListDlg!=NULL && g_pPathListDlg->GetSafeHwnd())
		{
			g_pPathListDlg->ShowWindow(SW_HIDE);
		}
	}
}

void CTLiteDoc::OnMoePathlist()
{
	m_bShowPathList = !m_bShowPathList;
	ShowPathListDlg(m_bShowPathList);
}


bool g_TestValidDocument(CTLiteDoc* pDoc)
{
	std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
	while (iDoc != g_DocumentList.end())
	{
		if(pDoc == (*iDoc))
			return true;

		iDoc++;
	}

	return false;
}

void CTLiteDoc::OnViewShowmoe()
{
	g_LinkMOEDlgShowFlag = !g_LinkMOEDlgShowFlag;
	if(g_LinkMOEDlgShowFlag)
	{
		if(g_LinkMOEDlg==NULL)
		{
			g_LinkMOEDlg = new CDlgMOE();

			g_LinkMOEDlg->m_TimeLeft = 0 ;
			g_LinkMOEDlg->m_TimeRight = g_Simulation_Time_Horizon ;
			g_LinkMOEDlg->m_pDoc = this;
			g_LinkMOEDlg->SetModelessFlag(TRUE);
			g_LinkMOEDlg->Create(IDD_DIALOG_MOE);
		}

		if(g_LinkMOEDlg->GetSafeHwnd())
		{
			// assignemnt document pointers
			std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
			while (iDoc != g_DocumentList.end())
			{
				if ((*iDoc)->m_NodeSet.size()>0)
				{
					if(g_LinkMOEDlg->m_pDoc ==NULL)
						g_LinkMOEDlg->m_pDoc = (*iDoc);
					else if((*iDoc)!= g_LinkMOEDlg->m_pDoc)
					{
						g_LinkMOEDlg->m_pDoc2 = (*iDoc);
						g_LinkMOEDlg->m_bDoc2Ready = true;
					}
				}
				iDoc++;
			}
			g_LinkMOEDlg->ShowWindow(SW_HIDE);
			g_LinkMOEDlg->ShowWindow(SW_SHOW);
		}
	}else
	{
		if(g_LinkMOEDlg!=NULL && g_LinkMOEDlg->GetSafeHwnd())
		{
			g_LinkMOEDlg->ShowWindow(SW_HIDE);
		}
	}

}

void CTLiteDoc::OnUpdateViewShowmoe(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(g_LinkMOEDlgShowFlag);
}

void CTLiteDoc::OnFileUploadlinkdatatogooglefusiontable()
{
	AfxMessageBox("Please first save the data set to a folder, and they you can upload input_link.csv to Goolge Fusion Tables.");
}

void CTLiteDoc::On3Viewdatainexcel()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_ODMOE.csv");
}

void CTLiteDoc::On5Viewdatainexcel()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_LinkTDMOE.csv");
}

void CTLiteDoc::OnMoeViewnetworktimedependentmoe()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_NetworkTDMOE.csv");
}

void CTLiteDoc::On2Viewdatainexcel33398()
{
	// TODO: Add your command handler code here
}

void CTLiteDoc::On2Viewnetworkdata()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_NetworkTDMOE.csv");
}

void CTLiteDoc::On3Viewoddatainexcel()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_ODMOE.csv");
}

void CTLiteDoc::OnMoeOpenallmoetables()
{


	OpenCSVFileInExcel(m_ProjectDirectory+"output_summary.csv");
	OpenCSVFileInExcel(m_ProjectDirectory+"output_NetworkTDMOE.csv");
	OpenCSVFileInExcel(m_ProjectDirectory+"output_ODMOE.csv");
	OpenCSVFileInExcel(m_ProjectDirectory+"output_LinkMOE.csv");



	//	OpenCSVFileInExcel(m_ProjectDirectory+"output_NetworkTDMOE.csv");
	//	OpenCSVFileInExcel(m_ProjectDirectory+"output_LinkTDMOE.csv");
}



void CTLiteDoc::OnBnClickedButtonDatabase()
{

}

void CTLiteDoc::OnToolsUnittesting()
{

	if(g_pUnitTestingListDlg==NULL)
	{
		g_pUnitTestingListDlg = new CDlg_UnitTestingList();
		g_pUnitTestingListDlg->m_pDoc = this;
		g_pUnitTestingListDlg->Create(IDD_DIALOG_UNIT_TEST_LIST);
	}

	// update using pointer to the active document; 

	if(g_pUnitTestingListDlg->GetSafeHwnd())
	{
		g_pUnitTestingListDlg->m_pDoc = this;
		g_pUnitTestingListDlg->ShowWindow(SW_HIDE);
		g_pUnitTestingListDlg->ShowWindow(SW_SHOW);
	}
}


void CTLiteDoc::OnViewTraininfo()
{
// potential memory leak
	CDlgTrainInfo* m_pDlg = NULL; 

	m_pDlg = new CDlgTrainInfo; 

	m_pDlg->m_pDoc = this;
	m_pDlg->SetModelessFlag(true); // voila! this is all it takes to make your dlg modeless!
	m_pDlg->Create(IDD_DIALOG_Train); 
	m_pDlg->ShowWindow(SW_SHOW); 

}

void CTLiteDoc::OnImportAmsdataset()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Importing Configuration (*.ini)|*.ini|"));
	if(dlg.DoModal() == IDOK)
	{
		ImportingTransportationPlanningDataSet(dlg.GetPathName(),true,true);
	}

	CDlgFileLoading dlg_loading;
	dlg_loading.m_pDoc = this;
	dlg_loading.DoModal ();

	UpdateAllViews(0);

}

void CTLiteDoc::OnDemandfileOddemandmatrix()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Demand matrix csv (*.csv)|*.csv|"));
	if(dlg.DoModal() == IDOK)
	{
		ReadDemandMatrixFile(dlg.GetPathName(),1);
	}
}

void CTLiteDoc::OnDemandfileHovoddemandmatrix()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Demand matrix csv (*.csv)|*.csv|"));
	if(dlg.DoModal() == IDOK)
	{
		ReadDemandMatrixFile(dlg.GetPathName(),2);
	}
}

void CTLiteDoc::OnDemandfileTruckoddemandmatrix()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Demand matrix csv (*.csv)|*.csv|"));
	if(dlg.DoModal() == IDOK)
	{
		ReadDemandMatrixFile(dlg.GetPathName(),3);
	}
}

void CTLiteDoc::OnDemandfileIntermodaloddemandmatrix()
{
	CFileDialog dlg(TRUE, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Demand matrix csv (*.csv)|*.csv|"));
	if(dlg.DoModal() == IDOK)
	{
		ReadDemandMatrixFile(dlg.GetPathName(),4);
	}
}


void CTLiteDoc::OnToolsGeneratephysicalzonecentroidsonroadnetwork()
{
	CWaitCursor cursor;
	// step 1: mark old centroids, remove old activity locations 
	std::map<int, DTAZone>	:: iterator itr;
	for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)
	{
		for(int i = 0; i< itr->second.m_ActivityLocationVector .size(); i++)
		{
			DTAActivityLocation element = itr->second.m_ActivityLocationVector[i];

			if( element.NodeNumber == 54317 )
			{
				TRACE("");
			}

			m_NodeIDMap [m_NodeNumbertoIDMap[element.NodeNumber ] ] -> m_CentroidUpdateFlag = 1; // old centroid
		}


	}

	// step 2: create new centriods: find a pair of incoming and outgoing links to centroid, if exist

	std::list<DTALink*>::iterator iLink;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		DTALink * pLink = (*iLink);

		if(pLink==NULL)
			break;
		DTALink * pRevLink = NULL; //reversed link
		unsigned long ReversedLinkKey = GetLinkKey(pLink->m_ToNodeID, pLink->m_FromNodeID);

		int reversed_link_id = 0;
		if ( m_NodeIDtoLinkMap.find ( ReversedLinkKey) != m_NodeIDtoLinkMap.end())
		{
			pRevLink = m_NodeIDtoLinkMap[ReversedLinkKey];
		}

		DTANode* pFromNode = m_NodeIDMap[pLink->m_FromNodeID ];
		DTANode* pToNode = m_NodeIDMap[pLink->m_ToNodeID ];

		if(pFromNode->m_NodeNumber == 220 &&  pToNode->m_NodeNumber == 57185)
		{
			TRACE("%d ->%d\n ",  pFromNode->m_NodeNumber ,  pToNode->m_NodeNumber);
		}

		if(pFromNode->m_CentroidUpdateFlag >=1 && pRevLink != NULL && m_LinkTypeMap[pLink->m_link_type ].IsConnector()) // old centroid and no-external origin node // if it is an external origin node, then we do not need to add nodes
		{
			int zone_number = pFromNode->m_ZoneID ;
			int new_node_number;
			// construct new node number based on from and to node ids
			if(pFromNode->m_NodeID < pToNode->m_NodeID)
				new_node_number = pFromNode->m_NodeID * 10000 + pToNode->m_NodeID ;  // fromID*10000+ToID
			else
				new_node_number = pToNode->m_NodeID * 10000 +  pFromNode->m_NodeID ;  // ToID*10000+fromID

			// add new node and update from_node
			if(m_NodeNumbertoIDMap.find(new_node_number) == m_NodeNumbertoIDMap.end() )
			{
				GDPoint	pt;
				pt.x = (pFromNode->pt.x + pToNode->pt.x) /2;
				pt.y = (pFromNode->pt.y + pToNode->pt.y) /2;

				AddNewNode(pt, new_node_number,false);

				// mark it as activity location.
				DTAActivityLocation element;
				element.ZoneID  = zone_number;
				element.NodeNumber = new_node_number;
				element.External_OD_flag = 0;

				int new_node_id = m_NodeNumbertoIDMap[new_node_number];

				DTANode* pNewNode = m_NodeIDMap [new_node_id ] ;
				m_NodeIDtoZoneNameMap[new_node_id] = zone_number;

				pNewNode -> m_ZoneID = zone_number;
				pNewNode ->m_Name == "New_Node";
				pNewNode ->m_External_OD_flag = element.External_OD_flag;

				m_ZoneMap [zone_number].m_ActivityLocationVector .push_back (element);

				//update from node of this link
				pLink->m_FromNodeNumber = new_node_number;
				pLink->m_FromNodeID  = new_node_id;
				pLink->m_FromPoint = pt;
				pLink->m_CentroidUpdateFlag = 1;

				// do not look at the downstream node as centroid again!
				pToNode->m_CentroidUpdateFlag  = 0;

				if( pToNode->m_NodeNumber == 54317)
					TRACE("");

				pLink->m_ShapePoints .clear();
				pLink->m_ShapePointRatios  .clear();
				pLink->m_ShapePoints.push_back (pLink->m_FromPoint);
				pLink->m_ShapePoints.push_back (pLink->m_ToPoint);

				//update downstream node of the reversed link
				pRevLink->m_CentroidUpdateFlag = 1;
				pRevLink->m_ToNodeNumber = new_node_number;
				pRevLink->m_ToNodeID  = new_node_id;
				pRevLink->m_ToPoint = pt;

				pRevLink->m_ShapePoints .clear();
				pRevLink->m_ShapePointRatios  .clear();
				pRevLink->m_ShapePoints.push_back (pLink->m_ToPoint);
				pRevLink->m_ShapePoints.push_back (pLink->m_FromPoint);

			}  //two-way link  from node

			if( pFromNode->m_NodeNumber == 54317)
				TRACE("");
			pFromNode->m_CentroidUpdateFlag+=1; // +1 as to be removed, we "touch" this node

		}  // centriod 

	}  // for each link


	// step 2.1: do not delete adjacent nodes of physical links


	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		DTALink * pLink = (*iLink);
		if(m_LinkTypeMap[pLink->m_link_type ].IsConnector()== false)  // not a connector, so the ajacent node number will still appear in input_link.csv file after generating physical links 
		{

			m_NodeIDMap [pLink->m_FromNodeID ] -> m_CentroidUpdateFlag = 0; // no change
			m_NodeIDMap [pLink->m_ToNodeID ] -> m_CentroidUpdateFlag = 0; // no change


		}

	}
	// step 3: remove old centroids
	std::list<DTANode*>::iterator iNode;
	iNode = m_NodeSet.begin();

	while(iNode != m_NodeSet.end())
	{
		if((*iNode)->m_CentroidUpdateFlag >= 2)
		{
			int ZoneID = (*iNode)->m_ZoneID;

			m_ZoneMap[ZoneID].RemoveNodeActivityMode ((*iNode)->m_NodeNumber);

			m_NodeIDMap[(*iNode)->m_NodeID ] = NULL;
			m_NodeNumbertoIDMap[(*iNode)->m_NodeNumber  ] = -1;
			iNode = m_NodeSet.erase  (iNode); //  correctly update the iterator to point to the location after the iterator we removed.


		}else
		{
			++iNode;
		}
	}

	// update m_path map 
	m_Movement3NodeMap.clear();

	std::map<CString, PathStatistics> ::iterator itr_path;
	for (itr_path = m_PathMap.begin(); itr_path != m_PathMap.end(); itr_path++)
	{

		// update node numbers
		(*itr_path).second.m_NodeVector.clear();

		int i;
		for(i = 0; i < (*itr_path).second.m_LinkPointerVector.size(); i++)
		{

			DTALink* pLink = (*itr_path).second.m_LinkPointerVector[i];

			if(pLink!=NULL)
			{
				int from_node_number = pLink->m_FromNodeNumber   ;
				int to_node_number = pLink->m_ToNodeNumber ;

				if(i==0) 
				{
					(*itr_path).second.m_NodeVector.push_back(from_node_number);
				}

				(*itr_path).second.m_NodeVector.push_back(to_node_number);
			}


		}

		// update movement counts
		for(i = 0; i < (*itr_path).second.m_NodeVector.size(); i++)
		{
			if(i>=2)
			{
				CString movement_label;
				movement_label.Format ("%d;%d;%d",(*itr_path).second.m_NodeVector[i-2],(*itr_path).second.m_NodeVector[i-1],(*itr_path).second.m_NodeVector[i]);

				m_Movement3NodeMap[movement_label].TotalVehicleSize ++;


			}

		}


		// one vehicle type for now

	}

	GenerateOffsetLinkBand();
	UpdateAllViews(0);
}



void CTLiteDoc::OnNodeIncreasenodetextsize()
{

	m_NodeTextDisplayRatio *=1.1;

	UpdateAllViews(0);

}

void CTLiteDoc::OnNodeDecreasenodetextsize()
{
	m_NodeTextDisplayRatio /=1.1;

	if(m_NodeTextDisplayRatio <2)  
		m_NodeTextDisplayRatio = 2;

	UpdateAllViews(0);
}



void CTLiteDoc::OnImportSynchroutdfcsvfiles()
{
	ReadSynchroUniversalDataFiles();
	OnFileSaveProjectAs();

}

void CTLiteDoc::OnProjectEditmoesettings()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"input_MOE_settings.csv");
}

void CTLiteDoc::OnProjectMultiScenarioResults()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_multi_scenario_results.csv");
}

void CTLiteDoc::OnProject12()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_Path.csv");
}

void CTLiteDoc::OnViewMovementMoe()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_MovementMOE.csv");
}

void CTLiteDoc::OnProjectTimeDependentLinkMoe()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_LinkTDMOE.csv");
}

void CTLiteDoc::OnViewOdmeResult()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_ODME_MOE.csv");
}

void CTLiteDoc::OnProjectViewAgentMoe()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"output_agent.csv");
}

void CTLiteDoc::OnProjectOdmatrixestimationinput()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"ODME_Settings.txt");
}

void CTLiteDoc::OnProjectInputsensordataforodme()
{
	OpenCSVFileInExcel(m_ProjectDirectory+"input_sensor.csv");
}

void CTLiteDoc::OnHelpUserguide()
{
	CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

	CString UserGuide;

	UserGuide.Format ("%s\\USERS_GUIDE_NEXTA_DTALITE.pdf",pMainFrame->m_CurrentDirectory);

	OpenCSVFileInExcel(UserGuide);
}

void CTLiteDoc::OnToolsGenerateodmatrixgravitymodel()
{
	if(RunGravityModel() == true)
	{
	SaveAMS_ODTable();
	OnToolsProjectfolder();
	}
}

void CTLiteDoc::ShowTextLabel()
{
}
void CTLiteDoc::OnLinkattributedisplayLinkname()
{
	m_LinkMOEMode = MOE_none;
	ShowTextLabel();
	UpdateAllViews(0);
}

void CTLiteDoc::OnUpdateLinkattributedisplayLinkname(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_LinkMOEMode == MOE_none);
}

void CTLiteDoc::OnToolsGeneratesignalcontrollocations()
{
	CWaitCursor wait;

	int MinimumSpeedLimit4SignalControl  = 30;
	int MaximumSpeedLimit4SignalControl  = 60;

	std::list<DTANode*>::iterator iNode;
	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
		DTANode* pNode = (*iNode);
		pNode->m_ControlType = 0;  // reset control type
		pNode->m_CycleLengthInSecond =0;

	}

	std::list<DTALink*>::iterator iLink;


	int signal_count  = 0;
	for (iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{

		DTALink* pLink = (*iLink);

		if( m_LinkTypeMap[pLink->m_link_type ].IsArterial () == true &&
			pLink->m_SpeedLimit> MinimumSpeedLimit4SignalControl && pLink->m_SpeedLimit<= MaximumSpeedLimit4SignalControl && 
			m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType == 0 && 
			m_NodeIDMap[pLink->m_ToNodeID ]->m_IncomingLinkVector .size() >=3) 
		{ // speed range between 30 and 60, arterial streets, intersection has at least 3 legs
			m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType = m_ControlType_PretimedSignal;  // signal control
			m_NodeIDMap[pLink->m_ToNodeID ]->m_CycleLengthInSecond  = 120;  // signal control
			signal_count ++;
		}

		if(pLink->m_SpeedLimit<=30 && m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType == 0)
		{
			m_NodeIDMap[pLink->m_ToNodeID ]->m_ControlType = m_ControlType_4wayStopSign;  // signal control
		}

	}

	CString msg;
	msg.Format("%d nodes are associated with signal control type.\nPlease save the project to change control type in file input_node.csv.", signal_count);

	AfxMessageBox(msg,MB_ICONINFORMATION);
	UpdateAllViews(0);
}

void CTLiteDoc::GenerateMovementCountFromVehicleFile()
{

	if(m_Movement3NodeMap.size()>=1)
		return;

	if(m_VehicleSet.size()>=50000)
	{
		AfxMessageBox("Generating movement count from vehicle/agent files might take a while...", MB_ICONINFORMATION);

	}

	std::list<DTAVehicle*>::iterator iVehicle;

	for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++)
	{
		DTAVehicle* pVehicle = (*iVehicle);

		for(int i= 2; i<pVehicle->m_NodeSize; i++)
		{
			DTALink* pLink0 = m_LinkNoMap[pVehicle->m_NodeAry[i-1].LinkNo]; // i=0, LinkNo = -1;
			DTALink* pLink1 = m_LinkNoMap[pVehicle->m_NodeAry[i].LinkNo];

			CString movement_label;
			movement_label.Format ("%d;%d;%d", pLink0->m_FromNodeNumber  ,  pLink0->m_ToNodeNumber , pLink1->m_ToNodeNumber);
			m_Movement3NodeMap[movement_label].TotalVehicleSize ++;

		}
	}

	FILE* st;

	CString directory;
	directory = m_ProjectFile.Left(m_ProjectFile.ReverseFind('\\') + 1);

	fopen_s(&st,directory+"AMS_movement.csv","w");
	if(st!=NULL)
	{
		fprintf(st,"movement_index,three-node key,count\n");

		int movement_index = 1;
		std::map<CString, Movement3Node> ::const_iterator itr;
		for (itr = m_Movement3NodeMap.begin(); itr != m_Movement3NodeMap.end(); itr++)
		{
			fprintf(st, "%d,%s,%d\n", 
				movement_index++,
				(*itr).first, 
				(*itr).second.TotalVehicleSize );

		}
		fclose(st);
	}else
	{
		AfxMessageBox("File AMS_movement.csv cannot be opened.");

	}


}

void CTLiteDoc::OnAssignmentSimulatinSettinsClicked()
{
	if(m_ProjectDirectory.GetLength()==0)
	{
		AfxMessageBox("The project has not been loaded.");
		return;
	}
	CAssignmentSimulationSettingDlg dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();
}

void CTLiteDoc::OnProjectNetworkData()
{
	if(m_ProjectDirectory.GetLength()==0)
	{
		AfxMessageBox("The project has not been loaded.");
		return;
	}
	CNetworkDataSettingDlg dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();

}

void CTLiteDoc::OnLinkAddsensor()
{
	if(m_SelectedLinkNo==-1)
	{
		AfxMessageBox("Please select a link first.");
		return;
	}

	DTALink* pLink= m_LinkNoMap [m_SelectedLinkNo];
	if(pLink!=NULL)
	{

		CNetworkDataSettingDlg dlg;
		dlg.m_pDoc = this;
		dlg.m_SelectTab = 4; // _SENSOR_DATA
		dlg.m_SelectedFromNodeName = pLink->m_FromNodeNumber;
		dlg.m_SelectedToNodeName = pLink->m_ToNodeNumber;

		if(dlg.DoModal()==IDOK)
		{
			ReadSensorData(m_ProjectDirectory+"input_sensor.csv");
			UpdateAllViews(0);
		}

	}
}



void CTLiteDoc::OnToolsObtainCyclelengthfromNearbySignals()
{


	FILE* st = NULL;

	CString str_log_file;
	str_log_file.Format ("%s\\signal_data_reading_log.csv",m_ProjectDirectory);

	fopen_s(&st,str_log_file,"w");

	int count = 0;
	if( st !=NULL)
	{

		fprintf(st,"baseline_node_id,reference_node_id,base_line_cycle_length_in_second,reference_cycle_length_in_second\n");

		double min_distance =  m_UnitFeet * 1000;  //1000 feet


		// iNode is the node in the current view, pNode is the reference node in the reference document
		for (std::list<DTANode*>::iterator  iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
			if((*iNode)->m_LayerNo == 0 && (*iNode)->m_ControlType == m_ControlType_PretimedSignal)
			{
				DTANode* pNode = FindSignalNodeWithCoordinate((*iNode)->pt.x , (*iNode)->pt.y, 1, min_distance);

				if(pNode!=NULL)
				{
					fprintf(st, "%d,%d,%d,%d\n", (*iNode)->m_NodeNumber , pNode->m_NodeNumber ,(*iNode)->m_CycleLengthInSecond,pNode->m_CycleLengthInSecond);
					(*iNode)->m_CycleLengthInSecond = pNode->m_CycleLengthInSecond;

					count++;
				}



			}
		}


		fclose(st);
	}


	OpenCSVFileInExcel(str_log_file);

	CString str;
	str.Format("%d nodes have obtained cycle length from the reference network. To accept the changes, please save the network.",count);
	AfxMessageBox(str, MB_ICONINFORMATION);

}


void CTLiteDoc::OnToolsSaveprojectforexternallayer()
{
	CFileDialog fdlg (FALSE, "*.tnp", "*.tnp",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_LONGNAMES,
		"Transportation Network Project (*.tnp)|*.tnp|");

	if(fdlg.DoModal()==IDOK)
	{
		CString path = fdlg.GetFileName ();
		CWaitCursor wait;
		m_ProjectFile = fdlg.GetPathName();
		m_ProjectTitle = GetWorkspaceTitleName(m_ProjectFile);
		int ExternalLayerNo = 1;
		if(SaveProject(fdlg.GetPathName(),ExternalLayerNo))
		{
			CString msg;
			msg.Format ("Files input_node.csv, input_link.csv and input_zone.csv have been successfully saved with %d nodes, %d links, %d zones.",m_NodeSet.size(), m_LinkSet.size(), m_ZoneMap.size());
			AfxMessageBox(msg,MB_OK|MB_ICONINFORMATION);

			SetTitle(m_ProjectTitle);


		}
	}
}

void CTLiteDoc::OnToolsUpdateeffectivegreentimebasedoncyclelength()
{

	CWaitCursor wait;
	int count = 0;
	for (std::list<DTALink*>::iterator  iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
	{
		if((*iLink)->m_LayerNo == 0 )
		{
			int ToNodeID = (*iLink)->m_ToNodeID ;
			DTANode* pNode = m_NodeIDMap[ToNodeID];
			//set default green time 
			if(pNode->m_ControlType == m_ControlType_PretimedSignal || 
				pNode->m_ControlType == m_ControlType_ActuatedSignal)
			{

				// from given BPR capacity to determine the effective green time
				(*iLink)->m_EffectiveGreenTimeInSecond = (int)(pNode->m_CycleLengthInSecond * (*iLink)->m_LaneCapacity / (*iLink)->m_Saturation_flow_rate_in_vhc_per_hour_per_lane);
				count++;


			}else
			{
				(*iLink)->m_EffectiveGreenTimeInSecond =0;

			}
		}
	}

	CString str;
	str.Format("%d links have effective updated green time. To accept the changes, please save the network.",count);
	AfxMessageBox(str, MB_ICONINFORMATION);


}
void CTLiteDoc::OnMoeTableDialog()
{
	CDlgMOETabView dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();

}

bool CTLiteDoc::ReadTraceBinFile(LPCTSTR lpszFileName, int date_id)
{
	//   cout << "Read vehicle file... "  << endl;
	// vehicle_id,  origin_zone_id, destination_zone_id, departure_time,
	//	arrival_time, complete_flag, trip_time, demand_type, occupancy, information_type,
	//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
	//	node id, node arrival time




	typedef struct  
	{
		int original_vehicle_id;
		int date_id;
		int vehicle_id;
		int from_zone_id;
		int to_zone_id;
		float departure_time;
		float arrival_time;
		float trip_time;
		float total_distance_in_km;
		int number_of_nodes;
		float reserved_field1;
		float reserved_field2;
		float reserved_field3;

	} struct_Vehicle_Header;


	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"rb");
	if(st!=NULL)
	{


		m_VehicleSet.clear();

		struct_Vehicle_Header header;

		int count =0;
		while(!feof(st))
		{

			size_t result = fread(&header,sizeof(struct_Vehicle_Header),1,st);

			if( header.vehicle_id < 0)
				break;

			if(result!=1)  // read end of file
				break;

			int PathNodeVector[MAX_NODE_SIZE_IN_A_PATH];
			int PathNodeArrivalTimeVector[MAX_NODE_SIZE_IN_A_PATH];
			int PathNodeVectorNo = 0;
			int NetworkNodeNo = -1;
			int NetworkLinkID = -1;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			//			pVehicle->m_bGPSVehicle = true;
			pVehicle->m_DateID 		= header.date_id;

			pVehicle->m_VehicleID		= m_VehicleSet.size();

			pVehicle->m_OriginZoneID	= header.from_zone_id;
			pVehicle->m_DestinationZoneID= header.to_zone_id;

			pVehicle->m_Distance = header.total_distance_in_km * 0.621371; // km to miles


			pVehicle->m_DepartureTime	=  header.departure_time;
			pVehicle->m_ArrivalTime =  header.arrival_time;

			if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
				g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

			pVehicle->m_bComplete = true;
			pVehicle->m_DemandType = 1;

			pVehicle->m_PricingType = 1;

			//pVehicle->m_VehicleType = (unsigned char)g_read_integer(pFile);

			pVehicle->m_VOT = 10;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions = 0;


			pVehicle->m_TripTime  = header.trip_time;

			pVehicle->m_VehicleLocationSize	= header.number_of_nodes;

			if(pVehicle->m_VehicleLocationSize==0 )
			{
				CString error_msg;
				error_msg.Format("reading error in file %s: vehicle id =%d",lpszFileName, header.vehicle_id);

			}

			if(pVehicle->m_VehicleLocationSize>=1)  // in case reading error
			{

				float total_distance = 0;
				pVehicle->m_LocationRecordAry = new VehicleLocationRecord[pVehicle->m_VehicleLocationSize];

				pVehicle->m_NodeNumberSum = 0;
				for(int i=0; i< pVehicle->m_VehicleLocationSize; i++)
				{
					VehicleLocationRecord node_element;
					fread(&pVehicle->m_LocationRecordAry[i],sizeof(VehicleLocationRecord),1,st);

					double min_distance = m_UnitMile*0.5;

					// update origin and destination zone ids

					if(i==0 && pVehicle->m_OriginZoneID == 0)
					{
						int ZoneNumber  = FindClosestZone( pVehicle->m_LocationRecordAry[i].x, pVehicle->m_LocationRecordAry[i].y, min_distance	);

						if(ZoneNumber!=-1)
							pVehicle->m_OriginZoneID = ZoneNumber;

					}

					//					if(i== pVehicle->m_VehicleLocationSize -1 && pVehicle->m_DestinationZoneID == 0)
					//					{
					//					int ZoneNumber  = FindClosestZone( pVehicle->m_LocationRecordAry[i].x, pVehicle->m_LocationRecordAry[i].y, min_distance	);
					//					if(ZoneNumber!=-1)
					//					pVehicle->m_DestinationZoneID = ZoneNumber;
					//
					//					}
					//
					//					int LinkID = pVehicle->m_LocationRecordAry[i].linkid;
					//
					//					if(LinkID != -1  && LinkID != NetworkLinkID) 
					//					{
					//						PathNodeVector[PathNodeVectorNo] = pVehicle->m_LocationRecordAry[i].linkid ;
					//
					////						TRACE("no. %d,link id = %,\n",i,pVehicle->m_LocationRecordAry[i].linkid);
					//						PathNodeArrivalTimeVector[PathNodeVectorNo] = pVehicle->m_LocationRecordAry[i].time_stamp_in_min;
					//						//pVehicle->m_NodeNumberSum += NodeNo;
					//						PathNodeVectorNo ++;
					//						NetworkLinkID = LinkID;
					//
					//					}
				}


				if(PathNodeVectorNo>=1)
				{

					pVehicle->m_NodeSize = PathNodeVectorNo;
					pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

					pVehicle->m_NodeAry[0].ArrivalTimeOnDSN = PathNodeArrivalTimeVector[0];
					for(int i = 1; i< PathNodeVectorNo; i++)
					{
						DTALink* pLink = FindLinkWithLinkID(PathNodeVector[i]);
						if(pLink!=NULL)
						{
							pVehicle->m_NodeAry[i].LinkNo  = pLink->m_LinkNo ;
							pLink->m_total_link_volume +=1;

							pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = PathNodeArrivalTimeVector[i];
							pVehicle->m_NodeNumberSum += pLink->m_FromNodeNumber ;

							//	//overwrite zone id for nodes
							//	if(i==1 && pVehicle->m_OriginZoneID !=0 )
							//	{

							//DTAActivityLocation element;
							//element.ZoneID  = pVehicle->m_OriginZoneID;
							//element.NodeNumber = pLink->m_FromNodeNumber;
							//element.External_OD_flag = 0;

							//m_NodeIDMap[pLink->m_FromNodeID ] ->m_ZoneID = pVehicle->m_OriginZoneID;
							//m_ZoneMap [element.ZoneID ].m_ActivityLocationVector .push_back (element);
							//	
							//	}

						}else
						{
							pVehicle->m_NodeAry[i].LinkNo = -1;

							//						TRACE("cannot find link %d\n", PathLinkVector[i]);
						}
					}

				}

				if(pVehicle->m_OriginZoneID ==0 || pVehicle->m_DestinationZoneID ==0)
				{
					TRACE("\nvehicle id: %d, O: %d, D: %d", header.vehicle_id, header.from_zone_id,header.to_zone_id );
				}


				// movement count

				m_VehicleSet.push_back (pVehicle);
				m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;


				if(count%100 == 0 && count >=1)
				{
					CString str;
					str.Format ("Loading %d GPS records...", count);
					SetStatusText(str);
				}

				count++;
			} 
		}


		fclose(st);

		if(count >=1 ) //with GPS data
		{
			CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
			pMainFrame->m_bShowLayerMap[layer_vehicle_position]  = true;
			//			m_bGPSDataSet  = true;

		}

		m_SimulationVehicleDataLoadingStatus.Format ("%d traces are loaded from files %s...",m_VehicleSet.size(),lpszFileName);
		return true;

	}
	else
	{
		CString msg;
		msg.Format("File %s cannot be opened.", lpszFileName);
		AfxMessageBox(msg);
		return false;
	}
}


bool CTLiteDoc::ReadGPSBinFile(LPCTSTR lpszFileName, int date_id)
{
	//   cout << "Read vehicle file... "  << endl;
	// vehicle_id,  origin_zone_id, destination_zone_id, departure_time,
	//	arrival_time, complete_flag, trip_time, demand_type, occupancy, information_type,
	//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
	//	node id, node arrival time




	typedef struct  
	{
		int original_vehicle_id;
		int date_id;
		int vehicle_id;
		int from_zone_id;
		int to_zone_id;
		float departure_time;
		float arrival_time;
		float trip_time;
		float total_distance_in_km;
		int number_of_nodes;
		float reserved_field1;
		float reserved_field2;
		float reserved_field3;

	} struct_Vehicle_Header;


	FILE* st = NULL;
	fopen_s(&st,lpszFileName,"rb");
	if(st!=NULL)
	{


		m_VehicleSet.clear();

		struct_Vehicle_Header header;

		int count =0;
		while(!feof(st))
		{

			size_t result = fread(&header,sizeof(struct_Vehicle_Header),1,st);

			if( header.vehicle_id < 0)
				break;

			if(result!=1)  // read end of file
				break;

			int PathLinkVector[MAX_NODE_SIZE_IN_A_PATH];
			int PathNodeArrivalTimeVector[MAX_NODE_SIZE_IN_A_PATH];
			int PathNodeVectorNo = 0;
			int NetworkNodeNo = -1;
			int NetworkLinkID = -1;

			DTAVehicle* pVehicle = 0;
			pVehicle = new DTAVehicle;
			pVehicle->m_bGPSVehicle = true;
			pVehicle->m_DateID 		= header.date_id;

			pVehicle->m_VehicleID		= m_VehicleSet.size();

			pVehicle->m_OriginZoneID	= header.from_zone_id;
			pVehicle->m_DestinationZoneID= header.to_zone_id;

			pVehicle->m_Distance = header.total_distance_in_km * 0.621371; // km to miles


			pVehicle->m_DepartureTime	=  header.departure_time;
			pVehicle->m_ArrivalTime =  header.arrival_time;

			if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
				g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

			pVehicle->m_bComplete = true;
			pVehicle->m_DemandType = 1;

			pVehicle->m_PricingType = 1;

			//pVehicle->m_VehicleType = (unsigned char)g_read_integer(pFile);

			pVehicle->m_VOT = 10;
			pVehicle->m_TollDollarCost = 0;
			pVehicle->m_Emissions = 0;


			pVehicle->m_TripTime  = header.trip_time;

			pVehicle->m_VehicleLocationSize	= header.number_of_nodes;

			if(pVehicle->m_VehicleLocationSize==0 )
			{
				CString error_msg;
				error_msg.Format("reading error in file %s: vehicle id =%d",lpszFileName, header.vehicle_id);

			}

			if(pVehicle->m_VehicleLocationSize>=1)  // in case reading error
			{

				float total_distance = 0;
				pVehicle->m_LocationRecordAry = new VehicleLocationRecord[pVehicle->m_VehicleLocationSize];

				pVehicle->m_NodeNumberSum = 0;
				for(int i=0; i< pVehicle->m_VehicleLocationSize; i++)
				{
					VehicleLocationRecord node_element;
					fread(&pVehicle->m_LocationRecordAry[i],sizeof(VehicleLocationRecord),1,st);

					double min_distance = m_UnitMile*0.5;

					// update origin and destination zone ids

					if(i==0 && pVehicle->m_OriginZoneID == 0)
					{
						int ZoneNumber  = FindClosestZone( pVehicle->m_LocationRecordAry[i].x, pVehicle->m_LocationRecordAry[i].y, min_distance	);

						if(ZoneNumber!=-1)
							pVehicle->m_OriginZoneID = ZoneNumber;

					}

					if(i== pVehicle->m_VehicleLocationSize -1 && pVehicle->m_DestinationZoneID == 0)
					{
						int ZoneNumber  = FindClosestZone( pVehicle->m_LocationRecordAry[i].x, pVehicle->m_LocationRecordAry[i].y, min_distance	);
						if(ZoneNumber!=-1)
							pVehicle->m_DestinationZoneID = ZoneNumber;

					}

					int LinkID = pVehicle->m_LocationRecordAry[i].linkid;


					PathLinkVector[PathNodeVectorNo] = pVehicle->m_LocationRecordAry[i].linkid ;

					//						TRACE("no. %d,link id = %,\n",i,pVehicle->m_LocationRecordAry[i].linkid);
					PathNodeArrivalTimeVector[PathNodeVectorNo] = pVehicle->m_LocationRecordAry[i].time_stamp_in_min;
					//pVehicle->m_NodeNumberSum += NodeNo;
					PathNodeVectorNo ++;
					NetworkLinkID = LinkID;

				}


				if(PathNodeVectorNo>=1)
				{

					pVehicle->m_NodeSize = PathNodeVectorNo;
					pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];

					pVehicle->m_NodeAry[0].ArrivalTimeOnDSN = PathNodeArrivalTimeVector[0];
					for(int i = 0; i< PathNodeVectorNo; i++)
					{
						DTALink* pLink = FindLinkWithLinkID(PathLinkVector[i]);
						if(pLink!=NULL)
						{
							pVehicle->m_NodeAry[i].LinkNo  = pLink->m_LinkNo ;
							pLink->m_total_link_volume +=1;

							pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = PathNodeArrivalTimeVector[i];
							pVehicle->m_NodeNumberSum += pLink->m_FromNodeNumber ;

							//	//overwrite zone id for nodes
							//	if(i==1 && pVehicle->m_OriginZoneID !=0 )
							//	{

							//DTAActivityLocation element;
							//element.ZoneID  = pVehicle->m_OriginZoneID;
							//element.NodeNumber = pLink->m_FromNodeNumber;
							//element.External_OD_flag = 0;

							//m_NodeIDMap[pLink->m_FromNodeID ] ->m_ZoneID = pVehicle->m_OriginZoneID;
							//m_ZoneMap [element.ZoneID ].m_ActivityLocationVector .push_back (element);
							//	
							//	}

						}else
						{
							pVehicle->m_NodeAry[i].LinkNo = -1;

							//						TRACE("cannot find link %d\n", PathLinkVector[i]);
						}
					}

				}

				if(pVehicle->m_OriginZoneID ==0 || pVehicle->m_DestinationZoneID ==0)
				{
					TRACE("\nvehicle id: %d, O: %d, D: %d", header.vehicle_id, header.from_zone_id,header.to_zone_id );
				}


				// movement count

				m_VehicleSet.push_back (pVehicle);
				m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;


				if(count%100 == 0 && count >=1)
				{
					CString str;
					str.Format ("Loading %d GPS records...", count);
					SetStatusText(str);
				}

				count++;
			} 
		}


		fclose(st);

		if(count >=1 ) //with GPS data
		{
			CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
			pMainFrame->m_bShowLayerMap[layer_vehicle_position]  = true;
			//			m_bGPSDataSet  = true;

		}

		m_SimulationVehicleDataLoadingStatus.Format ("%d traces are loaded from files %s...",m_VehicleSet.size(),lpszFileName);
		return true;

	}
	else
	{
		CString msg;
		msg.Format("File %s cannot be opened.", lpszFileName);
		AfxMessageBox(msg);
		return false;
	}
}


void CTLiteDoc::ResetODMOEMatrix()
{
	m_ZoneNumberVector.clear();
	m_ZoneIDVector.clear ();

	for(int i = 0; i<= m_ODSize; i++)
	{
		m_ZoneIDVector.push_back(-1);
	}

	int count = 0;

	if(m_ZoneMap.size () ==0)  // no data
	{
		for(int i = 1; i<= m_ODSize; i++)
		{
			m_ZoneIDVector[i]=count++;
			m_ZoneNumberVector.push_back (i);
		}

		m_ZoneNoSize  =  m_ODSize;


	}else
	{

		std::map<int, DTAZone>	:: const_iterator itr;

		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)
		{
			m_ZoneNumberVector.push_back ( itr->first);

			if(itr->first < m_ZoneIDVector.size())
			{
			m_ZoneIDVector [itr->first ] = count++;
			}
		}

		m_ZoneNoSize  = m_ZoneNumberVector.size();

	}


	if(m_ODMOEMatrix == NULL  )
	{
		m_ODMOEMatrix = Allocate3DDynamicArray<VehicleStatistics>(m_DemandTypeSize,m_ZoneNoSize,m_ZoneNoSize);
		m_PreviousDemandTypeSize = m_DemandTypeSize;
		m_PreviousZoneNoSize = m_ZoneNoSize ;
	}
	else
	{
		Deallocate3DDynamicArray<VehicleStatistics>(m_ODMOEMatrix,m_PreviousDemandTypeSize, m_PreviousZoneNoSize);
		m_ODMOEMatrix = Allocate3DDynamicArray<VehicleStatistics>(m_DemandTypeSize,m_ZoneNoSize,m_ZoneNoSize);
		m_PreviousDemandTypeSize = m_DemandTypeSize;
		m_PreviousZoneNoSize = m_ZoneNoSize ;
	}

	int p, i, j;

	for(p=0;p<m_DemandTypeSize; p++)
	{
		for(i= 0; i < m_ZoneNoSize ; i++)
		{
			for(j= 0; j< m_ZoneNoSize ; j++)
			{
				m_ODMOEMatrix[p][i][j].Reset ();
			}

		}

	}



	count  = 0;
	std::list<DTAVehicle*>::iterator iVehicle;


	for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++, count++)
	{
		DTAVehicle* pVehicle = (*iVehicle);
		if(/*pVehicle->m_NodeSize >= 2 && */pVehicle->m_bComplete )
		{
			int p = pVehicle->m_DemandType ;
			p = 0;

			if(pVehicle->m_OriginZoneID >m_ZoneIDVector.size())
			{

				CString msg;
				msg.Format("Zone ID %d for vehicle %d has not been defined in the input_zone.csv file. Please check.",pVehicle->m_OriginZoneID , pVehicle->m_VehicleID );
				AfxMessageBox(msg);
				return;
			}


			if(pVehicle->m_OriginZoneID >= m_ZoneIDVector.size())
				continue;

			if(pVehicle->m_DestinationZoneID >= m_ZoneIDVector.size())
				continue;

			int OrgNo = m_ZoneIDVector[pVehicle->m_OriginZoneID];
			int DesNo = m_ZoneIDVector[pVehicle->m_DestinationZoneID];

			if(OrgNo<0 || DesNo< 0)
				continue;
			m_ODMOEMatrix[p][OrgNo][DesNo].TotalVehicleSize+=1;
			m_ODMOEMatrix[p][OrgNo][DesNo].TotalTravelTime += (pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime);
			m_ODMOEMatrix[p][OrgNo][DesNo].TotalDistance += pVehicle->m_Distance;
			m_ODMOEMatrix[p][OrgNo][DesNo].TotalCost += pVehicle->m_TollDollarCost;

			m_ODMOEMatrix[p][OrgNo][DesNo].emissiondata.Energy += pVehicle->m_EmissionData .Energy;
			m_ODMOEMatrix[p][OrgNo][DesNo].emissiondata.CO2 += pVehicle->m_EmissionData .CO2;
			m_ODMOEMatrix[p][OrgNo][DesNo].emissiondata.NOX += pVehicle->m_EmissionData .NOX;
			m_ODMOEMatrix[p][OrgNo][DesNo].emissiondata.CO += pVehicle->m_EmissionData .CO;
			m_ODMOEMatrix[p][OrgNo][DesNo].emissiondata.HC += pVehicle->m_EmissionData .HC;

		}
	}


}

void CTLiteDoc::OnSensortoolsConverttoHourlyVolume()
{
	OpenCSVFileInExcel( m_ProjectDirectory +"output_validation_results.csv");

	if( m_SimulationStartTime_in_min > m_SimulationEndTime_in_min)
	{
		AfxMessageBox("Please load time-dependent simulation data to generate aggregated validation result.");
	}

	int calibration_data_start_time_in_min=0;
	int calibration_data_end_time_in_min= 1440;

	CCSVParser SettingFile;

	if(SettingFile.OpenCSVFile (CString2StdString(m_ProjectDirectory+"input_scenario_settings.csv")))
	{
		while(SettingFile.ReadRecord())
		{
			SettingFile.GetValueByFieldName("calibration_data_start_time_in_min",calibration_data_start_time_in_min);
			SettingFile.GetValueByFieldName("calibration_data_end_time_in_min",calibration_data_end_time_in_min);

		}

	}


	CCSVWriter DataFile;

	CString data_str = m_ProjectDirectory +"output_validation_results_aggregated.csv";

	// Convert a TCHAR string to a LPCSTR
	if(DataFile.Open(CString2StdString(data_str)))
	{

		DataFile.SetFieldName ("from_node_id");
		DataFile.SetFieldName ("to_node_id");
		DataFile.SetFieldName ("name");
		DataFile.SetFieldName ("link_type_name");
		DataFile.SetFieldName ("observed_link_count");
		DataFile.SetFieldName ("simulated_link_count");
		DataFile.SetFieldName ("start_time_in_min");
		DataFile.SetFieldName ("end_time_in_min");
		DataFile.SetFieldName ("sensor_type");
		DataFile.SetFieldName ("aggregation_interval");
		DataFile.SetFieldName ("start_time_in_hour");
		DataFile.SetFieldName ("end_time_in_hour");
		DataFile.SetFieldName ("reference_avg_speed");
		DataFile.SetFieldName ("reference_from_node_id");
		DataFile.SetFieldName ("reference_to_node_id");
		DataFile.SetFieldName ("reference_matching_distance");

		DataFile.WriteHeader ();

		//find refernce document pointer

		CTLiteDoc* pReferenceDoc = NULL;

		std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
		while (iDoc != g_DocumentList.end())
		{	

			if( (*iDoc) != this && (*iDoc)->m_NodeSet .size()>0)
			{
				pReferenceDoc = (*iDoc);
			}

			iDoc++;
		}

		int AggregationTimeIntervalInMin = 60;
		g_MOEAggregationIntervalInMin = 60;

		for (std::list<DTALink*>::iterator iLink  = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			DTALink* pLink = (*iLink);

			if(pLink->m_bSensorData )
			{

				for(int ti = max(calibration_data_start_time_in_min, m_SimulationStartTime_in_min);  
					ti < min(calibration_data_end_time_in_min,m_SimulationEndTime_in_min); ti+=AggregationTimeIntervalInMin)
				{

					int t = int(ti/60)*60;
					float SensorCount  = pLink->GetSensorLinkHourlyVolume(t, t+AggregationTimeIntervalInMin)/60*AggregationTimeIntervalInMin;
					if( SensorCount>1)
					{
						float SimulatedCount = pLink->GetAvgLinkHourlyVolume (t, t+AggregationTimeIntervalInMin)/60*AggregationTimeIntervalInMin;

						DataFile.SetValueByFieldName ("from_node_id", pLink->m_FromNodeNumber);
						DataFile.SetValueByFieldName ("to_node_id", pLink->m_ToNodeNumber);
						DataFile.SetValueByFieldName ("name", pLink->m_Name);
						DataFile.SetValueByFieldName ("link_type_name", m_LinkTypeMap[pLink->m_link_type].link_type_name);
						DataFile.SetValueByFieldName ("sensor_type","link_count_speed");


						DataFile.SetValueByFieldName ("start_time_in_min", t);

						int end_time = t+AggregationTimeIntervalInMin;
						DataFile.SetValueByFieldName ("end_time_in_min", end_time);
						int start_time_in_hour = t/60;
						DataFile.SetValueByFieldName ("start_time_in_hour", start_time_in_hour);

						int end_time_in_hour = end_time/60;
						DataFile.SetValueByFieldName ("end_time_in_hour", end_time_in_hour);
						DataFile.SetValueByFieldName ("aggregation_interval", AggregationTimeIntervalInMin);
						DataFile.SetValueByFieldName ("observed_link_count", SensorCount);
						DataFile.SetValueByFieldName ("simulated_link_count", SimulatedCount);

						float reference_avg_speed = 0;

						if(pReferenceDoc!=NULL)
						{
							float ratio = 0.5;
							GDPoint current_link_point = pLink->GetRelativePosition(ratio);

							double matching_distance = 1000;
							int linkid = pReferenceDoc->SelectLink(current_link_point, matching_distance);
							if(linkid >=0)
							{
								DTALink* pReferenceLink = NULL;

								if( pReferenceDoc->m_LinkNoMap.find(linkid) != pReferenceDoc->m_LinkNoMap.end())
								{
									pReferenceLink = pReferenceDoc->m_LinkNoMap [linkid];
									reference_avg_speed = pReferenceLink->GetSimulationSpeed(start_time_in_hour);

									int reference_from_node_id = pReferenceLink->m_FromNodeNumber ;
									int reference_to_node_id = pReferenceLink->m_ToNodeNumber ;


									double matching_distance_in_feet = matching_distance / m_UnitFeet ;

									DataFile.SetValueByFieldName ("reference_from_node_id", reference_from_node_id);
									DataFile.SetValueByFieldName ("reference_to_node_id", reference_to_node_id);
									DataFile.SetValueByFieldName ("reference_matching_distance", matching_distance_in_feet);


								}

							}

						}

						DataFile.SetValueByFieldName ("reference_avg_speed", reference_avg_speed);

						DataFile.WriteRecord ();


					}

				}

			}
		}
	}

	DataFile.CloseCSVFile ();
	OpenCSVFileInExcel( m_ProjectDirectory +"output_validation_results_aggregated.csv");


}


void CTLiteDoc::ReadTMCSpeedData(LPCTSTR lpszFileName)
{


	std::vector<string> Missing_TMC_vector;
		CCSVParser parser;
		int i= 0;
		if (parser.OpenCSVFile(lpszFileName))
		{
			g_Simulation_Time_Horizon = 1440;

			for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
			{
				(*iLink)->ResetMOEAry(g_Simulation_Time_Horizon);  // use one day horizon as the default value
			}


			while(parser.ReadRecord())
			{

				string TMC;
				if(parser.GetValueByFieldName("TMC",TMC) == false)
					break;

				DTALink* pLink = NULL;

				if(m_TMC2LinkMap.find(TMC)!=m_TMC2LinkMap.end())
				{

					pLink = m_TMC2LinkMap[TMC];
					float max_speed = 10;
					float min_speed = 100;

					for (int t = 0; t< 1440; t+=15)
					{
						CString timestamp;
						timestamp.Format ("Min_%d",t);
						std::string StdString = CString2StdString(timestamp);
						float speed_in_mph = 0;
						if(parser.GetValueByFieldName(StdString,speed_in_mph) != false)
						{
							//TRACE("speed = %f,\n",speed_in_mph);
							if(min_speed > speed_in_mph)
								min_speed = speed_in_mph;

							if(max_speed < speed_in_mph)
								max_speed = speed_in_mph;

							for(int s= 0 ; s<15; s++)
							{
								pLink->m_LinkMOEAry[t+s].SimulationLinkFlow = 1000; // default hourly volume

								pLink->m_LinkMOEAry[t+s].SimulationSpeed = speed_in_mph;
							}

						}

					}
					//reset speed limit
					pLink->m_SpeedLimit = max_speed;


					i++;
				}else
				{
					TRACE("TMC not found: %s\n",TMC.c_str ());

					Missing_TMC_vector.push_back(TMC);

				}


			}



		if(Missing_TMC_vector.size())
		{
			CCSVWriter DataFile;

			CString data_str = m_ProjectDirectory +"log_missing_TMC.csv";

			// Convert a TCHAR string to a LPCSTR
			if(DataFile.Open(CString2StdString(data_str)))
			{

				DataFile.SetFieldName ("TMC");
				DataFile.WriteHeader ();


				for(unsigned i = 0; i < Missing_TMC_vector.size(); i++)
				{
				
					DataFile.SetValueByFieldName ("TMC",Missing_TMC_vector[i] );

					DataFile.WriteRecord ();
				}
				

			}
		CString str;
		str.Format("%d TMC records cannot be found.\nPlease Check the missing TMC log file (%s)", Missing_TMC_vector.size(), data_str);
		AfxMessageBox(str);

		}
			m_bSimulationDataLoaded = true;

			g_Simulation_Time_Stamp = 0; // reset starting time
			g_SimulationStartTime_in_min = 0;

			m_SimulationStartTime_in_min = 0;
			m_SimulationEndTime_in_min = 1440;

			m_TrafficFlowModelFlag = 3; //enable dynamic moe display
			m_SimulationLinkMOEDataLoadingStatus.Format ("%d TMC records are loaded from file %s.",i,lpszFileName);

		
	}
	

}

int CTLiteDoc::SelectLink(GDPoint point, double& final_matching_distance)
{
	double Min_distance  = 99; // at least interset with a link

	int SelectedLinkNo = -1;

	for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink !=m_LinkSet.end(); iLink++)
	{

		for(int si = 0; si < (*iLink) ->m_ShapePoints .size()-1; si++)
		{

			GDPoint p0 = point;
			GDPoint pfrom =  (*iLink)->m_ShapePoints[si];
			GDPoint pto = (*iLink)->m_ShapePoints[si+1];;

			float distance = g_GetPoint2LineDistance(p0, pfrom, pto, m_UnitMile);

			if(distance >0 && distance < Min_distance && (*iLink)->m_DisplayLinkID <0)  // not selected
			{

				m_SelectedLinkNo = (*iLink)->m_LinkNo ;
				Min_distance = distance;
				SelectedLinkNo = (*iLink)->m_LinkNo ;
			}
		}
	}

	final_matching_distance = Min_distance;

	return SelectedLinkNo;
}



void CTLiteDoc::OnTrafficcontroltoolsTransfermovementdatafromreferencenetworktocurrentnetwork()
{

	MapSignalDataAcrossProjects();
}

void CTLiteDoc::OnDemandtoolsGenerateinput()
{

	typedef struct  
	{
		int vehicle_id;
		int from_zone_id;
		int to_zone_id;
		float departure_time;
		float arrival_time;
		int complete_flag;
		float trip_time;
		int demand_type;
		int pricing_type;
		int vehicle_type;
		int information_type;
		float value_of_time;
		float toll_cost_in_dollar;
		float emissions;
		float distance_in_mile;
		int number_of_nodes;
		float Energy;
		float CO2;
		float NOX;
		float CO;
		float HC;

		int age;
		int version_no;

		int reserverd_field1;
		float reserverd_field2;
		int reserverd_field3;

	} struct_VehicleInfo_Header;

	FILE* st_struct = NULL;

	fopen_s(&st_struct,m_ProjectDirectory+"input_agent.bin","wb");


	typedef  struct  
	{
		int NodeName;
		float AbsArrivalTimeOnDSN;
	} struct_Vehicle_Node;

	if(st_struct!=NULL)
	{
		std::list<DTAVehicle*>::iterator iVehicle;

		//   int vehicle_id=0;//index for every vehicle 

		for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++)
		{

			DTAVehicle* pVehicle = (*iVehicle);

			if(pVehicle->m_NodeSize >= 2)  // with physical path in the network
			{
				int UpstreamNodeID = 0;
				int DownstreamNodeID = 0;

				float TripTime = 0;

				if(pVehicle->m_bComplete)
					TripTime = pVehicle->m_ArrivalTime-pVehicle->m_DepartureTime;


				float m_gap = 0;

				struct_VehicleInfo_Header header;
				header.vehicle_id = pVehicle->m_VehicleID-1; //start from 0
				header.from_zone_id = pVehicle->m_OriginZoneID;
				header. to_zone_id = pVehicle->m_DestinationZoneID;
				header. departure_time = pVehicle->m_DepartureTime;
				header. arrival_time = pVehicle->m_ArrivalTime;
				header. complete_flag = pVehicle->m_bComplete;
				header. trip_time = TripTime;
				header. demand_type = pVehicle->m_DemandType;
				header. pricing_type = pVehicle->m_PricingType;
				header. vehicle_type =pVehicle->m_VehicleType;
				header. information_type = pVehicle->m_InformationClass;
				header. value_of_time =pVehicle->m_VOT;
				header. toll_cost_in_dollar = pVehicle->m_TollDollarCost;
				header. distance_in_mile = pVehicle->m_Distance;
				header. number_of_nodes = pVehicle->m_NodeSize;

				struct_VehicleInfo_Header InfoHeaderAsVehicleInput;
				InfoHeaderAsVehicleInput.vehicle_id = pVehicle->m_VehicleID;
				InfoHeaderAsVehicleInput.from_zone_id = pVehicle->m_OriginZoneID;
				InfoHeaderAsVehicleInput. to_zone_id = pVehicle->m_DestinationZoneID;
				InfoHeaderAsVehicleInput. departure_time = pVehicle->m_DepartureTime;
				InfoHeaderAsVehicleInput. demand_type = pVehicle->m_DemandType;
				InfoHeaderAsVehicleInput. pricing_type = pVehicle->m_PricingType;
				InfoHeaderAsVehicleInput. vehicle_type =pVehicle->m_VehicleType;
				InfoHeaderAsVehicleInput. information_type = pVehicle->m_InformationClass;
				InfoHeaderAsVehicleInput. value_of_time =pVehicle->m_VOT;

				fwrite(&header, sizeof(struct_VehicleInfo_Header), 1, st_struct);


				int j = 0;


				struct_Vehicle_Node node_element;

				float LinkWaitingTime = 0;
				for(j = 1; j< pVehicle->m_NodeSize; j++)  // for all nodes
				{
					int LinkID = pVehicle->m_NodeAry [j].LinkNo;

					node_element. NodeName = m_LinkNoMap[LinkID]->m_FromNodeNumber ;
					node_element. AbsArrivalTimeOnDSN = 0;
					fwrite(&node_element, sizeof(node_element), 1, st_struct);

					if(j== pVehicle->m_NodeSize-1)
					{
						node_element. NodeName = m_LinkNoMap[LinkID]->m_ToNodeNumber ;
						node_element. AbsArrivalTimeOnDSN = 0;
						fwrite(&node_element, sizeof(node_element), 1, st_struct);
					}

				} //for all nodes in path

			}
		} // for all paths

		// not loaded in simulation


		fclose(st_struct);
	}

}


bool CTLiteDoc::ReadAimCSVFiles(LPCTSTR lpszFileName, int date_id)
{


	//   cout << "Read vehicle file... "  << endl;
	// vehicle_id,  origin_zone_id, destination_zone_id, departure_time,
	//	arrival_time, complete_flag, trip_time, demand_type, occupancy, information_type,
	//	value_of_time, path_min_cost,distance_in_mile, number_of_nodes,
	//	node id, node arrival time

	// step 1: read zone mapping files

	CString error_message_list;

	if(AfxMessageBox("Do you want to load Aimsun trajector and trip files?", MB_YESNO) == IDNO)
		return false;

	// coordinate conversion

	double m_XScale = 1;
	double m_YScale = 1;

	double Original_Point1_x=990312.078783;
	double Original_Point1_y=215179.584433;
	double Original_Point2_x=997335.415388;
	double Original_Point2_y=219143.052253;
	double Changed_Point1_x=-73.978119;
	double Changed_Point1_y=40.757293;
	double Changed_Point2_x=-73.95276;
	double Changed_Point2_y=40.768164;




	m_XScale = (Changed_Point2_x - Changed_Point1_x)/(Original_Point2_x - Original_Point1_x);
	m_YScale = (Changed_Point2_y - Changed_Point1_y)/(Original_Point2_y - Original_Point1_y);

	double m_XOrigin = Original_Point1_x - Changed_Point1_x /m_XScale;

	double m_YOrigin =  Original_Point1_y - Changed_Point1_y /m_YScale;



	std::map<int,int> AimsunZone2TAZMapping;

	CString taz_mapping_file = m_ProjectDirectory + "input_microsimulation_zone_to_TAZ_mapping.csv";
	CCSVParser parser;
	int i= 0;
	if (parser.OpenCSVFile(CString2StdString(taz_mapping_file)))
	{

		while(parser.ReadRecord())
		{

			int AimsunZone;
			if(parser.GetValueByFieldName("AimsunZone",AimsunZone) == false)
				break;
			int TAZ ;
			if(parser.GetValueByFieldName("TAZ",TAZ) == false)
				break;

			AimsunZone2TAZMapping[AimsunZone]= TAZ;

		}
	}else
	{
		CString msg;
		msg.Format ("Please check if file %s exists.", taz_mapping_file);  // +2 for the first field name line
		AfxMessageBox(msg);
		return false;

	}

	// step 2: read trip file

	CString trip_file = m_ProjectDirectory + "input_microsimulation_trip_data.csv";

	class SimpleVehicleData
	{
	public:
		long oid;
		int TAZ_origin;
		int TAZ_destination;
		float departure_time_in_min;
		float travel_time_in_min;
		float arrival_time_in_min;
	};

	std::map<long, SimpleVehicleData> SimpleVehicleDataMap;
	FILE* st = NULL;
	fopen_s(&st,trip_file,"r");

	long lineno = 1;
	int error_log_count = 0;
	std::map<int, int > missing_vehicle_id_map;
	if(st!=NULL)
	{
		char  str_line[2000]; // input string
		int str_line_size;
		g_read_a_line(st,str_line, str_line_size); //  skip the first line


		while(!feof(st))
		{

			int oid = -1;
			int origin= -1;
			int destination = -1;
			float entranceTime = 0;
			float exitTime = 0;
			float travelTime = 0;
			float delayTime = 0;

			fscanf(st,"%d,%d,%d,%f,%f,%f,%f\n", &oid, &origin, &destination, &entranceTime, &exitTime, &travelTime, &delayTime);

			if(oid<=0)  // invalid record;
				break;

			SimpleVehicleData element;
			element.oid = oid; 
		element.departure_time_in_min = entranceTime/60;
		element.travel_time_in_min = travelTime/60;
		element.arrival_time_in_min = exitTime/60;

			if(AimsunZone2TAZMapping.find(origin)!= AimsunZone2TAZMapping.end())
			{
				element.TAZ_origin = AimsunZone2TAZMapping[origin];
			}else
			{

				CString error_message;
				error_message.Format("Error: origin = %d does not have a TAZ mapping record. Line %d in file %s.",origin, lineno,  trip_file);

				AfxMessageBox(error_message);

				fclose(st);
				return false;
			}
			if(AimsunZone2TAZMapping.find(destination)!= AimsunZone2TAZMapping.end())
			{
				element.TAZ_destination  = AimsunZone2TAZMapping[destination];
			}else
			{

				CString error_message;
				error_message.Format("Error: destination = %d does not have a TAZ mapping record. Line %d in file %s.",origin, lineno,  trip_file);

				AfxMessageBox(error_message);

				fclose(st);
				return false;
			}

			SimpleVehicleDataMap[oid] = element;


			if(lineno%100 == 0 && lineno >=1)
			{
				CString str;
				str.Format ("Loading %d Aimsum trip records...", lineno);
				SetStatusText(str);
			}
			lineno++;
		}

	}else
	{
		CString msg;
		msg.Format ("Please check if file %s exists.", taz_mapping_file);  // +2 for the first field name line
		AfxMessageBox(msg);
		return false;

	}

	std::string trajectory_file = m_ProjectDirectory + "input_microsimulation_trajectory.csv";

	typedef struct 
	{
		int oid;
		int sectionId;
		float xCoord;
		float yCoord;
		float timeSta;
	} section_struc;

	std::vector<section_struc> section_vector; 
	std::vector<section_struc> valid_section_vector; 
	std::vector<section_struc> location_vector; 

	CString LogFile_str = m_ProjectDirectory+"microsimulation_data_error_log.csv";
	FILE * st_log;
	st_log = fopen(LogFile_str,"w");

	CString LogFile_str2 = m_ProjectDirectory+"microsimulation_data_log.csv";
	FILE * st_log2;
	st_log2 = fopen(LogFile_str2,"w");

	if(st_log2!=NULL)
	{
		fprintf(st_log2, "vehicle id, origin TAZ, destination TAZ, departure time in min, hour, section size, valid section size, diff,last section id\n");			
	}

	fopen_s(&st,lpszFileName,"r");

	lineno = 1;
	int valid_vehicle_count = 0;
	if(st!=NULL)
	{
		char  str_line[2000]; // input string
		int str_line_size;
		g_read_a_line(st,str_line, str_line_size); //  skip the first line

		int last_valid_oid= 0;
		int last_valid_sectionId= 0;


		while(!feof(st))
		{

			long oid= 0;
			long sectionId = 0;
			float xCoord = 0;
			float yCoord = 0;
			float timeSta = 0;

			section_struc element;

			//oid = g_read_integer(st);
			//if(oid<=0)
			//	break;

			//sectionId= g_read_integer(st);
			//if(sectionId<=0)
			//	break;
			//xCoord = g_read_float(st);
			//yCoord = g_read_float(st);

			//timeSta = g_read_float(st);

			fscanf(st,"%d,%d,%f,%f,%f\n", &oid, &sectionId, &xCoord, &yCoord, &timeSta);

			if(oid == 30026)
			{
				TRACE("section id: %d\n", sectionId);
			}


			element.oid = oid;
			element.sectionId = sectionId;
			element.xCoord = xCoord;
			element.yCoord = yCoord;
			element.timeSta = timeSta/60;


			if(last_valid_oid==0)  //initial condition 
			{
				last_valid_oid = oid;

			}

			element.oid = oid;
			element.sectionId = sectionId;
			element.xCoord = xCoord;
			element.yCoord = yCoord;

			if(oid!=last_valid_oid)  //finish all records
			{
				//add section id sequence 

				if( SimpleVehicleDataMap.find(last_valid_oid)  != SimpleVehicleDataMap.end())
				{	
					if(section_vector.size()>=2)  // find vehicle id
					{

						valid_section_vector.clear();

						for(unsigned k = 0; k< section_vector.size(); k++)
						{
							DTALink* pLink = FindLinkWithLinkID(section_vector[k].sectionId);
							if(pLink!=NULL)
							{
								valid_section_vector.push_back(section_vector[k]);
							}

						}


						if(valid_section_vector.size() >=2)  // at least two valid sections
						{

							if(last_valid_oid==2)
								TRACE("oid = 2!");

							DTAVehicle* pVehicle = new DTAVehicle;
							//			pVehicle->m_bGPSVehicle = true;
							pVehicle->m_DateID 		= 0;

							SimpleVehicleData se =  SimpleVehicleDataMap[last_valid_oid];

							pVehicle->m_VehicleID	= se.oid;
							pVehicle->m_OriginZoneID	= se.TAZ_origin;
							pVehicle->m_DestinationZoneID= se.TAZ_destination;

							pVehicle->m_DepartureTime	=  se.departure_time_in_min;
							pVehicle->m_ArrivalTime =  se.arrival_time_in_min;
							pVehicle->m_TripTime  =  se.travel_time_in_min;

							if(g_Simulation_Time_Horizon < pVehicle->m_ArrivalTime)
								g_Simulation_Time_Horizon = pVehicle->m_ArrivalTime;

							pVehicle->m_bComplete = true;
							pVehicle->m_DemandType = 1;
							pVehicle->m_PricingType = 1;
							pVehicle->m_VOT = 10;
							pVehicle->m_TollDollarCost = 0;
							pVehicle->m_Emissions = 0;

							pVehicle->m_VehicleLocationSize	= 0;

							pVehicle->m_NodeSize = valid_section_vector.size()+1;
							pVehicle->m_NodeAry = new SVehicleLink[pVehicle->m_NodeSize];
							pVehicle->m_NodeNumberSum = 0;


							pVehicle->m_NodeAry[0].ArrivalTimeOnDSN = valid_section_vector[0].timeSta;
							for(int i = 1; i< valid_section_vector.size(); i++)
							{
								DTALink* pLink = FindLinkWithLinkID(valid_section_vector[i-1].sectionId);
								if(pLink!=NULL)
								{
									pVehicle->m_NodeAry[i-1].LinkNo  = pLink->m_LinkNo ;
									pLink->m_total_link_volume +=1;

									pVehicle->m_NodeAry[i].ArrivalTimeOnDSN = valid_section_vector[i].timeSta;
									pVehicle->m_NodeNumberSum += pLink->m_FromNodeNumber ;

									pVehicle->m_Distance += pLink->m_Length ;

								}
							}

							DTALink* pLastLink = FindLinkWithLinkID(valid_section_vector[valid_section_vector.size()-1].sectionId);
							if(pLastLink!=NULL)
							{
								pVehicle->m_NodeAry[valid_section_vector.size()].LinkNo  = pLastLink->m_LinkNo ;
								pVehicle->m_NodeNumberSum += pLastLink->m_FromNodeNumber ;
								pVehicle->m_Distance += pLastLink->m_Length ;

							}

							pVehicle->m_VehicleLocationSize = 0;

							bool load_location_data = false;

							if(load_location_data)
							{

								pVehicle->m_VehicleLocationSize = location_vector.size();
								pVehicle->m_LocationRecordAry = new VehicleLocationRecord[location_vector.size()];


								for(int i=0; i< pVehicle->m_VehicleLocationSize; i++)
								{
									VehicleLocationRecord location_element;

									location_element.x = (location_vector[i].xCoord - m_XOrigin)*m_XScale;
									location_element.y = (location_vector[i].yCoord - m_YOrigin)*m_YScale;
									location_element.time_stamp_in_min = location_vector[i].timeSta;
									location_element.distance_in_km = 0.0001;
									location_element.linkid = location_vector[i].sectionId;

									pVehicle->m_LocationRecordAry[i] = location_element;
								}


							}
							m_VehicleSet.push_back (pVehicle);
							m_VehicleIDMap[pVehicle->m_VehicleID]  = pVehicle;
							valid_vehicle_count++;

							if(st_log2!=NULL)
							{

							fprintf(st_log2, "%d,%d,%d,%.2f,%.2f,%d,%d,%d,%d\n", pVehicle->m_VehicleID , pVehicle->m_OriginZoneID , pVehicle->m_DestinationZoneID , pVehicle->m_DepartureTime,pVehicle->m_DepartureTime/60,section_vector.size(), valid_section_vector.size(),section_vector.size()- valid_section_vector.size(),last_valid_sectionId );
							}

						}else
						{  // output vehicle data with invalid section ids
							if(st_log!=NULL)
							{
								fprintf(st_log, "veh %d,# of valid sections <=1 \n",last_valid_oid);
							}
						}

						last_valid_oid = oid;
						section_vector.clear();
						section_vector.push_back (element);  // first section of the next vehicle
						location_vector.clear();
						location_vector.push_back (element);  // first section of the next vehicle

					}else
					{
						if(st_log!=NULL && error_log_count<1000)
						{
								fprintf(st_log, "veh %d,# of valid sections <=1 \n",last_valid_oid);
						}			
					}
				}else
				{
					if(st_log!=NULL && error_log_count<1000)
					{  

						if(missing_vehicle_id_map.find(last_valid_oid)==missing_vehicle_id_map.end())
						{
						fprintf(st_log, "missing vehicle id %d, which has not been defined in trip file\n", last_valid_oid );
						missing_vehicle_id_map[last_valid_oid] =1;
						error_log_count++;
						}

						last_valid_oid = 0; // reset vehicle id to 0, so the remaining vehicles can be read
					}
				}

			}else if(last_valid_sectionId!= sectionId) 
			{
				last_valid_sectionId = sectionId;
				section_vector.push_back (element);
			}

			location_vector.push_back(element);

			if(valid_vehicle_count%1000 == 0 && valid_vehicle_count >=1)
			{
				CString str;
				str.Format ("Loading %d K trip trajectories...", valid_vehicle_count);
				SetStatusText(str);
			}

			lineno++;
		}

		m_SimulationVehicleDataLoadingStatus.Format ("%d trips are loaded from trajector files...", valid_vehicle_count);

		AfxMessageBox(m_SimulationVehicleDataLoadingStatus, MB_ICONINFORMATION );

		//if(error_message_list.GetLength ()>=1)
		//	AfxMessageBox(error_message_list);

		std::list<DTAVehicle*>::iterator iVehicle;

	for (iVehicle = m_VehicleSet.begin(); iVehicle != m_VehicleSet.end(); iVehicle++)
	{
		DTAVehicle* pVehicle = (*iVehicle);

		if(pVehicle->m_bComplete == false && pVehicle->m_NodeSize ==0)
		{
				if(st_log!=NULL)
				{
				fprintf(st_log, "%d,%.2f,%.2f do not have valid section records\n", pVehicle->m_VehicleID ,  pVehicle->m_DepartureTime,pVehicle->m_DepartureTime/60);
				}			
		
		}
	}

	}
	else
	{
		CString msg;
		msg.Format("File %s cannot be opened.", trajectory_file.c_str());
		AfxMessageBox(msg);
		return false;
	}

	if(st_log!=NULL)
		fclose(st_log);

	if(st_log2!=NULL)
		fclose(st_log2);


	if(m_VehicleSet.size() >=1 ) //with GPS data
	{
		CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();
		pMainFrame->m_bShowLayerMap[layer_vehicle_position]  = true;
		//			m_bGPSDataSet  = true;
	}

	return true;


}


void CTLiteDoc::OnDemandReconstructlinkmoeth()
{
	if(AfxMessageBox("Do you want to use vehicle trajectory data to reconstruct link MOEs(inflow, outflow counts, density)?", MB_YESNO) == IDYES)
	{
		CWaitCursor wait;
		RecalculateLinkMOEFromVehicleTrajectoryFile();
	}

}

void CTLiteDoc::OnNetworktoolsResetlinklength()
{
	if(AfxMessageBox("Do you want to use long/lat coordinate data in the link shape file to recalculate the link length?", MB_YESNO) == IDYES)
	{
		CWaitCursor wait;

		double total_length = 0;

		double default_coordinate_distance_sum = 0;

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			(*iLink)->m_Length  = 0;

			
		for(unsigned int si = 0; si < (*iLink) ->m_ShapePoints .size()-1; si++)
		{
			double distance  =  g_CalculateP2PDistanceInMileFromLatitudeLongitude((*iLink)->m_ShapePoints[si] , (*iLink)->m_ShapePoints[si+1]);
			(*iLink)->m_Length += distance;
		}

		total_length+= (*iLink)->m_Length;

		default_coordinate_distance_sum+= (*iLink)->DefaultDistance();


		}

		if(total_length>0.000001f)
		{
			m_UnitMile= default_coordinate_distance_sum / total_length ;
			m_UnitFeet = m_UnitMile/5280.0f;  

			m_bUnitMileInitialized = true;  // the unit mile has been reset according to the actual distance 
		}

	
		CString str;
		str.Format("The updated average link length per link is now %.3f.\nPlease save the project to confirm the change.", total_length/max(1,m_LinkSet.size() ));
		AfxMessageBox(str,MB_ICONINFORMATION);

	}
}

void CTLiteDoc::UpdateUnitMile()
{
		CWaitCursor wait;

		double total_length_in_mile = 0;

		double default_coordinate_distance_sum = 0;

		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			
		total_length_in_mile+= (*iLink)->m_Length;

		default_coordinate_distance_sum+= (*iLink)->DefaultDistance();


		}

		if(total_length_in_mile>0.000001f)
		{
			m_UnitMile= default_coordinate_distance_sum / total_length_in_mile ;
			m_UnitFeet = m_UnitMile/5280.0f;  

			m_bUnitMileInitialized = true;  // the unit mile has been reset according to the actual distance 
		}


}

void CTLiteDoc::OnSubareaCreatezonefromsubarea()
{
	int zone_number = -1;  // starting default number 


	if(m_SubareaNodeSet.size()>0 )
	{
	
		DTANode* pNode = m_SubareaNodeSet[0];



		if(m_ZoneMap.find(pNode->m_NodeNumber)==m_ZoneMap.end())
		{
		zone_number = pNode->m_NodeNumber ;

		m_ZoneMap [zone_number].m_ZoneID = zone_number;
		m_ZoneMap [zone_number].SetNodeActivityMode(pNode->m_NodeNumber,0);

		}else
		{  // Zone has been defined. we can just update zone boundary

			// this node has a zone Id already, we just update their zone boundary 
			if(m_ZoneMap.find(pNode->m_ZoneID)!=m_ZoneMap.end())
			{

				zone_number = pNode->m_ZoneID;
				m_ZoneMap [zone_number].m_ShapePoints.clear ();

				for (unsigned int sub_i= 0; sub_i < m_SubareaShapePoints.size(); sub_i++)
		{
				m_ZoneMap [zone_number].m_ShapePoints .push_back (m_SubareaShapePoints[sub_i]);

		}

			

			}
	
		
			return;
		}

	}
	if(zone_number==-1)
	{ // find largest zone number for the new zone
	

		std::map<int, DTAZone>	:: const_iterator itr;

		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)
		{
			if( zone_number <= itr->first)
				zone_number = itr->first +1;
				
		}
		m_ZoneMap [zone_number].m_ZoneID = max(1,zone_number);

	}


			for (unsigned int sub_i= 0; sub_i < m_SubareaShapePoints.size(); sub_i++)
		{
				m_ZoneMap [zone_number].m_ShapePoints .push_back (m_SubareaShapePoints[sub_i]);

		}
	

			// add activity locations if there is no activity location being assigned yet
		RegenerateactivitylocationsForEmptyZone(zone_number);

		m_SubareaShapePoints.clear();
		UpdateAllViews(0);
}


bool CTLiteDoc::FindObject(eSEARCHMODE SearchMode, int value1, int value2)
{

		if(SearchMode == efind_link)
		{

			DTALink* pLink = FindLinkWithNodeNumbers(value1 ,value2 );

			if(pLink !=NULL)
			{
				m_SelectedLinkNo = pLink->m_LinkNo ;
				m_SelectedNodeID = -1;

				ZoomToSelectedLink(pLink->m_LinkNo);


			}
		}

		if(SearchMode == efind_node)
		{
			DTANode* pNode = FindNodeWithNodeNumber (value1);
			if(pNode !=NULL)
			{
				m_SelectedLinkNo = -1;
				m_SelectedNodeID = pNode->m_NodeID ;
				
				ZoomToSelectedNode(value1);

			} return false;
		}

		if(SearchMode == efind_path)
		{

			DTANode* pFromNode = FindNodeWithNodeNumber (value1);
			if(pFromNode ==NULL)
			{
				CString str;
				str.Format ("From Node %d cannot be found.",value1);
				AfxMessageBox(str);
				return false; 
			}else
			{
				m_OriginNodeID = pFromNode->m_NodeID;

			}
			DTANode* pToNode = FindNodeWithNodeNumber (value2);
			if(pToNode ==NULL)
			{
				CString str;
				str.Format ("To Node %d cannot be found.",value2);
				AfxMessageBox(str);
				return false;
			}else
			{
				m_DestinationNodeID = pToNode->m_NodeID;
			}

			Routing(false);

		}

		if(SearchMode == efind_vehicle)
		{
			int SelectedVehicleID = value1; // internal vehicle index starts from zero
			std::vector<int> LinkVector;

			if(m_VehicleIDMap.find(SelectedVehicleID) == m_VehicleIDMap.end())
			{
				if(SelectedVehicleID>=0)
				{
					CString str_message;
					str_message.Format ("Vehicle Id %d cannot be found.", SelectedVehicleID+1);
					AfxMessageBox(str_message);
				}

				m_SelectedVehicleID = -1;
				HighlightPath(LinkVector,1);

				return true;
			}

			m_SelectedVehicleID = SelectedVehicleID;

			DTAVehicle* pVehicle = m_VehicleIDMap[SelectedVehicleID];

			CPlayerSeekBar m_wndPlayerSeekBar;


			// set departure time to the current time of display
			g_Simulation_Time_Stamp = pVehicle->m_DepartureTime +1;

			CMainFrame* pMainFrame = (CMainFrame*) AfxGetMainWnd();

			pMainFrame->m_wndPlayerSeekBar.SetPos(g_Simulation_Time_Stamp);

			m_LinkMOEMode = MOE_vehicle;

			for(int link= 1; link<pVehicle->m_NodeSize; link++)
			{
				LinkVector.push_back (pVehicle->m_NodeAry[link].LinkNo);
			}

			HighlightPath(LinkVector,1);

			m_HighlightGDPointVector.clear();

			m_HighlightGDPointVector = pVehicle->m_GPSLocationVector ; // assign the GPS points to be highlighted

		}
	UpdateAllViews(0);

	return true;
}

void CTLiteDoc::RegenerateactivitylocationsForEmptyZone(int zoneid)
{
	if(m_ZoneMap[zoneid].m_ActivityLocationVector.size()<0)
		return;

		// scan all nodes 
		std::list<DTANode*>::iterator iNode;
		for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{
		
		DTANode* pNode = (*iNode);
		int nodeid = pNode->m_NodeID ;

		// three conditions: not pretimed signal, not actuated signal, not connected to freeway and ramp
		if(pNode->m_ControlType != m_ControlType_PretimedSignal && pNode->m_ControlType != m_ControlType_ActuatedSignal && pNode->m_bConnectedToFreewayORRamp == false)
		{

			if(m_ZoneMap[zoneid].IsInside (pNode->pt))
			{
		
			DTAActivityLocation element;
			element.ZoneID  = zoneid;
			element.NodeNumber = pNode->m_NodeNumber ;
			element.pt = pNode ->pt;

			int External_OD_Flag  = 0;

			m_NodeIDtoZoneNameMap[nodeid] = zoneid;
			m_NodeIDMap [nodeid ] -> m_ZoneID = zoneid;
			m_NodeIDMap [nodeid] ->m_External_OD_flag = 0;

			m_ZoneMap[zoneid].m_ActivityLocationVector.push_back (element);

			}
		

		} 


		} // for each node
}
void CTLiteDoc::OnDemandRegenerateactivitylocations()
{
	CWaitCursor wait;

	if(m_ZoneMap.size()>=100)
	{
		if(AfxMessageBox("There are more than 100 zones, so reidentifing all activity locations might take a while.\nDo you wan to continue?", MB_YESNO|MB_ICONINFORMATION)==IDNO)
		return;
	
	}

	int activity_node_count = 0; 
	CString message_vector;

	std::vector<int> zones_without_activity_nodes;

	//reset activity flag
	std::list<DTANode*>::iterator iNode;
	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
	{
	DTANode* pNode = (*iNode);
	pNode->m_bZoneActivityLocationFlag  = false;

	pNode->m_ZoneID = 0;
	}
		std::map<int, DTAZone>	:: iterator itr;

		for(itr = m_ZoneMap.begin(); itr != m_ZoneMap.end(); itr++)
		{
		(*itr).second .m_ActivityLocationVector .clear();

		int zoneid = (*itr).first;

		RegenerateactivitylocationsForEmptyZone(zoneid);

	
			activity_node_count+=(*itr).second.m_ActivityLocationVector.size();

		if((*itr).second.m_ActivityLocationVector.size() ==0)
		{
			zones_without_activity_nodes.push_back (zoneid);


		}

	}  // for each zone


		if(zones_without_activity_nodes.size()>0)
		{
			message_vector = "The following zones do not have any activity location/node being assigned.\n";

			for(unsigned int i= 0; i < zones_without_activity_nodes.size(); i++)
			{
				CString str;
				str.Format("%d,",zones_without_activity_nodes[i]);
				message_vector+= str;
			
			}

			AfxMessageBox(message_vector);

		}else
		{
		 
				CString str;
				str.Format("%d nodes have been assigned as activcity locations.",activity_node_count);

				AfxMessageBox(str,MB_ICONINFORMATION);

		}

}


void CTLiteDoc::OnTrafficcontroltoolsTransfersignaldatafromreferencenetworktocurrentnetwork()
{
	// step 0: make sure two networks have been opened

	CTLiteDoc* pReferenceDoc = NULL;
		std::list<CTLiteDoc*>::iterator iDoc = g_DocumentList.begin ();
	int project_index = 0 ;
	while (iDoc != g_DocumentList.end())
	{
		if((*iDoc)->m_NodeSet.size() >0 && (*iDoc)!=this)  
		{

		pReferenceDoc = (*iDoc);

		project_index++;
		}
		iDoc++;
	}

	if(pReferenceDoc == NULL)
	{
	
		AfxMessageBox("Please open the reference network data set in anoter window.");
		return;
	}
	
	int count_node = 0 ;
	int count_link = 0 ;
	std::list<DTANode*>::iterator  iNode;


	FILE* st = NULL;

	CString str_log_file;
	str_log_file.Format ("%s\\signal_data_reading_log.csv",m_ProjectDirectory);
	
	fopen_s(&st,str_log_file,"w");


	if( st !=NULL)
	{

	
//	fprintf(st,"intersection_name1,intersection_name2, baseline_node_id,reference_node_id,base_line_cycle_length_in_second,reference_cycle_length_in_second\n");
		

		fprintf(st,"------\n");
		fprintf(st,"Step 2: Read Cycle length and offset,\n");

	for (iNode = m_NodeSet.begin(); iNode != m_NodeSet.end(); iNode++)
		{

			int baseline_node_id = (*iNode)->m_NodeNumber  ;
				if((*iNode)->m_ControlType == m_ControlType_PretimedSignal || (*iNode)->m_ControlType == m_ControlType_ActuatedSignal)
				{
					
					DTANode* pRefNode = NULL;
					int reference_node_number =  (*iNode)->m_NodeNumber;


					if( pReferenceDoc->m_NodeNumbertoIDMap.find(reference_node_number) !=  pReferenceDoc->m_NodeNumbertoIDMap.end())						
					{
						pRefNode = pReferenceDoc->m_NodeNumberMap[reference_node_number];

						if(pRefNode->m_ControlType == pReferenceDoc->m_ControlType_PretimedSignal || pRefNode->m_ControlType == pReferenceDoc->m_ControlType_ActuatedSignal)
						{

						fprintf(st,"Node,%d,obtains cycle length =,%d,updated cycle length,%d,\n", 
							reference_node_number,
							(*iNode)->m_CycleLengthInSecond,
							pRefNode ->m_CycleLengthInSecond );

						(*iNode)->m_CycleLengthInSecond = pRefNode ->m_CycleLengthInSecond ;
						(*iNode)->m_SignalOffsetInSecond = pRefNode ->m_SignalOffsetInSecond  ;
						count_node++;

						}
					}
					
				}
			
		}

	// step 2: fetch data for all links
		for (std::list<DTALink*>::iterator iLink = m_LinkSet.begin(); iLink != m_LinkSet.end(); iLink++)
		{
			DTALink* pThisLink = (*iLink);
			DTALink* pReferenceLink =  pReferenceDoc->FindLinkWithNodeNumbers (pThisLink->m_FromNodeNumber ,pThisLink->m_ToNodeNumber );

			DTANode* pDownstreamNode = m_NodeNumberMap[pThisLink->m_ToNodeNumber ];

			if(pReferenceLink!=NULL && 
				(pDownstreamNode->m_ControlType == m_ControlType_PretimedSignal || pDownstreamNode->m_ControlType == m_ControlType_ActuatedSignal))
			{
				DTANode* pRefNode = pReferenceDoc->m_NodeNumberMap[pThisLink->m_ToNodeNumber ];

				if(pRefNode->m_ControlType == pReferenceDoc->m_ControlType_PretimedSignal || pRefNode->m_ControlType == pReferenceDoc->m_ControlType_ActuatedSignal)
			{

				fprintf(st,"Link,%d->%d, Old Effective green,%d, Updated Effective green,%d, Old # of left turn lanes,%d, Updated of left turn lanes,%d,Old # of right turn lanes,%d, Updated of right turn lanes,%d\n", 
						pThisLink->m_FromNodeNumber,pThisLink->m_ToNodeNumber,
						pThisLink->m_EffectiveGreenTimeInSecond,
						pReferenceLink->m_EffectiveGreenTimeInSecond,
						pThisLink->m_NumberOfLeftTurnLanes,
						 pReferenceLink->m_NumberOfLeftTurnLanes,
						 pThisLink->m_NumberOfRightTurnLanes ,
						  pReferenceLink->m_NumberOfRightTurnLanes
						);

					pThisLink->m_EffectiveGreenTimeInSecond = pReferenceLink->m_EffectiveGreenTimeInSecond;
					pThisLink->m_GreenStartTimetInSecond  = pReferenceLink->m_GreenStartTimetInSecond;
					pThisLink->m_NumberOfLeftTurnLanes   = pReferenceLink->m_NumberOfLeftTurnLanes;
					pThisLink->m_NumberOfRightTurnLanes    = pReferenceLink->m_NumberOfRightTurnLanes;
					
				
			count_link++;
			}
		}
		}

	
		fclose(st);
	}else
	{
		CString str;
		str.Format("File %s cannot be opened.", str_log_file);
		AfxMessageBox(str);
	}
	
	
	OpenCSVFileInExcel(str_log_file);

	CString str;
	str.Format("%d nodes and %d links have obtained cycle length and signal timing from the reference network. To accept the changes, please save the network.",count_node, count_link);
	AfxMessageBox(str, MB_ICONINFORMATION);

}
