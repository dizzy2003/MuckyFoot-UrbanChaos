#include	"Editor.hpp"

#include	"MapTab.hpp"
#include	"engine.h"
//#include	"collide.hpp"

static		counter;

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

#define	SELECT_WHOLE_THING					99

#define	MAP_MODE_WAIT						0
#define	MAP_MODE_DEFINE_MAP					1
#define	MAP_MODE_SELECT_AND_PLACE			2
#define	MAP_MODE_PLACE_CURRENT_MAP			3


#define	CTRL_MAP_X_AXIS_FREE		1
#define	CTRL_MAP_Y_AXIS_FREE		2
#define	CTRL_MAP_Z_AXIS_FREE		3
#define	CTRL_MAP_DEF_MODE			4
#define	CTRL_MAP_DEFINE_MAP			5
#define	CTRL_MAP_EDIT_TEXT			6
#define	CTRL_MAP_SELECT_AND_PLACE	7
#define	CTRL_MAP_ZOOM_1				8
#define	CTRL_MAP_ZOOM_2				9
#define	CTRL_MAP_ZOOM_3				10


ControlDef	map_tab_def[]	=	
{
	{	CHECK_BOX,	0,	"X Free",	120,	300-10,	0,	10					},
	{	CHECK_BOX,	0,	"Y Free",	120,	313-10,	0,	10					},
	{	CHECK_BOX,	0,	"Z Free",	120,	326-10,	0,	10					},
	{	CHECK_BOX,	0,	"Def Mode",	10,	10,	0,	10					},
	{	BUTTON,		0,	"Define A MapBlock"	,	100,		10,		0,	0},
	{	EDIT_TEXT,	0,	"",						150,	28,	70,		0			},
	{	BUTTON,		0,	"Select & Place MapBlock"	,	10,		50,		0,	0},
	{	BUTTON,	0,	"Zoom Close",	200,	300-10,	0,	10					},
	{	BUTTON,	0,	"Zoom Medium",	200,	313-10,	0,	10					},
	{	BUTTON,	0,	"Zoom Far",	200,	326-10,	0,	10					},
	{  	0	}
};


MapTab	*the_maptab;
	
struct	MapInfo	map_info[MAX_MAP_INFO];
UWORD	next_map_info=1;

void	redraw_map_tab(void);
//---------------------------------------------------------------

// MOVE THIS TO ANOTHER FILE
void	draw_world_map(void)
{
	SLONG	c0;
	struct	MapInfo	*p_map;
	SLONG	prim;

	for(c0=1;c0<next_map_info;c0++)
	{
		p_map=&map_info[c0];
		if(p_map->Background)
		if(p_map->X)
		{
			set_camera_angledy(p_map->AngleY);
			prim=map_things[p_map->Background].IndexOther;
			draw_a_prim_at(prim,p_map->X,p_map->Y,p_map->Z,1);
			clear_camera_angledy();
		}
	}
}




MapTab::MapTab(EditorModule *parent)
{
	Parent=parent;

	InitControlSet(map_tab_def);
	AxisMode=3;

	SetControlState(CTRL_MAP_X_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_MAP_Y_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_MAP_Z_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_MAP_DEF_MODE,CTRL_SELECTED);
	((CEditText*)GetControlPtr(CTRL_MAP_EDIT_TEXT))->SetFlags(CONTROL_INACTIVE);
	Axis=X_AXIS|Y_AXIS|Z_AXIS;
	Mode=0;
	DefMode=1;
//	map_info[0].Plane.Depth=320;
	the_maptab=this;
}

MapTab::~MapTab()
{
}

void	MapTab::Clear(void)
{
}

void	delete_map_info(SWORD index)
{
	SLONG	c0;
	for(c0=index;c0<next_map_info-1;c0++)
	{
		map_info[c0]=map_info[c0+1];
	}
	next_map_info--;
}

void	MapTab::Recalc(void)
{
}

void	MapTab::DrawTabContent(void)
{
	EdRect		content_rect;



	content_rect	=	ContentRect;	
	content_rect.ShrinkRect(1,1);
	content_rect.FillRect(CONTENT_COL);

	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
	DrawControlSet();
	ShowWorkWindow(0);
}

void	redraw_map_tab(void)
{

	switch(the_maptab->Mode)
	{
		

		default:
				
			the_maptab->DrawTabContent();
			the_maptab->Parent->DrawContent();
			SetWorkWindowBounds(the_maptab->Parent->GetLeft(),
								the_maptab->Parent->GetTop(),
								the_maptab->Parent->GetWidth(),
								the_maptab->Parent->GetHeight());
			break;
	}

}

//---------------------------------------------------------------
extern	void	hilight_map_info(UBYTE view_flag);

void	MapTab::DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG	wwx,wwy,www,wwh;
	EdRect	drawrect;

	RedrawModuleContent=0;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

		if(DefMode)
		{
			SetWorkWindowBounds(x,y,w-1,h-3);
			drawrect.SetRect(0,0,w-1,h-3);

			drawrect.FillRect(CONTENT_COL_BR);
			drawrect.HiliteRect(HILITE_COL,HILITE_COL);
			set_camera_front();
			draw_editor_map(0);
			render_view(0);
			hilight_map_info(0);

		}
		else
		{

			SetWorkWindowBounds(x,y,w-1,h/2-3);
			drawrect.SetRect(0,0,w-1,h/2-3);

			drawrect.FillRect(CONTENT_COL_BR);
			drawrect.HiliteRect(HILITE_COL,HILITE_COL);
			set_camera_plan();
//			draw_editor_map(0);
			draw_world_map();
			render_view(0);
			hilight_map_info(1);

			SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
			drawrect.SetRect(0,0,w-1,h/2-4);

			drawrect.FillRect(CONTENT_COL_BR);
			drawrect.HiliteRect(HILITE_COL,HILITE_COL);

			set_camera_front();
//			draw_editor_map(0);
			draw_world_map();
			render_view(0);
	//		hilight_map_things(MAP_THING_TYPE_MAP);
			hilight_map_info(1);
		}


	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
}

//---------------------------------------------------------------


void	MapTab::HandleTab(MFPoint *current_point)
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


SLONG	MapTab::KeyboardInterface(void)
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
				SetControlState(CTRL_MAP_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_MAP_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_MAP_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=X_AXIS;
				break;
			case	1:
				SetControlState(CTRL_MAP_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_MAP_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_MAP_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=Y_AXIS;
				break;
			case	2:
				SetControlState(CTRL_MAP_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_MAP_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_MAP_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=Z_AXIS;
				break;
			case	3:
				SetControlState(CTRL_MAP_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_MAP_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_MAP_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=X_AXIS|Y_AXIS|Z_AXIS;
				break;
		}

		SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
  		DrawControlSet();
		ShowWorkWindow(0);
	}
	return(0);
}

//#define	QDIST3(x,y,z)	(x>y ? (x>z ? x+(y>>2)+(z>>2) : z+(x>>2)+(y>>2)) : (y>z ? (y+(x>>2)+(z>>2) : z+(x>>2)+(y>>2) ))


inline	SLONG normalise_xyz(SLONG *x,SLONG *y,SLONG *z)
{
	SLONG	dist;

	dist=(*x)*(*x)+(*y)*(*y)+(*z)*(*z);
	dist=sqrl(dist);

	if(dist==0)
		dist=1;

	*x=(*x<<8)/dist;
	*y=(*y<<8)/dist;
	*z=(*z<<8)/dist;

	return(dist);
}


UWORD	CreateMapPlane(SLONG x,SLONG y,SLONG z)
{
	z=z;
	map_info[next_map_info].Left=x;
	map_info[next_map_info].Right=x+300;
	map_info[next_map_info].Top=y;
	map_info[next_map_info].Bottom=y+300;
//	map_info[next_map_info].Depth=z;
	next_map_info++;
	return(next_map_info-1);
}


/*
void	draw_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG col)
{
	struct	SVector	point[2];
	struct	SVector	res[2];
	SLONG	sx,sy,sz,c0;
	ULONG	f1,f2;
	point[0].X=x1;
	point[0].Y=y1;
	point[0].Z=z1;
	f1=rotate_point_gte(&point[0],&res[0]);

	point[1].X=x2;
	point[1].Y=y2;
	point[1].Z=z2;
	f2=rotate_point_gte(&point[1],&res[1]);
	f1=f1&f2;
	if(!(f1 & EF_CLIPFLAGS))
		DrawLineC(res[0].X,res[0].Y,res[1].X,res[1].Y,col);
}
*/
void	draw_3d_text(SLONG x1,SLONG y1,SLONG z1,CBYTE *str,SLONG col)
{
	struct	SVector	point;
	struct	SVector	res;
	ULONG	f1;
	point.X=x1;
	point.Y=y1;
	point.Z=z1;
	f1=rotate_point_gte(&point,&res);

	if(!(f1 & EF_CLIPFLAGS))
		QuickTextC(res.X,res.Y,str,col);
}

void	draw_a_map_info(UBYTE view_flag,UWORD index)
{
	struct	MapInfo	*p_map;
	SLONG	col=WHITE_COL;
	EdRect	rect;
	CBYTE	str[100];
	p_map=&map_info[index];

	if(view_flag)
	{

		if(p_map->Background)
			if(p_map->X)
			{
				
				set_camera_angledy(p_map->AngleY);
				calc_prims_screen_box(map_things[p_map->Background].IndexOther,p_map->X,p_map->Y,p_map->Z,&rect);
				clear_camera_angledy();
				rect.OutlineRect(WHITE_COL);

				sprintf(str," %s back %d index %d \n",p_map->Name,p_map->Background,index);
				QuickTextC(rect.GetLeft()+3,rect.GetTop()+3,str,col);
			}
	}
	else
	{
		draw_3d_line(p_map->Left,p_map->Top,0,p_map->Right,p_map->Top,0,col);
		draw_3d_line(p_map->Right,p_map->Top,0,p_map->Right,p_map->Bottom,0,col);
		draw_3d_line(p_map->Left,p_map->Bottom,0,p_map->Right,p_map->Bottom,0,col);
		draw_3d_line(p_map->Left,p_map->Bottom,0,p_map->Left,p_map->Top,0,col);
//		sprintf(str," %s back %d x %d y %d z %d \n",p_map->Name,p_map->Background,p_map->X,p_map->Y,p_map->Z);
		draw_3d_text(p_map->Left+3,p_map->Top+3,0,p_map->Name,col);

	}
}

void	hilight_map_info(UBYTE view_flag)
{
	SLONG	c0;
	if(next_map_info>1)
	for(c0=1;c0<next_map_info;c0++)
	{
		draw_a_map_info(view_flag,c0);
	}
}


static void	create_box_from_vect(EdRect *rect,SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
	struct	SVector	point[2];
	struct	SVector	res[2];
	ULONG	f1,f2;
	point[0].X=x1;
	point[0].Y=y1;
	point[0].Z=z1;
	f1=rotate_point_gte(&point[0],&res[0]);

	point[1].X=x2;
	point[1].Y=y2;
	point[1].Z=z2;
	f2=rotate_point_gte(&point[1],&res[1]);
//	f1=f1&f2;
	rect->SetRect(res[0].X,res[0].Y,res[1].X-res[0].X,res[1].Y-res[0].Y);
	rect->NormalRect();
	rect->SetRect(rect->GetLeft()-3,rect->GetTop()-3,rect->GetWidth()+6,rect->GetHeight()+6);
	rect->OutlineRect(WHITE_COL);
}

static void	create_box_from_point(EdRect *rect,SLONG x1,SLONG y1,SLONG z1)
{
	struct	SVector	point[1];
	struct	SVector	res[1];
	ULONG	f1,f2;
	point[0].X=x1;
	point[0].Y=y1;
	point[0].Z=z1;
	f1=rotate_point_gte(&point[0],&res[0]);

	rect->SetRect(res[0].X,res[0].Y,10,10);
	rect->NormalRect();
	rect->SetRect(rect->GetLeft()-10,rect->GetTop()-10,rect->GetWidth()+10,rect->GetHeight()+10);
	rect->OutlineRect(WHITE_COL);
}

	
SLONG	select_this_map_info(SLONG index,MFPoint *mouse)
{
	struct	MapInfo	*p_map;
	EdRect	rect;

	p_map=&map_info[index];

			//corners

		create_box_from_point(&rect,p_map->Left,p_map->Top,0);
		if(rect.PointInRect(mouse))
			return(5);
		create_box_from_point(&rect,p_map->Right,p_map->Top,0);
		if(rect.PointInRect(mouse))
			return(6);
		create_box_from_point(&rect,p_map->Right,p_map->Bottom,0);
		if(rect.PointInRect(mouse))
			return(7);
		create_box_from_point(&rect,p_map->Left,p_map->Bottom,0);
		if(rect.PointInRect(mouse))
			return(8);


		//edges
		create_box_from_vect(&rect,p_map->Left,p_map->Top,0,p_map->Right,p_map->Top,0);
		if(rect.PointInRect(mouse))
			return(1);
		create_box_from_vect(&rect,p_map->Right,p_map->Top,0,p_map->Right,p_map->Bottom,0);
		if(rect.PointInRect(mouse))
			return(2);
		create_box_from_vect(&rect,p_map->Left,p_map->Bottom,0,p_map->Right,p_map->Bottom,0);
		if(rect.PointInRect(mouse))
			return(3);
		create_box_from_vect(&rect,p_map->Left,p_map->Top,0,p_map->Left,p_map->Bottom,0);
		if(rect.PointInRect(mouse))
			return(4);

		//whole thing
		create_box_from_vect(&rect,p_map->Left,p_map->Top,0,p_map->Right,p_map->Bottom,0);
		if(rect.PointInRect(mouse))
			return(SELECT_WHOLE_THING);

	return(0);
}

extern	void	calc_prims_world_box(UWORD	prim,SLONG x,SLONG y,SLONG z, EdRect *rect);

void	calc_things_world_box(SLONG	map_thing,EdRect *rect)
{
	struct	MapThing	*p_mthing;

	p_mthing=TO_MTHING(map_thing);
	switch(p_mthing->Type)
	{
		case	MAP_THING_TYPE_PRIM:
			//3ds Prim Mesh 
			calc_prims_world_box(p_mthing->IndexOther,p_mthing->X,p_mthing->Y,p_mthing->Z,rect);
			break;
		case	MAP_THING_TYPE_LIGHT:
			break;

		case	MAP_THING_TYPE_SPRITE:
		case	MAP_THING_TYPE_AGENT:
			break;

	}
}


void	SetBackgroundForMap(SWORD map)
{
	SWORD	index;
	struct	MapThing	*p_thing;
	EdRect	rect;
	EdRect	map_rect;
	struct	MapInfo	*p_map;
	p_map=&map_info[map];

	map_rect.SetRect(p_map->Left,p_map->Top,p_map->Bottom-p_map->Top,p_map->Right-p_map->Left);
	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		calc_things_world_box(index,&rect);
		if(map_rect.IntersectRect(&rect))
		{
			p_map->Background=index;
			return;
		}
		index=p_thing->IndexNext;
	}

	
}


SLONG	select_map_info(MFPoint *mouse,SLONG *ret)
{
	SLONG	c0;

	for(c0=1;c0<next_map_info;c0++)
	{
		*ret=select_this_map_info(c0,mouse);
		if(*ret)
			return(c0);
	}
	return(0);
}

SLONG	select_map_infoxyz(MFPoint *mouse)
{
	SLONG	c0;
	struct	MapThing	*p_mthing;
	struct	MapInfo		*p_map;
	EdRect	rect;

	for(c0=1;c0<next_map_info;c0++)
	{
		p_map=&map_info[c0];

		if(p_map->Background)
		{

			p_mthing=TO_MTHING(p_map->Background);

			set_camera_angledy(p_map->AngleY);
			calc_prims_screen_box(p_mthing->IndexOther,p_map->X,p_map->Y,p_map->Z,&rect);
			clear_camera_angledy();
			rect.OutlineRect(WHITE_COL);

			if(rect.PointInRect(mouse))
			{
				return(c0);
			}
		}
	}
	return(0);
}


void	normal_info(struct MapInfo *p_map)
{
	if(p_map->Right<p_map->Left)
		SWAP(p_map->Left,p_map->Right);

	if(p_map->Bottom<p_map->Top)
		SWAP(p_map->Top,p_map->Bottom);


}	

SLONG	MapTab::DragAMapDef(UBYTE flags,MFPoint *clicked_point,UWORD copy)
{
	SLONG	side;
	SLONG	drag=0;
	SLONG	x,y,w,h;
	SLONG	wwx,wwy,www,wwh;
	SLONG	window=0;
	SLONG	col  = 0;
	SLONG	screen_change=0;
	SLONG	last_world_mouse;

	flags=flags;
	copy=copy;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	col++;

	SetWorkWindowBounds(x,y,w-1,h-3);
 	set_camera_front();
	drag=select_map_info(clicked_point,&side);


	if(drag)
	{
		struct	MapInfo	*p_map;
		p_map=&map_info[drag];
		
		CurrentMap=drag;

		while(SHELL_ACTIVE && LeftButton)
		{
			last_world_mouse=SetWorldMouse(0);

			if(last_world_mouse)
			{
				switch(side)
				{
					case 1: //top
						map_info[CurrentMap].Top=engine.MousePosY;
						break;
					case 3: //bot
						map_info[CurrentMap].Bottom=engine.MousePosY;
						break;
					case 2: //right
						map_info[CurrentMap].Right=engine.MousePosX;
						break;
					case 4: //left
						map_info[CurrentMap].Left=engine.MousePosX;
						break;
					case	5: //topleft
						map_info[CurrentMap].Top=engine.MousePosY;
						map_info[CurrentMap].Left=engine.MousePosX;
						break;
					case	6: //topright
						map_info[CurrentMap].Right=engine.MousePosX;
						map_info[CurrentMap].Top=engine.MousePosY;
						break;
					case	7: //botright
						map_info[CurrentMap].Right=engine.MousePosX;
						map_info[CurrentMap].Bottom=engine.MousePosY;
						break;
					case	8: //botleft
						map_info[CurrentMap].Left=engine.MousePosX;
						map_info[CurrentMap].Bottom=engine.MousePosY;
						break;
				}
			}

			DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			ShowWorkWindow(0);
			screen_change=1;

/*
			SetWorkWindowBounds(this->ContentLeft()+1,this->ContentTop()+1,this->ContentWidth(),this->ContentHeight()); 

			DrawBox(0,0,this->ContentWidth(),this->ContentHeight(),CONTENT_COL);

			set_camera();
			draw_editor_map(0);
			render_view(0);
			ShowWorkWindow(0);
*/
			editor_user_interface(0);
			KeyboardInterface();
		}
		normal_info(&map_info[CurrentMap]);

		SetBackgroundForMap(CurrentMap);

		if(!last_world_mouse)
		{	//delete col
			delete_map_info(drag);
			CurrentMap=0;
		}

		RequestUpdate();
	}
	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

	return(screen_change);

}

SLONG	MapTab::DragAMapDefXYZ(UBYTE flags,MFPoint *clicked_point,UWORD copy)
{
	SLONG	drag=0;
	SLONG	x,y,w,h;
	SLONG	wwx,wwy,www,wwh;
	SLONG	window=0;
	SLONG	col = 0;
	SLONG	screen_change=0;
	SLONG	last_world_mouse;
	SLONG	old_angley,old_mousex;
	MFPoint		local_point;

	flags=flags;
	copy=copy;


	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	col++;

	SetWorkWindowBounds(x,y,w-1,(h/2)-3);
 	set_camera_plan();
	drag=select_map_infoxyz(clicked_point);

	if(!drag)
	{
		SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
	 	set_camera_front();
		local_point	  =*clicked_point;
		local_point.Y-=h/2;
		drag=select_map_infoxyz(&local_point);
	}

	if(drag)
	{
		SLONG	dx,dy,dz;
		struct	MapInfo	*p_map;
		p_map=&map_info[drag];
		
		CurrentMap=drag;
		last_world_mouse=SetWorldMouse(0);
		dx=map_info[drag].X-engine.MousePosX;
		dy=map_info[drag].Y-engine.MousePosY;
		dz=map_info[drag].Z-engine.MousePosZ;

		engine.MousePosX=map_info[drag].X;
		engine.MousePosY=map_info[drag].Y;
		engine.MousePosZ=map_info[drag].Z;
		old_angley=map_info[drag].AngleY;
		old_mousex=MouseX;

		while(SHELL_ACTIVE && (LeftButton||RightButton))
		{
			SLONG	nx,ny,nz;
			last_world_mouse=SetWorldMouse(0);



			nx=map_info[drag].X;
			ny=map_info[drag].Y;
			nz=map_info[drag].Z;

			if(last_world_mouse)
			{
				if(Axis&X_AXIS)
					nx=engine.MousePosX+dx;
				if(Axis&Y_AXIS)
					ny=engine.MousePosY+dy;
				if(Axis&Z_AXIS)
					nz=engine.MousePosZ+dz;
				if(LeftButton)
				{
					map_info[CurrentMap].X=nx;
					map_info[CurrentMap].Y=ny;
					map_info[CurrentMap].Z=nz;
				}
				if(RightButton)
				{
					map_info[CurrentMap].AngleY=old_angley+((-MouseX+old_mousex)<<2);
					if(map_info[CurrentMap].AngleY<0)
						map_info[CurrentMap].AngleY=(2048+map_info[CurrentMap].AngleY)&2047;
				}
			}

			DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			ShowWorkWindow(0);
			screen_change=1;

			editor_user_interface(0);
			KeyboardInterface();
		}

		if(!last_world_mouse)
		{
			CurrentMap=0;
		}

		RequestUpdate();
	}
	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

	return(screen_change);

}

SLONG	MapTab::DragEngine(UBYTE flags,MFPoint *clicked_point)
{
	SLONG	wwx,wwy,www,wwh;
	SLONG	screen_change=0;
	SLONG	last_world_mouse;

	flags=flags;
	clicked_point=clicked_point;

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

void	MapTab::SetMapPos(SLONG x,SLONG y,SLONG z)
{
	map_info[CurrentMap].X=x;
	map_info[CurrentMap].Y=y;
	map_info[CurrentMap].Z=z;
	map_info[CurrentMap].AngleY=0;
}

SLONG	MapTab::HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h)
{
	x=x;
	y=y;
	w=w;
	h=h;
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:

			switch (Mode)
			{
				case	MAP_MODE_DEFINE_MAP:
					if(SetWorldMouse(1))
					{
						CurrentMap=CreateMapPlane(engine.MousePosX,engine.MousePosY,engine.MousePosZ);
						strcpy(map_info[CurrentMap].Name,((CEditText*)GetControlPtr(CTRL_MAP_EDIT_TEXT))->GetEditString());

						Mode=0;
					}
					return(1);

				case	MAP_MODE_WAIT:
					if(DefMode)
						DragAMapDef(flags,clicked_point,0);
					else
						DragAMapDefXYZ(flags,clicked_point,0);

					break;
				case	MAP_MODE_SELECT_AND_PLACE:
					{
						SLONG	side;
						CurrentMap=select_map_info(clicked_point,&side);
						DefMode=0;
						SetControlState(CTRL_MAP_DEF_MODE,CTRL_DESELECTED);
						Mode=MAP_MODE_PLACE_CURRENT_MAP;
						RequestUpdate();
					}
					return(1);
				 case	MAP_MODE_PLACE_CURRENT_MAP:
					if(SetWorldMouse(1))
					{
						SetMapPos(engine.MousePosX,engine.MousePosY,engine.MousePosZ);
						RequestUpdate();
						Mode=0;
					}
					break;

			}
			break;
		case	RIGHT_CLICK:
			switch (Mode)
			{
				case	MAP_MODE_WAIT:
					if(!DefMode)
						DragAMapDefXYZ(flags,clicked_point,0);
					break;
			}
			// Right click in content.
			break;
		case	MIDDLE_CLICK:
			DragEngine(flags,clicked_point);
			break;
	}
	return(0);
	
}

UWORD	MapTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
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

SLONG	MapTab::SetWorldMouse(ULONG flag)
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
	if(DefMode)
	{
		if(is_point_in_box(local_point.X,local_point.Y,0,0,Parent->ContentWidth()-1,Parent->ContentHeight()))
		{
			SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth()-1,Parent->ContentHeight()-4);
			set_camera_front();
			calc_world_pos_front(local_point.X,local_point.Y);
			if(flag)
				engine.MousePosZ=engine.Z>>8;
			SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
			return(1);

		}
		else
			return(0);
		
	}
	else
	{
		
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
			set_camera_front();
			calc_world_pos_front(local_point.X,local_point.Y-Parent->ContentHeight()/2);
			if(flag)
				engine.MousePosZ=engine.Z>>8;
			SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
			return(1);

		}
		else
			return(0);
	}

}


void	MapTab::HandleControl(UWORD control_id)
{
	switch(control_id&0xff)
	{

		case	CTRL_MAP_X_AXIS_FREE:
			ToggleControlSelectedState(CTRL_MAP_X_AXIS_FREE);
			if(Axis&X_AXIS)
				Axis&=~X_AXIS;
			else
				Axis|=X_AXIS;
			break;
		case	CTRL_MAP_Y_AXIS_FREE:
			ToggleControlSelectedState(CTRL_MAP_Y_AXIS_FREE);
			if(Axis&Y_AXIS)
				Axis&=~Y_AXIS;
			else
				Axis|=Y_AXIS;
			break;
		case	CTRL_MAP_Z_AXIS_FREE:
			ToggleControlSelectedState(CTRL_MAP_Z_AXIS_FREE);
			if(Axis&Z_AXIS)
				Axis&=~Z_AXIS;
			else
				Axis|=Z_AXIS;
			break;
		case	CTRL_MAP_DEFINE_MAP:
			DefMode=1;
			SetControlState(CTRL_MAP_DEF_MODE,CTRL_SELECTED);
			Mode=MAP_MODE_DEFINE_MAP;
			((CEditText*)GetControlPtr(CTRL_MAP_EDIT_TEXT))->SetFlags( ((CEditText*)GetControlPtr(CTRL_MAP_EDIT_TEXT))->GetFlags()&~CONTROL_INACTIVE);
			((CEditText*)GetControlPtr(CTRL_MAP_EDIT_TEXT))->SetEditString("No Name");
			RequestUpdate();
			break;
		case	CTRL_MAP_DEF_MODE:
			ToggleControlSelectedState(CTRL_MAP_DEF_MODE);
			DefMode^=1;
			RequestUpdate();
			break;
/*
		case	CTRL_ANIM_NAME_EDIT:
			if(CurrentMap)
				strcpy(map_info[CurrentMap].Name,((CEditText*)GetControlPtr(CTRL_MAP_EDIT_TEXT))->GetEditString());
			RequestUpdate();
			break;
*/
		case	CTRL_MAP_SELECT_AND_PLACE:
			Mode=MAP_MODE_SELECT_AND_PLACE;
			DefMode=1;
			SetControlState(CTRL_MAP_DEF_MODE,CTRL_SELECTED);
			RequestUpdate();
			break;
		case	CTRL_MAP_ZOOM_1:
			engine.Scale=896;
			RequestUpdate();
			break;

		case	CTRL_MAP_ZOOM_2:
			engine.Scale=200;
			RequestUpdate();
			break;
		case	CTRL_MAP_ZOOM_3:
			engine.Scale=60;
			RequestUpdate();
			break;
/*
		case	CTRL_MAP_DELETE:
			delete_all_lights();
			RequestUpdate();
			Mode=0;
			break;
*/
	}
}

//---------------------------------------------------------------
