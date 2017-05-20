//
// Direct play stuff.
//

#include <dplay.h>
#include <dplobby.h>
#include <stdlib.h>

#include "always.h"
#include "net.h"



//
// Our unique application GUID.
//

static const GUID NET_application_guid = { 0xfb222ea3, 0x6565, 0x11d3, { 0xa7, 0xd8, 0x0, 0xa0, 0xc9, 0x1d, 0x60, 0x9e } };



//
// The main interface to DirectPlay4 and the lobby interface we need
// to create addresses.
// 

LPDIRECTPLAY4A      NET_dp;
LPDIRECTPLAYLOBBY3A	NET_lobby;


//
// The server player and the local player.
//

DPID NET_player_server;
DPID NET_player_local;


//
// The buffer into which messages are received.
//

UBYTE NET_buffer[NET_MAX_MESSAGE_LENGTH];


//
// The current connection.
//

#define NET_CONNECTION_NONE     0
#define NET_CONNECTION_LAN      1
#define NET_CONNECITON_INTERNET 2

SLONG NET_connection;




SLONG NET_init(void)
{
	HRESULT res;

	//
	// Initialise the COM module.
	//

	CoInitialize(NULL);

	//
	// Create the DirectPlay3 interface.
	// 

	res = CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay4A, (LPVOID*)&NET_dp);

	if (res != S_OK)
	{
		CoUninitialize();

		return FALSE;
	}

	res = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlayLobby3A, (LPVOID *) &NET_lobby);

	if (res != S_OK)
	{
		CoUninitialize();

		return FALSE;
	}

	return TRUE;
}


void NET_kill(void)
{
	if (NET_dp)
	{
		NET_dp->Release();
	}

	if (NET_lobby)
	{
		NET_lobby->Release();
	}

	//
	// Get rid of the tossy COM module.
	//

	CoUninitialize();
}





SLONG NET_connection_lan()
{
	DPCOMPOUNDADDRESSELEMENT address_element[1];
	SLONG                    num_elements;

	void *address;
	ULONG address_size;

	HRESULT res;

	if (NET_connection == NET_CONNECTION_LAN)
	{
		//
		// Already connected to the LAN!
		//

		return TRUE;
	}

	//
	// Create an IPX address.
	//

	address_element[0].guidDataType = DPAID_ServiceProvider;
	address_element[0].dwDataSize   = sizeof(GUID);
	address_element[0].lpData       = (LPVOID) &DPSPGUID_IPX;

	num_elements = 1;

	//
	// How big is the address going to be?
	//

	res = NET_lobby->CreateCompoundAddress(address_element, num_elements, NULL, &address_size);

	if (res != DPERR_BUFFERTOOSMALL)
	{
		return FALSE;
	}

	//
	// Allocate memory for the address.
	//

	address = (void *) malloc(address_size);

	//
	// Actually create the address now.
	//

	res = NET_lobby->CreateCompoundAddress(address_element, num_elements, address, &address_size);

	if (res != DP_OK)
	{
		return FALSE;
	}

	//
	// Make the connection.
	//

	res = NET_dp->InitializeConnection(address, 0);
	
	if (res != DP_OK)
	{
		return FALSE;
	}

	NET_connection = NET_CONNECTION_LAN;

	return TRUE;
}









SLONG NET_session_create(CBYTE *session_name, SLONG max_players)
{
	SLONG i;

	UBYTE   ans;
	HRESULT res;

	DPNAME         dpname;
	DPSESSIONDESC2 session;

	memset(&session, 0, sizeof(session));
	
	session.dwSize           = sizeof(session);
	session.dwFlags          = DPSESSION_CLIENTSERVER | DPSESSION_KEEPALIVE | DPSESSION_OPTIMIZELATENCY | DPSESSION_DIRECTPLAYPROTOCOL;
	session.guidApplication  = NET_application_guid;
	session.dwMaxPlayers     = max_players + 1;	// Add one because the server counts as a player.
	session.lpszSessionNameA = session_name;

	res = NET_dp->Open(&session, DPOPEN_CREATE);

	if (res == DP_OK)
	{
		//
		// Create a server player.
		//

		res = NET_dp->CreatePlayer(
			   &NET_player_server,
				NULL,	// No player name.
				NULL,	// No event,
				NULL,	// No data,
				NULL,	// No data,
				DPPLAYER_SERVERPLAYER);
		
		if (res == DP_OK)
		{
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
	CBYTE name[NET_MAX_NAME_LENGTH + 8];
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

	ns->num_players     = lpThisSD->dwCurrentPlayers;	// Subtract one because the server counts as a player.
	ns->max_players     = lpThisSD->dwMaxPlayers - 1;	// Subtract one because the server counts as a player.
	ns->guidInstance    = lpThisSD->guidInstance;
	ns->guidApplication = lpThisSD->guidApplication;

	sprintf(ns->name, "%s (%d/%d)", lpThisSD->lpszSessionNameA, ns->num_players, ns->max_players);

	NET_session_upto += 1;

	return TRUE;
}

SLONG NET_session_get_number()
{
	DPSESSIONDESC2 desc;

	memset(&desc, 0, sizeof(desc));

	desc.dwSize          = sizeof(desc);
	desc.guidApplication = NET_application_guid;

	NET_session_upto = 0;

	NET_dp->EnumSessions(&desc, 0, NET_enum_sessions, NULL, 0);

	return NET_session_upto;
}

NET_Sinfo NET_session_get_info(SLONG session)
{
	NET_Sinfo ans;

	ASSERT(WITHIN(session, 0, NET_MAX_SESSIONS - 1));

	strncpy(ans.name, NET_session[session].name, NET_MAX_NAME_LENGTH - 1);

	ans.num_players = NET_session[session].num_players;
	ans.max_players = NET_session[session].max_players;

	return ans;
}


SLONG NET_session_join(SLONG session)
{
	HRESULT        res;
	DPSESSIONDESC2 desc;

	ASSERT(WITHIN(session, 0, NET_MAX_SESSIONS - 1));

	//
	// Describe the session we want to join.
	//

	memset(&desc, 0, sizeof(desc));

	desc.dwSize       = sizeof(desc);
	desc.guidInstance = NET_session[session].guidInstance;
	
	res = NET_dp->Open(&desc, DPOPEN_JOIN);

	if (res == DP_OK)
	{
		//
		// Create a player.
		//

		res = NET_dp->CreatePlayer(
				&NET_player_local,
				 NULL,	// No name.
				 NULL,	// No event.
				 NULL,	// No data.
				 NULL,	// No data.
				 0);	// No special flags.
		
		if (res == DP_OK)
		{
			return TRUE;
		}
		else
		{
			NET_dp->Close();

			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}

void NET_session_destroy()
{
	NET_dp->Close();
}

void NET_session_leave()
{
	NET_dp->Close();
}




void NET_player_message_send(SLONG num_bytes, void *data, SLONG guaranteed)
{
	HRESULT res;

	UBYTE *u = (UBYTE *) data;

	if (u[4] == 4 && num_bytes == 6)
	{
		u = u + 1;
	}

	/*

	if (!guaranteed)
	{
		if (rand() & 0x1)
		{
			return;
		}
	}

	*/

	res = NET_dp->Send(NET_player_local, DPID_SERVERPLAYER, (guaranteed) ? DPSEND_GUARANTEED : 0, data, num_bytes);
}



SLONG NET_player_message_receive(SLONG *num_bytes, void **data)
{
  tail_recurse:;

	ULONG buffer_length = NET_MAX_MESSAGE_LENGTH;

	DPID from;
	DPID to;

	HRESULT res;

	res = NET_dp->Receive(&from, &to, 0, (void *) NET_buffer, &buffer_length);

	if (res == DPERR_NOMESSAGES)
	{	
		//
		// There were no pending messages in the receive queue.
		//

		return NET_PLAYER_MESSAGE_NONE;
	}

	ASSERT(res == DP_OK);

	if (from == DPID_SERVERPLAYER)
	{
		//
		// This is a message from the server.
		//

	   *num_bytes = buffer_length;
	   *data      = (void *) NET_buffer;

		return NET_PLAYER_MESSAGE_FROM_SERVER;
	}

	if (from == DPID_SYSMSG)
	{
		//
		// We have got a system message.
		//

		DPMSG_GENERIC *sysmsg = (DPMSG_GENERIC *) NET_buffer;

		if (sysmsg->dwType == DPSYS_SESSIONLOST)
		{
			return NET_PLAYER_MESSAGE_LOST_CONNECTION;
		}
		else
		{
			//
			// Ignore all other system messages?
			//

			goto tail_recurse;
		}
	}

	//
	// We've got a message from some random entity!
	//

	ASSERT(0);

	return NET_PLAYER_MESSAGE_NONE;
}






void NET_server_message_to_player(NET_Player player, SLONG num_bytes, void *data, SLONG guaranteed)
{
	HRESULT res;

	/*

	if (!guaranteed)
	{
		if (rand() & 0x1)
		{
			return;
		}
	}

	*/

	res = NET_dp->Send(NET_player_server, (DPID) player, (guaranteed) ? DPSEND_GUARANTEED : 0, data, num_bytes);
}



SLONG NET_server_message_receive(NET_Player *player, SLONG *num_bytes, void **data)
{
  tail_recurse:;

	ULONG buffer_length = NET_MAX_MESSAGE_LENGTH;

	DPID from;
	DPID to;

	HRESULT res;

	res = NET_dp->Receive(&from, &to, 0, (void *) NET_buffer, &buffer_length);

	if (res == DPERR_NOMESSAGES)
	{	
		//
		// There were no pending messages in the receive queue.
		//

		return NET_SERVER_MESSAGE_NONE;
	}

	ASSERT(res == DP_OK);

	if (from == DPID_SYSMSG)
	{
		//
		// We have got a system message.
		//

		DPMSG_GENERIC *sysmsg = (DPMSG_GENERIC *) NET_buffer;

		switch(sysmsg->dwType)
		{
			case DPSYS_SESSIONLOST:
				return NET_SERVER_MESSAGE_LOST_CONNECTION;

			case DPSYS_CREATEPLAYERORGROUP:
				
				{
					DPMSG_CREATEPLAYERORGROUP *cpmsg = (DPMSG_CREATEPLAYERORGROUP *) sysmsg;

					//
					// We don't support groups...
					//

					ASSERT(cpmsg->dwPlayerType == DPPLAYERTYPE_PLAYER);

				   *player = (SLONG) cpmsg->dpId;
					
					return NET_SERVER_MESSAGE_PLAYER_JOINED;
				}

			case DPSYS_DESTROYPLAYERORGROUP:
				
				{
					DPMSG_DESTROYPLAYERORGROUP *dpmsg = (DPMSG_DESTROYPLAYERORGROUP *) sysmsg;

					//
					// We don't support groups...
					//

					ASSERT(dpmsg->dwPlayerType == DPPLAYERTYPE_PLAYER);

				   *player = (SLONG) dpmsg->dpId;
					
					return NET_SERVER_MESSAGE_PLAYER_LEFT;
				}

			default:
		
				//
				// Ignore all other system messages?
				//

				goto tail_recurse;
		}
	}
	else
	{
		//
		// This is a message from a player.
		//

	   *player    = (SLONG) from;
	   *num_bytes = buffer_length;
	   *data      = (void *) NET_buffer;

		return NET_SERVER_MESSAGE_FROM_PLAYER;
	}


	//
	// We've got a message from some random entity!
	//

	ASSERT(0);

	return NET_SERVER_MESSAGE_NONE;
}




