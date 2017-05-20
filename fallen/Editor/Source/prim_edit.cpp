
#include	"Editor.hpp"

#include	"game.h"
//#include	"trig.h"
#include	"prim_draw.h"
#include	"c:\fallen\headers\animate.h"
#include	"c:\fallen\headers\io.h"
#include	"primtex.h"
#include	"perstex.h"
#include	"c:\fallen\headers\memory.h"
#include	"c:\fallen\headers\noserver.h"

CBYTE	*body_part_names[]=
{
	"pelvis",
	"lfemur",
	"ltibia",
	"lfoot",
	"torso",
	"rhumorus",
	"rradius",
	"rhand",
	"lhumorus",
	"lradius",
	"lhand",
	"skull",
	"rfemur",
	"rtibia",
	"rfoot",
	0
};

struct	Material
{
	UBYTE	R,G,B,Index;

};

struct DXMaterial
{
	float	R,
			G,
			B;
};

SLONG		material_count	=	1;

extern	void	smooth_a_prim(SLONG prim);                     //prim.h
extern	SLONG	find_colour(UBYTE *the_palette,SLONG r,SLONG g,SLONG b);
SLONG	calc_object_index(CBYTE *name,SLONG *extra);

SWORD	SelectFlag;
SWORD	SelectDrawn=0;

void	calc_prims_screen_box(UWORD	prim,SLONG x,SLONG y,SLONG z,EdRect *rect)
{
	SLONG	c0,flags;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG	min_x=999999,max_x=-999999,min_y=999999,max_y=-999999;

	p_obj    =&prim_objects[prim];

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;
	
	engine.X-=x<<8;
	engine.Y-=y<<8;
	engine.Z-=z<<8;
		
	for(c0=sp;c0<ep;c0++)
	{
		struct	SVector	pp;
		pp.X=prim_points[c0].X;
		pp.Y=prim_points[c0].Y;
		pp.Z=prim_points[c0].Z;

		//transform all points for this Object
		flags=rotate_point_gte(&pp,&global_res[c0-sp]);
//		if(!(flags & EF_CLIPFLAGS))
		{
			if(global_res[c0-sp].X<min_x)
				min_x=global_res[c0-sp].X;

			if(global_res[c0-sp].X>max_x)
				max_x=global_res[c0-sp].X;

			if(global_res[c0-sp].Y<min_y)
				min_y=global_res[c0-sp].Y;

			if(global_res[c0-sp].Y>max_y)
				max_y=global_res[c0-sp].Y;
		}
		
	}

	engine.X+=x<<8;
	engine.Y+=y<<8;
	engine.Z+=z<<8;
	if(min_x<0)
		min_x=0;
	if(min_y<0)
		min_y=0;

	rect->SetRect(min_x-2,min_y-2,max_x-min_x+4,max_y-min_y+4);
}

void	calc_prims_world_box(UWORD	prim,SLONG x,SLONG y,SLONG z,EdRect *rect)
{
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG	min_x=999999,max_x=-999999,min_y=999999,max_y=-999999;

	p_obj    =&prim_objects[prim];

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;
	
		
	for(c0=sp;c0<ep;c0++)
	{
		global_res[c0-sp].X=prim_points[c0].X+x;
		global_res[c0-sp].Y=prim_points[c0].Y+y;
		global_res[c0-sp].Z=prim_points[c0].Z+z;
		{
			if(global_res[c0-sp].X<min_x)
				min_x=global_res[c0-sp].X;

			if(global_res[c0-sp].X>max_x)
				max_x=global_res[c0-sp].X;

			if(global_res[c0-sp].Y<min_y)
				min_y=global_res[c0-sp].Y;

			if(global_res[c0-sp].Y>max_y)
				max_y=global_res[c0-sp].Y;
		}
		
	}

	if(min_x<0)
		min_x=0;
	if(min_y<0)
		min_y=0;

	rect->SetRect(min_x,min_y,max_x-min_x,max_y-min_y);
}


void	re_texture_face_using_screen_co_ords(SWORD	face,SWORD p0,SWORD p1,SWORD p2,SWORD p3)
{
	SLONG	lx,ly,dx,dy;
//	SLONG	uv[4][1];
	if(face<0)
	{
		
	}
	else
	{
//		tx=prim_faces4[face].UV[0][0]+prim_faces4[face].UV[0][1]+prim_faces4[face].UV[0][2]+prim_faces4[face].UV[0][3];
//		ty=prim_faces4[face].UV[1][0]+prim_faces4[face].UV[1][1]+prim_faces4[face].UV[1][2]+prim_faces4[face].UV[1][3];

//		tx&=~31;
//		ty&=~31;

		{
/*			
			x=prim_points[prim_faces4[face].Points[point]].X;
			y=prim_points[prim_faces4[face].Points[point]].Y;

			prim_faces4[face].UV[0][point]=tx+(x&31);
			prim_faces4[face].UV[1][point]=ty+(y&31);
*/


			lx=global_res[p0].X>>1;
			ly=global_res[p0].Y>>1;
			if(global_res[p1].X+global_res[p1].Y<lx+ly)
			{
				lx=global_res[p1].X;
				ly=global_res[p1].Y;
			}
			if(global_res[p2].X+global_res[p2].Y<lx+ly)
			{
				lx=global_res[p2].X;
				ly=global_res[p2].Y;
			}
			if(global_res[p3].X+global_res[p3].Y<lx+ly)
			{
				lx=global_res[p3].X;
				ly=global_res[p3].Y;
			}
//lx,ly
			dx=lx&0xfffe0;
			dy=ly&0xfffe0;

			prim_faces4[face].UV[0][0]=(global_res[p0].X>>1)-dx;
			prim_faces4[face].UV[0][1]=(global_res[p0].Y>>1)-dy;

			prim_faces4[face].UV[1][0]=(global_res[p1].X>>1)-dx;
			prim_faces4[face].UV[1][1]=(global_res[p1].Y>>1)-dy;

			prim_faces4[face].UV[2][0]=(global_res[p2].X>>1)-dx;
			prim_faces4[face].UV[2][1]=(global_res[p2].Y>>1)-dy;

			prim_faces4[face].UV[3][0]=(global_res[p3].X>>1)-dx;
			prim_faces4[face].UV[3][1]=(global_res[p3].Y>>1)-dy;
		}
	}
}

void	re_texture_face_using_screen_co_ords2(SWORD	face,SLONG p0,SLONG p1,SLONG p2,SLONG p3)
{
	SLONG	lx,ly,dx,dy;
//	SLONG	uv[4][1];
	if(SelectFlag==6)
	{
		if(face<0)
		{
			{

				prim_faces3[-face].UV[0][0]=-((global_res[p0].X>>1)&255)+255;
				prim_faces3[-face].UV[0][1]=((global_res[p0].Y>>1)&255);//+255;

				prim_faces3[-face].UV[1][0]=-((global_res[p1].X>>1)&255)+255;
				prim_faces3[-face].UV[1][1]=((global_res[p1].Y>>1)&255);//+255;

				prim_faces3[-face].UV[2][0]=-((global_res[p2].X>>1)&255)+255;
				prim_faces3[-face].UV[2][1]=((global_res[p2].Y>>1)&255);//+255;

			}
			
		}
		else
		{

			{

				prim_faces4[face].UV[0][0]=-((global_res[p0].X>>1)&255)+255;
				prim_faces4[face].UV[0][1]=((global_res[p0].Y>>1)&255);//+255;

				prim_faces4[face].UV[1][0]=-((global_res[p1].X>>1)&255)+255;
				prim_faces4[face].UV[1][1]=((global_res[p1].Y>>1)&255);//+255;

				prim_faces4[face].UV[2][0]=-((global_res[p2].X>>1)&255)+255;
				prim_faces4[face].UV[2][1]=((global_res[p2].Y>>1)&255);//+255;

				prim_faces4[face].UV[3][0]=-((global_res[p3].X>>1)&255)+255;
				prim_faces4[face].UV[3][1]=((global_res[p3].Y>>1)&255);//+255;
			}
		}

	}
	else
	{
		if(face<0)
		{
			{

				prim_faces3[-face].UV[0][0]=(global_res[p0].X>>1)&255;
				prim_faces3[-face].UV[0][1]=(global_res[p0].Y>>1)&255;

				prim_faces3[-face].UV[1][0]=(global_res[p1].X>>1)&255;
				prim_faces3[-face].UV[1][1]=(global_res[p1].Y>>1)&255;

				prim_faces3[-face].UV[2][0]=(global_res[p2].X>>1)&255;
				prim_faces3[-face].UV[2][1]=(global_res[p2].Y>>1)&255;

			}
			
		}
		else
		{

			{

				prim_faces4[face].UV[0][0]=(global_res[p0].X>>1)&255;
				prim_faces4[face].UV[0][1]=(global_res[p0].Y>>1)&255;

				prim_faces4[face].UV[1][0]=(global_res[p1].X>>1)&255;
				prim_faces4[face].UV[1][1]=(global_res[p1].Y>>1)&255;

				prim_faces4[face].UV[2][0]=(global_res[p2].X>>1)&255;
				prim_faces4[face].UV[2][1]=(global_res[p2].Y>>1)&255;

				prim_faces4[face].UV[3][0]=(global_res[p3].X>>1)&255;
				prim_faces4[face].UV[3][1]=(global_res[p3].Y>>1)&255;
			}
		}
	}
}

void	do_quad_clip_list(SWORD face,SLONG p0,SLONG p1,SLONG p2,SLONG p3)
{
	if(SelectFlag==5||SelectFlag==6)
	{
  		if(face_is_in_list(face))
			re_texture_face_using_screen_co_ords2(face,p0,p1,p2,p3);
		return;
	}

	if(SelectFlag==4)
	{

		if(face_is_in_list(face))
			add_face_to_list(face);
	}
	if(SelectFlag==3)
	{

		if(!face_is_in_list(face))
			add_face_to_list(face);
	}
	if(SelectFlag==2)
	{
		re_texture_face_using_screen_co_ords(face,p0,p1,p2,p3);
	}
	else
	if(global_res[p0].X>edit_info.SelectRect.GetLeft()&&
	   global_res[p0].X<edit_info.SelectRect.GetRight()&&
	   global_res[p0].Y>edit_info.SelectRect.GetTop()&&
	   global_res[p0].Y<edit_info.SelectRect.GetBottom())
		if(global_res[p1].X>edit_info.SelectRect.GetLeft()&&
		   global_res[p1].X<edit_info.SelectRect.GetRight()&&
		   global_res[p1].Y>edit_info.SelectRect.GetTop()&&
		   global_res[p1].Y<edit_info.SelectRect.GetBottom())
			if(global_res[p2].X>edit_info.SelectRect.GetLeft()&&
			   global_res[p2].X<edit_info.SelectRect.GetRight()&&
			   global_res[p2].Y>edit_info.SelectRect.GetTop()&&
			   global_res[p2].Y<edit_info.SelectRect.GetBottom())
				if(global_res[p3].X>edit_info.SelectRect.GetLeft()&&
				   global_res[p3].X<edit_info.SelectRect.GetRight()&&
				   global_res[p3].Y>edit_info.SelectRect.GetTop()&&
				   global_res[p3].Y<edit_info.SelectRect.GetBottom())
				
				{
					//What A Bloody Great If statement 
					if(SelectFlag==1)
					{
//						LogText(" quad select flag1  face %d nsf %d\n",face,next_face_selected);
						if(!face_is_in_list(face))
						{
//							LogText(" face not in list so add it\n");
							add_face_to_list(face);
						}
					}
					else
					if(SelectFlag==-1)
					{
//						LogText(" quad select flag =-1  face %d nsf %d\n",face,next_face_selected);
						if(face_is_in_list(face))
						{
//							LogText(" face is in list so remove it\n");
							add_face_to_list(face); //adding one that exists removes it
						}
					}
				}
}

void	do_tri_clip_list(SWORD face,SLONG p0,SLONG p1,SLONG p2)
{

	if(SelectFlag==8)
	{
		if(face>0)
		{
		}
	}

	if(SelectFlag==5||SelectFlag==6)
	{
  		if(face_is_in_list(face))
			re_texture_face_using_screen_co_ords2(face,p0,p1,p2,0);
		return;
	}
	if(SelectFlag==4)
	{

		if(face_is_in_list(face))
			add_face_to_list(face);
	}
	if(SelectFlag==3)
	{

		if(!face_is_in_list(face))
			add_face_to_list(face);
	}
	else
	if(global_res[p0].X>edit_info.SelectRect.GetLeft()&&
	   global_res[p0].X<edit_info.SelectRect.GetRight()&&
	   global_res[p0].Y>edit_info.SelectRect.GetTop()&&
	   global_res[p0].Y<edit_info.SelectRect.GetBottom())
		if(global_res[p1].X>edit_info.SelectRect.GetLeft()&&
		   global_res[p1].X<edit_info.SelectRect.GetRight()&&
		   global_res[p1].Y>edit_info.SelectRect.GetTop()&&
		   global_res[p1].Y<edit_info.SelectRect.GetBottom())
			if(global_res[p2].X>edit_info.SelectRect.GetLeft()&&
			   global_res[p2].X<edit_info.SelectRect.GetRight()&&
			   global_res[p2].Y>edit_info.SelectRect.GetTop()&&
			   global_res[p2].Y<edit_info.SelectRect.GetBottom())
				{
					//What A Bloody Great If statement 
					if(SelectFlag==1)
					{
						if(!face_is_in_list(face))
							add_face_to_list(face);
					}
					else
					if(SelectFlag==-1)
					{
						if(face_is_in_list(face))
							add_face_to_list(face); //adding one that exists removes it
					}
				}
}


#define prim_object_faces	prim_faces3
#define prim_object_faces4	prim_faces4
#define prim_object_points	prim_points
#define	SingleObjectFace3	PrimFace3
#define	SingleObjectFace4	PrimFace4
#define	SingleObject		PrimObject
#define	prim_next_object_face4 next_prim_face4
#define	prim_next_object_face3 next_prim_face3
#define	prim_next_object_point next_prim_point
#define	PointNo					Points



#define	PAIR_BODGE	520000

SWORD	corners[3];

// find a matching poly for hal_face that would form a quad
// finds another poly sharing 2 points, with shared edge being longest edge on poly

// find a matching poly for hal_face that would form a quad
// finds another poly sharing 2 points, with shared edge being longest edge on poly
UWORD	find_pair(UWORD start_face,UWORD end_face,UWORD half_face,UWORD tolerance)
{
	struct	PrimFace3 *p_half_face;
	struct	PrimFace3 *search_face;
	SLONG	current_face;
	UWORD	count=0,c0,points=0;
	SWORD	p1,p2;
	SLONG	len1,len2,len3,longest,second_longest,temp_len;
	UBYTE	which_longest=0;
	SLONG	dx,dy,dz;
	UBYTE	failed_match;


	corners[0]=-1;
	corners[1]=-1;
	corners[2]=-1;
	p_half_face=&prim_object_faces[half_face];
//	printf(" SEARCHING FACES from %d to %d  for match with %d \n",start_face,end_face,half_face);

	dx=prim_object_points[p_half_face->Points[0]].X-
		prim_object_points[p_half_face->Points[1]].X;

	dy=prim_object_points[p_half_face->Points[0]].Y-
		prim_object_points[p_half_face->Points[1]].Y;

	dz=prim_object_points[p_half_face->Points[0]].Z-
		prim_object_points[p_half_face->Points[1]].Z;

	len1=dx*dx+dy*dy+dz*dz;
	if(abs(dy)<5)
		len1>>=1;

	dx=prim_object_points[p_half_face->Points[1]].X-
		prim_object_points[p_half_face->Points[2]].X;
	dy=prim_object_points[p_half_face->Points[1]].Y-
		prim_object_points[p_half_face->Points[2]].Y;
	dz=prim_object_points[p_half_face->Points[1]].Z-
		prim_object_points[p_half_face->Points[2]].Z;

	len2=dx*dx+dy*dy+dz*dz;
	if(abs(dy)<5)
		len2>>=1;
	if(len2>len1)
	{
		second_longest=len1;
		longest=len2;
	}
	else
	{
		second_longest=len2;
		longest=len1;
	}

	dx=prim_object_points[p_half_face->Points[2]].X-
		prim_object_points[p_half_face->Points[0]].X;
	dy=prim_object_points[p_half_face->Points[2]].Y-
		prim_object_points[p_half_face->Points[0]].Y;
	dz=prim_object_points[p_half_face->Points[2]].Z-
		prim_object_points[p_half_face->Points[0]].Z;

	len3=dx*dx+dy*dy+dz*dz;
	if(abs(dy)<5)
		len3>>=1;
	if(len3>longest)
	{
		second_longest=longest;
		longest=len3;
	}
	else
	{
		if(len3>second_longest)
			second_longest=len3;
	}

//	printf(" len1 %d len2 %d len3 %d  longest %d \n",len1,len2,len3,longest);
	
	for(which_longest=0;which_longest<tolerance+1;which_longest++)
	{
		for(current_face=start_face;current_face<=end_face;current_face++)
		{
			count=0;
			corners[0]=-1;
			corners[1]=-1;
			corners[2]=-1;
			points=0;

			failed_match=0;

			//compare normals
			{
				struct	SVector	pn1,pn2;
				SLONG dx,dy,dz;
				calc_normal(-half_face,&pn1);
				calc_normal(-current_face,&pn2);
				dx=abs(pn1.X-pn2.X);
				dy=abs(pn1.Y-pn2.Y);
				dz=abs(pn1.Z-pn2.Z);

				if((dx*dx+dy*dy+dz*dz)>32*32)
				{
					failed_match=1;
				}
			}


//
//			if(current_face!=half_face)
			if((current_face!=half_face) && failed_match==0)
			{
				search_face=&prim_object_faces[current_face];

				// I cant use loops as I intend to stop Points being an array
				if(search_face->Points[0]==p_half_face->Points[0])
				{
					corners[0]=0;
					count++;
				} else
				if(search_face->Points[1]==p_half_face->Points[0])
				{
					corners[1]=0;
					count++;
				} else
				if(search_face->Points[2]==p_half_face->Points[0])
				{
					corners[2]=0;
					count++;
				}

				if(search_face->Points[0]==p_half_face->Points[1])
				{
					corners[0]=1;
					count++;
				} else
				if(search_face->Points[1]==p_half_face->Points[1])
				{
					corners[1]=1;
					count++;
				} else
				if(search_face->Points[2]==p_half_face->Points[1])
				{
					corners[2]=1;
					count++;
				}

				if(search_face->Points[0]==p_half_face->Points[2])
				{
					corners[0]=2;
					count++;
				} else
				if(search_face->Points[1]==p_half_face->Points[2])
				{
					corners[1]=2;
					count++;
				} else
				if(search_face->Points[2]==p_half_face->Points[2])
				{
					corners[2]=2;
					count++;
				}
				if(count==1)
				{
	//md				printf(" found a count of one  face %d\n",current_face);
	//				return(current_face);
				}
//				if(count>2)
//					printf(" we have found an identical poly poss diff order\n");

				if(count==2)
				{ // have found a poly with two shared points
	//				printf( " found a 2 shared poly  face %d\n",current_face);
	//				printf(" corner1 %d c2 %d c3 %d \n",corners[0],corners[1],corners[2]);

					//		return (current_face);
					p1=p2=-1;
					for(count=0;count<3;count++)
					{
						if(corners[count]!=-1)
							if(p1==-1)
								p1=corners[count];
							else
								p2=corners[count];
					}

					dx=prim_object_points[p_half_face->Points[p1]].X-
						prim_object_points[p_half_face->Points[p2]].X;
					dy=prim_object_points[p_half_face->Points[p1]].Y-
						prim_object_points[p_half_face->Points[p2]].Y;
					dz=prim_object_points[p_half_face->Points[p1]].Z-
						prim_object_points[p_half_face->Points[p2]].Z;

					len1=dx*dx+dy*dy+dz*dz;
					if(abs(dy)<5)
						len1>>=1;
					if(len1==longest)
					{
	//					printf("found a match  len %d  face %d\n",len1,current_face);
						return(current_face);
					}

					if(len1<longest)
					{
	//					printf("found a smaller match   len %d  face %d\n",len1,current_face);
					}

					if(len1>longest)
					{
//						printf("ERROR found a larger match   len %d  face %d\n",len1,current_face);
					}
					if(which_longest==1)
					{
						if(len1==second_longest)
						{
	//						printf("found a second longest match  len %d  face %d\n",len1,current_face);
							return(current_face);
						}
					}
/*
					if(which_longest==2)
					{
							return(current_face);
					}
*/
				}
			}
		}
	}	
	return(0);
}	/* end find_pair */




/****************************************************************************
*																			*
*																	*
*																			*
****************************************************************************/


// delete primitive face
// subtract 1 from face count
// shuffle down all later faces
// 
void	delete_prim_face3(UWORD object,UWORD	face)
{
	struct	PrimFace3 *this_face;
	struct	PrimObject *point_object;
	SLONG	end_face,start_face;
	UWORD	c0;
	point_object=&prim_objects[object];
	this_face=&prim_faces3[face];
	end_face=point_object->EndFace3;
	start_face=point_object->StartFace3;
	point_object->EndFace3--;

	for(c0=face+1;c0<=end_face;c0++)
	{
		prim_faces3[c0-1]=prim_faces3[c0];
	}
}

void	delete_prim_face4(UWORD object,UWORD	face)
{
	struct	PrimFace4 *this_face;
	struct	PrimObject *point_object;
	SLONG	end_face,start_face;
	UWORD	c0;
	point_object=&prim_objects[object];
	this_face=&prim_faces4[face];
	end_face=point_object->EndFace4;
	start_face=point_object->StartFace4;
	point_object->EndFace4--;
	for(c0=face+1;c0<=end_face;c0++)
	{
		prim_faces4[c0-1]=prim_faces4[c0];
//		printf(" del quad %d  start face %d no faces %d    face[%d]=face[%d] \n",face,start_face,no_faces,c0-1,c0);
	}
}


//
// This version of make_object_quad() uses the EDGE_VISIBLE flags in
// the prim.  It clears those flags from the prim after loading it in.
//

void make_object_quad(UWORD prim)
{
	SLONG i;
	SLONG j;
	SLONG k;

	SLONG p1;
	SLONG p2;
	SLONG other;

	PrimObject *po   = &prim_objects[prim];
	PrimFace3  *f3_a;
	PrimFace3  *f3_b;
	PrimFace4  *f4;
	
	SLONG combine;
	SLONG combine_edge;
	SLONG combine_p1;
	SLONG combine_p2;
	SLONG combine_other;

	//
	// Initialise the quads.
	//

	if (po->StartFace4 == 0)
	{
		po->StartFace4 = prim_next_object_face4;
		po->EndFace4   = prim_next_object_face4;
	}

	//
	// Go through all the face3s. If we find one with an invalid edge,
	// look for another face3 that shares that edge.  Then combine the
	// two face3s to form a quad.
	//

	for (i = po->StartFace3; i < po->EndFace3; i++)
	{
		f3_a = &prim_faces3[i];

		combine = FALSE;

		if (!(f3_a->FaceFlags & FACE_FLAG_EDGE_VISIBLE_A)) {combine = TRUE; combine_edge = 0;}
		if (!(f3_a->FaceFlags & FACE_FLAG_EDGE_VISIBLE_B)) {combine = TRUE; combine_edge = 1;}
		if (!(f3_a->FaceFlags & FACE_FLAG_EDGE_VISIBLE_C)) {combine = TRUE; combine_edge = 2;}

		if (combine)
		{
			//
			// Find a face to combine with this one.
			//

			combine_p1    = f3_a->Points[(combine_edge + 0) % 3];
			combine_p2    = f3_a->Points[(combine_edge + 1) % 3];
			combine_other = f3_a->Points[(combine_edge + 2) % 3];

			for (j = po->StartFace3; j < po->EndFace3; j++)
			{
				f3_b = &prim_faces3[j];

				if (j == i)
				{
					//
					// Don't combine a face with itself!
					//

					continue;
				}

				//
				// Check each edge.
				// 

				for (k = 0; k < 3; k++)
				{
					if (!(f3_b->FaceFlags & (FACE_FLAG_EDGE_VISIBLE << k)))
					{
						//
						// Found an invalid edge. Does this edge match the one
						// we are looking for?
						//

						p1    = f3_b->Points[(k + 0) % 3];
						p2    = f3_b->Points[(k + 1) % 3];
						other = f3_b->Points[(k + 2) % 3];

						if (p1 == combine_p2 &&
							p2 == combine_p1)
						{
							ASSERT(WITHIN(next_prim_face4, 1, MAX_PRIM_FACES4 - 1));

							//
							// We can merge triangles i and j into a quad.
							//

							f4 = &prim_faces4[next_prim_face4];

							// 2 0 3 1

							f4->Points[0]   = other;
							f4->Points[1]   = combine_p2;
							f4->Points[2]   = combine_p1;
							f4->Points[3]   = combine_other;
							f4->UV[0][0]	= f3_b->UV[(k + 2) % 3][0];	// other
							f4->UV[0][1]	= f3_b->UV[(k + 2) % 3][1];
							f4->UV[1][0]	= f3_b->UV[(k + 0) % 3][0];	// combine_p2
							f4->UV[1][1]	= f3_b->UV[(k + 0) % 3][1];
							f4->UV[2][0]	= f3_b->UV[(k + 1) % 3][0]; // combine_p1
							f4->UV[2][1]	= f3_b->UV[(k + 1) % 3][1];
							f4->UV[3][0]	= f3_a->UV[(combine_edge + 2) % 3][0];	// combine other
							f4->UV[3][1]	= f3_a->UV[(combine_edge + 2) % 3][1];
							f4->TexturePage	= f3_b->TexturePage;
							f4->Bright[0]	= 64;
							f4->Bright[1]	= 64;
							f4->Bright[2]	= 64;
							f4->Bright[3]	= 64;
							f4->DrawFlags   = POLY_GT;

							po->EndFace4    += 1;
							next_prim_face4 += 1;

							//
							// Get rid of the two old faces... this is tricky!
							//

							if (i > j)
							{
								delete_prim_face3(prim, i);
								delete_prim_face3(prim, j);

								i -= 2;
							}
							else
							{
								delete_prim_face3(prim, j);
								delete_prim_face3(prim, i);

								i -= 1;
							}

							goto finished_combining;
						}
					}
				}
			}

		  finished_combining:;
		}
	}

	//
	// Clear the edge flags from the faces.
	//

	for (i = po->StartFace3; i < po->EndFace3; i++)
	{
		f3_a = &prim_faces3[i];

		f3_a->FaceFlags &= ~(FACE_FLAG_EDGE_VISIBLE_A |
							 FACE_FLAG_EDGE_VISIBLE_B |
							 FACE_FLAG_EDGE_VISIBLE_C);
	}
}



//convert_object_to_quads()
void	make_object_quad_old(UWORD prim_object)
{
	struct	SingleObjectFace3 *this_face,*cur_face,*temp_face;
	struct	SingleObjectFace4 *this_face4;
	struct SingleObject *point_object;
	SLONG	no_faces,start_face,end_face;
	SLONG	no_faces4,start_face4,end_face4;
	UWORD	c0,c1,temp_match;
	UWORD	p_cur,p_temp;
	UWORD	failed_match=0,temp_unique,cur_unique;
	UWORD	match_tolerance;
	UWORD	al;
	point_object=&prim_objects[prim_object];

	no_faces=point_object->EndFace3-point_object->StartFace3;
	start_face=point_object->StartFace3;
	end_face=point_object->EndFace3;


	if(point_object->StartFace4==0)
	{
		point_object->StartFace4=prim_next_object_face4;
		point_object->EndFace4=prim_next_object_face4;
	}

/*
	temp_normal=next_normal;
	for(c0=start_face;c0<end_face;c0++)
	{
		prim_object_faces[c0].FaceNormal=new_prim_normal(c0);
	}
	next_normal=temp_normal;
*/

	for(match_tolerance=0;match_tolerance<2;match_tolerance++)	
	for(c0=start_face;c0<end_face;c0++)
	{
		cur_face=&prim_object_faces[c0];
		failed_match=1;
		temp_match=start_face;
		while(failed_match && temp_match<=end_face)
		{
			temp_match=find_pair(temp_match,end_face,c0,match_tolerance);

			failed_match=0;
			temp_face=&prim_object_faces[temp_match];
			if(!temp_match)
			{
				failed_match=1;
				temp_match=end_face;
			}
/*
			else
			if(match_tolerance==0)
			{
				struct	SVector	pn1,pn2;
				SLONG dx,dy,dz;
				calc_normal(temp_match,&pn1);
				calc_normal(c0,&pn2);
				dx=abs(pn1.X-pn2.X);
				dy=abs(pn1.Y-pn2.Y);
				dz=abs(pn1.Z-pn2.Z);

				if((dx*dx+dy*dy+dz*dz)>20)
					failed_match=1;


			}
*/


			if(!failed_match)
			{
				if(corners[0]!=-1)
				{
					p_cur=corners[0];
					p_temp=0;

				}
				else
					temp_unique=0;

			}
			if(!failed_match)
			{
				if(corners[1]!=-1)
				{

					p_cur=corners[1];
					p_temp=1;
				}
				else
					temp_unique=1;

			}

			if(!failed_match)
			{
				if(corners[2]!=-1)
				{
					p_cur=corners[2];
					p_temp=2;

				}
				else
					temp_unique=2;
			}

			if(!failed_match)
			{
				
//				printf(" passed texture temp unique %d \n",temp_unique);
			}
			else
			{
				temp_match++;
//				printf(" inc temp_match \n");
			}
		}
		if(!failed_match)
		{
			if(corners[0]==0||corners[1]==0||corners[2]==0)
				if(corners[0]==1||corners[1]==1||corners[2]==1)
					cur_unique=2;
			if(corners[0]==2||corners[1]==2||corners[2]==2)
				if(corners[0]==1||corners[1]==1||corners[2]==1)
					cur_unique=0;
			if(corners[0]==0||corners[1]==0||corners[2]==0)
				if(corners[0]==2||corners[1]==2||corners[2]==2)
					cur_unique=1;

			
			this_face4=&prim_object_faces4[prim_next_object_face4];

			for(c1=0;c1<3;c1++)
			{
				this_face4->Points[c1]=cur_face->Points[(c1+cur_unique)%3];

			}
			this_face4->Points[3]=temp_face->Points[temp_unique];

			this_face4->UV[0][0]	=	0;
			this_face4->UV[0][1]	=	0;
			this_face4->UV[1][0]	=	31;
			this_face4->UV[1][1]	=	0;
			this_face4->UV[2][0]	=	0;
			this_face4->UV[2][1]	=	31;
			this_face4->UV[3][0]	=	31;
			this_face4->UV[3][1]	=	31;
			this_face4->TexturePage	=	0;



			if(cur_face->DrawFlags==POLY_G)
			{
				this_face4->DrawFlags=POLY_G;
				this_face4->Col2=cur_face->Col2;
//				LogText(" quad %d gets colour index of %d = %f %f %f \n",prim_next_object_face4,cur_face->Col2,dx_materials[cur_face->Col2].R,dx_materials[cur_face->Col2].G,dx_materials[cur_face->Col2].B);

			}
			else
			{
				this_face4->DrawFlags=POLY_FLAG_TEXTURED|POLY_FLAG_GOURAD;
				this_face4->Col2=cur_face->Col2;
			}
			if(this_face4->Col2>255)
				LogText(" funny col %d\n",this_face4->Col2);
			this_face4->Bright[0]	=	0; //128;
			this_face4->Bright[1]	=	0; //128;
			this_face4->Bright[2]	=	0; //128;
			this_face4->Bright[3]	=	0; //128;
			
			point_object->EndFace4++;
			prim_next_object_face4++;
			if(c0>temp_match)
			{
				delete_prim_face3(prim_object,c0);
				delete_prim_face3(prim_object,temp_match);
				c0-=2;
			}
			else
			{
				delete_prim_face3(prim_object,temp_match);
				delete_prim_face3(prim_object,c0);
				c0--;
			}
			
			end_face-=2;
		}
	}
}





/*
void	fix_multi_object_for_anim(UWORD obj)
{
	SLONG	sp,ep;
	UWORD	c0,c1;
	struct	PrimMultiAnim	*p_anim;
	struct	PrimObject	*p_obj;
	struct	Matrix33 *mat;
	struct	Matrix31	temp;

	for(c0=prim_multi_objects[obj].StartObject;c0<=prim_multi_objects[obj].EndObject;c0++)
	{
		p_obj=&prim_objects[c0];
		mat=&prim_multi_anims[p_obj->MatrixHead].Mat;
//		p_obj->MatrixCurr=prim_multi_anims[p_obj->MatrixHead].Next;

		sp=p_obj->StartPoint;
		ep=p_obj->EndPoint;
		p_anim=&prim_multi_anims[p_obj->MatrixCurr];

		for(c1=sp;c1<ep;c1++)
		{
			prim_points[c1].X-=p_anim->DX;
			prim_points[c1].Y-=p_anim->DY;
			prim_points[c1].Z-=p_anim->DZ;

//			matrix_transform((struct Matrix31*)&temp,mat, (struct Matrix31*)&prim_points[c1]);
//			memcpy(&prim_points[c1],&temp,sizeof(struct PrimPoint));
		}
	}
}

#define	SWAP(x,y)	{SLONG temp;temp=x;x=y;y=temp;}
SBYTE	read_multi_vue(SLONG m_object)
{
	FILE	*handle;
	char	fname[36];
	char	ts[MAX_3DS_LEN];
	float	f,fM[3][3],fx,fy,fz;
	struct	Matrix33	mat;
	int		retval;
	char	pname[100],pname2[100];
	SLONG	c_object=prim_multi_objects[1].StartObject-1,c_matrix=next_prim_matrix;
	SLONG	c0;
	UWORD	mat_index[100];

	sprintf(fname,"data/man.vue");
	memset(mat_index,0,200);

	if(handle = fopen(fname,"r"))
	{
		do
		{						  
			retval = fscanf(handle,"%s",ts);
			if(!strcmp(ts,"frame"))
			{
				SLONG	frame;

				fscanf(handle,"%d",&frame);

				for(c0=prim_multi_objects[m_object].StartObject;c0<=prim_multi_objects[m_object].EndObject;c0++)
				{
					fscanf(handle,"%s %s %s %f %f %f %f %f %f %f %f %f %f %f %f",ts,pname,pname2,&fM[0][0],&fM[0][1],&fM[0][2],&fM[1][0],&fM[1][1],&fM[1][2],&fM[2][0],&fM[2][1],&fM[2][2],&fx,&fy,&fz);


					//    r	 c



#ifdef	MARKA
					mat.M[0][0]=(SLONG)(fM[1][0]*(1<<15));
					mat.M[0][1]=(SLONG)(fM[1][2]*(1<<15));
					mat.M[0][2]=(SLONG)(-fM[1][1]*(1<<15));
					mat.M[1][0]=(SLONG)(-fM[0][0]*(1<<15));
					mat.M[1][1]=(SLONG)(-fM[0][2]*(1<<15));
					mat.M[1][2]=(SLONG)(fM[0][1]*(1<<15));
					mat.M[2][0]=(SLONG)(fM[2][0]*(1<<15));
					mat.M[2][1]=(SLONG)(fM[2][2]*(1<<15));
					mat.M[2][2]=(SLONG)(-fM[2][1]*(1<<15));

					SWAP(mat.M[0][1],mat.M[1][0])
					SWAP(mat.M[0][2],mat.M[2][0])
					SWAP(mat.M[1][2],mat.M[2][1])
#endif


					mat.M[0][0]=(SLONG)(fM[0][0]*(1<<15));
					mat.M[0][1]=(SLONG)(fM[1][0]*(1<<15));
					mat.M[0][2]=(SLONG)(fM[2][0]*(1<<15));
					mat.M[1][0]=(SLONG)(fM[0][1]*(1<<15));
					mat.M[1][1]=(SLONG)(fM[1][1]*(1<<15));
					mat.M[1][2]=(SLONG)(fM[2][1]*(1<<15));
					mat.M[2][0]=(SLONG)(fM[0][2]*(1<<15));
					mat.M[2][1]=(SLONG)(fM[1][2]*(1<<15));
					mat.M[2][2]=(SLONG)(fM[2][2]*(1<<15));

					memcpy(&prim_multi_anims[next_prim_multi_anim].Mat,&mat.M[0][0],sizeof(struct Matrix33));
					prim_multi_anims[next_prim_multi_anim].DX=(SLONG)fx;
					prim_multi_anims[next_prim_multi_anim].DY=(SLONG)fy;
					prim_multi_anims[next_prim_multi_anim].DZ=(SLONG)fz;
					if(mat_index[c0-prim_multi_objects[m_object].StartObject]==0)
					{
						prim_objects[c0].MatrixHead=next_prim_multi_anim;
						prim_objects[c0].MatrixCurr=next_prim_multi_anim;
					}
					else
					{
						prim_multi_anims[mat_index[c0-prim_multi_objects[m_object].StartObject]].Next=next_prim_multi_anim;
					}
					mat_index[c0-prim_multi_objects[m_object].StartObject]=next_prim_multi_anim;
					next_prim_multi_anim++;
				}
			}
		} while(retval>=0);
		fclose(handle);
	}
	fix_multi_object_for_anim(m_object);
	return(0);
}
*/

UBYTE		got_torso	=	0;


#ifdef	MATRIX_CALC
rot X
1   0   0
0   cx  sx
0   -sx cx

rot Y            rotXY
cy  0   -sy      cy   0    -sy
0   1    0       sxsy cx   sxcy
sy  0    cy      cxsy -sx  cxcy


rot Z            rotXYZ
cz  sz   0       czcy         cysz          -sy
-sz cz   0       sxsycz-cxsz  sxsysz+cxcz   sxcy
0   0    1       cxsycz+sxsz  cxsysz-sxcz   cxcy  


[A B C]
[D E F]
[G H I]


czcy=A
cysz=B
-sy=C
sxsycz-cxsz=D
sxsysz+cxcz=E
sxcy=F
cxsycz+sxsz=G
cxsysz-sxcz=H
cxcy=I


czcy=A
cysz=B
-sy=C
sxcz*(-C)-cxsz=D
sx*(-C)sz+cxcz=E
sxcy=F
cx(-C)cz+sxsz=G
cx(-C)sz-sxcz=H
cxcy=I


cy=A/cz

cy=I/cx

A/cz = I/cx	      (A*cx)/cz=I      I*cz/A

(cz*I)/A=cx


cx(-C)cz+sxsz=G

(cz*I(-C)*cz)/A+sxsz=G



#endif

void	comma_to_dot(CBYTE *str)
{
	while(*str)
	{
		if(*str==',')
			*str='.';
		str++;
	}
}

extern	SLONG	x_centre,y_centre,z_centre;
//extern DXMaterial dx_materials[200];





//
// Functions to load SEX files. They return FALSE on failure.
//
// 'scale' is ignored.  If (offset) then the 'y' of the centre of the object
// is taken as being the lowest point, otherwise it is at the average y.
//

//
// A buffer for texture coordinates.
// 

#define MAX_SEX_UVS 4096

struct
{
	float u;
	float v;

} sex_uv[MAX_SEX_UVS];

#define MAX_LINE_LEN  256
#define MAX_NAME_LEN  64
#define MAX_MATERIALS 64

SLONG read_sex(CBYTE *fname, SLONG scale /* Ignored */, SLONG offset)
{
	SLONG i;

	SLONG match;

	SLONG num_f;
	SLONG num_v;
	SLONG num_tv;
	SLONG num_m;

	SLONG base_f;
	SLONG base_v;

	SLONG edge_a;
	SLONG edge_b;
	SLONG edge_c;

	float version = 1.0F;

	float r;
	float g;
	float b;

	float x;
	float y;
	float z;

	float v;

	float tu;
	float tv;

	float av_x;
	float av_y;
	float av_z;
	float av_u;
	float av_v;

	double base_fu;
	double base_fv;

	SLONG base_iu;
	SLONG base_iv;

	SLONG iu;
	SLONG iv;

	float min_x;
	float min_y;
	float min_z;

	float max_x;
	float max_y;
	float max_z;

	float centre_x;
	float centre_y;
	float centre_z;

	float pivot_x;
	float pivot_y;
	float pivot_z;

	SLONG num_pivots;

	SLONG num_points = 0;
	SLONG num_faces  = 0;

	SLONG m;
	SLONG p1;
	SLONG p2;
	SLONG p3;
	SLONG t[3];

	CBYTE sided[MAX_NAME_LEN];
	CBYTE alpha[MAX_NAME_LEN];
	CBYTE tname[MAX_NAME_LEN];

	PrimObject *po;

	struct
	{
		float r;
		float g;
		float b;

		//
		// Bools...
		//

		UBYTE double_sided;
		UBYTE additive_alpha;
		UBYTE page;				// Prim texture number 0 - 191	// 255 => not loaded.
		UBYTE col;				// Nearest match in palette.

		UBYTE texture_page;		// Which page 0 - 15 this texture will be plonked on.
		UBYTE texture_base_u;
		UBYTE texture_base_v;
		UBYTE padding;

		CBYTE name[MAX_NAME_LEN];

	} mat[MAX_MATERIALS];

	CBYTE oname[MAX_NAME_LEN];
	CBYTE line [MAX_LINE_LEN];

	FILE *handle;

	handle = fopen(fname, "rb");

	if (handle == NULL)
	{
		//
		// Could not open file.
		//

		return FALSE;
	}

	//
	// Our new prim.
	//

	ASSERT(WITHIN(next_prim_object, 1, MAX_PRIM_OBJECTS - 1));

	po = &prim_objects[next_prim_object++];

	po->StartPoint = next_prim_point;
	po->StartFace3 = next_prim_face3;
	po->StartFace4 = next_prim_face4;
	po->EndPoint   = 0;
	po->EndFace3   = 0;
	po->EndFace4   = 0;

	//
	// Keep track of the bounding box and average of the point.
	//

	av_x = 0;
	av_y = 0;
	av_z = 0;

	min_x = float(+INFINITY);
	min_y = float(+INFINITY);
	min_z = float(+INFINITY);

	max_x = float(-INFINITY);
	max_y = float(-INFINITY);
	max_z = float(-INFINITY);

	num_pivots = 0;

	//
	// What version of SEX file we are loading.
	//

	SLONG old_version = FALSE;

	//
	// Decode the file a line at a time.
	//

	while(fgets(line, MAX_LINE_LEN, handle))
	{
		//
		// Ignore comment lines
		//

		if (line[0] == '#')
		{
			continue;
		}

		//
		// Is this a triangle mesh line?
		//

		if (strncmp(line, "Triangle mesh: ", 15) == 0)
		{
			//
			// Put the name in the prim.
			//

			//po->ObjectName
			strncpy(prim_names[next_prim_object-1], line + 15, 32);

			base_f = next_prim_face3;
			base_v = next_prim_point;
			
			num_f  = 0;
			num_v  = 0;
			num_tv = 0;
			num_m  = 0;

			continue;
		}

		//
		// Version number?
		// 

		match = sscanf(line, "Version: %f", &v);

		if (match == 1)
		{
			version = v;
		}

		//
		// Is this a material definition line?
		//
	   
		match = sscanf(line, "Material: DiffuseRGB (%f,%f,%f), %s sided, %s alpha, filename %s", &r, &g, &b, sided, alpha, tname);

		if (match != 6)
		{
			float shininess;
			float shinstr;

			//
			// We might have the very latest version!
			//

			match = sscanf(line, "Material: DiffuseRGB (%f,%f,%f), shininess %f, shinstr %f, %s sided, %s alpha, filename %s", &r, &g, &b, &shininess, &shinstr, sided, alpha, tname);

			if (match == 8)
			{
				match = 6;
			}
		}


		if (match == 6)
		{
			ASSERT(WITHIN(num_m, 0, MAX_MATERIALS - 1));

			//
			// Found a material.
			//

			strncpy(mat[num_m].name, tname, MAX_NAME_LEN);

			mat[num_m].additive_alpha = (strcmp(alpha,  "Additive") == 0);
			mat[num_m].double_sided   = (strcmp(sided,    "Double") == 0);
			mat[num_m].page           = 255;	// 255 => Not loaded
			mat[num_m].r              = r;
			mat[num_m].g              = g;
			mat[num_m].b              = b;

			//
			// Find the colour in the palette that matched most closely
			// this colour (r,g,b)
			//

			{
				mat[num_m].col  = find_colour((UBYTE*)ENGINE_palette, UBYTE(r * 255.0F), UBYTE(g * 255.0F), UBYTE(b * 255.0F));
			}

			num_m += 1;

			continue;
		}

		//
		// Is this a pivot line?
		//

		match = sscanf(line, "Pivot: (%f, %f, %f)", &x, &y, &z);

		if (match == 3)
		{
			//
			// Convert from 3ds orientation to our one.
			//
			
			SWAP_FL(y,z);

			pivot_x = (x * GAME_SCALE) / GAME_SCALE_DIV;
			pivot_y = (y * GAME_SCALE) / GAME_SCALE_DIV;
			pivot_z = (z * GAME_SCALE) / GAME_SCALE_DIV;

			num_pivots += 1;

			continue;
		}

		//
		// Is this a vertex line?
		//

		match = sscanf(line, "Vertex: (%f,%f,%f)", &x, &y, &z);

		if (match == 3)
		{	
			ASSERT(WITHIN(next_prim_point, 1, MAX_PRIM_POINTS - 1));

			//
			// Convert from 3ds orientation to our one.
			//

			SWAP_FL(y,z);

			prim_points[next_prim_point].X = (SLONG) ((x * GAME_SCALE) / GAME_SCALE_DIV);
			prim_points[next_prim_point].Y = (SLONG) ((y * GAME_SCALE) / GAME_SCALE_DIV);
			prim_points[next_prim_point].Z = (SLONG) ((z * GAME_SCALE) / GAME_SCALE_DIV);

			//
			// Update the bounding rectangle of this object.
			//

			if (x < min_x) {min_x = x;}
			if (y < min_y) {min_y = y;}
			if (z < min_z) {min_z = z;}

			if (x > max_x) {max_x = x;}
			if (y > max_y) {max_y = y;}
			if (z > max_z) {max_z = z;}

			av_x += x;
			av_y += y;
			av_z += z;

			num_v           += 1;
			num_points      += 1;
			next_prim_point += 1;

			continue;
		}

		//
		// Is this a TVertex line?
		//

		match = sscanf(line, "Texture Vertex: (%f,%f)", &tu, &tv);

		if (match == 2)
		{
			ASSERT(WITHIN(num_tv, 0, MAX_SEX_UVS));

			sex_uv[num_tv].u = tu;
			sex_uv[num_tv].v = tv;

			num_tv += 1;

			continue;
		}

		//
		// Is this a face line?
		//

		match = sscanf(line, "Face: Material %d xyz (%d,%d,%d) uv (%d,%d,%d) edge (%d,%d,%d)", &m, &p1, &p2, &p3, &t[0], &t[1], &t[2], &edge_a, &edge_b, &edge_c);

		if (match == 7)
		{
			//
			// Found an old-version face.
			//

			edge_a = 1;
			edge_b = 1;
			edge_c = 1;

			match  = 10;

			//
			// Use the old quadify routine.
			//

			old_version = TRUE;
		}

		if (match == 10)
		{	
			ASSERT(WITHIN(m,  0, num_m  - 1));

			ASSERT(WITHIN(p1, 0, num_v  - 1));
			ASSERT(WITHIN(p2, 0, num_v  - 1));
			ASSERT(WITHIN(p3, 0, num_v  - 1));

			ASSERT(WITHIN(t[0], 0, num_tv - 1));
			ASSERT(WITHIN(t[1], 0, num_tv - 1));
			ASSERT(WITHIN(t[2], 0, num_tv - 1));

			ASSERT(WITHIN(next_prim_face3, 1, MAX_PRIM_FACES3 - 1));

			//
			// Has the texture been loaded? 
			//

			if (mat[m].page == 255)
			{
				//
				// Find the page number of this texture.
				//

				mat[m].page = PRIMTEX_get_number(mat[m].name);

				//
				// Where abouts on which page is this texture kept?
				//

				mat[m].texture_page   =  (mat[m].page >> 6) + 11;
				mat[m].texture_base_u = ((mat[m].page >> 0) & 0x7) * 32;
				mat[m].texture_base_v = ((mat[m].page >> 3) & 0x7) * 32;
			}

			prim_faces3[next_prim_face3].Points[0] = base_v + p3;	// 3 2 1... backwards!
			prim_faces3[next_prim_face3].Points[1] = base_v + p2;
			prim_faces3[next_prim_face3].Points[2] = base_v + p1;

			prim_faces3[next_prim_face3].TexturePage = mat[m].texture_page;
			prim_faces3[next_prim_face3].DrawFlags   = POLY_GT;
			prim_faces3[next_prim_face3].FaceFlags   = 0;
			prim_faces3[next_prim_face3].Col2         = mat[m].col;

			av_u = (sex_uv[t[0]].u + sex_uv[t[1]].u + sex_uv[t[2]].u) * 0.333F;
			av_v = (sex_uv[t[0]].v + sex_uv[t[1]].v + sex_uv[t[2]].v) * 0.333F;

			base_iu = SLONG(av_u);
			base_iv = SLONG(av_v);

			base_fu = float(base_iu);
			base_fv = float(base_iv);

			for (i = 0; i < 3; i++)
			{
				tu = sex_uv[t[2 - i]].u - base_fu;
				tv = sex_uv[t[2 - i]].v - base_fv;

				tv = 1.0F - tv;	// TGA are upside down.

				iu = SLONG(tu * 31.0F);
				iv = SLONG(tv * 31.0F);

				SATURATE(iu, 0, 31);
				SATURATE(iv, 0, 31);

				iu += mat[m].texture_base_u;
				iv += mat[m].texture_base_v;

				prim_faces3[next_prim_face3].UV[i][0] = iu;
				prim_faces3[next_prim_face3].UV[i][1] = iv;
			}

			//
			// The flags are used by make_object_quad()
			//

			if (edge_a) {prim_faces3[next_prim_face3].FaceFlags |= FACE_FLAG_EDGE_VISIBLE_B;}
			if (edge_b) {prim_faces3[next_prim_face3].FaceFlags |= FACE_FLAG_EDGE_VISIBLE_A;}
			if (edge_c) {prim_faces3[next_prim_face3].FaceFlags |= FACE_FLAG_EDGE_VISIBLE_C;}

			num_f           += 1;
			num_faces       += 1;
			next_prim_face3 += 1;

			continue;
		}
	}

	fclose(handle);

	//
	// Finish off the prim.
	//

	po->EndPoint = po->StartPoint + num_points;
	po->EndFace3 = po->StartFace3 + num_faces;
	po->EndFace4 = next_prim_face4;

	ASSERT(po->EndPoint == next_prim_point);
	ASSERT(po->EndFace3 == next_prim_face3);

	//
	// Calculate the average.
	//

	av_x /= float(num_points);
	av_y /= float(num_points);
	av_z /= float(num_points);

	//
	// Calculate the centre of the bounding box.
	//

	float mid_x = (min_x + max_x) * 0.5f;
	float mid_y = (min_y + max_y) * 0.5f;
	float mid_z = (min_z + max_z) * 0.5f;

	//
	// Find what we use as the centre of the object.
	//

	if (num_pivots == 1)
	{
		//
		// use the pivot.
		//

		centre_x = pivot_x;
		centre_y = pivot_y;
		centre_z = pivot_z;
	}
	else
	{
		centre_x = mid_x;
		centre_z = mid_z;

		if (offset)
		{
			//
			// 'y' is relative to the min of the bounding box.
			//

			centre_y = min_y;
		}
		else
		{
			//
			// 'y' is relative to the average y of all the points.
			//

			centre_y = av_y;
		}
	}

	//
	// Centre the object.
	//

	for (i = po->StartPoint; i < po->EndPoint; i++)
	{
		prim_points[i].X -= centre_x;
		prim_points[i].Y -= centre_y;
		prim_points[i].Z -= centre_z;
	}

	//
	// Create face4s out of the face3s.
	//

	if (old_version)
	{
		make_object_quad_old(next_prim_object - 1);
	}
	else
	{
		make_object_quad(next_prim_object - 1);
	}

	//
	// This might have made new textures on n:
	//


//	free_game_textures(FREE_SHARED_TEXTURES);
//	load_game_textures(LOAD_SHARED_TEXTURES);

	return TRUE;
}
/*
typedef struct
{
	UBYTE red;
	UBYTE green;
	UBYTE blue;

} ENGINE_Col;

extern ENGINE_Col ENGINE_palette[256];
*/


float	det22(float a,float b,float c,float d)
{
	float ans;

	ans=a*b-c*d;
	return(ans);
}

float	det33(float a1,float a2,float a3,float b1,float b2,float b3,float c1,float c2,float c3)
{
	float	ans;

	ans= a1*det22(b2,b3,c2,c3)
		-b1*det22(a2,a3,c2,c3)
		+c1*det22(a2,a3,b2,b3);

	return(ans);

}


void	invert_matrix(float *mat,float *out)
{
	float	det;

	det=det33(mat[0],mat[1],mat[2],mat[3],mat[4],mat[5],mat[6],mat[7],mat[8]);

	//
	// now finish it off?
	//
	


}

SLONG read_multi_sex(CBYTE *fname,float shrink)
{
	SLONG i;

	SLONG match;
	SLONG c0;

	SLONG num_f;
	SLONG num_v;
	SLONG num_tv;
	SLONG num_m;

	SLONG base_f;
	SLONG base_v;

	SLONG edge_a;
	SLONG edge_b;
	SLONG edge_c;

	float version = 1.0F;

	float r;
	float g;
	float b;

	float x;
	float y;
	float z;

	float v;

	float tu;
	float tv;

	float av_u;
	float av_v;

	double base_fu;
	double base_fv;

	SLONG base_iu;
	SLONG base_iv;

	SLONG iu;
	SLONG iv;

	SLONG num_points = 0;
	SLONG num_faces  = 0;

	SLONG m;
	SLONG p1;
	SLONG p2;
	SLONG p3;
	SLONG t[3];

	CBYTE sided[MAX_NAME_LEN];
	CBYTE alpha[MAX_NAME_LEN];

	SLONG	object_index,start_object,object_count=0,extra_object_count=0;
	CBYTE	object_name[100];
	SLONG	extra;
	SLONG	re_center=0;
	SLONG	its_human=0;
	UBYTE	its_doggy=0;


	PrimObject      *po;
	PrimMultiObject *pmo;
	SLONG	darci=0;
	SLONG	c2;


	struct
	{
		float r;
		float g;
		float b;

		//
		// Bools...
		//

		UBYTE double_sided;
		UBYTE additive_alpha;
		UWORD page;			// People texture number 0 - 127 // 255 => not loaded
		UBYTE col;			// Nearest match in palette.

		UBYTE texture_page;		// Which page 0 - 15 this texture will be plonked on.
		UBYTE texture_base_u;
		UBYTE texture_base_v;
		UBYTE padding;

		CBYTE name[MAX_NAME_LEN];

	} mat[MAX_MATERIALS];

	CBYTE oname[MAX_NAME_LEN];
	CBYTE line [MAX_LINE_LEN];

	FILE *handle;
	UBYTE	unique[35];

	memset(unique,0,35);

	//
	// Oh me oh my! What's this?
	//

	_strlwr(fname);

	if(fname[6]=='a'&& fname[7]=='r' && fname[8]=='c')
		darci=1;
	if(fname[5]=='d' && fname[6]=='o'&& fname[7]=='g')
		its_doggy=1;

	handle = fopen(fname, "rb");

	DebugText(" load mesh %s \n",fname);

	if (handle == NULL)
	{
		//
		// Could not open file.
		//

		return FALSE;
	}
	
	//
	// Our new multi-prim.
	//

	ASSERT(WITHIN(next_prim_multi_object, 1, MAX_PRIM_MOBJECTS - 1));

	pmo = &prim_multi_objects[next_prim_multi_object++];

	pmo->StartObject = next_prim_object;
	pmo->EndObject   = 0;
	pmo->Tween       = 0;
	pmo->Frame       = 0;

	//
	// What version of SEX file we are loading.
	//

	SLONG old_version = FALSE;

	//
	// Our first prim is marked as non-existent.
	//

	po = NULL;

	//
	// Decode the file a line at a time.
	//

	while(fgets(line, MAX_LINE_LEN, handle))
	{
		//
		// Ignore comment lines
		//

		if (line[0] == '#')
		{
			continue;
		}


		//
		// Version number?
		// 

		match = sscanf(line, "Version: %f", &v);

		if (match == 1)
		{
			version = v;
		}

		//
		// Is this a triangle mesh line?
		//

		if (strncmp(line, "Triangle mesh: ", 15) == 0)
		{
			if (po != NULL)
			{
				ASSERT(WITHIN(po, &prim_objects[1], &prim_objects[MAX_PRIM_OBJECTS - 1]));

				//
				// Finish off the current prim-object.
				//

				po->EndPoint = next_prim_point;
				po->EndFace3 = next_prim_face3;
				po->EndFace4 = 0; //next_prim_face4;

				ASSERT(po->EndPoint == po->StartPoint + num_points);
				ASSERT(po->EndFace3 == po->StartFace3 + num_faces );

				//
				// DONT QUADIFY THE CURRENT PRIM.
				//
			}

			//
			// Create a new prim object.
			// 

			ASSERT(WITHIN(next_prim_object, 1, MAX_PRIM_OBJECTS - 1));

			strncpy(object_name, line + 15, 32);
			for(c2=0;c2<strlen(object_name);c2++)
			{
				object_name[c2]=tolower(object_name[c2]);
			}

			object_index=calc_object_index(object_name,&extra);
									   
			DebugText(" object name %s INDEX %d extra %d\n",object_name,object_index,extra);
//			if(object_index<0)
//				return(0);      //body part name unknown
			//
			// load strange objects in like normal
			//

			if(object_index<0)
			{
				extra=0;
				object_index=object_count;
			}

			if(extra)
			{
//					test_chunk.BodyBits[object_index][extra]=15+extra_object_count;
				object_index=15+extra_object_count;
				extra_object_count++;
				DebugText(" extra object count now %d object_index %d\n",extra_object_count,object_index+next_prim_object);
			}
			else
			{
				unique[object_index]++;

				ASSERT(unique[object_index]==1);

				object_count++;
				DebugText(" object count now %d object_index %d\n",object_count,object_index+next_prim_object);
			}

			object_index+=next_prim_object;

			po = &prim_objects[object_index];

			po->StartPoint = next_prim_point;
			po->StartFace3 = next_prim_face3;
			po->StartFace4 = 0; //next_prim_face4;
			po->EndPoint   = next_prim_point;
			po->EndFace3   = next_prim_face3;
			po->EndFace4   = 0; //next_prim_face4;

			//
			// Put the name in the prim.
			//

//			strncpy(po->ObjectName, line + 15, 32);
			strncpy(prim_names[object_index], line + 15, 32);

			for(c2=0;c2<strlen(prim_names[object_index]);c2++)
			{
				prim_names[object_index][c2]=tolower(prim_names[object_index][c2]);
//				po->ObjectName[c2]=tolower(po->ObjectName[c2]);
			}


			base_f = next_prim_face3;
			base_v = next_prim_point;
			
			num_f  = 0;
			num_v  = 0;
			num_tv = 0;
			num_m  = 0;
			num_points = 0;
			num_faces = 0;

			continue;
		}

		//
		// Is this a material definition line?
		//
	   
		match = sscanf(line, "Material: DiffuseRGB (%f,%f,%f), %s sided, %s alpha, filename", &r, &g, &b, sided, alpha);

		if (match != 5)
		{
			float shininess;
			float shinstr;

			//
			// We might have the very latest version!
			//

			match = sscanf(line, "Material: DiffuseRGB (%f,%f,%f), shininess %f, shinstr %f, %s sided, %s alpha, filename", &r, &g, &b, &shininess, &shinstr, sided, alpha);

			if (match == 7)
			{
				match = 5;
			}
		}

		if (match == 5)
		{
			//
			// We must find the texture name now.
			//

			CBYTE *tname = strstr(line, "filename");

			ASSERT(tname);

			tname += 9;

			ASSERT(WITHIN(num_m, 0, MAX_MATERIALS - 1));

			//
			// Found a material.
			//

			strncpy(mat[num_m].name, tname, MAX_NAME_LEN);

			//
			// Get rid of control characters at the end of the string.
			// 

			{
				CBYTE *ch;

				for (ch = mat[num_m].name; *ch; ch++);

				while(1)
				{
					ch -= 1;

					if (isalnum(*ch))
					{
						break;
					}

					*ch = 0;
				}
			}

			mat[num_m].additive_alpha = (strcmp(alpha,  "Additive") == 0);
			mat[num_m].double_sided   = (strcmp(sided,    "Double") == 0);
			mat[num_m].page           =  255;
			mat[num_m].r              =  r;
			mat[num_m].g              =  g;
			mat[num_m].b              =  b;

			//
			// Find the colour in the palette that matched most closely
			// this colour (r,g,b)
			//

			{
				SLONG col;

				mat[num_m].col = find_colour((UBYTE*)ENGINE_palette, UBYTE(r * 255.0F), UBYTE(g * 255.0F), UBYTE(b * 255.0F));

				LogText(" found col %d for obj rgb %d %d %d \n",num_m,(SLONG)(r*255.0), (SLONG)(g*255.0), (SLONG)(b*255.0) );
				col=mat[num_m].col;
				LogText(" col used %d  rgb %d %d %d \n",col,ENGINE_palette[col].red,ENGINE_palette[col].green,ENGINE_palette[col].blue);

			}

			num_m += 1;

			continue;
		}

		//
		// Is this a vertex line?
		//

		match = sscanf(line, "Vertex: (%f,%f,%f)", &x, &y, &z);

		if (match == 3)
		{	
//			DebugText(" num_v %d num_p %d next_prim_point %d\n",num_v,num_points,next_prim_point);
			ASSERT(WITHIN(next_prim_point, 1, MAX_PRIM_POINTS - 1));

			//
			// Convert from 3ds orientation to our one.
			//

			SWAP_FL(y,z);

			prim_points[next_prim_point].X	= (SLONG)((x*GAME_SCALE)/(GAME_SCALE_DIV*shrink));
			prim_points[next_prim_point].Y	= (SLONG)((y*GAME_SCALE)/(GAME_SCALE_DIV*shrink)); //- md
			prim_points[next_prim_point].Z	= (SLONG)((z*GAME_SCALE)/(GAME_SCALE_DIV*shrink));

			num_v           += 1;
			num_points      += 1;
			next_prim_point += 1;

			continue;
		}

		//
		// Is this a TVertex line?
		//

		match = sscanf(line, "Texture Vertex: (%f,%f)", &tu, &tv);

		if (match == 2)
		{
			ASSERT(WITHIN(num_tv, 0, MAX_SEX_UVS));

			sex_uv[num_tv].u = tu;
			sex_uv[num_tv].v = tv;

			num_tv += 1;

			continue;
		}

		//
		// Is this a face line?
		//

		match = sscanf(line, "Face: Material %d xyz (%d,%d,%d) uv (%d,%d,%d) edge (%d,%d,%d)", &m, &p1, &p2, &p3, &t[0], &t[1], &t[2], &edge_a, &edge_b, &edge_c);

		if (match == 7)
		{
			//
			// Found an old-version face.
			//

			edge_a = 1;
			edge_b = 1;
			edge_c = 1;

			match  = 10;

			//
			// Use the old quadify routine.
			//

			old_version = TRUE;
		}

		if (match == 10)
		{	
			ASSERT(WITHIN(m,  0, num_m  - 1));

			ASSERT(WITHIN(p1, 0, num_v  - 1));
			ASSERT(WITHIN(p2, 0, num_v  - 1));
			ASSERT(WITHIN(p3, 0, num_v  - 1));

			ASSERT(WITHIN(t[0], 0, num_tv - 1));
			ASSERT(WITHIN(t[1], 0, num_tv - 1));
			ASSERT(WITHIN(t[2], 0, num_tv - 1));

			ASSERT(WITHIN(next_prim_face3, 1, MAX_PRIM_FACES3 - 1));

			//
			// Has the texture been loaded? 
			//

			if (mat[m].page == 255)
			{
				//
				// Find the page number of this texture.
				//

				mat[m].page = PERSTEX_get_number(mat[m].name);


				//
				// Where abouts on which page is this texture kept?
				//

				if(mat[m].page>128)
				{

					//
					// bodge these into the second half of the insides
					//
//					if(mat[m].page>128+64)
					{
						mat[m].page-=128;
						mat[m].texture_page   =  (mat[m].page >> 6) + 18;
						mat[m].texture_base_u = ((mat[m].page >> 0) & 0x7) * 32;
						mat[m].texture_base_v = ((mat[m].page >> 3) & 0x7) * 32;

					}
/*
					else
					{

						mat[m].page-=128;
						mat[m].texture_page   =  (mat[m].page >> 6) + 8;
						mat[m].texture_base_u = ((mat[m].page >> 0) & 0x7) * 32;
						mat[m].texture_base_v = ((mat[m].page >> 3) & 0x7) * 32;
					}
*/

				}
				else
				{
					mat[m].texture_page   =  (mat[m].page >> 6) + 9;
					mat[m].texture_base_u = ((mat[m].page >> 0) & 0x7) * 32;
					mat[m].texture_base_v = ((mat[m].page >> 3) & 0x7) * 32;
				}
			}

			prim_faces3[next_prim_face3].Points[0] = base_v + p3;	// 3 2 1... backwards!
			prim_faces3[next_prim_face3].Points[1] = base_v + p2;
			prim_faces3[next_prim_face3].Points[2] = base_v + p1;

			prim_faces3[next_prim_face3].TexturePage = mat[m].texture_page;
			prim_faces3[next_prim_face3].DrawFlags   = POLY_GT;
			prim_faces3[next_prim_face3].FaceFlags   = 0;
			prim_faces3[next_prim_face3].Col2         = mat[m].col;

			av_u = (sex_uv[t[0]].u + sex_uv[t[1]].u + sex_uv[t[2]].u) * 0.333F;
			av_v = (sex_uv[t[0]].v + sex_uv[t[1]].v + sex_uv[t[2]].v) * 0.333F;

			base_iu = SLONG(av_u);
			base_iv = SLONG(av_v);

			base_fu = float(base_iu);
			base_fv = float(base_iv);

			for (i = 0; i < 3; i++)
			{
				tu = sex_uv[t[2 - i]].u - base_fu;
				tv = sex_uv[t[2 - i]].v - base_fv;

				tv = 1.0F - tv;	// TGA are upside down.

				iu = SLONG(tu * 31.0F);
				iv = SLONG(tv * 31.0F);

				SATURATE(iu, 0, 31);
				SATURATE(iv, 0, 31);

				iu += mat[m].texture_base_u;
				iv += mat[m].texture_base_v;

				prim_faces3[next_prim_face3].UV[i][0] = iu;
				prim_faces3[next_prim_face3].UV[i][1] = iv;
			}

			//prim_faces3[next_prim_face3].UV[0][0] = SLONG(sex_uv[t1].u * 255.99F);
			//prim_faces3[next_prim_face3].UV[0][1] = SLONG(sex_uv[t1].v * 255.99F);
			//prim_faces3[next_prim_face3].UV[1][0] = SLONG(sex_uv[t2].u * 255.99F);
			//prim_faces3[next_prim_face3].UV[1][1] = SLONG(sex_uv[t2].v * 255.99F);
			//prim_faces3[next_prim_face3].UV[2][0] = SLONG(sex_uv[t3].u * 255.99F);
			//prim_faces3[next_prim_face3].UV[2][1] = SLONG(sex_uv[t3].v * 255.99F);

			//
			// The flags are used by make_object_quad()
			//

			if(darci)
			{
					if (edge_a) {prim_faces3[next_prim_face3].FaceFlags |= FACE_FLAG_EDGE_VISIBLE_C;}
					if (edge_b) {prim_faces3[next_prim_face3].FaceFlags |= FACE_FLAG_EDGE_VISIBLE_A;}
					if (edge_c) {prim_faces3[next_prim_face3].FaceFlags |= FACE_FLAG_EDGE_VISIBLE_B;}
			}
			else
			{
					if (edge_a) {prim_faces3[next_prim_face3].FaceFlags |= FACE_FLAG_EDGE_VISIBLE_B;}
					if (edge_b) {prim_faces3[next_prim_face3].FaceFlags |= FACE_FLAG_EDGE_VISIBLE_A;}
					if (edge_c) {prim_faces3[next_prim_face3].FaceFlags |= FACE_FLAG_EDGE_VISIBLE_C;}
			}

			num_f           += 1;
			num_faces       += 1;
			next_prim_face3 += 1;

			continue;
		}
	}

	fclose(handle);
	DebugText(" object_count %d extras %d \n",object_count,extra_object_count);

	next_prim_object+=object_count+extra_object_count;

	if (po != NULL)
	{
		ASSERT(WITHIN(po, &prim_objects[1], &prim_objects[MAX_PRIM_OBJECTS - 1]));

		//
		// Finish off the last prim-object.
		//
		
		po->EndPoint = next_prim_point;
		po->EndFace3 = next_prim_face3;
		po->EndFace4 = 0; //next_prim_face4;

		ASSERT(po->EndPoint == po->StartPoint + num_points);
		ASSERT(po->EndFace3 == po->StartFace3 + num_faces );

		//
		// DONT QUADIFY THE LAST PRIM.
		//
	}

	//
	// Finish off the multi prim.
	//

	pmo->EndObject = next_prim_object;

	//
	// Some function that uses read_multi_asc(), expects some global SLONGS
	// x_centre, y_centre, z_centre to be set to the centre of the last
	// object called either, "CROTCH", "Lfoot" or "l-shoe"... wierd!
	//

	x_centre = 0;
	y_centre = 0;
	z_centre = 0;

	DebugText(" so %d eo %d \n",pmo->StartObject,pmo->EndObject);


	for (i = pmo->StartObject ; i < pmo->EndObject; i++)
	{
		ASSERT(WITHIN(i, 1, next_prim_object - 1));

		po = &prim_objects[i];
		DebugText(" object %d name %s sp %d ep %d\n",i,prim_names[i],po->StartPoint,po->EndPoint);

		//
		// Has this object got a name we want?
		// (names have numbers on end so just check the start characters)

//			if(memcmp(po->ObjectName, "lfoot" ,5) == 0 )
			if(memcmp(prim_names[i], "lfoot" ,5) == 0 )
			{
				re_center=1;
				its_human=1;
				for (SLONG j = po->StartPoint; j < po->EndPoint; j++)
				{
					ASSERT(WITHIN(j, 1, next_prim_point - 1));

					y_centre += prim_points[j].Y;
				}
				y_centre /= po->EndPoint - po->StartPoint;
				y_centre -=FOOT_HEIGHT;
			}

			if(memcmp(prim_names[i], "pelvis" ,5) == 0 )
			{
				for (SLONG j = po->StartPoint; j < po->EndPoint; j++)
				{
					ASSERT(WITHIN(j, 1, next_prim_point - 1));

					x_centre += prim_points[j].X;
					z_centre += prim_points[j].Z;
//					y_centre += prim_points[j].Y;
				}
				x_centre /= po->EndPoint - po->StartPoint;
				z_centre /= po->EndPoint - po->StartPoint;
//				y_centre /= po->EndPoint - po->StartPoint;

				re_center=1;
				its_human=1;
			}
/*
		if (memcmp(po->ObjectName, "crotch",6) == 0 ||
//			memcmp(po->ObjectName, "Object17" ,8) == 0 ||
			memcmp(po->ObjectName, "lfoot" ,5) == 0 ||
			memcmp(po->ObjectName, "l-shoe",6) == 0)
		{
			//
			// This is the object.
			//

			if (po->StartPoint != po->EndPoint)
			{
				for (SLONG j = po->StartPoint; j < po->EndPoint; j++)
				{
					ASSERT(WITHIN(j, 1, next_prim_point - 1));

					x_centre += prim_points[j].X;
					y_centre += prim_points[j].Y;
					z_centre += prim_points[j].Z;
				}

				x_centre /= po->EndPoint - po->StartPoint;
				y_centre /= po->EndPoint - po->StartPoint;
				z_centre /= po->EndPoint - po->StartPoint;
				re_center=1;
			}
			if (memcmp(po->ObjectName, "crotch",6) == 0 ||
				memcmp(po->ObjectName, "lfoot" ,5) == 0 ||
				memcmp(po->ObjectName, "l-shoe",6) == 0)
				its_human=1;

			break;
		}
*/
	}

	if ((!re_center)&&(!its_doggy))
	{
		x_centre = 15;//-30;
		y_centre = 30;
		z_centre = 0;//-51; 
	}


	// make it into quads and light it

	for(c0=prim_multi_objects[next_prim_multi_object-1].StartObject;c0<prim_multi_objects[next_prim_multi_object-1].EndObject;c0++)
	{
		//
		// Create face4s out of the face3s.
		//

//mark I've commented this back in, in this wrong?		

		if (old_version)
		{
			make_object_quad_old(c0);
		}
		else
		{
			make_object_quad(c0);
		}

		

		//apply_ambient_light_to_object(c0,-12,-128,-40,256);
		//smooth_a_prim(c0);
	}

	if(its_human)
		object_count=15;

	return (object_count);
}


#define	MAX_3DS_LEN	1000

extern void	read_object_name(FILE *file_handle,CBYTE *dest_string);

SBYTE read_asc(CBYTE *fname,SLONG scale,ULONG offset)
{
	//
	// Are we loading a '.SEX' file or a '.ASC' file?
	//

	SLONG fname_len = strlen(fname);

	if (fname_len >= 5)
	{
		if (toupper(fname[fname_len - 3]) == 'S' &&
			toupper(fname[fname_len - 2]) == 'E' &&
			toupper(fname[fname_len - 1]) == 'X')
		{
			//
			// This is a 'SEX' file, use the SEX file loader.
			//

			return read_sex(fname, scale, offset);
		}
	}

	float	f_x;
	float	f_y;
	float	f_z;

	FILE	*handle;

	int		cno_vertices = 0,dummy=0;
	int		no_vertices = 0;
	int		no_tvertices = 0,no_materials = 0, no_normals = 0;  //3ds max stuff
	int		tds_max=0;
	int		cno_faces = 0;
	int		no_faces = 0;
	int		fpointa, fpointb, fpointc;
	int		ab, bc, ca;
	int		retval;
	char	ts[MAX_3DS_LEN];
	char	test_string[200];
	UBYTE	found_a_face = FALSE;
//	char	info_text[80];
//	UWORD	first_point;
	SLONG	current_point,start_point=99999,end_point=0;
	SLONG	average_x=0,average_y=0,average_z=0;
	SLONG	num_points=0,real_start_point;
	SLONG	min_y=10000;
	UBYTE	found_object=0;
	struct	Material	materials[20];
	SLONG	mat_count;
	SLONG	c2;

	SLONG minx = +INFINITY;
	SLONG minz = +INFINITY;
	SLONG maxx = -INFINITY;
	SLONG maxz = -INFINITY;

	handle = fopen(fname,"r");
	if(handle)
	{
		next_prim_object++;
		prim_objects[next_prim_object-1].EndFace4=0;
		prim_objects[next_prim_object-1].StartFace4=0;
		do
		{
			retval = fscanf(handle,"%s",ts);

			if(!strcmp(ts,"Named"))
			{
				fscanf(handle,"%s",ts);		// The 'object:' bit.
//				read_object_name(handle,prim_objects[next_prim_object-1].ObjectName);
				read_object_name(handle,prim_names[next_prim_object-1]);
				for(c2=0;c2<strlen(prim_names[next_prim_object-1]);c2++)
				{
					prim_names[next_prim_object-1][c2]=tolower(prim_names[next_prim_object-1][c2]);
				}

			}
			else if(!strcmp(ts,"Object")) //3ds max is "object:" rather than "named object:"
			{
				tds_max=1;
				fscanf(handle,"%s",ts);		// The '0:' bit.
//				read_object_name(handle,prim_objects[next_prim_object-1].ObjectName);
				fscanf(handle,"%s",ts);		// read object name currently fucked for max due to no "
				mat_count=0;
			}
			else if(!strcmp(ts,"Material"))
			{
				CBYTE	str_r[30],str_g[30],str_b[30];
				fscanf(handle,"%s",ts);
				if(strcmp(ts,"list:"))
				{
					float	r,g,b;
					//if its not a list then its a material
					while(strcmp(ts,"Diffuse:"))
					{
						fscanf(handle,"%s",ts); //skip number
					}
					fscanf(handle,"%*s %s %*s %s %*s %s",&str_r, &str_g, &str_b); //%s*
					comma_to_dot(str_r);
					comma_to_dot(str_g);
					comma_to_dot(str_b);
					r=atof(str_r)*255;
					g=atof(str_g)*255;
					b=atof(str_b)*255;

					materials[mat_count].R=(UBYTE)r;
					materials[mat_count].G=(UBYTE)g;
					materials[mat_count].B=(UBYTE)b;
					{
						materials[mat_count].Index=find_colour((UBYTE*)ENGINE_palette,(UBYTE)r,(UBYTE)g,(UBYTE)b);
					}


					mat_count++;
				}

			}
			else if(!strcmp(ts,"Tri-mesh,"))  //3ds style name
			{
				fscanf(handle,"%*s %d %*s %d", &no_vertices, &no_faces);
				cno_vertices = no_vertices;
				cno_faces = no_faces;
				num_points+=no_vertices;
				if(found_object)
				{
					prim_objects[next_prim_object-1].EndFace3+=no_faces;
					prim_objects[next_prim_object-1].EndPoint+=no_vertices;
				}
				else
				{
					prim_objects[next_prim_object-1].EndPoint=next_prim_point+no_vertices;
					prim_objects[next_prim_object-1].StartPoint=next_prim_point;

					prim_objects[next_prim_object-1].EndFace3=next_prim_face3+no_faces;
					prim_objects[next_prim_object-1].StartFace3=next_prim_face3;
					real_start_point=next_prim_point;
				}
				start_point=next_prim_point;
				found_object=1;
			}
			else if(!strcmp(ts,"Vertices:")) //3ds max  3dx export
			{
				tds_max=1;
				fscanf(handle,"%d %*s %d %*s %d %*s %d %*s %d", &no_vertices,&no_tvertices, &no_faces,&no_materials,&no_normals);
				cno_vertices = no_vertices;
				cno_faces = no_faces;
				num_points+=no_vertices;
				if(found_object)
				{
					prim_objects[next_prim_object-1].EndFace3+=no_faces;
					prim_objects[next_prim_object-1].EndPoint+=no_vertices;
				}
				else
				{
					prim_objects[next_prim_object-1].EndPoint=next_prim_point+no_vertices;
					prim_objects[next_prim_object-1].StartPoint=next_prim_point;

					prim_objects[next_prim_object-1].EndFace3=next_prim_face3+no_faces;
					prim_objects[next_prim_object-1].StartFace3=next_prim_face3;
					real_start_point=next_prim_point;
				}
				start_point=next_prim_point;
				found_object=1;
			}
			else if(!strcmp(ts,"Vertex"))
			{
				fscanf(handle,"%s",ts);
				if(strcmp(ts,"list:"))
				{
//reads in garbage values					fscanf(handle,"%*s %*d %*s %*2s %f %*s%f %*s%f",&f_x, &f_y, &f_z); //%s*
					if(tds_max)
					{
						CBYTE	str_x[20],str_y[20],str_z[20];
						f_x=0;
						f_y=0;
						f_z=0;

						fscanf(handle,"%3s%s Y:%s Z:%s",test_string,&str_x, &str_y, &str_z); //%s*
						comma_to_dot(str_x);
						comma_to_dot(str_y);
						comma_to_dot(str_z);
						f_x=atof(str_x);
						f_y=atof(str_y);
						f_z=atof(str_z);


					}
					else
					{
						fscanf(handle,"%2s%f Y:%f Z:%f",test_string,&f_x, &f_y, &f_z); //%s*
					}
//poo					printf(" start string =>%s\n",test_string);

//					prim_points[next_prim_point].Z=(SLONG)((-f_x*(float)scale)/100.0); //+(engine.Z>>8)*offset;
//					prim_points[next_prim_point].X=(SLONG)((f_y*(float)scale)/100.0) ; //+(engine.X>>8)*offset;
//					prim_points[next_prim_point].Y=(SLONG)((-f_z*(float)scale)/100.0); //+(engine.Y>>8)*offset;

					prim_points[next_prim_point].X	=	(SLONG)((f_x*GAME_SCALE)/GAME_SCALE_DIV);
					prim_points[next_prim_point].Y	=	(SLONG)((f_z*GAME_SCALE)/GAME_SCALE_DIV); //- md
					prim_points[next_prim_point].Z	=	(SLONG)((f_y*GAME_SCALE)/GAME_SCALE_DIV);

/*
					if(offset)
					{
						if(prim_points[next_prim_point].Z<min_y)
							min_y=prim_points[next_prim_point].Z;
					}
					else
*/
					{
						if(prim_points[next_prim_point].Y<min_y)
							min_y=prim_points[next_prim_point].Y;
					}
					
					average_x+=prim_points[next_prim_point].X;
					average_y+=prim_points[next_prim_point].Y;
					average_z+=prim_points[next_prim_point].Z;

					if (prim_points[next_prim_point].X < minx) {minx = prim_points[next_prim_point].X;}
					if (prim_points[next_prim_point].Z < minz) {minz = prim_points[next_prim_point].Z;}

					if (prim_points[next_prim_point].X > maxx) {maxx = prim_points[next_prim_point].X;}
					if (prim_points[next_prim_point].Z > maxz) {maxz = prim_points[next_prim_point].Z;}


					cno_vertices--;
					next_prim_point++;
				}
			}
			else if(!strcmp(ts,"Face"))
			{
				fscanf(handle,"%s",ts);
				if(strcmp(ts,"list:"))
				{
					if(found_a_face)
					//	init_face(&iface);
					{
					}
					found_a_face = TRUE;
					fscanf(handle,"%*2s %d %*2s %d %*2s %d"
						"%*3s %d, %*3s, %d %*3s %d",
						&fpointa,&fpointb,&fpointc,
						&ab, &bc, &ca);
//					iface.Flags = (ab<<VF_AB)+(bc<<VF_BC)+(ca<<VF_CA);
					if(tds_max)
					{
						prim_faces3[next_prim_face3].Points[0]=fpointc+start_point;
						prim_faces3[next_prim_face3].Points[1]=fpointb+start_point;
						prim_faces3[next_prim_face3].Points[2]=fpointa+start_point;
					}
					else
					{
						prim_faces3[next_prim_face3].Points[0]=fpointa+start_point;
						prim_faces3[next_prim_face3].Points[1]=fpointb+start_point;
						prim_faces3[next_prim_face3].Points[2]=fpointc+start_point;

						prim_faces3[next_prim_face3].UV[0][0]	=	0;
						prim_faces3[next_prim_face3].UV[0][1]	=	0;
						prim_faces3[next_prim_face3].UV[1][0]	=	31;
						prim_faces3[next_prim_face3].UV[1][1]	=	0;
						prim_faces3[next_prim_face3].UV[2][0]	=	0;
						prim_faces3[next_prim_face3].UV[2][1]	=	31;
						prim_faces3[next_prim_face3].TexturePage=0;
					}

					if(mat_count==1)
					{
						prim_faces3[next_prim_face3].Col2=materials[0].Index;
						prim_faces3[next_prim_face3].DrawFlags=POLY_G;

					}
					
					next_prim_face3++;
					cno_faces--;
				}
			}
			else if(!strcmp(ts,"Material:"))
			{
				SLONG	mat;
				fscanf(handle,"%d",&mat); 
				prim_faces3[next_prim_face3-1].Col2=materials[mat].Index;
				prim_faces3[next_prim_face3-1].DrawFlags=POLY_G;

			}
			else if(!strcmp(ts,"Smoothing:"))
			{
			}
		} while(retval>=0);

		fclose(handle);

/*
	average_x=0;		
	average_y=0;		
	average_z=0;		
	for(current_point=start_point;current_point<=end_point;current_point++)
	{
		average_x+=prim_points[current_point].X;
		average_y+=prim_points[current_point].Y;
		average_z+=prim_points[current_point].Z;
		
	}
	num_points=end_point-start_point;
	num_points++;
	if(num_points>0)
	{
		average_x=average_x/(num_points);
		average_y=average_y/(num_points);
		average_z=average_z/(num_points);
	}
*/
	average_x/=num_points;
	average_y/=num_points;
	average_z/=num_points;

	//
	// Dont use the average(x,z) use the centre of the
	// bounding box.
	//

	average_x = minx + maxx >> 1;
	average_z = minz + maxz >> 1;

	for(current_point=real_start_point;current_point<real_start_point+num_points;current_point++)
	{
//		prim_points[current_point].X-=average_x;
//		prim_points[current_point].Y-=average_y;
//		prim_points[current_point].Z-=average_z;
		if(offset)
		{
			prim_points[current_point].Y-=min_y; //average_y;
		}
		else
		{
			prim_points[current_point].Y-=average_y; //average_y;
		}

		prim_points[current_point].Z-=average_z;
		prim_points[current_point].X-=average_x;
	}

	}
	else
	{
		return(0);
	}
	make_object_quad(next_prim_object-1);
//	apply_ambient_light_to_object(next_prim_object-1,-128,128,-40,256);
//	load_textures_for_prim("",next_prim_object-1);
	return(1);		
}


SLONG	calc_object_index(CBYTE *name,SLONG *extra)
{
	SLONG	c0;
	for(c0=0;c0<strlen(name);c0++)
	{
		name[c0]=tolower(name[c0]);
	}

	for(c0=0;c0<20;c0++)
	{
		if(body_part_names[c0]==0)
			break;
		if(!memcmp(body_part_names[c0],name,strlen(body_part_names[c0])))
		{
			*extra=atoi(&name[strlen(body_part_names[c0])]);
			//
			// found a string match at pos c0
			//
			return(c0);
		}
	}
	return(-1);
}

SLONG read_multi_asc(CBYTE *asc_name,UBYTE flag,float shrink)
{

	SLONG fname_len = strlen(asc_name);

	if (fname_len >= 5)
	{
		SLONG	ret;
		asc_name[fname_len - 3]='S';
		asc_name[fname_len - 2]='E';
		asc_name[fname_len - 1]='X';

		//
		// This is a 'SEX' file, use the SEX file loader.
		//

		ret=read_multi_sex(asc_name,shrink);
		if(ret)
		{
			return(ret);
		}
		else
		{
			asc_name[fname_len - 3]='A';
			asc_name[fname_len - 2]='S';
			asc_name[fname_len - 1]='C';
		}
	}

	float	f_x;
	float	f_y;
	float	f_z;

	FILE	*handle;

	int		cno_vertices = 0,dummy=0;
	int		no_tvertices = 0,no_materials = 0, no_normals = 0;  //3ds max stuff
	int		tds_max=0;
	int		no_vertices = 0;
	int		cno_faces = 0;
	int		no_faces = 0;
	int		fpointa, fpointb, fpointc;
	int		ab, bc, ca;
	int		retval;
	char	ts[MAX_3DS_LEN];
	char	test_string[200];
	UBYTE	found_a_face = FALSE;
//	char	info_text[80];
//	UWORD	first_point;
	SLONG	current_point,start_point=99999,end_point=0;
	SLONG	average_x=0,average_y=0,average_z=0;
	SLONG	num_points=0,real_start_point;
	SLONG	min_y=0,c0;
	UBYTE	found_object=0;
	struct	Material	materials[20];
	SLONG	mat_count,
			material_index	=	0;
	SLONG	object_index,start_object,object_count=0,extra_object_count=0;
	CBYTE	object_name[100];
	SLONG	extra;
	SLONG	its_human=0;
	SLONG	c2;



	LogText(" read multi asc %s next_prim_multi_object %d next_prim_object %d\n",asc_name,next_prim_multi_object,next_prim_object);

	start_object=next_prim_object;

	got_torso	=	0;
	handle = fopen(asc_name,"r");
	if(handle)
	{
		do
		{
			retval = fscanf(handle,"%s",ts);
			
			if(!strcmp(ts,"Named"))
			{
				if(got_torso)
				{
					x_centre	=	average_x/no_vertices;
					y_centre	=	average_y/no_vertices;
					z_centre	=	average_z/no_vertices;
					got_torso	=	0;
				}
				fscanf(handle,"%s",ts);		// The 'object:' bit.
				read_object_name(handle,object_name); //ObjectName);
				for(c2=0;c2<strlen(object_name);c2++)
				{
					object_name[c2]=tolower(object_name[c2]);
				}

//				read_object_name(handle,prim_objects[next_prim_object].ObjectName);

				if(!memcmp("crotch",prim_names[next_prim_object],6))
				{
					got_torso	=	1;
					LogText(" got crotch \n");
				}
				if(!memcmp("pelvis",prim_names[next_prim_object],6))
				{
					its_human	=	1;
				}
			}
			else if(!strcmp(ts,"object")) //3ds max is "object:" rather than "named object:"
			{

				if(got_torso)
				{
					x_centre	=	average_x/no_vertices;
					y_centre	=	average_y/no_vertices;
					z_centre	=	average_z/no_vertices;
					got_torso	=	0;
				}

				tds_max=1;
				fscanf(handle,"%s",ts);		// The '0:' bit.
//				read_object_name(handle,prim_objects[next_prim_object-1].ObjectName);

				fscanf(handle,"%s",object_name); //prim_objects[next_prim_object].ObjectName);		// no quotes to avoid in max

				object_index=calc_object_index(object_name,&extra);
				if(object_index<0)
					return(0);      //body part name unknown

				if(extra)
				{
//					test_chunk.BodyBits[object_index][extra]=15+extra_object_count;
					object_index=15+extra_object_count;
					extra_object_count++;

				}
				else
					object_count++;




				object_index+=next_prim_object;

//				strcpy(prim_objects[object_index].ObjectName,object_name);
				strcpy(prim_names[object_index],object_name);

				if(!memcmp("lfoot",prim_names[object_index],5))
				{
					got_torso	=	1;
					LogText(" found lfoot\n");
				}


				if(!memcmp("l-shoe",prim_names[object_index],6))
				{
					got_torso	=	1;
					LogText(" found lfoot\n");
				}

				LogText(" read multi asc part %s prim %d\n",object_name,object_index);
				mat_count=0;
			}
			else if(!strcmp(ts,"Material"))
			{
				CBYTE	str_r[30],str_g[30],str_b[30];
				float	r,g,b;

				fscanf(handle,"%s",ts);
				if(!strcmp(ts,"list:"))
				{//start of material list for coming faces
					material_index	=	material_count;
				}
				else
				{
					//if its not a list then its a material

					while(strcmp(ts,"Diffuse:"))
					{
						fscanf(handle,"%s",ts); //skip number
					}
					fscanf(handle,"%*s %s %*s %s %*s %s",&str_r, &str_g, &str_b); //%s*
					comma_to_dot(str_r);
					comma_to_dot(str_g);
					comma_to_dot(str_b);
					r=atof(str_r)*255;
					g=atof(str_g)*255;
					b=atof(str_b)*255;

					materials[mat_count].R=(UBYTE)r;
					materials[mat_count].G=(UBYTE)g;
					materials[mat_count].B=(UBYTE)b;
					{
// Guy - fudge round Mikes fudge, this will prolly stop editor bits from working.
						if(pals[0])
							materials[mat_count].Index=find_colour((UBYTE*)ENGINE_palette,(UBYTE)r,(UBYTE)g,(UBYTE)b);
							LogText("find material = %d \n",materials[mat_count].Index);
//						else
//							materials[mat_count].Index	=	0;
					}

					mat_count++;

					/*
					// Guy's bit.
					dx_materials[material_count].R	=	atof(str_r);
					dx_materials[material_count].G	=	atof(str_g);
					dx_materials[material_count].B	=	atof(str_b);

					material_count++;
					*/
				}

			}
			else if(!strcmp(ts,"Tri-mesh,"))
			{
/*
				if(found_object)
				for(current_point=real_start_point;current_point<real_start_point+no_vertices;current_point++)
				{
					prim_points[current_point].X-=average_x/no_vertices;
					prim_points[current_point].Y-=average_y/no_vertices;
			//		prim_points[current_point].Z-=min_y; //average_y;
					prim_points[current_point].Z-=average_z/no_vertices;
				}
*/

				fscanf(handle,"%*s %d %*s %d", &no_vertices, &no_faces);
				cno_vertices = no_vertices;
				cno_faces = no_faces;
				num_points+=no_vertices;
//				LogText(" read obj  next_prim_multi_object %d object_index %d\n",next_prim_multi_object,object_index);
				if(!found_object)
				{
					found_object=1;
					prim_multi_objects[next_prim_multi_object].Frame=0;
					prim_multi_objects[next_prim_multi_object].Tween=0;
					prim_multi_objects[next_prim_multi_object].StartObject=start_object;
					prim_multi_objects[next_prim_multi_object++].EndObject=start_object;

				}

				{
					prim_objects[object_index].StartPoint	=	next_prim_point;
					prim_objects[object_index].EndPoint	=	next_prim_point+no_vertices;

					prim_objects[object_index].StartFace3	=	next_prim_face3;
					prim_objects[object_index].EndFace3	=	next_prim_face3+no_faces;

					prim_objects[object_index].StartFace4	=	0;
					prim_objects[object_index].EndFace4	=	0;

					real_start_point=next_prim_point;
					if(object_index>prim_multi_objects[next_prim_multi_object-1].EndObject)
						prim_multi_objects[next_prim_multi_object-1].EndObject=object_index;
				}
				average_x=0;
				average_y=0;
				average_z=0;
				start_point=next_prim_point;
			}
			else if(!strcmp(ts,"Vertices:")) //3ds max  3dx export
			{
				tds_max=1;
				fscanf(handle,"%d %*s %d %*s %d %*s %d %*s %d", &no_vertices,&no_tvertices, &no_faces,&no_materials,&no_normals);
				cno_vertices = no_vertices;
				cno_faces = no_faces;
				num_points+=no_vertices;
				if(!found_object)
				{
					found_object=1;
					prim_multi_objects[next_prim_multi_object].Frame=0;
					prim_multi_objects[next_prim_multi_object].Tween=0;
					prim_multi_objects[next_prim_multi_object].StartObject=start_object;
					prim_multi_objects[next_prim_multi_object++].EndObject=start_object;

				}

				{
					prim_objects[object_index].StartPoint	=	next_prim_point;
					prim_objects[object_index].EndPoint	=	next_prim_point+no_vertices;

					prim_objects[object_index].StartFace3	=	next_prim_face3;
					prim_objects[object_index].EndFace3	=	next_prim_face3+no_faces;

					prim_objects[object_index].StartFace4	=	0;
					prim_objects[object_index].EndFace4	=	0;

					real_start_point=next_prim_point;
					if(object_index>prim_multi_objects[next_prim_multi_object-1].EndObject)
						prim_multi_objects[next_prim_multi_object-1].EndObject=object_index;
				}
				average_x=0;
				average_y=0;
				average_z=0;
				start_point=next_prim_point;
			}
			else if(!strcmp(ts,"Vertex"))
			{
				fscanf(handle,"%s",ts);
				if(strcmp(ts,"list:"))
				{
//reads in garbage values					fscanf(handle,"%*s %*d %*s %*2s %f %*s%f %*s%f",&f_x, &f_y, &f_z); //%s*
					if(tds_max)
					{
						CBYTE	str_x[20],str_y[20],str_z[20];

						fscanf(handle,"%3s%s Y:%s Z:%s",test_string,&str_x, &str_y, &str_z); //%s*
						comma_to_dot(str_x);
						comma_to_dot(str_y);
						comma_to_dot(str_z);
						f_x=atof(str_x);
						f_y=atof(str_y);
						f_z=atof(str_z);


					}
					else
					{
						fscanf(handle,"%2s%f Y:%f Z:%f",test_string,&f_x, &f_y, &f_z); //%s*
					}
//poo					printf(" start string =>%s\n",test_string);

/*
// Guy - Added scaling to multi objects.
					prim_points[next_prim_point].X=(SLONG)((f_x)); //+(engine.X>>8);
					prim_points[next_prim_point].Y=-(SLONG)((f_z)); //+(engine.Y>>8);
					prim_points[next_prim_point].Z=(SLONG)((f_y)); //+(engine.Z>>8);
*/
//					LogText(" multi asc in %f,%f,%f\n",f_x,f_y,f_z);

					prim_points[next_prim_point].X	=	(SLONG)((f_x*GAME_SCALE)/(GAME_SCALE_DIV*shrink));
					prim_points[next_prim_point].Y	=	(SLONG)((f_z*GAME_SCALE)/(GAME_SCALE_DIV*shrink)); //- md
					prim_points[next_prim_point].Z	=	(SLONG)((f_y*GAME_SCALE)/(GAME_SCALE_DIV*shrink));

//					LogText(" multi asc store %d,%d,%d \n",prim_points[next_prim_point].X,prim_points[next_prim_point].Y,prim_points[next_prim_point].Z);



					if(prim_points[next_prim_point].Z<min_y)
						min_y=prim_points[next_prim_point].Z;
					
					average_x	+=	prim_points[next_prim_point].X;
					average_y	+=	prim_points[next_prim_point].Y;
					average_z	+=	prim_points[next_prim_point].Z;
					cno_vertices--;
					next_prim_point++;
				}
			}
			else if(!strcmp(ts,"Face"))
			{
				fscanf(handle,"%s",ts);
				if(strcmp(ts,"list:"))
				{
					if(found_a_face)
					//	init_face(&iface);
					{
					}
					found_a_face = TRUE;
					fscanf(handle,"%*2s %d %*2s %d %*2s %d"
						"%*3s %d, %*3s, %d %*3s %d",
						&fpointa,&fpointb,&fpointc,
						&ab, &bc, &ca);
//					iface.Flags = (ab<<VF_AB)+(bc<<VF_BC)+(ca<<VF_CA);
					if(tds_max)
					{
						prim_faces3[next_prim_face3].Points[0]=fpointc+start_point;
						prim_faces3[next_prim_face3].Points[1]=fpointb+start_point;
						prim_faces3[next_prim_face3].Points[2]=fpointa+start_point;
					}
					else
					{
						prim_faces3[next_prim_face3].Points[0]=fpointa+start_point;
						prim_faces3[next_prim_face3].Points[1]=fpointb+start_point;
						prim_faces3[next_prim_face3].Points[2]=fpointc+start_point;
					}

					if(mat_count==1)
					{
						prim_faces3[next_prim_face3].Col2=materials[0].Index;
						prim_faces3[next_prim_face3].DrawFlags=POLY_G;
						prim_faces3[next_prim_face3].TexturePage=0;

					}
					
					// Guy
//					prim_faces3[next_prim_face3].Col2	=	material_index;
					
					next_prim_face3++;
					cno_faces--;
				}
			}
			else if(!strcmp(ts,"Material:"))
			{
				SLONG	mat_no=0;
				fscanf(handle,"%d",&mat_no); //get material number
//				LogText(" mat_no %d \n",mat_no);

				prim_faces3[next_prim_face3-1].Col2=materials[mat_no].Index;
				prim_faces3[next_prim_face3-1].DrawFlags=POLY_G;
				prim_faces3[next_prim_face3-1].TexturePage=0;
// Guy
//				prim_faces3[next_prim_face3-1].Col2	=	material_index+mat_no;
//				prim_faces3[next_prim_face3-1].DrawFlags=POLY_G;
			}
			else if(!strcmp(ts,"Smoothing:"))
			{
			}
		} while(retval>=0);

		fclose(handle);

	/*
		average_x=0;		
		average_y=0;		
		average_z=0;		
		for(current_point=start_point;current_point<=end_point;current_point++)
		{
			average_x+=prim_points[current_point].X;
			average_y+=prim_points[current_point].Y;
			average_z+=prim_points[current_point].Z;
			
		}
		num_points=end_point-start_point;
		num_points++;
		if(num_points>0)
		{
			average_x=average_x/(num_points);
			average_y=average_y/(num_points);
			average_z=average_z/(num_points);
		}
	*/

	}
	else
	{
		return(0);
	}

	next_prim_object+=object_count+extra_object_count;

	for(c0=prim_multi_objects[next_prim_multi_object-1].StartObject;c0<prim_multi_objects[next_prim_multi_object-1].EndObject;c0++)
	{
		make_object_quad(c0);
		apply_ambient_light_to_object(c0,-12,-128,-40,256);
		smooth_a_prim(c0);
	}
	return(object_count);		
}


void	load_textures_for_prim(CBYTE *str,UWORD prim)
{
	CBYTE			file_name[64];
	UWORD			text;
	SLONG			c0,sf,ef,point,mf;
	MFFileHandle	handle;
	PrimPoint		pf[3];


	sprintf(file_name,"Editor/EdSave/prim%d.sav",1);
	handle	=	FileOpen(file_name);
	if(handle!=FILE_OPEN_ERROR)
	{
		sf=prim_objects[prim].StartFace4;
		ef=prim_objects[prim].EndFace4;

		for(c0=sf;c0<=ef;c0++)
		{
			for(point=0;point<3;point++)
			{
				FileRead(handle,(UBYTE*)&pf[point],sizeof(struct	PrimPoint));
			}
			FileRead(handle,(UBYTE*)&prim_faces4[c0].TexturePage,sizeof(prim_faces4[c0].TexturePage));
			FileRead(handle,(UBYTE*)&prim_faces4[c0].UV[0][0],sizeof(prim_faces4[c0].UV));
/*
			mf=find_matching_face(&pf[0],&pf[1],&pf[2],prim);
			if(mf>0)
			{
//				prim_faces4[mf].Texture=text;
				prim_faces4[mf].UV[0][0]	=	0;
				prim_faces4[mf].UV[0][1]	=	0;
				prim_faces4[mf].UV[1][0]	=	31;
				prim_faces4[mf].UV[1][1]	=	0;
				prim_faces4[mf].UV[2][0]	=	0;
				prim_faces4[mf].UV[2][1]	=	31;
				prim_faces4[mf].UV[3][0]	=	31;
				prim_faces4[mf].UV[3][1]	=	31;
			}
*/
		}
		FileClose(handle);
	}
}


void	save_textures_for_prim(CBYTE *str,UWORD prim)
{
	CBYTE			file_name[64];
	SLONG			c0,sf,ef,point;
	MFFileHandle	handle;


	sprintf(file_name,"Editor/EdSave/prim%d.sav",1);
	handle	=	FileCreate(file_name,1);
	if(handle!=FILE_OPEN_ERROR)
	{
		sf=prim_objects[prim].StartFace4;
		ef=prim_objects[prim].EndFace4;

		for(c0=sf;c0<=ef;c0++)
		{
			for(point=0;point<3;point++)
			{
				FileWrite(handle,(UBYTE*)&prim_points[prim_faces4[c0].Points[point]],sizeof(struct	PrimPoint));
			}
			FileWrite(handle,(UBYTE*)&prim_faces4[c0].TexturePage,sizeof(prim_faces4[c0].TexturePage));
			FileWrite(handle,(UBYTE*)&prim_faces4[c0].UV[0][0],sizeof(prim_faces4[c0].UV));
		}
		FileClose(handle);
	}
}


SLONG	save_all_prims(CBYTE	*name)
{
	SLONG			c0,point;
	MFFileHandle	handle;
	CBYTE			file_name[64];
	UWORD		dummy[100];

	//
	// In case it hasn't been done already.
	//

	calc_prim_info();

	/*

	//
	// DONT DO THIS! IT FUCKS UP THE CENTRE OF THE OBJECT IF WE
	// GOT IT FROM THE PIVOT IN THE .SEX FILE
	//

	//
	// Change the prims so that the origin (x,z) is the centre
	// of the bounding box.
	//

	SLONG i;
	SLONG j;
	SLONG centre_x;
	SLONG centre_z;

	PrimInfo   *pi;
	PrimObject *obj;
	PrimPoint  *pt;

	for (i = 1; i < next_prim_object; i++)
	{
		pi = get_prim_info(i);

		centre_x = pi->minx + pi->maxx >> 1;
		centre_z = pi->minz + pi->maxz >> 1;

		//
		// Make this the centre of the prim.
		//

		obj = &prim_objects[i];

		for (j = obj->StartPoint; j < obj->EndPoint; j++)
		{
			pt = &prim_points[j];

			pt->X -= centre_x;
			pt->Z -= centre_z;
		}
	}

	*/

	sprintf(file_name,"data/%s",name);
	handle	=	FileCreate(file_name,1);
	if(handle!=FILE_OPEN_ERROR)
	{
		FileWrite(handle,(UBYTE*)&next_prim_point,sizeof(UWORD));
		FileWrite(handle,(UBYTE*)&next_prim_face4,sizeof(UWORD));
		FileWrite(handle,(UBYTE*)&next_prim_face3,sizeof(UWORD));
		FileWrite(handle,(UBYTE*)&next_prim_object,sizeof(UWORD));

		FileWrite(handle,(UBYTE*)dummy,sizeof(UWORD)*10);

		FileWrite(handle,(UBYTE*)prim_points,sizeof(struct PrimPoint)*next_prim_point);
		FileWrite(handle,(UBYTE*)prim_faces4,sizeof(struct PrimFace4)*next_prim_face4);
		FileWrite(handle,(UBYTE*)prim_faces3,sizeof(struct PrimFace3)*next_prim_face3);
		FileWrite(handle,(UBYTE*)prim_objects,sizeof(struct PrimObject)*next_prim_object);

		FileClose(handle);
		return(1);
	}
	return(0);
}

void	write_a_prim(SLONG prim,MFFileHandle	handle)
{
	SLONG	c0;
	SLONG	sf,ef,sp,ep;


	if(handle!=FILE_OPEN_ERROR)
	{
		sp=prim_objects[prim].StartPoint;
		ep=prim_objects[prim].EndPoint;

		FileWrite(handle,(UBYTE*)prim_names[prim],32); //sizeof(prim_objects[prim].ObjectName));
//		prim_objects[prim].Dummy[3]=PRIM_START_SAVE_TYPE+1; pointless

		FileWrite(handle,(UBYTE*)&sp,sizeof(sp));
		FileWrite(handle,(UBYTE*)&ep,sizeof(ep));
		for(c0=sp;c0<ep;c0++)
			FileWrite(handle,(UBYTE*)&prim_points[c0],sizeof(struct PrimPoint));


		sf=prim_objects[prim].StartFace3;
		ef=prim_objects[prim].EndFace3;
		FileWrite(handle,(UBYTE*)&sf,sizeof(sf));
		FileWrite(handle,(UBYTE*)&ef,sizeof(ef));
		for(c0=sf;c0<ef;c0++)
			FileWrite(handle,(UBYTE*)&prim_faces3[c0],sizeof(struct PrimFace3));

		sf=prim_objects[prim].StartFace4;
		ef=prim_objects[prim].EndFace4;
		FileWrite(handle,(UBYTE*)&sf,sizeof(sf));
		FileWrite(handle,(UBYTE*)&ef,sizeof(ef));
		for(c0=sf;c0<ef;c0++)
			FileWrite(handle,(UBYTE*)&prim_faces4[c0],sizeof(struct PrimFace4));
	}
}


SLONG	save_a_multi_prim(CBYTE	*name,SLONG multi)
{
	SLONG			c0,point;
	MFFileHandle	handle;
	CBYTE			file_name[64];
	SLONG			save_type=4;
	SLONG			so,eo;
	CBYTE			ext_name[80];

	change_extension(name,"moj",ext_name);
//	sprintf(ext_name,"data/%s",ext_name);
	handle	=	FileCreate(ext_name,1);
	if(handle!=FILE_OPEN_ERROR)
	{
		
		FileWrite(handle,(UBYTE*)&save_type,sizeof(save_type));

		so=prim_multi_objects[multi].StartObject;
		eo=prim_multi_objects[multi].EndObject;

		FileWrite(handle,(UBYTE*)&so,sizeof(so));
		FileWrite(handle,(UBYTE*)&eo,sizeof(eo));

		for(c0=so;c0<eo;c0++)
			write_a_prim(c0,handle);

		FileClose(handle);
		return(1);
	}
	return(0);
	
}



void	dump_face_info_for_prim(MFFileHandle handle,UWORD prim)
{
	SLONG			c0,sf,ef,point;
	SLONG			count;
	
	sf=prim_objects[prim].StartFace4;
	ef=prim_objects[prim].EndFace4;
	count=ef-sf;
	FileWrite(handle,(UBYTE*)&count,sizeof(count));

	for(c0=sf;c0<ef;c0++)
	{
		for(point=0;point<3;point++)
		{
			FileWrite(handle,(UBYTE*)&prim_points[prim_faces4[c0].Points[point]],sizeof(struct	PrimPoint));
		}
		FileWrite(handle,(UBYTE*)&prim_faces4[c0].TexturePage,sizeof(prim_faces4[c0].TexturePage));
		FileWrite(handle,(UBYTE*)&prim_faces4[c0].DrawFlags,sizeof(prim_faces4[c0].DrawFlags));
		FileWrite(handle,(UBYTE*)&prim_faces4[c0].Col2,sizeof(prim_faces4[c0].Col2));
		FileWrite(handle,(UBYTE*)&prim_faces4[c0].FaceFlags,sizeof(prim_faces4[c0].FaceFlags));
		FileWrite(handle,(UBYTE*)&prim_faces4[c0].UV[0][0],sizeof(prim_faces4[c0].UV));
	}

	sf=prim_objects[prim].StartFace3;
	ef=prim_objects[prim].EndFace3;
	count=ef-sf;
	FileWrite(handle,(UBYTE*)&count,sizeof(count));
	for(c0=sf;c0<ef;c0++)
	{
		for(point=0;point<3;point++)
		{
			FileWrite(handle,(UBYTE*)&prim_points[prim_faces3[c0].Points[point]],sizeof(struct	PrimPoint));
		}
		FileWrite(handle,(UBYTE*)&prim_faces3[c0].TexturePage,sizeof(prim_faces3[c0].TexturePage));
		FileWrite(handle,(UBYTE*)&prim_faces3[c0].DrawFlags,sizeof(prim_faces3[c0].DrawFlags));
		FileWrite(handle,(UBYTE*)&prim_faces3[c0].Col2,sizeof(prim_faces3[c0].Col2));
		FileWrite(handle,(UBYTE*)&prim_faces3[c0].FaceFlags,sizeof(prim_faces3[c0].FaceFlags));
		FileWrite(handle,(UBYTE*)&prim_faces3[c0].UV[0][0],sizeof(prim_faces3[c0].UV));
	}
}


UWORD	change_fname_extension(CBYTE *name,CBYTE *ext)
{
	SLONG	c0;
	for(c0=0;c0<strlen(name);c0++)
	{
		if(name[c0]=='.')
		{
			name[c0+1]=ext[0];
			name[c0+2]=ext[1];
			name[c0+3]=ext[2];
			return(1);
		}
	}
	return(0);
	
}

void	export_tex(CBYTE *fname)
{
	SWORD	index;
	struct	MapThing	*p_thing;
	MFFileHandle	handle;
	SLONG	save_type=5;

	change_fname_extension(fname,"tex");
	handle	=	FileCreate(fname,1);
	if(handle!=FILE_OPEN_ERROR)
	{
		SLONG	count=-1;
		FileWrite(handle,(UBYTE*)&save_type,sizeof(save_type));
		index=background_prim;
		while(index)
		{
			p_thing=TO_MTHING(index);
//			draw_a_prim_at(p_thing->IndexOther,p_thing->X,p_thing->Y,p_thing->Z);
			dump_face_info_for_prim(handle,p_thing->IndexOther);
			index=p_thing->IndexNext;
		}
		FileWrite(handle,(UBYTE*)&count,sizeof(count));
		FileWrite(handle,(UBYTE*)&map_things[0],sizeof(struct MapThing)*MAX_MAP_THINGS);
		FileWrite(handle,(UBYTE*)&edit_info.amb_dx,4*5);



	}
	FileClose(handle);

	
}

#define	CLOSE(x,y)	(abs((x)-(y))<5)

SLONG	find_and_apply_to_quad(struct PrimPoint *pp,struct PrimFace4 *face4,SLONG gx,SLONG gy,SLONG gz)
{
	SWORD	index;
	struct	MapThing	*p_thing;

	SLONG			c0,sf,ef,point;
	SLONG			count;
	UWORD	prim;
	
	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		prim=p_thing->IndexOther;

		sf=prim_objects[prim].StartFace4;
		ef=prim_objects[prim].EndFace4;

		count=ef-sf;

		for(c0=sf;c0<ef;c0++)
		{
			if(CLOSE(prim_points[prim_faces4[c0].Points[0]].X,pp[0].X-gx)&&
			   CLOSE(prim_points[prim_faces4[c0].Points[0]].Y,pp[0].Y-gy)&&
			   CLOSE(prim_points[prim_faces4[c0].Points[0]].Z,pp[0].Z-gz)&&
			   CLOSE(prim_points[prim_faces4[c0].Points[1]].X,pp[1].X-gx)&&
			   CLOSE(prim_points[prim_faces4[c0].Points[1]].Y,pp[1].Y-gy)&&
			   CLOSE(prim_points[prim_faces4[c0].Points[1]].Z,pp[1].Z-gz)&&
			   CLOSE(prim_points[prim_faces4[c0].Points[2]].X,pp[2].X-gx)&&
			   CLOSE(prim_points[prim_faces4[c0].Points[2]].Y,pp[2].Y-gy)&&
			   CLOSE(prim_points[prim_faces4[c0].Points[2]].Z,pp[2].Z-gz))
			   {
					prim_faces4[c0].FaceFlags=face4->FaceFlags;
					prim_faces4[c0].DrawFlags=face4->DrawFlags;
					prim_faces4[c0].TexturePage=face4->TexturePage;
					prim_faces4[c0].Col2=face4->Col2;
					memcpy((UBYTE*)prim_faces4[c0].UV,(UBYTE*)face4->UV,sizeof(prim_faces4[c0].UV));
					return(1);
			   }
		}
		index=p_thing->IndexNext;
	}
	return(0);
}

SLONG	find_and_apply_to_tri(struct PrimPoint *pp,struct PrimFace3 *face3,SLONG gx,SLONG gy,SLONG gz)
{
	SWORD	index;
	struct	MapThing	*p_thing;

	SLONG			c0,sf,ef,point;
	SLONG			count;
	UWORD	prim;
	
	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		prim=p_thing->IndexOther;

		sf=prim_objects[prim].StartFace3;
		ef=prim_objects[prim].EndFace3;

		count=ef-sf;

		for(c0=sf;c0<ef;c0++)
		{
			if(CLOSE(prim_points[prim_faces3[c0].Points[0]].X,pp[0].X-gx)&&
			   CLOSE(prim_points[prim_faces3[c0].Points[0]].Y,pp[0].Y-gy)&&
			   CLOSE(prim_points[prim_faces3[c0].Points[0]].Z,pp[0].Z-gz)&&
			   CLOSE(prim_points[prim_faces3[c0].Points[1]].X,pp[1].X-gx)&&
			   CLOSE(prim_points[prim_faces3[c0].Points[1]].Y,pp[1].Y-gy)&&
			   CLOSE(prim_points[prim_faces3[c0].Points[1]].Z,pp[1].Z-gz)&&
			   CLOSE(prim_points[prim_faces3[c0].Points[2]].X,pp[2].X-gx)&&
			   CLOSE(prim_points[prim_faces3[c0].Points[2]].Y,pp[2].Y-gy)&&
			   CLOSE(prim_points[prim_faces3[c0].Points[2]].Z,pp[2].Z-gz))
			   {
					prim_faces3[c0].FaceFlags=face3->FaceFlags;
					prim_faces3[c0].DrawFlags=face3->DrawFlags;
					prim_faces3[c0].TexturePage=face3->TexturePage;
					prim_faces3[c0].Col2=face3->Col2;
					memcpy((UBYTE*)prim_faces3[c0].UV,(UBYTE*)face3->UV,sizeof(prim_faces3[c0].UV));
					return(1);
			   }
		}
		index=p_thing->IndexNext;
	}
	return(0);
}


SLONG	is_this_unique(SLONG *dx,SLONG *dy,SLONG *dz,SLONG dx2,SLONG dy2,SLONG dz2)
{
	SWORD	index;
	struct	MapThing	*p_thing;

	SLONG			c0,sf,ef,point;
	SLONG			count,unique=0,at_face=0;
	SLONG	mx,my,mz,mx2,my2,mz2;
	UWORD	prim;
	SLONG rx,ry,rz;
	
	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		prim=p_thing->IndexOther;

		sf=prim_objects[prim].StartFace4;
		ef=prim_objects[prim].EndFace4;

		count=ef-sf;

		for(c0=sf;c0<ef;c0++)
		{
			mx=prim_points[prim_faces4[c0].Points[1]].X-prim_points[prim_faces4[c0].Points[0]].X;
			my=prim_points[prim_faces4[c0].Points[1]].Y-prim_points[prim_faces4[c0].Points[0]].Y;
			mz=prim_points[prim_faces4[c0].Points[1]].Z-prim_points[prim_faces4[c0].Points[0]].Z;

			mx2=prim_points[prim_faces4[c0].Points[2]].X-prim_points[prim_faces4[c0].Points[0]].X;
			my2=prim_points[prim_faces4[c0].Points[2]].Y-prim_points[prim_faces4[c0].Points[0]].Y;
			mz2=prim_points[prim_faces4[c0].Points[2]].Z-prim_points[prim_faces4[c0].Points[0]].Z;
			if(*dx==mx&&*dy==my&&*dz==mz&&dx2==mx2&&dy2==my2&&dz2==mz2)
			{
				rx=prim_points[prim_faces4[c0].Points[0]].X;
				ry=prim_points[prim_faces4[c0].Points[0]].Y;
				rz=prim_points[prim_faces4[c0].Points[0]].Z;
				unique++;
				at_face=c0;
				if(unique>1)
					return(0);
			}

		}
		index=p_thing->IndexNext;
	}
	*dx=rx;
	*dy=ry;
	*dz=rz;
	return(unique);


	
}
UWORD	find_unique_face_to_offset(CBYTE *fname,SLONG *x,SLONG *y,SLONG *z)
{
	UWORD			text;
	SLONG			c0;
	MFFileHandle	handle;
	PrimPoint		pf[3];
//	struct		PrimFace3 pf3;
	struct		PrimFace4 pf4;
	SLONG 	count;
	SLONG	save_type;
	SLONG unique;

	SLONG	dx,dy,dz,dx2,dy2,dz2;

	handle	=	FileOpen(fname);
	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,(UBYTE*)&save_type,sizeof(save_type));
//		LogText(" fing unique save type=%d \n",save_type);
		while(1)
		{
			
			FileRead(handle,(UBYTE*)&count,sizeof(count));
			if(count==-1)
				break;

			for(c0=0;c0<count;c0++)
			{
				FileRead(handle,(UBYTE*)&pf[0],sizeof(struct	PrimPoint)*3);

				FileRead(handle,(UBYTE*)&pf4.TexturePage,sizeof(pf4.TexturePage));
				FileRead(handle,(UBYTE*)&pf4.DrawFlags,sizeof(pf4.DrawFlags));
				if(save_type>2)
					FileRead(handle,(UBYTE*)&pf4.Col2,sizeof(pf4.Col2));
				if(save_type>3)
					FileRead(handle,(UBYTE*)&pf4.FaceFlags,sizeof(pf4.FaceFlags));
				FileRead(handle,(UBYTE*)&pf4.UV[0][0],sizeof(pf4.UV));

				dx=pf[1].X-pf[0].X;
				dy=pf[1].Y-pf[0].Y;
				dz=pf[1].Z-pf[0].Z;

				dx2=pf[2].X-pf[0].X;
				dy2=pf[2].Y-pf[0].Y;
				dz2=pf[2].Z-pf[0].Z;

				unique=is_this_unique(&dx,&dy,&dz,dx2,dy2,dz2);
				if(unique==1)
				{
					*x=pf[0].X-dx; //return offset
					*y=pf[0].Y-dy;
					*z=pf[0].Z-dz;
					FileClose(handle);
					return(1);
				}

			}
			FileRead(handle,(UBYTE*)&count,sizeof(count));
/*
			for(c0=0;c0<count;c0++)
			{
				FileRead(handle,(UBYTE*)&pf[0],sizeof(struct	PrimPoint)*3);

				FileRead(handle,(UBYTE*)&pf3.TexturePage,sizeof(pf3.TexturePage));
				FileRead(handle,(UBYTE*)&pf3.DrawFlags,sizeof(pf3.DrawFlags));
				FileRead(handle,(UBYTE*)&pf3.UV,sizeof(pf3.UV));

//				find_and_apply_to_tri(&pf[0],&pf3);
			}
*/
		}
		FileClose(handle);
	}
  	return(0);
}
SLONG	sum_shared_brightness_flag(SWORD shared_point)
{
	SLONG	c0,point;
	SLONG	face;
	SLONG	bright=0;
	SLONG count=0;

	SWORD	index;
	struct	MapThing	*p_thing;

	SLONG			sf,ef;
	UWORD	prim;
	
	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		prim=p_thing->IndexOther;

		sf=prim_objects[prim].StartFace3;
		ef=prim_objects[prim].EndFace3;


		for(c0=sf;c0<ef;c0++)
		{
			face=c0;
			if(prim_faces3[face].FaceFlags&FACE_FLAG_SMOOTH)
			{
				for(point=0;point<3;point++)
				{
					if(prim_faces3[face].Points[point]==shared_point)
					{
						bright+=prim_faces3[face].Bright[point];
						count++;
					}
				}
			}
		}

		sf=prim_objects[prim].StartFace4;
		ef=prim_objects[prim].EndFace4;

		for(c0=sf;c0<ef;c0++)
		{
			face=c0;
			if(prim_faces4[face].FaceFlags&FACE_FLAG_SMOOTH)
			{
				for(point=0;point<4;point++)
				{
					if(prim_faces4[face].Points[point]==shared_point)
					{
						bright+=prim_faces4[face].Bright[point];
						count++;
					}
				}
				
			}
		}
		index=p_thing->IndexNext;
	}

	if(count)
		return(bright/count);
	else
		return(0);
}

void	set_shared_brightness_flag(SWORD shared_point,SWORD bright)
{
	SLONG	c0,point;
	SLONG	face;

	SWORD	index;
	struct	MapThing	*p_thing;
	SLONG			sf,ef;
	UWORD	prim;

	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		prim=p_thing->IndexOther;

		sf=prim_objects[prim].StartFace3;
		ef=prim_objects[prim].EndFace3;


		for(c0=sf;c0<ef;c0++)
		{
			face=c0;
			if(prim_faces3[face].FaceFlags&FACE_FLAG_SMOOTH)
			{
				for(point=0;point<3;point++)
				{
					if(prim_faces3[face].Points[point]==shared_point)
					{
						prim_faces3[face].Bright[point]=bright;
					}
				}
			}
		}

		sf=prim_objects[prim].StartFace4;
		ef=prim_objects[prim].EndFace4;

		for(c0=sf;c0<ef;c0++)
		{
			face=c0;
			if(prim_faces4[face].FaceFlags&FACE_FLAG_SMOOTH)
			{
				for(point=0;point<4;point++)
				{
					if(prim_faces4[face].Points[point]==shared_point)
					{
						prim_faces4[face].Bright[point]=bright;
					}
				}
				
			}
		}
		index=p_thing->IndexNext;
	}
}

void	smooth_faces(void)
{
	SLONG	c0,c1,point;
	SLONG	face;
	SLONG	bright;

	SWORD	index;
	struct	MapThing	*p_thing;
	SLONG			sf,ef;
	UWORD	prim;

	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		prim=p_thing->IndexOther;

		sf=prim_objects[prim].StartFace3;
		ef=prim_objects[prim].EndFace3;


		for(c0=sf;c0<ef;c0++)
		{
			face=c0;
			if(prim_faces3[face].FaceFlags&FACE_FLAG_SMOOTH)
			{
				for(point=0;point<3;point++)
				{
					bright=sum_shared_brightness_flag(prim_faces3[face].Points[point]);
					set_shared_brightness_flag(prim_faces3[face].Points[point],bright);
				}
			}
		}

		sf=prim_objects[prim].StartFace4;
		ef=prim_objects[prim].EndFace4;
		for(c0=sf;c0<ef;c0++)
		{
			face=c0;
			if(prim_faces4[face].FaceFlags&FACE_FLAG_SMOOTH)
			{
				for(point=0;point<4;point++)
				{
					bright=sum_shared_brightness_flag(prim_faces4[face].Points[point]);
					set_shared_brightness_flag(prim_faces4[face].Points[point],bright);
				}
				
			}
		}
		index=p_thing->IndexNext;
	}
}


extern	void	apply_global_amb_to_map(void);

extern	SWORD	CreateALightThing(SLONG x,SLONG y,SLONG z,SLONG bright);

void	apply_map_thing(SLONG dx,SLONG dy,SLONG dz,struct	MapThing	*p_thing)
{
	switch(p_thing->Type)
	{
		case	MAP_THING_TYPE_LIGHT:
			CreateALightThing(dx+p_thing->X,dy+p_thing->Y,dz+p_thing->Z,p_thing->IndexOther);
			break;
	}
}

void	import_tex(CBYTE *fname)
{
	UWORD			text;
	SLONG			c0;
	MFFileHandle	handle;
	PrimPoint		pf[3];
	struct		PrimFace3 pf3;
	struct		PrimFace4 pf4;
	SLONG 	count;
	SLONG	save_type;
	SLONG	gx=0,gy=0,gz=0;
	SLONG	remap_quad=0,remap_tri=0;
	
	if(ShiftFlag)
	{
		
		if(!find_unique_face_to_offset(fname,&gx,&gy,&gz))
		{
//			LogText(" could not find a unique face to offset\n");
			return;
		}
	}

//	LogText("UNIQUE FACE offset= %d,%d,%d\n",gx,gy,gz);
	memset(&pf3,0,sizeof(struct PrimFace3));
	memset(&pf4,0,sizeof(struct PrimFace4));

	handle	=	FileOpen(fname);
	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,(UBYTE*)&save_type,sizeof(save_type));
//		LogText(" IMPORT TEX save type=%d \n",save_type);
		while(1)
		{
			
			FileRead(handle,(UBYTE*)&count,sizeof(count));
			if(count==-1)
				break;

			for(c0=0;c0<count;c0++)
			{
				if(save_type<5)
				{
					SLONG	c0;
					struct	OldPrimPoint	pp;
					for(c0=0;c0<3;c0++)
					{

						FileRead(handle,(UBYTE*)&pp,sizeof(struct	OldPrimPoint));
						pf[c0].X=(SWORD)pp.X;
						pf[c0].Y=(SWORD)pp.Y;
						pf[c0].Z=(SWORD)pp.Z;
					}
				}
				else
				{
					FileRead(handle,(UBYTE*)&pf[0],sizeof(struct	PrimPoint)*3);
				}

				FileRead(handle,(UBYTE*)&pf4.TexturePage,sizeof(pf4.TexturePage));
				FileRead(handle,(UBYTE*)&pf4.DrawFlags,sizeof(pf4.DrawFlags));
				if(save_type>2)
					FileRead(handle,(UBYTE*)&pf4.Col2,sizeof(pf4.Col2));
				if(save_type>3)
					FileRead(handle,(UBYTE*)&pf4.FaceFlags,sizeof(pf4.FaceFlags));
				FileRead(handle,(UBYTE*)&pf4.UV[0][0],sizeof(pf4.UV));

				if(find_and_apply_to_quad(&pf[0],&pf4,gx,gy,gz))
					remap_quad++;
			}
			FileRead(handle,(UBYTE*)&count,sizeof(count));

			for(c0=0;c0<count;c0++)
			{

				if(save_type<5)
				{
					SLONG	c0;
					struct	OldPrimPoint	pp;
					for(c0=0;c0<3;c0++)
					{

						FileRead(handle,(UBYTE*)&pp,sizeof(struct	OldPrimPoint));
						pf[c0].X=(SWORD)pp.X;
						pf[c0].Y=(SWORD)pp.Y;
						pf[c0].Z=(SWORD)pp.Z;
					}
				}
				else
				{
					FileRead(handle,(UBYTE*)&pf[0],sizeof(struct	PrimPoint)*3);
				}

				FileRead(handle,(UBYTE*)&pf3.TexturePage,sizeof(pf3.TexturePage));
				FileRead(handle,(UBYTE*)&pf3.DrawFlags,sizeof(pf3.DrawFlags));
				if(save_type>2)
					FileRead(handle,(UBYTE*)&pf3.Col2,sizeof(pf3.Col2));
				if(save_type>3)
					FileRead(handle,(UBYTE*)&pf3.FaceFlags,sizeof(pf3.FaceFlags));
				FileRead(handle,(UBYTE*)&pf3.UV[0][0],sizeof(pf3.UV));

				if(find_and_apply_to_tri(&pf[0],&pf3,gx,gy,gz))
					remap_tri++;
			}
		}
		if(save_type>1)
		{
			struct	MapThing	mt;
			SLONG	c0;
			for(c0=0;c0<MAX_MAP_THINGS;c0++)
			{
				FileRead(handle,(UBYTE*)&mt,sizeof(struct MapThing));
				apply_map_thing(gx,gy,gz,&mt);
			}
			FileRead(handle,(UBYTE*)&edit_info.amb_dx,4*5);
//			apply_global_amb_to_map();
			if(save_type>3)
				smooth_faces();
		}
		FileClose(handle);
//		LogText(" quads remaped %d  Tri's Remaped %d \n",remap_quad,remap_tri);
	}
}

void save_asc(UWORD building,UWORD version)
{
	
//	struct	SingleTexture	*texture;
//	struct	SingleFloorTexture	*texture4;

	struct	PrimFace4		*this_face4;
	struct	PrimFace3		*this_face;
	struct	BuildingFacet	*p_facet;
	struct	BuildingObject	*point_object;

	SLONG	no_faces,start_face,current_face;
	SLONG	no_faces4,start_face4;
	MFFileHandle	fpz;
	CBYTE name[50];
	UWORD	save_type=0,col;
	SLONG	start_point,end_point,no_points;
	SLONG	c0;
	CBYTE	string[100];
	UBYTE	flag=0;
	UWORD	count;

	point_object=&building_objects[building];
	no_faces=point_object->EndFace3-point_object->StartFace3;
	start_face=point_object->StartFace3;

	no_faces4=point_object->EndFace4-point_object->StartFace4;;
	start_face4=point_object->StartFace4;

	start_point=point_object->StartPoint;

	sprintf((char*)name,"data\\nobj%02d.asc",building);
	if((fpz=FileCreate(name,1)) != FILE_OPEN_ERROR)
	{
		sprintf((char*)string,"Ambient light color: Red=0.039216 Green=0.039216 Blue=0.039216%c\n%c\nNamed object: \"Object%02d\"%c\n",13,13,"ref_space",13);
		FileWrite(fpz,string,strlen(string));

		sprintf((char*)string,"Tri-mesh, Vertices: %d Faces: %d%c\n",point_object->EndPoint-point_object->StartPoint+1,no_faces+no_faces4*2,13);
		FileWrite(fpz,string,strlen(string));

		sprintf((char*)string,"Vertex list:%c\n",13);
		FileWrite(fpz,string,strlen(string));

		for(c0=start_point;c0<=point_object->EndPoint;c0++)
		{
			sprintf((char*)string,"Vertex %d:  X: %d  Y: %d  Z: %d %c\n",c0-point_object->StartPoint
											,prim_points[c0].X,prim_points[c0].Z,prim_points[c0].Y,13);
			FileWrite(fpz,string,strlen(string));
		}

		sprintf((char*)string,"%c\nFace list:%c\n",13,13);
		FileWrite(fpz,string,strlen(string));
		count=0;
		for(c0=0;c0<no_faces;c0++)
		{
			this_face=&prim_faces3[start_face+c0];
			sprintf((char*)string,"Face %d: A:%d B:%d C:%d AB:1 BC:1 CA:1 %c\n",count++,
			this_face->Points[0]-start_point,this_face->Points[1]-start_point,this_face->Points[2]-start_point,13);
			FileWrite(fpz,string,strlen(string));
		}

		for(c0=0;c0<no_faces4;c0++)
		{
			this_face4=&prim_faces4[start_face4+c0];
			sprintf((char*)string,"Face %d: A:%d B:%d C:%d AB:1 BC:1 CA:1 %c\n",count++,
			this_face4->Points[0]-start_point,this_face4->Points[1]-start_point,this_face4->Points[2]-start_point,13);
			FileWrite(fpz,string,strlen(string));

			sprintf((char*)string,"Face %d: A:%d B:%d C:%d AB:1 BC:1 CA:1 %c\n",count++,
			this_face4->Points[3]-start_point,this_face4->Points[1]-start_point,this_face4->Points[2]-start_point,13);
			FileWrite(fpz,string,strlen(string));
		}

		FileClose(fpz);
		return;
	}
	else
	{
		LogText(" ERROR WRITING ASC PRIM %d\n",building);
	}
}

void save_prim_asc(UWORD prim,UWORD version)
{
	
//	struct	SingleTexture	*texture;
//	struct	SingleFloorTexture	*texture4;

	struct	PrimFace4		*this_face4;
	struct	PrimFace3		*this_face;
	struct	PrimObject	*point_object;

	SLONG	no_faces,start_face,current_face;
	SLONG	no_faces4,start_face4;
	MFFileHandle	fpz;
	CBYTE name[50];
	UWORD	save_type=0,col;
	SLONG	start_point,end_point,no_points;
	SLONG	c0;
	CBYTE	string[100];
	UBYTE	flag=0;
	UWORD	count;

	point_object=&prim_objects[prim];
	no_faces=point_object->EndFace3-point_object->StartFace3;
	start_face=point_object->StartFace3;

	no_faces4=point_object->EndFace4-point_object->StartFace4;;
	start_face4=point_object->StartFace4;

	start_point=point_object->StartPoint;

	sprintf((char*)name,"data\\nobj%02d.asc",prim);
	if((fpz=FileCreate(name,1)) != FILE_OPEN_ERROR)
	{
		sprintf((char*)string,"Ambient light color: Red=0.039216 Green=0.039216 Blue=0.039216%c\n%c\nNamed object: \"Object%02d\"%c\n",13,13,"ref_space",13);
		FileWrite(fpz,string,strlen(string));

		sprintf((char*)string,"Tri-mesh, Vertices: %d Faces: %d%c\n",point_object->EndPoint-point_object->StartPoint+1,no_faces+no_faces4*2,13);
		FileWrite(fpz,string,strlen(string));

		sprintf((char*)string,"Vertex list:%c\n",13);
		FileWrite(fpz,string,strlen(string));

		for(c0=start_point;c0<=point_object->EndPoint;c0++)
		{
			sprintf((char*)string,"Vertex %d:  X: %d  Y: %d  Z: %d %c\n",c0-point_object->StartPoint
											,prim_points[c0].X,prim_points[c0].Z,prim_points[c0].Y,13);
			FileWrite(fpz,string,strlen(string));
		}

		sprintf((char*)string,"%c\nFace list:%c\n",13,13);
		FileWrite(fpz,string,strlen(string));
		count=0;
		for(c0=0;c0<no_faces;c0++)
		{
			this_face=&prim_faces3[start_face+c0];
			sprintf((char*)string,"Face %d: A:%d B:%d C:%d AB:1 BC:1 CA:1 %c\n",count++,
			this_face->Points[0]-start_point,this_face->Points[1]-start_point,this_face->Points[2]-start_point,13);
			FileWrite(fpz,string,strlen(string));
		}

		for(c0=0;c0<no_faces4;c0++)
		{
			this_face4=&prim_faces4[start_face4+c0];
			sprintf((char*)string,"Face %d: A:%d B:%d C:%d AB:1 BC:1 CA:1 %c\n",count++,
			this_face4->Points[0]-start_point,this_face4->Points[1]-start_point,this_face4->Points[2]-start_point,13);
			FileWrite(fpz,string,strlen(string));

			sprintf((char*)string,"Face %d: A:%d B:%d C:%d AB:1 BC:1 CA:1 %c\n",count++,
			this_face4->Points[3]-start_point,this_face4->Points[1]-start_point,this_face4->Points[2]-start_point,13);
			FileWrite(fpz,string,strlen(string));
		}

		FileClose(fpz);
		return;
	}
	else
	{
		//LogText(" ERROR WRITING ASC PRIM %d\n",building);
	}
}


//
// Saves out the given prim object. Returns FALSE on failure.
//

SLONG save_prim_object(SLONG prim)
{
	SLONG i;

	SLONG num_points;
	SLONG num_faces3;
	SLONG num_faces4;

	PrimObject *po;

	CBYTE fname[256];
	FILE *handle;

	UWORD	save_type=PRIM_START_SAVE_TYPE+1;

	ASSERT(WITHIN(prim, 0, 265));

	po = &prim_objects[prim];
#ifdef	NO_SERVER
	sprintf(fname, "server\\prims\\nprim%03d.prm", prim);
#else
	sprintf(fname, "u:\\urbanchaos\\prims\\nprim%03d.prm", prim);
#endif

	handle = fopen(fname, "wb");

	if (!handle)
	{
		//
		// Oh dear!
		// 

		return FALSE;
	}
	
	//
	// Does this prim have any walkable faces?
	//

	po->flag &= ~PRIM_FLAG_CONTAINS_WALKABLE_FACES;

	for (i = po->StartFace4; i < po->EndFace4; i++)
	{
		if ((prim_faces4[i].DrawFlags & POLY_FLAG_WALKABLE) ||
			(prim_faces4[i].FaceFlags & FACE_FLAG_WALKABLE))
		{
			prim_faces4[i].FaceFlags |= FACE_FLAG_WALKABLE;
			prim_objects[prim].flag  |= PRIM_FLAG_CONTAINS_WALKABLE_FACES;
		}
	}

	//
	// Save out the prim object and all the point and faces data.
	//

	num_points = po->EndPoint - po->StartPoint;
	num_faces3 = po->EndFace3 - po->StartFace3;
	num_faces4 = po->EndFace4 - po->StartFace4;

//	prim_objects[prim].Dummy[3]=PRIM_START_SAVE_TYPE+save_type;
	fwrite(&save_type,sizeof(save_type),1, handle);
	fwrite(&prim_names[prim],32,1, handle);

	if (fwrite(&prim_objects[prim],           sizeof(PrimObject),          1, handle) !=          1) goto file_error;
	if (fwrite(&prim_points [po->StartPoint], sizeof(PrimPoint),  num_points, handle) != num_points) goto file_error;
	if (fwrite(&prim_faces3 [po->StartFace3], sizeof(PrimFace3),  num_faces3, handle) != num_faces3) goto file_error;
	if (fwrite(&prim_faces4 [po->StartFace4], sizeof(PrimFace4),  num_faces4, handle) != num_faces4) goto file_error;

	fclose(handle);

	//
	// All ok.
	//

	return TRUE;

  file_error:;

	//
	// An error occurred.
	//

	fclose(handle);

	return FALSE;
}

//
// Saves out all the individual prims.
//

void save_all_individual_prims()
{
	SLONG i;

	for (i = 1; i < 266; i++)
	{
		save_prim_object(i);
	}
}


