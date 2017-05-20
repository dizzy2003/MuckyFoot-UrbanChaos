#ifndef	  INTERFACE_H
#define   INTERFACE_H

//
// Defines
//

#define	INPUT_FORWARDS		0
#define	INPUT_BACKWARDS		1
#define	INPUT_LEFT			2
#define	INPUT_RIGHT			3
#define	INPUT_START			4
#define	INPUT_CANCEL		5
#define	INPUT_PUNCH			6
#define	INPUT_KICK			7
#define	INPUT_ACTION		8
#define	INPUT_JUMP			9
#define	INPUT_CAMERA		10
#define INPUT_CAM_LEFT		11
#define INPUT_CAM_RIGHT		12
#define INPUT_CAM_BEHIND	13
#define INPUT_MOVE			14
#define INPUT_SELECT		15
#define	INPUT_STEP_LEFT		16
#define	INPUT_STEP_RIGHT	17

#define	INPUT_MASK_FORWARDS		(1 << INPUT_FORWARDS)
#define	INPUT_MASK_BACKWARDS	(1 << INPUT_BACKWARDS)
#define	INPUT_MASK_LEFT			(1 << INPUT_LEFT)
#define INPUT_MASK_START		(1 << INPUT_START)
// Used for menus
#define	INPUT_MASK_CANCEL		(1 << INPUT_CANCEL)
#define	INPUT_MASK_RIGHT		(1 << INPUT_RIGHT)
#define	INPUT_MASK_PUNCH		(1 << INPUT_PUNCH)
// Use for menus
#define	INPUT_MASK_DOMENU		(INPUT_MASK_PUNCH)
#define	INPUT_MASK_KICK			(1 << INPUT_KICK)
#define	INPUT_MASK_ACTION		(1 << INPUT_ACTION)
#define	INPUT_MASK_JUMP			(1 << INPUT_JUMP)
//#define	INPUT_MASK_MODE_CHANGE	(1 << INPUT_MODE_CHANGE)
#define	INPUT_MASK_CAMERA		(1 << INPUT_CAMERA)
#define INPUT_MASK_CAM_LEFT		(1 << INPUT_CAM_LEFT)
#define INPUT_MASK_CAM_RIGHT	(1 << INPUT_CAM_RIGHT)
#define INPUT_MASK_CAM_BEHIND	(1 << INPUT_CAM_BEHIND)
#define INPUT_MASK_MOVE			(1 << INPUT_MOVE)
#define INPUT_MASK_SELECT		(1 << INPUT_SELECT)
#define	INPUT_MASK_STEP_LEFT	(1 << INPUT_STEP_LEFT)
#define	INPUT_MASK_STEP_RIGHT	(1 << INPUT_STEP_RIGHT)
// This is hardwired - the analog values go in the top 14 bits.
#define INPUT_MASK_ALL_BUTTONS	(0x3ffff)

#ifdef	PSX
#define INPUT_CAR_ACCELERATE	(INPUT_MASK_FORWARDS | INPUT_MASK_MOVE | INPUT_MASK_JUMP)
#define INPUT_CAR_SIREN			(INPUT_MASK_KICK)
#define INPUT_CAR_DECELERATE	(INPUT_MASK_BACKWARDS | INPUT_MASK_PUNCH)
#define INPUT_CAR_GOFASTER		(INPUT_CAR_ACCELERATE|INPUT_CAR_DECELERATE)

#else //#ifdef	PSX
/*
#define INPUT_CAR_ACCELERATE	(INPUT_MASK_FORWARDS | INPUT_MASK_MOVE | INPUT_MASK_PUNCH)
#define INPUT_CAR_DECELERATE	(INPUT_MASK_BACKWARDS | INPUT_MASK_KICK)
#define INPUT_CAR_GOFASTER		(INPUT_MASK_ACTION)
#define INPUT_CAR_SIREN			(INPUT_MASK_JUMP)
*/
#define INPUT_CAR_ACCELERATE	(INPUT_MASK_FORWARDS | INPUT_MASK_MOVE | INPUT_MASK_PUNCH)
#define INPUT_CAR_DECELERATE	(INPUT_MASK_BACKWARDS | INPUT_MASK_KICK)
#define INPUT_CAR_GOFASTER		(INPUT_CAR_ACCELERATE|INPUT_CAR_DECELERATE)
#define INPUT_CAR_SIREN			(INPUT_MASK_JUMP)
#endif

#define	INPUT_MASKM_CAM_TYPE	(INPUT_MASK_CAM_LEFT|INPUT_MASK_CAM_RIGHT)
#define	INPUT_MASKM_CAM_SHIFT	(INPUT_CAM_LEFT)
#define	INPUT_MASKM_CAM1		(0)
#define	INPUT_MASKM_CAM2		(INPUT_MASK_CAM_LEFT)
#define	INPUT_MASKM_CAM3		(INPUT_MASK_CAM_RIGHT)
#define	INPUT_MASKM_CAM4		(INPUT_MASK_CAM_LEFT|INPUT_MASK_CAM_RIGHT)

#define	INPUT_MASK_DIR			(INPUT_MASK_FORWARDS|INPUT_MASK_BACKWARDS|INPUT_MASK_LEFT|INPUT_MASK_RIGHT)

//
// Structs
//


//
// Data
//


//
// Functions
//

extern	void	apply_button_input(struct Thing *p_thing,SLONG input);
extern	void	process_hardware_level_input_for_player(Thing *p_thing);
extern	void	init_user_interface(void);
extern	SLONG	continue_action(Thing *p_person);
extern	SLONG	continue_moveing(Thing *p_person);
extern	SLONG	continue_firing(Thing *p_person);
extern	SLONG	person_get_in_car(Thing *p_person);	// Returns TRUE if it finds a car and set the person's InCar field
extern	SLONG	person_get_in_specific_car(Thing *p_person, Thing *p_vehicle);





// Numbers to feed as "type" to get_hardware_input().
#define	INPUT_TYPE_KEY	(1<<0)
#define	INPUT_TYPE_JOY	(1<<1)

#ifdef TARGET_DC
// For DC, remaps the D-pad to the UDLR pad. Makes handling menus, etc, easier.
// Note that the original D-pad inputs are removed, so you can assign things to it and
// they'll be ignored.
#define INPUT_TYPE_REMAP_DPAD (1<<2)
// Does the same for the buttons - remaps them to punch and kick for easy menus.
// A, Y, Rtrig -> punch.
// B, X, Ltrig -> cancel.
#define INPUT_TYPE_REMAP_BUTTONS (1<<3)
// Remaps the START button to MENUDO.
#define INPUT_TYPE_REMAP_START_BUTTON (1<<4)
// DONT remap the X and Y buttons. Used during pause.
#define INPUT_TYPE_REMAP_NOT_X_Y (1<<5)
#endif

// Only sets buttons that have gone down since the last poll. This is useful for
// menus, etc.
#define INPUT_TYPE_GONEDOWN (1<<6)


#define INPUT_TYPE_ALL (INPUT_TYPE_KEY|INPUT_TYPE_JOY)

extern ULONG	get_hardware_input(UWORD type);
// Type can only be INPUT_TYPE_GONEDOWN
extern ULONG	get_last_input (UWORD type);
// Allow the last input state to autorepeat, despite GONEDOWN.
extern void allow_input_autorepeat ( void );



#ifdef TARGET_DC


// A flag that says that ABXYStart was pressed.
// When you handle it, set it to FALSE.
extern bool g_bDreamcastABXYStartComboPressed;



// Standard DirectInput buttons on DC.
#define DI_DC_BUTTON_A			1
#define DI_DC_BUTTON_B			0
#define DI_DC_BUTTON_X			8
#define DI_DC_BUTTON_Y			7
#define DI_DC_BUTTON_START		2
// D-pad buttons.				
#define DI_DC_BUTTON_UP			3
#define DI_DC_BUTTON_DOWN		4
#define DI_DC_BUTTON_LEFT		5
#define DI_DC_BUTTON_RIGHT		6
// The two analogue triggers are "buttons" - values range from 0 to 255,
// so if you just want them as normal buttons, you can still just test the top bit.
#define DI_DC_BUTTON_RTRIGGER	9
#define DI_DC_BUTTON_LTRIGGER	10

// Some not-so-standard ones.
// The game only uses C,D and Z - the extra analogue and hat buttons are ignored. Sorry.
#define DI_DC_BUTTON_C			11
#define DI_DC_BUTTON_D			13
#define DI_DC_BUTTON_Z			12
// General analogue buttons.
#define DI_DC_BUTTON_AN3		14
#define DI_DC_BUTTON_AN4		15
#define DI_DC_BUTTON_AN5		16
#define DI_DC_BUTTON_AN6		17
// Another direction pad. I think.
#define DI_DC_BUTTON_2_UP		18
#define DI_DC_BUTTON_2_DOWN		19
#define DI_DC_BUTTON_2_LEFT		20
#define DI_DC_BUTTON_2_RIGHT	21

// Special one - any unmapped buttons are remapped here. Don't read it!
#define DI_DC_BUTTON_BOGUS		31



// How many pre-made modes there are (not including custom).
#define NUM_OF_JOYPAD_MODES 4
void INTERFAC_SetUpJoyPadButtons ( int iMode );



#endif //#ifdef TARGET_DC


extern UBYTE	joypad_button_use[16];
#ifndef TARGET_DC
extern UBYTE	keybrd_button_use[16];
#endif

#define	JOYPAD_BUTTON_KICK       0
#define	JOYPAD_BUTTON_PUNCH      1
#define	JOYPAD_BUTTON_JUMP       2
#define	JOYPAD_BUTTON_ACTION     3
#define	JOYPAD_BUTTON_MOVE       4
#define	JOYPAD_BUTTON_START      5
#define	JOYPAD_BUTTON_SELECT     6
#define	JOYPAD_BUTTON_CAMERA     7
#define	JOYPAD_BUTTON_CAM_LEFT   8
#define	JOYPAD_BUTTON_CAM_RIGHT  9
#define JOYPAD_BUTTON_1STPERSON 10

#define KEYBRD_BUTTON_LEFT		11
#define KEYBRD_BUTTON_RIGHT		12
#define KEYBRD_BUTTON_FORWARDS	13
#define KEYBRD_BUTTON_BACK		14



#endif //#ifndef	  INTERFACE_H



