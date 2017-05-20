//
// Direct play stuff.
//

#include "DDLib.h"
#include "net.h"


//
// Our private cunning messages...
//

#define NET_CUNNING_TYPE_DPID		(177773.055F)
#define NET_CUNNING_TYPE_START_GAME	(559225.123F)

typedef struct
{
	float type;
	CBYTE filling[16];
	DPID  dpid;
	SLONG player_id;
	CBYTE padding[20];
	SLONG must_be_12345;
	CBYTE crap[16];
	SLONG must_be_314159;
	CBYTE shit[8];

} NET_Cunning;


//
// Error checking...
//

#ifndef VERIFY
#ifdef  NDEBUG
#define VERIFY(x) {x;}
#else
#define VERIFY(x) {ASSERT(x);}
#endif
#endif


//
// The main interface to DirectPlay3
// 

LPDIRECTPLAY3A NET_dp;

//
// TRUE if this machine is the host.
// The local player number.
// 

UBYTE NET_i_am_host;
UBYTE NET_local_player;

//
// All the players.
//

typedef struct
{
	UBYTE used;
	UBYTE player_id;
	UWORD shit;
	DPID  dpid;
	CBYTE name[NET_NAME_LENGTH];
	
} NET_Player;

#define NET_MAX_PLAYERS 32

NET_Player NET_player[NET_MAX_PLAYERS];

//
// The buffer into which messages are recieved.
//

CBYTE NET_buffer[NET_MESSAGE_LENGTH];

SLONG net_init_done=0;

void NET_init(void)
{
	HRESULT res;
	ASSERT(0);

	if(net_init_done==0)
	{
		net_init_done=1;
	}
	else
	{
		return;
	}
	//
	// Initialise the COM module.
	//

#ifdef TARGET_DC
	CoInitializeEx(NULL,COINIT_MULTITHREADED);	
#else
	CoInitialize(NULL);
#endif

	//
	// Create the DirectPlay3 interface.
	// 

	res = CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay3A, (LPVOID*)&NET_dp);

	if (res != S_OK)
	{
		NET_dp = NULL;
	}
}


void NET_kill(void)
{
	if(net_init_done)
	{
		ASSERT(0);
		if (NET_dp)
		{
			NET_dp->Release();
		}

		//
		// Get rid of the tossy COM module.
		//

		CoUninitialize();
	}
}


//
// All the avaliable connections.
//

typedef struct
{
	LPCGUID guid;
	void   *connection;
	SLONG   connection_size;
	CBYTE   name[NET_NAME_LENGTH];
	
} NET_Connection;

#define NET_MAX_CONNECTIONS 8

NET_Connection NET_connection[NET_MAX_CONNECTIONS];
SLONG          NET_connection_upto;

//
// The enumeration function for connections.
//

BOOL FAR PASCAL NET_enum_connections(
					LPCGUID   lpguidSP,
					LPVOID    lpConnection,
					DWORD     dwConnectionSize,
					LPCDPNAME lpName,
					DWORD     dwFlags,
					LPVOID    lpContext)
{
	if (!WITHIN(NET_connection_upto, 0, NET_MAX_CONNECTIONS - 1))
	{
		//
		// No more connections please!
		//

		return FALSE;
	}

	NET_Connection *nc = &NET_connection[NET_connection_upto++];

	nc->connection = MemAlloc(dwConnectionSize);

	if (nc->connection)
	{
		nc->guid            = lpguidSP;
		nc->connection_size = dwConnectionSize;

		memcpy(nc->connection, lpConnection, dwConnectionSize);
		strncpy(nc->name, (CBYTE *) lpName->lpszShortNameA, NET_NAME_LENGTH - 1);
	}
	else
	{
		//
		// No more memory???!!!
		//

		return FALSE;
	}
	
	//
	// Continue the enumeration please.
	//

	return TRUE;
}


SLONG NET_get_connection_number()
{
	SLONG i;

	//
	// Free up all the memory the connection have used up so far.
	//

	for (i = 0; i < NET_connection_upto; i++)
	{
		if (NET_connection[i].connection)
		{
			MemFree(NET_connection[i].connection);
		}
	}

	//
	// Enumerate all the connections.
	//

	NET_connection_upto = 0;

	VERIFY(NET_dp->EnumConnections(NULL, NET_enum_connections, NULL, DPCONNECTION_DIRECTPLAY) == DP_OK);

	return NET_connection_upto;
}

CBYTE *NET_get_connection_name(SLONG connection)
{
	ASSERT(WITHIN(connection, 0, NET_connection_upto - 1));

	return NET_connection[connection].name;
}


SLONG NET_connection_make(SLONG connection)
{
	HRESULT res;

	ASSERT(WITHIN(connection, 0, NET_connection_upto - 1));
	
	res = NET_dp->InitializeConnection(NET_connection[connection].connection, 0);
	
	if (res == DP_OK)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

SLONG NET_create_session(CBYTE *session_name, SLONG max_players, CBYTE *my_player_name)
{
	SLONG i;

	HRESULT res;

	DPNAME         dpname;
	DPSESSIONDESC2 session;

	InitStruct(session);

	session.dwFlags          = DPSESSION_KEEPALIVE;
	session.guidApplication  = GUID_NULL;
	session.dwMaxPlayers     = max_players;
	session.lpszSessionNameA = session_name;

	res = NET_dp->Open(&session, DPOPEN_CREATE);

	if (res == DP_OK)
	{
		//
		// Clear out old player info.
		//

		for (i = 0; i < NET_MAX_PLAYERS; i++)
		{
			NET_player[i].used = FALSE;
		}

		//
		// Create a local player.
		//

		InitStruct(dpname);

		dpname.lpszShortNameA = my_player_name;

		res = NET_dp->CreatePlayer(
				&NET_player[0].dpid,
				&dpname,
				 NULL,	// No event,
				 NULL,	// No data,
				 NULL,	// No data,
				 0);
		
		if (res == DP_OK)
		{
			NET_player[0].used      = TRUE;
			NET_player[0].player_id = NET_PLAYER_NONE;

			NET_i_am_host    = TRUE;
			NET_local_player = 0;

			return TRUE;
		}
		else
		{
			//
			// Close the session.
			//	

			NET_dp->Close();

			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

//
// All the available sessions
//

typedef struct
{
	CBYTE name[NET_NAME_LENGTH];
	SLONG num_players;
	SLONG max_players;
	GUID  guidInstance;
	GUID  guidApplication;
	
} NET_Session;

#define NET_MAX_SESSIONS 16

NET_Session NET_session[NET_MAX_SESSIONS];
SLONG       NET_session_upto;


//
// The session enumeration function.
//

BOOL FAR PASCAL NET_enum_sessions(
					LPCDPSESSIONDESC2 lpThisSD,
					LPDWORD           lpdwTimeOut,
					DWORD             dwFlags,
					LPVOID            lpContext)
{
	NET_Session *ns;

	if (dwFlags & DPESC_TIMEDOUT)
	{
		return FALSE;
	}

	if (!WITHIN(NET_session_upto, 0, NET_MAX_SESSIONS - 1))
	{
		return FALSE;
	}

	ns = &NET_session[NET_session_upto];

	ns->num_players     = lpThisSD->dwCurrentPlayers;
	ns->max_players     = lpThisSD->dwMaxPlayers;
	ns->guidInstance    = lpThisSD->guidInstance;
	ns->guidApplication = lpThisSD->guidApplication;

	strncpy(ns->name, lpThisSD->lpszSessionNameA, NET_NAME_LENGTH - 1);

	NET_session_upto += 1;

	return TRUE;
}

SLONG NET_get_session_number()
{
	DPSESSIONDESC2 desc;

	InitStruct(desc);

	desc.guidApplication = GUID_NULL;

	NET_session_upto = 0;

	NET_dp->EnumSessions(&desc, 0, NET_enum_sessions, NULL, 0);

	return NET_session_upto;
}

NET_Sinfo NET_get_session_info(SLONG session)
{
	NET_Sinfo ans;

	ASSERT(WITHIN(session, 0, NET_MAX_SESSIONS - 1));

	strncpy(ans.name, NET_session[session].name, NET_NAME_LENGTH - 1);

	return ans;
}

//
// The player enumeration function.
//

BOOL FAR PASCAL NET_enum_players(
					DPID      dpId,
					DWORD     dwPlayerType,
					LPCDPNAME lpName,
					DWORD     dwFlags,
					LPVOID    lpContext)
{
	SLONG i;

	if (dwPlayerType == DPPLAYERTYPE_PLAYER)
	{
		//
		// Look for a spare player structure.
		//

		for (i = 0; i < NET_MAX_PLAYERS; i++)
		{
			if (!NET_player[i].used)
			{
				NET_player[i].used      = TRUE;
				NET_player[i].dpid      = dpId;
				NET_player[i].player_id = NET_PLAYER_NONE;	// Dont know this yet.

				return TRUE;
			}
		}

		//
		// No more room for any more players.
		//

		return FALSE;
	}

	return TRUE;
}

SLONG NET_join_session(SLONG session, CBYTE *my_player_name)
{
	SLONG i;

	HRESULT res;

	DPNAME         dpname;
	DPSESSIONDESC2 desc;

	ASSERT(WITHIN(session, 0, NET_MAX_SESSIONS - 1));

	//
	// Describe the session we want to join.
	//

	InitStruct(desc);

	desc.guidInstance = NET_session[session].guidInstance;
	
	res = NET_dp->Open(&desc, DPOPEN_JOIN);

	if (res == DP_OK)
	{
		//
		// Find all other players in the session.
		//

		for (i = 0; i < NET_MAX_PLAYERS; i++)
		{
			NET_player[i].used = FALSE;
		}

		res = NET_dp->EnumPlayers(
						NULL,
						NET_enum_players,
						NULL,
						0);
		
		if (res != DP_OK)
		{
			NET_dp->Close();

			return FALSE;
		}

		for (i = 0; i < NET_MAX_PLAYERS; i++)
		{
			if (!NET_player[i].used)
			{
				NET_local_player = i;

				goto found_local_player;
			}
		}

		//
		// No space for a new player.
		//

		NET_dp->Close();

		return FALSE;

	  found_local_player:;

		//
		// Create a player.
		//

		InitStruct(dpname);

		dpname.lpszShortNameA = my_player_name;

		res = NET_dp->CreatePlayer(
				&NET_player[NET_local_player].dpid,
				&dpname,
				 NULL,	// No event,
				 NULL,	// No data,
				 NULL,	// No data,
				 0);	// No special flags.
		
		if (res == DP_OK)
		{
			NET_player[NET_local_player].used      = TRUE;
			NET_player[NET_local_player].player_id = NET_PLAYER_NONE;

			NET_i_am_host = FALSE;

			return TRUE;
		}
		else
		{
			//
			// Get rid of all players and leave the session.
			//

			for (i = 0; i < NET_MAX_PLAYERS; i++)
			{
				NET_player[i].used = FALSE;
			}

			NET_dp->Close();

			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}


UBYTE NET_start_game()
{
	SLONG  i;
	SLONG  player_upto;

	NET_Cunning *nc;

	HRESULT res;

	ASSERT(NET_i_am_host);
	ASSERT(WITHIN(NET_local_player, 0, NET_MAX_PLAYERS - 1));

	//
	// Assign the player ids.
	//

	player_upto = 0;

	for (i = 0; i < NET_MAX_PLAYERS; i++)
	{
		if (NET_player[i].used)
		{
			NET_player[i].player_id = player_upto++;
		}
	}
	
	//
	// Send a special chat message to the all the other players that
	// tells them their player_id.
	//

	for (i = 0; i < NET_MAX_PLAYERS; i++)
	{
		if (NET_player[i].used)
		{
			nc = (NET_Cunning *) NET_buffer;

			nc->type      = NET_CUNNING_TYPE_DPID;
			nc->dpid      = NET_player[i].dpid;
			nc->player_id = NET_player[i].player_id;

			nc->must_be_12345  = 12345;
			nc->must_be_314159 = 314159;

			res = NET_dp->Send(
						NET_player[NET_local_player].dpid,
						DPID_ALLPLAYERS,
						DPSEND_GUARANTEED,
					    nc,
						sizeof(NET_Cunning));

			if (res != DP_OK)
			{
				TRACE("Couldn't synchronisation message.\n");
			}
			else
			{
				TRACE("Send synchro message ok\n");
			}
		}
	}

	//
	// Send the start game message.
	//

	nc = (NET_Cunning *) NET_buffer;

	nc->type = NET_CUNNING_TYPE_START_GAME;

	nc->must_be_12345  = 12345;
	nc->must_be_314159 = 314159;

	res = NET_dp->Send(
				NET_player[NET_local_player].dpid,
				DPID_ALLPLAYERS,
				DPSEND_GUARANTEED,
				nc,
				sizeof(NET_Cunning));

	if (res != DP_OK)
	{
		TRACE("Couldn't start synchronisation message.\n");
	}
	else
	{
		TRACE("Send start synchro message ok\n");
	}

	return NET_player[NET_local_player].player_id;
}


void NET_leave_session()
{
	NET_dp->Close();
}


SLONG NET_get_num_players()
{
	SLONG i;
	SLONG ans = 0;

	#define NAME_BUFFER_SIZE 1024

	ULONG name_buffer_size;
	UBYTE name_buffer[NAME_BUFFER_SIZE];

	DPNAME *dpname;

	HRESULT res;

	for (i = 0; i < NET_MAX_PLAYERS; i++)
	{
		if (NET_player[i].used)
		{
			name_buffer_size = NAME_BUFFER_SIZE;

			res = NET_dp->GetPlayerName(NET_player[i].dpid, name_buffer, &name_buffer_size);

			if (res == DP_OK)
			{
				dpname = (DPNAME *) name_buffer;

				strncpy(NET_player[i].name, dpname->lpszShortNameA, NET_NAME_LENGTH - 1);
			}
			else
			{
				TRACE("Could not get player name. %d", i);

				strncpy(NET_player[i].name, "Unknown", NET_NAME_LENGTH - 1);
			}

			ans += 1;
		}
	}

	return ans;
}

CBYTE *NET_get_player_name(SLONG player)
{
	SLONG i;

	SLONG player_number = 0;

	for (i = 0; i < NET_MAX_PLAYERS; i++)
	{
		if (NET_player[i].used)
		{
			if (player == player_number)
			{
				return NET_player[i].name;
			}

			player_number += 1;
		}
	}

	return "Unknown";
}

void NET_message_send(
		UBYTE  player_id,
		void  *data,
		UWORD  num_bytes)
{
	SLONG i;
	DPID  to;

	ASSERT(WITHIN(NET_local_player, 0, NET_MAX_PLAYERS - 1));

	if (player_id == NET_PLAYER_ALL)
	{
		to = DPID_ALLPLAYERS;
	}
	else
	{
		//
		// Find the DPID for this player.
		//

		for (i = 0; i < NET_MAX_PLAYERS; i++)
		{
			if (NET_player[i].used)
			{
				if (NET_player[i].player_id == player_id)
				{
					to = NET_player[i].dpid;

					goto found_dpid;
				}
			}
		}

		//
		// Could not find anyone with this dpid.
		//

		ASSERT(0);
	}

  found_dpid:;

	NET_dp->Send(
				NET_player[NET_local_player].dpid,
				to,
				0,
				data,
				num_bytes);
}


SLONG NET_message_waiting()
{
	ULONG ans;

	HRESULT res;

	ASSERT(WITHIN(NET_local_player, 0, NET_MAX_PLAYERS - 1));
	
	res = NET_dp->GetMessageCount(
					NET_player[NET_local_player].dpid,
				   &ans);

	if (res == DP_OK)
	{
		if (ans)
		{
			TRACE("Message waiting\n");
		}

		return ans;
	}
	else
	{
		TRACE("Error getting message count\n");

		return 0;
	}
}

void NET_message_get(NET_Message *ans)
{
	SLONG i;

	DPID from;
	DPID to;
	ULONG num_bytes;

	SLONG match;
	DPID  dpid;
	SLONG player_id;

	NET_Player  *np;
	NET_Cunning *nc;

	DPMSG_GENERIC              *sm = (DPMSG_GENERIC *) NET_buffer;
	DPMSG_CREATEPLAYERORGROUP  *sp;
	DPMSG_DESTROYPLAYERORGROUP *sd;
	DPMSG_CHAT                 *sc;

	HRESULT res;

	ASSERT(WITHIN(NET_local_player, 0, NET_MAX_PLAYERS - 1));

	to        = NET_player[NET_local_player].dpid;
	num_bytes = NET_MESSAGE_LENGTH;

	res = NET_dp->Receive(
					&from,
					&to,
					 DPRECEIVE_TOPLAYER,
					 NET_buffer,
					&num_bytes);

	TRACE("Received %d bytes\n", num_bytes);
	TRACE("	DPMSG_GENERIC = %d bytes\n", sizeof(DPMSG_GENERIC));

	if (res == DP_OK)
	{
		TRACE("E\n");

		if (from == DPID_SYSMSG)
		{
			TRACE("System message.\n");

			//
			// Handle a system message.
			//

			switch(sm->dwType)
			{
				case DPSYS_CHAT:
					
					TRACE("Got chat message\n");

					sc = (DPMSG_CHAT *) NET_buffer;

					//
					// This could be one of our syschat messages.
					//

					match = sscanf(sc->lpChat->lpszMessageA, "SYSCHAT: DPID %d => player id %d", &dpid, &player_id);

					if (match == 2)
					{
						TRACE("Chat DPID message %d player_id %d\n", dpid, player_id);

						//
						// Find the player with the given dpid and set his
						// player id.
						//

						for (i = 0; i < NET_MAX_PLAYERS; i++)
						{
							if (NET_player[i].used)
							{
								if (NET_player[i].dpid == dpid)
								{
									NET_player[i].player_id = player_id;

									goto found_player;
								}
							}
						}

						//
						// No such player!
						//

						ASSERT(0);

					  found_player:;

						//
						// Create a NOP message.
						//			  

						ans->player_id      = NET_PLAYER_SYSTEM;
						ans->system.sysmess = NET_SYSMESS_NOP;
					}

					if (strncmp(sc->lpChat->lpszMessageA, "SYSCHAT: Start the game!", 24) == 0)
					{
						TRACE("Start game message\n");

						//
						// Create a START_GAME message.
						//

						ans->player_id      = NET_PLAYER_SYSTEM;
						ans->system.sysmess = NET_SYSMESS_START_GAME;
					}

					return;

				case DPSYS_CREATEPLAYERORGROUP:

					TRACE("C\n");

					sp = (DPMSG_CREATEPLAYERORGROUP *) NET_buffer;

					//
					// Find a spare player structure.
					//

					for (i = 0; i < NET_MAX_PLAYERS; i++)
					{
						np = &NET_player[i];

						if (!np->used)
						{
							//
							// Create the new player.
							//

							ASSERT(sp->dwPlayerType == DPPLAYERTYPE_PLAYER);

							np->used = TRUE;
							np->dpid = sp->dpId;

							goto found_spare_player;;
						}
					}

					ASSERT(0);

				  found_spare_player:;

					//
					// Create a NOP message.
					//			  

					ans->player_id      = NET_PLAYER_SYSTEM;
					ans->system.sysmess = NET_SYSMESS_NOP;

					return;

				case DPSYS_DESTROYPLAYERORGROUP:

					TRACE("B\n");


					sd = (DPMSG_DESTROYPLAYERORGROUP *) NET_buffer;

					//
					// Find this player.
					//

					for (i = 0; i < NET_MAX_PLAYERS; i++)
					{
						np = &NET_player[i];

						if (np->used && np->dpid == sd->dpId)
						{
							np->used = FALSE;

							goto destroyed_player;
						}
					}

					ASSERT(0);

				  destroyed_player:;

					if (i == NET_local_player)
					{
						//
						// I have been ejected from the game... create a 
						// LOST CONNECTION system message.
						//

						ans->player_id      = NET_PLAYER_SYSTEM;
						ans->system.sysmess = NET_SYSMESS_LOST_CONNECTION;
					}
					else
					{
						//
						// Create a NOP message.
						//			  

						ans->player_id      = NET_PLAYER_SYSTEM;
						ans->system.sysmess = NET_SYSMESS_NOP;
					}

					return;

				case DPSYS_SESSIONLOST:

					TRACE("A\n");

					//
					// Lost our connection...
					//

					ans->player_id      = NET_PLAYER_SYSTEM;
					ans->system.sysmess = NET_SYSMESS_LOST_CONNECTION;
					
					return;

				default:

					TRACE("Unknown system message %d\n", sm->dwType);

					//
					// Create a NULL message.
					//

					ans->player_id      = NET_PLAYER_SYSTEM;
					ans->system.sysmess = NET_SYSMESS_NOP;

					return;
			}
		}
		else
		{
			TRACE("User message.\n");

			if (num_bytes == sizeof(NET_Cunning))
			{
				TRACE("Size ok\n");

				nc = (NET_Cunning *) NET_buffer;

				if (nc->must_be_12345  == 12345 &&
					nc->must_be_314159 == 314159)
				{
					TRACE("Magic numbers ok\n");

					if (nc->type == NET_CUNNING_TYPE_DPID)
					{
						TRACE("NET CUNNING DPID\n");

						//
						// Set this players player_id...
						//

						for (i = 0; i < NET_MAX_PLAYERS; i++)
						{
							if (NET_player[i].used)
							{
								if (NET_player[i].dpid == nc->dpid)
								{
									NET_player[i].player_id = nc->player_id;

									if (i != NET_local_player)
									{

										//
										// Tell the user to ignore this message...
										//

										ans->player_id      = NET_PLAYER_SYSTEM;
										ans->system.sysmess = NET_SYSMESS_NOP;
									}
									else
									{
										//
										// Start the game once we know our player id.
										// 

										ans->player_id        = NET_PLAYER_SYSTEM;
										ans->system.sysmess   = NET_SYSMESS_START_GAME;
										ans->system.player_id = nc->player_id;
									}

									TRACE("Nop\n");

									return;
								}
							}
						}

						//
						// Oh dear!!! Maybe this wasn't a cunning message after all...
						//

						ASSERT(0);
					}
					else
					if (nc->type == NET_CUNNING_TYPE_START_GAME)
					{
						TRACE("Ignored start game message\n");

						//
						// Start the game!
						//

						ans->player_id      = NET_PLAYER_SYSTEM;
						ans->system.sysmess = NET_SYSMESS_NOP;

						return;
					}
				}
			}


			//
			// A message from another player.
			//

			for (i = 0; i < NET_MAX_PLAYERS; i++)
			{
				np = &NET_player[i];

				if (np->used)
				{
					if (np->dpid == from)
					{
						ans->player_id = np->player_id;

						goto found_player_id;
					}
				}
			}

			ASSERT(0);

		  found_player_id:;

			ans->player.num_bytes = num_bytes;
			ans->player.data      = NET_buffer;

			return;
		}
	}
	else
	{
		TRACE("Didn't receive messages properly\n");

		//
		// Create a NOP message.
		// 

		ans->player_id      = NET_PLAYER_SYSTEM;
		ans->system.sysmess = NET_SYSMESS_NOP;

		return;
	}
}










