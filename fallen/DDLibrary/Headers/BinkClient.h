// BinkClient.h
//
// simple client for BINK file playback

extern void BinkPlay(const char* filename, IDirectDrawSurface* lpdds, bool (*flip)());

extern void BinkMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);