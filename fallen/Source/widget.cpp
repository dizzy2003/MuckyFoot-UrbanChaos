//
// widget.cpp
// Matthew Rosenfeld	24 February 1999
//
// widget provides windows-esque controls such as buttons, checkboxes and stuff
// grouped together into 'forms' (palmpilot style)
//

#include "widget.h"
#include "menufont.h"
#include "drawxtra.h"
#include "sound.h"
#include "MFx.h"
#include    "C:\fallen\DDLibrary\headers\D3DTexture.h"
#include    "C:\fallen\DDLibrary\headers\GDisplay.h"
#include "interfac.h"

#define RECT_LEFT	1
#define RECT_RIGHT	2
#define RECT_TOP 	4
#define RECT_BOTTOM	8

#define	WIF_EDIT	1
#define	WIF_PASS	2

extern UBYTE InkeyToAscii[];
extern UBYTE InkeyToAsciiShift[];

static SLONG WidgetTick;
static SLONG EatenKey;

struct ListEntry {
	CBYTE		*text;
	ListEntry	*next,*prev;
};

/*
		Data for the various types:
		Button	--	data[0] is shrink flag?
		Static	--	same
		Check	--	data[0] is tick state
		Radio	--	as Check, plus data[1] is 'group id'
		Input	--	data[0] is pointer to text (while in edit mode)
					data[1] is edit mode state
					data[2] is caret position
					data[3] is scroll position
		Lists	--	data[0] is pointer to string list head
					data[1]	is keyboard selection mode state
					data[2] is pointer to active item (or 0)
					data[3] is vertical scroll state
					data[4]	is index to active item
		Texts	--	data[0] is pointer to string list head
					data[1] is vertical scroll state
					data[2] is lines count
					data[3] is page length count
		Glyph	--  data[0] is poly_page
					data[1]-data[4] is uvs
		Shade	--	data[0]	is fade-in rate
					data[1] is fade-in pos
					data[2] is rgb
					data[3] is add/sub flag

 */

//---------------------------------------------------------------------------------
// Section The First:  Method Prototypes
//

//void BUTTON_Init(Widget *widget);
void	BUTTON_Free(Widget *widget);
void	BUTTON_Draw(Widget *widget);
void	BUTTON_Push(Widget *widget);
BOOL	BUTTON_HitTest(Widget *widget, SLONG x, SLONG y);

void	STATIC_Init(Widget *widget); 

void	CHECK_Draw (Widget *widget);
void	CHECK_Push (Widget *widget);
void	RADIO_Push (Widget *widget);

void	INPUT_Init (Widget *widget); 
void	INPUT_Free (Widget *widget);
void	INPUT_Draw (Widget *widget);
BOOL	INPUT_Char (Widget *widget, CBYTE key);
SLONG	INPUT_Data (Widget *widget, SLONG code, SLONG data1, SLONG data2);

void	LISTS_Free (Widget *widget);
void	LISTS_Draw (Widget *widget);
BOOL	LISTS_Char (Widget *widget, CBYTE key);
SLONG	LISTS_Data (Widget *widget, SLONG code, SLONG data1, SLONG data2);
void	LISTS_Push (Widget *widget);

void	TEXTS_Init (Widget *widget); 
void	TEXTS_Free (Widget *widget);
void	TEXTS_Draw (Widget *widget);
BOOL	TEXTS_Char (Widget *widget, CBYTE key);
SLONG	TEXTS_Data (Widget *widget, SLONG code, SLONG data1, SLONG data2);

void	GLYPH_Draw (Widget *widget);

void	SHADE_Draw (Widget *widget);

//---------------------------------------------------------------------------------
// Section The Second:  Widget Class Definitions
//

Methods BUTTON_Methods = { 0,			BUTTON_Free, BUTTON_Draw, BUTTON_Push, 0,	0,			BUTTON_HitTest };
Methods STATIC_Methods = { STATIC_Init, BUTTON_Free, BUTTON_Draw, BUTTON_Push, 0,	0,			BUTTON_HitTest };
Methods CHECK_Methods  = { 0,           BUTTON_Free, CHECK_Draw,  CHECK_Push,  0,	0,			BUTTON_HitTest };
Methods RADIO_Methods  = { 0,           BUTTON_Free, CHECK_Draw,  RADIO_Push,  0,	0,			BUTTON_HitTest };
Methods INPUT_Methods  = { INPUT_Init,  INPUT_Free,  INPUT_Draw,  0,  INPUT_Char,	INPUT_Data,	BUTTON_HitTest };
Methods LISTS_Methods  = { 0,           LISTS_Free,  LISTS_Draw,  LISTS_Push,  LISTS_Char,	LISTS_Data,	BUTTON_HitTest };
Methods TEXTS_Methods  = { TEXTS_Init,  TEXTS_Free,  TEXTS_Draw,  0,  TEXTS_Char,	TEXTS_Data,	BUTTON_HitTest };
Methods GLYPH_Methods  = { 0,           BUTTON_Free, GLYPH_Draw,  BUTTON_Push, 0,	0,			BUTTON_HitTest };
Methods SHADE_Methods  = { 0,           BUTTON_Free, SHADE_Draw,  0, 0,	0,			0 };


//---------------------------------------------------------------------------------
// Section The Third:  Utility & "base class" Methods
//

#define _WS_MOVE	S_MENU_START
#define _WS_FADEIN	S_MENU_START+2
#define _WS_FADEOUT	S_MENU_START+1
#define	_WS_OK		S_MENU_END
#define _WS_FAIL	S_MENU_END-1
#define _WS_BLIP	S_MENU_START+3

void WIDGET_snd(SLONG snd) {
	switch(snd) {
	case WS_MOVE:		snd=_WS_MOVE;		break;
	case WS_FADEOUT:	snd=_WS_FADEOUT;	break;
	case WS_FADEIN:		snd=_WS_FADEIN;		break;
	case WS_OK:			snd=_WS_OK;			break;
	case WS_FAIL:		snd=_WS_FAIL;		break;
	case WS_BLIP:		snd=_WS_BLIP;		break;
	}
//	play_ambient_wave(snd,0,0,256,1);
	MFX_play_ambient(0,snd,MFX_REPLACE);
}

inline SLONG	ShiftAlpha(ULONG rgba, SBYTE shift) {
	return (rgba&0xffffff)|((rgba>>shift)&0xFF000000);
}

inline SLONG	AlterAlpha(ULONG rgba, ULONG alpha) {
	return (rgba&0xffffff)|((alpha&0xff)<<24);
}

void	WIDGET_Rect(SLONG x, SLONG y, SLONG ox, SLONG oy, ULONG rgb, UBYTE inverse=0, UBYTE flags=RECT_LEFT|RECT_RIGHT|RECT_BOTTOM|RECT_TOP) {

	if (flags&RECT_TOP) {
		DRAW2D_Box(x+3,y,ox-3,y+3,rgb,inverse);
		DRAW2D_Tri(x,y+3,x+3,y,x+3,y+3,rgb,inverse);
		DRAW2D_Tri(ox-3,y,ox-3,y+3,ox,y+3,rgb,inverse);
	}
	if (flags&RECT_BOTTOM) {
		DRAW2D_Box(x+3,oy-3,ox-3,oy,rgb,inverse);
		DRAW2D_Tri(x,oy-3,x+3,oy,x+3,oy-3,rgb,inverse);
		DRAW2D_Tri(ox-3,oy-3,ox-3,oy,ox,oy-3,rgb,inverse);
	}
	if (flags&RECT_LEFT) {
		DRAW2D_Box(x,y+3,x+3,oy-3,rgb,inverse);
	}
	if (flags&RECT_RIGHT) {
		DRAW2D_Box(ox-3,y+3,ox,oy-3,rgb,inverse);
	}



/*	SLONG dx,dy,sx,sy,lx,ly,dummy;
	dx=ox-x; dy=oy-y;
	MENUFONT_Dimensions("_",sx,sy); sx-=1; sy-=1; // borders
	dy-=sy;
	MENUFONT_Dimensions("|",dummy,sy); sy-=1;
	lx=dx/sx;
	lx-=2;
	ly=dy/sy;
	oy=ly*sy;
	MENUFONT_Draw(x,y,256,"{",rgb,0);
	MENUFONT_Draw(x,oy,256,"[",rgb,0);
	dx=x;
	while (lx--) {
		dx+=sx;
		MENUFONT_Draw(dx,y,256,"_",rgb,0);
		MENUFONT_Draw(dx,oy,256,"_",rgb,0);
	}
	dx+=sx;
	MENUFONT_Draw(dx,y,256,"}",rgb,0);
	MENUFONT_Draw(dx,oy,256,"]",rgb,0);*/
}

void	WIDGET_Free(Widget *widget) {
	if (widget->caption) MemFree(widget->caption);
	if (widget->form) FORM_DelWidget(widget);
	MemFree(widget);
}

//---------------------------------------------------------------------------------
// Section The Fourth:  Methods
//

// -- buttons --
/*
void BUTTON_Init(Widget *widget) {
}
*/
void BUTTON_Free(Widget *widget) {
	WIDGET_Free(widget);
}

void BUTTON_Draw(Widget *widget) {
	SLONG flags=0, localctr=widget->form->age*8;
	ULONG rgb=widget->form->textcolour;
	WidgetPoint wp = WIDGET_Centre(widget);
	UBYTE shift = widget->data[0];

	if (!widget->caption) return;

//	flags|=(widget->state&WIDGET_STATE_FOCUS) ? MENUFONT_GLIMMER : 0;
//	flags|=(widget->state&WIDGET_STATE_FOCUS) ? MENUFONT_SHAKE : 0;

	if (widget->state&WIDGET_STATE_FOCUS) rgb=AlterAlpha(rgb,0xff);
	if (widget->state&WIDGET_STATE_DISABLED) rgb=((rgb>>1)&0xFF000000) | (rgb&0xFFFFFF);
	if (!(widget->state&WIDGET_STATE_ALIGNLEFT))
		flags|=MENUFONT_CENTRED;
	else
		wp.x=widget->x+widget->form->x;

	if (localctr<0x1ff) {
		flags |= MENUFONT_HSCALEONLY;
		if (localctr<0xff) {
			MENUFONT_Draw_floats(wp.x,wp.y,localctr>>shift,widget->caption,AlterAlpha(rgb,localctr),flags);
			return;
		} else {
			MENUFONT_Draw_floats(wp.x,wp.y,localctr>>shift,widget->caption,AlterAlpha(rgb,(0x1ff-localctr)>>1),flags);
		}
	}

	MENUFONT_Draw(wp.x,wp.y,256>>shift,(CBYTE*)widget->caption,rgb,flags);
}

void BUTTON_Push(Widget *widget) {
	// buttons don't do anything except notify when pushed
	if (widget->form->proc) widget->form->proc(widget->form,widget,WBN_PUSH);
}

BOOL BUTTON_HitTest(Widget *widget, SLONG x, SLONG y) {
	WidgetPoint pt;

    pt = FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->x,widget->y));
	if ((x<pt.x)||(y<pt.y)) return 0;
    pt = FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->ox,widget->oy));
	if ((x>pt.x)||(y>pt.y)) return 0;
	return 1;
}

// -- statics --

void STATIC_Init(Widget *widget) {
	widget->state|=WIDGET_STATE_BLOCKFOCUS|WIDGET_STATE_ALIGNLEFT;
}

// --  check boxes & radio buttons --

void CHECK_Draw(Widget *widget) {
	SLONG flags=0;
	ULONG rgb=widget->form->textcolour;
	WidgetPoint pt,wp = WIDGET_Centre(widget);
	CBYTE check[2] = {0,0};

	MENUFONT_Dimensions("°",pt.x,pt.y);

	wp.x=widget->x+pt.x+6+widget->form->x;

	if (!widget->caption) return;

	flags|=(widget->state&WIDGET_STATE_FOCUS) ? MENUFONT_GLIMMER : 0;
	if (widget->state&WIDGET_STATE_DISABLED) rgb=((rgb>>1)&0xFF000000) | (rgb&0xFFFFFF);
	if (widget->methods->Push==CHECK_Push)
		check[0] = (widget->data[0] ? '÷' : '°');
	else
		check[0] = (widget->data[0] ? '·' : '°');
	MENUFONT_Draw(widget->x+widget->form->x,wp.y,256,check,rgb,flags|MENUFONT_SUPER_YCTR);
	MENUFONT_Draw(wp.x,wp.y,256,(CBYTE*)widget->caption,rgb,flags);
}

void CHECK_Push(Widget *widget) {
	widget->data[0]^=1;
	if (widget->form->proc) widget->form->proc(widget->form,widget,WBN_PUSH);
}

void RADIO_Push(Widget *widget) {
	Widget *walk;

	widget->data[0]=1;
	walk=widget->form->children;
	while (walk) {
		if ((walk!=widget)&&(walk->methods->Push==RADIO_Push)&&(walk->data[1]==widget->data[1]))
			walk->data[0]=0;
		walk=walk->next;
	}
	if (widget->form->proc) widget->form->proc(widget->form,widget,WBN_PUSH);
}

// --  input boxes --

void INPUT_Init(Widget *widget) {
	widget->state|=WIDGET_STATE_ALIGNLEFT;
}

void INPUT_Draw (Widget *widget) {
	WidgetPoint pt,pt2;
	SLONG flags=0, len, xd, yd, w;
	ULONG rgb=widget->form->textcolour;
	CBYTE *str=0;

	pt=FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->x,widget->y));
	pt2=FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->ox,widget->oy));

/*	//debug
	if (widget->data[1])
		MENUFONT_Draw(widget->form->x+10,widget->form->oy-40,128,(CBYTE*)widget->data[0],widget->form->textcolour,0);*/

	if (widget->data[1]&WIF_EDIT) {
		SLONG dummy,caret=0;
		if (widget->data[0]) {
			str=(CBYTE*)(widget->data[0]+widget->data[3]);
			len=strlen(str);
			if (widget->data[1] & WIF_PASS) {
				str=(CBYTE*)MemAlloc(len+1);
				memset(str,'?',len);
				*(str+len)=0;
			}
			MENUFONT_Dimensions(str,caret,dummy,widget->data[2]-widget->data[3]);
		}
		dummy=(WidgetTick<<1)&511;
		if (dummy>255) dummy=511-dummy;
		DRAW2D_Box(pt.x+caret+5,pt.y+4,pt.x+caret+7,pt2.y-4,(rgb&0xFFFFFF)|(dummy<<24),widget->form->inverse);
		WIDGET_Rect(pt.x,pt.y,pt2.x,pt2.y,ShiftAlpha(widget->form->textcolour,2),widget->form->inverse);
	} else {
		flags|=(widget->state&WIDGET_STATE_FOCUS) ? MENUFONT_GLIMMER : 0;
		str=widget->caption;
		len=strlen(str);
		if (widget->data[1] & WIF_PASS) {
			str=(CBYTE*)MemAlloc(len+1);
			memset(str,'?',len);
			*(str+len)=0;
		}
	}

	if (!str) return;

	if (widget->state&WIDGET_STATE_DISABLED) rgb=((rgb>>1)&0xFF000000) | (rgb&0xFFFFFF);
	if (!(widget->state&WIDGET_STATE_ALIGNLEFT)) flags|=MENUFONT_CENTRED;


	MENUFONT_Dimensions(str,xd,yd);
	if (xd>(pt2.x-pt.x)-6) { // need to clip
		len=MENUFONT_CharFit(str,(widget->ox-widget->x)-6);
		MENUFONT_Draw(pt.x+6,pt.y+((widget->oy-widget->y)>>1),256,str,rgb,flags,len);
	} else
		MENUFONT_Draw(pt.x+6,pt.y+((widget->oy-widget->y)>>1),256,str,rgb,flags);
	if (widget->data[1] & WIF_PASS) MemFree(str);
	
}

void INPUT_caretchk(Widget *widget) {
	if (widget->data[2]<widget->data[3]) {
		while (widget->data[2]<widget->data[3])
			widget->data[3]-=5;
		if (widget->data[3]<0) widget->data[3]=0;
	} else {
		ULONG ctr=MENUFONT_CharFit((CBYTE*)(widget->data[0]+widget->data[3]),(widget->ox-widget->x)-6);
		if (ctr<strlen((CBYTE*)(widget->data[0]+widget->data[3])))
			while (widget->data[2]-widget->data[3]>ctr) widget->data[3]++;
	}
}

BOOL INPUT_Char (Widget *widget, CBYTE key) {
	if (widget->state&WIDGET_STATE_DISABLED) return 0;
	if (key==13) { // tested seperately -- only key that works in -both- modes
		widget->data[1]^=WIF_EDIT;
		if (widget->data[1] & WIF_EDIT) {
			SLONG sz=strlen(widget->caption)+1;
			if (sz<256) sz=256;
			widget->data[0]=(SLONG)MemAlloc(sz);
			strcpy((CBYTE*)widget->data[0],widget->caption);
			widget->data[2]=strlen(widget->caption);
		} else {
			MemFree(widget->caption);
			widget->caption=(CBYTE*)MemAlloc(strlen((CBYTE*)widget->data[0])+1);
			strcpy(widget->caption,(CBYTE*)widget->data[0]);
			MemFree((CBYTE*)widget->data[0]);
			widget->data[0]=0;
			if (widget->form->proc) widget->form->proc(widget->form,widget,WIN_ENTER);
		}
	}
	if (!(widget->data[1]&WIF_EDIT)) return 0;
	switch(key) {
	case 8:
		if (widget->data[2]) {
			widget->data[2]--;
			WIDGET_snd(WS_MOVE);
		} else WIDGET_snd(WS_FAIL);
		break;
	case 9:
		if ((unsigned)(widget->data[2])<strlen((CBYTE*)widget->data[0])) {
			widget->data[2]++;
			WIDGET_snd(WS_MOVE);
		} else WIDGET_snd(WS_FAIL);
		break;
	case 3:
		widget->data[2]=0;
		WIDGET_snd(WS_MOVE);
		break;
	case 4:
		widget->data[2]=strlen((CBYTE*)widget->data[0]);
		WIDGET_snd(WS_MOVE);
		break;
	case 27:
		MemFree((CBYTE*)widget->data[0]);
		widget->data[1]&=~WIF_EDIT;
		widget->data[0]=0;
		WIDGET_snd(WS_FAIL);
		break;
	case 127:
		{
			CBYTE *str=(CBYTE*)widget->data[0];
			SLONG len=strlen(str);
			if (widget->data[2]<1) { WIDGET_snd(WS_FAIL); break; }
			WIDGET_snd(WS_MOVE);
			memmove(str+widget->data[2]-1,str+widget->data[2],(len+1)-widget->data[2]);
			widget->data[2]--;
			
		}
		break;
	case 5:
		{
			CBYTE *str=(CBYTE*)widget->data[0];
			SLONG len=strlen(str);
			if (widget->data[2]>=len) { WIDGET_snd(WS_FAIL); break; }
			WIDGET_snd(WS_MOVE);
			memmove(str+widget->data[2],str+widget->data[2]+1,(len+1)-widget->data[2]);
			
		}
		break;
	default:
//		if ( ((key>='A')&&(key<='Z')) || ((key>='a')&&(key<='z')) || ((key>='0')&&(key<='9')) || (key==' '))
		if ( MENUFONT_CharWidth(key) )
		{
			CBYTE *str=(CBYTE*)widget->data[0];
			SLONG len;
			len=strlen(str);
			if (len<255)
			{
				if (widget->data[2]==len) {
					*(str+widget->data[2]+1)=0; 
				} else {
					memmove(str+widget->data[2]+1,str+widget->data[2],(len+1)-widget->data[2]);
				}
				*(str+widget->data[2])=key;
				widget->data[2]++;
				WIDGET_snd(WS_BLIP);
			} else WIDGET_snd(WS_FAIL);
		}
		break;
	}
	if (widget->data[1] & WIF_EDIT) INPUT_caretchk(widget);
	return widget->data[1] & WIF_EDIT;
}

SLONG INPUT_Data(Widget *widget, SLONG code, SLONG data1, SLONG data2) {
	ListEntry *item, *item2;

	switch(code) {
	case WIM_SETSTRING:
		if (widget->data[1] & WIF_EDIT)
			strcpy((CBYTE*)widget->data[0],(CBYTE*)data2);
		else
			strcpy(widget->caption,(CBYTE*)data2);
		break;
	case WIM_SETMODE:
		if (data1) widget->data[1]|=WIF_PASS; else widget->data[1]&=~WIF_PASS;
		break;
	}
	return 0;
}

void INPUT_Free(Widget *widget) {
	if (widget->data[0]) MemFree((CBYTE*)widget->data[0]);
	WIDGET_Free(widget);
}

// --  text scrolly boxes --

void TEXTS_Init(Widget *widget) {
	SLONG xs, ys;

	MENUFONT_Dimensions("M",xs,ys); ys>>=1;
	widget->data[3]=(widget->oy-widget->y)/ys;
}

void TEXTS_Free(Widget *widget) {
	ListEntry *item;
	while (widget->data[0]) {
		item=(ListEntry*)widget->data[0];
		widget->data[0]=(SLONG)item->next;
		MemFree(item);
	}
	WIDGET_Free(widget);
}

void TEXTS_Draw (Widget *widget) {
	WidgetPoint pt,pt2,pt3, ctr=WIDGET_Centre(widget);
	ListEntry *item, *item2;
	SLONG flags,y,ys,yctr=0;
	UBYTE shift = widget->state&WIDGET_STATE_FOCUS ? 0 : 1;
	ULONG rgb, localctr=widget->form->age*6;
	SLONG subctr;
	SWORD alpha, offset;

	pt =FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->x,widget->y));
	pt2=FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->ox,widget->oy));
	rgb=ShiftAlpha(widget->form->textcolour,shift);

	if (localctr<0xff) {
		offset=255-localctr;
		alpha=(rgb>>24)-offset;
		if (alpha<0) alpha=0;
		rgb=AlterAlpha(rgb,alpha);
		pt.x+=offset;	 pt.y+=offset;
		pt2.x-=offset; pt2.y-=offset;
		if (pt.x>ctr.x) pt.x=ctr.x;
		if (pt.y>ctr.y) pt.y=ctr.y;
		if (pt2.x<ctr.x) pt2.x=ctr.x;
		if (pt2.y<ctr.y) pt2.y=ctr.y;
	}
//	WIDGET_Rect(pt.x,pt.y,pt2.x,pt2.y,rgb,widget->form->inverse);

	MENUFONT_Dimensions("M",flags,ys); ys>>=1;

	if (widget->data[1]>0) MENUFONT_Draw(pt2.x-12,pt.y+4+(ys>>1),256,"{",rgb,0);
	pt3=pt2;

	if (localctr<0xff) {
		pt =FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->x,widget->y));
		pt2=FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->ox,widget->oy));
	}

	item =(ListEntry*)widget->data[0];

	y=widget->data[1];
	while (y&&item) {
		y--; item=item->next;
	}

	if (localctr<255) return;

	y=pt.y+(ys>>1);
	while (item&&(y+(ys>>1)<pt2.y-4)) {
		subctr=localctr-255;
		subctr-=yctr*20;
		yctr++;
		if (subctr>0) {
//			shift=1;
//			rgb=ShiftAlpha(widget->form->textcolour,shift);
			
			// nasty brightness bodge
			rgb=ShiftAlpha(AlterAlpha(widget->form->textcolour,0xff),shift);
			/////////////////////////

			if (subctr<0xff) {
				alpha=(rgb>>24)-(255-subctr);
				if (alpha<0) alpha=0;
				rgb=AlterAlpha(rgb,alpha);
			}
//			MENUFONT_Draw(pt.x+6,y,256,item->text,rgb,0);
			MENUFONT_Draw(pt.x+6,y,128,item->text,rgb,0);
		}
		item=item->next;
		y+=ys;
	}
	if (widget->data[1]<widget->data[2]-widget->data[3]) MENUFONT_Draw(pt3.x-12,pt3.y-9+(ys>>1),256,"}",rgb,0);
}

SLONG TEXTS_Data(Widget *widget, SLONG code, SLONG data1, SLONG data2) {
	ListEntry *item, *item2;

	switch(code) {
	case WTM_ADDSTRING:
		item2=(ListEntry*)MemAlloc(sizeof(ListEntry));
		memset(item2,0,sizeof(ListEntry));
		item2->text=(CBYTE*)MemAlloc(strlen((CBYTE*)data2)+1);
		strcpy(item2->text,(CBYTE*)data2);

		item=(ListEntry*)widget->data[0];
		if (item) while (item->next) item=item->next;
		if (!item) {
			widget->data[0]=(SLONG)item2;
		} else {
			item->next=item2;
			item2->prev=item;
		}
		widget->data[2]++;
		break;
	case WTM_ADDBLOCK:
		// wrap string, pass back via WTM_ADDSTRING
		{
			ULONG chrs, temp;
			CBYTE *str, *walk;
			CBYTE tmp[_MAX_PATH];
			str=(CBYTE*)data2;
			walk=str;
			while(*walk) { // temporary thingy?
				if ((*walk==10)||(*walk==13)) *walk=32;
				walk++;
			}
			while (str) {
				chrs=MENUFONT_CharFit(str,(widget->ox-widget->x)-6,128);
				if (chrs==strlen(str)) {
					TEXTS_Data(widget,WTM_ADDSTRING,0,(SLONG)str);
					break;
				}
				temp=chrs;
				while (chrs&&(*(str+chrs)!=32)) {
					chrs--;
				}
				memset(tmp,0,_MAX_PATH);
				if (!chrs) {
					strncpy(tmp,str,temp);
					str+=temp;
					TEXTS_Data(widget,WTM_ADDSTRING,0,(SLONG)tmp);
				} else {
					strncpy(tmp,str,chrs);
					TEXTS_Data(widget,WTM_ADDSTRING,0,(SLONG)tmp);
					str+=chrs+1;
				}
			}
		}
		break;
	}
	return 0;
}

BOOL TEXTS_Char (Widget *widget, CBYTE key) {
	ListEntry *item;

	if (widget->state&WIDGET_STATE_DISABLED) return 0;
	if (key==13) { // tested seperately -- only key that works in -both- modes
		WIDGET_ToggleState(widget,WIDGET_STATE_ACTIVATED);
	}
	if (key==27) WIDGET_SetState(widget,0,WIDGET_STATE_ACTIVATED);
	if (!WIDGET_State(widget,WIDGET_STATE_ACTIVATED)) return 0;

	switch(key) {
	case 11:
		if (widget->data[1]>0) {
			widget->data[1]--;
			WIDGET_snd(WS_BLIP);
		} else WIDGET_snd(WS_FAIL);
		break;
	case 10:
		if (widget->data[1]<=widget->data[2]-widget->data[3]) {
			widget->data[1]++;
			WIDGET_snd(WS_BLIP);
		} else WIDGET_snd(WS_FAIL);
		break;
	case  3:
		widget->data[1]=0;
		WIDGET_snd(WS_BLIP);
		break;
	case  4:
		widget->data[1]=widget->data[2];
		WIDGET_snd(WS_BLIP);
		break;
	default:
		// we don't process it
		return 0;
	}
	return WIDGET_State(widget,WIDGET_STATE_ACTIVATED);
}

// --  list boxes --

void LISTS_Free(Widget *widget) {
	ListEntry *item;
	while (widget->data[0]) {
		item=(ListEntry*)widget->data[0];
		widget->data[0]=(SLONG)item->next;
		MemFree(item);
	}
	WIDGET_Free(widget);
}

void LISTS_Draw (Widget *widget) {
	WidgetPoint pt,pt2,pt3, ctr=WIDGET_Centre(widget);
	ListEntry *item, *item2;
	SLONG flags,y,ys;
	UBYTE shift = widget->state&WIDGET_STATE_FOCUS ? 1 : 2;
	UBYTE szshift = widget->state&WIDGET_STATE_SHRINKTEXT ? 1 : 0;
	ULONG rgb, localctr=widget->form->age*6;
	SWORD alpha, offset;

	pt =FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->x,widget->y));
	pt2=FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->ox,widget->oy));
	rgb=ShiftAlpha(widget->form->textcolour,shift);
	if (localctr<0xff) {
		offset=255-localctr;
		alpha=(rgb>>24)-offset;
		if (alpha<0) alpha=0;
		rgb=AlterAlpha(rgb,alpha);
		pt.x+=offset;	 pt.y+=offset;
		pt2.x-=offset; pt2.y-=offset;
		if (pt.x>ctr.x) pt.x=ctr.x;
		if (pt.y>ctr.y) pt.y=ctr.y;
		if (pt2.x<ctr.x) pt2.x=ctr.x;
		if (pt2.y<ctr.y) pt2.y=ctr.y;
	}
	WIDGET_Rect(pt.x,pt.y,pt2.x,pt2.y,rgb,widget->form->inverse);

	MENUFONT_Dimensions("M",flags,ys); ys>>=szshift;

	if (widget->data[3]>0) MENUFONT_Draw(pt2.x-12,pt.y+4+(ys>>1),256,"{",rgb,0);
	pt3=pt2;

	if (localctr<0xff) {
		pt =FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->x,widget->y));
		pt2=FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->ox,widget->oy));
	}

	item =(ListEntry*)widget->data[0];
	item2=(ListEntry*)widget->data[2];

	y=widget->data[3];
	while (y&&item) {
		y--; item=item->next;
	}

	y=pt.y+(ys>>1);
	while (item&&(y+(ys>>1)<pt2.y-4)) {
		flags = ((item==item2) && (widget->data[1])) ? MENUFONT_GLIMMER : 0;
		shift=(item==item2)?0:1;
		rgb=ShiftAlpha(widget->form->textcolour,shift);
		if (localctr<0xff) {
			alpha=(rgb>>24)-(255-localctr);
			if (alpha<0) alpha=0;
			rgb=AlterAlpha(rgb,alpha);
		}
		MENUFONT_Draw(pt.x+6,y,256>>szshift,item->text,rgb,flags);
		item=item->next;
		y+=ys;
	}
	if (item) MENUFONT_Draw(pt3.x-12,pt3.y-9+(ys>>1),256,"}",rgb,0);
}

SLONG LISTS_Data(Widget *widget, SLONG code, SLONG data1, SLONG data2) {
	ListEntry *item, *item2;

	switch(code) {
	case WLM_ADDSTRING:
		item2=(ListEntry*)MemAlloc(sizeof(ListEntry));
		memset(item2,0,sizeof(ListEntry));
		item2->text=(CBYTE*)MemAlloc(strlen((CBYTE*)data2)+1);
		strcpy(item2->text,(CBYTE*)data2);

		item=(ListEntry*)widget->data[0];
		if (item) while (item->next) item=item->next;
		if (!item) {
			widget->data[0]=widget->data[2]=(SLONG)item2;
		} else {
			item->next=item2;
			item2->prev=item;
		}
		break;
	}
	return 0;
}

void LISTS_caretchk(Widget *widget) {
  if (widget->data[4]<widget->data[3])
	  widget->data[3]=widget->data[4];
  else {
	  SLONG dx,dy,rows = (widget->oy-widget->y)-6;
	  MENUFONT_Dimensions("M",dx,dy); if (widget->state&WIDGET_STATE_SHRINKTEXT) dy>>=1;
	  rows/=dy;
	  while (widget->data[4]>=widget->data[3]+rows) widget->data[3]++;
  }
}

void LISTS_Push(Widget *widget) {
	if (!widget->data[1]) // we're "ok"ing... sorta
			if (widget->form->proc) widget->form->proc(widget->form,widget,WLN_ENTER);
}

BOOL LISTS_Char (Widget *widget, CBYTE key) {
	ListEntry *item;

	if (widget->state&WIDGET_STATE_DISABLED) return 0;
	if (key==13) { // tested seperately -- only key that works in -both- modes
		widget->data[1]^=1;
/*		if (!widget->data[1]) // we're "ok"ing... sorta
			if (widget->form->proc) widget->form->proc(widget->form,widget,WLN_ENTER);*/

	}
	if (key==27) widget->data[1]=0;
	if (!widget->data[1]) return 0;
	item=(ListEntry*)widget->data[2];
	if (!item) return 0;

	switch(key) {
	case 11:
		if (item->prev) {
			widget->data[2]=(SLONG)item->prev;
			widget->data[4]--;
			WIDGET_snd(WS_BLIP);
		} else WIDGET_snd(WS_FAIL);
		break;
	case 10:
		if (item->next) {
			widget->data[2]=(SLONG)item->next;
			widget->data[4]++;
			WIDGET_snd(WS_BLIP);
		} else WIDGET_snd(WS_FAIL);
		break;
	case  3:
		widget->data[2]=widget->data[0];
		widget->data[4]=0;
		WIDGET_snd(WS_BLIP);
		break;
	case  4:
		while (item->next) {
			item=item->next;
			widget->data[4]++;
		}
		widget->data[2]=(SLONG)item;
		WIDGET_snd(WS_BLIP);
		break;
	default:
		// we don't process it
		return 0;
	}
	LISTS_caretchk(widget);
	return widget->data[1];
}

// -- glyph buttons --

void GLYPH_Draw (Widget *widget) {
  WidgetPoint pt,pt2;
  float x,y,ox,oy;
  SLONG rgb;

  pt =FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->x,widget->y));
  pt2=FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->ox,widget->oy));
  x=(float)widget->data[1] / 256.0f; 
  y=(float)widget->data[2] / 256.0f; 
  ox=(float)widget->data[3] / 256.0f; 
  oy=(float)widget->data[4] / 256.0f;
  rgb = (widget->state & WIDGET_STATE_FOCUS) ? 0xFFFFFF7f : 0xFF7fFF7f;
  DRAW2D_Sprite(pt.x,pt.y,pt2.x,pt2.y,x,y,ox,oy,widget->data[0],rgb);
}

// -- shaded rect --

void SHADE_Draw (Widget *widget) {
  WidgetPoint pt,pt2;
  float x,y,ox,oy;
  SLONG rgb,a;

  pt =FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->x,widget->y));
  pt2=FORM_To_Screen(widget->form,TO_WIDGETPNT(widget->ox,widget->oy));
  a=widget->data[2]>>24; a-=widget->data[1];
  if (a<0) a=0;
  rgb=AlterAlpha(widget->data[2],a);
  DRAW2D_Box(pt.x,pt.y,pt2.x,pt2.y,rgb,widget->data[3],100);
  if (widget->data[1]>0) {
	  widget->data[1]-=widget->data[0];
	  if (widget->data[1]<0) widget->data[1]=0;
  }
}

//---------------------------------------------------------------------------------
// Section The Fifth:  Generic Widget Stuff
//

Widget* WIDGET_Create(Methods *widget_class, SLONG x, SLONG y, SLONG ox, SLONG oy, CBYTE *caption) {
	Widget *widget;

	widget = (Widget*) MemAlloc(sizeof(Widget));
	memset(widget,0,sizeof(Widget));
	widget->x=x; widget->y=y; widget->ox=ox; widget->oy=oy;
	if (caption) {
		widget->caption=(CBYTE*)MemAlloc(strlen(caption)+1);
		strcpy(widget->caption,caption);
	}
	widget->methods=widget_class;
	if (widget->methods->Init) widget->methods->Init(widget);

	return widget;
}

void WIDGET_menu(Form *form, ...) {
	va_list marker;
	UBYTE	count,i;
	SLONG	ox=(form->ox-form->x);
	SLONG	oy=(form->oy-form->y);
	SLONG	xofs,yofs, pos;
	CBYTE*	txt;

	va_start(marker, form);
		count=0;
		while (va_arg(marker,CBYTE*)) count++;
	va_end(marker);
	
	yofs=oy/count;

	va_start(marker, form);
//		pos=yofs>>1;
	pos=0;
		for(i=0;i<count;i++) {
			txt=va_arg(marker,CBYTE*);
			FORM_AddWidget(form,WIDGET_Create(&BUTTON_Methods,0,pos, ox,pos+40,txt))->tag=i+1;
			pos+=yofs;
		}
	va_end(marker);

}


//---------------------------------------------------------------------------------
// Section The Sixth:  Generic Form Stuff
//

Form*	FORM_Create(CBYTE *caption, FORM_Proc proc, SLONG x, SLONG y, SLONG ox, SLONG oy, ULONG textcolour) {
	Form *form;

	form = (Form*) MemAlloc(sizeof(Form));
	memset(form,0,sizeof(Form));

	strcpy(form->caption,caption);
	form->x=x; form->y=y; form->ox=ox; form->oy=oy;
	form->proc=proc;
	form->textcolour=textcolour;

	return form;
}

void	FORM_Free(Form* form) {
	Widget *last;

	while (form->children) {
		last=form->children;
		form->children=form->children->next;
		last->form=0; // saves list headaches
		last->methods->Free(last);
	}
	MemFree(form);
}

Widget*	FORM_AddWidget(Form *form, Widget *widget) {
	widget->form=form;
	widget->next=0;
	if (!form->children)							// first widget on form
		FORM_Focus(form,form->children=widget);		// so focus it
	else {
		widget->prev=form->children;				// grab the first widget on the form
		while (widget->prev->next)					// and walk to the last
			widget->prev=widget->prev->next;
		widget->prev->next=widget;					// it's now our backlink
		if (!form->focus)							// we don't have focus yet (1st ctl static?)
			FORM_Focus(form,widget);						// so focus it

	}
	
	return widget;
}

void	FORM_DelWidget(Widget *widget) {
	Form *form=widget->form;

	if (widget==form->focus) FORM_Focus(form, widget,-1);
	if (widget==form->focus) FORM_Focus(form, 0, 0); // still -- must be the only focusable thing

	if (widget->next) widget->next->prev=widget->prev;
	if (widget->prev) widget->prev->next=widget->next;
		else form->children=widget->next;	

	widget->prev=widget->next=0;
	widget->form=0;
	
}

inline BOOL	FORM_KeyProc(SLONG key) {
	if (Keys[key]) {
		Keys[key]=0;
		EatenKey=1;
		return 1;
	}
	return 0;
}

SLONG	FORM_Process(Form* form) {
	CBYTE key;
	Widget *lastfocus;
	static int lastx = 0,lasty = 0;
	static int input = 0, lastinput = 0;
	static int ticker=0;

	if (!form->age) 
		if (form->children) WIDGET_snd(WS_FADEIN); else WIDGET_snd(WS_FAIL);

	WidgetTick++;
	form->age++;

	if (LastKey) {
		key= (Keys[KB_LSHIFT]||Keys[KB_RSHIFT]) ?  InkeyToAsciiShift[LastKey] : InkeyToAscii[LastKey] ;
		if (key==8) key=127; // heh
		if (!key)
			switch(LastKey) {
			case KB_UP:		key=11;	break;
			case KB_RIGHT:	key=9;	break;
			case KB_LEFT:	key=8;	break;
			case KB_DOWN:	key=10;	break;
			case KB_ESC:	key=27;	break;
			case KB_ENTER:	key=13; break;
				// arbitrary ones:
			case KB_PGUP:	key=1;	break;
			case KB_PGDN:	key=2;	break;
			case KB_HOME:	key=3;	break;
			case KB_END:	key=4;	break;
			case KB_DEL:	key=5;	break;
			}
	}
	else
		key=0;

//#define	INPUT_TYPE_JOY	(1<<1)
//extern ULONG get_hardware_input(UWORD type);

	lastinput=input;

	input=get_hardware_input(INPUT_TYPE_JOY);
//	TRACE("input: %d\n",input);
	if (input&&(input!=lastinput)&&(ticker<1)) {
		if (input&(INPUT_MASK_JUMP|INPUT_MASK_KICK|INPUT_MASK_PUNCH|INPUT_MASK_ACTION)) {
			key=13;
			Keys[KB_ENTER]=1;
		}
		if (input&INPUT_MASK_FORWARDS) { key=11; Keys[KB_UP]=1; }
		if (input&INPUT_MASK_BACKWARDS) { key=10; Keys[KB_DOWN]=1; }
		if (input&INPUT_MASK_START) { form->returncode=-69; }
		ticker=10;
	}
	ticker--;

	if (key) {
		if (form->focus&&form->focus->methods->Char&&form->focus->methods->Char(form->focus,key))
			; // swallow the keypress
		else {
			  // default key processing
			EatenKey=0;
			lastfocus=form->focus;
			if (FORM_KeyProc(KB_UP))	{ FORM_Focus(form,0,-1); WIDGET_snd(WS_MOVE); }
			if (FORM_KeyProc(KB_DOWN))	{ FORM_Focus(form,0, 1); WIDGET_snd(WS_MOVE); }
			if (FORM_KeyProc(KB_HOME))	{ FORM_Focus(form,form->children, 0); WIDGET_snd(WS_MOVE); }
			if (FORM_KeyProc(KB_END))	{ FORM_Focus(form,form->children, -1); WIDGET_snd(WS_MOVE); }
			if (FORM_KeyProc(KB_ENTER))	if (form->focus&&form->focus->methods->Push) form->focus->methods->Push(form->focus);
			if (FORM_KeyProc(KB_TAB)&&form->focus) {
				if (form->focus->methods->Char) form->focus->methods->Char(form->focus,27);
				FORM_Focus(form,0, ShiftFlag ? -1 : 1);
				WIDGET_snd(WS_MOVE);
			}
			if (lastfocus!=form->focus) form->proc(form,0,WFN_FOCUS);
			if (LastKey&&form->proc&&!EatenKey) form->proc(form,0,WFN_CHAR);

		}
	} else {
#ifndef PSX
			POINT pt;
			SLONG res;
			GetCursorPos(&pt);
extern volatile HWND	hDDLibWindow;
			if(!the_display.IsFullScreen()) ScreenToClient(hDDLibWindow,&pt);
			if ((pt.x!=lastx)||(pt.y!=lasty)) {
				lastx=pt.x; lasty=pt.y;
				Widget *scan=FORM_GetWidgetFromPoint(form,TO_WIDGETPNT(lastx,lasty));
				if (scan&&(scan!=form->focus)) {
					if (form->focus&&form->focus->methods->Char) form->focus->methods->Char(form->focus,27);
					FORM_Focus(form,scan,0);
				}
			}
			res=GetAsyncKeyState(VK_LBUTTON);
			if ((res & (1<<15))&&(res&1)) {
				Widget *scan=FORM_GetWidgetFromPoint(form,TO_WIDGETPNT(lastx,lasty));
				if (scan&&scan->methods->Push) scan->methods->Push(scan);
				else
				  if (scan&&scan->methods->Char) scan->methods->Char(scan,13);
			}
#endif
	}

	LastKey=0;

	return form->returncode; // form is processing normally
}

Widget*	FORM_GetWidgetFromPoint(Form *form, WidgetPoint pt) {
	Widget *scan=form->children;
	while (scan) {
		if ((!(scan->state&WIDGET_STATE_DISABLED))&&scan->methods->HitTest&&(scan->methods->HitTest(scan,pt.x,pt.y)))
			return scan;
		scan=scan->next;
	}
	return 0;
}


void	FORM_Draw(Form *form) {
	Widget *walk;

/*	if (form->caption) {
		SLONG dx,dy;
		MENUFONT_Dimensions(form->caption,dx,dy); dx+=16;
		MENUFONT_Draw(form->x+8,form->y+10,256,(CBYTE*)form->caption,ShiftAlpha(form->textcolour,1),0);
		WIDGET_Rect(form->x,form->y,form->x+dx,form->y+23,ShiftAlpha(form->textcolour,2),form->inverse,RECT_TOP|RECT_RIGHT);
		WIDGET_Rect(form->x+3,form->y+20,form->ox,form->oy,ShiftAlpha(form->textcolour,2),form->inverse,RECT_TOP|RECT_RIGHT);
		WIDGET_Rect(form->x,form->y,form->ox,form->oy,ShiftAlpha(form->textcolour,2),form->inverse,RECT_LEFT|RECT_BOTTOM);
	} else {
		WIDGET_Rect(form->x,form->y,form->ox,form->oy,ShiftAlpha(form->textcolour,2),form->inverse);
	}
*/
	walk=form->children;
	while (walk) {
		walk->methods->Draw(walk);
		walk=walk->next;
	}
	form->proc(form,0,WFN_PAINT);
}

Widget*	FORM_Focus(Form *form, Widget *widget, SBYTE direction) {
	if (form->focus) WIDGET_SetState(form->focus,0,WIDGET_STATE_FOCUS);
	if (!widget) widget=form->focus;
	form->focus=widget;
	if ((!widget)||((widget->state&(WIDGET_STATE_BLOCKFOCUS|WIDGET_STATE_DISABLED))&&!direction)) return (form->focus=0);
	if (direction) {
		if (direction==-1) {
			if (form->focus->prev)
				form->focus=form->focus->prev;
			else
				while (form->focus->next) form->focus=form->focus->next;
		} else {
			if (form->focus->next)
				form->focus=form->focus->next;
			else
				while (form->focus->prev) form->focus=form->focus->prev;
		}
		// the following line may be psx unfriendly in extreme circumstances:
		if (form->focus->state&(WIDGET_STATE_BLOCKFOCUS|WIDGET_STATE_DISABLED)) return FORM_Focus(form,0,direction);
		// but only when there's a LOT of gnasty things on a form
	}
	if (form->focus) WIDGET_SetState(form->focus,WIDGET_STATE_FOCUS);
	return widget;
}
