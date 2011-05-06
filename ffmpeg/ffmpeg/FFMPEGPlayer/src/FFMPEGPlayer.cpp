/*
 ============================================================================
 Name		: FFMPEGPlayer.cpp
 Author	  : Alex
 Copyright   : Your copyright notice
 Description : Main application class
 ============================================================================
 */

// INCLUDE FILES
#include <eikstart.h>
#include "FFMPEGPlayerApplication.h"

LOCAL_C CApaApplication* NewApplication()
	{
	return new CFFMPEGPlayerApplication;
	}

GLDEF_C TInt E32Main()
	{
	return EikStart::RunApplication(NewApplication);
	}

