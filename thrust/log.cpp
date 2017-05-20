//
// For displaying debug messages.
//

#include "always.h"
#include "font.h"






// ========================================================
//
// THE LOG DISPLAY MESSAGE CIRCULAR QUEUE.
//
// ========================================================

#define LOG_DISPLAY_MESSAGE_LENGTH 128

typedef struct
{
	ULONG colour;
	CBYTE message[LOG_DISPLAY_MESSAGE_LENGTH];

} LOG_Display;

#define LOG_MAX_DISPLAY 32	// Power of 2 please!

LOG_Display LOG_display[LOG_MAX_DISPLAY];
SLONG       LOG_display_last;	// The index of the last message.


//
// The file we output to.
//

FILE *LOG_handle;




//
// Clears out all messages.
//

void LOG_init()
{
	LOG_display_last = 0;

	memset(LOG_display, 0, sizeof(LOG_display));

	LOG_handle = fopen("c:\\debuglog.txt", "wb");
}


//
// Adds a message to the display system.
//

void LOG_message(ULONG colour, CBYTE *fmt, ...)
{
	//
	// Work out the real message.
	//

	CBYTE   message[512];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	//
	// Add to the circular queue.
	//

	LOG_display_last += 1;
	LOG_display_last &= LOG_MAX_DISPLAY - 1;

	strncpy(LOG_display[LOG_display_last].message, message, LOG_DISPLAY_MESSAGE_LENGTH - 1);

	LOG_display[LOG_display_last].colour = colour;
}


void LOG_draw()
{
	SLONG i;
	SLONG mess;

	float y = 0.95F;

	FONT_format(FONT_FLAG_JUSTIFY_LEFT);

	for (i = 0; i < LOG_MAX_DISPLAY; i++)
	{
		mess  = LOG_display_last - i;
		mess &= LOG_MAX_DISPLAY - 1;

		if (LOG_display[mess].message)
		{
			FONT_draw(0.03F, y, LOG_display[mess].colour, 0.5F, -1, LOG_display[mess].message);

			y -= 0.025F;
		}
	}
}

void LOG_file(CBYTE *fmt, ...)
{
	//
	// Work out the real message.
	//

	CBYTE   message[512];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	if (LOG_handle)
	{
		fprintf(LOG_handle, "%s", message);
	}
}

void LOG_fini()
{
	if (LOG_handle)
	{
		fclose(LOG_handle);
	}
}