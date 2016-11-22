/*
* xAutoPause.cpp
*
* This plugin will provide an ability to pause the simulator at a given point
*
*/

/* #pragma warning directive disables Windows specific warning about recommended alternative
*  string function */

#pragma warning(disable:4996)

#define XPLM200 = 1;  // This plugin requires SDK2.0

/* My precompiled headers */
#include "stdafx.h"

/* C++ headers */
#include "xtbgLibrary.h"
#include "xFMS.h"

using namespace std;								// Strings etc are in namespace std

/*
* Global Variables.
*/

/* Plugin name and version in C++ and c types */
const string pluginName = "xAutoPause";
const string pluginVersion = "v1.0.0";
const string pluginSignature = "xtbg.autopause";
const string pluginDescription = "Plugin to pause sim at given location.";

/* File to write log to. */
static Xtbg::Log Logger;							// Create the Logging object
static string	logFileName="xAutoPause.txt";		// Log file name
static char		cOutputPath[512];					// char C string as X-Plane function returns a char type
//static string   outputPath="";						// char path converted to c++ string

#if APL && __MACH__ && RW
static char   outputPath=[512];
static char outputPath2[512];
int Result = 0;
#else
static string outputPath="";
#endif

/* Data refs we will get from x-plane. */
static XPLMDataRef		gPlaneLat;
static XPLMDataRef		gPlaneLon;

/* Current Latitude/Longitude */
static Xtbg::LatLong	currentLatLong;

// Widget style windows and elements for the settings window
static XPWidgetID xAutoPauseSettingsWidget = NULL, xAutoPauseMonitorWidget = NULL;
static XPWidgetID xAutoPauseWindow1 = NULL, xAutoPauseWindow2 = NULL;
static XPWidgetID GetFMSEntryButton, ClearEntryButton;
static XPWidgetID GetEntryIndexButton, SetEntryIndexButton, GetDestinationEntryButton, SetDestinationEntryButton;
static XPWidgetID EntryIndexEdit, DestinationEntryIndexEdit;
static XPWidgetID GetNumberOfEntriesButton, GetNumberOfEntriesText;
static XPWidgetID IndexCaption, IndexEdit, SegmentCaption, SegmentCaption2, AirportIDCaption, AirportIDEdit, AltitudeCaption, AltitudeEdit;
static XPWidgetID NavTypeCaption, NavTypeEdit, NavTypeText, UpArrow, DownArrow, SetFMSEntryButton;
static XPWidgetID LatCaption, LatEdit, LonCaption, LonEdit, SetLatLonButton;

// Widget style windows and elements for the settings window
static XPWidgetID MonitorLat, MonitorLong, MonitorWaypoint, MonitorDistance;

static bool ToggleMonitor=false;									// Visibility toggle for the monitor window

static int MenuItem1, NavTypeLinePosition, MonitorCreated;			// Flags

static Xtbg::PauseSegment currentPauseSegment;						// Pause segment obj to hold the current Pause Segment

/*
* Forward references to callback handlers
*
*/

// Menu Handlers
static void xAutoPauseMenuHandler(void *, void *);

// Show/Hide the monitor window widget
void Toggle_xAutoPauseMonitorWidget(bool inToggle);

// Create Widget style settings window
static void Create_xAutoPauseSettingsWidget(int x1, int y1, int w, int h);

// Create widget style monitor window
static void Create_xAutoPauseMonitorWidget(int x1, int y1, int w, int h);

static int GetCBIndex(int Type);

// Handler for our settings widget
static int xAutoPauseSettingsHandler(
	XPWidgetMessage			inMessage,
	XPWidgetID				inWidget,
	intptr_t				inParam1,
	intptr_t				inParam2);

// Handler for our monitor widget
static int xAutoPauseMonitorHandler(
	XPWidgetMessage			inMessage,
	XPWidgetID				inWidget,
	intptr_t				inParam1,
	intptr_t				inParam2);

// Flight Loop Callback
static float	MyFlightLoopCallback(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void *               inRefcon);

// Update the Monitor window
void UpdateMonitor(Xtbg::LatLong inLatLong);

// Pause the Simulator
static bool PauseSimulator();

#if APL && __MACH__ && RW
static int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen);
#endif

/*
* End of Callback Forward References
*/

/*
* XPluginStart
*
* Our start routine registers our window and does any other initialization we
* must do.
*
*/
PLUGIN_API int XPluginStart(
	char *		outName,
	char *		outSig,
	char *		outDesc)
{
	/* First we must fill in the passed in buffers to describe our
	* plugin to the plugin-system. */

	strcpy(outName, pluginName.c_str());
	strcpy(outSig,	pluginSignature.c_str());
	strcpy(outDesc, pluginDescription.c_str());

	/*
	*
	* Create the Log
	*
	*/

	// Locate the X-System directory
	XPLMGetSystemPath(cOutputPath);

	// Convert the char style path to a c++ string and append the path to our plugin
	string outputPath = string(cOutputPath) + "Resources\\Plugins\\" + outName;

	// Concatenate the file name to the path
	outputPath += "\\" + logFileName;

	#if APL && __MACH__ && RW
		Result = ConvertPath(outputPath, outputPath2, sizeof(outputPath));
		if (Result == 0)
			strcpy(outputPath, outputPath2);
		else
			XPLMDebugString("TimedProccessing - Unable to convert path\n");
	#endif

	#if APL
		XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);									// Use Posix paths on OS X
	#endif

	Logger.Open(outputPath);															// Create and open the log file

	// Create our menu
	XPLMMenuID	id;
	int			item;

	item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "xAutoPause", NULL, 1);
	id = XPLMCreateMenu("xAutoPause", XPLMFindPluginsMenu(), item, xAutoPauseMenuHandler, NULL);
	XPLMAppendMenuItem(id, "Settings", (void *)"xAutoPause", 1);

	/* Find the data refs we want to use. */
	gPlaneLat = XPLMFindDataRef("sim/flightmodel/position/latitude");
	gPlaneLon = XPLMFindDataRef("sim/flightmodel/position/longitude");

	/* Register our callback for once a second.  Positive intervals
	* are in seconds, negative are the negative of sim frames.  Zero
	* registers but does not schedule a callback for time. */
	XPLMRegisterFlightLoopCallback(
		MyFlightLoopCallback,	/* Callback */
		1.0,					/* Interval */
		NULL);					/* refcon not used. */

	// Flag to tell us if the settings widget is being displayed.
	MenuItem1 = 0;

	Logger.Write("Plugin started");

	return 1;						/* We must return 1 to indicate successful initialization, otherwise we
									* will not be called back again. */
}

/*
* XPluginStop
*
* Our cleanup routine deallocates our window.
*
*/
PLUGIN_API void	XPluginStop(void)
{

	/* Unregister the Flight Loop callback */
	XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);

	// Destroy widget window
	if (MenuItem1 == 1)
	{
		XPDestroyWidget(xAutoPauseSettingsWidget, 1);
		MenuItem1 = 0;
	}

	Logger.Write("Plugin stopped");
	Logger.Close();
}

/*
* XPluginDisable
*
* We do not need to do anything when we are disabled, but we must provide the handler.
*
*/
PLUGIN_API void XPluginDisable(void)
{
	Logger.Write("Plugin disabled");
	Logger.Flush();							// Flush any buffer contents so we can read the log file
}

/*
* XPluginEnable.
*
* We don't do any enable-specific initialization, but we must return 1 to indicate
* that we may be enabled at this time.
*
*/
PLUGIN_API int XPluginEnable(void)
{
	Logger.Write("Plugin enabled");
	return 1;
}

/*
* XPluginReceiveMessage
*
* We don't have to do anything in our receive message handler, but we must provide one.
*
*/
PLUGIN_API void XPluginReceiveMessage(
	XPLMPluginID	inFromWho,
	long			inMessage,
	void *			inParam)
{
}

/*
 * Callback functions for Widget
 */


// This will create our widget from menu selection.
// MenuItem1 flag stops it from being created more than once.
void xAutoPauseMenuHandler(void * mRef, void * iRef)
{
	// If menu selected create our widget dialog
	if (!strcmp((char *)iRef, "xAutoPause"))
	{
		if (MenuItem1 == 0)
		{
			Create_xAutoPauseSettingsWidget(221, 640, 420, 290);
			MenuItem1 = 1;
		}
		else
			if (!XPIsWidgetVisible(xAutoPauseSettingsWidget))
				XPShowWidget(xAutoPauseSettingsWidget);

		////////////////////////////////////////////////////////////////////////////
		/* For testing only */
		Toggle_xAutoPauseMonitorWidget(true);

		// Create test LatLong coordinates - GCTS Airport
		Xtbg::LatLong TestDestination;
		TestDestination.setLat(28.0445);
		TestDestination.setLong(-16.5725);

		if (currentPauseSegment.setNavAid("EGLL", xplm_Nav_Airport))
		{

		}
		else {

			Logger.Write(" Pause destination set to default: GCTS 28.0445,-16.5725");
		}

		////////////////////////////////////////////////////////////////////////////
	}
}

// Show/Hide the monitor window widget
// MonitorCreated flag stops it from being created more than once.
void Toggle_xAutoPauseMonitorWidget(bool inToggle)
{
	if (MonitorCreated == 0)										// Test if the monitor has already been created
	{
		Create_xAutoPauseMonitorWidget(100, 300, 420, 60);			// Create the window
		MonitorCreated = 1;											// Set the flag
		XPShowWidget(xAutoPauseMonitorWidget);						// Show it
	}
	else
	{
		if (!XPIsWidgetVisible(xAutoPauseMonitorWidget))			// Test if window is visible
			{
				XPShowWidget(xAutoPauseMonitorWidget);				// Show it
			}
			else
			{
				XPHideWidget(xAutoPauseMonitorWidget);				// Hide it
			}
	}
}

// Create the Monitor window widget
// All child widgets are relative to the input paramter.
void Create_xAutoPauseMonitorWidget(int x, int y, int w, int h)
{
	int x2 = x + w;
	int y2 = y - h;

	// Create the Monitor Widget window
	xAutoPauseMonitorWidget = XPCreateWidget(x, y, x2, y2,
		1,	// Visible
		"xAutoPause Monitor",	// desc
		1,		// root
		NULL,	// no container
		xpWidgetClass_MainWindow);

	// Set properties for the Main Widget
	XPSetWidgetProperty(xAutoPauseMonitorWidget, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
	XPSetWidgetProperty(xAutoPauseMonitorWidget, xpProperty_MainWindowHasCloseBoxes, 0);

	// Latitude text field
	MonitorLat = XPCreateWidget(x + 20, y - 20, x + 100, y - 42,
		1, "0", 0, xAutoPauseMonitorWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(MonitorLat, xpProperty_TextFieldType, xpTextTranslucent);
	XPSetWidgetProperty(MonitorLat, xpProperty_Enabled, 0);

	// Longitude text field
	MonitorLong = XPCreateWidget(x + 120, y - 20, x + 200, y - 42,
		1, "0", 0, xAutoPauseMonitorWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(MonitorLong, xpProperty_TextFieldType, xpTextTranslucent);
	XPSetWidgetProperty(MonitorLong, xpProperty_Enabled, 0);

	// Pause waypoint text field
	MonitorWaypoint = XPCreateWidget(x + 220, y - 20, x + 300, y - 42,
		1, "", 0, xAutoPauseMonitorWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(MonitorWaypoint, xpProperty_TextFieldType, xpTextTranslucent);
	XPSetWidgetProperty(MonitorWaypoint, xpProperty_Enabled, 0);

	// Distance to Pause point text field
	MonitorDistance = XPCreateWidget(x + 320, y - 20, x + 400, y - 42,
		1, "0.0", 0, xAutoPauseMonitorWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(MonitorDistance, xpProperty_TextFieldType, xpTextTranslucent);
	XPSetWidgetProperty(MonitorDistance, xpProperty_Enabled, 0);

	// Register our widget handler
	XPAddWidgetCallback(xAutoPauseMonitorWidget, xAutoPauseMonitorHandler);

}

// Handler for the monitor widget
int	xAutoPauseMonitorHandler(
	XPWidgetMessage			inMessage,
	XPWidgetID				inWidget,
	intptr_t				inParam1,
	intptr_t				inParam2)
{
	return 0;
}

// Create the Settings window widget
// All child widgets are relative to the input paramter.
void Create_xAutoPauseSettingsWidget(int x, int y, int w, int h)
{
	int x2 = x + w;
	int y2 = y - h;
	char Buffer[255];

	strcpy(Buffer, "xAutoPause by Richard Walters - 2016");

	// Create the Main Widget window
	xAutoPauseSettingsWidget = XPCreateWidget(x, y, x2, y2,
		1,	// Visible
		Buffer,	// desc
		1,		// root
		NULL,	// no container
		xpWidgetClass_MainWindow);

	// Set properties for the Main Widget
	XPSetWidgetProperty(xAutoPauseSettingsWidget, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
	XPSetWidgetProperty(xAutoPauseSettingsWidget, xpProperty_MainWindowHasCloseBoxes, 1);

	// Create the Sub Widget1 window
	xAutoPauseWindow1 = XPCreateWidget(x + 10, y - 30, x + 160, y2 + 10,
		1,	// Visible
		"",	// desc
		0,		// root
		xAutoPauseSettingsWidget,
		xpWidgetClass_SubWindow);

	// Set the style to sub window
	XPSetWidgetProperty(xAutoPauseWindow1, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);

	// Create the Sub Widget2 window
	xAutoPauseWindow2 = XPCreateWidget(x + 170, y - 30, x2 - 10, y2 + 10,
		1,	// Visible
		"",	// desc
		0,		// root
		xAutoPauseSettingsWidget,
		xpWidgetClass_SubWindow);

	// Set the style to sub window
	XPSetWidgetProperty(xAutoPauseWindow2, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);

	/// Entry Index
	GetEntryIndexButton = XPCreateWidget(x + 20, y - 40, x + 110, y - 62,
		1, " Get Entry Index", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(GetEntryIndexButton, xpProperty_ButtonType, xpPushButton);

	EntryIndexEdit = XPCreateWidget(x + 120, y - 40, x + 150, y - 62,
		1, "0", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(EntryIndexEdit, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(EntryIndexEdit, xpProperty_Enabled, 1);

	SetEntryIndexButton = XPCreateWidget(x + 20, y - 70, x + 110, y - 92,
		1, " Set Entry Index", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(SetEntryIndexButton, xpProperty_ButtonType, xpPushButton);

	/// Destination Index
	GetDestinationEntryButton = XPCreateWidget(x + 20, y - 100, x + 110, y - 122,
		1, " Get Dest Index", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(GetDestinationEntryButton, xpProperty_ButtonType, xpPushButton);

	DestinationEntryIndexEdit = XPCreateWidget(x + 120, y - 100, x + 150, y - 122,
		1, "0", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(DestinationEntryIndexEdit, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(DestinationEntryIndexEdit, xpProperty_Enabled, 1);

	SetDestinationEntryButton = XPCreateWidget(x + 20, y - 130, x + 110, y - 152,
		1, " Set Dest Index", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(SetDestinationEntryButton, xpProperty_ButtonType, xpPushButton);

	/// Number of Entries
	GetNumberOfEntriesButton = XPCreateWidget(x + 20, y - 160, x + 110, y - 182,
		1, " Get No. Entries", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(GetNumberOfEntriesButton, xpProperty_ButtonType, xpPushButton);

	GetNumberOfEntriesText = XPCreateWidget(x + 120, y - 160, x + 150, y - 182,
		1, "", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(GetNumberOfEntriesText, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(GetNumberOfEntriesText, xpProperty_Enabled, 0);

	/// Clear Entry
	ClearEntryButton = XPCreateWidget(x + 20, y - 190, x + 110, y - 212,
		1, " Clear Entry", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(ClearEntryButton, xpProperty_ButtonType, xpPushButton);

	/// Index (Segment - 1)
	IndexCaption = XPCreateWidget(x + 180, y - 40, x + 230, y - 62,
		1, "Index", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Caption);

	IndexEdit = XPCreateWidget(x + 240, y - 40, x + 290, y - 62,
		1, "0", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(IndexEdit, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(IndexEdit, xpProperty_Enabled, 1);

	SegmentCaption = XPCreateWidget(x + 300, y - 40, x + 350, y - 62,
		1, "Segment", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Caption);

	SegmentCaption2 = XPCreateWidget(x + 360, y - 40, x + 410, y - 62,
		1, "1", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Caption);

	/// Airport ID
	AirportIDCaption = XPCreateWidget(x + 180, y - 70, x + 230, y - 92,
		1, "Airport ID", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Caption);

	AirportIDEdit = XPCreateWidget(x + 240, y - 70, x + 290, y - 92,
		1, "----", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(AirportIDEdit, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(AirportIDEdit, xpProperty_Enabled, 1);

	/// Altitude
	AltitudeCaption = XPCreateWidget(x + 180, y - 100, x + 230, y - 122,
		1, "Altitude", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Caption);

	AltitudeEdit = XPCreateWidget(x + 240, y - 100, x + 290, y - 122,
		1, "0", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(AltitudeEdit, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(AltitudeEdit, xpProperty_Enabled, 1);

	/// Nav Type
	NavTypeCaption = XPCreateWidget(x + 180, y - 130, x + 230, y - 152,
		1, "Nav Type", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Caption);

	sprintf(Buffer, "%s", Xtbg::NavTypeLookup[0].Description);
	NavTypeEdit = XPCreateWidget(x + 240, y - 130, x + 340, y - 152,
		1, Buffer, 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(NavTypeEdit, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(NavTypeEdit, xpProperty_Enabled, 0);

	// Used for selecting Nav Type
	UpArrow = XPCreateWidget(x + 340, y - 130, x + 362, y - 141,
		1, "", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(UpArrow, xpProperty_ButtonType, xpLittleUpArrow);

	// Used for selecting Nav Type
	DownArrow = XPCreateWidget(x + 340, y - 141, x + 362, y - 152,
		1, "", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(DownArrow, xpProperty_ButtonType, xpLittleDownArrow);

	NavTypeText = XPCreateWidget(x + 362, y - 130, x + 400, y - 152,
		1, "0", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(NavTypeText, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(NavTypeText, xpProperty_Enabled, 0);

	/// Get FMS Entry Info
	GetFMSEntryButton = XPCreateWidget(x + 180, y - 160, x + 270, y - 182,
		1, " Get FMS Entry", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(GetFMSEntryButton, xpProperty_ButtonType, xpPushButton);

	/// Set FMS Entry Info
	SetFMSEntryButton = XPCreateWidget(x + 280, y - 160, x + 370, y - 182,
		1, " Set FMS Entry", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(SetFMSEntryButton, xpProperty_ButtonType, xpPushButton);

	/// Lat / Lon
	LatCaption = XPCreateWidget(x + 180, y - 190, x + 230, y - 212,
		1, "Latitude", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Caption);

	LatEdit = XPCreateWidget(x + 240, y - 190, x + 310, y - 212,
		1, "0", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(LatEdit, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(LatEdit, xpProperty_Enabled, 1);

	LonCaption = XPCreateWidget(x + 180, y - 220, x + 230, y - 242,
		1, "Longitude", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Caption);

	LonEdit = XPCreateWidget(x + 240, y - 220, x + 310, y - 242,
		1, "0", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_TextField);

	XPSetWidgetProperty(LonEdit, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(LonEdit, xpProperty_Enabled, 1);

	SetLatLonButton = XPCreateWidget(x + 180, y - 250, x + 270, y - 272,
		1, " Set Lat/Lon", 0, xAutoPauseSettingsWidget,
		xpWidgetClass_Button);

	XPSetWidgetProperty(SetLatLonButton, xpProperty_ButtonType, xpPushButton);

	// Register our widget handler
	XPAddWidgetCallback(xAutoPauseSettingsWidget, xAutoPauseSettingsHandler);
}


// Handler for the settings widget
// It can be used to process button presses etc.
// In this example we are only interested when the close box is pressed
int	xAutoPauseSettingsHandler(
	XPWidgetMessage			inMessage,
	XPWidgetID				inWidget,
	intptr_t				inParam1,
	intptr_t				inParam2)
	{
		void *Param;
		Param = 0;
		char Buffer[255];

		// Close button will get rid of our main widget
		// All child widgets will get the bullet as well
		if (inMessage == xpMessage_CloseButtonPushed)
		{
			if (MenuItem1 == 1)
			{
				XPHideWidget(xAutoPauseSettingsWidget);
			}
			return 1;
		}


		// Handle any button pushes
		if (inMessage == xpMsg_PushButtonPressed)
		{
			/// Most of these handlers get a value.
			/// It then has to be converted to a string using sprintf.
			/// This is because "XPSetWidgetDescriptor" expects a string as its second parameter.
			if (inParam1 == (intptr_t)ClearEntryButton)
			{
				XPLMClearFMSEntry(XPLMGetDisplayedFMSEntry());
				return 1;
			}

			if (inParam1 == (intptr_t)GetEntryIndexButton)
			{
				int Index = XPLMGetDisplayedFMSEntry();
				sprintf(Buffer, "%d", Index);
				XPSetWidgetDescriptor(EntryIndexEdit, Buffer);
				return 1;
			}

			if (inParam1 == (intptr_t)SetEntryIndexButton)
			{
				XPGetWidgetDescriptor(EntryIndexEdit, Buffer, sizeof(Buffer));
				int Index = atoi(Buffer);
				XPLMSetDisplayedFMSEntry(Index);
				return 1;
			}

			if (inParam1 == (intptr_t)GetDestinationEntryButton)
			{
				int Index = XPLMGetDestinationFMSEntry();
				sprintf(Buffer, "%d", Index);
				XPSetWidgetDescriptor(DestinationEntryIndexEdit, Buffer);
				return 1;
			}

			if (inParam1 == (intptr_t)SetDestinationEntryButton)
			{
				XPGetWidgetDescriptor(DestinationEntryIndexEdit, Buffer, sizeof(Buffer));
				int Index = atoi(Buffer);
				XPLMSetDestinationFMSEntry(Index);
				return 1;
			}

			if (inParam1 == (intptr_t)GetNumberOfEntriesButton)
			{
				int Count = XPLMCountFMSEntries();
				sprintf(Buffer, "%d", Count);
				XPSetWidgetDescriptor(GetNumberOfEntriesText, Buffer);
				return 1;
			}

			if (inParam1 == (intptr_t)GetFMSEntryButton)
			{
				int Index = XPLMGetDisplayedFMSEntry();
				XPLMNavType outType;
				char outID[80];
				XPLMNavRef outRef;
				int outAltitude;
				float outLat;
				float outLon;

				XPLMGetFMSEntryInfo(Index, &outType, outID, &outRef, &outAltitude, &outLat, &outLon);
				sprintf(Buffer, "%d", Index);
				XPSetWidgetDescriptor(IndexEdit, Buffer);
				sprintf(Buffer, "%d", Index + 1);
				XPSetWidgetDescriptor(SegmentCaption2, Buffer);
				if (outType == xplm_Nav_LatLon)
					XPSetWidgetDescriptor(AirportIDEdit, "----");
				else
					XPSetWidgetDescriptor(AirportIDEdit, outID);
				sprintf(Buffer, "%d", outAltitude);
				XPSetWidgetDescriptor(AltitudeEdit, Buffer);
				XPSetWidgetDescriptor(NavTypeEdit, Xtbg::NavTypeLookup[GetCBIndex(outType)].Description);
				sprintf(Buffer, "%d", Xtbg::NavTypeLookup[GetCBIndex(outType)].EnumValue);
				XPSetWidgetDescriptor(NavTypeText, Buffer);
				//sprintf(Buffer, "%+05.2f", Xtbg::HACKFLOAT(outLat));
				XPSetWidgetDescriptor(LatEdit, Buffer);
				//sprintf(Buffer, "%+05.2f", Xtbg::HACKFLOAT(outLon));
				XPSetWidgetDescriptor(LonEdit, Buffer);

				return 1;
			}

			if (inParam1 == (intptr_t)SetFMSEntryButton)
			{
				XPGetWidgetDescriptor(IndexEdit, Buffer, sizeof(Buffer));
				int Index = atoi(Buffer);
				sprintf(Buffer, "%d", Index + 1);
				XPSetWidgetDescriptor(SegmentCaption2, Buffer);
				XPGetWidgetDescriptor(AltitudeEdit, Buffer, sizeof(Buffer));
				int Altitude = atoi(Buffer);
				XPGetWidgetDescriptor(NavTypeText, Buffer, sizeof(Buffer));
				int NavType = atoi(Buffer);
				XPGetWidgetDescriptor(AirportIDEdit, Buffer, sizeof(Buffer));
				XPLMSetFMSEntryInfo(Index, XPLMFindNavAid(NULL, Buffer, NULL, NULL, NULL, NavType), Altitude);

				return 1;
			}

			if (inParam1 == (intptr_t)SetLatLonButton)
			{
				XPGetWidgetDescriptor(IndexEdit, Buffer, sizeof(Buffer));
				int Index = atoi(Buffer);
				sprintf(Buffer, "%d", Index + 1);
				XPSetWidgetDescriptor(SegmentCaption2, Buffer);
				XPGetWidgetDescriptor(AltitudeEdit, Buffer, sizeof(Buffer));
				int Altitude = atoi(Buffer);
				XPGetWidgetDescriptor(LatEdit, Buffer, sizeof(Buffer));
				//float Lat = atof(Buffer);                                 // RW edit - C++ returns float style numbers as a double
				float Lat = static_cast<float>(atof(Buffer));				// Resolved by casting to float
				XPGetWidgetDescriptor(LonEdit, Buffer, sizeof(Buffer));
				//float Lon = atof(Buffer);                                 // RW edit - C++ returns float style numbers as a double
				float Lon = static_cast<float>(atof(Buffer));				// Resolved by casting to float

				XPLMSetFMSEntryLatLon(Index, Lat, Lon, Altitude);			// Function expects Lat & Lon as floats

				return 1;
			}

			// Up Arrow is used to modify the NavTypeLookup Array Index
			if (inParam1 == (intptr_t)UpArrow)
			{
				NavTypeLinePosition--;
				if (NavTypeLinePosition < 0)
					NavTypeLinePosition = MAX_NAV_TYPES - 1;
				XPSetWidgetDescriptor(NavTypeEdit, Xtbg::NavTypeLookup[NavTypeLinePosition].Description);
				sprintf(Buffer, "%d", Xtbg::NavTypeLookup[NavTypeLinePosition].EnumValue);
				XPSetWidgetDescriptor(NavTypeText, Buffer);

				return 1;
			}

			// Down Arrow is used to modify the NavTypeLookup Array Index
			if (inParam1 == (intptr_t)DownArrow)
			{
				NavTypeLinePosition++;
				if (NavTypeLinePosition > MAX_NAV_TYPES - 1)
					NavTypeLinePosition = 0;
				XPSetWidgetDescriptor(NavTypeEdit, Xtbg::NavTypeLookup[NavTypeLinePosition].Description);
				sprintf(Buffer, "%d", Xtbg::NavTypeLookup[NavTypeLinePosition].EnumValue);
				XPSetWidgetDescriptor(NavTypeText, Buffer);

				return 1;
			}

		}

		return 0;
	}

/// This function takes an XPLMNavType and
/// returns the index into the NavTypeLookup array.
/// We can then use that index to access the description or enum.
int GetCBIndex(int Type)
{
	int CBIndex = 0;

	for (int Index = 0; Index<MAX_NAV_TYPES; Index++)
	{
		if (Xtbg::NavTypeLookup[Index].EnumValue == Type)
		{
			CBIndex = Index;
			break;
		}
	}
	return CBIndex;
}

// Flight Loop Callback
float	MyFlightLoopCallback(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void *               inRefcon)
{
	/* The actual callback.  First we read the sim's time and the data. */
	currentLatLong.setTime(XPLMGetElapsedTime());
	currentLatLong.setLat(XPLMGetDataf(gPlaneLat));
	currentLatLong.setLong(XPLMGetDataf(gPlaneLon));

	/* Update the monitor window*/
	if (MonitorCreated)
	{
		UpdateMonitor(currentLatLong);							// Update the monitor window passing in the current lat/long coordinates
	}

	/* Return 1.0 to indicate that we want to be called again in 1 second. */
	return 1.0;
}

// Update the Monitor window
void UpdateMonitor(Xtbg::LatLong inLatLong)
{
	char Buffer[20];																					// Initialise string buffer

	Xtbg::PauseSegment::strctDistances cbDistances = currentPauseSegment.SetCurrentPosition(inLatLong);	// Get the calculated distances to
																										// the current position which is
																										// passed in.
	string Waypoint = currentPauseSegment.NavAidID();

	sprintf(Buffer, "%f", currentLatLong.Lat());														// Update Latitude display
	XPSetWidgetDescriptor(MonitorLat, Buffer);

	sprintf(Buffer, "%f", currentLatLong.Long());														// Update Longitude display
	XPSetWidgetDescriptor(MonitorLong, Buffer);

	sprintf(Buffer, "%s", Waypoint.c_str());															// Update Waypoint display
	XPSetWidgetDescriptor(MonitorWaypoint, Buffer);

	sprintf(Buffer, "%5.1f", cbDistances.Distance2PausePoint);											// Update Distance display - formatted 9999.9
	XPSetWidgetDescriptor(MonitorDistance, Buffer);

	/* Test for Pause to be initiated*/
	if (cbDistances.Distance2PausePoint <= 0 && currentPauseSegment.Enabled())	// If Pause is enabled and we are within the pause radius
	{																			// of the chosen point

		currentPauseSegment.setEnabled(false);					// Disable Pause so it isnt triggered again on next callback loop cycle
		PauseSimulator();										// Pause the simulator


	}

}

// Pause the Simulator
bool PauseSimulator()
{
	XPLMCommandKeyStroke(xplm_key_pause);

	Logger.Write("Simulator paused at: ");

	return true;

}


#if APL && __MACH__ && RW
#include <Carbon/Carbon.h>
int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen)
{
	CFStringRef inStr = CFStringCreateWithCString(kCFAllocatorDefault, inPath ,kCFStringEncodingMacRoman);
	if (inStr == NULL)
		return -1;
	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inStr, kCFURLHFSPathStyle,0);
	CFStringRef outStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
	if (!CFStringGetCString(outStr, outPath, outPathMaxLen, kCFURLPOSIXPathStyle))
		return -1;
	CFRelease(outStr);
	CFRelease(url);
	CFRelease(inStr);
	return 0;
}
#endif

