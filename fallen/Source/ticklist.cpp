/*******************************************************************
 *
 *	Tick List -- saves a lot of buggering about with checkboxes...
 *
 */

#include "MFStdLib.h"
#include <windows.h>
#include "ticklist.h"

/*
 * OEM Resource Ordinal Numbers
 */
#define OBM_CLOSE           32754
#define OBM_UPARROW         32753
#define OBM_DNARROW         32752
#define OBM_RGARROW         32751
#define OBM_LFARROW         32750
#define OBM_REDUCE          32749
#define OBM_ZOOM            32748
#define OBM_RESTORE         32747
#define OBM_REDUCED         32746
#define OBM_ZOOMD           32745
#define OBM_RESTORED        32744
#define OBM_UPARROWD        32743
#define OBM_DNARROWD        32742
#define OBM_RGARROWD        32741
#define OBM_LFARROWD        32740
#define OBM_MNARROW         32739
#define OBM_COMBO           32738
#define OBM_UPARROWI        32737
#define OBM_DNARROWI        32736
#define OBM_RGARROWI        32735
#define OBM_LFARROWI        32734

#define OBM_OLD_CLOSE       32767
#define OBM_SIZE            32766
#define OBM_OLD_UPARROW     32765
#define OBM_OLD_DNARROW     32764
#define OBM_OLD_RGARROW     32763
#define OBM_OLD_LFARROW     32762
#define OBM_BTSIZE          32761
#define OBM_CHECK           32760
#define OBM_CHECKBOXES      32759
#define OBM_BTNCORNERS      32758
#define OBM_OLD_REDUCE      32757
#define OBM_OLD_ZOOM        32756
#define OBM_OLD_RESTORE     32755


void ticklist_init(HWND hWnd, SLONG id, CBYTE *pc[], SLONG bitmask) {
	SLONG		c0			=	1;									
	HWND		the_ctrl	=	GetDlgItem(hWnd,id);					
	LPTSTR		lbitem_str	=	pc[0];								
	while(*lbitem_str!='!')											
	{																
		SendMessage(the_ctrl,LB_ADDSTRING,0,(LPARAM)lbitem_str);	
		SendMessage(the_ctrl,LB_SETITEMDATA,c0-1,(bitmask & (1<<(c0-1))) ? 1 : 0);  
		lbitem_str	=	pc[c0++];									
	}																
	WNDPROC previous = (WNDPROC) SetWindowLong(the_ctrl,			
		GWL_WNDPROC,(long)ticklist_proc);							
	SetWindowLong(the_ctrl,GWL_USERDATA,(long)previous);			

}

void ticklist_close(HWND hWnd, SLONG id) {
	HWND	the_ctrl =				GetDlgItem(hWnd,id);
	WNDPROC previous = (WNDPROC)	GetWindowLong(the_ctrl, GWL_USERDATA);
	SetWindowLong(the_ctrl,GWL_WNDPROC,(long)previous);
}

BOOL	CALLBACK	ticklist_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	WNDPROC previous = (WNDPROC) GetWindowLong(hWnd, GWL_USERDATA);
	
	switch(message) {
	case WM_CHAR:
		if (wParam==32) {
			SLONG res,item;
			RECT rc;
			item=SendMessage(hWnd,LB_GETCURSEL,0,0);
			res=1-SendMessage(hWnd,LB_GETITEMDATA,item,0);
			SendMessage(hWnd,LB_SETITEMDATA,item,res);
			SendMessage(hWnd,LB_GETITEMRECT,item,(long)&rc);
			InvalidateRect(hWnd,&rc,0);
			return FALSE;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam)<16) {
			SLONG res,item;
			RECT rc;
			res=SendMessage(hWnd,LB_ITEMFROMPOINT,0,lParam);
			item=LOWORD(res);
			res=1-SendMessage(hWnd,LB_GETITEMDATA,item,0);
			SendMessage(hWnd,LB_SETITEMDATA,item,res);
			SendMessage(hWnd,LB_GETITEMRECT,item,(long)&rc);
			InvalidateRect(hWnd,&rc,0);
			return FALSE;
		}
		break;
	}

	return CallWindowProc(previous, hWnd, message, wParam, lParam);
}


SLONG ticklist_bitmask(HWND hWnd, SLONG id) {
	HWND ctl = GetDlgItem(hWnd,id);
	UBYTE i,ctr;
	SLONG mask=0;

	ctr=SendMessage(ctl,LB_GETCOUNT,0,0);
	for (i=0;i<ctr;i++) {
		mask|=(SendMessage(ctl,LB_GETITEMDATA,i,0)<<i);
	}
	return mask;
}


BOOL ticklist_measure(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT item = (LPMEASUREITEMSTRUCT) lParam;
	RECT rc;
	GetWindowRect(GetDlgItem(hWnd,item->CtlID),&rc);
	item->itemWidth=rc.right-rc.left;
	item->itemHeight=16;
	return TRUE;
}

BOOL ticklist_draw(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND ctl = GetDlgItem(hWnd,wParam);
	HDC  memdc;
	LPDRAWITEMSTRUCT item = (LPDRAWITEMSTRUCT) lParam;
	HBITMAP bmp,obmp;
	CBYTE pc[255];

	FillRect(item->hDC, &item->rcItem, (HBRUSH) GetStockObject(WHITE_BRUSH));
	SendMessage(ctl,LB_GETTEXT,item->itemID,(long)pc);
	TextOut(item->hDC,item->rcItem.left+20,item->rcItem.top+2,pc,strlen(pc));
	

	memdc=CreateCompatibleDC(item->hDC);
	bmp=(HBITMAP)LoadImage(NULL, (LPCTSTR)OBM_CHECKBOXES, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	obmp=(HBITMAP)SelectObject(memdc,bmp);

	BitBlt(item->hDC,item->rcItem.left+2,item->rcItem.top+2,12,12,memdc,(13*item->itemData),0,SRCCOPY);

	SelectObject(memdc,obmp);
	DeleteDC(memdc);
	DeleteObject(bmp);

	if (item->itemState & ODS_FOCUS) 
		DrawFocusRect(item->hDC,&item->rcItem);

	return TRUE;
}
