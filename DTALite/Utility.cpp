
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
extern CTime g_AppStartTime;
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


bool g_GetVehicleAttributes(int demand_type, int &VehicleType, int &PricingType, int &InformationClass, float &VOT)
{

	if(g_DemandTypeMap.find(demand_type) == g_DemandTypeMap.end())
	{
	cout << "Error: The demand file has demand_type = " << demand_type << ", which has not been defined in input_demand_type.csv."<< endl;
	g_ProgramStop();
	}

	float RandomPercentage= g_GetRandomRatio() * 100; 

	// step 1. vehicle type
	VehicleType = 1;
	// default to a single value
	int i;
	for(i= 1; i<= g_VehicleTypeVector.size(); i++)
	{
		if(RandomPercentage >= g_DemandTypeMap[demand_type].cumulative_type_percentage[i-1] &&  RandomPercentage < g_DemandTypeMap[demand_type].cumulative_type_percentage[i])
			VehicleType = i;
	}

	//step 2: pricing type
	PricingType = g_DemandTypeMap[demand_type].pricing_type -1;  // -1 is to make the type starts from 0

	//step 3: information type
	// default to historical info as class 1
	InformationClass = 1;
	RandomPercentage= g_GetRandomRatio() * 100; 
	for(i= 1; i< MAX_INFO_CLASS_SIZE; i++)
	{
		if(RandomPercentage >= g_DemandTypeMap[demand_type].cumulative_info_class_percentage[i-1] &&  RandomPercentage < g_DemandTypeMap[demand_type].cumulative_info_class_percentage[i])
			InformationClass = i+1; // return pretrip as 2 or enoute as 3
	}

	VOT = g_DemandTypeMap[demand_type].average_VOT;
	RandomPercentage= g_GetRandomRatio() * 100; 

	for(std::vector<VOTDistribution>::iterator itr = g_VOTDistributionVector.begin(); itr != g_VOTDistributionVector.end(); ++itr)
	{
		if( (*itr).demand_type == demand_type
			&& RandomPercentage >= (*itr).cumulative_percentage_LB 
			&& RandomPercentage <= (*itr).cumulative_percentage_UB )

			VOT = (*itr).VOT;
	}

	if(VOT < 1)  // enforcing minimum travel time
		VOT = 1;
	return true;
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

void g_FreeVehicleVector()
{
	cout << "Free vehicle set... " << endl;
	std::vector<DTAVehicle*>::iterator iterVehicle;							
	for (iterVehicle = g_VehicleVector.begin(); iterVehicle != g_VehicleVector.end(); iterVehicle++)
	{
		delete *iterVehicle;
	}

	g_VehicleVector.clear();
	g_VehicleMap.clear();
}

void g_FreeODTKPathVector()
{
	cout << "Free global path set... " << endl;

	g_ODTKPathVector.clear(); 
}


std::string g_GetTimeStampStrFromIntervalNo(int time_interval)
{
	CString str;
	int hour = time_interval/4;
	int min = (time_interval - hour*4)*15;

	if(hour<10)
		str.Format ("'0%d:%02d",hour,min);
	else
		str.Format ("'%2d:%02d",hour,min);

	// Convert a TCHAR string to a LPCSTR
	CT2CA pszConvertedAnsiString (str);

	// construct a std::string using the LPCSTR input
	std::string  strStd (pszConvertedAnsiString);


	return strStd;
}

CString g_GetAppRunningTime()
{
	CString str;
	CTime EndTime = CTime::GetCurrentTime();
	CTimeSpan ts = EndTime  - g_AppStartTime;

	str = ts.Format( "App Clock: %H:%M:%S --" );
	return str;
}
