//
// A player.
//

#include "always.h"
#include "key.h"
#include "font.h"
#include "game.h"
#include "gamestate.h"
#include "log.h"
#include "net.h"
#include "os.h"
#include "player.h"
#include "server.h"
#include "ship.h"


//
// The keypresses of all the players.
//

PLAYER_Key PLAYER_key[PLAYER_MAX_PLAYERS];
SLONG      PLAYER_key_start;
SLONG      PLAYER_key_gameturn;

//
// The messages relating to players joining and leaving the game.
//

PLAYER_Message *PLAYER_message[PLAYER_NUM_MESSAGES];
SLONG           PLAYER_message_start;
SLONG           PLAYER_message_gameturn;

//
// The gamestate into the past.
//

PLAYER_Gamestate PLAYER_gamestate[PLAYER_NUM_GAMESTATES];
SLONG            PLAYER_gamestate_start;
SLONG            PLAYER_gamestate_gameturn;


//
// Our local SHIP_ship \ PLAYER_key index.
//

SLONG PLAYER_local_index;




void PLAYER_init()
{
	memset(PLAYER_key,       0, sizeof(PLAYER_key      ));
	memset(PLAYER_message,   0, sizeof(PLAYER_message  ));
	memset(PLAYER_gamestate, 0, sizeof(PLAYER_gamestate));

	PLAYER_key_start    = -1;
	PLAYER_key_gameturn = -1;

	PLAYER_message_start    = -1;
	PLAYER_message_gameturn = -1;

	PLAYER_gamestate_start    = -1;
	PLAYER_gamestate_gameturn = -1;
}



void PLAYER_new_gameturn(SLONG gameturn)
{
	if (PLAYER_gamestate_start == -1)
	{
		PLAYER_gamestate_start    = gameturn;
		PLAYER_gamestate_gameturn = gameturn;
	}

	if (WITHIN(gameturn, PLAYER_gamestate_start, PLAYER_gamestate_gameturn))
	{
		//
		// Overwrite an existing gamestate.
		//
	}
	else
	{
		//
		// We should never have to remember so long in the past!
		//

		ASSERT(gameturn >= PLAYER_gamestate_start);

		PLAYER_gamestate_gameturn = gameturn;

		if (PLAYER_gamestate_start < PLAYER_gamestate_gameturn - (PLAYER_NUM_GAMESTATES - 1))
		{
			PLAYER_gamestate_start = PLAYER_gamestate_gameturn - (PLAYER_NUM_GAMESTATES - 1);
		}
	}

	//
	// Store gamestate.
	//

	GAMESTATE_store(&PLAYER_gamestate[gameturn & (PLAYER_NUM_GAMESTATES - 1)].gs);

	PLAYER_gamestate[gameturn & (PLAYER_NUM_GAMESTATES - 1)].gameturn = gameturn;
}

void PLAYER_restore_gamestate(SLONG gameturn)
{
	ASSERT(WITHIN(gameturn, PLAYER_gamestate_start, PLAYER_gamestate_gameturn));
	ASSERT(PLAYER_gamestate[gameturn & (PLAYER_NUM_GAMESTATES - 1)].gameturn == gameturn);

	GAMESTATE_restore(&PLAYER_gamestate[gameturn & (PLAYER_NUM_GAMESTATES - 1)].gs);
}




SLONG PLAYER_create_local(
		CBYTE *name,
		UBYTE  red,
		UBYTE  green,
		UBYTE  blue,
		float  ship_mass,
		float  ship_power)
{
	SLONG num_bytes;
	void *data;

	//
	// Say what we're doing...
	//

	OS_clear_screen();
	OS_scene_begin();
	FONT_draw(0.5F, 0.4F, 0xffffff, 1.0F, -1, "Awaiting gamestate...");
	OS_scene_end();
	OS_show();

	//
	// Send a message to the server requesting to join the game.
	//

	LOG_message(0xffffff, "Requesting to joing game");

	struct
	{
		SLONG                     gameturn;
		SERVER_Block_request_join join;

	} join_message;

	join_message.gameturn   = GAME_turn;
	join_message.join.type  = SERVER_BLOCK_TYPE_REQUEST_JOIN;
	join_message.join.red   = red;
	join_message.join.green = green;
	join_message.join.blue  = blue;
	join_message.join.mass  = ship_mass;
	join_message.join.power = ship_power;

	//
	// Send this message guaranteed.
	// 

	NET_player_message_send(sizeof(join_message), &join_message, TRUE);

	//
	// Wait until the server responds with a copy of gamestate.
	//

	SLONG start = OS_ticks();

	while(1)
	{
		//
		// Keep our gameturns up-to-date.
		//

		while(GAME_tick < OS_ticks())
		{
			GAME_process += 1;
			GAME_tick    += GAME_TICKS_PER_PROCESS;
			GAME_turn     = GAME_process >> 4;
		}

		//
		// Poll for the gamestate received message.
		//

		switch(NET_player_message_receive(&num_bytes, &data))
		{
			case NET_PLAYER_MESSAGE_NONE:
				
				if (OS_ticks() > start + 4 * 1000)
				{
					//
					// More than 4 seconds!
					//

					return PLAYER_CREATE_TIMED_OUT;
				}

				break;

			case NET_PLAYER_MESSAGE_LOST_CONNECTION:
				return PLAYER_CREATE_LOST_CONNECTION;

			case NET_PLAYER_MESSAGE_FROM_SERVER:

				//
				// We should only ever get a GAMESTATE message.
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
								case SERVER_BLOCK_TYPE_GAMESTATE:

									{
										SERVER_Block_gamestate *sbg = (SERVER_Block_gamestate *) upto;

										LOG_message(0xffffff, "Got gamestate");

										//
										// Initialise all the state.
										//

										memset(PLAYER_key,       0, sizeof(PLAYER_key      ));
										memset(PLAYER_message,   0, sizeof(PLAYER_message  ));
										memset(PLAYER_gamestate, 0, sizeof(PLAYER_gamestate));

										PLAYER_key_start    = -1;
										PLAYER_key_gameturn = -1;

										PLAYER_message_start    = -1;
										PLAYER_message_gameturn = -1;

										PLAYER_gamestate_start    = -1;
										PLAYER_gamestate_gameturn = -1;

										//
										// We know one of the gamestates...
										//

										PLAYER_gamestate_start    = sbg->gameturn;
										PLAYER_gamestate_gameturn = sbg->gameturn;

										PLAYER_gamestate[sbg->gameturn & (PLAYER_NUM_GAMESTATES - 1)].gameturn = sbg->gameturn;
										PLAYER_gamestate[sbg->gameturn & (PLAYER_NUM_GAMESTATES - 1)].gs       = sbg->gs;

										//
										// We don't know about any keypresses or messages.
										//

										PLAYER_key_start    = sbg->gameturn;
										PLAYER_key_gameturn = sbg->gameturn;

										PLAYER_message_start    = sbg->gameturn;
										PLAYER_message_gameturn = sbg->gameturn;

										//
										// Restore gamestate.
										//

										GAMESTATE_restore(&sbg->gs);

										//
										// This is the gameturn we are upto...
										//

										{
											SLONG dprocess = (sbg->gameturn << 4) - GAME_process;

											GAME_turn    = sbg->gameturn;
											GAME_process = sbg->gameturn << 4;
											GAME_tick   += dprocess * GAME_TICKS_PER_PROCESS;
										}

										//
										// This is the order of events...
										//
										//	GAME_turn += 1;
										//  SHIP_set_active()
										//  Get keypresses.
										//  Process game...
										//
										// So subtract one from the process. Get it?
										//

										GAME_process -= 1;
										GAME_turn    -= 1;
										GAME_tick    -= GAME_TICKS_PER_PROCESS;

										PLAYER_key_start    -= 1;
										PLAYER_key_gameturn -= 1;

										PLAYER_message_start    -= 1;
										PLAYER_message_gameturn -= 1;

										goto got_gamestate;
									}

									break;

								case SERVER_BLOCK_TYPE_PING:

									//
									// We can ignore these now...
									//

									upto += sizeof(SERVER_Block_ping);

									break;

								default:
									ASSERT(0);
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
	}

  got_gamestate:;

	//
	// Tell the server that we've got gamestate. Send the data guaranteed.
	//

	LOG_message(0xffffff, "Sending acknoledgement that I received gamestate.");

	struct
	{
		SLONG                           gameturn;
		SERVER_Block_received_gamestate received;

	} got_it;

	got_it.gameturn      = GAME_turn;
	got_it.received.type = SERVER_BLOCK_TYPE_RECEIVED_GAMESTATE;

	NET_player_message_send(sizeof(got_it), &got_it, TRUE);

	return PLAYER_CREATE_OK;
}





SLONG PLAYER_process(SLONG *rollback, SLONG ignore_server_messages)
{
	SLONG i;
	SLONG j;
	SLONG num_bytes;
	SLONG rollback_happened;

	void       *data;
	PLAYER_Key *pk;
	SHIP_Ship  *ss;

	UBYTE local;

	//
	// Rollback only happens we get a different keypress before PLAYER_key_gameturn,
	// not GAME_turn. They should only ever be one different!
	//

	ASSERT(PLAYER_key_gameturn == GAME_turn - 1);

   *rollback          = PLAYER_key_gameturn;
    rollback_happened = FALSE;

	//
	// Clear out the new keypresses and update the start of the keypress history.
	//

	while (PLAYER_key_gameturn < GAME_turn)
	{
		PLAYER_key_gameturn += 1;

		for (i = 0; i < PLAYER_MAX_PLAYERS; i++)
		{
			PLAYER_key[i].key[PLAYER_key_gameturn & (PLAYER_NUM_KEYS - 1)] = 0;
		}

		if (PLAYER_key_start <= PLAYER_key_gameturn - PLAYER_NUM_KEYS)
		{
			//
			// The circular queue is going to overwrite an old keypress.
			//

			PLAYER_key_start = PLAYER_key_gameturn - (PLAYER_NUM_KEYS - 1);
		}
	}

	//
	// Update the keypress history.
	//

	local = 0;

	if (KEY_on[KEY_Z]) {local |= PLAYER_KEY_LEFT; }
	if (KEY_on[KEY_X]) {local |= PLAYER_KEY_RIGHT;}
	if (KEY_on[KEY_V]) {local |= PLAYER_KEY_THRUST;}

	if (KEY_on[KEY_G])
	{
		KEY_on[KEY_G] = 0;

		local |= PLAYER_KEY_TRACTOR_BEAM;
	}

	for (i = 0; i < PLAYER_MAX_PLAYERS; i++)
	{
		pk = &PLAYER_key[i];
		ss = &SHIP_ship [i];

		if (ss->flag & SHIP_FLAG_USED)
		{
			if (ss->flag & SHIP_FLAG_LOCAL)
			{
				//
				// This is a local player.
				//

				if (ss->flag & SHIP_FLAG_ACTIVE)
				{
					pk->key[GAME_turn & (PLAYER_NUM_KEYS - 1)] = local;
				}
				else
				{
					//
					// A non-active player can't press his keys.
					//

					pk->key[GAME_turn & (PLAYER_NUM_KEYS - 1)] = 0;
				}
			}
			else
			{
				if (ss->flag & SHIP_FLAG_ACTIVE)
				{
					UBYTE last;

					if (ss->active == GAME_turn)
					{
						//
						// The keypresses start off blank.
						//

						last = 0;
					}
					else
					{
						//
						// Guess the keys that the remote player has pressed down.
						// The last gameturns keypresses are a good starting point!
						//

						last = pk->key[(GAME_turn - 1) & (PLAYER_NUM_KEYS - 1)];

						//
						// Assume remote player's key stay the same- except for tractor
						// beam, which we assume stays on for only one gameturn.
						//

						last &= ~PLAYER_KEY_TRACTOR_BEAM;
					}

					pk->key[GAME_turn & (PLAYER_NUM_KEYS - 1)] = last;
				}
				else
				{
					//
					// A non-active player can't press his keys.
					//

					pk->key[GAME_turn & (PLAYER_NUM_KEYS - 1)] = 0;
					
				}
			}
		}
	}

	//
	// Send the server our keypresses.
	//

	ASSERT(WITHIN(PLAYER_local_index, 0, SHIP_MAX_SHIPS - 1));

	if (SHIP_ship[PLAYER_local_index].flag & SHIP_FLAG_ACTIVE)
	{
		SLONG num_turns;

		num_turns = GAME_turn - SHIP_ship[PLAYER_local_index].active + 1;

		if (num_turns > 32)
		{
			num_turns = 32;
		}

		if (num_turns > 0)
		{
			PLAYER_Key *pk = &PLAYER_key[PLAYER_local_index];

			struct
			{
				#define PLAYER_MAX_NUM_KEYS 256

				SLONG                         gameturn;
				union
				{
					SERVER_Block_my_keypress_list sbm;
					UBYTE                         room[sizeof(SERVER_Block_my_keypress_list) + 2 * PLAYER_MAX_NUM_KEYS];
				};

			} my_keys;

			my_keys.gameturn       = GAME_turn;
			my_keys.sbm.type       = SERVER_BLOCK_TYPE_MY_KEYPRESS_LIST;
			my_keys.sbm.num_keys   = num_turns;
			my_keys.sbm.ship_index = PLAYER_local_index;
			my_keys.sbm.hash       = SHIP_ship[PLAYER_local_index].x * SHIP_ship[PLAYER_local_index].y + SHIP_ship[PLAYER_local_index].angle;

			for (i = 0; i < num_turns; i++)
			{
				my_keys.sbm.key[i] = pk->key[(GAME_turn - i) & (PLAYER_NUM_KEYS - 1)];
			}

			NET_player_message_send(sizeof(SLONG) + sizeof(SERVER_Block_my_keypress_list) + my_keys.sbm.num_keys, &my_keys);
		}
	}

	//
	// Free up the old wrapped-around messages.
	//

	while(PLAYER_message_gameturn < GAME_turn)
	{
		PLAYER_message_gameturn += 1;

		if (PLAYER_message_start <= PLAYER_message_gameturn - PLAYER_NUM_MESSAGES)
		{
			//
			// The circular queue is going to overwrite an old message.
			//

			PLAYER_message_start = PLAYER_message_gameturn - (PLAYER_NUM_MESSAGES - 1);
		}

		//
		// free() up the old messages from the last time this
		// gameturn was used.
		//

		while(PLAYER_message[PLAYER_message_gameturn & (PLAYER_NUM_MESSAGES - 1)])
		{
			void *free_me_up;
			
			free_me_up                                                          = PLAYER_message[PLAYER_message_gameturn & (PLAYER_NUM_MESSAGES - 1)];
			PLAYER_message[PLAYER_message_gameturn & (PLAYER_NUM_MESSAGES - 1)] = PLAYER_message[PLAYER_message_gameturn & (PLAYER_NUM_MESSAGES - 1)]->next;

			free(free_me_up);
		}
	}

	if (!ignore_server_messages)
	{
		//
		// Get the server messages.
		//

		while(1)
		{
			//
			// Now process network messages. We might have to change the
			// keypresses of the remote players.
			//

			switch(NET_player_message_receive(&num_bytes, &data))
			{
				case NET_PLAYER_MESSAGE_NONE:

					//
					// No more messages.
					//

					goto no_more_messages;

				case NET_PLAYER_MESSAGE_LOST_CONNECTION:

					//
					// Arggh! We can't find the server anymore.
					//

				   *rollback          = 0;
					rollback_happened = FALSE;

					return FALSE;

				case NET_PLAYER_MESSAGE_FROM_SERVER:

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
									case SERVER_BLOCK_TYPE_REQUEST_JOIN:
										ASSERT(0);
										break;

									case SERVER_BLOCK_TYPE_NEW_PLAYER:

										{
											SERVER_Block_new_player *sbnp = (SERVER_Block_new_player *) upto;

											LOG_message(0xffffff, "Got new player %d packet", sbnp->ship_index);

											//
											// Create a new player message structure and put this message in
											// the player message queue.
											//

											PLAYER_Message *pm = (PLAYER_Message *) malloc(sizeof(PLAYER_Message));

											pm->new_player                                        = *sbnp;
											pm->next                                              =  PLAYER_message[GAME_turn & (PLAYER_NUM_MESSAGES - 1)];
											PLAYER_message[GAME_turn & (PLAYER_NUM_MESSAGES - 1)] =  pm;

											LOG_message(0xffff00, "New player active %d", sbnp->active);

											upto += sizeof(SERVER_Block_new_player);
										}

										break;

									case SERVER_BLOCK_TYPE_MY_KEYPRESS_LIST:
										ASSERT(0);
										break;

									case SERVER_BLOCK_TYPE_REMOTE_KEYPRESS_LIST:
										
										{
											SLONG  i;
											SLONG  j;
											SLONG  turn;
											UBYTE  key;
											UBYTE  repeat;

											SHIP_Ship  *ss;
											PLAYER_Key *pk;

											SERVER_Block_remote_keypress_list *sbk = (SERVER_Block_remote_keypress_list *) upto;

											ASSERT(WITHIN(sbk->ship_index, 0, SHIP_MAX_SHIPS - 1));

											ss = &SHIP_ship[sbk->ship_index];

											if (!(ss->flag & SHIP_FLAG_USED))
											{
												//
												// This is a keypress list for a ship that we haven't
												// created yet!
												//
											}
											else
											{
												//
												// Overwrite the keypresses we guessed with the keypresses that
												// actaully hapenned. If there is a discrepency, then we must
												// roll back to that gameturn and reprocess everything.
												//

												pk = &PLAYER_key[sbk->ship_index];

												for (i = 0; i < sbk->num_keys; i++)
												{
													turn = sbk->gameturn - i;

													if (WITHIN(turn, PLAYER_key_start, PLAYER_key_gameturn))
													{
														if (pk->key[turn & (PLAYER_NUM_KEYS - 1)] != sbk->key[i])
														{
															if (turn <= *rollback)
															{
																//
																// We guessed wrong! We must rollback.
																//

															   *rollback          = turn;
																rollback_happened = TRUE;

																ASSERT(*rollback < GAME_turn);

																if (i == 0)
																{
																	//
																	// Update the guessing...
																	//

																	for (j = sbk->gameturn + 1; j <= GAME_turn; j++)
																	{
																		//
																		// Assume the keys we don't know are the same as the
																		// last key press we got. Except the tractor beam.
																		//

																		pk->key[j & (PLAYER_NUM_KEYS - 1)] = sbk->key[0] & ~PLAYER_KEY_TRACTOR_BEAM;
																	}
																}
															}
														}

														pk->key[turn & (PLAYER_NUM_KEYS - 1)] = sbk->key[i];
													}
												}
											}

											upto += sizeof(SERVER_Block_remote_keypress_list);
											upto += sizeof(UBYTE) * sbk->num_keys;
										}

										break;

									case SERVER_BLOCK_TYPE_PING:
										ASSERT(0);
										break;

									default:
										ASSERT(0);
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
		}
 
	  no_more_messages:;
	}

	if (!rollback_happened)
	{
		//
		// 0 means no rollback.
		//

		*rollback = 0;
	}
	else
	{
		ASSERT(*rollback < GAME_turn);

		//
		// Rollback gamestate.
		//

		PLAYER_restore_gamestate(*rollback);
	}

	return TRUE;
}



void PLAYER_process_messages(SLONG gameturn)
{
	ASSERT(WITHIN(gameturn, PLAYER_message_start, PLAYER_message_gameturn));

	PLAYER_Message *pm = PLAYER_message[gameturn & (PLAYER_NUM_MESSAGES - 1)];

	while(pm)
	{
		switch(pm->type)
		{	
			case SERVER_BLOCK_TYPE_NEW_PLAYER:

				{
					SHIP_Ship *ss;

					ASSERT(WITHIN(pm->new_player.ship_index, 0, SHIP_MAX_SHIPS - 1));

					ss = &SHIP_ship[pm->new_player.ship_index];

					memset(ss, 0, sizeof(SHIP_Ship));

					ss->flag   = SHIP_FLAG_USED;
					ss->active = pm->new_player.active;
					ss->red    = pm->new_player.red;
					ss->green  = pm->new_player.green;
					ss->blue   = pm->new_player.blue;
					ss->mass   = pm->new_player.mass;
					ss->power  = pm->new_player.power;
					ss->x      = pm->new_player.x;
					ss->y      = pm->new_player.y;
					ss->angle  = 0.0F;
					
					if (pm->new_player.local)
					{
						//
						// This is the local player.
						//

						ss->flag |= SHIP_FLAG_LOCAL;

						PLAYER_local_index = pm->new_player.ship_index;
					}

					//
					// Up until the gameturn before this ship goes active, all it's
					// keypresses are zero.
					//

					memset(&PLAYER_key[pm->new_player.ship_index], 0, sizeof(PLAYER_Key));

					PLAYER_key[pm->new_player.ship_index].known = ss->active - 1;
				}

				break;

			case SERVER_BLOCK_TYPE_PLAYER_LEFT:
				break;

			default:
				ASSERT(0);
				break;
		}

		pm = pm->next;
	}
}



void PLAYER_press_keys(SLONG gameturn)
{
	SLONG i;

	PLAYER_Key *pk;	
	SHIP_Ship  *ss;

	for (i = 0; i < PLAYER_MAX_PLAYERS; i++)
	{
		ss = &SHIP_ship[i];

		if (ss->flag & SHIP_FLAG_USED)
		{
			ss->flag &= ~SHIP_FLAG_KEY_LEFT;
			ss->flag &= ~SHIP_FLAG_KEY_RIGHT;
			ss->flag &= ~SHIP_FLAG_KEY_THRUST;
			ss->flag &= ~SHIP_FLAG_KEY_TRACTOR_BEAM;

			if (gameturn < ss->active)
			{
				//
				// No keys presses before is goes active.
				//
			}
			else
			{
				pk = &PLAYER_key[i];

				if (pk->key[gameturn & (PLAYER_NUM_KEYS - 1)] & PLAYER_KEY_LEFT        ) {ss->flag |= SHIP_FLAG_KEY_LEFT        ;}
				if (pk->key[gameturn & (PLAYER_NUM_KEYS - 1)] & PLAYER_KEY_RIGHT       ) {ss->flag |= SHIP_FLAG_KEY_RIGHT       ;}
				if (pk->key[gameturn & (PLAYER_NUM_KEYS - 1)] & PLAYER_KEY_THRUST      ) {ss->flag |= SHIP_FLAG_KEY_THRUST      ;}
				if (pk->key[gameturn & (PLAYER_NUM_KEYS - 1)] & PLAYER_KEY_TRACTOR_BEAM) {ss->flag |= SHIP_FLAG_KEY_TRACTOR_BEAM;}
			}
		}
	}
}


