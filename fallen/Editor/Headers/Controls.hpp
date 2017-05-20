// Controls.hpp
// Guy Simmons, 18th February 1997

#ifndef	_CONTROLS_HPP_
#define	_CONTROLS_HPP_

#include	"Primativ.hpp"

#define	BUTTON				1
#define	RADIO_BUTTON		2
#define	CHECK_BOX			3
#define	STATIC_TEXT			4
#define	EDIT_TEXT			5
#define	PULLDOWN_MENU		6
#define	POPUP_MENU			7
#define	H_SLIDER			8
#define	V_SLIDER			9

#define	CONTROL_NONE			0
#define	CONTROL_INACTIVE	(1<<0)
#define	CONTROL_HILITED		(1<<1)
#define	CONTROL_SELECTED	(1<<2)
#define	CONTROL_CLICKED		(1<<3)
#define	CONTROL_SHOW_EXTRA	(1<<4)

#define	EDIT_TEXT_LENGTH	64

// ^ - Seperator line.
// ~ - Check line.
// @ - Inactive.
// ! - End of list.

#define	MENU_NORMAL			(1<<0)
#define	MENU_SEPERATOR		(1<<1)
#define	MENU_END			(1<<2)
#define	MENU_HILITED		(1<<3)
#define	MENU_INACTIVE		(1<<4)
#define	MENU_CHECK			(1<<5)
#define	MENU_CHECK_MASK		(1<<7)

#define	CTRL_SELECTED	0
#define	CTRL_DESELECTED	1
#define	CTRL_ACTIVE		2
#define	CTRL_INACTIVE	3

class MenuDef2
{
public:
	CBYTE		*ItemText;
	UBYTE		HotKey,
				ItemFlags,
				ItemID,
				MutualExclusiveID;
	EdRect		ItemRect;
/*
#ifdef	_MSC_VER
				MenuDef2(CBYTE *s)			{	ItemText=s;	}
				MenuDef2(CBYTE *s,UBYTE k)	{	ItemText=s;HotKey=k;	}
#endif
*/
};

struct ControlDef
{
	UBYTE		ControlType,
				HotKey;
	CBYTE		*Title;
	SWORD		ControlLeft,
				ControlTop,
				ControlWidth,
				ControlHeight;
	MenuDef2	*TheMenuDef;
};


class	Control	:	public EdRect
{
	private:
		UBYTE			Flags,
						ControlID,
						ControlType,
						HotKey;
		CBYTE			*ControlTitle;
		Control			*LastControl,
						*NextControl;
		ControlDef		*TheDef;

	public:
								Control(void)							{	ControlTitle=NULL;LastControl=NULL;NextControl=NULL;	}
		virtual void			DrawControl(void);
		virtual UWORD			TrackControl(MFPoint *down_point);
		virtual	void			TrackKey(void);
		virtual	void			HiliteControl(MFPoint *current_point);
		virtual	void			UnHiliteControl(void);

		virtual inline BOOL		PointInControl(MFPoint *the_point)		{	return PointInRect(the_point);	}

		inline void				SetFlags(UBYTE flags)					{	Flags=flags;					}
		inline UBYTE			GetFlags(void)							{	return Flags;					}
		inline void				SetID(UBYTE id)							{	ControlID=id;					}
		inline UBYTE			GetID(void)								{	return ControlID;				}
		inline void				SetType(UBYTE type)						{	ControlType=type;				}
		inline UBYTE			GetType(void)							{	return ControlType;				}
		inline void				SetTitle(CBYTE *title)					{	ControlTitle=title;				}
		inline CBYTE			*GetTitle(void)							{	return ControlTitle;			}
		inline void				SetHotKey(UBYTE key)					{	HotKey=key;						}
		inline UBYTE			GetHotKey(void)							{	return HotKey;					}

		inline void				SetLastControl(Control *last_control)	{	LastControl=last_control;		}
		inline void				SetNextControl(Control *next_control)	{	NextControl=next_control;		}
		inline Control			*GetLastControl(void)					{	return LastControl;				}
		inline Control			*GetNextControl(void)					{	return NextControl;				}
};


class	CButton	:	public	Control
{
	public:
						CButton(ControlDef *the_def);
		void			DrawControl(void);
};

class	CRadioButton	:	public Control
{
	public:
						CRadioButton(ControlDef *the_def);
		void			DrawControl(void);
};

class	CCheckBox	:	public Control
{
	public:
						CCheckBox(ControlDef *the_def);
		void			DrawControl(void);
};

class	CStaticText	:	public Control
{
	private:
		CBYTE			String1[EDIT_TEXT_LENGTH],
						String2[EDIT_TEXT_LENGTH];
	public:
						CStaticText(ControlDef *the_def);
		void			DrawControl(void);
		inline CBYTE	*SetString1(CBYTE *the_string)		{	strncpy(String1,the_string,EDIT_TEXT_LENGTH);String1[EDIT_TEXT_LENGTH-1]=0;return String1;	}
		inline CBYTE	*SetString2(CBYTE *the_string)		{	strncpy(String2,the_string,EDIT_TEXT_LENGTH);String2[EDIT_TEXT_LENGTH-1]=0;return String2;	}
};

class	CEditText	:	public Control
{
	private:
		CBYTE			EditText[EDIT_TEXT_LENGTH];
		ULONG			SelectEnd,
						SelectStart;
		SLONG			CursorPos,
						TextX;

	public:
						CEditText(ControlDef *the_def);
		void			DrawControl(void);
		UWORD			TrackControl(MFPoint *down_point);
		inline CBYTE	*GetEditString(void)				{	return EditText;								}
		inline CBYTE	*SetEditString(CBYTE *the_string)	{	strcpy(EditText,the_string); return EditText;	}
};

class	CPullDown	:	public Control
{
	private:
		MenuDef2		*TheMenu;
		EdRect			ItemsRect;

	public:
						CPullDown(ControlDef *the_def);
		void			DrawControl(void);
		UWORD			TrackControl(MFPoint *down_point);
		inline void		SetItemFlags(UWORD item,UBYTE flags){	TheMenu[item-1].ItemFlags=flags;	}
		inline UBYTE	GetItemFlags(UWORD item)			{	return	TheMenu[item-1].ItemFlags;	}
};

class	CPopUp	:	public Control
{
	private:
		MenuDef2		*TheMenu;
		EdRect			ItemsRect;

	public:
						CPopUp(ControlDef *the_def);
		void			DrawControl(void);
		UWORD			TrackControl(MFPoint *down_point);
		void			SetItemState(UWORD item,UBYTE state);
		inline void		SetItemFlags(UWORD item,UBYTE flags){	TheMenu[item-1].ItemFlags=flags;	}
		inline UBYTE	GetItemFlags(UWORD item)			{	return	TheMenu[item-1].ItemFlags;	}
};

#define	SLIDER_SIZE		13

class	CHSlider	:	public	Control
{
	private:
		UBYTE			DragFlags,
						LeftButtonFlags,
						RightButtonFlags;
		SLONG			CurrentValue,
						MinValue,
						MaxValue,
						ValueStep;
		SLONG			CurrentDrag,
						MinDrag,
						MaxDrag,
						DragStep;
		EdRect			DragRect,
						LeftButtonRect,
						RightButtonRect;
		void			(*update_function)(void);

		void			SetupDrag(void);

	public:
						CHSlider(ControlDef *the_def);
		void			DrawControl();
		void			HiliteControl(MFPoint *current_point);
		void			UnHiliteControl(void);
		UWORD			TrackControl(MFPoint *down_point);
		BOOL			PointInControl(MFPoint *the_point);

		void			SetCurrentValue(SLONG value);
		inline void		SetUpdateFunction(void (*the_fn)(void))		{	update_function=the_fn;					}
		inline SLONG	GetCurrentValue(void)				{	return CurrentValue;					}
		inline void		SetValueRange(SLONG min,SLONG max)	{	MinValue=min;MaxValue=max;SetupDrag();	}
		inline void		SetValueStep(SLONG value_step)		{	ValueStep=value_step;					}

		inline void		SetDragFlags(UBYTE flags)			{	DragFlags=flags;					}
		inline UBYTE	GetDragFlags(void)					{	return DragFlags;					}
		inline void		SetLeftButtonFlags(UBYTE flags)		{	LeftButtonFlags=flags;				}
		inline UBYTE	GetLeftButtonFlags(void)			{	return LeftButtonFlags;				}
		inline void		SetRightButtonFlags(UBYTE flags)	{	RightButtonFlags=flags;				}
		inline UBYTE	GetRightButtonFlags(void)			{	return RightButtonFlags;			}
};

class	CVSlider	:	public	Control
{
	private:
		UBYTE			DragFlags,
						TopButtonFlags,
						BottomButtonFlags;
		SLONG			CurrentValue,
						MinValue,
						MaxValue,
						ValueStep;
		SLONG			CurrentDrag,
						MinDrag,
						MaxDrag,
						DragStep;
		EdRect			DragRect,
						TopButtonRect,
						BottomButtonRect;
		void			(*update_function)(void);

		void			SetupDrag(void);

	public:
						CVSlider(ControlDef *the_def);
		void			DrawControl();
		void			HiliteControl(MFPoint *current_point);
		void			UnHiliteControl(void);
		UWORD			TrackControl(MFPoint *down_point);
		BOOL			PointInControl(MFPoint *the_point);

		void			SetCurrentValue(SLONG value);
		inline void		SetUpdateFunction(void (*the_fn)(void))		{	update_function=the_fn;					}
		inline SLONG	GetCurrentValue(void)				{	return CurrentValue;					}
		inline void		SetValueRange(SLONG min,SLONG max)	{	MinValue=min;MaxValue=max;SetupDrag();	}
		inline void		SetValueStep(SLONG value_step)		{	ValueStep=value_step;					}

		inline void		SetDragFlags(UBYTE flags)			{	DragFlags=flags;					}
		inline UBYTE	GetDragFlags(void)					{	return DragFlags;					}
		inline void		SetTopButtonFlags(UBYTE flags)		{	TopButtonFlags=flags;				}
		inline UBYTE	GetTopButtonFlags(void)				{	return TopButtonFlags;				}
		inline void		SetBottomButtonFlags(UBYTE flags)	{	BottomButtonFlags=flags;			}
		inline UBYTE	GetBottomButtonFlags(void)			{	return BottomButtonFlags;			}
};

#endif
