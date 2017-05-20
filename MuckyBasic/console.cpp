//
// Provides a PRINT and INPUT for our graphical display.
// Uses the FONT module and takes control of the display.
//

#include "always.h"
#include "console.h"
#include "font.h"
#include "key.h"
#include "os.h"



//
// Each line of text.
//

#define CONSOLE_MAX_TEXT_PER_LINE 512

typedef struct
{
	CBYTE text[CONSOLE_MAX_TEXT_PER_LINE];
	SLONG cursor;

} CONSOLE_Line;

#define CONSOLE_MAX_LINES 35

CONSOLE_Line CONSOLE_line[CONSOLE_MAX_LINES];
SLONG        CONSOLE_line_upto;	// Just keeps going up beyond CONSOLE_MAX_LINES...



//
// Line height... 
//

#define CONSOLE_LINE_HEIGHT (1.0F / (CONSOLE_MAX_LINES + 1))



void CONSOLE_init()
{
	memset(CONSOLE_line, 0, sizeof(CONSOLE_line));

	CONSOLE_line_upto = 0;
}



//
// Draws the current console.
//

void CONSOLE_draw()
{
	SLONG i;
	SLONG first;
	SLONG last;
	SLONG num;
	float ypos;

	OS_clear_screen();

	if (CONSOLE_line_upto <= CONSOLE_MAX_LINES)
	{
		first = 0;
		last  = CONSOLE_line_upto - 1;
	}
	else
	{
		first = CONSOLE_line_upto - CONSOLE_MAX_LINES;
		last  = CONSOLE_line_upto - 1;
	}

	num  = last - first + 1;
	ypos = 0.009F + (num - 1) * CONSOLE_LINE_HEIGHT; 

	for (i = last; i >= first; i--)
	{
		FONT_draw(
			0.013F,
			ypos,
			0xffffff,
			FONT_FLAG_JUSTIFY_LEFT,
			1.0F,
		    CONSOLE_line[i % CONSOLE_MAX_LINES].cursor,
			CONSOLE_line[i % CONSOLE_MAX_LINES].text);

		ypos -= CONSOLE_LINE_HEIGHT;
	}

	OS_show();
}




void CONSOLE_print(CBYTE *fmt, ...)
{
	//
	// Work out the real message.
	//

	CBYTE   message[2048];		// Real long... just in case!
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	//
	// Create a new line on the console.
	//

	CONSOLE_Line *cl;
	
	cl = &CONSOLE_line[CONSOLE_line_upto++ % CONSOLE_MAX_LINES];
	
	memcpy(cl->text, message, CONSOLE_MAX_TEXT_PER_LINE - 1);

	cl->cursor = -1;

	//
	// Draw the console.
	//

	CONSOLE_draw();
}


CBYTE *CONSOLE_input()
{
	CBYTE        *ch;
	CONSOLE_Line *cl;

	SLONG flash  = OS_ticks();
	SLONG draw   = TRUE;
	SLONG cursor = 0;

	//
	// Create a new line on the console that we will use as our input.
	//

	cl = &CONSOLE_line[CONSOLE_line_upto++ % CONSOLE_MAX_LINES];
	
	memset(cl->text, 0, sizeof(cl->text));

	cl->cursor = -1;

	//
	// Process keyboard input...
	//

	KEY_inkey = 0;

	while(1)
	{
		if (KEY_on[KEY_BACKSPACE])
		{
			KEY_on[KEY_BACKSPACE] = 0;

			if (cursor > 0)
			{
				cursor -= 1;

				ch = &cl->text[cursor];

				while(1)
				{
					ch[0] = ch[1];

					if (ch[0] == '\000')
					{
						break;
					}

					ch++;
				}
			}

			flash = OS_ticks();
			draw  = TRUE;
		}

		if (KEY_on[KEY_DELETE])
		{
			KEY_on[KEY_DELETE] = 0;

		}

		if (KEY_on[KEY_RETURN])
		{
			KEY_on[KEY_RETURN] = 0;

			cl->cursor = -1;

			return cl->text;
		}

		if (KEY_on[KEY_LEFT])
		{
			KEY_on[KEY_LEFT] = 0;

			if (cursor > 0)
			{
				cursor -= 1;
			}

			flash = OS_ticks();
			draw  = TRUE;
		}

		if (KEY_on[KEY_RIGHT])
		{
			KEY_on[KEY_RIGHT] = 0;

			if (cl->text[cursor])
			{
				cursor += 1;
			}

			flash = OS_ticks();
			draw  = TRUE;
		}

		if (KEY_on[KEY_END])
		{
			KEY_on[KEY_END] = 0;

			while(cl->text[cursor])
			{
				cursor++;
			}

			flash = OS_ticks();
			draw  = TRUE;
		}

		if (KEY_on[KEY_HOME])
		{
			KEY_on[KEY_HOME] = 0;

			cursor = 0;

			flash = OS_ticks();
			draw  = TRUE;
		}

		if (KEY_inkey)
		{
			if (isprint(KEY_inkey))
			{
				if (WITHIN(cursor, 0, CONSOLE_MAX_TEXT_PER_LINE - 2))
				{
					//
					// Insert a character at the cursor position.
					//

					for (ch = &cl->text[cursor]; *ch; ch++);

					while(ch > &cl->text[cursor])
					{
						ch[0] = ch[-1];

						ch--;
					}

					cl->text[cursor] = KEY_inkey;

					cl->text[CONSOLE_MAX_TEXT_PER_LINE - 1] = '\000';

					cursor += 1;
				}
			}

			flash = OS_ticks();
			draw  = TRUE;

			KEY_inkey = 0;
		}

		if (OS_ticks() > flash + 500)
		{
			flash = OS_ticks();
			draw ^= TRUE;
		}

		if (draw)
		{
			cl->cursor = cursor;
		}
		else
		{
			cl->cursor = -1;
		}

		CONSOLE_draw();
	}
}
