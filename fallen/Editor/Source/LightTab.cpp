#include	"Editor.hpp"

#include	"LightTab.hpp"
#include	"engine.h"
#include	"c:\fallen\headers\memory.h"

static		counter;
#define	SHADOW_LIGHT_SHIFT		(5)
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

void	apply_ambient_to_floor(void);
void	remove_ambient_from_floor(void);

#define	CTRL_LIGHT_X_AXIS_FREE		1
#define	CTRL_LIGHT_Y_AXIS_FREE		2
#define	CTRL_LIGHT_Z_AXIS_FREE		3
#define	CTRL_LIGHT_PLACE			4
#define	CTRL_LIGHT_SMOOTH_LOTS		5
#define	CTRL_LIGHT_PLACE_AMBIENT	6
#define	CTRL_LIGHT_BRIGHT			7
#define	CTRL_LIGHT_SHADOW			8
#define	CTRL_LIGHT_DELETE			9
#define	CTRL_LIGHT_FINISH_AMBIENT	10

#define CTRL_LIGHT_WHITE			11
#define CTRL_LIGHT_PALE_BLUE		12
#define CTRL_LIGHT_ORANGE			13
#define CTRL_LIGHT_YELLOW			14
#define CTRL_LIGHT_RED				15
#define CTRL_LIGHT_BLUE				16
#define CTRL_LIGHT_SLIDE_RED		17
#define CTRL_LIGHT_SLIDE_GREEN		18
#define CTRL_LIGHT_SLIDE_BLUE		19

#define CTRL_LIGHT_NORMAL			20
#define CTRL_LIGHT_BROKEN			21
#define CTRL_LIGHT_FLASH			22
#define CTRL_LIGHT_PARAM			23


ControlDef	light_tab_def[]	=	
{														 
	{	CHECK_BOX,	0,	"X Free",					20,  150,	0,	10	},
	{	CHECK_BOX,	0,	"Y Free",					20,  165,	0,	10	},
	{	CHECK_BOX,	0,	"Z Free",					20,  180,	0,	10	},
	{	BUTTON,		0,	"Place a Light"	,			20,  50,	0,	0	},
	{	BUTTON,		0,	"Smooth Shades For Group",	20,  65,	0,	0	},
	{	BUTTON,		0,	"Replace Ambient",			20,  80,	0,	0	},
	{	H_SLIDER,	0,	"",							20,  100,	250,0	},
	{	CHECK_BOX,	0,	"Shadow",					20,	 115,	0,	10	},
	{	BUTTON,		0,	"DELETE ALL LIGHTS",		180, 50,	0,	0	},
	{	BUTTON,		0,	"Finish ambient",			20,  130,	0,	0	},
	{	BUTTON,		0,	"White",					20,  200,	0,	0	},
	{	BUTTON,		0,	"Pale blue",				20,  215,	0,	0	},
	{	BUTTON,		0,	"Orange",					20,  230,	0,	0	},
	{	BUTTON,		0,	"Yellow",					20,  245,	0,	0	},
	{	BUTTON,		0,	"Red",						20,  260,	0,	0	},
	{	BUTTON,		0,	"Blue",						20,  275,	0,	0	},
	{	H_SLIDER,	0,	"",							90,  207,	150,0	},
	{	H_SLIDER,	0,	"",							90,  227,	150,0	},
	{	H_SLIDER,	0,	"",							90,  247,	150,0	},
	{	CHECK_BOX,	0,	"Normal",					20,	 295,   0,  0   },
	{	CHECK_BOX,	0,	"Broken",					20,	 310,   0,  0   },
	{	CHECK_BOX,	0,	"Flash",					20,	 325,   0,  0   },
	{	H_SLIDER,	0,	"",							90,	 310,	150,0	},
	{	0	}
};



LightTab	*the_lighttab;
static	light_x,light_y,light_z,light_bright;

void	redraw_tab(void);
void	scan_apply_ambient(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra);
void	link_all_lights(void);
void	scan_undo_ambient(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra);
//---------------------------------------------------------------

LightTab::LightTab(EditorModule *parent)
{
	Parent=parent;

	InitControlSet(light_tab_def);
	AxisMode=3;

	SetControlState(CTRL_LIGHT_X_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_LIGHT_Y_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_LIGHT_Z_AXIS_FREE,CTRL_SELECTED);

	((CHSlider*)GetControlPtr(CTRL_LIGHT_BRIGHT))->SetValueRange(0,4096);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_BRIGHT))->SetCurrentValue(512);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_BRIGHT))->SetUpdateFunction(redraw_tab);
	Axis=X_AXIS|Y_AXIS|Z_AXIS;
	CurrentLight=0;
	Mode=0;
	Shadow=0;

	the_lighttab=this;

	((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED))->SetValueRange(0,255);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED))->SetCurrentValue(255);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED))->SetUpdateFunction(redraw_tab);

	((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->SetValueRange(0,255);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->SetCurrentValue(255);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->SetUpdateFunction(redraw_tab);

	((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE))->SetValueRange(0,255);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE))->SetCurrentValue(255);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE))->SetUpdateFunction(redraw_tab);

	((CHSlider*)GetControlPtr(CTRL_LIGHT_PARAM))->SetValueRange(0,100);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_PARAM))->SetCurrentValue(50);
	((CHSlider*)GetControlPtr(CTRL_LIGHT_PARAM))->SetUpdateFunction(redraw_tab);

}

LightTab::~LightTab()
{
}


void	LightTab::DrawTabContent(void)
{
	EdRect		content_rect;



	content_rect	=	ContentRect;	
	content_rect.ShrinkRect(1,1);
	content_rect.FillRect(CONTENT_COL);

	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
	if(CurrentLight)
	{
		SLONG	bright;
		static	SLONG	old_bright;
	 	bright =	((CHSlider*)GetControlPtr(CTRL_LIGHT_BRIGHT))->GetCurrentValue();
		if(bright!=old_bright)
		{
			apply_light_to_map(map_things[CurrentLight].X,map_things[CurrentLight].Y,map_things[CurrentLight].Z,-map_things[CurrentLight].IndexOther);
			old_bright=map_things[CurrentLight].IndexOther;
			map_things[CurrentLight].IndexOther=bright;
			apply_light_to_map(map_things[CurrentLight].X,map_things[CurrentLight].Y,map_things[CurrentLight].Z,map_things[CurrentLight].IndexOther);
//			RedrawModuleContent=1;
		}

		map_things[CurrentLight].AngleX		= ((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED  ))->GetCurrentValue();
		map_things[CurrentLight].AngleY		= ((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->GetCurrentValue();
		map_things[CurrentLight].AngleZ		= ((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE ))->GetCurrentValue();

		map_things[CurrentLight].IndexOrig	= ((CHSlider*)GetControlPtr(CTRL_LIGHT_PARAM      ))->GetCurrentValue();

		switch(map_things[CurrentLight].IndexOrig)
		{
			case LIGHT_TYPE_NORMAL:
			case LIGHT_TYPE_BROKEN:
			case LIGHT_TYPE_PULSE:
				map_things[CurrentLight].IndexOrig >>= 1;
				break;
		}
	}

	DrawControlSet();
	ShowWorkWindow(0);
//	DrawPrims();
}



SLONG	sum_shared_brightness(SWORD shared_point)
{
	SLONG	c0,point;
	SLONG	face;
	SLONG	bright=0;
	SLONG count=0;

	for(c0=1;c0<next_face_selected;c0++)
	{
		face=face_selected_list[c0];
		if(face<0)
		{
			for(point=0;point<3;point++)
			{
				if(prim_faces3[-face].Points[point]==shared_point)
				{
					bright+=prim_faces3[-face].Bright[point];
					count++;
				}

			}
		}
		else
		if(face>0)
		{
			for(point=0;point<4;point++)
			{
				if(prim_faces4[face].Points[point]==shared_point)
				{
					bright+=prim_faces4[face].Bright[point];
					count++;
				}
			}
			
		}
	}
	if(count)
		return(bright/count);
	else
		return(0);
}

void	set_shared_brightness(SWORD shared_point,SWORD bright)
{
	SLONG	c0,point;
	SLONG	face;

	for(c0=1;c0<next_face_selected;c0++)
	{
		face=face_selected_list[c0];
		if(face<0)
		{
			for(point=0;point<3;point++)
			{
				if(prim_faces3[-face].Points[point]==shared_point)
				{
					prim_faces3[-face].Bright[point]=bright;
				}
			}
		}
		else
		if(face>0)
		{
			for(point=0;point<4;point++)
			{
				if(prim_faces4[face].Points[point]==shared_point)
				{
					prim_faces4[face].Bright[point]=bright;
				}
			}
		}
	}
}

void	LightTab::SmoothGroup(void)
{
	SLONG	c0,c1,point;
	SLONG	face;
	SLONG	bright;

	for(c0=1;c0<next_face_selected;c0++)
	{
		face=face_selected_list[c0];
		if(face<0)
		{
			prim_faces3[-face].FaceFlags|=FACE_FLAG_SMOOTH;
			for(point=0;point<3;point++)
			{
				bright=sum_shared_brightness(prim_faces3[-face].Points[point]);
				set_shared_brightness(prim_faces3[-face].Points[point],bright);

			}
		}
		else
		if(face>0)
		{
			prim_faces4[face].FaceFlags|=FACE_FLAG_SMOOTH;
			for(point=0;point<4;point++)
			{
				bright=sum_shared_brightness(prim_faces4[face].Points[point]);
				set_shared_brightness(prim_faces4[face].Points[point],bright);
			}
		}
	}
}




void	apply_ambient_to_floor(void)
{
	SLONG	x,z;
	SLONG	bright,temp_bright;

	bright=(edit_info.amb_bright*(-edit_info.amb_dy))>>9;

	if(bright<0)
		bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;
	else
		if(bright>128)
			bright=128;

//	bright=64;

	for(x=0;x<EDIT_MAP_WIDTH;x++)
	for(z=0;z<EDIT_MAP_DEPTH;z++)
	{

		temp_bright=edit_map[x][z].Bright+bright;
		if(temp_bright<0)
			temp_bright=0;
		if(edit_info.amb_flags&2)
			edit_map[x][z].Bright=bright;
		else
			edit_map[x][z].Bright=temp_bright;
	}
}
void	remove_ambient_from_floor(void)
{
	SLONG	x,z;
	SLONG	bright,temp_bright;

	bright=(edit_info.amb_bright*(-edit_info.amb_dy))>>9;
//	bright=edit_info.amb_bright>>1;

	if(bright<0)
		bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;
	else
		if(bright>128)
			bright=128;

	for(x=0;x<EDIT_MAP_WIDTH;x++)
	for(z=0;z<EDIT_MAP_DEPTH;z++)
	{

		temp_bright=edit_map[x][z].Bright-bright;
		if(temp_bright<0)
			temp_bright=0;
		edit_map[x][z].Bright=temp_bright;
	}
}


void	LightTab::RecalcAllLights(void)
{
	SLONG	c0;
	edit_info.amb_flags|=2;

	scan_function=scan_apply_ambient;
	scan_map();	
	apply_ambient_to_floor();

	//now re-apply all lights

	for(c0=1;c0<MAX_MAP_THINGS;c0++)
	{
		if(map_things[c0].Type==MAP_THING_TYPE_LIGHT)
		{

			apply_light_to_map(map_things[c0].X,map_things[c0].Y,map_things[c0].Z,map_things[c0].IndexOther);
		}
	}

	edit_info.amb_flags&=~2;
	Mode=0;
}


void	redraw_tab(void)
{

	switch(the_lighttab->Mode)
	{
		
		case LIGHT_TAB_MODE_PLACE_AMBIENT:

			scan_function=scan_undo_ambient;
			scan_map();	
			remove_ambient_from_floor();


		 	edit_info.amb_bright =	((CHSlider*)the_lighttab->GetControlPtr(CTRL_LIGHT_BRIGHT))->GetCurrentValue();
//			link_all_lights();
			scan_function=scan_apply_ambient;
			scan_map();	
			apply_ambient_to_floor();
			the_lighttab->DrawTabContent();
			the_lighttab->Parent->DrawContent();
//			the_lighttab->DrawTabContent();

			SetWorkWindowBounds(the_lighttab->Parent->GetLeft(),
								the_lighttab->Parent->GetTop(),
								the_lighttab->Parent->GetWidth(),
								the_lighttab->Parent->GetHeight());

				break;

		default:
				
			the_lighttab->DrawTabContent();
			the_lighttab->Parent->DrawContent();
			SetWorkWindowBounds(the_lighttab->Parent->GetLeft(),
								the_lighttab->Parent->GetTop(),
								the_lighttab->Parent->GetWidth(),
								the_lighttab->Parent->GetHeight());
			break;
	}

}

//---------------------------------------------------------------

void	LightTab::DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h)
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
		render_view(0);
		hilight_map_things(MAP_THING_TYPE_LIGHT);

		if(Mode==LIGHT_TAB_MODE_PLACE_AMBIENT)
			draw_3d_line(engine.X>>8,engine.Y>>8,engine.Z>>8,(engine.X>>8)+(edit_info.amb_dx>>2),(engine.Y>>8)+(edit_info.amb_dy>>2),(engine.Z>>8)+(edit_info.amb_dz>>2),WHITE_COL);

		SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
		drawrect.SetRect(0,0,w-1,h/2-4);

//		drawrect.FillRect(LOLITE_COL);
		drawrect.FillRect(CONTENT_COL_BR);
		drawrect.HiliteRect(HILITE_COL,HILITE_COL);

		set_camera_front();
		draw_editor_map(0);
		render_view(0);
		hilight_map_things(MAP_THING_TYPE_LIGHT);

		if(Mode==LIGHT_TAB_MODE_PLACE_AMBIENT)
			draw_3d_line(engine.X>>8,engine.Y>>8,engine.Z>>8,(engine.X>>8)+(edit_info.amb_dx>>2),(engine.Y>>8)+(edit_info.amb_dy>>2),(engine.Z>>8)+(edit_info.amb_dz>>2),WHITE_COL);

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
}

//---------------------------------------------------------------


void	LightTab::HandleTab(MFPoint *current_point)
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


SLONG	LightTab::KeyboardInterface(void)
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
				SetControlState(CTRL_LIGHT_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_LIGHT_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_LIGHT_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=X_AXIS;
				break;
			case	1:
				SetControlState(CTRL_LIGHT_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_LIGHT_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_LIGHT_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=Y_AXIS;
				break;
			case	2:
				SetControlState(CTRL_LIGHT_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_LIGHT_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_LIGHT_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=Z_AXIS;
				break;
			case	3:
				SetControlState(CTRL_LIGHT_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_LIGHT_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_LIGHT_Z_AXIS_FREE,CTRL_SELECTED);
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


void	scan_apply_light(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra)
{
	SVector	normal;
	SLONG dx,dy,dz,dist;
	SLONG	point,bright;
	SLONG	angle_light;

	SLONG	lnx,lny,lnz;



	if(face<0)
	{	//tris
#ifdef	NORMAL
		dx=dy=dz=0;
		for(point=0;point<3;point++)
		{
			dx+=(prim_points[prim_faces3[-face].Points[point]].X);
			dy+=(prim_points[prim_faces3[-face].Points[point]].Y);
			dz+=(prim_points[prim_faces3[-face].Points[point]].Z);
		}
		dx/=3;
		dy/=3;
		dz/=3;

		dx+=x;
		dy+=y;
		dz+=z;

		lnx=dx-light_x;
		lny=dy-light_y;
		lnz=dz-light_z;

		dist=normalise_xyz(&lnx,&lny,&lnz);

		calc_normal(-face,&normal);
		angle_light=(normal.X*lnx+normal.Y*lny+normal.Z*lnz)>>8;
		if(angle_light<0)
			return;
#endif
		for(point=0;point<3;point++)
		{
			dx=light_x-(x+prim_points[prim_faces3[-face].Points[point]].X);
			dy=light_y-(y+prim_points[prim_faces3[-face].Points[point]].Y);
			dz=light_z-(z+prim_points[prim_faces3[-face].Points[point]].Z);

			dist=dx*dx+dy*dy+dz*dz;
			if(dist==0)
				dist=1;


			bright=(light_bright<<11)/dist;   //scale brightness because of distance
#ifdef	NORMAL
			bright=(angle_light*bright)>>8;         //scale brightness because of angle to light
#endif
			bright+=prim_faces3[-face].Bright[point];
			if(bright>=32767)
				bright=32767;
			prim_faces3[-face].Bright[point]=bright;
		}
	}
	else
	if(face>0)
	{ 	//quads
#ifdef	NORMAL
		dx=dy=dz=0;
		for(point=0;point<4;point++)
		{
			dx+=(prim_points[prim_faces4[face].Points[point]].X);
			dy+=(prim_points[prim_faces4[face].Points[point]].Y);
			dz+=(prim_points[prim_faces4[face].Points[point]].Z);
		}
		dx/=4;
		dy/=4;
		dz/=4;

		dx+=x;
		dy+=y;
		dz+=z;

		lnx=dx-light_x;
		lny=dy-light_y;
		lnz=dz-light_z;

		normalise_xyz(&lnx,&lny,&lnz);


		calc_normal(face,&normal);
		angle_light=(normal.X*lnx+normal.Y*lny+normal.Z*lnz)>>8;
		if(angle_light<0)
			return;
#endif
		for(point=0;point<4;point++)
		{
			dx=light_x-(x+prim_points[prim_faces4[face].Points[point]].X);
			dy=light_y-(y+prim_points[prim_faces4[face].Points[point]].Y);
			dz=light_z-(z+prim_points[prim_faces4[face].Points[point]].Z);

			dist=dx*dx+dy*dy+dz*dz;
			if(dist==0)
				dist=1;

			bright=(light_bright<<11)/dist;
#ifdef	NORMAL
			bright=(angle_light*bright)>>8;         //scale brightness because of angle to light
#endif

			bright+=prim_faces4[face].Bright[point];
			if(bright>=32767)
				bright=32767;
			prim_faces4[face].Bright[point]=bright;
		}
	}
}

static	light_head=0;

void	link_all_lights(void)
{
	SLONG	c0;

	for(c0=1;c0<MAX_MAP_THINGS;c0++)
	{
		if(map_things[c0].Type==MAP_THING_TYPE_LIGHT)
		{
			map_things[c0].LinkSame=light_head;
			light_head=c0;
		}
	}
	
}

void	apply_all_lights_to_this_face(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra)
{
	SLONG	index;
	SLONG temp_bright;
	return;
	temp_bright=light_bright;
	index=light_head;
	while(index)
	{
		if(map_things[index].Type==MAP_THING_TYPE_LIGHT) //not needed
		{
			light_x=map_things[index].X;
			light_y=map_things[index].Y;
			light_z=map_things[index].Z;
			light_bright=map_things[index].IndexOther;
			scan_apply_light(face,x,y,z,extra);
		}
	}
	light_bright=temp_bright;
}

void	scan_apply_ambient(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra)
{
	SVector	normal;
	SLONG dx,dy,dz,dist;
	SLONG	point,bright;
	SLONG	angle_light;

	SLONG	lnx,lny,lnz;



	if(face<0)
	{	//tris

		lnx=edit_info.amb_dx;
		lny=edit_info.amb_dy;
		lnz=edit_info.amb_dz;

		calc_normal(face,&normal);
		angle_light=(normal.X*lnx+normal.Y*lny+normal.Z*lnz)>>8;
		if(angle_light<0)
			bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;
		else
			bright=(angle_light*edit_info.amb_bright)>>9;         //scale brightness because of angle to light

		if(bright<edit_info.amb_bright>>SHADOW_LIGHT_SHIFT)
			bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;

		if(bright>128)
			bright=128;

		if(edit_info.amb_flags&2)
		{
			prim_faces3[-face].Bright[0]=bright;
			prim_faces3[-face].Bright[1]=bright;
			prim_faces3[-face].Bright[2]=bright;
		}
		else
		{
			prim_faces3[-face].Bright[0]+=bright;
			prim_faces3[-face].Bright[1]+=bright;
			prim_faces3[-face].Bright[2]+=bright;
		}

	}
	else
	if(face>0)
	{ 	//quads
		dx=dy=dz=0;

		lnx=edit_info.amb_dx;
		lny=edit_info.amb_dy;
		lnz=edit_info.amb_dz;

		calc_normal(face,&normal);
		angle_light=(normal.X*lnx+normal.Y*lny+normal.Z*lnz)>>8;
		if(angle_light<0)
			bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;
		else
			bright=(angle_light*edit_info.amb_bright)>>9;         //scale brightness because of angle to light

		if(bright<edit_info.amb_bright>>SHADOW_LIGHT_SHIFT)
			bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;

		if(bright>128)
			bright=128;

//		if(bright<edit_info.amb_bright>>3)
//			bright=edit_info.amb_bright>>3;

		if(edit_info.amb_flags&2)
		{
			prim_faces4[face].Bright[0]=bright;
			prim_faces4[face].Bright[1]=bright;
			prim_faces4[face].Bright[2]=bright;
			prim_faces4[face].Bright[3]=bright;
		}
		else
		{
			prim_faces4[face].Bright[0]+=bright;
			prim_faces4[face].Bright[1]+=bright;
			prim_faces4[face].Bright[2]+=bright;
			prim_faces4[face].Bright[3]+=bright;
		}
//		apply_all_lights_to_this_face(face,x,y,z,extra);
	}
}
void	scan_unlight(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra)
{
	if(face<0)
	{	//tris
		prim_faces3[-face].Bright[0]=0;
		prim_faces3[-face].Bright[1]=0;
		prim_faces3[-face].Bright[2]=0;

	}
	else
	if(face>0)
	{ 	//quads
		prim_faces4[face].Bright[0]=0;
		prim_faces4[face].Bright[1]=0;
		prim_faces4[face].Bright[2]=0;
		prim_faces4[face].Bright[3]=0;
	}
	
}

void	scan_undo_ambient(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra)
{
	SVector	normal;
	SLONG dx,dy,dz,dist;
	SLONG	point,bright;
	SLONG	angle_light;

	SLONG	lnx,lny,lnz;

	return;



	if(face<0)
	{	//tris

		lnx=edit_info.amb_dx;
		lny=edit_info.amb_dy;
		lnz=edit_info.amb_dz;

		calc_normal(face,&normal);
		angle_light=(normal.X*lnx+normal.Y*lny+normal.Z*lnz)>>9;
		if(angle_light<0)
			bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;
		else
			bright=(angle_light*edit_info.amb_bright)>>8;         //scale brightness because of angle to light

		if(bright<edit_info.amb_bright>>SHADOW_LIGHT_SHIFT)
			bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;

		if(bright>128)
			bright=128;

		prim_faces3[-face].Bright[0]-=bright;
		prim_faces3[-face].Bright[1]-=bright;
		prim_faces3[-face].Bright[2]-=bright;
		CLIPV_0(prim_faces3[-face].Bright[0]);
		CLIPV_0(prim_faces3[-face].Bright[1]);
		CLIPV_0(prim_faces3[-face].Bright[2]);
//		apply_all_lights_to_this_face(face,x,y,z,extra);

	}
	else
	if(face>0)
	{ 	//quads
		dx=dy=dz=0;

		lnx=edit_info.amb_dx;
		lny=edit_info.amb_dy;
		lnz=edit_info.amb_dz;

		calc_normal(face,&normal);
		angle_light=(normal.X*lnx+normal.Y*lny+normal.Z*lnz)>>8;
		if(angle_light<0)
			bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;
		else
			bright=(angle_light*edit_info.amb_bright)>>9;         //scale brightness because of angle to light


		if(bright<edit_info.amb_bright>>SHADOW_LIGHT_SHIFT)
			bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;

		if(bright>128)
			bright=128;

		prim_faces4[face].Bright[0]-=bright;
		prim_faces4[face].Bright[1]-=bright;
		prim_faces4[face].Bright[2]-=bright;
		prim_faces4[face].Bright[3]-=bright;
		CLIPV_0(prim_faces4[face].Bright[0]);
		CLIPV_0(prim_faces4[face].Bright[1]);
		CLIPV_0(prim_faces4[face].Bright[2]);
		CLIPV_0(prim_faces4[face].Bright[3]);
//		apply_all_lights_to_this_face(face,x,y,z,extra);
	}
}

void	apply_light_to_floor(SLONG x,SLONG y,SLONG z,SLONG light_bright)
{
	SLONG	mx,mz,dx,dz;

	SLONG	ldx,ldy,ldz,dist;
	SLONG	bright;

	mx=x>>ELE_SHIFT;
	mz=z>>ELE_SHIFT;

	for(dx=-32;dx<32;dx++)
	for(dz=-32;dz<32;dz++)
	{
		if(dx+mx>0&&dx+mx<EDIT_MAP_WIDTH&&dz+mz>0&&dz+mz<EDIT_MAP_DEPTH)
		{
			ldx=((mx+dx)<<ELE_SHIFT)-x;
			ldy=(0)-y;
			ldz=((mz+dz)<<ELE_SHIFT)-z;

			dist=ldx*ldx+ldy*ldy+ldz*ldz;
			if(dist==0)
				dist=1;

			bright=(light_bright<<13)/dist;
			bright+=edit_map[mx+dx][mz+dz].Bright;
			if(bright<0)
				bright=edit_info.amb_bright>>SHADOW_LIGHT_SHIFT;
			else
				if(bright>128)
					bright=128;

			edit_map[mx+dx][mz+dz].Bright=bright;
		}
	}
}

void	apply_light_to_map(SLONG x,SLONG y,SLONG z,SLONG bright)
{
	return;
	apply_light_to_floor(x,y,z,bright);
	light_x=x;
	light_y=y;
	light_z=z;
	light_bright=bright;
	scan_function=scan_apply_light;
	scan_map();	
}


SWORD	CreateALightThing(SLONG x,SLONG y,SLONG z,SLONG bright)
{
	UWORD	map_thing;
	struct	MapThing	*p_mthing;

	map_thing=find_empty_map_thing();
	if(!map_thing)
		return(0);
	add_thing_to_edit_map(x>>ELE_SHIFT,z>>ELE_SHIFT,map_thing);
	p_mthing=TO_MTHING(map_thing);
	p_mthing->X=x;
	p_mthing->Y=y;
	p_mthing->Z=z;

	p_mthing->Type=MAP_THING_TYPE_LIGHT;
	p_mthing->IndexOther=(SWORD)bright;
	apply_light_to_map(x,y,z,bright);

	return(map_thing);
}

SWORD	LightTab::CreateLightThing(SLONG x,SLONG y,SLONG z,SLONG bright)
{
	UWORD	map_thing;
	struct	MapThing	*p_mthing;

	map_thing=find_empty_map_thing();
	if(!map_thing)
		return(0);
	add_thing_to_edit_map(x>>ELE_SHIFT,z>>ELE_SHIFT,map_thing);
	p_mthing=TO_MTHING(map_thing);
	p_mthing->X=x;
	p_mthing->Y=y;
	p_mthing->Z=z;

	p_mthing->Type=MAP_THING_TYPE_LIGHT;
	p_mthing->SubType = LIGHT_TYPE_NORMAL;
	p_mthing->IndexOther=(SWORD)bright;

	p_mthing->AngleX	= ((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED  ))->GetCurrentValue();
	p_mthing->AngleY	= ((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->GetCurrentValue();
	p_mthing->AngleZ	= ((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE ))->GetCurrentValue();

	p_mthing->IndexOrig	= ((CHSlider*)GetControlPtr(CTRL_LIGHT_PARAM      ))->GetCurrentValue();

	apply_light_to_map(x,y,z,bright);

	return(map_thing);
}



SLONG	LightTab::ClickOnLight(MFPoint *clicked_point)
{
	MFPoint		local_point;
	SLONG	x,y,w,h;
	SLONG	drag;

	x=Parent->ContentLeft();
	y=Parent->ContentTop();
	w=Parent->ContentWidth();
	h=Parent->ContentHeight();

	
	SetWorkWindowBounds(x,y,w-1,h/2-3);
	set_camera_plan();

	local_point	=	*clicked_point;
	drag=select_map_things(clicked_point,MAP_THING_TYPE_LIGHT);


	if(!drag)
	{
		SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
	 	set_camera_front();
		local_point	  =*clicked_point;
		local_point.Y-=h/2;
		drag=select_map_things(&local_point,MAP_THING_TYPE_LIGHT);
	}
	if(drag)
	{
		return(drag);
	}
	return(0);
}

SLONG	LightTab::DragALight(UBYTE flags,MFPoint *clicked_point,UWORD copy)
{
	SLONG	dx,dy,dz;
	UWORD	index;
	EdRect	prim_rect;
	struct	MapThing	*p_mthing;
	static	UBYTE col=0;
	SLONG	screen_change=0;
	MFPoint		local_point;
	SLONG	x,y,w,h;
	SLONG	drag;
	SLONG	wwx,wwy,www,wwh;
	SLONG	ox,oy,oz;

	SLONG	last_world_mouse;

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
	drag=select_map_things(clicked_point,MAP_THING_TYPE_LIGHT);


	if(!drag)
	{
		SetWorkWindowBounds(x,y+h/2+4,w-1,h/2-4);
	 	set_camera_front();
		local_point	  =*clicked_point;
		local_point.Y-=h/2;
		drag=select_map_things(&local_point,MAP_THING_TYPE_LIGHT);
	}

	if(copy)
	{
		drag=CreateLightThing(map_things[drag].X,
						map_things[drag].Y,
						map_things[drag].Z,
						map_things[drag].IndexOther);
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
		last_world_mouse=SetWorldMouse(0);
		offset_x=map_things[drag].X-engine.MousePosX;
		offset_y=map_things[drag].Y-engine.MousePosY;
		offset_z=map_things[drag].Z-engine.MousePosZ;
		
		CurrentLight=drag;

		while(SHELL_ACTIVE && ((copy==0&&LeftButton)||(copy==1&&RightButton)))
		{
			SLONG	nx,ny,nz;
			last_world_mouse=SetWorldMouse(0);

			nx=map_things[drag].X;
			ny=map_things[drag].Y;
			nz=map_things[drag].Z;

			if(GridFlag)
			{
				SLONG	grid_and;
				grid_and=~(HALF_ELE_SIZE-1);
				if(Axis&X_AXIS)
					nx=(engine.MousePosX+offset_x)&grid_and;
				if(Axis&Y_AXIS)
					ny=(engine.MousePosY+offset_y)&grid_and;
				if(Axis&Z_AXIS)
					nz=(engine.MousePosZ+offset_z)&grid_and;
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

			apply_light_to_map(map_things[drag].X,map_things[drag].Y,map_things[drag].Z,-map_things[drag].IndexOther);
			move_thing_on_cells(drag,nx,ny,nz);
			apply_light_to_map(nx,ny,nz,map_things[drag].IndexOther);

			DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
			ShowWorkWindow(0);
			screen_change=1;

			SetWorkWindowBounds(this->ContentLeft()+1,this->ContentTop()+1,this->ContentWidth(),this->ContentHeight()); 
//			ContentRect.FillRect(CONTENT_COL);

			DrawBox(0,0,this->ContentWidth(),this->ContentHeight(),CONTENT_COL);
			set_camera();
			draw_editor_map(0);
			render_view(0);
			ShowWorkWindow(0);
			editor_user_interface(0);
			KeyboardInterface();
		}

//		MyUndo.MoveObject(0,drag,ox,oy,oz);
		if(!last_world_mouse)
		{
			delete_thing(drag);
			CurrentLight=0;
		}
		RequestUpdate();
	}
//	SetWorkWindowBounds(this->ContentLeft()+1,this->ContentTop()+1,this->ContentWidth(),this->ContentHeight()); 
//	DrawBox(0,0,this->ContentWidth(),this->ContentHeight(),CONTENT_COL);
//	ContentRect.FillRect(CONTENT_COL);
//	UpdatePrimPickWindow();

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

	return(screen_change);
}
void	LightTab::SetAmbientAngle(void)
{
	SLONG	bright;
	//there has been a left click in a content window
	while(SHELL_ACTIVE && LeftButton)
	{
		
		if(SetWorldMouse(0))
		{
//			link_all_lights();
			scan_function=scan_undo_ambient;
			scan_map();	
			remove_ambient_from_floor();

			edit_info.amb_dx=engine.MousePosX-(engine.X>>8);
			edit_info.amb_dy=engine.MousePosY-(engine.Y>>8);
			edit_info.amb_dz=engine.MousePosZ-(engine.Z>>8);
			normalise_xyz(&edit_info.amb_dx,&edit_info.amb_dy,&edit_info.amb_dz);

		 	edit_info.amb_bright =	((CHSlider*)GetControlPtr(CTRL_LIGHT_BRIGHT))->GetCurrentValue();
//			link_all_lights();
			scan_function=scan_apply_ambient;
			scan_map();	
			apply_ambient_to_floor();

			if(LockWorkScreen())
			{
	//			DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h)
				Parent->DrawContent();
				UnlockWorkScreen();
			}
			ShowWorkWindow(0);

			{
				SetWorkWindowBounds(this->ContentLeft()+1,this->ContentTop()+1,this->ContentWidth(),this->ContentHeight()); 

				DrawBox(0,0,this->ContentWidth(),this->ContentHeight(),CONTENT_COL);
				set_camera();
				draw_editor_map(0);
				render_view(0);
				ShowWorkWindow(0);
				editor_user_interface(0);
				KeyboardInterface();
			}			


	//		RedrawModuleContent=1;
		}
	}

}
SLONG	LightTab::DragEngine(UBYTE flags,MFPoint *clicked_point)
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

SLONG	LightTab::HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h)
{
	SWORD	thing;
	SWORD	bright;
	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			if(Mode!=LIGHT_TAB_MODE_PLACE_AMBIENT)
				if(ClickOnLight(clicked_point))
					Mode=LIGHT_TAB_MODE_WAIT;

			switch (Mode)
			{
				case LIGHT_TAB_MODE_PLACE_AMBIENT:
					SetAmbientAngle();
					break;
				case LIGHT_TAB_MODE_REPEAT_PLACE_LIGHT:
					Mode=LIGHT_TAB_MODE_WAIT;

				case LIGHT_TAB_MODE_WAIT:
					DragALight(flags,clicked_point,0);
					break;
				case LIGHT_TAB_MODE_PLACE_LIGHT:
					if(SetWorldMouse(1))
					{
	 				 	bright =	((CHSlider*)GetControlPtr(CTRL_LIGHT_BRIGHT))->GetCurrentValue();
						CurrentLight=CreateLightThing(engine.MousePosX,engine.MousePosY,engine.MousePosZ,bright);
						Mode=0;
					}
					return(1);
			}
			break;
		case	RIGHT_CLICK:
			if(Mode!=LIGHT_TAB_MODE_PLACE_AMBIENT)
				if(ClickOnLight(clicked_point))
					Mode=LIGHT_TAB_MODE_WAIT;    //if you have clicked on a light you must want to drag it
			switch (Mode)
			{
				case LIGHT_TAB_MODE_PLACE_AMBIENT:
					SetAmbientAngle();
					break;
				case LIGHT_TAB_MODE_WAIT:
					DragALight(flags,clicked_point,1);
					break;
				case LIGHT_TAB_MODE_REPEAT_PLACE_LIGHT:
				case LIGHT_TAB_MODE_PLACE_LIGHT:
					if(SetWorldMouse(1))
					{
	 				 	bright =	((CHSlider*)GetControlPtr(CTRL_LIGHT_BRIGHT))->GetCurrentValue();
						CurrentLight=CreateLightThing(engine.MousePosX,engine.MousePosY,engine.MousePosZ,bright);
						Mode=LIGHT_TAB_MODE_REPEAT_PLACE_LIGHT;
					}
					return(1);
			}
			// Right click in content.
			break;
		case	MIDDLE_CLICK:
				DragEngine(flags,clicked_point);
			break;
	}
	return(0);
	
}

UWORD	LightTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
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

SLONG	LightTab::SetWorldMouse(ULONG flag)
{
	MFPoint		mouse_point;
	MFPoint		local_point;
	SVector		point,out;
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

void	delete_all_lights(void)
{
	SLONG	c0;
	

	for(c0=1;c0<MAX_MAP_THINGS;c0++)
	{
		if(map_things[c0].Type==MAP_THING_TYPE_LIGHT)
		{
			delete_thing(c0);
		}
	}

	scan_function=scan_unlight;
	scan_map();	

	edit_info.amb_offset=0;
	edit_info.amb_bright=0;

}

void	LightTab::HandleControl(UWORD control_id)
{
	switch(control_id&0xff)
	{

		case CTRL_LIGHT_FINISH_AMBIENT:
			RecalcAllLights();
			break;
		case	CTRL_LIGHT_X_AXIS_FREE:
			ToggleControlSelectedState(CTRL_LIGHT_X_AXIS_FREE);
			if(Axis&X_AXIS)
				Axis&=~X_AXIS;
			else
				Axis|=X_AXIS;
			break;
		case	CTRL_LIGHT_Y_AXIS_FREE:
			ToggleControlSelectedState(CTRL_LIGHT_Y_AXIS_FREE);
			if(Axis&Y_AXIS)
				Axis&=~Y_AXIS;
			else
				Axis|=Y_AXIS;
			break;
		case	CTRL_LIGHT_Z_AXIS_FREE:
			ToggleControlSelectedState(CTRL_LIGHT_Z_AXIS_FREE);
			if(Axis&Z_AXIS)
				Axis&=~Z_AXIS;
			else
				Axis|=Z_AXIS;
			break;
		case CTRL_LIGHT_PLACE:
			Mode=LIGHT_TAB_MODE_PLACE_LIGHT;
			break;

		case CTRL_LIGHT_SMOOTH_LOTS:
			SmoothGroup();
			RequestUpdate();
			break;
		case CTRL_LIGHT_PLACE_AMBIENT:
			((CHSlider*)GetControlPtr(CTRL_LIGHT_BRIGHT))->SetCurrentValue(256);
			CurrentLight=0;
			Mode=LIGHT_TAB_MODE_PLACE_AMBIENT;
			break;
		case	CTRL_LIGHT_SHADOW:
			ToggleControlSelectedState(CTRL_LIGHT_SHADOW);
			break;
		case	CTRL_LIGHT_DELETE:
			delete_all_lights();
			RequestUpdate();
			Mode=0;
			break;

		case CTRL_LIGHT_WHITE:
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED  ))->SetCurrentValue(255);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->SetCurrentValue(255);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE ))->SetCurrentValue(255);
			RequestUpdate();
			break;

		case CTRL_LIGHT_PALE_BLUE:
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED  ))->SetCurrentValue(255);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->SetCurrentValue(235);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE ))->SetCurrentValue(225);
			RequestUpdate();
			break;

		case CTRL_LIGHT_ORANGE:
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED  ))->SetCurrentValue(255);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->SetCurrentValue(235);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE ))->SetCurrentValue( 85);
			RequestUpdate();
			break;

		case CTRL_LIGHT_YELLOW:
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED  ))->SetCurrentValue(255);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->SetCurrentValue(255);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE ))->SetCurrentValue(105);
			RequestUpdate();
			break;

		case CTRL_LIGHT_RED:
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED  ))->SetCurrentValue(255);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->SetCurrentValue( 55);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE ))->SetCurrentValue( 55);
			RequestUpdate();
			break;

		case CTRL_LIGHT_BLUE:
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_RED  ))->SetCurrentValue( 75);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_GREEN))->SetCurrentValue( 75);
			((CHSlider*)GetControlPtr(CTRL_LIGHT_SLIDE_BLUE ))->SetCurrentValue(255);
			RequestUpdate();
			break;

		case CTRL_LIGHT_NORMAL:
			SetControlState(CTRL_LIGHT_NORMAL, CTRL_SELECTED);
			SetControlState(CTRL_LIGHT_BROKEN, CTRL_DESELECTED);
			SetControlState(CTRL_LIGHT_FLASH,  CTRL_DESELECTED);
			RequestUpdate();

			if (CurrentLight)
			{
				map_things[CurrentLight].SubType = LIGHT_TYPE_NORMAL;
			}

			break;

		case CTRL_LIGHT_BROKEN:
			SetControlState(CTRL_LIGHT_NORMAL, CTRL_DESELECTED);
			SetControlState(CTRL_LIGHT_BROKEN, CTRL_SELECTED);
			SetControlState(CTRL_LIGHT_FLASH,  CTRL_DESELECTED);
			RequestUpdate();

			if (CurrentLight)
			{
				map_things[CurrentLight].SubType = LIGHT_TYPE_BROKEN;
			}

			break;

		case CTRL_LIGHT_FLASH:
			SetControlState(CTRL_LIGHT_NORMAL, CTRL_DESELECTED);
			SetControlState(CTRL_LIGHT_BROKEN, CTRL_DESELECTED);
			SetControlState(CTRL_LIGHT_FLASH,  CTRL_SELECTED);
			RequestUpdate();

			if (CurrentLight)
			{
				map_things[CurrentLight].SubType = LIGHT_TYPE_PULSE;
			}

			break;
	}
}

//---------------------------------------------------------------
