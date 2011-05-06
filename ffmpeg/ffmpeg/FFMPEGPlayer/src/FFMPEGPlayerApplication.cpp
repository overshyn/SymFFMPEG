/*
 ============================================================================
 Name		: FFMPEGPlayerApplication.cpp
 Author	  : Alex
 Copyright   : Your copyright notice
 Description : Main application class
 ============================================================================
 */

// INCLUDE FILES
#include "FFMPEGPlayer.hrh"
#include "FFMPEGPlayerDocument.h"
#include "FFMPEGPlayerApplication.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFFMPEGPlayerApplication::CreateDocumentL()
// Creates CApaDocument object
// -----------------------------------------------------------------------------
//
CApaDocument* CFFMPEGPlayerApplication::CreateDocumentL()
	{
	// Create an FFMPEGPlayer document, and return a pointer to it
	return CFFMPEGPlayerDocument::NewL(*this);
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerApplication::AppDllUid()
// Returns application UID
// -----------------------------------------------------------------------------
//
TUid CFFMPEGPlayerApplication::AppDllUid() const
	{
	// Return the UID for the FFMPEGPlayer application
	return KUidFFMPEGPlayerApp;
	}

// End of File
