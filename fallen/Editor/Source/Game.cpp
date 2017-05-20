// Game.cpp
// Guy Simmons, 24th March 1997.

#include	"Editor.hpp"
#include	"Thing.h"
#include	"Structs.h"

#define	GAME_OKAY		(1<<0)
#define	GAME_EDITOR		(1<<1)
#define	GAME_RECORD		(1<<2)

#define	LEFT_WALK		0
#define	RIGHT_WALK		1
#define	LEFT_STAND		2
#define	RIGHT_STAND		3
#define	LEFT_JUMP		4
#define	LEFT_HIT		5

#define	CLIP256(x)		(x>255?255:x)

extern	SLONG	calc_height_at(SLONG x,SLONG z);

UBYTE					palette[768];
ULONG					game_flags;

void	handle_game_control(void);
void	handle_character_controls(void);

//---------------------------------------------------------------

void	init_game(void)
{
}

//---------------------------------------------------------------




extern	void	setup_game(void);
#ifdef	POO
void	game(void)
{
	UWORD					map_chap;
	SLONG					c0,c1,
							test_x;
	struct KeyFrameElement	*the_element,
							*the_next_element;
	struct MapThing			*t_thing;
	struct Matrix33			r_matrix;


	setup_game();

	//init_thing();


#define	CHAP_X		(HALF_ELE_SIZE+(512<<ELE_SHIFT))
#define	CHAP_Y		(HALF_ELE_SIZE+(64<<ELE_SHIFT))
#define	CHAP_Z		(HALF_ELE_SIZE+(6<<ELE_SHIFT))

	map_chap		=	find_empty_map_thing();
	t_thing			=	TO_MTHING(map_chap);
//	t_thing->X		=	CHAP_X;
//	t_thing->Y		=	CHAP_Y;
//	t_thing->Z		=	CHAP_Z;

	t_thing->X		=	map_things[background_prim].X;
	t_thing->Y		=	map_things[background_prim].Y-100;
	t_thing->Z		=	(HALF_ELE_SIZE+(6<<ELE_SHIFT));

	add_thing_to_edit_map(t_thing->X>>ELE_SHIFT,t_thing->Y>>ELE_SHIFT,map_chap);
	t_thing->AngleX	=	0;
	t_thing->AngleY	=	0;
	t_thing->AngleZ	=	0;
	t_thing->Type	=	MAP_THING_TYPE_ROT_MULTI;
	t_thing->IndexOther	=	1;

	cam_chap			=	find_empty_map_thing();
	cam_thing			=	TO_MTHING(cam_chap);
	cam_thing->X		=	CHAP_X;
	cam_thing->Y		=	CHAP_Y;
	cam_thing->Z		=	CHAP_Z-2;
	cam_thing->AngleX	=	0;
	cam_thing->AngleY	=	0;
	cam_thing->AngleZ	=	0;
	cam_thing->Type		=	0;
	engine.Scale=266;
	test_x			=	t_thing->X;
	while(SHELL_ACTIVE && (game_flags&GAME_OKAY))
	{
//		move_thing_on_cells(map_chap,test_x,t_thing->Y,t_thing->Z);
		handle_game_control();

		if(Keys[KB_LEFT])
		{
			anim_offset_x	-=	2;
		}
		else if(Keys[KB_RIGHT])
		{
			anim_offset_x	+=	2;
		}
		if(Keys[KB_UP])
		{
			anim_offset_y	-=	2;
		}
		else if(Keys[KB_DOWN])
		{
			anim_offset_y	+=	2;
		}
		if(Keys[KB_DEL])
		{
			anim_angle_zx	-=	4;
		}
		else if(Keys[KB_PGDN])
		{
			anim_angle_zx	+=	4;
		}

		if(Keys[KB_HOME])
		{
			anim_angle_zy	-=	4;
		}
		else if(Keys[KB_END])
		{
			anim_angle_zy	+=	4;
		}

//		motion	=	(LEFT_WALK+1);

		if(motion!=(LEFT_JUMP+1) && motion!=(LEFT_HIT+1))
		{
			if(Keys[KB_X])
			{
				if(motion==(RIGHT_WALK+1))
				{
					test_x			+=	4;
//					t_thing->AngleY	=	1024;
				}
				else
				{
					motion			=	(RIGHT_WALK+1);
					queued_frame	=	anim_array[RIGHT_WALK];
				}
			}
			else if(motion==(RIGHT_WALK+1))
			{
				motion			=	(RIGHT_STAND+1);
				queued_frame	=	anim_array[RIGHT_STAND];
			}

			if(Keys[KB_Z])
			{
				if(motion==(LEFT_WALK+1))
				{
					test_x			-=	4;
//					t_thing->AngleY	=	-1024;
				}
				else
				{
					motion			=	(LEFT_WALK+1);
					queued_frame	=	anim_array[LEFT_WALK];
				}
			}
			else if(motion==(LEFT_WALK+1))
			{
				motion			=	(LEFT_STAND+1);
				queued_frame	=	anim_array[LEFT_STAND];
			}

			if(Keys[KB_SPACE])
			{
				motion			=	(LEFT_JUMP+1);
				queued_frame	=	anim_array[LEFT_JUMP];
			}
		}
		else
		{
			if(!next_frame->NextFrame)
			{
				if(Keys[KB_X])
				{
					motion			=	(LEFT_WALK+1);
					queued_frame	=	anim_array[LEFT_WALK];
				}
				else if(motion==(LEFT_JUMP+1))
				{
					motion			=	(LEFT_STAND+1);
					queued_frame	=	anim_array[LEFT_STAND];
				}
			}
		}

		if(Keys[KB_ENTER] && motion!=(LEFT_HIT+1))
		{
			motion	=	(LEFT_HIT+1);
			queued_frame	=	anim_array[LEFT_HIT];
		}

		if(current_anim)
		{
//			anim_tween	+=	16;
//			anim_tween	+=	256/(current_frame->TweenStep+1);
			anim_tween	+=	current_frame->TweenStep<<1;
			if(anim_tween>256)
			{
				anim_tween	-=	256;
				current_frame	=	next_frame;
				if(queued_frame)
				{
					next_frame		=	queued_frame;
					queued_frame	=	0;
				}
				else
				{
					if(next_frame->NextFrame)
						next_frame		=	next_frame->NextFrame;
				}
			}
		}

extern SLONG	count;
		if(LastKey==KB_R)
		{
			game_flags	|=	GAME_RECORD;
			count	=	0;
			LastKey	=	0;
		}

		SetWorkWindowBounds(0,0,WorkScreenWidth,WorkScreenHeight);

		ClearWorkScreen(0);

		if(LockWorkScreen())
		{
			if(current_anim)
			{
				if(current_frame)
				{
					t_thing->TweenStage	=	anim_tween;
					t_thing->AnimElements	=	current_frame->FirstElement;
					if(next_frame)
						t_thing->NextAnimElements	=	next_frame->FirstElement;
					else
						t_thing->NextAnimElements	=	t_thing->AnimElements;
				}

				set_game_camera(t_thing);
//				set_camera();
				draw_editor_map(1);
extern void	draw_map_thing(SLONG	map_thing);
				draw_map_thing(map_chap);
extern void	interface_thing(void);
//poo				interface_thing();
				render_view(0);
			}
			UnlockWorkScreen();
			ShowWorkScreen(DS_WAIT_VBI);
			if(game_flags&GAME_RECORD)
			{
extern void do_record_frame(UBYTE *screen,UBYTE *palette);
				if(LockWorkScreen())
				{
					do_record_frame(WorkScreen,CurrentPalette);
					UnlockWorkScreen();
				}
			}
		}
	}

	reset_game();
}
#endif

//---------------------------------------------------------------

void	handle_game_control(void)
{
	if(LastKey==KB_E)
	{
		if(Keys[KB_LCONTROL] || Keys[KB_RCONTROL])
			game_flags	|=	GAME_EDITOR;
		LastKey	=	0;
	}
	if(LastKey==KB_ESC)
		game_flags	=	0;

	if(game_flags&GAME_EDITOR)
	{
		editor_loop();
		game_flags	&=	~GAME_EDITOR;
//		LastKey		=	0;			// So we don't exit the game on exiting the editor.
	}
}


