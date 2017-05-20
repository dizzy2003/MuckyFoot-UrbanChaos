// Sprites.cpp
// Guy Simmons, 13th February 1997.

#include	<MFHeader.h>
extern	UBYTE					CurrentPalette[256*3];

#define	RGB_TO_RGB565(r,g,b)		(UWORD)(((r>>4)<<11)|((g>>3)<<5)|(b>>4))
#define	RGB_TO_RGB888(r,g,b)		((r<<16)|(g<<8)|(b))
#define COL_TO_RGB565(col,PALETTE)	(UWORD)(((PALETTE[(col)*3]>>3)<<11)|((PALETTE[(col)*3+1]>>2)<<5)|(PALETTE[(col)*3+2]>>3))
#define COL_TO_RGB888(col,PALETTE)	((PALETTE[(col)*3]<<16)|(PALETTE[(col)*3+1]<<8)|PALETTE[(col)*3+2])

#define	DRAW_SPRITE		while(1)										\
				   		{												\
				   			packet	=	*src_ptr++;						\
				   			switch(packet)								\
				   			{											\
				   				case	END_LINE:						\
				   					dst_ptr		+=	WorkScreenWidth;	\
				   					line_ptr	=	dst_ptr;			\
				   					break;								\
				   				case	COPY_PIXELS:					\
				   					c0			=	(*src_ptr++)+1;		\
				   					while(c0--)							\
				   						*line_ptr++	=	*src_ptr++;		\
				   					break;								\
				   				case	SKIP_PIXELS:					\
				   					line_ptr	+=	(*src_ptr++)+1;		\
				   					break;								\
				   				case	DUPLICATE_PIXELS:				\
				   					c0			=	(*src_ptr++)+1;		\
				   					dup_pixel	=	*src_ptr++;			\
				   					while(c0--)							\
				   						*line_ptr++	=	dup_pixel;		\
				   					break;								\
				   				case	FINISHED:						\
				   					return;								\
				   			}											\
				   		}

#define	DRAW_SPRITE16(pal)	while(1)									\
				   		{												\
				   			packet	=	*src_ptr++;						\
				   			switch(packet)								\
				   			{											\
				   				case	END_LINE:						\
				   					dst_ptr	+=	WorkScreenPixelWidth;	\
				   					line_ptr	=	dst_ptr;			\
				   					break;								\
				   				case	COPY_PIXELS:					\
				   					c0			=	(*src_ptr++)+1;		\
				   					while(c0--)							\
				   					{									\
				   						*line_ptr++	=COL_TO_RGB565(*src_ptr,pal);		\
										src_ptr++;						\
				   					}									\
				   					break;								\
				   				case	SKIP_PIXELS:					\
				   					line_ptr	+=	*(src_ptr++)+1;		\
				   					break;								\
				   				case	DUPLICATE_PIXELS:				\
				   					c0			=	(*src_ptr++)+1;		\
				   					dup_pixel	=COL_TO_RGB565(*src_ptr,pal);			\
									src_ptr++;							\
				   					while(c0--)							\
				   						*line_ptr++	=	dup_pixel;		\
				   					break;								\
				   				case	FINISHED:						\
				   					return;								\
				   			}											\
				   		}

#define	DRAW_SPRITE32(pal)	while(1)									\
				   		{												\
				   			packet	=	*src_ptr++;						\
				   			switch(packet)								\
				   			{											\
				   				case	END_LINE:						\
				   					dst_ptr		+=	WorkScreenPixelWidth;	\
				   					line_ptr	=	dst_ptr;			\
				   					break;								\
				   				case	COPY_PIXELS:					\
				   					c0			=	(*src_ptr++)+1;		\
				   					while(c0--)							\
				   					{									\
				   						*line_ptr++	=COL_TO_RGB888(*src_ptr,pal);		\
										src_ptr++;						\
				   					}									\
				   					break;								\
				   				case	SKIP_PIXELS:					\
				   					line_ptr	+=	*(src_ptr++)+1;		\
				   					break;								\
				   				case	DUPLICATE_PIXELS:				\
				   					c0			=	(*src_ptr++)+1;		\
				   					dup_pixel	=COL_TO_RGB888(*src_ptr,pal);			\
									src_ptr++;							\
				   					while(c0--)							\
				   						*line_ptr++	=	dup_pixel;		\
				   					break;								\
				   				case	FINISHED:						\
				   					return;								\
				   			}											\
				   		}

#define	DRAW_M_SPRITE		while(1)									\
				   		{												\
				   			packet	=	*src_ptr++;						\
				   			switch(packet)								\
				   			{											\
				   				case	END_LINE:						\
				   					dst_ptr		+=	WorkScreenPixelWidth;	\
				   					line_ptr	=	dst_ptr;			\
				   					break;								\
				   				case	COPY_PIXELS:					\
				   					c0			=	(*src_ptr++)+1;		\
									src_ptr	+=	c0;						\
				   					while(c0--)							\
				   						*line_ptr++	=	dup_pixel;		\
				   					break;								\
				   				case	SKIP_PIXELS:					\
				   					line_ptr	+=	(*src_ptr++)+1;		\
				   					break;								\
				   				case	DUPLICATE_PIXELS:				\
				   					c0			=	(*src_ptr++)+1;		\
				   					src_ptr++;							\
				   					while(c0--)							\
				   						*line_ptr++	=	dup_pixel;		\
				   					break;								\
				   				case	FINISHED:						\
				   					return;								\
				   			}											\
				   		}


#define	V_SCAN			while(v_scan)									\
						{												\
							packet	=	*src_ptr++;						\
							switch(packet)								\
							{											\
								case	END_LINE:						\
									v_scan--;							\
									break;								\
								case	COPY_PIXELS:					\
									src_ptr	+=	1+*(src_ptr++);			\
									break;								\
								case	SKIP_PIXELS:					\
									src_ptr++;							\
									break;								\
								case	DUPLICATE_PIXELS:				\
									src_ptr	+=	2;						\
									break;								\
							}											\
						}

#define	L_SCAN			while(l_scan&&sprite_height)					\
						{												\
							packet	=	*src_ptr++;						\
							switch(packet)								\
							{											\
								case	END_LINE:						\
				   					dst_ptr	   +=	WorkScreenPixelWidth;	\
				   					line_ptr	=	dst_ptr;			\
									sprite_height--;					\
									pixel_count	=	0;					\
									break;								\
								case	COPY_PIXELS:					\
				   					c0			=	(*src_ptr++)+1;		\
									if((pixel_count+c0)>=l_scan)		\
									{									\
										src_ptr	+=	l_scan-pixel_count;	\
										c0		-=	l_scan-pixel_count;	\
										pixel_count	=	l_scan;			\
										goto copy_pixels;				\
									}									\
									else								\
									{									\
										pixel_count	+=	c0;				\
										src_ptr		+=	c0;				\
									}									\
									break;								\
								case	SKIP_PIXELS:					\
									c0	=	(*src_ptr++)+1;				\
									if((pixel_count+c0)>=l_scan)		\
									{									\
										c0		-=	l_scan-pixel_count;	\
										pixel_count	=	l_scan;			\
										goto skip_pixels;				\
									}									\
									else								\
									{									\
										pixel_count	+=	c0;				\
									}									\
									break;								\
								case	DUPLICATE_PIXELS:				\
									c0	=	(*src_ptr++)+1;				\
									if((pixel_count+c0)>=l_scan)		\
									{									\
										c0	-=	l_scan-pixel_count;		\
										pixel_count	=	l_scan;			\
										goto duplicate_pixels;			\
									}									\
									else								\
									{									\
										pixel_count	+=	c0;				\
										src_ptr++;						\
									}									\
									break;								\
				   				case	FINISHED:						\
				   					return;								\
							}											\
						}

#define	R_SCAN			while(1)										\
						{												\
							packet	=	*src_ptr++;						\
							switch(packet)								\
							{											\
								case	END_LINE:						\
									goto end_line;						\
								case	COPY_PIXELS:					\
									src_ptr	+=	1+*(src_ptr++);			\
									break;								\
								case	SKIP_PIXELS:					\
									src_ptr++;							\
									break;								\
								case	DUPLICATE_PIXELS:				\
									src_ptr	+=	2;						\
									break;								\
				   				case	FINISHED:						\
				   					return;								\
							}											\
						}


//---------------------------------------------------------------


void	DrawBSpritePal16(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal)
{
	UBYTE		packet,
				*src_ptr;
	ULONG		c0;
	UWORD		dup_pixel,
				*dst_ptr,
				*line_ptr;


	dst_ptr		=	(UWORD*)WorkWindow+x+(y*WorkScreenPixelWidth);
	line_ptr	=	dst_ptr;
	src_ptr		=	the_sprite->SpriteData;
	DRAW_SPRITE16(pal)
}

void	DrawBSpritePal32(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal)
{
	UBYTE		packet,
				*src_ptr;
	ULONG		c0;
	ULONG		dup_pixel,
				*dst_ptr,
				*line_ptr;


	dst_ptr		=	(ULONG*)WorkWindow+x+(y*WorkScreenPixelWidth);
	line_ptr	=	dst_ptr;
	src_ptr		=	the_sprite->SpriteData;
	DRAW_SPRITE32(pal)
}

void	DrawBSprite8(SLONG x,SLONG y,BSprite *the_sprite)
{
	UBYTE		dup_pixel,
				packet,
				*dst_ptr,
				*line_ptr,
				*src_ptr;
	ULONG		c0;


	dst_ptr		=	WorkWindow+x+(y*WorkScreenWidth);
	line_ptr	=	dst_ptr;
	src_ptr		=	the_sprite->SpriteData;
	DRAW_SPRITE
}

void	DrawBSprite16(SLONG x,SLONG y,BSprite *the_sprite)
{
	DrawBSpritePal16(x,y,the_sprite,CurrentPalette);
}

void	DrawBSprite32(SLONG x,SLONG y,BSprite *the_sprite)
{
	DrawBSpritePal32(x,y,the_sprite,CurrentPalette);
}

//---------------------------------------------------------------

void	DrawMonoBSprite8(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour)
{
	UBYTE		dup_pixel,
				packet,
				*dst_ptr,
				*line_ptr,
				*src_ptr;
	ULONG		c0;

	dst_ptr		=	WorkWindow+x+(y*WorkScreenWidth);
	line_ptr	=	dst_ptr;
	src_ptr		=	the_sprite->SpriteData;
	dup_pixel	=	(UBYTE)colour;
	DRAW_M_SPRITE
}

void	DrawMonoBSprite16(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour)
{
	UBYTE		packet,
				*src_ptr;
	ULONG		c0;

	UWORD		dup_pixel,
				*dst_ptr,
				*line_ptr;

	dst_ptr		=	(UWORD*)WorkWindow+x+(y*WorkScreenPixelWidth);
	line_ptr	=	dst_ptr;
	src_ptr		=	the_sprite->SpriteData;
	dup_pixel	=	(UWORD)colour;
	DRAW_M_SPRITE
}

void	DrawMonoBSprite32(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour)
{
	UBYTE		packet,
				*src_ptr;
	ULONG		c0;

	ULONG		dup_pixel,
				*dst_ptr,
				*line_ptr;

	dst_ptr		=	(ULONG*)WorkWindow+x+(y*WorkScreenPixelWidth);
	line_ptr	=	dst_ptr;
	src_ptr		=	the_sprite->SpriteData;
	dup_pixel	=	(ULONG)colour;
	DRAW_M_SPRITE
}

void	DrawMonoBSpriteC8(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour)
{
	if(x<0 || (x+the_sprite->SpriteWidth)>=WorkWindowWidth || y<0 || (y+the_sprite->SpriteHeight)>=WorkWindowHeight)
		return;

	DrawMonoBSprite8(x,y,the_sprite,colour);
}

void	DrawMonoBSpriteC16(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour)
{
	if(x<0 || (x+the_sprite->SpriteWidth)>=WorkWindowWidth || y<0 || (y+the_sprite->SpriteHeight)>=WorkWindowHeight)
		return;

	DrawMonoBSprite16(x,y,the_sprite,colour);
}

void	DrawMonoBSpriteC32(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour)
{
	if(x<0 || (x+the_sprite->SpriteWidth)>=WorkWindowWidth || y<0 || (y+the_sprite->SpriteHeight)>=WorkWindowHeight)
		return;

	DrawMonoBSprite32(x,y,the_sprite,colour);
}

//---------------------------------------------------------------

void	DrawBSpriteC8(SLONG x,SLONG y,BSprite *the_sprite)
{
	UBYTE		dup_pixel,
				packet,
				*dst_ptr,
				*line_ptr,
				*src_ptr;
	ULONG		c0,
				clip,
				count_diff,
				pixel_count,
				sprite_height;
	ULONG		l_scan,
				v_scan;
	SLONG		sprite_x;


	if(x>=WorkWindowWidth || (x+the_sprite->SpriteWidth)<0 || y>=WorkWindowHeight || (y+the_sprite->SpriteHeight)<0)
		return;

	sprite_height	=	the_sprite->SpriteHeight;
	sprite_x		=	x;
	src_ptr		=	the_sprite->SpriteData;
	clip		=	0;
	l_scan		=	0;


	if(x<0)
	{
		l_scan	=	-x;
		sprite_x=	0;
		clip	=	1;
	}
	if(y<0)
	{
		v_scan			=	-y;
		sprite_height	+=	y;
		V_SCAN
		y		=	0;
		clip	=	1;
	}
	if((SLONG)(x+the_sprite->SpriteWidth)>=WorkWindowWidth)
	{
		clip	=	1;
	}
	if((SLONG)(y+sprite_height)>=WorkWindowHeight)
	{
		sprite_height	-=	(y+sprite_height)-WorkWindowHeight;
		clip	=	1;
	}

	dst_ptr		=	WorkWindow+sprite_x+(y*WorkScreenWidth);
	line_ptr	=	dst_ptr;

	if(clip)
	{
		pixel_count	=	0;
		L_SCAN
		while(sprite_height)
   		{
   			packet		=	*src_ptr++;
   			switch(packet)
   			{
   				case	END_LINE:
end_line:
   					dst_ptr		+=	WorkScreenWidth;
   					line_ptr	=	dst_ptr;
					sprite_height--;
					pixel_count	=	0;
					L_SCAN
   					break;
   				case	COPY_PIXELS:
   					c0			=	(*src_ptr++)+1;
copy_pixels:
					pixel_count	+=	c0;
					if((SLONG)(x+pixel_count)>=WorkWindowWidth)
					{
						count_diff	=	(x+pixel_count)-WorkWindowWidth;
						c0	-=	count_diff;
	   					while(c0--)
	   						*line_ptr++	=	*src_ptr++;
						src_ptr	+=	count_diff;
						R_SCAN
					}
					else
					{
	   					while(c0--)
	   						*line_ptr++	=	*src_ptr++;
					}
   					break;
   				case	SKIP_PIXELS:
					c0			=	(*src_ptr++)+1;
skip_pixels:
					pixel_count	+=	c0;
					if((SLONG)(x+pixel_count)>=WorkWindowWidth)
					{
						R_SCAN
					}
					else
	   					line_ptr	+=	c0;
   					break;
   				case	DUPLICATE_PIXELS:
   					c0			=	(*src_ptr++)+1;
duplicate_pixels:
					pixel_count	+=	c0;
   					dup_pixel	=	*src_ptr++;
					if((SLONG)(x+pixel_count)>=WorkWindowWidth)
					{
						c0	-=	(x+pixel_count)-WorkWindowWidth;
	   					while(c0--)
	   						*line_ptr++	=	dup_pixel;
						R_SCAN
					}
					else
					{
	   					while(c0--)
	   						*line_ptr++	=	dup_pixel;
					}
   					break;
   				case	FINISHED:
   					return;
   			}
   		}
   	}
	else
	{
		DRAW_SPRITE
	}
}

extern	void	DrawBSpritePalC16(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);
extern	void	DrawBSpritePalC32(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);

void	DrawBSpriteC16(SLONG x,SLONG y,BSprite *the_sprite)
{
	DrawBSpritePalC16(x,y,the_sprite,CurrentPalette);	
}

void	DrawBSpriteC32(SLONG x,SLONG y,BSprite *the_sprite)
{
	DrawBSpritePalC32(x,y,the_sprite,CurrentPalette);	
}

void	DrawBSpritePalC16(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal)
{
	UBYTE		packet,
				*src_ptr;
	ULONG		c0,
				clip,
				count_diff,
				pixel_count,
				sprite_height;
	ULONG		l_scan,
				v_scan;
	SLONG		sprite_x;

	UWORD		dup_pixel,
				*dst_ptr,
				*line_ptr;


	if(x>=WorkWindowWidth || (x+the_sprite->SpriteWidth)<0  || y>=WorkWindowHeight ||  (y+the_sprite->SpriteHeight)<0)
		return;

	sprite_height	=	the_sprite->SpriteHeight;
	sprite_x		=	x;
	src_ptr		=	the_sprite->SpriteData;
	clip		=	0;
	l_scan		=	0;


	if(x<0)
	{
		l_scan	=	-x;
		sprite_x=	0;
		clip	=	1;
	}
	if(y<0)
	{
		v_scan			=	-y;
		sprite_height	+=	y;
		V_SCAN
		y		=	0;
		clip	=	1;
	}
	if((SLONG)(x+the_sprite->SpriteWidth)>=WorkWindowWidth)
	{
		clip	=	1;
	}
	if((SLONG)(y+sprite_height)>=WorkWindowHeight)
	{
		sprite_height	-=	(y+sprite_height)-WorkWindowHeight;
		clip	=	1;
	}

	dst_ptr		=	(UWORD*)WorkWindow+sprite_x+(y*WorkScreenPixelWidth);
	line_ptr	=	dst_ptr;

	if(clip)
	{
		pixel_count	=	0;
		L_SCAN
		while(sprite_height)
   		{
   			packet		=	*src_ptr++;
   			switch(packet)
   			{
   				case	END_LINE:
end_line:
   					dst_ptr		+=	WorkScreenPixelWidth;
   					line_ptr	=	dst_ptr;
					sprite_height--;
					pixel_count	=	0;
					L_SCAN
   					break;
   				case	COPY_PIXELS:
   					c0			=	(*src_ptr++)+1;
copy_pixels:
					pixel_count	+=	c0;
					if((SLONG)(x+pixel_count)>=WorkWindowWidth)
					{
						count_diff	=	(x+pixel_count)-WorkWindowWidth;
						c0	-=	count_diff;
	   					while(c0--)
	   					{
	   						*line_ptr++	=COL_TO_RGB565(*src_ptr,pal);		
							src_ptr++;
	   					}
						src_ptr	+=	count_diff;
						R_SCAN
					}
					else
					{
	   					while(c0--)
	   					{
	   						*line_ptr++	=COL_TO_RGB565(*src_ptr,pal);		
							src_ptr++;
	   					}
					}
   					break;
   				case	SKIP_PIXELS:
					c0			=	(*src_ptr++)+1;
skip_pixels:
					pixel_count	+=	c0;
					if((SLONG)(x+pixel_count)>=WorkWindowWidth)
					{
						R_SCAN
					}
					else
	   					line_ptr	+=	c0;
   					break;
   				case	DUPLICATE_PIXELS:
   					c0			=	(*src_ptr++)+1;
duplicate_pixels:
					pixel_count	+=	c0;
   					dup_pixel	=	COL_TO_RGB565(*src_ptr,pal);
					src_ptr++;
	   						
					if((SLONG)(x+pixel_count)>=WorkWindowWidth)
					{
						c0	-=	(x+pixel_count)-WorkWindowWidth;
	   					while(c0--)
	   						*line_ptr++	=	dup_pixel;
						R_SCAN
					}
					else
					{
	   					while(c0--)
	   						*line_ptr++	=	dup_pixel;
					}
   					break;
   				case	FINISHED:
   					return;
   			}
   		}
   	}
	else
	{
		DRAW_SPRITE16(pal)
	}
}

void	DrawBSpritePalC32(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal)
{
	UBYTE		packet,
				*src_ptr;
	ULONG		c0,
				clip,
				count_diff,
				pixel_count,
				sprite_height;
	ULONG		l_scan,
				v_scan;
	SLONG		sprite_x;

	ULONG		dup_pixel,
				*dst_ptr,
				*line_ptr;


	if(x>=WorkWindowWidth || (x+the_sprite->SpriteWidth)<0 || y>=WorkWindowHeight || (y+the_sprite->SpriteHeight)<0)
		return;

	sprite_height	=	the_sprite->SpriteHeight;
	sprite_x		=	x;
	src_ptr		=	the_sprite->SpriteData;
	clip		=	0;
	l_scan		=	0;


	if(x<0)
	{
		l_scan	=	-x;
		sprite_x=	0;
		clip	=	1;
	}
	if(y<0)
	{
		v_scan			=	-y;
		sprite_height	+=	y;
		V_SCAN
		y		=	0;
		clip	=	1;
	}
	if((SLONG)(x+the_sprite->SpriteWidth)>=WorkWindowWidth)
	{
		clip	=	1;
	}
	if((SLONG)(y+sprite_height)>=WorkWindowHeight)
	{
		sprite_height	-=	(y+sprite_height)-WorkWindowHeight;
		clip	=	1;
	}

	dst_ptr		=	(ULONG*)WorkWindow+sprite_x+(y*WorkScreenPixelWidth);
	line_ptr	=	dst_ptr;

	if(clip)
	{
		pixel_count	=	0;
		L_SCAN
		while(sprite_height)
   		{
   			packet		=	*src_ptr++;
   			switch(packet)
   			{
   				case	END_LINE:
end_line:
   					dst_ptr		+=	WorkScreenPixelWidth;
   					line_ptr	=	dst_ptr;
					sprite_height--;
					pixel_count	=	0;
					L_SCAN
   					break;
   				case	COPY_PIXELS:
   					c0			=	(*src_ptr++)+1;
copy_pixels:
					pixel_count	+=	c0;
					if((SLONG)(x+pixel_count)>=WorkWindowWidth)
					{
						count_diff	=	(x+pixel_count)-WorkWindowWidth;
						c0	-=	count_diff;
	   					while(c0--)
	   					{									
	   						*line_ptr++	=COL_TO_RGB888(*src_ptr,pal);		
							src_ptr++;						
	   					}									
						src_ptr	+=	count_diff;
						R_SCAN
					}
					else
					{
	   					while(c0--)
	   					{									
	   						*line_ptr++	=COL_TO_RGB888(*src_ptr,pal);		
							src_ptr++;						
	   					}									
					}
   					break;
   				case	SKIP_PIXELS:
					c0			=	(*src_ptr++)+1;
skip_pixels:
					pixel_count	+=	c0;
					if((SLONG)(x+pixel_count)>=WorkWindowWidth)
					{
						R_SCAN
					}
					else
	   					line_ptr	+=	c0;
   					break;
   				case	DUPLICATE_PIXELS:
   					c0			=	(*src_ptr++)+1;
duplicate_pixels:
					pixel_count	+=	c0;
   					dup_pixel	=	COL_TO_RGB888(*src_ptr,pal);
					src_ptr++;
					if((SLONG)(x+pixel_count)>=WorkWindowWidth)
					{
						c0	-=	(x+pixel_count)-WorkWindowWidth;
	   					while(c0--)
	   						*line_ptr++	=	dup_pixel;
						R_SCAN
					}
					else
					{
	   					while(c0--)
	   						*line_ptr++	=	dup_pixel;
					}
   					break;
   				case	FINISHED:
   					return;
   			}
   		}
   	}
	else
	{
		DRAW_SPRITE32(pal)
	}
}

//---------------------------------------------------------------

void	SetupBSprites(BSprite *sprite_ref,UBYTE *sprite_data)
{
	ULONG		spr_count;


	spr_count	=	*(ULONG*)(&sprite_ref->SpriteHeight);
	sprite_ref++;
	while(spr_count--)
	{
		sprite_ref->SpriteData	+=	(ULONG)sprite_data;
		sprite_ref++;
	}
}

//---------------------------------------------------------------
