//
// A font! That's all there is to it.
//

#ifndef _FONT_
#define _FONT_


//
// Loads in the font texture and calculates the uv's of the letters.
//

void FONT_init(void);



//
// Returns TRUE if the FONT module can draw the ASCII character.
//

SLONG FONT_char_is_valid(CBYTE ch);


//
// Draws some text.  A scale of 1.0F is normal!  If cursor is >= 0, then a cursor
// draw after the 'cursor'th character.
//

#define FONT_FLAG_JUSTIFY_LEFT   (1 << 0)
#define FONT_FLAG_JUSTIFY_CENTRE (1 << 1)
#define FONT_FLAG_JUSTIFY_RIGHT  (1 << 2)
#define FONT_FLAG_DROP_SHADOW    (1 << 3)

void FONT_draw(float start_x, float start_y, ULONG colour, ULONG flag, float scale, SLONG cursor, CBYTE *fmt, ...);


#endif
