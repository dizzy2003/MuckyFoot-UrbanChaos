#include	"Game.h"
//#include	"fallen/editor/headers/collide.hpp"
#include	"fallen/editor/headers/map.h"
#include	"animate.h"
#include	"dirt.h"
#include	"fog.h"
#include	"mist.h"
#include	"water.h"
#include	"statedef.h"
#include	"mav.h"
#include	"pap.h"
#include	"supermap.h"
#include	"ob.h"
//#include	"id.h"
#include	"ns.h"
#include	"mav.h"
#include	"build2.h"
#ifndef		PSX
#include	"fallen/DDEngine/Headers/console.h"
#endif
#include	"person.h"
#include	"sound.h"
#include	"interact.h"
#include	"fallen/headers/inside2.h"
#include	"barrel.h"
#include	"walkable.h"
#include	"fc.h"
#include	"memory.h"
#include	"ware.h"
#include	"mfx.h"
#include	"pcom.h"



extern	UBYTE	cheat;

#undef  BLOCK_SIZE
#define BLOCK_SIZE (1 << 6)

#ifdef	DOG_POO
struct	CollisionVectLink	col_vects_links[MAX_COL_VECT_LINK]; //40K
struct	CollisionVect		col_vects[MAX_COL_VECT];            //300K
#endif

void	highlight_quad(SLONG face,SLONG face_x,SLONG face_y,SLONG face_z);
void	add_debug_line(SLONG x1,SLONG my_y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG colour);
void sweep_feet(Thing *p_person,Thing *p_aggressor,SLONG  death_type); //people.cpp

SLONG collide_with_circle(SLONG cx,SLONG cz,SLONG cradius,SLONG *x2,SLONG *z2);
SLONG	get_fence_hole(struct DFacet *p_facet);
SLONG	get_fence_hole_next(struct DFacet *p_facet,SLONG along);

extern BOOL allow_debug_keys;

inline	SLONG slide_around_box_lowstack(SLONG box_mid_x,SLONG box_mid_z,SLONG box_min_x,SLONG box_min_z,	SLONG box_max_x,SLONG box_max_z,SLONG box_yaw,SLONG radius,SLONG  x1,SLONG  z1,SLONG *x2,SLONG *z2);

//
// These are defined in person.cpp- I couldn't bear a complete rebuild.
//

SLONG get_fence_bottom(SLONG x, SLONG z, SLONG col);
SLONG get_fence_top   (SLONG x, SLONG z, SLONG col);



UWORD	next_col_vect=1;
UWORD	next_col_vect_link=1;

#ifdef	PSX
7SLONG MUL64(SLONG i,SLONG j)
{
	SLONG	res;
	SLONG	work;
//	asm( " add %0,%1,%2 " : "=r"(k) : "r"(i) ,"r"(j) );
//	asm( " add %0,%1,$0 " : "=r"(k) : "r"(i) );

	asm( " mult %0,%1 " : : "r"(i) ,"r"(j) : "hi","lo" );
	asm( " mflo %0 " : "=r"(work) );
	asm( " srl %0,%1,%2 " : "=r"(res) : "r"(work),"r"(16) );

	asm( " mfhi %0 " : "=r"(work) );
	asm( " sll %0,%1,%2 " : "=r"(work) : "r"(work),"r"(16) );
	asm( " or %0,%1,%2 " : "=r"(res) : "r"(res),"r"(work) );


	return(res);
}
#endif

///////////////////////////
//
//	   		 o
//      xox	oxx
//		xooo ox
//		 xo
//		  xx   x

#ifdef	EDITOR
struct	WalkLink	walk_links[MAX_WALK_POOL];	//120K

UWORD	next_walk_link=1;
#endif

//extern	SLONG	do_move_collide(SLONG x,SLONG y,SLONG z,SLONG dx,SLONG dy,SLONG dz,SLONG cell_dx,SLONG cell_dz,SLONG	scale_move);
//extern	SLONG	do_move_collide_circle(SLONG x,SLONG y,SLONG z,SLONG len,SLONG cell_dx,SLONG cell_dz);
//extern	SLONG dist_to_line(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b);
//extern	void nearest_point_on_line(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b,SLONG *ret_x,SLONG *ret_z);

extern	void	e_draw_3d_line(SLONG x1,SLONG my_y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
extern	void	e_draw_3d_mapwho(SLONG x1,SLONG z1);
extern	void	e_draw_3d_mapwho_y(SLONG x1,SLONG my_y1,SLONG z1);


/**************************************************************
 *                                                            *
 *    NOTE:  The following macro to determine if two numbers  *
 *    have the same sign, is for 2's complement number        *
 *    representation.  It will need to be modified for other  *
 *    number systems.                                         *
 *                                                            *
 **************************************************************/

#define SAME_SIGNS( a, b )	(((SLONG) ((ULONG) a ^ (ULONG) b)) >= 0 )

//
// Lines that share a point count as intersecting.
//
#ifdef	UNUSED
SLONG lines_intersect(SLONG x1,SLONG  my_y1,SLONG x2,SLONG  y2,SLONG x3,SLONG  y3,SLONG x4,SLONG  y4,SLONG *x,SLONG  *y)
{
    long a1, a2, b1, b2, c1, c2;
    long r1, r2, r3, r4;
    long denom, offset, num;

    a1 = y2 - my_y1;
    b1 = x1 - x2;
    c1 = x2 * my_y1 - x1 * y2;

    r3 = a1 * x3 + b1 * y3 + c1;
    r4 = a1 * x4 + b1 * y4 + c1;

    if ( r3 != 0 &&
         r4 != 0 &&
         SAME_SIGNS( r3, r4 ))
        return ( DONT_INTERSECT );


    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;


    r1 = a2 * x1 + b2 * my_y1 + c2;
    r2 = a2 * x2 + b2 * y2 + c2;

    if ( r1 != 0 &&
         r2 != 0 &&
         SAME_SIGNS( r1, r2 ))
        return ( DONT_INTERSECT );

    denom = a1 * b2 - a2 * b1;
    if ( denom == 0 )
        return ( COLLINEAR );
    offset = denom < 0 ? - denom / 2 : denom / 2;

    num = b1 * c2 - b2 * c1;
    *x = ( num < 0 ? num - offset : num + offset ) / denom;

    num = a2 * c1 - a1 * c2;
    *y = ( num < 0 ? num - offset : num + offset ) / denom;

    return ( DO_INTERSECT );
}
#endif

UBYTE	two4_line_intersection(SLONG x1,SLONG my_y1,SLONG x2,SLONG y2,SLONG x3,SLONG y3,SLONG x4,SLONG y4)
{
	SLONG	ax,bx,cx,ay,by,cy,d,e,f; //,offset;
	short	x1lo,x1hi; //,x3lo,x3hi;
	short	y1lo,y1hi; //,y3lo,y3hi;

	ax=x2-x1;
	bx=x3-x4;

	if(ax<0)
	{
		x1lo=(SWORD)x2;
		x1hi=(SWORD)x1;
	}
	else
	{
		x1hi=(SWORD)x2;
		x1lo=(SWORD)x1;
	}

	if(bx>0)
	{
		if(x1hi < (SWORD)x4 || (SWORD)x3 < x1lo)
			return(0);
	}
	else
	{
			if(x1hi < (SWORD)x3 || (SWORD)x4 < x1lo)
				return(0);
	}

	ay=y2-my_y1;
	by=y3-y4;

	if(ay<0)
	{
		y1lo=(SWORD)y2;
		y1hi=(SWORD)my_y1;
	}
	else
	{
		y1hi=(SWORD)y2;
		y1lo=(SWORD)my_y1;
	}

	if(by>0)
	{
		if(y1hi<(SWORD)y4 || (SWORD)y3<y1lo)
			return(0);
	}
	else
	{
		if(y1hi<(SWORD)y3 || (SWORD)y4<y1lo)
			return(0);
	}

	cx=x1-x3;
	cy=my_y1-y3;

	d=by*cx-bx*cy;
	f=ay*bx-ax*by;
	if(f>0)
	{
		if(d<0||d>f)
			return(0);
	}
	else
	{
		if(d>0||d<f)
			return(0);
	}

	e=ax*cy-ay*cx;

	if(f>0)
	{
		if(e<0||e>f)
			return(0);
	}
	else
	{
		if(e>0||e<f)
			return(0);
	}
		if(f==0)
			return(1);

	return(2);

}

void	clear_all_col_info(void)
{
#ifdef	DOG_POO
#ifdef	EDITOR
	SLONG	dx,dy,dz;
	for(dx=0;dx<EDIT_MAP_WIDTH;dx++)
	for(dy=0;dy<EDIT_MAP_HEIGHT;dy++)
		edit_map[dx][dy].ColVectHead=0;
	memset((UBYTE *)col_vects,0,sizeof(struct CollisionVect)*MAX_COL_VECT);
	memset((UBYTE *)col_vects_links,0,sizeof(struct CollisionVectLink)*MAX_COL_VECT_LINK);
	next_col_vect=1;
	next_col_vect_link=1;
#endif
#endif

}

#ifndef	PSX
#ifndef TARGET_DC
SLONG get_height_along_vect(SLONG ax,SLONG az,struct CollisionVect *p_vect)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG x1;
	SLONG my_y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;


	SLONG along,y;




	x1=p_vect->X[0];
	my_y1=p_vect->Y[0];
	z1=p_vect->Z[0];

	x2=p_vect->X[1];
	y2=p_vect->Y[1];
	z2=p_vect->Z[1];




	dx = x2 - x1;
	dy = y2 - my_y1;
	dz = z2 - z1;

	ax -= x1;
	az -= z1;




	if (abs(dx) > abs(dz))
	{
		if (dx==0)
		{
			dz = 1;
		}

		along = (ax << 8) / dx;
	}
	else
	{
		if(dz==0)
		{
			dz=1;
		}

		along = (az << 8) / dz;
	}

	y=my_y1+((dy*along)>>8);


	return y;
}

SLONG get_height_along_facet(SLONG ax,SLONG az,struct DFacet *p_facet)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG x1;
	SLONG my_y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG along,y;

	//
	// A quick early-out...
	//

	if (p_facet->Y[0] == p_facet->Y[1])
	{
		return p_facet->Y[0];
	}

	x1=p_facet->x[0] << 8;
	my_y1=p_facet->Y[0];
	z1=p_facet->z[0] << 8;

	x2=p_facet->x[1] << 8;
	y2=p_facet->Y[1];
	z2=p_facet->z[1] << 8;

	dx = x2 - x1;
	dy = y2 - my_y1;
	dz = z2 - z1;

	ax -= x1;
	az -= z1;

	if (abs(dx) > abs(dz))
	{
		if (dx==0)
		{
			dz = 1;
		}

		along = (ax << 8) / dx;
	}
	else
	{
		if(dz==0)
		{
			dz=1;
		}

		along = (az << 8) / dz;
	}

	y=my_y1+((dy*along)>>8);

	return y;
}
#endif
#endif
#ifdef	DOG_POO
ULONG	add_collision_to_single_cell(UWORD	index,SLONG x,SLONG z)
{
	if(x<0||z<0||x>=MAP_WIDTH||z>=MAP_HEIGHT)
		return(0);

	if(col_vects_links[MAP2(x,z).ColVectHead].VectIndex==index)
		return(0);


//	LogText(" add collision link %d to map %d %d  (maphead %d)\n",next_col_vect_link,x,z,MAP2(x,z).ColVectHead);

	col_vects_links[next_col_vect_link].Next=MAP2(x,z).ColVectHead;
	col_vects_links[next_col_vect_link].VectIndex=index;

	MAP2(x,z).ColVectHead=next_col_vect_link;

	next_col_vect_link++;

	return(1);
}

//
// Returns TRUE if it actually removed something.
//

ULONG remove_collision_from_single_cell(UWORD index, SLONG x, SLONG z)
{
	UWORD *prev;
	UWORD  next;

	if(x<0||z<0||x>=MAP_WIDTH||z>=MAP_HEIGHT)
	{
		return(0);
	}

	prev = &MAP2(x,z).ColVectHead;
	next =  MAP2(x,z).ColVectHead;

	while(next)
	{
		if (col_vects_links[next].VectIndex == index)
		{
			//
			// This is the link to get rid of.
			//

		   *prev = col_vects_links[next].Next;

			return(1);
		}

		prev = &col_vects_links[next].Next;
		next =  col_vects_links[next].Next;
	}

	return(0);
}



ULONG	similar_col_vect(UWORD index)
{
	ULONG	c0;
	for(c0=1;c0<next_col_vect;c0++)
	{
		if(col_vects[c0].X[0]==col_vects[index].X[0] &&
		   col_vects[c0].Y[0]==col_vects[index].Y[0] &&
		   col_vects[c0].X[1]==col_vects[index].X[1] &&
		   col_vects[c0].Y[1]==col_vects[index].Y[1])
			return(1);
/*
		if(col_vects[c0].X[1]==col_vects[index].X[0] &&
		   col_vects[c0].Y[1]==col_vects[index].Y[0] &&
		   col_vects[c0].X[0]==col_vects[index].X[1] &&
		   col_vects[c0].Y[0]==col_vects[index].Y[1])
			return(1);
*/

	}
	return(0);
}


SLONG	insert_collision_vect(SLONG x1,SLONG my_y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,UBYTE prim_type,UBYTE prim_extra,SWORD face)
{
	SLONG	step_x,step_y,temp_x,temp_z;
	SLONG	length;

	SLONG	step_z;

	step_x=x2-x1;
	step_y=y2-my_y1;
	step_z=z2-z1;
	length=Root(step_x*step_x+step_y*step_y+step_z*step_z);
#ifdef	PSX
//	asm("break 0");
#endif

	if(length<3)
		return(0);

	if(next_col_vect>=MAX_COL_VECT)
		return(0);

	col_vects[next_col_vect].X[0]=x1;
	col_vects[next_col_vect].X[1]=x2;

	col_vects[next_col_vect].Y[0]=my_y1;
	col_vects[next_col_vect].Y[1]=y2;

	col_vects[next_col_vect].Z[0]=(SWORD)z1;
	col_vects[next_col_vect].Z[1]=(SWORD)z2;

	col_vects[next_col_vect].Face=face;
	col_vects[next_col_vect].PrimType=prim_type;
	col_vects[next_col_vect].PrimExtra=prim_extra;

//	LogText(" insert collision %d \n",next_col_vect);

	//
	// this block will add collision to every mapwho line overlaps
	// doing it the easy to code slow to run way
	//

	//
	// IF YOU CHANGE THIS... CHANGE THE SIMILAR CODE IN remove_collision_vect()
	//

	temp_x=x1<<10;
	temp_z=z1<<10;
	step_x=(step_x<<10)/length;
	step_z=(step_z<<10)/length;
	for(;length>=0;length--)
	{
		add_collision_to_single_cell(next_col_vect,(temp_x>>10)>>ELE_SHIFT,(temp_z>>10)>>ELE_SHIFT);
		temp_x+=step_x;
		temp_z+=step_z;
	}
	next_col_vect++;
	return(next_col_vect-1);
}

void remove_collision_vect(UWORD vect)
{
	SLONG x1;
	SLONG my_y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

	SLONG step_x;
	SLONG step_y;
	SLONG step_z;

	SLONG temp_x;
	SLONG temp_z;

	SLONG map_x;
	SLONG map_z;

	SLONG length;

	ASSERT(WITHIN(vect, 1, next_col_vect - 1));

	x1 = col_vects[vect].X[0];
	my_y1 = col_vects[vect].Y[0];
	z1 = col_vects[vect].Z[0];

	x2 = col_vects[vect].X[1];
	y2 = col_vects[vect].Y[1];
	z2 = col_vects[vect].Z[1];

	//
	// This must be the same code as for inserting a vect!
	//

	step_x = x2 - x1;
	step_y = y2 - my_y1;
	step_z = z2 - z1;

	length = Root(step_x*step_x + step_y*step_y + step_z*step_z);

	if (length < 3)
	{
		//
		// Too small to worth bothering about.
		//

		return;
	}

	temp_x = x1 << 10;
	temp_z = z1 << 10;

	step_x = (step_x << 10) / length;
	step_z = (step_z << 10) / length;

	while(length-- >= 0)
	{
		map_x = (temp_x >> 10) >> ELE_SHIFT;
		map_z = (temp_z >> 10) >> ELE_SHIFT;

		remove_collision_from_single_cell(
			vect,
			map_x,
			map_z);

		temp_x += step_x;
		temp_z += step_z;
	}
}
#endif


#define	TSHIFT	8
#ifndef	PSX
#ifndef TARGET_DC
UBYTE	check_big_point_triangle_col(SLONG x,SLONG y,SLONG ux,SLONG uy,SLONG vx,SLONG vy,SLONG wx,SLONG wy)
{
	SLONG	s,t,top,bot,res;
	top=(y-uy)*(wx-ux)+(ux-x)*(wy-uy);
	bot=(vy-uy)*(wx-ux)-(vx-ux)*(wy-uy);


//	if(next_col_column<5)
//		printf(" top %d bot %d \n",top,bot);

	if(bot==0)
		return 0;

	s=(top<<TSHIFT)/bot;
	if(s<0)
		return 0;
	if((wx-ux)==0)
		t=((y<<TSHIFT)-(uy<<TSHIFT)-s*(vy-uy))/(wy-uy);
	else
		t=((x<<TSHIFT)-(ux<<TSHIFT)-s*(vx-ux))/(wx-ux);
	if(t<0)
		return 0;

	res=s+t;
	if( res<(1<<TSHIFT))
	{
//		if(next_col_column<5)
//			printf(" s %d t %d \n",s>>6,t>>6);


		return	1;  // point inside triangle
	}
	else
		return	0;  // point outside triangle
}
#endif
#endif

#ifndef PSX
#ifndef TARGET_DC
SLONG	point_in_quad_old(SLONG px,SLONG pz,SLONG x,SLONG y,SLONG z,SWORD face)
{
	SLONG x1,my_y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4;
	SLONG ret;

	x1=x+prim_points[prim_faces4[face].Points[0]].X;
	my_y1=y+prim_points[prim_faces4[face].Points[0]].Y;
	z1=z+prim_points[prim_faces4[face].Points[0]].Z;

	x2=x+prim_points[prim_faces4[face].Points[1]].X;
	y2=y+prim_points[prim_faces4[face].Points[1]].Y;
	z2=z+prim_points[prim_faces4[face].Points[1]].Z;

	x3=x+prim_points[prim_faces4[face].Points[2]].X;
	y3=y+prim_points[prim_faces4[face].Points[2]].Y;
	z3=z+prim_points[prim_faces4[face].Points[2]].Z;

	x4=x+prim_points[prim_faces4[face].Points[3]].X;
	y4=y+prim_points[prim_faces4[face].Points[3]].Y;
	z4=z+prim_points[prim_faces4[face].Points[3]].Z;






	ret=check_big_point_triangle_col(px-x1,pz-z1,x1-x1,z1-z1,x2-x1,z2-z1,x3-x1,z3-z1);
	if(ret)
		return(ret);
	else
		return(check_big_point_triangle_col(px-x2,pz-z2,x2-x2,z2-z2,x4-x2,z4-z2,x3-x2,z3-z2));

//	SLONG x,SLONG y,SLONG ux,SLONG uy,SLONG vx,SLONG vy,SLONG wx,SLONG wy)


}
#endif
#endif

//
// Given a segment of a line (x1,z1)-(x2,z2) and a point (a,b) returns
// *dist and *along.  *along is how far along the line the perpendicular line
// from the point would intersect the given line and *dist is how far the point is from the line
//
// dest and along are returned in fixed point 8.
//


SLONG dprod;
SLONG cprod;
#ifndef	PSX
#ifndef TARGET_DC
SLONG dist_to_line(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b)
{
	SLONG dx, dz;
	SLONG da, db;
	SLONG m;
//	SLONG n;

	SLONG dist;

//	SLONG along;



	// First we see if the perpendicular intersection of (a,b) to the line
	// (x1,z1) - (x2,z2) lies inside the line segment.

	// If it does, then dist is the perpendicular distance and along is how
	// far along the line the perpendicular intersection is.

	// If it isn't, then we take dist to be the distance of (a,b) to the
	// nearest point and along to be 0 or 1.

	dx = x2 - x1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;

	// if the dot product of these two vectors is less than zero then
	// (x,z) lies 'behind' point (x1,z1)

	LogText(" DTL1 dx %d dz %d da %d db %d \n",dx,dz,da,db);


	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{
//		*along = 0;

		da    = abs(da);
		db    = abs(db);
		dist = QDIST2(da, db);
		//printf("dist=%d\n", *dist);

		return(dist);
	}

	dx = x1 - x2;
	dz = z1 - z2;

	da = a - x2;
	db = b - z2;


	// if the dot product of these two vectors is less than zero then
	// (a,b) lies 'behind' point (x2,z2)

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{
//		*along = 255;

		da    = abs(da);
		db    = abs(db);
		dist = QDIST2(da, db);

		//printf("dist=%d\n", *dist);
		return(dist);
	}


	// The perpendicular intersection lies beween (x1,z1) and (x2,z2)...

	dx = x2 - x1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;


	cprod = da*dz - db*dx;
//	dprod = da*dx + db*dz;

	dx = abs(dx);
	dz = abs(dz);

	cprod = abs(cprod);
//	dprod = abs(dprod);

	m  = QDIST2(dx, dz);

	dist  = cprod / m;
//	along = dprod / ((dx*dx + dz*dz) >> 8);

	return(dist);
}
#endif
#endif


//
// Returns which side of the colvect you are on.
//

SLONG which_side(
		SLONG  x1, SLONG z1,
		SLONG  x2, SLONG z2,
		SLONG  a,  SLONG b)
{
	SLONG dx;
	SLONG dz;

	SLONG da;
	SLONG db;

	SLONG cprod;

	dx = x2 - x1;
	dz = z2 - z1;

	da = a  - x1;
	db = b  - z1;

	cprod = da*dz - db*dx;

	return cprod;
}


SLONG calc_along_vect(SLONG ax,SLONG az,struct DFacet *p_vect)
{
	SLONG dx;
	SLONG dz;

	SLONG x1;
	SLONG z1;

	SLONG x2;
	SLONG z2;


	SLONG along;

	x1=p_vect->x[0] << 8;
	z1=p_vect->z[0] << 8;

	x2=p_vect->x[1] << 8;
	z2=p_vect->z[1] << 8;

	dx = x2 - x1;
	dz = z2 - z1;

	ax -= x1;
	az -= z1;

	if (abs(dx) > abs(dz))
	{
		if (dx==0)
		{
			dz = 1;
		}

		along = (ax << 8) / dx;
	}
	else
	{
		if(dz==0)
		{
			dz=1;
		}

		along = (az << 8) / dz;
	}

	return along;
}




//
// Given a line segment and a point P, this function returns the signed
// distance to the nearest point on the line Q.  The distance is negative
// if the point is on the right-hand-side looking down the line segment.
// The function returns a vector connecting P with Q.  'onoroff' is set to
// TRUE if the nearest point on the line Q, lies within the line segment
// and FALSE if it lies on one of the end points.
//



//
// normal is always the normal of the line
//
void signed_dist_to_line_with_normal(
		SLONG  x1, SLONG z1,
		SLONG  x2, SLONG z2,
		SLONG  a,  SLONG b,
		SLONG *dist,
		SLONG *vec_x,
		SLONG *vec_z,
		SLONG *on)
{
	SLONG dx;
	SLONG dz;

	SLONG da;
	SLONG db;

	SLONG len;

//	SLONG dprod;
//	SLONG cprod;

	dx = x2 - x1;
	dz = z2 - z1;

	*vec_x = dz;
	*vec_z = -dx;

	da = a  - x1;
	db = b  - z1;

	cprod = da*dz - db*dx; //-ve*0
	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{
		//
		// The nearest point on the line segment to (a,b) is (x1,z1).
		//

//		*vec_x = da;
//		*vec_z = db;

		*dist  =  QDIST2(abs(da),abs(db));
		*on    =  FALSE;

//		MSG_add("Nearest to point 1: cprod %d", cprod);

		//
		// Make the distance signed.
		//

		if (cprod < 0) {*dist = -*dist;}

		return;
	}

	dx = x1 - x2;
	dz = z1 - z2;

	da = a  - x2;
	db = b  - z2;

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{
		//
		// The nearest point of the line segment to (a,b) is (x2,z2).
		//

//		*vec_x = da;
//		*vec_z = db;

		*dist  =  QDIST2(abs(da),abs(db));
		*on    =  FALSE;

//		MSG_add("Nearest to point 2: cprod %d", cprod);

		//
		// Make the distance signed.
		//

		if (cprod < 0) {*dist = -*dist;}

		return;
	}

	//
	// The perpendicular intersection lies on the line segment.
	//

	len = QDIST2(abs(dx),abs(dz));

    *dist  =  cprod / len;
	*vec_x = -dz;
	*vec_z =  dx;
	*on    =  TRUE;

	return;
}



//
// version that returns a different normal if you are interacting with the end of the vect
//

//
// different as in the vector from the end of the line to the test point
//
#ifndef	PSX
#ifndef TARGET_DC

void signed_dist_to_line_with_normal_mark(
		SLONG  x1, SLONG z1,
		SLONG  x2, SLONG z2,
		SLONG  a,  SLONG b,
		SLONG *dist,
		SLONG *vec_x,
		SLONG *vec_z,
		SLONG *on)
{
	SLONG dx;
	SLONG dz;

	SLONG da;
	SLONG db;

	SLONG len;

//	SLONG dprod;
//	SLONG cprod;

	dx = x2 - x1;
	dz = z2 - z1;

	da = a  - x1;
	db = b  - z1;

	cprod = da*dz - db*dx;
	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{
		//
		// The nearest point on the line segment to (a,b) is (x1,z1).
		//

		*vec_x = da;
		*vec_z = db;

		*dist  =  QDIST2(abs(da),abs(db));
		*on    =  FALSE;

//		MSG_add("Nearest to point 1: cprod %d", cprod);

		//
		// Make the distance signed.
		//

		if (cprod < 0) {*dist = -*dist;}

		return;
	}

	dx = x1 - x2;
	dz = z1 - z2;

	da = a  - x2;
	db = b  - z2;

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{
		//
		// The nearest point of the line segment to (a,b) is (x2,z2).
		//

		*vec_x = da;
		*vec_z = db;

		*dist  =  QDIST2(abs(da),abs(db));
		*on    =  FALSE;

//		MSG_add("Nearest to point 2: cprod %d", cprod);

		//
		// Make the distance signed.
		//

		if (cprod < 0) {*dist = -*dist;}

		return;
	}

	//
	// The perpendicular intersection lies on the line segment.
	//

	len = QDIST2(abs(dx),abs(dz));

    *dist  =  cprod / len;
	*vec_x = -dz;
	*vec_z =  dx;

/*
	if(cprod>0)
	{
		*vec_x = -dz;
		*vec_z =  dx;
	}
	else
	{
		*vec_x = dz;
		*vec_z = -dx;
	}
	*/
	*on    =  TRUE;

	return;
}
#endif
#endif




void nearest_point_on_line(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b,SLONG *ret_x,SLONG *ret_z)
{
	SLONG dx, dz;
	SLONG da, db;
//	SLONG dprod;
//	SLONG cprod;
//	SLONG m, n;

	SLONG along;



	// First we see if the perpendicular intersection of (a,b) to the line
	// (x1,z1) - (x2,z2) lies inside the line segment.

	// If it does, then dist is the perpendicular distance and along is how
	// far along the line the perpendicular intersection is.

	// If it isn't, then we take dist to be the distance of (a,b) to the
	// nearest point and along to be 0 or 1.

	dx = x2 - x1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;

	// if the dot product of these two vectors is less than zero then
	// (x,z) lies 'behind' point (x1,z1)

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{

		*ret_x=x1;
		*ret_z=z1;


		return;
	}

	dx = x1 - x2;
	dz = z1 - z2;

	da = a - x2;
	db = b - z2;

	// if the dot product of these two vectors is less than zero then
	// (a,b) lies 'behind' point (x2,z2)

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{

		*ret_x=x2;
		*ret_z=z2;

		return;
	}


	// The perpendicular intersection lies beween (x1,z1) and (x2,z2)...

	dx = x2 - x1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;

	dprod = da*dx + db*dz;

	dx = abs(dx);
	dz = abs(dz);

	dprod = abs(dprod);

	along = dprod / ((dx*dx + dz*dz) >> 8);

//	along is a course value for ratio along line 0..128
	dx = x2 - x1;
	dz = z2 - z1;
	*ret_x=x1+((dx*along)>>8);
	*ret_z=z1+((dz*along)>>8);

	return;
}

SLONG	global_on=0;

SLONG nearest_point_on_line_and_dist(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b,SLONG *ret_x,SLONG *ret_z)
{
	SLONG dx, dz;
	SLONG da, db;
//	SLONG dprod;
//	SLONG cprod;
	SLONG m;
//	SLONG n;

	SLONG along;
	SLONG	dist;



	// First we see if the perpendicular intersection of (a,b) to the line
	// (x1,z1) - (x2,z2) lies inside the line segment.

	// If it does, then dist is the perpendicular distance and along is how
	// far along the line the perpendicular intersection is.

	// If it isn't, then we take dist to be the distance of (a,b) to the
	// nearest point and along to be 0 or 1.

	dx = x2 - x1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;

	// if the dot product of these two vectors is less than zero then
	// (x,z) lies 'behind' point (x1,z1)

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{

		da    = abs(da);
		db    = abs(db);
		dist = QDIST2(da, db);
		*ret_x=x1;
		*ret_z=z1;
		global_on=0;


		return(dist);
	}

	dx = x1 - x2;
	dz = z1 - z2;

	da = a - x2;
	db = b - z2;

	// if the dot product of these two vectors is less than zero then
	// (a,b) lies 'behind' point (x2,z2)

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{

		da    = abs(da);
		db    = abs(db);
		dist = QDIST2(da, db);
		*ret_x=x2;
		*ret_z=z2;
		global_on=0;

		return(dist);
	}


	// The perpendicular intersection lies beween (x1,z1) and (x2,z2)...

	dx = x2 - x1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;

	cprod = da*dz - db*dx;
	dprod = da*dx + db*dz;

	dx = abs(dx);
	dz = abs(dz);

	cprod = abs(cprod);
	dprod = abs(dprod);

	along = dprod / (((dx*dx + dz*dz) >> 8) + 1);

	m  = QDIST2(dx, dz) + 1;

	dist  = cprod / m;

//	along is a course value for ratio along line 0..128
	dx = x2 - x1;
	dz = z2 - z1;
	*ret_x=x1+((dx*along)>>8);
	*ret_z=z1+((dz*along)>>8);
	global_on=1;

	return(dist);
}

SLONG nearest_point_on_line_and_dist_calc_y(	SLONG x1, SLONG my_y1,SLONG z1,	SLONG x2, SLONG y2,SLONG z2,	SLONG a,  SLONG b,SLONG *ret_x,SLONG *ret_y,SLONG *ret_z)
{
	SLONG dx, dz,dy;
	SLONG da, db;
//	SLONG dprod;
//	SLONG cprod;
	SLONG m;
//	SLONG n;

	SLONG along;
	SLONG	dist;



	// First we see if the perpendicular intersection of (a,b) to the line
	// (x1,z1) - (x2,z2) lies inside the line segment.

	// If it does, then dist is the perpendicular distance and along is how
	// far along the line the perpendicular intersection is.

	// If it isn't, then we take dist to be the distance of (a,b) to the
	// nearest point and along to be 0 or 1.

	dx = x2 - x1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;

	// if the dot product of these two vectors is less than zero then
	// (x,z) lies 'behind' point (x1,z1)

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{

		da    = abs(da);
		db    = abs(db);
		dist = QDIST2(da, db);
		*ret_x=x1;
		*ret_y=my_y1;
		*ret_z=z1;


		return(dist);
	}

	dx = x1 - x2;
	dz = z1 - z2;

	da = a - x2;
	db = b - z2;

	// if the dot product of these two vectors is less than zero then
	// (a,b) lies 'behind' point (x2,z2)

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{

		da    = abs(da);
		db    = abs(db);
		dist = QDIST2(da, db);
		*ret_x=x2;
		*ret_y=y2;
		*ret_z=z2;

		return(dist);
	}


	// The perpendicular intersection lies beween (x1,z1) and (x2,z2)...

	dx = x2 - x1;
	dy = y2 - my_y1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;

	cprod = da*dz - db*dx;
	dprod = da*dx + db*dz;

	dx = abs(dx);
	dz = abs(dz);

	cprod = abs(cprod);
	dprod = abs(dprod);

	along = dprod / (((dx*dx + dz*dz) >> 8) + 1);

	m  = QDIST2(dx, dz) + 1;

	dist  = cprod / m;

//	along is a course value for ratio along line 0..128
	dx = x2 - x1;
	dz = z2 - z1;
	*ret_x=x1+((dx*along)>>8);
	*ret_y=my_y1+((dy*along)>>8);
	*ret_z=z1+((dz*along)>>8);

	return(dist);
}

SLONG distance_to_line(
		SLONG  x1, SLONG z1,
		SLONG  x2, SLONG z2,
		SLONG  a,  SLONG b)
{
	SLONG nearest_x;
	SLONG nearest_z;

	SLONG dist = nearest_point_on_line_and_dist(
					x1, z1,
					x2, z2,
					a, b,
				   &nearest_x,
				   &nearest_z);

	return dist;
}


SLONG nearest_point_on_line_and_dist_and_along(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b,SLONG *ret_x,SLONG *ret_z,SLONG *ret_along)
{
	SLONG dx, dz;
	SLONG da, db;
//	SLONG dprod;
//	SLONG cprod;
	SLONG m;
//	SLONG n;

	SLONG along;
	SLONG	dist;



	// First we see if the perpendicular intersection of (a,b) to the line
	// (x1,z1) - (x2,z2) lies inside the line segment.

	// If it does, then dist is the perpendicular distance and along is how
	// far along the line the perpendicular intersection is.

	// If it isn't, then we take dist to be the distance of (a,b) to the
	// nearest point and along to be 0 or 1.

	dx = x2 - x1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;

	// if the dot product of these two vectors is less than zero then
	// (x,z) lies 'behind' point (x1,z1)

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{

		da    = abs(da);
		db    = abs(db);
		dist = QDIST2(da, db);
		*ret_x=x1;
		*ret_z=z1;

		*ret_along=0;

		return(dist);
	}

	dx = x1 - x2;
	dz = z1 - z2;

	da = a - x2;
	db = b - z2;

	// if the dot product of these two vectors is less than zero then
	// (a,b) lies 'behind' point (x2,z2)

	dprod = da*dx + db*dz;

	if (dprod <= 0)
	{

		da    = abs(da);
		db    = abs(db);
		dist = QDIST2(da, db);
		*ret_x=x2;
		*ret_z=z2;

		*ret_along=255;

		return(dist);
	}


	// The perpendicular intersection lies beween (x1,z1) and (x2,z2)...

	dx = x2 - x1;
	dz = z2 - z1;

	da = a - x1;
	db = b - z1;

	cprod = da*dz - db*dx;
	dprod = da*dx + db*dz;

	dx = abs(dx);
	dz = abs(dz);

	cprod = abs(cprod);
	dprod = abs(dprod);

	along = dprod / ((dx*dx + dz*dz) >> 8);

	m  = QDIST2(dx, dz);

	dist  = cprod / m;

//	along is a course value for ratio along line 0..128
	dx = x2 - x1;
	dz = z2 - z1;
	*ret_x=x1+((dx*along)>>8);
	*ret_z=z1+((dz*along)>>8);


	*ret_along=along;
	return(dist);


}

/*
SLONG	get_height_on_plane_quad_f(SLONG x,SLONG z,UWORD face)
{
	struct	PrimFace4 *this_face4;
	SLONG	obj_x,obj_z,obj_y;
	SLONG	ux,uy,uz,vx,vy,vz,wx,wy,wz;

	this_face4=&prim_faces4[face];

	obj_x=map_things[this_face4->ThingIndex].X;
	obj_z=map_things[this_face4->ThingIndex].Z;
	obj_y=map_things[this_face4->ThingIndex].Y;

	ux=	obj_x+prim_points[this_face4->Points[0]].X;
	uy=	obj_y+prim_points[this_face4->Points[0]].Y;
	uz=	obj_z+prim_points[this_face4->Points[0]].Z;

	vx=	obj_x+prim_points[this_face4->Points[1]].X;
	vy=	obj_y+prim_points[this_face4->Points[1]].Y;
	vz=	obj_z+prim_points[this_face4->Points[1]].Z;

	wx=	obj_x+prim_points[this_face4->Points[2]].X;
	wy=	obj_y+prim_points[this_face4->Points[2]].Y;
	wz=	obj_z+prim_points[this_face4->Points[2]].Z;

	return(get_height_on_plane_tri(x,z,ux,uy,uz,vx,vy,vz,wx,wy,wz));
}

SLONG	get_height_on_plane_tri_f(SLONG x,SLONG z,UWORD face)
{
	struct	PrimFace3 *this_face3;
	SLONG	obj_x,obj_z,obj_y;
	SLONG	ux,uy,uz,vx,vy,vz,wx,wy,wz;

	this_face3=&prim_faces3[face];

	obj_x=map_things[this_face3->ThingIndex].X;
	obj_z=map_things[this_face3->ThingIndex].Z;
	obj_y=map_things[this_face3->ThingIndex].Y;

	ux=	obj_x+prim_points[this_face3->Points[0]].X;
	uy=	obj_y+prim_points[this_face3->Points[0]].Y;
	uz=	obj_z+prim_points[this_face3->Points[0]].Z;

	vx=	obj_x+prim_points[this_face3->Points[1]].X;
	vy=	obj_y+prim_points[this_face3->Points[1]].Y;
	vz=	obj_z+prim_points[this_face3->Points[1]].Z;

	wx=	obj_x+prim_points[this_face3->Points[2]].X;
	wy=	obj_y+prim_points[this_face3->Points[2]].Y;
	wz=	obj_z+prim_points[this_face3->Points[2]].Z;

	return(get_height_on_plane_tri(x,z,ux,uy,uz,vx,vy,vz,wx,wy,wz));
}
*/

/*
SLONG	get_height_on_face_quad64(SLONG x, SLONG z, UWORD face)
{
	SLONG 	ux,uy,uz,vx,vy,vz,wx,wy,wz;
	struct	PrimFace4 *this_face4;
	SLONG	obj_x,obj_z,obj_y;
	SLONG	ax,ay,az,bx,by,bz;

	SLONG	top, bot;
	SLONG	alpha, beta;
	SLONG	y;

	this_face4=&prim_faces4[face];

	obj_x=map_things[this_face4->ThingIndex].X;
	obj_z=map_things[this_face4->ThingIndex].Z;
	obj_y=map_things[this_face4->ThingIndex].Y;

	ux=	obj_x+prim_points[this_face4->Points[0]].X;
	uy=	obj_y+prim_points[this_face4->Points[0]].Y;
	uz=	obj_z+prim_points[this_face4->Points[0]].Z;

	vx=	obj_x+prim_points[this_face4->Points[1]].X;
	vy=	obj_y+prim_points[this_face4->Points[1]].Y;
	vz=	obj_z+prim_points[this_face4->Points[1]].Z;

	wx=	obj_x+prim_points[this_face4->Points[2]].X;
	wy=	obj_y+prim_points[this_face4->Points[2]].Y;
	wz=	obj_z+prim_points[this_face4->Points[2]].Z;


	ax = (vx - ux) << 8;
	ay = (vy - uy) << 8;
	az = (vz - uz) << 8;

	bx = (wx - ux) << 8;
	by = (wy - uy) << 8;
	bz = (wz - uz) << 8;

	x  = (x<<8) - (ux << 8);
	z  = (z<<8) - (uz << 8);

	//printf("face =%d a=(%d,%d,%d) b =(%d,%d,%d) xz=(%d,%d)\n",face,ax,ay,az,bx,by,bz,x,z);

	// Work out alpha and beta such that x = alpha*ax + beta*bx and y = alhpa*ay + beta*by

	// First alpha...

	top   = MUL64(x,  bz) - MUL64(z,  bx);
	bot   = MUL64(bz, ax) - MUL64(bx, az);

	if (bot == 0) bot = 1;

	alpha = DIV64(top, bot);

	// Now beta...

	top   = MUL64(z,  ax) - MUL64(x,  az);
	beta  = DIV64(top, bot);

	y     = uy << 8;
	y    += MUL64(alpha, ay);
	y    += MUL64(beta,  by);

	if (alpha < 0 || alpha > 0x10000 || beta < 0 || beta > 0x10000)
	{
//		LogText(" get height on QUAD NOT %d alpha %x beta %x \n",face,alpha,beta);
		return 0;
	}
	else
	{
//		LogText(" get height on face=%d alpha %x beta %x  uy %d vy %d wy %d\n",y>>8,face,alpha,beta),uy,vy,wy;
		return y >> 8;
	}

}
*/

SLONG	calc_height_at(SLONG x,SLONG z)
{
	return 0;

	/*

	MapElement *me;
	SLONG	new_y,h0,h1,h2,h3;

//	if(x<0||x>=(EDIT_MAP_WIDTH<<ELE_SHIFT)||z<0||z>=(EDIT_MAP_DEPTH<<ELE_SHIFT))
//		return(0);
//	me=&edit_map[x>>ELE_SHIFT][z>>ELE_SHIFT];
	if(x<0||z<0||x>MAP_WIDTH<<8||z>MAP_WIDTH<<8)
		return(0);
	me=&MAP2((x>>ELE_SHIFT),(z>>ELE_SHIFT));

	h1=me->Alt<<FLOOR_HEIGHT_SHIFT;                //my_big_map[(new_x.L.HWord)+((new_z.L.HWord)*MAP_WIDTH)].Alt;
	if( (x>>8) >=(MAP_WIDTH))
		h0=h1;
	else
		h0=((me+MAP_WIDTH)->Alt)<<FLOOR_HEIGHT_SHIFT;            //my_big_map[(new_x.L.HWord+1)+((new_z.L.HWord)*MAP_WIDTH)].Alt;

	if( (z>>8) >=(MAP_HEIGHT))
	{
		h2=h1;
		h3=h0;
	}
	else
	{
		h2=((me+1)->Alt)<<FLOOR_HEIGHT_SHIFT;     //my_big_map[(new_x.L.HWord)+((new_z.L.HWord+1)*MAP_WIDTH)].Alt;
		h3=((me+MAP_WIDTH+1)->Alt)<<FLOOR_HEIGHT_SHIFT;   //my_big_map[(new_x.L.HWord+1)+((new_z.L.HWord+1)*MAP_WIDTH)].Alt;
	}
	if(h0==h1&&h1==h2&&h2==h3)
	{
		if (me->Flags & FLOOR_SINK_SQUARE)
		{
			h0-=32;
		}
		if (me->Flags & FLOOR_TRENCH)
			h0-=256;
		return(h0);
	}
//	SWAP(h0,h2);
//	SWAP(h1,h3);
	//LogText("CALC HEIGHT x %d.%d z %d.%d  h1 %d h0 %d h2 %d h3 %d \n",x>>ELE_SHIFT,x&(~ELE_AND),z>>ELE_SHIFT,z&(~ELE_AND),h1,h0,h2,h3);
	x=x&(~ELE_AND);
	z=z&(~ELE_AND);
//	if(x+z<ELE_SIZE)
//		new_y=(h1+(((h0-h1)*(x))>>ELE_SHIFT)+(((h2-h1)*(z))>>ELE_SHIFT));
//	else
//		new_y=(h3+(((h2-h3)*(ELE_SIZE-x))>>ELE_SHIFT)+(((h0-h3)*(ELE_SIZE-z))>>ELE_SHIFT));

	if(x+z<ELE_SIZE)
		new_y=(h1+(((h0-h1)*(x))>>ELE_SHIFT)+(((h2-h1)*(z))>>ELE_SHIFT));
	else
		new_y=(h3+(((h2-h3)*(ELE_SIZE-x))>>ELE_SHIFT)+(((h0-h3)*(ELE_SIZE-z))>>ELE_SHIFT));

	if (me->Flags & FLOOR_SINK_SQUARE)
	{
		new_y-=32;
	}
	if (me->Flags & FLOOR_TRENCH)
		new_y-=256;
//	MSG_add(" h0 %d h1 %d h2 %d h3 %d dx %d dz %d  ===%d\n",h0,h1,h2,h3,x,z,new_y);
	//LogText(" result %d \n",new_y);
	return(new_y);

	*/
}

//
// replace with table look up
//


SLONG	collision_storey(SLONG type)
{
	switch(type)
	{
		case	STOREY_TYPE_NORMAL:
		case	STOREY_TYPE_NORMAL_FOUNDATION:
		case	STOREY_TYPE_WALL:
		case	STOREY_TYPE_FENCE:
		case	STOREY_TYPE_FENCE_BRICK:
		case	STOREY_TYPE_FENCE_FLAT:
		case	STOREY_TYPE_NOT_REALLY_A_STOREY_TYPE_BUT_A_VALUE_TO_PUT_IN_THE_PRIM_TYPE_FIELD_OF_COLVECTS_GENERATED_BY_INSIDE_BUILDINGS:
		case	STOREY_TYPE_LADDER:
		case	STOREY_TYPE_TRENCH:
		case	STOREY_TYPE_SKYLIGHT:
		case	STOREY_TYPE_STAIRCASE:
		case	STOREY_TYPE_CABLE:
		case	STOREY_TYPE_OUTSIDE_DOOR:
			return(1);
		default:
			return(0);
	}
}
#ifdef	DOG_POO
SLONG	do_move_collide(SLONG x,SLONG y,SLONG z,SLONG dx,SLONG dy,SLONG dz,SLONG cell_dx,SLONG cell_dz,SLONG	scale_move)
{
	SLONG	vect;
	struct	CollisionVect	*p_vect;
	SLONG	mx,mz;

	y+=50; //middle of body?

	mx=((x)>>ELE_SHIFT)+cell_dx; //was x+dx
	mz=((z)>>ELE_SHIFT)+cell_dz; //was z+dz
//	e_draw_3d_mapwho(mx,mz);
	if(mx>0&&mx<EDIT_MAP_WIDTH&&mz>0&&mz<EDIT_MAP_DEPTH)
	{
		vect=MAP2(mx,mz).ColVectHead;
		dx*=scale_move;
		dy*=scale_move;
		dz*=scale_move;
		while(vect)
		{
			SLONG	actual_vect;
			actual_vect=col_vects_links[vect].VectIndex;
			p_vect=&col_vects[actual_vect];
			LogText(" does (%d,%d->%d,%d) .. (%d,%d->%d,%d) intersect \n",x,z,x+dx,z+dz,p_vect->X[0],p_vect->Z[0],p_vect->X[1],p_vect->Z[1]);
//			e_draw_3d_line(p_vect->X[0],0,p_vect->Z[0],p_vect->X[1],0,p_vect->Z[1]);
			if(collision_storey(p_vect->PrimType))
			{
				if( (y>=p_vect->Y[0]) && y<=(p_vect->Y[0]+BLOCK_SIZE*p_vect->PrimExtra))
				{
					if(two4_line_intersection(x,z,x+dx,z+dz,p_vect->X[0],p_vect->Z[0],p_vect->X[1],p_vect->Z[1]))
					{
						LogText(" yes \n");
						return(actual_vect);
					}
					else
						LogText(" no \n");
				}
				else
				{
//					MSG_add(" Y oor y%d miny %d maxy %d height %d",y,p_vect->Y[0],p_vect->Y[0]+BLOCK_SIZE*4*p_vect->PrimExtra,p_vect->PrimExtra);

				}
			}
//			else
//				MSG_add(" non normal storey \n");
			vect=col_vects_links[vect].Next;
		}
		return(0);
	}
	else
		return(-1);
}

SLONG	do_move_collide_circle(SLONG x,SLONG y,SLONG z,SLONG len,SLONG cell_dx,SLONG cell_dz)
{
	SLONG	vect;
	struct	CollisionVect	*p_vect;
	SLONG	mx,mz;

	mx=((x)>>ELE_SHIFT)+cell_dx;
	mz=((z)>>ELE_SHIFT)+cell_dz;
	//e_draw_3d_mapwho(mx,mz);
//	LogText(" collide circle at (%d,%d) cell %d,%d\n",mx,mz,cell_dx,cell_dz);
	if(mx>0&&mx<EDIT_MAP_WIDTH && mz>0&&mz<EDIT_MAP_DEPTH)
	{
		vect=MAP2(mx,mz).ColVectHead;
		while(vect)
		{
			SLONG	actual_vect;
			actual_vect=col_vects_links[vect].VectIndex;
			p_vect=&col_vects[actual_vect];
			//e_draw_3d_line(p_vect->X[0],0,p_vect->Z[0],p_vect->X[1],0,p_vect->Z[1]);

			if(dist_to_line(p_vect->X[0],p_vect->Z[0],p_vect->X[1],p_vect->Z[1],x,z)<len)
			{
				return(actual_vect);
			}
			vect=col_vects_links[vect].Next;
		}
		return(0);
	}
	else
		return(-1);
}

//
// given a col vect index and a world co-ord, returns a new co-ord
// new_dist away from col vect,
//

SLONG	get_point_dist_from_col_vect(SLONG vect,SLONG x,SLONG z,SLONG *ret_x,SLONG *ret_z,SLONG new_dist)
{
	struct	CollisionVect	*p_vect;
	SLONG	near_x,near_z;
	SLONG	dx,dz;
	SLONG	dist;
	SLONG	angle=0;

	p_vect=&col_vects[vect];

	//
	// find distance to line
	//
	dist=dist_to_line(p_vect->X[0],p_vect->Z[0],p_vect->X[1],p_vect->Z[1],x,z);

	//
	// find co_ord on line that we are near
	//
	nearest_point_on_line(p_vect->X[0],p_vect->Z[0],p_vect->X[1],p_vect->Z[1],x,z,&near_x,&near_z);

	//
	// near_xz and xz form a vector of length dist which we must position ourselves
	// REQUIRED_DIST_JUMP_GRAB along
	//

	dx=x-near_x;
	dz=z-near_z;
	LogText(" vect to line %d %d \n",dx,dz);



	if(dx||dz)
	{
		angle=Arctan(-dx,dz)+1024;
		LogText(" angle %d \n",angle);
		if(angle<0)
			angle=2048+angle;
		angle=angle&2047;
	}

	LogText(" was dist %d  new_dist %d near_x %d near_z %d\n",dist,new_dist,near_x,near_z);

	if (dist == 0) {dist = 1;}

	dx=(dx*new_dist)/dist;
	dz=(dz*new_dist)/dist;

	LogText(" result dx dz %d %d \n",dx,dz);

	*ret_x=near_x+dx;
	*ret_z=near_z+dz;

	return(angle);


}

//
// check to see if movement vector intersects a col vect
//

SLONG	check_vect_vect(MAPCO16 m_dx,MAPCO16 m_dy,MAPCO16 m_dz,Thing *p_thing,SLONG scale)
{
	SLONG	cell_dx,cell_dz;
	ULONG	col;
//	SLONG	len;
	SLONG	wx,wy,wz;


	calc_sub_objects_position(p_thing,p_thing->Draw.Tweened->AnimTween,SUB_OBJECT_LEFT_FOOT,&wx,&wy,&wz);

	wx+=p_thing->WorldPos.X>>8;
	wy+=p_thing->WorldPos.Y>>8;
	wz+=p_thing->WorldPos.Z>>8;

/*
	len=QDIST2(abs(m_dx),abs(m_dz));
	LogText(" movement dist %d \n",len);

	if(len<PERSON_RADIUS)
	{
		if(len==0)
			len=1;
		m_dx=(m_dx*PERSON_RADIUS)/len;
		m_dy=(m_dy*PERSON_RADIUS)/len;
		m_dz=(m_dz*PERSON_RADIUS)/len;
		scale=1;
	}
*/


//	e_draw_3d_line(p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z,p_thing->WorldPos.X+m_dx*scale,p_thing->WorldPos.Y+m_dy*scale,p_thing->WorldPos.Z+m_dz*scale);
//	LogText("UNI1 check vect vect mdx %d %d %d \n",m_dx,m_dy,m_dz);

	cell_dx=-((wx)>>ELE_SHIFT)+((wx+m_dx*scale)>>ELE_SHIFT);
	cell_dz=-((wz)>>ELE_SHIFT)+((wz+m_dz*scale)>>ELE_SHIFT);

	//LogText(" cell dx %d %d scale %d\n",cell_dx,cell_dz,scale);

	col=do_move_collide(wx,wy,wz,m_dx,m_dy,m_dz,0,0,scale);

	if(cell_dx&&!col)
	{
		col=do_move_collide(wx,wy,wz,m_dx,m_dy,m_dz,cell_dx,0,scale);
	}
	if(cell_dz&&!col)
	{
		col=do_move_collide(wx,wy,wz,m_dx,m_dy,m_dz,0,cell_dz,scale);
	}
	if(cell_dx && cell_dz && !col)
	{
		col=do_move_collide(wx,wy,wz,m_dx,m_dy,m_dz,cell_dx,cell_dz,scale);
	}
	return(col);
}

SLONG	check_vect_circle(SLONG m_dx,SLONG m_dy,SLONG m_dz,Thing *p_thing,SLONG radius)
{
	SLONG	x,y,z;
	SLONG	dx,dz;
	SLONG	cell_radius;
	ULONG	col;



	x=m_dx+(p_thing->WorldPos.X>>8);
	y=m_dy+(p_thing->WorldPos.Y>>8);
	z=m_dz+(p_thing->WorldPos.Z>>8);

	cell_radius=(radius>>ELE_SHIFT)+1;
	for(dx=-cell_radius;dx<=cell_radius;dx++)
	for(dz=-cell_radius;dz<=cell_radius;dz++)
	{
		col=do_move_collide_circle(x,y,z,radius,dx,dz);
		if(col)
			return(col);
	}
	return(0);
}
#endif

extern	SLONG	is_thing_on_this_quad(SLONG x,SLONG z,SLONG face);
extern	void	e_draw_3d_line(SLONG x1,SLONG my_y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);


void	highlight_face(SLONG face)
{
#ifndef	PSX
#ifndef TARGET_DC
	return;
	if(face>0)
	{
		SLONG	face_x,face_y,face_z;
//		SLONG	wall,storey,building;
		SLONG	p0,p1,p2,p3;
/*
		wall=prim_faces4[face].ThingIndex;
		if(wall<0)
		{
			storey=wall_list[-wall].StoreyHead;
			building=storey_list[storey].BuildingHead;
			if(prim_faces4[face].Type==FACE_TYPE_CABLE)
			{
				face_x=storey_list[storey].DX;
				face_y=0; //building_list[building].Y;
				face_z=storey_list[storey].DZ;
			}
			else
			{
				SLONG  thing;
				Thing *p_thing;

				thing   = building_list[building].ThingIndex;
				p_thing = TO_THING(thing);

				face_x=p_thing->WorldPos.X >> 8;
				face_y=p_thing->WorldPos.Y >> 8;
				face_z=p_thing->WorldPos.Z >> 8;
			}
		}
		else
		{
				Thing *p_thing;
				p_thing = TO_THING(wall);

				face_x=p_thing->WorldPos.X >> 8;
				face_y=p_thing->WorldPos.Y >> 8;
				face_z=p_thing->WorldPos.Z >> 8;
		}
		*/

		face_x=0;
		face_y=0;
		face_z=0;

		p0=prim_faces4[face].Points[0];
		p1=prim_faces4[face].Points[1];
		p2=prim_faces4[face].Points[2];
		p3=prim_faces4[face].Points[3];



		e_draw_3d_line(prim_points[p1].X+face_x,prim_points[p1].Y+face_y,prim_points[p1].Z+face_z,
						prim_points[p3].X+face_x,prim_points[p3].Y+face_y,prim_points[p3].Z+face_z);


		e_draw_3d_line(
						prim_points[p3].X+face_x,prim_points[p3].Y+face_y,prim_points[p3].Z+face_z,
						prim_points[p2].X+face_x,prim_points[p2].Y+face_y,prim_points[p2].Z+face_z);

		e_draw_3d_line(
						prim_points[p2].X+face_x,prim_points[p2].Y+face_y,prim_points[p2].Z+face_z,
						prim_points[p0].X+face_x,prim_points[p0].Y+face_y,prim_points[p0].Z+face_z);

	}
#endif
#endif
}
void	highlight_rface(SLONG rface)
{
#ifndef TARGET_DC

	return;
	if(rface>0)
	{
		SLONG	x,y,z;
//		SLONG	wall,storey,building;

		x=(roof_faces4[rface].RX&127)<<8;
		y=roof_faces4[rface].Y;
		z=(roof_faces4[rface].RZ&127)<<8;



		e_draw_3d_line(x,y,z,x+256,y,z);
		e_draw_3d_line(x+256,y,z,x+256,y,z+256);
		e_draw_3d_line(x+256,y,z+256,x,y,z+256);
		e_draw_3d_line(x,y,z+256,x,y,z);
	}
#endif
}


void	highlight_quad(SLONG face,SLONG face_x,SLONG face_y,SLONG face_z)
{
#ifndef TARGET_DC
	return;

	if(face>0)
	{
		SLONG	p0,p1,p2,p3;


		p0=prim_faces4[face].Points[0];
		p1=prim_faces4[face].Points[1];
		p2=prim_faces4[face].Points[2];
		p3=prim_faces4[face].Points[3];

		e_draw_3d_line(prim_points[p0].X+face_x,prim_points[p0].Y+face_y,prim_points[p0].Z+face_z,
						prim_points[p1].X+face_x,prim_points[p1].Y+face_y,prim_points[p1].Z+face_z);


		e_draw_3d_line(prim_points[p1].X+face_x,prim_points[p1].Y+face_y,prim_points[p1].Z+face_z,
						prim_points[p3].X+face_x,prim_points[p3].Y+face_y,prim_points[p3].Z+face_z);


		e_draw_3d_line(
						prim_points[p3].X+face_x,prim_points[p3].Y+face_y,prim_points[p3].Z+face_z,
						prim_points[p2].X+face_x,prim_points[p2].Y+face_y,prim_points[p2].Z+face_z);

		e_draw_3d_line(
						prim_points[p2].X+face_x,prim_points[p2].Y+face_y,prim_points[p2].Z+face_z,
						prim_points[p0].X+face_x,prim_points[p0].Y+face_y,prim_points[p0].Z+face_z);

	}
#endif
}

SLONG	vect_intersect_wall(SLONG x1,SLONG my_y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
	// go through col vects this vect might intersect

	return(0);
}


/*
#define	PERSON_RADIUS	(50)
SLONG	check_vect(SLONG m_dx,SLONG m_dy,SLONG m_dz,Thing *p_thing,SLONG scale)
{
	SLONG	cell_dx,cell_dz;
	ULONG	col;
	SLONG	len;

	len=QDIST2(abs(m_dx),abs(m_dz));
	LogText(" movement dist %d \n",len);

	if(len<PERSON_RADIUS)
	{
		if(len==0)
			len=1;
		m_dx=(m_dx*PERSON_RADIUS)/len;
		m_dy=(m_dy*PERSON_RADIUS)/len;
		m_dz=(m_dz*PERSON_RADIUS)/len;
		scale=1;
	}
//	e_draw_3d_line(p_thing->X,p_thing->Alt,p_thing->Z,p_thing->X+m_dx*scale,p_thing->Alt+m_dy*scale,p_thing->Z+m_dz*scale);

	cell_dx=-((p_thing->X)>>ELE_SHIFT)+((p_thing->X+m_dx*scale)>>ELE_SHIFT);
	cell_dz=-((p_thing->Z)>>ELE_SHIFT)+((p_thing->Z+m_dz*scale)>>ELE_SHIFT);

	col=do_move_collide(p_thing->X,p_thing->Alt,p_thing->Z,m_dx,m_dy,m_dz,0,0,scale);

	if(cell_dx&&!col)
	{
		col=do_move_collide(p_thing->X,p_thing->Alt,p_thing->Z,m_dx,m_dy,m_dz,cell_dx,0,scale);
	}
	if(cell_dz&&!col)
	{
		col=do_move_collide(p_thing->X,p_thing->Alt,p_thing->Z,m_dx,m_dy,m_dz,0,cell_dz,scale);
	}
	if(cell_dx && cell_dz && !col)
	{
		col=do_move_collide(p_thing->X,p_thing->Alt,p_thing->Z,m_dx,m_dy,m_dz,cell_dx,cell_dz,scale);
	}
	return(col);
}

SLONG	check_vect_circle(SLONG m_dx,SLONG m_dy,SLONG m_dz,Thing *p_thing,SLONG radius)
{
	SLONG	x,y,z;
	SLONG	dx,dz;
	SLONG	cell_radius;
	ULONG	col;



	x=m_dx+p_thing->WorldPos.X;
	y=m_dy+p_thing->WorldPos.Y;
	z=m_dz+p_thing->WorldPos.Z;

	cell_radius=(radius>>ELE_SHIFT)+2;
	for(dx=-cell_radius;dx<cell_radius;dx++)
	for(dz=-cell_radius;dz<cell_radius;dz++)
	{
		col=do_move_collide_circle(x,y,z,radius,dx,dz);
		if(col)
			return(col);
	}
	return(0);
}
*/

extern	void	set_tween_for_dy(Thing *p_person,SLONG dy);

SLONG	find_face_near_y(MAPCO16 x,MAPCO16 y,MAPCO16 z, SLONG ignore_faces_of_this_building,Thing *p_person,SLONG neg_dy,SLONG pos_dy,SLONG *ret_y)
{
	SLONG	mx,mz;
	SLONG	index;
	SLONG	check_face;
	SLONG	new_y,dy;

	SLONG mx1 = (x - 0x200) >> PAP_SHIFT_LO;
	SLONG mz1 = (z - 0x200) >> PAP_SHIFT_LO;

	SLONG mx2 = (x + 0x200) >> PAP_SHIFT_LO;
	SLONG mz2 = (z + 0x200) >> PAP_SHIFT_LO;

	SATURATE(mx1, 0, PAP_SIZE_LO - 1);
	SATURATE(mz1, 0, PAP_SIZE_LO - 1);

	SATURATE(mx2, 0, PAP_SIZE_LO - 1);
	SATURATE(mz2, 0, PAP_SIZE_LO - 1);

	if(PAP_hi[x>>8][z>>8].Flags&PAP_FLAG_ROOF_EXISTS)
	{

		check_face=ROOF_HIDDEN_GET_FACE(x>>8,z>>8);
		new_y=MAVHEIGHT(x>>8,z>>8)<<6;
		dy=y-new_y;


		if(dy>neg_dy-100 && dy<pos_dy)
		{
			*ret_y=new_y;

			return(check_face);
		}
		else
		if(dy>-128)
		{
			if(p_person->SubState==SUB_STATE_RUNNING_JUMP_FLY)
			{
				set_tween_for_dy(p_person,y-new_y);

			}
		}
	}


	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		index = PAP_2LO(mx,mz).Walkable;

		while(index)
		{
			SLONG	offset_y;
			check_face = index; //walk_links[index].Face;

//			if (check_face > 0)
			{
				if (check_face>0 && prim_faces4[check_face].ThingIndex == ignore_faces_of_this_building)
				{
					//
					// Dont collide with this face.
					//
				}
				else
				{
					if (is_thing_on_this_quad(x,z,check_face))
					{
						//calc height for location, if big difference in height then say no

						if(check_face<0)
						{
							calc_height_on_rface(x,z,-check_face,&new_y);
							offset_y=100;
						}
						else
						{
							calc_height_on_face(x,z,check_face,&new_y);
							offset_y=0;
						}


						//
						// we are on this face so use the result which may have been clipped to the edge of the face
						//

					//	if(new_y==1000000)
					//		ASSERT(0);
						dy=y-new_y;

#ifndef	PSX
#ifndef TARGET_DC

						if(ControlFlag&&allow_debug_keys)
						{
							CBYTE	str[100];
							sprintf(str," land on walkable dy %d >%d  <%d \n",dy,neg_dy,pos_dy);
							CONSOLE_text(str,10000);
						}
#endif
#endif
						if(dy>neg_dy-offset_y && dy<pos_dy)
						{
	//						MSG_add(" hit face \n");
							*ret_y=new_y;

							return(check_face);
						}
						else
//						if(dy<pos_dy)
//						{
//							ASSERT(0)
//						}
//						else
						if(dy>-128)
						{
							if(p_person->SubState==SUB_STATE_RUNNING_JUMP_FLY)
							{
								set_tween_for_dy(p_person,y-new_y);

							}
							MSG_add(" LAND on face y  %d peep y %d miss dy=%d ",new_y,y,y-new_y);
						}
					}
				}

				if(index<0)
					index = roof_faces4[-index].Next;
				else
					index = prim_faces4[index].WALKABLE;

			}

		}
	}
	*ret_y=0;
	return(0);
}
#ifndef	PSX
#ifndef TARGET_DC
SLONG find_alt_for_this_pos(SLONG  x,SLONG  z)
{
	SLONG mx;
	SLONG mz;
//	SLONG dy;
	SLONG facey;
	SLONG index;
	SLONG groundy;
	SLONG	count;

	SLONG mx1 = x - 0x200 >> PAP_SHIFT_LO;
	SLONG mz1 = z - 0x200 >> PAP_SHIFT_LO;

	SLONG mx2 = x + 0x200 >> PAP_SHIFT_LO;
	SLONG mz2 = z + 0x200 >> PAP_SHIFT_LO;

	SATURATE(mx1, 0, PAP_SIZE_LO - 1);
	SATURATE(mz1, 0, PAP_SIZE_LO - 1);

	SATURATE(mx2, 0, PAP_SIZE_LO - 1);
	SATURATE(mz2, 0, PAP_SIZE_LO - 1);

	if(PAP_hi[x>>8][z>>8].Flags&PAP_FLAG_ROOF_EXISTS)
	{
		return(MAVHEIGHT(x>>8,z>>8)<<6);
	}

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		index = PAP_2LO(mx,mz).Walkable;
		count=0;

		 while(index&&count++<1000)
		 {
			ASSERT(index >= 0);
//			ASSERT(WITHIN(index, 1, next_prim_face4 - 1));

			if (is_thing_on_this_quad(x,z, index))
			{
				//
				// We've found a face to stand on. But at what height?
				//

				//
				// must be on this face so don't check the result
				//

				if(index<0)
				{
					calc_height_on_rface(x,z, -index,&facey);
				}
				else
				{
					calc_height_on_face(x,z, index,&facey);
				}

				//
				// Too much difference in y?
				//

					return facey;
			}

			if(index<0)
			{
				index = roof_faces4[-index].Next;

			}
			else
			{
				index = prim_faces4[index].WALKABLE;
			}
			ASSERT(count<800);
		 }
	}



	//
	// How about the ground?
	//

	groundy = PAP_calc_height_at(x,z) + 5;

	return groundy; // step onto floor

}
#endif
#endif
void	correct_pos_for_ladder(struct DFacet *p_facet,SLONG *px,SLONG *pz,SLONG *angle,SLONG scale)
{
	SLONG	x1,z1,x2,z2,dx,dz;

	x1=p_facet->x[0] << 8;
	z1=p_facet->z[0] << 8;

	x2=p_facet->x[1] << 8;
	z2=p_facet->z[1] << 8;

   *px=(x1+x2)>>1;
   *pz=(z1+z2)>>1;

	dx=x2-x1;
	dz=z2-z1;

	//if(dx||dz)
	{
		*angle=Arctan(-dx,dz)+1024+512;
		if(*angle<0)
			*angle=2048+*angle;
		*angle=*angle&2047;
	}

//	MSG_add(" correct pos dx %d dz %d \n",(dz*scale)>>2,-(dx*scale)>>2);
	*px+=(dz*scale)>>10;
	*pz-=(dx*scale)>>10;
}


SLONG	ok_to_mount_ladder(struct Thing *p_thing,struct DFacet *p_facet)
{
	SLONG	dx,dz,px,pz,angle;

	correct_pos_for_ladder(p_facet,&px,&pz,&angle,256);

	dx=abs(px-(p_thing->WorldPos.X>>8));
	dz=abs(pz-(p_thing->WorldPos.Z>>8));

	if(QDIST2(dx,dz)<75)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

SLONG	mount_ladder(Thing *p_thing,SLONG facet)
{
//	SWORD	storey,wall;


	if(ok_to_mount_ladder(p_thing,&dfacets[facet]))
	{

		//
		// Mount ladder
		//

		set_person_climb_ladder(p_thing,facet);

		return TRUE;
	}

	return FALSE;
}

extern	void	locked_anim_change_end_type(Thing *p_person,UWORD locked_object,UWORD anim,SLONG type);

SLONG	set_person_climb_down_onto_ladder(Thing *p_person,SLONG colvect)
{
	SLONG	x,z,dx,dz,angle;
	SLONG	dist=64;
	GameCoord	new_position;

	if(p_person->Genus.Person->PersonType==PERSON_ROPER)
		dist=-64;

	correct_pos_for_ladder(&dfacets[colvect],&x,&z,&angle,dist);

	dx=abs(x-(p_person->WorldPos.X>>8));
	dz=abs(z-(p_person->WorldPos.Z>>8));

	if(QDIST2(dx,dz)>75)
	{
		return(0);
	}

	new_position.X  =x<<8;
	new_position.Y  =p_person->WorldPos.Y;
	new_position.Z  =z<<8;

	p_person->Draw.Tweened->Angle=angle;

	move_thing_on_map(p_person,&new_position);

	set_generic_person_state_function(p_person,STATE_CLIMB_LADDER);


	//	set_anim(p_person,ANIM_OFF_LADDER_TOP);

	if(p_person->Genus.Person->PersonType==PERSON_ROPER)
 		locked_anim_change_end_type(p_person,SUB_OBJECT_LEFT_FOOT,COP_ROPER_ANIM_LADDER_END_L,ANIM_TYPE_ROPER);
	else
	if(p_person->Genus.Person->PersonType==PERSON_COP||p_person->Genus.Person->PersonType==PERSON_THUG_GREY||p_person->Genus.Person->PersonType==PERSON_THUG_RASTA||p_person->Genus.Person->PersonType==PERSON_THUG_RED)
 		locked_anim_change_end_type(p_person,SUB_OBJECT_LEFT_FOOT,COP_ROPER_ANIM_LADDER_END_L,ANIM_TYPE_ROPER);
	else

	locked_anim_change_end_type(p_person,SUB_OBJECT_LEFT_FOOT,ANIM_OFF_LADDER_TOP,p_person->Genus.Person->AnimType);


	p_person->Velocity=0;
	p_person->Genus.Person->Action=ACTION_CLIMBING;
	p_person->SubState=SUB_STATE_CLIMB_DOWN_ONTO_LADDER;
	p_person->Genus.Person->OnFacet=colvect;
	p_person->OnFace=0;

	p_person->Genus.Person->Flags|=FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C;
	return(1);
}



//
// Looks for a ladder col-vect.
//

SLONG find_nearby_ladder_colvect_radius(
		SLONG mid_x,
		SLONG mid_z,
		SLONG radius)
{
	SLONG mx;
	SLONG mz;

	SLONG dx;
	SLONG dz;

	SLONG dist;

	SLONG minx;
	SLONG minz;

	SLONG maxx;
	SLONG maxz;

//	SLONG storey;
//	SLONG wall;

	SLONG v_list;
	SLONG i_vect;

	SLONG best_dist = INFINITY;
	SLONG best_vect = NULL;

	struct	DFacet *p_vect;

	minx = mid_x - radius >> PAP_SHIFT_LO;
	minz = mid_z - radius >> PAP_SHIFT_LO;

	maxx = mid_x + radius >> PAP_SHIFT_LO;
	maxz = mid_z + radius >> PAP_SHIFT_LO;

	for (mx = minx; mx <= maxx; mx++)
	for (mz = minz; mz <= maxz; mz++)
	{
		if (WITHIN(mx, 0, PAP_SIZE_LO  - 1) &&
			WITHIN(mz, 0, PAP_SIZE_LO - 1))
		{
			SLONG	exit=0;
			v_list = PAP_2LO(mx,mz).ColVectHead;

			if(v_list)
			while(!exit)
			{
				i_vect =  facet_links[v_list];
				if(i_vect<0)
				{
					i_vect=-i_vect;
					exit=1;
				}
				p_vect = &dfacets[i_vect];

				if (p_vect->FacetType == STOREY_TYPE_LADDER)
				{
					dx = (p_vect->x[0] + p_vect->x[1] << 7) - mid_x;
					dz = (p_vect->z[0] + p_vect->z[1] << 7) - mid_z;

					dist = abs(dx) + abs(dz);

					if (dist < best_dist)
					{
						best_dist = dist;
						best_vect = i_vect;
					}
				}

				v_list++;
			}
		}
	}

	return best_vect;
}


//
// Looks for a nearby ladder col-vect.
//

SLONG find_nearby_ladder_colvect(Thing *p_thing)
{
	SLONG mid_x;
	SLONG mid_z;

	SLONG ladder;

	#define LADDER_NEARBY_RADIUS 100

	mid_x = p_thing->WorldPos.X >> 8;
	mid_z = p_thing->WorldPos.Z >> 8;

	ladder = find_nearby_ladder_colvect_radius(mid_x, mid_z, LADDER_NEARBY_RADIUS);

	return ladder;
}





void	set_feet_to_y(Thing *p_person,SLONG new_y)
{
	SLONG	x,y,z;
	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_LEFT_FOOT,&x,&y,&z);
	MSG_add(" set feet to y %d, foot y %d \n",new_y,y);

	new_y-=y;

	p_person->WorldPos.Y=(new_y)<<8; //+4 ?
}

extern void person_splash(Thing *p_person, SLONG limb); // limb == -1 => splash on center not at limb.


SLONG	height_above_anything(Thing *p_person,SLONG body_part,SWORD *onface)
{
	SLONG	on_face;
	SLONG	fx,fy,fz,new_y;

//	SLONG ignore_building;

	*onface = 0;

	if(p_person->Genus.Person->InsideIndex)
		return(0);  //floor height of inside
	//

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,body_part,&fx,&fy,&fz);

	//
	// This shift is here to mimic the code in predict_collision_with_face().
	// Otherwise the two functions will disagree on whether you have hit a face.
	//


	fx+=p_person->WorldPos.X>>8;
	fy+=p_person->WorldPos.Y>>8;
	fz+=p_person->WorldPos.Z>>8;

	//
	// got body part position in world space
	//
	if(1||p_person->Genus.Person->Ware)
		on_face = find_face_for_this_pos(fx,fy,fz,&new_y,0,FIND_FACE_NEAR_BELOW);
	else
		on_face = find_face_for_this_pos(fx,fy,fz,&new_y,0,FIND_ANYFACE);
/*
	if(on_face==GRAB_FLOOR)
	{
		*onface=0;
		return(fy-new_y);
	}
	else
*/
	if(on_face)
	{
		*onface=on_face;
		return(fy-new_y);
	}
	else
	{
		new_y = PAP_calc_height_at_thing(p_person,fx,fz);
		*onface=0;
		return(fy-new_y);
	}
}


//
// For a person in 3d space, find either a face to stand on, or the floor to stand on, and setup height/onface ...
//
SLONG	plant_feet(Thing *p_person)
{
	SLONG	on_face;
	SLONG	fx,fy,fz,new_y;

//	SLONG ignore_building;

	//
	// Make a splash...

	if(p_person->Genus.Person->InsideIndex)
		return(0);
	//

	person_splash(p_person, -1);	// -1 => In the middle of the person.

	calc_sub_objects_position(p_person,p_person->Draw.Tweened->AnimTween,SUB_OBJECT_LEFT_FOOT,&fx,&fy,&fz);

	//
	// This shift is here to mimic the code in predict_collision_with_face().
	// Otherwise the two functions will disagree on whether you have hit a face.
	//

	fx = 0;  // Keep these shifts the same as in predict_collision_with_face();
	fz = 0;

	fx+=p_person->WorldPos.X>>8;
	fy+=p_person->WorldPos.Y>>8;
	fz+=p_person->WorldPos.Z>>8;
/*
	if (p_person->Flags & FLAGS_IN_BUILDING)
	{
		ignore_building = INDOORS_DBUILDING;
	}
	else
	{
		ignore_building = NULL;
	}
*/

	//
	// got foot position in world space
	//

	on_face = find_face_for_this_pos(fx,fy,fz,&new_y,0,0);

	if(on_face==GRAB_FLOOR)
	{
//		ASSERT(0);

		MSG_add(" PLANT FEET  on grab floor \n");
		set_feet_to_y(p_person,new_y);
		p_person->OnFace=0;
		return(-1);
	}
	else
	if(on_face)
	{
		//p_person->WorldPos.Y=new_y;
		MSG_add(" PLANT FEETa  on face at %d old y %d\n",new_y,p_person->WorldPos.Y>>8);
		if(new_y<fy-60)
		{
			//
			// It's	a bit of a drop so set person fall
			//

			MSG_add(" big drop so fall2 \n");
			p_person->OnFace=0;
			set_person_drop_down(p_person,PERSON_DROP_DOWN_KEEP_VEL);
//			p_person->velocity=0;

			return(0);
		}

		set_feet_to_y(p_person,new_y);
		p_person->OnFace=on_face;
		return(1);
	}
	else
	{
		new_y = PAP_calc_height_at_thing(p_person,fx,fz);
/*
		if (p_person->Flags & FLAGS_IN_SEWERS)
		{
			new_y = NS_calc_height_at(fx,fz);
		}
		else
		{
			new_y = PAP_calc_height_at(fx,fz);
		}
*/

		MSG_add(" PLANT FEET  at y %d old y %d %d\n",new_y,p_person->WorldPos.Y>>8,fy);

		if(new_y<fy-30)
		{
			//
			// It's	a bit of a drop so set person fall
			//

			MSG_add(" big drop so fall \n");
			p_person->OnFace=0;
			set_person_drop_down(p_person,PERSON_DROP_DOWN_KEEP_VEL);
//			p_person->velocity=0;

			return(0);
		}


		if(fy>new_y+20)
		{
			// have height so must fall
//			set_person_fall(p_person);
		}
		else
		{
			MSG_add(" small drop so teleport to alt \n");
			p_person->OnFace=0;
//			p_person->WorldPos.Y=new_y;
			set_feet_to_y(p_person,new_y);
			return(-1);
		}
	}


	return(0);
}


SLONG	get_person_radius(SLONG type)
{
	switch(type)
	{
		case	PERSON_COP:
			return(40);
		case	PERSON_DARCI:
			return(30);
	}

	return(50); // an average person
}

SLONG	get_person_radius2(SLONG type)
{
	switch(type)
	{
		case	PERSON_COP:
			return(40);
		case	PERSON_DARCI:
			return(30);
	}

	return(35); // an average person
}


/*

//
// Obsolete nowadays...
//

SLONG	bump_someone(Thing *p_thing,MAPCO24 mdx,MAPCO24 mdy,MAPCO24 mdz)
{
	SLONG	x,y,z;
	SLONG	dx,dz;
	SLONG	cell_radius;
	ULONG	col;
	SLONG	my_radius;
	THING_INDEX	index;



	x=((p_thing->WorldPos.X+mdx)>>8); //+mdx;
	y=((p_thing->WorldPos.Y+mdy)>>8); //+mdy;
	z=((p_thing->WorldPos.Z+mdz)>>8); //+mdz;

	my_radius = get_person_radius(p_thing->Genus.Person->PersonType);

	cell_radius=1;
	for(dx=-cell_radius;dx<=cell_radius;dx++)
	for(dz=-cell_radius;dz<=cell_radius;dz++)
	{

		index = MAP2((x>>ELE_SHIFT)+dx,(z>>ELE_SHIFT)+dz).MapWho;
		while(index)
		{
			SLONG	dist,ddx,ddy,ddz;
			Thing	*p_bumped;
			SLONG	bump_radius;

			p_bumped=TO_THING(index);
			switch(p_bumped->Class)
			{
				case	CLASS_PERSON:
					bump_radius = get_person_radius(p_bumped->Genus.Person->PersonType);

					ddx=abs(x-(p_bumped->WorldPos.X>>8));
					ddy=abs(y-(p_bumped->WorldPos.Y>>8));
					ddz=abs(z-(p_bumped->WorldPos.Z>>8));

					dist=QDIST2(ddx,ddz);

					if(dist<(bump_radius+my_radius))
					{
						SLONG	dist2;
						//
						// are radia intersect so check if we are moving nearer or further from collision
						//

						ddx=abs((x-(mdx>>8))-(p_bumped->WorldPos.X>>8));
						ddz=abs((z-(mdz>>8))-(p_bumped->WorldPos.Z>>8));

						dist2=QDIST2(ddx,ddz);

						if(dist<dist2)
						{
							//
							// distance with move vect is smaller than without movement therefore collision is valid
							//
							p_thing->Genus.Person->InWay=index;
							return(1);
						}
					}
					break;
			}
			index=p_bumped->Child;
		}
	}

	p_thing->Genus.Person->InWay=0;
	return(0);
}

*/

SLONG	get_fence_height(SLONG h)
{
	if(h==2)
		return(85);
	else
		return(h*64);

}

//
//
//
#ifndef	PSX
#ifndef TARGET_DC

// side_required     0 is the other side or 1 is this side
void	step_back_along_vect(SLONG x1,SLONG z1,SLONG *x2,SLONG *z2,SLONG vx1,SLONG vz1,SLONG vx2,SLONG vz2,SLONG side_required)
{
//	SLONG	mx,mz,prev_mx,prev_mz;
	SLONG	dx,dz;
	SLONG	side;

	dx=*x2-x1;
	dz=*z2-z1;

	dx>>=2;
	dz>>=2;

	dx += SIGN(*x2 - x1);
	dz += SIGN(*z2 - z1);

	while(1)
	{
//		add_debug_line(*x2,0,*z2,*x2-dx,0,*z2-dz,0xff0000);
		*x2-=dx;
		*z2-=dz;
		side=which_side(vx1,vz1,vx2,vz2,*x2,*z2);

		if((side<=0&&side_required==1)|| (side>=0&&side_required==0 ) || (side==0))
		{
			//
			// still wrong
			//
		}
		else
		{
			SLONG	norm_x,norm_z,dist,on;
			//
			// now on the right side
			//
			signed_dist_to_line_with_normal_mark(vx1,vz1,vx2,vz2,*x2,*z2,&dist,&norm_x,&norm_z,&on);

			if(side_required==1)
			{
				ASSERT(dist>=0);
			}
			else
			{
				ASSERT(dist<=0);
			}

			return;
		}
	}
}
#endif
#endif

#define	EXTRA_RADIUS	10
//
// The length of our normalised vector.
//

#define VEC_SHIFT  (17)
#define VEC_LENGTH (1 << VEC_SHIFT)


//
// Remember colvect we have already done, and dont do them twice.
//

#ifndef PSX
#define MAX_ALREADY 50
#else
#define MAX_ALREADY 8
#endif

UWORD already[MAX_ALREADY];



//
// Given a movement vector, this function changes it to slide
// along any col-vects it may encounter. If it find a colvect
// onto a walkable face, then it doesn't slide, it just returns
// that col-vect.
//

UWORD	max_facet_find=0;

SLONG last_slide_colvect;
SLONG last_slide_dist;
SLONG actual_sliding;
SLONG slide_door;
SLONG slide_ladder;
SLONG slide_into_warehouse;		// NULL if you have not gone into a warehouse or the index of the warehouse if you have.
SLONG slide_outof_warehouse;

/*

/*

#ifdef PSX
	SLONG old_start_x;
	SLONG old_start_y;
	SLONG old_start_z;

	SLONG old_end_x;
	SLONG old_end_y;
	SLONG old_end_z;
#endif

SLONG slide_along_old(
		SLONG  x1, SLONG  my_y1, SLONG  z1,
		SLONG *x2, SLONG *y2, SLONG *z2,
		SLONG  extra_wall_height,
		SLONG  radius)
{
	SLONG i;

	SLONG dx;
	SLONG dz;

	SWORD minx;
	SWORD minz;
	SWORD maxx;
	SWORD maxz;

//	SLONG cprod;
//	SLONG dprod;

	SLONG y_top;
	SLONG y_bot;

	SLONG mx;
	SLONG mz;

//	SLONG vx;
//	SLONG vz;

//	SLONG sx;
//	SLONG sz;

	SLONG dist;
	SLONG ndist;
	SLONG norm_x;
	SLONG norm_z;
	SLONG on;

	SLONG v_list;
	SLONG i_vect;

	SLONG overndist;
	UWORD num_slides;
	UWORD last_slide;

//	CollisionVect *p_vect;
	DFacet *p_vect;
//	SLONG ans = 0;
	SLONG	same_again=0;
	UBYTE already_upto=0;

	UBYTE slid_along_fence = FALSE;
	UWORD fence_colvect    = 0;

#ifndef PSX
 	SLONG old_start_x = x1;
	SLONG old_start_y = my_y1;
	SLONG old_start_z = z1;

	SLONG old_end_x = *x2;
	SLONG old_end_y = *y2;
	SLONG old_end_z = *z2;
#else
 	old_start_x = x1;
	old_start_y = my_y1;
	old_start_z = z1;

	old_end_x = *x2;
	old_end_y = *y2;
	old_end_z = *z2;
#endif

//	CollisionVect *p_vect;

	//
	// How many col-vects we have slid along and
	// the last col-vect we slide along.
	//

	num_slides         = 0;
	last_slide         = 0;
	last_slide_colvect = NULL;
	slide_door         = 0;
	slide_ladder       = 0;
	actual_sliding     = FALSE;

	if (COLLIDE_can_i_fastnav( x1 >> 16,  z1 >> 16) &&
		COLLIDE_can_i_fastnav(*x2 >> 16, *z2 >> 16))
	{
		//
		// If these are fastnav squares, we can do nowt.  'fastnav'
		// means there aren't any colvects next to them.
		//

		return FALSE;
	}

	//
	// The box in which to search for colvects.
	//

	x1 >>= 8;
	z1 >>= 8;
	my_y1 >>= 8;

	*x2 = (*x2) >> 8;
	*y2 = (*y2) >> 8;
	*z2 = (*z2) >> 8;

	add_debug_line(x1,my_y1,z1,*x2,*y2,*z2,0xffffff);

	radius <<= 1;

	minx = ((*x2)) - radius >> PAP_SHIFT_LO;
	minz = ((*z2)) - radius >> PAP_SHIFT_LO;

	maxx = ((*x2)) + radius >> PAP_SHIFT_LO;
	maxz = ((*z2)) + radius >> PAP_SHIFT_LO;

	radius >>= 1;

	for (mx = minx; mx <= maxx; mx++)
	for (mz = minz; mz <= maxz; mz++)
	{
		UWORD	fence=0;
		if (WITHIN(mx, 0, PAP_SIZE_LO  - 1) &&
			WITHIN(mz, 0, PAP_SIZE_LO - 1))
		{
			SLONG	exit=0;
			//
			// Check all the colvects on this square.
			//

			v_list = PAP_2LO(mx,mz).ColVectHead;

			if(v_list)
			while(!exit)
			{
//				ASSERT(WITHIN(v_list,                            0, next_dfacet - 1));
//				ASSERT(WITHIN(col_vects_links[v_list].VectIndex, 0, (next_col_vect      - 1)));


				i_vect =  facet_links[v_list];
				if(i_vect<0)
				{
					i_vect=-i_vect;
					exit=1;
				}
				p_vect = &dfacets[i_vect];

				//
				// Done this one already?
				//

				for (i = already_upto - 1; i >= 0; i--)
				{
					if (already[i] == i_vect)
					{
						//
						// Go to the next colvect in the linked list.
						//

						goto next_colvect;
					}
				}

				//
				// Remember that we have already done this colvect.
				//

				if (already_upto < MAX_ALREADY)
				{
//					ASSERT(WITHIN(already_upto, 0, MAX_ALREADY - 1));

					already[already_upto++] = i_vect;
				}

				if(already_upto>max_facet_find)
				{
					max_facet_find=already_upto;
					DebugText("max_facet_find %d \n",max_facet_find);
				}


				//
				// Is the 'y' component in range?
				//

				{
					SLONG vect_y;

					{
						if (p_vect->FacetType == STOREY_TYPE_FENCE_FLAT ||
							p_vect->FacetType == STOREY_TYPE_FENCE      ||
							p_vect->FacetType == STOREY_TYPE_FENCE_BRICK)
						{
							y_top = get_fence_top(x1, z1, i_vect);
							y_bot = get_fence_bottom(x1, z1, i_vect)-30;

							fence = TRUE;
						}
						else
						{
							SLONG	height;

							height=(p_vect->Height*p_vect->BlockHeight)<<2;
							fence=0;
							vect_y = p_vect->Y[0]; //get_height_along_vect(x1,z1,p_vect);

							y_bot = vect_y - 64;
							y_top = vect_y + height;

							if(p_vect->FacetType != STOREY_TYPE_CABLE)
							if (p_vect->FHeight ||
								p_vect->FacetType == STOREY_TYPE_NORMAL_FOUNDATION ||
								p_vect->FacetType == STOREY_TYPE_FENCE_FLAT)
							{
								y_bot = -INFINITY;
							}
						}
					}
				}

				//
				// A kludge to stop people going through walls and let people go over walls.
				//

				y_top += extra_wall_height;

				if (WITHIN(my_y1, y_bot, y_top)) // Mike! I know this was you -> ||WITHIN(my_y1+60,y_bot,y_top)||WITHIN(my_y1-60,y_bot,y_top))
				{
					SLONG	side;
					UWORD	start_index=0,end_index=1;

//					MSG_add("WITHIN slide along my_y1 %d between  %d->%d \n",my_y1,y_bot,y_top);
					//
					// How far is (*x2,*z2) from the col-vect?
					//
//again_for_fence:;
					signed_dist_to_line_with_normal_mark(
							p_vect->x[start_index] << 8, p_vect->z[start_index] << 8,
							p_vect->x[end_index  ] << 8, p_vect->z[end_index  ] << 8,
							*x2, *z2,
							&dist,
							&norm_x,
							&norm_z,
							&on);

//					MSG_add(" slide along  dist %d nx %d nz %d on %d\n",dist,norm_x,norm_z,on);



					//
					// This piece of code would probably be quite readable if it wasnt for all the comments getting in the way
					//

					//
					// decideds weather to perform a slide or allow the movement, based on if you have gotten too close to the wall
					// or have passed from one side of the wall to the other
					// for normal buildings it allows you to pass from inside the building to the outside
					{
						SLONG	slide=0;

						if(!on)
						{
							//
							// dealing with a simple end point, which does not need any concept of sidedness
							//	(except for now it does)
							if((abs(dist) < radius+EXTRA_RADIUS))
							{
								same_again=1;

								side = which_side(
										p_vect->x[start_index] << 8, p_vect->z[start_index] << 8,
										p_vect->x[end_index  ] << 8, p_vect->z[end_index  ] << 8,
										x1, z1);

								if(dist<0)
								{
									if(side<0)
									{
										// you ended on the wrong side of the wall
										// and you started out on the wrong side of the wall

//										MSG_add("!ON1 dist %d side %d nx %d nz %d\n",dist,side,norm_x,norm_z);

										dist=-dist;
										slide=1;
									}
									else
									{

										// you ended on the wrong side of the wall
										// and you started out on the right side of the wall
//										MSG_add("!ON2 dist %d side %d nx %d nz %d\n",dist,side,norm_x,norm_z);

										step_back_along_vect(
											x1, z1,
											x2, z2,
											p_vect->x[start_index] << 8, p_vect->z[start_index] << 8,
											p_vect->x[end_index  ] << 8, p_vect->z[end_index  ] << 8,
											1);	// 1 eh...?

										signed_dist_to_line_with_normal_mark(
											p_vect->x[start_index] << 8, p_vect->z[start_index] << 8,
											p_vect->x[end_index  ] << 8, p_vect->z[end_index  ] << 8,
										   *x2,
										   *z2,
										   &dist,
										   &norm_x,
										   &norm_z,
										   &on);

										//
										// you should now be on the right side of the wall, therefore your dist will be >0
										//

										ASSERT(dist>=0);

										if(on)
										{
											if(dist<radius)
											{
												slide=1;
											}
											else
											{
												slide=0;
											}
										}
										else
										{
											if(dist<radius+EXTRA_RADIUS)
											{
												slide=1;
											}
											else
											{
												slide=0;
											}
										}


									}
								}
								else
								{
									if(side<0)
									{
										// you ended on the right side
										// and you started on the wrong side

//										MSG_add("!ON3 step back  dist %d side %d nx %d nz %d\n",dist,side,norm_x,norm_z);

										step_back_along_vect(
											x1,z1,
											x2,z2,
											p_vect->x[start_index] << 8, p_vect->z[start_index] << 8,
											p_vect->x[end_index  ] << 8, p_vect->z[end_index  ] << 8,
											0);	//

										signed_dist_to_line_with_normal_mark(
											p_vect->x[start_index] << 8, p_vect->z[start_index] << 8,
											p_vect->x[end_index  ] << 8, p_vect->z[end_index  ] << 8,
										  *x2,
										  *z2,
										  &dist,
										  &norm_x,
										  &norm_z,
										  &on);

										if (on)
										{
											//
											// you are now back on the wall
											//

											dist=-dist;
											norm_x=-norm_x;
											norm_z=-norm_z;
											if(dist<radius)
											{
												slide=1;
											}
											else
											{
												slide=0;
											}
										}
										else
										{

											//
											// you should now be on the wrong of the wall, therefore your dist will be <0
											//
											dist=-dist;
											if(dist<radius+EXTRA_RADIUS)
											{
												slide=1;
											}
											else
											{
												slide=0;
											}
										}
									}
									else
									{
										slide=1;
//										MSG_add("!ON4 dist %d side %d nx %d nz %d\n",dist,side,norm_x,norm_z);
									}
								}
							}
						}
						else


						{
							//
							// what side did we start on
							//
							side = which_side(
										p_vect->x[start_index] << 8, p_vect->z[start_index] << 8,
										p_vect->x[end_index  ] << 8, p_vect->z[end_index  ] << 8,
										x1, z1);


//							MSG_add("ON dist %d side %d nx %d nz %d\n",dist,side,norm_x,norm_z);
							if(dist<0)
							{
								//
								// you ended on the wrong side of the wall
								//

								if(side<0)
								{
									//
									// and you started out on the wrong side of the wall
									//

									if(fence && (abs(dist)<radius))
									{
										//
										// Its a fence so we must slide along it if we are near it, whatever side we are on
										//

										slide=1;

										//
										// reverse the col vect direction for proper slideing
										//

										dist=-dist;
										norm_x=-norm_x;
										norm_z=-norm_z;


									}
									else
									{
										//
										// if we are somehow inside a facet vect, and its not a fence then we should allow the player to move
										// unhindered
										//
									}

								}
								else
								{
//									SLONG	dx,dz;
									//
									// you started out on the correct side, so you have tried to go from one side to the other
									// so you must slide

									if (p_vect->FacetType == STOREY_TYPE_LADDER)
										slide_ladder=i_vect;

									if (p_vect->FacetType == STOREY_TYPE_DOOR)
									{
										//
										// its a door so let player through and setup inside data structures
										//

										//
										// assume door is open for now
										//


*/
/*
										slide_door=p_vect->DStorey;

										*x2 = old_end_x;
										*y2 = old_end_y;
										*z2 = old_end_z;


										slide=0;	// This was _AFTER_ the return!

										return(0);
									}
									else
									{

										slide=1;
									}
								}
							}
							else
							{
								//
								// you ended on the correct side
								//

								if(side<0)
								{
									//
									// but you started on the wrong side
									//


									if (p_vect->FacetType == STOREY_TYPE_DOOR)
									{
										//
										// its a door so let player through and setup inside data structures
										//
										slide_door=-p_vect->DStorey;

										*x2 = old_end_x;
										*y2 = old_end_y;
										*z2 = old_end_z;

										slide=0;	// This was _AFTER_ the return!

										return(0);
									}
									else
									if(fence)
									{

										slide=1;

										norm_x=-norm_x;
										norm_z=-norm_z;
										dist=-dist;//QDIST2(dx,dz);
									}
									else
									{
										// you are inside the building
										// so allow the movement out of the building
									}
								}
								else
								{
									//
									// and you started on the correct side
									//
									if (p_vect->FacetType == STOREY_TYPE_LADDER)
										slide_ladder=i_vect;

									if(dist<radius)
									{
										//
										// And you ended quite close to the wall
										// so do a normal slide
										//

										if (p_vect->FacetType == STOREY_TYPE_DOOR)
											slide=0;
										else
											slide=1;
									}
									else
									{
										//
										//not near enough to bother slideing
										//
									}

								}


							}
						}


						if(slide)
						{

							last_slide_dist=dist;
//do_slide:;
							//
							// Find the normalised slide-along vector.
							//

							if(norm_x==0)
							{
								if(norm_z<0)
									norm_z=-VEC_LENGTH;
								else
									norm_z=VEC_LENGTH;

							}
							else
							if(norm_z==0)
							{
								if(norm_x<0)
									norm_x=-VEC_LENGTH;
								else
									norm_x=VEC_LENGTH;

							}
							else
							{

								ndist = norm_x*norm_x + norm_z*norm_z;
								ndist = Root(ndist);

								if (ndist < 4 && on==0)
								{
									//
									// Maybe we shouldn't slide along the face after all!
									//

										MSG_add("ERROR: TINY Colvect! nx %d nz %d",norm_x,norm_z);
								}
								else
								{
//									SLONG	dx,dz;

									//
									// Make sure that dist is sensible...
									//


									if(ndist==0)
									{
//										MSG_add(" no slide because normal is zero\n");
										add_debug_line(*x2,*y2,*z2,old_start_x>>8,old_start_y>>8,old_start_z>>8,0x0000ff);
										*x2 = old_start_x;
										*y2 = old_start_y;
										*z2 = old_start_z;
										ASSERT(0);

										return same_again;

									}
									if(on)
										ASSERT( WITHIN(ndist, 4, VEC_LENGTH / 4));

									//
									// One one divide instead of two!
									//

									overndist = VEC_LENGTH / ndist;

									norm_x *= overndist;
									norm_z *= overndist;
								}
							}



							//
							// Make sure we end up 'radius' distance from the col-vect.
							//

							if(on)
							{
//								MSG_add("ON radius %d dist %d normx %d normz %d \n",radius,dist,norm_x,norm_z);
								dx = (radius - dist) * norm_x >> VEC_SHIFT;
								dz = (radius - dist) * norm_z >> VEC_SHIFT;
							}
							else
							{
								//
								// end points have a larger radius
								//
//								MSG_add(" radius %d dist %d normx %d normz %d \n",radius,dist,norm_x,norm_z);
								dx = (radius+EXTRA_RADIUS - dist) * norm_x >> VEC_SHIFT;
								dz = (radius+EXTRA_RADIUS - dist) * norm_z >> VEC_SHIFT;

							}

							if (dx || dz)
							{
								actual_sliding = TRUE;

								if(on)
								{
									if(num_slides==0)
										add_debug_line(*x2,*y2,*z2,*x2+dx,*y2,*z2+dz,0x2f2f00);
									else
										add_debug_line(*x2,*y2,*z2,*x2+dx,*y2,*z2+dz,0x002f00);
								}
								else
								{
									if(num_slides==0)
										add_debug_line(*x2,*y2,*z2,*x2+dx,*y2,*z2+dz,0xffff00);
									else
										add_debug_line(*x2,*y2,*z2,*x2+dx,*y2,*z2+dz,0x00ff00);
								}

								*x2 += dx;
								*z2 += dz;

								if (fence)
								{
									//
									// We have slide along a fence. Remember this colvect for
									// future reference. If we slid along a fence we want to return
									// the fence as the last_slide_colvect.
									//

									slid_along_fence = TRUE;
									fence_colvect    = i_vect;
								}
							}

							num_slides        += 1;
							last_slide         = i_vect;
							last_slide_colvect = i_vect;
						}

//					  dont_slide_along:;
					}
				}
//				MSG_add("NOT WITHIN slide along my_y1 %d between  %d->%d \n",my_y1,y_bot,y_top);

			  next_colvect:;

				v_list++;
			}
		}
	}

	if (slid_along_fence)
	{
		//
		// If we slid along a fence make sure that is the one we return
		// as our last_slide_colvect;
		//

		last_slide_colvect = fence_colvect;
	}

	if (actual_sliding) //num_slides)
	{
		*x2 <<= 8;
		*y2 <<= 8;
		*z2 <<= 8;
	}
	else
	{
		//
		// Use the original destination if we haven't slid along
		// any walls. This preserves the bottom 8 bits or else we
		// lose accuracy at high frame rates.
		//
//
//		if(num_slides>1)
//		{
//			*x2 = old_start_x;
//			*y2 = old_start_y;
//			*z2 = old_start_z;
//
//		}
//		else

		{
			*x2 = old_end_x;
			*y2 = old_end_y;
			*z2 = old_end_z;
		}
	}

	return same_again;
}
*/

UBYTE   slid_along_fence=0;
UWORD   fence_colvect=0;
//SLONG	slide_nogo;

SLONG slide_along(
		SLONG  x1, SLONG  my_y1, SLONG  z1,
		SLONG *x2, SLONG *y2, SLONG *z2,
		SLONG  extra_wall_height,
		SLONG  radius,
		ULONG  flags)
{
//	SLONG i;

	SWORD minx;
	SWORD minz;
	SWORD maxx;
	SWORD maxz;

	SWORD y_top;
	SWORD y_bot;

	SWORD mx;
	SWORD mz;

	SLONG fx1;
	SLONG fz1;
	SLONG fx2;
	SLONG fz2;

	SLONG dx;
	SLONG dz;
//	SLONG push;
	SLONG dist;

	SWORD f_list;
	SWORD i_facet;

//	SLONG overndist;
	UWORD last_slide;

	DFacet *df;
	UBYTE   exit;
	UBYTE	fence=0;
	UBYTE	reverse;

	slid_along_fence=0;
	fence_colvect=0;

//	if(cheat)
//		return(0);


	extern UBYTE just_started_falling_off_backwards;

	if (just_started_falling_off_backwards)
	{
		radius += radius >> 1;
	}


//	add_debug_line(x1>>8,my_y1>>8,z1>>8,*x2>>8,*y2>>8,*z2>>8,0xffffff);

	//
	// How many col-vects we have slid along and
	// the last col-vect we slide along.
	//

	last_slide            = 0;
	last_slide_colvect    = NULL;
	slide_door            = 0;
	slide_ladder          = 0;
	actual_sliding        = FALSE;
	slide_into_warehouse  = 0;
	slide_outof_warehouse = FALSE;
//	slide_nogo=1;

	/*

	//
	// Make sure that the radius is big enough.
	//

	dx = abs(*x2 - x1) >> 8;
	dz = abs(*z2 - z1) >> 8;

	if (dx > radius)
	{
		radius = dx + 4;
	}

	if (dz > radius)
	{
		radius = dz + 4;
	}

	*/

	//
	// Slide along the no-go squares.
	//

	#if OLD_METHOD_THAT_LETS_YOU_GET_RIGHT_TO_THE_EDGE_OF_NOGO_SQUARES

	{
	 	if (PAP_2HI(*x2 >> 16, *z2 >> 16).Flags & PAP_FLAG_NOGO)
		{
			/*
			if (PAP_2HI(x1 >> 16, z1 >> 16).Flags & PAP_FLAG_NOGO)
			{
				SLONG tx;
				SLONG tz;
				SLONG score;
				SLONG best_score = INFINITY;
				SLONG best_x = *x2;
				SLONG best_z = *z2;
				SLONG here  = PAP_calc_map_height_at(x1 >> 8, z1 >> 8);
				SLONG there;

				//
				// Emergency! This person is on a no-go square! Find somewhere
				// nearby that isn't a no-go square.
				//

				for (dx = -0x2000; dx <= +0x2000; dx += 0x2000)
				for (dz = -0x2000; dz <= +0x2000; dz += 0x2000)
				{
					if (dx == 0 && dz == 0)
					{
						continue;
					}

					tx = x1 + dx;
					tz = z1 + dz;

					if (!(PAP_2HI(tx >> 16, tz >> 16).Flags & PAP_FLAG_NOGO))
					{
						there = PAP_calc_map_height_at(tx >> 8, tz >> 8);

						if (abs(here - there) <= 0x50)
						{
							score = abs(dx) + abs(dz) + abs(here - there);

							if (score < best_score)
							{
								best_score = score;
								best_x     = tx;
								best_z     = tz;
							}
						}
					}
				}

			   *x2 = best_x;
			   *z2 = best_z;
			}
			else
			*/
			{
				//
				// Not allowed to go here. Where should we go?
				//

				if ((x1 >> 16) != (*x2 >> 16) && !(PAP_2HI(x1 >> 16, *z2 >> 16).Flags & PAP_FLAG_NOGO))
				{
					*x2 = x1;
				}
				else
				if ((z1 >> 16) != (*z2 >> 16) && !(PAP_2HI(*x2 >> 16, z1 >> 16).Flags & PAP_FLAG_NOGO))
				{
					*z2 = z1;
				}
				else
				{
					*x2 = x1;
					*z2 = z1;
				}
			}
		}
	}

	#else

	{
		UBYTE collide = 0;

		#define NOGO_COLLIDE_XS (1 << 0)
		#define NOGO_COLLIDE_XL (1 << 1)
		#define NOGO_COLLIDE_ZS (1 << 2)
		#define NOGO_COLLIDE_ZL (1 << 3)

		#define NOGO_COLLIDE_SS (1 << 4)
		#define NOGO_COLLIDE_LS (1 << 5)
		#define NOGO_COLLIDE_SL (1 << 6)
		#define NOGO_COLLIDE_LL (1 << 7)

		#define NOGO_COLLIDE_WIDTH (0x5800)

		mx = *x2 >> 16;
		mz = *z2 >> 16;

		if (WITHIN(mx, 1, PAP_SIZE_HI - 2) &&
			WITHIN(mz, 1, PAP_SIZE_HI - 2))
		{
			if (PAP_2HI(mx - 1, mz).Flags & PAP_FLAG_NOGO) {collide |= NOGO_COLLIDE_XS;}
			if (PAP_2HI(mx + 1, mz).Flags & PAP_FLAG_NOGO) {collide |= NOGO_COLLIDE_XL;}
			if (PAP_2HI(mx, mz - 1).Flags & PAP_FLAG_NOGO) {collide |= NOGO_COLLIDE_ZS;}
			if (PAP_2HI(mx, mz + 1).Flags & PAP_FLAG_NOGO) {collide |= NOGO_COLLIDE_ZL;}

			if (PAP_2HI(mx - 1, mz - 1).Flags & PAP_FLAG_NOGO) {collide |= NOGO_COLLIDE_SS;}
			if (PAP_2HI(mx + 1, mz - 1).Flags & PAP_FLAG_NOGO) {collide |= NOGO_COLLIDE_LS;}
			if (PAP_2HI(mx - 1, mz + 1).Flags & PAP_FLAG_NOGO) {collide |= NOGO_COLLIDE_SL;}
			if (PAP_2HI(mx + 1, mz + 1).Flags & PAP_FLAG_NOGO) {collide |= NOGO_COLLIDE_LL;}

			if (flags & SLIDE_ALONG_FLAG_CARRYING)
			{
				//
				// Don't go up/down even quarter high blocks!
				//

				if (MAVHEIGHT(mx,mz) != MAVHEIGHT(mx - 1, mz)) {collide |= NOGO_COLLIDE_XS;}
				if (MAVHEIGHT(mx,mz) != MAVHEIGHT(mx + 1, mz)) {collide |= NOGO_COLLIDE_XL;}
				if (MAVHEIGHT(mx,mz) != MAVHEIGHT(mx, mz - 1)) {collide |= NOGO_COLLIDE_ZS;}
				if (MAVHEIGHT(mx,mz) != MAVHEIGHT(mx, mz + 1)) {collide |= NOGO_COLLIDE_ZL;}

				if (MAVHEIGHT(mx,mz) != MAVHEIGHT(mx - 1, mz - 1)) {collide |= NOGO_COLLIDE_SS;}
				if (MAVHEIGHT(mx,mz) != MAVHEIGHT(mx + 1, mz - 1)) {collide |= NOGO_COLLIDE_LS;}
				if (MAVHEIGHT(mx,mz) != MAVHEIGHT(mx - 1, mz + 1)) {collide |= NOGO_COLLIDE_SL;}
				if (MAVHEIGHT(mx,mz) != MAVHEIGHT(mx + 1, mz + 1)) {collide |= NOGO_COLLIDE_LL;}
			}

			if (collide & NOGO_COLLIDE_XS)
			{
				if ((*x2 & 0xffff) < NOGO_COLLIDE_WIDTH)
				{
					*x2 &= ~0xffff;
					*x2 |=  NOGO_COLLIDE_WIDTH;
//					slide_nogo=1;
				}
			}

			if (collide & NOGO_COLLIDE_XL)
			{
				if ((*x2 & 0xffff) > 0x10000 - NOGO_COLLIDE_WIDTH)
				{
					*x2 &= ~0xffff;
					*x2 |=  0x10000 - NOGO_COLLIDE_WIDTH;
//					slide_nogo=1;
				}
			}

			if (collide & NOGO_COLLIDE_ZS)
			{
				if ((*z2 & 0xffff) < NOGO_COLLIDE_WIDTH)
				{
					*z2 &= ~0xffff;
					*z2 |=  NOGO_COLLIDE_WIDTH;
//					slide_nogo=1;
				}
			}

			if (collide & NOGO_COLLIDE_ZL)
			{
				if ((*z2 & 0xffff) > 0x10000 - NOGO_COLLIDE_WIDTH)
				{
					*z2 &= ~0xffff;
					*z2 |=  0x10000 - NOGO_COLLIDE_WIDTH;
//					slide_nogo=1;
				}
			}

			if (collide & (NOGO_COLLIDE_SS|NOGO_COLLIDE_LS|NOGO_COLLIDE_SL|NOGO_COLLIDE_LL))
			{
				dx = *x2 & 0xffff;
				dz = *z2 & 0xffff;

				if (collide & (NOGO_COLLIDE_LS|NOGO_COLLIDE_LL)) {dx = 0x10000 - dx;}
				if (collide & (NOGO_COLLIDE_SL|NOGO_COLLIDE_LL)) {dz = 0x10000 - dz;}

				dist = QDIST2(dx,dz) + 1;

				if (dist < NOGO_COLLIDE_WIDTH)
				{
					dx = dx * NOGO_COLLIDE_WIDTH / dist;
					dz = dz * NOGO_COLLIDE_WIDTH / dist;

				   *x2 &= ~0xffff;
				   *z2 &= ~0xffff;

					if (collide & (NOGO_COLLIDE_LS|NOGO_COLLIDE_LL)) {dx = 0x10000 - dx;}
					if (collide & (NOGO_COLLIDE_SL|NOGO_COLLIDE_LL)) {dz = 0x10000 - dz;}

				   *x2 |= dx;
				   *z2 |= dz;
//					slide_nogo=1;
				}
			}
		}
	}

	#endif

	if (COLLIDE_can_i_fastnav( x1 >> 16,  z1 >> 16) &&
		COLLIDE_can_i_fastnav(*x2 >> 16, *z2 >> 16))
	{
		//
		// If these are fastnav squares, we can do nowt.  'fastnav'
		// means there aren't any colvects next to them.
		//

		return FALSE;
	}

	//
	// The box in which to search for colvects.
	//

	radius <<= 1;	// Search for colvects in twice the radius

	minx = ((*x2 >> 8)) - radius >> PAP_SHIFT_LO;
	minz = ((*z2 >> 8)) - radius >> PAP_SHIFT_LO;

	maxx = ((*x2 >> 8)) + radius >> PAP_SHIFT_LO;
	maxz = ((*z2 >> 8)) + radius >> PAP_SHIFT_LO;

	radius <<= 7;	// Put radius into 16-bits per mapsquare

	//
	// Make sure we do each facet only once.
	//

	SUPERMAP_counter_increase(0);

	//
	// Don't go off the map.
	//

	SATURATE(minx, 0, PAP_SIZE_LO - 1);
	SATURATE(minz, 0, PAP_SIZE_LO - 1);
	SATURATE(maxx, 0, PAP_SIZE_LO - 1);
	SATURATE(maxz, 0, PAP_SIZE_LO - 1);

	for (mx = minx; mx <= maxx; mx++)
	for (mz = minz; mz <= maxz; mz++)
	{
		//
		// Check all the facets on this square.
		//

		exit   = FALSE;
		f_list = PAP_2LO(mx,mz).ColVectHead;

		if (!f_list)
		{
			//
			// No facets on this square.
			//

			continue;
		}

		while(!exit)
		{

			i_facet = facet_links[f_list++];
			ASSERT(i_facet<next_dfacet);

			if (i_facet < 0)
			{
				i_facet = -i_facet;
				exit   =   TRUE;
			}

			df = &dfacets[i_facet];

			//
			// Done this one already?
			//

			if (df->Counter[0] == SUPERMAP_counter[0])
			{
				//
				// We've done this facet already.
				//

				continue;
			}

			//
			// Mark this facet as already done.
			//

			df->Counter[0] = SUPERMAP_counter[0];

			if (df->FacetType == STOREY_TYPE_CABLE)
			{
				//
				// Ignore cables in collision.
				//

				continue;
			}

			if (df->FacetType == STOREY_TYPE_OUTSIDE_DOOR && (df->FacetFlags & FACET_FLAG_OPEN))
			{
				//
				// Ignore open doors.
				//

				continue;
			}

			//
			// Is the 'y' component in range?
			//

			if (df->FacetType == STOREY_TYPE_FENCE_FLAT  ||
				df->FacetType == STOREY_TYPE_FENCE       ||
				df->FacetType == STOREY_TYPE_FENCE_BRICK ||
				df->FacetType == STOREY_TYPE_OUTSIDE_DOOR)
			{
				y_top = get_fence_top   (x1 >> 8, z1 >> 8, i_facet);
				y_bot = get_fence_bottom(x1 >> 8, z1 >> 8, i_facet) - 0x48;

				fence = TRUE;
			}
			else
			{
				fence=0;
				y_bot = df->Y[0] - 64;
				y_top = df->Y[0] + (df->Height * df->BlockHeight << 2);

				if (df->FHeight||df->FacetFlags&FACET_FLAG_HUG_FLOOR)
				{
					//
					// You can never walk under foundations.
					//

					y_bot = -0x7fff;
				}
			}

			//
			// Shall we ignore this facet?
			//

			SLONG ignore_this_facet = FALSE;

			if (just_started_falling_off_backwards)
			{
				//
				// Only collide with normal facets that go much
				// higher than you.
				//

				ignore_this_facet = TRUE;

				if (df->FacetType == STOREY_TYPE_NORMAL)
				{
					if (y_top > ((my_y1 >> 8) + 0x80))
					{
						ignore_this_facet = FALSE;
						y_bot -= 0x100;
					}
				}
			}

			//
			// A kludge to stop people going through walls and let people go over walls.
			//

			y_top += extra_wall_height;

			if (WITHIN(my_y1 >> 8, y_bot, y_top) && !ignore_this_facet)
			{
				//
				// Collide with this facet.
				//

				fx1 = df->x[0] << 16;
				fz1 = df->z[0] << 16;
				fx2 = df->x[1] << 16;
				fz2 = df->z[1] << 16;
#ifdef	UNUSED_WIRECUTTERS
				if(fence&& (flags&SLIDE_ALONG_FLAG_CRAWL))
				{
					SLONG	along;
					along=get_fence_hole(df);
					while(along)
					{
						SLONG	mx,mz;
						mx=(((fx2-fx1)*along)>>8)+fx1;
						mz=(((fz2-fz1)*along)>>8)+fz1;

						dx = *x2 - mx;
						dz = *z2 - mz;

						dist = QDIST2(abs(dx),abs(dz));

						if (dist < radius)
						{
							//
							// don't collide with this fence
							//
							goto skip_collide;
						}
						along=get_fence_hole_next(df, along);
					}
				}
#endif

				if (fx1 == fx2)
				{
					//
					// We ignore the sided-ness of the facet nowadays...
					//

					reverse = FALSE;

					if (fz1 > fz2) {SWAP(fz1,fz2); reverse = TRUE;}

					if (WITHIN(*z2, fz1 - radius, fz2 + radius) && (WITHIN(*x2, fx1 - radius, fx2 + radius) || (x1 < fx1 && *x2 > fx1) || (x1 > fx1 && *x2 < fx1)))
					{
						if (*z2 < fz1 && z1 < fz1)
						{
							//dx = (*x2 + x1 * 3 >> 2) - fx1;
							//dz = (*z2 + z1 * 3 >> 2) - fz1;

							dx = *x2 - fx1;
							dz = *z2 - fz1;

							dist = QDIST2(abs(dx),abs(dz));

							if (dist < radius)
							{
						 		*x2 = fx1 + dx * (radius >> 8) / ((dist >> 8) + 1);
								*z2 = fz1 + dz * (radius >> 8) / ((dist >> 8) + 1);
							}
						}
						else
						if (*z2 > fz2 && z1 > fz2)
						{
							//dx = (*x2 + x1 * 3 >> 2) - fx2;
							//dz = (*z2 + z1 * 3 >> 2) - fz2;

							dx = *x2 - fx2;
							dz = *z2 - fz2;

							dist = QDIST2(abs(dx),abs(dz));

							if (dist < radius)
							{
								*x2 = fx2 + dx * (radius >> 8) / ((dist >> 8) + 1);
								*z2 = fz2 + dz * (radius >> 8) / ((dist >> 8) + 1);
							}
						}
						else
						{
							if (df->FacetType == STOREY_TYPE_DOOR && !(flags&SLIDE_ALONG_FLAG_JUMPING))
							{
								//
								// Let the person go through the door. But only set slide_door if the
								// person is on the inside of the facet.
								//

								if (reverse)
								{
									if (*x2 > fx1)
									{
										if (dbuildings[df->Building].Type == BUILDING_TYPE_WAREHOUSE)
										{
											slide_into_warehouse = df->Building;
										}
										else
										{
											slide_door = df->DStorey;
										}
									}
									else
									{
										if (dbuildings[df->Building].Type == BUILDING_TYPE_WAREHOUSE)
										{
											slide_outof_warehouse = TRUE;
										}
									}
								}
								else
								{
									if (*x2 < fx1)
									{
										if (dbuildings[df->Building].Type == BUILDING_TYPE_WAREHOUSE)
										{
											slide_into_warehouse = df->Building;
										}
										else
										{
											slide_door = df->DStorey;
										}
									}
									else
									{
										if (dbuildings[df->Building].Type == BUILDING_TYPE_WAREHOUSE)
										{
											slide_outof_warehouse = TRUE;
										}
									}
								}

								return FALSE;
							}

							if (x1 < fx1)
							{
								*x2 = fx1 - radius;
							}
							else
							{
								*x2 = fx1 + radius;
							}
						}

						//
						// End up on the side you started.
						//

						/*

						if (x1 < fx1 && *x2 >= fx1)
						{
						   *x2 = fx1 - (*x2 - fx1);
						}
						else
						if (x1 > fx1 && *x2 <= fx1)
						{
						   *x2 = fx1 + (fx1 - *x2);
						}

						*/

						//
						// Set up the globals!
						//

						actual_sliding     = TRUE;
						last_slide_colvect = i_facet;

						if (fence)
						{
							slid_along_fence = TRUE;
							fence_colvect    = i_facet;
						}

						if (df->FacetType == STOREY_TYPE_LADDER)
						{
							slide_ladder = i_facet;
						}
					}
				}
				else
				{
					ASSERT(fz1 == fz2);

					//
					// We ignore the sided-ness of the facet nowadays...
					//

					reverse = FALSE;

					if (fx1 > fx2) {SWAP(fx1,fx2); reverse = TRUE;}

					if (WITHIN(*x2, fx1 - radius, fx2 + radius) && (WITHIN(*z2, fz1 - radius, fz2 + radius) || (z1 < fz1 && *z2 > fz1) || (z1 > fz1 && *z2 < fz1)))
					{
						if (*x2 < fx1 && x1 < fx1)
						{
							//dx = (*x2 + x1 * 3 >> 2) - fx1;
							//dz = (*z2 + z1 * 3 >> 2) - fz1;

							dx = *x2 - fx1;
							dz = *z2 - fz1;

							dist = QDIST2(abs(dx),abs(dz));

							if (dist < radius)
							{
								*x2 = fx1 + dx * (radius >> 8) / ((dist >> 8) + 1);
								*z2 = fz1 + dz * (radius >> 8) / ((dist >> 8) + 1);
							}
						}
						else
						if (*x2 > fx2 && x1 > fx2)
						{
							//dx = (*x2 + x1 * 3 >> 2) - fx2;
							//dz = (*z2 + z1 * 3 >> 2) - fz2;

							dx = *x2 - fx2;
							dz = *z2 - fz2;

							dist = QDIST2(abs(dx),abs(dz));

							if (dist < radius)
							{
								*x2 = fx2 + dx * (radius >> 8) / ((dist >> 8) + 1);
								*z2 = fz2 + dz * (radius >> 8) / ((dist >> 8) + 1);
							}
						}
						else
						{
							if (df->FacetType == STOREY_TYPE_DOOR && !(flags&SLIDE_ALONG_FLAG_JUMPING))
							{
								//
								// Let the person go through the door. But only set slide_door if the
								// person is on the inside of the facet.
								//

								if (reverse)
								{
									if (*z2 < fz1)
									{
										if (dbuildings[df->Building].Type == BUILDING_TYPE_WAREHOUSE)
										{
											slide_into_warehouse = df->Building;
										}
										else
										{
											slide_door = df->DStorey;
										}
									}
									else
									{
										if (dbuildings[df->Building].Type == BUILDING_TYPE_WAREHOUSE)
										{
											slide_outof_warehouse = TRUE;
										}
									}
								}
								else
								{
									if (*z2 > fz1)
									{
										if (dbuildings[df->Building].Type == BUILDING_TYPE_WAREHOUSE)
										{
											slide_into_warehouse = df->Building;
										}
										else
										{
											slide_door = df->DStorey;
										}
									}
									else
									{
										if (dbuildings[df->Building].Type == BUILDING_TYPE_WAREHOUSE)
										{
											slide_outof_warehouse = TRUE;
										}
									}
								}

								return FALSE;
							}

							if (z1 < fz1)
							{
								*z2 = fz1 - radius;
							}
							else
							{
								*z2 = fz1 + radius;
							}
						}

						//
						// End up on the side you started.
						//

						/*

						if (z1 < fz1 && *z2 >= fz1)
						{
						   *z2 = fz1 - (*z2 - fz1);
						}
						else
						if (z1 > fz1 && *z2 <= fz1)
						{
						   *z2 = fz1 + (fz1 - *z2);
						}

						*/

						//
						// Set up the globals!
						//

						actual_sliding     = TRUE;
						last_slide_colvect = i_facet;

						if (fence)
						{
							slid_along_fence = TRUE;
							fence_colvect    = i_facet;
						}

						if (df->FacetType == STOREY_TYPE_LADDER)
						{
							slide_ladder = i_facet;
						}
					}
				}
			}
#ifdef	UNUSED_WIRECUTTERS
skip_collide:;
#endif
		}
	}

	if (slid_along_fence)
	{
		//
		// If we slid along a fence make sure that is the one we return
		// as our last_slide_colvect;
		//

		last_slide_colvect = fence_colvect;
	}

	return FALSE;
}


#ifndef	PSX
#ifndef TARGET_DC

SLONG cross_door(SLONG  x1, SLONG  my_y1, SLONG  z1,
			SLONG  x2, SLONG  y2, SLONG  z2,SLONG  radius)
{
	SLONG i;

//	SLONG dx;
//	SLONG dz;

	SLONG minx;
	SLONG minz;
	SLONG maxx;
	SLONG maxz;

//	SLONG cprod;
//	SLONG dprod;

	SLONG y_top;
	SLONG y_bot;

	SLONG mx;
	SLONG mz;

//	SLONG vx;
//	SLONG vz;

//	SLONG sx;
//	SLONG sz;

	SLONG dist;
//	SLONG ndist;
	SLONG norm_x;
	SLONG norm_z;
	SLONG on;

	SLONG v_list;
	SLONG i_vect;

//	SLONG overndist;
	SLONG num_slides;
	SLONG last_slide;

//	SLONG old_start_x = x1;
//	SLONG old_start_y = my_y1;
//	SLONG old_start_z = z1;

//	SLONG old_end_x = x2;
//	SLONG old_end_y = y2;
//	SLONG old_end_z = z2;

//	CollisionVect *p_vect;
	DFacet *p_vect;
//	SLONG ans = 0;
//	SLONG	same_again=0;
	//
	// Remember colvect we have already done, and dont do them twice.
	//

//	#define MAX_ALREADY 8

//	UWORD already[MAX_ALREADY];
	SLONG already_upto = 0;


	//
	// The box in which to search for colvects.
	//

	x1 >>= 8;
	z1 >>= 8;
	my_y1 >>= 8;

	x2 = (x2) >> 8;
	y2 = (y2) >> 8;
	z2 = (z2) >> 8;
//	add_debug_line(x1,my_y1,z1,x2,y2,z2,0xffffff);

	minx = ((x2)) - radius >> PAP_SHIFT_LO;
	minz = ((z2)) - radius >> PAP_SHIFT_LO;

	maxx = ((x2)) + radius >> PAP_SHIFT_LO;
	maxz = ((z2)) + radius >> PAP_SHIFT_LO;

	//minx -= 1;
	//minz -= 1;
	//maxx += 1;
	//maxz += 1;

	//
	// How many col-vects we have slid along and
	// the last col-vect we slide along.
	//

	num_slides         = 0;
	last_slide         = 0;
	last_slide_colvect = NULL;
	slide_door         = 0;
	actual_sliding     = FALSE;

	for (mx = minx; mx <= maxx; mx++)
	for (mz = minz; mz <= maxz; mz++)
	{
		SLONG	fence=0;
		if (WITHIN(mx, 0, PAP_SIZE_LO  - 1) &&
			WITHIN(mz, 0, PAP_SIZE_LO - 1))
		{
			SLONG	exit=0;
			//
			// Check all the colvects on this square.
			//

			v_list = PAP_2LO(mx,mz).ColVectHead;

			if(v_list)
			while(!exit)
			{
//				ASSERT(WITHIN(v_list,                            0, next_dfacet - 1));
//				ASSERT(WITHIN(col_vects_links[v_list].VectIndex, 0, (next_col_vect      - 1)));


				i_vect =  facet_links[v_list];
				if(i_vect<0)
				{
					i_vect=-i_vect;
					exit=1;
				}
				p_vect = &dfacets[i_vect];

				//
				// Done this one already?
				//

				if (p_vect->FacetType == STOREY_TYPE_DOOR)
				{
					for (i = already_upto - 1; i >= 0; i--)
					{
						if (already[i] == i_vect)
						{
							//
							// Go to the next colvect in the linked list.
							//

							goto next_colvect;
						}
					}

					//
					// Remember that we have already done this colvect.
					//

					if (already_upto < MAX_ALREADY)
					{
						ASSERT(WITHIN(already_upto, 0, MAX_ALREADY - 1));

						already[already_upto++] = i_vect;
					}

					//
					// Is the 'y' component in range?
					//

					{
						SLONG vect_y;

						{
							//
							// its a door type facet
							//
							{
								SLONG	height;

								height=(p_vect->Height*p_vect->BlockHeight)<<2;
								fence=0;
								vect_y = p_vect->Y[0]; //get_height_along_vect(x1,z1,p_vect);

								y_bot = vect_y - 64;
								y_top = vect_y + height;

								if(p_vect->FacetType != STOREY_TYPE_CABLE)
								if (p_vect->FHeight )
								{
									y_bot = -INFINITY;
								}
							}
						}
					}

					//
					// A kludge to stop people going through walls and let people go over walls.
					//

//					y_top += extra_wall_height;

					if (WITHIN(my_y1, y_bot, y_top)) // Mike! I know this was you -> ||WITHIN(my_y1+60,y_bot,y_top)||WITHIN(my_y1-60,y_bot,y_top))
					{
						SLONG	side;
						SLONG	start_index=0,end_index=1;

						//
						// How far is (*x2,*z2) from the col-vect?
						//
//	again_for_fence:;
						signed_dist_to_line_with_normal_mark(
								p_vect->x[start_index] << 8, p_vect->z[start_index] << 8,
								p_vect->x[end_index  ] << 8, p_vect->z[end_index  ] << 8,
								x2, z2,
								&dist,
								&norm_x,
								&norm_z,
								&on);

						//
						// This piece of code would probably be quite readable if it wasnt for all the comments getting in the way
						//

						//
						// decideds weather to perform a slide or allow the movement, based on if you have gotten too close to the wall
						// or have passed from one side of the wall to the other
						// for normal buildings it allows you to pass from inside the building to the outside
						{
//							SLONG	slide=0;

							if(!on)
							{

							}
							else
							{
								//
								// what side did we start on
								//
								side = which_side(
										p_vect->x[start_index] << 8, p_vect->z[start_index] << 8,
										p_vect->x[end_index  ] << 8, p_vect->z[end_index  ] << 8,
										x1, z1);

								if(dist<0 && dist>-60 &&side<0)
								{
									// you ended on the wrong side
									// you started on the wrong side
									// your near enough the wall

									return(-1);
									//
									// which means dont do slide inside so we can cross the door
									//
								}
								if(dist<0&&side>0)
								{
									// you ended on the wrong side of the wall
									// but you started on the right side

									// so you have entered a building which is odd
									ASSERT(0);
									return(-1);
								}
								else
								if(dist>0&&side<0)
								{
									// you started on the wrong side of the wall
									// but you ended on the right side

									return(1); //have left the building
								}
							}
						}
					}

				}

			  next_colvect:;

				v_list++;
			}
		}
	}

	return 0;
}
#endif
#endif
#ifndef	PSX
#ifndef TARGET_DC
SLONG	bump_person(Thing *p_person,THING_INDEX index,SLONG x1,SLONG my_y1,SLONG z1,SLONG *x2,SLONG *y2,SLONG *z2)
{
	SLONG	bump_radius,my_radius;
	Thing	*p_bumped;

	SLONG	ddx,ddy,ddz,dist;

	SLONG	ex,ey,ez;

	ex=*x2>>8;
	ey=*y2>>8;
	ez=*z2>>8;

	p_bumped=TO_THING(index);

	bump_radius = get_person_radius(p_bumped->Genus.Person->PersonType);
	my_radius = get_person_radius(p_person->Genus.Person->PersonType);

	x1>>=8;
	my_y1>>=8;
	z1>>=8;

	*x2>>=8;
	*y2>>=8;
	*z2>>=8;

	ddx=abs(ex-(p_bumped->WorldPos.X>>8));
	ddy=abs(ey-(p_bumped->WorldPos.Y>>8));
	ddz=abs(ez-(p_bumped->WorldPos.Z>>8));

	dist=QDIST2(ddx,ddz);

	if(dist<(bump_radius+my_radius)&&ddy<150)
	{
		SLONG	dist2;
		SLONG	odx,odz;
		//
		// are radia intersect so check if we are moving nearer or further from collision
		//

		odx=abs((x1)-(p_bumped->WorldPos.X>>8));
		odz=abs((z1)-(p_bumped->WorldPos.Z>>8));

		dist2=QDIST2(odx,odz);

		if(dist<dist2)
		{
//			SLONG	tdx,tdz;
			SLONG	angle;
//			SLONG	radius=bump_radius+my_radius;


			angle=Arctan(-ddx,ddz)+1024;  //oh no mystery arctan function

			//
			// Angle round the circle we bump at
			//

			*x2=x1;//+(COS(angle)*radius>>16);
			*z2=z1;//+(SIN(angle)*radius>>16);

			//
			// should use the tangent as a movement vector to make up for lost distance
			//





			//
			// distance with move vect is smaller than without movement therefore collision is valid
			//

			p_person->Genus.Person->InWay=index;

			//
			// Return in the same coordinate system.
			//

			*x2 <<= 8;
			*y2 <<= 8;
			*z2 <<= 8;

			return(0);
		}
	}

	*x2 <<= 8;
	*y2 <<= 8;
	*z2 <<= 8;

	return(0);
}
#endif
#endif


//
// Slides the vector along the slidey edges of the given walkable face4.
//

void slide_along_edges(
		SLONG face4,
		SLONG  x1, SLONG  z1,
		SLONG *x2, SLONG *z2)
{
	SLONG i;
	SLONG p1;
	SLONG p2;

	SLONG ex1, ez1;
	SLONG ex2, ez2;

	SLONG dex;
	SLONG dez;

	SLONG elen;

	SLONG vecx;
	SLONG vecz;



	SLONG ox;
	SLONG oz;



	UBYTE point_order[4] = {0, 1, 3, 2};

	ASSERT(WITHIN(face4, 1, next_prim_face4));

	PrimFace4 *f4 = &prim_faces4[face4];



	ox = 0; //p_thing->WorldPos.X >> 8;
	oz = 0; //p_thing->WorldPos.Z >> 8;

	//
	// Check each edge in turn.
	//

	for (i = 0; i < 4; i++)
	{
		if (f4->FaceFlags & (FACE_FLAG_SLIDE_EDGE << i))
		{
			p1 = f4->Points[point_order[(i + 0) & 0x3]];
			p2 = f4->Points[point_order[(i + 1) & 0x3]];

			//
			// This edge.
			//

			ex1 = prim_points[p1].X + ox;
			ez1 = prim_points[p1].Z + oz;

			ex2 = prim_points[p2].X + ox;
			ez2 = prim_points[p2].Z + oz;

			dex = ex2 - ex1;
			dez = ez2 - ez1;

			//
			// Are we moving to the wrong side of this edge?
			//

			vecx = (*x2>>8) - ex1;
			vecz = (*z2>>8) - ez1;

			cprod = dex*vecz - dez*vecx;

			if (cprod < 0)
			{
				//
				// Move (*x2,*z2) onto this face.
				//

				elen  = QDIST2(abs(dex),abs(dez));
				cprod = (cprod *  256) / (elen * elen);

				*x2 += dez * cprod ;//>> 8;
				*z2 -= dex * cprod ;//>> 8;

				//
				// A little bit away from the edge...
				//

				*x2 -= SIGN(dez) << (3+8);
				*z2 += SIGN(dex) << (3+8);

//				MSG_add("Sliding");
			}
		}
	}
}


void slide_along_redges(
		SLONG face4,
		SLONG  x1, SLONG  z1,
		SLONG *x2, SLONG *z2)
{
	SLONG i;

	SLONG mx;
	SLONG mz;

	SLONG height1;
	SLONG height2;

	SLONG hard_edge;

	RoofFace4 *f4;

	if (IS_ROOF_HIDDEN_FACE(-face4))
	{
		f4      = NULL;
		mx      = ROOF_HIDDEN_X(-face4);
		mz      = ROOF_HIDDEN_Z(-face4);

		mx = *x2 >> 16;
		mz = *z2 >> 16;

		height1 = MAVHEIGHT(mx,mz);
	}
	else
	{
		f4      = &roof_faces4[face4];
		mx      =  f4->RX & 127;
		mz      =  f4->RZ & 127;
		height1 =  0;
	}

	for (i = 0; i < 4; i++)
	{
		//
		// Do we slide against this edge?
		//

		hard_edge = FALSE;

		if (f4)
		{
			hard_edge = f4->DrawFlags & (RFACE_FLAG_SLIDE_EDGE << i);
		}
		else
		{
			const struct
			{
				SBYTE dx;
				SBYTE dz;

			} dir[4] =
			{
				{ 0,-1},
				{ 1, 0},
				{ 0, 1},
				{-1, 0}
			};

			if (WITHIN(mx + dir[i].dx, 0, PAP_SIZE_HI - 1) &&
				WITHIN(mz + dir[i].dz, 0, PAP_SIZE_HI - 1))
			{
				height2 = MAVHEIGHT(mx + dir[i].dx, mz + dir[i].dz);

				if (abs(height1 - height2) > 1)
				{
					hard_edge = TRUE;
				}
			}
		}

		if (hard_edge)
		{
			#define EDGE_WIDTH 0x4000

			switch(i)
			{
				case 0:	// ZS

					if ((*z2 & 0xffff) < EDGE_WIDTH)
					{
						*z2 &= ~0xffff;
						*z2 |=  EDGE_WIDTH;
					}

					break;

				case 1:	// XL

					if ((*x2 & 0xffff) > 0x10000 - EDGE_WIDTH)
					{
						*x2 &= ~0xffff;
						*x2 |=  0x10000 - EDGE_WIDTH;
					}

					break;

				case 2: // ZL

					if ((*z2 & 0xffff) > 0x10000 - EDGE_WIDTH)
					{
						*z2 &= ~0xffff;
						*z2 |=  0x10000 - EDGE_WIDTH;
					}

					break;

				case 3: // XS

					if ((*x2 & 0xffff) < EDGE_WIDTH)
					{
						*x2 &= ~0xffff;
						*x2 |=  EDGE_WIDTH;
					}

					break;

				default:
					ASSERT(0);
					break;
			}
		}
	}
}

/*

UBYTE	roofdxdz[4][2]=
{
	{0,0},
	{1,0},
	{1,1},
	{0,1}
};


SBYTE	roof_dx[]=
{
	0,//n
	1, //e
	0, //s
	-1, //w
};

SBYTE	roof_dz[]=
{
	-1,//n
	0, //e
	1, //s
	0, //w
};

void slide_along_redges(
		SLONG face4,
		SLONG  x1, SLONG  z1,
		SLONG *x2, SLONG *z2)
{
	SLONG i;
	SLONG p1;
	SLONG p2;

	SLONG ex1, ez1;
	SLONG ex2, ez2;

	SLONG dex;
	SLONG dez;

	SLONG elen;

	SLONG vecx;
	SLONG vecz;

	SLONG ox;
	SLONG oz;
	SLONG	height1;

	RoofFace4 *f4;// = &roof_faces4[face4];

	ASSERT(face4>0);

	if(IS_ROOF_HIDDEN_FACE(-face4))
	{
		//
		// need to make a slide along that works on mavheight!
		//
		f4=0;
		ox=ROOF_HIDDEN_X(-face4)<<8;
		oz=ROOF_HIDDEN_Z(-face4)<<8;
		height1=MAVHEIGHT(ox>>8,oz>>8);
	}
	else
	{
		f4 = &roof_faces4[face4];
		ox=(f4->RX&127)<<8;
		oz=(f4->RZ&127)<<8;
	}


	//
	// Check each edge in turn.
	//

	for (i = 0; i < 4; i++)
	{
		SLONG hard_edge;
		SLONG	height2;

		if(f4)
		{
			hard_edge=f4->DrawFlags & (RFACE_FLAG_SLIDE_EDGE << i);
		}
		else
		{
			height2=MAVHEIGHT((ox>>8)+roof_dx[i],(oz>>8)+roof_dz[i]);

			if(abs(height1-height2)>1)
				hard_edge=1;
			else
				hard_edge=0;
		}

		if (hard_edge)//f4->DrawFlags & (RFACE_FLAG_SLIDE_EDGE << i))
		{
			ex1=ox+(roofdxdz[i][0]<<8);
			ez1=oz+(roofdxdz[i][1]<<8);

			ex2=ox+(roofdxdz[(i+1)&3][0]<<8);
			ez2=oz+(roofdxdz[(i+1)&3][1]<<8);

			dex = ex2 - ex1;
			dez = ez2 - ez1;

			//
			// Are we moving to the wrong side of this edge?
			//

			vecx = (*x2>>8) - ex1;
			vecz = (*z2>>8) - ez1;

			cprod = dex*vecz - dez*vecx;

			if (cprod < 0)
			{
				//
				// Move (*x2,*z2) onto this face.
				//

				elen  = QDIST2(abs(dex),abs(dez));
				cprod = (cprod *  256) / (elen * elen);

				*x2 += dez * cprod ;//>> 8;
				*z2 -= dex * cprod ;//>> 8;
		ASSERT(abs(*x2-x1)>>16<2);
		ASSERT(abs(*z2-z1)>>16<2);

				//
				// A little bit away from the edge...
				//

				*x2 -= SIGN(dez) << (3+8);
				*z2 += SIGN(dex) << (3+8);

		ASSERT(abs(*x2-x1)>>16<2);
		ASSERT(abs(*z2-z1)>>16<2);

//				MSG_add("Sliding");
			}
		}
	}
}

*/

ULONG move_thing_quick(SLONG dx,SLONG dy,SLONG dz,Thing *p_thing)
{
	GameCoord new_position;

	new_position.X = p_thing->WorldPos.X+dx;
	new_position.Y = p_thing->WorldPos.Y+dy;
	new_position.Z = p_thing->WorldPos.Z+dz;

	move_thing_on_map(p_thing,&new_position);
	return(0);
}


//
// Collides the given movement vector with objects. Returns TRUE if
// a collision occurred.
//

SLONG collide_against_objects(
		Thing *p_thing,
		SLONG  radius,
		SLONG  x1, SLONG  my_y1, SLONG  z1,
		SLONG *x2, SLONG *y2, SLONG *z2)
{
	OB_Info *oi;

	SLONG old_x2;
	SLONG old_y2;
	SLONG old_z2;

	SLONG mx;
	SLONG mz;

	SLONG y_top;

	SLONG xmin, zmin;
	SLONG xmax, zmax;

	SLONG ans;

	PrimInfo *pi;

	ans = FALSE;

	xmin = (*x2 >> 8) - 0x180 >> PAP_SHIFT_LO;
	zmin = (*z2 >> 8) - 0x180 >> PAP_SHIFT_LO;

	xmax = (*x2 >> 8) + 0x180 >> PAP_SHIFT_LO;
	zmax = (*z2 >> 8) + 0x180 >> PAP_SHIFT_LO;

	SATURATE(xmin, 0, PAP_SIZE_LO - 1);
	SATURATE(zmin, 0, PAP_SIZE_LO - 1);

	SATURATE(xmax, 0, PAP_SIZE_LO - 1);
	SATURATE(zmax, 0, PAP_SIZE_LO - 1);

	//
	// If this person is standing on a prim, don't collide with that prim!
	//

	SLONG ignore_prim = NULL;

	if (p_thing->OnFace>0)
	{
		if (prim_faces4[p_thing->OnFace].FaceFlags &FACE_FLAG_PRIM)
		{
			//
			// The ThingIndex of the face contains the index of the OB * -1
			//

			ignore_prim = -prim_faces4[p_thing->OnFace].ThingIndex;
		}
	}

	for (mx = xmin; mx <= xmax; mx++)
	for (mz = zmin; mz <= zmax; mz++)
	{
		//
		// Find all the objects on this lo-res mapsquare.
		//

		for (oi = OB_find(mx,mz); oi->prim; oi++)
		{
			if (oi->index == ignore_prim)
			{
				//
				// Ignore this prim.
				//

				continue;
			}

			switch(prim_get_collision_model(oi->prim))
			{
				case PRIM_COLLIDE_BOX:
				case PRIM_COLLIDE_SMALLBOX:

					old_x2 = *x2;
					old_y2 = *y2;
					old_z2 = *z2;

					if (slide_along_prim(
							oi->prim,
							oi->x,
							oi->y,
							oi->z,
							oi->yaw,
							x1, my_y1, z1,
							x2, y2, z2,
							radius,
							prim_get_collision_model(oi->prim) == PRIM_COLLIDE_SMALLBOX,
							FALSE))
					{
						//
						// Slid along this prim.
						//

						ans = TRUE;

						if (p_thing)
						{
							if (p_thing->Class == CLASS_PERSON)
							{
								if (p_thing->Genus.Person->PlayerID)
								{
									//
									// This is a player. Are they walking backwards?
									//

									if (p_thing->SubState == SUB_STATE_WALKING_BACKWARDS)
									{
										//
										// Is it a prim we can sit on?
										//

										if (oi->prim == 105 ||
											oi->prim == 101 ||
											oi->prim == 110 ||
											oi->prim == 89  ||
											oi->prim == 126 ||
											oi->prim == 95  ||
											oi->prim == 102)
										{
											SLONG dangle = oi->yaw - p_thing->Draw.Tweened->Angle;

											dangle &= 2047;

											if (dangle > 1024)
											{
												dangle -= 2048;
											}

											if (abs(dangle) < 220)
											{
												//
												// Sit on the prim!
												//

												set_person_sit_down(p_thing);

												return ans;
											}
										}
									}
								}
								else
								{
									//
									// This is not a player: turn to avoid the prim.
									//

									SLONG angle;
									SLONG dangle;

									angle  = p_thing->Draw.Tweened->Angle;
									dangle = OB_avoid(
												oi->x,
												oi->y,
												oi->z,
												oi->yaw,
												oi->prim,
												x1,  z1,
											   *x2, *z2);

									angle += dangle * 256;
									angle &= 2047;

									p_thing->Draw.Tweened->Angle = angle;
								}
							}
						}
					}

					break;

				case PRIM_COLLIDE_NONE:
					break;

				case PRIM_COLLIDE_CYLINDER:

					//
					// The prim info...
					//

					pi = get_prim_info(oi->prim);

					y_top = oi->y + pi->maxy;

					if (oi->prim == PRIM_OBJ_SPIKE)
					{
						y_top = 0xc0;
					}

					if ((my_y1 >> 8) < y_top)
					{
						if (slide_around_circle(
								oi->x << 8,
								oi->z << 8,
								radius + 0x40 << 8,
								x1, z1, x2, z2))
						{
							ans = TRUE;

							if (p_thing)
							{
								if (p_thing->Class == CLASS_PERSON)
								{
									if (oi->prim == PRIM_OBJ_SPIKE)
									{
										//
										// Too near to the spike! DEATH!
										//

										set_person_dead(
											p_thing,
											NULL,
											PERSON_DEATH_TYPE_OTHER,
											FALSE,
											0);
									}

									if (p_thing->Genus.Person->PlayerID)
									{
										//
										// This is a player. Are they walking backwards?
										//

										if (p_thing->SubState == SUB_STATE_WALKING_BACKWARDS)
										{
											//
											// Is it a prim we can sit on?
											//

											if (oi->prim == 70  ||
												oi->prim == 30  ||
												oi->prim == 208 ||
												oi->prim == 215 ||
												oi->prim == 250 ||
												oi->prim == 251 ||
												oi->prim == 252)
											{
												//
												// Are we facing the thing?
												//

												SLONG dx = oi->x - (p_thing->WorldPos.X >> 8);
												SLONG dz = oi->z - (p_thing->WorldPos.Z >> 8);

												SLONG lx = SIN(p_thing->Draw.Tweened->Angle) >> 8;
												SLONG lz = COS(p_thing->Draw.Tweened->Angle) >> 8;

												SLONG len = QDIST2(abs(dx),abs(dz)) + 1;

												dx = dx * 256 / len;
												dz = dz * 256 / len;

												SLONG dprod = lx*dx + dz*lz >> 8;

												if (dprod > 200)
												{
													//
													// Sit on the prim!
													//

													set_person_sit_down(p_thing);

													return ans;
												}
											}
										}
									}
								}
							}
						}
					}

					break;

				default:
					ASSERT(0);
					break;
			}
		}
	}

	return ans;
}


//
// Returns TRUE if you should collide against the given anim prim
//

#ifdef	ANIM_PRIM
SLONG should_i_collide_against_this_anim_prim(Thing *p_aprim)
{
	SLONG collide_or_not = FALSE;

	switch(get_anim_prim_type(p_aprim->Index))
	{
		default:
		case ANIM_PRIM_TYPE_NORMAL:

			//
			// Always collide with normal anim-prims.
			//

			collide_or_not = TRUE;

			break;

		case ANIM_PRIM_TYPE_DOOR:

			//
			// Collide only if shut.
			//

			collide_or_not = !!(p_aprim->Flags & FLAGS_SWITCHED_ON);

			break;

		case ANIM_PRIM_TYPE_SWITCH:

			//
			// Ignore these: never collide.
			//

			break;
	}

	return collide_or_not;

}
#endif


//
// Collides the given movement vector with things. Returns TRUE if a
// collision occurred.
//

#define MAX_COL_WITH 16

THING_INDEX col_with_things[MAX_COL_WITH];

SLONG collide_against_things(
		Thing *p_thing,
		SLONG  radius,
		SLONG  x1, SLONG  my_y1, SLONG  z1,
		SLONG *x2, SLONG *y2, SLONG *z2)
{
	UBYTE i;
	UBYTE col_with_upto;
//	UBYTE collide_or_not;
	Thing *col_thing;
	ULONG collide_types;
	SLONG ans;

	SLONG on;

	ans = FALSE;

	//
	// Which things do we collide with?
	//

	on=person_is_on(p_thing);

	if (on== PERSON_ON_PRIM || on==PERSON_ON_METAL)
	{
		collide_types = (1 << CLASS_PERSON);
	}
	else
	{
		collide_types =
			(1 << CLASS_PERSON)    |
			(1 << CLASS_FURNITURE) |
			(1 << CLASS_VEHICLE)   |
			(1 << CLASS_ANIM_PRIM) |
			(1 << CLASS_PYRO)      |
			(1 << CLASS_PLAT)      |
			(1 << CLASS_BAT)       |
			(1 << CLASS_BIKE);
	}

	//
	// Find any things we ought to collide with.
	//

 	col_with_upto = THING_find_sphere(
					    p_thing->WorldPos.X >> 8,
						p_thing->WorldPos.Y >> 8,
						p_thing->WorldPos.Z >> 8,
						radius + 0x1a0,	// Some things can be quite big... vans?
						col_with_things,
						MAX_COL_WITH,
						collide_types);

	ASSERT(col_with_upto <= MAX_COL_WITH);

	for (i = 0; i < col_with_upto; i++)
	{
		col_thing = TO_THING(col_with_things[i]);

		if (col_thing->State == STATE_DEAD ||
		    col_thing->State == STATE_DYING)
		{
			//
			// Dead or dying things are of no interest
			//

			if (col_thing->Class == CLASS_VEHICLE)
			{
				//
				// Still collide against dead cars...
				//
			}
			else
			{
				continue;
			}
		}

		switch(col_thing->Class)
		{
			case CLASS_PERSON:

				if (col_thing != p_thing)
				{
					if(p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP)
					{
						SLONG	tx2,tz2;
						SLONG	radius;

						if(p_thing->Genus.Person->PlayerID)
							radius=66<<8;
						else
							radius=50<<8;
#ifndef PSX
/*
						{
							SLONG	cx,cz,cy,r,ang,x1,z1,x2,z2;
							cx=col_thing->WorldPos.X>>8;
							cy=col_thing->WorldPos.Y>>8;
							cz=col_thing->WorldPos.Z>>8;
							r=radius>>8;

							x1=cx+(r*COS(0)>>16);
							z1=cz+(r*SIN(0)>>16);
							for(ang=0;ang<2048;ang+=128)
							{
								x2=cx+(r*COS(ang)>>16);
								z2=cz+(r*SIN(ang)>>16);

								add_debug_line(x1,cy+10,z1,x2,cy+10,z2,0xffffff);
								x1=x2;
								z1=z2;

							}
						}

						add_debug_line(x1,10,z1,col_thing->WorldPos.X>>8,10,col_thing->WorldPos.Z>>8,0xff00ff);
*/
#endif

						tx2=*x2;
						tz2=*z2;
						{

							if(slide_around_circle(
								col_thing->WorldPos.X,
								col_thing->WorldPos.Z,
								radius ,
								x1, z1, &tx2, &tz2))
							{
								//*x2=tx2;
								//*z2=tz2;
								sweep_feet(col_thing,p_thing,0);

								ans = TRUE;
							}
						}
					}
					else
					if(p_thing->SubState==SUB_STATE_STEP_FORWARD)
					{
						//
						// if your fighting people we don't want to slide round them we want them to stop you in your tracks
						//
						if(collide_with_circle(
							col_thing->WorldPos.X,
							col_thing->WorldPos.Z,
							50 << 8,
							x2, z2))
						{
							*x2=x1;
							*z2=z1;

							ans = TRUE;
						}
					}
					else
					{
						if (p_thing  ->Genus.Person->pcom_ai == PCOM_AI_CIV &&
							col_thing->Genus.Person->pcom_ai == PCOM_AI_CIV &&
							col_thing < p_thing)
						{
							//
							// An ordering on civs so nobody gets stuck at lamposts...
							//
						}
						else
						if (col_thing->Genus.Person->pcom_ai_state == PCOM_AI_STATE_FOLLOWING &&
							col_thing->Genus.Person->pcom_ai_arg   == THING_NUMBER(p_thing))
						{
							//
							// Don't collide with people who are following you.
							//
						}
						else
						{
							if (slide_around_circle(
									col_thing->WorldPos.X,
									col_thing->WorldPos.Z,
									50 << 8,
									x1, z1, x2, z2))
							{
								ans = TRUE;
							}
						}
					}
				}

				break;

			case CLASS_PLAT:

				if (slide_along_prim(
						col_thing->Draw.Mesh->ObjectId,
						col_thing->WorldPos.X >> 8,
						col_thing->WorldPos.Y >> 8,
						col_thing->WorldPos.Z >> 8,
						col_thing->Draw.Mesh->Angle,
						x1, my_y1, z1,
						x2, y2, z2,
						radius,
						FALSE,
						FALSE))
				{
					ans = TRUE;
				}

				break;

			case CLASS_VEHICLE:

				{
					SLONG prim;

					if(p_thing->Genus.Person->InCar==THING_NUMBER(col_thing))
					{

					}
					else
					{
						prim = get_vehicle_body_prim(col_thing->Genus.Vehicle->Type);

						if (slide_along_prim(
								prim,
								col_thing->WorldPos.X                                                           >> 8,
								col_thing->WorldPos.Y - get_vehicle_body_offset(col_thing->Genus.Vehicle->Type) >> 8,
								col_thing->WorldPos.Z                                                           >> 8,
								col_thing->Genus.Vehicle->Angle,
								x1, my_y1, z1,
								x2, y2, z2,
								radius,
								FALSE,
								FALSE))
						{
							//
							// Slid along this prim.
							//

							ans = TRUE;

							if (p_thing && p_thing->Class == CLASS_PERSON)
							{
								if (col_thing->Velocity > 200)
								{
									/*

									Thing *p_driver = get_vehicle_driver(col_thing);

									//
									// Run this person over.
									//

									knock_person_down(
										p_thing,
										100 + (col_thing->Velocity >> 4),
										col_thing->WorldPos.X >> 8,
										col_thing->WorldPos.Z >> 8,
										p_driver);

									MFX_play_thing(THING_NUMBER(p_thing),S_THUMP_SQUISH,MFX_REPLACE,p_thing);

									return;

									*/

								}
								else
								{
									if (!p_thing->Genus.Person->PlayerID)
									{
										//
										// Turn to avoid the prim.
										//

										SLONG angle;
										SLONG dangle;

										angle  = p_thing->Draw.Tweened->Angle;
										dangle = OB_avoid(
													col_thing->WorldPos.X>>8,
													col_thing->WorldPos.Y>>8,
													col_thing->WorldPos.Z>>8,
													col_thing->Genus.Vehicle->Angle,
													prim,
													x1,  z1,
												   *x2, *z2);

										angle += dangle * 160;
										angle &= 2047;

										p_thing->Draw.Tweened->Angle = angle;
									}
								}
							}
						}
					}
				}

				break;
#ifdef	ANIM_PRIMS
			case CLASS_ANIM_PRIM:

				//
				// Do we collide with this anim-prim or not?
				//

				//collide_or_not = should_i_collide_against_this_anim_prim(col_thing);

				//if (collide_or_not)
				if(should_i_collide_against_this_anim_prim(col_thing))
				{
					SLONG old_x2 = *x2;
					SLONG old_z2 = *z2;

				   *x2 >>= 8;
				   *z2 >>= 8;

					//
					// We have to collide. What is the bounding box of this anim_prim?
					//

					AnimPrimBbox *apb = &anim_prim_bbox[col_thing->Index];

					if (!slide_around_box_lowstack(
							col_thing->WorldPos.X >> 8,
							col_thing->WorldPos.Z >> 8,
							apb->minx,
							apb->minz,
							apb->maxx,
							apb->maxz,
							col_thing->Draw.Tweened->Angle,
							50,
							x1 >> 8, z1 >> 8,
							x2,
							z2))
					{
						*x2 = old_x2;
						*z2 = old_z2;

						ans = TRUE;
					}
					else
					{
						ASSERT(WITHIN(*x2, 0, PAP_SIZE_HI << PAP_SHIFT_HI));
						ASSERT(WITHIN(*z2, 0, PAP_SIZE_HI << PAP_SHIFT_HI));

						*x2 <<= 8;
						*z2 <<= 8;
					}
				}

				break;
#endif
			case CLASS_PYRO:
				switch(col_thing->Genus.Pyro->PyroType) {
				case PYRO_BONFIRE:
				case PYRO_FLICKER:
					if (p_thing->Class==CLASS_PERSON)
					{
						if ((p_thing->Genus.Person->Flags     & FLAG2_PERSON_INVULNERABLE) ||
							(p_thing->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL))
						{
							//
							// This person doesn't catch alight.
							//
						}
						else
						{
							SLONG	dist2;
							SLONG	odx,ody,odz;

							{
								odx=abs((p_thing->WorldPos.X>>8)-(col_thing->WorldPos.X>>8));
								odz=abs((p_thing->WorldPos.Z>>8)-(col_thing->WorldPos.Z>>8));

								dist2=QDIST2(odx,odz);

								if (p_thing->Genus.Person->pcom_ai   == PCOM_AI_CIV ||
									p_thing->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
								{
									if (dist2 < 200)
									{
										//
										// Wandering civs don't catch fire- they run away.
										//

										extern void PCOM_set_person_ai_flee_place(
															Thing *p_person,
															SLONG  scary_x,
															SLONG  scary_z);

										PCOM_set_person_ai_flee_place(
											p_thing,
											col_thing->WorldPos.X >> 8,
											col_thing->WorldPos.Z >> 8);

										return ans;
									}
								}

								if (dist2 < 100)
								{
									SLONG fx;
									SLONG fy;
									SLONG fz;

									calc_sub_objects_position(
										p_thing,
										p_thing->Draw.Tweened->AnimTween,
										SUB_OBJECT_LEFT_FOOT,
									   &fx,
									   &fy,
									   &fz);

									fy += p_thing->WorldPos.Y >> 8;

									ody = abs(fy - (col_thing->WorldPos.Y >> 8));

									if (ody < 0x50)
									{
										p_thing->Flags |= FLAGS_BURNING;
									}
								}
							}
						}
					}
				}
				break;

			#if BIKE

			case CLASS_BIKE:

				{
					SLONG dy = col_thing->WorldPos.Y - my_y1 >> 8;

					if (abs(dy) < 0x100)
					{
						slide_around_circle(
							col_thing->WorldPos.X,
							col_thing->WorldPos.Z,
							0x40 << 8,
							x1, z1, x2, z2);

						ans = TRUE;
					}
				}

				break;

			#endif

			case CLASS_BAT:

				if (col_thing->Genus.Bat->type == BAT_TYPE_BALROG)
				{
					SLONG dy = col_thing->WorldPos.Y - my_y1 >> 8;

					if (abs(dy) < 0x180)
					{
						SLONG dx = abs(col_thing->WorldPos.X - x1 >> 8);
						SLONG dz = abs(col_thing->WorldPos.Z - z1 >> 8);

						if (QDIST2(dx,dz) < 0x100)
						{
							if (p_thing->State != STATE_JUMPING &&
								p_thing->State != STATE_DANGLING)
							{
								knock_person_down(
									p_thing,
									50,
									col_thing->WorldPos.X >> 8,
									col_thing->WorldPos.Z >> 8,
									col_thing);
							}
						}
					}
				}

				break;

			default:
				ASSERT(0);
				break;
		}
	}

	return ans;
}

void drop_on_heads(Thing *p_thing)
{
	UBYTE i;
	UBYTE col_with_upto;
	UBYTE collide_or_not;
	Thing *col_thing;
	ULONG collide_types;
	SLONG	fx,fy,fz,hx,hy,hz;

	//
	// Which things do we collide with?
	//

	if(p_thing->DY >  -15000)
		return;

//	ASSERT(0);
	collide_types = (1 << CLASS_PERSON);

	calc_sub_objects_position(p_thing,p_thing->Draw.Tweened->AnimTween,SUB_OBJECT_LEFT_FOOT,&fx,&fy,&fz);
	fx+=p_thing->WorldPos.X>>8;
	fy+=p_thing->WorldPos.Y>>8;
	fz+=p_thing->WorldPos.Z>>8;


	//
	// Find any things we ought to collide with.
	//

	col_with_upto = THING_find_sphere(
					    fx,
						fy,
						fz,
						200,
						col_with_things,
						MAX_COL_WITH,
						collide_types|(1<<31));

	ASSERT(col_with_upto <= MAX_COL_WITH);

	for (i = 0; i < col_with_upto; i++)
	{
		SLONG	dx,dy,dz;
		col_thing = TO_THING(col_with_things[i]);

		ASSERT(col_thing->Class==CLASS_PERSON);

		if (col_thing->State == STATE_DEAD ||
		    col_thing->State == STATE_DYING)
		{
			//
			// Dead or dying things are of no interest
			//

			continue;
		}

		//
		// calc position of head
		//
extern	SLONG	people_allowed_to_hit_each_other(Thing *p_victim,Thing *p_agressor);

		if (col_thing != p_thing)
		{
			if (col_thing->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
			{
				//
				// Don't knock out invulnerable people.
				//
			}
			else
			{
			   if(people_allowed_to_hit_each_other(col_thing,p_thing))
			   if (PCOM_person_a_hates_b(p_thing, col_thing))
			   {
					calc_sub_objects_position(col_thing,col_thing->Draw.Tweened->AnimTween,SUB_OBJECT_HEAD,&hx,&hy,&hz);
					hx+=col_thing->WorldPos.X>>8;
					hy+=col_thing->WorldPos.Y>>8;
					hz+=col_thing->WorldPos.Z>>8;

					dx=abs(fx-hx);
					dz=abs(fz-hz);

					if(QDIST2(dx,dz)<100)
					{
						SLONG	miny,maxy;

						miny=hy-(MAX(abs(p_thing->DY>>8),100)+10);
						maxy=hy+MAX(abs(p_thing->DY>>9),100);

						if(fy>miny && fy<maxy)
						{
							//
							// dropping onto someone from above
							//

							if (col_thing->Genus.Person->PersonType == PERSON_MIB1 ||
								col_thing->Genus.Person->PersonType == PERSON_MIB2 ||
								col_thing->Genus.Person->PersonType == PERSON_MIB3)
							{
								//
								// You can't jump on MIB heads.
								//

								PCOM_attack_happened(col_thing, p_thing);
							}
							else
							{
								//
								// Tell the AI system it happened.
								//

								PCOM_attack_happened(col_thing, p_thing);

								//
								// behind and high
								//

								extern SLONG is_there_room_behind_person(Thing *p_person, SLONG hit_from_behind);

								SLONG behind = (Random() & 0x1);

								if (!is_there_room_behind_person(col_thing, behind))
								{
									behind = !behind;
								}

								set_person_dead(col_thing,p_thing,PERSON_DEATH_TYPE_STAY_ALIVE,behind,3);
							}
						}
					}
				}
			}
		}
	}
}

#define	FALL_OFF_FLAG_TRUE		  (1 << 0)
#define	FALL_OFF_FLAG_FIRE_ESCAPE (1 << 1)
#define FALL_OFF_FLAG_DONT_GRAB   (1 << 2)

SLONG x1, my_y1, z1;
SLONG x2, y2, z2;

ULONG move_thing(
		SLONG dx,
		SLONG dy,
		SLONG dz,
		Thing *p_thing)
{
	SLONG col;
	SLONG radius;
	SLONG new_y;
	SLONG new_face;
	SLONG slid_odd;
//	SLONG ignore_building = NULL;
	SLONG fall_off_flag = 0;

	extern SLONG yomp_speed;
	extern SLONG sprint_speed;
/*
	if(p_thing->Genus.Person->PlayerID)
	{
		dx<<=2;
//		dy<<=1;
		dz<<=2;
	}
*/



	GameCoord new_position;

	ASSERT(abs(dx)>>16<2);
	ASSERT(abs(dz)>>16<2);
	//
	// Valid args?
	//

	if (!p_thing)
	{
		MSG_add("ERROR: move_slidey_thing(NULL!)");

		return 0;
	}

	ASSERT(WITHIN(p_thing, TO_THING(1), TO_THING(MAX_THINGS - 1)));
	ASSERT(p_thing->Class == CLASS_PERSON);

	//
	// The size of the thing and the max size of any furniture..
	//

	#define THING_RADIUS 128

	radius = get_person_radius2(p_thing->Genus.Person->PersonType);

	//
	// we want to get a bit nearer the wall
	//

	if(p_thing->State==STATE_HUG_WALL)
		radius>>=2;

	//
	// The thing's movement vector.
	//

	x1 = p_thing->WorldPos.X;
	my_y1 = p_thing->WorldPos.Y;
	z1 = p_thing->WorldPos.Z;

	x2 = x1 + dx;
	y2 = my_y1 + dy;
	z2 = z1 + dz;

	if(x2<0 || x2>=128<<16 || z2<0 || z2>=128<<16)
	{
		//
		// move off the map, no thanks
		//
//		CONSOLE_text(" thing tried to move off map  CRASH avoided");
		return(0);
	}

#ifndef PSX
	/*
	if(p_thing->Class==CLASS_PERSON)
	{
		if(p_thing->Genus.Person->PlayerID)
		{
extern	void	set_player_visited(UBYTE x,UBYTE z);
			set_player_visited(x2>>16,z2>>16);

		}
	}
	*/
#endif
	//
	// Don't move outside the map.
	//

	if(x2<0||z2<0||(x2>>16)>=MAP_WIDTH||(z2>>16>=MAP_WIDTH))
	{
		return(0);
	}

	//
	// Remember if you've slid against something that isn't a wall or a fence.
	//

	slid_odd = FALSE;

	//
	// Collide with things.
	//

	slid_odd |= collide_against_things(
					 p_thing,
					 radius,
					 x1,  my_y1,  z1,
					&x2, &y2, &z2);
		ASSERT(abs(x2-x1)>>16<2);
		ASSERT(abs(z2-z1)>>16<2);

	if (p_thing->State == STATE_DYING)
	{
		//
		// He's died while colliding with things.
		//

		return 0;
	}

	//
	// Collide with objects.
	//

	slid_odd |= collide_against_objects(
					 p_thing,
					 radius,
					 x1,  my_y1,  z1,
					&x2, &y2, &z2);
		ASSERT(abs(x2-x1)>>16<2);
		ASSERT(abs(z2-z1)>>16<2);

	if (slid_odd)
	{
		p_thing->Genus.Person->SlideOdd += 1;
	}
	else
	{
		p_thing->Genus.Person->SlideOdd = 0;
	}

	//
	// Wierd collision if you are walking on faces.
	//
//	if(p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP)
//		radius+=60;

	if (p_thing->OnFace)
	{

		if(p_thing->OnFace>0)
		{
			if(prim_faces4[p_thing->OnFace].FaceFlags&FACE_FLAG_FIRE_ESCAPE)
			{
				fall_off_flag|=FALL_OFF_FLAG_FIRE_ESCAPE;
			}

		}

		{
			SLONG saflag = 0;
#ifdef	UNUSED_WIRECUTTERS
			if (p_thing->SubState               == SUB_STATE_CRAWLING) {saflag |= SLIDE_ALONG_FLAG_CRAWL;}
#endif
			if (p_thing->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING) {saflag |= SLIDE_ALONG_FLAG_CARRYING;}


			slide_along(
				x1,  my_y1,  z1,
			   &x2, &y2, &z2,
				SLIDE_ALONG_DEFAULT_EXTRA_WALL_HEIGHT,
				radius+20,
				saflag);

		}

		ASSERT(abs(x2-x1)>>16<2);
		ASSERT(abs(z2-z1)>>16<2);

		if (actual_sliding && p_thing->Velocity > yomp_speed)
		{
			p_thing->Velocity = yomp_speed;
			p_thing->Genus.Person->Mode=PERSON_MODE_RUN;
		}

//		if((PAP_2HI(x2>>16,z2>>16).Height<<3)>y2)
//			ASSERT(0);

void slide_along_edges(SLONG face4,SLONG  x1, SLONG  z1,SLONG *x2, SLONG *z2);
void slide_along_edgesr(SLONG face4,SLONG  x1, SLONG  z1,SLONG *x2, SLONG *z2);


		//
		// Should we slide along the edges?
		//

		if(p_thing->State==STATE_CIRCLING || p_thing->SubState==SUB_STATE_STEP_FORWARD|| ((p_thing->SubState==SUB_STATE_WALKING)&&p_thing->Genus.Person->PlayerID)||(fall_off_flag&FALL_OFF_FLAG_FIRE_ESCAPE))
//		if(p_thing->State==STATE_CIRCLING || (p_thing->SubState==SUB_STATE_WALKING&&p_thing->Genus.Person->PlayerID)||(fall_off_flag&FALL_OFF_FLAG_FIRE_ESCAPE))
		{
			//
			// Yes during combat.
			//
/*
			if (p_thing->SubState == SUB_STATE_WALKING_BACKWARDS)
			{
				//
				// But never when walking backwards...
				//
			}
			else
*/
			{
				if(p_thing->OnFace>0)
				{
					slide_along_edges(p_thing->OnFace,x1,z1,&x2,&z2);
		ASSERT(abs(x2-x1)>>16<2);
		ASSERT(abs(z2-z1)>>16<2);
				}
				else
				{
					slide_along_redges(-p_thing->OnFace,x1,z1,&x2,&z2);
		ASSERT(abs(x2-x1)>>16<2);
		ASSERT(abs(z2-z1)>>16<2);

				}
			}
		}


		if (is_thing_on_this_quad(x2>>8, z2>>8, p_thing->OnFace))
		{
			MSG_add(" still on same quad ");

			if(actual_sliding)
			{
				p_thing->Genus.Person->Flags|=FLAG_PERSON_HIT_WALL;
			}

			//
			// No problem, still on the same quad
			//
		}
		else
		{
			MSG_add(" NOT on same quad ");

			//
			// Find the new quad the thing has walked onto.
			//

			new_face = find_face_for_this_pos(x2>>8, y2>>8, z2>>8, &new_y, 0,0); //ignore_building,0)

			if (new_face == NULL)
			{
				//
				// Walked off a face.
				//

				fall_off_flag |= FALL_OFF_FLAG_TRUE;

				if (p_thing->OnFace > 0 && (prim_faces4[p_thing->OnFace].FaceFlags & (FACE_FLAG_WMOVE|FACE_FLAG_PRIM)))
				{
					//
					// Don't do this to crates.
					//

					if (prim_faces4[p_thing->OnFace].FaceFlags & FACE_FLAG_PRIM)
					{
						SLONG ob_index = -prim_faces4[p_thing->OnFace].ThingIndex;

						ASSERT(WITHIN(ob_index, 1, OB_ob_upto - 1));

						if (OB_ob[ob_index].prim == 129)
						{
							//
							// Don't pull up
							//

							goto do_grab;
						}
					}


					fall_off_flag |= FALL_OFF_FLAG_DONT_GRAB;

				  do_grab:;
				}

				p_thing->OnFace = 0;
			}
			else
			if (new_face == GRAB_FLOOR)
			{
				//
				// Stepped off this face and onto the floor.
				//

				p_thing->OnFace = 0;
				fall_off_flag  &= ~FALL_OFF_FLAG_TRUE;
				y2              = new_y << 8;
			}
			else
			{
				if (actual_sliding)
				{
					p_thing->Genus.Person->Flags|=FLAG_PERSON_HIT_WALL;
				}

				if ((y2 >> 8) - new_y > 0x50)
				{
					//
					// A long drop
					//

					fall_off_flag        |= FALL_OFF_FLAG_TRUE;

					if (p_thing->OnFace > 0 && prim_faces4[p_thing->OnFace].FaceFlags & FACE_FLAG_WMOVE)
					{
						fall_off_flag |= FALL_OFF_FLAG_DONT_GRAB;
					}
				}
				else
				{
					//
					// Short drop or small step up.
					//

					y2 = new_y << 8;
				}

				//
				// You're on the new face now.
				//

				p_thing->OnFace = new_face;
			}
		}
	}
	else
	{
		UWORD	person_inside=0,look_for_face=0;
		SLONG	cd=0;

		//
		// Collision if you are not on a face.
		//

		/*

		if(person_inside==0 || look_for_face)
		{
			new_face = find_face_for_this_pos(x2>>8, y2>>8, z2>>8, &new_y, 0,0); //ignore,0);
		}
		else
		{
			new_face=0;
		}

		if (new_face && (new_face!=GRAB_FLOOR))
		{
			ASSERT(abs(new_y - (y2 >> 8)) < 0x100);

			//
			// You're over a new face now.
			//

//			highlight_face(new_face);
//			MSG_add("walk onto face dy diff %d",(y2>>8)-new_y);

			p_thing->OnFace = new_face;

			if( (y2>>8)-new_y > 0x50)
			{
				// long drop
				fall_off_flag        |= FALL_OFF_FLAG_TRUE;
//				fall_off = TRUE;
			}
			else
			{
				// short drop or small step up
				y2=(new_y)<<8;  //+4?
			}
		}
		else

		*/
		{
			SLONG ox2,oy2,oz2;

			ox2=x2;
			oy2=y2;
			oz2=z2;

			{

				SLONG saflag = 0;
#ifdef	UNUSED_WIRECUTTERS
				if (p_thing->SubState               == SUB_STATE_CRAWLING) {saflag |= SLIDE_ALONG_FLAG_CRAWL;}
#endif
				if (p_thing->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING) {saflag |= SLIDE_ALONG_FLAG_CARRYING;}
/* //this code is never true, as jumping people call slide along from projectile_move in darci.cpp,  Commented while testing occurs
				if(p_thing->Genus.Person->PlayerID)
				{
					if (p_thing->SubState               == SUB_STATE_RUNNING_JUMP||
						p_thing->SubState               == SUB_STATE_RUNNING_JUMP_FLY||
						p_thing->SubState               == SUB_STATE_FLYING_KICK||
						p_thing->SubState               == SUB_STATE_FLYING_KICK_FALL)
					{
						saflag |= SLIDE_ALONG_FLAG_JUMPING;
					}
				}
*/

				col = slide_along(
						x1,  my_y1,  z1,
					   &x2, &y2, &z2,
						SLIDE_ALONG_DEFAULT_EXTRA_WALL_HEIGHT,
						radius+20,
						saflag);

				if (actual_sliding && p_thing->Velocity > yomp_speed)
				{
					p_thing->Velocity = yomp_speed;
					p_thing->Genus.Person->Mode=PERSON_MODE_RUN;
				}


				new_face = find_face_for_this_pos(x2>>8, y2>>8, z2>>8, &new_y, 0,0); //ignore,0);

				if (new_face && (new_face!=GRAB_FLOOR))
				{
#ifndef	PSX
//					ASSERT(abs(new_y - (y2 >> 8)) < 0x100);
#endif

					//
					// You're over a new face now.
					//

		//			highlight_face(new_face);
		//			MSG_add("walk onto face dy diff %d",(y2>>8)-new_y);

					p_thing->OnFace = new_face;

					if( (y2>>8)-new_y > 0x50)
					{
						// long drop
						fall_off_flag        |= FALL_OFF_FLAG_TRUE;
		//				fall_off = TRUE;
					}
					else
					{
						// short drop or small step up
						y2=(new_y)<<8;  //+4?
					}
				}


				/*


				if(actual_sliding)
				{
					SLONG	dx,dy,dz;
					SLONG	ax,ay,az;
					SLONG	len,r;

					dx=(ox2-x1)>>8;
					dy=(oy2-my_y1)>>8;
					dz=(oz2-z1)>>8;

					len=Root(dx*dx+dz*dz);
					if(len==0)
						len=1;

					r=last_slide_dist+15;


					radius+=25;

					dx=(dx*radius)/len;
					dz=(dz*radius)/len;

					ax=x2+(dx<<8);
					ay=y2; //+(dy<<8);
					az=z2+(dz<<8);

					//
					// Might have to walk onto this face.
					//

					new_face = find_face_for_this_pos(ax>>8, ay>>8, az>>8, &new_y, ignore_building);

					if(new_face && (new_face!=GRAB_FLOOR))// && ( (y2>>8)-new_y > 64))
					{
						//
						// There's a face there so carry on with your movement
						//
						x2=ox2;
						y2=oy2;
						z2=oz2;
					}
					else
					{

						p_thing->Genus.Person->Flags|=FLAG_PERSON_HIT_WALL;

						if(col)
						{
							//
							// slide along because first slide needs to check previous facets again
							//

							slide_along(
								x1,
								my_y1,
								z1,
							   &x2,
							   &y2,
							   &z2,
							    SLIDE_ALONG_DEFAULT_EXTRA_WALL_HEIGHT,
								radius+20);
						}
					}
				}

				*/
			}
		}
	}

	if (fall_off_flag&FALL_OFF_FLAG_TRUE)
	{
		//
		// Get rid of that nasty fall-through-the-roof bug.
		//

		x2 += dx >> 2;
		y2 += dy >> 2;
		z2 += dz >> 2;
	}


	//
	//	 code removed that
	//

	new_position.X = x2;
	new_position.Y = y2;
	new_position.Z = z2;

	//
	// Rustle up some leaves.
	//

	DIRT_gust(
		p_thing,
		p_thing->WorldPos.X>>8,
		p_thing->WorldPos.Z>>8,
		new_position.X>>8,
		new_position.Z>>8);

	//
	// Swirl the mist.
	//
#ifndef PSX
	MIST_gust(
		p_thing->WorldPos.X>>8,
		p_thing->WorldPos.Z>>8,
		new_position.X>>8,
		new_position.Z>>8);
#endif
	//
	// Hit some barrels.
	//

	BARREL_hit_with_sphere(
		p_thing->WorldPos.X >> 8,
		p_thing->WorldPos.Y >> 8,
		p_thing->WorldPos.Z >> 8,
		0x50);

	//
	// Actually move the person.
	//
/*
	if(p_thing->SubState==SUB_STATE_WALKING &&(fall_off_flag&FALL_OFF_FLAG_TRUE))
		return(1);

	if( (fall_off_flag&(FALL_OFF_FLAG_FIRE_ESCAPE|FALL_OFF_FLAG_TRUE))==(FALL_OFF_FLAG_FIRE_ESCAPE|FALL_OFF_FLAG_TRUE) )
	{
		DebugText("hello\n");
		return(1);
	}

	if(fall_off_flag==(FALL_OFF_FLAG_FIRE_ESCAPE|FALL_OFF_FLAG_TRUE))
	{
		DebugText("hello\n");
		return(1);

	}
*/
	move_thing_on_map(p_thing,&new_position);

	/*

	if(slide_door)
	{
		//
		// pass through door
		//
		if(slide_door<0)
		{
			if(p_thing->Genus.Person->PlayerID)
			{
				INDOORS_INDEX=0;
				INDOORS_DBUILDING=0;
			}
			p_thing->Genus.Person->InsideIndex=0;
			p_thing->Genus.Person->InsideRoom=0;
			p_thing->OnFace=0;
		}
		else
		{
			p_thing->Genus.Person->InsideIndex=slide_door;
			p_thing->Genus.Person->InsideRoom=find_inside_room(slide_door,new_position.X>>16,new_position.Z>>16);
			p_thing->OnFace=0;
			fall_off=0;
			if(p_thing->Genus.Person->PlayerID)
			{
				INDOORS_INDEX=slide_door;
				INDOORS_DBUILDING=inside_storeys[slide_door].Building;
				INDOORS_ROOM=p_thing->Genus.Person->InsideRoom&0xf;

			}
		}
	}

	*/

	if (slide_into_warehouse)
	{
		p_thing->Genus.Person->Flags |= FLAG_PERSON_WAREHOUSE;
		p_thing->Genus.Person->Ware   = dbuildings[slide_into_warehouse].Ware;

		/*

		if (p_thing->Genus.Person->PlayerID && !WARE_in)
		{
			//
			// The game should go into 'warehouse' mode.
			//

			WARE_enter(slide_into_warehouse);
		}

		*/
	}

	if (slide_outof_warehouse)
	{
		p_thing->Genus.Person->Flags &= ~FLAG_PERSON_WAREHOUSE;
		p_thing->Genus.Person->Ware   =  0;

		/*

		if (p_thing->Genus.Person->PlayerID && WARE_in)
		{
			//
			// The game should go out of 'warehouse' mode.
			//

			WARE_exit();
		}

		*/
	}

#ifndef PSX
#ifdef TARGET_DC
	ASSERT(!(p_thing->Genus.Person->InsideIndex));
#else
	//
	// update persons inside room info
	//
	if(p_thing->Genus.Person->InsideIndex)
	{
		p_thing->Genus.Person->InsideRoom=find_inside_room(p_thing->Genus.Person->InsideIndex,new_position.X>>16,new_position.Z>>16);

		if(p_thing->Genus.Person->PlayerID)
			INDOORS_ROOM=p_thing->Genus.Person->InsideRoom;

	}
#endif
#endif

	if(p_thing->SubState!=SUB_STATE_WALKING || p_thing->Genus.Person->PlayerID==0)
	if(p_thing->SubState!=SUB_STATE_SLIPPING)
	if(fall_off_flag & FALL_OFF_FLAG_TRUE)
	{
		MSG_add(" fall off \n");

		{
			SLONG flag = PERSON_DROP_DOWN_KEEP_VEL;

			if (fall_off_flag & FALL_OFF_FLAG_DONT_GRAB)
			{
				flag |= PERSON_DROP_DOWN_OFF_FACE;
			}

			set_person_drop_down(p_thing,flag);
		}
/* //why
		if (p_thing->Velocity>0)
			p_thing->Velocity>>=1;
		else
			p_thing->Velocity>>=2;
*/
	}

	#ifndef NDEBUG

	#if MARKS_EXTRA_DEBUG

	/*

	if (!(p_thing->Genus.Person->Flags & FLAG_PERSON_WAREHOUSE))
	{
		//
		// If you're not in a warehouse then you should never be beneath the (MAVHEIGHT)
		//

		SLONG ground = PAP_calc_map_height_at(
							p_thing->WorldPos.X >> 8,
							p_thing->WorldPos.Z >> 8) << 8;

		if (p_thing->WorldPos.Y < ground - 0x4000)
		{
			//
			// Oh dear!
			//

			ASSERT(0);

			p_thing->WorldPos.Y = ground + 0x1000;

			plant_feet(p_thing);
		}
	}

	*/

	#endif

	#endif

	return 0;
}

//
// Set when check_vector_against_mapsquare() returns TRUE.
//

SLONG los_failure_x;
SLONG los_failure_y;
SLONG los_failure_z;
SLONG los_failure_dfacet;


//
// check_vector_against_mapsquare() remembers a history of facets it has
// checked against.  It does not bother checking the same dfacet twice.
// Be sure to call this function each time you change vectors.
//

SLONG los_done[4];
SLONG los_wptr;

void start_checking_against_a_new_vector(void)
{
	los_wptr = 0;
	los_done[0] = -INFINITY;
	los_done[1] = -INFINITY;
	los_done[2] = -INFINITY;
	los_done[3] = -INFINITY;
}

//
// Returns TRUE if the vector intersects a dfacet above the
// given lo-res mapsquare.  It fills in los_failure_... variables too.
//

struct
{
	SLONG x1;
	SLONG my_y1;
	SLONG z1;

	SLONG x2;
	SLONG y2;
	SLONG z2;

} save_stack;

SLONG check_vector_against_mapsquare(
			SLONG map_x,
			SLONG map_z,
			SLONG los_flags)
{
	SLONG ix;
	SLONG iy;
	SLONG iz;
	SLONG along;
	SLONG f_list;
	SLONG i_dfacet;
	SLONG exit;

	SLONG xmin;
	SLONG xmax;
	SLONG ymin;
	SLONG ymax;
	SLONG zmin;
	SLONG zmax;

	DFacet *df;
	PAP_Lo *pl;

	/*

	if (ControlFlag)
	{
		SLONG mx1 = map_x + 0 << PAP_SHIFT_LO;
		SLONG mz1 = map_z + 0 << PAP_SHIFT_LO;
		SLONG mx2 = map_x + 1 << PAP_SHIFT_LO;
		SLONG mz2 = map_z + 1 << PAP_SHIFT_LO;

		AENG_world_line(
			mx1, 0, mz1, 16, 0xffffffff,
			mx2, 0, mz1, 16, 0xffffffff,
			TRUE);

		AENG_world_line(
			mx2, 0, mz1, 16, 0xffffffff,
			mx2, 0, mz2, 16, 0xffffffff,
			TRUE);

		AENG_world_line(
			mx2, 0, mz2, 16, 0xffffffff,
			mx1, 0, mz2, 16, 0xffffffff,
			TRUE);

		AENG_world_line(
			mx1, 0, mz2, 16, 0xffffffff,
			mx1, 0, mz1, 16, 0xffffffff,
			TRUE);
	}

	*/

	ASSERT(WITHIN(map_x, 0, PAP_SIZE_LO - 1));
	ASSERT(WITHIN(map_z, 0, PAP_SIZE_LO - 1));

	pl = &PAP_2LO(map_x,map_z);

	//
	// For each of the dfacets on this mapsquare.
	//

	f_list = pl->ColVectHead;

	if (f_list == NULL)
	{
		//
		// No dfacets on this mapsquare!
		//

		return FALSE;
	}

	SLONG dlx = save_stack.x2 - save_stack.x1;
	SLONG dly = save_stack.y2 - save_stack.my_y1;
	SLONG dlz = save_stack.z2 - save_stack.z1;

	exit = FALSE;

	while(1)
	{
		ASSERT(WITHIN(f_list, 1, next_facet_link - 1));

		i_dfacet = facet_links[f_list];

		if(i_dfacet==0)
		{
 			ASSERT(0);
			return FALSE;
		}

		if (i_dfacet < 0)
		{
			//
			// The marker for the last facet above this mapsquare.
			//

			exit     =  TRUE;
			i_dfacet = -i_dfacet;
		}

		//
		// Have we done this facet before?
		//

		if (i_dfacet == los_done[0] ||
			i_dfacet == los_done[1] ||
			i_dfacet == los_done[2] ||
			i_dfacet == los_done[3])
		{
			//
			// Done this facet before.
			//
		}
		else
		{
			//
			// Remember we've done this facet.
			//

			los_done[los_wptr] = i_dfacet;
			los_wptr = (los_wptr + 1) & 3;

			ASSERT(WITHIN(i_dfacet, 1, next_dfacet - 1));

			df = &dfacets[i_dfacet];

			if (!(los_flags & LOS_FLAG_IGNORE_SEETHROUGH_FENCE_FLAG) && (df->FacetFlags & FACET_FLAG_SEETHROUGH))
			{
				//
				// A see through fence.
				//
			}
			else
			{
				ASSERT(df->FacetType != STOREY_TYPE_PARTITION);

				//
				// Only do LOS on certain colvects...
				//

				if (df->FacetType == STOREY_TYPE_NORMAL       ||
					df->FacetType == STOREY_TYPE_FENCE        ||
					df->FacetType == STOREY_TYPE_FENCE_FLAT   ||
					df->FacetType == STOREY_TYPE_FENCE_BRICK  ||
					df->FacetType == STOREY_TYPE_OUTSIDE_DOOR ||
					df->FacetType == STOREY_TYPE_DOOR         ||
					df->FacetType == STOREY_TYPE_NORMAL_FOUNDATION)
				{
					if (df->x[0] == df->x[1])
					{
						if ((save_stack.x1 <= (df->x[0] << 8) && save_stack.x2 > (df->x[0] << 8)) ||
							(save_stack.x1 >= (df->x[0] << 8) && save_stack.x2 < (df->x[0] << 8)))
						{
							//
							// Find how far along the line (x1,my_y1,z1)-(x2,y2,z2) crosses
							// the line x = df->X[0].
							//

							along = ((df->x[0] << 8) - save_stack.x1 << 8) / (save_stack.x2 - save_stack.x1);

							//
							// Intersect in y?
							//

							iy = save_stack.my_y1 + (along * dly >> 8);

							ymin = df->Y[0];
							ymax = df->Y[0] + (df->Height * df->BlockHeight << 2);

							if (WITHIN(iy, ymin, ymax))
							{
								//
								// Intersect in z?
								//

								iz = save_stack.z1 + (along * dlz >> 8);

								zmin = df->z[0] << 8;
								zmax = df->z[1] << 8;

								if (zmin > zmax) {SWAP(zmin,zmax);}

								if (WITHIN(iz, zmin, zmax))
								{
									los_failure_x      = df->x[0] << 8;
									los_failure_y      = iy;
									los_failure_z      = iz;
									los_failure_dfacet = i_dfacet;

									return TRUE;
								}
							}
						}
					}
					else
					if (df->z[0] == df->z[1])
					{
						if ((save_stack.z1 <= (df->z[0] << 8) && save_stack.z2 > (df->z[0] << 8)) ||
							(save_stack.z1 >= (df->z[0] << 8) && save_stack.z2 < (df->z[0] << 8)))
						{
							//
							// Find how far along the line (x1,my_y1,z1)-(x2,y2,z2) crosses
							// the line z = df->Z[0].
							//

							along = ((df->z[0] << 8) - save_stack.z1 << 8) / (save_stack.z2 - save_stack.z1);

							//
							// Intersect in y?
							//

							iy = save_stack.my_y1 + (along * dly >> 8);

							ymin = df->Y[0];
							ymax = df->Y[0] + (df->Height * df->BlockHeight << 2);

							if (WITHIN(iy, ymin, ymax))
							{
								//
								// Intersect in x?
								//

								ix = save_stack.x1 + (along * dlx >> 8);

								xmin = df->x[0] << 8;
								xmax = df->x[1] << 8;

								if (xmin > xmax) {SWAP(xmin,xmax);}

								if (WITHIN(ix, xmin, xmax))
								{
									los_failure_x      = ix;
									los_failure_y      = iy;
									los_failure_z      = df->z[0] << 8;
									los_failure_dfacet = i_dfacet;

									return TRUE;
								}
							}
						}
					}
					else
					{
						//
						// No diagonal facets allowed!
						//

						if (df->FacetType == STOREY_TYPE_NORMAL)
							ASSERT(1);
					}
				}
				else
				{
					//
					// Found a strange facet type!
					//

					// iy = iy;
				}
			}
		}

		if (exit)
		{
				return FALSE;
		}

		f_list += 1;
	}
}


//
// Returns TRUE if the given ray intersects any object on the mapsquare.
//

SLONG check_vector_against_mapsquare_objects(
			SLONG map_x,
			SLONG map_z,
			SLONG include_cars)
{
	SLONG y;
	SLONG dx;
	SLONG dz;
	SLONG along;

	{
		OB_Info  *oi;
		PrimInfo *pi;

		for (oi = OB_find(map_x,map_z); oi->prim; oi++)
		{
			if (prim_objects[oi->prim].damage & PRIM_DAMAGE_NOLOS)
			{
				pi = get_prim_info(oi->prim);

				//
				// We must do the y-check ourselves.
				//

				{
					dx = save_stack.x2 - save_stack.x1;
					dz = save_stack.z2 - save_stack.z1;

					if (abs(dx) > abs(dz))
					{
						along = (oi->x - save_stack.x1 << 8) / (dx + 1);
					}
					else
					{
						along = (oi->z - save_stack.z1 << 8) / (dz + 1);
					}

					y = save_stack.my_y1 + ((save_stack.y2 - save_stack.my_y1) * along >> 8);

					if (WITHIN(y, oi->y + pi->miny, oi->y + pi->maxy))
					{
						if (collide_box_with_line(
								oi->x,
								oi->z,
								pi->minx,
								pi->minz,
								pi->maxx,
								pi->maxz,
								oi->yaw,
								save_stack.x1, save_stack.z1,
								save_stack.x2, save_stack.z2))
						{
							return TRUE;
						}
					}
				}
			}
		}
	}

	//
	// Include large cars in the line of sight check.
	//

	{
		SLONG  t_index;
		Thing *p_thing;

		t_index = PAP_2LO(map_x,map_z).MapWho;

		while(t_index)
		{
			p_thing = TO_THING(t_index);

			if (p_thing->Class == CLASS_VEHICLE)
			{
				if (include_cars ||
					p_thing->Genus.Vehicle->Type == VEH_TYPE_VAN       ||
					p_thing->Genus.Vehicle->Type == VEH_TYPE_AMBULANCE ||
					p_thing->Genus.Vehicle->Type == VEH_TYPE_WILDCATVAN)
				{
					//
					// We must do the y-check ourselves.
					//

					{
						dx = save_stack.x2 - save_stack.x1;
						dz = save_stack.z2 - save_stack.z1;

						if (abs(dx) > abs(dz))
						{
							along = ((p_thing->WorldPos.X >> 8) - save_stack.x1 << 8) / (dx + 1);
						}
						else
						{
							along = ((p_thing->WorldPos.Z >> 8) - save_stack.z1 << 8) / (dz + 1);
						}

						y = save_stack.my_y1 + ((save_stack.y2 - save_stack.my_y1) * along >> 8);

						if (y < (p_thing->WorldPos.Y >> 8) + 0xc0)
						{
							dx = SIN(p_thing->Genus.Vehicle->Angle);
							dz = COS(p_thing->Genus.Vehicle->Angle);

							if (collide_box_with_line(
									p_thing->WorldPos.X >> 8,
									p_thing->WorldPos.Z >> 8,
								   -110, -200,
									110,  256,
									p_thing->Genus.Vehicle->Angle,
									save_stack.x1, save_stack.z1,
									save_stack.x2, save_stack.z2))
							{
								return TRUE;
							}
						}
					}
				}
			}

			t_index = p_thing->Child;
		}
	}

	return FALSE;
}



//
// 'local' variables for there_is_a_los...
//

SLONG los_v_x;
SLONG los_v_y;
SLONG los_v_z;

SLONG los_v_dx;
SLONG los_v_dy;
SLONG los_v_dz;

SLONG los_v_mx;
SLONG los_v_mz;

SLONG los_v_end_mx;
SLONG los_v_end_mz;

SLONG there_is_a_los_things(Thing *p_person_a,Thing *p_person_b,SLONG los_flags)
{
	return(there_is_a_los(
						(p_person_a->WorldPos.X >> 8),
						(p_person_a->WorldPos.Y >> 8) + 100,
						(p_person_a->WorldPos.Z >> 8),
						(p_person_b->WorldPos.X >> 8),
						(p_person_b->WorldPos.Y >> 8) + 100,
						(p_person_b->WorldPos.Z >> 8),
						los_flags));
}

SLONG there_is_a_los(
				SLONG x1, SLONG my_y1, SLONG z1,
				SLONG x2, SLONG y2, SLONG z2,
				SLONG los_flags)
{
	SLONG frac;

	SLONG xfrac;
	SLONG zfrac;

	#ifndef NDEBUG
	SLONG count = 0;
	#endif

	save_stack.x1 = x1;
	save_stack.my_y1 = my_y1;
	save_stack.z1 = z1;

	save_stack.x2 = x2;
	save_stack.y2 = y2;
	save_stack.z2 = z2;

	los_v_dx = x2 - x1;
	los_v_dy = y2 - my_y1;
	los_v_dz = z2 - z1;

	los_failure_dfacet = NULL;	// might help

	if (abs(los_v_dx) + abs(los_v_dz) < 16)
	{
		//
		// Looking straight down... practically.
		//

		return TRUE;
	}

	//
	// Check for the vector going under-ground (or through a hill)
	//

	if (!(los_flags & LOS_FLAG_IGNORE_UNDERGROUND_CHECK))
	{
		SLONG i;
		SLONG dist  = QDIST3(abs(los_v_dx),abs(los_v_dy),abs(los_v_dz));
		SLONG steps = (dist >> 9) + 1;
		SLONG cdx   = los_v_dx / steps;
		SLONG cdy   = los_v_dy / steps;
		SLONG cdz   = los_v_dz / steps;

		los_v_x = x1 + cdx;
		los_v_y = my_y1 + cdy;
		los_v_z = z1 + cdz;

		for (i = 0; i < steps; i++)
		{
			if (los_v_y <= PAP_calc_map_height_at(los_v_x,los_v_z))
			{
				los_failure_x      = los_v_x;
				los_failure_y      = los_v_y;
				los_failure_z      = los_v_z;
				los_failure_dfacet = NULL;

				return FALSE;
			}

			los_v_x += cdx;
			los_v_y += cdy;
			los_v_z += cdz;
		}
	}

	//
	// Clear old info.
	//

	start_checking_against_a_new_vector();

	los_v_mx     = x1 >> PAP_SHIFT_LO;
	los_v_mz     = z1 >> PAP_SHIFT_LO;
	los_v_end_mx = x2 >> PAP_SHIFT_LO;
	los_v_end_mz = z2 >> PAP_SHIFT_LO;

	xfrac = x1 & ((1 << PAP_SHIFT_LO) - 1);
	zfrac = z1 & ((1 << PAP_SHIFT_LO) - 1);

	if (abs(los_v_dx) > abs(los_v_dz))
	{
		frac = (los_v_dz << PAP_SHIFT_LO) / los_v_dx;

		if (los_v_dx > 0)
		{
			los_v_z  = z1;
			los_v_z -= frac * xfrac >> PAP_SHIFT_LO;
		}
		else
		{
			los_v_z  = z1;
			los_v_z += frac * ((1 << PAP_SHIFT_LO) - xfrac) >> PAP_SHIFT_LO;
		}

		while(1)
		{
			#ifndef NDEBUG
			ASSERT(count++ < 64);
			#endif

			if (WITHIN(los_v_mx, 0, PAP_SIZE_LO - 1) &&
				WITHIN(los_v_mz, 0, PAP_SIZE_LO - 1))
			{
				if (check_vector_against_mapsquare(
						los_v_mx, los_v_mz,
						los_flags))
				{
					return FALSE;
				}

				if (!(los_flags & LOS_FLAG_IGNORE_PRIMS))
				{
					if (check_vector_against_mapsquare_objects(
							los_v_mx, los_v_mz,
							los_flags & LOS_FLAG_INCLUDE_CARS))
					{
						return FALSE;
					}
				}
			}

			if (los_v_mx == los_v_end_mx &&
				los_v_mz == los_v_end_mz)
			{
				return TRUE;
			}

			//
			// Step in z.
			//

			if (los_v_dx > 0)
			{
				los_v_z += frac;
			}
			else
			{
				los_v_z -= frac;
			}

			if ((los_v_z >> PAP_SHIFT_LO) != los_v_mz)
			{
				//
				// Step up/down in z through another mapsquare.
				//

				los_v_mz = los_v_z >> PAP_SHIFT_LO;

				if (WITHIN(los_v_mx, 0, PAP_SIZE_LO - 1) &&
					WITHIN(los_v_mz, 0, PAP_SIZE_LO - 1))
				{
					if (check_vector_against_mapsquare(
							los_v_mx, los_v_mz,
							los_flags))
					{
						return FALSE;
					}

					if (!(los_flags & LOS_FLAG_IGNORE_PRIMS))
					{
						if (check_vector_against_mapsquare_objects(
								los_v_mx, los_v_mz,
								los_flags & LOS_FLAG_INCLUDE_CARS))
						{
							return FALSE;
						}
					}
				}
			}

			//
			// Step in x.
			//

			if (los_v_dx > 0)
			{
				los_v_mx += 1;

				if (los_v_mx > los_v_end_mx)
				{
					return TRUE;
				}
			}
			else
			{
				los_v_mx -= 1;

				if (los_v_mx < los_v_end_mx)
				{
					return TRUE;
				}
			}
		}
	}
	else
	{
		frac = (los_v_dx << PAP_SHIFT_LO) / los_v_dz;

		if (los_v_dz > 0)
		{
			los_v_x  = x1;
			los_v_x -= frac * zfrac >> PAP_SHIFT_LO;
		}
		else
		{
			los_v_x  = x1;
			los_v_x += frac * ((1 << PAP_SHIFT_LO) - zfrac) >> PAP_SHIFT_LO;
		}

		while(1)
		{
			#ifndef NDEBUG
			ASSERT(count++ < 64);
			#endif

			if (WITHIN(los_v_mx, 0, PAP_SIZE_LO - 1) &&
				WITHIN(los_v_mz, 0, PAP_SIZE_LO - 1))
			{
				if (check_vector_against_mapsquare(
						los_v_mx, los_v_mz,
						los_flags))
				{
					return FALSE;
				}

				if (!(los_flags & LOS_FLAG_IGNORE_PRIMS))
				{
					if (check_vector_against_mapsquare_objects(
							los_v_mx, los_v_mz,
							los_flags & LOS_FLAG_INCLUDE_CARS))
					{
						return FALSE;
					}
				}
			}

			if (los_v_mx == los_v_end_mx &&
				los_v_mz == los_v_end_mz)
			{
				return TRUE;
			}

			//
			// Step in x.
			//

			if (los_v_dz > 0)
			{
				los_v_x += frac;
			}
			else
			{
				los_v_x -= frac;
			}

			if ((los_v_x >> PAP_SHIFT_LO) != los_v_mx)
			{
				//
				// Step up/down in z through another mapsquare.
				//

				los_v_mx = los_v_x >> PAP_SHIFT_LO;

				if (WITHIN(los_v_mx, 0, PAP_SIZE_LO - 1) &&
					WITHIN(los_v_mz, 0, PAP_SIZE_LO - 1))
				{
					if (check_vector_against_mapsquare(
							los_v_mx, los_v_mz,
							los_flags))
					{
						return FALSE;
					}

					if (!(los_flags & LOS_FLAG_IGNORE_PRIMS))
					{
						if (check_vector_against_mapsquare_objects(
								los_v_mx, los_v_mz,
								los_flags & LOS_FLAG_INCLUDE_CARS))
						{
							return FALSE;
						}
					}
				}
			}

			//
			// Step in z.
			//

			if (los_v_dz > 0)
			{
				los_v_mz += 1;

				if (los_v_mz > los_v_end_mz)
				{
					return TRUE;
				}
			}
			else
			{
				los_v_mz -= 1;

				if (los_v_mz < los_v_end_mz)
				{
					return TRUE;
				}
			}
		}
	}
}

UBYTE	last_mav_square_x;
UBYTE	last_mav_square_z;
SBYTE	last_mav_dx;
SBYTE	last_mav_dz;


SLONG there_is_a_los_mav(
				SLONG x1, SLONG my_y1, SLONG z1,
				SLONG x2, SLONG y2, SLONG z2)
{
	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG mx;
	SLONG mz;

	SLONG end_mx;
	SLONG end_mz;

	SLONG sdx;
	SLONG sdz;

	SLONG frac;

	SLONG xfrac;
	SLONG zfrac;

	MAV_Opt *mo;

	#ifndef NDEBUG
	SLONG count = 0;
	#endif

	dx = x2 - x1;
	dz = z2 - z1;

	SLONG adx = abs(dx);
	SLONG adz = abs(dz);

	//
	// Clear old info.
	//

//	start_checking_against_a_new_vector();

	sdx = SIGN(dx);
	sdz = SIGN(dz);

	mx     = x1 >> PAP_SHIFT_HI;
	mz     = z1 >> PAP_SHIFT_HI;
	end_mx = x2 >> PAP_SHIFT_HI;
	end_mz = z2 >> PAP_SHIFT_HI;

	if(mx==end_mx && mz==end_mz)
	{
//		movement is entirely within one mapwho cell
		return(0);
	}

	xfrac = x1 & ((1 << PAP_SHIFT_HI) - 1);
	zfrac = z1 & ((1 << PAP_SHIFT_HI) - 1);

	if (adx > adz)
	{
		frac = (dz << PAP_SHIFT_HI) / dx;

		if (dx > 0)
		{
			z  = z1;
			z -= frac * xfrac >> PAP_SHIFT_HI;
		}
		else
		{
			z  = z1;
			z += frac * ((1 << PAP_SHIFT_HI) - xfrac) >> PAP_SHIFT_HI;
		}

		while(1)
		{
			#ifndef NDEBUG
			ASSERT(count++ < 128);
			#endif

			if (mx == end_mx &&
				mz == end_mz)
			{
				return 0;
			}

			//
			// Step in z.
			//

			if (dx > 0)
			{
				z += frac;
			}
			else
			{
				z -= frac;
			}

			if (WITHIN(mx, 1, PAP_SIZE_HI - 2) &&
				WITHIN(mz, 1, PAP_SIZE_HI - 2))
			{
			}
			else
			{
				//
				// If it goes off near the edge of the map,
				// then assume there is a line of sight
				//

				return 0;
			}

			mo = &MAV_opt[MAV_NAV(mx,mz)];

			last_mav_square_x = mx;
			last_mav_square_z = mz;

			if ((z >> PAP_SHIFT_HI) != mz)
			{
				SLONG direction;

				//
				// Step up/down in z through another mapsquare.
				//

				if( (z>>PAP_SHIFT_HI) > mz)
				{
					direction=MAV_DIR_ZL;
				}
				else
				{
					direction=MAV_DIR_ZS;
				}

				//
				// Was there a wall in the way?
				//

				if(!(mo->opt[direction] & MAV_CAPS_GOTO))
				{
					SLONG	x1,z1,x2,z2;
//					SLONG	y;
					x1=mx;
					x2=mx+(dx>0)?1:-1;
					if(direction==MAV_DIR_ZL)
					{
						z1=mz+1; //>>PAP_SHIFT_HI;
						z2=z1;
					}
					else
					{
						z1=mz; //>>PAP_SHIFT_HI;
						z2=z1;
					}

//					for(y=0;y<256;y+=16)
//						AENG_e_draw_3d_line_col_sorted(x1<<PAP_SHIFT_HI,y,z1<<PAP_SHIFT_HI,x2<<PAP_SHIFT_HI,y,z2<<PAP_SHIFT_HI,128,128,128);

					return(2);
				}

				mz = z >> PAP_SHIFT_HI;
			}

			//
			// Step in x.
			//

			mo = &MAV_opt[MAV_NAV(mx,mz)];

			if (dx > 0)
			{
				if (mx+1 > end_mx)
				{
					return 0;
				}
				if(!(mo->opt[MAV_DIR_XL] & MAV_CAPS_GOTO))
				{
					SLONG	x1,z1,x2,z2;
//					SLONG	y;
					x1=mx+1;
					x2=mx+1;
					z1=mz; //>>PAP_SHIFT_HI;
					z2=z1+1;

//					for(y=0;y<256;y+=16)
//						AENG_e_draw_3d_line_col_sorted(x1<<PAP_SHIFT_HI,y,z1<<PAP_SHIFT_HI,x2<<PAP_SHIFT_HI,y,z2<<PAP_SHIFT_HI,128,128,128);
					return(1);
				}
				mx += 1;

			}
			else
			{

				if (mx-1 < end_mx)
				{
					return 0;
				}
				if(!(mo->opt[MAV_DIR_XS] & MAV_CAPS_GOTO))
				{
					SLONG	x1,z1,x2,z2;
//					SLONG	y;
					x1=mx;
					x2=mx;
					z1=mz;//>>PAP_SHIFT_HI;
					z2=z1+1;

//					for(y=0;y<256;y+=16)
//						AENG_e_draw_3d_line_col_sorted(x1<<PAP_SHIFT_HI,y,z1<<PAP_SHIFT_HI,x2<<PAP_SHIFT_HI,y,z2<<PAP_SHIFT_HI,128,128,128);
					return(1);
				}
				mx -= 1;

			}
		}
	}
	else
	{
		frac = (dx << PAP_SHIFT_HI) / dz;

		if (dz > 0)
		{
			x  = x1;
			x -= frac * zfrac >> PAP_SHIFT_HI;
		}
		else
		{
			x  = x1;
			x += frac * ((1 << PAP_SHIFT_HI) - zfrac) >> PAP_SHIFT_HI;
		}

		while(1)
		{
			#ifndef NDEBUG
			ASSERT(count++ < 128);
			#endif


			if (mx == end_mx &&
				mz == end_mz)
			{
				return 0;
			}

			//
			// Step in x.
			//

			if (dz > 0)
			{
				x += frac;
			}
			else
			{
				x -= frac;
			}

			if (WITHIN(mx, 1, PAP_SIZE_HI - 2) &&
				WITHIN(mz, 1, PAP_SIZE_HI - 2))
			{
			}
			else
			{
				//
				// If it goes off near the edge of the map,
				// then assume there is a line of sight
				//

				return 0;
			}

			last_mav_square_x = mx;
			last_mav_square_z = mz;

			if ((x >> PAP_SHIFT_HI) != mx)
			{
				SLONG	direction;
				//
				// Step up/down in z through another mapsquare.
				//

				if( (x>>PAP_SHIFT_HI) > mx)
				{
					direction=MAV_DIR_XL;
				}
				else
				{
					direction=MAV_DIR_XS;
				}


				//
				// Was there a wall in the way?
				//

				mo = &MAV_opt[MAV_NAV(mx,mz)];

				if(!(mo->opt[direction] & MAV_CAPS_GOTO))
				{
					SLONG	x1,z1,x2,z2;
//					SLONG	y;
					if(direction=MAV_DIR_XL)
					{
						x1=mx+1;
						x2=mx+1;
					}
					else
					{
						x1=mx;
						x2=mx;
					}
					z1=mz; //>>PAP_SHIFT_HI;
					z2=z1+1;

//					for(y=0;y<256;y+=16)
//						AENG_e_draw_3d_line_col_sorted(x1<<PAP_SHIFT_HI,y,z1<<PAP_SHIFT_HI,x2<<PAP_SHIFT_HI,y,z2<<PAP_SHIFT_HI,128,128,128);
					return(1);
				}

				//
				// Step up/down in z through another mapsquare.
				//

				mx = x >> PAP_SHIFT_HI;

			}

			//
			// Step in z.
			//

			mo = &MAV_opt[MAV_NAV(mx,mz)];

			if (dz > 0)
			{
				if (mz+1 > end_mz)
				{
					return 0;
				}
				if(!(mo->opt[MAV_DIR_ZL] & MAV_CAPS_GOTO))
				{
					SLONG	x1,z1,x2,z2;
//					SLONG y;

					x1=mx;
					x2=mx+1;
					z1=(mz+1);//>>PAP_SHIFT_HI;
					z2=z1+1;

//					for(y=0;y<256;y+=16)
//						AENG_e_draw_3d_line_col_sorted(x1<<PAP_SHIFT_HI,y,z1<<PAP_SHIFT_HI,x2<<PAP_SHIFT_HI,y,z2<<PAP_SHIFT_HI,128,128,128);
					return(2);
				}

				mz += 1;

			}
			else
			{
				if (mz-1 < end_mz)
				{
					return 0;
				}
				if(!(mo->opt[MAV_DIR_ZS] & MAV_CAPS_GOTO))
				{
					SLONG	x1,z1,x2,z2;
//					SLONG	y;

					x1=mx;
					x2=mx+1;
					z1=(mz);//>>PAP_SHIFT_HI;
					z2=z1;

//					for(y=0;y<256;y+=16)
//						AENG_e_draw_3d_line_col_sorted(x1<<PAP_SHIFT_HI,y,z1<<PAP_SHIFT_HI,x2<<PAP_SHIFT_HI,y,z2<<PAP_SHIFT_HI,128,128,128);
					return(2);
				}

				mz -= 1;

			}
		}
	}
}

// there_is_a_los_car
//
// cut-and-pasted in true Mucky Foot style!

SLONG there_is_a_los_car(
				SLONG x1, SLONG my_y1, SLONG z1,
				SLONG x2, SLONG y2, SLONG z2)
{
	SLONG x;
	SLONG z;

	SLONG dx;
	SLONG dz;

	SLONG mx;
	SLONG mz;

	SLONG end_mx;
	SLONG end_mz;

	SLONG sdx;
	SLONG sdz;

	SLONG frac;

	SLONG xfrac;
	SLONG zfrac;

	#ifndef NDEBUG
	SLONG count = 0;
	#endif

	dx = x2 - x1;
	dz = z2 - z1;

	SLONG adx = abs(dx);
	SLONG adz = abs(dz);

	//
	// Clear old info.
	//

//	start_checking_against_a_new_vector();

	sdx = SIGN(dx);
	sdz = SIGN(dz);

	mx     = x1 >> PAP_SHIFT_HI;
	mz     = z1 >> PAP_SHIFT_HI;
	end_mx = x2 >> PAP_SHIFT_HI;
	end_mz = z2 >> PAP_SHIFT_HI;

	if(mx==end_mx && mz==end_mz)
	{
//		movement is entirely within one mapwho cell
		return(0);
	}

	xfrac = x1 & ((1 << PAP_SHIFT_HI) - 1);
	zfrac = z1 & ((1 << PAP_SHIFT_HI) - 1);

	if (adx > adz)
	{
		frac = (dz << PAP_SHIFT_HI) / dx;

		if (dx > 0)
		{
			z  = z1;
			z -= frac * xfrac >> PAP_SHIFT_HI;
		}
		else
		{
			z  = z1;
			z += frac * ((1 << PAP_SHIFT_HI) - xfrac) >> PAP_SHIFT_HI;
		}

		while(1)
		{
			#ifndef NDEBUG
			ASSERT(count++ < 128);
			#endif

			if (mx == end_mx &&
				mz == end_mz)
			{
				return 0;
			}

			//
			// Step in z.
			//

			if (dx > 0)
			{
				z += frac;
			}
			else
			{
				z -= frac;
			}

			if (WITHIN(mx, 1, PAP_SIZE_HI - 2) &&
				WITHIN(mz, 1, PAP_SIZE_HI - 2))
			{
			}
			else
			{
				//
				// If it goes off near the edge of the map,
				// then assume there is a line of sight
				//

				return 0;
			}

			if ((z >> PAP_SHIFT_HI) != mz)
			{
				SLONG direction;

				//
				// Step up/down in z through another mapsquare.
				//

				if( (z>>PAP_SHIFT_HI) > mz)
				{
					direction=MAV_DIR_ZL;
				}
				else
				{
					direction=MAV_DIR_ZS;
				}

				//
				// Was there a wall in the way?
				//

				if(!MAV_CAR_GOTO(mx, mz, direction))
				{
					last_mav_square_x = mx;
					last_mav_square_z = mz;
					last_mav_dx = 0;
					last_mav_dz = (direction == MAV_DIR_ZL) ? 1 : -1;
					return(2);
				}

				mz = z >> PAP_SHIFT_HI;
			}

			//
			// Step in x.
			//

			if (dx > 0)
			{
				if (mx+1 > end_mx)
				{
					return 0;
				}
				if(!MAV_CAR_GOTO(mx, mz, MAV_DIR_XL))
				{
					last_mav_square_x = mx;
					last_mav_square_z = mz;
					last_mav_dx = 1;
					last_mav_dz = 0;
					return(1);
				}
				mx += 1;

			}
			else
			{

				if (mx-1 < end_mx)
				{
					return 0;
				}
				if (!MAV_CAR_GOTO(mx, mz, MAV_DIR_XS))
				{
					last_mav_square_x = mx;
					last_mav_square_z = mz;
					last_mav_dx = -1;
					last_mav_dz = 0;
					return(1);
				}
				mx -= 1;
			}
		}
	}
	else
	{
		frac = (dx << PAP_SHIFT_HI) / dz;

		if (dz > 0)
		{
			x  = x1;
			x -= frac * zfrac >> PAP_SHIFT_HI;
		}
		else
		{
			x  = x1;
			x += frac * ((1 << PAP_SHIFT_HI) - zfrac) >> PAP_SHIFT_HI;
		}

		while(1)
		{
			#ifndef NDEBUG
			ASSERT(count++ < 128);
			#endif


			if (mx == end_mx &&
				mz == end_mz)
			{
				return 0;
			}

			//
			// Step in x.
			//

			if (dz > 0)
			{
				x += frac;
			}
			else
			{
				x -= frac;
			}

			if (WITHIN(mx, 1, PAP_SIZE_HI - 2) &&
				WITHIN(mz, 1, PAP_SIZE_HI - 2))
			{
			}
			else
			{
				//
				// If it goes off near the edge of the map,
				// then assume there is a line of sight
				//

				return 0;
			}

			if ((x >> PAP_SHIFT_HI) != mx)
			{
				SLONG	direction;
				//
				// Step up/down in z through another mapsquare.
				//

				if( (x>>PAP_SHIFT_HI) > mx)
				{
					direction=MAV_DIR_XL;
				}
				else
				{
					direction=MAV_DIR_XS;
				}

				//
				// Was there a wall in the way?
				//

				if (!MAV_CAR_GOTO(mx, mz, direction))
				{
					last_mav_square_x = mx;
					last_mav_square_z = mz;
					last_mav_dx = (direction == MAV_DIR_XL) ? 1 : -1;
					last_mav_dz = 0;
					return(1);
				}

				//
				// Step up/down in z through another mapsquare.
				//

				mx = x >> PAP_SHIFT_HI;

			}

			//
			// Step in z.
			//

			if (dz > 0)
			{
				if (mz+1 > end_mz)
				{
					return 0;
				}
				if (!MAV_CAR_GOTO(mx, mz, MAV_DIR_ZL))
				{
					last_mav_square_x = mx;
					last_mav_square_z = mz;
					last_mav_dx = 0;
					last_mav_dz = 1;
					return(2);
				}

				mz += 1;

			}
			else
			{
				if (mz-1 < end_mz)
				{
					return 0;
				}
				last_mav_dz = -1;
				if (!MAV_CAR_GOTO(mx, mz, MAV_DIR_ZS))
				{
					last_mav_square_x = mx;
					last_mav_square_z = mz;
					last_mav_dx = 0;
					last_mav_dz = -1;
					return(2);
				}

				mz -= 1;

			}
		}
	}
}

/*


SLONG there_is_a_los(
				SLONG x1, SLONG my_y1, SLONG z1,
				SLONG x2, SLONG y2, SLONG z2)
{
	SLONG i;

	SLONG x;
	SLONG y;
	SLONG z;

	SLONG mx;
	SLONG mz;

	SLONG last_mx;
	SLONG last_mz;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dlx;
	SLONG dly;
	SLONG dlz;

	SLONG sdlx;
	SLONG sdlz;

	SLONG y_bot;
	SLONG y_top;

	SLONG len;
	SLONG overlen;
	SLONG frac;

	SLONG v_list;
	SLONG i_vect;

	SLONG last[2] = {0,0};

	struct	DFacet *p_facet;

	SLONG ix;
	SLONG iy;
	SLONG iz;

	SLONG along;

	los_failure_x = x1;
	los_failure_y = my_y1;
	los_failure_z = z1;

	dlx = x2 - x1;
	dly = y2 - my_y1;
	dlz = z2 - z1;

	//
	// Step down the line looking for colvects using Bresenhamish
	//

	mx = x1 >> PAP_SHIFT_LO;
	mz = z1 >> PAP_SHIFT_LO;

	if (abs(dlx) > abs(dlz))
	{

	}
	else
	{
	}

	last_mx = INFINITY;
	last_mz = INFINITY;

	for (i = 0; i <= len; i++)
	{
		if (!WITHIN(mx, 0, PAP_SIZE_LO - 1) ||
			 WITHIN(mz, 0, PAP_SIZE_LO - 1))
		{
			break;
		}

		SLONG exit = 0;

		last_mx = mx;
		last_mz = mz;

		v_list = PAP_2LO(mx,mz).ColVectHead;

		if(v_list)
		while(!exit)
		{
			i_vect = facet_links[v_list];
			if(i_vect<0)
			{
				i_vect=-i_vect;
				exit=1;
			}

			//
			// Have we just done this colvect?
			//

			if (i_vect == last[0] ||
				i_vect == last[1])
			{
				//
				// Don't do the colvect again.
				//
			}
			else
			{
				//
				// Incase this colvect is the one we collide with.
				//

				los_failure_colvect = i_vect;

				//
				// The colvect...
				//

				p_facet = &dfacets[i_vect];

				if (collision_storey(p_facet->FacetType))
				{
					SLONG vect_y;

					//
					// Is the 'y' component in range?
					//

					vect_y = get_height_along_facet(x1, z1, p_facet);

					y_bot  = vect_y - UNDERNEATH_A_BIT;
					y_top  = vect_y + BLOCK_SIZE * p_facet->Height;

					if (p_facet->FacetType==STOREY_TYPE_NORMAL_FOUNDATION||
						p_facet->FacetType==STOREY_TYPE_FENCE_FLAT)
					{
						y_bot = -INFINITY;
					}

					if (p_facet->FacetType == STOREY_TYPE_LADDER)
					{
						//
						// Ignore ladder colvects...
						//
					}
					else
					{
						if (p_facet->X[0] == p_facet->X[1])
						{
							if ((x1 <= p_facet->X[0] && x2 > p_facet->X[0]) ||
								(x1 >= p_facet->X[0] && x2 < p_facet->X[0]))
							{
								//
								// Find the point on the line (x1,my_y1,z1)-(x2,y2,z2) that crosses
								// the line x = p_facet->X[0].
								//

								along = (p_facet->X[0] - x1 << 8) / (x2 - x1);

								iy = my_y1 + (along * dly >> 8);
								iz = z1 + (along * dlz >> 8);

								if (p_facet->Z[0] < p_facet->Z[1])
								{
									if (WITHIN(iz, p_facet->Z[0], p_facet->Z[1]) &&
										WITHIN(iy, y_bot, y_top))
									{
										los_failure_x = p_facet->X[0] - dx;
										los_failure_y = iy            - dy;
										los_failure_z = iz            - dz;

										return FALSE;
									}
								}
								else
								{
									if (WITHIN(iz, p_facet->Z[1], p_facet->Z[0]) &&
										WITHIN(iy, y_bot, y_top))
									{
										los_failure_x = p_facet->X[0] - dx;
										los_failure_y = iy            - dy;
										los_failure_z = iz            - dz;

										return FALSE;
									}
								}
							}
						}
						else
						if (p_facet->Z[0] == p_facet->Z[1])
						{
							if ((z1 <= p_facet->Z[0] && z2 > p_facet->Z[0]) ||
								(z1 >= p_facet->Z[0] && z2 < p_facet->Z[0]))
							{
								//
								// Find the point on the line (x1,my_y1,z1)-(x2,y2,z2) that crosses
								// the line x = p_facet->X[0].
								//

								along = (p_facet->Z[0] - z1 << 8) / (z2 - z1);

								ix = x1 + (along * dlx >> 8);
								iy = my_y1 + (along * dly >> 8);

								if (p_facet->X[0] < p_facet->X[1])
								{
									if (WITHIN(ix, p_facet->X[0], p_facet->X[1]) &&
										WITHIN(iy, y_bot, y_top))
									{
										los_failure_x = ix            - dx;
										los_failure_y = iy            - dy;
										los_failure_z = p_facet->Z[0] - dz;

										return FALSE;
									}
								}
								else
								{
									if (WITHIN(ix, p_facet->X[1], p_facet->X[0]) &&
										WITHIN(iy, y_bot, y_top))
									{

										los_failure_x = ix            - dx;
										los_failure_y = iy            - dy;
										los_failure_z = p_facet->Z[0] - dz;

										return FALSE;
									}
								}
							}
						}
						else
						{
							//
							// No diagonals?
							//

							static once_only = FALSE;

							ASSERT(once_only);
					}
				}

				last[0] = last[1];
				last[1] = i_vect;
			}

			v_list++;
		}
	}

	return TRUE;
}

*/

#ifdef	DOG_POO
SLONG collide_box(
		SLONG midx,
		SLONG midy,
		SLONG midz,
		SLONG minx, SLONG minz,
		SLONG maxx, SLONG maxz,
		SLONG yaw)
{
	SLONG mx;
	SLONG mz;

	SLONG x1, x2;
	SLONG z1, z2;

	SLONG useangle;
	SLONG sin_yaw;
	SLONG cos_yaw;

	SLONG matrix[4];

	SLONG v_list;
	SLONG i_vect;

	CollisionVect *p_vect;

	SLONG tx1;
	SLONG tz1;
	SLONG tx2;
	SLONG tz2;

	SLONG rx1;
	SLONG rz1;
	SLONG rx2;
	SLONG rz2;

//	SLONG dx;
//	SLONG dy;
//	SLONG dz;

	SLONG vx;
	SLONG vz;
	SLONG px;
	SLONG pz;

//	SLONG cprod;
	SLONG side;

	SLONG y_bot;
	SLONG y_top;

	UBYTE flag1;
	UBYTE flag2;
	UBYTE flag_and;
//	UBYTE flag_or;

	//
	// The rotation matrix
	//

	useangle  = -yaw;
	useangle &=  2047;

	sin_yaw = SIN(useangle);
	cos_yaw = COS(useangle);

	matrix[0] =  cos_yaw;
	matrix[1] =  sin_yaw;
	matrix[2] = -sin_yaw;
	matrix[3] =  cos_yaw;

	//
	// The bounding box to search for colvects.
	//

	#define PUSH_OUT_A_BIT 64

	x1 = midx + (minx * 362 >> 8) - PUSH_OUT_A_BIT >> ELE_SHIFT;
	z1 = midz + (minz * 362 >> 8) - PUSH_OUT_A_BIT >> ELE_SHIFT;
	x2 = midx + (maxx * 362 >> 8) + PUSH_OUT_A_BIT >> ELE_SHIFT;
	z2 = midz + (maxz * 362 >> 8) + PUSH_OUT_A_BIT >> ELE_SHIFT;

	SATURATE(x1, 0, MAP_WIDTH  - 1);
	SATURATE(z1, 0, MAP_HEIGHT - 1);
	SATURATE(x2, 0, MAP_WIDTH  - 1);
	SATURATE(z2, 0, MAP_HEIGHT - 1);

	for (mx = x1; mx <= x2; mx++)
	for (mz = z1; mz <= z2; mz++)
	{
		v_list = MAP2(mx,mz).ColVectHead;

		while(v_list)
		{
			i_vect =  col_vects_links[v_list].VectIndex;
			p_vect = &col_vects[i_vect];

			if (collision_storey(p_vect->PrimType))
			{
				//
				// Is the 'y' compenent in range?
				//

				y_bot = p_vect->Y[0];
				y_top = p_vect->Y[0] + BLOCK_SIZE * p_vect->PrimExtra;

				if (WITHIN(midy, y_bot, y_top))
				{
					//
					// Find the vector in the space of the bounding box.
					//

					tx1 = p_vect->X[0] - midx;
					tz1 = p_vect->Z[0] - midz;

					tx2 = p_vect->X[1] - midx;
					tz2 = p_vect->Z[1] - midz;

					rx1 = MUL64(tx1, matrix[0]) + MUL64(tz1, matrix[1]);
					rz1 = MUL64(tx1, matrix[2]) + MUL64(tz1, matrix[3]);

					rx2 = MUL64(tx2, matrix[0]) + MUL64(tz2, matrix[1]);
					rz2 = MUL64(tx2, matrix[2]) + MUL64(tz2, matrix[3]);

					//
					// Does the vector (rx1,rz1)-(rx2,rz2) intersect the box?
					//

					flag1 = 0;
					flag2 = 0;

					if (rx1 < minx) {flag1 |= (1 << 0);}
					if (rx2 < minx) {flag2 |= (1 << 0);}
					if (rx1 > maxx) {flag1 |= (1 << 1);}
					if (rx2 > maxx) {flag2 |= (1 << 1);}
					if (rz1 < minz) {flag1 |= (1 << 2);}
					if (rz2 < minz) {flag2 |= (1 << 2);}
					if (rz1 > maxz) {flag1 |= (1 << 3);}
					if (rz2 > maxz) {flag2 |= (1 << 3);}

					flag_and = flag1 & flag2;

					if (flag_and)
					{
						//
						// The colvect does not collide.
						//
					}
					else
					{
						//
						// Is one of the points of the colvect inside the box?
						//

						if ((WITHIN(rx1, minx, maxx) && WITHIN(rz1, minz, maxz)) ||
							(WITHIN(rx2, minx, maxx) && WITHIN(rz2, minz, maxz)))
						{
							//
							// Definitely collided.
							//

							return TRUE;
						}

						//
						// The colvect still might collide. It doesn't collide only
						// if all the points of the square are on the same side of
						// the line.
						//

						vx = rx2 - rx1;
						vz = rz2 - rz1;

						//
						// (minx,minz)
						//

						px = minx - rx1;
						pz = minz - rz1;

						cprod = vx*pz - vz*px;

						if (cprod == 0)
						{
							//
							// One point of the box lies on the line!
							//

							return TRUE;
						}

						side = SIGN(cprod);

						//
						// (maxx,maxz)
						//

						px = maxx - rx1;
						pz = maxz - rz1;

						cprod = vx*pz - vz*px;

						if (SIGN(cprod) != side)
						{
							//
							// Different side of the line. Intersection!
							//

							return TRUE;
						}

						//
						// (minx,maxz)
						//

						px = minx - rx1;
						pz = maxz - rz1;

						cprod = vx*pz - vz*px;

						if (SIGN(cprod) != side)
						{
							//
							// Different side of the line. Intersection!
							//

							return TRUE;
						}


						//
						// (maxx,minz)
						//

						px = maxx - rx1;
						pz = minz - rz1;

						cprod = vx*pz - vz*px;

						if (SIGN(cprod) != side)
						{
							//
							// Different side of the line. Intersection!
							//

							return TRUE;
						}
					}
				}
			}

			v_list = col_vects_links[v_list].Next;
		}
	}

	return FALSE;
}
#endif
SLONG slide_around_circle(
		SLONG cx,
		SLONG cz,
		SLONG cradius,

		SLONG  x1,
		SLONG  z1,
		SLONG *x2,
		SLONG *z2)
{
	SLONG dx;
	SLONG dz;

	SLONG dist;

	dx = *x2 - cx;
	dz = *z2 - cz;

	dist = QDIST2(abs(dx),abs(dz));

	if (dist < cradius)
	{
		if (dist == 0)
		{
			//
			// Oh dear!
			//

			*x2 = x1;
			*z2 = z1;

			return TRUE;
		}
		else
		{
			//
			// Push yourself out from the centre of the circle.
			//

			dx *= (cradius - dist);
			dz *= (cradius - dist);

			dx /=  dist;
			dz /=  dist;

			*x2 += dx;
			*z2 += dz;

			return TRUE;
		}
	}

	return FALSE;
}
SLONG collide_with_circle(
		SLONG cx,
		SLONG cz,
		SLONG cradius,

		SLONG *x2,
		SLONG *z2)
{
	SLONG dx;
	SLONG dz;

	SLONG dist;

	dx = *x2 - cx;
	dz = *z2 - cz;

	dist = QDIST2(abs(dx),abs(dz));

	if (dist < cradius)
	{

		return TRUE;
	}

	return FALSE;
}


SLONG in_my_fov(
		SLONG me_x,  SLONG me_z,
		SLONG him_x, SLONG him_z,
		SLONG lookx,
		SLONG lookz)
{
	SLONG dx    = him_x - me_x;
	SLONG dz    = him_z - me_z;
	SLONG dprod = dx*lookx + dz*lookz;

	if (dprod > 0)
	{
		//
		// In my 180degree FOV
		//

		return TRUE;
	}
	else
	{
		//
		// He is behind me.
		//

		return FALSE;
	}
}
#ifdef	UNUSED
#define MAX_NEARBY_PEOPLE 16

THING_INDEX nearby[MAX_NEARBY_PEOPLE];

THING_INDEX find_nearby_person(
				THING_INDEX me,
				UWORD       person_type_bits,
				SLONG       max_range)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;

	SLONG look_x;
	SLONG look_z;

	SLONG       best_dist;
	THING_INDEX best_thing;

	Thing *p_me = TO_THING(me);
	Thing *p_near;

	SLONG       nearby_upto;

	nearby_upto = THING_find_sphere(
					    p_me->WorldPos.X >> 8,
					    p_me->WorldPos.Y >> 8,
					    p_me->WorldPos.Z >> 8,
						max_range,
						nearby,
						MAX_NEARBY_PEOPLE,
						(1 << CLASS_PERSON));

	//
	// The direction I am looking in.
	//

	look_x = -SIN(p_me->Draw.Tweened->Angle) >> 11;
	look_z = -COS(p_me->Draw.Tweened->Angle) >> 11;

	//
	// Check each person.
	//

	best_dist  = INFINITY;
	best_thing = NULL;

	for (i = 0; i < nearby_upto; i++)
	{
		p_near = TO_THING(nearby[i]);

		ASSERT(p_near->Class == CLASS_PERSON);

		if (p_near != p_me && ((p_near->Genus.Person->PersonType+1) & person_type_bits))
		{
			//
			// Correct person type and not me.
			//

			if (in_my_fov(
					p_me->WorldPos.X,
					p_me->WorldPos.Z,
					p_near->WorldPos.X,
					p_near->WorldPos.Z,
					look_x,
					look_z))
			{
				//
				// He is in my FOV
				//

				if (there_is_a_los(
						(p_me->WorldPos.X >> 8),
						(p_me->WorldPos.Y >> 8) + 0x60,
						(p_me->WorldPos.Z >> 8),
						(p_near->WorldPos.X >> 8),
						(p_near->WorldPos.Y >> 8) + 0x60,
						(p_near->WorldPos.Z >> 8),
						0))
				{
					//
					// In my LOS. How far away is he?
					//

					dx = abs(p_near->WorldPos.X - p_me->WorldPos.X);
					dy = abs(p_near->WorldPos.Y - p_me->WorldPos.Y);
					dz = abs(p_near->WorldPos.Z - p_me->WorldPos.Z);

					dist = dx + dy + dz;

					if (dist < best_dist)
					{
						best_dist  = dist;
						best_thing = nearby[i];
					}
				}
			}
		}
		else
		{
			//
			// Ignore this person.
			//
		}
	}

	return best_thing;
}
#endif
/*
SLONG find_intersected_colvect(
			SLONG x1, SLONG z1,
			SLONG x2, SLONG z2,
			SLONG y)
{
	SLONG mx;
	SLONG mz;

	SLONG y_bot;
	SLONG y_top;

	SLONG index;
	SLONG score;
	SLONG best_ans;
	SLONG best_score;

	SLONG v_list;
	SLONG i_vect;

	CollisionVect *p_vect;

	SLONG mx1 = MIN(x1, x2) - PUSH_OUT_A_BIT >> 8;
	SLONG mz1 = MIN(z2, z2) - PUSH_OUT_A_BIT >> 8;

	SLONG mx2 = MAX(x1, x2) + PUSH_OUT_A_BIT >> 8;
	SLONG mz2 = MAX(z1, z2) + PUSH_OUT_A_BIT >> 8;

	SATURATE(mx1, 0, MAP_WIDTH  - 1);
	SATURATE(mz1, 0, MAP_HEIGHT - 1);

	SATURATE(mx2, 0, MAP_WIDTH  - 1);
	SATURATE(mz2, 0, MAP_HEIGHT - 1);

	best_ans   =  NULL;
	best_score = -INFINITY;

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		v_list = MAP2(mx,mz).ColVectHead;

		while(v_list)
		{
			i_vect =  col_vects_links[v_list].VectIndex;
			p_vect = &col_vects[i_vect];

			//
			// Check the y-range of this colvect.
			//

			y_bot  = p_vect->Y[0] - 128;	// Underneath a bit too...
			y_top  = p_vect->Y[0] + BLOCK_SIZE *  p_vect->PrimExtra;
			y_top -= 16;					// So we can jump off crates!

			if (WITHIN(y, y_bot, y_top))
			{
				//
				// Do we intersect this colvect?
				//

				if (two4_line_intersection(
						x1, z1,
						x2, z2,
						p_vect->X[0],
						p_vect->Z[0],
						p_vect->X[1],
						p_vect->Z[1]))
				{
					//
					// How good is this colvect?
					//

					switch(p_vect->PrimType)
					{
						case STOREY_TYPE_LADDER:
							score = 3;
							break;
						case STOREY_TYPE_FENCE:
						case STOREY_TYPE_FENCE_FLAT:
							score = 2;
							break;
						default:
							score = 1;
							break;
					}

					if (score > best_score)
					{
						best_score = score;
						best_ans   = i_vect;
					}
				}
			}

			v_list = col_vects_links[v_list].Next;
		}
	}

	return best_ans;
}
*/


SLONG calc_map_height_at(SLONG x, SLONG z)
{
	SLONG mx = x >> 8;
	SLONG mz = z >> 8;

	if (!WITHIN(mx, 0, MAP_WIDTH  - 1) ||
		!WITHIN(mz, 0, MAP_HEIGHT - 1))
	{
		return 0;
	}

	if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
	{
		return MAVHEIGHT(mx,mz)<<6; //MAV_height[mx][z] << 6;
	}
	else
	{
		return PAP_calc_height_at(x,z);
	}
}



//
// An collision routine for othogonal walls only.  It collides a thin vector
// against a thick orthogonal sausage shape.
//
#ifndef	PSX
#ifndef TARGET_DC
SLONG collide_against_sausage(
		SLONG sx1, SLONG sz1,
		SLONG sx2, SLONG sz2,
		SLONG swidth,

		SLONG vx1, SLONG vz1,
		SLONG vx2, SLONG vz2,

		SLONG *slide_x,
		SLONG *slide_z)
{
	SLONG dx;
	SLONG dz;
	SLONG dist;

	if (sx1 > sx2) {SWAP(sx1, sx2);}
	if (sz1 > sz2) {SWAP(sz1, sz2);}

	SLONG dsx = sx2 - sx1;
	SLONG dsz = sz2 - sz1;

	//
	// This function only works with orthogonal walls.
	//

	ASSERT(dsx == 0 || dsz == 0);

	if (dsx == 0)
	{
		dx = abs(vx2 - sx1);

		if (dx < swidth)
		{
			if (WITHIN(vz2, sz1, sz2))
			{
				//
				// A rectangle along the middle of the sausage.
				//

				if (vx1 > sx1)
				{
					*slide_x = sx1 + swidth;
					*slide_z = vz2;
				}
				else
				{
					*slide_x = sx1 - swidth;
					*slide_z = vz2;
				}

				return TRUE;
			}
			else
			{
				//
				// Falls through to check the circles at each end of the sausage.
				//
			}
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		ASSERT(dsz == 0);

		dz = abs(vz2 - sz1);

		if (dz < swidth)
		{
			if (WITHIN(vx2, sx1, sx2))
			{
				//
				// A rectangle along the middle of the sausage.
				//

				if (vz1 > sz1)
				{
					*slide_x = vx2;
					*slide_z = sz1 + swidth;
				}
				else
				{
					*slide_x = vx2;
					*slide_z = sz1 - swidth;
				}

				return TRUE;
			}
			else
			{
				//
				// Falls through to check the circles at each end of the sausage.
				//
			}
		}
		else
		{
			return FALSE;
		}
	}

	//
	// Two circles at either end of the sausage.
	//

	dx = vx2 - sx1;
	dz = vz2 - sz1;

	dist = QDIST2(abs(dx),abs(dz)) + 1;

	if (dist < swidth)
	{
		//
		// Normalise (dx,dz)
		//

		dx *= swidth;
		dz *= swidth;

		dx /= dist;
		dz /= dist;

		//
		// Push out to the nearest point on the circle.
		//

		*slide_x = sx1 + dx;
		*slide_z = sz1 + dz;

		return TRUE;
	}

	dx = vx2 - sx2;
	dz = vz2 - sz2;

	dist = QDIST2(abs(dx),abs(dz)) + 1;

	if (dist < swidth)
	{
		//
		// Normalise (dx,dz)
		//

		dx *= swidth;
		dz *= swidth;

		dx /= dist;
		dz /= dist;

		//
		// Push out to the nearest point on the circle.
		//

		*slide_x = sx2 + dx;
		*slide_z = sz2 + dz;

		return TRUE;
	}

	return FALSE;
}
#endif
#endif


#ifndef	PSX
#ifndef TARGET_DC
SLONG slide_around_sausage(
		SLONG sx1,
		SLONG sz1,
		SLONG sx2,
		SLONG sz2,
		SLONG sradius,

		SLONG  x1,
		SLONG  z1,
		SLONG *x2,
		SLONG *z2)
{
	SLONG dist;
	SLONG vec_x;
	SLONG vec_z;
	SLONG on;
	SLONG len;
	SLONG overlen;

	SLONG dx;
	SLONG dz;

	//
	// for !on normal is vector from end point to destination
	//

	signed_dist_to_line_with_normal_mark(
		sx1, sz1,
		sx2, sz2,
	   *x2,
	   *z2,
	   &dist,
	   &vec_x,
	   &vec_z,
	   &on);

	dist = abs(dist);

	if (dist > sradius)
	{
		//
		// No collision.
		//

		return FALSE;
	}

	//
	// Normalise (vec_x, vec_z).
	//

	len     = QDIST2(abs(vec_x),abs(vec_z)) + 1;
	overlen = 0x10000 / len;
	vec_x  *= overlen;
	vec_z  *= overlen;

	//
	// Make sure (*x2,*z2) is 'radius' distance from the colvect.
	//

	dx = MUL64(sradius - dist, vec_x);
	dz = MUL64(sradius - dist, vec_z);

   *x2 += dx;
   *z2 += dz;

    return TRUE;
}
#endif
#endif

//
// Returns TRUE if Darci can't move from from one square to the other and
// there should be a collision vect along the join.
//

SLONG stop_movement_between(
		SLONG mx1,
		SLONG mz1,
		SLONG mx2,
		SLONG mz2)
{
/*
	SLONG midx1 = (mx1 << 8) + 0x80;
	SLONG midz1 = (mz1 << 8) + 0x80;

	SLONG midx2 = (mx2 << 8) + 0x80;
	SLONG midz2 = (mz2 << 8) + 0x80;

	ASSERT(WITHIN(mx1, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(mz1, 0, PAP_SIZE_HI - 1));

	ASSERT(WITHIN(mx2, 0, PAP_SIZE_HI - 1));
	ASSERT(WITHIN(mz2, 0, PAP_SIZE_HI - 1));

	SLONG height1 = PAP_calc_height_at(midx1, midz1);
	SLONG height2 = PAP_calc_height_at(midx2, midz2);
*/
	PAP_Hi *ph1 = &PAP_2HI(mx1,mz1);
	PAP_Hi *ph2 = &PAP_2HI(mx2,mz2);

	SLONG normal = TRUE;

	if ((ph1->Flags & (PAP_FLAG_WATER | PAP_FLAG_HIDDEN)) ||
		(ph2->Flags & (PAP_FLAG_WATER | PAP_FLAG_HIDDEN)))
	{
		normal = FALSE;
	}

	/*

	This is a bad way to do this... facets are meant to be for walls.

	if (normal && abs(height1 - height2) > 0x50)
	{
		//
		// Too much of a height difference.
		//

		return TRUE;
	}

	*/

	if ((ph1->Flags & PAP_FLAG_WATER) && !(ph2->Flags & PAP_FLAG_WATER))
	{
		//
		// The edge of some water.
		//

		return TRUE;
	}

	/*

	if ((ph1->Flags & PAP_FLAG_SEWER_SQUARE) && !(ph2->Flags & PAP_FLAG_SEWER_SQUARE))
	{
		//
		// The edge of a sewer entrance.
		//

		return TRUE;
	}

	*/

	return FALSE;
}



//
// Creates a new JUST_COLLISION facet suitable for a water's edge and
// the edge of a sewer entrance.
//
#ifndef PSX
#ifndef TARGET_DC
void create_just_collision_facet(
		SLONG x1,
		SLONG z1,
		SLONG x2,
		SLONG z2)
{
	SLONG dx = x2 - x1;
	SLONG dz = z2 - z1;

	SLONG hx1 = x1 + SIGN(dx) + SIGN(dz);
	SLONG hz1 = z1 + SIGN(dz) - SIGN(dx);

	SLONG hx2 = x2 - SIGN(dx) + SIGN(dz);
	SLONG hz2 = z2 - SIGN(dz) - SIGN(dx);

	SLONG my_y1 = PAP_calc_height_at(hx1, hz1);
	SLONG y2 = PAP_calc_height_at(hx2, hz2);

	if (next_dfacet >= MAX_DFACETS)
	{
		//
		// No more dfacets!
		//

		return;
	}

	SLONG dfacet = next_dfacet++;

	DFacet *df = &dfacets[dfacet];

	df->FacetType   = STOREY_TYPE_JUST_COLLISION;
	df->Height      = 4;
	df->BlockHeight = 16;
	df->x[0]        = x1 >> 8;
	df->Y[0]        = my_y1;
	df->z[0]        = z1 >> 8;
	df->x[1]        = x2 >> 8;
	df->Y[1]        = y2;
	df->z[1]        = z2 >> 8;
	df->FacetFlags  = 0;

	//
	// All this is undefined.
	//

	df->StyleIndex = 0;
	df->Building   = 0;
	df->DStorey    = 0;
	df->FHeight    = 0;
	df->Dfcache    = 0;
	df->Counter[0] = 0;
	df->Counter[1] = 0;

	//
	// Put the facet on the mapwho.
	//

	add_facet_to_map(dfacet);
}


void insert_collision_facets()
{
	SLONG x;
	SLONG z;

	SLONG fx1;
	SLONG fz1;

	SLONG fx2;
	SLONG fz2;

	SLONG fstart;
	SLONG blocked;

	//
	// Lines of constant x.
	//

	for (x = 1; x < PAP_SIZE_HI - 1; x++)
	{
		//
		// To a lower x.
		//

		fstart = FALSE;

		for (z = 1; z < PAP_SIZE_HI; z++)
		{
			if (z == PAP_SIZE_HI - 1)
			{
				blocked = FALSE;
			}
			else
			{
				blocked = stop_movement_between(x, z, x - 1, z);
			}

			if (blocked)
			{
				if (!fstart)
				{
					fx1 = x << PAP_SHIFT_HI;
					fz1 = z << PAP_SHIFT_HI;

					fstart = TRUE;
				}
			}
			else
			{
				if (fstart)
				{
					fx2 = x << PAP_SHIFT_HI;
					fz2 = z << PAP_SHIFT_HI;

					fstart = FALSE;

					//
					// Create a JUST_COLLISION facet.
					//

					create_just_collision_facet(
						fx1, fz1,
						fx2, fz2);
				}
			}
		}

		//
		// To a higher x.
		//

		fstart = FALSE;

		for (z = 1; z < PAP_SIZE_HI; z++)
		{
			if (z == PAP_SIZE_HI - 1)
			{
				blocked = FALSE;
			}
			else
			{
				blocked = stop_movement_between(x, z, x + 1, z);
			}

			if (blocked)
			{
				if (!fstart)
				{
					fx1 = x + 1 << PAP_SHIFT_HI;
					fz1 = z     << PAP_SHIFT_HI;

					fstart = TRUE;
				}
			}
			else
			{
				if (fstart)
				{
					fx2 = x + 1 << PAP_SHIFT_HI;
					fz2 = z     << PAP_SHIFT_HI;

					fstart = FALSE;

					//
					// Create a JUST_COLLISION facet.
					//

					create_just_collision_facet(
						fx2, fz2,
						fx1, fz1);
				}
			}
		}
	}


	//
	// Lines of constant z.
	//

	for (z = 1; z < PAP_SIZE_HI - 1; z++)
	{
		//
		// To a lower z.
		//

		fstart = FALSE;

		for (x = 1; x < PAP_SIZE_HI; x++)
		{
			if (x == PAP_SIZE_HI - 1)
			{
				blocked = FALSE;
			}
			else
			{
				blocked = stop_movement_between(x, z, x, z - 1);
			}

			if (blocked)
			{
				if (!fstart)
				{
					fx1 = x << PAP_SHIFT_HI;
					fz1 = z << PAP_SHIFT_HI;

					fstart = TRUE;
				}
			}
			else
			{
				if (fstart)
				{
					fx2 = x << PAP_SHIFT_HI;
					fz2 = z << PAP_SHIFT_HI;

					fstart = FALSE;

					//
					// Create a JUST_COLLISION facet.
					//

					create_just_collision_facet(
						fx2, fz2,
						fx1, fz1);
				}
			}
		}

		//
		// To a higher z.
		//

		fstart = FALSE;

		for (x = 1; x < PAP_SIZE_HI; x++)
		{
			if (x == PAP_SIZE_HI - 1)
			{
				blocked = FALSE;
			}
			else
			{
				blocked = stop_movement_between(x, z, x, z + 1);
			}

			if (blocked)
			{
				if (!fstart)
				{
					fx1 = x    << PAP_SHIFT_HI;
					fz1 = z + 1 << PAP_SHIFT_HI;

					fstart = TRUE;
				}
			}
			else
			{
				if (fstart)
				{
					fx2 = x     << PAP_SHIFT_HI;
					fz2 = z + 1 << PAP_SHIFT_HI;

					fstart = FALSE;

					//
					// Create a JUST_COLLISION facet.
					//

					create_just_collision_facet(
						fx1, fz1,
						fx2, fz2);
				}
			}
		}
	}
}
#endif
#endif

//
// should be locals but stack overflow
//
SLONG	tried;
SLONG	used_this_go;
SLONG	failed;

SLONG slide_around_box(
		SLONG box_mid_x,
		SLONG box_mid_z,
		SLONG box_min_x,
		SLONG box_min_z,
		SLONG box_max_x,
		SLONG box_max_z,
		SLONG box_yaw,
		SLONG radius,

		SLONG  x1,
		SLONG  z1,
		SLONG *x2,
		SLONG *z2)
{
	SLONG tx1;
	SLONG tz1;
	SLONG tx2;
	SLONG tz2;

	SLONG rx1;
	SLONG rz1;
	SLONG rx2;
	SLONG rz2;

	SLONG dminx;
	SLONG dminz;
	SLONG dmaxx;
	SLONG dmaxz;

	SLONG minx;
	SLONG minz;

	SLONG maxx;
	SLONG maxz;

	SLONG best;
	SLONG best_x;
	SLONG best_z;

	tried=0;
	used_this_go=0;
	failed=1;

#ifndef PSX
	SLONG matrix[4];
	SLONG useangle;

	SLONG sin_yaw;
	SLONG cos_yaw;

	useangle  = -box_yaw;
	useangle &=  2047;

	sin_yaw = SIN(useangle);
	cos_yaw = COS(useangle);

	matrix[0] =  cos_yaw;
	matrix[1] =  sin_yaw;
	matrix[2] = -sin_yaw;
	matrix[3] =  cos_yaw;

	//
	// Rotate the positions.
	//

	tx1 =  x1 - box_mid_x;
	tz1 =  z1 - box_mid_z;

	tx2 = *x2 - box_mid_x;
	tz2 = *z2 - box_mid_z;

	rx1 = MUL64(tx1, matrix[0]) + MUL64(tz1, matrix[1]);
	rz1 = MUL64(tx1, matrix[2]) + MUL64(tz1, matrix[3]);

	rx2 = MUL64(tx2, matrix[0]) + MUL64(tz2, matrix[1]);
	rz2 = MUL64(tx2, matrix[2]) + MUL64(tz2, matrix[3]);
#else
	SLONG useangle;

	useangle  = -box_yaw;
	useangle &=  2047;

	tx1 =  x1 - box_mid_x;
	tz1 =  z1 - box_mid_z;

	tx2 = *x2 - box_mid_x;
	tz2 = *z2 - box_mid_z;

	rx1 = MUL64(tx1, COS(useangle)) + MUL64(tz1, SIN(useangle));
	rz1 = MUL64(tx1, -SIN(useangle)) + MUL64(tz1, COS(useangle));

	rx2 = MUL64(tx2, COS(useangle)) + MUL64(tz2, SIN(useangle));
	rz2 = MUL64(tx2, -SIN(useangle)) + MUL64(tz2, COS(useangle));
#endif
	//
	// The bounding box.
	//

	minx = box_min_x - radius;
	minz = box_min_z - radius;

	maxx = box_max_x + radius;
	maxz = box_max_z + radius;

	//
	// Do we collide?
	//

	if (rx2 > maxx ||
		rx2 < minx ||
		rz2 > maxz ||
		rz2 < minz)
	{
		//
		// We don't collide with the bounding box.
		//

		return FALSE;
	}

	//
	// Slide to the nearest point to the edge of the box.
	//

	dminx = rx2 - minx;
	dmaxx = maxx - rx2;

	dminz = rz2 - minz;
	dmaxz = maxz - rz2;

#ifndef	PSX
	SWAP(matrix[1], matrix[2]);
#endif

	while(failed)
	{
		if(tried&2)
		{
			best=0x7fffffff;
		}
		else
		{
			best   = dminx;
			best_x = minx - 1;
			best_z = rz2;
			used_this_go=1;
		}

		if(!(tried&4))
		if (dmaxx < best)
		{
			{
				best   = dmaxx;
				best_x = maxx - 1;
				best_z = rz2;
				used_this_go=2;
			}
		}

		if(!(tried&8))
		if (dminz < best)
		{
			{
				best   = dminz;
				best_x = rx2;
				best_z = minz - 1;
				used_this_go=3;
			}
		}

		if(!(tried&16))
		if (dmaxz < best)
		{
			{
				best   = dmaxz;
				best_x = rx2;
				best_z = maxz + 1;
				used_this_go=4;
			}
		}

		//
		// We have to un-rotate the points. The inverse of the
		// matrix is its transpose.
		//

	#ifndef PSX

		*x2 = MUL64(best_x, matrix[0]) + MUL64(best_z, matrix[1]);
		*z2 = MUL64(best_x, matrix[2]) + MUL64(best_z, matrix[3]);
	#else
		*x2 = MUL64(best_x, COS(useangle)) + MUL64(best_z, -SIN(useangle));
		*z2 = MUL64(best_x, SIN(useangle)) + MUL64(best_z, COS(useangle));
	#endif

		*x2 += box_mid_x;
		*z2 += box_mid_z;

		if(PAP_2HI((*x2)>>8,(*z2)>>8).Flags&PAP_FLAG_NOGO)
		{
			tried|=1<<used_this_go;
			failed=1;
		}
		else
		{
			failed=0;
		}
	}

	return TRUE;
}

inline	SLONG slide_around_box_lowstack(
		SLONG box_mid_x,
		SLONG box_mid_z,
		SLONG box_min_x,
		SLONG box_min_z,
		SLONG box_max_x,
		SLONG box_max_z,
		SLONG box_yaw,
		SLONG radius,

		SLONG  x1,
		SLONG  z1,
		SLONG *x2,
		SLONG *z2)
{
	SLONG tx1;
	SLONG tz1;
	SLONG tx2;
	SLONG tz2;

	SLONG rx1;
	SLONG rz1;
	SLONG rx2;
	SLONG rz2;

	SLONG dminx;
	SLONG dminz;
	SLONG dmaxx;
	SLONG dmaxz;

	SLONG minx;
	SLONG minz;

	SLONG maxx;
	SLONG maxz;

	SLONG best;
	SLONG best_x;
	SLONG best_z;

#ifndef PSX
	SLONG matrix[4];
	SLONG useangle;

	SLONG sin_yaw;
	SLONG cos_yaw;

	useangle  = -box_yaw;
	useangle &=  2047;

	sin_yaw = SIN(useangle);
	cos_yaw = COS(useangle);

	matrix[0] =  cos_yaw;
	matrix[1] =  sin_yaw;
	matrix[2] = -sin_yaw;
	matrix[3] =  cos_yaw;

	//
	// Rotate the positions.
	//

	tx1 =  x1 - box_mid_x;
	tz1 =  z1 - box_mid_z;

	tx2 = *x2 - box_mid_x;
	tz2 = *z2 - box_mid_z;

	rx1 = MUL64(tx1, matrix[0]) + MUL64(tz1, matrix[1]);
	rz1 = MUL64(tx1, matrix[2]) + MUL64(tz1, matrix[3]);

	rx2 = MUL64(tx2, matrix[0]) + MUL64(tz2, matrix[1]);
	rz2 = MUL64(tx2, matrix[2]) + MUL64(tz2, matrix[3]);
#else
	SLONG useangle;

	useangle  = -box_yaw;
	useangle &=  2047;

	tx1 =  x1 - box_mid_x;
	tz1 =  z1 - box_mid_z;

	tx2 = *x2 - box_mid_x;
	tz2 = *z2 - box_mid_z;

	rx1 = MUL64(tx1, COS(useangle)) + MUL64(tz1, SIN(useangle));
	rz1 = MUL64(tx1, -SIN(useangle)) + MUL64(tz1, COS(useangle));

	rx2 = MUL64(tx2, COS(useangle)) + MUL64(tz2, SIN(useangle));
	rz2 = MUL64(tx2, -SIN(useangle)) + MUL64(tz2, COS(useangle));
#endif
	//
	// The bounding box.
	//

	minx = box_min_x - radius;
	minz = box_min_z - radius;

	maxx = box_max_x + radius;
	maxz = box_max_z + radius;

	//
	// Do we collide?
	//

	if (rx2 > maxx ||
		rx2 < minx ||
		rz2 > maxz ||
		rz2 < minz)
	{
		//
		// We don't collide with the bounding box.
		//

		return FALSE;
	}

	//
	// Slide to the nearest point to the edge of the box.
	//

	dminx = rx2 - minx;
	dmaxx = maxx - rx2;

	dminz = rz2 - minz;
	dmaxz = maxz - rz2;

	best   = dminx;
	best_x = minx - 1;
	best_z = rz2;

	if (dmaxx < best)
	{
		best   = dmaxx;
		best_x = maxx - 1;
		best_z = rz2;
	}

	if (dminz < best)
	{
		best   = dminz;
		best_x = rx2;
		best_z = minz - 1;
	}

	if (dmaxz < best)
	{
		best   = dmaxz;
		best_x = rx2;
		best_z = maxz + 1;
	}

	//
	// We have to un-rotate the points. The inverse of the
	// matrix is its transpose.
	//

#ifndef PSX
	SWAP(matrix[1], matrix[2]);

	*x2 = MUL64(best_x, matrix[0]) + MUL64(best_z, matrix[1]);
	*z2 = MUL64(best_x, matrix[2]) + MUL64(best_z, matrix[3]);
#else
	*x2 = MUL64(best_x, COS(useangle)) + MUL64(best_z, -SIN(useangle));
	*z2 = MUL64(best_x, SIN(useangle)) + MUL64(best_z, COS(useangle));
#endif

	*x2 += box_mid_x;
	*z2 += box_mid_z;

	return TRUE;
}

SLONG collide_box_with_line(
		SLONG midx,
		SLONG midz,
		SLONG minx, SLONG minz,
		SLONG maxx, SLONG maxz,
		SLONG yaw,
		SLONG lx1,
		SLONG lz1,
		SLONG lx2,
		SLONG lz2)
{
	SLONG tx1;
	SLONG tz1;
	SLONG tx2;
	SLONG tz2;

	SLONG rx1;
	SLONG rz1;
	SLONG rx2;
	SLONG rz2;

	SLONG ix;
	SLONG iz;

	SLONG matrix[4];
	SLONG useangle;

	SLONG sin_yaw;
	SLONG cos_yaw;

	useangle  = -yaw;
	useangle &=  2047;

	sin_yaw = SIN(useangle);
	cos_yaw = COS(useangle);

	matrix[0] =  cos_yaw;
	matrix[1] =  sin_yaw;
	matrix[2] = -sin_yaw;
	matrix[3] =  cos_yaw;

	//
	// Rotate the positions.
	//

	tx1 = lx1 - midx;
	tz1 = lz1 - midz;

	tx2 = lx2 - midx;
	tz2 = lz2 - midz;

	rx1 = MUL64(tx1, matrix[0]) + MUL64(tz1, matrix[1]);
	rz1 = MUL64(tx1, matrix[2]) + MUL64(tz1, matrix[3]);

	rx2 = MUL64(tx2, matrix[0]) + MUL64(tz2, matrix[1]);
	rz2 = MUL64(tx2, matrix[2]) + MUL64(tz2, matrix[3]);

	#define COL_CLIP_XS (1 << 0)
	#define COL_CLIP_XL (1 << 1)
	#define COL_CLIP_ZS (1 << 2)
	#define COL_CLIP_ZL (1 << 3)

	UBYTE clip1 = 0;
	UBYTE clip2 = 0;

	if (rx1 < minx) {clip1 |= COL_CLIP_XS;}
	if (rx1 > maxx) {clip1 |= COL_CLIP_XL;}
	if (rz1 < minz) {clip1 |= COL_CLIP_ZS;}
	if (rz1 > maxz) {clip1 |= COL_CLIP_ZL;}

	if (clip1 == 0)
	{
		//
		// The line definitely intersects the box.
		//

		return TRUE;
	}

	if (rx2 < minx) {clip2 |= COL_CLIP_XS;}
	if (rx2 > maxx) {clip2 |= COL_CLIP_XL;}
	if (rz2 < minz) {clip2 |= COL_CLIP_ZS;}
	if (rz2 > maxz) {clip2 |= COL_CLIP_ZL;}

	if (clip2 == 0)
	{
		//
		// The line definitely intersects the box.
		//

		return TRUE;
	}

	UBYTE clip_and = clip1 & clip2;

	if (clip_and)
	{
		//
		// The line does not intersect the box.
		//

		return FALSE;
	}

	//
	// We are going to have to arse around.
	//

	UBYTE clip_xor = clip1 ^ clip2;

	if (clip_xor & COL_CLIP_XS)
	{
		iz = rz1 + (rz2 - rz1) * (minx - rx1) / (rx2 - rx1);

		if (WITHIN(iz, minz, maxz))
		{
			return TRUE;
		}
	}

	if (clip_xor & COL_CLIP_XL)
	{
		iz = rz1 + (rz2 - rz1) * (maxx - rx1) / (rx2 - rx1);

		if (WITHIN(iz, minz, maxz))
		{
			return TRUE;
		}
	}

	if (clip_xor & COL_CLIP_ZS)
	{
		ix = rx1 + (rx2 - rx1) * (minz - rz1) / (rz2 - rz1);

		if (WITHIN(ix, minx, maxx))
		{
			return TRUE;
		}
	}

	if (clip_xor & COL_CLIP_ZL)
	{
		ix = rx1 + (rx2 - rx1) * (maxz - rz1) / (rz2 - rz1);

		if (WITHIN(iz, minx, maxx))
		{
			return TRUE;
		}
	}

	//
	// It dosn't collide after all.
	//

	return FALSE;
}


void create_shockwave(
		SLONG  x,
		SLONG  y,
		SLONG  z,
		SLONG  radius,
		SLONG  maxdamage,
		Thing *p_aggressor,ULONG just_people)
{
	SLONG i;
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG hitpoints;

	#define SHOCKWAVE_FIND 16

	UWORD found[SHOCKWAVE_FIND];
	SLONG num;

	Thing *p_found;
	ULONG	classes;

	//
	// Shake the camera.
	//

	FC_explosion(x,y,z, maxdamage);

	//
	// Find all people within the shockwave.
	//


	if(just_people)
	{
		classes=1<<CLASS_PERSON;
	}
	else
	{
		classes=(1 << CLASS_PERSON) | (1 << CLASS_SPECIAL) | (1 << CLASS_VEHICLE) | (1 << CLASS_BAT);
	}
	num = THING_find_sphere(x, y, z, radius, found, SHOCKWAVE_FIND, classes);

	for (i = 0; i < num; i++)
	{
		p_found = TO_THING(found[i]);

		//
		// How far is this thing from the epicentre?
		//

		dx = abs((p_found->WorldPos.X >> 8) - x);
		dy = abs((p_found->WorldPos.Y >> 8) - y);
		dz = abs((p_found->WorldPos.Z >> 8) - z);

		dist = QDIST3(dx,dy,dz);

		extern SLONG is_person_ko(Thing *p_person);

		{
			if (p_found->Class == CLASS_PERSON && !is_person_ko(p_found))
			{
				//
				// What damage does this person recieve?
				//
#ifdef	VERSION_DEMO
				if (!(p_found->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL))
#endif
				{
					hitpoints = maxdamage * (radius - dist) / radius;

					if (p_found->State == STATE_JUMPING || (p_found->State == STATE_MOVEING && p_found->SubState == SUB_STATE_FLIPING))
					{
						hitpoints -= hitpoints >> 1;
						hitpoints += 1;
					}

					if (hitpoints > 30)
					{
						//
						// Knock this person down.
						//

						knock_person_down(
							p_found,
							hitpoints,
							x,
							z,
							p_aggressor);
					}
					else
					{
						if (p_found->State == STATE_JUMPING || (p_found->State == STATE_MOVEING && p_found->SubState == SUB_STATE_FLIPING))
						{
							//
							// Jumping people don't recoil from a shockwave.
							//
						}
						else
						{
							//
							// Recoil the person.
							//

							set_face_pos(
								p_found,
								x,
								z);

							set_person_recoil(
								p_found,
								ANIM_HIT_FRONT_MID,
								0);
						}
					}
				}
			}
			else
			if (p_found->Class == CLASS_SPECIAL)
			{
				if (p_found->Genus.Special->SpecialType == SPECIAL_MINE)
				{
					if (dist < 0x120)
					{
						//
						// Mines don't explode immediately! Their ammo field is a counter down to
						// exploding.
						//

						p_found->Genus.Special->ammo = (Random() & 0x7) + 5;
					}
				}
			}
			else
			if (p_found->Class == CLASS_VEHICLE)
			{
				dist -= 0x100;

				if (dist <= 0)
				{
					hitpoints = maxdamage;
				}
				else
				{
					hitpoints = maxdamage * (radius - dist) / radius;
				}

				hitpoints <<= 1;

				extern void VEH_reduce_health(
								Thing *p_car,
								Thing *p_person,
								SLONG  damage);

				VEH_reduce_health(
					p_found,
					p_aggressor,
					hitpoints>>1);
			}
			else
			if (p_found->Class == CLASS_BAT)
			{
				//
				// Balrogs are damaged by shockwaves.
				//

				// if (p_found->Genus.Bat->type == BAT_TYPE_BALROG)	Only hurt Balrogs?

				{
					if (p_aggressor == p_found)
					{
						//
						// If the Balrog caused the explosion then it doesn't hurt him.
						//
					}
					else
					{
						hitpoints = maxdamage * (radius - dist + 0x80) / radius;

						if (p_found->Genus.Bat->type == BAT_TYPE_BALROG)
						{
							//
							// Balrogs are so hard that we'd better hurt them more here!
							//

							hitpoints <<= 2;
						}

						BAT_apply_hit(
							p_found,
							p_aggressor,
							hitpoints);
					}
				}
			}
		}
	}

	//
	// Find obs within the shockwave.
	//
	if(!just_people)
	{
		UBYTE mx;
		UBYTE mz;

		SWORD mx1;
		SWORD mz1;
		SWORD mx2;
		SWORD mz2;

		SLONG dx;
		SLONG dy;
		SLONG dz;
		SLONG dist;

		OB_Info *oi;

		mx1 = x - radius >> PAP_SHIFT_LO;
		mz1 = z - radius >> PAP_SHIFT_LO;
		mx2 = x + radius >> PAP_SHIFT_LO;
		mz2 = z + radius >> PAP_SHIFT_LO;

		SATURATE(mx1, 0, PAP_SIZE_LO - 1);
		SATURATE(mz1, 0, PAP_SIZE_LO - 1);
		SATURATE(mx2, 0, PAP_SIZE_LO - 1);
		SATURATE(mz2, 0, PAP_SIZE_LO - 1);

		for (mx = mx1; mx <= mx2; mx++)
		for (mz = mz1; mz <= mz2; mz++)
		{
			for (oi = OB_find(mx,mz); oi->prim; oi++)
			{
				if (prim_objects[oi->prim].damage & PRIM_DAMAGE_DAMAGABLE)
				{
					dx = oi->x - x;
					dy = oi->y - y;
					dz = oi->z - z;

					dist = abs(dx) + abs(dy) + abs(dz);

					if (dist < radius)
					{
						//
						// What damage does this object recieve?
						//

						hitpoints = maxdamage * (radius - dist) / radius;

						if (hitpoints > 50)
						{
							//
							// Damage the object.
							//

							OB_damage(
								oi->index,
								x,
								z,
								oi->x,
								oi->z,
								p_aggressor);
						}
					}
				}

				oi += 1;

			}
		}
	}
}



void COLLIDE_find_seethrough_fences()
{
	SLONG i;

	DFacet *df;

	for (i = 1; i < next_dfacet; i++)
	{
		df = &dfacets[i];

		if (df->FacetType == STOREY_TYPE_FENCE      ||
			df->FacetType == STOREY_TYPE_FENCE_FLAT ||
			df->FacetType == STOREY_TYPE_OUTSIDE_DOOR)
		{
			//
			// If this fence contains a see-through texture... or all of them for now!
			//

			df->FacetFlags |= FACET_FLAG_SEETHROUGH;
		}
	}
}







// ========================================================
//
// FASTNAV CALCULATION
//
// ========================================================

//
// The fastnav bits array.
//

COLLIDE_Fastnavrow *COLLIDE_fastnav;

//
// Calculate the fastnav bits array.
//
#ifndef	PSX
#ifndef TARGET_DC
void COLLIDE_calc_fastnav_bits()
{
	//
	// Mark all squares as being fastnav.
	//

	memset(COLLIDE_fastnav, -1, PAP_SIZE_HI * PAP_SIZE_HI >> 3);

	//
	// Go through all the facets and mark neighbouring squares
	// as not being fastnav squares.
	//

	SLONG i;
	SLONG j;
	SLONG k;
	SLONG x;
	SLONG z;
	SLONG dx;
	SLONG dz;
	SLONG mx;
	SLONG mz;
	SLONG len;

	DFacet *df;

	for (i = 1; i < next_dfacet; i++)
	{
		df = &dfacets[i];

		dx = df->x[1] - df->x[0];
		dz = df->z[1] - df->z[0];

		len = MAX(abs(dx),abs(dz));

		if (!(dx == 0 || dz == 0))
		{
			//
			// Ignore diagonal facets.
			//

			continue;
		}

		dx = SIGN(dx);
		dz = SIGN(dz);

		x = df->x[0];
		z = df->z[0];

		for (j = 0; j < len; j++)
		{
			//
			// Mark all the squares adjoining (x,z) as not being available
			// for fastnav.
			//

			for (k = 0; k < 4; k++)
			{
				mx = x - (k &  1);
				mz = z - (k >> 1);

				if (WITHIN(mx, 0, PAP_SIZE_HI - 1) &&
					WITHIN(mz, 0, PAP_SIZE_HI - 1))
				{
					COLLIDE_fastnav[mx][mz >> 3] &= ~(1 << (mz & 0x7));
				}
			}

			x += dx;
			z += dz;
		}
	}
}
#endif
#endif

#ifndef NDEBUG

//
// Returns TRUE if you can fastnav in the given square.
//

SLONG COLLIDE_can_i_fastnav(SLONG x, SLONG z)
{
	if (!WITHIN(x, 0, PAP_SIZE_HI - 1) ||
		!WITHIN(z, 0, PAP_SIZE_HI - 1))
	{
		return TRUE;
	}

	SLONG byte = z >> 3;
	SLONG bit  = 1 << (z & 0x7);
	SLONG ans  = COLLIDE_fastnav[x][byte] & bit;

	return ans;
}

#endif // NDEBUG In release build, this function is a #define

#ifndef PSX
#ifndef TARGET_DC

void COLLIDE_debug_fastnav(
		SLONG world_x,		// 8-bits per mapsquare.
		SLONG world_z)
{
	SLONG mx;
	SLONG mz;

	SLONG cx;
	SLONG cy;
	SLONG cz;

	SLONG mx1 = world_x - 0x800 >> 8;
	SLONG mz1 = world_z - 0x800 >> 8;
	SLONG mx2 = world_x + 0x800 >> 8;
	SLONG mz2 = world_z + 0x800 >> 8;

	SATURATE(mx1, 0, PAP_SIZE_HI - 1);
	SATURATE(mz1, 0, PAP_SIZE_HI - 1);
	SATURATE(mx2, 0, PAP_SIZE_HI - 1);
	SATURATE(mz2, 0, PAP_SIZE_HI - 1);

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		if (COLLIDE_can_i_fastnav(mx,mz))
		{
			cx = (mx << 8) + 0x80;
			cz = (mz << 8) + 0x80;

			cy = PAP_calc_map_height_at(cx,cz);

			AENG_world_line(
				cx - 0x10,
				cy,
				cz - 0x10,
				16,
				0xff00ff,
				cx + 0x10,
				cy,
				cz + 0x10,
				16,
				0xff00ff,
				TRUE);

			AENG_world_line(
				cx + 0x10,
				cy,
				cz - 0x10,
				16,
				0xff00ff,
				cx - 0x10,
				cy,
				cz + 0x10,
				16,
				0xff00ff,
				TRUE);
		}
	}
}

#endif
#endif

// ========================================================
//
// EARLY OUT TESTS
//
// ========================================================

#ifndef TARGET_DC
void box_box_early_out(
		SLONG box1_mid_x,
		SLONG box1_mid_z,
		SLONG box1_min_x,
		SLONG box1_min_z,
		SLONG box1_max_x,
		SLONG box1_max_z,
		SLONG box1_yaw,
		SLONG box2_mid_x,
		SLONG box2_mid_z,
		SLONG box2_min_x,
		SLONG box2_min_z,
		SLONG box2_max_x,
		SLONG box2_max_z,
		SLONG box2_yaw)
{
}
#endif

void box_circle_early_out(
		SLONG box1_mid_x,
		SLONG box1_mid_z,
		SLONG box1_min_x,
		SLONG box1_min_z,
		SLONG box1_max_x,
		SLONG box1_max_z,
		SLONG box1_yaw,
		SLONG cx,
		SLONG cz,
		SLONG cradius);

//
// returns if a fence has a hole and also its along position from 1 to 255
//

#ifdef	UNUSED_WIRECUTTERS
UWORD	next_cut_hole=0;
UBYTE	hole_pos[8];

SLONG	get_fence_hole(struct DFacet *p_facet)
{
	SLONG	c0;
	UWORD	flags;
	SLONG	best=999;

	if((flags=p_facet->CutHole)==0)
		return(0);

	for(c0=0;c0<8;c0++)
	{
		if(flags&(1<<c0))
		{
			flags&=~(1<<c0);
			if(hole_pos[c0]<best)
				best=hole_pos[c0];
			if(flags==0)
				return(best==999?0:best);
		}
	}
	return(0);
}

SLONG	get_fence_hole_next(struct DFacet *p_facet,SLONG along)
{
	SLONG	c0;
	UWORD	flags;
	SLONG	best=999;

	if((flags=p_facet->CutHole)==0)
		return(0);

	for(c0=0;c0<8;c0++)
	{
		if(flags&(1<<c0))
		{
			flags&=~(1<<c0);

			if(hole_pos[c0]<best && hole_pos[c0]>along)
				best=hole_pos[c0];

			if(flags==0)
				return(best==999?0:best);
		}
	}
	return(0);
}

void	set_fence_hole(struct DFacet *p_facet,SLONG pos)
{
	if(next_cut_hole<8)
	{

		p_facet->CutHole|=1<<(next_cut_hole);
		hole_pos[next_cut_hole]=pos;
		next_cut_hole++;
	}
}
#endif
