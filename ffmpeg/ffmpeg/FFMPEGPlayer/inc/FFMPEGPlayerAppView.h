/*
 ============================================================================
 Name		: FFMPEGPlayerAppView.h
 Author	  : Alex
 Copyright   : Your copyright notice
 Description : Declares view class for application.
 ============================================================================
 */

#ifndef __FFMPEGPLAYERAPPVIEW_h__
#define __FFMPEGPLAYERAPPVIEW_h__

// INCLUDES
#include <coecntrl.h>

// CLASS DECLARATION
class CFFMPEGPlayerAppView : public CCoeControl
	{
public:
	// New methods

	/**
	 * NewL.
	 * Two-phased constructor.
	 * Create a CFFMPEGPlayerAppView object, which will draw itself to aRect.
	 * @param aRect The rectangle this view will be drawn to.
	 * @return a pointer to the created instance of CFFMPEGPlayerAppView.
	 */
	static CFFMPEGPlayerAppView* NewL(const TRect& aRect);

	/**
	 * NewLC.
	 * Two-phased constructor.
	 * Create a CFFMPEGPlayerAppView object, which will draw itself
	 * to aRect.
	 * @param aRect Rectangle this view will be drawn to.
	 * @return A pointer to the created instance of CFFMPEGPlayerAppView.
	 */
	static CFFMPEGPlayerAppView* NewLC(const TRect& aRect);

	/**
	 * ~CFFMPEGPlayerAppView
	 * Virtual Destructor.
	 */
	virtual ~CFFMPEGPlayerAppView();

public:
	// Functions from base classes

	/**
	 * From CCoeControl, Draw
	 * Draw this CFFMPEGPlayerAppView to the screen.
	 * @param aRect the rectangle of this view that needs updating
	 */
	void Draw(const TRect& aRect) const;

	/**
	 * From CoeControl, SizeChanged.
	 * Called by framework when the view size is changed.
	 */
	virtual void SizeChanged();

	/**
	 * From CoeControl, HandlePointerEventL.
	 * Called by framework when a pointer touch event occurs.
	 * Note: although this method is compatible with earlier SDKs, 
	 * it will not be called in SDKs without Touch support.
	 * @param aPointerEvent the information about this event
	 */
	virtual void HandlePointerEventL(const TPointerEvent& aPointerEvent);

private:
	// Constructors

	/**
	 * ConstructL
	 * 2nd phase constructor.
	 * Perform the second phase construction of a
	 * CFFMPEGPlayerAppView object.
	 * @param aRect The rectangle this view will be drawn to.
	 */
	void ConstructL(const TRect& aRect);

	/**
	 * CFFMPEGPlayerAppView.
	 * C++ default constructor.
	 */
	CFFMPEGPlayerAppView();

	};

#endif // __FFMPEGPLAYERAPPVIEW_h__
// End of File
