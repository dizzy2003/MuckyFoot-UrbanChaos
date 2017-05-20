// Trig.h
// Guy Simmons, 16th February 1997.

#ifndef	GMATHS_H
#define	GMATHS_H

#define	SIN(a)				SinTable[a]
#define	COS(a)				CosTable[a]

#define	SIN_F(a)			SinTableF[a]
#define	COS_F(a)			CosTableF[a]

//---------------------------------------------------------------

SLONG					Arctan(SLONG X,SLONG Y);
SLONG					Root(SLONG square);
#ifdef	__WATCOMC__
extern UWORD	ini_table[];
SLONG					Root(SLONG square);
#pragma aux Root =								\
		"		xor		ebx,ebx					"\
		"		bsr		eax,ecx					"\
		"		je		done_it					"\
		"		movzx	ebx,ini_table[eax*2]	"\
		"do_it:									"\
		"		mov		eax,ecx					"\
		"		xor		edx,edx					"\
		"		div		ebx						"\
		"		cmp		eax,ebx					"\
		"		jge		done_it					"\
		"		add		ebx,eax					"\
		"		shr		ebx,1					"\
		"		jmp		do_it					"\
		"done_it:								"\
		"		mov		eax,ebx					"\
				parm[ecx]						\
				modify[eax ebx ecx edx]			\
				value[eax];
#endif


//---------------------------------------------------------------

extern float	*CosTableF,
				SinTableF[];
extern SWORD	AtanTable[];
extern SLONG	*CosTable,
				SinTable[];
extern SLONG	Proportions[];


//---------------------------------------------------------------

#define	PROPTABLE_SIZE		256
#define	PROP(x)				Proportions[(x)+PROPTABLE_SIZE]

static inline SLONG		Hypotenuse(SLONG x,SLONG y)	
{											
	x	=	abs(x);
	y	=	abs(y);
	if(x>y)
		return((PROP((y<<8)/x)*x)>>13);
	else
		if(y)
			return((PROP((x<<8)/y)*y)>>13);
		else
			return(0);
}

//---------------------------------------------------------------

#endif
