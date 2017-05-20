//	SubClass.h
//	Guy Simmons, 15th August 1998.

#ifndef	SUBCLASS_H
#define	SUBCLASS_H

//---------------------------------------------------------------

extern WNDPROC		check_procs[],
					combo_procs[],
					edit_procs[],
					radio_procs[],
					tree_proc;


// LRESULT	CALLBACK	sc_check_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
// LRESULT	CALLBACK	sc_combo_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
// LRESULT	CALLBACK	sc_edit_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
// LRESULT	CALLBACK	sc_radio_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT	CALLBACK	sc_tree_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

//---------------------------------------------------------------

#endif
