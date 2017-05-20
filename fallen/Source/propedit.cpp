// propedit.h
// windowsy Property Editor

#include "propedit.h"
#include <commctrl.h>
#include "resource.h"

/**********************************************************************
 *
 *    Toolkit Functions
 *
 */

//extern HWND			CUTSCENE_edit_wnd;

HMENU CreateMultiChoiceMenu(CBYTE *opts) {
	HMENU menu;
	CBYTE *pt,*buff;
	int   i=1;

	buff=(CBYTE*)malloc(strlen(opts)+1);
	strcpy(buff,opts);
//	SetWindowText(CUTSCENE_edit_wnd,opts);
	opts=buff; // now we can play with opts without destroying the original
	menu=CreatePopupMenu();
	while (pt=strchr(opts,'|')) {
		*pt=0;
		pt++;
		AppendMenu(menu,MF_STRING|MF_ENABLED,i++,opts);
		opts=pt;
	}
	AppendMenu(menu,MF_STRING|MF_ENABLED,i++,opts);
	free(buff);
	return menu;
}

/**********************************************************************
 *
 *    Base Class
 *
 */

void GadgetBase::Repaint() {
  InvalidateRect(hWnd,NULL,FALSE);
}

/**********************************************************************
 *
 *    Property Editor Class
 *
 */

//--- init, fini ---

PropertyEditor::PropertyEditor(HWND nhWnd) {
	LVCOLUMN lvc;
	SLONG mask,style;

	hWnd=nhWnd;

	lvc.mask=LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH|LVCF_ORDER;
	lvc.fmt=LVCFMT_BITMAP_ON_RIGHT|LVCFMT_RIGHT;
	lvc.cx=94;
	lvc.pszText="Value";
	lvc.iSubItem=0;
	lvc.iOrder=1;
	ListView_InsertColumn(hWnd,0,&lvc);

	lvc.mask=LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH|LVCF_ORDER;
	lvc.fmt=LVCFMT_BITMAP_ON_RIGHT|LVCFMT_LEFT;
	lvc.cx=94;
	lvc.pszText="Property";
	lvc.iSubItem=1;
	lvc.iOrder=0;
	ListView_InsertColumn(hWnd,1,&lvc);

	lvc.mask=LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH|LVCF_ORDER;
	lvc.fmt=LVCFMT_BITMAP_ON_RIGHT|LVCFMT_RIGHT;
	lvc.cx=0;
	lvc.pszText="(hidden storage)";
	lvc.iSubItem=2;
	lvc.iOrder=2;
	ListView_InsertColumn(hWnd,2,&lvc);


	mask=style=LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyleEx(hWnd,mask,style);
}

//--- property reading/info ---

int  PropertyEditor::Type(UWORD index) {
	LVITEM item;
	
	item.iItem=index;
	item.mask=LVIF_PARAM;
	ListView_GetItem(hWnd,&item);
	return item.lParam;
}

BOOL PropertyEditor::Verify(UBYTE type, CBYTE *value) {
    switch(type) {
	case PROPTYPE_STRING: return (BOOL)value;
	case PROPTYPE_INT:
		{
			if (!value||(!*value)) return false;
			int i=atoi(value);
			if (!i) { // might be really 0 or might be invalid
				if (strcmp(value,"0")) return false;
			}
			return true;
		}
	case PROPTYPE_BOOL:
		{
			if (((long)value<-1)||((long)value>1)) return false;
			return true;
		}
	default:
		return false; // we have no idea wtf this is...
	}
}


//--- property setting ---

void PropertyEditor::SetCallback(PropEditCB cb) {
	callback=cb;
}

void PropertyEditor::Clear() {
	ListView_DeleteAllItems(hWnd);
	property_count=0;
}


int PropertyEditor::Add(CBYTE *name, CBYTE *value, UBYTE type) {
	LVITEM item;

	item.iItem=property_count;
	item.iSubItem=0;
	item.pszText=value;
	item.mask=LVIF_TEXT|LVIF_PARAM;
	item.lParam=type;
	ListView_InsertItem(hWnd,&item);

	item.iItem=property_count;
	item.iSubItem=1;
	item.pszText=name;
	item.mask=LVIF_TEXT;
	ListView_SetItem(hWnd,&item);

	if (type==PROPTYPE_MULTI) { // cunningly stash the options in invisible 3rd column
		CBYTE *pt, *buff;
		buff=(CBYTE*)malloc(strlen(value)+1);
		strcpy(buff,value);
		item.iItem=property_count;
		item.iSubItem=2;
		item.pszText=value;
		item.mask=LVIF_TEXT;
		ListView_SetItem(hWnd,&item);
		pt=strchr(buff,'|');
		if (pt) *pt=0;
		ListView_SetItemText(hWnd,property_count,0,buff);
		free(buff);
	}
	
	return property_count++;
}

void PropertyEditor::Update(UWORD index, CBYTE *value) {
	LVITEM item;

	switch(Type(index)) {
	case PROPTYPE_BOOL:
		if (!value) value="off";
		if ((long)value==1) value="on";
		if ((long)value==-1) {
		  CBYTE buff[5];
		  item.iItem=index;
		  item.iSubItem=0;
		  item.pszText=buff;
		  item.cchTextMax=4;
		  item.mask=LVIF_TEXT;
		  ListView_GetItem(hWnd,&item);
		  if (!stricmp(buff,"off")) value="on"; else value="off";
		}
		break;
	}

	item.iItem=index;
	item.iSubItem=0;
	item.pszText=value;
	item.mask=LVIF_TEXT;
	ListView_SetItem(hWnd,&item);
	if (callback) callback(this, PECB_UPDATE, index, value);
}


//--- main processing ---


BOOL PropertyEditor::Process(HWND parent, WPARAM wParam, LPARAM lParam) {
	NMHDR *nm=(NMHDR*)lParam;
	NMLVODSTATECHANGE *state=(NMLVODSTATECHANGE*)lParam;
	NMLVDISPINFO *dispinfo=(NMLVDISPINFO*)lParam;
	CBYTE *txt;

	switch(nm->code) {
	case LVN_BEGINLABELEDIT:
		switch(Type(dispinfo->item.iItem)) {
		case PROPTYPE_BOOL:
		case PROPTYPE_READONLY:
		case PROPTYPE_BUTTON:
			return TRUE;
		default:
			return FALSE;
		}
		break;
	case LVN_ENDLABELEDIT:
		if (callback) callback(this, PECB_EDITMODE, 0, 0);
		txt=dispinfo->item.pszText;
		if (Verify((UBYTE)dispinfo->item.lParam,txt))
		    Update(dispinfo->item.iItem,txt);
		break;
	case NM_CLICK:
		{
			LVHITTESTINFO hti;
			LVITEM item;
			int flirble;
			POINT pt;

			GetCursorPos(&hti.pt); pt=hti.pt;
			ScreenToClient(hWnd,&hti.pt);
			item.iItem=ListView_HitTest(hWnd,&hti);
			if (item.iItem>-1) {
				item.mask=LVIF_PARAM;
				ListView_GetItem(hWnd,&item);
				switch(item.lParam) {
				case PROPTYPE_STRING:
				case PROPTYPE_INT:
					flirble=GetWindowLong(hWnd,GWL_STYLE)|LVS_EDITLABELS;
					SetWindowLong(hWnd,GWL_STYLE,flirble);
					if (callback) callback(this, PECB_EDITMODE, 1, 0);
					ListView_EditLabel(hWnd,item.iItem);
					break;
				case PROPTYPE_BOOL:
					flirble=GetWindowLong(hWnd,GWL_STYLE)&~LVS_EDITLABELS;
					SetWindowLong(hWnd,GWL_STYLE,flirble);
					Update(item.iItem,(CBYTE*)-1); // for bools, means toggle :}
					break;
				case PROPTYPE_BUTTON:
					flirble=GetWindowLong(hWnd,GWL_STYLE)&~LVS_EDITLABELS;
					SetWindowLong(hWnd,GWL_STYLE,flirble);
					if (callback) callback(this,PECB_BUTTON,item.iItem,0);
					break;
				case PROPTYPE_MULTI:
//					CBYTE buff[_MAX_PATH];
					CBYTE *buff;
					HMENU popup;
					int	   res;

					buff=(CBYTE*)malloc(10240);
					flirble=GetWindowLong(hWnd,GWL_STYLE)&~LVS_EDITLABELS;
					SetWindowLong(hWnd,GWL_STYLE,flirble);
					item.iItem=hti.iItem;
					item.iSubItem=2;
					item.mask=LVIF_TEXT;
					item.pszText=buff;
					item.cchTextMax=_MAX_PATH;
					ListView_GetItem(hWnd,&item);
					popup=CreateMultiChoiceMenu(buff);
					res=TrackPopupMenuEx(popup,
									 TPM_CENTERALIGN|TPM_VCENTERALIGN|TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTBUTTON,
									 pt.x,pt.y,
									 hWnd, NULL);								 
					if (res>0) {
						GetMenuString(popup,res,buff,_MAX_PATH,MF_BYCOMMAND);
						Update(item.iItem,buff);
					}
					free(buff);
					DestroyMenu(popup);
					break;
				}
			}
		}
		break;
	}
	return TRUE;
}


/**********************************************************************
 *
 *    Generic Tree Browser Class
 *
 */


TreeBrowser::TreeBrowser(HWND nhWnd) {
	hWnd=nhWnd;
	ZeroMemory(itemstack,sizeof(itemstack));
	drag=0;
	image_list=0;
}

TreeBrowser::~TreeBrowser() {
	SetImageList(0,0);
}

HTREEITEM TreeBrowser::Add(CBYTE *name, HTREEITEM parent, UBYTE indent, SLONG param, SLONG img) {
	TVINSERTSTRUCT is;
	HTREEITEM res;

	if ((indent>0)&&!parent) parent=itemstack[indent-1];

	is.hParent=parent;
	is.hInsertAfter=TVI_LAST;
	is.itemex.mask=TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	is.itemex.iImage=img;
	is.itemex.iSelectedImage=img;
	is.itemex.pszText=name;
	is.itemex.lParam=param;
	res=TreeView_InsertItem(hWnd,&is);

	itemstack[indent]=res;

	return res;
}

int TreeBrowser::AddDir(CBYTE *path, BOOL subdirs, HTREEITEM parent, UBYTE indent, SLONG param, SLONG img, SLONG imgfld) {
	HANDLE handle;
	BOOL res;
	WIN32_FIND_DATA data;
	CBYTE *pt;
	int count=0;

    handle = FindFirstFile(path, &data);

	res=(handle!=INVALID_HANDLE_VALUE);
	while (res) {
		if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			Add(data.cFileName,parent,indent,param++,img);
			count++;
		}
		res=FindNextFile(handle,&data);
	}
	FindClose(handle);

	if (!subdirs) return count;

	pt=strrchr(path,'\\');
	strcpy(pt,"\\*.*");

    handle = FindFirstFile(path, &data);

	res=(handle!=INVALID_HANDLE_VALUE);
	while (res) {
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)&&(data.cFileName[strlen(data.cFileName)-1]!='.')) {
			CBYTE path2[_MAX_PATH], wild[_MAX_PATH];
			int added;

			strcpy(path2,path);
			pt=strrchr(path2,'\\');
			strcpy(wild,pt); pt++;
			strcpy(pt,data.cFileName);
			strcat(path2,wild);
			Add(data.cFileName,parent,indent,-1,imgfld);
			added=AddDir(path2,true,0,indent+1,param,img,imgfld);
			param+=added; count+=added;
		} 
		res=FindNextFile(handle,&data);
	}
	FindClose(handle);

	return count;


}


//--- set properties ---

void TreeBrowser::SetImageList(HINSTANCE inst, SLONG idx) {

	if (image_list) {
		TreeView_SetImageList(hWnd,NULL,TVSIL_NORMAL);
		ImageList_Destroy(image_list);
	}

	if (!inst) return;

	//	Set up the trees image list.
	image_list	=	ImageList_LoadBitmap(
		inst,
		MAKEINTRESOURCE(idx),
		16,
		1,
		RGB (255, 0, 255)
	);

	TreeView_SetImageList(hWnd,image_list,TVSIL_NORMAL);
}

void TreeBrowser::SetDraggable(DragServer *ndrag) {
	drag=ndrag;
}

void TreeBrowser::SetCallback(TreeBrowserCB cb) {
	callback=cb;
}

int  TreeBrowser::GetSelection() {
	return selection;
}

//--- get properties ---

void TreeBrowser::Clear() {
	TreeView_DeleteAllItems(hWnd);
}

void TreeBrowser::GetTextFromItem(HTREEITEM hItem, char *txt, int max) {
	TVITEM item;

	if (!hItem) hItem=TreeView_GetSelection(hWnd);

	item.mask	= TVIF_TEXT;
	item.hItem	= hItem;
	item.pszText= txt;
	item.cchTextMax = max;
	TreeView_GetItem(hWnd, &item);
}

int	 TreeBrowser::GetImageFromItem(HTREEITEM hItem) {
	TVITEM item;

	if (!hItem) hItem=TreeView_GetSelection(hWnd);

	item.mask	= TVIF_IMAGE;
	item.hItem	= hItem;
	TreeView_GetItem(hWnd, &item);
	return item.iImage;

}

HTREEITEM TreeBrowser::GetItemFromOffset(HTREEITEM hItem, int ofs) {
	if (!ofs) return hItem;
	if (ofs>0) {
		while (ofs--) hItem=TreeView_GetNextVisible(hWnd,hItem);
	} else {
		while (ofs++) hItem=TreeView_GetPrevVisible(hWnd,hItem);
	}
	return hItem;
}

HTREEITEM TreeBrowser::GetChildFromItem(HTREEITEM hItem, int ofs) {
	HTREEITEM test;
	int depth=0;

	hItem=TreeView_GetChild(hWnd,hItem);
	while (ofs--) {
		test=TreeView_GetChild(hWnd,hItem);
		if (test) 
			depth++;
		else
			test=TreeView_GetNextSibling(hWnd,hItem);
		while (depth&&!test) {
		  depth--;
		  hItem=TreeView_GetParent(hWnd,hItem);
		  test=TreeView_GetNextSibling(hWnd,hItem);
		  if (test) hItem=test;
		}
		if (test) hItem=test;
/*		if (test)
			hItem=test;
		else {
			if (depth) {
				depth--;
				hItem=TreeView_GetParent(hWnd,hItem);
				test=TreeView_GetNextSibling(hWnd,hItem);
			}
		}*/
		test=TreeView_GetChild(hWnd,hItem);
		if (test) ofs++;
	}

	return hItem;
}


//--- process messages ---

BOOL TreeBrowser::Process(HWND parent, WPARAM wParam, LPARAM lParam) {
	char msg[128];

	LPNMTREEVIEW nm=(LPNMTREEVIEW)lParam;
	switch(nm->hdr.code) {
	case TVN_BEGINDRAG:
		if (drag) {
			if (callback) {
				GetTextFromItem(nm->itemNew.hItem,msg,128);
				if (!callback(this,TBCB_DRAG,nm->itemNew.lParam,nm->itemNew.hItem,msg)) return FALSE;
			}
			drag_item=nm->itemNew;
			drag_item.iImage=GetImageFromItem(nm->itemNew.hItem); // b'cos the provided one doesn't give it. gits.
			drag->Begin(hWnd);
		}
		break;
	case TVN_SELCHANGED:
		selection=nm->itemNew.lParam;
		break;
	case NM_DBLCLK:
		GetTextFromItem(0,msg,128);
		if (callback) callback(this,TBCB_DBLCLK,selection,TreeView_GetSelection(hWnd),msg);
		break;
	}
	return FALSE;
}

/**********************************************************************
 *
 *    Drag-n-Drop handler
 *
 */

DragServer::DragServer(HWND nparent, HINSTANCE inst) {
	source=target=0; 
	parent=nparent;
	cursor=(HCURSOR)LoadImage(inst,MAKEINTRESOURCE(IDC_POINTER_DRAGDROP),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE);
}

DragServer::~DragServer() {
	DestroyCursor(cursor);
}


void DragServer::Begin(HWND src) {
	source=src; target=0;
	SetCursor(cursor);
	SetCapture(parent);
}

BOOL DragServer::Process(UINT message, WPARAM wParam, LPARAM lParam) {
	POINT pt;

	switch (message) {
	case WM_MOUSEMOVE:
		if (source) {
			SetCursor(cursor);
			return TRUE;
		}
		break;
	case WM_LBUTTONUP:
		pt.x=LOWORD(lParam);
		pt.y=HIWORD(lParam);
		target=ChildWindowFromPoint(parent,pt);
		ReleaseCapture();
		SendMessage(parent,UM_DROP,(WPARAM)source,lParam);
		source=0;
		break;
	}
	return FALSE;
}


/**********************************************************************
 *
 *    Time Line
 *
 */

struct TLEntry {
	CBYTE	title[50];
	CBYTE	marks[2000];
};

TimeLine::TimeLine(HWND nhWnd, TimeLineRuler *nrule, TimeLineScroll *nscroll) {
	hWnd=nhWnd;
	read_head=0;
	scroll_offset=0;
	ruler=nrule;
	if (ruler) ruler->SetOwner(this);
	scroll=nscroll;
	if (scroll) scroll->SetOwner(this);
	callback=NULL;
}

TimeLine::~TimeLine() {
	SLONG c0,ctr=SendMessage(hWnd,LB_GETCOUNT,0,0);
	for (c0=ctr;c0;) {
		c0--;
		Del(c0);
	}
	SetImageList(0,0);
}


BOOL TimeLine::Process(HWND parent, WPARAM wParam, LPARAM lParam) {
	POINT pt;

	switch (HIWORD(wParam)) {
	case LBN_SELCHANGE:
		GetCursorPos(&pt);
		ScreenToClient(hWnd,&pt);
		if (pt.x>100) {
		  SetReadHead(GetCellFromX(pt.x-100)/*-scroll_offset*/);
		}
		if (callback) callback(this, TLCB_SELECT, SendMessage(hWnd,LB_GETCURSEL,0,0), 1, read_head);
		return 0;
	}
	return 1;
}


TimeLine::Measure(LPARAM lParam) {
	LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;

	mis->itemHeight=16; mis->itemWidth=GetWidth();
}

void TimeLine::SetImageList(HINSTANCE inst, SLONG idx) {
	
	if (image_list) ImageList_Destroy(image_list);

	if (!inst) return;

	//	Set up the image list.
	image_list	=	ImageList_LoadBitmap(
		inst,
		MAKEINTRESOURCE(idx),
		16,
		1,
		RGB (255, 0, 255)
	);

}

void TimeLine::Draw(LPARAM lParam) {
	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
	HBRUSH brs;
//	CBYTE txt[_MAX_PATH];
	TLEntry *entry=0;
	CBYTE *txt;
	SLONG len, c0;
	SLONG rgb, oldrgb, oldalign, draw_read_head, rgbmod=0;
	RECT rc;
	UBYTE which;

	SendMessage(hWnd,LB_GETTEXT,dis->itemID,(LPARAM)&entry);
	if (!entry) return;
	txt=entry->title;
	len=strlen(txt);

	if (dis->itemState&ODS_SELECTED) rgbmod=RGB(20,20,0);

	// left-hand border
	
	rgb=(dis->itemID&1) ? RGB(235,235,235) : RGB(225,225,225);
	brs=CreateSolidBrush( rgb-rgbmod );

	rc=dis->rcItem; rc.right=100;
	FillRect(dis->hDC,&rc,brs);

	// LH border text

	oldrgb=SetBkColor(dis->hDC,rgb-rgbmod);
	oldalign=SetTextAlign(dis->hDC,TA_RIGHT);

	TextOut(dis->hDC,dis->rcItem.left+96,dis->rcItem.top+1,txt+1,len-1);
	if (image_list&&(txt[0]!=0xff))
		ImageList_Draw(image_list,txt[0],dis->hDC,dis->rcItem.left,dis->rcItem.top+1,ILD_NORMAL);

	SetTextAlign(dis->hDC,oldalign);

	SetBkColor(dis->hDC,oldrgb);

	DeleteObject(brs);

	// main "stripe"

	rgb=(dis->itemID&1) ? RGB(255,255,255) : RGB(245,245,245);
	brs=CreateSolidBrush( rgb-rgbmod );
	rc.left=100; rc.right=dis->rcItem.right;
	FillRect(dis->hDC,&rc,brs);
	DeleteObject(brs);

	// read-head blob

	draw_read_head=read_head-scroll_offset;
	if (draw_read_head>=0) {
		draw_read_head=(draw_read_head*16)+100;
		rgb=(dis->itemID&1) ? RGB(205,205,205) : RGB(195,195,195);
		brs=CreateSolidBrush( rgb-rgbmod );
		rc.left=draw_read_head; rc.right=draw_read_head+16;
		FillRect(dis->hDC,&rc,brs);
		DeleteObject(brs);
	}

	// cell contents
	
	draw_read_head=100-scroll_offset*16;
	for (c0=0;c0<2000;c0++) {
		if ((draw_read_head>=100)&&(entry->marks[c0])) {
			rc.left=draw_read_head; rc.right=draw_read_head+16;
			if (entry->marks[c0]&3) {
			  which=(entry->marks[c0]&1)?1:2;
			  MoveToEx(dis->hDC,rc.left,rc.top+(which*8)-4,NULL);
			  LineTo(dis->hDC,rc.right,rc.top+(which*8)-4);
			  if (entry->marks[c0]&128) { // start
				MoveToEx(dis->hDC,rc.left,rc.top+(which*8)-8,NULL);
				LineTo(dis->hDC,rc.left,rc.top+(which*8));
			  }
			  if (entry->marks[c0]&64) {  // end
				MoveToEx(dis->hDC,rc.right-4,rc.top+(which*8)-8,NULL);
				LineTo(dis->hDC,rc.right,rc.top+(which*8)-4);
				LineTo(dis->hDC,rc.right-4,rc.top+(which*8));
			  }
			} else {
			  if ((entry->marks[c0]&4)&&image_list&&(txt[0]!=0xff))
				ImageList_Draw(image_list,txt[0],dis->hDC,rc.left,rc.top+1,ILD_NORMAL);
			}
		}
		draw_read_head+=16;
	}

}

TimeLine::GetWidth() {
	RECT rc;
	GetClientRect(hWnd, &rc);
	return rc.right-rc.left;
}

char* TimeLine::GetText(int chan, char *buf) {
	TLEntry* item;
	SendMessage(hWnd,LB_GETTEXT,chan,(LPARAM)&item);
	strcpy(buf,item->title+1);
	return buf;
}

TimeLine::Add(CBYTE *str) {
	TLEntry* entry = new TLEntry;
	ZeroMemory(entry,sizeof(TLEntry));
	strncpy(entry->title,str,50);
	SendMessage(hWnd,LB_ADDSTRING,0,(LPARAM)entry);
}

TimeLine::Del(UWORD index) {
	TLEntry *entry;

	SendMessage(hWnd,LB_GETTEXT,index,(LPARAM)&entry);
	delete entry;
	SendMessage(hWnd,LB_DELETESTRING,index,0);

}

TimeLine::MarkEntry(UWORD index, UWORD start, UWORD length, UBYTE which) {
	TLEntry *entry;
	SLONG c0;
	UBYTE v;

	SendMessage(hWnd,LB_GETTEXT,index,(LPARAM)&entry);
	if ((start>0)&&(entry->marks[start-1])) entry->marks[start-1]|=64;
	for (c0=start;c0<(start+length);c0++) {
		v=which|(entry->marks[c0]&7);
		if (c0==start) v|=128;
		if (c0==start+length-1) v|=64;
		if (which==0xff) v=0; // blank it out
		entry->marks[c0]=v;
	}
	if ((start+length<2000)&&(entry->marks[start+length])) entry->marks[start+length]|=128;
}

void TimeLine::SetReadHead(int newpos) {
	if (read_head!=newpos) {
		read_head=newpos;
		InvalidateRect(hWnd,NULL,false);
		if (ruler) InvalidateRect(ruler->hWnd,NULL,false);
	}
	if (callback) callback(this, TLCB_SELECT, SendMessage(hWnd,LB_GETCURSEL,0,0), 1, newpos);
}

int	 TimeLine::GetSelectedRow() {
	return SendMessage(hWnd,LB_GETCURSEL,0,0);
}

void TimeLine::SetScrollPos(int newpos) {
	if (scroll_offset!=newpos) {
		scroll_offset=newpos;
		InvalidateRect(hWnd,NULL,false);
		if (ruler) InvalidateRect(ruler->hWnd,NULL,false);
	}
}

int	 TimeLine::GetCellFromX(int x) {
	return (x/16)+scroll_offset;
}

int	 TimeLine::GetRowFromY(int y) {
	return (y/16);
}

/**********************************************************************
 *
 *    Time Line Ruler
 *
 */

TimeLineRuler::TimeLineRuler(HWND nhWnd) {
	hWnd=nhWnd;
	owner=0;
}

TimeLineRuler::Draw(LPARAM lParam) {
	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
	SLONG rgb;
	HBRUSH brs;
	RECT rc;
	SLONG c0;
	SLONG tick=0;
	SWORD dhead=0, rhead=0;

	if (owner) {
		rhead=owner->GetReadHead();
		dhead=owner->GetScrollPos();
		tick=dhead; // ??
	}

	rgb=RGB(215,215,215);
	brs=CreateSolidBrush( rgb );

	rc=dis->rcItem; rc.right=102;
	FillRect(dis->hDC,&rc,brs);

	DeleteObject(brs);

	for (c0=102;c0<dis->rcItem.right;c0+=16) {
		tick++;
		rgb=(tick&1) ? RGB(225,225,225) : RGB(215,215,215);
		if (dhead==rhead) rgb-=0x002020;
		brs=CreateSolidBrush( rgb );
		rc.left=c0; rc.right=c0+16;
		FillRect(dis->hDC,&rc,brs);
		if ((tick%20)==1) {
			SLONG orgb=SetBkColor(dis->hDC,rgb);
			CBYTE msg[10];
			itoa(tick/20,msg,10);
			TextOut(dis->hDC,rc.left,rc.top,msg,strlen(msg));

			SetBkColor(dis->hDC,orgb);
		}
		DeleteObject(brs);
		dhead++;
	}

}

BOOL TimeLineRuler::Process(HWND parent, WPARAM wParam, LPARAM lParam) {
	POINT pt;

	switch(HIWORD(wParam)) {
	case BN_CLICKED:
		GetCursorPos(&pt);
		ScreenToClient(hWnd,&pt);
		if (owner) owner->SetReadHead(owner->GetCellFromX(pt.x-100));

		break;
	}
	return 0;
}

void TimeLineRuler::SetOwner(TimeLine *nown) {
	owner=nown;
}

/**********************************************************************
 *
 *    Time Line Scroller
 *
 */

TimeLineScroll::TimeLineScroll(HWND nhWnd) {
	hWnd=nhWnd;
	SendMessage(hWnd,SBM_SETRANGE,0,100*20); // just under two minute's worth
}

void TimeLineScroll::SetOwner(TimeLine *nown) {
	owner=nown;
}

BOOL TimeLineScroll::Process(HWND parent, WPARAM wParam, LPARAM lParam) {
	short int nPos = (short int)HIWORD(wParam);
	int oldpos = GetScrollPos(hWnd,SB_CTL);
	int inc=0;

	if ((HWND)lParam!=hWnd) return false;
	if (!owner) return false;
	switch (LOWORD(wParam)) {
	case SB_LINELEFT:
		inc=-1;
		break;	
	case SB_LINERIGHT:
		inc= 1;
		break;	
	case SB_PAGELEFT:
		inc= -30;
		break;
	case SB_PAGERIGHT:
		inc=  30;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		oldpos=nPos;
		break;
	}
	oldpos+=inc;
	if (oldpos<0) oldpos=0;
	SetScrollPos(hWnd,SB_CTL,oldpos,TRUE);
	owner->SetScrollPos(oldpos);
	return false;
}
