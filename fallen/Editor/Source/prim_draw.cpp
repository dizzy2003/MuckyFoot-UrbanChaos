#include	"Editor.hpp"

#include	"game.h"
#include	"engine.h"
#include	"prim.h"
#include	"prim_draw.h"
#include	"anim.h"
#include	"c:\fallen\headers\animtmap.h"
#include	"c:\fallen\headers\memory.h"

#define	EDITOR	1




extern	UWORD	calc_lights(SLONG x,SLONG y,SLONG z,struct SVector *p_vect);         //prim.c??
extern	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
extern void matrix_transform(struct Matrix31* result, struct Matrix33* trans,struct  Matrix31* mat2);
extern void matrix_transform_small(struct Matrix31* result, struct Matrix33* trans,struct  SMatrix31* mat2);

#ifdef	EDITOR
extern	void	do_quad_clip_list(SWORD face,SLONG p0,SLONG p1,SLONG p2,SLONG p3);   //prim_edit.h
extern	void	do_tri_clip_list(SWORD face,SLONG p0,SLONG p1,SLONG p2);             //prim_edit.h
extern	BOOL	check_mouse_over_prim_quad(struct SVector *res,SLONG p1,SLONG p2,SLONG p3,SLONG p4,SLONG face); //edit.h
extern	BOOL	check_mouse_over_prim_tri(struct SVector *res,SLONG p1,SLONG p2,SLONG p3,SLONG face); //edit.h
extern	struct SVector	selected_prim_xyz;
extern	SWORD	SelectFlag;
extern	SWORD	SelectDrawn;
#endif






//void	rotate_thing_point(struct SVector *vect,struct Matrix33	*matrix)
/*
void	apply_matrix_to_vect(struct SVector *before,struct Matrix33	*matrix,struct SVector *result)
{																			
	SLONG	x,y,z;
	SLONG	lcosa,lsina;
	SLONG	lcost,lsint;


	matrix_transform(&result,matrix, &before);

	vect->X= result.X>>15;
	vect->Y= result.Y>>15;   // doubles up else
	vect->Z= result.Z>>15;

}
*/

void	add_quad_to_bucket(SLONG p0,SLONG p1,SLONG p2,SLONG p3,struct SVector *res,struct SVector *points,UWORD *bright,SLONG *flags,struct	PrimFace4 *p_f4)
{
	SLONG az;
	ULONG	flag_and,flag_or;

	points=points;

	if(current_bucket_pool>=end_bucket_pool)
		return;
		flag_and = flags[p0]&flags[p1]&flags[p2]&flags[p3];	
		flag_or = flags[p0]|flags[p1]|flags[p2]|flags[p3];	

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
			
			az=(res[p0].Z+res[p1].Z+res[p2].Z+res[p3].Z)>>2;

			setPolyType4(
							current_bucket_pool,
							p_f4->DrawFlags
						);
			setXY4	(
						(struct BucketQuad*)current_bucket_pool,
						res[p0].X,res[p0].Y,
						res[p1].X,res[p1].Y,
						res[p2].X,res[p2].Y,
						res[p3].X,res[p3].Y
					);

			setUV4	(
						(struct BucketQuad*)current_bucket_pool,
						p_f4->UV[0][0],p_f4->UV[0][1],
						p_f4->UV[1][0],p_f4->UV[1][1],
						p_f4->UV[2][0],p_f4->UV[2][1],
						p_f4->UV[3][0],p_f4->UV[3][1],
						p_f4->TexturePage
					);

			setZ4((struct BucketQuad*)current_bucket_pool,-res[p0].Z,-res[p1].Z,-res[p2].Z,-res[p3].Z);
			

//			setShade4((struct BucketQuad*)current_bucket_pool,p_f4->Bright[0],p_f4->Bright[1],p_f4->Bright[2],p_f4->Bright[3]);
			setShade4((struct BucketQuad*)current_bucket_pool,
			CLIP256(p_f4->Bright[0]+bright[p0]),
			CLIP256(p_f4->Bright[1]+bright[p1]),
			CLIP256(p_f4->Bright[2]+bright[p2]),
			CLIP256(p_f4->Bright[3]+bright[p3]));

			add_bucket((void *)current_bucket_pool,az);
			current_bucket_pool	+=	sizeof(struct BucketQuad);
		}
	
}	

inline void	add_split_quad_to_bucket(SLONG p0,SLONG p1,SLONG p3,SLONG p2,struct SVector *info,struct SVector *res,SLONG *flags)
{
	SLONG az;
	ULONG	flag_and,flag_or;

	if(current_bucket_pool>=end_bucket_pool)
		return;

	flag_and = flags[p0]&flags[p1]&flags[p2]&flags[p3];	
	flag_or = flags[p0]|flags[p1]|flags[p2]|flags[p3];	

	if((flag_or&EF_BEHIND_YOU)==0)
	if(!(flag_and & EF_CLIPFLAGS))
	{
		
		az=(res[p0].Z+res[p1].Z+res[p2].Z+res[p3].Z)>>2;

		setPolyType4(
						current_bucket_pool,
						3
					);
		setXY4	(
					(struct BucketQuad*)current_bucket_pool,
					res[p0].X,res[p0].Y,
					res[p1].X,res[p1].Y,
					res[p2].X,res[p2].Y,
					res[p3].X,res[p3].Y
				);

		setUV4	(
					(struct BucketQuad*)current_bucket_pool,
							info[p0].X,info[p0].Y,
							info[p1].X,info[p1].Y,
							info[p2].X,info[p2].Y,
							info[p3].X,info[p3].Y,(SWORD)0
				);

		setZ4((struct BucketQuad*)current_bucket_pool,-res[p0].Z,-res[p1].Z,-res[p2].Z,-res[p3].Z);
		

//			setShade4((struct BucketQuad*)current_bucket_pool,p_f4->Bright[0],p_f4->Bright[1],p_f4->Bright[2],p_f4->Bright[3]);
		setShade4((struct BucketQuad*)current_bucket_pool,
							info[p0].Z,info[p1].Z,
							info[p2].Z,info[p3].Z);

		add_bucket((void *)current_bucket_pool,az);
		current_bucket_pool	+=	sizeof(struct BucketQuad);
	}
	
}	



#define	AVERAGE2(x,y)	(((x)+(y))>>1)
#define	AVERAGE4(x,y,A,B)	(((x)+(y)+(A)+(B))>>2)

inline void	average_svect(struct SVector *mid,struct SVector *p1,struct SVector *p2)
{
	mid->X=AVERAGE2(p1->X,p2->X);
	mid->Y=AVERAGE2(p1->Y,p2->Y);
	mid->Z=AVERAGE2(p1->Z,p2->Z);
}

inline void	average_svect4(struct SVector *mid,struct SVector *p1,struct SVector *p2,struct SVector *p3,struct SVector *p4)
{
	mid->X=AVERAGE4(p1->X,p2->X,p3->X,p4->X);
	mid->Y=AVERAGE4(p1->Y,p2->Y,p3->Y,p4->Y);
	mid->Z=AVERAGE4(p1->Z,p2->Z,p3->Z,p4->Z);
}

/*
		p1        p5         p2


		p8		  p9		 p6


	    p4   	  p7		 p3
*/


ULONG	should_i_split_it(SLONG p1,SLONG p2,SLONG p3,SLONG p4,struct SVector *res)
{
	SLONG	top,bottom;
	SLONG	left,right;
	top    = MIN( MIN( MIN(res[p1].Y,res[p2].Y),res[p3].Y ),res[p4].Y );
	bottom = MAX( MAX( MAX(res[p1].Y,res[p2].Y),res[p3].Y ),res[p4].Y );

	left   = MIN( MIN( MIN(res[p1].X,res[p2].X),res[p3].X ),res[p4].X );
	right  = MAX( MAX( MAX(res[p1].X,res[p2].X),res[p3].X ),res[p4].X );
	if(bottom-top>190||right-left>190)
		return(1);
	else
		return(0);

	
}
ULONG	check_flags(SLONG p1,SLONG p2,SLONG p3,SLONG p0,SLONG *flags)
{
	SLONG	flags_or;

	flags_or=flags[p0]|flags[p1]|flags[p2]|flags[p3];

	if((flags_or&EF_BEHIND_YOU)==0||flags_or&EF_TOO_BIG)
	{
		
		if(!(flags[p0]&flags[p1]&flags[p2]&flags[p3]& (EF_CLIPFLAGS|EF_BEHIND_YOU)))
			return(1);
	}
	return(0);
}

void	split_quad_r(SLONG p1,SLONG p2,SLONG p3,SLONG p4,struct SVector *res,struct SVector *points,UWORD *bright,SLONG *flags,struct	SVector *info,UWORD depth)
{
	struct	SVector	new_points[10],new_res[10],new_info[10];
//	struct	PrimFace4	new_faces[10];
	SLONG	new_flags[10];
//	SWORD	new_bright[10];

	new_points[1]=points[p1]; 
	new_points[2]=points[p2];  
	new_points[3]=points[p3];   
	new_points[4]=points[p4];	 

	new_flags[1]=flags[p1];
	new_flags[2]=flags[p2];
	new_flags[3]=flags[p3];
	new_flags[4]=flags[p4];

	new_res[1]=res[p1];
	new_res[2]=res[p2];
	new_res[3]=res[p3];
	new_res[4]=res[p4];

	new_info[1]=info[p1];
	new_info[2]=info[p2];
	new_info[3]=info[p3];
	new_info[4]=info[p4];

	average_svect(&new_points[5],&points[p1],&points[p2]);
	average_svect(&new_points[6],&points[p2],&points[p3]);
	average_svect(&new_points[7],&points[p3],&points[p4]);
	average_svect(&new_points[8],&points[p4],&points[p1]);
	average_svect4(&new_points[9],&points[p1],&points[p2],&points[p3],&points[p4]);

	average_svect(&new_info[5],&new_info[1],&new_info[2]);
	average_svect(&new_info[6],&new_info[2],&new_info[3]);
	average_svect(&new_info[7],&new_info[3],&new_info[4]);
	average_svect(&new_info[8],&new_info[4],&new_info[1]);
	average_svect4(&new_info[9],&new_info[1],&new_info[2],&new_info[3],&new_info[4]);

	new_flags[5]=rotate_point_gte((struct SVector*)&new_points[5],&new_res[5]);
	new_flags[6]=rotate_point_gte((struct SVector*)&new_points[6],&new_res[6]);
	new_flags[7]=rotate_point_gte((struct SVector*)&new_points[7],&new_res[7]);
	new_flags[8]=rotate_point_gte((struct SVector*)&new_points[8],&new_res[8]);
	new_flags[9]=rotate_point_gte((struct SVector*)&new_points[9],&new_res[9]);

//now we have all the texture info, all the draw coords, all the shades

	if(check_flags(1,5,9,8,new_flags))
	{
		if(depth<3&&should_i_split_it(1,5,9,8,new_res))
			split_quad_r(1,5,9,8,new_res,new_points,bright,new_flags,new_info,depth+1);
		else
			add_split_quad_to_bucket(1,5,9,8,new_info,new_res,new_flags);
	}

	if(check_flags(5,2,6,9,new_flags))
	{
		if(depth<3&&should_i_split_it(5,2,6,9,new_res))
			split_quad_r(5,2,6,9,new_res,new_points,bright,new_flags,new_info,depth+1);
		else
			add_split_quad_to_bucket(5,2,6,9,new_info,new_res,new_flags);
	}

	if(check_flags(9,6,3,7,new_flags))
	{
		if(depth<3&&should_i_split_it(9,6,3,7,new_res))
			split_quad_r(9,6,3,7,new_res,new_points,bright,new_flags,new_info,depth+1);
		else
			add_split_quad_to_bucket(9,6,3,7,new_info,new_res,new_flags);
	}

	if(check_flags(8,9,7,4,new_flags))
	{
		if(depth<3&&should_i_split_it(8,9,7,4,new_res))
			split_quad_r(8,9,7,4,new_res,new_points,bright,new_flags,new_info,depth+1);
		else
			add_split_quad_to_bucket(8,9,7,4,new_info,new_res,new_flags);
	}
/*
	add_split_quad_to_bucket(1,5,9,8,new_info,new_res,new_flags);
	add_split_quad_to_bucket(5,2,6,9,new_info,new_res,new_flags);
	add_split_quad_to_bucket(9,6,3,7,new_info,new_res,new_flags);
	add_split_quad_to_bucket(8,9,7,4,new_info,new_res,new_flags);
*/


//	add_quad_to_bucket(p0,p1,p2,p3,res,points,bright,flags,p_f4);
}


inline void	split_quad(SLONG p1,SLONG p2,SLONG p3,SLONG p4,struct SVector *res,struct SVector *points,UWORD *bright,SLONG *flags,struct	PrimFace4 *p_f4,UWORD depth)
{
	struct	SVector	new_points[10],new_res[10],new_info[10];
//	struct	PrimFace4	new_faces[10];
	SLONG	new_flags[10];
//	SWORD	new_bright[10];


	new_points[1]=points[p1]; 
	new_points[2]=points[p2];  
	new_points[3]=points[p3];   
	new_points[4]=points[p4];	 


	new_flags[1]=flags[p1];
	new_flags[2]=flags[p2];
	new_flags[3]=flags[p3];
	new_flags[4]=flags[p4];

	new_res[1]=res[p1];
	new_res[2]=res[p2];
	new_res[3]=res[p3];
	new_res[4]=res[p4];

	new_info[1].X=p_f4->UV[0][0];
	new_info[1].Y=p_f4->UV[0][1];
	new_info[1].Z=p_f4->Bright[0];

	new_info[2].X=p_f4->UV[1][0];
	new_info[2].Y=p_f4->UV[1][1];
	new_info[2].Z=p_f4->Bright[1];

	new_info[3].X=p_f4->UV[3][0];
	new_info[3].Y=p_f4->UV[3][1];
	new_info[3].Z=p_f4->Bright[3];

	new_info[4].X=p_f4->UV[2][0];
	new_info[4].Y=p_f4->UV[2][1];
	new_info[4].Z=p_f4->Bright[2];

	average_svect(&new_points[5],&points[p1],&points[p2]);
	average_svect(&new_points[6],&points[p2],&points[p3]);
	average_svect(&new_points[7],&points[p3],&points[p4]);
	average_svect(&new_points[8],&points[p4],&points[p1]);
	average_svect4(&new_points[9],&points[p1],&points[p2],&points[p3],&points[p4]);

	average_svect(&new_info[5],&new_info[1],&new_info[2]);
	average_svect(&new_info[6],&new_info[2],&new_info[3]);
	average_svect(&new_info[7],&new_info[3],&new_info[4]);
	average_svect(&new_info[8],&new_info[4],&new_info[1]);
	average_svect4(&new_info[9],&new_info[1],&new_info[2],&new_info[3],&new_info[4]);

	new_flags[5]=rotate_point_gte((struct SVector*)&new_points[5],&new_res[5]);
	new_flags[6]=rotate_point_gte((struct SVector*)&new_points[6],&new_res[6]);
	new_flags[7]=rotate_point_gte((struct SVector*)&new_points[7],&new_res[7]);
	new_flags[8]=rotate_point_gte((struct SVector*)&new_points[8],&new_res[8]);
	new_flags[9]=rotate_point_gte((struct SVector*)&new_points[9],&new_res[9]);

//now we have all the texture info, all the draw coords, all the shades
	
	if(check_flags(1,5,9,8,new_flags))
	{
		depth=should_i_split_it(1,5,9,8,new_res);
		if(depth)
			split_quad_r(1,5,9,8,new_res,new_points,bright,new_flags,new_info,depth);
		else
			add_split_quad_to_bucket(1,5,9,8,new_info,new_res,new_flags);
	}

	if(check_flags(5,2,6,9,new_flags))
	{
		depth=should_i_split_it(5,2,6,9,new_res);
		if(depth)
			split_quad_r(5,2,6,9,new_res,new_points,bright,new_flags,new_info,depth);
		else
			add_split_quad_to_bucket(5,2,6,9,new_info,new_res,new_flags);
	}

	if(check_flags(9,6,3,7,new_flags))
	{
		depth=should_i_split_it(9,6,3,7,new_res);
		if(depth)
			split_quad_r(9,6,3,7,new_res,new_points,bright,new_flags,new_info,depth);
		else
			add_split_quad_to_bucket(9,6,3,7,new_info,new_res,new_flags);
	}

	if(check_flags(8,9,7,4,new_flags))
	{
		depth=should_i_split_it(8,9,7,4,new_res);
		if(depth)
			split_quad_r(8,9,7,4,new_res,new_points,bright,new_flags,new_info,depth);
		else
			add_split_quad_to_bucket(8,9,7,4,new_info,new_res,new_flags);
	}


//	add_quad_to_bucket(p0,p1,p2,p3,res,points,bright,flags,p_f4);
}

extern	UWORD	is_it_clockwise(struct SVector *res,SLONG p1,SLONG p2,SLONG p3);

void	draw_a_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,UBYTE shade)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	ULONG	flag_and,flag_or;
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG az;
	SLONG col=0,cor=0,cob=0,cot=0,total=0;


	p_obj    =	&prim_objects[prim];
	p_f4     =	&prim_faces4[p_obj->StartFace4];
	p_f3     =	&prim_faces3[p_obj->StartFace3];

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;
	
	engine.X-=x<<8;
	engine.Y-=y<<8;
	engine.Z-=z<<8;
		
	for(c0=sp;c0<ep;c0++)
	{
		SVector	pp;
		//transform all points for this Object

		pp.X=prim_points[c0].X;
		pp.Y=prim_points[c0].Y;
		pp.Z=prim_points[c0].Z;
		global_flags[c0-sp]=rotate_point_gte(&pp,&global_res[c0-sp]);
		global_bright[c0-sp]=calc_lights(x,y,z,&pp);
	}


//		LogText(" quads = %d \n",p_obj->EndFace4-p_obj->StartFace4);

	if(p_obj->EndFace4)
	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		SLONG	p0,p1,p2,p3;
		UWORD	depth;

		if(current_bucket_pool>=end_bucket_pool)
			goto	exit;

		p0=p_f4->Points[0]-sp;
		p1=p_f4->Points[1]-sp;
		p2=p_f4->Points[2]-sp;
		p3=p_f4->Points[3]-sp;

/*
		if(c0==9866)
		{
			if(depth=should_i_split_it(p0,p1,p3,p2,global_res))
			{
				
				if(check_flags(p0,p1,p3,p2,global_flags))
					split_quad(p0,p1,p3,p2,global_res,(struct SVector*)&prim_points[sp],global_bright,global_flags,p_f4,depth);
			}
		}
		else
*/

		{
			
	
			flag_and = global_flags[p0]&global_flags[p1]&global_flags[p2]&global_flags[p3];	
			flag_or = global_flags[p0]|global_flags[p1]|global_flags[p2]|global_flags[p3];	
/*
			if(c0==28334)
			{
				CBYTE	str[100];
				sprintf(str," f0 %x f1 %x f2 %x f3 %x \n",global_flags[p0],global_flags[p1],global_flags[p2],global_flags[p3]);
				QuickText(10,10,str,255);
				
			}
*/

			if((p_f4->FaceFlags&FACE_FLAG_SMOOTH)&&ShiftFlag)
			{
				
			}
			else
			{

				
				if( (!(flag_and & EF_CLIPFLAGS))&&((flag_or&EF_BEHIND_YOU)==0))
				{
					SLONG	wid,height;
					total++;
//					az=(global_res[p0].Z+global_res[p1].Z+global_res[p2].Z+global_res[p3].Z)>>2;
					az=global_res[p0].Z;

					if(global_res[p1].Z>az)
						az=global_res[p1].Z;

					if(global_res[p2].Z>az)
						az=global_res[p2].Z;

					if(global_res[p3].Z>az)
						az=global_res[p3].Z;


					wid=WorkWindowWidth;
					height=WorkWindowHeight;

					setPolyType4(
									current_bucket_pool,
									p_f4->DrawFlags
								);


					setCol4	(
								(struct BucketQuad*)current_bucket_pool,
								p_f4->Col2
							);


					setXY4	(
								(struct BucketQuad*)current_bucket_pool,
								global_res[p0].X,global_res[p0].Y,
								global_res[p1].X,global_res[p1].Y,
								global_res[p2].X,global_res[p2].Y,
								global_res[p3].X,global_res[p3].Y
							);
#ifdef	EDITOR
					if(SelectFlag)
					if(SelectDrawn==0||is_it_clockwise(global_res,p0,p1,p2))
						do_quad_clip_list(c0,p0,p1,p2,p3);
#endif

					if(p_f4->FaceFlags&FACE_FLAG_ANIMATE)
					{
						struct	AnimTmap	*p_a;
						SLONG	cur;
						p_a=&anim_tmaps[p_f4->TexturePage];
						cur=p_a->Current;

						setUV4	(
									(struct BucketQuad*)current_bucket_pool,
									p_a->UV[cur][0][0],p_a->UV[cur][0][1],
									p_a->UV[cur][1][0],p_a->UV[cur][1][1],
									p_a->UV[cur][2][0],p_a->UV[cur][2][1],
									p_a->UV[cur][3][0],p_a->UV[cur][3][1],
									p_a->Page[cur]
								);

					}
					else
					{
//						if(p_f4->TexturePage>15)
//							p_f4->TexturePage=15;
						setUV4	(
									(struct BucketQuad*)current_bucket_pool,
									p_f4->UV[0][0],p_f4->UV[0][1],
									p_f4->UV[1][0],p_f4->UV[1][1],
									p_f4->UV[2][0],p_f4->UV[2][1],
									p_f4->UV[3][0],p_f4->UV[3][1],
									p_f4->TexturePage
								);
//								ASSERT(p_f4->TexturePage<8);
					}

					setZ4((struct BucketQuad*)current_bucket_pool,-global_res[p0].Z,-global_res[p1].Z,-global_res[p2].Z,-global_res[p3].Z);
					

		//			setShade4((struct BucketQuad*)current_bucket_pool,p_f4->Bright[0],p_f4->Bright[1],p_f4->Bright[2],p_f4->Bright[3]);
					if(shade)
					{
						setShade4((struct BucketQuad*)current_bucket_pool,
							CLIP256(p_f4->Bright[0]+global_bright[p0]),
							CLIP256(p_f4->Bright[1]+global_bright[p1]),
							CLIP256(p_f4->Bright[2]+global_bright[p2]),
							CLIP256(p_f4->Bright[3]+global_bright[p3]));
					}
					else
					{
						setShade4((struct BucketQuad*)current_bucket_pool,128,128,128,128);
					}
					((struct BucketQuad*)current_bucket_pool)->DebugInfo=c0;
					((struct BucketQuad*)current_bucket_pool)->DebugFlags=p_f4->FaceFlags;

					add_bucket((void *)current_bucket_pool,az);
#ifdef	EDITOR
					if(check_mouse_over_prim_quad(global_res,p0,p1,p2,p3,c0))
					{
						selected_prim_xyz.X	=	x;
						selected_prim_xyz.Y	=	y;
						selected_prim_xyz.Z	=	z;
					}
#endif

					current_bucket_pool	+=	sizeof(struct BucketQuad);
				}
				else
				{
					if(flag_and&EF_OFF_LEFT)
						col++;
					if(flag_and&EF_OFF_RIGHT)
						cor++;
					if(flag_and&EF_OFF_TOP)
						cot++;
					if(flag_and&EF_OFF_BOTTOM)
						cob++;


		
				}	
//				LogText(" clipped face %d \n",c0);
			}
		}

		p_f4++;
	}

	if(p_obj->EndFace3)
	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		if(current_bucket_pool>=end_bucket_pool)
				goto	exit;

		if(p_f3->Points[0]<p_obj->StartPoint||
		   p_f3->Points[1]<p_obj->StartPoint||
		   p_f3->Points[2]<p_obj->StartPoint)
		   {
				ERROR_MSG(0," point before start point");
				goto	exit;
		   }
		if(p_f3->Points[0]>p_obj->EndPoint||
		   p_f3->Points[1]>p_obj->EndPoint||
		   p_f3->Points[2]>p_obj->EndPoint)
		   {
				ERROR_MSG(0," point after end point");
				goto	exit;
		   }


		p0=p_f3->Points[0]-sp;
		p1=p_f3->Points[1]-sp;
		p2=p_f3->Points[2]-sp;

		flag_and = global_flags[p0]&global_flags[p1]&global_flags[p2];	
		flag_or  = global_flags[p0]|global_flags[p1]|global_flags[p2];	

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
//			az=(global_res[p0].Z+global_res[p1].Z+global_res[p2].Z)/3;
			az=global_res[p0].Z;
			if(global_res[p1].Z>az)
				az=global_res[p1].Z;
			if(global_res[p2].Z>az)
				az=global_res[p2].Z;

			setPolyType3(
							current_bucket_pool,
							p_f3->DrawFlags
						);

			setCol3	(
						(struct BucketTri*)current_bucket_pool,
						p_f3->Col2
					);

			setXY3	(
						(struct BucketTri*)current_bucket_pool,
						global_res[p0].X,global_res[p0].Y,
						global_res[p1].X,global_res[p1].Y,
						global_res[p2].X,global_res[p2].Y
					);

#ifdef	EDITOR
			if(SelectFlag)
			if(SelectDrawn==0||is_it_clockwise(global_res,p0,p1,p2))
				do_tri_clip_list(-c0,p0,p1,p2);
#endif

//			if(p_f3->TexturePage>15)
//				p_f3->TexturePage=15;
			setUV3	(
						(struct BucketTri*)current_bucket_pool,
						p_f3->UV[0][0],p_f3->UV[0][1],
						p_f3->UV[1][0],p_f3->UV[1][1],
						p_f3->UV[2][0],p_f3->UV[2][1],
						p_f3->TexturePage
					);
//			ASSERT(p_f3->TexturePage<8);

			if(shade)
			{
				setShade3((struct BucketTri*)current_bucket_pool,
					CLIP256(p_f3->Bright[0]+global_bright[p0]),
					CLIP256(p_f3->Bright[1]+global_bright[p1]),
					CLIP256(p_f3->Bright[2]+global_bright[p2]));
			}
			else
			{
				setShade3((struct BucketTri*)current_bucket_pool,128,128,128);
			}

			((struct BucketTri*)current_bucket_pool)->DebugInfo=c0;
			((struct BucketTri*)current_bucket_pool)->DebugFlags=p_f3->FaceFlags;

			add_bucket((void *)current_bucket_pool,az);
#ifdef	EDITOR
			if(check_mouse_over_prim_tri(global_res,p0,p1,p2,c0))
			{
				selected_prim_xyz.X	=	x;
				selected_prim_xyz.Y	=	y;
				selected_prim_xyz.Z	=	z;
			}
#endif
			current_bucket_pool	+=	sizeof(struct BucketQuad);
		}
		p_f3++;
	}
exit:;
	engine.X+=x<<8;
	engine.Y+=y<<8;
	engine.Z+=z<<8;

//	LogText(" draw a prim  left %d right %d top %d bot %d  ok %d \n",col,cor,cot,cob,total);


}

void	draw_a_rot_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct	PrimMultiAnim *anim_info,struct Matrix33 *rot_mat)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	ULONG	flag_and,flag_or;
	struct	SVector			temp; //max points per object?
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG az;
	struct	Matrix33 *mat,*mat_next,mat2,mat_final;
	SLONG	i,j;
	struct	PrimMultiAnim *anim_info_next;
	struct	Matrix31	offset;

	p_obj    =&prim_objects[prim];
	p_f4     =&prim_faces4[p_obj->StartFace4];
	p_f3     =&prim_faces3[p_obj->StartFace3];

	
	anim_info_next=&prim_multi_anims[anim_info->Next];

	mat      = &anim_info->Mat;
	mat_next = &anim_info_next->Mat;



	//move object "tweened quantity"  , z&y flipped
	offset.M[0]=(anim_info->DX+(((anim_info_next->DX-anim_info->DX)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[1]=(anim_info->DY+(((anim_info_next->DY-anim_info->DY)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[2]=(anim_info->DZ+(((anim_info_next->DZ-anim_info->DZ)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
	x+= temp.X;
	y+= temp.Y;
	z+= temp.Z;
/*
	x+=(anim_info->DX+(((anim_info_next->DX-anim_info->DX)*tween)>>8))>>2;
	y+=(anim_info->DY+(((anim_info_next->DY-anim_info->DY)*tween)>>8))>>2;
	z+=(anim_info->DZ+(((anim_info_next->DZ-anim_info->DZ)*tween)>>8))>>2;
*/
	

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;
	
	engine.X-=x<<8;
	engine.Y-=y<<8;
	engine.Z-=z<<8;

	//create a temporary "tween" matrix between current and next
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			mat2.M[i][j]=mat->M[i][j]+(((mat_next->M[i][j]-mat->M[i][j])*tween)>>8);
		}
	}

//apply local rotation matrix
	matrix_mult33(&mat_final,rot_mat,&mat2);

		
	for(c0=sp;c0<ep;c0++)
	{


		matrix_transform_small((struct Matrix31*)&temp,&mat_final, (struct SMatrix31*)&prim_points[c0]);
//		matrix_transform((struct Matrix31*)&temp,&mat2, (struct Matrix31*)&prim_points[c0]);
//      matrix now does the yz flip
//		swap(temp.Y,temp.Z)  //MDOPT do this inside the matrix multiply
//		temp.Y=-temp.Y;
		global_flags[c0-sp]=rotate_point_gte((struct SVector*)&temp,&global_res[c0-sp]);
	}

	engine.X+=x<<8;
	engine.Y+=y<<8;
	engine.Z+=z<<8;



	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		SLONG	p0,p1,p2,p3;

		if(current_bucket_pool>=end_bucket_pool)
			return;

		p0=p_f4->Points[0]-sp;
		p1=p_f4->Points[1]-sp;
		p2=p_f4->Points[2]-sp;
		p3=p_f4->Points[3]-sp;

		flag_and = global_flags[p0]&global_flags[p1]&global_flags[p2]&global_flags[p3];	
		flag_or = global_flags[p0]|global_flags[p1]|global_flags[p2]|global_flags[p3];	

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
			
			az=(global_res[p0].Z+global_res[p1].Z+global_res[p2].Z+global_res[p3].Z)>>2;

			setPolyGT4(current_bucket_pool);

			setXY4((struct BucketQuad*)current_bucket_pool,global_res[p0].X,global_res[p0].Y,global_res[p1].X,global_res[p1].Y,global_res[p2].X,global_res[p2].Y,global_res[p3].X,global_res[p3].Y);

			setUV4	(
						(struct BucketQuad*)current_bucket_pool,
						p_f4->UV[0][0],p_f4->UV[0][1],
						p_f4->UV[1][0],p_f4->UV[1][1],
						p_f4->UV[2][0],p_f4->UV[2][1],
						p_f4->UV[3][0],p_f4->UV[3][1],
						p_f4->TexturePage
					);

			setZ4((struct BucketQuad*)current_bucket_pool,global_res[p0].Z,global_res[p1].Z,global_res[p2].Z,global_res[p3].Z);

			setShade4((struct BucketQuad*)current_bucket_pool,p_f4->Bright[0],p_f4->Bright[1],p_f4->Bright[2],p_f4->Bright[3]);
			((struct BucketQuad*)current_bucket_pool)->DebugInfo=c0;
			((struct BucketQuad*)current_bucket_pool)->DebugFlags=p_f4->FaceFlags;

			add_bucket((void *)current_bucket_pool,az);

			current_bucket_pool+=sizeof(struct BucketQuad);
#ifdef	EDITOR
			check_mouse_over_prim_quad(global_res,p0,p1,p2,p3,c0);
#endif
		}
		p_f4++;
	}

	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		if(current_bucket_pool>=end_bucket_pool)
			return;

		p0=p_f3->Points[0]-sp;
		p1=p_f3->Points[1]-sp;
		p2=p_f3->Points[2]-sp;

		flag_and = global_flags[p0]&global_flags[p1]&global_flags[p2];	
		flag_or  = global_flags[p0]|global_flags[p1]|global_flags[p2];	

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
			az=(global_res[p0].Z+global_res[p1].Z+global_res[p2].Z)/3;

			setPolyGT3(current_bucket_pool);

			setXY3((struct BucketTri*)current_bucket_pool,global_res[p0].X,global_res[p0].Y,global_res[p1].X,global_res[p1].Y,global_res[p2].X,global_res[p2].Y);

			setUV3((struct BucketQuad*)current_bucket_pool,0,0,32,0,32,32,1);

			setShade3((struct BucketQuad*)current_bucket_pool,128,128,128);
			((struct BucketTri*)current_bucket_pool)->DebugInfo=c0;
			((struct BucketTri*)current_bucket_pool)->DebugFlags=p_f3->FaceFlags;

			add_bucket((void *)current_bucket_pool,az);
#ifdef	EDITOR
			check_mouse_over_prim_tri(global_res,p0,p1,p2,c0);
#endif

			current_bucket_pool+=sizeof(struct BucketQuad);
		}
		p_f3++;
	}
					
}
extern	void	build_tween_matrix(struct Matrix33 *mat,struct CMatrix33 *cmat1,struct CMatrix33 *cmat2,SLONG tween);

void	draw_anim_prim_tween(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct GameKeyFrameElement *anim_info,struct GameKeyFrameElement *anim_info_next,struct Matrix33 *rot_mat)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	flags[1560];
	UWORD	bright[1560];
	ULONG	flag_and,flag_or;
	struct	SVector			res[1560],temp; //max points per object?
	struct	SVector			res_shadow[1560],temp_shadow; //max points per object?
	SLONG	flags_shadow[1560];
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG az;
	struct	Matrix33 *mat,*mat_next,mat2,mat_final;
	SLONG	i,j;
	struct	Matrix31	offset;
	struct	GameKeyFrame	*the_keyframe1,*the_keyframe2;
	SLONG	dx,dy,dz;
	SLONG	shadow=1;
	SLONG	mapx,mapy,mapz;

	p_obj    =&prim_objects[prim];
	p_f4     =&prim_faces4[p_obj->StartFace4];
	p_f3     =&prim_faces3[p_obj->StartFace3];
	
//	mat      = &anim_info->Matrix;
//	mat_next = &anim_info_next->Matrix;

	//move object "tweened quantity"  , z&y flipped
	offset.M[0]	=	(anim_info->OffsetX+(((anim_info_next->OffsetX-anim_info->OffsetX)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[1]	=	(anim_info->OffsetY+(((anim_info_next->OffsetY-anim_info->OffsetY)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[2]	=	(anim_info->OffsetZ+(((anim_info_next->OffsetZ-anim_info->OffsetZ)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
/*
	the_keyframe1=&(test_chunk.KeyFrames[anim_info->Parent]);
	the_keyframe2=&(test_chunk.KeyFrames[anim_info_next->Parent]);

	offset.M[0]+=(the_keyframe1->Dx+(((the_keyframe2->Dx-the_keyframe1->Dx)*tween)>>8))>>2;
	offset.M[1]+=(the_keyframe1->Dy+(((the_keyframe2->Dy-the_keyframe1->Dy)*tween)>>8))>>2;
	offset.M[2]+=(the_keyframe1->Dz+(((the_keyframe2->Dz-the_keyframe1->Dz)*tween)>>8))>>2;
*/

	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
	x	+= temp.X;
	y	+= temp.Y;
	z	+= temp.Z;	

	CMatrix33	m1, m2;
	GetCMatrix(anim_info, &m1);
	GetCMatrix(anim_info_next, &m2);
	build_tween_matrix(&mat2, &m1, &m2 ,tween);

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;
/*	
	engine.X-=x<<8;
	engine.Y-=y<<8;
	engine.Z-=z<<8;
*/
	mapx=x;
	mapy=y;
	mapz=z;

	//apply local rotation matrix
	matrix_mult33(&mat_final,rot_mat,&mat2);

	for(c0=sp;c0<ep;c0++)
	{
		SVector	pp;
		//transform all points for this Object

		pp.X=prim_points[c0].X;
		pp.Y=prim_points[c0].Y;
		pp.Z=prim_points[c0].Z;

		matrix_transform_small((struct Matrix31*)&temp,&mat_final, (struct SMatrix31*)&prim_points[c0]);
		temp.X+=mapx;
		temp.Y+=mapy;
		temp.Z+=mapz;

		flags[c0-sp]	=	rotate_point_gte((struct SVector*)&temp,&res[c0-sp]);
		bright[c0-sp]	=	calc_lights(x,y,z,&pp);
	}

//	engine.X+=x<<8;
//	engine.Y+=y<<8;
//	engine.Z+=z<<8;

	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		SLONG	p0,p1,p2,p3;

		if(current_bucket_pool>=end_bucket_pool)
			return;

		p0=p_f4->Points[0]-sp;
		p1=p_f4->Points[1]-sp;
		p2=p_f4->Points[2]-sp;
		p3=p_f4->Points[3]-sp;

		flag_and = flags[p0]&flags[p1]&flags[p2]&flags[p3];	
		flag_or = flags[p0]|flags[p1]|flags[p2]|flags[p3];	

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
			
			az=(res[p0].Z+res[p1].Z+res[p2].Z+res[p3].Z)>>2;
			az-=200;

			setPolyType4(
							current_bucket_pool,
							p_f4->DrawFlags
						);

			setCol4	(
						(struct BucketQuad*)current_bucket_pool,
						p_f4->Col2
					);

			setXY4	(
						(struct BucketQuad*)current_bucket_pool,
						res[p0].X,res[p0].Y,
						res[p1].X,res[p1].Y,
						res[p2].X,res[p2].Y,
						res[p3].X,res[p3].Y
					);

			setUV4	(
						(struct BucketQuad*)current_bucket_pool,
						p_f4->UV[0][0],p_f4->UV[0][1],
						p_f4->UV[1][0],p_f4->UV[1][1],
						p_f4->UV[2][0],p_f4->UV[2][1],
						p_f4->UV[3][0],p_f4->UV[3][1],
						p_f4->TexturePage
					);

			setZ4((struct BucketQuad*)current_bucket_pool,-res[p0].Z,-res[p1].Z,-res[p2].Z,-res[p3].Z);
			
			setShade4	(
							(struct BucketQuad*)current_bucket_pool,
							CLIP256(p_f4->Bright[0]+bright[p0]),
							CLIP256(p_f4->Bright[1]+bright[p1]),
							CLIP256(p_f4->Bright[2]+bright[p2]),
							CLIP256(p_f4->Bright[3]+bright[p3])
						);
			((struct BucketQuad*)current_bucket_pool)->DebugInfo=c0;
			((struct BucketQuad*)current_bucket_pool)->DebugFlags=0;

			add_bucket((void *)current_bucket_pool,az);

			current_bucket_pool+=sizeof(struct BucketQuad);
		}
		p_f4++;
	}

	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		if(current_bucket_pool>=end_bucket_pool)
			return;

		p0=p_f3->Points[0]-sp;
		p1=p_f3->Points[1]-sp;
		p2=p_f3->Points[2]-sp;
		if(shadow)
		{
			flag_and = flags_shadow[p0]&flags_shadow[p1]&flags_shadow[p2];	
			flag_or = flags_shadow[p0]|flags_shadow[p1]|flags_shadow[p2];	

			if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
			{
				az=(res_shadow[p0].Z+res_shadow[p1].Z+res_shadow[p2].Z)/3;
				az-=150;

				setPolyType3(
								current_bucket_pool,
								POLY_F
								
							);

				setCol3	(
							(struct BucketTri*)current_bucket_pool,
							0
						);

				setXY3	(
							(struct BucketTri*)current_bucket_pool,
							res_shadow[p0].X,res_shadow[p0].Y,
							res_shadow[p1].X,res_shadow[p1].Y,
							res_shadow[p2].X,res_shadow[p2].Y,
						);


				setZ3((struct BucketTri*)current_bucket_pool,-res_shadow[p0].Z,-res_shadow[p1].Z,-res_shadow[p2].Z);

				((struct BucketTri*)current_bucket_pool)->DebugInfo=c0;
				((struct BucketTri*)current_bucket_pool)->DebugFlags=0;
				
				add_bucket((void *)current_bucket_pool,az);

				current_bucket_pool+=sizeof(struct BucketTri);

			}
		}

		flag_and = flags[p0]&flags[p1]&flags[p2];	
		flag_or  = flags[p0]|flags[p1]|flags[p2];	

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
			az=(res[p0].Z+res[p1].Z+res[p2].Z)/3;
			az-=200;

			setPolyType3(
							current_bucket_pool,
							p_f3->DrawFlags
						);

			setCol3	(
						(struct BucketTri*)current_bucket_pool,
						p_f3->Col2
					);

			setXY3	(
						(struct BucketTri*)current_bucket_pool,
						res[p0].X,res[p0].Y,
						res[p1].X,res[p1].Y,
						res[p2].X,res[p2].Y
					);

			setUV3	(
						(struct BucketTri*)current_bucket_pool,
						p_f3->UV[0][0],p_f3->UV[0][1],
						p_f3->UV[1][0],p_f3->UV[1][1],
						p_f3->UV[2][0],p_f3->UV[2][1],
						p_f3->TexturePage
					);

			setShade3	(
							(struct BucketTri*)current_bucket_pool,
							CLIP256(p_f3->Bright[0]+bright[p0]),
							CLIP256(p_f3->Bright[1]+bright[p1]),
							CLIP256(p_f3->Bright[2]+bright[p2])
						);

			((struct BucketTri*)current_bucket_pool)->DebugInfo=c0;
			((struct BucketTri*)current_bucket_pool)->DebugFlags=0;

			add_bucket((void *)current_bucket_pool,az);

			current_bucket_pool+=sizeof(struct BucketQuad);
		}
		p_f3++;
	}	
/*				
	engine.X+=x<<8;
	engine.Y+=y<<8;
	engine.Z+=z<<8;
*/
}


void	draw_a_multi_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z)
{
	SLONG	c0;

	for(c0=prim_multi_objects[prim].StartObject;c0<prim_multi_objects[prim].EndObject;c0++)
	{
		draw_a_prim_at(c0,x,y,z,1);
	}
}
