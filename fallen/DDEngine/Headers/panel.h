#ifndef	AENG_PANEL_HPP

#define	AENG_PANEL_HPP


extern	UBYTE	PANEL_scanner_poo; // sets whether you want a poo scanner or not

extern	void	PANEL_start(void);
extern	void	PANEL_draw_gun_sight(SLONG x,SLONG y,SLONG z,SLONG radius,SLONG scale);
extern	void	PANEL_draw_timer(SLONG time_in_hundreths, SLONG x, SLONG y);
extern	void	PANEL_draw_buffered(void);	// Actually draws the timers....
extern	void	PANEL_draw_health_bar(SLONG x,SLONG y,SLONG percentage);
#if 0
// Never used!
extern	void	PANEL_draw_angelic_status(SLONG x, SLONG y, SLONG size, SLONG am_i_an_angel);
extern	void	PANEL_draw_press_button(SLONG x, SLONG y, SLONG size, SLONG frame);	// Even/odd frame
#endif
extern	void	PANEL_finish(void);
extern	void	PANEL_inventory(Thing *darci, Thing *player);


//
// The new funky panel!
//
#ifdef PSX
void PANEL_new_funky(void);
#endif

//
// Clears all new text messages.
//

void PANEL_new_text_init(void);

//
// The new funky messages-from-people system.
//

void PANEL_new_text(Thing *who, SLONG delay, CBYTE *fmt, ...);


//
// Help system messages.
//

void PANEL_new_help_message(CBYTE *fmt, ...);



//
// Messages that give you info. They go where the beacon text
// normally goes.
//

void PANEL_new_info_message(CBYTE *fmt, ...);


//
// Flashes up a sign on the screen for the next few second.
//

#define PANEL_SIGN_WHICH_UTURN                0
#define PANEL_SIGN_WHICH_TURN_RIGHT           1
#define PANEL_SIGN_WHICH_DONT_TAKE_RIGHT_TURN 2
#define PANEL_SIGN_WHICH_STOP                 3

#define PANEL_SIGN_FLIP_LEFT_AND_RIGHT (1 << 0)
#define PANEL_SIGN_FLIP_TOP_AND_BOTTOM (1 << 1)

void PANEL_flash_sign(SLONG which, SLONG flip);


//
// Darkens the screen where x goes from 0 to 640.
//

void PANEL_darken_screen(SLONG x);


//
// How to fade out the screen.
//

void  PANEL_fadeout_init    (void);
void  PANEL_fadeout_start   (void);	// Only starts if the fadeout is not already going...
void  PANEL_fadeout_draw    (void);
SLONG PANEL_fadeout_finished(void);




//
// The last panel for Urban Chaos (with a bit of luck).
//

void PANEL_last(void);



//
// Where  completion goes from 0 to 256
//

void PANEL_draw_completion_bar(SLONG completion);



// Returns a value to use for RHW that says "this is on top".
// Use 1.0f - this for a Z value.
// Each time this is called, it increases a little to help
// with cards that depth buffer oddly.
float PANEL_GetNextDepthBodge ( void );

// Call at the start of each frame.
void PANEL_ResetDepthBodge ( void );


// Screensaver stuff.
void PANEL_enable_screensaver ( void );
void PANEL_disable_screensaver ( bool bImmediately=FALSE );
void PANEL_screensaver_draw ( void );



void PANEL_draw_quad(
				float left,
				float top,
				float right,
				float bottom,
				SLONG page,
				ULONG colour = 0xffffffff,
				float u1 = 0.0F,
				float v1 = 0.0F,
				float u2 = 1.0F,
				float v2 = 1.0F);


#endif
