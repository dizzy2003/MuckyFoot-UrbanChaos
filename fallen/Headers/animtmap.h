#ifndef	ANIM_TMAP_H
#define	ANIM_TMAP_H


// defines



#define	MAX_TMAP_FRAMES	16
#define	MAX_ANIM_TMAPS	16


// structures
struct	AnimTmap
{
	UBYTE	UV[MAX_TMAP_FRAMES][4][2];
	SBYTE	Delay[MAX_TMAP_FRAMES];
	UBYTE	Page[MAX_TMAP_FRAMES];
	UWORD	Current;
	UWORD	Timer;
	UWORD	Flags;
};









// data
extern	struct	AnimTmap	anim_tmaps[MAX_ANIM_TMAPS];

// functions
extern	void	save_animtmaps(void);
extern	void	sync_animtmaps(void);
extern	void	animate_texture_maps(void);
extern	void	load_animtmaps(void);


#endif