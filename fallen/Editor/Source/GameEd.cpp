// GameEd.cpp
// Guy Simmons, 12th January 1998

#include	"Editor.hpp"
#include	"EdCom.h"
#include	"Edway.h"
#include	"fallen/headers/game.h"
#include	"fallen/headers/animtmap.h"
#include	"fallen/headers/memory.h"

#define	MODE_PLACE		1
#define	MODE_SELECT		2

#define	THING_BOX_SIZE	13
#define	WP_BOX_SIZE		9
#define	SH_BOX_SIZE		9

//---------------------------------------------------------------

extern	CBYTE	*class_text[];
extern	CBYTE	*genus_text[][10];

//---------------------------------------------------------------

GameEditor::~GameEditor()
{
	DestroyTabs();
}

//---------------------------------------------------------------

void	GameEditor::SetupModule(void)
{
	GameEdDefaults		the_defaults;


	// Setup the module defaults.
	the_defaults.Left	=	0;
	the_defaults.Top	=	20;
	the_defaults.Width	=	780;
	the_defaults.Height	=	560;
	SetupWindow	(
					"Game Editor",
					(HAS_TITLE|HAS_GROW|HAS_CONTROLS),
					the_defaults.Left,
					the_defaults.Top,
					the_defaults.Width,
					the_defaults.Height
				);
	SetContentColour(CONTENT_COL);
	SetControlsHeight(440);
	SetControlsWidth(300);

	// Initialise module members.
	HilitedItem.ItemType	=	0;
	LastItem.ItemType		=	0;
	SelectedItem.ItemType	=	0;
	FlashState				=	TRUE;
	EdEngine				=	engine;
	SelectMode				=	0;

	// Create the mode tabs.
	CreateTabs();

	if(ThingMode)
	{
		ThingMode->UpdateTabInfo();
		ThingMode->UpdateClassInfo();
	}

	init_ed_waypoints();
	init_ed_clists();

	// The Win conditions.
	win_conditions	=	alloc_ed_clist();
	add_condition(win_conditions,alloc_ed_condition());
	sprintf(win_conditions->CListName,"Win Conditions");

	// The Lose conditions.
	lose_conditions	=	alloc_ed_clist();
	add_condition(lose_conditions,alloc_ed_condition());
	sprintf(lose_conditions->CListName,"Lose Conditions");
}

//---------------------------------------------------------------

void	GameEditor::CreateTabs(void)
{
	EdRect		bounds_rect;


	bounds_rect.SetRect(ControlsLeft(),ControlsTop(),ControlsWidth(),ControlsHeight());

	SaveMode	=	new	SaveTab;
	if(SaveMode)
	{
		AddTab(SaveMode);
		SaveMode->SetupModeTab("Levels",TAB_LEVELS,&bounds_rect,ExternalUpdate);
	}

	ThingMode	=	new	ThingTab;
	if(ThingMode)
	{
		AddTab(ThingMode);
		ThingMode->SetupModeTab("Things",TAB_THINGS,&bounds_rect,ExternalUpdate);
	}

	CommandMode	=	new	CommandTab;
	if(CommandMode)
	{
		AddTab(CommandMode);
		CommandMode->SetupModeTab("Commands",TAB_COMMANDS,&bounds_rect,ExternalUpdate);
	}

	ConditionMode	=	new	ConditionTab;
	if(ConditionMode)
	{
		AddTab(ConditionMode);
		ConditionMode->SetupModeTab("Conditions",TAB_CONDITIONS,&bounds_rect,ExternalUpdate);
	}
}

//---------------------------------------------------------------

void	GameEditor::DestroyTabs(void)
{
	if(ThingMode)
		delete	ThingMode;

	if(SaveMode)
		delete	SaveMode;
}

//---------------------------------------------------------------

void	GameEditor::DrawContent(void)
{
	EngineStuff		temp_engine;


	// Backup current engine stuff.
	temp_engine	=	engine;
	engine		=	EdEngine;

	SetContentDrawArea();
	ClearContent();

	set_camera();

	GameEdEngine();
	ScanEngine();
	RenderEngine();

	// Restore engine stuff.
	engine	=	temp_engine;
}

//---------------------------------------------------------------

extern	SLONG	calc_height_at(SLONG x,SLONG z);

void	GameEditor::HandleContentClick(UBYTE flags,MFPoint *clicked_point)
{
	UWORD		new_thing;
	SLONG		mappos;


	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:
			SaveMode->SetSaveState(FALSE);
			switch(HilitedItem.ItemType)
			{
				case	ED_ITEM_NONE:
					break;
				case	ED_ITEM_THING:
					switch(SelectMode)
					{
						case	SELECT_COND_TAB_THING:
						case	SELECT_COND_TAB_SWITCH:
							ConditionMode->SetTabData(HilitedItem.ItemRef);
							ClearSelectMode();
							break;
						case	SELECT_THING_TAB_THING:
						case	SELECT_THING_TAB_SWITCH:
							ThingMode->SetTabData(HilitedItem.ItemRef);
							ClearSelectMode();
							break;
						case	SELECT_COM_TAB_WAYPOINT:
							break;
						case	SELECT_COM_TAB_THING:
						case	SELECT_COM_TAB_SWITCH:
							CommandMode->SetTabData(HilitedItem.ItemRef);
							ClearSelectMode();
							break;
						default:
							SelectedItem	=	HilitedItem;
							ThingMode->SetCurrentClass(map_things[SelectedItem.ItemRef].Class);
							ThingMode->SetCurrentGenus(map_things[SelectedItem.ItemRef].Genus);
							ThingMode->SetCurrentThing(SelectedItem.ItemRef);

							DownPoint	=	*clicked_point;
							HandleThingDrag();
					}
					break;
				case	ED_ITEM_MAP_BLOCK:
					if(!SelectMode)
					{
						if	(
								ThingMode->GetCurrentClass()	&&
								ThingMode->GetCurrentGenus()
							)
						{
							mappos	=	HilitedItem.ItemRef;
							new_thing	=	find_empty_map_thing();
							if(new_thing)
							{
								add_thing_to_edit_map(mappos>>8,mappos&0x00ff,new_thing);
 								map_things[new_thing].X		=	(mappos&0xff00)+128;
 								map_things[new_thing].Z		=	((mappos&0x00ff)<<8)+128;
 								map_things[new_thing].Y		=	calc_height_at(map_things[new_thing].X,map_things[new_thing].Z);
 								map_things[new_thing].Type	=	MAP_THING_TYPE_ED_THING;
								map_things[new_thing].Class	=	ThingMode->GetCurrentClass();
								map_things[new_thing].Genus	=	ThingMode->GetCurrentGenus();
								map_things[new_thing].CommandRef	=	0;
								map_things[new_thing].Data[0]		=	256;

								SelectedItem.ItemType	=	ED_ITEM_THING;
								SelectedItem.ItemRef	=	new_thing;
								ThingMode->SetCurrentThing(new_thing);
							}
						}
					}
					break;
				case	ED_ITEM_BUILDING:
					SelectedItem	=	HilitedItem;
					ThingMode->SetCurrentClass(CLASS_BUILDING);
					ThingMode->SetCurrentGenus(0);
					ThingMode->SetCurrentThing(SelectedItem.ItemRef);
					break;
				case	ED_ITEM_WAYPOINT:
					switch(SelectMode)
					{
						case	SELECT_NONE:
							DownPoint	=	*clicked_point;
							HandleWaypointDrag();
							break;
						case	SELECT_WAYPOINT:
							map_things[SelectedItem.ItemRef].CommandRef	=	HilitedItem.ItemRef;
							ClearSelectMode();
							break;
						case	SELECT_NEXT_WAYPOINT:
							link_next_waypoint(SelectedItem.ItemRef,HilitedItem.ItemRef);
							ClearSelectMode();
							break;
						case	SELECT_PREV_WAYPOINT:
							link_prev_waypoint(SelectedItem.ItemRef,HilitedItem.ItemRef);
							ClearSelectMode();
							break;
						case	SELECT_COND_TAB_THING:
						case	SELECT_COND_TAB_SWITCH:
						case	SELECT_THING_TAB_THING:
						case	SELECT_THING_TAB_SWITCH:
							break;
						case	SELECT_COM_TAB_WAYPOINT:
							CommandMode->SetTabData(HilitedItem.ItemRef);
							ClearSelectMode();
							break;
						case	SELECT_COM_TAB_THING:
						case	SELECT_COM_TAB_SWITCH:
							break;
					}
					break;
				case	ED_ITEM_SIZE_HOOK:
					DownPoint	=	*clicked_point;
					HandleSizeDrag();
					break;
			}
			break;
		case	MIDDLE_CLICK:
			break;
		case	RIGHT_CLICK:
			switch(HilitedItem.ItemType)
			{
				case	ED_ITEM_NONE:
					break;
				case	ED_ITEM_THING:
					if(RightButton)
					{
						DoThingPopup(clicked_point);
					}
					break;
				case	ED_ITEM_MAP_BLOCK:
					if(RightButton)
					{
						DoBlockPopup(clicked_point);
					}
					break;
				case	ED_ITEM_BUILDING:
					break;
				case	ED_ITEM_WAYPOINT:
					if(RightButton)
					{
						DoWaypointPopup(clicked_point);
					}
					break;
			}
			break;
	}
}

//---------------------------------------------------------------

void	GameEditor::HandleControlClick(UBYTE flags,MFPoint *clicked_point)
{
	UWORD		control_id;


	if(CurrentModeTab())
	{
		control_id	=	CurrentModeTab()->HandleTabClick(flags,clicked_point);
		switch(CurrentModeTab()->GetTabID())
		{
			case	TAB_NONE:
				break;
			case	TAB_THINGS:
				switch(ThingMode->GetTabMode())
				{
					case	THING_MODE_NONE:
						break;
					case	THING_MODE_SELECT_THING:
						SetSelectMode(SELECT_THING_TAB_THING);
						break;
					case	THING_MODE_SELECT_SWITCH:
						SetSelectMode(SELECT_THING_TAB_SWITCH);
						break;
				}
				break;
			case	TAB_LEVELS:
				break;
			case	TAB_COMMANDS:
				switch(CommandMode->GetTabMode())
				{
					case	COM_MODE_NONE:
						break;
					case	COM_MODE_SELECT_THING:
						SetSelectMode(SELECT_COM_TAB_THING);
						break;
					case	COM_MODE_SELECT_WAYPOINT:
						SetSelectMode(SELECT_COM_TAB_WAYPOINT);
						break;
					case	COM_MODE_SELECT_SWITCH:
						SetSelectMode(SELECT_COM_TAB_SWITCH);
						break;
				}
				break;
			case	TAB_CONDITIONS:
				switch(ConditionMode->GetTabMode())
				{
					case	COND_MODE_NONE:
						break;
					case	COND_MODE_SELECT_THING:
						SetSelectMode(SELECT_COND_TAB_THING);
						break;
					case	COND_MODE_SELECT_SWITCH:
						SetSelectMode(SELECT_COND_TAB_SWITCH);
						break;
				}
				break;
		}
	}
}

//---------------------------------------------------------------

void	GameEditor::HandleModule(void)
{
	BOOL			flash	=	FALSE;
	ULONG			update	=	0;
   	SLONG			dx		=	0,
					dy		=	0,
					dz		=	0;
	MFPoint			mouse_point;
	MFTime			current_time;
	static EditFace	last_face;


	Time(&current_time);
	if(current_time.MSeconds>500)
	{
		flash		=	TRUE;
	}
	if(FlashState!=flash)
	{
		FlashState		=	flash;
		update			=	2;
	}

	mouse_point.X	=	MouseX;
	mouse_point.Y	=	MouseY;
	if(CurrentModeTab())
	{
		CurrentModeTab()->HandleTab(&mouse_point);
	}

	if(PointInContent(&mouse_point))
	{
		update			+=	1;
	}

	if(LastItem.ItemType!=HilitedItem.ItemType || LastItem.ItemRef!=HilitedItem.ItemRef)
	{
		LastItem	=	HilitedItem;
		update				=	2;
	}

	if(LastKey==KB_ESC)
	{
		if(SelectMode)
		{
			ClearSelectMode();
			LastKey		=	0;
		}
	}

	if(EngineKeys())
		update	=	2;

	if(update)
	{
		if(LockWorkScreen())
		{
			DrawContent();
			DrawGrowBox();
			UnlockWorkScreen();
		}
		if(update>1)
			ShowWorkWindow(0);
	}
}

//---------------------------------------------------------------

void	GameEditor::HandleThingDrag(void)
{
	SLONG		new_x,
				new_y,
				new_z,
				x_diff,
				y_diff;


	if(ShiftFlag)
	{
		while(SHELL_ACTIVE && LeftButton)
		{
			new_x		=	map_things[SelectedItem.ItemRef].X;
			new_y		=	map_things[SelectedItem.ItemRef].Y;
			new_z		=	map_things[SelectedItem.ItemRef].Z;
			y_diff		=	(DownPoint.Y-MouseY);

			new_y		+=	(y_diff<<11)/EdEngine.Scale;

			DownPoint.Y	-=	y_diff;
			move_thing_on_cells(SelectedItem.ItemRef,new_x,new_y,new_z);

			if(LockWorkScreen())
			{
				DrawContent();
				DrawGrowBox();
				UnlockWorkScreen();
				ShowWorkWindow(0);
			}
		}
	}
	else
	{
		while(SHELL_ACTIVE && LeftButton)
		{
			new_x		=	map_things[SelectedItem.ItemRef].X;
			new_y		=	map_things[SelectedItem.ItemRef].Y;
			new_z		=	map_things[SelectedItem.ItemRef].Z;
			x_diff		=	(DownPoint.X-MouseX);
			y_diff		=	(DownPoint.Y-MouseY);

			new_x		+=	((SIN( ((EdEngine.AngleY>>8)-512)&2047)*x_diff)>>5)/EdEngine.Scale;
			new_z		+=	((COS( ((EdEngine.AngleY>>8)-512)&2047)*x_diff)>>5)/EdEngine.Scale;
			new_x		+=	((SIN( ((EdEngine.AngleY>>8)+1024)&2047)*y_diff)>>5)/EdEngine.Scale;
			new_z		+=	((COS( ((EdEngine.AngleY>>8)+1024)&2047)*y_diff)>>5)/EdEngine.Scale;

			DownPoint.X	-=	x_diff;
			DownPoint.Y	-=	y_diff;
			move_thing_on_cells(SelectedItem.ItemRef,new_x,new_y,new_z);

			if(LockWorkScreen())
			{
				DrawContent();
				DrawGrowBox();
				UnlockWorkScreen();
				ShowWorkWindow(0);
			}
		}
	}
}

//---------------------------------------------------------------

UWORD	GameEditor::EngineKeys(void)
{
	UBYTE			change	=	0;
	EngineStuff		temp_engine;


	// Backup current engine stuff.
	temp_engine	=	engine;
	engine		=	EdEngine;

	change	|=	engine_keys_scroll_game();
	change	|=	engine_keys_spin();
	change	|=	engine_keys_zoom();

	if(change)
		change	=	2;

	// Restore engine stuff.
	EdEngine	=	engine;
	engine		=	temp_engine;

	return	change;
}

//---------------------------------------------------------------

static ControlDef		block_popup_def	=	{	POPUP_MENU,	0,	""};
MenuDef2	block_popup[]	=
{
	{	"Place Waypoint"	},
	{	"!"					}
};

void	GameEditor::DoBlockPopup(MFPoint *clicked_point)
{
	UWORD			new_wp;
	ULONG			control_id;
	SLONG			mappos;
	CPopUp			*the_control	=	0;
	MFPoint			local_point;


	local_point	=	*clicked_point;
	GlobalToLocal(&local_point);
	block_popup_def.ControlLeft	=	local_point.X+4;
	block_popup_def.ControlTop	=	local_point.Y-4;

	block_popup_def.TheMenuDef	=	block_popup;
	the_control		=	new CPopUp(&block_popup_def);
	control_id		=	the_control->TrackControl(&local_point);

	switch(control_id>>8)
	{
		case	0:
			break;
		case	1:
			new_wp	=	alloc_ed_waypoint();
			if(new_wp)
			{
				mappos	=	HilitedItem.ItemRef;

				edit_waypoints[new_wp].X	=	(mappos&0xff00)+128;
				edit_waypoints[new_wp].Z	=	((mappos&0x00ff)<<8)+128;
				edit_waypoints[new_wp].Y	=	calc_height_at(edit_waypoints[new_wp].X,edit_waypoints[new_wp].Z);
			}
			break;
	}
}

//---------------------------------------------------------------

static ControlDef		thing_popup_def	=	{	POPUP_MENU,	0,	""};
MenuDef2	thing_popup[]	=
{
	{	"Delete Thing"		},
	{	"^"					},
	{	"Snap to Floor"		},
	{	"Snap to Surface"	},
	{	"^"					},
	{	"Link to Waypoint"	},
	{	"!"					}
};

void	GameEditor::DoThingPopup(MFPoint *clicked_point)
{
	ULONG			control_id;
	SLONG			thing_x,
					thing_y,
					thing_z;
	CPopUp			*the_control	=	0;
	MFPoint			local_point;


	local_point	=	*clicked_point;
	GlobalToLocal(&local_point);
	thing_popup_def.ControlLeft	=	local_point.X+4;
	thing_popup_def.ControlTop	=	local_point.Y-4;

	thing_popup_def.TheMenuDef	=	thing_popup;
	the_control		=	new CPopUp(&thing_popup_def);
	control_id		=	the_control->TrackControl(&local_point);

	thing_x	=	map_things[HilitedItem.ItemRef].X;
	thing_y	=	map_things[HilitedItem.ItemRef].Y;
	thing_z	=	map_things[HilitedItem.ItemRef].Z;
	switch(control_id>>8)
	{
		case	0:	// Null.
			break;
		case	1:	// Delete Thing.
			delete_thing(HilitedItem.ItemRef);
			HilitedItem.ItemType	=	ED_ITEM_NONE;
			SelectedItem.ItemType	=	ED_ITEM_NONE;
			break;
		case	2:	// Blank line.
			break;
		case	3:	// Snap to Floor.
			move_thing_on_cells(HilitedItem.ItemRef,thing_x,calc_height_at(thing_x,thing_z),thing_z);
			break;
		case	4:	// Snap to Face.
			// Stick in the snap to face stuff here.
			break;
		case	5:	// Blank line.
			break;
		case	6:	// Link to Waypoint.
			SetSelectMode(SELECT_WAYPOINT);
			break;
	}

	if(the_control)
	{
		delete	the_control;
	}
}

//---------------------------------------------------------------

void	GameEditor::HandleWaypointDrag(void)
{
	SLONG			new_x,
					new_y,
					new_z,
					x_diff,
					y_diff;
	EditWaypoint	*the_wp;


	the_wp	=	&edit_waypoints[HilitedItem.ItemRef];
	if(ShiftFlag)
	{
		while(SHELL_ACTIVE && LeftButton)
		{
			new_y		=	the_wp->Y;
			y_diff		=	(DownPoint.Y-MouseY);
			new_y		+=	(y_diff<<11)/EdEngine.Scale;
			DownPoint.Y	-=	y_diff;
			the_wp->Y	=	new_y;

			if(LockWorkScreen())
			{
				DrawContent();
				DrawGrowBox();
				UnlockWorkScreen();
				ShowWorkWindow(0);
			}
		}
	}
	else
	{
		while(SHELL_ACTIVE && LeftButton)
		{
			new_x		=	the_wp->X;
			new_z		=	the_wp->Z;
			x_diff		=	(DownPoint.X-MouseX);
			y_diff		=	(DownPoint.Y-MouseY);

			new_x		+=	((SIN( ((EdEngine.AngleY>>8)-512)&2047)*x_diff)>>5)/EdEngine.Scale;
			new_z		+=	((COS( ((EdEngine.AngleY>>8)-512)&2047)*x_diff)>>5)/EdEngine.Scale;
			new_x		+=	((SIN( ((EdEngine.AngleY>>8)+1024)&2047)*y_diff)>>5)/EdEngine.Scale;
			new_z		+=	((COS( ((EdEngine.AngleY>>8)+1024)&2047)*y_diff)>>5)/EdEngine.Scale;

			DownPoint.X	-=	x_diff;
			DownPoint.Y	-=	y_diff;

			the_wp->X	=	new_x;
			the_wp->Z	=	new_z;

			if(LockWorkScreen())
			{
				DrawContent();
				DrawGrowBox();
				UnlockWorkScreen();
				ShowWorkWindow(0);
			}
		}
	}
}

//---------------------------------------------------------------

static ControlDef		way_popup_def	=	{	POPUP_MENU,	0,	""};
MenuDef2	way_popup[]	=
{
	{	"Delete Waypoint"		},
	{	"^"						},
	{	"Link to Next"			},
	{	"Link to Previous"		},
	{	"^"						},
	{	"Snap to Floor"			},
	{	"Snap to Surface"		},
	{	"!"						}
};


void	GameEditor::DoWaypointPopup(MFPoint *clicked_point)
{
	UWORD			new_wp;
	ULONG			control_id;
	SLONG			c0,mappos;
	CPopUp			*the_control	=	0;
	MFPoint			local_point;


	local_point	=	*clicked_point;
	GlobalToLocal(&local_point);
	way_popup_def.ControlLeft	=	local_point.X+4;
	way_popup_def.ControlTop	=	local_point.Y-4;

	way_popup_def.TheMenuDef	=	way_popup;
	the_control		=	new CPopUp(&way_popup_def);
	control_id		=	the_control->TrackControl(&local_point);

	switch(control_id>>8)
	{
		case	0:	// NULL.
			break;
		case	1:	// Delete Waypoint.
			// Search through all the things to clear any waypoint references.
			for(c0=1;c0<MAX_MAP_THINGS;c0++)
			{
				if(map_things[c0].Type==MAP_THING_TYPE_ED_THING && map_things[c0].CommandRef==HilitedItem.ItemRef)
				{
					map_things[c0].CommandRef	=	0;
				}
			}
			// Now delete the waypoint.
			free_ed_waypoint(HilitedItem.ItemRef);
			HilitedItem.ItemType	=	ED_ITEM_NONE;
			break;
		case	2:	// Blank Line.
			break;
		case	3:	// Link to Next.
			SetSelectMode(SELECT_NEXT_WAYPOINT);
			break;
		case	4:	// Link to Previous.
			SetSelectMode(SELECT_PREV_WAYPOINT);
			break;
		case	5:	// Blank Line.
			break;
	}
}

//---------------------------------------------------------------

void	GameEditor::HandleSizeDrag(void)
{
	SWORD			switch_ref;
	SLONG			new_rad,
					x_diff,
					y_diff;
	EngineStuff		temp_engine;
	MFPoint			mouse_point;
	SVector			origin,
					point;


	// Backup current engine stuff.
	temp_engine	=	engine;
	engine		=	EdEngine;

	switch_ref	=	HilitedItem.ItemRef;

	set_camera();
	point.X		=	map_things[switch_ref].X;
	point.Y		=	map_things[switch_ref].Y;
	point.Z		=	map_things[switch_ref].Z;
	rotate_point_gte(&point,&origin);

	while(SHELL_ACTIVE && LeftButton)
	{
		mouse_point.X	=	MouseX;
		mouse_point.Y	=	MouseY;
		GlobalToLocal(&mouse_point);

		x_diff		=	mouse_point.X-origin.X;
		y_diff		=	mouse_point.Y-origin.Y;

		map_things[switch_ref].Data[0]	=	(Root((x_diff*x_diff)+(y_diff*y_diff))<<11)/EdEngine.Scale;

		if(LockWorkScreen())
		{
			DrawContent();
			DrawGrowBox();
			UnlockWorkScreen();
			ShowWorkWindow(0);
		}

	}

	// Restore engine stuff.
	engine	=	temp_engine;
}

//---------------------------------------------------------------
// Game Editor Engine.
// A bastardised version of Mikes original editor engine.

#define	MAX_RADIUS	(24)
extern TinyXZ	radius_pool[MAX_RADIUS*4*MAX_RADIUS*2];
extern TinyXZ	*radius_ptr[MAX_RADIUS+2];
SLONG	draw_a_facet_at(UWORD	facet,SLONG x,SLONG y,SLONG z);

void	GameEditor::GameEdEngine(void)
{
	ULONG		buffer_flags[MAX_RADIUS*(MAX_RADIUS+1)*4],
				*ptr_flag;
	SLONG		c0,
				cdx,cdz,
				clip_flags,
				dx,dz,
				flag_and,flag_or,
				mx,my,mz,
				radius;
	MapThing	*p_mthing;
	SVector		base_point,
				new_point,
				point,
				buffer_points[MAX_RADIUS*(MAX_RADIUS+1)*4],
				wp_points[MAX_EDIT_WAYPOINTS],
				*ptr;
	TinyXZ		*ptr_rad;


	current_bucket_pool	=	bucket_pool;

	mx	=	(EdEngine.X>>8)>>ELE_SHIFT;
	my	=	(EdEngine.Y>>8)>>ELE_SHIFT;
	mz	=	(EdEngine.Z>>8)>>ELE_SHIFT;

	// Do the waypoints.
	for(c0=0;c0<MAX_EDIT_WAYPOINTS;c0++)
	{
		if(edit_waypoints[c0].Used)
		{
			if(current_bucket_pool>=end_bucket_pool)
				goto	exit;

			point.X	=	edit_waypoints[c0].X;
			point.Y	=	edit_waypoints[c0].Y;
			point.Z	=	edit_waypoints[c0].Z;
			if(!(rotate_point_gte(&point,&wp_points[c0])&EF_CLIPFLAGS))
			{
				((BucketWaypoint*)current_bucket_pool)->BucketType	=	BT_WAYPOINT;
				((BucketWaypoint*)current_bucket_pool)->X			=	wp_points[c0].X;
				((BucketWaypoint*)current_bucket_pool)->Y			=	wp_points[c0].Y;
				((BucketWaypoint*)current_bucket_pool)->EditRef.ItemType	=	ED_ITEM_WAYPOINT;
				((BucketWaypoint*)current_bucket_pool)->EditRef.ItemRef		=	c0;

				base_point		=	point;
				base_point.Y	=	calc_height_at(base_point.X,base_point.Z);
				rotate_point_gte(&base_point,&new_point);
				((BucketWaypoint*)current_bucket_pool)->BaseX		=	new_point.X;
				((BucketWaypoint*)current_bucket_pool)->BaseY		=	new_point.Y;

				add_bucket((void *)current_bucket_pool,new_point.Z-300);
				current_bucket_pool	+=	sizeof(BucketWaypoint);
			}
		}
	}

	// Do the waypoint connection lines.
	for(c0=0;c0<MAX_EDIT_WAYPOINTS;c0++)
	{
		if(edit_waypoints[c0].Used && edit_waypoints[c0].Next)
		{
			if(current_bucket_pool>=end_bucket_pool)
				goto	exit;

			((BucketLine*)current_bucket_pool)->BucketType	=	BT_LINE;
			((BucketLine*)current_bucket_pool)->X1			=	wp_points[c0].X;
			((BucketLine*)current_bucket_pool)->Y1			=	wp_points[c0].Y;
			((BucketLine*)current_bucket_pool)->X2			=	wp_points[edit_waypoints[c0].Next].X;
			((BucketLine*)current_bucket_pool)->Y2			=	wp_points[edit_waypoints[c0].Next].Y;

			add_bucket((void *)current_bucket_pool,1);
			current_bucket_pool	+=	sizeof(BucketLine);
		}
	}


	memset((UBYTE*)buffer_flags,0,MAX_RADIUS*(MAX_RADIUS+1)*4*4);
	ptr		=	&buffer_points[MAX_RADIUS+MAX_RADIUS*MAX_RADIUS*2];

	point.X	=	(mx<<ELE_SHIFT);
	point.Y	=	edit_map[mx][mz].Y<<FLOOR_HEIGHT_SHIFT;
	point.Z	=	(mz<<ELE_SHIFT);

	clip_flags	=	rotate_point_gte(&point,ptr);
	buffer_flags[MAX_RADIUS+MAX_RADIUS*MAX_RADIUS*2]	=	clip_flags;

	// Transform all the vertices for the current view.
	for(radius=1;radius<MAX_RADIUS;radius++)
	{
		ptr_rad	=	radius_ptr[radius];
		while(ptr_rad<radius_ptr[radius-1])
		{
			cdx	=	ptr_rad->Dx;
			cdz	=	ptr_rad->Dz;

			dx	=	(cdx*ELE_SIZE);
			dz	=	(cdz*ELE_SIZE);

			cdx	+=	MAX_RADIUS;
			cdz	+=	MAX_RADIUS;

			ptr	=	&buffer_points[cdx+cdz*MAX_RADIUS*2];

			point.X	=	dx+(mx<<ELE_SHIFT);
			point.Y	=	edit_map[cdx+mx-MAX_RADIUS][cdz+mz-MAX_RADIUS].Y<<FLOOR_HEIGHT_SHIFT;
			point.Z	=	dz+(mz<<ELE_SHIFT); //(engine.Z>>8);

			clip_flags	=	rotate_point_gte(&point,ptr);
//			if( ((clip_flags&EF_BEHIND_YOU)==0) && !(clip_flags & EF_CLIPFLAGS))
			{
				buffer_flags[cdx+cdz*MAX_RADIUS*2]	=	clip_flags;
			}
			ptr_rad++;
		}
	}


	ptr_flag	=	buffer_flags;
	for(dz=0;dz<MAX_RADIUS*2;dz++)
	for(dx=0;dx<MAX_RADIUS*2;dx++)
	{
		if(dx+mx-MAX_RADIUS>0&&dx+mx-MAX_RADIUS<EDIT_MAP_WIDTH&&dz+mz-MAX_RADIUS>0&&dz+mz-MAX_RADIUS<EDIT_MAP_DEPTH)
		{
			// Do the map who.
			CurrentThing	=	edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)].MapThingIndex;
			while(CurrentThing)
			{
				p_mthing		=	TO_MTHING(CurrentThing);
				switch(p_mthing->Type)
				{
					case	MAP_THING_TYPE_PRIM:
						break;
					case	MAP_THING_TYPE_BUILDING:
						if(ThingMode->GetThingFlags()&(1<<CLASS_BUILDING))
						{
							UWORD		facet_index;
							SLONG		az,best_z;


							facet_index	=	building_objects[p_mthing->IndexOther].FacetHead;
							while(facet_index)
							{
								az	=	DrawFacet	(
														facet_index,
														p_mthing->X,p_mthing->Y,p_mthing->Z
													);
								if(best_z<az)
									best_z	=	az;
								facet_index	=	building_facets[facet_index].NextFacet;
							}
						}
						break;
					case	MAP_THING_TYPE_MULTI_PRIM:
						break;
					case	MAP_THING_TYPE_ROT_MULTI:
						break;
					case	MAP_THING_TYPE_ED_THING:
						if(current_bucket_pool>=end_bucket_pool)
							goto	exit;

						point.X	=	p_mthing->X;
						point.Y	=	p_mthing->Y;
						point.Z	=	p_mthing->Z;

						base_point	=	point;
						base_point.Y	=	calc_height_at(base_point.X,base_point.Z);
						rotate_point_gte(&base_point,&new_point);
						((BucketMapThing*)current_bucket_pool)->BaseX		=	new_point.X;
						((BucketMapThing*)current_bucket_pool)->BaseY		=	new_point.Y;

						rotate_point_gte(&point,&new_point);
						((BucketMapThing*)current_bucket_pool)->BucketType	=	BT_MAP_THING;
						((BucketMapThing*)current_bucket_pool)->X			=	new_point.X;
						((BucketMapThing*)current_bucket_pool)->Y			=	new_point.Y;
						((BucketMapThing*)current_bucket_pool)->EditRef.ItemType	=	ED_ITEM_THING;
						((BucketMapThing*)current_bucket_pool)->EditRef.ItemRef		=	CurrentThing;

						add_bucket((void *)current_bucket_pool,new_point.Z-300);
						current_bucket_pool	+=	sizeof(BucketMapThing);

						// Do the size thing for switches.
						if(p_mthing->Class==CLASS_SWITCH)
						{
							if(current_bucket_pool>=end_bucket_pool)
								goto	exit;

							((BucketSphereArea*)current_bucket_pool)->BucketType	=	BT_SPHERE_AREA;
							((BucketSphereArea*)current_bucket_pool)->X				=	new_point.X;
							((BucketSphereArea*)current_bucket_pool)->Y				=	new_point.Y;
							((BucketSphereArea*)current_bucket_pool)->Radius		=	(p_mthing->Data[0]*EdEngine.Scale)>>11;
							((BucketSphereArea*)current_bucket_pool)->ShowSizeHook	=	TRUE;
							((BucketSphereArea*)current_bucket_pool)->EditRef.ItemType	=	ED_ITEM_SIZE_HOOK;
							((BucketSphereArea*)current_bucket_pool)->EditRef.ItemRef	=	CurrentThing;

							add_bucket((void *)current_bucket_pool,1);
							current_bucket_pool	+=	sizeof(BucketSphereArea);
						}

						// Do the link lines.
						if(p_mthing->CommandRef)
						{
							if(current_bucket_pool>=end_bucket_pool)
								goto	exit;

							((BucketLine*)current_bucket_pool)->BucketType	=	BT_LINE;
							((BucketLine*)current_bucket_pool)->X1			=	new_point.X;
							((BucketLine*)current_bucket_pool)->Y1			=	new_point.Y;

							((BucketLine*)current_bucket_pool)->X2			=	wp_points[p_mthing->CommandRef].X;
							((BucketLine*)current_bucket_pool)->Y2			=	wp_points[p_mthing->CommandRef].Y;

							add_bucket((void *)current_bucket_pool,1);
							current_bucket_pool	+=	sizeof(BucketLine);
						}
						break;
				}
				CurrentThing	=	map_things[CurrentThing].MapChild;
			}

			// Do the map floor.
			if(!(edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)].Flags&FLOOR_HIDDEN) || !(ThingMode->GetThingFlags()&(1<<CLASS_BUILDING)))
			{
				flag_and=(*ptr_flag) & (*(ptr_flag+1)) & (*(ptr_flag+MAX_RADIUS*2)) & (*(ptr_flag+1+MAX_RADIUS*2));
				flag_or =(*ptr_flag) | (*(ptr_flag+1)) | (*(ptr_flag+MAX_RADIUS*2)) | (*(ptr_flag+1+MAX_RADIUS*2));

				if( ((flag_or&EF_BEHIND_YOU)==0) && !(flag_and & EF_CLIPFLAGS) && (flag_and & EF_TRANSLATED))
				{
					UBYTE			tx,ty,tsize,page;
					SLONG			az;
					DepthStrip		*p_map;


					if(current_bucket_pool>=end_bucket_pool)
						goto	exit;

					CurrentThing	=	(((dx+mx-MAX_RADIUS)<<8)|(dz+mz-MAX_RADIUS));

					az	=	buffer_points[dx+dz*MAX_RADIUS*2].Z;

					setPolyType4(
									current_bucket_pool,
									POLY_GT
								);

					setCol4	(
								(struct BucketQuad*)current_bucket_pool,
								1
							);

					setXY4	(
								(struct BucketQuad*)current_bucket_pool,
										buffer_points[dx+dz*MAX_RADIUS*2].X,buffer_points[dx+dz*MAX_RADIUS*2].Y,
										buffer_points[dx+dz*MAX_RADIUS*2+1].X,buffer_points[dx+dz*MAX_RADIUS*2+1].Y,
										buffer_points[dx+(dz+1)*MAX_RADIUS*2].X,buffer_points[dx+(dz+1)*MAX_RADIUS*2].Y,
										buffer_points[dx+(dz+1)*MAX_RADIUS*2+1].X,buffer_points[dx+(dz+1)*MAX_RADIUS*2+1].Y
							);

					p_map	=	&edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)];
					tx		=	((struct	MiniTextureBits*)(&p_map->Texture))->X<<5;
					ty		=	((struct	MiniTextureBits*)(&p_map->Texture))->Y<<5;
					page	=	((struct	MiniTextureBits*)(&p_map->Texture))->Page;
					tsize	=	31;//floor_texture_sizes[((struct	MiniTextureBits*)(&p_map->Texture))->Size]-1;
					switch(((struct	MiniTextureBits*)(&p_map->Texture))->Rot)
					{
						case	0:
							setUV4(	(struct BucketQuad*)current_bucket_pool,tx,ty,tx+tsize,ty,tx,ty+tsize,tx+tsize,ty+tsize,page);
							break;
						case	1:
							setUV4(	(struct BucketQuad*)current_bucket_pool,tx+tsize,ty,tx+tsize,ty+tsize,tx,ty,tx,ty+tsize,page);
							break;
						case	2:
							setUV4(	(struct BucketQuad*)current_bucket_pool,tx+tsize,ty+tsize,tx,ty+tsize,tx+tsize,ty,tx,ty,page);
							break;
						case	3:
							setUV4(	(struct BucketQuad*)current_bucket_pool,tx,ty+tsize,tx,ty,tx+tsize,ty+tsize,tx+tsize,ty,page);
							break;
					}

					setZ4((struct BucketQuad*)current_bucket_pool,az,0,0,0);

					setShade4	((BucketQuad*)current_bucket_pool,
									edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS)].Bright,
									edit_map[(dx+mx-MAX_RADIUS+1)][(dz+mz-MAX_RADIUS)].Bright,
									edit_map[(dx+mx-MAX_RADIUS)][(dz+mz-MAX_RADIUS+1)].Bright,
									edit_map[(dx+mx-MAX_RADIUS+1)][(dz+mz-MAX_RADIUS+1)].Bright
								);

					((BucketQuad*)current_bucket_pool)->DebugInfo	=	az;
					((BucketQuad*)current_bucket_pool)->DebugFlags	=	0;
					((BucketQuad*)current_bucket_pool)->EditRef.ItemType	=	ED_ITEM_MAP_BLOCK;
					((BucketQuad*)current_bucket_pool)->EditRef.ItemRef		=	CurrentThing;
					add_bucket((void *)current_bucket_pool,az+300);

					current_bucket_pool	+=	sizeof(struct BucketQuad);
				}
			}

		}
		ptr_flag++;
	}
exit:;
}

//---------------------------------------------------------------

SLONG	GameEditor::DrawFacet(UWORD facet_index,SLONG x,SLONG y,SLONG z)
{
	ULONG			flag_and,flag_or;
	SLONG			az;
	SLONG			col=0,
					cor=0,
					cob=0,
					cot=0,
					total=0;
	SLONG			first_face=1;
	SLONG			best_z		=	9999999,
					min_z		=	9999999,
					max_z		=	9999999,
					c0,
					facet_flags,
					sp,mp,ep;
	BuildingFacet	*p_facet;
	PrimFace3		*p_f3;
	PrimFace4		*p_f4;


	p_facet     =	&building_facets[facet_index];
	facet_flags =	p_facet->FacetFlags;
	p_f4        =	&prim_faces4[p_facet->StartFace4];
	p_f3        =	&prim_faces3[p_facet->StartFace3];
	if(facet_flags&FACET_FLAG_ROOF)
	{
		first_face	=	0;
	}

	sp	=	p_facet->StartPoint;
	mp	=	p_facet->MidPoint;
	ep	=	p_facet->EndPoint;

	engine.X	-=	x<<8;
	engine.Y	-=	y<<8;
	engine.Z	-=	z<<8;

	for(c0=sp;c0<mp;c0++)
	{
		struct	SVector	pp;
		pp.X=prim_points[c0].X;
		pp.Y=prim_points[c0].Y;
		pp.Z=prim_points[c0].Z;

		//transform all points for this Object
		global_flags[c0-sp]	=	rotate_point_gte(&pp,&global_res[c0-sp]);
		global_bright[c0-sp]=	calc_lights(x,y,z,&pp);

		if(min_z>global_res[c0-sp].Z)
			min_z	=	global_res[c0-sp].Z;
		if(max_z<global_res[c0-sp].Z)
			max_z	=	global_res[c0-sp].Z;
	}
	for(c0=mp;c0<ep;c0++)
	{
		//transform all points for this Object
		global_flags[c0-sp]=rotate_point_gte((struct SVector*)&prim_points[c0],&global_res[c0-sp]);
		global_bright[c0-sp]=calc_lights(x,y,z,(struct SVector*)&prim_points[c0]);
	}
	engine.X	+=	x<<8;
	engine.Y	+=	y<<8;
	engine.Z	+=	z<<8;
	best_z		=	min_z;

	if(p_facet->EndFace4)
	for(c0=p_facet->StartFace4;c0<p_facet->EndFace4;c0++)
	{
		SLONG	p0,p1,p2,p3;

		if(current_bucket_pool>=end_bucket_pool)
			goto	exit;

		p0	=	p_f4->Points[0]-sp;
		p1	=	p_f4->Points[1]-sp;
		p2	=	p_f4->Points[2]-sp;
		p3	=	p_f4->Points[3]-sp;


		flag_and	=	global_flags[p0]&global_flags[p1]&global_flags[p2]&global_flags[p3];
		flag_or		=	global_flags[p0]|global_flags[p1]|global_flags[p2]|global_flags[p3];

		if( (!(flag_and & EF_CLIPFLAGS))&&((flag_or&EF_BEHIND_YOU)==0))
		{
			az	=	(	global_res[p0].Z	+
						global_res[p1].Z	+
						global_res[p2].Z	+
						global_res[p3].Z	)	/	4;

			setPolyType4(
							current_bucket_pool,
							p_f4->DrawFlags
						);

			setCol4	(
						(struct BucketQuad*)current_bucket_pool,
						((UBYTE)p_f4->Col2)
					);

			setXY4	(
						(struct BucketQuad*)current_bucket_pool,
						global_res[p0].X,global_res[p0].Y,
						global_res[p1].X,global_res[p1].Y,
						global_res[p2].X,global_res[p2].Y,
						global_res[p3].X,global_res[p3].Y
					);

//RUD
			if(p_f4->DrawFlags&POLY_FLAG_TEXTURED)
			{
				setUV4	(
						(struct BucketQuad*)current_bucket_pool,
						p_f4->UV[0][0],p_f4->UV[0][1],
						p_f4->UV[1][0],p_f4->UV[1][1],
						p_f4->UV[2][0],p_f4->UV[2][1],
						p_f4->UV[3][0],p_f4->UV[3][1],
						(UBYTE)p_f4->TexturePage
						);
			}
			setZ4((struct BucketQuad*)current_bucket_pool,global_res[p0].Z,global_res[p1].Z,global_res[p2].Z,global_res[p3].Z);

			setShade4((struct BucketQuad*)current_bucket_pool,
			CLIP256(p_f4->Bright[0]+global_bright[p0]),
			CLIP256(p_f4->Bright[1]+global_bright[p1]),
			CLIP256(p_f4->Bright[2]+global_bright[p2]),
			CLIP256(p_f4->Bright[3]+global_bright[p3]));

			((BucketQuad*)current_bucket_pool)->DebugInfo	=	az;
			((BucketQuad*)current_bucket_pool)->DebugFlags	=	p_f4->FaceFlags;
			((BucketQuad*)current_bucket_pool)->EditRef.ItemType	=	ED_ITEM_BUILDING;
			((BucketQuad*)current_bucket_pool)->EditRef.ItemRef		=	CurrentThing;


			add_bucket((void *)current_bucket_pool,az);
			current_bucket_pool	+=	sizeof(struct BucketQuad);
		}
		else
		{
			if(flag_and&EF_OFF_LEFT)
				col++;
			if(flag_and&EF_OFF_RIGHT)
				cor++;
			if(flag_and&EF_OFF_TOP)
				cot++;
			if(flag_and&EF_OFF_BOTTOM)
				cob++;
		}

		p_f4++;
	}

	if(p_facet->EndFace3)
	for(c0=p_facet->StartFace3;c0<p_facet->EndFace3;c0++)
	{
		SLONG	p0,p1,p2;

		if(current_bucket_pool>=end_bucket_pool)
				goto	exit;


		p0	=	p_f3->Points[0]-sp;
		p1	=	p_f3->Points[1]-sp;
		p2	=	p_f3->Points[2]-sp;

		flag_and	=	global_flags[p0]&global_flags[p1]&global_flags[p2];
		flag_or		=	global_flags[p0]|global_flags[p1]|global_flags[p2];

		if((flag_or&EF_BEHIND_YOU)==0)
		if(!(flag_and & EF_CLIPFLAGS))
		{
			az	=	(	global_res[p0].Z	+
						global_res[p1].Z	+
						global_res[p2].Z	)	/	3;

			setPolyType3(
							current_bucket_pool,
							p_f3->DrawFlags
						);

			setCol3	(
						(struct BucketTri*)current_bucket_pool,
						p_f3->Col2
					);

			setXY3	(
						(struct BucketTri*)current_bucket_pool,
						global_res[p0].X,global_res[p0].Y,
						global_res[p1].X,global_res[p1].Y,
						global_res[p2].X,global_res[p2].Y
					);

//RUD
			if(p_f3->DrawFlags&POLY_FLAG_TEXTURED)
			{
				setUV3	(
							(struct BucketTri*)current_bucket_pool,
							p_f3->UV[0][0],p_f3->UV[0][1],
							p_f3->UV[1][0],p_f3->UV[1][1],
							p_f3->UV[2][0],p_f3->UV[2][1],
							p_f3->TexturePage
						);
			}

			setShade3((struct BucketTri*)current_bucket_pool,
				CLIP256(p_f3->Bright[0]+global_bright[p0]),
				CLIP256(p_f3->Bright[1]+global_bright[p1]),
				CLIP256(p_f3->Bright[2]+global_bright[p2]));

			setZ3((struct BucketQuad*)current_bucket_pool,global_res[p0].Z,global_res[p1].Z,global_res[p2].Z);

			((BucketTri*)current_bucket_pool)->DebugInfo	=	c0;
			((BucketTri*)current_bucket_pool)->DebugFlags	=	p_f3->FaceFlags;
			((BucketTri*)current_bucket_pool)->EditRef.ItemType	=	ED_ITEM_BUILDING;
			((BucketTri*)current_bucket_pool)->EditRef.ItemRef	=	CurrentThing;

			add_bucket((void *)current_bucket_pool,az);
			current_bucket_pool	+=	sizeof(struct BucketQuad);
		}
		p_f3++;
	}
exit:;

	 return(best_z);
}

//---------------------------------------------------------------

#define	Z_NORMAL(pa,pb)		((mouse_point.X-(pb).X)*((pb).Y-(pa).Y)-(mouse_point.Y-(pb).Y)*((pb).X-(pa).X))

// This is mad, however it works & it's only the editor after all.
void	GameEditor::ScanEngine(void)
{
	void				*bucket;
	ULONG				c0,
						offset_x,
						offset_y;
	BucketHead			*p;
	BucketMapThing		*the_map_thing;
	BucketQuad			*the_quad;
	BucketSphereArea	*the_sphere;
	BucketTri			*the_tri;
	BucketWaypoint		*the_waypoint;
	MapThing			*t_mthing;
	MFPoint				mouse_point;


	HilitedItem.ItemType	=	ED_ITEM_NONE;

	mouse_point.X	=	MouseX;
	mouse_point.Y	=	MouseY;
	if(PointInContent(&mouse_point))
	{
		GlobalXYToLocal(&mouse_point.X,&mouse_point.Y);

		p	=	&bucket_heads[engine.BucketSize-1];
		for(c0=0;c0<engine.BucketSize;c0++)
		{
			bucket	=	p->BucketPtr;
			{
				while(bucket)
				{
					switch(((BucketGeneric*)bucket)->BucketType)
					{
						case	BT_QUAD:
							the_quad	=	(BucketQuad*)bucket;
							if(the_quad->DrawFlags&POLY_FLAG_DOUBLESIDED)
							{
								if	(
										(Z_NORMAL(the_quad->P[0],the_quad->P[1]))>0	&&
										(Z_NORMAL(the_quad->P[1],the_quad->P[3]))>0	&&
										(Z_NORMAL(the_quad->P[3],the_quad->P[2]))>0	&&
										(Z_NORMAL(the_quad->P[2],the_quad->P[0]))>0
									)
								{
									HilitedItem	=	the_quad->EditRef;
								}
							}
							if	(
									(Z_NORMAL(the_quad->P[0],the_quad->P[1]))<0	&&
									(Z_NORMAL(the_quad->P[1],the_quad->P[3]))<0	&&
									(Z_NORMAL(the_quad->P[3],the_quad->P[2]))<0	&&
									(Z_NORMAL(the_quad->P[2],the_quad->P[0]))<0
								)
							{
								HilitedItem	=	the_quad->EditRef;
							}
							break;
						case	BT_TRI:
							the_tri	=	(BucketTri*)bucket;
							if(the_tri->DrawFlags&POLY_FLAG_DOUBLESIDED)
							{
								if	(
										(Z_NORMAL(the_tri->P[0],the_tri->P[1]))>0	&&
										(Z_NORMAL(the_tri->P[1],the_tri->P[2]))>0	&&
										(Z_NORMAL(the_tri->P[2],the_tri->P[0]))>0
									)
								{
									HilitedItem	=	the_tri->EditRef;
								}
							}
							if	(
									(Z_NORMAL(the_tri->P[0],the_tri->P[1]))<0	&&
									(Z_NORMAL(the_tri->P[1],the_tri->P[2]))<0	&&
									(Z_NORMAL(the_tri->P[2],the_tri->P[0]))<0
								)
							{
								HilitedItem	=	the_tri->EditRef;
							}
							break;
						case	BT_VECT:
							break;
						case	BT_RECT:
							break;
						case	BT_MAP_THING:
							the_map_thing	=	(BucketMapThing*)bucket;
							if	(
									mouse_point.X>=the_map_thing->X-(THING_BOX_SIZE>>1)	&&
									mouse_point.X<the_map_thing->X+(THING_BOX_SIZE>>1)	&&
									mouse_point.Y>=the_map_thing->Y-(THING_BOX_SIZE>>1)	&&
									mouse_point.Y<the_map_thing->Y+(THING_BOX_SIZE>>1)
								)
							{
								t_mthing	=	TO_MTHING(the_map_thing->EditRef.ItemRef);
								if(!SelectMode || SelectMode==SELECT_COND_TAB_THING || SelectMode==SELECT_THING_TAB_THING || SelectMode==SELECT_COM_TAB_THING)
									HilitedItem	=	the_map_thing->EditRef;
								else if((SelectMode==SELECT_COND_TAB_SWITCH || SelectMode==SELECT_COM_TAB_SWITCH || SelectMode==SELECT_THING_TAB_SWITCH) && t_mthing->Class==CLASS_SWITCH)
									HilitedItem	=	the_map_thing->EditRef;
							}
							break;
						case	BT_WAYPOINT:
							the_waypoint	=	(BucketWaypoint*)bucket;
							if	(
									mouse_point.X>=the_waypoint->X-(WP_BOX_SIZE>>1)	&&
									mouse_point.X<the_waypoint->X+(WP_BOX_SIZE>>1)	&&
									mouse_point.Y>=the_waypoint->Y-(WP_BOX_SIZE>>1)	&&
									mouse_point.Y<the_waypoint->Y+(WP_BOX_SIZE>>1)
								)
							{
								HilitedItem	=	the_waypoint->EditRef;
							}
							break;
						case	BT_LINE:
							break;
						case	BT_SPHERE_AREA:
							the_sphere		=	(BucketSphereArea*)bucket;
							offset_x		=	((SIN(256)*the_sphere->Radius)>>16)+the_sphere->X;
							offset_y		=	(-(COS(256)*the_sphere->Radius)>>16)+the_sphere->Y;
							if	(
									the_sphere->ShowSizeHook					&&
									mouse_point.X>=offset_x-(SH_BOX_SIZE>>1)	&&
									mouse_point.X<offset_x+(SH_BOX_SIZE>>1)		&&
									mouse_point.Y>=offset_y-(SH_BOX_SIZE>>1)	&&
									mouse_point.Y<offset_y+(SH_BOX_SIZE>>1)
								)
							{
								HilitedItem	=	the_sphere->EditRef;
							}
							break;
						case	BT_RECT_AREA:
							break;
					}
					bucket	=	((BucketGeneric*)bucket)->BucketPtr;
				}
			}
			p--;
		}
	}
}

//---------------------------------------------------------------

extern SLONG	view_mode;
void	draw_quad_bucket(struct BucketQuad *p_b,SLONG z);

void	GameEditor::RenderEngine(void)
{
	void				*bucket;
	ULONG				c0,
						draw_colour,
						offset_x,
						offset_y;
	BucketHead			*p;
	BucketLine			*the_line;
	BucketMapThing		*hilited_thing	=	NULL,
						*selected_thing	=	NULL,
						*the_map_thing;
	BucketQuad			*the_quad;
	BucketRectArea		*the_rect;
	BucketSphereArea	*hilited_sphere	=	NULL,
						*the_sphere;
	BucketTri			*the_tri;
	BucketWaypoint		*hilited_waypoint,
						*selected_wp,
						*the_waypoint;
	MFPoint				local_point;


	switch(WorkScreenDepth)
	{
		case 1:
			render_span=render_span8;
			break;
		case 2:
			render_span=render_span16;
			break;
		case 4:
			render_span=render_span32;
			break;

	}

	p	=	&bucket_heads[engine.BucketSize-1];
	for(c0=0;c0<engine.BucketSize;c0++)
	{
		bucket	=	p->BucketPtr;
		{
			p->BucketPtr	=	0;
			while(bucket)
			{
				switch(((BucketGeneric*)bucket)->BucketType)
				{
					case	BT_QUAD:
						the_quad	=	(BucketQuad*)bucket;
						if	(
								the_quad->EditRef.ItemType==SelectedItem.ItemType	&&
								the_quad->EditRef.ItemRef==SelectedItem.ItemRef		&&
								FlashState
							)
						{
							poly_info.DrawFlags	=	the_quad->DrawFlags&(POLY_FLAG_DOUBLESIDED);
							poly_info.Col		=	ACTIVE_COL;
						}
						else if(
									the_quad->EditRef.ItemType==HilitedItem.ItemType	&&
									the_quad->EditRef.ItemRef==HilitedItem.ItemRef		&&
									SelectMode	==	0
								)
						{
							poly_info.DrawFlags	=	the_quad->DrawFlags&(POLY_FLAG_DOUBLESIDED);
							poly_info.Col		=	HILITE_COL;
						}
						else
						{
							if(the_quad->DrawFlags&POLY_FLAG_TEXTURED)
							{
								poly_info.PTexture	=	tmaps[the_quad->TextPage]; //OOR
								poly_info.Page		=	the_quad->TextPage;
							}
							poly_info.DrawFlags	=	the_quad->DrawFlags;
							poly_info.Col		=	the_quad->Col;
						}

						if(the_quad->DrawFlags&POLY_FLAG_DOUBLESIDED)
							my_quad_noz(&the_quad->P[2],&the_quad->P[3],&the_quad->P[1],&the_quad->P[0]);
						my_quad_noz(&the_quad->P[0],&the_quad->P[1],&the_quad->P[3],&the_quad->P[2]);
						break;
					case	BT_TRI:
						the_tri	=	(BucketTri*)bucket;
						if	(
								the_tri->EditRef.ItemType==SelectedItem.ItemType	&&
								the_tri->EditRef.ItemRef==SelectedItem.ItemRef		&&
								FlashState
							)
						{
							poly_info.DrawFlags	=	the_tri->DrawFlags&(POLY_FLAG_DOUBLESIDED);
							poly_info.Col		=	ACTIVE_COL;
						}
						else if	(
									the_tri->EditRef.ItemType==HilitedItem.ItemType	&&
									the_tri->EditRef.ItemRef==HilitedItem.ItemRef	&&
									SelectMode	==	0
								)
						{
							poly_info.DrawFlags	=	the_tri->DrawFlags&(POLY_FLAG_DOUBLESIDED);
							poly_info.Col		=	HILITE_COL;
						}
						else
						{
							if(the_tri->DrawFlags&POLY_FLAG_TEXTURED)
							{
								poly_info.PTexture	=	tmaps[the_tri->TextPage]; //OOR
								poly_info.Page		=	the_tri->TextPage;
							}
							poly_info.DrawFlags	=	the_tri->DrawFlags;
							poly_info.Col		=	the_tri->Col;
						}
						if(the_tri->DrawFlags&POLY_FLAG_DOUBLESIDED)
							my_trig_noz(&the_tri->P[2],&the_tri->P[1],&the_tri->P[0]);
						my_trig_noz(&the_tri->P[0],&the_tri->P[1],&the_tri->P[2]);
						break;
					case	BT_VECT:
						break;
					case	BT_RECT:
						break;
					case	BT_MAP_THING:
						the_map_thing	=	(BucketMapThing*)bucket;
						DrawVLineC(the_map_thing->BaseX,the_map_thing->Y,the_map_thing->BaseY,0xffff);
						draw_colour	=	RED_COL;

						if	(
								the_map_thing->EditRef.ItemType==HilitedItem.ItemType	&&
								the_map_thing->EditRef.ItemRef==HilitedItem.ItemRef		&&
								(SelectMode	==	0 || (SelectMode>=SELECT_COND_TAB_THING && SelectMode<=SELECT_THING_TAB_SWITCH) || (SelectMode>=SELECT_COM_TAB_THING && SelectMode<=SELECT_COM_TAB_SWITCH))
							)
						{
							draw_colour	=	HILITE_COL;
							hilited_thing	=	the_map_thing;
						}
						else if	(
									the_map_thing->EditRef.ItemType==SelectedItem.ItemType	&&
									the_map_thing->EditRef.ItemRef==SelectedItem.ItemRef
								)
						{
							if(FlashState)
								draw_colour	=	ACTIVE_COL;
							selected_thing	=	the_map_thing;
						}

						DrawBoxC(
									the_map_thing->X-(THING_BOX_SIZE>>1),
									the_map_thing->Y-(THING_BOX_SIZE>>1),
									THING_BOX_SIZE,
									THING_BOX_SIZE,
									draw_colour
								);

						MapText	(
									the_map_thing->X+(THING_BOX_SIZE>>1)+2,
									the_map_thing->Y-(THING_BOX_SIZE>>1),
									genus_text[map_things[the_map_thing->EditRef.ItemRef].Class][map_things[the_map_thing->EditRef.ItemRef].Genus],
									0xffff
								);
						break;
					case	BT_WAYPOINT:
						the_waypoint	=	(BucketWaypoint*)bucket;
						DrawVLineC(the_waypoint->BaseX,the_waypoint->Y,the_waypoint->BaseY,0xffff);
						draw_colour	=	GREEN_COL;

						if	(
								the_waypoint->EditRef.ItemType==HilitedItem.ItemType	&&
								the_waypoint->EditRef.ItemRef==HilitedItem.ItemRef		&&
								(SelectMode==0 || SelectMode<=SELECT_PREV_WAYPOINT || SelectMode==SELECT_COM_TAB_WAYPOINT)
							)
						{
							draw_colour			=	HILITE_COL;
							hilited_waypoint	=	the_waypoint;
						}
						else if	(
									the_waypoint->EditRef.ItemType==SelectedItem.ItemType	&&
									the_waypoint->EditRef.ItemRef==SelectedItem.ItemRef
								)
						{
							if(FlashState)
								draw_colour	=	ACTIVE_COL;
							selected_wp	=	the_waypoint;
						}
						DrawBoxC(
									the_waypoint->X-(WP_BOX_SIZE>>1),
									the_waypoint->Y-(WP_BOX_SIZE>>1),
									WP_BOX_SIZE,
									WP_BOX_SIZE,
									draw_colour
								);
						break;
					case	BT_LINE:
						the_line		=	(BucketLine*)bucket;
						DrawLineC(the_line->X1,the_line->Y1,the_line->X2,the_line->Y2,0xffff);
						break;
					case	BT_SPHERE_AREA:
						the_sphere		=	(BucketSphereArea*)bucket;
						draw_colour		=	BLUE_COL;

						offset_x		=	((SIN(256)*the_sphere->Radius)>>16)+the_sphere->X;
						offset_y		=	(-(COS(256)*the_sphere->Radius)>>16)+the_sphere->Y;

						if	(
								the_sphere->EditRef.ItemType==HilitedItem.ItemType		&&
								the_sphere->EditRef.ItemRef==HilitedItem.ItemRef		&&
								SelectMode	==	0
							)
						{
							draw_colour		=	HILITE_COL;
							hilited_sphere	=	the_sphere;
						}
						DrawCircleC(the_sphere->X,the_sphere->Y,the_sphere->Radius,BLUE_COL);
						DrawBoxC(
									offset_x-(SH_BOX_SIZE>>1),
									offset_y-(SH_BOX_SIZE>>1),
									SH_BOX_SIZE,
									SH_BOX_SIZE,
									draw_colour
								);
						break;
					case	BT_RECT_AREA:

						break;
				}
				bucket	=	((BucketGeneric*)bucket)->BucketPtr;
			}
		}
		p--;
	}

	// Draw the info for a hilited item.
	switch(HilitedItem.ItemType)
	{
		case	ED_ITEM_NONE:
			break;
		case	ED_ITEM_THING:
			if(hilited_thing)
			{
				MapThingInfo(
								hilited_thing->X+(THING_BOX_SIZE>>1)+2,
								hilited_thing->Y-(THING_BOX_SIZE>>1)+12,
								hilited_thing
							);
			}
			break;
		case	ED_ITEM_MAP_BLOCK:
			break;
		case	ED_ITEM_BUILDING:
			break;
		case	ED_ITEM_WAYPOINT:
			if(hilited_waypoint)
			{
				MapWaypointInfo(
									hilited_waypoint->X+(WP_BOX_SIZE>>1)+2,
									hilited_waypoint->Y-(WP_BOX_SIZE>>1)+12,
									hilited_waypoint
								);
			}
			break;
		case	ED_ITEM_SIZE_HOOK:
			if(hilited_sphere)
			{
				MapSphereInfo	(
									hilited_sphere->X+((SIN(256)*hilited_sphere->Radius)>>16)+(SH_BOX_SIZE>>1)+2,
									hilited_sphere->Y-((COS(256)*hilited_sphere->Radius)>>16)+(SH_BOX_SIZE>>1),
									hilited_sphere
								);
			}
			break;
	}

	// Connect a line from the selected thing to the mouse pointer.
	local_point.X	=	MouseX;
	local_point.Y	=	MouseY;
	if(PointInContent(&local_point))
	{
		GlobalToLocal(&local_point);
		if(selected_wp!=NULL)
		switch(SelectMode)
		{
			case	SELECT_NONE:
				break;
			case	SELECT_WAYPOINT:
				DrawLineC(selected_thing->X,selected_thing->Y,local_point.X,local_point.Y,0xffff);
				break;
			case	SELECT_NEXT_WAYPOINT:
			case	SELECT_PREV_WAYPOINT:
				DrawLineC(selected_wp->X,selected_wp->Y,local_point.X,local_point.Y,0xffff);
				break;
		}
	}
}

//---------------------------------------------------------------

void	GameEditor::MapText(SLONG x,SLONG y,CBYTE *the_str,ULONG col)
{
	QuickTextC(x-1,y,the_str,0);
	QuickTextC(x+1,y,the_str,0);
	QuickTextC(x,y+1,the_str,0);
	QuickTextC(x,y-1,the_str,0);
	QuickTextC(x,y,the_str,col);
}

//---------------------------------------------------------------

void	GameEditor::MapThingInfo(SLONG x,SLONG y,BucketMapThing *the_map_thing)
{
	CBYTE		info_text[256];
	EdRect		info_rect;


	info_rect.SetRect(x,y,100,50);
	info_rect.FillRect(CONTENT_COL);
	info_rect.HiliteRect(HILITE_COL,LOLITE_COL);
	info_rect.ShrinkRect(2,2);
	info_rect.HiliteRect(LOLITE_COL,HILITE_COL);

	sprintf(info_text,"Thing No: %ld",the_map_thing->EditRef.ItemRef);
	MapText(info_rect.GetLeft()+1,info_rect.GetTop()+1,info_text,0xffff);

	sprintf(info_text,"MapX : %ld",map_things[the_map_thing->EditRef.ItemRef].X);
	MapText(info_rect.GetLeft()+1,info_rect.GetTop()+11,info_text,0xffff);

	sprintf(info_text,"MapY : %ld",map_things[the_map_thing->EditRef.ItemRef].Z);
	MapText(info_rect.GetLeft()+1,info_rect.GetTop()+21,info_text,0xffff);

	sprintf(info_text,"Alt   : %ld",map_things[the_map_thing->EditRef.ItemRef].Y);
	MapText(info_rect.GetLeft()+1,info_rect.GetTop()+31,info_text,0xffff);
}

//---------------------------------------------------------------

void	GameEditor::MapWaypointInfo(SLONG x,SLONG y,BucketWaypoint *the_waypoint)
{
	CBYTE		info_text[256];
	EdRect		info_rect;


	info_rect.SetRect(x,y,100,50);
	info_rect.FillRect(CONTENT_COL);
	info_rect.HiliteRect(HILITE_COL,LOLITE_COL);
	info_rect.ShrinkRect(2,2);
	info_rect.HiliteRect(LOLITE_COL,HILITE_COL);

	sprintf(info_text,"Waypoint No: %ld",the_waypoint->EditRef.ItemRef);
	MapText(info_rect.GetLeft()+1,info_rect.GetTop()+1,info_text,0xffff);

	sprintf(info_text,"MapX : %ld",edit_waypoints[the_waypoint->EditRef.ItemRef].X);
	MapText(info_rect.GetLeft()+1,info_rect.GetTop()+11,info_text,0xffff);

	sprintf(info_text,"MapY : %ld",edit_waypoints[the_waypoint->EditRef.ItemRef].Z);
	MapText(info_rect.GetLeft()+1,info_rect.GetTop()+21,info_text,0xffff);

	sprintf(info_text,"Alt   : %ld",edit_waypoints[the_waypoint->EditRef.ItemRef].Y);
	MapText(info_rect.GetLeft()+1,info_rect.GetTop()+31,info_text,0xffff);
}

//---------------------------------------------------------------

void	GameEditor::MapSphereInfo(SLONG x,SLONG y,BucketSphereArea *the_sphere)
{
	float		radius;
	CBYTE		info_text[256];
	EdRect		info_rect;


	info_rect.SetRect(x,y,150,30);
	info_rect.FillRect(CONTENT_COL);
	info_rect.HiliteRect(HILITE_COL,LOLITE_COL);
	info_rect.ShrinkRect(2,2);
	info_rect.HiliteRect(LOLITE_COL,HILITE_COL);

	radius	=	map_things[the_sphere->EditRef.ItemRef].Data[0]/128.0;
	sprintf(info_text,"Radius (in blocks) : %4.2f",radius);
	MapText(info_rect.GetLeft()+1,info_rect.GetTop()+1,info_text,0xffff);
}

//---------------------------------------------------------------

void	GameEditor::ClearTabMode(void)
{
	switch(SelectMode)
	{
		case	SELECT_COND_TAB_THING:
		case	SELECT_COND_TAB_SWITCH:
			ConditionMode->SetTabMode(COND_MODE_NONE);
			break;
		case	SELECT_THING_TAB_THING:
		case	SELECT_THING_TAB_SWITCH:
			ThingMode->SetTabMode(THING_MODE_NONE);
			break;
		case	SELECT_COM_TAB_WAYPOINT:
		case	SELECT_COM_TAB_THING:
		case	SELECT_COM_TAB_SWITCH:
			CommandMode->SetTabMode(COM_MODE_NONE);
			break;
	}
}

//---------------------------------------------------------------
