//
// Network stuff...
//

#ifndef NET_H
#define NET_H



//
// The maximum length of message and name strings
//

#define NET_MAX_MESSAGE_LENGTH (4 * 1024)	// 4k
#define NET_MAX_NAME_LENGTH    (256)



// ========================================================
//
// Initialising and releasing all the DirectPlay stuff...
// Call once each at the start and end of the program.
// 
// ========================================================

SLONG NET_init(void);	// Returns FALSE on failure.
void  NET_kill(void);



// ========================================================
//
// CONNECTION STUFF
//
// ========================================================

//
// Establishes a connection or returns TRUE is the connection is
// already active.  Return FALSE on failure.
//

SLONG NET_connection_lan     (void);		// Makes a connection to the LAN
SLONG NET_connection_internet(CBYTE *str);	// The internet using TCP/IP give is an string like "124.15.239.24" or "www.muckyfoot.com"




// ========================================================
//
// SERVER SESSION STUFF
//
// ========================================================

//
// Creates a session. Returns TRUE on success.
//		 

SLONG NET_session_create (CBYTE *session_name, SLONG max_players);
void  NET_session_destroy(void);


// ========================================================
//
// PLAYER SESSION STUFF
//
// ========================================================

//
// Returns the number of sessions available.
// Gets info about the given session.
//

typedef struct
{
	CBYTE name[NET_MAX_NAME_LENGTH];
	SLONG max_players;
	SLONG num_players;

} NET_Sinfo;

SLONG     NET_session_get_number(void);
NET_Sinfo NET_session_get_info  (SLONG session);

//
// Joins the given session. Returns TRUE on success.
//

SLONG NET_session_join(SLONG session);

//
// Leaves the session.
//

void NET_session_leave(void);



// ========================================================
//
// PLAYER MESSAGE STUFF
//
// ========================================================

//
// For a player to send a message to the server.
//

void NET_player_message_send(SLONG num_bytes, void *data, SLONG guaranteed = FALSE);

//
// Receiving messages. If you get NET_MESSAGE_FROM_SERVER, then *num_bytes
// and *data will be filled in with the message received.
//

#define NET_PLAYER_MESSAGE_NONE            0
#define NET_PLAYER_MESSAGE_LOST_CONNECTION 1
#define NET_PLAYER_MESSAGE_FROM_SERVER     2

SLONG NET_player_message_receive(SLONG *num_bytes, void **data);



// ========================================================
//
// SERVER MESSAGE STUFF
//
// ========================================================

typedef SLONG NET_Player;

//
// Sends a message to the given player.
//

void NET_server_message_to_player(NET_Player player, SLONG num_bytes, void *data, SLONG guaranteed = FALSE);

//
// Receives a message.
//

#define NET_SERVER_MESSAGE_NONE            0
#define NET_SERVER_MESSAGE_LOST_CONNECTION 1
#define NET_SERVER_MESSAGE_PLAYER_LEFT     2	// *player contains the player who left
#define NET_SERVER_MESSAGE_PLAYER_JOINED   3	// *player contains the new player
#define NET_SERVER_MESSAGE_FROM_PLAYER     4	// A message from player *player.

SLONG NET_server_message_receive(NET_Player *player, SLONG *num_bytes, void **data);



#endif

