//	MapView.cpp
//	Guy Simmons, 12th August 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"fmatrix.h"
#include	"inline.h"
#include	"gi.h"

#include	"GEdit.h"
#include	"EdStrings.h"
#include	"WaypointSetup.h"
#include	"EnemySetup.h"
#include	"ItemSetup.h"
#include	"Mission.h"
#include	"PlayerSetup.h"
#include	"TriggerSetup.h"
#include	"CameraSetup.h"
#include	"PlatformSetup.h"
#include	"CamTargetSetup.h"
#include	"MapExitSetup.h"
#include	"ActivateSetup.h"
#include	"TrapSetup.h"
#include	"WayWind.h"
#include	"WSpace.h"
#include	"C:\fallen\DDEngine\Headers\poly.h"
#include	"inputbox.h"
#include	"pap.h"
#include	"mav.h"

#include	"mesh.h"
#include	"ob.h"

#include	"game.h"
#include	"inside2.h"
#include	"c:\fallen\headers\memory.h"

#include	"cutscene.h"

//---------------------------------------------------------------
// from supermap.cpp
extern UWORD	calc_inside_for_xyz(SLONG x,SLONG y,SLONG z,UWORD *room);

// from aeng.cpp
extern UBYTE AENG_transparent_warehouses;


//---------------------------------------------------------------

void	do_creature_setup(EventPoint *the_ep);
void	do_message_setup(EventPoint *the_ep);
//void	do_shout_setup(EventPoint *the_ep);
void	do_vehicle_setup(EventPoint *the_ep);
void	do_bomb_setup(EventPoint *the_ep);
void	do_burn_setup(EventPoint *the_ep);
CBYTE	*get_vehicle_message(EventPoint *ep, CBYTE *msg);
void	do_vfx_setup(EventPoint *the_ep);
void	do_sfx_setup(EventPoint *the_ep);
void	do_wpt_pick(EventPoint *the_ep);
void	do_barrel_setup(EventPoint *the_ep);
void	do_spotfx_setup(EventPoint *the_ep);
CBYTE	*get_spotfx_message(EventPoint *ep, CBYTE *msg);
void	do_warefx_setup(EventPoint *the_ep);
CBYTE	*get_warefx_message(EventPoint *ep, CBYTE *msg);
void	do_treasure_setup(EventPoint *the_ep);
CBYTE	*get_treasure_message(EventPoint *ep, CBYTE *msg);
void	do_bonus_setup(EventPoint *the_ep);
//CBYTE	*get_bonus_message(EventPoint *ep, CBYTE *msg);
void	do_converse_setup(EventPoint *the_ep);
void	do_counter_setup(EventPoint *the_ep);
CBYTE	*get_counter_message(EventPoint *ep, CBYTE *msg);
void	do_lite_setup(EventPoint *the_ep);
CBYTE	*get_lite_message(EventPoint *ep, CBYTE *msg);
void	do_nav_setup(EventPoint *the_ep);
void	do_anim_pick(EventPoint *the_ep);
void	do_transfer_pick(EventPoint *the_ep);
void	do_lock_setup(EventPoint *the_ep);
void	do_reset_pick(EventPoint *the_ep);
void	do_enemy_flags_setup(EventPoint *the_ep);
void	do_stall_setup(EventPoint *the_ep);
void	do_extend_setup(EventPoint *the_ep);
void	do_move_setup(EventPoint *the_ep);
void	do_pee_setup(EventPoint *the_ep);
void	do_sign_setup(EventPoint *the_ep);




//---------------------------------------------------------------

CBYTE	*get_message_message(EventPoint *ep, CBYTE *msg);

//---------------------------------------------------------------

#define	EM_NONE					0
#define	EM_PLACE_WAYPOINT		1
#define	EM_EDIT_WAYPOINT		2



BOOL			dragging_ep	=	FALSE,
				map_valid	=	FALSE;
UBYTE			link_mode = 0;

//	The camera.
SLONG			cam_x,
				cam_y,
				cam_z,
				cam_yaw,
				cam_pitch,
				cam_focus_x,
				cam_focus_y,
				cam_focus_z,
				cam_focus_dist,
				cam_matrix[9],
				cam_forward[3],
				cam_left[3];

//	The mouse.
SLONG			mouse_valid,
				mouse_over,
				mouse_world_x,
				mouse_world_y,
				mouse_world_z,
				mouse_waypoint;

EventPoint		*hilited_ep		=	NULL,
				*selected_ep	=	NULL,
				*link_start_ep	=	NULL;

extern int		waypoint_colour,
				waypoint_group;
extern CBYTE	*GEDIT_map_name;
extern BOOL		map_valid;
extern CBYTE	map_name[];
extern UBYTE	button_colours[][3];
extern UBYTE	edit_mode;
extern UBYTE	leaping_disabled;
extern SLONG	prim_num,
				prim_height,
				prim_index,
				prim_drag,
				prim_dir,
				prim_x,prim_z;
extern BOOL		prim_psxmode;

//	The camera.
extern SLONG	cam_x,
				cam_y,
				cam_z,
				cam_yaw,
				cam_pitch,
				cam_focus_x,
				cam_focus_z,
				cam_focus_dist,
				cam_matrix[9],
				cam_forward[3],
				cam_left[3];

//	The mouse.
extern SLONG	mouse_valid,
				mouse_over,
				mouse_world_x,
				mouse_world_y,
				mouse_world_z,
				mouse_waypoint;

extern HCURSOR			GEDIT_arrow;
extern HINSTANCE		GEDIT_hinstance;
extern HWND				GEDIT_client_wnd,
						GEDIT_engine_wnd,
						GEDIT_edit_wnd,
						GEDIT_frame_wnd,
						GEDIT_view_wnd,
						GEDIT_way_wnd;

extern void		calc_camera_pos(void);
extern UBYTE aeng_draw_cloud_flag;

SLONG zone_colours[ZF_NUM] = { 0x000000, 0x7f7f7f, 0xff0000, 0x0000ff, 0x00ffff, 0xffff00, 0xff00ff, 0x555555};

//---------------------------------------------------------------

BOOL TypeHasProperties(SLONG type) {
	switch(type) {
		case WPT_NONE:
		case WPT_TELEPORT:
		case WPT_TELEPORT_TARGET:
		case WPT_END_GAME_LOSE:
		case WPT_END_GAME_WIN:
		case WPT_GROUP_LIFE:
		case WPT_GROUP_DEATH:
		case WPT_MAKE_SEARCHABLE:
		case WPT_GROUP_RESET:
		case WPT_COUNT_UP_TIMER:
		case WPT_CREATE_MIST:
		case WPT_CONE_PENALTIES:
		case WPT_NO_FLOOR:
		case WPT_SHAKE_CAMERA:
			return 0;

		case WPT_CREATE_PLAYER:
		case WPT_CREATE_ENEMIES:
		case WPT_CREATE_VEHICLE:
		case WPT_CREATE_ITEM:
		case WPT_CREATE_CREATURE:
		case WPT_CREATE_CAMERA:
		case WPT_CREATE_TARGET:
		case WPT_SOUND_EFFECT:
		case WPT_VISUAL_EFFECT:
		case WPT_SPOT_EFFECT:
		case WPT_MESSAGE:
		case WPT_SHOUT:
		case WPT_CAMERA_WAYPOINT:
		case WPT_TARGET_WAYPOINT:
		case WPT_CREATE_MAP_EXIT:
		case WPT_ACTIVATE_PRIM:
		case WPT_CREATE_TRAP:
		case WPT_ADJUST_ENEMY:
		case WPT_LINK_PLATFORM:
		case WPT_SIMPLE:
		case WPT_CREATE_BOMB:
		case WPT_BURN_PRIM:
		case WPT_NAV_BEACON:
		case WPT_CREATE_BARREL:
		case WPT_KILL_WAYPOINT:
		case WPT_CREATE_TREASURE:
		case WPT_BONUS_POINTS:
		case WPT_CONVERSATION:
		case WPT_INCREMENT:
		case WPT_DYNAMIC_LIGHT:
		case WPT_CUT_SCENE:
		case WPT_GOTHERE_DOTHIS:
		case WPT_TRANSFER_PLAYER:
		case WPT_LOCK_VEHICLE:
		case WPT_RESET_COUNTER:
		case WPT_ENEMY_FLAGS:
		case WPT_STALL_CAR:
		case WPT_EXTEND:
		case WPT_MOVE_THING:
		case WPT_MAKE_PERSON_PEE:
		case WPT_SIGN:
		case WPT_WAREFX:
			return 1;

	}
	return 0;
}

BOOL HasProperties(EventPoint *ep) {
	return TypeHasProperties(ep->WaypointType);
}

void CleanProperties(EventPoint *ep) {
/*	switch(ep->WaypointType) {
	case WPT_MESSAGE:
	case WPT_CONVERSATION:
	case WPT_BONUS_POINTS:
	case WPT_CREATE_MAP_EXIT:
	case WPT_SHOUT:
	case WPT_NAV_BEACON:
		if (ep->Data[0])
			free((void*)ep->Data[0]);
	}*/
	if (HasText(ep)) free((void*)ep->Data[0]);
	ZeroMemory(ep->Data,10*sizeof(ep->Data[0]));
}

SLONG OpenProperties(EventPoint *ep) {

	if (!ep) return 0;

	switch(ep->WaypointType) {
		case WPT_NONE:
			break;

		case WPT_SIMPLE:
			do_waypoint_setup(ep);
			break;

		case WPT_CREATE_PLAYER:
			do_player_setup(ep);
			break;

		case WPT_CREATE_ENEMIES:
			do_enemy_setup(ep);
			break;

		case WPT_ADJUST_ENEMY:
			do_enemy_setup(ep,TRUE);
			break;

		case WPT_CREATE_VEHICLE:
			do_vehicle_setup(ep);
			break;

		case WPT_CREATE_ITEM:
			do_item_setup(ep);
			break;

		case WPT_SOUND_EFFECT:
			do_sfx_setup(ep);
			break;

		case WPT_VISUAL_EFFECT:
			do_vfx_setup(ep);
			break;

		case WPT_SPOT_EFFECT:
			do_spotfx_setup(ep);
			break;

		case WPT_CUT_SCENE:
			do_cutscene_setup(ep);
			break;

		case WPT_MESSAGE:
			do_message_setup(ep);
			break;

		case WPT_CREATE_CREATURE:
			do_creature_setup(ep);
			break;
		
		case WPT_CREATE_CAMERA:
		case WPT_CAMERA_WAYPOINT:
			// essentially same properties...
			do_camera_setup(ep);
			break;

		case WPT_TELEPORT:
		case WPT_TELEPORT_TARGET:
			break;

		case WPT_CREATE_TARGET:
		case WPT_TARGET_WAYPOINT:
			do_camtarget_setup(ep);
			break;

		case WPT_CREATE_MAP_EXIT:
			do_mapexit_setup(ep);
			break;

		case WPT_SHOUT:
			if (ep) ep->Data[0]=(SLONG)InputBox("Shout Code","Enter code to be 'shouted':",(CBYTE*)ep->Data[0]);
			break;

		case WPT_NAV_BEACON:
//			if (ep) ep->Data[0]=(SLONG)InputBox("Beacon Message","Enter beacon message:",(CBYTE*)ep->Data[0]);
			do_nav_setup(ep);
			break;

		case WPT_ACTIVATE_PRIM:
			do_activate_setup(ep);
			break;

		case WPT_CREATE_TRAP:
			do_trap_setup(ep);
			break;

		case WPT_LINK_PLATFORM:
			do_platform_setup(ep);
			break;

		case WPT_CREATE_BOMB:
			do_bomb_setup(ep);
			break;

		case WPT_BURN_PRIM:
			do_burn_setup(ep);
			break;

		case WPT_CREATE_BARREL:
			do_barrel_setup(ep);
			break;

		case WPT_KILL_WAYPOINT:
			do_wpt_pick(ep);
			break;

		case WPT_CREATE_TREASURE:
			do_treasure_setup(ep);
			break;

		case WPT_BONUS_POINTS:
			do_bonus_setup(ep);
			break;

		case WPT_CONVERSATION:
			do_converse_setup(ep);
			break;

		case WPT_INCREMENT:
			do_counter_setup(ep);
			break;

		case WPT_DYNAMIC_LIGHT:
			do_lite_setup(ep);
			break;

		case WPT_GOTHERE_DOTHIS:
			do_anim_pick(ep);
			break;

		case WPT_TRANSFER_PLAYER:
			do_transfer_pick(ep);
			break;

		case WPT_LOCK_VEHICLE:
			do_lock_setup(ep);
			break;

		case WPT_RESET_COUNTER:
			do_reset_pick(ep);
			break;

		case WPT_ENEMY_FLAGS:
			do_enemy_flags_setup(ep);
			break;

		case WPT_STALL_CAR:
			do_stall_setup(ep);
			break;

		case WPT_EXTEND:
			do_extend_setup(ep);
			break;

		case WPT_MOVE_THING:
			do_move_setup(ep);
			break;

		case WPT_MAKE_PERSON_PEE:
			do_pee_setup(ep);
			break;

		case WPT_SIGN:
			do_sign_setup(ep);
			break;

		case WPT_WAREFX:
			do_warefx_setup(ep);
			break;

	}
	return 0;
}

SLONG GetEventY(EventPoint *ep, BOOL base=0) {
	if (ep->Flags&WPT_FLAGS_INSIDE) {
		if (ep->Y)
			return get_inside_alt(ep->Y);
		else
			return PAP_calc_map_height_at(ep->X, ep->Z);
	} else {
		if (base)
			return PAP_calc_map_height_at(ep->X, ep->Z);
		else
			return ep->Y;
	}
}


SLONG GetNextFloor(EventPoint *ep, SBYTE dir, UWORD *room) {
	SLONG y,base,floor=0;

	if (ep->Y) { // is already inside
		y=get_inside_alt(ep->Y);
		if (dir>0) {
			base=PAP_calc_map_height_at(ep->X,ep->Z);
//			while ((floor==ep->Y)) {
//			while ((y<base)&&(floor!=ep->Y)&&!floor) {
			while ((y<base)&&((floor==ep->Y)||!floor)) {
				y++;
				floor=calc_inside_for_xyz(ep->X,y,ep->Z,room);
			}
		} else {
			base=PAP_calc_height_at(ep->X,ep->Z);
			while ((y>base)&&!floor) {
				y--;
				floor=calc_inside_for_xyz(ep->X,y,ep->Z,room);
			}
		}
	} else { // place inside?
		if (dir>0) return 0;
		y=PAP_calc_map_height_at(ep->X,ep->Z);
		base=PAP_calc_height_at(ep->X,ep->Z);
		while ((y>base)&&!floor) {
			y--;
			floor=calc_inside_for_xyz(ep->X,y,ep->Z,room);
		}
	}
	return floor;
}


void SetMenuItemText( HMENU menu, SLONG item, CBYTE *str) {
	SLONG res;

	res=ModifyMenu(menu,item,MF_STRING|MF_BYCOMMAND,item,str);
	if (!res) {
		res=GetLastError();
		ASSERT(0);
	}
}

//---------------------------------------------------------------

CBYTE *WaypointTitle(EventPoint *ep, CBYTE *msg) {
	if (!ep->WaypointType)
		sprintf(msg,"%d",EVENTPOINT_NUMBER(current_mission->EventPoints,ep));
	else
		sprintf(msg,"%d %c: %s",EVENTPOINT_NUMBER(current_mission->EventPoints,ep),ep->Group + 'A',wtype_strings[ep->WaypointType-1]);
	return msg;
}

CBYTE *WaypointExtra(EventPoint *ep, CBYTE *msg) {

	msg[0]=0;
	if (ep) {
		switch(ep->WaypointType) {
		case WPT_SIMPLE:
			get_waypoint_message(ep,msg);
			break;
		case WPT_CREATE_ENEMIES:
		case WPT_ADJUST_ENEMY:
			get_enemy_message(ep,msg);
			break;
		case WPT_CREATE_PLAYER:
			get_player_message(ep,msg);
			break;
		case WPT_CREATE_CREATURE:
			{
				switch(ep->Data[0])
				{
					case 1: sprintf(msg, "Bat");      break;
					case 2: sprintf(msg, "Gargoyle"); break;
					case 3: sprintf(msg, "Balrog");   break;
					case 4: sprintf(msg, "Bane");     break;
				}
			}
			break;
		case WPT_CREATE_CAMERA:
		case WPT_CAMERA_WAYPOINT:
			get_camera_message(ep,msg);
			break;
		case WPT_CREATE_TARGET:
		case WPT_TARGET_WAYPOINT:
			get_camtarget_message(ep,msg);
			break;
		case WPT_CREATE_MAP_EXIT:
			get_mapexit_message(ep,msg);
			break;
		case WPT_MESSAGE:
		case WPT_SHOUT:
		case WPT_NAV_BEACON:
		case WPT_BONUS_POINTS:
			if (GetEPText(ep)) strcpy(msg,GetEPText(ep));
//			get_message_message(ep,msg); // just a checked strcpy, so compatible...
			break;
		case WPT_ACTIVATE_PRIM:
			get_activate_message(ep,msg);
			break;
		case WPT_CREATE_TRAP:
			get_trap_message(ep,msg);
			break;
		case WPT_CREATE_VEHICLE:
			get_vehicle_message(ep,msg);
			break;
		case WPT_CREATE_ITEM:
			get_item_message(ep,msg);
			break;
		case WPT_SPOT_EFFECT:
			get_spotfx_message(ep,msg);
			break;
		case WPT_CREATE_TREASURE:
			get_treasure_message(ep,msg);
			break;
		case WPT_INCREMENT:
			get_counter_message(ep,msg);
			break;
		case WPT_DYNAMIC_LIGHT:
			get_lite_message(ep,msg);
			break;
		case WPT_WAREFX:
			get_warefx_message(ep,msg);
			break;
		}
	}

	return msg;
}

void WaypointCaption(EventPoint *ep) {
  CBYTE msg[300],msga[300],msgb[300];

  strcpy(msg,"Waypoint ");
  if (!ep) {
	  strcat(msg, "info:");
  } else {
	  sprintf(msg,"%s (%s)",WaypointTitle(ep,msga),WaypointExtra(ep,msgb));
  }
  SendMessage( GetDlgItem( GEDIT_edit_wnd, IDC_WAYPOINT_GROUP ),
	  WM_SETTEXT, 0, (long) msg);
}

//---------------------------------------------------------------

UBYTE GetZone(SLONG x, SLONG y) {
	if ((x<0)||(x>128)||(y<0)||(y>128)) return 0;
	return MissionZones[current_mission-mission_pool][x][y];
}

UBYTE MatchZone(SLONG x, SLONG y, UBYTE match) {
	if ((x<0)||(x>128)||(y<0)||(y>128)) return 0;
	return MissionZones[current_mission-mission_pool][x][y] & match;
}

void SetZone(SLONG x, SLONG y, UBYTE set) {
	if ((x<0)||(x>128)||(y<0)||(y>128)) return;
	MissionZones[current_mission-mission_pool][x][y]=set;
}

//---------------------------------------------------------------

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);

void UpdateDir(EventPoint *ep) {
	SLONG dx,dz,dir;

	dx=mouse_world_x-ep->X;
	dz=ep->Z-mouse_world_z;

	if (ep->TriggeredBy==TT_CUBOID) {
		ep->Radius=MAKELONG(abs(dx),abs(dz));
		return;
	}

	dir=Arctan(dx,dz)&2047;
	dir>>=3;

	ep->Direction=dir;
}

UBYTE UpdatePrimDir(SLONG px, SLONG pz) {
	SLONG dx,dz,dir;

	dx=mouse_world_x-px;
	dz=pz-mouse_world_z;

	dir=Arctan(dx,dz)&2047;
	dir>>=3;
	if (ShiftFlag) dir&=~15;

	return dir;
}

SLONG LocatePrim(SLONG current) {
	OB_Info *oi;
	SLONG best,dist,bestdist,x,z,mx,mz;

	best=-1;
	bestdist=128*128;
	mx=mouse_world_x>>PAP_SHIFT_LO;
	mz=mouse_world_z>>PAP_SHIFT_LO;
	oi=OB_find(mx, mz);
	while(oi->prim)
	{
//		if (oi->index==current) return current;
		x=mouse_world_x - oi->x;
		z=mouse_world_z - oi->z;
		dist=SDIST2(x,z);
		if (dist<bestdist) {
			best=oi->index;
			bestdist=dist;
			if (current!=-2) {
				prim_x=oi->x;
				prim_z=oi->z;
			}
		}
		oi++;
	}
	return best;

}

LRESULT	CALLBACK	map_view_proc	(
										HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam
									)
{
	EventPoint		*new_event;
	HDC				hdc;
	HMENU			mv_menu;
	HRESULT			result;
	PAINTSTRUCT		ps;
	POINT			client_pos;
	RECT			dst,
					src;
	static UBYTE	dir_setting=0;
	static UBYTE	zone_state=0;
	static UBYTE	zone_mask=0;


	switch(message)
	{
		case	WM_CREATE:
			map_valid	=	0;

			//	Clear the map view.
			the_display.SetUserColour(0x00, 0x80, 0x80);
			the_display.SetUserBackground();
			the_display.ClearViewport();
			return	0;

		case	WM_LBUTTONDOWN:
			if (edit_mode==2) {
				SLONG last;

				last=prim_index;
				prim_index=LocatePrim(prim_index);

				if (prim_index==-1) {
					UBYTE flag;
					if (last!=-1) return 0;

					last=OB_ob_upto;

					flag=0;
//					flag=(!prim_height) ? OB_FLAG_ON_FLOOR : 0;

					INDOORS_INDEX=calc_inside_for_xyz(mouse_world_x, (mouse_world_y + prim_height), mouse_world_z,&INDOORS_ROOM);
					if (INDOORS_INDEX) 
						INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
					else INDOORS_DBUILDING=0;

					flag |= (AENG_transparent_warehouses) ? OB_FLAG_WAREHOUSE : 0;	// This flag is set properly by WARE_init() but set it all the time in here so the obs will be drawn

					OB_create( mouse_world_x, mouse_world_y + prim_height, mouse_world_z,
								prim_dir<<3,0,0,
								prim_num,
								flag ,INDOORS_INDEX,INDOORS_ROOM);
					if (OB_ob_upto!=last) { // success
						prim_index=OB_ob_upto-1;
						prim_x=mouse_world_x;
						prim_z=mouse_world_z;
					}
				} else {
					prim_drag=1;
					prim_height=OB_ob[prim_index].y-mouse_world_y;
					prim_num=OB_ob[prim_index].prim;
					INDOORS_INDEX=calc_inside_for_xyz(mouse_world_x, (mouse_world_y + prim_height), mouse_world_z,&INDOORS_ROOM);
					if (INDOORS_INDEX) 
						INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
					else INDOORS_DBUILDING=0;
				}
				return 0;
			}
			if (edit_mode==1) {
				SLONG mask;
				mask = SendMessage(GetDlgItem(GEDIT_edit_wnd,IDC_COMBO1),CB_GETCURSEL,0,0);
				zone_mask = 1<<mask;
				mask=GetZone(mouse_world_x>>8,mouse_world_z>>8) ^ zone_mask;
				SetZone(mouse_world_x>>8,mouse_world_z>>8,mask);
				zone_state=mask & zone_mask;
				return 0;
			} 
			if ((link_mode>0)&&(link_mode<3)) {
				if (map_valid && mouse_valid && hilited_ep) {
					if (selected_ep) {
						if (link_mode==1)
							selected_ep->EPRef=(hilited_ep-current_mission->EventPoints);
						else
							selected_ep->EPRefBool=(hilited_ep-current_mission->EventPoints);
						ep_to_controls2(selected_ep);
					}
				}
				link_mode=0;
				return 0;
			}
			if(map_valid && mouse_valid)
			{
				if(hilited_ep)
				{
					//	Select the event point.
					selected_ep	=	hilited_ep;
					ep_to_controls2(selected_ep);
					leaping_disabled=1;
					ws_sel_waypoint(selected_ep);
					leaping_disabled=0;

					//	Set up for dragging.
					dragging_ep	=	TRUE;
					SetCapture(hWnd);
				}
				else
				{
					//	Create a new event point.
					new_event	=	alloc_eventpoint();
					if(new_event)
					{

						if ((link_mode==3)&&(selected_ep)) {
							new_event->AfterTimer=selected_ep->AfterTimer;
							new_event->Colour=selected_ep->Colour;
							new_event->Direction=selected_ep->Direction;
							new_event->EPRef=selected_ep->EPRef;
							new_event->EPRefBool=selected_ep->EPRefBool;
							new_event->Group=selected_ep->Group;
							new_event->OnTrigger=selected_ep->OnTrigger;
							//new_event->Radius=selected_ep->Radius;
							new_event->Radius=0;
							new_event->TriggeredBy=selected_ep->TriggeredBy;
							new_event->WaypointType=selected_ep->WaypointType;
							memcpy(new_event->Data,selected_ep->Data,10*4);
							switch(new_event->WaypointType) {
							case WPT_MESSAGE:
							case WPT_CREATE_MAP_EXIT:
							case WPT_SHOUT:
							case WPT_NAV_BEACON:
							case WPT_BONUS_POINTS:
							case WPT_CONVERSATION:
								new_event->Data[0]=0;	// (sharing message points baaaaad)
								break;
							}

							link_mode=0;
						} else {
							//  init the stuff not visible
							new_event->OnTrigger=OT_ACTIVE;
							new_event->WaypointType=WPT_SIMPLE;
							controls_to_ep2(new_event);
						}

						//	Set position.
						new_event->X	=	mouse_world_x;
						new_event->Y	=	mouse_world_y;
						new_event->Z	=	mouse_world_z;


/*
						//	Set its other bits.
						controls_to_ep2(new_event);

						leaping_disabled=1;
						ws_add_waypoint(selected_ep);
						ws_sel_waypoint(selected_ep);
						leaping_disabled=0;
*/
						//	Select new epoint.
						selected_ep	=	new_event;

						leaping_disabled=1;
						ws_add_waypoint(selected_ep);
						if ((link_mode==3)&&(selected_ep)) controls_to_ep2(new_event);
						ws_sel_waypoint(selected_ep);
						leaping_disabled=0;

						if (new_event->WaypointType>WPT_SIMPLE) OpenProperties(new_event);

						ep_to_controls2(new_event);

//						WaypointCaption(new_event); // update to reflect settings

						workspace_changed	=	TRUE;
					}
					return	0;
				}
			}
			break;

		case	WM_LBUTTONUP:
			zone_mask=0;
			prim_drag=0;
			if(dragging_ep)
			{
				ReleaseCapture();
				dragging_ep	=	FALSE;
				return	0;
			}
			break;

		case	WM_LBUTTONDBLCLK:
			if(hilited_ep)
			{
				OpenProperties(hilited_ep);
				ep_to_controls2(selected_ep); 
				return 0;
			}
			break;

		case	WM_MBUTTONDOWN:
			switch(edit_mode) {
			case 0:
				if (selected_ep) {
					UpdateDir(selected_ep);
					dir_setting=1;
				}
				break;
			case 2:
				if (prim_index!=-1) {
					OB_ob[prim_index].yaw=prim_dir=UpdatePrimDir(prim_x,prim_z);
					dir_setting=1;
				}
			}
			break;

		case	WM_MBUTTONUP:
			dir_setting=0;
			break;

		case	WM_RBUTTONDOWN:
			switch (edit_mode) {
			case 0:
			case 1: // heh
				if(link_mode>0) {
					link_mode=0;
					link_start_ep=NULL;
				}
				if(selected_ep&&!hilited_ep)
				{
					WaypointCaption(0);
					selected_ep	=	NULL;
					ws_sel_waypoint(NULL);
					return	0;
				}
				break;
			case 2:
				if ((prim_index!=-1)&&!prim_drag) {
					OB_Info oi;

					oi.x=prim_x; oi.z=prim_z;
					oi.index=prim_index;

					if (prim_psxmode) {
						OB_ob[prim_index].flags^=OB_FLAG_NOT_ON_PSX;
					} else {
						OB_remove(&oi);
						prim_index=-1;
						prim_height=0;
					}
				}
				break;
			}
			break;

		case	WM_CONTEXTMENU:
			if(hilited_ep)
			{
				SLONG check;
				//	Bring up the context menu.
				mv_menu	=	GetSubMenu(LoadMenu(GEDIT_hinstance,MAKEINTRESOURCE(IDR_GEDIT_POPUPS)),3);
				
				// let's just be extra windowsy about it... heh. :P
				if (HasProperties(hilited_ep))
					SetMenuDefaultItem( mv_menu, ID_EVENTPOINTROOT_PROPERTIES, 0); 
				else
					EnableMenuItem( mv_menu, ID_EVENTPOINTROOT_PROPERTIES, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED );

				check = ((hilited_ep->Flags&WPT_FLAGS_INSIDE) ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND;
				CheckMenuItem(mv_menu,ID_EVENTPOINTROOT_INSIDE, check);
				check = ((hilited_ep->Flags&WPT_FLAGS_WARE) ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND;
				CheckMenuItem(mv_menu,ID_EVENTPOINTROOT_WARE, check);
				check = ((hilited_ep->Flags&(WPT_FLAGS_INSIDE|WPT_FLAGS_WARE)) ? MF_UNCHECKED : MF_CHECKED) | MF_BYCOMMAND;
				CheckMenuItem(mv_menu,ID_EVENTPOINTROOT_NORMAL, check);

				CBYTE magicnum[40];

				sprintf(magicnum,"Magic number: %d",hilited_ep->Data[9]);

				ModifyMenu(mv_menu,ID_EVENTPOINTROOT_MAGICNUMBER,MF_BYCOMMAND|MF_STRING,
					ID_EVENTPOINTROOT_MAGICNUMBER, magicnum);

				if (link_start_ep) {
					SetMenuItemText( GetSubMenu(mv_menu,2), ID_EVENTPOINTROOT_DEPENDENCY_LINK, "End Link");
					EnableMenuItem( GetSubMenu(mv_menu,2), ID_EVENTPOINTROOT_DEPENDENCY_CANCEL, MF_BYCOMMAND|MF_ENABLED);
				}

				TrackPopupMenu	(
									mv_menu,
									TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
									LOWORD(lParam),HIWORD(lParam),
									0,GEDIT_view_wnd,NULL
								);
				DestroyMenu(mv_menu);
				
				workspace_changed	=	TRUE;

				return	0;
			}
			break;

		case	WM_MOUSEMOVE:
			if (edit_mode==2) {
				if (dir_setting&&(prim_index!=-1)) {
					OB_ob[prim_index].yaw=prim_dir=UpdatePrimDir(prim_x,prim_z);
				}
				if (prim_drag) {
					OB_Info oi;
					SLONG last;
					static SLONG drx,dry,drz;
					UBYTE flag;

					if (prim_drag==1) {
						prim_drag++;
						drx=mouse_world_x;
						drz=mouse_world_z;
						dry=mouse_world_y;
					}

//					oi.x=(OB_ob[prim_index].x << 2) + ( some_x << 10);
//					oi.z=(OB_ob[prim_index].z << 2) + ( some_z << 10);

					oi.x=drx; oi.z=drz;

					prim_height=OB_ob[prim_index].y-mouse_world_y;
					prim_num=OB_ob[prim_index].prim;
					prim_dir=OB_ob[prim_index].yaw;

					oi.index=prim_index;
					OB_remove(&oi);

					last=OB_ob_upto;

					flag=0;
//					flag=(!prim_height) ? OB_FLAG_ON_FLOOR : 0;

//					if (ControlFlag) mouse_world_y=dry;

					INDOORS_INDEX=calc_inside_for_xyz(mouse_world_x, (mouse_world_y + prim_height), mouse_world_z,&INDOORS_ROOM);
					if (INDOORS_INDEX) 
						INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
					else INDOORS_DBUILDING=0;

					drx=mouse_world_x; drz=mouse_world_z; dry=mouse_world_y;

					{
						static SLONG last_drx;
						static SLONG last_drz;

						if (ShiftFlag)
						{
							drx&=0xFF80;
							drz&=0xFF80;

							if (last_drx != drx ||
								last_drz != drz)
							{
								CONSOLE_text("Snap to half mapsquares");

								last_drx = drx;
								last_drz = drz;
							}

						}

						if (ControlFlag)
						{

							drx&=0xFF00;
							drz&=0xFF00;

							drx|=0x0080;
							drz|=0x0080;

							if (last_drx != drx ||
								last_drz != drz)
							{
								CONSOLE_text("Snap to whole mapsquares");

								last_drx = drx;
								last_drz = drz;
							}
						}
					}

					// it's all arse.
					flag |= (AENG_transparent_warehouses) ? OB_FLAG_WAREHOUSE : 0;	// This flag is set properly by WARE_init() but set it all the time in here so the obs will be drawn


//					flag |= OB_FLAG_WAREHOUSE;	// This flag is set properly by WARE_init() but set it all the time in here so the obs will be drawn

/*					OB_create( mouse_world_x, mouse_world_y + prim_height, mouse_world_z,
								prim_dir<<3,0,0,
								prim_num,
								flag ,0,0);*/
					OB_create( drx, mouse_world_y + prim_height, drz,
								prim_dir<<3,0,0,
								prim_num,
								flag ,INDOORS_INDEX,INDOORS_ROOM);
					if (OB_ob_upto!=last) { // success
						prim_index=OB_ob_upto-1;
						prim_x=mouse_world_x;
						prim_z=mouse_world_z;
					}

				}
				return 0;
			}
			if ((edit_mode==1)&&zone_mask) {
				SLONG temp;
				temp=GetZone(mouse_world_x>>8,mouse_world_z>>8);
				temp&=~zone_mask;
				temp|=zone_state;
				SetZone(mouse_world_x>>8,mouse_world_z>>8,temp);
			}
			if(dir_setting&&selected_ep) UpdateDir(selected_ep);
			if(dragging_ep && selected_ep)
			{
//				SLONG height = selected_ep->Y-PAP_calc_map_height_at(selected_ep->X, selected_ep->Z);
				SLONG lastY  = GetEventY(selected_ep);
				SLONG height = lastY-PAP_calc_map_height_at(selected_ep->X, selected_ep->Z);

				if (selected_ep->Flags&WPT_FLAGS_INSIDE) {
					INDOORS_INDEX=calc_inside_for_xyz(mouse_world_x, lastY+128, mouse_world_z,&INDOORS_ROOM);
					if (!INDOORS_INDEX)
					  INDOORS_INDEX=calc_inside_for_xyz(mouse_world_x, mouse_world_y-128, mouse_world_z,&INDOORS_ROOM);
					if (!INDOORS_INDEX) {// still...
						mouse_world_y = PAP_calc_map_height_at(mouse_world_x, mouse_world_z);
						height=0;
					}
				} else 
					INDOORS_INDEX=0;

				//	Set new position.
				selected_ep->X	=	mouse_world_x;
				if (!(selected_ep->Flags&WPT_FLAGS_INSIDE)) selected_ep->Y	=	mouse_world_y+height;
				selected_ep->Z	=	mouse_world_z;

				if (INDOORS_INDEX) {
//					selected_ep->Y = get_inside_alt(INDOORS_INDEX);
					selected_ep->Y = INDOORS_INDEX;
					INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
				} else INDOORS_DBUILDING=0;


				if (ShiftFlag) {
					selected_ep->X&=0xFF00;
					selected_ep->Z&=0xFF00;
				}

				workspace_changed	=	TRUE;

				return	0;
			}
			break;

		case	WM_KEYDOWN:
		case	WM_KEYUP:
			KeyboardProc(message,wParam,lParam);
			return 0;

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

			//	Blit the view.
			result	=	the_display.lp_DD_FrontSurface->Blt(&dst,the_display.lp_DD_BackSurface,&src,DDBLT_WAIT,0);

			EndPaint(hWnd,&ps);
			return	0;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	ID_EVENTPOINTROOT_PROPERTIES:
					if (hilited_ep) {
						EventPoint *remember_me=hilited_ep;
						OpenProperties(hilited_ep);
						if (selected_ep==remember_me) WaypointCaption(selected_ep);
					}
					break;

/*				case	ID_EVENTPOINTROOT_INSIDE:
					hilited_ep->Flags^=WPT_FLAGS_INSIDE;
					if (hilited_ep->Flags&WPT_FLAGS_INSIDE) {
						hilited_ep->Y=INDOORS_INDEX=calc_inside_for_xyz(hilited_ep->X, PAP_calc_map_height_at(hilited_ep->X, hilited_ep->Z)-128, hilited_ep->Z,&INDOORS_ROOM);
					} else {
						hilited_ep->Y=GetEventY(hilited_ep,1);
						INDOORS_INDEX=INDOORS_ROOM=0;
					}
					if (INDOORS_INDEX)
						INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
					else INDOORS_DBUILDING=0;
					break;*/
				case	ID_EVENTPOINTROOT_NORMAL:
					hilited_ep->Flags&=~(WPT_FLAGS_INSIDE|WPT_FLAGS_WARE);
					hilited_ep->Y=GetEventY(hilited_ep,1);
					INDOORS_INDEX=INDOORS_ROOM=0;
					INDOORS_DBUILDING=0;
					AENG_transparent_warehouses=0;
extern void MAV_calc_height_array(SLONG ignore_warehouses);
					MAV_calc_height_array(0);
					break;
				case	ID_EVENTPOINTROOT_INSIDE:
					hilited_ep->Flags&=~WPT_FLAGS_WARE;
					hilited_ep->Flags|=WPT_FLAGS_INSIDE;
					hilited_ep->Y=INDOORS_INDEX=calc_inside_for_xyz(hilited_ep->X, PAP_calc_map_height_at(hilited_ep->X, hilited_ep->Z)-128, hilited_ep->Z,&INDOORS_ROOM);
					if (INDOORS_INDEX)
						INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
					else INDOORS_DBUILDING=0;
					AENG_transparent_warehouses=0;
					MAV_calc_height_array(FALSE);
					break;
				case	ID_EVENTPOINTROOT_WARE:
					hilited_ep->Flags&=~WPT_FLAGS_INSIDE;
					hilited_ep->Flags|=WPT_FLAGS_WARE;
					hilited_ep->Y=GetEventY(hilited_ep,1);
					INDOORS_INDEX=INDOORS_ROOM=0;
					INDOORS_DBUILDING=0;
					AENG_transparent_warehouses=1;
					MAV_calc_height_array(TRUE);
					break;

				case	ID_EVENTPOINTROOT_DEPENDENCY_LINK:
					if (link_start_ep) {
						// new one is end
						if (hilited_ep) {
						  hilited_ep->EPRef=(link_start_ep-current_mission->EventPoints);
						  if (selected_ep) ep_to_controls2(selected_ep);
						}
						link_start_ep=NULL;
						link_mode=0;
					} else {
						if (hilited_ep) link_start_ep=hilited_ep;
					}
					break;

				case	ID_EVENTPOINTROOT_DEPENDENCY_CANCEL:
					link_start_ep=NULL;
					break;

				case	ID_EVENTPOINTROOT_COPYWAYPOINT:
					if (hilited_ep&&(hilited_ep!=selected_ep)) {
						// The Mark Memorial Feature(tm)
						selected_ep	= hilited_ep;
						ep_to_controls2(selected_ep);
						leaping_disabled=1;
						ws_sel_waypoint(selected_ep);
						leaping_disabled=0;
					}
					link_mode=3;
					break;

				case	ID_EVENTPOINTROOT_DELETEWAYPOINT:
					if (hilited_ep) {
						if (selected_ep==hilited_ep) selected_ep=NULL;
						ws_del_waypoint(hilited_ep);
						free_eventpoint(hilited_ep);
						if (hilited_ep==link_start_ep) link_start_ep=NULL;
						hilited_ep	=	NULL;
					}
					break;

				case ID_EVENTPOINTROOT_RENUMBERWAYPOINTLOWER:

					if (hilited_ep)
					{
						SLONG i;

						EventPoint *ep;
						EventPoint *lower = NULL;

						//
						// Look for a lower numbered waypoint.
						//

						for (i = 1; i < MAX_EVENTPOINTS; i++)
						{
							ep = &current_mission->EventPoints[i];

							if (ep >= hilited_ep)
							{
								break;
							}

							if (!ep->Used)
							{
								lower = ep;

								break;
							}
						}

						if (lower)
						{
							MessageBox(NULL,"Select OK to renumber the waypoint", "Hello Simon", MB_ABORTRETRYIGNORE|MB_ICONQUESTION);
						}
					}

					break;
			}
			break;
	}
	return	DefWindowProc(hWnd,message,wParam,lParam);
}

//---------------------------------------------------------------

BOOL	init_map_view(void)
{
	WNDCLASSEX		new_class;


	//	Create map view window class.
	new_class.cbSize		=	sizeof(WNDCLASSEX);
	new_class.style			=	CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
	new_class.lpfnWndProc	=	map_view_proc;
	new_class.cbClsExtra	=	0;
	new_class.cbWndExtra	=	sizeof(HANDLE);
	new_class.hInstance		=	GEDIT_hinstance;
	new_class.hIcon			=	NULL;
	new_class.hCursor		=	GEDIT_arrow;
	new_class.hbrBackground	=	(struct HBRUSH__ *)GetStockObject(LTGRAY_BRUSH);
	new_class.lpszMenuName	=	NULL;
	new_class.lpszClassName	=	GEDIT_map_name;
	new_class.hIconSm		=	NULL;
	if(!RegisterClassEx(&new_class))
		return	FALSE;		//	Couldn't register the class.

	hilited_ep	  =	NULL;
	selected_ep	  =	NULL;
	link_start_ep =	NULL;

	return	TRUE;
}

//---------------------------------------------------------------


#define	SCROLL_RATE		4
#define	ZOOM_RATE		4
#define	YAW_RATE		1
#define	PITCH_RATE		1

void AENG_world_text(
		SLONG  x,
		SLONG  y,
		SLONG  z,
		UBYTE  red,
		UBYTE  blue,
		UBYTE  green,
		UBYTE  shadowed_or_not,
		CBYTE *fmt, ...);
void AENG_world_line(
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front);


void AENG_world_line_alpha(
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front)
{
	POLY_Point p1;
	POLY_Point p2;

	POLY_transform(float(x1), float(y1), float(z1), &p1);
	POLY_transform(float(x2), float(y2), float(z2), &p2);

	if (POLY_valid_line(&p1, &p2))
	{
		p1.colour   = colour1;
		p1.specular = 0xff000000;

		p2.colour   = colour2;
		p2.specular = 0xff000000;

		POLY_add_line(&p1, &p2, float(width1), float(width2), POLY_PAGE_ALPHA, sort_to_front);
	}
}


void FONT_buffer_draw(void);

void world_cube(SLONG cx, SLONG cy, SLONG cz, SLONG ox, SLONG oy, SLONG oz, SLONG colour) {
	// base
	AENG_world_line(
		cx-ox, cy, cz-oz, 8, colour,
		cx+ox, cy, cz-oz, 8, colour, 0);
	AENG_world_line(
		cx-ox, cy, cz+oz, 8, colour,
		cx+ox, cy, cz+oz, 8, colour, 0);
	AENG_world_line(
		cx-ox, cy, cz-oz, 8, colour,
		cx-ox, cy, cz+oz, 8, colour, 0);
	AENG_world_line(
		cx+ox, cy, cz-oz, 8, colour,
		cx+ox, cy, cz+oz, 8, colour, 0);
	// top
	AENG_world_line(
		cx-ox, cy+oy, cz-oz, 8, colour,
		cx+ox, cy+oy, cz-oz, 8, colour, 0);
	AENG_world_line(
		cx-ox, cy+oy, cz+oz, 8, colour,
		cx+ox, cy+oy, cz+oz, 8, colour, 0);
	AENG_world_line(
		cx-ox, cy+oy, cz-oz, 8, colour,
		cx-ox, cy+oy, cz+oz, 8, colour, 0);
	AENG_world_line(
		cx+ox, cy+oy, cz-oz, 8, colour,
		cx+ox, cy+oy, cz+oz, 8, colour, 0);
    // joints
	AENG_world_line(
		cx+ox, cy   , cz+oz, 8, colour,
		cx+ox, cy+oy, cz+oz, 8, colour, 0);
	AENG_world_line(
		cx-ox, cy   , cz+oz, 8, colour,
		cx-ox, cy+oy, cz+oz, 8, colour, 0);
	AENG_world_line(
		cx+ox, cy   , cz-oz, 8, colour,
		cx+ox, cy+oy, cz-oz, 8, colour, 0);
	AENG_world_line(
		cx-ox, cy   , cz-oz, 8, colour,
		cx-ox, cy+oy, cz-oz, 8, colour, 0);
}

void	process_view_wind(void)
{
	UBYTE			hilite,
					no_hilite	=	1,
					on_epoint	=	0;
	ULONG			colour,
					current_ep;
	SLONG			df,dl,dy,dp,dd,
					dist,dx,dz;
	EventPoint		*ep_ptr,
					*targ,
					*ep_base;
	POINT			mouse;
	RECT			client_rect;
	UBYTE			lines_mode, zones_mode;
	static SLONG	turn	=	0;
	SLONG			x,z,h;

	lines_mode = SendMessage(GetDlgItem(GEDIT_edit_wnd,IDC_COMBO4),CB_GETCURSEL,0,0);
	zones_mode = SendMessage(GetDlgItem(GEDIT_edit_wnd,IDC_COMBO1),CB_GETCURSEL,0,0);

	turn	+=	1;
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

		if (Keys[KB_F11]) {
			Keys[KB_F11]=0;
			if (aeng_draw_cloud_flag) {
				aeng_draw_cloud_flag=0;
				CONSOLE_text("clouds off");
			} else {
				aeng_draw_cloud_flag=1;
				CONSOLE_text("clouds on");
			}
		}

		//--- prim edit keys ---//
		if (edit_mode==2) {
			if (Keys[KB_SPACE]) {
				Keys[KB_SPACE]=0;
				if (prim_index!=-1) {
					prim_num=OB_ob[prim_index].prim;
				} else {
					SLONG temp;
					temp=LocatePrim(-2);
					if (temp!=-1) prim_num=OB_ob[temp].prim;
				}
			}
			if (Keys[KB_PPLUS]) {
				Keys[KB_PPLUS]=0;
				prim_num++;
			}
			if (Keys[KB_PMINUS]) {
				Keys[KB_PMINUS]=0;
				prim_num--;
			}
		}
		//----------------------//

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
		ScreenToClient(GEDIT_view_wnd,&mouse);
		GetClientRect(GEDIT_view_wnd,&client_rect);

		// This is _not_ efficient.
		// But not a buttload of choice without hacking editor stuff into the main game code :P
		if (edit_mode==1) {
			POLY_frame_init(FALSE,FALSE);
			for (x=0;x<128;x++)
				for (z=0;z<128;z++) {
//					PAP_Hi here = PAP_2HI(x,z);
					SLONG y;
					UBYTE mousein=((x==mouse_world_x>>8)&&(z==mouse_world_z>>8));
//					if ((here.Flags & PAP_FLAG_NAUGHTY_SQUARE)||mousein) {
//					if (IsZoned(x,z)||mousein) {
					if (MatchZone(x,z,1<<zones_mode)||mousein) {
						y=MAVHEIGHT(x,z)<<6;
						if (mousein)
							GI_groundsquare_draw(x<<8,y+0x40,z<<8,zone_colours[zones_mode]|(0xFF<<24),0);
						else
							GI_groundsquare_draw(x<<8,y+0x40,z<<8,zone_colours[zones_mode]|(0x7F<<24),0);
					}
				}
			POLY_frame_draw(FALSE,FALSE);
		}

		// experimental prim nonsense
		if ((edit_mode==2)&&(prim_index==-1))
		{
			POLY_frame_init(FALSE,FALSE);
			MESH_draw_poly(
				prim_num,
				mouse_world_x,
				mouse_world_y + prim_height,
				mouse_world_z,
				prim_dir<<3,
				0,
				0,
				NULL,0);

			INDOORS_INDEX=calc_inside_for_xyz(mouse_world_x, (mouse_world_y + prim_height), mouse_world_z,&INDOORS_ROOM);
			if (INDOORS_INDEX) 
				INDOORS_DBUILDING=inside_storeys[INDOORS_INDEX].Building;
			else INDOORS_DBUILDING=0;

			POLY_frame_draw(FALSE,FALSE);

		}

		//	Draw all waypoints within a certain distance of the camera.
		if(current_mission&&(edit_mode!=2))
		{
//			hilited_ep		=	NULL;
			ep_base		=	current_mission->EventPoints;
			current_ep	=	current_mission->UsedEPoints;
			while(current_ep)
			{
				ep_ptr	=	TO_EVENTPOINT(ep_base,current_ep);

				dx		=	ep_ptr->X	-	cam_x;
				dz		=	ep_ptr->Z	-	cam_z;
				dist	=	QDIST2(abs(dx),abs(dz));

				if(dist < (20 << 8))
				{
					if(ep_ptr==hilited_ep)
						hilite	=	255;
					else if(ep_ptr==selected_ep)
						hilite	=	abs((SIN((turn<<6)&2047)*255)>>16);
					else
						hilite	=	0;

					//	Draw it.
					colour	=	(button_colours[ep_ptr->Colour][0]<<16)	|
								(button_colours[ep_ptr->Colour][1]<<8)	|
								(button_colours[ep_ptr->Colour][2]);

					on_epoint	=	(unsigned char)GI_waypoint_draw	(
															mouse.x,
															mouse.y,
															ep_ptr->X,
															GetEventY(ep_ptr,1),
															ep_ptr->Z,
															colour,
															hilite
														);
					if (ep_ptr->TriggeredBy==TT_CUBOID) {
						SLONG cx=ep_ptr->X, cz=ep_ptr->Z, cy=PAP_calc_map_height_at(cx,cz);
//						SLONG ox=LOWORD(ep_ptr->Radius),oy=ep_ptr->Y-cy,oz=HIWORD(ep_ptr->Radius);
						SLONG ox=LOWORD(ep_ptr->Radius),oy=GetEventY(ep_ptr)-cy,oz=HIWORD(ep_ptr->Radius);
						POLY_frame_init(FALSE,FALSE);
						   world_cube(cx,cy,cz,ox,oy,oz,colour);
						POLY_frame_draw(FALSE,FALSE);

					} else {
						if (GetEventY(ep_ptr)>(h=PAP_calc_map_height_at(ep_ptr->X, ep_ptr->Z))) {
							POLY_frame_init(FALSE,FALSE);
								AENG_world_line(
									ep_ptr->X, GetEventY(ep_ptr), ep_ptr->Z, 8, colour,
									ep_ptr->X, h, ep_ptr->Z, 8, colour, 0);
							POLY_frame_draw(FALSE,FALSE);
						}
					}

					if(ep_ptr->WaypointType<=WPT_SIMPLE)
						AENG_world_text	(
											ep_ptr->X,
											GetEventY(ep_ptr) + 0x80,
											ep_ptr->Z,
											0xff,
											0xff,
											0xff,
											0,
											"%d %c",EVENTPOINT_NUMBER(ep_base,ep_ptr),'A' + ep_ptr->Group
										);
					else {
						CBYTE msg[300],msgb[300];
						WaypointTitle(ep_ptr,msg);
						if (ep_ptr->WaypointType==WPT_ADJUST_ENEMY)
						{
							sprintf(msgb," (%d)",ep_ptr->Data[6]);
							strcat(msg,msgb);
						}
						strcat(msg,"\n");
						strcat(msg,WaypointExtra(ep_ptr,msgb));
						AENG_world_text	(
											ep_ptr->X,
											GetEventY(ep_ptr) + 0x80,
											ep_ptr->Z,
											0xff,
											0xff,
											0xff,
											0,
											"%s",msg
										);
					}

//					if(ep_ptr->TriggeredBy==TT_RADIUS)
					if (WaypointUses[ep_ptr->TriggeredBy] & WPU_RADIUS) 
					{
						GI_rad_trigger_draw	(
												mouse.x,
												mouse.y,
												ep_ptr->X,
//												ep_ptr->Y + 0x100,
												GetEventY(ep_ptr),
												ep_ptr->Z,
												ep_ptr->Radius,
												colour,
												hilite
											);
					}

					if(on_epoint)
					{
						hilited_ep	=	ep_ptr;
						no_hilite	=	0;
					}

					// draw the dependency lines
//					if ((ep_ptr->TriggeredBy==TT_DEPENDENCY)||(ep_ptr->TriggeredBy==TT_BOOLEANAND)||(ep_ptr->TriggeredBy==TT_BOOLEANOR)||(ep_ptr->TriggeredBy==TT_KILLED)||(ep_ptr->TriggeredBy==TT_COUNTDOWN)) {
					if (WaypointUses[ep_ptr->TriggeredBy] & WPU_DEPEND) {
						targ=current_mission->EventPoints+ep_ptr->EPRef;
						if ((!lines_mode)
						 ||((lines_mode==1)&&((ep_ptr==selected_ep)||(ep_ptr==hilited_ep)))
						 ||((lines_mode==2)&&((ep_ptr==selected_ep)||(ep_ptr==hilited_ep)||(targ==hilited_ep)||(targ==selected_ep)))
						 ) {
							if (ep_ptr->EPRef) {
								SLONG colour2	=	(button_colours[targ->Colour][0]<<16)	|
													(button_colours[targ->Colour][1]<<8)	|
													(button_colours[targ->Colour][2]);
								if (targ->Used) {
									POLY_frame_init(FALSE, FALSE);
									AENG_world_line(
										ep_ptr->X, GetEventY(ep_ptr,1)+0x10, ep_ptr->Z, 10, colour,
										targ->X, GetEventY(targ,1)+0x10, targ->Z, 2, colour2, 0);
									POLY_frame_draw(FALSE, FALSE);
								} else {
									ep_ptr->EPRef=0;
									ep_to_controls2(selected_ep);
								}
							}
						}
					}
					
					// and the boolean lines where appropriate
//					if ((ep_ptr->TriggeredBy==TT_BOOLEANAND)||(ep_ptr->TriggeredBy==TT_BOOLEANOR)||(ep_ptr->TriggeredBy==TT_KILLED)||(ep_ptr->TriggeredBy==TT_COUNTDOWN)) {
					if (WaypointUses[ep_ptr->TriggeredBy] & WPU_BOOLEAN) {
						targ=current_mission->EventPoints+ep_ptr->EPRefBool;
						if ((!lines_mode)
						 ||((lines_mode==1)&&((ep_ptr==selected_ep)||(ep_ptr==hilited_ep)))
						 ||((lines_mode==2)&&((ep_ptr==selected_ep)||(ep_ptr==hilited_ep)||(targ==hilited_ep)||(targ==selected_ep)))
						 ) {
							if (ep_ptr->EPRefBool) {
								SLONG colour2	=	(button_colours[targ->Colour][0]<<16)	|
													(button_colours[targ->Colour][1]<<8)	|
													(button_colours[targ->Colour][2]);
								if (targ->Used) {
									POLY_frame_init(FALSE, FALSE);
									AENG_world_line(
										ep_ptr->X, GetEventY(ep_ptr,1)+0x10, ep_ptr->Z, 10, colour,
										targ->X, GetEventY(targ,1)+0x10, targ->Z, 2, colour2, 0);
									POLY_frame_draw(FALSE, FALSE);
								} else {
									ep_ptr->EPRefBool=0;
									ep_to_controls2(selected_ep);
								}
							}
						}
					}

					// direction
					if (ep_ptr->WaypointType == WPT_CREATE_PLAYER   ||
						ep_ptr->WaypointType == WPT_CREATE_ENEMIES  ||
						ep_ptr->WaypointType == WPT_CREATE_VEHICLE	||
						ep_ptr->WaypointType == WPT_CREATE_CREATURE	||
						ep_ptr->WaypointType == WPT_CREATE_CAMERA   ||
						ep_ptr->WaypointType == WPT_CAMERA_WAYPOINT ||
						ep_ptr->WaypointType == WPT_CREATE_ITEM		||
						ep_ptr->WaypointType == WPT_SIMPLE			||
						ep_ptr->WaypointType == WPT_GOTHERE_DOTHIS	||
						ep_ptr->WaypointType == WPT_CREATE_TRAP		||
						ep_ptr->TriggeredBy  == TT_THING_RADIUS_DIR ||
						ep_ptr->TriggeredBy  == TT_MOVE_RADIUS_DIR  ||
						ep_ptr->WaypointType == WPT_MOVE_THING)
					{
						SLONG dir, dirx, dirz;

						dir=ep_ptr->Direction;
						dir<<=3;
						dirx = SIN(dir)/512; dirz=COS(dir)/512;
						dirx+=ep_ptr->X; dirz+=ep_ptr->Z;

						POLY_frame_init(FALSE, FALSE);
						AENG_world_line_alpha(
							ep_ptr->X,GetEventY(ep_ptr,1)+0x10, ep_ptr->Z, 00, 0xFFFFFF00,
							dirx,	  GetEventY(ep_ptr,1)+0x10, dirz, 16, 0x00FFFF00, 0);
						POLY_frame_draw(FALSE, FALSE);
					}


					// camera "view" -- faked...
					if ((ep_ptr->WaypointType==WPT_CREATE_CAMERA)||(ep_ptr->WaypointType==WPT_CAMERA_WAYPOINT)) {
						SLONG tmp = ep_ptr->Prev;
						if (tmp) {
							targ=TO_EVENTPOINT(ep_base,tmp);
							if ((targ)&&((targ->WaypointType==WPT_CREATE_TARGET)||(targ->WaypointType==WPT_TARGET_WAYPOINT))) {
								POLY_frame_init(FALSE, FALSE);
								AENG_world_line_alpha(
									ep_ptr->X, GetEventY(ep_ptr,1)+0x10, ep_ptr->Z, 1, 0xFFFFFF00,
									targ->X, GetEventY(ep_ptr,1)+0x10, targ->Z, 200, 0x00FFFF00, 0);
								POLY_frame_draw(FALSE, FALSE);
							}
						}
					}


				}
				current_ep	=	ep_ptr->Next;
			}

			if(no_hilite)
				hilited_ep	=	NULL;
/*
			//	Draw the hilited ep, if any.
			if(hilited_ep)
			{
				colour	=	(button_colours[hilited_ep->Colour][0]<<16)	|
							(button_colours[hilited_ep->Colour][1]<<8)	|
							(button_colours[hilited_ep->Colour][2]);

				GI_waypoint_draw	(
										mouse.x,
										mouse.y,
										hilited_ep->X,
										hilited_ep->Y + 0x100,
										hilited_ep->Z,
										colour,
										255
									);
			}
*/
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
														&mouse_world_z,
														INDOORS_INDEX
													);

		}
		else
		{
			mouse_valid	=	0;
		}

		if(mouse_valid && !hilited_ep)
		{
			SLONG draw_y;

			colour	=	(button_colours[waypoint_colour][0]<<16)	|
						(button_colours[waypoint_colour][1]<<8)		|
						(button_colours[waypoint_colour][2]);

			if (INDOORS_INDEX)
				draw_y=get_inside_alt(INDOORS_INDEX);
			else
				draw_y=mouse_world_y;
			GI_waypoint_draw	(
									mouse.x,
									mouse.y,
									mouse_world_x,
									draw_y /*+ 0x100*/,
									mouse_world_z,
									colour,
									0
								);
			if (edit_mode==2) {
				SLONG test;
				CBYTE msg[800];
				msg[0]=0;
				test=LocatePrim(-2);
				if (prim_index!=-1) { // we have a selection already
					if (test==-1)
						strcpy(msg,"Deselect");
					else
						if (test==prim_index)
							strcpy(msg,"Current selection");
						else
							strcpy(msg,"Change selection");
				} else { // was no selection
					if (test!=-1)
						strcpy(msg,"Select");
					else
						itoa(prim_num,msg,10);
				}
				if (msg[0])
					AENG_world_text(
									mouse_world_x,
									mouse_world_y,
									mouse_world_z,
									0xd0, 0xff, 0xd0,
									0, msg);
			}
			if (link_mode>0) {
				CBYTE msg[300];
				switch(link_mode) {
				case 1:
					strcpy(msg,"Point to dependency...");
					break;
				case 2:
					strcpy(msg,"Point to boolean...");
					break;
				case 3:
					strcpy(msg,"Click to copy...");
				}
				AENG_world_text	(
									mouse_world_x,
									mouse_world_y,
									mouse_world_z,
									0xd0,
									0xff,
									0xd0,
									0,
									"    %s",msg
								);
			}
		}
		
		//	Draw the text.
		FONT_buffer_draw();
		CONSOLE_draw();

		InvalidateRect(GEDIT_view_wnd, NULL, FALSE);

	}
}

//---------------------------------------------------------------
