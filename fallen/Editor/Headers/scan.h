#ifndef	SCAN_H
#define	SCAN_H			1

extern	void	(*scan_function)(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra);
extern	void	scan_map(void);
extern	void	scan_map_thing(SLONG	map_thing);
extern	void	scan_a_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z);

#endif