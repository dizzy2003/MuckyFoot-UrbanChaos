// Editor.cpp
// Guy Simmons, 19th February 1997.

#include	"Editor.hpp"
#include	"fallen/headers/sound.h"
#include	"mfx.h"

//#include	"DXEngine.h"

#define	FRONT_MODULE		CurrentModule
//#define	DEMO 		1
//#define	DX_TEST		1

UBYTE				editor_status;
ULONG				editor_turn,
					update;
ControlSet			module_set;
EditorModule		*CurrentModule,
					*ModuleList;
EditorModule		*test1;
EdRect				module_bounds;
GameEditor			*game_editor;
KeyFrameEditor		*key_frame_editor;
KeyFrameEditor2		*key_framer;
LevelEditor			*level_editor;

//
// The current texture set.
//

SLONG editor_texture_set = 1;


void	update_modules(void);
void	add_module(EditorModule *the_module);

ControlDef	module_def[]	=
{
	{	BUTTON,			0,	"Level Editor",		2,		2,	0,	10			},
	{	BUTTON,			0,	"Key Frame Editor",	68,		2,	0,	10			},
	{	BUTTON,			0,	"Game Editor",		159,	2,	0,	10			},

	{	0															   		}
};

//---------------------------------------------------------------

void	handle_front_module(ULONG clicked_area,MFPoint *clicked_point)
{
	switch(clicked_area)
	{
		case	OUTSIDE_WINDOW:				// Invalid click.
			break;
		case	IN_WINDOW:					// Clicked in non relevant part of window.
			break;
		case	IN_TITLE:
			FRONT_MODULE->MoveModule(clicked_point);
			update_modules();
			break;
		case	IN_GROW:
			FRONT_MODULE->SizeModule(clicked_point);
			update_modules();
			break;
		case	IN_ICONS:
			FRONT_MODULE->TopIcons.HandleIconClick(0,clicked_point);
			update_modules(); //Only really need to redraw the icon strip
			break;
		case	IN_HSCROLL:
			break;
		case	IN_VSCROLL:
			break;
		case	IN_CONTENT:
			FRONT_MODULE->HandleContentClick(LEFT_CLICK,clicked_point);
			break;
		case	IN_CONTROLS:
			FRONT_MODULE->HandleControlClick(LEFT_CLICK,clicked_point);
			break;
	}
}

//---------------------------------------------------------------
SLONG	should_module_exist(EditorModule *m)
{
	if(m==CurrentModule)
		return(1);

	if(CurrentModule!=level_editor && CurrentModule!=key_frame_editor && CurrentModule!=game_editor)
	{
		//
		// only draw keyframer modules
		//
		if(m!=level_editor && m!=game_editor)
			return(1);
	}



	if(CurrentModule==key_frame_editor)
	{
		if(m!=level_editor && m!=game_editor)
			return(1);
	}
	return(0);
}
//		if(current_module==CurrentModule|| (CurrentModule==key_frame_editor && (current_module!=level_editor) && (current_module!=key_frame_editor) && (current_module!=game_editor) ) )

void	update_modules(void)
{
	EditorModule	*current_module;


	ClearWorkScreen(0);
	current_module	=	ModuleList;
	while(current_module)
	{
		if(should_module_exist(current_module))
			current_module->DrawWindow();
		current_module	=	current_module->GetNextModuleLink();
	}

	module_set.ControlSetBounds(&module_bounds);
	module_set.DrawControlSet();

	ShowWorkScreen(0);
}

//---------------------------------------------------------------

void	bring_module_to_front(EditorModule *the_module)
{
	if(the_module == CurrentModule)
		return;							// Already at the end of list (Front of display).

	if(the_module->GetLastModuleLink())
	{									// Not at start of list.
		the_module->GetLastModuleLink()->SetNextModuleLink(the_module->GetNextModuleLink());
		the_module->GetNextModuleLink()->SetLastModuleLink(the_module->GetLastModuleLink());
	}
	else
	{									// At start of list.
		ModuleList		=	the_module->GetNextModuleLink();
		ModuleList->SetLastModuleLink(NULL);
	}
	the_module->SetLastModuleLink(NULL);
	the_module->SetNextModuleLink(NULL);
	add_module(the_module);

	update_modules();
}

//---------------------------------------------------------------

EditorModule	*point_in_module(MFPoint *clicked_point)
{
	EditorModule	*clicked_module,
					*current_module;


	clicked_module	=	NULL;
	current_module	=	ModuleList;
	while(current_module)
	{
//		if(current_module==CurrentModule|| (CurrentModule==key_frame_editor && (current_module!=level_editor) && (current_module!=key_frame_editor) && (current_module!=game_editor) ) )
	//	if(current_module==CurrentModule|| ((current_module!=level_editor) && (current_module!=key_frame_editor) && (current_module!=game_editor) ) )
		if(should_module_exist(current_module))
			if(current_module->PointInRect(clicked_point))
				clicked_module	=	current_module;
		current_module	=	current_module->GetNextModuleLink();
	}
	return	clicked_module;
}

//---------------------------------------------------------------

void	add_module(EditorModule *the_module)
{
	if(CurrentModule==NULL)			// Start of list?
	{
		ModuleList		=	the_module;
		CurrentModule	=	ModuleList;
		CurrentModule->SetStateFlags(ACTIVE);
		CurrentModule->SetExternalUpdatePtr(&update);
	}
	else
	{
		CurrentModule->SetStateFlags(0);
		CurrentModule->SetNextModuleLink(the_module);
		the_module->SetLastModuleLink(CurrentModule);
		CurrentModule	=	the_module;
		CurrentModule->SetStateFlags(ACTIVE);
		CurrentModule->SetExternalUpdatePtr(&update);
	}
}

//---------------------------------------------------------------

void create_editor_modules(void)
{
	CurrentModule	=	NULL;
	ModuleList		=	NULL;

	level_editor	=	new LevelEditor;
	ERROR_MSG(level_editor,"Unable to create LevelEditor object.");
	if(level_editor)
	{
		add_module(level_editor);
		level_editor->SetupModule();
	}

	key_frame_editor	=	new	KeyFrameEditor;
	ERROR_MSG(key_frame_editor,"Unable to create KeyFrameEditor object.");
	if(key_frame_editor)
	{
		add_module(key_frame_editor);
		key_frame_editor->SetupModule();
		key_frame_editor->LinkLevelEditor=level_editor;
	}

	game_editor			=	new GameEditor;
	ERROR_MSG(game_editor,"Unable to create GameEditor object.");
	if(game_editor)
	{
		add_module(game_editor);
		game_editor->SetupModule();
//		game_editor->LinkLevelEditor=level_editor;
	}

#ifdef	GUY
	key_framer	=	new	KeyFrameEditor2;
	ERROR_MSG(key_frame_editor,"Unable to create KeyFrameEditor2 object.");
	if(key_framer)
	{
		add_module(key_framer);
		key_framer->SetupModule();
	}
#endif

}

//---------------------------------------------------------------

void destroy_editor_modules(void)
{
#ifdef	GUY
	if(key_framer)
		delete	key_framer;
#endif
	if(key_frame_editor)
		delete	key_frame_editor;

	if(level_editor)
		delete	level_editor;
}

//---------------------------------------------------------------


MFPoint		last_mouse;

void	editor(void)
{
	UWORD			clicked;
	ULONG			clicked_area;
	Alert			*quit_alert;
	EditorModule	*clicked_module;
	MFPoint			mouse_point;


	editor_status	=	EDITOR_NORMAL;
	editor_turn		=	0;
	update			=	0;
	while(SHELL_ACTIVE && (editor_status&EDITOR_NORMAL))
	{
		//
		// Control + Number keys selects the current texture set.
		//

		if (Keys[KB_TILD])
		{
			Keys[KB_TILD] = 0;
			editor_texture_set = 1;
			free_game_textures(FREE_UNSHARED_TEXTURES);
			load_game_textures(LOAD_UNSHARED_TEXTURES);
		}
		if (Keys[KB_PPOINT])
		{
			SLONG old_texture_set = editor_texture_set;

			if (!ShiftFlag)
			{
				if (Keys[KB_P1]) {Keys[KB_P1] = 0; editor_texture_set = 1;}
				if (Keys[KB_P2]) {Keys[KB_P2] = 0; editor_texture_set = 2;}
				if (Keys[KB_P3]) {Keys[KB_P3] = 0; editor_texture_set = 3;}
				if (Keys[KB_P4]) {Keys[KB_P4] = 0; editor_texture_set = 4;}
				if (Keys[KB_P5]) {Keys[KB_P5] = 0; editor_texture_set = 5;}
				if (Keys[KB_P6]) {Keys[KB_P6] = 0; editor_texture_set = 6;}
				if (Keys[KB_P7]) {Keys[KB_P7] = 0; editor_texture_set = 7;}
				if (Keys[KB_P8]) {Keys[KB_P8] = 0; editor_texture_set = 8;}
				if (Keys[KB_P9]) {Keys[KB_P9] = 0; editor_texture_set = 9;}
				if (Keys[KB_P0]) {Keys[KB_P0] = 0; editor_texture_set = 10;}
			}
			else
			{
				if (Keys[KB_P1]) {Keys[KB_P1] = 0; editor_texture_set = 11;}
				if (Keys[KB_P2]) {Keys[KB_P2] = 0; editor_texture_set = 12;}
				if (Keys[KB_P3]) {Keys[KB_P3] = 0; editor_texture_set = 13;}
				if (Keys[KB_P4]) {Keys[KB_P4] = 0; editor_texture_set = 14;}
				if (Keys[KB_P5]) {Keys[KB_P5] = 0; editor_texture_set = 15;}
				if (Keys[KB_P6]) {Keys[KB_P6] = 0; editor_texture_set = 16;}
				if (Keys[KB_P7]) {Keys[KB_P7] = 0; editor_texture_set = 17;}
				if (Keys[KB_P8]) {Keys[KB_P8] = 0; editor_texture_set = 18;}
				if (Keys[KB_P9]) {Keys[KB_P9] = 0; editor_texture_set = 19;}
				if (Keys[KB_P0]) {Keys[KB_P0] = 0; editor_texture_set = 20;}

#ifdef	I_AM_A_LOON
				#define MAX_TICKS 16

				static SLONG tick_key [MAX_TICKS];
				static SLONG tick_time[MAX_TICKS];
				static SLONG tick_upto;

				ASSERT(WITHIN(tick_upto, 0, MAX_TICKS - 1));

				if (Keys[KB_P0]) {Keys[KB_P0] = 0; tick_key[tick_upto] = 0; tick_time[tick_upto] = GetTickCount(); tick_upto++;}
				if (Keys[KB_P1]) {Keys[KB_P1] = 0; tick_key[tick_upto] = 1; tick_time[tick_upto] = GetTickCount(); tick_upto++;}
				if (Keys[KB_P2]) {Keys[KB_P2] = 0; tick_key[tick_upto] = 2; tick_time[tick_upto] = GetTickCount(); tick_upto++;}
				if (Keys[KB_P3]) {Keys[KB_P3] = 0; tick_key[tick_upto] = 3; tick_time[tick_upto] = GetTickCount(); tick_upto++;}
				if (Keys[KB_P4]) {Keys[KB_P4] = 0; tick_key[tick_upto] = 4; tick_time[tick_upto] = GetTickCount(); tick_upto++;}
				if (Keys[KB_P5]) {Keys[KB_P5] = 0; tick_key[tick_upto] = 5; tick_time[tick_upto] = GetTickCount(); tick_upto++;}
				if (Keys[KB_P6]) {Keys[KB_P6] = 0; tick_key[tick_upto] = 6; tick_time[tick_upto] = GetTickCount(); tick_upto++;}
				if (Keys[KB_P7]) {Keys[KB_P7] = 0; tick_key[tick_upto] = 7; tick_time[tick_upto] = GetTickCount(); tick_upto++;}
				if (Keys[KB_P8]) {Keys[KB_P8] = 0; tick_key[tick_upto] = 8; tick_time[tick_upto] = GetTickCount(); tick_upto++;}
				if (Keys[KB_P9]) {Keys[KB_P9] = 0; tick_key[tick_upto] = 9; tick_time[tick_upto] = GetTickCount(); tick_upto++;}

				if (tick_upto >= MAX_TICKS)
				{
					tick_upto = 0;
				}

				//
				// Has the correct rhythm been typed in?
				//

				if (tick_key[(tick_upto - 1) & (MAX_TICKS - 1)] == tick_key[(tick_upto - 2) & (MAX_TICKS - 1)] &&
					tick_key[(tick_upto - 1) & (MAX_TICKS - 1)] == tick_key[(tick_upto - 3) & (MAX_TICKS - 1)] &&
					tick_key[(tick_upto - 1) & (MAX_TICKS - 1)] == tick_key[(tick_upto - 4) & (MAX_TICKS - 1)] &&
					tick_key[(tick_upto - 1) & (MAX_TICKS - 1)] == tick_key[(tick_upto - 5) & (MAX_TICKS - 1)])
				{
					SLONG t1 = tick_time[(tick_upto - 1) & (MAX_TICKS - 1)] - tick_time[(tick_upto - 2) & (MAX_TICKS - 1)];
					SLONG t2 = tick_time[(tick_upto - 2) & (MAX_TICKS - 1)] - tick_time[(tick_upto - 3) & (MAX_TICKS - 1)];
					SLONG t3 = tick_time[(tick_upto - 3) & (MAX_TICKS - 1)] - tick_time[(tick_upto - 4) & (MAX_TICKS - 1)];
					SLONG t4 = tick_time[(tick_upto - 4) & (MAX_TICKS - 1)] - tick_time[(tick_upto - 5) & (MAX_TICKS - 1)];

					if (t1 > 0 && t2 > 0 && t3 > 0 && t4 > 0)
					{
						SLONG same1 = (t1 << 8) / t2;
						SLONG same2 = (t3 << 8) / t4;
						SLONG quaver;
						SLONG crotchet;
						SLONG same;

						if (WITHIN(same1, 200, 320) &&
							WITHIN(same2, 200, 320))
						{
							quaver   = t1 + t2 >> 1;
							crotchet = t3 + t4 >> 1;

							same = (quaver * 2 << 8) / crotchet;

							if (WITHIN(same, 200, 320))
							{
//								play_quicker_wave(S_THEHILLSAREALIVE);
								MFX_play_ambient(0,S_THEHILLSAREALIVE,MFX_REPLACE);

								//
								// The right rhythm! Change the texture set.
								//

								editor_texture_set = tick_key[(tick_upto - 1) & (MAX_TICKS - 1)];

								//
								// Make sure it doesn't do it any more.
								//

								memset(tick_time, 0, sizeof(tick_time));
							}
						}
					}
				}
#endif
			}

			if (editor_texture_set != old_texture_set)
			{
				free_game_textures(FREE_UNSHARED_TEXTURES);
				load_game_textures(LOAD_UNSHARED_TEXTURES);

				update_modules();
			}
		}
		else
		{
			mouse_point.X	=	MouseX;
			mouse_point.Y	=	MouseY;
			module_set.HandleControlSet(&mouse_point);

			if(LeftMouse.ButtonState)
			{
				mouse_point.X	=	LeftMouse.MouseX;
				mouse_point.Y	=	LeftMouse.MouseY;
				LeftMouse.ButtonState	=	0;

				if(module_set.PointInControlSet(&mouse_point))
				{
					clicked	=	module_set.HandleControlSetClick(LEFT_CLICK,&mouse_point);
					switch(clicked)
					{
						case	1:
							bring_module_to_front(level_editor);
							update	=	1;
							break;
						case	2:
							bring_module_to_front(key_frame_editor);
							update	=	1;
							break;
						case	3:
							bring_module_to_front(game_editor);
							update	=	1;
							break;
					}
				}
				else
				{
					clicked_module	=	point_in_module(&mouse_point);
					if(clicked_module==FRONT_MODULE)
					{
						handle_front_module	(
												clicked_module->WhereInWindow(&mouse_point),
												&mouse_point
											);
					}
					else if(clicked_module)
					{
						bring_module_to_front(clicked_module);
						handle_front_module	(
												clicked_module->WhereInWindow(&mouse_point),
												&mouse_point
											);
					}
				}
			}
			else if(RightMouse.ButtonState)
			{
				mouse_point.X	=	RightMouse.MouseX;
				mouse_point.Y	=	RightMouse.MouseY;
				RightMouse.ButtonState	=	0;

				clicked_module	=	point_in_module(&mouse_point);
				if(clicked_module==FRONT_MODULE)
				{
					clicked_area	=	clicked_module->WhereInWindow(&mouse_point);
					if(clicked_area==IN_CONTENT)
						FRONT_MODULE->HandleContentClick(RIGHT_CLICK,&mouse_point);
					else if(clicked_area==IN_CONTROLS)
						FRONT_MODULE->HandleControlClick(RIGHT_CLICK,&mouse_point);
				}
			}
			else if(MiddleMouse.ButtonState)
			{
				mouse_point.X	=	MiddleMouse.MouseX;
				mouse_point.Y	=	MiddleMouse.MouseY;
				MiddleMouse.ButtonState	=	0;

				clicked_module	=	point_in_module(&mouse_point);
				if(clicked_module==FRONT_MODULE)
				{
					clicked_area	=	clicked_module->WhereInWindow(&mouse_point);
					if(clicked_area==IN_CONTENT)
						FRONT_MODULE->HandleContentClick(MIDDLE_CLICK,&mouse_point);
					else if(clicked_area==IN_CONTROLS)
						FRONT_MODULE->HandleControlClick(MIDDLE_CLICK,&mouse_point);
				}
			}

			if(FRONT_MODULE)
			{
				FRONT_MODULE->HandleModule();
			}

			if(update)
			{
				update_modules();
				update	=	0;
			}

			if(LastKey==KB_ESC) //MD || MiddleButton)
			{
				if(!FRONT_MODULE->LocalEscape())
				{
					LastKey			=	0;
					quit_alert		=	new	Alert;
					editor_status	=	!quit_alert->HandleAlert("Are you sure you want to quit?",NULL);
					delete	quit_alert;
					update_modules();
				}
			}
			else if(LastKey==KB_R && ControlFlag)
			{
				editor_status	^=	EDITOR_RECORD;
				LastKey	=	0;
			}
			else if(LastKey==KB_C && ControlFlag)
			{
				if(LockWorkScreen())
				{
	extern	void	screen_shot(void);
					screen_shot();

	//				do_single_shot(WorkScreen,CurrentPalette);
					UnlockWorkScreen();
				}
				LastKey	=	0;
			}
			else if(LastKey==KB_TAB && (Keys[KB_LCONTROL] || Keys[KB_RCONTROL]))
			{
				if(Keys[KB_LSHIFT] || Keys[KB_RSHIFT])
				{
					FRONT_MODULE->ActivateLastTab();
					LastKey	=	0;
				}
				else
				{
					FRONT_MODULE->ActivateNextTab();
					LastKey	=	0;
				}
			}

			if(editor_status&EDITOR_RECORD)
			{
				if(last_mouse.X!=MouseX||last_mouse.Y!=MouseY)
				{
					last_mouse.X	=	MouseX;
					last_mouse.Y	=	MouseY;
					ShowWorkScreen(0);
				}
			}
			editor_turn++;
		}
	}
}

//---------------------------------------------------------------


UBYTE	editor_loop(void)
{
	GameTexture		*the_texture;
	SLONG	i;

	the_display.MenuOff();

	editor_texture_set = 0;
/*
#ifndef USE_A3D
	LoadWaveList("Data\\SFX\\1622\\","Data\\SFX\\Samples.txt");
#else
	A3DLoadWaveList("Data\\SFX\\1622\\","Data\\SFX\\Samples.txt");
#endif
*/
	for(i=0;i<15;i++)
	{
		the_texture	= &game_textures[i];
		the_texture->TexturePtr	=	(UWORD*)MemAlloc(TEXTURE_PAGE_SIZE);
	}
	free_game_textures(FREE_SHARED_TEXTURES);

	load_game_textures(LOAD_SHARED_TEXTURES);

	if(SetDisplay(800,600,16)==NoError)
	{
		SetDrawFunctions(16);
		SetWorkWindowBounds(0,0,800,600);
/*
		while(!LeftButton)
		{
			ClearWorkScreen(0);
			if(LockWorkScreen())
			{
				DrawCircleC(400,300,abs(MouseX-400),0xffff);
				UnlockWorkScreen();
				ShowWorkScreen(0);
			}
		}

*/
		module_bounds.SetRect(0,0,800,20);
		LogText("Into editor_loop");
		InterfaceDefaults	=	new	Interface;
		if(InterfaceDefaults)
		{
			InterfaceDefaults->SetupInterfaceDefaults();

			create_editor_modules();

			module_set.InitControlSet(module_def);

			update_modules();

			editor();

			destroy_editor_modules();

			delete	InterfaceDefaults;
		}
		ClearWorkScreen(0);
		ShowWorkScreen(0);
	}
	else
		LogText(" SET DISPLAY 800,600 ERROR \n");

	MFX_free_wave_list();

	free_game_textures(FREE_ALL_TEXTURES);
	the_display.MenuOn();
void	free_edit_memory(void);
	free_edit_memory();

	SetDisplay(640,480,16);


	return	0;
}

//---------------------------------------------------------------
