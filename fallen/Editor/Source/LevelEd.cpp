// Guy Simmons, 19th Feb ruary 1997.

#include	"Editor.hpp"
#include	"engine.h"
#include	"c:\fallen\headers\game.h"
#include	"c:\fallen\headers\animtmap.h"
#include	"c:\fallen\headers\inside2.h"
#include	"c:\fallen\headers\memory.h"
#include	"c:\fallen\headers\io.h"
//#include	"collide.hpp"  //needed for ele_shift
//#define		ShowWorkWindow(x)	{DrawLineC(0,0,WorkWindowWidth-1,WorkWindowHeight-1,255);DrawLineC(0,WorkWindowHeight-1,WorkWindowWidth-1,0,255); ShowWorkWindow(x);}

extern	SLONG	editor_texture_set;


//---------------------------------------------------------------
extern	void	icon_load_map(UWORD id);
extern	void	icon_save_map(UWORD id);
extern	void	handle_icon_click(UWORD id);
extern	void	slider_redraw(void);
extern	void	slider_redraw_in(void);

#define	ICON_LOAD_MAP		0
#define	ICON_SAVE_MAP		1
#define	ICON_TEST_LORES		2
#define	ICON_TOGGLE_GRID	4
#define	ICON_PLACE_LIGHT	6
#define	ICON_CREATE_CITY	8


#define	CTRL_PSX_SAVE		1
#define	CTRL_PSX_PAGE1		1
#define	CTRL_PSX_PAGE2		2
#define	CTRL_PSX_PAGE3		3
#define	CTRL_PSX_PAGE4		4
#define	CTRL_PSX_NOREMAP	5

#define	CTRL_STYLE_POS_SLIDER	1
#define	CTRL_STYLE_NAME_EDIT	2
#define	CTRL_STYLE_SAVE			3

#define	CTRL_INSTYLE_POS_SLIDER	1
#define	CTRL_INSTYLE_NAME_EDIT	2
#define	CTRL_INSTYLE_SAVE		3
ControlDef	psx_content_def[]	=
{
//	{	BUTTON,			0,	"Save Remap",			20,	480,	70,		0			},
	{	BUTTON,			0,	"P1",			20,	460,	20,		0			},
	{	BUTTON,			0,	"P2",			50,	460,	20,		0			},
	{	BUTTON,			0,	"P3",			80,	460,	20,		0			},
	{	BUTTON,			0,	"P4",			110,	460,	20,		0			},
	{	BUTTON,			0,	"No Remap",			200,	480,	70,		0			},

	{	0																	}
};

ControlDef	style_content_def[]	=
{
	{	V_SLIDER,		0,	"",						1,30,		0,		400			},
	{	EDIT_TEXT,		0,	"",						20,	460,	120,		0			},
	{	BUTTON,			0,	"Save Styles",			20,	480,	70,		0			},

	{	0																	}
};

ControlDef	inside_style_content_def[]	=
{
	{	V_SLIDER,		0,	"",						1,30,		0,		400			},
	{	EDIT_TEXT,		0,	"",						20,	460,	120,		0			},
	{	BUTTON,			0,	"Save Styles",			20,	480,	70,		0			},

	{	0																	}
};

CBYTE	inside_names[64][20]=
{
	"Apartment",
	"Hotel",
	"WareHouse",
	"Office",
	"Museum",
	"Industrial1",
	"Industrial2",
	"Police",
	"Hospital"
};





struct	AWindowIcon	win_bar_icons[]=
{
	{handle_icon_click,0,18,18},  //Load 
	{handle_icon_click,0,19,19},  //Save
	{handle_icon_click,1,20,20},  //Test Lores
	{0,0,0,-1},
	{handle_icon_click,1,16,17},  //Grid Lines
	{0,0,0,-1},
	{handle_icon_click,1,22,22},  //Place Light
	{0,0,0,-1},
	{handle_icon_click,1,24,24},  //Create City
	{0,0,0,0}
};

ControlDef	prim_content_def[]	=
{
	{	H_SLIDER,		0,	"",		   				2,		200,		300,0						},

	{	0	}
};
//from edutil.cpp
extern void	draw_a_key_frame_at(UWORD prim,SLONG x,SLONG y,SLONG z);
extern			void	reset_game(void);

UBYTE	back_dat[256*256];

SLONG	draw_background(void)
{				 
	SLONG	x,y,mod_x;
	SLONG	c0,c1,width,size,height;
	UBYTE *ptr,*ptr_dat;
	static	UBYTE	type=1;

	if(Keys[KB_1])
	{
		FileLoadAt("data\\back.dat",back_dat);
		type=1;
	}
	if(Keys[KB_2])
	{
		FileLoadAt("data\\back.dat",back_dat);
		type=2;
	}
	if(Keys[KB_3])
		type=3;
	if(Keys[KB_4])
		type=4;

	if(type==4)
	{
		return(type);
	}



	x=(engine.X>>8)+(engine.AngleY>>5);
	x>>=3;
	x&=0xff;

	y=((engine.AngleX>>8));
	if(y>1024)
		y=y-2048;
//	y=y-240;


	if(y<0)
	{
		if(-y>WorkScreenHeight)
		{
			memset(WorkWindow,0,WorkScreenHeight*WorkScreenWidth);
			return(type);
		}

		memset(WorkWindow,0,-y*WorkScreenWidth);
		ptr=(UBYTE*)WorkWindow-y*WorkScreenWidth;
		ptr_dat=(UBYTE*)back_dat;
		height=WorkScreenHeight+y;
	}
	else
	{
		ptr=(UBYTE*)WorkWindow;
		ptr_dat=(UBYTE*)back_dat+y*WorkScreenWidth;
		if((256-y)<WorkScreenHeight)
		{
			height=(256-y);
			if(height<0)
				height=0;
			memset(WorkWindow+height*WorkScreenWidth,0,(WorkScreenHeight-height)*WorkScreenWidth);

		}
		else
		{
			height=WorkScreenHeight;
		}
	}

	for(c1=0;c1<height;c1++)
	{
		mod_x=x;
		width=WorkScreenWidth;

		while(width)
		{
			size=256-mod_x;
			if(size > width)
				size = width;

			memcpy(ptr,&ptr_dat[mod_x],size);
			width-=size;
			mod_x+=size;
			mod_x&=0xff;
			ptr+=size;
		}
		ptr_dat+=256;
	}
	return(type);
}	

extern	void	init_hair(SLONG x,SLONG y,SLONG z);
extern	void	draw_hair(void);

void	do_clip_keys(void)
{
//	if(engine.ClipFlag)
//		engine.ClipFlag--;
	if(ShiftFlag)
		engine.ClipFlag=ENGINE_CLIPZ_FLAG;
	else
		engine.ClipFlag&=~ENGINE_CLIPZ_FLAG;

	if(Keys[KB_LBRACE])
	{
		engine.ClipZ-=25;
		engine.ClipFlag=ENGINE_CLIPZ_FLAG;
//		engine.ClipFlag=300;
	}
	if(Keys[KB_RBRACE])
	{
		engine.ClipZ+=25;
		engine.ClipFlag=ENGINE_CLIPZ_FLAG;
//		engine.ClipFlag=300;
	}

	
}
extern		void	mini_game_test(void);
extern	void	interface_thing2(struct MapThing *p_thing);

//extern	void	draw_col_vects(UWORD	col_vect_link);
//extern	void	draw_actual_col_vect(UWORD	col_vect);
/*
void	draw_map_col_vects(void)
{
	SLONG	c0;
	for(c0=1;c0<next_col_vect;c0++)
		draw_actual_col_vect(c0);
	render_view(0);

}
*/
void	screen_shot(void)
{
	UBYTE header[30];
	UWORD *p_w,*p_w2;
	UWORD	x,y;
	UWORD	line[2048];
	UWORD	red,green,blue,col;
	static	count=0;
	CBYTE	fname[50];
	count++;


	MFFileHandle	handle	=	FILE_OPEN_ERROR;

	LogText(" wsw wsh %d %d \n",WorkScreenWidth,WorkScreenHeight);

	sprintf(fname,"shots/pic%03d.tga",count);
	handle=FileCreate(fname,1);
	if(handle!=FILE_OPEN_ERROR)
	{

		header[0]=0;
		header[1]=0; //no colour map
		header[2]=2;
		p_w=(UWORD*)&header[8];
		*p_w++=0;
		*p_w++=0;
		*p_w++=WorkScreenPixelWidth;
		*p_w++=WorkScreenHeight;
		header[16]=16;
		header[17]=0x21;  //flags
		header[18]=0x01;
		header[19]=0;
	
		FileWrite(handle,&header[0],(ULONG)20);

		p_w=(UWORD*)&header[20];
		p_w2=(UWORD*)WorkScreen;

		for(y=0;y<WorkScreenHeight;y++)
		{
			for(x=0;x<WorkScreenPixelWidth;x++)
			{
				col=*p_w2++;

				blue=col&0x1f;
				green=(col>>5)&0x1f;
				red=(col>>10)&0x1f;
				col=(red<<10)+(green<<5)+blue;

				line[x]=col;
			}
			FileWrite(handle,&line[0],(ULONG)WorkScreenPixelWidth*2);
		}
		FileClose(handle);
	}
}

LevelEditor		*the_leveleditor;
void	handle_icon_click(UWORD id)
{
	switch(id)
	{
		case ICON_LOAD_MAP:
			{
				
				FileRequester	*fr;
				CBYTE	fname[100];
				fr=new FileRequester("data\\","*.map","Load A MAP","temp.map");
				if(fr->Draw())
				{
					strcpy(fname,fr->Path);
					strcat(fname,fr->FileName);
					strcpy(edit_info.MapName,fr->FileName);
					load_map(fname);
					the_leveleditor->BuildMode->ResetBuildTab();

				}

				delete fr;
			}
			break;
		case ICON_SAVE_MAP:
			{
				FileRequester	*fr;
				CBYTE	fname[100];
				CBYTE	temp_name[100];

				strcpy(temp_name,edit_info.MapName);
				fr=new FileRequester("data\\","*.map","Save A MAP",temp_name);
				if(fr->Draw())
				{
					strcpy(fname,fr->Path);
					strcat(fname,fr->FileName);
					strcpy(edit_info.MapName,fr->FileName);
					save_map(fname,0);
				}
				
				delete fr;
			//	RequestUpdate();
			}
			break;
		case ICON_TOGGLE_GRID:
			edit_info.GridOn=win_bar_icons[ICON_TOGGLE_GRID].Flag;
//			SetDisplay(800,600,8);
			break;
		case ICON_TEST_LORES:
			{
#ifdef	DOGPOO
#ifdef	TEST_3DFX
extern void	Demo3Dfx(void);
				Demo3Dfx();
#else
				UBYTE	display=0;
				SLONG	tx,ty,tz;
				SLONG	back_type=0;
				UBYTE	test_bloke=0;
				UBYTE	film=0;
				struct	MapThing	*darci;

extern	struct MapThing *init_test_bloke_system(void);
extern	void	draw_test_bloke(SLONG x,SLONG y,SLONG z,UBYTE anim,SLONG angle);
//				if(!Keys[KB_B])
				{
					darci=init_test_bloke_system();
					test_bloke=1;
				}

				engine.Y=500<<8;
				tx=engine.X>>8;
				ty=engine.Y>>8;
				tz=engine.Z>>8;
				save_map("data/bak.map",1);
				rotate_point_gte=rotate_point_gte_perspective;
				switch(WorkScreenDepth)
				{
					case 1:
						display=SetDisplay(320,200,8);
						break;
					case 2:
						display=SetDisplay(320,200,16);
						break;
					case 4:
						display=SetDisplay(320,200,32);
						break;
				}
				LogText(" display = %d \n",display);
				FileLoadAt("data\\back.dat",back_dat);
				SetWorkWindowBounds(0,0,320,199);

				
extern	SLONG	calc_height_at(SLONG x,SLONG z);

				darci->X=engine.X>>8;
				darci->Y=calc_height_at(darci->X,darci->Z);
				darci->Z=engine.Z>>8;

				engine.Scale=296;	 
				if(Keys[KB_SPACE])
				{
//					mini_game_test();
				}
				else
				{
					

//FLI				anim_record();
					init_hair(MouseX,MouseY,0);
					if(display==NoError)
					while(SHELL_ACTIVE && !RightButton)
					{
						SLONG	depth;
						if(ShiftFlag)
							depth=16;
						else
						if(ControlFlag)
							depth=32;
						else
							depth=8;
						if(Keys[KB_6])
						{
							display=SetDisplay(320,200,depth);
							Keys[KB_6]=0;
						}
						if(Keys[KB_7])
						{
							display=SetDisplay(640,480,depth);
							Keys[KB_7]=0;
						}

						if(Keys[KB_8])
						{
							display=SetDisplay(800,600,depth);
							Keys[KB_8]=0;
						}

						if(Keys[KB_9])
						{
							display=SetDisplay(1024,768,depth);
							Keys[KB_9]=0;
						}

						if(back_type==4)
							ClearWorkScreen(0);
						if(LockWorkScreen())
						{

	//						DrawBoxC(0,0,640,400,0);
	//						editor_user_interface();
							do_clip_keys();
							set_camera();
							if(test_bloke)
							{
								SLONG	y;

extern	SLONG	calc_height_at(SLONG x,SLONG z);

//								y=calc_height_at(engine.X>>8,engine.Z>>8);
extern	SLONG	play_x,play_y,play_z;
								draw_test_bloke(darci->X,darci->Y,darci->Z,1,0);
//								draw_test_bloke(engine.X>>8,y,engine.Z>>8,1,0);
								if(ShiftFlag)
								{
									draw_test_bloke(darci->X-100,0,darci->Z,0,512);
									draw_test_bloke(darci->X+100,0,darci->Z,0,1024);
									draw_test_bloke(darci->X+300,0,darci->Z,0,1530);
								}
							}
							draw_quick_map();
//							draw_editor_map(1);
	//						test_poly();
							interface_thing2(darci);
//							back_type=draw_background();
							back_type=4;
							render_view(0);
//extern	void	draw_fader(void);
//	draw_fader();

							{
								CBYTE	str[100];
								sprintf(str,"face %d id %d",darci->OnFace,prim_faces4[darci->OnFace].ID);
								QuickTextC(1,179,str,0);
								QuickTextC(0,180,str,WHITE_COL);
							}
//COL VECTS							draw_map_col_vects();
//							draw_hair();
	//FLI						anim_make_next_frame((UBYTE*)WorkScreen, PALETTE);

							if(LastKey==KB_C && ControlFlag)
							{
							   	do_single_shot(WorkScreen,CurrentPalette);
								LastKey	=	0;
							}

							UnlockWorkScreen();
							if(LastKey==KB_R && ControlFlag)
							{
								editor_status	^=	EDITOR_RECORD;
								LastKey	=	0;
							}
						}

						if(LastKey==KB_B)
						{
							film=1;
						}
						if(LastKey==KB_N)
						{
							film=0;
						}

							if(film)
								screen_shot();
						ShowWorkScreen(0);
					}
	//FLI				anim_stop();
				}
/*
				switch(WorkScreenDepth)
				{
					case 1:
						display=SetDisplay(800,600,8);
						break;
					case 2:
						display=SetDisplay(800,600,16);
						break;
					case 4:
						display=SetDisplay(800,600,32);
						break;
				}
*/
				display=SetDisplay(800,600,16);
				rotate_point_gte=rotate_point_gte_normal;
				reset_game();
				create_city(BUILD_MODE_EDITOR);
//				RequestUpdate();
#endif
#endif
			}
			break;
		case ICON_PLACE_LIGHT:
			the_leveleditor->BringTabIDToFront(TAB_LIGHT);
			the_leveleditor->LightMode->Mode=LIGHT_TAB_MODE_PLACE_LIGHT;
			break;
		case ICON_CREATE_CITY:
			create_city(BUILD_MODE_EDITOR);
extern	UWORD	count_empty_map_things(void);
				LogText(" npp %d npf %d npf4 %d npo %d UNUSED th %d \n",next_prim_point,next_prim_face3,next_prim_face4,next_prim_object,count_empty_map_things());
			break;
	}
}

LevelEditor::~LevelEditor()
{
	DestroyLevelTabs();
}

void	LevelEditor::SetupModule(void)
{
	LevelEdDefaults		the_defaults;


	the_defaults.Left	=	0;
	the_defaults.Top	=	20;
	the_defaults.Width	=	780;
	the_defaults.Height	=	560;
	SetupWindow	(
					"Level Editor",
					(HAS_TITLE|HAS_ICONS|HAS_GROW|HAS_CONTROLS),
					the_defaults.Left,
					the_defaults.Top,
					the_defaults.Width,
					the_defaults.Height
				);

	TopIcons.InitIcons(win_bar_icons);

	SetContentColour(CONTENT_COL);
	//SetControlsHeight(440);
	SetControlsHeight(520);
	SetControlsWidth(300);

	CreateLevelTabs();

	hilited_face.Face	=	0;
	selected_face.Face	=	0;

	// Mikes editor stuff.
	init_engine();

	
	next_prim_point		=	1;
	next_prim_face4		=	1;
	next_prim_face3		=	1;
	next_prim_object	=	1;
	next_prim_multi_object=	1;

	memset(prim_points,0,sizeof(PrimPoint)*MAX_PRIM_POINTS);
	memset(prim_faces4,0,sizeof(PrimFace4)*MAX_PRIM_FACES3);
	memset(prim_faces3,0,sizeof(PrimFace3)*MAX_PRIM_FACES4);
	memset(prim_objects,0,sizeof(PrimObject)*MAX_PRIM_OBJECTS);
	memset(prim_multi_objects,0,sizeof(PrimMultiObject)*MAX_PRIM_MOBJECTS);
	
	init_editor();

	background_prim		=	0;


	make_fade_table(PALETTE);
	make_mix_map(PALETTE);
	the_leveleditor=this;

	PrimSet.InitControlSet(prim_content_def);

//
//	For psx_alt
//
	PSXControls.InitControlSet(psx_content_def);

//
//	For style paint
//
	StyleControls.InitControlSet(style_content_def);
	((CEditText*)StyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->SetFlags(CONTROL_INACTIVE);

	((CEditText*)StyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->SetEditString("No Anim");

	((CVSlider*)StyleControls.GetControlPtr(CTRL_STYLE_POS_SLIDER))->SetUpdateFunction(slider_redraw);

	((CVSlider*)StyleControls.GetControlPtr(CTRL_STYLE_POS_SLIDER))->SetValueRange(0,60);
	((CVSlider*)StyleControls.GetControlPtr(CTRL_STYLE_POS_SLIDER))->SetCurrentValue(1);

//
//	For Inside Styles
//
	InStyleControls.InitControlSet(inside_style_content_def);
	((CEditText*)InStyleControls.GetControlPtr(CTRL_INSTYLE_NAME_EDIT))->SetFlags(CONTROL_INACTIVE);

	((CEditText*)InStyleControls.GetControlPtr(CTRL_INSTYLE_NAME_EDIT))->SetEditString("No Anim");

	((CVSlider*)InStyleControls.GetControlPtr(CTRL_INSTYLE_POS_SLIDER))->SetUpdateFunction(slider_redraw_in);

	((CVSlider*)InStyleControls.GetControlPtr(CTRL_INSTYLE_POS_SLIDER))->SetValueRange(0,50);
	((CVSlider*)InStyleControls.GetControlPtr(CTRL_INSTYLE_POS_SLIDER))->SetCurrentValue(1);
}

//---------------------------------------------------------------

void	LevelEditor::CreateLevelTabs(void)
{
	EdRect		bounds_rect;


	bounds_rect.SetRect(ControlsLeft(),ControlsTop(),ControlsWidth(),ControlsHeight());

	TestMode	=	new	ModeTab;
	if(TestMode)
	{
		AddTab(TestMode);
		TestMode->SetupModeTab("Test",TAB_NONE,&bounds_rect,ExternalUpdate);
	}

	PaintMode	=	new	PaintTab(this);
	if(PaintMode)
	{
		AddTab(PaintMode);
		PaintMode->SetupModeTab("Paint",TAB_PAINT,&bounds_rect,ExternalUpdate);
	}

	PrimMode	=	new	PrimPickTab(this);
	if(PrimMode)
	{
		AddTab(PrimMode);
		PrimMode->SetupModeTab("Prims",TAB_PRIMPICK,&bounds_rect,ExternalUpdate);
	}

	LightMode	=	new	LightTab(this);
	if(LightMode)
	{
		AddTab(LightMode);
		LightMode->SetupModeTab("Lights",TAB_LIGHT,&bounds_rect,ExternalUpdate);
	}

	/*

	ColMode	=	new	ColTab(this);
	if(ColMode)
	{
		AddTab(ColMode);
		ColMode->SetupModeTab("Collide",TAB_COL,&bounds_rect,ExternalUpdate);
	}

	*/
/*
	MapMode	=	new	MapTab(this);
	if(MapMode)
	{
		AddTab(MapMode);
		MapMode->SetupModeTab("Map bl",TAB_MAP,&bounds_rect,ExternalUpdate);
	}
*/
	BuildMode	=	new	BuildTab(this);
	if(BuildMode)
	{
		AddTab(BuildMode);
		BuildMode->SetupModeTab("Buildings",TAB_BUILD,&bounds_rect,ExternalUpdate);
	}

	MapEdMode	=	new	MapEdTab(this);
	if(MapEdMode)
	{
		AddTab(MapEdMode);
		MapEdMode->SetupModeTab("MapEDIT",TAB_MAPED,&bounds_rect,ExternalUpdate);
		MapEdMode->BuildMode=BuildMode;
	}

	HmMode		=	new HmTab(this);
	if (HmMode)
	{
		AddTab(HmMode);
		HmMode->SetupModeTab("HmEDIT",TAB_HM,&bounds_rect,ExternalUpdate);
	}

	SewerMode	=	new SewerTab(this);
	if (SewerMode)
	{
		AddTab(SewerMode);
		SewerMode->SetupModeTab("InsideEDIT",TAB_SEWER,&bounds_rect,ExternalUpdate);
	}
}


//---------------------------------------------------------------

void	LevelEditor::DestroyLevelTabs(void)
{
	/*
	if(ColMode)		 
		delete	ColMode;
	*/

	if(LightMode)		 
		delete	LightMode;

	if(PrimMode)		 
		delete	PrimMode;

	if(PaintMode)
		delete	PaintMode;

	if(TestMode)		 
		delete	TestMode;


	//if(MapMode)
	//	delete	MapMode;

	if(MapEdMode)
		delete	MapEdMode;

	if(BuildMode)
		delete	BuildMode;

	if (HmMode)
	{
		delete	HmMode;
	}

	if (SewerMode)
	{
		delete SewerMode;
	}
}

//---------------------------------------------------------------
//CONTENT WINDOW HAS DIFFERENT VIEWS DEPENDING ON TAB SELECTED 

void	build_texture(SLONG x,SLONG y,SLONG w,SLONG h,UBYTE page,UBYTE u0,UBYTE v0,UBYTE u1,UBYTE v1,UBYTE u2,UBYTE v2,UBYTE u3,UBYTE v3)
{
	
	setPolyType4(current_bucket_pool,POLY_T);

	setXY4((struct BucketQuad*)current_bucket_pool,
			x,y,
			x+w,y,
			x,y+h,
			x+w,y+h);

	setUV4NA((struct BucketQuad*)current_bucket_pool,
			u0,v0,u1,v1,u2,v2,u3,v3,page);


	((struct BucketQuad*)current_bucket_pool)->DebugInfo=0;

//	setShade4((struct BucketQuad*)current_bucket_pool,128,128,128,128);

	add_bucket((void *)current_bucket_pool,1);
	current_bucket_pool	+=	sizeof(struct BucketQuad);

}


void	LevelEditor::DrawAnimTmapContent(SLONG current_anim_tmap)
{
	EdRect	tex_rect;
	SLONG	w,h=32,count=0;
	CBYTE	str[100];

	animate_texture_maps();
	sprintf(str," EDIT ANIM TEXTURE %d",current_anim_tmap);
	QuickText(50,10,str,0);

	while(h<200&&count<MAX_TMAP_FRAMES)
	{
		for(w=10;(w<ContentWidth()-64)&&count<MAX_TMAP_FRAMES;w+=64)
		{

			tex_rect.SetRect(w-1,h-1,34,34);
//			tex_rect.OutlineRect(WHITE_COL);
			tex_rect.HiliteRect(HILITE_COL,LOLITE_COL);

			tex_rect.SetRect(w+40,h+10,16,12);
//			tex_rect.OutlineRect(WHITE_COL);
			tex_rect.HiliteRect(HILITE_COL,LOLITE_COL);



//			if(anim_tmaps[current_anim_tmap].Flags&&anim_tmaps[current_anim_tmap].Delay[count]>=0&&count<MAX_TMAP_FRAMES)
			if(anim_tmaps[current_anim_tmap].Flags&(1<<count))
			{
				sprintf(str,"%d",anim_tmaps[current_anim_tmap].Delay[count]);
				QuickTextC(w+42,h+12,str,WHITE_COL);
				build_texture(w,h,32,32,anim_tmaps[current_anim_tmap].Page[count],
				anim_tmaps[current_anim_tmap].UV[count][0][0],anim_tmaps[current_anim_tmap].UV[count][0][1],
				anim_tmaps[current_anim_tmap].UV[count][1][0],anim_tmaps[current_anim_tmap].UV[count][1][1],
				anim_tmaps[current_anim_tmap].UV[count][2][0],anim_tmaps[current_anim_tmap].UV[count][2][1],
				anim_tmaps[current_anim_tmap].UV[count][3][0],anim_tmaps[current_anim_tmap].UV[count][3][1]);

			}
			count++;
		}
		h+=50;
	}

	h=250;
	count=1;
	while(h<ContentHeight()-20&&count<MAX_ANIM_TMAPS)
	{
		for(w=10;(w<ContentWidth()-30)&&count<MAX_ANIM_TMAPS;w+=19)
		{

			tex_rect.SetRect(w,h,18,18);
//			tex_rect.OutlineRect(WHITE_COL);
			tex_rect.HiliteRect(HILITE_COL,LOLITE_COL);

			if(anim_tmaps[count].Flags)
			{
				UWORD	frame;
				frame=anim_tmaps[count].Current;

				build_texture(w+1,h+1,16,16,anim_tmaps[count].Page[frame],
				anim_tmaps[count].UV[frame][0][0],anim_tmaps[count].UV[frame][0][1],
				anim_tmaps[count].UV[frame][1][0],anim_tmaps[count].UV[frame][1][1],
				anim_tmaps[count].UV[frame][2][0],anim_tmaps[count].UV[frame][2][1],
				anim_tmaps[count].UV[frame][3][0],anim_tmaps[count].UV[frame][3][1]);

			}
			count++;
		}
		h+=22;
	}



	render_view(0);
}

void 	LevelEditor::HandleAnimTmapClick(UBYTE flags,MFPoint *clicked_point)
{
	EdRect	tex_rect;
	SLONG	w,h=32,count=0,c0;
	EdTexture	*current_texture;
	MFPoint		local_point;
	SLONG 	current_anim_tmap;

	current_anim_tmap=PaintMode->GetAnimTmap();

	local_point=*clicked_point;

	current_texture	=	PaintMode->GetTexture();
	GlobalToLocal(&local_point);

	while(h<200&&count<MAX_TMAP_FRAMES)
	{
		for(w=10;(w<ContentWidth()-64)&&count<MAX_TMAP_FRAMES;w+=64)
		{
			tex_rect.SetRect(w,h,32,32);
			if(tex_rect.PointInRect(&local_point)&&count<MAX_TMAP_FRAMES)
			{
				switch(flags)
				{
					case	LEFT_CLICK:
						SLONG	page;
						if(PaintMode->GetTexturePage()>=0)
						{
							
							for(c0=0;c0<4;c0++)
							{
								anim_tmaps[current_anim_tmap].UV[count][c0][0]=current_texture->U[c0];
								anim_tmaps[current_anim_tmap].UV[count][c0][1]=current_texture->V[c0];
							}
							anim_tmaps[current_anim_tmap].Flags|=(1<<count);
							page=(UBYTE)PaintMode->GetTexturePage();
							if(page<0)
								page=0;
							anim_tmaps[current_anim_tmap].Page[count]=page;
						}
						break;
					case	RIGHT_CLICK:
						for(c0=0;c0<4;c0++)
						{
							current_texture->U[c0]=anim_tmaps[current_anim_tmap].UV[count][c0][0];
							current_texture->V[c0]=anim_tmaps[current_anim_tmap].UV[count][c0][1];
						}
						PaintMode->SetTexturePage(anim_tmaps[current_anim_tmap].Page[count]);
						break;
					case	MIDDLE_CLICK:
						anim_tmaps[current_anim_tmap].Flags&=~(1<<count);
						break;
				}

				RequestUpdate();
				return;	
			}
			tex_rect.SetRect(w+40,h+10,16,12);
			if(tex_rect.PointInRect(&local_point)&&count<MAX_TMAP_FRAMES)
			{
				switch(flags)
				{
						case	NO_CLICK:
							break;
						case	MIDDLE_CLICK:
							break;
						case	LEFT_CLICK:
							anim_tmaps[current_anim_tmap].Delay[count]++;
							break;
						case	RIGHT_CLICK:
							anim_tmaps[current_anim_tmap].Delay[count]--;
							break;
				}
				RequestUpdate();
				return;
				
			}
			count++;

		}
		h+=50;
	}

	h=250;
	count=1;
	while(h<ContentHeight()-20&&count<MAX_ANIM_TMAPS)
	{
		for(w=10;(w<ContentWidth()-30)&&count<MAX_ANIM_TMAPS;w+=19)
		{

			tex_rect.SetRect(w,h,18,18);

			if(tex_rect.PointInRect(&local_point)&&count<MAX_ANIM_TMAPS)
			{
				switch(flags)
				{
						case	NO_CLICK:
							break;
						case	MIDDLE_CLICK:
							anim_tmaps[count].Flags=0;
							break;
						case	LEFT_CLICK:
							PaintMode->SetAnimTmap(count);
//							current_anim_tmap=count;
//							anim_tmaps[current_anim_tmap]=anim_tmaps[count];
							break;
						case	RIGHT_CLICK:
							PaintMode->SetAnimTmap(count);
							break;
				}
				RequestUpdate();
				return;
			}

			count++;
		}
		h+=22;
	}

}


void	draw_quad_now(SLONG x,SLONG y,SLONG w,SLONG h,UBYTE tx,UBYTE ty,UBYTE page,UBYTE flip,UBYTE flags)
{
	struct MfEnginePoint p1,p2,p3,p4;
	UBYTE	uv[4][2];

	p1.X=x;
	p1.Y=y;
	p2.X=x+w;
	p2.Y=y;
	p3.X=x;
	p3.Y=y+h;
	p4.X=x+w;
	p4.Y=y+h;

	switch(flip)
	{
		case	0:
			p1.TX=tx;
			p1.TY=ty;
			p2.TX=tx+31;
			p2.TY=ty;
			p3.TX=tx;
			p3.TY=ty+31;
			p4.TX=tx+31;
			p4.TY=ty+31;
			break;
		case	1: //flip x
			p1.TX=tx+31;
			p1.TY=ty;
			p2.TX=tx;
			p2.TY=ty;
			p3.TX=tx+31;
			p3.TY=ty+31;
			p4.TX=tx;
			p4.TY=ty+31;
			break;
		case	2: //flip y
			p1.TX=tx;
			p1.TY=ty+31;
			p2.TX=tx+31;
			p2.TY=ty+31;
			p3.TX=tx;
			p3.TY=ty;
			p4.TX=tx+31;
			p4.TY=ty;
			break;
		case	3: //flip y
			p1.TX=tx+31;
			p1.TY=ty+31;
			p2.TX=tx;
			p2.TY=ty+31;
			p3.TX=tx+31;
			p3.TY=ty;
			p4.TX=tx;
			p4.TY=ty;
			break;
	}


	poly_info.DrawFlags=flags&(~POLY_FLAG_GOURAD);
	poly_info.PTexture=tmaps[page];
	poly_info.Page=page;
	my_quad_noz(&p1,&p2,&p4,&p3);
}


void	LevelEditor::DrawTexStyleContent(void)
{
	SLONG	scroll_pos=PaintMode->CurrentStylePos;
	SLONG	c0,pos;
	EdRect	tex_rect;
	
	QuickTextC(10,10,"    Name                  MidL MidM  MidR  Mid2  Mid3 ",0);

	for(c0=0;c0<15;c0++)
	{
		if(PaintMode->CurrentStyleEdit==c0+scroll_pos)
		{
			tex_rect.SetRect(25,c0*26+30,110,24);
			tex_rect.OutlineRect(0);
		}
		QuickTextC(25,c0*26+36,&texture_style_names[c0+scroll_pos][0],0);

		for(pos=0;pos<5;pos++)
		{
//			draw_quad_now(200+pos*38,c0*20+30,16,16,textures_xy[c0+scroll_pos][pos].Tx<<5,textures_xy[c0+scroll_pos][pos].Ty<<5,textures_xy[c0+scroll_pos][pos].Page);
			draw_quad_now(140+pos*30,c0*26+30-2,24,24,textures_xy[c0+scroll_pos][pos].Tx<<5,textures_xy[c0+scroll_pos][pos].Ty<<5,textures_xy[c0+scroll_pos][pos].Page,textures_xy[c0+scroll_pos][pos].Flip,textures_flags[c0+scroll_pos][pos]);
		}

	}
	StyleControls.ControlSetBounds(GetContentRect());
	StyleControls.DrawControlSet();
}

void	LevelEditor::DrawPSXTexContent(void)
{
	SLONG	texpage;
	SLONG	c0,pos;
	EdRect	tex_rect;
	SLONG	x,y;
	if(PaintMode->CurrentStylePos>3)
		PaintMode->CurrentStylePos=3;
	texpage=PaintMode->CurrentStylePos;

	QuickTextC(100,2," PSX Texture Alternatives ",0);

	for(y=0;y<8;y++)
	for(x=0;x<8;x++)
	{
		draw_quad_now(40+x*50,20+y*50,48,48,x*32,y*32,texpage+25,0,POLY_T);

	}
	PSXControls.ControlSetBounds(GetContentRect());
	PSXControls.DrawControlSet();
}


void	slider_redraw(void)
{
//		the_leveleditor->PaintMode->CurrentStylePos	=	((CVSlider*)the_leveleditor->StyleControls.GetControlPtr(CTRL_STYLE_POS_SLIDER))->GetCurrentValue();
//		the_leveleditor->RequestUpdate();
	the_leveleditor->PaintMode->CurrentStylePos	=	((CVSlider*)the_leveleditor->StyleControls.GetControlPtr(CTRL_STYLE_POS_SLIDER))->GetCurrentValue();
	if(LockWorkScreen())
	{
		the_leveleditor->DrawContent();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}
void	slider_redraw_in(void)
{
//		the_leveleditor->PaintMode->CurrentStylePos	=	((CVSlider*)the_leveleditor->StyleControls.GetControlPtr(CTRL_STYLE_POS_SLIDER))->GetCurrentValue();
//		the_leveleditor->RequestUpdate();
	the_leveleditor->PaintMode->CurrentStylePos	=	((CVSlider*)the_leveleditor->InStyleControls.GetControlPtr(CTRL_INSTYLE_POS_SLIDER))->GetCurrentValue();
	if(LockWorkScreen())
	{
		the_leveleditor->DrawContent();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}
SLONG	LevelEditor::HandleTexStyleClick(UBYTE flags,MFPoint *clicked_point)
{
	SLONG	scroll_pos=PaintMode->CurrentStylePos;
	SLONG	c0,pos;
	EdRect	tex_rect;
	EdTexture	*current_texture;
	MFPoint		local_point;

	local_point=*clicked_point;

	current_texture	=	PaintMode->GetTexture();
	GlobalToLocal(&local_point);


	for(c0=0;c0<15;c0++)
	{
		tex_rect.SetRect(25,c0*26+30,110,24);
		if(tex_rect.PointInRect(&local_point))
		{
			//want to text edit the name
			PaintMode->CurrentStyleEdit=c0+scroll_pos;
			((CEditText*)StyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->SetEditString(&texture_style_names[c0+scroll_pos][0]);
			((CEditText*)StyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->SetFlags((UBYTE)(((CEditText*)StyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->GetFlags()&~CONTROL_INACTIVE));

			return(1);
			
		}
		for(pos=0;pos<5;pos++)
		{
			tex_rect.SetRect(140+pos*30,c0*26+30-2,24,24);
			if(tex_rect.PointInRect(&local_point))
			{
				SLONG	x,y;
				switch(flags)
				{
					case	LEFT_CLICK:
						if(Keys[KB_X])
						{
							if(textures_xy[c0+scroll_pos][pos].Flip&1)
								textures_xy[c0+scroll_pos][pos].Flip&=~1;
							else
								textures_xy[c0+scroll_pos][pos].Flip|=1;
							Keys[KB_X]=0;
							return(1);
						}

						if(Keys[KB_Y])
						{
							if(textures_xy[c0+scroll_pos][pos].Flip&2)
								textures_xy[c0+scroll_pos][pos].Flip&=~2;
							else
								textures_xy[c0+scroll_pos][pos].Flip|=2;
							Keys[KB_Y]=0;
							return(1);
						}
						x=	current_texture->U[0]+current_texture->U[1]+current_texture->U[2]+current_texture->U[3];
						y=	current_texture->V[0]+current_texture->V[1]+current_texture->V[2]+current_texture->V[3];

						x>>=(2+5);
						y>>=(2+5);
						textures_xy[c0+scroll_pos][pos].Tx=x;
						textures_xy[c0+scroll_pos][pos].Ty=y;
						textures_xy[c0+scroll_pos][pos].Page=PaintMode->GetTexturePage();

//						texture_info[x+y*8+64*PaintMode->GetTexturePage()].Type=c0+scroll_pos;
						return(1);

						break;
					case	RIGHT_CLICK:
						textures_flags[c0+scroll_pos][pos]=DoStylePopup(clicked_point,textures_flags[c0+scroll_pos][pos]);
						return(1);
				}
			}
		}

	}
	return(0);
}
extern	UWORD	page_remap[64*8];

SLONG	LevelEditor::HandlePSXTexClick(UBYTE flags,MFPoint *clicked_point)
{
	SLONG	tex_page=PaintMode->CurrentStylePos;
	EdRect	tex_rect;
	EdTexture	*current_texture;
	MFPoint		local_point;
	SLONG	x,y,u,v,page;

	if(tex_page>3)
		tex_page=3;

	local_point=*clicked_point;

	current_texture	=	PaintMode->GetTexture();
	GlobalToLocal(&local_point);

	for(y=0;y<8;y++)
	for(x=0;x<8;x++)
	{
		tex_rect.SetRect(40+x*50,20+y*50,48,48);
		if(tex_rect.PointInRect(&local_point))
		{
				switch(flags)
				{
					case	LEFT_CLICK:
						u=	current_texture->U[0]+current_texture->U[1]+current_texture->U[2]+current_texture->U[3];
						v=	current_texture->V[0]+current_texture->V[1]+current_texture->V[2]+current_texture->V[3];

						u>>=(2+5);
						v>>=(2+5);
						
						page=u+v*8+PaintMode->GetTexturePage()*64;
						if(page<8*64)
						{
							page_remap[page]=PaintMode->CurrentStylePos*64+x+y*8+1;
							if(Keys[KB_X])
							{
								page_remap[page]|=1<<14;
							}

							if(Keys[KB_Y])
							{
								page_remap[page]|=1<<15;
							}
						}
						RequestUpdate();
						return(1);
					break;
				}

		}

	}

	return(0);
}

void	LevelEditor::HandleStyleControl(ULONG  control_id)
{
	switch(control_id)
	{
		case	CTRL_STYLE_NAME_EDIT:
			if(PaintMode->CurrentStyleEdit)
			{
extern	void	fix_style_names(void);
				fix_style_names();
				strcpy(&texture_style_names[PaintMode->CurrentStyleEdit][0],((CEditText*)StyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->GetEditString());
			}
			//if(CurrentAnim)
			//	CurrentAnim->SetAnimName(((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->GetEditString());
			break;
		case	CTRL_STYLE_SAVE:
extern	void	save_texture_styles(UBYTE world);
			save_texture_styles(editor_texture_set);

			//SaveAllAnims(&test_chunk);			
			break;
		case	CTRL_STYLE_POS_SLIDER:
			PaintMode->CurrentStylePos	=	((CVSlider*)StyleControls.GetControlPtr(CTRL_STYLE_POS_SLIDER))->GetCurrentValue();
			slider_redraw();
			break;
	}

}

void	LevelEditor::HandlePSXControl(ULONG  control_id)
{
	switch(control_id)
	{
/*
		case	CTRL_PSX_SAVE:
extern	void	save_texture_styles(UBYTE world);
//			save_texture_styles(editor_texture_set);

			//SaveAllAnims(&test_chunk);			
			break;
*/
		case	CTRL_PSX_PAGE1:
			PaintMode->CurrentStylePos=0;
			break;
		case	CTRL_PSX_PAGE2:
			PaintMode->CurrentStylePos=1;
			break;
		case	CTRL_PSX_PAGE3:
			PaintMode->CurrentStylePos=2;
			break;
		case	CTRL_PSX_PAGE4:
			PaintMode->CurrentStylePos=3;
			break;
		case	CTRL_PSX_NOREMAP:
			{
				EdTexture	*current_texture;
				SLONG	u,v,page;
				current_texture	=	PaintMode->GetTexture();
				u=	current_texture->U[0]+current_texture->U[1]+current_texture->U[2]+current_texture->U[3];
				v=	current_texture->V[0]+current_texture->V[1]+current_texture->V[2]+current_texture->V[3];

				u>>=(2+5);
				v>>=(2+5);
				
				page=u+v*8+PaintMode->GetTexturePage()*64;
				if(page<8*64)
				{
					page_remap[page]=0;
				}
				RequestUpdate();
			}
			break;
	}

}



//
// Inside Styles
//

#define	IN_BOX_STEP	22
#define	IN_BOX_SIZE	18


void	LevelEditor::DrawTexInStyleContent(void)
{
	SLONG	scroll_pos=PaintMode->CurrentStylePos;
	SLONG	c0,pos;
	EdRect	tex_rect;
	
//	QuickTextC(10,10,"    Name                  MidL MidM  MidR  Mid2  Mid3 ",0);

	for(c0=0;c0<15;c0++)
	{
		if(PaintMode->CurrentStyleEdit==c0+scroll_pos)
		{
			tex_rect.SetRect(25,c0*26+30,110,24);
			tex_rect.OutlineRect(0);
		}
		QuickTextC(25,c0*26+36,inside_names[c0+scroll_pos],0);

		for(pos=0;pos<16;pos++)
		{
			SLONG	x,y,page,flip,flags,value;
//			draw_quad_now(200+pos*38,c0*20+30,16,16,textures_xy[c0+scroll_pos][pos].Tx<<5,textures_xy[c0+scroll_pos][pos].Ty<<5,textures_xy[c0+scroll_pos][pos].Page);
			value=inside_tex[c0+scroll_pos][pos];

			page=value/64+START_PAGE_FOR_FLOOR;
			value=value&63;
			x=(value&7)<<5;
			y=(value&(7<<3))<<2;

//			draw_quad_now(120+pos*20,c0*26+30-2,24,24,textures_xy[c0+scroll_pos][pos].Tx<<5,textures_xy[c0+scroll_pos][pos].Ty<<5,textures_xy[c0+scroll_pos][pos].Page,textures_xy[c0+scroll_pos][pos].Flip,textures_flags[c0+scroll_pos][pos]);
			draw_quad_now(120+pos*IN_BOX_STEP,c0*26+30-2,IN_BOX_SIZE,IN_BOX_SIZE,x,y,page,0,POLY_FLAG_TEXTURED);
		}

	}
	InStyleControls.ControlSetBounds(GetContentRect());
	InStyleControls.DrawControlSet();
}

void	slider_redraw_inside(void)
{
	if(LockWorkScreen())
	{
		the_leveleditor->PaintMode->CurrentStylePos	=	((CVSlider*)the_leveleditor->InStyleControls.GetControlPtr(CTRL_STYLE_POS_SLIDER))->GetCurrentValue();
		the_leveleditor->DrawContent();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}
SLONG	LevelEditor::HandleTexInStyleClick(UBYTE flags,MFPoint *clicked_point)
{
	SLONG	scroll_pos=PaintMode->CurrentStylePos;
	SLONG	c0,pos;
	EdRect	tex_rect;
	EdTexture	*current_texture;
	MFPoint		local_point;

	local_point=*clicked_point;

	current_texture	=	PaintMode->GetTexture();
	GlobalToLocal(&local_point);


	for(c0=0;c0<15;c0++)
	{
		tex_rect.SetRect(25,c0*26+30,90,24);
		if(tex_rect.PointInRect(&local_point))
		{
			//want to text edit the name
			PaintMode->CurrentStyleEdit=c0+scroll_pos;
			((CEditText*)InStyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->SetEditString(&texture_style_names[c0+scroll_pos][0]);
			((CEditText*)InStyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->SetFlags((UBYTE)(((CEditText*)InStyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->GetFlags()&~CONTROL_INACTIVE));

			return(1);
			
		}
		for(pos=0;pos<16;pos++)
		{
			tex_rect.SetRect(120+pos*IN_BOX_STEP,c0*26+30-2,IN_BOX_SIZE,IN_BOX_SIZE);
			if(tex_rect.PointInRect(&local_point))
			{
				SLONG	x,y;
				switch(flags)
				{
					case	LEFT_CLICK:
						x=	current_texture->U[0]+current_texture->U[1]+current_texture->U[2]+current_texture->U[3];
						y=	current_texture->V[0]+current_texture->V[1]+current_texture->V[2]+current_texture->V[3];

						x>>=(5+2);
						y>>=(5+2);

						if(PaintMode->GetTexturePage()>=START_PAGE_FOR_FLOOR)
							inside_tex[c0+scroll_pos][pos]=((PaintMode->GetTexturePage()-START_PAGE_FOR_FLOOR)<<6)+(x+(y<<3));

//						texture_info[x+y*8+64*PaintMode->GetTexturePage()].Type=c0+scroll_pos;
						return(1);

						break;
					case	RIGHT_CLICK:
//						textures_flags[c0+scroll_pos][pos]=DoStylePopup(clicked_point,textures_flags[c0+scroll_pos][pos]);
						return(1);
				}
			}
		}

	}
	return(0);
}

void	save_texture_instyles(UBYTE world)
{
	UWORD	temp,temp2;
	SLONG	save_type=1;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	CBYTE fname[MAX_PATH];

//	sprintf(fname, "u:\urbanchaos\\textures\\world%d\\instyle.tma", world);
	sprintf(fname, "%sinstyle.tma", TEXTURE_WORLD_DIR);

	handle=FileCreate(fname,1);

	if(handle!=FILE_OPEN_ERROR)
	{
		FileWrite(handle,(UBYTE*)&save_type,4);
		temp=9;		//how many texture_pages

		temp=64;
		temp2=16;
		FileWrite(handle,(UBYTE*)&temp,2);
		FileWrite(handle,(UBYTE*)&temp2,2);
		FileWrite(handle,(UBYTE*)&inside_tex[0][0],sizeof(UBYTE)*temp*temp2);
		temp=64;
		temp2=20;
		FileWrite(handle,(UBYTE*)&temp,2);
		FileWrite(handle,(UBYTE*)&temp2,2);
		FileWrite(handle,(UBYTE*)&inside_names[0][0],temp*temp2);

		FileClose(handle);
	}

}

void	LevelEditor::HandleInStyleControl(ULONG  control_id)
{
	switch(control_id)
	{
		case	CTRL_STYLE_NAME_EDIT:
			if(PaintMode->CurrentStyleEdit)
			{
extern	void	fix_style_names(void);
				fix_style_names();
				strcpy(&texture_style_names[PaintMode->CurrentStyleEdit][0],((CEditText*)InStyleControls.GetControlPtr(CTRL_STYLE_NAME_EDIT))->GetEditString());
			}
			//if(CurrentAnim)
			//	CurrentAnim->SetAnimName(((CEditText*)AnimControls.GetControlPtr(CTRL_ANIM_NAME_EDIT))->GetEditString());
			break;
		case	CTRL_STYLE_SAVE:
			save_texture_instyles(editor_texture_set);

			//SaveAllAnims(&test_chunk);			
			break;
		case	CTRL_STYLE_POS_SLIDER:
			PaintMode->CurrentStylePos	=	((CVSlider*)InStyleControls.GetControlPtr(CTRL_STYLE_POS_SLIDER))->GetCurrentValue();
			slider_redraw_inside();
			break;
	}

}

void	LevelEditor::DrawContent(void)
{
	EdRect	clear_rect;
	MapBlock	background;
//	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
	SetContentDrawArea();
//	ClearContent();

	clear_rect.SetRect(0,0,ContentWidth()-1,ContentHeight()-1);
	clear_rect.FillRect(CONTENT_COL_BR);
	switch(CurrentModeTab()->GetTabID())
	{
		case	TAB_NONE:
			set_camera_side();
			draw_editor_map(0);
			render_view(1);
			break;
		case	TAB_PAINT:
			set_camera();
			hilited_face.Face	=	0;
			if(PaintMode->GetPaintMode()==ANIM_TMAP_PAINT)
			{
				DrawAnimTmapContent(PaintMode->GetAnimTmap());	
			}
			else
			if(PaintMode->GetPaintMode()==PSX_TEX_DEFINE)
			{
				DrawPSXTexContent();	
			}
			else
			if(PaintMode->GetPaintMode()==STYLE_DEFINE)
			{
				DrawTexStyleContent();	
			}
			else
			if(PaintMode->GetPaintMode()==INSTYLE_DEFINE)
			{
				DrawTexInStyleContent();	
			}
			else
			if(PaintMode->GetPaintMode()==FLOOR_PAINT)
			{
				UWORD	temp;
				MapBlock	background;
				SLONG		mx,my,mz;
				MFPoint		mouse_point;

				temp=BuildMode->Texture;
				BuildMode->SetViewToEngine();
				BuildMode->Texture=2;


				if(PaintMode->SubMode==FLOOR_PASTE_BRUSH)
				{
					mouse_point.X=MouseX;
					mouse_point.Y=MouseY;
					GlobalToLocal(&mouse_point);
					BuildMode->CalcMapCoord(&mx,&my,&mz,ContentLeft(),ContentTop(),ContentWidth(),ContentHeight(),&mouse_point);
					background.Cut(mx>>ELE_SHIFT,mz>>ELE_SHIFT,PaintMode->CutMapBlock.GetWidth(),PaintMode->CutMapBlock.GetDepth(),0);
					PaintMode->CutMapBlock.Paste(mx>>ELE_SHIFT,mz>>ELE_SHIFT,PASTE_TEXTURE,0);
				}

				BuildMode->DrawModuleContent(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());

				//restore background
				if(PaintMode->SubMode==FLOOR_PASTE_BRUSH)
				{
					background.Paste(mx>>ELE_SHIFT,mz>>ELE_SHIFT,PASTE_TEXTURE,0);
					BuildMode->DrawContentRect(mx,mz,mx+(PaintMode->CutMapBlock.GetWidth()<<ELE_SHIFT),mz+(PaintMode->CutMapBlock.GetDepth()<<ELE_SHIFT),WHITE_COL);
				}
					//BuildMode->DrawBrush(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());
				BuildMode->Texture=temp;
			}
			else
			if(PrimMode)
			{
				switch(PrimMode->GetPrimTabMode())
				{
					case	PRIM_MODE_MULTI:
						draw_a_key_frame_at(PrimMode->GetCurrentPrim(),(engine.X>>8)+edit_info.DX,(engine.Y>>8)-edit_info.DY,(engine.Z>>8));
						animate_texture_maps();
						render_view(1);
						if(SelectFlag<5)
							SelectFlag=0;
						else
						if(LockWorkScreen())
						{
							PaintMode->DrawTab();
							UnlockWorkScreen();
						}
						ShowWorkWindow(0);

						break;
					case	PRIM_MODE_SINGLE:
					case	PRIM_MODE_BACK:
						if(PrimMode->GetCurrentPrim())
						{
							animate_texture_maps();
							draw_a_prim_at(PrimMode->GetCurrentPrim(),(engine.X>>8)+edit_info.DX,(engine.Y>>8)-edit_info.DY,(engine.Z>>8),0);
//							draw_a_building_at(PrimMode->GetCurrentPrim(),engine.X>>8,engine.Y>>8,engine.Z>>8);
							if(SelectFlag<5)
								SelectFlag=0;
							else
							if(LockWorkScreen())
							{
								PaintMode->DrawTab();
								UnlockWorkScreen();
							}
							ShowWorkWindow(0);
//							PaintMode->DrawTabContent();
						}
						else
							draw_editor_map(0);
						render_view(1);
						break;
				}
			}
			break;
		case	TAB_PRIMPICK:
			PrimMode->DrawModuleContent(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());

			PrimSet.ControlSetBounds(GetContentRect());
//			PrimSet.DrawControlSet();
			break;
		case	TAB_LIGHT:
			LightMode->DrawModuleContent(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());
			break;
		/*
		case	TAB_COL:
			ColMode->DrawModuleContent(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());
			break;
		*/
		case	TAB_MAP:
			MapMode->DrawModuleContent(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());
			break;
		case	TAB_MAPED:
				UWORD	temp;
				SLONG		mx,my,mz;
				MFPoint		mouse_point;

				temp=BuildMode->Texture;
				BuildMode->SetViewToEngine();
				if(MapEdMode->Texture)
				{
					BuildMode->Texture=6;
				}
				else
				{
					BuildMode->Texture=4;
				}
				BuildMode->RoofTop=MapEdMode->RoofTop;


				if(MapEdMode->Mode==FLOOR_PASTE_BRUSH)
				{
					mouse_point.X=MouseX;
					mouse_point.Y=MouseY;
					GlobalToLocal(&mouse_point);
					BuildMode->CalcMapCoord(&mx,&my,&mz,ContentLeft(),ContentTop(),ContentWidth(),ContentHeight(),&mouse_point);
					background.Cut(mx>>ELE_SHIFT,mz>>ELE_SHIFT,MapEdMode->CutMapBlock.GetWidth(),MapEdMode->CutMapBlock.GetDepth(),MapEdMode->RoofTop);
					MapEdMode->CutMapBlock.Paste(mx>>ELE_SHIFT,mz>>ELE_SHIFT,PASTE_ALTITUDE,MapEdMode->RoofTop);
				}


				BuildMode->DrawModuleContent(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());

				//restore background
				if(MapEdMode->Mode==FLOOR_PASTE_BRUSH)
				{
					background.Paste(mx>>ELE_SHIFT,mz>>ELE_SHIFT,PASTE_ALTITUDE,MapEdMode->RoofTop);
					BuildMode->DrawContentRect(mx,mz,mx+(MapEdMode->CutMapBlock.GetWidth()<<ELE_SHIFT),mz+(MapEdMode->CutMapBlock.GetDepth()<<ELE_SHIFT),WHITE_COL);
				}
				if(MapEdMode->Mode==FLOOR_HOLD_BRUSH)
				{

					mx=MapEdMode->CutMapBlock.GetX()<<ELE_SHIFT;
					mz=MapEdMode->CutMapBlock.GetZ()<<ELE_SHIFT;
					BuildMode->DrawContentRect(mx,mz,mx+(MapEdMode->CutMapBlock.GetWidth()<<ELE_SHIFT),mz+(MapEdMode->CutMapBlock.GetDepth()<<ELE_SHIFT),WHITE_COL);
				}

					//BuildMode->DrawBrush(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());
				BuildMode->Texture=temp;
			break;
		case	TAB_BUILD:
			BuildMode->DrawModuleContent(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());
			SewerMode->OutsideEditStorey=BuildMode->EditStorey;
			SewerMode->EditStorey=storey_list[BuildMode->EditStorey].InsideStorey;
			SewerMode->EditBuilding=BuildMode->EditBuilding;

			if(storey_list[BuildMode->EditStorey].InsideIDIndex)
				SewerMode->CurrentFloorType=room_ids[storey_list[BuildMode->EditStorey].InsideIDIndex].FloorType;

			SewerMode->SetView(storey_list[BuildMode->EditStorey].DX,storey_list[BuildMode->EditStorey].DZ);
			break;

		case	TAB_HM:
			HmMode->DrawModuleContent(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());
			break;

		case	TAB_SEWER:
			SewerMode->DrawModuleContent(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());
			break;

	}
}



void	LevelEditor::DragEngine(UBYTE flags,MFPoint *clicked_point)
{
	SLONG	wwx,wwy,www,wwh;
	SLONG	screen_change=0;
	SLONG	last_world_mouse;
	SLONG	ox,oy,oz;

	MFPoint		local_point;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;
	local_point=*clicked_point;

	GlobalToLocal(&local_point);
	ox=edit_info.DX;
	oy=edit_info.DY;
	oz=edit_info.DZ;

	{
		SLONG	old_x,old_y,old_z;


//		edit_info.DX=0;
//		edit_info.DY=0;
//		edit_info.DZ=0;

		old_x=(local_point.X<<11)/engine.Scale;
		old_y=(local_point.Y<<11)/engine.Scale;

		while(SHELL_ACTIVE && (MiddleButton||Keys[KB_SPACE]))
		{
			local_point.X	=	MouseX;
			local_point.Y	=	MouseY;
			GlobalToLocal(&local_point);
//			last_world_mouse=SetWorldMouse(0);
			edit_info.DX=ox+((local_point.X<<11)/engine.Scale)-old_x;
			edit_info.DY=oy+((local_point.Y<<11)/engine.Scale)-old_y;
			DrawContent();
			SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight());
			ShowWorkWindow(0);
			screen_change=1;
		}
	}
}
//---------------------------------------------------------------
//content clicks do different things depending on the tab selected 

void	LevelEditor::HandleContentClick(UBYTE flags,MFPoint *clicked_point)
{
	ULONG				update	=	0;
	SLONG				c0;
	EdTexture			*current_texture;
	MFPoint				local_point;

	switch(CurrentModeTab()->GetTabID())
	{
		case	TAB_PAINT:
			switch(PaintMode->GetPaintMode())
			{
				case	INSTYLE_DEFINE:
					if(!HandleTexInStyleClick(flags,clicked_point))
					{
						Control				*current_control;

						InStyleControls.SetControlDrawArea();
						local_point	=	*clicked_point;
						InStyleControls.GlobalToLocal(&local_point);
						current_control	=	InStyleControls.GetControlList();
						while(current_control)
						{
							if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(&local_point))
							{
								// Handle control.
								HandleInStyleControl(current_control->TrackControl(&local_point));

								// Tidy up display.
								if(LockWorkScreen())
								{
									InStyleControls.DrawControlSet();
									UnlockWorkScreen();
								}
								ShowWorkWindow(0);
							}
							current_control	=	current_control->GetNextControl();
						}

					}
					else
					{
						if(LockWorkScreen())
						{
							DrawContent();
							UnlockWorkScreen();
						}
						ShowWorkWindow(0);

					}
					break;
				case	PSX_TEX_DEFINE:
					if(!HandlePSXTexClick(flags,clicked_point))
					{
						Control				*current_control;

						PSXControls.SetControlDrawArea();
						local_point	=	*clicked_point;
						PSXControls.GlobalToLocal(&local_point);
						current_control	=	PSXControls.GetControlList();
						while(current_control)
						{
							if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(&local_point))
							{
								// Handle control.
								HandlePSXControl(current_control->TrackControl(&local_point));

								// Tidy up display.
								if(LockWorkScreen())
								{
									StyleControls.DrawControlSet();
									UnlockWorkScreen();
								}
								ShowWorkWindow(0);
							}
							current_control	=	current_control->GetNextControl();
						}

					}
					else
					{
								if(LockWorkScreen())
								{
									DrawContent();
									UnlockWorkScreen();
								}
								ShowWorkWindow(0);

					}
					break;
				case	STYLE_DEFINE:
					if(!HandleTexStyleClick(flags,clicked_point))
					{
						Control				*current_control;

						StyleControls.SetControlDrawArea();
						local_point	=	*clicked_point;
						StyleControls.GlobalToLocal(&local_point);
						current_control	=	StyleControls.GetControlList();
						while(current_control)
						{
							if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(&local_point))
							{
								// Handle control.
								HandleStyleControl(current_control->TrackControl(&local_point));

								// Tidy up display.
								if(LockWorkScreen())
								{
									StyleControls.DrawControlSet();
									UnlockWorkScreen();
								}
								ShowWorkWindow(0);
							}
							current_control	=	current_control->GetNextControl();
						}

					}
					else
					{
								if(LockWorkScreen())
								{
									DrawContent();
									UnlockWorkScreen();
								}
								ShowWorkWindow(0);

					}
					break;
				case	ANIM_TMAP_PAINT:
					HandleAnimTmapClick(flags,clicked_point);
					break;
				case	PALETTE_PAINT:
				case	TEXTURE_PAINT:
				case	FLOOR_PAINT:
				case	PLANAR_PAINT:
				case	STYLE_PAINT:
					SLONG	paint_city=0;
						if(PrimMode)
						{
							switch(PrimMode->GetPrimTabMode())
							{
								case	PRIM_MODE_SINGLE:
								case	PRIM_MODE_BACK:
								if(!PrimMode->GetCurrentPrim())
								{
									paint_city=1;
								}
							}
						}

					switch(flags)
					{
						case	NO_CLICK:
							break;
						case	MIDDLE_CLICK:
							DragEngine(flags,clicked_point);
							break;

						case	LEFT_CLICK:
							current_texture	=	PaintMode->GetTexture();
							if(PaintMode->SubMode==FLOOR_CUT_BRUSH)
							{
								PaintMode->CutFloorBrush(BuildMode,&local_point);

							}
							else
							if(ControlFlag&&!paint_city)
							{
								MFPoint		point1,point2;
								SLONG		con_top,con_left;
								point1.X	=	MouseX;
								point1.Y	=	MouseY;
		//						point1		=	*clicked_point;
								GlobalToLocal(&point1);
								while(SHELL_ACTIVE && LeftButton)
								{
									engine_keys_scroll();
									engine_keys_spin();
									engine_keys_zoom();

									point2.X	=	MouseX;//-con_left;
									point2.Y	=	MouseY;//-con_top;
									GlobalToLocal(&point2);

									edit_info.SelectRect.SetRect(point1.X,point1.Y,point2.X-point1.X,point2.Y-point1.Y);
									if(LockWorkScreen())
									{
										DrawContent();
										DrawGrowBox();
										edit_info.SelectRect.OutlineRect(WHITE_COL);
										UnlockWorkScreen();
									}
									ShowWorkWindow(0);
								}
								edit_info.SelectRect.NormalRect();
								SelectFlag=1;
								RequestUpdate();
							}
							else
							while(SHELL_ACTIVE && LeftButton)
							{
								if(PaintMode->SubMode==FLOOR_PASTE_BRUSH)
								{
									MFPoint		point1;
									SLONG		mx,my,mz;

										point1.X	=	MouseX;
										point1.Y	=	MouseY;
										GlobalToLocal(&point1);
										BuildMode->CalcMapCoord(&mx,&my,&mz,ContentLeft(),ContentTop(),ContentWidth(),ContentHeight(),&point1);
										PaintMode->CutMapBlock.Paste(mx>>ELE_SHIFT,mz>>ELE_SHIFT,PASTE_TEXTURE|(AltFlag?PASTE_ALTITUDE:0),0);

								}
								else
								if(ShiftFlag&&hilited_face.Face&&!paint_city)
								{  //selecting lots of faces
									if(!face_is_in_list(hilited_face.Face))
										add_face_to_list(hilited_face.Face);
								}
								else
								{
/*
									if(paint_city&&ShiftFlag)
									{
										if(hilited_face.Face>0)
										{
											SLONG	face;
											face=hilited_face.Face;
											set_wall_texture_info(-prim_faces4[face].ThingIndex,(SBYTE)texture_mode->GetTexturePage(),current_texture);
										}

									}
*/
									if(!ShiftFlag)
									if(ApplyTexture(&hilited_face))
									{
										update	=	1;
											
									}
								}
								if(LockWorkScreen())
								{
									DrawContent();
									DrawGrowBox();
									UnlockWorkScreen();
								}
								ShowWorkWindow(0);
								editor_turn++;

							}	
							break;
						case	RIGHT_CLICK:
							if(LockWorkScreen())
							{
								DrawContent();
								DrawGrowBox();
								UnlockWorkScreen();
							}
							ShowWorkWindow(0);
							if(ControlFlag&&!paint_city)
							{
								MFPoint		point1,point2;
								SLONG		con_top,con_left;
								point1.X	=	MouseX;
								point1.Y	=	MouseY;
								GlobalToLocal(&point1);
		//						con_left=ContentLeft();
		//						con_top=ContentTop();
								while(SHELL_ACTIVE && RightButton)
								{
									point2.X	=	MouseX;//-con_left;
									point2.Y	=	MouseY;//-con_top;
									GlobalToLocal(&point2);
									edit_info.SelectRect.SetRect(point1.X,point1.Y,point2.X-point1.X,point2.Y-point1.Y);
									if(LockWorkScreen())
									{
										DrawContent();
										DrawGrowBox();
										edit_info.SelectRect.OutlineRect(WHITE_COL);
										UnlockWorkScreen();
									}
									ShowWorkWindow(0);
								}
								edit_info.SelectRect.NormalRect();
								SelectFlag=-1;
								RequestUpdate();
							}
							else
							{
								
								current_texture	=	PaintMode->GetTexture();
								if(hilited_face.PEle)
								{
									selected_face	=	hilited_face;
									if(ShiftFlag&&hilited_face.Face&&hilited_face.PEle!=(struct EditMapElement*)-2)
									{  //selecting lots of faces
										if(face_is_in_list(hilited_face.Face))
											add_face_to_list(hilited_face.Face);
									}
									else
									if(hilited_face.PEle==(struct EditMapElement*)-2)
									{
										
										struct	MiniTextureBits	*tex;
										LogText(" get floor tex co-ords for on screen edit \n");
										if(edit_info.RoofTex)
										{
											tex=(struct	MiniTextureBits*)(&tex_map[selected_face.MapX][selected_face.MapZ]);
										}
										else
										{
											tex=(struct	MiniTextureBits*)(&edit_map[selected_face.MapX][selected_face.MapZ].Texture);
										}
										PaintMode->ConvertMiniTex(tex);
									}
									else
									if(hilited_face.PEle==(struct EditMapElement*)-1)
									{
										if(hilited_face.Face<0)
										{
											PaintMode->MyUndo.ApplyTexturePrim3(0,prim_faces3[-hilited_face.Face].TexturePage,-hilited_face.Face,
													prim_faces3[-hilited_face.Face].UV[0][0],
													prim_faces3[-hilited_face.Face].UV[0][1],
													prim_faces3[-hilited_face.Face].UV[1][0],
													prim_faces3[-hilited_face.Face].UV[1][1],
													prim_faces3[-hilited_face.Face].UV[2][0],
													prim_faces3[-hilited_face.Face].UV[2][1]);

											for(c0=0;c0<3;c0++)
											{
				//								prim_faces3[-edit_face.Face].TexturePage	=	(UWORD)TextureMode->GetTexturePage();
												current_texture->U[c0]	=	prim_faces3[-hilited_face.Face].UV[c0][0];
												current_texture->V[c0]	=	prim_faces3[-hilited_face.Face].UV[c0][1];
											}		
											PaintMode->SetTexturePage(prim_faces3[-hilited_face.Face].TexturePage);
											PaintMode->SetTextureFlags(PaintMode->GetTextureFlags()&~FLAGS_QUADS);

											PaintMode->SetCurrentColour(prim_faces3[-hilited_face.Face].Col2);


										}
										else
										{
											if(prim_faces4[hilited_face.Face].FaceFlags & FACE_FLAG_ANIMATE)
											{
												PaintMode->SetTexturePage(-prim_faces4[hilited_face.Face].TexturePage);
											}
											else
											{
												PaintMode->MyUndo.ApplyTexturePrim4(0,prim_faces3[hilited_face.Face].TexturePage,hilited_face.Face,
														prim_faces4[hilited_face.Face].UV[0][0],
														prim_faces4[hilited_face.Face].UV[0][1],
														prim_faces4[hilited_face.Face].UV[1][0],
														prim_faces4[hilited_face.Face].UV[1][1],
														prim_faces4[hilited_face.Face].UV[2][0],
														prim_faces4[hilited_face.Face].UV[2][1],
														prim_faces4[hilited_face.Face].UV[3][0],
														prim_faces4[hilited_face.Face].UV[3][1]);
												for(c0=0;c0<4;c0++)
												{
					//								prim_faces4[edit_face.Face].TexturePage	=	(UWORD)TextureMode->GetTexturePage();
													current_texture->U[c0]	=	prim_faces4[hilited_face.Face].UV[c0][0];
													current_texture->V[c0]	=	prim_faces4[hilited_face.Face].UV[c0][1];
												}


												{
													SLONG	dx,dy;
													dx=-current_texture->U[0]+current_texture->U[1];
													dy=-current_texture->V[0]+current_texture->V[1];

													if(dx>0&&dy==0)
													{
														PaintMode->CurrentTextureRot=0;
													}
													else
													if(dx==0&&dy>0)
													{
														PaintMode->CurrentTextureRot=1;
													}
													else
													if(dx<0&&dy==0)
													{
														PaintMode->CurrentTextureRot=2;
													}
													else
													if(dx==0&&dy<0)
													{
														PaintMode->CurrentTextureRot=3;
													}


												}
											


												PaintMode->SetTexturePage(prim_faces4[hilited_face.Face].TexturePage);
												PaintMode->SetTextureFlags(PaintMode->GetTextureFlags()|FLAGS_QUADS);
												if(prim_faces4[hilited_face.Face].ThingIndex<0)
												{
													SLONG	wall;
													wall=prim_faces4[hilited_face.Face].ThingIndex;

													PaintMode->CurrentStyleEdit=wall_list[wall].TextureStyle;

												}

												PaintMode->SetCurrentColour(prim_faces4[hilited_face.Face].Col2);
											}
										}
										if(paint_city)
											PaintMode->SetTextureFlags((PaintMode->GetTextureFlags())|FLAGS_SHOW_TEXTURE);
										else
											PaintMode->SetTextureFlags((PaintMode->GetTextureFlags()&~FLAGS_FIXED)|FLAGS_SHOW_TEXTURE);
									}
									else
									{
										PaintMode->ConvertFixedToFree(&selected_face.PEle->Textures[selected_face.Face]);
										PaintMode->SetTextureFlags(PaintMode->GetTextureFlags()|FLAGS_QUADS|FLAGS_FIXED);
									}

									if(RightButton)
									{
										DoFacePopup(clicked_point);
										update	=	1;
									}
									if(LockWorkScreen())
									{
										CurrentModeTab()->DrawTab();
										UnlockWorkScreen();
										ShowWorkWindow(0);
									}
								}
							}
							break;
					}
					break;
			}
			break;
		case	TAB_PRIMPICK:
		//in here we may be pasteing a 3ds prim
		//we may be dragging it somewhere
		{
			
			MFPoint	local_point;
			local_point=*clicked_point;
			GlobalToLocal(&local_point);


			if(PrimMode->HandleModuleContentClick(&local_point,flags,ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight()))
			{
				if(LockWorkScreen())
				{
					DrawContent();
					DrawGrowBox();
					UnlockWorkScreen();
				}
				ShowWorkWindow(0);
			}
		}
			
			break;
		case	TAB_LIGHT:
		{
			

			MFPoint	local_point;
			local_point=*clicked_point;
			GlobalToLocal(&local_point);


			if(LightMode->HandleModuleContentClick(&local_point,flags,ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight()))
			{
				if(LockWorkScreen())
				{
					DrawContent();
					DrawGrowBox();
					UnlockWorkScreen();
				}
				ShowWorkWindow(0);
			}
		}
			
			break;
		/*
		case	TAB_COL:
		{
			MFPoint	local_point;
			local_point=*clicked_point;
			GlobalToLocal(&local_point);


			if(ColMode->HandleModuleContentClick(&local_point,flags,ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight()))
			{
				if(LockWorkScreen())
				{
					DrawContent();
					DrawGrowBox();
					UnlockWorkScreen();
				}
				ShowWorkWindow(0);
			}
		}
			
			break;
		*/
		case	TAB_MAP:
		{
			MFPoint	local_point;
			local_point=*clicked_point;
			GlobalToLocal(&local_point);

			if(MapMode->HandleModuleContentClick(&local_point,flags,ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight()))
			{
				if(LockWorkScreen())
				{
					DrawContent();
					DrawGrowBox();
					UnlockWorkScreen();
				}
				ShowWorkWindow(0);
			}
		}
			
			break;
		case	TAB_MAPED:
		{
			MFPoint	local_point;
			local_point=*clicked_point;
			GlobalToLocal(&local_point);


			if(MapEdMode->HandleModuleContentClick(&local_point,flags,ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight()))
			{
				if(LockWorkScreen())
				{
					DrawContent();
					DrawGrowBox();
					UnlockWorkScreen();
				}
				ShowWorkWindow(0);
			}
		}
		break;
		case	TAB_BUILD:
		{
			MFPoint	local_point;
			local_point=*clicked_point;
			GlobalToLocal(&local_point);


			if(BuildMode->HandleModuleContentClick(&local_point,flags,ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight()))
			{
				if(LockWorkScreen())
				{
					DrawContent();
					DrawGrowBox();
					UnlockWorkScreen();
				}
				ShowWorkWindow(0);
			}
			SewerMode->OutsideEditStorey=BuildMode->EditStorey;
			SewerMode->EditStorey=storey_list[BuildMode->EditStorey].InsideStorey;
			SewerMode->EditBuilding=BuildMode->EditBuilding;

			SewerMode->SetView(storey_list[BuildMode->EditStorey].DX,storey_list[BuildMode->EditStorey].DZ);

			if(SewerMode->OutsideEditStorey)
			{
				SLONG	inside,index;
				index=storey_list[SewerMode->OutsideEditStorey].InsideIDIndex; //building_list[building].StoreyHead;
				if(index)
				{
					SewerMode->CurrentFloorType=room_ids[index].FloorType;
				}

			}

		}
		break;
		case	TAB_HM:
		{
			MFPoint	local_point;
			local_point=*clicked_point;
			GlobalToLocal(&local_point);


			if(HmMode->HandleModuleContentClick(&local_point,flags,ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight()))
			{
				if(LockWorkScreen())
				{
					DrawContent();
					DrawGrowBox();
					UnlockWorkScreen();
				}
				ShowWorkWindow(0);
			}
		}

		case	TAB_SEWER:
		{
			MFPoint	local_point;
			local_point=*clicked_point;
			GlobalToLocal(&local_point);


			if(SewerMode->HandleModuleContentClick(&local_point,flags,ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight()))
			{
				if(LockWorkScreen())
				{
					DrawContent();
					DrawGrowBox();
					UnlockWorkScreen();
				}
				ShowWorkWindow(0);
			}
		}

			
	}
	if(update)
	{
		if(LockWorkScreen())
		{
			DrawContent();
			DrawGrowBox();
			UnlockWorkScreen();
		}
		ShowWorkWindow(0);
	}
	clicked_point	=	clicked_point;
}

//---------------------------------------------------------------

void	LevelEditor::HandleControlClick(UBYTE flags,MFPoint *clicked_point)
{
	UWORD		control_id;


	if(CurrentModeTab())
	{
		control_id	=	CurrentModeTab()->HandleTabClick(flags,clicked_point);
	}
}

//---------------------------------------------------------------
extern	void	zoom_map_onto_screen(void);

void	ApplyShadow(struct EditFace *edit_face,SLONG shadow)
{
	SLONG			c0;
	
	if(edit_face->PEle==(struct EditMapElement*)-2)
	{
		{
			edit_map[edit_face->MapX][edit_face->MapZ].Flags=(edit_map[edit_face->MapX][edit_face->MapZ].Flags&~7)|shadow;
		}
	}
}


void	LevelEditor::HandleModule(void)
{
	ULONG			update	=	0;
	SLONG			c0,
					temp_u,
					temp_v;
	EdTexture		*current_texture;
	MFPoint			mouse_point;
	MFTime			the_time;
	static SLONG	last_msecond;
	static EditFace	last_face;

	/*

	if(LastKey==KB_P0)
	{
		switch(CurrentModeTab()->GetTabID())
		{
			case	TAB_NONE:
				break;
			case	TAB_PRIMPICK:
			case	TAB_PAINT:
			case	TAB_LIGHT:
			//case	TAB_COL:
			case	TAB_MAP:
			case	TAB_MAPED:
				zoom_map_onto_screen();
				update			=	2;
				break;
		}
		LastKey=0;
	}

	*/


	if(Keys[KB_J])
	{
		Keys[KB_J]=0;
void	swap_maps();
		swap_maps();

	}

	if((CurrentModeTab()->GetTabID()==TAB_PAINT))
	{
		if(PrimMode)
		{
			switch(PrimMode->GetPrimTabMode())
			{
				case	PRIM_MODE_SINGLE:
				case	PRIM_MODE_MULTI:
				case	PRIM_MODE_BACK:
				if(PrimMode->GetCurrentPrim())
				{


					if(LastKey==KB_S&&(!ShiftFlag))
					{
						
						SelectFlag=3;
						update			=	1;
					}

					if(LastKey==KB_S&&(ShiftFlag))
					{
						SelectFlag=4;
						update			=	1;
					}
				}
			}
		}
	}
/*
	if(LastKey==KB_Q)
	{
		ApplyShadow(&hilited_face,0);
		update			=	2;
		LastKey=0;
	}

	if(LastKey==KB_W)
	{
		ApplyShadow(&hilited_face,1);
		update			=	2;
		LastKey=0;
	}
	if(LastKey==KB_E)
	{
		ApplyShadow(&hilited_face,2);
		update			=	2;
		LastKey=0;
	}
	if(LastKey==KB_R)
	{
		ApplyShadow(&hilited_face,3);
		update			=	2;
		LastKey=0;
	}
	if(LastKey==KB_T)
	{
		ApplyShadow(&hilited_face,4);
		update			=	2;
		LastKey=0;
	}

	if(LastKey==KB_Y)
	{
		ApplyShadow(&hilited_face,5);
		update			=	2;
		LastKey=0;
	}
  */
	mouse_point.X	=	MouseX;
	mouse_point.Y	=	MouseY;
	if(CurrentModeTab())
	{
		CurrentModeTab()->HandleTab(&mouse_point);
		switch(CurrentModeTab()->GetTabID())
		{
			case	TAB_NONE:
				break;
			case	TAB_PAINT:
				current_texture	=	PaintMode->GetTexture();
				if(LastKey==KB_SPACE)
				{
					MFPoint		local_point;

					local_point.X	=	MouseX;
					local_point.Y	=	MouseY;

					DragEngine(0,&local_point);
				}

				if(PointInContent(&mouse_point))
				{
					update			=	2;
				}
				if(PaintMode->SubMode==FLOOR_PASTE_BRUSH)
					update=2;
				else if(hilited_face.Face)
				{
					hilited_face.Face	=	0;
					hilited_face.Bucket=	0;
					update			=	1;
				}

				if(LastKey==KB_Z)
				{
					if(PaintMode->SubMode==FLOOR_PASTE_BRUSH)
					{
						PaintMode->CutMapBlock.Rotate(1);
					}
					else
					if(selected_face.PEle==(struct EditMapElement*)-2)
					{
						struct	MiniTextureBits	*tex;
						if(edit_info.RoofTex)
						{
							tex=(struct	MiniTextureBits*)(&tex_map[selected_face.MapX][selected_face.MapZ]);
						}
						else
						{
							tex=(struct	MiniTextureBits*)(&edit_map[selected_face.MapX][selected_face.MapZ].Texture);
						}
						PaintMode->CurrentTextureRot++;
						PaintMode->CurrentTextureRot&=3;
						tex->Rot=PaintMode->CurrentTextureRot;
						PaintMode->ConvertMiniTex(tex);
						
					}
					else
					{
						
						PaintMode->CurrentTextureRot++;
						PaintMode->CurrentTextureRot&=3;

						temp_u	=	current_texture->U[0];
						temp_v	=	current_texture->V[0];
						if(PaintMode->GetTextureFlags()&FLAGS_QUADS)
						{
							current_texture->U[0]	=	current_texture->U[1];
							current_texture->V[0]	=	current_texture->V[1];
							current_texture->U[1]	=	current_texture->U[3];
							current_texture->V[1]	=	current_texture->V[3];
							current_texture->U[3]	=	current_texture->U[2];
							current_texture->V[3]	=	current_texture->V[2];
						}
						else
						{
							for(c0=0;c0<2;c0++)
							{
								current_texture->U[c0]	=	current_texture->U[c0+1];
								current_texture->V[c0]	=	current_texture->V[c0+1];
							}
						}
						current_texture->U[2]	=	temp_u;
						current_texture->V[2]	=	temp_v;
						ApplyTexture(&selected_face);
					}
					update	=	2;
					LastKey	=	0;
				}
				if(LastKey==KB_X)
				{
					if(PaintMode->GetTextureFlags()&FLAGS_QUADS)
					{
						SWAP(current_texture->U[0],current_texture->U[1]);
						SWAP(current_texture->V[0],current_texture->V[1]);
						SWAP(current_texture->U[2],current_texture->U[3]);
						SWAP(current_texture->V[2],current_texture->V[3]);

					}
					ApplyTexture(&selected_face);
					update	=	2;
					LastKey	=	0;
				}
				break;
			case	TAB_PRIMPICK:
				SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
				PrimMode->SetWorldMouse(0);
/*
				if(PrimMode->HiLightObjects(ContentLeft()+1,ContentTop()+1,ContentWidth(),ContentHeight()))
				{
					ShowWorkWindow(0);
				}
*/
				if(PrimMode->RedrawModuleContent)
					DrawContent();
			  	break;
			case	TAB_LIGHT:
				if(LightMode->RedrawModuleContent)
					DrawContent();
				break;
			/*
			case	TAB_COL:
				if(ColMode->RedrawModuleContent)
					DrawContent();
				break;
			*/
			case	TAB_MAP:
				if(MapMode->RedrawModuleContent)
					DrawContent();
				break;
			case	TAB_BUILD:
				if(BuildMode->RedrawModuleContent)
					DrawContent();
					{
						MFPoint		mouse_point;
						mouse_point.X	=	MouseX;
						mouse_point.Y	=	MouseY;
						if(GetContentRect()->PointInRect(&mouse_point))
						{
							BuildMode->MouseInContent();
						}
			SewerMode->OutsideEditStorey=BuildMode->EditStorey;
			SewerMode->EditStorey=storey_list[BuildMode->EditStorey].InsideStorey;
			SewerMode->EditBuilding=BuildMode->EditBuilding;
			if(storey_list[BuildMode->EditStorey].InsideIDIndex)
				SewerMode->CurrentFloorType=room_ids[storey_list[BuildMode->EditStorey].InsideIDIndex].FloorType;

			SewerMode->SetView(storey_list[BuildMode->EditStorey].DX,storey_list[BuildMode->EditStorey].DZ);

						
					}
				break;
			case	TAB_SEWER:
				if(BuildMode->RedrawModuleContent)
					DrawContent();
					{
						MFPoint		mouse_point;
						mouse_point.X	=	MouseX;
						mouse_point.Y	=	MouseY;
						if(GetContentRect()->PointInRect(&mouse_point))
						{
							SewerMode->MouseInContent();
						}
						
					}
				break;
			case	TAB_MAPED:
				if(MapEdMode->RedrawModuleContent)
					DrawContent();
					{
						MFPoint		mouse_point;
						mouse_point.X	=	MouseX;
						mouse_point.Y	=	MouseY;
						if(GetContentRect()->PointInRect(&mouse_point))
						{
							MapEdMode->MouseInContent();
						}
						
					}
				break;

			case	TAB_HM:
				if (HmMode->RedrawModuleContent)
				{
					DrawContent();
				}
				break;
		}
	}

	// This weird bit of code only updates the front display if the user
	// changes the view in any way (scroll, rotate etc.) or the currently
	// hilited poly changes.
	Time(&the_time);
	if(hilited_face.Face!=last_face.Face)
	{
		last_face	=	hilited_face;
		update		=	2;
	}
	if(selected_face.Face && (the_time.MSeconds/251)!=last_msecond)
	{
		last_msecond	=	the_time.MSeconds/251;
		select_colour	=	~select_colour;
		update			=	2;
	}
	if(CurrentModeTab())
	{
		SLONG	temp_update=0;
		CurrentModeTab()->HandleTab(&mouse_point);
		switch(CurrentModeTab()->GetTabID())
		{

			case	TAB_BUILD:
				update=BuildMode->DoKeys();
				break;
			case	TAB_SEWER:
				update=SewerMode->DoKeys();
				break;

			case	TAB_MAPED:
extern	ULONG	engine_keys_scroll_plan(void);
				//temp_update|=engine_keys_zoom();
				update  += BuildMode->DoZoom();
				temp_update|=engine_keys_scroll_plan();
				update	+=	(temp_update<<1);
				break;
			case	TAB_PAINT:
				if(PaintMode->GetPaintMode()==FLOOR_PAINT)
				{
					
					update  +=  BuildMode->DoZoom();
					update	+=	(editor_user_interface(2)<<1);
				}
				else
					update	+=	(editor_user_interface(1)<<1);
				break;

			case	TAB_HM:
				
				//
				// The HM tab does it own keys thankyou very much!!!
				//

				break;

			default:
				update	+=	(editor_user_interface(0)<<1);
				break;
			
		}
	}

	if(update)
	{
		if(LockWorkScreen())
		{
			DrawContent();
			DrawGrowBox();
			UnlockWorkScreen();
		}
		if(update>1)
			ShowWorkWindow(0);
	}

}

//---------------------------------------------------------------

static ControlDef		popup_def	=	{	POPUP_MENU,	0,	""};
MenuDef2	face_popup[]	=
{
	{	"~Gouraud"			},
	{	"~Textured"			},
	{	"~Masked"			},
	{	"~Transparent"		},
	{	"~Alpha"			},
	{	"~Tiled"			},
	{	"~2Sided"			},
	{	"~Walkable"			},
	{	"~Other Split"		},
	{	"~Non Planar"		},
	{	"~Env-mapped"		},
	{	"~Tinted"			},
	{	"!"					}
};
















SLONG	find_texture_point_for_face(SWORD face)
{
	SLONG	stx=99999,sty=99999;
	SLONG	x,y;
	SLONG	tx,ty;
	SLONG	point,tp=0;

	if(face<0) //tri's
	{
		for(point=0;point<3;point++)
		{
			x=prim_points[prim_faces3[-face].Points[point]].X;
			y=prim_points[prim_faces3[-face].Points[point]].Y;

			if(x+y<stx+sty)
			{
				stx=x;
				sty=y;
				tp=point;
			}
		}
	}
	else
	{


		for(point=0;point<4;point++)
		{
			x=prim_points[prim_faces4[face].Points[point]].X;
			y=prim_points[prim_faces4[face].Points[point]].Y;

			if(x+y<stx+sty)
			{
				stx=x;
				sty=y;
				tp=point;
			}

		}
	}
	return(tp);
	
}

void	fix_all_selected_faces_for_tile_mode(void)
{
	SLONG	c0;
	SWORD	face;
	SLONG	sx=99999,sy=99999;
	SLONG	stx=99999,sty=99999;
	SLONG	scale_point_x,scale_point_y;

	SLONG	x,y,dx,dy;
	SLONG	tx,ty;
	SLONG	point;
	ULONG	tp;

//	SelectFlag=2;

	edit_info.TileFlag=1;
	for(c0=1;c0<next_face_selected;c0++)
	{
		face=face_selected_list[c0];

		if(face<0) //tri's
		{
			for(point=0;point<3;point++)
			{
				x=prim_points[prim_faces3[-face].Points[point]].X;
				y=prim_points[prim_faces3[-face].Points[point]].Y;

				if(x<sx)
					sx=x;
				if(y<sy)
					sy=y;

				x=prim_faces3[-face].UV[point][0];
				y=prim_faces3[-face].UV[point][1];

				if(x+y<stx+sty)
				{
					stx=x;
					sty=y;
				}
			}
		}
		else
		{
			for(point=0;point<4;point++)
			{
				x=prim_points[prim_faces4[face].Points[point]].X;
				y=prim_points[prim_faces4[face].Points[point]].Y;

				if(x<sx)
					sx=x;
				if(y<sy)
					sy=y;

				x=prim_faces4[face].UV[point][0];
				y=prim_faces4[face].UV[point][1];

				if(x+y<stx+sty)
				{
					stx=x;
					sty=y;
				}
			}
		}
	}
	for(c0=1;c0<next_face_selected;c0++)
	{
		face=face_selected_list[c0];

		if(face<0) //tri's
		{
			stx=stx&0xffffffe0;
			sty=sty&0xffffffe0;
			tp=find_texture_point_for_face(face);
			scale_point_x=((prim_points[prim_faces3[-face].Points[tp]].X-sx)*edit_info.TileScale)>>7;
			scale_point_y=((prim_points[prim_faces3[-face].Points[tp]].Y-sy)*edit_info.TileScale)>>7;
			dx=stx-(stx+scale_point_x&0xffffffe0);
			dy=sty-(sty+scale_point_y&0xffffffe0);
			for(point=0;point<3;point++)
			{

				x=((prim_points[prim_faces3[-face].Points[point]].X-sx)*edit_info.TileScale)>>7;
				y=((prim_points[prim_faces3[-face].Points[point]].Y-sy)*edit_info.TileScale)>>7;

				if(stx+x+dx>255)
					prim_faces3[-face].UV[point][0]=255;
				else
					prim_faces3[-face].UV[point][0]=stx+x+dx;
				if(sty+y+dy>255)
					prim_faces3[-face].UV[point][1]=255;
				else
					prim_faces3[-face].UV[point][1]=sty+y+dy;
			}
		}
		else
		{

			stx=stx&0xffffffe0;
			sty=sty&0xffffffe0;
			tp=find_texture_point_for_face(face);
			scale_point_x=((prim_points[prim_faces4[face].Points[tp]].X-sx)*edit_info.TileScale)>>7;
			scale_point_y=((prim_points[prim_faces4[face].Points[tp]].Y-sy)*edit_info.TileScale)>>7;
			dx=stx-(stx+scale_point_x&0xffffffe0);
			dy=sty-(sty+scale_point_y&0xffffffe0);
			if(ShiftFlag)
			{
				dx=0;
				dy=0;
			}
			for(point=0;point<4;point++)
			{

				x=((prim_points[prim_faces4[face].Points[point]].X-sx)*edit_info.TileScale)>>7;
				y=((prim_points[prim_faces4[face].Points[point]].Y-sy)*edit_info.TileScale)>>7;

				if(stx+x+dx>255)
					prim_faces4[face].UV[point][0]=255;
				else
					prim_faces4[face].UV[point][0]=stx+x+dx;
				if(sty+y+dy>255)
					prim_faces4[face].UV[point][1]=255;
				else
					prim_faces4[face].UV[point][1]=sty+y+dy;
			}
		}
	}
}

#define	POPUP_HEIGHT	10*20
void	LevelEditor::DoFacePopup(MFPoint *clicked_point)
{
	ULONG			flags;
	ULONG			c0,
					control_id;
	CPopUp			*the_control	=	0;
	MFPoint			local_point;
	UBYTE			old_flags;

	local_point	=	*clicked_point;
	GlobalToLocal(&local_point);
	popup_def.ControlLeft	=	local_point.X+4;
	popup_def.ControlTop	=	local_point.Y-4;	 

//	if(local_point.Y+POPUP_HEIGHT>ContentHeight())
//		local_point.Y=ContentHeight()-POPUP_HEIGHT;

	if(CurrentModeTab())
	{
		switch(CurrentModeTab()->GetTabID())
		{
			case	TAB_NONE:
				break;
			case	TAB_PAINT:
				if(hilited_face.PEle==(struct EditMapElement*)-2)
				{
					return;
					
				}
				else
				if(hilited_face.PEle==(struct EditMapElement*)-1)
				{

					if(hilited_face.Face<0)
						flags	=	prim_faces3[-hilited_face.Face].DrawFlags;
					else
						flags	=	prim_faces4[hilited_face.Face].DrawFlags;
				}
				else
				if(hilited_face.PEle)
				{
					flags=hilited_face.PEle->Textures[hilited_face.Face].DrawFlags;

				}
				if(hilited_face.Face>0)
				{
					if(prim_faces4[hilited_face.Face].FaceFlags&FACE_FLAG_OTHER_SPLIT)
					{
						flags|=1<<8;

					}
					if(prim_faces4[hilited_face.Face].FaceFlags&FACE_FLAG_NON_PLANAR)
					{
						flags|=1<<9;
					}

					//
					// Set 'flags' depending on the environment and tint flags.
					//

					if (prim_faces4[hilited_face.Face].FaceFlags & FACE_FLAG_ENVMAP) {flags |= 1 << 10;}
					if (prim_faces4[hilited_face.Face].FaceFlags & FACE_FLAG_TINT)   {flags |= 1 << 11;}
				}

				if (hilited_face.Face < 0)
				{
					//
					// Set 'flags' depending on the environment and tint flags.
					//

					if (prim_faces3[-hilited_face.Face].FaceFlags & FACE_FLAG_ENVMAP) {flags |= 1 << 10;}
					if (prim_faces3[-hilited_face.Face].FaceFlags & FACE_FLAG_TINT)   {flags |= 1 << 11;}
				}

				old_flags=flags;
//				face_popup[7].ItemFlags	=	0;
				for(c0=0;c0<12;c0++)
				{
					face_popup[c0].ItemFlags	=	0;
					if(flags&(1<<c0))
						face_popup[c0].ItemFlags	|=	MENU_CHECK_MASK;
				}
				popup_def.TheMenuDef	=	face_popup;
				the_control		=	new CPopUp(&popup_def);
				control_id		=	the_control->TrackControl(&local_point);
				flags			=	0;
/*
				if(next_face_selected>1&&face_is_in_list(hilited_face.Face))
					if(face_popup[7].ItemFlags&MENU_CHECK_MASK)
					{
						fix_all_selected_faces_for_tile_mode();
						return;
					}
*/

				for(c0=0;c0<12;c0++)
				{
					if(face_popup[c0].ItemFlags&MENU_CHECK_MASK)
						flags	|=	(1<<c0);
				}

				if(hilited_face.Face>0)
				{
					if(face_popup[8].ItemFlags&MENU_CHECK_MASK)
					{
						prim_faces4[hilited_face.Face].FaceFlags|=FACE_FLAG_OTHER_SPLIT;

					}
					if(face_popup[9].ItemFlags&MENU_CHECK_MASK)
					{
						prim_faces4[hilited_face.Face].FaceFlags|=FACE_FLAG_NON_PLANAR;
					}

					//
					// Set the envinronment map and tint flags.
					// 

					prim_faces4[hilited_face.Face].FaceFlags &= ~FACE_FLAG_ENVMAP;
					prim_faces4[hilited_face.Face].FaceFlags &= ~FACE_FLAG_TINT;

					if (face_popup[10].ItemFlags & MENU_CHECK_MASK) {prim_faces4[hilited_face.Face].FaceFlags |= FACE_FLAG_ENVMAP;}
					if (face_popup[11].ItemFlags & MENU_CHECK_MASK) {prim_faces4[hilited_face.Face].FaceFlags |= FACE_FLAG_TINT;}
				}

				if (hilited_face.Face < 0)
				{
					//
					// Set the envinronment map and tint flags.
					// 

					prim_faces3[-hilited_face.Face].FaceFlags &= ~FACE_FLAG_ENVMAP;
					prim_faces3[-hilited_face.Face].FaceFlags &= ~FACE_FLAG_TINT;

					if (face_popup[10].ItemFlags & MENU_CHECK_MASK) {prim_faces3[-hilited_face.Face].FaceFlags |= FACE_FLAG_ENVMAP;}
					if (face_popup[11].ItemFlags & MENU_CHECK_MASK) {prim_faces3[-hilited_face.Face].FaceFlags |= FACE_FLAG_TINT;}
				}

				if(hilited_face.PEle==(struct EditMapElement*)-2)
				{
					
				}
				else
				if(hilited_face.PEle==(struct EditMapElement*)-1)
				{
					if(next_face_selected>1&&face_is_in_list(hilited_face.Face))
					{
						SLONG	c0;
						if((flags&POLY_FLAG_TILED)&&!(old_flags&POLY_FLAG_TILED) )
						{
							//tiled mode just selecteD
							fix_all_selected_faces_for_tile_mode();
						}

						for(c0=1;c0<next_face_selected;c0++)
						{
							if (face_selected_list[c0]<0)
							{
								prim_faces3[-face_selected_list[c0]].DrawFlags=flags;

								//
								// Set the envinronment map and tint flags.
								// 

								prim_faces3[-face_selected_list[c0]].FaceFlags &= ~FACE_FLAG_ENVMAP;
								prim_faces3[-face_selected_list[c0]].FaceFlags &= ~FACE_FLAG_TINT;

								if (face_popup[10].ItemFlags & MENU_CHECK_MASK) {prim_faces3[-face_selected_list[c0]].FaceFlags |= FACE_FLAG_ENVMAP;}
								if (face_popup[11].ItemFlags & MENU_CHECK_MASK) {prim_faces3[-face_selected_list[c0]].FaceFlags |= FACE_FLAG_TINT;}
							}
							else
							{
								prim_faces4[face_selected_list[c0]].DrawFlags=flags;

								//
								// Set the envinronment map and tint flags.
								// 

								prim_faces4[face_selected_list[c0]].FaceFlags &= ~FACE_FLAG_ENVMAP;
								prim_faces4[face_selected_list[c0]].FaceFlags &= ~FACE_FLAG_TINT;

								if (face_popup[10].ItemFlags & MENU_CHECK_MASK) {prim_faces4[face_selected_list[c0]].FaceFlags |= FACE_FLAG_ENVMAP;}
								if (face_popup[11].ItemFlags & MENU_CHECK_MASK) {prim_faces4[face_selected_list[c0]].FaceFlags |= FACE_FLAG_TINT;}
							}
						}
						
					}
					else
					{
						if(hilited_face.Face<0)
						{
							prim_faces3[-hilited_face.Face].DrawFlags = flags;
						}
						else
						{
							prim_faces4[hilited_face.Face].DrawFlags = flags;
						}
					}
				}
				else
				if(hilited_face.PEle)
				{
					hilited_face.PEle->Textures[hilited_face.Face].DrawFlags=flags;
				}
				break;
			case	TAB_PRIMPICK:
				break;
		}
	}
	if(the_control)
	{
		delete	the_control;
	}
}

UBYTE	LevelEditor::DoStylePopup(MFPoint *clicked_point,UBYTE flags)
{
	ULONG			c0,
					control_id;
	CPopUp			*the_control	=	0;
	MFPoint			local_point;
	UBYTE			old_flags;

	local_point	=	*clicked_point;
	GlobalToLocal(&local_point);
	popup_def.ControlLeft	=	local_point.X+4;
	popup_def.ControlTop	=	local_point.Y-4;	 


	if(CurrentModeTab())
	{
				old_flags=flags;
				face_popup[7].ItemFlags	=	0;
				for(c0=0;c0<7;c0++)
				{
					face_popup[c0].ItemFlags	=	0;
					if(flags&(1<<c0))
						face_popup[c0].ItemFlags	|=	MENU_CHECK_MASK;
				}
				popup_def.TheMenuDef	=	face_popup;
				the_control		=	new CPopUp(&popup_def);
				control_id		=	the_control->TrackControl(&local_point);
				flags			=	0;

				for(c0=0;c0<7;c0++)
				{
					if(face_popup[c0].ItemFlags&MENU_CHECK_MASK)
						flags	|=	(1<<c0);
				}

	}
	if(the_control)
	{
		delete	the_control;
	}
	return(flags);
}






void	set_wall_texture_info(SLONG	wall,UBYTE page,EdTexture	*current_texture,UBYTE type,UBYTE side)
{
	SLONG	x,y,index;
	UBYTE	sub_type;
	SLONG	storey,wall_index;
	x=	current_texture->U[0]+current_texture->U[1]+current_texture->U[2]+current_texture->U[3];
	y=	current_texture->V[0]+current_texture->V[1]+current_texture->V[2]+current_texture->V[3];

	x>>=(2+5);
	y>>=(2+5);
/*
	if(PaintMode->GetPaintMode()==STYLE_PAINT)
	{
		type=PaintMode->CurrentStyleEdit;
	}
	else
*/

//	 http://smfeng.yeah.net
	/*
	if(type==0)
	{
		index=page*64+x+y*8;

		type=texture_info[index].Type;
		
		sub_type=texture_info[index].SubType;
	}
	*/

	LogText("NUKE set wall %d  walls storey %d style %d \n",wall,wall_list[wall].StoreyHead,type);


	if(side==0)
		wall_list[wall].TextureStyle=type;
	else
		wall_list[wall].TextureStyle2=type;
	if(!ControlFlag)
	{
		storey=wall_list[wall].StoreyHead;
		wall_index=storey_list[storey].WallHead;
		while(wall_index)
		{
			LogText(" set wall texture index %d \n",wall_index);
			if(side==0)
				wall_list[wall_index].TextureStyle=type;
			else
				wall_list[wall_index].TextureStyle2=type;

			wall_index=wall_list[wall_index].Next;
		}
	}
}

//---------------------------------------------------------------

void	LevelEditor::TextureFace(SWORD face,PaintTab *texture_mode)
{
	SLONG 		c0;
	EdTexture	*current_texture;



	if(texture_mode->GetPaintMode()==PALETTE_PAINT)
	{
		if(face<0)
		{	
			if	(
					!(prim_faces3[-face].DrawFlags&POLY_FLAG_TEXTURED) &&
					prim_faces3[-face].Col2==texture_mode->GetCurrentColour()
				)
				return;
		}
		else
		{
			if	(
					!(prim_faces4[face].DrawFlags&POLY_FLAG_TEXTURED)	&&
					prim_faces4[face].Col2==texture_mode->GetCurrentColour()
				)
				return;
		}
	}
	else
	{
		current_texture	=	texture_mode->GetTexture();

		if(face<0)
		{
			if	(
					prim_faces3[-face].DrawFlags&POLY_FLAG_TEXTURED							&&
					prim_faces3[-face].TexturePage==(SBYTE)texture_mode->GetTexturePage()	&&
					prim_faces3[-face].UV[0][0]==current_texture->U[0]		&&
					prim_faces3[-face].UV[0][1]==current_texture->V[0]		&&
					prim_faces3[-face].UV[1][0]==current_texture->U[1]		&&
					prim_faces3[-face].UV[1][1]==current_texture->V[1]		&&
					prim_faces3[-face].UV[2][0]==current_texture->U[2]		&&
					prim_faces3[-face].UV[2][1]==current_texture->V[2]
				)
				return;
		}
		else
		{

			if	(
					prim_faces4[face].DrawFlags&POLY_FLAG_TEXTURED		&&
					prim_faces4[face].TexturePage==(UBYTE)texture_mode->GetTexturePage()	&&
					prim_faces4[face].UV[0][0]==current_texture->U[0]	&&
					prim_faces4[face].UV[0][1]==current_texture->V[0]	&&
					prim_faces4[face].UV[1][0]==current_texture->U[1]	&&
					prim_faces4[face].UV[1][1]==current_texture->V[1]	&&
					prim_faces4[face].UV[2][0]==current_texture->U[2]	&&
					prim_faces4[face].UV[2][1]==current_texture->V[2]	&&
					prim_faces4[face].UV[3][0]==current_texture->U[3]	&&
					prim_faces4[face].UV[3][1]==current_texture->V[3]
					)
				return;
		}
	}

	if(face<0)
	{
		texture_mode->MyUndo.ApplyPrim3(0,-face,&prim_faces3[-face]);
	}
	else
	{
		texture_mode->MyUndo.ApplyPrim4(0,face,&prim_faces4[face]);
	}
	
	if(texture_mode->GetPaintMode()==PALETTE_PAINT)
	{
		if(face<0)
		{
			prim_faces3[-face].DrawFlags	&=	~(POLY_FLAG_TEXTURED);
			prim_faces3[-face].Col2	=	texture_mode->GetCurrentColour();
		}
		else
		{
			prim_faces4[face].DrawFlags		&=	~(POLY_FLAG_TEXTURED);
			prim_faces4[face].Col2	=	texture_mode->GetCurrentColour();
		}
	}
	else
	{
		if(face<0)
		{
			prim_faces3[-face].DrawFlags	|=	POLY_FLAG_TEXTURED;
			prim_faces3[-face].TexturePage	=	(SBYTE)texture_mode->GetTexturePage();
			for(c0=0;c0<3;c0++)
			{
				prim_faces3[-face].UV[c0][0]	=	current_texture->U[c0];
				prim_faces3[-face].UV[c0][1]	=	current_texture->V[c0];
			}
		}
		else
		{
			if(texture_mode->GetTexturePage()<0)
			{
				prim_faces4[face].TexturePage=-texture_mode->GetTexturePage();
				prim_faces4[face].FaceFlags|=FACE_FLAG_ANIMATE;
			}
			else
			{
				prim_faces4[face].DrawFlags		|=	POLY_FLAG_TEXTURED;
				prim_faces4[face].TexturePage	=	(SBYTE)texture_mode->GetTexturePage();

				for(c0=0;c0<4;c0++)
				{
					prim_faces4[face].UV[c0][0]	=	current_texture->U[c0];
					prim_faces4[face].UV[c0][1]	=	current_texture->V[c0];
				}
				if(prim_faces4[face].FaceFlags&FACE_FLAG_WALKABLE)
				{
					//
					// try and paint roof texture to floor
					//
					SLONG	mx=0,mz=0;
					for(c0=0;c0<4;c0++)
					{
						mx+=prim_points[prim_faces4[face].Points[c0]].X;
						mz+=prim_points[prim_faces4[face].Points[c0]].Z;
					}
					mx>>=2;
					mz>>=2;

					edit_map[mx>>ELE_SHIFT][mz>>ELE_SHIFT].Texture=PaintMode->ConvertTexToMiniTex();
				}

				if(prim_faces4[face].ThingIndex<0)
				{	// face is part of a buildtab building, so set walls texture info
					UBYTE	type=0;
					if(PaintMode->GetPaintMode()==STYLE_PAINT)
					{
						type=PaintMode->CurrentStyleEdit;
						set_wall_texture_info(-prim_faces4[face].ThingIndex,(SBYTE)texture_mode->GetTexturePage(),current_texture,type,(prim_faces4[face].FaceFlags&FACE_FLAG_TEX2)?1:0);
					}
					else
					{
						//
						// apply an individual texture to a wall face
						//
extern	void	apply_texture_to_wall_face(SLONG face,SLONG texture);
						SLONG	t;
						SLONG	x=0,y=0,c0;

						for(c0=0;c0<4;c0++)
						{
							x+=current_texture->U[c0];
							y+=current_texture->V[c0];
						}
						x>>=2;
						y>>=2;

						x>>=5;
						y>>=5;

						t=x+y*8+texture_mode->GetTexturePage()*64;
						if(t<128)
						{
							if(current_texture->U[0]>current_texture->U[1])
								t|=0x80;
							apply_texture_to_wall_face(face,t);
						}

					}

				}
			}
		}
	}
}

void	calc_face_midpoint(SWORD face,SLONG *x,SLONG *y,SLONG *z)
{
	SLONG	x1=0,y1=0,z1=0,point;
	if(face>0)
	{
		for(point=0;point<4;point++)
		{
			x1+=prim_points[prim_faces4[face].Points[point]].X;
			y1+=prim_points[prim_faces4[face].Points[point]].Y;
			z1+=prim_points[prim_faces4[face].Points[point]].Z;
		}
	}
	*x=x1>>2;
	*y=y1>>2;
	*z=z1>>2;

}

SLONG	find_map_coord(SLONG *x,SLONG *y,SLONG *z,struct	EditMapElement	*p_ele)
{
/*
	SLONG index;
	struct	EditMapElement	*p_ele2;
	SLONG mx,my,mz,dx,dy,dz;

	mx=(engine.X>>8)>>ELE_SHIFT;
	my=(engine.Y>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;

	for(dx=-12;dx<12;dx++)
	for(dy=-12;dy<12;dy++)
	for(dz=-12;dz<12;dz++)
	{
			index=edit_map[(dx+mx)][(dy+my)].Depth[(dz+mz)];
//			draw_map_thing(index);
			if(index)
			{
				p_ele2=&edit_map_eles[index];
				if(p_ele2==p_ele)
				{
					*x=mx+dx;
					*y=my+dy;
					*z=mz+dz;
					return(1);
				}
			}
	}
*/
	return(0);
}

SLONG	static_face_no;
SLONG	flood_fill_texture(SLONG x,SLONG y,SLONG z,ULONG tex_bits)
{
/*
	struct	EditMapElement	*PEle;
	SLONG	tx,ty,tz;
	SLONG	index;
	struct	TextureBits	tex_bits2;
	*(ULONG*)&tex_bits2=tex_bits;

	tx=x;
	ty=y;
	tz=z;

	index=edit_map[(x)][(y)].Depth[(z)];
	if(index)
	{
		PEle=&edit_map_eles[index];
		PEle->Textures[static_face_no]=tex_bits2;
		switch(static_face_no)
		{
			case	CUBE_INDEX_FRONT:
			case	CUBE_INDEX_BACK:
				if(index=edit_map[(x+1)][(y)].Depth[(z)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x+1,y,z,tex_bits);
				}
				if(index=edit_map[(x-1)][(y)].Depth[(z)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x-1,y,z,tex_bits);
				}
				if(index=edit_map[(x)][(y-1)].Depth[(z)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x,y-1,z,tex_bits);
				}
				if(index=edit_map[(x)][(y+1)].Depth[(z)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x,y+1,z,tex_bits);
				}

				break;
			case	CUBE_INDEX_TOP:
			case	CUBE_INDEX_BOTTOM:
				if(index=edit_map[(x+1)][(y)].Depth[(z)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x+1,y,z,tex_bits);
				}
				if(index=edit_map[(x-1)][(y)].Depth[(z)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x-1,y,z,tex_bits);
				}
				if(index=edit_map[(x)][(y)].Depth[(z-1)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x,y,z-1,tex_bits);
				}
				if(index=edit_map[(x)][(y)].Depth[(z+1)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x,y,z+1,tex_bits);
				}
				break;

			case	CUBE_INDEX_LEFT:
			case	CUBE_INDEX_RIGHT:	
				if(index=edit_map[(x)][(y)].Depth[(z-1)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x,y,z-1,tex_bits);
				}
				if(index=edit_map[(x)][(y)].Depth[(z+1)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x,y,z+1,tex_bits);
				}
				if(index=edit_map[(x)][(y-1)].Depth[(z)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x,y-1,z,tex_bits);
				}
				if(index=edit_map[(x)][(y+1)].Depth[(z)])
				{
					
					PEle=&edit_map_eles[index];
					if(*(ULONG*)&PEle->Textures[static_face_no]!=tex_bits)
						flood_fill_texture(x,y+1,z,tex_bits);
				}
				break;
		}
	}
*/
	return(0);
}


extern	UWORD	make_poly_into_glass_shatter_prim(SWORD face,SWORD mid_x,SWORD mid_y,SWORD mid_z);
BOOL	LevelEditor::ApplyTexture(struct EditFace *edit_face)
{
	SLONG			c0;

	
	if(edit_face->PEle==(struct EditMapElement*)-2)
	{
		if(PaintMode==PALETTE_PAINT)
		{
			
		}
		else
		{
			LogText(" paint texture to floor at %d %d \n",edit_face->MapX,edit_face->MapZ);
			//
			// apply texture to floor
			//

			if(PaintMode->GetTexturePage()<0)
			{
				edit_map[edit_face->MapX][edit_face->MapZ].Texture=-PaintMode->GetTexturePage(); //-CurrentTexturePage; //PaintMode->ConvertTexToMiniTex();
				edit_map[edit_face->MapX][edit_face->MapZ].Walkable=-1; //CurrentTexturePage; //PaintMode->ConvertTexToMiniTex();

			}
			else
			{
				if(edit_info.RoofTex)
				{
					tex_map[edit_face->MapX][edit_face->MapZ]=PaintMode->ConvertTexToMiniTex();
				}
				else
				{
					edit_map[edit_face->MapX][edit_face->MapZ].Texture=PaintMode->ConvertTexToMiniTex();
				}
			}
		}

		
	}
	else
	if(edit_face->PEle==(struct EditMapElement*)-1)
	{
		SLONG	mid_x,mid_y,mid_z;

//		calc_face_midpoint(edit_face->Face,&mid_x,&mid_y,&mid_z);
//		make_poly_into_glass_shatter_prim(edit_face->Face,mid_x,mid_y,mid_z);

		if(face_is_in_list(edit_face->Face))
		{
			for(c0=1;c0<next_face_selected;c0++)
			{
				TextureFace(face_selected_list[c0],PaintMode);
			}
		}
		else
		{
			TextureFace(edit_face->Face,PaintMode);
//			next_face_selected=1;
		}
		return	1;
	}
	else if(edit_face->PEle)
	{
		SLONG x,y,width,height,page;
		SLONG mx,my,mz;
		ULONG	texbits;
		PaintMode->ConvertFreeToFixedEle(&edit_face->PEle->Textures[edit_face->Face],&x,&y,&width,&height,&page);
		if(ShiftFlag)
		{
			
			texbits=(*(ULONG*)&edit_face->PEle->Textures[edit_face->Face]);

			find_map_coord(&mx,&my,&mz,edit_face->PEle);
			static_face_no=edit_face->Face;
			flood_fill_texture(mx,my,mz,texbits);
		}
		return	1;
	}
	return	0;
}

//---------------------------------------------------------------
