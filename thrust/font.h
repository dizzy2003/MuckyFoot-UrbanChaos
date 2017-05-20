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
// Sets the format of the text.
//

#define FONT_FLAG_WRAP           (1 << 0)
#define FONT_FLAG_JUSTIFY_LEFT   (1 << 0)
#define FONT_FLAG_JUSTIFY_CENTRE (1 << 1)
#define FONT_FLAG_JUSTIFY_RIGHT  (1 << 2)
#define FONT_FLAG_DROP_SHADOW    (1 << 3)

void FONT_format(
		ULONG flag    = FONT_FLAG_JUSTIFY_LEFT,
		float bbox_x1 = 0.0F,
		float bbox_y1 = 0.0F,
		float bbox_x2 = 1.0F,
		float bbox_y2 = 1.0F);


//
// Draws some text.  A scale of 1.0F is normal!  If cursor is >= 0, then a cursor
// draw after the 'cursor'th character.
//

void FONT_draw(float start_x, float start_y, ULONG colour, float scale, SLONG cursor, CBYTE *fmt, ...);


#endif
