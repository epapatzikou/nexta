
// Becasese the functions below might relate to file interfaces with other proprietary software packages, no copyright or GPL statement is made here.

// Utility.cpp : Utility functions used for reading and outputing
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
#include "math.h"
#include "Network.h"
#include "Utility.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
void g_ProgramStop()
{
	getchar();
	exit(0);
};

float g_P2P_Distance(GDPoint p1, GDPoint p2)
{
return pow(((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y)),0.5f);
}

float g_DistancePointLine(GDPoint pt, GDPoint FromPt, GDPoint ToPt)
{
    float U;
    GDPoint Intersection;


    float  LineLength = g_P2P_Distance( FromPt, ToPt );
 
    U = (  (pt.x - ToPt.x) * (FromPt.x - ToPt.x ) + ( pt.y - ToPt.y ) * ( FromPt.y - ToPt.y ) ) /(LineLength * LineLength );
 
    if( U < 0.0f || U > 1.0f )
        return -1;   // intersection does not fall within the segment
 
    Intersection.x = ToPt.x + U * ( FromPt.x - ToPt.x );
    Intersection.y = ToPt.y + U * ( FromPt.y - ToPt.y );
    
    return g_P2P_Distance( pt, Intersection );
}
float g_GetRandomRatio()
{
	//		g_RandomSeed = (g_LCG_a * g_RandomSeed + g_LCG_c) % g_LCG_M;  //m_RandomSeed is automatically updated.

	//		return float(g_RandomSeed)/g_LCG_M;

	return float(rand())/RAND_MAX;
}


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

void ReadDSm_pNetworkData(char fname[_MAX_PATH])
{
/*   FILE* st = NULL;
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
   g_RTNodePredAry = Allocate3DDynamicArray<int>(g_InfoTypeSize,g_NodeSet.size(),g_NodeSet.size());
   g_RTLabelCostAry = Allocate3DDynamicArray<unsigned char>(g_InfoTypeSize,g_NodeSet.size(),g_NodeSet.size());

   if(g_RTNodePredAry==NULL || g_RTLabelCostAry ==NULL)
      {
      cout << "No sufficient memory..."<< endl;
      exit(1);
      }
   cout << "Number of Zones = "<< g_ODZoneSize  << endl;
   cout << "Number of Nodes = "<< g_NodeSet.size() << endl;
   cout << "Number of Links = "<< g_LinkSet.size() << endl;
*/
}

void ReadDSPDestinationData(char fname[_MAX_PATH])
{
/*   FILE* st = NULL;
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
*/
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
	   value = (float)(atof(lpbuffer)); 
   }

   if(value == def_value)  //  the parameter might not exist
   {
   sprintf_s(lpbuffer,"%5.2f",def_value);
   WritePrivateProfileString(section,key,lpbuffer,filename);
   }

	   return value; 
} 

double  g_FindClosestYResolution(double Value)
{
	 vector<double> ResolutionVector;

		 ResolutionVector.push_back(0.001);
		 ResolutionVector.push_back(0.005);
		 ResolutionVector.push_back(0.01);
		 ResolutionVector.push_back(0.05);
		 ResolutionVector.push_back(0.1);
		 ResolutionVector.push_back(0.2);
		 ResolutionVector.push_back(0.5);
		 ResolutionVector.push_back(1);
		 ResolutionVector.push_back(2);
		 ResolutionVector.push_back(5);
		 ResolutionVector.push_back(10);
		 ResolutionVector.push_back(20);
		 ResolutionVector.push_back(50);
		 ResolutionVector.push_back(100);
		 ResolutionVector.push_back(200);
		 ResolutionVector.push_back(500);
		 ResolutionVector.push_back(1000);
		 ResolutionVector.push_back(2000);
		 ResolutionVector.push_back(5000);
		 ResolutionVector.push_back(10000);
		 ResolutionVector.push_back(20000);
		 ResolutionVector.push_back(50000);
		 ResolutionVector.push_back(100000);
		 ResolutionVector.push_back(200000);
		 ResolutionVector.push_back(500000);
		 ResolutionVector.push_back(1000000);
		 
		double min_distance  = 99999999;

		double ClosestResolution=1;
		for(unsigned int i=0; i<ResolutionVector.size();i++)
		{
			if(	fabs(Value-ResolutionVector[i]) < min_distance)
			{
				min_distance = fabs(Value-ResolutionVector[i]);
				ClosestResolution = ResolutionVector[i];
			}
		}
		return ClosestResolution;
}

int  g_FindClosestTimeResolution(double Value)
{

		int ResolutionVector[9] = {1,10,30,60,120,240,480,720,1440};
		double min_distance  = 9999999;

		Value = max(1,Value/6);  //1/6 of time horizion as resolution

		int ClosestResolution=1;
		for(int i=0; i<9;i++)
		{
			if(	fabs(Value-ResolutionVector[i]) < min_distance)
			{
				min_distance = fabs(Value-ResolutionVector[i]);
				ClosestResolution = ResolutionVector[i];
			}
		}
		return ClosestResolution;
}