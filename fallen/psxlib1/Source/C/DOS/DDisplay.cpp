#include	<MFHeader.h>
#include	"I86.h"
#include	"string.h"
#include	"stdio.h"
#include	"conio.h"
#include	"dos.h"

typedef	struct
{
	ULONG	MyEDI,MyESI,MyEBP,MyReserved,MyEBX,MyEDX,MyECX,MyEAX;
	UWORD	MyFlags,MyES,MyDS,MyFS,MyGS,MyIP,MyCS,MySP,MySS;
}
	TbRMREGS2;

UBYTE					*WorkScreen,
						WorkScreenDepth;
SLONG					WorkScreenHeight,
						WorkScreenWidth,
						WorkScreenPixelWidth;
UBYTE					CurrentPalette[256*3];


extern	SLONG	SetupMouse(void);
extern	SLONG	ResetMouse(void);
extern	void	SetDrawFunctions(ULONG depth);


//------------------------------------------------------------

SWORD					VesaBytesPerLine	= 0;
SWORD					VesaHRes			= 0;
SWORD					VesaVRes			= 0;
UBYTE					*VesaData=0;		
SWORD   				VesaPage			= 0;
SWORD					VesaGran			= 0;


SWORD	vesa_modes[]=
{
	0x13,0x100,0x101,0x103,0x105,0x107,0,0,0,0,0,0,0,0,0,0,0,0,0
};	

//-----------------------------------------------------------------------

extern	SWORD	video_int(SWORD,SWORD,SWORD,SWORD);
#pragma aux		video_int=\
				"push	bp ",\
				"int	10h ",\
				"pop	bp ",\
				parm caller [ax] [bx] [cx] [dx] value [ax];

				

SLONG	VesaGetGran(SWORD mode)
{
	union	REGS	inregs;
	union	REGS	outregs;
	struct	SREGS	ssregs;
	TbRMREGS2		rmregs;

	memset(&rmregs,0,sizeof(TbRMREGS2));
	rmregs.MyES=(UWORD)(((ULONG)VesaData)>>4);
	rmregs.MyDS=(UWORD)(((ULONG)VesaData)>>4);
	rmregs.MyEDI=0;
	rmregs.MyEAX=0x4f*256+1;
	rmregs.MyECX=mode;

	memset(&inregs,0,sizeof(union REGS));
	memset(&outregs,0,sizeof(union REGS));
	memset(&ssregs,0,sizeof(struct SREGS));
	segread(&ssregs);
	inregs.x.eax=0x300;
	inregs.x.ebx=0x010;
	inregs.x.edi=(ULONG)&rmregs;
	int386x(0x31,&inregs,&outregs,&ssregs);

	VesaGran=(SWORD)(VesaData[5]*256+VesaData[4]);
	VesaBytesPerLine=(SWORD)(VesaData[17]*256+VesaData[16]);
	VesaHRes=(SWORD)(VesaData[19]*256+VesaData[18]);
	VesaVRes=(SWORD)(VesaData[21]*256+VesaData[20]);
	
	return (1);
}


SLONG	VesaSetMode(SWORD bx)
{
	union	REGS	inregs;
	union	REGS	outregs;
	memset(&inregs,0,sizeof(union REGS));
	memset(&outregs,0,sizeof(union REGS));
	inregs.h.ah=0x4f;
	inregs.h.al=2;
	inregs.w.bx=bx;
	int386(0x10,&inregs,&outregs);
	VesaGetGran(bx);

	return (1);
}


SLONG	VesaSetPage(SWORD dx)
{
	if (VesaPage != dx)
	{
		video_int(0x4f05,0,0,(SWORD)((dx*64)/VesaGran));
		video_int(0x4f05,1,0,(SWORD)((dx*64)/VesaGran));

		VesaPage = dx;
	}
	return (1);
}


SLONG	VesaGetInfo(void)
{
	union	REGS	inregs;
	union	REGS	outregs;
	struct	SREGS	ssregs;
	TbRMREGS2			rmregs;

	memset(&rmregs,0,sizeof(TbRMREGS2));
	rmregs.MyES=(UWORD)(((ULONG)VesaData)>>4);
	rmregs.MyDS=(UWORD)(((ULONG)VesaData)>>4);
	rmregs.MyEDI=0;
	rmregs.MyEAX=0x4f*256;
	rmregs.MyEBX=0x101;

	memset(&inregs,0,sizeof(union REGS));
	memset(&outregs,0,sizeof(union REGS));
	memset(&ssregs,0,sizeof(struct SREGS));
	segread(&ssregs);
	inregs.x.eax=0x300;
	inregs.x.ebx=0x010;
	inregs.x.edi=(ULONG)&rmregs;
	int386x(0x31,&inregs,&outregs,&ssregs);

	return ((strncmp((CBYTE*)VesaData,"VESA",4) == 0) ? 1 : 0);
}


UBYTE	VesaIsModeAvailable(UWORD mode)
{
	UWORD	*wp;
	SLONG	t;

	if(VesaGetInfo() == 1)
	{
		wp = (UWORD*)&VesaData[14];
		t = *wp;
		wp++;
		t += *wp<<4;
		wp = (UWORD*)t;
		for(;*wp != 0xffff; wp++)
		{
			if(*wp == mode)
			{
				return (1);
			}
		}
	}
	return (0);
}

#define	DPMI_INT	(0x31)

void *allocDOS(unsigned long int nbytes, short int *pseg, short int *psel)
{
	union  REGS     regs;
	struct SREGS    sregs;
  	unsigned npara = (nbytes + 15) / 16;
	void *pprot;

  	pprot = NULL;
  	*pseg = 0;			// assume will fail
  	*psel = 0;

	// DPMI call 100h allocates DOS memory
	segread(&sregs);
	regs.x.eax = 0x0100;			// DPMI: Allocate DOS Memory
	regs.x.ebx = npara;			// number of paragraphs to alloc

	int386(DPMI_INT,&regs,&regs);

	if (regs.w.cflag == 0)
	{
	  *pseg = regs.x.eax;	// the real-mode segment
	  *psel = regs.x.edx;	// equivalent protected-mode selector
	  // pprot is the protected mode address of the same allocated block.
	  // The Rational extender maps the 1 MB physical DOS memory into
	  // the bottom of our virtual address space.
	  pprot = (void *)((unsigned)*pseg << 4);
	}
	return(pprot);   
}

void freeDOS(short int sel)
{
   union  REGS     regs;
   regs.x.eax = 0x0101;		// DPMI free DOS memory
   regs.x.edx = sel;

   int386(DPMI_INT,&regs,&regs);
}

struct	ScreenModes
{
	UWORD	Mode;
	UWORD	Width;
	UWORD	Height;
};

struct	ScreenModes screen_modes[]=
{
	{0x13,320,200},
	{0x13,320,200},
	{0x100,640,400},
	{0x101,640,480},
	{0x103,800,600},
	{0x105,1024,768},
	{0x107,1280,1024},
	{0,0,0}
};

#define	SCREEN_MODE_320_200_8			1
#define	SCREEN_MODE_640_400_8			2
#define	SCREEN_MODE_640_480_8			3
#define	SCREEN_MODE_800_600_8			4
#define	SCREEN_MODE_1024_768_8			5
#define	SCREEN_MODE_1280_1024_8			6

UWORD	old_display_mode=0;

UWORD	find_mode(SLONG width,SLONG height)
{
	SLONG	c0=1;

	while(screen_modes[c0].Mode)
	{
		if(screen_modes[c0].Width==width && screen_modes[c0].Height==height)
			return(screen_modes[c0].Mode);
		c0++;
	}
	printf(" res %d * %d not found \n",width,height);
	return(0);
}

	
void MEM_COPY_LONG(UBYTE *,UBYTE *, ULONG);
#pragma aux MEM_COPY_LONG = \
		"rep 	movsd"\
		parm [edi] [esi] [ecx]\
		modify [edi esi ecx];

static	vesa_flag=0;
static	screen_setup=0;

static	SWORD	sect1, sect2;
static	UBYTE	alloc_for_vesa=0;
static	UBYTE	*screen_mem=0;
SLONG	OpenDisplay(ULONG width, ULONG height, ULONG depth)
{
	if(SetDisplay(width,height,depth)==0)
		return(0);
	SetDrawFunctions(8);
	SetupMouse();
	return(1);
}	

SLONG	CloseDisplay(void)
{
	ResetMouse();
	if(alloc_for_vesa)
		freeDOS(sect2);
	if(old_display_mode)
		VesaSetMode((SWORD)old_display_mode);

	if(screen_mem)
		MemFree((void*)screen_mem);

	return(1);
}

SLONG	SetDisplay(ULONG width,ULONG height,ULONG depth)
{
	union	REGS		inregs;
	union	REGS		outregs;
	SWORD	mode;

	SetDrawFunctions(8);
	if(!alloc_for_vesa)
	{
		alloc_for_vesa=1;
		VesaData=(UBYTE*)allocDOS(512,&sect1,&sect2);
	}	
	if(!screen_mem)
		screen_mem=(UBYTE*)MemAlloc(1280*1024);

	if(!screen_mem)
		return(0);

	if((mode=find_mode(width,height))==0)
		return(0);

	if(old_display_mode==0)
	{
		inregs.x.eax=0x0f00;	
		int386(0x10,&inregs,&outregs);
		old_display_mode=outregs.h.al;
	}
	if(mode==0x13)
	{
		inregs.x.eax = mode;
		int386(0x10,&inregs,&outregs);
		vesa_flag=0;
		screen_setup=1;
	}
	else
	{
		
		if(VesaSetMode((SWORD)mode))
			screen_setup=1;
		vesa_flag=1;
	}
	WorkScreenPixelWidth=width;
	WorkScreenWidth=width;
	WorkScreenHeight=height;
	return(1);
}

void	ShowWorkScreen(ULONG flags)
{
	SLONG	copy_size;
	SLONG	page = 0;
	SLONG	pages;
	SLONG	size;
	UBYTE	*ptr;
	
	ptr = WorkScreen;
	if(!screen_setup)
		return;
//	LbMousePlace();

	if(!vesa_flag)
	{
		VesaSetPage((SWORD)0);
		MEM_COPY_LONG((UBYTE*) 0xa0000, WorkScreen, (320*200)>>2);
	}
	else
	{
		page=0;
		size = WorkScreenWidth*WorkScreenHeight;
		pages=size>>16;

		for(page=0; page<pages; page++)
		{
			if(size < 65536)
				copy_size = size;
			else
				copy_size = 65536;
			VesaSetPage((SWORD)page);
			MEM_COPY_LONG((UBYTE*) 0xa0000, ptr, copy_size>>2);
			ptr+=65536;
		}
	}
//	LbMouseRemove();

}

void	SetPalette(UBYTE *pal)
{
	UWORD	c0;

	outp(0x3c6, 0xff);  //tell card some palette things are a comin
	for	(c0 = 0; c0 < 256; ++c0)
	{
		outp(0x3c8, c0);
		outp(0x3c9, (*pal++));
		outp(0x3c9, (*pal++));
		outp(0x3c9, (*pal++));
	}
}



void	ClearWorkScreen(UBYTE colour)
{
	memset(WorkScreen,0,WorkScreenWidth*WorkScreenHeight);
}

void	FadeDisplay(UBYTE mode)
{
	
}

void	*LockWorkScreen(void)
{
	return((void*)WorkScreen);
}

void	UnlockWorkScreen(void)
{
	
}

void	ClearDisplay(void)
{
	
}

void	ShowWorkWindow(ULONG flags)
{
	SLONG	line;
	UBYTE	*dest,*source;
	UBYTE	current_page,page,left_page,right_page;
	ULONG	offset;


	offset=WorkWindowRect.Top*WorkScreenWidth+WorkWindowRect.Left;
	dest=(UBYTE*)0xa0000;
	source=WorkWindow;
	current_page=offset>>16;
	offset&=0xffff;

	VesaSetPage((UWORD)current_page);

	for(line=WorkWindowRect.Top;line<WorkWindowRect.Bottom;line++)
	{
		if(offset>=0x10000)
		{	//address out of range, so page change
			offset-=0x10000;
			VesaSetPage((UWORD)++current_page);
		}
		
		if((offset+WorkWindowWidth)>=0x10000)
		{
			//line spans 2 pages
			SLONG	copy_size;

			copy_size=0x10000-offset;
			memcpy(dest+offset,source,copy_size);
			VesaSetPage((UWORD)++current_page);
			offset-=0x10000;

			memcpy((UBYTE*)0xa0000,source+copy_size,WorkWindowWidth-copy_size);
		}
		else
			memcpy(dest+offset,source,WorkWindowWidth);

		offset+=WorkScreenWidth;
		source+=WorkScreenWidth;
	}
	
}


SLONG	FindColour(UBYTE *the_palette,SLONG r,SLONG g,SLONG b)
{
	SLONG	found	=	-1;

	if(r>255)
		r=255;
	if(g>255)
		g=255;
	if(b>255)
		b=255;

	switch(WorkScreenDepth)
	{
		case 1:
		{
			
			SLONG	dist	=	0x7fffffff,
					c0,
					dist2,
					tr,
					tg,
					tb;


			for(c0=0;c0<256;c0++)
			{
				tr	=	*the_palette++;
				tg	=	*the_palette++;
				tb	=	*the_palette++;

				tr	-=	r;
				tg	-=	g;
				tb	-=	b;

				dist2=	abs(tr*tr)+abs(tg*tg)+abs(tb*tb);
				if(dist2<dist)
				{
					found	=	c0;
					dist	=	dist2;
					if(dist<8)
						return(c0);
				}
			}
			break;
		}
		case 2:

			 found=(((r>>3)<<11)|((g>>2)<<5)|(b>>3));
			break;
		case 4:
			 found=((r<<16)|(g<<8)|(b));
			break;

	}
	return(found);
}
