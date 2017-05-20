#ifndef	PRIM_EDIT_H
#define	PRIM_EDIT_H	1

extern	void	calc_prims_screen_box(UWORD	prim,SLONG x,SLONG y,SLONG z,EdRect *rect);
extern	void	calc_prims_world_box(UWORD	prim,SLONG x,SLONG y,SLONG z,EdRect *rect);


extern	void	save_asc(UWORD building,UWORD version);
extern	void	import_tex(CBYTE *fname);
extern	void	export_tex(CBYTE *fname);

extern	SBYTE	read_asc(CBYTE *fname,SLONG scale,ULONG offset);
extern	SLONG	read_multi_asc(CBYTE *asc_name,UBYTE flag,float scale=1.0);
extern	SBYTE	read_multi_dxf(void);
extern	SBYTE	read_multi_vue(SLONG m_object);
extern	void	load_textures_for_prim(CBYTE *str,UWORD prim);
extern	void	save_textures_for_prim(CBYTE *str,UWORD prim);
extern	SWORD	SelectFlag;
extern	SWORD	SelectDrawn;
extern	void	calc_prims_screen_box(UWORD	prim,SLONG x,SLONG y,SLONG z,EdRect *rect);

//
// Saves out the given prim object. Returns FALSE on failure.
//

SLONG save_prim_object(SLONG prim);

//
// Saves out all the loaded prims as individual objects.
//

void save_all_individual_prims(void);


#endif
