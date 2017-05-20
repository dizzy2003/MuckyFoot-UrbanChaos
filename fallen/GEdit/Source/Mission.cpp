//	Mission.cpp
//	Guy Simmons, 2nd August 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"Mission.h"
#include	"WSpace.h"
#include	"inputbox.h"
#include	"console.h"
#include	"cutscene.h"


//---------------------------------------------------------------

GameMap			game_maps[MAX_MAPS],
				*current_map;
Mission			mission_pool[MAX_MISSIONS],
				*current_mission;
CBYTE			MissionZones[MAX_MISSIONS][128][128];

extern CBYTE	map_default_dir[_MAX_PATH],
				mission_name[_MAX_PATH];
extern HWND		ws_tree;
extern EventPoint *selected_ep,*hilited_ep;

//---------------------------------------------------------------

CBYTE WaypointUses[TT_NUMBER] =
{
	0,											// none
	WPU_DEPEND,									// dependency
	WPU_RADIUS,									// radius
	0,											// door
	0,											// tripwire
	0,											// pressurepad
	0,											// leccy fency
	0,											// water level?
	0,											// securicam
	0,											// switch
	0,											// aniprim
	WPU_TIME,									// timer
	WPU_RADTEXT,								// shoutall
	WPU_DEPEND|WPU_BOOLEAN,						// booland
	WPU_DEPEND|WPU_BOOLEAN,						// boolor
	WPU_DEPEND,									// itemheld
	0,											// itemseen
	WPU_DEPEND,									// killed
	WPU_RADTEXT,								// shoutany
	WPU_DEPEND|WPU_TIME,						// countdown
	WPU_DEPEND|WPU_RADIUS,						// enemy radius
	WPU_DEPEND|WPU_TIME,						// visi countdown
	WPU_RADBOX,									// cuboid
	WPU_DEPEND,									// halfdead
	0,											// group dead
	WPU_DEPEND|WPU_BOOLEAN,						// person seen
	WPU_DEPEND,									// person used
	WPU_RADIUS,									// person 'uses' in radius
	0,											// prim damaged
	WPU_DEPEND,									// person arrested
	WPU_DEPEND,									// conversation finished
	WPU_COUNTER,								// WPU_DEPEND|WPU_BOOLEAN but indicates special meanings
	WPU_DEPEND,									// killed but not arrested
	WPU_TIME,									// crime rate, it's lying about the time, but close enuff
	WPU_TIME,									// the other crime rate...
	WPU_DEPEND,									// Person is murderer
	WPU_DEPEND|WPU_BOOLEAN,						// Person in vehicle
	WPU_DEPEND|WPU_RADIUS,						// Thing radius dir
	WPU_DEPEND,									// Player carries person
	WPU_DEPEND,									// Specific item held
	WPU_DEPEND,									// Random,
	0,											// Player fires gun
	0,											// Darci has grabbed someone
	WPU_DEPEND,									// Punched and kicked
	WPU_DEPEND|WPU_RADIUS,						// Move radius dir
};

//---------------------------------------------------------------

void	MISSION_init(void)
{
	ZeroMemory(game_maps,sizeof(game_maps));
	ZeroMemory(mission_pool,sizeof(mission_pool));
	current_map		=	NULL;
	current_mission	=	NULL;
}

//---------------------------------------------------------------

UWORD	alloc_map(void)
{
	UWORD		c0;


	for(c0=1;c0<MAX_MAPS;c0++)
	{
		if(!game_maps[c0].Used)
		{
			game_maps[c0].Used	=	TRUE;
			return	c0;
		}
	}
	return	0;
}

//---------------------------------------------------------------

void	free_map(UWORD map)
{
	UWORD		c0;


	//	Free up associated missions.
	for(c0=1;c0<MAX_MISSIONS;c0++)
	{
		if(game_maps[map].Missions[c0])
			free_mission(game_maps[map].Missions[c0]);
	}

	//	Free up the map.
	ZeroMemory(&game_maps[map],sizeof(GameMap));
}

//---------------------------------------------------------------

UWORD	alloc_mission(UWORD	map_ref)
{
	UWORD		c0,c1;


	for(c0=1;c0<MAX_MISSIONS;c0++)
	{
		//	Find a spare mission.
		if(!(mission_pool[c0].Flags & MISSION_FLAG_USED))
		{
			//	Initialise it.
			mission_pool[c0].Flags	   |=	MISSION_FLAG_USED;
			mission_pool[c0].MapIndex	=	map_ref;
			strcpy(mission_pool[c0].MapName,game_maps[map_ref].MapName);

			mission_pool[c0].CivsRate   =	4;

			//	Shove its index into the maps mission list.
			for(c1=1;c1<MAX_MISSIONS;c1++)
			{
				if(!game_maps[map_ref].Missions[c1])
				{
					game_maps[map_ref].Missions[c1]	=	c0;
					break;
				}
			}
			return	c0;
		}
	}
	return	0;
}

//---------------------------------------------------------------

void	free_mission(UWORD mission)
{
	ZeroMemory(&mission_pool[mission],sizeof(Mission));
}

//---------------------------------------------------------------

void	init_mission(UWORD mission_ref,CBYTE *mission_name)
{
	UWORD		c0;
	Mission		*the_mission;


	//	Init the mission name.
	the_mission	=	&mission_pool[mission_ref];
	strcpy(the_mission->MissionName,mission_name);
	
	//	Init the event points.
	for(c0=1;c0<MAX_EVENTPOINTS;c0++)
	{
		the_mission->EventPoints[c0].Next	=	c0+1;
		the_mission->EventPoints[c0].Prev	=	c0-1;
	}
	the_mission->EventPoints[MAX_EVENTPOINTS-1].Next	=	0;
	the_mission->FreeEPoints	=	1;
	the_mission->UsedEPoints	=	0;
	the_mission->BoredomRate	=	4;
	ZeroMemory(the_mission->SkillLevels,sizeof(the_mission->SkillLevels));
}

//---------------------------------------------------------------

void ResetFreepoint(Mission *mission) {
	EventPoint *ep_base,*the_ep;

	ep_base=mission->EventPoints;
	the_ep=ep_base;
	the_ep++; // 0 is null
	while (the_ep->Used) the_ep++;
	mission->FreeEPoints=EVENTPOINT_NUMBER(ep_base,the_ep);
}

void ResetUsedpoint(Mission *mission) {
	EventPoint *ep_base,*the_ep;

	ep_base=mission->EventPoints;
	the_ep=ep_base;
	the_ep++; // 0 is null
	while (!the_ep->Used) the_ep++;
	mission->UsedEPoints=EVENTPOINT_NUMBER(ep_base,the_ep);
}

void ResetFreelist(Mission *mission) {
	// sheesh
	SLONG prv=0, c0;
	EventPoint *last=NULL,*curr=NULL;

	curr=mission->EventPoints;
	for (c0=1;c0<MAX_EVENTPOINTS;c0++) {
		curr++;
		if (!curr->Used) {
			if (last) last->Next=c0;
			curr->Prev=prv;
			curr->Next=0;
			prv=c0;
			last=curr;
		}
	}
}

void ResetUsedlist(Mission *mission) {
	// sheesh
	SLONG prv=0, c0;
	EventPoint *last=NULL,*curr;

	curr=mission->EventPoints;
	for (c0=1;c0<MAX_EVENTPOINTS;c0++) {
		curr++;
		if (curr->Used) {
			if (last) last->Next=c0;
			curr->Prev=prv;
			curr->Next=0;
			prv=c0;
			last=curr;
		}
	}
}

void ResetLink(EventPoint *ep_base, EventPoint *the_ep, bool used) {
	EventPoint *ep_joint;
	SLONG the_point, the_joint;

	the_point=EVENTPOINT_NUMBER(ep_base,the_ep);

	ep_joint=the_ep;
	the_joint=the_point;
	while ((ep_joint->Used!=used)&&(the_joint<MAX_EVENTPOINTS)) {
		ep_joint++;		the_joint++;
	}
	// the last one?
	if (the_joint>=MAX_EVENTPOINTS) {
		the_ep->Next=0;
		ep_joint=the_ep;
		the_joint=the_point;
		while ((the_joint>0)&&(ep_joint->Used!=used)) {
			ep_joint--; the_joint--;
		}
		the_ep->Prev=the_joint;
		if (the_joint) ep_joint->Next=the_point;
		return;
	}

	the_ep->Next=the_joint;
	the_ep->Prev=ep_joint->Prev;
	if (ep_joint->Prev)
		TO_EVENTPOINT(ep_base,ep_joint->Prev)->Next=the_point;

	ep_joint->Prev=the_point;

}

void BreakLink(EventPoint *ep_base, EventPoint *the_ep) {
	if (the_ep->Prev)
		TO_EVENTPOINT(ep_base,the_ep->Prev)->Next = the_ep->Next;
	if (the_ep->Next)
		TO_EVENTPOINT(ep_base,the_ep->Next)->Prev = the_ep->Prev;
	the_ep->Prev=the_ep->Next=0;
}

//---------------------------------------------------------------

EventPoint	*alloc_eventpoint(void)
{
	UWORD			new_epoint;
	EventPoint		*ep_base,
					*the_epoint,
					*the_ep;


	if(current_mission)
	{
		//	Get a free event_point.
		new_epoint	=	current_mission->FreeEPoints;
		if(new_epoint)
		{
			ep_base						=	current_mission->EventPoints;
			the_epoint					=	TO_EVENTPOINT(ep_base,new_epoint);

			//	Clear it out.
			the_epoint->Colour			=	0;
			the_epoint->Group			=	0;
			the_epoint->WaypointType	=	0;
			the_epoint->TriggeredBy		=	0;
			the_epoint->EPRef			=	0;
			the_epoint->Radius			=	0;
			ZeroMemory(the_epoint->Data,sizeof(the_epoint->Data));

			// Remove it from the free list
			BreakLink(ep_base,the_epoint);

			// Add it to the used list
			ResetLink(ep_base,the_epoint,TRUE);


/*			//	Remove it from the free list.
			current_mission->FreeEPoints								=	TO_EVENTPOINT(ep_base,current_mission->FreeEPoints)->Next;
			TO_EVENTPOINT(ep_base,current_mission->FreeEPoints)->Prev	=	0;

			//	Add it to the used list.
			TO_EVENTPOINT(ep_base,new_epoint)->Next						=	current_mission->UsedEPoints;
			if(current_mission->UsedEPoints)
				TO_EVENTPOINT(ep_base,current_mission->UsedEPoints)->Prev	=	new_epoint;
			current_mission->UsedEPoints								=	new_epoint;
*/
			the_epoint->Used	=	TRUE;

			ResetFreepoint(current_mission);
			ResetUsedpoint(current_mission);


			return	the_epoint;
		}		
	}
	return	NULL;
  

/*  // shitty method to keep things in numerical order. ish.
	if(current_mission&&current_mission->FreeEPoints)
	{
		//	Get a free event_point.
		the_epoint  =	current_mission->EventPoints;
		while (the_epoint&&(the_epoint->Used)) the_epoint++;
		if(the_epoint)
		{
			ep_base						=	current_mission->EventPoints;
			new_epoint					=	the_epoint-ep_base;

			//	Clear it out.
			the_epoint->Colour			=	0;
			the_epoint->Group			=	0;
			the_epoint->WaypointType	=	0;
			the_epoint->TriggeredBy		=	0;
			the_epoint->EPRef			=	0;
			the_epoint->Radius			=	0;
			ZeroMemory(the_epoint->Data,sizeof(the_epoint->Data));

			//	Remove it from the free list.
			current_mission->FreeEPoints								=	TO_EVENTPOINT(ep_base,current_mission->FreeEPoints)->Next;
			TO_EVENTPOINT(ep_base,current_mission->FreeEPoints)->Prev	=	0;

			//	Add it to the used list.
			TO_EVENTPOINT(ep_base,new_epoint)->Next						=	current_mission->UsedEPoints;
			if(current_mission->UsedEPoints)
				TO_EVENTPOINT(ep_base,current_mission->UsedEPoints)->Prev	=	new_epoint;
			current_mission->UsedEPoints								=	new_epoint;

			the_epoint->Used	=	TRUE;

			return	the_epoint;
		}		
	}
	return	NULL;
*/
}

//---------------------------------------------------------------

void	free_eventpoint(EventPoint *the_ep)
{
	EventPoint		*ep_base, *ep_joint;
	SLONG			 the_point,ins_point;


	//	Validate the param.
	if(current_mission && the_ep)
	{
		if ((the_ep->TriggeredBy==TT_SHOUT_ALL)||(the_ep->TriggeredBy==TT_SHOUT_ANY)) {
			if (the_ep->Radius)
				free((void*)the_ep->Radius);
		}
		switch (the_ep->WaypointType) {
		case WPT_MESSAGE:
		case WPT_CREATE_MAP_EXIT:
		case WPT_SHOUT:
		case WPT_NAV_BEACON:
		case WPT_BONUS_POINTS:
		case WPT_CONVERSATION:
			if(the_ep->Data[0]) {
				free((void*)the_ep->Data[0]);
				the_ep->Data[0]	=	0;
			}
			break;
		case WPT_CUT_SCENE:
			if (the_ep->Data[0])
				CUTSCENE_data_free((CSData*)the_ep->Data[0]);
			the_ep->Data[0]=0;
			break;
		}
		ep_base	=	current_mission->EventPoints;

		// Remove from used list
		BreakLink(ep_base,the_ep);

		// Add it to the used list
		ResetLink(ep_base,the_ep,FALSE);

		the_ep->Used=FALSE;

		ResetFreepoint(current_mission);
		ResetUsedpoint(current_mission);


/*
		//	Remove it from the used list.
		TO_EVENTPOINT(ep_base,the_ep->Next)->Prev	=	the_ep->Prev;
		if(the_ep->Prev)
			TO_EVENTPOINT(ep_base,the_ep->Prev)->Next	=	the_ep->Next;
		else
			current_mission->UsedEPoints	=	the_ep->Next;

		//	Add it to the free list.
		the_ep->Next	=	current_mission->FreeEPoints;
		if(current_mission->FreeEPoints)
			TO_EVENTPOINT(ep_base,current_mission->FreeEPoints)->Prev	=	EVENTPOINT_NUMBER(ep_base,the_ep);
		the_ep->Prev	=	0;
		current_mission->FreeEPoints	=	EVENTPOINT_NUMBER(ep_base,the_ep);

		the_ep->Used	=	FALSE;
*/
	}
}

//---------------------------------------------------------------

void	write_event_extra(FILE *file_handle, EventPoint *ep) {
	SLONG l;
	UBYTE u;

	if(ep->Used)
	{
		switch(ep->WaypointType){
		case WPT_MESSAGE:
		case WPT_CREATE_MAP_EXIT:
		case WPT_SHOUT:
		case WPT_NAV_BEACON:
		case WPT_CONVERSATION:
		case WPT_BONUS_POINTS:
			u=1;
			fwrite(&u,1,1,file_handle);
			if (!ep->Data[0]) {
				l=0; fwrite(&l,4,1,file_handle);
			} else {
				l=strlen((char*)ep->Data[0]);
				fwrite(&l,4,1,file_handle);
				fwrite((void*)ep->Data[0],l,1,file_handle);
			}
			break;
		case WPT_CUT_SCENE:
			u=2;
			fwrite(&u,1,1,file_handle);
			CUTSCENE_write(file_handle, (CSData*)ep->Data[0]);
			break;
		}
		switch(ep->TriggeredBy){
		case TT_SHOUT_ALL:
		case TT_SHOUT_ANY:
			u=1;
			fwrite(&u,1,1,file_handle);
			if (!ep->Radius) {
				l=0; fwrite(&l,4,1,file_handle);
			} else {
				l=strlen((char*)ep->Radius);
				fwrite(&l,4,1,file_handle);
				fwrite((void*)ep->Radius,l,1,file_handle);
			}
			break;
		}
	}
	
}

//---------------------------------------------------------------

void	read_event_extra(FILE *file_handle, EventPoint *ep, EventPoint *base, SLONG ver) {
	SLONG l;//,m;
	UWORD *pt;
	UBYTE u;

	if(ep->Used)
	{
		switch(ep->WaypointType){
		case WPT_NAV_BEACON:
			// a strange little thing cropped up.
			if (!ep->Data[9])
				ep->Data[9]=1000+(ep-base); // unique :}
			// FALL THRU
		case WPT_MESSAGE:
		case WPT_BONUS_POINTS:
		case WPT_CONVERSATION:
			// these will need translations done
			if (ver<7) {
				// generate fresh ID
				ep->Data[9]=ep-base;
			}
			// FALL THRU
		case WPT_CREATE_MAP_EXIT:
		case WPT_SHOUT:
			if (ver>7) // has a byte-code indicating what it is
				fread(&u,1,1,file_handle);
			// these don't have translations, they're internal codes or filenames
			if (ver>4) fread(&l,4,1,file_handle); else l=_MAX_PATH;
			if (l) {
				ep->Data[0]	=	(SLONG)malloc(l+1);
				ZeroMemory((char*)ep->Data[0],l+1);
				fread((void*)ep->Data[0],l,1,file_handle);
			} else ep->Data[0]=0;
			break;
		case WPT_CUT_SCENE:
			if (ver>7) // has a byte-code indicating what it is
				fread(&u,1,1,file_handle);
			CUTSCENE_read(file_handle,(CSData**)&ep->Data[0]);
			break;
		}
		switch(ep->TriggeredBy){
		case TT_SHOUT_ANY:
		case TT_SHOUT_ALL:
			if (ver>7) // has a byte-code indicating what it is
				fread(&u,1,1,file_handle);
			if (ver>4) fread(&l,4,1,file_handle); else l=_MAX_PATH;
			if (l) {
				ep->Radius = (SLONG)malloc(l+1);
				ZeroMemory((char*)ep->Radius,l+1);
				fread((void*)ep->Radius,l,1,file_handle);
			} else ep->Radius=0;
			break;
		}
	}
	
}

//---------------------------------------------------------------

#define	M_VERSION		10
#define	EP_VERSION		1

BOOL	export_mission(void)
{
	CBYTE				curr_dir[_MAX_PATH];
	ULONG				m_vers;
	ULONG				c0,
						count,
						current_ep;
	EventPoint			*ep_base,
						*ep_ptr;
	FILE				*file_handle;
	OPENFILENAME		save_mission;

	Mission				*temp_mission;	


	if (!valid_mission()) {
		MessageBox(0,"The mission is invalid. Check the list of waypoints for warning symbols, fix the mistakes, and try again.","Error",MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}

	if(current_mission)
	{
		//	Set up the default directory.
		sprintf(map_default_dir,"c:\\Fallen\\Levels",curr_dir);

		//	Set up the save file structure.
		ZeroMemory(&save_mission,sizeof(OPENFILENAME));
//		sprintf(mission_name,"*.ucm");
		sprintf(mission_name,"%s.ucm",current_mission->MissionName);
		save_mission.lStructSize		=	sizeof(OPENFILENAME);
		save_mission.hwndOwner			=	NULL;
		save_mission.lpstrFilter		=	"Game Mission Files\0*.ucm\0\0";
		save_mission.lpstrFile			=	mission_name;
		save_mission.nMaxFile			=	_MAX_PATH;
		save_mission.lpstrInitialDir	=	map_default_dir;
		save_mission.lpstrTitle			=	"Export a mission file";
		save_mission.Flags				=	OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
		save_mission.lpstrDefExt		=	"ucm";

		if(GetSaveFileName(&save_mission))
		{
			//	Save the mission.
			file_handle	=	fopen(mission_name,"wb");

			if(file_handle)
			{
				//	Save the file version.
				m_vers	=	M_VERSION;
				fwrite(&m_vers,sizeof(m_vers),1,file_handle);

				temp_mission = new Mission();
				memcpy(temp_mission,current_mission,sizeof(*temp_mission));

				for (c0=0;c0<MAX_EVENTPOINTS;c0++)
				{
//					if (temp_mission->EventPoints[c0].WaypointType==WPT_CREATE_PLAYER) 
//						ASSERT(0);

					if (temp_mission->EventPoints[c0].WaypointType==WPT_CREATE_ENEMIES) 
					{
						SWORD ai=LOWORD(temp_mission->EventPoints[c0].Data[5]);
						SLONG skill=HIWORD(temp_mission->EventPoints[c0].Data[5]);
						if (!skill) skill=current_mission->SkillLevels[ai];
						temp_mission->EventPoints[c0].Data[5]=ai | (skill<<16);
					}
				}
				
				//
				// Work out which event_points are referenced from other ones.
				//

				{
					SLONG i;

					EventPoint *ep;

					//
					// Mark all event points as unreferenced.
					//

					for (i = 0; i < MAX_EVENTPOINTS; i++)
					{
						ep = &temp_mission->EventPoints[i];

						ep->Flags &= ~WPT_FLAGS_REFERENCED;
					}

					//
					// Marks the given waypoint as referenced if it is in a valid range.
					//

					#define MARK_REFERENCED(w) {if (WITHIN((w),1,MAX_EVENTPOINTS - 1)) {temp_mission->EventPoints[w].Flags |= WPT_FLAGS_REFERENCED;}}

					//
					// Flag each referenced waypoint. This is a bit hit and miss!
					//

					for (i = 0; i < MAX_EVENTPOINTS; i++)
					{
						ep = &temp_mission->EventPoints[i];

						switch(ep->WaypointType)
						{
							case WPT_NONE:
							case WPT_SIMPLE:
							case WPT_CREATE_PLAYER:
								break;

							case WPT_ADJUST_ENEMY:
								MARK_REFERENCED(ep->Data[6]);
								break;

							case WPT_CREATE_ENEMIES:

								if (ep->Data[5] == 8)	// Bodyguard
								{
									MARK_REFERENCED(ep->Data[7]);
								}

								if (ep->Data[3] == 4)	// Follow
								{
									MARK_REFERENCED(ep->Data[1]);
								}

								break;

							case WPT_CREATE_VEHICLE:
							case WPT_CREATE_ITEM:
							case WPT_CREATE_CREATURE:
							case WPT_CREATE_CAMERA:
							case WPT_CAMERA_WAYPOINT:
							case WPT_CREATE_TARGET:
							case WPT_TARGET_WAYPOINT:
							case WPT_CREATE_MAP_EXIT:
							case WPT_MESSAGE:
							case WPT_WAREFX:
								break;

							case WPT_CONVERSATION:
								MARK_REFERENCED(ep->Data[1]);
								MARK_REFERENCED(ep->Data[2]);
								break;

							case WPT_SOUND_EFFECT:
							case WPT_VISUAL_EFFECT:
							case WPT_SPOT_EFFECT:
							case WPT_CUT_SCENE:
							case WPT_TELEPORT:
							case WPT_TELEPORT_TARGET:
							case WPT_END_GAME_LOSE:
							case WPT_END_GAME_WIN:
							case WPT_SHOUT:
							case WPT_ACTIVATE_PRIM:
							case WPT_CREATE_TRAP:
							case WPT_LINK_PLATFORM:
							case WPT_CREATE_BOMB:
							case WPT_BURN_PRIM:
							case WPT_NAV_BEACON:
							case WPT_CREATE_BARREL:
							case WPT_CONE_PENALTIES:
							case WPT_NO_FLOOR:
							case WPT_SHAKE_CAMERA:
								break;

							case WPT_KILL_WAYPOINT:
								MARK_REFERENCED(ep->Data[0]);
								break;

							case WPT_CREATE_TREASURE:
							case WPT_BONUS_POINTS:
							case WPT_GROUP_LIFE:
							case WPT_GROUP_DEATH:
							case WPT_INTERESTING:
							case WPT_INCREMENT:
							case WPT_DYNAMIC_LIGHT:
							case WPT_GOTHERE_DOTHIS:
							case WPT_GROUP_RESET:
							case WPT_COUNT_UP_TIMER:
							case WPT_RESET_COUNTER:
							case WPT_CREATE_MIST:
								break;

							case WPT_TRANSFER_PLAYER:
								MARK_REFERENCED(ep->Data[0]);
								break;

							case WPT_AUTOSAVE:
							case WPT_MAKE_SEARCHABLE:
								break;

							case WPT_LOCK_VEHICLE:
								MARK_REFERENCED(ep->Data[0]);
								break;

							case WPT_STALL_CAR:
								MARK_REFERENCED(ep->Data[0]);
								break;

							case WPT_EXTEND:
							case WPT_MOVE_THING:
							case WPT_MAKE_PERSON_PEE:
								MARK_REFERENCED(ep->Data[0]);
								break;

							case WPT_SIGN:
								break;


							default: // arg. yet another bloody assert that goes off when you add a new wpt type
									 // Got me too- like the validate one did!

								ASSERT(0);
								return FALSE;
						}

						switch(ep->TriggeredBy)
						{
							case TT_NONE:
								break;

							case TT_DEPENDENCY:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_RADIUS:
							case TT_DOOR:
							case TT_TRIPWIRE:
							case TT_PRESSURE_PAD:
							case TT_ELECTRIC_FENCE:
							case TT_WATER_LEVEL:
							case TT_SECURITY_CAMERA:
							case TT_SWITCH:
							case TT_ANIM_PRIM:
							case TT_TIMER:
							case TT_SHOUT_ALL:
								break;

							case TT_BOOLEANAND:
							case TT_BOOLEANOR:
								MARK_REFERENCED(ep->EPRef);
								MARK_REFERENCED(ep->EPRefBool);
								break;

							case TT_ITEM_HELD:
							case TT_SPECIFIC_ITEM_HELD:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_ITEM_SEEN:
								break;

							case TT_KILLED:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_SHOUT_ANY:
								break;

							case TT_COUNTDOWN:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_ENEMYRADIUS:
							case TT_THING_RADIUS_DIR:
							case TT_MOVE_RADIUS_DIR:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_VISIBLECOUNTDOWN:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_CUBOID:
								break;

							case TT_HALFDEAD:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_PERSON_SEEN:
							case TT_PERSON_IN_VEHICLE:
								MARK_REFERENCED(ep->EPRef);
								MARK_REFERENCED(ep->EPRefBool);
								break;

							case TT_PERSON_USED:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_PLAYER_USES_RADIUS:
							case TT_GROUPDEAD:
							case TT_PRIM_DAMAGED:
								break;

							case TT_PERSON_ARRESTED:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_CONVERSATION_OVER:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_COUNTER:
								break;

							case TT_KILLED_NOT_ARRESTED:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_CRIME_RATE_ABOVE:
							case TT_CRIME_RATE_BELOW:
								break;

							case TT_PERSON_IS_MURDERER:
							case TT_RANDOM:
								MARK_REFERENCED(ep->EPRef);
								break;

							case TT_PLAYER_FIRES_GUN:
							case TT_DARCI_GRABBED:
								break;

							case TT_PUNCHED_AND_KICKED:
								MARK_REFERENCED(ep->EPRef);
								break;

							default:
								ASSERT(0);
								break;
						}
					}
				}

				fwrite((void*)temp_mission,sizeof(Mission),1,file_handle);

				delete temp_mission;

				//	Save out the text.
				for(c0=0;c0<MAX_EVENTPOINTS;c0++)
					write_event_extra(file_handle,&current_mission->EventPoints[c0]);

				fwrite((void*)MissionZones[current_mission-mission_pool],128*128,1,file_handle);

				fclose(file_handle);
			}

			return	TRUE;
		}
	}
	return	FALSE;
}


void import_mission(void) {
	UWORD				new_mission;
	OPENFILENAME		open_mission;
	CBYTE				curr_dir[_MAX_PATH];
	FILE				*file_handle;
	ULONG				m_vers;
	HTREEITEM			map_handle;
	TV_ITEM				map_item;
	TV_INSERTSTRUCT		tv_is;
	WSElement			*map_element,
						*new_element;
	SLONG				c0;
	Mission				*the_mission;

	sprintf(map_default_dir,"c:\\Fallen\\Levels",curr_dir);

	//	Set up the save file structure.
	ZeroMemory(&open_mission,sizeof(OPENFILENAME));
	sprintf(mission_name,"*.ucm");
	open_mission.lStructSize		=	sizeof(OPENFILENAME);
	open_mission.hwndOwner			=	NULL;
	open_mission.lpstrFilter		=	"Game Mission Files\0*.ucm\0\0";
	open_mission.lpstrFile			=	mission_name;
	open_mission.nMaxFile			=	_MAX_PATH;
	open_mission.lpstrInitialDir	=	map_default_dir;
	open_mission.lpstrTitle			=	"Import a mission file";
	open_mission.Flags				=	OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	open_mission.lpstrDefExt		=	"ucm";

	//	First get the parent map info.
	map_handle		=	TreeView_GetSelection(ws_tree);
	map_item.hItem	=	map_handle;
	map_item.mask	=	TVIF_PARAM;
	TreeView_GetItem(ws_tree,&map_item);
	map_element		=	(WSElement*)map_item.lParam;

	if(GetOpenFileName(&open_mission))
	{
		//...
		file_handle	=	fopen(mission_name,"rb");

		if(file_handle)
		{
			//	Read the file version.
			m_vers	=	M_VERSION;
			fread(&m_vers,sizeof(m_vers),1,file_handle);

			new_mission	=	alloc_mission(map_element->MapRef);
			if(new_mission)
			{
				the_mission	=	&mission_pool[new_mission];
				// Read the mission data
				if (m_vers>8)
					fread((void*)the_mission,sizeof(Mission),1,file_handle);
				else
					if (m_vers>5) {
						OldMissionB temp;
						fread((void*)&temp,sizeof(OldMissionB),1,file_handle);
						ZeroMemory(the_mission,sizeof(Mission));
						memcpy(the_mission,&temp,sizeof(OldMissionB));
/*						the_mission->CarsRate=0;
						memcpy(the_mission->EventPoints,temp.EventPoints,sizeof(EventPoint)*MAX_EVENTPOINTS);
						memcpy(the_mission->SkillLevels,temp.SkillLevels,255);*/
					} else {
						OldMission temp;
						fread((void*)&temp,sizeof(OldMission),1,file_handle);
						ZeroMemory(the_mission,sizeof(Mission));
						memcpy(the_mission,&temp,sizeof(OldMission));
					}
				if (m_vers<10) the_mission->BoredomRate=4;

				//	Read in the text.
				for(c0=0;c0<MAX_EVENTPOINTS;c0++)
					read_event_extra(file_handle,&the_mission->EventPoints[c0],the_mission->EventPoints,m_vers);

				fread((void*)MissionZones[the_mission-mission_pool],128*128,1,file_handle);

				strcpy(the_mission->MissionName,InputBox("Import Mission", "Enter new name for mission:", the_mission->MissionName));
				the_mission->MapIndex=map_element->MapRef;
				strcpy(the_mission->MapName,game_maps[map_element->MapRef].MapName);
				
				//	Now create the new mission tree entry
				new_element	=	new WSElement;
				if(new_element)
				{
					tv_is.hParent				=	map_handle;
					tv_is.hInsertAfter			=	TVI_LAST;
					tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
					tv_is.item.iImage			=	IM_MISSION;
					tv_is.item.iSelectedImage	=	IM_MISSION;
					tv_is.item.pszText			=	the_mission->MissionName;
					tv_is.item.lParam			=	(LPARAM)new_element;
					new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
					new_element->ElementType	=	ET_MISSION;
					new_element->MapRef			=	map_element->MapRef;
					new_element->MissionRef		=	new_mission;

					//	Update the window.
					InvalidateRect(ws_tree, NULL, FALSE);
				}
			}

			fclose(file_handle);

			ResetFreepoint(the_mission);
			ResetFreelist(the_mission);
		}
	}

}

//---------------------------------------------------------------

void refresh_mission(void)
{
//	UWORD				new_mission;
	OPENFILENAME		open_mission;
	CBYTE				file_name[_MAX_PATH], *chr, msg[_MAX_PATH+200];
	FILE				*file_handle;
	ULONG				m_vers;
	SLONG				temp_map;
//	HTREEITEM			map_handle;
//	TV_ITEM				map_item;
//	TV_INSERTSTRUCT		tv_is;
//	WSElement			*map_element,
//						*new_element;
	SLONG				c0;
//	Mission				*the_mission;

	if (!current_mission) {
		MessageBox(0,"Select a mission to refresh first.","Error",MB_ICONEXCLAMATION|MB_OK);
	}

	strcpy(file_name,"c:\\fallen\\levels\\");
	strcat(file_name,current_mission->MissionName);
	_strlwr(file_name);
	if (!strstr(file_name,".ucm"))
		strcat(file_name,".ucm");

	if (!FileExists(file_name)) {
		sprintf(msg,"Cannot find an updated version of the mission in %s.",file_name);
		MessageBox(0,msg,"Error",MB_ICONEXCLAMATION|MB_OK);
		return;
	}

	temp_map=current_mission->MapIndex;

	file_handle	=	fopen(file_name,"rb");

	if(!file_handle) {
		sprintf(msg,"Cannot open the updated version of the mission from %s.",file_name);
		MessageBox(0,msg,"Error",MB_ICONEXCLAMATION|MB_OK);
		return;
	}

	//  Go through and clear out the old mission data (esp. the memory allocated to waypoints!)

	for(c0=0;c0<MAX_EVENTPOINTS;c0++)
	  free_eventpoint(current_mission->EventPoints+c0);

	//	Read the file version.
	m_vers	=	M_VERSION;
	fread(&m_vers,sizeof(m_vers),1,file_handle);

	// Read the mission data
	if (m_vers>8)
		fread((void*)current_mission,sizeof(Mission),1,file_handle);
	else
		if (m_vers>5) {
			OldMissionB temp;
			fread((void*)&temp,sizeof(OldMissionB),1,file_handle);
			ZeroMemory(current_mission,sizeof(Mission));
			memcpy(current_mission,&temp,sizeof(OldMissionB));
		} else {
			OldMission temp;
			fread((void*)&temp,sizeof(OldMission),1,file_handle);
			ZeroMemory(current_mission,sizeof(Mission));
			memcpy(current_mission,&temp,sizeof(OldMission));
		}
	if (m_vers<10) current_mission->BoredomRate=4;

	//	Read in the text.
	for(c0=0;c0<MAX_EVENTPOINTS;c0++)
		read_event_extra(file_handle,&current_mission->EventPoints[c0],current_mission->EventPoints,m_vers);

	fread((void*)MissionZones[current_mission-mission_pool],128*128,1,file_handle);

	current_mission->MapIndex=temp_map;//MAP_NUMBER(current_map);

//		the_mission->MapIndex=map_element->MapRef;
//		strcpy(the_mission->MapName,game_maps[map_element->MapRef].MapName);
	
/*		//	Now create the new mission tree entry
	new_element	=	new WSElement;
	if(new_element)
	{
		tv_is.hParent				=	map_handle;
		tv_is.hInsertAfter			=	TVI_LAST;
		tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
		tv_is.item.iImage			=	IM_MISSION;
		tv_is.item.iSelectedImage	=	IM_MISSION;
		tv_is.item.pszText			=	the_mission->MissionName;
		tv_is.item.lParam			=	(LPARAM)new_element;
		new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
		new_element->ElementType	=	ET_MISSION;
		new_element->MapRef			=	map_element->MapRef;
		new_element->MissionRef		=	new_mission;

		//	Update the window.
		InvalidateRect(ws_tree, NULL, FALSE);
	}*/

	fclose(file_handle);

	ResetFreelist(current_mission);
	ResetUsedlist(current_mission);
	ResetFreepoint(current_mission);
	ResetUsedpoint(current_mission);

	reset_wptlist();
	fill_wptlist(current_mission);
}

//---------------------------------------------------------------

BOOL NoWaypointsFor(EventPoint *ep) {
	EventPoint *scan;
	SLONG c0;

	scan=current_mission->EventPoints;
	for(c0=0;c0<MAX_EVENTPOINTS;c0++,scan++) 
		if (scan->Used) {
			if ((scan!=ep)&&(ep->Group==scan->Group)&&(ep->Colour==scan->Colour)&&(scan->WaypointType==WPT_SIMPLE))
				return FALSE;
		}
	return TRUE;
}

//---------------------------------------------------------------

BOOL  HasText(EventPoint *ep) {
	switch (ep->WaypointType) {
		case WPT_MESSAGE:
		case WPT_CREATE_MAP_EXIT:
		case WPT_SHOUT:
		case WPT_NAV_BEACON:
		case WPT_BONUS_POINTS:
		case WPT_CONVERSATION:
			return (ep->Data[0]) ? 1 : 0;
		default:
			return 0;
	}
}

UWORD GetEPTextID(EventPoint *ep) {
	if ((!ep->Used)||(!HasText(ep))) return 0;
	return (UWORD)ep->Data[9];
}

void SetEPTextID(EventPoint *ep, SLONG value) {

	if (value==-1) { // fetch it
		EventPoint *scan;
		SLONG c0,tid;

		scan=current_mission->EventPoints;
		for(c0=0;c0<MAX_EVENTPOINTS;c0++,scan++) {
			tid=GetEPTextID(scan);
			if (tid>value) value=tid;
		}
		value++;
		if (value==0) value=1;
	}
	ep->Data[9]=value;
}

CBYTE *GetEPText(EventPoint *ep) {
	if (!ep->Data[0]) return 0;
	return (CBYTE*)(ep->Data[0]);
}

//---------------------------------------------------------------


SLONG treasure_counter;

BOOL SingleFlagCheck(SLONG test) {
	SLONG gotone=0, i;
	for (i=0;i<32;i++) {
		if (test&(1<<i)) {
			if (gotone) return FALSE; // more than one of em
			gotone=1;
		}
	}
	return (BOOL)gotone;
}


BOOL valid_ep(EventPoint *ep) {


	if ((WaypointUses[ep->TriggeredBy] & WPU_DEPEND)&&!ep->EPRef) return FALSE;

	if ((WaypointUses[ep->TriggeredBy] & WPU_BOOLEAN) && !ep->EPRefBool)
	{
		//
		// person_seen and person_in_vehicle use bool and depend, but allows zero bool
		//

		if (ep->TriggeredBy != TT_PERSON_SEEN &&
			ep->TriggeredBy != TT_PERSON_IN_VEHICLE)
		{
			return FALSE;
		}
	}

	if (ep->TriggeredBy == TT_PERSON_IN_VEHICLE)
	{
		//
		// EPRef must be a person and if EPRefBool then it must be a vehicle.
		//

		if (!ep->EPRef)
		{
			return FALSE;
		}

		if (current_mission->EventPoints[ep->EPRef].WaypointType != WPT_CREATE_PLAYER &&
			current_mission->EventPoints[ep->EPRef].WaypointType != WPT_CREATE_ENEMIES)
		{
			return FALSE;
		}

		if (ep->EPRefBool)
		{
			if (current_mission->EventPoints[ep->EPRefBool].WaypointType != WPT_CREATE_VEHICLE)
			{
				return FALSE;
			}
		}
	}

//	if ((WaypointUses[ep->TriggeredBy] & WPU_TIME)&&!ep->Radius) return FALSE;
	//if ((WaypointUses[ep->TriggeredBy] & WPU_RADIUS ... eh, who cares
	if (WaypointUses[ep->TriggeredBy] & WPU_COUNTER) {
		if ((!ep->Radius)||(ep->EPRef<1)||(ep->EPRef>10)) return FALSE; // trigger would go off immediately, or bad counter num
	}
	if ((WaypointUses[ep->TriggeredBy] & WPU_RADTEXT)&&!ep->Radius) return FALSE;
	// whereas that one's a doozy

	// special depends
	if (ep->TriggeredBy==TT_HALFDEAD) {
		if (current_mission->EventPoints[ep->EPRef].WaypointType!=WPT_CREATE_ENEMIES) return FALSE;
	}
	if (ep->TriggeredBy==TT_ITEM_HELD) {
		if (!ep->EPRef) return FALSE;
		if (current_mission->EventPoints[ep->EPRef].WaypointType==WPT_CREATE_ITEM) return TRUE;
		if (current_mission->EventPoints[ep->EPRef].WaypointType==WPT_CREATE_ENEMIES) {
			if (!current_mission->EventPoints[ep->EPRef].Data[8]) return SingleFlagCheck(current_mission->EventPoints[ep->EPRef].Data[9]);
			if (!current_mission->EventPoints[ep->EPRef].Data[9]) return SingleFlagCheck(current_mission->EventPoints[ep->EPRef].Data[8]);
			return FALSE;
		}
		return FALSE;
	}

	if (ep->TriggeredBy==TT_SPECIFIC_ITEM_HELD) {
		if (!ep->EPRef) return FALSE;
		if (current_mission->EventPoints[ep->EPRef].WaypointType!=WPT_CREATE_ITEM) return FALSE;
	}

	if (ep->TriggeredBy==TT_PERSON_SEEN) {
		SLONG type;
		// must be a person (enemy or player)
		type=current_mission->EventPoints[ep->EPRef].WaypointType;
		if ((type!=WPT_CREATE_ENEMIES)&&(type!=WPT_CREATE_PLAYER)) return FALSE;
		if (ep->EPRefBool) { // null bool is ok ("see anyone") but if it points, must be person
			type=current_mission->EventPoints[ep->EPRefBool].WaypointType;
			if ((type!=WPT_CREATE_ENEMIES)&&(type!=WPT_CREATE_PLAYER)) return FALSE;
		}
	}
	if ((ep->TriggeredBy==TT_PERSON_USED)||(ep->TriggeredBy==TT_PERSON_ARRESTED)||(ep->TriggeredBy==TT_PLAYER_CARRY_PERSON)) {
		SLONG type;
		// must be a person (enemy)
		type=current_mission->EventPoints[ep->EPRef].WaypointType;
		if (type!=WPT_CREATE_ENEMIES) return FALSE;
	}

	if (ep->TriggeredBy == TT_THING_RADIUS_DIR ||
	    ep->TriggeredBy == TT_MOVE_RADIUS_DIR)
	{
		//
		// Waypoint pointed to must be either a person or a vehicle...
		//

		SLONG type = current_mission->EventPoints[ep->EPRef].WaypointType;

		if (type != WPT_CREATE_ENEMIES &&
			type != WPT_CREATE_PLAYER  &&
			type != WPT_CREATE_VEHICLE)
		{
			return FALSE;
		}
	}

	if ((ep->TriggeredBy==TT_CONVERSATION_OVER)&&(current_mission->EventPoints[ep->EPRef].WaypointType!=WPT_CONVERSATION)) return FALSE;

	switch (ep->WaypointType) {
	case WPT_NONE:
		return FALSE;					// these don't exist

	case WPT_SIMPLE:
		return TRUE;					// these are simple

	case WPT_CREATE_PLAYER:
		return TRUE;					// these too

	case WPT_ADJUST_ENEMY:
		// same as create but extra check... (so, falling thru to next case)
		if (!ep->Data[6]) return FALSE;
		if (current_mission->EventPoints[ep->Data[6]].WaypointType!=WPT_CREATE_ENEMIES &&
			current_mission->EventPoints[ep->Data[6]].WaypointType!=WPT_CREATE_PLAYER)
		{
			return FALSE;
		}
		return TRUE;

	case WPT_CREATE_ENEMIES:
//		if ((!ep->Data[0])||(!ep->Data[1]))
		if ((!HIWORD(ep->Data[0]))||(!LOWORD(ep->Data[0])))
			return FALSE;
		// ai
		switch (ep->Data[5]&0xffff) {
//		if ((ep->Data[5]==3)||(ep->Data[5]==8)||(ep->Data[5]==17)) { // assassin/bodyguard/genocide
		case 3:
		case 8:
		case 17:
			SLONG targ;
			if (!ep->Data[7]) return FALSE;
			targ=current_mission->EventPoints[ep->Data[7]].WaypointType;
			if ((targ!=WPT_CREATE_ENEMIES)&&(targ!=WPT_CREATE_PLAYER))
				return FALSE;
		default:
			;// moo
		}
		// move
		if ((ep->Data[3]==1)||(ep->Data[3]==2)) {
			if (NoWaypointsFor(ep)) return FALSE;
		}
		if (ep->Data[3]==4) { // follow
			SLONG targ;
			if (!ep->Data[1]) return FALSE;
			targ=current_mission->EventPoints[ep->Data[1]].WaypointType;
			if ((targ!=WPT_CREATE_ENEMIES)&&(targ!=WPT_CREATE_PLAYER))
				return FALSE;
		}
		return TRUE;

	case WPT_ENEMY_FLAGS:
		if (!ep->Data[0]) return FALSE;
		if (current_mission->EventPoints[ep->Data[0]].WaypointType!=WPT_CREATE_ENEMIES)
			return FALSE;
		return TRUE;

	case WPT_CREATE_VEHICLE:
		if (!ep->Data[0]) return FALSE;
		if ((ep->Data[1]==3)&&(!ep->Data[2])) return FALSE;
		return TRUE;

	case WPT_CREATE_ITEM:
		if ((!ep->Data[0])||(!ep->Data[1]))
			return FALSE;
		return TRUE;

	case WPT_CREATE_CREATURE:
		if ((!ep->Data[0])||(!ep->Data[1]))
			return FALSE;
		return TRUE;

	case WPT_CREATE_CAMERA:
	case WPT_CAMERA_WAYPOINT:
		return TRUE;		// need to find out what makes cameras invalid

	case WPT_CREATE_TARGET:
	case WPT_TARGET_WAYPOINT:
		if ((!ep->Data[0])||(!ep->Data[1]))
			return FALSE;
		return TRUE;

	case WPT_CREATE_MAP_EXIT:
		if (!ep->Data[0]) return FALSE;
		return TRUE;

	case WPT_MESSAGE:
		if (!ep->Data[0]) return FALSE;
		return TRUE;

	case WPT_CONVERSATION:
		if (!ep->Data[0]) return FALSE;
		if (!ep->Data[1]) return FALSE;
		if (!ep->Data[2]) return FALSE;
		return TRUE;


	case WPT_SOUND_EFFECT:
	case WPT_VISUAL_EFFECT:
	case WPT_SPOT_EFFECT:
	case WPT_CUT_SCENE:
	case WPT_TELEPORT:
	case WPT_TELEPORT_TARGET:
	case WPT_END_GAME_LOSE:
	case WPT_END_GAME_WIN:
	case WPT_CONE_PENALTIES:
	case WPT_NO_FLOOR:
	case WPT_SHAKE_CAMERA:
		return TRUE;

	case WPT_SHOUT:
		if (!ep->Data[0]) return FALSE;
		return TRUE;

	case WPT_ACTIVATE_PRIM:
	case WPT_CREATE_TRAP:
	case WPT_LINK_PLATFORM:
		return TRUE;

	case WPT_CREATE_BOMB:
		// default is legit; it'll just be a dud...
		return TRUE;

	case WPT_BURN_PRIM:
		return TRUE;

	case WPT_NAV_BEACON:
		return (ep->Data[0]) ? TRUE : FALSE;

	case WPT_CREATE_BARREL:
		return TRUE;

	case WPT_KILL_WAYPOINT:
		return (ep->Data[0]) ? TRUE : FALSE;

	case WPT_CREATE_TREASURE:
		treasure_counter++;
		return (ep->Data[0]) ? TRUE : FALSE;

	case WPT_BONUS_POINTS:
		return (ep->Data[0]) ? TRUE : FALSE;

	case WPT_GROUP_LIFE:
		return TRUE;

	case WPT_GROUP_DEATH:
		return TRUE;

	case WPT_INTERESTING:
		return TRUE;

	case WPT_DYNAMIC_LIGHT:
		// might even not be a lie...
		return TRUE;

	case WPT_INCREMENT:

		//
		// Make sure the counter is in range.
		//

		if (WITHIN(ep->Data[0], 1, 10))
		{
			return TRUE;
		}

		return FALSE;

	case WPT_GOTHERE_DOTHIS:
		return TRUE;

	case WPT_TRANSFER_PLAYER:
		
		if (!WITHIN(ep->Data[0], 1, MAX_EVENTPOINTS - 1))
		{
			return FALSE;
		}

		if (current_mission->EventPoints[ep->Data[0]].WaypointType != WPT_CREATE_ENEMIES &&
			current_mission->EventPoints[ep->Data[0]].WaypointType != WPT_CREATE_PLAYER)
		{
			return FALSE;
		}

		return TRUE;

	case WPT_AUTOSAVE:
	case WPT_MAKE_SEARCHABLE:
		return TRUE;

	case WPT_LOCK_VEHICLE:

		{
			if (ep->Data[0])
			{
				if (current_mission->EventPoints[ep->Data[0]].WaypointType == WPT_CREATE_VEHICLE)
				{
					return TRUE;
				}
			}
		}

		return FALSE;

		break;

	case WPT_GROUP_RESET:
		return TRUE;

	case WPT_COUNT_UP_TIMER:
		return TRUE;

	case WPT_RESET_COUNTER:
		return WITHIN(ep->Data[0], 0, 9);

	case WPT_CREATE_MIST:
		return TRUE;

	case WPT_WAREFX:
		return TRUE;

	case WPT_STALL_CAR:

		{
			if (ep->Data[0])
			{
				if (current_mission->EventPoints[ep->Data[0]].WaypointType == WPT_CREATE_VEHICLE)
				{
					return TRUE;
				}
			}
		}

		return FALSE;

	case WPT_EXTEND:

		{
			if (ep->Data[0])
			{
				if (current_mission->EventPoints[ep->Data[0]].TriggeredBy == TT_VISIBLECOUNTDOWN)
				{
					if (ep->Data[1])
					{
						return TRUE;
					}
				}
			}
		}

		return FALSE;

	case WPT_MOVE_THING:

		if (ep->Data[0])
		{
			if (current_mission->EventPoints[ep->Data[0]].WaypointType == WPT_CREATE_VEHICLE ||
				current_mission->EventPoints[ep->Data[0]].WaypointType == WPT_CREATE_PLAYER  ||
				current_mission->EventPoints[ep->Data[0]].WaypointType == WPT_CREATE_ENEMIES ||
				current_mission->EventPoints[ep->Data[0]].WaypointType == WPT_CREATE_ITEM    ||
				current_mission->EventPoints[ep->Data[0]].WaypointType == WPT_CREATE_CREATURE)
			{
				return TRUE;
			}
		}

		return FALSE;

	case WPT_MAKE_PERSON_PEE:

		if (ep->Data[0])
		{
			if (current_mission->EventPoints[ep->Data[0]].WaypointType == WPT_CREATE_PLAYER  ||
				current_mission->EventPoints[ep->Data[0]].WaypointType == WPT_CREATE_ENEMIES)
			{
				return TRUE;
			}
		}

		return FALSE;

	case WPT_SIGN:
		return TRUE;

	default:
		ASSERT(0);
		return FALSE;
	}
}


BOOL valid_mission() {
	SLONG c0;
	EventPoint *ep;
	BOOL miss_valid=1;

	if(!current_mission) return FALSE;
	selected_ep=hilited_ep=NULL;
	TreeView_SelectItem(wpt_tree,NULL);

	treasure_counter=0;
	
	ep=current_mission->EventPoints;
	for(c0=0;c0<MAX_EVENTPOINTS;c0++,ep++) 
		if (ep->Used) {
			ep->Flags=(ep->Flags&~WPT_FLAGS_SUCKS)|(!valid_ep(ep));
			miss_valid&=!(ep->Flags&WPT_FLAGS_SUCKS);
		}
	reset_wptlist();
	fill_wptlist(current_mission);

	if (treasure_counter!=10) {
		CBYTE msg[100];
		sprintf(msg,"There are %d treasure items.",treasure_counter);
		CONSOLE_text(msg,10000);
	}

	//
	// Go through all the messages and work out if some should be
	// street names when they are not.
	//

	CBYTE *title = "Is this message a street or place name?";

	FILE *handle;
	
	handle = fopen("c:\\quats.txt", "rb");

	if (handle)
	{
		title = "Hello Simon! Is this message a street or place name?";

		fclose(handle);
	}

	handle = fopen("c:\\win64.dim", "rb");

	if (handle)
	{
		title = "Hi Barry! Is this message a street or place name?";

		fclose(handle);
	}

	{
		SLONG i;

		for (i = 0; i < MAX_EVENTPOINTS; i++)
		{
			ep = &current_mission->EventPoints[i];

			if (ep->WaypointType == WPT_MESSAGE)
			{
				if (ep->Data[0])
				{
					extern SLONG is_street_name(CBYTE *str_in);

					if (ep->Data[2] == 0xffff)
					{
						//
						// This is already a street name.
						// 
					}
					else
					if (ep->Data[2] != NULL)
					{
						//
						// This is a message from someone... so it can't be a street name.
						//
					}
					else
					if (is_street_name((CBYTE *) ep->Data[0]))
					{
						CBYTE mess[512];

						sprintf(mess, "%s\n\n\"%s\"\n\n(Tell Mark if it misses out a street name or mistakes a normal message for one.)", title, ep->Data[0]);

						//
						// This could be a street name... better ask the level designer!
						//

						if (MessageBox(NULL, mess, "Change message type?", MB_ICONQUESTION | MB_YESNO) == IDYES)
						{
							ep->Data[2] = 0xffff;
						}
					}
				}
			}
		}
	}

	return miss_valid;
}
