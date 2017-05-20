// Guy Simmons
// 20th February 1997.

//eaworldhq@eahq-exchange.ea.com


#include	"Editor.hpp"
#include	"c:\fallen\headers\game.h"
#include	"c:\fallen\headers\animtmap.h"
#include	"c:\fallen\ddlibrary\headers\tga.h"
#include	"c:\fallen\headers\memory.h"
#define	SET_TEXTURE_COORDS			texture_x=TextureX;if(texture_x>(256-(1<<TextureZoom)))texture_x=(256-(1<<TextureZoom));	\
									texture_y=TextureY;if(texture_y>(256-(1<<TextureZoom)))texture_y=(256-(1<<TextureZoom));

#define	PAINT2
//---------------------------------------------------------------
void	scale_selected_tmaps(SWORD scale);
void	remove_style_textures(void);
void	remove_painted_textures(void);

UWORD	page_remap[64*8];
SLONG	show_info=0;

CBYTE	texture_style_names[200][21]=
{
	{"                    "},
	{"BROWN_BRICK1"},
	{"BROWN_BRICK2"},
	{"GREY_RIM1"},
	{"GREY_RIM2"},
	{"RED_WINDOW"},
	{"GREY_CORIGATED"},
	{"CRATES_SMALL_BROWN"},
	{"GREY_POSH"},
	{"HOTEL_SIGN1"},
	{"HOTEL_SIGN2"},
	{"                    "},
	{"                    "},
	{"                    "},
	{"                    "},
	{"                    "},
	{"                    "},
	{"                    "}
};

MenuDef2		texture_menu[]	=
{
	{(CBYTE*)"World Page 1"},
	{(CBYTE*)"World Page 2"},
	{(CBYTE*)"World Page 3"},
	{(CBYTE*)"World Page 4"},
	{(CBYTE*)"Shared Page 1"},
	{(CBYTE*)"Shared Page 2"},
	{(CBYTE*)"Shared Page 3"},
	{(CBYTE*)"Shared Page 4"},
	{(CBYTE*)"Inside Page 1"},
	{(CBYTE*)"People Page 1"},
	{(CBYTE*)"People Page 2"},
	{(CBYTE*)"Prims Page 1"},
	{(CBYTE*)"Prims Page 2"},
	{(CBYTE*)"Prims Page 3"},
	{(CBYTE*)"other14"},
	{(CBYTE*)"other15"},
	{(CBYTE*)"other16"},
	{(CBYTE*)"other17"},
	{(CBYTE*)"other18"},
	{(CBYTE*)"other19"},
	{(CBYTE*)"other20"},
	{(CBYTE*)"other21"},
	{(CBYTE*)"other9"},
	{(CBYTE*)"other1"},
	{(CBYTE*)"other1"},
	{(CBYTE*)"other1"},
	{(CBYTE*)"other1"},
	{(CBYTE*)"other1"},
	{"!"}
};

MenuDef2		texture_size[]	=
{
	{"8"},{"16"},{"32"},{"64"},{"96"},{"128"},{"160"},{"192"},{"!"}
};

MenuDef2		texture_size2[]	=
{
	{"8"},{"16"},{"32"},{"64"},{"96"},{"128"},{"160"},{"192"},{"!"}
};

MenuDef2		mode_menu[]	=
{
	{"Palette"},{"Textures"},
	{"planar"},{"ANIMS"},{"PLAN VIEW"},{"Style Paint"},{"Style Def"},{"Inside Style Def"},{"PSX Alt"},{"!"}
};

UBYTE	texture_sizes[]={8,16,32,64,96,128,160,192};

//---------------------------------------------------------------

//#define	CTRL_PAINT_MENU		1
//#define	CTRL_PAINT_TEXT		2
/*
ControlDef	paint_tab_def[]	=	
{

	{	0															   				}
};
*/

//---------------------------------------------------------------

ControlDef	colour_def[]	=
{
	{	0															   			}
};

#define	CTRL_STYLE_PAINT_SLIDER		1
ControlDef	style_def[]	=
{
	{	V_SLIDER,		0,	"",						1,30,		50,		300			},
	{	0															   			}
};

#define	CTRL_TEX_PAGE_TEXT	1
#define	CTRL_TEX_PAGE_MENU	2
#define	CTRL_TEX_QUAD_BOX	3
#define	CTRL_TEX_FIXED_BOX	4
//#define	CTRL_TEX_SIZE_MENUW	5
//#define	CTRL_TEX_SIZE_MENUH	6
#define	CTRL_TEX_SIZE_TEXT	5
#define	CTRL_TEX_IMPORT_TEX	6
#define	CTRL_TEX_TILE_SIZE	7
#define	CTRL_TEX_LOAD_ANIMTMAP	8
#define	CTRL_TEX_SAVE_ANIMTMAP	9
#define	CTRL_TEX_ANIMTMAP_UP	10
#define	CTRL_TEX_ANIMTMAP_DOWN	11
#define	CTRL_TEX_SELECT_DRAWN	12
#define	CTRL_TEX_PLANAR_MAP		13
#define	CTRL_TEX_NO_HIDDEN		14
#define	CTRL_TEX_PLANAR_MAPF	15
#define	CTRL_TEX_USE_CLIPPED	16
#define	CTRL_TEX_SET_CLIPPED	17
#define	CTRL_TEX_ROOF_TEX		18
#define	CTRL_PAINT_MENU		19
#define	CTRL_PAINT_TEXT		20
#define	CTRL_TEX_HIDE_L		21
#define	CTRL_TEX_HIDE_R		22
#define	CTRL_TEX_HIDE_ROOF		23
#define	CTRL_BUILD_OTHER		24
#define	CTRL_TEX_INFO			25
#define	CTRL_TEX_CLEAR			26


ControlDef	texture_def[]	=
{
	{	STATIC_TEXT,	0,	"Current Page : ",	2,	334,	0,	10					},
	{	PULLDOWN_MENU,	0,	"Texture Page",		140,	24,	56,	10,	texture_menu	},

	{	CHECK_BOX,		0,	"Quads",			2,	370,	0,	10					},
	{	CHECK_BOX,		KB_F,"Fixed Textures",	2,	384,	0,	10					},

//	{	PULLDOWN_MENU,	0,	"Texture Width",	150,340,	100,	10,	texture_size	},
//	{	PULLDOWN_MENU,	0,	"Texture Height",	150,354,	100,	10,	texture_size2	},
	{	STATIC_TEXT,	0,	"",					100,384,	0,	10					},
	{	BUTTON,			0,	"Import Textures",	2,	410,	0,	0					},
	{	H_SLIDER,		0,	"",					2,	430,    190,	0			},
	{	BUTTON,			0,	"Load AnimTmaps",	100,	410,	0,	0					},
	{	BUTTON,			0,	"Save AnimTmaps",	200,	410,	0,	0					},
	{	BUTTON,			0,	"UP",	273,	20,	0,	0					},
	{	BUTTON,			0,	"DWN",	273,	294,	0,	0					},
	{	CHECK_BOX,		0,	"Select Drawn",			200,	350,	0,	10					},
	{	CHECK_BOX,			0,	"Map1",			200,	330,	0,	10					},
	{	CHECK_BOX,		0,	"NoHidden",			250,	390,	0,	10					},
	{	CHECK_BOX,		0,	"MapF",			250,	330,	0,	10					},
	{	CHECK_BOX,		0,	"Use Clipping",	240,	450,	0,	10					},
	{	BUTTON,			0,	"Set Clipping",	240,	465,	0,	0					},
	{	CHECK_BOX,		0,	"Roof Textures",	40,	470,	0,	0					},
	{	PULLDOWN_MENU,	0,	"Paint mode",		2,	10,		0,	0,	mode_menu		},
	{	STATIC_TEXT,	0,	"Current mode : ",	14,	24,		0,	10					},
	{	CHECK_BOX,			0,	"Hide Left",	20,	325,	0,	0					},
	{	CHECK_BOX,			0,	"Hide Right",	80,325,	0,	0					},
	{	CHECK_BOX,			0,	"Hide Roofs",	140,325,	0,	0					},
	{	BUTTON,			0,	"BuildOther",	10,	350,	0,	0					},
	{	BUTTON,			0,	"Show Tex Info",	100,	350,	0,	0					},
	{	BUTTON,			0,	"ClearTex",	150,	470,	0,	0					},


	{	0															   			}
};


extern	void	sync_animtmaps(void);





void	new_tile_size(void);
PaintTab	*the_painttab;	
//---------------------------------------------------------------

SLONG	max_textures=NUM_GAME_TEXTURES - 1;

PaintTab::PaintTab(EditorModule *parent)
{
	SLONG			c0;


	CurrentColour=0;
	TextureFlags=0;
	CurrentAnimTmap=0;

	Parent	=	parent;

	/*(

	// Set the texture menu up for valid textures only.
//	for fucks sake

	for(c0=0;c0<14;c0++)
	{		
		if(!game_textures[c0].TextureName)
			break;
	}

	*/

	max_textures=NUM_GAME_TEXTURES - 1;


//	texture_menu[c0].ItemText[0]	=	'!';
//	texture_menu[c0].ItemText[1]	=	0;

//	InitControlSet(paint_tab_def);
	//InitControlSet(style_tab_def); //default control set used by them all
	CurrentTexturePage	=	0;
	CurrentTextureRot	=	0;

	PaintMode			=	TEXTURE_PAINT;
	load_animtmaps();
	ShowAnimTmap=1;

	PaletteSet.InitControlSet(colour_def);
	StyleSet.InitControlSet(style_def);
	InitControlSet(texture_def);
//	TextureSet.InitControlSet(texture_def);

	PaintRect.SetRect(14,40,258,258);
	AnimRect.SetRect(276,45,19,246);
	TextureWidth	=	32;
	TextureHeight	=	32;
	TextureX	=	0;
	TextureY	=	0;
	TextureZoom	=	8;
	SubMode=0;
	SubStatus=0;

	edit_info.TileScale=100;

	TextureFlags=	FLAGS_QUADS|FLAGS_FIXED;

	SetControlState(CTRL_TEX_HIDE_L,CTRL_DESELECTED); 
	SetControlState(CTRL_TEX_HIDE_R,CTRL_DESELECTED); 
	SetControlState(CTRL_TEX_HIDE_ROOF,CTRL_DESELECTED); 

	SetControlState(CTRL_TEX_QUAD_BOX,CTRL_SELECTED); //ts
	SetControlState(CTRL_TEX_FIXED_BOX,CTRL_SELECTED);
	((CHSlider*)GetControlPtr(CTRL_TEX_TILE_SIZE))->SetValueRange(1,256);
	((CHSlider*)GetControlPtr(CTRL_TEX_TILE_SIZE))->SetCurrentValue(edit_info.TileScale);
	((CHSlider*)GetControlPtr(CTRL_TEX_TILE_SIZE))->SetUpdateFunction(new_tile_size);

void	new_scroll_pos(void);
	((CVSlider*)StyleSet.GetControlPtr(CTRL_STYLE_PAINT_SLIDER))->SetUpdateFunction(new_scroll_pos);
	((CVSlider*)StyleSet.GetControlPtr(CTRL_STYLE_PAINT_SLIDER))->SetValueRange(0,60);
	((CVSlider*)StyleSet.GetControlPtr(CTRL_STYLE_PAINT_SLIDER))->SetCurrentValue(1);

	CurrentStyleEdit=0;
	CurrentStylePos=1;
	the_painttab=this;
	UpdateTabInfo();
	UpdatePaletteInfo();
	UpdateTextureInfo();

	for(c0=0;c0<4;c0++)
	{
		CurrentTexture.U[c0]=0;
		CurrentTexture.V[c0]=0;
	}
}

PaintTab::~PaintTab()
{
	CutMapBlock.Free();
}

void	fix_all_selected_faces_for_tile_mode(void);
extern	void	find_highest_selected_tmap(SLONG *tx,SLONG *ty);
/*
void	scale_selected_tmaps(SLONG scale)
{
	SLONG	c0,c1;
	struct	PrimFace4	*p_f4;
	struct	PrimFace3	*p_f3;
	SLONG tx,ty;

	find_highest_selected_tmap(&tx,&ty);
	scale+=32;


	for(c0=1;c0<next_face_selected;c0++)
	{
		if(face_selected_list[c0]>0)
		{
			p_f4=&prim_faces4[face_selected_list[c0]];
			for(c1=0;c1<4;c1++)
			{
				SLONG wx,wy;

			   	wx=p_f4->UV[c1][0];
			   	wy=p_f4->UV[c1][1];
			   	wx=(((wx-tx)*scale)>>5)+tx;
			   	wy=(((wy-ty)*scale)>>5)+ty;

			   	p_f4->UV[c1][0]=wx;
			   	p_f4->UV[c1][1]=wy;
			}
		}
		else
		if(face_selected_list[c0]<0)
		{
			p_f3=&prim_faces3[-face_selected_list[c0]];
			for(c1=0;c1<3;c1++)
			{
			   	p_f3->UV[c1][0]=(((p_f3->UV[c1][0]-tx)*scale)>>7)+p_f4->UV[c1][0];
			   	p_f3->UV[c1][1]=(((p_f3->UV[c1][1]-ty)*scale)>>7)+p_f3->UV[c1][1];
			}

		}
	}
}
*/
void	new_tile_size(void)
{
	edit_info.TileScale	=	((CHSlider*)the_painttab->GetControlPtr(CTRL_TEX_TILE_SIZE))->GetCurrentValue();
	if(edit_info.TileFlag)
	{
//		fix_all_selected_faces_for_tile_mode();
	}
	else
	{
	//		scale_selected_tmaps(tx,ty,edit_info.TileScale);
	}
//	the_painttab->UpdateTexture();
	the_painttab->Parent->DrawContent();
//   	the_painttab->DrawTabContent();
/*
	SetWorkWindowBounds(the_painttab->Parent->GetLeft(),
						the_painttab->Parent->GetTop(),
						the_painttab->Parent->GetWidth(),
						the_painttab->Parent->GetHeight());
*/
}

void	new_scroll_pos(void)
{
	the_painttab->CurrentStylePos	=	((CVSlider*)the_painttab->StyleSet.GetControlPtr(CTRL_STYLE_PAINT_SLIDER))->GetCurrentValue();
	the_painttab->RequestUpdate();

}
//---------------------------------------------------------------

void	PaintTab::UpdateTabInfo(void)
{
	((CStaticText*)GetControlPtr(CTRL_PAINT_TEXT))->SetString1(mode_menu[PaintMode].ItemText);
}

//---------------------------------------------------------------
extern	SLONG	editor_texture_set;

void	PaintTab::DrawTabContent(void)
{
	EdRect		bounds_rect,
				content_rect;


	content_rect	=	ContentRect;	
	content_rect.ShrinkRect(1,1);
	content_rect.FillRect(CONTENT_COL);
	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);

	DrawControlSet();
	extern	UWORD	diff_page_count1,diff_page_count2;
	{
		CBYTE	str[100];
		sprintf(str,"diff tex(world %d) %d  shared %d",editor_texture_set,diff_page_count1,diff_page_count2); 
		QuickText(0,0,str,0);
		QuickText(1,0,str,WHITE_COL);
	}


	switch(PaintMode)
	{
		case	PALETTE_PAINT:
			UpdatePaletteInfo();
			DrawPalette();
//			DrawControlSet();
//			PaintRect.HiliteRect(LOLITE_COL,HILITE_COL);
			PaintRect.HiliteRect(LOLITE_COL,HILITE_COL);
			break;
		case	INSTYLE_DEFINE:
		case	STYLE_DEFINE:
		case	PSX_TEX_DEFINE:
		case	ANIM_TMAP_PAINT:
		case	FLOOR_PAINT:
		case	TEXTURE_PAINT:
		case	PLANAR_PAINT:
			UpdateTextureInfo();
			DrawTexture();
//			bounds_rect.SetRect(ContentLeft(),ContentTop(),ContentWidth(),ContentHeight());
//			TextureSet.ControlSetBounds(&bounds_rect);
//			TextureSet.DrawControlSet();
//			PaintRect.HiliteRect(LOLITE_COL,HILITE_COL);
			break;
		case	STYLE_PAINT:
			DrawStyleTexture();
			bounds_rect.SetRect(ContentLeft(),ContentTop(),ContentWidth(),ContentHeight());
			StyleSet.ControlSetBounds(&bounds_rect);
			StyleSet.DrawControlSet();
			break;
		default:
			break;

	}

	//
	// Default control set used by all
	//
}

//---------------------------------------------------------------

void	draw_selected_face_textures(SLONG tx,SLONG ty,SLONG zoom)
{
	SLONG	c0;
	struct	PrimFace4	*p_f4;
	struct	PrimFace3	*p_f3;

//	zoom=1;

	for(c0=1;c0<next_face_selected;c0++)
	{
		if(face_selected_list[c0]>0)
		{
			SLONG	x1,y1,x2,y2,x3,y3,x4,y4;
			p_f4=&prim_faces4[face_selected_list[c0]];
			x1=((p_f4->UV[0][0]+tx)*zoom);
			y1=((p_f4->UV[0][1]+ty)*zoom);
			x2=((p_f4->UV[1][0]+tx)*zoom);
			y2=((p_f4->UV[1][1]+ty)*zoom);
			x3=((p_f4->UV[2][0]+tx)*zoom);
			y3=((p_f4->UV[2][1]+ty)*zoom);
			x4=((p_f4->UV[3][0]+tx)*zoom);
			y4=((p_f4->UV[3][1]+ty)*zoom);

			DrawLineC(x1,y1,x2,y2,WHITE_COL);
			DrawLineC(x2,y2,x4,y4,WHITE_COL);
			DrawLineC(x4,y4,x3,y3,WHITE_COL);
			DrawLineC(x3,y3,x1,y1,WHITE_COL);
		}
		else
		if(face_selected_list[c0]<0)
		{
			SLONG	x1,y1,x2,y2,x3,y3;
			p_f3=&prim_faces3[-face_selected_list[c0]];
			x1=(p_f3->UV[0][0]+tx)*zoom;
			y1=(p_f3->UV[0][1]+ty)*zoom;
			x2=(p_f3->UV[1][0]+tx)*zoom;
			y2=(p_f3->UV[1][1]+ty)*zoom;
			x3=(p_f3->UV[2][0]+tx)*zoom;
			y3=(p_f3->UV[2][1]+ty)*zoom;

			DrawLineC(x1,y1,x2,y2,WHITE_COL);
			DrawLineC(x2,y2,x3,y3,WHITE_COL);
			DrawLineC(x3,y3,x1,y1,WHITE_COL);
		}															 
	}
}

extern	void	build_texture(SLONG x,SLONG y,SLONG w,SLONG h,UBYTE page,UBYTE u0,UBYTE v0,UBYTE u1,UBYTE v1,UBYTE u2,UBYTE v2,UBYTE u3,UBYTE v3);

void	PaintTab::DrawAnimTmapSelector(void)
{
	SLONG	w,h,count;
	EdRect	tex_rect;
	w=AnimRect.GetLeft();
	count=ShowAnimTmap;

	for(h=AnimRect.GetTop();(h<AnimRect.GetBottom())&&count<MAX_ANIM_TMAPS;h+=19)
	{

		if(CurrentAnimTmap==count)
		{
			tex_rect.SetRect(w-1,h-1,20,20);
			tex_rect.OutlineRect(WHITE_COL);
//			tex_rect.HiliteRect(HILITE_COL,LOLITE_COL);
		}

		tex_rect.SetRect(w,h,18,18);
//		tex_rect.OutlineRect(WHITE_COL);
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
	render_view(0);
}

void	PaintTab::SelectStyle(MFPoint *clicked_point)
{
	MFPoint		local_point;
	SLONG	scroll_pos=CurrentStylePos;
	SLONG	c0,pos;
	EdRect	tex_rect;
	SLONG	tx=20,ty=20;
	SLONG	c1;
	UBYTE	UV[4][2];

	local_point=*clicked_point;
	GlobalToLocal(&local_point);
	for(c0=0;c0<10;c0++)
	{
		tex_rect.SetRect(tx+25,ty+c0*26+30,230,24);
//		LogText("MCD point x %d t %d , box x,y,w,h (%d,%d,%d,%d) \n",local_point.X,local_point.Y,tx+25,ty+c0*26+30,110,24);
		if(tex_rect.PointInRect(&local_point))
		{
			LogText(" hit one \n");
			//want to text edit the name
			CurrentStyleEdit=c0+scroll_pos;
			RequestUpdate();

			tx=textures_xy[CurrentStyleEdit][0].Tx<<5;
			ty=textures_xy[CurrentStyleEdit][0].Ty<<5;
			CurrentTexturePage=textures_xy[CurrentStyleEdit][0].Page;

			switch(textures_xy[CurrentStyleEdit][0].Flip)
			{
				case	0:
					UV[0][0]=tx;
					UV[0][1]=ty;
					UV[1][0]=tx+31;
					UV[1][1]=ty;
					UV[2][0]=tx;
					UV[2][1]=ty+31;
					UV[3][0]=tx+31;
					UV[3][1]=ty+31;
					break;
				case	1: //flip x
					UV[0][0]=tx+31;
					UV[0][1]=ty;
					UV[1][0]=tx;
					UV[1][1]=ty;
					UV[2][0]=tx+31;
					UV[2][1]=ty+31;
					UV[3][0]=tx;
					UV[3][1]=ty+31;
					break;
				case	2: //flip y
					UV[0][0]=tx;
					UV[0][1]=ty+31;
					UV[1][0]=tx+31;
					UV[1][1]=ty+31;
					UV[2][0]=tx;
					UV[2][1]=ty;
					UV[3][0]=tx+31;
					UV[3][1]=ty;
					break;
				case	3: //flip x+y
					UV[0][0]=tx+31;
					UV[0][1]=ty+31;
					UV[1][0]=tx;
					UV[1][1]=ty+31;
					UV[2][0]=tx+31;
					UV[2][1]=ty;
					UV[3][0]=tx;
					UV[3][1]=ty;
					break;
			}

			for(c1=0;c1<4;c1++)
			{
				 CurrentTexture.U[c1]=UV[c1][0];
				 CurrentTexture.V[c1]=UV[c1][1];
			}

			return;
		}
	}

}

extern	UBYTE	tmap2[];
extern	void	draw_quad_now(SLONG x,SLONG y,SLONG w,SLONG h,UBYTE tx,UBYTE ty,UBYTE page,UBYTE flip,UBYTE flags);

void	PaintTab::DrawStyleTexture(void)
{
	SLONG	scroll_pos=CurrentStylePos;
	SLONG	c0,pos;
	EdRect	tex_rect;

	SLONG	tx=20,ty=20;
	

	for(c0=0;c0<10;c0++)
	{
		if(CurrentStyleEdit==c0+scroll_pos)
		{
			tex_rect.SetRect(tx+25,ty+c0*26+30,110,24);
			tex_rect.OutlineRect(0);
		}
		QuickTextC(tx+25,ty+c0*26+36,&texture_style_names[c0+scroll_pos][0],0);

		draw_quad_now(tx+140,ty+c0*26+30-2,24,24,textures_xy[c0+scroll_pos][0].Tx<<5,textures_xy[c0+scroll_pos][0].Ty<<5,textures_xy[c0+scroll_pos][0].Page,textures_xy[c0+scroll_pos][0].Flip,textures_flags[c0+scroll_pos][0]);
		draw_quad_now(tx+140+28,ty+c0*26+30-2,24,24,textures_xy[c0+scroll_pos][1].Tx<<5,textures_xy[c0+scroll_pos][1].Ty<<5,textures_xy[c0+scroll_pos][1].Page,textures_xy[c0+scroll_pos][1].Flip,textures_flags[c0+scroll_pos][1]);
		draw_quad_now(tx+140+56,ty+c0*26+30-2,24,24,textures_xy[c0+scroll_pos][2].Tx<<5,textures_xy[c0+scroll_pos][2].Ty<<5,textures_xy[c0+scroll_pos][2].Page,textures_xy[c0+scroll_pos][2].Flip,textures_flags[c0+scroll_pos][2]);
	}
//	StyleControls.ControlSetBounds(GetContentRect());
//	StyleControls.DrawControlSet();
}
void	PaintTab::DrawTexture(void)
{
	ULONG		pixel;
	SLONG		c0,c1,
				texture_x,
				texture_y,
				zoom;
	SLONG		x[4],y[4];


	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
	DrawAnimTmapSelector();
	if(CurrentTexturePage>=0)
	{

		switch(WorkScreenDepth)
		{
			case	1:
				{
/*
				UBYTE		*texture_ptr;
				UBYTE		*rect_ptr;
				UBYTE		*buffer_ptr;
				UBYTE		texture_buffer[256];

				
				SET_TEXTURE_COORDS

				rect_ptr	=	WorkWindow+PaintRect.GetLeft()+1+((PaintRect.GetTop()+1)*WorkScreenWidth);
				if(Keys[KB_9])
					texture_ptr	=	tmap2+texture_x+(texture_y*256);
				else
					texture_ptr	=	game_textures[CurrentTexturePage].TexturePtr+texture_x+(texture_y*256);
				c0	=	256;

				while(c0)
				{
					buffer_ptr	=	&texture_buffer[255];
					c1			=	1<<TextureZoom;
					while(c1)
					{
						pixel	=	texture_ptr[c1-1];
						zoom	=	256>>TextureZoom;
						while(zoom--)
						{
							*(buffer_ptr--)	=	pixel;
						}
						c1--;
					}

					zoom	=	256>>TextureZoom;
					c0		-=	zoom;
					while(zoom)
					{
						memcpy(rect_ptr,texture_buffer,256);
						rect_ptr	+=	WorkScreenWidth;
						zoom--;
					}
					texture_ptr	+=	256;
				}
*/
				}
				break;

			case	2:
/*
				if (Keys[KB_8])
				{
					UBYTE		*texture_ptr;
					UWORD		*rect_ptr;
					UWORD		*buffer_ptr;
					UWORD		texture_buffer[256];

					
					SET_TEXTURE_COORDS

					rect_ptr	=	((UWORD*)WorkWindow)+PaintRect.GetLeft()+1+((PaintRect.GetTop()+1)*WorkScreenPixelWidth);
					if(Keys[KB_9])
						texture_ptr	=	tmap2+texture_x+(texture_y*256);
					else
						texture_ptr	=	game_textures[CurrentTexturePage].TexturePtr+texture_x+(texture_y*256);
					c0	=	256;

					while(c0)
					{
						buffer_ptr	=	&texture_buffer[255];
						c1			=	1<<TextureZoom;
						while(c1)
						{
	extern	UWORD	pal_to_16[];
	extern	UWORD	yc_to_555[8][256*64];
							pixel	=	yc_to_555[CurrentTexturePage][texture_ptr[c1-1]+(256*32)];
							zoom	=	256>>TextureZoom;
							while(zoom--)
							{
								*(buffer_ptr--)	=	pixel;
							}
							c1--;
						}

						zoom	=	256>>TextureZoom;
						c0		-=	zoom;
						while(zoom)
						{
							memcpy(rect_ptr,texture_buffer,256*2);
							rect_ptr	+=	WorkScreenPixelWidth;
							zoom--;
						}
						texture_ptr	+=	256;
					}
				}
				else
*/
				{
					//
					// Ugh! Whats this!!!
					//

					SET_TEXTURE_COORDS

					UWORD *rect_ptr = ((UWORD*)WorkWindow)+PaintRect.GetLeft()+1+((PaintRect.GetTop()+1)*WorkScreenPixelWidth);
					UWORD *text_ptr = game_textures[CurrentTexturePage].TexturePtr;

					SLONG i;
					SLONG j;

					SLONG zoom   = 256 >> TextureZoom;
					SLONG unzoom = 8 - TextureZoom;

					if (zoom == 1)
					{
						for (i = 0; i < 256; i++)
						{
							memcpy(rect_ptr, text_ptr, 512);

							text_ptr += 256;
							rect_ptr += WorkScreenPixelWidth;
						}
						if(show_info)
						if(CurrentTexturePage<8)
						{
							CBYTE str[100];
							SLONG	x,y,page;
							page=CurrentTexturePage*64;
extern	UWORD	page_count[];
							for(y=0;y<8;y++)
							for(x=0;x<8;x++)
							{
								//sprintf(str,"%d",page_count[page]);
								sprintf(str,"%d(%d)",page_count[page],page);
								QuickText(PaintRect.GetLeft()+x*32,PaintRect.GetTop()+y*32,str,0);
								QuickText(PaintRect.GetLeft()+x*32+1,PaintRect.GetTop()+y*32+1,str,WHITE_COL);
								if(page_remap[page])
								{
									SLONG	u,v,flip=0;
									UWORD	rpage;

									rpage=page_remap[page]-1;

									u=(rpage&7)<<5;
									v=((rpage>>3)&7)<<5;
									if(rpage&(1<<14))
										flip=1;
									if(rpage&(1<<15))
										flip|=2;

									draw_quad_now(PaintRect.GetLeft()+x*32+16,PaintRect.GetTop()+y*32+16,16,16,u,v,((rpage>>6)&7)+25,flip,POLY_T);

								}
								page++;
							}
						}

					}
					else
					{

						for (i = 0; i < 256; i++)
						for (j = 0; j < 256; j++)
						{
							rect_ptr[i + j * WorkScreenPixelWidth] = text_ptr[((i >> unzoom) + texture_x) + ((j >> unzoom) + texture_y) * 256];
						}
						if(show_info)
						if(CurrentTexturePage<8)
						{
							CBYTE str[100];
							SLONG	x,y,page;
							page=CurrentTexturePage*64;
extern	UWORD	page_count[];
							for(y=0;y<8;y++)
							for(x=0;x<8;x++)
							{
								SLONG	zoom_x,zoom_y;

								zoom_x=((x*32)<<unzoom)-(texture_x<<unzoom);
								zoom_y=((y*32)<<unzoom)-(texture_y<<unzoom);

								if(zoom_x>=0&&zoom_x<246&&zoom_y>=0&&zoom_y<246)
								{
									sprintf(str,"%d(%d)",page_count[page],page);
									QuickText(PaintRect.GetLeft()+zoom_x,PaintRect.GetTop()+zoom_y,str,0);
									QuickText(PaintRect.GetLeft()+zoom_x+1,PaintRect.GetTop()+zoom_y+1,str,WHITE_COL);
	
									if(page_remap[page])
									{
										SLONG	u,v,flip=0;
										UWORD	rpage;

										rpage=page_remap[page]-1;

										u=(rpage&7)<<5;
										v=((rpage>>3)&7)<<5;
										if(rpage&(1<<14))
											flip=1;
										if(rpage&(1<<15))
											flip|=2;

										draw_quad_now(PaintRect.GetLeft()+zoom_x+(16<<unzoom),PaintRect.GetTop()+zoom_y+(16<<unzoom),16<<unzoom,16<<unzoom,u,v,((rpage>>6)&7)+25,flip,POLY_T);

									}
	
								}
								page++;
							}
						}

					}
				}

				break;

		}
	//show fade table
	/*
		{
			SLONG	x,y;
			for(y=0;y<64;y++)
			{
				memcpy(rect_ptr,&fade_tables[y<<8],256);
				rect_ptr	+=	WorkScreenWidth;
				memcpy(rect_ptr,&fade_tables[y<<8],256);
				rect_ptr	+=	WorkScreenWidth;
				memcpy(rect_ptr,&fade_tables[y<<8],256);
				rect_ptr	+=	WorkScreenWidth;
				memcpy(rect_ptr,&fade_tables[y<<8],256);
				rect_ptr	+=	WorkScreenWidth;
			}
		}
	*/

		if(TextureFlags&FLAGS_SHOW_TEXTURE)
		{
			SetWorkWindowBounds	(
									ContentLeft()+PaintRect.GetLeft()+2,
									ContentTop()+PaintRect.GetTop()+2,
									256,
									256
								);
			SetWorkWindowBounds	(
									ContentLeft()+PaintRect.GetLeft()+2,
									ContentTop()+PaintRect.GetTop()+2,
									256,
									256
								);
			zoom	=	256>>TextureZoom;

			for(c0=0;c0<4;c0++)
			{
				x[c0]	=	(CurrentTexture.U[c0]-texture_x)*zoom;
				y[c0]	=	(CurrentTexture.V[c0]-texture_y)*zoom;
			}

			DrawLineC(x[0],y[0],x[1],y[1],WHITE_COL);
			DrawLineC(x[2],y[2],x[0],y[0],WHITE_COL);
			if(TextureFlags&FLAGS_QUADS)
			{
				DrawLineC(x[1],y[1],x[3],y[3],WHITE_COL);
				DrawLineC(x[3],y[3],x[2],y[2],WHITE_COL);
			}
			else
				DrawLineC(x[1],y[1],x[2],y[2],WHITE_COL);

	//		if(!(TextureFlags&FLAGS_FIXED))
			{
				if(TextureFlags&FLAGS_QUADS)
					for(c0=0;c0<4;c0++)
					{
						ClickRect[c0].SetRect(x[c0]-3,y[c0]-3,7,7);
						ClickRect[c0].OutlineRect(WHITE_COL);
//						ClickRect[c0].HiliteRect(HILITE_COL,LOLITE_COL);
					}
				else
					for(c0=0;c0<3;c0++)
					{
						ClickRect[c0].SetRect(x[c0]-3,y[c0]-3,7,7);
						ClickRect[c0].OutlineRect(WHITE_COL);
//						ClickRect[c0].HiliteRect(HILITE_COL,LOLITE_COL);
					}
			}
			if(next_face_selected)
				draw_selected_face_textures(-texture_x,-texture_y,zoom);
	//			draw_selected_face_textures(-texture_x+((PaintRect.GetLeft()+2)),-texture_y+((PaintRect.GetTop()+2)),zoom);
		}
	}
}

//---------------------------------------------------------------

void	PaintTab::UpdateTexture(void)
{
	if(LockWorkScreen())
	{
		DrawTexture();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}

//---------------------------------------------------------------

UBYTE	bit_tab[10]	=	{	0,1,2,3,4,4,4,4,4,4	};

void	PaintTab::UpdateTextureInfo(void)
{
	CBYTE	str[20];
	if(CurrentTexturePage>=0)
		((CStaticText*)GetControlPtr(CTRL_TEX_PAGE_TEXT))->SetString1(texture_menu[CurrentTexturePage].ItemText);
//		((CStaticText*)TextureSet.GetControlPtr(CTRL_TEX_PAGE_TEXT))->SetString1(texture_menu[CurrentTexturePage].ItemText);
	sprintf(str,"%d x %d",TextureWidth,TextureHeight);

	((CStaticText*)GetControlPtr(CTRL_TEX_SIZE_TEXT))->SetString1(str);
//md	((CStaticText*)TextureSet.GetControlPtr(CTRL_TEX_SIZE_TEXT))->SetString1(str);
//	((CStaticText*)GetControlPtr(CTRL_TEX_SIZE_TEXT))->SetString1(texture_size[bit_tab[TextureSize>>4]].ItemText);

//	TextureSet.SetControlState(CTRL_TEX_NO_LIGHT,(edit_info.FlatShade ? CTRL_SELECTED : CTRL_DESELECTED));
	SetControlState(CTRL_TEX_NO_HIDDEN,(edit_info.NoHidden ? CTRL_SELECTED : CTRL_DESELECTED));
	SetControlState(CTRL_TEX_USE_CLIPPED,(edit_info.Clipped&2 ? CTRL_SELECTED : CTRL_DESELECTED));
	SetControlState(CTRL_TEX_ROOF_TEX,(edit_info.RoofTex ? CTRL_SELECTED : CTRL_DESELECTED));
	SetControlState(CTRL_TEX_SELECT_DRAWN,(SelectDrawn ? CTRL_SELECTED : CTRL_DESELECTED));
	SetControlState(CTRL_TEX_QUAD_BOX,(TextureFlags&FLAGS_QUADS ? CTRL_SELECTED : CTRL_DESELECTED));
	SetControlState(CTRL_TEX_FIXED_BOX,(TextureFlags&FLAGS_FIXED ? CTRL_SELECTED : CTRL_DESELECTED));
//	TextureSet.SetControlState(CTRL_TEX_SIZE_MENUW,(TextureFlags&FLAGS_FIXED ? CTRL_ACTIVE : CTRL_INACTIVE));
//	TextureSet.SetControlState(CTRL_TEX_SIZE_MENUH,(TextureFlags&FLAGS_FIXED ? CTRL_ACTIVE : CTRL_INACTIVE));
	if(SelectFlag==5)
		SetControlState(CTRL_TEX_PLANAR_MAP,(CTRL_SELECTED ));
	else
		SetControlState(CTRL_TEX_PLANAR_MAP,(CTRL_DESELECTED ));
	if(SelectFlag==6)
		SetControlState(CTRL_TEX_PLANAR_MAPF,(CTRL_SELECTED ));
	else
		SetControlState(CTRL_TEX_PLANAR_MAPF,(CTRL_DESELECTED ));

}

//---------------------------------------------------------------

void	PaintTab::DrawPalette(void)
{
	ULONG		c0,c1,
				colour,
				*rect_ptr,
				*temp_ptr;
	EdRect		colour_rect;

	LogText(" draw pal currentcol %d \n",CurrentColour);


	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
	rect_ptr	=	(ULONG*)(WorkWindow+PaintRect.GetLeft()*2+2+((PaintRect.GetTop()+1)*WorkScreenWidth));
	for(c0=0;c0<256;c0++)
	{
		colour		=	pal_to_16[c0]*0x00010001;
		temp_ptr	=	rect_ptr+((c0&0x0f)<<3)+(((c0&0xff0)>>2)*WorkScreenWidth);
		for(c1=0;c1<16;c1++,temp_ptr+=(WorkScreenWidth>>2))
		{
			*(temp_ptr+0)	=	colour;
			*(temp_ptr+1)	=	colour;
			*(temp_ptr+2)	=	colour;
			*(temp_ptr+3)	=	colour;
			*(temp_ptr+4)	=	colour;
			*(temp_ptr+5)	=	colour;
			*(temp_ptr+6)	=	colour;
			*(temp_ptr+7)	=	colour;
		}
	}

	colour_rect.SetRect	(
							(PaintRect.GetLeft()+1)+((CurrentColour&0x0f)<<4),
							(PaintRect.GetTop()+1)+(CurrentColour&0xf0),
							16,
							16
						);
	colour_rect.OutlineRect(WHITE_COL);
}

//---------------------------------------------------------------

void	PaintTab::UpdatePalette(void)
{
	if(LockWorkScreen())
	{
		DrawPalette();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}

//---------------------------------------------------------------

void	PaintTab::UpdatePaletteInfo(void)
{
	
}

//---------------------------------------------------------------

void	PaintTab::do_undo_me_bloody_self_then(SLONG index)
{
	struct	GenericUndo	*p_u;
	SLONG	c0;
	if(index<0)
		p_u=&MyUndo.undo_undo_info[-index];
	else
		p_u=&MyUndo.undo_info[index];

	switch(p_u->Type)
	{
		case	UNDO_MOVE_TEXTURE:
				p_u->Type=0;
				MyUndo.MoveTexture(index<0?0:1,CurrentTexturePage,0,CurrentTexture.U[0],CurrentTexture.V[0],CurrentTexture.U[1],CurrentTexture.V[1],
									CurrentTexture.U[2],CurrentTexture.V[2],CurrentTexture.U[3],CurrentTexture.V[3]);

				CurrentTexturePage=p_u->Texture.Page;
				CurrentTexture.U[0]=p_u->Texture.U[0];
				CurrentTexture.U[1]=p_u->Texture.U[1];
				CurrentTexture.U[2]=p_u->Texture.U[2];
				CurrentTexture.U[3]=p_u->Texture.U[3];
				CurrentTexture.V[0]=p_u->Texture.V[0];
				CurrentTexture.V[1]=p_u->Texture.V[1];
				CurrentTexture.V[2]=p_u->Texture.V[2];
				CurrentTexture.V[3]=p_u->Texture.V[3];
//				if(selected_face.Face)
				if(selected_face.PEle)
				{
					if(selected_face.PEle==(struct EditMapElement*)-2)
					{
						
					}
					else
					if(selected_face.PEle==(struct EditMapElement*)-1)
					{
						if(selected_face.Face<0)
						{
							prim_faces3[-selected_face.Face].TexturePage	=	(UWORD)CurrentTexturePage;
							for(c0=0;c0<3;c0++)
							{
								prim_faces3[-selected_face.Face].UV[c0][0]		=	CurrentTexture.U[c0];
								prim_faces3[-selected_face.Face].UV[c0][1]		=	CurrentTexture.V[c0];
							}
						}
						else
						{
							prim_faces4[selected_face.Face].TexturePage	=	(UWORD)CurrentTexturePage;
							for(c0=0;c0<4;c0++)
							{
								prim_faces4[selected_face.Face].UV[c0][0]	=	CurrentTexture.U[c0];
								prim_faces4[selected_face.Face].UV[c0][1]	=	CurrentTexture.V[c0];
							}
						}
					}
					if(LockWorkScreen())
					{
						Parent->DrawContent();
						UnlockWorkScreen();
						ShowWorkWindow(0);
					}
				}
			break;
	}
}

void	PaintTab::CutFloorBrush(BuildTab *BuildMode,MFPoint *current_point)
{

	MFPoint		point1,point2;
	SLONG		con_top,con_left;
	SLONG		x,y,w,h;
	SLONG		mx1,my1,mz1;
	SLONG		mx2,my2,mz2;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	point1.X	=	MouseX;
	point1.Y	=	MouseY;
	Parent->GlobalToLocal(&point1);
	BuildMode->CalcMapCoord(&mx1,&my1,&mz1,x,y,w,h,&point1);
//						point1		=	*clicked_point;
	while(SHELL_ACTIVE && LeftButton)
	{
		engine_keys_scroll();
		engine_keys_spin();
		engine_keys_zoom();

		point2.X	=	MouseX;//-con_left;
		point2.Y	=	MouseY;//-con_top;
		Parent->GlobalToLocal(&point2);
		BuildMode->CalcMapCoord(&mx2,&my2,&mz2,x,y,w,h,&point2);

		if(LockWorkScreen())
		{
			Parent->DrawContent();
			Parent->DrawGrowBox();
			BuildMode->Texture=2;
			BuildMode->DrawContentRect(mx1,mz1,mx2,mz2,WHITE_COL);
			BuildMode->Texture=0;
			UnlockWorkScreen();
		}
		ShowWorkWindow(0);
	}
	{
		SLONG	mw,mh;
		mw=mx2-mx1;
		if(mw<0)
		{
			mw=-mw;
			mx1=mx2;
		}
		mh=mz2-mz1;
		if(mh<0)
		{
			mh=-mh;
			mz1=mz2;
		}
		CutMapBlock.Cut(mx1>>ELE_SHIFT,mz1>>ELE_SHIFT,mw>>ELE_SHIFT,mh>>ELE_SHIFT,0);
		SubMode=FLOOR_PASTE_BRUSH;

	}

	RequestUpdate();
}

void	PaintTab::HandleTab(MFPoint *current_point)
{
	ULONG		control_id;
	SLONG		update	=	0;


//	ModeTab::HandleTab(current_point);
	if(PaintMode==PALETTE_PAINT)
		PaletteSet.HandleControlSet(current_point);
	else
	if(PaintMode==STYLE_PAINT)
		StyleSet.HandleControlSet(current_point);
	else
		HandleControlSet(current_point);

	if(PaintMode==ANIM_TMAP_PAINT)
	{
		if(CurrentTexturePage<0)
			CurrentTexturePage=0;

	}
	if(PaintMode==FLOOR_PAINT)
	{
		if(Keys[KB_B])
		{
			SubMode=FLOOR_CUT_BRUSH;
			SubStatus=0;
		}
	}

	if(Keys[KB_U])
	{
		SLONG	index;
		Keys[KB_U]=0;
		index=MyUndo.DoUndo(ShiftFlag?1:0);
		if(index)
			do_undo_me_bloody_self_then(index);
		update	=	1;
	}
	if(Keys[KB_PMINUS])
	{
		if(TextureZoom<8)
		{
			TextureZoom++;
			update	=	1;
		}
	}
	else if(Keys[KB_PPLUS])
	{
		if(TextureZoom>0)
		{
			TextureZoom--;
			update	=	1;
		}
	}
	if(Keys[KB_P4])
	{
		if(TextureX>0)
		{
			TextureX--;
			update	=	1;
		}
	}
	else if(Keys[KB_P6])
	{
		if(TextureX<(256-(1<<TextureZoom)))
		{
			TextureX++;
			update	=	1;
		}
	}
	if(Keys[KB_P8])
	{
		if(TextureY>0)
		{
			TextureY--;
			update	=	1;
		}
	}
	else if(Keys[KB_P2])
	{
		if(TextureY<(256-(1<<TextureZoom)))
		{
			TextureY++;
			update	=	1;
		}
	}

	if(Keys[KB_K])
	{
		SLONG	scale=	260;
		if(ShiftFlag)
			scale+=20;
		scale_selected_tmaps(scale);
//		edit_info.TileScale+=5;

		Keys[KB_K]=0;

		update=1;
	}

	if(Keys[KB_L])
	{
		SLONG	scale=	250;
		if(ShiftFlag)
			scale-=20;
		scale_selected_tmaps(scale);
//		edit_info.TileScale-=5;
//		if(edit_info.TileScale<0)
//			edit_info.TileScale=0;

		Keys[KB_L]=0;
		update=1;
	}

void	offset_selected_tex_page(SWORD offset);
	if(Keys[KB_PLUS])
	{
		SLONG	offset=1;
		Keys[KB_PLUS]=0;
		if(ShiftFlag)
			offset=64;
		offset_selected_tex_page(offset);
	}
	if(Keys[KB_MINUS])
	{
		SLONG	offset=-1;
		Keys[KB_MINUS]=0;
		if(ShiftFlag)
			offset=-64;
		offset_selected_tex_page(offset);
	}

	if(LastKey==KB_P&&(!ShiftFlag))
	{
		LastKey=0;
		SelectFlag=5;
		SetPaintMode(PLANAR_PAINT);
		update=1;
//		DrawTabContent();
	}

	if(!ControlFlag)
	{
		SLONG	offset=0;
		
		if(ShiftFlag)
			offset=10;
		if(Keys[KB_1])
		{
			CurrentTexturePage	=	0+offset;
			update=1;
		}
		if(Keys[KB_2])
		{
			CurrentTexturePage	=	1+offset;
			update=1;
		}
		if(Keys[KB_3])
		{
			CurrentTexturePage	=	2+offset;
			update=1;
		}
		if(Keys[KB_4])
		{
			CurrentTexturePage	=	3+offset;
			update=1;
		}
		if(Keys[KB_5])
		{
			CurrentTexturePage	=	4+offset;
			update=1;
		}
		if(Keys[KB_6])
		{
			CurrentTexturePage	=	5+offset;
			update=1;
		}
		if(Keys[KB_7])
		{
			CurrentTexturePage	=	6+offset;
			update=1;
		}
		if(Keys[KB_8])
		{
			CurrentTexturePage	=	7+offset;
			update=1;
		}


		if(Keys[KB_9])
		{
			CurrentTexturePage	=	8+offset;
			update=1;
		}
 //not yet needed
		if(Keys[KB_0])
		{
			CurrentTexturePage	=	9+offset;
			update=1;
		}
		if(update)
		if(CurrentTexturePage>20)
			CurrentTexturePage=20;

		if(LastKey)
		{
			control_id	=	HandleControlSetKey(LastKey);
			if(control_id)
			{
				HandleControl(control_id);
				LastKey	=	0;
			}
			else
			{
				if(PaintMode==PALETTE_PAINT)
				{
					if(LastKey==KB_LBRACE)
					{
						CurrentColour	=	(CurrentColour-1)&0xff;
						update			=	1;
						LastKey			=	0;
					}
					else if(LastKey==KB_RBRACE)
					{
						CurrentColour	=	(CurrentColour+1)&0xff;
						update			=	1;
						LastKey			=	0;
					}
					else
					{
						control_id	=	PaletteSet.HandleControlSetKey(LastKey);
						if(control_id)
						{
							HandlePaletteControl(control_id);
							LastKey	=	0;
						}
					}
				}
				else
				if(PaintMode==STYLE_PAINT)
				{
					if(LastKey==KB_LBRACE)
					{
						update			=	1;
						LastKey			=	0;
					}
					else if(LastKey==KB_RBRACE)
					{
						update			=	1;
						LastKey			=	0;
					}
					else
					{
						control_id	=	StyleSet.HandleControlSetKey(LastKey);
						if(control_id)
						{
							HandleStyleControl(control_id);
							LastKey	=	0;
						}
					}
				}
				else
				{
					control_id	=	HandleControlSetKey(LastKey); //ts
					if(control_id)
					{
						HandleTextureControl(control_id);
						LastKey	=	0;
					}
				}
			}
		}
/*
		if(Keys[KB_F])
		{
			Keys[KB_F]=0;
			TextureFlags	^=	FLAGS_FIXED;
			if(TextureFlags & FLAGS_FIXED)
				SetControlState(CTRL_TEX_FIXED_BOX,CTRL_SELECTED);
			else
				SetControlState(CTRL_TEX_FIXED_BOX,CTRL_DESELECTED);
			if(LockWorkScreen())
			{
				
				DrawControlSet();
//				DrawTabContent();
				UnlockWorkScreen();
			}
			ShowWorkWindow(0);
		UpdateTexture();
			return;
		}
*/
	}

	if(update)
	{
		if(PaintMode==PALETTE_PAINT)
			UpdatePalette();
		else
			UpdateTexture();
	}
}

//---------------------------------------------------------------

#define	DRAG_NORMAL			0
#define	DRAG_SHIFT			1
#define	DRAG_SHIFT_H		2
#define	DRAG_SHIFT_V		3
#define	DRAG_CONTROL		4

ULONG	hooks[4][2]	=
{
	{	2,1	},
	{	3,0	},
	{	0,3	},
	{	1,2	}
};

void	PaintTab::SelectAnimTexture(MFPoint *clicked_point)
{
	SLONG x,y;

	x=clicked_point->X-AnimRect.GetLeft();
	y=clicked_point->Y-AnimRect.GetTop();

	y=(y/19)+ShowAnimTmap;
	if(PaintMode==ANIM_TMAP_PAINT)
	{
		
	}
	else
	{
		CurrentTexturePage=-y;
		RequestUpdate();
	}
}

void	PaintTab::SetEditAnimTexture(MFPoint *clicked_point)
{
	SLONG x,y;

	x=clicked_point->X-AnimRect.GetLeft();
	y=clicked_point->Y-AnimRect.GetTop();

	y=(y/19)+ShowAnimTmap;
	if(PaintMode==ANIM_TMAP_PAINT)
	{
		
	}
	else
	{
		SetPaintMode(ANIM_TMAP_PAINT);
//		PaintMode=ANIM_TMAP_PAINT;
		CurrentTexturePage=0;
		CurrentAnimTmap=y;
		RequestUpdate();
	}
}

UWORD	PaintTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
{
	ULONG		control_id;
	SLONG		zoom;
	Control		*current_control;
	MFPoint		current_point,
				local_point;


	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
			local_point	=	*clicked_point;
			GlobalToLocal(&local_point);
			if(AnimRect.PointInRect(&local_point)&&RightButton==0&&PaintMode!=STYLE_PAINT)
			{
				SelectAnimTexture(&local_point);
				UpdateTexture();
			}
			else
			if( (PaintRect.PointInRect(&local_point)) && (RightButton==0) && (PaintMode!=STYLE_PAINT) )
			{
				if(PaintMode==FLOOR_PAINT||PaintMode==TEXTURE_PAINT||PaintMode==ANIM_TMAP_PAINT||PaintMode==STYLE_DEFINE||PaintMode==PSX_TEX_DEFINE||PaintMode==INSTYLE_DEFINE)
				{
					SelectTexture(clicked_point);
					SubMode=0;
				}
				else
				if(PaintMode==PALETTE_PAINT)
					SelectColour(clicked_point);
				else
				if(PaintMode==PLANAR_PAINT)
					PlanarMapping(clicked_point);

			}
			else
			{
				if(PaintMode==STYLE_PAINT)
				{
					SelectStyle(clicked_point);
				}

				local_point	=	*clicked_point;
				GlobalToLocal(&local_point);
/*
				control_id	=	HandleControlSetClick(flags,&local_point);
				HandleControl(control_id);
*/

				current_control	=	GetControlList();//ts
				while(current_control)
				{
					if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(&local_point))
					{
						// Handle control.
						control_id	=	current_control->TrackControl(&local_point);
						HandleControl(control_id);

						// Tidy up display.
						if(LockWorkScreen())
						{
							DrawTab();
							UnlockWorkScreen();
						}
						ShowWorkWindow(0);

						return	control_id;
					}
					current_control	=	current_control->GetNextControl();
				}

/*
				if(control_id)
				{
					return	control_id;
				}
				else
*/
				{
					if(PaintMode==PALETTE_PAINT)
					{
						control_id	=	PaletteSet.HandleControlSetClick(flags,clicked_point);
						HandlePaletteControl(control_id);
					}
					else
					if(PaintMode==STYLE_PAINT)
					{
						control_id	=	StyleSet.HandleControlSetClick(flags,clicked_point);
						HandleStyleControl(control_id);
					}
					else
					{
						control_id	=	HandleControlSetClick(flags,clicked_point); //ts
						HandleTextureControl(control_id);
					}
				}
			}
			break;
		case	RIGHT_CLICK:
			SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
			local_point	=	*clicked_point;
			GlobalToLocal(&local_point);
			if(PaintMode==STYLE_PAINT)
			{
			}
			else
			if(AnimRect.PointInRect(&local_point))
			{
				SetEditAnimTexture(&local_point);
				UpdateTexture();
			}
			else
			if(PaintRect.PointInRect(&local_point))
			{
				zoom	=	0;
				SubMode=0;
				while(SHELL_ACTIVE && RightButton)
				{
					current_point.X	=	MouseX;
					current_point.Y	=	MouseY;
					GlobalToLocal(&current_point);
					if(current_point.X!=local_point.X || current_point.Y!=local_point.Y)
					{
						if(LeftButton)
						{
							zoom		+=	local_point.Y-current_point.Y;
							if((zoom/4))
							{
								TextureZoom	+=	zoom/4;
								zoom		=	0;
							}
							if(TextureZoom<0)
								TextureZoom	=	0;
							else if(TextureZoom>8)
								TextureZoom	=	8;
						}
						else
						{
							TextureX	+=	local_point.X-current_point.X;
							TextureY	+=	local_point.Y-current_point.Y;
							if(TextureX<0)
								TextureX	=	0;
							else if(TextureX>(256-(1<<TextureZoom)))
								TextureX	=	(256-(1<<TextureZoom));
							if(TextureY<0)
								TextureY	=	0;
							else if(TextureY>(256-(1<<TextureZoom)))
								TextureY	=	(256-(1<<TextureZoom));
						}
						UpdateTexture();
						local_point	=	current_point;
					}
				}
			}
			break;
	}
	return	0;
}

//---------------------------------------------------------------

void	PaintTab::HandleControl(UWORD control_id)
{



	switch(control_id&0xff)
	{

		case	CTRL_TEX_TILE_SIZE:
			edit_info.TileScale	=	((CHSlider*)GetControlPtr(CTRL_TEX_TILE_SIZE))->GetCurrentValue(); //ts

			RequestUpdate();
			break;

		case	CTRL_PAINT_MENU:
			PaintMode	=	(control_id>>8)-1;
			RequestUpdate();
			break;
		case	CTRL_PAINT_TEXT:
			break;
		case	CTRL_TEX_PAGE_TEXT:
			break;
		case	CTRL_TEX_PAGE_MENU:
			CurrentTexturePage	=	(control_id>>8)-1;
			if(CurrentTexturePage>max_textures)
			{
				CurrentTexturePage=max_textures;

			}
			break;
		case	CTRL_TEX_HIDE_L:
			if(edit_info.HideMap&1)
			{
				edit_info.HideMap&=~1;
				SetControlState(CTRL_TEX_HIDE_L,CTRL_DESELECTED); 
			}
			else
			{
				SetControlState(CTRL_TEX_HIDE_L,CTRL_SELECTED); 
				edit_info.HideMap|=1;
			}

			break;
		case	CTRL_TEX_HIDE_R:
			if(edit_info.HideMap&2)
			{
				edit_info.HideMap&=~2;
				SetControlState(CTRL_TEX_HIDE_R,CTRL_DESELECTED); 
			}
			else
			{
				SetControlState(CTRL_TEX_HIDE_R,CTRL_SELECTED); 
				edit_info.HideMap|=2;
			}
			break;
		case	CTRL_TEX_INFO:
			show_info^=1;
			break;
		case	CTRL_TEX_CLEAR:
			{
				Alert				rus;
				if(rus.HandleAlert("Remove painted textures?",NULL)==1)
				{
					remove_painted_textures();
				}
				if(rus.HandleAlert("Remove style textures?",NULL)==1)
				{
					remove_style_textures();
				}
			}
			break;

		case	CTRL_BUILD_OTHER:
extern	SLONG	build_psx;
			build_psx^=1;
			create_city(BUILD_MODE_EDITOR);
			break;
		case	CTRL_TEX_HIDE_ROOF:
			if(edit_info.HideMap&4)
			{
				edit_info.HideMap&=~4;
				SetControlState(CTRL_TEX_HIDE_ROOF,CTRL_DESELECTED); 
			}
			else
			{
				SetControlState(CTRL_TEX_HIDE_ROOF,CTRL_SELECTED); 
				edit_info.HideMap|=4;
			}
			break;

		case	CTRL_TEX_QUAD_BOX:
//			ToggleControlSelectedState(CTRL_TEX_QUAD_BOX);
			TextureFlags	^=	FLAGS_QUADS;
			break;
		case	CTRL_TEX_FIXED_BOX:
//			ToggleControlSelectedState(CTRL_TEX_FIXED_BOX);
			TextureFlags	^=	FLAGS_FIXED;
//			ToggleControlActiveState(CTRL_TEX_SIZE_MENU);
			break;
/*
		case	CTRL_TEX_SIZE_MENUW:
//			TextureWidth	=	1<<((control_id>>8)+2);
			TextureWidth	=	32; //texture_sizes[(control_id>>8)-1];
			break;
		case	CTRL_TEX_SIZE_MENUH:
//			TextureHeight	=	1<<((control_id>>8)+2);
			TextureHeight	=	32;//texture_sizes[(control_id>>8)-1];
			break;
*/
		case	CTRL_TEX_SIZE_TEXT:
			break;
		case	CTRL_TEX_IMPORT_TEX:
			{
				FileRequester	*fr;
				CBYTE	fname[100];
				fr=new FileRequester("data\\","*.tex","Save A Prim","temp.tex");
				if(fr->Draw())
				{
					strcpy(fname,fr->Path);
					strcat(fname,fr->FileName);
					import_tex(fname);
				}
				delete fr;
				RequestUpdate();
			}
			break;
		case	CTRL_TEX_LOAD_ANIMTMAP:
			load_animtmaps();
			break;
		case	CTRL_TEX_SAVE_ANIMTMAP:
			save_animtmaps();
			break;
		case	CTRL_TEX_ANIMTMAP_UP:
			if(ShowAnimTmap>1)
			{
				ShowAnimTmap--;
				RequestUpdate();
			}
			break;
		case	CTRL_TEX_ANIMTMAP_DOWN:
			if(ShowAnimTmap<64)
			{
				ShowAnimTmap++;
				RequestUpdate();
			}
			break;
		case	CTRL_TEX_PLANAR_MAP:
			DoPlanarMap();
			break;
		case	CTRL_TEX_PLANAR_MAPF:
			DoPlanarMapF();
			break;
		case	CTRL_TEX_SELECT_DRAWN:
			SelectDrawn	^=	1;
			break;
//		case	CTRL_TEX_NO_LIGHT:
//			edit_info.FlatShade^=1;
//			break;
		case	CTRL_TEX_NO_HIDDEN:
			edit_info.NoHidden^=1;
			break;
		case	CTRL_TEX_ROOF_TEX:
			edit_info.RoofTex^=1;
			break;
		case	CTRL_TEX_USE_CLIPPED:
			edit_info.Clipped^=2;
			break;
		case	CTRL_TEX_SET_CLIPPED:
extern	void	find_map_clip(SLONG *minx,SLONG *maxx,SLONG *minz,SLONG *maxz);
			{

				SLONG	minx,maxx,minz,maxz;

				find_map_clip(&minx,&maxx,&minz,&maxz);
				edit_info.MinX=minx;
				edit_info.MinZ=minz;
				edit_info.MaxX=maxx;
				edit_info.MaxZ=maxz;
				edit_info.Clipped=2;

			}

			break;
	}

	UpdateTextureInfo();
	UpdateTabInfo();

	// Tidy up display.
	if(LockWorkScreen())
	{
		DrawTab();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}

//---------------------------------------------------------------

void	PaintTab::HandlePaletteControl(UWORD control_id)
{
	UpdatePaletteInfo();

	// Tidy up display.
	if(LockWorkScreen())
	{
		DrawTab();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}

void	PaintTab::HandleStyleControl(UWORD control_id)
{

	switch(control_id&0xff)
	{
		case	CTRL_STYLE_PAINT_SLIDER:
			new_scroll_pos();
//			RequestUpdate();
			break;
	}

	// Tidy up display.
	if(LockWorkScreen())
	{
		DrawTab();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}

void	PaintTab::DoPlanarMap(void)
{
 	if(LockWorkScreen())
	{
		if(SelectFlag==5)
		{
			SetControlState(CTRL_TEX_PLANAR_MAP,(CTRL_DESELECTED )); //ts
			SetControlState(CTRL_TEX_PLANAR_MAPF,(CTRL_DESELECTED ));
			SelectFlag=0;
		}
		else
		if(SelectFlag==6)
		{
			SetControlState(CTRL_TEX_PLANAR_MAP,(CTRL_SELECTED ));
			SetControlState(CTRL_TEX_PLANAR_MAPF,(CTRL_DESELECTED ));
			SelectFlag=5;
		}
		else
		{
			SetControlState(CTRL_TEX_PLANAR_MAP,(CTRL_SELECTED ));
			SetControlState(CTRL_TEX_PLANAR_MAPF,(CTRL_DESELECTED ));
			SelectFlag=5;

		}
		Parent->DrawContent();
		UnlockWorkScreen();
		ShowWorkWindow(0);
	}
	ShowWorkWindow(0);
	RequestUpdate();

}
void	PaintTab::DoPlanarMapF(void)
{
 	if(LockWorkScreen())
	{
		if(SelectFlag==6)
		{
			SetControlState(CTRL_TEX_PLANAR_MAP,(CTRL_DESELECTED ));
			SetControlState(CTRL_TEX_PLANAR_MAPF,(CTRL_DESELECTED ));
			SelectFlag=0;
		}
		else
		if(SelectFlag==5)
		{
			SetControlState(CTRL_TEX_PLANAR_MAP,(CTRL_DESELECTED ));
			SetControlState(CTRL_TEX_PLANAR_MAPF,(CTRL_SELECTED ));
			SelectFlag=6;
		}
		else
		{
			SetControlState(CTRL_TEX_PLANAR_MAP,(CTRL_DESELECTED ));
			SetControlState(CTRL_TEX_PLANAR_MAPF,(CTRL_SELECTED ));
			SelectFlag=6;

		}
		Parent->DrawContent();
		UnlockWorkScreen();
		ShowWorkWindow(0);
	}
	ShowWorkWindow(0);
	RequestUpdate();

}

//---------------------------------------------------------------

void	PaintTab::HandleTextureControl(UWORD control_id)
{
	
	switch(control_id&0xff)
	{
		case	CTRL_TEX_PAGE_TEXT:
			break;
		case	CTRL_TEX_PAGE_MENU:
			CurrentTexturePage	=	(control_id>>8)-1;
			if(CurrentTexturePage>max_textures)
			{
				CurrentTexturePage=max_textures;

			}
			break;
		case	CTRL_TEX_QUAD_BOX:
//			ToggleControlSelectedState(CTRL_TEX_QUAD_BOX);
			TextureFlags	^=	FLAGS_QUADS;
			break;
		case	CTRL_TEX_FIXED_BOX:
//			ToggleControlSelectedState(CTRL_TEX_FIXED_BOX);
			TextureFlags	^=	FLAGS_FIXED;
//			ToggleControlActiveState(CTRL_TEX_SIZE_MENU);
			break;
/*
		case	CTRL_TEX_SIZE_MENUW:
//			TextureWidth	=	1<<((control_id>>8)+2);
			TextureWidth	=	32; //texture_sizes[(control_id>>8)-1];
			break;
		case	CTRL_TEX_SIZE_MENUH:
//			TextureHeight	=	1<<((control_id>>8)+2);
			TextureHeight	=	32;//texture_sizes[(control_id>>8)-1];
			break;
*/
		case	CTRL_TEX_SIZE_TEXT:
			break;
		case	CTRL_TEX_IMPORT_TEX:
			{
				FileRequester	*fr;
				CBYTE	fname[100];
				fr=new FileRequester("data\\","*.tex","Save A Prim","temp.tex");
				if(fr->Draw())
				{
					strcpy(fname,fr->Path);
					strcat(fname,fr->FileName);
					import_tex(fname);
				}
				delete fr;
				RequestUpdate();
			}
			break;
		case	CTRL_TEX_LOAD_ANIMTMAP:
			load_animtmaps();
			break;
		case	CTRL_TEX_SAVE_ANIMTMAP:
			save_animtmaps();
			break;
		case	CTRL_TEX_ANIMTMAP_UP:
			if(ShowAnimTmap>1)
			{
				ShowAnimTmap--;
				RequestUpdate();
			}
			break;
		case	CTRL_TEX_ANIMTMAP_DOWN:
			if(ShowAnimTmap<64)
			{
				ShowAnimTmap++;
				RequestUpdate();
			}
			break;
		case	CTRL_TEX_PLANAR_MAP:
			DoPlanarMap();
			break;
		case	CTRL_TEX_PLANAR_MAPF:
			DoPlanarMapF();
			break;
		case	CTRL_TEX_SELECT_DRAWN:
			SelectDrawn	^=	1;
			break;
//		case	CTRL_TEX_NO_LIGHT:
//			edit_info.FlatShade^=1;
//			break;
		case	CTRL_TEX_NO_HIDDEN:
			edit_info.NoHidden^=1;
			break;
		case	CTRL_TEX_ROOF_TEX:
			edit_info.RoofTex^=1;
			break;
		case	CTRL_TEX_USE_CLIPPED:
			edit_info.Clipped^=2;
			break;
		case	CTRL_TEX_SET_CLIPPED:
extern	void	find_map_clip(SLONG *minx,SLONG *maxx,SLONG *minz,SLONG *maxz);
			{

				SLONG	minx,maxx,minz,maxz;

				find_map_clip(&minx,&maxx,&minz,&maxz);
				edit_info.MinX=minx;
				edit_info.MinZ=minz;
				edit_info.MaxX=maxx;
				edit_info.MaxZ=maxz;
				edit_info.Clipped=2;

			}

			break;
	}
	UpdateTextureInfo();
//	DrawTabContent();

// Tidy up display.
	if(LockWorkScreen())
	{
		DrawTab();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}

//---------------------------------------------------------------

void	PaintTab::SelectColour(MFPoint *clicked_point)
{
	MFPoint		local_point;


	local_point	=	*clicked_point;
	GlobalToLocal(&local_point);

	local_point.X	-=	PaintRect.GetLeft()+2;
	local_point.Y	-=	PaintRect.GetTop()+2;

	CurrentColour	=	((local_point.X&0xff)>>4)+(local_point.Y&0xf0);
	UpdatePalette();
}

//---------------------------------------------------------------
void	find_highest_selected_tmap(SLONG *tx,SLONG *ty)
{
	SLONG	c0,c1;
	struct	PrimFace4	*p_f4;
	struct	PrimFace3	*p_f3;
	*tx=1000000;
	*ty=1000000;

	for(c0=1;c0<next_face_selected;c0++)
	{
		if(face_selected_list[c0]>0)
		{
			p_f4=&prim_faces4[face_selected_list[c0]];
			for(c1=0;c1<4;c1++)
			{
				if(p_f4->UV[c1][1]<*ty||(p_f4->UV[c1][1]==*ty&&p_f4->UV[c1][0]<*tx))
				{
					*tx=p_f4->UV[c1][0];
					*ty=p_f4->UV[c1][1];
				}
			}
		}
		else
		if(face_selected_list[c0]<0)
		{
			p_f3=&prim_faces3[-face_selected_list[c0]];
			for(c1=0;c1<3;c1++)
			{
				if(p_f3->UV[c1][1]<*ty||(p_f3->UV[c1][1]==*ty&&p_f3->UV[c1][0]<*tx))
				{
					*tx=p_f3->UV[c1][0];
					*ty=p_f3->UV[c1][1];
				}
			}

		}
	}		 
}

void	offset_selected_tmaps(SLONG tx,SLONG ty,UBYTE	page,SLONG sx,SLONG sy,SLONG flag)
{
	SLONG	c0,c1;
	struct	PrimFace4	*p_f4;
	struct	PrimFace3	*p_f3;

	for(c0=1;c0<next_face_selected;c0++)
	{
		if(face_selected_list[c0]>0)
		{
			p_f4=&prim_faces4[face_selected_list[c0]];
			p_f4->TexturePage=page;
			for(c1=0;c1<4;c1++)
			{
				if( (p_f4->UV[c1][0]==sx&&p_f4->UV[c1][1]==sy) || flag)
				{
					p_f4->UV[c1][0]+=tx;
					p_f4->UV[c1][1]+=ty;
				}
			}
		}
		else
		if(face_selected_list[c0]<0)
		{
			p_f3=&prim_faces3[-face_selected_list[c0]];
			p_f3->TexturePage=page;
			for(c1=0;c1<3;c1++)
			{
				if((p_f3->UV[c1][0]==sx&&p_f3->UV[c1][1]==sy) ||flag)
				{
					p_f3->UV[c1][0]+=tx;
					p_f3->UV[c1][1]+=ty;
				}
			}

		}
	}
}

void	scale_selected_tmaps(SWORD scale)
{
	SLONG	c0,c1;
	struct	PrimFace4	*p_f4;
	struct	PrimFace3	*p_f3;
	SLONG	mid_x=0,mid_y=0,count=0;

	for(c0=1;c0<next_face_selected;c0++)
	{
		if(face_selected_list[c0]>0)
		{
			p_f4=&prim_faces4[face_selected_list[c0]];
			for(c1=0;c1<4;c1++)
			{
				mid_x+=p_f4->UV[c1][0];
				mid_y+=p_f4->UV[c1][1];
				count++;
			}
		}
		else
		if(face_selected_list[c0]<0)
		{
			p_f3=&prim_faces3[-face_selected_list[c0]];
			for(c1=0;c1<3;c1++)
			{
				mid_x+=p_f3->UV[c1][0];
				mid_y+=p_f3->UV[c1][1];
				count++;
			}
		}
	}

	if(count)
	{
		mid_x/=count;
		mid_y/=count;
		for(c0=1;c0<next_face_selected;c0++)
		{
			if(face_selected_list[c0]>0)
			{
				p_f4=&prim_faces4[face_selected_list[c0]];
				for(c1=0;c1<4;c1++)
				{
					SLONG	temp;

					temp=p_f4->UV[c1][0];
					temp-=mid_x;
					temp=((temp*scale)/256);
					temp+=mid_x;
					p_f4->UV[c1][0]=temp;
					
					temp=p_f4->UV[c1][1];
					temp-=mid_y;
					temp=((temp*scale)/256);
					temp+=mid_y;
					p_f4->UV[c1][1]=temp;

				}
			}
			else
			if(face_selected_list[c0]<0)
			{
				p_f3=&prim_faces3[-face_selected_list[c0]];
				for(c1=0;c1<3;c1++)
				{
					SLONG	temp;

					temp=p_f3->UV[c1][0];
					temp-=mid_x;
					temp=((temp*scale)/256);
					temp+=mid_x;
					p_f3->UV[c1][0]=temp;
					
					temp=p_f3->UV[c1][1];
					temp-=mid_y;
					temp=((temp*scale)/256);
					temp+=mid_y;
					p_f3->UV[c1][1]=temp;

				}
			}
		}
	}

}

void	offset_selected_tex_page(SWORD offset)
{
	SLONG	c0,c1;
	struct	PrimFace4	*p_f4;
	struct	PrimFace3	*p_f3;
	SLONG	mid_x=0,mid_y=0,count=0;
	SLONG	u,v,page;

	for(c0=1;c0<next_face_selected;c0++)
	{
		if(face_selected_list[c0]>0)
		{
			p_f4=&prim_faces4[face_selected_list[c0]];
			page=p_f4->TexturePage;
			if(offset==64)
				page++;
			else
			if(offset==-64)
				page--;

			if(page<0)
				page=14;

			for(c1=0;c1<4;c1++)
			{
				u=p_f4->UV[c1][0];
				v=p_f4->UV[c1][1];

				if(offset==1)
				{
					u+=32;
					if(u>255)
					{
						u-=256;
						v+=32;
						if(v>255)
						{
							page++;
							if(page>14)
								page=0;
							v-=256;
						}
					}
				}
				else
				if(offset==-1)
				{
					u-=32;
					if(u<0)
					{
						u+=256;
						v-=32;
						if(v<0)
						{
							page--;
							if(page<0)
								page=14;
							v+=256;
						}
					}

				}

				p_f4->UV[c1][0]=u;
				p_f4->UV[c1][1]=v;
				count++;
			}
			p_f4->TexturePage=page;
		}
		else
		if(face_selected_list[c0]<0)
		{
			p_f3=&prim_faces3[-face_selected_list[c0]];
			page=p_f3->TexturePage;
			if(offset==64)
				page++;
			else
			if(offset==-64)
				page--;

			if(page<0)
				page=14;
			for(c1=0;c1<3;c1++)
			{
				u=p_f3->UV[c1][0];
				v=p_f3->UV[c1][1];

				if(offset==1)
				{
					u+=32;
					if(u>255)
					{
						u-=255;
						v+=32;
						if(v>255)
						{
							page++;
							if(page>14)
								page=0;
							v-=255;
						}
					}
				}
				else
				if(offset==-1)
				{
					u-=32;
					if(u<0)
					{
						u+=255;
						v-=32;
						if(v<0)
						{
							page--;
							if(page<0)
								page=14;
							v+=255;
						}
					}

				}

				p_f3->UV[c1][0]=u;
				p_f3->UV[c1][1]=v;

				count++;
			}
			p_f3->TexturePage=page;
		}
	}

}

void	PaintTab::PlanarMapping(MFPoint *clicked_point)
{
	SLONG		c0,c1,
				scale,
				texture_x,
				texture_y,
				dx,
				dy,
				zoom;
	MFPoint		current_point,
				fixed_point,
				local_point;


	local_point	=	*clicked_point;
	GlobalToLocal(&local_point);

	zoom	=	256>>TextureZoom;
	TextureFlags		|=	FLAGS_SHOW_TEXTURE;
//	local_point.X	=	(((local_point.X-(PaintRect.GetLeft()+2))>>(8-TextureZoom)));
//	local_point.Y	=	(((local_point.Y-(PaintRect.GetTop()+2))>>(8-TextureZoom)));
	LogText(" Planar  local point %d,%d \n",local_point.X,local_point.Y);
	LogText(" Planar  paint rect %d,%d \n",PaintRect.GetLeft(),PaintRect.GetTop());

	local_point.X	=	(((local_point.X-(PaintRect.GetLeft()+2))));
	local_point.Y	=	(((local_point.Y-(PaintRect.GetTop()+2))));
	LogText(" Planar  local point %d,%d AFTER -paintrect\n",local_point.X,local_point.Y);
	

	SET_TEXTURE_COORDS
//	local_point.X	=	(local_point.X+texture_x);
//	local_point.Y	=	(local_point.Y+texture_y);
//	find_highest_selected_tmap(&dx,&dy);
//	LogText(" highest texture %d,%d \n",dx,dy);

//	dx-=local_point.X;
//	dy-=local_point.Y;
//	offset_selected_tmaps(dx,dy);
	while(SHELL_ACTIVE && LeftButton)
	{
		current_point.X	=	MouseX;
		current_point.Y	=	MouseY;
		GlobalToLocal(&current_point);

		zoom	=	256>>TextureZoom;
//		current_point.X	=	(((current_point.X-(PaintRect.GetLeft()+2))>>(8-TextureZoom)));
//		current_point.Y	=	(((current_point.Y-(PaintRect.GetTop()+2))>>(8-TextureZoom)));

		current_point.X	=	(((current_point.X-(PaintRect.GetLeft()+2))));
		current_point.Y	=	(((current_point.Y-(PaintRect.GetTop()+2))));

		SET_TEXTURE_COORDS
//		current_point.X	=	(current_point.X+texture_x);
//		current_point.Y	=	(current_point.Y+texture_y);

		if(current_point.X!=local_point.X || current_point.Y!=local_point.Y)
		{
			dx	=	local_point.X-current_point.X;
			dy	=	local_point.Y-current_point.Y;
//			offset_selected_tmaps(-dx,-dy,CurrentTexturePage);
			UpdateTexture();
			local_point	=	current_point;
		}
		if(LockWorkScreen())
		{
			Parent->DrawContent();
			UnlockWorkScreen();
			ShowWorkWindow(0);
		}
	}
	UpdateTexture();
}


void	PaintTab::SelectTexture(MFPoint *clicked_point)
{
	ULONG		drag_flags,
				update,
				x_quad,
				y_quad;
	SLONG		angle,
				c0,c1,
				scale,
				texture_x,
				texture_y,
				x_offset,
				y_offset,
				zoom;
	EdTexture	temp_texture;
	MFPoint		current_point,
				fixed_point,
				local_point;


	local_point	=	*clicked_point;
	GlobalToLocal(&local_point);

	zoom	=	256>>TextureZoom;
	local_point.X	=	(((local_point.X-(PaintRect.GetLeft()+2))>>(8-TextureZoom)));
	local_point.Y	=	(((local_point.Y-(PaintRect.GetTop()+2))>>(8-TextureZoom)));

	SET_TEXTURE_COORDS
	local_point.X	=	(local_point.X+texture_x);
	local_point.Y	=	(local_point.Y+texture_y);

	if(TextureWidth==8)
		current_point.X	=	local_point.X&~7;
	else
		current_point.X	=	local_point.X&~15;

	if(TextureHeight==8)
		current_point.Y	=	local_point.Y&~7;
	else
		current_point.Y	=	local_point.Y&~15;

	if(TextureFlags&FLAGS_QUADS)
		c1	=	4;
	else
		c1	=	3;

	if(TextureFlags&FLAGS_FIXED)
	{
		fixed_point=current_point;
		if(TextureFlags&FLAGS_QUADS)
		{
			update	=	0;
			for(c0=0;c0<c1;c0++)
			{
				SLONG	width,height;
				SLONG	sx,sy;
				current_point	=	*clicked_point;
				current_point.X	-=	PaintRect.GetLeft()+2;
				current_point.Y	-=	PaintRect.GetTop()+2;
				GlobalToLocal(&current_point);
				if(ClickRect[c0].PointInRect(&current_point))
				{
					update		=	1;
					MyUndo.MoveTexture(0,CurrentTexturePage,0,CurrentTexture.U[0],CurrentTexture.V[0],CurrentTexture.U[1],CurrentTexture.V[1],
										CurrentTexture.U[2],CurrentTexture.V[2],CurrentTexture.U[3],CurrentTexture.V[3]);

//
//	Drag	a corner of the fixed rectangle
//
					while(SHELL_ACTIVE && LeftButton)
					{
						current_point.X	=	MouseX;
						current_point.Y	=	MouseY;
						GlobalToLocal(&current_point);

						zoom	=	256>>TextureZoom;
						current_point.X	=	(((current_point.X-(PaintRect.GetLeft()+2))>>(8-TextureZoom)));
						current_point.Y	=	(((current_point.Y-(PaintRect.GetTop()+2))>>(8-TextureZoom)));

						SET_TEXTURE_COORDS
						current_point.X	=	(current_point.X+texture_x);
						current_point.Y	=	(current_point.Y+texture_y);

						if(current_point.X!=local_point.X || current_point.Y!=local_point.Y)
						{
							x_offset	=	local_point.X-current_point.X;
							y_offset	=	local_point.Y-current_point.Y;
							if(TextureFlags&FLAGS_QUADS)
							{
									CurrentTexture.U[c0]	-=	x_offset;
									CurrentTexture.V[c0]	-=	y_offset;

									switch(c0)
									{
										case	0:
											if(CurrentTexture.U[0]>=CurrentTexture.U[1]-1)
												CurrentTexture.U[0]=CurrentTexture.U[1]-1;
											if(CurrentTexture.V[0]>=CurrentTexture.V[2]-1)
												CurrentTexture.V[0]=CurrentTexture.V[2]-1;
											break;
										case	1:
											if(CurrentTexture.U[1]<=CurrentTexture.U[0]+1)
												CurrentTexture.U[1]=CurrentTexture.U[0]+1;
											if(CurrentTexture.V[1]>=CurrentTexture.V[2]-1)
												CurrentTexture.V[1]=CurrentTexture.V[2]-1;
											break;
										case	2:
											if(CurrentTexture.U[2]>=CurrentTexture.U[1]-1)
												CurrentTexture.U[2]=CurrentTexture.U[1]-1;
											if(CurrentTexture.V[2]<=CurrentTexture.V[0]+1)
												CurrentTexture.V[2]=CurrentTexture.V[0]+1;
											break;
										case	3:
											if(CurrentTexture.U[3]<=CurrentTexture.U[0]+1)
												CurrentTexture.U[3]=CurrentTexture.U[0]+1;
											if(CurrentTexture.V[3]<=CurrentTexture.V[0]+1)
												CurrentTexture.V[3]=CurrentTexture.V[0]+1;
											break;
									}
							}



							if(c0<2)
								width=CurrentTexture.U[1]-CurrentTexture.U[0];
							else
							{
								width=CurrentTexture.U[3]-CurrentTexture.U[2];
								if(c0==2)
									CurrentTexture.U[0]=CurrentTexture.U[2];

							}

							TextureWidth=32; //width; //&~31;

							if(c0==1||c0==3)
							{
								height=CurrentTexture.V[3]-CurrentTexture.V[1];
								if(c0==1)
									CurrentTexture.V[0]=CurrentTexture.V[1];
							}
							else
								height=CurrentTexture.V[2]-CurrentTexture.V[0];

							TextureHeight=32; //height; //&~31;

							sx=CurrentTexture.U[0];
							sy=CurrentTexture.V[0];

							CurrentTexture.U[1]=sx+TextureWidth;
							CurrentTexture.V[1]=sy;
							CurrentTexture.U[2]=sx;
							CurrentTexture.V[2]=sy+TextureHeight;
							CurrentTexture.U[3]=sx+TextureWidth;
							CurrentTexture.V[3]=sy+TextureHeight;

							UpdateTexture();
							local_point	=	current_point;
						}
					} //end of while loop 

					TextureHeight=32; //(TextureHeight+7)&~15;
					TextureWidth=32; //(TextureWidth+7)&~15;

					if(TextureHeight<=0)
						TextureHeight=8;
					if(TextureWidth<=0)
						TextureWidth=8;



					CurrentTexture.U[0]=(CurrentTexture.U[0]+7)&~15;
					CurrentTexture.V[0]=(CurrentTexture.V[0]+7)&~15;

					sx=CurrentTexture.U[0];
					sy=CurrentTexture.V[0];

					CurrentTexture.U[1]=sx+TextureWidth;
					CurrentTexture.V[1]=sy;
					CurrentTexture.U[2]=sx;
					CurrentTexture.V[2]=sy+TextureHeight;
					CurrentTexture.U[3]=sx+TextureWidth;
					CurrentTexture.V[3]=sy+TextureHeight;
					UpdateTexture();
					local_point	=	current_point;
#ifndef	PAINT2	
					if(selected_face.PEle)
					{
						ApplyTexture(&selected_face);
						if(LockWorkScreen())
						{
							Parent->DrawContent();
							UnlockWorkScreen();
							ShowWorkWindow(0);
						}
					}
#endif

					break;
				}
			}
			if(!update)
			{
				current_point=fixed_point;
					CurrentTexture.U[0]	=	current_point.X;
				CurrentTexture.V[0]	=	current_point.Y;
				CurrentTexture.U[1]	=	current_point.X+(TextureWidth-1);
				CurrentTexture.V[1]	=	current_point.Y;
				CurrentTexture.U[3]	=	current_point.X+(TextureWidth-1);
				CurrentTexture.V[3]	=	current_point.Y+(TextureHeight-1);
				CurrentTexture.U[2]	=	current_point.X;
				CurrentTexture.V[2]	=	current_point.Y+(TextureHeight-1);
				TextureFlags		|=	FLAGS_SHOW_TEXTURE;
				UpdateTexture();
			}


		}
		else
		{
			x_quad				=	(local_point.X&(TextureWidth-1))&~((TextureWidth-1)>>1) ? 1 : 0;
			y_quad				=	(local_point.Y&(TextureHeight-1))&~((TextureHeight-1)>>1) ? 1 : 0;
			CurrentTexture.U[0]	=	current_point.X+(x_quad*(TextureWidth-1));
			CurrentTexture.V[0]	=	current_point.Y+(y_quad*(TextureHeight-1));
			CurrentTexture.U[1]	=	current_point.X+((1^y_quad)*(TextureWidth-1));
			CurrentTexture.V[1]	=	current_point.Y+(x_quad*(TextureHeight-1));
			CurrentTexture.U[2]	=	current_point.X+(y_quad*(TextureWidth-1));
			CurrentTexture.V[2]	=	current_point.Y+((1^x_quad)*(TextureHeight-1));
			TextureFlags		|=	FLAGS_SHOW_TEXTURE;
			UpdateTexture();
		}
#ifndef	PAINT2	
		if(selected_face.PEle)
		{
			ApplyTexture(&selected_face);
			if(LockWorkScreen())
			{
				Parent->DrawContent();
				UnlockWorkScreen();
				ShowWorkWindow(0);
			}
		}
#endif
	}
	else
	{
		update	=	0;
		for(c0=0;c0<c1;c0++)
		{
			current_point	=	*clicked_point;
			current_point.X	-=	PaintRect.GetLeft()+2;
			current_point.Y	-=	PaintRect.GetTop()+2;
			GlobalToLocal(&current_point);
			if(ClickRect[c0].PointInRect(&current_point))
			{
				drag_flags	=	0;
				if(Keys[KB_LCONTROL] || Keys[KB_RCONTROL])
					drag_flags	=	DRAG_CONTROL;
				else if(Keys[KB_LSHIFT] || Keys[KB_RSHIFT])
					drag_flags	=	DRAG_SHIFT;
				update		=	1;

				MyUndo.MoveTexture(0,CurrentTexturePage,0,CurrentTexture.U[0],CurrentTexture.V[0],CurrentTexture.U[1],CurrentTexture.V[1],
									CurrentTexture.U[2],CurrentTexture.V[2],CurrentTexture.U[3],CurrentTexture.V[3]);

				while(SHELL_ACTIVE && LeftButton)
				{
					current_point.X	=	MouseX;
					current_point.Y	=	MouseY;
					GlobalToLocal(&current_point);

					zoom	=	256>>TextureZoom;
					current_point.X	=	(((current_point.X-(PaintRect.GetLeft()+2))>>(8-TextureZoom)));
					current_point.Y	=	(((current_point.Y-(PaintRect.GetTop()+2))>>(8-TextureZoom)));

					SET_TEXTURE_COORDS
					current_point.X	=	(current_point.X+texture_x);
					current_point.Y	=	(current_point.Y+texture_y);

					if(current_point.X!=local_point.X || current_point.Y!=local_point.Y)
					{
						x_offset	=	local_point.X-current_point.X;
						y_offset	=	local_point.Y-current_point.Y;
						if(selected_face.PEle==(struct EditMapElement*)-1 && face_is_in_list(selected_face.Face))
						{
							SLONG	p;
							SLONG	sx,sy;
							SLONG	flag=0;

							if(ShiftFlag)
								flag=1;

							if(selected_face.Face<0)
							{
								sx=prim_faces3[-selected_face.Face].UV[c0][0];
								sy=prim_faces3[-selected_face.Face].UV[c0][1];
							}
							else
							{
								sx=prim_faces4[selected_face.Face].UV[c0][0];
								sy=prim_faces4[selected_face.Face].UV[c0][1];
							}
							offset_selected_tmaps(-x_offset,-y_offset,CurrentTexturePage,sx,sy,flag);

							for(p=0;p<c1;p++)
							{
								if((CurrentTexture.U[p]==sx && CurrentTexture.V[p]==sy) || flag)
								{
									CurrentTexture.U[p]	-=	x_offset;
									CurrentTexture.V[p]	-=	y_offset;
								}
							}
						}
						else
						if(TextureFlags&FLAGS_QUADS)
						{
							switch(drag_flags)
							{
								case	DRAG_NORMAL:
									CurrentTexture.U[c0]	-=	x_offset;
									CurrentTexture.V[c0]	-=	y_offset;
									break;
								case	DRAG_SHIFT:
									if(abs(MouseX-clicked_point->X)>=abs(MouseY-clicked_point->Y))
										drag_flags	=	DRAG_SHIFT_H;
									else
										drag_flags	=	DRAG_SHIFT_V;
									x_offset=0;
									y_offset=0;
									break;
								case	DRAG_SHIFT_H:
									CurrentTexture.U[c0]	-=	x_offset;
									y_offset=0;
									break;
								case	DRAG_SHIFT_V:
									CurrentTexture.V[c0]	-=	y_offset;
									x_offset=0;
									break;
								case	DRAG_CONTROL:
									CurrentTexture.U[c0]	-=	x_offset;
									CurrentTexture.V[c0]	-=	y_offset;
									CurrentTexture.U[hooks[c0][0]]	-=	x_offset;
									CurrentTexture.V[hooks[c0][1]]	-=	y_offset;
									break;
							}


						}
						else
						{
							switch(drag_flags)
							{
								case	DRAG_NORMAL:
									CurrentTexture.U[c0]	-=	x_offset;
									CurrentTexture.V[c0]	-=	y_offset;
									break;
								case	DRAG_SHIFT_H:
									CurrentTexture.U[c0]	-=	x_offset;
									break;
								case	DRAG_SHIFT_V:
									CurrentTexture.V[c0]	-=	y_offset;
									break;
								case	DRAG_CONTROL:
									CurrentTexture.U[c0]	-=	x_offset;
									CurrentTexture.V[c0]	-=	y_offset;
									break;
							}
						}

						UpdateTexture();
						local_point	=	current_point;
//#ifndef	PAINT2	
//						if(selected_face.PEle)
						{
							ApplyTexture(&selected_face);
							if(LockWorkScreen())
							{
								Parent->DrawContent();
								UnlockWorkScreen();
								ShowWorkWindow(0);
							}
						}
//#endif
					}
				}
				break;
			}
		}
		if(!update)
		{
			if	(
					(
						TextureFlags&FLAGS_QUADS
						&&
						check_big_point_triangle(
													MouseX-PaintRect.GetLeft(),MouseY-PaintRect.GetTop(),
													ClickRect[1].GetLeft(),ClickRect[1].GetTop(),
													ClickRect[2].GetLeft(),ClickRect[2].GetTop(),
													ClickRect[3].GetLeft(),ClickRect[3].GetTop()
												)
					)
					||
					check_big_point_triangle(
												MouseX-PaintRect.GetLeft(),MouseY-PaintRect.GetTop(),
												ClickRect[2].GetLeft(),ClickRect[2].GetTop(),
												ClickRect[1].GetLeft(),ClickRect[1].GetTop(),
												ClickRect[0].GetLeft(),ClickRect[0].GetTop()
											)
				)
			{
				drag_flags		=	DRAG_NORMAL;
				if(Keys[KB_LSHIFT] || Keys[KB_RSHIFT])
				{
					drag_flags		=	DRAG_SHIFT;
				}
				angle			=	0;
				scale			=	1;
				update			=	0;
				temp_texture	=	CurrentTexture;
				x_offset		=	0;
				y_offset		=	0;
				for(c0=0;c0<c1;c0++)
				{
					x_offset	+=	temp_texture.U[c0];
					y_offset	+=	temp_texture.V[c0];
				}
				x_offset	/=	c1;
				y_offset	/=	c1;
				for(c0=0;c0<c1;c0++)
				{
					temp_texture.U[c0]	-=	x_offset;
					temp_texture.V[c0]	-=	y_offset;
				}
				MyUndo.MoveTexture(0,CurrentTexturePage,0,CurrentTexture.U[0],CurrentTexture.V[0],CurrentTexture.U[1],CurrentTexture.V[1],
									CurrentTexture.U[2],CurrentTexture.V[2],CurrentTexture.U[3],CurrentTexture.V[3]);
				while(SHELL_ACTIVE && LeftButton)
				{
					current_point.X	=	MouseX;
					current_point.Y	=	MouseY;
					GlobalToLocal(&current_point);
					zoom	=	256>>TextureZoom;
					current_point.X	=	(((current_point.X-(PaintRect.GetLeft()+2))>>(8-TextureZoom)));
					current_point.Y	=	(((current_point.Y-(PaintRect.GetTop()+2))>>(8-TextureZoom)));

					SET_TEXTURE_COORDS
					current_point.X	=	(current_point.X+texture_x);
					current_point.Y	=	(current_point.Y+texture_y);

					if(current_point.X!=local_point.X || current_point.Y!=local_point.Y)
					{
						switch(drag_flags)
						{
							case	DRAG_NORMAL:
								x_offset	-=	local_point.X-current_point.X;
								y_offset	-=	local_point.Y-current_point.Y;
								break;
							case	DRAG_SHIFT:
								if(abs(MouseX-clicked_point->X)>=abs(MouseY-clicked_point->Y))
									drag_flags	=	DRAG_SHIFT_H;
								else
									drag_flags	=	DRAG_SHIFT_V;
								break;
							case	DRAG_SHIFT_H:
								x_offset	-=	local_point.X-current_point.X;
								break;
							case	DRAG_SHIFT_V:
								y_offset	-=	local_point.Y-current_point.Y;
								break;
						}

						update	=	1;
						local_point	=	current_point;
					}
					if(Keys[KB_LEFT])
					{
						angle	=	(angle-3)&2047;
						update	=	1;
					}
					else if(Keys[KB_RIGHT])
					{
						angle	=	(angle+3)&2047;
						update	=	1;
					}
					if(Keys[KB_UP])
					{
						if(scale<100)
							scale++;
					}
					else if(Keys[KB_DOWN])
					{
						if(scale>1)
							scale--;
					}
					if(update)
					{
						for(c0=0;c0<c1;c0++)
						{
							CurrentTexture.U[c0]	=	x_offset+((((temp_texture.U[c0]*COS(angle))-(temp_texture.V[c0]*SIN(angle)))*scale)>>16);
							CurrentTexture.V[c0]	=	y_offset+((((temp_texture.U[c0]*SIN(angle))+(temp_texture.V[c0]*COS(angle)))*scale)>>16);
						}

						UpdateTexture();
#ifndef	PAINT2	
						if(selected_face.PEle)
						{
							ApplyTexture(&selected_face);
							if(LockWorkScreen())
							{
								Parent->DrawContent();
								UnlockWorkScreen();
								ShowWorkWindow(0);
							}
						}
#endif
						update	=	0;
					}
				}
			}
		}
	}
}

//---------------------------------------------------------------

UWORD	PaintTab::ConvertFreeToFixedEle(struct TextureBits *t,SLONG *x,SLONG *y,SLONG *width,SLONG *height,SLONG *page)
{
	SLONG	size;
	t->X=CurrentTexture.U[0]>>3;
	t->Y=CurrentTexture.V[0]>>3;

	t->Page=CurrentTexturePage;
	switch(TextureWidth)
	{
		case	8:
			size=0;
			break;
		case	16:
			size=1;
			break;
		case	32:
			size=2;
			break;
		case	64:
			size=3;
			break;
		case	96:
			size=4;
			break;
		case	128:
			size=5;
			break;
		case	160:
			size=6;
			break;
		case	192:
			size=7;
			break;
	}
	size=32;
	t->Width=size;

	switch(TextureHeight)
	{
		case	8:
			size=0;
			break;
		case	16:
			size=1;
			break;
		case	32:
			size=2;
			break;
		case	64:
			size=3;
			break;
		case	96:
			size=4;
			break;
		case	128:
			size=5;
			break;
		case	160:
			size=6;
			break;
		case	192:
			size=7;
			break;
	}
	size=32;
	t->Height=size;
	*x=t->X;
	*y=t->Y;
	*width=t->Width;
	*height=t->Height;
	*page=t->Page;

	return(1);	
}

void	PaintTab::ConvertFixedToFree(struct TextureBits *t)
{
	UBYTE	sx,sy,w,h;

	sx=t->X<<3;
	sy=t->Y<<3;
	w=texture_sizes[t->Width];
	h=texture_sizes[t->Height];

	CurrentTexture.U[0]=sx;
	CurrentTexture.V[0]=sy;
	CurrentTexture.U[1]=sx+w;
	CurrentTexture.V[1]=sy;
	CurrentTexture.U[2]=sx;
	CurrentTexture.V[2]=sy+h;
	CurrentTexture.U[3]=sx+w;
	CurrentTexture.V[3]=sy+h;
	TextureWidth=32; //w;
	TextureHeight=32; //w;

}

void	PaintTab::ConvertMiniTex(struct	MiniTextureBits	*tex)
{
	SLONG	sx,sy,w;
	SLONG	rot;

	sx=tex->X<<5;
	sy=tex->Y<<5;
	rot=tex->Rot;
	w=32;//floor_texture_sizes[tex->Size];
	CurrentTextureRot=tex->Rot;
	CurrentTexturePage=tex->Page;

	CurrentTexture.U[0]=sx;
	CurrentTexture.V[0]=sy;
	CurrentTexture.U[1]=sx+w;
	CurrentTexture.V[1]=sy;
	CurrentTexture.U[2]=sx;
	CurrentTexture.V[2]=sy+w;
	CurrentTexture.U[3]=sx+w;
	CurrentTexture.V[3]=sy+w;

	for(;rot>0;rot--)
	{
		SLONG	temp_u,temp_v;
		temp_u	=	CurrentTexture.U[0];
		temp_v	=	CurrentTexture.V[0];
		CurrentTexture.U[0]	=	CurrentTexture.U[1];
		CurrentTexture.V[0]	=	CurrentTexture.V[1];
		CurrentTexture.U[1]	=	CurrentTexture.U[3];
		CurrentTexture.V[1]	=	CurrentTexture.V[3];
		CurrentTexture.U[3]	=	CurrentTexture.U[2];
		CurrentTexture.V[3]	=	CurrentTexture.V[2];
		CurrentTexture.U[2]	=	temp_u;
		CurrentTexture.V[2]	=	temp_v;
		
	}

	TextureWidth=w;
	TextureHeight=w;

	
}
UWORD	PaintTab::ConvertTexToMiniTex(void)
{
	UBYTE x1, y1, x2, y2, x3, y3, x4, y4, page;
	struct	MiniTextureBits	tex;

	tex.X=(CurrentTexture.U[0]+CurrentTexture.U[1]+CurrentTexture.U[2]+CurrentTexture.U[3])>>7;
	tex.Y=(CurrentTexture.V[0]+CurrentTexture.V[1]+CurrentTexture.V[2]+CurrentTexture.V[3])>>7;
	tex.Page=CurrentTexturePage;
	tex.Size=0;
	tex.Rot=CurrentTextureRot;
	return(*((UWORD*)&tex));
}

BOOL	PaintTab::ApplyTexture(struct EditFace *edit_face)
{
	SLONG		c0;


	if(edit_face->PEle==(struct EditMapElement*)-2)
	{
		
		if(PaintMode==PALETTE_PAINT)
		{
			
		}
		else
		{
			edit_map[edit_face->MapX][edit_face->MapZ].Texture=ConvertTexToMiniTex();
		}
	}
	else
	if(edit_face->PEle==(struct EditMapElement*)-1)
	{
		if(edit_face->Face<0)
		{
			MyUndo.ApplyPrim3(0,-edit_face->Face,&prim_faces3[-edit_face->Face]);
		}
		else
		{
			MyUndo.ApplyPrim4(0,edit_face->Face,&prim_faces4[edit_face->Face]);
		}
		if(PaintMode==PALETTE_PAINT)
		{
			if(edit_face->Face<0)
			{
				prim_faces3[-edit_face->Face].DrawFlags		&=	~(POLY_FLAG_TEXTURED);
				prim_faces3[-edit_face->Face].Col2	=	CurrentColour;
			}
			else
			{
				prim_faces4[edit_face->Face].DrawFlags		&=	~(POLY_FLAG_TEXTURED);
				prim_faces4[edit_face->Face].Col2		=	CurrentColour;
			}
		}
		else
		{
			if(edit_face->Face<0)
			{
				prim_faces3[-edit_face->Face].DrawFlags		|=	POLY_FLAG_TEXTURED;
				prim_faces3[-edit_face->Face].TexturePage	=	(UWORD)CurrentTexturePage;
				for(c0=0;c0<3;c0++)
				{
					prim_faces3[-edit_face->Face].UV[c0][0]		=	CurrentTexture.U[c0];
					prim_faces3[-edit_face->Face].UV[c0][1]		=	CurrentTexture.V[c0];
				}
			}
			else
			{
				prim_faces4[edit_face->Face].DrawFlags		|=	POLY_FLAG_TEXTURED;
				prim_faces4[edit_face->Face].TexturePage	=	(UWORD)CurrentTexturePage;
				for(c0=0;c0<4;c0++)
				{
					prim_faces4[edit_face->Face].UV[c0][0]		=	CurrentTexture.U[c0];
					prim_faces4[edit_face->Face].UV[c0][1]		=	CurrentTexture.V[c0];
				}
				if(prim_faces4[edit_face->Face].ThingIndex<0)
				{
extern	void	set_wall_texture_info(SLONG	wall,UBYTE page,EdTexture	*current_texture,UBYTE type,UBYTE side);
					UBYTE	type=0;
					if(PaintMode==STYLE_PAINT)
					{
						type=CurrentStyleEdit;
						set_wall_texture_info(-prim_faces4[edit_face->Face].ThingIndex,(SBYTE)CurrentTexturePage,&CurrentTexture,type,(prim_faces4[edit_face->Face].FaceFlags&FACE_FLAG_TEX2)?1:0);
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
							x+=CurrentTexture.U[c0];
							y+=CurrentTexture.V[c0];
						}
						x>>=7;
						y>>=7;

						t=x+y*8+CurrentTexturePage*64;
						apply_texture_to_wall_face(edit_face->Face,t);

					}

				}
			}
		}
		return	1;
	}
	else if(edit_face->PEle)
	{
		SLONG x,y,width,height,page;
//		ConvertFreeToFixedEle(&edit_face->PEle->Textures[edit_face->Face]);
		ConvertFreeToFixedEle(&edit_face->PEle->Textures[edit_face->Face],&x,&y,&width,&height,&page);
		return	1;
	}
	return	0;
}


void	paint_texture_to_wall(SLONG wall,SLONG pos,SLONG texture)
{
	ASSERT(pos<200);
		
	if(wall_list[wall].Tcount<=pos)
	{
		SLONG	size;
		UBYTE	*old=0;
		if(wall_list[wall].Textures&&wall_list[wall].Tcount)
			old=wall_list[wall].Textures;

		size=pos+1;
		if(size<8)
			size=8;
		wall_list[wall].Textures=(UBYTE*)MemAlloc(size+1);
		if(wall_list[wall].Textures==0)
		{
			wall_list[wall].Tcount=0;
			return;
		}
		if(old && wall_list[wall].Tcount)
		{
			memcpy(wall_list[wall].Textures,old,wall_list[wall].Tcount+1);
			MemFree(old);
		}
		wall_list[wall].Tcount=size;
	
	}
	wall_list[wall].Textures[pos]=texture;
}
void	paint_texture_to_wall2(SLONG wall,SLONG pos,SLONG texture)
{
	ASSERT(pos<200);
		
	if(wall_list[wall].Tcount2<=pos)
	{
		SLONG	size;
		UBYTE	*old=0;
		if(wall_list[wall].Textures2 && wall_list[wall].Tcount2)
			old=wall_list[wall].Textures2;

		size=pos+1;
		if(size<8)
			size=8;
		wall_list[wall].Textures2=(UBYTE*)MemAlloc(size+1);
		if(wall_list[wall].Textures2==0)
		{
			wall_list[wall].Tcount2=0;
			return;
		}
		if(old && wall_list[wall].Tcount2)
		{
			memcpy(wall_list[wall].Textures2,old,wall_list[wall].Tcount2+1);
			MemFree(old);
		}
		wall_list[wall].Tcount2=size;
	
	}
	wall_list[wall].Textures2[pos]=texture;
}

void	remove_painted_textures(void)
{
	SLONG	c0;
	for(c0=0;c0<MAX_WALLS;c0++)
	{
		if(wall_list[c0].Textures&&wall_list[c0].Tcount)
		{
			MemFree(wall_list[c0].Textures);
			wall_list[c0].Textures=0;
			wall_list[c0].Tcount=0;
		}

		if(wall_list[c0].Textures2&&wall_list[c0].Tcount2)
		{
			MemFree(wall_list[c0].Textures);
			wall_list[c0].Textures2=0;
			wall_list[c0].Tcount2=0;
		}

	}
}
void	remove_style_textures(void)
{
	SLONG	c0;
	for(c0=0;c0<MAX_WALLS;c0++)
	{
			wall_list[c0].TextureStyle=0;
			wall_list[c0].TextureStyle2=0;
	}
}

void	paint_texture_to_storey(SLONG storey,SLONG pos,SLONG texture)
{
	ASSERT(pos<100);
/*		
	if(storey_list[storey].Tcount<=pos)
	{
		SLONG	size;
		if(storey_list[storey].Textures&&storey_list[storey].Tcount)
			MemFree(storey_list[storey].Textures);

		size=pos;
		if(size<8)
			size=8;
		storey_list[storey].Textures=(UBYTE*)MemAlloc(size);
		if(storey_list[storey].Textures==0)
		{
			storey_list[storey].Tcount=0;
			return;
		}
		storey_list[storey].Tcount=size;
	
	}
	storey_list[storey].Textures[pos]=texture;
*/
}

void	apply_texture_to_wall_face(SLONG face,SLONG texture)
{
	SLONG	wall,storey,building;
	SLONG	c0;
	SLONG	mx=0,my=0,mz=0;
	SLONG	head;
	SLONG	x1,z1;
	SLONG	dx,dz;
	SLONG	dist;


	// quads only

	if(face<0)
	{
		return;
	}


	wall=-prim_faces4[face].ThingIndex;
	if(wall<=0)
		return;

	for(c0=0;c0<4;c0++)
	{
		mx+=prim_points[prim_faces4[face].Points[c0]].X;
		my+=prim_points[prim_faces4[face].Points[c0]].Y;
		mz+=prim_points[prim_faces4[face].Points[c0]].Z;
	}

//	ASSERT(wall!=13);

	mx>>=2;
	my>>=2;
	mz>>=2;

	if (wall > 0)
	{
		struct	MapThing	*p_mthing;
		SLONG	thing;

		storey   = wall_list[wall].StoreyHead;
		building = storey_list[storey].BuildingHead;
		thing    = building_list[building].ThingIndex;
		p_mthing=TO_MTHING(thing);

		mx+=p_mthing->X;
		my+=p_mthing->Y;
		mz+=p_mthing->Z;


	}

	//
	// find the start point for this wall, storey or another wall
	//
	storey=wall_list[wall].StoreyHead;
	if(storey_list[storey].WallHead==wall)
	{
		head=-storey;
		x1=storey_list[storey].DX;
		z1=storey_list[storey].DZ;
	}
	else
	{
		SLONG	index;
		index=storey_list[storey].WallHead;
		while(index)
		{
			if(wall_list[index].Next==wall)
			{
				head=index;
				x1=wall_list[index].DX;
				z1=wall_list[index].DZ;
				break;
			}

			index=wall_list[index].Next;
		}

	}

	ASSERT(head);

	dx=mx-x1;
	dz=mz-z1;

	dist=Root(dx*dx+dz*dz);
	dist>>=8;

	//
	// dist is now the face number along this facet to paint
	//
/*
	if(head<0)
	{
		paint_texture_to_storey(wall,dist,texture);
	}
	else
	*/
	{
		
		if(prim_faces4[face].FaceFlags&FACE_FLAG_TEX2)
		{
			// this wall is double sided and so uses different texture info
			paint_texture_to_wall2(wall,dist,texture);
		}
		else
		{
			paint_texture_to_wall(wall,dist,texture);
		}
	}



}
//---------------------------------------------------------------
