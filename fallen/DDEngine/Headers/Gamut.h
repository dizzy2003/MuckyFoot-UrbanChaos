// Gamut.h
// Guy Simmons, 4th November 1997.

#ifndef	GAMUT_H
#define	GAMUT_H

//---------------------------------------------------------------

#define	MAX_GAMUT_RADIUS		(24)

//---------------------------------------------------------------

typedef	struct
{
	SBYTE		DX,
				DZ;
	SWORD		Angle;
}GamutElement;

//---------------------------------------------------------------

extern GamutElement		gamut_ele_pool[],
						*gamut_ele_ptr[];

void	build_gamut_table(void);
void	draw_gamut(SLONG x,SLONG y);

//---------------------------------------------------------------

#endif


