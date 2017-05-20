// WindProcs.h
// Guy Simmons, 14th November 1997.

#ifndef	WINDPROCS_H
#define	WINDPROCS_H

#include	"DDManager.h"


//---------------------------------------------------------------

typedef struct
{
	DDDriverInfo	*DriverCurrent,
					*DriverNew;
	
	D3DDeviceInfo	*DeviceCurrent,
					*DeviceNew;

	DDModeInfo		*ModeCurrent,
					*ModeNew;
}ChangeDDInfo;

//---------------------------------------------------------------

BOOL FAR PASCAL		AboutBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK		ChangeDriverProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT	CALLBACK	DDLibShellProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------

#endif
