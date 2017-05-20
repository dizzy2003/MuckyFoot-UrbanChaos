// EdUtils.h
// Guy Simmons, 5th April 1997.

#include "c:\fallen\headers\structs.h"
//#define	GAME_SCALE			2560.0

extern SLONG					current_element;
extern struct KeyFrameElement	*the_elements;


void	read_object_name(FILE *file_handle,CBYTE *dest_string);
void	load_multi_vue(struct	KeyFrameChunk *the_chunk,float scale);
//void	load_key_frame_chunks(KeyFrameChunk *the_chunk,CBYTE *vue_name);
void	sort_multi_object(struct KeyFrameChunk *the_chunk);
void	setup_anim_stuff(void);
void	reset_anim_stuff(void);
void	load_chunk_texture_info(KeyFrameChunk *the_chunk);
void	do_single_shot(UBYTE *screen,UBYTE *palette);
void	do_record_frame(UBYTE *screen,UBYTE *palette);
SLONG	write_pcx(CBYTE *fname,UBYTE *src,UBYTE *pal);
void	editor_show_work_screen(ULONG flags);
void	editor_show_work_window(ULONG flags);


//---------------------------------------------------------------

inline	UWORD	calc_lights(SLONG x,SLONG y,SLONG z,struct SVector *p_vect)
{
#ifdef	EDITOR
	SLONG	dx,dy,dz,dist;
	SLONG	lx,ly,lz;
	ULONG	c0;
	SLONG	total=0;
	lx=p_vect->X+x;
	ly=p_vect->Y+y;
	lz=p_vect->Z+z;

	for(c0=1;c0<next_d_light;c0++)
	{

		dx=abs(lx-d_lights[c0].X);
		dy=abs(ly-d_lights[c0].Y);
		dz=abs(lz-d_lights[c0].Z);


//		dist=QDIST3(dx,dy,dz);
		dist=dx*dx+dy*dy+dz*dz;

		if(dist==0)
			dist=1;
		if(dist<(256<<11))
			total+=(d_lights[c0].Intensity<<11)/dist;
	}
	return((UWORD)total);
#else
	return ( 0 );
#endif	
}

//---------------------------------------------------------------
