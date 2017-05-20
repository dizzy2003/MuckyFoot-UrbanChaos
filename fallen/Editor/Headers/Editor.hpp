// Editor.hpp
// Guy Simmons, 18th February 1997.

#ifndef	THE_EDITOR_HPP
#define	THE_EDITOR_HPP

#ifdef	__WINDOWS_386__
#pragma	warning	389	9 
#endif

#include	"EditorLib.h"
#pragma warning( disable : 4244)
#include	"Alert.hpp"
#include	"Anim.h"
//#include	"building.hpp"
#include	"Controls.hpp"
#include	"CtrlSet.hpp"
#include	"Map.h"
#include	"thing.h"
#include	"Edit.h"
#include	"EditMod.hpp"
#include	"engine.h"
#include	"EdUtils.h"
#include	"FileReq.hpp"
#include	"GameEd.h"
#include	"Icon.hpp"
#include	"Intrface.hpp"
#include	"KFrameEd.hpp"
#include	"KFramer.hpp"
#include	"LevelEd.hpp"
#include	"math.h"
#include	"macros.h"
#include	"c:\fallen\headers\inline.h"
#include	"c:\fallen\headers\building.h"
#include	"ModeTab.hpp"
#include	"PaintTab.hpp"
#include	"poly.h"
#include	"Prim.h"
#include	"prim_draw.h"
#include	"prim_edit.h"
#include	"Primativ.hpp"
#include	"scan.h"
#include	"scrflc.h"
#include	"ThingTab.h"
#include	"Window.hpp"
#include	"undo.hpp"
#include	"c:\fallen\headers\game.h"
//#include	"collide.hpp"


#define	EDITOR_NORMAL		(1<<0)
#define	EDITOR_RECORD		(1<<1)

extern UBYTE				editor_status;

UBYTE						editor_loop(void);

#define	ShowWorkScreen(f)	editor_show_work_screen(f)
#define	ShowWorkWindow(f)	editor_show_work_window(f)

#define	LOAD_SHARED_TEXTURES	(1<<0)
#define	LOAD_UNSHARED_TEXTURES	(1<<1)
#define	LOAD_ALL_TEXTURES		(3)


#define	FREE_SHARED_TEXTURES	(1<<0)
#define	FREE_UNSHARED_TEXTURES	(1<<1)
#define	FREE_ALL_TEXTURES		(3)

extern	void	load_game_textures(UBYTE flags);
extern	void	free_game_textures(UBYTE flags);


#endif