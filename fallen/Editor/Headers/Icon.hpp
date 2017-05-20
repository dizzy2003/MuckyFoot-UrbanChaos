// Window.hpp
// Guy Simmons, 18th February 1997.

#ifndef	_ICON_HPP_
#define	_ICON_HPP_

#include	"Primativ.hpp"


struct	AWindowIcon
{
	void	(*Function)(UWORD id);
	UBYTE	Flag;
	UWORD	ImageOn;
	SWORD	ImageOff;
};

class	WinBarIcon	:	public  EdRect
{
	private:
		struct	AWindowIcon	*WindowIcons;
	public:
	inline				WinBarIcon()   {}
	void				DrawIcons(void);
	void				HandleIconClick(UBYTE flags,MFPoint *clicked_point);
	void				InitIcons(struct AWindowIcon *p_icons);
};

#endif