//
// Network stuff...
//

#ifndef NET_H
#define NET_H



// ========================================================
//
// Initialising and releasing all the DirectPlay stuff...
// Call once each at the start and end of the program.
// 
// ========================================================

void NET_init(void);
void NET_kill(void);


//
// The NULL player ID.
// Use this player ID to send a message to everyone.
// If the system sends a message, then it comes from NET_PLAYER_SYSTEM
//

#define NET_PLAYER_NONE		255
#define NET_PLAYER_ALL		254
#define NET_PLAYER_SYSTEM	253


// ========================================================
//
// CONNECTION STUFF
//
// ========================================================

//
// Returns the number of connections available.
// Returns the name of the given connection.
//

SLONG  NET_get_connection_number(void);
CBYTE *NET_get_connection_name  (SLONG connection);

//
// Establishes a connection. Returns TRUE on success.
//

SLONG NET_connection_make(SLONG connection);


// ========================================================
//
// SESSION STUFF
//
// ========================================================

//
// Creates a session. Makes this machine the host. Returns FALSE
// on failure.
//		 

SLONG NET_create_session(CBYTE *name, SLONG max_players, CBYTE *my_player_name);

//
// Returns the number of sessions available.
// Gets info about the given session.
// Joins the given session.
//

#define NET_NAME_LENGTH 64

typedef struct
{
	CBYTE name[NET_NAME_LENGTH];

} NET_Sinfo;

SLONG      NET_get_session_number(void);
NET_Sinfo  NET_get_session_info  (SLONG session);

//
// Joins the given session. Returns FALSE on failure.
//

SLONG NET_join_session(SLONG session, CBYTE *my_player_name);

//
// Leaves the session.
//

void NET_leave_session(void);

//
// Starts the game. It sends out a START_GAME system message
// and assigns player_ids to all the other players. Only the host
// should call this function.  Other players should wait for
// a START_GAME system message.
//
// This function returns your player_id. Do not wait for a START_GAME
// message because you won't get one. Returns NET_PLAYER_NONE on failure.
//

UBYTE NET_start_game(void);

// ========================================================
//
// PLAYER STUFF
//
// ========================================================

//
// To find the player in the current session. To update this list you
// have to process the message loop. The player argument to NET_get_player_name
// is not a player_id, just the n'th player in the session.
//

SLONG  NET_get_num_players(void);
CBYTE *NET_get_player_name(SLONG player);


// ========================================================
//
// MESSAGE STUFF
//
// ========================================================

//
// Maximum length of a message in bytes.
//

#define NET_MESSAGE_LENGTH (16 * 1024)

//
// Sends a message to the given player. Pass NET_PLAYER_ALL to send
// a broadcast message to everyone.
//

void NET_message_send(
		UBYTE  player_id,
		void  *data,
		UWORD  num_bytes);

//
// Returns TRUE if there is a message in the queue.
//

SLONG NET_message_waiting(void);

//
// Gets the next message in the queue.
//

#define NET_SYSMESS_NOP				0	// Do nothing!
#define NET_SYSMESS_START_GAME      1
#define NET_SYSMESS_LOST_CONNECTION	2
#define NET_SYSMESS_HOST_QUIT_OUT	3

typedef struct
{
	UBYTE player_id;
	UBYTE shit1;
	UWORD shit2;

	union
	{
		struct	// For system messages, when player == NET_PLAYER_SYSTEM
		{
			UBYTE sysmess;
			UBYTE player_id;	// For the START_GAME system message.
			UBYTE shite;
			
		} system;

		struct	// For all other messages.
		{
			UWORD num_bytes;
			UWORD more_shit;
			void *data;
			
		} player;
	};

} NET_Message;

void NET_message_get(NET_Message *answer);


#endif
