// Utils.h
// Guy Simmons, 7th October 1996.

#ifndef _MF_UTILS_H_
#define _MF_UTILS_H_

#ifdef	__WATCOMC__
#define	abs(a)				(((a)<0) ? -(a) : (a))
#endif
#define	sgn(a)				(((a)<0) ? -1 : 1)
#define	swap(a,b)			{a^=b;b^=a;a^=b;}

#define	in_range(a,min,max)	{if(a>(max))a=(max);else if(a<(min))a=(min);}
#ifndef	min
#define	min(a,b)			(((a)<(b)) ? (a) : (b))
#endif

#ifndef	max
#define	max(a,b)			(((a)>(b)) ? (a) : (b))
#endif


#endif
