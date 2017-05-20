// Gamut.cpp
// Guy Simmons, 4th November 1997.

#include	"Engine.h"


GamutElement	gamut_ele_pool[MAX_GAMUT_RADIUS*4*MAX_GAMUT_RADIUS],
				*gamut_ele_ptr[MAX_GAMUT_RADIUS+2];

//---------------------------------------------------------------

void	build_gamut_table(void)
{
	SBYTE			*grid;
	SLONG			actual_radius,
					angle,
					count,
					dx,dz,
					old_radius		=	0,
					radius,
					radius_offset,
					sum_count		=	0;
	GamutElement	*ele_ptr;


	ele_ptr	=	gamut_ele_pool;

	grid	=	(SBYTE*)MemAlloc((MAX_GAMUT_RADIUS+1)*(MAX_GAMUT_RADIUS+1)*4);
	if(grid)
	{
		for(radius=(MAX_GAMUT_RADIUS<<2);radius>2;radius--)
		{
			if((radius>>2)!=old_radius)
			{
				old_radius					=	radius>>2;
				gamut_ele_ptr[radius>>2]	=	ele_ptr;
			}
			for(angle=0;angle<2048;angle+=4)
			{
				for(radius_offset=-4;radius_offset<4;radius_offset++)
				{	
					dx				=	(SIN(angle)*(radius+radius_offset))>>(16+2);
					dz				=	(COS(angle)*(radius+radius_offset))>>(16+2);
					actual_radius	=	Root((dx*dx)+(dz*dz));
					if(actual_radius==(radius>>2))
					{
						if(grid[(dx+MAX_GAMUT_RADIUS)+(dz+MAX_GAMUT_RADIUS)*(MAX_GAMUT_RADIUS*2)]!=-1)
						{
							grid[(dx+MAX_GAMUT_RADIUS)+(dz+MAX_GAMUT_RADIUS)*(MAX_GAMUT_RADIUS*2)]	=	-1;
							ele_ptr->DX		=	(SBYTE)dx;
							ele_ptr->DZ		=	(SBYTE)dz;
							ele_ptr->Angle	=	(SWORD)angle;
							ele_ptr++;
						}
					}
				}
			}
		}
		gamut_ele_ptr[0]	=	ele_ptr;
		MemFree(grid);
	}
	for(radius=1;radius<MAX_GAMUT_RADIUS;radius++)
	{
		ele_ptr		=	gamut_ele_ptr[radius];
		count		=	0;
		while(ele_ptr<gamut_ele_ptr[radius-1])
		{
			ele_ptr++;
			count++;
		}
		sum_count	+=	count;
	}
}

//---------------------------------------------------------------

void	draw_gamut(SLONG x,SLONG y)
{
	ULONG			c0;
	SLONG			scr_x,
					scr_y;
	GamutElement	*ele_ptr;


	for(c0=1;c0<MAX_GAMUT_RADIUS;c0++)
	{
		ele_ptr	=	gamut_ele_ptr[c0];
		while(ele_ptr<gamut_ele_ptr[c0-1])
		{
			scr_x	=	ele_ptr->DX+x;
			scr_y	=	ele_ptr->DZ+y;
			
//			DrawPixel(scr_x,scr_y,0xffff);

			ele_ptr++;
		}
	}
}

//---------------------------------------------------------------
