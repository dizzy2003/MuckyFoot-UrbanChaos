/*
** File: Panel.h
**
** Author: James Watson
**
** Copyright: 1999 Mucky Foot Productions Ltd
*/

#ifndef _PANEL_H
#define _PANEL_H

#include "MFStdLib.h"
#include "psxeng.h"

void PANEL_draw_quad(SLONG left,SLONG top,SLONG width,SLONG height,SLONG page,ULONG colour,UBYTE u1,UBYTE v1,UBYTE u2,UBYTE v2);
void PANEL_funky_quad(SLONG which,SLONG x,SLONG y,ULONG colour);
void PANEL_new_funky();
void PANEL_inventory(Thing *darci) ;
void PANEL_new_text(Thing *who, SLONG delay, CBYTE *fmt, ...);
void PANEL_new_text_process();
void PANEL_new_help_message(CBYTE *fmt, ...);
void PANEL_new_text_draw();
void PANEL_draw_beacons(void);
void PANEL_new_widescreen();
void PANEL_render_thugs();
void PANEL_DrawSign(SLONG x,SLONG y,SLONG type);
void PANEL_flash_sign(SLONG sign, SLONG flip);
void PANEL_new_info_message(CBYTE *fmt, ...);
void PANEL_new_face(Thing *who,SLONG x,SLONG y);
void PANEL_draw_search(SLONG timer);
void PANEL_draw_eog(SLONG win);
void PANEL_new_text_init(void);
void PANEL_new_widescreen_init();
void PANEL_draw_local_health(SLONG mx,SLONG my,SLONG mz,SLONG percentage,SLONG radius = 60);


#define PANEL_SIGN_WHICH_UTURN                0
#define PANEL_SIGN_WHICH_TURN_RIGHT           1
#define PANEL_SIGN_WHICH_DONT_TAKE_RIGHT_TURN 2
#define PANEL_SIGN_WHICH_STOP                 3

#define PANEL_SIGN_FLIP_LEFT_AND_RIGHT (1 << 0)
#define PANEL_SIGN_FLIP_TOP_AND_BOTTOM (1 << 1)

#endif