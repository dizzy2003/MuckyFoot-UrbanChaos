// propedit.h
// windowsy Property Editor
// you must create a window List View (NOT a list _box_) on your
// dialog like the one in the cutscene editor in order for it to work

#include "MFStdLib.h"
#include "windows.h"
#include "commctrl.h"

#define PROPTYPE_STRING		(0)
#define PROPTYPE_INT		(1)
#define PROPTYPE_BOOL		(2)
#define PROPTYPE_MULTI		(3)
#define PROPTYPE_READONLY	(4)
#define PROPTYPE_BUTTON		(5)

#define UM_DROP				(WM_USER + 1)

#define	TBCB_DBLCLK			(1)
#define	TBCB_DRAG			(2)

#define TLCB_GETBARINFO		(1)
#define TLCB_SELECT			(2)

#define PECB_UPDATE			(1)
#define PECB_BUTTON			(2)
#define PECB_EDITMODE		(3)

class PropertyEditor;
class TreeBrowser;
class DragServer;
class TimeLine;
class TimeLineRuler;
class TimeLineScroll;

//typedef void (*PROPEDIT_notify)(PropertyEditor *pe);

typedef BOOL (*TreeBrowserCB)(TreeBrowser *tb, int reason, int index, HTREEITEM item, char *txt);

typedef BOOL (*TimeLineCB)(TimeLine *tb, int reason, int index, int subline, int cell);

typedef BOOL (*PropEditCB)(PropertyEditor *tb, int reason, int index, CBYTE *value);

class GadgetBase {
public:
	HWND		hWnd;
	void Repaint();
};

class PropertyEditor : public GadgetBase {
private:
	int			property_count;
	PropEditCB  callback;
public:
    PropertyEditor(HWND nhWnd);

	void Clear();
	int  Add(CBYTE *name, CBYTE *value, UBYTE type);
	BOOL Process(HWND parent, WPARAM wParam, LPARAM lParam);
	int  Type(UWORD index);
	void Update(UWORD index, CBYTE *value);
	BOOL Verify(UBYTE type, CBYTE *value);
	void SetCallback(PropEditCB cb);
};


class TreeBrowser : public GadgetBase {
private:
	HTREEITEM		itemstack[10];
	DragServer		*drag;
	int				selection;
	HIMAGELIST		image_list;
	TreeBrowserCB	callback;
public:
	TVITEM			drag_item;
	TreeBrowser(HWND nhWnd);
	~TreeBrowser();
	HTREEITEM Add(CBYTE *name, HTREEITEM parent, UBYTE indent, SLONG param, SLONG img);
	int  AddDir(CBYTE *path, BOOL subdirs, HTREEITEM parent, UBYTE indent, SLONG param, SLONG img, SLONG imgfld);
	BOOL Process(HWND parent, WPARAM wParam, LPARAM lParam);
	void SetImageList(HINSTANCE inst, SLONG idx);
	void SetDraggable(DragServer *ndrag);
	void SetCallback(TreeBrowserCB cb);
	int  GetSelection();
	int	 GetImageFromItem(HTREEITEM hItem);
	void GetTextFromItem(HTREEITEM hItem, char *txt, int max);
	HTREEITEM GetItemFromOffset(HTREEITEM hItem, int ofs);
	HTREEITEM GetChildFromItem(HTREEITEM hItem, int ofs);
	void Clear();
};


class DragServer {
private:
	HCURSOR cursor;
	HWND	source,parent,target;
public:
	DragServer(HWND nparent, HINSTANCE inst);
	~DragServer();
	BOOL Process(UINT message, WPARAM wParam, LPARAM lParam);
	void Begin(HWND src);
	HWND Target() { return target; };
};


class TimeLine : public GadgetBase {
private:
	HIMAGELIST		image_list;
	int				read_head;
	int				scroll_offset;
	TimeLineRuler	*ruler;
	TimeLineScroll  *scroll;
	TimeLineCB		callback;
public:
	TimeLine(HWND nhWnd, TimeLineRuler *nrule, TimeLineScroll *nscroll);
	~TimeLine();
	GetWidth();
	Measure(LPARAM lParam);
	Add(CBYTE *str);
	Del(UWORD index);
	MarkEntry(UWORD index, UWORD start, UWORD length, UBYTE which);
	void  Draw(LPARAM lParam);
	BOOL  Process(HWND parent, WPARAM wParam, LPARAM lParam);
	void  SetImageList(HINSTANCE inst, SLONG idx);
	void  SetReadHead(int newpos);
	int   GetReadHead() { return read_head; };
	void  SetScrollPos(int newpos);
	int   GetScrollPos() { return scroll_offset; };
	int	  GetCellFromX(int x);
	int	  GetRowFromY(int y);
	int	  GetSelectedRow();
	void  SetCallback(TimeLineCB cb) { callback=cb; };
	char* GetText(int chan, char* buf);
};


class TimeLineRuler : public GadgetBase {
private:
	TimeLine	*owner;
public:
	TimeLineRuler(HWND nhWnd);
	Draw(LPARAM lParam);
	BOOL Process(HWND parent, WPARAM wParam, LPARAM lParam);
	void SetOwner(TimeLine *nown);
};

class TimeLineScroll : public GadgetBase {
private:
	TimeLine	*owner;
public:
	TimeLineScroll(HWND nhWnd);
	BOOL Process(HWND parent, WPARAM wParam, LPARAM lParam);
	void SetOwner(TimeLine *nown);
};