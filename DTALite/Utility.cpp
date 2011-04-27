
// Becasese the functions below might relate to file interfaces with other proprietary software packages, no copyright or GPL statement is made here.

// Utility.cpp : Utility functions used for reading and outputing

#include "stdafx.h"
#include "math.h"
#include "DTALite.h"
#include "GlobalData.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// polar form of the Box-Muller transformation to get two random numbers that follow a standard normal distribution 
float g_RNNOF()
{
    float x1, x2, w, y1, y2;
 
         do {
                 x1 = 2.0f * g_GetRandomRatio() - 1.0f;
                 x2 = 2.0f * g_GetRandomRatio()- 1.0f;
                 w = x1 * x1 + x2 * x2;
         } while ( w >= 1.0f );

         w = sqrt( (-2.0f * log( w ) ) / w );
         y1 = x1 * w;
         y2 = x2 * w;

	 return y1;  // we only use one random number
}


int g_read_integer_with_char_O(FILE* f)
   // read an integer from the current pointer of the file, skip all spaces, if read "O", return 0;
{
   char ch, buf[ 32 ];
   int i = 0;
   int flag = 1;
   /* returns -1 if end of file is reached */

   while(true)
      {
      ch = getc( f );
      if( ch == EOF ) return -1;
      if( ch == 'O' ) return 0;  // special handling

      if (isdigit(ch))
         break;
      if (ch == '-')
         flag = -1;
      else
         flag = 1;
      };
   if( ch == EOF ) return -1;
   while( isdigit( ch )) {
      buf[ i++ ] = ch;
      ch = fgetc( f );
      }
   buf[ i ] = 0;


   return atoi( buf ) * flag;

}

void g_ProgramStop()
{
	getchar();
	exit(0);
};

int g_read_integer(FILE* f)
   // read an integer from the current pointer of the file, skip all spaces
{
   char ch, buf[ 32 ];
   int i = 0;
   int flag = 1;
   /* returns -1 if end of file is reached */

   while(true)
      {
      ch = getc( f );
      if( ch == EOF ) return -1;
      if (isdigit(ch))
         break;
      if (ch == '-')
         flag = -1;
      else
         flag = 1;
      };
   if( ch == EOF ) return -1;
   while( isdigit( ch )) {
      buf[ i++ ] = ch;
      ch = fgetc( f );
      }
   buf[ i ] = 0;


   return atoi( buf ) * flag;

}

float g_read_float(FILE *f)
   //read a floating point number from the current pointer of the file,
   //skip all spaces

{
   char ch, buf[ 32 ];
   int i = 0;
   int flag = 1;

   /* returns -1 if end of file is reached */

   while(true)
      {
      ch = getc( f );
      if( ch == EOF ) return -1;
      if (isdigit(ch))
         break;

      if (ch == '-')
         flag = -1;
      else
         flag = 1;

      };
   if( ch == EOF ) return -1;
   while( isdigit( ch ) || ch == '.' ) {
      buf[ i++ ] = ch;
      ch = fgetc( f );

      }
   buf[ i ] = 0;

   /* atof function converts a character string (char *) into a doubleing
      pointer equivalent, and if the string is not a floting point number,
      a zero will be return.
      */

   return (float)(atof( buf ) * flag);

}

void ReadDSPNetworkData(char fname[_MAX_PATH])
{
   FILE* st = NULL;
   fopen_s(&st,fname,"r");
   if(st!=NULL)
      {
      g_ODZoneSize = g_read_integer(st);
      int num_nodes= g_read_integer(st);
      int num_links = g_read_integer(st);
      int KSPNumShortestPaths = g_read_integer(st);
      int UseSuperzonesFlag = g_read_integer(st);


      // Read node block
      int i;
      int id = 0, zoneNum = 0;
      DTANode* pNode = 0;
      for(i = 0; i < num_nodes; i++)
         {

         id			= g_read_integer(st);
         zoneNum	= g_read_integer(st);

         // Create and insert the node
         pNode = new DTANode;
         pNode->m_NodeID = i;
         pNode->m_ZoneID = 0;
         g_NodeSet.insert(pNode);
         g_NodeMap[id] = pNode;
         g_NodeIDtoNameMap[i] = id;
         }

      // Read link block
      DTALink* pLink = 0;
      for(i = 0; i < num_links; i++)
         {
         pLink = new DTALink(g_SimulationHorizon);
         pLink->m_LinkID = i;
         pLink->m_FromNodeNumber = g_read_integer(st);
         pLink->m_ToNodeNumber = g_read_integer(st);
         pLink->m_FromNodeID = g_NodeMap[pLink->m_FromNodeNumber ]->m_NodeID;
         pLink->m_ToNodeID= g_NodeMap[pLink->m_ToNodeNumber]->m_NodeID;

         int m_LeftBays= g_read_integer(st);
         int m_RightBays= g_read_integer(st);

         pLink->m_Length= g_read_float(st);
         pLink->m_NumLanes= g_read_integer(st);

         int m_FlowModel= g_read_integer(st);
         int m_SpeedAdjustment= g_read_integer(st);
         pLink->m_SpeedLimit= g_read_integer(st);
         pLink->m_MaximumServiceFlowRatePHPL= g_read_float(st);
         int m_SaturationFlowRate= g_read_integer(st);
         pLink->m_link_type= g_read_integer(st);
         int m_grade= g_read_integer(st);

         pLink->SetupMOE();
         g_LinkSet.insert(pLink);
         g_LinkMap[i]  = pLink;
         }

      fclose(st);
      }
   g_RTNodePredAry = Allocate3DDynamicArray<int>(g_InfoTypeSize,g_ODZoneSize+1,g_NodeSet.size()+g_ODZoneSize+1);
   g_RTLabelCostAry = Allocate3DDynamicArray<unsigned char>(g_InfoTypeSize,g_ODZoneSize+1,g_NodeSet.size()+g_ODZoneSize+1);

   if(g_RTNodePredAry==NULL || g_RTLabelCostAry ==NULL)
      {
      cout << "No sufficient memory..."<< endl;
      exit(1);
      }
   cout << "Number of Zones = "<< g_ODZoneSize  << endl;
   cout << "Number of Nodes = "<< g_NodeSet.size() << endl;
   cout << "Number of Links = "<< g_LinkSet.size() << endl;

}

void ReadDSPDestinationData(char fname[_MAX_PATH])
{
   FILE* st = NULL;
   fopen_s(&st,fname,"r");
   if(st!=NULL)
      {
      int i;
      int id = 0, zoneNum = 0;
      DTANode* pNode = 0;
      for(i = 0; i < g_ODZoneSize; i++)
         {

         zoneNum	= g_read_integer(st);
         int num_nodes= g_read_integer(st);

         if(num_nodes > g_AdjLinkSize)
            g_AdjLinkSize = num_nodes + 10;  // increaes buffer size


         for(int n = 0; n< num_nodes; n++)
            {
            int node_number= g_read_integer(st);


            g_NodeMap[node_number]->m_ZoneID = zoneNum;
            }
         }
      fclose(st);
      }

}

void ReadDSPVehicleData(char fname[_MAX_PATH])
{
   cout << "Read vehicle file... "  << endl;

   FILE* st = NULL;
   fopen_s(&st,fname,"r");
   if(st!=NULL)
      {
      int num_vehicles = g_read_integer(st);
      int num_stops    = g_read_integer(st);

      DTAVehicle* pVehicle = 0;
      for(int i = 0; i< num_vehicles; i++)
         {

         pVehicle = new DTAVehicle;
         pVehicle->m_VehicleID		= i;
//         pVehicle->m_VehicleName	= g_read_integer(st);
		 pVehicle->m_RandomSeed = pVehicle->m_VehicleID;

         int USN  = g_read_integer(st);
         int DSN  = g_read_integer(st);

         if(g_NodeMap[USN])
            int UpstreamNodeID	= g_NodeMap[USN]->m_NodeID;

         if(g_NodeMap[DSN])
            int DownstreamNodeID = g_NodeMap[DSN]->m_NodeID;

         pVehicle->m_DepartureTime	= g_read_float(st);


         pVehicle->m_VehicleType	= g_read_integer(st);
         pVehicle->m_VehicleType	= g_read_integer(st);
         pVehicle->m_Occupancy		= g_read_integer(st);
         int NodeSize		= g_read_integer(st);
         int NumberOfDestinations= g_read_integer(st);
         pVehicle->m_InformationClass		= g_read_integer(st);
         float ribf			= g_read_float(st);
         float comp			= g_read_float(st);
         pVehicle->m_OriginZoneID	= g_read_integer(st);

         for(int j=0; j<NumberOfDestinations;j++)
            {
            pVehicle->m_DestinationZoneID=g_read_integer(st);
            float waiting_time =  g_read_float(st);
            }

         pVehicle->m_NodeSize = 0;  // initialize NodeSize as o
         g_VehicleVector.push_back(pVehicle);
         g_VehicleMap[i]  = pVehicle;

         int AssignmentInterval = int(pVehicle->m_DepartureTime/g_DepartureTimetInterval);



         }

      fclose(st);
      cout << "Number of Vehicles = "<< g_VehicleVector.size() << endl;
      cout << "Demand Loading Horizon = "<< g_DemandLoadingHorizon << " min" << endl;
      cout << "Simulation Horizon = "<< g_SimulationHorizon << " min" << endl;
      cout << "Assignment Interval = "<< g_DepartureTimetInterval << " min" << endl;

      }
}

int g_GetPrivateProfileInt( LPCTSTR section, LPCTSTR key, int def_value, LPCTSTR filename) 
{
   char lpbuffer[64];
   int value = def_value;
   if(GetPrivateProfileString(section,key,"",lpbuffer,sizeof(lpbuffer),filename)) 
   {
	   value =  atoi(lpbuffer); 
   }

   if(value == def_value)  //  the parameter might not exist
   {
   sprintf_s(lpbuffer,"%d",def_value);
   WritePrivateProfileString(section,key,lpbuffer,filename);
   }
	   return value; 
}

float g_GetPrivateProfileFloat( LPCTSTR section, LPCTSTR key, float def_value, LPCTSTR filename) 
{ 
   char lpbuffer[64];
   float value = def_value;
   if(GetPrivateProfileString(section,key,"",lpbuffer,sizeof(lpbuffer),filename)) 
   {
	   value =  (float)(atof(lpbuffer)); 
   }

   if(value == def_value)  //  the parameter might not exist
   {
   sprintf_s(lpbuffer,"%5.2f",def_value);
   WritePrivateProfileString(section,key,lpbuffer,filename);
   }

	   return value; 
} 
