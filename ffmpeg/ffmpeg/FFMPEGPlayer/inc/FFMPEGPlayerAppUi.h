/*
 ============================================================================
 Name		: FFMPEGPlayerAppUi.h
 Author	  : Alex
 Copyright   : Your copyright notice
 Description : Declares UI class for application.
 ============================================================================
 */

#ifndef __FFMPEGPLAYERAPPUI_h__
#define __FFMPEGPLAYERAPPUI_h__

// INCLUDES
#include <aknappui.h>

// FORWARD DECLARATIONS
class CFFMPEGPlayerAppView;

// CLASS DECLARATION
/**
 * CFFMPEGPlayerAppUi application UI class.
 * Interacts with the user through the UI and request message processing
 * from the handler class
 */
class CFFMPEGPlayerAppUi : public CAknAppUi
	{
public:
	// Constructors and destructor

	/**
	 * ConstructL.
	 * 2nd phase constructor.
	 */
	void ConstructL();

	/**
	 * CFFMPEGPlayerAppUi.
	 * C++ default constructor. This needs to be public due to
	 * the way the framework constructs the AppUi
	 */
	CFFMPEGPlayerAppUi();

	/**
	 * ~CFFMPEGPlayerAppUi.
	 * Virtual Destructor.
	 */
	virtual ~CFFMPEGPlayerAppUi();

private:
	// Functions from base classes

	/**
	 * From CEikAppUi, HandleCommandL.
	 * Takes care of command handling.
	 * @param aCommand Command to be handled.
	 */
	void HandleCommandL(TInt aCommand);

	/**
	 *  HandleStatusPaneSizeChange.
	 *  Called by the framework when the application status pane
	 *  size is changed.
	 */
	void HandleStatusPaneSizeChange();

	/**
	 *  From CCoeAppUi, HelpContextL.
	 *  Provides help context for the application.
	 *  size is changed.
	 */
	CArrayFix<TCoeHelpContext>* HelpContextL() const;

private:
	// Data

	/**
	 * The application view
	 * Owned by CFFMPEGPlayerAppUi
	 */
	CFFMPEGPlayerAppView* iAppView;

	};

#endif // __FFMPEGPLAYERAPPUI_h__
// End of File
