/******************************************************************8
 *
 *	Tab Controls -- saves a lot of buggering about with tabs...
 *
 */


#include "TabCtl.h"
#include	<commctrl.h>

DLGTEMPLATE * WINAPI LockDlgResource(HINSTANCE hInstance, LPCSTR lpszResName) 	{ 
    HRSRC hrsrc = FindResource(NULL, lpszResName, RT_DIALOG); 
    HGLOBAL hglb = LoadResource(hInstance, hrsrc); 
    return (DLGTEMPLATE *) LockResource(hglb);
} 

void TABCTL_add(HINSTANCE hInstance, HWND wnd, DWORD tabctl, ...) {
	TCITEM tie;
	va_list marker;
	UWORD  i,count=0,j=0;
	HWND hwndTab=GetDlgItem(wnd,tabctl);
	CBYTE *pc;
	DLGPROC wp;
    TabInfo *pTabInfo = new TabInfo;

	pTabInfo->hwndTabCtl=hwndTab;
	pTabInfo->hwndDisplay=0;
	pTabInfo->hInstance=hInstance;

	SetWindowLong(hwndTab, GWL_USERDATA, (LONG) pTabInfo);

	// get count
	va_start(marker,tabctl);
		count=0;
		i=va_arg(marker,UWORD);
		while (i) {
			count++;
			pc=va_arg(marker,CBYTE*);
			wp=va_arg(marker,DLGPROC);
			i=va_arg(marker,UWORD);
		}
	va_end(marker);

	pTabInfo->resTabs = new DLGTEMPLATE*[count];
	pTabInfo->ChildProc = new DLGPROC[count];

	va_start(marker,tabctl);
		i=va_arg(marker,UWORD);
		while (i) {

			pTabInfo->resTabs[j]=LockDlgResource(hInstance,MAKEINTRESOURCE(i));

			tie.mask	= TCIF_TEXT | TCIF_IMAGE;
			tie.iImage	= -1;
			tie.pszText = va_arg(marker,CBYTE*);
			TabCtrl_InsertItem(hwndTab, j, &tie);
			pTabInfo->ChildProc[j]=va_arg(marker,DLGPROC);
			j++;

			i=va_arg(marker,UWORD);
		}
	va_end(marker);

	pTabInfo->tabcount=count;
	TABCTL_sel(wnd, tabctl);
}

void TABCTL_del(HWND wnd, DWORD tabctl) {
	HWND tabhWnd=GetDlgItem(wnd,tabctl);
	TabInfo *pTabInfo = (TabInfo*) GetWindowLong(tabhWnd,GWL_USERDATA);
//	SLONG i;

//  well there was me thinking we'd need to free up those Locked resources
//  like any other sane thing in the universe. 'parrently not. silly me.
//	for (i=0;i<TabInfo->tabcount;i++) {
//	}

	if (!pTabInfo) return; 

	delete [] (pTabInfo->resTabs);
	delete [] (pTabInfo->ChildProc);
	delete pTabInfo;
	SetWindowLong(tabhWnd,GWL_USERDATA,0);
}

void TABCTL_sel(HWND wnd, DWORD tabctl) {
	HWND tabhWnd=GetDlgItem(wnd,tabctl);
	TabInfo *pTabInfo = (TabInfo*) GetWindowLong(tabhWnd,GWL_USERDATA);
    int iSel = TabCtrl_GetCurSel(tabhWnd);

    // Destroy the current child dialog box, if any. 
    if (pTabInfo->hwndDisplay)
		DestroyWindow(pTabInfo->hwndDisplay);  
    // Create the new child dialog box. 
    pTabInfo->hwndDisplay =
		CreateDialogIndirect(pTabInfo->hInstance, pTabInfo->resTabs[iSel], tabhWnd, pTabInfo->ChildProc[iSel]);
}

SLONG TABCTL_getsel(HWND wnd, DWORD tabctl) {
	HWND tabhWnd=GetDlgItem(wnd,tabctl);
    return TabCtrl_GetCurSel(tabhWnd);
}

HWND	TABCTL_gethwnd(HWND wnd, DWORD tabctl) {
	HWND tabhWnd=GetDlgItem(wnd,tabctl);
	TabInfo *pTabInfo = (TabInfo*) GetWindowLong(tabhWnd,GWL_USERDATA);
	if (pTabInfo) 
		return pTabInfo->hwndDisplay;
	else
		return 0;
}