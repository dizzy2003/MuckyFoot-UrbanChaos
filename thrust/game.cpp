//
// Game-wide stuff
//

#include "always.h"
#include "font.h"
#include "game.h"
#include "gamestate.h"
#include "key.h"
#include "land.h"
#include "log.h"
#include "net.h"
#include "orb.h"
#include "os.h"
#include "ping.h"
#include "player.h"
#include "server.h"
#include "ship.h"
#include "tb.h"



//
// The gameturn, process counter and proess time.
// 

SLONG GAME_turn;
SLONG GAME_process;
SLONG GAME_tick;





//
// The game menu type.
//

typedef struct game_menu GAME_Menu;

#define GAME_MENU_ACTION_GOTO_CHILD_MENU		(1 <<  0)
#define GAME_MENU_ACTION_INITIATE_LAN_GAME		(1 <<  1)
#define GAME_MENU_ACTION_JOIN_GAME              (1 <<  2)
#define GAME_MENU_ACTION_EDIT_TEXT				(1 <<  3)
#define GAME_MENU_ACTION_EDIT_COLOUR			(1 <<  4)
#define GAME_MENU_ACTION_EDIT_VALUE			    (1 <<  5)
#define GAME_MENU_ACTION_EDIT_YESNO				(1 <<  6)
#define GAME_MENU_ACTION_BEGIN					(1 <<  7)
#define GAME_MENU_ACTION_EXIT_GAME				(1 <<  8)
#define GAME_MENU_ACTION_MAKE_LAN_CONNECTION    (1 <<  9)
#define GAME_MENU_ACTION_ENUMERATE_LAN_SESSIONS (1 << 10)
#define GAME_MENU_ACTION_MAKE_CHOICE            (1 << 11)


//
// Maximum length of the editable text strings and
//

#define GAME_MENU_TEXT_SIZE 24


typedef struct
{
	CBYTE     *name;	// "!" means the end of the item array.
	GAME_Menu *child;
	SLONG      action;
	CBYTE      text[GAME_MENU_TEXT_SIZE];
	SLONG      value;
	SLONG      min_value;
	SLONG      max_value;
	SLONG      num_choices;
	SLONG      cur_choice;
	CBYTE    **choice;

} GAME_Item;

typedef struct game_menu
{
	CBYTE     *title;
	GAME_Menu *parent;	// This is where you go when you press escape.
	SLONG      selection;
	GAME_Item  item[];

} GAME_Menu;


//
// The menus.
//

extern GAME_Menu GAME_menu_join;
extern GAME_Menu GAME_menu_top;
extern GAME_Menu GAME_menu_server;
extern GAME_Menu GAME_menu_player;


GAME_Menu GAME_menu_top =
{
	"Multiplayer Thrust (c) Mark Adami 1999",
	NULL,
	0,
	{
		{
			"Find LAN Game",
		   &GAME_menu_join,
			GAME_MENU_ACTION_MAKE_LAN_CONNECTION |
			GAME_MENU_ACTION_ENUMERATE_LAN_SESSIONS
		},

		{
			"Create New LAN Server",
		   &GAME_menu_server,
			GAME_MENU_ACTION_GOTO_CHILD_MENU,
		},

		{
			"Configure Player",
		   &GAME_menu_player,
			GAME_MENU_ACTION_GOTO_CHILD_MENU,
		},

		{
			"Exit Game",
			NULL,
			GAME_MENU_ACTION_EXIT_GAME
		},

		{
			NULL
		}
	}
};

GAME_Menu GAME_menu_join =
{
	"Join LAN game",
   &GAME_menu_top,
	0,
	{
		{
			"Choose Game",
			NULL,
			GAME_MENU_ACTION_MAKE_CHOICE,
		},

		{
			"Refresh Server list",
			NULL,
			GAME_MENU_ACTION_ENUMERATE_LAN_SESSIONS,
		},

		{
			"Join Game",
			NULL,
			GAME_MENU_ACTION_JOIN_GAME,
		},

		{
			NULL
		}
	}
};

GAME_Menu GAME_menu_server =
{
	"Create server",
   &GAME_menu_top,
	0,
	{
		{
			"Game Name:",
			NULL,
			GAME_MENU_ACTION_EDIT_TEXT,
			"Unnamed"
		},

		{
			"Maximum Players:",
			NULL,
			GAME_MENU_ACTION_EDIT_VALUE,
			"4",
			4,
			2,
			16
		},

		{
			"Create LAN game",
			NULL,
			GAME_MENU_ACTION_INITIATE_LAN_GAME,
		},

		{
			NULL
		}
	}
};

GAME_Menu GAME_menu_player =
{
	"Configure player",
   &GAME_menu_top,
	0,
	{
		{
			"Option a"
		},

		{
			"Option b",
		},

		{
			NULL
		}
	}	
};





#define GAME_START_MENU_EXIT   0
#define GAME_START_MENU_SERVER 1
#define GAME_START_MENU_PLAYER 2

SLONG GAME_start_menu()
{
	SLONG i;
	SLONG j;
	SLONG colour;

	float y;

	GAME_Menu *gm        = &GAME_menu_top;
	GAME_Item *gi        =  NULL;
	CBYTE     *error     =  NULL;
	CBYTE     *text      =  NULL;
	SLONG      cursor    =  0;
	SLONG      choose    =  FALSE;
	CBYTE     *ch;

	while(1)
	{
		if (OS_process_messages() == OS_QUIT_GAME)
		{
			return GAME_START_MENU_EXIT;
		}

		if (KEY_on[KEY_ESCAPE])
		{
			KEY_on[KEY_ESCAPE] = 0;

			if (error)
			{
				error = NULL;
			}
			else
			if (text)
			{
				text = NULL;
			}
			else
			if (choose)
			{
				choose = FALSE;
			}
			else
			{
				if (gm->parent == NULL)
				{
					return GAME_START_MENU_EXIT;
				}
				else
				{
					gm = gm->parent;
				}
			}
		}

		if (!error)
		{
			if (text)
			{
				//
				// Editing text.
				//

				if (KEY_on[KEY_RETURN] ||
					KEY_on[KEY_ENTER ])
				{
					KEY_on[KEY_RETURN] = 0;
					KEY_on[KEY_ENTER ] = 0;

					//
					// Stop editing the text.
					//

					text = NULL;
				}

				if (KEY_on[KEY_UP] ||
					KEY_on[KEY_DOWN])
				{
					//
					// Stop editing the text and don't clear the keys
					// so that they leak into changing the menu selection.
					//														

					text = NULL;
				}

				if (KEY_on[KEY_BACKSPACE])
				{
					KEY_on[KEY_BACKSPACE] = 0;
	
					if (cursor > 0)
					{
						for (i = cursor - 1; text[i]; i++)
						{
							text[i] = text[i + 1];
						}

						cursor -= 1;
					}
				}
				
				if (KEY_on[KEY_DELETE])
				{
					KEY_on[KEY_DELETE] = 0;

					for (i = cursor; text[i]; i++)
					{
						text[i] = text[i + 1];
					}
				}

				if (KEY_on[KEY_LEFT])
				{
					KEY_on[KEY_LEFT] = 0;

					if (cursor > 0)
					{
						cursor -= 1;
					}
				}

				if (KEY_on[KEY_RIGHT])
				{
					KEY_on[KEY_RIGHT] = 0;

					if (text[cursor])
					{
						cursor += 1;
					}
				}

				if (KEY_on[KEY_HOME])
				{	
					KEY_on[KEY_HOME] = 0;

					cursor = 0;
				}	

				if (KEY_on[KEY_END])
				{
					KEY_on[KEY_END] = 0;

					for (cursor = 0; text[cursor]; cursor++);
				}

				if (KEY_inkey)
				{
					if (FONT_char_is_valid(KEY_inkey) || KEY_inkey == ' ')
					{
						if (strlen(text) < GAME_MENU_TEXT_SIZE - 1)
						{
							//
							// Insert character at current cursor position.
							//

							for (ch = text, i = 0; *ch; ch++, i++);

							while(i > cursor)
							{
								text[i + 1] = text[i];

								i -= 1;
							}

							text[cursor] = KEY_inkey;

							cursor += 1;
						}
					}

					KEY_inkey = 0;
				}
			}

			if (choose)
			{
				//
				// Change current choice.
				//

				if (KEY_on[KEY_UP])
				{
					KEY_on[KEY_UP] = 0;

					if (gm->item[gm->selection].cur_choice > 0)
					{
						gm->item[gm->selection].cur_choice -= 1;
					}
				}

				if (KEY_on[KEY_DOWN])
				{
					KEY_on[KEY_DOWN] = 0;

					if (gm->item[gm->selection].cur_choice < gm->item[gm->selection].num_choices)
					{
						gm->item[gm->selection].cur_choice += 1;
					}
				}

				if (KEY_on[KEY_RETURN] ||
					KEY_on[KEY_ENTER ])
				{
					KEY_on[KEY_RETURN] = 0;
					KEY_on[KEY_ENTER ] = 0;

					choose = FALSE;
				}

				//
				// In case the number of choices suddenly changes!
				//

				SATURATE(gm->item[gm->selection].cur_choice, 0, gm->item[gm->selection].num_choices - 1);
			}

			//
			// Not an ELSE on purpose... so that when a menu is cancelled
			// with UP or DOWN, it gets through to here in the same gameturn.
			//

			if (!text && !choose)
			{
				//
				// Selecting a menu.
				//

				if (KEY_on[KEY_UP])
				{
					KEY_on[KEY_UP] = 0;

					if (gm->selection > 0)
					{
						gm->selection -= 1;
					}
					else
					{
						//
						// Find the last valid item.
						//

						for (gm->selection = 0; gm->item[gm->selection + 1].name; gm->selection += 1);
					}
				}

				if (KEY_on[KEY_DOWN])
				{
					KEY_on[KEY_DOWN] = 0;

					if (gm->item[gm->selection + 1].name)
					{
						gm->selection += 1;
					}
					else
					{
						gm->selection = 0;
					}
				}

				if (KEY_on[KEY_LEFT])
				{
					KEY_on[KEY_LEFT] = 0;

					if (gm->item[gm->selection].action & GAME_MENU_ACTION_EDIT_VALUE)
					{
						if (gm->item[gm->selection].value > gm->item[gm->selection].min_value)
						{
							gm->item[gm->selection].value -= 1;
						}
						else
						{
							//
							// Beep sound.
							//
						}
					}
				}

				if (KEY_on[KEY_RIGHT])
				{
					KEY_on[KEY_RIGHT] = 0;

					if (gm->item[gm->selection].action & GAME_MENU_ACTION_EDIT_VALUE)
					{
						if (gm->item[gm->selection].value < gm->item[gm->selection].max_value)
						{
							gm->item[gm->selection].value += 1;
						}
						else
						{
							//
							// Beep sound.
							//
						}
					}
				}

				if (KEY_on[KEY_RETURN] || KEY_on[KEY_SPACE] || KEY_on[KEY_ENTER])
				{
					KEY_on[KEY_RETURN] = 0;
					KEY_on[KEY_SPACE ] = 0;
					KEY_on[KEY_ENTER ] = 0;

					//
					// Do the current selection's action.
					//

					gi = &gm->item[gm->selection];

					for (i = 0; i < 32; i++)
					{
						if (gi->action & (1 << i))
						{
							switch(1 << i)
							{
								case GAME_MENU_ACTION_GOTO_CHILD_MENU:

									if (gi->child)
									{
										gm = gi->child;
									}

									break;

								case GAME_MENU_ACTION_INITIATE_LAN_GAME:

									if (!SERVER_session_create(
											GAME_menu_server.item[0].text,
											GAME_menu_server.item[1].value,
											SERVER_CONNECT_TYPE_LAN,
											NULL))
									{
										error = "Failed to create new game";
									}
									else
									{
										//
										// Start being the server!
										//

										return GAME_START_MENU_SERVER;
									}

									break;

								case GAME_MENU_ACTION_JOIN_GAME:

									//
									// Join the currently selected session.
									//

									if (WITHIN(GAME_menu_join.item[0].cur_choice, 0, GAME_menu_join.item[0].num_choices - 1))
									{
										if (!NET_session_join(GAME_menu_join.item[0].cur_choice))
										{
											error = "Failed to connect to server";
										}
										else
										{
											//
											// Establish a common time with the server.
											//

											if (!PING_do())
											{
												error = "Failed to establish common time with server";
											}
											else
											{
												return GAME_START_MENU_PLAYER;
											}
										}
									}

									break;

								case GAME_MENU_ACTION_EDIT_TEXT:
									
									//
									// Start editing this menu's text.
									//

									text = gm->item[gm->selection].text;

									//
									// Put the cursor at the end of the text string.
									//

									for (cursor = 0; text[cursor]; cursor++);

									//
									// We use inkey to edit the text...
									//

									KEY_inkey = NULL;

									break;

								case GAME_MENU_ACTION_EDIT_COLOUR:
									break;

								case GAME_MENU_ACTION_EDIT_VALUE:

									if (gm->item[gm->selection].value < gm->item[gm->selection].max_value)
									{
										gm->item[gm->selection].value += 1;
									}
									else
									{
										gm->item[gm->selection].value = gm->item[gm->selection].min_value;
									}

									break;

								case GAME_MENU_ACTION_EDIT_YESNO:
								case GAME_MENU_ACTION_BEGIN:
									break;

								case GAME_MENU_ACTION_EXIT_GAME:
									return GAME_START_MENU_EXIT;

								case GAME_MENU_ACTION_MAKE_LAN_CONNECTION:

									//
									// Connect to the LAN.
									//

									if (!NET_connection_lan())
									{
										error = "Failed to find LAN";
									}
									else
									{
										//
										// Ok.. so goto child menu.
										//

										if (gi->child)
										{
											gm = gi->child;
										}
									}

									break;

								case GAME_MENU_ACTION_ENUMERATE_LAN_SESSIONS:
									
									//
									// Make sure we are connected to the LAN.
									//

									if (NET_connection_lan())
									{
										#define GAME_MAX_SESSION_CHOICES       256
										#define GAME_MAX_SESSION_CHOICE_BUFFER 8192

										static CBYTE *session_choice       [GAME_MAX_SESSION_CHOICES      ];
										static CBYTE  session_choice_buffer[GAME_MAX_SESSION_CHOICE_BUFFER];

										CBYTE *session_choice_buffer_upto = session_choice_buffer;

										memset(session_choice_buffer, 0, sizeof(session_choice_buffer));

										//
										// Find all the games in progress.
										//

										SLONG sessions = NET_session_get_number();

										if (sessions == 0)
										{
											GAME_menu_join.item[0].num_choices = 0;
											GAME_menu_join.item[0].choice      = NULL;
										}
										else
										{
											if (sessions > GAME_MAX_SESSION_CHOICES)
											{
												sessions = GAME_MAX_SESSION_CHOICES;
											}

											GAME_menu_join.item[0].num_choices = sessions;
											GAME_menu_join.item[0].choice      = session_choice;

											for (j = 0; j < sessions; j++)
											{
												strcpy(session_choice_buffer_upto, NET_session_get_info(j).name);

												session_choice[j] = session_choice_buffer_upto;

												while(*session_choice_buffer_upto++);
											}
										}
									}

									break;
								
								case GAME_MENU_ACTION_MAKE_CHOICE:

									if (gi->choice && gi->num_choices)
									{
										choose = TRUE;
									}

									break;

								default:
									ASSERT(0);
									break;
							}
						}
					}
				}
			}
		}

		//
		// Draw the current menu.
		//

		OS_clear_screen();
		OS_scene_begin();

		//
		// Draw the name of the menu in the top left.
		//

		FONT_format(FONT_FLAG_JUSTIFY_LEFT);
		FONT_draw(0.01F, 0.013F, 0xaaaaaa, 0.5F, -1, gm->title);

		//
		// Draw each item down the middle of the screen.
		//

		i = 0;
		y = 0.3F;

		while(gm->item[i].name)
		{
			if (gm->item[i].action & (GAME_MENU_ACTION_EDIT_TEXT | GAME_MENU_ACTION_EDIT_VALUE))
			{
				if (gm->item[i].action & GAME_MENU_ACTION_EDIT_VALUE)
				{
					sprintf(gm->item[i].text, "%d", gm->item[i].value);
				}

				FONT_format(FONT_FLAG_JUSTIFY_RIGHT);

				FONT_draw(0.49F, y, (gm->selection == i && !text) ? 0xffffff : 0xaaaaaa, 1.0F, -1, gm->item[i].name);

				FONT_format(FONT_FLAG_JUSTIFY_LEFT);

				FONT_draw(0.51F, y, (gm->selection == i &&  text) ? 0xffffff : 0x666666, 1.0F, ((text && gm->selection == i) ? cursor : -1), gm->item[i].text);
			}
			else
			if (gm->item[i].action & GAME_MENU_ACTION_MAKE_CHOICE)
			{
				FONT_format(FONT_FLAG_JUSTIFY_RIGHT);

				FONT_draw(0.49F, y, (gm->selection == i && !choose) ? 0xffffff : 0xaaaaaa, 1.0F, -1, gm->item[i].name);

				FONT_format(FONT_FLAG_JUSTIFY_LEFT);

				if (gm->item[i].num_choices == 0 || gm->item[i].choice == NULL)
				{
					FONT_draw(0.51F, y, 0x666666, 1.0F, -1, "<None>");
				}
				else
				{
					for (j = 0; j < gm->item[i].num_choices; j++)
					{
						if (gm->selection == i && choose)
						{
							if (gm->item[i].cur_choice == j)
							{
								colour = 0xffffff;
							}
							else
							{
								colour = 0xaaaaaa;
							}
						}
						else
						{
							if (gm->item[i].cur_choice == j)
							{
								colour = 0xcccccc;
							}
							else
							{
								colour = 0x666666;
							}
						}

						FONT_draw(0.51F, y, colour, 1.0F, -1, gm->item[i].choice[j]);

						if (j > 0)
						{
							y += 0.05F;
						}
					}
				}
			}
			else
			{
				FONT_format(FONT_FLAG_JUSTIFY_CENTRE);

				FONT_draw(0.5F, y, (gm->selection == i) ? 0xffffff : 0xaaaaaa, 1.0F, -1, gm->item[i].name);
			}

			i += 1;
			y += 0.05F;
		}

		if (error)
		{
			OS_Buffer *ob = OS_buffer_new();

			OS_buffer_add_sprite(
				ob,
				0.0F, 0.0F,
				1.0F, 1.0F,
				0.0F, 0.0F,
				1.0F, 1.0F,
				0.0F,
				0x101020);

			OS_buffer_draw(ob, NULL, NULL, OS_DRAW_MULTIPLY | OS_DRAW_ZALWAYS | OS_DRAW_NOZWRITE);

			FONT_format(FONT_FLAG_JUSTIFY_CENTRE);

			FONT_draw(0.5F, 0.4F, 0xff0000, 1.5F, -1, "ERROR");
			FONT_draw(0.5F, 0.5F, 0xff0000, 1.5F, -1, error);
		}

		OS_scene_end();
		OS_show();
	}
}



void GAME_init_level(SLONG level)
{
	//
	// Make sure we are in single precision floating point.
	// Loading DirectPlay fiddles about with the settings
	// and seems to effect AMD and Intel in different ways!
	//

	fpp_set(FPP_SINGLE);

	//
	// Initailise game world.
	//

	SHIP_init();
	ORB_init();
	TB_init();
	LAND_init();

	Point2d level1[25] =
	{
		{12,1},
		{14,8},
		{11,5},
		{8,6},
		{6,18},
		{27,18},
		{29,-19},
		{4,-16},
		{4,-7},
		{-2,-14},
		{-15,-17},
		{-18,-12},
		{-15,-9},
		{-10,-6},
		{-13,-3},
		{-12,4},
		{-17,3},
		{-16,-3},
		{-18,-5},
		{-24,-4},
		{-29,16},
		{-13,17},
		{-10,9},
		{-5,10},
		{12,1}
	};

	LAND_add_line(25, level1);

	LAND_calc_normals();

	{
		SLONG i;

		for (i = 0; i < 12; i += 4)
		{
			ORB_create(-19, -2, 0.50F + i * (0.025F));
		}
	}

	ORB_create(-19, -3, 1.05F);
}





#define GAME_DO_PLAYER_LEFT_SESSION    0
#define GAME_DO_PLAYER_LOST_CONNECTION 1

SLONG GAME_do_player()
{
	SLONG i;
	SLONG rollback               = 0;
	SLONG ignore_server_messages = FALSE;

	GAME_init_level(0);

	switch(PLAYER_create_local(
			"(mucky) Madami",
			55,
			40,
			200,
			1.0F,
			1.0F))
	{
		case PLAYER_CREATE_OK:
			break;

		case PLAYER_CREATE_LOST_CONNECTION:
		case PLAYER_CREATE_TIMED_OUT:
			return GAME_DO_PLAYER_LOST_CONNECTION;

		default:
			ASSERT(0);
			break;	
	}
		
	while(1)
	{
		if (OS_process_messages() == OS_QUIT_GAME || KEY_on[KEY_ESCAPE])
		{
			KEY_on[KEY_ESCAPE] = 0;

			//
			// Leave the game.
			//

			NET_session_leave();

			return GAME_DO_PLAYER_LEFT_SESSION;
		}

		ignore_server_messages = FALSE;

		while(GAME_tick < OS_ticks())
		{	
			GAME_process += 1;
			GAME_turn     = GAME_process >> 4;

			GAME_tick += GAME_TICKS_PER_PROCESS;
			
			if ((GAME_process & 0xf) == 0)
			{
				if ((GAME_turn & 0x3f) == 0)
				{
					for (i = 0; i < SHIP_MAX_SHIPS; i++)
					{
						SHIP_Ship *ss = &SHIP_ship[i];

						ss->hash = ss->x + ss->y + ss->angle;
					}
				}

				//
				// Flag all ships that go active this gameturn.
				//

				SHIP_flag_active(GAME_turn);

				if (!PLAYER_process(&rollback, ignore_server_messages))
				{
					//
					// We've lost a connection with the server.
					//

					return GAME_DO_PLAYER_LEFT_SESSION;
				}

				//
				// Only process server messages once for each time we draw the screen.
				//

				ignore_server_messages = TRUE;

				if (rollback)
				{
					//
					// Gamestate has been rolled back! Re-process with the new
					// keypresses.
					//

					ASSERT(rollback < GAME_turn);

					while(1)
					{
						SHIP_flag_active(rollback);

						PLAYER_process_messages(rollback);
						PLAYER_press_keys      (rollback);

						#if 0

						if (rollback & 1)
						{
							OS_clear_screen(30,40,60);
						}
						else
						{
							OS_clear_screen(60,40,30);
						}

						FONT_format(FONT_FLAG_JUSTIFY_CENTRE);
						FONT_draw(0.5F, 0.5F, 0x583768, 1.0F, -1, "Rollback from turn %d to turn %d", rollback - 1, GAME_turn);

						OS_show();

						#endif

						for (i = 0; i < 16; i++)
						{
							SHIP_process_all();
							ORB_process_all();
							TB_process_all();
						}

						if ((rollback & 0x3f) == 0)
						{
							SHIP_Ship *ss = &SHIP_ship[0];

							ss->hash = ss->x + ss->y + ss->angle;
						}

						rollback += 1;

						if (rollback >= GAME_turn)
						{
							//
							// Flag all ships that go active this gameturn.
							//

							SHIP_flag_active(GAME_turn);

							break;
						}

						PLAYER_new_gameturn(rollback);
					}

					rollback = 0;
				}

				PLAYER_process_messages(GAME_turn);
				PLAYER_press_keys      (GAME_turn);

				if (SHIP_ship[0].flag & SHIP_FLAG_KEY_THRUST)
				{
					LOG_message(0x00ff00, "Thrust %d", GAME_turn);
				}

				if (SHIP_ship[0].flag & SHIP_FLAG_ACTIVE)
				{
					LOG_file("SHIP_process_one:   GAME_turn %3d key %3d (%f,%f) %f\n", GAME_turn, SHIP_ship[0].flag, SHIP_ship[0].x, SHIP_ship[0].y, SHIP_ship[0].angle);
				}

				//
				// Remember this gameturn.
				//

				PLAYER_new_gameturn(GAME_turn);
			}

			SHIP_process_all();
			ORB_process_all();
			TB_process_all();
		}

		if ((GAME_turn & 0x1f) == 0)
		{
			OS_clear_screen(0x22, 0x33, 0xff);
		}
		else
		{
			OS_clear_screen();
		}

		OS_scene_begin();

		{
			float mid_x = 0.0F;
			float mid_y = 0.0F;
			float zoom  = 0.014F;

			SHIP_draw_all(mid_x, mid_y, zoom);
			ORB_draw_all (mid_x, mid_y, zoom);
			TB_draw_all  (mid_x, mid_y, zoom);
			LAND_draw_all(mid_x, mid_y, zoom);
		}

		LOG_draw();

		FONT_draw(0.052F, 0.052F, 0x000000, 0.7F, -1, "Game turn: %d Real turn: %d", GAME_turn >> 5, GAME_turn);
		FONT_draw(0.050F, 0.050F, 0xff0000, 0.7F, -1, "Game turn: %d Real turn: %d", GAME_turn >> 5, GAME_turn);
		//FONT_draw(0.600F, 0.900F, 0xffff00, 0.7F, -1, "(%5.2f,%5.2f) %5.2f", SHIP_ship[0].x, SHIP_ship[0].y, SHIP_ship[0].angle);

		FONT_draw(0.600F, 0.700F, 0xffff00, 1.7F, -1, "%f", SHIP_ship[0].hash);
		FONT_draw(0.600F, 0.800F, 0xffff00, 1.7F, -1, "%f", SHIP_ship[1].hash);
		FONT_draw(0.600F, 0.900F, 0xffff00, 1.7F, -1, "%f", SHIP_ship[2].hash);
		
		if (rollback)
		{	
			FONT_draw(0.3F, 0.3F, 0x583869, 1.0F, -1, "Rollback %d", GAME_turn - rollback);
		}


		OS_scene_end();
		OS_show();
	}
}


#if 0

void GAME_do_player_old()
{
	GAMESTATE_State *gs;

	float mid_x = 0.00F;
	float mid_y = 0.00F;
	float zoom  = 0.014F;

	SHIP_Ship *ss[2];
	ORB_Orb   *oo;
	TB_Tb     *tt[2] = {NULL, NULL};

	SHIP_init();
	ORB_init();
	TB_init();
	LAND_init();
	FONT_init();

	ss[0] = SHIP_create( 0.0F, -4.0F);
	ss[1] = SHIP_create(-5.0F, -4.0F);

	/*
	oo    = ORB_create (-14,-13, 0.6F);
	oo    = ORB_create (-19, -3, 0.8F);
	oo    = ORB_create ( 20,-13, 1.0F);
	*/

	{
		SLONG i;

		for (i = 0; i < 12; i++)
		{
			ORB_create(-19, -2, 0.50F + i * (0.025F));
		}
	}

	ORB_create(-19, -3, 1.05F);

	
	//
	// The landscape.
	//
	
	/*

	LAND_add_line( 15.0F, -12.0F, -10.0F, -10.0F);
	LAND_add_line( 12.0F,   9.0F,  15.0F, -12.0F);
	LAND_add_line(-13.0F,   9.0F,  12.0F,   9.0F);
	LAND_add_line(-10.0F, -10.0F, -13.0F,   9.0F);

	*/

	/*

	LAND_add_line( 10.0F, -10.0F, -10.0F, -10.0F);
	LAND_add_line( 10.0F,  10.0F,  10.0F, -10.0F);
	LAND_add_line(-10.0F,  10.0F,  10.0F,  10.0F);
	LAND_add_line(-10.0F, -10.0F, -10.0F,  10.0F);

	*/

	Point2d level1[25] =
	{
		{12,1},
		{14,8},
		{11,5},
		{8,6},
		{6,18},
		{27,18},
		{29,-19},
		{4,-16},
		{4,-7},
		{-2,-14},
		{-15,-17},
		{-18,-12},
		{-15,-9},
		{-10,-6},
		{-13,-3},
		{-12,4},
		{-17,3},
		{-16,-3},
		{-18,-5},
		{-24,-4},
		{-29,16},
		{-13,17},
		{-10,9},
		{-5,10},
		{12,1}
	};

	LAND_add_line(25, level1);

	LAND_calc_normals();

	while(1)
	{
		if (OS_process_messages() == OS_QUIT_GAME || KEY_on[KEY_ESCAPE])
		{
			KEY_on[KEY_ESCAPE] = 0;

			//
			// Leave the game.
			//

			NET_session_leave();

			return;
		}

		while(GAME_tick < OS_ticks())
		{
			GAME_tick    += GAME_TICKS_PER_PROCESS;
			GAME_process += 1;

			if ((GAME_process % 16) == 0)
			{
				GAME_turn = GAME_process >> 4;

				{
					ss[0]->flag &= ~(SHIP_FLAG_LEFT | SHIP_FLAG_RIGHT | SHIP_FLAG_THRUST);

					if (KEY_on[KEY_Z]) {ss[0]->flag |= SHIP_FLAG_LEFT; }
					if (KEY_on[KEY_X]) {ss[0]->flag |= SHIP_FLAG_RIGHT;}

					if (KEY_on[KEY_V]) {ss[0]->flag |= SHIP_FLAG_THRUST;}

					if (KEY_on[KEY_G])
					{
						KEY_on[KEY_G] = 0;

						//
						// Create/destroy the tractor beam.
						//

						if (tt[0] == NULL)
						{
							tt[0] = TB_create(ss[0], 5.0F);
						}
						else
						{
							TB_destroy(tt[0]);

							tt[0] = NULL;
						}
					}
				}

				{
					ss[1]->flag &= ~(SHIP_FLAG_LEFT | SHIP_FLAG_RIGHT | SHIP_FLAG_THRUST);

					if (KEY_on[KEY_P1]) {ss[1]->flag |= SHIP_FLAG_LEFT; }
					if (KEY_on[KEY_P2]) {ss[1]->flag |= SHIP_FLAG_RIGHT;}

					if (KEY_on[KEY_P6]) {ss[1]->flag |= SHIP_FLAG_THRUST;}

					if (KEY_on[KEY_PADD])
					{
						KEY_on[KEY_PADD] = 0;

						//
						// Create/destroy the tractor beam.
						//

						if (tt[1] == NULL)
						{
							tt[1] = TB_create(ss[1], 5.0F);
						}
						else
						{
							TB_destroy(tt[1]);

							tt[1] = NULL;
						}
					}
				}
			}

			SHIP_process_all(GAME_turn);
			ORB_process_all();
			TB_process_all();

			if (tt[0])
			{
				//
				// Has the ship or the orb collided?
				//

				if (tt[0]->ss->flag & SHIP_FLAG_COLLIDED)
				{
					TB_destroy(tt[0]);

					tt[0] = NULL;
				}
				else
				if (tt[0]->oo->flag & ORB_FLAG_COLLIDED)
				{
					float speed = qdist2(fabs(tt[0]->oo->dx), fabs(tt[0]->oo->dy));

					if (speed > 0.01F)
					{
						TB_destroy(tt[0]);

						tt[0] = NULL;
					}
				}
			}

			if (tt[1])
			{
				//
				// Has the ship or the orb collided?
				//

				if (tt[1]->ss->flag & SHIP_FLAG_COLLIDED)
				{
					TB_destroy(tt[1]);

					tt[1] = NULL;
				}
				else
				if (tt[1]->oo->flag & ORB_FLAG_COLLIDED)
				{
					float speed = qdist2(fabs(tt[1]->oo->dx), fabs(tt[1]->oo->dy));

					if (speed > 0.01F)
					{
						TB_destroy(tt[1]);

						tt[1] = NULL;
					}
				}
			}

			float want_mid_x;
			float want_mid_y;

			want_mid_x  = ss[0]->x + ss[1]->x;
			want_mid_y  = ss[0]->y + ss[1]->y;
			want_mid_x *= 0.5F;
			want_mid_y *= 0.5F;

			mid_x += (want_mid_x - mid_x) * 0.01F;
			mid_y += (want_mid_y - mid_y) * 0.01F;
		}

		if (KEY_on[KEY_I]) {zoom *= 1.005F;}
		if (KEY_on[KEY_O]) {zoom *= 0.995F;}

		OS_scene_begin();
		OS_clear_screen();
		SHIP_draw_all(mid_x, mid_y, zoom);
		ORB_draw_all (mid_x, mid_y, zoom);
		TB_draw_all  (mid_x, mid_y, zoom);
		LAND_draw_all(mid_x, mid_y, zoom);
		OS_fps();
		OS_scene_end();
		OS_show();
	}
}

#endif


void GAME_do_server()
{
	GAME_init_level(0);

	GAME_turn    = 0;
	GAME_process = 0;
	GAME_tick    = OS_ticks();

	while(1)
	{
		if (OS_process_messages() == OS_QUIT_GAME || KEY_on[KEY_ESCAPE])
		{
			KEY_on[KEY_ESCAPE] = 0;
			
			//
			// Kill the game.
			//

			NET_session_destroy();

			return;
		}

		while(GAME_tick < OS_ticks())
		{	
			GAME_process += 1;
			GAME_turn     = GAME_process >> 4;

			GAME_tick += GAME_TICKS_PER_PROCESS;
		}

		SERVER_process();
		SERVER_draw();
	}
}



void GAME_do()
{
	OS_string("Gamestate is %d bytes\n", sizeof(GAMESTATE_State));

	while(1)
	{
		switch(GAME_start_menu())
		{
			case GAME_START_MENU_EXIT:
				return;

			case GAME_START_MENU_PLAYER:
				GAME_do_player();
				break;

			case GAME_START_MENU_SERVER:
				GAME_do_server();
				break;

			default:
				ASSERT(0);
				break;
		}
	}
}
