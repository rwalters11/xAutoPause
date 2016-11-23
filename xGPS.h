/*
* xGPS.h
*
* Custom classes and function headers for the TBG X-Plane GPS interaction
*/

#pragma once

#ifndef XTBGGPS_H

	#define XTBGGPS_H

	/* C++ headers */
	#include <iostream>
	#include <fstream>
	#include <string>
	#include <cmath>
	#include <ctime>

	#include "xtbgLibrary.h"

	using namespace std;

	/* Custom namespace for our plugins */
	namespace Xtbg
	{
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/// Structure for the FMS Leg Entries
		typedef struct _GPSENTRY_t
		{
			int Index;
			XPLMNavType navType = xplm_Nav_Unknown;							// XPlane NavAid types
			string navID = "";												// NavAid Identifier
			XPLMNavRef navRef = 0;											// XPlane Nav DB reference
			LatLong navLatLong;												// NavAid Lat/Long coordinates held in a structure			
		} GPSENTRY;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Class xGPS
		*
		* Obj for FMS interaction
		*
		*/

		class xGPS
		{

			// Private members and functions
		private:

			bool m_GPS_Present = false;						// FMS present in aircraft or not
			bool m_GPS_Active = false;						// FMS engaged or not

			int m_GPS_LegCount = 0;							// FMS number of entries

			int m_GPS_DisplayIndex = 0;						// FMS display index
			int m_GPS_DestinationIndex = 0;					// FMS destination index

			GPSENTRY m_GPS_LegInfo;							// FMS leg information

															// Public members and functions
		public:

			/* Constructors */
			xGPS();

			//Log(string logFileName);						// Constructor accepting Log File Name as parameter

			/* Set/Get*/
			//string getLogFilePathName();

			/* Functions */
			void Initialise();								// Set initial values

		};

		/* End of class xGPS */

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////











		/* End of custom namespace */
	}

#endif
