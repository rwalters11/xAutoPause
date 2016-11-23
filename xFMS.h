/*
* xFMS.h
*
* Custom classes and function headers for the TBG X-Plane FMS interaction
*/

#pragma once

#ifndef XTBGFMS_H

	#define XTBGFMS_H

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
		typedef struct _FMSENTRY_t
		{
			int Index;
			XPLMNavType navType = xplm_Nav_Unknown;							// XPlane NavAid types
			string navID = "";												// NavAid Identifier
			XPLMNavRef navRef = 0;											// XPlane Nav DB reference
			LatLong navLatLong;												// NavAid Lat/Long coordinates held in a structure			
		} FMSENTRY;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/* Class xFMS
		*
		* Obj for FMS interaction
		*
		*/

		class xFMS
		{

			// Private members and functions
		private:

			bool m_FMS_Present = false;						// FMS present in aircraft or not
			bool m_FMS_Active = false;						// FMS engaged or not

			int m_FMS_LegCount = 0;							// FMS number of entries

			int m_FMS_DisplayIndex = 0;						// FMS display index
			int m_FMS_DestinationIndex = 0;					// FMS destination index

			FMSENTRY m_FMS_LegInfo[MAX_FMS_ENTRIES];		// FMS leg information

			void Initialise();								// Set initial values
			void GetFMSData();

			// Public members and functions
		public:

			/* Constructors */
			xFMS();

			/* Set/Get*/
			int FMSCount();									// Get

			/* Functions */
			

		};

		/* End of class xFMS */

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////











		/* End of custom namespace */
	}

#endif