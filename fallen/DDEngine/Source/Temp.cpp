#ifdef TARGET_DC
#error Why is this included in a DC build? Bad person....
#endif



#include	"Engine.h"
#include	"crinkle.h"

#include	"c:\fallen\editor\headers\primativ.hpp"
//#include	"c:\fallen\editor\headers\building.hpp"
#include	"c:\fallen\editor\headers\Edit.h"
//#include	"c:\fallen\editor\headers\Engine.h"
#include	"c:\fallen\editor\headers\Map.h"
#include	"c:\fallen\editor\headers\prim_draw.h"
#include	"c:\fallen\editor\headers\Thing.h"
#include	"c:\fallen\headers\interact.h"
#include	"c:\fallen\headers\FMatrix.h"
//#include	"c:\fallen\editor\headers\collide.hpp"

#define	POLY_FLAG_TEXTURED		(1<<1)
#define	POLY_FLAG_MASKED		(1<<2)
#define	POLY_FLAG_DOUBLESIDED	(1<<6)

SVECTOR_F			dx_global_res[5560]; //max points per object?

struct	DXMaterial
{
	float		R,
				G,
				B;

};

extern SLONG                material_count;
extern DXMaterial			dx_materials[200];
extern SVECTOR_F			dx_prim_points[MAX_PRIM_POINTS];

#define	SHOE_SIZE			8
//---------------------------------------------------------------
void matrix_mult33(struct Matrix33* result,struct Matrix33* mat1,struct  Matrix33* mat2)
{
	result->M[0][0] = ((mat1->M[0][0]*mat2->M[0][0])+(mat1->M[0][1]*mat2->M[1][0])+(mat1->M[0][2]*mat2->M[2][0]))>>15; 
	result->M[0][1] = ((mat1->M[0][0]*mat2->M[0][1])+(mat1->M[0][1]*mat2->M[1][1])+(mat1->M[0][2]*mat2->M[2][1]))>>15; 
	result->M[0][2] = ((mat1->M[0][0]*mat2->M[0][2])+(mat1->M[0][1]*mat2->M[1][2])+(mat1->M[0][2]*mat2->M[2][2]))>>15; 
	result->M[1][0] = ((mat1->M[1][0]*mat2->M[0][0])+(mat1->M[1][1]*mat2->M[1][0])+(mat1->M[1][2]*mat2->M[2][0]))>>15; 
	result->M[1][1] = ((mat1->M[1][0]*mat2->M[0][1])+(mat1->M[1][1]*mat2->M[1][1])+(mat1->M[1][2]*mat2->M[2][1]))>>15; 
	result->M[1][2] = ((mat1->M[1][0]*mat2->M[0][2])+(mat1->M[1][1]*mat2->M[1][2])+(mat1->M[1][2]*mat2->M[2][2]))>>15; 
	result->M[2][0] = ((mat1->M[2][0]*mat2->M[0][0])+(mat1->M[2][1]*mat2->M[1][0])+(mat1->M[2][2]*mat2->M[2][0]))>>15; 
	result->M[2][1] = ((mat1->M[2][0]*mat2->M[0][1])+(mat1->M[2][1]*mat2->M[1][1])+(mat1->M[2][2]*mat2->M[2][1]))>>15; 
	result->M[2][2] = ((mat1->M[2][0]*mat2->M[0][2])+(mat1->M[2][1]*mat2->M[1][2])+(mat1->M[2][2]*mat2->M[2][2]))>>15;
}

void rotate_obj(SWORD xangle,SWORD yangle,SWORD zangle, Matrix33 *r3) 
{
	SLONG	sinx, cosx, siny, cosy, sinz, cosz;
 	SLONG	cxcz,sysz,sxsycz,sxsysz,sysx,cxczsy,sxsz,cxsysz,czsx,cxsy,sycz,cxsz;

	sinx = SIN(xangle & (2048-1)) >>1;  	// 15's
	cosx = COS(xangle & (2048-1)) >>1;
	siny = SIN(yangle & (2048-1)) >>1;
	cosy = COS(yangle & (2048-1)) >>1;
	sinz = SIN(zangle & (2048-1)) >>1;
	cosz = COS(zangle & (2048-1)) >>1;

	cxsy    = (cosx*cosy);				  		// 30's
	sycz    = (cosy*cosz);
	cxcz	= (cosx*cosz);
	cxsz	= (cosx*sinz);
	czsx	= (cosz*sinx);
	sysx    = (cosy*sinx);
	sysz	= (cosy*sinz);
	sxsz 	= (sinx*sinz);
	sxsysz  = (sxsz>>15)*siny;
	cxczsy	= (cxcz>>15)*siny;
	cxsysz  = ((cosx*siny)>>15)*sinz;
	sxsycz  = (czsx>>15)*siny;

	// Define rotation matrix r3.

	r3->M[0][0] = (sycz)>>15;						// 14's
	r3->M[0][1] = (-sysz)>>15;
	r3->M[0][2] = siny;
	r3->M[1][0] = (sxsycz+cxsz)>>15;
	r3->M[1][1] = (cxcz-sxsysz)>>15;
	r3->M[1][2] = (-sysx)>>15;
	r3->M[2][0] = (sxsz-cxczsy)>>15;
	r3->M[2][1] = (cxsysz+czsx)>>15;
	r3->M[2][2] = (cxsy)>>15;
}

struct	QuickMap
{
	SWORD	Prim;
	SWORD	Texture;
	SWORD	Bright;
	SWORD	Thing;
};




struct	TinyXZ	radius_pool[MAX_RADIUS*4*MAX_RADIUS*2];
struct	TinyXZ	*radius_ptr[MAX_RADIUS+2];

void	build_radius_info(void)
{
	SBYTE	*grid;
	SLONG	dx,dz;
	struct	TinyXZ	*ptr_rad;
	SLONG	actual_radius,radius,radius_offset,old_radius=0;
	SLONG	angle;
	SLONG	sum_count=0;
	SLONG	count=0;

	ptr_rad=radius_pool;


	grid=(SBYTE*)MemAlloc((MAX_RADIUS+1)*(MAX_RADIUS+1)*4);
	if(grid)
	{

		for(radius=(MAX_RADIUS<<2);radius>3;radius--)
		{
			if((radius>>2)!=old_radius)
			{
				old_radius=radius>>2;
//				LogText(" radius %d max_radius %d \n",radius>>2,MAX_RADIUS);
				radius_ptr[(radius>>2)]=ptr_rad;

			}
			for(angle=0;angle<2048;angle+=4)
			{
				for(radius_offset=-4;radius_offset<4;radius_offset++)
				{
					
					dx=(SIN(angle)*(radius+radius_offset))>>(16+2);
					dz=(COS(angle)*(radius+radius_offset))>>(16+2);
					actual_radius=Root(SDIST2(dx,dz));
					if(actual_radius==(radius>>2))
					{
						if(grid[(dx+MAX_RADIUS)+(dz+MAX_RADIUS)*(MAX_RADIUS*2)]!=-1)
						{
							grid[(dx+MAX_RADIUS)+(dz+MAX_RADIUS)*(MAX_RADIUS*2)]=-1;
							ptr_rad->Dx=dx;
							ptr_rad->Dz=dz;
							ptr_rad->Angle=angle;
							ptr_rad++;
						}
					}
				}
			}
		}
		radius_ptr[0]=ptr_rad;
		MemFree(grid);
	}
	for(radius=1;radius<MAX_RADIUS;radius++)
	{
//		LogText("count=%d \n",count);
//		LogText(" rad %d ->",radius);
		ptr_rad=radius_ptr[radius];
		count=0;
		while(ptr_rad<radius_ptr[radius-1])
		{
//			LogText("[%d](%d,%d) ",ptr_rad->Angle,ptr_rad->Dx,ptr_rad->Dz);
			ptr_rad++;
			count++;
		}
		sum_count+=count;
//		LogText("\n");
	}
//		LogText("count=%d sum %d\n",count,sum_count);
}

DXMaterial	dx_materials[200]=
{
	{0,0,0}
};

void	setup_anim_stuff(void)
{
#ifdef	EDITOR
extern	SLONG	key_frame_count,current_element;
	current_element	=	0;
	key_frame_count	=	0;
#endif
	if(the_elements)
		MemFree(the_elements);
	the_elements	=	(KeyFrameElement*)MemAlloc(MAX_NUMBER_OF_ELEMENTS*sizeof(KeyFrameElement));
}

//---------------------------------------------------------------

void	reset_anim_stuff(void)
{
	//if(test_chunk)
	{
		test_chunk.MultiObject=0;
		test_chunk.ElementCount=0;
	}
	if(the_elements)
		MemFree(the_elements);
}


void	load_anim(MFFileHandle file_handle,Anim *the_anim,KeyFrameChunk *the_chunk)
{
	CBYTE			anim_name[ANIM_NAME_SIZE];
	SLONG			anim_flags,
					c0,
					frame_count,
					frame_id,
					tween_step;
	KeyFrame		*the_frame;
	SWORD			chunk_id;
	SWORD			fixed=0;
	CBYTE			version=0;

	
	FileRead(file_handle,&version,1);

	if(version==0||version>20)
	{
		anim_name[0]=version;
		FileRead(file_handle,&anim_name[1],ANIM_NAME_SIZE-1);
		version=0;
	}
	else
		FileRead(file_handle,anim_name,ANIM_NAME_SIZE);
	

	FileRead(file_handle,&anim_flags,sizeof(anim_flags));
	FileRead(file_handle,&frame_count,sizeof(frame_count));
	the_anim->SetAnimName(anim_name);
	for(c0=0;c0<frame_count;c0++)
	{
		FileRead(file_handle,&chunk_id,sizeof(chunk_id));
		FileRead(file_handle,&frame_id,sizeof(frame_id));
		FileRead(file_handle,&tween_step,sizeof(tween_step));
		if(version>0)
			FileRead(file_handle,&fixed,sizeof(fixed));

		the_frame				=	&the_chunk->KeyFrames[frame_id];
		the_frame->TweenStep	=	tween_step;
		the_frame->Fixed		=	fixed;
		the_frame->Fight		=	0;
		if(version>1)
		{
			struct	FightCol	*fcol,*fcol_prev=0;
			SLONG	count,c0;

			FileRead(file_handle,&count,sizeof(count));
//			LogText(" fight count load = %d \n",count);

			for(c0=0;c0<count;c0++)
			{
				fcol = (struct FightCol*)MemAlloc(sizeof(struct FightCol));
				if(fcol)
				{
					FileRead(file_handle,fcol,sizeof(struct FightCol));
					if(c0==0)
					{
						the_frame->Fight=fcol;
					}
					else
					{
						fcol_prev->Next=fcol;
					}
					fcol_prev=fcol;
				}
			}
		}
		the_anim->AddKeyFrame(the_frame);
		the_frame->Fight=0;
	}
	the_anim->SetAnimFlags(anim_flags);
	if(anim_flags&ANIM_LOOP)
		the_anim->StartLooping();

}

//---------------------------------------------------------------

//---------------------------------------------------------------
Anim	*current_anim;

void	create_anim(Anim **the_list)
{
	CBYTE		text[32];
	Anim		*next_anim,
				*the_anim;


	the_anim	=	MFnew<Anim>();
	if(the_anim)
	{
	//	anim_count++;
   	//	sprintf(text,"New Anim %d",anim_count);
//		the_anim->SetAnimName(text);
		if(*the_list)
		{
			next_anim	=	*the_list;
			while(1)
			{
				if(next_anim->GetNextAnim())
					next_anim	=	next_anim->GetNextAnim();
				else
					break;
			}
			next_anim->SetNextAnim(the_anim);
			the_anim->SetLastAnim(next_anim);
		}
		else
		{
			*the_list	=	the_anim;
		}
		current_anim	=	the_anim;
	}
}

//---------------------------------------------------------------
void	load_body_part_info(MFFileHandle file_handle,SLONG version,KeyFrameChunk *the_chunk)
{
	SLONG	c0,c1;
	SLONG	no_people,no_body_bits,string_len;

	FileRead(file_handle,&no_people,sizeof(SLONG));

	FileRead(file_handle,&no_body_bits,sizeof(SLONG));

	FileRead(file_handle,&string_len,sizeof(SLONG));

	for(c0=0;c0<no_people;c0++)
	{
		FileRead(file_handle,the_chunk->PeopleNames[c0],string_len);
		for(c1=0;c1<no_body_bits;c1++)
			FileRead(file_handle,&the_chunk->PeopleTypes[c0].BodyPart[c1],sizeof(UBYTE));
	}
}

void	load_all_anims(KeyFrameChunk *the_chunk,Anim **anim_list)
{
//#ifdef	EDITOR
	SLONG			anim_count,version,
					c0;
	MFFileHandle	file_handle;


	file_handle	=	FileOpen(the_chunk->ANMName);
	if(file_handle!=FILE_OPEN_ERROR)
	{
		FileRead(file_handle,&anim_count,sizeof(anim_count));
		if(anim_count<0)
		{
			version=anim_count;

			FileRead(file_handle,&anim_count,sizeof(anim_count));

			load_body_part_info(file_handle,version,the_chunk);
		}
		for(c0=0;c0<anim_count;c0++)
		{
			create_anim(anim_list);
			load_anim(file_handle,current_anim,the_chunk);
		}
	}
	else
	{
		// Unable to open file.
	}
//#endif
}


//---------------------------------------------------------------
SLONG	calc_shadow_co_ord(struct SVECTOR *input,struct SVECTOR *output,SLONG l_x,SLONG l_y,SLONG l_z)
{
	SLONG	dx,dy,dz;
	SLONG	alt=0;
	SLONG	m,c;

//	dx=l_x-input->X;
//	dy=l_y-input->Y;
//	dz=l_z-input->Z;

	dx=-128;
	dy=128;
	dz=128;

extern	SLONG	calc_height_at(SLONG x,SLONG z);
	alt=calc_height_at(input->X,input->Z);

	if(dx)
	{
		m=(dy<<8)/dx;

		if(abs(m)>65536)
		{
			m>>=8;
			c=input->Y-((m*input->X));
			if(m)
				output->X=((alt-c))/m;
		}
		else
		{
			c=input->Y-((m*input->X)>>8);
			if(m)
				output->X=((alt-c)<<8)/m;
		}
	}
	else
	{
		output->X=input->X;
	}

	if(dz)
	{
		m=(dy<<8)/dz;

		if(abs(m)>65536)
		{
			m>>=8;
			c=input->Y-((m*input->Z));
			if(m)
				output->Z=((alt-c))/m;
		}
		else
		{
			c=input->Y-((m*input->Z)>>8);
			if(m)
				output->Z=((alt-c)<<8)/m;
		}
	}
	else
	{
		output->Z=input->Z;
	}

	//output->X=input->X;
	//output->Z=input->Z;

	output->Y=calc_height_at(output->X,output->Z);

	return(0);
}

SLONG	points_clockwise(SLONG p0,SLONG p1,SLONG p2)
{
	float res;
	float	vx,vy,wx,wy,z;

	vx=dx_global_res[p1].X-dx_global_res[p0].X;
	wx=dx_global_res[p2].X-dx_global_res[p1].X;
	vy=dx_global_res[p1].Y-dx_global_res[p0].Y;
	wy=dx_global_res[p2].Y-dx_global_res[p1].Y;

	z=vx*wy-vy*wx;

	if(z>0.0)
		return	1;
	else
		return	0;

}

SLONG	e_draw_a_facet_at(UWORD building_object, UWORD f_building, UWORD facet, SLONG x,SLONG y,SLONG z)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	ULONG	flag_and,flag_or;
	SLONG	c0;
	struct	BuildingFacet	*p_facet;
	struct  FBuilding       *p_building;
	SLONG	sp,ep;
	SLONG	az;
	SLONG	col=0,cor=0,cob=0,cot=0,total=0;
	SLONG	best_z=9999999;
	SLONG	min_z=9999999,max_z=-9999999;
	SLONG	first_face=1;

	SLONG	facet_flags;
	SLONG	offset_z=0;
	SLONG	list;

	float			average_z,
					f_x,f_y,f_z,
					r,g,b,
					shade,
					*rgb;
	BucketQuad		*the_quad;
	BucketTri		*the_tri;
	D3DTLVERTEX		*the_vertex;
	SVECTOR_F		temp;

	float			max_height;

	p_building  = &building_list  [f_building];
	p_facet     = &building_facets[facet];
	facet_flags =  p_facet->FacetFlags;

	UBYTE point_order[4] = {0, 1, 3, 2};
	
	/*

	//
	// Draw the normals of all the points.
	// 

	{
		SLONG i;
		SLONG x1, y1, z1;
		SLONG x2, y2, z2;

		for (i = p_facet->StartPoint; i < p_facet->EndPoint; i++)
		{
			x1 = prim_points[i].X + x;
			y1 = prim_points[i].Y + y;
			z1 = prim_points[i].Z + z;

			x2 = x1 + (prim_normal[i].X >> 3);
			y2 = y1 + (prim_normal[i].Y >> 3);
			z2 = z1 + (prim_normal[i].Z >> 3);

			e_draw_3d_line_col_sorted(
				x1, y1, z1, 
				x2, y2, z2,
				128, 130, 150);
		}
	}

	

	for(c0=p_facet->StartFace4;c0<p_facet->EndFace4;c0++)
	{
		p_f4 = &prim_faces4[c0];

		if (p_f4->FaceFlags & FACE_FLAG_WALKABLE)
		{
			SLONG i;
			SLONG p1;
			SLONG p2;

			for (i = 0; i < 4; i++)
			{
				if (p_f4->FaceFlags & (FACE_FLAG_SLIDE_EDGE << i))
				{
					p1 = p_f4->Points[point_order[(i + 0) & 0x3]];
					p2 = p_f4->Points[point_order[(i + 1) & 0x3]];

					e_draw_3d_line(
						prim_points[p1].X + x,
						prim_points[p1].Y + y,
						prim_points[p1].Z + z,
						prim_points[p2].X + x,
						prim_points[p2].Y + y,
						prim_points[p2].Z + z);
				}
			}
		}
	}
*/

	p_f4 = &prim_faces4[p_facet->StartFace4];
	p_f3 = &prim_faces3[p_facet->StartFace3];

	//
	// If we are indoors, then we only draw polys that are lower than
	// the storey we are currently in.
	//

	if (GAME_FLAGS & GF_INDOORS)
	{
		//
		// Add on a little bit just to make sure!
		//

		max_height  = (float) INDOORS_HEIGHT_CEILING;
		max_height += 128.0F;

		if (p_building->BuildingType == BUILDING_TYPE_CRATE_IN)
		{
			//
			// Always draw all of it.
			//

			max_height = (float) INFINITY;
		}
		else
		if (facet_flags & FACET_FLAG_ROOF)
		{
			//
			// Don't draw roofs inside buildings.
			//

			return best_z;	// Best zed is very far away!
		}
		else
		if (p_building->BuildingType == BUILDING_TYPE_WAREHOUSE)
		{
			//
			// Draw an extra storeys worth outside.
			//

			max_height += 256.0F;
		}
	}
	else
	{
		//
		// Draw everything if we are not indoors.
		//

		max_height = (float) INFINITY;
	}

	if(facet_flags&FACET_FLAG_ROOF)
	{
		first_face=0;
		offset_z=-50;
	}

	sp	=	p_facet->StartPoint;
	ep	=	p_facet->EndPoint;

	f_x	=	(float)x;
	f_y	=	(float)y;
	f_z	=	(float)z;

//	LogText(" facet sp ep %d %d \n",sp,ep);
	for(c0=sp;c0<ep;c0++)
	{
		//transform all points for this Object

		temp.X	=	dx_prim_points[c0].X+f_x;
		temp.Y	=	dx_prim_points[c0].Y+f_y;
		temp.Z	=	dx_prim_points[c0].Z+f_z;

		//
		// Only draw upto the max-height.
		//

		if (temp.Y <= max_height)
		{
			global_flags[c0-sp]	= transform_point(&temp,&dx_global_res[c0-sp]);
		}
		else
		{
			global_flags[c0-sp] = EF_BEHIND_YOU;
		}
//		if(c0==sp)
//			LogText(" draw facet %d point %d   xyz %d %d %d res %d %d %d flags %x\n",facet,c0,temp.X,temp.Y,temp.Z,dx_global_res[0].X,dx_global_res[0].Y,dx_global_res[0].Z,global_flags[0]);

//		calc_lights_dx(&temp,&prim_point_normals[c0],&global_light[(c0-sp)*3]);
	}

	if(p_facet->EndFace4)
	for(c0=p_facet->StartFace4;c0<p_facet->EndFace4;c0++)
	{
		SLONG	p0,p1,p2,p3;

		if(BUCKETS_FULL)
			goto	exit;

		p0	=	p_f4->Points[0]-sp;
		p1	=	p_f4->Points[1]-sp;
		p2	=	p_f4->Points[2]-sp;
		p3	=	p_f4->Points[3]-sp;
		ASSERT(p0>=0&&p0<ep-sp);
		ASSERT(p1>=0&&p1<ep-sp);
		ASSERT(p2>=0&&p2<ep-sp);
		ASSERT(p3>=0&&p3<ep-sp);

		if( (p_f4->DrawFlags&POLY_FLAG_DOUBLESIDED) || points_clockwise(p0,p1,p2))
		{

		
			flag_and	=	global_flags[p0]&global_flags[p1]&global_flags[p2]&global_flags[p3];
			flag_or		=	global_flags[p0]|global_flags[p1]|global_flags[p2]|global_flags[p3];

			if( (!(flag_and & EF_CLIPFLAGS))&&((flag_or&EF_BEHIND_YOU)==0))
			{
				the_quad			=	GET_BUCKET(BucketQuad);

				the_vertex			=	&vertex_pool[current_vertex];
				the_vertex->sx		=	dx_global_res[p0].X;
				the_vertex->sy		=	dx_global_res[p0].Y;
				the_vertex->sz		=	dx_global_res[p0].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);

				the_vertex++;
				the_vertex->sx		=	dx_global_res[p1].X;
				the_vertex->sy		=	dx_global_res[p1].Y;
				the_vertex->sz		=	dx_global_res[p1].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);

				the_vertex++;
				the_vertex->sx		=	dx_global_res[p2].X;
				the_vertex->sy		=	dx_global_res[p2].Y;
				the_vertex->sz		=	dx_global_res[p2].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);

				the_vertex++;
				the_vertex->sx		=	dx_global_res[p3].X;
				the_vertex->sy		=	dx_global_res[p3].Y;
				the_vertex->sz		=	dx_global_res[p3].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);

				if(p_f4->DrawFlags&POLY_FLAG_MASKED)
				{
					list	=	MASK_LIST_1;
					vertex_pool[current_vertex+0].tu	=	p_f4->UV[0][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+0].tv	=	p_f4->UV[0][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+1].tu	=	p_f4->UV[1][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+1].tv	=	p_f4->UV[1][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+2].tu	=	p_f4->UV[2][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+2].tv	=	p_f4->UV[2][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+3].tu	=	p_f4->UV[3][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+3].tv	=	p_f4->UV[3][1]*TEXTURE_MUL;
				}
				else if(p_f4->DrawFlags&POLY_FLAG_TEXTURED)
				{
					list	=	p_f4->TexturePage;
					vertex_pool[current_vertex+0].tu	=	p_f4->UV[0][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+0].tv	=	p_f4->UV[0][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+1].tu	=	p_f4->UV[1][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+1].tv	=	p_f4->UV[1][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+2].tu	=	p_f4->UV[2][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+2].tv	=	p_f4->UV[2][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+3].tu	=	p_f4->UV[3][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+3].tv	=	p_f4->UV[3][1]*TEXTURE_MUL;

					shade								=	p_f4->Bright[0]*SHADE_MUL;
					vertex_pool[current_vertex+0].color	=	D3DRGB(1.0*shade,1.0*shade,1.0*shade);
					shade								=	p_f4->Bright[1]*SHADE_MUL;
					vertex_pool[current_vertex+1].color	=	D3DRGB(1.0*shade,1.0*shade,1.0*shade);
					shade								=	p_f4->Bright[2]*SHADE_MUL;
					vertex_pool[current_vertex+2].color	=	D3DRGB(1.0*shade,1.0*shade,1.0*shade);
					shade								=	p_f4->Bright[3]*SHADE_MUL;
					vertex_pool[current_vertex+3].color	=	D3DRGB(1.0*shade,1.0*shade,1.0*shade);
				}
				else
				{
					list	=	COLOUR_LIST_1;
					r	=	dx_materials[p_f4->Col].R;
					g	=	dx_materials[p_f4->Col].G;
					b	=	dx_materials[p_f4->Col].B;
					shade								=	p_f4->Bright[0]*SHADE_MUL;
					vertex_pool[current_vertex+0].color	=	D3DRGB(r*shade,g*shade,b*shade);
					shade								=	p_f4->Bright[1]*SHADE_MUL;
					vertex_pool[current_vertex+1].color	=	D3DRGB(r*shade,g*shade,b*shade);
					shade								=	p_f4->Bright[2]*SHADE_MUL;
					vertex_pool[current_vertex+2].color	=	D3DRGB(r*shade,g*shade,b*shade);
					shade								=	p_f4->Bright[3]*SHADE_MUL;
					vertex_pool[current_vertex+3].color	=	D3DRGB(r*shade,g*shade,b*shade);
				}

				//
				// Draw a crinkle!
				//

				if (Keys[KB_Y])
				{
					SLONG point[4];

					point[0] = p0;
					point[1] = p1;
					point[2] = p2;
					point[3] = p3;

					CRINKLE_do(0, 1.0, &vertex_pool[current_vertex], point, list, (SLONG) average_z);
				}
				else
				{

					the_quad->P[0]			=	current_vertex++;
					the_quad->P[1]			=	current_vertex++;
					the_quad->P[2]			=	current_vertex++;
					the_quad->P[3]			=	current_vertex++;

					average_z				=	(
													dx_global_res[p0].Z,
													dx_global_res[p1].Z,
													dx_global_res[p2].Z,
													dx_global_res[p3].Z
												)	*	256.0f;

					add_bucket(&the_quad->BucketHeader,BUCKET_QUAD,list,(SLONG)average_z);

					if(p_f4->DrawFlags&POLY_FLAG_DOUBLESIDED)
					{
						the_quad			=	GET_BUCKET(BucketQuad);

						the_quad->P[0]			=	current_vertex-2;
						the_quad->P[1]			=	current_vertex-1;
						the_quad->P[2]			=	current_vertex-4;
						the_quad->P[3]			=	current_vertex-3;

						add_bucket(&the_quad->BucketHeader,BUCKET_QUAD,list,(SLONG)average_z);
					}
				}
			}
		}

		p_f4++;
	}

	if(p_facet->EndFace3)
	for(c0=p_facet->StartFace3;c0<p_facet->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		if(BUCKETS_FULL)
			goto	exit;

		p0	=	p_f3->Points[0]-sp;
		p1	=	p_f3->Points[1]-sp;
		p2	=	p_f3->Points[2]-sp;

		flag_and = global_flags[p0]&global_flags[p1]&global_flags[p2];	
		flag_or  = global_flags[p0]|global_flags[p1]|global_flags[p2];	

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
			the_tri			=	GET_BUCKET(BucketTri);

			the_vertex			=	&vertex_pool[current_vertex];
			the_vertex->sx		=	dx_global_res[p0].X;
			the_vertex->sy		=	dx_global_res[p0].Y;
			the_vertex->sz		=	dx_global_res[p0].Z;
//			the_vertex->rhw		=	1.0f/the_vertex->sz;
			the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
			the_vertex->tu		=	p_f3->UV[0][0]*TEXTURE_MUL;
			the_vertex->tv		=	p_f3->UV[0][1]*TEXTURE_MUL;
			the_vertex->color	=	D3DRGB	(
												1.0f,
												1.0f,
												1.0f
											);

			the_vertex++;
			the_vertex->sx		=	dx_global_res[p1].X;
			the_vertex->sy		=	dx_global_res[p1].Y;
			the_vertex->sz		=	dx_global_res[p1].Z;
//			the_vertex->rhw		=	1.0f/the_vertex->sz;
			the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
			the_vertex->tu		=	p_f3->UV[1][0]*TEXTURE_MUL;
			the_vertex->tv		=	p_f3->UV[1][1]*TEXTURE_MUL;
			the_vertex->color	=	D3DRGB	(
												1.0f,
												1.0f,
												1.0f
											);

			the_vertex++;
			the_vertex->sx		=	dx_global_res[p2].X;
			the_vertex->sy		=	dx_global_res[p2].Y;
			the_vertex->sz		=	dx_global_res[p2].Z;
//			the_vertex->rhw		=	1.0f/the_vertex->sz;
			the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
			the_vertex->tu		=	p_f3->UV[2][0]*TEXTURE_MUL;
			the_vertex->tv		=	p_f3->UV[2][1]*TEXTURE_MUL;
			the_vertex->color	=	D3DRGB	(
												1.0f,
												1.0f,
												1.0f
											);

			if(p_f3->DrawFlags&POLY_FLAG_MASKED)
			{
				list	=	MASK_LIST_1;
				vertex_pool[current_vertex+0].tu	=	p_f3->UV[0][0]*TEXTURE_MUL;
				vertex_pool[current_vertex+0].tv	=	p_f3->UV[0][1]*TEXTURE_MUL;
				vertex_pool[current_vertex+1].tu	=	p_f3->UV[1][0]*TEXTURE_MUL;
				vertex_pool[current_vertex+1].tv	=	p_f3->UV[1][1]*TEXTURE_MUL;
				vertex_pool[current_vertex+2].tu	=	p_f3->UV[2][0]*TEXTURE_MUL;
				vertex_pool[current_vertex+2].tv	=	p_f3->UV[2][1]*TEXTURE_MUL;
			}
			else if(p_f3->DrawFlags&POLY_FLAG_TEXTURED)
			{
				list	=	p_f3->TexturePage;
				vertex_pool[current_vertex+0].tu	=	p_f3->UV[0][0]*TEXTURE_MUL;
				vertex_pool[current_vertex+0].tv	=	p_f3->UV[0][1]*TEXTURE_MUL;
				vertex_pool[current_vertex+1].tu	=	p_f3->UV[1][0]*TEXTURE_MUL;
				vertex_pool[current_vertex+1].tv	=	p_f3->UV[1][1]*TEXTURE_MUL;
				vertex_pool[current_vertex+2].tu	=	p_f3->UV[2][0]*TEXTURE_MUL;
				vertex_pool[current_vertex+2].tv	=	p_f3->UV[2][1]*TEXTURE_MUL;

				shade								=	p_f3->Bright[0]*SHADE_MUL;
				vertex_pool[current_vertex+0].color	=	D3DRGB(1.0*shade,1.0*shade,1.0*shade);
				shade								=	p_f3->Bright[1]*SHADE_MUL;
				vertex_pool[current_vertex+1].color	=	D3DRGB(1.0*shade,1.0*shade,1.0*shade);
				shade								=	p_f3->Bright[2]*SHADE_MUL;
				vertex_pool[current_vertex+2].color	=	D3DRGB(1.0*shade,1.0*shade,1.0*shade);
			}
			else
			{
				list	=	COLOUR_LIST_1;
				r	=	dx_materials[p_f3->Col].R;
				g	=	dx_materials[p_f3->Col].G;
				b	=	dx_materials[p_f3->Col].B;
				shade								=	p_f3->Bright[0]*SHADE_MUL;
				vertex_pool[current_vertex+0].color	=	D3DRGB(r*shade,g*shade,b*shade);
				shade								=	p_f3->Bright[1]*SHADE_MUL;
				vertex_pool[current_vertex+1].color	=	D3DRGB(r*shade,g*shade,b*shade);
				shade								=	p_f3->Bright[2]*SHADE_MUL;
				vertex_pool[current_vertex+2].color	=	D3DRGB(r*shade,g*shade,b*shade);
			}

			the_tri->P[0]			=	current_vertex++;
			the_tri->P[1]			=	current_vertex++;
			the_tri->P[2]			=	current_vertex++;

			average_z				=	(
											dx_global_res[p0].Z,
											dx_global_res[p1].Z,
											dx_global_res[p2].Z
										)	*	341.33f;

			add_bucket(&the_tri->BucketHeader,BUCKET_TRI,list,(SLONG)average_z);

			if(p_f3->DrawFlags&POLY_FLAG_DOUBLESIDED)
			{
				the_tri			=	GET_BUCKET(BucketTri);

				the_tri->P[0]			=	current_vertex-2;
				the_tri->P[1]			=	current_vertex-1;
				the_tri->P[2]			=	current_vertex-3;

				add_bucket(&the_tri->BucketHeader,BUCKET_TRI,list,(SLONG)average_z);
			}
		}
		p_f3++;
	}
exit:;

	return(best_z);
}

//---------------------------------------------------------------

void	e_draw_a_building_at(UWORD building_o, UWORD building_f,SLONG x,SLONG y,SLONG z)
{
	SLONG	index;
	SLONG	best_z=-999999,az;


	index	=	building_objects[building_o].FacetHead;
	while(index)
	{
		az = e_draw_a_facet_at(building_o, building_f, index, x, y, z);
		if(best_z<az)
			best_z=az;
		index = building_facets[index].NextFacet;
	}
}

//---------------------------------------------------------------

void	e_draw_a_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,UBYTE shade)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	ULONG	flag_and,flag_or;
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG az;
	SLONG col=0,cor=0,cob=0,cot=0,total=0;

	float			average_z,
					f_x,f_y,f_z,
					*rgb;
	BucketQuad		*the_quad;
	BucketTri		*the_tri;
	D3DTLVERTEX		*the_vertex;
	SVECTOR_F		temp;

	p_obj	=	&prim_objects[prim];
	p_f4	=	&prim_faces4[p_obj->StartFace4];
	p_f3	=	&prim_faces3[p_obj->StartFace3];

	sp		=	p_obj->StartPoint;
	ep		=	p_obj->EndPoint;
	
	f_x	=	(float)x;
	f_y	=	(float)y;
	f_z	=	(float)z;
		
	for(c0=sp;c0<ep;c0++)
	{
		//transform all points for this Object

		temp.X	=	dx_prim_points[c0].X+f_x;
		temp.Y	=	dx_prim_points[c0].Y+f_y;
		temp.Z	=	dx_prim_points[c0].Z+f_z;

		global_flags[c0-sp]	=	transform_point(&temp,&dx_global_res[c0-sp]);
//		calc_lights_dx(&temp,&prim_point_normals[c0],&global_light[(c0-sp)*3]);
	}

	if(p_obj->EndFace4)
	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		SLONG	p0,p1,p2,p3;

		if(BUCKETS_FULL)
			goto	exit;

		p0	=	p_f4->Points[0]-sp;
		p1	=	p_f4->Points[1]-sp;
		p2	=	p_f4->Points[2]-sp;
		p3	=	p_f4->Points[3]-sp;
	
		if( (p_f4->DrawFlags&POLY_FLAG_DOUBLESIDED) || points_clockwise(p0,p1,p2))
		{


			flag_and = global_flags[p0]&global_flags[p1]&global_flags[p2]&global_flags[p3];	
			flag_or = global_flags[p0]|global_flags[p1]|global_flags[p2]|global_flags[p3];	

			if( (!(flag_and & EF_CLIPFLAGS))&&((flag_or&EF_BEHIND_YOU)==0))
			{
				the_quad			=	GET_BUCKET(BucketQuad);

				the_vertex			=	&vertex_pool[current_vertex];
				the_vertex->sx		=	dx_global_res[p0].X;
				the_vertex->sy		=	dx_global_res[p0].Y;
				the_vertex->sz		=	dx_global_res[p0].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->tu		=	p_f4->UV[0][0]*TEXTURE_MUL;
				the_vertex->tv		=	p_f4->UV[0][1]*TEXTURE_MUL;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);
				the_vertex++;
				the_vertex->sx		=	dx_global_res[p1].X;
				the_vertex->sy		=	dx_global_res[p1].Y;
				the_vertex->sz		=	dx_global_res[p1].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->tu		=	p_f4->UV[1][0]*TEXTURE_MUL;
				the_vertex->tv		=	p_f4->UV[1][1]*TEXTURE_MUL;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);
				the_vertex++;
				the_vertex->sx		=	dx_global_res[p2].X;
				the_vertex->sy		=	dx_global_res[p2].Y;
				the_vertex->sz		=	dx_global_res[p2].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->tu		=	p_f4->UV[2][0]*TEXTURE_MUL;
				the_vertex->tv		=	p_f4->UV[2][1]*TEXTURE_MUL;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);
				the_vertex++;
				the_vertex->sx		=	dx_global_res[p3].X;
				the_vertex->sy		=	dx_global_res[p3].Y;
				the_vertex->sz		=	dx_global_res[p3].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->tu		=	p_f4->UV[3][0]*TEXTURE_MUL;
				the_vertex->tv		=	p_f4->UV[3][1]*TEXTURE_MUL;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);

				if(p_f4->DrawFlags&POLY_FLAG_TEXTURED)
				{

				}
				else
				{
	/*
					setCol4	(
								(struct BucketQuad*)current_bucket_pool,
								p_f4->Col
							);
	*/
				}
				the_quad->P[0]			=	current_vertex++;
				the_quad->P[1]			=	current_vertex++;
				the_quad->P[2]			=	current_vertex++;
				the_quad->P[3]			=	current_vertex++;

				average_z				=	(
												dx_global_res[p0].Z,
												dx_global_res[p1].Z,
												dx_global_res[p2].Z,
												dx_global_res[p3].Z
											)	*	256.0f;

				add_bucket(&the_quad->BucketHeader,BUCKET_QUAD,p_f4->TexturePage,(SLONG)average_z);

				if(p_f4->DrawFlags&POLY_FLAG_DOUBLESIDED)
				{
					the_quad			=	GET_BUCKET(BucketQuad);

					the_quad->P[0]			=	current_vertex-2;
					the_quad->P[1]			=	current_vertex-1;
					the_quad->P[2]			=	current_vertex-4;
					the_quad->P[3]			=	current_vertex-3;

					add_bucket(&the_quad->BucketHeader,BUCKET_QUAD,p_f4->TexturePage,(SLONG)average_z);
				}
			}
		}

		p_f4++;
	}

	if(p_obj->EndFace3)
	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		if(BUCKETS_FULL)
			goto	exit;

		p0	=	p_f3->Points[0]-sp;
		p1	=	p_f3->Points[1]-sp;
		p2	=	p_f3->Points[2]-sp;

		if( (p_f3->DrawFlags&POLY_FLAG_DOUBLESIDED) || points_clockwise(p0,p1,p2))
		{
		flag_and = global_flags[p0]&global_flags[p1]&global_flags[p2];	
		flag_or  = global_flags[p0]|global_flags[p1]|global_flags[p2];	

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
			{
				the_tri			=	GET_BUCKET(BucketTri);

				the_vertex			=	&vertex_pool[current_vertex];
				the_vertex->sx		=	dx_global_res[p0].X;
				the_vertex->sy		=	dx_global_res[p0].Y;
				the_vertex->sz		=	dx_global_res[p0].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->tu		=	p_f3->UV[0][0]*TEXTURE_MUL;
				the_vertex->tv		=	p_f3->UV[0][1]*TEXTURE_MUL;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);
				the_vertex++;
				the_vertex->sx		=	dx_global_res[p1].X;
				the_vertex->sy		=	dx_global_res[p1].Y;
				the_vertex->sz		=	dx_global_res[p1].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->tu		=	p_f3->UV[1][0]*TEXTURE_MUL;
				the_vertex->tv		=	p_f3->UV[1][1]*TEXTURE_MUL;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);
				the_vertex++;
				the_vertex->sx		=	dx_global_res[p2].X;
				the_vertex->sy		=	dx_global_res[p2].Y;
				the_vertex->sz		=	dx_global_res[p2].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->tu		=	p_f3->UV[2][0]*TEXTURE_MUL;
				the_vertex->tv		=	p_f3->UV[2][1]*TEXTURE_MUL;
				the_vertex->color	=	D3DRGB	(
													1.0f,
													1.0f,
													1.0f
												);
	/*
				setPolyType3(
								current_bucket_pool,
								p_f3->DrawFlags
							);
	*/
				if(p_f3->DrawFlags&POLY_FLAG_TEXTURED)
				{

				}
				else
				{
	/*
					setCol3	(
								(struct BucketQuad*)current_bucket_pool,
								p_f3->Col
							);
	*/
				}
				the_tri->P[0]			=	current_vertex++;
				the_tri->P[1]			=	current_vertex++;
				the_tri->P[2]			=	current_vertex++;

				average_z				=	(
												dx_global_res[p0].Z,
												dx_global_res[p1].Z,
												dx_global_res[p2].Z
											)	*	341.33f;

				add_bucket(&the_tri->BucketHeader,BUCKET_TRI,p_f3->TexturePage,(SLONG)average_z);

				if(p_f3->DrawFlags&POLY_FLAG_DOUBLESIDED)
				{
					the_tri			=	GET_BUCKET(BucketTri);

					the_tri->P[0]			=	current_vertex-2;
					the_tri->P[1]			=	current_vertex-1;
					the_tri->P[2]			=	current_vertex-3;

					add_bucket(&the_tri->BucketHeader,BUCKET_TRI,p_f3->TexturePage,(SLONG)average_z);
				}
			}
		}
		p_f3++;
	}
exit:;
}

//---------------------------------------------------------------

//void	e_draw_prim_tween(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,struct Matrix33 *rot_mat,ULONG shadow);
void	e_draw_prim_tween(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct GameKeyFrameElement *anim_info,struct GameKeyFrameElement *anim_info_next,struct Matrix33 *rot_mat,ULONG shadow,SWORD dx,SWORD dy,SWORD dz);
/*
void	e_draw_test_bloke(SLONG x,SLONG y,SLONG z,UBYTE anim,SLONG angle)
{
	SLONG				c0,c1;
	struct Matrix33		r_matrix;
	struct	MapThing	*p_mthing;




void	animate_bloke(void);
	if(anim)
		animate_bloke();
extern MapThing	man_thing;
	p_mthing=&man_thing;
	
	p_mthing->X	=	x;
	p_mthing->Y	=	y;
	p_mthing->Z	=	z;

	rotate_obj	(
					p_mthing->AngleX,
					(p_mthing->AngleY+angle)&2047,
					p_mthing->AngleZ,
					&r_matrix
				);
extern KeyFrameChunk 	test_chunk;
	if(p_mthing->AnimElements&&p_mthing->NextAnimElements)
	{
		for(c1=0,c0=prim_multi_objects[test_chunk.MultiObject].StartObject;c0<=prim_multi_objects[test_chunk.MultiObject].EndObject;c0++,c1++)
		{
			if(c1==1)
			{
extern UBYTE	store_pos;
				store_pos	=	1;
			}
			e_draw_prim_tween	(
								c0,
								p_mthing->X,p_mthing->Y,p_mthing->Z,
								p_mthing->TweenStage,
								&p_mthing->AnimElements[c1],
								&p_mthing->NextAnimElements[c1],
								&r_matrix,0,0,0
							);
		}
	}
}
*/
//---------------------------------------------------------------

void	e_draw_figure(Thing *p_thing,DrawTween *draw_info,SLONG x,SLONG y,SLONG z)
{
	SLONG			c0,c1;
	Matrix33		r_matrix;
	GameKeyFrameElement		*anim_elements,
						*next_anim_elements;

	SLONG	dx=0,dy=0,dz=0;
	SLONG	x1,y1,z1,x2,y2,z2;

	//
	// Find the lighting context for this thing.  The e_draw_prim_tween() function
	// calls LIGHT_get_point(). LIGHT_get_point returns the light on the given
	// point in this context.
	//

	LIGHT_get_context(THING_NUMBER(p_thing));

	if(draw_info->CurrentFrame==0 || draw_info->NextFrame==0)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR e_draw_figure");
		return;
	}

	if(draw_info->Locked)
	{
		MSG_add("Locked animate %d \n",draw_info->Locked);
		calc_sub_objects_position_global(draw_info->CurrentFrame,draw_info->NextFrame,0,draw_info->Locked,&x1,&y1,&z1);
		calc_sub_objects_position_global(draw_info->CurrentFrame,draw_info->NextFrame,256,draw_info->Locked,&x2,&y2,&z2);
		dx=-x2+x1;
		dy=-y2+y1;
		dz=-z2+z1;
//		LogText(" figure start frame %d %d %d \n",x1+p_thing->WorldPos.X,y1+p_thing->WorldPos.Y,z1+p_thing->WorldPos.Z);
//		LogText(" figure end frame %d %d %d \n",x2+p_thing->WorldPos.X,y2+p_thing->WorldPos.Y,z2+p_thing->WorldPos.Z);
//		LogText(" dx dy dz %d %d %d \n",dx,dy,dz);


	}



//		draw_info->TweenStage	=	draw_info->AnimTween;
	anim_elements	=	draw_info->CurrentFrame->FirstElement;
	if(draw_info->NextFrame)
		next_anim_elements	=	draw_info->NextFrame->FirstElement;
	else
		next_anim_elements	=	anim_elements;
	

	
	rotate_obj	(
					draw_info->Tilt,
					draw_info->Angle,
					draw_info->Roll,
					&r_matrix
				);

	if(anim_elements && next_anim_elements)
	{
		SLONG			ele_count,start_object;

		ele_count=draw_info->TheChunk->ElementCount;
		start_object=prim_multi_objects[draw_info->TheChunk->MultiObject[0]].StartObject;
		for(c1=0;c1<test_chunk.ElementCount;c1++)
		{
			SLONG	object_offset;
			//
			// for each vue frame we need an object to draw
			//
			object_offset=draw_info->TheChunk->PeopleTypes[draw_info->PersonID].BodyPart[c1];

			e_draw_prim_tween	(
								start_object+object_offset,
								x,y+SHOE_SIZE,z,
								draw_info->AnimTween,
								&anim_elements[c1],
								&next_anim_elements[c1],
								&r_matrix,
								draw_info->Flags&FLAGS_DRAW_SHADOW,
								dx,dy,dz             // funny bodge 
							);
//			test_draw(start_object+object_offset,AnimOffsetX,AnimOffsetY,100,AnimTween,the_element,the_next_element,&r_matrix);

//			local_object_flags=0;
		}
	}
}

//---------------------------------------------------------------

#define	MAX_POINTS		1560

/*
struct	SVECTOR
{
	SLONG	X,Y,Z;
};
*/

#define MAX_COLOURS 256

ULONG d3d_colour  [MAX_COLOURS];
ULONG d3d_specular[MAX_COLOURS];

void	e_draw_prim_tween(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct GameKeyFrameElement *anim_info,struct GameKeyFrameElement *anim_info_next,struct Matrix33 *rot_mat,ULONG shadow,SWORD off_dx,SWORD off_dy,SWORD off_dz)
{
	float			average_z,
					bright[MAX_POINTS*3],
					r,g,b,
//					shade,
					*rgb;
	ULONG			flag_and,flag_or;
	SLONG           p_index;
	SLONG			az,
					c0,
					dx,dy,dz,
					flags[MAX_POINTS],
					flags_shadow[MAX_POINTS],
					i,j,
					list,
					sp,ep;
	BucketQuad		*the_quad;
	BucketTri		*the_tri;
	D3DTLVERTEX		*the_vertex;
	KeyFrame		*the_keyframe1,
					*the_keyframe2;
	M31				normal;
	Matrix31		offset;
	Matrix33		mat2;
	Matrix33		mat_final;
	PrimFace4		*p_f4;
	PrimFace3		*p_f3;
	PrimObject		*p_obj;
	SVECTOR			temp,
					temp_shadow;
	SVECTOR_F		res_shadow[MAX_POINTS],
					temp_f,
					temp_f_shadow;
	GameCoord		pos;
	SLONG			shade;

	SLONG red;
	SLONG green;
	SLONG blue;


//	LogText(" draw prim tween %d \n",prim);
	p_obj    =	&prim_objects[prim];
	p_f4     =	&prim_faces4[p_obj->StartFace4];
	p_f3     =	&prim_faces3[p_obj->StartFace3];
	
//	mat			=	&anim_info->Matrix;
//	mat_next	=	&anim_info_next->Matrix;

	//move object "tweened quantity"  , z&y flipped

	//LogText(" ANIMATE ***  offset start %d %d %d  ofset end %d %d %d,  BODGE %d %d %d\n",anim_info->OffsetX,anim_info->OffsetY,anim_info->OffsetZ,
	offset.M[0]	=	(anim_info->OffsetX+(((anim_info_next->OffsetX+off_dx-(anim_info->OffsetX))*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[1]	=	(anim_info->OffsetY+(((anim_info_next->OffsetY+off_dy-(anim_info->OffsetY))*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[2]	=	(anim_info->OffsetZ+(((anim_info_next->OffsetZ+off_dz-(anim_info->OffsetZ))*tween)>>8))>>TWEEN_OFFSET_SHIFT;

void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
	x			+=	temp.X;
	y			+=	temp.Y;
	z			+=	temp.Z;	


	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;

	//create a temporary "tween" matrix between current and next
	/*
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			mat2.M[i][j]=mat->M[i][j]+(((mat_next->M[i][j]-mat->M[i][j])*tween)>>8);
		}
	}
	*/
	build_tween_matrix(&mat2,&anim_info->CMatrix,&anim_info_next->CMatrix,tween);
	normalise_matrix(&mat2);

	//apply local rotation matrix
void matrix_mult33(struct Matrix33* result,struct Matrix33* mat1,struct  Matrix33* mat2);
	matrix_mult33(&mat_final,rot_mat,&mat2);

	SET_M31	(
				&normal,
				0,-1,0
			);

void matrix_transform(struct Matrix31* result, struct Matrix33* trans,struct  Matrix31* mat2);
SLONG	calc_shadow_co_ord(struct SVECTOR *input,struct SVECTOR *output,SLONG l_x,SLONG l_y,SLONG l_z);
	for(c0=sp;c0<ep;c0++)
	{
		matrix_transform((struct Matrix31*)&temp,&mat_final, (struct Matrix31*)&prim_points[c0]);

		temp.X		+=	x;
		temp.Y		+=	y;
		temp.Z		+=	z;
		temp_f.X		=	(float)temp.X;
		temp_f.Y		=	(float)temp.Y;
		temp_f.Z		=	(float)temp.Z;

		if(shadow)
		{
			calc_shadow_co_ord(&temp,&temp_shadow,9000*2,4000,8000*2);//light co_ord

			temp_f_shadow.X	=	(float)temp_shadow.X;
			temp_f_shadow.Y	=	(float)temp_shadow.Y;
			temp_f_shadow.Z	=	(float)temp_shadow.Z;

			flags_shadow[c0-sp]	=	transform_point(&temp_f_shadow,&res_shadow[c0-sp]);
		}

		flags[c0-sp]		=	transform_point(&temp_f,&dx_global_res[c0-sp]);

		//
		// Work out the lighting at this point and put it in the colour array.
		//

		p_index = c0 - sp;

		ASSERT(WITHIN(p_index, 0, MAX_COLOURS - 1));

		pos.X = temp.X;
		pos.Y = temp.Y;
		pos.Z = temp.Z;

		LIGHT_get_d3d_colour(LIGHT_get_point(pos.X, pos.Y, pos.Z), &d3d_colour[p_index], &d3d_specular[p_index]);
	}

	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		SLONG	p0,p1,p2,p3;

		if(BUCKETS_FULL)
			goto	exit;

		p0	=	p_f4->Points[0]-sp;
		p1	=	p_f4->Points[1]-sp;
		p2	=	p_f4->Points[2]-sp;
		p3	=	p_f4->Points[3]-sp;

		if(shadow)
		{
			flag_and = flags_shadow[p0]&flags_shadow[p1]&flags_shadow[p2]&flags_shadow[p3];	
			flag_or = flags_shadow[p0]|flags_shadow[p1]|flags_shadow[p2]|flags_shadow[p3];	

			if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
			{
				the_quad			=	GET_BUCKET(BucketQuad);

				the_vertex			=	&vertex_pool[current_vertex];
				the_vertex->sx		=	res_shadow[p0].X;
				the_vertex->sy		=	res_shadow[p0].Y;
				the_vertex->sz		=	res_shadow[p0].Z-0.0005f;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	0;
				the_vertex->specular=	0;

				the_vertex++;
				the_vertex->sx		=	res_shadow[p1].X;
				the_vertex->sy		=	res_shadow[p1].Y;
				the_vertex->sz		=	res_shadow[p1].Z-0.0005f;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB(0.0,0.0,0.0);
				the_vertex->specular=	0;

				the_vertex++;
				the_vertex->sx		=	res_shadow[p2].X;
				the_vertex->sy		=	res_shadow[p2].Y;
				the_vertex->sz		=	res_shadow[p2].Z-0.0005f;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB(0.0,0.0,0.0);
				the_vertex->specular=	0;

				the_vertex++;
				the_vertex->sx		=	res_shadow[p3].X;
				the_vertex->sy		=	res_shadow[p3].Y;
				the_vertex->sz		=	res_shadow[p3].Z-0.0005f;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB(0.0,0.0,0.0);
				the_vertex->specular=	0;

				the_quad->P[0]			=	current_vertex++;
				the_quad->P[1]			=	current_vertex++;
				the_quad->P[2]			=	current_vertex++;
				the_quad->P[3]			=	current_vertex++;

				average_z				=	(
												res_shadow[p0].Z,
												res_shadow[p1].Z,
												res_shadow[p2].Z,
												res_shadow[p3].Z
											)	*	256.0f;

				add_bucket(&the_quad->BucketHeader,BUCKET_QUAD,COLOUR_LIST_1,(SLONG)average_z);
			}
		}

		if( (p_f4->DrawFlags&POLY_FLAG_DOUBLESIDED) || points_clockwise(p0,p1,p2))
		{
			flag_and = flags[p0]&flags[p1]&flags[p2]&flags[p3];	
			flag_or = flags[p0]|flags[p1]|flags[p2]|flags[p3];	

			if( (!(flag_and & EF_CLIPFLAGS))&&((flag_or&EF_BEHIND_YOU)==0))
			{
				the_quad			=	GET_BUCKET(BucketQuad);

				the_vertex			=	&vertex_pool[current_vertex];
				the_vertex->sx		=	dx_global_res[p0].X;
				the_vertex->sy		=	dx_global_res[p0].Y;
				the_vertex->sz		=	dx_global_res[p0].Z;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;

				the_vertex++;
				the_vertex->sx		=	dx_global_res[p1].X;
				the_vertex->sy		=	dx_global_res[p1].Y;
				the_vertex->sz		=	dx_global_res[p1].Z;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;

				the_vertex++;
				the_vertex->sx		=	dx_global_res[p2].X;
				the_vertex->sy		=	dx_global_res[p2].Y;
				the_vertex->sz		=	dx_global_res[p2].Z;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;

				the_vertex++;
				the_vertex->sx		=	dx_global_res[p3].X;
				the_vertex->sy		=	dx_global_res[p3].Y;
				the_vertex->sz		=	dx_global_res[p3].Z;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;

				if(p_f4->DrawFlags&POLY_FLAG_TEXTURED)
				{
					list	=	p_f4->TexturePage;
					vertex_pool[current_vertex+0].tu	=	p_f4->UV[0][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+0].tv	=	p_f4->UV[0][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+1].tu	=	p_f4->UV[1][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+1].tv	=	p_f4->UV[1][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+2].tu	=	p_f4->UV[2][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+2].tv	=	p_f4->UV[2][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+3].tu	=	p_f4->UV[3][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+3].tv	=	p_f4->UV[3][1]*TEXTURE_MUL;

					vertex_pool[current_vertex+0].color = d3d_colour[p0];
					vertex_pool[current_vertex+1].color = d3d_colour[p1];
					vertex_pool[current_vertex+2].color = d3d_colour[p2];
					vertex_pool[current_vertex+3].color = d3d_colour[p3];

					vertex_pool[current_vertex+0].specular = d3d_specular[p0];
					vertex_pool[current_vertex+1].specular = d3d_specular[p1];
					vertex_pool[current_vertex+2].specular = d3d_specular[p2];
					vertex_pool[current_vertex+3].specular = d3d_specular[p3];


				}
				else
				{
					SLONG	r,g,b;
					list	=	COLOUR_LIST_1;

					//
					// The colour of each point.
					//
					r    = ENGINE_palette[p_f4->Col].red;
					g    = ENGINE_palette[p_f4->Col].green;
					b    = ENGINE_palette[p_f4->Col].blue;

//					LogText(" draw fig col %d  rgb %d %d %d \n",p_f4->Col,r,g,b);


		#define	P_BRIGHT_L 32
		#define	P_BRIGHT 0
					//((32<<16)+(32<<8)+32)

					shade = p_f4->Bright[0]+P_BRIGHT_L;// * 256;
					red   = (r*shade)>>8;
					green = (g*shade)>>8;
					blue  = (b*shade)>>8;
					vertex_pool[current_vertex+0].color = (red << 16) | (green << 8) | (blue << 0);

					shade = p_f4->Bright[1]+P_BRIGHT_L;// * 256;
					red   = (r*shade)>>8;
					green = (g*shade)>>8;
					blue  = (b*shade)>>8;
					vertex_pool[current_vertex+1].color = (red << 16) | (green << 8) | (blue << 0);

					shade = p_f4->Bright[2]+P_BRIGHT_L;// * 256;
					red   = (r*shade)>>8;
					green = (g*shade)>>8;
					blue  = (b*shade)>>8;
					vertex_pool[current_vertex+2].color = (red << 16) | (green << 8) | (blue << 0);

					shade = p_f4->Bright[3]+P_BRIGHT_L;// * 256;
					red   = (r*shade)>>8;
					green = (g*shade)>>8;
					blue  = (b*shade)>>8;
					vertex_pool[current_vertex+3].color = (red << 16) | (green << 8) | (blue << 0);

					vertex_pool[current_vertex+0].specular = P_BRIGHT; //d3d_specular[p0];
					vertex_pool[current_vertex+1].specular = P_BRIGHT; //d3d_specular[p1];
					vertex_pool[current_vertex+2].specular = P_BRIGHT; //d3d_specular[p2];
					vertex_pool[current_vertex+3].specular = P_BRIGHT; //d3d_specular[p3];
		/*
					shade = p_f4->Bright[0] * SHADE_MUL * 256;

					red   = (SLONG) (dx_materials[p_f4->Col].R * shade);
					green = (SLONG) (dx_materials[p_f4->Col].G * shade);
					blue  = (SLONG) (dx_materials[p_f4->Col].B * shade);



					red   *= (d3d_colour[p0] >>  0) & 0xff;
					green *= (d3d_colour[p0] >>  8) & 0xff;
					blue  *= (d3d_colour[p0] >> 16) & 0xff;

					red   >>= 8;
					green >>= 8;
					blue  >>= 8;


					vertex_pool[current_vertex+0].color = (red << 16) | (green << 8) | (blue << 0);

					shade = p_f4->Bright[1] * SHADE_MUL * 256;

					red   = (SLONG) (dx_materials[p_f4->Col].R * shade);
					green = (SLONG) (dx_materials[p_f4->Col].G * shade);
					blue  = (SLONG) (dx_materials[p_f4->Col].B * shade);

					red   *= (d3d_colour[p1] >>  0) & 0xff;
					green *= (d3d_colour[p1] >>  8) & 0xff;
					blue  *= (d3d_colour[p1] >> 16) & 0xff;

					red   >>= 8;
					green >>= 8;
					blue  >>= 8;

					vertex_pool[current_vertex+1].color = (red << 16) | (green << 8) | (blue << 0);

					shade = p_f4->Bright[2] * SHADE_MUL * 256;

					red   = (SLONG) (dx_materials[p_f4->Col].R * shade);
					green = (SLONG) (dx_materials[p_f4->Col].G * shade);
					blue  = (SLONG) (dx_materials[p_f4->Col].B * shade);

					red   *= (d3d_colour[p2] >>  0) & 0xff;
					green *= (d3d_colour[p2] >>  8) & 0xff;
					blue  *= (d3d_colour[p2] >> 16) & 0xff;

					red   >>= 8;
					green >>= 8;
					blue  >>= 8;

					vertex_pool[current_vertex+2].color = (red << 16) | (green << 8) | (blue << 0);

					shade = p_f4->Bright[3] * SHADE_MUL * 256;

					red   = (SLONG) (dx_materials[p_f4->Col].R * shade);
					green = (SLONG) (dx_materials[p_f4->Col].G * shade);
					blue  = (SLONG) (dx_materials[p_f4->Col].B * shade);

					red   *= (d3d_colour[p3] >>  0) & 0xff;
					green *= (d3d_colour[p3] >>  8) & 0xff;
					blue  *= (d3d_colour[p3] >> 16) & 0xff;

					red   >>= 8;
					green >>= 8;
					blue  >>= 8;

					vertex_pool[current_vertex+3].color = (red << 16) | (green << 8) | (blue << 0);

					//
					// Don't take face colour into account with the specular...
					//

					vertex_pool[current_vertex+0].specular = d3d_specular[p0];
					vertex_pool[current_vertex+1].specular = d3d_specular[p1];
					vertex_pool[current_vertex+2].specular = d3d_specular[p2];
					vertex_pool[current_vertex+3].specular = d3d_specular[p3];
		*/



					/*

					r	=	dx_materials[p_f4->Col].R;
					g	=	dx_materials[p_f4->Col].G;
					b	=	dx_materials[p_f4->Col].B;
					shade								=	p_f4->Bright[0]*SHADE_MUL;
					vertex_pool[current_vertex+0].color	=	D3DRGB(r*shade,g*shade,b*shade);
					shade								=	p_f4->Bright[1]*SHADE_MUL;
					vertex_pool[current_vertex+1].color	=	D3DRGB(r*shade,g*shade,b*shade);
					shade								=	p_f4->Bright[2]*SHADE_MUL;
					vertex_pool[current_vertex+2].color	=	D3DRGB(r*shade,g*shade,b*shade);
					shade								=	p_f4->Bright[3]*SHADE_MUL;
					vertex_pool[current_vertex+3].color	=	D3DRGB(r*shade,g*shade,b*shade);

					*/
				}

				the_quad->P[0]			=	current_vertex++;
				the_quad->P[1]			=	current_vertex++;
				the_quad->P[2]			=	current_vertex++;
				the_quad->P[3]			=	current_vertex++;

				average_z				=	(
												dx_global_res[p0].Z,
												dx_global_res[p1].Z,
												dx_global_res[p2].Z,
												dx_global_res[p3].Z
											)	*	256.0f;

				add_bucket(&the_quad->BucketHeader,BUCKET_QUAD,list,(SLONG)average_z);
			}
		}
		p_f4++;
	}

	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		if(BUCKETS_FULL)
			goto	exit;

		p0	=	p_f3->Points[0]-sp;
		p1	=	p_f3->Points[1]-sp;
		p2	=	p_f3->Points[2]-sp;

		if(shadow)
		{
			flag_and = flags_shadow[p0]&flags_shadow[p1]&flags_shadow[p2];
			flag_or = flags_shadow[p0]|flags_shadow[p1]|flags_shadow[p2];

			if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
			{
				the_tri			=	GET_BUCKET(BucketTri);

				the_vertex			=	&vertex_pool[current_vertex];
				the_vertex->sx		=	res_shadow[p0].X;
				the_vertex->sy		=	res_shadow[p0].Y;
				the_vertex->sz		=	res_shadow[p0].Z;
//				the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB(0.0,0.0,0.0);
				the_vertex->specular=	0;

				the_vertex++;
				the_vertex->sx		=	res_shadow[p1].X;
				the_vertex->sy		=	res_shadow[p1].Y;
				the_vertex->sz		=	res_shadow[p1].Z;
//				the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB(0.0,0.0,0.0);
				the_vertex->specular=	0;

				the_vertex++;
				the_vertex->sx		=	res_shadow[p2].X;
				the_vertex->sy		=	res_shadow[p2].Y;
				the_vertex->sz		=	res_shadow[p2].Z;
//				the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
				the_vertex->color	=	D3DRGB(0.0,0.0,0.0);
				the_vertex->specular=	0;

				the_tri->P[0]			=	current_vertex++;
				the_tri->P[1]			=	current_vertex++;
				the_tri->P[2]			=	current_vertex++;

				average_z				=	(
												res_shadow[p0].Z,
												res_shadow[p1].Z,
												res_shadow[p2].Z
											)	*	341.33f;

				add_bucket(&the_tri->BucketHeader,BUCKET_TRI,COLOUR_LIST_1,(SLONG)average_z);
			}
		}

		if( (p_f3->DrawFlags&POLY_FLAG_DOUBLESIDED) || points_clockwise(p0,p1,p2))
		{
			flag_and = flags[p0]&flags[p1]&flags[p2];	
			flag_or  = flags[p0]|flags[p1]|flags[p2];	

			if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
			{
				the_tri				=	GET_BUCKET(BucketTri);

				the_vertex			=	&vertex_pool[current_vertex];
				the_vertex->sx		=	dx_global_res[p0].X;
				the_vertex->sy		=	dx_global_res[p0].Y;
				the_vertex->sz		=	dx_global_res[p0].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;

				the_vertex++;
				the_vertex->sx		=	dx_global_res[p1].X;
				the_vertex->sy		=	dx_global_res[p1].Y;
				the_vertex->sz		=	dx_global_res[p1].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;

				the_vertex++;
				the_vertex->sx		=	dx_global_res[p2].X;
				the_vertex->sy		=	dx_global_res[p2].Y;
				the_vertex->sz		=	dx_global_res[p2].Z;
	//			the_vertex->rhw		=	1.0f/the_vertex->sz;
				the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;

				if(p_f3->DrawFlags&POLY_FLAG_TEXTURED)
				{
					list	=	p_f3->TexturePage;
					vertex_pool[current_vertex+0].tu	=	p_f3->UV[0][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+0].tv	=	p_f3->UV[0][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+1].tu	=	p_f3->UV[1][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+1].tv	=	p_f3->UV[1][1]*TEXTURE_MUL;
					vertex_pool[current_vertex+2].tu	=	p_f3->UV[2][0]*TEXTURE_MUL;
					vertex_pool[current_vertex+2].tv	=	p_f3->UV[2][1]*TEXTURE_MUL;

					
					vertex_pool[current_vertex+0].color = d3d_colour[p0];
					vertex_pool[current_vertex+1].color = d3d_colour[p1];
					vertex_pool[current_vertex+2].color = d3d_colour[p2];

					vertex_pool[current_vertex+0].specular = d3d_specular[p0];
					vertex_pool[current_vertex+1].specular = d3d_specular[p1];
					vertex_pool[current_vertex+2].specular = d3d_specular[p2];
				}
				else
				{
					SLONG	r,g,b;
					r    = ENGINE_palette[p_f3->Col].red;
					g    = ENGINE_palette[p_f3->Col].green;
					b    = ENGINE_palette[p_f3->Col].blue;
					list = COLOUR_LIST_1;

//					LogText(" draw fig col %d  rgb %d %d %d \n",p_f3->Col,r,g,b);

					
					//
					// The colour of each point.
					//

					shade = p_f3->Bright[0]+P_BRIGHT_L;// * 256;
					red   = (r*shade)>>8;
					green = (g*shade)>>8;
					blue  = (b*shade)>>8;
					vertex_pool[current_vertex+0].color = (red << 16) | (green << 8) | (blue << 0);

					shade = p_f3->Bright[1]+P_BRIGHT_L;// * 256;
					red   = (r*shade)>>8;
					green = (g*shade)>>8;
					blue  = (b*shade)>>8;
					vertex_pool[current_vertex+1].color = (red << 16) | (green << 8) | (blue << 0);

					shade = p_f3->Bright[2]+P_BRIGHT_L;// * 256;
					red   = (r*shade)>>8;
					green = (g*shade)>>8;
					blue  = (b*shade)>>8;
					vertex_pool[current_vertex+2].color = (red << 16) | (green << 8) | (blue << 0);

					vertex_pool[current_vertex+0].specular = P_BRIGHT; //d3d_specular[p0];
					vertex_pool[current_vertex+1].specular = P_BRIGHT; //d3d_specular[p1];
					vertex_pool[current_vertex+2].specular = P_BRIGHT; //d3d_specular[p2];

	/*
					shade = p_f3->Bright[0] * SHADE_MUL * 256;

					red   = (SLONG) (dx_materials[p_f3->Col].R * shade);
					green = (SLONG) (dx_materials[p_f3->Col].G * shade);
					blue  = (SLONG) (dx_materials[p_f3->Col].B * shade);


					red   *= (d3d_colour[p0] >>  0) & 0xff;
					green *= (d3d_colour[p0] >>  8) & 0xff;
					blue  *= (d3d_colour[p0] >> 16) & 0xff;

					red   >>= 8;
					green >>= 8;
					blue  >>= 8;

					vertex_pool[current_vertex+0].color = (red << 16) | (green << 8) | (blue << 0);

					//
					// The colour of each point.
					//

					shade = p_f3->Bright[0] * SHADE_MUL * 256;

					red   = (SLONG) (dx_materials[p_f3->Col].R * shade);
					green = (SLONG) (dx_materials[p_f3->Col].G * shade);
					blue  = (SLONG) (dx_materials[p_f3->Col].B * shade);

					red   *= (d3d_colour[p0] >>  0) & 0xff;
					green *= (d3d_colour[p0] >>  8) & 0xff;
					blue  *= (d3d_colour[p0] >> 16) & 0xff;

					red   >>= 8;
					green >>= 8;
					blue  >>= 8;

					vertex_pool[current_vertex+1].color = (red << 16) | (green << 8) | (blue << 0);

					//
					// The colour of each point.
					//

					shade = p_f3->Bright[0] * SHADE_MUL * 256;

					red   = (SLONG) (dx_materials[p_f3->Col].R * shade);
					green = (SLONG) (dx_materials[p_f3->Col].G * shade);
					blue  = (SLONG) (dx_materials[p_f3->Col].B * shade);

					red   *= (d3d_colour[p0] >>  0) & 0xff;
					green *= (d3d_colour[p0] >>  8) & 0xff;
					blue  *= (d3d_colour[p0] >> 16) & 0xff;

					red   >>= 8;
					green >>= 8;
					blue  >>= 8;

					vertex_pool[current_vertex+2].color = (red << 16) | (green << 8) | (blue << 0);
					
					//
					// Don't take face colour into account with the specular...
					//

					vertex_pool[current_vertex+0].specular = d3d_specular[p0];
					vertex_pool[current_vertex+1].specular = d3d_specular[p1];
					vertex_pool[current_vertex+2].specular = d3d_specular[p2];
	*/
					
					/*

					r		=	dx_materials[p_f3->Col].R;
					g		=	dx_materials[p_f3->Col].G;
					b		=	dx_materials[p_f3->Col].B;
					shade								=	p_f3->Bright[0]*SHADE_MUL;
					vertex_pool[current_vertex+0].color	=	D3DRGB(r*shade,g*shade,b*shade);
					shade								=	p_f3->Bright[1]*SHADE_MUL;
					vertex_pool[current_vertex+1].color	=	D3DRGB(r*shade,g*shade,b*shade);
					shade								=	p_f3->Bright[2]*SHADE_MUL;
					vertex_pool[current_vertex+2].color	=	D3DRGB(r*shade,g*shade,b*shade);

					*/

				}
				the_tri->P[0]			=	current_vertex++;
				the_tri->P[1]			=	current_vertex++;
				the_tri->P[2]			=	current_vertex++;

				average_z				=	(
												dx_global_res[p0].Z,
												dx_global_res[p1].Z,
												dx_global_res[p2].Z
											)	*	341.33f;

				add_bucket(&the_tri->BucketHeader,BUCKET_TRI,list,(SLONG)average_z);
			}
		}
		p_f3++;
	}	
exit:;
}



void	e_draw_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{

	SVECTOR_F		temp;
	D3DTLVERTEX		*the_vertex;
	BucketLine		*the_line;
	ULONG	f1,f2;

	SLONG flag_and;
	SLONG flag_or;

#ifndef	_DEBUG
//	return;
#endif

	temp.X = (float) x1;
	temp.Y = (float) y1;
	temp.Z = (float) z1;

	f1	= transform_point(&temp, &dx_global_res[0]);

	temp.X = (float) x2;
	temp.Y = (float) y2;
	temp.Z = (float) z2;

	f2	= transform_point(&temp, &dx_global_res[1]);

	flag_and = f1 & f2;
	flag_or  = f1 | f2;

	if ((flag_or  & EF_BEHIND_YOU) || (flag_and & EF_CLIPFLAGS))
	{
		
//		LogText(" not drawn f1 %x f2 %x or %x and %x \n",f1,f2,flag_or,flag_and);
		//
		// Don't draw the line.
		// 
	}
	else
	{
		the_line		=	GET_BUCKET(BucketLine);

		the_vertex			=	&vertex_pool[current_vertex];
		the_vertex->sx		=	dx_global_res[0].X;
		the_vertex->sy		=	dx_global_res[0].Y;
		the_vertex->sz		=	SCALE_SZ; //dx_global_res[0].Z;
		the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
		the_vertex->color	=	D3DRGB	(
											1.0f,
											1.0f,
											1.0f
										);
		the_vertex++;
		the_vertex->sx		=	dx_global_res[1].X;
		the_vertex->sy		=	dx_global_res[1].Y;
		the_vertex->sz		=	SCALE_SZ; //dx_global_res[1].Z;
		the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
		the_vertex->color	=	D3DRGB	(
											1.0f,
											1.0f,
											1.0f
										);



		the_line->P[0]	=	current_vertex++;
		the_line->P[1]	=	current_vertex++;

		add_bucket(&the_line->BucketHeader,BUCKET_LINE,COLOUR_LIST_1,0);
	}

}


void	e_draw_3d_line_dir(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
	SLONG	dist;
	SLONG	nx,ny,nz;
	SLONG	vx,vy,vz;
	SLONG   c0;

#ifndef	_DEBUG
	return;
#endif
	vx=x2-x1;
	vy=y2-y1;
	vz=z2-z1;
	nx=vz;
	nz=-vx;
	dist=(SLONG)sqrt((float) (nx*nx+nz*nz));

	if(dist==0)
	{
		return;
	}

	nx=(nx*80)/dist;
	nz=(nz*80)/dist;

	vx=(vx*20)/dist;
	vy=(vy*20)/dist;
	vz=(vz*20)/dist;

	for(c0=1;c0<(dist/20);c0+=8)
	{
		e_draw_3d_line(x1+vx*c0,y1+vy*c0,z1+vz*c0,x1+vx*(c0-1)+nx,y1+vy*c0,z1+vz*(c0-1)+nz);
		e_draw_3d_line(x1+vx*c0,y1+vy*c0,z1+vz*c0,x1+vx*(c0-1)-nx,y1+vy*c0,z1+vz*(c0-1)-nz);
	}

	e_draw_3d_line(x1, y1, z1, x2, y2, z2);
}



void	e_draw_3d_line_col_sorted(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b)
{

	SVECTOR_F		temp;
	D3DTLVERTEX		*the_vertex;
	BucketLine		*the_line;
	SLONG	flag_and,flag_or;

	ULONG colour;

#ifndef	_DEBUG
	return;
#endif
	temp.X	=	(float)x1;
	temp.Y	=	(float)y1;
	temp.Z	=	(float)z1;



	global_flags[0]	=	transform_point(&temp,&dx_global_res[0]);
	temp.X	=	(float)x2;
	temp.Y	=	(float)y2;
	temp.Z	=	(float)z2;
	global_flags[1]	=	transform_point(&temp,&dx_global_res[1]);



	flag_and	=	global_flags[0]&global_flags[1];
	flag_or		=	global_flags[0]|global_flags[1];

	if ((flag_or  & EF_BEHIND_YOU) ||
		(flag_and & EF_CLIPFLAGS))
	{
		//
		// Don't draw the line.
		// 
	}
	else
	{

		colour = (r << 0) | (g << 8) | (b << 16);

		the_line		=	GET_BUCKET(BucketLine);

		the_vertex			=	&vertex_pool[current_vertex];
		the_vertex->sx		=	dx_global_res[0].X;
		the_vertex->sy		=	dx_global_res[0].Y;
		the_vertex->sz		=	dx_global_res[0].Z;
		the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
		the_vertex->color	=	colour;

		the_vertex++;
		the_vertex->sx		=	dx_global_res[1].X;
		the_vertex->sy		=	dx_global_res[1].Y;
		the_vertex->sz		=	dx_global_res[1].Z;
		the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
		the_vertex->color	=	colour;

		the_line->P[0]	=	current_vertex++;
		the_line->P[1]	=	current_vertex++;

		add_bucket(&the_line->BucketHeader,BUCKET_LINE,COLOUR_LIST_1,0);
	}
}


void	e_draw_3d_line_col(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b)
{

	SVECTOR_F		temp;
	D3DTLVERTEX		*the_vertex;
	BucketLine		*the_line;
	SLONG	flag_and,flag_or;

#ifndef	_DEBUG
	return;
#endif
		temp.X	=	(float)x1;
		temp.Y	=	(float)y1;
		temp.Z	=	(float)z1;



		global_flags[0]	=	transform_point(&temp,&dx_global_res[0]);
		temp.X	=	(float)x2;
		temp.Y	=	(float)y2;
		temp.Z	=	(float)z2;
		global_flags[1]	=	transform_point(&temp,&dx_global_res[1]);



		flag_and	=	global_flags[0]&global_flags[1];
		flag_or		=	global_flags[0]|global_flags[1];

		if ((flag_or  & EF_BEHIND_YOU) ||
			(flag_and & EF_CLIPFLAGS))
		{
			//
			// Don't draw the line.
			// 
		}
		else
		{

			the_line		=	GET_BUCKET(BucketLine);

			the_vertex			=	&vertex_pool[current_vertex];
			the_vertex->sx		=	dx_global_res[0].X;
			the_vertex->sy		=	dx_global_res[0].Y;
			the_vertex->sz		=	SCALE_SZ; //dx_global_res[0].Z;
			the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
			the_vertex->color	=	D3DRGB	(
												((float)r)/256.0,
												((float)g)/256.0,
												((float)b)/256.0,
											);
			the_vertex++;
			the_vertex->sx		=	dx_global_res[1].X;
			the_vertex->sy		=	dx_global_res[1].Y;
			the_vertex->sz		=	SCALE_SZ; //dx_global_res[1].Z;
			the_vertex->rhw		=	SCALE_SZ/the_vertex->sz;
			the_vertex->color	=	D3DRGB	(
												((float)r)/256.0,
												((float)g)/256.0,
												((float)b)/256.0,
											);



			the_line->P[0]	=	current_vertex++;
			the_line->P[1]	=	current_vertex++;

			add_bucket(&the_line->BucketHeader,BUCKET_LINE,COLOUR_LIST_1,0);
		}

}

void	e_draw_3d_mapwho(SLONG x1,SLONG z1)
{
	x1<<=ELE_SHIFT;
	z1<<=ELE_SHIFT;

#ifdef NDEBUG
	return;
#endif
	e_draw_3d_line(x1,0,z1,x1+256,0,z1);
	e_draw_3d_line(x1+256,0,z1,x1+256,0,z1+256);
	e_draw_3d_line(x1+256,0,z1+256,x1,0,z1+256);
	e_draw_3d_line(x1,0,z1+256,x1,0,z1);


}


void	e_draw_3d_mapwho_y(SLONG x1,SLONG y1,SLONG z1)
{
	x1<<=ELE_SHIFT;
	z1<<=ELE_SHIFT;

#ifdef NDEBUG
	return;
#endif
	e_draw_3d_line(x1,y1,z1,x1+256,y1,z1);
	e_draw_3d_line(x1+256,y1,z1,x1+256,y1,z1+256);
	e_draw_3d_line(x1+256,y1,z1+256,x1,y1,z1+256);
	e_draw_3d_line(x1,y1,z1+256,x1,y1,z1);


}

void	e_draw_actual_col_vect(UWORD	col_vect)
{
	struct	SVECTOR	points[2];
	struct	SVECTOR	res[2];
	SLONG	flags[2];

#ifdef NDEBUG
	return;
#endif
		points[0].X=col_vects[ col_vect].X[0];
		points[0].Y=col_vects[ col_vect].Y[0];
		points[0].Z=col_vects[ col_vect].Z[0];

		points[1].X=col_vects[ col_vect].X[1];
		points[1].Y=col_vects[ col_vect].Y[1];
		points[1].Z=col_vects[ col_vect].Z[1];

//		LogText(" DRAW COL VECT %d   (%d,%d,%d)--(%d,%d,%d)\n",points[0].X,points[0].Y,points[0].Z,points[1].X,points[1].Y,points[1].Z);
		e_draw_3d_line_dir(points[0].X,points[0].Y,points[0].Z,points[1].X,points[1].Y,points[1].Z);

}
void	e_draw_col_vects(UWORD	col_vect_link)
{
#ifdef NDEBUG
	return;
#endif
	if (!Keys[KB_8])
	{
		return;
	}

	while(col_vect_link)
	{
		e_draw_actual_col_vect(col_vects_links[col_vect_link].VectIndex);

		col_vect_link=col_vects_links[col_vect_link].Next;
	}

}



//---------------------------------------------------------------
