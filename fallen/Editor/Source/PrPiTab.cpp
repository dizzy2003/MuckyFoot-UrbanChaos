#include	"Editor.hpp"

#include	"PrPiTab.hpp"
#include	"engine.h"
#include	"math.h"
#include	"FileReq.hpp"
#include	"c:\fallen\headers\io.h"
#include	"c:\fallen\headers\memory.h"

//#include	"collide.hpp"  //needed for ele_shift

extern void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
extern void matrix_transform(struct Matrix31* result, struct Matrix33* trans,struct  Matrix31* mat2);
extern void matrix_transform_small(struct Matrix31* result, struct Matrix33* trans,struct  SMatrix31* mat2);


// static		counter;

//#define		ShowWorkWindow(x)	{DrawLineC(0+(counter-1)&255,0,WorkWindowWidth-1,WorkWindowHeight-1,0);DrawLineC(0+(counter++)&255,0,WorkWindowWidth-1,WorkWindowHeight-1,255);DrawLineC(0,WorkWindowHeight-1,WorkWindowWidth-1,0,255); ShowWorkWindow(x);}

//---------------------------------------------------------------
//debug stuff
void	cross_work_window(void)
{
	DrawLineC(0,0,WorkWindowWidth-1,WorkWindowHeight-1,WHITE_COL);
	DrawLineC(0,WorkWindowHeight-1,WorkWindowWidth-1,0,WHITE_COL);
	
}
extern	void	scan_apply_ambient(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra);

//---------------------------------------------------------------


#define	CTRL_PRIM_LOAD_BACKGROUND	1
#define	CTRL_PRIM_SAVE				2
#define	CTRL_PRIM_X_AXIS_FREE		3
#define	CTRL_PRIM_Y_AXIS_FREE		4
#define	CTRL_PRIM_Z_AXIS_FREE		5
#define	CTRL_PRIM_GRID_ON			6
#define	CTRL_PRIM_ERASE_MAP			7
#define	CTRL_PRIM_V_SLIDE_PRIM		8
#define	CTRL_PRIM_MODE_MENU			9
#define	CTRL_PRIM_MODE_TEXT			10
#define	CTRL_PRIM_REPLACE			11

#define CTRL_PRIM_COLLIDE_NONE		12
#define CTRL_PRIM_COLLIDE_BOX		13
#define CTRL_PRIM_COLLIDE_CYLINDER	14
#define CTRL_PRIM_COLLIDE_SMALLBOX	15

#define CTRL_PRIM_SHADOW_NONE		16
#define CTRL_PRIM_SHADOW_BOXEDGE	17
#define CTRL_PRIM_SHADOW_CYLINDER	18
#define CTRL_PRIM_SHADOW_FOURLEGS	19
#define CTRL_PRIM_SHADOW_FULLBOX	20
#define CTRL_PRIM_FLAG_LAMPOST		21
#define CTRL_PRIM_FLAG_GLARE		22
#define CTRL_PRIM_GRID_MAX			23
#define CTRL_PRIM_GRID_CORNER		24
#define CTRL_PRIM_FLAG_ON_FLOOR		25
#define CTRL_PRIM_FLAG_TREE			26
#define CTRL_PRIM_VIEW_SIDE			27
#define CTRL_PRIM_FLAG_JUST_FLOOR	28
#define CTRL_PRIM_FLAG_INSIDE		29

#define CTRL_PRIM_GROW				30
#define CTRL_PRIM_SHRINK			31

#define CTRL_PRIM_DAMAGABLE			32
#define CTRL_PRIM_LEANS				33
#define CTRL_PRIM_CRUMPLES			34
#define CTRL_PRIM_EXPLODES			35
#define CTRL_PRIM_CENTRE_PIVOT		36

MenuDef2		prim_mode_menu[]	=
{
	{"Single Prims"},{"Multi Prims"},{"unused"},{"Anim Prims"},{"Morph Prims"},{"!"}
};


ControlDef	prim_pick_tab_def[]	=	
{
	{	BUTTON,			0,	"Load A BackGround",		10,		473,	0,	0	},
	{	BUTTON,			0,	"Save selected prim",		10,		485,	0,	0	},
	{	CHECK_BOX,		0,	"X Free",					10,		310,	0,	10	},
	{	CHECK_BOX,		0,	"Y Free",					10,		323,	0,	10	},
	{	CHECK_BOX,		0,	"Z Free",					10,		336,	0,	10	},
	{	CHECK_BOX,		0,	"Grid Mode",				10,		362,	0,	10	},
	{	BUTTON,			0,	"Remove Prim",					190,	401,	0,	0	},
	{	V_SLIDER,		0,	"",		   					272,	40,		0,	257	},

	{	PULLDOWN_MENU,	0,	"Prim mode",				10,		10,		0,	0,	prim_mode_menu},
	{	STATIC_TEXT,	0,	"Current mode : ",			10,		24,		0,	0	},
	{	BUTTON,			0,	"Replace Selected Prim"	,	10,		460,	0,	0	},

	{	CHECK_BOX,		0,	"Collide none",				100,	310,	0,	10	},
	{	CHECK_BOX,		0,	"Collide box",				100,	323,	0,	10	},
	{	CHECK_BOX,		0,	"Collide cylinder",			100,	336,	0,	10	},
	{	CHECK_BOX,		0,	"Collide small box",		100,	349,	0,	10	},

	{	CHECK_BOX,		0,	"Shadow none",				190,	310,	0,	10	},
	{	CHECK_BOX,		0,	"Shadow box edge",			190,	323,	0,	10	},
	{	CHECK_BOX,		0,	"Shadow cylinder",			190,	336,	0,	10	},
	{	CHECK_BOX,		0,	"Shadow fourlegs",			190,	349,	0,	10	},
	{	CHECK_BOX,		0,	"Shadow full box",			190,	362,	0,	10	},

	{	CHECK_BOX,		0,	"Flag lampost",				100,	362,	0,	10	},
	{	CHECK_BOX,		0,	"Flag GLARE",				100,	375,	0,	10	},

	{	CHECK_BOX,		0,	"RHS",						10,		349,	0,	10	},
	{	CHECK_BOX,		0,	"Corners",					10,		375,	0,	10	},
	{	CHECK_BOX,		0,	"Flag on Roof/floor",		100,	388,	0,	10	},
	{	CHECK_BOX,		0,	"Flag tree",				100,    414,	0,	10  },
	{	CHECK_BOX,		0,	"Side View",				200,    440,	0,	10  },
	{	CHECK_BOX,		0,	"Flag on floor",			100,	401,	0,	10	},
	{	CHECK_BOX,		0,	"Inside",					200,	455,	0,	10	},

	{	BUTTON,			0,	"Grow",						140,	460,	0,	10	},
	{	BUTTON,			0,	"Shrink",					140,	473,	0,	10	},

	{	CHECK_BOX,		0,	"Damagable",				10,		395,	0,	10	},
	{	CHECK_BOX,		0,	"Leans",					10,		408,	0,	10	},
	{	CHECK_BOX,		0,	"Crumples",					10,		421,	0,	10	},
	{	CHECK_BOX,		0,	"Explodes",					10,     434,	0,	10  },

	{	BUTTON,			0,	"Centre Pivot",				140,	486,	0,	10	},

	{	0	}
};

PrimPickTab	*the_primpicktab;
void	redraw_all_prims(void);

UWORD	prim_count[256];
UWORD	prim_diff=0;
//---------------------------------------------------------------


/*
static	SLONG	angle_x=0;
void	set_user_rotate(SLONG	ax,SLONG	ay,SLONG	az)
{
	angle_x=ax;

	// Stop the compiler complaining.
	ay	=	ay;
	az	=	az;
}


void	apply_user_rotates(struct PrimPoint *point)
{
	SLONG	rx,rz;
	rx=point->X*COS(angle_x)-point->Z*SIN(angle_x);
	rz=point->X*SIN(angle_x)+point->Z*COS(angle_x);

	point->X=rx>>16;
	point->Z=rz>>16;
}
*/
SLONG	angle_prim_y=0;
void	rotate_prim_thing(UWORD thing)
{
	
//	anglex=map_things[thing].AngleY;
	if(LastKey==KB_H)
	{
		LastKey=0;
		angle_prim_y+=64;
		angle_prim_y=((angle_prim_y+2048)&2047);
	}
	if(LastKey==KB_J)
	{
		LastKey=0;
		angle_prim_y-=64;
		angle_prim_y=((angle_prim_y+2048)&2047);
	}
	map_things[thing].AngleY=angle_prim_y;

	/*
	SLONG	c0;
	struct	PrimObject	*p_obj,*p_obj_o;
	struct	PrimPoint	p;
	SLONG	sp,ep,offset_p;

	p_obj    =&prim_objects[map_things[thing].IndexOther];
	p_obj_o  =&prim_objects[map_things[thing].IndexOrig];

	if( (p_obj->EndPoint-p_obj->StartPoint)!=(p_obj_o->EndPoint-p_obj_o->StartPoint) )
		return;


	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;

	offset_p=sp-p_obj_o->StartPoint;

	if(LastKey==KB_H)
	{
		LastKey=0;
		angle_x+=64;
		angle_x=((angle_x+2048)&2047);
	}
	if(LastKey==KB_J)
	{
		LastKey=0;
		angle_x-=64;
		angle_x=((angle_x+2048)&2047);
	}
	
	for(c0=sp;c0<ep;c0++)
	{
		p=prim_points[c0-offset_p];
		apply_user_rotates(&p);
		prim_points[c0]=p;
	}

	map_things[thing].AngleX=angle_x;
	*/
}


PrimPickTab::PrimPickTab(EditorModule *parent)
{
	Parent=parent;
	the_primpicktab=this;

	PrimRect.SetRect(14,40,257,257);
	InitControlSet(prim_pick_tab_def);
	ListPos=0;
	CurrentPrim=0;
	UpdatePrimInfo();
	AxisMode=3;
	GridFlag=0;
	GridMax=0;
	GridCorner=0;
	View2Mode=0;

	PrimTabMode	=	PRIM_MODE_SINGLE;

	SetControlState(CTRL_PRIM_X_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_PRIM_Y_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_PRIM_Z_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_PRIM_VIEW_SIDE,     CTRL_DESELECTED);

	((CVSlider*)GetControlPtr(CTRL_PRIM_V_SLIDE_PRIM))->SetValueRange(0,266/3);
	((CVSlider*)GetControlPtr(CTRL_PRIM_V_SLIDE_PRIM))->SetCurrentValue(0);
	((CVSlider*)GetControlPtr(CTRL_PRIM_V_SLIDE_PRIM))->SetUpdateFunction(redraw_all_prims);
	Axis=X_AXIS|Y_AXIS|Z_AXIS;
	PrimScale=2688;
	BackScale=40;
	UpdatePrimInfo();
}

PrimPickTab::~PrimPickTab()
{
	UBYTE	blah	=	1;
}

//---------------------------------------------------------------

void	PrimPickTab::UpdatePrimInfo(void)
{
	//
	// Checks/unchecks the collide buttons.
	// 

	SetControlState(CTRL_PRIM_COLLIDE_NONE,     CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_COLLIDE_BOX,      CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_COLLIDE_CYLINDER, CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_COLLIDE_SMALLBOX, CTRL_DESELECTED);

	if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
	{
		switch(prim_objects[CurrentPrim].coltype)
		{
			case PRIM_COLLIDE_NONE:		SetControlState(CTRL_PRIM_COLLIDE_NONE,     CTRL_SELECTED); break;
			case PRIM_COLLIDE_BOX:		SetControlState(CTRL_PRIM_COLLIDE_BOX,      CTRL_SELECTED); break;
			case PRIM_COLLIDE_CYLINDER:	SetControlState(CTRL_PRIM_COLLIDE_CYLINDER, CTRL_SELECTED); break;
			case PRIM_COLLIDE_SMALLBOX:	SetControlState(CTRL_PRIM_COLLIDE_SMALLBOX, CTRL_SELECTED); break;
		}
	}

	//
	// Checks/unchecks the shadow buttons.
	// 

	SetControlState(CTRL_PRIM_SHADOW_NONE,	   CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_SHADOW_BOXEDGE,  CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_SHADOW_CYLINDER, CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_SHADOW_FOURLEGS, CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_SHADOW_FULLBOX,  CTRL_DESELECTED);

	if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
	{
		switch(prim_objects[CurrentPrim].shadowtype)
		{
			case PRIM_SHADOW_NONE:	   SetControlState(CTRL_PRIM_SHADOW_NONE,	  CTRL_SELECTED); break;
			case PRIM_SHADOW_BOXEDGE:  SetControlState(CTRL_PRIM_SHADOW_BOXEDGE,  CTRL_SELECTED); break;
			case PRIM_SHADOW_CYLINDER: SetControlState(CTRL_PRIM_SHADOW_CYLINDER, CTRL_SELECTED); break;
			case PRIM_SHADOW_FOURLEGS: SetControlState(CTRL_PRIM_SHADOW_FOURLEGS, CTRL_SELECTED); break;
			case PRIM_SHADOW_FULLBOX:  SetControlState(CTRL_PRIM_SHADOW_FULLBOX,  CTRL_SELECTED); break;
		}
	}

	//
	// The prim flags.
	// 

	SetControlState(CTRL_PRIM_FLAG_LAMPOST,  CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_FLAG_GLARE, CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_FLAG_ON_FLOOR, CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_FLAG_TREE,     CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_FLAG_JUST_FLOOR, CTRL_DESELECTED);

	if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
	{
		if (prim_objects[CurrentPrim].flag & PRIM_FLAG_LAMPOST )	{SetControlState(CTRL_PRIM_FLAG_LAMPOST,    CTRL_SELECTED);}
		if (prim_objects[CurrentPrim].flag & PRIM_FLAG_GLARE)		{SetControlState(CTRL_PRIM_FLAG_GLARE,      CTRL_SELECTED);}
		if (prim_objects[CurrentPrim].flag & PRIM_FLAG_ON_FLOOR)	{SetControlState(CTRL_PRIM_FLAG_ON_FLOOR,   CTRL_SELECTED);}
		if (prim_objects[CurrentPrim].flag & PRIM_FLAG_JUST_FLOOR)	{SetControlState(CTRL_PRIM_FLAG_JUST_FLOOR, CTRL_SELECTED);}
		if (prim_objects[CurrentPrim].flag & PRIM_FLAG_TREE)		{SetControlState(CTRL_PRIM_FLAG_TREE,       CTRL_SELECTED);}
	}

	if (edit_info.Inside )
	{
		SetControlState(CTRL_PRIM_FLAG_INSIDE,  CTRL_SELECTED);
	}
	else
	{
		SetControlState(CTRL_PRIM_FLAG_INSIDE,  CTRL_DESELECTED);
	}

	//
	// Checks/unchecks the damageable flags.
	// 

	SetControlState(CTRL_PRIM_DAMAGABLE, CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_LEANS,     CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_CRUMPLES,  CTRL_DESELECTED);
	SetControlState(CTRL_PRIM_EXPLODES,  CTRL_DESELECTED);

	if (prim_objects[CurrentPrim].damage & PRIM_DAMAGE_DAMAGABLE)
	{
		SetControlState(CTRL_PRIM_DAMAGABLE, CTRL_SELECTED);
		
		if (prim_objects[CurrentPrim].damage & PRIM_DAMAGE_LEAN)     {SetControlState(CTRL_PRIM_LEANS,    CTRL_SELECTED);}
		if (prim_objects[CurrentPrim].damage & PRIM_DAMAGE_CRUMPLE)  {SetControlState(CTRL_PRIM_CRUMPLES, CTRL_SELECTED);}
		if (prim_objects[CurrentPrim].damage & PRIM_DAMAGE_EXPLODES) {SetControlState(CTRL_PRIM_EXPLODES, CTRL_SELECTED);}
	}

	//
	// This crashes?!...
	// 

//	((CStaticText*)GetControlPtr(CTRL_PRIM_MODE_TEXT))->SetString1(prim_mode_menu[PrimTabMode].ItemText);
}

//---------------------------------------------------------------

void	PrimPickTab::DrawTabContent(void)
{
	EdRect		content_rect;

	if(PrimTabMode==PRIM_MODE_SINGLE)
		((CVSlider*)GetControlPtr(CTRL_PRIM_V_SLIDE_PRIM))->SetValueRange(0,266/3);
	else
		((CVSlider*)GetControlPtr(CTRL_PRIM_V_SLIDE_PRIM))->SetValueRange(0,next_prim_multi_object+9);

	content_rect	=	ContentRect;	
	content_rect.ShrinkRect(1,1);
	content_rect.FillRect(CONTENT_COL);

//	ShowAngles();
/*
	if(ValueButton)
		ValueButton->DrawValueGadget();
*/

	DrawPrims();
	DrawControlSet();
}

//---------------------------------------------------------------

void	PrimPickTab::DrawAPrimInRect(ULONG prim,SLONG x,SLONG y,SLONG w,SLONG h)
{
	CBYTE	*text;
	SLONG	*flags; //[560];
	struct	SVector			*res; //[560]; //max points per object?
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;
	SLONG	min_x=999999,max_x=-999999,min_y=999999,max_y=-999999;
	SLONG	width,height,scale,scale_y;
	EdRect	rect;
	SLONG	os;

	flags=(SLONG*)MemAlloc(sizeof(SLONG)*13000);
	if(flags==0)
		return;
		
	res=(struct SVector*)MemAlloc(sizeof(struct SVector)*13000);
	if(res==0)
	{
		MemFree(flags);
		return;
	}
	

	os=engine.Scale;
	engine.Scale=1000;

//set clip to content window
	SetWorkWindowBounds	(
							ContentLeft()+PrimRect.GetLeft()+x+1,
							ContentTop()+PrimRect.GetTop()+y+1,
							w,
							h
						);

	rect.SetRect(0,0,w,h);
	if(prim==CurrentPrim)
		rect.FillRect(LOLITE_COL);

//	rect.HiliteRect(LOLITE_COL,LOLITE_COL);

//	SetWorkWindowBounds(ContentLeft()+x,ContentTop()+y,w,h);

/*
	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);

	rect.SetRect(x,y,w,h);
	if(prim==CurrentPrim)
		rect.FillRect(HILITE_COL);
	else
		rect.FillRect(LOLITE_COL);
	rect.HiliteRect(LOLITE_COL,LOLITE_COL);

	SetWorkWindowBounds(ContentLeft()+x,ContentTop()+y,w,h);
*/
		set_camera();

//	p_rect->FillRect(CONTENT_COL);
	p_obj    =&prim_objects[prim];

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;
		
	for(c0=sp;c0<ep;c0++)
	{
		struct SVector	pp;

		pp.X=prim_points[c0].X;
		pp.Y=prim_points[c0].Y;
		pp.Z=prim_points[c0].Z;

		//transform all points for this Object
		flags[c0-sp]=rotate_point_gte(&pp,&res[c0-sp]);
		if(res[c0-sp].X<min_x)
			min_x=res[c0-sp].X;

		if(res[c0-sp].X>max_x)
			max_x=res[c0-sp].X;

		if(res[c0-sp].Y<min_y)
			min_y=res[c0-sp].Y;

		if(res[c0-sp].Y>max_y)
			max_y=res[c0-sp].Y;
	}
	width=max_x-min_x;
	height=max_y-min_y;

	if(width==0||height==0)
	{
	}
	else
	{
		CBYTE	str[100];

		scale  =(w<<16)/width;
		scale_y=(h<<16)/height;

		if(scale_y<scale)
			scale=scale_y;

		scale=(scale*200)>>8;
		engine.Scale=(1000*scale)>>16;
		draw_a_prim_at(prim,0,0,0,0);
		render_view(1);

		text	=	prim_names[prim];

		sprintf(str,"(%d,%d) %s",prim,prim<256?prim_count[prim]:0,text);
		DrawBoxC(rect.GetLeft()+1,rect.GetTop()+1,QTStringWidth(text),QTStringHeight()+1,0);
		QuickText(rect.GetLeft()+1,rect.GetTop()+1,str,TEXT_COL);
		rect.OutlineRect(0);

	}

	engine.Scale=os;

	MemFree(res);
	MemFree(flags);
}

void	PrimPickTab::DrawABuildingInRect(ULONG prim,SLONG x,SLONG y,SLONG w,SLONG h)
{
	CBYTE	*text;
	SLONG	*flags; //[560];
	struct	SVector			*res; //[560]; //max points per object?
	SLONG	c0;
	struct	BuildingObject	*p_obj;
	SLONG	sp,ep;
	SLONG	min_x=999999,max_x=-999999,min_y=999999,max_y=-999999;
	SLONG	width,height,scale,scale_y;
	EdRect	rect;
	SLONG	os;

	flags=(SLONG*)MemAlloc(sizeof(SLONG)*3000);
	if(flags==0)
		return;
		
	res=(struct SVector*)MemAlloc(sizeof(struct SVector)*3000);
	if(res==0)
	{
		MemFree(flags);
		return;
	}
	

	os=engine.Scale;
	engine.Scale=1000;

	SetWorkWindowBounds	(
							ContentLeft()+PrimRect.GetLeft()+x+1,
							ContentTop()+PrimRect.GetTop()+y+1,
							w,
							h
						);

	rect.SetRect(0,0,w,h);
	if(prim==CurrentPrim)
		rect.FillRect(LOLITE_COL);

		set_camera();

	p_obj    =&building_objects[prim];

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;

	LogText(" build in rect  prim %d sp %d ep %d \n",prim,sp,ep);
		
	for(c0=sp;c0<ep;c0++)
	{
		struct	SVector	pp;
		pp.X=prim_points[c0].X;
		pp.Y=prim_points[c0].Y;
		pp.Z=prim_points[c0].Z;

		//transform all points for this Object
		flags[c0-sp]=rotate_point_gte(&pp,&res[c0-sp]);
		if(res[c0-sp].X<min_x)
			min_x=res[c0-sp].X;

		if(res[c0-sp].X>max_x)
			max_x=res[c0-sp].X;

		if(res[c0-sp].Y<min_y)
			min_y=res[c0-sp].Y;

		if(res[c0-sp].Y>max_y)
			max_y=res[c0-sp].Y;
	}
	width=max_x-min_x;
	height=max_y-min_y;
	LogText(" build widh %d height %d \n",width,height);

	if(width==0||height==0)
	{
	}
	else
	{
		scale  =(w<<16)/width;
		scale_y=(h<<16)/height;

		if(scale_y<scale)
			scale=scale_y;

		scale=(scale*200)>>8;
		engine.Scale=(1000*scale)>>16;
		draw_a_building_at(prim,0,0,0);
		render_view(1);

//		text	=	prim_objects[prim].ObjectName;
//		DrawBoxC(rect.GetLeft()+1,rect.GetTop()+1,QTStringWidth(text),QTStringHeight()+1,0);
//		QuickText(rect.GetLeft()+1,rect.GetTop()+1,text,TEXT_COL);
		rect.OutlineRect(0);

		engine.Scale=os;
	}

	MemFree(res);
	MemFree(flags);
}

//---------------------------------------------------------------

void	PrimPickTab::DrawAMultiPrimInRect(ULONG prim,SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG					c0,c1,c2,
							num_points,
							max_x,max_y,
							min_x,min_y,
							end_point,
							scale,
							scale_y,
							start_point,
							temp_scale,
							temp_x,
							temp_y,
							temp_z,
							width,height,
							*flags;
	EdRect					bounds_rect,
							outline_rect;
	struct KeyFrame			*the_frame;
	struct KeyFrameElement	*the_element;
	struct Matrix31			offset;
	struct Matrix33			r_matrix;
	struct PrimObject		*the_obj;
	struct SVector			*rotate_vectors,
							t_vector,
							t_vector2;


// This bit bodged in for now.
extern struct KeyFrameChunk 	*test_chunk;
	if(!test_chunk->MultiObject)
		return;
	the_frame	=	&test_chunk->KeyFrames[0];
//

	c1	=	0;
	flags			=	(SLONG*)MemAlloc(sizeof(SLONG)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");
	rotate_vectors	=	(struct SVector*)MemAlloc(sizeof(struct SVector)*3000);
	ERROR_MSG(flags,"Unable to allocate memory for DrawKeyFrame");

	min_x	=	min_y	=	999999;
	max_x	=	max_y	=	-999999;

	temp_scale		=	engine.Scale;
	temp_x			=	engine.X;
	temp_y			=	engine.Y;
	temp_z			=	engine.Z;

	engine.X		=	0;
	engine.Y		=	0;
	engine.Z		=	0;
	engine.Scale	=	5000;
	engine.ShowDebug=	0;
	engine.BucketSize=	MAX_BUCKETS>>4;

	SetWorkWindowBounds	(
							ContentLeft()+PrimRect.GetLeft()+x+1,
							ContentTop()+PrimRect.GetTop()+y+1,
							w,
							h
						);
	bounds_rect.SetRect(0,0,w,h);
	if(prim==CurrentPrim)
		bounds_rect.FillRect(LOLITE_COL);

/*
	outline_rect.SetRect(
							0,0,
							bounds_rect.GetWidth(),bounds_rect.GetHeight()
						);
*/
	set_camera();
	set_camera_to_base();

	rotate_obj(0,0,0,&r_matrix);

	for(c2=0,c0=prim_multi_objects[prim].StartObject;c0<prim_multi_objects[prim].EndObject;c0++)
	{
		the_obj		=	&prim_objects[c0];
		start_point	=	the_obj->StartPoint;
		end_point	=	the_obj->EndPoint;
		num_points	=	end_point-start_point;

		the_element	=	&the_frame->FirstElement[c2++];
		offset.M[0]	=	the_element->OffsetX;
		offset.M[1]	=	the_element->OffsetY;
		offset.M[2]	=	the_element->OffsetZ;
		matrix_transformZMY((struct Matrix31*)&t_vector,&r_matrix, &offset);

		engine.X	-=	t_vector.X<<8;
		engine.Y	-=	t_vector.Y<<8;
		engine.Z	-=	t_vector.Z<<8;

		for(c1=0;c1<num_points;c1++)
		{
			matrix_transform_small((struct Matrix31*)&t_vector2,&r_matrix,(struct SMatrix31*)&prim_points[c1]);
			flags[c1]	=	rotate_point_gte(&t_vector2,&rotate_vectors[c1]);
			if(rotate_vectors[c1].X<min_x)
				min_x	=	rotate_vectors[c1].X;
			if(rotate_vectors[c1].X>max_x)
				max_x	=	rotate_vectors[c1].X;
			if(rotate_vectors[c1].Y<min_y)
				min_y	=	rotate_vectors[c1].Y;
			if(rotate_vectors[c1].Y>max_y)
				max_y	=	rotate_vectors[c1].Y;
		}

		engine.X	+=	t_vector.X<<8;
		engine.Y	+=	t_vector.Y<<8;
		engine.Z	+=	t_vector.Z<<8;
	}

	width	=	max_x-min_x;
	height	=	max_y-min_y;

	if(width>0 && height>0)
	{
		scale	=	(bounds_rect.GetWidth()<<16)/width;
		scale_y	=	(bounds_rect.GetHeight()<<16)/height;
		if(scale_y<scale)
			scale	=	scale_y;

		scale			=	(scale*900)>>8;
		engine.Scale	=	(5000*scale)>>16;
	}

	for(c2=0,c0=prim_multi_objects[prim].StartObject;c0<prim_multi_objects[prim].EndObject;c0++)
	{
		the_element			=	&the_frame->FirstElement[c2++];
		test_draw(c0,0,0,0,0,the_element,the_element,&r_matrix, NULL, NULL, NULL, NULL, NULL);
	}
	render_view(1);
	bounds_rect.OutlineRect(0);

	engine.X		=	temp_x;
	engine.Y		=	temp_y;
	engine.Z		=	temp_z;
	engine.Scale	=	temp_scale;
	engine.ShowDebug=	1;
	engine.BucketSize=	MAX_BUCKETS;

	MemFree(rotate_vectors);
	MemFree(flags);
}

//---------------------------------------------------------------

// for both views
SLONG	PrimPickTab::HiLightObjects(SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG	mx,my,mz;
	SLONG	screen_change=0;
	SLONG	wwx,wwy,www,wwh;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;
	
	mx=(engine.X>>8)>>ELE_SHIFT;
	my=(engine.Y>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;

	SetWorkWindowBounds(x,y,w-1,h/2-3);
	set_camera_plan();

	screen_change=hilight_map_things(MAP_THING_TYPE_PRIM);
	screen_change|=hilight_map_things(MAP_THING_TYPE_ANIM_PRIM);

 	SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
	if(View2Mode)
	{
 		set_camera_side();
	}
	else
	{
 		set_camera_front();
	}

	screen_change|=hilight_map_things(MAP_THING_TYPE_PRIM);
	screen_change|=hilight_map_things(MAP_THING_TYPE_ANIM_PRIM);

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

	return(screen_change);
}

void	PrimPickTab::DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG				wwx,
						wwy,
						www,
						wwh;
	EdRect		drawrect;


	RedrawModuleContent=0;


	// Back up clipping rect.
	wwx	=	WorkWindowRect.Left;
	wwy	=	WorkWindowRect.Top;
	www	=	WorkWindowRect.Width;
	wwh	=	WorkWindowRect.Height;
/*
	View1.SetRect(x,y,w-1,h/2-8);
	View2.SetRect(x,y+h/2+8,w-1,h/2-16);

	SetWorkWindowBounds	(
							View1.GetLeft(),
							View1.GetTop(),
							View1.GetWidth(),
							View1.GetHeight()
						);
	
	View1.FillRect(CONTENT_COL);
	View1.OutlineRect(0);
	
	SetWorkWindowBounds	(
							View2.GetLeft(),
							View2.GetTop(),
							View2.GetWidth(),
							View2.GetHeight()
						);
	
	View2.FillRect(CONTENT_COL);
	View2.OutlineRect(0);
*/	

	View1.SetRect(x,y,w-1,h/2-3);
	View2.SetRect(x,y+h/2+4,w-1,h/2-4);

	SetWorkWindowBounds(x,y,w-1,h/2-3);
	drawrect.SetRect(0,0,w-1,h/2-3);

	drawrect.FillRect(CONTENT_COL_BR);
	drawrect.HiliteRect(HILITE_COL,HILITE_COL);
	set_camera_plan();
extern	void	find_map_clip(SLONG *minx,SLONG *maxx,SLONG *minz,SLONG *maxz);
	{

		SLONG	minx,maxx,minz,maxz;

		find_map_clip(&minx,&maxx,&minz,&maxz);
		edit_info.MinX=minx;
		edit_info.MinZ=minz;
		edit_info.MaxX=maxx;
		edit_info.MaxZ=maxz;
		edit_info.Clipped|=1;

	}

	draw_editor_map(0);
	render_view(1);

	switch(PrimTabMode)
	{
		case PRIM_MODE_SINGLE:
		case PRIM_MODE_MULTI:
		case PRIM_MODE_ANIM_KEY:
		case PRIM_MODE_ANIM_MORPH:
			hilight_map_things(MAP_THING_TYPE_PRIM);
			hilight_map_things(MAP_THING_TYPE_ANIM_PRIM);
			break;
//		case PRIM_MODE_BACK:
//			hilight_map_backgrounds(0);
			break;
	}

	SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
	drawrect.SetRect(0,0,w-1,h/2-4);

//		drawrect.FillRect(LOLITE_COL);
	drawrect.FillRect(CONTENT_COL_BR);
	drawrect.HiliteRect(HILITE_COL,HILITE_COL);

	if(View2Mode)
	{
 		set_camera_side();
	}
	else
	{
 		set_camera_front();
	}


	draw_editor_map(0);

	render_view(1);
	switch(PrimTabMode)
	{
		case PRIM_MODE_SINGLE:
		case PRIM_MODE_MULTI:
		case PRIM_MODE_ANIM_KEY:
		case PRIM_MODE_ANIM_MORPH:
			hilight_map_things(MAP_THING_TYPE_PRIM);
			hilight_map_things(MAP_THING_TYPE_ANIM_PRIM);
			break;
//		case PRIM_MODE_BACK:
//			hilight_map_backgrounds(0);
			break;
	}

	{
		CBYTE str[50];

		sprintf(str, "Current prim %d", CurrentPrim);

		QuickTextC(10, 40, str, RED_COL);
		QuickTextC(11, 41, str, WHITE_COL);
	}

	// Restore clipping rect.
	SetWorkWindowBounds(wwx,wwy,www,wwh);
	edit_info.Clipped&=~1;
}

void	PrimPickTab::DrawPrims(void)
{
	SLONG	ox,oy,oz,x,y,prim=1;
	SLONG	wwx,wwy,www,wwh;


	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);

	PrimRect.FillRect(ACTIVE_COL);
	PrimRect.HiliteRect(LOLITE_COL,HILITE_COL);

	{
		SLONG	c0;

		memset(prim_count,0,512);
		prim_diff=0;

		for(c0=0;c0<MAX_MAP_THINGS;c0++)
		{
			struct	MapThing	*t_mthing;
			t_mthing=&map_things[c0];

			switch(t_mthing->Type)
			{
				case	MAP_THING_TYPE_PRIM:		
					if(t_mthing->IndexOther<256)
					{
						prim_count[t_mthing->IndexOther]++;
						if(prim_count[t_mthing->IndexOther]==1)
							prim_diff++;
					}
					break;
			}
		}
	}
	{
		CBYTE	str[100];
		sprintf(str," %d..%d     DIFFERENT PRIMS %d",next_prim_point,MAX_PRIM_POINTS,prim_diff);
		QuickTextC(1,1,str,0);
	}

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;
//	SetWorkWindowBounds(PrimRect.GetLeft()+1,PrimRect.GetTop()+1,PrimRect.GetWidth()-1,PrimRect.GetHeight()-1);

//	PrimRect.FillRect(CONTENT_COL);
//	PrimRect.HiliteRect(LOLITE_COL,LOLITE_COL);
	ox=engine.X;
	oy=engine.Y;
	oz=engine.Z;

	engine.X=0;
	engine.Y=0;
	engine.Z=0;

	engine.ShowDebug=0;
	engine.BucketSize=MAX_BUCKETS>>4;

  	prim =	(((CVSlider*)GetControlPtr(CTRL_PRIM_V_SLIDE_PRIM))->GetCurrentValue()*3)+1;

	if(PrimTabMode==PRIM_MODE_ANIM_KEY)
	{
		EdRect		frame_rect,rect;
		struct Matrix33			r_matrix;
		rotate_obj(0,0,0,&r_matrix);

		for(y=1;y<255;y+=85)
		for(x=1;x<255;x+=85)
		{
			SetWorkWindowBounds	(ContentLeft()+PrimRect.GetLeft()+x+1,
								ContentTop()+PrimRect.GetTop()+y+1,
								85,85);

			rect.SetRect(0,0,85,85);
			if(prim==CurrentPrim)
				rect.FillRect(LOLITE_COL);

extern	void	drawkeyframeboxgamechunk(UWORD multi_object,EdRect *bounds_rect,struct GameKeyFrame *the_frame,struct Matrix33 *r_matrix,SLONG person_id,struct GameKeyFrameChunk *the_chunk);

			if(anim_chunk[prim].MultiObject[0])
				drawkeyframeboxgamechunk(anim_chunk[prim].MultiObject[0],&rect,anim_chunk[prim].AnimList[1],&r_matrix,0,&anim_chunk[prim]);
			prim++;
		}
	}
	else
	if(PrimTabMode==PRIM_MODE_SINGLE)
	{
		EdRect		frame_rect,rect;
		struct Matrix33			r_matrix;
		rotate_obj(0,0,0,&r_matrix);

		for(y=1;y<255;y+=85)
		for(x=1;x<255;x+=85)
		{

			if(prim < next_prim_object)
				DrawAPrimInRect(prim++,x,y,85,85);

		}
	}
	else
	{
		for(y=1;y<255;y+=85)
		for(x=1;x<255;x+=85)
		{
			if(prim < next_prim_multi_object)
				DrawAMultiPrimInRect(prim++,x,y,85,85);
		}
	}

	engine.ShowDebug=1;
	engine.BucketSize=MAX_BUCKETS;

//	draw_a_prim_at(3,0,0,0);

	engine.X=ox;
	engine.Y=oy;
	engine.Z=oz;
/*	
	{
	SetWorkWindowBounds(ContentLeft(),ContentTop(),300,300);
		CBYTE	str[100];
		sprintf(str,"CURRENT_PRIM= %d dtv1 %d dtv2 %d x %d y %d z %d",CurrentPrim,DragThingView1,DragThingView2,engine.MousePosX,engine.MousePosY,engine.MousePosZ);
		DrawBox(20,20,300,10,0);
		QuickTextC(20+1,20+1,str,255);
	}
*/
	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
	RedrawTabContent=0;

}

void	PrimPickTab::UpdatePrimPickWindow(void)
{
	if(LockWorkScreen())
	{
		DrawControlSet();
		DrawPrims();
		UnlockWorkScreen();
	}
	ShowWorkWindow(0);
}


void	redraw_all_prims(void)
{
	the_primpicktab->DrawTabContent();
}
//---------------------------------------------------------------


void	PrimPickTab::HandleTab(MFPoint *current_point)
{
	SLONG		   update	=	0;
	

	ModeTab::HandleTab(current_point);
/*
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
*/
	if(Keys[KB_U])
	{
		Keys[KB_U]=0;
		MyUndo.DoUndo(ShiftFlag?1:0);
		RedrawModuleContent=1;
	}

	if(RedrawTabContent||update)
	{
		UpdatePrimPickWindow();
	}
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


SLONG	PrimPickTab::KeyboardInterface(void)
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
				SetControlState(CTRL_PRIM_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_PRIM_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_PRIM_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=X_AXIS;
				break;
			case	1:
				SetControlState(CTRL_PRIM_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_PRIM_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_PRIM_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=Y_AXIS;
				break;
			case	2:
				SetControlState(CTRL_PRIM_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_PRIM_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_PRIM_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=Z_AXIS;
				break;
			case	3:
				SetControlState(CTRL_PRIM_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_PRIM_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_PRIM_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=X_AXIS|Y_AXIS|Z_AXIS;
				break;
		}

		SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
  		DrawControlSet();
		ShowWorkWindow(0);
	}
	return(0);
}




void	find_things_min_point(SLONG drag,SLONG *px,SLONG *py,SLONG *pz)
{
	SLONG	c0;
	SLONG	mx,my,mz;

	*px=99999;
	*py=99999;
	*pz=99999;

	if(drag)
	{
		SLONG	sp,ep;
		sp=prim_objects[drag].StartPoint;
		ep=prim_objects[drag].EndPoint;

		for(c0=sp;c0<ep;c0++)
		{

			if(prim_points[c0].X<*px)
				*px=prim_points[c0].X;
			if(prim_points[c0].Y<*py)
				*py=prim_points[c0].Y;
			if(prim_points[c0].Z<*pz)
				*pz=prim_points[c0].Z;
		}
	}
}
void	find_things_max_point(SLONG drag,SLONG *px,SLONG *py,SLONG *pz)
{
	SLONG	c0;
	SLONG	mx,my,mz;

	*px=-99999;
	*py=-99999;
	*pz=-99999;

	if(drag)
	{
		SLONG	sp,ep;
		sp=prim_objects[drag].StartPoint;
		ep=prim_objects[drag].EndPoint;

		for(c0=sp;c0<ep;c0++)
		{

			if(prim_points[c0].X>*px)
				*px=prim_points[c0].X;
			if(prim_points[c0].Y>*py)
				*py=prim_points[c0].Y;
			if(prim_points[c0].Z>*pz)
				*pz=prim_points[c0].Z;
		}
	}
}

void	find_things_min_point_corner(SLONG drag,SLONG *px,SLONG *py,SLONG *pz)
{
	SLONG	c0;
	SLONG	mx,my,mz;
	SLONG	dist,mdist=99999,best=0;

	*py=99999;

	if(drag)
	{
		SLONG	sp,ep;
		sp=prim_objects[drag].StartPoint;
		ep=prim_objects[drag].EndPoint;

		for(c0=sp;c0<ep;c0++)
		{
			if(prim_points[c0].Y<*py)
				*py=prim_points[c0].Y;

			dist=prim_points[c0].X+prim_points[c0].Z;

			if(dist<mdist)
			{
				mdist=dist;
				best=c0;
			}
		}
	}
	if(best)
	{
		*px=prim_points[best].X;
//		*py=prim_points[best].Y;
		*pz=prim_points[best].Z;
	}
}

void	find_things_max_point_corner(SLONG drag,SLONG *px,SLONG *py,SLONG *pz)
{
	SLONG	c0;
	SLONG	mx,my,mz;
	SLONG	dist,mdist=-99999,best=0;

	*py=-99999;

	if(drag)
	{
		SLONG	sp,ep;
		sp=prim_objects[drag].StartPoint;
		ep=prim_objects[drag].EndPoint;

		for(c0=sp;c0<ep;c0++)
		{
			if(prim_points[c0].Y>*py)
				*py=prim_points[c0].Y;

			dist=prim_points[c0].X+prim_points[c0].Z;

			if(dist>mdist)
			{
				mdist=dist;
				best=c0;
			}
		}
	}
	if(best)
	{
		*px=prim_points[best].X;
//		*py=prim_points[best].Y;
		*pz=prim_points[best].Z;
	}
}

//
// button 0 is left
// button 1 is right (for creating a new prim identical to the last

SLONG	PrimPickTab::DragAPrim(UBYTE flags,MFPoint *clicked_point,SLONG button)
{
	static	UBYTE col=0;
	SLONG	screen_change=0;
	MFPoint		local_point;
	SLONG	x,y,w,h;
	SLONG	drag;
	SLONG	wwx,wwy,www,wwh;
	SLONG	ox,oy,oz;

	SLONG	px=0,py=0,pz=0;

	


	// Stop compiler moaning.	
	flags	=	flags;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	col++;
	
	SetWorkWindowBounds(x,y,w-1,h/2-3);
	set_camera_plan();

	local_point	=	*clicked_point;
/*
	switch(PrimTabMode)
	{
		case PRIM_MODE_SINGLE:
			drag=select_map_things(clicked_point,MAP_THING_TYPE_PRIM);
			break;
		case PRIM_MODE_MULTI:
			break;
		case PRIM_MODE_SINGLE:
		case PRIM_MODE_ANIM_KEY:
		case PRIM_MODE_ANIM_MORPH:

			drag=select_map_things(clicked_point,MAP_THING_TYPE_PRIM);
			if(drag==0)
				drag=select_map_things(clicked_point,MAP_THING_TYPE_ANIM_PRIM);
			break;
//		case PRIM_MODE_BACK:
//			drag=select_map_backgrounds(clicked_point,0);
			break;
	}
	*/

extern	void	find_map_clip(SLONG *minx,SLONG *maxx,SLONG *minz,SLONG *maxz);
	{

		SLONG	minx,maxx,minz,maxz;

		find_map_clip(&minx,&maxx,&minz,&maxz);
		edit_info.MinX=minx;
		edit_info.MinZ=minz;
		edit_info.MaxX=maxx;
		edit_info.MaxZ=maxz;
		edit_info.Clipped|=1;

	}

	drag=select_map_things(clicked_point,MAP_THING_TYPE_PRIM);
	if(drag==0)
		drag=select_map_things(clicked_point,MAP_THING_TYPE_ANIM_PRIM);
	

	if(!drag)
	{
		SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
		if(View2Mode)
		{
 			set_camera_side();
		}
		else
		{
 			set_camera_front();
		}
		local_point	  =*clicked_point;
		local_point.Y-=h/2;
		drag=select_map_things(&local_point,MAP_THING_TYPE_PRIM);
		if(drag==0)
			drag=select_map_things(&local_point,MAP_THING_TYPE_ANIM_PRIM);
/*
		switch(PrimTabMode)
		{
			case PRIM_MODE_SINGLE:
			case PRIM_MODE_MULTI:
			case PRIM_MODE_ANIM_KEY:
			case PRIM_MODE_ANIM_MORPH:
				drag=select_map_things(&local_point,MAP_THING_TYPE_PRIM);
				if(drag==0)
					drag=select_map_things(&local_point,MAP_THING_TYPE_ANIM_PRIM);
				break;
//			case PRIM_MODE_BACK:
//				drag=select_map_backgrounds(&local_point,0);
				break;
		}
*/
	}

	if(drag)
	{
		SLONG	index;

		index=map_things[drag].IndexOther;
		if(GridCorner)
		{
			if(GridMax)
			{
				find_things_max_point_corner(index,&px,&py,&pz);
			}
			else
			{
				find_things_min_point_corner(index,&px,&py,&pz);
			}
		}
		else
		{
			if(GridMax)
			{
				find_things_max_point(index,&px,&py,&pz);
			}
			else
			{
				find_things_min_point(index,&px,&py,&pz);
			}
		}
	}

	engine.MousePosX=map_things[drag].X;
	engine.MousePosY=map_things[drag].Y;
	engine.MousePosZ=map_things[drag].Z;
	ox=map_things[drag].X;
	oy=map_things[drag].Y;
	oz=map_things[drag].Z;

	

	if(drag)  //drag in plan view
	{
		SLONG	offset_x,offset_y,offset_z;
		SLONG	in=1;

		if(button)
		{
			SLONG	old_thing;
			old_thing=drag;
			// right button so create a new identical prim and drag it

			drag=place_prim_at(map_things[drag].IndexOther,map_things[drag].X,map_things[drag].Y,map_things[drag].Z);
			if(drag==0)
				return(0);

			map_things[drag].AngleY=map_things[old_thing].AngleY;
			map_things[drag].X-engine.MousePosX;
		}

		SetWorldMouse(0);
		offset_x=map_things[drag].X-engine.MousePosX;
		offset_y=map_things[drag].Y-engine.MousePosY;
		offset_z=map_things[drag].Z-engine.MousePosZ;
//		set_user_rotate(map_things[drag].AngleX,0,0);

		angle_prim_y=map_things[drag].AngleY;
		

		while(SHELL_ACTIVE && ( (button==0) ? LeftButton : RightButton))
		{
			SLONG	nx,ny,nz;
			in=SetWorldMouse(0);

			nx=map_things[drag].X;
			ny=map_things[drag].Y;
			nz=map_things[drag].Z;

			if(GridFlag)
			{
				SLONG	grid_and;
				grid_and=~(HALF_ELE_SIZE-1);
				if(Axis&X_AXIS)
					nx=((engine.MousePosX+offset_x)&grid_and)-px;

				if(py<0)
				{
					if(Axis&Y_AXIS)
						ny=((engine.MousePosY+offset_y)&grid_and)+((-py)&0x7f);
				}
				else
				{
					if(Axis&Y_AXIS)
						ny=((engine.MousePosY+offset_y)&grid_and)-((py)&0x7f);
				}
				if(Axis&Z_AXIS)
					nz=((engine.MousePosZ+offset_z)&grid_and)-(pz);
			}
			else
			{
				if(Axis&X_AXIS)
					nx=engine.MousePosX+offset_x;
				if(Axis&Y_AXIS)
					ny=engine.MousePosY+offset_y;
				if(Axis&Z_AXIS)
					nz=engine.MousePosZ+offset_z;
			}

			if(PrimTabMode==PRIM_MODE_BACK)
			{
				map_things[drag].X=nx;
				map_things[drag].Y=ny;
				map_things[drag].Z=nz;
//				LogText("Drag To nx %d ny %d nz %d \n",nx,ny,nz);
			}
			else
				move_thing_on_cells(drag,nx,ny,nz);

			DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			ShowWorkWindow(0);
			screen_change=1;

			SetWorkWindowBounds(this->ContentLeft()+1,this->ContentTop()+1,this->ContentWidth(),this->ContentHeight()); 
//			ContentRect.FillRect(CONTENT_COL);

			DrawBox(0,0,this->ContentWidth(),this->ContentHeight(),CONTENT_COL);
			set_camera();
			draw_editor_map(0);
			render_view(1);
			ShowWorkWindow(0);
			editor_user_interface(0);
			KeyboardInterface();
			if(PrimTabMode!=PRIM_MODE_BACK)
				rotate_prim_thing(drag);
		}
		if(!in)
		{
			delete_thing(drag);
		}

		MyUndo.MoveObject(0,drag,ox,oy,oz);

	}
	SetWorkWindowBounds(this->ContentLeft()+1,this->ContentTop()+1,this->ContentWidth(),this->ContentHeight()); 
	DrawBox(0,0,this->ContentWidth(),this->ContentHeight(),CONTENT_COL);
//	ContentRect.FillRect(CONTENT_COL);
	UpdatePrimPickWindow();

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
	edit_info.Clipped&=~1;

	return(screen_change);
}

SLONG	PrimPickTab::DragEngine(UBYTE flags,MFPoint *clicked_point)
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

extern	SLONG	calc_edit_height_at(SLONG x,SLONG z);

SLONG	PrimPickTab::HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h)
{
	SWORD	thing;


	// Stop compiler moaning.	
	x	=	x;
	y	=	y;
	w	=	w;
	h	=	h;

	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			if(CurrentPrim==0)
			{
				DragAPrim(flags,clicked_point,0);
			}
			else
			{
				SetWorldMouse(1);
//				set_user_rotate(0,0,0);
				angle_prim_y=0;
				switch(PrimTabMode)
				{
					case	PRIM_MODE_SINGLE:

						if(ShiftFlag|| (prim_objects[CurrentPrim].flag & PRIM_FLAG_ON_FLOOR) )
						{
							SLONG	px,py,pz,y;
							find_things_min_point(CurrentPrim,&px,&py,&pz);

extern	SLONG find_alt_for_this_pos(SLONG  x,SLONG  z);
							y=find_alt_for_this_pos(engine.MousePosX,engine.MousePosZ);
							//y=calc_edit_height_at(engine.MousePosX,engine.MousePosZ);

							y-=py;
							thing=place_prim_at(CurrentPrim,engine.MousePosX,y,engine.MousePosZ);
							if(ShiftFlag)
								map_things[thing].Flags|=FLAG_EDIT_PRIM_ON_FLOOR;
						}
						else
						if(ShiftFlag|| (prim_objects[CurrentPrim].flag & PRIM_FLAG_JUST_FLOOR) )
						{
							SLONG	px,py,pz,y;
							find_things_min_point(CurrentPrim,&px,&py,&pz);

extern	SLONG find_alt_for_this_pos(SLONG  x,SLONG  z);
							//y=find_alt_for_this_pos(engine.MousePosX,engine.MousePosZ);
							y=calc_edit_height_at(engine.MousePosX,engine.MousePosZ);

							y-=py;
							thing=place_prim_at(CurrentPrim,engine.MousePosX,y,engine.MousePosZ);
//							map_things[thing].Flags|=FLAG_EDIT_PRIM_ON_FLOOR;
						}
						else
						{
							thing=place_prim_at(CurrentPrim,engine.MousePosX,engine.MousePosY,engine.MousePosZ);
						}
						break;
					case	PRIM_MODE_ANIM_KEY:
SLONG	place_anim_prim_at(UWORD prim,SLONG x,SLONG y,SLONG z);
						if(ShiftFlag)
						{
							SLONG	px,py,pz,y;
/* HERE HERE HERE */		find_things_min_point(CurrentPrim,&px,&py,&pz);

extern	SLONG	find_alt_for_this_pos(SLONG x,SLONG z);

							y=find_alt_for_this_pos(px,pz);
//							y=find_alt_for_this_pos(engine.MousePosX,engine.MousePosZ);

//							y=calc_edit_height_at(engine.MousePosX,engine.MousePosZ);
							y-=py;
							thing=place_prim_at(CurrentPrim,engine.MousePosX,y,engine.MousePosZ);
						}
						else
						{

							thing=place_anim_prim_at(CurrentPrim,engine.MousePosX,engine.MousePosY,engine.MousePosZ);
						}
						break;

				}

				MyUndo.PlaceObject(0,CurrentPrim,thing,engine.MousePosX,engine.MousePosY,engine.MousePosZ);
				CurrentPrim=0;
				UpdatePrimInfo();
				return(1);
			}
			break;
		case	RIGHT_CLICK:
			if(CurrentPrim==0)
				DragAPrim(flags,clicked_point,1);

			// Right click in content.
			break;
		case	MIDDLE_CLICK:
				DragEngine(flags,clicked_point);
			break;
	}
	return(0);
	
}

UWORD	PrimPickTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
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
			if(PrimRect.PointInRect(&local_point))
			{
				SLONG	max,
						x,
						y,

						prim	=	(((CVSlider*)GetControlPtr(CTRL_PRIM_V_SLIDE_PRIM))->GetCurrentValue()*3)+1;

				switch(PrimTabMode)
				{
					case	PRIM_MODE_SINGLE:
						max	=	next_prim_object; //next_prim_object
						max=266;
						break;
					case	PRIM_MODE_MULTI:
						max	=	next_prim_multi_object;
						break;
					case	PRIM_MODE_ANIM_KEY:
					case	PRIM_MODE_ANIM_MORPH:
						max=256;
				}

				for(y=1;y<255;y+=85)
				for(x=1;x<255;x+=85)
				{
					if(prim < max)
						if(is_point_in_box(local_point.X,local_point.Y,PrimRect.GetLeft()+x,PrimRect.GetTop()+y,85,85))
						{
							if(CurrentPrim!=prim)
							{
								RedrawTabContent=1;
								CurrentPrim=prim;
								UpdatePrimInfo();
								extern SLONG HMTAB_current_prim;
								HMTAB_current_prim = prim;
							}
							else
							{
								CurrentPrim=0;
								UpdatePrimInfo();
							}

							UpdatePrimPickWindow();

						}
					prim++;
				}
			}
			else
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

SLONG	PrimPickTab::SetWorldMouse(ULONG flag)
{
	MFPoint		mouse_point;
	MFPoint		local_point;
	SLONG	wwx,wwy,www,wwh;


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
		return(1);
	}
	else
	if(is_point_in_box(local_point.X,local_point.Y,0,Parent->ContentHeight()/2,Parent->ContentWidth()-1,Parent->ContentHeight()/2))
	{
		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+Parent->ContentHeight()/2+3,Parent->ContentWidth()-1,Parent->ContentHeight()/2-4);
		if(View2Mode)
		{
 			set_camera_side();
		}
		else
		{
 			set_camera_front();
		}
		calc_world_pos_front(local_point.X,local_point.Y-Parent->ContentHeight()/2);
		if(flag)
			engine.MousePosZ=engine.Z>>8;
		SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
		return(1);

	}
/*
		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth()-1,Parent->ContentHeight()/2-3);
		set_camera_plan();
		point.X=engine.MousePosX;
		point.Y=engine.MousePosY;
		point.Z=engine.MousePosZ;

		rotate_point_gte(&point,&out);

		DrawLineC(out.X-10,out.Y,out.X+10,out.Y,1);
		DrawLineC(out.X,out.Y-10,out.X,out.Y+10,1);

		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+Parent->ContentHeight()/2+3,Parent->ContentWidth()-1,Parent->ContentHeight()/2-4);
		set_camera_front();
		point.X=engine.MousePosX;
		point.Y=engine.MousePosY;
		point.Z=engine.MousePosZ;

		rotate_point_gte(&point,&out);

		DrawLineC(out.X-10,out.Y,out.X+10,out.Y,1);
		DrawLineC(out.X,out.Y-10,out.X,out.Y+10,1);
		ShowWorkWindow(0);
*/
	return(0);
}



void	add_a_background_thing(UWORD prim,SLONG x,SLONG y,SLONG z)
{
	UWORD	map_thing;
	struct	MapThing	*p_mthing;

	map_thing=find_empty_map_thing();
	if(!map_thing)
		return;
	p_mthing=TO_MTHING(map_thing);
	p_mthing->X=x;
	p_mthing->Y=y;
	p_mthing->Z=z;
	
	p_mthing->IndexOther=prim;
	p_mthing->IndexNext=background_prim;
	background_prim=map_thing;
	set_things_faces(map_thing);
	p_mthing->Type=MAP_THING_TYPE_PRIM;
	scan_function=scan_apply_ambient;
	scan_map_thing(map_thing);
	
}

extern		void	clear_map2(void);

void	PrimPickTab::HandleControl(UWORD control_id)
{
	switch(control_id&0xff)
	{
		/*
		case	CTRL_PRIM_APPEND_NEW:
			{
				FileRequester	*fr;
				CBYTE	fname[100];

				clear_map2();
//				clear_prims();
				load_all_prims("allprim.sav");

				fr=new FileRequester("objects\\","*.*","Load A Prim","hello");
				if(fr->Draw())
				{
					
					strcpy(fname,fr->Path);
					strcat(fname,fr->FileName);
					read_asc(fname,100,1);
					compress_prims();
					record_prim_status();
					save_all_prims("allprim.sav");
				}
				delete fr;
				UpdatePrimPickWindow();
				RequestUpdate();
			}
			break;
		*/
		case	CTRL_PRIM_REPLACE:
			if(CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				FileRequester	*fr;
				CBYTE	fname[100];

				clear_map2();
//				clear_prims();
				load_all_individual_prims();

				fr=new FileRequester("objects\\","*.*","Load A Prim","hello");
				if(fr->Draw())
				{
					SLONG	temp;
					
					strcpy(fname,fr->Path);
					strcat(fname,fr->FileName);
					temp=next_prim_object;
					next_prim_object=CurrentPrim;

					read_asc(fname,100,1);
					next_prim_object=temp;
					compress_prims();
					record_prim_status();

					save_prim_object(CurrentPrim);

					//save_all_individual_prims();
				}
				delete fr;
				UpdatePrimPickWindow();
				RequestUpdate();
			}
			break;
		case	CTRL_PRIM_SAVE:

			if (CurrentPrim)
			{
//				revert_to_prim_status();
				save_prim_object(CurrentPrim);
				//save_all_individual_prims();
			}
			break;
		case	CTRL_PRIM_X_AXIS_FREE:
			ToggleControlSelectedState(CTRL_PRIM_X_AXIS_FREE);
			if(Axis&X_AXIS)
				Axis&=~X_AXIS;
			else
				Axis|=X_AXIS;
			break;
		case	CTRL_PRIM_Y_AXIS_FREE:
			ToggleControlSelectedState(CTRL_PRIM_Y_AXIS_FREE);
			if(Axis&Y_AXIS)
				Axis&=~Y_AXIS;
			else
				Axis|=Y_AXIS;
			break;
		case	CTRL_PRIM_Z_AXIS_FREE:
			ToggleControlSelectedState(CTRL_PRIM_Z_AXIS_FREE);
			if(Axis&Z_AXIS)
				Axis&=~Z_AXIS;
			else
				Axis|=Z_AXIS;
			break;
		case	CTRL_PRIM_GRID_ON:
			ToggleControlSelectedState(CTRL_PRIM_GRID_ON);
			GridFlag^=1;
			break;
		case	CTRL_PRIM_GRID_MAX:
			ToggleControlSelectedState(CTRL_PRIM_GRID_MAX);
			GridMax^=1;
			break;
		case	CTRL_PRIM_GRID_CORNER:
			ToggleControlSelectedState(CTRL_PRIM_GRID_CORNER);
			GridCorner^=1;
			break;
		case	CTRL_PRIM_LOAD_BACKGROUND:
/*
			{
				FileRequester	*fr;
				CBYTE	fname[100];
				fr=new FileRequester("data\\","*.asc","Load A Prim","hello");
				if(fr->Draw())
				{
					UWORD	temp;
					strcpy(fname,fr->Path);
					strcat(fname,fr->FileName);
					read_asc(fname,640,1);
					temp=copy_prim_to_end(next_prim_object-1,0,0);
					add_a_background_thing(temp,HALF_ELE_SIZE+(512<<ELE_SHIFT),HALF_ELE_SIZE+(64<<ELE_SHIFT),HALF_ELE_SIZE+(3<<ELE_SHIFT));
//					add_a_background_thing(next_prim_object-1,HALF_ELE_SIZE+(512<<ELE_SHIFT),HALF_ELE_SIZE+(64<<ELE_SHIFT),HALF_ELE_SIZE+(3<<ELE_SHIFT));
					delete_last_prim();
				}
				UpdatePrimPickWindow();
				delete fr;
				RequestUpdate();
				edit_info.amb_dx=-128;
				edit_info.amb_dy=128;
				edit_info.amb_dz=-128;
				edit_info.amb_bright=256;
				edit_info.amb_offset=0;
//				edit_info.amb_flag=1;

//				scan_map();	

			}
*/
			break;
		case	CTRL_PRIM_ERASE_MAP:
			if(CurrentPrim && CurrentPrim<256&&prim_count[CurrentPrim])
			{
				SLONG	c0;

				for(c0=0;c0<MAX_MAP_THINGS;c0++)
				{
					struct	MapThing	*t_mthing;
					t_mthing=&map_things[c0];

					switch(t_mthing->Type)
					{
						case	MAP_THING_TYPE_PRIM:		
							if(t_mthing->IndexOther==CurrentPrim)
							{
								delete_thing(c0);
								prim_count[CurrentPrim]=0;
							}
							break;
					}
				}
			}

//			save_map("data/bak.map",1);
//			clear_map();
//			RequestUpdate();


			break;
		case	CTRL_PRIM_MODE_MENU:
			PrimTabMode	=	(control_id>>8)-1;
			switch(PrimTabMode)
			{
				case PRIM_MODE_SINGLE:
				case PRIM_MODE_MULTI:
				case PRIM_MODE_ANIM_KEY:
				case PRIM_MODE_ANIM_MORPH:
//					BackScale=engine.Scale;
//					engine.Scale=PrimScale;
//					RequestUpdate();
					break;
//				case PRIM_MODE_BACK:
//					PrimScale=engine.Scale;
//					engine.Scale=BackScale;
//					RequestUpdate();
//					break;
			}
			UpdatePrimInfo();
			break;
		case	CTRL_PRIM_MODE_TEXT:
			break;

/*
			AngleY++;
			break;
		case	CTRL_CAM_BUTTON_ZPLUS:
			AngleZ++;
			break;
		case	CTRL_CAM_BUTTON_XMINUS:
			AngleX--;
			break;
		case	CTRL_CAM_BUTTON_YMINUS:
			AngleY--;
			break;
		case	CTRL_CAM_BUTTON_ZMINUS:
			AngleZ--;
			break;
*/
		case	CTRL_PRIM_COLLIDE_NONE:
			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				prim_objects[CurrentPrim].coltype = PRIM_COLLIDE_NONE;
				UpdatePrimInfo();
			}
			break;
		case	CTRL_PRIM_COLLIDE_BOX:
			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				prim_objects[CurrentPrim].coltype = PRIM_COLLIDE_BOX;
				UpdatePrimInfo();
			}
			break;
		case	CTRL_PRIM_COLLIDE_CYLINDER:
			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				prim_objects[CurrentPrim].coltype = PRIM_COLLIDE_CYLINDER;
				UpdatePrimInfo();
			}
			break;

		case	CTRL_PRIM_COLLIDE_SMALLBOX:
			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				prim_objects[CurrentPrim].coltype   = PRIM_COLLIDE_SMALLBOX;
				UpdatePrimInfo();
			}
			break;

		case CTRL_PRIM_SHADOW_NONE:     if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE) {prim_objects[CurrentPrim].shadowtype = PRIM_SHADOW_NONE;}     UpdatePrimInfo(); break;
		case CTRL_PRIM_SHADOW_BOXEDGE:  if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE) {prim_objects[CurrentPrim].shadowtype = PRIM_SHADOW_BOXEDGE;}  UpdatePrimInfo(); break;
		case CTRL_PRIM_SHADOW_CYLINDER: if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE) {prim_objects[CurrentPrim].shadowtype = PRIM_SHADOW_CYLINDER;} UpdatePrimInfo(); break;
		case CTRL_PRIM_SHADOW_FOURLEGS: if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE) {prim_objects[CurrentPrim].shadowtype = PRIM_SHADOW_FOURLEGS;} UpdatePrimInfo(); break;
		case CTRL_PRIM_SHADOW_FULLBOX:  if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE) {prim_objects[CurrentPrim].shadowtype = PRIM_SHADOW_FULLBOX;}  UpdatePrimInfo(); break;

		case CTRL_PRIM_FLAG_LAMPOST:  if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE) {prim_objects[CurrentPrim].flag ^= PRIM_FLAG_LAMPOST;  UpdatePrimInfo();} break;
		case CTRL_PRIM_FLAG_GLARE: if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE) {prim_objects[CurrentPrim].flag ^= PRIM_FLAG_GLARE; UpdatePrimInfo();} break;
		case CTRL_PRIM_FLAG_TREE:     if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE) {prim_objects[CurrentPrim].flag ^= PRIM_FLAG_TREE;     UpdatePrimInfo();} break;
		case CTRL_PRIM_FLAG_ON_FLOOR:
			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				
				prim_objects[CurrentPrim].flag ^= PRIM_FLAG_ON_FLOOR;
				if(prim_objects[CurrentPrim].flag & PRIM_FLAG_ON_FLOOR)
				{
					prim_objects[CurrentPrim].flag &=~ PRIM_FLAG_JUST_FLOOR;
				}
				UpdatePrimInfo();
			}
			break;
		case CTRL_PRIM_FLAG_JUST_FLOOR:
			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				
				prim_objects[CurrentPrim].flag ^= PRIM_FLAG_JUST_FLOOR;
				if(prim_objects[CurrentPrim].flag & PRIM_FLAG_JUST_FLOOR)
				{
					prim_objects[CurrentPrim].flag &=~ PRIM_FLAG_ON_FLOOR;
				}
				UpdatePrimInfo();
			}
			break;
		case CTRL_PRIM_FLAG_INSIDE:
			{
				
				edit_info.Inside^=1;
				UpdatePrimInfo();
			}
			break;

		case CTRL_PRIM_VIEW_SIDE:

			if(View2Mode==0)
			{
				View2Mode=1;
				SetControlState(CTRL_PRIM_VIEW_SIDE,     CTRL_SELECTED);
			}
			else
			{
				View2Mode=0;
				SetControlState(CTRL_PRIM_VIEW_SIDE,     CTRL_DESELECTED);

			}
			break;

		case CTRL_PRIM_GROW:

			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				SLONG i;
				PrimObject *po;

				//
				// Grow the selected prim.
				//

				po = &prim_objects[CurrentPrim];

				for (i = po->StartPoint; i < po->EndPoint; i++)
				{
					prim_points[i].X += prim_points[i].X >> 2;
					prim_points[i].Y += prim_points[i].Y >> 2;
					prim_points[i].Z += prim_points[i].Z >> 2;
				}

				void update_modules(void);

				update_modules();
			}

			break;

		case CTRL_PRIM_SHRINK:

			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				SLONG i;
				PrimObject *po;

				//
				// Shrink the selected prims.
				//

				po = &prim_objects[CurrentPrim];

				for (i = po->StartPoint; i < po->EndPoint; i++)
				{
					prim_points[i].X -= prim_points[i].X >> 2;
					prim_points[i].Y -= prim_points[i].Y >> 2;
					prim_points[i].Z -= prim_points[i].Z >> 2;
				}

				void update_modules(void);

				update_modules();
			}

			break;

		case CTRL_PRIM_DAMAGABLE:

			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				prim_objects[CurrentPrim].damage ^= PRIM_DAMAGE_DAMAGABLE;

				UpdatePrimInfo();
			}

			break;

		case CTRL_PRIM_LEANS:

			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				prim_objects[CurrentPrim].damage ^= PRIM_DAMAGE_LEAN;
				prim_objects[CurrentPrim].damage |= PRIM_DAMAGE_DAMAGABLE;

				if (prim_objects[CurrentPrim].damage & PRIM_DAMAGE_LEAN)
				{
					prim_objects[CurrentPrim].damage &= ~PRIM_DAMAGE_CRUMPLE;
				}

				UpdatePrimInfo();
			}

			break;

		case CTRL_PRIM_CRUMPLES:

			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				prim_objects[CurrentPrim].damage ^= PRIM_DAMAGE_CRUMPLE;
				prim_objects[CurrentPrim].damage |= PRIM_DAMAGE_DAMAGABLE;

				if (prim_objects[CurrentPrim].damage & PRIM_DAMAGE_CRUMPLE)
				{
					prim_objects[CurrentPrim].damage &= ~PRIM_DAMAGE_LEAN;
				}

				UpdatePrimInfo();
			}

			break;

		case CTRL_PRIM_EXPLODES:

			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				prim_objects[CurrentPrim].damage |= PRIM_DAMAGE_DAMAGABLE;
				prim_objects[CurrentPrim].damage ^= PRIM_DAMAGE_EXPLODES;

				UpdatePrimInfo();
			}
			
			break;

		case CTRL_PRIM_CENTRE_PIVOT:

			if (CurrentPrim && PrimTabMode == PRIM_MODE_SINGLE)
			{
				SLONG i;

				SLONG min_x = +INFINITY;
				SLONG min_y = +INFINITY;
				SLONG min_z = +INFINITY;

				SLONG max_x = -INFINITY;
				SLONG max_y = -INFINITY;
				SLONG max_z = -INFINITY;

				SLONG mid_x;
				SLONG mid_y;
				SLONG mid_z;

				PrimObject *po;

				//
				// Shrink the selected prims.
				//

				po = &prim_objects[CurrentPrim];

				for (i = po->StartPoint; i < po->EndPoint; i++)
				{
					if (prim_points[i].X < min_x) {min_x = prim_points[i].X;}
					if (prim_points[i].Y < min_y) {min_y = prim_points[i].Y;}
					if (prim_points[i].Z < min_z) {min_z = prim_points[i].Z;}

					if (prim_points[i].X > max_x) {max_x = prim_points[i].X;}
					if (prim_points[i].Y > max_y) {max_y = prim_points[i].Y;}
					if (prim_points[i].Z > max_z) {max_z = prim_points[i].Z;}
				}

				mid_x = min_x + max_x >> 1;
				mid_y = min_y + max_y >> 1;
				mid_z = min_z + max_z >> 1;

				for (i = po->StartPoint; i < po->EndPoint; i++)
				{
					prim_points[i].X -= mid_x;
					prim_points[i].Y -= mid_y;
					prim_points[i].Z -= mid_z;
				}

				void update_modules(void);

				update_modules();
			}

			break;
	}
}

//---------------------------------------------------------------
