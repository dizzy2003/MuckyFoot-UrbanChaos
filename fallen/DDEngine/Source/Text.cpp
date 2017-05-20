//	Text.cpp
//	Guy Simmons, 17th May 1998.

#include	<DDLib.h>
#include	"poly.h"
#include	"texture.h"
#include	"vertexbuffer.h"
#include	"polypoint.h"
#include	"renderstate.h"
#include	"polypage.h"

//---------------------------------------------------------------
#define	TEXTURE_STEP	0.00390625f

//D3DTexture		font_page;

extern D3DTexture TEXTURE_texture[];
extern SLONG      TEXTURE_page_font;

#define font_page (TEXTURE_texture[TEXTURE_page_font])

//---------------------------------------------------------------

SLONG	text_width(CBYTE *message,SLONG font_id,SLONG *char_count)
{
	SLONG				count	=	0,
						width	=	0;
	Char				*the_char;
	Font				*the_font;

	
	the_font	=	font_page.GetFont(font_id);
	if(!the_font)
		return	0;

	while(*message && *message!='\n' && *message!='\r')
	{
		if(*message==32)
		{
			width	+=	5;
		}
		else
		{
			the_char	=	&the_font->CharSet[*message-33];
			if(the_char)
			{
				width	+=	the_char->Width+1;
			}
		}
		message++;
		count++;
	}

	if(char_count)
		*char_count	=	count;

	return	width;
}

//---------------------------------------------------------------

SLONG	text_height(CBYTE *message,SLONG font_id,SLONG *char_count)
{
	SLONG				count	=	0,
						height	=	0;
	Char				*the_char;
	Font				*the_font;

	
	the_font	=	font_page.GetFont(font_id);
	if(!the_font)
		return	0;

	while(*message && *message!='\n' && *message!='\r')
	{
		the_char	=	&the_font->CharSet[*message-33];
		if(the_char)
		{
			if(the_char->Height>height)
				height	=	the_char->Height+1;
		}

		message++;
		count++;
	}

	if(char_count)
		*char_count	=	count;

	return	height;
}

//---------------------------------------------------------------

#define	TEXT_TOP		280
#define	TEXT_BOT		430

BOOL		text_fudge	=	FALSE;
ULONG       text_colour;

void	draw_text_at(float x, float y,CBYTE *message,SLONG font_id)
{
	float				offset_x,
						offset_y;
	SLONG				b_colour,
						t_colour;
	Char				*the_char;
	Font				*the_font;
	POLY_Point			pp[4],
						*quad[4];


	the_font	=	font_page.GetFont(font_id);
	if(!the_font)
		return;

	offset_x	=	(float)x;
	offset_y	=	(float)y;

	quad[0]	=	&pp[0];
	quad[1] =	&pp[1];
	quad[2] =	&pp[2];
	quad[3] =	&pp[3];

	while(*message)
	{
		switch(*message)
		{
			case	10:
			case	13:
				offset_x	=	(float)x;
				offset_y	+=	(float)the_font->CharSet['y'-33].Height+2;
				break;
			case	32:
				offset_x	+=	5.0f;
				break;

			default:
				if (WITHIN(*message, 33, 127))
				{
					the_char	=	&the_font->CharSet[*message-33];
					if(the_char)
					{
						if(text_fudge)
						{
							if(offset_y<TEXT_TOP)
							{
								t_colour	=	(((SLONG)offset_y-TEXT_TOP)*20)+255;
								if(t_colour<0)
									t_colour	=	0;
								t_colour	*=	0x00010101; //(t_colour<<24)|0x00ffffff;

								b_colour	=	((((SLONG)offset_y+the_char->Height)-TEXT_TOP)*20)+255;
								if(b_colour>255)
									b_colour	=	255;
								else if(b_colour<0)
									b_colour	=	0;
								b_colour	*=	0x00010101; //(b_colour<<24)|0x00ffffff;
							}
							else if((offset_y+the_char->Height)>TEXT_BOT)
							{
								b_colour	=	((TEXT_BOT-((SLONG)offset_y+the_char->Height))*20)+255;
								if(b_colour<0)
									b_colour	=	0;
								b_colour	*=	0x00010101; //(b_colour<<24)|0x00ffffff;

								t_colour	=	((TEXT_BOT-(SLONG)offset_y)*20)+255;
								if(t_colour>255)
									t_colour	=	255;
								else if(t_colour<0)
									t_colour	=	0;
								t_colour	*=	0x00010101; //(t_colour<<24)|0x00ffffff;
							}
							else
							{
								t_colour	=	b_colour	=	0x00ffffff;
							}
						}
						else
						{
							t_colour	=	b_colour	=	text_colour;
						}

						pp[0].X		=	offset_x;
						pp[0].Y		=	offset_y;
						pp[0].Z		=	0.5f;
						pp[0].u		=	the_char->X*TEXTURE_STEP;
						pp[0].v		=	the_char->Y*TEXTURE_STEP;
						pp[0].colour	=	t_colour;
						pp[0].specular=	0;

						pp[1].X		=	offset_x+the_char->Width+1;
						pp[1].Y		=	offset_y;
						pp[1].Z		=	0.5f;
						pp[1].u		=	(the_char->X+the_char->Width+1)*TEXTURE_STEP;
						pp[1].v		=	the_char->Y*TEXTURE_STEP;
						pp[1].colour	=	t_colour;
						pp[1].specular	=	0;

						pp[2].X		=	offset_x;
						pp[2].Y		=	offset_y+the_char->Height+1;
						pp[2].Z		=	0.5f;
						pp[2].u		=	the_char->X*TEXTURE_STEP;
						pp[2].v		=	(the_char->Y+the_char->Height+1)*TEXTURE_STEP;
						pp[2].colour	=	b_colour;
						pp[2].specular	=	0;

						pp[3].X		=	offset_x+the_char->Width+1;
						pp[3].Y		=	offset_y+the_char->Height+1;
						pp[3].Z		=	0.5f;
						pp[3].u		=	(the_char->X+the_char->Width+1)*TEXTURE_STEP;
						pp[3].v		=	(the_char->Y+the_char->Height+1)*TEXTURE_STEP;
						pp[3].colour	=	b_colour;
						pp[3].specular	=	0;

						POLY_add_quad(quad,POLY_PAGE_TEXT,FALSE,TRUE);

						offset_x	+=	the_char->Width+1.0f;
					}
				}

				break;
		}

		message++;
	}
}

//---------------------------------------------------------------

void	draw_centre_text_at(float x, float y,CBYTE *message,SLONG font_id,SLONG flag)
{
	CBYTE				temp;
	SLONG				char_count	=	0;
	float				height;
	float				width;

	height	=	(float) text_height("y",font_id,NULL);
	while(*message)
	{
 		width	=	(float) text_width(message,font_id,&char_count);
//		LogText(" message >%s< len %d \n",message,char_count);

		temp	=	*(message+char_count);
		if(flag)
		*(message+char_count)	=	0;

//		LogText(" draw text message >%s< width %d \n",message,width);
		draw_text_at((640-((SLONG)width))>>1,y,message,font_id);
		if(!flag)
			break;

		y	+=	height+2;

		*(message+char_count)	=	temp;


		message	+=	char_count+1;
	}
}

//---------------------------------------------------------------

void	show_text(void)
{
	return;

	PolyPage			*pa = &POLY_Page[POLY_PAGE_TEXT];

#ifndef TARGET_DC
	DDCOLORKEY			ck;
	if (font_page.GetD3DTexture())
	{
		ck.dwColorSpaceLowValue		=	0;
		ck.dwColorSpaceHighValue	=	0;
		font_page.SetColorKey(DDCKEY_SRCBLT,&ck);
	}
	else
	{
//		return;
	}
#endif




#define SET_RENDER_STATE(I,V)	pa->RS.SetRenderState(I,V)


	if (pa->NeedsRendering())
	{
		BEGIN_SCENE;

		SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAG,D3DFILTER_NEAREST);
		SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMIN,D3DFILTER_NEAREST);
		SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE,FALSE);
//		SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREHANDLE,texture);
		//pa->RS.SetTexture(pa->RS.SetTexture(handle));

#ifndef TARGET_DC
		SET_RENDER_STATE(D3DRENDERSTATE_COLORKEYENABLE,TRUE);
		if(text_fudge)
#endif
		{
			SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATEALPHA);
			SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
			SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);
			SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
		}
#ifndef TARGET_DC
		else
		{
			SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
		}
#endif

		pa->RS.SetChanged();
		pa->Render(the_display.lp_D3D_Device);

		END_SCENE;
	}
}

//---------------------------------------------------------------

