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

	using namespace std;

	/* Custom namespace for our plugins */
	namespace Xtbg
	{

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

			FMSENTRY m_GPS_LegInfo;							// FMS leg information

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
