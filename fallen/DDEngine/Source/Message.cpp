//
// Message passing to the user.
//


#include <MFStdLib.h>
#include "font.h"
#include "message.h"


#ifdef TARGET_DC
#define MSG_MAX_LENGTH   128
#define MSG_MAX_MESSAGES 100
#else
#define MSG_MAX_LENGTH   256
#define MSG_MAX_MESSAGES 1000
#endif

extern BOOL allow_debug_keys;

//
// The actual messages.
//

typedef struct
{
	SLONG timer;
	CBYTE message[MSG_MAX_LENGTH];

} MSG_Message;

MSG_Message MSG_message[MSG_MAX_MESSAGES];
SLONG	current_message=0;
SLONG	message_count=0;
SLONG	draw_message_offset=0;

void MSG_clear()
{
	SLONG i;

	for (i = 0; i < MSG_MAX_MESSAGES; i++)
	{
		MSG_message[i].timer = 0;
	}
}

//
// How long the messages hang around for.
//

#define MSG_TIMER 128

void MSG_add(CBYTE *fmt, ...)
{
	//SLONG i;
	SLONG oldest   = 0;
	SLONG oldtimer = INFINITY;

	if (!allow_debug_keys) return;

	//
	// Work out the real message.
	//

	CBYTE   message[512];
	va_list	ap;
//	return;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);
	if(ControlFlag)
		LogText("MSG->%s\n",message);
/*
	for (i = 0; i < MSG_MAX_MESSAGES; i++)
	{
		//
		// If we find the same message already present, just make that message
		// hang around longer.
		//

		if (MSG_message[i].timer)
		{
			if (strncmp(MSG_message[i].message, message, MSG_MAX_LENGTH - 1) == 0)
			{
				MSG_message[i].timer = MSG_TIMER;
				return;
			}
		}

		if (MSG_message[i].timer < oldtimer)
		{
			oldest   = i;
			oldtimer = MSG_message[i].timer;
		}
	}
*/


	//
	// Overwrite the oldest message.
	//

	strncpy(MSG_message[current_message].message, message, MSG_MAX_LENGTH - 1);

	MSG_message[current_message].timer = MSG_TIMER;

	current_message++;
	if(current_message>MSG_MAX_MESSAGES-2)
		current_message=0;

	return;
}


#define	SCREEN_SIZE	45

void MSG_draw()
{
	SLONG i;
	SLONG x;
	SLONG y;

	UBYTE red;
	UBYTE green;
	UBYTE blue;

	SLONG	pos;
	static	draw_flag=0;
	SLONG	size=1;
	//
	// Go through the messages and draw them.
	//

	if (!allow_debug_keys) return;

	pos=current_message-SCREEN_SIZE+draw_message_offset;
	if(pos<0)
		pos+=MSG_MAX_MESSAGES;

	if(pos>MSG_MAX_MESSAGES)
		pos-=MSG_MAX_MESSAGES;

	if(ShiftFlag)
		size=20;

	if(Keys[KB_PPLUS])
		draw_message_offset+=size;

	if(Keys[KB_PMINUS])
		draw_message_offset-=size;

	if(Keys[KB_PENTER])
		draw_message_offset=0;

	/*

	if(Keys[KB_PPOINT])
	{
		Keys[KB_PPOINT]=0;
		if(draw_flag)
			draw_flag=0;
		else
			draw_flag=1;
	}

	*/

//	if(draw_flag)
	for (i = 0; i < SCREEN_SIZE; i++)
	{

		x = 10;
		y = 10 + i * 10;

		if (MSG_message[pos].timer)
		{
			//red   = 80 + (MSG_message[i].timer >> 2);
			//green = 80 + (MSG_message[i].timer >> 2);
			//blue  = 80 + (MSG_message[i].timer >> 2);

//			if (MSG_message[i].timer == MSG_TIMER)
			{
				red   = 223;
				green = 223;
				blue  = 223;
			}

			FONT_draw_coloured_text(
				x + 1,
				y + 1,
				red,
				0,
				blue,
				MSG_message[pos].message);
			
			FONT_draw_coloured_text(
				x + 0,
				y + 0,
				red   + 32,
				green + 32,
				blue  + 32,
				MSG_message[pos].message);

//			MSG_message[pos].timer -= 1;
		}
		pos++;
		if(pos>MSG_MAX_MESSAGES)
			pos=0;
	}
}
