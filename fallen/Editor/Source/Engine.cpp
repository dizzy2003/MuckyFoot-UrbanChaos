#include	"editor.hpp"
#include	"edit.h"
#include	"engine.h"
#include	"prim.h"
#include	"c:\fallen\headers\game.h"
#include	"c:\fallen\headers\memory.h"
//#include	"trig.h"


/*
******************************
z is large in the distance, and small near the eye

z>>1 for bucket size


******************************
*/


//#define	my_trig_noz	my_trig
//#define	my_quad_noz	my_quad


//struct	Bucket	buckets[MAX_BUCKET_POOL];
UBYTE	bucket_pool[MAX_BUCKET_POOL];
UWORD	select_colour	=	0xffff;
struct	BucketHead	bucket_heads[MAX_BUCKETS];
UBYTE	*current_bucket_pool=bucket_pool;
UBYTE	*end_bucket_pool=&bucket_pool[MAX_BUCKET_POOL-100];

ULONG (*rotate_point_gte)(struct	SVector *v,struct SVector *r);


struct	EngineStuff	engine;

SLONG	poly_count=0;
SLONG	vect_count=0;
SLONG	small_z=0,big_z=0;

SLONG	view_mode=0;


struct	ExplodeFaces
{
	struct	PrimPoint P[3];
	struct	PrimFace3 Face3;
	SLONG	X,Y,Z;
	SLONG	VX,VY,VZ;
	SLONG	Timer;
	SWORD	AX,AY;
};

#define	MAX_EX_FACE	20
struct	ExplodeFaces	ex_faces[MAX_EX_FACE];
SLONG	next_ex_face=1;
SLONG	debug_info=0;
void	draw_explode_faces(void);

#define	DEBUG_INFO	(1<<0)
#define	DEBUG_FPS	(1<<1)
#define	DEBUG_VECTS	(1<<2)

inline void	rotate_point_old(struct EnginePoint* eptr)								
{																			
	SLONG	x,y,z;

	eptr->X3d-=engine.X>>8;
	eptr->Y3d-=engine.Y>>8;
	eptr->Z3d-=engine.Z>>8;

	x = (eptr->X3d*engine.CosY-eptr->Z3d*engine.SinY)>>16;	// rotate about y 				
	z = (eptr->X3d*engine.SinY+eptr->Z3d*engine.CosY)>>16;

	y = (eptr->Y3d*engine.CosX-z*engine.SinX)>>16;			// rotate about x
	eptr->Z3d=((eptr->Y3d*engine.SinX)+z*engine.CosX)>>16;

	eptr->X3d= x*engine.Scale;
	eptr->Y3d= y*engine.Scale;

	eptr->X = engine.VW2+((eptr->X3d)>>11); //((eptr->X3d*lens)/eptr->Z3d);

	if(eptr->X<0)
	{
		eptr->Flags |=EF_OFF_LEFT;
		if(eptr->X<-2000)
			eptr->X=-2000;

	}
	else if(eptr->X>= WorkWindowWidth)
	{
		if(eptr->X>2000)
			eptr->X=2000;
		eptr->Flags |=EF_OFF_RIGHT;
	}

		eptr->Y = engine.VH2-((eptr->Y3d)>>11); //((eptr->Y3d*lens)/eptr->Z3d);

	if(eptr->Y<0)
	{
		eptr->Flags |=EF_OFF_TOP;
		if(eptr->Y<-2000)
			eptr->Y=-2000;
	}
	else if(eptr->Y>= WorkWindowHeight)
	{
		eptr->Flags |=EF_OFF_BOTTOM;
		if(eptr->Y>2000)
			eptr->Y=2000;
	}

	eptr->Flags |= EF_TRANSLATED;
}

#define	EYE_DIST	0

ULONG	rotate_point_gte_perspective(struct	SVector *v,struct SVector *r)
{
	SLONG	x,y,z,vx,vy,vz;
	ULONG	flags=0;
/*
	if(engine.ClipFlag&ENGINE_CLIPY_FLAG)
	if( v->Y > engine.ClipMaxY||v->Y < engine.ClipMinY)
	{
		flags |=EF_BEHIND_YOU;
		return(flags);
	}
*/
	vx=v->X-(engine.X>>8);
	vy=v->Y-(engine.Y>>8);
	vz=v->Z-(engine.Z>>8);
//	LogText(" into rotate %d %d %d\n",vx,vy,vz);

/*
	if(abs(vx)>4000||abs(vy)>4000)
	{
		flags |=EF_BEHIND_YOU;
		return(flags);
	}
*/


	x = ((vx*engine.CosY-vz*engine.SinY));	// rotate about y 				
	z = (vx*engine.SinY+vz*engine.CosY)>>16;

	y = ((vy*engine.CosX-z*engine.SinX));			// rotate about x
	r->Z=((vy*engine.SinX)+z*engine.CosX)>>16;
/*
	if(engine.ClipFlag&ENGINE_CLIPZ_FLAG)
	if( r->Z > engine.ClipZ)
	{
		flags |=EF_BEHIND_YOU;
		return(flags);
	}
*/
	if(r->Z<=-(EYE_DIST))
	{
		flags |=EF_BEHIND_YOU;
		return(flags);
	}
	else
	{
		SLONG	xdiv,ydiv;
		xdiv=( x/(r->Z+EYE_DIST) );
		ydiv=( y/(r->Z+EYE_DIST) );

		if(abs(xdiv)>(0x7fffffff/2000)||abs(ydiv)>(0x7fffffff/2000))
		{
//			LogText("xdiv/ydiv too big %d,%d \n",xdiv,ydiv);
			flags |=EF_TOO_BIG|EF_BEHIND_YOU;
			return(EF_BEHIND_YOU);
		}

		r->X= engine.VW2+(( xdiv*engine.Scale)>>16);
		r->Y= engine.VH2+(( ydiv*engine.Scale)>>16);
	}

//	LogText(" OUT rotate %d %d (%d)\n",r->X,r->Y,r->Z);

	if(r->X<0)
	{
		flags |=EF_OFF_LEFT;
//		if(r->X<-2000)
//			r->X=-2000;

	}
	else if(r->X>= WorkWindowWidth)
	{
//		if(r->X>2000)
//			r->X=2000;
		flags |=EF_OFF_RIGHT;
	}


	if(r->Y<0)
	{
		flags |=EF_OFF_TOP;
//		if(r->Y<-2000)
//			r->Y=-2000;
	}
	else if(r->Y>= WorkWindowHeight)
	{
		flags |=EF_OFF_BOTTOM;
//		if(r->Y>2000)
//			r->Y=2000;
	}
	flags |= EF_TRANSLATED;
//	LogText(" out %d %d %d flag %x \n",r->X,r->Y,r->Z,flags);
	return(flags);
}

ULONG	rotate_point_gte_normal(struct	SVector *v,struct SVector *r)
{
	SLONG	x,y,z,vx,vy,vz;
	ULONG	flags=0;
/*
	if(engine.ClipFlag&ENGINE_CLIPY_FLAG)
	{
		SLONG	dy;
		dy=(engine.TrueY-engine.Y)>>8;
		
		if( v->Y+dy > engine.ClipMaxY||v->Y+dy < engine.ClipMinY)
		{
			flags |=EF_BEHIND_YOU;
			flags |=EF_OFF_LEFT;
			return(flags);
		}
	}
*/

	if(engine.AngleDY)
	{
		vx=v->X;
		vz=v->Z;

		x = ((vx*engine.CosDY-vz*engine.SinDY)>>16);	// rotate about LOCAL y 				
		z = (vx*engine.SinDY+vz*engine.CosDY)>>16;

		vx=x-(engine.X>>8);
		vz=z-(engine.Z>>8);

	}
	else
	{
		vx=v->X-(engine.X>>8);
		vz=v->Z-(engine.Z>>8);
	}
	vy=v->Y-(engine.Y>>8);

#ifndef	NO_TRANSFORM

	x = ((vx*engine.CosY-vz*engine.SinY));	// rotate about y 				
	z = (vx*engine.SinY+vz*engine.CosY)>>16;

	y = ((vy*engine.CosX-z*engine.SinX));			// rotate about x
	r->Z=((vy*engine.SinX)+z*engine.CosX)>>16;

	{
		x>>=11;
		y>>=11;

		r->X= engine.VW2+(( x*engine.Scale)>>16);
		r->Y= engine.VH2+(( y*engine.Scale)>>16);
	}
#else

	r->Z=((vy*engine.SinX)+vz*engine.CosX)>>16;
	r->X= engine.VW2+(( (vx<<5)*engine.Scale)>>16);
	r->Y= engine.VH2+(( (vy<<5)*engine.Scale)>>16);
#endif
	if(r->X<0)
	{
		flags |=EF_OFF_LEFT;
		if(r->X<-2400)
			r->X=-2400;

	}
	else if(r->X>= WorkWindowWidth)
	{
		if(r->X>2400)
			r->X=2400;
		flags |=EF_OFF_RIGHT;
	}


	if(r->Y<0)
	{
		flags |=EF_OFF_TOP;
		if(r->Y<-2400)
			r->Y=-2400;
	}
	else if(r->Y>= WorkWindowHeight)
	{
		flags |=EF_OFF_BOTTOM;
		if(r->Y>2400)
			r->Y=2400;
	}
	flags |= EF_TRANSLATED;
	return(flags);
}


void	init_engine(void)
{	
	init_camera();
	rotate_point_gte=rotate_point_gte_perspective;
	rotate_point_gte=rotate_point_gte_normal;
	engine.ShowDebug=1;
	engine.BucketSize=MAX_BUCKETS;
	engine.ClipZ=5000;
}

void	add_bucket(void *p_bucket,SLONG z)
{
	struct	BucketQuad	*the_quad;
	struct	BucketTri	*the_tri;

	if(p_bucket<bucket_pool)
	{
//		LogText(" p_bucket oor LOW");
		return;
	}
	if(p_bucket>=end_bucket_pool)
	{
//		LogText(" p_bucket oor HIGH");
		return;
	}
	if(z<small_z)
		small_z=z;
	if(z>big_z)
		big_z=z;


/*
	if(z<500)
	{
//		LogText(" add DIRECT z %d == %d\n",z,z+500);
		z+=500;
		if(z<0)
			z=0;
		if(z>=engine.BucketSize) //only need this for the small bucket mode in editor
			z=engine.BucketSize-1;
		((struct	BucketGeneric*)p_bucket)->BucketPtr=bucket_heads[z].BucketPtr;
		bucket_heads[z].BucketPtr=p_bucket;
		return;
	}
*/


//	LogText(" add bucked z %d ",z);
	z>>=1;

//	z+=875; //(engine.BucketSize>>1);
	z+=(engine.BucketSize>>1);
	if(z>=engine.BucketSize)
		z=engine.BucketSize-1;
	if(z<0)
		z=0;
//	LogText(" == %d \n",z);
	((struct	BucketGeneric*)p_bucket)->BucketPtr=bucket_heads[z].BucketPtr;
	bucket_heads[z].BucketPtr=p_bucket;
}


/* Example bucket use
void	draw_block(UWORD	prev_block,UWORD	c_block)
{
	struct	EnginePoint		*p_start,*p1;
		struct	SVector			res[8];
	SLONG	flags[8],flag;
	SLONG	c0;
	struct	Block			*p_b;
	struct	Bucket			*p_bucket;
		
	p_b=&blocks[prev_block];
	p_bucket=&buckets[next_bucket_pool];

	for(c0=0;c0<4;c0++)
	{
		flags[c0]=rotate_point_gte(&p_b->V[c0],&res[c0]);
	}

	p_b=&blocks[c_block];
	for(c0=0;c0<4;c0++)
	{
		flags[c0+4]=rotate_point_gte(&p_b->V[c0],&res[c0+4]);
	}

	//we now have all 8 points transformed to screen co-ords
	//and we have the flag stored so we can do some clipping

//lid
	flag=flags[0]&flags[1]&flags[5]&flags[4];
	if(!(flag & EF_CLIPFLAGS))
	{
		p_bucket->Type=BT_QUAD;
		p_bucket->DrawType=VM_GT;
		setXY4(p_bucket,res[0].X,res[0].Y,res[1].X,res[1].Y,res[5].X,res[5].Y,res[4].X,res[4].Y);
		setUV4(p_bucket,0,0,64,0,64,64,0,64);
		setShade4(p_bucket,255,255,255,255);
		add_face4(p_bucket++,res[0].Z);
	}
}
*/

void	draw_quad(struct	MfEnginePoint *p1,struct	MfEnginePoint *p2,struct	MfEnginePoint *p3,struct	MfEnginePoint *p4)
{
//	vec_mode=VM_GT;

	
		my_trig(p1,p2,p3);
		my_trig(p3,p4,p1);
}


void	draw_quad_anti(struct	MfEnginePoint *p1,struct	MfEnginePoint *p2,struct	MfEnginePoint *p3,struct	MfEnginePoint *p4)
{
//	poly_count+=2;

	{
//		my_trig(p3,p2,p1);
//		my_trig(p1,p4,p3);

		

		my_trig(p1,p2,p3);
		my_trig(p4,p3,p2);
	}

}


//change poly draw to understand buckets directly
void	draw_quad_bucket(struct BucketQuad *p_b,SLONG z)
{
	struct EnginePoint	p1,p2,p3,p4;

	if(p_b->DrawFlags&POLY_FLAG_TEXTURED)
	{
//		if(p_b->TextPage>5)
//			LogText(" strange textpage %d \n",p_b->TextPage);
		if(p_b->TextPage>25+8)
			p_b->TextPage=0;
		poly_info.PTexture=tmaps[p_b->TextPage]; //OOR
		poly_info.Page=p_b->TextPage;
	}
	if(edit_info.FlatShade)
	{
		p_b->DrawFlags&=~POLY_FLAG_GOURAD;
	}

	poly_info.DrawFlags=p_b->DrawFlags;
	poly_info.Col=p_b->Col;
//	LogText(" draw quad %d \n",poly_count);
	poly_count+=2;
	if(Keys[KB_TAB])
		goto	wire;
	//if(!ShiftFlag)
/*
//
// these are old poly draw types, no longer supported
//
	{
		if(view_mode&2)
		{
			if(p_b->DrawFlags&POLY_FLAG_DOUBLESIDED)
			{
				if(Keys[KB_P])
				{
					my_trigp(&p_b->P[2],&p_b->P[1],&p_b->P[0]);
					my_trigp(&p_b->P[1],&p_b->P[2],&p_b->P[3]);
				}
				else
				{
					my_trig(&p_b->P[2],&p_b->P[1],&p_b->P[0]);
					my_trig(&p_b->P[1],&p_b->P[2],&p_b->P[3]);
				}

			}
			if(Keys[KB_P])
			{
				my_trigp(&p_b->P[0],&p_b->P[1],&p_b->P[2]);
				my_trigp(&p_b->P[3],&p_b->P[2],&p_b->P[1]);
			}
			else
			{
				my_trig(&p_b->P[0],&p_b->P[1],&p_b->P[2]);
				my_trig(&p_b->P[3],&p_b->P[2],&p_b->P[1]);
			}
		}
		else
		{
			if(p_b->DrawFlags&POLY_FLAG_DOUBLESIDED)
			{
				if(view_mode&1)		
					my_quad(&p_b->P[2],&p_b->P[3],&p_b->P[1],&p_b->P[0]);
				else
					my_quad_noz(&p_b->P[2],&p_b->P[3],&p_b->P[1],&p_b->P[0]);
				
			}
			if(view_mode&1)
				my_quad(&p_b->P[0],&p_b->P[1],&p_b->P[3],&p_b->P[2]);
			else
				my_quad_noz(&p_b->P[0],&p_b->P[1],&p_b->P[3],&p_b->P[2]);
		}
	}
	*/

	if(p_b->DebugFlags&FACE_FLAG_NON_PLANAR)

	{
		// 0   1
		//
		// 2   3

		if(p_b->DebugFlags&FACE_FLAG_OTHER_SPLIT)
		{
			my_trig_noz(&p_b->P[0],&p_b->P[1],&p_b->P[3]);
			my_trig_noz(&p_b->P[3],&p_b->P[2],&p_b->P[0]);
		}
		else
		{
			my_trig_noz(&p_b->P[0],&p_b->P[1],&p_b->P[2]);
			my_trig_noz(&p_b->P[1],&p_b->P[3],&p_b->P[2]);
		}
	}
	else
	{
		if(p_b->DrawFlags&POLY_FLAG_DOUBLESIDED)
		{
			my_quad_noz(&p_b->P[2],&p_b->P[3],&p_b->P[1],&p_b->P[0]);
		}
		{
			my_quad_noz(&p_b->P[0],&p_b->P[1],&p_b->P[3],&p_b->P[2]);
		}
	}



	if(p_b->DebugFlags&FACE_FLAG_OUTLINE)
	{
wire:;		 
		DrawLineC(p_b->P[0].X,p_b->P[0].Y,p_b->P[1].X,p_b->P[1].Y,255);
		DrawLineC(p_b->P[1].X,p_b->P[1].Y,p_b->P[3].X,p_b->P[3].Y,255);
		DrawLineC(p_b->P[3].X,p_b->P[3].Y,p_b->P[2].X,p_b->P[2].Y,255);
		DrawLineC(p_b->P[2].X,p_b->P[2].Y,p_b->P[0].X,p_b->P[0].Y,255);
		return;
	}

/*
	if(p_b->DebugInfo)
	{

		CBYTE	str1[10];
		CBYTE	str2[10];
		CBYTE	str3[10];
		SLONG 	x,y;
		sprintf(str1,"f%d",p_b->DebugInfo);
		sprintf(str2,"W%d",prim_faces4[p_b->DebugInfo].ThingIndex);
		sprintf(str3,"S%d",wall_list[-prim_faces4[p_b->DebugInfo].ThingIndex].StoreyHead);
		x=(p_b->P[0].X+p_b->P[1].X+p_b->P[2].X+p_b->P[3].X)>>2;
		y=(p_b->P[0].Y+p_b->P[1].Y+p_b->P[2].Y+p_b->P[3].Y)>>2;
		
		if(prim_faces4[p_b->DebugInfo].ThingIndex<0)
		if(y>0&&y<WorkWindowHeight-16)
		{
			QuickTextC(x,y,str1,0);
			QuickTextC(x+1,y+1,str1,255);
			QuickTextC(x,y+15,str2,0);
			QuickTextC(x+1,y+1+15,str2,255);
			QuickTextC(x,y+30,str3,0);
			QuickTextC(x+1,y+1+30,str3,255);
		}
	}
*/


//	draw_quad_anti(&p1,&p2,&p3,&p4);
#ifdef	DEBUG_POOSHIT
	//if(p_b->DebugInfo==28334)
//	if(ShiftFlag)
	{
		CBYTE	str[100];
		SLONG 	x,y;
		sprintf(str,"%d",p_b->DebugInfo);
		x=(p_b->P[0].X+p_b->P[1].X+p_b->P[2].X+p_b->P[3].X)>>2;
		y=(p_b->P[0].Y+p_b->P[1].Y+p_b->P[2].Y+p_b->P[3].Y)>>2;
		if(y>0&&y<WorkWindowHeight-16&&x>0&&x<320)
		{
			QuickTextC(x,y,str,0);
			QuickTextC(x+1,y+1,str,255);
		}
	}
#endif
}

void	draw_tri_bucket(struct BucketTri *p_b)
{
	struct EnginePoint	p1,p2,p3;
	if(p_b->DrawFlags&POLY_FLAG_TEXTURED)
	{
//		if(p_b->TextPage>5)
//			LogText(" strange textpage %d \n",p_b->TextPage);
		if(p_b->TextPage>25+8)
			p_b->TextPage=0;

		poly_info.PTexture=tmaps[p_b->TextPage]; //OOR
		poly_info.Page=p_b->TextPage;
	}
	if(edit_info.FlatShade)
	{
		p_b->DrawFlags&=~POLY_FLAG_GOURAD;
	}
	poly_info.DrawFlags=p_b->DrawFlags;
	poly_info.Col=p_b->Col;

	poly_count++;

	if(Keys[KB_TAB])
	{
		goto wire;
	}
//	LogText(" draw tri %d \n",poly_count);
/*
	p1.X=p_b->SX[0];
	p2.X=p_b->SX[1];
	p3.X=p_b->SX[2];

	p1.Y=p_b->SY[0];
	p2.Y=p_b->SY[1];
	p3.Y=p_b->SY[2];

	p1.Shade=p_b->Shade[0]<<14;
	p2.Shade=p_b->Shade[1]<<14;
	p3.Shade=p_b->Shade[2]<<14;

	p1.TMapX=p_b->TX[0];
	p2.TMapX=p_b->TX[1];
	p3.TMapX=p_b->TX[2];		


	p1.TMapY=p_b->TY[0];
	p2.TMapY=p_b->TY[1];
	p3.TMapY=p_b->TY[2];
	my_trig(&p1,&p2,&p3);
*/

	if(!(view_mode&1))
	{
		if(p_b->DrawFlags&POLY_FLAG_DOUBLESIDED)
			my_trig_noz(&p_b->P[2],&p_b->P[1],&p_b->P[0]);
		my_trig_noz(&p_b->P[0],&p_b->P[1],&p_b->P[2]);
/*
		DrawLineC(p_b->P[0].X,p_b->P[0].Y,p_b->P[1].X,p_b->P[1].Y,255);
		DrawLineC(p_b->P[1].X,p_b->P[1].Y,p_b->P[2].X,p_b->P[2].Y,255);
		DrawLineC(p_b->P[2].X,p_b->P[2].Y,p_b->P[0].X,p_b->P[0].Y,255);
*/
	}
	if(p_b->DebugFlags&FACE_FLAG_OUTLINE)
	{
wire:;		 
		DrawLineC(p_b->P[0].X,p_b->P[0].Y,p_b->P[1].X,p_b->P[1].Y,255);
		DrawLineC(p_b->P[1].X,p_b->P[1].Y,p_b->P[2].X,p_b->P[2].Y,255);
		DrawLineC(p_b->P[2].X,p_b->P[2].Y,p_b->P[0].X,p_b->P[0].Y,255);
		return;
	}
	/*
	if(prim_faces3[p_b->DebugInfo].FaceFlags&FACE_FLAG_OUTLINE)
	{
wire:;
		DrawLineC(p_b->P[0].X,p_b->P[0].Y,p_b->P[1].X,p_b->P[1].Y,255);
		DrawLineC(p_b->P[1].X,p_b->P[1].Y,p_b->P[2].X,p_b->P[2].Y,255);
		DrawLineC(p_b->P[2].X,p_b->P[2].Y,p_b->P[0].X,p_b->P[0].Y,255);
		return;
	}
	*/

	if(0)
	{
		CBYTE	str[100];
		SLONG 	x,y;
		if(p_b->DebugInfo>0)
			sprintf(str," z=%d",p_b->DebugInfo);
		else
			sprintf(str," z=N%d",p_b->DebugInfo);
		x=(p1.X+p2.X+p2.X+p3.X)/3;
		y=(p1.Y+p2.Y+p2.Y+p3.Y)/3;
		if(x>0&&x<WorkWindowWidth-16)
		if(y>0&&y<WorkWindowHeight-16)
		{
			QuickTextC(x,y,str,0);
			QuickTextC(x+1,y+1,str,1);
		}
	}
}



void	draw_vect_bucket(struct BucketVect *p_b)
{
//	DrawLineC(p_b->SX[0],p_b->SY[0],p_b->SX[1],p_b->SY[1],p_b->Shade[0]);
	vect_count++;
	DrawLineC(p_b->P[0].X,p_b->P[0].Y,p_b->P[1].X,p_b->P[1].Y,p_b->Col);

}
void	draw_rect_bucket(struct BucketRect *p_b)
{
	DrawBoxC(p_b->P[0].X,p_b->P[0].Y,p_b->Width,p_b->Height,p_b->Col);
}

void	hilite_a_floor_face(SWORD face,UBYTE info_flag)
{
	struct	SVector	point,res[4];
	SLONG	dx,dy,dz;

	dx=selected_prim_xyz.X;
	dy=selected_prim_xyz.Y;
	dz=selected_prim_xyz.Z;

	point.X=dx*ELE_SIZE;
	point.Y=0;
	point.Z=(dz*ELE_SIZE); //(engine.Z>>8);
	rotate_point_gte(&point,&res[0]);

	point.X=(dx+1)*ELE_SIZE;
	point.Y=0;
	point.Z=(dz*ELE_SIZE); //(engine.Z>>8);
	rotate_point_gte(&point,&res[1]);

	point.X=(dx+1)*ELE_SIZE;
	point.Y=0;
	point.Z=((dz+1)*ELE_SIZE); //(engine.Z>>8);
	rotate_point_gte(&point,&res[2]);

	point.X=(dx)*ELE_SIZE;
	point.Y=0;
	point.Z=((dz+1)*ELE_SIZE); //(engine.Z>>8);
	rotate_point_gte(&point,&res[3]);

	DrawLineC(res[0].X,res[0].Y,res[1].X,res[1].Y,select_colour);
	DrawLineC(res[1].X,res[1].Y,res[2].X,res[2].Y,select_colour);
	DrawLineC(res[2].X,res[2].Y,res[3].X,res[3].Y,select_colour);
	DrawLineC(res[3].X,res[3].Y,res[0].X,res[0].Y,select_colour);
	
}

void	hilite_a_face(SWORD face,UBYTE info_flag)
{
	UWORD			*points;
	SLONG			flags[4];
	struct SVector	res[4];
	SLONG			ox,oy,oz,c0;
	struct SVector	norm;
	SLONG	flag_and,flag_or;
	CBYTE	str[100];

	SLONG	wall;
	SLONG	storey;
	SLONG	building;
	SLONG	thing;

	Thing *p_thing;
	if(face<0)
	{
		points	=	prim_faces3[-face].Points;

		wall = prim_faces3[-face].ThingIndex;

		if (wall < 0)
		{
			storey   = wall_list[-wall].StoreyHead;
			building = storey_list[storey].BuildingHead;
			thing    = building_list[building].ThingIndex;

			p_thing = TO_THING(thing);

			ox = p_thing->WorldPos.X >> 8;
			oy = p_thing->WorldPos.Y >> 8;
			oz = p_thing->WorldPos.Z >> 8;

		}
		else
		{
			Thing	*p_thing;

			p_thing=TO_THING(wall);
			ox=p_thing->WorldPos.X>>8;
			oy=p_thing->WorldPos.Y>>8;
			oz=p_thing->WorldPos.Z>>8;
		}

		engine.X	-=	ox<<8;
		engine.Y	-=	oy<<8;
		engine.Z	-=	oz<<8;
		for(c0=0;c0<3;c0++)
		{
			struct	SVector	pp;
			pp.X=prim_points[*(points+c0)].X;
			pp.Y=prim_points[*(points+c0)].Y;
			pp.Z=prim_points[*(points+c0)].Z;

			flags[c0]	=	rotate_point_gte(&pp,&res[c0]);
		}
		engine.X	+=	ox<<8;
		engine.Y	+=	oy<<8;
		engine.Z	+=	oz<<8;



		flag_and = flags[0]&flags[1];	
		flag_or = flags[0]|flags[1];	
		if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
				DrawLineC(res[0].X,res[0].Y,res[1].X,res[1].Y,select_colour);

		flag_and = flags[1]&flags[2];	
		flag_or = flags[1]|flags[2];	
		if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
				DrawLineC(res[1].X,res[1].Y,res[2].X,res[2].Y,select_colour);

		flag_and = flags[0]&flags[2];	
		flag_or = flags[0]|flags[2];	
		if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
				DrawLineC(res[2].X,res[2].Y,res[0].X,res[0].Y,select_colour);

		if(ShiftFlag)
		if(info_flag)
		{
			sprintf(str,"TRI Face %d",-face);
			QuickTextC(30,31,str,0);
			QuickTextC(31,30,str,0);
			QuickTextC(30,30,str,255);
			
		}
/*
		calc_normal(face,&norm);
		sprintf(str,"NORT F %d %d,%d,%d",face,norm.X,norm.Y,norm.Z);
		QuickTextC(res[0].X+1,res[0].Y,str,0);
		QuickTextC(res[0].X,res[0].Y+1,str,0);
		QuickTextC(res[0].X,res[0].Y,str,255);
*/

	}
	else
	{
		points	=	prim_faces4[face].Points;

		thing=prim_faces4[face].ThingIndex;
		if(thing<0)
		{
			thing=wall_list[-thing].StoreyHead;
			thing=storey_list[thing].BuildingHead;
			thing=building_list[thing].ThingIndex;

			Thing *p_thing = TO_THING(thing);

			ox = p_thing->WorldPos.X >> 8;
			oy = p_thing->WorldPos.Y >> 8;
			oz = p_thing->WorldPos.Z >> 8;

		}
		else
		{
			ox=map_things[prim_faces4[face].ThingIndex].X;
			oy=map_things[prim_faces4[face].ThingIndex].Y;
			oz=map_things[prim_faces4[face].ThingIndex].Z;
		}

		engine.X	-=	ox<<8;
		engine.Y	-=	oy<<8;
		engine.Z	-=	oz<<8;
		for(c0=0;c0<4;c0++)
		{
			struct	SVector	pp;
			pp.X=prim_points[*(points+c0)].X;
			pp.Y=prim_points[*(points+c0)].Y;
			pp.Z=prim_points[*(points+c0)].Z;

			flags[c0]	=	rotate_point_gte(&pp,&res[c0]);
		}
		engine.X	+=	ox<<8;
		engine.Y	+=	oy<<8;
		engine.Z	+=	oz<<8;



		flag_and = flags[0]&flags[1];	
		flag_or = flags[0]|flags[1];	
		if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
				DrawLineC(res[0].X,res[0].Y,res[1].X,res[1].Y,select_colour);

		flag_and = flags[1]&flags[3];	
		flag_or = flags[1]|flags[3];	
		if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
				DrawLineC(res[1].X,res[1].Y,res[3].X,res[3].Y,select_colour);

		flag_and = flags[3]&flags[2];	
		flag_or = flags[3]|flags[2];	
		if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
				DrawLineC(res[2].X,res[2].Y,res[3].X,res[3].Y,select_colour);
		DrawLineC(res[2].X,res[2].Y,res[0].X,res[0].Y,select_colour);

		flag_and = flags[0]&flags[2];	
		flag_or = flags[0]|flags[2];	
		if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
				DrawLineC(res[2].X,res[2].Y,res[0].X,res[0].Y,select_colour);

		if(ShiftFlag)
		if(info_flag)
		{
			sprintf(str,"QUAD Face %d",face);
			QuickTextC(30,31,str,0);
			QuickTextC(31,30,str,0);
			QuickTextC(30,30,str,255);
			
		}

//			calc_normal(face,&norm);
//			sprintf(str,"NORQ F %d %d,%d,%d",face,norm.X,norm.Y,norm.Z);
//			sprintf(str,"NORQ F %d ",face);
//			QuickTextC(res[0].X+1,res[0].Y,str,0);
//			QuickTextC(res[0].X,res[0].Y+1,str,0);
//			QuickTextC(res[0].X,res[0].Y,str,255);
	}
}

void	render_buckets(UBYTE highlight)
{
	SLONG	c0;
	struct	BucketHead	*p;
	struct	BucketQuad	*the_quad;
	struct	BucketTri	*the_tri;
	void	*bucket;
	CBYTE	str[100];

	if(view_mode&8)
		p=&bucket_heads[0];
	else
		p=&bucket_heads[engine.BucketSize-1];

	for(c0=0;c0<engine.BucketSize;c0++)
	{
		bucket=p->BucketPtr;
		{
			SLONG	count=0;
			p->BucketPtr=0;
			while(bucket)
			{
				UWORD	temp;
				count++;
				switch(((struct BucketGeneric*)bucket)->BucketType)
				{
					case	BT_QUAD:
							draw_quad_bucket((struct BucketQuad*)bucket,engine.BucketSize-c0);
						break;
					case	BT_TRI:
							draw_tri_bucket((struct BucketTri*)bucket);
						break;
					case	BT_VECT:
							draw_vect_bucket((struct BucketVect*)bucket);
						break;
					case	BT_RECT:
							draw_rect_bucket((struct BucketRect*)bucket);
						break;

				}			
				bucket=((struct BucketGeneric*)bucket)->BucketPtr;
			}
		}
	if(view_mode&8)
		p++;
	else
		p--;
	}


	if(highlight)
	{
/*		
		if(next_face_selected>1)
		{
			SLONG	c0;
			for(c0=0;c0<next_face_selected;c0++)
			{
				hilite_a_face(face_selected_list[c0],0);
			}
		}
		else
*/
		if(hilited_face.PEle==(struct EditMapElement*)-2)
		{
			hilite_a_floor_face(0,0);
		}
		else
		{

			if(hilited_face.Face)
				hilite_a_face(hilited_face.Face,1);

			if(selected_face.Face)
				hilite_a_face(selected_face.Face,0);
		}
	}

	if(ShiftFlag||(engine.ClipFlag&ENGINE_CLIPZ_FLAG)||(debug_info&DEBUG_INFO))
	if(engine.ShowDebug)
	{
	sprintf(str," pc=%d ",poly_count);
	QuickTextC(10,10,str,0);
	QuickTextC(11,11,str,255);

		{
			
			if((engine.ClipFlag&ENGINE_CLIPZ_FLAG))
				sprintf(str," CLIP %d sc %d pc=%d  ex %d ey %d ez %d ax %d ay %d az %d\n",engine.ClipZ,engine.Scale,poly_count,engine.X>>8,engine.Y>>8,engine.Z>>8,engine.AngleX,engine.AngleY,engine.AngleZ);
			else
				sprintf(str,"sc %d pc=%d  ex %d ey %d ez %d ax %d ay %d az %d\n",engine.Scale,poly_count,engine.X>>8,engine.Y>>8,engine.Z>>8,engine.AngleX,engine.AngleY,engine.AngleZ);
			QuickTextC(10,10,str,0);
			QuickTextC(11,11,str,255);
		}
//		draw_text(0,0,str,32);
	}
	if(Keys[KB_D])
	{
		Keys[KB_D]=0;
		debug_info++;
		
	}
//	LogText(" 0 it \n");
	poly_count=0;
	vect_count=0;
	current_bucket_pool=bucket_pool;
}


void	render_view(UBYTE highlight)
{
	static	SLONG			prev_time;
	struct MFTime 	the_time;
	draw_explode_faces();

//	LogText(" render view scale %d enginex %d \n",engine.Scale,engine.X);
	switch(WorkScreenDepth)
	{
		case 1:
			render_span=render_span8;
			break;
		case 2:
			render_span=render_span16;
			break;
		case 4:
			render_span=render_span32;
			break;
		
	}
//	draw_blocks(1);

	if(Keys[KB_F2]&&ShiftFlag)
	{
		Keys[KB_F2]=0;
		view_mode^=1;
	}
	if(Keys[KB_F5]&&ShiftFlag)
	{
		Keys[KB_F5]=0;
		view_mode^=8;
	}
	if(Keys[KB_F3]&&ShiftFlag)
	{
		Keys[KB_F3]=0;
		view_mode^=2;
		if((view_mode&3)==3)
			view_mode&=0xfffffffe;

	}
	if(Keys[KB_F4]&&ShiftFlag)
	{
		Keys[KB_F4]=0;
		view_mode^=4;
		if(view_mode&4)
			rotate_point_gte=rotate_point_gte_perspective;
		else
			rotate_point_gte=rotate_point_gte_normal;
	}
	Time(&the_time);
#ifdef	TEST_3DFX
void	render_buckets_3dfx(UBYTE highlight);
	render_buckets_3dfx(highlight);
#else
	render_buckets(highlight);
#endif
//	if(view_mode&1)
		draw_all_spans();

	if((engine.ShowDebug)) //||(ShiftFlag))
	{
		CBYTE	str[100];
		static	SLONG	times[10],turn=0;
		SLONG	time;
		turn++;
		Time(&the_time);
		time=the_time.Ticks;
		while((time-prev_time)<31)
		{
			Time(&the_time);
			time=the_time.Ticks;
		}
		times[turn%10]=time-prev_time;

		{
			SLONG	c0;
			time=0;
			for(c0=0;c0<10;c0++)
				time+=times[c0];
//			time/=10;
		}
		time=(time<<8)/10; //time for an averaged frame
		if(time==0)
			time=1;
		time=(1000<<8)/time; 
		if(debug_info&DEBUG_FPS)
		{
			sprintf(str," FPS %d sz %d bz %d BS %d ax %d ay %d",time,small_z,big_z,engine.BucketSize,engine.AngleX>>8,engine.AngleY>>8);
			QuickTextC(0,23,str,255);
			small_z=100000;
			big_z=-100000;
		}
/*


		prev_time=time;
		
		if(!(view_mode&2))
			QuickTextC(10,75,"PERSPECTIVE CORRECT",255); 
		if((view_mode&4))
			QuickTextC(10,90,"PERSPECTIVE VIEW",255); 
*/
	}
	prev_time=the_time.Ticks;
//	LogText(" after render view scale %d enginex %d \n",engine.Scale,engine.X);
}


void	init_camera(void)
{
	engine.AngleX=1070<<8;
	engine.AngleY=0;
	engine.AngleZ=0;
	engine.Scale=2688;

	engine.X=0;
	engine.Y=0;
	engine.Z=0;
	set_camera();
}

void	set_camera_to_base(void)
{
	engine.VH2=engine.VH;
}

void	set_camera_to_mid(void)
{
	engine.VH2=engine.VH>>1;
}

void	set_camera(void)
{

	engine.VW=WorkWindowWidth;
	engine.VH=WorkWindowHeight;

	engine.VW2=engine.VW>>1;
	engine.VH2=engine.VH>>1;


	engine.CosY=COS((engine.AngleY>>8)&2047);
	engine.SinY=SIN((engine.AngleY>>8)&2047);

	engine.CosX=COS((engine.AngleX>>8)&2047);
	engine.SinX=SIN((engine.AngleX>>8)&2047);

	engine.CosZ=COS((engine.AngleZ>>8)&2047);
	engine.SinZ=SIN((engine.AngleZ>>8)&2047);
}

void	set_camera_angledy(SWORD angle)
{

	if(angle)
	{
		angle=(angle+2048)&2047;
//		LogText("set camera angledy %d \n",angle);

		engine.CosDY=COS(2048-angle);
		engine.SinDY=SIN(2048-angle);
		engine.AngleDY=angle;
	}
	else
		engine.AngleDY=0;
}

void	clear_camera_angledy(void)
{
//		LogText("CLEAR camera angledy \n");
	engine.AngleDY=0;
}

void	set_camera_plan(void)
{
	SLONG	angle;

	engine.VW=WorkWindowWidth;
	engine.VH=WorkWindowHeight;

	engine.VW2=engine.VW>>1;
	engine.VH2=engine.VH>>1;


	engine.CosY=COS(0);
	engine.SinY=SIN(0);

	engine.CosX=COS(512+1024);
	engine.SinX=SIN(512+1024);

	engine.CosZ=COS(0);
	engine.SinZ=SIN(0);
}

void	set_camera_front(void)
{
	SLONG	angle;

	engine.VW=WorkWindowWidth;
	engine.VH=WorkWindowHeight;

	engine.VW2=engine.VW>>1;
	engine.VH2=engine.VH>>1;


	engine.CosY=COS(0);
	engine.SinY=SIN(0);

	engine.CosX=COS(1024);
	engine.SinX=SIN(1024);

	engine.CosZ=COS(0);
	engine.SinZ=SIN(0);
}

void	set_camera_side(void)
{
	SLONG	angle;

	engine.VW=WorkWindowWidth;
	engine.VH=WorkWindowHeight;

	engine.VW2=engine.VW>>1;
	engine.VH2=engine.VH>>1;


	engine.CosY=COS(512+1024);
	engine.SinY=SIN(512+1024);

	engine.CosX=COS(1024);
	engine.SinX=SIN(1024);

	engine.CosZ=COS(0);
	engine.SinZ=SIN(0);
}

//mousex,0,mousey
/*
void	reverse_transform(SLONG x,SLONG y,SLONG z,SLONG *out_x,SLONG *out_y,SLONG *out_z)
{
	if(sint==0)
		return;
	x = ((x-view_width_over_2)<<11)/overall_scale;
	z = ((z-view_height_over_2)<<11)/overall_scale;



	z=-((z<<16)/sint); // I dont know how this formulae works, its crazy

	*out_x = (x*cosa-z*sina)>>16;	// rotate about y 				
	*out_z = (x*sina+z*cosa)>>16;

	*out_y=-y;
	*out_z=-*out_z;
}
*/


void	calc_world_pos_plan(SLONG x,SLONG y)
{
	SLONG	temp_x,temp_z;
	if(engine.Scale)
	{
		temp_x=(((x-engine.VW2)<<16)/engine.Scale);
		temp_z=(((y-engine.VH2)<<16)/engine.Scale);
		engine.MousePosX=(engine.X>>8)+(temp_x>>5);
		engine.MousePosZ=(engine.Z>>8)+(temp_z>>5);
	}

}

void	calc_world_pos_front(SLONG x,SLONG y)
{
	SLONG	temp_x,temp_y;

	if(engine.Scale)
	{
		temp_x=(((x-engine.VW2)<<16)/engine.Scale);
		temp_y=((-(y-engine.VH2)<<16)/engine.Scale);
		engine.MousePosX=(engine.X>>8)+(temp_x>>5);
		engine.MousePosY=(engine.Y>>8)+(temp_y>>5);
	}
}


void	dump_face_info(SWORD face)
{
	struct	MapThing	*p_thing;
	SLONG	x,y,z;
	SLONG	c0;
	SWORD	index[]={0,1,3,2,0};
	
	if(face>0)
	{
		struct	PrimFace4 *p_face;
		p_face=&prim_faces4[face];
		p_thing=TO_MTHING(p_face->ThingIndex);
		x=p_thing->X;
		y=p_thing->Y;
		z=p_thing->Z;
		LogText("QUAD face %d  x %d y %d z %d df %x\n",face,x,y,z,p_face->DrawFlags);
		LogText("UV %d,%d %d,%d %d,%d %d,%d",p_face->UV[0][0],p_face->UV[0][1],p_face->UV[1][0],p_face->UV[1][1],p_face->UV[2][0],p_face->UV[2][1],p_face->UV[3][0],p_face->UV[3][1]);
		for(c0=0;c0<4;c0++)
		{
			LogText(" (%d)=",p_face->Points[index[c0]]);
			LogText(" (%d,%d,%d)",prim_points[p_face->Points[index[c0]]].X,prim_points[p_face->Points[index[c0]]].Y,prim_points[p_face->Points[index[c0]]].Z);
		}
	}
	else
	{
		struct	PrimFace3 *p_face;
		face=-face;
		p_face=&prim_faces3[face];
		p_thing=TO_MTHING(p_face->ThingIndex);
		x=p_thing->X;
		y=p_thing->Y;
		z=p_thing->Z;
		LogText("TRI face %d  x %d y %d z %d df %x\n",face,x,y,z,p_face->DrawFlags);
		LogText("UV %d,%d %d,%d %d,%d ",p_face->UV[0][0],p_face->UV[0][1],p_face->UV[1][0],p_face->UV[1][1],p_face->UV[2][0],p_face->UV[2][1]);

		for(c0=0;c0<3;c0++)
		{
			LogText(" (%d)=",p_face->Points[c0]);
			LogText(" (%d,%d,%d) ",prim_points[p_face->Points[c0]].X,prim_points[p_face->Points[c0]].Y,prim_points[p_face->Points[c0]].Z);
		}
		
	}

	LogText("\n");

}

void	calc_txty(SWORD face,SLONG *tx,SLONG *ty,SWORD mid_x,SWORD mid_y,SWORD mid_z)
{

	if(face>0)
	{
		*tx=(prim_faces4[face].UV[0][0]+prim_faces4[face].UV[1][0]+prim_faces4[face].UV[2][0]+prim_faces4[face].UV[3][0])>>2;
		*ty=(prim_faces4[face].UV[0][1]+prim_faces4[face].UV[1][1]+prim_faces4[face].UV[2][1]+prim_faces4[face].UV[3][1])>>2;
	}
}

void	rotate_point_by_xyz(struct SVector *p,SLONG ax,SLONG ay,SLONG az)
{
	SLONG	cosy,siny,cosx,sinx,cosz,sinz;
	SLONG	rx,ry,rz;

	ax=(ax+2048)&2047;
	ay=(ay+2048)&2047;
	az=(az+2048)&2047;

	cosx=COS(ax);
	sinx=SIN(ax);
	cosy=COS(ay);
	siny=SIN(ay);
	cosz=COS(az);
	sinz=SIN(az);

	rx=(p->X*cosy-p->Z*siny)>>16;
	rz=(p->X*siny+p->Z*cosy)>>16;

	ry=(p->Y*cosx-rz*sinx)>>16;
	rz=(p->Y*sinx+rz*cosx)>>16;
	p->X=rx;
	p->Y=ry;
	p->Z=rz;
	
}

void	rotate_ex_face(SWORD index)
{

	rotate_point_by_xyz((SVector*)&ex_faces[index].P[0],ex_faces[index].AX,ex_faces[index].AY,0);
	rotate_point_by_xyz((SVector*)&ex_faces[index].P[1],ex_faces[index].AX,ex_faces[index].AY,0);
	rotate_point_by_xyz((SVector*)&ex_faces[index].P[2],ex_faces[index].AX,ex_faces[index].AY,0);
	
}

void	remove_ex_face(SWORD	index)
{
	ex_faces[index].X=0;
	if(index==next_ex_face-1)
		next_ex_face--;
	
}

SLONG	find_an_empty_explode_face(void)
{
	SLONG	c0;
	for(c0=1;c0<MAX_EX_FACE;c0++)
	{
		if(ex_faces[c0].X==0)
		{
			
			if(c0>=next_ex_face)
				next_ex_face=c0+1;
			return(c0);
		}
	}
	return(0);
}


void	split_explode_face(SWORD index)
{
	SWORD	aindex[]={0,1,2,0,1,2};
	SLONG	dx,dy,dz;
	SLONG	len,b_len=0,b_index=0;
	SLONG	mid_x,mid_y,mid_z,mid_shade,mid_tx,mid_ty;
	SLONG	face1,face2;
	SLONG	tri1[3],tri2[3];
	SLONG	*tri;
	SLONG	c0,c1;
	for(c0=0;c0<3;c0++)
	{
		
		dx=ex_faces[index].P[aindex[c0+1]].X-ex_faces[index].P[aindex[c0]].X;
		dy=ex_faces[index].P[aindex[c0+1]].Y-ex_faces[index].P[aindex[c0]].Y;
		dz=ex_faces[index].P[aindex[c0+1]].Z-ex_faces[index].P[aindex[c0]].Z;
		len=dx*dx+dy*dy+dz*dz;
		if(len>b_len)
		{
			b_len=len;
			b_index=c0;
		}
	}

	mid_x	 =(ex_faces[index].P[aindex[b_index]].X+ex_faces[index].P[aindex[b_index+1]].X)>>1;
	mid_y	 =(ex_faces[index].P[aindex[b_index]].Y+ex_faces[index].P[aindex[b_index+1]].Y)>>1;
	mid_z	 =(ex_faces[index].P[aindex[b_index]].Z+ex_faces[index].P[aindex[b_index+1]].Z)>>1;
	mid_shade=(ex_faces[index].Face3.Bright[aindex[b_index]]+ex_faces[index].Face3.Bright[aindex[b_index+1]])>>1;
	mid_tx   =(ex_faces[index].Face3.UV[aindex[b_index]][0]+ex_faces[index].Face3.UV[aindex[b_index]][0])>>1;
	mid_ty   =(ex_faces[index].Face3.UV[aindex[b_index]][1]+ex_faces[index].Face3.UV[aindex[b_index]][1])>>1;

	face1=find_an_empty_explode_face();

	if(face1)
	{
		
		switch(b_index)
		{
			case	0:
				tri1[0]=0;
				tri1[1]=-1;
				tri1[2]=2;

				tri2[0]=-1;
				tri2[1]=1;
				tri2[2]=2;
				break;

			case	1:
				tri1[0]=-1;
				tri1[1]=0;
				tri1[2]=1;

				tri2[0]=-1;
				tri2[1]=2;
				tri2[2]=0;
				break;

			case	2:
				tri1[0]=-1;
				tri1[1]=0;
				tri1[2]=1;

				tri2[0]=-1;
				tri2[1]=1;
				tri2[2]=2;
				break;
		}

		tri=tri1;
		for(c1=0;c1<2;c1++)
		{
			
			if(face1)
			{
				
				ex_faces[face1].Face3.TexturePage=ex_faces[index].Face3.TexturePage;
				ex_faces[face1].Face3.DrawFlags=ex_faces[index].Face3.DrawFlags;
				ex_faces[face1].AX=ex_faces[index].AX+(rand()&7);
				ex_faces[face1].AY=ex_faces[index].AY+(rand()&7);
				ex_faces[face1].VX=ex_faces[index].VX+(rand()&127)-64;
				ex_faces[face1].VY=ex_faces[index].VY+(rand()&127)-64;
				ex_faces[face1].VZ=ex_faces[index].VZ+(rand()&127)-64;
				ex_faces[face1].Timer=ex_faces[index].Timer;
				ex_faces[face1].X=ex_faces[index].X+(mid_x<<8);
				ex_faces[face1].Y=ex_faces[index].Y+(mid_y<<8);
				ex_faces[face1].Z=ex_faces[index].Z+(mid_z<<8);
				for(c0=0;c0<3;c0++)
				{
					if(tri[c0]>=0)
					{
	//					ex_faces[face1].P[c0]=ex_faces[index].P[tri[c0]];

						ex_faces[face1].P[c0].X=ex_faces[index].P[tri[c0]].X-mid_x;
						ex_faces[face1].P[c0].Y=ex_faces[index].P[tri[c0]].Y-mid_y;
						ex_faces[face1].P[c0].Z=ex_faces[index].P[tri[c0]].Z-mid_z;

						ex_faces[face1].Face3.UV[c0][0]=ex_faces[index].Face3.UV[tri[c0]][0];
						ex_faces[face1].Face3.UV[c0][1]=ex_faces[index].Face3.UV[tri[c0]][1];
						ex_faces[face1].Face3.Bright[c0]=ex_faces[index].Face3.Bright[tri[c0]];
					}
					else
					{
						ex_faces[face1].P[c0].X=0;//mid_x;			   //0;//
						ex_faces[face1].P[c0].Y=0;//mid_y;			   //0;//
						ex_faces[face1].P[c0].Z=0;//mid_z;			   //0;//

						ex_faces[face1].Face3.UV[c0][0] =mid_tx;
						ex_faces[face1].Face3.UV[c0][1] =mid_ty;
						ex_faces[face1].Face3.Bright[c0]=mid_shade;
					}
				}
				tri=tri2;
				face1=find_an_empty_explode_face();
			}
		}
	remove_ex_face(index);
	}

}



void	move_explode_faces(void)
{
	SLONG	c0;
	for(c0=1;c0<next_ex_face;c0++)
	{
		if(ex_faces[c0].X)
		{
			ex_faces[c0].X+=ex_faces[c0].VX;
			ex_faces[c0].Y+=ex_faces[c0].VY;
			ex_faces[c0].Z+=ex_faces[c0].VZ;
/*
			ex_faces[c0].VX=(ex_faces[c0].VX*250)>>8;
			ex_faces[c0].VY=((ex_faces[c0].VY*250)>>8)+30;
			ex_faces[c0].VZ=(ex_faces[c0].VZ*250)>>8;
*/

			ex_faces[c0].VY+=70;
			rotate_ex_face(c0);
			if((rand()&7)==0)
				split_explode_face(c0);

			if(ex_faces[c0].Timer--<0)
			{
				remove_ex_face(c0);
			}

		}
	}
}


void	draw_explode_faces(void)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	ULONG	flag_and,flag_or;
	SLONG	c0;
	SLONG az;
	SLONG	flags[3];

	SVector	res[3];

	move_explode_faces();

	for(c0=1;c0<next_ex_face;c0++)
	{
		if(ex_faces[c0].X)
		{
			engine.X-=ex_faces[c0].X;
			engine.Y-=ex_faces[c0].Y;
			engine.Z-=ex_faces[c0].Z;

			flags[0]=rotate_point_gte((struct SVector*)&ex_faces[c0].P[0],&res[0]);
			flags[1]=rotate_point_gte((struct SVector*)&ex_faces[c0].P[1],&res[1]);
			flags[2]=rotate_point_gte((struct SVector*)&ex_faces[c0].P[2],&res[2]);

			engine.X+=ex_faces[c0].X;
			engine.Y+=ex_faces[c0].Y;
			engine.Z+=ex_faces[c0].Z;

			az=(res[0].Z+res[1].Z+res[2].Z)/3;

			flag_and = flags[0]&flags[1]&flags[2];
			flag_or  = flags[0]|flags[1]|flags[2];

			if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
			{

				setPolyType3(
								current_bucket_pool,
								ex_faces[c0].Face3.DrawFlags
							);

				setCol3	(
							(struct BucketTri*)current_bucket_pool,
							ex_faces[c0].Face3.Col2
						);

				setXY3	(
							(struct BucketTri*)current_bucket_pool,
							res[0].X,res[0].Y,
							res[1].X,res[1].Y,
							res[2].X,res[2].Y
						);

				setUV3	(
							(struct BucketTri*)current_bucket_pool,
							ex_faces[c0].Face3.UV[0][0],ex_faces[c0].Face3.UV[0][1],
							ex_faces[c0].Face3.UV[1][0],ex_faces[c0].Face3.UV[1][1],
							ex_faces[c0].Face3.UV[2][0],ex_faces[c0].Face3.UV[2][1],
							ex_faces[c0].Face3.TexturePage
						);

				setShade3((struct BucketTri*)current_bucket_pool,
							ex_faces[c0].Face3.Bright[0],
							ex_faces[c0].Face3.Bright[1],
							ex_faces[c0].Face3.Bright[2]);

				((struct BucketTri*)current_bucket_pool)->DebugInfo=c0;

				add_bucket((void *)current_bucket_pool,az);

				current_bucket_pool	+=	sizeof(struct BucketQuad);
			}
		}
	}
}



void	add_explode_face(struct	PrimPoint *p0,struct	PrimPoint	*p1,struct PrimPoint *p2,struct	PrimFace3 *p_face,SLONG x,SLONG y,SLONG z,SLONG vx,SLONG vy,SLONG vz)
{
	SLONG	index;
	SLONG	ax,ay,az;
	index=find_an_empty_explode_face();
//	y=y+50;
	if(index)
	{
/*
		if(rand()&1)
		{
			ax=(p0->X+p1->X+p2->X)/3;
			ay=(p0->Y+p1->Y+p2->Y)/3;
			az=(p0->Z+p1->Z+p2->Z)/3;
		}
		else
*/
		{
			ax=(p0->X);
			ay=(p0->Y);
			az=(p0->Z);
		}

		p0->X-=ax;
		p1->X-=ax;
		p2->X-=ax;

		p0->Y-=ay;
		p1->Y-=ay;
		p2->Y-=ay;

		p0->Z-=az;
		p1->Z-=az;
		p2->Z-=az;

		x+=ax;
		y+=ay;
		z+=az;

		ex_faces[index].P[0]=*p0;
		ex_faces[index].P[1]=*p1;
		ex_faces[index].P[2]=*p2;

		ex_faces[index].Face3=(const struct PrimFace3)*p_face;
//		memcpy(&ex_faces[index].Face3,p_face,sizeof(struct PrimFace3));

		ex_faces[index].X=x<<8;
		ex_faces[index].Y=y<<8;
		ex_faces[index].Z=z<<8;
		ex_faces[index].VX=((p0->X-p1->X)<<3)+((rand()&255)-128);
		ex_faces[index].VY=(+1000+(rand()&511));
		ex_faces[index].VZ=((p0->Z-p1->Z)<<3)+((rand()&255)-128);
		ex_faces[index].Face3.DrawFlags|=POLY_FLAG_DOUBLESIDED;
		ex_faces[index].AX=((rand()&63));//<<1;
		ex_faces[index].AY=((rand()&63));//<<1;
		ex_faces[index].Timer=30+(rand()&7);
	}
}


UWORD	make_poly_into_glass_shatter_prim(SWORD face,SWORD mid_x,SWORD mid_y,SWORD mid_z)
{
	UWORD	next_game_point=next_prim_point;
	UWORD	next_game_face3=next_prim_face3;
	UWORD	next_game_object=next_prim_object;
	SLONG	count,cp=0,snp=0,c0,side;
	SLONG	x1,y1,z1,x2,y2,z2,tx1,ty1,tx2,ty2;
	SWORD	index[]={0,1,3,2,0};
	SWORD	text_x[1000];
	SWORD	text_y[1000];
	struct	PrimPoint	points[1000];
	struct	MapThing	*p_thing;
	UWORD 	tindex = 0;
	SWORD	tf;

	struct	PrimPoint	pp0,pp1,pp2;
	struct	PrimFace3	face3;

	LogText("shatter face \n");
	dump_face_info(face);

	if(face>0)
	{
		SLONG	dx,dy,tx,ty;
// 		if(prim_faces4[face].DrawFlags==POLY_NULL)
//			return(0);

//		prim_points[next_game_point+cp].X=mid_x;
//		prim_points[next_game_point+cp].Y=mid_y;
//		prim_points[next_game_point+cp++].Z=mid_z;

		points[cp].X=mid_x;
		points[cp].Y=mid_y;
		points[cp++].Z=mid_z;

		calc_txty(face,&tx,&ty,mid_x,mid_y,mid_z);

		text_x[0]=tx;
		text_y[0]=ty;

		for(side=0;side<4;side++)
		{
			
			x1=prim_points[prim_faces4[face].Points[index[side]]].X;
			y1=prim_points[prim_faces4[face].Points[index[side]]].Y;
			z1=prim_points[prim_faces4[face].Points[index[side]]].Z;

			x2=prim_points[prim_faces4[face].Points[index[side+1]]].X;
			y2=prim_points[prim_faces4[face].Points[index[side+1]]].Y;
			z2=prim_points[prim_faces4[face].Points[index[side+1]]].Z;

			tx1=prim_faces4[face].UV[index[side]][0];
			ty1=prim_faces4[face].UV[index[side]][1];

			tx2=prim_faces4[face].UV[index[side+1]][0];
			ty2=prim_faces4[face].UV[index[side+1]][1];

			count=(rand()%5)+2;

			for(c0=0;c0<=count;c0++)
			{
				text_x[cp]=tx1+(((tx2-tx1)*c0)/count);
				text_y[cp]=ty1+(((ty2-ty1)*c0)/count);

/*
				prim_points[next_game_point+cp].X=x1+(((x2-x1)*c0)/count);
				prim_points[next_game_point+cp].Y=y1+(((y2-y1)*c0)/count);
6				prim_points[next_game_point+cp++].Z=z1+(((z2-z1)*c0)/count);
*/

				points[cp].X=x1+(((x2-x1)*c0)/count);
				points[cp].Y=y1+(((y2-y1)*c0)/count);
				points[cp++].Z=z1+(((z2-z1)*c0)/count);
			

			}
		}
		LogText(" GENERATE POINTS  %d to %d(inc) \n",next_game_point,cp-1);

		p_thing=&map_things[prim_faces4[face].ThingIndex];


		for(tf=1;tf<cp;tf++)
		{
			SLONG	p0,p1,p2;

			p2=tf;
			p1=0;
			if(tf==cp-1)
				p0=1;
			else
				p0=tf+1;

			pp0=points[p0];
			pp1=points[p1];
			pp2=points[p2];


			if(tf==cp-1)
			{
				face3.UV[0][0]=text_x[1];
				face3.UV[0][1]=text_y[1];
			}
			else
			{
				face3.UV[0][0]=text_x[tf+1];
				face3.UV[0][1]=text_y[tf+1];
			}

			face3.UV[1][0]=text_x[0];
			face3.UV[1][1]=text_y[0];
			face3.UV[2][0]=text_x[tf];
			face3.UV[2][1]=text_y[tf];

			face3.DrawFlags=prim_faces4[face].DrawFlags;
			face3.TexturePage=prim_faces4[face].TexturePage;

			face3.Bright[0]=128;
			face3.Bright[1]=128;
			face3.Bright[2]=128;

			add_explode_face(&pp0,&pp1,&pp2,(struct PrimFace3*)&face3,p_thing->X,p_thing->Y,p_thing->Z,0,0,0);

//			LogText("FRAG %d \n",tf);
//			dump_face_info(-(next_game_face3+tf-1));

		}

 		prim_faces4[face].DrawFlags=POLY_NULL;
	}
	return(tindex);
}


