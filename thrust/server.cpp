//
// The server.
//

#include "always.h"
#include "font.h"
#include "game.h"
#include "land.h"
#include "log.h"
#include "net.h"
#include "os.h"
#include "player.h"
#include "server.h"





//
// The info we store for each player.
//

#define SERVER_PLAYER_FLAG_USED        (1 << 0)
#define SERVER_PLAYER_FLAG_OUT_OF_SYNC (1 << 1)

#define SERVER_PLAYER_MODE_CONNECTED		0	// Just joined.
#define SERVER_PLAYER_MODE_REQUESTED		1	// Requested joining- awaiting gamestate.
#define SERVER_PLAYER_MODE_SENT_GAMESTATE	2	// Has been sent a copy of gamestate.
#define SERVER_PLAYER_MODE_PLAYING			3	// Received gamestate- awaiting activation.

typedef struct
{
	UBYTE      flag;
	UBYTE      mode;
	UBYTE      red;
	UBYTE      green;
	UBYTE      blue;
	UBYTE      ship_index;	// The index into the SHIP_ship array.
	UBYTE      padding[2];
	NET_Player player_id;
	float      mass;
	float      power;
	CBYTE      name[32];

	//
	// The keypresses we know about for sure.
	//

	#define SERVER_MAX_KEYS 256

	UBYTE key[SERVER_MAX_KEYS];
	SLONG gameturn;				// The gameturn we know all the keys for.
	SLONG active;				// The gameturn when this ship became active.
	float hash;					// The hash value at 'gameturn'

} SERVER_Player;

#define SERVER_MAX_PLAYERS PLAYER_MAX_PLAYERS

SERVER_Player SERVER_player[SERVER_MAX_PLAYERS];


//
// The gameturn we've processed the world upto.
//

SLONG SERVER_turn;



//
// Who we are connecting or -1 if we are not connecting anybody.
//

SLONG SERVER_connecting;	





SLONG SERVER_session_create(
		CBYTE *name,
		SLONG  max_players,
		SLONG  connection_type,
		CBYTE *internet_address)
{
	ASSERT(max_players <= SERVER_MAX_PLAYERS);

	switch(connection_type)
	{
		case SERVER_CONNECT_TYPE_LAN:

			if (!NET_connection_lan())
			{
				return FALSE;
			}

			break;

		case SERVER_CONNECT_TYPE_INTERNET:
			return FALSE;

		default:
			ASSERT(0);
			return FALSE;
	}

	if (NET_session_create(name, max_players))
	{
		//
		// Initialise a new game.
		//

		//
		// No players to start with...
		//

		memset(SERVER_player, 0, sizeof(SERVER_player));

		//
		// Start off not connecting anyone.
		//

		SERVER_connecting = -1;
		SERVER_turn       =  0;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}




void SERVER_process()
{
	SLONG i;
	SLONG j;
	SLONG num_bytes;

	SERVER_Player *sp;
	SERVER_Player *sp_2;
	SHIP_Ship     *ss;
	NET_Player     player;
	void          *data;

	switch(NET_server_message_receive(&player, &num_bytes, &data))
	{
		case NET_SERVER_MESSAGE_NONE:
			break;

		case NET_SERVER_MESSAGE_LOST_CONNECTION:
			LOG_message(0xffffff, "Lost connection");
			break;

		case NET_SERVER_MESSAGE_PLAYER_LEFT:
			LOG_message(0xffffff, "Player %d disconnected", player);
			break;

		case NET_SERVER_MESSAGE_PLAYER_JOINED:

			{
				SLONG i;

				//
				// Look for a free player.
				//

				for (i = 0; i < SERVER_MAX_PLAYERS; i++)
				{
					if (SERVER_player[i].flag & SERVER_PLAYER_FLAG_USED)
					{
						continue;
					}

					memset(&SERVER_player[i], 0, sizeof(SERVER_Player));

					SERVER_player[i].flag      = SERVER_PLAYER_FLAG_USED;
					SERVER_player[i].player_id = player;
					SERVER_player[i].mode      = SERVER_PLAYER_MODE_CONNECTED;

					LOG_message(0xffffff, "Player %d connected", player);

					return;
				}

				//
				// WHAT?!
				//

				ASSERT(0);
			}

			break;

		case NET_SERVER_MESSAGE_FROM_PLAYER:

			// LOG_message(0xffffff, "Message from player %d (%d bytes long)", player, num_bytes);

			//
			// Process the message.
			//

			{
				SLONG exit_loop = FALSE;

				UBYTE *upto     = ((UBYTE *) data) + 4;
				SLONG  gameturn = ((SLONG *) data)[0];

				while(!exit_loop)
				{
					if (upto - ((UBYTE *) data) >= num_bytes)
					{
						//
						// Finished processing the message.
						//

						break;
					}
					else
					{
						switch(*upto)
						{
							case SERVER_BLOCK_TYPE_PING:

								//
								// Send a ping message straight back!
								//

								{
									SERVER_Block_ping *sbp = (SERVER_Block_ping *) upto;

									LOG_message(0xffffff, "Ping %d", sbp->id);

									struct
									{
										SLONG             gameturn;
										SERVER_Block_ping ping;

									} ping_message;

									ping_message.gameturn          = GAME_turn;
									ping_message.ping.type         = SERVER_BLOCK_TYPE_PING;
									ping_message.ping.id           = sbp->id;
									ping_message.ping.game_process = GAME_process;

									NET_server_message_to_player(
										player,
										sizeof(ping_message),
									   &ping_message);

									upto += sizeof(SERVER_Block_ping);
								}

								break;

							case SERVER_BLOCK_TYPE_REQUEST_JOIN:

								{
									SERVER_Block_request_join *sbj = (SERVER_Block_request_join *) upto;

									//
									// The player is requesting to join the game. Look for the player
									// in our player structure.
									//

									SLONG i;

									SERVER_Player *sp;

									for (i = 0; i < SERVER_MAX_PLAYERS; i++)
									{
										sp = &SERVER_player[i];

										if ((sp->flag & SERVER_PLAYER_FLAG_USED) && sp->player_id == player)
										{
											goto found_player;
										}
									}

									ASSERT(0);

								  found_player:;

									LOG_message(0xffffff, "Request Join from player %d", i);

									//
									// Fill in the player details.
									//

									sp->red   = sbj->red;
									sp->blue  = sbj->blue;
									sp->green = sbj->green;
									sp->mass  = sbj->mass;
									sp->power = sbj->power;
									
									strncpy(sp->name, sbj->name, 31);

									//
									// The player id is the same as the server_player index.
									//

									sp->ship_index = sp - SERVER_player;
									
									//
									// Set the mode of the player to REQUESTED. When no
									// other players are joining, this player will be
									// sent a copy of gamestate.
									//

									sp->mode = SERVER_PLAYER_MODE_REQUESTED;

									upto += sizeof(SERVER_Block_request_join);
								}

								break;

							case SERVER_BLOCK_TYPE_RECEIVED_GAMESTATE:

								{
									SLONG i;
									SLONG me;

									SERVER_Player *sp;
									SERVER_Player *sp_me;

									//
									// Which SERVER_Player has sent us this message?
									//

									for (i = 0; i < SERVER_MAX_PLAYERS; i++)
									{
										sp = &SERVER_player[i];

										if (sp->flag & SERVER_PLAYER_FLAG_USED)
										{	
											if (sp->player_id == player)
											{
												//
												// Found the person who sent us the message.
												//

												me    = i;
												sp_me = sp;

												goto found_him;
											}
										}
									}

									ASSERT(0);

								  found_him:;

									LOG_message(0xffffff, "Received acknoledgement of receipt of gamestate from player %d", me);

									//
									// This is the gameturn he wlil become active.
									//

									sp_me->mode     = SERVER_PLAYER_MODE_PLAYING;
									sp_me->active   = GAME_turn + 16 * 2;
									sp_me->gameturn = sp_me->active - 2;

									memset(&sp_me->key, 0, sizeof(sp_me->key));

									//
									// Send out a message to all the players, telling them about the
									// new ship joining.
									//

									for (i = 0; i < SERVER_MAX_PLAYERS; i++)
									{
										sp = &SERVER_player[i];

										if (sp->flag & SERVER_PLAYER_FLAG_USED)
										{
											switch(sp->mode)
											{
												case SERVER_PLAYER_MODE_CONNECTED:

													//
													// Hasn't been sent gamestate yet, so he doesn't
													// need to know about the new player.
													//

													break;

												case SERVER_PLAYER_MODE_SENT_GAMESTATE:
												case SERVER_PLAYER_MODE_PLAYING:

													{

														//
														// Send this player a new_player message.
														//

														struct
														{
															SLONG                   gameturn;
															SERVER_Block_new_player sbn;

														} new_player;

														new_player.gameturn       =  GAME_turn;
														new_player.sbn.type       =  SERVER_BLOCK_TYPE_NEW_PLAYER;
														new_player.sbn.red        =  sp_me->red;
														new_player.sbn.green      =  sp_me->green;
														new_player.sbn.blue       =  sp_me->blue;
														new_player.sbn.mass       =  sp_me->mass;
														new_player.sbn.power      =  sp_me->power;
														new_player.sbn.x          =  0.0F;
														new_player.sbn.y          = -2.0F;
														new_player.sbn.ship_index =  sp_me->ship_index;
														new_player.sbn.local      =  (i == me);
														new_player.sbn.active     =  sp_me->active;

														strncpy(new_player.sbn.name, sp_me->name, 31);

														new_player.sbn.name[31] = '\000';

														NET_server_message_to_player(sp->player_id, sizeof(new_player), &new_player, TRUE);

														//
														// Create the new ship locally.
														//

														memset(&SHIP_ship[sp_me->ship_index], 0, sizeof(SHIP_Ship));

														SHIP_ship[sp_me->ship_index].red    = new_player.sbn.red;
														SHIP_ship[sp_me->ship_index].green  = new_player.sbn.green;
														SHIP_ship[sp_me->ship_index].blue   = new_player.sbn.blue;
														SHIP_ship[sp_me->ship_index].active = new_player.sbn.active;
														SHIP_ship[sp_me->ship_index].x      = new_player.sbn.x;
														SHIP_ship[sp_me->ship_index].y      = new_player.sbn.y;
														SHIP_ship[sp_me->ship_index].flag   = SHIP_FLAG_USED;
														SHIP_ship[sp_me->ship_index].mass   = new_player.sbn.mass;
														SHIP_ship[sp_me->ship_index].power  = new_player.sbn.power;

														LOG_message(0xffff00, "New player active %d", new_player.sbn.active);
													}

													break;

												default:
													ASSERT(0);
													break;
											}
										}
									}

									upto += sizeof(SERVER_Block_received_gamestate);
								}

								break; 
							
							case SERVER_BLOCK_TYPE_MY_KEYPRESS_LIST:

								{
									SLONG i;

									SERVER_Block_my_keypress_list *sbk = (SERVER_Block_my_keypress_list *) upto;

									//LOG_message(0xffffff, "Keypresses from player %d turn %d", sbk->ship_index, gameturn);

									ASSERT(WITHIN(sbk->ship_index, 0, SERVER_MAX_PLAYERS - 1));
									ASSERT((SERVER_player[sbk->ship_index].flag & SERVER_PLAYER_FLAG_USED) && SERVER_player[sbk->ship_index].mode == SERVER_PLAYER_MODE_PLAYING);

									SERVER_Player *sp = &SERVER_player[sbk->ship_index];
									
									//
									// Update the player structure.
									//

									sp->gameturn = gameturn;	// We know the keys upto this gameturn now.
									sp->hash     = sp->hash;

									for (i = 0; i < sbk->num_keys; i++)
									{
										sp->key[(gameturn - i) & (SERVER_MAX_KEYS - 1)] = sbk->key[i];
									}

									upto += sizeof(SERVER_Block_my_keypress_list);
									upto += sbk->num_keys;
								}

								break;

							default:
								LOG_message(0xffffff, "Unknown block type.");
								exit_loop = TRUE;
								break;
						}
					}
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Connecting players...
	//

	{
		SLONG i;

		SERVER_Player *sp;

		if (SERVER_connecting == -1)
		{
			//
			// We aren't in the process of connecting. Look for a player
			// who has requested joining.
			//

			for (i = 0; i < SERVER_MAX_PLAYERS; i++)
			{	
				sp = &SERVER_player[i];

				if ((sp->flag & SERVER_PLAYER_FLAG_USED) && sp->mode == SERVER_PLAYER_MODE_REQUESTED)
				{
					SERVER_connecting = i;

					LOG_message(0xffffff, "Connecting player %d", SERVER_connecting);

					break;
				}
			}
		}

		if (SERVER_connecting != -1)
		{
			//
			// Process this person joining.
			//

			ASSERT(WITHIN(SERVER_connecting, 0, SERVER_MAX_PLAYERS - 1));

			sp = &SERVER_player[SERVER_connecting];

			switch(sp->mode)
			{	
				case SERVER_PLAYER_MODE_CONNECTED:
					ASSERT(0);
					break;

				case SERVER_PLAYER_MODE_REQUESTED:

					{
						LOG_message(0xffffff, "Sending gamestate to player %d", SERVER_connecting);

						struct
						{
							SLONG                  gameturn;
							SERVER_Block_gamestate sbg;

						} send_gamestate;

						//
						// Send the player a copy of gamestate.
						//

						send_gamestate.gameturn     = GAME_turn;
						send_gamestate.sbg.type     = SERVER_BLOCK_TYPE_GAMESTATE;
						send_gamestate.sbg.gameturn = SERVER_turn + 1;

						GAMESTATE_store(&send_gamestate.sbg.gs);

						NET_server_message_to_player(sp->player_id, sizeof(send_gamestate), &send_gamestate, TRUE);

						//
						// Change the mode.
						//

						sp->mode = SERVER_PLAYER_MODE_SENT_GAMESTATE;
					}

					break;

				case SERVER_PLAYER_MODE_SENT_GAMESTATE:

					//
					// We are waiting for the player to send us a RECEIVED_GAMESTATE message.
					//

					break;

				case SERVER_PLAYER_MODE_PLAYING:

					//
					// Finished connecting this player.
					//

					LOG_message(0xffffff, "Finished connecting player %d", SERVER_connecting);

					SERVER_connecting = -1;

					break;

				default:
					ASSERT(0);
					break;
			}
		}
	}

	//
	// Send out the keypresses to all the players.
	//

	{
		UBYTE *upto;
		UBYTE  buffer[1024];

		memset(buffer, 0, sizeof(buffer));

		SLONG *slong = (SLONG *) buffer;

	   *slong = GAME_turn;

		for (i = 0; i < SERVER_MAX_PLAYERS; i++)
		{
			sp = &SERVER_player[i];
			
			if ((sp->flag & SERVER_PLAYER_FLAG_USED) && sp->mode == SERVER_PLAYER_MODE_PLAYING)
			{
				upto  = buffer + 4;

				for (j = 0; j < SERVER_MAX_PLAYERS; j++)
				{
					if (j == i)
					{
						//
						// Don't send a players messages to himself.
						//

						continue;
					}

					sp_2 = &SERVER_player[j];

					//
					// Start building the remote keypress message.
					//

					if ((sp_2->flag & SERVER_PLAYER_FLAG_USED) && sp->mode == SERVER_PLAYER_MODE_PLAYING)
					{
						SLONG k;
						SLONG num_turns;

						SERVER_Block_remote_keypress_list *sbr = (SERVER_Block_remote_keypress_list *) upto;

						num_turns = sp_2->gameturn - sp_2->active;

						if (num_turns > 32)
						{
							num_turns = 32;
						}

						if (num_turns > 0)
						{
							sbr->type       = SERVER_BLOCK_TYPE_REMOTE_KEYPRESS_LIST;
							sbr->num_keys   = num_turns;
							sbr->ship_index = sp_2->ship_index;
							sbr->gameturn   = sp_2->gameturn;

							for (k = 0; k < num_turns; k++)
							{
								sbr->key[k] = sp_2->key[(sp_2->gameturn - k) & (SERVER_MAX_KEYS - 1)];
							}

							upto += sizeof(SERVER_Block_remote_keypress_list);
							upto += num_turns;
						}
					}
				}

				if (upto != buffer + 4)
				{
					NET_server_message_to_player(sp->player_id, upto - buffer, buffer);
				}
			}
		}
	}

	//
	// What gameturn do we know all the keypresses upto?
	//

	SLONG upto = GAME_turn;

	for (i = 0; i < SERVER_MAX_PLAYERS; i++)
	{
		sp = &SERVER_player[i];

		if ((sp->flag & SERVER_PLAYER_FLAG_USED) && sp->mode == SERVER_PLAYER_MODE_PLAYING)
		{
			if (sp->gameturn < upto)
			{
				upto = sp->gameturn;
			}
		}
	}

	//
	// Process the game upto this gameturn.
	//

	while(SERVER_turn < upto)
	{								   
		SERVER_turn += 1;

		//
		// Check for out-of-sync.
		//

		for (i = 0; i < SERVER_MAX_PLAYERS; i++)
		{
			sp = &SERVER_player[i];

			if ((sp->flag & SERVER_PLAYER_FLAG_USED) && sp->mode == SERVER_PLAYER_MODE_PLAYING)
			{
				if ((SERVER_turn & 0x3f) == 0)
				{
					SHIP_Ship *ss = &SHIP_ship[sp->ship_index];

					sp->hash = ss->x + ss->y + ss->angle;
				}
			}
		}

		//
		// Mark all ships that go active this gameturn.
		//

		SHIP_flag_active(SERVER_turn);

		//
		// Update the keypresses in the players.
		//

		for (i = 0; i < SERVER_MAX_PLAYERS; i++)
		{
			sp = &SERVER_player[i];

			if ((sp->flag & SERVER_PLAYER_FLAG_USED) && sp->mode == SERVER_PLAYER_MODE_PLAYING)
			{
				ASSERT(SERVER_turn <= sp->gameturn);
				ASSERT(WITHIN(sp->ship_index, 0, SHIP_MAX_SHIPS - 1));

				ss = &SHIP_ship[sp->ship_index];

				ss->flag &= ~SHIP_FLAG_KEY_LEFT;
				ss->flag &= ~SHIP_FLAG_KEY_RIGHT;
				ss->flag &= ~SHIP_FLAG_KEY_THRUST;
				ss->flag &= ~SHIP_FLAG_KEY_TRACTOR_BEAM;

				if (sp->key[SERVER_turn & (SERVER_MAX_KEYS - 1)] & PLAYER_KEY_LEFT        ) {ss->flag |= SHIP_FLAG_KEY_LEFT        ;}
				if (sp->key[SERVER_turn & (SERVER_MAX_KEYS - 1)] & PLAYER_KEY_RIGHT       ) {ss->flag |= SHIP_FLAG_KEY_RIGHT       ;}
				if (sp->key[SERVER_turn & (SERVER_MAX_KEYS - 1)] & PLAYER_KEY_THRUST      ) {ss->flag |= SHIP_FLAG_KEY_THRUST      ;}
				if (sp->key[SERVER_turn & (SERVER_MAX_KEYS - 1)] & PLAYER_KEY_TRACTOR_BEAM) {ss->flag |= SHIP_FLAG_KEY_TRACTOR_BEAM;}

				if (ss->flag & SHIP_FLAG_KEY_THRUST)
				{
					LOG_message(0x00ff00, "Thrust %d", SERVER_turn);
				}
			}
		}

		if (SHIP_ship[0].flag & SHIP_FLAG_ACTIVE)
		{
			LOG_file("SHIP_process_one: SERVER_turn %3d key %3d (%f,%f) %f\n", SERVER_turn, SHIP_ship[0].flag, SHIP_ship[0].x, SHIP_ship[0].y, SHIP_ship[0].angle);
		}

		//
		// Sixteen processes per gameturn!
		//

		for (i = 0; i < 16; i++)
		{
			SHIP_process_all();
			ORB_process_all();
			TB_process_all();
		}
	}
}



void SERVER_draw()
{
	SLONG i;

	static SLONG last;
	static SLONG now;

	now = OS_ticks();

	if (now < last + (1000 / 20))
	{
		//
		// Don't draw more than 20 times a second.
		//

		return;
	}

	last = now;

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

	FONT_draw(0.052F, 0.052F, 0x000000, 0.7F, -1, "Gameturn %4d: Server turn: %4d Real turn: %d", GAME_turn >> 5, SERVER_turn >> 5, GAME_turn);
	FONT_draw(0.050F, 0.050F, 0xff0000, 0.7F, -1, "Gameturn %4d: Server turn: %4d Real turn: %d", GAME_turn >> 5, SERVER_turn >> 5, GAME_turn);
	FONT_draw(0.600F, 0.700F, 0xffff00, 1.7F, -1, "%f", SERVER_player[0].hash);
	FONT_draw(0.600F, 0.800F, 0xffff00, 1.7F, -1, "%f", SERVER_player[1].hash);
	FONT_draw(0.600F, 0.900F, 0xffff00, 1.7F, -1, "%f", SERVER_player[2].hash);

	/*

	for (i = 0; i < SERVER_MAX_PLAYERS; i++)
	{
		SERVER_Player *sp = &SERVER_player[i];
		
		if (sp->flag & SERVER_PLAYER_FLAG_USED)
		{
			if (sp->flag & SERVER_PLAYER_FLAG_OUT_OF_SYNC)
			{
				FONT_draw(0.6F, 0.2F + (i * 0.05F), 0x3344ff, 0.4F, -1, "Player %d out of sync", sp->ship_index);
			}
		}
	}

	*/
	
	LOG_draw();

	OS_scene_end();
	OS_show();
}

