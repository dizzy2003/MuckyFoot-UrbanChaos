// EdUtils.cpp
// Guy Simmons, 5th April 1997

#include	"Editor.hpp"
#include	"animtmap.h"
#include	"memory.h"


extern	SLONG			key_frame_count,current_element;

extern SLONG					x_centre,
								y_centre,
								z_centre;
extern struct KeyFrameChunk 	*test_chunk;


//---------------------------------------------------------------

//---------------------------------------------------------------


//---------------------------------------------------------------
/*
void	invert_mult(struct Matrix33 *mat,struct PrimPoint *pp)
{
	Matrix33	temp_mat;
	SLONG	i,j;
	SLONG	x,y,z;

	for(i=0;i<3;i++)
	for(j=0;j<3;j++)
	{
		temp_mat.M[i][j]=mat->M[j][i];
	}

//	LogText(" len before %d \n",SDIST3(pp->X,pp->Y,pp->Z));

	x = (pp->X * temp_mat.M[0][0])+(pp->Y * temp_mat.M[0][1])+(pp->Z * temp_mat.M[0][2])>>15; 
	y = (pp->X * temp_mat.M[1][0])+(pp->Y * temp_mat.M[1][1])+(pp->Z * temp_mat.M[1][2])>>15; 
	z = (pp->X * temp_mat.M[2][0])+(pp->Y * temp_mat.M[2][1])+(pp->Z * temp_mat.M[2][2])>>15;
//	LogText(" len after %d \n",SDIST3(x,y,z));

	pp->X=x;
	pp->Y=y;
	pp->Z=z;
}
*/
//---------------------------------------------------------------


//---------------------------------------------------------------

extern void	do_quad_clip_list(SWORD face,SLONG p0,SLONG p1,SLONG p2,SLONG p3);
extern void	do_tri_clip_list(SWORD face,SLONG p0,SLONG p1,SLONG p2);
extern void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
extern void matrix_transform(struct Matrix31* result, struct Matrix33* trans,struct  Matrix31* mat2);
extern void matrix_transform_small(struct Matrix31* result, struct Matrix33* trans,struct  SMatrix31* mat2);

#define	CLIP256(x)		(x>255?255:x)
extern	struct	SVector			global_res[]; //max points per object?
extern	SLONG					global_flags[];

extern	UWORD	is_it_clockwise(struct SVector *res,SLONG p1,SLONG p2,SLONG p3);

void	draw_element(UWORD	prim,SLONG x,SLONG y,SLONG z,struct KeyFrameElement *anim_info)
{
	UWORD				bright[1560];
	ULONG				flag_and,flag_or;
	SLONG				az,
						c0,
						sp,ep;
	struct Matrix31		offset;
	struct Matrix33		local_matrix,
						*mat,
						mat_final;
	struct PrimFace4	*p_f4;
	struct PrimFace3	*p_f3;
	struct SVector		temp; //max points per object?
	struct PrimObject	*p_obj;

	static	SLONG	scale=256;

	if(Keys[KB_LBRACE])
		scale+=5;

	if(Keys[KB_RBRACE])
		scale-=5;

	if(scale<256)
		scale=256;



	p_obj		=	&prim_objects[prim];
	p_f4		=	&prim_faces4[p_obj->StartFace4];
	p_f3		=	&prim_faces3[p_obj->StartFace3];

	rotate_obj	(
					0, //engine.AngleX,
					0, //engine.AngleY,
					0, //engine.AngleZ,
					&local_matrix
				);

	mat      	=	&anim_info->Matrix;

	offset.M[0]	=	(anim_info->OffsetX*scale)>>(TWEEN_OFFSET_SHIFT+8);
	offset.M[1]	=	(anim_info->OffsetY*scale)>>(TWEEN_OFFSET_SHIFT+8);
	offset.M[2]	=	(anim_info->OffsetZ*scale)>>(TWEEN_OFFSET_SHIFT+8);
	matrix_transformZMY((struct Matrix31*)&temp,&local_matrix,&offset);
	x	+=	temp.X;
	y	+=	temp.Y;
	z	+=	temp.Z;

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;


	engine.X	-=	x<<8;
	engine.Y	-=	y<<8;
	engine.Z	-=	z<<8;

//apply local rotation matrix
	matrix_mult33(&mat_final,&local_matrix,mat);
		
	for(c0=sp;c0<ep;c0++)
	{
		matrix_transform_small((struct Matrix31*)&temp,&mat_final, (struct SMatrix31*)&prim_points[c0]);
		global_flags[c0-sp]	=	rotate_point_gte((struct SVector*)&temp,&global_res[c0-sp]);
		bright[c0-sp]	=	128; //calc_lights(x,y,z,(struct SVector*)&prim_points[c0]);
	}

	engine.X	+=	x<<8;
	engine.Y	+=	y<<8;
	engine.Z	+=	z<<8;
		
	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		SLONG	p0,p1,p2,p3;

		if(current_bucket_pool>=end_bucket_pool)
			goto	exit;

		p0=p_f4->Points[0]-sp;
		p1=p_f4->Points[1]-sp;
		p2=p_f4->Points[2]-sp;
		p3=p_f4->Points[3]-sp;

		flag_and	=	global_flags[p0]&global_flags[p1]&global_flags[p2]&global_flags[p3];	
		flag_or		=	global_flags[p0]|global_flags[p1]|global_flags[p2]|global_flags[p3];	

		{
			
			if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
			{			
				az	=	(global_res[p0].Z+global_res[p1].Z+global_res[p2].Z+global_res[p3].Z)>>2;

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

				if(SelectFlag)
				if(SelectDrawn==0||is_it_clockwise(global_res,p0,p1,p2))
					do_quad_clip_list(c0,p0,p1,p2,p3);

				setUV4	(
							(struct BucketQuad*)current_bucket_pool,
							p_f4->UV[0][0],p_f4->UV[0][1],
							p_f4->UV[1][0],p_f4->UV[1][1],
							p_f4->UV[2][0],p_f4->UV[2][1],
							p_f4->UV[3][0],p_f4->UV[3][1],
							p_f4->TexturePage
						);

				setZ4	(
							(struct BucketQuad*)current_bucket_pool,
							-global_res[p0].Z,
							-global_res[p1].Z,
							-global_res[p2].Z,
							-global_res[p3].Z
						);

				setShade4	(
								(struct BucketQuad*)current_bucket_pool,
								CLIP256(p_f4->Bright[0]+bright[p0]),
								CLIP256(p_f4->Bright[1]+bright[p1]),
								CLIP256(p_f4->Bright[2]+bright[p2]),
								CLIP256(p_f4->Bright[3]+bright[p3])
							);
				((struct BucketQuad*)current_bucket_pool)->DebugInfo=c0;
				((struct BucketQuad*)current_bucket_pool)->DebugFlags=p_f4->FaceFlags;

				add_bucket((void *)current_bucket_pool,az);

				if(check_mouse_over_prim_quad(global_res,p0,p1,p2,p3,c0))
				{
					selected_prim_xyz.X	=	x;
					selected_prim_xyz.Y	=	y;
					selected_prim_xyz.Z	=	z;
				}

				current_bucket_pool	+=	sizeof(struct BucketQuad);
			}
		}
		p_f4++;
	}

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
			az	=	(global_res[p0].Z+global_res[p1].Z+global_res[p2].Z)/3;

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

			if(SelectFlag)
			if(SelectDrawn==0||is_it_clockwise(global_res,p0,p1,p2))
				do_tri_clip_list(-c0,p0,p1,p2);

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
			((struct BucketTri*)current_bucket_pool)->DebugFlags=p_f3->FaceFlags;

			add_bucket((void *)current_bucket_pool,az);

			if(check_mouse_over_prim_tri(global_res,p0,p1,p2,c0))
			{
				selected_prim_xyz.X	=	x;
				selected_prim_xyz.Y	=	y;
				selected_prim_xyz.Z	=	z;
			}

			current_bucket_pool	+=	sizeof(struct BucketQuad);
		}
		p_f3++;
	}					
exit:;
}

//---------------------------------------------------------------

//void	draw_multi_prim(EdRect *bounds_rect,struct KeyFrame *the_frame,struct Matrix33 *r_matrix)
void	draw_a_key_frame_at(UWORD prim,SLONG x,SLONG y,SLONG z)
{
	SLONG				c0,c1;
	KeyFrame			*the_frame	=	&test_chunk->KeyFrames[0];


	for(c1=0,c0=prim_multi_objects[prim].StartObject;c0<prim_multi_objects[prim].EndObject;c0++,c1++)
	{
		draw_element(
						c0,
						x,
						y,
						z,
						&the_frame->FirstElement[c1]
					);
	}
}

//---------------------------------------------------------------


void	clear_anim_stuff(void)
{
	test_chunk->MultiObject=0;
	test_chunk->ElementCount=0;

}

//---------------------------------------------------------------


void	load_chunk_texture_info_old(KeyFrameChunk *the_chunk)
{
	CBYTE				file_name[64];
	SLONG				c0	=	0,
						c1,c2;
	MFFileHandle		file_handle;
	struct PrimFace4	*p_f4;
	struct PrimFace3	*p_f3;
	struct PrimObject	*p_obj;


	strcpy(file_name,the_chunk->VUEName);
	while(file_name[c0]!='.' && file_name[c0]!=0)c0++;
	if(file_name[c0]=='.')
	{
		file_name[c0+1]	=	'T';
		file_name[c0+2]	=	'E';
		file_name[c0+3]	=	'X';
		file_name[c0+4]	=	0;
	}

	file_handle	=	FileOpen(file_name);
	if(file_handle!=FILE_OPEN_ERROR)
	{
		LogText(" read anims texture old %s \n",file_name);
		for(c0=prim_multi_objects[the_chunk->MultiObject].StartObject;c0<prim_multi_objects[the_chunk->MultiObject].EndObject;c0++,c1++)
		{
			p_obj		=	&prim_objects[c0];
			p_f4		=	&prim_faces4[p_obj->StartFace4];
			p_f3		=	&prim_faces3[p_obj->StartFace3];

			
//			LogText(" obj %d has f4 %d f3 %d \n",c0,-p_obj->StartFace4+p_obj->EndFace4,-p_obj->StartFace3+p_obj->EndFace3);

			for(c1=p_obj->StartFace4;c1<p_obj->EndFace4;c1++,p_f4++)
			{
				FileRead(file_handle,&p_f4->DrawFlags,sizeof(p_f4->DrawFlags));
				FileRead(file_handle,&p_f4->Col2,sizeof(p_f4->Col2));
				FileRead(file_handle,&p_f4->TexturePage,sizeof(p_f4->TexturePage));
//				if(p_f4->TexturePage>5)
//					p_f4->TexturePage	=	0;

				for(c2=0;c2<4;c2++)
				{
					FileRead(file_handle,&p_f4->UV[c2][0],sizeof(p_f4->UV[c2][0]));
					FileRead(file_handle,&p_f4->UV[c2][1],sizeof(p_f4->UV[c2][1]));
//					LogText("Q uv %d,%d \n",p_f4->UV[c2][0],p_f4->UV[c2][1]);
				}
			}

			for(c1=p_obj->StartFace3;c1<p_obj->EndFace3;c1++,p_f3++)
			{
				FileRead(file_handle,&p_f3->DrawFlags,sizeof(p_f3->DrawFlags));
				FileRead(file_handle,&p_f3->Col2,sizeof(p_f3->Col2));
				FileRead(file_handle,&p_f3->TexturePage,sizeof(p_f3->TexturePage));
//				if(p_f3->TexturePage>5)
//					p_f3->TexturePage	=	0;

				for(c2=0;c2<3;c2++)
				{
					FileRead(file_handle,&p_f3->UV[c2][0],sizeof(p_f3->UV[c2][0]));
					FileRead(file_handle,&p_f3->UV[c2][1],sizeof(p_f3->UV[c2][1]));
//					LogText("T uv %d,%d \n",p_f4->UV[c2][0],p_f4->UV[c2][1]);
				}
			}
		}
		FileClose(file_handle);
	}
}

void	load_chunk_texture_info(KeyFrameChunk *the_chunk)
{
	CBYTE				file_name[64];
	SLONG				c0	=	0,
						c1,c2;
	MFFileHandle		file_handle;
	struct PrimFace4	*p_f4;
	struct PrimFace3	*p_f3;
	struct PrimObject	*p_obj;
	SLONG	save_type;
	SLONG	multi,count=0;
	SLONG   sizeof_face_data;


	strcpy(file_name,the_chunk->VUEName);
	while(file_name[c0]!='.' && file_name[c0]!=0)c0++;
	if(file_name[c0]=='.')
	{
		file_name[c0+1]	=	'T';
		file_name[c0+2]	=	'E';
		file_name[c0+3]	=	'X';
		file_name[c0+4]	=	0;
	}

	for(multi=the_chunk->MultiObjectStart;multi<=the_chunk->MultiObjectEnd;multi++)
	{
		if(count>0)
		{
			file_name[5]='1'+count-1;
		}
		count++;
		file_handle	=	FileOpen(file_name);
		if(file_handle!=FILE_OPEN_ERROR)
		{
			FileRead(file_handle,&save_type,sizeof(save_type));

			{
				for(c0=prim_multi_objects[multi].StartObject;c0<prim_multi_objects[multi].EndObject;c0++,c1++)
				{
					SWORD	count;
					p_obj		=	&prim_objects[c0];
					p_f4		=	&prim_faces4[p_obj->StartFace4];
					p_f3		=	&prim_faces3[p_obj->StartFace3];
					
					LogText("LCTI obj %d has f4 %d f3 %d \n",c0,-p_obj->StartFace4+p_obj->EndFace4,-p_obj->StartFace3+p_obj->EndFace3);

					FileRead(file_handle,&count,sizeof(count));

					for(c1=p_obj->StartFace4;c1<p_obj->EndFace4&&count>0;c1++,p_f4++,count--)
					{
						FileRead(file_handle,&p_f4->DrawFlags,sizeof(p_f4->DrawFlags));
						FileRead(file_handle,&p_f4->Col2,sizeof(p_f4->Col2));
						FileRead(file_handle,&p_f4->TexturePage,sizeof(p_f4->TexturePage));

						if (save_type > 0)
						{
							//
							// After save type zero, we save the FaceFlags here.
							//

							FileRead(file_handle, &p_f4->FaceFlags, sizeof(p_f4->FaceFlags));
						}

						for(c2=0;c2<4;c2++)
						{
							FileRead(file_handle,&p_f4->UV[c2][0],sizeof(p_f4->UV[c2][0]));
							FileRead(file_handle,&p_f4->UV[c2][1],sizeof(p_f4->UV[c2][1]));
						}
					}

					//
					// The size of each face's data.
					// 

					sizeof_face_data  = sizeof(p_f4->DrawFlags);
					sizeof_face_data += sizeof(p_f4->Col2);
					sizeof_face_data += sizeof(p_f4->TexturePage);
					sizeof_face_data += sizeof(p_f4->UV[0][0]) * 8;

					if (save_type > 0)
					{
						sizeof_face_data += sizeof(p_f4->FaceFlags);
					}

					while(count)
					{
						ULONG	poo[50];
						FileRead(file_handle,&poo[0],sizeof_face_data);
						count--;
					}

					FileRead(file_handle,&count,sizeof(count));
					for(c1=p_obj->StartFace3;c1<p_obj->EndFace3&&count>0;c1++,p_f3++,count--)
					{
						FileRead(file_handle,&p_f3->DrawFlags,sizeof(p_f3->DrawFlags));
						FileRead(file_handle,&p_f3->Col2,sizeof(p_f3->Col2));
						FileRead(file_handle,&p_f3->TexturePage,sizeof(p_f3->TexturePage));

						if (save_type > 0)
						{
							//
							// After save type zero, we save the FaceFlags here.
							//

							FileRead(file_handle, &p_f3->FaceFlags, sizeof(p_f3->FaceFlags));
						}

						for(c2=0;c2<3;c2++)
						{
							FileRead(file_handle,&p_f3->UV[c2][0],sizeof(p_f3->UV[c2][0]));
							FileRead(file_handle,&p_f3->UV[c2][1],sizeof(p_f3->UV[c2][1]));
						}
					}

					//
					// The size of each face's data.
					// 

					sizeof_face_data  = sizeof(p_f3->DrawFlags);
					sizeof_face_data += sizeof(p_f3->Col2);
					sizeof_face_data += sizeof(p_f3->TexturePage);
					sizeof_face_data += sizeof(p_f3->UV[0][0]) * 6;

					if (save_type > 0)
					{
						sizeof_face_data += sizeof(p_f3->FaceFlags);
					}

					while(count)
					{
						ULONG	poo[50];
						FileRead(file_handle,&poo[0],sizeof_face_data);
						count--;
					}

				}
			}
			FileClose(file_handle);

			/*
			if(save_type)
			{
				load_chunk_texture_info_old(the_chunk);
			}
			*/
		}
	}
}


//---------------------------------------------------------------

void do_single_shot(UBYTE *screen,UBYTE *palette)
{
	CBYTE			f_name[128];
	SLONG			c0	=	0;


	do
	{
		sprintf(f_name,"SHOTS\\SCR%04d.PCX",c0);
		c0++;
	}while(FileExists(f_name));

	write_pcx(f_name,screen,palette);
}

//---------------------------------------------------------------

SLONG	count	=	0;

void do_record_frame(UBYTE *screen,UBYTE *palette)
{
	CBYTE			f_name[128];
/*
	SLONG			c0	=	0;


	do
	{
		sprintf(f_name,"FRAMES\\FRA%04d.PCX",c0);
		c0++;
	}while(FileExists(f_name));
*/
	sprintf(f_name,"FRAMES\\FRA%04d.PCX",count++);
	write_pcx(f_name,screen,palette);
}

//---------------------------------------------------------------

struct PCXHeader
{
	UBYTE				Manufacturer;
	UBYTE				Version;
	UBYTE				Encoding;
	UBYTE				BitsPerPixel;
	UWORD				X;
	UWORD				Y;
	UWORD				Width;
	UWORD				Height;
	UWORD				HorizRes;
	UWORD				VertRes;
	UBYTE				EGAPalette[48];
	UBYTE				Reserved;
	UBYTE				NumColorPlanes;
	UWORD				BytesPerLine;
	UWORD				PaletteType;
	UBYTE				Padding[58];
};

SLONG	write_pcx(CBYTE *fname,UBYTE *src,UBYTE *pal)
{
	UBYTE					pixel,
							palette[769],
							*ptr,buf[1024];
	SLONG					c0,
							count,
							x,y;
	MFFileHandle			f_handle;
	PCXHeader				the_header;


	f_handle	=	FileCreate(fname,TRUE);
	if(f_handle!=FILE_CREATION_ERROR)
	{
		// Create & write out header.
		memset((UBYTE*)&the_header,0,sizeof(the_header));
		the_header.Manufacturer		=	10;
		the_header.Version			=	5;
		the_header.Encoding			=	1;
		the_header.BitsPerPixel		=	8;
		the_header.X 				=	0;
		the_header.Y				= 	0;
		the_header.Width			=	WorkScreenPixelWidth-1;
		the_header.Height			=	WorkScreenHeight-1;
		the_header.HorizRes			=	WorkScreenPixelWidth;
		the_header.VertRes			=	WorkScreenHeight;
		the_header.NumColorPlanes	=	1;
		the_header.BytesPerLine		=	WorkScreenPixelWidth;
		FileWrite(f_handle,(UBYTE*)&the_header,sizeof(the_header));

		// Compress image & write the sucker out.
		for(y=0;y<WorkScreenHeight;y++,src+=WorkScreenPixelWidth)
		{
			ptr = src;
			x = 0;
			c0 = 0;
			while(x<WorkScreenPixelWidth)
			{
				if(x+1<WorkScreenPixelWidth && *ptr==*(ptr+1))
				{
					count=1;
					x++;
					while(count<63 && (x+1)<WorkScreenPixelWidth && *ptr==*(ptr+1))
					{
						x++;
						ptr++;
						count++;
					}
					buf[c0++] = count+192;
					pixel	=	*(ptr++);
					buf[c0++] = pixel;
				}
				else
				{
					if(*ptr < 192)
					{
						buf[c0++] = *(ptr++);
					}
					else
					{
						buf[c0++] = 193;
						pixel	=	*(ptr++);
						buf[c0++] = pixel;
					}
					x++;
				}
			}
			FileWrite(f_handle,(UBYTE*)buf,c0);
		}

		// Sort out the palette & write it.
		palette[0]	=	0x0c;
		for(c0=0;c0<256;c0++)
		{
			palette[(c0*3)+1] = *(pal++);
			palette[(c0*3)+2] = *(pal++);
			palette[(c0*3)+3] = *(pal++);
		}
		FileWrite(f_handle,&palette[0],769);
		FileClose(f_handle);	
		return 0;
	}
	else
	{
		return -1;
	}
}

//---------------------------------------------------------------

#undef	ShowWorkScreen

void	editor_show_work_screen(ULONG flags)
{
	UBYTE		temp_bit[32][32],
				*cursor_ptr;
	SLONG		c0,c1,
				mouse_x,
				mouse_y;


	ShowWorkScreen(flags);
	if(editor_status&EDITOR_RECORD)
	{
		SLONG		x,y,w,h;


		x	=	WorkWindowRect.Left;
		y	=	WorkWindowRect.Top;
		w	=	WorkWindowRect.Width;
		h	=	WorkWindowRect.Height;
		SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);

		if(LockWorkScreen())
		{
			// Backup cursor background.
			mouse_x		=	MouseX;
			mouse_y		=	MouseY;
			GlobalXYToLocal(&mouse_x,&mouse_y);
			cursor_ptr	=	WorkWindow+mouse_x+(mouse_y*WorkScreenWidth);
			for(c0=0;c0<32;c0++)
			{
				for(c1=0;c1<32;c1++)
				{
					temp_bit[c0][c1]	=	*(cursor_ptr+c1+(c0*WorkScreenWidth));
				}
			}

			// Draw a cursor sprite.
			DrawBSpriteC(mouse_x,mouse_y,INTERFACE_SPRITE(23));

			// Save the frame.
			do_record_frame(WorkScreen,CurrentPalette);

			// Restore cursor background.
			for(c0=0;c0<32;c0++)
			{
				for(c1=0;c1<32;c1++)
				{
					*(cursor_ptr+c1+(c0*WorkScreenWidth))	=	temp_bit[c0][c1];
				}
			}
			UnlockWorkScreen();
		}
		SetWorkWindowBounds(x,y,w,h);
	}
}

//---------------------------------------------------------------

#undef	ShowWorkWindow

void	editor_show_work_window(ULONG flags)
{
	UBYTE		temp_bit[32][32],
				*cursor_ptr;
	SLONG		c0,c1,
				mouse_x,
				mouse_y;


	ShowWorkWindow(flags);
	if(editor_status&EDITOR_RECORD)
	{
		SLONG		x,y,w,h;


		x	=	WorkWindowRect.Left;
		y	=	WorkWindowRect.Top;
		w	=	WorkWindowRect.Width;
		h	=	WorkWindowRect.Height;
		SetWorkWindowBounds(0,0,WorkScreenPixelWidth,WorkScreenHeight);

		if(LockWorkScreen())
		{
			// Backup cursor background.
			mouse_x		=	MouseX;
			mouse_y		=	MouseY;
			GlobalXYToLocal(&mouse_x,&mouse_y);
			cursor_ptr	=	WorkWindow+mouse_x+(mouse_y*WorkScreenWidth);
			for(c0=0;c0<32;c0++)
			{
				for(c1=0;c1<32;c1++)
				{
					temp_bit[c0][c1]	=	*(cursor_ptr+c1+(c0*WorkScreenWidth));
				}
			}
			// Draw a cursor sprite.
			DrawBSpriteC(mouse_x,mouse_y,INTERFACE_SPRITE(23));

			// Save the frame.
			do_record_frame(WorkScreen,CurrentPalette);

			// Restore cursor background.
			for(c0=0;c0<32;c0++)
			{
				for(c1=0;c1<32;c1++)
				{
					*(cursor_ptr+c1+(c0*WorkScreenWidth))	=	temp_bit[c0][c1];
				}
			}
			UnlockWorkScreen();
		}

		SetWorkWindowBounds(x,y,w,h);
	}
}

//---------------------------------------------------------------
/*
void	test_draw(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,struct Matrix33 *rot_mat)
{
	ULONG					flag_and,flag_or;
	SLONG					az,
							c0,
							ep,
							flags[1560],

							i,j,
							sp,
							tx,ty,tz;
	struct	Matrix31		offset;
	struct	Matrix33		mat2,
							mat_final,
							*mat,
							*mat_next;
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	struct	PrimObject		*p_obj;
	struct	SVector			res[1560],temp; //max points per object?


	p_obj    =	&prim_objects[prim];
	p_f4     =	&prim_faces4[p_obj->StartFace4];
	p_f3     =	&prim_faces3[p_obj->StartFace3];
	

	mat      =	&anim_info->Matrix;
	mat_next =	&anim_info_next->Matrix;

	//move object "tweened quantity"  , z&y flipped
	offset.M[0]	=	(anim_info->OffsetX+(((anim_info_next->OffsetX-anim_info->OffsetX)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[1]	=	(anim_info->OffsetY+(((anim_info_next->OffsetY-anim_info->OffsetY)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[2]	=	(anim_info->OffsetZ+(((anim_info_next->OffsetZ-anim_info->OffsetZ)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
	x	+= temp.X;
	y	+= temp.Y;
	z	+= temp.Z;

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;

	tx=engine.X;
	ty=engine.Y;
	tz=engine.Z;

	engine.X=-x<<8;
	engine.Y=-y<<8;
	engine.Z=-z<<8;


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
		matrix_transform((struct Matrix31*)&temp,&mat_final, (struct Matrix31*)&prim_points[c0]);
		
		flags[c0-sp]	=	rotate_point_gte((struct SVector*)&temp,&res[c0-sp]);
	}

	engine.X=tx;
	engine.Y=ty;
	engine.Z=tz;

	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		SLONG	p0,p1,p2,p3;

		p0=p_f4->Points[0]-sp;
		p1=p_f4->Points[1]-sp;
		p2=p_f4->Points[2]-sp;
		p3=p_f4->Points[3]-sp;

		flag_and	=	flags[p0]&flags[p1]&flags[p2]&flags[p3];	
		flag_or		=	flags[p0]|flags[p1]|flags[p2]|flags[p3];	
		if(!(flag_and & EF_CLIPFLAGS))
		{
			
			az=(res[p0].Z+res[p1].Z+res[p2].Z+res[p3].Z)>>2;

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
							(p_f4->Bright[0]),
							(p_f4->Bright[1]),
							(p_f4->Bright[2]),
							(p_f4->Bright[3])
						);
			((struct BucketQuad*)current_bucket_pool)->DebugInfo=c0;

			add_bucket((void *)current_bucket_pool,az);

			current_bucket_pool+=sizeof(struct BucketQuad);
		}
		p_f4++;
	}

	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		p0=p_f3->Points[0]-sp;
		p1=p_f3->Points[1]-sp;
		p2=p_f3->Points[2]-sp;

		flag_and = flags[p0]&flags[p1]&flags[p2];	
		flag_or  = flags[p0]|flags[p1]|flags[p2];	

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
			az=(res[p0].Z+res[p1].Z+res[p2].Z)/3;

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
							(p_f3->Bright[0]),
							(p_f3->Bright[1]),
							(p_f3->Bright[2])
						);

			((struct BucketTri*)current_bucket_pool)->DebugInfo=c0;

			add_bucket((void *)current_bucket_pool,az);

			current_bucket_pool+=sizeof(struct BucketQuad);
		}
		p_f3++;
	}
}

//---------------------------------------------------------------

void	test_draw_all_get_sizes(SWORD multi_prim,struct KeyFrame *the_frame,SLONG x,SLONG y,SLONG z,SLONG tween,struct Matrix33 *rot_mat,SLONG *width,SLONG *height,SLONG *mid_x,SLONG *mid_y)
{
	UWORD					prim;
	ULONG					flag_and,flag_or;
	SLONG					az,
							c0,c1,c2,
							count			=	0,
							ep,
							flags[1560],
							i,j,
							max_x,max_y,
							min_x,min_y,
							mx				=	0,
							my				=	0,
							sp,
							tx,ty,tz;
	struct	KeyFrameElement	*the_element;
	struct	Matrix31		offset;
	struct	Matrix33		*mat,
							*mat_next,
							mat2,
							mat_final;
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	struct	PrimObject		*p_obj;
	struct	SVector			res[1560],temp; //max points per object?



	min_x	=	min_y	=	999999;
	max_x	=	max_y	=	-999999;

	tx=engine.X;
	ty=engine.Y;
	tz=engine.Z;

	for(c2=0,c1=prim_multi_objects[multi_prim].StartObject;c1<=prim_multi_objects[multi_prim].EndObject;c1++)
	{
		the_element			=	&the_frame->FirstElement[c2++];
//   		test_draw(c0,0,0,0,0,the_element,the_element,r_matrix);
		prim=c1;

		p_obj    =&prim_objects[prim];
		p_f4     =&prim_faces4[p_obj->StartFace4];
		p_f3     =&prim_faces3[p_obj->StartFace3];
		

		mat      = &the_element->Matrix;
		mat_next = &the_element->Matrix;

		//move object "tweened quantity"  , z&y flipped
		offset.M[0]	=	(the_element->OffsetX)>>TWEEN_OFFSET_SHIFT;
		offset.M[1]	=	(the_element->OffsetY)>>TWEEN_OFFSET_SHIFT;
		offset.M[2]	=	(the_element->OffsetZ)>>TWEEN_OFFSET_SHIFT;
		matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
		x	= temp.X;
		y	= temp.Y;
		z	= temp.Z;
		

		sp=p_obj->StartPoint;
		ep=p_obj->EndPoint;


		engine.X=-x<<8;
		engine.Y=-y<<8;
		engine.Z=-z<<8;

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

			matrix_transform((struct Matrix31*)&temp,&mat_final, (struct Matrix31*)&prim_points[c0]);
			flags[c0-sp]	=	rotate_point_gte((struct SVector*)&temp,&res[c0-sp]);
			mx+=res[c0-sp].X;
			my+=res[c0-sp].Y;
			count++;

			if(res[c0-sp].X<min_x)
				min_x	= res[c0-sp].X;

			if(res[c0-sp].X>max_x)
				max_x	=	res[c0-sp].X;

			if(res[c0-sp].Y<min_y)
				min_y	=	res[c0-sp].Y;

			if(res[c0-sp].Y>max_y)
				max_y	=	res[c0-sp].Y;
		}
	}

	engine.X=tx;
	engine.Y=ty;
	engine.Z=tz;
	*width	=	max_x-min_x;
	*height	=	max_y-min_y;

	if(count)
	{
		*mid_x=mx/count;
		*mid_y=my/count;
	}

}
*/
//---------------------------------------------------------------
