/*
* xtbglibrary.h
*
* Custom classes and function headers for the TBG X-Plane C++ Library
*/

#pragma once

#ifndef XTBGLIB_H

	#define XTBGLIB_H

	/* C++ headers */
	#include <iostream>
	#include <fstream>
	#include <string>
	#include <cmath>
	#include <ctime>

	using namespace std;

	/* Custom namespace for our library */
	namespace Xtbg
	{
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Defined default values
		#define PAUSE_RADIUS 5

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/// Structure for the nav types and a more meaningful description
		typedef struct _NAVTYPE
		{
			char Description[80];
			XPLMNavType EnumValue;
		} NAVTYPE;

		#define MAX_NAV_TYPES 13
		#define MAX_FMS_ENTRIES 100

		static NAVTYPE NavTypeLookup[MAX_NAV_TYPES] = { { "Unknown",			xplm_Nav_Unknown },
														{ "Airport",			xplm_Nav_Airport },
														{ "NDB",				xplm_Nav_NDB },
														{ "VOR",				xplm_Nav_VOR },
														{ "ILS",				xplm_Nav_ILS },
														{ "Localizer",			xplm_Nav_Localizer },
														{ "Glide Slope",		xplm_Nav_GlideSlope },
														{ "Outer Marker",		xplm_Nav_OuterMarker },
														{ "Middle Marker",		xplm_Nav_MiddleMarker },
														{ "Inner Marker",		xplm_Nav_InnerMarker },
														{ "Fix",				xplm_Nav_Fix },
														{ "DME",				xplm_Nav_DME },
														{ "Lat/Lon",			xplm_Nav_LatLon } };

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Class Log
		*
		* Implements a log file and associated operations
		*
		*/

		class Log
		{

			// Private members and functions
		private:

			string m_logFilePathName = "log.txt";			// Default file name
			ofstream strmLogFile;							// Define the stream object

			// Public members and functions
		public:

			/* Constructors */
			Log();
			Log(string logFileName);						// Constructor accepting Log File Name as parameter

			/* Set/Get*/
			string getLogFilePathName();

			/* Functions */
			int Open(string logFilePathName);
			int Write(string inString);
			int Flush();
			int Close();

		};

		/* End of class Log */

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Class LatLong
		*
		* Holds a Latitude-Longitude Coordinate
		*
		*/

		class LatLong
		{
		private:											// Internal variables
			double m_Lat = 0;								// Latitude
			double m_Long = 0;								// Longitude
			float  m_time = 0;								// Timestamp

		public:

			/* Constructors */
			LatLong();
			LatLong(double inLat, double inLong);			// Constructor accepting lat/long values in decimal degrees

			/* Set/Get*/
			double Lat();									// Get Latitude
			double Long();									// Get Longitude

			void setLat(double inLat);						// Set Latitude
			void setLong(double inLong);					// Set Longitude
			void setTime(float inTime);						// Set time

			/* Functions */


		};

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/// Structure for Nav Data and it's validity
		typedef struct _NAVAIDDATA_t
		{
			XPLMNavRef navRef = 0;										// XPlane Nav DB reference
			string navName = "";											// Name of NavAid
			string navID = "";												// NavAid Identifier
			LatLong navLatLong;												// NavAid Lat/Long coordinates held in a structure
			int navFrequency = 0;											// NavAid Frequency
			XPLMNavType navType = xplm_Nav_Unknown;							// XPlane NavAid types

			bool navValid = false;											// Valid NavAid flag for function returns
		}NAVAID_DATA;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Class PauseSegment
		*
		* Holds the data and functions relating to the current Pause Segment
		*
		*/

		class PauseSegment
		{
		public:
			// Structure to hold the values from a distance calculation
			struct strctDistances {
				float Distance2Destination = 0;				// Distance to the destination
				float Distance2PausePoint = 0;				// Distance to the pause point
			};

		private:											// Internal variables
			NAVAID_DATA m_NavData;							// Structure containing NavAid data
			LatLong m_CurrentPosition;						// Latitude/Longitude coordinates of the current a/c position
			bool m_DestinationSet = false;					// Valid destination set flag
			int m_Radius = PAUSE_RADIUS;					// Radius in nautical miles of defining circle around pause coordinates
			strctDistances m_Distances;						// Distance to the destination & pause points
			bool m_Enabled = false;							// Pause enabled flag - default false

		/* Private Functions */

		public:

		/* Constructors */
			PauseSegment();									// Empty constructor
			PauseSegment(LatLong inDestination);

		/* Set/Get*/
			strctDistances SetCurrentPosition(LatLong inCurrentPosition);		// Sets the current position & returns the distance to pause point
			void setDestinationCoordinates(LatLong inDestinationCoordinates);	// Sets the destination coordinates

			bool Enabled();														// Get
			void setEnabled(bool inValue);										// Set

			int Radius();														// Get
			void setRadius(int inValue);										// Set

			bool setNavAid(string inID, XPLMNavType inNavType);					// Set
			NAVAID_DATA NavAid();												// Get

			string NavAidID();													// Get

		/* Functions */


		};

		/* End of class PauseSegment */

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Function Point2Point
		*
		* Calculates the distance between a pair of Latitude-Longitude Coordinates
		*
		*/

		double Point2Point(LatLong inLatLong1, LatLong inLatLong2);
		double dist(double th1, double ph1, double th2, double ph2);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Function GetXPData
		*
		* Gets NavAidData from the XPlane database
		*
		*/

		NAVAID_DATA GetXPData(string inDestinationIdentifier, XPLMNavType inNavType);		// Gets the lat/long coordinates of a naviad from X-Plane database

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Function currentDateTime
		*
		* Returns the current date/time Localised in format YYYY-MM-DD.HH:mm:ss
		*
		*/

		const string currentDateTime();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/*
		*
		* Apple MAC specific functions
		*
		*/

		#if APL && __MACH__
			static int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen);
		#endif

		/// This is used to get around a sprintf float bug with codewarrior on the mac
			inline	long long	HACKFLOAT(float val);

	/* End of custom namespace */
	}

#endif
