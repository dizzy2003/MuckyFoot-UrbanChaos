

#include	"game.h"

#include	"inside2.h"
#include	"memory.h"
#include	"mav.h"



//struct 	DInsideRect	inside_rect[MAX_INSIDE_RECT];

UWORD	next_inside_storey=1;
UWORD	next_inside_stair=1;
SLONG	next_inside_block=1;




SLONG find_inside_room(SLONG inside,SLONG x,SLONG z)
{
	
 	UBYTE	*rooms;
	SWORD	width;

	if(x>=inside_storeys[inside].MaxX || z>=inside_storeys[inside].MaxZ)
		return(0);						  
	if(x<inside_storeys[inside].MinX || z<inside_storeys[inside].MinZ)
		return(0);

	width=inside_storeys[inside].MaxX-inside_storeys[inside].MinX;
	rooms=&inside_block[inside_storeys[inside].InsideBlock];

	x-=inside_storeys[inside].MinX;
	z-=inside_storeys[inside].MinZ;

	z=z*width;
	return(rooms[x+z]&0xf);
}

SLONG find_inside_flags(SLONG inside,SLONG x,SLONG z)
{
	
	UBYTE	*rooms;
	SWORD	width;

	if(x>=inside_storeys[inside].MaxX || z>=inside_storeys[inside].MaxZ)
		return(0);
	if(x<inside_storeys[inside].MinX || z<inside_storeys[inside].MinZ)
		return(0);

	width=inside_storeys[inside].MaxX-inside_storeys[inside].MinX;
	rooms=&inside_block[inside_storeys[inside].InsideBlock];

	x-=inside_storeys[inside].MinX;
	z-=inside_storeys[inside].MinZ;

	z=z*width;
	return(rooms[x+z]);
}

SLONG	get_inside_alt(SLONG	inside)
{
/*
#ifndef	PSX
	CBYTE	str[100];
	sprintf(str," Inside %d alt %d \n",inside,inside_storeys[inside].StoreyY);
	CONSOLE_text(str);
#endif
*/
	return(inside_storeys[inside].StoreyY);
}



UBYTE	slide_inside_stair=0;
extern	SLONG	slide_door;

//
// mark's
//
SLONG person_slide_inside(
		SLONG  inside,
		SLONG  x1,
		SLONG  y1,
		SLONG  z1,
		SLONG *x2,
		SLONG *y2,
		SLONG *z2)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;

	SWORD mx = *x2 >> 16;
	SWORD mz = *z2 >> 16;
	SLONG	ret_val=0;

	slide_door = 0;
	slide_inside_stair=0;

	//
	// All the room surrounding us.
	//

	UBYTE id_here = find_inside_flags(inside, mx, mz);
	UBYTE id_xs	  = find_inside_flags(inside, mx - 1, mz);
	UBYTE id_xl	  = find_inside_flags(inside, mx + 1, mz);
	UBYTE id_zs	  = find_inside_flags(inside, mx, mz - 1);
	UBYTE id_zl	  = find_inside_flags(inside, mx, mz + 1);

	if((id_here&0xf)==0)
	{
		//
		// try to step outside of permitted area
		// return 1, to see if exited door
		//
		ret_val=1;
	}
	//
	// The width of the walls.
	// 

	#define INSIDE_RADIUS (50 << 8)

	//
	// Work out which wall we must collide with.
	//

	UBYTE col = 0;

	#define INSIDE_COL_XS (1 << 0)
	#define INSIDE_COL_XL (1 << 1)
	#define INSIDE_COL_ZS (1 << 2)
	#define INSIDE_COL_ZL (1 << 3)

	if ((id_here & 0xf) != (id_xs & 0xf)) {col |= INSIDE_COL_XS;}
	if ((id_here & 0xf) != (id_xl & 0xf)) {col |= INSIDE_COL_XL;}
	if ((id_here & 0xf) != (id_zs & 0xf)) {col |= INSIDE_COL_ZS;}
	if ((id_here & 0xf) != (id_zl & 0xf)) {col |= INSIDE_COL_ZL;}

	//
	// Collide with each edge of the mapsquare you are in that doesn't lead to
	// the same room.
	//

	if (col & INSIDE_COL_XS)
	{
		if ((*x2 & 0xffff) < INSIDE_RADIUS)
		{
			//
			// Is there a door here?
			//

			if (id_here & FLAG_DOOR_LEFT)
			{
				//
				// Collide with a door frame. An elipse at each end of the mapsquare.
				//

				if ((*z2 & 0xffff) > 0x8000)
				{
					dx   = *x2 & 0xffff;
					dz   = *z2 & 0xffff;
					dz   = 0x10000 - dz;
					dz >>= 1;				// To make it an elipse.

					dist = QDIST2(dx,dz);	// dx and dz are both > 0

					if (dist < INSIDE_RADIUS)
					{
						dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
						dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
					}

					dz <<= 1;				// Go back out of 'ellpise space'

				   *x2 &= ~0xffff;
				   *z2 &= ~0xffff;

				   *x2 |=  dx;
				   *z2 |=  0x10000 - dz;
				}
				else
				{
					dx   = *x2 & 0xffff;
					dz   = *z2 & 0xffff;
					dz >>= 1;				// To make it an elipse.

					dist = QDIST2(dx,dz);	// dx and dz are both > 0

					if (dist < INSIDE_RADIUS)
					{
						dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
						dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
					}

					dz <<= 1;				// Go back out of 'ellpise space'

				   *x2 &= ~0xffff;
				   *z2 &= ~0xffff;

				   *x2 |=  dx;
				   *z2 |=  dz;
				}
			}
			else
			{
				//
				// Collide with a wall.
				//

			   *x2 &= ~0xffff;
			   *x2 |=  INSIDE_RADIUS;
			}
		}
	}

	if (col & INSIDE_COL_XL)
	{
		if ((*x2 & 0xffff) > 0x10000 - INSIDE_RADIUS)
		{
			//
			// Is there a door here?
			//

			if (id_xl & FLAG_DOOR_LEFT)
			{
				//
				// Collide with a door frame. An elipse at each end of the mapsquare.
				//

				if ((*z2 & 0xffff) > 0x8000)
				{
					dx   = *x2 & 0xffff;
					dz   = *z2 & 0xffff;
					dx   = 0x10000 - dx;
					dz   = 0x10000 - dz;
					dz >>= 1;				// To make it an elipse.

					dist = QDIST2(dx,dz);	// dx and dz are both > 0

					if (dist < INSIDE_RADIUS)
					{
						dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
						dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
					}

					dz <<= 1;				// Go back out of 'ellpise space'

				   *x2 &= ~0xffff;
				   *z2 &= ~0xffff;

				   *x2 |=  0x10000 - dx;
				   *z2 |=  0x10000 - dz;
				}
				else
				{
					dx   = *x2 & 0xffff;
					dx   = 0x10000 - dx;
					dz   = *z2 & 0xffff;
					dz >>= 1;				// To make it an elipse.

					dist = QDIST2(dx,dz);	// dx and dz are both > 0

					if (dist < INSIDE_RADIUS)
					{
						dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
						dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
					}

					dz <<= 1;				// Go back out of 'ellpise space'

				   *x2 &= ~0xffff;
				   *z2 &= ~0xffff;

				   *x2 |= 0x10000 - dx;
				   *z2 |= dz;
				}
			}
			else
			{
				*x2 &= ~0xffff;
				*x2 |=  0x10000 - INSIDE_RADIUS;
			}
		}
	}

	if (col & INSIDE_COL_ZS)
	{
		if ((*z2 & 0xffff) < INSIDE_RADIUS)
		{
			//
			// Is there a door here?
			//

			if (id_here & FLAG_DOOR_UP)
			{
				//
				// Collide with a door frame. An elipse at each end of the mapsquare.
				//

				if ((*x2 & 0xffff) > 0x8000)
				{
					dx   = *x2 & 0xffff;
					dz   = *z2 & 0xffff;
					dx   = 0x10000 - dx;
					dx >>= 1;				// To make it an elipse.

					dist = QDIST2(dx,dz);	// dx and dz are both > 0

					if (dist < INSIDE_RADIUS)
					{
						dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
						dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
					}

					dx <<= 1;				// Go back out of 'ellpise space'

				   *x2 &= ~0xffff;
				   *z2 &= ~0xffff;

				   *x2 |=  0x10000 - dx;
				   *z2 |=  dz;
				}
				else
				{
					dx   = *x2 & 0xffff;
					dz   = *z2 & 0xffff;
					dx >>= 1;				// To make it an elipse.

					dist = QDIST2(dx,dz);	// dx and dz are both > 0

					if (dist < INSIDE_RADIUS)
					{
						dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
						dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
					}

					dx <<= 1;				// Go back out of 'ellpise space'

				   *x2 &= ~0xffff;
				   *z2 &= ~0xffff;

				   *x2 |= dx;
				   *z2 |= dz;
				}
			}
			else
			{
				*z2 &= ~0xffff;
				*z2 |=  INSIDE_RADIUS;
			}
		}
	}

	if (col & INSIDE_COL_ZL)
	{
		if ((*z2 & 0xffff) > 0x10000 - INSIDE_RADIUS)
		{
			//
			// Is there a door here?
			//

			if (id_zl & FLAG_DOOR_UP)
			{
				//
				// Collide with a door frame. An elipse at each end of the mapsquare.
				//

				if ((*x2 & 0xffff) > 0x8000)
				{
					dx   = *x2 & 0xffff;
					dz   = *z2 & 0xffff;
					dx   = 0x10000 - dx;
					dz   = 0x10000 - dz;
					dx >>= 1;				// To make it an elipse.

					dist = QDIST2(dx,dz);	// dx and dz are both > 0

					if (dist < INSIDE_RADIUS)
					{
						dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
						dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
					}

					dx <<= 1;				// Go back out of 'ellpise space'

				   *x2 &= ~0xffff;
				   *z2 &= ~0xffff;

				   *x2 |=  0x10000 - dx;
				   *z2 |=  0x10000 - dz;
				}
				else
				{
					dx   = *x2 & 0xffff;
					dz   = *z2 & 0xffff;
					dz   = 0x10000 - dz;
					dx >>= 1;				// To make it an elipse.

					dist = QDIST2(dx,dz);	// dx and dz are both > 0

					if (dist < INSIDE_RADIUS)
					{
						dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
						dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
					}

					dx <<= 1;				// Go back out of 'ellpise space'

				   *x2 &= ~0xffff;
				   *z2 &= ~0xffff;

				   *x2 |= dx;
				   *z2 |= 0x10000 - dz;
				}
			}
			else
			{
				*z2 &= ~0xffff;
				*z2 |=  0x10000 - INSIDE_RADIUS;
			}
		}
	}

	//
	// Sliding against sticky-outy corner bits.
	//

	if (!(col & (INSIDE_COL_XS|INSIDE_COL_ZS)))
	{
		if ((id_here & 0xf) != (find_inside_flags(inside, mx - 1, mz - 1) & 0xf))
		{
			dx = *x2 & 0xffff;
			dz = *z2 & 0xffff;

			dist = QDIST2(dx,dz);

			if (dist < INSIDE_RADIUS)
			{
				dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
				dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
			}

			*x2 &= ~0xffff;
			*z2 &= ~0xffff;

			*x2 |= dx;
			*z2 |= dz;
		}
	}

	if (!(col & (INSIDE_COL_XS|INSIDE_COL_ZL)))
	{
		if ((id_here & 0xf) != (find_inside_flags(inside, mx - 1, mz + 1) & 0xf))
		{
			dx = *x2 & 0xffff;
			dz = *z2 & 0xffff;
			dz = 0x10000 - dz;

			dist = QDIST2(dx,dz);

			if (dist < INSIDE_RADIUS)
			{
				dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
				dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
			}

			*x2 &= ~0xffff;
			*z2 &= ~0xffff;

			*x2 |= dx;
			*z2 |= 0x10000 - dz;
		}
	}

	if (!(col & (INSIDE_COL_XL|INSIDE_COL_ZS)))
	{
		if ((id_here & 0xf) != (find_inside_flags(inside, mx + 1, mz - 1) & 0xf))
		{
			dx = *x2 & 0xffff;
			dz = *z2 & 0xffff;
			dx = 0x10000 - dx;

			dist = QDIST2(dx,dz);

			if (dist < INSIDE_RADIUS)
			{
				dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
				dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
			}

			*x2 &= ~0xffff;
			*z2 &= ~0xffff;

			*x2 |= 0x10000 - dx;
			*z2 |= dz;
		}
	}

	if (!(col & (INSIDE_COL_XL|INSIDE_COL_ZL)))
	{
		if ((id_here & 0xf) != (find_inside_flags(inside, mx + 1, mz + 1) & 0xf))
		{
			dx = *x2 & 0xffff;
			dz = *z2 & 0xffff;
			dx = 0x10000 - dx;
			dz = 0x10000 - dz;

			dist = QDIST2(dx,dz);

			if (dist < INSIDE_RADIUS)
			{
				dx = dx * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
				dz = dz * (INSIDE_RADIUS >> 8) / ((dist >> 8) + 1);
			}

			*x2 &= ~0xffff;
			*z2 &= ~0xffff;

			*x2 |= 0x10000 - dx;
			*z2 |= 0x10000 - dz;
		}
	}

	//
	// I wander what we are meant to be returning!
	//

	{
		UBYTE id_here = find_inside_flags(inside, *x2>>16, *z2>>16);
		if(id_here&FLAG_INSIDE_STAIR)
		{
			slide_inside_stair=id_here&FLAG_INSIDE_STAIR;
		}

	}

	return (ret_val);
}


/*
SLONG	find_stair_routes(UWORD inside,UBYTE x,UBYTE z,UWORD *up,UWORD *down)
{
	SLONG	stair;
//	ASSERT(0);

	stair=inside_storeys[inside].StairCaseHead;
	while(stair)
	{
		UBYTE	sx,sz;
		UBYTE	dir;

		sx=inside_stairs[stair].X;
		sz=inside_stairs[stair].Z;

		dir=GET_STAIR_DIR(inside_stairs[stair].Flags);


		if(x==sx && z==sz)
		{
			*up=inside_stairs[stair].UpInside;
			*down=inside_stairs[stair].DownInside;
			return(1);
		}

		switch(dir)
		{
			case	0:
				//n
				sz--;
				break;
			case	1:
				//e
				sx++;
				break;
			case	2:
				//s
				sz++;
				break;
			case	3:
				//w
				sx--;
				break;

		}

		if(x==sx && z==sz)
		{
			*up=inside_stairs[stair].UpInside;
			*down=inside_stairs[stair].DownInside;
			return(1);
		}
		stair=inside_stairs[stair].NextStairs;
	}
	return(0);
}
*/
UWORD	find_stair_in(SLONG mx,SLONG mz,SLONG *rdx,SLONG *rdz,UWORD	inside)
{
	SLONG	dx,dz;
	SLONG	stair;
	SLONG	x_ok,z_ok;
//	ASSERT(0);


	stair=inside_storeys[inside].StairCaseHead;
	while(stair)
	{
		UBYTE	sx,sz;
		UBYTE	dir;

		sx=inside_stairs[stair].X;
		sz=inside_stairs[stair].Z;

		dir=GET_STAIR_DIR(inside_stairs[stair].Flags);


		dx=0;
		dz=0;

		switch(dir)
		{
			case	0:
				dz=-2;
				dx=1;
				//n
				break;
			case	1:
				dx=2;
				dz=1;
				//e
				break;
			case	2:
				dz=2;
				dx=-1;
				//s
				break;
			case	3:
				dx=-2;
				dz=-1;
				//w
				break;

		}

		x_ok=0;
		z_ok=0;
		if(dx>0)
		{
			if( mx>=sx && mx<sx+dx)
			{
				x_ok=1;
			}
		}
		else
		{
			if(mx>=sx+dx && mx<sx)
			{
				x_ok=1;
			}
		}

		if(x_ok)
		{
			if(dz>0)
			{
				if( mz>=sz && mz<sz+dz)
				{
					z_ok=1;
				}
			}
			else
			{
				if(mz>=sz+dz && mz<sz)
				{
					z_ok=1;
				}
			}

			if(x_ok&&z_ok)
			{
				*rdx=dx;
				*rdz=dz;
				return(stair);
			}


		}
		stair=inside_stairs[stair].NextStairs;
	}
	return(0);

}

SLONG	find_stair_y(Thing *p_person,SLONG *y1,SLONG x,SLONG y,SLONG z,UWORD *new_floor)
{
	SLONG	dx,dz;
	SLONG	stair;
	SLONG	off_x,off_z;
	SLONG	mx,mz;
	SLONG	s,t;
	SLONG	new_y;
	SLONG	can_move=1;

	MSG_add(" INDOORS INDEX %d  INDEX NEXT %d \n",INDOORS_INDEX,INDOORS_INDEX_NEXT);
#ifndef TARGET_DC
	INDOORS_INDEX_NEXT=0;
	INDOORS_INDEX_FADE=255;
#endif


	*new_floor=0;

	mx=x>>8;
	mz=z>>8;

	stair=find_stair_in(mx,mz,&dx,&dz,p_person->Genus.Person->InsideIndex);

	if(stair)
	{
		off_x=x-(inside_stairs[stair].X<<8);
		off_z=z-(inside_stairs[stair].Z<<8);

//		MSG_add(" find stair y dx %d dz %d off_x %d off_z %d \n",dx,dz,off_x,off_z);

		//
		// do this super quick way of getting s & t without muls or divs
		//

		if(abs(dx)==2)
		{
			off_x>>=1;
		}
		if(abs(dz)==2)
		{
			off_z>>=1;
		}

		s=abs(off_x);
		t=abs(off_z);
		if(abs(dx)==2)
		{
			SWAP(s,t);
		}
//		MSG_add("ox %d oz %d s %d t %d \n",off_x,off_z,s,t);

		//
		// given s and t are we over a step 
		//

		if(inside_stairs[stair].UpInside)
		{
			if(s<128 && t>64)
			{
				if(t>192)
				{
					new_y=256;
				}
				else
				{
					new_y=(t-64)>>3;
					new_y<<=4;
				}
				new_y+=inside_storeys[INDOORS_INDEX].StoreyY;

				MSG_add(" inside  new_y = %d old y %d \n",new_y,y);
				if(abs(new_y-y)<96)
				{
					//
					// go up the stairs
					//

					if(t>128)
					{
						//
						// ping up the stairs
						//
						MSG_add(" NEW FLOOR up %d prev %d\n",inside_stairs[stair].UpInside,INDOORS_INDEX);
						*new_floor=inside_stairs[stair].UpInside;
						MSG_add(" upthe stairs is index %d \n",inside_stairs[stair].UpInside);

#ifdef TARGET_DC
						ASSERT(FALSE);
#else
						INDOORS_INDEX_FADE=0;
						INDOORS_INDEX_NEXT=INDOORS_INDEX;
						INDOORS_ROOM_NEXT =find_inside_room(INDOORS_INDEX_NEXT,x>>8,z>>8);
#endif
					}
					*y1=new_y;
					return(1);
				}
				else
				{
					can_move=0;
				}
			}
		}
		if(inside_stairs[stair].DownInside)
		{
			if(s<128 && t>64)
			{
				if(t>192)
				{
					new_y=0;
				}
				else
				{
					new_y=(t-64)>>4;
					new_y<<=5;
					new_y-=256;
				}
				new_y+=inside_storeys[INDOORS_INDEX].StoreyY;
				if(abs(new_y-y)<96)
				{
					//
					// go down the stairs
					//

					if(t<128)
					{
						//
						// ping down the stairs
						//
						*new_floor=inside_stairs[stair].DownInside;
						MSG_add(" NEW FLOOR down %d prev %d\n",inside_stairs[stair].DownInside,INDOORS_INDEX);
					}
					else
					{

#ifdef TARGET_DC
						ASSERT(FALSE);
#else
						if(t<192)
						{
							INDOORS_INDEX_FADE=MIN(((t-128)<<2),255);

							INDOORS_INDEX_NEXT=inside_stairs[stair].DownInside;
							INDOORS_ROOM_NEXT = find_inside_room(INDOORS_INDEX_NEXT,x>>8,z>>8);
						}
						else
						{
							INDOORS_INDEX_NEXT=0;
							INDOORS_ROOM_NEXT =0;
							INDOORS_INDEX_FADE=255; //MIN(((t)<<1),255);

						}
#endif


					}
					*y1=new_y;
					return(1);
				}
				else
				{
					can_move=0;
				}
			}
		}
	}

	new_y=inside_storeys[INDOORS_INDEX].StoreyY;
	*y1=new_y;
	return(can_move);
	
}
/*
void	stair_teleport_bodge(Thing *p_person)
{
	SLONG	x,z;
	UWORD	up,down;
	SLONG	found;

	x=p_person->WorldPos.X>>16;
	z=p_person->WorldPos.Z>>16;

	found=find_stair_routes(p_person->Genus.Person->InsideIndex,x,z,&up,&down);



	if(found)
	{
//		ASSERT(0);
		if(up&&(slide_inside_stair&FLAG_INSIDE_STAIR_UP))
		{
			MSG_add(" go upstairs from %d to %d \n",INDOORS_INDEX,up);
			INDOORS_INDEX=up;
		}
		if(down&&(slide_inside_stair&FLAG_INSIDE_STAIR_DOWN))
		{
			MSG_add(" go upstairs from %d to %d \n",INDOORS_INDEX,up);
			INDOORS_INDEX=down;
		}
//		if(down)
//			INDOORS_INDEX=down;

		p_person->Genus.Person->InsideIndex=INDOORS_INDEX;
		INDOORS_ROOM=find_inside_room(INDOORS_INDEX,x,z);
		p_person->WorldPos.Y=get_inside_alt(INDOORS_INDEX);

	}

}
*/



#if 0

// ========================================================
//
// INSIDE NAVIGATION
//
// ========================================================


//
// A buffer where we store a 2D MAV_nav array for a floor while we
// navigating on it.
//

#define INSIDE2_MAX_NAV (32 * 32)

UWORD INSIDE2_mav_nav[INSIDE2_MAX_NAV];
SLONG INSIDE2_mav_nav_pitch;
SLONG INSIDE2_mav_nav_inside;	// The inside the mav_nav array is currently storing.

//
// Calculates the mav nav array for the given storey
//

void INSIDE2_mav_nav_calc(SLONG inside)
{
	UBYTE i;
	UBYTE mo_index;
	UBYTE door;
	UBYTE width;
	UBYTE height;
	UBYTE here;
	UBYTE there;
	UBYTE x;
	UBYTE z;

	InsideStorey *is;

	if (INSIDE2_mav_nav_inside == inside)
	{
		//
		// We've already got this inside worked out.
		//

		return;
	}

	is = &inside_storeys[inside];

	width  = is->MaxX - is->MinX;
	height = is->MaxZ - is->MinZ;

	INSIDE2_mav_nav_pitch  = height;
	INSIDE2_mav_nav_inside = inside;

	//
	// Make sure we have enough room.
	//

	ASSERT(width * height <= INSIDE2_MAX_NAV);

	for (x = is->MinX; x < is->MaxX; x++)
	for (z = is->MinX; z < is->MaxX; z++)
	{
		here = find_inside_flags(inside, x,z);

		mo_index = 0;

		for (i = 0; i < 4; i++)
		{
			switch(i)
			{
				case MAV_DIR_XS: there = find_inside_flags(inside, x - 1, z); break;
				case MAV_DIR_XL: there = find_inside_flags(inside, x + 1, z); break;
				case MAV_DIR_ZS: there = find_inside_flags(inside, x, z - 1); break;
				case MAV_DIR_ZL: there = find_inside_flags(inside, x, z + 1); break;
			}

			if ((there & 0xf) == 0)
			{
				//
				// Cant go outside the floorplan.
				//
			}
			else
			if ((there & 0xf) != (here & 0xf))
			{
				//
				// 'here' and 'there' are in different rooms. Only allow a navigation
				// if there is a door connecting the two squares.
				//

				door = FALSE;

				switch(i)
				{
					case MAV_DIR_XS: door = here  & FLAG_DOOR_LEFT; break;
					case MAV_DIR_XL: door = there & FLAG_DOOR_LEFT; break;
					case MAV_DIR_ZS: door = here  & FLAG_DOOR_UP;   break;
					case MAV_DIR_ZL: door = there & FLAG_DOOR_UP;   break;
				}

				if (door)
				{
					//
					// We can do between the two squares.
					//

					mo_index |= 1 << i;
				}
			}
			else
			{
				//
				// The two squares are in the same room.
				//

				mo_index |= 1 << i;
			}
		}

		ASSERT(WITHIN(mo_index, 0, 15));

		//
		// The first 16 MAV_opt[]s are guaranteed to be able to 
		// indexed with a lookup like mo_index.
		//

		INSIDE2_mav_nav[x * INSIDE2_mav_nav_pitch + z] = mo_index;
	}
}

//
// Backs up the current MAV_nav array and replaces it with
// our INSIDE2_mav_nav array.
//

UWORD *INSIDE2_backup_mav_nav;
SLONG  INSIDE2_backup_mav_nav_pitch;

void INSIDE2_setup_mav(void)
{
	INSIDE2_backup_mav_nav       = MAV_nav;
	INSIDE2_backup_mav_nav_pitch = MAV_nav_pitch;

	MAV_nav       = INSIDE2_mav_nav;
	MAV_nav_pitch = INSIDE2_mav_nav_pitch;
}

void INSIDE2_restore_mav(void)
{
	MAV_nav       = INSIDE2_backup_mav_nav;
	MAV_nav_pitch =	INSIDE2_backup_mav_nav_pitch;
}



//
// Finds all the doors for the given building.
//

#define INSIDE2_MAX_DOORS 2

struct
{
	UBYTE out_x;
	UBYTE out_z;

	UBYTE in_x;
	UBYTE in_z;

	SWORD y;

}     INSIDE2_door[INSIDE2_MAX_DOORS];
SLONG INSIDE2_door_upto;
SLONG INSIDE2_door_building;

void INSIDE2_find_doors(SLONG building)
{
	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;
	SLONG mx;
	SLONG mz;
	SLONG dx;
	SLONG dz;

	SLONG facet;

	DBuilding *db;
	DFacet    *df;

	if (building == INSIDE2_door_building)
	{
		//
		// We have already worked out these door positions.
		//

		return;
	}

	INSIDE2_door_upto     = 0;
	INSIDE2_door_building = building;

	//
	// Go through all the facets of this building.
	//

	db = &dbuildings[building];

	for (facet = db->StartFacet; facet < db->EndFacet; facet++)
	{
		df = &dfacets[facet];

		if (df->FacetType == STOREY_TYPE_DOOR)
		{
			//
			// This is a door. Where should we mavigate to, to be
			// outside the door?
			//

			x1 = df->x[0] << 8;
			z1 = df->z[0] << 8;

			x2 = df->x[1] << 8;
			z2 = df->z[1] << 8;

			dx = x2 - x1 >> 1;
			dz = z2 - z1 >> 1;

			if (!WITHIN(INSIDE2_door_upto, 0, INSIDE2_MAX_DOORS - 1))
			{
				//
				// Already found too many doors!
				//

				break;
			}

			//
			// The mapsquares the outside and inside of this door are on.
			//

			INSIDE2_door[INSIDE2_door_upto].out_x = x1 + dx + dz >> 8;
			INSIDE2_door[INSIDE2_door_upto].out_z = z1 + dz - dx >> 8;

			INSIDE2_door[INSIDE2_door_upto].in_x = x1 + dx - dz >> 8;
			INSIDE2_door[INSIDE2_door_upto].in_z = z1 + dz + dx >> 8;

			INSIDE2_door[INSIDE2_door_upto].y = df->Y[0];

			INSIDE2_door_upto += 1;
		}
	}
}



MAV_Action INSIDE2_mav_enter(Thing *p_person, SLONG inside, UBYTE caps)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG score;

	UBYTE best_x;
	UBYTE best_z;
	SLONG best_score;

	MAV_Action ma;

	//
	// Find the doors of this building.
	//

	INSIDE2_find_doors(inside_storeys[inside].Building);

	//
	// Which is the nearest door?
	//

	best_x     = p_person->WorldPos.X >> 16;
	best_z     = p_person->WorldPos.Z >> 16;
	best_score = INFINITY;	// The lower the score the better.

	for (i = 0; i < INSIDE2_door_upto; i++)
	{
		dx = INSIDE2_door[i].out_x - (p_person->WorldPos.X >> 16);
		dz = INSIDE2_door[i].out_z - (p_person->WorldPos.Z >> 16);

		dy = INSIDE2_door[i].y - (p_person->WorldPos.Y >> 8);

		//
		// Change in y is more important than (dx,dz)... but (dx,dz) are
		// in a different scale.
		//

		score  = dx + dz;
		score += dy >> 5;

		if (best_score > score)
		{
			best_score = score;
			best_x     = INSIDE2_door[i].out_x;
			best_z     = INSIDE2_door[i].out_z;
		}
	}

	//
	// MAVigate to the door.
	//

	ma = MAV_do(
			p_person->WorldPos.X >> 16,
			p_person->WorldPos.Z >> 16,
			best_x,
			best_z,
			caps);	

	return ma;
}

MAV_Action INSIDE2_mav_inside(Thing *p_person, SLONG inside, UBYTE x, UBYTE z)
{
	SLONG start_x;
	SLONG start_z;

	MAV_Action ma;

	InsideStorey *is;

	is = &inside_storeys[inside];

	//
	// Where do we start and end? Make sure they are inside this inside.
	//

	start_x = p_person->WorldPos.X >> 16;
	start_z = p_person->WorldPos.Z >> 16;

	ASSERT(WITHIN(start_x, is->MinX, is->MaxX - 1));
	ASSERT(WITHIN(start_z, is->MinZ, is->MaxZ - 1));

	ASSERT(WITHIN(x, is->MinX, is->MaxX - 1));
	ASSERT(WITHIN(z, is->MinZ, is->MaxZ - 1));

	//
	// The navigation for this inside.
	//

	ASSERT(0);	// ma is never initialized

	return ma;
}

MAV_Action INSIDE2_mav_stair(Thing *p_person, SLONG inside, SLONG new_inside)
{
	MAV_Action ma;

	ma.action = MAV_ACTION_GOTO;
	ma.dest_x = p_person->WorldPos.X > 16;
	ma.dest_z = p_person->WorldPos.Z > 16;
	ma.dir    = 0;

	return ma;
}

MAV_Action INSIDE2_mav_exit(Thing *p_person, SLONG inside)
{
	MAV_Action ma;

	ma.action = MAV_ACTION_GOTO;
	ma.dest_x = p_person->WorldPos.X > 16;
	ma.dest_z = p_person->WorldPos.Z > 16;
	ma.dir    = 0;

	return ma;
}


#endif