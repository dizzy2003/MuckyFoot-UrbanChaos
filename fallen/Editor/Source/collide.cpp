#include	"Editor.hpp"
#include	"ColTab.hpp"


#pragma warning( disable : 4244 )


extern	SLONG	point_in_quad(SLONG px,SLONG pz,SLONG x,SLONG y,SLONG z,SWORD face);

struct	
{
	SLONG Left;
	SLONG Right;
	SLONG Top;
	SLONG Bottom;
	SLONG Depth;
	SLONG Near;
	SLONG Far;
	SLONG Flag;
}col_data;




#define	FIXSHIFT	9

#define	TSHIFT	8

#ifdef	_MSC_VER
SBYTE	SGN_CROSS64(SLONG ax8, SLONG ay8, SLONG bx8, SLONG by8)
{
	return	0;	
}
#else
SBYTE	SGN_CROSS64(SLONG ax8, SLONG ay8, SLONG bx8, SLONG by8);
#pragma	aux	SGN_CROSS64 = 											\
																	\
"		imul	edx		" 	/* edx:eax = axby					*/	\
"		xchg	eax,ebx	"	/*									*/	\
"		xchg	edx,ecx	"	/*	ecx:ebx = axby					*/	\
"		imul	edx		"	/*	edx:eax = aybx					*/	\
"		sub		ebx,eax	"	/*	lower 32 (ax now free)			*/	\
"		setnz	al		"	/*  assuming upper=0, al=0,+1		*/	\
"		sbb		ecx,edx "	/*	upper 32 ecx:ebx = axby-aybx	*/	\
"		jz		skip	"	/*  =0 -> use lower 32 result in al	*/	\
"		 setl	ah		"	/*	(or jl	-1 case)				*/	\
"		 setg	al		"	/*	(or jg	+1 case)				*/	\
"		 sub	al,ah	"	/*	al = -1,+1						*/	\
"skip:					"	/*  al = -1,0,+1					*/	\
																	\
		parm	[eax][ebx][ecx][edx]								\
		modify	[eax ebx ecx edx]									\
		value	[al];
#endif

#define SGNCROSS(ax8,ay8,bx8,by8,cx8,cy8)		\
	SGN_CROSS64(((bx8)-(ax8)),((by8)-(ay8)),((cx8)-(bx8)),((cy8)-(by8)))

#define BENDS_NONEXY64(ax8,ay8,bx8,by8,cx8,cy8)		((SGNCROSS((ax8),(ay8),(bx8),(by8),(cx8),(cy8)))== 0)
#define BENDS_LEFTXY64(ax8,ay8,bx8,by8,cx8,cy8)		((SGNCROSS((ax8),(ay8),(bx8),(by8),(cx8),(cy8))) < 0)
#define BENDS_RIGHTXY64(ax8,ay8,bx8,by8,cx8,cy8)	((SGNCROSS((ax8),(ay8),(bx8),(by8),(cx8),(cy8))) > 0)


UBYTE	check_big_point_triangle64(SLONG x,SLONG y,SLONG ux,SLONG uy,SLONG vx,SLONG vy,SLONG wx,SLONG wy)
{
	if (BENDS_RIGHTXY64(ux,uy, vx,vy, x,y) &&
		BENDS_RIGHTXY64(vx,vy, wx,wy, x,y) &&
		BENDS_RIGHTXY64(wx,wy, ux,uy, x,y)) return 1;
	
	return 0;
}

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

#ifdef	__WINDOWS_386__
SLONG DIV64(SLONG, SLONG);
#pragma aux DIV64 =\
				"	mov		eax,edx			"\
				"	shl		eax,16			"\
				"	sar		edx,16			"\
				"	idiv	ebx				"\
				parm[edx][ebx]			 	 \
				modify[eax ebx edx]		     \
				value[eax]

SLONG MUL64(SLONG, SLONG);
#pragma aux MUL64 =\
				"	imul	ebx				"\
				"	mov		ax,dx			"\
				"	rol		eax,16			"\
				parm[eax][ebx]          	 \
				modify[eax ebx edx]		     \
				value[eax]
#endif

SLONG	get_height_on_plane_tri(SLONG x,SLONG z,SLONG ux,SLONG uy,SLONG uz,SLONG vx,SLONG vy,SLONG vz,SLONG wx,SLONG wy,SLONG wz)
{

	SLONG	y;
	SLONG	s,t,top,bot,res;
	SLONG	obj_x,obj_z,obj_y;
	UBYTE	flag=0;
	
	top=(z-uz)*(wx-ux)+(ux-x)*(wz-uz);
	bot=(vz-uz)*(wx-ux)-(vx-ux)*(wz-uz);

	if(bot==0)
		return 0;

	s=(top<<FIXSHIFT)/bot;
//	if(s<0)
//		return 0;

	if((wx-ux)==0)
	{
		t=((z<<FIXSHIFT)-(uz<<FIXSHIFT)-s*(vz-uz))/(wz-uz);
		flag=1;
	}
	else
		t=((x<<FIXSHIFT)-(ux<<FIXSHIFT)-s*(vx-ux))/(wx-ux);
//	if(t<0)
//		return 0;

	res=s+t;
	y=uy+(((wy-uy)*t)>>FIXSHIFT)+(((vy-uy)*s)>>FIXSHIFT);
	return(y);
/*
	if( res<(1<<FIXSHIFT))
	{
		y=uy+(((wy-uy)*t)>>FIXSHIFT)+(((vy-uy)*s)>>FIXSHIFT);
		return(y);
	}
	else
	   return	0;  // point outside triangle
*/
}


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

SLONG	get_height_on_face_quad64_at(SLONG x, SLONG z,SLONG obj_x,SLONG obj_y,SLONG obj_z, UWORD face)
{
	SLONG 	ux,uy,uz,vx,vy,vz,wx,wy,wz;
	struct	PrimFace4 *this_face4;
	SLONG	ax,ay,az,bx,by,bz;

	SLONG	top, bot;
	SLONG	alpha, beta;
	SLONG	y;

	this_face4=&prim_faces4[face];

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
		LogText(" get height on QUAD NOT %d alpha %x beta %x \n",face,alpha,beta);
		return 1000000;
	}
	else
	{
//		LogText(" get height on face=%d alpha %x beta %x  uy %d vy %d wy %d\n",y>>8,face,alpha,beta),uy,vy,wy;
		return y >> 8;
	}

}


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

SLONG lines_intersect(SLONG x1,SLONG  y1,SLONG x2,SLONG  y2,SLONG x3,SLONG  y3,SLONG x4,SLONG  y4,SLONG *x,SLONG  *y)
{
    long a1, a2, b1, b2, c1, c2; 
    long r1, r2, r3, r4;         
    long denom, offset, num;     

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = x2 * y1 - x1 * y2;

    r3 = a1 * x3 + b1 * y3 + c1;
    r4 = a1 * x4 + b1 * y4 + c1;

    if ( r3 != 0 &&
         r4 != 0 &&
         SAME_SIGNS( r3, r4 ))
        return ( DONT_INTERSECT );


    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = x4 * y3 - x3 * y4;


    r1 = a2 * x1 + b2 * y1 + c2;
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


UBYTE	two4_line_intersection(SLONG x1,SLONG y1,SLONG x2,SLONG y2,SLONG x3,SLONG y3,SLONG x4,SLONG y4)
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

	ay=y2-y1;
	by=y3-y4;

	if(ay<0)
	{
		y1lo=(SWORD)y2;
		y1hi=(SWORD)y1;
	}
	else
	{
		y1hi=(SWORD)y2;
		y1lo=(SWORD)y1;
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
	cy=y1-y3;

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
//SLONG	play_x,play_y,play_z;





void	insert_rect(struct SVECTOR *point,SLONG width,SLONG height)
{
	struct	BucketRect	*p_bucket;

	if(current_bucket_pool>=end_bucket_pool)
		return;

	p_bucket=(struct BucketRect*)current_bucket_pool;

	setRect(p_bucket);
	
	setCol2(p_bucket,255);
	setXY1(p_bucket,point->X-5,point->Y-5);
	p_bucket->Width=width;
	p_bucket->Height=height;

	add_bucket(p_bucket,point->Z);
	current_bucket_pool+=sizeof(struct	BucketRect);
}	



SLONG	calc_height_at(SLONG x,SLONG z)
{
	struct	DepthStrip *me;
	SLONG	new_y,h0,h1,h2,h3;

	if(x<0||x>=(EDIT_MAP_WIDTH<<ELE_SHIFT)||z<0||z>=(EDIT_MAP_DEPTH<<ELE_SHIFT))
		return(0);
	me=&edit_map[x>>ELE_SHIFT][z>>ELE_SHIFT];

	h1=me->Y<<FLOOR_HEIGHT_SHIFT;                //my_big_map[(new_x.L.HWord)+((new_z.L.HWord)*MAP_WIDTH)].Alt;
	if( (x>>8) >=(EDIT_MAP_WIDTH))
		h0=h1;
	else
		h0=((me+EDIT_MAP_WIDTH)->Y)<<FLOOR_HEIGHT_SHIFT;            //my_big_map[(new_x.L.HWord+1)+((new_z.L.HWord)*MAP_WIDTH)].Alt;

	if( (z>>8) >=(EDIT_MAP_DEPTH))
	{
		h2=h1;
		h3=h0;
	}
	else
	{
		h2=((me+1)->Y)<<FLOOR_HEIGHT_SHIFT;     //my_big_map[(new_x.L.HWord)+((new_z.L.HWord+1)*MAP_WIDTH)].Alt;
		h3=((me+EDIT_MAP_WIDTH+1)->Y)<<FLOOR_HEIGHT_SHIFT;   //my_big_map[(new_x.L.HWord+1)+((new_z.L.HWord+1)*MAP_WIDTH)].Alt;
	}

	if(h0==h1&&h1==h2&&h2==h3)
	{
		return(h0);
	}
	//LogText("CALC HEIGHT x %d.%d z %d.%d  h1 %d h0 %d h2 %d h3 %d \n",x>>ELE_SHIFT,x&(~ELE_AND),z>>ELE_SHIFT,z&(~ELE_AND),h1,h0,h2,h3);
	x=x&(~ELE_AND);	
	z=z&(~ELE_AND);	
	if(x+z<ELE_SIZE)
		new_y=(h1+(((h0-h1)*(x))>>ELE_SHIFT)+(((h2-h1)*(z))>>ELE_SHIFT));
	else
		new_y=(h3+(((h2-h3)*(ELE_SIZE-x))>>ELE_SHIFT)+(((h0-h3)*(ELE_SIZE-z))>>ELE_SHIFT));
	//LogText(" result %d \n",new_y);
	return(new_y);
}


void	process_camera(struct MapThing *p_thing)
{
	static	r_camera_dist=2000,r_camera_angle_x=1200,camera_angle_y=0;
	static	r_camera_angle_dx=0,r_camera_angle_dy=0;
	SLONG	dx,dy,dz,rx,ry,rz;
	SLONG	ang_x,ang_y;
// camera wants to point at player who is at play_x,play_y,play_z

//for now use engine.X,engine.Y,engine.Z as current camera pos

//engine.AngleX

	//r_camera_angle_y=engine.AngleY>>8;
	{
		SLONG	dy;
		dy=p_thing->AngleY-camera_angle_y;
		if(dy>1024)
			dy=-(2048-dy);
		if(dy<-1024)
		{
			dy=2048+dy;
		}
		dy>>=4;

		camera_angle_y+=dy;
		camera_angle_y=(camera_angle_y+2048)&2047;
	}

	if(Keys[KB_Q])
	{
		r_camera_dist-=10;
	}
	if(Keys[KB_W])
	{
		r_camera_dist+=10;
	}

	if(Keys[KB_A])
	{
		r_camera_angle_dy+=10;
		r_camera_angle_dy=(r_camera_angle_dy+2048)&2047;
	}

	if(Keys[KB_S])
	{
		r_camera_angle_dy-=10;
		r_camera_angle_dy=(r_camera_angle_dy+2048)&2047;
	}

	if(Keys[KB_R])
	{
		r_camera_angle_dx+=10;
		r_camera_angle_dx=(r_camera_angle_dx+2048)&2047;
	}

	if(Keys[KB_F])
	{
		r_camera_angle_dx-=10;
		r_camera_angle_dx=(r_camera_angle_dx+2048)&2047;
	}

	dz=-r_camera_dist;
	dy=0;
	dx=0;

	ang_x=(r_camera_angle_x+r_camera_angle_dx+2048)&2047;
	ang_y=(camera_angle_y+r_camera_angle_dy+2048)&2047;

	//rotate about x
	ry = (dy*COS(ang_x)-dz*SIN(ang_x))>>16;
	rz = (dy*SIN(ang_x)+dz*COS(ang_x))>>16;

	//rotate about y

	rx =	(dx*COS(ang_y)-rz*SIN(ang_y))>>16; 
	rz =	(dx*SIN(ang_y)+rz*COS(ang_y))>>16;


	engine.X=(p_thing->X-rx)<<8;
	engine.Y=(p_thing->Y-ry)<<8;
	engine.Z=(p_thing->Z+rz)<<8;

	engine.AngleX=(ang_x)<<8;
	engine.AngleY=(ang_y)<<8;

}

SLONG	calc_height_on_face(SLONG x,SLONG z,SLONG face)
{
	SLONG	face_x,face_y,face_z;
	SLONG   wall;
	SLONG   storey;
	SLONG	building;
	SLONG   thing;
	Thing  *p_thing;

	if(face>0)
	{
		wall = prim_faces4[face].ThingIndex;

		if (wall < 0)
		{
			storey   = wall_list[-wall].StoreyHead;
			building = storey_list[storey].BuildingHead;
			thing    = building_list[building].ThingIndex;

			p_thing = TO_THING(thing);

			face_x = p_thing->WorldPos.X >> 8;
			face_y = p_thing->WorldPos.Y >> 8;
			face_z = p_thing->WorldPos.Z >> 8;

//			if(point_in_quad(x,z,face_x,face_y,face_z,face))
			{
				SLONG	new_y;
				new_y=get_height_on_face_quad64_at(x,z,face_x,face_y,face_z,face);
				if(new_y!=1000000)
				{
					return(new_y);
					//play_y=new_y;
					//LogText(" calc things height on QUAD %d \n",p_thing->Y);
				}
			}
//			else
//				LogText(" Warning thing on face not on face, no height calc \n");
		}
	}
	return(-100);
}

void calc_things_height(struct MapThing *p_thing)
{
	if(p_thing->OnFace)
	{
		if(p_thing->OnFace>0)
		{
			p_thing->Y = calc_height_on_face(p_thing->X, p_thing->Y, p_thing->OnFace);
			
			/*
			SLONG	face_x,face_y,face_z;
			SLONG	building;
			building=prim_faces4[p_thing->OnFace].ThingIndex;
			if(building<0)
			{
				building=wall_list[-building].StoreyHead;
				building=storey_list[building].BuildingHead;
				face_x=building_list[building].X;
				face_y=building_list[building].Y;
				face_z=building_list[building].Z;

				if(point_in_quad(p_thing->X,p_thing->Z,face_x,face_y,face_z,p_thing->OnFace))
				{
					SLONG	new_y;
					new_y=get_height_on_face_quad64_at(p_thing->X,p_thing->Z,face_x,face_y,face_z,p_thing->OnFace);
					if(new_y!=1000000)
					{
						p_thing->Y=new_y;
						//play_y=new_y;
						//LogText(" calc things height on QUAD %d \n",p_thing->Y);
					}
				}
				else
					LogText(" Warning thing on face not on face, no height calc \n");
			}

			*/
		}
		else
		{

		}
	}
	else
	{
		p_thing->Y=calc_height_at(p_thing->X,p_thing->Z);
		//LogText(" calc things height on FLOOR %d \n",p_thing->Y);

	}
}
void	init_thing(void)
{
}
ULONG	move_thing(SLONG m_dx,SLONG m_dy,SLONG m_dz)
{
	return(0);
}





static	UWORD	done_list[116],done_count=0;

void	add_to_done_list(UWORD p1,UWORD p2)
{
	if(done_count>100)
		return;
	done_list[done_count]=(UWORD)(p1 | (p2<<4));
	done_count++;
}

UWORD	is_it_done(UWORD p1,UWORD p2)
{
	UWORD	flip1,flip2,c0;
	flip2 = (UWORD)(p1 | (p2<<4));
	flip1 = (UWORD)(p2 | (p1<<4));
	for(c0=0;c0<done_count;c0++)
	{
		if(done_list[c0]==flip1 ||done_list[c0]==flip2)
			return(1);
	}
	return(0);
}


void	insert_col_for_quad(struct EditMapElement *p_ele,struct SVECTOR *points,SLONG p1,SLONG p2,SLONG p3,SLONG p4)
{
	SLONG	c1,c2;
	ULONG	indi[4];
	SLONG	az;
	indi[0]=p1;
	indi[1]=p2;
	indi[2]=p3;
	indi[3]=p4;
	done_count=0;

	az=(points[p1].Z+points[p2].Z+points[p3].Z+points[p4].Z)>>2;

//	if(points[p1].Y==points[p2].Y && points[p2].Y==points[p3].Y && points[p3].Y==points[p4].Y)
	{ //in plane XZ
		
		for(c2=0;c2<4;c2++)
		for(c1=0;c1<4;c1++)
		{
			if(c1!=c2)
			if(points[indi[c1]].Z==points[indi[c2]].Z)
			{
				if(points[indi[c1]].X!=points[indi[c2]].X||points[indi[c1]].Y!=points[indi[c2]].Y)
				if(!is_it_done((UWORD)indi[c1],(UWORD)indi[c2]) )
				{
//					insert_collision_vect(points[indi[c1]].X,points[indi[c1]].Y,points[indi[c1]].Z,points[indi[c2]].X,points[indi[c2]].Y,points[indi[c2]].Z,*((UBYTE*)&(p_ele->CubeType)),0);
					insert_collision_vect(points[indi[c1]].X,points[indi[c1]].Y,az,points[indi[c2]].X,points[indi[c2]].Y,az,*((UBYTE*)&(p_ele->CubeType)),0,0);
					add_to_done_list((UWORD)indi[c1],(UWORD)indi[c2]);
				}
			}
		}
	}
/*
	else
	if(points[p1].X==points[p2].X && points[p2].X==points[p3].X && points[p3].X==points[p4].X)
	{ //in plane YZ
		
		for(c2=0;c2<4;c2++)
		for(c1=0;c1<4;c1++)
		{
			if(c1!=c2)
			if(points[indi[c1]].Z==points[indi[c2]].Z)
			{
				if(points[indi[c1]].Y!=points[indi[c2]].Y)
				if(!is_it_done(indi[c1],indi[c2]) )
				{
					insert_collision_vect(points[indi[c1]].X,points[indi[c1]].Y,points[indi[c1]].Z,points[indi[c2]].X,points[indi[c2]].Y,points[indi[c2]].Z,*((UBYTE*)&(p_ele->CubeType)),0);
					add_to_done_list(indi[c1],indi[c2]);
				}
			}
		}
	}
*/

}

void	insert_col_for_tri(struct EditMapElement *p_ele,struct SVECTOR *points,SLONG p1,SLONG p2,SLONG p3)
{
	SLONG	c1,c2;
	ULONG	indi[4];
	indi[0]=p1;
	indi[1]=p2;
	indi[2]=p3;
	done_count=0;

	{
		
		for(c2=0;c2<3;c2++)
		for(c1=0;c1<3;c1++)
		{
			if(c1!=c2)
			if(points[indi[c1]].Z==points[indi[c2]].Z)
			{
				if(points[indi[c1]].Y!=points[indi[c2]].Y)
				if(!is_it_done((UWORD)indi[c1],(UWORD)indi[c2]) )
				{
					insert_collision_vect(points[indi[c1]].X,points[indi[c1]].Y,points[indi[c1]].Z,points[indi[c2]].X,points[indi[c2]].Y,points[indi[c2]].Z,*((UBYTE*)&(p_ele->CubeType)),0,0);
					add_to_done_list(indi[c1],indi[c2]);
				}
			}
		}
	}

}

inline	void	rotate_local_points(SLONG angle,struct	SVECTOR	*point,SLONG x,SLONG y,SLONG z)
{
	SLONG	cosa,sina;
	SLONG	rx,rz;
	point->X-=x;	
//	point->Y-=y;	
	point->Z-=z;	
	angle<<=9;
	cosa=COS(angle);
	sina=SIN(angle);

	rx=(cosa*point->X+sina*point->Z)>>16;
	rz=(-sina*point->X+cosa*point->Z)>>16;
	point->X=rx+x;
//	point->Y=ry+y;
	point->Z=rz+z;
	y=y;
}

static	UBYTE	rotate_table[6][4]=
{
	{CUBE_FLAG_FRONT,CUBE_FLAG_LEFT,0,CUBE_FLAG_RIGHT},
	{CUBE_FLAG_TOP,CUBE_FLAG_TOP,CUBE_FLAG_TOP,CUBE_FLAG_TOP},
	{CUBE_FLAG_BOTTOM,CUBE_FLAG_BOTTOM,CUBE_FLAG_BOTTOM,CUBE_FLAG_BOTTOM},
	{CUBE_FLAG_LEFT,0,CUBE_FLAG_RIGHT,CUBE_FLAG_FRONT},
	{CUBE_FLAG_RIGHT,CUBE_FLAG_FRONT,CUBE_FLAG_LEFT,0},
	{0,CUBE_FLAG_RIGHT,CUBE_FLAG_FRONT,CUBE_FLAG_LEFT}
};


void	col_for_cube_ele_at(SLONG	x,SLONG y,SLONG z,struct EditMapElement *p_ele)
{
	struct	SVECTOR	points[8];

	points[0].X=x-HALF_ELE_SIZE;
	points[0].Y=y-HALF_ELE_SIZE;
	points[0].Z=z-HALF_ELE_SIZE;

	points[1].X=x+HALF_ELE_SIZE;
	points[1].Y=y-HALF_ELE_SIZE;
	points[1].Z=z-HALF_ELE_SIZE;
	
	points[2].X=x+HALF_ELE_SIZE;
	points[2].Y=y+HALF_ELE_SIZE;
	points[2].Z=z-HALF_ELE_SIZE;

	points[3].X=x-HALF_ELE_SIZE;
	points[3].Y=y+HALF_ELE_SIZE;
	points[3].Z=z-HALF_ELE_SIZE;

	points[4].X=x-HALF_ELE_SIZE;
	points[4].Y=y-HALF_ELE_SIZE;
	points[4].Z=z+HALF_ELE_SIZE;

	points[5].X=x+HALF_ELE_SIZE;
	points[5].Y=y-HALF_ELE_SIZE;
	points[5].Z=z+HALF_ELE_SIZE;

	points[6].X=x+HALF_ELE_SIZE;
	points[6].Y=y+HALF_ELE_SIZE;
	points[6].Z=z+HALF_ELE_SIZE;

	points[7].X=x-HALF_ELE_SIZE;
	points[7].Y=y+HALF_ELE_SIZE;
	points[7].Z=z+HALF_ELE_SIZE;

/*
	if(p_ele->CubeFlags&CUBE_FLAG_FRONT)
	{
		insert_col_for_quad(p_ele,points,3,2,1,0);
	}
*/
	if(p_ele->CubeFlags&CUBE_FLAG_RIGHT)
	{
		insert_col_for_quad(p_ele,points,5,1,2,6);
	}
	
	if(p_ele->CubeFlags&CUBE_FLAG_TOP)
	{
		insert_col_for_quad(p_ele,points,4,0,1,5);
	}
	
	if(p_ele->CubeFlags&CUBE_FLAG_LEFT)
	{
		insert_col_for_quad(p_ele,points,4,7,3,0);
	}

	if(p_ele->CubeFlags&CUBE_FLAG_BOTTOM)
	{
		insert_col_for_quad(p_ele,points,3,7,6,2);
	}
}


//
//  0---1  10   11
//	| 	| 		|
//	|   2---3	12	13
//	|	 	|		 |
//	|	    4----5	 14	   15
//  |			 |		   |
//	9---8----7----6	 	   |
//		   19	18	 17	   16
void	col_for_steps_lr_ele_at(SLONG	x,SLONG y,SLONG z,struct EditMapElement *p_ele)
{
	struct	SVECTOR	points[20];
	SLONG	c0;
	SLONG	rot;

	points[0].X=x-HALF_ELE_SIZE;
	points[0].Y=y-HALF_ELE_SIZE;
	points[0].Z=z-HALF_ELE_SIZE;

	
	points[1].X=x-HALF_ELE_SIZE/3;
	points[1].Y=y-HALF_ELE_SIZE;
	points[1].Z=z-HALF_ELE_SIZE;

	points[2].X=x-HALF_ELE_SIZE/3;
	points[2].Y=y-HALF_ELE_SIZE/3;
	points[2].Z=z-HALF_ELE_SIZE;

	points[3].X=x+HALF_ELE_SIZE/3;
	points[3].Y=y-HALF_ELE_SIZE/3;
	points[3].Z=z-HALF_ELE_SIZE;

	points[4].X=x+HALF_ELE_SIZE/3;
	points[4].Y=y+HALF_ELE_SIZE/3;
	points[4].Z=z-HALF_ELE_SIZE;

	points[5].X=x+HALF_ELE_SIZE;
	points[5].Y=y+HALF_ELE_SIZE/3;
	points[5].Z=z-HALF_ELE_SIZE;

	points[6].X=x+HALF_ELE_SIZE;
	points[6].Y=y+HALF_ELE_SIZE;
	points[6].Z=z-HALF_ELE_SIZE;

	points[7].X=x+HALF_ELE_SIZE/3;
	points[7].Y=y+HALF_ELE_SIZE;
	points[7].Z=z-HALF_ELE_SIZE;			    

	points[8].X=x-HALF_ELE_SIZE/3;
	points[8].Y=y+HALF_ELE_SIZE;
	points[8].Z=z-HALF_ELE_SIZE;

	points[9].X=x-HALF_ELE_SIZE;
	points[9].Y=y+HALF_ELE_SIZE;
	points[9].Z=z-HALF_ELE_SIZE;


// far side
	points[10].X=x-HALF_ELE_SIZE;
	points[10].Y=y-HALF_ELE_SIZE;
	points[10].Z=z+HALF_ELE_SIZE;
	
	points[11].X=x-HALF_ELE_SIZE/3;
	points[11].Y=y-HALF_ELE_SIZE;
	points[11].Z=z+HALF_ELE_SIZE;

	points[12].X=x-HALF_ELE_SIZE/3;
	points[12].Y=y-HALF_ELE_SIZE/3;
	points[12].Z=z+HALF_ELE_SIZE;

	points[13].X=x+HALF_ELE_SIZE/3;
	points[13].Y=y-HALF_ELE_SIZE/3;
	points[13].Z=z+HALF_ELE_SIZE;

	points[14].X=x+HALF_ELE_SIZE/3;
	points[14].Y=y+HALF_ELE_SIZE/3;
	points[14].Z=z+HALF_ELE_SIZE;

	points[15].X=x+HALF_ELE_SIZE;
	points[15].Y=y+HALF_ELE_SIZE/3;
	points[15].Z=z+HALF_ELE_SIZE;

	points[16].X=x+HALF_ELE_SIZE;
	points[16].Y=y+HALF_ELE_SIZE;
	points[16].Z=z+HALF_ELE_SIZE;

	points[17].X=x+HALF_ELE_SIZE/3;
	points[17].Y=y+HALF_ELE_SIZE;
	points[17].Z=z+HALF_ELE_SIZE;

	points[18].X=x-HALF_ELE_SIZE/3;
	points[18].Y=y+HALF_ELE_SIZE;
	points[18].Z=z+HALF_ELE_SIZE;

	points[19].X=x-HALF_ELE_SIZE;
	points[19].Y=y+HALF_ELE_SIZE;
	points[19].Z=z+HALF_ELE_SIZE;

	rot=p_ele->CubeType.Rot;

	if(rot)
	{
		for(c0=0;c0<20;c0++)
			rotate_local_points(rot,&points[c0],x,y,z);
	}
/*
	for(c0=0;c0<20;c0++)
	{
		//transform all points for this Object
		flags[c0]=rotate_point_gte(&points[c0],&res[c0]);
	}
*/
//	if(p_ele->CubeFlags&CUBE_FLAG_FRONT)
/*
	if(p_ele->CubeFlags&rotate_table[CUBE_INDEX_FRONT][rot])
	{
		insert_col_for_quad(p_ele,points,0,9,8,1);
		insert_col_for_quad(p_ele,points,2,8,7,3);
		insert_col_for_quad(p_ele,points,4,7,6,5);
	}
*/

	if(p_ele->CubeFlags&rotate_table[5][rot])
	{
		insert_col_for_quad(p_ele,points,10,11,18,19);
		insert_col_for_quad(p_ele,points,12,13,17,18);
		insert_col_for_quad(p_ele,points,14,15,16,17);
	}

	
	if(p_ele->CubeFlags&CUBE_FLAG_TOP)
	{
		insert_col_for_quad(p_ele,points,10,0,1,11);
		insert_col_for_quad(p_ele,points,12,2,3,13);
		insert_col_for_quad(p_ele,points,14,4,5,15);
	}
	
	if(p_ele->CubeFlags&rotate_table[CUBE_INDEX_LEFT][rot])
  //	if(p_ele->CubeFlags&CUBE_FLAG_LEFT)
	{
		insert_col_for_quad(p_ele,points,0,10,19,9);
	}

//	if(p_ele->CubeFlags&CUBE_FLAG_RIGHT)
	if(p_ele->CubeFlags&rotate_table[CUBE_INDEX_RIGHT][rot])
	{
		insert_col_for_quad(p_ele,points,1,2,12,11);
		insert_col_for_quad(p_ele,points,3,4,14,13);
		insert_col_for_quad(p_ele,points,5,6,16,15);
	}

	if(p_ele->CubeFlags&CUBE_FLAG_BOTTOM)
	{
		insert_col_for_quad(p_ele,points,6,9,19,16);
	}
}

void	col_for_ledge1_ele_at(SLONG	x,SLONG y,SLONG z,struct EditMapElement *p_ele)
{
	struct	SVECTOR	points[8];

	points[0].X=x-HALF_ELE_SIZE;
	points[0].Y=y-HALF_ELE_SIZE;
	points[0].Z=z-HALF_ELE_SIZE;

	points[1].X=x+HALF_ELE_SIZE;
	points[1].Y=y-HALF_ELE_SIZE;
	points[1].Z=z-HALF_ELE_SIZE;
	
	points[2].X=x+HALF_ELE_SIZE;
	points[2].Y=y-HALF_ELE_SIZE/3;
	points[2].Z=z-HALF_ELE_SIZE;

	points[3].X=x-HALF_ELE_SIZE;
	points[3].Y=y-HALF_ELE_SIZE/3;
	points[3].Z=z-HALF_ELE_SIZE;

	points[4].X=x-HALF_ELE_SIZE;
	points[4].Y=y-HALF_ELE_SIZE;
	points[4].Z=z+HALF_ELE_SIZE;

	points[5].X=x+HALF_ELE_SIZE;
	points[5].Y=y-HALF_ELE_SIZE;
	points[5].Z=z+HALF_ELE_SIZE;

	points[6].X=x+HALF_ELE_SIZE;
	points[6].Y=y-HALF_ELE_SIZE/3;
	points[6].Z=z+HALF_ELE_SIZE;

	points[7].X=x-HALF_ELE_SIZE;
	points[7].Y=y-HALF_ELE_SIZE/3;
	points[7].Z=z+HALF_ELE_SIZE;

/*
	if(p_ele->CubeFlags&CUBE_FLAG_FRONT)
	{
		insert_col_for_quad(p_ele,points,3,2,1,0);
	}
*/
	if(p_ele->CubeFlags&CUBE_FLAG_RIGHT)
	{
		insert_col_for_quad(p_ele,points,5,1,2,6);
	}
	
	if(p_ele->CubeFlags&CUBE_FLAG_TOP)
	{
		insert_col_for_quad(p_ele,points,4,0,1,5);
	}
	
	if(p_ele->CubeFlags&CUBE_FLAG_LEFT)
	{
		insert_col_for_quad(p_ele,points,4,7,3,0);
	}

	if(p_ele->CubeFlags&CUBE_FLAG_BOTTOM)
	{
		insert_col_for_quad(p_ele,points,3,7,6,2);
	}
}



//
//  0---3
//	| \	 \
//	|  \   \
//	|	 \	 \
//	|	   \   \
//	2	5	1---4

void	col_for_slope_lr_ele_at(SLONG	x,SLONG y,SLONG z,struct EditMapElement *p_ele)
{
	struct	SVECTOR	points[8];
	SLONG	c0;
	SLONG	rot;

	points[0].X=x-HALF_ELE_SIZE;
	points[0].Y=y-HALF_ELE_SIZE;
	points[0].Z=z-HALF_ELE_SIZE;

	
	points[1].X=x+HALF_ELE_SIZE;
	points[1].Y=y+HALF_ELE_SIZE;
	points[1].Z=z-HALF_ELE_SIZE;

	points[2].X=x-HALF_ELE_SIZE;
	points[2].Y=y+HALF_ELE_SIZE;
	points[2].Z=z-HALF_ELE_SIZE;

	points[3].X=x-HALF_ELE_SIZE;
	points[3].Y=y-HALF_ELE_SIZE;
	points[3].Z=z+HALF_ELE_SIZE;

	points[4].X=x+HALF_ELE_SIZE;
	points[4].Y=y+HALF_ELE_SIZE;
	points[4].Z=z+HALF_ELE_SIZE;

	points[5].X=x-HALF_ELE_SIZE;
	points[5].Y=y+HALF_ELE_SIZE;
	points[5].Z=z+HALF_ELE_SIZE;

	rot=p_ele->CubeType.Rot;

	if(rot)
	{
		for(c0=0;c0<6;c0++)
			rotate_local_points(rot,&points[c0],x,y,z);
	}

/*
	if(p_ele->CubeFlags&rotate_table[CUBE_INDEX_FRONT][rot])
	{
		insert_col_for_tri(p_ele,points,0,1,2);
	}
*/

	if(p_ele->CubeFlags&rotate_table[5][rot])
	{
		insert_col_for_tri(p_ele,points,3,5,4);
	}


	if(p_ele->CubeFlags&CUBE_FLAG_TOP)
	{
		insert_col_for_quad(p_ele,points,0,1,4,3);
	}
	
//	if(p_ele->CubeFlags&CUBE_FLAG_LEFT)
	if(p_ele->CubeFlags&rotate_table[CUBE_INDEX_LEFT][rot])
	{
		insert_col_for_quad(p_ele,points,0,3,5,2);
	}

	if(p_ele->CubeFlags&CUBE_FLAG_BOTTOM)
	{
		insert_col_for_quad(p_ele,points,1,2,5,4);
	}
}

//general poly collision
// need to be able to set collision with the XY plane at some point along the Z axis

ULONG	intersect_vector_xy(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG clip_z,SLONG *res_x,SLONG *res_y)
{
	SLONG ratio;
	SLONG	vx,vy,vz;

	vx=x1-x2;
	vy=y1-y2;
	vz=z1-z2;

	if(clip_z>z1&&clip_z>z2 || clip_z<z1 && clip_z<z2||vz==0)
		return(0);

//vector crosses plane

	ratio=((clip_z-z2)<<16)/(vz);

	vx=(vx*ratio)>>16;
	vy=(vy*ratio)>>16;

	*res_x=x2+vx;	
	*res_y=y2+vy;	
	return(1);
}

SLONG	is_point_in_clip(SLONG x,SLONG y,SLONG z)
{
	if(!col_data.Flag)
		return(1);
	else
	{
		if(x>col_data.Left&&x<col_data.Right&&
		   y>col_data.Top&&y<col_data.Bottom)
			return(1);
	}
	return(0);
	
}

void	clip_a_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG clip_z)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	c0,c1;
	struct	PrimObject	*p_obj;
	SLONG	point;

	SLONG	c_x[12],c_y[12],clip_count=0;

	p_obj    =&prim_objects[prim];
	p_f4     =&prim_faces4[p_obj->StartFace4];
	p_f3     =&prim_faces3[p_obj->StartFace3];

	if(p_obj->EndFace4)
	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		clip_count=0;

		for(point=0;point<4;point++)
		{
			if(!is_point_in_clip(x+prim_points[p_f4->Points[point]].X,y+prim_points[p_f4->Points[point]].Y,z+prim_points[p_f4->Points[point]].Z))
				goto	skip_face4;
		}

		if(intersect_vector_xy(	x+prim_points[p_f4->Points[0]].X,y+prim_points[p_f4->Points[0]].Y,z+prim_points[p_f4->Points[0]].Z,
								x+prim_points[p_f4->Points[1]].X,y+prim_points[p_f4->Points[1]].Y,z+prim_points[p_f4->Points[1]].Z,clip_z,&c_x[clip_count],&c_y[clip_count]))
		{
			clip_count++;
		}
		if(intersect_vector_xy(	x+prim_points[p_f4->Points[1]].X,y+prim_points[p_f4->Points[1]].Y,z+prim_points[p_f4->Points[1]].Z,
								x+prim_points[p_f4->Points[3]].X,y+prim_points[p_f4->Points[3]].Y,z+prim_points[p_f4->Points[3]].Z,clip_z,&c_x[clip_count],&c_y[clip_count]))
		{
			clip_count++;
		}
		if(intersect_vector_xy(	x+prim_points[p_f4->Points[3]].X,y+prim_points[p_f4->Points[3]].Y,z+prim_points[p_f4->Points[3]].Z,
								x+prim_points[p_f4->Points[2]].X,y+prim_points[p_f4->Points[2]].Y,z+prim_points[p_f4->Points[2]].Z,clip_z,&c_x[clip_count],&c_y[clip_count]))
		{
			clip_count++;
		}
		if(intersect_vector_xy(	x+prim_points[p_f4->Points[2]].X,y+prim_points[p_f4->Points[2]].Y,z+prim_points[p_f4->Points[2]].Z,
								x+prim_points[p_f4->Points[0]].X,y+prim_points[p_f4->Points[0]].Y,z+prim_points[p_f4->Points[0]].Z,clip_z,&c_x[clip_count],&c_y[clip_count]))
		{
			clip_count++;
		}

		if(clip_count)
		{
/*
			if(clip_count!=2)
			{
				ERROR_MSG(0," strange number of clip locations");
			}
			else
*/
			if(clip_count>=2)
			{
				insert_collision_vect(c_x[0],c_y[0],clip_z,c_x[1],c_y[1],clip_z,0,0,0);
			}
		}
skip_face4:;
		p_f4++;
	}

	if(p_obj->EndFace3)
	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		clip_count=0;

		for(point=0;point<3;point++)
		{
			if(!is_point_in_clip(x+prim_points[p_f3->Points[point]].X,y+prim_points[p_f3->Points[point]].Y,z+prim_points[p_f3->Points[point]].Z))
				goto	skip_face4;
		}

		if(intersect_vector_xy(	x+prim_points[p_f3->Points[0]].X,y+prim_points[p_f3->Points[0]].Y,z+prim_points[p_f3->Points[0]].Z,
								x+prim_points[p_f3->Points[1]].X,y+prim_points[p_f3->Points[1]].Y,z+prim_points[p_f3->Points[1]].Z,clip_z,&c_x[clip_count],&c_y[clip_count]))
		{
			clip_count++;
		}
		if(intersect_vector_xy(	x+prim_points[p_f3->Points[1]].X,y+prim_points[p_f3->Points[1]].Y,z+prim_points[p_f3->Points[1]].Z,
								x+prim_points[p_f3->Points[2]].X,y+prim_points[p_f3->Points[2]].Y,z+prim_points[p_f3->Points[2]].Z,clip_z,&c_x[clip_count],&c_y[clip_count]))
		{
			clip_count++;
		}
		if(intersect_vector_xy(	x+prim_points[p_f3->Points[2]].X,y+prim_points[p_f3->Points[2]].Y,z+prim_points[p_f3->Points[2]].Z,
								x+prim_points[p_f3->Points[0]].X,y+prim_points[p_f3->Points[0]].Y,z+prim_points[p_f3->Points[0]].Z,clip_z,&c_x[clip_count],&c_y[clip_count]))
		{
			clip_count++;
		}

		if(clip_count)
		{
			if(clip_count!=2)
			{
				ERROR_MSG(0," strange number of clip locations");
			}
			else
			{
				insert_collision_vect(c_x[0],c_y[0],clip_z,c_x[1],c_y[1],clip_z,0,0,0);
			}
		}
skip_face3:;
		p_f3++;
	}
}

inline	SLONG	is_point_in_clip_bez(SLONG x,SLONG y,SLONG z)
{
//	LogText(" try y %d   bez top %d bez bot %d \n",y,col_data.Top,col_data.Bottom);
	if(x>col_data.Left&&x<col_data.Right&&
	   z>col_data.Near&&z<col_data.Far&&
	   y>col_data.Top&&y<col_data.Bottom)
		return(1);
	else
		return(0);
	
}


SLONG	point_in_quad(SLONG px,SLONG pz,SLONG x,SLONG y,SLONG z,SWORD face)
{
	SLONG x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4;
	SLONG ret;

	x1=x+prim_points[prim_faces4[face].Points[0]].X;
	y1=y+prim_points[prim_faces4[face].Points[0]].Y;
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

SLONG	point_in_tri(SLONG px,SLONG pz,SLONG x,SLONG y,SLONG z,SWORD face)
{
	SLONG x1,y1,z1,x2,y2,z2,x3,y3,z3;

	x1=x+prim_points[prim_faces3[face].Points[0]].X;
	y1=y+prim_points[prim_faces3[face].Points[0]].Y;
	z1=z+prim_points[prim_faces3[face].Points[0]].Z;

	x2=x+prim_points[prim_faces3[face].Points[1]].X;
	y2=y+prim_points[prim_faces3[face].Points[1]].Y;
	z2=z+prim_points[prim_faces3[face].Points[1]].Z;

	x3=x+prim_points[prim_faces3[face].Points[2]].X;
	y3=y+prim_points[prim_faces3[face].Points[2]].Y;
	z3=z+prim_points[prim_faces3[face].Points[2]].Z;

	return(check_big_point_triangle_col(px-x1,pz-z1,x1-x1,z1-z1,x2-x1,z2-z1,x3-x1,z3-z1));
}

void	apply_vect_to_face_square(SLONG x,SLONG y,SLONG z,SWORD face)
{
	SLONG	c0;
	SLONG	x3,y3,z3,x4,y4,z4;

	if(face>0)
	{

		for (c0=0;c0<4;c0++ )
		{
			x3=x+prim_points[prim_faces4[face].Points[c0]].X;
			y3=y+prim_points[prim_faces4[face].Points[c0]].Y;
			z3=z+prim_points[prim_faces4[face].Points[c0]].Z;

			x4=x+prim_points[prim_faces4[face].Points[(c0+1)&3]].X;
			y4=y+prim_points[prim_faces4[face].Points[(c0+1)&3]].Y;
			z4=z+prim_points[prim_faces4[face].Points[(c0+1)&3]].Z;

			insert_collision_vect(x3,y3,z3,x4,y4,z4,0,0,0);
		}
	}
}

void	box_vect_point(SLONG x,SLONG y,SLONG z)
{
	insert_collision_vect(x-7,y,z-7,x+7,y,z-7,0,0,0);
	insert_collision_vect(x+7,y,z-7,x+7,y,z+7,0,0,0);
	insert_collision_vect(x+7,y,z+7,x-7,y,z+7,0,0,0);
	insert_collision_vect(x-7,y,z+7,x-7,y,z-7,0,0,0);
}

void	cross_vect_point(SLONG x,SLONG y,SLONG z)
{
	insert_collision_vect(x-7,y,z-7,x+7,y,z+7,0,0,0);
	insert_collision_vect(x+7,y,z-7,x-7,y,z+7,0,0,0);
}

void	tri_vect_point(SLONG x,SLONG y,SLONG z)
{
	insert_collision_vect(x,y,z+5,x+5,y,z-5,0,0,0);
	insert_collision_vect(x+5,y,z-5,x-5,y,z-5,0,0,0);
	insert_collision_vect(x-5,y,z-5,x,y,z+5,0,0,0);
}

SLONG	get_height_on_edge(SLONG px,SLONG pz,SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
	SLONG	dx,dy,dz;

	px-=x1;
	pz-=z1;
	
	dx=x2-x1;		
	dy=y2-y1;		
	dz=z2-z1;		
	if(abs(dx)>abs(dz))
	{
		if(dx==0)
			return(y1);
		return(((dy*px)/dx)+y1);
	}
	
	if(dz==0)
		return(y1);

	return(((dy*pz)/dz)+y1);
}

void	highlight_point_on_face(SLONG fx,SLONG fy,SLONG fz,SLONG x,SLONG z,SWORD face)
{
	SLONG	y;
	if(face>0)
	{
		y=fy+prim_points[prim_faces4[face].Points[0]].Y;
		box_vect_point(x,y,z);
		apply_vect_to_face_square(fx,fy,fz,face);
		
	}
}

extern	void	quick_normal(SWORD	face,SLONG *nx,SLONG *ny,SLONG *nz);

void	apply_vect_to_face(SLONG x,SLONG y,SLONG z,SWORD face,SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
	SLONG	x3,y3,z3,x4,y4,z4;
	SLONG	xc[6],yc[6],zc[6],found=0;
	SLONG	i,index[]={0,1,3,2,0};
	SLONG	index3[]={0,1,2,0};
	SLONG	found_pass1;
	SLONG	nx,ny,nz;
	SLONG	flip_flag=0;

//	LogText(" apply vect to face  (%d,%d,%d)->(%d,%d,%d) face %d\n",x1,y1,z1,x2,y2,z2,face);
	quick_normal(face,&nx,&ny,&nz);
	if(ny<0)
		return;

	if(face>0)
	{
		if(prim_faces4[face].DrawFlags&POLY_FLAG_DOUBLESIDED)
			flip_flag=1;
//		if(face==30335)
//			LogText(" checking face %d\n",face);
		if(point_in_quad(x1,z1,x,y,z,face))
		{
			if(point_in_quad(x2,z2,x,y,z,face))
			{
			 //both points in quad

//				y1=get_height_on_face_quad64(x1,z1,face);
				y1=get_height_on_plane_quad_f(x1,z1,face);
				if(y1==0)
					highlight_point_on_face(x,y,z,x1,z1,face);

//				y2=get_height_on_face_quad64(x2,z2,face);
				y2=get_height_on_plane_quad_f(x2,z2,face);
				if(y2==0)
					highlight_point_on_face(x,y,z,x2,z2,face);

				insert_collision_vect(x1,y1,z1,x2,y2,z2,0,flip_flag,0);
//				if(flip_flag)
//					insert_collision_vect(x2,y2,z2,x1,y1,z1,0,1);
//				LogText(" insert vect for face %d \n",face);

				return;
			}
			else
			{

//				y1=get_height_on_face_quad64(x1,z1,face);
				y1=get_height_on_plane_quad_f(x1,z1,face);
				if(y1==0)
					highlight_point_on_face(x,y,z,x1,z1,face);
				xc[found]=x1;
				yc[found]=y1;
				zc[found]=z1;
				found++;

			}
		}
		else
		if(point_in_quad(x2,z2,x,y,z,face))
		{

//			y2=get_height_on_face_quad64(x2,z2,face);
			y2=get_height_on_plane_quad_f(x2,z2,face);
				if(y2==0)
					highlight_point_on_face(x,y,z,x2,z2,face);
			xc[found]=x2;
			yc[found]=y2;
			zc[found]=z2;
			found++;

		}


		found_pass1=found;
		for (i=0;i<4;i++ )
		{
			SLONG	p1,p2;
			p1=index[i];
			p2=index[i+1];
			x3=x+prim_points[prim_faces4[face].Points[p1]].X;
			y3=y+prim_points[prim_faces4[face].Points[p1]].Y;
			z3=z+prim_points[prim_faces4[face].Points[p1]].Z;

			x4=x+prim_points[prim_faces4[face].Points[p2]].X;
			y4=y+prim_points[prim_faces4[face].Points[p2]].Y;
			z4=z+prim_points[prim_faces4[face].Points[p2]].Z;
//			if(face==30335)
//				LogText(" vect (%d,%d)->(%d,%d)   intersect poly edge (%d,%d)->(%d,%d)?\n",x1,z1,x2,z2,x3,z3,x4,z4);


			if(two4_line_intersection(x1-x1,z1-z1,x2-x1,z2-z1,x3-x1,z3-z1,x4-x1,z4-z1))
			{
				SLONG	rx,rz;
//				if(face==9102)
//					rx=0;
//				LogText(" vects intersect found %d\n",found);

				lines_intersect(x1-x1,z1-z1,x2-x1,z2-z1,x3-x1,z3-z1,x4-x1,z4-z1,&rx,&rz);
				xc[found]=rx+x1;
				zc[found]=rz+z1;

//				yc[found]=get_height_on_face_quad64(rx+x1,rz+z1,face);
				yc[found]=get_height_on_edge(rx+x1,rz+z1,x3,y3,z3,x4,y4,z4);

					
				found++;

//				yc[0]=4250;
//				yc[1]=4250;

				if(found-found_pass1>=2)
				{
					insert_collision_vect(xc[found-2],yc[found-2],zc[found-2],xc[found-1],yc[found-1],zc[found-1],0,flip_flag,0);
//					if(flip_flag)
//						insert_collision_vect(xc[found-1],yc[found-1],zc[found-1],xc[found-2],yc[found-2],zc[found-2],0,1);
//					LogText(" insert vect for face %d  (%d,%d,%d)->(%d,%d,%d)\n",face,xc[found-2],yc[found-2],zc[found-2],xc[found-1],yc[found-1],zc[found-1]);
					return;
				}
			}
		}
		if(found>=2)
		{
//			LogText(" funny found pass1 %d total %d \n",found_pass1,found);
			insert_collision_vect(xc[0],yc[0],zc[0],xc[1],yc[1],zc[1],0,flip_flag,0);
//			if(flip_flag)
//				insert_collision_vect(xc[1],yc[1],zc[1],xc[0],yc[0],zc[0],0,0);

//			LogText(" insert vect for face %d  (%d,%d,%d)->(%d,%d,%d)\n",face,xc[0],yc[0],zc[0],xc[1],yc[1],zc[1]);
		}
//		if(found==1)
//			LogText(" error  strange intersect count QUAD\n");

	}
	else
	if(face<0)
	{
		if(prim_faces3[-face].DrawFlags&POLY_FLAG_DOUBLESIDED)
			flip_flag=1;
		if(point_in_tri(x1,z1,x,y,z,-face))
		{
			if(point_in_tri(x2,z2,x,y,z,-face))
			{
				y1=get_height_on_plane_tri_f(x1,z1,-face);
				y2=get_height_on_plane_tri_f(x2,z2,-face);
//				y1=get_height_on_face_tri(x1,z1,-face);
//				y2=get_height_on_face_tri(x2,z2,-face);
				insert_collision_vect(x1,y1,z1,x2,y2,z2,0,flip_flag,0);
//				if(flip_flag)
//					insert_collision_vect(x2,y2,z2,x1,y1,z1,0,0);
				return;
			}
			else
			{
//				y1=get_height_on_face_tri(x1,z1,-face);
				y1=get_height_on_plane_tri_f(x1,z1,-face);
				xc[found]=x1;
				yc[found]=y1;
				zc[found]=z1;
				found++;
			}
		}
		else
		if(point_in_tri(x2,z2,x,y,z,-face))
		{

//			y2=get_height_on_face_tri(x2,z2,-face);
			y2=get_height_on_plane_tri_f(x2,z2,-face);
			xc[found]=x2;
			yc[found]=y2;
			zc[found]=z2;
			found++;
		}

		found_pass1=found;
		for (i=0;i<3;i++ )
		{
			SLONG	p1,p2;
			p1=index3[i];
			p2=index3[i+1];
			x3=x+prim_points[prim_faces3[-face].Points[p1]].X;
			y3=y+prim_points[prim_faces3[-face].Points[p1]].Y;
			z3=z+prim_points[prim_faces3[-face].Points[p1]].Z;

			x4=x+prim_points[prim_faces3[-face].Points[p2]].X;
			y4=y+prim_points[prim_faces3[-face].Points[p2]].Y;
			z4=z+prim_points[prim_faces3[-face].Points[p2]].Z;


			if(two4_line_intersection(x1,z1,x2,z2,x3,z3,x4,z4))
			{
				SLONG	rx,rz;
				lines_intersect(x1-x1,z1-z1,x2-x1,z2-z1,x3-x1,z3-z1,x4-x1,z4-z1,&rx,&rz);
				xc[found]=rx+x1;
				zc[found]=rz+z1;

//				yc[found]=get_height_on_face_tri(rx+x1,rz+z1,-face);
				yc[found]=get_height_on_edge(rx+x1,rz+z1,x3,y3,z3,x4,y4,z4);
					
				found++;

				if(found-found_pass1>=2)
				{
//					insert_collision_vect(xc[0],yc[0],zc[0],xc[1],yc[1],zc[1],0,0);
					insert_collision_vect(xc[found-2],yc[found-2],zc[found-2],xc[found-1],yc[found-1],zc[found-1],0,flip_flag,0);
//					if(flip_flag)
//						insert_collision_vect(xc[found-1],yc[found-1],zc[found-1],xc[found-2],yc[found-2],zc[found-2],0,0);
					return;
				}
			}
		}
		if(found>=2)
		{
//			LogText(" funny found pass1 %d total %d \n",found_pass1,found);
			insert_collision_vect(xc[0],yc[0],zc[0],xc[1],yc[1],zc[1],0,flip_flag,0);
//			if(flip_flag)
//				insert_collision_vect(xc[1],yc[1],zc[1],xc[0],yc[0],zc[0],0,0);
//			LogText(" insert vect for face %d  (%d,%d,%d)->(%d,%d,%d)\n",face,xc[0],yc[0],zc[0],xc[1],yc[1],zc[1]);
			
			
		}
		if(found==1)
			LogText(" error  strange intersect count TRI\n");
		
	}
		
}


#define	SHIFT_BEZ	(10)
#define	BEZ_ONE		(1<<SHIFT_BEZ)
void	clip_face_bez(SLONG x,SLONG y,SLONG z,SWORD face,struct ColInfo *p_col)
{
	SLONG	t=0;
	SLONG	bx,bz;
	SLONG 	px,pz;

	SLONG	ox,oz;
	SLONG	x0,z0,x1,z1,x2,z2,x3,z3;

	static	struct	ColInfo	*old_p_col;

	ox=p_col->Bezier.X[0];
	oz=p_col->Bezier.Z[0];

	x1=p_col->Bezier.X[1]-ox;
	x2=p_col->Bezier.X[2]-ox;
	x3=p_col->Bezier.X[3]-ox;

	z1=p_col->Bezier.Z[1]-oz;
	z2=p_col->Bezier.Z[2]-oz;
	z3=p_col->Bezier.Z[3]-oz;

	x0=0;
	z0=0;
	px=0;
	pz=0;

//	LogText("face %d in Bounding Box  \n",face);


	for(t=BEZ_ONE>>3;t<=BEZ_ONE;t+=BEZ_ONE>>3)
	{

		bx=(((((BEZ_ONE-t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*x0+
		   (((((3*t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*x1+
		   (((((3*t)*(1*t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*x2+
		   ((((t*t)>>SHIFT_BEZ)*t)>>SHIFT_BEZ)*x3;

		bz=(((((BEZ_ONE-t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*z0+
		   (((((3*t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*z1+
		   (((((3*t)*(1*t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*z2+
		   ((((t*t)>>SHIFT_BEZ)*t)>>SHIFT_BEZ)*z3;

		bx>>=SHIFT_BEZ;
		bz>>=SHIFT_BEZ;
		apply_vect_to_face(x,y,z,face ,ox+bx,0,oz+bz,ox+px,0,oz+pz);
//		if(p_col!=old_p_col)
//			insert_collision_vect(ox+bx,4150,oz+bz,ox+px,4150,oz+pz,0,0);
		px=bx;
		pz=bz;
//		draw_3d_line(ox+bx,0,oz+bz,ox+px,0,oz+pz,255);

	}
	old_p_col=p_col;

}

void	clip_a_prim_with_bezier(UWORD	prim,SLONG x,SLONG y,SLONG z,struct ColInfo *p_col)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	c0,c1;
	struct	PrimObject	*p_obj;
	SLONG	point;
	
	SLONG	c_x[12],c_y[12],clip_count=0;

	p_obj    =&prim_objects[prim];
	p_f4     =&prim_faces4[p_obj->StartFace4];
	p_f3     =&prim_faces3[p_obj->StartFace3];

//	LogText(" check prim %d against bEZIER sf4 %d ef4 %d \n",prim,p_obj->StartFace4,p_obj->EndFace4);

	if(p_obj->EndFace4)
	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		clip_count=0;

		for(point=0;point<4;point++)
		{
			if(is_point_in_clip_bez(x+prim_points[p_f4->Points[point]].X,y+prim_points[p_f4->Points[point]].Y,z+prim_points[p_f4->Points[point]].Z))
			{
				
//				LogText("Quad %d is in bounding box\n",c0);
				clip_face_bez(x,y,z,c0,p_col);
				break;
			}
		}

		p_f4++;
	}

	if(p_obj->EndFace3)
	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		clip_count=0;

		for(point=0;point<3;point++)
		{
			if(is_point_in_clip_bez(x+prim_points[p_f3->Points[point]].X,y+prim_points[p_f3->Points[point]].Y,z+prim_points[p_f3->Points[point]].Z))
			{
//				LogText("Tri %d is in bounding box\n",c0);
				clip_face_bez(x,y,z,-c0,p_col);
				break;
			}
		}
skip_face3:;
		p_f3++;
	}
}

SLONG	is_point_in_clip_box(SLONG x,SLONG y,SLONG z,struct ColInfo *p_col)
{

	if(x>p_col->Plane.Left&&x<p_col->Plane.Right&&
		y>p_col->Plane.Top&&y<p_col->Plane.Bottom&&
		z>p_col->Plane.Depth-200&&z<p_col->Plane.Depth+200)
			return(1);
		else
			return(0);	

}

void	clip_a_prim_with_box(UWORD	prim,SLONG x,SLONG y,SLONG z,struct ColInfo *p_col)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	point;
	

	p_obj    =&prim_objects[prim];
	p_f4     =&prim_faces4[p_obj->StartFace4];
	p_f3     =&prim_faces3[p_obj->StartFace3];
//	LogText(" check prim %d against box sf4 %d ef4 %d \n",prim,p_obj->StartFace4,p_obj->EndFace4);

	if(p_obj->EndFace4)
	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		for(point=0;point<4;point++)
		{
			if(is_point_in_clip_box(x+prim_points[p_f4->Points[point]].X,y+prim_points[p_f4->Points[point]].Y,z+prim_points[p_f4->Points[point]].Z,p_col))
			{
				
					SLONG	bx,by,bz,sx,sy,sz,length,c1;
//				LogText("Quad %d is in bounding box\n",c0);

					sx=p_col->Plane.Right-p_col->Plane.Left;
					sy=0;
					sz=0;

					length=sqrl(sx*sx+sy*sy+sz*sz);
					if(length==0)
						length=1;
					if(length<=100)
					{
						apply_vect_to_face(x,y,z,-c0 ,p_col->Plane.Left,0,p_col->Plane.Depth,p_col->Plane.Right,0,p_col->Plane.Depth);
					}
					else
					{
						length=length/100;
						bx=p_col->Plane.Left<<8;
						by=0;//p_col->Plane.Left<<8;
						bz=p_col->Plane.Depth<<8;

						sx=(sx<<8)/length;
						sy=(sy<<8)/length;
						sz=(sz<<8)/length;

						for(c1=0;c1<length;c1++)
						{
							apply_vect_to_face(x,y,z,c0 ,bx>>8,by>>8,bz>>8,(bx+sx)>>8,(by+sy)>>8,(bz+sz)>>8);
							bx+=sx;
							by+=sy;
							bz+=sz;
						}
					}
//				apply_vect_to_face(x,y,z,c0 ,p_col->Plane.Left,0,p_col->Plane.Depth,p_col->Plane.Right,0,p_col->Plane.Depth);
				goto skip_face4;
			}
		}
skip_face4:;
		p_f4++;
	}

	if(p_obj->EndFace3)
	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{

		for(point=0;point<3;point++)
		{
			if(is_point_in_clip_box(x+prim_points[p_f3->Points[point]].X,y+prim_points[p_f3->Points[point]].Y,z+prim_points[p_f3->Points[point]].Z,p_col))
			{
//				LogText("Tri %d is in bounding box\n",c0);
				{
					SLONG	bx,by,bz,sx,sy,sz,length,c1;

					sx=p_col->Plane.Right-p_col->Plane.Left;
					sy=0;
					sz=0;

					length=sqrl(sx*sx+sy*sy+sz*sz);
					if(length==0)
						length=1;
					if(length<=100)
					{
						apply_vect_to_face(x,y,z,-c0 ,p_col->Plane.Left,0,p_col->Plane.Depth,p_col->Plane.Right,0,p_col->Plane.Depth);
					}
					else
					{
						length=length/100;
						bx=p_col->Plane.Left<<8;
						by=0;
						bz=p_col->Plane.Depth<<8;

						sx=(sx<<8)/length;
						sy=(sy<<8)/length;
						sz=(sz<<8)/length;

						for(c1=0;c1<length;c1++)
						{
							apply_vect_to_face(x,y,z,-c0 ,bx>>8,by>>8,bz>>8,(bx+sx)>>8,(by+sy)>>8,(bz+sz)>>8);
							bx+=sx;
							by+=sy;
							bz+=sz;
						}
					}
				}
				goto skip_face3;
			}
		}
skip_face3:;
		p_f3++;
	}
}





void	col_for_map_thing(SLONG	map_thing)
{
	struct	MapThing	*p_mthing;

	p_mthing=TO_MTHING(map_thing);
	switch(p_mthing->Type)
	{
		case	MAP_THING_TYPE_PRIM:
			//3ds Prim Mesh 
			clip_a_prim_at(p_mthing->IndexOther,p_mthing->X,p_mthing->Y,p_mthing->Z,col_data.Depth);
			break;

		case	MAP_THING_TYPE_SPRITE:
		case	MAP_THING_TYPE_AGENT:
			break;

	}
}

void	col_for_backgrounds(struct ColInfo *p_col)
{
	SWORD	index;
	struct	MapThing	*p_thing;
	index=background_prim;
//	LogText(" col for backs, type %d \n",p_col->Type);
	while(index)
	{
//		LogText(" Found a background index %d \n",index);
		p_thing=TO_MTHING(index);
		switch(p_col->Type)
		{
			case	COL_TYPE_PLANE:
//				clip_a_prim_at(p_thing->IndexOther,p_thing->X,p_thing->Y,p_thing->Z,col_data.Depth);
				clip_a_prim_with_box(p_thing->IndexOther,p_thing->X,p_thing->Y,p_thing->Z,p_col);
				break;
			case	COL_TYPE_BEZIER:
				clip_a_prim_with_bezier(p_thing->IndexOther,p_thing->X,p_thing->Y,p_thing->Z,p_col);
				break;
		}
		index=p_thing->IndexNext;
	}
}

#define	SHIFT_BEZ	(10)
#define	BEZ_ONE		(1<<SHIFT_BEZ)

void	calc_bounding_box_for_bezier(struct ColInfo *p_col)
{
	SLONG	t=0;
	SLONG	bx,bz;

	SLONG	ox,oz;
	SLONG	x0,z0,x1,z1,x2,z2,x3,z3;
	SLONG	min_x=9999999,min_z=9999999,max_x=-9999999,max_z=-9999999;
	ox=p_col->Bezier.X[0];
	oz=p_col->Bezier.Z[0];

	x1=p_col->Bezier.X[1]-ox;
	x2=p_col->Bezier.X[2]-ox;
	x3=p_col->Bezier.X[3]-ox;

	z1=p_col->Bezier.Z[1]-oz;
	z2=p_col->Bezier.Z[2]-oz;
	z3=p_col->Bezier.Z[3]-oz;

	x0=0;
	z0=0;


	if(ox<min_x)
		min_x=ox;
	if(ox>max_x)
		max_x=ox;
	if(oz<min_z)
		min_z=oz;
	if(oz>max_z)
		max_z=oz;

	if(x3+ox<min_x)
		min_x=x3+ox;
	if(x3+ox>max_x)
		max_x=x3+ox;
	if(z3+oz<min_z)
		min_z=z3+oz;
	if(z3+oz>max_z)
		max_z=z3+oz;

	

	for(t=BEZ_ONE>>5;t<=BEZ_ONE;t+=BEZ_ONE>>5)
	{

		bx=(((((BEZ_ONE-t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*x0+
		   (((((3*t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*x1+
		   (((((3*t)*(1*t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*x2+
		   ((((t*t)>>SHIFT_BEZ)*t)>>SHIFT_BEZ)*x3;

		bz=(((((BEZ_ONE-t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*z0+
		   (((((3*t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*z1+
		   (((((3*t)*(1*t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*z2+
		   ((((t*t)>>SHIFT_BEZ)*t)>>SHIFT_BEZ)*z3;

		bx>>=SHIFT_BEZ;
		bz>>=SHIFT_BEZ;


	if(bx+ox<min_x)
		min_x=bx+ox;
	if(bx+ox>max_x)
		max_x=bx+ox;
	if(bz+oz<min_z)
		min_z=bz+oz;
	if(bz+oz>max_z)
		max_z=bz+oz;
//		draw_3d_line(ox+bx,0,oz+bz,ox+px,0,oz+pz,255);

	}
	min_x-=100;
	max_x+=100;

	min_z-=100;
	max_z+=100;

//	LogText(" Bounding BEZ Box  left %d right %d near %d far %d \n",min_x,max_x,min_z,max_z);
	col_data.Left=min_x;
	col_data.Right=max_x;
	col_data.Near=min_z;
	col_data.Far=max_z;
	col_data.Flag=1;

	col_data.Top=p_col->Bezier.Top;
	col_data.Bottom=p_col->Bezier.Bottom;

}

void	calc_collision_info(struct ColInfo *p_col)
{
	SLONG	dx,dy,dz;	
	struct	DepthStrip	*p_depth;
	UWORD	index;
	struct	EditMapElement	*p_ele;

	switch(p_col->Type)
	{
		
		case	COL_TYPE_PLANE:
			col_data.Left  =p_col->Plane.Left;
			col_data.Right =p_col->Plane.Right;
			col_data.Top   =p_col->Plane.Top;
			col_data.Bottom=p_col->Plane.Bottom;
			col_data.Depth =p_col->Plane.Depth;
			col_data.Flag=1;
			break;
		case	COL_TYPE_BEZIER:
			calc_bounding_box_for_bezier(p_col);
			break;

	}
/*
	for(dx=0;dx<EDIT_MAP_WIDTH;dx++)
	for(dy=0;dy<EDIT_MAP_HEIGHT;dy++)
	for(dz=0;dz<5;dz++)
	{
		index=edit_map[(dx)][(dy)].MapThingIndex;
		if(index)
			col_for_map_thing(index);

		index=edit_map[dx][dy].Depth[dz];
	   	if(index)
	   	{
			p_ele=&edit_map_eles[index];
			switch(p_ele->CubeType.Prim)
			{
				case	0:
					//error
					break;
				case	CUBE_TYPE_FULL:
					col_for_cube_ele_at((dx)<<ELE_SHIFT,(dy)<<ELE_SHIFT,(dz)<<ELE_SHIFT,p_ele);
					break;

				case	CUBE_TYPE_SLOPE_LR:
					col_for_slope_lr_ele_at((dx)<<ELE_SHIFT,(dy)<<ELE_SHIFT,(dz)<<ELE_SHIFT,p_ele);
					break;

				case	CUBE_TYPE_STEPS_LR:
					col_for_steps_lr_ele_at((dx)<<ELE_SHIFT,(dy)<<ELE_SHIFT,(dz)<<ELE_SHIFT,p_ele);
					break;

				case	CUBE_TYPE_LEDGE1:
					col_for_ledge1_ele_at((dx)<<ELE_SHIFT,(dy)<<ELE_SHIFT,(dz)<<ELE_SHIFT,p_ele);
					break;

			}
			
		}
	}
*/
	col_for_backgrounds(p_col);
}



