#include	"editor.hpp"
#include	"game.h"
//#include	"poly.h"
//#include	"edit.h"
//#include	"math.h"

/****************************************************************************/
/* DRAW.C		- All drawing to the screen should be done here.			*/
/****************************************************************************/

//assume anti clockwise

UWORD	*tmaps[50];
UBYTE	*pals[50];

//UBYTE	tmap[256*256];
//UBYTE	tmap2[256*256];
UBYTE	fade_tables[256*65];
UBYTE	brightness[256];
UBYTE	mix_map[256*256];

SLONG	div_table[65536];   //65536/x
UWORD	yc_to_555[8][256*64];
ULONG	yc_to_888[65536];
UWORD	pal_to_16[256];
UWORD	filter_is[64];
UBYTE	filter_age[64];

// WORD/UWORD
#define	QDIV(x,d)	(d<0? ((-x*div_table[-d])>>16) : ((x*div_table[d])>>16) )
#define	QDIV64(x,d)	(d<0? ((-MUL64(x,div_table[-d]))) : ((MUL64(x,div_table[d]))) )

//#define	DEBUG_SPAN	1
#define	Z_SORT	1

static	SLONG	ASMstep_tx,ASMstep_ty,ASMstep_shade;
static	SLONG	ASMtx,ASMty,ASMshade;
static	UWORD	*ASMtext_page;
static	UWORD	*ASMpal_address;
static	UWORD	*ASMfade_page;
static	SLONG	ASMCol;
struct	PolyInfo	poly_info;


struct	PolyParameters
{
	SLONG	Y;
	SLONG	LeftX;
	SLONG	RightX;
	SLONG	LeftShade;
	SLONG	LeftTextX;
	SLONG	LeftTextY;
	SLONG	StepShade;
	SLONG	StepTextX;
	SLONG	StepTextY;
};

struct	FloatPolyParameters
{
	SLONG	Y;
	SLONG	LeftX;
	SLONG	RightX;
	SLONG	LeftShade;

	float	FLeftTextX;
	float	FLeftTextY;
	SLONG	StepShade;
	float	FStepTextX;
	float	FStepTextY;
	float	Q;
	float	StepQ;

};

struct	Boint
{
	SLONG	RightX;
	SLONG	LeftX;
	SLONG	LeftTX;
	SLONG	LeftTY;
	SLONG	LeftShade;
	SLONG	LeftZ;
	SLONG	RightZ;
	SLONG	RightTX;
	SLONG	RightTY;
	SLONG	RightShade;
	struct	Boint  *PNext;
	SLONG	DrawFlags;
//	SLONG	Y;
};

struct	Boint	*z_spans[1000];
//
// z_buffer off
//
#define	MAX_BOINT	20000

struct	Boint	boint_pool[MAX_BOINT];
UWORD	next_boint=1;
struct	Boint	*current_boint=&boint_pool[0];

SLONG	debug_y=200;
SLONG	count_find=0,count_insert=0,count_chop_lhs=0,count_chop_rhs=0;

inline ULONG	span_overlaps(struct	Boint	*new_span,struct	Boint	*old_span)
{
//	if(new_span->LeftX==old_span->LeftX&&new_span->RightX==old_span->RightX)
//		return(2);

	if(new_span->LeftX>=old_span->RightX)
//	if(new_span->RightX<=old_span->LeftX||new_span->LeftX>=old_span->RightX)
		return(0);
	else
		return(1);
/*
	SLONG	hw1,hw2,mx1,mx2;
	hw1=(new_span->RightX-new_span->LeftX)>>1;
	hw2=(old_span->RightX-old_span->LeftX)>>1;

	mx1=new_span->LeftX+hw1;
	mx1=new_span->LeftX+hw1;
*/
}

inline SLONG	spans_might_intersect(struct	Boint	*new_span,struct	Boint	*old_span)
{
/*
	if(new_span->LeftZ<old_span->RightZ)
		return(1); //new behind old 
	else
		return(2); //new behind old 
*/

	if( (new_span->RightZ<=old_span->LeftZ&&new_span->RightZ<=old_span->RightZ) &&
	    (new_span->LeftZ <=old_span->LeftZ&&new_span->LeftZ <=old_span->RightZ))
			return(1); //new behind old 

	if( (new_span->RightZ>=old_span->LeftZ&&new_span->RightZ>=old_span->RightZ) &&
	    (new_span->LeftZ >=old_span->LeftZ&&new_span->LeftZ >=old_span->RightZ))
			return(2); //new infront of old 
	return(0);
}


inline	SLONG	calc_intersection(struct	Boint	*new_span,struct	Boint	*old_span)
{
	SLONG	r,s;
	SLONG	div;

	r=(new_span->LeftZ-old_span->LeftZ)*(old_span->RightX-old_span->LeftX)-(new_span->LeftX-old_span->LeftX)*(old_span->RightZ-old_span->LeftZ);
	div=( (new_span->RightX-new_span->LeftX)*(old_span->RightZ-old_span->LeftZ)-(new_span->RightZ-new_span->LeftZ)*(old_span->RightX-old_span->LeftX) );
	if(div==0)
		return(-1);
	r=(r<<16)/div;

	s=(new_span->LeftZ-old_span->LeftZ)*(new_span->RightX-new_span->LeftX)-(new_span->LeftX-old_span->LeftX)*(new_span->RightZ-new_span->LeftZ);
	s=(r<<16)/div;

	if(r>0 && r<(1<<16) && s>0 && s<(1<<16))
	{
		return(r);
	}
	else
		return(-1);
}

inline void	clip_lhs_span(struct	Boint	*chopee,SLONG x)
{
	SLONG	ratio;
	count_chop_lhs++;
//	if(debug_y==chopee->Y)
//		LogText(" Clip LHS Span x %d %d new X %d \n",chopee->LeftX,chopee->RightX,x);
	ratio=((x-chopee->LeftX)<<16)/ (chopee->RightX-chopee->LeftX);
#ifdef	DEBUG_SPAN
			{
				SLONG	c0;
				if(chopee->LeftX<x)
					for(c0=chopee->LeftX;c0<x;c0++)
						if(c0&1)
							DrawPixelC(c0,chopee->Y,2);
				
			}
#endif

//	chopee->LeftShade=	((((chopee->RightShade-chopee->LeftShade)>>16)*ratio)>>0)+chopee->LeftShade;
//	chopee->LeftTX=	((((chopee->RightTX-chopee->LeftTX)>>16)*ratio)>>0)+chopee->LeftTX;
//	chopee->LeftTY=	((((chopee->RightTY-chopee->LeftTY)>>16)*ratio)>>0)+chopee->LeftTY;


	chopee->LeftShade=	MUL64((chopee->RightShade-chopee->LeftShade),ratio)+chopee->LeftShade;
	chopee->LeftTX=	    MUL64((chopee->RightTX-chopee->LeftTX),ratio)+chopee->LeftTX;
	chopee->LeftTY=	    MUL64((chopee->RightTY-chopee->LeftTY),ratio)+chopee->LeftTY;
	chopee->LeftZ=	    MUL64((chopee->RightZ-chopee->LeftZ),ratio)+chopee->LeftZ;
	chopee->LeftX=	x;
}
	
inline void	clip_rhs_span(struct	Boint	*chopee,SLONG x)
{
	SLONG	ratio;
	count_chop_rhs++;
 //	if(debug_y==chopee->Y)
 //		LogText(" Clip RHS Span x %d %d new X %d \n",chopee->LeftX,chopee->RightX,x);

	ratio=((x-chopee->LeftX)<<16)/ (chopee->RightX-chopee->LeftX);
#ifdef	DEBUG_SPAN
			{
				SLONG	c0;
				if(x<chopee->RightX)
					for(c0=x;c0<chopee->RightX;c0++)
						if(c0&1)
							DrawPixelC(c0,chopee->Y,1);
				
			}
#endif

	chopee->RightShade=	MUL64((chopee->RightShade-chopee->LeftShade),ratio)+chopee->LeftShade;
	chopee->RightTX=	MUL64((chopee->RightTX-chopee->LeftTX),ratio)+chopee->LeftTX;
	chopee->RightTY=	MUL64((chopee->RightTY-chopee->LeftTY),ratio)+chopee->LeftTY;
	chopee->RightZ=	    MUL64((chopee->RightZ-chopee->LeftZ),ratio)+chopee->LeftZ;
	chopee->RightX   =	x;
}	

extern	void	insert_span(struct	Boint	*span,struct	Boint	**head);

void	do_nowt(void)
{
	
}

#define	check_spans(x)	{}
// {check_spans2(x);do_nowt();}


	

ULONG	check_spans2(struct	Boint	**head)
{
/*
	struct	Boint	*p=*head;
	struct	Boint	*prev=0;
	if(debug_y!=1000)
		return(0);
	while(p)
	{
		if( (prev) && p->LeftX<=prev->LeftX)
		{

			LogText(" span error %d \n",p->Y);
			debug_y=p->Y;
			return(1);
		}
		prev=p;
		p=p->PNext;
	}
	return(0);
*/
	return(0);
}
inline void	delete_span(struct	Boint	*span,struct	Boint	**head,struct	Boint	*prev)
{
	if(prev)
	{
		prev->PNext=span->PNext;
	}
	else
		*head=span->PNext;
}

inline void	sort_add_span(struct	Boint	*span,struct	Boint	**head,struct	Boint	*prev)
{
	struct	Boint	*p=*head;
//	struct	Boint	*prev=0;
//	if(debug_y==span->Y)
//		LogText(" Sort Add Span x %d %d z %d %d \n",span->LeftX,span->RightX,span->LeftZ,span->RightZ);
	if(prev)
	{
		
		if(prev->LeftX<=span->LeftX)
		{
			p=prev->PNext;
			

		}
		else
			prev=0;
	}

	while(p)
	{
		count_insert++;
		if(span->LeftX<=p->LeftX)
		{
			if(prev)
			{
				span->PNext=prev->PNext;
				prev->PNext=span;
			}
			else
			{
		 		span->PNext=*head;
				*head=span;
			}
			return;
		}
		prev=p;
		p=p->PNext;
	}
	if(prev)
	{
		span->PNext=prev->PNext;
		prev->PNext=span;
	}
	else
	{
 		span->PNext=*head;
		*head=span;
	}
}



inline ULONG chop_span(struct	Boint	**head,struct	Boint	*prev,struct	Boint	*chopee,struct	Boint	*choper,UBYTE chop_new)
{
	if(choper->RightX>=chopee->RightX)
	{

		if(choper->LeftX<=chopee->LeftX)
		{
//			remove chopee span from link list
//			if(debug_y==chopee->Y)
//				LogText(" remove chopee completely %d \n",chop_new);
#ifdef	DEBUG_SPAN
			{
				SLONG	c0;
				if(chopee->LeftX<chopee->RightX)
					for(c0=chopee->LeftX;c0<chopee->RightX;c0++)
						DrawPixelC(c0,chopee->Y,1);
				
			}
#endif
			if(chop_new)
				return(1);
			else
				return(-1);
		}
		else
		{
			//remove rhs of chopee at point choper->LeftX
			if(chopee->LeftX==choper->LeftX)
			{
//				if(debug_y==chopee->Y)
//					LogText(" remove chopee completely bodge %d \n",chop_new);
				if(chop_new)
					return(1);
				else
					return(-1);
				
			}
			else
			{
				
//			if(debug_y==chopee->Y)
//				LogText(" remove RHS1 %d",chop_new);
			clip_rhs_span(chopee,choper->LeftX);
			}
		}
	}
	else
	{
		if(choper->LeftX>chopee->LeftX)
		{
			// remove all of choper from middle of chopee, splitting chopee into 2
#ifdef	DEBUG_SPAN
			{
				SLONG	c0;
				if(choper->LeftX<choper->RightX)
					for(c0=choper->LeftX;c0<choper->RightX;c0++)
						DrawPixelC(c0,choper->Y,2);
				
			}
#endif

			if(!chop_new)
			{
//				if(debug_y==chopee->Y)
//					LogText(" new one cuts whole in old one \n");
				//new one cuts middle out of one that existed 
				if(next_boint++>MAX_BOINT)
					return(0);
					*current_boint=*chopee;
					clip_lhs_span(current_boint,choper->RightX);
					clip_rhs_span(chopee,choper->LeftX);
					current_boint->PNext=chopee->PNext;
					chopee->PNext=choper;
					choper->PNext=current_boint;
//				chopee->PNext=current_boint;


				next_boint++;
				current_boint++;
				return(1);
			}
			else
			{
				// new span needs to be split
//				if(debug_y==chopee->Y)
//					LogText(" new span has chuck removed from middle \n");


//this appears to cause sort errors!!!!


				if(next_boint++>MAX_BOINT)
					return(0);
				*current_boint=*chopee;

				clip_rhs_span(chopee,choper->LeftX);
				clip_lhs_span(current_boint,choper->RightX);
				sort_add_span(chopee,head,prev); //span split, lhs sort added , RHS can be bolted onto thing that split us

//				current_boint->PNext=choper->PNext;
//				choper->PNext=current_boint++;
				next_boint++;
				insert_span(current_boint++,head); //ooo brave bloke doing a recursive add into a linked list
				return(1);

			}


		}
		else
		{
			//remove lhs of chopee at point choper->LeftX
				clip_lhs_span(chopee,choper->RightX);
		}
		
	}
	return(0);
}

ULONG	span_exists(struct	Boint	*span,struct	Boint	**head)
{
	struct	Boint	*p;
	SLONG	count;
	p=*head;
	count=0;
	while(p&&count++<100)
	{
		if(p==span)
		{
			count=100;
			return(1);
		}
		p=p->PNext;	
	}
	return(0);
	
}

void	show_line(struct	Boint	**head,CBYTE *str)
{
	struct	Boint	*p;
	p=*head;
	LogText("%s\n",str);
	while(p)
	{
		LogText("%x X%d..%d Z %d..%d\n",p,p->LeftX,p->RightX,p->LeftZ,p->RightZ);
		p=p->PNext;
	}
	
}


void	insert_span(struct	Boint	*span,struct	Boint	**head)
{
	struct	Boint	*p,*prev=0;
	SLONG	count;
	struct	Boint	*insert_here=0;

	span->PNext=0;
/*
	if(debug_y==span->Y)
	{

		LogText(" INSERT SPAN %x X%d %d Z %d..%d\n",span,span->LeftX,span->RightX,span->LeftZ,span->RightZ);
		show_line(head,"RASTER BEFORE");
	}
*/
	if(span->LeftX>=span->RightX)
	{
//		span->RightX=span->LeftX;
		goto	exit;
	}
	p=*head;
	if(p==0)
	{
		*head=span;
			goto	exit;
	}
#ifdef	Z_SORT
	count=0;
	while(p) //&&count++<100)
	{

		count_find++;
//		if(span->LeftX<=p->LeftX)
//			insert_here=prev;
		if(span->RightX<=p->LeftX)
		{
			
			sort_add_span(span,head,prev);
/*

			if(insert_here)
			{
				span->PNext=insert_here->PNext;
				insert_here->PNext=span;
			}
			else
			{
		 		span->PNext=*head;
				*head=span;
			}
*/
			goto	exit;
		}

		if(span_overlaps(span,p))
		{
			SLONG	done=0,type;
			//new span overlaps this one, the result will be either new one will lose a piece
			//or old one loses a piece, loseing a piece may come from the middle results in a boint split into 2
			if((type=spans_might_intersect(span,p))==0)
			{
/*
				SLONG	t;
				if( (t=calc_intersection(span,p))!=-1)
				{
					//it intersects 0<t<1<<6   along the length of span
					//throw away part of span behind p

					done=1;
				}
				else
*/
				{
					if( (span->LeftZ+span->RightZ)>>1   < (p->LeftZ+p->RightZ)>>1)
						type=1;
					else
						type=2;
					
				}
				
			}

			if(!done)
			{
				if(type==1)
				{  //new span is behind this one
					if(chop_span(head,prev,span,p,1)) //chop some off new span
					{
						//none of this span can be seen
						goto	exit;
					}
				}
				else
				{ //new span is in front of this one
					type=chop_span(head,prev,p,span,0); //chop some off old span
					if(type==-1) //chop some off old span
					{
						//need to remove current span from list
						if(prev)
						{
							prev->PNext=p->PNext;
							goto skip_calc_prev;
						}
						else
						{
							*head=p->PNext;
							goto skip_calc_prev;
						}
						
					}
					if(type==1) //span has been imbeded in the middle of this one
						goto	exit;
				}
			}


		}
		prev=p;
skip_calc_prev:;	
		p=p->PNext;	
	}
#endif
/*
	if(insert_here)
	{
		span->PNext=insert_here->PNext;
		insert_here->PNext=span;
	}
	else
	{
		sort_add_span(span,head); //,prev);
 	//	span->PNext=*head;
//		*head=span;
	}
*/
	if(prev)
	{
//		prev->PNext=span;
//		span->PNext=0; //prev->PNext;
		sort_add_span(span,head,prev);
	}
	else
	{
		span->PNext=*head;
		*head=span;
	}
exit:;
//	if(debug_y==span->Y)
//		show_line(head,"RASTER AFTER");
//	span->PNext=*head;
//	*head=span;
}




SLONG	FileSaveAt(CBYTE *name,UBYTE *ptr,ULONG size)
{
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	handle=FileCreate(name,1);
	if(handle!=FILE_OPEN_ERROR)
	{
		FileWrite(handle,(UBYTE*)ptr,size);
		FileClose(handle);
		return(0);
	}
	return(-1);
}

extern UBYTE					palette[768];

void	make_555_table(void)
{
	UBYTE	pal_no;
	SLONG	col,bright;
	SLONG	r,g,b;
	UBYTE	*pal;
//	pal=palette;

	for(pal_no=0;pal_no<8;pal_no++)
	{
		pal=pals[pal_no];
		for (col=0;col<256 ;col++ )
		{
			for (bright=0;bright<64;bright++ )
			{
				r=pal[(col*3)+0];
				g=pal[(col*3)+1];
				b=pal[(col*3)+2];

				r=(r*bright>>5);
				g=(g*bright>>5);
				b=(b*bright>>5);
				if(r>255)
					r=255;
				if(g>255)
					g=255;
				if(b>255)
					b=255;

				
				yc_to_555[pal_no][col+(bright<<8)]=the_display.GetFormattedPixel(r,g,b);  //((r>>3)<<11)|((g>>2)<<5)|((b>>3)<<0);
				//yc_to_555[pal_no][col+(bright<<8)]=((r>>3)<<11)|((g>>2)<<5)|((b>>3)<<0);
				//yc_to_888[pal_no][col+(bright<<6)]=(r<<16)|(g<<8)|(b); //0xf800; //((b>>3)<<10)|((r>>3)<<5)|((g>>3)<<0);
			}
		}
	}
	pal=pals[0];
		for(col=0;col<256 ;col++ )
		{
				r=pal[(col*3)+0];
				g=pal[(col*3)+1];
				b=pal[(col*3)+2];

				pal_to_16[col]=the_display.GetFormattedPixel(r,g,b);

		}
}

void	draw_fader(void)
{
	UWORD	*ptr,*ptr2;
	SLONG	x,y;

	ptr=(UWORD*)WorkScreen;
	for(x=0;x<256;x++)
	for(y=0;y<64;y++)
	{
		ptr[y*320+x]=yc_to_555[0][x+y*256];
	}
}

void	init_poly_system(void)
{
	SLONG	c0;
//	memset(boint_pool,0,sizeof(struct Boint)*(MAX_BOINT-1));
	for(c0=1;c0<1<<16;c0++)
	{
		div_table[c0]=(1<<16)/c0;
	}
	make_555_table();
	memset(&filter_is[0],0xff,32);
}

void	init_tmap(void)
{
/*
	FileLoadAt("data/tex01.dat",tmap);
	tmaps[0]=tmap;
	tmaps[1]=tmap;
	tmaps[2]=tmap;
	tmaps[3]=tmap;
*/
}

extern	SLONG	find_colour(UBYTE *pal,SLONG r,SLONG g,SLONG b);
/*
SLONG	find_colour(UBYTE *pal,SLONG r,SLONG g,SLONG b)
{
	SLONG	found=-1,dist=0x7fffffff,c0,dist2,tr,tg,tb;
	for(c0=0;c0<256;c0++)
	{
		tr=*pal++;
		tg=*pal++;
		tb=*pal++;

		tr-=r;
		tg-=g;
		tb-=b;

		dist2=abs(tr*tr)+abs(tg*tg)+abs(tb*tb);
		if(dist2<dist)
		{
			found=c0;
			dist=dist2;
			if(dist<8)
				return(c0);
		}
	}
	return(found);
}
*/
void	make_mix_map(UBYTE *pal)
{
	SLONG	col1,col2,r,g,b,r1,g1,b1;
	UBYTE	*p1,*p2;


	if(FileExists("data/mix.dat"))
	{
		FileLoadAt("data/mix.dat",mix_map);
	}
	else
	{
		p1=pal;
		for(col2=0;col2<256;col2++)
		{
			p2=pal;
			r1=*p1++;
			g1=*p1++;
			b1=*p1++;
			for(col1=0;col1<256;col1++)
			{
				r=*p2++;
				g=*p2++;
				b=*p2++;
				r=(r+r1)>>1;
				g=(g+g1)>>1;
				b=(b+b1)>>1;
				if(r>255)
					r=255;
				if(g>255)
					g=255;
				if(b>255)
					b=255;
				mix_map[col2*256+col1]=find_colour(pal,r,g,b);
			}
		}
		FileSaveAt("data/mix.dat",mix_map,256*256);
	}
}

void	make_fade_table(UBYTE *pal)
{
	SLONG	col,bright,r,g,b,temp_bright;
	UBYTE	*p;
	if(FileExists("data/fade.dat"))
	{
		FileLoadAt("data/fade.dat",fade_tables);
	}
	else
	{
		for(bright=0;bright<64;bright++)
		{
			p=pal;
			for(col=0;col<256;col++)
			{
				r=*p++;
				g=*p++;
				b=*p++;
										 
				if(bright<=32)
					temp_bright=bright;
				else
					temp_bright=((bright-32)<<1)+32;

				if(bright==32)
				{
					r++;
				}

				r=(r*temp_bright)>>5;
				g=(g*temp_bright)>>5;
				b=(b*temp_bright)>>5;
				if(r>255)
					r=255;
				if(g>255)
					g=255;
				if(b>255)
					b=255;
				fade_tables[bright*256+col]=find_colour(pal,r,g,b);
			}
		}
		FileSaveAt("data/fade.dat",fade_tables,64*256);
	}
}

UWORD	is_it_clockwise(const struct	MfEnginePoint	*point1,const struct	MfEnginePoint *point2,const struct	MfEnginePoint *point3)
{
	SLONG	z;
	SLONG	vx,vy,wx,wy;
	
	vx=point2->X-point1->X;
	wx=point3->X-point2->X;
	vy=point2->Y-point1->Y;
	wy=point3->Y-point2->Y;
	z=vx*wy-vy*wx;

	if(z>0)
		return	1;
	else
		return	0;
}

// eax=   Shade.0-16|-|TexY 16-24
// ebx=    TexX.0-16|-|Shade16-24
// ecx=    TexY.0-16|-|TexX 16-24

//				"	mov	eax,ASMstep_shade"\
//				"	mov	ebx,ASMstep_tx"\
//				"	mov	ecx,ASMstep_ty"\
//		ASMtx=start_text_x;

#ifdef	_MSC_VER
void	RENDER_SETUP(UBYTE *,SLONG,SLONG,SLONG,SLONG)
{
	
}
#else
void	RENDER_SETUP(UBYTE *,SLONG,SLONG,SLONG,SLONG);
#pragma aux RENDER_SETUP =\
				"	rol	eax,16"\
				"	rol	ebx,16"\
				"	rol	ecx,16"\
				"	mov	dl,cl"\
				"	mov	cl,bl"\
				"	mov	bl,al"\
				"	mov	al,dl"\
				"	mov	ASMstep_shade,eax"\
				"	mov	ASMstep_tx,ebx"\
				"	mov	ASMstep_ty,ecx"\
				"	mov	eax,ASMshade"\
				"	mov	ebx,ASMtx"\
				"	mov	ecx,ASMty"\
				parm[edi][esi][eax][ebx][ecx]\
				modify[eax ebx ecx edx]
//				value[eax]
#endif

#ifdef	_MSC_VER
void	RENDER_SETUP2(void)
{
	
}
#else
void	RENDER_SETUP2(void);
#pragma aux RENDER_SETUP2 =\
				"	rol	eax,16"\
				"	rol	ebx,16"\
				"	rol	ecx,16"\
				"	mov	dl,cl"\
				"	mov	cl,bl"\
				"	mov	bl,al"\
				"	mov	al,dl"\
				"	xor	edx,edx"\
				modify[eax ebx ecx edx]
#endif

#ifdef	_MSC_VER
void	RENDER_MSC_GT(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_shade,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		mov	eax,param_step_shade
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
		mov	ebp,ASMtext_page

lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl
		mov dl,[ebp+edx]
		inc edi
		mov dh,bl
		mov dl,fade_tables[edx]
		mov [edi],dl
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}

void	RENDER_GO_GT(void)
{
}

#else
void	RENDER_GO_GT(void);
#pragma aux RENDER_GO_GT =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	mov dl,[ebp+edx]"\
				"	inc edi"\
				"	mov dh,bl"\
				"	mov dl,fade_tables[edx]"\
				"	mov [edi],dl"\
				"	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]
//				value[eax]
#endif

#ifdef	_MSC_VER
void	RENDER_GO_COL(void)
{

}
#else
void	RENDER_GO_COL(void);
#pragma aux RENDER_GO_COL =\
				"	mov	ecx,ASMCol"\
				modify[ecx]
#endif

#ifdef	_MSC_VER
void	RENDER_MSC_G(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_shade)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		mov	eax,param_step_shade

//setup
		rol	eax,16
		mov	bl,al
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	eax,ASMshade

//setup2
		rol	eax,16
		rol	ebx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		mov	ecx,ASMCol
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		mov dh,al
		inc edi
		mov dh,bl
		mov	dl,cl
		mov dl,fade_tables[edx]
		mov [edi],dl
		dec esi
		jnz  lp
		pop	esi
		pop	edi

	}
}

void	RENDER_MSC_G16(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_shade)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		mov	eax,param_step_shade

//setup
		rol	eax,16
		mov	bl,al
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	eax,ASMshade

//setup2
		rol	eax,16
		rol	ebx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		mov	ecx,ASMCol
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		mov dh,al
		add edi,2
		mov dh,bl
		mov	dl,cl
		mov dx,yc_to_555[edx*2]
		mov [edi],dx
		dec esi
		jnz  lp
		pop	esi
		pop	edi

	}
}

#else
SLONG	RENDER_GO_G(void);
#pragma aux RENDER_GO_G =\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	inc edi"\
				"	mov ch,bl"\
				"	mov dl,fade_tables[ecx]"\
				"	mov [edi],dl"\
				"	dec esi"\
				"	jnz  lp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif


#ifdef	_MSC_VER
void	RENDER_MSC_50F(UBYTE *param_ptr_screen,SLONG param_width)
{
	
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	eax,param_width
//setup
		mov	ecx,ASMCol

//setup2

// do render loop
lp:
		inc edi
		mov ch,[edi]
//		mov [edi],cl
		mov dl,mix_map[ecx]
		mov [edi],dl
		dec eax
		jnz  lp
		pop	esi
		pop	edi
	}
}
#else
SLONG	RENDER_GO_50F(void);
#pragma aux RENDER_GO_50F =\
			    "	lp:"\
				"	inc edi"\
				"	mov [edi],cl"\
				"	dec esi"\
				"	jnz  lp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif


#ifdef	_MSC_VER
void	RENDER_MSC_F(UBYTE *param_ptr_screen,SLONG param_width)
{
	
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	eax,param_width
//setup
		mov	ecx,ASMCol

//setup2

// do render loop
lp:
		inc edi
		mov [edi],cl
		dec eax
		jnz  lp
		pop	esi
		pop	edi

	}
}
void	RENDER_MSC_F16(UBYTE *param_ptr_screen,SLONG param_width)
{
	
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	eax,param_width
//setup
		mov	ecx,ASMCol

//setup2

// do render loop
lp:
		add edi,2
		mov [edi],cx
		dec eax
		jnz  lp
		pop	esi
		pop	edi

	}
}
#else
SLONG	RENDER_GO_F(void);
#pragma aux RENDER_GO_F =\
			    "	lp:"\
				"	inc edi"\
				"	mov [edi],cl"\
				"	dec esi"\
				"	jnz  lp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif


#ifdef	_MSC_VER
void	RENDER_MSC_T16(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		xor	eax,eax
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
//		mov	ebp,ASMtext_page
		mov	ebp,ASMtext_page
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl
		mov dx,[ebp+edx*2]
		add edi,2
//		mov dh,0
		//mov dh,bl
		//mov dl,fade_tables[edx]
//		mov	ebp,ASMpal_address
//		mov dx,[ebp+edx*2]
		mov [edi],dx
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}
SLONG	RENDER_GO_T16(void)
{
	return	0;
}
#endif

#ifdef	_MSC_VER

void	RENDER_MSC_GT16(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_shade,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		mov	eax,param_step_shade
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
		mov	ebp,ASMtext_page
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl
		mov dx,[ebp+edx*2]
		add edi,2
//		mov dh,bl
//		mov	ebp,ASMfade_page
//		mov dx,[ebp+edx*2]
		mov [edi],dx
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}

void	RENDER_MSC_TGT16(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_shade,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		mov	eax,param_step_shade
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl
		mov	ebp,ASMtext_page
		and	edx,0x1f1f
		mov dx,[ebp+edx*2]
		add edi,2
//		mov dh,bl
//		mov dx,yc_to_555[edx*2]
//		mov	ebp,ASMfade_page
//		mov dx,[ebp+edx*2]
		mov [edi],dx
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}

SLONG	RENDER_GO_GT16(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_GT16(void);
#pragma aux RENDER_GO_GT16 =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	mov dl,[ebp+edx]"\
				"	add edi,2"\
				"	mov dh,bl"\
				"	mov dx,yc_to_555[edx*2]"\
				"	mov [edi],dx"\
				"	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
SLONG	RENDER_GO_GT32(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_GT32(void);
#pragma aux RENDER_GO_GT32 =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	xor	edx,edx"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	mov dl,[ebp+edx]"\
				"	add edi,4"\
				"	mov dh,bl"\
				"	mov edx,yc_to_888[edx*4]"\
				"	mov [edi],edx"\
				"	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
SLONG	RENDER_GO_TGT(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_TGT(void);
#pragma aux RENDER_GO_TGT =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			"lp:	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	and dx,0x1f1f"\
				"	mov dl,[ebp+edx]"\
				"	inc edi"\
				"	mov dh,bl"\
				"	mov dl,fade_tables[edx]"\
				"	mov [edi],dl"\
				"	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]

#endif

#ifdef	_MSC_VER
SLONG	RENDER_GO_TT(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_TT(void);
#pragma aux RENDER_GO_TT =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			"lp:	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	and dx,0x1f1f"\
				"	mov dl,[ebp+edx]"\
				"	inc edi"\
				"	mov [edi],dl"\
				"	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
void	RENDER_MSC_50GT(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_shade,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		mov	eax,param_step_shade
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
		mov	ebp,ASMtext_page
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl
		mov dl,[ebp+edx]
		inc edi
		mov dh,bl
		mov dl,fade_tables[edx]
		mov	dh,[edi]
		mov	dl,mix_map[edx]
		mov [edi],dl
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}
SLONG	RENDER_GO_50GT(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_50GT(void);
#pragma aux RENDER_GO_50GT =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			"lp:	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	mov dl,[ebp+edx]"\
				"	inc edi"\
				"	mov dh,bl"\
				"	mov dl,fade_tables[edx]"\
				"	mov dh,[edi]"\
				"	mov dl,mix_map[edx]"\
				"	mov [edi],dl"\
				"	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
void	RENDER_MSC_50T(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		xor	eax,eax
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
		mov	ebp,ASMtext_page
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl
		mov dl,[ebp+edx]
		inc edi
		mov	dh,[edi]
		mov	dl,mix_map[edx]
		mov [edi],dl
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}
SLONG	RENDER_GO_50T(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_50T(void);
#pragma aux RENDER_GO_50T =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			"lp:	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	mov dl,[ebp+edx]"\
				"	inc edi"\
				"	mov dh,[edi]"\
				"	mov dl,mix_map[edx]"\
				"	mov [edi],dl"\
				"	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
SLONG	RENDER_GO_50MGT(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_50MGT(void);
#pragma aux RENDER_GO_50MGT =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	inc edi"\
				"	mov dl,[ebp+edx]"\
				"   or	dl,dl "\
				"   jz	skip"\
				"	mov dh,bl"\
				"	mov dl,fade_tables[edx]"\
				"	mov dh,[edi]"\
				"	mov dl,mix_map[edx]"\
				"	mov [edi],dl"\
				"skip:	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
void	RENDER_MSC_MGT(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_shade,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		mov	eax,param_step_shade
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
		mov	ebp,ASMtext_page
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl
		mov dl,[ebp+edx]
		inc edi
		or	dl,dl
		jz	skip
		mov dh,bl
		mov dl,fade_tables[edx]
		mov [edi],dl
skip:
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}

void	RENDER_MSC_MGT16(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_shade,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		mov	eax,param_step_shade
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
		mov	ebp,ASMtext_page
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl

		mov dx,[ebp+edx*2]
		add edi,2
//		mov dh,bl
		or	dx,dx
		jz	skip
//		mov dx,yc_to_555[edx*2]
//		mov	ebp,ASMfade_page
//		mov dx,[ebp+edx*2]
		mov [edi],dx

/*		
		mov dl,[ebp+edx]
		add edi,2
		or	dl,dl
		jz	skip
		mov dh,bl
		mov dx,yc_to_555[edx*2]
		mov [edi],dx
*/
skip:
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}

#else
SLONG	RENDER_GO_MGT(void);
#pragma aux RENDER_GO_MGT =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	inc edi"\
				"	mov dl,[ebp+edx]"\
				"   or	dl,dl "\
				"   jz	skip"\
				"	mov dh,bl"\
				"	mov dl,fade_tables[edx]"\
				"	mov [edi],dl"\
				"skip:	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
void	RENDER_MSC_MT(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		xor	eax,eax
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
		mov	ebp,ASMtext_page
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl
		mov dl,[ebp+edx]
		inc edi
		or	dl,dl
		jz	skip
		mov [edi],dl
skip:
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}

void	RENDER_MSC_MT16(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		xor	eax,eax
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
		mov	ebp,ASMtext_page
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl


		mov dx,[ebp+edx*2]
		add edi,2
		or	dx,dx
		jz	skip
//		mov dh,0

//		mov	ebp,ASMpal_address
//		mov dx,[ebp+edx*2]
		mov [edi],dx


skip:
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}
#else
SLONG	RENDER_GO_MT(void);
#pragma aux RENDER_GO_MT =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	inc edi"\
				"	mov dl,[ebp+edx]"\
				"   or	dl,dl "\
				"   jz	skip"\
				"	mov [edi],dl"\
				"skip:	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
void	RENDER_MSC_T(UBYTE *param_ptr_screen,SLONG param_width,SLONG param_step_texx,SLONG param_step_texy)
{
	//	RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
	__asm
	{
		push	edi
		push	esi
		mov	edi,param_ptr_screen
		mov	esi,param_width
		xor	eax,eax
		mov	ebx,param_step_texx
		mov	ecx,param_step_texy
//setup
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		mov	ASMstep_shade,eax
		mov	ASMstep_tx,ebx
		mov	ASMstep_ty,ecx
		mov	eax,ASMshade
		mov	ebx,ASMtx
		mov	ecx,ASMty

//setup2
		rol	eax,16
		rol	ebx,16
		rol	ecx,16
		mov	dl,cl
		mov	cl,bl
		mov	bl,al
		mov	al,dl
		xor	edx,edx

// do render loop
		push	ebp
		mov	ebp,ASMtext_page
lp:
		add	eax,ASMstep_shade
		adc	ebx,ASMstep_tx
		adc	ecx,ASMstep_ty
		adc	eax,0
		mov dh,al
		mov dl,cl
		mov dl,[ebp+edx]
		inc edi
		//mov dh,bl
		//mov dl,fade_tables[edx]
		mov [edi],dl
		dec esi
		jnz  lp
		pop	ebp
		pop	esi
		pop	edi

	}
}
SLONG	RENDER_GO_T(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_T(void);
#pragma aux RENDER_GO_T =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	mov dl,[ebp+edx]"\
				"	inc edi"\
				"	mov [edi],dl"\
				"	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
SLONG	RENDER_GO_AMT(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_AMT(void);
#pragma aux RENDER_GO_AMT =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	inc edi"\
				"	mov dh,[ebp+edx]"\
				"   or	dh,dh "\
				"   jz	skip"\
				"	mov dl,[edi]"\
				"	mov dl,fade_tables[edx]"\
				"	mov [edi],dl"\
				"skip:	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif

#ifdef	_MSC_VER
SLONG	RENDER_GO_AT(void)
{
	return	0;
}
#else
SLONG	RENDER_GO_AT(void);
#pragma aux RENDER_GO_AT =\
				"	push	ebp"\
				"	mov	ebp,ASMtext_page"\
			    "	lp:"\
				"	add	eax,ASMstep_shade"\
				"	adc	ebx,ASMstep_tx"\
				"	adc	ecx,ASMstep_ty"\
				"	adc	eax,0"\
				"	mov dh,al"\
				"	mov dl,cl"\
				"	mov dh,[ebp+edx]"\
				"	inc edi"\
				"	mov dl,[edi]"\
				"	mov dl,fade_tables[edx]"\
				"	mov [edi],dl"\
				"	dec esi"\
				"	jnz  lp"\
				"	pop	ebp"\
				modify[edi esi eax ebx ecx edx]\
				value[eax]
#endif


//inline	void	SCAN_LINE_GT(const SLONG y,SLONG lx,SLONG rx,SLONG s1,SLONG shade_step,SLONG tx1,SLONG textx_step,SLONG ty1,SLONG texty_step)
//const SLONG y,SLONG lx,SLONG rx,SLONG s1,SLONG shade_step,SLONG tx1,SLONG textx_step,SLONG ty1,SLONG texty_step)
#define	POLY_TEXT_SHIFT	7




void	PSCAN_LINE_GT(struct	FloatPolyParameters *poly)
{
	UBYTE	*ptr;
	SLONG	width;
	struct	FloatPolyParameters 	lpoly;

	lpoly.Y        =poly->Y;        	     
	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftShade=poly->LeftShade;	     //CLIP
	lpoly.FLeftTextX=poly->FLeftTextX;	     //CLIP
	lpoly.FLeftTextY=poly->FLeftTextY;	     //CLIP
	lpoly.StepShade=poly->StepShade;	     
	lpoly.FStepTextX=poly->FStepTextX;	
	lpoly.FStepTextY=poly->FStepTextY;	
	lpoly.Q        =poly->Q;	
	lpoly.StepQ	   =poly->StepQ;	

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftShade+=poly->StepShade*(-lpoly.LeftX);
		lpoly.FLeftTextX+=poly->FStepTextX*(-lpoly.LeftX);
		lpoly.FLeftTextY+=poly->FStepTextY*(-lpoly.LeftX);
		lpoly.Q+=poly->StepQ*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"C" version
		UBYTE	col;
		while(width)
		{
			SLONG	tx,ty;
			tx=(SLONG)(lpoly.FLeftTextX/lpoly.Q);
			ty=(SLONG)(lpoly.FLeftTextY/lpoly.Q);
			if(tx<0||tx>255||ty<0||ty>255)
				tx=ty=0;

			col=ASMtext_page[(tx)+((ty<<8))];
//			*ptr++=col;
			*ptr++=fade_tables[col+((lpoly.LeftShade>>8)&0xff00)];
			width--;
			lpoly.LeftShade+=lpoly.StepShade;
			lpoly.FLeftTextX+=lpoly.FStepTextX;
			lpoly.FLeftTextY+=lpoly.FStepTextY;
			lpoly.Q        +=lpoly.StepQ;
		}
	}
}



void	SCAN_LINE_GT(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

/*
	{
		CBYTE	str[100];
		sprintf(str," %d -> %d ",poly->LeftX>>16,poly->RightX>>16);
		QuickTextC(poly->LeftX>>16,(poly->Y-280)*10,str,1);
	}
*/


//	memcpy(&lpoly,poly,sizeof(struct PolyParameters));

//	lpoly.Y        =poly->Y;        	     
	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftShade=poly->LeftShade;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP
//	lpoly.StepShade=poly->StepShade;	     
//	lpoly.StepTextX=poly->StepTextX;	
//	lpoly.StepTextY=poly->StepTextY;	

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;
	
	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftShade+=poly->StepShade*(-lpoly.LeftX);
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;
		ASMshade= lpoly.LeftShade;

		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_GT();
	}
}

void	SCAN_LINE_TGT(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftShade=poly->LeftShade;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftShade+=poly->StepShade*(-lpoly.LeftX);
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;
		ASMshade= lpoly.LeftShade;

		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_TGT();
	}
}

void	SCAN_LINE_50GT(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftShade=poly->LeftShade;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftShade+=poly->StepShade*(-lpoly.LeftX);
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;
		ASMshade= lpoly.LeftShade;

#ifdef	_MSC_VER
		RENDER_MSC_50GT(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
#else
		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_50GT();
#endif
	}
}

void	SCAN_LINE_50MGT(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftShade=poly->LeftShade;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftShade+=poly->StepShade*(-lpoly.LeftX);
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;
		ASMshade= lpoly.LeftShade;

		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_50MGT();
	}
}

void	SCAN_LINE_MGT(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftShade=poly->LeftShade;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftShade+=poly->StepShade*(-lpoly.LeftX);
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;
		ASMshade= lpoly.LeftShade;

#ifdef	_MSC_VER
		RENDER_MSC_MGT(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
#else
		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_MGT();
#endif
	}
}

void	SCAN_LINE_MT(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;

#ifdef	_MSC_VER
		RENDER_MSC_MT(ptr_screen-1,width,poly->StepTextX,poly->StepTextY);
#else
		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_MT();
#endif
	}
}


void	SCAN_LINE_T(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;

#ifdef	_MSC_VER
				RENDER_MSC_T(ptr_screen-1,width,poly->StepTextX,poly->StepTextY);
#else
		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_T();
#endif
	}
}

void	SCAN_LINE_50T(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;

#ifdef	_MSC_VER
		RENDER_MSC_50T(ptr_screen-1,width,poly->StepTextX,poly->StepTextY);
#else
		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_50T();
#endif
	}
}

void	SCAN_LINE_AT(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;

		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_AT();
	}
}

void	SCAN_LINE_AMT(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftTextX=poly->LeftTextX>>POLY_TEXT_SHIFT;	     //CLIP
	lpoly.LeftTextY=poly->LeftTextY>>POLY_TEXT_SHIFT;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftTextX+=poly->StepTextX*(-lpoly.LeftX);
		lpoly.LeftTextY+=poly->StepTextY*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	if(width)
	{  //"ASM" version

		ASMtx   = lpoly.LeftTextX;
		ASMty   = lpoly.LeftTextY;

		RENDER_SETUP(ptr_screen-1,width,poly->StepShade,poly->StepTextX,poly->StepTextY);
		RENDER_SETUP2();
		RENDER_GO_AMT();
	}
}



void	SCAN_LINE_G(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
//	lpoly.RightX   =(poly->RightX+(1<<15))>>16;	     //CLIP
	lpoly.RightX   =(poly->RightX)>>16;	     //CLIP
	lpoly.LeftShade=poly->LeftShade;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftShade+=poly->StepShade*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	while(width)
	{
		*ptr_screen++=fade_tables[poly_info.Col+((lpoly.LeftShade>>8)&0xff00)];
		width--;
		lpoly.LeftShade+=poly->StepShade;
	}
}

void	SCAN_LINE_AG(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
//	lpoly.RightX   =(poly->RightX+(1<<15))>>16;	     //CLIP
	lpoly.RightX   =(poly->RightX)>>16;	     //CLIP
	lpoly.LeftShade=poly->LeftShade;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftShade+=poly->StepShade*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	while(width)
	{
		*ptr_screen=fade_tables[*ptr_screen+((lpoly.LeftShade>>8)&0xff00)];
		ptr_screen++;
		width--;
		lpoly.LeftShade+=poly->StepShade;
	}
}

void	SCAN_LINE_50G(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
//	lpoly.RightX   =(poly->RightX+(1<<15))>>16;	     //CLIP
	lpoly.RightX   =(poly->RightX)>>16;	     //CLIP
//	lpoly.RightX   =poly->RightX>>16;	     //CLIP
	lpoly.LeftShade=poly->LeftShade;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftShade+=poly->StepShade*(-lpoly.LeftX);
		lpoly.LeftX=0;

		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	while(width)
	{
		*ptr_screen=mix_map[(*ptr_screen<<8)+fade_tables[poly_info.Col+((lpoly.LeftShade>>8)&0xff00)]];
		ptr_screen++;
		width--;
		lpoly.LeftShade+=poly->StepShade;
	}
}


void	SCAN_LINE_F(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
//	lpoly.RightX   =(poly->RightX+(1<<15))>>16;	     //CLIP
	lpoly.RightX   =(poly->RightX)>>16;	     //CLIP
//	lpoly.RightX   =poly->RightX>>16;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftX=0;
		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);

	while(width)
	{
		*ptr_screen++=poly_info.Col;
		width--;
	}
}

void	SCAN_LINE_50F(struct	PolyParameters *poly)
{
	UBYTE	*ptr_screen;
	SLONG	width;
	struct	PolyParameters lpoly;
	SLONG	col;

	lpoly.LeftX    =poly->LeftX>>16;		 //CLIP
//	lpoly.RightX   =(poly->RightX+(1<<15))>>16;	     //CLIP
	lpoly.RightX   =(poly->RightX)>>16;	     //CLIP
  //	lpoly.RightX   =poly->RightX>>16;	     //CLIP

	if(lpoly.LeftX>=WorkWindowWidth || lpoly.RightX<0)
		return;


	if(lpoly.RightX>=WorkWindowWidth)
	{
		lpoly.RightX	=	WorkWindowWidth-1;
	}

	width=lpoly.RightX-lpoly.LeftX;

	if(width<=0)
		return;

	if(lpoly.LeftX<0)
	{
		lpoly.LeftX=0;
		width=lpoly.RightX;
	}

	ptr_screen=WorkWindow+lpoly.LeftX+(WorkScreenWidth*poly->Y);
	col=poly_info.Col<<8;
	while(width)
	{
		*ptr_screen=mix_map[col+*ptr_screen];
		++ptr_screen;
		width--;
	}
}

void	SCAN_LINE_NULL(struct	PolyParameters *poly)
{
	poly_info.Col=(MouseX+MouseY)&255;
	SCAN_LINE_G(poly);
}


/*
void	SCAN_LINE_GT(struct	PolyParameter *p)
{
	UBYTE	*ptr,col,col2;
	SLONG	p,shade;
	UBYTE	*ptr_text;
	SLONG	start_text_x,textx;
	SLONG	start_text_y,texty;
	

	if(rx<0)
		return;

	p=rx-lx;
	if(p<=0)
		return;


	start_text_x=tx1;
	start_text_y=ty1;


	if(rx>=WorkWindowWidth)
	{
		rx=WorkWindowWidth;
		p=rx-lx;
		if(p<0)
			return;
	}


	if(lx<0)
	{
		s1+=shade_step*(-lx);
		start_text_x+=textx_step*(-lx);
		start_text_y+=texty_step*(-lx);
		p=rx;
		lx=0;
	}
	ptr=WorkWindow+lx+(WorkScreenWidth*y);

#ifndef	SHIP_IT	
	if((s1>>16)>=64)
		s1=63<<16;
	if(((s1+shade_step*p)>>16)>64)
		shade_step=((63<<16)-s1)/p;

	if(s1<0)
		s1=0;
	if(((s1+shade_step*p)>>16)<0)
		shade_step=(0-s1)/p;
#endif

#ifdef	KEYBOARD
//	if(!KeyOn[KB_H])
	{  //"C" version
		UBYTE	*optr;
		optr=ptr;
		while(p)
		{
			col=tmap[(start_text_x>>16)+((start_text_y>>8)&0xff00)];
			*ptr++=fade_tables[col+((s1>>8)&0xff00)];
			p--;
			s1+=shade_step;
			start_text_x+=textx_step;
			start_text_y+=texty_step;
		}
//	if(KeyOn[KB_O])
	{
		*(ptr-1)=128;
		*(optr)=128;
	}
	}
//	else
#else
	if(p)
	{  //"ASM" version

//		ASMstep_shade=shade_step;
//		ASMstep_tx=textx_step;
//		ASMstep_ty=texty_step;
		ASMtx=start_text_x;
		ASMty=start_text_y;
		ASMshade=s1;

		RENDER_SETUP(ptr-1,p,shade_step,textx_step,texty_step);
		RENDER_SETUP2();
		RENDER_GO_GT();
	}
#endif
}
*/





#define	DITHER_WIDTH	256
void	build_dither_tmap(SLONG	tx,SLONG ty,UBYTE *dest)
{
	UBYTE	*ptr_s,*ptr_d;
	SLONG	x,y;
/*
//
// no longer 8 but compatible
//
	ptr_s=&poly_info.PTexture[((tx>>5)<<5)+((ty>>5)<<5)*256];
	ptr_d=dest;

	for(y=0;y<32;y++)
	{
		*ptr_d++=*ptr_s;
		for(x=1;x<32;x++)
		{
			*ptr_d=mix_map[*ptr_s+(*(ptr_s+1))*256];
			ptr_d++;
			*ptr_d=*++ptr_s;
			ptr_d++;
		}
		*(ptr_d++)=*ptr_s;
		ptr_d+=DITHER_WIDTH*2-64;
		ptr_s+=256-31;
	}

	ptr_d=dest+DITHER_WIDTH;
	for(y=0;y<32;y++)
	{
		for(x=0;x<64;x++)
		{
			*ptr_d=mix_map[*(ptr_d+DITHER_WIDTH)+(*(ptr_d-DITHER_WIDTH)<<8)];
			ptr_d++;
		}
		ptr_d+=DITHER_WIDTH*2-64;
	}
*/
}


SLONG	find_and_use_block(SLONG *dx,SLONG *dy,SLONG id)
{
	SLONG	best=-1,best_age=2;
	SLONG	c0;
	for(c0=0;c0<16;c0++)
	{
		if(filter_is[c0]==0xffff)
		{
			best=c0;
			goto	early_out;
		}
		if(filter_age[c0]>=best_age)   //>= allows reuse this frame
		{
			best_age=filter_age[c0];
			best=c0;
		}
	}
early_out:;
	if(best>=0)
	{

		*dx=(best&3)<<6;
		*dy=(best&12)<<4;
//		LogText("Find and USE block id %x at %d age %d \n",id,best,filter_age[best]);
		filter_age[best]=1;
		filter_is[best]=id;
		return(1);
	}
//	LogText("Find failed");
	return(0);
}

static	SLONG   local_edit_turn=-1;
inline SLONG	filter_poly_tmap(struct	MfEnginePoint	*p1,struct	MfEnginePoint *p2,struct	MfEnginePoint *p3,SLONG *dx,SLONG *dy)
{
	SLONG	tx,ty,use_x,use_y;
	SLONG	id;
/*
//	LogText("FILTER POLY3 \n");

	if(local_edit_turn==-1)
	{
		SLONG	c0;
//	LogText("CLEAR LIST\n");
		for(c0=0;c0<16;c0++)
		{
			filter_age[c0]=99;
			filter_is[c0]=0xffff;
		}

	}
	if(editor_turn!=local_edit_turn)
	{
		SLONG	c0;
// 		LogText("FILTERED TMAPS TRI\n");
// 		LogText("---=----------\n");
		local_edit_turn=editor_turn;
		for(c0=0;c0<16;c0++)
		{
			filter_age[c0]++;
			if(filter_age[c0]==0)
				filter_age[c0]=255;
//			LogText("%d is %x age %d\n",c0,filter_is[c0],filter_age[c0]);
		}

	}

	tx=(p1->TX+p2->TX+p3->TX)/3;
	ty=(p1->TY+p2->TY+p3->TY)/3;
	id=(tx>>5)+((ty>>5)<<3)+(poly_info.Page<<6);
//	LogText("NEW TRI to FILTER  tx %d ty %d page %d id %x\n",tx,ty,poly_info.Page,id);
	if(find_and_use_block(&use_x,&use_y,id)==0)
		return(0);
	build_dither_tmap(tx,ty,&tmap2[use_x+(use_y<<8)]);
//	LogText("TRI build dither tx %d ty %d from use_x %d use_Y %d \n",tx,ty,use_x,use_y);
	ASMtext_page=tmap2;
	*dx=(tx>>5)<<5;
	*dy=(ty>>5)<<5;
	*dx-=use_x>>1;
	*dy-=use_y>>1;
	p1->TX=(p1->TX-*dx)<<1;
	p1->TY=(p1->TY-*dy)<<1;
	p2->TX=(p2->TX-*dx)<<1;
	p2->TY=(p2->TY-*dy)<<1;
	p3->TX=(p3->TX-*dx)<<1;
	p3->TY=(p3->TY-*dy)<<1;
*/
	return(1);
}

inline SLONG	filter_poly_tmap4(struct	MfEnginePoint	*p1,struct	MfEnginePoint *p2,struct	MfEnginePoint *p3,struct	MfEnginePoint	*p4,SLONG *dx,SLONG *dy)
{
	SLONG	tx,ty,use_x,use_y;
	SLONG	id;

/*

//	LogText("FILTER POLY4 \n");

	if(local_edit_turn==-1)
	{
		SLONG	c0;
// 		LogText("CLEAR TMAPS\n");
// 		LogText("---=----------\n");
		for(c0=0;c0<16;c0++)
		{
			
//			LogText("%d is %x age %d\n",c0,filter_is[c0],filter_age[c0]);
			filter_is[c0]=0xffff;
			filter_age[c0]=99;
		}

	}
	if(editor_turn!=local_edit_turn)
	{
		SLONG	c0;
// 		LogText("FILTERED TMAPS\n");
// 		LogText("---=----------\n");
		local_edit_turn=editor_turn;
		for(c0=0;c0<16;c0++)
		{
			
//			LogText("%d is %x age %d\n",c0,filter_is[c0],filter_age[c0]);
			filter_age[c0]++;
			if(filter_age[c0]==0)
				filter_age[c0]=255;
		}

	}

	tx=(p1->TX+p2->TX+p3->TX+p4->TX)>>2;
	ty=(p1->TY+p2->TY+p3->TY+p4->TY)>>2;
	id=(tx>>5)+((ty>>5)<<3)+(poly_info.Page<<6);
	if(find_and_use_block(&use_x,&use_y,id)==0)
		return(0);
	build_dither_tmap(tx,ty,&tmap2[use_x+(use_y<<8)]);
//	LogText("build dither tx %d ty %d from use_x %d use_Y %d \n",tx,ty,use_x,use_y);
	ASMtext_page=tmap2;
	*dx=(tx>>5)<<5;
	*dy=(ty>>5)<<5;
	*dx-=use_x>>1;
	*dy-=use_y>>1;
	p1->TX=(p1->TX-*dx)<<1;
	p1->TY=(p1->TY-*dy)<<1;
	p2->TX=(p2->TX-*dx)<<1;
	p2->TY=(p2->TY-*dy)<<1;
	p3->TX=(p3->TX-*dx)<<1;
	p3->TY=(p3->TY-*dy)<<1;
	p4->TX=(p4->TX-*dx)<<1;
	p4->TY=(p4->TY-*dy)<<1;
	*/
	return(1);
}


inline SLONG	allready_filtered(struct	MfEnginePoint	*p1,struct	MfEnginePoint *p2,struct	MfEnginePoint *p3,SLONG *dx,SLONG *dy)
{
	SLONG	c0;
	SLONG	tx,ty;
	SLONG	id;

/*

	tx=(p1->TX+p2->TX+p3->TX)/3;
	ty=(p1->TY+p2->TY+p3->TY)/3;
	id=(tx>>5)+((ty>>5)<<3)+(poly_info.Page<<6);
	for(c0=0;c0<16;c0++)
	{
		if(filter_is[c0]==id)
		{
			*dx=(tx>>5)<<5;
			*dy=(ty>>5)<<5;

			*dx-=(c0&3)<<5;
			*dy-=(c0&12)<<3;
			ASMtext_page=tmap2;
			
			p1->TX=(p1->TX-*dx)<<1;
			p1->TY=(p1->TY-*dy)<<1;
			p2->TX=(p2->TX-*dx)<<1;
			p2->TY=(p2->TY-*dy)<<1;
			p3->TX=(p3->TX-*dx)<<1;
			p3->TY=(p3->TY-*dy)<<1;
			return(1);
		}
	}		
//	LogText("TRI NOT Allready Filtered tx %d ty %x id %d page %d \n",tx,ty,id,poly_info.Page);
*/
	return(0);
}

inline SLONG	allready_filtered4(struct	MfEnginePoint	*p1,struct	MfEnginePoint *p2,struct	MfEnginePoint *p3,struct	MfEnginePoint *p4,SLONG *dx,SLONG *dy)
{
/*
	SLONG	c0;
	SLONG	tx,ty;
	SLONG	id;

	tx=(p1->TX+p2->TX+p3->TX+p4->TX)>>2;
	ty=(p1->TY+p2->TY+p3->TY+p4->TY)>>2;

	id=(tx>>5)+((ty>>5)<<3)+(poly_info.Page<<6);
	for(c0=0;c0<16;c0++)
	{
		if(filter_is[c0]==id)
		{
			*dx=(tx>>5)<<5;
			*dy=(ty>>5)<<5;

			*dx-=(c0&3)<<5;
			*dy-=(c0&12)<<3;
			ASMtext_page=tmap2;
			
			p1->TX=(p1->TX-*dx)<<1;
			p1->TY=(p1->TY-*dy)<<1;
			p2->TX=(p2->TX-*dx)<<1;
			p2->TY=(p2->TY-*dy)<<1;
			p3->TX=(p3->TX-*dx)<<1;
			p3->TY=(p3->TY-*dy)<<1;
			p4->TX=(p4->TX-*dx)<<1;
			p4->TY=(p4->TY-*dy)<<1;
			return(1);
		}
	}		
//	LogText("QUA NOT Allready Filtered tx %d ty %x id %d page %d \n",tx,ty,id,poly_info.Page);
*/
	return(0);
}

struct	FPointer
{
	void (*FunctionPointer)(struct	PolyParameters*);
};

struct	FPointer p_functions[]=
{
	{SCAN_LINE_F},	 	//0  flat
	{SCAN_LINE_G},    	//1  Gourad Shaded 
	{SCAN_LINE_T},		//2  Textured 
	{SCAN_LINE_GT},		//3  Gourad & Textured 
	{SCAN_LINE_NULL},	//4  cant have just masked 
	{SCAN_LINE_NULL},	//5           masked & gourad
	{SCAN_LINE_MT},		//6  Masked Textures
	{SCAN_LINE_MGT},	//7  Masked Gourad Textured
	{SCAN_LINE_50F},	//8  Semi Transparent flat coloured face
	{SCAN_LINE_50G},	//9  Semi Transparent Gourad face
	{SCAN_LINE_50T},	//10 Semi Transparent Textured face
	{SCAN_LINE_50GT},	//11 Semi Transparent Gourad & Textured Face
	{SCAN_LINE_NULL},	//12 50M 
	{SCAN_LINE_NULL},	//13 50MG 
	{SCAN_LINE_NULL},	//14 50MT 
	{SCAN_LINE_50MGT},	//15 Semi Transparent Masked Gourad & Textured Face

	{SCAN_LINE_NULL}, 	//16  flat
	{SCAN_LINE_AG},  	//17  Gourad Shaded 
	{SCAN_LINE_AT},		//18  Textured 
	{SCAN_LINE_NULL},	//19 Gourad & Textured 
	{SCAN_LINE_NULL},	//20 cant have just masked 
	{SCAN_LINE_NULL},	//21          masked & gourad
	{SCAN_LINE_AMT}, 	//22 Masked Textures
	{SCAN_LINE_NULL},	//23 Masked Gourad Textured
	{SCAN_LINE_NULL},	//24 Semi Transparent flat coloured face
	{SCAN_LINE_NULL},	//25 Semi Transparent Gourad face
	{SCAN_LINE_NULL},	//26 Semi Transparent Textured face
	{SCAN_LINE_NULL},	//27 Semi Transparent Gourad & Textured Face
	{SCAN_LINE_NULL},	//28 50M 
	{SCAN_LINE_NULL},	//29 50MG 
	{SCAN_LINE_NULL},	//30 50MT 
	{SCAN_LINE_NULL},	//31 Semi Transparent Masked Gourad & Textured Face

	{SCAN_LINE_F},	 	//0  TILED flat
	{SCAN_LINE_G},    	//1  TILED Gourad Shaded 
	{SCAN_LINE_T},		//2  TILED Textured 
	{SCAN_LINE_TGT},		//3  TILED Gourad & Textured 
	{SCAN_LINE_NULL},	//4  TILED cant have just masked 
	{SCAN_LINE_NULL},	//5  TILED          masked & gourad
	{SCAN_LINE_MT},		//6  TILED Masked Textures
	{SCAN_LINE_MGT},	//7  TILED Masked Gourad Textured
	{SCAN_LINE_50F},	//8  TILED Semi Transparent flat coloured face
	{SCAN_LINE_50G},	//9  TILED Semi Transparent Gourad face
	{SCAN_LINE_50T},	//10 TILED Semi Transparent Textured face
	{SCAN_LINE_50GT},	//11 TILED Semi Transparent Gourad & Textured Face
	{SCAN_LINE_NULL},	//12 TILED 50M 
	{SCAN_LINE_NULL},	//13 TILED 50MG 
	{SCAN_LINE_NULL},	//14 TILED 50MT 
	{SCAN_LINE_50MGT},	//15 TILED Semi Transparent Masked Gourad & Textured Face
	{SCAN_LINE_NULL}, 	//16 TILED  flat

	{SCAN_LINE_AG},  	//17 TILED  Gourad Shaded 
	{SCAN_LINE_AT},		//18 TILED  Textured 
	{SCAN_LINE_NULL},	//19 TILED Gourad & Textured 
	{SCAN_LINE_NULL},	//20 TILED cant have just masked 
	{SCAN_LINE_NULL},	//21 TILED          masked & gourad
	{SCAN_LINE_AMT}, 	//22 TILED Masked Textures
	{SCAN_LINE_NULL},	//23 TILED Masked Gourad Textured
	{SCAN_LINE_NULL},	//24 TILED Semi Transparent flat coloured face
	{SCAN_LINE_NULL},	//25 TILED Semi Transparent Gourad face
	{SCAN_LINE_NULL},	//26 TILED Semi Transparent Textured face
	{SCAN_LINE_NULL},	//27 TILED Semi Transparent Gourad & Textured Face
	{SCAN_LINE_NULL},	//28 TILED 50M 
	{SCAN_LINE_NULL},	//29 TILED 50MG 
	{SCAN_LINE_NULL},	//30 TILED 50MT 
	{SCAN_LINE_NULL}	//31 TILED Semi Transparent Masked Gourad & Textured Face
};

#define	PSWAP(x,y) {struct MfEnginePoint *t; t=x ; x=y ; y=t; }

inline void	bodge_textures(SLONG *dtx,SLONG *dty)
{
	if(*dtx>1<<16)
		*dtx-=1<<15;
	else
	if(*dtx<-1<<16)
		*dtx+=1<<15;

	if(*dty>1<<16)
		*dty-=1<<15;
	else
	if(*dty<-1<<16)
		*dty+=1<<15;
}


//p1 is top p2 is p1->p2 is left hand side  p1->p3 is right hand side
void	calc_steps_for_tri(struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3,SLONG dy_lhs,SLONG dy_rhs,struct PolyParameters *poly)
{
	SLONG  length,ratio;
	SLONG	dtx,dty;

	if(dy_lhs==dy_rhs)
	{
		length=p3->X-p2->X;

		if(length==0)
			length=999999;

		poly->StepShade=((p3->Shade-p2->Shade)<<14)/length;
		dtx=(p3->TX-p2->TX)<<16;
		dty=(p3->TY-p2->TY)<<16;

		bodge_textures(&dtx,&dty);

		poly->StepTextX=(dtx)/length;
		poly->StepTextY=(dty)/length;
	}
	else
	if(dy_lhs<dy_rhs)
	{
		SLONG	mid_shade,mid_textx,mid_texty,mid_x;

		ratio=(dy_rhs<<16)/dy_lhs;

		mid_x	 =((p3->X    -p1->X    )<<16)/ratio+(p1->X    );
		mid_shade=((p3->Shade-p1->Shade)<<23)/ratio+(p1->Shade<<7);
		mid_textx=((p3->TX   -p1->TX   )<<23)/ratio+(p1->TX   <<7);
		mid_texty=((p3->TY   -p1->TY   )<<23)/ratio+(p1->TY   <<7);   

		length=((mid_x)-p2->X);

		if(length==0)
			length=999999;

		dtx=(mid_textx-(p2->TX   <<7))<<9;
		dty=(mid_texty-(p2->TY   <<7))<<9;

		bodge_textures(&dtx,&dty);

		poly->StepShade=((mid_shade-(p2->Shade<<7))<<7)/length;
		poly->StepTextX=(dtx)/length;
		poly->StepTextY=(dty)/length;
	}
	else
	if(dy_lhs>dy_rhs)
	{
		SLONG	mid_shade,mid_textx,mid_texty,mid_x;

		ratio=(dy_lhs<<16)/dy_rhs;

		mid_x	 =((p2->X    -p1->X    )<<16)/ratio+(p1->X    );
		mid_shade=((p2->Shade-p1->Shade)<<23)/ratio+(p1->Shade<<7);
		mid_textx=((p2->TX   -p1->TX   )<<23)/ratio+(p1->TX   <<7);
		mid_texty=((p2->TY   -p1->TY   )<<23)/ratio+(p1->TY   <<7);   

		length=p3->X-(mid_x);

		if(length==0)
			length=999999;

		dtx=((p3->TX<<7)-mid_textx)<<9;
		dty=((p3->TY<<7)-mid_texty)<<9;

		bodge_textures(&dtx,&dty);

		poly->StepShade=(((p3->Shade<<7)-mid_shade)<<7)/length;
		poly->StepTextX=(dtx)/length;
		poly->StepTextY=(dty)/length;
	}
}

#define	can_texture_be_filtered(a,b,c)		0
#define	can_texture_be_filtered4(a,b,c,d)		0

/*
SLONG	can_texture_be_filtered(struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3)
{
	SLONG	id1,id2,id3;
	if(Keys[KB_0])
		return(0);
		

	id1=(p1->TX>>5)+((p1->TY>>5)<<3);
	id2=(p2->TX>>5)+((p2->TY>>5)<<3);
	id3=(p3->TX>>5)+((p3->TY>>5)<<3);

	if(id1==id2&&id2==id3)
		return(1);
	else
		return(0);

}
SLONG	can_texture_be_filtered4(struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3,struct MfEnginePoint *p4)
{
	SLONG	id1,id2,id3,id4;
	if(Keys[KB_0])
		return(0);
		

	id1=(p1->TX>>5)+((p1->TY>>5)<<3);
	id2=(p2->TX>>5)+((p2->TY>>5)<<3);
	id3=(p3->TX>>5)+((p3->TY>>5)<<3);
	id4=(p4->TX>>5)+((p4->TY>>5)<<3);

	if(id1==id2&&id2==id3&&id3==id4)
		return(1);
	else
		return(0);

}
*/

ULONG	calc_texture_offset(struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3)
{
	SLONG	tx,ty;
	tx=p1->TX;
	ty=p1->TY;

	if(p2->TX<tx)
		tx=p2->TX;
	if(p2->TY<ty)
		ty=p2->TY;

	if(p3->TX<tx)
		tx=p3->TX;
	if(p3->TY<ty)
		ty=p3->TY;

	tx&=0xffffffe0;
	ty&=0xffffffe0;

	return(tx+(ty<<8));

}
ULONG	calc_texture_offset4(struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3,struct MfEnginePoint *p4)
{
	SLONG	tx,ty;
	tx=p1->TX;
	ty=p1->TY;

	if(p2->TX+p2->TY<tx+ty)
	{
		tx=p2->TX;
		ty=p2->TY;
	}
	if(p3->TX+p3->TY<tx+ty)
	{
		tx=p3->TX;
		ty=p3->TY;
	}
	if(p4->TX+p4->TY<tx+ty)
	{
		tx=p4->TX;
		ty=p4->TY;
	}

	tx&=0xffffffe0;
	ty&=0xffffffe0;

	return(tx+(ty<<8));

}
//#define	FILTERING_ON	1

void	my_trig(struct MfEnginePoint *p3,struct MfEnginePoint *p2,struct MfEnginePoint *p1)
{
	SLONG	dx_lhs,dy_lhs,dx_rhs,dy_rhs,dx2,dy2;

//shades	
	SLONG	ds_lhs,ds_2;
	SLONG	dtx_lhs,dtx_2;
	SLONG	dty_lhs,dty_2;
	SLONG	flat_top=0;
#ifdef	FILTERING_ON
	SLONG	f_dx=0,f_dy=0;
	SLONG	filtered=0,can_filter=0;
#endif
	struct	PolyParameters	poly;
	struct	FPointer	raster_fill;	
//	p1->Shade=1;
	if(is_it_clockwise(p1,p2,p3))
		return;
	
#ifdef	FILTERING_ON
	if(poly_info.DrawFlags&POLY_FLAG_TEXTURED)
		can_filter=can_texture_be_filtered(p1,p2,p3);
#endif

	raster_fill.FunctionPointer=p_functions[poly_info.DrawFlags&POLY_MODES].FunctionPointer;

	ASMtext_page=poly_info.PTexture;

	if(ASMtext_page==0)
		return;

	if(poly_info.DrawFlags&POLY_FLAG_TILED)
	{
		ASMtext_page+=calc_texture_offset(p1,p2,p3);
#ifdef	FILTERING_ON
		can_filter=0;
#endif
	}

	if(p2->Y<p1->Y&&p2->Y<=p3->Y)
	{ // p2 is at top

		PSWAP(p2,p1);
		PSWAP(p2,p3); //this to remain clockwise
	}
	else
	if(p3->Y<p1->Y&&p3->Y<=p2->Y)
	{ 
	// p3 is at top

		{
			PSWAP(p3,p1);
			PSWAP(p3,p2); //this to remain clockwise
		}
	}

#ifdef	FILTERING_ON
	if(can_filter)
		filtered=allready_filtered(p1,p2,p3,&f_dx,&f_dy);
#endif

//we know p1 is the highest
	{
		dy_lhs=(p2->Y-p1->Y);
		if(dy_lhs==0) //flat top with p2,p1
		{
			SLONG	dtx,dty;
			SLONG	dist;
			flat_top=1;

			dty_lhs=0; //((p2->TY-p1->TY)<<16)/dy_lhs;
			dtx_lhs=0; //((p2->TX-p1->TX)<<16)/dy_lhs;
			ds_lhs=0; //((p2->Shade-p1->Shade))/dy_lhs;
			dx_lhs=0;
			dist=p1->X-p2->X;
			if(dist)
			{
				dtx=(p1->TX-p2->TX)<<16;
				dty=(p1->TY-p2->TY)<<16;

				bodge_textures(&dtx,&dty);

				poly.StepShade=((p1->Shade-p2->Shade)<<14)/dist;
				poly.StepTextX=(dtx)/dist;
				poly.StepTextY=(dty)/dist;
			}
			

		}
		else
		{
//			dty_lhs=((p2->TY-p1->TY)<<24)/dy_lhs;
//			dtx_lhs=((p2->TX-p1->TX)<<24)/dy_lhs;
			ds_lhs=((p2->Shade-p1->Shade)<<14)/dy_lhs;
			dx_lhs=((p2->X-p1->X)<<16)/dy_lhs;
		}

		dy_rhs=(p3->Y-p1->Y);
		if(dy_rhs==0)  //flat top with p3,p1
		{
			SLONG	dtx,dty;
			SLONG	dist;

			dx_rhs=0;

			flat_top=1;
			dist=p1->X-p3->X;
			if(dist)
			{
				dtx=(p1->TX-p3->TX)<<16;
				dty=(p1->TY-p3->TY)<<16;

				bodge_textures(&dtx,&dty);

				poly.StepShade=((p1->Shade-p3->Shade)<<14)/dist;
				poly.StepTextX=(dtx)/dist;
				poly.StepTextY=(dty)/dist;
			}
		}
		else
		{
			dx_rhs=((p3->X-p1->X)<<16)/dy_rhs;
		}

		poly.LeftShade=p1->Shade<<14;
		poly.LeftX=(p1->X<<16);//+(1<<15);
		poly.Y=p1->Y;

		if(!flat_top)
			calc_steps_for_tri(p1,p2,p3,dy_lhs,dy_rhs,&poly);

#ifdef	FILTERING_ON
/*-----------**
** FILTERING **
**-----------*/
	if(can_filter)
			if(!filtered)
//			if(Keys[KB_B])
//			if(abs(poly.StepTextX)< (1<<10) || abs(poly.StepTextY)< (1<<10))
			{
				filtered=filter_poly_tmap(p1,p2,p3,&f_dx,&f_dy);
				if(!flat_top&&filtered)
					calc_steps_for_tri(p1,p2,p3,dy_lhs,dy_rhs,&poly);

				if(dy_lhs==0)
				{
					SLONG	dtx,dty,dist;
					dist=p1->X-p2->X;
					if(dist)
					{
						dtx=(p1->TX-p2->TX)<<16;
						dty=(p1->TY-p2->TY)<<16;

						bodge_textures(&dtx,&dty);

						poly.StepShade=((p1->Shade-p2->Shade)<<14)/dist;
						poly.StepTextX=(dtx)/dist;
						poly.StepTextY=(dty)/dist;
					}
				}
			}
#endif

			if(dy_lhs!=0)
			{
				dty_lhs=((p2->TY-p1->TY)<<23)/dy_lhs;
				dtx_lhs=((p2->TX-p1->TX)<<23)/dy_lhs;
			}

//************************ END OF FILTERING

//texturex step along sides
		poly.LeftTextX=p1->TX<<23;
//texturey step along sides
		poly.LeftTextY=p1->TY<<23;



		if(dy_lhs<dy_rhs)   //LHS runs out before RHS
		{
			SLONG	temp_dx;

			dy2=dy_rhs-dy_lhs;  //how far left to go
			if(dy2==0)
				dy2=1;

			temp_dx=(p3->X-p2->X)<<16;
			dx2=(temp_dx)/dy2;
			ds_2=((p3->Shade-p2->Shade)<<14)/dy2;
			dtx_2=((p3->TX-p2->TX)<<23)/dy2;
			dty_2=((p3->TY-p2->TY)<<23)/dy2;
		}
		else
		{               //RHS shorter than LHS

			dy2=dy_lhs-dy_rhs;
			if(dy2==0)
				dy2=1;
			dx2=((p2->X-p3->X)<<16)/dy2;

		}
	}
#ifdef	DEBUG_POLY
	{
		CBYTE	str[100];
		sprintf(str,"stx %d sty %d tdx %d tdy %d",stx,sty,dtx_lhs,dty_lhs);
		QuickText((p2->X+p1->X+p3->X)/3,380,str,1);
	}
#endif
//now scan the edges and render between edges
	{
		SLONG	count;
//		CBYTE	str[100];

		poly.RightX=poly.LeftX;

		count=MIN(dy_rhs,dy_lhs);
//	sprintf(str,"count %d,dy_lhs %d,dy_rhs %d",count,dy_lhs,dy_rhs);
//	draw_text(80,0,str,32);

		if(count!=0) //if not flat top
		{
			if(poly.Y<0) // if clipped off top of screen
			{
				poly.Y=-poly.Y; // poly.Y is lines to skip
				if(count>=poly.Y)
				{	// will be on screen before this count finishes
					poly.LeftX+=dx_lhs*poly.Y;
					poly.RightX+=dx_rhs*poly.Y;
					poly.LeftShade+=ds_lhs*poly.Y;

					poly.LeftTextX+=dtx_lhs*poly.Y;
					poly.LeftTextY+=dty_lhs*poly.Y;
					count-=poly.Y;
					poly.Y=0;

				}
				else
				{
					poly.LeftX+=dx_lhs*count;
					poly.RightX+=dx_rhs*count;
					poly.LeftShade+=ds_lhs*count;

					poly.LeftTextX+=dtx_lhs*count;
					poly.LeftTextY+=dty_lhs*count;
					poly.Y=-poly.Y+count;
					count=0;
				}
			}
				if(poly.Y>=WorkWindowHeight)
					goto clip_out;

			//do the top part of the triangle
			while(count>0)
			{
				raster_fill.FunctionPointer(&poly);
//				SCAN_LINE_GT(&poly);
//				SCAN_LINE_GT(poly.Y,poly.LeftX>>16,poly.RightX>>16,poly.LeftShade,poly.StepShade,poly.LeftTextX>>8,poly.StepTextX,poly.LeftTextY>>POLY_TEXT_SHIFT,poly.StepTextY);
				poly.LeftX+=dx_lhs;
				poly.RightX+=dx_rhs;
				poly.LeftShade+=ds_lhs;

				poly.LeftTextX+=dtx_lhs;
				poly.LeftTextY+=dty_lhs;
				poly.Y++;
				count--;
				if(poly.Y>=WorkWindowHeight)
					goto clip_out;
			}
		}
		else
		{
//			flat top
//			ft=1;
			if(p2->Y==p1->Y)
			{
				poly.LeftTextY=p2->TY<<23;
				poly.LeftTextX=p2->TX<<23;
				poly.LeftShade=p2->Shade<<14;
				poly.LeftX=(p2->X<<16);//+(1<<15);

				poly.RightX=(p1->X<<16);//+(1<<15);
			}
			else
			{
				poly.LeftTextY=p1->TY<<23;
				poly.LeftTextX=p1->TX<<23;
				poly.LeftShade=p1->Shade<<14;
				poly.LeftX=(p1->X<<16);//+(1<<15);

				poly.RightX=(p3->X<<16);//+(1<<15);

			}


		}

		//setup for bottom half of tri
		if(dy_lhs<dy_rhs)
		{
		//lhs kinks

			count=dy_rhs-dy_lhs;
			dx_lhs=dx2;
			ds_lhs=ds_2;
			dtx_lhs=dtx_2;
			dty_lhs=dty_2;

			poly.LeftX=p2->X<<16;

			poly.LeftTextX=p2->TX<<23;
			poly.LeftTextY=p2->TY<<23;

		}
		else
		if(dy_lhs>dy_rhs)
		{
		//rhs kinks

			count=dy_lhs-dy_rhs;
			dx_rhs=dx2;
			poly.RightX=p3->X<<16;
		}

		//clip
		if(poly.Y<0)
		{
			poly.Y=-poly.Y; // poly.Y is lines to skip
			if(count>=poly.Y)
			{	// will be on screen before this count finishes
				poly.LeftX+=dx_lhs*poly.Y;
				poly.RightX+=dx_rhs*poly.Y;
				poly.LeftShade+=ds_lhs*poly.Y;

				poly.LeftTextX+=dtx_lhs*poly.Y;
				poly.LeftTextY+=dty_lhs*poly.Y;
				count-=poly.Y;
				poly.Y=0;
			}
			else
				goto clip_out;
		}
		if(poly.Y>=WorkWindowHeight)
			goto clip_out;


		while(count>0)
		{
  			raster_fill.FunctionPointer(&poly);
//			SCAN_LINE_GT(&poly);

			poly.LeftX+=dx_lhs;
			poly.RightX+=dx_rhs;
			poly.LeftShade+=ds_lhs;
			poly.LeftTextX+=dtx_lhs;
			poly.LeftTextY+=dty_lhs;
			poly.Y++;
			count--;
 			if(poly.Y>=WorkWindowHeight)
				goto clip_out;
		}
	}

clip_out:;

#ifdef	FILTERING_ON
	if(filtered)
	{
		p1->TX=(p1->TX>>1)+f_dx;
		p1->TY=(p1->TY>>1)+f_dy;
		p2->TX=(p2->TX>>1)+f_dx;
		p2->TY=(p2->TY>>1)+f_dy;
		p3->TX=(p3->TX>>1)+f_dx;
		p3->TY=(p3->TY>>1)+f_dy;
	}
#endif
//		p1->Shade=128;
}


//p1 is top p2 is p1->p2 is left hand side  p1->p3 is right hand side
void	pcalc_steps_for_tri(struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3,SLONG dy_lhs,SLONG dy_rhs,struct FloatPolyParameters *poly)
{
	SLONG  length,ratio;
	float	dtx,dty,dq,q1,q2,q3;
	q1=1.0/(float)p1->Z3d;
	q2=1.0/(float)p2->Z3d;
	q3=1.0/(float)p3->Z3d;

	if(dy_lhs==dy_rhs)
	{
		length=p3->X-p2->X;

		if(length==0)
			length=999999;

		poly->StepShade=((p3->Shade-p2->Shade)<<14)/length;
		dtx=((float)p3->TX/(float)p3->Z3d-(float)p2->TX/(float)p2->Z3d);
		dty=((float)p3->TY/(float)p3->Z3d-(float)p2->TY/(float)p2->Z3d);
		
		poly->StepQ=(q3-q2)/(float)length;
		poly->FStepTextX=(dtx)/(float)length;
		poly->FStepTextY=(dty)/(float)length;
	}
	else
	if(dy_lhs<dy_rhs)
	{
		SLONG	mid_shade,mid_x;
		float	mid_q,ratiof,mid_textx,mid_texty;


		ratio=(dy_rhs<<16)/dy_lhs;
		ratiof=(float)dy_rhs/(float)dy_lhs;

		mid_x	 =((p3->X    -p1->X    )<<16)/ratio+(p1->X    );
		mid_shade=((p3->Shade-p1->Shade)<<23)/ratio+(p1->Shade<<7);
//		mid_z	 =((p3->Z3d  -p1->Z3d  )<<16)/ratio+(p1->Z3d);
//		mid_textx=((p3->TX   -p1->TX   )<<23)/ratio+(p1->TX   <<7);
//		mid_texty=((p3->TY   -p1->TY   )<<23)/ratio+(p1->TY   <<7);   

		mid_textx=((float)(p3->TX   -p1->TX   ))/ratiof+((float)p1->TX);
		mid_texty=((float)(p3->TY   -p1->TY   ))/ratiof+((float)p1->TY);   

		mid_q    =(q3       -q1       )/ratiof+(q1);
		length=(mid_x)-p2->X;

		if(length==0)
			length=999999;

		dtx=mid_textx*mid_q-((float)p2->TX/(float)p2->Z3d   );
		dty=mid_texty*mid_q-((float)p2->TY/(float)p2->Z3d   );

//		dtx=((float)(mid_textx>>7)/(float)mid_z-((float)p2->TX/(float)p2->Z3d   ));
//		dty=((float)(mid_texty>>7)/(float)mid_z-((float)p2->TY/(float)p2->Z3d   ));

		poly->StepQ=(mid_q-q2)/length;
		poly->StepShade=((mid_shade-(p2->Shade<<7))<<7)/length;
		poly->FStepTextX=(dtx)/length;
		poly->FStepTextY=(dty)/length;
	}
	else
	if(dy_lhs>dy_rhs)
	{
		SLONG	mid_shade,mid_x;
//		SLONG	mid_z;
		float	mid_textx,mid_texty;
		float	mid_q,ratiof;

		ratio=(dy_lhs<<16)/dy_rhs;
		ratiof=(float)dy_lhs/(float)dy_rhs;

		mid_x	 =((p2->X    -p1->X    )<<16)/ratio+(p1->X    );
		mid_shade=((p2->Shade-p1->Shade)<<23)/ratio+(p1->Shade<<7);
//		mid_z	 =((p2->Z3d  -p1->Z3d  )<<16)/ratio+(p1->Z3d);
		mid_textx=((float)(p2->TX - p1->TX))/ratiof+(p1->TX);
		mid_texty=((float)(p2->TY - p1->TY))/ratiof+(p1->TY);   
		mid_q    =(q2        -q1       )/ratiof+(q1);

		length=p3->X-(mid_x);

		if(length==0)
			length=999999;

		dtx=((float)p3->TX/(float)p3->Z3d   )-mid_textx*mid_q;
		dty=((float)p3->TY/(float)p3->Z3d   )-mid_texty*mid_q;

//		dtx=(((float)p3->TX/(float)p3->Z3d)-(float)mid_textx/(float)mid_z);
//		dty=(((float)p3->TY/(float)p3->Z3d)-(float)mid_texty/(float)mid_z);

		poly->StepQ=(q3-mid_q)/length;
		poly->StepShade=(((p3->Shade<<7)-mid_shade)<<7)/length;
		poly->FStepTextX=(dtx)/length;
		poly->FStepTextY=(dty)/length;
	}
}
#undef	FILTERING_ON

void	my_trigp(struct MfEnginePoint *p3,struct MfEnginePoint *p2,struct MfEnginePoint *p1)
{
	SLONG	dx_lhs,dy_lhs,dx_rhs,dy_rhs,dx2,dy2;
	
//shades	
	SLONG	ds_lhs,ds_2;
	float	dtx_lhs,dty_lhs;
	float	dtx_2,dty_2;

	float	q_lhs,dq_lhs,dq2,q1,q2,q3;

	SLONG	flat_top=0;
	SLONG	f_dx=0,f_dy=0;
#ifdef	FILTERING_ON
	SLONG	filtered=0,can_filter=0;
#endif
	struct	FloatPolyParameters	poly;
//	struct	FPointer	raster_fill;	
//	p1->Shade=1;

	if(is_it_clockwise(p1,p2,p3))
		return;
	
#ifdef	FILTERING_ON
	if(poly_info.DrawFlags&POLY_FLAG_TEXTURED)
		can_filter=can_texture_be_filtered(p1,p2,p3);
#endif
//	raster_fill.FunctionPointer=p_functions[poly_info.DrawFlags&POLY_MODES].FunctionPointer;

	ASMtext_page=poly_info.PTexture;
	if(ASMtext_page==0)
		return;

	if(poly_info.DrawFlags&POLY_FLAG_TILED)
	{
		ASMtext_page+=calc_texture_offset(p1,p2,p3);
#ifdef	FILTERING_ON
		can_filter=0;
#endif
	}

	if(p2->Y<p1->Y&&p2->Y<=p3->Y)
	{ // p2 is at top

		PSWAP(p2,p1);
		PSWAP(p2,p3); //this to remain clockwise
	}
	else
	if(p3->Y<p1->Y&&p3->Y<=p2->Y)
	{ 
	// p3 is at top

		{
			PSWAP(p3,p1);
			PSWAP(p3,p2); //this to remain clockwise
		}
	}


	q1=1.0/(float)p1->Z3d;
	q2=1.0/(float)p2->Z3d;
	q3=1.0/(float)p3->Z3d;

#ifdef	FILTERING_ON
	if(can_filter)
		filtered=allready_filtered(p1,p2,p3,&f_dx,&f_dy);
#endif

//we know p1 is the highest
	{
		dy_lhs=(p2->Y-p1->Y);
		if(dy_lhs==0) //flat top with p2,p1
		{
			float	dtx,dty;
			SLONG	dist;
			flat_top=1;

			dty_lhs=0; //((p2->TY-p1->TY)<<16)/dy_lhs;
			dtx_lhs=0; //((p2->TX-p1->TX)<<16)/dy_lhs;
			ds_lhs=0; //((p2->Shade-p1->Shade))/dy_lhs;
			dx_lhs=0;
			dq_lhs=0;
			dist=p1->X-p2->X;
			if(dist)
			{
				dtx=((float)p1->TX/(float)p1->Z3d-(float)p2->TX/(float)p2->Z3d);
				dty=((float)p1->TY/(float)p1->Z3d-(float)p2->TY/(float)p2->Z3d);

				poly.StepShade=((p1->Shade-p2->Shade)<<14)/dist;
				poly.FStepTextX=((dtx)/(float)dist);
				poly.FStepTextY=((dty)/(float)dist);
			}
		}
		else
		{
//			dty_lhs=((p2->TY-p1->TY)<<24)/dy_lhs;
//			dtx_lhs=((p2->TX-p1->TX)<<24)/dy_lhs;
			ds_lhs=((p2->Shade-p1->Shade)<<14)/dy_lhs;
			dx_lhs=((p2->X-p1->X)<<16)/dy_lhs;
			dq_lhs=((q2-q1))/dy_lhs;

		}

		dy_rhs=(p3->Y-p1->Y);
		if(dy_rhs==0)  //flat top with p3,p1
		{
			float	dtx,dty;
			SLONG	dist;

			dx_rhs=0;

			flat_top=1;
			dist=p1->X-p3->X;
			if(dist)
			{
				dtx=((float)p1->TX/(float)p1->Z3d)-((float)p3->TX/(float)p3->Z3d);
				dty=((float)p1->TY/(float)p1->Z3d)-((float)p3->TY/(float)p3->Z3d);

				poly.StepShade=((p1->Shade-p3->Shade)<<14)/dist;
				poly.FStepTextX=((dtx)/(float)dist);
				poly.FStepTextY=((dty)/(float)dist);
			}
		}
		else
		{
			dx_rhs=((p3->X-p1->X)<<16)/dy_rhs;
		}

		poly.LeftShade=p1->Shade<<14;
		poly.LeftX=(p1->X<<16);//+(1<<15);
		poly.Q=q1;
		poly.Y=p1->Y;

		if(!flat_top)
			pcalc_steps_for_tri(p1,p2,p3,dy_lhs,dy_rhs,&poly);

#ifdef	FILTERING_ON
/*-----------**
** FILTERING **
**-----------*/
	if(can_filter)
			if(!filtered)
//			if(Keys[KB_B])
//			if(abs(poly.StepTextX)< (1<<10) || abs(poly.StepTextY)< (1<<10))
			{
				filtered=filter_poly_tmap(p1,p2,p3,&f_dx,&f_dy);
				if(!flat_top&&filtered)
					pcalc_steps_for_tri(p1,p2,p3,dy_lhs,dy_rhs,&poly);

				if(dy_lhs==0)
				{
					SLONG	dist;
					float	dtx,dty;
					dist=p1->X-p2->X;
					if(dist)
					{
						dtx=((float)p1->TX/(float)p1->Z3d-(float)p2->TX/(float)p2->Z3d);
						dty=((float)p1->TY/(float)p1->Z3d-(float)p2->TY/(float)p2->Z3d);

						poly.StepShade=((p1->Shade-p2->Shade)<<14)/dist;
						poly.FStepTextX=(float)((dtx)/dist);
						poly.FStepTextY=(float)((dty)/dist);
					}
				}
			}
#endif

			if(dy_lhs!=0)
			{
				float	dtx,dty;
				dtx=((float)p2->TX/(float)p2->Z3d-(float)p1->TX/(float)p1->Z3d);
				dty=((float)p2->TY/(float)p2->Z3d-(float)p1->TY/(float)p1->Z3d);
				dtx_lhs=(float)(dtx)/dy_lhs;
				dty_lhs=(float)(dty)/dy_lhs;
			}

//************************ END OF FILTERING

//texturex step along sides
		poly.FLeftTextX=(float)p1->TX/(float)p1->Z3d;
//texturey step along sides
		poly.FLeftTextY=(float)p1->TY/(float)p1->Z3d;



		if(dy_lhs<dy_rhs)   //LHS runs out before RHS
		{
			SLONG	temp_dx;
			float	dtx,dty;

			dy2=dy_rhs-dy_lhs;  //how far left to go
			if(dy2==0)
				dy2=1;

			temp_dx=(p3->X-p2->X)<<16;
			dx2=(temp_dx)/dy2;
			dq2=(q3-q2)/dy2;
			ds_2=((p3->Shade-p2->Shade)<<14)/dy2;

			dtx=((float)p3->TX/(float)p3->Z3d-(float)p2->TX/(float)p2->Z3d);
			dty=((float)p3->TY/(float)p3->Z3d-(float)p2->TY/(float)p2->Z3d);
			dtx_2=(dtx)/dy2;
			dty_2=(dty)/dy2;
		}
		else
		{               //RHS shorter than LHS

			dy2=dy_lhs-dy_rhs;
			if(dy2==0)
				dy2=1;
			dx2=((p2->X-p3->X)<<16)/dy2;
			dq2=(q2-q3)/dy2;

		}
	}
//now scan the edges and render between edges
	{
		SLONG	count;
//		CBYTE	str[100];

		poly.RightX=poly.LeftX;

		count=MIN(dy_rhs,dy_lhs);
//	sprintf(str,"count %d,dy_lhs %d,dy_rhs %d",count,dy_lhs,dy_rhs);
//	draw_text(80,0,str,32);

		if(count!=0) //if not flat top
		{
			if(poly.Y<0) // if clipped off top of screen
			{
				poly.Y=-poly.Y; // poly.Y is lines to skip
				if(count>=poly.Y)
				{	// will be on screen before this count finishes
					poly.LeftX+=dx_lhs*poly.Y;
					poly.RightX+=dx_rhs*poly.Y;
					poly.LeftShade+=ds_lhs*poly.Y;

					poly.FLeftTextX+=dtx_lhs*poly.Y;
					poly.FLeftTextY+=dty_lhs*poly.Y;
					poly.Q+=dq_lhs*poly.Y;
					count-=poly.Y;
					poly.Y=0;

				}
				else
				{
					poly.LeftX+=dx_lhs*count;
					poly.RightX+=dx_rhs*count;
					poly.LeftShade+=ds_lhs*count;

					poly.FLeftTextX+=dtx_lhs*count;
					poly.FLeftTextY+=dty_lhs*count;
					poly.Y=-poly.Y+count;
					poly.Q+=dq_lhs*count;
					count=0;
				}
			}
				if(poly.Y>=WorkWindowHeight)
					goto clip_out;

			//do the top part of the triangle
			while(count>0)
			{
				PSCAN_LINE_GT(&poly);
//				raster_fill.FunctionPointer(&poly);
//				SCAN_LINE_GT(&poly);
//				SCAN_LINE_GT(poly.Y,poly.LeftX>>16,poly.RightX>>16,poly.LeftShade,poly.StepShade,poly.LeftTextX>>8,poly.StepTextX,poly.LeftTextY>>POLY_TEXT_SHIFT,poly.StepTextY);
				poly.LeftX+=dx_lhs;
				poly.RightX+=dx_rhs;
				poly.LeftShade+=ds_lhs;

				poly.FLeftTextX+=dtx_lhs;
				poly.FLeftTextY+=dty_lhs;
				poly.Q+=dq_lhs;
				poly.Y++;
				count--;
				if(poly.Y>=WorkWindowHeight)
					goto clip_out;
			}
		}
		else
		{
//			flat top
//			ft=1;
			if(p2->Y==p1->Y)
			{
				poly.FLeftTextY=(float)((float)p2->TY/(float)p2->Z3d);
				poly.FLeftTextX=(float)((float)p2->TX/(float)p2->Z3d);
				poly.LeftShade=p2->Shade<<14;
				poly.LeftX=(p2->X<<16);//+(1<<15);
				poly.Q=q2;

				poly.RightX=(p1->X<<16);//+(1<<15);
			}
			else
			{
				poly.FLeftTextY=(float)((float)p1->TY/(float)p1->Z3d);
				poly.FLeftTextX=(float)((float)p1->TX/(float)p1->Z3d);
				poly.LeftShade=p1->Shade<<14;
				poly.LeftX=(p1->X<<16);//+(1<<15);
				poly.Q=q1;

				poly.RightX=(p3->X<<16);//+(1<<15);

			}


		}

		//setup for bottom half of tri
		if(dy_lhs<dy_rhs)
		{
		//lhs kinks

			count=dy_rhs-dy_lhs;
			dx_lhs=dx2;
			dq_lhs=dq2;
			ds_lhs=ds_2;
			dtx_lhs=dtx_2;
			dty_lhs=dty_2;

			poly.LeftX=(p2->X<<16);//+(1<<15);
			poly.FLeftTextY=(float)((float)p2->TY/(float)p2->Z3d);
			poly.FLeftTextX=(float)((float)p2->TX/(float)p2->Z3d);

		}
		else
		{
		//rhs kinks

			count=dy_lhs-dy_rhs;
			dx_rhs=dx2;
		}

		//clip
		if(poly.Y<0)
		{
			poly.Y=-poly.Y; // poly.Y is lines to skip
			if(count>=poly.Y)
			{	// will be on screen before this count finishes
				poly.LeftX+=dx_lhs*poly.Y;
				poly.RightX+=dx_rhs*poly.Y;
				poly.LeftShade+=ds_lhs*poly.Y;

				poly.FLeftTextX+=(float)(dtx_lhs*poly.Y);
				poly.FLeftTextY+=(float)(dty_lhs*poly.Y);
				poly.Q+=dq_lhs*poly.Y;
				count-=poly.Y;
				poly.Y=0;
			}
			else
				goto clip_out;
		}
		if(poly.Y>=WorkWindowHeight)
			goto clip_out;


		while(count>0)
		{
//  			raster_fill.FunctionPointer(&poly);
			PSCAN_LINE_GT(&poly);

			poly.LeftX+=dx_lhs;
			poly.RightX+=dx_rhs;
			poly.LeftShade+=ds_lhs;
			poly.FLeftTextX+=dtx_lhs;
			poly.FLeftTextY+=dty_lhs;
			poly.Q+=dq_lhs;
			poly.Y++;
			count--;
 			if(poly.Y>=WorkWindowHeight)
				goto clip_out;
		}
	}

clip_out:;

#ifdef	FILTERING_ON
	if(filtered)
	{
		p1->TX=(p1->TX>>1)+f_dx;
		p1->TY=(p1->TY>>1)+f_dy;
		p2->TX=(p2->TX>>1)+f_dx;
		p2->TY=(p2->TY>>1)+f_dy;
		p3->TX=(p3->TX>>1)+f_dx;
		p3->TY=(p3->TY>>1)+f_dy;
	}
#endif
//		p1->Shade=128;
}

void	rotate_a_point(struct	MfEnginePoint *p1,SLONG cx,SLONG cy,SLONG a)
{
	SLONG	dx,dy;

	dx=p1->X-cx;
	dy=p1->Y-cy;

	p1->X=(dx*COS(a)-dy*SIN(a))>>16;
	p1->Y=(dx*SIN(a)+dy*COS(a))>>16;

	p1->X+=cx;
	p1->Y+=cy;
	
}



struct	Boint span_info[1024];


void	(*render_span)(struct	Boint *p_b,UBYTE	*ptr_screen,SLONG draw_flags);

void	render_span8(struct	Boint *p_b,UBYTE	*ptr_screen,SLONG draw_flags)
{
	SLONG	width;
	SLONG	step_shade,step_tx,step_ty;

	width=p_b->RightX-p_b->LeftX;
	if(width<=0)
		return;
/*
	step_shade=((p_b->RightShade - p_b->LeftShade))/width;
	step_tx   =((p_b->RightTX    - p_b->LeftTX   ))/width;
	step_ty   =((p_b->RightTY    - p_b->LeftTY   ))/width;
*/
		step_shade=QDIV64((p_b->RightShade - p_b->LeftShade),width+1);
		step_tx   =QDIV64((p_b->RightTX    - p_b->LeftTX   ),width+1);
		step_ty   =QDIV64((p_b->RightTY    - p_b->LeftTY   ),width+1);
	


	if(p_b->LeftX >= WorkWindowWidth || p_b -> RightX<0)
		return;


	if(p_b->RightX > WorkWindowWidth)
	{
		p_b->RightX	= WorkWindowWidth;
		width=p_b->RightX-p_b->LeftX;
		if(width<=0)
			return;
	}

	if(p_b->LeftX < 0)
	{
		p_b->LeftShade+=step_shade*(-p_b->LeftX);
		p_b->LeftTX+=step_tx*(-p_b->LeftX);
		p_b->LeftTY+=step_ty*(-p_b->LeftX);
		p_b->LeftX=0;

		width=p_b->RightX;
	}
//	ptr_screen=WorkWindow+p_b->LeftX+(WorkScreenWidth*y);
	if(width)
	{  //"ASM" version

		ASMtx   = p_b->LeftTX;
		ASMty   = p_b->LeftTY;
		ASMshade= p_b->LeftShade;

		ptr_screen+=p_b->LeftX;
//		RENDER_GO_GT();

		switch(draw_flags&0x3f)
		{
			case POLY_F:
#ifdef	_MSC_VER
				RENDER_MSC_F(ptr_screen-1,width);
#else
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_COL();
				RENDER_GO_F();
#endif
				break;		
			case POLY_50F:
#ifdef	_MSC_VER
				RENDER_MSC_50F(ptr_screen-1,width);
#else
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_COL();
				RENDER_GO_50F();
#endif
				break;		
			case POLY_G:
#ifdef	_MSC_VER
				RENDER_MSC_G(ptr_screen-1,width,step_shade);
#else
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_COL();
				RENDER_GO_G();
#endif
				break;		
			case POLY_TGT:
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_TGT();
				break;		
			case POLY_GT:
#ifdef	_MSC_VER
				RENDER_MSC_GT(ptr_screen-1,width,step_shade,step_tx,step_ty);
#else
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_GT();
#endif
				break;		
			case POLY_MGT:
#ifdef	_MSC_VER
				RENDER_MSC_MGT(ptr_screen-1,width,step_shade,step_tx,step_ty);
#else
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_MGT();
#endif
				break;		
			case POLY_50GT:
#ifdef	_MSC_VER
				RENDER_MSC_50GT(ptr_screen-1,width,step_shade,step_tx,step_ty);
#else
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_50GT();
#endif
				break;		
			case POLY_50T:
#ifdef	_MSC_VER
				RENDER_MSC_50T(ptr_screen-1,width,step_tx,step_ty);
#else
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_50T();
#endif

				break;		
			case POLY_50MGT:
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_50MGT();
				break;		
			case POLY_MT:
#ifdef	_MSC_VER
				RENDER_MSC_MT(ptr_screen-1,width,step_tx,step_ty);
#else
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_MT();
#endif
				break;		
			case POLY_T:
#ifdef	_MSC_VER
				RENDER_MSC_T(ptr_screen-1,width,step_tx,step_ty);
#else
				RENDER_SETUP(ptr_screen-1,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_T();
#endif
				break;		
		}

	}

//	*ptr_screen=255;
//	*(ptr_screen+(width)-1)=255;
}

void	render_span16(struct	Boint *p_b,UBYTE	*ptr_screen,SLONG draw_flags)
{
	SLONG	width;
	SLONG	step_shade,step_tx,step_ty;

	width=p_b->RightX-p_b->LeftX;

	//p_b->LeftShade<<=2;
	//p_b->RightShade<<=2;

	if(width<=0)
		return;
		step_shade=QDIV64((p_b->RightShade - p_b->LeftShade),width+1);
		step_tx   =QDIV64((p_b->RightTX    - p_b->LeftTX   ),width+1);
		step_ty   =QDIV64((p_b->RightTY    - p_b->LeftTY   ),width+1);
	
	if(p_b->LeftX >= WorkWindowWidth || p_b -> RightX<0)
		return;


	if(p_b->RightX > WorkWindowWidth)
	{
		p_b->RightX	= WorkWindowWidth;
		width=p_b->RightX-p_b->LeftX;
		if(width<=0)
			return;
	}

	if(p_b->LeftX < 0)
	{
		p_b->LeftShade+=step_shade*(-p_b->LeftX);
		p_b->LeftTX+=step_tx*(-p_b->LeftX);
		p_b->LeftTY+=step_ty*(-p_b->LeftX);
		p_b->LeftX=0;

		width=p_b->RightX;
	}
//	ptr_screen=WorkWindow+p_b->LeftX+(WorkScreenWidth*y);
	if(width)
	{  //"ASM" version

		ASMtx   = p_b->LeftTX;
		ASMty   = p_b->LeftTY;
		ASMshade= p_b->LeftShade;

		ptr_screen+=p_b->LeftX*2;

		switch(draw_flags&0x3f)
		{
			case POLY_G:
#ifdef	_MSC_VER
				RENDER_MSC_G16(ptr_screen-2,width,step_shade);
#endif
				break;
			case	POLY_F:
#ifdef	_MSC_VER
				RENDER_MSC_F16(ptr_screen-2,width);
#endif
				break;
			case POLY_MT:
#ifdef	_MSC_VER
				RENDER_MSC_MT16(ptr_screen-2,width,step_tx,step_ty);
#endif
				break;
			case POLY_T:
#ifdef	_MSC_VER
				RENDER_MSC_T16(ptr_screen-2,width,step_tx,step_ty);

#endif
				break;
			case POLY_TGT:
#ifdef	_MSC_VER
				RENDER_MSC_TGT16(ptr_screen-2,width,step_shade,step_tx,step_ty);
#endif
				break;

			case POLY_GT:
#ifdef	_MSC_VER
				if(ASMtext_page)
					RENDER_MSC_GT16(ptr_screen-2,width,step_shade,step_tx,step_ty);
				else
				{
					LogText(" weirdness\n");
				}
#else
				RENDER_SETUP(ptr_screen-2,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_GT16();
#endif
				break;		
			case POLY_MGT:
#ifdef	_MSC_VER
				RENDER_MSC_MGT16(ptr_screen-2,width,step_shade,step_tx,step_ty);
#endif
			default:
				break;		
		}
	}
}

void	render_span32(struct	Boint *p_b,UBYTE	*ptr_screen,SLONG draw_flags)
{
	SLONG	width;
	SLONG	step_shade,step_tx,step_ty;

	width=p_b->RightX-p_b->LeftX;
	if(width<=0)
		return;

	p_b->LeftShade<<=2;
	p_b->RightShade<<=2;
/*
	step_shade=((p_b->RightShade - p_b->LeftShade))/width;
	step_tx   =((p_b->RightTX    - p_b->LeftTX   ))/width;
	step_ty   =((p_b->RightTY    - p_b->LeftTY   ))/width;
*/
	step_shade=QDIV64((p_b->RightShade - p_b->LeftShade),width);
	step_tx   =QDIV64((p_b->RightTX    - p_b->LeftTX   ),width);
	step_ty   =QDIV64((p_b->RightTY    - p_b->LeftTY   ),width);
	


	if(p_b->LeftX >= WorkWindowWidth || p_b -> RightX<0)
		return;


	if(p_b->RightX >= WorkWindowWidth)
	{
		p_b->RightX	= WorkWindowWidth-1;
		width=p_b->RightX-p_b->LeftX;
		if(width<=0)
			return;
	}

	if(p_b->LeftX < 0)
	{
		p_b->LeftShade+=step_shade*(-p_b->LeftX);
		p_b->LeftTX+=step_tx*(-p_b->LeftX);
		p_b->LeftTY+=step_ty*(-p_b->LeftX);
		p_b->LeftX=0;

		width=p_b->RightX;
	}
	if(width)
	{  //"ASM" version

		ASMtx   = p_b->LeftTX;
		ASMty   = p_b->LeftTY;
		ASMshade= p_b->LeftShade;

		ptr_screen+=p_b->LeftX*4;

		switch(draw_flags&0x1f)
		{
			default:
				RENDER_SETUP(ptr_screen-4,width,step_shade,step_tx,step_ty);
				RENDER_SETUP2();
				RENDER_GO_GT32();
				break;		
		}
	}
}

#define	FILTERING_ON	1


inline void	average_points(struct MfEnginePoint *mid,struct MfEnginePoint *p1,struct MfEnginePoint *p2)
{
	mid->X=(p1->X+p2->X)>>1;
	mid->Y=(p1->Y+p2->Y)>>1;
	mid->TX=(p1->TX+p2->TX)>>1;
	mid->TY=(p1->TY+p2->TY)>>1;
	mid->Shade=(p1->Shade+p2->Shade)>>1;
}

inline void	pers_average_points(struct MfEnginePoint *mid,struct MfEnginePoint *p1,struct MfEnginePoint *p2)
{
	float	as,at,aq;
	float	az,atx,aty;

	atx=((float)(p1->TX+p2->TX ))/2.0;
	aty=((float)(p1->TY+p2->TY ))/2.0;
/*
	az=((float)(p1->Z3d+p2->Z3d))/2.0;

	aq=1.0/az;

	as=atx/az;


	at=aty/az;
*/
	aq=((1.0/(float)p1->Z3d)+(1.0/(float)p2->Z3d))/2.0;

	as=atx*aq;
	at=aty*aq;

	mid->X=(p1->X+p2->X)>>1;
	mid->Y=(p1->Y+p2->Y)>>1;
	mid->TX=(SLONG)(as/aq);
	mid->TY=(SLONG)(at/aq);
	mid->Shade=(p1->Shade+p2->Shade)>>1;
	mid->Z3d=(p1->Z3d+p2->Z3d)>>1;
}

inline void	pers_average_points4(struct MfEnginePoint *mid,struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3,struct MfEnginePoint *p4)
{
	float	as,at,aq;
	float	az,atx,aty;


	atx=((float)(p1->TX+p2->TX+p3->TX+p4->TX))/4.0;
	aty=((float)(p1->TY+p2->TY+p3->TY+p4->TY))/4.0;
/*
	az=((float)(p1->Z3d+p2->Z3d+p3->Z3d+p4->Z3d))/4.0;

	aq=1.0/az;
	as=atx/az;
	at=aty/az;
*/
	aq=((1.0/(float)p1->Z3d)+(1.0/(float)p2->Z3d)+(1.0/(float)p3->Z3d)+(1.0/(float)p4->Z3d))/2.0;

	as=atx*aq;
	at=aty*aq;


	mid->X=(p1->X+p2->X+p3->X+p4->X)>>2;
	mid->Y=(p1->Y+p2->Y+p3->Y+p4->Y)>>2;
	mid->TX=(SLONG)(as/aq);
	mid->TY=(SLONG)(at/aq);
	mid->Shade=(p1->Shade+p2->Shade+p3->Shade+p4->Shade)>>2;
	mid->Z3d=(p1->Z3d+p2->Z3d+p3->Z3d+p4->Z3d)>>2;
}

inline void	average_points4(struct MfEnginePoint *mid,struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3,struct MfEnginePoint *p4)
{
	mid->X=(p1->X+p2->X+p3->X+p4->X)>>2;
	mid->Y=(p1->Y+p2->Y+p3->Y+p4->Y)>>2;
	mid->TX=(p1->TX+p2->TX+p3->TX+p4->TX)>>2;
	mid->TY=(p1->TY+p2->TY+p3->TY+p4->TY)>>2;
	mid->Shade=(p1->Shade+p2->Shade+p3->Shade+p4->Shade)>>2;
}

//scan from p1 to p2 filling in the span info
inline void	scan_a_line_noz(struct	MfEnginePoint *p1,struct MfEnginePoint *p2)
{
	SLONG	dx,dy,cx,cy;
	SLONG	tx,ty,dtx,dty;
	SLONG	shade,d_shade;
	SLONG	side=0;
	struct Boint *ptr_side;
	{
/*
		CBYTE	str[100];
		sprintf(str,"(%d,%d)",p1->X,p1->Y);
		QuickTextC(p1->X,p1->Y,str,0);
		QuickTextC(p1->X+1,p1->Y+1,str,255);

		sprintf(str,"(%d,%d)",p2->X,p2->Y);
		QuickTextC(p2->X,p2->Y,str,0);
		QuickTextC(p2->X+1,p2->Y+1,str,255);
*/
		

	}
	if(p1->Y>p2->Y)
	{
		struct	MfEnginePoint	*ptemp;
		ptemp=p2;
		p2=p1;
		p1=ptemp;
		side=1;
	}


	dy=p2->Y-p1->Y;
	dx=p2->X-p1->X;

	if(dy==0)
		return;

	cx=p1->X<<16;
	cy=p1->Y;


	tx=p1->TX<<16;
	ty=p1->TY<<16;

	dtx=p2->TX-p1->TX;
	dty=p2->TY-p1->TY;

	shade     = (p1->Shade)<<14;           //14
	d_shade   = (p2->Shade-p1->Shade);


	if(dy==0)
	{
		return;
	}
	else
	{

		dx      = (dx<<16)/dy;
//		dx      = QDIV64((dx<<16),dy);
		dtx     = QDIV64((dtx<<16),dy);
		dty     = QDIV64((dty<<16),dy);
	    d_shade = QDIV64((d_shade<<14),dy);	   //14


		if(cy<0)
		{
			SLONG	over=-cy;

			cx   += dx*over;
			tx   += dtx*over;
			ty   += dty*over;
			shade+= d_shade*over;
			dy-=over;
			cy    = 0;
			if(dy<=0)
			{
				return;
			}
		}

		ptr_side=&span_info[cy];

		if(side==0)
		{
			
			for(;dy>=0;dy--,cy++)
			{
				if(cy>WorkWindowHeight)
				{
					return;
				}

				ptr_side->RightX =	(cx)>>16;
				ptr_side->RightTX=	tx;
				ptr_side->RightTY=	ty;
				ptr_side->RightShade=shade;

				cx   += dx;
				tx   += dtx;
				ty   += dty;
				shade+= d_shade;
				ptr_side++;
			}
				(ptr_side-1)->RightX =	p2->X;
		}
		else
		{
			for(;dy>=0;dy--,cy++)
			{
				if(cy>WorkWindowHeight)
				{
					return;
				}

				ptr_side->LeftX =	(cx)>>16;
				ptr_side->LeftTX=	tx;
				ptr_side->LeftTY=	ty;
				ptr_side->LeftShade=shade;

				cx   += dx;
				tx   += dtx;
				ty   += dty;
				shade+= d_shade;
				ptr_side++;
			}
	  		(ptr_side-1)->LeftX =	p2->X;
		}
	}
}

/*
		p1        p5         p2


		p8		  p9		 p6


	    p4   	  p7		 p3
*/


void	my_quad_noz(struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3,struct MfEnginePoint *p4)
{
	SLONG	top,bottom;
	SLONG	left,right;
	struct	Boint	*p_span;
	UBYTE	*ptr_screen;

#ifdef	FILTERING_ON
	SLONG	f_dx=0,f_dy=0;
	SLONG	filtered=0,can_filter=0;
#endif
		
	top    = MIN( MIN( MIN(p1->Y,p2->Y),p3->Y ),p4->Y );
	bottom = MAX( MAX( MAX(p1->Y,p2->Y),p3->Y ),p4->Y );

	left   = MIN( MIN( MIN(p1->X,p2->X),p3->X ),p4->X );
	right  = MAX( MAX( MAX(p1->X,p2->X),p3->X ),p4->X );
	ASMCol=poly_info.Col;
	ASMfade_page=yc_to_555[poly_info.Page];
	ASMpal_address=&yc_to_555[poly_info.Page][32*256];


/*
	if(bottom-top>128||right-left>128)
	{
		struct	MfEnginePoint p5,p6,p7,p8,p9;

		average_points(&p5,p1,p2);
		average_points(&p6,p2,p3);
		average_points(&p7,p3,p4);
		average_points(&p8,p4,p1);
		average_points4(&p9,p1,p2,p3,p4);

		my_quad(&p8,p1,&p5,&p9);
		my_quad(&p5,p2,&p6,&p9);
		my_quad(&p7,&p9,&p6,p3);
		my_quad(&p8,&p9,&p7,p4);
		return;
	}
*/

	if(bottom<0||top>=WorkWindowHeight)
		return;

#ifdef	FILTERING_ON
	if(poly_info.DrawFlags&POLY_FLAG_TEXTURED)
		can_filter=can_texture_be_filtered4(p1,p2,p3,p4);
#endif

	ASMtext_page=poly_info.PTexture;
	if(ASMtext_page==0)
		return;
	if(poly_info.DrawFlags&POLY_FLAG_TILED)
	{
		ASMtext_page+=calc_texture_offset4(p1,p2,p3,p4);
#ifdef	FILTERING_ON
		can_filter=0;
#endif
	}

/*-----------**
** FILTERING **
**-----------*/
#ifdef	FILTERING_ON
	if(can_filter)
		filtered=allready_filtered4(p1,p2,p3,p4,&f_dx,&f_dy);

	if(can_filter&&!filtered)
	{
		filtered=filter_poly_tmap4(p1,p2,p3,p4,&f_dx,&f_dy);
	}
#endif

		
	scan_a_line_noz(p1,p2);
	scan_a_line_noz(p2,p3);
	scan_a_line_noz(p3,p4);
	scan_a_line_noz(p4,p1);


	if(top<0)
		top=0;
	if(bottom>WorkWindowHeight)
		bottom=WorkWindowHeight;

	p_span=&span_info[top];
//	ptr_screen=WorkWindow+p_b->LeftX+(WorkScreenWidth*y);
	ptr_screen=WorkWindow+top*WorkScreenWidth; //(UBYTE*)((ULONG)WorkWindow+(ULONG)top(ULONG)*WorkScreenWidth);
	for(;top<bottom;top++)
	{
		render_span(p_span,ptr_screen,poly_info.DrawFlags);
		p_span++;
		ptr_screen+=WorkScreenWidth;
	}

//clip_out:;

#ifdef	FILTERING_ON
	if(filtered)
	{
		p1->TX=(p1->TX>>1)+f_dx;
		p1->TY=(p1->TY>>1)+f_dy;
		p2->TX=(p2->TX>>1)+f_dx;
		p2->TY=(p2->TY>>1)+f_dy;
		p3->TX=(p3->TX>>1)+f_dx;
		p3->TY=(p3->TY>>1)+f_dy;
		p4->TX=(p4->TX>>1)+f_dx;
		p4->TY=(p4->TY>>1)+f_dy;
	}
#endif


}


void	my_trig_noz(struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p3)
{
	SLONG	top,bottom;
	SLONG	left,right;
	struct	Boint	*p_span;
	UBYTE	*ptr_screen;

#ifdef	FILTERING_ON
	SLONG	f_dx=0,f_dy=0;
	SLONG	filtered=0,can_filter=0;
#endif
		
	top    =  MIN( MIN(p1->Y,p2->Y),p3->Y );
	bottom =  MAX( MAX(p1->Y,p2->Y),p3->Y );

	left   =  MIN( MIN(p1->X,p2->X),p3->X );
	right  =  MAX( MAX(p1->X,p2->X),p3->X );
	ASMCol=poly_info.Col;
	ASMfade_page=&yc_to_555[poly_info.Page][0];
	ASMpal_address=&yc_to_555[poly_info.Page][32*256];

	if(bottom<0||top>=WorkWindowHeight)
		return;

#ifdef	FILTERING_ON
	if(poly_info.DrawFlags&POLY_FLAG_TEXTURED)
		can_filter=can_texture_be_filtered(p1,p2,p3);
#endif

	ASMtext_page=poly_info.PTexture;
	if(ASMtext_page==0)
		return;

	if(poly_info.DrawFlags&POLY_FLAG_TILED)
	{
		ASMtext_page+=calc_texture_offset(p1,p2,p3);
#ifdef	FILTERING_ON
		can_filter=0;
#endif
	}

/*-----------**
** FILTERING **
**-----------*/
#ifdef	FILTERING_ON
	if(can_filter)
		filtered=allready_filtered(p1,p2,p3,&f_dx,&f_dy);

	if(can_filter&&!filtered)
	{
		filtered=filter_poly_tmap(p1,p2,p3,&f_dx,&f_dy);
	}
#endif

		
	scan_a_line_noz(p1,p2);
	scan_a_line_noz(p2,p3);
	scan_a_line_noz(p3,p1);

	if(top<0)
		top=0;
	if(bottom>WorkWindowHeight)
		bottom=WorkWindowHeight;

	p_span=&span_info[top];
//	ptr_screen=WorkWindow+p_b->LeftX+(WorkScreenWidth*y);
	ptr_screen=WorkWindow+top*WorkScreenWidth; //(UBYTE*)((ULONG)WorkWindow+(ULONG)top(ULONG)*WorkScreenWidth);
	for(;top<bottom;top++)
	{
		render_span(p_span,ptr_screen,poly_info.DrawFlags);
		p_span++;
		ptr_screen+=WorkScreenWidth;
	}

//clip_out:;

#ifdef	FILTERING_ON
	if(filtered)
	{
		p1->TX=(p1->TX>>1)+f_dx;
		p1->TY=(p1->TY>>1)+f_dy;
		p2->TX=(p2->TX>>1)+f_dx;
		p2->TY=(p2->TY>>1)+f_dy;
		p3->TX=(p3->TX>>1)+f_dx;
		p3->TY=(p3->TY>>1)+f_dy;
	}
#endif


}


ULONG	poly_def;
//scan from p1 to p2 filling in the span info
inline void	scan_a_line(struct	Boint *p_b,SLONG top,struct	MfEnginePoint *p1,struct MfEnginePoint *p2)
{
	SLONG	dx,dy,cx,cy,dz,cz;
	SLONG	tx,ty,dtx,dty;
	SLONG	shade,d_shade;
	struct Boint *ptr_side;

	dz=p2->Z3d-p1->Z3d;
	dy=p2->Y-p1->Y;
	dx=p2->X-p1->X;

	if(dy==0)
	{
		return;
	}

	cz=p1->Z3d<<16;
	cx=p1->X<<16;
	cy=p1->Y;


	tx=p1->TX<<16;
	ty=p1->TY<<16;

	dtx=p2->TX-p1->TX;
	dty=p2->TY-p1->TY;

	shade     = (p1->Shade)<<14;           //14
	d_shade   = (p2->Shade-p1->Shade);


	if(dy==0)
	{
		return;
	}
	else
	if(dy<0)
	{

		dy=-dy;

		dx      = (dx<<16)/dy;
		dz      = (dz<<16)/dy;
//		dx      = QDIV64((dx<<16),dy);
		dtx     = QDIV64((dtx<<16),dy);
		dty     = QDIV64((dty<<16),dy);
	    d_shade = QDIV64((d_shade<<14),dy); //14

		if(cy>=WorkWindowHeight)
		{
			SLONG	over=cy-(WorkWindowHeight-1);

			cz   += dz*over;
			cx   += dx*over;
			tx   += dtx*over;
			ty   += dty*over;
			shade+= d_shade*over;
			dy   -=over;
			cy    = WorkWindowHeight-1;
			if(dy<=0)
			{
				
				return;
			}
		}


//		ptr_side=&span_info[cy];
		ptr_side=&p_b[cy-top];

		for(;dy>=0;dy--,cy--)
		{
			if(cy<0)
			{
				return;
			}
				ptr_side->LeftX =	cx>>16;
			ptr_side->LeftZ =	cz>>16;
			ptr_side->LeftTX=	tx;
			ptr_side->LeftTY=	ty;
			ptr_side->LeftShade=shade;
			ptr_side->DrawFlags=poly_info.DrawFlags;

//			ptr_side->Y=cy;
		
			cx   += dx;
			cz   += dz;
			tx   += dtx;
			ty   += dty;
			shade+= d_shade;
			ptr_side--;
		}
	}
	else
	{

		dx      = (dx<<16)/dy;
		dz      = (dz<<16)/dy;
//		dx      = QDIV64((dx<<16),dy);
		dtx     = QDIV64((dtx<<16),dy);
		dty     = QDIV64((dty<<16),dy);
	    d_shade = QDIV64((d_shade<<14),dy);	   //14


		if(cy<0)
		{
			SLONG	over=-cy;

			cz   += dz*over;
			cx   += dx*over;
			tx   += dtx*over;
			ty   += dty*over;
			shade+= d_shade*over;
			dy-=over;
			cy    = 0;
			if(dy<=0)
			{
				return;
			}
		}

//		ptr_side=&span_info[cy];
		ptr_side=&p_b[cy-top];

		for(;dy>=0;dy--,cy++)
		{
			if(cy>=WorkWindowHeight)
			{
				return;
			}

				ptr_side->RightX =	cx>>16;
			ptr_side->RightZ =	cz>>16;
			ptr_side->RightTX=	tx;
			ptr_side->RightTY=	ty;
			ptr_side->RightShade=shade;

//			ptr_side->Y=cy;
			cx   += dx;
			cz   += dz;
			tx   += dtx;
			ty   += dty;
			shade+= d_shade;
			ptr_side++;
		}
	}
}

/*
		p1        p5         p2


		p8		  p9		 p6


	    p4   	  p7		 p3
*/

#undef	FILTERING_ON
void	my_quad(struct MfEnginePoint *p1,struct MfEnginePoint *p2,struct MfEnginePoint *p4,struct MfEnginePoint *p3)
{
	SLONG	top,bottom;
	SLONG	left,right;
	struct	Boint	*p_span;
	UBYTE	*ptr_screen;

#ifdef	FILTERING_ON
	SLONG	f_dx=0,f_dy=0;
	SLONG	filtered=0,can_filter=0;
#endif
	ASMCol=poly_info.Col;
	if(!is_it_clockwise(p1,p2,p3))
		return;
	p1->X%=320;
	p2->X%=320;
	p3->X%=320;
	p4->X%=320;

	p1->Y%=200;
	p2->Y%=200;
	p3->Y%=200;
	p4->Y%=200;
		
	top    = MIN( MIN( MIN(p1->Y,p2->Y),p3->Y ),p4->Y );
	bottom = MAX( MAX( MAX(p1->Y,p2->Y),p3->Y ),p4->Y );

	left   = MIN( MIN( MIN(p1->X,p2->X),p3->X ),p4->X );
	right  = MAX( MAX( MAX(p1->X,p2->X),p3->X ),p4->X );


	if(bottom<0||top>=WorkWindowHeight)
		return;

	ASMtext_page=poly_info.PTexture;
	if(ASMtext_page==0)
		return;

	poly_def=(ULONG)p1*(ULONG)p2*(ULONG)p3;
/*-----------**
** FILTERING **
**-----------*/
#ifdef	FILTERING_ON
//	if(poly_info.DrawFlags&POLY_FLAG_TEXTURED)
		can_filter=can_texture_be_filtered4(p1,p2,p3,p4);

	if(can_filter)
		filtered=allready_filtered4(p1,p2,p3,p4,&f_dx,&f_dy);

	if(can_filter&&!filtered)
	{
		filtered=filter_poly_tmap4(p1,p2,p3,p4,&f_dx,&f_dy);
	}
#endif
	if(top<0)
		top=0;
	if(bottom>WorkWindowHeight)
		bottom=WorkWindowHeight;

	scan_a_line(current_boint,top,p1,p2);
	scan_a_line(current_boint,top,p2,p3);
	scan_a_line(current_boint,top,p3,p4);
	scan_a_line(current_boint,top,p4,p1);



	{
		struct	Boint	**p_head;
		p_head=&z_spans[top];
		p_span=current_boint; //[top];
		current_boint+=(bottom-top)+1;
		next_boint+=(bottom-top);
		if(next_boint<MAX_BOINT)
		for(;top<=bottom;top++)
		{
//			p_span->LeftZ=(p_span->LeftZ+p_span->RightZ)>>1;
			insert_span(p_span,p_head);
			p_span++;
			p_head++;
		}
	}		

//clip_out:;

#ifdef	FILTERING_ON
	if(filtered)
	{
		p1->TX=(p1->TX>>1)+f_dx;
		p1->TY=(p1->TY>>1)+f_dy;
		p2->TX=(p2->TX>>1)+f_dx;
		p2->TY=(p2->TY>>1)+f_dy;
		p3->TX=(p3->TX>>1)+f_dx;
		p3->TY=(p3->TY>>1)+f_dy;
		p4->TX=(p4->TX>>1)+f_dx;
		p4->TY=(p4->TY>>1)+f_dy;
	}
#endif
}


void	draw_a_single_span(UBYTE *p_screen,struct	Boint	p_b)
{
	
}
void	draw_all_spans(void)
{
	SLONG	c0;
	SLONG	count;
	struct	Boint	**b,*p_c;
	UBYTE	*p_screen;
	ULONG	dont_draw=0,dump=0;
	SLONG	y=0;
	if(Keys[KB_A])
		dont_draw=1;
	if(Keys[KB_D])
		dump=1;

	p_screen=WorkWindow;
	b=&z_spans[0];
	for(c0=0;c0<WorkWindowHeight>>1;c0++)
	{
		p_c=*b;
		count=0;
		if(!dont_draw)
			while(p_c&&count++<150)
			{
				render_span(p_c,p_screen,p_c->DrawFlags);
#ifdef	DEBUG_SPAN
				p_screen[count]=count+1;
#endif
				p_c=p_c->PNext;
			}
		if(dump)
		{
			CBYTE	str[100];
			sprintf(str," DUMP SCREEN y %d ",y);
			show_line(b,str);
		}
		if(MouseY==y&&LeftButton)
		{
			show_line(b,"WHOLE SPAN");
			LeftButton=0;
		}
		*b=0;
		b++;
		y++;
		p_screen+=WorkScreenWidth;
	}
	current_boint=&boint_pool[0];
	{
		CBYTE	str[100];
		sprintf(str,"Z BUFFERED %d cfind %d cin %d clhs %d crhs %d",next_boint,count_find,count_insert,count_chop_lhs,count_chop_rhs);
		QuickTextC(10,50,str,255);
		
	}
	next_boint=1;
	count_find=0,count_insert=0,count_chop_lhs=0,count_chop_rhs=0;
}

void	test_poly(void)
{
	static	struct	MfEnginePoint p1,p2,p3,p4,p5,p6;
	static	init=0;

	if(init==0)
	{
		init=1;
		p1.Z3d=64;
		p1.X=108;
		p1.Y=84+100;
		p1.TX=0;
		p1.TY=0;
		p1.Shade=128;

		p2.Z3d=64;
		p2.X=268;
		p2.Y=84+100;
		p2.TX=31;
		p2.TY=0;
		p2.Shade=128;

		p3.Z3d=64;
		p3.X=427;
		p3.Y=284+100;
		p3.TX=0;
		p3.TY=31;
		p3.Shade=128;

		p4.Z3d=64;
		p4.X=164;
		p4.Y=20+100;
		p4.TX=31;
		p4.TY=31;
		p4.Shade=128;

		p5.X=260;
		p5.Y=352;
		p5.TX=196;
		p5.TY=0;
		p5.Shade=128;

		p6.X=457;
		p6.Y=352;
		p6.TX=0;
		p6.TY=196;
		p6.Shade=128;


	}

	if(Keys[KB_PLUS])
	{
		Keys[KB_PLUS]=0;
		p1.TX<<=1;
		p1.TY<<=1;
		p2.TX<<=1;
		p2.TY<<=1;
		p3.TX<<=1;
		p3.TY<<=1;
		p4.TX<<=1;
		p4.TY<<=1;
	}

	if(Keys[KB_MINUS])
	{
		Keys[KB_MINUS]=0;
		p1.TX>>=1;
		p1.TY>>=1;
		p2.TX>>=1;
		p2.TY>>=1;
		p3.TX>>=1;
		p3.TY>>=1;
		p4.TX>>=1;
		p4.TY>>=1;
	}

	if(Keys[KB_G]&&!ShiftFlag)
	{

		rotate_a_point(&p1,200,200,1);
		rotate_a_point(&p2,200,200,1);
		rotate_a_point(&p3,200,200,1);
		rotate_a_point(&p4,200,200,1);

	}

	if(Keys[KB_H]&&!ShiftFlag)
		p2.X++;
	if(Keys[KB_H]&&ShiftFlag)
		p2.X--;

	if(Keys[KB_J]&&!ShiftFlag)
		p3.X++;
	if(Keys[KB_J]&&ShiftFlag)
		p3.X--;

	if(Keys[KB_K]&&!ShiftFlag)
		p4.X++;
	if(Keys[KB_K]&&ShiftFlag)
		p4.X--;

	if(Keys[KB_V]&&!ShiftFlag)
		p1.Y++;
	if(Keys[KB_V]&&ShiftFlag)
		p1.Y--;

	if(Keys[KB_B]&&!ShiftFlag)
		p2.Y++;
	if(Keys[KB_B]&&ShiftFlag)
		p2.Y--;

	if(Keys[KB_N]&&!ShiftFlag)
		p3.Y++;
	if(Keys[KB_N]&&ShiftFlag)
		p3.Y--;

	if(Keys[KB_M]&&!ShiftFlag)
		p4.Y++;
	if(Keys[KB_M]&&ShiftFlag)
		p4.Y--;

	poly_info.Page=0;
	poly_info.PTexture=tmaps[0];
	poly_info.DrawFlags=POLY_FLAG_GOURAD|POLY_FLAG_SEMI_TRANS; //|POLY_FLAG_TEXTURED;
	poly_info.Col=255;
	my_trig(&p1,&p2,&p3);
	my_trig(&p3,&p2,&p1);
	my_trig(&p1,&p2,&p4);
	my_trig(&p4,&p2,&p1);
//	my_trigp(&p4,&p2,&p1);
}


#define	DITHER_WIDTH	256
void	double_work_window(void)
{
	UBYTE	*ptr_s,*ptr_d;
	SLONG	x=0,y=0;
	ptr_s=WorkWindow;
	ptr_d=WorkWindow;
	if(Keys[KB_D])
	{
		for(y=(WorkWindowHeight/2)-1;y>=0;y--)
		{
			for(x=(WorkWindowWidth/2)-1;x>=0;x--)
			{
				ptr_d[(x*2)+(y*2*WorkScreenWidth)]=ptr_s[(x)+(y*WorkScreenWidth)];
				ptr_d[(x*2+1)+(y*2*WorkScreenWidth)]=ptr_s[(x)+(y*WorkScreenWidth)];
			}
		}
		for(y=1;y<WorkWindowHeight;y+=2)
		{
			for(x=0;x<WorkWindowWidth;x++)
			{
				ptr_d[(x)+(y*WorkScreenWidth)]=ptr_d[(x)+((y-1)*WorkScreenWidth)];
			}
		}
	}
	else
	{

		for(y=(WorkWindowHeight/2)-1;y>=0;y--)
		{
			for(x=(WorkWindowWidth/2)-1;x>=0;x--)
			{
				ptr_d[(x*2)+(y*2*WorkScreenWidth)]=ptr_s[(x)+(y*WorkScreenWidth)];
			}
		}
		for(y=0;y<WorkWindowHeight;y+=2)
		{
			for(x=1;x<WorkWindowWidth;x+=2)
			{
				ptr_d[(x)+(y*WorkScreenWidth)]=mix_map[ptr_d[(x-1)+(y*WorkScreenWidth)]+(ptr_d[(x+1)+(y*WorkScreenWidth)]<<8)];
			}
		}

		for(y=1;y<WorkWindowHeight;y+=2)
		{
			for(x=0;x<WorkWindowWidth;x++)
			{
				ptr_d[(x)+(y*WorkScreenWidth)]=mix_map[ptr_d[(x)+((y-1)*WorkScreenWidth)]+(ptr_d[(x)+((y+1)*WorkScreenWidth)]<<8)];
			}
		}
	}



/*
	ptr_d=dest+DITHER_WIDTH;
	for(y=0;y<32;y++)
	{
		for(x=0;x<64;x++)
		{
			*ptr_d=mix_map[*(ptr_d+DITHER_WIDTH)+(*(ptr_d-DITHER_WIDTH)<<8)];
			ptr_d++;
		}
		ptr_d+=DITHER_WIDTH*2-64;
	}
*/
}





//-----------------------------------------------------------------------------



/****************************************************************************/
/* END																		*/
/****************************************************************************/

