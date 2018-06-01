/**********************************************************************
 *<
	FILE: DllEntry.cpp

	DESCRIPTION: Contains the Dll Entry stuff

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "harmonicController.h"

extern ClassDesc2* GetHarmonicControllerDesc();

HINSTANCE hInstance;
int controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		MaxSDK::Util::UseLanguagePackLocale();
		hInstance = hinstDLL; // Hang on to this DLL's instance handle.
		DisableThreadLibraryCalls(hInstance);
	}
	return (TRUE);
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR* LibDescription() {
	return GetString(IDS_LIBDESCRIPTION);
}

// This function returns the number of plug-in classes this DLL
//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses() {
	return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) {
	switch(i) {
		case 0: return GetHarmonicControllerDesc();
		default: return 0;
	}
}

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion() {
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id) {
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

