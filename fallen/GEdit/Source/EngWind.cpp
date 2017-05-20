//	EngWind.cpp
//	Guy Simmons, 27th July 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"fmatrix.h"
#include	"inline.h"
#include	"gi.h"

#include	"MapView.h"
#include	"Mission.h"
#include	"WayWind.h"


//---------------------------------------------------------------



extern int				waypoint_colour,
						waypoint_group;
extern volatile	BOOL	ShellActive;
extern CBYTE			*GEDIT_engine_name;
extern UBYTE			button_colours[][3];
extern HCURSOR			GEDIT_arrow;
extern HICON			GEDIT_app_icon;
extern HINSTANCE		GEDIT_hinstance;
extern HMENU			GEDIT_main_menu;
extern HWND				GEDIT_client_wnd,
						GEDIT_edit_wnd,
						GEDIT_engine_wnd,
						GEDIT_frame_wnd;
extern WNDCLASSEX		GEDIT_class_engine;

//void		GI_init();
//SLONG		GI_load_map(CBYTE *name);

//---------------------------------------------------------------

void	calc_camera_pos(void)
{
	FMATRIX_calc(
					cam_matrix,
					cam_yaw,
					cam_pitch,
					0
				);

	cam_x	=	cam_focus_x;
	cam_y	=	0x100; // PAP_calc_height_at(LEDIT_cam_focus_x, LEDIT_cam_focus_z) + 0x100;
	cam_z	=	cam_focus_z;

	cam_x	-=	MUL64(cam_matrix[6], cam_focus_dist);
	cam_y	-=	MUL64(cam_matrix[7], cam_focus_dist);
	cam_z	-=	MUL64(cam_matrix[8], cam_focus_dist);

	FMATRIX_vector	(
						cam_forward,
						cam_yaw,
						0
					);

	FMATRIX_vector	(
						cam_left,
						(cam_yaw + 512) & 2047,
						0
					);
}
/*
//---------------------------------------------------------------
//	WindProc for engine window.

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK	engine_proc	(
									HWND hWnd,
									UINT message,
									WPARAM wParam,
									LPARAM lParam
								)
{
	SLONG			colour;
	EventPoint		*new_event;
	HDC				hdc;
	HRESULT			result;
	PAINTSTRUCT		ps;
	POINT			client_pos;
	RECT			dst,
					src;
	static HWND		hclient_wnd,
					hframe_wnd;


	switch(message)
	{
		case	WM_CREATE:
			//	Default setup for map view.
			cam_focus_x		=	64 << 8;
			cam_focus_z		=	64 << 8;
			cam_focus_dist	=	14 << 8;
			cam_pitch		=	1700;
			cam_yaw			=	0;
			calc_camera_pos();
			GI_render_view_into_backbuffer	(
												cam_x,
												cam_y,
												cam_z,
												cam_yaw,
												cam_pitch,
												0
											);

			//	Save important window handles.
			hclient_wnd	=	GetParent(hWnd);
			hframe_wnd	=	GetParent(hclient_wnd);
			return	0;

		case	WM_KEYDOWN:
		case	WM_KEYUP:
			KeyboardProc(message,wParam,lParam);
			return	0;

		case	WM_PAINT:
			hdc	=	BeginPaint(hWnd,&ps);
			
			client_pos.x	=	0;
			client_pos.y	=	0;
			ClientToScreen(hWnd,&client_pos);

			GetClientRect(hWnd,&src);
			dst	=	src;
			OffsetRect(&dst,client_pos.x,client_pos.y);

			//	Set the clipper.
			result	=	the_display.lp_DD_Clipper->SetHWnd(0,hWnd);

			//	Blit the engine.
			result	=	the_display.lp_DD_FrontSurface->Blt(&dst,the_display.lp_DD_BackSurface,&src,DDBLT_WAIT,0);

			EndPaint(hWnd,&ps);
			return	0;

		case	WM_LBUTTONDOWN:
			if(map_valid && mouse_valid)
			{
				colour	=	(button_colours[waypoint_colour][0]<<16)	|
							(button_colours[waypoint_colour][1]<<8)		|
							(button_colours[waypoint_colour][2]);

				new_event	=	MISSION_create_eventpoint();
				if(new_event)
				{
					new_event->X		=	mouse_world_x;
					new_event->Y		=	mouse_world_y;
					new_event->Z		=	mouse_world_z;
					new_event->Colour	=	waypoint_colour;
					new_event->Group	=	waypoint_group;
				}
			}
			return	0;
	}
	return	DefMDIChildProc(hWnd,message,wParam,lParam);
}

//---------------------------------------------------------------

BOOL	init_ewind(void)
{
	DWORD		style,
				style_ex;
	RECT		engine_rect;


	//	Sneakily pretend that this was the window created by SetupHost!
//	hDDLibWindow	=	GEDIT_frame_wnd;
	hDDLibWindow	=	GEDIT_edit_wnd;
	ShellActive		=	TRUE;

	//	Open this display using our engine window.
	if(OpenDisplay(640,480,16,FLAGS_USE_3D) != 0)
		return	FALSE;		//	Couldn't open the display.

	//	Create engine window class.
	GEDIT_class_engine.cbSize			=	sizeof(WNDCLASSEX);
	GEDIT_class_engine.style			=	CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
	GEDIT_class_engine.lpfnWndProc		=	engine_proc;
	GEDIT_class_engine.cbClsExtra		=	0;
	GEDIT_class_engine.cbWndExtra		=	sizeof(HANDLE);
	GEDIT_class_engine.hInstance		=	GEDIT_hinstance;
	GEDIT_class_engine.hIcon			=	GEDIT_app_icon;
	GEDIT_class_engine.hCursor			=	GEDIT_arrow;
	GEDIT_class_engine.hbrBackground	=	GetStockObject(LTGRAY_BRUSH);
	GEDIT_class_engine.lpszMenuName		=	NULL;
	GEDIT_class_engine.lpszClassName	=	GEDIT_engine_name;
	GEDIT_class_engine.hIconSm			=	GEDIT_app_icon;
	if(!RegisterClassEx(&GEDIT_class_engine))
		return	FALSE;		//	Couldn't register the class.

	return	TRUE;
}

//---------------------------------------------------------------

void	fini_ewind(void)
{
	GI_fini();
//	CloseDisplay();

	DestroyWindow(GEDIT_engine_wnd);
	UnregisterClass(GEDIT_engine_name, GEDIT_hinstance);
}

//---------------------------------------------------------------

BOOL	open_map(MDICREATESTRUCT *mdi_create)
{
	CBYTE		w_name[_MAX_PATH];
	DWORD		style;
	RECT		engine_rect;


	//	Do a bodge load of the map.
	GI_init();
	map_valid		=	GI_load_map(map_name);
	if(map_valid)
	{
		//	The standard styles for an MDI child window.
		style	=	WS_CHILD |WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
					WS_SYSMENU | WS_CAPTION | WS_THICKFRAME |
					WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

		//	Set the client rect size.
		SetRect(&engine_rect,0,0,640,480);
		AdjustWindowRect(
							&engine_rect,
							style,
							FALSE
						);

		//	The window name.
		sprintf(w_name,"Map - %s",map_name);

		//	Set up the MDI Create structure.
		mdi_create->szClass	=	GEDIT_engine_name;
		mdi_create->szTitle	=	map_name;
		mdi_create->hOwner	=	GEDIT_hinstance;
		mdi_create->x		=	CW_DEFAULT;
		mdi_create->y		=	CW_DEFAULT;
		mdi_create->cx		=	engine_rect.right  - engine_rect.left;
		mdi_create->cy		=	engine_rect.bottom - engine_rect.top;
		mdi_create->style	=	0;
		mdi_create->lParam	=	0;

		return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

#define	SCROLL_RATE		4
#define	ZOOM_RATE		4
#define	YAW_RATE		1
#define	PITCH_RATE		1

void	process_ewind(void)
{
	ULONG			colour;
	SLONG			df,dl,dy,dp,dd,
					dist,dx,dz;
	EventPoint		*current_epoint;
	POINT			mouse;
	RECT			client_rect;


	if(map_valid)
	{
		calc_camera_pos();

		df	=	0;
		dl	=	0;
		dd	=	0;
		dy	=	0;
		dp	=	0;

		if(Keys[KB_LEFT ])	{dl	+=	SCROLL_RATE;	}
		if(Keys[KB_RIGHT])	{dl	-=	SCROLL_RATE;	}
		if(Keys[KB_UP   ])	{df	+=	SCROLL_RATE;	}
		if(Keys[KB_DOWN ])	{df	-=	SCROLL_RATE;	}

		if(Keys[KB_HOME ])	{dd -=	ZOOM_RATE;		}
		if(Keys[KB_END  ])	{dd +=	ZOOM_RATE;		}

		if(Keys[KB_DEL  ])	{dy -=	YAW_RATE;		}
		if(Keys[KB_PGDN ])	{dy +=	YAW_RATE;		}
		if(Keys[KB_INS  ])	{dp -=	PITCH_RATE;		}
		if(Keys[KB_PGUP ])	{dp +=	PITCH_RATE;		}

		// Are we moving?
		if(ShiftFlag)
		{
			dl <<= 2;
			df <<= 2;
			dd <<= 2;
			dy <<= 2;
			dp <<= 2;
		}

		// Update position.
		cam_focus_x += df * cam_forward[0] >> 12;
		cam_focus_z += df * cam_forward[2] >> 12;

		cam_focus_x += dl * cam_left[0] >> 12;
		cam_focus_z += dl * cam_left[2] >> 12;

		cam_focus_dist += dd * 16;
		cam_yaw        += dy * 16;
		cam_pitch      += dp * 16;

		cam_yaw   &= 2047;
		cam_pitch &= 2047;

		calc_camera_pos();

		// Draw the engine.
		GI_render_view_into_backbuffer	(
											cam_x,
											cam_y,
											cam_z,
											cam_yaw,
											cam_pitch,
											0
										);

		//	Get mouse position relative to engine window.
		GetCursorPos(&mouse);
		ScreenToClient(GEDIT_engine_wnd,&mouse);
		GetClientRect(GEDIT_engine_wnd,&client_rect);

		//	Draw all waypoints within a certain distance of the camera.
		current_epoint	=	used_epoints;
		while(current_epoint)
		{
			dx		=	current_epoint->X	-	cam_x;
			dz		=	current_epoint->Z	-	cam_z;
			dist	=	QDIST2(abs(dx),abs(dz));

			if(dist < (20 << 8))
			{
				//	Draw it.
				colour	=	(button_colours[current_epoint->Colour][0]<<16)	|
							(button_colours[current_epoint->Colour][1]<<8)	|
							(button_colours[current_epoint->Colour][2]);

				GI_waypoint_draw	(
										mouse.x,
										mouse.y,
										current_epoint->X,
										current_epoint->Y + 0x100,
										current_epoint->Z,
										colour,
										0
									);
			}

			current_epoint	=	current_epoint->Next;
		}

		// Is the mouse in the engine window?
		if(PtInRect(&client_rect,mouse))
		{
			mouse_over	=	0;
			mouse_valid	=	GI_get_pixel_world_pos	(
														mouse.x,
														mouse.y,
														&mouse_world_x,
														&mouse_world_y,
														&mouse_world_z
													);

		}
		else
		{
			mouse_valid	=	0;
		}

		if(mouse_valid)
		{
			colour	=	(button_colours[waypoint_colour][0]<<16)	|
						(button_colours[waypoint_colour][1]<<8)		|
						(button_colours[waypoint_colour][2]);

			GI_waypoint_draw	(
									mouse.x,
									mouse.y,
									mouse_world_x,
									mouse_world_y + 0x100,
									mouse_world_z,
									colour,
									0
								);
		}


		InvalidateRect(GEDIT_engine_wnd, NULL, FALSE);

	}
}

//---------------------------------------------------------------
*/