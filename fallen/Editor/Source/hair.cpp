#include	"Editor.hpp"

//---------------------------------------------------------------

#define	MAX_STRANDS		1
#define	STRAND_PIECES	30
#define	STRAND_LENGTH	5
#define	STRAND_SHIFT	1
struct	Strand
{
	SLONG	X[STRAND_PIECES];
	SLONG	Y[STRAND_PIECES];
	SWORD	VX[STRAND_PIECES];
	SWORD	VY[STRAND_PIECES];
	SWORD	Length;
};

struct	Strand	strands[MAX_STRANDS];

void	init_hair(SLONG x,SLONG y,SLONG z)
{
	SLONG	c0,c1;

	for(c0=0;c0<MAX_STRANDS;c0++)
	{
		for (c1=0;c1<STRAND_PIECES;c1++)
		{
			strands[c0].X[c1]=(x+4*c0)<<STRAND_SHIFT;
			strands[c0].Y[c1]=(y+STRAND_LENGTH*c1)<<STRAND_SHIFT;
			strands[c0].VX[c1]=0;
			strands[c0].VY[c1]=0;
		}
	}
}


void	offset_strand(SLONG c0,SLONG c1,SLONG dx,SLONG dy)
{
	for (;c1<STRAND_PIECES;c1++)
	{
		strands[c0].X[c1]+=dx;
		strands[c0].X[c1]+=dy;
	}
}


inline	SLONG normalise_xy(SLONG *dx,SLONG *dy,SLONG size)
{
	SLONG	dist;
	SLONG	tx,ty;

	tx=abs(*dx);
	ty=abs(*dy);

	dist=QDIST2(tx,ty);
	if(dist==0)
		dist=1;

	*dx=(*dx*size)/dist;
	*dy=(*dy*size)/dist;

	return(dist);
}

void	move_hair_root(SLONG gdx,SLONG gdy,SLONG dz)
{
	SLONG	c0,c1;
	SLONG	dx,dy;
	for(c0=0;c0<MAX_STRANDS;c0++)
	{
		strands[c0].X[0]+=gdx<<STRAND_SHIFT;
		strands[c0].Y[0]+=gdy<<STRAND_SHIFT;
		for (c1=1;c1<STRAND_PIECES;c1++)
		{
			SLONG	dx,dy,vx,vy;
			dx=strands[c0].X[c1]-strands[c0].X[c1-1];
			dy=strands[c0].Y[c1]-strands[c0].Y[c1-1];
			normalise_xy(&dx,&dy,STRAND_LENGTH<<STRAND_SHIFT);


//			strands[c0].VX[c1]+=(gdx>>1)+(strands[c0].VX[c1-1]>>2);
//			strands[c0].VY[c1]+=(gdy>>1)+(strands[c0].VY[c1-1]>>2);

			strands[c0].X[c1]=strands[c0].X[c1-1]+dx;
			strands[c0].Y[c1]=strands[c0].Y[c1-1]+dy;
		}
	}
}

void	move_hair_velocity(void)
{
	SLONG	c0,c1;
	for(c0=0;c0<MAX_STRANDS;c0++)
	{
		for (c1=1;c1<STRAND_PIECES;c1++)
		{
			SLONG	dx,dy;
			strands[c0].VY[c1]+=1;
			strands[c0].X[c1]+=strands[c0].VX[c1];
			strands[c0].Y[c1]+=strands[c0].VY[c1];

			strands[c0].VX[c1]=(strands[c0].VX[c1]*200)>>8;
			strands[c0].VY[c1]=(strands[c0].VY[c1]*200)>>8;
			if(abs(strands[c0].VX[c1])<2)
				strands[c0].VX[c1]=0;
			if(abs(strands[c0].VY[c1])<2)
				strands[c0].VY[c1]=0;

		}
	}
}


void	draw_hair(void)
{
	static	SLONG	old_x=0,old_y=0,old_z=0;
	SLONG	dx,dy,dz;

	dz = 0;

	if(old_x==0)
	{
		dx=dy=0;
	}
	else
	{
		dx=MouseX-old_x;
		dy=MouseY-old_y;
		
	}
	old_x=MouseX;
	old_y=MouseY;
	move_hair_velocity();
//	if(dx||dy)
		move_hair_root(dx,dy,dz);


	{
		SLONG	c0,c1;
		for(c0=0;c0<MAX_STRANDS;c0++)
		{
			for (c1=0;c1<STRAND_PIECES-1;c1++)
			{
				CBYTE	str[100];
				DrawLineC(strands[c0].X[c1]>>STRAND_SHIFT,strands[c0].Y[c1]>>STRAND_SHIFT,strands[c0].X[c1+1]>>STRAND_SHIFT,strands[c0].Y[c1+1]>>STRAND_SHIFT,255);
				sprintf(str,"%d \n",strands[c0].VX[c1]);
				QuickTextC(0,c1*15,str,255);
			}
		}
	}
	if(Keys[KB_P])
	{
		while(Keys[KB_P])
		{
			
		}
	}
}


//---------------------------------------------------------------
