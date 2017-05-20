#include	"Editor.hpp"

#include	"ColTab.hpp"
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


#define	COL_MODE_WAIT			0
#define	COL_MODE_PLACE_PLANE	1
#define	COL_MODE_EDIT_PLANE		2
#define	COL_MODE_DRAG_PLANE		3
#define	COL_MODE_PLACE_BEZIER	4


#define	CTRL_COL_X_AXIS_FREE		1
#define	CTRL_COL_Y_AXIS_FREE		2
#define	CTRL_COL_Z_AXIS_FREE		3
#define	CTRL_COL_PLACE_PLANE		4
#define	CTRL_COL_PLACE_BEZIER		5
#define	CTRL_COL_RECALC				6
#define	CTRL_COL_CLEAR_COL			7
#define	CTRL_COL_CLIPPED_VIEW		8


ControlDef	col_tab_def[]	=	
{
	{	CHECK_BOX,	0,	"X Free",	120,	300-10,	0,	10					},
	{	CHECK_BOX,	0,	"Y Free",	120,	313-10,	0,	10					},
	{	CHECK_BOX,	0,	"Z Free",	120,	326-10,	0,	10					},
	{	BUTTON,		0,	"Place A Collision Z Plane"	,	10,		10,		0,	0},
	{	BUTTON,		0,	"Place A Bezier Col Curve"	,	10,		30,		0,	0},
	{	BUTTON,		0,	"Recalc Collision"	,	10,		50,		0,	0},
	{	BUTTON,		0,	"CLEAR Collision"	,	90,		200,		0,	0},
	{	CHECK_BOX,	0,	"Clipped View",	20,	300-10,	0,	10					},
	{  	0	}
};


ColTab	*the_coltab;
	
struct	ColInfo	col_info[MAX_COL_INFO];
UWORD	next_col_info=1;

void	redraw_col_tab(void);
//---------------------------------------------------------------

ColTab::ColTab(EditorModule *parent)
{
	Parent=parent;

	InitControlSet(col_tab_def);
	AxisMode=3;

	SetControlState(CTRL_COL_X_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_COL_Y_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_COL_Z_AXIS_FREE,CTRL_SELECTED);

	Axis=X_AXIS|Y_AXIS|Z_AXIS;
	Mode=0;
	col_info[0].Plane.Depth=320;
	CurrentCol=0;
	the_coltab=this;
}

ColTab::~ColTab()
{
}

void	ColTab::Clear(void)
{
//	clear_all_col_info();
}

void	delete_col_info(SWORD index)
{
	SLONG	c0;
	for(c0=index;c0<next_col_info-1;c0++)
	{
		col_info[c0]=col_info[c0+1];
	}
	next_col_info--;
}

void	ColTab::Recalc(void)
{
	SLONG	c0;
	struct	ColInfo	*p_col;
	//clear_all_col_info();

	if(next_col_info==1)
	{
//		calc_collision_info(0,0,0,0,p_col->Plane.Depth,0);
	}
	else
	for(c0=1;c0<next_col_info;c0++)
	{
		p_col=&col_info[c0];
		switch(p_col->Type)
		{
			case	COL_TYPE_BEZIER:
			case	COL_TYPE_PLANE:
//				calc_collision_info(p_col); //->Plane.Left,p_col->Plane.Right,p_col->Plane.Top,p_col->Plane.Bottom,p_col->Plane.Depth,1);
				break;
		}
	}
}

void	ColTab::DrawTabContent(void)
{
	EdRect		content_rect;


	content_rect	=	ContentRect;	
	content_rect.ShrinkRect(1,1);
	content_rect.FillRect(CONTENT_COL);

	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
	DrawControlSet();
	ShowWorkWindow(0);
}

void	redraw_col_tab(void)
{

	switch(the_coltab->Mode)
	{
		

		default:
				
			the_coltab->DrawTabContent();
			the_coltab->Parent->DrawContent();
			SetWorkWindowBounds(the_coltab->Parent->GetLeft(),
								the_coltab->Parent->GetTop(),
								the_coltab->Parent->GetWidth(),
								the_coltab->Parent->GetHeight());
			break;
	}

}

//---------------------------------------------------------------
extern	void	hilight_col_info(void);

void	ColTab::DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG	wwx,wwy,www,wwh;
	EdRect	drawrect;

	RedrawModuleContent=0;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

		View1.SetRect(x,y,w-1,h/2-3);
		View2.SetRect(x,y+h/2+4,w-1,h/2-4);

		SetWorkWindowBounds(x,y,w-1,h/2-3);
		drawrect.SetRect(0,0,w-1,h/2-3);

		drawrect.FillRect(CONTENT_COL_BR);
		drawrect.HiliteRect(HILITE_COL,HILITE_COL);
		set_camera_plan();
		draw_editor_map(0);
		render_view(1);
//		hilight_map_things(MAP_THING_TYPE_COL);
		hilight_col_info();


		SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
		drawrect.SetRect(0,0,w-1,h/2-4);

//		drawrect.FillRect(LOLITE_COL);
		drawrect.FillRect(CONTENT_COL_BR);
		drawrect.HiliteRect(HILITE_COL,HILITE_COL);

		set_camera_front();
		draw_editor_map(0);
		render_view(1);
//		hilight_map_things(MAP_THING_TYPE_COL);
		hilight_col_info();


	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
}

//---------------------------------------------------------------


void	ColTab::HandleTab(MFPoint *current_point)
{
	SLONG		   update	=	0;
	

	ModeTab::HandleTab(current_point);
	KeyboardInterface();

	if(CurrentCol&&(engine.ClipFlag&ENGINE_CLIPY_FLAG))
	{
		struct	ColInfo	*p_col;
		p_col=&col_info[CurrentCol];
		switch(p_col->Type)
		{
			case	COL_TYPE_PLANE:
				engine.ClipMinY=p_col->Plane.Top;
				engine.ClipMaxY=p_col->Plane.Bottom;
				break;
			case	COL_TYPE_BEZIER:
				engine.ClipMinY=p_col->Bezier.Top;
				engine.ClipMaxY=p_col->Bezier.Bottom;
				break;
		}

		engine.ClipFlag=ENGINE_CLIPY_FLAG;
	}

}

inline SLONG is_point_in_box(SLONG x,SLONG y,SLONG left,SLONG top,SLONG w,SLONG h)
{
	if(x>left&&x<left+w&&y>top&&y<top+h)
		return(1);
	else
		return(0);
}
//---------------------------------------------------------------


SLONG	ColTab::KeyboardInterface(void)
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
				SetControlState(CTRL_COL_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_COL_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_COL_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=X_AXIS;
				break;
			case	1:
				SetControlState(CTRL_COL_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_COL_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_COL_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=Y_AXIS;
				break;
			case	2:
				SetControlState(CTRL_COL_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_COL_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_COL_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=Z_AXIS;
				break;
			case	3:
				SetControlState(CTRL_COL_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_COL_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_COL_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=X_AXIS|Y_AXIS|Z_AXIS;
				break;
		}

		SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
  		DrawControlSet();
		ShowWorkWindow(0);
	}
	return(0);
}



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

UWORD	CreateColBezier(SLONG x,SLONG y,SLONG z)
{
	col_info[next_col_info].Type=COL_TYPE_BEZIER;
	col_info[next_col_info].Bezier.X[0]=x;
	col_info[next_col_info].Bezier.X[1]=x+100;
	col_info[next_col_info].Bezier.X[2]=x+200;
	col_info[next_col_info].Bezier.X[3]=x+300;

	col_info[next_col_info].Bezier.Z[0]=z;
	col_info[next_col_info].Bezier.Z[1]=z;
	col_info[next_col_info].Bezier.Z[2]=z;
	col_info[next_col_info].Bezier.Z[3]=z;

	col_info[next_col_info].Bezier.Top=y-100;
	col_info[next_col_info].Bezier.Bottom=y+100;

	next_col_info++;
	return(next_col_info-1);
}

UWORD	CreateColPlane(SLONG x,SLONG y,SLONG z)
{
	col_info[next_col_info].Type=COL_TYPE_PLANE;
	col_info[next_col_info].Plane.Left=x;
	col_info[next_col_info].Plane.Right=x+300;
	col_info[next_col_info].Plane.Top=y;
	col_info[next_col_info].Plane.Bottom=y+300;
	col_info[next_col_info].Plane.Depth=z;
	next_col_info++;
	return(next_col_info-1);
}


#define	SHIFT_BEZ	(10)
#define	BEZ_ONE		(1<<SHIFT_BEZ)
void	draw_bezier(SLONG x0,SLONG z0,SLONG x1,SLONG z1,SLONG x2,SLONG z2,SLONG x3,SLONG z3)
{
	SLONG	t=0;
	SLONG	bx,bz;
	SLONG	px,pz;

	SLONG	ox,oz;

	ox=x0;
	oz=z0;

	x0-=ox;
	x1-=ox;
	x2-=ox;
	x3-=ox;

	z0-=oz;
	z1-=oz;
	z2-=oz;
	z3-=oz;

	px=x0;
	pz=z0;

	for(t=BEZ_ONE>>5;t<BEZ_ONE;t+=BEZ_ONE>>5)
	{

		bx=(((((BEZ_ONE-t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*x0+
		   (((((3*t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*x1+
		   (((((3*t)*(1*t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*x2+
		   ((((t*t)>>SHIFT_BEZ)*t)>>SHIFT_BEZ)*x3;

		bz=(((((BEZ_ONE-t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*z0+
		   (((((3*t)*(BEZ_ONE-t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*z1+
		   (((((3*t)*(1*t))>>SHIFT_BEZ)*(BEZ_ONE-t))>>SHIFT_BEZ)*z2+
		   ((((t*t)>>SHIFT_BEZ)*t)>>SHIFT_BEZ)*z3;

		bx>>=SHIFT_BEZ;
		bz>>=SHIFT_BEZ;
		draw_3d_line(ox+bx,0,oz+bz,ox+px,0,oz+pz,WHITE_COL);
		px=bx;
		pz=bz;
	}
	draw_3d_line(ox+px,0,oz+pz,ox+x3,0,oz+z3,WHITE_COL);
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
void	draw_a_col_info(UWORD index)
{
	struct	ColInfo	*p_col;
	SLONG	col=WHITE_COL;
	EdRect	rect;
	p_col=&col_info[index];
	switch(p_col->Type)
	{
		case	COL_TYPE_PLANE:
				draw_3d_line(p_col->Plane.Left,p_col->Plane.Top,p_col->Plane.Depth,p_col->Plane.Right,p_col->Plane.Top,p_col->Plane.Depth,col);
				draw_3d_line(p_col->Plane.Right,p_col->Plane.Top,p_col->Plane.Depth,p_col->Plane.Right,p_col->Plane.Bottom,p_col->Plane.Depth,col);
				draw_3d_line(p_col->Plane.Left,p_col->Plane.Bottom,p_col->Plane.Depth,p_col->Plane.Right,p_col->Plane.Bottom,p_col->Plane.Depth,col);
				draw_3d_line(p_col->Plane.Left,p_col->Plane.Bottom,p_col->Plane.Depth,p_col->Plane.Left,p_col->Plane.Top,p_col->Plane.Depth,col);
				break;

		case	COL_TYPE_BEZIER:
			SLONG	mid_y,temp;
				temp=engine.ClipFlag;
				engine.ClipFlag=0;

				mid_y=p_col->Bezier.Top+p_col->Bezier.Bottom;
				mid_y>>=1;

				set_screen_box(p_col->Bezier.X[0],mid_y,p_col->Bezier.Z[0],&rect,5,5);
				rect.OutlineRect(WHITE_COL);
				set_screen_box(p_col->Bezier.X[1],mid_y,p_col->Bezier.Z[1],&rect,5,5);
				rect.OutlineRect(WHITE_COL);
				set_screen_box(p_col->Bezier.X[2],mid_y,p_col->Bezier.Z[2],&rect,5,5);
				rect.OutlineRect(WHITE_COL);
				set_screen_box(p_col->Bezier.X[3],mid_y,p_col->Bezier.Z[3],&rect,5,5);
				rect.OutlineRect(WHITE_COL);
				draw_bezier(p_col->Bezier.X[0],p_col->Bezier.Z[0],p_col->Bezier.X[1],p_col->Bezier.Z[1],p_col->Bezier.X[2],p_col->Bezier.Z[2],p_col->Bezier.X[3],p_col->Bezier.Z[3]);

				draw_3d_line(p_col->Bezier.X[0],p_col->Bezier.Top,9999,p_col->Bezier.X[3],p_col->Bezier.Top,9999,col);
				draw_3d_line(p_col->Bezier.X[0],p_col->Bezier.Bottom,9999,p_col->Bezier.X[3],p_col->Bezier.Bottom,9999,col);
				engine.ClipFlag=temp;
				break;
	}
}

void	hilight_col_info(void)
{
	SLONG	c0;
	for(c0=1;c0<next_col_info;c0++)
	{
		draw_a_col_info(c0);
	}
}


static void	create_box_from_vect(EdRect *rect,SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
	struct	SVector	point[2];
	struct	SVector	res[2];
	SLONG	sx,sy,sz,c0;
	ULONG	f1,f2;
	SLONG	temp;
	temp=engine.ClipFlag;
	engine.ClipFlag=0;

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

	engine.ClipFlag=temp;
}

static void	create_box_from_point(EdRect *rect,SLONG x1,SLONG y1,SLONG z1)
{
	struct	SVector	point[1];
	struct	SVector	res[1];
	SLONG	sx,sy,sz,c0;
	ULONG	f1,f2;
	SLONG	temp;
	temp=engine.ClipFlag;
	engine.ClipFlag=0;
	point[0].X=x1;
	point[0].Y=y1;
	point[0].Z=z1;
	f1=rotate_point_gte(&point[0],&res[0]);

	rect->SetRect(res[0].X,res[0].Y,10,10);
	rect->NormalRect();
	rect->SetRect(rect->GetLeft()-10,rect->GetTop()-10,rect->GetWidth()+10,rect->GetHeight()+10);
	rect->OutlineRect(WHITE_COL);
	engine.ClipFlag=temp;
}

	
SLONG	select_this_col_info(SLONG index,MFPoint *mouse)
{
	struct	ColInfo	*p_col;
	EdRect	rect;

	p_col=&col_info[index];
	switch(p_col->Type)
	{
		case	COL_TYPE_PLANE:
			//corners
			create_box_from_point(&rect,p_col->Plane.Left,p_col->Plane.Top,p_col->Plane.Depth);
			if(rect.PointInRect(mouse))
				return(5);
			create_box_from_point(&rect,p_col->Plane.Right,p_col->Plane.Top,p_col->Plane.Depth);
			if(rect.PointInRect(mouse))
				return(6);
			create_box_from_point(&rect,p_col->Plane.Right,p_col->Plane.Bottom,p_col->Plane.Depth);
			if(rect.PointInRect(mouse))
				return(7);
			create_box_from_point(&rect,p_col->Plane.Left,p_col->Plane.Bottom,p_col->Plane.Depth);
			if(rect.PointInRect(mouse))
				return(8);


			//edges
			create_box_from_vect(&rect,p_col->Plane.Left,p_col->Plane.Top,p_col->Plane.Depth,p_col->Plane.Right,p_col->Plane.Top,p_col->Plane.Depth);
			if(rect.PointInRect(mouse))
				return(1);
			create_box_from_vect(&rect,p_col->Plane.Right,p_col->Plane.Top,p_col->Plane.Depth,p_col->Plane.Right,p_col->Plane.Bottom,p_col->Plane.Depth);
			if(rect.PointInRect(mouse))
				return(2);
			create_box_from_vect(&rect,p_col->Plane.Left,p_col->Plane.Bottom,p_col->Plane.Depth,p_col->Plane.Right,p_col->Plane.Bottom,p_col->Plane.Depth);
			if(rect.PointInRect(mouse))
				return(3);
			create_box_from_vect(&rect,p_col->Plane.Left,p_col->Plane.Top,p_col->Plane.Depth,p_col->Plane.Left,p_col->Plane.Bottom,p_col->Plane.Depth);
			if(rect.PointInRect(mouse))
				return(4);

			break;
		case	COL_TYPE_BEZIER:
			set_screen_box(p_col->Bezier.X[0],0,p_col->Bezier.Z[0],&rect,10,10);
			if(rect.PointInRect(mouse))
				return(1);
			set_screen_box(p_col->Bezier.X[1],0,p_col->Bezier.Z[1],&rect,10,10);
			if(rect.PointInRect(mouse))
				return(2);
			set_screen_box(p_col->Bezier.X[2],0,p_col->Bezier.Z[2],&rect,10,10);
			if(rect.PointInRect(mouse))
				return(3);
			set_screen_box(p_col->Bezier.X[3],0,p_col->Bezier.Z[3],&rect,10,10);
			if(rect.PointInRect(mouse))
				return(4);

			create_box_from_vect(&rect,p_col->Bezier.X[0],p_col->Bezier.Top,9999,p_col->Bezier.X[3],p_col->Bezier.Top,9999);
			if(rect.PointInRect(mouse))
				return(5);

			create_box_from_vect(&rect,p_col->Bezier.X[0],p_col->Bezier.Bottom,9999,p_col->Bezier.X[3],p_col->Bezier.Bottom,9999);
			if(rect.PointInRect(mouse))
				return(6);

			break;

		
	}
	return(0);
}

SLONG	select_col_info(MFPoint *mouse,SLONG *ret)
{
	static	UBYTE col=0;
	SLONG	c0;
	col++;

	for(c0=1;c0<next_col_info;c0++)
	{
		*ret=select_this_col_info(c0,mouse);
		if(*ret)
			return(c0);
	}
	return(0);
}

SLONG	ColTab::DragACol(UBYTE flags,MFPoint *clicked_point,UWORD copy)
{
	SLONG	side;
	SLONG	drag=0,drag_type;
	SLONG	x,y,w,h;
	SLONG	wwx,wwy,www,wwh;
	SLONG	window=0;
	SLONG	col;
	SLONG	screen_change=0;
	SLONG	last_world_mouse;
	if(engine.ClipFlag)
	{
		LogText(" in dragacol with clipy\n");
	}

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	col++;
//	clicked_point->X-=x;	
//	clicked_point->Y-=y;	
	SetWorkWindowBounds(x,y,w-1,h/2-3);
	set_camera_plan();
	drag=select_col_info(clicked_point,&side);
//	DrawPixelC(clicked_point->X,clicked_point->Y,255);
//	ShowWorkWindow(0);

	if(!drag) //if not selected one in that window try other
	{
		MFPoint		local_point;

		SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
	 	set_camera_front();
		local_point	  =*clicked_point;
		local_point.Y-=h/2+4;
		drag=select_col_info(&local_point,&side);
		window=1;
//		ShowWorkWindow(0);
	}

	if(drag)  //drag in plan view
	{
		struct	ColInfo	*p_col;
		drag_type=col_info[drag].Type;
		p_col=&col_info[drag];
		
		CurrentCol=drag;

		while(LeftButton)
		{
			SLONG	nx,ny,nz;
			last_world_mouse=SetWorldMouse(0);

			if(last_world_mouse)
			{
				switch(drag_type)
				{
					case	COL_TYPE_PLANE:
						if(window==0)
						{
							if(side)
								col_info[CurrentCol].Plane.Depth=engine.MousePosZ;
						}
						switch(side)
						{
							case 1: //top
								col_info[CurrentCol].Plane.Top=engine.MousePosY;
								break;
							case 3: //bot
								col_info[CurrentCol].Plane.Bottom=engine.MousePosY;
								break;
							case 2: //right
								col_info[CurrentCol].Plane.Right=engine.MousePosX;
								break;
							case 4: //left
								col_info[CurrentCol].Plane.Left=engine.MousePosX;
								break;
							case	5: //topleft
								col_info[CurrentCol].Plane.Top=engine.MousePosY;
								col_info[CurrentCol].Plane.Left=engine.MousePosX;
								break;
							case	6: //topright
								col_info[CurrentCol].Plane.Right=engine.MousePosX;
								col_info[CurrentCol].Plane.Top=engine.MousePosY;
								break;
							case	7: //botright
								col_info[CurrentCol].Plane.Right=engine.MousePosX;
								col_info[CurrentCol].Plane.Bottom=engine.MousePosY;
								break;
							case	8: //botleft
								col_info[CurrentCol].Plane.Left=engine.MousePosX;
								col_info[CurrentCol].Plane.Bottom=engine.MousePosY;
								break;
						}
						break;
					case	COL_TYPE_BEZIER:
							switch(side)
							{
								case	5:
									p_col->Bezier.Top=engine.MousePosY;
									break;
								case	6:
									p_col->Bezier.Bottom=engine.MousePosY;
									break;
								default:
								if(!ShiftFlag)
								{
									p_col->Bezier.X[side-1]=engine.MousePosX;
									p_col->Bezier.Z[side-1]=engine.MousePosZ;
								}
								else
								{
									SLONG	dx,dz;
									dx=p_col->Bezier.X[side-1]-engine.MousePosX;
									dz=p_col->Bezier.Z[side-1]-engine.MousePosZ;
									p_col->Bezier.X[0]-=dx;
									p_col->Bezier.Z[0]-=dz;
									p_col->Bezier.X[1]-=dx;
									p_col->Bezier.Z[1]-=dz;
									p_col->Bezier.X[2]-=dx;
									p_col->Bezier.Z[2]-=dz;
									p_col->Bezier.X[3]-=dx;
									p_col->Bezier.Z[3]-=dz;
									
								}
								break;
							}
						break;
				}
			}

			DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			ShowWorkWindow(0);
			screen_change=1;

			SetWorkWindowBounds(this->ContentLeft()+1,this->ContentTop()+1,this->ContentWidth(),this->ContentHeight()); 

			DrawBox(0,0,this->ContentWidth(),this->ContentHeight(),CONTENT_COL);
			set_camera();
			draw_editor_map(0);
			render_view(1);
			ShowWorkWindow(0);
			editor_user_interface(0);
			KeyboardInterface();
		}
		if(!last_world_mouse)
		{	//delete col
			delete_col_info(drag);
			CurrentCol=0;
		}

		RequestUpdate();
	}
	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

	return(screen_change);

}

SLONG	ColTab::DragEngine(UBYTE flags,MFPoint *clicked_point)
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

		while(MiddleButton)
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

SLONG	ColTab::HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h)
{
	SWORD	thing;
	SWORD	bright;
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:

			switch (Mode)
			{
				case	COL_MODE_PLACE_PLANE:
					if(SetWorldMouse(1))
					{
						CurrentCol=CreateColPlane(engine.MousePosX,engine.MousePosY,engine.MousePosZ);
						Mode=0;
					}
					return(1);

					break;
				case	COL_MODE_WAIT:
					DragACol(flags,clicked_point,0);
					break;

				case	COL_MODE_PLACE_BEZIER:
					if(SetWorldMouse(1))
					{
						CurrentCol=CreateColBezier(engine.MousePosX,engine.MousePosY,engine.MousePosZ);
						Mode=0;
					}
					return(1);
			}
			break;
		case	RIGHT_CLICK:
			switch (Mode)
			{
			}
			// Right click in content.
			break;
		case	MIDDLE_CLICK:
				DragEngine(flags,clicked_point);
			break;
	}
	return(0);
	
}

UWORD	ColTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
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

SLONG	ColTab::SetWorldMouse(ULONG flag)
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
	if(is_point_in_box(local_point.X,local_point.Y,0,Parent->ContentHeight()/2,Parent->ContentWidth()-1,Parent->ContentHeight()/2))
	{
		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+Parent->ContentHeight()/2+3,Parent->ContentWidth()-1,Parent->ContentHeight()/2-4);
		set_camera_front();
		calc_world_pos_front(local_point.X,local_point.Y-Parent->ContentHeight()/2);
		if(flag)
			engine.MousePosZ=engine.Z>>8;
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


void	ColTab::HandleControl(UWORD control_id)
{
	switch(control_id&0xff)
	{

		case	CTRL_COL_X_AXIS_FREE:
			ToggleControlSelectedState(CTRL_COL_X_AXIS_FREE);
			if(Axis&X_AXIS)
				Axis&=~X_AXIS;
			else
				Axis|=X_AXIS;
			break;
		case	CTRL_COL_Y_AXIS_FREE:
			ToggleControlSelectedState(CTRL_COL_Y_AXIS_FREE);
			if(Axis&Y_AXIS)
				Axis&=~Y_AXIS;
			else
				Axis|=Y_AXIS;
			break;
		case	CTRL_COL_Z_AXIS_FREE:
			ToggleControlSelectedState(CTRL_COL_Z_AXIS_FREE);
			if(Axis&Z_AXIS)
				Axis&=~Z_AXIS;
			else
				Axis|=Z_AXIS;
			break;
		case	CTRL_COL_PLACE_PLANE:
			Mode=COL_MODE_PLACE_PLANE;
			break;
		case	CTRL_COL_PLACE_BEZIER:
			Mode=COL_MODE_PLACE_BEZIER;
			break;
		case	CTRL_COL_RECALC:
			Recalc();

			break;
		case	CTRL_COL_CLEAR_COL:
			Clear();

			break;
		case	CTRL_COL_CLIPPED_VIEW:
			ToggleControlSelectedState(CTRL_COL_CLIPPED_VIEW);
			if(ClipView)
			{
				ClipView=0;
				engine.ClipFlag=0;
			}
			else
			{
				struct	ColInfo	*p_col;
				ClipView=1;
				p_col=&col_info[CurrentCol];
				switch(p_col->Type)
				{
					case	COL_TYPE_PLANE:
						engine.ClipMinY=p_col->Plane.Top;
						engine.ClipMaxY=p_col->Plane.Bottom;
						break;
					case	COL_TYPE_BEZIER:
						engine.ClipMinY=p_col->Bezier.Top;
						engine.ClipMaxY=p_col->Bezier.Bottom;
						break;
				}

				engine.ClipFlag=ENGINE_CLIPY_FLAG;
			}
//			RequestRedraw();
			break;
/*
		case	CTRL_COL_DELETE:
			delete_all_lights();
			RequestUpdate();
			Mode=0;
			break;
*/
	}
}

//---------------------------------------------------------------
