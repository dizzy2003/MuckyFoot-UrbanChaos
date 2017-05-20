//
// widget.h
// Matthew Rosenfeld	24 February 1999
//
// widget provides windows-esque controls such as buttons, checkboxes and stuff
// grouped together into 'forms' (palmpilot style)
//

//
// of course, you _do_ realise this is a fuck of a lot easier with C++ right?
//
// i knew you did.
//

#include "MFStdLib.h"

//----------------------------------------------------------------------------
// Flags and other misc defines
//

#define WIDGET_STATE_FOCUS		1
#define WIDGET_STATE_DISABLED	2
#define WIDGET_STATE_ACTIVATED	4
#define WIDGET_STATE_BLOCKFOCUS	8
#define	WIDGET_STATE_ALIGNLEFT	16
#define WIDGET_STATE_SHRINKTEXT 32

#define WS_MOVE		1
#define WS_FADEIN	2
#define WS_FADEOUT	3
#define	WS_OK		4
#define WS_FAIL		5
#define WS_BLIP		6

//----------------------------------------------------------------------------
// General type declarations
//

class Widget;
class Form;

typedef void (*WIDGET_Void)(Widget *widget);
typedef BOOL (*WIDGET_Clik)(Widget *widget, SLONG x, SLONG y);
typedef BOOL (*WIDGET_Char)(Widget *widget, CBYTE key);
typedef SLONG(*WIDGET_Data)(Widget *widget, SLONG code, SLONG data1, SLONG data2);
typedef BOOL (*FORM_Proc)(Form *form, Widget *widget, SLONG message);

//----------------------------------------------------------------------------
// Widget Structures
//

struct Methods {
	WIDGET_Void	Init, Free, Draw, Push;
	WIDGET_Char Char;
	WIDGET_Data Data;
	WIDGET_Clik HitTest;
};

struct WidgetPoint {
	SLONG		x, y;
};

class Widget {	// C++ Now! 

	public:

	SLONG		x, y, ox, oy;		// parent-relative coords
	SLONG		tag;				// a value for its parent to use as it sees fit
	SLONG		state;				// focused, etc
	SLONG		data[5];			// widget-specific data
	CBYTE		*caption;			// text label as appropriate
	Form		*form;				// parent form
	Methods		*methods;			// pointer to its virtual method table (muhahaha)
	Widget		*prev, *next;		// linked list pointers
};

class Form {

	public:

	SLONG		x, y, ox, oy;		// screen coords
	ULONG		textcolour;			// rgb for text
	SLONG		returncode;			// if !0 then form returns
	SLONG		age;				// for fading in forms
	SLONG		inverse;			// bet this gets turned into flags later
	Widget		*children;			// child widgets
	Widget		*focus;				// current focus control
	FORM_Proc	proc;				// dialog procedure
	CBYTE		caption[32];		// form text
};

//----------------------------------------------------------------------------
// Externally-called functions
//

// Forms
Form*	FORM_Create(CBYTE *caption, FORM_Proc proc, SLONG x, SLONG y, SLONG ox, SLONG oy, ULONG textcolour);
Widget*	FORM_AddWidget(Form *form, Widget *widget);
void   	FORM_DelWidget(Widget *widget);
SLONG	FORM_Process(Form* form);
void	FORM_Draw(Form* form);
void	FORM_Free(Form* form);
Widget*	FORM_Focus(Form* form, Widget* widget, SBYTE direction=0);
Widget*	FORM_GetWidgetFromPoint(Form *form, WidgetPoint pt);

// Widgets
void	WIDGET_menu(Form *form, ...);
Widget*	WIDGET_Create(Methods *widget_class, SLONG x, SLONG y, SLONG ox, SLONG oy, CBYTE *caption);
//void	WIDGET_State(Widget *widget, SLONG data, SLONG mask=0xFFFF);
inline void	WIDGET_SetState(Widget *widget, SLONG data, SLONG mask=0) {
	widget->state=(widget->state&~mask)|data;
}
inline void	WIDGET_ToggleState(Widget *widget, SLONG data) {
	widget->state^=data;
}
inline SLONG	WIDGET_State(Widget *widget, SLONG mask=0xFFFF) {
	return (widget->state&mask);
}


// Utility
inline WidgetPoint TO_WIDGETPNT(SLONG x, SLONG y) { WidgetPoint pt = { x, y }; return pt; }
inline WidgetPoint FORM_To_Screen(Form *form, WidgetPoint pt) { return TO_WIDGETPNT(pt.x+form->x, pt.y+form->y); }
inline WidgetPoint WIDGET_Centre(Widget *widget) {
	return FORM_To_Screen(widget->form, TO_WIDGETPNT((widget->x+widget->ox)>>1,(widget->y+widget->oy)>>1));
}
void WIDGET_snd(SLONG snd);

//----------------------------------------------------------------------------
// Widget "class library"
//

extern Methods BUTTON_Methods;
extern Methods STATIC_Methods;
extern Methods CHECK_Methods;
extern Methods RADIO_Methods;
extern Methods INPUT_Methods;
extern Methods LISTS_Methods;
extern Methods TEXTS_Methods;
extern Methods GLYPH_Methods;
extern Methods SHADE_Methods;

//----------------------------------------------------------------------------
// Widget messages
//
// WxN_* -- Widget <whatever> Notification -- sent to dlg proc
// WxM_* -- Widget <whatever> Message      -- sent to widgets 

#define WBN_PUSH		1
#define WIN_ENTER		1
#define WLN_ENTER		1
#define WFN_CHAR		1
#define WFN_FOCUS		2
#define WFN_PAINT		3

#define WLM_ADDSTRING	1
#define WIM_SETSTRING	1
#define WIM_SETMODE		2
#define WTM_ADDSTRING	1
#define WTM_ADDBLOCK	2
