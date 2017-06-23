#include <MFStdLib.h>
#include "game.h"
#ifndef	PSX
#include "fallen/ddlibrary/headers/net.h"
#else
#include "fallen/psxlib/headers/net.h"
#endif

UBYTE CNET_network_game;
UBYTE CNET_i_am_host;

UBYTE CNET_player_id=0;
UBYTE CNET_num_players=1;

#ifndef	PSX
#ifndef TARGET_DC

void CNET_display_error(CBYTE *error)
{
	SLONG i;

	UBYTE e[18];


	(e[1]) = 10;

	for (i = 0; i < 100; i++)
	{
		AENG_clear_screen();

		if (AENG_lock())
		{
			if (i & 1)
			{
				FONT_draw(50, 50, error);
			}

			AENG_unlock();
		}

		AENG_flip();
	}
}







UBYTE CNET_connected;

SLONG CNET_configure()
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG y;

	SLONG word1;
	SLONG word2;

	SLONG num_connections;
	SLONG num_sessions;
	SLONG num_players;

	NET_Message mess;

	CBYTE *my_name;

	#define NUM_WORDS 8
	#define NUM_NAMES 16
#ifndef	PSX
#ifndef TARGET_DC
	srand(time(NULL));
#endif
#endif

	CBYTE *word[NUM_WORDS] =
	{
		"City",
		"Fallen",
		"Dark",
		"Prophecy",
		"Sodium",
		"Shadow",
		"Howling",
		"Siren"
	};

	CBYTE *name[NUM_NAMES] =
	{
		"Dipsy",
		"La La",
		"Po",
		"Tinky Winky",
		"Eric",
		"George",
		"Mike",
		"Mark",
		"Fin",
		"Guy",
		"Ollie",
		"Simon",
		"Samantha",
		"Biggus Dickus",
		"Sarah",
		"Miranda"
	};

	CBYTE session_name[NET_NAME_LENGTH];

	//
	// Choose a name...
	//

	my_name = name[rand() % NUM_NAMES];

	CNET_network_game = FALSE;
	CNET_i_am_host    = FALSE;
	CNET_num_players  = 1;
	CNET_player_id    = 0;
#ifndef	PSX
	if (!CNET_connected)
	{
		if (Keys[KB_ESC])
		{
			Keys[KB_ESC] = 0;

			return FALSE;
		}

		//
		// Choose connection.
		//

	  choose_connection:;

		num_connections = NET_get_connection_number();

		AENG_clear_screen();

		if (AENG_lock())
		{
			FONT_draw(50, 30, "Available connections");

			for (i = 0; i < num_connections; i++)
			{
				x = 50;
				y = 50 + i * 10;

				FONT_draw(x, y, "%d : %s", i + 1, NET_get_connection_name(i));
			}

			AENG_unlock();
			AENG_flip();
		}
		else
		{
			return FALSE;
		}

		while(1)
		{
			SHELL_ACTIVE;

			if (Keys[KB_ESC])
			{
				Keys[KB_ESC] = 0;

				return FALSE;
			}

			for (i = 0; i < num_connections; i++)
			{
				if (Keys[KB_1 + i])
				{
					Keys[KB_1 + i] = 0;

					//
					// Try to establish connection 'i'.
					//

					if (NET_connection_make(i))
					{
						CNET_connected = TRUE;

						goto choose_session;
					}

					else
					{
						//
						// Failed to make a conection.
						//

						CNET_display_error("Failed to make a connection");

						goto choose_connection;
					}
				}
			}
		}
	}

  choose_session:;

	while(1)
	{
		SHELL_ACTIVE;

		if (Keys[KB_ESC])
		{
			Keys[KB_ESC] = 0;

			return FALSE;
		}

		//
		// Look for available sessions.
		//

		num_sessions = NET_get_session_number();

		AENG_clear_screen();

		if (AENG_lock())
		{
			FONT_draw(50, 30, "Choose a session number");

			if (i == 0)
			{
				FONT_draw(50, 50, "No sessions available");
			}

			for (i = 0; i < num_sessions; i++)
			{
				x = 50;
				y = 50 + i * 10;

				FONT_draw(x, y, "Session %d : %s", i + 1, NET_get_session_info(i).name);
			}

			x = 50;
			y = 50 + (i + 1) * 10;

			FONT_draw(x, y, "Press 'H' to host a new game.");

			AENG_unlock();
		}

		AENG_flip();

		if (Keys[KB_H])
		{
			Keys[KB_H] = 0;

			//
			// Make up a name for this session.
			//

			word1 = rand() % NUM_WORDS;

			do
			{
				word2 = rand() % NUM_WORDS;

			} while (word2 == word1);

			_snprintf(session_name, NET_NAME_LENGTH, "%s %s", word[word1], word[word2]);

			//
			// Create the new session.
			//

			if (NET_create_session(session_name, 4, my_name))
			{
				CNET_i_am_host = TRUE;

				goto joined_session;
			}
			else
			{
				CNET_display_error("Could not create sessions");
			}
		}
		else
		{
			for (i = 0; i < num_sessions; i++)
			{
				if (Keys[KB_1 + i])
				{
					Keys[KB_1 + i] = 0;

					if (NET_join_session(i, my_name))
					{
						goto joined_session;
					}
					else
					{
						CNET_display_error("Could not join session");
					}
				}
			}
		}
	}

  joined_session:;

	while(1)
	{
		SHELL_ACTIVE;

		if (Keys[KB_ESC])
		{
			Keys[KB_ESC] = 0;

			//
			// Leave the session.
			//

			NET_leave_session();

			return FALSE;
		}

		//
		// List all the players in the session.
		//

		num_players = NET_get_num_players();

		AENG_clear_screen();

		if (AENG_lock())
		{

			if (CNET_i_am_host)
			{
				FONT_draw(50, 30, "Press space to start session: %s", session_name);
			}
			else
			{
				FONT_draw(50, 30, "Waiting for host to start session: %s", session_name);
			}

			for (i = 0; i < num_players; i++)
			{
				x = 50;
				y = 50 + i * 10;

				FONT_draw(x, y, "\t%s", NET_get_player_name(i));
			}

			AENG_unlock();
		}

		AENG_flip();

		if (CNET_i_am_host)
		{
			if (Keys[KB_SPACE])
			{
				Keys[KB_SPACE] = 0;

				CNET_player_id = NET_start_game();

				if (CNET_player_id != NET_PLAYER_NONE)
				{
					CNET_network_game = TRUE;
					CNET_num_players  = num_players;

					return FALSE;
				}
			}
		}

		TRACE("CNET sizeof NET_Message = %d\n", sizeof(NET_Message));

		//
		// Process the message loop.
		//

		while(NET_message_waiting())
		{
			NET_message_get(&mess);

			if (mess.player_id == NET_PLAYER_SYSTEM)
			{
				TRACE("NET system message recieved\n");

				if (mess.system.sysmess == NET_SYSMESS_START_GAME)
				{
					CNET_player_id    = mess.system.player_id;
					CNET_num_players  = num_players;
					CNET_network_game = TRUE;

					return FALSE;
				}

				if (mess.system.sysmess == NET_SYSMESS_LOST_CONNECTION ||
					mess.system.sysmess == NET_SYSMESS_HOST_QUIT_OUT)
				{
					//
					// Leave the session and choose another one.
					//

					NET_leave_session();

					goto choose_session;
				}
			}
		}
	}
#endif
}



#endif
#endif

