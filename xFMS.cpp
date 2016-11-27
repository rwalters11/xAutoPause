/*
* xFMS.cpp
*
Custom classes and functions for the TBG X-Plane FMS interaction
*
*/

/* My precompiled headers */
#include "stdafx.h"

/* xFMS header */
#include "xFMS.h"

#define MAX_FMS_ENTRIES 100

/* Unique Namespace for our plugins */
namespace Xtbg
{

	using namespace std;										// Strings etc are in namespace std

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* Class xFMS
	*
	* Obj for FMS interaction

	/* Constructors */

	xFMS::xFMS() { Initialise(); }											// Empty constructor

	/* Set/Get*/
	int xFMS::FMSCount() { return m_FMS_LegCount; }							// Get

	/* Functions */

	/* Function to set initial values*/
	void xFMS::Initialise()
	{
		// Determine if FMS programmed and/or active
		m_FMS_LegCount = XPLMCountFMSEntries();								// Get number of legs

		// Get FMS data
		if (m_FMS_LegCount > 0)												// Test for positive count
		{
			GetFMSData();													// Get the leg data
		}

	}

	/* Function to retrieve leg data from the FMS */
	void xFMS::GetFMSData()
	{
		XPLMNavType outType;
		char outID[80];
		XPLMNavRef outRef;
		int outAltitude;
		float outLat;
		float outLon;

		if (m_FMS_LegCount > 0)															// Test for positive count
		{
			m_FMS_DisplayIndex = XPLMGetDisplayedFMSEntry();							// Get FMS entry currently displayed
			m_FMS_DestinationIndex = XPLMGetDestinationFMSEntry();						// Get FMS current leg destination

			for (int Index = 0; Index < m_FMS_LegCount; Index += 1)						// Loop through 0 based array
			{
				XPLMGetFMSEntryInfo(Index, &outType, outID, &outRef, &outAltitude, &outLat, &outLon);

				m_FMS_LegInfo[Index].Index = 0;
				m_FMS_LegInfo[Index].navID = outID;
				m_FMS_LegInfo[Index].navRef = outRef;
				m_FMS_LegInfo[Index].navType = outType;
				m_FMS_LegInfo[Index].navLatLong.setLat(outLat);
				m_FMS_LegInfo[Index].navLatLong.setLong(outLon);

			}
		}
	}






	/*
	*
	* End of class xFMS
	*
	*/

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////




}
/* End of Namespace Xtbg */