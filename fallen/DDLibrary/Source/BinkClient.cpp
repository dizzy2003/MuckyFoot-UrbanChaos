// BinkClient.cpp
//
// simple client for BINK file playback
//
// DO NOT PUT ANY OF YOUR USUAL CRAP INTO THIS FILE
// If you want to change anything, do it in do_bink_intro() or bink_flipper() in GDisplay.cpp

#include "DDLib.h"


// Sod it - spoof the lot of them.
void BinkMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
}

void BinkPlay(const char* filename, IDirectDrawSurface* lpdds, bool (*flip)())
{
}
