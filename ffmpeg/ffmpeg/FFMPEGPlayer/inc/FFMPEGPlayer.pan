/*
 ============================================================================
 Name		: FFMPEGPlayer.pan
 Author	  : Alex
 Copyright   : Your copyright notice
 Description : This file contains panic codes.
 ============================================================================
 */

#ifndef __FFMPEGPLAYER_PAN__
#define __FFMPEGPLAYER_PAN__

/** FFMPEGPlayer application panic codes */
enum TFFMPEGPlayerPanics
	{
	EFFMPEGPlayerUi = 1
	// add further panics here
	};

inline void Panic(TFFMPEGPlayerPanics aReason)
	{
	_LIT(applicationName, "FFMPEGPlayer");
	User::Panic(applicationName, aReason);
	}

#endif // __FFMPEGPLAYER_PAN__
