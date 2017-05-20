//
// For establishing a common time with the server.
//

#include "always.h"
#include "font.h"
#include "game.h"
#include "net.h"
#include "os.h"
#include "ping.h"
#include "server.h"



//
// The ping message we send to and get from the server.
//

typedef struct
{
	SLONG             gameturn;
	SERVER_Block_ping ping;

} PING_Message;


//
// Whenever we get a valid ping back from the server...
//

typedef struct
{
	SLONG os_ticks_sent;		// The OS_ticks() when we sent the message
	SLONG os_ticks_recieved;	// The OS_ticks() when we recieved the message
	SLONG game_process;			// The game_process from the server when he sent the message.
	SLONG delta;				// The number so that '<game_process on server> = (<local OS_ticks()> / 4) + delta'
 
} PING_Sample;

#define PING_MAX_SAMPLES 30

PING_Sample PING_sample[PING_MAX_SAMPLES];
SLONG       PING_sample_upto;



SLONG PING_do()
{
	SLONG i;

	SLONG sent = 0;
	SLONG last = 0;
	SLONG now  = 0;

	SLONG ping;
	SLONG delta;

	SLONG num_bytes;
	void *data;

	PING_Message ping_message;
	PING_Sample *ps;

	//
	// Say what we're doing...
	//

	OS_clear_screen();
	OS_scene_begin();
	FONT_draw(0.5F, 0.4F, 0xffffff, 1.0F, -1, "Connecting...");
	OS_scene_end();
	OS_show();

	//
	// Clear out any old samples.
	//

	memset(PING_sample, 0, sizeof(PING_sample));

	PING_sample_upto = 0;

	while(1)
	{
		//
		// Send 10 pings to the server each second.
		//

		now = OS_ticks();

		if (now < last + (1000 / 10))
		{
			//
			// Don't send another message yet.
			//
		}
		else
		{
			last = now;

			//
			// Build the ping message.
			//

			ping_message.gameturn          = GAME_turn;
			ping_message.ping.type         = SERVER_BLOCK_TYPE_PING;
			ping_message.ping.id           = OS_ticks();
			ping_message.ping.game_process = GAME_process;

			NET_player_message_send(sizeof(ping_message), &ping_message);

			sent += 1;
		}

		//
		// Recieve messages.
		//

		SLONG exit_loop = FALSE;

		while(!exit_loop)
		{
			switch(NET_player_message_receive(&num_bytes, &data))
			{
				case NET_PLAYER_MESSAGE_NONE:
					exit_loop = TRUE;
					break;

				case NET_PLAYER_MESSAGE_LOST_CONNECTION:
					return FALSE;

				case NET_PLAYER_MESSAGE_FROM_SERVER:

					//
					// We've got a message back from the server.
					//
					
					{
						PING_Message *pm = (PING_Message *) data;

						ASSERT(pm->ping.type == SERVER_BLOCK_TYPE_PING);
						ASSERT(num_bytes == sizeof(PING_Message)); // This is the only message we should recieve.

						//
						// Create a new sample.
						//

						ASSERT(WITHIN(PING_sample_upto, 0, PING_MAX_SAMPLES - 1));

						now = OS_ticks();

						ping  = now - pm->ping.id;

						if (ping > 500)
						{
							//
							// Too great a ping! More than  a second is really bad! Ignore this sample.
							//
						}
						else
						{
							delta = pm->ping.game_process - ((now - ping / 2) / GAME_TICKS_PER_PROCESS);

							PING_sample[PING_sample_upto].os_ticks_sent     = pm->ping.id;
							PING_sample[PING_sample_upto].os_ticks_recieved = now;
							PING_sample[PING_sample_upto].game_process      = pm->ping.game_process;
							PING_sample[PING_sample_upto].delta             = delta;

							PING_sample_upto += 1;

							if (PING_sample_upto >= PING_MAX_SAMPLES)
							{
								//
								// We are now in a position work out our game turn...
								//

								delta = 0;

								for (i = 0; i < PING_MAX_SAMPLES; i++)
								{
									ps = &PING_sample[i];

									delta += ps->delta;
								}

								delta /= PING_MAX_SAMPLES;

								now = OS_ticks();

								GAME_process = (now / GAME_TICKS_PER_PROCESS) + delta;
								GAME_turn    = GAME_process >> 4;
								GAME_tick    = now;

								return TRUE;
							}
						}
					}

					break;
				
				default:
					ASSERT(0);
					break;
			}
		}

		if (sent > 150)
		{
			//
			// We've sent loads of messages to the server and haven't made
			// a connection yet... so give up.
			//

			return FALSE;
		}
	}
}






