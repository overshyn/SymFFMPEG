/*
 ============================================================================
 Name		: FFMPEGPlayerDocument.cpp
 Author	  : Alex
 Copyright   : Your copyright notice
 Description : CFFMPEGPlayerDocument implementation
 ============================================================================
 */

// INCLUDE FILES
#include "FFMPEGPlayerAppUi.h"
#include "FFMPEGPlayerDocument.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFFMPEGPlayerDocument::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFFMPEGPlayerDocument* CFFMPEGPlayerDocument::NewL(CEikApplication& aApp)
	{
	CFFMPEGPlayerDocument* self = NewLC(aApp);
	CleanupStack::Pop(self);
	return self;
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerDocument::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFFMPEGPlayerDocument* CFFMPEGPlayerDocument::NewLC(CEikApplication& aApp)
	{
	CFFMPEGPlayerDocument* self = new (ELeave) CFFMPEGPlayerDocument(aApp);

	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerDocument::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFFMPEGPlayerDocument::ConstructL()
	{
	// No implementation required
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerDocument::CFFMPEGPlayerDocument()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CFFMPEGPlayerDocument::CFFMPEGPlayerDocument(CEikApplication& aApp) :
	CAknDocument(aApp)
	{
	// No implementation required
	}

// ---------------------------------------------------------------------------
// CFFMPEGPlayerDocument::~CFFMPEGPlayerDocument()
// Destructor.
// ---------------------------------------------------------------------------
//
CFFMPEGPlayerDocument::~CFFMPEGPlayerDocument()
	{
	// No implementation required
	}

// ---------------------------------------------------------------------------
// CFFMPEGPlayerDocument::CreateAppUiL()
// Constructs CreateAppUi.
// ---------------------------------------------------------------------------
//
CEikAppUi* CFFMPEGPlayerDocument::CreateAppUiL()
	{
	// Create the application user interface, and return a pointer to it;
	// the framework takes ownership of this object
	return new (ELeave) CFFMPEGPlayerAppUi;
	}

// End of File
