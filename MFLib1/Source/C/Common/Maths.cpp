// Maths.cpp
// Guy Simmons, 27th March 1997.

#include	<MFHeader.h>
#ifdef TARGET_DC
#include <shintr.h>
#endif


//---------------------------------------------------------------

#define LookUp				ax = AtanTable[(ax<<8)/bx];
#define	xchg(a,b)			{a^=b;b^=a;a^=b;}


SLONG	Arctan(SLONG X,SLONG Y)
{
	register SLONG		ax,bx;

	ax = X;
	if(ax)
		goto just_do_it;
	bx = Y;
	if(bx)
		goto done_it;
	return 0;
just_do_it:
	bx = Y;
done_it:
	if(ax < 0)
		goto xneg;

// x positive			
	if(bx < 0)
		goto xposyneg;

// x positive, y positive
	if(ax < bx)
		goto ppyprimary;

// ppxprimary
	xchg(ax,bx)
	LookUp
	return ax+512;
ppyprimary:
	LookUp
	return (-ax)+1024;
xposyneg: //*******************************************************************
	bx = -bx;
	if(ax < bx)
		goto pnyprimary;

// pnxprimary
	xchg(ax,bx)
	LookUp
	return (-ax)+512;
pnyprimary:
	LookUp
	return ax;
xneg:
	ax = -ax;
	if(bx < 0)
		goto xnegyneg;						
// x negative, y positive
	if(ax < bx)
		goto npyprimary;

// npxprimary
	xchg(ax,bx)
	LookUp
	return (-ax)+1536;
npyprimary:
	LookUp
	return ax+1024;

// x negative, y negative
xnegyneg:
	bx = -bx;
	if(ax < bx)
		goto nnyprimary;

// nnxprimary
	xchg(ax,bx)
	LookUp
	return ax+1536;
nnyprimary:
	LookUp
	return (-ax)+2048;					
}

//---------------------------------------------------------------


#ifndef TARGET_DC

UWORD	ini_table[]	=
{
	1,		2,		2,		4,
	5,		8,		11,		16,
	22,		32,		45,		64,
	90,		128,	181,	256,
	362,	512,	724,	1024,
	1448,	2048,	2896,	4096,
	5792,	8192,	11585,	16384,
	23170,	32768,	46340,	65535
};

#ifdef _MSC_VER
SLONG	Root(SLONG square)
{
	__asm
	{
		xor		ebx,ebx
		mov		ecx,square
		bsr		eax,ecx
		je		done_it
		movzx	ebx,ini_table[eax*2]
do_it:
		mov		eax,ecx
		xor		edx,edx
		div		ebx
		cmp		eax,ebx
		jge		done_it
		add		ebx,eax
		shr		ebx,1
		jmp		do_it
done_it:
		mov		eax,ebx
	}
}
#endif


#else //#ifndef TARGET_DC

// Just use the standard rout for the moment - it uses a fast path anyway.
SLONG Root ( SLONG square )
{
	return ( (int) sqrtf ( (float)square ) );
}

#endif //#else //#ifndef TARGET_DC


//---------------------------------------------------------------
