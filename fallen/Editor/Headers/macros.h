#ifndef	MACROS_H
#define	MACROS_H			1
#include	"math.h"
//#define	GetTickCount()	0
#ifdef	__MSC_VER
#define	sqrl(x)		(Root((SLONG)x))
#else
#define	sqrl(x)		(SLONG)(sqrt((float)x))
#endif

#define COL_TO_RGB565(col)	(((PALETTE[(col)*3]>>3)<<11)|((PALETTE[(col)*3+1]>>2)<<5)|(PALETTE[(col)*3+2]>>3))
#define COL_TO_RGB888(col)	((PALETTE[(col)*3]<<16)|(PALETTE[(col)*3+1]<<8)|PALETTE[(col)*3+2])

#define	RGB_TO_RGB565(r,g,b)	(((r>>3)<<11)|((g>>2)<<5)|(b>>3))
#define	RGB_TO_565(r,g,b)	(((r>>3)<<11)|((g>>2)<<5)|(b>>3))
#define	RGB_TO_RGB888(r,g,b)	((r<<16)|(g<<8)|(b))

#define	CLIP_256(x)			((x)>=256?255:(x))
#define	CLIPV_0(x)			if(x<0)x=0



#define	TO_CELL(x,z)		   &game_map->Cells[((x)>>8)+(((z)>>8)*MAX_GAME_MAP_WIDTH)]
#define	POLY_DATA_MAP(x,z)		game_map->Cells[((x)>>8)+(((z)>>8)*MAX_GAME_MAP_WIDTH)].PolyIndex
#define	THING_DATA_MAP(x,z)		game_map->Cells[((x)>>8)+(((z)>>8)*MAX_GAME_MAP_WIDTH)].ThingIndex

#define	TO_MAP(x,z)		&game_map->PolyIndex[((x)>>8)+(((z)>>8)*MAX_GAME_MAP_WIDTH)]
#define	TO_BMAP(x,z)		&game_map->PolyBackIndex[((x)>>10)+(((z)>>10)*BACK_MAP_WIDTH)]
#define	ON_MAP(x,z)		( ((x)>0) && ((x)<MAX_GAME_MAP_WIDTH*MAP_CELL_DEPTH) && ((z)>0) && ((z)<MAX_GAME_MAP_DEPTH*MAP_CELL_DEPTH) )

#define	DATA_MAP(x,z)		game_map->PolyIndex[((x)>>8)+(((z)>>8)*MAX_GAME_MAP_WIDTH)]
#define	DATA_BMAP(x,z)		game_map->PolyBackIndex[((x)>>10)+(((z)>>10)*BACK_MAP_WIDTH)]
#define	CLIP256(x)		(x>255?255:x)
inline	SLONG	QLEN(SLONG x1,SLONG z1,SLONG x2,SLONG z2)
{
	SLONG temp_dx;
	SLONG temp_dz;
	temp_dx=abs(x2-x1);
	temp_dz=abs(z2-z1);
	return(QDIST2(temp_dx,temp_dz));
}
inline	SLONG TLEN(SLONG x1,SLONG z1,SLONG x2,SLONG z2)
{
	SLONG temp_dx;
	SLONG temp_dz;
	temp_dx=abs(x2-x1);
	temp_dz=abs(z2-z1);
	return(sqrl(SDIST2(temp_dx,temp_dz)));
}
#endif