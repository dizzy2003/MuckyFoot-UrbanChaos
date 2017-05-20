// KFrameEd.cpp
// Guy Simmons, 12th March 1997.


#include	"Editor.hpp"
#include	"FileReq.hpp"
#include	"c:\fallen\editor\headers\Prim.h"
#include	"c:\fallen\headers\FMatrix.h"
#include	"c:\fallen\headers\animtmap.h"
#include	"c:\fallen\headers\animate.h"
#include	"c:\fallen\headers\memory.h"
#include	"c:\fallen\headers\io.h"

// JCL
#include	"c:\fallen\ddengine\headers\Quaternion.h"
#include	"c:\fallen\headers\Hierarchy.h"

#define	CONTROLS_HEIGHT			400
#define	CONTROLS_WIDTH			300
#define	KEY_FRAME_IMAGE_SIZE	48

#define	VIDEO_MODE_PLAY			(1)
#define	VIDEO_MODE_PAUSE		(2)
#define	VIDEO_MODE_NEXT_FRAME	(3)
#define	VIDEO_MODE_PREV_FRAME	(4)
#define	VIDEO_MODE_PAUSE_BACK	(5)


UBYTE	re_center_flags[400];
UBYTE	unused_flags[400];


void	load_recenter_flags(CBYTE *filename)
{
	SLONG	c0=0;
	SLONG	size;
	CBYTE	name[100];
	MFFileHandle	file_handle;

	strcpy(name,filename);
	while(name[c0]!='.')
	{
		c0++;
	}
	name[c0+1]='r';
	name[c0+2]='e';
	name[c0+3]='c';


	file_handle	=	FileOpen(name);
	if(file_handle!=FILE_OPEN_ERROR)
	{
		FileRead(file_handle,&size,sizeof(size));
		FileRead(file_handle,&re_center_flags[0],size);
		if(size>300)
		FileRead(file_handle,&unused_flags[0],size);

		FileClose(file_handle);
	}
	else
	{
		memset(&re_center_flags[0],0,400);
	}


}

void	save_recenter_flags(CBYTE *filename)
{
	SLONG	c0=0;
	SLONG	size;
	CBYTE	name[100];
	MFFileHandle	file_handle;

	strcpy(name,filename);
	while(name[c0]!='.')
	{
		c0++;
	}
	name[c0+1]='r';
	name[c0+2]='e';
	name[c0+3]='c';


	file_handle	=	FileCreate(name,TRUE);
	if(file_handle!=FILE_OPEN_ERROR)
	{
		size=400;
		FileWrite(file_handle,&size,sizeof(size));
		FileWrite(file_handle,&re_center_flags[0],size);
		FileWrite(file_handle,&unused_flags[0],size);
		FileClose(file_handle);
	}




}



#include	"KFDef.c"


KeyFrameEditor		*the_editor;
void	draw_key_frames(void);
void	draw_anim_frames(void);
void	draw_all_anims_box(void);
//void	test_draw(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,struct Matrix33 *rot_mat);
void	set_key_framer_camera();
void	set_key_framer_camera_plan();
void	build_tween_matrix(struct Matrix33 *mat,struct CMatrix33 *cmat1,struct CMatrix33 *cmat2,SLONG tween);

struct KeyFrameChunk 	edit_chunk1,edit_chunk2;

struct KeyFrameElement	*elements_bank1=0,*elements_bank2=0;

//extern SLONG					test_chunk->KeyFrameCount;
extern struct KeyFrameChunk 	*test_chunk;
extern struct KeyFrameElement	*the_elements;
//extern void	load_multi_vue(struct	KeyFrameChunk *the_chunk,float scale);
extern void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
extern void matrix_transform(struct Matrix31* result, struct Matrix33* trans,struct  Matrix31* mat2);
extern void matrix_transform_small(struct Matrix31* result, struct Matrix33* trans,struct  SMatrix31* mat2);

// Used by fudgy centering bit.
extern SLONG		x_centre,
					y_centre,
					z_centre;

void	drawkeyframebox(UWORD multi_object,EdRect *bounds_rect,struct KeyFrame *the_frame,struct Matrix33 *r_matrix,SLONG person_id);

static	UWORD	local_object_flags=0;

#define	CTRL_LOAD_BUTTON			1
#define	CTRL_FRAME_SLIDER			2
#define CTRL_NEW_CHAR_BUTTON		3
#define	CTRL_CHAR_SLIDER			3
#define	CTRL_CHAR_NAME_EDIT			4
#define	CTRL_LOAD_BUTTON2			7
#define	CTRL_DRAW_BOTH				8
#define	CTRL_FLIP_BANK				9
#define	CTRL_SYNC_BOTH				10
#define	CTRL_NEXT_MESH				11
#define	CTRL_PREV_MESH				12

#define	CTRL_RE_CENTER_ALL			13
#define	CTRL_RE_CENTER_START		14
//#define	CTRL_RE_CENTER_END			15
#define	CTRL_RE_CENTER_NONE			15
#define	CTRL_RE_CENTER_XZ			16
#define	CTRL_RE_CENTER_XZ_START		17

#define	CTRL_MOVE_SEPARATELY		18


#define	CTRL_ANIM_FRAME_SLIDER		1
#define	CTRL_ANIM_FPS_TEXT			2
#define	CTRL_ANIM_FPS_SLIDER		3
#define	CTRL_ANIM_NEW_ANIM_BUTTON	4
#define	CTRL_ANIM_ALL_ANIMS_SLIDER	5
#define	CTRL_ANIM_NAME_EDIT			6
#define	CTRL_ANIM_LOOP_SELECT		7
#define	CTRL_ANIM_LORES_TEST		8
#define	CTRL_ANIM_SAVE_ANIMS		9

#define	CTRL_ANIM_SUPER_SMOOTH		10
#define	CTRL_ANIM_JUST_KEYS			11
#define	CTRL_PAINT_MESH				12

#define	CTRL_VIDEO_PREV				13
#define	CTRL_VIDEO_PLAY				14
#define	CTRL_VIDEO_NEXT				15
#define	CTRL_VIDEO_PAUSE			16
#define	CTRL_FIGHT_HEIGHT_PLUS		17
#define	CTRL_FIGHT_HEIGHT_MINUS		18
#define	CTRL_FIGHT_DAMAGE_PLUS		19
#define	CTRL_FIGHT_DAMAGE_MINUS		20
#define	CTRL_FIGHT_TWEEN0			21
#define	CTRL_FIGHT_TWEEN1			22
#define	CTRL_FIGHT_TWEEN2			23
#define	CTRL_FIGHT_TWEEN3			24
#define	CTRL_ANIM_USE_QUATERNIONS	25
#define	CTRL_ANIM_TWEAK_SLIDER		26

#define CTRL_ANIM_DISTANCE_SLIDER	27
#define CTRL_ANIM_FLIP_1			28
#define CTRL_ANIM_FLIP_2			29
#define CTRL_ANIM_RESET_VIEW		30
#define	CTRL_ANIM_SAVE_GAME_ANIM	31


#define	RE_CENTER_NONE		0
#define	RE_CENTER_ALL		1
#define	RE_CENTER_START		2
#define	RE_CENTER_END		3
#define	RE_CENTER_XZ		4
#define	RE_CENTER_XZ_START	5

SLONG	CurrentMesh;

ControlDef	kframe_def[]	=	
{
	{	BUTTON,			0,	"Load Anim Into Bank 1",	2,	CONTROLS_HEIGHT-(20+KEY_FRAME_IMAGE_SIZE+40),	0,						10			},
	{	H_SLIDER,		0,	"",						2,	CONTROLS_HEIGHT-20,								6*KEY_FRAME_IMAGE_SIZE,	0			},
//	{	BUTTON,			0,	"New Character",		2,	4,		0,		0			},
	{	V_SLIDER,		0,	"",						104,20,		0,		200			},
	{	EDIT_TEXT,		0,	"",						2,	230,	70,		0			},
	{	STATIC_TEXT,	0,	"Current Prim : ",		2,	246,	0,		0			},
	{	PULLDOWN_MENU,	0,	"Multi Prim",			2,	262,	70,		0,	0		},
	{	BUTTON,			0,	"Load Anim Into Bank 2",	2,	CONTROLS_HEIGHT-(20+KEY_FRAME_IMAGE_SIZE+20),	0,						10			},
	{	CHECK_BOX,		0,	"Don't Draw Both",		200,	230,		0,		0			},
	{	BUTTON,			0,	"Flip Bank",		200,	250,		0,		0			},
	{	BUTTON,			0,	"Sync Both",		200,	280,		0,		0			},
	{	BUTTON,			0,	"Next Mesh",		130,	230,	0,		0			},
	{	BUTTON,			0,	"Prev Mesh",		130,	245,	0,		0			},
	
	{	BUTTON,			0,	"Re-Cen All",	130,	260,	0,		0			},
	{	BUTTON,			0,	"Re-Cen Start",	130,	275,	0,		0			},
//	{	BUTTON,			0,	"Re-Cen End",	130,	290,	0,		0			},
	{	BUTTON,			0,	"No Center",	230,	305,	0,		0			},
	{	BUTTON,			0,	"Re-Cen X&Z",	130,	305,	0,		0			},
	{	BUTTON,			0,	"Re-Cen X&Z(Start)",	130,	290,	0,		0			},
	{	CHECK_BOX,		0,	"Move Separately",		100,	265,		0,		0			},

	{	0															   				}
};



//---------------------------------------------------------------

ControlDef	content_def[]	=
{
	{	H_SLIDER,		0,	"",						0,	CONTROLS_HEIGHT-16,	6*KEY_FRAME_IMAGE_SIZE,	0			},
	{	STATIC_TEXT,	0,	"FPS",					2,	CONTROLS_HEIGHT-(16+KEY_FRAME_IMAGE_SIZE+16),	0,		10		  	},
	{	H_SLIDER,		0,	"",						22,	CONTROLS_HEIGHT-(16+KEY_FRAME_IMAGE_SIZE+18),	100,	0			},
	{	BUTTON,			0,	"New Anim",				2,	4,		0,		0			},
	{	V_SLIDER,		0,	"",						104,20,		0,		200			},
	{	EDIT_TEXT,		0,	"",						2,	230,	70,		0			},
	{	CHECK_BOX,		KB_L,	"Looped",			2,	246,	70,		0			},
	{	BUTTON,			0,	"Lo-res test",			2,	260,	70,		0			},
	{	BUTTON,			0,	"Save Anims",			2,	276,	70,		0			},
	{	CHECK_BOX,		0,	"Super Smooth",			182,	260,	70,		0			},
	{	CHECK_BOX,		0,	"Just Keys",			182,	276,	70,		0			},
	{	BUTTON,			0,	"Paint Mesh",			2,	296,	70,		0			},
	{	BUTTON,			0,	"$26",					100+2,	296,	10,		0			},
	{	BUTTON,			0,	"$25",					100+30,	296,	10,		0			},
	{	BUTTON,			0,	"$27",					100+60,	296,	10,		0			},
	{	BUTTON,			0,	"$28",					100+90,	296,	10,		0			},
	{	BUTTON,			0,	"+",					300+90,	20,	10,		0			},
	{	BUTTON,			0,	"-",					300+90,	40,	10,		0			},
	{	BUTTON,			0,	"+",					200+90,	10,	10,		0			},
	{	BUTTON,			0,	"-",					270+90,	10,	10,		0			},
	{	BUTTON,			0,	"0",					200+30,	296,	10,		0			},
	{	BUTTON,			0,	"1",					200+50,	296,	10,		0			},
	{	BUTTON,			0,	"2",					200+70,	296,	10,		0			},
	{	BUTTON,			0,	"3",					200+90,	296,	10,		0			},
	{	CHECK_BOX,		0,	"No Quaternions",		280,	260,	70,		0			},
	{	H_SLIDER,		0,	"Tweak",				260,	280,	100,	0			},
	{	H_SLIDER,		0,	"Distance",				200,	240,	100,	0			},
	{	CHECK_BOX,		0,	"Flip 1",				380,	240,	70,		0			},
	{	CHECK_BOX,		0,	"Flip 2",				380,	260,	70,		0			},
	{	BUTTON,			0,	"Reset View",			380,	280,	70,		0			},
	{	BUTTON,			0,	"SAVE ALL File",		60,	276,	70,		0			},

	{	0																			}
};

#define	CTRL_KEY_SLIDER			1
#define	CTRL_KEY_SIZE_PLUS		2
#define	CTRL_KEY_SIZE_MINUS		3

ControlDef	key_def[]	=
{
	{	H_SLIDER,		0,	"",						0,	0,	4*KEY_FRAME_IMAGE_SIZE,	0	},
	{	BUTTON,			0,	"+",			40+4*KEY_FRAME_IMAGE_SIZE,	0,	12,		0			},
	{	BUTTON,			0,	"-",			70+4*KEY_FRAME_IMAGE_SIZE,	0,	12,		0			},
	{	0																			}
};

//---------------------------------------------------------------

KeyFrameEditor::~KeyFrameEditor()
{
//	SaveAllAnims(&test_chunk);
	reset_anim_stuff();
}

//---------------------------------------------------------------

class	KeyFrameList	:	public	EditorModule
{

	private:
		KeyFrameEditor		*ParentEdit;
		EdRect				KeyFrameRect;
		SLONG				KeyFrameSize;		

	protected:

	public:
							KeyFrameList(KeyFrameEditor *parent,SLONG x,SLONG y,Anim *anim,SLONG max);
		void				DrawContent(void);
		void				DrawKeyFrames(void);
		void				HandleContentClick(UBYTE flags,MFPoint *clicked_point);
		void				HandleModule(void);
		void				HandleKeyControl(ULONG control_id);
		Anim				*TheAnim;
		ULONG				StartPos;
		ControlSet			KeyControls;

};

KeyFrameList		*the_key_list[5];

KeyFrameList::KeyFrameList(KeyFrameEditor *parent,SLONG x,SLONG y,Anim *anim,SLONG max)
{
extern	void	add_module(EditorModule *the_module);
	ParentEdit=parent;
	add_module(this);
	this->SetupWindow	(
					"Key Frames",
					(HAS_GROW),
					x,
					y,
					790,
					KEY_FRAME_IMAGE_SIZE+30-5
				);
	SetContentColour(CONTENT_COL);

	KeyControls.InitControlSet(key_def);

	((CHSlider*)KeyControls.GetControlPtr(CTRL_KEY_SLIDER))->SetValueRange(0,max);
	((CHSlider*)KeyControls.GetControlPtr(CTRL_KEY_SLIDER))->SetCurrentValue(0);
//	((CHSlider*)KeyControls.GetControlPtr(CTRL_KEY_SLIDER))->SetUpdateFunction();

	KeyFrameRect.SetRect(0,20,ContentWidth(),ContentHeight()-20);

	TheAnim=anim;

	StartPos=0;

	KeyFrameSize=KEY_FRAME_IMAGE_SIZE-5;


}

void	KeyFrameList::HandleKeyControl(ULONG  control_id)
{
	switch(control_id)
	{
		case	CTRL_KEY_SLIDER:
			break;
		case	CTRL_KEY_SIZE_PLUS:
			KeyFrameSize+=4;
			if(KeyFrameSize>300)
				KeyFrameSize=300;
			break;
		case	CTRL_KEY_SIZE_MINUS:
			KeyFrameSize-=4;
			if(KeyFrameSize<10)
				KeyFrameSize=10;
			break;
	}

}
void	KeyFrameList::HandleModule(void)
{
	ParentEdit->HandleModuleKeys();
}

void	KeyFrameList::DrawContent(void)
{
	SetContentDrawArea();
	ClearContent();
	DrawKeyFrames();
	KeyControls.ControlSetBounds(GetContentRect());
	KeyControls.DrawControlSet();

//	DrawAnimFrames(CurrentAnim[Bank],TRUE);
}

#define	KEY_OFFSET_Y		(16)
void	KeyFrameList::DrawKeyFrames(void)
{
	CBYTE				text[64];
	SLONG				c0,
						first_frame,
						max_frames;
	EdRect				frame_rect,
						hilite_rect;
	MFPoint				mouse_point;
	struct Matrix33		r_matrix;
	struct KeyFrame		*current_frame;
	if(TheAnim)
		current_frame	=	TheAnim->GetFrameList();

	ParentEdit->SetSelectedFrame(NULL);

	if(test_chunk->MultiObject)
	{
		SLONG	x=0,y=0;
		rotate_obj(ParentEdit->GetAnimAngleX(),ParentEdit->GetAnimAngleY(),0,&r_matrix);
		max_frames	=	ContentWidth()/KeyFrameSize;
		if((ContentHeight()-20)>=KeyFrameSize*2)
			max_frames*=(ContentHeight()-20)/KeyFrameSize;

		first_frame	= ((CHSlider*)KeyControls.GetControlPtr(CTRL_KEY_SLIDER))->GetCurrentValue();
		if((first_frame+max_frames)>test_chunk->KeyFrameCount)
			max_frames	=	(test_chunk->KeyFrameCount-first_frame)+1;

		if(TheAnim)
		{
			SLONG	temp;
			temp	=	(TheAnim->GetFrameCount()-first_frame);
			if(temp<max_frames)
				max_frames=temp;

			if(first_frame)
			{
				for(c0=0;c0<first_frame;c0++)
				{
					if(current_frame)
						current_frame=current_frame->NextFrame;
					else
						return;
				}
			}
		}

		for(c0=0;c0<max_frames;c0++)
		{
			//WindowControls.SetControlDrawArea();
			SetContentDrawArea();
//md

			frame_rect.SetRect	(
									ContentLeft()+x+2,
									ContentTop()+y+2+KEY_OFFSET_Y,
									KeyFrameSize,
									KeyFrameSize
								);

//			frame_rect.SetRect	(0,0,400,400);
			hilite_rect	=	frame_rect;
			//frame_rect.OffsetRect(ControlsLeft(),ControlsTop());
			mouse_point.X	=	MouseX;//-ContentLeft();
			mouse_point.Y	=	MouseY;//-ContentTop();

//			LogText(" mouse x %d y %d  frame x %d y %d \n",mouse_point.X,mouse_point.Y,ContentLeft()+x+2,ContentTop()+y+2+KEY_OFFSET_Y);
			if(frame_rect.PointInRect(&mouse_point))
			{
				SetWorkWindowBounds(0,0,WorkScreenWidth,WorkScreenHeight);
				hilite_rect.FillRect(HILITE_COL);
//				LogText(" highlight \n");
				if(TheAnim)
					ParentEdit->SetSelectedFrame(current_frame);
			}
			
			if(TheAnim==0)
				current_frame=&test_chunk->KeyFrames[first_frame+c0];

			//ParentEdit->DrawKeyFrame(test_chunk->MultiObject,&frame_rect,current_frame,&r_matrix);
			drawkeyframebox(test_chunk->MultiObject,&frame_rect,current_frame,&r_matrix,ParentEdit->GetPersonID());
			{
				CBYTE	str[100];
				sprintf(str,"%d",current_frame->FrameID);
				QuickTextC(2,2,str,WHITE_COL);
			}
			x+=KeyFrameSize;
			if(x>ContentWidth()-KeyFrameSize)
			{
				x=0;
				y+=KeyFrameSize;
				if(y>ContentHeight()-KeyFrameSize)
					break;
			}
			if(TheAnim)
			{
				current_frame=current_frame->NextFrame;
				if(current_frame==0)
					break;
			}

//md			break;
		}
	}
}


void	KeyFrameList::HandleContentClick(UBYTE flags,MFPoint *clicked_point)
{
	ULONG		cleanup,
				update;
	SLONG		c0,
				first_frame,
				max_frames,
				x_diff,
				y_diff;
	Control		*current_control;
	EdRect		frame_rect,
				last_rect,
				temp_rect;
	MFPoint		local_point;
	struct KeyFrame		*current_frame;
	struct KeyFrame		*selected_frame;

	KeyFrameRect.SetRect(ContentLeft(),ContentTop()+20,ContentWidth(),ContentHeight()-20);

//	WindowControls.SetControlDrawArea();
	local_point	=	*clicked_point;
//	WindowControls.GlobalToLocal(&local_point);
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
//			LogText(" click in box x %d y %d  box= (%d,%d,%d,%d) \n",local_point.X,local_point.Y,KeyFrameRect.GetLeft(),KeyFrameRect.GetTop(),KeyFrameRect.GetWidth(),KeyFrameRect.GetHeight());
			if(KeyFrameRect.PointInRect(&local_point))
			{
				SLONG	x=0,y=0;
				// Find out which frame has been selected.
				if(TheAnim)
					current_frame	=	TheAnim->GetFrameList();

				selected_frame	=	0;
				max_frames		=	ContentWidth()/KeyFrameSize;
				if(ContentHeight()>=KeyFrameSize*2)
					max_frames*=ContentHeight()/KeyFrameSize;
				first_frame	= ((CHSlider*)KeyControls.GetControlPtr(CTRL_KEY_SLIDER))->GetCurrentValue();

				if((first_frame+max_frames)>test_chunk->KeyFrameCount)
					max_frames	=	(test_chunk->KeyFrameCount-first_frame)+1;
				if(TheAnim)
				{
					SLONG	temp;
					temp	=	(TheAnim->GetFrameCount()-first_frame);
					if(temp<max_frames)
						max_frames=temp;
				}
				for(c0=0;c0<max_frames&&test_chunk->KeyFrameCount;c0++)
				{

					frame_rect.SetRect	(
											ContentLeft()+(x)+2,
											ContentTop()+y+2+KEY_OFFSET_Y,
											KeyFrameSize,
											KeyFrameSize
										);
					if(frame_rect.PointInRect(&local_point))
					{
						if(TheAnim==0)
						{
							selected_frame	=	&test_chunk->KeyFrames[first_frame+c0];
						}
						else
						{
							selected_frame	=	current_frame;
						}
						break;
					}
					x+=KeyFrameSize;
					if(x>ContentWidth()-KeyFrameSize)
					{
						x=0;
						y+=KeyFrameSize;
						if(y>ContentHeight()-KeyFrameSize)
						{
							break;
						}
					}
					if(TheAnim)
					{
						current_frame=current_frame->NextFrame;
						if(current_frame==0)
						{
							break;
						}
					}
				}

				// Allow selected frame to be dragged around.
				if(selected_frame>=0)
				{
					SLONG drop;
					//
					// drag frames out of a key window, could be main one or scratch pad
					//
					drop=ParentEdit->DragAndDropFrame(selected_frame,frame_rect.GetLeft(),frame_rect.GetTop(),frame_rect.GetWidth(),frame_rect.GetHeight(),clicked_point,0);

					// never want to delete from main list, or scratch list unless draging to bin
					if(drop==0)
					{
						if(this==the_key_list[0])
						{
							the_key_list[0]->TheAnim->RemoveKeyFrame(selected_frame);
						}

					}
				}
			}
			else
			{
				current_control	=	KeyControls.GetControlList();
				local_point.X-=ContentLeft();
				local_point.Y-=ContentTop();
				while(current_control)
				{
					if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(&local_point))
					{
						// Handle control.
						current_control->TrackControl(&local_point);
						HandleKeyControl(current_control->TrackControl(&local_point));

						// Tidy up display.
						if(LockWorkScreen())
						{
//							KeyControls.DrawControlSet();
							DrawContent();
							UnlockWorkScreen();
						}
						ShowWorkWindow(0);
					}
					current_control	=	current_control->GetNextControl();
				}
			}

			break;
		case	RIGHT_CLICK:
			break;
	}
}



void	KeyFrameEditor::SetupModule(void)
{
	KeyFrameEdDefaults	the_defaults;

	the_key_list[0]		=	new KeyFrameList(this,0,450,0,60);
	the_key_list[1]		=	new KeyFrameList(this,0,525,0,650);


	memset(&PersonBits[0],0,MAX_BODY_BITS);
	the_editor			=	this;
	the_defaults.Left	=	20;
	the_defaults.Top	=	20;
	the_defaults.Width	=	760;
	the_defaults.Height	=	345;
	SetupWindow	(
					"Key Frame Editor",
					(HAS_TITLE|HAS_CONTROLS),
					the_defaults.Left,
					the_defaults.Top,
					the_defaults.Width,
					the_defaults.Height
				);
	SetContentColour(CONTENT_COL);
	SetControlsHeight(CONTROLS_HEIGHT);
	SetControlsWidth(CONTROLS_WIDTH);

	CreateKeyFrameTabs();

	AnimAngleX[0]		=	0;
	AnimAngleY[0]		=	-512;
	AnimAngleZ[0]		=	0;
	AnimAngleX[1]		=	0;
	AnimAngleY[1]		=	512;
	AnimAngleZ[1]		=	0;

	AnimGlobalAngleX	=	0;
	AnimGlobalAngleY	=	0;
	AnimGlobalOffsetX	=	0;
	AnimGlobalOffsetY	=	0;

	AnimOffsetX[0]		=	0;
	AnimOffsetY[0]		=	0;
	AnimOffsetZ[0]		=	0;

	AnimOffsetX[1]		=	100;
	AnimOffsetY[1]		=	0;
	AnimOffsetZ[1]		=	0;

	AnimScale			=	2000; //296;
	AnimTween[0]		=	0;
	AnimTween[1]		=	0;
//	CurrentElement	=	0;
	CurrentFrame[0]	=	0;
	CurrentFrame[1]	=	0;

	Bank=0;

	//TestChunk		=	(KeyFrameChunk*)MemAlloc(sizeof(KeyFrameChunk));
//	TheElements		=	(KeyFrameElement*)MemAlloc(MAX_NUMBER_OF_ELEMENTS*sizeof(KeyFrameElement));
	//TestChunk->MultiObject	=	0;

	setup_anim_stuff();

	AnimList[0]		=	NULL;
	AnimList[1]		=	NULL;

	AnimCount[0]		=	0;
	AnimCount[1]		=	0;

	CurrentAnim[0]		=	NULL;
	CurrentAnim[1]		=	NULL;

	PlayingAnim[0]		=	0;
	PlayingAnim[1]		=	0;

	Flags			=	0;
	SelectedFrame	=	NULL;

	KeyFrameRect.SetRect(2,CONTROLS_HEIGHT-(20+KEY_FRAME_IMAGE_SIZE+4),(6*KEY_FRAME_IMAGE_SIZE)+2,KEY_FRAME_IMAGE_SIZE+2);
	((CHSlider*)WindowControls.GetControlPtr(CTRL_FRAME_SLIDER))->SetValueRange(0,test_chunk->KeyFrameCount);
	((CHSlider*)WindowControls.GetControlPtr(CTRL_FRAME_SLIDER))->SetUpdateFunction(draw_key_frames);

	AnimControls.InitControlSet(content_def);
	AnimFrameRect.SetRect(0,CONTROLS_HEIGHT-(16+KEY_FRAME_IMAGE_SIZE+4),(9*KEY_FRAME_IMAGE_SIZE)+2,KEY_FRAME_IMAGE_SIZE+2);
	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetUpdateFunction(draw_anim_frames);
//	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetCurrentValue(0);
	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FPS_SLIDER))->SetValueRange(0,60);
	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FPS_SLIDER))->SetCurrentValue(20);

	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_TWEAK_SLIDER))->SetValueRange(0,255);
	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_TWEAK_SLIDER))->SetCurrentValue(128);

	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_DISTANCE_SLIDER))->SetValueRange(0,255);
	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_DISTANCE_SLIDER))->SetCurrentValue(100);

	((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->SetUpdateFunction(draw_all_anims_box);
	((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->SetFlags(CONTROL_INACTIVE);
	((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->SetEditString("No Anim");
	AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_DESELECTED);
	AnimControls.SetControlState(CTRL_DRAW_BOTH,CTRL_DESELECTED);
	AnimControls.SetControlState(CTRL_MOVE_SEPARATELY,CTRL_DESELECTED);

	((CEditText*)WindowControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->SetFlags(CONTROL_INACTIVE);
	((CEditText*)WindowControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->SetEditString("No Name");

	AllAnimsRect.SetRect(2,20,100,200);
	CharactersRect.SetRect(2,20,100,200);
	BodyPartRect.SetRect(150,20,100,200);

	CurrentCharacter	=	&TestCharacter;

	SpeedFlag=0;
	QuaternionFlag=0;
	Flip1 = 0;
	Flip2 = 0;
	FightColBank=0;
	FightingColPtr=0;
	PersonID=0;
	DontDrawBoth=0;
	MoveSeparately=0;
//	LoadKeyFrameChunks();
//	LoadAllAnims();
}

//---------------------------------------------------------------

void	KeyFrameEditor::CreateKeyFrameTabs(void)
{
	WindowControls.InitControlSet(kframe_def);
}

//---------------------------------------------------------------

void	KeyFrameEditor::DestroyKeyFrameTabs(void)
{
}

//---------------------------------------------------------------

void	KeyFrameEditor::DrawContent(void)
{
	SetContentDrawArea();
	ClearContent();

	engine.ShowDebug=	0;
	set_key_framer_camera();

	if(CurrentAnim[Bank])
	{
		DoCurrentAnim();
	}
	if(DontDrawBoth == 0)
	{
		if (CurrentAnim[(Bank == 0) ? 1 : 0])
		{
			SLONG	old_bank=Bank;
			if(Bank==0)
				SetAnimBank(1);
			else
				SetAnimBank(0);

			DoCurrentAnim();

			SetAnimBank(old_bank);
		}
	}
	render_view(0);
	engine.ShowDebug=	1;

	if(!ShiftFlag)
	{
		SetContentDrawArea();
		AnimControls.ControlSetBounds(GetContentRect());
		AnimControls.DrawControlSet();

		DrawAllAnimsBox();

//	if(CurrentFrame[Bank]&&CurrentFrame[Bank]->FirstElement&&CurrentFrame[Bank]->PrevFrame) //MD
		DrawAnimFrames(CurrentAnim[Bank],TRUE);
	}
}

//---------------------------------------------------------------
extern	CBYTE	*body_part_names[];

void	KeyFrameEditor::DrawPeopleTypes(void)
{
	SLONG	x,y,c0;
	EdRect		name_rect;
//	MFPoint		mouse_point;

	x=CharactersRect.GetLeft();
	y=CharactersRect.GetTop();
	x+=3;
	y+=3;
	for(c0=0;c0<15;c0++)
	{
		/*
		mouse_point.X	=	MouseX;
		mouse_point.Y	=	MouseY;

		mouse_point.X -= ContentLeft();
		mouse_point.Y -= ContentTop();
		*/

//		GlobalToLocal(&mouse_point);
//		mouse_point.X -= 

		name_rect.SetRect(x-1,y-1,CharactersRect.GetWidth()-2,13);
		if(c0==PersonID)
		{
			name_rect.FillRect(LOLITE_COL);
		}
/*
		if(name_rect.PointInRect(&mouse_point))
		{
			name_rect.FillRect(HILITE_COL);
		}
*/

		test_chunk->PeopleNames[c0][PEOPLE_NAME_SIZE-1]=0;
		QuickTextC(x,y,&test_chunk->PeopleNames[c0][0],0);
		y+=13;
	}

	x=BodyPartRect.GetLeft();
	y=BodyPartRect.GetTop();
	x+=3;
	y+=3;
	for(c0=0;c0<MAX_BODY_BITS;c0++)
	{
		CBYTE	str[100];

		if(body_part_names[c0]==0)
			break;
		sprintf(str,"%s %d",body_part_names[c0],PersonBits[c0]);
		QuickTextC(x,y,str,0);
		y+=13;
	}

}

void	KeyFrameEditor::DrawControls(void)
{
	WindowControls.DrawControlSet();

	CharactersRect.FillRect(ACTIVE_COL);
	CharactersRect.HiliteRect(LOLITE_COL,HILITE_COL);

	BodyPartRect.FillRect(ACTIVE_COL);
	BodyPartRect.HiliteRect(LOLITE_COL,HILITE_COL);

	 DrawKeyFrames();
	 DrawPeopleTypes();
}

//---------------------------------------------------------------

static ControlDef		popup_def	=	{	POPUP_MENU,	0,	""};

//
// Handle clicks in the RHS of the keyframe editor (known as the content area)
//
void	KeyFrameEditor::HandleContentClick(UBYTE flags,MFPoint *clicked_point)
{
	ULONG				cleanup,
						control_id,
						update;
	SLONG				c0,
						first_frame,
						max_frames,
						x_diff,
						y_diff;
	Anim				*temp_anim;
	Control				*current_control;
	CPopUp				*the_control	=	0;
	EdRect				frame_rect,
						last_rect,
						temp_rect;
	MFFileHandle		buffer_file;
	MFPoint				local_point;
	struct KeyFrame		*current_frame,
						*selected_frame;


	AnimControls.SetControlDrawArea();
	local_point	=	*clicked_point;
	AnimControls.GlobalToLocal(&local_point);
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			if(AnimFrameRect.PointInRect(&local_point) && CurrentAnim[Bank]) 
			{
				//
				// Left click on keyframe (on current playing animation) to drag and drop it somewhere
				//
				selected_frame	=	NULL;
				max_frames	=	AnimFrameRect.GetWidth()/KEY_FRAME_IMAGE_SIZE;
				first_frame	=	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->GetCurrentValue();
				if((first_frame+max_frames)>CurrentAnim[Bank]->GetFrameCount())
					max_frames	=	(CurrentAnim[Bank]->GetFrameCount()-first_frame);
				current_frame	=	CurrentAnim[Bank]->GetFrameList();
				if(current_frame)
				{
					for(c0=0;c0<first_frame;c0++)
					{
						current_frame	=	current_frame->NextFrame;
					}
					for(c0=0;c0<max_frames&&current_frame;c0++)
					{
						WindowControls.SetControlDrawArea();
						frame_rect.SetRect	(
												AnimFrameRect.GetLeft()+(c0*KEY_FRAME_IMAGE_SIZE)+2,
												AnimFrameRect.GetTop()+2,
												KEY_FRAME_IMAGE_SIZE,
												KEY_FRAME_IMAGE_SIZE
											);
						if(frame_rect.PointInRect(&local_point))
						{
							selected_frame	=	current_frame;
							break;
						}
						current_frame	=	current_frame->NextFrame;
					}
					if(selected_frame>=0)
					{
						SLONG	drop;
						//
						// drag frame out of actual editor window
						//

						clicked_point->X-=300;
						drop=DragAndDropFrame(selected_frame,frame_rect.GetLeft(),frame_rect.GetTop(),frame_rect.GetWidth(),frame_rect.GetHeight(),clicked_point,0);
						if(drop==0||drop==1)
						{
							// drop =1 is back on itself
							// drop =2 is into scratch pad
							// drop =0 is bin
							CurrentAnim[Bank]->RemoveKeyFrame(selected_frame);
							((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetValueRange(0,CurrentAnim[Bank]->GetFrameCount()-1);
							AnimTween[Bank]		=	0;
							CurrentFrame[Bank]	=	0;
						}

					}
				}
			}
			else if(AllAnimsRect.PointInRect(&local_point))			   //this is the anim named list thing
			{
				CurrentAnim[Bank]	=	DrawAllAnimsBox();
				if(CurrentAnim[Bank])
				{
					/*
					if(the_key_list[0])
					{
						the_key_list[0]->TheAnim=CurrentAnim[Bank];
						((CHSlider*)the_key_list[0]->KeyControls.GetControlPtr(CTRL_KEY_SLIDER))->SetValueRange(0,the_key_list[0]->TheAnim->GetFrameCount()-1); 
						the_key_list[0]->DrawContent();
						ShowWorkScreen(0);
					}
					*/
					((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_LOOP_SELECT))->SetFlags((UBYTE)(((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_LOOP_SELECT))->GetFlags()&~CONTROL_INACTIVE));
					if(CurrentAnim[Bank]->GetAnimFlags()&ANIM_LOOP)
						AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_SELECTED);
					else
						AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_DESELECTED);

					((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->SetFlags((UBYTE)(((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->GetFlags()&~CONTROL_INACTIVE));
					((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->SetEditString(CurrentAnim[Bank]->GetAnimName());

					((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetValueRange(0,CurrentAnim[Bank]->GetFrameCount()-1); //added by MD to try and fix slider bug
					((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_TWEAK_SLIDER))->SetCurrentValue(CurrentAnim[Bank]->GetTweakSpeed());
				}
				else
				{
					((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_LOOP_SELECT))->SetFlags(CONTROL_INACTIVE);
					((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->SetFlags(CONTROL_INACTIVE);
					((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->SetEditString("No Anim");
				}
				RequestUpdate();
			}
			else
			{
				current_control	=	AnimControls.GetControlList();
				while(current_control)
				{
					if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(&local_point))
					{
						// Handle control.
						HandleAnimControl(current_control->TrackControl(&local_point));

						// Tidy up display.
						if(LockWorkScreen())
						{
							AnimControls.DrawControlSet();
							UnlockWorkScreen();
						}
						ShowWorkWindow(0);
					}
					current_control	=	current_control->GetNextControl();
				}
			}
			break;
		case	RIGHT_CLICK:
			if(AnimFrameRect.PointInRect(&local_point) && CurrentAnim[Bank]) 
			{
				//
				// Right click in keyframe (on current playing animation) to select it 
				//
				selected_frame	=	NULL;
				max_frames	=	AnimFrameRect.GetWidth()/KEY_FRAME_IMAGE_SIZE;
				first_frame	=	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->GetCurrentValue();
				if((first_frame+max_frames)>CurrentAnim[Bank]->GetFrameCount())
					max_frames	=	(CurrentAnim[Bank]->GetFrameCount()-first_frame);
				current_frame	=	CurrentAnim[Bank]->GetFrameList();
				if(current_frame)
				{
					for(c0=0;c0<first_frame;c0++)
					{
						current_frame	=	current_frame->NextFrame;
					}
					for(c0=0;c0<max_frames&&current_frame;c0++)
					{
						WindowControls.SetControlDrawArea();
						frame_rect.SetRect	(
												AnimFrameRect.GetLeft()+(c0*KEY_FRAME_IMAGE_SIZE)+2,
												AnimFrameRect.GetTop()+2,
												KEY_FRAME_IMAGE_SIZE,
												KEY_FRAME_IMAGE_SIZE
											);
						if(frame_rect.PointInRect(&local_point))
						{
							selected_frame	=	current_frame;
							break;
						}
						current_frame	=	current_frame->NextFrame;
					}
					if(selected_frame>=0)
					{
							CurrentFrame[Bank]=selected_frame;
					}
				}
			}
			else
			if(AllAnimsRect.PointInRect(&local_point))
			{
				//
				// Check for right click on anim list, for cut/copy/paste functions
				//
				temp_anim	=	DrawAllAnimsBox();

				local_point	=	*clicked_point;
				GlobalToLocal(&local_point);
				popup_def.ControlLeft	=	local_point.X+4;
				popup_def.ControlTop	=	local_point.Y-4;
				popup_def.TheMenuDef	=	anim_popup;
				the_control				=	new CPopUp(&popup_def);

				if(temp_anim)
				{
					the_control->SetItemState(1,CTRL_ACTIVE);
					the_control->SetItemState(2,CTRL_ACTIVE);
					the_control->SetItemState(4,CTRL_ACTIVE);
				}
				else
				{
					the_control->SetItemState(1,CTRL_INACTIVE);
					the_control->SetItemState(2,CTRL_INACTIVE);
				}
				if(Flags&GOT_ANIM_COPY)
					the_control->SetItemState(3,CTRL_ACTIVE);
				else
					the_control->SetItemState(3,CTRL_INACTIVE);

				//
				// Select Cut,Copy,Paste 
				//
				control_id				=	the_control->TrackControl(&local_point);
				switch(control_id>>8)
				{
					KeyFrame	*frame;
					case	1: //cut
						buffer_file	=	FileCreate("Editor\\EdSave\\scrap.dat",TRUE);
						if(buffer_file)
						{
							SaveAnim(buffer_file,temp_anim);
							FileClose(buffer_file);
						}
						frame	=	temp_anim->GetFrameList();
						while(frame)
						{
							//
							// add frame to anim[0]
							//
							AnimList[Bank]->AddKeyFrame(frame);

							frame=frame->NextFrame;
							if(frame==temp_anim->GetFrameList())
								frame=0;

						}
						DestroyAnim(temp_anim);
						Flags	|=	GOT_ANIM_COPY;
						break;
					case	2: //copy
						buffer_file	=	FileCreate("Editor\\EdSave\\scrap.dat",TRUE);
						if(buffer_file)
						{
							SaveAnim(buffer_file,temp_anim);
							FileClose(buffer_file);
						}
						frame	=	temp_anim->GetFrameList();
						while(frame)
						{
							//
							// add frame to anim[0]
							//
							AnimList[Bank]->AddKeyFrame(frame);

							frame=frame->NextFrame;
							if(frame==temp_anim->GetFrameList())
								frame=0;

						}
						Flags	|=	GOT_ANIM_COPY;
						break;
					case	3: //paste
						InsertAnim(temp_anim);
						buffer_file	=	FileOpen("Editor\\EdSave\\scrap.dat");
						if(buffer_file)
						{
							LoadAnim(buffer_file,CurrentAnim[Bank]);
							FileClose(buffer_file);
						}
						break;
					case	4: //toggle_unused
						{
							Anim	*current_anim;
							SLONG	index=0;


							if(AnimList[Bank])
							{
								current_anim	=	AnimList[Bank];
								while(current_anim!=temp_anim && current_anim)
								{
									index++;
									current_anim = current_anim->GetNextAnim();
								}
							}
							unused_flags[index]^=1;
						}
						break;
				}

				if(the_control)
				{
					delete	the_control;
				}
			}			
			break;
	}
}

//---------------------------------------------------------------
SLONG	KeyFrameEditor::DragAndDropFrame(KeyFrame *selected_frame,SLONG x,SLONG y,SLONG w,SLONG h,MFPoint *clicked_point,ULONG delete_flag)
{
	MFPoint		local_point;
	ULONG		cleanup;
	SLONG		update;

	SLONG		x_diff,
				y_diff;
	EdRect		last_rect,
				temp_rect;
	SetWorkWindowBounds(0,0,WorkScreenWidth,WorkScreenHeight);

	temp_rect.SetRect	(
							ControlsLeft()+x,
							ControlsTop()+y,
							w,
							h
						);
	x_diff	=	clicked_point->X-temp_rect.GetLeft();
	y_diff	=	clicked_point->Y-temp_rect.GetTop();
	last_rect.SetRect(0,0,0,0);
	cleanup	=	0;
	update	=	0;
	while(SHELL_ACTIVE && LeftButton)
	{
		temp_rect.SetRect(MouseX-x_diff,MouseY-y_diff,temp_rect.GetWidth(),temp_rect.GetHeight());
		if(temp_rect.GetLeft()<0)
			temp_rect.MoveRect(0,temp_rect.GetTop());
		if(temp_rect.GetTop()<0)
			temp_rect.MoveRect(temp_rect.GetLeft(),0);
		if(temp_rect.GetRight()>=WorkScreenWidth)
			temp_rect.MoveRect(WorkScreenWidth-temp_rect.GetWidth(),temp_rect.GetTop());
		if(temp_rect.GetBottom()>=WorkScreenHeight)
			temp_rect.MoveRect(temp_rect.GetLeft(),WorkScreenHeight-temp_rect.GetHeight());

		if	(
				temp_rect.GetLeft()!=last_rect.GetLeft() ||
				temp_rect.GetTop()!=last_rect.GetTop() ||
				temp_rect.GetRight()!=last_rect.GetRight() ||
				temp_rect.GetBottom()!=last_rect.GetBottom()
			)
		{
			local_point.X	=	MouseX;
			local_point.Y	=	MouseY;
			if(PointInContent(&local_point))
			{
				AnimControls.GlobalToLocal(&local_point);
				if(AnimFrameRect.PointInRect(&local_point))
				{
					cleanup	=	1;
					update	=	1;
				}
				else if(cleanup)
				{
					cleanup	=	0;
					update	=	1;
				}
			}
			else if(the_key_list[0]&&the_key_list[0]->WhereInWindow(&local_point)==IN_CONTENT)
			{
				update=-1;
			}
			else if(cleanup)
			{
				cleanup	=	0;
				update	=	1;
			}
			if(update>0)
			{
				if(LockWorkScreen())
				{
					DrawAnimFrames(CurrentAnim[Bank],TRUE);
					UnlockWorkScreen();
				}
				SetWorkWindowBounds(0,0,WorkScreenWidth,WorkScreenHeight);
			}
			else
			if(update<0)
			{
				if(the_key_list[0])
				{
					if(LockWorkScreen())
					{
						LogText(" draw funny window (because mouse over it)\n");

						the_key_list[0]->DrawContent();
						UnlockWorkScreen();
					}
					SetWorkWindowBounds(0,0,WorkScreenWidth,WorkScreenHeight);
				}

			}
			if(LockWorkScreen())
			{
				temp_rect.OutlineInvertedRect();
				UnlockWorkScreen();
			}
			ShowWorkScreen(0);
			if(LockWorkScreen())
			{
				temp_rect.OutlineInvertedRect();
				UnlockWorkScreen();
			}
			last_rect	=	temp_rect;
		}
	}
	local_point.X	=	MouseX;
	local_point.Y	=	MouseY;
	if(PointInContent(&local_point))
	{
		AnimControls.GlobalToLocal(&local_point);
		if(AnimFrameRect.PointInRect(&local_point) && CurrentAnim[Bank])
		{
			CurrentAnim[Bank]->SetCurrentFrame(SelectedFrame);
//							CurrentAnim[Bank]->AddKeyFrame(&TestChunk->KeyFrames[selected_frame]);
			CurrentAnim[Bank]->AddKeyFrame(selected_frame);
//			CurrentAnim[Bank]->RemoveKeyFrame(selected_frame);
			((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetValueRange(0,CurrentAnim[Bank]->GetFrameCount()-1);
			((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_TWEAK_SLIDER))->SetCurrentValue(CurrentAnim[Bank]->GetTweakSpeed());
			AnimTween[Bank]		=	0;
			CurrentFrame[Bank]	=	0;
			RequestUpdate();
			return(1);
		}
	}
	else if(the_key_list[0]&&the_key_list[0]->TheAnim)
	{
		if(the_key_list[0]->WhereInWindow(&local_point)==IN_CONTENT)
		{
			the_key_list[0]->TheAnim->SetCurrentFrame(SelectedFrame);
			the_key_list[0]->TheAnim->AddKeyFrame(selected_frame);
//			((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetValueRange(0,CurrentAnim[Bank]->GetFrameCount()-1);
			AnimTween[Bank]		=	0;
			CurrentFrame[Bank]	=	0;
					if(LockWorkScreen())
					{
						LogText(" draw funny window2 (because mouse over it)\n");

						the_key_list[0]->DrawContent();
						UnlockWorkScreen();
					}
					SetWorkWindowBounds(0,0,WorkScreenWidth,WorkScreenHeight);
			RequestUpdate();
			return(2);
		}
		//
		//Instead of deleting, return where it was dropped and let the calling function delete it if necessary
		//
/*
		else if(delete_flag)
		{

			the_key_list[0]->TheAnim->RemoveKeyFrame(selected_frame);
	//		((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetValueRange(0,CurrentAnim[Bank]->GetFrameCount()-1);
			AnimTween[Bank]		=	0;
			CurrentFrame[Bank]	=	0;
		}
*/
	}


	RequestUpdate();
	return(0);

}

//
// for one body part take the body part index and find a suitably named object
//
void	KeyFrameEditor::SetBodyType(SLONG part)
{
	SLONG	c0,so,eo;
	SLONG	index;

	index=PersonBits[part];
	if(index==0)
	{
		test_chunk->PeopleTypes[PersonID].BodyPart[part]=part;
		return;
	}
	so=prim_multi_objects[test_chunk->MultiObject].StartObject;
	eo=prim_multi_objects[test_chunk->MultiObject].EndObject;

	for(c0=so;c0<=eo;c0++)
	{
//		if(!memcmp(body_part_names[part],prim_objects[c0].ObjectName,strlen(body_part_names[part])))
		if(!memcmp(body_part_names[part],prim_names[c0],strlen(body_part_names[part])))
		{
			SLONG value;
			SLONG	str_len;

//			str_len=strlen(prim_objects[c0].ObjectName);
			str_len=strlen(prim_names[c0]);
			value=atoi(&prim_names[c0][str_len-2]);

			if(value==index)
			{
				test_chunk->PeopleTypes[PersonID].BodyPart[part]=c0-so;
				return;

			}

		}
	}

}

//
// For personID set all the object indexes (body parts) in the test_chunk
//
void	KeyFrameEditor::SetPersonBits(void)
{
	SLONG	c0;
	SLONG	object_index,index;

	for(c0=0;c0<MAX_BODY_BITS;c0++)
	{
		SLONG	str_len;
		object_index=test_chunk->PeopleTypes[PersonID].BodyPart[c0];

		object_index+=prim_multi_objects[test_chunk->MultiObject].StartObject;
		str_len=strlen(prim_names[object_index]);
		index=atoi(&prim_names[object_index][str_len-2]);
		PersonBits[c0]=index;
	}
}
void	KeyFrameEditor::HandleControlClick(UBYTE flags,MFPoint *clicked_point)
{
	SLONG		c0,
				first_frame,
				max_frames,
				selected_frame;
	Control		*current_control;
	EdRect		frame_rect;
	MFPoint		local_point;
	SLONG		edit_name=0;


	WindowControls.SetControlDrawArea();
	local_point	=	*clicked_point;
	WindowControls.GlobalToLocal(&local_point);
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			if(CharactersRect.PointInRect(&local_point))
			{
				SLONG	x,y,c0;
				x=CharactersRect.GetLeft();
				y=CharactersRect.GetTop();
				y+=1;

				for(c0=0;c0<15;c0++)
				{
					frame_rect.SetRect(x,y,CharactersRect.GetWidth(),13);
					if(frame_rect.PointInRect(&local_point))
					{
						PersonID	=	c0;
						SetPersonBits();
						((CEditText*)WindowControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->SetFlags((UBYTE)(((CEditText*)AnimControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->GetFlags()&~CONTROL_INACTIVE));
						((CEditText*)WindowControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->SetEditString(test_chunk->PeopleNames[PersonID]);
						edit_name=1;

						break;
					}
					y+=13;
				}
			}
			else
			if(BodyPartRect.PointInRect(&local_point))
			{
				SLONG	x,y,c0;

				x=BodyPartRect.GetLeft();
				y=BodyPartRect.GetTop();
				y+=1;
				for(c0=0;c0<MAX_BODY_BITS;c0++)
				{
					CBYTE	str[100];

					if(body_part_names[c0]==0)
						break;
					frame_rect.SetRect(x,y,BodyPartRect.GetWidth(),13);

					if(frame_rect.PointInRect(&local_point))
					{
						PersonBits[c0]++;
						SetBodyType(c0);

						break;
					}

					y+=13;
				}
			}
			else
			if(KeyFrameRect.PointInRect(&local_point))
			{
				// Find out which frame has been selected.
				selected_frame	=	-1;
				max_frames		=	KeyFrameRect.GetWidth()/KEY_FRAME_IMAGE_SIZE;
				first_frame		=	((CHSlider*)WindowControls.GetControlPtr(CTRL_FRAME_SLIDER))->GetCurrentValue();
//				if((first_frame+max_frames)>KeyFrameCount)
//					max_frames	=	(KeyFrameCount-first_frame)+1;
				if((first_frame+max_frames)>test_chunk->KeyFrameCount)
					max_frames	=	(test_chunk->KeyFrameCount-first_frame)+1;
//				for(c0=0;c0<max_frames&&KeyFrameCount;c0++)
				for(c0=0;c0<max_frames&&test_chunk->KeyFrameCount;c0++)
				{
					WindowControls.SetControlDrawArea();
					frame_rect.SetRect	(
											KeyFrameRect.GetLeft()+(c0*KEY_FRAME_IMAGE_SIZE)+2,
											KeyFrameRect.GetTop()+2,
											KEY_FRAME_IMAGE_SIZE,
											KEY_FRAME_IMAGE_SIZE
										);
					if(frame_rect.PointInRect(&local_point))
					{
						selected_frame	=	first_frame+c0;
						break;
					}
				}

				// Allow selected frame to be dragged around.
				if(selected_frame>=0)
				{
					//
					// Drag Frames out of the main keyframe list
					//
					DragAndDropFrame(&test_chunk->KeyFrames[selected_frame],frame_rect.GetLeft(),frame_rect.GetTop(),frame_rect.GetWidth(),frame_rect.GetHeight(),clicked_point,0);
				}
			}
			else
			{
				current_control	=	WindowControls.GetControlList();
				while(current_control)
				{
					if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(&local_point))
					{
						// Handle control.
						HandleControl(current_control->TrackControl(&local_point));

						// Tidy up display.
						if(LockWorkScreen())
						{
							WindowControls.DrawControlSet();
							UnlockWorkScreen();
						}
						ShowWorkWindow(0);
					}
					current_control	=	current_control->GetNextControl();
				}
			}
			break;
		case	RIGHT_CLICK:
			if(BodyPartRect.PointInRect(&local_point))
			{
				SLONG	x,y,c0;

				x=BodyPartRect.GetLeft();
				y=BodyPartRect.GetTop();
				y+=1;
				for(c0=0;c0<20;c0++)
				{
					CBYTE	str[100];

					if(body_part_names[c0]==0)
						break;
					frame_rect.SetRect(x,y,BodyPartRect.GetWidth(),13);

					if(frame_rect.PointInRect(&local_point))
					{
//						test_chunk->PeopleTypes[PersonID].BodyPart[c0]--;
						PersonBits[c0]--;
						SetBodyType(c0);
						break;
					}

					y+=13;
				}
			}
			break;
	}
	if(!edit_name)
	{
		((CEditText*)WindowControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->SetFlags(CONTROL_INACTIVE);
		((CEditText*)WindowControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->SetEditString("No Name");
	}
}

//---------------------------------------------------------------

SLONG	KeyFrameEditor::HandleModuleKeys(void)
{
	SLONG	update=0;

	if(Keys[KB_0])
	{
		AnimOffsetX[Bank]=0;
		AnimOffsetY[Bank]=0;

	}
	if(Keys[KB_1])
	{
		AnimAngleX[Bank]=0;
		AnimAngleY[Bank]=0;
	}
	if(Keys[KB_2])
	{
		AnimAngleX[Bank]=512;
		AnimAngleY[Bank]=0;
	}
	if(Keys[KB_3])
	{
		AnimAngleX[Bank]=0;
		AnimAngleY[Bank]=512;
	}
	if(Keys[KB_4])
	{
		AnimAngleX[Bank]=256;
		AnimAngleY[Bank]=256;
	}

	if(Keys[KB_SPACE])
	{
		AnimOffsetX[Bank] =0;
		AnimOffsetY[Bank] =0;
		AnimOffsetZ[Bank] =0;

	}
	if(Keys[KB_LEFT])
	{
		if (MoveSeparately)
			AnimOffsetX[Bank] -= 20;
		else
			AnimGlobalOffsetX -= 20;
		update	=	1;
	}
	else if(Keys[KB_RIGHT])
	{
		if (MoveSeparately)
			AnimOffsetX[Bank] += 20;
		else
			AnimGlobalOffsetX += 20;
		update	=	1;
	}
	if(Keys[KB_UP])
	{
		if (MoveSeparately)
			AnimOffsetY[Bank] -= 20;
		else
			AnimGlobalOffsetY -= 20;
		update	=	1;
	}
	else if(Keys[KB_DOWN])
	{
		if (MoveSeparately)
			AnimOffsetY[Bank] += 20;
		else
			AnimGlobalOffsetY += 20;
		update	=	1;
	}

#define ROTATION_STEP (4 << 3)

	if(Keys[KB_DEL])
	{
		if (MoveSeparately)
			AnimAngleY[Bank]	-=	ROTATION_STEP;
		else
		{
			AnimGlobalAngleY	-=	ROTATION_STEP;

/*			// rotate around the centre of both points.
			SLONG	centrex = (AnimOffsetX[0] + AnimOffsetX[1]) / 2;
			SLONG	centrey = (AnimOffsetY[0] + AnimOffsetY[1]) / 2;
			SLONG	centrez = (AnimOffsetZ[0] + AnimOffsetZ[1]) / 2;

			SLONG   sn = SIN(ROTATION_STEP & (2048-1));
			SLONG   cs = COS(ROTATION_STEP & (2048-1));
			
			SLONG	ox, oz;

			ox = AnimOffsetX[0] - centrex;
			oz = AnimOffsetZ[0] - centrez;
			AnimOffsetX[0] = centrex + ((ox * cs) >> 16) + ((oz * sn) >> 16);
			AnimOffsetZ[0] = centrez - ((ox * sn) >> 16) + ((oz * cs) >> 16);

			ox = AnimOffsetX[1] - centrex;
			oz = AnimOffsetZ[1] - centrez;
			AnimOffsetX[1] = centrex + ((ox * cs) >> 16) + ((oz * sn) >> 16);
			AnimOffsetZ[1] = centrez - ((ox * sn) >> 16) + ((oz * cs) >> 16);*/

		}
		update	=	1;
	}
	else if(Keys[KB_PGDN])
	{
		if (MoveSeparately)
			AnimAngleY[Bank]	+=	ROTATION_STEP;
		else
			AnimGlobalAngleY	+=	ROTATION_STEP;

		update	=	1;
	}

	if(Keys[KB_HOME])
	{
		if (MoveSeparately)
			AnimAngleX[Bank]	-=	ROTATION_STEP;
		else
			AnimGlobalAngleX	-=	ROTATION_STEP;
		update	=	1;
	}
	else if(Keys[KB_END])
	{
		if (MoveSeparately)
			AnimAngleX[Bank]	+=	ROTATION_STEP;
		else
			AnimGlobalAngleX	+=	ROTATION_STEP;
		update	=	1;
	}

	if(Keys[KB_I])
	{
		AnimScale		+=	ShiftFlag?16:64;
		update	=	1;
	}
	if(Keys[KB_O])
	{
		AnimScale		-=	ShiftFlag?16:64;
		update	=	1;
	}

	if(FightingColPtr)
	{
		struct	FightCol	*edit_fcol;

		edit_fcol=FightingColPtr;
		if(ShiftFlag)
		{
			if(Keys[KB_MINUS])
			{
				update	=	1;
				edit_fcol->Dist1-=4;
			}
			if(Keys[KB_PLUS])
			{
				update	=	1;
				edit_fcol->Dist1+=4;
			}
			if(Keys[KB_LBRACE])
			{
				update	=	1;
				FightColBank--;
			}
			if(Keys[KB_RBRACE])
			{
				update	=	1;
				FightColBank++;
			}
/*
			if(Keys[KB_QUOTE])
			{
				update	=	1;

			}
*/
		}
		else
		{
			if(Keys[KB_LBRACE])
			{
				update	=	1;
				edit_fcol->Angle-=4;
			}
			if(Keys[KB_RBRACE])
			{
				update	=	1;
				edit_fcol->Angle+=4;
			}
			if(Keys[KB_MINUS])
			{
				update	=	1;
				edit_fcol->Dist2-=4;
			}
			if(Keys[KB_PLUS])
			{
				update	=	1;
				edit_fcol->Dist2+=4;
			}
		}
		if(edit_fcol->Dist2<edit_fcol->Dist1+10)
			edit_fcol->Dist2=edit_fcol->Dist1+10;
	}

	return(update);
}


SLONG	KeyFrameEditor::GetPartID(UWORD current)
{
	EdRect				frame_rect;
	SLONG	mx,my;
	SLONG	sx=200,sy=0;

	while(SHELL_ACTIVE)
	{
		SLONG	c0,multi,multi_count;
		SLONG	col;

		mx=MouseX-WorkWindowRect.Left;
		my=MouseY-WorkWindowRect.Top;

		frame_rect.SetRect(sx,sy,50,300);

		frame_rect.FillRect(ACTIVE_COL);
		frame_rect.HiliteRect(LOLITE_COL,HILITE_COL);

		multi=test_chunk->MultiObject;
		if(RightButton)
		{
			return(current);
		}
		if(multi)
		{
			multi_count=prim_multi_objects[multi].EndObject-prim_multi_objects[multi].StartObject;
			for(c0=0;c0<=multi_count;c0++)
			{
				CBYTE	str[100];
				sprintf(str,"%s",prim_names[c0+prim_multi_objects[multi].StartObject]);

				if(c0==current)
					col=RED_COL;
				else
					col=0;


				if(mx>sx+5&&mx<sx+5+50)
					if(my>sy+c0*12+5&&my<sy+(c0+1)*(12)+5)
					{
						DrawBoxC(sx+2,sy+c0*12+5,48,12,WHITE_COL);
						if(LeftButton)
						{
							return(c0);
						}
					}
				QuickTextC(sx+5,sy+c0*12+5,str,col);
			}
		}

		//DrawBoxC(sx+2,my,48,12,WHITE_COL);
		ShowWorkWindow(0);
	}

	return(0);

}

void	KeyFrameEditor::HandleModule(void)
{
	UBYTE			control;
	ULONG			current_time,
					fps,
					update				=	0;
	MFPoint			local_point,
					mouse_point;
	struct MFTime	the_time;
	static UBYTE	cleanup_content		=	0,
					cleanup_controls	=	0;
	static ULONG	last_time			=	0;
	static MFPoint	last_point;
	SLONG			tweak_speed;
	SLONG			bank=0;


	if(LastKey)
	{
		control	=	WindowControls.HandleControlSetKey(LastKey);
		if(control)
		{
			HandleControl(control);
			LastKey	=	0;
		}
		else
		{
			control	=	AnimControls.HandleControlSetKey(LastKey);
			if(control)
			{
				HandleAnimControl(control);
				LastKey	=	0;
			}
		}

		if(!LastKey)
		{
			// Tidy up display.
			if(LockWorkScreen())
			{
				WindowControls.DrawControlSet();
				UnlockWorkScreen();
			}
			ShowWorkWindow(0);
		}
	}

	mouse_point.X	=	MouseX;
	mouse_point.Y	=	MouseY;
	WindowControls.HandleControlSet(&mouse_point);
	AnimControls.HandleControlSet(&mouse_point);

	if(PointInControls(&mouse_point))
	{		
		local_point	=	mouse_point;
		WindowControls.GlobalToLocal(&local_point);
		if(KeyFrameRect.PointInRect(&local_point))
		{
			if(local_point.X!=last_point.X || local_point.Y!=last_point.Y)
			{
				last_point		=	local_point;
				update			=	1;
				cleanup_controls=	1;
			}
		}
		else if(cleanup_controls)
		{
			update			=	1;
			cleanup_controls=	0;
		}
	}
	else if(cleanup_controls)
	{
		update			=	1;
		cleanup_controls=	0;
	}

	if(PointInContent(&mouse_point))
	{
		local_point	=	mouse_point;
		AnimControls.GlobalToLocal(&local_point);
		if(AnimFrameRect.PointInRect(&local_point))
		{
			if(local_point.X!=last_point.X || local_point.Y!=last_point.Y)
			{
				last_point		=	local_point;
				update			=	1;
				cleanup_content	=	1;
			}
			if(LastKey==KB_F)
			{
				if(SelectedFrame)
				{
					SelectedFrame->Fixed=GetPartID(SelectedFrame->Fixed);
					update			=	1;
					cleanup_content	=	1;
				}
				LastKey	=	0;
			}
			if(LastKey==KB_C)
			{
				if(SelectedFrame)
				{
					struct	FightCol	*fcol;
					if(ShiftFlag)
					{
						fcol=SelectedFrame->Fight;
						SelectedFrame->Fight=fcol->Next;
						MemFree((void*)fcol);
					}
					else
					{
						//SelectedFrame->Fixed=GetPartID(SelectedFrame->Fixed);
						fcol=(struct FightCol*)MemAlloc(sizeof(struct FightCol));
						fcol->Next=SelectedFrame->Fight;
						fcol->Dist1=100;
						fcol->Dist2=200;
						fcol->Angle=0;
						fcol->AngleHitFrom=0;
						FightingColPtr=fcol;


						SelectedFrame->Fight=fcol;
					}
					update			=	1;
					cleanup_content	=	1;
				}
				LastKey	=	0;
			}

			if(LastKey==KB_A)
			{
					KeyFrame	*selected_frame;
					selected_frame	=	&test_chunk->KeyFrames[SelectedFrame->FrameID-1];
					CurrentAnim[Bank]->SetCurrentFrame(SelectedFrame);
					CurrentAnim[Bank]->AddKeyFrame(selected_frame);
					((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetValueRange(0,CurrentAnim[Bank]->GetFrameCount()-1);
					AnimTween[Bank]		=	0;
					CurrentFrame[Bank]	=	0;
					update			=	1;
					cleanup_content	=	1;
					LastKey	=	0;
			}

			if(LastKey==KB_COMMA)
			{
				if(SelectedFrame && SelectedFrame->TweenStep>0)
				{
					SelectedFrame->TweenStep	-=	1;
					AnimTween[Bank]=0;
					update			=	1;
					cleanup_content	=	1;
				}
				LastKey	=	0;
			}
			else if(LastKey==KB_POINT)
			{
				if(SelectedFrame && SelectedFrame->TweenStep<15)
				{
					SelectedFrame->TweenStep	+=	1;
					AnimTween[Bank]=0;
					update			=	1;
					cleanup_content	=	1;
				}
				LastKey	=	0;
			}
		}
		else if(cleanup_content)
		{
			update			=	1;
			cleanup_content	=	0;
		}
	}
	else if(cleanup_content)
	{
		update			=	1;
		cleanup_content	=	0;
	}

	update+=HandleModuleKeys();


	tweak_speed		=	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_TWEAK_SLIDER))->GetCurrentValue()+128;

	SLONG	t = ((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_DISTANCE_SLIDER))->GetCurrentValue();
	if (t != AnimOffsetX[1])
	{
		AnimOffsetX[1]	= t;
		RequestUpdate();
	}
	if(CurrentAnim[Bank])
		CurrentAnim[Bank]->SetTweakSpeed(tweak_speed-128);
	fps				=	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FPS_SLIDER))->GetCurrentValue();
	if(VideoMode!=VIDEO_MODE_PAUSE && VideoMode!=VIDEO_MODE_PAUSE_BACK)
	if(fps)
	{
		current_time=GetTickCount();
		if((current_time-last_time)>(1000/fps))
		{
//			AnimTween[Bank]	+=	16;
			for(bank=0;bank<2;bank++)
			if(CurrentFrame[bank])
			{
				if(SpeedFlag==0)
				{
					if(VideoMode==VIDEO_MODE_PREV_FRAME)
						AnimTween[bank]	-=	tweak_speed/(CurrentFrame[bank]->TweenStep+1);
					else
						AnimTween[bank]	+=	tweak_speed/(CurrentFrame[bank]->TweenStep+1);
				}
				else
				{
					if(SpeedFlag==1)
					{
						if(VideoMode==VIDEO_MODE_PREV_FRAME)
							AnimTween[bank]	-=	256;
						else
							AnimTween[bank]	+=	256;
					}
					else
					{
						if(SpeedFlag==2)
						{
							if(VideoMode==VIDEO_MODE_PREV_FRAME)
								AnimTween[bank]	-=	8;
							else
								AnimTween[bank]	+=	8;
						}
					}
				}

			}
			if(VideoMode==VIDEO_MODE_NEXT_FRAME)
				VideoMode=VIDEO_MODE_PAUSE;
			if(VideoMode==VIDEO_MODE_PREV_FRAME)
				VideoMode=VIDEO_MODE_PAUSE_BACK;
			update		=	1;
			last_time		=	current_time;
		}
	}
	else
	{

	}

	if(update)
	{
		if(LockWorkScreen())
		{
			DrawContent();
			DrawGrowBox();
			UnlockWorkScreen();
			ShowWorkWindow(0);
		}
		if(LockWorkScreen())
		{
			DrawControls();
			UnlockWorkScreen();
			ShowWorkWindow(0);
		}
	}
}

//---------------------------------------------------------------

void	KeyFrameEditor::AddKeyFrameChunk(void)
{
	
}

//---------------------------------------------------------------
extern	void	set_default_people_types(struct	KeyFrameChunk *the_chunk);

void	KeyFrameEditor::SetAnimBank(SLONG bank)
{
	switch(bank)
	{
		case	0:
			test_chunk=&edit_chunk1;
			the_elements=elements_bank1;
			Bank=0;
			current_element = 0;
			break;

		case	1:
			test_chunk=&edit_chunk2;
			the_elements=elements_bank2;
			Bank=1;
			current_element = 0;
			break;
	}
}


UWORD	done[512];
UBYTE	next_done=0;

void	add_done(UWORD idone)
{
	done[next_done++]=idone;

}

UBYTE	allready_done(UWORD idone)
{
	SLONG	c0;
	for(c0=0;c0<next_done;c0++)
	{
		if(done[c0]==idone)
			return(1);
	}
	return(0);
}


void	clear_done(void)
{
	memset(&done[0],0,2*512);
	next_done=0;
}

void	re_center_anim(struct KeyFrame	*current_frame,SLONG do_y,SLONG do_all)
{
	struct KeyFrameElement *anim_info;
	SLONG	c0,dx,dy,dz;

//	ASSERT(0);
	clear_done();

	anim_info=&current_frame->FirstElement[SUB_OBJECT_LEFT_FOOT];
	dy	=	anim_info->OffsetY-FOOT_HEIGHT;

	anim_info=&current_frame->FirstElement[SUB_OBJECT_PELVIS];
	dx	=	anim_info->OffsetX;
	dz	=	anim_info->OffsetZ;

	while(current_frame)
	{
		if(do_all)
		{
			anim_info=&current_frame->FirstElement[SUB_OBJECT_LEFT_FOOT];
			dy	=	anim_info->OffsetY-FOOT_HEIGHT;

			anim_info=&current_frame->FirstElement[SUB_OBJECT_PELVIS];
			dx	=	anim_info->OffsetX;
			dz	=	anim_info->OffsetZ;
		}

		if(!allready_done(current_frame->FrameID))
		{
			add_done(current_frame->FrameID);

			for(c0=0;c0<test_chunk->ElementCount;c0++)
			{
				anim_info=&current_frame->FirstElement[c0];
				anim_info->OffsetX-=dx;
				if(do_y)
					anim_info->OffsetY-=dy;
				anim_info->OffsetZ-=dz;
			}
		}
		if(!(current_frame->Flags&ANIM_FLAG_LAST_FRAME))
		{
			current_frame=current_frame->NextFrame;
		}
		else
		{
			current_frame=0;
		}
	}

}


void	KeyFrameEditor::HandleControl(ULONG control_id)
{
	SLONG			c0;
	FileRequester	*fr;
	SLONG	ele_count;
	SLONG	max_range;


	switch(control_id)
	{
		case	0:
			break;
		case	CTRL_DRAW_BOTH:
			if(DontDrawBoth)
			{
				DontDrawBoth=0;
				WindowControls.SetControlState(CTRL_DRAW_BOTH,CTRL_DESELECTED);
			}
			else
			{
				DontDrawBoth=1;
				WindowControls.SetControlState(CTRL_DRAW_BOTH,CTRL_SELECTED);
			}
			break;
		case	CTRL_MOVE_SEPARATELY:
			if(MoveSeparately)
			{
				MoveSeparately=0;
				WindowControls.SetControlState(CTRL_MOVE_SEPARATELY,CTRL_DESELECTED);
			}
			else
			{
				MoveSeparately=1;
				WindowControls.SetControlState(CTRL_MOVE_SEPARATELY,CTRL_SELECTED);
			}
			break;
		case	CTRL_FLIP_BANK:
			if(test_chunk==&edit_chunk2)
			{
				SetAnimBank(0);
			}
			else
			{
				SetAnimBank(1);
			}
			
				((CHSlider*)WindowControls.GetControlPtr(CTRL_FRAME_SLIDER))->SetValueRange(0,test_chunk->KeyFrameCount-2);
			/*	if(AnimList[Bank])
					((CHSlider*)the_key_list[0]->KeyControls.GetControlPtr(CTRL_KEY_SLIDER))->SetValueRange(0,the_key_list[0]->TheAnim->GetFrameCount()+20); 
			*/

				max_range	=	AnimCount[Bank]-22;
				if(max_range<0)
					max_range	=	0;
				((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->SetValueRange(0,max_range);
//				((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->SetCurrentValue(max_range);

			break;
		case	CTRL_NEXT_MESH:
			test_chunk->MultiObject++;
			if(test_chunk->MultiObject>=next_prim_multi_object)
			{
				test_chunk->MultiObject--;

			}
			break;
		case	CTRL_PREV_MESH:
			test_chunk->MultiObject--;
			if(test_chunk->MultiObject<=0)
			{
				test_chunk->MultiObject++;

			}
			break;
		case	CTRL_RE_CENTER_ALL:
			{

				SLONG	dx,dy,dz,c0;
				struct  KeyFrame		*current_frame;
				Anim	*current_anim;
				SLONG	index=0;


				if(AnimList[Bank])
				{
					current_anim	=	AnimList[Bank];
					while(current_anim!=CurrentAnim[Bank] && current_anim)
					{
						index++;
						current_anim = current_anim->GetNextAnim();
					}
				}
				re_center_flags[index]=RE_CENTER_ALL;




				if(CurrentAnim[Bank])
				{
					re_center_anim(CurrentAnim[Bank]->GetFrameListStart(),1,1);
				}
			}


			break;
		case	CTRL_RE_CENTER_NONE:
			{

				SLONG	dx,dy,dz,c0;
				struct  KeyFrame		*current_frame;
				Anim	*current_anim;
				SLONG	index=0;


				if(AnimList[Bank])
				{
					current_anim	=	AnimList[Bank];
					while(current_anim!=CurrentAnim[Bank] && current_anim)
					{
						index++;
						current_anim = current_anim->GetNextAnim();
					}
				}
				re_center_flags[index]=RE_CENTER_NONE;
			}
			break;
		case	CTRL_RE_CENTER_START:
			{

				SLONG	dx,dy,dz,c0;
				struct  KeyFrame		*current_frame;
				Anim	*current_anim;
				SLONG	index=0;


				if(AnimList[Bank])
				{
					current_anim	=	AnimList[Bank];
					while(current_anim!=CurrentAnim[Bank] && current_anim)
					{
						index++;
						current_anim = current_anim->GetNextAnim();
					}
				}
				re_center_flags[index]=RE_CENTER_START;




				if(CurrentAnim[Bank])
				{
					re_center_anim(CurrentAnim[Bank]->GetFrameListStart(),1,0);
				}
			}
			break;
//		case	CTRL_RE_CENTER_END:
//			break;
		case	CTRL_RE_CENTER_XZ_START:
			{

				SLONG	dx,dy,dz,c0;
				struct  KeyFrame		*current_frame;
				Anim	*current_anim;
				SLONG	index=0;


				if(AnimList[Bank])
				{
					current_anim	=	AnimList[Bank];
					while(current_anim!=CurrentAnim[Bank] && current_anim)
					{
						index++;
						current_anim = current_anim->GetNextAnim();
					}
				}
				re_center_flags[index]=RE_CENTER_XZ_START;

				if(CurrentAnim[Bank])
				{
					re_center_anim(CurrentAnim[Bank]->GetFrameListStart(),0,0);
				}
			}
			break;
		case	CTRL_RE_CENTER_XZ:
			{

				SLONG	dx,dy,dz,c0;
				struct  KeyFrame		*current_frame;
				Anim	*current_anim;
				SLONG	index=0;


				if(AnimList[Bank])
				{
					current_anim	=	AnimList[Bank];
					while(current_anim!=CurrentAnim[Bank] && current_anim)
					{
						index++;
						current_anim = current_anim->GetNextAnim();
					}
				}
				re_center_flags[index]=RE_CENTER_XZ;

				if(CurrentAnim[Bank])
				{
					re_center_anim(CurrentAnim[Bank]->GetFrameListStart(),0,1);
				}
			}
			break;
		case	CTRL_SYNC_BOTH:

			if(CurrentAnim[0])
			{
				PlayingAnim[0]		=	CurrentAnim[0];
				CurrentFrame[0]	=	PlayingAnim[0]->GetFrameList();
			}

			if(CurrentAnim[1])
			{
				PlayingAnim[1]		=	CurrentAnim[1];
				CurrentFrame[1]	=	PlayingAnim[1]->GetFrameList();
			}

			AnimTween[0]	=0;
			AnimTween[1]	=0;

			break;

		case	CTRL_LOAD_BUTTON2:
		case	CTRL_LOAD_BUTTON:

			if(control_id==CTRL_LOAD_BUTTON)
			{
				SetAnimBank(0);
			}
			else
			{
				SetAnimBank(1);
			}

			fr	=	new FileRequester("DATA\\","*.VUE","Load a VUE file",".VUE");
			if(fr->Draw())
			{
				PersonID=0;
				set_default_people_types(test_chunk);

//	void	setup_anim_stuff(void);
//				setup_anim_stuff();

				ClearAll(); //problem function because it should go after loading the sex file, so you know it exists

//	void	load_key_frame_chunks(KeyFrameChunk *the_chunk,CBYTE *vue_name);
				{
					float	shrink=1.0;
					//
					// bodge ahoy
					//
extern	SLONG	save_psx;
					if(save_psx)
					{
					if(strcmp(fr->FileName,"balrog.VUE")==0)
						shrink=4.0;
					if(strcmp(fr->FileName,"gargoyle1.VUE")==0)
						shrink=4.0;
					if(strcmp(fr->FileName,"bane.VUE")==0)
						shrink=4.0;
					}

					load_key_frame_chunks(test_chunk,fr->FileName,shrink);
					load_recenter_flags(test_chunk->ANMName);
				}

				delete	fr;
				RequestUpdate();

				((CHSlider*)WindowControls.GetControlPtr(CTRL_FRAME_SLIDER))->SetValueRange(0,test_chunk->KeyFrameCount-2);
				LoadAllAnims(test_chunk);
				LogText(" keyframe edit load \n");
				LoadChunkTextureInfo(test_chunk);
				if(the_key_list[0])
				{
					the_key_list[0]->TheAnim=AnimList[Bank];
					if(AnimList[Bank])
						((CHSlider*)the_key_list[0]->KeyControls.GetControlPtr(CTRL_KEY_SLIDER))->SetValueRange(0,the_key_list[0]->TheAnim->GetFrameCount()+20); 
				}
				SetPersonBits();
				AnimTween[Bank]		=	0;
				CurrentFrame[Bank]	=	0;
				if(CurrentAnim[Bank])
					((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_TWEAK_SLIDER))->SetCurrentValue(CurrentAnim[Bank]->GetTweakSpeed());
			}

			//
			// use re_center_flags
			//
			{

				SLONG	dx,dy,dz,c0;
				struct  KeyFrame		*current_frame;
				Anim	*current_anim;
				SLONG	index=0;


				if(AnimList[Bank])
				{
					current_anim	=	AnimList[Bank];
					while(current_anim)
					{
						current_frame=current_anim->GetFrameListStart();
						switch(re_center_flags[index])
						{
							case	RE_CENTER_NONE:
								break;
							case	RE_CENTER_ALL:
								re_center_anim(current_frame,1,1);
								break;

							case	RE_CENTER_START:
								re_center_anim(current_frame,1,0);
								break;
							case	RE_CENTER_END:
								break;
							case	RE_CENTER_XZ:
								re_center_anim(current_frame,0,1);
								break;
							case	RE_CENTER_XZ_START:
								re_center_anim(current_frame,0,0);
								break;
						}


						index++;
						current_anim = current_anim->GetNextAnim();
					}
				}
			}


			break;

//			KeyFrameChunk *the_chunk,CBYTE *vue_name)
#ifdef	POO_SHIT_GOD_DAMN

			strcpy(test_chunk->VUEName,fr->Path);
			strcat(test_chunk->VUEName,fr->FileName);
			strcpy(test_chunk->ASCName,test_chunk->VUEName);
			strcpy(test_chunk->ANMName,test_chunk->VUEName);
			c0=0;
			while(test_chunk->ASCName[c0]!='.' && test_chunk->ASCName[c0]!=0)c0++;
			if(test_chunk->ASCName[c0]=='.')
			{
				test_chunk->ASCName[c0+1]	=	'A';
				test_chunk->ASCName[c0+2]	=	'S';
				test_chunk->ASCName[c0+3]	=	'C';
				test_chunk->ASCName[c0+4]	=	0;

				test_chunk->ANMName[c0+1]	=	'A';
				test_chunk->ANMName[c0+2]	=	'N';
				test_chunk->ANMName[c0+3]	=	'M';
				test_chunk->ANMName[c0+4]	=	0;
			}
			delete	fr;
			RequestUpdate();
void	setup_anim_stuff(void);
			setup_anim_stuff();

			if(ele_count=read_multi_asc(test_chunk->ASCName,0))
			{
				ClearAll();
				test_chunk->MultiObject	=	next_prim_multi_object-1;
				test_chunk->ElementCount	=	ele_count; //prim_multi_objects[test_chunk->MultiObject].EndObject-prim_multi_objects[test_chunk->MultiObject].StartObject;
				LogText("IN KEYFRAMER element count %d \n",test_chunk->ElementCount);
				// Fudgy bit for centering.
				{
SLONG				c1,
					sp,ep;
struct PrimObject	*p_obj;

					
					for(c0=prim_multi_objects[test_chunk->MultiObject].StartObject;c0<prim_multi_objects[test_chunk->MultiObject].EndObject;c0++)
					{
						p_obj   =	&prim_objects[c0];
						sp		=	p_obj->StartPoint;
						ep		=	p_obj->EndPoint;

						for(c1=sp;c1<ep;c1++)
						{
							prim_points[c1].X	-=	x_centre;
							prim_points[c1].Y	-=	y_centre;
							prim_points[c1].Z	-=	z_centre;
						}
					}
				}				

//				LoadMultiVUE(test_chunk);
				load_multi_vue(test_chunk);
				((CHSlider*)WindowControls.GetControlPtr(CTRL_FRAME_SLIDER))->SetValueRange(0,test_chunk->KeyFrameCount-2);
				LoadAllAnims(test_chunk);
				LogText(" keyframe edit load \n");
				LoadChunkTextureInfo(test_chunk);
				if(the_key_list[0])
				{
					the_key_list[0]->TheAnim=AnimList[Bank];
					if(AnimList[Bank])
						((CHSlider*)the_key_list[0]->KeyControls.GetControlPtr(CTRL_KEY_SLIDER))->SetValueRange(0,the_key_list[0]->TheAnim->GetFrameCount()+20); 
				}
			}
			SetPersonBits();
//			((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetValueRange(0,CurrentAnim[Bank]->GetFrameCount()-1);
			AnimTween[Bank]		=	0;
			CurrentFrame[Bank]	=	0;

			break;
#endif
		case	CTRL_FRAME_SLIDER:
			if(LockWorkScreen())
			{
				 DrawKeyFrames();
				UnlockWorkScreen();					  
				ShowWorkWindow(0);
			}
			break;
		case	CTRL_CHAR_NAME_EDIT:
			strcpy(test_chunk->PeopleNames[PersonID],(((CEditText*)WindowControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->GetEditString()));
//			((CEditText*)WindowControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->SetFlags(CONTROL_ACTIVE);
//			((CEditText*)WindowControls.GetControlPtr(CTRL_CHAR_NAME_EDIT))->SetEditString(test_chunk->PeopleNames[PersonID]);
			break;

	}
}

//---------------------------------------------------------------

void	KeyFrameEditor::HandleAnimControl(ULONG  control_id)
{
	switch(control_id)
	{
		case	CTRL_ANIM_FRAME_SLIDER:
			if(LockWorkScreen())
			{
				DrawAnimFrames(CurrentAnim[Bank],FALSE);
				UnlockWorkScreen();
				ShowWorkWindow(0);
			}
			break;
		case	CTRL_ANIM_FPS_TEXT:
			break;
		case	CTRL_ANIM_FPS_SLIDER:
			break;
		case	CTRL_ANIM_NEW_ANIM_BUTTON:
			AppendAnim();
			((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->SetFlags((UBYTE)(((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->GetFlags()&~CONTROL_INACTIVE));
			((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->SetEditString(CurrentAnim[Bank]->GetAnimName());
			break;
		case	CTRL_ANIM_ALL_ANIMS_SLIDER:
			break;
		case	CTRL_ANIM_NAME_EDIT:
			if(CurrentAnim[Bank])
				CurrentAnim[Bank]->SetAnimName(((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->GetEditString());
			break;
		case	CTRL_FIGHT_TWEEN0:
			CurrentAnim[Bank]->SetAllTweens(0);
			break;
		case	CTRL_FIGHT_TWEEN1:
			CurrentAnim[Bank]->SetAllTweens(1);
			break;
		case	CTRL_FIGHT_TWEEN2:
			CurrentAnim[Bank]->SetAllTweens(2);
			break;
		case	CTRL_FIGHT_TWEEN3:
			CurrentAnim[Bank]->SetAllTweens(3);
			break;

		case	CTRL_ANIM_LOOP_SELECT:
			if(CurrentAnim[Bank])
			{
				CurrentAnim[Bank]->SetAnimFlags((UBYTE)(CurrentAnim[Bank]->GetAnimFlags()^ANIM_LOOP));
				if(CurrentAnim[Bank]->GetAnimFlags()&ANIM_LOOP)
				{
					AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_SELECTED);
					CurrentAnim[Bank]->StartLooping();
				}
				else
				{
					AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_DESELECTED);
					CurrentAnim[Bank]->StopLooping();
				}
			}
			break;
		case	CTRL_ANIM_SUPER_SMOOTH:
			if(CurrentAnim[Bank])
			{
				if(SpeedFlag==2)
				{
					AnimControls.SetControlState(CTRL_ANIM_SUPER_SMOOTH,CTRL_DESELECTED);
					SpeedFlag=0;
				}
				else
				{
					SpeedFlag=2;
					AnimControls.SetControlState(CTRL_ANIM_SUPER_SMOOTH,CTRL_SELECTED);
					AnimControls.SetControlState(CTRL_ANIM_JUST_KEYS,CTRL_DESELECTED);
				}
			}
			break;
		case	CTRL_ANIM_USE_QUATERNIONS:
			if(CurrentAnim[Bank])
			{
				if(QuaternionFlag==1)
				{
					AnimControls.SetControlState(CTRL_ANIM_USE_QUATERNIONS,CTRL_DESELECTED);
					QuaternionFlag=0;
				}
				else
				{
					QuaternionFlag=1;
					AnimControls.SetControlState(CTRL_ANIM_USE_QUATERNIONS,CTRL_SELECTED);
				}
				RequestUpdate();
			}
			break;
		case	CTRL_ANIM_FLIP_1:
			if(CurrentAnim[Bank])
			{
				if(Flip1==1)
				{
					AnimControls.SetControlState(CTRL_ANIM_FLIP_1,CTRL_DESELECTED);
					Flip1=0;
					AnimAngleY[0] += 1024;
					if (AnimAngleY[0] > 1024)
						AnimAngleY[0] -= 2048;
				}
				else
				{
					AnimControls.SetControlState(CTRL_ANIM_FLIP_1,CTRL_SELECTED);
					Flip1=1;
					AnimAngleY[0] -= 1024;
					if (AnimAngleY[0] < -1024)
						AnimAngleY[0] += 2048;
				}
				RequestUpdate();
			}
			break;
		case	CTRL_ANIM_FLIP_2:
			if(CurrentAnim[Bank])
			{
				if(Flip2==1)
				{
					AnimControls.SetControlState(CTRL_ANIM_FLIP_2,CTRL_DESELECTED);
					Flip2=0;
					AnimAngleY[1] += 1024;
					if (AnimAngleY[1] > 1024)
						AnimAngleY[1] -= 2048;
				}
				else
				{
					AnimControls.SetControlState(CTRL_ANIM_FLIP_2,CTRL_SELECTED);
					Flip2=1;
					AnimAngleY[1] -= 1024;
					if (AnimAngleY[1] < -1024)
						AnimAngleY[1] += 2048;
				}
				RequestUpdate();
			}
			break;
		case	CTRL_ANIM_JUST_KEYS:
			if(CurrentAnim[Bank])
			{
				if(SpeedFlag==1)
				{
					AnimControls.SetControlState(CTRL_ANIM_JUST_KEYS,CTRL_DESELECTED);
					SpeedFlag=0;
				}
				else
				{
					SpeedFlag=1;
					AnimControls.SetControlState(CTRL_ANIM_SUPER_SMOOTH,CTRL_DESELECTED);
					AnimControls.SetControlState(CTRL_ANIM_JUST_KEYS,CTRL_SELECTED);
					AnimTween[Bank]=0;
				}
			}
			break;
		case	CTRL_PAINT_MESH:
			{
extern	void	bring_module_to_front(EditorModule *the_module);
				bring_module_to_front(LinkLevelEditor);
				LinkLevelEditor->PrimMode->SetPrimTabMode(PRIM_MODE_MULTI);
				LinkLevelEditor->PrimMode->SetCurrentPrim(test_chunk->MultiObject);
				LinkLevelEditor->BringTabToFront(LinkLevelEditor->PaintMode);
				RequestUpdate();
			}
			break;

		case	CTRL_VIDEO_NEXT:
			VideoMode=VIDEO_MODE_NEXT_FRAME;

			break;

		case	CTRL_VIDEO_PLAY:
			VideoMode=VIDEO_MODE_PLAY;
			break;

		case	CTRL_VIDEO_PREV:
			VideoMode=VIDEO_MODE_PREV_FRAME;
			break;

		case	CTRL_VIDEO_PAUSE:
			VideoMode=VIDEO_MODE_PAUSE;
			break;
		case	CTRL_FIGHT_HEIGHT_PLUS:
			if(FightingColPtr)
			{
				FightingColPtr->Height++;
				if(FightingColPtr->Height>3)
					FightingColPtr->Height=0;
				RequestUpdate();
			}
			break;
		case	CTRL_FIGHT_HEIGHT_MINUS:
			if(FightingColPtr)
			{
				FightingColPtr->Height--;
				if(FightingColPtr->Height>3)
					FightingColPtr->Height=3;
				RequestUpdate();
			}
			break;
		case	CTRL_FIGHT_DAMAGE_PLUS:
			if(FightingColPtr)
			{
				SLONG	size=1;
				if(ShiftFlag)
					size=10;

				if(ControlFlag)
					size=50;

				FightingColPtr->Damage+=size;
				RequestUpdate();
			}
			break;
		case	CTRL_FIGHT_DAMAGE_MINUS:
			if(FightingColPtr)
			{
				SLONG	size=1;
				if(ShiftFlag)
					size=10;

				if(ControlFlag)
					size=50;

				FightingColPtr->Damage-=size;
				RequestUpdate();
			}
			break;



		case	CTRL_ANIM_LORES_TEST:
			ULONG			current_time,
							fps,
							last_time;
			SLONG			depth,
							height,
							width;
			struct MFTime	the_time;
			SLONG	tweak_speed;


			width	=	WorkScreenPixelWidth;
			height	=	WorkScreenHeight;
			depth	=	WorkScreenDepth;
			SetDisplay(320,200,16);
			SetDrawFunctions(16);
			SetWorkWindowBounds(0,0,320,200);

			while(SHELL_ACTIVE && LastKey!=KB_ESC)
			{
				ClearWorkScreen(0);

				if(LockWorkScreen())
				{
					if(CurrentAnim[Bank])
					{
						HandleModuleKeys();
						engine.ShowDebug=	0;
						set_key_framer_camera();
						DoCurrentAnim();
						render_view(0);
						engine.ShowDebug=	1;
					}
					
					UnlockWorkScreen();
					ShowWorkScreen(0);
				}

				fps				=	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FPS_SLIDER))->GetCurrentValue();
				tweak_speed		=	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_TWEAK_SLIDER))->GetCurrentValue()+128;
				if(fps)
				{
					//Time(&the_time);
					current_time	=	GetTickCount();
					if((current_time-last_time)>(1000/fps))
					{
			//			AnimTween[Bank]	+=	16;
						if(CurrentFrame[Bank])
						{
							AnimTween[Bank]	+=	tweak_speed/(CurrentFrame[Bank]->TweenStep+1);
						}
						last_time		=	current_time;
					}
				}
			}
			LastKey	=	0;
			SetDisplay(width,height,16);
			SetDrawFunctions(16);
			SetWorkWindowBounds(0,0,width,height);
			RequestUpdate();
			break;
		case	CTRL_ANIM_SAVE_ANIMS:
			SaveAllAnims(test_chunk,0);			
			break;
		case	CTRL_ANIM_SAVE_GAME_ANIM:
			SaveAllAnims(test_chunk,1);			
			break;
		case	CTRL_ANIM_RESET_VIEW:
			AnimAngleX[0]		=	0;
			AnimAngleY[0]		=	-512;
			AnimAngleZ[0]		=	0;
			AnimAngleX[1]		=	0;
			AnimAngleY[1]		=	512;
			AnimAngleZ[1]		=	0;

			AnimGlobalAngleX	=	0;
			AnimGlobalAngleY	=	0;
			AnimGlobalOffsetX	=	0;
			AnimGlobalOffsetY	=	0;

			AnimOffsetX[0]		=	0;
			AnimOffsetY[0]		=	0;
			AnimOffsetZ[0]		=	0;

			AnimOffsetX[1]		=	100;
			AnimOffsetY[1]		=	0;
			AnimOffsetZ[1]		=	0;

			AnimScale			=	2000; //296;
			
			Flip1 = 0;
			Flip2 = 0;
			AnimControls.SetControlState(CTRL_ANIM_FLIP_1,CTRL_DESELECTED);
			AnimControls.SetControlState(CTRL_ANIM_FLIP_2,CTRL_DESELECTED);
			RequestUpdate();

			break;
	}
}

//---------------------------------------------------------------
struct	LinkSort
{
	SWORD	Z;
	SWORD	Item;
	SWORD	Next;	
};


struct	LinkSort	link_sorts[100];
SLONG	link_head;
SWORD	next_link_head;

SLONG	calc_z_for_piece(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,struct Matrix33 *rot_mat)
{
	SLONG	flags;
	struct	SVector			res,temp; //max points per object?
	struct	Matrix31	offset;
	SLONG	tx,ty,tz;


	prim=prim;	


	offset.M[0]	=	(anim_info->OffsetX+(((anim_info_next->OffsetX-anim_info->OffsetX)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[1]	=	(anim_info->OffsetY+(((anim_info_next->OffsetY-anim_info->OffsetY)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[2]	=	(anim_info->OffsetZ+(((anim_info_next->OffsetZ-anim_info->OffsetZ)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
//	LogText(" CALCZ  %d %d %d\n",offset.M[0],offset.M[1],offset.M[2]);
	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset); 			//temp=offset*rot matrix
	x	+= temp.X;
	y	+= temp.Y;
	z	+= temp.Z;


	tx=engine.X;
	ty=engine.Y;
	tz=engine.Z;

	engine.X=-x<<8;
	engine.Y=-y<<8;
	engine.Z=-z<<8;

//apply local rotation matrix

		
	flags=rotate_point_gte((struct SVector*)&temp,&res);


	engine.X=tx;
	engine.Y=ty;
	engine.Z=tz;

	return(res.Z);
}


void	DumpList(void)
{
	SLONG	index;
	index=link_head;
	while(index)
	{
		LogText(" [z %d item %d] ",link_sorts[index].Z,link_sorts[index].Item);  
		index=link_sorts[index].Next;
		LogText("\n");
	}
}

void	insert_z_in_sort_list(SLONG z,SLONG item)
{
	SLONG	prev=0;
	SLONG	index;
//	LogText(" Insert Node z %d item %d \n",z,item);

	link_sorts[next_link_head].Z=z;
	link_sorts[next_link_head].Item=item;

	index=link_head;
	while(index)
	{
		if(z<link_sorts[index].Z)
		{

			link_sorts[next_link_head].Next=index;
			if(prev==0)
				link_head=next_link_head;
			else
				link_sorts[prev].Next=next_link_head;
			next_link_head++;
//			DumpList();
			return;
		}
		prev=index;
		index=link_sorts[index].Next;
	}

	link_sorts[next_link_head].Next=0;
	if(prev==0)
		link_head=next_link_head;
	else
		link_sorts[prev].Next=next_link_head;

	next_link_head++;
//			DumpList();
}

SLONG	build_sort_order_tweened(UWORD multi_prim,KeyFrame		*CurrentFrame,SLONG x,SLONG y,SLONG z,SLONG tween,struct Matrix33 *r_matrix)
{
	struct KeyFrameElement	*the_element,
							*the_next_element;
	SLONG	pz;
	SLONG	az=0;
	SLONG	c0,c1;
	link_head=0;
	next_link_head=1;
//	LogText(" sort list \n");

	for(c1=0,c0=prim_multi_objects[multi_prim].StartObject;c0<prim_multi_objects[multi_prim].EndObject;c0++,c1++)
	{
		the_element			=	&CurrentFrame->FirstElement[c1];
		if(CurrentFrame->NextFrame)
			the_next_element	=	&CurrentFrame->NextFrame->FirstElement[c1];
		else
			the_next_element	=	the_element;

		pz=calc_z_for_piece(c0,x,y,z,tween,the_element,the_next_element,r_matrix);
		insert_z_in_sort_list(pz,c1);
		az+=pz;
	}
	if(c1)
		az/=c1;
	return(az);
}



void	test_draw_z(SLONG az,UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,struct Matrix33 *rot_mat)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	flags[1560];
	ULONG	flag_and,flag_or;
	struct	SVector			res[1560],temp; //max points per object?
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	struct	Matrix33 *mat,*mat_next,mat2,mat_final;
	SLONG	i,j;
//	struct	KeyFrameElement *anim_info_next;
	struct	Matrix31	offset;
	SLONG	tx,ty,tz;


	p_obj    =&prim_objects[prim];
	p_f4     =&prim_faces4[p_obj->StartFace4];
	p_f3     =&prim_faces3[p_obj->StartFace3];
	
//	anim_info_next	=	&TheElements[anim_info->Next];
//	anim_info_next	=	&&TestChunkTestChunk->KeyFrames[3].FirstElement;

	mat      = &anim_info->Matrix;
	mat_next = &anim_info_next->Matrix;

	//move object "tweened quantity"  , z&y flipped
	offset.M[0]	=	(anim_info->OffsetX+(((anim_info_next->OffsetX-anim_info->OffsetX)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[1]	=	(anim_info->OffsetY+(((anim_info_next->OffsetY-anim_info->OffsetY)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[2]	=	(anim_info->OffsetZ+(((anim_info_next->OffsetZ-anim_info->OffsetZ)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
	x	+= temp.X;
	y	+= temp.Y;
	z	+= temp.Z;
/*
	x+=(anim_info->DX+(((anim_info_next->DX-anim_info->DX)*tween)>>8))>>2;
	y+=(anim_info->DY+(((anim_info_next->DY-anim_info->DY)*tween)>>8))>>2;
	z+=(anim_info->DZ+(((anim_info_next->DZ-anim_info->DZ)*tween)>>8))>>2;
*/
	

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;

	tx=engine.X;
	ty=engine.Y;
	tz=engine.Z;

	engine.X=-x<<8;
	engine.Y=-y<<8;
	engine.Z=-z<<8;

//	engine.X=temp.X<<8;
  //	engine.Y=temp.Y<<8;
	//engine.Z=temp.Z<<8;

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



//apply local rotation matrix
	matrix_mult33(&mat_final,rot_mat,&mat2);

		
	for(c0=sp;c0<ep;c0++)
	{

		matrix_transform_small((struct Matrix31*)&temp,&mat_final, (struct SMatrix31*)&prim_points[c0]);
//		matrix_transform((struct Matrix31*)&temp,&mat2, (struct Matrix31*)&prim_points[c0]);
//      matrix now does the yz flip
//		swap(temp.Y,temp.Z)  //MDOPT do this inside the matrix multiply
//		temp.Y=-temp.Y;

		
		flags[c0-sp]	=	rotate_point_gte((struct SVector*)&temp,&res[c0-sp]);
//		LogText(" after rot (%d,%d,%d)  \n",res[c0-sp].X,res[c0-sp].Y,res[c0-sp].Z);
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
//		LogText(" test draw face %d  p0 x (%d,%d,%d)\n",c0,res[p0].X,res[p0].Y,res[p0].Z);
//		DrawLineC(engine.VW2,engine.VH2,res[p0].X,res[p0].Y,0);
//		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
			
//			az=(res[p0].Z+res[p1].Z+res[p2].Z+res[p3].Z)>>2;

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
//			az=(res[p0].Z+res[p1].Z+res[p2].Z)/3;

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

//extern	void	create_bucket_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col);
extern	void	create_bucket_3d_line_whole(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col);

void	draw_3d_mat_circle(SLONG x,SLONG y,SLONG z,SLONG radius,struct Matrix33	*r_matrix,UWORD col)
{
	SLONG	angle=0;
	struct	SVector		offset,res,prev;
	SLONG	tx,ty,tz;

	tx=(COS(angle&2047)*radius)>>16;
	tz=(SIN(angle&2047)*radius)>>16;

	offset.X=x+tx;
	offset.Y=y;
	offset.Z=z+tz;
	matrix_transformZMY((struct Matrix31*)&prev,r_matrix, (struct Matrix31*)&offset);

	for(angle=16;angle<=2048;angle+=16)
	{
		tx=(COS(angle&2047)*radius)>>16;
		tz=(SIN(angle&2047)*radius)>>16;

		offset.X=x+tx;
		offset.Y=y;
		offset.Z=z+tz;

		matrix_transformZMY((struct Matrix31*)&res,r_matrix, (struct Matrix31*)&offset);
		create_bucket_3d_line_whole(prev.X,prev.Y,prev.Z,res.X,res.Y,res.Z,col);

		prev=res;
	}
}

void	KeyFrameEditor::DrawCombatEditor(void)
{
	SLONG	tx,ty,tz;
	SLONG	x,y,z;
	SLONG	angle,angle_step;
	struct Matrix33			r_matrix;
	SLONG	temp_scale;
	static	SLONG	r1=100,r2=200;
	struct	FightCol	*fcol,*edit_fcol=0;
	SLONG	dy=0;
	SLONG	count=0,col;
	CBYTE	str[100];

	SVECTOR	pelvis,pelvis_after;

	pelvis.X=CurrentFrame[Bank]->FirstElement[0].OffsetX;
	pelvis.Y=CurrentFrame[Bank]->FirstElement[0].OffsetY;
	pelvis.Z=CurrentFrame[Bank]->FirstElement[0].OffsetZ;





	rotate_obj(AnimAngleX[Bank] + AnimGlobalAngleX,AnimAngleY[Bank] + AnimGlobalAngleY,0,&r_matrix);

	matrix_transformZMY((struct Matrix31*)&pelvis_after,&r_matrix, (struct Matrix31*)&pelvis);


	fcol=CurrentFrame[Bank]->Fight;
	while(fcol)
	{
		if(count==FightColBank)
		{
			FightingColPtr=fcol;
			edit_fcol=fcol;
		}
		fcol=fcol->Next;
		count++;
	}
	if(FightColBank>=count)
	{
		FightColBank=count-1;
		edit_fcol=CurrentFrame[Bank]->Fight;
	}
	if(FightingColPtr)
	{
		SLONG	c0;
		EdRect	rect;
		for(c0=0;c0<4;c0++)
		{
			rect.SetRect(410,10+(3-c0)*20,16,16);
			rect.HiliteRect(LOLITE_COL,HILITE_COL);
			if(FightingColPtr->Height==c0)
				rect.FillRect(0);
		}
		sprintf(str,"DAM: %d",FightingColPtr->Damage);
		QuickTextC(310,10,str,0);
	}
/*
	if(edit_fcol)
	{
		FightingColPtr=edit_fcol;
		if(ShiftFlag)
		{
			if(Keys[KB_MINUS])
				edit_fcol->Dist1--;
			if(Keys[KB_PLUS])
				edit_fcol->Dist1++;
		}
		else
		{
			if(Keys[KB_MINUS])
				edit_fcol->Dist2--;
			if(Keys[KB_PLUS])
				edit_fcol->Dist2++;
		}
		if(ShiftFlag)
		{
			if(Keys[KB_LBRACE])
			{

			}
			if(Keys[KB_RBRACE])
			{
			}
		}
		else
		{
			if(Keys[KB_LBRACE])
			{
				edit_fcol->Angle--;
			}
			if(Keys[KB_RBRACE])
			{
				edit_fcol->Angle++;
			}
		}
	}
*/



	fcol=CurrentFrame[Bank]->Fight;
	count=0;

	while(fcol)
	{
		r1=fcol->Dist1;
		r2=fcol->Dist2;



		x=AnimOffsetX[Bank] + AnimGlobalOffsetX+pelvis_after.X;
		y=AnimOffsetY[Bank] + AnimGlobalOffsetY+pelvis_after.Y;
		z=100+pelvis_after.Z;

		tx=engine.X;
		ty=engine.Y;
		tz=engine.Z;

		temp_scale	=	engine.Scale;
		engine.Scale	=	AnimScale;
		engine.X=-x<<8;
		engine.Y=-y<<8;
		engine.Z=-z<<8;

		angle_step=2048/12;
		if(count==FightColBank)
		{
			col=RED_COL;
		}
		else
		{
			col=GREY_COL;
		}

		for(angle=angle_step>>1;angle<(2048);angle+=angle_step)
		{
			SLONG	dx,dz;
			struct	SVector		offset,res,res2;

			dx=(COS(angle&2047)*300)>>16;
			dz=(SIN(angle&2047)*300)>>16;

			offset.X=dx;
			offset.Y=dy;
			offset.Z=dz;

			matrix_transformZMY((struct Matrix31*)&res,&r_matrix, (struct Matrix31*)&offset);
			offset.X=0;
			offset.Y=dy;
			offset.Z=0;

			matrix_transformZMY((struct Matrix31*)&res2,&r_matrix, (struct Matrix31*)&offset);
			create_bucket_3d_line_whole(res2.X,res2.Y,res2.Z,res.X,res.Y,res.Z,col);
		}
		angle=fcol->Angle<<3;
		for(angle=-10;angle<(10);angle+=3)
		{
			SLONG	dx,dz;
			struct	SVector		offset,res,res2;

			dx=(COS((angle+(fcol->Angle<<3)+2048)&2047)*300)>>16;
			dz=(SIN((angle+(fcol->Angle<<3)+2048)&2047)*300)>>16;

			offset.X=dx;
			offset.Y=dy;
			offset.Z=dz;

			matrix_transformZMY((struct Matrix31*)&res,&r_matrix, (struct Matrix31*)&offset);
			offset.X=0;
			offset.Y=dy;
			offset.Z=0;

			matrix_transformZMY((struct Matrix31*)&res2,&r_matrix, (struct Matrix31*)&offset);
			create_bucket_3d_line_whole(res2.X,res2.Y,res2.Z,res.X,res.Y,res.Z,col);
		}

		draw_3d_mat_circle(0,dy,0,r1,&r_matrix,col);
		draw_3d_mat_circle(0,dy,0,r2,&r_matrix,col);
		draw_3d_mat_circle(0,dy,0,300,&r_matrix,col);


		engine.X=tx;
		engine.Y=ty;
		engine.Z=tz;
		engine.Scale	=	temp_scale;

		fcol=fcol->Next;
		dy-=20;
		count++;
	}
}

void	KeyFrameEditor::DoAnimRecurse(SLONG part_number, struct Matrix33 *mat, SLONG start_object,
									  struct Matrix31 *parent_pos, struct Matrix33 *parent_mat,
									  KeyFrameElement *parent_element)
{
	struct KeyFrameElement	*the_element,
							*the_next_element;

	struct	Matrix31		end_pos;
	struct	Matrix33		end_mat;

	SLONG	object_offset;
	object_offset=test_chunk->PeopleTypes[PersonID].BodyPart[part_number];

	if(CurrentFrame[Bank]->Fixed==part_number)
		local_object_flags=FACE_FLAG_OUTLINE;

	the_element			=	&CurrentFrame[Bank]->FirstElement[part_number]; //vue part
	if(CurrentFrame[Bank]->NextFrame)
		the_next_element	=	&CurrentFrame[Bank]->NextFrame->FirstElement[part_number];
	else
		the_next_element	=	the_element;

/*	// debug - make sure the bone doesn't scale
	if ((body_part_parent_numbers[part_number] != -1) && (CurrentFrame[Bank]->NextFrame))
	{
		struct KeyFrameElement	*parent_next_element;
		parent_next_element  =	&CurrentFrame[Bank]->NextFrame->FirstElement[body_part_parent_numbers[part_number]];

		float a, b, c, d1, d2;
		a = float(the_element->OffsetX - parent_element->OffsetX);
		b = float(the_element->OffsetY - parent_element->OffsetY);
		c = float(the_element->OffsetZ - parent_element->OffsetZ);
		d1 = sqrt(a*a + b*b + c*c);

		a = float(the_next_element->OffsetX - parent_next_element->OffsetX);
		b = float(the_next_element->OffsetY - parent_next_element->OffsetY);
		c = float(the_next_element->OffsetZ - parent_next_element->OffsetZ);
		d2 = sqrt(a*a + b*b + c*c);

		ASSERT(fabs(d1 - d2) / d1 < 0.2f);
	}*/

	SLONG	x, y, z;

	x = AnimOffsetX[Bank];
	y = AnimOffsetY[Bank];
	z = AnimOffsetZ[Bank];

	SLONG	centrex;
	SLONG	centrey;
	SLONG	centrez;

//	SLONG	centrex = (AnimOffsetX[0] + AnimOffsetX[1]) / 2;
//	SLONG	centrey = (AnimOffsetY[0] + AnimOffsetY[1]) / 2;
//	SLONG	centrez = (AnimOffsetZ[0] + AnimOffsetZ[1]) / 2;

	if (DontDrawBoth)
	{
		centrex = 50;
		centrey = 0;
		centrez = 0;
	}
	else
	{
		centrex = 0;
		centrey = 0;
		centrez = 0;
	}


	SLONG	ox, oy, oz;
	SLONG	sn, cs;
	
	sn = SIN(AnimGlobalAngleY & (2048-1));
	cs = COS(AnimGlobalAngleY & (2048-1));
	ox = x - centrex;
	oz = z - centrez;
	
	x = centrex + ((ox * cs) >> 16) + ((oz * sn) >> 16);
	z = centrez - ((ox * sn) >> 16) + ((oz * cs) >> 16);

	sn = SIN(AnimGlobalAngleX & (2048-1));
	cs = COS(AnimGlobalAngleX & (2048-1));
	oy = y - centrey;
	oz = z - centrez;
	
	y = centrey + ((oy * cs) >> 16) - ((oz * sn) >> 16);
	z = centrez + ((oy * sn) >> 16) + ((oz * cs) >> 16);

	x += AnimGlobalOffsetX;
	y += AnimGlobalOffsetY;

	test_draw(start_object+object_offset,x, y, z + 100,AnimTween[Bank],
			  the_element,the_next_element, 
		      mat,
			  parent_pos, parent_mat, parent_element,
			  &end_pos, &end_mat);

	local_object_flags=0;

	// and do my children
	SLONG	c0 = 0;
	while (body_part_children[part_number][c0] != -1)
	{
		DoAnimRecurse(body_part_children[part_number][c0], mat, start_object, &end_pos, &end_mat, the_element);
		c0++;
	};

}

void	KeyFrameEditor::DoHierarchicalAnim()
{
	SLONG	start_object;
	SLONG	c1;
	struct Matrix33			r_matrix;

//	rotate_obj(AnimAngleX[Bank],AnimAngleY[Bank],0,&r_matrix);
	rotate_obj(AnimAngleX[Bank] + AnimGlobalAngleX,AnimAngleY[Bank] + AnimGlobalAngleY, 0, &r_matrix);

	if(CurrentFrame[Bank]->FirstElement==0)
		return;

	start_object=prim_multi_objects[test_chunk->MultiObject].StartObject;

	DoAnimRecurse(0, &r_matrix, start_object, NULL, NULL, NULL);
}

void	KeyFrameEditor::DoCurrentAnim(void)
{
	SLONG					animate	=	0,
							c0,c1,
							temp_scale;
	struct Matrix33			r_matrix;
	struct KeyFrameElement	*the_element,
							*the_next_element;
	SLONG	start_object;


	if(PlayingAnim[Bank]!=CurrentAnim[Bank])
	{
		CurrentFrame[Bank]	=	0;
	}

	if(CurrentFrame[Bank]==0)
	{
		PlayingAnim[Bank]		=	CurrentAnim[Bank];
		CurrentFrame[Bank]	=	PlayingAnim[Bank]->GetFrameList();
		if(CurrentFrame[Bank]==0)
			return;

		/*
		//
		// Clearly this is bollocks, if you ofset frame is 0 , c0 will always be non zero
		//
		c0	=	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->GetCurrentValue();
		while(c0--&&CurrentFrame[Bank]->NextFrame)
		{
			CurrentFrame[Bank]	=	CurrentFrame[Bank]->NextFrame;
		}
		if(c0)
			((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetCurrentValue(0);
			*/

	}


	//
	// go to next keyframe if animtween >255
	//
	{
		SLONG	bank;

		for(bank=0;bank<2;bank++)
		{
			if(CurrentFrame[bank]&&CurrentFrame[bank]->FirstElement) //&&CurrentFrame[Bank]->PrevFrame) //MD
//jcl		if(AnimTween[bank]>255)
			while(AnimTween[bank]>255)
			{
				AnimTween[bank]	-=	255;
				if (CurrentFrame[bank]) // jcl - I can't find where the bank wraps, so I'll just duplicate the last frame in this obscure case...
					CurrentFrame[bank]	=	CurrentFrame[bank]->NextFrame;
			}
//jcl		if(AnimTween[bank]<0)
			while(AnimTween[bank]<0)
			{
				AnimTween[bank]	+=	255;
				if (CurrentFrame[bank])
					CurrentFrame[bank]	=	CurrentFrame[bank]->PrevFrame;
			}
		}
	}					 

	if(CurrentFrame[Bank]&&CurrentFrame[Bank]->FirstElement) //&&CurrentFrame[Bank]->PrevFrame) //MD
	{
		rotate_obj(AnimAngleX[Bank] + AnimGlobalAngleX,AnimAngleY[Bank] + AnimGlobalAngleY,0,&r_matrix);
//		rotate_obj(0,AnimAngleY[Bank],0,&r_matrix);
//		LogText(" current anim  anglex %d angley %d \n",AnimAngleX[Bank],AnimAngleY[Bank]);

		if(CurrentFrame[Bank])
			if(CurrentFrame[Bank]->Fight)
				DrawCombatEditor();

		{
			SLONG	tx,ty,tz;
			SVECTOR	p,p1,p2;
			tx=engine.X;
			ty=engine.Y;
			tz=engine.Z;

			temp_scale	=	engine.Scale;
			engine.Scale	=	AnimScale;
			engine.X=0;
			engine.Y=0;
			engine.Z=0;

			rotate_obj(AnimAngleX[Bank] + AnimGlobalAngleX,AnimAngleY[Bank] + AnimGlobalAngleY,0,&r_matrix);

			p.X=-30;
			p.Y=0;
			p.Z=0;
			matrix_transformZMY((struct Matrix31*)&p1,&r_matrix, (struct Matrix31*)&p);

			p.X=30;
			p.Y=0;
			p.Z=0;
			matrix_transformZMY((struct Matrix31*)&p2,&r_matrix, (struct Matrix31*)&p);
			create_bucket_3d_line_whole(p1.X,p1.Y,p1.Z,p2.X,p2.Y,p2.Z,0);

			p.Y=-30;
			p.X=0;
			p.Z=0;
			matrix_transformZMY((struct Matrix31*)&p1,&r_matrix, (struct Matrix31*)&p);

			p.Y=30;
			p.X=0;
			p.Z=0;
			matrix_transformZMY((struct Matrix31*)&p2,&r_matrix, (struct Matrix31*)&p);
			create_bucket_3d_line_whole(p1.X,p1.Y,p1.Z,p2.X,p2.Y,p2.Z,0);

			p.Z=-30;
			p.Y=0;
			p.X=0;
			matrix_transformZMY((struct Matrix31*)&p1,&r_matrix, (struct Matrix31*)&p);

			p.Z=30;
			p.Y=0;
			p.X=0;
			matrix_transformZMY((struct Matrix31*)&p2,&r_matrix, (struct Matrix31*)&p);
			create_bucket_3d_line_whole(p1.X,p1.Y,p1.Z,p2.X,p2.Y,p2.Z,0);

			engine.X=tx;
			engine.Y=ty;
			engine.Z=tz;
		}

#ifdef	CUNNING_SORT
		if(CurrentFrame[Bank])
		{
			SLONG	o_count,c2,index,az;
			
			temp_scale	=	engine.Scale;
			engine.Scale	=	AnimScale;

			o_count=prim_multi_objects[test_chunk->MultiObject].EndObject-prim_multi_objects[test_chunk->MultiObject].StartObject;
			index=link_head;
			az=build_sort_order_tweened(test_chunk->MultiObject,CurrentFrame[Bank],AnimOffsetX[Bank] + AnimGlobalOffsetX,AnimOffsetY[Bank] + AnimGlobalOffsetY,100,AnimTween[Bank],&r_matrix);
//			LogText(" draw ORDER \n[");
			for(c2=0;c2<=o_count;c2++)
			{
				c1=link_sorts[index].Item;
				c0=c1+prim_multi_objects[test_chunk->MultiObject].StartObject;
//				LogText("%d ",c1);	
				the_element			=	&CurrentFrame[Bank]->FirstElement[c1];
				if(VideoMode==VIDEO_MODE_PREV_FRAME||VideoMode==VIDEO_MODE_PAUSE_BACK)
				{
					if(CurrentFrame[Bank]->PrevFrame)
						the_next_element	=	&CurrentFrame[Bank]->PrevFrame->FirstElement[c1];
					else
						the_next_element	=	the_element;

				}
				else
				{
					if(CurrentFrame[Bank]->NextFrame)
						the_next_element	=	&CurrentFrame[Bank]->NextFrame->FirstElement[c1];
					else
						the_next_element	=	the_element;
				}

				test_draw_z(az+(c2<<2),c0,AnimOffsetX[Bank] + AnimGlobalOffsetX,AnimOffsetY[Bank] + AnimGlobalOffsetY,100,AnimTween[Bank],the_element,the_next_element,&r_matrix);
				index=link_sorts[index].Next;
			}
//				LogText("]\n");	
			engine.Scale	=	temp_scale;
		}
#else
		if(CurrentFrame[Bank])
		{
			temp_scale	=	engine.Scale;
			engine.Scale	=	AnimScale;
//			for(c1=0,c0=prim_multi_objects[TestChunk->MultiObject].StartObject;c0<=prim_multi_objects[TestChunk->MultiObject].EndObject;c0++,c1++)


			if (the_editor->GetQuaternionFlag() &&
				(test_chunk->ElementCount == 15))
				DoHierarchicalAnim();
			else
			{
				start_object=prim_multi_objects[test_chunk->MultiObject].StartObject;
				for(c1=0;c1<test_chunk->ElementCount;c1++)
				{
					SLONG	object_offset;
					if(CurrentFrame[Bank]->FirstElement==0)
						goto error;
					//
					// for each vue frame we need an object to draw
					//
					if(test_chunk->ElementCount==15)
						object_offset=test_chunk->PeopleTypes[PersonID].BodyPart[c1];
					else
						object_offset=c1;

	//				object_offset=test_chunk->PeopleTypes[PersonID].BodyPart[c1];

					the_element			=	&CurrentFrame[Bank]->FirstElement[c1]; //vue part
					if(CurrentFrame[Bank]->NextFrame)
						the_next_element	=	&CurrentFrame[Bank]->NextFrame->FirstElement[c1];
					else
						the_next_element	=	the_element;

					if(CurrentFrame[Bank]->Fixed==c1)
						local_object_flags=FACE_FLAG_OUTLINE;

	//   				test_draw(start_object+object_offset,0,0,0,0,the_element,the_element,r_matrix);
	//				test_draw(start_object+object_offset,AnimOffsetX[Bank],AnimOffsetY[Bank],100,AnimTween[Bank],the_element,the_next_element,&r_matrix, NULL, NULL);

/*					// JCL - work out the parent of the element.
					SLONG	parent = body_part_parent_numbers[c1];

					struct KeyFrameElement	*parent_element, *parent_next_element;

					if (parent == -1)
					{
						parent_element = NULL;
					}
					else
					{
						parent_element = &CurrentFrame[Bank]->FirstElement[parent]; //vue part
						if(CurrentFrame[Bank]->NextFrame)
							parent_next_element	=	&CurrentFrame[Bank]->NextFrame->FirstElement[parent];
						else
							parent_next_element	=	the_element;
					}*/
					
					test_draw(start_object+object_offset,AnimOffsetX[Bank] + AnimGlobalOffsetX,AnimOffsetY[Bank] + AnimGlobalOffsetY,100,AnimTween[Bank],the_element,the_next_element,&r_matrix, NULL, NULL, NULL, NULL, NULL);

					local_object_flags=0;
				}
			}


/*
			for(c1=0,c0=prim_multi_objects[test_chunk->MultiObject].StartObject;c0<=prim_multi_objects[test_chunk->MultiObject].EndObject;c0++,c1++)
			{
				the_element			=	&CurrentFrame[Bank]->FirstElement[c1];
				if(CurrentFrame[Bank]->NextFrame)
					the_next_element	=	&CurrentFrame[Bank]->NextFrame->FirstElement[c1];
				else
					the_next_element	=	the_element;

				if(CurrentFrame[Bank]->Fixed==c1)
					local_object_flags=FACE_FLAG_OUTLINE;
				test_draw(c0,AnimOffsetX[Bank],AnimOffsetY[Bank],100,AnimTween[Bank],the_element,the_next_element,&r_matrix);
				local_object_flags=0;
			}
*/
			engine.Scale	=	temp_scale;
		}
#endif
	}
			{
				CBYTE	str[90];
				if(CurrentFrame[Bank])
					sprintf(str,"%d %d",AnimTween[Bank],CurrentFrame[Bank]->FrameID);
				else
					sprintf(str,"%d %d",AnimTween[Bank],0);
				QuickTextC(180,0,str,0);
			}
error:;
}

extern	UWORD	is_it_clockwise(struct SVector *global_res,SLONG p0,SLONG p1,SLONG p2);
/*
UWORD	is_it_clockwise(struct SVector *global_res,SLONG p0,SLONG p1,SLONG p2)
{
	SLONG	z;
	SLONG	vx,vy,wx,wy;

	vx=global_res[p1].X-global_res[p0].X;
	wx=global_res[p2].X-global_res[p1].X;
	vy=global_res[p1].Y-global_res[p0].Y;
	wy=global_res[p2].Y-global_res[p1].Y;

	z=vx*wy-vy*wx;

	if(z>0)
		return	1;
	else
		return	0;
}
*/

//---------------------------------------------------------------
/*

  GUY I HAVE CHANGED HOW X,Y,Z ARE USED, SO IF YOU ARE CALLING THIS FUNCTION ELSE WHERE THEN
  IT WILL BE WRONG

*/

//JCL - not needed here #define	PI	(3.14159625)

void MATRIX_calc(float *matrix, float yaw, float pitch, float roll)
{
	float cy, cp, cr;
	float sy, sp, sr;

	cy = cos(yaw);
	cp = cos(pitch);
	cr = cos(roll);
	
	sy = sin(yaw);
	sp = sin(pitch);
	sr = sin(roll);

	//
	// Jan I trust you... but only becuase I've already seen it working!
	//

	matrix[0] =  cy * cr + sy * sp * sr;
	matrix[3] =  cy * sr - sy * sp * cr;
	matrix[6] =  sy * cp;
	matrix[1] = -cp * sr;
	matrix[4] =  cp * cr;
	matrix[7] =  sp;
	matrix[2] = -sy * cr + cy * sp * sr;
	matrix[5] = -sy * sr - cy * sp * cr;
	matrix[8] =  cy * cp;
}

#define MATRIX_FA_VECTOR_TOO_SMALL (0.0001F)
#define MATRIX_FA_ANGLE_TOO_SMALL  (PI / 36000.0F)
#include <math.h>

void	MATRIX_find_angles(float *matrix,float *yaw,float *pitch,float * roll)
{
	float x;
	float y;
	float z;
	float xz;


	//
	// Look from above at the z-vector to work out the yaw.
	//

	x = matrix[6];
	y = matrix[7];
	z = matrix[8];

	if (fabs(x) + fabs(z) < MATRIX_FA_VECTOR_TOO_SMALL)
	{
		*yaw = 0.0F;
	}
	else
	{
	   *yaw = (float)atan2(x, z);
	}

	//
	// Look down the x vector to at the z-vector to work out the pitch.
	//

	xz = sqrt(x*x + z*z);

	if (fabs(xz) + fabs(y) < MATRIX_FA_VECTOR_TOO_SMALL)
	{
		if (y < 0) 
		{
			*pitch = -PI;
		}
		else
		{
			*pitch = +PI;
		}
	}
	else
	{
		*pitch = (float)atan2(y, xz);
	}

	//
	// Now... matrix[4] =  cos(pitch) * cos(roll)
	//		  matrix[1] = -cos(pitch) * sin(roll)
	//
	// so...  cos(roll) = matrix[4] /  cos(pitch)
	//        sin(roll) = matrix[1] / -cos(pitch)
	//
	
	float cos_roll;
	float sin_roll;
	float cos_pitch;

	cos_pitch = (float)cos(*pitch);

	if (fabs(cos_pitch) < MATRIX_FA_ANGLE_TOO_SMALL)
	{
		*roll = 0.0F;
	}
	else
	{
		cos_roll = matrix[4] /  cos_pitch;
		sin_roll = matrix[1] / -cos_pitch;
		*roll = (float)atan2(sin_roll, cos_roll);
	}

}

void	convert_mat_to_float(float *mat_f,Matrix33 *mat_m)
{
	//LogText(" mat (%d,%d,%d)(%d,%d,%d)(%d,%d,%d)== ",mat_m->M[0][0],mat_m->M[0][1],mat_m->M[0][2],mat_m->M[1][0],mat_m->M[1][1],mat_m->M[1][2],mat_m->M[2][0],mat_m->M[2][1],mat_m->M[2][2]);

	mat_f[0]=((float)mat_m->M[0][0])/32768.0;
	mat_f[1]=((float)mat_m->M[0][1])/32768.0;
	mat_f[2]=((float)mat_m->M[0][2])/32768.0;
	mat_f[3]=((float)mat_m->M[1][0])/32768.0;
	mat_f[4]=((float)mat_m->M[1][1])/32768.0;
	mat_f[5]=((float)mat_m->M[1][2])/32768.0;
	mat_f[6]=((float)mat_m->M[2][0])/32768.0;
	mat_f[7]=((float)mat_m->M[2][1])/32768.0;
	mat_f[8]=((float)mat_m->M[2][2])/32768.0;
	//LogText(" mat (%f,%f,%f)(%f,%f,%f)(%f,%f,%f) \n",mat_f[0],mat_f[1],mat_f[2],mat_f[3],mat_f[4],mat_f[5],mat_f[6],mat_f[7],mat_f[8]);
}

void	convert_float_to_mat(Matrix33 *mat_m,float *mat_f)
{
	//LogText(" mat (%f,%f,%f)(%f,%f,%f)(%f,%f,%f)== ",mat_f[0],mat_f[1],mat_f[2],mat_f[3],mat_f[4],mat_f[5],mat_f[6],mat_f[7],mat_f[8]);
	mat_m->M[0][0]=(SLONG)(mat_f[0]*32768.0);
	mat_m->M[0][1]=(SLONG)(mat_f[1]*32768.0);
	mat_m->M[0][2]=(SLONG)(mat_f[2]*32768.0);
	              	        		
	mat_m->M[1][0]=(SLONG)(mat_f[3]*32768.0);
	mat_m->M[1][1]=(SLONG)(mat_f[4]*32768.0);
	mat_m->M[1][2]=(SLONG)(mat_f[5]*32768.0);
	              	        		
	mat_m->M[2][0]=(SLONG)(mat_f[6]*32768.0);
	mat_m->M[2][1]=(SLONG)(mat_f[7]*32768.0);
	mat_m->M[2][2]=(SLONG)(mat_f[8]*32768.0);
	//LogText(" mat (%d,%d,%d)(%d,%d,%d)(%d,%d,%d)\n ",mat_m->M[0][0],mat_m->M[0][1],mat_m->M[0][2],mat_m->M[1][0],mat_m->M[1][1],mat_m->M[1][2],mat_m->M[2][0],mat_m->M[2][1],mat_m->M[2][2]);
}

float	tween_angles(float a1,float a2,SLONG tween)
{
	float da;
	UBYTE	log=0;
	// angles go from -pi to +pi


	da=a2-a1;
//	if(da)
//		log=1;
	if(log)
		LogText(" a1 %f a2 %f da %f tween %d \n",a1,a2,da,tween);

	if(da<-PI)
	{
		if(log)
			LogText(" Dangle over-a %f ->",da);
		da=(2.0*PI+da); // 180-181  (-181 should be +179
		if(log)
			LogText(" %f \n",da);
	}
	if(da>PI)
	{
		if(log)
			LogText(" Dangle over-b %f ->",da);
		da= -(2.0*PI-da);   //181 should be -179
		if(log)
			LogText(" %f \n",da);
	}

	if(log)
		LogText("da %f pre tween \n",da);

	da=da*tween;
	if(log)
		LogText("da %f mid tween \n",da);

	da=(da)/256.0;
	if(log)
		LogText("da %f after tween \n",da);
	a1+=da;

	if(a1>PI)
	{
		if(log)
			LogText(" a1 over-a %f ->",a1);
		a1-=2.0*PI;
		if(log)
			LogText(" %f \n",a1);
	}
	if(a1<-PI)
	{
		if(log)
			LogText(" a1 over-a %f ->",a1);
		a1+=2.0*PI;
		if(log)
			LogText(" %f \n",a1);
	}
	if(log)
		LogText("res %f \n",a1);

	return(a1);

}
/*
void	build_tween_matrix(Matrix33 *mat_res,Matrix33 *mat1,Matrix33 *mat2,SLONG tween)
{
	float	mata[9],yaw1,pitch1,roll1,yaw2,pitch2,roll2;

	
	convert_mat_to_float(mata,mat1);
	MATRIX_find_angles(mata,&yaw1,&pitch1,&roll1);

	convert_mat_to_float(mata,mat2);
	MATRIX_find_angles(mata,&yaw2,&pitch2,&roll2);

	yaw1=tween_angles(yaw1,yaw2,tween);
	pitch1=tween_angles(pitch1,pitch2,tween);
	roll1=tween_angles(roll1,roll2,tween);

//	LogText(" y p r %f %f %f \n",yaw1,pitch1,roll1);

	MATRIX_calc(mata,yaw1,pitch1,roll1);
	convert_float_to_mat(mat_res,mata);

	//LogText(" yaw %f->%f, pitch %f->%f, roll %f->%f\n",yaw1,yaw2,pitch1,pitch2,roll1,roll2);

}
*/

// void	draw_a_rot_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct	PrimMultiAnim *anim_info,struct Matrix33 *rot_mat)
//void	KeyFrameEditor::test_draw(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,struct Matrix33 *rot_mat)
void	test_draw(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,
				  struct KeyFrameElement *anim_info,struct KeyFrameElement *anim_info_next,
				  struct Matrix33 *rot_mat,
				  struct Matrix31 *parent_pos,  struct Matrix33 *parent_mat,
				  KeyFrameElement *parent_element,
				  struct Matrix31 *end_pos,		struct Matrix33 *end_mat
				  )
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	flags[1560];
	ULONG	flag_and,flag_or;
	struct	SVector			res[1560],temp; //max points per object?
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG az;
	struct	Matrix33 *mat,*mat_next,mat2,mat_final;
	SLONG	i,j;
//	struct	KeyFrameElement *anim_info_next;
	struct	Matrix31	offset;
	SLONG	tx,ty,tz;
	SLONG	len,len2;

	p_obj    =&prim_objects[prim];
	p_f4     =&prim_faces4[p_obj->StartFace4];
	p_f3     =&prim_faces3[p_obj->StartFace3];
	
//	anim_info_next	=	&TheElements[anim_info->Next];
//	anim_info_next	=	&&TestChunkTestChunk->KeyFrames[3].FirstElement;

	mat      = &anim_info->Matrix;
	mat_next = &anim_info_next->Matrix;

	//move object "tweened quantity"  , z&y flipped
	

	//!JCL - temp - use the quaternion from the "parent" node to generate the position.
	// or don't bother if we haven't got a parent...
	if ((!parent_pos) || (!parent_mat) || (!parent_element) || (the_editor->GetQuaternionFlag() == 0))
	{
		offset.M[0]	=	(anim_info->OffsetX+(((anim_info_next->OffsetX-anim_info->OffsetX)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
		offset.M[1]	=	(anim_info->OffsetY+(((anim_info_next->OffsetY-anim_info->OffsetY)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
		offset.M[2]	=	(anim_info->OffsetZ+(((anim_info_next->OffsetZ-anim_info->OffsetZ)*tween)>>8))>>TWEEN_OFFSET_SHIFT;

		if (end_pos)
		{
			*end_pos = offset;

			// ramp up the accuracy
			end_pos->M[0] *=256;
			end_pos->M[1] *=256;
			end_pos->M[2] *=256;
		}

		offset.M[0] *= 256;
		offset.M[1] *= 256;
		offset.M[2] *= 256;
	}
	else
	{

		HIERARCHY_Get_Body_Part_Offset(&offset, (Matrix31 *)(&anim_info->OffsetX),
									   &parent_element->CMatrix, (Matrix31 *)(&parent_element->OffsetX),
									   parent_mat, parent_pos);

		if (end_pos)
			*end_pos = offset;
	}

	offset.M[0] >>= 2; // otherwise the matrix rotation overflows...
	offset.M[1] >>= 2;
	offset.M[2] >>= 2;

	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
	x = (x * 256) + (temp.X << 2);
	y = (y * 256) + (temp.Y << 2);
	z = (z * 256) + (temp.Z << 2);

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;

	tx=engine.X;
	ty=engine.Y;
	tz=engine.Z;

//	engine.X=-x<<8;
//	engine.Y=-y<<8;
//	engine.Z=-z<<8;
	engine.X=-x;
	engine.Y=-y;
	engine.Z=-z;

	SLONG q = the_editor->GetQuaternionFlag();
	if ( q == 0)
//	if (1)
	{
		build_tween_matrix(&mat2,&anim_info->CMatrix,&anim_info_next->CMatrix,tween);
	}
	else
	{
		//JCL - temp - use quaternions to interpolate matrices
		CQuaternion::BuildTween(&mat2,&anim_info->CMatrix,&anim_info_next->CMatrix,tween);
	}

	if (end_mat)
		*end_mat = mat2;

	matrix_mult33(&mat_final,rot_mat,&mat2);

		
	for(c0=sp;c0<ep;c0++)
	{

		matrix_transform_small((struct Matrix31*)&temp,&mat_final, (struct SMatrix31*)&prim_points[c0]);
//		matrix_transform((struct Matrix31*)&temp,&mat2, (struct Matrix31*)&prim_points[c0]);
//      matrix now does the yz flip
//		swap(temp.Y,temp.Z)  //MDOPT do this inside the matrix multiply
//		temp.Y=-temp.Y;

		
		flags[c0-sp]	=	rotate_point_gte((struct SVector*)&temp,&res[c0-sp]);
//		LogText(" after rot (%d,%d,%d)  \n",res[c0-sp].X,res[c0-sp].Y,res[c0-sp].Z);
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
		if(!is_it_clockwise(res,p0,p1,p2))
		   goto	next_face4;


		flag_and	=	flags[p0]&flags[p1]&flags[p2]&flags[p3];	
		flag_or		=	flags[p0]|flags[p1]|flags[p2]|flags[p3];	
//		LogText(" test draw face %d  p0 x (%d,%d,%d)\n",c0,res[p0].X,res[p0].Y,res[p0].Z);
//		DrawLineC(engine.VW2,engine.VH2,res[p0].X,res[p0].Y,0);
//		if((flag_or&EF_BEHIND_YOU)==0)
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
			if(p_f4->Col2>256)
				LogText(" material test draw poly col %d co %d\n",(SLONG)p_f4->Col2,c0);

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
			ASSERT(p_f4->TexturePage>=0);

			setZ4((struct BucketQuad*)current_bucket_pool,-res[p0].Z,-res[p1].Z,-res[p2].Z,-res[p3].Z);
			
			setShade4	(
							(struct BucketQuad*)current_bucket_pool,
							(p_f4->Bright[0]),
							(p_f4->Bright[1]),
							(p_f4->Bright[2]),
							(p_f4->Bright[3])
						);
			((struct BucketQuad*)current_bucket_pool)->DebugInfo=c0;
			((struct BucketQuad*)current_bucket_pool)->DebugFlags=local_object_flags;

			add_bucket((void *)current_bucket_pool,az);

			current_bucket_pool+=sizeof(struct BucketQuad);
		}
next_face4:;
		p_f4++;
	}

	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		p0=p_f3->Points[0]-sp;
		p1=p_f3->Points[1]-sp;
		p2=p_f3->Points[2]-sp;
//		LogText(" test draw p012 %d %d %d \n",p0,p1,p2);
		if(p0<0||p1<0||p2<0)
		{
			ERROR_MSG(0," point before start_point in test_draw");

			return;
		}
		if(!is_it_clockwise(res,p0,p1,p2))
		   goto	next_face3;

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
			ASSERT(p_f3->TexturePage>=0);

			setShade3	(
							(struct BucketTri*)current_bucket_pool,
							(p_f3->Bright[0]),
							(p_f3->Bright[1]),
							(p_f3->Bright[2])
						);

			((struct BucketTri*)current_bucket_pool)->DebugInfo=c0;
			((struct BucketTri*)current_bucket_pool)->DebugFlags=p_f3->FaceFlags;

			add_bucket((void *)current_bucket_pool,az);

			current_bucket_pool+=sizeof(struct BucketQuad);
		}
next_face3:;
		p_f3++;
	}					
}

void	test_draw_all_get_sizes(SWORD multi_prim,struct KeyFrame *the_frame,SLONG x,SLONG y,SLONG z,SLONG tween,struct Matrix33 *rot_mat,SLONG *width,SLONG *height,SLONG *mid_x,SLONG *mid_y)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	flags[1560];
	ULONG	flag_and,flag_or;
	struct	SVector			res[1560],temp; //max points per object?
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG az;
	struct	Matrix33 *mat,*mat_next,mat2,mat_final;
	struct KeyFrameElement	*the_element;
	SLONG	i,j;
//	struct	KeyFrameElement *anim_info_next;
	struct	Matrix31	offset;
	SLONG	tx,ty,tz;

	SLONG	max_x,max_y;
	SLONG	min_x,min_y;
	SLONG	mx=0,my=0,count=0;

	SLONG	c0,c1,c2;
	UWORD	prim;

	min_x	=	min_y	=	999999;
	max_x	=	max_y	=	-999999;

	tx=engine.X;
	ty=engine.Y;
	tz=engine.Z;
	if(the_frame->FirstElement==0)
		return;

	for(c2=0,c1=prim_multi_objects[multi_prim].StartObject;c1<prim_multi_objects[multi_prim].EndObject;c1++)
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

			matrix_transform_small((struct Matrix31*)&temp,&mat_final, (struct SMatrix31*)&prim_points[c0]);
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

void	test_draw_game(UWORD	prim,SLONG x,SLONG y,SLONG z,SLONG tween,struct GameKeyFrameElement *anim_info,struct GameKeyFrameElement *anim_info_next,struct Matrix33 *rot_mat)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	flags[1560];
	ULONG	flag_and,flag_or;
	struct	SVector			res[1560],temp; //max points per object?
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG az;
	struct	Matrix33 *mat,*mat_next,mat2,mat_final;
	SLONG	i,j;
//	struct	KeyFrameElement *anim_info_next;
	struct	Matrix31	offset;
	SLONG	tx,ty,tz;
	SLONG	len,len2;

	p_obj    =&prim_objects[prim];
	p_f4     =&prim_faces4[p_obj->StartFace4];
	p_f3     =&prim_faces3[p_obj->StartFace3];
	
//	anim_info_next	=	&TheElements[anim_info->Next];
//	anim_info_next	=	&&TestChunkTestChunk->KeyFrames[3].FirstElement;

	//mat      = &anim_info->CMatrix;
	//mat_next = &anim_info_next->CMatrix;

	//move object "tweened quantity"  , z&y flipped
	

	offset.M[0]	=	(anim_info->OffsetX+(((anim_info_next->OffsetX-anim_info->OffsetX)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[1]	=	(anim_info->OffsetY+(((anim_info_next->OffsetY-anim_info->OffsetY)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
	offset.M[2]	=	(anim_info->OffsetZ+(((anim_info_next->OffsetZ-anim_info->OffsetZ)*tween)>>8))>>TWEEN_OFFSET_SHIFT;
/*	
	len=SDIST3(anim_info->OffsetX,anim_info->OffsetY,anim_info->OffsetZ);
	len=sqrl(len);

	len2=SDIST3(offset.M[0],offset.M[1],offset.M[2]);
	len2=sqrl(len2);

	offset.M[0]	=	(offset.M[0]	*len)/len2;
	offset.M[1]	=	(offset.M[1]	*len)/len2;
	offset.M[2]	=	(offset.M[2]	*len)/len2;
*/

	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
	x	+= temp.X;
	y	+= temp.Y;
	z	+= temp.Z;

	CMatrix33	m1, m2;
	GetCMatrix(anim_info, &m1);
	GetCMatrix(anim_info_next, &m2);

	build_tween_matrix(&mat2, &m1, &m2 ,tween);

/*
	x+=(anim_info->DX+(((anim_info_next->DX-anim_info->DX)*tween)>>8))>>2;
	y+=(anim_info->DY+(((anim_info_next->DY-anim_info->DY)*tween)>>8))>>2;
	z+=(anim_info->DZ+(((anim_info_next->DZ-anim_info->DZ)*tween)>>8))>>2;
*/
	

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;

//	LogText(" test draw sp %d ep %d \n",sp,ep);

	tx=engine.X;
	ty=engine.Y;
	tz=engine.Z;

	engine.X=-x<<8;
	engine.Y=-y<<8;
	engine.Z=-z<<8;

//	engine.X=temp.X<<8;
  //	engine.Y=temp.Y<<8;
	//engine.Z=temp.Z<<8;

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

	GetCMatrix(anim_info, &m1);
	GetCMatrix(anim_info_next, &m2);
	
	build_tween_matrix(&mat2, &m1, &m2, tween);
	normalise_matrix(&mat2);

//	build_tween_matrix(&mat2,mat,mat_next,tween);
//apply local rotation matrix
	matrix_mult33(&mat_final,rot_mat,&mat2);

		
	for(c0=sp;c0<ep;c0++)
	{

		matrix_transform_small((struct Matrix31*)&temp,&mat_final, (struct SMatrix31*)&prim_points[c0]);
//		matrix_transform((struct Matrix31*)&temp,&mat2, (struct Matrix31*)&prim_points[c0]);
//      matrix now does the yz flip
//		swap(temp.Y,temp.Z)  //MDOPT do this inside the matrix multiply
//		temp.Y=-temp.Y;

		
		flags[c0-sp]	=	rotate_point_gte((struct SVector*)&temp,&res[c0-sp]);
//		LogText(" after rot (%d,%d,%d)  \n",res[c0-sp].X,res[c0-sp].Y,res[c0-sp].Z);
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
		if(!is_it_clockwise(res,p0,p1,p2))
		   goto	next_face4;


		flag_and	=	flags[p0]&flags[p1]&flags[p2]&flags[p3];	
		flag_or		=	flags[p0]|flags[p1]|flags[p2]|flags[p3];	
//		LogText(" test draw face %d  p0 x (%d,%d,%d)\n",c0,res[p0].X,res[p0].Y,res[p0].Z);
//		DrawLineC(engine.VW2,engine.VH2,res[p0].X,res[p0].Y,0);
//		if((flag_or&EF_BEHIND_YOU)==0)
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
			if(p_f4->Col2>256)
				LogText(" material test draw poly col %d co %d\n",(SLONG)p_f4->Col2,c0);

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
			ASSERT(p_f4->TexturePage>=0);

			setZ4((struct BucketQuad*)current_bucket_pool,-res[p0].Z,-res[p1].Z,-res[p2].Z,-res[p3].Z);
			
			setShade4	(
							(struct BucketQuad*)current_bucket_pool,
							(p_f4->Bright[0]),
							(p_f4->Bright[1]),
							(p_f4->Bright[2]),
							(p_f4->Bright[3])
						);
			((struct BucketQuad*)current_bucket_pool)->DebugInfo=c0;
			((struct BucketQuad*)current_bucket_pool)->DebugFlags=local_object_flags;

			add_bucket((void *)current_bucket_pool,az);

			current_bucket_pool+=sizeof(struct BucketQuad);
		}
next_face4:;
		p_f4++;
	}

	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		p0=p_f3->Points[0]-sp;
		p1=p_f3->Points[1]-sp;
		p2=p_f3->Points[2]-sp;
//		LogText(" test draw p012 %d %d %d \n",p0,p1,p2);
		if(p0<0||p1<0||p2<0)
		{
			ERROR_MSG(0," point before start_point in test_draw");

			return;
		}
		if(!is_it_clockwise(res,p0,p1,p2))
		   goto	next_face3;

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
			ASSERT(p_f3->TexturePage>=0);

			setShade3	(
							(struct BucketTri*)current_bucket_pool,
							(p_f3->Bright[0]),
							(p_f3->Bright[1]),
							(p_f3->Bright[2])
						);

			((struct BucketTri*)current_bucket_pool)->DebugInfo=c0;
			((struct BucketTri*)current_bucket_pool)->DebugFlags=p_f3->FaceFlags;

			add_bucket((void *)current_bucket_pool,az);

			current_bucket_pool+=sizeof(struct BucketQuad);
		}
next_face3:;
		p_f3++;
	}					
}

void	test_draw_all_get_sizes_game(SWORD multi_prim,struct GameKeyFrame *the_frame,SLONG x,SLONG y,SLONG z,SLONG tween,struct Matrix33 *rot_mat,SLONG *width,SLONG *height,SLONG *mid_x,SLONG *mid_y)
{
	struct	PrimFace4		*p_f4;
	struct	PrimFace3		*p_f3;
	SLONG	flags[1560];
	ULONG	flag_and,flag_or;
	struct	SVector			res[1560],temp; //max points per object?
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG az;
	struct	Matrix33 *mat,*mat_next,mat2,mat_final;
	struct GameKeyFrameElement	*the_element;
	SLONG	i,j;
//	struct	KeyFrameElement *anim_info_next;
	struct	Matrix31	offset;
	SLONG	tx,ty,tz;

	SLONG	max_x,max_y;
	SLONG	min_x,min_y;
	SLONG	mx=0,my=0,count=0;

	SLONG	c0,c1,c2;
	UWORD	prim;

	min_x	=	min_y	=	999999;
	max_x	=	max_y	=	-999999;

	tx=engine.X;
	ty=engine.Y;
	tz=engine.Z;
	if(the_frame->FirstElement==0)
		return;

	for(c2=0,c1=prim_multi_objects[multi_prim].StartObject;c1<prim_multi_objects[multi_prim].EndObject;c1++)
	{
		the_element			=	&the_frame->FirstElement[c2++];
//   		test_draw(c0,0,0,0,0,the_element,the_element,r_matrix);
		prim=c1;

		p_obj    =&prim_objects[prim];
		p_f4     =&prim_faces4[p_obj->StartFace4];
		p_f3     =&prim_faces3[p_obj->StartFace3];
		

//		mat      = &the_element->Matrix;
//		mat_next = &the_element->Matrix;

		//move object "tweened quantity"  , z&y flipped
		offset.M[0]	=	(the_element->OffsetX)>>TWEEN_OFFSET_SHIFT;
		offset.M[1]	=	(the_element->OffsetY)>>TWEEN_OFFSET_SHIFT;
		offset.M[2]	=	(the_element->OffsetZ)>>TWEEN_OFFSET_SHIFT;
		matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
		x	= temp.X;
		y	= temp.Y;
		z	= temp.Z;
		
		// (jcl) don't blame me - I didn't write this...
		CMatrix33	m1, m2;
		GetCMatrix(the_element, &m1);
		GetCMatrix(the_element, &m1);

		build_tween_matrix(&mat2, &m1, &m2,tween);

		sp=p_obj->StartPoint;
		ep=p_obj->EndPoint;


		engine.X=-x<<8;
		engine.Y=-y<<8;
		engine.Z=-z<<8;
/*
		for(i=0;i<3;i++)
		{
			for(j=0;j<3;j++)
			{
				mat2.M[i][j]=mat->M[i][j]+(((mat_next->M[i][j]-mat->M[i][j])*tween)>>8);
			}
		}
  */
	//apply local rotation matrix
		matrix_mult33(&mat_final,rot_mat,&mat2);

			
		for(c0=sp;c0<ep;c0++)
		{

			matrix_transform_small((struct Matrix31*)&temp,&mat_final, (struct SMatrix31*)&prim_points[c0]);
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

//---------------------------------------------------------------


void	KeyFrameEditor::DrawKeyFrame(UWORD multi_object,EdRect *bounds_rect,struct KeyFrame *the_frame,struct Matrix33 *r_matrix)
{
	SLONG					c0,c1,c2,
							num_points,
							end_point,
							scale,
							scale_y,
							start_point,
							temp_scale,
							temp_x,
							temp_y,
							temp_z,
							temp_vw2,
							temp_vh2,
							width,height,
							*flags;
	EdRect					outline_rect;
	struct KeyFrameElement	*the_element;
	struct Matrix31			offset;
	struct PrimObject		*the_obj;
	struct SVector			*rotate_vectors,
							t_vector,
							t_vector2;

	SLONG	mid_x=0,mid_y=0;
	SLONG	start_object;

	// Stop the compiler moaning.
	multi_object	=	multi_object;

	if(!test_chunk->MultiObject)
		return;

	c1	=	0;
	flags			=	(SLONG*)MemAlloc(sizeof(SLONG)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");
	rotate_vectors	=	(struct SVector*)MemAlloc(sizeof(struct SVector)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");


	temp_scale		=	engine.Scale;
	temp_x			=	engine.X;
	temp_y			=	engine.Y;
	temp_z			=	engine.Z;
	temp_vw2		=	engine.VW2;
	temp_vh2		=	engine.VH2;

	engine.X		=	0;
	engine.Y		=	0;
	engine.Z		=	0;
	engine.Scale	=	5000;
	engine.ShowDebug=	0;
	engine.BucketSize=	MAX_BUCKETS>>4;

	SetWorkWindowBounds	(
							bounds_rect->GetLeft(),
							bounds_rect->GetTop(),
							bounds_rect->GetWidth(),
							bounds_rect->GetHeight()
						);
	outline_rect.SetRect(
							0,0,
							bounds_rect->GetWidth(),bounds_rect->GetHeight()
						);
	set_camera();
//	set_camera_to_base();
	

//md	LogText(" width %d height %d \n",width,height);
	test_draw_all_get_sizes(test_chunk->MultiObject,the_frame,0,0,0,0,r_matrix,&width,&height,&mid_x,&mid_y);
	if(width>0 && height>0)
	{


		scale	=	(bounds_rect->GetWidth()<<16)/width;
		scale_y	=	(bounds_rect->GetHeight()<<16)/height;
		if(scale_y<scale)
			scale	=	scale_y;

//		scale			=	(scale*900)>>8;
		engine.Scale	=	(5000*scale)>>16;

		//calc new mids with this scale
		test_draw_all_get_sizes(test_chunk->MultiObject,the_frame,0,0,0,0,r_matrix,&width,&height,&mid_x,&mid_y);

		mid_x-=bounds_rect->GetWidth()>>1;
		mid_y-=bounds_rect->GetHeight()>>1;
		engine.VW2-=mid_x;
		engine.VH2-=mid_y;

	}
//	LogText(" drawkeyframe scale %d \n",engine.Scale);

	//
	// body part system
	//
	start_object=prim_multi_objects[test_chunk->MultiObject].StartObject;
	for(c2=0;c2<test_chunk->ElementCount;c2++)
	{
		SLONG	object_offset;
		if(the_frame->FirstElement==0)
			goto error;

		//
		// for each vue frame we need an object to draw
		//

		if(test_chunk->ElementCount==15)
			object_offset=test_chunk->PeopleTypes[PersonID].BodyPart[c2];
		else
			object_offset=c2; //test_chunk->PeopleTypes[PersonID].BodyPart[c2];

		the_element			=	&the_frame->FirstElement[c2]; //vue part
   		test_draw(start_object+object_offset,0,0,0,0,the_element,the_element,r_matrix, NULL, NULL, NULL, NULL, NULL);
	}

/*	old system
	for(c2=0,c0=prim_multi_objects[test_chunk->MultiObject].StartObject;c0<=prim_multi_objects[test_chunk->MultiObject].EndObject;c0++)
	{
		if(the_frame->FirstElement==0)
			goto error;
		the_element			=	&the_frame->FirstElement[c2++];
   		test_draw(c0,0,0,0,0,the_element,the_element,r_matrix);
	}
*/
	render_view(0);
	outline_rect.OutlineRect(0);

	engine.X		=	temp_x;
	engine.Y		=	temp_y;
	engine.Z		=	temp_z;
	engine.Scale	=	temp_scale;
	engine.ShowDebug=	1;
	engine.BucketSize=	MAX_BUCKETS;
	engine.VW2	=	temp_vw2;
	engine.VH2	=	temp_vh2;
error:;
	MemFree(rotate_vectors);
	MemFree(flags);
}

void	drawkeyframebox(UWORD multi_object,EdRect *bounds_rect,struct KeyFrame *the_frame,struct Matrix33 *r_matrix,SLONG person_id)
{
	SLONG					c0,c1,c2,
							num_points,
							end_point,
							scale,
							scale_y,
							start_point,
							temp_scale,
							temp_x,
							temp_y,
							temp_z,
							temp_vw2,
							temp_vh2,
							width,height,
							*flags;
	EdRect					outline_rect;
	struct KeyFrameElement	*the_element;
	struct Matrix31			offset;
	struct PrimObject		*the_obj;
	struct SVector			*rotate_vectors,
							t_vector,
							t_vector2;

	SLONG	mid_x=0,mid_y=0;
	SLONG	start_object;

	// Stop the compiler moaning.
	multi_object	=	multi_object;

	if(!test_chunk->MultiObject)
		return;

	c1	=	0;
	flags			=	(SLONG*)MemAlloc(sizeof(SLONG)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");
	rotate_vectors	=	(struct SVector*)MemAlloc(sizeof(struct SVector)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");


	temp_scale		=	engine.Scale;
	temp_x			=	engine.X;
	temp_y			=	engine.Y;
	temp_z			=	engine.Z;
	temp_vw2		=	engine.VW2;
	temp_vh2		=	engine.VH2;

	engine.X		=	0;
	engine.Y		=	0;
	engine.Z		=	0;
	engine.Scale	=	5000;
	engine.ShowDebug=	0;
	engine.BucketSize=	MAX_BUCKETS>>4;

	SetWorkWindowBounds	(
							bounds_rect->GetLeft(),
							bounds_rect->GetTop(),
							bounds_rect->GetWidth(),
							bounds_rect->GetHeight()
						);
	outline_rect.SetRect(
							0,0,
							bounds_rect->GetWidth(),bounds_rect->GetHeight()
						);
	set_camera();
//	set_camera_to_base();
	

//md	LogText(" width %d height %d \n",width,height);
	test_draw_all_get_sizes(test_chunk->MultiObject,the_frame,0,0,0,0,r_matrix,&width,&height,&mid_x,&mid_y);
	if(width>0 && height>0)
	{


		scale	=	(bounds_rect->GetWidth()<<16)/width;
		scale_y	=	(bounds_rect->GetHeight()<<16)/height;
		if(scale_y<scale)
			scale	=	scale_y;

//		scale			=	(scale*900)>>8;
		engine.Scale	=	(5000*scale)>>16;

		//calc new mids with this scale
		test_draw_all_get_sizes(test_chunk->MultiObject,the_frame,0,0,0,0,r_matrix,&width,&height,&mid_x,&mid_y);

		mid_x-=bounds_rect->GetWidth()>>1;
		mid_y-=bounds_rect->GetHeight()>>1;
		engine.VW2-=mid_x;
		engine.VH2-=mid_y;

	}
//	LogText(" drawkeyframe scale %d \n",engine.Scale);

	//
	// body part system
	//
	start_object=prim_multi_objects[test_chunk->MultiObject].StartObject;
	for(c2=0;c2<test_chunk->ElementCount;c2++)
	{
		SLONG	object_offset;
		if(the_frame->FirstElement==0)
			goto error;

		//
		// for each vue frame we need an object to draw
		//
		if(test_chunk->ElementCount==15)
			object_offset=test_chunk->PeopleTypes[person_id].BodyPart[c2];
		else
			object_offset=c2;


//		object_offset=test_chunk->PeopleTypes[person_id].BodyPart[c2];
		the_element			=	&the_frame->FirstElement[c2]; //vue part
   		test_draw(start_object+object_offset,0,0,0,0,the_element,the_element,r_matrix, NULL, NULL, NULL, NULL, NULL);
	}

/*	old system
	for(c2=0,c0=prim_multi_objects[test_chunk->MultiObject].StartObject;c0<=prim_multi_objects[test_chunk->MultiObject].EndObject;c0++)
	{
		if(the_frame->FirstElement==0)
			goto error;
		the_element			=	&the_frame->FirstElement[c2++];
   		test_draw(c0,0,0,0,0,the_element,the_element,r_matrix);
	}
*/
	render_view(0);
	outline_rect.OutlineRect(0);

	engine.X		=	temp_x;
	engine.Y		=	temp_y;
	engine.Z		=	temp_z;
	engine.Scale	=	temp_scale;
	engine.ShowDebug=	1;
	engine.BucketSize=	MAX_BUCKETS;
	engine.VW2	=	temp_vw2;
	engine.VH2	=	temp_vh2;
error:;
	MemFree(rotate_vectors);
	MemFree(flags);
}

void	drawkeyframeboxgamechunk(UWORD multi_object,EdRect *bounds_rect,struct GameKeyFrame *the_frame,struct Matrix33 *r_matrix,SLONG person_id,struct GameKeyFrameChunk *the_chunk)
{
	SLONG					c0,c1,c2,
							num_points,
							end_point,
							scale,
							scale_y,
							start_point,
							temp_scale,
							temp_x,
							temp_y,
							temp_z,
							temp_vw2,
							temp_vh2,
							width,height,
							*flags;
	EdRect					outline_rect;
	struct GameKeyFrameElement	*the_element;
	struct Matrix31			offset;
	struct PrimObject		*the_obj;
	struct SVector			*rotate_vectors,
							t_vector,
							t_vector2;

	SLONG	mid_x=0,mid_y=0;
	SLONG	start_object;

	// Stop the compiler moaning.
	multi_object	=	multi_object;

	if(!the_chunk->MultiObject)
		return;

	c1	=	0;
	flags			=	(SLONG*)MemAlloc(sizeof(SLONG)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");
	rotate_vectors	=	(struct SVector*)MemAlloc(sizeof(struct SVector)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");


	temp_scale		=	engine.Scale;
	temp_x			=	engine.X;
	temp_y			=	engine.Y;
	temp_z			=	engine.Z;
	temp_vw2		=	engine.VW2;
	temp_vh2		=	engine.VH2;

	engine.X		=	0;
	engine.Y		=	0;
	engine.Z		=	0;
	engine.Scale	=	5000;
	engine.ShowDebug=	0;
	engine.BucketSize=	MAX_BUCKETS>>4;
/*
	SetWorkWindowBounds	(
							bounds_rect->GetLeft(),
							bounds_rect->GetTop(),
							bounds_rect->GetWidth(),
							bounds_rect->GetHeight()
						);

	outline_rect.SetRect(
							0,0,
							bounds_rect->GetWidth(),bounds_rect->GetHeight()
						);
*/
	set_camera();
//	set_camera_to_base();
	

//md	LogText(" width %d height %d \n",width,height);
	test_draw_all_get_sizes_game(the_chunk->MultiObject[0],the_frame,0,0,0,0,r_matrix,&width,&height,&mid_x,&mid_y);
	if(width>0 && height>0)
	{


		scale	=	(bounds_rect->GetWidth()<<16)/width;
		scale_y	=	(bounds_rect->GetHeight()<<16)/height;
		if(scale_y<scale)
			scale	=	scale_y;

//		scale			=	(scale*900)>>8;
		engine.Scale	=	(5000*scale)>>16;

		//calc new mids with this scale
		test_draw_all_get_sizes_game(the_chunk->MultiObject[0],the_frame,0,0,0,0,r_matrix,&width,&height,&mid_x,&mid_y);

		mid_x-=bounds_rect->GetWidth()>>1;
		mid_y-=bounds_rect->GetHeight()>>1;
		engine.VW2-=mid_x;
		engine.VH2-=mid_y;

	}
//	LogText(" drawkeyframe scale %d \n",engine.Scale);

	//
	// body part system
	//
	start_object=prim_multi_objects[the_chunk->MultiObject[0]].StartObject;
	for(c2=0;c2<the_chunk->ElementCount;c2++)
	{
		SLONG	object_offset;
		if(the_frame->FirstElement==0)
			goto error;

		//
		// for each vue frame we need an object to draw
		//

		object_offset=c2;//the_chunk->PeopleTypes[person_id].BodyPart[c2];
		the_element			=	&the_frame->FirstElement[c2]; //vue part
   		test_draw_game(start_object+object_offset,0,0,0,0,the_element,the_element,r_matrix);
	}

	render_view(0);
	outline_rect.OutlineRect(0);

	engine.X		=	temp_x;
	engine.Y		=	temp_y;
	engine.Z		=	temp_z;
	engine.Scale	=	temp_scale;
	engine.ShowDebug=	1;
	engine.BucketSize=	MAX_BUCKETS;
	engine.VW2	=	temp_vw2;
	engine.VH2	=	temp_vh2;
error:;
	MemFree(rotate_vectors);
	MemFree(flags);
}

//---------------------------------------------------------------
//---------------------------------------------------------------

extern void	invert_mult(struct Matrix33 *mat,struct PrimPoint *pp);
/*
void	KeyFrameEditor::SortMultiObject(struct KeyFrameChunk *the_chunk)
{
	SLONG					c0,c1,
							sp,ep;
	struct KeyFrameElement	*the_element;
	struct PrimObject		*p_obj;


	p_obj		=	&prim_objects[prim_multi_objects[the_chunk->MultiObject].StartObject];
	the_element	=	the_chunk->KeyFrames[0].FirstElement;
	for(c0=0;c0<the_chunk->ElementCount;c0++,p_obj++,the_element++)
	{
		sp	=	p_obj->StartPoint;
		ep	=	p_obj->EndPoint;

		for(c1=sp;c1<ep;c1++)
		{
			prim_points[c1].X	-=	the_element->OffsetX;
			prim_points[c1].Y	-=	the_element->OffsetY;
			prim_points[c1].Z	-=	the_element->OffsetZ;
			invert_mult(&the_element->Matrix,&prim_points[c1]);
		}
	}
}
*/
//---------------------------------------------------------------

void	KeyFrameEditor::DrawKeyFrames(void)
{
	CBYTE				text[64];
	SLONG				c0,
						first_frame,
						max_frames;
	EdRect				frame_rect,
						hilite_rect;
	MFPoint				mouse_point;
	struct Matrix33		r_matrix;


	WindowControls.SetControlDrawArea();
	if(test_chunk->MultiObject)
	{
	 	KeyFrameRect.FillRect(ACTIVE_COL);
		rotate_obj(AnimAngleX[Bank] + AnimGlobalAngleX,AnimAngleY[Bank] + AnimGlobalAngleY,0,&r_matrix);
		max_frames	=	KeyFrameRect.GetWidth()/KEY_FRAME_IMAGE_SIZE;
		first_frame	=	((CHSlider*)WindowControls.GetControlPtr(CTRL_FRAME_SLIDER))->GetCurrentValue();
//		if((first_frame+max_frames)>KeyFrameCount)
//			max_frames	=	(KeyFrameCount-first_frame)+1;
		if((first_frame+max_frames)>test_chunk->KeyFrameCount)
			max_frames	=	(test_chunk->KeyFrameCount-first_frame)+1;
//md		LogText(" DRAW KEY FRAMES\n");
//		LogText(" draw keyframes  anglex %d angley %d \n",AnimAngleX[Bank],AnimAngleY[Bank]);
		for(c0=0;c0<max_frames;c0++)
		{
			WindowControls.SetControlDrawArea();
//md

			frame_rect.SetRect	(
									KeyFrameRect.GetLeft()+(c0*KEY_FRAME_IMAGE_SIZE)+2,
									KeyFrameRect.GetTop()+2,
									KEY_FRAME_IMAGE_SIZE,
									KEY_FRAME_IMAGE_SIZE
								);

//			frame_rect.SetRect	(0,0,400,400);
			hilite_rect	=	frame_rect;
			frame_rect.OffsetRect(ControlsLeft(),ControlsTop());
			mouse_point.X	=	MouseX;
			mouse_point.Y	=	MouseY;
			if(frame_rect.PointInRect(&mouse_point))
				hilite_rect.FillRect(HILITE_COL);
			
			DrawKeyFrame(test_chunk->MultiObject,&frame_rect,&test_chunk->KeyFrames[first_frame+c0],&r_matrix);

			sprintf(text,"%d",(first_frame+c0));
			QuickTextC(3,1,text,WHITE_COL);
//md			break;

		}
	}
	WindowControls.SetControlDrawArea();
 	KeyFrameRect.HiliteRect(LOLITE_COL,HILITE_COL);
}

//---------------------------------------------------------------

void	KeyFrameEditor::DrawAnimFrames(Anim *the_anim,BOOL hilite)
{
	CBYTE				text[16];
	SLONG				c0,
						first_frame,
						max_frames;
	EdRect				frame_rect,
						hilite_rect,
						tween_rect;
	MFPoint				mouse_point;
	struct KeyFrame		*current_frame;
	struct Matrix33		r_matrix;


	AnimControls.SetControlDrawArea();
	if(the_anim)
	{
	 	AnimFrameRect.FillRect(ACTIVE_COL);
//md		rotate_obj(AnimAngleX[Bank],AnimAngleY[Bank],0,&r_matrix);
		rotate_obj(0,AnimAngleY[Bank],0,&r_matrix);
		max_frames	=	AnimFrameRect.GetWidth()/KEY_FRAME_IMAGE_SIZE;
		first_frame	=	((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->GetCurrentValue();
		if((first_frame+max_frames)>the_anim->GetFrameCount())
			max_frames	=	(the_anim->GetFrameCount()-first_frame);
		current_frame	=	the_anim->GetFrameList();
		if(current_frame)
		{
			for(c0=0;c0<first_frame&&current_frame;c0++)
			{
				current_frame	=	current_frame->NextFrame;
			}
			SelectedFrame	=	NULL;
			for(c0=0;c0<max_frames&&current_frame;c0++)
			{
				AnimControls.SetControlDrawArea();
				frame_rect.SetRect	(
										AnimFrameRect.GetLeft()+(c0*KEY_FRAME_IMAGE_SIZE)+2,
										AnimFrameRect.GetTop()+2,
										KEY_FRAME_IMAGE_SIZE,
										KEY_FRAME_IMAGE_SIZE
									);
				hilite_rect	=	frame_rect;
				frame_rect.OffsetRect(ContentLeft(),ContentTop());
				mouse_point.X	=	MouseX;
				mouse_point.Y	=	MouseY;
				if(hilite && frame_rect.PointInRect(&mouse_point))
				{
					hilite_rect.FillRect(HILITE_COL);
					SelectedFrame	=	current_frame;
				}
				if(CurrentFrame[Bank]==current_frame)
				{
					hilite_rect.FillRect(RED_COL);
				}

//md
				DrawKeyFrame(test_chunk->MultiObject,&frame_rect,current_frame,&r_matrix);
				{
					CBYTE	str[100];
					sprintf(str,"%d",current_frame->FrameID);
					QuickTextC(2,2,str,WHITE_COL);
				}

				AnimControls.SetControlDrawArea();
				tween_rect.SetRect	(
										AnimFrameRect.GetLeft()+(c0*KEY_FRAME_IMAGE_SIZE)+2,
										(AnimFrameRect.GetBottom()-(QTStringHeight()+5))-1,
										22,
										QTStringHeight()+5
									);
				tween_rect.FillRect(0);
				sprintf(text,"%d",current_frame->TweenStep);
				QuickTextC	(
								tween_rect.GetLeft()+((tween_rect.GetWidth()-QTStringWidth(text))>>1),
								tween_rect.GetTop()+((tween_rect.GetHeight()-QTStringHeight())>>1),
								text,
								255
							);
				current_frame	=	current_frame->NextFrame;
			}
		}
	}
	AnimControls.SetControlDrawArea();
 	AnimFrameRect.HiliteRect(LOLITE_COL,HILITE_COL);
}

//---------------------------------------------------------------

Anim	*KeyFrameEditor::DrawAllAnimsBox(void)
{
	SLONG		c0,
				first_frame,
				text_x,
				text_y;
	Anim		*current_anim,
				*selected_anim;
	EdRect		name_rect;
	MFPoint		mouse_point;
	CBYTE		str[100];
	SLONG		skip_frames;


	selected_anim	=	0;
	AllAnimsRect.FillRect(ACTIVE_COL);
	if(AnimList[Bank])
	{
		first_frame	=	((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->GetCurrentValue()+1;
		skip_frames=first_frame;
		current_anim	=	AnimList[Bank];
		while(first_frame--)
		{
			if(current_anim->GetNextAnim())
				current_anim = current_anim->GetNextAnim();
			else
				break;
		}

		text_x	=	AllAnimsRect.GetLeft()+2;
		text_y	=	AllAnimsRect.GetTop()+1;
		mouse_point.X	=	MouseX;
		mouse_point.Y	=	MouseY;
		GlobalToLocal(&mouse_point);
		for(c0=0;c0<22&&current_anim;c0++)
		{
			name_rect.SetRect(text_x-1,text_y,AllAnimsRect.GetWidth()-2,QTStringHeight()+1);
			if(current_anim==CurrentAnim[Bank])
			{
				name_rect.FillRect(LOLITE_COL);
			}
			if(name_rect.PointInRect(&mouse_point))
			{
				name_rect.FillRect(HILITE_COL);
				selected_anim	=	current_anim;
			}
			sprintf(str,"(%d) %s",c0+skip_frames, current_anim->GetAnimName());

			if(unused_flags[skip_frames+c0])
				QuickText(text_x,text_y,str,YELLOW_COL);
			else
				QuickText(text_x,text_y,str,0);
			text_y			+=	QTStringHeight()+1;
			current_anim	=	current_anim->GetNextAnim();
		}
	}
	AllAnimsRect.HiliteRect(LOLITE_COL,HILITE_COL);
	return	selected_anim;
}

//---------------------------------------------------------------
void	KeyFrameEditor::ClearAll(void)
{

	while(AnimList[Bank])
		DestroyAnim(AnimList[Bank]);

	test_chunk->MultiObject=0;
	test_chunk->ElementCount=0;

}

void	KeyFrameEditor::AppendAnim(void)
{
	CBYTE		text[32];
	SLONG		max_range;
	Anim		*next_anim,
				*the_anim;


	the_anim	=	new Anim;
	if(the_anim)
	{
		AnimCount[Bank]++;
		sprintf(text,"New Anim %d",AnimCount[Bank]);
		the_anim->SetAnimName(text);
		if(AnimList[Bank])
		{
			next_anim	=	AnimList[Bank];
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
			AnimList[Bank]	=	the_anim;
		}
		CurrentAnim[Bank]	=	the_anim;
		AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_DESELECTED);
		max_range	=	AnimCount[Bank]-22;
		if(max_range<0)
			max_range	=	0;
		((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->SetValueRange(0,max_range);
		((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->SetCurrentValue(max_range);
	}
}
void	KeyFrameEditor::InsertAnim(Anim *insert_here)
{
	CBYTE		text[32];
	SLONG		max_range;
	Anim		*next_anim,
				*prev_anim,
				*the_anim;


	the_anim	=	new Anim;
	if(the_anim)
	{
		AnimCount[Bank]++;
		sprintf(text,"New Anim %d",AnimCount[Bank]);
		the_anim->SetAnimName(text);
		if(insert_here)
		{
			prev_anim=insert_here->GetLastAnim();
			if(prev_anim)
			{
				prev_anim->SetNextAnim(the_anim);
				insert_here->SetLastAnim(the_anim);
				the_anim->SetLastAnim(prev_anim);
				the_anim->SetNextAnim(insert_here);
			}
			else
			{
				the_anim->SetNextAnim(AnimList[Bank]);
				AnimList[Bank]->SetLastAnim(the_anim);
				AnimList[Bank]=the_anim;
				
			}
		}
		CurrentAnim[Bank]	=	the_anim;
		AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_DESELECTED);
		max_range	=	AnimCount[Bank]-22;
		if(max_range<0)
			max_range	=	0;
		((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->SetValueRange(0,max_range);
		((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->SetCurrentValue(max_range);
	}
}

//---------------------------------------------------------------

void	KeyFrameEditor::DestroyAnim(Anim *the_anim)
{
	SLONG		max_range;


	if(the_anim)
	{
		if(the_anim->GetLastAnim())
			the_anim->GetLastAnim()->SetNextAnim(the_anim->GetNextAnim());
		else
			AnimList[Bank]	=	the_anim->GetNextAnim();

		if(the_anim->GetNextAnim())
			the_anim->GetNextAnim()->SetLastAnim(the_anim->GetLastAnim());

		delete	the_anim;

		AnimCount[Bank]--;
		max_range	=	AnimCount[Bank]-22;
		if(max_range<0)
			max_range	=	0;
		((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->SetValueRange(0,max_range);
		((CVSlider*)AnimControls.GetControlPtr(CTRL_ANIM_ALL_ANIMS_SLIDER))->SetCurrentValue(max_range);

		CurrentAnim[Bank]	=	0;
		AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_DESELECTED);
	}
}

//---------------------------------------------------------------

void	KeyFrameEditor::LoadAllAnims(KeyFrameChunk *the_chunk)
{
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

			LoadBodyPartInfo(file_handle,version,the_chunk);
			
		}

		for(c0=0;c0<anim_count;c0++)
		{
			AppendAnim();
			LoadAnim(file_handle,CurrentAnim[Bank]);
		}
		CurrentAnim[Bank]	=	AnimList[Bank];
		if(CurrentAnim[Bank]->GetAnimFlags()&ANIM_LOOP)
			AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_SELECTED);
		else
			AnimControls.SetControlState(CTRL_ANIM_LOOP_SELECT,CTRL_DESELECTED);
		((CHSlider*)AnimControls.GetControlPtr(CTRL_ANIM_FRAME_SLIDER))->SetValueRange(0,CurrentAnim[Bank]->GetFrameCount()-1);
		AnimTween[Bank]		=	0;
		CurrentFrame[Bank]	=	0;
		FileClose(file_handle);
	}
	else
	{
		// Unable to open file.
	}
}

//---------------------------------------------------------------
void	KeyFrameEditor::SaveBodyPartInfo(MFFileHandle file_handle,SLONG version,KeyFrameChunk *the_chunk)
{
	SLONG	c0,c1;
	SLONG	data;

	data=MAX_PEOPLE_TYPES;
	FileWrite(file_handle,&data,sizeof(data));

	data=MAX_BODY_BITS;
	FileWrite(file_handle,&data,sizeof(data));

	data=PEOPLE_NAME_SIZE;
	FileWrite(file_handle,&data,sizeof(data));

	for(c0=0;c0<MAX_PEOPLE_TYPES;c0++)
	{
		FileWrite(file_handle,the_chunk->PeopleNames[c0],PEOPLE_NAME_SIZE);
		for(c1=0;c1<MAX_BODY_BITS;c1++)
			FileWrite(file_handle,&the_chunk->PeopleTypes[c0].BodyPart[c1],sizeof(UBYTE));
	}
}

void	KeyFrameEditor::LoadBodyPartInfo(MFFileHandle file_handle,SLONG version,KeyFrameChunk *the_chunk)
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

extern	SLONG	save_a_multi_prim(CBYTE	*name,SLONG multi);
void	KeyFrameEditor::SaveAllAnims(KeyFrameChunk *the_chunk,SLONG save_all)
{
	Anim			*next_anim;
	MFFileHandle	file_handle;
	SLONG	version=-1;


	if(AnimList[Bank])
	{
		save_a_multi_prim(the_chunk->ANMName,the_chunk->MultiObject);

		file_handle	=	FileCreate(the_chunk->ANMName,TRUE);
		if(file_handle!=FILE_CREATION_ERROR)
		{
			FileWrite(file_handle,&version,sizeof(version));
			FileWrite(file_handle,&AnimCount[Bank],sizeof(AnimCount[Bank]));
			SaveBodyPartInfo(file_handle,version,the_chunk);
			next_anim	=	AnimList[Bank];
			while(next_anim)
			{
				SaveAnim(file_handle,next_anim);
				next_anim	=	next_anim->GetNextAnim();
			}
			FileClose(file_handle);
		}
		else
		{
			SLONG	res;
			res=GetLastError();
			DebugText(" failed to create %s error %d \n",the_chunk->ANMName,res);
		}
	}
	SaveChunkTextureInfo(the_chunk);
	save_recenter_flags(the_chunk->ANMName);


	if(save_all)
	{
		void convert_anim(Anim *key_list,GameKeyFrameChunk *game_chunk,KeyFrameChunk *the_chunk);
		extern	void	free_game_chunk(GameKeyFrameChunk *the_chunk);
		free_game_chunk(&game_chunk[0]);

		convert_anim(AnimList[Bank],&game_chunk[0],the_chunk);

		{
			CBYTE	file_name[100];
			SLONG	c0=0;

			strcpy(file_name,the_chunk->VUEName);
			while(file_name[c0]!='.' && file_name[c0]!=0)c0++;
			if(file_name[c0]=='.')
			{
				file_name[c0+1]	=	'A';
				file_name[c0+2]	=	'L';
				file_name[c0+3]	=	'L';
				file_name[c0+4]	=	0;
			}

	extern	SLONG	save_anim_system(struct GameKeyFrameChunk *game_chunk,CBYTE	*name);

			save_anim_system(&game_chunk[0],file_name);
			free_game_chunk(&game_chunk[0]);
		}
	}
}

//---------------------------------------------------------------

void	KeyFrameEditor::LoadAnim(MFFileHandle file_handle,Anim *the_anim)
{
	CBYTE			anim_name[ANIM_NAME_SIZE];
	SLONG			anim_flags,
					c0,
					frame_count,
					frame_id,
					tween_step;
	KeyFrame		*the_frame,*new_frame;
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
	if(version>3)
	{
		UBYTE	speed;
		FileRead(file_handle,&speed,1);
		the_anim->SetTweakSpeed(speed);
	}
	else
		the_anim->SetTweakSpeed(128);

	for(c0=0;c0<frame_count;c0++)
	{
		FileRead(file_handle,&chunk_id,sizeof(chunk_id));
		FileRead(file_handle,&frame_id,sizeof(frame_id));
		FileRead(file_handle,&tween_step,sizeof(tween_step));
		if(version>0)
			FileRead(file_handle,&fixed,sizeof(fixed));

		the_frame				=	&test_chunk->KeyFrames[frame_id];
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
					fcol->Next=0;
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

void	KeyFrameEditor::SaveAnim(MFFileHandle file_handle,Anim *the_anim)
{
	ULONG			anim_flags;
	SLONG			c0,
					frame_count;
	KeyFrame		*frame_list;
	CBYTE			version=4;
	UBYTE			speed;


	frame_list	=	the_anim->GetFrameList();
	anim_flags	=	the_anim->GetAnimFlags();
	frame_count	=	the_anim->GetFrameCount();

	FileWrite(file_handle,&version,1);
	FileWrite(file_handle,the_anim->GetAnimName(),ANIM_NAME_SIZE);
	FileWrite(file_handle,&anim_flags,sizeof(anim_flags));
	FileWrite(file_handle,&frame_count,sizeof(frame_count));
	speed=the_anim->GetTweakSpeed();
	FileWrite(file_handle,&speed,1);
	for(c0=0;c0<frame_count;c0++)
	{
		SLONG	count=0;
		struct	FightCol	*fcol;
		FileWrite(file_handle,&frame_list->ChunkID,sizeof(frame_list->ChunkID));
		FileWrite(file_handle,&frame_list->FrameID,sizeof(frame_list->FrameID));
		FileWrite(file_handle,&frame_list->TweenStep,sizeof(frame_list->TweenStep));
		FileWrite(file_handle,&frame_list->Fixed,sizeof(frame_list->Fixed));
		if(version>1)
		{
			fcol=frame_list->Fight;
			count=0;
			while(fcol)
			{
				count++;
				fcol=fcol->Next;
			}
			LogText(" fight count save = %d \n",count);
			FileWrite(file_handle,&count,sizeof(count));
			fcol=frame_list->Fight;
			while(fcol)
			{
				FileWrite(file_handle,fcol,sizeof(struct FightCol));
				fcol=fcol->Next;
			}
		}

		frame_list	=	frame_list->NextFrame;
	}
}
//---------------------------------------------------------------

void	KeyFrameEditor::LoadKeyFrameChunks(void)
{
	SLONG		c0;
/*

	test_chunk->KeyFrameCount	=	0;
	strcpy(test_chunk->VUEName,"data\\");
	strcat(test_chunk->VUEName,"man.vue");
	strcpy(test_chunk->ASCName,test_chunk->VUEName);
	strcpy(test_chunk->ANMName,test_chunk->VUEName);
	c0=0;
	while(test_chunk->ASCName[c0]!='.' && test_chunk->ASCName[c0]!=0)c0++;
	if(test_chunk->ASCName[c0]=='.')
	{
		test_chunk->ASCName[c0+1]	=	'A';
		test_chunk->ASCName[c0+2]	=	'S';
		test_chunk->ASCName[c0+3]	=	'C';
		test_chunk->ASCName[c0+4]	=	0;

		test_chunk->ANMName[c0+1]	=	'A';
		test_chunk->ANMName[c0+2]	=	'N';
		test_chunk->ANMName[c0+3]	=	'M';
		test_chunk->ANMName[c0+4]	=	0;
	}
	if(read_multi_asc(test_chunk->ASCName,0))
	{
		test_chunk->MultiObject	=	next_prim_multi_object-1;
		test_chunk->ElementCount	=	prim_multi_objects[test_chunk->MultiObject].EndObject-prim_multi_objects[test_chunk->MultiObject].StartObject;
		load_multi_vue(&test_chunk);
		LoadChunkTextureInfo(&test_chunk);
	}

  */
}

//---------------------------------------------------------------

void	KeyFrameEditor::SaveChunkTextureInfo(KeyFrameChunk *the_chunk)
{
	CBYTE				file_name[64];
	SLONG				c0	=	0,
						c1,c2;
	MFFileHandle		file_handle;
	struct PrimFace4	*p_f4;
	struct PrimFace3	*p_f3;
	struct PrimObject	*p_obj;
	SLONG	save_type=1;
	SLONG	count=0;
	SLONG	multi;


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

		file_handle	=	FileCreate(file_name,TRUE);
		if(file_handle!=FILE_CREATION_ERROR)
		{
			//SLONG	multi=the_chunk->MultiObjectStart;
			FileWrite(file_handle,&save_type,sizeof(save_type));

	//		for(multi=the_chunk->MultiObjectStart;multi<=the_chunk->MultiObjectEnd;multi++)
			for(c0=prim_multi_objects[multi].StartObject;c0<prim_multi_objects[multi].EndObject;c0++,c1++)
			{
				UWORD	count;

				p_obj		=	&prim_objects[c0];
				p_f4		=	&prim_faces4[p_obj->StartFace4];
				p_f3		=	&prim_faces3[p_obj->StartFace3];
				
				count=p_obj->EndFace4-p_obj->StartFace4;
				FileWrite(file_handle,&count,sizeof(count));
				for(c1=p_obj->StartFace4;c1<p_obj->EndFace4;c1++,p_f4++)
				{
					FileWrite(file_handle,&p_f4->DrawFlags,sizeof(p_f4->DrawFlags));
					FileWrite(file_handle,&p_f4->Col2,sizeof(p_f4->Col2));
					FileWrite(file_handle,&p_f4->TexturePage,sizeof(p_f4->TexturePage));
					FileWrite(file_handle,&p_f4->FaceFlags,sizeof(p_f4->FaceFlags));

					for(c2=0;c2<4;c2++)
					{
						FileWrite(file_handle,&p_f4->UV[c2][0],sizeof(p_f4->UV[c2][0]));
						FileWrite(file_handle,&p_f4->UV[c2][1],sizeof(p_f4->UV[c2][1]));
					}
				}

				count=p_obj->EndFace3-p_obj->StartFace3;
				FileWrite(file_handle,&count,sizeof(count));
				for(c1=p_obj->StartFace3;c1<p_obj->EndFace3;c1++,p_f3++)
				{
					FileWrite(file_handle,&p_f3->DrawFlags,sizeof(p_f3->DrawFlags));
					FileWrite(file_handle,&p_f3->Col2,sizeof(p_f3->Col2));
					FileWrite(file_handle,&p_f3->TexturePage,sizeof(p_f3->TexturePage));
					FileWrite(file_handle,&p_f3->FaceFlags,sizeof(p_f3->FaceFlags));

					for(c2=0;c2<3;c2++)
					{
						FileWrite(file_handle,&p_f3->UV[c2][0],sizeof(p_f3->UV[c2][0]));
						FileWrite(file_handle,&p_f3->UV[c2][1],sizeof(p_f3->UV[c2][1]));
					}
				}
			}
			FileClose(file_handle);
		}
		count++;
	}
}

//---------------------------------------------------------------


void	KeyFrameEditor::LoadChunkTextureInfo(KeyFrameChunk *the_chunk)
{

	//
	// this is in edutils.cpp
	//
	load_chunk_texture_info(the_chunk);
/*
	CBYTE				file_name[64];
	SLONG				c0	=	0,
						c1,c2;
	MFFileHandle		file_handle;
	struct PrimFace4	*p_f4;
	struct PrimFace3	*p_f3;
	struct PrimObject	*p_obj;
	SLONG	save_type;


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
	//	FileRead(file_handle,&save_type,sizeof(save_type));
		//if(save_type==0)
		{
			for(c0=prim_multi_objects[the_chunk->MultiObject].StartObject;c0<=prim_multi_objects[the_chunk->MultiObject].EndObject;c0++,c1++)
			{
				SLONG	count;
				p_obj		=	&prim_objects[c0];
				p_f4		=	&prim_faces4[p_obj->StartFace4];
				p_f3		=	&prim_faces3[p_obj->StartFace3];

				
				//FileRead(file_handle,&count,sizeof(count));
				for(c1=p_obj->StartFace4;c1<p_obj->EndFace4;c1++,p_f4++,count--)
				{
					FileRead(file_handle,&p_f4->DrawFlags,sizeof(p_f4->DrawFlags));
					FileRead(file_handle,&p_f4->Col2,sizeof(p_f4->Col2));
					FileRead(file_handle,&p_f4->TexturePage,sizeof(p_f4->TexturePage));

					for(c2=0;c2<4;c2++)
					{
						FileRead(file_handle,&p_f4->UV[c2][0],sizeof(p_f4->UV[c2][0]));
						FileRead(file_handle,&p_f4->UV[c2][1],sizeof(p_f4->UV[c2][1]));
					}
				}

				//FileRead(file_handle,&count,sizeof(count));
				for(c1=p_obj->StartFace3;c1<p_obj->EndFace3;c1++,p_f3++,count--)
				{
					FileRead(file_handle,&p_f3->DrawFlags,sizeof(p_f3->DrawFlags));
					FileRead(file_handle,&p_f3->Col2,sizeof(p_f3->Col2));
					FileRead(file_handle,&p_f3->TexturePage,sizeof(p_f3->TexturePage));

					for(c2=0;c2<3;c2++)
					{
						FileRead(file_handle,&p_f3->UV[c2][0],sizeof(p_f3->UV[c2][0]));
						FileRead(file_handle,&p_f3->UV[c2][1],sizeof(p_f3->UV[c2][1]));
					}
				}
			}
		}
		FileClose(file_handle);
	}
*/	
}

//---------------------------------------------------------------

void	draw_key_frames(void)
{
	//MD the_editor->DrawKeyFrames();
}

//---------------------------------------------------------------

void	draw_anim_frames(void)
{ 
	if(the_editor->GetCurrentAnim())
		the_editor->DrawAnimFrames(the_editor->GetCurrentAnim(),FALSE);
}

//---------------------------------------------------------------

void	draw_all_anims_box(void)
{
	the_editor->DrawAllAnimsBox();
}

//---------------------------------------------------------------

void	set_key_framer_camera()
{
	SLONG	angle;
	set_camera();
	return;

	engine.VW=WorkWindowWidth;
	engine.VH=WorkWindowHeight;

	engine.VW2=engine.VW>>1;
	engine.VH2=engine.VH>>1;


	engine.CosY=COS(0);
	engine.SinY=SIN(0);

	engine.CosX=COS(0);
	engine.SinX=SIN(0);

	engine.CosZ=COS(0);
	engine.SinZ=SIN(0);
}

void	set_key_framer_camera_plan()
{
	SLONG	angle;

	engine.VW=WorkWindowWidth;
	engine.VH=WorkWindowHeight;

	engine.VW2=engine.VW>>1;
	engine.VH2=engine.VH>>1;


	engine.CosY=COS(0);
	engine.SinY=SIN(0);

	engine.CosX=COS(512);
	engine.SinX=SIN(512);

	engine.CosZ=COS(0);
	engine.SinZ=SIN(0);
}

//---------------------------------------------------------------
