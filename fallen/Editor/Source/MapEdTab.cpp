#include	"Editor.hpp"

#include	"MapEdTab.hpp"
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


#define	MAPED_MODE_WAIT			0


#define	CTRL_MAPED_X_AXIS_FREE		1
#define	CTRL_MAPED_Y_AXIS_FREE		2
#define	CTRL_MAPED_Z_AXIS_FREE		3
#define	CTRL_MAPED_V_SLIDE_LEVEL	4
#define	CTRL_MAPED_PAINT			5
#define	CTRL_MAPED_MARK_BLOCK		6
#define	CTRL_MAPED_COPY_BLOCK		7
#define	CTRL_MAPED_WIBBLE			8
#define	CTRL_MAPED_FLATTEN			9
#define	CTRL_MAPED_ROOF_TOP			10
#define	CTRL_MAPED_TEXTURE			11

#define	MAPED_MODE_PAINT		0
#define	MAPED_MODE_MARK			1
#define	MAPED_MODE_PASTE		2
#define	MAPED_MODE_LINE_DRAW	3
#define	MAPED_MODE_RECT_DRAW	4


ControlDef	maped_tab_def[]	=	
{
	{	CHECK_BOX,	0,	"X Free",	120,	300-10,	0,	10					},
	{	CHECK_BOX,	0,	"Y Free",	120,	313-10,	0,	10					},
	{	CHECK_BOX,	0,	"Z Free",	120,	326-10,	0,	10					},
	{	V_SLIDER,	0,	"",			272,	40,			0,	257						},
	{	BUTTON,		0,	"Paint",	10,	180,	0,	0					},
	{	BUTTON,		0,	"Mark Block",	10,	160,	0,	0					},
	{	BUTTON,		0,	"Copy",	10,	200,	0,	0					},
	{	BUTTON,		0,	"WibbleMap",10,	30,	0,	0					},
	{	BUTTON,		0,	"Flatten Map",10,	50,	0,	0					},
	{	CHECK_BOX,	0,	"Roof Tops",	200,	180-10,	0,	10					},
	{	CHECK_BOX,	0,	"Textures",	200,	180+5,	0,	10					},
	{  	0	}
};


MapEdTab	*the_maped;
	
//---------------------------------------------------------------

MapEdTab::MapEdTab(EditorModule *parent)
{
	Parent=parent;

	InitControlSet(maped_tab_def);
	AxisMode=3;

	SetControlState(CTRL_MAPED_X_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_MAPED_Y_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_MAPED_Z_AXIS_FREE,CTRL_SELECTED);
	SetControlState(CTRL_MAPED_ROOF_TOP,CTRL_DESELECTED);
	SetControlState(CTRL_MAPED_TEXTURE,CTRL_DESELECTED);
	((CVSlider*)GetControlPtr(CTRL_MAPED_V_SLIDE_LEVEL))->SetValueRange(0,128);
	((CVSlider*)GetControlPtr(CTRL_MAPED_V_SLIDE_LEVEL))->SetCurrentValue(125);
//	((CVSlider*)GetControlPtr(CTRL_PRIM_V_SLIDE_PRIM))->SetUpdateFunction(redraw_all_prims);

	Axis=X_AXIS|Y_AXIS|Z_AXIS;
	RoofTop=0;
	Texture=0;
	RedrawModuleContent=0;
	Mode=0;
	the_maped=this;
}

MapEdTab::~MapEdTab()
{
}

void	MapEdTab::Clear(void)
{
//	clear_all_col_info();
}

void	MapEdTab::DrawTabContent(void)
{
	EdRect		content_rect;


	content_rect	=	ContentRect;	
	content_rect.ShrinkRect(1,1);
	content_rect.FillRect(CONTENT_COL);

	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
	DrawControlSet();
	ShowWorkWindow(0);
}


//---------------------------------------------------------------
extern	void	hilight_col_info(void);



void	MapEdTab::DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h)
{
	SLONG	wwx,wwy,www,wwh;
	EdRect	drawrect;

	SLONG dx,dy,dz,width,height,count_across,count_high;
	SLONG	c0,c1;
	SLONG	mx,my,mz;
	SLONG	index;
	struct	EditMapElement	*p_ele;

#ifdef	POO
//	my=((CVSlider*)GetControlPtr(CTRL_MAPED_V_SLIDE_LEVEL))->GetCurrentValue();
//	my=((CVSlider*)GetControlPtr(CTRL_MAPED_V_SLIDE_LEVEL))->GetCurrentValue();

	my=(engine.Y>>8)>>ELE_SHIFT;
	mx=(engine.X>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;


	RedrawModuleContent=0;
	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;
	SetWorkWindowBounds(x,y,w-1,h-1);
	drawrect.SetRect(0,0,w-1,h-1);
	drawrect.FillRect(CONTENT_COL_BR);

	width=((16<<5)*engine.Scale)>>16;

	count_across=((w/(width+1))>>1)+1;
	count_high=(h/(width+1))>>1;

	for(dx=-count_across;dx<count_across;dx++)
	for(dz=-count_high;dz<count_high;dz++)
	{
/*
		index=edit_map[(dx+mx)][(my)].Depth[(dz+mz)];
		if(index)
		{
				p_ele=&edit_map_eles[index];
				index=p_ele->CubeType.Prim;
				DrawBoxC(0+(w>>1)+dx*(width+1),0+(h>>1)+dz*(width+1),width,width,0);
		}
		else
			DrawBoxC(0+(w>>1)+dx*(width+1),0+(h>>1)+dz*(width+1),width,width,1);

		if(my<127)
		{
			index=edit_map[(dx+mx)][(my+1)].Depth[(dz+mz)];
			if(index)
			{
					p_ele=&edit_map_eles[index];
					index=p_ele->CubeType.Prim;
					DrawVLineC((w>>1)+dx*(width+1)-1,(h>>1)+dz*(width+1)-1,(h>>1)+(dz+1)*(width+1),2);
					DrawVLineC((w>>1)+(dx+1)*(width+1),(h>>1)+dz*(width+1)-1,(h>>1)+(dz+1)*(width+1),2);
					DrawHLineC((w>>1)+dx*(width+1)-1,(w>>1)+(dx+1)*(width+1),(h>>1)+dz*(width+1)-1,2);
					DrawHLineC((w>>1)+dx*(width+1)-1,(w>>1)+(dx+1)*(width+1),(h>>1)+(dz+1)*(width+1),2);
			}
		}

		if(my>0)
		{
			index=edit_map[(dx+mx)][(my-1)].Depth[(dz+mz)];
			if(index)
			{
					p_ele=&edit_map_eles[index];
					index=p_ele->CubeType.Prim;
					DrawPixelC(0+(w>>1)+dx*(width+1)+(width>>1),0+(h>>1)+dz*(width+1)+(width>>1),2);
			}
		}
	}

	switch(Mode)
	{
		case	MAPED_MODE_MARK:
			{
				static	col=0;
				SLONG	x1,y1,x2,y2;
				col++;

				x1=(w>>1)+(X1-mx)*(width+1)-1;
				x2=(w>>1)+(X2-mx+1)*(width+1)-1;
				y1=(h>>1)+(Z1-mz)*(width+1)-1;
				y2=(h>>1)+(Z2-mz+1)*(width+1)-1;
				DrawVLineC(x1,y1,y2,(col&1)+4);
				DrawVLineC(x2,y1,y2,(col&1)+4);
				DrawHLineC(x1,x2,y1,(col&1)+4);
				DrawHLineC(x1,x2,y2,(col&1)+4);
			}
			break;
		case	MAPED_MODE_PASTE:
				MFPoint		mouse_point;
				SLONG	screen_mapx,screen_mapy,screen_mapz;
				mouse_point.X	=	MouseX;
				mouse_point.Y	=	MouseY;
				Parent->GlobalToLocal(&mouse_point);
//				DrawBoxC(mouse_point.X,mouse_point.Y,40,40,99);

				CalcMapCoord(&screen_mapx,&screen_mapy,&screen_mapz,x,y,w,h,&mouse_point);
//map to screen
				for(dx=X1;dx<=X2;dx++)
				for(dz=Z1;dz<=Z2;dz++)
				{
					index=edit_map[(dx)][(Y1)].Depth[(dz)];
					if(index)
					{
							p_ele=&edit_map_eles[index];
							index=p_ele->CubeType.Prim;

							DrawBoxC(0+(w>>1)+(screen_mapx+dx-X1-mx)*(width+1),0+(h>>1)+(screen_mapz+dz-Z1-mz)*(width+1),width,width,0);
					}
				}
			break;
	}
*/




///*

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
//*/

	SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT
#endif
}

//---------------------------------------------------------------


void	MapEdTab::HandleTab(MFPoint *current_point)
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


SLONG	MapEdTab::KeyboardInterface(void)
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
				SetControlState(CTRL_MAPED_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_MAPED_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_MAPED_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=X_AXIS;
				break;
			case	1:
				SetControlState(CTRL_MAPED_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_MAPED_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_MAPED_Z_AXIS_FREE,CTRL_DESELECTED);
				Axis=Y_AXIS;
				break;
			case	2:
				SetControlState(CTRL_MAPED_X_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_MAPED_Y_AXIS_FREE,CTRL_DESELECTED);
				SetControlState(CTRL_MAPED_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=Z_AXIS;
				break;
			case	3:
				SetControlState(CTRL_MAPED_X_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_MAPED_Y_AXIS_FREE,CTRL_SELECTED);
				SetControlState(CTRL_MAPED_Z_AXIS_FREE,CTRL_SELECTED);
				Axis=X_AXIS|Y_AXIS|Z_AXIS;
				break;
		}

		SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
  		DrawControlSet();
		ShowWorkWindow(0);
	}
	if(Keys[KB_SPACE])
	{
		Mode=0;
		RequestUpdate();
	}
	if(Mode==FLOOR_HOLD_BRUSH)
	{
		if(Keys[KB_F])
		{
			FlattenArea();
			RequestUpdate();
		}
		if(Keys[KB_S])
		{
			SmoothArea();
			RequestUpdate();
			Keys[KB_S]=0;
		}
		if(Keys[KB_Z])
		{
			SlopeArea();
			RequestUpdate();
			Keys[KB_Z]=0;
		}
	}
	if(Keys[KB_B])
	{
		if(ShiftFlag)
		{
			Mode=FLOOR_CUT_BRUSH_DEF;
		}
		else
		{
			Mode=FLOOR_CUT_BRUSH;
		}
	}
	return(0);
}

//#define	QDIST3(x,y,z)	(x>y ? (x>z ? x+(y>>2)+(z>>2) : z+(x>>2)+(y>>2)) : (y>z ? (y+(x>>2)+(z>>2) : z+(x>>2)+(y>>2) ))



SLONG	MapEdTab::DragEngine(UBYTE flags,MFPoint *clicked_point)
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

SLONG	MapEdTab::CalcMapCoord(SLONG	*mapx,SLONG	*mapy,SLONG	*mapz,SLONG	x,SLONG	y,SLONG	w,SLONG	h,MFPoint	*clicked_point)
{
	SLONG	width,count_across,count_high;
	SLONG	mx,my,mz;
	SLONG	dx,dy;

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
	return(1);
}

extern	void	insert_cube(SWORD x,SWORD y,SWORD z);
extern	void	remove_cube(SLONG x,SLONG y,SLONG z);



SLONG	MapEdTab::MouseInContent(void)
{
	if(Mode==FLOOR_PASTE_BRUSH)
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
		Parent->DrawContent();

		//DrawModuleContent(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());
		ShowWorkWindow(0);

		SetWorkWindowBounds(wwx,wwy,www,wwh); //RESTORE CLIP RECT

		
	}
	return(0);
	
}

SLONG	MapEdTab::DragPaint(UBYTE flags)
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

		while(LeftButton)
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

SLONG	MapEdTab::DragMark(UBYTE flags)
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

void	MapEdTab::ChangeMapAltitude(SLONG mx,SLONG mz,SLONG step,UBYTE offset_flag)
{
	//if the co_ord is within a cut rectangle then raise/lower whole rectangle
	if(Mode==FLOOR_HOLD_BRUSH)
	{
		if(mx>CutMapBlock.GetX()&&mx<CutMapBlock.GetX()+CutMapBlock.GetWidth()&&
			mz>CutMapBlock.GetZ()&&mz<CutMapBlock.GetZ()+CutMapBlock.GetDepth())
		{
			SLONG	dx,dz;
			for(dx=CutMapBlock.GetX();dx<CutMapBlock.GetX()+CutMapBlock.GetWidth();dx++)
			for(dz=CutMapBlock.GetZ();dz<CutMapBlock.GetZ()+CutMapBlock.GetDepth();dz++)
			{
				if(RoofTop)
				{
					if(offset_flag)
						edit_map_roof_height[dx][dz]+=step;
					else
						edit_map_roof_height[dx][dz]=step;
				}
				else
				{
					if(offset_flag)
						edit_map[dx][dz].Y+=step;
					else
						edit_map[dx][dz].Y=step;

				}
			}
		}
		else
		{
			if(RoofTop)
			{
				if(offset_flag)
					edit_map_roof_height[mx][mz]+=step;
				else
					edit_map_roof_height[mx][mz]=step;
			}
			else
			{
				if(offset_flag)
					edit_map[mx][mz].Y+=step;
				else
					edit_map[mx][mz].Y=step;
			}
		}
		return;
	}
	else
	{
		if(RoofTop)
		{
			edit_map_roof_height[mx][mz]+=step;
		}
		else
		{
			edit_map[mx][mz].Y+=step;
		}
	}

}

SLONG	MapEdTab::FlattenArea(void)
{
	if(Mode==FLOOR_HOLD_BRUSH)
	{
//		if(mx>CutMapBlock.GetX()&&mx<CutMapBlock.GetX()+CutMapBlock.GetWidth()&&
//			mz>CutMapBlock.GetZ()&&mz<CutMapBlock.GetZ()+CutMapBlock.GetDepth())
		{
			SLONG	dx,dz;
			for(dx=CutMapBlock.GetX();dx<CutMapBlock.GetX()+CutMapBlock.GetWidth();dx++)
			for(dz=CutMapBlock.GetZ();dz<CutMapBlock.GetZ()+CutMapBlock.GetDepth();dz++)
			{
				if(RoofTop)
				{
					edit_map_roof_height[dx][dz]=0;
				}
				else
				{
					edit_map[dx][dz].Y=0;
				}
			}
		}
		return(0);
	}
	else
	{
//		edit_map[mx][mz].Y+=step;
	}
	return(0);

}
SLONG	MapEdTab::SmoothArea(void)
{
	if(Mode==FLOOR_HOLD_BRUSH)
	{
//		if(mx>CutMapBlock.GetX()&&mx<CutMapBlock.GetX()+CutMapBlock.GetWidth()&&
//			mz>CutMapBlock.GetZ()&&mz<CutMapBlock.GetZ()+CutMapBlock.GetDepth())
		{
			SLONG	dx,dz;
			for(dx=CutMapBlock.GetX();dx<CutMapBlock.GetX()+CutMapBlock.GetWidth();dx++)
			for(dz=CutMapBlock.GetZ();dz<CutMapBlock.GetZ()+CutMapBlock.GetDepth();dz++)
			{
				SLONG	total=0;
				SLONG	mdx,mdz;
				for(mdx=-1;mdx<=1;mdx++)
				for(mdz=-1;mdz<=1;mdz++)
				{
					if(RoofTop)
					{
						total+=edit_map_roof_height[dx+mdx][dz+mdz];
					}
					else
					{
						total+=edit_map[dx+mdx][dz+mdz].Y;
					}
				}

				if(RoofTop)
				{
					total+=edit_map_roof_height[dx][dz]*3;
				}
				else
				{
					total+=edit_map[dx][dz].Y*3;
				}

				total=total/12;

				if(RoofTop)
				{
					edit_map_roof_height[dx][dz]=total;
				}
				else
				{
					edit_map[dx][dz].Y=total;
				}
			}
		}
		return(0);
	}
	else
	{
//		edit_map[mx][mz].Y+=step;
	}
	return(0);

}

SLONG	MapEdTab::SlopeArea(void)
{
	if(Mode==FLOOR_HOLD_BRUSH)
	{
		if(CutMapBlock.GetDepth()>1&&CutMapBlock.GetWidth()>1)
		{
			SLONG	dx,dz;
			SLONG	yl1,yl2,dly;
			SLONG	yr1,yr2,dry;

			if(RoofTop)
			{
				yl1=edit_map_roof_height[CutMapBlock.GetX()][CutMapBlock.GetZ()];
				yl2=edit_map_roof_height[CutMapBlock.GetX()][CutMapBlock.GetZ()+CutMapBlock.GetDepth()-1];
			}
			else
			{
				yl1=edit_map[CutMapBlock.GetX()][CutMapBlock.GetZ()].Y;
				yl2=edit_map[CutMapBlock.GetX()][CutMapBlock.GetZ()+CutMapBlock.GetDepth()-1].Y;

			}
			dly=((yl2-yl1)<<16)/(CutMapBlock.GetDepth()-1);

			if(RoofTop)
			{
				yr1=edit_map_roof_height[CutMapBlock.GetX()+CutMapBlock.GetWidth()-1][CutMapBlock.GetZ()];
				yr2=edit_map_roof_height[CutMapBlock.GetX()+CutMapBlock.GetWidth()-1][CutMapBlock.GetZ()+CutMapBlock.GetDepth()-1];
			}
			else
			{
				yr1=edit_map[CutMapBlock.GetX()+CutMapBlock.GetWidth()-1][CutMapBlock.GetZ()].Y;
				yr2=edit_map[CutMapBlock.GetX()+CutMapBlock.GetWidth()-1][CutMapBlock.GetZ()+CutMapBlock.GetDepth()-1].Y;

			}
			dry=((yr2-yr1)<<16)/(CutMapBlock.GetDepth()-1);

			yl1<<=16;
			yr1<<=16;

			for(dz=CutMapBlock.GetZ()+1;dz<CutMapBlock.GetZ()+CutMapBlock.GetDepth()-1;dz++)
			{
				yl1+=dly;
				yr1+=dry;
				if(RoofTop)
				{
					edit_map_roof_height[CutMapBlock.GetX()][dz]=(yl1>>16);
					edit_map_roof_height[CutMapBlock.GetX()+CutMapBlock.GetWidth()-1][dz]=yr1>>16;
				}
				else
				{
					edit_map[CutMapBlock.GetX()][dz].Y=(yl1>>16);
					edit_map[CutMapBlock.GetX()+CutMapBlock.GetWidth()-1][dz].Y=yr1>>16;

				}
			}

			for(dz=CutMapBlock.GetZ();dz<CutMapBlock.GetZ()+CutMapBlock.GetDepth();dz++)
			{
				if(RoofTop)
				{
					yl1=edit_map_roof_height[CutMapBlock.GetX()][dz];
					yr1=edit_map_roof_height[CutMapBlock.GetX()+CutMapBlock.GetWidth()-1][dz];
				}
				else
				{
					yl1=edit_map[CutMapBlock.GetX()][dz].Y;
					yr1=edit_map[CutMapBlock.GetX()+CutMapBlock.GetWidth()-1][dz].Y;
				}

				dry=((yr1-yl1)<<16)/(CutMapBlock.GetWidth()-1);

				yl1<<=16;
				for(dx=CutMapBlock.GetX()+1;dx<CutMapBlock.GetX()+CutMapBlock.GetWidth()-1;dx++)
				{
					yl1+=dry;

					if(RoofTop)
					{
						edit_map_roof_height[dx][dz]=yl1>>16;
					}
					else
					{
						edit_map[dx][dz].Y=yl1>>16;
					}


				}
			}

			/*
			for(dx=CutMapBlock.GetX();dx<CutMapBlock.GetX()+CutMapBlock.GetWidth();dx++)
			for(dz=CutMapBlock.GetZ();dz<CutMapBlock.GetZ()+CutMapBlock.GetDepth();dz++)
			{
				SLONG	total=0;
				SLONG	mdx,mdz;
				for(mdx=-1;mdx<=1;mdx++)
				for(mdz=-1;mdz<=1;mdz++)
					total+=edit_map[dx+mdx][dz+mdz].Y;
				total+=edit_map[dx][dz].Y*3;

				total=total/12;

				edit_map[dx][dz].Y=total;
			}
			*/
		}
		return(0);
	}
	else
	{
//		edit_map[mx][mz].Y+=step;
	}
		return(0);

}

void	MapEdTab::DragAltitude(SLONG mx,SLONG mz)
{
	SLONG	wwx,wwy,www,wwh;
	SLONG	alt1,starty;
	SLONG	prev_alt;
	alt1=MouseY;

	if(RoofTop)
	{
		starty=edit_map_roof_height[mx>>ELE_SHIFT][mz>>ELE_SHIFT];
	}
	else
	{
		starty=edit_map[mx>>ELE_SHIFT][mz>>ELE_SHIFT].Y;

	}
	prev_alt=starty;

	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;

	SetWorkWindowBounds(Parent->ContentLeft()+1,Parent->ContentTop()+1,Parent->ContentWidth(),Parent->ContentHeight());

	while(SHELL_ACTIVE && LeftButton)
	{
		SLONG	alt;
		SLONG	req_alt,dy;
		alt=(MouseY-alt1)>>1;

		req_alt=alt+starty;
		dy=prev_alt-req_alt;
//		edit_map[mx>>ELE_SHIFT][mz>>ELE_SHIFT].Y=alt+starty;
		if(ControlFlag)
		{
			dy=req_alt;
			ChangeMapAltitude(mx>>ELE_SHIFT,mz>>ELE_SHIFT,dy,0);

		}
		else
		{
			ChangeMapAltitude(mx>>ELE_SHIFT,mz>>ELE_SHIFT,dy,1);
		}

		Parent->DrawContent();
		ShowWorkWindow(0);
		prev_alt=req_alt;
	}

	SetWorkWindowBounds(wwx,wwy,www,wwh);

}

SLONG	MapEdTab::HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h)
{
	SWORD	thing;
	SWORD	bright;
	SLONG	mx,my,mz;
	SLONG	step=1;


	switch(Mode)
	{
		case	0:
		case	FLOOR_HOLD_BRUSH:
			switch(flags)
			{
				case	NO_CLICK:
					break;
				case	LEFT_CLICK:

						if(ShiftFlag)
						{
							Mode=FLOOR_CUT_BRUSH_DEF;
							CutFloorBrush(clicked_point,1);
						}
						else
						{
							BuildMode->CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);
							DragAltitude(mx,mz);

						}
/*
						LogText(" LEFT \n");

						mx=mx>>ELE_SHIFT;
						mz=mz>>ELE_SHIFT;
						if(ShiftFlag)
							step=10;
						if(ControlFlag)
							step=100;
						ChangeMapAltitude(mx,mz,step);
						//edit_map[mx][mz].Y+=step;
*/
						return(1);

					break;
				case	RIGHT_CLICK:
							Mode=FLOOR_CUT_BRUSH;
							CutFloorBrush(clicked_point,2);
/*
						LogText(" RIGHT \n");
	
						BuildMode->CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);
						mx=mx>>ELE_SHIFT;
						mz=mz>>ELE_SHIFT;
						if(ShiftFlag)
							step=10;
						if(ControlFlag)
							step=100;
						ChangeMapAltitude(mx,mz,-step);
*/
						//edit_map[mx][mz].Y-=step;
//					remove_cube(mx,my,mz);
					return(1);

					break;
				case	MIDDLE_CLICK:
						//DragEngine(flags,clicked_point);
							//if(SubMode==FLOOR_CUT_BRUSH)
/*
							{
								LogText(" MIDDLE \n");
								CutFloorBrush(clicked_point);
							}
*/
					break;
			}
			break;
		case	FLOOR_PASTE_BRUSH:
				switch(flags)
				{
					case	LEFT_CLICK:
						BuildMode->CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);
						CutMapBlock.Paste(mx>>ELE_SHIFT,mz>>ELE_SHIFT,PASTE_ALTITUDE,RoofTop);
						break;

					case	RIGHT_CLICK:
							Mode=FLOOR_CUT_BRUSH;
							CutFloorBrush(clicked_point,2);
						break;
				}
			break;

		case	FLOOR_CUT_BRUSH:
		case	FLOOR_CUT_BRUSH_DEF:
			switch(flags)
			{
				case	LEFT_CLICK:
					CutFloorBrush(clicked_point,1);
					break;
			}
			break;


	}
	if(flags==MIDDLE_CLICK)
		LogText(" middle2\n");


/*
	switch(Mode)
	{
		case	MAPED_MODE_MARK:
			DragMark(flags);
			break;
		case	MAPED_MODE_PASTE:
			switch(flags)
			{
				case	NO_CLICK:
					break;
				case	LEFT_CLICK:
					{
					
						SLONG	mx,my,mz,dx,dz,index;

						CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);
		//map to screen
						for(dx=X1;dx<=X2;dx++)
						for(dz=Z1;dz<=Z2;dz++)
						{
							index=edit_map[(dx)][(Y1)].Depth[(dz)];
							if(index)
							{
								insert_cube(mx+dx-X1,my,mz+dz-Z1);
							}
						}
					}
					break;
			}
			break;

		case	0:
			switch(flags)
			{
				case	NO_CLICK:
					break;
				case	LEFT_CLICK:

						CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);
						insert_cube(mx,my,mz);
						DragPaint(flags);
						
						return(1);

					switch (Mode)
					{
		//				case	MAPED_MODE_WAIT:
		//					DragACol(flags,clicked_point,0);
							break;

					}
					break;
				case	RIGHT_CLICK:

					CalcMapCoord(&mx,&my,&mz,x,y,w,h,clicked_point);
					remove_cube(mx,my,mz);
					return(1);

					switch (Mode)
					{
					}
					// Right click in content.
					break;
				case	MIDDLE_CLICK:
						DragEngine(flags,clicked_point);
					break;
			}
			break;
	}
*/
	return(0);
	
}


UWORD	MapEdTab::HandleTabClick(UBYTE flags,MFPoint *clicked_point)
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

void	MapEdTab::CutFloorBrush(MFPoint *current_point,SLONG button)
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
	while(SHELL_ACTIVE && ((LeftButton&&button==1)||(RightButton&&button==2)))
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
		CutMapBlock.Cut(mx1>>ELE_SHIFT,mz1>>ELE_SHIFT,mw>>ELE_SHIFT,mh>>ELE_SHIFT,RoofTop);
		if(Mode==FLOOR_CUT_BRUSH_DEF)
		{
			Mode=FLOOR_HOLD_BRUSH;
		}
		else
		{
			Mode=FLOOR_PASTE_BRUSH;
		}

	}

	RequestUpdate();
}


//---------------------------------------------------------------

SLONG	MapEdTab::SetWorldMouse(ULONG flag)
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

SLONG	calc_edit_height_at(SLONG x,SLONG z)
{
	DepthStrip *me;
	SLONG	new_y,h0,h1,h2,h3;

	if(x<0||z<0||x>EDIT_MAP_WIDTH<<8||z>EDIT_MAP_WIDTH<<8)
		return(0);
	me=&edit_map[(x>>ELE_SHIFT)][(z>>ELE_SHIFT)];
//	me=&MAP2((x>>ELE_SHIFT),(z>>ELE_SHIFT));

	h1=me->Y<<FLOOR_HEIGHT_SHIFT;                //my_big_map[(new_x.L.HWord)+((new_z.L.HWord)*MAP_WIDTH)].Alt;
	if( (x>>8) >=(MAP_WIDTH))
		h0=h1;
	else
		h0=((me+1)->Y)<<FLOOR_HEIGHT_SHIFT;            //my_big_map[(new_x.L.HWord+1)+((new_z.L.HWord)*MAP_WIDTH)].Alt;

	if( (z>>8) >=(MAP_HEIGHT))
	{
		h2=h1;
		h3=h0;
	}
	else
	{
		h2=((me+MAP_WIDTH)->Y)<<FLOOR_HEIGHT_SHIFT;     //my_big_map[(new_x.L.HWord)+((new_z.L.HWord+1)*MAP_WIDTH)].Alt;
		h3=((me+MAP_WIDTH+1)->Y)<<FLOOR_HEIGHT_SHIFT;   //my_big_map[(new_x.L.HWord+1)+((new_z.L.HWord+1)*MAP_WIDTH)].Alt;
	}
	if(h0==h1&&h1==h2&&h2==h3)
	{
		return(h0);
	}
	x=x&(~ELE_AND);	
	z=z&(~ELE_AND);	

	if(x+z<ELE_SIZE)
		new_y=(h1+(((h0-h1)*(x))>>ELE_SHIFT)+(((h2-h1)*(z))>>ELE_SHIFT));
	else
		new_y=(h3+(((h2-h3)*(ELE_SIZE-x))>>ELE_SHIFT)+(((h0-h3)*(ELE_SIZE-z))>>ELE_SHIFT));
	
	return(new_y);
}


extern	void set_map_height(SLONG x,SLONG z,SLONG y);


void	fix_furn_height(void)
{	
	SLONG	c0;
	for(c0=1;c0<MAX_MAP_THINGS;c0++)
	{
		if(map_things[c0].Type==MAP_THING_TYPE_PRIM)
		{
			SLONG	y;

			y=calc_edit_height_at(map_things[c0].X,map_things[c0].Z);
			
			map_things[c0].Y=y;

		}
	}
}

void	set_all_map_height(SLONG x,SLONG z,SLONG y)
{
		edit_map[x][z].Y=(SBYTE)y;
}



void	wibble_map(void)
{
	SLONG	dx,dz;
	for(dx=0;dx<EDIT_MAP_WIDTH;dx++)
	for(dz=0;dz<EDIT_MAP_DEPTH;dz++)
	{
		set_all_map_height(dx,dz,(COS((dx*15)&2047)+SIN((dz*15)&2047))>>10);
	}
	fix_furn_height();
}

void	flatten_map(void)
{
	SLONG	dx,dz;
	for(dx=0;dx<EDIT_MAP_WIDTH;dx++)
	for(dz=0;dz<EDIT_MAP_DEPTH;dz++)
	{
		set_all_map_height(dx,dz,0);
	}
	fix_furn_height();
}



void	MapEdTab::HandleControl(UWORD control_id)
{
	switch(control_id&0xff)
	{

		case	CTRL_MAPED_ROOF_TOP:
			ToggleControlSelectedState(CTRL_MAPED_ROOF_TOP);
			if(RoofTop)
				RoofTop=0;
			else
				RoofTop=1;
			break;
		case	CTRL_MAPED_TEXTURE:
			ToggleControlSelectedState(CTRL_MAPED_TEXTURE);
			if(Texture)
				Texture=0;
			else
				Texture=1;
			break;
		case	CTRL_MAPED_X_AXIS_FREE:
			ToggleControlSelectedState(CTRL_MAPED_X_AXIS_FREE);
			if(Axis&X_AXIS)
				Axis&=~X_AXIS;
			else
				Axis|=X_AXIS;
			break;
		case	CTRL_MAPED_Y_AXIS_FREE:
			ToggleControlSelectedState(CTRL_MAPED_Y_AXIS_FREE);
			if(Axis&Y_AXIS)
				Axis&=~Y_AXIS;
			else
				Axis|=Y_AXIS;
			break;
		case	CTRL_MAPED_Z_AXIS_FREE:
			ToggleControlSelectedState(CTRL_MAPED_Z_AXIS_FREE);
			if(Axis&Z_AXIS)
				Axis&=~Z_AXIS;
			else
				Axis|=Z_AXIS;
			break;
		case	CTRL_MAPED_MARK_BLOCK:
			Mode=MAPED_MODE_MARK;
			break;
		case	CTRL_MAPED_PAINT:
			Mode=MAPED_MODE_PAINT;
			break;
		case	CTRL_MAPED_COPY_BLOCK:
			Mode=MAPED_MODE_PASTE;
			break;
		case	CTRL_MAPED_WIBBLE:
			wibble_map();
			break;
		case	CTRL_MAPED_FLATTEN:
			flatten_map();
			break;

	}
}

//---------------------------------------------------------------
