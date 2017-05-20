
#include	"Editor.hpp"

#include	"game.h"
#include	"Thing.h"
#include	"Structs.h"
#include	"map.h"
//#include	"collide.hpp"
#include	"prim.h"
#include	"prim_draw.h"
//#include	"building.hpp"
#include	"edit.h"

extern void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
extern void matrix_transform(struct Matrix31* result, struct Matrix33* trans,struct  Matrix31* mat2);

extern	UBYTE	two4_line_intersection(SLONG x1,SLONG y1,SLONG x2,SLONG y2,SLONG x3,SLONG y3,SLONG x4,SLONG y4);
extern	SLONG	point_in_quad(SLONG px,SLONG pz,SLONG x,SLONG y,SLONG z,SWORD face);
extern	void	process_camera(struct MapThing *p_thing);
extern	UWORD	calc_lights(SLONG x,SLONG y,SLONG z,struct SVECTOR *p_vect); //prim.h


void	draw_prim_tween(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,struct Matrix33 *rot_mat);
void	load_all_anims(KeyFrameChunk *the_chunk,Anim **anim_list);

UWORD	find_empty_map_thing(void);

extern	void	setup_anim_stuff(void); //edutils
extern	void	load_key_frame_chunks(KeyFrameChunk *the_chunk,CBYTE *vue_name); //edutils
extern	void	load_chunk_texture_info(KeyFrameChunk *the_chunk); //edutils
extern	void	reset_anim_stuff(void);
extern	void	clear_anim_stuff(void);
extern	UWORD	find_empty_map_thing(void);

extern	void	e_draw_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
extern	void	e_draw_3d_mapwho(SLONG x1,SLONG z1);

SLONG					anim_count,
						current_element,
						key_frame_count,
						motion;
SLONG					anim_offset_x,anim_offset_y,
						anim_angle_zx,anim_angle_zy;
Anim					*anim_list,
						*cop_list,
						*darci_list,
						*roper_list,
						*current_anim;
struct KeyFrame			*anim_array[200],
						*cop_array[200],
						*darci_array[200],
						*roper_array[200],
//						*current_frame,
//						*next_frame,
						*queued_frame;
/*
struct KeyFrame			*anim_array[200],
						//*current_frame,
						//*next_frame,
						*queued_frame;
*/
struct KeyFrameChunk 	test_chunk,
						test_chunk2,
						test_chunk3;
struct KeyFrameElement	*the_elements;

static	SLONG	anim_no=0,max_anim=0;


struct MapThing		*cam_thing;
UWORD				cam_chap;

struct	AnimModes
{
	UBYTE	FixLimb;
	UBYTE	StickToFloor;
	UBYTE	EndStance;
	UBYTE	StartStance;
	UBYTE	StartFlags;
	UBYTE	EndFlags;
};

#define	END_ON_FACE	(1<<0)

#define	MODE_READY		(1)
#define	MODE_RUN		(2)
#define	MODE_WALK		(3)
#define	MODE_SIDLE		(4)
#define	MODE_CRAWL		(5)
#define	MODE_CLIMB		(6)
#define	MODE_HANG		(7)
#define	MODE_LONG_JUMP	(8)
#define	MODE_HIGH_JUMP	(9)
#define	MODE_PULL_UP	(10)
#define	MODE_HURDLE		(11)
#define	MODE_DRAW_GUN	(12)
#define	MODE_SHOOT		(13)


//are thse modes interrupatble 
/*

  cancel a pull up by dropping back to the floor
  cancel a run, just stop
           

*/

#define	FLAG_THING_GUN1_OUT	(1<<?)
#define	FLAG_THING_GUN2_OUT	(1<<?)

struct	AnimModes anim_modes[]=
{
	{0,1,0,0,0,0}, //stand
	{0,1,0,0,0,0}, //walk
	{0,1,0,0,0,0}, //run
	{0,1,0,0,0,0}, //Stand (hip)
	{0,1,0,0,0,0}, //stand (ready)
	{0,1,0,0,0,0}, //Punch
	{0,0,0,0,0,0}, //Fly (jump from run)
	{0,0,0,0,0,0}, //Fly grab
	{0,0,0,0,0,0}, //Run, Jump, Land
	{0,0,0,0,0,0}, //Stand, Jump,Land
	{0,1,0,0,0,0}, //Reverse Round Kick
	{0,1,0,0,0,0}, // Draw Gun
	{0,1,0,0,0,0}, // Draw And Shoot
	{0,1,0,0,0,0}, // Aim Gun
	{0,0,0,0,0,END_ON_FACE}, //Jump Up, Grab, Pullup And Climb On
	{0,0,0,0,0,0}, //Mount Ladder, climb 2 runs
	{0,0,0,0,0,0}, //Mount Ladder
	{0,0,0,0,0,0}, //Climb One Run
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},
	{0,0,0,0,0,0},

};


CBYTE	*id_name[]=
{
	"none",
	"FIRST SLOPE",
	"PLINTH1",
	"WALKWAY1",
	"PLINTH2",
	"SLOPE2",
	""
};

SLONG	is_thing_on_this_quad(SLONG x,SLONG z,SLONG face)
{
	SLONG ox;
	SLONG oy;
	SLONG oz;

	SLONG	wall;
	SLONG	storey;
	SLONG	building;
	SLONG	thing;
	Thing  *p_thing;

	ASSERT(WITHIN(face, 1, next_prim_face4));

	wall = prim_faces4[face].ThingIndex;

	if (wall < 0)
	{
		storey   = wall_list[-wall].StoreyHead;
		building = storey_list[storey].BuildingHead;
		thing    = building_list[building].ThingIndex;

		p_thing = TO_THING(thing);

		ox = p_thing->WorldPos.X >> 8;
		oy = p_thing->WorldPos.Y >> 8;
		oz = p_thing->WorldPos.Z >> 8;

		if (point_in_quad(x, z, ox, oy, oz, face))
		{
			return 1;
		}
	}
	
	return 0;
}

#ifdef	DOGPOO

#define	PERSON_RADIUS	(50)
SLONG	check_vect(SLONG m_dx,SLONG m_dy,SLONG m_dz,struct MapThing *p_thing,SLONG scale)
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
//	e_draw_3d_line(p_thing->X,p_thing->Y,p_thing->Z,p_thing->X+m_dx*scale,p_thing->Y+m_dy*scale,p_thing->Z+m_dz*scale);

	cell_dx=-((p_thing->X)>>ELE_SHIFT)+((p_thing->X+m_dx*scale)>>ELE_SHIFT);
	cell_dz=-((p_thing->Z)>>ELE_SHIFT)+((p_thing->Z+m_dz*scale)>>ELE_SHIFT);

	col=do_move_collide(p_thing->X,p_thing->Y,p_thing->Z,m_dx,m_dy,m_dz,0,0,scale);		

	if(cell_dx&&!col)
	{
		col=do_move_collide(p_thing->X,p_thing->Y,p_thing->Z,m_dx,m_dy,m_dz,cell_dx,0,scale);		
	}	
	if(cell_dz&&!col)
	{
		col=do_move_collide(p_thing->X,p_thing->Y,p_thing->Z,m_dx,m_dy,m_dz,0,cell_dz,scale);		
	}	
	if(cell_dx && cell_dz && !col)
	{
		col=do_move_collide(p_thing->X,p_thing->Y,p_thing->Z,m_dx,m_dy,m_dz,cell_dx,cell_dz,scale);		
	}
	return(col);
}


SLONG	check_vect_circle(SLONG m_dx,SLONG m_dy,SLONG m_dz,struct MapThing *p_thing,SLONG radius)
{
	SLONG	x,y,z;
	SLONG	dx,dz;
	SLONG	cell_radius;
	ULONG	col;



	x=m_dx+p_thing->X;
	y=m_dy+p_thing->Y;
	z=m_dz+p_thing->Z;

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

ULONG	move_thing(SLONG m_dx,SLONG m_dy,SLONG m_dz,struct MapThing *p_thing)
{
	ULONG	col;
	SLONG	on_its_quad=0;
	SLONG	on_connected_quad=0;

//	if(!Keys[KB_C])
//		col=0;

/*
	// check perpendicular
	col=check_vect(m_dz,m_dy,-m_dx,p_thing,3);
*/
	//check movement
//	if(col==0)
//		col=check_vect(m_dx,m_dy,m_dz,p_thing,4);

	if(p_thing&&p_thing->OnFace)
	{
		on_its_quad = is_thing_on_this_quad(p_thing->X+m_dx,p_thing->Z+m_dz,p_thing->OnFace);
		if(!on_its_quad)
		{
extern	SLONG	find_face_for_this_pos(SLONG x,SLONG y,SLONG z,SLONG ignore_faces_of-this_building);
		on_connected_quad=find_face_for_this_pos(p_thing->X+m_dx,p_thing->Y+m_dy,p_thing->Z+m_dz,0);

/*
			SLONG	count=0,check_face,offset,index;

			index=edit_map[(p_thing->X+m_dx)>>ELE_SHIFT][(p_thing->Z+m_dz)>>ELE_SHIFT].Walkable;
			while(index&&on_connected_quad==0)
			{
				check_face=walk_links[index].Face;

				if(check_face>0)
				if(is_thing_on_this_quad(p_thing->X+m_dx,p_thing->Z+m_dz,check_face))
					on_connected_quad=check_face;

				index=walk_links[index].Next;
			}
*/

/*
			offset=next_connected_face(FACE_TYPE_FIRE_ESCAPE,prim_faces4[p_thing->OnFace].ID,count);
			LogText(" current face %d ID %s first offset %d \n",check_face,id_name[prim_faces4[check_face].ID],offset);
			check_face+=offset;
			count++;

			while(on_connected_quad==0&&offset)
			{
				LogText(" check face with id %s \n",id_name[prim_faces4[check_face].ID]);
				if(is_thing_on_this_quad(p_thing->X+m_dx,p_thing->Z+m_dz,check_face))
				{
					LogText(" on it \n");
					on_connected_quad=check_face;
				}
				LogText(" on connected face %d check_face %d id %d count %d \n",on_connected_quad,check_face,prim_faces4[p_thing->OnFace].ID,count);
				offset=next_connected_face(FACE_TYPE_FIRE_ESCAPE,prim_faces4[p_thing->OnFace].ID,count);
				LogText(" offset %d \n",offset);
				check_face=p_thing->OnFace+offset;
				count++;
			}
*/

		}
	}
	else
		col=check_vect_circle(m_dx,m_dy,m_dz,p_thing,30);

	if(col&&p_thing)
	{
		SWORD face;
		//we have hit something 
		face=col_vects[col].Face;
//		LogText(" col %d face %d onface %d on its quad %d \n",col,face,p_thing->OnFace,on_its_quad);  //RUD

		if(p_thing->OnFace)
		{

			if(face==p_thing->OnFace)
			{
				LogText("crossed vector for face I was on\n");
				//crossed vector for face I'm on
				if(on_its_quad)
				{
					LogText("crossed vector for face I'm on but I'm still on it\n");
					col=0;
					goto	do_move;
					//yet I'm still on the face which is odd, as I must have been off it previously and so shouldnt of had onface set
				}
				else
				{
					// crossed vect, now off face, so must be on floor
					LogText(" back on floor \n");
					col=0;
					p_thing->OnFace=0;
					goto do_move;
				}
			}
			else
			{
				//crossed a vect not connected with face I'm on so ignore it
					LogText(" ignore unimportant vect \n");
				col=0;
			}
		}
		else
		if(face>0)
		{
			if(prim_faces4[face].FaceFlags&FACE_FLAG_WALKABLE)
			{
				LogText(" walk onto face %d \n",face);
				col=0;
				p_thing->OnFace=face;
				if(!is_thing_on_this_quad(p_thing->X+m_dx,p_thing->Z+m_dz,p_thing->OnFace))
					p_thing->OnFace=0;								// i thought I was on the face but alas no

				goto do_move;
			}
		}
	}
	if(p_thing&&p_thing->OnFace)
	{
		if(!on_its_quad)
		{
			if(on_connected_quad)
			{
				p_thing->OnFace=on_connected_quad;
				goto	do_move;
			}
			//trying to walk off face
			LogText(" walk off edge of face halted \n");
			return(0); // return that couldnt make move, should check connecting faces
		}
	}

do_move:;		
	if(!col)
	{
		//LogText(" move occurs \n");
		p_thing->X+=m_dx;
		p_thing->Y+=m_dy;
		p_thing->Z+=m_dz;
		return(1);
	}	
	return(0);
}

/* poo
void	place_player_on_nearby_face(struct MapThing p_thing)
{
	SLONG	count=0,check_face,offset,index;

	index=edit_map[(p_thing->X)>>ELE_SHIFT][(p_thing->Z)>>ELE_SHIFT].Walkable;
	while(index&&on_connected_quad==0)
	{
		check_face=walk_links[index].Face;

		if(check_face>0)
		if(is_thing_on_this_quad(p_thing->X+m_dx,p_thing->Z+m_dz,check_face))
			on_connected_quad=check_face;

		index=walk_links[index].Next;
	}

}
*/

void	interface_thing2(struct MapThing *p_thing)
{
   	SLONG	dx=0,dy=0,dz=0;
	static	turn;
	SLONG	col;
	static	bright=256;
	static	SLONG light_z=320;

	if(ShiftFlag)
	{
		if(Keys[KB_RIGHT])
		{
			dx=(SIN( ((p_thing->AngleY)+2048+512)&2047)*8)>>16;
			dz=(COS( ((p_thing->AngleY)+2048+512)&2047)*8)>>16;
		}
		if(Keys[KB_LEFT])
		{
			dx=-(SIN( ((p_thing->AngleY)+2048+512)&2047)*8)>>16;
			dz=-(COS( ((p_thing->AngleY)+2048+512)&2047)*8)>>16;
		}
		if(Keys[KB_UP])
		{
			dy=40;
		}
		if(Keys[KB_DOWN])
		{
			dy=-40;
		}
	}
	else
	{
		if(Keys[KB_RIGHT])
		{
			p_thing->AngleY-=2048>>6;
			p_thing->AngleY=((p_thing->AngleY+(2048))&((2048)-1));
		}

		if(Keys[KB_LEFT])
		{
			p_thing->AngleY+=2048>>6;
			p_thing->AngleY=((p_thing->AngleY+(2048))&((2048)-1));
		}
		
		if(Keys[KB_UP])
		{
			dx=-(SIN( ((p_thing->AngleY)+2048)&2047)*18)>>16;
			dz=-(COS( ((p_thing->AngleY)+2048)&2047)*18)>>16;
		}


		if(Keys[KB_DOWN])
		{
			dx=(SIN( ((p_thing->AngleY)+2048)&2047)*18)>>16;
			dz=(COS( ((p_thing->AngleY)+2048)&2047)*18)>>16;
		}
	}


	engine_keys_zoom();
	engine_keys_spin();

	col=move_thing(dx,dy,dz,p_thing);

	if(p_thing)
	{
		//p_thing->X=play_x;
		//p_thing->Z=play_z;
		if(anim_modes[anim_no].StickToFloor)
			calc_things_height(p_thing);
	}
	else
	{
		//play_y=calc_height_at(play_x,play_z);
	}

	process_camera(p_thing);
	//engine.X=play_x<<8;
	//engine.Y=(play_y+200)<<8;
	//engine.Z=play_z<<8;

//	draw_thing();
	//hilited_face.Face=0;
	//selected_face.Face=0;

}

//***********************************************************************************

//stuf ripped from game.cpp

//***********************************************************************************

void	setup_game(void)
{

	setup_anim_stuff();

	anim_count		=	0;


	anim_list		=	NULL;
	current_anim	=	NULL;
	motion			=	0;

	darci_list		=	NULL;
	roper_list		=	NULL;
	cop_list		=	NULL;

   	load_key_frame_chunks(&test_chunk,"darci1.vue");
	load_all_anims(&test_chunk,&darci_list);
//	load_chunk_texture_info(&test_chunk);
	if(darci_list)
	{
		SLONG	c0=0;
		Anim	*curr;

		curr=darci_list;
		while(curr)
		{
			darci_array[c0]	=	curr->GetFrameList();
			anim_array[c0]	=	darci_array[c0];
			curr=curr->GetNextAnim();
			c0++;
		}
		max_anim=c0;
	}

	load_key_frame_chunks(&test_chunk2,"roper.vue");
	load_all_anims(&test_chunk2,&roper_list);
	load_chunk_texture_info(&test_chunk2);
	if(roper_list)
	{
		SLONG	c0=0;
		Anim	*curr;

		curr=roper_list;
		while(curr)
		{
			roper_array[c0]	=	curr->GetFrameList();
			curr=curr->GetNextAnim();
			c0++;
		}
		max_anim=c0;
	}

	load_key_frame_chunks(&test_chunk3,"cop.vue");
	load_all_anims(&test_chunk3,&cop_list);
	load_chunk_texture_info(&test_chunk3);
	if(cop_list)
	{
		SLONG	c0=0;
		Anim	*curr;

		curr=cop_list;
		while(curr)
		{
			cop_array[c0]	=	curr->GetFrameList();
			curr=curr->GetNextAnim();
			c0++;
		}
		max_anim=c0;
	}

	anim_offset_x	=	0;
	anim_offset_y	=	0;
	anim_angle_zx	=	0;
	anim_angle_zy	=	0;

/*
	if(anim_list)
	{
		SLONG	c0=0;
		Anim	*curr;

		curr=anim_list;
		while(curr)
		{
			anim_array[c0]	=	curr->GetFrameList();
			curr=curr->GetNextAnim();
			c0++;
		}
		max_anim=c0;
	}
*/
	//current_frame	=	anim_array[0];
	//next_frame      =   current_frame->NextFrame;
	queued_frame	=	0;

	//game_flags		|=	GAME_OKAY;

}

//---------------------------------------------------------------

void	reset_game(void)
{
//	MemFree(the_elements);
//	reset_anim_stuff();
	clear_anim_stuff();
}

//---------------------------------------------------------------


struct	MapThing	man_thing;

struct MapThing *init_test_bloke_system(void)
{

	setup_game();

	man_thing.X=engine.X>>8;
	man_thing.Y=engine.Y>>8;
	man_thing.Z=engine.Z>>8;

	man_thing.Y=calc_height_at(man_thing.X,man_thing.Z);


	man_thing.AngleX	=	0;
	man_thing.AngleY	=	0;
	man_thing.AngleZ	=	0;
	man_thing.Type	=	MAP_THING_TYPE_ROT_MULTI;
	man_thing.IndexOther	=	1;
	man_thing.TweenStage	=	0;

	man_thing.CurrentFrame=anim_array[0];
	man_thing.NextFrame=anim_array[0];
	anim_no=0;

//	man_thing.AnimElements	=	current_frame->FirstElement;
//	man_thing.NextAnimElements	=	man_thing.AnimElements;
	man_thing.OnFace=0;

	man_thing.State=MODE_READY;

	return(&man_thing);

}

/*
 if same piece locked as last time
 {
	find co_ord of piece last time
	find co_ord of piece this time
	move object such that co_ords are the same
 }
 else
 {

 }
*/ 

void	calc_sub_objects_position(struct MapThing *p_mthing,SLONG tween,UWORD object,SLONG *x,SLONG *y,SLONG *z)
{
	struct	SVECTOR		temp; //max points per object?
	struct Matrix33		r_matrix;
	struct	Matrix31	offset;
	struct KeyFrameElement *anim_info;
	struct KeyFrameElement *anim_info_next;
	struct Matrix33 *rot_mat;

	anim_info=&p_mthing->CurrentFrame->FirstElement[object];
	anim_info_next=&p_mthing->NextFrame->FirstElement[object];
//	anim_info_next=&p_mthing->NextAnimElements[object];


	
	offset.M[0]	=	(anim_info->OffsetX+(((anim_info_next->OffsetX-anim_info->OffsetX)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[1]	=	(anim_info->OffsetY+(((anim_info_next->OffsetY-anim_info->OffsetY)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[2]	=	(anim_info->OffsetZ+(((anim_info_next->OffsetZ-anim_info->OffsetZ)*tween)>>8))>>TWEEN_OFFSET_SHIFT;

	rotate_obj	(
					p_mthing->AngleX,
					p_mthing->AngleY,
					p_mthing->AngleZ,
					&r_matrix
				);

	matrix_transformZMY((struct Matrix31*)&temp,&r_matrix, &offset);
	*x=temp.X;
	*y=temp.Y;
	*z=temp.Z;

}

void	calc_sub_objects_position_ele(struct MapThing *p_mthing,KeyFrameElement *current,SLONG tween,UWORD object,SLONG *x,SLONG *y,SLONG *z)
{
	struct	SVECTOR		temp; //max points per object?
	struct Matrix33		r_matrix;
	struct	Matrix31	offset;
	struct KeyFrameElement *anim_info;
	struct KeyFrameElement *anim_info_next;
	struct Matrix33 *rot_mat;

	anim_info=&current[object];
	
	offset.M[0]	=	anim_info->OffsetX;
	offset.M[1]	=	anim_info->OffsetY;
	offset.M[2]	=	anim_info->OffsetZ;

	rotate_obj	(
					p_mthing->AngleX,
					p_mthing->AngleY,
					p_mthing->AngleZ,
					&r_matrix
				);

	matrix_transformZMY((struct Matrix31*)&temp,&r_matrix, &offset);
	*x=temp.X;
	*y=temp.Y;
	*z=temp.Z;

}


void	animate_bloke(struct MapThing *p_thing)
{
	struct	KeyFrame *the_keyframe;
	static	SLONG	prev_fixed,prev_x,prev_y,prev_z;
	static	SLONG	prev_dx=0,prev_dy=0,prev_dz=0;
	SLONG	fixed,next_fixed,x,y,z;
	UBYTE	anim_change=0;
	UBYTE	add_ten=0;

	if(ControlFlag)
		add_ten=10;

	if(Keys[KB_F11])
		p_thing->Y=calc_height_at(p_thing->X,p_thing->Z);

	if(Keys[KB_F1])
	{
		anim_no=1+add_ten;
		queued_frame=anim_array[1+add_ten];
	}
	if(Keys[KB_F2])
	{
		anim_no=2+add_ten;
		queued_frame=anim_array[2+add_ten];
	}
	if(Keys[KB_F3])
	{
		anim_no=3+add_ten;
		queued_frame=anim_array[3+add_ten];
	}
	if(Keys[KB_F4])
	{
		anim_no=4+add_ten;
		queued_frame=anim_array[4+add_ten];
	}
	if(Keys[KB_F5])
	{
		anim_no=5+add_ten;
		queued_frame=anim_array[5+add_ten];
	}
	if(Keys[KB_F6])
	{
		anim_no=6+add_ten;
		queued_frame=anim_array[6+add_ten];
	}
	if(Keys[KB_F7])
	{
		anim_no=7+add_ten;
		queued_frame=anim_array[7+add_ten];
	}
	if(Keys[KB_F8])
	{
		anim_no=8+add_ten;
		queued_frame=anim_array[8+add_ten];
	}
	if(Keys[KB_F9])
	{
		anim_no=9+add_ten;
		queued_frame=anim_array[9+add_ten];
	}
	if(Keys[KB_F10])
	{
		anim_no=10+add_ten;
		queued_frame=anim_array[10+add_ten];
	}

	if(Keys[KB_POINT])
	{
		Keys[KB_POINT]=0;
		anim_no++;
		if(anim_no<max_anim&&anim_array[anim_no])
			queued_frame=anim_array[anim_no];
		else
			anim_no--;
	}
	if(Keys[KB_COMMA])
	{
		Keys[KB_COMMA]=0;
		anim_no--;
		
		if(anim_no>=0&&anim_array[anim_no])
			queued_frame=anim_array[anim_no];
		else
			anim_no++;
	}

	if(p_thing->CurrentFrame)
	{
//			anim_tween	+=	16;
		p_thing->TweenStage	+=	256/(p_thing->CurrentFrame->TweenStep+1);
		if(p_thing->TweenStage>255)
		{
			p_thing->TweenStage	-=	256;
			p_thing->CurrentFrame	=	p_thing->NextFrame;
/*
			if(queued_frame)
			{
				current_frame	=	queued_frame;
				next_frame		=	current_frame->NextFrame;
				anim_tween=0;

				anim_change=1;
				queued_frame	=	0;
			}
			else
*/
			{
				if(p_thing->NextFrame->NextFrame)
					p_thing->NextFrame		=	p_thing->NextFrame->NextFrame;
				//else
				//	if(anim_mode[anim_no].EndFlags&END_ON_FACE)
				//		place_player_on_nearby_face(&man_thing);
			}

		}
	}
	if(queued_frame)
	{
		SLONG	x1,y1,z1,x2,y2,z2;
		anim_change=1;
		p_thing->TweenStage	=	0;

		calc_sub_objects_position(&man_thing,p_thing->TweenStage,3,&x1,&y1,&z1);
		calc_sub_objects_position_ele(&man_thing,queued_frame->FirstElement,0,3,&x2,&y2,&z2);


		p_thing->CurrentFrame	=	queued_frame;
		p_thing->NextFrame		=	p_thing->CurrentFrame->NextFrame;
		if(p_thing->NextFrame==0)
			p_thing->NextFrame		=	p_thing->CurrentFrame;
		queued_frame	=	0;
//		p_thing->AnimElements	=	current_frame->FirstElement;
//		if(next_frame)
//			p_thing->NextAnimElements	=	next_frame->FirstElement;
//		else
//		{
//			next_frame=current_frame;
//			p_thing->NextAnimElements	=	current_frame->FirstElement; // same current and next frame
//		}

		LogText(" change to anim %d dx %d dy %d dz %d \n",anim_no,x1-x2,y1-y2,z1-z2);
		p_thing->X-=x2-x1;
		p_thing->Y-=y2-y1; //this will keep getting reset to the floor height
		p_thing->Z-=z2-z1;
		move_thing(-(x2-x1),-(y2-y1),-(z2-z1),p_thing);
	}
	else
	if(current_anim)
	{
/*
		if(p_thing->CurrentFrame)
		{
			p_thing->AnimElements	=	current_frame->FirstElement;
			if(next_frame)
				p_thing->NextAnimElements	=	next_frame->FirstElement;
			else
				p_thing->NextAnimElements	=	p_thing->AnimElements;
		}
*/
	}

	fixed=p_thing->CurrentFrame->Fixed;
	next_fixed=p_thing->NextFrame->Fixed;
	if(fixed!=next_fixed)
		fixed=0;
	calc_sub_objects_position(p_thing,p_thing->TweenStage,fixed,&x,&y,&z);

	if(anim_change)
	{

		prev_dx=0;
		prev_dy=0;
		prev_dz=0;
		

	}
	else
	{
		//LogText(" man anim parent %d \n",p_thing->AnimElements->Parent);


		//LogText(" prev animate bloke fixed %d x %d y %d z %d \n",prev_fixed,prev_x,prev_y,prev_z);
		//LogText("      animate bloke fixed %d x %d y %d z %d \n",fixed,x,y,z);


	/*
		if(anim_change)
		{
			SLONG	tx,ty,tz;

			prev_dx=0;
			prev_dy=0;
			prev_dz=0;

			calc_sub_objects_position(&man_thing,p_thing->TweenStage,prev_fixed,&tx,&ty,&tz);
			tx=prev_x-tx;
			ty=prev_y-ty;
			tz=prev_z-tz;

			p_thing->X+=tx;
			p_thing->Y+=ty; //this will keep getting reset to the floor height
			p_thing->Z+=tz;

		}
		else
	*/
		{
//			calc_sub_objects_position(&man_thing,p_thing->TweenStage,fixed,&x,&y,&z);
		}

		if(fixed==prev_fixed&&fixed)
		{
			prev_dx=prev_x-x;
			prev_dy=prev_y-y;
			prev_dz=prev_z-z;
		}

		//p_thing->X+=prev_dx;
		//p_thing->Y+=prev_dy; //this will keep getting reset to the floor height
//		p_thing->Z+=prev_dz;
		LogText(" sticky foot correction dx %d dy %d dz %d \n",prev_dx,prev_dy,prev_dz);
		move_thing(prev_dx,prev_dy,prev_dz,p_thing);

	//	LogText(" offset %d %d %d \n",x-prev_x,y-prev_y,z-prev_z);

	}
	prev_fixed=fixed;
	prev_x=x;
	prev_y=y;
	prev_z=z;
}

void	draw_test_bloke(SLONG x,SLONG y,SLONG z,UBYTE anim,SLONG angle)
{
	SLONG				c0,c1;
	struct Matrix33		r_matrix;
	struct	MapThing	*p_mthing;




	p_mthing=&man_thing;
	if(anim)
		animate_bloke(p_mthing);
	rotate_obj	(
					p_mthing->AngleX,
					(p_mthing->AngleY+angle)&2047,
					p_mthing->AngleZ,
					&r_matrix
				);
	//anim_info=&p_mthing->CurrentFrame->FirstElement[object];
	//anim_info_next=&p_mthing->NextFrame->FirstElement[object];
	//if(p_mthing->AnimElements&&p_mthing->NextAnimElements)
	if(p_mthing->CurrentFrame&&p_mthing->NextFrame)
	{
		for(c1=0,c0=prim_multi_objects[test_chunk.MultiObject].StartObject;c0<=prim_multi_objects[test_chunk.MultiObject].EndObject;c0++,c1++)
		{
			if(c1==1)
			{
extern UBYTE	store_pos;
				store_pos	=	1;
			}
			draw_prim_tween	(
								c0,
								p_mthing->X,p_mthing->Y,p_mthing->Z,
								p_mthing->TweenStage,
								&p_mthing->CurrentFrame->FirstElement[c1],
								&p_mthing->NextFrame->FirstElement[c1],
								&r_matrix
							);
		}
	}
}



//SLONG	calc_shadow_co_ord((struct Matrix31*)&temp,&temp_shadow,10000,10000,10000);//light co_ord
extern	SLONG	calc_height_at(SLONG x,SLONG z);
SLONG	calc_shadow_co_ord(struct SVECTOR *input,struct SVECTOR *output,SLONG l_x,SLONG l_y,SLONG l_z)
{
	SLONG	dx,dy,dz;
	SLONG	alt=0;
	SLONG	m,c;

	dx=l_x-input->X;
	dy=l_y-input->Y;
	dz=l_z-input->Z;

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



UBYTE	store_pos	=	0;



void	draw_prim_tween(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,struct Matrix33 *rot_mat)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	flags[1560];
	UWORD	bright[1560];
	ULONG	flag_and,flag_or;
	struct	SVECTOR			res[1560],temp; //max points per object?
	struct	SVECTOR			res_shadow[1560],temp_shadow; //max points per object?
	SLONG	flags_shadow[1560];
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG az;
	struct	Matrix33 *mat,*mat_next,mat2,mat_final;
	SLONG	i,j;
	struct	Matrix31	offset;
	struct	KeyFrame	*the_keyframe1,*the_keyframe2;
	SLONG	dx,dy,dz;
	SLONG	shadow=1;
	SLONG	mapx,mapy,mapz;

	p_obj    =&prim_objects[prim];
	p_f4     =&prim_faces4[p_obj->StartFace4];
	p_f3     =&prim_faces3[p_obj->StartFace3];
	
	mat      = &anim_info->Matrix;
	mat_next = &anim_info_next->Matrix;

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


	if(store_pos)
	{
		extern Coord		thing_position;

		thing_position.X	=	x;
		thing_position.Y	=	y;
		thing_position.Z	=	z;
		store_pos	=	0;
	}

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
		temp.X+=mapx;
		temp.Y+=mapy;
		temp.Z+=mapz;
		calc_shadow_co_ord(&temp,&temp_shadow,9000*2,4000,8000*2);//light co_ord
		flags_shadow[c0-sp]	=	rotate_point_gte((struct SVECTOR*)&temp_shadow,&res_shadow[c0-sp]);

		flags[c0-sp]	=	rotate_point_gte((struct SVECTOR*)&temp,&res[c0-sp]);
		bright[c0-sp]	=	calc_lights(x,y,z,(struct SVECTOR*)&prim_points[c0]);
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

//*****************************************
		if(shadow)
		{
			flag_and = flags_shadow[p0]&flags_shadow[p1]&flags_shadow[p2]&flags_shadow[p3];	
			flag_or = flags_shadow[p0]|flags_shadow[p1]|flags_shadow[p2]|flags_shadow[p3];	

			if((flag_or&EF_BEHIND_YOU)==0)
			if(!(flag_and & EF_CLIPFLAGS))
			{
				az=(res_shadow[p0].Z+res_shadow[p1].Z+res_shadow[p2].Z+res_shadow[p3].Z)>>2;
				az-=150;

				setPolyType4(
								current_bucket_pool,
								POLY_F
							);

				setCol4	(
							(struct BucketQuad*)current_bucket_pool,
							0
						);

				setXY4	(
							(struct BucketQuad*)current_bucket_pool,
							res_shadow[p0].X,res_shadow[p0].Y,
							res_shadow[p1].X,res_shadow[p1].Y,
							res_shadow[p2].X,res_shadow[p2].Y,
							res_shadow[p3].X,res_shadow[p3].Y
						);


				setZ4((struct BucketQuad*)current_bucket_pool,-res_shadow[p0].Z,-res_shadow[p1].Z,-res_shadow[p2].Z,-res_shadow[p3].Z);
				((struct BucketQuad*)current_bucket_pool)->DebugInfo=c0;
				((struct BucketQuad*)current_bucket_pool)->DebugFlags=0;
				
				add_bucket((void *)current_bucket_pool,az);

				current_bucket_pool+=sizeof(struct BucketQuad);

			}
		}
//************************************************


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
						p_f4->Col
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
						p_f3->Col
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

//---------------------------------------------------------------

SLONG	get_distance(Coord *position1,Coord *position2);
SLONG	get_approx_distance(Coord *position1,Coord *position2);
SLONG	get_distance_xz(Coord *position1,Coord *position2);
SLONG	get_angle_xz(Coord *position1,Coord *position2);
SLONG	get_angle_yz(Coord *position1,Coord *position2);

SLONG		cam_x_offset	=	0,
			cam_y_offset	=	0,
			cam_z_offset	=	-50;

Coord		thing_position;

void	set_game_camera(struct MapThing *track_thing)
{
	SLONG			angle_xz,
					angle_yz,
					cam_distance,
					cam_x_distance,
					cam_y_distance,
					cam_z_distance,
					new_x,
					new_y,
					new_z;
	Coord			cam_position,
					dest_position;
//					thing_position;
	KeyFrameElement	*element1,
					*element2;

		  
	engine.VW=WorkWindowWidth;
	engine.VH=WorkWindowHeight;

	engine.VW2=engine.VW>>1;
	engine.VH2=engine.VH>>1;

	engine.Scale	=	296;
	rotate_point_gte=	rotate_point_gte_perspective;

	// Set up the cameras position, its dest position & the position
	// of the thing its pointing at.
	// (We should be able to get rid of this once we've formalised the Coord struct).
	cam_position.X		=	cam_thing->X;
	cam_position.Y		=	cam_thing->Y;
	cam_position.Z		=	cam_thing->Z;    

/*
	element1			=	&track_thing->AnimElements[1];
	element2			=	&track_thing->NextAnimElements[1];

	thing_position.X	=	(element1->OffsetX+(((element2->OffsetX-element1->OffsetX)*track_thing->TweenStage)>>8))>>1;
	thing_position.Y	=	(element1->OffsetY+(((element2->OffsetY-element1->OffsetY)*track_thing->TweenStage)>>8))>>1;
	thing_position.Z	=	(element1->OffsetZ+(((element2->OffsetZ-element1->OffsetZ)*track_thing->TweenStage)>>8))>>1;

	thing_position.X	+=	track_thing->X;
	thing_position.Y	+=	track_thing->Y;
	thing_position.Z	+=	track_thing->Z;
*/
/*
	thing_position.X	=	track_thing->X;
	thing_position.Y	=	track_thing->Y;
	thing_position.Z	=	track_thing->Z;
*/

	dest_position.X		=	thing_position.X+cam_x_offset;
	dest_position.Y		=	thing_position.Y+cam_y_offset;
	dest_position.Z		=	thing_position.Z+cam_z_offset;


	// Move the camera towards its offset position. (I know, its a shit algorithm).
	cam_x_distance		=	(dest_position.X-cam_position.X);
	cam_y_distance		=	(dest_position.Y-cam_position.Y);
	cam_z_distance		=	(dest_position.Z-cam_position.Z);
	cam_position.X		+=	(cam_x_distance*20)/256;
	cam_position.Y		+=	(cam_y_distance*20)/256;
	cam_position.Z		+=	(cam_z_distance*20)/256;


	// Botch to stop camera pointing at chaps feet.
//	thing_position.Y	-=	150;

	// Point camera at thing we're tracking.	

	angle_xz	=	-get_angle_xz(&thing_position,&cam_position);
	engine.CosY=COS((angle_xz)&2047);
	engine.SinY=SIN((angle_xz)&2047);

	angle_yz	=	get_angle_yz(&cam_position,&thing_position);
	engine.CosX=COS((angle_yz)&2047);
	engine.SinX=SIN((angle_yz)&2047);

/*
	engine.CosX=COS(0);
	engine.SinX=SIN(0);

	engine.CosY=COS(0);
	engine.SinY=SIN(0);

	engine.CosZ=COS((engine.AngleZ>>8)&2047);
	engine.SinZ=SIN((engine.AngleZ>>8)&2047);
*/
/*
	engine.X	=	dest_position.X<<8;
	engine.Y	=	dest_position.Y<<8;
	engine.Z	=	dest_position.Z<<8;

	return;
*/
	// Remove botch.
//	thing_position.Y	+=	150;


	// Set the engine coords to the cam position.	
	cam_thing->X	=	cam_position.X;
	cam_thing->Y	=	cam_position.Y;
	cam_thing->Z	=	cam_position.Z;

	engine.X	=	cam_thing->X<<8;
	engine.Y	=	cam_thing->Y<<8;
	engine.Z	=	cam_thing->Z<<8;


	// Offset camera z.
	if(Keys[KB_INS])
	{
		cam_z_offset	-=	16;
		LogText("Z offset - %ld, engine scale - %ld",cam_z_offset,engine.Scale);
	}
	else if(Keys[KB_PGUP])
	{
		cam_z_offset	+=	16;
		LogText("Z offset - %ld, engine scale - %ld",cam_z_offset,engine.Scale);
	}

	// Offset camera y.
	if(Keys[KB_HOME])
	{
		cam_y_offset	-=	16;
	}
	else if(Keys[KB_END])
	{
		cam_y_offset	+=	16;
	}

	// Offset camera x.
	if(Keys[KB_DEL])
	{
		cam_x_offset	-=	16;
	}
	else if(Keys[KB_PGDN])
	{
		cam_x_offset	+=	16;
	}

	if(Keys[KB_G])
	{
		SLONG	mid_x,mid_y,mid_z;
extern	UWORD	make_poly_into_glass_shatter_prim(SWORD face,SWORD mid_x,SWORD mid_y,SWORD mid_z);
extern	void	calc_face_midpoint(SWORD face,SLONG *x,SLONG *y,SLONG *z);
		calc_face_midpoint(31989,&mid_x,&mid_y,&mid_z);
		make_poly_into_glass_shatter_prim(31989,mid_x,mid_y,mid_z);
		Keys[KB_G]=0;
		
	}
}
#endif



//---------------------------------------------------------------

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
			LogText(" fight count load = %d \n",count);

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

void	save_anim(MFFileHandle file_handle,Anim *the_anim)
{
	ULONG			anim_flags;
	SLONG			c0,
					frame_count;
	KeyFrame		*frame_list;
	UBYTE			version=1;


	frame_list	=	the_anim->GetFrameList();
	anim_flags	=	the_anim->GetAnimFlags();
	frame_count	=	the_anim->GetFrameCount();

	FileWrite(file_handle,&version,1);
	FileWrite(file_handle,the_anim->GetAnimName(),ANIM_NAME_SIZE);
	FileWrite(file_handle,&anim_flags,sizeof(anim_flags));
	FileWrite(file_handle,&frame_count,sizeof(frame_count));
	for(c0=0;c0<frame_count;c0++)
	{
		FileWrite(file_handle,&frame_list->ChunkID,sizeof(frame_list->ChunkID));
		FileWrite(file_handle,&frame_list->FrameID,sizeof(frame_list->FrameID));
		FileWrite(file_handle,&frame_list->TweenStep,sizeof(frame_list->FrameID));
		FileWrite(file_handle,&frame_list->Fixed,sizeof(frame_list->Fixed));
		frame_list	=	frame_list->NextFrame;
	}
	frame_list	=	the_anim->GetFrameList();
}

//---------------------------------------------------------------

void	create_anim(Anim **the_list)
{
	CBYTE		text[32];
	Anim		*next_anim,
				*the_anim;


	the_anim	=	new Anim;
	if(the_anim)
	{
		anim_count++;
   		sprintf(text,"New Anim %d",anim_count);
		the_anim->SetAnimName(text);
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

void	load_all_anims(KeyFrameChunk *the_chunk,Anim **anim_list)
{
	SLONG			anim_count,
					c0;
	MFFileHandle	file_handle;


	file_handle	=	FileOpen(the_chunk->ANMName);
	if(file_handle!=FILE_OPEN_ERROR)
	{
		FileRead(file_handle,&anim_count,sizeof(anim_count));
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
}

//---------------------------------------------------------------
