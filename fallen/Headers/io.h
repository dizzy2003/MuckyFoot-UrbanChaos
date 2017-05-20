#ifndef		IO_H
#define		IO_H

//
// Defines
//



//
// Structs
//

struct	LoadGameThing
{
	UWORD	Type;
	UWORD	SubStype;

	SLONG	X;
	SLONG	Y;
	SLONG	Z;
	ULONG	Flags;

	UWORD	IndexOther;
	UWORD	AngleX;

	UWORD	AngleY;
	UWORD	AngleZ;

	ULONG	Dummy[4];



};

//
// Data
//

extern	CBYTE	DATA_DIR[];
extern	CBYTE	LEVELS_DIR[];
extern	CBYTE	TEXTURE_WORLD_DIR[];

//
// Functions
//


extern	void	change_extension(CBYTE	*name,CBYTE *add,CBYTE *new_name);
extern	void	load_game_map(CBYTE     *name);
extern	SLONG	load_all_prims(CBYTE	*name);
extern	SLONG	load_a_multi_prim(CBYTE *name);
extern  void	load_palette(CBYTE *palette);
extern	void	load_key_frame_chunks(KeyFrameChunk *the_chunk,CBYTE *vue_name,float shrink=1.0);
extern	SLONG	save_anim_system(struct GameKeyFrameChunk *game_chunk,CBYTE	*name);


//
// Loads the textures styles from the given world. Only set (load_editor_names) if
// you are calling this function from the editor.
//

extern void load_texture_styles(UBYTE load_editor_names, UBYTE world);

//
// Loads in the given prim object if it is not already loaded.
// Returns FALSE on failure.
//

SLONG load_prim_object(SLONG prim);

//
// Loads in all the individual prim objects.
//

void load_all_individual_prims(void);


#endif
