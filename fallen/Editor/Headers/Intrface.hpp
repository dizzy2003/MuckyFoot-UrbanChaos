// Intrface.hpp
// Guy Simmons, 18th February 1997.

#ifndef	_INTRFACE_HPP_
#define	_INTRFACE_HPP_

class	Interface
{
	private:
		ULONG			ContentColour,
						ContentColourBr,
						HiliteColour,
						LoliteColour,
						ActiveColour,
						InactiveColour,
						WhiteColour,
						GreyColour,
						YellowColour,
						RedColour,
						GreenColour,
						BlueColour,
						TextColour,
						SelectColour;
		UBYTE				InterfacePalette[768];
		UBYTE			*interface_sprite_data;
		BSprite			*interface_sprites;

	public:
						Interface();
						~Interface();
		void			SetupInterfaceDefaults(void);
		inline ULONG	GetContentColour(void)			{	return	ContentColour;			};
		inline ULONG	GetContentColourBr(void)		{	return	ContentColourBr;		};
		inline ULONG	GetHiliteColour(void)			{	return	HiliteColour;			};
		inline ULONG	GetWhiteColour(void)			{	return	WhiteColour;			};
		inline ULONG	GetGreyColour(void)				{	return	GreyColour;				};
		inline ULONG	GetYellowColour(void)			{	return	YellowColour;			};
		inline ULONG	GetRedColour(void)				{	return	RedColour;				};
		inline ULONG	GetGreenColour(void)			{	return	GreenColour;			};
		inline ULONG	GetBlueColour(void)				{	return	BlueColour;				};
		inline ULONG	GetLoliteColour(void)			{	return	LoliteColour;			};
		inline ULONG	GetActiveColour(void)			{	return	ActiveColour;			};
		inline ULONG	GetInactiveColour(void)			{	return	InactiveColour;			};
		inline ULONG	GetTextColour(void)				{	return	TextColour;				};
		inline ULONG	GetSelectColour(void)			{	return	SelectColour;			};
		inline UBYTE	*GetPalette(void)				{	return	InterfacePalette;		};
		inline BSprite*	GetInterfaceSprite(UWORD id)	{	return	&interface_sprites[id];	};
//		inline BSprite*	GetInterfacePointer(UWORD id)	{	return	&interface_pointers[id];};
};




extern Interface				*InterfaceDefaults;
#define	CONTENT_COL_BR			InterfaceDefaults->GetContentColourBr()
#define	CONTENT_COL				InterfaceDefaults->GetContentColour()
#define	HILITE_COL  			InterfaceDefaults->GetHiliteColour()
#define	LOLITE_COL		  		InterfaceDefaults->GetLoliteColour()
#define	ACTIVE_COL				InterfaceDefaults->GetActiveColour()
#define	INACTIVE_COL			InterfaceDefaults->GetInactiveColour()
#define	TEXT_COL				InterfaceDefaults->GetTextColour()
#define	SELECT_COL	  			InterfaceDefaults->GetSelectColour()
#define	WHITE_COL  				InterfaceDefaults->GetWhiteColour()
#define	GREY_COL  				InterfaceDefaults->GetGreyColour()
#define	YELLOW_COL  				InterfaceDefaults->GetYellowColour()
#define	RED_COL  				InterfaceDefaults->GetRedColour()
#define	GREEN_COL  				InterfaceDefaults->GetGreenColour()
#define	BLUE_COL  				InterfaceDefaults->GetBlueColour()

#define	INTERFACE_SPRITE(ID)	InterfaceDefaults->GetInterfaceSprite(ID)
#define	INTERFACE_POINTER(ID)	InterfaceDefaults->GetInterfacePointer(ID)

#define	PALETTE					InterfaceDefaults->GetPalette()

#define	DEFAULTS_DIRECTORY		"EditDefs"

#define	GROW_ICON				1

#define	UP_ICON					6
#define	DOWN_ICON				7
#define	LEFT_ICON				8
#define	RIGHT_ICON				9
#define	CHECK_ICON				10
#define	RADIO_ICON1				11
#define	RADIO_ICON2				12
#define	RADIO_ICON3				13
#define	RADIO_ICON4				14
#define MENU_ICON				15

#define	NO_CLICK				0
#define	LEFT_CLICK				1
#define	RIGHT_CLICK				2
#define	MIDDLE_CLICK			3


#endif
