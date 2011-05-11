/*
 ============================================================================
 Name		: FFMPEGPlayerAppView.cpp
 Author	  : Alex
 Copyright   : Your copyright notice
 Description : Application view implementation
 ============================================================================
 */

// INCLUDE FILES
#include <coemain.h>
#include "FFMPEGPlayerAppView.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFFMPEGPlayerAppView::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFFMPEGPlayerAppView* CFFMPEGPlayerAppView::NewL(const TRect& aRect)
	{
	CFFMPEGPlayerAppView* self = CFFMPEGPlayerAppView::NewLC(aRect);
	CleanupStack::Pop(self);
	return self;
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerAppView::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFFMPEGPlayerAppView* CFFMPEGPlayerAppView::NewLC(const TRect& aRect)
	{
	CFFMPEGPlayerAppView* self = new (ELeave) CFFMPEGPlayerAppView;
	CleanupStack::PushL(self);
	self->ConstructL(aRect);
	return self;
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerAppView::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFFMPEGPlayerAppView::ConstructL(const TRect& aRect)
	{
	// Create a window for this application view
	CreateWindowL();

	// Set the windows size
	SetRect(aRect);

	// Activate the window, which makes it ready to be drawn
	ActivateL();
	
	m_pAVFormat = new AVFormatWrapper();
	
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerAppView::CFFMPEGPlayerAppView()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CFFMPEGPlayerAppView::CFFMPEGPlayerAppView()
	{
	// No implementation required
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerAppView::~CFFMPEGPlayerAppView()
// Destructor.
// -----------------------------------------------------------------------------
//
CFFMPEGPlayerAppView::~CFFMPEGPlayerAppView()
	{
		delete m_pAVFormat;
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerAppView::Draw()
// Draws the display.
// -----------------------------------------------------------------------------
//
void CFFMPEGPlayerAppView::Draw(const TRect& /*aRect*/) const
	{
	// Get the standard graphics context
	CWindowGc& gc = SystemGc();

	// Gets the control's extent
	TRect drawRect(Rect());

	// Clears the screen
	gc.Clear(drawRect);

	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerAppView::SizeChanged()
// Called by framework when the view size is changed.
// -----------------------------------------------------------------------------
//
void CFFMPEGPlayerAppView::SizeChanged()
	{
	DrawNow();
	}

// -----------------------------------------------------------------------------
// CFFMPEGPlayerAppView::HandlePointerEventL()
// Called by framework to handle pointer touch events.
// Note: although this method is compatible with earlier SDKs, 
// it will not be called in SDKs without Touch support.
// -----------------------------------------------------------------------------
//
void CFFMPEGPlayerAppView::HandlePointerEventL(
		const TPointerEvent& aPointerEvent)
	{

	// Call base class HandlePointerEventL()
	CCoeControl::HandlePointerEventL(aPointerEvent);
	}

// End of File
