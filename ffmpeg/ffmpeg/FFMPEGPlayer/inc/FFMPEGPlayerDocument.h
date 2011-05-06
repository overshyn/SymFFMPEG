/*
 ============================================================================
 Name		: FFMPEGPlayerDocument.h
 Author	  : Alex
 Copyright   : Your copyright notice
 Description : Declares document class for application.
 ============================================================================
 */

#ifndef __FFMPEGPLAYERDOCUMENT_h__
#define __FFMPEGPLAYERDOCUMENT_h__

// INCLUDES
#include <akndoc.h>

// FORWARD DECLARATIONS
class CFFMPEGPlayerAppUi;
class CEikApplication;

// CLASS DECLARATION

/**
 * CFFMPEGPlayerDocument application class.
 * An instance of class CFFMPEGPlayerDocument is the Document part of the
 * AVKON application framework for the FFMPEGPlayer example application.
 */
class CFFMPEGPlayerDocument : public CAknDocument
	{
public:
	// Constructors and destructor

	/**
	 * NewL.
	 * Two-phased constructor.
	 * Construct a CFFMPEGPlayerDocument for the AVKON application aApp
	 * using two phase construction, and return a pointer
	 * to the created object.
	 * @param aApp Application creating this document.
	 * @return A pointer to the created instance of CFFMPEGPlayerDocument.
	 */
	static CFFMPEGPlayerDocument* NewL(CEikApplication& aApp);

	/**
	 * NewLC.
	 * Two-phased constructor.
	 * Construct a CFFMPEGPlayerDocument for the AVKON application aApp
	 * using two phase construction, and return a pointer
	 * to the created object.
	 * @param aApp Application creating this document.
	 * @return A pointer to the created instance of CFFMPEGPlayerDocument.
	 */
	static CFFMPEGPlayerDocument* NewLC(CEikApplication& aApp);

	/**
	 * ~CFFMPEGPlayerDocument
	 * Virtual Destructor.
	 */
	virtual ~CFFMPEGPlayerDocument();

public:
	// Functions from base classes

	/**
	 * CreateAppUiL
	 * From CEikDocument, CreateAppUiL.
	 * Create a CFFMPEGPlayerAppUi object and return a pointer to it.
	 * The object returned is owned by the Uikon framework.
	 * @return Pointer to created instance of AppUi.
	 */
	CEikAppUi* CreateAppUiL();

private:
	// Constructors

	/**
	 * ConstructL
	 * 2nd phase constructor.
	 */
	void ConstructL();

	/**
	 * CFFMPEGPlayerDocument.
	 * C++ default constructor.
	 * @param aApp Application creating this document.
	 */
	CFFMPEGPlayerDocument(CEikApplication& aApp);

	};

#endif // __FFMPEGPLAYERDOCUMENT_h__
// End of File
