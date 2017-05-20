//	Text.h
//	Guy Simmons, 17th May 1998.

#ifndef	TEXT_H
#define	TEXT_H

//---------------------------------------------------------------
extern BOOL text_fudge;
extern ULONG text_colour;


SLONG	text_width(CBYTE *message,SLONG font_id,SLONG *char_count);
SLONG	text_height(CBYTE *message,SLONG font_id,SLONG *char_count);
void	draw_text_at(float x,float y,CBYTE *message,SLONG font_id);
void	draw_centre_text_at(float x,float y,CBYTE *message,SLONG font_id,SLONG flag);
void	show_text(void);

//---------------------------------------------------------------

#endif

