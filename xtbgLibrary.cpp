/*
* xtbglibrary.cpp
*
TBG C++ Class and Function Library
*
*/

/* My precompiled headers */
#include "stdafx.h"

/* xtbgLibrary header */
#include "xtbgLibrary.h"

/* Unique Namespace for our plugins */
namespace Xtbg
{

	using namespace std;										// Strings etc are in namespace std

		/* Class Log
		*
		* Implements a log file and associated operations
		*
		*/

		/* Constructors */

		Log::Log() {}											// Empty constructor

		Log::Log(string logFilePathName)						// Constructor accepting Log File PathName as parameter
		{
			m_logFilePathName = logFilePathName;				// Assign the passed in file path and/or name
			Open(logFilePathName);
		}

		/* Set/Get*/
		string Log::getLogFilePathName() { return m_logFilePathName; }

		/* Functions */

		/* Function to write to the log file */
		int Log::Write(string inString)
		{
			if (strmLogFile.is_open())										// Test for file open
			{
				strmLogFile << currentDateTime() << " " + inString << '\n';		// Append log entry prepended with current date & time
				return 1;
			}
			else
			{
				// return error if file is not open
				return -1;
			}
		}

		/* Internal Function to open the log file */
		int Log::Open(string logFilePathName)
		{
			if (!strmLogFile.is_open())							// Test for file already open
			{
				strmLogFile.open(logFilePathName);				// Open the file for writing
				Write(logFilePathName);
				return 1;										// default return value
			}
			else
			{
				return -1;										// return error if file is not open
			}
		}

		/* Internal Function to close the log file */
		int Log::Close()
		{
			if (strmLogFile.is_open())							// Test for file open
			{
				strmLogFile.close();								// Close the file
				return 1;
			}
			else
			{
				return -1;										// return error if file is not open
			}
		}

		/* Function to flush the log file */					// Writes any buffer content into the file
		int Log::Flush()
		{
			if (strmLogFile.is_open())							// Test for file open
			{
				strmLogFile.flush();								// Flush the file
				return 1;
			}
			else
			{
				return -1;										// return error if file is not open
			}
		}

		/*
		*
		* End of class Log
		*
		*/

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Class LatLong
		*
		* Holds a Latitude-Longitude Coordinate
		*
		*/

		/* Constructors */

		LatLong::LatLong() {}

		LatLong::LatLong(double inLat, double inLong)
		{
			m_Lat = inLat;
			m_Long = inLong;
		}

		/* Set/Get*/
		double LatLong::Lat() { return m_Lat; }								// Get latitude
		double LatLong::Long() { return m_Long; }							// Get longitude

		void LatLong::setLat(double inLat) { m_Lat = inLat; }				// Set latitude
		void LatLong::setLong(double inLong) { m_Long = inLong; }			// Set longitude
		void LatLong::setTime(float inTime) { m_time = inTime; }			// Set time

		/* Functions */



		/* End of class LatLong */

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Class PauseSegment
		*
		* Holds the data and functions relating to the current Pause Segment
		*
		*/

		/* Constructors */
		PauseSegment::PauseSegment(){}												// Empty constructor

		PauseSegment::PauseSegment(LatLong inDestination)							// Constructor passing in destination coordinates
		{
			m_NavData.navLatLong = inDestination;
		}

		// Enabled status
		bool PauseSegment::Enabled() { return m_Enabled; }							// Get
		void PauseSegment::setEnabled(bool inValue) { m_Enabled = inValue; }		// Set

		// Pause Radius
		int PauseSegment::Radius() { return m_Radius; }								// Get
		void PauseSegment::setRadius(int inValue) { m_Radius = inValue; }			// Set

		// NavAid
		NAVAID_DATA PauseSegment::NavAid() { return m_NavData; }					// Get
		bool PauseSegment::setNavAid(string inID, XPLMNavType inNavType)			// Set
		{
			m_NavData.navID = inID;													// Put the user entered ID & type into the Pause NavData obj
			m_NavData.navType = inNavType;

			Xtbg::NAVAID_DATA XPNavData;

			XPNavData = Xtbg::GetXPData(inID, inNavType);							// Get the data from XPlane

			if (XPNavData.navRef != XPLM_NAV_NOT_FOUND)								// Test for validity
			{
				m_NavData.navRef = XPNavData.navRef;								// Update the Pause NavData obj
				m_NavData.navName = XPNavData.navName;
				m_NavData.navLatLong = XPNavData.navLatLong;
				m_DestinationSet = true;											// Flag our obj as holding a valid destination

				return true;
			}
			else {
				return false;
			}
		}

		//NavAid ID
		string PauseSegment::NavAidID() { return m_NavData.navID; }

		// Sets the destination coordinates
		void PauseSegment::setDestinationCoordinates(LatLong inDestinationCoordinates)
		{
			m_NavData.navLatLong = inDestinationCoordinates;
			m_DestinationSet = true;
		}

		// Sets the current position & returns the distance to pause point
		PauseSegment::strctDistances PauseSegment::SetCurrentPosition(LatLong inCurrentPosition)
		{
			m_CurrentPosition = inCurrentPosition;									// import the current lat/long position

			if (m_DestinationSet)													// If a valid destination is set then do the calculations
			{
				/* Calculate the distances */
				m_Distances.Distance2Destination = Point2Point(inCurrentPosition, m_NavData.navLatLong);

				m_Distances.Distance2PausePoint = m_Distances.Distance2Destination - m_Radius;

			}
			else {																	// If not then return 0's

				m_Distances.Distance2Destination = 0;
				m_Distances.Distance2PausePoint = 0;

			}

			return m_Distances;														// return
		}

		/* End of class PauseSegment */

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Function Point2Point
		*
		* Calculates the distance between a pair of Latitude-Longitude Coordinates
		*
		*/

		#define R 6371															// Earth's radius in km
		#define TO_RAD (3.1415926536 / 180)										// Conversion to Radians

		double Point2Point(LatLong inLatLong1, LatLong inLatLong2)
		{
			double  th1 = inLatLong1.Lat();										// Get Lat/Longs from class members
			double  ph1 = inLatLong1.Long();
			double  th2 = inLatLong2.Lat();
			double  ph2 = inLatLong2.Long();

			return dist(th1, ph1, th2, ph2);									// return the calculated distance

		}

		// Distance calculation using the Haversine great circle formula
		double dist(double th1, double ph1, double th2, double ph2)
		{
			double dx, dy, dz;
			ph1 -= ph2;
			ph1 *= TO_RAD, th1 *= TO_RAD, th2 *= TO_RAD;

			dz = sin(th1) - sin(th2);
			dx = cos(ph1) * cos(th1) - cos(th2);
			dy = sin(ph1) * cos(th1);
			return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Function GetXPData
		*
		* Gets NavAidData from the XPlane database
		*
		*/

		NAVAID_DATA GetXPData(string inDestinationIdentifier, XPLMNavType inNavType)
		{
			XPLMNavRef ref;																				// Initialise return ref for FindNavAid
			float Lat, Long;
			char Name[256];

			NAVAID_DATA XPNavData;

			XPNavData.navType = inNavType;																// Put NavType into struct
			XPNavData.navID = inDestinationIdentifier;

			ref = XPLMFindNavAid(NULL, inDestinationIdentifier.c_str(), NULL, NULL, NULL, inNavType);	// Find the nav ref

			XPNavData.navRef = ref;																		// Put ref into struct

			if (ref != XPLM_NAV_NOT_FOUND)																// Test for valid ref found
			{
				XPLMGetNavAidInfo(ref, NULL, &Lat, &Long, NULL, NULL, NULL, NULL, Name, NULL);			// Get the nav data

				XPNavData.navLatLong.setLat(double(Lat));												// Put Lat/Long into struct
				XPNavData.navLatLong.setLong(double(Long));
				XPNavData.navName = string(Name);
				XPNavData.navValid = true;																// Flag as valid

			}

			return XPNavData;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Function currentDateTime
		*
		* Returns the current date/time Localised in format YYYY-MM-DD.HH:mm:ss
		*
		*/

		const std::string currentDateTime() {
			time_t     now = time(0);
			struct tm  tstruct;
			char       buf[80];
			tstruct = *localtime(&now);
			// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
			// for more information about date/time format
			strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

			return buf;
		}


		//////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Function XP_Info
		*
		* Retrieves info about X-Plane Installation
		*
		*/

		XPLANE_INFO XP_Info()
		{
			XPLANE_INFO m_XPInfo;

			XPLMGetVersions(&m_XPInfo.Version, &m_XPInfo.XPLM_version, &m_XPInfo.ApplicationID);

			return m_XPInfo;

		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////




	/*
	*
	* Apple MAC specific functions
	*
	*/

	#if APL && __MACH__
	#include <Carbon/Carbon.h>
		int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen)
		{
			CFStringRef inStr = CFStringCreateWithCString(kCFAllocatorDefault, inPath, kCFStringEncodingMacRoman);
			if (inStr == NULL)
				return -1;
			CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inStr, kCFURLPOSIXPathStyle, 0);
			CFStringRef outStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
			if (!CFStringGetCString(outStr, outPath, outPathMaxLen, kCFURLPOSIXPathStyle))
				return -1;
			CFRelease(outStr);
			CFRelease(url);
			CFRelease(inStr);
			return 0;
		}
	#endif

		/// This is used to get around a sprintf float bug with codewarrior on the mac

		#if IBM || LIN
		//inline	float	HACKFLOAT(float val)
		inline	long long	HACKFLOAT(float val)
		{
		return val;
		}
		#else
		inline long long HACKFLOAT(float val)
		{
		double	d = val;
		long long temp;
		temp = *((long long *) &d);
		return temp;
		}
		#endif


}
/* End of Namespace Xtbg */

