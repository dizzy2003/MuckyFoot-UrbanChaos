// QuickText.c
// Guy Simmons, 7th October 1996.

#include	"Editor.hpp"

extern UBYTE	*CharTable[];

#define	DRAW_CHAR		for(c0=char_height;c0;c0--,string_dest+=WorkScreenWidth)	\
						{															\
							for(c1=0;c1<char_width;c1++)							\
							{														\
								if(*(the_char_def++))									\
								{													\
									*(string_dest+c1)	=	(UBYTE)colour;			\
								}													\
							}														\
						}

#define	DRAW_CHAR16		for(c0=char_height;c0;c0--,string_dest+=WorkScreenWidth>>1)	\
						{															\
							for(c1=0;c1<char_width;c1++)							\
							{														\
								if(*(the_char_def++))									\
								{													\
									*(string_dest+c1)	=	(UWORD)colour;			\
								}													\
							}														\
						}

#define	DRAW_CHAR32		for(c0=char_height;c0;c0--,string_dest+=WorkScreenWidth>>2)	\
						{															\
							for(c1=0;c1<char_width;c1++)							\
							{														\
								if(*(the_char_def++))									\
								{													\
									*(string_dest+c1)	=	(ULONG)colour;			\
								}													\
							}														\
						}

//---------------------------------------------------------------

void	QuickText8(SLONG x,SLONG y,CBYTE *the_string,ULONG colour)
{
	UBYTE		*the_char_def,
				*string_dest;
	ULONG		char_height,
				char_width;
	ULONG		c0,c1;
	
	
	if(the_string)
	while(*the_string)
	{
		the_char_def	=	CharTable[*(the_string++)];
		char_width		=	*(the_char_def++);
		char_height		=	*(the_char_def++);
		string_dest		=	WorkWindow+x+((y+*(the_char_def++))*WorkScreenWidth);

		DRAW_CHAR
		x			+=	char_width+1;
	}
}

void	QuickText16(SLONG x,SLONG y,CBYTE *the_string,ULONG colour)
{
	UBYTE		*the_char_def;
	UWORD		*string_dest;
	ULONG		char_height,
				char_width;
	ULONG		c0,c1;
	
	
	if(the_string)
	while(*the_string)
	{
		the_char_def	=	CharTable[*(the_string++)];
		char_width		=	*(the_char_def++);
		char_height		=	*(the_char_def++);
		string_dest		=	(UWORD*)WorkWindow+x+((y+*(the_char_def++))*WorkScreenWidth>>1);

		DRAW_CHAR16
		x			+=	char_width+1;
	}
}

void	QuickText32(SLONG x,SLONG y,CBYTE *the_string,ULONG colour)
{
	UBYTE		*the_char_def;
	ULONG		*string_dest;
	ULONG		char_height,
				char_width;
	ULONG		c0,c1;
	
	
	if(the_string)
	while(*the_string)
	{
		the_char_def	=	CharTable[*(the_string++)];
		char_width		=	*(the_char_def++);
		char_height		=	*(the_char_def++);
		string_dest		=	(ULONG*)WorkWindow+x+((y+*(the_char_def++))*WorkScreenWidth>>2);

		DRAW_CHAR32
		x			+=	char_width+1;
	}
}

//---------------------------------------------------------------

void	QuickTextC8(SLONG x,SLONG y,CBYTE *the_string,ULONG colour)
{
	UBYTE		*string_dest,
				*the_char_def,
				*the_pixel;
	SLONG		c0,c1,
				char_draw_height,
				char_draw_width,
				char_height,
				char_width,
				char_v_offset,
				char_x_offset,
				char_y,
				char_y_offset,
				clip;
	

	if(x>=WorkWindowWidth || y>=WorkWindowHeight)
		return;

	if(the_string)
	while(*the_string)
	{
		the_char_def	=	CharTable[*(the_string++)];
		char_width		=	*(the_char_def++);
		char_height		=	*(the_char_def++);
		char_v_offset	=	*(the_char_def++);
		char_draw_width	=	char_width;
		char_draw_height=	char_height;
		char_x_offset	=	0;
		char_y_offset	=	0;
		char_y			=	y;

		clip	=	0;
		if(x<0)
		{
			if((x+char_width)<0)
				goto done_char;
			char_x_offset	=	-x;
			char_draw_width	+=	x;
			x		=	0;
			clip	=	1;
		}
		if(y<0)
		{
			if((y+char_v_offset+char_height)<0)
				goto done_char;
			if((y+char_v_offset)<0)
			{					
				char_y_offset	=	-(y+char_v_offset);
				char_draw_height+=	y+char_v_offset;
				char_v_offset	=	0;
			}
			else
			{
				char_v_offset	+=	y;
			}
			char_y	=	0;
			clip	=	1;
		}
		if((x+char_draw_width)>=WorkWindowWidth)
		{
			char_draw_width	-=	(x+char_draw_width)-WorkWindowWidth;
			clip	=	1;
		}
		if((y+char_v_offset+char_draw_height)>=WorkWindowHeight)
		{
			char_draw_height-=	(char_y+char_v_offset+char_draw_height)-WorkWindowHeight;
			if(char_draw_height<=0)
				goto done_char;
			clip	=	1;
		}

		string_dest	=	WorkWindow+x+((char_y+char_v_offset)*WorkScreenWidth);

		if(clip)
		{
			for(c0=0;c0<char_draw_height;c0++,string_dest+=WorkScreenWidth,the_char_def+=char_width)
			{
				the_pixel	=	the_char_def+(char_y_offset*char_width)+char_x_offset;
				for(c1=0;c1<char_draw_width;c1++)
				{
					if(*(the_pixel++))
					{
						*(string_dest+c1)	=	(UBYTE)colour;
					}
				}
			}
		}
		else
		{
			DRAW_CHAR
		}
done_char:
		x	+=	char_draw_width+1;
		if(x>=WorkWindowWidth)
			return;
	}
}

void	QuickTextC16(SLONG x,SLONG y,CBYTE *the_string,ULONG colour)
{
	UWORD		*string_dest;
	UBYTE		*the_char_def,
				*the_pixel;
	SLONG		c0,c1,
				char_draw_height,
				char_draw_width,
				char_height,
				char_width,
				char_v_offset,
				char_x_offset,
				char_y,
				char_y_offset,
				clip;
	

	if(x>=WorkWindowWidth || y>=WorkWindowHeight)
		return;

	if(the_string)
	while(*the_string)
	{
		the_char_def	=	CharTable[*(the_string++)];
		char_width		=	*(the_char_def++);
		char_height		=	*(the_char_def++);
		char_v_offset	=	*(the_char_def++);
		char_draw_width	=	char_width;
		char_draw_height=	char_height;
		char_x_offset	=	0;
		char_y_offset	=	0;
		char_y			=	y;

		clip	=	0;
		if(x<0)
		{
			if((x+char_width)<0)
				goto done_char;
			char_x_offset	=	-x;
			char_draw_width	+=	x;
			x		=	0;
			clip	=	1;
		}
		if(y<0)
		{
			if((y+char_v_offset+char_height)<0)
				goto done_char;
			if((y+char_v_offset)<0)
			{					
				char_y_offset	=	-(y+char_v_offset);
				char_draw_height+=	y+char_v_offset;
				char_v_offset	=	0;
			}
			else
			{
				char_v_offset	+=	y;
			}
			char_y	=	0;
			clip	=	1;
		}
		if((x+char_draw_width)>=WorkWindowWidth)
		{
			char_draw_width	-=	(x+char_draw_width)-WorkWindowWidth;
			clip	=	1;
		}
		if((y+char_v_offset+char_draw_height)>=WorkWindowHeight)
		{
			char_draw_height-=	(char_y+char_v_offset+char_draw_height)-WorkWindowHeight;
			if(char_draw_height<=0)
				goto done_char;
			clip	=	1;
		}

		string_dest	=	(UWORD*)WorkWindow+x+((char_y+char_v_offset)*WorkScreenWidth>>1);

		if(clip)
		{
			for(c0=0;c0<char_draw_height;c0++,string_dest+=WorkScreenWidth,the_char_def+=char_width)
			{
				the_pixel	=	the_char_def+(char_y_offset*char_width)+char_x_offset;
				for(c1=0;c1<char_draw_width;c1++)
				{
					if(*(the_pixel++))
					{
						*(string_dest+c1)	=	(UWORD)colour;
					}
				}
			}
		}
		else
		{
			DRAW_CHAR16
		}
done_char:
		x	+=	char_draw_width+1;
		if(x>=WorkWindowWidth)
			return;
	}
}

void	QuickTextC32(SLONG x,SLONG y,CBYTE *the_string,ULONG colour)
{
	ULONG		*string_dest;
	UBYTE		*the_char_def,
				*the_pixel;
	SLONG		c0,c1,
				char_draw_height,
				char_draw_width,
				char_height,
				char_width,
				char_v_offset,
				char_x_offset,
				char_y,
				char_y_offset,
				clip;
	

	if(x>=WorkWindowWidth || y>=WorkWindowHeight)
		return;

	if(the_string)
	while(*the_string)
	{
		the_char_def	=	CharTable[*(the_string++)];
		char_width		=	*(the_char_def++);
		char_height		=	*(the_char_def++);
		char_v_offset	=	*(the_char_def++);
		char_draw_width	=	char_width;
		char_draw_height=	char_height;
		char_x_offset	=	0;
		char_y_offset	=	0;
		char_y			=	y;

		clip	=	0;
		if(x<0)
		{
			if((x+char_width)<0)
				goto done_char;
			char_x_offset	=	-x;
			char_draw_width	+=	x;
			x		=	0;
			clip	=	1;
		}
		if(y<0)
		{
			if((y+char_v_offset+char_height)<0)
				goto done_char;
			if((y+char_v_offset)<0)
			{					
				char_y_offset	=	-(y+char_v_offset);
				char_draw_height+=	y+char_v_offset;
				char_v_offset	=	0;
			}
			else
			{
				char_v_offset	+=	y;
			}
			char_y	=	0;
			clip	=	1;
		}
		if((x+char_draw_width)>=WorkWindowWidth)
		{
			char_draw_width	-=	(x+char_draw_width)-WorkWindowWidth;
			clip	=	1;
		}
		if((y+char_v_offset+char_draw_height)>=WorkWindowHeight)
		{
			char_draw_height-=	(char_y+char_v_offset+char_draw_height)-WorkWindowHeight;
			if(char_draw_height<=0)
				goto done_char;
			clip	=	1;
		}

		string_dest	=	(ULONG*)WorkWindow+x+((char_y+char_v_offset)*WorkScreenWidth>>2);

		if(clip)
		{
			for(c0=0;c0<char_draw_height;c0++,string_dest+=WorkScreenWidth,the_char_def+=char_width)
			{
				the_pixel	=	the_char_def+(char_y_offset*char_width)+char_x_offset;
				for(c1=0;c1<char_draw_width;c1++)
				{
					if(*(the_pixel++))
					{
						*(string_dest+c1)	=	(UWORD)colour;
					}
				}
			}
		}
		else
		{
			DRAW_CHAR32
		}
done_char:
		x	+=	char_draw_width+1;
		if(x>=WorkWindowWidth)
			return;
	}
}

//---------------------------------------------------------------

void	QuickChar8(SLONG x,SLONG y,CBYTE the_char,ULONG colour)
{
	UBYTE		*the_char_def,
				*string_dest;
	ULONG		char_height,
				char_width;
	ULONG		c0,c1;
	
	
	the_char_def	=	CharTable[the_char];
	char_width		=	*(the_char_def++);
	char_height		=	*(the_char_def++);
	string_dest		=	WorkWindow+x+((y+*(the_char_def++))*WorkScreenWidth);
	DRAW_CHAR
}

void	QuickChar16(SLONG x,SLONG y,CBYTE the_char,ULONG colour)
{
	UBYTE		*the_char_def;
	UWORD		*string_dest;
	ULONG		char_height,
				char_width;
	ULONG		c0,c1;
	
	
	the_char_def	=	CharTable[the_char];
	char_width		=	*(the_char_def++);
	char_height		=	*(the_char_def++);
	string_dest		=	(UWORD*)WorkWindow+x+((y+*(the_char_def++))*WorkScreenWidth>>1);
	DRAW_CHAR16
}

void	QuickChar32(SLONG x,SLONG y,CBYTE the_char,ULONG colour)
{
	UBYTE		*the_char_def;
	ULONG		*string_dest;
	ULONG		char_height,
				char_width;
	ULONG		c0,c1;
	
	
	the_char_def	=	CharTable[the_char];
	char_width		=	*(the_char_def++);
	char_height		=	*(the_char_def++);
	string_dest		=	(ULONG*)WorkWindow+x+((y+*(the_char_def++))*WorkScreenWidth>>2);
	DRAW_CHAR32
}
//---------------------------------------------------------------

void	QuickCharC8_16_32(SLONG x,SLONG y,CBYTE the_char,ULONG colour)
{
	if(x>=WorkWindowWidth || y>=WorkWindowHeight)
		return;

	if(x<0 || y<0)
		return;

	QuickChar(x,y,the_char,colour);
}

//---------------------------------------------------------------

SLONG	QTStringWidth(CBYTE *the_string)
{
	SLONG		width	=	0;
	
	if(the_string)
	while(*the_string)
	{
		width	+=	(*CharTable[*(the_string++)])+1;
	}
	return	width;
}

//---------------------------------------------------------------
