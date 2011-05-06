/*
 ============================================================================
 Name		: FFMPEGPlayerApplication.h
 Author	  : Alex
 Copyright   : Your copyright notice
 Description : Declares main application class.
 ============================================================================
 */

#ifndef __FFMPEGPLAYERAPPLICATION_H__
#define __FFMPEGPLAYERAPPLICATION_H__

// INCLUDES
#include <aknapp.h>
#include "FFMPEGPlayer.hrh"

// UID for the application;
// this should correspond to the uid defined in the mmp file
const TUid KUidFFMPEGPlayerApp =
	{
	_UID3
	};

// CLASS DECLARATION

/**
 * CFFMPEGPlayerApplication application class.
 * Provides factory to create concrete document object.
 * An instance of CFFMPEGPlayerApplication is the application part of the
 * AVKON application framework for the FFMPEGPlayer example application.
 */
class CFFMPEGPlayerApplication : public CAknApplication
	{
public:
	// Functions from base classes

	/**
	 * From CApaApplication, AppDllUid.
	 * @return Application's UID (KUidFFMPEGPlayerApp).
	 */
	TUid AppDllUid() const;

protected:
	// Functions from base classes

	/**
	 * From CApaApplication, CreateDocumentL.
	 * Creates CFFMPEGPlayerDocument document object. The returned
	 * pointer in not owned by the CFFMPEGPlayerApplication object.
	 * @return A pointer to the created document object.
	 */
	CApaDocument* CreateDocumentL();
	};

#endif // __FFMPEGPLAYERAPPLICATION_H__
// End of File
