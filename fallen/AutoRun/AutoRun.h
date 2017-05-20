// AutoRun.h
//
// main header file for AutoRun

// standard pre-compiled crap
#include "stdafx.h"

// our stuff
#include "Director.h"

// error reporting
inline void ReportError(const TCHAR* error)	{ MessageBox(NULL, error, "Error", MB_OK | MB_ICONINFORMATION); }
