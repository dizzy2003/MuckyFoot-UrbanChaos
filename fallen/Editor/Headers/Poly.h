#ifndef	POLY_H
#define	POLY_H			1

#include	<d3dtypes.h>
//DEFINES

#define	POLY_FLAG_GOURAD		(1<<0)
#define	POLY_FLAG_TEXTURED		(1<<1)
#define	POLY_FLAG_MASKED		(1<<2)
#define	POLY_FLAG_SEMI_TRANS	(1<<3)
#define	POLY_FLAG_ALPHA			(1<<4)
#define	POLY_FLAG_TILED			(1<<5)
#define	POLY_FLAG_DOUBLESIDED	(1<<6)
#define	POLY_FLAG_WALKABLE		(1<<7)
//#define	POLY_FLAG_	(1<<)
#define	VM_GT			1
#define	POLY_MODES			0x3f

// STRUCTS

struct	EnginePoint //redesign this
{
	SLONG	X;			// these
	SLONG	Y;			// elements
	SLONG	TMapX;		// fixed
	SLONG	TMapY;		// by
	SLONG	Shade;		// trig function
	SLONG	X3d;
	SLONG	Y3d;
	SLONG	Z3d;
	SLONG	DistSqr;
	UWORD	padw;
	UBYTE	Flags;
	UBYTE	padb;
};

struct	MfEnginePoint //redesign this
{
	SLONG	X;			// these
	SLONG	Y;			// elements
	SLONG	Z3d;
	UBYTE	TX;		// fixed
	UBYTE	TY;		// by
	UBYTE	Shade;		// trig function
	UBYTE	padw;
};

struct	MFDXEnginePoint
{
	float		ScrX,
				ScrY,
				ScrZ,
				U,
				V;
	D3DCOLOR	Colour;
};

struct	PolyInfo
{
	UWORD	*PTexture;
	UWORD	Col;
	UWORD	DrawFlags;
	UBYTE	Page;
};


//DATA

extern	SLONG	div_table[65536];
extern	UWORD	*tmaps[];
extern	UBYTE	*pals[];
extern	UBYTE	tmap[];
extern	UBYTE	tmap2[];
extern	UBYTE	fade_tables[256*65];
extern	UBYTE	mix_map[256*256];
extern	UWORD	pal_to_16[256];
extern	struct	PolyInfo	poly_info;

// FUNCTIONS

extern	UWORD	is_it_clockwise(const struct	EnginePoint	*point1,const struct	EnginePoint *point2,const struct	EnginePoint *point3);
extern	void	my_trig(struct MfEnginePoint *p3,struct MfEnginePoint *p2,struct MfEnginePoint *p1);
extern	void	my_trigp(struct MfEnginePoint *p3,struct MfEnginePoint *p2,struct MfEnginePoint *p1);
//extern	void	my_trig(struct EnginePoint *p3,struct EnginePoint *p2,struct EnginePoint *p1);
extern	void	init_tmap(void);
extern	void	make_fade_table(UBYTE *pal);
extern	void	make_mix_map(UBYTE *pal);
extern	void	double_work_window(void);
extern	void	init_poly_system(void);
extern	void	my_quad(struct MfEnginePoint *p4,struct MfEnginePoint *p3,struct MfEnginePoint *p2,struct MfEnginePoint *p1);

extern	void	render_span8(struct	Boint *p_b,UBYTE	*ptr_screen,SLONG draw_flags);
extern	void	render_span16(struct	Boint *p_b,UBYTE	*ptr_screen,SLONG draw_flags);
extern	void	render_span32(struct	Boint *p_b,UBYTE	*ptr_screen,SLONG draw_flags);

extern	void	(*render_span)(struct	Boint *p_b,UBYTE	*ptr_screen,SLONG draw_flags);


extern	void	draw_all_spans(void);
extern	void	my_quad_noz(struct MfEnginePoint *p4,struct MfEnginePoint *p3,struct MfEnginePoint *p2,struct MfEnginePoint *p1);
extern	void	my_trig_noz(struct MfEnginePoint *p4,struct MfEnginePoint *p3,struct MfEnginePoint *p2);


#endif