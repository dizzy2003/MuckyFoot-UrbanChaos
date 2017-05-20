#ifndef _TABCTL_H_
#define _TABCTL_H_

/*******************************************************************
 *
 *							Tab Controls
 *	In WM_INITDIALOG call:
 *		TABCTL_add( handle_of_instance,
 *					handle_of_dialog_window,
 *					resource_id_of_tabctl, 
 *					resource_id_of_first_tab,
 *					title_of_first_tab,
 *					window_proc_of_first_tab,
 *					[repeat last 3 as required]  );
 *
 *  In WM_NOTIFY / TCN_SELCHANGE call:
 *		TABCTL_sel(	handle_of_dialog_window, resource_id_of_tabctl);
 *
 *	In WM_CLOSE call:
 *		TABCTL_del( handle_of_dialog_window, resource_id_of_tabctl);
 *
 */

#include <windows.h>
#include "MFStdLib.h"

struct TabInfo {
	 HWND			hwndTabCtl;
	 HWND			hwndDisplay;
	 HINSTANCE		hInstance;
	 SLONG			tabcount;
	 DLGTEMPLATE	**resTabs;
	 DLGPROC		*ChildProc;
};

// Add tabs to a tabcontrol:
void	TABCTL_add(HINSTANCE hInstance, HWND wnd, DWORD tabctl, ...);
// Free structures used:
void	TABCTL_del(HWND wnd, DWORD tabctl);
// Respond to selection change:
void	TABCTL_sel(HWND wnd, DWORD tabctl);
// Query for which tab is selected:
SLONG	TABCTL_getsel(HWND wnd, DWORD tabctl);
// Query for visible tab's window handle:
HWND	TABCTL_gethwnd(HWND wnd, DWORD tabctl);


#endif