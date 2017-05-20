#include	"Editor.hpp"


#include	"SewerTab.hpp"
#include	"engine.h"
#include	"c:\fallen\headers\enter.h"
#include	"c:\fallen\headers\id.h"
#include	"extra.h"
#include	"c:\fallen\headers\supermap.h"
#include	"c:\fallen\headers\inside2.h"
#include	"c:\fallen\headers\memory.h"

extern	void	draw_quad_now(SLONG x,SLONG y,SLONG w,SLONG h,UBYTE tx,UBYTE ty,UBYTE page,UBYTE flip,UBYTE flags);


//#pragma	warning	14 9

//static		counter;
//#define		ShowWorkWindow(x)	{DrawLineC(0+(counter-1)&255,0,WorkWindowWidth-1,WorkWindowHeight-1,0);DrawLineC(0+(counter++)&255,0,WorkWindowWidth-1,WorkWindowHeight-1,255);DrawLineC(0,WorkWindowHeight-1,WorkWindowWidth-1,0,255); ShowWorkWindow(x);}

//---------------------------------------------------------------
//debug stuff
/*
void	cross_work_window(void)
{
	DrawLineC(0,0,WorkWindowWidth-1,WorkWindowHeight-1,255);
	DrawLineC(0,WorkWindowHeight-1,WorkWindowWidth-1,0,255);
	
}
*/
//---------------------------------------------------------------

struct EnterableBuilding
{
	UWORD	GroundStorey;
};

struct	EnterStorey
{
	 UWORD	NextStorey;
	 
};



#define	CTRL_BUILD_NEW_WALLS			1
#define	CTRL_BUILD_NEW_DOOR				2
#define	CTRL_BUILD_NEXT_STOREY			3
#define	CTRL_BUILD_PREV_STOREY			4
#define	CTRL_BUILD_DEL_STOREY			5
#define	CTRL_INSTYLE_MENU				6
#define	CTRL_PLACE_STAIRS				23
#define	CTRL_DELETE_DUPLICATE_INSIDES	24

#define	CTRL_BUILD_Y_AXIS_FREE			2
#define	CTRL_BUILD_Z_AXIS_FREE			3
#define	CTRL_BUILD_NEW_BUILDING			4
//#define	CTRL_BUILD_NEXT_STOREY			5
#define	CTRL_TOGGLE_TILED_ROOF			7
#define	CTRL_ADD_FLAT_ROOF_QUAD			8
#define	CTRL_ADD_FIRE_ESCAPE			9
#define	CTRL_ADD_POKEY					10
#define	CTRL_DELETE_STOREY				11
#define	CTRL_ADD_STAIRCASE				12
#define	CTRL_ADD_SKYLIGHT				13
#define	CTRL_ADD_CABLE					14
#define	CTRL_BUILD_CREATE_BUILDING		15
#define	CTRL_ADD_LADDER					16
#define	CTRL_DELETE_BUILDING			17
#define	CTRL_NEW_FENCE					18
#define CTRL_ANOTHER_INSIDE_SEED		19
#define CTRL_PREV_INSIDE_SEED			20
#define CTRL_ANOTHER_STAIRCASE_SEED		21
#define CTRL_PREV_STAIRCASE_SEED		22
#define CTRL_NEXT_STOREY				23
#define CTRL_PREV_STOREY				24
#define CTRL_BUILDING_TYPE_HOUSE		25
#define CTRL_BUILDING_TYPE_WAREHOUSE	26
#define CTRL_BUILDING_TYPE_OFFICE		27
#define CTRL_BUILDING_TYPE_APARTEMENT	28
#define CTRL_BUILDING_TYPE_CRATE_IN		29
#define CTRL_BUILDING_TYPE_CRATE_OUT	30
#define	CTRL_ADD_TRENCH					31
#define	CTRL_SET_STOREY_HEIGHT_64		32
#define	CTRL_SET_STOREY_HEIGHT_128		33
#define	CTRL_SET_STOREY_HEIGHT_196		34
#define	CTRL_SET_STOREY_HEIGHT_256		35
#define	CTRL_TOGGLE_FLAT_TILED_ROOF		36

#define	BUILD_MODE_WAIT				1
#define	BUILD_MODE_PLACE_STOREY		2

#define	BUILD_MODE_CONT_STOREY		3
#define	BUILD_MODE_EDIT_STOREY		4

#define	BUILD_MODE_PLACE_ROOM		5
#define	BUILD_MODE_PLACE_STAIRS		6

MenuDef2		style_menu[64];



ControlDef	inside_tab_def[]	=	
{
	{	BUTTON,		0,	"New Walls",				10,		 40,	0,	0		},
	{	BUTTON,		0,	"New Door",					10,		 60,	0,	0		},
	{	BUTTON,		0,	"Next Storey",				10,		 160,	0,	0		},
	{	BUTTON,		0,	"Prev Storey",				10,		 180,	0,	0		},
	{	BUTTON,		0,	"Delete Storey",			180,	 80,	0,	0		},
	{	PULLDOWN_MENU,	0,"Inside Style",			140,	4,		0,	0,	style_menu		},
	{	BUTTON,		0,	"1",				10,		 380,	0,	0		},
	{	BUTTON,		0,	"2",				10+35*1,		 380,	0,	0		},
	{	BUTTON,		0,	"3",				10+35*2,		 380,	0,	0		},
	{	BUTTON,		0,	"4",				10+35*3,		 380,	0,	0		},
	{	BUTTON,		0,	"5",				10+35*4,		 380,	0,	0		},
	{	BUTTON,		0,	"6",				10+35*5,		 380,	0,	0		},
	{	BUTTON,		0,	"7",				10+35*6,		 380,	0,	0		},
	{	BUTTON,		0,	"8",				10+35*7,		 380,	0,	0		},

	{	BUTTON,		0,	"9",				10,		 440,	0,	0		},
	{	BUTTON,		0,	"10",				10+35*1,		 440,	0,	0		},
	{	BUTTON,		0,	"11",				10+35*2,		 440,	0,	0		},
	{	BUTTON,		0,	"12",				10+35*3,		 440,	0,	0		},
	{	BUTTON,		0,	"13",				10+35*4,		 440,	0,	0		},
	{	BUTTON,		0,	"14",				10+35*5,		 440,	0,	0		},
	{	BUTTON,		0,	"15",				10+35*6,		 440,	0,	0		},
	{	BUTTON,		0,	"16",				10+35*7,		 440,	0,	0		},
	{	BUTTON,		0,	"Place Stairs",		10,		 100,	0,	0		},
	{	BUTTON,		0,	"Delete Duplicate Insides",		10,		 250,	0,	0		},
/*
	{	CHECK_BOX,	0,	"Textures",					180,	200-50,	0,	10		},
	{	CHECK_BOX,	0,	"Y Free",					120,	213-10,	0,	10		},
	{	CHECK_BOX,	0,	"Z Free",					120,	226-10,	0,	10		},
	{	BUTTON,		0,	"New Building",				10,		 40,	0,	0		},
	{	BUTTON,		0,	"Next Storey",				10,		 80,	0,	0		},		 
	{	BUTTON,		0,	"Duplicate Storey",			10,		100,	0,	0		},
	{	BUTTON,		0,	"Toggle Tiled Roof",		10,		120,	0,	0		},
	{	BUTTON,		0,	"Add Flat Roof Quad",		10,		140,	0,	0		},
	{	BUTTON,		0,	"Add a FireEscape",			10,		160,	0,	0		},
	{	BUTTON,		0,	"Add a Ledge to Storey",	10,		180,	0,	0		},
	{	BUTTON,		0,	"Delete Storey",			180,	 80,	0,	0		},
	{	BUTTON,		0,	"Add a StairCase",			10,		200,	0,	0		},
	{	BUTTON,		0,	"Add a SkyLight",			10,		220,	0,	0		},
	{	BUTTON,		0,	"Add cable",				10,		240,	0,	0		},
	{	BUTTON,		0,	"Create Building",			10,		260,	0,	0		},
	{	BUTTON,		0,	"Add Ladder",				10,		280,	0,	0		},
	{	BUTTON,		0,	"Delete Building",			180,	100,	0,	0		},
	{	BUTTON,		0,	"New Fence",				10,		 60,	0,	0		},
	{	BUTTON,		0,	"Another  inside seed",		180,	180,	0,	0		},
	{	BUTTON,		0,	"Previous inside seed",		180,	200,	0,	0		},
	{	BUTTON,		0,	"Another  staircase seed",	180,	220,	0,	0		},
	{	BUTTON,		0,	"Previous staircase seed",	180,	240,	0,	0		},
	{	BUTTON,		0,	"Edit Next Storey",			180,	320,	0,	0		},
	{	BUTTON,		0,	"Edit Prev Storey",			180,	340,	0,	0		},
	{	CHECK_BOX,	0,	"House",					10,		320,	0,	10		},
	{	CHECK_BOX,	0,	"Warehouse",				10,		335,	0,	10		},
	{	CHECK_BOX,	0,	"Office",					10,		350,	0,	10		},
	{	CHECK_BOX,	0,	"Apartement",				10,		365,	0,	10		},
	{	CHECK_BOX,	0,	"Crate Inside",				10,		380,	0,	10		},
	{	CHECK_BOX,	0,	"Crate Outside",			10,		395,	0,	10		},
	{	BUTTON,		0,	"Add a Trench",				180,	160,	0,	0		},
	{	BUTTON,		0,	"Storey Height quart",			180,	360,	0,	0		},
	{	BUTTON,		0,	"Storey Height half",			180,	375,	0,	0		},
	{	BUTTON,		0,	"Storey Height 3quart",			180,	390,	0,	0		},
	{	BUTTON,		0,	"Storey Height normal",			180,	405,	0,	0		},
	{	BUTTON,		0,	"Toggle Flat Tiled Roof",		170,		120,	0,	0		},
*/
	{  	0	}
};

SewerTab *the_build;

extern	CBYTE	*storey_name[];

extern	SLONG inside_building;
extern	SLONG inside_storey;
extern	SLONG inside_valid;
extern	SLONG inside_failure;


#define MAX_SEED_BACKUPS 16

extern	SLONG seed_inside[MAX_SEED_BACKUPS];
extern	SLONG seed_stairs[MAX_SEED_BACKUPS];

extern	SLONG seed_inside_upto;
extern	SLONG seed_stairs_upto;


//
// The building the stairs have been calculated for.
// The storey the inside building stuff has been calculated for.
// TRUE => the inside stuff is valid.
//



#define MAX_SEED_BACKUPS 16


extern	CBYTE	inside_names[64][20];

CBYTE	end_str[]="!";


SewerTab::SewerTab(EditorModule *parent)
{
	SLONG	c0;
	Parent=parent;

	for(c0=0;c0<42;c0++)
		style_menu[c0].ItemText=&inside_names[c0][0];
	style_menu[c0].ItemText=&end_str[0];

	InitControlSet(inside_tab_def);
	AxisMode=3;
	CurrentFloorType=0;

//	SetControlState(CTRL_BUILD_Y_AXIS_FREE,CTRL_SELECTED);
//	SetControlState(CTRL_BUILD_Z_AXIS_FREE,CTRL_SELECTED);

	Axis=X_AXIS|Y_AXIS|Z_AXIS;
	Mode=0;
	ViewSize=16;
	ViewX=0+(64<<ELE_SHIFT);
	ViewY=0;
	ViewZ=0+(64<<ELE_SHIFT);
	GridFlag=0;
	Texture=0;
	EditBuilding=0;
	EditStorey=0;
	EditY=0;
	EditWall=0;
	OutsideEditStorey=0;
	OutsideEditWall=0;

	inside_building = 0;
	inside_storey   = 0;
	inside_valid    = FALSE;
	inside_failure  = FALSE;

	the_build=this;
	ResetSewerTab();
}

SewerTab::~SewerTab()
{
}


void	SewerTab::ResetSewerTab(void)
{
	SLONG	c0,storey;

	Mode=BUILD_MODE_WAIT;
	EditStorey=0;
	EditWall=0;
	EditWindow=0;
	EditY=0;

	for(c0=1;c0<MAX_BUILDINGS;c0++)
	{
		if(building_list[c0].BuildingFlags&1)
		{
			EditBuilding=c0;
			storey=building_list[c0].StoreyHead;
			if(storey)
			{
				ViewX=storey_list[storey].DX;
				ViewZ=storey_list[storey].DZ;
			}
		}
	}
	
}


void	SewerTab::Clear(void)
{
//	clear_all_col_info();
}

void	SewerTab::DrawTabContent(void)
{
	SLONG	pos;
	//
	// Make sure the building type buttons look proper...
	//
/*
	SetControlState(CTRL_BUILDING_TYPE_HOUSE,      CTRL_DESELECTED);
	SetControlState(CTRL_BUILDING_TYPE_WAREHOUSE,  CTRL_DESELECTED);
	SetControlState(CTRL_BUILDING_TYPE_OFFICE,     CTRL_DESELECTED);
	SetControlState(CTRL_BUILDING_TYPE_APARTEMENT, CTRL_DESELECTED);
	SetControlState(CTRL_BUILDING_TYPE_CRATE_IN,   CTRL_DESELECTED);
	SetControlState(CTRL_BUILDING_TYPE_CRATE_OUT,  CTRL_DESELECTED);

	if (WITHIN(EditBuilding, 1, MAX_BUILDINGS - 1))
	{
		switch(building_list[EditBuilding].BuildingType)
		{
			case BUILDING_TYPE_HOUSE:		SetControlState(CTRL_BUILDING_TYPE_HOUSE,       CTRL_SELECTED); break;
			case BUILDING_TYPE_WAREHOUSE:	SetControlState(CTRL_BUILDING_TYPE_WAREHOUSE,   CTRL_SELECTED); break;
			case BUILDING_TYPE_OFFICE:		SetControlState(CTRL_BUILDING_TYPE_OFFICE,      CTRL_SELECTED); break;
			case BUILDING_TYPE_APARTEMENT:	SetControlState(CTRL_BUILDING_TYPE_APARTEMENT,	CTRL_SELECTED); break;
			case BUILDING_TYPE_CRATE_IN:	SetControlState(CTRL_BUILDING_TYPE_CRATE_IN,    CTRL_SELECTED); break;
			case BUILDING_TYPE_CRATE_OUT:	SetControlState(CTRL_BUILDING_TYPE_CRATE_OUT,   CTRL_SELECTED); break;

			default:
				ASSERT(0);
				break;
		}
	}
*/

	EdRect		content_rect;

	content_rect	=	ContentRect;	
	content_rect.ShrinkRect(1,1);
	content_rect.FillRect(CONTENT_COL);

	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
	DrawControlSet();


		for(pos=0;pos<16;pos++)
		{
			SLONG	x,y,page,flip,flags,value;
//			draw_quad_now(200+pos*38,c0*20+30,16,16,textures_xy[c0+scroll_pos][pos].Tx<<5,textures_xy[c0+scroll_pos][pos].Ty<<5,textures_xy[c0+scroll_pos][pos].Page);
			value=inside_tex[CurrentFloorType][pos];

			page=value/64+START_PAGE_FOR_FLOOR;
			value=value&63;
			x=(value&7)<<5;
			y=(value&(7<<3))<<2;

//			draw_quad_now(120+pos*20,c0*26+30-2,24,24,textures_xy[c0+scroll_pos][pos].Tx<<5,textures_xy[c0+scroll_pos][pos].Ty<<5,textures_xy[c0+scroll_pos][pos].Page,textures_xy[c0+scroll_pos][pos].Flip,textures_flags[c0+scroll_pos][pos]);
			draw_quad_now(15+(pos&7)*35,390+( (pos>7)?60:0),30,30,x,y,page,0,POLY_FLAG_TEXTURED);
		}



	ShowWorkWindow(0);
}




//---------------------------------------------------------------
extern	void	hilight_col_info(void);

void	SewerTab::AddHeightOffset(SLONG *x,SLONG *y)
{
	if(Texture&(6))
		return;
//	*x-=((EditY-CurrentY)*ViewSize)/(BLOCK_SIZE<<4);
//	*y-=-((EditY-CurrentY)*ViewSize)/(BLOCK_SIZE<<4);

	*x-=((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
	*y-=-((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
	
}


SLONG	SewerTab::GetHeightColour(SLONG	storey)
{
//	if(Texture==2)
//		return(WHITE_COL);
	if(storey==OutsideEditStorey)
		return(RED_COL);
	if(storey==EditStorey)
		return(GREEN_COL);



		return(0);
	
}

void	SewerTab::HighlightVertexes(SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG	mx,mz,rect_size;
	EdRect	rect;
	SLONG	roof_flag=0;
	SLONG	building;

	mx=ViewX>>ELE_SHIFT;
	mz=ViewZ>>ELE_SHIFT;

	rect_size=ViewSize>>2;

	if(Mode==BUILD_MODE_WAIT)
	{
//		for(building=1;building<MAX_BUILDINGS;building++)
		{

			SLONG	x1,y1,z1,x2,y2,z2,index;
			MFPoint		mouse_point;
			SLONG	storey_index,wall;
			CBYTE	str[100];
			SLONG	ploty=20,c0;

			storey_index=storey_list[OutsideEditStorey].InsideStorey; //building_list[building].StoreyHead;

			while(storey_index)
			{
																				  
				
				x1=storey_list[storey_index].DX;
				y1=storey_list[storey_index].DY;
				z1=storey_list[storey_index].DZ;

				


				CurrentY=storey_list[storey_index].DY;
				index=storey_list[storey_index].WallHead;

				x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
				z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);
				AddHeightOffset(&x1,&z1);
				if(index)
				{
					rect.SetRect(x1-rect_size,z1-rect_size,rect_size<<1,rect_size<<1);
					rect.OutlineRect(GetHeightColour(storey_index));

					while(index)
					{
						x1=wall_list[index].DX;
						z1=wall_list[index].DZ;

						x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
						z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);

						AddHeightOffset(&x1,&z1);
						rect.SetRect(x1-rect_size,z1-rect_size,rect_size<<1,rect_size<<1);
						rect.OutlineRect(GetHeightColour(storey_index));

						index=wall_list[index].Next;	
					}
				}

				storey_index=storey_list[storey_index].Next;
			}
		}
	}
}



SLONG	SewerTab::ClickInVertexStoreyList(SLONG building,SLONG storey_index,SLONG w,SLONG h,MFPoint	*mouse_point)
{
	SLONG	roof_flag=0;
	EdRect	rect;
	SLONG	mx,mz,rect_size;
	SLONG	x1,y1,z1,x2,y2,z2,index;

	SLONG	ret_building=0,ret_storey,ret_wall,wall;

	mx=ViewX>>ELE_SHIFT;
	mz=ViewZ>>ELE_SHIFT;

	rect_size=ViewSize>>2;
	while(storey_index>0)
	{
		


		x1=storey_list[storey_index].DX;
		z1=storey_list[storey_index].DZ;

		CurrentY=storey_list[storey_index].DY;
		index=storey_list[storey_index].WallHead;

//		x1=((x1>>ELE_SHIFT)-mx)*ViewSize+(w>>1);
//		z1=((z1>>ELE_SHIFT)-mz)*ViewSize+(h>>1);

		x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(w>>1);
		z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(h>>1);
		AddHeightOffset(&x1,&z1);
		if(index)
		{
			rect.SetRect(x1-rect_size,z1-rect_size,rect_size<<1,rect_size<<1);
			if(rect.PointInRect(mouse_point))
			{
				if(storey_index==EditStorey)
				{
					
					EditBuilding=storey_list[storey_index].BuildingHead;
					EditStorey=storey_index;
					EditY=0;//storey_list[EditStorey].DY;
					EditWall=0;
					return(2);
				}
				else
				{
					ret_building=storey_list[storey_index].BuildingHead;
					ret_storey=storey_index;
					ret_wall=0;
				}
			}


			while(index)
			{
				x1=wall_list[index].DX;
				z1=wall_list[index].DZ;

//				x1=((x1>>ELE_SHIFT)-mx)*ViewSize+(w>>1);
//				z1=((z1>>ELE_SHIFT)-mz)*ViewSize+(h>>1);
				x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(w>>1);
				z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(h>>1);
				AddHeightOffset(&x1,&z1);

				rect.SetRect(x1-rect_size,z1-rect_size,rect_size<<1,rect_size<<1);
				if(rect.PointInRect(mouse_point))
				{
					if(storey_index==EditStorey)
					{
						EditBuilding=storey_list[storey_index].BuildingHead;
						EditStorey=storey_index;
						EditY=0;//storey_list[EditStorey].DY;
						EditWall=index;
						return(2);
					}
					else
					{
						ret_building=storey_list[storey_index].BuildingHead;
						ret_storey=storey_index;
						ret_wall=index;
					}
				}

				index=wall_list[index].Next;	
			}
		}
		storey_index=storey_list[storey_index].Next;
	}

	if(ret_building)
	{
		EditBuilding=ret_building;
		EditStorey=ret_storey;
		EditY=0;//storey_list[EditStorey].DY;
		EditWall=ret_wall;

		DrawTabContent();

		return(1);
		
	}
	return(0);
	
}

SLONG	SewerTab::ClickInVertex(SLONG x,SLONG y,SLONG w,SLONG h,MFPoint	*mouse_point)
{
	SLONG	storey_index,found;
	SLONG	found_one=0;

	SLONG	c0;

	storey_index=OutsideEditStorey; //storey_list[OutsideEditStorey].StoreyHead;

	if(storey_list[storey_index].InsideIDIndex)
	{
		CBYTE	str[10];
		SLONG	index;
		SLONG	c0;
		EdRect	rect;

		index=storey_list[storey_index].InsideIDIndex;
		for(c0=0;c0<MAX_STAIRS_PER_FLOOR;c0++)
		{
			
			if(room_ids[index].StairFlags[c0])
			{
				SLONG	dir;
				SLONG	dx,dz;
				SLONG	x1,z1,x2,z2;
				EdRect	drawrect;

				dir=GET_STAIR_DIR(room_ids[index].StairFlags[c0]);
				switch(dir)
				{
					case	0:
						//north
						dx=256;
						dz=-512;
						break;
					case	1:
						//east
						dx=512;
						dz=256;
						break;
					case	2:
						//south
						dx=-256;
						dz=512;
						break;
					case	3:
						//west
						dx=-512;
						dz=-256;
						break;
				}
				x1=	room_ids[index].StairsX[c0]<<8;
				z1=	room_ids[index].StairsY[c0]<<8;

				if(dx<0)
				{
					x2=x1;
					x1=x1+dx;
				}
				else
				{
					x2=x1+dx;
				}
				if(dz<0)
				{
					z2=z1;
					z1=z1+dz;
				}
				else
				{
					z2=z1+dz;
				}

				x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(w>>1); //(WorkWindowRect.Width>>1);
				z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(h>>1); //(WorkWindowRect.Height>>1);

				x2=((((x2)-(ViewX))*ViewSize)/ELE_SIZE)+(w>>1); //(WorkWindowRect.Width>>1);
				z2=((((z2)-(ViewZ))*ViewSize)/ELE_SIZE)+(h>>1); //(WorkWindowRect.Height>>1);

					CurrentY=storey_list[OutsideEditStorey].DY;
					AddHeightOffset(&x1,&z1);
					AddHeightOffset(&x2,&z2);
				
				drawrect.SetRect(x1,z1,x2-x1,z2-z1);
				if(drawrect.PointInRect(mouse_point))
				{
					return(-(c0+1));
				}
			}
		}
	}

//	ASSERT(0);


	storey_index=storey_list[OutsideEditStorey].InsideStorey;
	if(storey_index)
	{
		found=ClickInVertexStoreyList(0,storey_index,w,h,mouse_point);
		if(found==2)
			return(1);
		if(found==1)
			found_one=1;
	}
	return(found_one);
}

SLONG	SewerTab::DrawWindow(SLONG x1,SLONG z1,SLONG x2,SLONG z2,SLONG dx,SLONG dz)
{
	SLONG	pdx,pdz;

	pdx=dz;
	pdz=-dx;

	pdx=(pdx*20)>>10;
	pdz=(pdz*20)>>10;

	DrawContentLine(x1+(pdx>>1),z1+(pdz>>1),x1-(pdx>>1),z1-(pdz>>1),GetHeightColour(0));
	DrawContentLine(x2+(pdx>>1),z2+(pdz>>1),x2-(pdx>>1),z2-(pdz>>1),GetHeightColour(0));

	DrawContentLine(x1+(pdx>>1),z1+(pdz>>1),x2+(pdx>>1),z2+(pdz>>1),GetHeightColour(0));
	DrawContentLine(x1-(pdx>>1),z1-(pdz>>1),x2-(pdx>>1),z2-(pdz>>1),GetHeightColour(0));
	return(0);
}

SLONG 	SewerTab::DrawWall(SLONG px,SLONG pz,SLONG x1,SLONG z1,SLONG index,SLONG storey)
{
	SLONG	wcount,wwidth,wwidth_perc,wallwidth,wallwidth_perc,dx,dz,dist;
	SLONG	prev_x,prev_z;

	dx=abs(px-x1);
	dz=abs(pz-z1);

	dist=sqrl(SDIST2(dx,dz));
	if(dist==0)
		return(0);

	if(wall_list[index].WallFlags&FLAG_WALL_AUTO_WINDOWS)
	{
		wcount=dist/(BLOCK_SIZE*3);
		wwidth=dist/(wcount*2+1);
		
	}
	else
	{
		wcount=0; //wall_list[index].WindowCount;
		wwidth=BLOCK_SIZE;
	}

	dx=(px-x1);
	dz=(pz-z1);

	if(wcount<0)
		return(0);

	wallwidth=(dist-(wcount*wwidth))/(wcount+1);

	dx=(dx<<10)/dist;
	dz=(dz<<10)/dist;

	prev_x=x1;
	prev_z=z1;

	while(wcount)
	{
		x1=prev_x+((dx*wallwidth)>>10);
		z1=prev_z+((dz*wallwidth)>>10);

//		DrawLineC(prev_x,prev_z,x1,z1,1); //wall
		DrawContentLine(prev_x,prev_z,x1,z1,GetHeightColour(storey));

		prev_x=x1;
		prev_z=z1;

		x1=prev_x+((dx*wwidth)>>10);
		z1=prev_z+((dz*wwidth)>>10);

		DrawWindow(prev_x,prev_z,x1,z1,dx,dz);
//		DrawContentLine(prev_x,prev_z,x1,z1,0);

		prev_x=x1;
		prev_z=z1;
		wcount--;

	}
		x1=prev_x+((dx*wallwidth)>>10);
		z1=prev_z+((dz*wallwidth)>>10);

//		DrawLineC(prev_x,prev_z,x1,z1,1); //wall
		DrawContentLine(prev_x,prev_z,x1,z1,GetHeightColour(storey));
		return(0);

}


void	SewerTab::DrawContentLine(SLONG x1,SLONG y1,SLONG x2,SLONG y2,SLONG col)
{
/*
	x1=((x1>>ELE_SHIFT)-(ViewX>>ELE_SHIFT))*ViewSize+(WorkWindowRect.Width>>1);
	y1=((y1>>ELE_SHIFT)-(ViewZ>>ELE_SHIFT))*ViewSize+(WorkWindowRect.Height>>1);

	x2=((x2>>ELE_SHIFT)-(ViewX>>ELE_SHIFT))*ViewSize+(WorkWindowRect.Width>>1);
	y2=((y2>>ELE_SHIFT)-(ViewZ>>ELE_SHIFT))*ViewSize+(WorkWindowRect.Height>>1);
*/

		

	x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
	y1=((((y1)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);
	   
	x2=((((x2)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
	y2=((((y2)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);

	AddHeightOffset(&x1,&y1);
	AddHeightOffset(&x2,&y2);

	DrawLineC(x1,y1,x2,y2,col);

}

void	SewerTab::DrawContentLineY(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col)
{
	SLONG	temp;
/*
	x1=((x1>>ELE_SHIFT)-(ViewX>>ELE_SHIFT))*ViewSize+(WorkWindowRect.Width>>1);
	y1=((y1>>ELE_SHIFT)-(ViewZ>>ELE_SHIFT))*ViewSize+(WorkWindowRect.Height>>1);

	x2=((x2>>ELE_SHIFT)-(ViewX>>ELE_SHIFT))*ViewSize+(WorkWindowRect.Width>>1);
	y2=((y2>>ELE_SHIFT)-(ViewZ>>ELE_SHIFT))*ViewSize+(WorkWindowRect.Height>>1);
*/

	temp=CurrentY;
	x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
	z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);
	   
	x2=((((x2)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
	z2=((((z2)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);

	CurrentY=y1;
	AddHeightOffset(&x1,&z1);
	CurrentY=y2;
	AddHeightOffset(&x2,&z2);
	CurrentY=temp;

	DrawLineC(x1,z1,x2,z2,col);

}
void	SewerTab::DrawContentRect(SLONG x1,SLONG z1,SLONG x2,SLONG z2,SLONG col)
{
	DrawContentLine(x1,z1,x2,z1,col);
	DrawContentLine(x2,z1,x2,z2,col);
	DrawContentLine(x2,z2,x1,z2,col);
	DrawContentLine(x1,z2,x1,z1,col);
}

extern	SLONG	find_nearest_point(SLONG x,SLONG z,SLONG index,SLONG *rx,SLONG *rz);

void	SewerTab::DrawRoofFaces(SLONG roof,SLONG storey)
{
	SLONG wall,index;
	SLONG	rx,rz;

	wall=storey_list[storey].WallHead;
	while(wall)
	{
		SLONG	x1,z1;

		x1=wall_list[wall].DX;
		z1=wall_list[wall].DZ;

		if(find_nearest_point(x1,z1,roof,&rx,&rz))
		{
			CurrentY=storey_list[storey].DY;
			AddHeightOffset(&x1,&z1);
			CurrentY=storey_list[roof].DY;
			AddHeightOffset(&rx,&rz);

			DrawContentLine(x1,z1,rx,rz,RED_COL);
		}
		wall=wall_list[wall].Next;
	}
}


void	SewerTab::DrawFloorFaces(SLONG floor_head)
{
	SLONG 	wall,index;
	SLONG	rx,rz;
	SLONG	building;

	CurrentY=0;

	for(building=1;building<MAX_BUILDINGS;building++)
	{
		if(building_list[building].BuildingFlags&1)
		{
			wall=storey_list[building_list[building].StoreyHead].WallHead;
			while(wall)
			{
				SLONG	x1,z1;

				x1=wall_list[wall].DX;
				z1=wall_list[wall].DZ;

				if(find_nearest_point(x1,z1,floor_head,&rx,&rz))
				{
					AddHeightOffset(&x1,&z1);
					AddHeightOffset(&rx,&rz);

					DrawContentLine(x1,z1,rx,rz,GREEN_COL);
				}
				wall=wall_list[wall].Next;
			}
		}
	}
}

void	SewerTab::DrawFloorTextures(SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG	mx,my,mz;
	SLONG dx,dy,dz,width,height,count_across,count_high;
	UWORD	texture;

	mx=ViewX>>ELE_SHIFT;
	my=ViewY;
	mz=ViewZ>>ELE_SHIFT;


	width=ViewSize;

	count_across=((w/(width))>>1)+1;
	count_high=(h/(width))>>1;

	for(dx=-count_across;dx<=count_across;dx++)
	for(dz=-count_high;dz<=count_high;dz++)
	{
		SLONG	x1,y1,x2,y2;

		x1=(w>>1)+(dx)*(width);
		x2=(w>>1)+(dx)*(width)+width;

		y1=(h>>1)+dz*(width);
		y2=(h>>1)+dz*(width)+width;
		if(edit_info.RoofTex)
		{
			texture=tex_map[mx+dx][mz+dz];
		}
		else
		{
			texture=edit_map[mx+dx][mz+dz].Texture;
		}
		if(mx+dx>=0&&mx+dx<EDIT_MAP_WIDTH && mz+dz>=0&&mz+dz<EDIT_MAP_WIDTH)
		if(add_floor_face_to_bucket(x1,y1,0,x2,y1,0,x1,y2,0,x2,y2,0,&edit_map[mx+dx][mz+dz],128,128,128,128,texture))
		{
			selected_prim_xyz.X	=	dx+mx;
			selected_prim_xyz.Y	=	0;
			selected_prim_xyz.Z	=	dz+mz;

			hilited_face.MapX=dx+mx;
			hilited_face.MapY=0; //dx+mx-1;
			hilited_face.MapZ=dz+mz;
		}
	}
	render_view(0);
	for(dx=-count_across;dx<=count_across;dx++)
	for(dz=-count_high;dz<=count_high;dz++)
	{
		SLONG ox,oz,y;

		y=edit_map[mx+dx][mz+dz].Y;
		for(ox=-1;ox<=1;ox++)
		for(oz=-1;oz<=1;oz++)
		{
			if(mx+dx+ox>=0&&mx+dx+ox<EDIT_MAP_WIDTH && mz+dz+oz>=0&&mz+dz+oz<EDIT_MAP_WIDTH)
			if((ox||oz)&&y)
			if(y==edit_map[mx+dx+ox][mz+dz+oz].Y)
			{
				DrawContentLine((mx+dx)<<ELE_SHIFT,(mz+dz)<<ELE_SHIFT,(mx+dx+ox)<<ELE_SHIFT,(mz+dz+oz)<<ELE_SHIFT,0);
//				goto done;
			}
		}
done:;
	}

}

void	SewerTab::DrawFloorLabels(SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG	mx,my,mz;
	SLONG dx,dy,dz,width,height,count_across,count_high;
	mx=ViewX>>ELE_SHIFT;
	my=ViewY;
	mz=ViewZ>>ELE_SHIFT;

	width=ViewSize;

	count_across=((w/(width))>>1)+1;
	count_high=((h/(width))>>1)-1;

	for(dx=-count_across;dx<=count_across;dx++)
	for(dz=-count_high;dz<=count_high;dz++)
	{
		SLONG	x1,y1,x2,y2,alt,salt;
		CBYTE	str[100];

		x1=(w>>1)+(dx)*(width);
		y1=(h>>1)+dz*(width);
		if(RoofTop)
		{
			salt=edit_map_roof_height[mx+dx][mz+dz];
		}
		else
		{
			salt=edit_map[mx+dx][mz+dz].Y;
		}
		alt=abs(salt);

		sprintf(str,"%d",alt);
		QuickTextC(x1+1,y1+1,str,WHITE_COL);
		QuickTextC(x1-1,y1-1,str,WHITE_COL);
		if(salt<0)
			QuickTextC(x1,y1,str,RED_COL);
		else
			QuickTextC(x1,y1,str,0);

//		x2=(w>>1)+(dx)*(width)+width;
//		y2=(h>>1)+dz*(width)+width;
/*
		if(add_floor_face_to_bucket(x1,y1,0,x2,y1,0,x1,y2,0,x2,y2,0,&edit_map[mx+dx][mz+dz],128,128,128,128))
		{
			selected_prim_xyz.X	=	dx+mx;
			selected_prim_xyz.Y	=	0;
			selected_prim_xyz.Z	=	dz+mz;

			hilited_face.MapX=dx+mx;
			hilited_face.MapY=0; //dx+mx-1;
			hilited_face.MapZ=dz+mz;
		}
*/
	}
	render_view(0);
}

extern	void	draw_status_line(SLONG x,SLONG y,SLONG w,SLONG h,CBYTE *str);
extern	SLONG is_storey_habitable(SLONG storey);
extern	SLONG	identical_storey(SLONG px,SLONG pz,SLONG x1,SLONG z1,SLONG storey);

void	SewerTab::DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG	wwx,wwy,www,wwh;
	EdRect	drawrect;

	SLONG dx,dy,dz,width,height,count_across,count_high;
	SLONG	c0,c1;
	SLONG	mx,my,mz;
	SLONG	index;
	struct	EditMapElement	*p_ele;
	SLONG	roof_flag=0;
	SLONG	building;
	CBYTE	str[100];
	SLONG   storey_height;


//	my=((CVSlider*)GetControlPtr(CTRL_BUILD_V_SLIDE_LEVEL))->GetCurrentValue();
//	my=((CVSlider*)GetControlPtr(CTRL_BUILD_V_SLIDE_LEVEL))->GetCurrentValue();

	mx=ViewX>>ELE_SHIFT;
	my=ViewY;
	mz=ViewZ>>ELE_SHIFT;


	RedrawModuleContent=0;
	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;
	SetWorkWindowBounds(x,y,w-1,h-1);
	drawrect.SetRect(0,0,w-1,h-1);
	drawrect.FillRect(CONTENT_COL_BR);

	width=ViewSize;

	count_across=((w/(width))>>1)+1;
	count_high=(h/(width))>>1;


	if (!Keys[KB_T])
	{
		if(Texture&2)
			DrawFloorTextures(x,y,w,h);
		else
		{
			for(dx=-count_across;dx<=count_across;dx++)
			{
				DrawVLineC((w>>1)+(dx)*(width),0,h,RGB_TO_565(100,100,100));
			}
			for(dz=-count_high;dz<=count_high;dz++)
			{
				DrawHLineC(0,w,(h>>1)+dz*(width),RGB_TO_565(100,100,100));
			}
		}
		if(Texture&4)
			DrawFloorLabels(x,y,w,h);

		{
			sprintf(str," engine (%d,%d,%d)  view %d %d %d ",engine.X>>8,engine.Y>>8,engine.Z>>8,ViewX,ViewY,ViewZ);
			QuickTextC(3,3,str,0);
			QuickTextC(4,4,str,255);
		}
	}







//	if(EditBuilding)
	{



		SLONG	i,tx,tz,x1,y1,z1,x2,y2,z2,index,px,py,pz,fx,fy,fz, dx, dz, doorx1, doorx2, doorz1, doorz2;
		MFPoint		mouse_point;
		SLONG	storey_index;
		CBYTE	ploty=30;
		SLONG	pass=1;


		storey_index=OutsideEditStorey; //storey_list[OutsideEditStorey].StoreyHead;

		if(storey_list[storey_index].InsideIDIndex)
		{
			CBYTE	str[10];
			SLONG	index;
			SLONG	c0;

			index=storey_list[storey_index].InsideIDIndex;
			for(c0=0;c0<16;c0++)
			{
				if(room_ids[index].Flag[c0])
				{
					sprintf(str,"%d\n",c0+1);

					x1=	room_ids[index].X[c0]<<8;
					z1=	room_ids[index].Y[c0]<<8;


					x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
					z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);

					CurrentY=storey_list[OutsideEditStorey].DY;
					AddHeightOffset(&x1,&z1);


					QuickTextC(x1,z1,str,0);



				}
			}


			for(c0=0;c0<MAX_STAIRS_PER_FLOOR;c0++)
			{
				
				if(room_ids[index].StairFlags[c0])
				{
					SLONG	dir;
					SLONG	dx,dz;
					SLONG	x2,z2;
					EdRect	drawrect;

					dir=GET_STAIR_DIR(room_ids[index].StairFlags[c0]);
					switch(dir)
					{
						case	0:
							//north
							dx=256;
							dz=-512;
							break;
						case	1:
							//east
							dx=512;
							dz=256;
							break;
						case	2:
							//south
							dx=-256;
							dz=512;
							break;
						case	3:
							//west
							dx=-512;
							dz=-256;
							break;
					}
					x1=	room_ids[index].StairsX[c0]<<8;
					z1=	room_ids[index].StairsY[c0]<<8;

					if(dx<0)
					{
						x2=x1;
						x1=x1+dx;
					}
					else
					{
						x2=x1+dx;
					}
					if(dz<0)
					{
						z2=z1;
						z1=z1+dz;
					}
					else
					{
						z2=z1+dz;
					}




					x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
					z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);

					x2=((((x2)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
					z2=((((z2)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);

					CurrentY=storey_list[OutsideEditStorey].DY;
					AddHeightOffset(&x1,&z1);
					AddHeightOffset(&x2,&z2);



					drawrect.SetRect(x1,z1,x2-x1,z2-z1);
					switch(room_ids[index].StairFlags[c0]&3)
					{
						case	1:
							//up
							drawrect.FillRect(RED_COL);
							break;
						case	2:
							//down
							drawrect.FillRect(GREEN_COL);
							break;
						case	3:
							//up and down
							drawrect.FillRect(YELLOW_COL);
							break;

					}
				}
			}



		}
/*
			if(Mode==BUILD_MODE_PLACE_ROOM)
			{
				CBYTE	str[10];
				mouse_point.X	=	MouseX;
				mouse_point.Y	=	MouseY;
				sprintf(str,"%d\n",RoomID);
				Parent->GlobalToLocal(&mouse_point);
				
				QuickText(mouse_point.X,mouse_point.Y,str,0);
			}
*/

		
		while(storey_index)
		{
			UBYTE	drawn_normal=0;

			if(Keys[KB_D])
			{
				sprintf(str," storey %d  next %d prev %d dy %d height %d wallhead %d ->[%d] ->[%d]",storey_index,storey_list[storey_index].Next,storey_list[storey_index].Prev,storey_list[storey_index].DY,storey_list[storey_index].Height,storey_list[storey_index].WallHead,wall_list[storey_list[storey_index].WallHead].Next,wall_list[wall_list[storey_list[storey_index].WallHead].Next].Next);
				QuickText(20,ploty,str,0);
				ploty+=20;
			}

			mouse_point.X	=	MouseX;
			mouse_point.Y	=	MouseY;
			Parent->GlobalToLocal(&mouse_point);

			x1=storey_list[storey_index].DX;
			y1=storey_list[storey_index].DY;
			z1=storey_list[storey_index].DZ;

			CurrentY=storey_list[storey_index].DY;
			index=storey_list[storey_index].WallHead;
			switch(storey_list[storey_index].StoreyType)
			{
				case	STOREY_TYPE_NORMAL:
					drawn_normal=1;
					break;
				
			}

			if(Mode==BUILD_MODE_CONT_STOREY)
			{
				
				CurrentY=storey_list[EditStorey].DY;
				mouse_point.X+= ((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
				mouse_point.Y+=-((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);

				CalcMapCoord(&fx,&fy,&fz,x,y,w,h,&mouse_point);
				CurrentY=storey_list[storey_index].DY;
			}


			if(index==0)
			{
				
				if(Mode==BUILD_MODE_CONT_STOREY&&storey_index==EditStorey)
					DrawContentLine(x1,z1,fx,fz,GetHeightColour(storey_index));
			}
			else
			{

				px=x1;
				py=y1;
				pz=z1;
				while(index)
				{
					x1=wall_list[index].DX;
					y1=wall_list[index].DY;
					z1=wall_list[index].DZ;


					DrawWall(px,pz,x1,z1,index,storey_index);

					{
						SLONG	x1,y1,x2,y2;
						x1=px;
						y1=pz;
						x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Width>>1);
						y1=((((y1)-(ViewZ))*ViewSize)/ELE_SIZE)+(WorkWindowRect.Height>>1);
						   
						x2=x1;
						y2=y1;

						AddHeightOffset(&x1,&y1);
						CurrentY+=storey_list[storey_index].Height;
						AddHeightOffset(&x2,&y2);
						CurrentY-=storey_list[storey_index].Height;

						DrawLineC(x1,y1,x2,y2,0);

					}

					if(storey_list[storey_index].StoreyFlags&FLAG_ISTOREY_DOOR)
					{
						CurrentY+=128;
						DrawContentLine(px,pz,x1,z1,0); //otline tiled roofs
						CurrentY-=128;
					}

//					if(storey_list[storey_index].StoreyFlags&(FLAG_STOREY_TILED_ROOF|FLAG_STOREY_FLAT_TILED_ROOF))
					{

						CurrentY+=storey_list[storey_index].Height;
						DrawContentLine(px,pz,x1,z1,0); //otline tiled roofs
						CurrentY-=storey_list[storey_index].Height;

					}

					px=x1;
					py=y1;
					pz=z1;
					index=wall_list[index].Next;	
				}
				if(Mode==BUILD_MODE_CONT_STOREY&&storey_index==EditStorey)
					DrawContentLine(px,pz,fx,fz,GetHeightColour(storey_index));
			}

	
			if(pass==1)
			{
				pass=0;
				storey_index=storey_list[storey_index].InsideStorey;
			}
			else
			{
					storey_index=storey_list[storey_index].Next;

			}
		}
	}

	//
	// Draw the edge of the map...
	// 

	#define EDGE_COLOUR RED_COL

	for (SLONG i = 0; i < 128; i++)
	{
		DrawContentLine(i << 8,   0 << 8, i - 1 << 8,  -1 << 8, EDGE_COLOUR);
		DrawContentLine(i << 8, 128 << 8, i + 1 << 8, 129 << 8, EDGE_COLOUR);

		DrawContentLine(  0 << 8, i << 8,  -1 << 8, i - 1 << 8, EDGE_COLOUR);
		DrawContentLine(128 << 8, i << 8, 129 << 8, i + 1 << 8, EDGE_COLOUR);
	}



	if (!Keys[KB_T])
	{
		HighlightVertexes(x,y,w,h);
	}

	if(EditStorey&&storey_list[EditStorey].StoreyType==STOREY_TYPE_ROOF)
	{
		DrawRoofFaces(EditStorey,storey_list[EditStorey].Prev);
	}



	sprintf(str," Building: %d Storey %d (%s) wall %d dy %d height %d",EditBuilding,EditStorey,storey_name[storey_list[EditStorey].StoreyType],EditWall,storey_list[EditStorey].DY,storey_list[EditStorey].Height);

	draw_status_line(0,h-14,w,14,str);
	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

	//
	// Create a warning if the storey can't have an inside.
	//

}

//---------------------------------------------------------------


void	SewerTab::HandleTab(MFPoint *current_point)
{
	SLONG		   update	=	0;
	

	ModeTab::HandleTab(current_point);
	KeyboardInterface();

}

inline SLONG is_point_in_box(SLONG x,SLONG y,SLONG left,SLONG top,SLONG w,SLONG h)
{
	if(x>left&&x<left+w&&y>top&&y<top+h)
		return(1);
	else
		return(0);
}
//---------------------------------------------------------------


SLONG	SewerTab::KeyboardInterface(void)
{
	if(Keys[KB_TAB])
	{
		Keys[KB_TAB]=0;
		AxisMode++;
		if(AxisMode>3)
			AxisMode=0;
		switch(AxisMode)
		{
			case	0:
//				SetControlState(CTRL_BUILD_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_BUILD_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_BUILD_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=X_AXIS;
				break;
			case	1:
//				SetControlState(CTRL_BUILD_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_BUILD_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_BUILD_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=Y_AXIS;
				break;
			case	2:
//				SetControlState(CTRL_BUILD_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_BUILD_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_BUILD_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=Z_AXIS;
				break;
			case	3:
//				SetControlState(CTRL_BUILD_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_BUILD_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_BUILD_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=X_AXIS|Y_AXIS|Z_AXIS;
				break;
		}

		SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
  		DrawControlSet();
		ShowWorkWindow(0);
	}

	if (Keys[KB_U]) {Keys[KB_U] = 0; HandleControl(CTRL_NEXT_STOREY);}
	if (Keys[KB_D]) {Keys[KB_D] = 0; HandleControl(CTRL_PREV_STOREY);}

	if (Keys[KB_M] ||
		Keys[KB_P])
	{
		//
		// Where is the mouse in the world?
		//

		SLONG x;
		SLONG y;
		SLONG w;
		SLONG h;

		SLONG mx;
		SLONG my;
		SLONG mz;

		MFPoint mouse_point;

		mouse_point.X = MouseX;
		mouse_point.Y = MouseY;

		Parent->GlobalToLocal(&mouse_point);

		x = Parent->ContentLeft();
		y = Parent->ContentTop();
		w = Parent->ContentWidth();
		h = Parent->ContentHeight();

		CalcMapCoord(
			&mx,
			&my,
			&mz,
			 x,y,w,h,
			&mouse_point);

		if (Keys[KB_M]) {Keys[KB_M] = 0; EXTRA_create_or_delete(EXTRA_TYPE_MIST,   mx, mz);}
		if (Keys[KB_P]) {Keys[KB_P] = 0; EXTRA_create_or_delete(EXTRA_TYPE_PUDDLE, mx, mz);}

		RequestUpdate();
		SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
  		DrawControlSet();
		ShowWorkWindow(0);
	}

	return(0);
}

//#define	QDIST3(x,y,z)	(x>y ? (x>z ? x+(y>>2)+(z>>2) : z+(x>>2)+(y>>2)) : (y>z ? (y+(x>>2)+(z>>2) : z+(x>>2)+(y>>2) ))
//#define	QDIST3(x,y,z)	(x>y ? (x>z ? x+(y>>2)+(z>>2) : z+(x>>2)+(y>>2)) : (y>z ? (y+(x>>2)+(z>>2) : z+(x>>2)+(y>>2) ))



SLONG	SewerTab::DragEngine(UBYTE flags,MFPoint *clicked_point)
{
	SLONG	wwx,wwy,www,wwh;
	SLONG	screen_change=0;
	SLONG	last_world_mouse;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;


	{
		SLONG	start_x=0,start_y=0,start_z=0,flag=0;
		SLONG	old_x,old_y,old_z;
		SLONG	nx,ny,nz;

		old_x=nx=engine.X;
		old_y=ny=engine.Y;
		old_z=nz=engine.Z;

		while(SHELL_ACTIVE && MiddleButton)
		{
			last_world_mouse=SetWorldMouse(0);
			if(last_world_mouse)
			{
				if(!flag)
				{
					flag=1;
					start_x=engine.MousePosX<<8;
					start_y=engine.MousePosY<<8;
					start_z=engine.MousePosZ<<8;
				}

				nx=engine.MousePosX<<8;
				ny=engine.MousePosY<<8;
				nz=engine.MousePosZ<<8;

				engine.X = (old_x+(-nx+start_x));
				engine.Y = (old_y+(-ny+start_y));
				engine.Z = (old_z+(-nz+start_z));

//				engine.Z=nz<<8;
				
				DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
				SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
				ShowWorkWindow(0);
				screen_change=1;

				engine.X=old_x;
				engine.Y=old_y;
				engine.Z=old_z;
				
			}
		}
		if(flag)
		{
			engine.X= (old_x+(-nx+start_x));
			engine.Y= (old_y+(-ny+start_y));
			engine.Z= (old_z+(-nz+start_z));
		}
	}
	return(screen_change);

}

SLONG	SewerTab::CalcMapCoord(SLONG	*mapx,SLONG	*mapy,SLONG	*mapz,SLONG	x,SLONG	y,SLONG	w,SLONG	h,MFPoint	*clicked_point)
{
	SLONG	width,count_across,count_high;
	SLONG	mx,my,mz;
	SLONG	dx,dy,dz;
/*
	my=(engine.Y>>8)>>ELE_SHIFT;
	mx=(engine.X>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;

	width=((16<<5)*engine.Scale)>>16;
	LogText(" w %d h %d  click xy %d,%d  width %d res x %d res y %d \n",w,h,clicked_point->X,clicked_point->Y,width,(clicked_point->X-(w>>1))/(width+1),(clicked_point->Y-(h>>1))/(width+1));

	dx=(clicked_point->X-(w>>1));
	dy=(clicked_point->Y-(h>>1));

	if(dx>0)
		*mapx=dx/(width+1)+mx;
	else
		*mapx=((dx)/(width+1))+mx-1;


	if(dy>0)
		*mapz=dy/(width+1)+mz;
	else
		*mapz=((dy)/(width+1))+mz-1;

	*mapy=my;
*/

	mx=(ViewX>>(ELE_SHIFT));
	my=(ViewY>>(ELE_SHIFT));
	mz=(ViewZ>>(ELE_SHIFT));


	dx=(clicked_point->X-(w>>1));
	dz=(clicked_point->Y-(h>>1));
	if(dx<0)
		dx=((dx-(ViewSize>>1))<<GridFlag)/ViewSize+(mx<<GridFlag);
	else
		dx=((dx+(ViewSize>>1))<<GridFlag)/ViewSize+(mx<<GridFlag);

	if(dz<0)
		dz=((dz-(ViewSize>>1))<<GridFlag)/ViewSize+(mz<<GridFlag);
	else
		dz=((dz+(ViewSize>>1))<<GridFlag)/ViewSize+(mz<<GridFlag);

	

	*mapx=dx<<(ELE_SHIFT-GridFlag);
	*mapy=0;
	*mapz=dz<<(ELE_SHIFT-GridFlag);
	
	return(1);
}

extern	void	insert_cube(SWORD x,SWORD y,SWORD z);
extern	void	remove_cube(SLONG x,SLONG y,SLONG z);



SLONG	SewerTab::MouseInContent(void)
{



	if(Mode==BUILD_MODE_CONT_STOREY)
	{
		SLONG	x,y,w,h;
		SLONG	wwx,wwy,www,wwh;

		wwx=WorkWindowRect.Left;
		wwy=WorkWindowRect.Top;
		www=WorkWindowRect.Width;
		wwh=WorkWindowRect.Height;

		x=Parent->ContentLeft();
		y=Parent->ContentTop();
		w=Parent->ContentWidth();
		h=Parent->ContentHeight();

		SetWorkWindowBounds(x,y,w-1,h-1);

		DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		ShowWorkWindow(0);

		SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

		
	}
	return(0);
	
}

SLONG	SewerTab::DragPaint(UBYTE flags)
{
/*
	SLONG	x,y,w,h;
	SLONG	wwx,wwy,www,wwh;
	SLONG	col;
	SLONG	screen_change=0;
	MFPoint		mouse_point;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	col++;
	SetWorkWindowBounds(x,y,w-1,h-1);

		while(SHELL_ACTIVE && LeftButton)
		{
			SLONG	mx,my,mz,index;

			mouse_point.X	=	MouseX;
			mouse_point.Y	=	MouseY;
			Parent->GlobalToLocal(&mouse_point);

			CalcMapCoord(&mx,&my,&mz,x,y,w,h,&mouse_point);

			index=edit_map[(mx)][(my)].Depth[(mz)];
			if(!index)
				insert_cube(mx,my,mz);



			DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			ShowWorkWindow(0);
			screen_change=1;
//			editor_user_interface();
//			KeyboardInterface();
		}

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
	return(screen_change);
*/
	return(0);

}

SLONG	SewerTab::DragMark(UBYTE flags)
{
	SLONG	x,y,w,h;
	SLONG	wwx,wwy,www,wwh;
	SLONG	col = 0;
	SLONG	screen_change=0;
	SLONG	mx,my,mz,index;

	MFPoint		mouse_point;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	col++;
	SetWorkWindowBounds(x,y,w-1,h-1);

	mouse_point.X	=	MouseX;
	mouse_point.Y	=	MouseY;
	Parent->GlobalToLocal(&mouse_point);

	CalcMapCoord(&mx,&my,&mz,x,y,w,h,&mouse_point);
	X1=mx;
	Y1=my;
	Z1=mz;

		while(SHELL_ACTIVE && LeftButton)
		{

			mouse_point.X	=	MouseX;
			mouse_point.Y	=	MouseY;
			Parent->GlobalToLocal(&mouse_point);

			CalcMapCoord(&mx,&my,&mz,x,y,w,h,&mouse_point);
			X2=mx;
			Y2=my;
			Z2=mz;




			DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			ShowWorkWindow(0);
			screen_change=1;
		}

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

	return(screen_change);

}



SLONG	SewerTab::DragBuilding(UBYTE flags,UBYTE type)
{
	SLONG	x,y,w,h;
	SLONG	wwx,wwy,www,wwh;
	SLONG	col=0;
	SLONG	mx,my,mz,index;

	MFPoint		mouse_point;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	col++;
	SetWorkWindowBounds(x,y,w-1,h-1);

	LogText("before EditBuilding %d  EditStorey %d EditWall %d \n",EditBuilding,EditStorey,EditWall);

//	if(type==1)
//		EditBuilding=duplicate_building(EditBuilding);

	//LogText("after EditBuilding %d  EditStorey %d EditWall %d \n",EditBuilding,EditStorey,EditWall);

	while(SHELL_ACTIVE && (flags==LEFT_CLICK&&LeftButton)||(flags==RIGHT_CLICK && RightButton))
	{
		SLONG	dx,dz;

		mouse_point.X	=	MouseX;
		mouse_point.Y	=	MouseY;
		Parent->GlobalToLocal(&mouse_point);

		CalcMapCoord(&mx,&my,&mz,x,y,w,h,&mouse_point);


		if(EditWall)
		{
//			LogText(" editwall %d pos (%d,%d) \n",EditWall,wall_list[EditWall].DX,wall_list[EditWall].DZ);
			dx=mx-wall_list[EditWall].DX;
			dz=mz-wall_list[EditWall].DZ;
		}
		else
		{
			dx=mx-storey_list[EditStorey].DX;
			dz=mz-storey_list[EditStorey].DZ;
		}
//		move_building(EditBuilding,dx,0,dz);
		

		DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		ShowWorkWindow(0);
	}

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
	if(mouse_point.Y>h)
		return(0);
	else
		return(1);

}


//
// moves all vertices above map co-ord map_x,map_z to mx,mz
//
extern	void	move_all_vertices(SLONG map_x,SLONG map_z,SLONG mx,SLONG mz);

SLONG	SewerTab::DragVertex(UBYTE flags)
{
	SLONG	x,y,w,h;
	SLONG	wwx,wwy,www,wwh;
	SLONG	col = 0;
	SLONG	mx,my,mz,index;

	MFPoint		mouse_point;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	col++;
	SetWorkWindowBounds(x,y,w-1,h-1);

//	LogText("before EditBuilding %d  EditStorey %d EditWall %d \n",EditBuilding,EditStorey,EditWall);


	//LogText("after EditBuilding %d  EditStorey %d EditWall %d \n",EditBuilding,EditStorey,EditWall);

	while(SHELL_ACTIVE && (flags==LEFT_CLICK&&LeftButton)||(flags==RIGHT_CLICK && RightButton))
	{

		mouse_point.X	=	MouseX;
		mouse_point.Y	=	MouseY;
		Parent->GlobalToLocal(&mouse_point);

		CurrentY=storey_list[EditStorey].DY;

		//
		// This corrects the mouse offset problem
		//
		mouse_point.X+= ((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
		mouse_point.Y+=-((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);

		CalcMapCoord(&mx,&my,&mz,x,y,w,h,&mouse_point);

		if(ShiftFlag)
		{
			SLONG	dx,dz;

			if(EditWall)
			{
	//			LogText(" editwall %d pos (%d,%d) \n",EditWall,wall_list[EditWall].DX,wall_list[EditWall].DZ);
				dx=mx-wall_list[EditWall].DX;
				dz=mz-wall_list[EditWall].DZ;
			}
			else
			{
				dx=mx-storey_list[EditStorey].DX;
				dz=mz-storey_list[EditStorey].DZ;
			}
//			move_building(EditBuilding,dx,0,dz);
			

		}
		else
		if(Keys[KB_A])
		{
			SLONG	map_x,map_z;

			if(EditWall)
			{
				map_x=wall_list[EditWall].DX;
				map_z=wall_list[EditWall].DZ;
			}
			else
			{
				map_x=storey_list[EditStorey].DX;
				map_z=storey_list[EditStorey].DZ;
			}
			move_all_vertices(map_x,map_z,mx,mz);

		}
		else
		{
			if(EditWall)
			{
				wall_list[EditWall].DX=mx;
				wall_list[EditWall].DZ=mz;
			}
			else
			{
				storey_list[EditStorey].DX=mx;
				storey_list[EditStorey].DZ=mz;
			}
		}

		DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		ShowWorkWindow(0);
	}

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
	if(mouse_point.Y>h)
		return(0);
	else
		return(1);

}

SLONG	SewerTab::DragStairs(UWORD stair,UBYTE flags)
{
	SLONG	x,y,w,h;
	SLONG	wwx,wwy,www,wwh;
	SLONG	col = 0;
	SLONG	mx,my,mz,index;

	MFPoint		mouse_point;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	col++;
	SetWorkWindowBounds(x,y,w-1,h-1);

	while(SHELL_ACTIVE && (flags==LEFT_CLICK&&LeftButton)||(flags==RIGHT_CLICK && RightButton))
	{

		mouse_point.X	=	MouseX;
		mouse_point.Y	=	MouseY;
		Parent->GlobalToLocal(&mouse_point);

		CurrentY=storey_list[EditStorey].DY;

		//
		// This corrects the mouse offset problem
		//
		mouse_point.X+= ((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
		mouse_point.Y+=-((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);

		CalcMapCoord(&mx,&my,&mz,x,y,w,h,&mouse_point);

		room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairsX[stair]=mx>>8;
		room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairsY[stair]=mz>>8;

		DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		ShowWorkWindow(0);
	}

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

	if(mouse_point.Y>h)
		return(0);
	else
		return(1);

}

extern	SLONG	find_previous_wall(SLONG edit_storey,SLONG wall);

void	SewerTab::DeleteVertex(void)
{
	SLONG	prev;
	if(EditWall)
	{
		prev=find_previous_wall(EditStorey,EditWall);
		if(prev>0)
		{
			wall_list[prev].Next=wall_list[EditWall].Next;
			free_wall(EditWall);
			EditWall=0;
		}
		else
		{ //prev is a storeyhead
			storey_list[EditStorey].WallHead=wall_list[EditWall].Next;
			free_wall(EditWall);
			EditWall=0;
		}
	}
	else
	{			   
		SLONG	next;
		next=storey_list[EditStorey].WallHead;
		if(next)
		{
			storey_list[EditStorey].DX=wall_list[next].DX;
			storey_list[EditStorey].DZ=wall_list[next].DZ;
			storey_list[EditStorey].WallHead=wall_list[next].Next;
			free_wall(next);
		}

	}
}



SLONG	SewerTab::ClickNearWall(SLONG x,SLONG y,SLONG w,SLONG h,MFPoint	*mouse_point)
{
	SLONG	mx,mz,rect_size;
	EdRect	rect;
	SLONG	best_building,best_storey=0,best_wall=0,best_dist=0x7fffffff,dist;
	SLONG	roof_flag=0,building;

	mx=ViewX>>ELE_SHIFT;
	mz=ViewZ>>ELE_SHIFT;

	rect_size=ViewSize>>2;

	for(building=0;building<MAX_BUILDINGS;building++)
	{
		if(building_list[building].BuildingFlags&1)
		{

			SLONG	x1,y1,z1,x2,y2,z2,px,py,pz,index;
			SLONG	storey_index;

			storey_index=building_list[building].StoreyHead;

			while(storey_index)
			{
				
				x1=storey_list[storey_index].DX;
				z1=storey_list[storey_index].DZ;
				CurrentY=storey_list[storey_index].DY;
				index=storey_list[storey_index].WallHead;

	//			x1=((x1>>ELE_SHIFT)-mx)*ViewSize+(w>>1);
	//			z1=((z1>>ELE_SHIFT)-mz)*ViewSize+(h>>1);

				x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(w>>1);
				z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(h>>1);
				AddHeightOffset(&x1,&z1);
				if(index)
				{
					px=x1;
					pz=z1;


					while(index)
					{
						x1=wall_list[index].DX;
						z1=wall_list[index].DZ;

	//					x1=((x1>>ELE_SHIFT)-mx)*ViewSize+(w>>1);
	//					z1=((z1>>ELE_SHIFT)-mz)*ViewSize+(h>>1);
						x1=((((x1)-(ViewX))*ViewSize)/ELE_SIZE)+(w>>1);
						z1=((((z1)-(ViewZ))*ViewSize)/ELE_SIZE)+(h>>1);
						AddHeightOffset(&x1,&z1);

	//					dist=dist_between_vertex_and_vector(px,pz,x1,z1,mouse_point->X,mouse_point->Y);
						dist=dist_to_line(px,pz,x1,z1,mouse_point->X,mouse_point->Y);
						if(dist<best_dist)
						{
							best_building=building;
							best_storey=storey_index;
							best_wall=index;
							best_dist=dist;
							
						}
						index=wall_list[index].Next;	
						px=x1;
						pz=z1;
					}
				}
	//			storey_index=storey_list[storey_index].Next;
				if(roof_flag)
				{
					storey_index=storey_list[storey_list[storey_index].Prev].Next;
					roof_flag=0;
				}
				else
				{
					/*
					SLONG	temp_index;
					temp_index=storey_list[storey_index].Roof;
					if(temp_index)
					{
						storey_index=temp_index;
						roof_flag=1;
						
					}
					else
					*/
					{
						storey_index=storey_list[storey_index].Next;
						roof_flag=0;
					}

				}
			}
		}
	}
	if(best_storey)
	{
		EditBuilding=best_building;
		EditStorey=best_storey;
		EditY=0;//storey_list[EditStorey].DY;
		EditWall=best_wall;

		DrawTabContent();

		return(1);
	}
	return(0);
}

static ControlDef		popup_def	=	{	POPUP_MENU,	0,	""};
MenuDef2	wallsew_popup[]	=
{
	{	"~Poly Windows",	0	},
	{	"~Fence Post1",		0	},
	{	"~Roof Rim2",		0	},
	{	"~Recessed Door",	0	},
	{	"~Roof has Rim",	0	},
//	{	"~Archside",	0	},
//	{	"~ArchTop",	0	},
	{	"!",				0	}
};


SLONG	SewerTab::WallOptions(void)
{
	UBYTE			flags;
	ULONG			c0,
					control_id;
	CPopUp			*the_control	=	0;
	MFPoint			local_point;
	UBYTE			old_flags;

	local_point.X	=	MouseX;
	local_point.Y	=	MouseY;

	Parent->GlobalToLocal(&local_point);

	popup_def.ControlLeft	=	local_point.X+4;
	popup_def.ControlTop	=	local_point.Y-4;

	flags=0; //wall_list[EditWall].WallFlags;

	old_flags=flags;
	wallsew_popup[7].ItemFlags	=	0;
	for(c0=0;c0<7;c0++)
	{
		wallsew_popup[c0].ItemFlags	=	0;
		wallsew_popup[c0].MutualExclusiveID=0;
		if(flags&(1<<c0))
			wallsew_popup[c0].ItemFlags	|=	MENU_CHECK_MASK;
	}
	if(wall_list[EditWall].WallFlags&FLAG_WALL_AUTO_WINDOWS)
		wallsew_popup[0].ItemFlags	|=	MENU_CHECK_MASK;

	if(wall_list[EditWall].WallFlags&FLAG_WALL_FENCE_POST1)
		wallsew_popup[1].ItemFlags	|=	MENU_CHECK_MASK;

//	if(wall_list[EditWall].WallFlags&FLAG_WALL_FENCE_POST2)
//		wallsew_popup[2].ItemFlags	|=	MENU_CHECK_MASK;

	if(wall_list[EditWall].WallFlags&FLAG_WALL_RECESSED)
		wallsew_popup[3].ItemFlags	|=	MENU_CHECK_MASK;


	if(storey_list[wall_list[EditWall].StoreyHead].StoreyFlags&FLAG_STOREY_ROOF_RIM2)
		wallsew_popup[2].ItemFlags	|=	MENU_CHECK_MASK;

	if(storey_list[wall_list[EditWall].StoreyHead].StoreyFlags&FLAG_STOREY_ROOF_RIM)
		wallsew_popup[4].ItemFlags	|=	MENU_CHECK_MASK;

/*	if(wall_list[EditWall].WallFlags&FLAG_WALL_ARCH_SIDE)
		wallsew_popup[5].ItemFlags	|=	MENU_CHECK_MASK;

	if(wall_list[EditWall].WallFlags&FLAG_WALL_ARCH_TOP)
		wallsew_popup[6].ItemFlags	|=	MENU_CHECK_MASK;
*/

		wallsew_popup[1].MutualExclusiveID=1;
		wallsew_popup[2].MutualExclusiveID=1;


	popup_def.TheMenuDef	=	wallsew_popup;
	the_control		=	new CPopUp(&popup_def);
	control_id		=	the_control->TrackControl(&local_point);
	flags			=	0;

//	wall_list[EditWall].WindowCount=0;

	if(wallsew_popup[0].ItemFlags&MENU_CHECK_MASK)
		wall_list[EditWall].WallFlags|=FLAG_WALL_AUTO_WINDOWS;
	else
		wall_list[EditWall].WallFlags&=~FLAG_WALL_AUTO_WINDOWS;

	if(wallsew_popup[1].ItemFlags&MENU_CHECK_MASK)
		wall_list[EditWall].WallFlags|=FLAG_WALL_FENCE_POST1;
	else
		wall_list[EditWall].WallFlags&=~FLAG_WALL_FENCE_POST1;

	if(wallsew_popup[3].ItemFlags&MENU_CHECK_MASK)
		wall_list[EditWall].WallFlags|=FLAG_WALL_RECESSED;
	else
		wall_list[EditWall].WallFlags&=~FLAG_WALL_RECESSED;

//	if(wallsew_popup[2].ItemFlags&MENU_CHECK_MASK)
//		wall_list[EditWall].WallFlags|=FLAG_WALL_FENCE_POST2;
//	else
//		wall_list[EditWall].WallFlags&=~FLAG_WALL_FENCE_POST2;

	if(wallsew_popup[2].ItemFlags&MENU_CHECK_MASK)
		storey_list[wall_list[EditWall].StoreyHead].StoreyFlags|=FLAG_STOREY_ROOF_RIM2;
	else
		storey_list[wall_list[EditWall].StoreyHead].StoreyFlags&=~FLAG_STOREY_ROOF_RIM2;


	if(wallsew_popup[4].ItemFlags&MENU_CHECK_MASK)
		storey_list[wall_list[EditWall].StoreyHead].StoreyFlags|=FLAG_STOREY_ROOF_RIM;
	else
		storey_list[wall_list[EditWall].StoreyHead].StoreyFlags&=~FLAG_STOREY_ROOF_RIM;
/*
	if(wallsew_popup[5].ItemFlags&MENU_CHECK_MASK)
		wall_list[EditWall].WallFlags|=FLAG_WALL_ARCH_SIDE;
	else
		wall_list[EditWall].WallFlags&=~FLAG_WALL_ARCH_SIDE;

	if(wallsew_popup[6].ItemFlags&MENU_CHECK_MASK)
		wall_list[EditWall].WallFlags|=FLAG_WALL_ARCH_TOP;
	else
		wall_list[EditWall].WallFlags&=~FLAG_WALL_ARCH_TOP;
*/
	if(the_control)
	{
		delete	the_control;
	}
	return(1);
}

MenuDef2	roofsew_popup[]	=
{
	{	"~Flat Roof",		0	},
	{	"~Overlap Small",	0	},
	{	"~Overlap Medium",	0	},
	{	"~Walled",			0	},
	{	"~Reccesed",		0	},
	{	"!",				0	}
};



SLONG	SewerTab::RoofOptions(void)
{
	ULONG			flags=0;
	ULONG			c0,
					control_id;
	CPopUp			*the_control	=	0;
	MFPoint			local_point;

	local_point.X	=	MouseX;
	local_point.Y	=	MouseY;

	Parent->GlobalToLocal(&local_point);

	popup_def.ControlLeft	=	local_point.X+4;
	popup_def.ControlTop	=	local_point.Y-4;


	flags=storey_list[EditStorey].StoreyFlags;
	roofsew_popup[5].ItemFlags	=	0;
	for(c0=0;c0<5;c0++)
	{
		roofsew_popup[c0].ItemFlags	=	0;
		roofsew_popup[c0].MutualExclusiveID=0;
		if(flags&(1<<(c0+1)))
			roofsew_popup[c0].ItemFlags	|=	MENU_CHECK_MASK;
		else
			roofsew_popup[c0].ItemFlags	&=	~MENU_CHECK_MASK;
	}


	for(c0=1;c0<4;c0++)
	{
//		roofsew_popup[c0].MutualExclusiveID=1;
	}

	popup_def.TheMenuDef	=	roofsew_popup;
	the_control		=	new CPopUp(&popup_def);
	control_id		=	the_control->TrackControl(&local_point);
	flags			=	0;

//	storey_list[EditStorey].StoreyFlags&=~0x1f;
	for(c0=0;c0<5;c0++)
	{

		if(roofsew_popup[c0].ItemFlags&MENU_CHECK_MASK)
			storey_list[EditStorey].StoreyFlags|=1<<(c0+1);
		else
			storey_list[EditStorey].StoreyFlags&=~(1<<(c0+1));
	}


	if(the_control)
	{
		delete	the_control;
	}
	return(1);
}



MenuDef2	fencesew_popup[]	=
{
	{	"~Angle Top",0},
	{	"~BRICK WALL",0},
	{	"~HIGH Chain Fence",0},
	{	"~1 High Chain Fence",0},
	{	"~75% High Chain Fence",0},
	{	"~33% High Chain Fence",0},

	{	"!",				0	}
};



SLONG	SewerTab::FenceOptions(void)
{
	ULONG			flags=0;
	ULONG			c0,
					control_id;
	CPopUp			*the_control	=	0;
	MFPoint			local_point;
	CBYTE			str[100];

	local_point.X	=	MouseX;
	local_point.Y	=	MouseY;

	Parent->GlobalToLocal(&local_point);

	popup_def.ControlLeft	=	local_point.X+4;
	popup_def.ControlTop	=	local_point.Y-4;


//	flags=storey_list[EditStorey].StoreyFlags;
	switch(storey_list[EditStorey].StoreyType)
	{
		case	STOREY_TYPE_FENCE:
			flags=1<<1;
			break;
		case	STOREY_TYPE_FENCE_BRICK:
			flags=1<<2;
			break;
		case	STOREY_TYPE_FENCE_FLAT:
			sprintf(str,"HEIGHT %d \n",storey_list[EditStorey].Height);
			QuickText(0,0,str,100);
			QuickText(100,100,str,100);
			QuickText(200,200,str,100);

			switch(storey_list[EditStorey].Height)
			{
				case(512):
					flags=1<<3;
					break;
				case(256):
					flags=1<<4;
					break;
				case(256-64):
					flags=1<<5;
					break;
				case(256-128):
					flags=1<<6;
					break;
			}
			break;
	}

	fencesew_popup[6].ItemFlags	=	0;
	for(c0=0;c0<6;c0++)
	{
		fencesew_popup[c0].ItemFlags	=	0;
		fencesew_popup[c0].MutualExclusiveID=1;
		if(flags&(1<<(c0+1)))
			fencesew_popup[c0].ItemFlags	|=	MENU_CHECK_MASK;
		else
			fencesew_popup[c0].ItemFlags	&=	~MENU_CHECK_MASK;
	}



	popup_def.TheMenuDef	=	fencesew_popup;
	the_control		=	new CPopUp(&popup_def);
	control_id		=	the_control->TrackControl(&local_point);
	flags			=	0;

//	storey_list[EditStorey].StoreyFlags&=~0x1f;
	
	for(c0=0;c0<6;c0++)
	{

		if(fencesew_popup[c0].ItemFlags&MENU_CHECK_MASK)
		{
			switch(c0)
			{
				case	0:
					storey_list[EditStorey].StoreyType=STOREY_TYPE_FENCE;
					storey_list[EditStorey].Height=256;
					break;
				case	1:
					storey_list[EditStorey].StoreyType=STOREY_TYPE_FENCE_BRICK;
					storey_list[EditStorey].Height=256;
					break;
				case	2:
					storey_list[EditStorey].StoreyType=STOREY_TYPE_FENCE_FLAT;
					storey_list[EditStorey].Height=512;
					break;
				case	3:
					storey_list[EditStorey].StoreyType=STOREY_TYPE_FENCE_FLAT;
					storey_list[EditStorey].Height=256;
					break;
				case	4:
					storey_list[EditStorey].StoreyType=STOREY_TYPE_FENCE_FLAT;
					storey_list[EditStorey].Height=256-64;
					break;
				case	5:
					storey_list[EditStorey].StoreyType=STOREY_TYPE_FENCE_FLAT;
					storey_list[EditStorey].Height=128;
					break;
			}
			break;
		}
/*
			storey_list[EditStorey].StoreyFlags|=1<<(c0+1);
		else
			storey_list[EditStorey].StoreyFlags&=~(1<<(c0+1));
*/
	}


	if(the_control)
	{
		delete	the_control;
	}
	return(1);
}


extern	SLONG	count_wall_size(UWORD storey);
extern	SLONG	find_n_from_end(SLONG n,UWORD storey);
extern	void	show_storey(UWORD index);
extern	void	flip_storey(UWORD storey);


void	SewerTab::CheckStoreyIntegrity(UWORD storey)
{
	SLONG	x1,z1,x2,z2,x3,z3;
	SLONG	wall;


	if(storey_list[storey].WallHead&&wall_list[storey_list[storey].WallHead].Next)
	{
		
		x1=storey_list[storey].DX;
		z1=storey_list[storey].DZ;

		wall=storey_list[storey].WallHead;

		x2=wall_list[wall].DX;
		z2=wall_list[wall].DZ;

		wall=wall_list[wall].Next;

		x3=wall_list[wall].DX;
		z3=wall_list[wall].DZ;

/*
		if(!is_it_clockwise_xy(x1,z1,x2,z2,x3,z3))
		{
			flip_storey(storey);
			
		}
*/
	}
	
}


SLONG	get_new_inside_id(void)
{
	if(next_inside>MAX_INSIDE_STOREYS-2)
		return(0);
	next_inside++;
	return(next_inside-1);

}

SLONG	SewerTab::HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h)
{
	SWORD	thing;
	SLONG	index;
	SWORD	bright;
	SLONG	mx,my,mz;
	SLONG	ret;

	switch(Mode)
	{
		case	BUILD_MODE_WAIT:
				switch(flags)
				{
					case	LEFT_CLICK:
						if(ret=ClickInVertex(x,y,w,h,clicked_point))
						{
							if(ret<0)
							{
									DragStairs((-ret)-1,flags);
							}
							else
							{
								switch(storey_list[EditStorey].StoreyType)
								{
									case	STOREY_TYPE_FIRE_ESCAPE:
										{
											if(EditWall<0)
											{
												
												EditWall=0;
												storey_list[EditStorey].Height++;
												return(1);
											}
										}
										break;
									case	STOREY_TYPE_LADDER:
											if(EditWall<0)
											{
												SLONG	size=4;
												if(ShiftFlag)
													size=1;
												
												EditWall=0;
												storey_list[EditStorey].Height+=size;
												return(1);
											}
											break;
									case	STOREY_TYPE_STAIRCASE:
											if(EditWall<0)
											{
												EditWall=0;
												storey_list[EditStorey].Info1++;
												return(1);
											}

										break;
									case	STOREY_TYPE_CABLE:
											if(EditWall==99999)
											{
												EditWall=0;
												storey_list[EditStorey].DY+=64;
												return(1);
											}
											else
											if(EditWall<0)
											{
												wall_list[-EditWall].DY+=64;
												return(1);

											}
										break;
									
								}

								LogText(" dragging editwall %d\n",EditWall);
								if(ShiftFlag)
								{
									DragBuilding(flags,0);
								}
								else
								if(ControlFlag)
								{
									DragBuilding(flags,1);
								}
								else
								if(DragVertex(flags)==0)
								{
									DeleteVertex();
									return(1);
								}
							}

						}
						//drag vertex
						break;
					case	RIGHT_CLICK:
						if(ret=ClickInVertex(x,y,w,h,clicked_point))
						{
							if(ret<0)
							{
								//
								// right clicked on a staircase
								//
								DoStairPopUp((-ret)-1,clicked_point);


							}
							else
							{
								switch(storey_list[EditStorey].StoreyType)
								{
									case	STOREY_TYPE_FIRE_ESCAPE:
										{
											if(EditWall<0)
											{
												
												EditWall=0;
												storey_list[EditStorey].Height--;
												return(1);
											}
										}
										break;
									case	STOREY_TYPE_LADDER:
											if(EditWall<0)
											{
												SLONG	size=4;
												if(ShiftFlag)
													size=1;
												
												EditWall=0;
												storey_list[EditStorey].Height-=size;
												return(1);
											}
											break;

									case	STOREY_TYPE_STAIRCASE:
											if(EditWall<0)
											{
												EditWall=0;
												storey_list[EditStorey].Info1--;
												return(1);
											}
										break;
									case	STOREY_TYPE_CABLE:
											if(EditWall==99999)
											{
												EditWall=0;
												storey_list[EditStorey].DY-=64;
												return(1);
											}
											else
											if(EditWall<0)
											{
												wall_list[-EditWall].DY-=64;
												return(1);

											}
										break;
									
								}
								if(EditWall>0)
								{
									SLONG	temp_next;
									temp_next=wall_list[EditWall].Next;
									index=get_new_wall();
									wall_list[index].StoreyHead=EditStorey;
									wall_list[index].WallFlags=1; //|FLAG_WALL_AUTO_WINDOWS;
										
									wall_list[EditWall].Next=index;

									wall_list[index].DX=wall_list[EditWall].DX;
									wall_list[index].DZ=wall_list[EditWall].DZ;
									wall_list[index].Next=temp_next;
	//								wall_list[index].WindowCount=0;
									EditWall=index;
									if(DragVertex(flags)==0)
									{
										DeleteVertex();
										return(1);
									}
								}
								else
								{
									//trying to drag one of the root
									
								}
							}
						}
						else
						{
							if(ClickNearWall(x,y,w,h,clicked_point))
							{
								SetWorkWindowBounds(x,y,w-1,h-1);
								switch(storey_list[EditStorey].StoreyType)
								{
									case	STOREY_TYPE_ROOF:
										RoofOptions();
										break;
									case	STOREY_TYPE_NORMAL:
										WallOptions();
										break;
									case	STOREY_TYPE_FENCE:
									case	STOREY_TYPE_FENCE_BRICK:
									case	STOREY_TYPE_FENCE_FLAT:
										FenceOptions();
										break;
									default:
										WallOptions();
										break;
								}
								return(1);
								
							}
						}
						//delete vertex
						break;
				}
				break;

		case	BUILD_MODE_PLACE_STAIRS:
				switch(flags)
				{
					SLONG	inside;
					case	LEFT_CLICK:
						Mode=BUILD_MODE_WAIT;


						CurrentY=storey_list[EditStorey].DY;
						clicked_point->X+= ((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
						clicked_point->Y+=-((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);

						CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);


						mx>>=8;
						mz>>=8;

						if(storey_list[OutsideEditStorey].InsideIDIndex)
						{
							inside=storey_list[OutsideEditStorey].InsideIDIndex;
						}
						else
						{
							storey_list[OutsideEditStorey].InsideIDIndex=get_new_inside_id();
							inside=storey_list[OutsideEditStorey].InsideIDIndex;
						}

						if(inside)
						{
							SLONG	c0;
							for(c0=0;c0<MAX_STAIRS_PER_FLOOR;c0++)
							{
								if(room_ids[inside].StairFlags[c0]==0)
								{
									room_ids[inside].StairsX[c0]=mx;
									room_ids[inside].StairsY[c0]=mz;
									room_ids[inside].StairFlags[c0]=3; //default up and down
									RequestUpdate();

									return(1);
								}
							}

						}
						RequestUpdate();
						break;
				}
				break;

		case	BUILD_MODE_PLACE_ROOM:
				switch(flags)
				{
					SLONG	inside;
					case	LEFT_CLICK:
						Mode=BUILD_MODE_WAIT;

						CurrentY=storey_list[EditStorey].DY;
						clicked_point->X+= ((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
						clicked_point->Y+=-((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);

						CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);


						mx>>=8;
						mz>>=8;

						if(storey_list[OutsideEditStorey].InsideIDIndex)
						{
							inside=storey_list[OutsideEditStorey].InsideIDIndex;
						}
						else
						{
							storey_list[OutsideEditStorey].InsideIDIndex=get_new_inside_id();
							inside=storey_list[OutsideEditStorey].InsideIDIndex;
						}

						if(inside)
						{
							room_ids[inside].X[RoomID]=mx;
							room_ids[inside].Y[RoomID]=mz;
							room_ids[inside].Flag[RoomID]=1;

						}
						RequestUpdate();
						break;
					case	RIGHT_CLICK:

						if(storey_list[OutsideEditStorey].InsideIDIndex)
						{
							inside=storey_list[OutsideEditStorey].InsideIDIndex;
						}
						else
						{
							storey_list[OutsideEditStorey].InsideIDIndex=get_new_inside_id();
							inside=storey_list[OutsideEditStorey].InsideIDIndex;
						}

						if(inside)
						{
							room_ids[inside].X[RoomID]=0;
							room_ids[inside].Y[RoomID]=0;
							room_ids[inside].Flag[RoomID]=0;
						}
						RequestUpdate();
						break;


					}


			break;
		case	BUILD_MODE_PLACE_STOREY:
				switch(flags)
				{
					case	LEFT_CLICK:
						//
						// This corrects the mouse offset problem
						//
						CurrentY=storey_list[EditStorey].DY;
						clicked_point->X+= ((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
						clicked_point->Y+=-((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
					
						CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);
						storey_list[EditStorey].DX=mx;
		//				storey_list[EditStorey].DY=my;
						storey_list[EditStorey].DZ=mz;
						Mode=BUILD_MODE_CONT_STOREY;
						return(1);
					case	RIGHT_CLICK:
						Mode=BUILD_MODE_WAIT;
						break;
				}
			break;

		case	BUILD_MODE_CONT_STOREY:
				switch(flags)
				{
					case	LEFT_CLICK:
						//
						// This corrects the mouse offset problem
						//
						CurrentY=storey_list[EditStorey].DY;
						clicked_point->X+= ((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);
						clicked_point->Y+=-((-CurrentY)*(ViewSize+3))/(BLOCK_SIZE<<3);

						CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);
						index=get_new_wall();
						wall_list[index].StoreyHead=EditStorey;
						wall_list[index].WallFlags=1; //|FLAG_WALL_AUTO_WINDOWS;
//						wall_list[index].WindowCount=0;
						if(EditWall)
						{
							
							wall_list[EditWall].Next=index;
						}
						else
						{
							storey_list[EditStorey].WallHead=index;
						}

						EditWall=index;

						wall_list[EditWall].DX=mx;
						if(EditStorey>0 && storey_list[EditStorey].StoreyType==STOREY_TYPE_CABLE)
						{
							wall_list[EditWall].DY=storey_list[EditStorey].DY;

						}

						wall_list[EditWall].DZ=mz;
						wall_list[EditWall].Next=0;
						if(storey_list[EditStorey].StoreyType==STOREY_TYPE_LADDER)
						{
							Mode=BUILD_MODE_WAIT;
						}

						break;
					case	RIGHT_CLICK:
						Mode=BUILD_MODE_WAIT;


						return(1);
				}
				break;
	}
	return(0);
	
}

UWORD	SewerTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
{
	UWORD		control_id;
	Control		*current_control;
	MFPoint		local_point;


	// This is a fudge to update the front screen buffer.
	ShowWorkScreen(0);

	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
			local_point	=	*clicked_point;
			GlobalToLocal(&local_point);
			{
				current_control	=	GetControlList();
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
			}

			break;
		case	RIGHT_CLICK:
			SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
			local_point	=	*clicked_point;
			GlobalToLocal(&local_point);
			break;
	}
	return	0;
}

//---------------------------------------------------------------

SLONG  SewerTab::DoZoom(void)
{
	SLONG	update=0;
	if(Keys[KB_I])
	{
		ViewSize++;
		if(ViewSize>100)
			ViewSize=100;
		update=2;
	}

	if(Keys[KB_O])
	{
		ViewSize--;
		if(ViewSize<3)
			ViewSize=3;
		update=2;
	}
	return(update);
	
}

SLONG  SewerTab::DoKeys(void)
{
	SLONG	update=0;
	SLONG	scroll_step;

	scroll_step=110/(ViewSize+39);
	if(scroll_step<1)
		scroll_step=1;

	scroll_step<<=ELE_SHIFT;



	update=DoZoom();
	if(Keys[KB_LEFT])
	{
		ViewX-=scroll_step;
		update=2;
	}
	if(Keys[KB_RIGHT])
	{
		ViewX+=scroll_step;
		update=2;
	}
	if(!ShiftFlag)
	{
		
		if(Keys[KB_UP])
		{
			ViewZ-=scroll_step;
			update=2;
		}
		if(Keys[KB_DOWN])
		{
			ViewZ+=scroll_step;
			update=2;
		}
	}
	else
	{
		if(Keys[KB_UP])
		{
			if(storey_list[EditStorey].Next)
			{
				Keys[KB_UP]=0;
				EditStorey=storey_list[EditStorey].Next;
				EditY=0;//storey_list[EditStorey].DY;
				
			}
			update=2;
		}
		if(Keys[KB_DOWN])
		{
			if(storey_list[EditStorey].Prev)
			{
				Keys[KB_DOWN]=0;
				EditStorey=storey_list[EditStorey].Prev;
				EditY=0;//storey_list[EditStorey].DY;
			}
			update=2;
		}
	}

	return(update);

}

SLONG	SewerTab::SetWorldMouse(ULONG flag)
{
	MFPoint		mouse_point;
	MFPoint		local_point;
	SVector		point,out;
	SLONG	wwx,wwy,www,wwh;
	SLONG	temp;

	temp=engine.ClipFlag;
	engine.ClipFlag=0;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	mouse_point.X	=	MouseX;
	mouse_point.Y	=	MouseY;


	local_point	=	mouse_point;
	Parent->GlobalToLocal(&local_point);
	if(is_point_in_box(local_point.X,local_point.Y,0,0,Parent->ContentWidth()-1,Parent->ContentHeight()/2))
	{
		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth()-1,Parent->ContentHeight()/2-3);
		set_camera_plan();
		calc_world_pos_plan(local_point.X,local_point.Y);
		if(flag)
			engine.MousePosY=engine.Y>>8;
		SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
		engine.ClipFlag=temp;
		return(1);
	}
	else
	{
		
		engine.ClipFlag=temp;
		return(0);
	}

}

extern	void	free_walls(SLONG wall);
extern	void	delete_storey_list(SWORD storey);
extern	void	delete_building(UWORD building);
extern	void	set_storey_height(SLONG building,SLONG storey ,SLONG height);


void	SewerTab::HandleControl(UWORD control_id)
{
	SLONG	id;
	id=control_id&0xff;

	if(id>=7&&id<7+16)
	{
		Mode=BUILD_MODE_PLACE_ROOM;
		RoomID=id-7;
		return;
	}

	switch(id)
	{

		case	CTRL_BUILD_CREATE_BUILDING:
				SLONG	y;
				create_building_prim(EditBuilding,&y);
			break;
		case	CTRL_NEW_FENCE:
			if(Mode==BUILD_MODE_WAIT||Mode==0)
			{
				UWORD	building,storey;
				building=get_new_building();
				if(building)
				{
					EditBuilding=building;
					building_list[building].BuildingFlags|=1;
					storey=get_new_storey();
					storey_list[storey].BuildingHead=EditBuilding;
					storey_list[storey].StoreyFlags=1;
					storey_list[storey].DY=0;
					storey_list[storey].Height=BLOCK_SIZE*4;
					storey_list[storey].StoreyType=STOREY_TYPE_FENCE;
					storey_list[storey].Prev=0;
//					EditY=storey_list[storey].DY;
					if(storey)
					{
						building_list[building].StoreyHead=storey;
						building_list[building].StoreyCount=1;
						EditStorey=storey;
					}
					EditWall=0;
				}
				Mode=BUILD_MODE_PLACE_STOREY;
			}
			break;
		case	CTRL_DELETE_STOREY:
			if(Mode==BUILD_MODE_WAIT)
				if(EditStorey)
				{
					SLONG	storey,prev,index;
					storey=EditStorey;
					storey_list[storey].StoreyFlags=0;
					EditStorey=storey_list[storey].Next;
//					EditY=storey_list[EditStorey].DY;
					if(storey_list[storey].WallHead)
						free_walls(storey_list[storey].WallHead);
/*
					if(storey_list[storey].Roof)
					{
						delete_storey_list(storey_list[storey].Roof);
						storey_list[storey].Roof=0;
					}
*/
					EditWall=0;
					LogText(" delete storey %d next %d prev %d \n",storey,EditStorey,storey_list[storey].Prev);
					prev=storey_list[storey].Prev;
					if(prev && (storey_list[prev].Next==storey))
					{
						storey_list[prev].Next=EditStorey;
						if(EditStorey==0)
						{
							LogText(" next==0, prev=%d \n",prev);
							EditStorey=prev;
//							EditY=storey_list[EditStorey].DY;
							RequestUpdate();
							return;
						}
						else
						{
							LogText("p2 next==%d, prev=%d \n",EditStorey,prev);
							storey_list[EditStorey].Prev=prev;
						}

					}
					else
					{
						LogText("parent is building %d next= %d \n",EditBuilding,EditStorey);
						building_list[EditBuilding].StoreyHead=EditStorey;
						building_list[EditBuilding].StoreyCount--;
						storey_list[EditStorey].Prev=0;
					}
					switch(storey_list[storey].StoreyType)
					{
						case	STOREY_TYPE_FIRE_ESCAPE:
						case	STOREY_TYPE_LADDER:
						case	STOREY_TYPE_ROOF_QUAD:
						case	STOREY_TYPE_CABLE:
						case	STOREY_TYPE_TRENCH:
							break;
						default:

							index=EditStorey;
							while(index)
							{
								storey_list[index].DY-=storey_list[storey].Height;
//								if(storey_list[index].Roof)
//									storey_list[storey_list[index].Roof].DY-=BLOCK_SIZE*4;

								index=storey_list[index].Next;
							}
					}
					RequestUpdate();
				}
			break;
			case	CTRL_BUILD_DEL_STOREY:
				if(EditStorey)
				{
					SLONG	storey,prev,index;
					storey=EditStorey;
					storey_list[storey].StoreyFlags=0;
					EditStorey=storey_list[storey].Next;

					if(storey_list[storey].WallHead)
						free_walls(storey_list[storey].WallHead);
					EditWall=0;
					prev=storey_list[storey].Prev;

					if(prev==OutsideEditStorey)
					{
						if(prev&&storey_list[prev].InsideStorey==storey)
						{
							storey_list[prev].InsideStorey=EditStorey;
							if(EditStorey==0)
							{
							//	EditStorey=prev;
								RequestUpdate();
								return;
							}
							else
							{
								storey_list[EditStorey].Prev=prev;
							}
						}
						else
						{
							ASSERT(0);
						}


					}
					else
					if(prev && (storey_list[prev].Next==storey))
					{
						storey_list[prev].Next=EditStorey;
						if(EditStorey==0)
						{
							//EditStorey=prev;
							RequestUpdate();
							return;
						}
						else
						{
							storey_list[EditStorey].Prev=prev;
						}

					}
					else
					{
						ASSERT(0);
					}
				}

				break;
			case	CTRL_BUILD_NEXT_STOREY:
				{
					if(storey_list[OutsideEditStorey].Next)
					{
						OutsideEditStorey=storey_list[OutsideEditStorey].Next;
						RequestUpdate();
					}
				}
				break;
			case	CTRL_BUILD_PREV_STOREY:
				{
					if(storey_list[OutsideEditStorey].Prev)
					{
						OutsideEditStorey=storey_list[OutsideEditStorey].Prev;
					RequestUpdate();
					}
				}
				break;

			case	CTRL_BUILD_NEW_WALLS:

			if(Mode==BUILD_MODE_WAIT)
//				if(EditBuilding)
				{
					if(EditStorey&&storey_list[EditStorey].StoreyType!=STOREY_TYPE_PARTITION)
					{
						Alert			*quit_alert;

						quit_alert		=	new	Alert;
						quit_alert->HandleAlert("Can't add next partition to current partition ",NULL);
						delete	quit_alert;
//`						RequestUpdate();
					}
					else
					{
						

						UWORD	storey;
						storey=get_new_storey();
						if(storey)
						{
							storey_list[storey].BuildingHead=EditBuilding;
							storey_list[storey].StoreyFlags=1;
//							storey_list[storey].Prev=EditStorey;
							storey_list[storey].StoreyType=STOREY_TYPE_PARTITION;
							storey_list[storey].DY=storey_list[OutsideEditStorey].DY; //BLOCK_SIZE*5;

							if(storey_list[OutsideEditStorey].InsideStorey)
							{
								storey_list[storey].Next=storey_list[OutsideEditStorey].InsideStorey;
								storey_list[storey].Prev=OutsideEditStorey;
								storey_list[storey_list[storey].Next].Prev=storey;

								storey_list[OutsideEditStorey].InsideStorey=storey;

							}
							else
							{
								storey_list[OutsideEditStorey].InsideStorey=storey;
								storey_list[storey].Prev=OutsideEditStorey;
							}
							EditStorey=storey;
							storey_list[EditStorey].WallHead=0;
							storey_list[EditStorey].Height=BLOCK_SIZE*4;
							EditWall=0;
							Mode=BUILD_MODE_PLACE_STOREY;
						}
					}
					RequestUpdate();
				}
				break;
			case	CTRL_BUILD_NEW_DOOR:

			if(Mode==BUILD_MODE_WAIT)
//				if(EditBuilding)
				{
					if(EditStorey&&storey_list[EditStorey].StoreyType!=STOREY_TYPE_PARTITION)
					{
						Alert			*quit_alert;

						quit_alert		=	new	Alert;
						quit_alert->HandleAlert("Can't add next partition to current partition ",NULL);
						delete	quit_alert;
//`						RequestUpdate();
					}
					else
					{
						

						UWORD	storey;
						storey=get_new_storey();
						if(storey)
						{
							storey_list[storey].BuildingHead=EditBuilding;
							storey_list[storey].StoreyFlags=1|FLAG_ISTOREY_DOOR;
//							storey_list[storey].Prev=EditStorey;
							storey_list[storey].StoreyType=STOREY_TYPE_PARTITION;
							storey_list[storey].DY=storey_list[OutsideEditStorey].DY; //BLOCK_SIZE*5;

							if(storey_list[OutsideEditStorey].InsideStorey)
							{
								storey_list[storey].Next=storey_list[OutsideEditStorey].InsideStorey;
								storey_list[storey].Prev=OutsideEditStorey;
								storey_list[storey_list[storey].Next].Prev=storey;

								storey_list[OutsideEditStorey].InsideStorey=storey;

							}
							else
							{
								storey_list[OutsideEditStorey].InsideStorey=storey;
								storey_list[storey].Prev=OutsideEditStorey;
							}
							EditStorey=storey;
							storey_list[EditStorey].WallHead=0;
							storey_list[EditStorey].Height=BLOCK_SIZE*4;
							EditWall=0;
							Mode=BUILD_MODE_PLACE_STOREY;
						}
					}
					RequestUpdate();
				}
				break;
		case	CTRL_ADD_STAIRCASE:
			if(Mode==BUILD_MODE_WAIT)
				if(EditBuilding)
				{

						
					SLONG	storey;
					storey=get_new_storey();
					if(storey)
					{
						storey_list[storey].BuildingHead=EditBuilding;
						storey_list[storey].StoreyFlags=1;
						storey_list[storey].Prev=EditStorey;
						storey_list[storey].StoreyType=STOREY_TYPE_STAIRCASE;
						storey_list[storey].DY=0; //storey_list[EditStorey].DY-BLOCK_SIZE*5;
//						EditY=storey_list[storey].DY;
						if(building_list[EditBuilding].StoreyHead)
						{
							storey_list[storey].Next=building_list[EditBuilding].StoreyHead;
							storey_list[storey_list[storey].Next].Prev=storey;
//							storey_list[EditStorey].Next=storey;
							building_list[EditBuilding].StoreyHead=storey;
						}
						else
						{
							building_list[EditBuilding].StoreyHead=storey;
						}
						EditStorey=storey;
						storey_list[EditStorey].WallHead=0;
//						storey_list[EditStorey].Roof=0;
						storey_list[EditStorey].Height=4;
						storey_list[EditStorey].Info1=0;
						EditWall=0;
						Mode=BUILD_MODE_PLACE_STOREY;
						RequestUpdate();
					}
				}
				break;

/*
		case	CTRL_NEXT_STOREY:
		
			if (WITHIN(EditBuilding, 1, MAX_BUILDINGS - 1) &&
				WITHIN(EditStorey, 1, MAX_STOREYS - 1))
			{
				SLONG i_storey;

				i_storey = storey_list[EditStorey].Next;

				if (WITHIN(i_storey, 1, MAX_STOREYS - 1))
				{
					EditStorey = i_storey;
//					EditY      = storey_list[i_storey].DY;
					RequestUpdate();
				}
			}

			break;


		case	CTRL_PREV_STOREY:
		
			if (WITHIN(EditBuilding, 1, MAX_BUILDINGS - 1) &&
				WITHIN(EditStorey, 1, MAX_STOREYS - 1))
			{
				SLONG i_storey;

				i_storey = storey_list[EditStorey].Prev;

				if (WITHIN(i_storey, 1, MAX_STOREYS - 1))
				{
					EditStorey = i_storey;
//					EditY      = storey_list[i_storey].DY;
					RequestUpdate();
				}
			}
*/
		case	CTRL_PLACE_STAIRS:
				Mode=BUILD_MODE_PLACE_STAIRS;
				RequestUpdate();
			break;
		case	CTRL_DELETE_DUPLICATE_INSIDES:

				storey_list[OutsideEditStorey].InsideIDIndex=0;
				storey_list[OutsideEditStorey].InsideStorey=0;
				EditStorey=0;
				EditWall=0;

				RequestUpdate();

			break;

		case	CTRL_INSTYLE_MENU:
			CurrentFloorType	=	(control_id>>8)-1;
			if(OutsideEditStorey)
			{
				SLONG	inside,index;
				index=storey_list[OutsideEditStorey].InsideIDIndex; //building_list[building].StoreyHead;
				room_ids[index].FloorType=CurrentFloorType;

			}
			RequestUpdate();

			break;


	}
}


//static ControlDef		popup_def	=	{	POPUP_MENU,	0,	""};

MenuDef2	stair_popup[]	=
{
	{"Delete Stairs"},{"Go Up"},{"Go Down"},{"Go UP And Down"},{"North"},{"East"},{"South"},{"West"},{"!"}
};

//static ControlDef		popup_def	=	{	POPUP_MENU,	0,	""};

void	SewerTab::DoStairPopUp(SLONG stair,MFPoint *clicked_point)
{
	UBYTE			flags;
	ULONG			c0,
					control_id;
	CPopUp			*the_control	=	0;
	MFPoint			local_point;

	SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth()-1,Parent->ContentHeight()-1);

	local_point	=	*clicked_point;
//	GlobalToLocal(&local_point);
	popup_def.ControlLeft	=	local_point.X+4;
	popup_def.ControlTop	=	local_point.Y-4;
	popup_def.TheMenuDef	=	stair_popup;
	the_control				=	new CPopUp(&popup_def);

	for(c0=0;c0<7;c0++)
		the_control->SetItemState(c0,CTRL_ACTIVE);

	//
	// Select Cut,Copy,Paste 
	//
	control_id				=	the_control->TrackControl(&local_point);
	switch(control_id>>8)
	{
		case	1:
			room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair]=0;
			break;
		case	2:
			//up
			room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair]&=~(STAIR_FLAG_UP|STAIR_FLAG_DOWN);
			room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair]|=STAIR_FLAG_UP;
			break;
		case	3:
			//down
			room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair]&=~(STAIR_FLAG_UP|STAIR_FLAG_DOWN);
			room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair]|=STAIR_FLAG_DOWN;
			break;
		case	4:
			//up and down
			room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair]&=~(STAIR_FLAG_UP|STAIR_FLAG_DOWN);
			room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair]|=(STAIR_FLAG_UP|STAIR_FLAG_DOWN);
			break;
		case	5:
			//n
			SET_STAIR_DIR(room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair],0);
			break;
		case	6:
			//e
			SET_STAIR_DIR(room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair],1);
			break;
		case	7:
			//s
			SET_STAIR_DIR(room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair],2);
			break;
		case	8:
			//w
			SET_STAIR_DIR(room_ids[storey_list[OutsideEditStorey].InsideIDIndex].StairFlags[stair],3);
			break;


	}

	if(the_control)
	{
		delete	the_control;
	}
	RequestUpdate();
}			



