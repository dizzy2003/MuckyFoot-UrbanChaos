/************************************************************
 *
 *   console.h
 *   a crude message console for queuing and writing
 *   messages to the user
 *
 */


#ifndef _CONSOLE_
#define _CONSOLE_

#include "MFStdLib.h"

void CONSOLE_font(CBYTE *fontpath, float scale=1.0);
void CONSOLE_draw();
void CONSOLE_text(CBYTE *text, SLONG delay = 4000);	// Delay in milliseconds
void CONSOLE_clear();
void CONSOLE_scroll();

//
// Messages that appear at specific points on the screen- If you place
// a message at the same place as an older message, the new message
// replaces that older one.
//

void CONSOLE_text_at(
		SLONG  x,
		SLONG  y,
		SLONG  delay,
		CBYTE *fmt, ...);

#endif
