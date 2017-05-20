// truetype.h
//
// TrueType font handling

#include "texture.h"

// if this is defined, we use TrueType fonts, otherwise we use normal fonts
//#define TRUETYPE

// TextCommand
//
// a text command

#define MAX_TT_TEXT	(2048 - 48)

struct TextCommand
{
	char		data[MAX_TT_TEXT];	// data for the command
	int			nbytes;				// number of bytes of data
	int			nchars;				// number of chars of data
	int			x,y;				// origin x,y
	int			rx;					// right x margin
	int			scale;				// scale (256 = x1)
	ULONG		rgb;				// RGB colour
	int			command;			// command type
	int			validity;			// validity
	bool		in_cache;			// in cache?
	int			lines;				// number of lines
	int			fwidth;				// formatted width
};

enum TextCommands
{
	LeftJustify = 0,	// left-justify and word-wrap
	RightJustify,		// right-justify and word-wrap
	Centred,			// centre and word-wrap
};

enum Validity
{
	Free = 0,			// command slot is free
	Current,			// command is current
	Pending,			// command is pending free
};

// CacheLine
//
// a line of texture, 256 x <h> in size
// one per line of allocated texture RAM

struct CacheLine
{
	TextCommand*	owner;			// owning TextCommand, or NULL if free
	int				sx,sy;			// screen x,y to render to
	int				width;			// width used
	int				height;			// height used

	D3DTexture*		texture;		// mapped texture
	int				y;				// mapped y coordinate
};

//
// API
//

// init library

extern void TT_Init();
extern void TT_Term();

// call before flipping the screen

extern void PreFlipTT();

// draw text

extern int DrawTextTT(char* string, int x, int y, int rx, int scale, ULONG rgb, int command, long* width = NULL);

// get height

extern int GetTextHeightTT();