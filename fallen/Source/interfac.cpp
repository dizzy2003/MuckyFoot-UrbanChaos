#include	"game.h"
#include	"fallen/headers/interfac.h"
#include	"animate.h"
#include	"statedef.h"
#include	"person.h"
#include	"id.h"
#include	"enter.h"
#include	"mav.h"
#include	"cam.h"
#include	"hook.h"
#include	"dirt.h"
#ifdef		PSX
#include	"fallen/ddengine/headers/console.h"
#include	"fallen/psxeng/headers/psxeng.h"
#endif
#include	"combat.h"
#include	"eway.h"
#include	"interfac.h"
#include	"ware.h"
#include	"sound.h"
#include	"pcom.h"
#include	"ob.h"
#include	"widget.h"
#include	"snipe.h"
#include	"fc.h"
#include	"cnet.h"
#include	"memory.h"
#include	"america.h"
#include	"wmove.h"
#ifndef PSX
#include	"panel.h"
#include	"env.h"
#include	"ddlib.h"
#include	"poly.h"
#endif


#ifdef TARGET_DC
#include <platutil.h>
#include <ceddcdrm.h>
#include <segagdrm.h>
#endif

UBYTE player_relative;


extern void add_damage_text(SWORD x,SWORD y,SWORD z,CBYTE *text);
extern	void person_normal_move(Thing *p_person);
UBYTE	cheat=0;

#ifndef	PSX
// set this to 0 if it all fucks up (it's about 5 in the morning so quite possibly i made
// a complete pig's arse of it). it sets whether the front-end 'keyboard config' stuff actually
// has any effect or not
#define REMAP_KEYBOARD	1

#endif

#define	GET_JOYX(input)		(((input>>17)&0xfe)-128)
#define	GET_JOYY(input)		(((input>>24)&0xfe)-128)




#ifdef TARGET_DC
// Allow DC cheats.
#define DREAMCAST_CHEATS_PLEASE_BOB defined

#endif

bool g_bPunishMePleaseICheatedOnThisLevel = FALSE;
//bool m_bDontMoveIfBothTriggersDown = FALSE;


#ifdef DEBUG
float m_fFogTableDebugStart = 0.0f;
float m_fFogTableDebugEnd = 0.0f;
float m_fFogTableDebugDensity = 0.0f;
DWORD m_dwFogTableDebugFogTableMode = D3DFOG_NONE;
#endif


#ifndef PSX
#define	ANALOGUE_MIN_VELOCITY	8
#else
#define	ANALOGUE_MIN_VELOCITY	32
#endif

#define	INPUT_KEYS		0
#define	INPUT_JOYPAD	1

SLONG	input_mode=0;
SLONG	mouse_input=0;
SLONG	analogue=1;


bool g_bEngineVibrations = TRUE;


// on PC controls_inventory_mode has no effect on your ability to move, its purely a graphical effect on top of screen while you cycle through weapons
#ifdef	PSX
extern SWORD CONTROLS_inventory_mode;
#else
#define	CONTROLS_inventory_mode	0
#endif

extern	void	set_person_hug_wall_leap_out(Thing *p_person,SLONG dir);
SLONG	player_turn_left_right_analogue(Thing *p_thing,SLONG input);
extern	SLONG is_person_crouching(Thing *p_person);
extern	SLONG person_has_gun_out(Thing *p_person);
extern	void	set_person_hug_wall_dir(Thing *p_person,SLONG dir);
extern	void	set_person_arrest(Thing *p_person,SLONG index);
extern	void	set_person_draw_special(Thing *p_person);
extern	SLONG set_person_kick_dir(Thing *p_person,SLONG dir);
extern	void	set_person_fight_idle(Thing *p_person);
extern	void	set_person_fight_anim(Thing *p_person,SLONG anim);
extern	void	reset_gang_attack(Thing *p_target);
extern	SLONG turn_to_direction_and_find_target(Thing *p_person,SLONG  find_dir);
extern	SLONG	set_person_cut_fence(Thing *p_person);
extern	SLONG find_searchable_person(Thing *p_person);
extern	SLONG set_person_search(Thing *p_person, SLONG ob_index,SLONG ox,SLONG oy,SLONG oz);
extern	SLONG set_person_search_corpse(Thing *p_person, Thing *p_personb);
extern	void	set_person_carry(Thing *p_person,SLONG s_index);
extern	UWORD	find_corpse(Thing *p_person);

extern  EWAY_Way *EWAY_magic_radius_flag;
extern  void EWAY_set_active(EWAY_Way *ew);
extern  SLONG EWAY_evaluate_condition(EWAY_Way *ew, EWAY_Cond *ec, SLONG EWAY_sub_condition_of_a_boolean = FALSE);
extern	SLONG	is_person_dead(Thing *p_person);
extern	SLONG	is_person_ko(Thing *p_person);
extern	void	person_pick_best_target(Thing *p_person,SLONG dir);
extern	void	set_person_walk_backwards(Thing *p_person);
/*

  The new controller philosophy

  Automate as much as possible

  forward is walk

  button is run

*/
#include	"fc.h"
void FC_force_camera_behind(SLONG cam);


#define	INPUT_KEYS		0
#define	INPUT_JOYPAD	1

/*

  The new controller philosophy

  Automate as much as possible

  forward is walk

  button is run

*/

#ifdef	PSX
#include	"LIBETC.h"
#include	"ctrller.h"
#include	"libpad.h"
extern ControllerPacket PAD_Input1,PAD_Input2;
int psx_pad_mask=0;
#endif

extern	Thing	*net_players[20];

void	player_apply_move(Thing *p_thing,ULONG input);
void	player_apply_move_analgue(Thing *p_thing,ULONG input);

#ifndef PSX
UBYTE	joypad_button_use[16];
#ifndef TARGET_DC
UBYTE	keybrd_button_use[16];
#endif



bool m_bForceWalk = FALSE;

int g_iCheatNumber = -1;


#ifdef TARGET_DC

bool g_bDreamcastABXYStartComboPressed = FALSE;
bool m_bDreamcastABXYStartComboPressedLastTime = FALSE;
bool g_bDreamcastABXYStartShouldGoToBootRomImmediately = TRUE;

bool g_bShowDarcisPositionOnScreen = FALSE;
char cCheatString[30] = "";
#ifdef DEBUG
// Can't be arsed typing in the cheat code all the time.
bool g_bCheatsEnabled = TRUE;
#else
bool g_bCheatsEnabled = FALSE;
#endif

void INTERFAC_SetUpJoyPadButtons ( int iMode )
{
	// Pre-set modes.
	ASSERT ( iMode <= NUM_OF_JOYPAD_MODES );

	switch ( iMode )
	{
	case 0:
		// Custom mode.
		joypad_button_use[JOYPAD_BUTTON_KICK]		= ENV_get_value_number("joypad_kick",		DI_DC_BUTTON_LTRIGGER, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_PUNCH]		= ENV_get_value_number("joypad_punch",		DI_DC_BUTTON_RTRIGGER, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_JUMP]		= ENV_get_value_number("joypad_jump",		DI_DC_BUTTON_Y, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_ACTION]		= ENV_get_value_number("joypad_action",		DI_DC_BUTTON_A, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_MOVE]		= ENV_get_value_number("joypad_move",		DI_DC_BUTTON_UP, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_START]		= ENV_get_value_number("joypad_start",		DI_DC_BUTTON_START, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_SELECT]		= ENV_get_value_number("joypad_select",		DI_DC_BUTTON_B, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_CAMERA]		= ENV_get_value_number("joypad_camera",		DI_DC_BUTTON_DOWN, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_CAM_LEFT]	= ENV_get_value_number("joypad_cam_left",	DI_DC_BUTTON_LEFT, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT]	= ENV_get_value_number("joypad_cam_right",	DI_DC_BUTTON_RIGHT, "Joypad");
		joypad_button_use[JOYPAD_BUTTON_1STPERSON]	= ENV_get_value_number("joypad_1stperson",	DI_DC_BUTTON_X, "Joypad");
		break;
	case 1:
		// Tom's defaults.
		joypad_button_use[JOYPAD_BUTTON_KICK]		= DI_DC_BUTTON_LTRIGGER;
		joypad_button_use[JOYPAD_BUTTON_PUNCH]		= DI_DC_BUTTON_RTRIGGER;
		joypad_button_use[JOYPAD_BUTTON_JUMP]		= DI_DC_BUTTON_Y;
		joypad_button_use[JOYPAD_BUTTON_ACTION]		= DI_DC_BUTTON_A;
		joypad_button_use[JOYPAD_BUTTON_MOVE]		= DI_DC_BUTTON_UP;
		joypad_button_use[JOYPAD_BUTTON_START]		= DI_DC_BUTTON_START;
		joypad_button_use[JOYPAD_BUTTON_SELECT]		= DI_DC_BUTTON_B;
		joypad_button_use[JOYPAD_BUTTON_CAMERA]		= DI_DC_BUTTON_DOWN;
		joypad_button_use[JOYPAD_BUTTON_CAM_LEFT]	= DI_DC_BUTTON_LEFT;
		joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT]	= DI_DC_BUTTON_RIGHT;
		joypad_button_use[JOYPAD_BUTTON_1STPERSON]	= DI_DC_BUTTON_X;
		break;
	case 2:
		// Swap jump & action to match PSX version.
		joypad_button_use[JOYPAD_BUTTON_KICK]		= DI_DC_BUTTON_LTRIGGER;
		joypad_button_use[JOYPAD_BUTTON_PUNCH]		= DI_DC_BUTTON_RTRIGGER;
		joypad_button_use[JOYPAD_BUTTON_JUMP]		= DI_DC_BUTTON_A;
		joypad_button_use[JOYPAD_BUTTON_ACTION]		= DI_DC_BUTTON_Y;
		joypad_button_use[JOYPAD_BUTTON_MOVE]		= DI_DC_BUTTON_UP;
		joypad_button_use[JOYPAD_BUTTON_START]		= DI_DC_BUTTON_START;
		joypad_button_use[JOYPAD_BUTTON_SELECT]		= DI_DC_BUTTON_B;
		joypad_button_use[JOYPAD_BUTTON_CAMERA]		= DI_DC_BUTTON_DOWN;
		joypad_button_use[JOYPAD_BUTTON_CAM_LEFT]	= DI_DC_BUTTON_LEFT;
		joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT]	= DI_DC_BUTTON_RIGHT;
		joypad_button_use[JOYPAD_BUTTON_1STPERSON]	= DI_DC_BUTTON_X;
		break;
	case 3:
		// Kick & punch on buttons, jump & action on triggers.
		joypad_button_use[JOYPAD_BUTTON_KICK]		= DI_DC_BUTTON_B;
		joypad_button_use[JOYPAD_BUTTON_PUNCH]		= DI_DC_BUTTON_A;
		joypad_button_use[JOYPAD_BUTTON_JUMP]		= DI_DC_BUTTON_RTRIGGER;
		joypad_button_use[JOYPAD_BUTTON_ACTION]		= DI_DC_BUTTON_LTRIGGER;
		joypad_button_use[JOYPAD_BUTTON_MOVE]		= DI_DC_BUTTON_UP;
		joypad_button_use[JOYPAD_BUTTON_START]		= DI_DC_BUTTON_START;
		joypad_button_use[JOYPAD_BUTTON_SELECT]		= DI_DC_BUTTON_Y;
		joypad_button_use[JOYPAD_BUTTON_CAMERA]		= DI_DC_BUTTON_DOWN;
		joypad_button_use[JOYPAD_BUTTON_CAM_LEFT]	= DI_DC_BUTTON_LEFT;
		joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT]	= DI_DC_BUTTON_RIGHT;
		joypad_button_use[JOYPAD_BUTTON_1STPERSON]	= DI_DC_BUTTON_X;
		break;
	case 4:
		// For 6-button pads - exciting things on ABXY, boring things on ZC.
		joypad_button_use[JOYPAD_BUTTON_KICK]		= DI_DC_BUTTON_B;
		joypad_button_use[JOYPAD_BUTTON_PUNCH]		= DI_DC_BUTTON_A;
		joypad_button_use[JOYPAD_BUTTON_JUMP]		= DI_DC_BUTTON_Y;
		joypad_button_use[JOYPAD_BUTTON_ACTION]		= DI_DC_BUTTON_X;
		joypad_button_use[JOYPAD_BUTTON_MOVE]		= DI_DC_BUTTON_UP;
		joypad_button_use[JOYPAD_BUTTON_START]		= DI_DC_BUTTON_START;
		joypad_button_use[JOYPAD_BUTTON_SELECT]		= DI_DC_BUTTON_LTRIGGER;
		joypad_button_use[JOYPAD_BUTTON_CAMERA]		= DI_DC_BUTTON_DOWN;
		joypad_button_use[JOYPAD_BUTTON_CAM_LEFT]	= DI_DC_BUTTON_LEFT;
		joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT]	= DI_DC_BUTTON_RIGHT;
		joypad_button_use[JOYPAD_BUTTON_1STPERSON]	= DI_DC_BUTTON_RTRIGGER;
		break;
	default:
		ASSERT ( FALSE );
		// Not good. Set up something sensible anyway.
		joypad_button_use[JOYPAD_BUTTON_KICK]		= DI_DC_BUTTON_LTRIGGER;
		joypad_button_use[JOYPAD_BUTTON_PUNCH]		= DI_DC_BUTTON_RTRIGGER;
		joypad_button_use[JOYPAD_BUTTON_JUMP]		= DI_DC_BUTTON_Y;
		joypad_button_use[JOYPAD_BUTTON_ACTION]		= DI_DC_BUTTON_A;
		joypad_button_use[JOYPAD_BUTTON_MOVE]		= DI_DC_BUTTON_UP;
		joypad_button_use[JOYPAD_BUTTON_START]		= DI_DC_BUTTON_START;
		joypad_button_use[JOYPAD_BUTTON_SELECT]		= DI_DC_BUTTON_B;
		joypad_button_use[JOYPAD_BUTTON_CAMERA]		= DI_DC_BUTTON_DOWN;
		joypad_button_use[JOYPAD_BUTTON_CAM_LEFT]	= DI_DC_BUTTON_LEFT;
		joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT]	= DI_DC_BUTTON_RIGHT;
		joypad_button_use[JOYPAD_BUTTON_1STPERSON]	= DI_DC_BUTTON_X;
		break;
	}
}


#endif

void	init_joypad_config(void)
{
	SLONG  val;
/*
	//was
	joypad_button_use[JOYPAD_BUTTON_KICK]		= ENV_get_value_number("joypad_kick",		2, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_PUNCH]		= ENV_get_value_number("joypad_punch",		3, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_JUMP]		= ENV_get_value_number("joypad_jump",		0, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_ACTION]		= ENV_get_value_number("joypad_action",		1, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_MOVE]		= ENV_get_value_number("joypad_move",		5, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_START]		= ENV_get_value_number("joypad_start",		9, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_SELECT]		= ENV_get_value_number("joypad_select",		10, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_CAMERA]		= ENV_get_value_number("joypad_camera",		6, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_CAM_LEFT]	= ENV_get_value_number("joypad_cam_left",	7, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT]	= ENV_get_value_number("joypad_cam_right",	8, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_1STPERSON]	= ENV_get_value_number("joypad_1stperson",	4, "Joypad");
*/

#ifndef TARGET_DC
	joypad_button_use[JOYPAD_BUTTON_KICK]		= ENV_get_value_number("joypad_kick",		4, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_PUNCH]		= ENV_get_value_number("joypad_punch",		3, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_JUMP]		= ENV_get_value_number("joypad_jump",		0, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_ACTION]		= ENV_get_value_number("joypad_action",		1, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_MOVE]		= ENV_get_value_number("joypad_move",		7, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_START]		= ENV_get_value_number("joypad_start",		8, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_SELECT]		= ENV_get_value_number("joypad_select",		2, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_CAMERA]		= ENV_get_value_number("joypad_camera",		6, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_CAM_LEFT]	= ENV_get_value_number("joypad_cam_left",	9, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT]	= ENV_get_value_number("joypad_cam_right",	10, "Joypad");
	joypad_button_use[JOYPAD_BUTTON_1STPERSON]	= ENV_get_value_number("joypad_1stperson",	5, "Joypad");
#else //#ifndef TARGET_DC

	// Actually set up the buttons in a cunning way.
	INTERFAC_SetUpJoyPadButtons ( ENV_get_value_number ("joypad_mode", 0, "Joypad") );

#endif //#else //#ifndef TARGET_DC

#ifndef TARGET_DC
	keybrd_button_use[KEYBRD_BUTTON_LEFT]		= ENV_get_value_number("keyboard_left",		203, "Keyboard");
	keybrd_button_use[KEYBRD_BUTTON_RIGHT]		= ENV_get_value_number("keyboard_right",	205, "Keyboard");
	keybrd_button_use[KEYBRD_BUTTON_FORWARDS]	= ENV_get_value_number("keyboard_forward",	200, "Keyboard");
	keybrd_button_use[KEYBRD_BUTTON_BACK]		= ENV_get_value_number("keyboard_back",		208, "Keyboard");

	keybrd_button_use[JOYPAD_BUTTON_PUNCH]		= ENV_get_value_number("keyboard_punch",	 44, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_KICK]		= ENV_get_value_number("keyboard_kick",		 45, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_ACTION]		= ENV_get_value_number("keyboard_action",	 46, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_MOVE]		= ENV_get_value_number("keyboard_run",		 47, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_JUMP]		= ENV_get_value_number("keyboard_jump",		 57, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_START]		= ENV_get_value_number("keyboard_start",	 15, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_SELECT]		= ENV_get_value_number("keyboard_select",	 28, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_CAMERA]		= ENV_get_value_number("keyboard_camera",	207, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_CAM_LEFT]	= ENV_get_value_number("keyboard_cam_left",	211, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_CAM_RIGHT]	= ENV_get_value_number("keyboard_cam_right",209, "Keyboard");
	keybrd_button_use[JOYPAD_BUTTON_1STPERSON]	= ENV_get_value_number("keyboard_1stperson", 30, "Keyboard");
#endif

}




#endif

//#define	INPUT_	(1<<)

/*
#define	INPUT_FORWARDS		(0)
#define	INPUT_BACKWARDS		(1)
#define	INPUT_LEFT			(2)
#define	INPUT_RIGHT			(3)
#define	INPUT_PUNCH			(4)
#define	INPUT_KICK			(5)
#define	INPUT_ACTION		(6)
#define	INPUT_JUMP			(7)
#define	INPUT_MODE_CHANGE	(8)
#define	INPUT_BLOCK			(9)
*/

#define	INPUT_MOVEMENT_MASK		((0x3f)|INPUT_MASK_MOVE)
#define	INPUT_ACTION_MASK	(~0x3f)


struct	ActionInfo
{
	UBYTE	Action;
	UBYTE	Logic;
	ULONG	Input;

};


struct	ActionInfo	action_idle[]=
{
	{ACTION_FLIP_LEFT,1,INPUT_MASK_JUMP|INPUT_MASK_LEFT},
	{ACTION_FLIP_RIGHT,1,INPUT_MASK_JUMP|INPUT_MASK_RIGHT},
	{ACTION_STANDING_JUMP,0,INPUT_MASK_JUMP},
	{ACTION_FIGHT_PUNCH,0,INPUT_MASK_PUNCH},
	{ACTION_FIGHT_KICK,0,INPUT_MASK_KICK},
//	{ACTION_DRAW_SPECIAL,0,INPUT_MASK_SELECT},

//	{ACTION_STEP_LEFT,INPUT_MASK_STEP_LEFT},
//	{ACTION_STEP_RIGHT,INPUT_MASK_STEP_RIGHT},
	{0}
};

struct	ActionInfo	action_walk[]=
{
	{ACTION_RUN,0,0},
	{ACTION_FIGHT_PUNCH,0,INPUT_MASK_PUNCH},
	{ACTION_FIGHT_KICK,0,INPUT_MASK_KICK},
	{ACTION_STANDING_JUMP,0,INPUT_MASK_JUMP},
	{0}
};
struct	ActionInfo	action_walk_back[]=
{
	{ACTION_FIGHT_PUNCH,0,INPUT_MASK_PUNCH},
	{ACTION_FIGHT_KICK,0,INPUT_MASK_KICK},
	{ACTION_STANDING_JUMP,0,INPUT_MASK_JUMP},
	{0}
};

struct ActionInfo	action_run[]=
{
	{ACTION_WALK,0,0},
	{ACTION_RUNNING_JUMP,0,INPUT_MASK_JUMP},
	{ACTION_SKID,0,INPUT_MASK_KICK},
	{ACTION_SHOOT,0,INPUT_MASK_PUNCH},
	{0}
};

struct ActionInfo	action_standing_jump[]=
{
	{ACTION_FLIP_LEFT,1,INPUT_MASK_LEFT},
	{ACTION_FLIP_RIGHT,1,INPUT_MASK_RIGHT},
	{ACTION_FIGHT_KICK,0,INPUT_MASK_KICK},
	{0,0,0}
};

struct ActionInfo	action_standing_jump_grab[]=
{
	{0,0,0}
};

struct ActionInfo	action_running_jump[]=
{
	{0,0,0}
};

struct ActionInfo	action_dangling[]=
{
	{ACTION_PULL_UP,0,INPUT_MASK_FORWARDS},
	{ACTION_PULL_UP,0,INPUT_MASK_MOVE},
	{ACTION_DROP_DOWN,0,INPUT_MASK_BACKWARDS},
	{ACTION_TRAVERSE_LEFT,0,INPUT_MASK_LEFT},
	{ACTION_TRAVERSE_RIGHT,0,INPUT_MASK_RIGHT},
	{0,0,0}
};


struct ActionInfo	action_climbing[]=
{
	{ACTION_DROP_DOWN,0,INPUT_MASK_JUMP},
	{0,0,0}
};

struct ActionInfo	action_cable[]=
{
	{ACTION_DROP_DOWN,0,INPUT_MASK_JUMP},
	{0,0,0}
};

struct ActionInfo	action_aim_gun[]=
{
	{ACTION_SHOOT,0,INPUT_MASK_PUNCH},
	{ACTION_STANDING_JUMP,0,INPUT_MASK_JUMP},
//	{ACTION_GUN_AWAY,0,INPUT_MASK_SELECT},
	{0,0,0}
};

struct ActionInfo	action_shoot[]=
{
	{ACTION_SHOOT,0,INPUT_MASK_PUNCH},
	{0,0,0}
};

struct ActionInfo	action_pull_up[]=
{
	{0,0,0}
};

struct ActionInfo	action_grabbing_ledge[]=
{
	{0,0,0}
};

struct ActionInfo	action_death_slide[]=
{
	{ACTION_DROP_DOWN,0,INPUT_MASK_JUMP},
	{0,0,0}
};
struct ActionInfo	action_run_jump[]=
{
	{ACTION_KICK_FLAG,0,INPUT_MASK_KICK},
	{0,0,0}
};

struct ActionInfo	action_fight[]=
{
	{ACTION_KICK_FLAG,0,INPUT_MASK_KICK},
	{ACTION_PUNCH_FLAG,0,INPUT_MASK_PUNCH},
	{ACTION_BLOCK_FLAG,0,INPUT_MASK_ACTION},
	{0,0,0}
};
struct ActionInfo	action_dying[]=
{
	{ACTION_BLOCK_FLAG,0,INPUT_MASK_ACTION} //BLOCK}
};

struct ActionInfo	action_grapple[]=
{
	{ACTION_KICK_FLAG,0,INPUT_MASK_KICK},
	{ACTION_PUNCH_FLAG,0,INPUT_MASK_PUNCH},
	{0,0,0}
};

struct ActionInfo	action_grapplee[]=
{
	{ACTION_BLOCK_FLAG,0,INPUT_MASK_ACTION},
	{0,0,0}
};


struct ActionInfo	action_stand_relax[]=
{
//	{ACTION_WALK,0,INPUT_MASK_FORWARDS},
	{ACTION_STANDING_JUMP,0,INPUT_MASK_JUMP},
	{ACTION_STANDING_JUMP_GRAB,0,INPUT_MASK_JUMP},
	{ACTION_WALK_BACK,0,INPUT_MASK_BACKWARDS},
	{0,0,0}
};


struct ActionInfo	action_ladder[] =
{
	{ACTION_DROP_DOWN,0,INPUT_MASK_JUMP},
	{0,0,0}
};

struct ActionInfo	action_dead[] =
{
//	{ACTION_RESPAWN,0,INPUT_MASK_JUMP},
	{ACTION_BLOCK_FLAG,0,INPUT_MASK_ACTION},
	{0,0,0}
};

struct	ActionInfo	action_flip_left[]=
{
	{ACTION_FLIP_LEFT,1,INPUT_MASK_LEFT},
	{0,0,0}
};

struct	ActionInfo	action_flip_right[]=
{
	{ACTION_FLIP_RIGHT,1,INPUT_MASK_RIGHT},
	{0,0,0}
};

struct	ActionInfo	action_hug_wall[]=
{
	{ACTION_HUG_RIGHT,1,INPUT_MASK_RIGHT},
	{ACTION_HUG_LEFT,1,INPUT_MASK_LEFT},
	{0,0,0}
};

struct	ActionInfo	action_sit[]=
{
	{ACTION_UNSIT,0,INPUT_MASK_FORWARDS},
	{ACTION_UNSIT,0,INPUT_MASK_MOVE},
	{0,0,0}
};

struct	ActionInfo	*action_tree[]=
{
	action_idle,
	action_walk,
	action_run,
	action_standing_jump,
	action_standing_jump_grab,
	action_running_jump,
	action_dangling,
	action_pull_up,
	action_stand_relax,
	action_grabbing_ledge,
	0,
	0,
	0,
	action_climbing, //13
	action_fight,
	action_fight,
	action_idle,	// 16 action_idle_fight
	action_cable,	// 17 action cable
	0,
	0,
	action_dying, //dying
	0,
	action_aim_gun,
	action_shoot,	   // Shoot gun
	0,				   // Gun Away
	0,				   // Respawn
	action_dead,	   // Dead
	0, // 27 action_flip_left,				   // flip left
	0, //action_flip_right,				   //flip right
	action_idle,
	0,
	action_run_jump,
	0,
	0,
	action_walk_back,
	action_death_slide,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	action_grapple,
	action_grapplee,
	0,							// Enter vehicle
	0,							// Inside vehicle
	action_sit,							// Sit bench
	action_hug_wall,							// hug wall
	0,											// hug left
	0,											//hug right
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

//
// player has pressed punch, look at her hand to see if anything needs throwing etc...
//
SLONG	player_activate_in_hand(Thing *p_person)
{
	if (p_person->Genus.Person->Flags & FLAG_PERSON_CANNING)
	{
		//
		// Release the coke can.
		//

		set_person_can_release(p_person, 128);

		return (1);
	}

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		if (p_person->Genus.Person->Ware)
		{
			//
			// Can't throw a grenade inside a warehouse!
			//
		}
		else
		{
			if (p_special->Genus.Special->SpecialType == SPECIAL_GRENADE)
			{
				if (p_special->SubState == SPECIAL_SUBSTATE_ACTIVATED)
				{
					//
					// The person should throw the grenade.
					//

					set_person_can_release(p_person, 128);
				}
				else
				{
					//
					// Prime the grenade.
					//

					SPECIAL_prime_grenade(p_special);
				}

				return (1);
			}
		}

		if (p_special->Genus.Special->SpecialType == SPECIAL_EXPLOSIVES)
		{
			//
			// Put down and prime the explosives.
			//

			SPECIAL_set_explosives(p_person);

			return (1);
		}
#ifdef	UNUSED_WIRECUTTER
		if (p_special->Genus.Special->SpecialType == SPECIAL_WIRE_CUTTER)
		{
			if(set_person_cut_fence(p_person))
				return (1);

		}
#endif
	}
	return (0);
}

void	set_player_shoot(Thing *p_person,SLONG param)
{
	if(person_has_gun_out(p_person))
	{
		set_person_shoot(p_person,param);
		return;
	}
	player_activate_in_hand(p_person);

}

void	set_player_punch(Thing *p_person)
{
	if(p_person->Genus.Person->Mode==PERSON_MODE_FIGHT)
	{
		set_person_punch(p_person);
		return;
	}
	if(!player_activate_in_hand(p_person))
	{
		if(p_person->SubState==SUB_STATE_WALKING_BACKWARDS && person_has_gun_out(p_person))
		{
			set_person_shoot(p_person,0);
			return;
		}


		set_person_punch(p_person);
	}
}


//
// Returns TRUE if a person can safely jump. Returns
// FALSE if they are in the doorway to a warehouse.
//

SLONG should_i_jump(Thing *darci)
{
	//
	// Is Darci standing in a warehouse doorway?
	//

	// SLONG dx = SIN(darci->Draw.Tweened->Angle) >> 11;
	// SLONG dz = COS(darci->Draw.Tweened->Angle) >> 11;

	const SLONG dx = 0x10000 >> 11;
	const SLONG dz = 0x00000 >> 11;

	SLONG x1 = (darci->WorldPos.X >> 8);
	SLONG z1 = (darci->WorldPos.Z >> 8);

	SLONG x2 = x1 + dx;
	SLONG z2 = z1 + dz;

	SLONG x3 = x1 - dz;
	SLONG z3 = z1 + dx;

	SLONG x4 = x1 + dz;
	SLONG z4 = z1 - dx;

	x1 -= dx;
	z1 -= dz;

	x1 >>= 8;
	z1 >>= 8;

	x2 >>= 8;
	z2 >>= 8;

	x3 >>= 8;
	z3 >>= 8;

	x4 >>= 8;
	z4 >>= 8;

	if (x1 == x2 && z1 == z2 &&
		x1 == x3 && z1 == z3 &&
		x1 == x4 && z1 == z4)
	{
		return TRUE;
	}
	else
	{
		return
			WARE_which_contains(x1,z1) == WARE_which_contains(x2,z2) &&
			WARE_which_contains(x3,z3) == WARE_which_contains(x4,z4);
	}
}


//
// Returns TRUE if a person can safely backflip.
//

SLONG should_person_backflip(Thing *darci)
{
	#if 0

	//
	// Don't backflip on lifts when playing "Grim Gardens"
	//

	if (darci->OnFace>0)
	{
		ASSERT(WITHIN(darci->OnFace, 1, next_prim_face4 - 1));

		PrimFace4 *f4 = &prim_faces4[darci->OnFace];

		ASSERT(f4->FaceFlags & FACE_FLAG_WALKABLE);

		if (f4->FaceFlags & FACE_FLAG_WMOVE)
		{
			//
			// If you're on a lift on botanic, then you can't backflip.
			//

			SLONG wmove_index;

			wmove_index = f4->ThingIndex;

			ASSERT(WITHIN(wmove_index, 1, WMOVE_face_upto - 1));

			if (WMOVE_face[wmove_index].thing)
			{
				Thing *p_plat = TO_THING(WMOVE_face[wmove_index].thing);

				if (p_plat->Class == CLASS_PLAT)
				{
					//
					// You can't backflip on platform on "Grim Gardens" a.k.a. botanicc.ucm
					//

					extern SLONG playing_level(const CBYTE *name);

					if (playing_level("botanicc.ucm"))
					{
						return FALSE;
					}
				}
			}
		}
	}

	#endif

	if(darci->Genus.Person->Ware)
	{
		SLONG	px,py,pz,wy;
		//
		// slide along warehouse roof
		//

		calc_sub_objects_position(darci,darci->Draw.Tweened->AnimTween,SUB_OBJECT_HEAD,&px,&py,&pz);
		px+=(darci->WorldPos.X)>>8;
		py+=(darci->WorldPos.Y)>>8;
		pz+=(darci->WorldPos.Z)>>8;

		wy=MAVHEIGHT(px>>8,pz>>8)<<6;
//		if((py)+128 > wy)
		if(wy-py<340)
		{
//			ASSERT(0);
			return(0);
		}

	}

	SLONG dx = SIN(darci->Draw.Tweened->Angle) >> 8;
	SLONG dz = COS(darci->Draw.Tweened->Angle) >> 8;

	SLONG x1 = (darci->WorldPos.X >> 8);
	SLONG y1 = (darci->WorldPos.Y >> 8) + 0x60;
	SLONG z1 = (darci->WorldPos.Z >> 8);

	SLONG x2 = x1 + dx;
	SLONG y2 = y1;
	SLONG z2 = z1 + dz;

	return there_is_a_los(
				x1, y1, z1,
				x2, y2, z2,
				LOS_FLAG_IGNORE_UNDERGROUND_CHECK);
}


SLONG	bad_place_for_car(Thing *p_person,Thing *p_vehicle)
{
	SLONG dx;
	SLONG dz;
	SLONG ix,jx;
	SLONG iz,jz;
	SLONG width,length;

	SLONG	dist,vx,vz,on;

	//
	// Are we near it's door?
	//

	dx = SIN(p_vehicle->Genus.Vehicle->Angle);
	dz = COS(p_vehicle->Genus.Vehicle->Angle);


	ix = p_vehicle->WorldPos.X >> 8;
	iz = p_vehicle->WorldPos.Z >> 8;

	jx=ix;
	jz=iz;


	ix += (dx*512) >> 16;
	iz += (dz*512) >> 16;

	jx -= (dx*512) >> 16;
	jz -= (dz*512) >> 16;
/*
	AENG_world_line(
		ix,0,iz,
		32,
		0xff0000,
		jx,0,jz,
		16,
		0x0000ff,
		TRUE);
*/

	signed_dist_to_line_with_normal(ix,iz,jx,jz,
		p_person->WorldPos.X>>8,
		p_person->WorldPos.Z>>8,
		&dist,&vx,&vz,&on);

	//
	//120 for vans 85 for cars!
	//

	if(abs(dist)<100 || abs(dist>200))
		return(1);




	return(0);

}

//
// Returns TRUE if a person can get into a particular car.
//
extern	void	get_car_door_offsets(SLONG type,SLONG door, SLONG *dx,SLONG *dz);

void	get_car_enter_xz(Thing *p_vehicle,SLONG door, SLONG *cx,SLONG *cz)
{
	SLONG dx;
	SLONG dz;
	SLONG ix;
	SLONG iz;
	SLONG dist;
	SLONG width,length;

	ASSERT(door == 0 || door == 1);

	get_car_door_offsets(p_vehicle->Genus.Vehicle->Type,door,&width,&length);

	//
	// Are we near it's door?
	//

	dx = -SIN(p_vehicle->Genus.Vehicle->Angle);
	dz = -COS(p_vehicle->Genus.Vehicle->Angle);


	ix = p_vehicle->WorldPos.X >> 8;
	iz = p_vehicle->WorldPos.Z >> 8;

	ix += (dx*length) >> 16;
	iz += (dz*length) >> 16;

	ix += (dz*width) >> 16;
	iz -= (dx*width) >> 16;

	//
	// (ix,iz) is now the position of the door.
	//

	*cx=ix;
	*cz=iz;

	#ifndef NDEBUG
#ifndef	PSX
void SHAPE_semisphere(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,	// Gives the and direction of the semi-sphere.
		SLONG dy,
		SLONG dz,
		SLONG radius,
		SLONG page,
		UBYTE red,
		UBYTE green,
		UBYTE blue);

//	SHAPE_semisphere(*cx,0,*cz,0,256,0,0xb0,POLY_PAGE_COLOUR_ALPHA,255,0,0);
#endif
/*

	AENG_world_line(
		p_vehicle->WorldPos.X >> 8,
		(p_vehicle->WorldPos.Y >> 8) + 0x60,
		p_vehicle->WorldPos.Z >> 8,
		32,
		0xff0000,
		ix,
		p_vehicle->WorldPos.X >> 8,
		iz,
		16,
		0x0000ff,
		TRUE);
*/

	#endif
}


SLONG in_right_place_for_car(Thing *p_person, Thing *p_vehicle, SLONG *door)
{
	SLONG i;

	SLONG	ix,iz;
	SLONG dx,dy;
	SLONG dz;
	SLONG dist;

	ASSERT(p_person ->Class == CLASS_PERSON );
	ASSERT(p_vehicle->Class == CLASS_VEHICLE);

	if(bad_place_for_car(p_person,p_vehicle))
	{
		return(0);
	}

	for (i = 0; i <= 1; i++)
	{
extern	UBYTE sneaky_do_it_for_positioning_a_person_to_do_the_enter_anim;

		get_car_enter_xz(p_vehicle,i,&ix,&iz);

		//
		// (ix,iz) is now the position of the door.
		//

		#ifndef NDEBUG
/*
		AENG_world_line(
			p_vehicle->WorldPos.X >> 8,
			p_vehicle->WorldPos.Y >> 8,
			p_vehicle->WorldPos.Z >> 8,
			32,
			0xffffff,
			ix,
			p_person->WorldPos.Y >> 8,
			iz,
			0,
			0x03f008,
			TRUE);
*/

		#endif

		dx = (p_person->WorldPos.X >> 8) - ix;
		dy = (p_person->WorldPos.Y >> 8) - ((p_vehicle->WorldPos.Y>>8)-90);
		dz = (p_person->WorldPos.Z >> 8) - iz;

		dist = QDIST2(abs(dx), abs(dz));

		if (dist < 0xb0 && abs(dy)<64)
		{
			dy=PAP_calc_map_height_at(ix,iz)-(p_person->WorldPos.Y>>8);
			if(abs(dy)<64)
			{
			   *door = i;

				ASSERT(*door == 0 || *door == 1);

				return TRUE;
			}
		}
	}

	return FALSE;
}



SLONG person_get_in_specific_car(Thing *p_person, Thing *p_vehicle, SLONG *door)
{
	if (p_vehicle->Genus.Vehicle->Driver)
	{
		//
		// This vehicle is already occupied.
		//

		return FALSE;
	}

	if (p_vehicle->State == STATE_DEAD)
	{
		//
		// Broken car!
		//

		return FALSE;
	}

	//
	// Are we near it's door?
	//

	return in_right_place_for_car(p_person, p_vehicle, door);
}


//
// Returns TRUE if the person can get into a car. Sets the person BUMP_CAR flag and
// their InCar field to be the index of the car they can get into and sets *door
// to be 0 or 1 depending on which door they are getting into.
//

SLONG person_get_in_car(Thing *p_thing, SLONG *door)
{
	SLONG i;

	Thing *col_thing;

	#define MAX_COL_WITH 16

	THING_INDEX col_with[MAX_COL_WITH];
	SLONG       col_with_upto;

	col_with_upto = THING_find_sphere(
					    p_thing->WorldPos.X >> 8,
						p_thing->WorldPos.Y >> 8,
						p_thing->WorldPos.Z >> 8,
						0x180,	// Some things can be quite big...
						col_with,
						MAX_COL_WITH,
						1 << CLASS_VEHICLE);

	ASSERT(col_with_upto <= MAX_COL_WITH);

	for (i = 0; i < col_with_upto; i++)
	{
		col_thing = TO_THING(col_with[i]);

		if (person_get_in_specific_car(p_thing, col_thing, door))
		{
			p_thing->Genus.Person->InCar = THING_NUMBER(col_thing);

			return TRUE;
		}
	}

	return FALSE;
}


//
// Returns the new input after the action has been performed.
//


//#ifdef	FINAL
//#define	PANEL_new_text(a,b,c)
//#endif
ULONG do_an_action(Thing *p_thing, ULONG input)
{
	ULONG	     closest;
	SLONG      	 ladder_col;
	Thing	    *special_thing;
	THING_INDEX  anim_switch;
	THING_INDEX  special_index;
	SLONG        dist;
	SLONG        door;

	if (p_thing->SubState == SUB_STATE_GRAPPLE_HELD||p_thing->SubState == SUB_STATE_GRAPPLE_HOLD)
	{
		//PANEL_new_text(NULL,4000,"ACTION cancel grapple");
		return(0);
	}
	if (p_thing->State ==SUB_STATE_IDLE_CROUTCH_ARREST)
	{
		//PANEL_new_text(NULL,4000,"ACTION cancel arresting");
		return(0);
	}


	if (p_thing->State == STATE_GUN)
	{
		if (p_thing->SubState == SUB_STATE_DRAW_GUN ||
			p_thing->SubState == SUB_STATE_DRAW_ITEM)
		{
			//
			// Wait 'till you've finished drawing your gun!
			//

				//PANEL_new_text(NULL,4000,"ACTION cancel drawing gun");
			return 0;
		}
	}

	/*

	if(p_thing->State==STATE_IDLE)
	if(input&INPUT_MASK_ACTION)
	{
		if(input&INPUT_MASK_LEFT)
			set_person_flip(p_thing,0);
		else
		if(input&INPUT_MASK_RIGHT)
			set_person_flip(p_thing,1);
	}

	*/

	//if(p_thing->SubState==SUB_STATE_HUG_WALL_STAND||p_thing->SubState==SUB_STATE_HUG_WALL_LOOK_L
extern	SLONG	can_i_hug_wall(Thing *p_person);
extern	void	set_person_turn_to_hug_wall(Thing *p_person);

	if(p_thing->State==STATE_CARRY)
	{
		if (p_thing->SubState == SUB_STATE_PICKUP_CARRY ||
			p_thing->SubState == SUB_STATE_DROP_CARRY)
		{
			//
			// Don't do anything while picking somebody up or down...
			//

			return INPUT_MASK_ACTION;
		}

		extern SLONG is_there_room_in_front_of_me(Thing *p_person, SLONG how_much_room);

		if (!is_there_room_in_front_of_me(p_thing, 192))
		{
/*
			add_damage_text(
				p_thing->WorldPos.X          >> 8,
				p_thing->WorldPos.Y + 0x6000 >> 8,
				p_thing->WorldPos.Z          >> 8,
				"No room!");
*/
		}
		else
		{
			extern void set_person_uncarry(Thing *p_person);

			set_person_uncarry(p_thing);
		}

		//PANEL_new_text(NULL,4000,"ACTION uncarry");
		return(INPUT_MASK_ACTION);

	}
	if(p_thing->SubState==SUB_STATE_HUG_WALL_LOOK_L)
	{
		set_person_hug_wall_leap_out(p_thing,0);
		//PANEL_new_text(NULL,4000,"ACTION leapout");
		return(INPUT_MASK_ACTION);

	}
	if(p_thing->SubState==SUB_STATE_HUG_WALL_LOOK_R)
	{
		//PANEL_new_text(NULL,4000,"ACTION leapout");
		set_person_hug_wall_leap_out(p_thing,1);
		return(INPUT_MASK_ACTION);

	}

	if(p_thing->State == STATE_DEAD ||p_thing->State == STATE_DYING||p_thing->State == STATE_SEARCH)
	{
		//PANEL_new_text(NULL,4000,"ACTION dead/search");
		return(0);
	}

	if (p_thing->Genus.Person->Flags&FLAG_PERSON_DRIVING)
	{
		Thing*	p_vehicle = TO_THING(p_thing->Genus.Person->InCar);

		if (p_vehicle->Genus.Vehicle->GrabAction&&(p_vehicle->Velocity>5))
		{
			//PANEL_new_text(NULL,4000,"ACTION cancel incar");

			return 0;	// button grabbed by car
		}

		if (abs(p_vehicle->Velocity) >= 50)
		{
			//PANEL_new_text(NULL,4000,"ACTION cancel incar toofast");
			return 0;	// moving too fast
		}

		if (p_vehicle->Genus.Vehicle->Skid >= 3) // SKID_START defined in vehicle.cpp!
		{
			SLONG	dist;

			dist=QDIST2(abs(p_vehicle->Genus.Vehicle->VelX),abs(p_vehicle->Genus.Vehicle->VelZ));

			if(abs(dist)>500)
				return 0;

			//PANEL_new_text(NULL,4000,"Skidding!");

		}

		if (p_vehicle->Genus.Vehicle->Flags & FLAG_VEH_IN_AIR)
		{
			//PANEL_new_text(NULL,4000,"In air!");

			return 0;
		}

		set_person_exit_vehicle(p_thing);
		//PANEL_new_text(NULL,4000,"ACTION exit car");
		return (INPUT_MASK_ACTION);
	}

	#ifdef BIKE
	if (p_thing->Genus.Person->Flags & FLAG_PERSON_BIKING)
	{
		//
		// Get off the bike.
		//

		set_person_dismount_bike(p_thing);

		return INPUT_MASK_ACTION;
	}
	#endif

#ifndef PSX
#ifndef TARGET_DC
	if (p_thing->Genus.Person->Flags & FLAG_PERSON_GRAPPLING)
	{
		//
		// Release the grappling hook.
		//

		set_person_grappling_hook_release(p_thing);
		//PANEL_new_text(NULL,4000,"ACTION hook");

		return INPUT_MASK_ACTION;
	}
#endif
#endif
	if (p_thing->State != STATE_MOVEING)
	{
		if ((p_thing->State    == STATE_IDLE   || (p_thing->State    == STATE_GUN && p_thing->SubState==SUB_STATE_AIM_GUN))  &&
			p_thing->SubState != SUB_STATE_IDLE_CROUTCH &&
			p_thing->SubState != SUB_STATE_IDLE_CROUTCHING)
		{
			//
			// Find someone to arrest?
			//

			{
				UWORD index;

				extern UWORD find_arrestee(Thing *p_person);
				//PANEL_new_text(NULL,4000,"ACTION find arrest");

				if (p_thing->Genus.Person->PersonType == PERSON_DARCI && (index = find_arrestee(p_thing)))
				{
					set_person_arrest(p_thing, index);

					//PANEL_new_text(NULL,4000,"ACTION DO arrest");

					return INPUT_MASK_ACTION;
				}
			}


			//
			//climb down a ladder?
			//
			{
				SLONG	ladder_col;
				struct	DFacet	*p_facet;
				ladder_col = find_nearby_ladder_colvect(p_thing);

	SLONG	set_person_climb_down_onto_ladder(Thing *p_person,SLONG colvect);

				if (ladder_col)
				{
					SLONG	top;
					//
					// Mount the ladder.  This could call set_person_climb_ladder(),
					// but then it would need the storey instead of the col_vect.
					//
					p_facet=&dfacets[ladder_col];

					top = p_facet->Y[0] + p_facet->Height * 64;

					if(abs(top-(p_thing->WorldPos.Y>>8))<64)
					{
						if(set_person_climb_down_onto_ladder(p_thing,ladder_col))
						{
							return (INPUT_MASK_ACTION);
							//PANEL_new_text(NULL,4000,"ACTION climb down ladder");
						}
					}


				}
			}


		}

extern	SLONG	person_on_floor(Thing *p_person);
		if(person_on_floor(p_thing))
		if (person_get_in_car(p_thing, &door))
		{
			if(TO_THING(p_thing->Genus.Person->InCar)->Class==CLASS_VEHICLE)
			{
				p_thing->Genus.Person->SpecialUse = NULL;
				p_thing->Draw.Tweened->PersonID&=  ~0xe0;
			//	p_thing->Draw.Tweened->PersonID=0;
				p_thing->Genus.Person->Flags&=~FLAG_PERSON_GUN_OUT;

				set_person_enter_vehicle(p_thing,TO_THING(p_thing->Genus.Person->InCar), door);
				//PANEL_new_text(NULL,4000,"ACTION enter car");
				if(p_thing->SubState == SUB_STATE_ENTERING_VEHICLE)
					return (INPUT_MASK_ACTION);
			}
		}

//		if(near_ladder_top(p_thing))

#ifdef	USE_ACTION_TO_THROW_GRENADE
#error
		{
			Thing *p_special = person_has_special(p_thing, SPECIAL_MINE);

			if (p_special)
			{
				if (p_special->SubState == SPECIAL_SUBSTATE_ACTIVATED)
				{
					//
					// Argh! Darci is carrying an activated mine. We had
					// better let her throw it.
					//

					set_person_can_release(p_thing, 128);
					//PANEL_new_text(NULL,4000,"ACTION throw can");

					return INPUT_MASK_ACTION;
				}
			}
		}

		if (p_thing->Genus.Person->Flags & FLAG_PERSON_CANNING)
		{
			//
			// Release the coke can.
			//

			set_person_can_release(p_thing, 128);
				//PANEL_new_text(NULL,4000,"ACTION throw can2");

			return INPUT_MASK_ACTION;
		}

		if (p_thing->Genus.Person->SpecialUse)
		{
			Thing *p_special = TO_THING(p_thing->Genus.Person->SpecialUse);

			if (p_special->Genus.Special->SpecialType == SPECIAL_GRENADE)
			{
				if (p_special->SubState == SPECIAL_SUBSTATE_ACTIVATED)
				{
					//
					// The person should throw the grenade.
					//

					//PANEL_new_text(NULL,4000,"ACTION throw grenade");
					set_person_can_release(p_thing, 128);
				}
				else
				{
					//
					// Prime the grenade.
					//

					//PANEL_new_text(NULL,4000,"ACTION prime grenade");
					SPECIAL_prime_grenade(p_special);
				}

				return INPUT_MASK_ACTION;
			}

			if (p_special->Genus.Special->SpecialType == SPECIAL_EXPLOSIVES)
			{
				//
				// Put down and prime the explosives.
				//

				SPECIAL_set_explosives(p_thing);
					//PANEL_new_text(NULL,4000,"ACTION arm explosive");

				return INPUT_MASK_ACTION;
			}
			if (p_special->Genus.Special->SpecialType == SPECIAL_WIRE_CUTTER)
			{
				if(set_person_cut_fence(p_thing))
				{
					//PANEL_new_text(NULL,4000,"ACTION cut fence");
					return INPUT_MASK_ACTION;
				}

			}
		}
#endif

		//
		// Near a barrel?
		//

	}

#ifndef PSX
#ifndef TARGET_DC
	//
	// How far is Darci from the grappling hook?
	//
	if(p_thing->State != STATE_MOVEING)
	{
		SLONG hook_x;
		SLONG hook_y;
		SLONG hook_z;
		SLONG hook_yaw;
		SLONG hook_pitch;
		SLONG hook_roll;

		HOOK_pos_grapple(
			&hook_x,
			&hook_y,
			&hook_z,
			&hook_yaw,
			&hook_pitch,
			&hook_roll);

		SLONG dx = p_thing->WorldPos.X - hook_x >> 8;
		SLONG dy = p_thing->WorldPos.Y - hook_y >> 8;
		SLONG dz = p_thing->WorldPos.Z - hook_z >> 8;

		if (abs(dy) < 0x20)
		{
			dist = abs(dx) + abs(dz);

			if (dist < 0x80)
			{
				//
				// Near enough, so bend down to pickup the grappling hook.
				//

				//PANEL_new_text(NULL,4000,"ACTION pickup hook");
				set_person_grappling_hook_pickup(p_thing);

				return INPUT_MASK_ACTION;
			}
		}
	}

#endif
#endif


	if ((p_thing->State    == STATE_IDLE   || (p_thing->State    == STATE_GUN && p_thing->SubState==SUB_STATE_AIM_GUN))  &&
		p_thing->SubState != SUB_STATE_IDLE_CROUTCH &&
		p_thing->SubState != SUB_STATE_IDLE_CROUTCHING)
	{
		#if 0	// We don't use anim_prim_switches any more

		//
		// If we are near a switch, then action toggles it.
		//

		anim_switch = find_anim_prim(
						(p_thing->WorldPos.X >> 8),
						(p_thing->WorldPos.Y >> 8) + 0x40,
						(p_thing->WorldPos.Z >> 8),
						0x100,
						(1 << ANIM_PRIM_TYPE_SWITCH));

		if (anim_switch)
		{
			//
			// We have found a switch, so try and toggle it.
			//

			//PANEL_new_text(NULL,4000,"ACTION toggle switch");
			toggle_anim_prim_switch_state(anim_switch);

			return (INPUT_MASK_ACTION);
		}

		#endif

/* moved up a bit
		//
		// Find someone to arrest?
		//

		{
			UWORD index;

			extern UWORD find_arrestee(Thing *p_person);

			if (p_thing->Genus.Person->PersonType == PERSON_DARCI && (index = find_arrestee(p_thing)))
			{
				set_person_arrest(p_thing, index);

				return INPUT_MASK_ACTION;
			}
		}
*/

		//
		// Near a switch or a valve?
		//

		{
			SLONG ob_x;
			SLONG ob_y;
			SLONG ob_z;
			SLONG ob_yaw;
			SLONG ob_prim;
			SLONG ob_index;

			if (OB_find_type(
					p_thing->WorldPos.X >> 8,
					p_thing->WorldPos.Y >> 8,
					p_thing->WorldPos.Z >> 8,
					0x180,
					FIND_OB_SWITCH_OR_VALVE,
				   &ob_x,
				   &ob_y,
				   &ob_z,
				   &ob_yaw,
				   &ob_prim,
				   &ob_index))
			{
				//
				// In the correctish place?
				//

				SLONG anim;

				SLONG want_x;
				SLONG want_z;

				if (ob_prim == PRIM_OBJ_SWITCH_OFF)
				{
					anim   = ANIM_BUTTON;
					want_x = ob_x + (SIN(ob_yaw) >> 10);
					want_z = ob_z + (COS(ob_yaw) >> 10);
				}
				else
				{
					anim  = ANIM_VALVE_LOOP;
					want_x = ob_x + (SIN(ob_yaw) >> 8) - (SIN(ob_yaw) >> 10);
					want_z = ob_z + (COS(ob_yaw) >> 8) - (COS(ob_yaw) >> 10);
				}

				SLONG dx = abs(want_x - (p_thing->WorldPos.X >> 8));
				SLONG dz = abs(want_z - (p_thing->WorldPos.Z >> 8));

				SLONG dist = QDIST2(dx,dz);

				if (dist < 0x40)
				{
					//
					// Facing in the right direction?
					//

					SLONG dangle;

					dangle  = p_thing->Draw.Tweened->Angle - ob_yaw;
					dangle &= 2047;

					if (dangle > 1024)
					{
						dangle -= 2048;
					}

					if (abs(dangle) < 200)
					{
						//
						// Move to the right place?
						//

						GameCoord newpos;

						newpos.X = want_x << 8;
						newpos.Y = p_thing->WorldPos.Y;
						newpos.Z = want_z << 8;

						move_thing_on_map(p_thing, &newpos);

						p_thing->Draw.Tweened->Angle = ob_yaw;

						//
						// Press the switch.
						//

						p_thing->Genus.Person->Timer1=2;
						set_person_do_a_simple_anim(p_thing, anim);

						//
						// Tell the waypoint system.
						//

						EWAY_prim_activated(ob_index);

						//
						// Turn the switch green!
						//

						if (OB_ob[ob_index].prim == PRIM_OBJ_SWITCH_OFF)
						{
							OB_ob[ob_index].prim = PRIM_OBJ_SWITCH_ON;
						}

						//PANEL_new_text(NULL,4000,"ACTION do switch");
						return INPUT_MASK_ACTION;
					}
				}
			}
		}

			//
			// Find someone to arrest? just to make sure
			//

			{
				UWORD index;

				extern UWORD find_arrestee(Thing *p_person);

				//PANEL_new_text(NULL,4000,"ACTION FIND arrest");
				if (p_thing->Genus.Person->PersonType == PERSON_DARCI && (index = find_arrestee(p_thing)))
				{
					set_person_arrest(p_thing, index);
					//PANEL_new_text(NULL,4000,"ACTION DO arrest");

					return INPUT_MASK_ACTION;
				}
			}


		//
		// Shall we bend down to pick up a special?
		//

		special_index = THING_find_nearest(
							p_thing->WorldPos.X >> 8,
							p_thing->WorldPos.Y >> 8,
							p_thing->WorldPos.Z >> 8,
							0xa0, //made same as canning_get_special
							1 << CLASS_SPECIAL);

		if (special_index)
		{
			special_thing = TO_THING(special_index);

			if (should_person_get_item(p_thing, special_thing))
			{
				//
				// Bend down to pick up the special.
				//

				set_person_special_pickup(p_thing);
					//PANEL_new_text(NULL,4000,"ACTION pickup special");

				return (INPUT_MASK_ACTION);
			}
		}

		//
		// Player hits 'use' in a radius
		//

		if (EWAY_magic_radius_flag) {
			EWAY_Way *test = EWAY_magic_radius_flag;
			EWAY_magic_radius_flag=0;
			EWAY_evaluate_condition(test, &test->ec);
			if (EWAY_magic_radius_flag)
			{
				EWAY_set_active(EWAY_magic_radius_flag);
				EWAY_magic_radius_flag=0;
				//PANEL_new_text(NULL,4000,"ACTION MAGIC use");
				return INPUT_MASK_ACTION;
			}
		}

		EWAY_magic_radius_flag=0;

		//
		// Using a person (only if you're on the mapwho)
		//

		if (p_thing->Flags & FLAGS_ON_MAPWHO)
		{


//#ifndef	PSX
			if(p_thing->Genus.Person->PersonType==PERSON_ROPER)
			{
				SLONG	index;
				if((index=find_corpse(p_thing)))
				{
					set_person_carry(p_thing,index);
					//PANEL_new_text(NULL,4000,"ACTION roper carry");
					return INPUT_MASK_ACTION;

				}
			}
//#endif
			{
			//	SLONG	ob_index;
				OB_Info *oi;

extern	SLONG OB_find_type(SLONG  mid_x,SLONG  mid_y,SLONG  mid_z,SLONG  max_range,ULONG  prim_flags,SLONG *ob_x,SLONG *ob_y,SLONG *ob_z,SLONG *ob_yaw,SLONG *ob_prim, SLONG *ob_index);

//				if(OB_find_type(p_thing->WorldPos.X>>8,p_thing->WorldPos.Y>>8,p_thing->WorldPos.Z>>8,256,0xff,&ob_x,&ob_y,&ob_z,&ob_yaw,&ob_prim))
				if(oi=OB_find_index(p_thing->WorldPos.X>>8,p_thing->WorldPos.Y>>8,p_thing->WorldPos.Z>>8,256,TRUE))
				{
					if(oi)
					if(set_person_search(p_thing,oi->index,oi->x,oi->y,oi->z))
					{
						//PANEL_new_text(NULL,4000,"ACTION search prim");
						return 0;
					}

				}

				{
					SLONG	index;
					index=find_searchable_person(p_thing);
					if(index)
					{
						//PANEL_new_text(NULL,4000,"ACTION search corpse");
						set_person_search_corpse(p_thing,TO_THING(index));
						return 0;

					}

				}
			}


			//
			// Find our nearest person not including ourselves.
			//

			remove_thing_from_map(p_thing);

			ULONG use = THING_find_nearest(
							p_thing->WorldPos.X >> 8,
							p_thing->WorldPos.Y >> 8,
							p_thing->WorldPos.Z >> 8,
							0xa0,
							1 << CLASS_PERSON);

			add_thing_to_map(p_thing);

			//
			// If this person is useable...
			//


extern	UBYTE	is_semtex;

#ifdef	PSX
			if(wad_level==25 && use==193) //psx way of skipping it
			{
			}
			else
#else
			 if(is_semtex && (use == 193 || use == 195) )
			 {
				 // skip the wetback line in stern revenge, (skymiss2)  PC && DREAMCAST
			 }
			 else
#endif
			if (use)
			{
				Thing *p_person = TO_THING(use);

				ASSERT(p_person->Class == CLASS_PERSON);

				if (p_person->Genus.Person->Flags & FLAG_PERSON_USEABLE)
				{
					//
					// Is this person doing something that can be interrupted?
					//

					if (PCOM_person_doing_nothing_important(p_person))
					{
						//
						// Pressed action near a useable person! Alert the waypoint system.
						//

						if (EWAY_used_person(use))
						{
//							ASSERT(0);
							//
							// The waypoint system is triggering off a message. Make the
							// person talk to darci.
							//

							PCOM_make_people_talk_to_eachother(
								TO_THING(use),
								p_thing,
								FALSE,
								FALSE);
							//PANEL_new_text(NULL,4000,"ACTION use person");
							return INPUT_MASK_ACTION;
						}
					}

					//PANEL_new_text(NULL,4000,"ACTION use person fail");
					return INPUT_MASK_ACTION;
				}
				else
				if (p_person->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER)
				{
					//
					// Is this person doing something that can be interrupted?
					//

					if(NET_PERSON(0)->Genus.Person->PersonType==PERSON_DARCI)
					if (PCOM_person_doing_nothing_important(p_person))
					{
						SLONG message_type = EWAY_FAKE_MESSAGE_NORMAL;

						//
						// Pick a random sentence for this person to say.
						//
						PCOM_make_people_talk_to_eachother(
							TO_THING(use),
							p_thing,
							FALSE,
							FALSE);


						if (TO_THING(use)->Genus.Person->Flags2 & FLAG2_PERSON_GUILTY)
						{
							message_type = EWAY_FAKE_MESSAGE_GUILTY;
						}
						if (TO_THING(use)->Genus.Person->pcom_ai_memory)
						{
							message_type = EWAY_FAKE_MESSAGE_ANNOYED;
						}

						CBYTE *response = EWAY_get_fake_wander_message(message_type);

						if (response)
						{
							PANEL_new_text(
								TO_THING(use),
								4000,
								response);

							PCOM_make_people_talk_to_eachother(
								TO_THING(use),
								p_thing,
								FALSE,
								FALSE);
						}
					}
				}
			}
		}
	}

#ifndef PSX
#ifdef  BIKE

	//
	// Get onto a bike?
	//

	if(p_thing->State != STATE_MOVEING)
	{
		SLONG bike = BIKE_person_can_mount(p_thing);

		if (bike)
		{
			set_person_mount_bike(p_thing, TO_THING(bike));

			return INPUT_MASK_ACTION;
		}

	}
#endif
#endif
	if (p_thing->State == STATE_IDLE ||(p_thing->State    == STATE_GUN && p_thing->SubState==SUB_STATE_AIM_GUN))
	{

		SLONG	angle;

		/*
		if (THING_find_nearest(
				p_thing->WorldPos.X >> 8,
				p_thing->WorldPos.Y >> 8,
				p_thing->WorldPos.Z >> 8,
				0x80,
				(1 << CLASS_BARREL)))
		{
			set_person_barrel_pickup(p_thing);

			return INPUT_MASK_ACTION;
		}
		*/
		if(p_thing->SubState!=SUB_STATE_IDLE_CROUTCH &&p_thing->SubState!=SUB_STATE_IDLE_CROUTCHING)
		if(angle=can_i_hug_wall(p_thing))
		{
			p_thing->Draw.Tweened->Angle=angle-1;
			set_person_turn_to_hug_wall(p_thing);
			//PANEL_new_text(NULL,4000,"ACTION hug wall");
			return(INPUT_MASK_ACTION);
		}

		//
		// Near a coke can and idle
		//
		if((p_thing->Draw.Tweened->PersonID>>5)==0)
		{
			dist = DIRT_get_nearest_can_or_head_dist(
						p_thing->WorldPos.X >> 8,
						p_thing->WorldPos.Y >> 8,
						p_thing->WorldPos.Z >> 8);

			if (dist < 0x80)
			{
				//
				// Near enough. Bend down to pick up the coke can.
				//

				set_person_can_pickup(p_thing);
				//PANEL_new_text(NULL,4000,"ACTION pickup can");

				return INPUT_MASK_ACTION;
			}
		}


	}

	if (p_thing->State == STATE_IDLE ||
		p_thing->State == STATE_MOVEING)
	{
/*
	// now auto climb ladders
		//
		// Can she mount a ladder? ...so to speak!
		//

		ladder_col = find_nearby_ladder_colvect(p_thing);

		if (ladder_col)
		{
			//
			// Mount the ladder.  This could call set_person_climb_ladder(),
			// but then it would need the storey instead of the col_vect.
			//

			if(mount_ladder(p_thing, ladder_col))
				return (INPUT_MASK_ACTION);
		}
*/


		if (p_thing->State == STATE_IDLE||p_thing->State==STATE_GUN)
		{
			//
			// failed to do any of the other stuff (pickup/flick switch/enter car/...), so croutch
			//
			switch(p_thing->SubState)
			{
				case	0:
/*
					if(p_thing->Draw.Tweened->DRoll	==	-1)
					{
						set_person_flip(p_thing,0);
					}
					else
					if(p_thing->Draw.Tweened->DRoll	==	1)
					{
						set_person_flip(p_thing,1);
					}
*/
/*
					if(p_thing->Draw.Tweened->CurrentAnim	==	ANIM_ROTATE_L)
					{
						set_person_flip(p_thing,0);
					}
					else
					if(p_thing->Draw.Tweened->CurrentAnim	==	ANIM_ROTATE_R)
					{
						set_person_flip(p_thing,1);
					}
					else
*/
						set_person_croutch(p_thing);
						//PANEL_new_text(NULL,4000,"ACTION croutch");
					return 0;//  we need to hold this down so dont use up the action button //INPUT_MASK_ACTION;
					break;
				case	SUB_STATE_IDLE_CROUTCH:
				case	SUB_STATE_IDLE_CROUTCHING:
						//PANEL_new_text(NULL,4000,"ACTION ALLREADY croutch");
					return(0);
					break;
/*
				case	SUB_STATE_IDLE_CROUTCHING:
					set_person_idle(p_thing);
					return INPUT_MASK_ACTION;
					break;
*/

			}
		}

		if (p_thing->State == STATE_MOVEING)
		{
			//
			// failed to do any of the other stuff (pickup/flick switch/enter car/...), so croutch
			//
			switch(p_thing->SubState)
			{
				case	SUB_STATE_RUNNING:
					if((p_thing->Genus.Person->Stamina>3)&&(p_thing->Genus.Person->PersonType!=PERSON_ROPER))
					{
						p_thing->Genus.Person->Mode=PERSON_MODE_SPRINT;
						//PANEL_new_text(NULL,4000,"ACTION ALLREADY SPRINT");
						return (0);
					}
					break;
				case	SUB_STATE_WALKING:
					p_thing->Genus.Person->Mode=PERSON_MODE_RUN;
					break;

			}
		}
	}
	else
	if (p_thing->State==STATE_GUN)
	{
		set_person_croutch(p_thing);
		//PANEL_new_text(NULL,4000,"ACTION croutch gun");
		return(0);
	}

	return (0);//INPUT_MASK_ACTION);
}


extern	void	set_person_hop_back(Thing *p_person);

//
// Given your input button mask, finds the best available option, if any.
// Currently returns the first it finds
//

SLONG	find_best_action_from_tree(SLONG action,ULONG input,ULONG *input_used)
{
	struct	ActionInfo *action_options;


	action_options=action_tree[action];
//	LogText(" FIND BEST current action %d input %x \n",action,input);

	if(action_options)
	{
		while(action_options->Action)
		{
//			LogText("possible action %d input neeed %d input got %d\n",action_options->Action,action_options->Input,input);
			if(action_options->Input)
			{
				if( action_options->Logic==0)
				{
					//
					// logic=0 means any bit set will do
					//

					if( input& action_options->Input ) // action structure may contain a bit mask rather that bit positionj in the future, yep it now does
					{
		//				LogText(" yesssssssssssssss\n");
						*input_used = action_options->Input;
						return(action_options->Action);
					}
				}
				else
				{
					//
					// logic!=0 means all required bits must be set
					//

					if( (input& action_options->Input)==action_options->Input ) // action structure may contain a bit mask rather that bit positionj in the future, yep it now does
					{
		//				LogText(" yesssssssssssssss\n");
						*input_used = action_options->Input;
						return(action_options->Action);
					}
				}
			}
			action_options++;
		}
	}
	return(0);
}


//states

//actions

/*
Moveing
     Walking,
	 Running,
	 Jumping,
	 Swimming,
	 dodgeing,
	 sideling,

Fighting
	Normal Attack
		Punching,
		Kicking,
		Head Butt
	Aerial Attack
		Flying Kick,
		Flying Block,
		Drop From Above Attack
		...

	Grappleing,
	Interactinge With Scenery,
		Vault Attack,
		Pole Spin Attack,
		Rope Swing Kick,
		...
	Blocking,

Interacting With Scenery
	Pulling Up Onto Ledge
	Vaulting
	Climbing Fence
	Climbing Ladder
	Climbing Rope
	Dangling from Item
	Performing an Attack with scenery
	Swing On Rope
	Slide Down death slide

Using a vehicle
	Car
	Van
	MotorBike
	Hangglider

Gun Moves
	Drawing Gun
	Holstering Gun
	Aiming
	Shooting
	Reloading

Idle moves
	Scratch Arse
	Look About
	Limber Up

Changing location/status
	Entering/Leaving Building through door
	Mounting/dismounting Motorbike
	Entering/exiting Vehicle


Summary
-------

NON_CHANGABLE    (the user can not stop/change in mid anim, exit vehicle,punch)
NON_INTERUPTABLE (the anim can not be broken by any means (not even someone jumping on your head), die anim)
PROJECTILE       (Movement is controlled by physics rather than animation)

each action has a list of others you can go to

Dangle			(Looped)
	Pull Up
	Drop Down
	SideWays Movement
	Swing
	Kick
	Take Hit then Fall

Pull Up	To Stand (Exit to Stand)
	Take Hit Then Fall

Pull Up	To Move  (Exit to Movement)
	Take Hit Then Fall

Drop Down (Exit to Land)
	Kick
	Land

Swing	(Exit to Dangle)
	Drop Down
	Kick

Falling
	Kick
	Grab
	Take Hit

Stand
	Fight Moves
	Kick Moves
	Jump Moves


Run
	Close Line (Punch)
	ShoulderBarge
	Forward Roll
	Jump


*/

/*
States & Substates
	Moveing
		Walking
		Running
		Siddling
		TakeHit

	Idle
		Waiting/Watching/Scratching

	Landing
		DropLand
		LandAndRun
		LandAndDie

	Jumping
		Attack
		TakeHit


	Fighting //is fighting a state you enter or is each sub state possible from idle&manouver
		Manouvering
		Performing an Attack
		Grappling
		TakeHit

	Falling
		TakeHit

	SceneryMove
		Dangling
		Vaulting
		Climbing
		Fighting
		Acrobatics
		Open Cupboard
		...

	Down

	TakeHit

	ChangeLocation
		EnterBuilding
		LeaveBuilding
		EnterVehicle
		LeaveVehicle

	DrivingVehicle

*/
//modes

SLONG	get_camera_angle(void)
{
	SLONG	ca;
	SLONG	cam_x,cam_y,cam_z,cam_yaw,cam_pitch,cam_roll,cam_lens;

	if(EWAY_grab_camera(&cam_x,&cam_y,&cam_z,&cam_yaw,&cam_pitch,&cam_roll,&cam_lens))
	{
		ca=cam_yaw>>8;
	}
	else
	{
		ca = FC_cam[0].yaw>>8;
	}

	return(ca);

}

void	player_stop_move(Thing *p_thing,ULONG input)
{
	if (p_thing->State == STATE_GRAPPLING ||
		p_thing->State == STATE_CANNING ||
		p_thing->State == STATE_FIGHTING)
	{
		//
		// Already not moving...
		//

		return;
	}
	if (p_thing->SubState == SUB_STATE_RUNNING_SKID_STOP)
		return;

//	p_thing->Genus.Person->Action=ACTION_STOPPING;
	//if(p_thing->Genus.Person->Flags&FLAG_PERSON_GUN_OUT)
	//{
	//	p_thing->SubState=SUB_STATE_AIM_GUN;
//		return;
//	}
//	if(p_thing->State!=STATE_MOVEING)
//		MSG_add(" set sub to stopping");
	if(p_thing->SubState==SUB_STATE_BLOCK)
		ASSERT(0);
	if(p_thing->SubState!=SUB_STATE_STOPPING&&p_thing->SubState!=SUB_STATE_STOPPING_DEAD&&p_thing->SubState!=SUB_STATE_RUN_STOP&&p_thing->SubState!=SUB_STATE_STOPPING_OT && p_thing->SubState!=SUB_STATE_SLIPPING&& p_thing->SubState!=SUB_STATE_SLIPPING_END && p_thing->SubState!=SUB_STATE_RUNNING_VAULT && p_thing->SubState!=SUB_STATE_RUNNING_HIT_WALL)
	{

//		if(p_thing->Genus.Person->Mode!=0||p_thing->SubState!=SUB_STATE_RUNNING ||person_has_gun_out(p_thing))
		if(1)
		{
     			p_thing->SubState=SUB_STATE_STOPPING;

		}

		else

		{
//			tween_to_anim(p_thing,ANIM_STOP_RUN);
			set_anim(p_thing,ANIM_STOP_RUN);
			p_thing->SubState=SUB_STATE_RUN_STOP;
		}
	}
}

void	get_analogue_dxdz(SLONG in_dx,SLONG in_dz,SLONG *dx,SLONG *dz)
{
	SLONG	angle;
	SLONG	ca;

	SLONG	dist;

	dist=Root(in_dx*in_dx+in_dz*in_dz);


	angle=Arctan(-in_dx,in_dz);


	//
	// Convert camera angle to player angle
	//

	ca = get_camera_angle(); //FC_cam[0].yaw>>8;

	angle+=ca ;//test_view.CameraAngle;
	angle-=512;
	angle=(angle+2048)&2047;


	in_dx=-COS(angle)>>8;
	in_dz=SIN(angle)>>8;

	in_dx=(in_dx*dist)>>8;
	in_dz=(in_dz*dist)>>8;

	SATURATE(in_dx,-128,127);
	SATURATE(in_dz,-128,127);

	*dx=in_dx;
	*dz=in_dz;




}


void	player_interface_move(Thing *p_thing,ULONG input)
{
	//
	// left and right at the same time is not allowed
	//
	if((input&(INPUT_MASK_LEFT|INPUT_MASK_RIGHT))==(INPUT_MASK_LEFT|INPUT_MASK_RIGHT))
		input&=~(INPUT_MASK_LEFT|INPUT_MASK_RIGHT);

	switch(USER_INTERFACE)
	{
		case	0:
			//
			// actual mode used (unless you do the keypress to use the other 'odd' mode
			//
			if (!CONTROLS_inventory_mode)
				player_apply_move(p_thing,input);

			break;
		case	1:
			//
			// old test mode for moveing in direction pressed
			//
				//player_apply_move_analogue(p_thing,input);
			break;
	}
}

void	init_user_interface(void)
{
	USER_INTERFACE=0;
#ifndef PSX
	PANEL_scanner_poo=ENV_get_value_number("scanner_follows",	TRUE, "Game");
#endif
#ifdef TARGET_DC
	analogue = ENV_get_value_number("analogue_pad_mode",	TRUE, "Game");

extern bool g_bEngineVibrations;
	if ( ENV_get_value_number("vibration_mode",	TRUE, "") == 0 )
	{
		SetVibrationEnable ( FALSE );
		g_bEngineVibrations = FALSE;
	}
	else
	{
		SetVibrationEnable ( TRUE );
		if ( ENV_get_value_number("vibration_engine",	TRUE, "") == 0 )
		{
			g_bEngineVibrations = FALSE;
		}
		else
		{
			g_bEngineVibrations = TRUE;
		}
	}
#endif
}

//
// If (flags) then turn left/right
//

//void	rotate_person

#define	DLOCK	32

//
//lock things angle to a compass direction
//
void	lock_to_compass(Thing *p_thing)
{
	SLONG	angle;

	//
	// I bet theres a clever way of doing this but I refuse to use my brain
	//
	angle=p_thing->Draw.Tweened->Angle;

	if(angle<DLOCK || angle>2048-DLOCK)
	{
		angle=0;
	}
	else
	if(angle>512-DLOCK && angle<512+DLOCK)
	{
		angle=512;
	}
	else
	if(angle>1024-DLOCK && angle<1024+DLOCK)
	{
		angle=1024;
	}
	else
	if(angle>1536-DLOCK && angle<1536+DLOCK)
	{
		angle=1536;
	}
	p_thing->Draw.Tweened->Angle=angle;

}


#define	TURN_TIMER	512

#define	JOY_REL_CAMERA	(1<<0)
SLONG	get_joy_angle(ULONG input,UWORD flags)
{
	SLONG	dx=0,dz=0;
	SLONG	angle=-1;
//	SLONG	velocity;


	dx=GET_JOYX(input);
	dz=GET_JOYY(input);

	angle=Arctan(-dx,dz);
//	velocity=QDIST2(abs(dx),abs(dz));

	if(flags&JOY_REL_CAMERA)
	{
		SLONG	ca;

		ca = get_camera_angle(); //FC_cam[0].yaw>>8;
//		ca = FC_cam[0].yaw>>8;

		angle+=ca ;//test_view.CameraAngle;
		angle=(angle+2048)&2047;
	}
	return(angle);
}

extern	SLONG	EWAY_cam_jumped;
#ifdef	MIKE
SLONG	globdx,globdz;
#endif

SLONG	player_turn_left_right_analogue(Thing *p_thing,SLONG input)
{
	static	UBYTE	reduce_turn=0;

	if (EWAY_cam_jumped > 0)
	{
		EWAY_cam_jumped -= 1;
	}

	if(EWAY_cam_jumped)
	{
		reduce_turn=8;
	}
	if(reduce_turn)
	{
		--reduce_turn;
	}

	if(p_thing->State==STATE_SEARCH)
	{
	}
	else
	if(p_thing->Genus.Person->Action == ACTION_SIDE_STEP)
	{
		return 1;
	}
	else
	if(p_thing->SubState==SUB_STATE_RUNNING_HIT_WALL)
	{
	}
	else
	if(p_thing->SubState==SUB_STATE_SIDLE)
	{
		if(input&INPUT_MASK_LEFT)
		{
			set_person_step_right(p_thing);
			return	1;
		}
		else
		if(input&INPUT_MASK_RIGHT)
		{
			set_person_step_left(p_thing);
			return	1;
		}


	}
	else
	{
		SLONG	dx=0,dz=0;
		SLONG	angle=-1;
		SLONG	velocity;

		dx=GET_JOYX(input);
		dz=GET_JOYY(input);

		velocity=QDIST2(abs(dx),abs(dz));

#ifdef	MIKE
//		globdx=dx;
//		globdz=dz;
#endif



		if(velocity>ANALOGUE_MIN_VELOCITY)
			angle=Arctan(-dx,dz);
//		if(p_thing->State==STATE_JUMPING||p_thing->SubState==SUB_STATE_CRAWLING||p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP)


		if(p_thing->Velocity>10 && p_thing->SubState==SUB_STATE_RUNNING)
		{
			p_thing->Draw.Tweened->Roll	=	((((p_thing->Velocity-9)*dx))>>5);
			p_thing->Draw.Tweened->DRoll	=	0;
		}

		if(angle>=0)
		{
			SLONG	ca;
			SLONG	max_angle=128;
			SLONG	dangle;

			ca = get_camera_angle(); //FC_cam[0].yaw>>8;

			if (player_relative)
			{
				ca = p_thing->Draw.Tweened->Angle;
			}

	//		ca = FC_cam[0].yaw>>8;

			angle+=ca ;//test_view.CameraAngle;
			if(p_thing->SubState==SUB_STATE_WALKING_BACKWARDS)
			{
				angle+=1024;

			}
			angle=(angle+2048)&2047;

			//if(p_thing->State==STATE_JUMPING||p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP)
			if(p_thing->State==STATE_JUMPING||p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP)
				max_angle=16;
			if(p_thing->SubState==SUB_STATE_CRAWLING)
				max_angle=64;

			if (player_relative)
			{
				max_angle >>= 1;
			}

			if (!player_relative)
			{
				if(p_thing->State==STATE_IDLE || (p_thing->SubState==SUB_STATE_AIM_GUN))
					max_angle=1024;
			}

			if(reduce_turn)
			{
				max_angle-=(reduce_turn<<5); //<<4== 0 16 32 48 64 80 96 112 128 144 160 176 192 108 224 240 256
				if(max_angle<=0)
					return(0); //cant turn
			}

			{
				dangle=-p_thing->Draw.Tweened->Angle+angle;
				dangle&=2047;
				if(dangle>=1024)
					dangle=-(2048-dangle);

				if(p_thing->Velocity>8)
				{


					dangle/=(p_thing->Velocity)>>3;
				}


//				if(p_thing->SubState==SUB_STATE_RUNNING ||p_thing->SubState==SUB_STATE_WALKING||p_thing->State==STATE_IDLE)


// ============


				if (player_relative)
				{
					if(p_thing->State==STATE_IDLE)// && velocity<ANALOGUE_MIN_VELOCITY*2)
					{
						if(abs(dangle)>1024-400)
						{
							set_person_walk_backwards(p_thing);
							return(0);
						}

					}
					else
					if(p_thing->SubState==SUB_STATE_WALKING_BACKWARDS)
					{
						if(abs(dangle)>600)
						{
							set_person_running(p_thing);
							return(0);

						}

					}
				}

// ============

				if(dangle<-max_angle)
					dangle=-max_angle;
				else
				if(dangle>max_angle)
					dangle=max_angle;

				p_thing->Draw.Tweened->Angle+=dangle;



				p_thing->Draw.Tweened->Angle&=2047;


			}
/*
			else
			{

				p_thing->Draw.Tweened->Angle=angle;
			}
*/
		}

	}
	return(0);

}

void	process_analogue_movement(Thing *p_thing,SLONG input)
{
	SLONG	dx,dz,velocity;
	SLONG	angle=-1;
	SLONG	rel_x,rel_y;
	SLONG	ca;


	dx=GET_JOYX(input);
	dz=GET_JOYY(input);

	velocity=QDIST2(abs(dx),abs(dz));

	angle=Arctan(-dx,dz);
	ca = get_camera_angle(); //FC_cam[0].yaw>>8;

	angle+=ca ;//test_view.CameraAngle;
	angle=(angle+2048)&2047;

	rel_x=COS(angle);
	rel_y=-SIN(angle);

//	rel_x=dx<<8;
//	rel_y=dz<<8;

	//
	// Is the thing facing the camera?
	//

	SLONG dangle = (ca & 2047) - (p_thing->Draw.Tweened->Angle & 20247);

	dangle &= 2047;

	if (dangle >  1024) {dangle -= 2048;}
	if (dangle < -1024) {dangle += 2048;}

	SLONG facing_camera;

	if (abs(dangle) > 512)
	{
		facing_camera = TRUE;
	}
	else
	{
		facing_camera = FALSE;
	}


	if(velocity>ANALOGUE_MIN_VELOCITY)
	{
		switch(p_thing->State)
		{
			case STATE_CARRY:
				if(p_thing->SubState!=SUB_STATE_STAND_CARRY)
				return;
			case	STATE_IDLE:
			case	STATE_HUG_WALL:
				if(p_thing->SubState==SUB_STATE_HUG_WALL_TURN)
					return;

			case	STATE_HIT_RECOIL:
			case	STATE_GUN:
				switch(p_thing->SubState)
				{
					case	0:
					case	SUB_STATE_SIDLE:
					default:
						set_person_running(p_thing);
						break;
					case	SUB_STATE_IDLE_CROUTCHING:
						set_person_crawling(p_thing);
						break;
					case	SUB_STATE_IDLE_CROUTCH:
						if(p_thing->Draw.Tweened->FrameIndex>3)
							return;
						else
							set_person_running(p_thing);
						break;

				}
				break;


				break;

			case	STATE_MOVEING:
				switch(p_thing->SubState)
				{
					case	SUB_STATE_RUNNING:
						if(p_thing->Genus.Person->Mode!=PERSON_MODE_SPRINT)
						{
							if(p_thing->Genus.Person->AnimType!=ANIM_TYPE_ROPER)
							{
								//
								//60 is   1/(128/(40*15/20))*256	//40 is darci yomp speed
								//
								p_thing->Velocity=(velocity*60)>>8;
							}
							else
							{
								//60 is   1/(128/(34*15/20))*256 //34 is roper yomp speed
								p_thing->Velocity=(velocity*51)>>8;
							}

//							if (input & INPUT_MASK_FORWARDS)


							if((p_thing->Velocity<20 || m_bForceWalk ) && p_thing->Genus.Person->AnimType!=ANIM_TYPE_ROPER ) //dont let roper walk
							{
								if (!(p_thing->Genus.Person->Flags2 & FLAG2_PERSON_CARRYING))
								{
									if(p_thing->Velocity>20)
										p_thing->Velocity=20;

									set_person_walking(p_thing);
									p_thing->Genus.Person->Mode=PERSON_MODE_WALK;

								}
							}

						}
						break;
					case	SUB_STATE_STOP_CRAWL:
		     			p_thing->SubState=SUB_STATE_CRAWLING;
						break;
					case	SUB_STATE_CRAWLING:
						break;
					case	SUB_STATE_WALKING:
							if(p_thing->Genus.Person->AnimType!=ANIM_TYPE_ROPER)
							{
								//
								//60 is   1/(128/(40*15/20))*256	//40 is darci yomp speed
								//
								p_thing->Velocity=(velocity*60)>>8;
							}
							else
							{
								//60 is   1/(128/(34*15/20))*256 //34 is roper yomp speed
								p_thing->Velocity=(velocity*51)>>8;
							}
							if((p_thing->Velocity>24 || p_thing->Genus.Person->AnimType==ANIM_TYPE_ROPER) && !m_bForceWalk )
							{
								set_person_running(p_thing);
								p_thing->Genus.Person->Mode=PERSON_MODE_RUN;
							}
							else
							{
								if(p_thing->Velocity>24)
									p_thing->Velocity=24;

							}
						break;
					case	SUB_STATE_STEP_LEFT:
					case	SUB_STATE_STEP_RIGHT:
						set_person_running(p_thing);
						break;

				}
				//need to continue moveing
				break;
			case	STATE_CLIMB_LADDER:
				if(dz<-MAX(abs(dx),8))
				{
					switch(p_thing->SubState)
					{
						case	SUB_STATE_MOUNT_LADDER:
							break;
						case	SUB_STATE_STOPPING:
						case	SUB_STATE_ON_LADDER:
							p_thing->SubState=SUB_STATE_CLIMB_UP_LADDER;
							break;
					}
				}
				break;
			case	STATE_CLIMBING:
				if(dz<0-MAX(abs(dx),8))
				{
					switch(p_thing->SubState)
					{
						case	SUB_STATE_STOPPING:
						case	SUB_STATE_CLIMB_AROUND_WALL:

							if (facing_camera)
							{
								p_thing->SubState=SUB_STATE_CLIMB_DOWN_WALL;
							}
							else
							{
								p_thing->SubState=SUB_STATE_CLIMB_UP_WALL;
							}

							break;
					}
				}
				break;
			case	STATE_DANGLING:
				if(dz<-MAX(abs(dx),8))
				{
					switch(p_thing->SubState)
					{
						case	SUB_STATE_STOPPING:
						case	SUB_STATE_DANGLING_CABLE_FORWARD:
						case	SUB_STATE_DANGLING_CABLE_BACKWARD:
						case	SUB_STATE_DANGLING_CABLE:
							if(p_thing->Flags&FLAG_PERSON_ON_CABLE)
								p_thing->SubState=SUB_STATE_DANGLING_CABLE_FORWARD; //cable errors
							else
 								ASSERT(0);
							break;
					}
				}
				break;


			case	STATE_CANNING:
				break;
		}
	}
	else
	{
		//
		// not pressing move
		//

		switch(p_thing->State)
		{

			case	STATE_MOVEING:
				switch(p_thing->SubState)
				{
					case	SUB_STATE_CRAWLING:
						set_person_idle_croutch(p_thing);
	//		     			p_thing->SubState=SUB_STATE_STOP_CRAWL;
	//						player_stop_crawl(p_thing,input);
						break;
					case	SUB_STATE_RUNNING:
					case	SUB_STATE_WALKING:
					case	SUB_STATE_SIDLE:
						player_stop_move(p_thing,input);
						break;
				}
				break;
		}


	}



	if(dz>MAX(dx,8))
	{
	//		LogText(" have movement input back state %d \n",p_thing->State);
		switch(p_thing->State)
		{

			case	STATE_HUG_WALL:
				if(p_thing->SubState==SUB_STATE_HUG_WALL_TURN)
					return;
//					break;
			case	STATE_IDLE:
			case	STATE_GUN:
	//				set_person_hop_back(p_thing);
				//need to start moveing then
				break;
			case	STATE_MOVEING:
				//need to continue moveing
				break;
			case	STATE_CLIMB_LADDER:
				switch(p_thing->SubState)
				{
					case	SUB_STATE_MOUNT_LADDER:
						break;
					case	SUB_STATE_STOPPING:
					case	SUB_STATE_ON_LADDER:
	//					case	SUB_STATE_CLIMB_UP_LADDER:
	//					case	SUB_STATE_CLIMB_DOWN_LADDER:
	//						p_thing->Draw.Tweened->FrameIndex=0;
						p_thing->SubState=SUB_STATE_CLIMB_DOWN_LADDER;
						break;
				}
				break;
			case	STATE_CLIMBING:
				switch(p_thing->SubState)
				{
					case	SUB_STATE_STOPPING:
	//					case	SUB_STATE_CLIMB_UP_WALL:
					case	SUB_STATE_CLIMB_AROUND_WALL:
	//					case	SUB_STATE_CLIMB_DOWN_WALL:
	//						p_thing->Genus.Person->Timer1=7;

						if (facing_camera)
						{
							p_thing->SubState=SUB_STATE_CLIMB_UP_WALL;
						}
						else
						{
							p_thing->SubState=SUB_STATE_CLIMB_DOWN_WALL;
						}
						break;
				}
			case	STATE_DANGLING:
				switch(p_thing->SubState)
				{
					case	SUB_STATE_STOPPING:
					case	SUB_STATE_DANGLING_CABLE_FORWARD:
					case	SUB_STATE_DANGLING_CABLE_BACKWARD:
					case	SUB_STATE_DANGLING_CABLE:
						if(p_thing->Flags&FLAG_PERSON_ON_CABLE)
							p_thing->SubState=SUB_STATE_DANGLING_CABLE_FORWARD; //cable errors
						else
							ASSERT(0);
						//p_thing->SubState=SUB_STATE_DANGLING_CABLE_BACKWARD;
						break;
				}
				break;

			case	STATE_CANNING:
				break;

		}
	}

}

#if 1
// New, clean style!
SLONG	player_turn_left_right(Thing *p_thing,SLONG input)
{
	if(p_thing->SubState==SUB_STATE_RUNNING_HIT_WALL)
	{
		return(1);
	}

	if(p_thing->Genus.Person->Action == ACTION_SIDE_STEP)
	{
		return 1;
	}

#if 0
	// Er.. I don't think can ever happen - handled by ACTION_SIDE_STEP above.
	ASSERT ( p_thing->SubState != SUB_STATE_SIDLE );
#else
	if(p_thing->SubState==SUB_STATE_SIDLE)
	{
		if(input&INPUT_MASK_LEFT)
		{
			set_person_step_right(p_thing);
		}
		else if(input&INPUT_MASK_RIGHT)
		{
			set_person_step_left(p_thing);
		}
		return 1;
	}
#endif

	SWORD wMaxTurn = 94;

	if(p_thing->State==STATE_JUMPING)
	{
		// Much less turning while in the air.
		wMaxTurn = 12;
	}

#if 0
	// This doesn't actually happen here
	ASSERT ( p_thing->State != STATE_SEARCH )
#else
	if(p_thing->State==STATE_SEARCH)
	{
		// Can't turn while searching.
		wMaxTurn = 0;
	}
#endif

	if ( ( p_thing->SubState==SUB_STATE_RUNNING ) || ( p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP ) )
	{
		// Speed of turn is inversely proportional to velocity.
		// If you're sprinting, you  can't turn sharply.
		// Note that skidding gets modified below as well.
		// Normal running velocity = 30.
		// Sprinting velocity = 52
		SWORD wRunningMaxTurn = 70 - ( p_thing->Velocity );
		ASSERT ( wRunningMaxTurn > 0 );
		if ( wMaxTurn > wRunningMaxTurn )
		{
			wMaxTurn = wRunningMaxTurn;
		}
	}
	if( p_thing->SubState==SUB_STATE_CRAWLING || p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP )
	{
		// Slower turning when crawling or skidding.
		wMaxTurn >>= 1;
	}


	// Now the actual turn is a fraction of this max speed.
	SWORD wJoyX = GET_JOYX(input);		// -128 -> +128
	SWORD wTurn;
	if ((input & INPUT_MASK_LEFT) || (input & INPUT_MASK_RIGHT))
	{
#ifndef TARGET_DC
		// For the PC, you may be using the keyboard, so do a cheesy cumulative thing.
		// This is detected by noticing that the top (analogue section) of input is 0, even though we're turning.
		if ( ( input & 0xfffc0000 ) == 0 )
		{
			// Keyboard only.
			static SWORD wLastTurn = 0;
			if ( input & INPUT_MASK_LEFT )
			{
				if ( wLastTurn > 0 )
				{
					wLastTurn = 0;
				}
				wLastTurn -= 16;
				if ( wLastTurn < -128 )
				{
					wLastTurn = -128;
				}
			}
			else
			{
				ASSERT ( input & INPUT_MASK_RIGHT );
				if ( wLastTurn < 0 )
				{
					wLastTurn = 0;
				}
				wLastTurn += 16;
				if ( wLastTurn > 128 )
				{
					wLastTurn = 128;
				}
			}

			wTurn = wLastTurn;

			SATURATE(wTurn, -wMaxTurn, +wMaxTurn);
		}
		else
#endif
		{
			// Joystick present.
			wTurn = ( wJoyX * wMaxTurn ) >> 7;
		}
	}
	else
	{
		wTurn = 0;
	}


#ifdef DEBUG
	// Sanity checks.
	if (input & INPUT_MASK_LEFT)
	{
		ASSERT ( wTurn <= 0 );
	}
	else if (input & INPUT_MASK_RIGHT)
	{
		ASSERT ( wTurn >= 0 );
	}
	else
	{
		ASSERT ( wTurn == 0 );
	}
#endif

	// Scale by frame rate.
	SWORD wFrameTurn = ( wTurn * TICK_RATIO ) >> TICK_SHIFT;

	// And apply the turn.
	p_thing->Draw.Tweened->Angle = ( p_thing->Draw.Tweened->Angle - wFrameTurn ) & 2047;

	// And now do the leaning correctly.
	// It's simple - lean is proportional to velocity times turn.
	// Also damp the lean, or it looks silly :-)


// Just coz you don't like it, doesn't mean you need to rewrite the whole thing.
// Especially when it means Darci leans fully even if only slightly
// turning, which means she looks like a twat.
//#ifdef	MIKE_I_DONT_LIKE_IT_DISKETT
#if 1

	SWORD wDesiredRoll = ( wTurn * p_thing->Velocity ) >> 2;



	if ( ( p_thing->SubState == SUB_STATE_WALKING_BACKWARDS )
	// These were the two lines of code you needed to add.
	// The magic number 12 is walking speed (not 10).
		|| ( p_thing->Velocity <= 12 )
		|| ( p_thing->State==STATE_JUMPING ) )
	{
		wDesiredRoll = 0;
	}

	// The lean isn't damped independent of frame rate, but that's fine - it's just a visual effect.

	#define MAX_LEAN_DELTA 50

	SWORD wCurrentRoll = p_thing->Draw.Tweened->Roll;
	if ( wCurrentRoll > wDesiredRoll )
	{
		wCurrentRoll -= MAX_LEAN_DELTA;
		if ( wCurrentRoll < wDesiredRoll )
		{
			wCurrentRoll = wDesiredRoll;
		}
	}
	else
	{
		wCurrentRoll += MAX_LEAN_DELTA;
		if ( wCurrentRoll > wDesiredRoll )
		{
			wCurrentRoll = wDesiredRoll;
		}
	}
	p_thing->Draw.Tweened->Roll = wCurrentRoll;
#else

//
// Changed By MikeD to make it not roll while walking, or jumping
//
	p_thing->Draw.Tweened->DRoll	=	0;

	if(wFrameTurn>0)
	if(p_thing->Velocity>10 && p_thing->SubState==SUB_STATE_RUNNING)
		p_thing->Draw.Tweened->DRoll	=	-(p_thing->Velocity-9)>>1;


	if(wFrameTurn<0)
	if(p_thing->Velocity>10 && p_thing->SubState==SUB_STATE_RUNNING)
		p_thing->Draw.Tweened->DRoll	=	((p_thing->Velocity-9)>>1);


#endif


	// Handle the animation.
	if ( p_thing->State==STATE_IDLE && !is_person_crouching(p_thing) )
	{
		p_thing->Draw.Tweened->DRoll	=	-1;
		if ( p_thing->Genus.Person->AnimType != ANIM_TYPE_ROPER )
		{
			if ( (input & INPUT_MASK_LEFT) != 0 )
			{
				if ( p_thing->Draw.Tweened->CurrentAnim != ANIM_ROTATE_L )
				{
					// Start anim.
					set_anim(p_thing,ANIM_ROTATE_L);
					p_thing->Genus.Person->pcom_ai_counter=0;
				}
				else
				{
					// Continue anim.
					ASSERT ( wFrameTurn <= 0 );
					p_thing->Genus.Person->pcom_ai_counter -= wFrameTurn;
					if(p_thing->Genus.Person->pcom_ai_counter>TURN_TIMER)
					{
						p_thing->Draw.Tweened->DRoll	=	0; //tell fn_person_idle to finish off spin anim
					}
				}
			}
			else if ( (input & INPUT_MASK_RIGHT) != 0 )
			{
				if ( p_thing->Draw.Tweened->CurrentAnim != ANIM_ROTATE_R )
				{
					// Start anim.
					set_anim(p_thing,ANIM_ROTATE_R);
					p_thing->Genus.Person->pcom_ai_counter=0;
				}
				else
				{
					// Continue anim.
					ASSERT ( wFrameTurn >= 0 );
					p_thing->Genus.Person->pcom_ai_counter += wFrameTurn;
					if(p_thing->Genus.Person->pcom_ai_counter>TURN_TIMER)
					{
						p_thing->Draw.Tweened->DRoll	=	0; //tell fn_person_idle to finish off spin anim
					}
				}
			}
			else
			{
				// No turning animation.
				p_thing->Genus.Person->pcom_ai_counter=0;
				p_thing->Draw.Tweened->DRoll	=	0;
			}
		}
	}

	return 0;
}

#else

SLONG	player_turn_left_right(Thing *p_thing,SLONG input)
{

	SLONG	da=94;

#if 0	//#ifdef TARGET_DC


	// Take da straight from the analogue stick position.
	da = ( (ULONG)( input & 0x01fc0000 ) >> 18 ) - 64;
	if ( da < 0 )
	{
		da = -da;
	}
	// And scale up to the same as the PC.
	da = (da * 80) / 64;

#else



	// use a static counter (only one player) and increase delta angle linearly
	static SLONG	s_da = 16;

	{
		SLONG joy_dx;
		SLONG turn_speed;
		SLONG max_speed;

		joy_dx = GET_JOYX(input);
		turn_speed = (abs(joy_dx) - 30) >> 3;
		max_speed  = (abs(joy_dx) - 30);

		if ((input & INPUT_MASK_LEFT) || (input & INPUT_MASK_RIGHT))
		{
			da = s_da;
			if (s_da < max_speed)	s_da += turn_speed;
		}
		else
		{
			s_da = 16;
		}
	}
#endif

	if(p_thing->State==STATE_JUMPING)
		da=12;

	if(p_thing->SubState==SUB_STATE_RUNNING_HIT_WALL)
		return(1);

	if(p_thing->Genus.Person->Action == ACTION_SIDE_STEP)
	{
		return 1;
	}

#ifndef PSX
#ifndef TARGET_DC

	if(mouse_input)
	{
		da=(-MouseDX*TICK_RATIO)>>TICK_SHIFT;
		if(p_thing->SubState==SUB_STATE_CRAWLING||p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP)
		{
			if(da>32)
				da=32;
			else
				if(da<-32)
					da=-32;
		}
		if(p_thing->State==STATE_JUMPING)
		{
			if(da>12)
				da=12;
			else
				if(da<-12)
					da=-12;
		}

		p_thing->Draw.Tweened->Angle	=	(p_thing->Draw.Tweened->Angle+da)&2047;
	}
	else
#endif
#endif
	{
		if(input&INPUT_MASK_LEFT)
		{
			if(p_thing->SubState==SUB_STATE_SIDLE)
			{
				set_person_step_right(p_thing);
				return 1;

			}
			else
			if(p_thing->State!=STATE_SEARCH)
			{
				if(p_thing->SubState==SUB_STATE_RUNNING)
				{
#ifdef TARGET_DC
					SLONG max_da = 14+((p_thing->Velocity-9)>>1);
#else
					SLONG max_da = 14+(abs(p_thing->Draw.Tweened->Roll)/3);
#endif
					if ( da > max_da )
					{
						da = max_da;
					}

/*

					if(p_thing->Velocity>8)
					{


						da/=(p_thing->Velocity)>>4;
					}
*/

				}
				da=(da*TICK_RATIO)>>TICK_SHIFT;

				if(p_thing->SubState==SUB_STATE_CRAWLING||p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP)
					da>>=1;


//						if(p_thing->Genus.Person->pcom_ai_counter<TURN_TIMER)
					p_thing->Draw.Tweened->Angle	=	(p_thing->Draw.Tweened->Angle+da)&2047;
				if(p_thing->Velocity>10 && p_thing->SubState==SUB_STATE_RUNNING)
				{
#ifdef TARGET_DC
					// Nasty hack to keep Darci leaning the same amount as she's turning, otherwise it looks awful.
					p_thing->Draw.Tweened->Roll	=	-(((p_thing->Velocity-9)*da)>>3);
					p_thing->Draw.Tweened->DRoll	=	0;
#else
					p_thing->Draw.Tweened->DRoll	=	(p_thing->Velocity-9)>>1;
#endif
				}

				if(p_thing->State==STATE_IDLE&&!is_person_crouching(p_thing))
				{
					p_thing->Draw.Tweened->DRoll	=	-1;
					if(p_thing->Genus.Person->AnimType!=ANIM_TYPE_ROPER)
					if(p_thing->Draw.Tweened->CurrentAnim	!=	ANIM_ROTATE_L)
					{
						set_anim(p_thing,ANIM_ROTATE_L);
						p_thing->Genus.Person->pcom_ai_counter=0;

//								lock_to_compass(p_thing);
					}
					else
					{
						p_thing->Genus.Person->pcom_ai_counter+=da;
						if(p_thing->Genus.Person->pcom_ai_counter>TURN_TIMER)
						{
							p_thing->Draw.Tweened->DRoll	=	0; //tell fn_person_idle to finish off spin anim
						}

					}
				}

			}

		}
		else
		if(input&INPUT_MASK_RIGHT)
		{
//				if(p_thing->Genus.Person->Action == ACTION_SIDE_STEP)
			if(p_thing->SubState==SUB_STATE_SIDLE)
			{
				set_person_step_left(p_thing);
				return 1;

			}
			else
			if(p_thing->State!=STATE_SEARCH)
			{
				if(p_thing->SubState==SUB_STATE_RUNNING)
				{
#ifdef TARGET_DC
					SLONG max_da = 14+((p_thing->Velocity-9)>>1);
#else
					SLONG max_da = 14+(abs(p_thing->Draw.Tweened->Roll)/3);
#endif
					if ( da > max_da )
					{
						da = max_da;
					}
				}
/*
					if(p_thing->Velocity>8)
					{


						da/=(p_thing->Velocity)>>4;
					}
*/

				da=(da*TICK_RATIO)>>TICK_SHIFT;

				if(p_thing->SubState==SUB_STATE_CRAWLING||p_thing->SubState==SUB_STATE_RUNNING_SKID_STOP)
					da>>=1;


//						if(p_thing->Genus.Person->pcom_ai_counter<TURN_TIMER)
					p_thing->Draw.Tweened->Angle	=	(p_thing->Draw.Tweened->Angle-da)&2047;

				//if(p_thing->Velocity>10)
				if(p_thing->Velocity>10 && p_thing->SubState==SUB_STATE_RUNNING)
				{
#ifdef TARGET_DC
					// Nasty hack to keep Darci leaning the same amount as she's turning, otherwise it looks awful.
					p_thing->Draw.Tweened->Roll	=	(((p_thing->Velocity-9)*da)>>3);
					p_thing->Draw.Tweened->DRoll	=	0;
#else
					p_thing->Draw.Tweened->DRoll	=	-((p_thing->Velocity-9)>>1);
#endif
				}
//						if(p_thing->State==STATE_IDLE)
				if(p_thing->State==STATE_IDLE&&!is_person_crouching(p_thing))
				{
					p_thing->Draw.Tweened->DRoll	=	1;
					if(p_thing->Genus.Person->AnimType!=ANIM_TYPE_ROPER)
					if(p_thing->Draw.Tweened->CurrentAnim	!=	ANIM_ROTATE_R)
					{
//								lock_to_compass(p_thing);
						set_anim(p_thing,ANIM_ROTATE_R);
						p_thing->Genus.Person->pcom_ai_counter=0;
					}
					else
					{
						p_thing->Genus.Person->pcom_ai_counter+=da;
						if(p_thing->Genus.Person->pcom_ai_counter>TURN_TIMER)
						{
							p_thing->Draw.Tweened->DRoll	=	0; //tell fn_person_idle to finish off spin anim
						}

					}
				}
			}
		}
		else
		{
			p_thing->Genus.Person->pcom_ai_counter=0;
		}
	}
	return(0);

}

#endif

void	player_apply_move(Thing *p_thing,ULONG input)
{
//	SLONG	da=94;
//	LogText(" player move  input %x \n",input);

	switch(p_thing->State)
	{
		case	STATE_CLIMBING:
			switch(p_thing->SubState)
			{
				case	SUB_STATE_STOPPING:
//				case	SUB_STATE_CLIMB_AROUND_WALL:
void	locked_anim_change(Thing *p_person,UWORD locked_object,UWORD anim,SLONG	dangle=0);
#ifdef	UNUSED
					if(input&INPUT_MASK_LEFT)
					{
						p_thing->SubState=SUB_STATE_CLIMB_LEFT_WALL;
						locked_anim_change(p_thing,0,ANIM_CLIMB_LEFT_FENCE);
						return;

					}
					else
					if(input&INPUT_MASK_RIGHT)
					{
						p_thing->SubState=SUB_STATE_CLIMB_RIGHT_WALL;
						locked_anim_change(p_thing,0,ANIM_CLIMB_RIGHT_FENCE);
						return;
					}
#endif
					break;
			}
			break;

		case STATE_CARRY:
			if(p_thing->SubState!=SUB_STATE_STAND_CARRY)
				return;

		case STATE_IDLE:

			/*

			if(input&INPUT_MASK_ACTION)
			{
				if(input&INPUT_MASK_LEFT)
					set_person_flip(p_thing,0);
				else
				if(input&INPUT_MASK_RIGHT)
					set_person_flip(p_thing,1);
			}

			*/

		case STATE_JUMPING:
		case STATE_MOVEING:
		case STATE_GUN:
		case STATE_GRAPPLING:

			if(analogue)
			{
				if(player_turn_left_right_analogue(p_thing,input))
					return;
			}
			else
			{
				if(player_turn_left_right(p_thing,input))
					return;
			}

			//
			// Turning left / right
			//



			break;

//		case STATE_CLIMBING:

			//
			// Climbing left/right
			//

			break;
	}

	switch(p_thing->State)
	{
		case STATE_JUMPING:
			return;
	}
/*
	if(input&INPUT_MASK_STEP_LEFT)
	{
		switch(p_thing->State)
		{
			case	STATE_IDLE:
				set_person_step_left(p_thing);
				return;
				break;
		}

	}
	else
	if(input&INPUT_MASK_STEP_RIGHT)
	{
		switch(p_thing->State)
		{
			case	STATE_IDLE:
				set_person_step_right(p_thing);
				return;
				break;
		}

	}
*/
	if(analogue)
	{
		process_analogue_movement(p_thing,input);
	}
	else
	{
		if(input&INPUT_MASK_MOVE)
		{
	//		LogText(" have movement input  state %d \n",p_thing->State);
			switch(p_thing->State)
			{

				case	STATE_HUG_WALL:
					if(p_thing->SubState==SUB_STATE_HUG_WALL_TURN)
						return;
					p_thing->Velocity=40;
//					p_thing->Draw.Tweened->Angle+=1024;
//					p_thing->Draw.Tweened->Angle&=2047;
					person_normal_move(p_thing);
//					p_thing->Draw.Tweened->Angle+=1024;
//					p_thing->Draw.Tweened->Angle&=2047;

					if(p_thing->SubState!=SUB_STATE_HUG_WALL_TURN)
						set_person_running(p_thing);
					break;
				case	STATE_CARRY:
					if(p_thing->SubState!=SUB_STATE_STAND_CARRY)
						return;
				case	STATE_IDLE:
				case	STATE_HIT_RECOIL:

				case	STATE_GUN:
					switch(p_thing->SubState)
					{
						case	0:
						case	SUB_STATE_SIDLE:
						default:
							set_person_running(p_thing);
							break;
						case	SUB_STATE_IDLE_CROUTCHING:
							set_person_crawling(p_thing);
							break;
						case	SUB_STATE_IDLE_CROUTCH:
							if(p_thing->Draw.Tweened->FrameIndex>3)
								return;
							else
								set_person_running(p_thing);
							break;
					}
					break;

	/*
				case	STATE_GUN:

					if (p_thing->Genus.Person->Mode == PERSON_MODE_FIGHT)
					{
						set_person_fight_step(p_thing,0);
					}
					else
					{
						if(input_mode==INPUT_JOYPAD)
						{
							if(input&(INPUT_MASK_SPRINT|INPUT_MASK_YOMP))
							{
								if((input&(INPUT_MASK_SPRINT))&&p_thing->Genus.Person->Stamina>3)
									p_thing->Genus.Person->Mode=PERSON_MODE_SPRINT;
								else
									p_thing->Genus.Person->Mode=PERSON_MODE_RUN;
							}
							else
							{
								p_thing->Genus.Person->Mode=PERSON_MODE_WALK;
							}
						}

						set_person_running(p_thing);
					}

	*/

					break;

				case	STATE_MOVEING:
					switch(p_thing->SubState)
					{
						case	SUB_STATE_RUNNING:
	/*
							{
								if(input_mode==INPUT_JOYPAD)
								{
									if(input&(INPUT_MASK_SPRINT|INPUT_MASK_YOMP))
									{
										if((input&(INPUT_MASK_SPRINT))&&p_thing->Genus.Person->Stamina>3)
											p_thing->Genus.Person->Mode=PERSON_MODE_SPRINT;
										else
											p_thing->Genus.Person->Mode=PERSON_MODE_RUN;

									}
									else
									{
										p_thing->Genus.Person->Mode=PERSON_MODE_WALK;
										set_person_running(p_thing);
									}
								}

							}
	*/
							break;
						case	SUB_STATE_STOP_CRAWL:
		     				p_thing->SubState=SUB_STATE_CRAWLING;
							break;
						case	SUB_STATE_CRAWLING:
							break;
						case	SUB_STATE_WALKING_BACKWARDS:
								player_stop_move(p_thing,input);

							break;
						case	SUB_STATE_WALKING:
							set_person_running(p_thing);
	/*
							{
								if(input_mode==INPUT_JOYPAD)
								{
									if(input&(INPUT_MASK_SPRINT|INPUT_MASK_YOMP))
									{
										if((input&(INPUT_MASK_SPRINT))&&p_thing->Genus.Person->Stamina>3)
											p_thing->Genus.Person->Mode=PERSON_MODE_SPRINT;
										else
											p_thing->Genus.Person->Mode=PERSON_MODE_RUN;
										set_person_running(p_thing);
									}
									else
									{
									//	p_thing->Genus.Person->Mode=PERSON_MODE_WALK;
									}
								}

							}
	*/
							break;
						case	SUB_STATE_STEP_LEFT:
						case	SUB_STATE_STEP_RIGHT:
							set_person_running(p_thing);
							break;

					}
					//need to continue moveing
					break;
				case	STATE_CLIMB_LADDER:
					switch(p_thing->SubState)
					{
						case	SUB_STATE_MOUNT_LADDER:
							break;
						case	SUB_STATE_STOPPING:
						case	SUB_STATE_ON_LADDER:
	//					case	SUB_STATE_CLIMB_UP_LADDER:
	//					case	SUB_STATE_CLIMB_DOWN_LADDER:
							p_thing->SubState=SUB_STATE_CLIMB_UP_LADDER;
	//						p_thing->Genus.Person->Timer1=12;
							break;
					}
					break;
				case	STATE_CLIMBING:
					switch(p_thing->SubState)
					{
						case	SUB_STATE_STOPPING:
						case	SUB_STATE_CLIMB_AROUND_WALL:
	//					case	SUB_STATE_CLIMB_UP_WALL:
	//					case	SUB_STATE_CLIMB_DOWN_WALL:
	//						p_thing->Genus.Person->Timer1=7;
							p_thing->SubState=SUB_STATE_CLIMB_UP_WALL;
							break;
					}
					break;
				case	STATE_DANGLING:
					switch(p_thing->SubState)
					{
						case	SUB_STATE_STOPPING:
						case	SUB_STATE_DANGLING_CABLE_FORWARD:
						case	SUB_STATE_DANGLING_CABLE_BACKWARD:
						case	SUB_STATE_DANGLING_CABLE:
							if(p_thing->Flags&FLAG_PERSON_ON_CABLE)
								p_thing->SubState=SUB_STATE_DANGLING_CABLE_FORWARD; //cable errors
							else
 								ASSERT(0);
							break;
					}
					break;

				case	STATE_GRAPPLING:

					//
					// We can't move while picking up or releasing the grappling hook.
					//

					if (p_thing->SubState == SUB_STATE_GRAPPLING_WINDUP)
					{
						set_person_running(p_thing);
					}

					break;

				case	STATE_CANNING:
					break;
			}
		}
		else
		{
			//
			// not pressing move
			//

			switch(p_thing->State)
			{

				case	STATE_MOVEING:
					switch(p_thing->SubState)
					{
						case	SUB_STATE_CRAWLING:
							set_person_idle_croutch(p_thing);
	//		     			p_thing->SubState=SUB_STATE_STOP_CRAWL;
	//						player_stop_crawl(p_thing,input);
							break;
						case	SUB_STATE_WALKING:
							if(!(input&INPUT_MASK_FORWARDS))
								player_stop_move(p_thing,input);
							break;


						case	SUB_STATE_RUNNING:
							if (input & INPUT_MASK_FORWARDS)
							{
								set_person_walking(p_thing);
							}
							else
							{
								player_stop_move(p_thing, input);
							}

							break;



						case	SUB_STATE_SIDLE:
							player_stop_move(p_thing,input);
							break;
					}
					break;
			}


		}

		//
		// press forward and not move does a walk
		//
		if((input&INPUT_MASK_FORWARDS) && !(input&INPUT_MASK_MOVE))
		{
			switch(p_thing->State)
			{

				case	STATE_IDLE:
				case	STATE_GUN:
						set_person_walking(p_thing);
						break;
			}
		}


		if(input&INPUT_MASK_BACKWARDS)
		{
	//		LogText(" have movement input back state %d \n",p_thing->State);
			switch(p_thing->State)
			{

				case	STATE_HUG_WALL:
					if(p_thing->SubState==SUB_STATE_HUG_WALL_TURN)
						break;
				case	STATE_IDLE:
				case	STATE_GUN:
	//				set_person_hop_back(p_thing);
						set_person_walk_backwards(p_thing);
					//need to start moveing then
					break;
				case	STATE_MOVEING:
					//need to continue moveing
					break;
				case	STATE_CLIMB_LADDER:
					switch(p_thing->SubState)
					{
						case	SUB_STATE_MOUNT_LADDER:
							break;
						case	SUB_STATE_STOPPING:
						case	SUB_STATE_ON_LADDER:
	//					case	SUB_STATE_CLIMB_UP_LADDER:
	//					case	SUB_STATE_CLIMB_DOWN_LADDER:
	//						p_thing->Draw.Tweened->FrameIndex=0;
							p_thing->SubState=SUB_STATE_CLIMB_DOWN_LADDER;
							break;
					}
					break;
				case	STATE_CLIMBING:
					switch(p_thing->SubState)
					{
						case	SUB_STATE_STOPPING:
	//					case	SUB_STATE_CLIMB_UP_WALL:
						case	SUB_STATE_CLIMB_AROUND_WALL:
	//					case	SUB_STATE_CLIMB_DOWN_WALL:
	//						p_thing->Genus.Person->Timer1=7;
							p_thing->SubState=SUB_STATE_CLIMB_DOWN_WALL;
							break;
					}
				case	STATE_DANGLING:
					switch(p_thing->SubState)
					{
						case	SUB_STATE_STOPPING:
						case	SUB_STATE_DANGLING_CABLE_FORWARD:
						case	SUB_STATE_DANGLING_CABLE_BACKWARD:
						case	SUB_STATE_DANGLING_CABLE:
							if(p_thing->Flags&FLAG_PERSON_ON_CABLE)
								p_thing->SubState=SUB_STATE_DANGLING_CABLE_FORWARD; //cable errors
							else
								ASSERT(0);
							//p_thing->SubState=SUB_STATE_DANGLING_CABLE_BACKWARD;
							break;
					}
					break;


				case	STATE_GRAPPLING:

					//
					// We can't move while picking up or releasing the grappling hook.
					//

					if (p_thing->SubState == SUB_STATE_GRAPPLING_WINDUP)
					{
						set_person_walk_backwards(p_thing);
					}

					break;

				case	STATE_CANNING:
					break;

			}
		}
	//	else   //this whole thing was a load of else's
	//		if(p_thing->State!=STATE_IDLE)
	//			player_stop_move(p_thing,input);
	}

}


extern	void	set_person_alive_alive_o(Thing *p_person);


/*

//
// Changes a person's mode.
//

void person_change_mode(Thing *p_person)
{
	if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
	{
		//
		// Start running away!
		//

		p_person->Genus.Person->Mode = PERSON_MODE_RUN;
	}
	else
	{
		if (p_person->Genus.Person->UnderAttack)
		{
			//
			// Enter fight mode.
			//

			p_person->Genus.Person->Mode = PERSON_MODE_FIGHT;
		}
		else
		{

			p_person->Genus.Person->Mode += 1;

			//
			// Don't select fight mode here.
			//

			if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
			{
				p_person->Genus.Person->Mode += 1;
			}


			if (p_person->Genus.Person->Mode >= PERSON_MODE_NUMBER)
			{
				p_person->Genus.Person->Mode = 0;
			}

//			if (p_person->State == STATE_MOVEING)
//			{
//				set_person_running(p_person);
//			}
		}
	}

	//
	// Tell the player what mode he is in.
	//

	CONSOLE_text_at(
		250, 50,
		3000,
		"%s mode",
		PERSON_mode_name[p_person->Genus.Person->Mode]);
}

*/


//
// Possible Rules for entering fight mode for player
//
/*

  1. You try and hitsomeone from a standing idle position

  or

  1. you are hit by someone

  or

  1. Someone is attempting to hit you
  2. you are standing still

*/
void person_enter_fight_mode(Thing *p_person)
{
	p_person->Genus.Person->Mode = PERSON_MODE_FIGHT;
}

//
// This Function interprets the user action, with the characters status
// and responds accordingly, ie punch when holding a gun is shoot
//

//persumably this will be aplied to a darci or a roper
ULONG	apply_button_input(struct Thing *p_player,struct Thing *p_person,ULONG input)
{
	ULONG	input_used=0;
	ULONG	processed=0;


	/*

	if (p_person->Genus.Person->UnderAttack && p_person->Genus.Person->Mode != PERSON_MODE_FIGHT)
	{
		//
		// Lets the player know that pressing 'MODE' with put him into
		// fight mode.
		//
	}

	*/
#ifndef PSX
#ifndef TARGET_DC

	if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
	{
		//
		// Tell the player he is in fight mode.
		//

		CONSOLE_text_at(400, 400, 50, "Fight mode");
	}
#endif
#endif
/*
	if(input&INPUT_MASK_MODE_CHANGE)
	{
		person_change_mode(p_person);

		processed |= INPUT_MASK_MODE_CHANGE;
	}
*/

	if (input & INPUT_MASK_ACTION)
	{
		//
		// Do an action!
		//

		//
		// Mark you don't understand, you should go through the action tree
		// idle should have an action button reference to do_an_action
		// so should lots of things, but should everything, if everything should then this is
		// the right way to do it.
		//

		//
		// You wrote this code, Mike. I only added to it.
		//

		processed|=do_an_action(p_person, input);
		input&=~processed;

	}
	else
	{
		//
		// no action
		//

		//
		// only sprint while you hold the action button
		//
//		if(p_person->Genus.Person->Mode==PERSON_MODE_SPRINT)

		// skanky roper always-run bodge:

/*
		if (p_person->Genus.Person->AnimType==ANIM_TYPE_ROPER)
		{
			if (p_person->Genus.Person->Mode==PERSON_MODE_RUN)
				p_person->Genus.Person->Mode=PERSON_MODE_SPRINT;
		}
		else
*/
		{
			if(p_person->Genus.Person->Mode==PERSON_MODE_SPRINT)
				p_person->Genus.Person->Mode=PERSON_MODE_RUN;
		}
/*
		if(p_person->Genus.Person->InsideIndex)
		{
			//
			// your inside so walk unless actioning
			//
			if(p_person->Genus.Person->Mode==PERSON_MODE_RUN)
				p_person->Genus.Person->Mode=PERSON_MODE_WALK;
		}
*/
		if(p_person->SubState==SUB_STATE_IDLE_CROUTCHING)
		{
			MSG_add("no action so stand up");
//			set_person_idle(p_person);
			set_person_idle_uncroutch(p_person); //locked to left foot, like the idle to croutch anim

		}
	}


	if(p_person->State==STATE_CARRY)
	{
		if (p_person->SubState == SUB_STATE_PICKUP_CARRY ||
			p_person->SubState == SUB_STATE_DROP_CARRY)
		{
			//
			// Don't do anything while picking somebody up or down...
			//

			return processed | input_used;
		}
	}

	//record last five inputs and times, so we can
	if(input && (p_person->State != STATE_CARRY)) //&INPUT_ACTION_MASK)
	{
		SLONG	new_action;

 		new_action=find_best_action_from_tree(p_person->Genus.Person->Action,input,&input_used);
//		if(new_action)
//			MSG_add(" new action %d old action %d input\n",new_action,p_person->Genus.Person->Action);

		switch(new_action)
		{
			case	ACTION_KICK_FLAG:
				p_person->Genus.Person->Flags|=FLAG_PERSON_REQUEST_KICK;
				break;
			case	ACTION_PUNCH_FLAG:
				p_person->Genus.Person->Flags|=FLAG_PERSON_REQUEST_PUNCH;
				break;
			case	ACTION_BLOCK_FLAG:
				p_person->Genus.Person->Flags|=FLAG_PERSON_REQUEST_BLOCK;
				break;
			case	ACTION_JUMP_FLAG:
				p_person->Genus.Person->Flags|=FLAG_PERSON_REQUEST_JUMP;
				break;
			case	ACTION_HUG_LEFT:
				if(p_person->SubState!=SUB_STATE_HUG_WALL_STEP_RIGHT && p_person->SubState!=SUB_STATE_HUG_WALL_LOOK_L && p_person->SubState!=SUB_STATE_HUG_WALL_TURN && p_person->SubState!=SUB_STATE_HUG_WALL_LEAP_OUT)
					set_person_hug_wall_dir(p_person,0);
				input_used=0;
				break;
			case	ACTION_HUG_RIGHT:
				if(p_person->SubState!=SUB_STATE_HUG_WALL_STEP_LEFT && p_person->SubState!=SUB_STATE_HUG_WALL_LOOK_R && p_person->SubState!=SUB_STATE_HUG_WALL_TURN && p_person->SubState!=SUB_STATE_HUG_WALL_LEAP_OUT)
					set_person_hug_wall_dir(p_person,1);
				input_used=0;
				break;
			case	ACTION_UNSIT:
void set_person_unsit(Thing *p_person);
				set_person_unsit(p_person);
				break;


			case	ACTION_FLIP_LEFT:
				if(p_person->Genus.Person->Action==ACTION_STANDING_JUMP)
				{
					if(p_person->Draw.Tweened->FrameIndex<3)
						set_person_flip(p_person,0);
				}
				else
					set_person_flip(p_person,0);
				input_used=0;
				break;
			case	ACTION_FLIP_RIGHT:
//				if(p_person->Action==ACTION_STANDING_JUMP)
				if(p_person->Genus.Person->Action==ACTION_STANDING_JUMP)
				{
					if(p_person->Draw.Tweened->FrameIndex<3)
						set_person_flip(p_person,1);
				}
				else
					set_person_flip(p_person,1);
				input_used=0;
				break;
			case	ACTION_SHOOT:
				if (p_person->Genus.Person->Action == ACTION_SHOOT)
				{
					/*

					//
					// Only the pistol animation can be interrupted so you
					// can shoot again.
					//

					if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
					{
						set_player_shoot(p_person,0);
						processed|=input_used; //needs a clear click
					}

					*/
				}
				else
				{
//					if(person_has_gun_out(p_person))
					{
						set_player_shoot(p_person,0);

						processed|=input_used; //needs a clear click
					}
				}
				break;
			case	ACTION_GUN_AWAY:
				set_person_gun_away(p_person);
				processed|=input_used; //nedds a clear click
				break;
			case	ACTION_RESPAWN:
//				set_person_alive_alive_o(p_person);
				processed|=input_used;
				break;
			case	ACTION_DROP_DOWN:
				if(p_person->Genus.Person->Action==ACTION_DEATH_SLIDE)
				{
					MFX_stop(THING_NUMBER(p_person), S_ZIPWIRE);
					set_person_drop_down(p_person,PERSON_DROP_DOWN_OFF_FACE);
					p_person->Velocity = 0;
				}
				else
				{
					set_person_drop_down(p_person,PERSON_DROP_DOWN_OFF_FACE);
				}
				break;
		}

		if(!(p_person->Genus.Person->Flags & (FLAG_PERSON_NON_INT_M|FLAG_PERSON_NON_INT_C)))
		{
			switch(new_action)
			{

				case	ACTION_SKID:
						if(p_person->SubState!=SUB_STATE_RUNNING_SKID_STOP)
						if(p_person->SubState!=SUB_STATE_RUNNING_JUMP_LAND_FAST)
						if(p_person->Velocity>25)
						{
							set_anim(p_person,ANIM_SLIDER_START);
							p_person->SubState=SUB_STATE_RUNNING_SKID_STOP;
							if (!(p_person->Genus.Person->Flags&FLAG_PERSON_SLIDING))
							{
#ifndef PSX
								MFX_play_thing(THING_NUMBER(p_person),S_SLIDE_START,MFX_LOOPED,p_person);
#else
								MFX_play_thing(THING_NUMBER(p_person),S_SLIDE_START,MFX_LOOPED|MFX_FLAG_SLIDER,p_person);
#endif
								p_person->Genus.Person->Flags |= FLAG_PERSON_SLIDING;
							}
						}


					break;
				case	ACTION_STANDING_JUMP:

					if (should_i_jump(p_person))
					{
						if(input&INPUT_MASK_FORWARDS)
							set_person_standing_jump_forwards(p_person);
						else
						if((input&INPUT_MASK_BACKWARDS) && should_person_backflip(p_person))
							set_person_standing_jump_backwards(p_person);
						else
						if(input&INPUT_MASK_LEFT)
							set_person_flip(p_person,0);
						else
						if(input&INPUT_MASK_RIGHT)
							set_person_flip(p_person,1);
						else
							set_person_standing_jump(p_person);
					}
					else
					{
						//
						// Only flipping allowed?
						//

						if(input&INPUT_MASK_LEFT)
							set_person_flip(p_person,0);
						else
						if(input&INPUT_MASK_RIGHT)
							set_person_flip(p_person,1);
					}

					break;
				case	ACTION_RUNNING_JUMP:
	//				LogText(" start running jump \n");
					if (p_person->SubState==SUB_STATE_RUNNING_SKID_STOP)
					{
						p_person->Genus.Person->Flags&=~FLAG_PERSON_SLIDING;
						MFX_stop(THING_NUMBER(p_person),S_SLIDE_START);
					}
					if (should_i_jump(p_person))
					{
						set_person_running_jump(p_person);
					}
					break;
				case	ACTION_TRAVERSE_LEFT:
					set_person_traverse(p_person,0);
					break;
				case	ACTION_TRAVERSE_RIGHT:
					set_person_traverse(p_person,1);
					break;
				case	ACTION_PULL_UP:
	//				LogText(" set person pulling up \n");
					set_person_pulling_up(p_person);
					input_used=0;
					break;
				case	ACTION_FIGHT_KICK:
	//				MSG_add(" ACTION TREE KICK");
	//				set_person_kick(p_person);
					if(input&INPUT_MASK_BACKWARDS)
					{
						if (p_person->State != STATE_JUMPING)
						{
							//
							// Kick Backwards
							//
							turn_to_target(
								p_person,
								FIND_DIR_BACK|FIND_DIR_DONT_TURN);

							p_person->Genus.Person->Timer1=6;


							set_person_kick_dir(p_person,2);

							person_enter_fight_mode(p_person);

							processed|=INPUT_MASK_KICK|INPUT_MASK_BACKWARDS;
						}
					}
					else
					{
						if (p_person->State != STATE_JUMPING)
						{
							if(turn_to_target_and_kick(p_person))
							{
								//
								// Enter fight mode.
								//

								person_enter_fight_mode(p_person);
							}
						}
					}

					//play_quick_wave(
					//	p_person,
					//	S_HIYAR,
					//	WAVE_PLAY_INTERUPT);


					break;
				case	ACTION_FIGHT_PUNCH:
	//				MSG_add(" ACTION TREE PUNCH");
	//				set_player_punch(p_person);

					if(person_has_gun_out(p_person))
					{
//						set_person_idle(p_person);
						set_person_shoot(p_person,0);
					}
					else
					if(!player_activate_in_hand(p_person))
					if(turn_to_target_and_punch(p_person))
//					if (p_person->Genus.Person->UnderAttack)
					{
						//
						// Enter fight mode.
						//

						person_enter_fight_mode(p_person);
					}

					//play_quick_wave(
					//	p_person,
					//	S_HIYAR,
					//	WAVE_PLAY_INTERUPT);


					break;
				case	ACTION_DRAW_SPECIAL:
					set_person_draw_special(p_person);
					processed|=input_used; //needs a clear click
					break;
			}
		}

//		LogText(" apply button input %d \n",input);

		// for the action currently being performed
		// does the action have any meaning

		// if it does choose the best meaning and initiate it
		// else ignore it.

	}
#ifndef PSX
	if((input&INPUT_MOVEMENT_MASK) ||(mouse_input &&MouseDX))
#else
	if((input&INPUT_MOVEMENT_MASK))
#endif
	{
//		LogText(" apply button input %d  state %d\n",input,p_person->State);
		if(!(p_person->Genus.Person->Flags & FLAG_PERSON_NON_INT_M))
		{
			switch(p_person->State)
			{
				case	STATE_HIT_RECOIL:
				case	STATE_CARRY:
				case	STATE_HUG_WALL:
				case	STATE_IDLE:
				case	STATE_MOVEING:
				case	STATE_GUN:
				case	STATE_CLIMB_LADDER:
				case	STATE_CLIMBING:
				case	STATE_DANGLING:
				case	STATE_JUMPING:
					player_interface_move(p_person,input);
					break;

				case	STATE_GRAPPLING:

					if (p_person->SubState == SUB_STATE_GRAPPLING_WINDUP)
					{
						//
						// Only move while spinning the hook, not when throwing
						// or picking it up.
						//

						player_interface_move(p_person,input);
					}

					break;
				default:
					break;
			}
		}
		else
		{
			if (p_person->Genus.Person->Action == ACTION_SIT_BENCH)
			{
				if (input & (INPUT_MASK_FORWARDS|INPUT_MASK_MOVE))
				{
					input_used |= INPUT_MASK_FORWARDS|INPUT_MASK_MOVE;

					set_person_idle(p_person);
				}
			}
		}

		//movement requested

		// person may be in middle of kick, or falling, or jumping or standing still
		//
	}
	else
	{
		if(!(p_person->Genus.Person->Flags & FLAG_PERSON_NON_INT_M))
		switch(p_person->State)
		{
			case	STATE_IDLE:
				player_turn_left_right(p_person, 0);
				break;
			case	STATE_MOVEING:
				switch(p_person->SubState)
				{
					case	SUB_STATE_CRAWLING:
						set_person_idle_croutch(p_person);
						break;
//					case	SUB_STATE_HOP_BACK:
//						break;
					default:
						player_stop_move(p_person,input);
				}

				break;
/*
			case	STATE_CLIMB_LADDER:
				switch(p_person->SubState)
				{
					case	SUB_STATE_MOUNT_LADDER:
						break;
					case	SUB_STATE_STOPPING:
					case	SUB_STATE_ON_LADDER:
					case	SUB_STATE_CLIMB_UP_LADDER:
					case	SUB_STATE_CLIMB_DOWN_LADDER:
						player_stop_move(p_person,input);
						break;
				}
*/
/*
				case	STATE_CLIMBING:
				switch(p_person->SubState)
				{
					case	SUB_STATE_STOPPING:
					case	SUB_STATE_CLIMB_UP_WALL:
					case	SUB_STATE_CLIMB_AROUND_WALL:
					case	SUB_STATE_CLIMB_DOWN_WALL:
						player_stop_move(p_person,input);
						break;
				}
				break;
*/
			case	STATE_DANGLING:
				switch(p_person->SubState)
				{
					case	SUB_STATE_STOPPING:
					case	SUB_STATE_DANGLING_CABLE_FORWARD:
					case	SUB_STATE_DANGLING_CABLE_BACKWARD:
					case	SUB_STATE_DANGLING_CABLE:
						player_stop_move(p_person,input);
						break;
				}
				break;
			default:
				p_player->Genus.Player->Input=input;
				break;
		}
	}
	return(processed|input_used);
}

//
// Need a double click to get out of fight mode.
//

#define MUST_DOUBLE_CLICK_FORWARDS_TO_GET_OUT_OF_FIGHT_MODE


//
// User input while fighting. //_combat_
//

//bits   3     2     1     0
//		 r	   l	 b	   f
UBYTE	input_to_angle[]=
{
	0,  //nowt
	0,  //f
	128,  //b
	 0,  // f & b impossible
	 192,  // left
	 192+32,  // for - left
	 192-32,  // back -left
	 0,  //
	 64,  // right
	 32,  // for-right
	 96,  // for-back
	 0,  //
	 0,  //
	 0,  //

};


/*
	Press a direction to move in that direction,  (+ punch or kick, to hit in that direction then return to normal direction)


  Double tap direction to turn in that direction (& choose a target if available)

  (plus punch or kick to hit in that direction and stay facing that direction)

*/

extern	UWORD	count_gang(Thing *p_target);


ULONG apply_button_input_fight(Thing *p_player, Thing *p_person, ULONG input)
{
	Player *pl = p_player->Genus.Player;

	if(p_person->SubState==SUB_STATE_IDLE_CROUTCH_ARREST)
		return(0);

	if(pl->Pressed & INPUT_MASK_PUNCH)
	if(person_has_gun_out(p_person))
	{
		set_player_shoot(p_person,0);
		//PANEL_new_text(NULL,4000,"ACTION combat shoot gun");
		return(INPUT_MASK_PUNCH);
	}

#ifdef	PSX
//		if (p_player->Genus.Player->Pressed & INPUT_MASK_MOVE &&
//		   !(p_player->Genus.Player->Pressed & INPUT_MASK_FORWARDS))
		if(!is_person_ko(p_person))
		{
			if(p_player->Genus.Player->Pressed & INPUT_MASK_STEP_RIGHT)
			{

				person_pick_best_target(p_person,1);
				GAME_FLAGS|=GF_SIDE_ON_COMBAT;
				pl->DoneSomething = TRUE;
				return INPUT_MASK_ACTION;
			}
			else
			if(p_player->Genus.Player->Pressed & INPUT_MASK_STEP_LEFT)
			{

				person_pick_best_target(p_person,-1);
				GAME_FLAGS|=GF_SIDE_ON_COMBAT;
				pl->DoneSomething = TRUE;
				return INPUT_MASK_ACTION;
			}
		}
#endif
	//
	// move button gets us out of combat mode (for fleeing or something)
	//
	if(!analogue)
	{
		if (p_player->Genus.Player->Pressed & INPUT_MASK_MOVE &&
		   !(p_player->Genus.Player->Pressed & INPUT_MASK_FORWARDS))
		{
			if (p_person->Genus.Person->Flags & (FLAG_PERSON_HELPLESS|FLAG_PERSON_KO))
			{
				//
				// In no fit state to run away!
				//
			}
			else
			{
				p_person->Genus.Person->Mode = PERSON_MODE_RUN;
				set_person_running(p_person);
				//PANEL_new_text(NULL,4000,"ACTION combat run");
				return(0);
			}
		}
	}

 /*
	#ifdef MUST_DOUBLE_CLICK_FORWARDS_TO_GET_OUT_OF_FIGHT_MODE

	//
	// Forward tapping twice gets you out of fight mode.
	//

	if (p_player->Genus.Player->Pressed & INPUT_MASK_FORWARDS)
	{
		if (p_player->Genus.Player->DoubleClick[INPUT_FORWARDS] == 2)
		{
			p_player->Genus.Player->DoubleClick[INPUT_FORWARDS] = 0;

			p_person->Genus.Person->Mode = PERSON_MODE_RUN;
		}
	}

	#else

	//
	// Moving forwards goes out of fight mode.
	//

	if (input & INPUT_MASK_FORWARDS)
	{
		p_person->Genus.Person->Mode = PERSON_MODE_RUN;
	}

	#endif
	//
	// Moving forwards goes out of fight mode.
	//
	if (input & INPUT_MASK_FORWARDS)
	{
		set_person_fight_step(p_person,0);
	}
	if (input & INPUT_MASK_BACKWARDS)
	{
		set_person_fight_step(p_person,2);
	}
   */

	//
	// Double-click back does a hop back.
	//
/*
	if (p_player->Genus.Player->Pressed & INPUT_MASK_BACKWARDS)
	{
		if (p_player->Genus.Player->DoubleClick[INPUT_BACKWARDS] == 2)
		{
			p_player->Genus.Player->DoubleClick[INPUT_BACKWARDS] = 0;

			set_person_hop_back(p_person);
		}
	}
*/

	if(input) //&INPUT_ACTION_MASK)
	{
		SLONG	new_action;
		ULONG	input_used;

 		new_action=find_best_action_from_tree(p_person->Genus.Person->Action,input,&input_used);
		switch(new_action)
		{
			case	ACTION_RESPAWN:
//				set_person_alive_alive_o(p_person);
				break;

		}
	}


	if(p_person->SubState==SUB_STATE_STEP_FORWARD)
	{
		if (p_player->Genus.Player->Pressed & INPUT_MASK_JUMP)
		{
			switch(p_person->Draw.Tweened->CurrentAnim)
			{
				case	ANIM_FIGHT_STEP_E:
					set_person_flip(p_person,1);
				//PANEL_new_text(NULL,4000,"ACTION combat flip");
					return INPUT_MASK_JUMP;
					break;
				case	ANIM_FIGHT_STEP_W:
					set_person_flip(p_person,0);
				//PANEL_new_text(NULL,4000,"ACTION combat flip");
					return INPUT_MASK_JUMP;
					break;
			}
		}
	}


	extern	SLONG	get_combat_type_for_node(UBYTE current_node);


	if ((pl->Pressed & INPUT_MASK_PUNCH)==0&&(pl->Pressed & INPUT_MASK_KICK)==0)
	if (p_person->State == STATE_IDLE ||(p_person->State == STATE_HIT_RECOIL &&p_person->Draw.Tweened->FrameIndex>2) || ((p_person->SubState ==SUB_STATE_STEP_FORWARD || p_person->SubState == SUB_STATE_PUNCH || p_person->SubState == SUB_STATE_KICK )))
		// && get_combat_type_for_node(p_person->Genus.Person->CombatNode)==COMBAT_NONE))
	if(p_person->Draw.Tweened->CurrentAnim!=ANIM_KICK_NAD)
	if(p_person->Draw.Tweened->CurrentAnim!=ANIM_LEG_SWEEP)
	if(p_person->Draw.Tweened->CurrentAnim!=ANIM_KICK_LEFT)
	if(p_person->Draw.Tweened->CurrentAnim!=ANIM_KICK_RIGHT)
	if(p_person->Draw.Tweened->CurrentAnim!=ANIM_KICK_NS)
	if(p_person->Draw.Tweened->CurrentAnim!=ANIM_KICK_BEHIND)
	{
		if (input & INPUT_MASK_BACKWARDS)
		{

			set_person_fight_step(p_person,2);
				//PANEL_new_text(NULL,4000,"ACTION combat stepb");
			return(0);//INPUT_MASK_BACKWARDS);
//			flags|=2;
		}
//		if (p_player->Genus.Player->Pressed & INPUT_MASK_FORWARDS)
		if (input & INPUT_MASK_FORWARDS)
		{
			set_person_fight_step(p_person,0);
				//PANEL_new_text(NULL,4000,"ACTION combat stepf");
			return(0);
//			flags|=1;
		}
//		if (p_player->Genus.Player->Pressed & INPUT_MASK_RIGHT)
		if (input & INPUT_MASK_RIGHT)
		{
//			flags|=4;
			//dx=1;
			set_person_fight_step(p_person,1);
				//PANEL_new_text(NULL,4000,"ACTION combat stepr");
			return(0);
		}
//		if (p_player->Genus.Player->Pressed & INPUT_MASK_LEFT)
		if (input & INPUT_MASK_LEFT)
		{
//			flags|=8;
//			dx=-1;
				//PANEL_new_text(NULL,4000,"ACTION combat stepl");
			set_person_fight_step(p_person,3);
			return(0);
		}
	}
	if ((pl->Pressed & INPUT_MASK_PUNCH)==0&&(pl->Pressed & INPUT_MASK_KICK)==0)
	if (p_player->Genus.Player->Pressed & (INPUT_MASK_LEFT|INPUT_MASK_RIGHT))
 	if (p_person->State == STATE_FIGHTING)
	{
		if(p_person->SubState == SUB_STATE_KICK)
		{
 			if(p_person->Draw.Tweened->CurrentAnim==ANIM_KICK_NS)
			{
				if(p_person->Draw.Tweened->FrameIndex<2)
				{
					if (p_player->Genus.Player->Pressed & INPUT_MASK_LEFT)
					{
						//PANEL_new_text(NULL,4000,"ACTION flip");
						set_person_flip(p_person,0);
					}
					else
					if (p_player->Genus.Player->Pressed & INPUT_MASK_RIGHT)
					{
						//PANEL_new_text(NULL,4000,"ACTION flip");
						set_person_flip(p_person,1);
					}
				}

			}
		}

	}

	if ((pl->Pressed & INPUT_MASK_PUNCH)==0&&(pl->Pressed & INPUT_MASK_KICK)==0)
	if (p_person->State == STATE_IDLE)
	{
		SLONG	flags=0;
		if (p_player->Genus.Player->Pressed & INPUT_MASK_JUMP)
		{
			if (p_player->Genus.Player->Pressed & INPUT_MASK_LEFT)
			{
						//PANEL_new_text(NULL,4000,"ACTION flip");
				set_person_flip(p_person,0);
			}
			else
			if (p_player->Genus.Player->Pressed & INPUT_MASK_RIGHT)
			{
						//PANEL_new_text(NULL,4000,"ACTION flip");
				set_person_flip(p_person,1);
			}
			else
			{
						//PANEL_new_text(NULL,4000,"ACTION jump kick");
				set_person_fight_anim(p_person,ANIM_KICK_NS);
			}

			return(0);
		}

		//if (p_player->Genus.Player->Pressed & INPUT_MASK_BACKWARDS)
		if(flags)
		{
			SLONG	angle=input_to_angle[flags];
			angle<<=3;
//			angle+=(FC_cam[p_person->Genus.Person->PlayerID-1].yaw>>8);
			angle+= get_camera_angle(); //FC_cam[0].yaw>>8;

			angle&=2047;

			angle-=p_person->Draw.Tweened->Angle;
			if(angle>1024)
				angle-=2048;
			if(angle<-1024)
				angle+=2048;
			angle>>=1;

			if(abs(angle)<64)
			{
void	set_person_fight_step_forward(Thing *p_person);
				set_person_fight_step_forward(p_person);
			}

			p_person->Draw.Tweened->Angle+=angle;
			p_person->Draw.Tweened->Angle&=2047;

			//
			// Find a target and turn towards him- if there is a target
			// within a reasonable angle of you.
			//

			turn_to_target(
				p_person,
				FIND_DIR_FRONT);
		}
	}
//#ifdef	TURN_WHILE_IN_PUNCH_OR_KICK_ANIM
	//
	// if your in a combo you can't turn to attack other people unless this is defined
	//

	else
		if(p_person->State==STATE_FIGHTING)
		{
/*
			if (p_player->Genus.Player->Pressed & INPUT_MASK_FORWARDS)
			{

				if(p_person->SubState==SUB_STATE_STEP_FORWARD && p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT_STEP_N && p_person->Draw.Tweened->FrameIndex<3)
				{
						CBYTE	str[50];
						sprintf(str,"FORWARD double time %d \n",p_person->Genus.Person->Timer1);
						CONSOLE_text(str);
					//
					// person is moving forward
					//
					if(p_person->Genus.Person->Timer1<4)
					{

						//
						// tap forwards then tap backwards within 5 gameturns
						//
						p_person->Genus.Person->Mode = PERSON_MODE_RUN;
						set_person_running(p_person);
						return(INPUT_MASK_FORWARDS);

					}
				}
			}
*/

			//
			//	Interrupt combo's
			//
/*

			if(p_person->SubState==SUB_STATE_KICK ||p_person->SubState==SUB_STATE_PUNCH)
			{
				if (p_player->Genus.Player->Pressed & INPUT_MASK_BACKWARDS)
				{
					turn_to_direction_and_find_target(p_person,FIND_DIR_BACK);
				}
				else
				if (p_player->Genus.Player->Pressed & INPUT_MASK_LEFT)
				{
					turn_to_direction_and_find_target(p_person,FIND_DIR_LEFT);
				}
				else
				if (p_player->Genus.Player->Pressed & INPUT_MASK_RIGHT)
				{
					turn_to_direction_and_find_target(p_person,FIND_DIR_RIGHT);
				}


			}
			else
*/
			if(p_person->SubState==SUB_STATE_STEP_FORWARD)
			{
				if (p_player->Genus.Player->Pressed & INPUT_MASK_JUMP)
				{
					switch(p_person->Draw.Tweened->CurrentAnim)
					{
						case	ANIM_FIGHT_STEP_E:
							set_person_flip(p_person,1);
						//PANEL_new_text(NULL,4000,"combat flip");
							return INPUT_MASK_JUMP;
							break;
						case	ANIM_FIGHT_STEP_W:
							set_person_flip(p_person,0);
						//PANEL_new_text(NULL,4000,"combat flip");
							return INPUT_MASK_JUMP;
							break;
					}
				}

				if (p_player->Genus.Player->Pressed & INPUT_MASK_BACKWARDS)
				{

					if(p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT_STEP_S)// && p_person->Draw.Tweened->FrameIndex<2)
					{

							//CBYTE	str[50];
							//sprintf(str,"BACKWARD double time %d \n",p_person->Genus.Person->Timer1);
							//CONSOLE_text(str,2000);
						//
						// person is moving backward
						//
						if(p_person->Genus.Person->Timer1<4)
						{
							//
							// tap backward then tap backwards within 5 gameturns so flip round
							//
							reset_gang_attack(p_person);
							turn_to_direction_and_find_target(p_person,FIND_DIR_BACK);

	//						p_person->Draw.Tweened->Angle+=1024;
	//						p_person->Draw.Tweened->Angle&=2047;
							set_person_fight_idle(p_person);
	//						FC_force_camera_behind();
						//PANEL_new_text(NULL,4000,"flip round");
							return(INPUT_MASK_BACKWARDS);
							//return(0);
						}
					}
				}
				if (p_player->Genus.Player->Pressed & INPUT_MASK_RIGHT)
				{
					if(p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT_STEP_E)// && p_person->Draw.Tweened->FrameIndex<2)
					{
							//CBYTE	str[50];
							//sprintf(str,"right double time %d \n",p_person->Genus.Person->Timer1);
							//CONSOLE_text(str);
						//
						// person is sidestepping right
						//
						if(p_person->Genus.Person->Timer1<4)
						{
							//
							// tap right then tap right within 5 gameturns
							//

							reset_gang_attack(p_person);
							turn_to_direction_and_find_target(p_person,FIND_DIR_RIGHT);
	//						p_person->Draw.Tweened->Angle-=512;
	//						p_person->Draw.Tweened->Angle&=2047;
	//						FC_force_camera_behind();
						//PANEL_new_text(NULL,4000,"turn right");
							set_person_fight_idle(p_person);
							return(INPUT_MASK_RIGHT);
						}
					}
				}
				if (p_player->Genus.Player->Pressed & INPUT_MASK_LEFT)
				{
					if(p_person->Draw.Tweened->CurrentAnim==ANIM_FIGHT_STEP_W)// && p_person->Draw.Tweened->FrameIndex<2)
					{
							//CBYTE	str[50];
							//sprintf(str,"LEFT double time %d \n",p_person->Genus.Person->Timer1);
							//CONSOLE_text(str);
						//
						// person is moving forward
						//
						if(p_person->Genus.Person->Timer1<4)
						{
							//
							// tap right then tap left within 5 gameturns
							//

							reset_gang_attack(p_person);
							turn_to_direction_and_find_target(p_person,FIND_DIR_LEFT);
	//						p_person->Draw.Tweened->Angle+=512;
	//						p_person->Draw.Tweened->Angle&=2047;
	//						FC_force_camera_behind();
						//PANEL_new_text(NULL,4000,"turn left");
							set_person_fight_idle(p_person);
							return(INPUT_MASK_LEFT);
						}
					}

				}
			}
		}
//#endif

	//
	// Can only attack while idle- can't be recoiling.
	//

	if (p_person->SubState == SUB_STATE_GRAPPLE_HELD||is_person_ko(p_person)) //(p_person->Genus.Person->Flags&FLAG_PERSON_KO))
	{
		if (pl->Pressed & (INPUT_MASK_ACTION|INPUT_MASK_PUNCH|INPUT_MASK_KICK|INPUT_MASK_JUMP))
		{
			p_person->Genus.Person->Flags|=FLAG_PERSON_REQUEST_BLOCK;
			pl->DoneSomething = TRUE;
						//PANEL_new_text(NULL,4000,"grapple");
			return (pl->Pressed & (INPUT_MASK_ACTION|INPUT_MASK_PUNCH|INPUT_MASK_KICK|INPUT_MASK_JUMP));

		}
		return(0);
	}

	if (pl->Pressed & INPUT_MASK_ACTION)
	{
		//
		// Shall we bend down to pick up a special or arrest someone?
		//

		extern	UWORD	find_arrestee(Thing *p_person);
		if (p_person->SubState != SUB_STATE_GRAPPLE_HELD && p_person->SubState != SUB_STATE_GRAPPLE_HOLD)
		if(p_person->State==STATE_IDLE || p_person->State==STATE_FIGHTING||p_person->SubState==SUB_STATE_RUN_STOP)
		{
			UWORD index;

				//PANEL_new_text(NULL,4000,"COMBAT find arrest");
			if (p_person->Genus.Person->PersonType==PERSON_DARCI && (index=find_arrestee(p_person)))
			{
				set_person_arrest(p_person,index);
				pl->DoneSomething = TRUE;
				//PANEL_new_text(NULL,4000,"combat DO arrest");
				return INPUT_MASK_ACTION;
			}

			index = THING_find_nearest(
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Y >> 8,
						p_person->WorldPos.Z >> 8,
						0xa0,
						1 << CLASS_SPECIAL);

			if (index)
			{
				Thing *special_thing = TO_THING(index);

				if (should_person_get_item(p_person, special_thing))
				{
					//
					// Bend down to pick up the special.
					//

					set_person_special_pickup(p_person);
				//PANEL_new_text(NULL,4000,"combat pickup item");

					return (INPUT_MASK_ACTION);
				}
			}

			person_pick_best_target(p_person,1);
			pl->DoneSomething = TRUE;
			return INPUT_MASK_ACTION;
		}
	}

	if (p_person->State == STATE_IDLE||p_person->SubState == SUB_STATE_STEP_FORWARD)
	{
		SLONG	dir=0;
		if(p_person->SubState == SUB_STATE_STEP_FORWARD)
		{
			switch(p_person->Draw.Tweened->CurrentAnim)
			{
				case	ANIM_FIGHT_STEP_N:
				case	ANIM_FIGHT_STEP_N_BAT:
					dir=1;
					break;
				case	ANIM_FIGHT_STEP_E:
				case	ANIM_FIGHT_STEP_E_BAT:
					dir=2;
					break;
				case	ANIM_FIGHT_STEP_S:
				case	ANIM_FIGHT_STEP_S_BAT:
					dir=3;
					break;
				case	ANIM_FIGHT_STEP_W:
				case	ANIM_FIGHT_STEP_W_BAT:
					dir=4;
					break;


			}
		}
		if (pl->Pressed & INPUT_MASK_PUNCH)
		{
			//if(p_person->Genus.Person->Timer1<5)
/*
			if(p_person->Genus.Person->Timer1<5 || (input&(INPUT_MASK_DIR)))
			{
				if ((input & INPUT_MASK_LEFT) ||dir==4)
				{
					//
					// Punch left.
					//

					//
					// This call positions Darci so that there is someone to punch who
					// is standing 90 degrees to her left (and 0x80 distance away)
					//
					// If it can't find a suitable target, then it does nothing.
					//
					if(count_gang(p_person)>1)
					{

						turn_to_target(
							p_person,
							FIND_DIR_LEFT);

						//
						// Punch someone standing 90 degrees to the left.
						//
						p_person->Draw.Tweened->Angle+=512;
						p_person->Draw.Tweened->Angle&=2047;
					}

					set_player_punch(p_person);

					MSG_add("Kick left");
					return(INPUT_MASK_PUNCH|INPUT_MASK_LEFT);
				}
				else
				if ((input & INPUT_MASK_RIGHT)||dir==2)
				{
					//
					// Punch right.
					//

					//
					// This call positions Darci so that there is someone to punch who
					// is standing 90 degrees to her right (and 0x80 distance away)
					//
					// If it can't find a suitable target, then it does nothing.
					//

					if(count_gang(p_person)>1)
					{
						turn_to_target(
							p_person,
							FIND_DIR_RIGHT);

						//
						// Punch someone standing 90 degrees to the right.
						//
						p_person->Draw.Tweened->Angle-=512;
						p_person->Draw.Tweened->Angle&=2047;
					}

					set_player_punch(p_person);

					MSG_add("Punch right");
					return(INPUT_MASK_PUNCH|INPUT_MASK_RIGHT);
				}
				else
//				if ((pl->ThisInput & INPUT_MASK_BACKWARDS)||dir==3)
				if ((input & INPUT_MASK_BACKWARDS)||dir==3)
				{
					//
					// Punch back?
					//

					//
					// This call positions Darci so that there is someone to punch who
					// is standing directly behind her (and 0x80 distance away)
					//
					// If it can't find a suitable target, then it does nothing.
					//

					if(count_gang(p_person)>1)
					{
						turn_to_target(
							p_person,
							FIND_DIR_BACK);

						//
						// Punch someone standing direcly behind you
						//

						p_person->Draw.Tweened->Angle+=1024;
						p_person->Draw.Tweened->Angle&=2047;
						//set_person_kick(p_person);
					}

					 set_player_punch(p_person);

					MSG_add("Punch back");
					 return(INPUT_MASK_PUNCH|INPUT_MASK_BACKWARDS);
				}
			}
*/
			{
				//
				// A forward punch.
				//

				//turn_to_target_and_punch(p_person);
				 set_player_punch(p_person);

				//play_quick_wave(
				//	p_person,
				//	S_HIYAR,
				//	WAVE_PLAY_INTERUPT);
			}

			pl->DoneSomething = TRUE;
		}
		else
		if (pl->Pressed & INPUT_MASK_KICK)
		{
			if(p_person->Genus.Person->Timer1<5 || (input&(INPUT_MASK_DIR)))
			{
				if ((input & INPUT_MASK_LEFT)||dir==4)
				{
					if (p_person->State != STATE_JUMPING)
					{
						//
						// Kick left.
						//

						//
						// This call positions Darci so that there is someone to punch who
						// is standing 90 degrees to her left (and 0x80 distance away)
						//
						// If it can't find a suitable target, then it does nothing.
						//

						turn_to_target(
							p_person,
							FIND_DIR_LEFT|FIND_DIR_DONT_TURN);

						//
						// Punch someone standing 90 degrees to the left.
						//

						//
						// this bypasses block code
						//
						set_person_kick_dir(p_person,3);

						MSG_add("Kick left");
						return(INPUT_MASK_KICK|INPUT_MASK_LEFT);
					}
				}
				else
				if ((input & INPUT_MASK_RIGHT)||dir==2)
				{
					if (p_person->State != STATE_JUMPING)
					{
						//
						// Kick right.
						//

						//
						// This call positions Darci so that there is someone to punch who
						// is standing 90 degrees to her right (and 0x80 distance away)
						//
						// If it can't find a suitable target, then it does nothing.
						//

						turn_to_target(
							p_person,
							FIND_DIR_RIGHT|FIND_DIR_DONT_TURN);

						//
						// Kick someone standing 90 degrees to the right.
						//

	//					p_person->Draw.Tweened->Angle-=512;
	//					p_person->Draw.Tweened->Angle&=2047;
						//
						// this bypasses block code
						//
						set_person_kick_dir(p_person,1);

						MSG_add("Kick right");
						return(INPUT_MASK_KICK|INPUT_MASK_RIGHT);
					}
				}
				else
				if ((input & INPUT_MASK_BACKWARDS)||dir==3)
				{
					if (p_person->State != STATE_JUMPING)
					{
						//
						// Kick back?
						//

						//
						// This call positions Darci so that there is someone to punch who
						// is standing directly behind her (and 0x80 distance away)
						//
						// If it can't find a suitable target, then it does nothing.
						//

						turn_to_target(
							p_person,
							FIND_DIR_BACK|FIND_DIR_DONT_TURN);

						//
						// Kick someone standing direcly behind you
						//

						//
						// this bypasses block code
						//
	#ifdef	SOME_POO
						AENG_world_line(
							p_person->WorldPos.X >> 8,
							p_person->WorldPos.Y >> 8,
							p_person->WorldPos.Z >> 8,
							0x10,
							0xffffff,
							p_person->WorldPos.X           >> 8,
							p_person->WorldPos.Y + 0x10000 >> 8,
							p_person->WorldPos.Z           >> 8,
							0x0,
							0xffeeee,
							FALSE);
						{
							CBYTE	str[100];
							sprintf(str," back kick timer1 %d\n",p_person->Genus.Person->Timer1);
							CONSOLE_text(str);
						}
	#endif
						p_person->Genus.Person->Timer1=6;


						set_person_kick_dir(p_person,2);
				//PANEL_new_text(NULL,4000,"combat kick dir");

						MSG_add("Kick back");
						return(INPUT_MASK_KICK|INPUT_MASK_BACKWARDS);
					}
				}
			}

			if (p_person->State != STATE_JUMPING)
			{
				//
				// A forward kick.
				//

				turn_to_target_and_kick(p_person);

				//play_quick_wave(
				//	p_person,
				//	S_HIYAR,
				//	WAVE_PLAY_INTERUPT);
			}

			//
			// So we can differentiate a left-release from a left-punch-release
			//

			pl->DoneSomething = TRUE;
		}
		else
		if (pl->Pressed & INPUT_MASK_ACTION)
		{
			//
			// action in fight is block
			//
			p_person->Genus.Person->Mode = PERSON_MODE_RUN;
			set_person_idle(p_person);


			//set_person_block(p_person);
			pl->DoneSomething = TRUE;
				//PANEL_new_text(NULL,4000,"action in fight is block");
			return(INPUT_MASK_ACTION);

		}
/*
		if ((pl->Released & INPUT_MASK_LEFT) && !pl->DoneSomething)
		{
			//
			// Turn to the person on your left.
			//

			turn_to_target(
				p_person,
				FIND_DIR_TURN_LEFT);	// Positions Darci to face a person on her left.
		}

		if ((pl->Released & INPUT_MASK_RIGHT) && !pl->DoneSomething)
		{
			//
			// Turn to a person on your right.
			//

			turn_to_target(
				p_person,
				FIND_DIR_TURN_RIGHT);	// Positions Darci to face a person on her right.
		}
*/
	}
	else
	{
		//
		// Make sure she can do combos.
		//

		if (pl->Pressed & INPUT_MASK_PUNCH)
		{
			if(!(p_person->Genus.Person->Flags & FLAG_PERSON_REQUEST_PUNCH))
			{
				p_person->Genus.Person->Flags |= FLAG_PERSON_REQUEST_PUNCH;

				//
				// timer used to determine if time between pressing punch and going to stage 2 of combo is short enough
				//
				p_person->Genus.Person->pcom_ai_counter=0;

			}

		}

		if (pl->Pressed & INPUT_MASK_KICK)
		{
			if(!(p_person->Genus.Person->Flags & FLAG_PERSON_REQUEST_KICK))
			{
				p_person->Genus.Person->Flags |= FLAG_PERSON_REQUEST_KICK;
				//
				// timer used to determine if time between pressing kick and going to stage 2 of combo is short enough
				//
				p_person->Genus.Person->pcom_ai_counter=0;
			}
		}
	}

	return 0;
}

/*
#define MAX_CAR_INPUT 1024

typedef struct
{
	SLONG dx;
	SLONG dz;

} CAR_Input;

CAR_Input car_input[MAX_CAR_INPUT];
SLONG     car_input_upto;
SLONG     car_inputting;
*/

ULONG	apply_button_input_car(Thing *p_furn,ULONG input)
{
	ULONG	processed_input=0;

	ASSERT(p_furn->Class==CLASS_VEHICLE);

	Vehicle*	veh = p_furn->Genus.Vehicle;

	// get analogue / digital steering inputs


	// DC is always analogue
#ifndef TARGET_DC
	if(analogue)
#endif
	{
/*
		SLONG	dx,vx;

		dx = GET_JOYX(input);	// -128 to 127
		vx = p_furn->Velocity-1000;
		SATURATE(vx,0,1000);

		vx = 64000-(vx*48);

		veh->IsAnalog = 1;

		dx =(((dx+31)&0xffffffe0) * vx) >> 17;	// now -64 to +63, * (290/256)
		SATURATE(dx,-32,32);

		veh->Steering = dx;
*/

#if 0
		veh->Steering = GET_JOYX(input);
		if(abs(veh->Steering)<48)
			veh->Steering=0;
#else
		// Damped a bit.
		static SWORD wCurrentSteering = 0;
#define STEERING_MAX_DELTA 30
		SWORD wSteering = ( GET_JOYX(input) );
		if ( ( input & ( INPUT_MASK_LEFT | INPUT_MASK_RIGHT ) ) == 0 )
		{
			// Not big enough yet.
			wSteering=0;
		}

		if ( wCurrentSteering > wSteering )
		{
			wCurrentSteering -= STEERING_MAX_DELTA;
			if ( wCurrentSteering < wSteering )
			{
				wCurrentSteering = wSteering;
			}
		}
		else
		{
			wCurrentSteering += STEERING_MAX_DELTA;
			if ( wCurrentSteering > wSteering )
			{
				wCurrentSteering = wSteering;
			}
		}
		veh->Steering = wCurrentSteering;
#endif

		veh->IsAnalog = 1;

	}
#ifndef TARGET_DC
	else
	{
		veh->IsAnalog = 0;
		veh->Steering = 0;

		if (input & INPUT_MASK_LEFT)	veh->Steering--;
		if (input & INPUT_MASK_RIGHT)	veh->Steering++;
	}
#endif

	// decode controls
	veh->DControl = 0;

	if (input & INPUT_CAR_ACCELERATE)		veh->DControl |= VEH_ACCEL;
	else if (input & INPUT_CAR_DECELERATE)	veh->DControl |= VEH_DECEL;
	if (input & INPUT_CAR_GOFASTER)			veh->DControl |= VEH_FASTER;
	if (input & INPUT_CAR_SIREN)			veh->DControl |= VEH_SIREN;

	return(processed_input);
}


//
// Driving a bike.
//
#ifndef PSX
#ifdef BIKE
ULONG apply_button_input_bike(Thing *p_bike, ULONG input)
{
	ASSERT(p_bike->Class == CLASS_BIKE);

	//
	// Get the current steering.
	//

	BIKE_Control bc = BIKE_control_get(p_bike);

	SLONG new_steer = 0;//bc.steer;
	SLONG new_accel = 0;//bc.accel;

	//
	// Turn different amounts depending on the speed.
	//

	SLONG speed  = BIKE_get_speed(p_bike);
	SLONG tspeed = 15 - abs(speed);

	SATURATE(tspeed, 15, 1);

	//
	// Turning left / right.
	//

	if (input & INPUT_MASK_LEFT)
	{
		new_steer =+1; //+= tspeed;
	}
	else
	if (input & INPUT_MASK_RIGHT)
	{
		new_steer =-1; //-= tspeed;
	}

	//
	// Accelerate/brake.
	//

	if (input & INPUT_MASK_FORWARDS)
	{
		new_accel =1;//+= 20;
	}

	if (input & (INPUT_MASK_BACKWARDS|INPUT_MASK_KICK))
	{
//		new_accel =-1;//-= 20;

		bc.brake=1;
	}
	else
	{
		bc.brake=0;
	}

	if (input & INPUT_MASK_JUMP)
	{
		bc.wheelie=1;
	}
	else
	{
		bc.wheelie=0;
	}

	SATURATE(new_steer, -127, +127);
	SATURATE(new_accel, -127, +127);

	bc.accel = new_accel;
	bc.steer = new_steer;

	BIKE_control_set(p_bike, bc);

	return 0;
}
#endif
#endif



SLONG last_camera_dx;
SLONG last_camera_dy;
SLONG last_camera_dz;

SLONG last_camera_yaw;
SLONG last_camera_pitch;

SLONG look_pitch = 0;

void	set_look_pitch(SLONG p)
{
	look_pitch=p;
}

ULONG	debug_input;

ULONG	m_CurrentInput = 0;
ULONG	m_PreviousInput = 0;
ULONG	m_CurrentGoneDownInput = 0;

ULONG	get_last_input ( UWORD type )
{
	if ( type == INPUT_TYPE_GONEDOWN )
	{
		return ( m_CurrentGoneDownInput );
	}
	else
	{
		ASSERT ( type == 0 );
		return ( m_CurrentInput );
	}
}


// When there was the last interesting input.
DWORD g_dwLastInputChangeTime = 0;


ULONG	get_hardware_input(UWORD type)
{
	ULONG	input=0;
	ULONG	padd;
	SLONG   dist;
	UWORD	c0;


	static bool bLastInputWasntAnInputCozThereWasNoController = TRUE;

//
//	Temporary joystick stuff.
//

#ifdef TARGET_DC
// Deal with analogue buttons as well as
// digital ones. This works fine for the analogue triggers
// of the DC.
#define BUTTON_IS_PRESSED(value) ((value&0x80)!=0)
#else
// Don't know what happens on the PC, so I'll assume that
// whatever was originally there works.
#define BUTTON_IS_PRESSED(value) (value)
#endif


#ifdef TARGET_DC
#define	AXIS_CENTRE				128
// In-game deadzone size.
//#define	NOISE_TOLERANCE			8
// If there's no walking, we need to grow this a bit so you can turn on the spot easier.
#define	NOISE_TOLERANCE			24

// Menu deadzone size.
#define	NOISE_TOLERANCE_REMAP	64
// This should be more than 32, because that is the value when the controller goes into the
// corner "klunk", and it's really annoying to have Darci walking just at that point.
//#define RUN_WALK_LEVEL			42
// Sod it - ditch walking by this method altogether - it's too much of a pain.
// If you want to walk, press the walk button.
#define RUN_WALK_LEVEL			128


#define	AXIS_MIN			(AXIS_CENTRE-NOISE_TOLERANCE)
#define	AXIS_MAX			(AXIS_CENTRE+NOISE_TOLERANCE)
#define	AXIS_MIN_REMAP		(AXIS_CENTRE-NOISE_TOLERANCE_REMAP)
#define	AXIS_MAX_REMAP		(AXIS_CENTRE+NOISE_TOLERANCE_REMAP)


#else
#define	AXIS_CENTRE			32768
#define	NOISE_TOLERANCE		8192
#define	AXIS_MIN			(AXIS_CENTRE-NOISE_TOLERANCE)
#define	AXIS_MAX			(AXIS_CENTRE+NOISE_TOLERANCE)
#endif


#ifndef	PSX
extern DIJOYSTATE			the_state;


	#ifdef TARGET_DC

	DWORD dwCurrentTime = timeGetTime();
	if ( g_dwLastInputChangeTime == 0 )
	{
		// Almost certainly start of day!
		g_dwLastInputChangeTime = dwCurrentTime;
	}
	// Do this now, rather than later (coz there are some returns dotted around).
	// Even if a key is pressed this time, that's no problem - screensaver will be
	// disabled next frame.
	// I don't think we need to worry about wraparound much - this only does so every 49 days.
	// The Sega spec says 300 seconds, so we shall make it 300 seconds.
	if ( (unsigned)( dwCurrentTime - g_dwLastInputChangeTime ) > 1000 * 300 )
	{
		// Enable the screensaver.
		PANEL_enable_screensaver();
	}
	else
	{
		// Nope - turn it off.
		PANEL_disable_screensaver();
	}

	#else

	DWORD dwCurrentTime = 0;

	#endif



	if(type&INPUT_TYPE_JOY)
	{
		if(ReadInputDevice())
		{
			DIJOYSTATE my_copy_of_the_state;
//			if (CAM_get_mode() != CAM_MODE_FIRST_PERSON)
			{
				ULONG ulAxisMax = AXIS_MAX;
				ULONG ulAxisMin = AXIS_MIN;
#ifdef TARGET_DC
				if ( type & INPUT_TYPE_REMAP_DPAD )
				{
					ulAxisMax = AXIS_MAX_REMAP;
					ulAxisMin = AXIS_MIN_REMAP;
				}
#endif

				if(the_state.lX > ulAxisMax)
				{
					g_dwLastInputChangeTime = dwCurrentTime;
					input |= INPUT_MASK_RIGHT;
				}
				else if(the_state.lX < ulAxisMin)
				{
					g_dwLastInputChangeTime = dwCurrentTime;
					input |= INPUT_MASK_LEFT;
				}


				if(the_state.lY > ulAxisMax)
				{
					g_dwLastInputChangeTime = dwCurrentTime;
					input|=INPUT_MASK_BACKWARDS;
				}
				else if(the_state.lY < ulAxisMin)
				{
#ifdef TARGET_DC
					input|=INPUT_MASK_FORWARDS;

					// MIKE! Roper doesn't exist in the frontend - you're confusing the poor thing.
					if ( BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_MOVE]] ) &&
						 ( NETPERSON != NULL ) &&
						 ( NET_PERSON(PLAYER_ID) != NULL ) &&
						 ( NET_PERSON(PLAYER_ID)->Genus.Person != NULL ) &&
						 ( NET_PERSON(PLAYER_ID)->Genus.Person->PersonType != PERSON_ROPER ) )
					{
						// Forces a walk - no running. (Except roper can't force Walk)
					}
					else
					{
#if 0
						//
						// run if its roper, MikeD
						//
						if ( ( the_state.lY < RUN_WALK_LEVEL ) ||
							 ( ( NETPERSON != NULL ) &&
							   ( NET_PERSON(PLAYER_ID) != NULL ) &&
							   ( NET_PERSON(PLAYER_ID)->Genus.Person != NULL ) &&
							   ( NET_PERSON(PLAYER_ID)->Genus.Person->PersonType==PERSON_ROPER ) ) )
						{
							// More than half forwards, so activate "run" as well.
							input|=INPUT_MASK_MOVE;
						}
#else
						// Everyone always runs now.
						{
							ASSERT ( RUN_WALK_LEVEL == 128 );
							input|=INPUT_MASK_MOVE;
						}
#endif
					}
#else
					input|=INPUT_MASK_FORWARDS;

					// roper can't run on PC analogue now

//					if (the_state.lY < 10000 || NET_PERSON(PLAYER_ID)->Genus.Person->PersonType==PERSON_ROPER)
//					{
//						input|=INPUT_MASK_MOVE;
//					}
					input|=INPUT_MASK_MOVE;
#ifdef AMERICA
					input|=INPUT_MASK_MOVE;
#endif
#endif
					g_dwLastInputChangeTime = dwCurrentTime;
				}




				if ( BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_MOVE]] ) )
				{
					// Force walk
					m_bForceWalk = TRUE;
				}
				else
				{
					m_bForceWalk = FALSE;
				}




#ifdef TARGET_DC
				// This is set to the right value by the "analogue" env var at start of game.
				//analogue = 1;
#if 0
				// Linear
				input|=((the_state.lX>>1)+0)<<18;
				input|=((the_state.lY>>1)+0)<<25;    //on PC -128 is up!
#else
				// Squared - seems to work much better - less sensitive in the middle.
				int iXaxis = the_state.lX - AXIS_CENTRE;
				int iYaxis = the_state.lY - AXIS_CENTRE;
				if ( iXaxis < 0 )
				{
					iXaxis *= -iXaxis;
				}
				else
				{
					iXaxis *= iXaxis;
				}
				if ( iYaxis < 0 )
				{
					iYaxis *= -iYaxis;
				}
				else
				{
					iYaxis *= iYaxis;
				}
				iXaxis >>= 7;
				iYaxis >>= 7;
				iXaxis = ( iXaxis + AXIS_CENTRE ) >> 1;
				iYaxis = ( iYaxis + AXIS_CENTRE ) >> 1;
				ASSERT ( ( iXaxis >= 0 ) && ( iXaxis <= 0x7f ) );
				ASSERT ( ( iYaxis >= 0 ) && ( iYaxis <= 0x7f ) );
				input |= iXaxis << 18;
				input |= iYaxis << 25;    //on PC -128 is up!
#endif

#else
				if(analogue)
				{
					//
					// -64 to +63    7 bits per axis
					//
					input|=((the_state.lX>>9)+0)<<18;
					input|=((the_state.lY>>9)+0)<<25;    //on PC -128 is up!
				}
/*
				analogue = 0;

				{
					SLONG xdir;
					SLONG ydir;

					xdir = ((the_state.lX) >> 9) & 0x7f;
					ydir = ((the_state.lY) >> 9) & 0x7f;

					input &= ~(0x7f << 18);
					input |= xdir << 18;

					input &= ~(0x7f << 25);
					input |= ydir << 25;
				}
*/


#endif


/*
				if (Keys[KB_I])
				{
					SLONG i;

					for (i = 0; i < 16; i++)
					{
						if (the_state.rgbButtons[i])
						{
							CBYTE tit[64];

							sprintf(tit, "Button %d", i);

							CONSOLE_text(tit, 500);
						}
					}
				}
*/






#ifdef TARGET_DC


				// Detect ABXY+Start behaviour. The first press must return you
				// immediately (i.e. no confirmation) to the front-end menu, and
				// from there another press must return to the boot menu.
				if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_A]) &&
					 BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_B]) &&
					 BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_X]) &&
					 BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_Y]) &&
					 BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_START]) )
				{
					if ( !m_bDreamcastABXYStartComboPressedLastTime )
					{
						// Yes. Set the flag.
						g_bDreamcastABXYStartComboPressed = TRUE;
						m_bDreamcastABXYStartComboPressedLastTime = TRUE;

						if ( g_bDreamcastABXYStartShouldGoToBootRomImmediately )
						{
							// We haven't go to the Main menu yet, so just toast the machine, back to the boot ROM.
							ASSERT ( FALSE );
							ResetToFirmware();
						}
					}
				}
				else
				{
					m_bDreamcastABXYStartComboPressedLastTime = FALSE;
				}





#ifdef DREAMCAST_CHEATS_PLEASE_BOB
				{
					// Register "button strings" and spot cheat strings.

					static DWORD dwLastTimeAButtonWasAdded = 0;


					if ( BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ) && BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ) )
					{
						// Add the current button (if any) to the string.
						char cAdd = ' ';
	#define ADD_CHEAT_BUTTON(name,character) if ( BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_ ## name] ) ) cAdd = character
						// Can't use the start button for cheats!
						ADD_CHEAT_BUTTON ( A,		'A' );
						ADD_CHEAT_BUTTON ( B,		'B' );
						ADD_CHEAT_BUTTON ( X,		'X' );
						ADD_CHEAT_BUTTON ( Y,		'Y' );
						ADD_CHEAT_BUTTON ( UP,		'U' );
						ADD_CHEAT_BUTTON ( DOWN,	'D' );
						ADD_CHEAT_BUTTON ( LEFT,	'L' );
						ADD_CHEAT_BUTTON ( RIGHT,	'R' );

						if ( cCheatString[0] != cAdd )
						{
							for ( int i = sizeof(cCheatString) - 1; i > 0; i-- )
							{
								cCheatString[i] = cCheatString[i-1];
							}
							cCheatString[0] = cAdd;
							dwLastTimeAButtonWasAdded = GAME_TURN;
						}


						// Now look for strings. Remember, these are _backwards_.
	#define IS_STRING(string) ( strncmp ( string, cCheatString, strlen (string) ) == 0 )
						if ( IS_STRING ( "Y R R A B Y R R A B" ) )
						{
							// BARRY BARRY
							// Enable the fast-key cheats.
							CONSOLE_text("OK, fast cheats enabled, D'arci.");
							g_bCheatsEnabled = TRUE;
						}
						else if ( IS_STRING ( "Y X B A" ) )
						{
							// Hehe.
							CONSOLE_text("That's not a cheat code, D'arci!");
							cCheatString[0] = '\0';
						}
						else if ( IS_STRING ( "A R A L A R A L" ) )
						{
							// LARALARA (yes, as in croft).
							// Unlock all missions.
							g_iCheatNumber = 0x1a1a0001;
						}
						else if ( IS_STRING ( "B A X R D L" ) )
						{
							// LDR XAB. Two "V" shapes.
							// Shows version number.
							g_iCheatNumber = 0x1a1a0002;
						}
						else if ( IS_STRING ( "R U D D A L B" ) )
						{
							// "BLADDUR" = "Bladder" - like a pigskin - inflate it!
							// Yeah, tenuous I know. Like I give a monkeys.
							// Toggle inflation!
							g_iCheatNumber = 0x10f1a7e;
						}
						else if ( IS_STRING ( "A B B A D A B B A Y" ) )
						{
							// YABBADABBA (doo)
							// Finish this level immediately.
							// 0xd01e7e1 = "do level" sort of.
							g_bPunishMePleaseICheatedOnThisLevel = TRUE;
							cheat = 1;
							g_iCheatNumber = 0xd01e7e1;
						}
						else if ( IS_STRING ( "L L U D Y B A B" ) )
						{
							// BABYDULL. Ha ha.
							// Toggle whether you can move when both triggers are down (for debugging stuff).
extern bool m_bTweakFramerates;
							m_bTweakFramerates = !m_bTweakFramerates;
							// And stop it triggering again.
							cCheatString[0] = '*';
						}
					}
					else
					{
						// Both triggers not held - end current cheat.
						cCheatString[0] = '\0';
						g_iCheatNumber = 0;
					}



#undef IS_STRING
#undef ADD_CHEAT_BUTTON
				}
#endif



#if 0
#if defined(DEBUG) && defined(TARGET_DC)
				static float fFogStart = 0.0f;
				static float fFogEnd = 100.0f;
				static float fFogDensity = 1.0f;
				static DWORD dwFogTableMode = D3DFOG_LINEAR;

				if (
					 ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ) ) &&
					 ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ) ) )
				{
					if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_X] ) )
					{
						// Change FogStart
						//fFogStart += ( (float)the_state.lX - AXIS_CENTRE ) / 1270.0f;
						fFogStart = (float)the_state.lX / 256.0f;
					}
					if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_B] ) )
					{
						// Change FogEnd
						//fFogEnd += ( (float)the_state.lX - AXIS_CENTRE ) / 1270.0f;
						fFogEnd = (float)the_state.lX;
					}
					if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_A] ) )
					{
						// Change FogEnd
						//fFogEnd += ( (float)the_state.lX - AXIS_CENTRE ) / 1270.0f;
						fFogDensity = (float)the_state.lX / 256.0f;
					}
					if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_Y] ) )
					{
						switch ( dwFogTableMode )
						{
						case D3DFOG_LINEAR:
							dwFogTableMode = D3DFOG_EXP;
							break;
						case D3DFOG_EXP:
							dwFogTableMode = D3DFOG_EXP2;
							break;
						case D3DFOG_EXP2:
							dwFogTableMode = D3DFOG_NONE;
							break;
						case D3DFOG_NONE:
							dwFogTableMode = D3DFOG_LINEAR;
							break;
						default:
							dwFogTableMode = D3DFOG_NONE;
							break;
						}
					}

					m_fFogTableDebugStart = fFogStart;
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLESTART, (*((DWORD *)(&fFogStart))) );
					m_fFogTableDebugEnd = fFogEnd;
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLEEND, (*((DWORD *)(&fFogEnd))) );
					m_fFogTableDebugDensity = fFogDensity;
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLEDENSITY, (*((DWORD *)(&fFogDensity))) );
					m_dwFogTableDebugFogTableMode = dwFogTableMode;
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLEMODE, dwFogTableMode);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGCOLOR, 0x00ff00ff);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, TRUE);

					memset ( the_state.rgbButtons, 0, 32 );
					input = ( 0x3f << 18 ) | ( 0x3f << 25 );
				}
				else
				{
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, FALSE);
				}
#endif
#endif



				if ( ( ( type & INPUT_TYPE_REMAP_DPAD ) != 0 ) || ( ( type & INPUT_TYPE_REMAP_BUTTONS ) != 0 ) )
				{

					// Take a copy of the state.
					my_copy_of_the_state = the_state;
					if ( ( type & INPUT_TYPE_REMAP_DPAD ) != 0 )
					{
						// Remap the D-pad.
						if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_UP] ))
						{
							// D-pad up.
							input |= INPUT_MASK_FORWARDS;
							the_state.rgbButtons[DI_DC_BUTTON_UP] = 0;
							g_dwLastInputChangeTime = dwCurrentTime;
						}
						if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_DOWN] ))
						{
							// D-pad down.
							input |= INPUT_MASK_BACKWARDS;
							the_state.rgbButtons[DI_DC_BUTTON_DOWN] = 0;
							g_dwLastInputChangeTime = dwCurrentTime;
						}
						if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_LEFT] ))
						{
							// D-pad left.
							input |= INPUT_MASK_LEFT;
							the_state.rgbButtons[DI_DC_BUTTON_LEFT] = 0;
							g_dwLastInputChangeTime = dwCurrentTime;
						}
						if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_RIGHT] ))
						{
							// D-pad right.
							input |= INPUT_MASK_RIGHT;
							the_state.rgbButtons[DI_DC_BUTTON_RIGHT] = 0;
							g_dwLastInputChangeTime = dwCurrentTime;
						}
					}

extern bool bWriteVMInsteadOfVMU;
					if ( !bWriteVMInsteadOfVMU )
					{
						// This is a Yank build - they like all buttons to do things.
						if ( ( type & INPUT_TYPE_REMAP_BUTTONS ) != 0 )
						{
							// Remap the buttons as well.
							// A,Y,C,LTRIG are DOMENU, B,X,Z,RTRIG are CANCEL
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ))
							{
								input |= INPUT_MASK_DOMENU;
								the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_A] ))
							{
								input |= INPUT_MASK_DOMENU;
								the_state.rgbButtons[DI_DC_BUTTON_A] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( ( type & INPUT_TYPE_REMAP_NOT_X_Y ) == 0 )
							{
								if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_Y] ))
								{
									input |= INPUT_MASK_DOMENU;
									the_state.rgbButtons[DI_DC_BUTTON_Y] = 0;
									g_dwLastInputChangeTime = dwCurrentTime;
								}
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_C] ))
							{
								input |= INPUT_MASK_DOMENU;
								the_state.rgbButtons[DI_DC_BUTTON_C] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_B] ))
							{
								input |= INPUT_MASK_CANCEL;
								the_state.rgbButtons[DI_DC_BUTTON_B] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( ( type & INPUT_TYPE_REMAP_NOT_X_Y ) == 0 )
							{
								if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_X] ))
								{
									input |= INPUT_MASK_CANCEL;
									the_state.rgbButtons[DI_DC_BUTTON_X] = 0;
									g_dwLastInputChangeTime = dwCurrentTime;
								}
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_Z] ))
							{
								input |= INPUT_MASK_CANCEL;
								the_state.rgbButtons[DI_DC_BUTTON_Z] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ))
							{
								input |= INPUT_MASK_CANCEL;
								the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
						}
					}
					else
					{
						// This is a European build - they like only A and B to work, and the rest to do nothing..
						if ( ( type & INPUT_TYPE_REMAP_BUTTONS ) != 0 )
						{
							// Remap the buttons as well.
							// A,Y,C,LTRIG are DOMENU, B,X,Z,RTRIG are CANCEL
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ))
							{
								//input |= INPUT_MASK_DOMENU;
								the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_A] ))
							{
								input |= INPUT_MASK_DOMENU;
								the_state.rgbButtons[DI_DC_BUTTON_A] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( ( type & INPUT_TYPE_REMAP_NOT_X_Y ) == 0 )
							{
								if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_Y] ))
								{
									//input |= INPUT_MASK_DOMENU;
									the_state.rgbButtons[DI_DC_BUTTON_Y] = 0;
									g_dwLastInputChangeTime = dwCurrentTime;
								}
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_C] ))
							{
								//input |= INPUT_MASK_DOMENU;
								the_state.rgbButtons[DI_DC_BUTTON_C] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_B] ))
							{
								input |= INPUT_MASK_CANCEL;
								the_state.rgbButtons[DI_DC_BUTTON_B] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( ( type & INPUT_TYPE_REMAP_NOT_X_Y ) == 0 )
							{
								if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_X] ))
								{
									//input |= INPUT_MASK_CANCEL;
									the_state.rgbButtons[DI_DC_BUTTON_X] = 0;
									g_dwLastInputChangeTime = dwCurrentTime;
								}
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_Z] ))
							{
								//input |= INPUT_MASK_CANCEL;
								the_state.rgbButtons[DI_DC_BUTTON_Z] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
							if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ))
							{
								//input |= INPUT_MASK_CANCEL;
								the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] = 0;
								g_dwLastInputChangeTime = dwCurrentTime;
							}
						}
					}

					if ( ( type & INPUT_TYPE_REMAP_START_BUTTON ) != 0 )
					{
						if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_START] ))
						{
							input |= INPUT_MASK_DOMENU;
							the_state.rgbButtons[DI_DC_BUTTON_START] = 0;
							g_dwLastInputChangeTime = dwCurrentTime;
						}
					}

				}
#endif //#ifdef TARGET_DC






				/*

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_MOVE]]))
				{
					input|=INPUT_MASK_MOVE;
				}

				*/

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_JUMP]]))
				{
					input|=INPUT_MASK_JUMP;
					g_dwLastInputChangeTime = dwCurrentTime;
				}

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_KICK]]))
				{
					input|=INPUT_MASK_KICK;
					g_dwLastInputChangeTime = dwCurrentTime;
				}

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_PUNCH]]))
				{
					input|=INPUT_MASK_PUNCH;
					g_dwLastInputChangeTime = dwCurrentTime;
				}

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_START]]))
				{
					input|=INPUT_MASK_START;
					g_dwLastInputChangeTime = dwCurrentTime;
				}

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_SELECT]]))
				{
					input|=INPUT_MASK_SELECT;
					g_dwLastInputChangeTime = dwCurrentTime;
				}

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_CAMERA]]))
				{
					input|=INPUT_MASK_CAMERA;
					g_dwLastInputChangeTime = dwCurrentTime;
				}

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_CAM_LEFT]]))
				{
					input|=INPUT_MASK_CAM_LEFT;
					g_dwLastInputChangeTime = dwCurrentTime;
				}

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_CAM_RIGHT]]))
				{
					input|=INPUT_MASK_CAM_RIGHT;
					g_dwLastInputChangeTime = dwCurrentTime;
				}

				if(BUTTON_IS_PRESSED(the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_ACTION]]))
				{
					MSG_add(" action pressed \n");
					input|=INPUT_MASK_ACTION;
					g_dwLastInputChangeTime = dwCurrentTime;
				}


#ifdef DREAMCAST_CHEATS_PLEASE_BOB
				if ( g_bCheatsEnabled )
				{
					static bool bWeCheatedLastFrame = FALSE;

					// Cheat enable is Start+both triggers.
					// These ignore remapping of course.
					if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_START]) &&
						 BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER]) &&
						 BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER]) )
					{
						// Captain Cheat to the rescue!
						// But disable fast time reporting.
						g_bPunishMePleaseICheatedOnThisLevel = TRUE;
						cheat = 1;
						// All cheats are on different directions of the D-pad.
						if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_UP]) )
						{
							// Health.
							CONSOLE_text("My wings are as a shield of steel.");
							NET_PERSON(0)->Genus.Person->Health=1000;
						}
						else if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_DOWN]) )
						{
							// Either:
							//		Guns. Lots of guns.
							// Or:
							//		Bigger guns. We need bigger fucking guns.
							// Depending on your SF film preference.

							if ( !bWeCheatedLastFrame )
							{

								// Stop multiple cheats every single frame, or you get guns all over the place.
								bWeCheatedLastFrame = TRUE;

#define CHEAT_RING_SIZE 128
								alloc_special(SPECIAL_AK47		, SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)+CHEAT_RING_SIZE ,NET_PERSON(0)->WorldPos.Y>>8 ,(NET_PERSON(0)->WorldPos.Z>>8)+CHEAT_RING_SIZE, 0);
								alloc_special(SPECIAL_SHOTGUN	, SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)-CHEAT_RING_SIZE ,NET_PERSON(0)->WorldPos.Y>>8 ,(NET_PERSON(0)->WorldPos.Z>>8)-CHEAT_RING_SIZE, 0);
								alloc_special(SPECIAL_GUN		, SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)-CHEAT_RING_SIZE ,NET_PERSON(0)->WorldPos.Y>>8 ,(NET_PERSON(0)->WorldPos.Z>>8)+CHEAT_RING_SIZE, 0);
								alloc_special(SPECIAL_GRENADE	, SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)+CHEAT_RING_SIZE-32 ,NET_PERSON(0)->WorldPos.Y>>8 ,(NET_PERSON(0)->WorldPos.Z>>8)-CHEAT_RING_SIZE-32, 0);
								alloc_special(SPECIAL_GRENADE	, SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)+CHEAT_RING_SIZE+00 ,NET_PERSON(0)->WorldPos.Y>>8 ,(NET_PERSON(0)->WorldPos.Z>>8)-CHEAT_RING_SIZE+00, 0);
								alloc_special(SPECIAL_GRENADE	, SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)+CHEAT_RING_SIZE+32 ,NET_PERSON(0)->WorldPos.Y>>8 ,(NET_PERSON(0)->WorldPos.Z>>8)-CHEAT_RING_SIZE+32, 0);
#undef CHEAT_RING_SIZE
								CONSOLE_text("We need guns. Lots of guns.");
							}
						}
						else if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_LEFT]) )
						{
							// More ammo.
							NET_PERSON(0)->Genus.Person->ammo_packs_pistol=240;
							NET_PERSON(0)->Genus.Person->ammo_packs_shotgun=240;
							NET_PERSON(0)->Genus.Person->ammo_packs_ak47=240;
							CONSOLE_text("Well, to tell you the truth, in all this excitement, I've kinda lost track myself.");
						}
						else if ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_RIGHT]) )
						{
							if ( !bWeCheatedLastFrame )
							{
								// Stop it toggling madly.
								bWeCheatedLastFrame = TRUE;

								// Toggle on-screen position.
								g_bShowDarcisPositionOnScreen = !g_bShowDarcisPositionOnScreen;
								CONSOLE_text("Where am I?");
								CONSOLE_text("In the village.");
							}
						}
						else
						{
							bWeCheatedLastFrame = FALSE;
						}

						// Remove any non-directional button inputs. Leave the analog and direction ones.
						input &= ( 0xfffc0000 | INPUT_MASK_FORWARDS | INPUT_MASK_BACKWARDS | INPUT_MASK_LEFT | INPUT_MASK_RIGHT );
					}
					else
					{
						// Cheat buttons not down - allow another.
						bWeCheatedLastFrame = FALSE;
					}
				}
#endif




#ifdef TARGET_DC
	extern bool m_bTweakFramerates;
				if ( m_bTweakFramerates &&
					 ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ) ) &&
					 ( BUTTON_IS_PRESSED(the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ) ) )
				{
					// Both triggers down - bin any input.
					// Otherwise stuff gets impossible to check.
					input = ( AXIS_CENTRE << 17 ) | ( AXIS_CENTRE << 24 );
				}
#endif


			}


			if(input)
			{
#ifdef TARGET_DC
				if ( ( ( type & INPUT_TYPE_REMAP_DPAD ) != 0 ) || ( ( type & INPUT_TYPE_REMAP_BUTTONS ) != 0 ) )
				{
					// Restore the copy.
					the_state = my_copy_of_the_state;
				}
#endif
				input_mode=INPUT_JOYPAD;
				m_CurrentInput = input;

				if ( bLastInputWasntAnInputCozThereWasNoController )
				{
					// There wasn't a last input, so ignore any buttons pressed when the controller is inserted/recognised.
					m_PreviousInput = m_CurrentInput;
					bLastInputWasntAnInputCozThereWasNoController = FALSE;
				}

				m_CurrentGoneDownInput = ( m_CurrentInput & ~(m_PreviousInput) ) & INPUT_MASK_ALL_BUTTONS;
				m_PreviousInput = m_CurrentInput;
				if ( type & INPUT_TYPE_GONEDOWN )
				{
					return ( m_CurrentGoneDownInput );
				}
				else
				{
					return ( m_CurrentInput );
				}
			}
		}
		else
		{
			// No controller.
			bLastInputWasntAnInputCozThereWasNoController = TRUE;
		}
	}
#endif
#ifdef	PSX
extern UBYTE PAD_Type;

	PAD_Type=PadInfoMode(0,InfoModeCurID,0);
	switch(PAD_Type)
	{
	case 0:
		GAME_FLAGS|=GF_PAUSED;
		break;
	case 4:
	case 7:
		for(c0=0;c0<14;c0++)
		{
			if (PadKeyIsPressed(&PAD_Input1,PAD_Current->data[c0].pad_button))
			{
				if (PAD_Current->data[c0].input_mask>0)
				{
					if (PAD_Current->data[c0].pad_flags&PAD_FLAG_DEBOUNCE)
					{
						if (!(psx_pad_mask&(1<<c0)))
						{
							input|=PAD_Current->data[c0].input_mask;
							psx_pad_mask|=(1<<c0);
						}
					} else
						input|=PAD_Current->data[c0].input_mask;
				}
				else
				{
					if (PAD_Current->data[c0].pad_flags&PAD_FLAG_DEBOUNCE)
					{
						if (!(psx_pad_mask&(1<<c0)))
						{
							psx_pad_mask|=(1<<c0);
							Keys[-PAD_Current->data[c0].input_mask]=1;
						}
					} else
						Keys[-PAD_Current->data[c0].input_mask]=1;
				}
			} else
				psx_pad_mask&=~(1<<c0);
		}
		if (PAD_Type==7)
		{
			UBYTE joyx,joyy;

			analogue=1;
	//		input|=INPUT_MASK_MOVE;
	//		printf("%d,%d\n",JoystickLeftX(&PAD_Input1),JoystickLeftY(&PAD_Input1));


/*
			{
				SLONG	in_dx,in_dz;
				SLONG	dx,dz;
				SLONG	angle;

				in_dx=JoystickLeftX(&PAD_Input1)-128;
				in_dz=JoystickLeftY(&PAD_Input1)-128;

				get_analogue_dxdz(in_dx,in_dz,&dx,&dz);
				angle=Arctan(-dx,dz);
				if(QDIST2(abs(dx),abs(dz))>32)
				{
					if(angle<256 || angle>2048-256)
					{
						input|=INPUT_MASK_FORWARDS;
					}
					if(angle>512-256 && angle<512+256)
						input|=INPUT_MASK_LEFT;

					if(angle>1024-256 && angle<1024+256)
						input|=INPUT_MASK_BACKWARDS;
					if(angle>1024+512-256 && angle<1024+512+256)
						input|=INPUT_MASK_RIGHT;

				}

				joyx=dx+128;
				joyy=dz+128;



			}
			input|=((joyx)>>1)<<18;
			input|=((joyy)>>1)<<25;    //on PC -128 is up!
*/

			input|=((joyx=JoystickLeftX(&PAD_Input1))>>1)<<18;
			input|=((joyy=JoystickLeftY(&PAD_Input1))>>1)<<25;    //on PC -128 is up!


			if (joyx<96-32) input|=INPUT_MASK_LEFT;
			if (joyx>160+32) input|=INPUT_MASK_RIGHT;
			if (joyy<96-32)
			{
				input|=INPUT_MASK_MOVE;
				input|=INPUT_MASK_FORWARDS;
			}
			if (joyy>160+32) input|=INPUT_MASK_BACKWARDS;


		}
		else
			analogue=0;
#ifndef	VERSION_DEMO
#ifdef CHEAT_LOCATION_KEYS
		if(input==(INPUT_MASK_PUNCH|INPUT_MASK_KICK|INPUT_MASK_ACTION|INPUT_MASK_JUMP|INPUT_MASK_LEFT))
			cheat=2;
#endif
//		if(input==(INPUT_MASK_PUNCH|INPUT_MASK_KICK|INPUT_MASK_ACTION|INPUT_MASK_JUMP|INPUT_MASK_RUN))
//			PYRO_create(NET_PERSON(0)->WorldPos, PYRO_GAMEOVER);

		if(input==(INPUT_MASK_PUNCH|INPUT_MASK_KICK|INPUT_MASK_ACTION|INPUT_MASK_JUMP|INPUT_MASK_RIGHT))
		{
			if(NET_PERSON(0)->Genus.Person->Health!=1000)
			{
				NET_PERSON(0)->Genus.Person->Health=1000;

				NET_PERSON(0)->Genus.Person->ammo_packs_pistol=240;
				NET_PERSON(0)->Genus.Person->ammo_packs_shotgun=240;
				NET_PERSON(0)->Genus.Person->ammo_packs_ak47=240;

				alloc_special(SPECIAL_AK47       , SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)+128 ,NET_PERSON(0)->WorldPos.Y>>8 ,NET_PERSON(0)->WorldPos.Z>>8 , 0);
				alloc_special(SPECIAL_SHOTGUN    , SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)-128 ,NET_PERSON(0)->WorldPos.Y>>8 ,NET_PERSON(0)->WorldPos.Z>>8 , 0);
//				alloc_special(SPECIAL_EXPLOSIVES, SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)-128 ,NET_PERSON(0)->WorldPos.Y>>8 ,(NET_PERSON(0)->WorldPos.Z>>8)+128 , 0);
				alloc_special(SPECIAL_GRENADE	 , SPECIAL_SUBSTATE_NONE,(NET_PERSON(0)->WorldPos.X>>8)+128 ,NET_PERSON(0)->WorldPos.Y>>8 ,(NET_PERSON(0)->WorldPos.Z>>8)-128 , 0);

extern SLONG Wadmenu_MuckyTime;

				Wadmenu_MuckyTime=0;
				cheat=1;
			}

		}
#endif
		debug_input=input;
		break;
	default:
		GAME_FLAGS|=GF_PAUSED;
		break;
	}
#endif //#ifdef	PSX


#ifndef REMAP_KEYBOARD

	if(type&INPUT_TYPE_KEY)
	{

/*
		if (CAM_get_mode() == CAM_MODE_FIRST_PERSON)
		{
			SLONG dyaw;
			SLONG dpitch;

			SLONG dleft;
			SLONG dright;
			SLONG dup;
			SLONG ddown;

			CAM_get_dangle(
				&dyaw,
				&dpitch);

			#define MAX_DYAW	700
			#define MAX_DPITCH	400
			#define MAX_MOVE	12

			dleft  = ( MAX_DYAW - dyaw) / 32;
			dright = (-MAX_DYAW - dyaw) / 32;

			dup   = ( MAX_DPITCH - dpitch) / 32;
			ddown = (-MAX_DPITCH - dpitch) / 32;

			SATURATE(dleft,   0, MAX_MOVE);
			SATURATE(dright, -MAX_MOVE, 0);
			SATURATE(dup,     0, MAX_MOVE);
			SATURATE(ddown,  -MAX_MOVE, 0);

			if (Keys[KB_LEFT])  {dyaw += dleft;}
			if (Keys[KB_RIGHT])	{dyaw += dright;}

			if (Keys[KB_DOWN]) {dpitch += dup;}
			if (Keys[KB_UP  ]) {dpitch += ddown;}

			if(ReadInputDevice())
			{
				if (the_state.lX > AXIS_MAX) {dyaw   += dright;}
				if (the_state.lX < AXIS_MIN) {dyaw   += dleft;}
				if (the_state.lY > AXIS_MAX) {dpitch += dup;}
				if (the_state.lY < AXIS_MIN) {dpitch += ddown;}
			}

			CAM_set_dangle(
				dyaw,
				dpitch);
		}
		else
*/
		{
			if(Keys[KB_UP])
			{
				input|=INPUT_MASK_FORWARDS;
			}

			if(Keys[KB_DOWN])
				input|=INPUT_MASK_BACKWARDS;

			if(Keys[KB_LEFT])
			{
				if(ShiftFlag)
					input|=INPUT_MASK_STEP_LEFT;
				else
					input|=INPUT_MASK_LEFT;
			}

			if(Keys[KB_RIGHT])
			{
				if(ShiftFlag)
					input|=INPUT_MASK_STEP_RIGHT;
				else
					input|=INPUT_MASK_RIGHT;
			}
		}
	}

	if(Keys[KB_ENTER])
		input|=INPUT_MASK_SELECT;

	if(type&INPUT_TYPE_KEY)
	{

		if(Keys[KB_F5])
		{
			input|=INPUT_MASK_CAMERA;
			input&=~INPUT_MASKM_CAM_TYPE;
			input|=INPUT_MASKM_CAM1;
			Keys[KB_F5]=0;
		}
		if(Keys[KB_F6])
		{
			input|=INPUT_MASK_CAMERA;
			input&=~INPUT_MASKM_CAM_TYPE;
			input|=INPUT_MASKM_CAM2;
			Keys[KB_F6]=0;
		}
		if(Keys[KB_F7])
		{
			input|=INPUT_MASK_CAMERA;
			input&=~INPUT_MASKM_CAM_TYPE;
			input|=INPUT_MASKM_CAM3;
			Keys[KB_F7]=0;
		}
		/*
		if(Keys[KB_F8])
		{
			input|=INPUT_MASK_CAMERA;
			input&=~INPUT_MASKM_CAM_TYPE;
			input|=INPUT_MASKM_CAM4;
			Keys[KB_F8]=0;
		}
		*/

		if (Keys[KB_END])
		{
			Keys[KB_END] = 0;
			input|=INPUT_MASK_CAM_BEHIND;

			//
			// Force the camera around behind the focus thing.
			//

			//FC_force_camera_behind(p_person->Genus.Person->PlayerID-1);
		}

		if (Keys[KB_DEL ])
		{
			Keys[KB_DEL ] = 0;
			input|=INPUT_MASK_CAM_LEFT;
		}
		if (Keys[KB_PGDN])
		{
			Keys[KB_PGDN] = 0;
			input|=INPUT_MASK_CAM_RIGHT;
		}

		if(Keys[KB_SPACE])
			input|=INPUT_MASK_JUMP;

		if(mouse_input)
		{
			if(Keys[KB_RCONTROL])
				input|=INPUT_MASK_JUMP;
		}

		if(Keys[KB_Z])
		{
			input|=INPUT_MASK_PUNCH;
		}
#ifndef PSX
/*
		if(LeftButton)
		{
			input|=INPUT_MASK_PUNCH;
		}

		if(RightButton)
		{
			input|=INPUT_MASK_KICK;
		}
*/
#endif
		if(Keys[KB_X])
		{
			MSG_add(" HARDWARE KICK");
			input|=INPUT_MASK_KICK;
		}

		if(Keys[KB_C])
		{
			input|=INPUT_MASK_ACTION;
		}
#ifndef PSX
		if(mouse_input)
		{
			//if(Keys[KB_P0])
			//	input|=INPUT_MASK_ACTION;
		}
#endif


/*
		if(Keys[KB_V])
		{
			input|=INPUT_MASK_MOVE;
		}
*/



		if (Keys[KB_UP])
		{
			input |= INPUT_MASK_MOVE;
		}
		/* this has since been taken over by mines...
		if(Keys[KB_M])
		{
			input|=INPUT_MASK_MODE_CHANGE;
		}*/
	}
#else // remap_keyboard

	if(type&INPUT_TYPE_KEY)
	{

#ifdef TARGET_DC
		ASSERT ( FALSE );
#else //#ifdef TARGET_DC

		if(Keys[keybrd_button_use[KEYBRD_BUTTON_FORWARDS]])
		{
			input|=INPUT_MASK_FORWARDS;
		}

		if(Keys[keybrd_button_use[KEYBRD_BUTTON_BACK]])
			input|=INPUT_MASK_BACKWARDS;

		if(Keys[keybrd_button_use[KEYBRD_BUTTON_LEFT]])
		{
			if(ShiftFlag)
				input|=INPUT_MASK_STEP_LEFT;
			else
				input|=INPUT_MASK_LEFT;
		}

		if(Keys[keybrd_button_use[KEYBRD_BUTTON_RIGHT]])
		{
			if(ShiftFlag)
				input|=INPUT_MASK_STEP_RIGHT;
			else
				input|=INPUT_MASK_RIGHT;
		}

		if(Keys[keybrd_button_use[JOYPAD_BUTTON_SELECT]])
			input|=INPUT_MASK_SELECT;

		if(Keys[KB_F5])
		{
			input|=INPUT_MASK_CAMERA;
			input&=~INPUT_MASKM_CAM_TYPE;
			input|=INPUT_MASKM_CAM1;
			Keys[KB_F5]=0;
		}
		if(Keys[KB_F6])
		{
			input|=INPUT_MASK_CAMERA;
			input&=~INPUT_MASKM_CAM_TYPE;
			input|=INPUT_MASKM_CAM2;
			Keys[KB_F6]=0;
		}
		if(Keys[KB_F7])
		{
			input|=INPUT_MASK_CAMERA;
			input&=~INPUT_MASKM_CAM_TYPE;
			input|=INPUT_MASKM_CAM3;
			Keys[KB_F7]=0;
		}

		/*
		if(Keys[KB_F8])
		{
			input|=INPUT_MASK_CAMERA;
			input&=~INPUT_MASKM_CAM_TYPE;
			input|=INPUT_MASKM_CAM4;
			Keys[KB_F8]=0;
		}
		*/

		if (Keys[keybrd_button_use[JOYPAD_BUTTON_CAMERA]])
		{
			Keys[keybrd_button_use[JOYPAD_BUTTON_CAMERA]] = 0;
			input|=INPUT_MASK_CAM_BEHIND;
		}

		if (Keys[keybrd_button_use[JOYPAD_BUTTON_CAM_LEFT]])
		{
			Keys[JOYPAD_BUTTON_CAM_LEFT] = 0;
			input|=INPUT_MASK_CAM_LEFT;
		}
		if (Keys[keybrd_button_use[JOYPAD_BUTTON_CAM_RIGHT]])
		{
			Keys[keybrd_button_use[JOYPAD_BUTTON_CAM_RIGHT]] = 0;
			input|=INPUT_MASK_CAM_RIGHT;
		}

		if(Keys[keybrd_button_use[JOYPAD_BUTTON_JUMP]])
			input|=INPUT_MASK_JUMP;

		if(Keys[keybrd_button_use[JOYPAD_BUTTON_PUNCH]])
		{
			input|=INPUT_MASK_PUNCH;
		}
		if(Keys[keybrd_button_use[JOYPAD_BUTTON_KICK]])
		{
			MSG_add(" HARDWARE KICK");
			input|=INPUT_MASK_KICK;
		}

		if(Keys[keybrd_button_use[JOYPAD_BUTTON_ACTION]])
		{
			input|=INPUT_MASK_ACTION;
		}

		/*

		// Take out the V key?!

		if(Keys[keybrd_button_use[JOYPAD_BUTTON_MOVE]])
		{
			input|=INPUT_MASK_MOVE;
		}
		*/

		if (Keys[keybrd_button_use[KEYBRD_BUTTON_FORWARDS]])
		{
//			input |= INPUT_MASK_FORWARD;
			input |= INPUT_MASK_MOVE;
		}
#endif //#else //#ifdef TARGET_DC

	}


#endif // remap_keyboard

	//
	// Sometimes, while a cutscene is playing, Simon wants Darci to stop moving.
	//

	/*

	if (EWAY_stop_player_moving())
	{
//		input = 0;
		input&=INPUT_MASK_JUMP;
	}

	*/

#ifndef PSX
/*
	if (SNIPE_on)
	{
		{
			//
			// Do sniper mode movement.
			//

			SLONG turn = 0;

			if (input & INPUT_MASK_LEFT)      {turn |= SNIPE_TURN_LEFT;}
			if (input & INPUT_MASK_RIGHT)     {turn |= SNIPE_TURN_RIGHT;}
			if (input & INPUT_MASK_FORWARDS)  {turn |= SNIPE_TURN_UP;}
			if (input & INPUT_MASK_BACKWARDS) {turn |= SNIPE_TURN_DOWN;}

			SNIPE_turn(turn);
		}

		//
		// Darci can't move while sniping...
		//

		input = 0;

	}
*/
#endif


	if(input)
	{
		input_mode=INPUT_KEYS;
	}

	m_CurrentInput = input;
	m_CurrentGoneDownInput = ( m_CurrentInput & ~(m_PreviousInput) ) & INPUT_MASK_ALL_BUTTONS;
	m_PreviousInput = m_CurrentInput;
	if ( type & INPUT_TYPE_GONEDOWN )
	{
		return ( m_CurrentGoneDownInput );
	}
	else
	{
		return ( m_CurrentInput );
	}
}


// Allow the last input state to autorepeat, despite GONEDOWN.
void allow_input_autorepeat ( void )
{
	m_PreviousInput = 0;
}




ULONG	pre_process_input(SLONG mode,ULONG input)
{
	return(input);
/*
	UWORD	output;

	output=input;

	switch(mode)
	{
		case	PERSON_MODE_FIGHT:
			break;
		case	PERSON_MODE_RUN:
			if(input&(INPUT_MASK_SPRINT|INPUT_MASK_YOMP))
			{
				output|=INPUT_MASK_FORWARDS;
			}
			break;
	}
	return(output);
*/

}

SLONG	FirstPersonMode = FALSE;

ULONG	apply_button_input_first_person(Thing *p_player, Thing *p_person,ULONG input,ULONG *processed)
{
	static SLONG look_ami   = FALSE;
	SLONG fpm = FALSE;
	SLONG	gun=0;

	*processed=0;

	//
	// Should we be in first person mode?
	//

#ifndef PSX
	extern DIJOYSTATE the_state;

#ifdef REMAP_KEYBOARD
#ifdef TARGET_DC
	if ( the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_1STPERSON]] )
#else //#ifdef TARGET_DC
	if ((Keys[keybrd_button_use[JOYPAD_BUTTON_1STPERSON]] ) || the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_1STPERSON]])
#endif //#else //#ifdef TARGET_DC
#else
	if ((Keys[KB_A] && !ShiftFlag) || the_state.rgbButtons[joypad_button_use[JOYPAD_BUTTON_1STPERSON]])
#endif
	{
		fpm = TRUE;
	}
#else


	if (input & INPUT_MASK_CAMERA)
	{
//#ifndef	PSX
		fpm = TRUE;
//#endif
	}
#endif

	if (p_person->State != STATE_IDLE && p_person->State != STATE_GUN && p_person->State != STATE_NORMAL && p_person->State != STATE_HIT_RECOIL)
	{
		fpm = FALSE;
	}

	/*
	else
	if (p_person->Genus.Person->Action == ACTION_AIM_GUN ||
		p_person->Genus.Person->Action == ACTION_SHOOT)
	{
		fpm = TRUE;
		gun=1;
	}
	else
	if (p_person->Genus.Person->Action == ACTION_DRAW_SPECIAL)
	{
		if (p_person->Genus.Person->SpecialDraw == NULL)
		{
			//
			// Drawing the pistol.
			//

			fpm = TRUE;
		}
		else
		{
			//
			// Drawing a special.
			//

			Thing *p_special = TO_THING(p_person->Genus.Person->SpecialDraw);

			switch(p_special->Genus.Special->SpecialType)
			{
				case SPECIAL_AK47:
				case SPECIAL_SHOTGUN:
					fpm = TRUE;
					break;
			}
		}
	}
	*/

	if(fpm)
	{

		if (!look_ami)
		{
			//
			// Just gone into first person mode.
			//

			p_person->Genus.Person->Flags2|=FLAG2_PERSON_LOOK;
			look_ami    =  TRUE;
			look_pitch  = -FC_cam[p_person->Genus.Person->PlayerID-1].pitch >> 8;
			look_pitch &= 2047;

			if (look_pitch > 1024) {look_pitch -= 2048;}

			look_pitch >>= 4;
			look_pitch  &= 2047;
#ifdef	PSX
			look_pitch=0;
#endif
		}

#ifndef PSX
		if(mouse_input)
		{
			if(MouseDY)
			{
				look_pitch -= MouseDY;
			}
			if(MouseDX)
			{
				p_person->Draw.Tweened->Angle	=	(p_person->Draw.Tweened->Angle-MouseDX)&2047;
			}
		}
#endif

		if (input & INPUT_MASK_FORWARDS)
		{
			look_pitch += 13;
			input &=~ INPUT_MASK_MOVE;
		}
		if (input & INPUT_MASK_BACKWARDS)
		{
			look_pitch -= 13;
		}

		if (!CONTROLS_inventory_mode)
		{
			if (input & INPUT_MASK_LEFT)
			{
				p_person->Draw.Tweened->Angle	=	(p_person->Draw.Tweened->Angle+32)&2047;
			}

			if (input & INPUT_MASK_RIGHT)
			{
				p_person->Draw.Tweened->Angle	=	(p_person->Draw.Tweened->Angle-32)&2047;
			}
		}

		if (input & INPUT_MASK_MOVE)
		{
			set_person_running(p_person);
		}

#ifdef PSX
		if (look_pitch >  128 && look_pitch <= 1024) {look_pitch =  128;}
		if (look_pitch < 1900 && look_pitch >= 1024) {look_pitch = 1900;}
#else
		if (look_pitch >  256 && look_pitch <= 1024) {look_pitch =  256;}
		if (look_pitch < 1850 && look_pitch >= 1024) {look_pitch = 1850;}
#endif

		look_pitch &= 2047;


		FC_position_for_lookaround(p_person->Genus.Person->PlayerID-1,look_pitch);


	//	if(gun)
		if(person_has_gun_out(p_person))
		{
			if(input&INPUT_MASK_PUNCH)
			{
				if (p_person->Genus.Person->Action == ACTION_SHOOT)
				{
					/*

					//
					// Only the pistol animation can be interrupted so you
					// can shoot again.
					//

					if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
					{
						set_player_shoot(p_person,0);
						*processed|=INPUT_MASK_PUNCH; //needs a clear click
					}

					*/
				}
				else
				{
					set_player_shoot(p_person,0);
						*processed|=INPUT_MASK_PUNCH; //needs a clear click
				}
			}

			/*
			if(input&INPUT_MASK_SELECT)
			{
				set_person_gun_away(p_person);
				*processed|=INPUT_MASK_SELECT; //needs a clear click

			}
			*/
		}
	}
	else
	{
		if (look_ami)
		{
			p_person->Genus.Person->Flags2&=~FLAG2_PERSON_LOOK;

			if(CNET_network_game)
			{
				if(p_person->Genus.Person->PlayerID-1==PLAYER_ID)
				{
					FC_force_camera_behind(0);
				}
			}
			else
				FC_force_camera_behind(p_person->Genus.Person->PlayerID-1);

			look_ami   = FALSE;
			look_pitch = 0;
		}

	}

	FirstPersonMode = fpm;
	return(fpm);
}


SLONG can_darci_change_weapon(Thing *p_person)
{
	if (EWAY_stop_player_moving())
	{
		return FALSE;
	}

	if (p_person->State == STATE_IDLE)
	{
		return TRUE;
	}

	if (p_person->State == STATE_MOVEING)
	{
		if (p_person->SubState == SUB_STATE_RUNNING ||
			p_person->SubState == SUB_STATE_WALKING)
		{
			return TRUE;
		}
	}

	if (p_person->State == STATE_GUN)
	{
		if (p_person->SubState == SUB_STATE_AIM_GUN)
		{
			return TRUE;
		}
	}
	if (p_person->SubState == SUB_STATE_ITEM_AWAY)
			return TRUE;

	if (p_person->SubState == SUB_STATE_DRAW_ITEM)
			return TRUE;

	if (p_person->SubState == SUB_STATE_DRAW_GUN)
			return TRUE;

	return FALSE;
}

//
// This need renaming to process packet input for player
//


int g_iPlayerCameraMode = 0;

void	process_hardware_level_input_for_player(Thing *p_player)
{
	SLONG  i;

	ULONG  input;
	ULONG  last;
	ULONG  processed = 0;
	SLONG  tick = GetTickCount();

	Thing *p_person;
	p_person = p_player->Genus.Player->PlayerPerson;

	//
	// yuk hardware keyboard input
	//


	input = PACKET_DATA(p_player->Genus.Player->PlayerID);



	Player *pl = p_player->Genus.Player;


	//
	// do the camera stuff
	//

//	if (p_person->Genus.Person->Mode != PERSON_MODE_FIGHT)
#ifndef	PSX


#ifdef TARGET_DC
	// Saner system.
	static bool bChangeCameraModePressedLastTime = FALSE;
	if ( pl->Pressed&INPUT_MASK_CAMERA )
	{
		if ( !bChangeCameraModePressedLastTime )
		{
			bChangeCameraModePressedLastTime = TRUE;

			// Cycle through the modes. There are four, apparently.
			g_iPlayerCameraMode++;
			if ( g_iPlayerCameraMode >= 4 )
			{
				g_iPlayerCameraMode = 0;
			}

#if 0
			if(CNET_network_game)
			{
				if(p_person->Genus.Person->PlayerID-1==PLAYER_ID)
				{
					FC_change_camera_type(0,g_iPlayerCameraMode);
					FC_force_camera_behind(0);
				}
			}
			else
#else
			ASSERT (!CNET_network_game);
#endif
			{
				FC_change_camera_type(p_person->Genus.Person->PlayerID-1,g_iPlayerCameraMode);
				FC_force_camera_behind(p_person->Genus.Person->PlayerID-1);
			}
		}
	}
	else
	{
		bChangeCameraModePressedLastTime = FALSE;
	}
#else
	// Blimey! Mad system!
	if (pl->Pressed&INPUT_MASK_CAMERA)
	{
		g_iPlayerCameraMode=input&INPUT_MASKM_CAM_TYPE;
		g_iPlayerCameraMode>>=INPUT_MASKM_CAM_SHIFT;
		input&=~INPUT_MASKM_CAM_TYPE;
		if(CNET_network_game)
		{
			if(p_person->Genus.Person->PlayerID-1==PLAYER_ID)
			{
				FC_change_camera_type(0,g_iPlayerCameraMode);
				FC_force_camera_behind(0);
			}
		}
		else
		{

			FC_change_camera_type(p_person->Genus.Person->PlayerID-1,g_iPlayerCameraMode);
			FC_force_camera_behind(p_person->Genus.Person->PlayerID-1);
		}
	}
#endif
#endif

	if (input&INPUT_MASK_CAM_BEHIND)
	{

		if(CNET_network_game)
		{
			if(p_person->Genus.Person->PlayerID-1==PLAYER_ID)
			{
				FC_force_camera_behind(0);
			}
		}
		else
		{
			FC_force_camera_behind(p_person->Genus.Person->PlayerID-1);
		}
	}

	if(input&INPUT_MASK_CAM_LEFT)
	{
//			CAM_rotate_left();
		if(CNET_network_game)
		{
			if(p_person->Genus.Person->PlayerID-1==PLAYER_ID)
			{
				FC_rotate_left(p_person->Genus.Person->PlayerID-1);
			}
		}
		else
			FC_rotate_left(p_person->Genus.Person->PlayerID-1);
	}

	if(input&INPUT_MASK_CAM_RIGHT)
	{
//			CAM_rotate_right();
		if(CNET_network_game)
		{
			if(p_person->Genus.Person->PlayerID-1==PLAYER_ID)
			{
				FC_rotate_left(0);
			}
		}
		else
			FC_rotate_right(p_person->Genus.Person->PlayerID-1);
	}





	//
	// Calculate stuff useful for the fight-mode.
	//

	pl->LastInput = pl->ThisInput;
	pl->ThisInput = input;

	pl->Pressed   =  pl->ThisInput & ~pl->LastInput;  //buttons newly pressed this game turn
	pl->Released  = ~pl->ThisInput &  pl->LastInput;  //buttons released this game turn

	/*

	if (pl->Pressed & INPUT_MASK_JUMP)
	{
		CBYTE str[100];

		sprintf(str, "Pressed jump turn %d\n", GAME_TURN);

		TRACE(str);
	}

	*/

/*
	if(pl->Pressed&INPUT_MASK_BACKWARDS)
	{
		CBYTE	str[100];
		sprintf(str,"BACKWARDS Pressed %d \n",GAME_TURN);
		CONSOLE_text(str);
	}
	if(pl->Released&INPUT_MASK_BACKWARDS)
	{
		CBYTE	str[100];
		sprintf(str,"BACKWARDS rel %d \n",GAME_TURN);
		CONSOLE_text(str);
	}
*/

	if (pl->Pressed)
	{
		pl->DoneSomething = FALSE;
	}

	//
	// Work out how many clicks each key has.
	//

	for (i = 0; i < 16; i++)
	{
		if (pl->Pressed & (1 << i))
		{
			if (pl->LastReleased[i] >= tick - 200)
			{
				pl->DoubleClick[i] += 1;
			}
			else
			{
				pl->DoubleClick[i] = 1;
			}
		}
	}

	//
	// Store release clicks.
	//

	for (i = 0; i < 16; i++)
	{
		if (pl->Released & (1 << i))
		{
			pl->LastReleased[i] = tick;
		}
	}

	if(input&INPUT_MASK_ACTION)
	{
		MSG_add(" still action");

	}

	SLONG no_control = FALSE;

	extern Form *form_leave_map;
	extern SLONG form_left_map;

	if (EWAY_stop_player_moving())
	{
		//
		// We are in the middle of a cutscene...
		//

		input         = 0;
		form_left_map = 15;
	}

	if (form_leave_map)
	{
		input      = 0;
	}

	extern UBYTE draw_map_screen;

	if (draw_map_screen)
	{
		if ((input & INPUT_MASK_SELECT) && (input & INPUT_MASK_MOVE))
		{
			//
			// Pressing select in the map screen exits game.
			//

			GAME_STATE = 0;
		}

		input = 0;
	}

	if (form_left_map)
	{
		input          = 0;
		form_left_map -= 1;
	}

	//
	// If InputDone bit is set we ignore that input until it is released
	//

	//
	// clear any inputdone if the input has been released
	//

	p_player->Genus.Player->InputDone &= (input&0x3ffff);
	p_player->Genus.Player->Input      = input;

	//
	// Remove any input bits that have allready been processed
	//
//	if(input&INPUT_MASK_PUNCH)
//		CONSOLE_text(" punch pressed A",500);

	input &= ~(p_player->Genus.Player->InputDone); //&~INPUT_MASK_ACTION);

//	if(input&INPUT_MASK_PUNCH)
//		CONSOLE_text(" punch pressed",500);


	if (!no_control)
	{
		if (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING)
		{
			//
			// You can't draw weapons while in a car...
			//
		}
		else
		{
			//
			// Should Darci be allowed to change weapon?
			//

			if (can_darci_change_weapon(p_person))
			{
				if (Keys[KB_1])
				{
					Keys[KB_1] = 0;

					if ((p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT) ||
						(p_person->Genus.Person->SpecialUse))
					{
						set_person_gun_away(p_person);
					}
				}

				if (Keys[KB_2])
				{
					Keys[KB_2] = 0;

					if (!(p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT))
					{
						if (p_person->Flags & FLAGS_HAS_GUN)
						{
							if (p_person->Genus.Person->SpecialUse)
							{
								set_person_item_away(p_person);
							}

							set_person_draw_gun(p_person);
						}
						else
						{
/*
							add_damage_text(
								p_person->WorldPos.X          >> 8,
								p_person->WorldPos.Y + 0x6000 >> 8,
								p_person->WorldPos.Z          >> 8,
								"No Gun");
*/
						}
					}
				}

				SLONG  special_type = SPECIAL_NONE;

				if (Keys[KB_3]) {Keys[KB_3] = 0; special_type = SPECIAL_SHOTGUN;}
				if (Keys[KB_4]) {Keys[KB_4] = 0; special_type = SPECIAL_AK47;}
				if (Keys[KB_5]) {Keys[KB_5] = 0; special_type = SPECIAL_GRENADE;}
				if (Keys[KB_6]) {Keys[KB_6] = 0; special_type = SPECIAL_EXPLOSIVES;}
				if (Keys[KB_7]) {Keys[KB_7] = 0; special_type = SPECIAL_KNIFE;}
				if (Keys[KB_8]) {Keys[KB_8] = 0; special_type = SPECIAL_BASEBALLBAT;}

				if (special_type)
				{
					if (person_has_special(p_person, special_type))
					{
						if (p_person->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
						{
							set_person_gun_away(p_person);
						}

						set_person_draw_item(p_person, special_type);
					}
					else
					{
/*
						CBYTE str[40];

						sprintf(str, "No %s", SPECIAL_info[special_type].name);

						add_damage_text(
							p_person->WorldPos.X
							>> 8,
							p_person->WorldPos.Y + 0x6000 >> 8,
							p_person->WorldPos.Z          >> 8,
							str);
*/
					}
				}
			}
		}

		//
		// Process the person's input.
		//

		if(!apply_button_input_first_person(p_player, p_person, input,&processed))
		{

			if (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING)
			{
				ASSERT(p_person->Genus.Person->InCar);
#if 0
				Thing *p_thing=TO_THING(p_person->Genus.Person->InCar);
				SLONG vel=QDIST2(abs(p_thing->Genus.Vehicle->VelX),abs(p_thing->Genus.Vehicle->VelZ));
				// Set a minimum for tick over shock
				vel>>=8;
				SATURATE(vel,40,255);
				PSX_SetShock(0,vel);
#endif
#ifdef TARGET_DC
				if ( g_bEngineVibrations )		// user preference setting.
				{
					Thing *p_thing=TO_THING(p_person->Genus.Person->InCar);
					SLONG vel=QDIST2(abs(p_thing->Genus.Vehicle->VelX),abs(p_thing->Genus.Vehicle->VelZ));
					vel>>=8;
#if 0
					Vibrate ( (float)( vel + 24 ) * 0.15, (float)( vel + 80 ) * 0.002f, 1.0f );
#else
					// Gears!
					// Vel goes from 0 to about 180,
					// so have three gears. 0->63, 64->127, 128->up
					int iGear = vel >> 6;
					Vibrate ( (float)( vel & 0x3f ) * 0.3f + 4.0f + 2.0f * (float)iGear, (float)( vel + 80 ) * 0.002f, 1.0f );
					//TRACE ( "Engine: gear %i, revs %i\n", iGear, (vel & 0x3f) );

#endif
				}
#endif

				processed  = apply_button_input_car(TO_THING(p_person->Genus.Person->InCar), input);
				processed |= apply_button_input(p_player, p_person, 0);//input & INPUT_MASK_ACTION);
			}
#ifndef PSX
#ifdef  BIKE
			else
			if (p_person->Genus.Person->Flags & FLAG_PERSON_BIKING)
			{
				ASSERT(p_person->Genus.Person->InCar);

				processed  = apply_button_input_bike(TO_THING(p_person->Genus.Person->InCar), input);
				processed |= apply_button_input(p_player, p_person, input & INPUT_MASK_ACTION);
			}
			else
#endif
#endif
			{
				if (p_person->Genus.Person->Mode == PERSON_MODE_FIGHT)
				{
					input=pre_process_input(PERSON_MODE_FIGHT,input); //this fucks up the processed business but what the hell
					processed = apply_button_input_fight(p_player,p_person,input);
				}
				else
				{
					input=pre_process_input(PERSON_MODE_RUN,input);
					processed = apply_button_input(p_player,p_person,input);
				}
			}
		}
	}

	p_player->Genus.Player->InputDone |= processed;

	//
	// Nice friendly bit of debug code...
	//
#ifndef	FINAL
#ifndef TARGET_DC
	if(Keys[KB_0])
	{
		Keys[KB_0] = 0;

		if(ShiftFlag)
		{
			if (p_person->Draw.Tweened->PersonID > 0)
			{
				p_person->Draw.Tweened->PersonID--;
			}
		}
		else
		{
			if (p_person->Draw.Tweened->PersonID < 8)
			{
				p_person->Draw.Tweened->PersonID++;
			}
		}
	}
#endif
#endif
}


SLONG	continue_action(Thing *p_person)
{
	Thing *p_player;
	ULONG	input,input_used,new_action;
	if(p_person->Genus.Person->PlayerID)
	{
		p_player=NET_PLAYER(p_person->Genus.Person->PlayerID-1);
		input=p_player->Genus.Player->Input;
		MSG_add(" continue action %d  input %x \n",p_person->Genus.Person->Action,input);
		switch(p_person->Genus.Person->Action)
		{
			case	ACTION_FLIP_LEFT:
				new_action=find_best_action_from_tree(ACTION_IDLE,input,&input_used);
				if(new_action==ACTION_FLIP_LEFT)
					return(1);
				break;
			case	ACTION_FLIP_RIGHT:
				new_action=find_best_action_from_tree(ACTION_IDLE,input,&input_used);
				if(new_action==ACTION_FLIP_RIGHT)
					return(1);
				break;
		}
	}
	return(0);
}

SLONG continue_pressing_action(Thing *p_person)
{
	Thing *p_player;
	ULONG	input,input_used,new_action;
	if(p_person->Genus.Person->PlayerID)
	{
		p_player=NET_PLAYER(p_person->Genus.Person->PlayerID-1);
		input=p_player->Genus.Player->Input;
		if(input&INPUT_MASK_ACTION)
			return(1);
		else
			return(0);
	}
	return(1);
}


void	set_action_used(Thing *p_person)
{
	Thing *p_player;
	if(p_person->Genus.Person->PlayerID)
	{
		p_player=NET_PLAYER(p_person->Genus.Person->PlayerID-1);
		p_player->Genus.Player->InputDone|=INPUT_MASK_ACTION;
	}
}

SLONG continue_dir(Thing *p_person,SLONG dir)
{
	Thing *p_player;
	ULONG	input;
	if(p_person->Genus.Person->PlayerID)
	{
		p_player=NET_PLAYER(p_person->Genus.Person->PlayerID-1);
		input=p_player->Genus.Player->Input;
		if(dir==1)
		{
			if(input&INPUT_MASK_RIGHT)
				return(1);
		}

		if(dir==0)
		{
			if(input&INPUT_MASK_LEFT)
				return(1);
		}

	}

	return(0);
}
SLONG continue_firing(Thing *p_person)
{
	Thing *p_player;
	ULONG	input;

	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		if (p_special->Genus.Special->SpecialType == SPECIAL_AK47)
		{
			if (p_special->Genus.Special->ammo == 0)
			{
				return FALSE;
			}
		}
	}

	if(p_person->Genus.Person->PlayerID)
	{
		p_player = NET_PLAYER(p_person->Genus.Person->PlayerID-1);
		input	 = PACKET_DATA(p_player->Genus.Player->PlayerID);

		if (input & INPUT_MASK_PUNCH)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Thing *p_target;
		UWORD  i_target;

		//
		// Fire until enemy dead!
		//

		i_target = PCOM_person_wants_to_kill(p_person);

		if (i_target)
		{
			p_target = TO_THING(i_target);

			if (p_target->State == STATE_DEAD)
			{
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}

		return FALSE;
	}
}


SLONG	continue_moveing(Thing *p_person)
{
	Thing *p_player;
	ULONG	input;
	if(p_person->Genus.Person->PlayerID)
	{
		p_player=NET_PLAYER(p_person->Genus.Person->PlayerID-1);
		input=p_player->Genus.Player->Input;
		if(analogue)
		{
			SLONG	angle,dx,dy;

			dx=abs(GET_JOYX(input));
			dy=abs(GET_JOYY(input));
			if(QDIST2(dx,dy)<ANALOGUE_MIN_VELOCITY)
			{
				return(0);
			}


			if(p_person->State==STATE_JUMPING)
				return(1);


			angle=get_joy_angle(input,JOY_REL_CAMERA);

			angle-=p_person->Draw.Tweened->Angle;

			if(abs(angle)<512 || angle>2048-512)
			{
				return(1);
			}
			else
			{
/*
				if(p_person->State==STATE_JUMPING)
				{
					if(abs(angle)<512+256 || angle>2048-512-256)
					{
						return(1);
					}
				}
*/
				return(0);
			}





		}
		else
		{
			if(input&(INPUT_MASK_MOVE|INPUT_MASK_FORWARDS)) //FORWARDS|INPUT_MASK_SPRINT|INPUT_MASK_YOMP))
				return(1);
			else
				return(0);
		}
	}
	else
	{
		//
		// Where does this person want to go to? If he doesn't want
		// to go far- he'll have to not continue to move.
		//

		return PCOM_jumping_navigating_person_continue_moving(p_person);
	}
}

SLONG	continue_blocking(Thing *p_person)
{
	Thing *p_player;
	ULONG	input;
	if(p_person->Genus.Person->PlayerID)
	{
		p_player=NET_PLAYER(p_person->Genus.Person->PlayerID-1);
		input=p_player->Genus.Player->Input;
		if(input&(INPUT_MASK_BACKWARDS)) //FORWARDS|INPUT_MASK_SPRINT|INPUT_MASK_YOMP))
			return(1);
		else
		{
			return(0);
		}
	}
	else
	{
		//
		// Where does this person want to go to? If he doesn't want
		// to go far- he'll have to not continue to move.
		//

		return 0; //PCOM_jumping_navigating_person_continue_moving(p_person);
	}
}

void	remove_action_used(Thing *p_person)
{
	Thing *p_player;
	ULONG	input;
	if(p_person->Genus.Person->PlayerID)
	{
		p_player=NET_PLAYER(p_person->Genus.Person->PlayerID-1);
		p_player->Genus.Player->InputDone&=~INPUT_MASK_ACTION;
	}
}
