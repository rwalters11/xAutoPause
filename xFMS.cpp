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

	/* Function set initial values*/
	void xFMS::Initialise()
	{
		// Determine if FMS programmed and/or active
		m_FMS_LegCount = XPLMCountFMSEntries();

		// Get FMS data
		if (m_FMS_LegCount > 0)
		{
			GetFMSData();
		}

	}

	/* Function to retrieve leg data from the FMS */
	void xFMS::GetFMSData()
	{

	}






	/*
	*
	* End of class xFMS
	*
	*/

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////




}
/* End of Namespace Xtbg */