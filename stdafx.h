#pragma once						// Header guard
#pragma warning(disable:4996)

/* If using a Windows compiler then include windows.h*/
#if IBM
#include <windows.h>
#endif

/* If using a MAC compiler then include carbon.h*/
#if APL
#if defined(__MACH__)
#include <Carbon/Carbon.h>
#endif
#endif

/* C++ headers */
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>

/* X-Plane SDK headers */
#include "XPLMDisplay.h"
#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"
#include "XPLMGraphics.h"
#include "XPLMPlanes.h"
#include "XPLMDataAccess.h"
#include "XPLMNavigation.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"




